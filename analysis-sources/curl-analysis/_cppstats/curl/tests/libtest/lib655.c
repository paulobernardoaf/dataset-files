#include "test.h"
#include "memdebug.h"
static const char *TEST_DATA_STRING = "Test data";
static int cb_count = 0;
static int
resolver_alloc_cb_fail(void *resolver_state, void *reserved, void *userdata)
{
(void)resolver_state;
(void)reserved;
cb_count++;
if(strcmp(userdata, TEST_DATA_STRING)) {
fprintf(stderr, "Invalid test data received");
exit(1);
}
return 1;
}
static int
resolver_alloc_cb_pass(void *resolver_state, void *reserved, void *userdata)
{
(void)resolver_state;
(void)reserved;
cb_count++;
if(strcmp(userdata, TEST_DATA_STRING)) {
fprintf(stderr, "Invalid test data received");
exit(1);
}
return 0;
}
int test(char *URL)
{
CURL *curl;
CURLcode res = CURLE_OK;
if(curl_global_init(CURL_GLOBAL_ALL) != CURLE_OK) {
fprintf(stderr, "curl_global_init() failed\n");
return TEST_ERR_MAJOR_BAD;
}
curl = curl_easy_init();
if(!curl) {
fprintf(stderr, "curl_easy_init() failed\n");
res = TEST_ERR_MAJOR_BAD;
goto test_cleanup;
}
test_setopt(curl, CURLOPT_URL, URL);
test_setopt(curl, CURLOPT_RESOLVER_START_DATA, TEST_DATA_STRING);
test_setopt(curl, CURLOPT_RESOLVER_START_FUNCTION, resolver_alloc_cb_fail);
res = curl_easy_perform(curl);
if(res != CURLE_COULDNT_RESOLVE_HOST) {
fprintf(stderr, "curl_easy_perform should have returned "
"CURLE_COULDNT_RESOLVE_HOST but instead returned error %d\n", res);
if(res == CURLE_OK)
res = TEST_ERR_FAILURE;
goto test_cleanup;
}
test_setopt(curl, CURLOPT_RESOLVER_START_FUNCTION, resolver_alloc_cb_pass);
res = curl_easy_perform(curl);
if(res) {
fprintf(stderr, "curl_easy_perform failed.\n");
goto test_cleanup;
}
if(cb_count != 2) {
fprintf(stderr, "Unexpected number of callbacks: %d\n", cb_count);
res = TEST_ERR_FAILURE;
goto test_cleanup;
}
test_cleanup:
curl_easy_cleanup(curl);
curl_global_cleanup();
return (int)res;
}
