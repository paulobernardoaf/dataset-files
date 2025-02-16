




















#include "test.h"

#include "memdebug.h"

static char data[]=
#if defined(CURL_DOES_CONVERSIONS)

"\x64\x75\x6d\x6d\x79\x0a";
#else
"dummy\n";
#endif

struct WriteThis {
char *readptr;
curl_off_t sizeleft;
int freecount;
};

static void free_callback(void *userp)
{
struct WriteThis *pooh = (struct WriteThis *) userp;

pooh->freecount++;
}

static size_t read_callback(char *ptr, size_t size, size_t nmemb, void *userp)
{
struct WriteThis *pooh = (struct WriteThis *)userp;
int eof = !*pooh->readptr;

if(size*nmemb < 1)
return 0;

eof = pooh->sizeleft <= 0;
if(!eof)
pooh->sizeleft--;

if(!eof) {
*ptr = *pooh->readptr; 
pooh->readptr++; 
return 1; 
}

return 0; 
}

int test(char *URL)
{
CURL *easy = NULL;
CURL *easy2 = NULL;
curl_mime *mime = NULL;
curl_mimepart *part;
struct curl_slist *hdrs = NULL;
CURLcode result;
int res = TEST_ERR_FAILURE;
struct WriteThis pooh;






if(curl_global_init(CURL_GLOBAL_ALL) != CURLE_OK) {
fprintf(stderr, "curl_global_init() failed\n");
return TEST_ERR_MAJOR_BAD;
}

easy = curl_easy_init();


test_setopt(easy, CURLOPT_URL, URL);


test_setopt(easy, CURLOPT_VERBOSE, 1L);


test_setopt(easy, CURLOPT_HEADER, 1L);


pooh.readptr = data;
pooh.sizeleft = (curl_off_t) strlen(data);
pooh.freecount = 0;


mime = curl_mime_init(easy);
part = curl_mime_addpart(mime);
curl_mime_data(part, "hello", CURL_ZERO_TERMINATED);
curl_mime_name(part, "greeting");
curl_mime_type(part, "application/X-Greeting");
curl_mime_encoder(part, "base64");
hdrs = curl_slist_append(hdrs, "X-Test-Number: 654");
curl_mime_headers(part, hdrs, TRUE);
part = curl_mime_addpart(mime);
curl_mime_filedata(part, "log/file654.txt");
part = curl_mime_addpart(mime);
curl_mime_data_cb(part, (curl_off_t) -1, read_callback, NULL, free_callback,
&pooh);


test_setopt(easy, CURLOPT_MIMEPOST, mime);


easy2 = curl_easy_duphandle(easy);
if(!easy2) {
fprintf(stderr, "curl_easy_duphandle() failed\n");
res = TEST_ERR_FAILURE;
goto test_cleanup;
}



curl_mime_free(mime);
mime = NULL; 


result = curl_easy_perform(easy);
if(result) {
fprintf(stderr, "curl_easy_perform(original) failed\n");
res = (int) result;
goto test_cleanup;
}



result = curl_easy_perform(easy2);
if(result) {
fprintf(stderr, "curl_easy_perform(duplicated) failed\n");
res = (int) result;
goto test_cleanup;
}




curl_easy_cleanup(easy2);
easy2 = NULL; 

if(pooh.freecount != 2) {
fprintf(stderr, "free_callback() called %d times instead of 2\n",
pooh.freecount);
res = TEST_ERR_FAILURE;
goto test_cleanup;
}

test_cleanup:
curl_easy_cleanup(easy);
curl_easy_cleanup(easy2);
curl_mime_free(mime);
curl_global_cleanup();
return res;
}
