#include <errno.h>
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

int _co_thread_switch(co_thread_t* next_thread) {
  co_thread_t* this_thread = co_thread_current();

  if (!co_thread_is_running(next_thread)) {
    return -1;
  }

  _set_thread_current(next_thread);
  next_thread->handle.uc_link = &this_thread->handle;
  printf("switch thread, [%p] next [%p]\n", next_thread, this_thread);
  if (0 != swapcontext(&this_thread->handle, &next_thread->handle)) {
    _set_thread_current(this_thread);
    printf("switch thread error\n");
    return errno;
  }
  _set_thread_current(this_thread);

  return 0;
}
