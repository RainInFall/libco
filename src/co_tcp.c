#include <uv.h>
#include "internal/co_loop.h"
#include "internal/co_thread.h"
#include "co_tcp.h"

struct co_tcp_t {
  uv_tcp_t handle;
};

typedef struct co_tcp_req_t {
  int status;
  co_thread_t* thread;
}co_tcp_req_t;

size_t co_tcp_size(void) {
  return sizeof(co_tcp_t);
}

int co_tcp_init(co_loop_t* loop, co_tcp_t* tcp) {
  tcp->handle.data = NULL;
  return uv_tcp_init(&loop->handle, &tcp->handle);
}

static void tcp_connect_cb(uv_connect_t* req, int status) {
  co_tcp_req_t* req_data = (co_tcp_req_t*)req->data;
  req_data->status = status;
  req->data = NULL;
  _co_thread_switch(req_data->thread);
}

int co_tcp_connect(co_tcp_t* tcp, const struct sockaddr* addr) {
  int ret;
  uv_connect_t req;
  co_tcp_req_t req_data;

  req_data.status = 0;
  req_data.thread = co_thread_current();
  req.data = &req_data;

  if (0 != (ret = uv_tcp_connect(&req, &tcp->handle, addr, tcp_connect_cb))) {
    return ret;
  }

  while (req.data != NULL) {
    _co_thread_switch(_uv_loop_get_co_loop(tcp->handle.loop)->thread);
  }

  return req_data.status;
}

int co_tcp_bind(co_tcp_t* tcp, const struct sockaddr* addr) {
  return uv_tcp_bind(&tcp->handle, addr, 0);
}

static void tcp_connection_cb(uv_stream_t* server, int status) {
  co_tcp_t* client = (co_tcp_t*)server->data;
  if (NULL == client) {
    /*
      See the code of libuv, when on_connection does not accept, it will stop
      poll in loop.So we just return here and try accept first when co_accept.
    */
    return;
  }
  co_tcp_req_t* req_data = client->handle.data;

  if (status < 0) {
    req_data->status = status;
    client->handle.data = NULL;
    _co_thread_switch(req_data->thread);
  }

  req_data->status = uv_accept(server, (uv_stream_t*)&client->handle);
  client->handle.data = NULL;
  _co_thread_switch(req_data->thread);
}

int co_tcp_listen(co_tcp_t* tcp, int backlog) {
  int ret;
  if (0 != (ret=uv_listen((uv_stream_t*)tcp, backlog, tcp_connection_cb))) {
    return ret;
  }

  return 0;
}

int co_tcp_accept(co_tcp_t* server, co_tcp_t* client) {
  co_tcp_req_t req_data;
  int ret;

  ret = uv_accept((uv_stream_t*)&server->handle, (uv_stream_t*)&client->handle);
  if (0 == ret) {
    return 0;
  } else if (UV_EAGAIN != ret) {
    return ret;
  }

  while(NULL != server->handle.data) {
    _co_thread_switch(_uv_loop_get_co_loop(server->handle.loop)->thread);
  }

  req_data.status = 0;
  req_data.thread = co_thread_current();
  client->handle.data = &req_data;
  server->handle.data = client;

  while(client->handle.data != NULL) {
    _co_thread_switch(_uv_loop_get_co_loop(server->handle.loop)->thread);
  }

  return req_data.status;
}

static void tcp_shutdown_cb(uv_shutdown_t* req, int status) {
  co_tcp_req_t* req_data = (co_tcp_req_t*)req->data;
  req_data->status = status;

  req->data = NULL;
  _co_thread_switch(req_data->thread);
}

int co_tcp_shutdown(co_tcp_t* tcp) {
  uv_shutdown_t req;
  co_tcp_req_t req_data;
  int ret;

  req_data.status = 0;
  req_data.thread = co_thread_current();

  if (0 != (ret=uv_shutdown(&req, (uv_stream_t*)&tcp->handle, tcp_shutdown_cb))) {
    return ret;
  }

  while(NULL != req.data) {
    _co_thread_switch(_uv_loop_get_co_loop(tcp->handle.loop)->thread);
  }

  return req_data.status;
}

static void tcp_close_cb(uv_handle_t* tcp) {
  co_tcp_req_t* req_data = (co_tcp_req_t*)tcp->data;

  tcp->data = NULL;
  _co_thread_switch(req_data->thread);
}

void co_tcp_close(co_tcp_t* tcp) {
  co_tcp_req_t req_data;

  req_data.thread = co_thread_current();

  tcp->handle.data = &req_data;

  uv_close((uv_handle_t*)&tcp->handle, tcp_close_cb);

  while(NULL != tcp->handle.data) {
    _co_thread_switch(_uv_loop_get_co_loop(tcp->handle.loop)->thread);
  }
}
