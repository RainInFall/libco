#include <stdbool.h>
#include "co_lock.h"

struct co_mutex_t {
  bool locked;
};

size_t co_mutex_size(void) {
  return sizeof(co_mutex_t);
}

int co_mutex_create(co_mutex_t* mutex) {
  mutex->locked = false;
  return 0;
}

void co_mutex_destroy(co_mutex_t* mutex) {

}
