#include "test.h"
#include "testutil.h"
#include "warnless.h"
#include "memdebug.h"
#define TEST_HANG_TIMEOUT 60 * 1000
int test(char *URL)
{
CURL *curls = NULL;
CURLM *multi = NULL;
int still_running;
int i = 0;
int res = 0;
CURLMsg *msg;
int counter = 3;
start_test_timing();
global_init(CURL_GLOBAL_ALL);
multi_init(multi);
easy_init(curls);
easy_setopt(curls, CURLOPT_URL, URL);
easy_setopt(curls, CURLOPT_HEADER, 1L);
easy_setopt(curls, CURLOPT_VERBOSE, 1L);
easy_setopt(curls, CURLOPT_USERPWD, "u:s");
multi_add_handle(multi, curls);
multi_perform(multi, &still_running);
abort_on_test_timeout();
while(still_running && counter--) {
int num;
res = curl_multi_wait(multi, NULL, 0, TEST_HANG_TIMEOUT, &num);
if(res != CURLM_OK) {
printf("curl_multi_wait() returned %d\n", res);
res = TEST_ERR_MAJOR_BAD;
goto test_cleanup;
}
abort_on_test_timeout();
multi_perform(multi, &still_running);
abort_on_test_timeout();
}
msg = curl_multi_info_read(multi, &still_running);
if(msg)
i = msg->data.result;
test_cleanup:
curl_multi_cleanup(multi);
curl_easy_cleanup(curls);
curl_global_cleanup();
if(res)
i = res;
return i; 
}
