#include <gtest/gtest.h>
#include "internal/co_loop.h"

TEST(_uv_loop_get_co_loop, correct) {
  co_loop_t loop;
  ASSERT_EQ(&loop, _uv_loop_get_co_loop(&loop.handle));
}
