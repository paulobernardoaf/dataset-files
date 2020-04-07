#include "test.h"
#include "memdebug.h"
#include <curl/multi.h>
int test(char *URL)
{
CURLM *handle;
int res = CURLE_OK;
static const char * const bl_servers[] =
{"Microsoft-IIS/6.0", "nginx/0.8.54", NULL};
static const char * const bl_sites[] =
{"curl.haxx.se:443", "example.com:80", NULL};
global_init(CURL_GLOBAL_ALL);
handle = curl_multi_init();
(void)URL; 
curl_multi_setopt(handle, CURLMOPT_PIPELINING_SERVER_BL, bl_servers);
curl_multi_setopt(handle, CURLMOPT_PIPELINING_SITE_BL, bl_sites);
curl_multi_cleanup(handle);
curl_global_cleanup();
return 0;
}
