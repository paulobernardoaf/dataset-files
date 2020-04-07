#include <stdio.h>
#include <string.h>
#include <curl/curl.h>
int main(void)
{
CURL *curl;
CURLcode res;
static const char *postthis = "moo mooo moo moo";
curl = curl_easy_init();
if(curl) {
curl_easy_setopt(curl, CURLOPT_URL, "https://example.com");
curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postthis);
curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)strlen(postthis));
res = curl_easy_perform(curl);
if(res != CURLE_OK)
fprintf(stderr, "curl_easy_perform() failed: %s\n",
curl_easy_strerror(res));
curl_easy_cleanup(curl);
}
return 0;
}
