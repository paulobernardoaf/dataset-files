
















#pragma once

#include <util/darray.h>

#if defined(_WIN32)
#include <ws2tcpip.h>
#include <winsock2.h>
#include <ws2ipdef.h>
#include <iphlpapi.h>
#else

#if defined(__linux__)
#include <linux/if_link.h>
#elif __FreeBSD__
#include <netinet/in.h>
#if !defined(_GNU_SOURCE)
#define _GNU_SOURCE
#define __NET_IF_GNU_SOURCE__
#endif 
#endif 

#include <ifaddrs.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#if defined(__FreeBSD__)
#if defined(___NET_IF_GNU_SOURCE__)
#undef ___NET_IF_GNU_SOURCE__
#undef _GNU_SOURCE
#endif
#endif

#endif

struct netif_saddr_item {
char *name;
char *addr;
};

struct netif_saddr_data {
DARRAY(struct netif_saddr_item) addrs;
};

static inline void netif_saddr_data_free(struct netif_saddr_data *data)
{
for (size_t i = 0; i < data->addrs.num; i++) {
bfree(data->addrs.array[i].name);
bfree(data->addrs.array[i].addr);
}
da_free(data->addrs);
}

extern bool netif_str_to_addr(struct sockaddr_storage *out, int *addr_len,
const char *addr);
extern void netif_get_addrs(struct netif_saddr_data *ifaddrs);
extern void netif_log_saddrs(struct netif_saddr_data *sd);
