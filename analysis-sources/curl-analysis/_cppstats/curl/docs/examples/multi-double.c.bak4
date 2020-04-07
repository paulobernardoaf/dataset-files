
























#include <stdio.h>
#include <string.h>


#include <sys/time.h>
#include <unistd.h>


#include <curl/curl.h>




int main(void)
{
CURL *http_handle;
CURL *http_handle2;
CURLM *multi_handle;

int still_running = 0; 

http_handle = curl_easy_init();
http_handle2 = curl_easy_init();


curl_easy_setopt(http_handle, CURLOPT_URL, "https://www.example.com/");


curl_easy_setopt(http_handle2, CURLOPT_URL, "http://localhost/");


multi_handle = curl_multi_init();


curl_multi_add_handle(multi_handle, http_handle);
curl_multi_add_handle(multi_handle, http_handle2);


curl_multi_perform(multi_handle, &still_running);

while(still_running) {
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

switch(rc) {
case -1:

break;
case 0:
default:

curl_multi_perform(multi_handle, &still_running);
break;
}
}

curl_multi_cleanup(multi_handle);

curl_easy_cleanup(http_handle);
curl_easy_cleanup(http_handle2);

return 0;
}
