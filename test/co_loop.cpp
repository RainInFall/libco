#include <gtest/gtest.h>
#include <co_loop.h>
#include <co_thread.h>

const size_t CO_SIZE = co_size();

static co_t* co_new() {
  co_t* co = (co_t*)malloc(CO_SIZE);
  co_init(co);

  return co;
}

static void co_delete(co_t* co) {
  co_deinit(co);
  free(co);
}

TEST(co_loop_init, success) {
  co_t* co = co_new();

  const size_t LOOP_SIZE = co_loop_size();
  co_loop_t* loop = (co_loop_t*)malloc(LOOP_SIZE);
  
  ASSERT_EQ(0, co_loop_init(loop, co));
  ASSERT_EQ(0, co_loop_deinit(loop));

  free(loop);

  co_delete(co);
}