#include <assert.h>
#include <stdio.h>
#include "internal/co_loop.h"
#include "co_thread.h"

#ifndef CO_MAIN_LOOP_STACK_SIZE
#define CO_MAIN_LOOP_STACK_SIZE (56 * 1024)
#endif

size_t co_loop_size(void) {
  return sizeof(co_loop_t);
}

static void main_loop(void* data) {
  co_loop_t* loop = (co_loop_t*)data;
  while (!loop->stop_flag) {
    uv_run(&loop->handle, UV_RUN_DEFAULT);
    co_thread_yield();
  }
}

int co_loop_init(co_loop_t* loop) {
  int ret = uv_loop_init(&loop->handle);
  if (0 != ret) {
    return ret;
  }

  loop->stop_flag = false;
  ret = co_thread_create(&loop->thread, CO_MAIN_LOOP_STACK_SIZE, main_loop, loop);
  
  return ret;
}

void co_loop_stop(co_loop_t* loop) {
  uv_stop(&loop->handle);
}

int co_loop_close(co_loop_t* loop) {
  loop->stop_flag = true;
  co_thread_join(&loop->thread);
  return uv_loop_close(&loop->handle);
}

void _co_loop_run(uv_loop_t* handle) {
  co_loop_t* loop = _uv_loop_get_co_loop(handle);
  assert(!loop->stop_flag);
  _co_thread_switch(&loop->thread);
}

co_loop_t* _uv_loop_get_co_loop(uv_loop_t* handle) {
  return (co_loop_t*)handle;
}
