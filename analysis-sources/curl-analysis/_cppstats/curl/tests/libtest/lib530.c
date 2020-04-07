#include "test.h"
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
int i;
char target_url[256];
int handles_added = 0;
for(i = 0; i < NUM_HANDLES; i++)
curl[i] = NULL;
start_test_timing();
global_init(CURL_GLOBAL_ALL);
multi_init(m);
for(i = 0; i < NUM_HANDLES; i++) {
easy_init(curl[i]);
msnprintf(target_url, sizeof(target_url), "%s%04i", URL, i + 1);
target_url[sizeof(target_url) - 1] = '\0';
easy_setopt(curl[i], CURLOPT_URL, target_url);
easy_setopt(curl[i], CURLOPT_VERBOSE, 1L);
easy_setopt(curl[i], CURLOPT_HEADER, 1L);
}
multi_add_handle(m, curl[handles_added++]);
multi_setopt(m, CURLMOPT_PIPELINING, 1L);
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
if(handles_added >= NUM_HANDLES)
break; 
while(handles_added < NUM_HANDLES)
multi_add_handle(m, curl[handles_added++]);
}
FD_ZERO(&rd);
FD_ZERO(&wr);
FD_ZERO(&exc);
multi_fdset(m, &rd, &wr, &exc, &maxfd);
select_test(maxfd + 1, &rd, &wr, &exc, &interval);
abort_on_test_timeout();
}
test_cleanup:
for(i = 0; i < NUM_HANDLES; i++) {
curl_multi_remove_handle(m, curl[i]);
curl_easy_cleanup(curl[i]);
}
curl_multi_cleanup(m);
curl_global_cleanup();
return res;
}
