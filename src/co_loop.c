#include <assert.h>
#include <stddef.h>
#include "internal/co_loop.h"
#include "internal/co_thread.h"

size_t co_loop_size(void) {
  return sizeof(co_loop_t);
}

void co_loop_main(void* data) {
  int ret;
  co_loop_t* loop = (co_loop_t*)data;

  loop->status = CO_LOOP_STATUS_RUNNING;
  
  do {
    ret = uv_run(&loop->handle, UV_RUN_DEFAULT);
    if (ret != 0 || loop->status == CO_LOOP_STATUS_FINISHING) {
      /*  uv_stop was called */
      loop->status = CO_LOOP_STATUS_FINISHED;
    }
    co_thread_suspend((co_t*)loop->handle.data, NULL, NULL);
  } while (loop->status != CO_LOOP_STATUS_FINISHED);

  assert(CO_LOOP_STATUS_FINISHED == loop->status);
}

int co_loop_init(co_loop_t* loop, co_t* co) {
  int ret;
  
  loop->status = CO_LOOP_STATUS_NONE;
  loop->handle.data = co;
  
  ret = uv_loop_init(&loop->handle);
  if (0 != ret) {
    return ret;
  }

  ret = co_thread_init(co, &loop->loop_thread, loop);
  if (0 != ret) {
    return ret;
  }

  ret = co_thread_create(&loop->loop_thread, CO_LOOP_STACK_SIZE, co_loop_main);

  assert(loop->status != CO_LOOP_STATUS_NONE);
  
  return ret;
}

static void loop_close_cb(uv_handle_t* handle) {

}

static void loop_walk_cb(uv_handle_t* handle, void* arg) {
  uv_close(handle, loop_close_cb);
}

int co_loop_deinit(co_loop_t* loop) {
  if (loop->status == CO_LOOP_STATUS_RUNNING) {
    loop->status = CO_LOOP_STATUS_FINISHING;
    uv_stop(&loop->handle);
    co_thread_resume(&loop->loop_thread);
  }

  assert(CO_LOOP_STATUS_FINISHED == loop->status);

  uv_walk(&loop->handle, loop_walk_cb, loop);
  
  return uv_run(&loop->handle, UV_RUN_DEFAULT);
}

co_loop_t* co_loop_get_from_uv_loop(uv_loop_t* loop) {
  off_t offset = offsetof(co_loop_t, handle);
  return  (co_loop_t*)(void*)(((unsigned char*)loop) - offset);
}