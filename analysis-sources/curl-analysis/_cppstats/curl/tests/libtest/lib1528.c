#include "test.h"
#include "memdebug.h"
int test(char *URL)
{
CURL *curl = NULL;
CURLcode res = CURLE_FAILED_INIT;
struct curl_slist *hhl = NULL;
struct curl_slist *phl = NULL;
if(curl_global_init(CURL_GLOBAL_ALL) != CURLE_OK) {
fprintf(stderr, "curl_global_init() failed\n");
return TEST_ERR_MAJOR_BAD;
}
curl = curl_easy_init();
if(!curl) {
fprintf(stderr, "curl_easy_init() failed\n");
curl_global_cleanup();
return TEST_ERR_MAJOR_BAD;
}
hhl = curl_slist_append(hhl, "User-Agent: Http Agent");
phl = curl_slist_append(phl, "Proxy-User-Agent: Http Agent2");
if(!hhl) {
goto test_cleanup;
}
test_setopt(curl, CURLOPT_URL, URL);
test_setopt(curl, CURLOPT_PROXY, libtest_arg2);
test_setopt(curl, CURLOPT_HTTPHEADER, hhl);
test_setopt(curl, CURLOPT_PROXYHEADER, phl);
test_setopt(curl, CURLOPT_HEADEROPT, CURLHEADER_SEPARATE);
test_setopt(curl, CURLOPT_VERBOSE, 1L);
test_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_HTTP);
test_setopt(curl, CURLOPT_HEADER, 1L);
res = curl_easy_perform(curl);
test_cleanup:
curl_easy_cleanup(curl);
curl_slist_free_all(hhl);
curl_slist_free_all(phl);
curl_global_cleanup();
return (int)res;
}
