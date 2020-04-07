#include "test.h"
#include <fcntl.h>
#include "testutil.h"
#include "warnless.h"
#include "memdebug.h"
#define TEST_HANG_TIMEOUT 30 * 1000
#define MAX_BLOCKED_TIME_MS 500
int test(char *URL)
{
CURL *handle = NULL;
CURLM *mhandle = NULL;
int res = 0;
int still_running = 0;
start_test_timing();
global_init(CURL_GLOBAL_ALL);
easy_init(handle);
easy_setopt(handle, CURLOPT_URL, URL);
easy_setopt(handle, CURLOPT_VERBOSE, 1L);
multi_init(mhandle);
multi_add_handle(mhandle, handle);
multi_perform(mhandle, &still_running);
abort_on_test_timeout();
while(still_running) {
struct timeval timeout;
fd_set fdread;
fd_set fdwrite;
fd_set fdexcep;
int maxfd = -99;
struct timeval before;
struct timeval after;
long e;
timeout.tv_sec = 0;
timeout.tv_usec = 100000L; 
FD_ZERO(&fdread);
FD_ZERO(&fdwrite);
FD_ZERO(&fdexcep);
multi_fdset(mhandle, &fdread, &fdwrite, &fdexcep, &maxfd);
select_test(maxfd + 1, &fdread, &fdwrite, &fdexcep, &timeout);
abort_on_test_timeout();
fprintf(stderr, "ping\n");
before = tutil_tvnow();
multi_perform(mhandle, &still_running);
abort_on_test_timeout();
after = tutil_tvnow();
e = tutil_tvdiff(after, before);
fprintf(stderr, "pong = %ld\n", e);
if(e > MAX_BLOCKED_TIME_MS) {
res = 100;
break;
}
}
test_cleanup:
curl_multi_cleanup(mhandle);
curl_easy_cleanup(handle);
curl_global_cleanup();
return res;
}
