#include "test.h"
#include <limits.h>
#include "testutil.h"
#include "warnless.h"
#include "memdebug.h"
#define TEST_HANG_TIMEOUT 5 * 1000
int test(char *URL)
{
CURL *easy = NULL;
CURLM *multi = NULL;
int res = 0;
int running;
int msgs_left;
CURLMsg *msg;
start_test_timing();
global_init(CURL_GLOBAL_ALL);
easy_init(easy);
multi_init(multi);
easy_setopt(easy, CURLOPT_VERBOSE, 1L);
easy_setopt(easy, CURLOPT_URL, URL);
easy_setopt(easy, CURLOPT_CONNECT_ONLY, 1L);
multi_add_handle(multi, easy);
for(;;) {
struct timeval interval;
fd_set fdread;
fd_set fdwrite;
fd_set fdexcep;
long timeout = -99;
int maxfd = -99;
multi_perform(multi, &running);
abort_on_test_timeout();
if(!running)
break; 
FD_ZERO(&fdread);
FD_ZERO(&fdwrite);
FD_ZERO(&fdexcep);
multi_fdset(multi, &fdread, &fdwrite, &fdexcep, &maxfd);
multi_timeout(multi, &timeout);
if(timeout != -1L) {
int itimeout = (timeout > (long)INT_MAX) ? INT_MAX : (int)timeout;
interval.tv_sec = itimeout/1000;
interval.tv_usec = (itimeout%1000)*1000;
}
else {
interval.tv_sec = TEST_HANG_TIMEOUT/1000 + 1;
interval.tv_usec = 0;
}
select_test(maxfd + 1, &fdread, &fdwrite, &fdexcep, &interval);
abort_on_test_timeout();
}
msg = curl_multi_info_read(multi, &msgs_left);
if(msg)
res = msg->data.result;
multi_remove_handle(multi, easy);
test_cleanup:
curl_multi_cleanup(multi);
curl_easy_cleanup(easy);
curl_global_cleanup();
return res;
}
