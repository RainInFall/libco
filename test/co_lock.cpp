#include <cstdlib>
#include <cstddef>
#include <utility>
#include <gtest/gtest.h>
#include <co.h>

static size_t STACK_SIZE = 64 * 1024;

TEST(co_mutex_create, success) {
  size_t mutex_size = co_mutex_size();
  co_mutex_t* mutex = (co_mutex_t*)malloc(mutex_size);
  ASSERT_NE((co_mutex_t*)NULL, mutex);
  ASSERT_EQ(0, co_mutex_create(mutex));
  co_mutex_destroy(mutex);
  free(mutex);
}

TEST(co_mutex_try_lock, success) {
  size_t mutex_size = co_mutex_size();
  co_mutex_t* mutex = (co_mutex_t*)malloc(mutex_size);
  ASSERT_NE((co_mutex_t*)NULL, mutex);
  ASSERT_EQ(0, co_mutex_create(mutex));

  ASSERT_TRUE(co_mutex_try_lock(mutex));

  co_mutex_unlock(mutex);

  co_mutex_destroy(mutex);
  free(mutex);
}

static void try_lock_function(void* data) {
  co_mutex_t* mutex = (co_mutex_t*)data;
  ASSERT_FALSE(co_mutex_try_lock(mutex));
}

TEST(co_mutex_lock, locked) {
  size_t mutex_size = co_mutex_size();
  co_mutex_t* mutex = (co_mutex_t*)malloc(mutex_size);
  ASSERT_NE((co_mutex_t*)NULL, mutex);
  ASSERT_EQ(0, co_mutex_create(mutex));

  co_mutex_lock(mutex);

  size_t thread_size = co_thread_size(STACK_SIZE);

  co_thread_t* thread = (co_thread_t*)malloc(thread_size);
  ASSERT_NE((co_thread_t*)NULL, thread);

  ASSERT_EQ(0, co_thread_create(thread,
                                STACK_SIZE,
                                try_lock_function,
                                mutex));
  co_thread_join(thread);
  free(thread);

  co_mutex_unlock(mutex);

  co_mutex_destroy(mutex);
  free(mutex);
}

static void wait_lock_function(void* data) {
  std::pair<co_mutex_t*, int>* pair = (std::pair<co_mutex_t*, int>*)data;
  ASSERT_EQ(1, pair->second);
  co_mutex_lock(pair->first);
  ASSERT_EQ(2, pair->second);
}

TEST(co_mutex_unlock, success) {
  size_t mutex_size = co_mutex_size();
  co_mutex_t* mutex = (co_mutex_t*)malloc(mutex_size);
  ASSERT_NE((co_mutex_t*)NULL, mutex);
  ASSERT_EQ(0, co_mutex_create(mutex));

  co_mutex_lock(mutex);

  size_t thread_size = co_thread_size(STACK_SIZE);

  co_thread_t* thread = (co_thread_t*)malloc(thread_size);
  ASSERT_NE((co_thread_t*)NULL, thread);

  std::pair<co_mutex_t*, int> pair = std::make_pair(mutex, 0);

  pair.second = 1;

  ASSERT_EQ(0, co_thread_create(thread,
                                STACK_SIZE,
                                wait_lock_function,
                                &pair));

  pair.second = 2;
  co_mutex_unlock(mutex);


  co_thread_join(thread);
  free(thread);

  co_mutex_unlock(mutex);
  co_mutex_destroy(mutex);
  free(mutex);
}
