#ifndef __CO_INTERNAL_THREAD_H__
#define __CO_INTERNAL_THREAD_H__

#include <ucontext.h>
#include "../co.h"
#include "../co_thread.h"

struct co_thread_t {
  ucontext_t handle;
#define CO_THREAD_STATUS_SUSPEND (-1)
#define CO_THREAD_STATUS_RUNNING (0)
#define CO_THREAD_STATUS_FINISH (1)
  void* data;
  int status;
  co_t* co;
  co_thread_t* link[2];
  size_t *link_size;
  char stack[1];
  
};

void co_add_thread(co_t* co, co_thread_t* thread);

void co_remove_thread(co_t* co, co_thread_t* thread);

#endif /* __CO_INTERNAL_THREAD_H__ */