#ifndef __CO_LOCK_H__
#define __CO_LOCK_H__

#include <stddef.h>
#include <stdbool.h>
#include "co.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct co_mutex_t co_mutex_t;

size_t co_mutex_size(void);

int co_mutex_create(co_t* co, co_mutex_t* mutex);

bool co_mutex_try_lock(co_mutex_t* mutex);

void co_mutex_lock(co_mutex_t* mutex);

void co_mutex_unlock(co_mutex_t* mutex);

void co_mutex_destroy(co_mutex_t* mutex);

#ifdef __cplusplus
}
#endif

#endif /* __CO_LOCK_H__ */
