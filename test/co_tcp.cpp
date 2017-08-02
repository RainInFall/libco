#include <gtest/gtest.h>
#include <co_tcp.h>
#include <co_loop.h>

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

const size_t LOOP_SIZE = co_loop_size();

static co_loop_t* co_loop_new(co_t* co) {
  co_loop_t* loop = (co_loop_t*)malloc(LOOP_SIZE);
  
  co_loop_init(loop, co);

  return loop;
}

static void co_loop_delete(co_loop_t* loop) {
  co_loop_deinit(loop);
  free(loop);
}

TEST(co_tcp_init, success) {
  co_t* co = co_new();
  co_loop_t* loop = co_loop_new(co);
  
  const size_t TCP_SIZE = co_tcp_size();
  co_tcp_t* tcp = (co_tcp_t*)malloc(TCP_SIZE);

  ASSERT_EQ(0, co_tcp_init(loop, tcp));
  co_tcp_close(tcp);

  co_loop_delete(loop);
  co_delete(co);
}