




















#include "test.h"
#include "memdebug.h"

static void my_lock(CURL *handle, curl_lock_data data,
curl_lock_access laccess, void *useptr)
{
(void)handle;
(void)data;
(void)laccess;
(void)useptr;
printf("-> Mutex lock\n");
}

static void my_unlock(CURL *handle, curl_lock_data data, void *useptr)
{
(void)handle;
(void)data;
(void)useptr;
printf("<- Mutex unlock\n");
}


int test(char *URL)
{
CURLcode res = CURLE_OK;
CURLSH *share;
int i;

global_init(CURL_GLOBAL_ALL);

share = curl_share_init();
if(!share) {
fprintf(stderr, "curl_share_init() failed\n");
curl_global_cleanup();
return TEST_ERR_MAJOR_BAD;
}

curl_share_setopt(share, CURLSHOPT_SHARE, CURL_LOCK_DATA_CONNECT);
curl_share_setopt(share, CURLSHOPT_LOCKFUNC, my_lock);
curl_share_setopt(share, CURLSHOPT_UNLOCKFUNC, my_unlock);




for(i = 0; i < 3; i++) {
CURL *curl = curl_easy_init();
if(curl) {
curl_easy_setopt(curl, CURLOPT_URL, URL);


curl_easy_setopt(curl, CURLOPT_SHARE, share);


res = curl_easy_perform(curl);

if(res != CURLE_OK)
fprintf(stderr, "curl_easy_perform() failed: %s\n",
curl_easy_strerror(res));


curl_easy_cleanup(curl);
}
}

curl_share_cleanup(share);
curl_global_cleanup();

return 0;
}
