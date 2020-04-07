





















#include "test.h"

#include "memdebug.h"

static curl_socket_t opensocket(void *clientp,
curlsocktype purpose,
struct curl_sockaddr *address)
{
(void)purpose;
(void)address;
(void)clientp;
fprintf(stderr, "opensocket() returns CURL_SOCKET_BAD\n");
return CURL_SOCKET_BAD;
}

int test(char *URL)
{
CURL *curl = NULL;
CURLcode res = CURLE_FAILED_INIT;
(void)URL;

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

test_setopt(curl, CURLOPT_URL, "http://99.99.99.99:9999");
test_setopt(curl, CURLOPT_VERBOSE, 1L);
test_setopt(curl, CURLOPT_OPENSOCKETFUNCTION, opensocket);

res = curl_easy_perform(curl);

test_cleanup:

curl_easy_cleanup(curl);
curl_global_cleanup();

return (int)res;
}
