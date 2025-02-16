#include "test.h"
#include <fcntl.h>
#include "testutil.h"
#include "warnless.h"
#include "memdebug.h"
#define TEST_HANG_TIMEOUT 60 * 1000
#define NUM_HANDLES 4
int test(char *URL)
{
int res = 0;
CURL *curl[NUM_HANDLES];
int running;
CURLM *m = NULL;
int current = 0;
int i;
for(i = 0; i < NUM_HANDLES; i++)
curl[i] = NULL;
start_test_timing();
global_init(CURL_GLOBAL_ALL);
for(i = 0; i < NUM_HANDLES; i++) {
easy_init(curl[i]);
easy_setopt(curl[i], CURLOPT_URL, URL);
easy_setopt(curl[i], CURLOPT_VERBOSE, 1L);
}
multi_init(m);
multi_add_handle(m, curl[current]);
fprintf(stderr, "Start at URL 0\n");
for(;;) {
struct timeval interval;
fd_set rd, wr, exc;
int maxfd = -99;
interval.tv_sec = 1;
interval.tv_usec = 0;
multi_perform(m, &running);
abort_on_test_timeout();
if(!running) {
#if defined(LIB527)
curl_easy_cleanup(curl[current]);
curl[current] = NULL;
#endif
if(++current < NUM_HANDLES) {
fprintf(stderr, "Advancing to URL %d\n", current);
#if defined(LIB532)
curl_multi_remove_handle(m, curl[0]);
curl_easy_reset(curl[0]);
easy_setopt(curl[0], CURLOPT_URL, URL);
easy_setopt(curl[0], CURLOPT_VERBOSE, 1L);
multi_add_handle(m, curl[0]);
#else
multi_add_handle(m, curl[current]);
#endif
}
else {
break; 
}
}
FD_ZERO(&rd);
FD_ZERO(&wr);
FD_ZERO(&exc);
multi_fdset(m, &rd, &wr, &exc, &maxfd);
select_test(maxfd + 1, &rd, &wr, &exc, &interval);
abort_on_test_timeout();
}
test_cleanup:
#if defined(LIB526)
for(i = 0; i < NUM_HANDLES; i++) {
curl_multi_remove_handle(m, curl[i]);
curl_easy_cleanup(curl[i]);
}
curl_multi_cleanup(m);
curl_global_cleanup();
#elif defined(LIB527)
if(res)
for(i = 0; i < NUM_HANDLES; i++)
curl_easy_cleanup(curl[i]);
curl_multi_cleanup(m);
curl_global_cleanup();
#elif defined(LIB532)
for(i = 0; i < NUM_HANDLES; i++)
curl_easy_cleanup(curl[i]);
curl_multi_cleanup(m);
curl_global_cleanup();
#endif
return res;
}
