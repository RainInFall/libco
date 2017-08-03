#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <uv.h>
#include "co_list.h"
#include "internal/co_thread.h"
#include "internal/co.h"
#include "internal/co_error.h"
#include "internal/co_loop.h"

size_t co_thread_size(size_t size_stack) {
  return sizeof(co_thread_t) + size_stack;
}

static void close_thread_sleep_timer_cb(uv_handle_t* handle) {
  co_thread_t* thread = (co_thread_t*) handle->data;
  co_thread_resume(thread);
}

static void co_thread_entry(co_thread_t* thread, co_thread_func_t entry, void* data) {
  int i;
  co_thread_t* join_thread;

  assert(co_thread_current(thread->co) == thread);

  thread->status = CO_THREAD_STATUS_RUNNING;
  entry(thread->data);
  thread->status = CO_THREAD_STATUS_FINISH;

  /* thread end, release the join thread */
  join_thread = thread->join_link;
  for (i = 0; i < thread->join_link_size; ++i) {
    co_thread_schedule(join_thread);
    join_thread = co_list_next(join_thread);
  }
  /* close sleep timer */
  if (thread->sleep_timer.data) {
    uv_close((uv_handle_t*)&thread->sleep_timer, close_thread_sleep_timer_cb);
    co_thread_suspend(co_loop_get_co(co_loop_get_from_uv_loop(thread->sleep_timer.loop)), NULL, NULL);
  }
}

int co_thread_init(co_t* co, co_thread_t* thread, void* data) {
  thread->co = co;
  thread->status = CO_THREAD_STATUS_SUSPEND;
  thread->data = data;
  thread->join_link = NULL;
  thread->join_link_size = 0;
  thread->sleep_timer.data = NULL; /* mark uninitial */

  return 0;
}

void co_thread_suspend(co_t* co, co_thread_suspend_cb_t cb, void* data) {
  co_thread_t* current = co_thread_current(co);
  /*
    The current_thread just head of the schedule list, not the current threas here!
  */
  co_list_remove(co->current_thread, current);
  assert(0 != co->link_size);
  current->status = CO_THREAD_STATUS_SUSPEND;

  if (cb) {
    cb(current, data);
  }
  
  if (0 != swapcontext(&current->handle, &co->schedule_handle)) {
    assert(NULL);
    return;
  }
  /* Only call co_thread_resume can jump here, so no need to set current thread */
  assert(co_thread_current(co) == current);
}

bool co_contains_thread(co_thread_t* head, co_thread_t* new_thread) {
  const size_t size = co_list_size(head);
  int i;

  for (i = 0; i < size; ++i) {
    if (head == new_thread) {
      return true;
    }
    head = co_list_next(head);
  }

  return false;
}

void co_thread_resume(co_thread_t* thread) {
  assert(thread->status == CO_THREAD_STATUS_SUSPEND);

  co_thread_t* current = co_thread_current(thread->co);
  
  thread->status = CO_THREAD_STATUS_RUNNING;
  co_add_thread(thread->co, thread);

  thread->co->current_thread = thread;
  if (0 != swapcontext(&current->handle, &thread->handle)) {
    co_fatal(NULL);
  }
  thread->co->current_thread = current;
}

void co_thread_schedule(co_thread_t* thread) {
  assert(co_thread_is_suspend(thread));
  thread->status = CO_THREAD_STATUS_RUNNING;
  co_add_thread(thread->co, thread);
}

void co_thread_replace(co_thread_t* thread, co_thread_suspend_cb_t cb, void* data) {
  co_t* co = thread->co;
  co_thread_t* current = co_thread_current(co);

  assert(current != thread);
  assert(!co_contains_thread(current, thread));
  assert(co_thread_is_suspend(thread));

  co_add_thread(co, thread);
  co_remove_thread(co, current);

  current->status = CO_THREAD_STATUS_SUSPEND;

  if (cb) {
    cb(current, data);
  }
  /* Do not go to schedule, need to set current thread manully */
  co_set_current(co, thread);
  if (0 != swapcontext(&current->handle, &thread->handle)) {
    co_fatal(NULL);
    return;
  }
  /* Only call co_thread_resume can jump here, so no need to set current thread */
  assert(co_thread_current(co) == current);
}

int co_thread_create(co_thread_t* thread, size_t stack_size, co_thread_func_t entry) {  
  if (0 != getcontext(&thread->handle)) {
    return uv_translate_sys_error(errno);
  }
  thread->handle.uc_stack.ss_sp = thread->stack;
  thread->handle.uc_stack.ss_size = stack_size;
  thread->handle.uc_link = &thread->co->schedule_handle;
  
  makecontext(&thread->handle, (void(*)(void))co_thread_entry, 2, thread, entry);

  co_thread_resume(thread);

  return 0;
}

static void co_thread_suspend_join_cb(co_thread_t* current, void* data) {
  co_thread_t* thread = (co_thread_t*)data;

  if (NULL == thread->join_link) {
    co_list_init(current, &thread->join_link_size);
    thread->join_link = current;
  } else {
    co_list_shift(thread->join_link, current);
  }
}

void co_thread_join(co_thread_t* thread) {
  if (thread->status == CO_THREAD_STATUS_FINISH) {
    return;
  }
  co_thread_suspend(thread->co, co_thread_suspend_join_cb, thread);
}
/*
void co_thread_yield(co_t* co) {
  co_thread_t* current = co_thread_current(co);

  if (0 != swapcontext(&current->handle, &co->schedule_handle)) {
    assert(NULL);
    return;
  }
}
*/
co_thread_t* co_thread_current(co_t* co) {
  return co->current_thread;
}

bool co_thread_is_suspend(co_thread_t* thread) {
  return thread->status == CO_THREAD_STATUS_SUSPEND;
}
