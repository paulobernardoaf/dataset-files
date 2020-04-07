




















#include "test.h"

#include "testtrace.h"
#include "memdebug.h"

int test(char *URL)
{
CURLcode res;
CURL *curl;

if(curl_global_init(CURL_GLOBAL_ALL) != CURLE_OK) {
fprintf(stderr, "curl_global_init() failed\n");
return TEST_ERR_MAJOR_BAD;
}

curl = curl_easy_init();
if(!curl) {
fprintf(stderr, "curl_easy_init() failed\n");
curl_global_cleanup();
return TEST_ERR_MAJOR_BAD;
}

test_setopt(curl, CURLOPT_URL, URL);
test_setopt(curl, CURLOPT_HEADER, 1L);
test_setopt(curl, CURLOPT_BUFFERSIZE, 1L); 

res = curl_easy_perform(curl);
test_cleanup:

curl_easy_cleanup(curl);
curl_global_cleanup();

return (int)res;
}
