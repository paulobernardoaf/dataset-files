#include <curl/curl.h>
#define CURL_HOST_NOMATCH 0
#define CURL_HOST_MATCH 1
int Curl_cert_hostcheck(const char *match_pattern, const char *hostname);
