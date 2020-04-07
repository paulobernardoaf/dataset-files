
























#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <curl/curl.h>

static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
return written;
}

int main(void)
{
CURL *curl_handle;
static const char *headerfilename = "head.out";
FILE *headerfile;
static const char *bodyfilename = "body.out";
FILE *bodyfile;

curl_global_init(CURL_GLOBAL_ALL);


curl_handle = curl_easy_init();


curl_easy_setopt(curl_handle, CURLOPT_URL, "https://example.com");


curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L);


curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_data);


headerfile = fopen(headerfilename, "wb");
if(!headerfile) {
curl_easy_cleanup(curl_handle);
return -1;
}


bodyfile = fopen(bodyfilename, "wb");
if(!bodyfile) {
curl_easy_cleanup(curl_handle);
fclose(headerfile);
return -1;
}


curl_easy_setopt(curl_handle, CURLOPT_HEADERDATA, headerfile);


curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, bodyfile);


curl_easy_perform(curl_handle);


fclose(headerfile);


fclose(bodyfile);


curl_easy_cleanup(curl_handle);

return 0;
}
