#include "test.h"
#include "memdebug.h"
static char data[]="this is what we post to the silly web server\n";
struct WriteThis {
char *readptr;
size_t sizeleft;
};
static size_t read_callback(void *ptr, size_t size, size_t nmemb, void *userp)
{
struct WriteThis *pooh = (struct WriteThis *)userp;
size_t tocopy = size * nmemb;
wait_ms(1000);
if(tocopy < 1 || !pooh->sizeleft)
return 0;
if(pooh->sizeleft < tocopy)
tocopy = pooh->sizeleft;
memcpy(ptr, pooh->readptr, tocopy);
pooh->readptr += tocopy; 
pooh->sizeleft -= tocopy; 
return tocopy;
}
int test(char *URL)
{
CURL *curl;
CURLcode res = CURLE_OK;
struct WriteThis pooh;
pooh.readptr = data;
pooh.sizeleft = strlen(data);
if(curl_global_init(CURL_GLOBAL_ALL)) {
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
test_setopt(curl, CURLOPT_POST, 1L);
#if defined(CURL_DOES_CONVERSIONS)
test_setopt(curl, CURLOPT_TRANSFERTEXT, 1L);
#endif
test_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)pooh.sizeleft);
test_setopt(curl, CURLOPT_READFUNCTION, read_callback);
test_setopt(curl, CURLOPT_READDATA, &pooh);
test_setopt(curl, CURLOPT_VERBOSE, 1L);
test_setopt(curl, CURLOPT_HEADER, 1L);
res = curl_easy_perform(curl);
test_cleanup:
curl_easy_cleanup(curl);
curl_global_cleanup();
return res;
}
