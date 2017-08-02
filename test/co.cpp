#include <cstdlib>
#include <gtest/gtest.h>
#include <co.h>

const size_t CO_SIZE = co_size();

TEST(co_init, success) {
  co_t* co = (co_t*)malloc(CO_SIZE);
  ASSERT_EQ(0, co_init(co));
  ASSERT_EQ(0, co_deinit(co));
  free(co);
  co = NULL;
}