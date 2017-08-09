#include <uv.h>
#include "co_ip4.h"

int co_ip4_addr(const char* ip, uint16_t port, struct sockaddr_in* addr) {
  return uv_ip4_addr(ip, port, addr);
}