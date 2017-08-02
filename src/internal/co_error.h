#ifndef __CO_INTERNAL_ERROR_H__
#define __CO_INTERNAL_ERROR_H__

#include <stdlib.h>

#define co_fatal(msg) \
  do {\
    *((char*)NULL) = 0xff;\
    abort();\
  } while (0)

#endif /* __CO_INTERNAL_ERROR_H__ */