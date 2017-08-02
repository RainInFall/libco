#ifndef __CO_INTERNAL_THREAD_H__
#define __CO_INTERNAL_THREAD_H__

#include <ucontext.h>
#include <stdbool.h>
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
  /* schedule list part*/
  co_thread_t* link[2];
  size_t *link_size;
  /* join part */
  co_thread_t* join_link;
  size_t join_link_size;
  /* stack */
  char stack[1];
};

typedef void (*co_thread_suspend_cb_t)(co_thread_t* current, void* data);

void co_thread_suspend(co_t* co, co_thread_suspend_cb_t cb, void* data);

void co_thread_resume(co_thread_t* thread);

void co_thread_schedule(co_thread_t* thread);

void co_thread_replace(co_thread_t* thread, co_thread_suspend_cb_t cb, void* data);

bool co_thread_is_suspend(co_thread_t* thread);

bool co_contains_thread(co_thread_t* head, co_thread_t* new_thread);

#endif /* __CO_INTERNAL_THREAD_H__ */