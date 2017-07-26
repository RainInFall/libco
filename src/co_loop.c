#include <stdio.h>
#include "internal/co_loop.h"
#include "co_thread.h"

size_t co_loop_size(void) {
  return sizeof(co_loop_t);
}

int co_loop_init(co_loop_t* loop) {
  /* in case of co_close called before co_loop_run */
  loop->thread = co_thread_current();
  return uv_loop_init(&loop->handle);
}

int co_loop_close(co_loop_t* loop) {
  return uv_loop_close(&loop->handle);
}

int co_loop_run(co_loop_t* loop) {
  loop->thread = co_thread_current();
  return uv_run(&loop->handle, UV_RUN_DEFAULT);
}
