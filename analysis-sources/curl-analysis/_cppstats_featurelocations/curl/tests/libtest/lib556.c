




















#include "test.h"

#include "warnless.h"
#include "memdebug.h"


#if !defined(STDIN_FILENO)
#define STDIN_FILENO 0
#endif
#if !defined(STDOUT_FILENO)
#define STDOUT_FILENO 1
#endif
#if !defined(STDERR_FILENO)
#define STDERR_FILENO 2
#endif

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
test_setopt(curl, CURLOPT_CONNECT_ONLY, 1L);
test_setopt(curl, CURLOPT_VERBOSE, 1L);

res = curl_easy_perform(curl);

if(!res) {

const char *request =
#if defined(CURL_DOES_CONVERSIONS)

"\x47\x45\x54\x20\x2f\x35\x35\x36\x20\x48\x54\x54\x50\x2f\x31\x2e"
"\x32\x0d\x0a\x48\x6f\x73\x74\x3a\x20\x6e\x69\x6e\x6a\x61\x0d\x0a"
"\x0d\x0a";
#else
"GET /556 HTTP/1.2\r\n"
"Host: ninja\r\n\r\n";
#endif
size_t iolen = 0;

res = curl_easy_send(curl, request, strlen(request), &iolen);

if(!res) {


do {
char buf[1024];

res = curl_easy_recv(curl, buf, sizeof(buf), &iolen);

#if defined(TPF)
sleep(1); 
#endif

if(iolen) {

if(!write(STDOUT_FILENO, buf, iolen))
break;
}

} while((res == CURLE_OK && iolen != 0) || (res == CURLE_AGAIN));
}

if(iolen != 0)
res = TEST_ERR_FAILURE;
}

test_cleanup:

curl_easy_cleanup(curl);
curl_global_cleanup();

return (int)res;
}
