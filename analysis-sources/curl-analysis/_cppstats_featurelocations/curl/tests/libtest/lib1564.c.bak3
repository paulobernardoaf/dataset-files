




















#include "test.h"

#include "testutil.h"
#include "warnless.h"
#include "memdebug.h"

#define TEST_HANG_TIMEOUT 60 * 1000
#define WAKEUP_NUM 10

int test(char *URL)
{
CURLM *multi = NULL;
int numfds;
int i;
int res = 0;
struct timeval time_before_wait, time_after_wait;

(void)URL;

start_test_timing();

global_init(CURL_GLOBAL_ALL);

multi_init(multi);



time_before_wait = tutil_tvnow();
multi_poll(multi, NULL, 0, 1000, &numfds);
time_after_wait = tutil_tvnow();

if(tutil_tvdiff(time_after_wait, time_before_wait) < 500) {
fprintf(stderr, "%s:%d curl_multi_poll returned too early\n",
__FILE__, __LINE__);
res = TEST_ERR_MAJOR_BAD;
goto test_cleanup;
}

abort_on_test_timeout();



multi_wakeup(multi);

time_before_wait = tutil_tvnow();
multi_poll(multi, NULL, 0, 1000, &numfds);
time_after_wait = tutil_tvnow();

if(tutil_tvdiff(time_after_wait, time_before_wait) > 500) {
fprintf(stderr, "%s:%d curl_multi_poll returned too late\n",
__FILE__, __LINE__);
res = TEST_ERR_MAJOR_BAD;
goto test_cleanup;
}

abort_on_test_timeout();



time_before_wait = tutil_tvnow();
multi_poll(multi, NULL, 0, 1000, &numfds);
time_after_wait = tutil_tvnow();

if(tutil_tvdiff(time_after_wait, time_before_wait) < 500) {
fprintf(stderr, "%s:%d curl_multi_poll returned too early\n",
__FILE__, __LINE__);
res = TEST_ERR_MAJOR_BAD;
goto test_cleanup;
}

abort_on_test_timeout();



for(i = 0; i < WAKEUP_NUM; ++i)
multi_wakeup(multi);

time_before_wait = tutil_tvnow();
multi_poll(multi, NULL, 0, 1000, &numfds);
time_after_wait = tutil_tvnow();

if(tutil_tvdiff(time_after_wait, time_before_wait) > 500) {
fprintf(stderr, "%s:%d curl_multi_poll returned too late\n",
__FILE__, __LINE__);
res = TEST_ERR_MAJOR_BAD;
goto test_cleanup;
}

abort_on_test_timeout();

#if !defined(WIN32) && !defined(_WIN32) && !defined(__WIN32__) && !defined(__CYGWIN__)









time_before_wait = tutil_tvnow();
multi_poll(multi, NULL, 0, 1000, &numfds);
time_after_wait = tutil_tvnow();

if(tutil_tvdiff(time_after_wait, time_before_wait) < 500) {
fprintf(stderr, "%s:%d curl_multi_poll returned too early\n",
__FILE__, __LINE__);
res = TEST_ERR_MAJOR_BAD;
goto test_cleanup;
}

abort_on_test_timeout();
#endif

test_cleanup:

curl_multi_cleanup(multi);
curl_global_cleanup();

return res;
}
