#include <assert.h>
#include "co_tcp.h"
#include "internal/co_thread.h"
#include "internal/co_loop.h"

struct co_tcp_t {
  uv_tcp_t handle;
};

size_t co_tcp_size(void) {
  return sizeof(co_tcp_t);
}

int co_tcp_init(co_loop_t* loop, co_tcp_t* tcp) {
  tcp->handle.data = NULL;
  return uv_tcp_init(&loop->handle, &tcp->handle);
}

typedef struct {
  co_thread_t* thread;
} co_tcp_close_req_t;

static void tcp_close_cb(uv_handle_t* handle) {
  co_tcp_close_req_t* req = (co_tcp_close_req_t*)handle->data;
  co_thread_schedule(req->thread);
}

static void tcp_close_thread_suspend_cb(co_thread_t* current, void* data) {
  uv_tcp_t* tcp_handle = (uv_tcp_t*)data;
  co_tcp_close_req_t* req = (co_tcp_close_req_t*)tcp_handle->data;
  req->thread = current;
  uv_close((uv_handle_t*)tcp_handle, tcp_close_cb);
}

void co_tcp_close(co_tcp_t* tcp) {
  co_tcp_close_req_t req;
  co_loop_t* loop;

  if (uv_is_closing((uv_handle_t*)&tcp->handle)) {
    return;
  }
  
  assert(tcp->handle.data == NULL);
  
  loop = co_loop_get_from_uv_loop(tcp->handle.loop);
  tcp->handle.data = &req;
  co_thread_replace(&loop->loop_thread, tcp_close_thread_suspend_cb, &tcp->handle);
}