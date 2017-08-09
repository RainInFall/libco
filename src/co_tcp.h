#ifndef __CO_TCP_H__
#define __CO_TCP_H__

#include <stddef.h>
#include "co_loop.h"
#include "co_ip4.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct co_tcp_t co_tcp_t;

size_t co_tcp_size(void);

int co_tcp_init(co_loop_t* loop, co_tcp_t* tcp);

void co_tcp_close(co_tcp_t* tcp);

int co_tcp_bind(co_tcp_t* tcp, const struct sockaddr* addr);

int co_tcp_listen(co_tcp_t* tcp, int backlog);

int co_tcp_accept(co_tcp_t* server, co_tcp_t* client);

int co_tcp_connect(co_tcp_t* client, const struct sockaddr* addr);

int co_tcp_write(co_tcp_t* tcp, const void* data, size_t len);

ssize_t co_tcp_read(co_tcp_t* tcp, void* buf, size_t len);

co_loop_t* co_tcp_get_loop(co_tcp_t* tcp);

#ifdef __cplusplus
}
#endif

#endif /* __CO_TCP_H__ */