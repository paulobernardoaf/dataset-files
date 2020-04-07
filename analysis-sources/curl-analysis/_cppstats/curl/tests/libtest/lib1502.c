#include "test.h"
#include <limits.h>
#include "testutil.h"
#include "warnless.h"
#include "memdebug.h"
#define TEST_HANG_TIMEOUT 60 * 1000
int test(char *URL)
{
CURL *easy = NULL;
CURL *dup;
CURLM *multi = NULL;
int still_running;
int res = 0;
char redirect[160];
struct curl_slist *dns_cache_list;
msnprintf(redirect, sizeof(redirect), "google.com:%s:%s", libtest_arg2,
libtest_arg3);
start_test_timing();
dns_cache_list = curl_slist_append(NULL, redirect);
if(!dns_cache_list) {
fprintf(stderr, "curl_slist_append() failed\n");
return TEST_ERR_MAJOR_BAD;
}
res_global_init(CURL_GLOBAL_ALL);
if(res) {
curl_slist_free_all(dns_cache_list);
return res;
}
easy_init(easy);
easy_setopt(easy, CURLOPT_URL, URL);
easy_setopt(easy, CURLOPT_HEADER, 1L);
easy_setopt(easy, CURLOPT_RESOLVE, dns_cache_list);
dup = curl_easy_duphandle(easy);
if(dup) {
curl_easy_cleanup(easy);
easy = dup;
}
else {
curl_slist_free_all(dns_cache_list);
curl_easy_cleanup(easy);
return CURLE_OUT_OF_MEMORY;
}
multi_init(multi);
multi_add_handle(multi, easy);
multi_perform(multi, &still_running);
abort_on_test_timeout();
while(still_running) {
struct timeval timeout;
fd_set fdread;
fd_set fdwrite;
fd_set fdexcep;
int maxfd = -99;
FD_ZERO(&fdread);
FD_ZERO(&fdwrite);
FD_ZERO(&fdexcep);
timeout.tv_sec = 1;
timeout.tv_usec = 0;
multi_fdset(multi, &fdread, &fdwrite, &fdexcep, &maxfd);
select_test(maxfd + 1, &fdread, &fdwrite, &fdexcep, &timeout);
abort_on_test_timeout();
multi_perform(multi, &still_running);
abort_on_test_timeout();
}
test_cleanup:
#if defined(LIB1502)
curl_multi_cleanup(multi);
curl_easy_cleanup(easy);
curl_global_cleanup();
#endif
#if defined(LIB1503)
curl_multi_remove_handle(multi, easy);
curl_multi_cleanup(multi);
curl_easy_cleanup(easy);
curl_global_cleanup();
#endif
#if defined(LIB1504)
curl_easy_cleanup(easy);
curl_multi_cleanup(multi);
curl_global_cleanup();
#endif
#if defined(LIB1505)
curl_multi_remove_handle(multi, easy);
curl_easy_cleanup(easy);
curl_multi_cleanup(multi);
curl_global_cleanup();
#endif
curl_slist_free_all(dns_cache_list);
return res;
}
