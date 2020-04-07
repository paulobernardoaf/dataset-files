#include "test.h"
#include "memdebug.h"
#if defined(CURL_DOES_CONVERSIONS)
#define UPLOADTHIS "\x74\x68\x69\x73\x20\x69\x73\x20\x74\x68\x65\x20\x62" "\x6c\x75\x72\x62\x20\x77\x65\x20\x77\x61\x6e\x74\x20" "\x74\x6f\x20\x75\x70\x6c\x6f\x61\x64\x0a"
#else
#define UPLOADTHIS "this is the blurb we want to upload\n"
#endif
#if !defined(LIB548)
static size_t readcallback(void *ptr,
size_t size,
size_t nmemb,
void *clientp)
{
int *counter = (int *)clientp;
if(*counter) {
fprintf(stderr, "READ ALREADY DONE!\n");
return 0;
}
(*counter)++; 
if(size * nmemb > strlen(UPLOADTHIS)) {
fprintf(stderr, "READ!\n");
strcpy(ptr, UPLOADTHIS);
return strlen(UPLOADTHIS);
}
fprintf(stderr, "READ NOT FINE!\n");
return 0;
}
static curlioerr ioctlcallback(CURL *handle,
int cmd,
void *clientp)
{
int *counter = (int *)clientp;
(void)handle; 
if(cmd == CURLIOCMD_RESTARTREAD) {
fprintf(stderr, "REWIND!\n");
*counter = 0; 
}
return CURLIOE_OK;
}
#endif
int test(char *URL)
{
CURLcode res;
CURL *curl;
#if !defined(LIB548)
int counter = 0;
#endif
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
test_setopt(curl, CURLOPT_VERBOSE, 1L);
test_setopt(curl, CURLOPT_HEADER, 1L);
#if defined(LIB548)
test_setopt(curl, CURLOPT_POSTFIELDS, UPLOADTHIS);
#else
test_setopt(curl, CURLOPT_IOCTLFUNCTION, ioctlcallback);
test_setopt(curl, CURLOPT_IOCTLDATA, &counter);
test_setopt(curl, CURLOPT_READFUNCTION, readcallback);
test_setopt(curl, CURLOPT_READDATA, &counter);
test_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)strlen(UPLOADTHIS));
#endif
test_setopt(curl, CURLOPT_POST, 1L);
test_setopt(curl, CURLOPT_PROXY, libtest_arg2);
test_setopt(curl, CURLOPT_PROXYUSERPWD, libtest_arg3);
test_setopt(curl, CURLOPT_PROXYAUTH,
(long) (CURLAUTH_NTLM | CURLAUTH_DIGEST | CURLAUTH_BASIC) );
res = curl_easy_perform(curl);
test_cleanup:
curl_easy_cleanup(curl);
curl_global_cleanup();
return (int)res;
}
