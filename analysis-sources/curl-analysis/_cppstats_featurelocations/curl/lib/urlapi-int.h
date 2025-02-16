#if !defined(HEADER_CURL_URLAPI_INT_H)
#define HEADER_CURL_URLAPI_INT_H





















#include "curl_setup.h"

#define MAX_SCHEME_LEN 40

bool Curl_is_absolute_url(const char *url, char *scheme, size_t buflen);

#if defined(DEBUGBUILD)
CURLUcode Curl_parse_port(struct Curl_URL *u, char *hostname);
#endif

#endif 
