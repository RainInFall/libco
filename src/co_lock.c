#include <assert.h>
#include <stdbool.h>
#include "co_lock.h"
#include "co_list.h"
#include "internal/co_thread.h"
#include "internal/co.h"

struct co_mutex_t{
  bool lock;
  co_thread_t* link;
  size_t link_size;
  co_t* co;
};

size_t co_mutex_size(void) {
  return sizeof(co_mutex_t);
}

int co_mutex_create(co_t* co, co_mutex_t* mutex) {
  mutex->lock = false;
  mutex->link = NULL;
  mutex->link_size = 0;
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

void co_mutex_lock(co_mutex_t* mutex) {
  co_thread_t* current;

  if (co_mutex_try_lock(mutex)){
    return;
  }

  current = co_thread_current(mutex->co);
  /* current next still remain valid, so it is not harmful to remove current thread from loop*/
  co_remove_thread(current->co, current);

  if (NULL == mutex->link) {
    co_list_init(current, &mutex->link_size);
    mutex->link = current;
  } else {
    co_list_shift(mutex->link, current);
  }

  current->status = CO_THREAD_STATUS_SUSPEND;
  /* FIXME: the infinite loop of thread yield when none thread is running */
  co_thread_yield(mutex->co);
    
  assert(current->status != CO_THREAD_STATUS_FINISH);
  assert(!mutex->lock);

  mutex->lock = true;  

  assert(mutex->link == current);
}

void co_mutex_unlock(co_mutex_t* mutex) {
  co_thread_t* unlock_thread = mutex->link;
  
  mutex->lock = false;

  if (!unlock_thread) {
    return;
  }

  unlock_thread->status = CO_THREAD_STATUS_RUNNING;
  co_thread_switch(unlock_thread);
}

void co_mutex_destroy(co_mutex_t* mutex) {
  assert(!mutex->link);
}