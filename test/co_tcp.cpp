#include <arpa/inet.h>
#include <cstdlib>
#include <cstddef>
#include <iostream>
#include <gtest/gtest.h>
#include "co.h"

static const size_t STACK_SIZE = 64 * 1024;
static const uint16_t LOCAL_PORT = htons(32222);

static void test_tcp_init(void* data) {
  co_loop_t* loop = (co_loop_t*)data;

  size_t tcp_size = co_tcp_size();
  co_tcp_t* tcp = (co_tcp_t*)malloc(tcp_size);
  ASSERT_EQ(0, co_tcp_init(loop, tcp));
  co_tcp_close(tcp);
  free(tcp);
}

TEST(co_tcp_init, success) {
  size_t loop_size = co_loop_size();
  co_loop_t* loop = (co_loop_t*)malloc(loop_size);
  ASSERT_EQ(0, co_loop_init(loop));

  /* Create thread */
  size_t thread_size = co_thread_size(STACK_SIZE);
  co_thread_t* thread = (co_thread_t*)malloc(thread_size);
  ASSERT_EQ(0, co_thread_create(thread, STACK_SIZE, test_tcp_init, loop));

  ASSERT_EQ(0, co_loop_run(loop));

  /* Destroy thread */
  co_thread_join(thread);
  free(thread);

  ASSERT_EQ(0, co_loop_close(loop));
  free(loop);
}

static void test_tcp_listen(void* data) {
  co_loop_t* loop = (co_loop_t*)data;

  size_t tcp_size = co_tcp_size();
  co_tcp_t* tcp = (co_tcp_t*)malloc(tcp_size);
  ASSERT_EQ(0, co_tcp_init(loop, tcp));

  struct sockaddr_in local;
  local.sin_family = AF_INET;
  local.sin_port = LOCAL_PORT;
  ASSERT_TRUE(inet_aton("0.0.0.0", &local.sin_addr));
  ASSERT_EQ(0, co_tcp_bind(tcp, (struct sockaddr*)&local));

  co_tcp_close(tcp);
  free(tcp);
}

TEST(co_tcp_listen, success) {
  size_t loop_size = co_loop_size();
  co_loop_t* loop = (co_loop_t*)malloc(loop_size);
  ASSERT_EQ(0, co_loop_init(loop));

  size_t thread_size = co_thread_size(STACK_SIZE);
  co_thread_t* thread = (co_thread_t*)malloc(thread_size);
  ASSERT_EQ(0, co_thread_create(thread, STACK_SIZE, test_tcp_listen, loop));

  ASSERT_EQ(0, co_loop_run(loop));

  co_thread_join(thread);
  free(thread);

  ASSERT_EQ(0, co_loop_close(loop));
  free(loop);
}


typedef struct {
  co_mutex_t* lock;
  co_loop_t* loop;
} test_connect_context_t;

static void test_tcp_connect_listen(void* data) {
  test_connect_context_t* ctx = (test_connect_context_t*)data;

  size_t tcp_size = co_tcp_size();
  co_tcp_t* tcp = (co_tcp_t*)malloc(tcp_size);
  ASSERT_EQ(0, co_tcp_init(ctx->loop, tcp));

  struct sockaddr_in local;
  local.sin_family = AF_INET;
  local.sin_port = LOCAL_PORT;
  ASSERT_TRUE(inet_aton("0.0.0.0", &local.sin_addr));
  ASSERT_EQ(0, co_tcp_bind(tcp, (struct sockaddr*)&local));
  std::cout<<3<<std::endl;
  ASSERT_EQ(0, co_tcp_listen(tcp, 128));
  std::cout<<5<<std::endl;
  co_mutex_lock(ctx->lock);
  std::cout<<6<<std::endl;
  co_tcp_close(tcp);
  free(tcp);
  std::cout<<7<<std::endl;
  co_mutex_unlock(ctx->lock);
  std::cout<<8<<std::endl;
}

static void test_tcp_connect(void* data) {
  test_connect_context_t* ctx = (test_connect_context_t*)data;

  size_t tcp_size = co_tcp_size();
  co_tcp_t* tcp = (co_tcp_t*)malloc(tcp_size);
  ASSERT_EQ(0, co_tcp_init(ctx->loop, tcp));

  struct sockaddr_in local;
  local.sin_family = AF_INET;
  local.sin_port = LOCAL_PORT;
  ASSERT_TRUE(inet_aton("127.0.0.1", &local.sin_addr));
  std::cout<<4<<std::endl;
  ASSERT_EQ(0, co_tcp_connect(tcp, (struct sockaddr*)&local));
  std::cout<<9<<std::endl;
  co_mutex_unlock(ctx->lock);
  std::cout<<10<<std::endl;
  co_tcp_close(tcp);
  free(tcp);
  std::cout<<11<<std::endl;
}

TEST(co_tcp_connect, success) {
  size_t loop_size = co_loop_size();
  co_loop_t* loop = (co_loop_t*)malloc(loop_size);
  ASSERT_EQ(0, co_loop_init(loop));

  co_mutex_t* lock = (co_mutex_t*)malloc(co_mutex_size());
  ASSERT_EQ(0, co_mutex_create(lock));

  test_connect_context_t ctx;
  ctx.loop = loop;
  ctx.lock = lock;

  co_mutex_lock(lock);

  size_t thread_size = co_thread_size(STACK_SIZE);
  co_thread_t* thread_listen = (co_thread_t*)malloc(thread_size);
  ASSERT_EQ(0, co_thread_create(thread_listen,
                                STACK_SIZE,
                                test_tcp_connect_listen,
                                &ctx));
                      std::cout<<1<<std::endl;
  co_thread_t* thread_connect = (co_thread_t*)malloc(thread_size);
  ASSERT_EQ(0, co_thread_create(thread_connect,
                                STACK_SIZE,
                                test_tcp_connect,
                                &ctx));
                      std::cout<<2<<std::endl;
  ASSERT_EQ(0, co_loop_run(loop));
  std::cout<<13<<std::endl;
  co_thread_join(thread_connect);
  free(thread_connect);
  thread_connect = NULL;
  std::cout<<14<<std::endl;
  co_thread_join(thread_listen);
  free(thread_listen);
  thread_listen = NULL;
  std::cout<<15<<std::endl;
  ASSERT_EQ(0, co_loop_close(loop));
  free(loop);
  loop = NULL;
  std::cout<<16<<std::endl;
  co_mutex_destroy(lock);
  free(lock);
  lock = NULL;
  std::cout<<17<<std::endl;
}
