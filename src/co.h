#ifndef __CO_H__
#define __CO_H__

#include "co_thread.h"
#include "co_lock.h"
#include "co_loop.h"
#include "co_tcp.h"

#ifdef __cplusplus
extern "C" {
#endif

int co_init(void);

int co_version(void);

#ifdef __cplusplus
}
#endif

#endif /* __CO_H__ */
