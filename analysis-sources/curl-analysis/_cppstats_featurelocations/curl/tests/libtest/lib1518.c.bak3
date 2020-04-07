




















#include "test.h"

#include "memdebug.h"



int test(char *URL)
{
CURL *curl;
CURLcode res = CURLE_OK;
long curlResponseCode;
long curlRedirectCount;
char *effectiveUrl = NULL;
char *redirectUrl = NULL;

curl = curl_easy_init();
if(!curl) {
fprintf(stderr, "curl_easy_init() failed\n");
curl_global_cleanup();
return TEST_ERR_MAJOR_BAD;
}

test_setopt(curl, CURLOPT_URL, URL);

test_setopt(curl, CURLOPT_FOLLOWLOCATION, 0L);


res = curl_easy_perform(curl);

curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &curlResponseCode);
curl_easy_getinfo(curl, CURLINFO_REDIRECT_COUNT, &curlRedirectCount);
curl_easy_getinfo(curl, CURLINFO_EFFECTIVE_URL, &effectiveUrl);
curl_easy_getinfo(curl, CURLINFO_REDIRECT_URL, &redirectUrl);

printf("res: %d\n"
"status: %d\n"
"redirects: %d\n"
"effectiveurl: %s\n"
"redirecturl: %s\n",
(int)res,
(int)curlResponseCode,
(int)curlRedirectCount,
effectiveUrl,
redirectUrl);

test_cleanup:


curl_easy_cleanup(curl);
curl_global_cleanup();

return res;
}
