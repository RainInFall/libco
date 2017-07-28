#ifndef __CO_NET_H__
#define __CO_NET_H__

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct co_loop_t co_loop_t;

size_t co_loop_size(void);

int co_loop_init(co_loop_t* loop);

int co_loop_close(co_loop_t* loop);

void co_loop_stop(co_loop_t* loop);

#ifdef __cplusplus
}
#endif

#endif /* __CO_NET_H__ */
