
























#include <stdio.h>
#include <curl/curl.h>

int main(void)
{
CURL *curl;
CURLcode res;
char *location;
long response_code;

curl = curl_easy_init();
if(curl) {
curl_easy_setopt(curl, CURLOPT_URL, "https://example.com");




res = curl_easy_perform(curl);

if(res != CURLE_OK)
fprintf(stderr, "curl_easy_perform() failed: %s\n",
curl_easy_strerror(res));
else {
res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
if((res == CURLE_OK) &&
((response_code / 100) != 3)) {

fprintf(stderr, "Not a redirect.\n");
}
else {
res = curl_easy_getinfo(curl, CURLINFO_REDIRECT_URL, &location);

if((res == CURLE_OK) && location) {


printf("Redirected to: %s\n", location);
}
}
}


curl_easy_cleanup(curl);
}
return 0;
}
