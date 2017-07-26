#ifndef __CO_LOOP_INTERNAL_H__
#define __CO_LOOP_INTERNAL_H__

#include <uv.h>
#include "../co_thread.h"
#include "../co_loop.h"

#ifdef __cplusplus
extern "C" {
#endif

struct co_loop_t {
  uv_loop_t handle;
  co_thread_t* thread;
};

co_loop_t* _uv_loop_get_co_loop(uv_loop_t* loop);

#ifdef __cplusplus
}
#endif

#endif /* __CO_LOOP_INTERNAL_H__ */
