#include "test.h"
#include "memdebug.h"
static char teststring[] =
#if defined(CURL_DOES_CONVERSIONS)
{ '\x54', '\x68', '\x69', '\x73', '\x00', '\x20', '\x69', '\x73', '\x20',
'\x74', '\x65', '\x73', '\x74', '\x20', '\x62', '\x69', '\x6e', '\x61',
'\x72', '\x79', '\x20', '\x64', '\x61', '\x74', '\x61', '\x20', '\x77',
'\x69', '\x74', '\x68', '\x20', '\x61', '\x6e', '\x20', '\x65', '\x6d',
'\x62', '\x65', '\x64', '\x64', '\x65', '\x64', '\x20', '\x4e', '\x55',
'\x4c'};
#else
{ 'T', 'h', 'i', 's', '\0', ' ', 'i', 's', ' ', 't', 'e', 's', 't', ' ',
'b', 'i', 'n', 'a', 'r', 'y', ' ', 'd', 'a', 't', 'a', ' ',
'w', 'i', 't', 'h', ' ', 'a', 'n', ' ',
'e', 'm', 'b', 'e', 'd', 'd', 'e', 'd', ' ', 'N', 'U', 'L'};
#endif
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
curl_global_cleanup();
return TEST_ERR_MAJOR_BAD;
}
test_setopt(curl, CURLOPT_URL, URL);
#if defined(LIB545)
test_setopt(curl, CURLOPT_POSTFIELDSIZE, (long) sizeof(teststring));
#endif
test_setopt(curl, CURLOPT_COPYPOSTFIELDS, teststring);
test_setopt(curl, CURLOPT_VERBOSE, 1L); 
test_setopt(curl, CURLOPT_HEADER, 1L); 
strcpy(teststring, "FAIL");
#if defined(LIB545)
{
CURL *handle2;
handle2 = curl_easy_duphandle(curl);
curl_easy_cleanup(curl);
curl = handle2;
}
#endif
res = curl_easy_perform(curl);
test_cleanup:
curl_easy_cleanup(curl);
curl_global_cleanup();
return (int)res;
}
