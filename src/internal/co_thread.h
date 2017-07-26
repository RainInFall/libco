#ifndef __CO_THREAD_INTERNAL_H__
#define __CO_THREAD_INTERNAL_H__

#include <ucontext.h>
#include <stdint.h>
#include "../co_thread.h"

#ifdef __cplusplus
extern "C" {
#endif

struct co_thread_t {
  ucontext_t handle;
  co_thread_func_t entry;
  void* data;
#define CO_THREAD_SUSPEND (0)
#define CO_THREAD_RUNNING (1)
#define CO_THREAD_EXIT (-1)
  uint8_t running;
  co_thread_t* mutex_link;
  char stack[1];
};

void _set_thread_current(co_thread_t* thread);

co_thread_t* _get_thread_current(void);

co_thread_t* _get_thread_from_ucontext(ucontext_t* context);

int _co_thread_switch(co_thread_t* next_thread);

#ifdef __cplusplus
}
#endif

#endif /* __CO_THREAD_INTERNAL_H__ */
