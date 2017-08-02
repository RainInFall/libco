#ifndef __CO_LOOP_H__
#define __CO_LOOP_H__

#include <stddef.h>
#include "co.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct co_loop_t co_loop_t;

size_t co_loop_size(void);

int co_loop_init(co_loop_t* loop, co_t* co);

int co_loop_deinit(co_loop_t* loop);

#ifdef __cplusplus
}
#endif

#endif /* __CO_LOOP_H__ */