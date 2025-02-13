




















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
};

static size_t read_callback(char *ptr, size_t size, size_t nmemb, void *userp)
{
#if defined(LIB644)
static int count = 0;
(void)ptr;
(void)size;
(void)nmemb;
(void)userp;
switch(count++) {
case 0: 
*ptr = '\n';
return 1;
case 1: 
return CURL_READFUNC_ABORT;
}
printf("Wrongly called >2 times\n");
exit(1); 
#else

struct WriteThis *pooh = (struct WriteThis *)userp;
int eof = !*pooh->readptr;

if(size*nmemb < 1)
return 0;

#if !defined(LIB645)
eof = pooh->sizeleft <= 0;
if(!eof)
pooh->sizeleft--;
#endif

if(!eof) {
*ptr = *pooh->readptr; 
pooh->readptr++; 
return 1; 
}

return 0; 
#endif
}

static int once(char *URL, bool oldstyle)
{
CURL *curl;
CURLcode res = CURLE_OK;

curl_mime *mime = NULL;
curl_mimepart *part = NULL;
struct WriteThis pooh;
struct WriteThis pooh2;
curl_off_t datasize = -1;

pooh.readptr = data;
#if !defined(LIB645)
datasize = (curl_off_t)strlen(data);
#endif
pooh.sizeleft = datasize;

curl = curl_easy_init();
if(!curl) {
fprintf(stderr, "curl_easy_init() failed\n");
curl_global_cleanup();
return TEST_ERR_MAJOR_BAD;
}

mime = curl_mime_init(curl);
if(!mime) {
fprintf(stderr, "curl_mime_init() failed\n");
curl_easy_cleanup(curl);
curl_global_cleanup();
return TEST_ERR_MAJOR_BAD;
}

part = curl_mime_addpart(mime);
if(!part) {
fprintf(stderr, "curl_mime_addpart(1) failed\n");
curl_mime_free(mime);
curl_easy_cleanup(curl);
curl_global_cleanup();
return TEST_ERR_MAJOR_BAD;
}


if(oldstyle) {
res = curl_mime_name(part, "sendfile");
if(!res)
res = curl_mime_data_cb(part, datasize, read_callback,
NULL, NULL, &pooh);
if(!res)
res = curl_mime_filename(part, "postit2.c");
}
else {

res = curl_mime_name(part, "sendfile alternative");
if(!res)
res = curl_mime_data_cb(part, datasize, read_callback,
NULL, NULL, &pooh);
if(!res)
res = curl_mime_filename(part, "file name 2");
}

if(res)
printf("curl_mime_xxx(1) = %s\n", curl_easy_strerror(res));




pooh2.readptr = data;
#if !defined(LIB645)
datasize = (curl_off_t)strlen(data);
#endif
pooh2.sizeleft = datasize;

part = curl_mime_addpart(mime);
if(!part) {
fprintf(stderr, "curl_mime_addpart(2) failed\n");
curl_mime_free(mime);
curl_easy_cleanup(curl);
curl_global_cleanup();
return TEST_ERR_MAJOR_BAD;
}

res = curl_mime_name(part, "callbackdata");
if(!res)
res = curl_mime_data_cb(part, datasize, read_callback,
NULL, NULL, &pooh2);

if(res)
printf("curl_mime_xxx(2) = %s\n", curl_easy_strerror(res));

part = curl_mime_addpart(mime);
if(!part) {
fprintf(stderr, "curl_mime_addpart(3) failed\n");
curl_mime_free(mime);
curl_easy_cleanup(curl);
curl_global_cleanup();
return TEST_ERR_MAJOR_BAD;
}


res = curl_mime_name(part, "filename");
if(!res)
res = curl_mime_data(part,
#if defined(CURL_DOES_CONVERSIONS)


"\x70\x6f\x73\x74\x69\x74\x32\x2e\x63",
#else
"postit2.c",
#endif
CURL_ZERO_TERMINATED);

if(res)
printf("curl_mime_xxx(3) = %s\n", curl_easy_strerror(res));


part = curl_mime_addpart(mime);
if(!part) {
fprintf(stderr, "curl_mime_addpart(4) failed\n");
curl_mime_free(mime);
curl_easy_cleanup(curl);
curl_global_cleanup();
return TEST_ERR_MAJOR_BAD;
}
res = curl_mime_name(part, "submit");
if(!res)
res = curl_mime_data(part,
#if defined(CURL_DOES_CONVERSIONS)


"\x73\x65\x6e\x64",
#else
"send",
#endif
CURL_ZERO_TERMINATED);

if(res)
printf("curl_mime_xxx(4) = %s\n", curl_easy_strerror(res));

part = curl_mime_addpart(mime);
if(!part) {
fprintf(stderr, "curl_mime_addpart(5) failed\n");
curl_mime_free(mime);
curl_easy_cleanup(curl);
curl_global_cleanup();
return TEST_ERR_MAJOR_BAD;
}
res = curl_mime_name(part, "somename");
if(!res)
res = curl_mime_filename(part, "somefile.txt");
if(!res)
res = curl_mime_data(part, "blah blah", 9);

if(res)
printf("curl_mime_xxx(5) = %s\n", curl_easy_strerror(res));


test_setopt(curl, CURLOPT_URL, URL);


test_setopt(curl, CURLOPT_MIMEPOST, mime);


test_setopt(curl, CURLOPT_VERBOSE, 1L);


test_setopt(curl, CURLOPT_HEADER, 1L);


res = curl_easy_perform(curl);

test_cleanup:


curl_easy_cleanup(curl);


curl_mime_free(mime);

return res;
}

static int cyclic_add(void)
{
CURL *easy = curl_easy_init();
curl_mime *mime = curl_mime_init(easy);
curl_mimepart *part = curl_mime_addpart(mime);
CURLcode a1 = curl_mime_subparts(part, mime);

if(a1 == CURLE_BAD_FUNCTION_ARGUMENT) {
curl_mime *submime = curl_mime_init(easy);
curl_mimepart *subpart = curl_mime_addpart(submime);

curl_mime_subparts(part, submime);
a1 = curl_mime_subparts(subpart, mime);
}

curl_mime_free(mime);
curl_easy_cleanup(easy);
if(a1 != CURLE_BAD_FUNCTION_ARGUMENT)

return 1;

return 0;
}

int test(char *URL)
{
int res;

if(curl_global_init(CURL_GLOBAL_ALL) != CURLE_OK) {
fprintf(stderr, "curl_global_init() failed\n");
return TEST_ERR_MAJOR_BAD;
}

res = once(URL, TRUE); 
if(!res)
res = once(URL, FALSE); 

if(!res)
res = cyclic_add();

curl_global_cleanup();

return res;
}
