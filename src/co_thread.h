#ifndef __CO_THREAD_H__
#define __CO_THREAD_H__

#include <stddef.h>
#include "co.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct co_thread_t co_thread_t;

typedef void (*co_thread_func_t)(void*);

size_t co_thread_size(size_t stack_size);

int co_thread_init(co_t* co, co_thread_t* thread, void* data);

int co_thread_create(co_thread_t* thread, size_t stack_size, co_thread_func_t entry);

void co_thread_join(co_thread_t* thread);

co_thread_t* co_thread_current(co_t* co);

void co_thread_yield(co_t* co);

#ifdef __cplusplus
}
#endif

#endif /* __CO_THREAD_H__ */