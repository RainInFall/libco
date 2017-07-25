#include "co_thread.h"

static co_thread_t* _co_thread_current = NULL;

void _set_thread_current(co_thread_t* thread) {
  _co_thread_current = thread;
}

co_thread_t* _get_thread_current(void) {
  return _co_thread_current;
}

co_thread_t* _get_thread_from_ucontext(ucontext_t* context) {
  return (co_thread_t*)context;
}
