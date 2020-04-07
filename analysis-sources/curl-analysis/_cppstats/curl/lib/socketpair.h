#include "curl_setup.h"
#if !defined(HAVE_SOCKETPAIR)
int Curl_socketpair(int domain, int type, int protocol,
curl_socket_t socks[2]);
#else
#define Curl_socketpair(a,b,c,d) socketpair(a,b,c,d)
#endif
#define USE_SOCKETPAIR 1
