#include <assert.h>
#include <stddef.h>
#include "internal/co_loop.h"
#include "internal/co_thread.h"
#include "internal/co.h"
#include "internal/co_error.h"

size_t co_loop_size(void) {
  return sizeof(co_loop_t);
}

void co_loop_main(void* data) {
  co_loop_t* loop = (co_loop_t*)data;

  loop->status = CO_LOOP_STATUS_RUNNING;
  
  do {
    do {
      /* check timers */
      uv_run(&loop->handle, UV_RUN_NOWAIT);
    } while (co_thread_yield(co_loop_get_co(loop)) && loop->status != CO_LOOP_STATUS_FINISHING);
    /*
      Yiled fail means all thread block, no need to no wait.
      But maybe unlock when next event pop from loop.
    */
    if (loop->status == CO_LOOP_STATUS_FINISHING) {
      break;
    }

    do {
      uv_run(&loop->handle, UV_RUN_ONCE);
    } while (!co_thread_yield(co_loop_get_co(loop)));
    /*
      Yield success means there is thread running without block,
      run loop in  none block mode.
    */
    /* This thread should never suspend, only block by event loop. */
  } while (loop->status != CO_LOOP_STATUS_FINISHING);

  loop->status = CO_LOOP_STATUS_FINISHED;
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
    do {
      co_thread_yield(co_loop_get_co(loop));
    } while (CO_LOOP_STATUS_FINISHED != loop->status);
  }

  assert(CO_LOOP_STATUS_FINISHED == loop->status);

  if (0 == uv_loop_close(&loop->handle)) {
    return 0;
  }

  uv_walk(&loop->handle, loop_walk_cb, loop);
  
  return uv_run(&loop->handle, UV_RUN_DEFAULT);
}

static void sleep_cb(uv_timer_t* timer) {
  co_thread_t* thread = (co_thread_t*) timer->data;
  co_thread_resume(thread);
  /* Warning: timer is not valid after resume the sleep thread! */
}

void co_sleep(co_loop_t* loop, uint64_t ms) {
  co_thread_t* current = co_thread_current(co_loop_get_co(loop));

  if (!current->sleep_timer.data) {
    if ( 0 != uv_timer_init(&loop->handle, &current->sleep_timer)) {
      return;
    }
    current->sleep_timer.data = current;
  }  
  
  if (0 != uv_timer_start(&current->sleep_timer, sleep_cb, ms, 0)) {
    return;
  }

  co_thread_suspend(co_loop_get_co(loop), NULL, NULL);
}

co_loop_t* co_loop_get_from_uv_loop(uv_loop_t* loop) {
  off_t offset = offsetof(co_loop_t, handle);
  return (co_loop_t*)(void*)(((unsigned char*)loop) - offset);
}

co_t* co_loop_get_co(co_loop_t* loop) {
  return (co_t*)loop->handle.data;
}

co_t* uv_loop_get_co(uv_loop_t* loop) {
  return co_loop_get_co(co_loop_get_from_uv_loop(loop));
}