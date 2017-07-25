#include <cstdlib>
#include <cstddef>
#include <utility>
#include <gtest/gtest.h>
#include <co.h>

static size_t STACK_SIZE = 64 * 1024;

TEST(co_thread_size, success) {
  size_t size = co_thread_size(STACK_SIZE);
  ASSERT_GE(size, STACK_SIZE);
}

static void empty_function(void* data) {
  ASSERT_EQ(data, &STACK_SIZE);
}

TEST(co_thread_create, success) {
  size_t size = co_thread_size(STACK_SIZE);

  co_thread_t* thread = (co_thread_t*)malloc(size);
  ASSERT_NE((co_thread_t*)NULL, thread);

  ASSERT_EQ(0, co_thread_create(thread,
                                STACK_SIZE,
                                empty_function,
                                &STACK_SIZE));

  free(thread);
}

static void set_after_call(void* data) {
  int* called = (int*)data;
  *called = 1;
}

TEST(co_thread_create, entry_called) {
  size_t size = co_thread_size(STACK_SIZE);

  co_thread_t* thread = (co_thread_t*)malloc(size);
  ASSERT_NE((co_thread_t*)NULL, thread);

  int called = 0;
  ASSERT_EQ(0, co_thread_create(thread,
                                STACK_SIZE,
                                set_after_call,
                                &called));
  ASSERT_EQ(1, called);

  free(thread);
}

static void check_thread_running(void* data) {
  co_thread_t* thread = (co_thread_t*)data;
  ASSERT_TRUE(co_thread_is_running(thread));
}

TEST(co_thread_is_running, success) {
  size_t size = co_thread_size(STACK_SIZE);

  co_thread_t* thread = (co_thread_t*)malloc(size);
  ASSERT_NE((co_thread_t*)NULL, thread);

  ASSERT_EQ(0, co_thread_create(thread,
                                STACK_SIZE,
                                check_thread_running,
                                thread));
  ASSERT_FALSE(co_thread_is_running(thread));

  free(thread);
}

static void check_thread_current(void* data) {
  co_thread_t* thread = (co_thread_t*)data;
  ASSERT_EQ(thread, co_thread_current());
}

TEST(co_thread_current, success) {
  size_t size = co_thread_size(STACK_SIZE);

  co_thread_t* thread = (co_thread_t*)malloc(size);
  ASSERT_NE((co_thread_t*)NULL, thread);

  co_thread_t* this_thread = co_thread_current();
  ASSERT_EQ(0, co_thread_create(thread,
                                STACK_SIZE,
                                check_thread_current,
                                thread));
  ASSERT_NE(thread, co_thread_current());
  ASSERT_EQ(this_thread, co_thread_current());

  free(thread);
}

static void check_thread_switch(void* data) {
  std::pair<int, co_thread_t*>* pair = (std::pair<int, co_thread_t*>*)data;
  pair->first += 0x1;
  co_thread_switch(pair->second);
  ASSERT_EQ(pair->first, 0x11);
  pair->first += 0x100;
}

TEST(co_thread_switch, success) {
  size_t size = co_thread_size(STACK_SIZE);

  co_thread_t* thread = (co_thread_t*)malloc(size);
  ASSERT_NE((co_thread_t*)NULL, thread);

  std::pair<int, co_thread_t*> pair = std::make_pair(0, co_thread_current());
  ASSERT_EQ(0, co_thread_create(thread,
                                STACK_SIZE,
                                check_thread_switch,
                                &pair));
  ASSERT_EQ(pair.first, 0x1);
  pair.first += 0x10;
  co_thread_switch(thread);
  ASSERT_EQ(pair.first, 0x111);

  free(thread);
}

static void check_thread_join(void* data) {
  std::pair<int, co_thread_t*>* pair = (std::pair<int, co_thread_t*>*)data;
  pair->first += 0x01;
  co_thread_switch(pair->second);
  pair->first = pair->first * 3 + 2; /* 1x3+2=5 */
  co_thread_switch(pair->second);
  pair->first = pair->first * 2 + 3; /* 5x2+3=13 */
  co_thread_switch(pair->second);
  pair->first = pair->first * 4 + 5; /* 13x4+5=57 */
}

TEST(co_thread_join, success) {
  size_t size = co_thread_size(STACK_SIZE);

  co_thread_t* thread = (co_thread_t*)malloc(size);
  ASSERT_NE((co_thread_t*)NULL, thread);

  std::pair<int, co_thread_t*> pair = std::make_pair(0, co_thread_current());
  ASSERT_EQ(0, co_thread_create(thread,
                                STACK_SIZE,
                                check_thread_join,
                                &pair));
  co_thread_join(thread);
  ASSERT_EQ(pair.first, 57);

  free(thread);
}

static void check_thread_yield(void* data) {
  int* called = (int*)data;
  *called = 1;
  co_thread_yield();
  *called = 2;
}

TEST(co_thread_yield, success) {
  size_t size = co_thread_size(STACK_SIZE);

  co_thread_t* thread = (co_thread_t*)malloc(size);
  ASSERT_NE((co_thread_t*)NULL, thread);

  int called = 0;
  ASSERT_EQ(0, co_thread_create(thread,
                                STACK_SIZE,
                                check_thread_yield,
                                &called));

  ASSERT_EQ(1, called);
  co_thread_join(thread);
  ASSERT_EQ(2, called);

  free(thread);
}
