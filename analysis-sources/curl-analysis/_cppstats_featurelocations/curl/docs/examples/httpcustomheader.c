
























#include <stdio.h>
#include <curl/curl.h>

int main(void)
{
CURL *curl;
CURLcode res;

curl = curl_easy_init();
if(curl) {
struct curl_slist *chunk = NULL;


chunk = curl_slist_append(chunk, "Accept:");


chunk = curl_slist_append(chunk, "Another: yes");


chunk = curl_slist_append(chunk, "Host: example.com");



chunk = curl_slist_append(chunk, "X-silly-header;");


curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

curl_easy_setopt(curl, CURLOPT_URL, "localhost");
curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

res = curl_easy_perform(curl);

if(res != CURLE_OK)
fprintf(stderr, "curl_easy_perform() failed: %s\n",
curl_easy_strerror(res));


curl_easy_cleanup(curl);


curl_slist_free_all(chunk);
}
return 0;
}
