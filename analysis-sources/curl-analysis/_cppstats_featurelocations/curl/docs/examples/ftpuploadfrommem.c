
























#include <stdio.h>
#include <string.h>
#include <curl/curl.h>

static const char data[]=
"Lorem ipsum dolor sit amet, consectetur adipiscing elit. "
"Nam rhoncus odio id venenatis volutpat. Vestibulum dapibus "
"bibendum ullamcorper. Maecenas finibus elit augue, vel "
"condimentum odio maximus nec. In hac habitasse platea dictumst. "
"Vestibulum vel dolor et turpis rutrum finibus ac at nulla. "
"Vivamus nec neque ac elit blandit pretium vitae maximus ipsum. "
"Quisque sodales magna vel erat auctor, sed pellentesque nisi "
"rhoncus. Donec vehicula maximus pretium. Aliquam eu tincidunt "
"lorem.";

struct WriteThis {
const char *readptr;
size_t sizeleft;
};

static size_t read_callback(void *ptr, size_t size, size_t nmemb, void *userp)
{
struct WriteThis *upload = (struct WriteThis *)userp;
size_t max = size*nmemb;

if(max < 1)
return 0;

if(upload->sizeleft) {
size_t copylen = max;
if(copylen > upload->sizeleft)
copylen = upload->sizeleft;
memcpy(ptr, upload->readptr, copylen);
upload->readptr += copylen;
upload->sizeleft -= copylen;
return copylen;
}

return 0; 
}

int main(void)
{
CURL *curl;
CURLcode res;

struct WriteThis upload;

upload.readptr = data;
upload.sizeleft = strlen(data);


res = curl_global_init(CURL_GLOBAL_DEFAULT);

if(res != CURLE_OK) {
fprintf(stderr, "curl_global_init() failed: %s\n",
curl_easy_strerror(res));
return 1;
}


curl = curl_easy_init();
if(curl) {

curl_easy_setopt(curl, CURLOPT_URL,
"ftp://example.com/path/to/upload/file");


curl_easy_setopt(curl, CURLOPT_USERPWD, "login:secret");


curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);


curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);


curl_easy_setopt(curl, CURLOPT_READDATA, &upload);


curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);


curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE,
(curl_off_t)upload.sizeleft);


res = curl_easy_perform(curl);

if(res != CURLE_OK)
fprintf(stderr, "curl_easy_perform() failed: %s\n",
curl_easy_strerror(res));


curl_easy_cleanup(curl);
}
curl_global_cleanup();
return 0;
}
