#include <uv.h>
#include "internal/co_loop.h"
#include "internal/co_thread.h"
#include "co_tcp.h"

struct co_tcp_t {
  uv_tcp_t handle;
  ssize_t read_status;
};

typedef struct co_tcp_req_t {
  int status;
  co_thread_t* thread;
}co_tcp_req_t;

typedef struct co_tcp_read_req_t {
  ssize_t read_size;
  size_t buf_size;
  char* buf;
  ssize_t status;
  co_thread_t* thread;
}co_tcp_read_req_t;

size_t co_tcp_size(void) {
  return sizeof(co_tcp_t);
}

int co_tcp_init(co_loop_t* loop, co_tcp_t* tcp) {
  tcp->handle.data = NULL;
  tcp->read_status = 0;
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
    _co_loop_run(tcp->handle.loop);
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
    _co_loop_run(server->handle.loop);
  }

  req_data.status = 0;
  req_data.thread = co_thread_current();
  client->handle.data = &req_data;
  server->handle.data = client;

  while(client->handle.data != NULL) {
    _co_loop_run(server->handle.loop);
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
    _co_loop_run(tcp->handle.loop);
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
    _co_loop_run(tcp->handle.loop);
  }
}

static void alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
  co_tcp_read_req_t* req_data = (co_tcp_read_req_t*)handle->data;
  buf->base = req_data->buf + req_data->read_size;
  buf->len = req_data->buf_size - req_data->read_size;
}

static void read_cb(uv_stream_t* handle, ssize_t nread, const uv_buf_t* buf) {
  co_tcp_read_req_t* req_data = (co_tcp_read_req_t*)handle->data;
  if (nread < 0) {
    uv_read_stop(handle);
    req_data->status = nread;
    handle->data = NULL;
    _co_thread_switch(req_data->thread);
    return;
  }

  req_data->read_size += nread;
  if (req_data->read_size == req_data->buf_size) {
    uv_read_stop(handle);
    req_data->status = 0;
    handle->data = NULL;
    _co_thread_switch(req_data->thread);
  }
}

ssize_t co_tcp_read(co_tcp_t* tcp, void* buf, size_t size) {
  if (0 != tcp->read_status) {
    return tcp->read_status;
  }

  co_tcp_read_req_t req_data;
  req_data.thread = co_thread_current();
  req_data.buf_size = size;
  req_data.buf = buf;

  tcp->handle.data = &req_data;

  int ret = uv_read_start((uv_stream_t*)&tcp->handle, alloc_cb, read_cb);
  if (0 != ret) {
    return ret;
  }

  while(tcp->handle.data != NULL) {
    _co_loop_run(tcp->handle.loop);
  }

  if (0 != req_data.status) {
    return tcp->read_status = req_data.status;
  }

  return req_data.read_size;
}
