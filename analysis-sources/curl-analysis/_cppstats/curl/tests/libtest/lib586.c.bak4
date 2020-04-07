




















#include "test.h"
#include "memdebug.h"

#define THREADS 2


struct Tdata {
CURLSH *share;
char *url;
};

struct userdata {
const char *text;
int counter;
};


static void my_lock(CURL *handle, curl_lock_data data,
curl_lock_access laccess, void *useptr)
{
const char *what;
struct userdata *user = (struct userdata *)useptr;

(void)handle;
(void)laccess;

switch(data) {
case CURL_LOCK_DATA_SHARE:
what = "share";
break;
case CURL_LOCK_DATA_DNS:
what = "dns";
break;
case CURL_LOCK_DATA_COOKIE:
what = "cookie";
break;
case CURL_LOCK_DATA_SSL_SESSION:
what = "ssl_session";
break;
default:
fprintf(stderr, "lock: no such data: %d\n", (int)data);
return;
}
printf("lock: %-6s [%s]: %d\n", what, user->text, user->counter);
user->counter++;
}


static void my_unlock(CURL *handle, curl_lock_data data, void *useptr)
{
const char *what;
struct userdata *user = (struct userdata *)useptr;
(void)handle;
switch(data) {
case CURL_LOCK_DATA_SHARE:
what = "share";
break;
case CURL_LOCK_DATA_DNS:
what = "dns";
break;
case CURL_LOCK_DATA_COOKIE:
what = "cookie";
break;
case CURL_LOCK_DATA_SSL_SESSION:
what = "ssl_session";
break;
default:
fprintf(stderr, "unlock: no such data: %d\n", (int)data);
return;
}
printf("unlock: %-6s [%s]: %d\n", what, user->text, user->counter);
user->counter++;
}


static void *fire(void *ptr)
{
CURLcode code;
struct Tdata *tdata = (struct Tdata*)ptr;
CURL *curl;

curl = curl_easy_init();
if(!curl) {
fprintf(stderr, "curl_easy_init() failed\n");
return NULL;
}

curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
curl_easy_setopt(curl, CURLOPT_URL, tdata->url);
printf("CURLOPT_SHARE\n");
curl_easy_setopt(curl, CURLOPT_SHARE, tdata->share);

printf("PERFORM\n");
code = curl_easy_perform(curl);
if(code != CURLE_OK) {
int i = 0;
fprintf(stderr, "perform url '%s' repeat %d failed, curlcode %d\n",
tdata->url, i, (int)code);
}

printf("CLEANUP\n");
curl_easy_cleanup(curl);

return NULL;
}


int test(char *URL)
{
int res;
CURLSHcode scode = CURLSHE_OK;
char *url;
struct Tdata tdata;
CURL *curl;
CURLSH *share;
int i;
struct userdata user;

user.text = "Pigs in space";
user.counter = 0;

printf("GLOBAL_INIT\n");
if(curl_global_init(CURL_GLOBAL_ALL) != CURLE_OK) {
fprintf(stderr, "curl_global_init() failed\n");
return TEST_ERR_MAJOR_BAD;
}


printf("SHARE_INIT\n");
share = curl_share_init();
if(!share) {
fprintf(stderr, "curl_share_init() failed\n");
curl_global_cleanup();
return TEST_ERR_MAJOR_BAD;
}

if(CURLSHE_OK == scode) {
printf("CURLSHOPT_LOCKFUNC\n");
scode = curl_share_setopt(share, CURLSHOPT_LOCKFUNC, my_lock);
}
if(CURLSHE_OK == scode) {
printf("CURLSHOPT_UNLOCKFUNC\n");
scode = curl_share_setopt(share, CURLSHOPT_UNLOCKFUNC, my_unlock);
}
if(CURLSHE_OK == scode) {
printf("CURLSHOPT_USERDATA\n");
scode = curl_share_setopt(share, CURLSHOPT_USERDATA, &user);
}
if(CURLSHE_OK == scode) {
printf("CURL_LOCK_DATA_SSL_SESSION\n");
scode = curl_share_setopt(share, CURLSHOPT_SHARE,
CURL_LOCK_DATA_SSL_SESSION);
}

if(CURLSHE_OK != scode) {
fprintf(stderr, "curl_share_setopt() failed\n");
curl_share_cleanup(share);
curl_global_cleanup();
return TEST_ERR_MAJOR_BAD;
}


res = 0;


for(i = 1; i <= THREADS; i++) {


tdata.url = URL;
tdata.share = share;


printf("*** run %d\n",i);
fire(&tdata);
}



printf("*** run %d\n", i);
curl = curl_easy_init();
if(!curl) {
fprintf(stderr, "curl_easy_init() failed\n");
curl_share_cleanup(share);
curl_global_cleanup();
return TEST_ERR_MAJOR_BAD;
}

url = URL;
test_setopt(curl, CURLOPT_URL, url);
printf("CURLOPT_SHARE\n");
test_setopt(curl, CURLOPT_SHARE, share);

printf("PERFORM\n");
curl_easy_perform(curl);


printf("try SHARE_CLEANUP...\n");
scode = curl_share_cleanup(share);
if(scode == CURLSHE_OK) {
fprintf(stderr, "curl_share_cleanup succeed but error expected\n");
share = NULL;
}
else {
printf("SHARE_CLEANUP failed, correct\n");
}

test_cleanup:


printf("CLEANUP\n");
curl_easy_cleanup(curl);


printf("SHARE_CLEANUP\n");
scode = curl_share_cleanup(share);
if(scode != CURLSHE_OK)
fprintf(stderr, "curl_share_cleanup failed, code errno %d\n",
(int)scode);

printf("GLOBAL_CLEANUP\n");
curl_global_cleanup();

return res;
}
