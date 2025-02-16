




















#include "test.h"

#include "memdebug.h"

static char data[]=
#if defined(CURL_DOES_CONVERSIONS)

"\x74\x68\x69\x73\x20\x69\x73\x20\x77\x68\x61\x74\x20\x77\x65\x20\x70"
"\x6f\x73\x74\x20\x74\x6f\x20\x74\x68\x65\x20\x73\x69\x6c\x6c\x79\x20"
"\x77\x65\x62\x20\x73\x65\x72\x76\x65\x72\x0a";
#else
"this is what we post to the silly web server\n";
#endif

struct WriteThis {
char *readptr;
size_t sizeleft;
};

static size_t read_callback(void *ptr, size_t size, size_t nmemb, void *userp)
{
#if defined(LIB587)
(void)ptr;
(void)size;
(void)nmemb;
(void)userp;
return CURL_READFUNC_ABORT;
#else

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
#endif
}

static int once(char *URL, bool oldstyle)
{
CURL *curl;
CURLcode res = CURLE_OK;
CURLFORMcode formrc;

struct curl_httppost *formpost = NULL;
struct curl_httppost *lastptr = NULL;
struct WriteThis pooh;
struct WriteThis pooh2;

pooh.readptr = data;
pooh.sizeleft = strlen(data);


if(oldstyle) {
formrc = curl_formadd(&formpost,
&lastptr,
CURLFORM_COPYNAME, "sendfile",
CURLFORM_STREAM, &pooh,
CURLFORM_CONTENTSLENGTH, (long)pooh.sizeleft,
CURLFORM_FILENAME, "postit2.c",
CURLFORM_END);
}
else {

formrc = curl_formadd(&formpost,
&lastptr,
CURLFORM_COPYNAME, "sendfile alternative",
CURLFORM_STREAM, &pooh,
CURLFORM_CONTENTLEN, (curl_off_t)pooh.sizeleft,
CURLFORM_FILENAME, "file name 2",
CURLFORM_END);
}

if(formrc)
printf("curl_formadd(1) = %d\n", (int)formrc);




pooh2.readptr = data;
pooh2.sizeleft = strlen(data);


formrc = curl_formadd(&formpost,
&lastptr,
CURLFORM_COPYNAME, "callbackdata",
CURLFORM_STREAM, &pooh2,
CURLFORM_CONTENTSLENGTH, (long)pooh2.sizeleft,
CURLFORM_END);

if(formrc)
printf("curl_formadd(2) = %d\n", (int)formrc);


formrc = curl_formadd(&formpost,
&lastptr,
CURLFORM_COPYNAME, "filename",
#if defined(CURL_DOES_CONVERSIONS)


CURLFORM_COPYCONTENTS,
"\x70\x6f\x73\x74\x69\x74\x32\x2e\x63",
#else
CURLFORM_COPYCONTENTS, "postit2.c",
#endif
CURLFORM_END);

if(formrc)
printf("curl_formadd(3) = %d\n", (int)formrc);


formrc = curl_formadd(&formpost,
&lastptr,
CURLFORM_COPYNAME, "submit",
#if defined(CURL_DOES_CONVERSIONS)


CURLFORM_COPYCONTENTS, "\x73\x65\x6e\x64",
#else
CURLFORM_COPYCONTENTS, "send",
#endif
CURLFORM_CONTENTTYPE, "text/plain",
CURLFORM_END);

if(formrc)
printf("curl_formadd(4) = %d\n", (int)formrc);

formrc = curl_formadd(&formpost, &lastptr,
CURLFORM_COPYNAME, "somename",
CURLFORM_BUFFER, "somefile.txt",
CURLFORM_BUFFERPTR, "blah blah",
CURLFORM_BUFFERLENGTH, (long)9,
CURLFORM_END);

if(formrc)
printf("curl_formadd(5) = %d\n", (int)formrc);

curl = curl_easy_init();
if(!curl) {
fprintf(stderr, "curl_easy_init() failed\n");
curl_formfree(formpost);
curl_global_cleanup();
return TEST_ERR_MAJOR_BAD;
}


test_setopt(curl, CURLOPT_URL, URL);


test_setopt(curl, CURLOPT_POST, 1L);


test_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)pooh.sizeleft);


test_setopt(curl, CURLOPT_READFUNCTION, read_callback);


test_setopt(curl, CURLOPT_HTTPPOST, formpost);


test_setopt(curl, CURLOPT_VERBOSE, 1L);


test_setopt(curl, CURLOPT_HEADER, 1L);


res = curl_easy_perform(curl);

test_cleanup:


curl_easy_cleanup(curl);


curl_formfree(formpost);

return res;
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

curl_global_cleanup();

return res;
}
