





























#include "test.h"

#include <limits.h>

#include "testutil.h"
#include "warnless.h"
#include "memdebug.h"

#define TEST_HANG_TIMEOUT 60 * 1000

#define PROXY libtest_arg2
#define PROXYUSERPWD libtest_arg3
#define HOST test_argv[4]

#define NUM_HANDLES 2

static CURL *eh[NUM_HANDLES];

static int init(int num, CURLM *cm, const char *url, const char *userpwd,
struct curl_slist *headers)
{
int res = 0;

res_easy_init(eh[num]);
if(res)
goto init_failed;

res_easy_setopt(eh[num], CURLOPT_URL, url);
if(res)
goto init_failed;

res_easy_setopt(eh[num], CURLOPT_PROXY, PROXY);
if(res)
goto init_failed;

res_easy_setopt(eh[num], CURLOPT_PROXYUSERPWD, userpwd);
if(res)
goto init_failed;

res_easy_setopt(eh[num], CURLOPT_PROXYAUTH, (long)CURLAUTH_ANY);
if(res)
goto init_failed;

res_easy_setopt(eh[num], CURLOPT_VERBOSE, 1L);
if(res)
goto init_failed;

res_easy_setopt(eh[num], CURLOPT_HEADER, 1L);
if(res)
goto init_failed;

res_easy_setopt(eh[num], CURLOPT_HTTPHEADER, headers); 
if(res)
goto init_failed;

res_multi_add_handle(cm, eh[num]);
if(res)
goto init_failed;

return 0; 

init_failed:

curl_easy_cleanup(eh[num]);
eh[num] = NULL;

return res; 
}

static int loop(int num, CURLM *cm, const char *url, const char *userpwd,
struct curl_slist *headers)
{
CURLMsg *msg;
long L;
int Q, U = -1;
fd_set R, W, E;
struct timeval T;
int res = 0;

res = init(num, cm, url, userpwd, headers);
if(res)
return res;

while(U) {

int M = -99;

res_multi_perform(cm, &U);
if(res)
return res;

res_test_timedout();
if(res)
return res;

if(U) {
FD_ZERO(&R);
FD_ZERO(&W);
FD_ZERO(&E);

res_multi_fdset(cm, &R, &W, &E, &M);
if(res)
return res;



res_multi_timeout(cm, &L);
if(res)
return res;



if(L != -1) {
int itimeout = (L > (long)INT_MAX) ? INT_MAX : (int)L;
T.tv_sec = itimeout/1000;
T.tv_usec = (itimeout%1000)*1000;
}
else {
T.tv_sec = 5;
T.tv_usec = 0;
}

res_select_test(M + 1, &R, &W, &E, &T);
if(res)
return res;
}

while((msg = curl_multi_info_read(cm, &Q)) != NULL) {
if(msg->msg == CURLMSG_DONE) {
int i;
CURL *e = msg->easy_handle;
fprintf(stderr, "R: %d - %s\n", (int)msg->data.result,
curl_easy_strerror(msg->data.result));
curl_multi_remove_handle(cm, e);
curl_easy_cleanup(e);
for(i = 0; i < NUM_HANDLES; i++) {
if(eh[i] == e) {
eh[i] = NULL;
break;
}
}
}
else
fprintf(stderr, "E: CURLMsg (%d)\n", (int)msg->msg);
}

res_test_timedout();
if(res)
return res;
}

return 0; 
}

int test(char *URL)
{
CURLM *cm = NULL;
struct curl_slist *headers = NULL;
char buffer[246]; 
int res = 0;
int i;

for(i = 0; i < NUM_HANDLES; i++)
eh[i] = NULL;

start_test_timing();

if(test_argc < 4)
return 99;

msnprintf(buffer, sizeof(buffer), "Host: %s", HOST);


headers = curl_slist_append(headers, buffer);
if(!headers) {
fprintf(stderr, "curl_slist_append() failed\n");
return TEST_ERR_MAJOR_BAD;
}

res_global_init(CURL_GLOBAL_ALL);
if(res) {
curl_slist_free_all(headers);
return res;
}

res_multi_init(cm);
if(res) {
curl_global_cleanup();
curl_slist_free_all(headers);
return res;
}

res = loop(0, cm, URL, PROXYUSERPWD, headers);
if(res)
goto test_cleanup;

fprintf(stderr, "lib540: now we do the request again\n");

res = loop(1, cm, URL, PROXYUSERPWD, headers);

test_cleanup:



for(i = 0; i < NUM_HANDLES; i++) {
curl_multi_remove_handle(cm, eh[i]);
curl_easy_cleanup(eh[i]);
}

curl_multi_cleanup(cm);
curl_global_cleanup();

curl_slist_free_all(headers);

return res;
}
