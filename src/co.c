#include <assert.h>
#include <uv.h>
#include "internal/co.h"
#include "internal/co_thread.h"
#include "co_list.h"

#define CO_VERSION_MAJOR (0)
#define CO_VERSION_MINUS (0)
#define CO_VERSION_PATCH (1)

int co_version(void) {
  return (((CO_VERSION_MAJOR) * 100) + CO_VERSION_MINUS) * 100 + CO_VERSION_PATCH;
}

size_t co_size(void) {
  return sizeof(co_t);
}

static void co_schedule(co_t* co) {
  int i;
  co_thread_t* thread;
  size_t link_size;

  if (0 != getcontext(&co->schedule_handle)) {
    assert(NULL);
  }

  if (co->status == CO_STATUS_RUNNING) {
    link_size = co->link_size;
    for(i = 0, thread = co->current_thread; i < link_size; ++i, thread = co_list_next(thread)) {      
      if (thread->status == CO_THREAD_STATUS_FINISH) {
        co_list_remove(thread);
      } else if (thread->status == CO_THREAD_STATUS_RUNNING) {
        co->current_thread = thread;
        setcontext(&thread->handle);
      } else {
        assert(NULL);
      }
    }
  } else if (co->status == CO_STATUS_NONE) {
    co->status = CO_STATUS_RUNNING;
    if (0 != setcontext(&co->main_thread.handle)) {
      assert(NULL);
    }
  }

  assert(co->status != CO_STATUS_FINISH);

  co->status = CO_STATUS_RUNNING;
}

int co_init(co_t* co) {
  co->status = CO_STATUS_NONE;

  co->main_thread.status = CO_THREAD_STATUS_RUNNING;
  co->main_thread.co = co;

  co_list_init(&co->main_thread, &co->link_size);

  co->current_thread = &co->main_thread;

  if (0 != getcontext(&co->schedule_handle)) {
    return uv_translate_sys_error(errno);
  }
  co->schedule_handle.uc_stack.ss_sp = co->schedule_stack;
  co->schedule_handle.uc_stack.ss_size = CO_SCHEDULE_STACK_SIZE;
  co->schedule_handle.uc_link = &co->main_thread.handle;
  
  makecontext(&co->schedule_handle, (void(*)(void))co_schedule, 1, co);

  if (0 != swapcontext(&co->main_thread.handle, &co->schedule_handle)) {
    return uv_translate_sys_error(errno);
  }

  return 0;
}