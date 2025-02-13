#include <stdio.h>
#include <string.h>
#include <curl/curl.h>
static const char data[]="Lorem ipsum dolor sit amet, consectetur adipiscing "
"elit. Sed vel urna neque. Ut quis leo metus. Quisque eleifend, ex at "
"laoreet rhoncus, odio ipsum semper metus, at tempus ante urna in mauris. "
"Suspendisse ornare tempor venenatis. Ut dui neque, pellentesque a varius "
"eget, mattis vitae ligula. Fusce ut pharetra est. Ut ullamcorper mi ac "
"sollicitudin semper. Praesent sit amet tellus varius, posuere nulla non, "
"rhoncus ipsum.";
struct WriteThis {
const char *readptr;
size_t sizeleft;
};
static size_t read_callback(void *dest, size_t size, size_t nmemb, void *userp)
{
struct WriteThis *wt = (struct WriteThis *)userp;
size_t buffer_size = size*nmemb;
if(wt->sizeleft) {
size_t copy_this_much = wt->sizeleft;
if(copy_this_much > buffer_size)
copy_this_much = buffer_size;
memcpy(dest, wt->readptr, copy_this_much);
wt->readptr += copy_this_much;
wt->sizeleft -= copy_this_much;
return copy_this_much; 
}
return 0; 
}
int main(void)
{
CURL *curl;
CURLcode res;
struct WriteThis wt;
wt.readptr = data;
wt.sizeleft = strlen(data);
res = curl_global_init(CURL_GLOBAL_DEFAULT);
if(res != CURLE_OK) {
fprintf(stderr, "curl_global_init() failed: %s\n",
curl_easy_strerror(res));
return 1;
}
curl = curl_easy_init();
if(curl) {
curl_easy_setopt(curl, CURLOPT_URL, "https://example.com/index.cgi");
curl_easy_setopt(curl, CURLOPT_POST, 1L);
curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);
curl_easy_setopt(curl, CURLOPT_READDATA, &wt);
curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
#if defined(USE_CHUNKED)
{
struct curl_slist *chunk = NULL;
chunk = curl_slist_append(chunk, "Transfer-Encoding: chunked");
res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
}
#else
curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)wt.sizeleft);
#endif
#if defined(DISABLE_EXPECT)
{
struct curl_slist *chunk = NULL;
chunk = curl_slist_append(chunk, "Expect:");
res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
}
#endif
res = curl_easy_perform(curl);
if(res != CURLE_OK)
fprintf(stderr, "curl_easy_perform() failed: %s\n",
curl_easy_strerror(res));
curl_easy_cleanup(curl);
}
curl_global_cleanup();
return 0;
}
