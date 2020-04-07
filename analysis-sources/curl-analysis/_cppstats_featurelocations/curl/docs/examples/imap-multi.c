


























#include <stdio.h>
#include <string.h>
#include <curl/curl.h>








#define MULTI_PERFORM_HANG_TIMEOUT 60 * 1000

static struct timeval tvnow(void)
{
struct timeval now;


now.tv_sec = (long)time(NULL);
now.tv_usec = 0;

return now;
}

static long tvdiff(struct timeval newer, struct timeval older)
{
return (newer.tv_sec - older.tv_sec) * 1000 +
(newer.tv_usec - older.tv_usec) / 1000;
}

int main(void)
{
CURL *curl;
CURLM *mcurl;
int still_running = 1;
struct timeval mp_start;

curl_global_init(CURL_GLOBAL_DEFAULT);

curl = curl_easy_init();
if(!curl)
return 1;

mcurl = curl_multi_init();
if(!mcurl)
return 2;


curl_easy_setopt(curl, CURLOPT_USERNAME, "user");
curl_easy_setopt(curl, CURLOPT_PASSWORD, "secret");


curl_easy_setopt(curl, CURLOPT_URL, "imap://imap.example.com/INBOX/;UID=1");


curl_multi_add_handle(mcurl, curl);


mp_start = tvnow();


curl_multi_perform(mcurl, &still_running);

while(still_running) {
struct timeval timeout;
fd_set fdread;
fd_set fdwrite;
fd_set fdexcep;
int maxfd = -1;
int rc;
CURLMcode mc; 

long curl_timeo = -1;


FD_ZERO(&fdread);
FD_ZERO(&fdwrite);
FD_ZERO(&fdexcep);


timeout.tv_sec = 1;
timeout.tv_usec = 0;

curl_multi_timeout(mcurl, &curl_timeo);
if(curl_timeo >= 0) {
timeout.tv_sec = curl_timeo / 1000;
if(timeout.tv_sec > 1)
timeout.tv_sec = 1;
else
timeout.tv_usec = (curl_timeo % 1000) * 1000;
}


mc = curl_multi_fdset(mcurl, &fdread, &fdwrite, &fdexcep, &maxfd);

if(mc != CURLM_OK) {
fprintf(stderr, "curl_multi_fdset() failed, code %d.\n", mc);
break;
}







if(maxfd == -1) {
#if defined(_WIN32)
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

if(tvdiff(tvnow(), mp_start) > MULTI_PERFORM_HANG_TIMEOUT) {
fprintf(stderr,
"ABORTING: Since it seems that we would have run forever.\n");
break;
}

switch(rc) {
case -1: 
break;
case 0: 
default: 
curl_multi_perform(mcurl, &still_running);
break;
}
}


curl_multi_remove_handle(mcurl, curl);
curl_multi_cleanup(mcurl);
curl_easy_cleanup(curl);
curl_global_cleanup();

return 0;
}
