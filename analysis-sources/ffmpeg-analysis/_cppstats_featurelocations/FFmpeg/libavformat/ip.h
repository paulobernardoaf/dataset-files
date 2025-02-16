



















#if !defined(AVFORMAT_IP_H)
#define AVFORMAT_IP_H

#include "network.h"




typedef struct IPSourceFilters {
int nb_include_addrs;
int nb_exclude_addrs;
struct sockaddr_storage *include_addrs;
struct sockaddr_storage *exclude_addrs;
} IPSourceFilters;






int ff_ip_check_source_lists(struct sockaddr_storage *source_addr_ptr, IPSourceFilters *s);






struct addrinfo *ff_ip_resolve_host(void *log_ctx,
const char *hostname, int port,
int type, int family, int flags);






int ff_ip_parse_sources(void *log_ctx, const char *buf, IPSourceFilters *filters);






int ff_ip_parse_blocks(void *log_ctx, const char *buf, IPSourceFilters *filters);





void ff_ip_reset_filters(IPSourceFilters *filters);

#endif 
