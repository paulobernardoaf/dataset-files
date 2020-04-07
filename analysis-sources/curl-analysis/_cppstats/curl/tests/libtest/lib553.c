#include "test.h"
#include "memdebug.h"
#define POSTLEN 40960
static size_t myreadfunc(void *ptr, size_t size, size_t nmemb, void *stream)
{
static size_t total = POSTLEN;
static char buf[1024];
(void)stream;
memset(buf, 'A', sizeof(buf));
size *= nmemb;
if(size > total)
size = total;
if(size > sizeof(buf))
size = sizeof(buf);
memcpy(ptr, buf, size);
total -= size;
return size;
}
#define NUM_HEADERS 8
#define SIZE_HEADERS 5000
static char buf[SIZE_HEADERS + 100];
int test(char *URL)
{
CURL *curl;
CURLcode res = CURLE_FAILED_INIT;
int i;
struct curl_slist *headerlist = NULL, *hl;
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
for(i = 0; i < NUM_HEADERS; i++) {
int len = msnprintf(buf, sizeof(buf), "Header%d: ", i);
memset(&buf[len], 'A', SIZE_HEADERS);
buf[len + SIZE_HEADERS] = 0; 
hl = curl_slist_append(headerlist, buf);
if(!hl)
goto test_cleanup;
headerlist = hl;
}
hl = curl_slist_append(headerlist, "Expect: ");
if(!hl)
goto test_cleanup;
headerlist = hl;
test_setopt(curl, CURLOPT_URL, URL);
test_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
test_setopt(curl, CURLOPT_POST, 1L);
#if defined(CURL_DOES_CONVERSIONS)
test_setopt(curl, CURLOPT_TRANSFERTEXT, 1L);
#endif
test_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)POSTLEN);
test_setopt(curl, CURLOPT_VERBOSE, 1L);
test_setopt(curl, CURLOPT_HEADER, 1L);
test_setopt(curl, CURLOPT_READFUNCTION, myreadfunc);
res = curl_easy_perform(curl);
test_cleanup:
curl_easy_cleanup(curl);
curl_slist_free_all(headerlist);
curl_global_cleanup();
return (int)res;
}
