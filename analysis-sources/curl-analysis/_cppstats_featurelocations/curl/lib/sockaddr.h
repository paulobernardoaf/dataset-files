#if !defined(HEADER_CURL_SOCKADDR_H)
#define HEADER_CURL_SOCKADDR_H






















#include "curl_setup.h"

struct Curl_sockaddr_storage {
union {
struct sockaddr sa;
struct sockaddr_in sa_in;
#if defined(ENABLE_IPV6)
struct sockaddr_in6 sa_in6;
#endif
#if defined(HAVE_STRUCT_SOCKADDR_STORAGE)
struct sockaddr_storage sa_stor;
#else
char cbuf[256]; 
#endif
} buffer;
};

#endif 
