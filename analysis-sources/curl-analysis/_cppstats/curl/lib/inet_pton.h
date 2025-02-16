#include "curl_setup.h"
int Curl_inet_pton(int, const char *, void *);
#if defined(HAVE_INET_PTON)
#if defined(HAVE_ARPA_INET_H)
#include <arpa/inet.h>
#elif defined(HAVE_WS2TCPIP_H)
#include <ws2tcpip.h>
#endif
#define Curl_inet_pton(x,y,z) inet_pton(x,y,z)
#endif
