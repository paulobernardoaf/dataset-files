#include "test.h"
#include "memdebug.h"
static char data[]="dummy";
struct WriteThis {
char *readptr;
size_t sizeleft;
};
static size_t read_callback(void *ptr, size_t size, size_t nmemb, void *userp)
{
struct WriteThis *pooh = (struct WriteThis *)userp;
if(size*nmemb < 1)
return 0;
if(pooh->sizeleft) {
*(char *)ptr = pooh->readptr[0]; 
pooh->readptr++; 
pooh->sizeleft--; 
return 1; 
}
return 0; 
}
int test(char *URL)
{
CURL *curl;
CURLcode result = CURLE_OK;
int res = 0;
struct WriteThis pooh = { data, sizeof(data)-1 };
global_init(CURL_GLOBAL_ALL);
easy_init(curl);
easy_setopt(curl, CURLOPT_URL, URL);
easy_setopt(curl, CURLOPT_POST, 1L);
easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);
easy_setopt(curl, CURLOPT_READDATA, &pooh);
result = curl_easy_perform(curl);
test_cleanup:
curl_easy_cleanup(curl);
curl_global_cleanup();
return (int)result;
}
