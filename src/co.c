#include <assert.h>
#include <uv.h>
#include "co_list.h"
#include "internal/co.h"
#include "internal/co_thread.h"
#include "internal/co_error.h"


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
    assert(0 != link_size);
    /*
      The current_thread just head of the schedule list, not the current threas here!
    */
    for(i = 0, thread = co->current_thread; i < link_size; ++i, thread = co_list_next(thread)) {      
      switch (thread->status) {
        case CO_THREAD_STATUS_FINISH: {
          co_list_remove(co->current_thread, thread);
          break;
        }
        case CO_THREAD_STATUS_RUNNING: {
          /* Reset valid current thread */
          co->current_thread = thread;
          setcontext(&thread->handle);
          break;
        }
        case CO_THREAD_STATUS_SUSPEND: {
          break;
        }
        default: {
          assert(NULL);
        }
      } /* switch  */
    }
    /*  Could not find any running thread */
    
  } else if (co->status == CO_STATUS_NONE) {
    co->status = CO_STATUS_RUNNING;
    if (0 != setcontext(&co->main_thread.handle)) {
      assert(NULL);
    }
  }
  /* TODO:Finishing process */
  assert(co->status == CO_STATUS_FINISH);
}

int co_deinit(co_t* co) {
  co->status = CO_STATUS_FINISH;
  if (0 != swapcontext(&co->main_thread.handle, &co->schedule_handle)) {
    return uv_translate_sys_error(errno);
  }
  /* cleanup main thread dummy */
  /* 
    The co_loop_deinit should have been called before, 
    the main thread sleep timer should have been closed before.
  */

  return 0;
}

int co_init(co_t* co) {
  int ret;

  co->status = CO_STATUS_NONE;

  co->main_thread.status = CO_THREAD_STATUS_RUNNING;
  co->main_thread.co = co;

  /* fake init and create thread */
  co_list_init(&co->main_thread, &co->link_size);
  ret = co_thread_init(co, &co->main_thread, co);
  if (0 != ret) {
    return ret;
  }

  co->main_thread.status = CO_STATUS_RUNNING;  
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

void co_add_thread(co_t* co, co_thread_t* thread) {
  assert(co->current_thread != NULL);
  assert(!co_contains_thread(co->current_thread, thread));

  co_list_shift(co->current_thread, thread);
}

void co_remove_thread(co_t* co, co_thread_t* thread) {
  assert(co->link_size > 1);

  co_list_remove(co->current_thread, thread);
}

void co_set_current(co_t* co, co_thread_t* thread) {
  assert(co_contains_thread(co->current_thread, thread));

  co->current_thread = thread;
}

size_t co_thread_runing_count(co_t* co) {
  size_t count = 0, i;
  co_thread_t* thread = co->current_thread;
  
  for (i = 0; i < co->link_size; ++i) {
    if (thread->status == CO_THREAD_STATUS_RUNNING) {
      ++count;
    }
    thread = co_list_next(thread);
  }
  
  return count;
}

bool co_thread_yield(co_t* co) {
  co_thread_t* current = co->current_thread;

  if (co_thread_runing_count(co) > 1) {
    /* avoid current thread to be scheduled first */
    co->current_thread = co_list_next(current);
    if (0 != swapcontext(&current->handle, &co->schedule_handle)) {
      co_fatal("swapcontext");
    }
    return true;
  }

  return false;
}