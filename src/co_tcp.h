#ifndef __CO_TCP_H__
#define __CO_TCP_H__

#include <stddef.h>
#include "co_loop.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct co_tcp_t co_tcp_t;

size_t co_tcp_size(void);

int co_tcp_init(co_loop_t* loop, co_tcp_t* tcp);

void co_tcp_close(co_tcp_t* tcp);

#ifdef __cplusplus
}
#endif

#endif /* __CO_TCP_H__ */