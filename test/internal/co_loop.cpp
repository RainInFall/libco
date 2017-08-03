#include <gtest/gtest.h>
#include "internal/co_loop.h"

TEST(co_loop_get_from_uv_loop, success) {
  co_loop_t loop;
  ASSERT_EQ(&loop, co_loop_get_from_uv_loop(&loop.handle));
}