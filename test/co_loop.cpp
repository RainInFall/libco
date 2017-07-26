#include <cstdlib>
#include <cstddef>
#include <gtest/gtest.h>
#include "co.h"

TEST(co_loop_init, success) {
  size_t loop_size = co_loop_size();
  co_loop_t* loop = (co_loop_t*)malloc(loop_size);
  ASSERT_EQ(0, co_loop_init(loop));

  free(loop);
}
