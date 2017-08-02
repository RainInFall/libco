#include <assert.h>
#include <stdbool.h>
#include "co_lock.h"
#include "co_list.h"
#include "internal/co_thread.h"
#include "internal/co.h"

struct co_mutex_t{
  bool lock;
  co_thread_t* wait_link;
  size_t wait_link_size;
  co_t* co;
};

size_t co_mutex_size(void) {
  return sizeof(co_mutex_t);
}

int co_mutex_create(co_t* co, co_mutex_t* mutex) {
  mutex->lock = false;
  mutex->wait_link = NULL;
  mutex->wait_link_size = 0;
  mutex->co = co;

  return 0;
}

bool co_mutex_try_lock(co_mutex_t* mutex) {
  if (mutex->lock) {
    return false;
  }
  
  mutex->lock = true;

  return true;
}

static void co_mutex_suspend_thread_cb(co_thread_t* current, void* data) {
  co_mutex_t* mutex = (co_mutex_t*)data;
  
  if (NULL == mutex->wait_link) {
    co_list_init(current, &mutex->wait_link_size);
    mutex->wait_link = current;
  } else {
    co_list_shift(mutex->wait_link, current);
  }
}

void co_mutex_lock(co_mutex_t* mutex) {
  co_thread_t* current;

  if (co_mutex_try_lock(mutex)){
    return;
  }

  co_thread_suspend(mutex->co, co_mutex_suspend_thread_cb, mutex);

  assert(!mutex->lock);

  mutex->lock = true;  

  assert(mutex->wait_link == current);
}

void co_mutex_unlock(co_mutex_t* mutex) {
  co_thread_t* unlock_thread = mutex->wait_link;
  
  mutex->lock = false;

  if (!unlock_thread) {
    return;
  }

  /* remove from lock list */
  co_list_remove(mutex->wait_link, unlock_thread);
  /* recover thread to schedule loop list */
  co_thread_resume(unlock_thread);
}

void co_mutex_destroy(co_mutex_t* mutex) {
  assert(!mutex->wait_link);
  assert(!mutex->lock);
}