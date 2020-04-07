#include "test.h"
#include "memdebug.h"
static char data[]=
#if defined(CURL_DOES_CONVERSIONS)
"\x64\x75\x6d\x6d\x79";
#else
"dummy";
#endif
struct WriteThis {
char *readptr;
curl_off_t sizeleft;
};
static size_t read_callback(char *ptr, size_t size, size_t nmemb, void *userp)
{
struct WriteThis *pooh = (struct WriteThis *)userp;
size_t len = strlen(pooh->readptr);
(void) size; 
if(len > nmemb)
len = nmemb;
if(len) {
memcpy(ptr, pooh->readptr, len);
pooh->readptr += len;
}
return len;
}
int test(char *URL)
{
CURL *easy = NULL;
curl_mime *mime = NULL;
curl_mimepart *part;
CURLcode result;
int res = TEST_ERR_FAILURE;
struct WriteThis pooh1, pooh2;
if(curl_global_init(CURL_GLOBAL_ALL) != CURLE_OK) {
fprintf(stderr, "curl_global_init() failed\n");
return TEST_ERR_MAJOR_BAD;
}
easy = curl_easy_init();
test_setopt(easy, CURLOPT_URL, URL);
test_setopt(easy, CURLOPT_VERBOSE, 1L);
test_setopt(easy, CURLOPT_HEADER, 1L);
pooh1.readptr = data;
pooh1.sizeleft = (curl_off_t) strlen(data);
pooh2 = pooh1;
mime = curl_mime_init(easy);
part = curl_mime_addpart(mime);
curl_mime_name(part, "field1");
curl_mime_data_cb(part, (curl_off_t) strlen(data),
read_callback, NULL, NULL, &pooh1);
part = curl_mime_addpart(mime);
curl_mime_name(part, "field2");
curl_mime_data_cb(part, (curl_off_t) -1, read_callback, NULL, NULL, &pooh2);
part = curl_mime_addpart(mime);
curl_mime_name(part, "field3");
curl_mime_filedata(part, "log/file668.txt");
test_setopt(easy, CURLOPT_MIMEPOST, mime);
result = curl_easy_perform(easy);
if(result) {
fprintf(stderr, "curl_easy_perform() failed\n");
res = (int) result;
}
test_cleanup:
curl_easy_cleanup(easy);
curl_mime_free(mime);
curl_global_cleanup();
return res;
}
