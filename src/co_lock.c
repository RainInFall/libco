#include <stdbool.h>
#include <stdio.h>
#include <assert.h>
#include "internal/co_thread.h"
#include "co_lock.h"

struct co_mutex_t {
  bool locked;
  co_thread_t* thread;
};

size_t co_mutex_size(void) {
  return sizeof(co_mutex_t);
}

int co_mutex_create(co_mutex_t* mutex) {
  mutex->locked = false;
  mutex->thread = NULL;
  return 0;
}

void co_mutex_destroy(co_mutex_t* mutex) {
  assert(NULL == mutex->thread);
}

bool co_mutex_try_lock(co_mutex_t* mutex) {
  if (!mutex->locked) {
    mutex->locked = true;
    return true;
  }
  return false;
}

void co_mutex_lock(co_mutex_t* mutex) {
  if (co_mutex_try_lock(mutex)) {
    return;
  }

  co_thread_t* current_thread = co_thread_current();
  current_thread->mutex_link = mutex->thread;
  mutex->thread = current_thread;

  do {
    co_thread_yield();
  } while(!co_mutex_try_lock(mutex));

  assert(current_thread == mutex->thread);
  mutex->thread = mutex->thread->mutex_link;
}

void co_mutex_unlock(co_mutex_t* mutex) {
  printf("enter unlock %p\n", mutex);
  mutex->locked = false;
  if (mutex->thread) {
    _co_thread_switch(mutex->thread);
  }
}
