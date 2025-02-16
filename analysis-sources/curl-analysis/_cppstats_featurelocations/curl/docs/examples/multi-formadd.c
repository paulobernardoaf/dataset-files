

























#include <stdio.h>
#include <string.h>
#include <sys/time.h>

#include <curl/curl.h>

int main(void)
{
CURL *curl;

CURLM *multi_handle;
int still_running = 0;

struct curl_httppost *formpost = NULL;
struct curl_httppost *lastptr = NULL;
struct curl_slist *headerlist = NULL;
static const char buf[] = "Expect:";



curl_formadd(&formpost,
&lastptr,
CURLFORM_COPYNAME, "sendfile",
CURLFORM_FILE, "postit2.c",
CURLFORM_END);


curl_formadd(&formpost,
&lastptr,
CURLFORM_COPYNAME, "filename",
CURLFORM_COPYCONTENTS, "postit2.c",
CURLFORM_END);


curl_formadd(&formpost,
&lastptr,
CURLFORM_COPYNAME, "submit",
CURLFORM_COPYCONTENTS, "send",
CURLFORM_END);

curl = curl_easy_init();
multi_handle = curl_multi_init();



headerlist = curl_slist_append(headerlist, buf);
if(curl && multi_handle) {


curl_easy_setopt(curl, CURLOPT_URL, "https://www.example.com/upload.cgi");
curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);

curl_multi_add_handle(multi_handle, curl);

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

printf("perform!\n");
curl_multi_perform(multi_handle, &still_running);
printf("running: %d!\n", still_running);
break;
}
}

curl_multi_cleanup(multi_handle);


curl_easy_cleanup(curl);


curl_formfree(formpost);


curl_slist_free_all(headerlist);
}
return 0;
}
