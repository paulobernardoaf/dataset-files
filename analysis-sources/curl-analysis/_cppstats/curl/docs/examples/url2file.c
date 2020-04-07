#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <curl/curl.h>
static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
return written;
}
int main(int argc, char *argv[])
{
CURL *curl_handle;
static const char *pagefilename = "page.out";
FILE *pagefile;
if(argc < 2) {
printf("Usage: %s <URL>\n", argv[0]);
return 1;
}
curl_global_init(CURL_GLOBAL_ALL);
curl_handle = curl_easy_init();
curl_easy_setopt(curl_handle, CURLOPT_URL, argv[1]);
curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 1L);
curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L);
curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_data);
pagefile = fopen(pagefilename, "wb");
if(pagefile) {
curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, pagefile);
curl_easy_perform(curl_handle);
fclose(pagefile);
}
curl_easy_cleanup(curl_handle);
curl_global_cleanup();
return 0;
}
