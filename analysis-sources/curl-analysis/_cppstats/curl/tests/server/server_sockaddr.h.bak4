#if !defined(HEADER_CURL_SERVER_SOCKADDR_H)
#define HEADER_CURL_SERVER_SOCKADDR_H





















#include "server_setup.h"

#if defined(HAVE_SYS_UN_H)
#include <sys/un.h> 
#endif

typedef union {
struct sockaddr sa;
struct sockaddr_in sa4;
#if defined(ENABLE_IPV6)
struct sockaddr_in6 sa6;
#endif
#if defined(USE_UNIX_SOCKETS)
struct sockaddr_un sau;
#endif
} srvr_sockaddr_union_t;

#endif 
