#include "co_loop.h"

co_loop_t* _uv_loop_get_co_loop(uv_loop_t* loop) {
  return (co_loop_t*)loop;
}
