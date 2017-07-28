#ifndef __CO_TCP_H__
#define __CO_TCP_H__

#include <stddef.h>
#include <netinet/in.h>
#include "co_loop.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct co_tcp_t co_tcp_t;

size_t co_tcp_size(void);

int co_tcp_init(co_loop_t* loop, co_tcp_t* tcp);

int co_tcp_connect(co_tcp_t* tcp, const struct sockaddr* addr);

int co_tcp_bind(co_tcp_t* tcp, const struct sockaddr* addr);

int co_tcp_listen(co_tcp_t* tcp, int backlog);

int co_tcp_accept(co_tcp_t* server, co_tcp_t* client);

ssize_t co_tcp_read(co_tcp_t* tcp, void* buf, size_t size);

int co_tcp_shutdown(co_tcp_t* tcp);

void co_tcp_close(co_tcp_t* tcp);

#ifdef __cplusplus
}
#endif

#endif /* __CO_TCP_H__ */
