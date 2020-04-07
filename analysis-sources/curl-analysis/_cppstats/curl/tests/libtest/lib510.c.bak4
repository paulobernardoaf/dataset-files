




















#include "test.h"

#include "memdebug.h"

static const char *post[]={
"one",
"two",
"three",
"and a final longer crap: four",
NULL
};


struct WriteThis {
int counter;
};

static size_t read_callback(void *ptr, size_t size, size_t nmemb, void *userp)
{
struct WriteThis *pooh = (struct WriteThis *)userp;
const char *data;

if(size*nmemb < 1)
return 0;

data = post[pooh->counter];

if(data) {
size_t len = strlen(data);
if(size*nmemb < len) {
fprintf(stderr, "read buffer is too small to run test\n");
return 0;
}
memcpy(ptr, data, len);
pooh->counter++; 
return len;
}
return 0; 
}

int test(char *URL)
{
CURL *curl;
CURLcode res = CURLE_OK;
struct curl_slist *slist = NULL;
struct WriteThis pooh;
pooh.counter = 0;

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

slist = curl_slist_append(slist, "Transfer-Encoding: chunked");
if(slist == NULL) {
fprintf(stderr, "curl_slist_append() failed\n");
curl_easy_cleanup(curl);
curl_global_cleanup();
return TEST_ERR_MAJOR_BAD;
}


test_setopt(curl, CURLOPT_URL, URL);


test_setopt(curl, CURLOPT_POST, 1L);

#if defined(CURL_DOES_CONVERSIONS)

test_setopt(curl, CURLOPT_TRANSFERTEXT, 1L);
#endif


test_setopt(curl, CURLOPT_READFUNCTION, read_callback);


test_setopt(curl, CURLOPT_READDATA, &pooh);


test_setopt(curl, CURLOPT_VERBOSE, 1L);


test_setopt(curl, CURLOPT_HEADER, 1L);


test_setopt(curl, CURLOPT_HTTPHEADER, slist);

#if defined(LIB565)
test_setopt(curl, CURLOPT_HTTPAUTH, (long)CURLAUTH_DIGEST);
test_setopt(curl, CURLOPT_USERPWD, "foo:bar");
#endif


res = curl_easy_perform(curl);

test_cleanup:


if(slist)
curl_slist_free_all(slist);


curl_easy_cleanup(curl);
curl_global_cleanup();

return res;
}
