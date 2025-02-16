#include "test.h"
#include "testutil.h"
#include "warnless.h"
#include "memdebug.h"
#define TEST_HANG_TIMEOUT 60 * 1000
static char const testData[] = ".abc\0xyz";
static off_t const testDataSize = sizeof(testData) - 1;
int test(char *URL)
{
CURL *easy;
CURLM *multi_handle;
int still_running; 
CURLMsg *msg; 
int msgs_left; 
int res = CURLE_OK;
start_test_timing();
global_init(CURL_GLOBAL_ALL);
easy = curl_easy_init();
multi_handle = curl_multi_init();
curl_multi_add_handle(multi_handle, easy);
curl_easy_setopt(easy, CURLOPT_URL, URL);
curl_easy_setopt(easy, CURLOPT_POSTFIELDSIZE_LARGE,
(curl_off_t)testDataSize);
curl_easy_setopt(easy, CURLOPT_POSTFIELDS, testData);
curl_multi_perform(multi_handle, &still_running);
abort_on_test_timeout();
do {
struct timeval timeout;
int rc; 
CURLMcode mc; 
fd_set fdread;
fd_set fdwrite;
fd_set fdexcep;
int maxfd = -1;
long curl_timeo = -1;
FD_ZERO(&fdread);
FD_ZERO(&fdwrite);
FD_ZERO(&fdexcep);
timeout.tv_sec = 1;
timeout.tv_usec = 0;
curl_multi_timeout(multi_handle, &curl_timeo);
if(curl_timeo >= 0) {
timeout.tv_sec = curl_timeo / 1000;
if(timeout.tv_sec > 1)
timeout.tv_sec = 1;
else
timeout.tv_usec = (curl_timeo % 1000) * 1000;
}
mc = curl_multi_fdset(multi_handle, &fdread, &fdwrite, &fdexcep, &maxfd);
if(mc != CURLM_OK) {
fprintf(stderr, "curl_multi_fdset() failed, code %d.\n", mc);
break;
}
if(maxfd == -1) {
#if defined(WIN32) || defined(_WIN32)
Sleep(100);
rc = 0;
#else
struct timeval wait = { 0, 100 * 1000 }; 
rc = select(0, NULL, NULL, NULL, &wait);
#endif
}
else {
rc = select(maxfd + 1, &fdread, &fdwrite, &fdexcep, &timeout);
}
switch(rc) {
case -1:
break;
case 0: 
default: 
curl_multi_perform(multi_handle, &still_running);
break;
}
abort_on_test_timeout();
} while(still_running);
do {
msg = curl_multi_info_read(multi_handle, &msgs_left);
if(msg && msg->msg == CURLMSG_DONE) {
printf("HTTP transfer completed with status %d\n", msg->data.result);
break;
}
abort_on_test_timeout();
} while(msg);
test_cleanup:
curl_multi_cleanup(multi_handle);
curl_easy_cleanup(easy);
curl_global_cleanup();
return res;
}
