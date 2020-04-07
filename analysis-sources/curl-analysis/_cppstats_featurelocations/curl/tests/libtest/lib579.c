




















#include "test.h"

#include "memdebug.h"

static const char * const post[]={
"one",
"two",
"three",
"and a final longer crap: four",
NULL
};


struct WriteThis {
int counter;
};

static int progress_callback(void *clientp, double dltotal, double dlnow,
double ultotal, double ulnow)
{
static int prev_ultotal = -1;
static int prev_ulnow = -1;
(void)clientp; 
(void)dltotal; 
(void)dlnow; 





if((prev_ultotal != (int)ultotal) ||
(prev_ulnow != (int)ulnow)) {

FILE *moo = fopen(libtest_arg2, "ab");
if(moo) {
fprintf(moo, "Progress callback called with UL %d out of %d\n",
(int)ulnow, (int)ultotal);
fclose(moo);
}
prev_ulnow = (int) ulnow;
prev_ultotal = (int) ultotal;
}
return 0;
}

static size_t read_callback(void *ptr, size_t size, size_t nmemb, void *userp)
{
struct WriteThis *pooh = (struct WriteThis *)userp;
const char *data;

if(size*nmemb < 1)
return 0;

data = post[pooh->counter];

if(data) {
size_t len = strlen(data);
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

test_setopt(curl, CURLOPT_HTTPAUTH, (long)CURLAUTH_DIGEST);
test_setopt(curl, CURLOPT_USERPWD, "foo:bar");


test_setopt(curl, CURLOPT_NOPROGRESS, 0L);
test_setopt(curl, CURLOPT_PROGRESSFUNCTION, progress_callback);


res = curl_easy_perform(curl);

test_cleanup:


if(slist)
curl_slist_free_all(slist);


curl_easy_cleanup(curl);
curl_global_cleanup();

return res;
}
