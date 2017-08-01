#include <assert.h>
#include <errno.h>
#include <uv.h>
#include "internal/co_thread.h"
#include "internal/co.h"
#include "co_list.h"

size_t co_thread_size(size_t size_stack) {
  return sizeof(co_thread_t*) + size_stack;
}

void co_add_thread(co_t* co, co_thread_t* thread) {
  co_list_shift(co->current_thread, thread);
}

void co_remove_thread(co_t* co, co_thread_t* thread) {
  co_list_remove(thread);
}

static void co_thread_entry(co_thread_t* thread, co_thread_func_t entry, void* data) {

  thread->co->current_thread = thread;
  thread->status = CO_THREAD_STATUS_RUNNING;
  entry(thread->data);
  thread->status = CO_THREAD_STATUS_FINISH;
}

int co_thread_init(co_t* co, co_thread_t* thread, void* data) {
  thread->co = co;
  thread->data = data;

  return 0;
}

int co_thread_create(co_thread_t* thread, size_t stack_size, co_thread_func_t entry) {  
  if (0 != getcontext(&thread->handle)) {
    return uv_translate_sys_error(errno);
  }
  thread->handle.uc_stack.ss_sp = thread->stack;
  thread->handle.uc_stack.ss_size = stack_size;
  thread->handle.uc_link = &thread->co->schedule_handle;
  
  makecontext(&thread->handle, (void(*)(void))co_thread_entry, 2, thread, entry);

  co_add_thread(thread->co, thread);
  co_thread_switch(thread);

  return 0;
}

void co_thread_join(co_thread_t* thread) {
  while(thread->status != CO_THREAD_STATUS_FINISH) {
    co_thread_yield(thread->co);
  }
}

void co_thread_switch(co_thread_t* thread) {
  co_thread_t* current = co_thread_current(thread->co);
  assert(current != thread);

  thread->co->current_thread = thread;
  if (0 != swapcontext(&current->handle, &thread->handle)) {
    thread->co->current_thread = current;
    assert(NULL);
    return;
  }

  thread->co->current_thread = current;
}

void co_thread_yield(co_t* co) {
  co_thread_t* current = co_thread_current(co);

  if (0 != swapcontext(&current->handle, &co->schedule_handle)) {
    assert(NULL);
    return;
  }
}

co_thread_t* co_thread_current(co_t* co) {
  return co->current_thread;
}
