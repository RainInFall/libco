#include <gtest/gtest.h>
#include <co.h>

class CoInit: public ::testing::Environment {
public:
  virtual void SetUp() {
    ASSERT_EQ(0, co_init());
  }
};

static ::testing::Environment* dummy = ::testing::AddGlobalTestEnvironment(new CoInit());
