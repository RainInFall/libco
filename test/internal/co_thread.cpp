#include <gtest/gtest.h>
#include "internal/co_thread.h"

static size_t STACK_SIZE = 64 * 1024;

TEST(_get_thread_from_ucontext, correct) {
  co_thread_t thread;
  ASSERT_EQ(&thread, _get_thread_from_ucontext(&thread.handle));
}

static void check_thread_switch(void* data) {
  std::pair<int, co_thread_t*>* pair = (std::pair<int, co_thread_t*>*)data;
  pair->first += 0x1;
  _co_thread_switch(pair->second);
  ASSERT_EQ(pair->first, 0x11);
  pair->first += 0x100;
}

TEST(_co_thread_switch, success) {
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
  _co_thread_switch(thread);
  ASSERT_EQ(pair.first, 0x111);

  free(thread);
}
