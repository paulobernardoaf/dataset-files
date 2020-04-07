#define TEST_HANG_TIMEOUT 2 * 1000
#include "test.h"
#include "testutil.h"
#include <sys/stat.h>
int test(char *URL)
{
int stillRunning;
CURLM *multiHandle = NULL;
CURL *curl = NULL;
CURLMcode res = CURLM_OK;
int timeout;
global_init(CURL_GLOBAL_ALL);
multi_init(multiHandle);
easy_init(curl);
easy_setopt(curl, CURLOPT_VERBOSE, 1L);
easy_setopt(curl, CURLOPT_URL, URL);
if(curl_easy_setopt(curl, CURLOPT_DNS_SERVERS, "0.0.0.0") == CURLE_OK)
timeout = TEST_HANG_TIMEOUT * 2;
else {
fprintf(stderr,
"CURLOPT_DNS_SERVERS not supported; "
"assuming curl_multi_remove_handle() will block\n");
timeout = TEST_HANG_TIMEOUT / 2;
}
easy_setopt(curl, CURLOPT_TIMEOUT_MS, timeout);
multi_add_handle(multiHandle, curl);
fprintf(stderr, "curl_multi_perform()...\n");
multi_perform(multiHandle, &stillRunning);
fprintf(stderr, "curl_multi_perform() succeeded\n");
fprintf(stderr, "curl_multi_remove_handle()...\n");
start_test_timing();
res = curl_multi_remove_handle(multiHandle, curl);
if(res) {
fprintf(stderr, "curl_multi_remove_handle() failed, "
"with code %d\n", (int)res);
goto test_cleanup;
}
fprintf(stderr, "curl_multi_remove_handle() succeeded\n");
abort_on_test_timeout();
test_cleanup:
curl_easy_cleanup(curl);
curl_multi_cleanup(multiHandle);
curl_global_cleanup();
return (int)res;
}
