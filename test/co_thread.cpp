#include <cstdlib>
#include <gtest/gtest.h>
#include <co_thread.h>

const size_t CO_SIZE = co_size();
const size_t STACK_SIZE = 56 * 1024;
const size_t CO_THREAED_SIZE = co_thread_size(STACK_SIZE);

static co_t* co_new() {
  co_t* co = (co_t*)malloc(CO_SIZE);
  co_init(co);

  return co;
}

static void co_delete(co_t* co) {
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

TEST(co_thread_init, success) {
  co_t* co = co_new();
  
  co_thread_t* thread = (co_thread_t*)malloc(CO_THREAED_SIZE);
  ASSERT_EQ(0, co_thread_init(co, thread, NULL));

  co_thread_delete(thread);

  co_delete(co);
}

static void test_thread_create(void* data) {
  int* call = (int*)data;
  *call += 1;
}


TEST(co_thread_create_and_join, success) {
  co_t* co = co_new();
  int call = 0;
  co_thread_t* thread = co_thread_new(co, &call);

  ASSERT_EQ(0, co_thread_create(thread, STACK_SIZE, test_thread_create));

  co_thread_join(thread);

  ASSERT_EQ(1, call);

  co_thread_delete(thread);

  co_delete(co);
}

TEST(co_thread_create_and_join, many) {
  co_t* co = co_new();
  int call = 0;
  co_thread_t* threads[100];
  
  for (int i =0; i < 100; ++i) {
    threads[i] = co_thread_new(co, &call);
    ASSERT_EQ(0, co_thread_create(threads[i], STACK_SIZE, test_thread_create));
  }

  for (int i = 0; i < 100; ++i) {
    co_thread_join(threads[i]);
  }

  ASSERT_EQ(100, call);

  for (int i = 0; i < 100; ++i) {
    co_thread_delete(threads[i]);
  }

  co_delete(co);
}