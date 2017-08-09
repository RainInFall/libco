#include <assert.h>
#include <gtest/gtest.h>
#include <co_tcp.h>
#include <co_loop.h>
#include <co_ip4.h>
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

const size_t TCP_SIZE = co_tcp_size();

static co_tcp_t* co_tcp_new(co_loop_t* loop) {
  co_tcp_t* tcp = (co_tcp_t*)malloc(TCP_SIZE);
  assert(0 == co_tcp_init(loop, tcp));

  return tcp;
}

static void co_tcp_delete(co_tcp_t* tcp) {
  co_tcp_close(tcp);
  free(tcp);
}

static uint16_t TEST_PORT = 10086;

const size_t STACK_SIZE = 56 * 1024;
const size_t CO_THREAED_SIZE = co_thread_size(STACK_SIZE);


static co_thread_t* co_thread_new(co_t* co, void* data = NULL) {
  co_thread_t* thread = (co_thread_t*)malloc(CO_THREAED_SIZE);
  co_thread_init(co, thread, data);

  return thread;
}

static void co_thread_delete(co_thread_t* thread) {
  free(thread);
}

TEST(co_tcp_init, success) {
  co_t* co = co_new();
  co_loop_t* loop = co_loop_new(co);
  
  co_tcp_t* tcp = (co_tcp_t*)malloc(TCP_SIZE);

  ASSERT_EQ(0, co_tcp_init(loop, tcp));
  co_tcp_close(tcp);

  co_loop_delete(loop);
  co_delete(co);
}

TEST(co_tcp_listen, success) {
  co_t* co = co_new();
  co_loop_t* loop = co_loop_new(co);

  co_tcp_t* server = co_tcp_new(loop);
  co_tcp_t* server_reused = co_tcp_new(loop);

  struct sockaddr_in addr;

  ASSERT_EQ(0, co_ip4_addr("0.0.0.0", TEST_PORT, &addr));
  
  ASSERT_EQ(0, co_tcp_bind(server, (const struct sockaddr*) &addr));
  ASSERT_EQ(0, co_tcp_listen(server, 128));

  ASSERT_EQ(0, co_tcp_bind(server_reused, (const struct sockaddr*) &addr));
  ASSERT_NE(0, co_tcp_listen(server_reused, 128));

  co_tcp_delete(server_reused);
  co_tcp_delete(server);

  co_loop_delete(loop);
  co_delete(co);
}

static void test_tcp_connect(void* data) {
  co_tcp_t* server = (co_tcp_t*)data;
  co_tcp_t* client = co_tcp_new(co_tcp_get_loop(server));

  assert(0 == co_tcp_accept(server, client));

  co_tcp_close(client);
  co_tcp_delete(client);
}

TEST(co_tcp_connect, success) {
  co_t* co = co_new();
  co_loop_t* loop = co_loop_new(co);

  co_tcp_t* client = co_tcp_new(loop);
  co_tcp_t* server = co_tcp_new(loop);

  struct sockaddr_in addr;

  ASSERT_EQ(0, co_ip4_addr("127.0.0.1", TEST_PORT, &addr));

  ASSERT_EQ(0, co_tcp_bind(server, (const struct sockaddr*) &addr));
  ASSERT_EQ(0, co_tcp_listen(server, 128));

  co_thread_t* thread = co_thread_new(co, server);
  ASSERT_EQ(0, co_thread_create(thread, STACK_SIZE, test_tcp_connect));

  ASSERT_EQ(0, co_tcp_connect(client, (const struct sockaddr *) &addr));

  co_thread_join(thread);
  co_thread_delete(thread); 

  co_tcp_delete(server);
  co_tcp_delete(client);

  co_loop_delete(loop);
  co_delete(co);
}
