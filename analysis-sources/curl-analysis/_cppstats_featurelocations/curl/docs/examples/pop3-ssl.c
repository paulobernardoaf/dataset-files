


























#include <stdio.h>
#include <curl/curl.h>








int main(void)
{
CURL *curl;
CURLcode res = CURLE_OK;

curl = curl_easy_init();
if(curl) {

curl_easy_setopt(curl, CURLOPT_USERNAME, "user");
curl_easy_setopt(curl, CURLOPT_PASSWORD, "secret");



curl_easy_setopt(curl, CURLOPT_URL, "pop3s://pop.example.com/1");









#if defined(SKIP_PEER_VERIFICATION)
curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
#endif





#if defined(SKIP_HOSTNAME_VERIFICATION)
curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
#endif




curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);


res = curl_easy_perform(curl);


if(res != CURLE_OK)
fprintf(stderr, "curl_easy_perform() failed: %s\n",
curl_easy_strerror(res));


curl_easy_cleanup(curl);
}

return (int)res;
}
