




















#include "test.h"

#include <fcntl.h>

#include "testutil.h"
#include "warnless.h"
#include "memdebug.h"

#define TEST_HANG_TIMEOUT 60 * 1000

int test(char *URL)
{
int res = 0;
CURL *curl = NULL;
int running;
CURLM *m = NULL;

start_test_timing();

global_init(CURL_GLOBAL_ALL);

easy_init(curl);

easy_setopt(curl, CURLOPT_URL, URL);
easy_setopt(curl, CURLOPT_VERBOSE, 1L);
easy_setopt(curl, CURLOPT_PROXY, libtest_arg2);
easy_setopt(curl, CURLOPT_PROXYTYPE, (long)CURLPROXY_SOCKS4);

multi_init(m);

multi_add_handle(m, curl);

fprintf(stderr, "Start at URL 0\n");

for(;;) {
struct timeval interval;
fd_set rd, wr, exc;
int maxfd = -99;

interval.tv_sec = 1;
interval.tv_usec = 0;

multi_perform(m, &running);

abort_on_test_timeout();

if(!running)
break; 

FD_ZERO(&rd);
FD_ZERO(&wr);
FD_ZERO(&exc);

multi_fdset(m, &rd, &wr, &exc, &maxfd);



select_test(maxfd + 1, &rd, &wr, &exc, &interval);

abort_on_test_timeout();
}

test_cleanup:



curl_easy_cleanup(curl);
curl_multi_cleanup(m);
curl_global_cleanup();

return res;
}
