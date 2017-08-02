#ifndef __CO_H__
#define __CO_H__

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct co_t co_t;

int co_version(void);

size_t co_size(void);

int co_init(co_t* co);

int co_deinit(co_t* co);

#ifdef __cplusplus
}
#endif

#endif /* __CO_H__ */
