
#include <errno.h>
#include <stdio.h>
#include "internal/co_thread.h"

static co_thread_t _co_thread_main;

int co_thread_init(void) {
  /* handle will be settted when swapcontext */
  _co_thread_main.entry = NULL;
  _co_thread_main.data = &_co_thread_main;
  _co_thread_main.running = CO_THREAD_RUNNING;
  _co_thread_main.mutex_link = NULL;

  _set_thread_current(&_co_thread_main);
  return 0;
}

size_t co_thread_size(size_t stack_size){
  return stack_size + sizeof(co_thread_t);
}

void co_thread_entry(co_thread_t* thread) {
  thread->running = CO_THREAD_RUNNING;
  thread->entry(thread->data);
  thread->running = CO_THREAD_EXIT;
}

int co_thread_create(co_thread_t* thread,
                     size_t stack_size,
                     co_thread_func_t entry,
                     void* data) {
  co_thread_t* this_thread = co_thread_current();

  getcontext(&thread->handle);
  thread->handle.uc_stack.ss_sp = thread->stack;
  thread->handle.uc_stack.ss_size = stack_size;
  thread->handle.uc_stack.ss_flags = 0;
  thread->handle.uc_link = &this_thread->handle;

  thread->entry = entry;
  thread->data = data;
  thread->running = CO_THREAD_SUSPEND;
  thread->mutex_link = NULL;

  makecontext(&thread->handle, (void (*)(void))co_thread_entry, 1, thread);

  _set_thread_current(thread);
  if (0 != swapcontext(&this_thread->handle, &thread->handle)) {
    _set_thread_current(this_thread);
    return errno;
  }
  _set_thread_current(this_thread);

  return 0;
}

bool co_thread_is_running(co_thread_t* thread) {
  return thread->running == CO_THREAD_RUNNING;
}

co_thread_t* co_thread_current(void) {
  return _get_thread_current();
}

int co_thread_yield(void) {
  ucontext_t* next_handle = co_thread_current()->handle.uc_link;
  co_thread_t* next_thread = _get_thread_from_ucontext(next_handle);
  _co_thread_switch(next_thread);

  return 0;
}

void co_thread_join(co_thread_t* thread) {
  while (co_thread_is_running(thread)) {
    _co_thread_switch(thread);
  }
}
