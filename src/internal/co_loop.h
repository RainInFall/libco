#ifndef __CO_INTERNAL_LOOP_H__
#define __CO_INTERNAL_LOOP_H__

#include <uv.h>
#include "../co_loop.h"
#include "co_thread.h"

#ifndef CO_LOOP_STACK_SIZE
#define CO_LOOP_STACK_SIZE (56 * 1024)
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct co_loop_t {
  uv_loop_t handle;
#define CO_LOOP_STATUS_NONE (-1)
#define CO_LOOP_STATUS_RUNNING (0)
#define CO_LOOP_STATUS_FINISHING (1)
#define CO_LOOP_STATUS_FINISHED (2)
  int status;
  co_thread_t loop_thread;
  char stack[CO_LOOP_STACK_SIZE];
};

/* When loop has new work to do, call this. */
void co_loop_run(co_loop_t* loop);

co_loop_t* co_loop_get_from_uv_loop(uv_loop_t* loop);

co_t* co_loop_get_co(co_loop_t* loop);

co_t* uv_loop_get_co(uv_loop_t* loop);

#ifdef __cplusplus
}
#endif

#endif /* __CO_INTERNAL_LOOP_H__ */