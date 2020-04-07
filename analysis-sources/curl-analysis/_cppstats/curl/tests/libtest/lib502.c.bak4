




















#include "test.h"

#include "testutil.h"
#include "warnless.h"
#include "memdebug.h"

#define TEST_HANG_TIMEOUT 60 * 1000





int test(char *URL)
{
CURL *c = NULL;
CURLM *m = NULL;
int res = 0;
int running;

start_test_timing();

global_init(CURL_GLOBAL_ALL);

easy_init(c);

easy_setopt(c, CURLOPT_URL, URL);

multi_init(m);

multi_add_handle(m, c);

for(;;) {
struct timeval timeout;
fd_set fdread, fdwrite, fdexcep;
int maxfd = -99;

timeout.tv_sec = 0;
timeout.tv_usec = 100000L; 

multi_perform(m, &running);

abort_on_test_timeout();

if(!running)
break; 

FD_ZERO(&fdread);
FD_ZERO(&fdwrite);
FD_ZERO(&fdexcep);

multi_fdset(m, &fdread, &fdwrite, &fdexcep, &maxfd);



select_test(maxfd + 1, &fdread, &fdwrite, &fdexcep, &timeout);

abort_on_test_timeout();
}

test_cleanup:



curl_multi_remove_handle(m, c);
curl_multi_cleanup(m);
curl_easy_cleanup(c);
curl_global_cleanup();

return res;
}
