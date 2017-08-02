#include <gtest/gtest.h>
#include <co_lock.h>
#include <co_thread.h>

const size_t MUTEX_SIZE = co_mutex_size();
const size_t CO_SIZE = co_size();
const size_t STACK_SIZE = 56 * 1024;
const size_t CO_THREAED_SIZE = co_thread_size(STACK_SIZE);

static co_t* co_new() {
  co_t* co = (co_t*)malloc(CO_SIZE);
  co_init(co);

  return co;
}

static void co_delete(co_t* co) {
  co_deinit(co);
  free(co);
}

static co_thread_t* co_thread_new(co_t* co, void* data = NULL) {
  co_thread_t* thread = (co_thread_t*)malloc(CO_THREAED_SIZE);
  co_thread_init(co, thread, data);

  return thread;
}

static void co_thread_delete(co_thread_t* thread) {
  free(thread);
}

TEST(co_mutex_create, success) {
  co_t* co = co_new();
  
  co_mutex_t* mutex = (co_mutex_t*)malloc(MUTEX_SIZE);
  ASSERT_EQ(0, co_mutex_create(co, mutex));
  
  co_mutex_destroy(mutex);

  co_delete(co);
}

TEST(co_mutex_lock, success) {
  co_t* co = co_new();

  co_mutex_t* mutex = (co_mutex_t*)malloc(MUTEX_SIZE);
  ASSERT_EQ(0, co_mutex_create(co, mutex));
  
  ASSERT_TRUE(co_mutex_try_lock(mutex));
  co_mutex_unlock(mutex);

  co_mutex_lock(mutex);
  ASSERT_FALSE(co_mutex_try_lock(mutex));
  co_mutex_unlock(mutex);

  co_mutex_destroy(mutex);
  free(mutex);

  co_delete(co);
}

static int test_mutex_lock_multi_thread_flag = 0;

static void test_mutex_lock_multi_thread(co_mutex_t* mutex) {
  co_mutex_lock(mutex);
  ASSERT_EQ(1, test_mutex_lock_multi_thread_flag);
  test_mutex_lock_multi_thread_flag = 2;
  co_mutex_unlock(mutex);
}

TEST(co_mutex_lock, multl_thread) {
  co_t* co = co_new();

  co_mutex_t* mutex = (co_mutex_t*)malloc(MUTEX_SIZE);
  ASSERT_EQ(0, co_mutex_create(co, mutex));

  co_mutex_lock(mutex);

  test_mutex_lock_multi_thread_flag = 1;

  co_thread_t* thread = co_thread_new(co, mutex);
  co_thread_create(thread, STACK_SIZE, (void(*)(void*))test_mutex_lock_multi_thread);
 
  ASSERT_EQ(1, test_mutex_lock_multi_thread_flag);

  co_mutex_unlock(mutex);

  co_thread_join(thread);

  ASSERT_EQ(2, test_mutex_lock_multi_thread_flag);

  co_mutex_destroy(mutex);
  free(mutex);

  co_thread_delete(thread);

  co_delete(co);
}

static void test_mutex_lock_join_suspend(co_mutex_t* mutex) {
  co_mutex_lock(mutex);
  ASSERT_EQ(1, test_mutex_lock_multi_thread_flag);
  test_mutex_lock_multi_thread_flag = 2;
  co_mutex_unlock(mutex);
}

TEST(co_mutex_lock, join_suspend) {
  co_t* co = co_new();

  co_mutex_t* mutex = (co_mutex_t*)malloc(MUTEX_SIZE);
  ASSERT_EQ(0, co_mutex_create(co, mutex));

  co_mutex_lock(mutex);

  test_mutex_lock_multi_thread_flag = 1;

  co_thread_t* thread = co_thread_new(co, mutex);
  co_thread_create(thread, STACK_SIZE, (void(*)(void*))test_mutex_lock_join_suspend);
 
  ASSERT_EQ(1, test_mutex_lock_multi_thread_flag);

  co_mutex_unlock(mutex);

  co_thread_join(thread);

  ASSERT_EQ(2, test_mutex_lock_multi_thread_flag);

  co_mutex_destroy(mutex);
  free(mutex);

  co_thread_delete(thread);

  co_delete(co);
}