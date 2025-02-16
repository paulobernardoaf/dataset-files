




















#include "test.h"

#include "testutil.h"
#include "warnless.h"
#include "memdebug.h"

struct transfer_status {
CURL *easy;
int halted;
int counter; 
int please; 
};

static int please_continue(void *userp,
curl_off_t dltotal,
curl_off_t dlnow,
curl_off_t ultotal,
curl_off_t ulnow)
{
struct transfer_status *st = (struct transfer_status *)userp;
(void)dltotal;
(void)dlnow;
(void)ultotal;
(void)ulnow;
if(st->halted) {
st->please++;
if(st->please == 2) {

curl_easy_pause(st->easy, CURLPAUSE_CONT);
}
}
fprintf(stderr, "xferinfo: paused %d\n", st->halted);
return 0; 
}

static size_t header_callback(void *ptr, size_t size, size_t nmemb,
void *userp)
{
size_t len = size * nmemb;
(void)userp;
(void)fwrite(ptr, size, nmemb, stdout);
return len;
}

static size_t write_callback(void *ptr, size_t size, size_t nmemb, void *userp)
{
struct transfer_status *st = (struct transfer_status *)userp;
size_t len = size * nmemb;
st->counter++;
if(st->counter > 1) {


fwrite(ptr, size, nmemb, stdout);
return len;
}
printf("Got %d bytes but pausing!\n", (int)len);
st->halted = 1;
return CURL_WRITEFUNC_PAUSE;
}

int test(char *URL)
{
CURL *curls = NULL;
int i = 0;
int res = 0;
struct transfer_status st;

start_test_timing();

memset(&st, 0, sizeof(st));

global_init(CURL_GLOBAL_ALL);

easy_init(curls);
st.easy = curls; 

easy_setopt(curls, CURLOPT_URL, URL);
easy_setopt(curls, CURLOPT_WRITEFUNCTION, write_callback);
easy_setopt(curls, CURLOPT_WRITEDATA, &st);
easy_setopt(curls, CURLOPT_HEADERFUNCTION, header_callback);
easy_setopt(curls, CURLOPT_HEADERDATA, &st);

easy_setopt(curls, CURLOPT_XFERINFOFUNCTION, please_continue);
easy_setopt(curls, CURLOPT_XFERINFODATA, &st);
easy_setopt(curls, CURLOPT_NOPROGRESS, 0L);

res = curl_easy_perform(curls);

test_cleanup:

curl_easy_cleanup(curls);
curl_global_cleanup();

if(res)
i = res;

return i; 
}
