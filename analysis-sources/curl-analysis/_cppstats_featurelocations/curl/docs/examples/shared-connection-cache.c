
























#include <stdio.h>
#include <curl/curl.h>

static void my_lock(CURL *handle, curl_lock_data data,
curl_lock_access laccess, void *useptr)
{
(void)handle;
(void)data;
(void)laccess;
(void)useptr;
fprintf(stderr, "-> Mutex lock\n");
}

static void my_unlock(CURL *handle, curl_lock_data data, void *useptr)
{
(void)handle;
(void)data;
(void)useptr;
fprintf(stderr, "<- Mutex unlock\n");
}

int main(void)
{
CURLSH *share;
int i;

share = curl_share_init();
curl_share_setopt(share, CURLSHOPT_SHARE, CURL_LOCK_DATA_CONNECT);

curl_share_setopt(share, CURLSHOPT_LOCKFUNC, my_lock);
curl_share_setopt(share, CURLSHOPT_UNLOCKFUNC, my_unlock);




for(i = 0; i < 3; i++) {
CURL *curl = curl_easy_init();
if(curl) {
CURLcode res;

curl_easy_setopt(curl, CURLOPT_URL, "https://curl.haxx.se/");


curl_easy_setopt(curl, CURLOPT_SHARE, share);


res = curl_easy_perform(curl);

if(res != CURLE_OK)
fprintf(stderr, "curl_easy_perform() failed: %s\n",
curl_easy_strerror(res));


curl_easy_cleanup(curl);
}
}

curl_share_cleanup(share);
return 0;
}
