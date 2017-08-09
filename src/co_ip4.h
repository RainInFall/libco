#ifndef __CO_IP_4_H__
#define __CO_IP_4_H__

#include <stdint.h>
#include <netinet/in.h>

#ifdef __cplusplus
extern "C" {
#endif

int co_ip4_addr(const char* ip, uint16_t port, struct sockaddr_in* addr);

#ifdef __cplusplus
}
#endif

#endif /* __CO_IP_4_H__ */