
























#include <stdio.h>
#include <unistd.h>
#include <curl/curl.h>

int main(void)
{
CURL *curl;
CURLcode res;

curl_global_init(CURL_GLOBAL_ALL);

curl = curl_easy_init();
if(curl) {
curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
curl_easy_setopt(curl, CURLOPT_HEADER, 1L);


curl_easy_setopt(curl, CURLOPT_URL, "https://example.com/");


res = curl_easy_perform(curl);

if(res != CURLE_OK)
fprintf(stderr, "curl_easy_perform() failed: %s\n",
curl_easy_strerror(res));



curl_easy_setopt(curl, CURLOPT_URL, "https://example.com/docs/");


res = curl_easy_perform(curl);

if(res != CURLE_OK)
fprintf(stderr, "curl_easy_perform() failed: %s\n",
curl_easy_strerror(res));


curl_easy_cleanup(curl);
}

return 0;
}
