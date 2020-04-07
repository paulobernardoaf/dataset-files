#include "test.h"
#include "memdebug.h"
static char buffer[17000]; 
int test(char *URL)
{
CURL *curl;
CURLcode res = CURLE_OK;
CURLFORMcode formrc;
struct curl_httppost *formpost = NULL;
struct curl_httppost *lastptr = NULL;
int i;
int size = (int)sizeof(buffer)/1000;
for(i = 0; i < size ; i++)
memset(&buffer[i * 1000], 65 + i, 1000);
buffer[ sizeof(buffer)-1] = 0; 
if(curl_global_init(CURL_GLOBAL_ALL) != CURLE_OK) {
fprintf(stderr, "curl_global_init() failed\n");
return TEST_ERR_MAJOR_BAD;
}
formrc = curl_formadd(&formpost, &lastptr,
CURLFORM_COPYNAME, "hello",
CURLFORM_COPYCONTENTS, buffer,
CURLFORM_END);
if(formrc)
printf("curl_formadd(1) = %d\n", (int) formrc);
curl = curl_easy_init();
if(!curl) {
fprintf(stderr, "curl_easy_init() failed\n");
curl_formfree(formpost);
curl_global_cleanup();
return TEST_ERR_MAJOR_BAD;
}
test_setopt(curl, CURLOPT_URL, URL);
test_setopt(curl, CURLOPT_HTTPPOST, formpost);
test_setopt(curl, CURLOPT_VERBOSE, 1L);
test_setopt(curl, CURLOPT_HEADER, 1L);
res = curl_easy_perform(curl);
test_cleanup:
curl_easy_cleanup(curl);
curl_formfree(formpost);
curl_global_cleanup();
return res;
}
