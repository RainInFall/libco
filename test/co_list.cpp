#include <gtest/gtest.h>
#include <co_list.h>

typedef struct node_t node_t;

struct node_t {
  node_t* link[2];
  size_t* link_size;
};

TEST(co_list_init, success) {
  node_t node;
  size_t size = 0;
  co_list_init(&node, &size);
  ASSERT_EQ(co_list_prev(&node), &node);
  ASSERT_EQ(co_list_next(&node), &node);
  ASSERT_EQ(1, size);
}

TEST(co_list_push, success) {
  node_t node1, node2;
  size_t size = 0;
  co_list_init(&node1, &size);
  //co_list_init(&node2);
  co_list_push(&node1, &node2);

  ASSERT_EQ(co_list_next(&node1), &node2);
  ASSERT_EQ(co_list_prev(&node1), &node2);
  ASSERT_EQ(co_list_next(&node2), &node1);
  ASSERT_EQ(co_list_prev(&node2), &node1);
  ASSERT_EQ(2, size);
}

TEST(co_list_shift, success) {
  node_t node1, node2;
  size_t size = 0;
  //co_list_init(&node1);
  co_list_init(&node2, &size);
  co_list_shift(&node2, &node1);

  ASSERT_EQ(co_list_next(&node1), &node2);
  ASSERT_EQ(co_list_prev(&node1), &node2);
  ASSERT_EQ(co_list_next(&node2), &node1);
  ASSERT_EQ(co_list_prev(&node2), &node1);
  ASSERT_EQ(2, size);
}

TEST(co_list_remove, success) {
  node_t node1, node2, *head;
  size_t size = 0;
  head = &node2;
  //co_list_init(&node1);
  co_list_init(head, &size);
  co_list_shift(head, &node1);
  co_list_remove(head, &node2);

  ASSERT_EQ(co_list_prev(&node1), (&node1));
  ASSERT_EQ(co_list_next(&node1), (&node1));
  ASSERT_EQ(1, size);
}

TEST(co_list_remove, one_node_list) {
  node_t node, *head;
  size_t size = 0;
  head = &node;
  co_list_init(head, &size);
  co_list_remove(head, &node);
  ASSERT_EQ(0, size);
}