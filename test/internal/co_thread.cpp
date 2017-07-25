#include <gtest/gtest.h>
#include "internal/co_thread.h"

TEST(_get_thread_from_ucontext, correct) {
  co_thread_t thread;
  ASSERT_EQ(&thread, _get_thread_from_ucontext(&thread.handle));
}
