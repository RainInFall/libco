#ifndef __CO_DEBUG_H__
#define __CO_DEBUG_H__

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __DEBUG
#define CO_LOG_DEBUG(format, ...) \
    printf("DEBUG[%s:%d][%s] "format, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#else
#define CO_LOG_DEBUG(format, ...) 
#endif

#ifdef __cplusplus
}
#endif

#endif /* __CO_DEBUG_H__ */