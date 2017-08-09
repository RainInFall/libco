#include <assert.h>
#include <stddef.h>
#include "co_tcp.h"
#include "co_list.h"
#include "internal/co_thread.h"
#include "internal/co_loop.h"
#include "internal/co_error.h"

typedef struct co_tcp_read_req_t co_tcp_read_req_t;
struct co_tcp_read_req_t{
  co_thread_t* thread;
  uv_buf_t buf;
  ssize_t read_size;
  /* list part */
  co_tcp_read_req_t* link[2];
  size_t *link_size;
};

typedef struct co_tcp_accept_req_t co_tcp_accept_req_t;
struct co_tcp_accept_req_t {
  co_thread_t* thread;
  co_tcp_t* client;
  int ret;
  /* list part */
  co_tcp_accept_req_t* link[2];
  size_t *link_size;
};

typedef struct co_tcp_connect_req_t co_tcp_connect_req_t;
struct co_tcp_connect_req_t {
  co_thread_t* thread;
  int status;
};

struct co_tcp_t {
  uv_tcp_t handle;
  /* close */
  co_thread_t* close_thread;
  /* read */
  co_tcp_read_req_t* read_queue;
  size_t  read_queue_size;
  ssize_t read_status;
  /* accept */
  co_tcp_accept_req_t* accept_queue;
  size_t  accept_queue_size;
  int accept_status;
};

size_t co_tcp_size(void) {
  return sizeof(co_tcp_t);
}

int co_tcp_init(co_loop_t* loop, co_tcp_t* tcp) {
  tcp->handle.data = tcp;
  tcp->close_thread = NULL;
  tcp->read_queue = NULL;
  tcp->read_queue_size = 0;
  tcp->read_status = 0;
  tcp->accept_queue = NULL;
  tcp->accept_queue_size = 0;
  tcp->accept_status = 0;

  return uv_tcp_init(&loop->handle, &tcp->handle);
}

static void tcp_alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
  co_tcp_t* tcp = (co_tcp_t*)handle->data;
  co_tcp_read_req_t* req = tcp->read_queue;

  buf->base = req->buf.base + req->read_size;
  buf->len = req->buf.len - req->read_size;
}

static void tcp_read_cb(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf) {
  co_tcp_t* tcp = (co_tcp_t*)stream->data;
  co_tcp_read_req_t *req = tcp->read_queue;

  assert(NULL != req);

  if (0 == nread) {
    return;
  }

  if (nread < 0) {
    tcp->read_status = nread;
    if (0 != uv_read_stop(stream)) {
      co_fatal("uv_read_stop");
    }
    while (tcp->read_queue_size > 0) {
      req = tcp->read_queue;
      co_list_remove(tcp->read_queue, req);
      co_thread_schedule(req->thread);
    }
    return;
  }

  assert(nread <= req->buf.len - req->read_size);
  
  req->read_size += nread;
  if (req->read_size == req->buf.len) {
    co_list_remove(tcp->read_queue, req);
    co_thread_schedule(req->thread);
    if (NULL == tcp->read_queue) {
      if (0 != uv_read_stop(stream)) {
        co_fatal("uv_read_stop");
      }
    }
  }
}

ssize_t co_tcp_read(co_tcp_t* tcp, void* buf, size_t len) {
  co_tcp_read_req_t req;
  co_t* co = uv_loop_get_co(tcp->handle.loop);
  int ret;

  if (0 != tcp->read_status) {
    return tcp->read_status;
  }

  req.thread = co_thread_current(co);
  req.buf = uv_buf_init((char*)buf, len);
  req.read_size = 0;

  if (!tcp->read_queue) {
    ret = uv_read_start((uv_stream_t*)&tcp->handle, tcp_alloc_cb, tcp_read_cb);
    if (0 != ret) {
      return ret;
    }
    tcp->read_queue = &req;
    co_list_init(tcp->read_queue, &tcp->read_queue_size);
  } else {
    co_list_shift(tcp->read_queue, &req);
  }

  co_thread_suspend(co, NULL, NULL);

  /* If other thread read a error, return it. */
  if (tcp->read_status < 0 && req.read_size <= 0) {
    return tcp->read_status;
  }

  assert(&req != tcp->read_queue);

  return req.read_size;
}

int co_tcp_bind(co_tcp_t* tcp, const struct sockaddr* addr) {
  return uv_tcp_bind(&tcp->handle, addr, 0);
}

static void tcp_on_connection(uv_stream_t* stream, int status) {
  co_tcp_t* server = (co_tcp_t*) stream->data;
  co_tcp_accept_req_t *req = server->accept_queue;

  if (status < 0) {
    server->accept_status = status;
    /* release the block accept function */
    while (server->accept_queue_size > 0) {
      req = server->accept_queue;
      co_list_remove(server->accept_queue, req);
      co_thread_schedule(req->thread);
    }
    return;
  }
  
  if (NULL == req) {
    /* mark able to accept */
    server->accept_status = 1;
    return;
  }

  req->ret = uv_accept((uv_stream_t*)&server->handle,
                       (uv_stream_t*)&req->client->handle);
  co_list_remove(server->accept_queue, req);
  co_thread_schedule(req->thread);
}

int co_tcp_listen(co_tcp_t* tcp, int backlog) {
  return uv_listen((uv_stream_t*)&tcp->handle, backlog, tcp_on_connection);
}

int co_tcp_accept(co_tcp_t* server, co_tcp_t* client) {
  co_tcp_accept_req_t req;
  co_t* co = uv_loop_get_co(server->handle.loop);

  assert(co == uv_loop_get_co(client->handle.loop));

  if (server->accept_status < 0) {
    return server->accept_status;
  } else if (server->accept_status > 0) {
    return uv_accept((uv_stream_t*)&server->handle,
                     (uv_stream_t*)&client->handle);
  }

  req.thread = co_thread_current(co);
  req.client = client;
  req.ret = 0;

  if (!server->accept_queue) {
    server->accept_queue = &req;
    co_list_init(server->accept_queue, &server->accept_queue_size);
  } else {
    co_list_shift(server->accept_queue, &req);
  }

  co_thread_suspend(co, NULL, NULL);

  /* If other thread read a error, return it. */
  if (server->accept_status < 0) {
    return server->accept_status;
  }

  return req.ret;
}

static void tcp_close_cb(uv_handle_t* handle) {
  co_tcp_t* tcp = (co_tcp_t*)handle->data;
  co_thread_schedule(tcp->close_thread);
}

void co_tcp_close(co_tcp_t* tcp) {
  co_t* co = uv_loop_get_co(tcp->handle.loop);

  if (uv_is_closing((uv_handle_t*)&tcp->handle)) {
    /*
      There is some thread dealing with closing,
      event it has not done yet.
    */
    return;
  }

  assert(NULL == tcp->close_thread);
  
  tcp->close_thread = co_thread_current(co);
  uv_close((uv_handle_t*)&tcp->handle, tcp_close_cb);

  co_thread_suspend(co, NULL, NULL);
}

static void connect_cb(uv_connect_t* connect, int status) {
  co_tcp_connect_req_t *req = (co_tcp_connect_req_t*)connect->data;
  
  req->status = status;
  co_thread_schedule(req->thread);
}

int co_tcp_connect(co_tcp_t* client, const struct sockaddr* addr) {
  uv_connect_t connect;
  co_tcp_connect_req_t req;
  co_t* co = uv_loop_get_co(client->handle.loop);
  int ret;

  connect.data = &req;
  req.thread = co_thread_current(co);
  req.status = 0;
  
  ret = uv_tcp_connect(&connect, &client->handle, addr, connect_cb);
  if (0 != ret) {
    return ret;
  }

  co_thread_suspend(co, NULL, NULL);
  
  return req.status;
}

co_loop_t* co_tcp_get_loop(co_tcp_t* tcp) {
  return co_loop_get_from_uv_loop(tcp->handle.loop);
}
