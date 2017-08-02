#ifndef __CO_INTERNAL_H__
#define __CO_INTERNAL_H__

#include <stddef.h>
#include <ucontext.h>
#include "co_thread.h"

#ifndef CO_SCHEDULE_STACK_SIZE
#define CO_SCHEDULE_STACK_SIZE (56 * 1024)
#endif

struct co_t {
  co_thread_t main_thread;
  co_thread_t* current_thread;
  size_t link_size;
#define CO_STATUS_NONE (-1)
#define CO_STATUS_RUNNING (0)
#define CO_STATUS_FINISH (1)
  int status;
  ucontext_t schedule_handle;
  char schedule_stack[CO_SCHEDULE_STACK_SIZE];
};

void co_thread_switch(co_thread_t* thread);

#endif /* __CO_INTERNAL_H__ */
