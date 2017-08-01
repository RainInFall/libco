#ifndef __CO_LIST_H__
#define __CO_LIST_H__

#ifdef __cplusplus
extern "C" {
#endif

#define co_list_prev(entry) ((entry)->link[0])
#define co_list_next(entry) ((entry)->link[1])

#define co_list_init(entry, ptr_link_size) \
  do {\
    co_list_prev(entry) = (entry);\
    co_list_next(entry) = (entry);\
    (entry)->link_size = ptr_link_size;\
    *(entry)->link_size = 1;\
  } while(0)

#define co_list_push(entry, node) \
  do {\
    co_list_next(node) = co_list_next(entry);\
    co_list_prev(node) = (entry);\
    co_list_next(entry) = (node);\
    co_list_prev(co_list_next(node)) = (node);\
    *(entry)->link_size += 1;\
    (node)->link_size = (entry)->link_size;\
  }while(0)

#define co_list_shift(entry, node) \
  do {\
    co_list_prev(node) = co_list_prev(entry);\
    co_list_next(node) = (entry);\
    co_list_prev(entry) = (node);\
    co_list_next(co_list_prev(node)) = (node);\
    *(entry)->link_size += 1;\
    (node)->link_size = (entry)->link_size;\
  } while(0)

#define co_list_remove(node) \
    do {\
      co_list_next(co_list_prev(node)) = co_list_next(node);\
      co_list_prev(co_list_next(node)) = co_list_prev(node);\
      *(node)->link_size -= 1;\
    } while (0)

#ifdef __cplusplus
}
#endif

#endif /* __CO_LIST_H__ */