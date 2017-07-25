#include "co.h"

#define CO_VERSION_MAJOR (0)
#define CO_VERSION_MINUS (0)
#define CO_VERSION_PATCH (1)

int co_init(void) {
  int ret;
  if (0 != (ret=co_thread_init())) {
    return ret;
  }

  return 0;
}

int co_version(void) {
  return (CO_VERSION_MAJOR * 100 + CO_VERSION_MINUS) * 1000 + CO_VERSION_PATCH;
}
