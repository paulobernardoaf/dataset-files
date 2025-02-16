#include "test.h"
#include "testutil.h"
#include "warnless.h"
#include "memdebug.h"
#define TEST_HANG_TIMEOUT 60 * 1000
#define DNS_TIMEOUT 1
#if defined(WIN32) || defined(_WIN32)
#define sleep(sec) Sleep ((sec)*1000)
#endif
static int debug_callback(CURL *curl, curl_infotype info, char *msg,
size_t len, void *ptr)
{
(void)curl;
(void)ptr;
if(info == CURLINFO_TEXT)
fprintf(stderr, "debug: %.*s", (int) len, msg);
return 0;
}
static int do_one_request(CURLM *m, char *URL, char *resolve)
{
CURL *curls;
struct curl_slist *resolve_list = NULL;
int still_running;
int res = 0;
CURLMsg *msg;
int msgs_left;
resolve_list = curl_slist_append(resolve_list, resolve);
easy_init(curls);
easy_setopt(curls, CURLOPT_URL, URL);
easy_setopt(curls, CURLOPT_RESOLVE, resolve_list);
easy_setopt(curls, CURLOPT_DEBUGFUNCTION, debug_callback);
easy_setopt(curls, CURLOPT_VERBOSE, 1);
easy_setopt(curls, CURLOPT_DNS_CACHE_TIMEOUT, DNS_TIMEOUT);
multi_add_handle(m, curls);
multi_perform(m, &still_running);
abort_on_test_timeout();
while(still_running) {
struct timeval timeout;
fd_set fdread, fdwrite, fdexcep;
int maxfd = -99;
FD_ZERO(&fdread);
FD_ZERO(&fdwrite);
FD_ZERO(&fdexcep);
timeout.tv_sec = 1;
timeout.tv_usec = 0;
multi_fdset(m, &fdread, &fdwrite, &fdexcep, &maxfd);
select_test(maxfd + 1, &fdread, &fdwrite, &fdexcep, &timeout);
abort_on_test_timeout();
multi_perform(m, &still_running);
abort_on_test_timeout();
}
do {
msg = curl_multi_info_read(m, &msgs_left);
if(msg && msg->msg == CURLMSG_DONE && msg->easy_handle == curls) {
res = msg->data.result;
break;
}
} while(msg);
test_cleanup:
curl_multi_remove_handle(m, curls);
curl_easy_cleanup(curls);
curl_slist_free_all(resolve_list);
return res;
}
int test(char *URL)
{
CURLM *multi = NULL;
int res = 0;
char *address = libtest_arg2;
char *port = libtest_arg3;
char *path = URL;
char dns_entry[256];
int i;
int count = 2;
msnprintf(dns_entry, sizeof(dns_entry), "testserver.example.com:%s:%s",
port, address);
start_test_timing();
global_init(CURL_GLOBAL_ALL);
multi_init(multi);
for(i = 1; i <= count; i++) {
char target_url[256];
msnprintf(target_url, sizeof(target_url),
"http://testserver.example.com:%s/%s%04d", port, path, i);
res = do_one_request(multi, target_url, dns_entry);
if(res)
goto test_cleanup;
if(i < count)
sleep(DNS_TIMEOUT + 1);
}
test_cleanup:
curl_multi_cleanup(multi);
curl_global_cleanup();
return (int) res;
}
