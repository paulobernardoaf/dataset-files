





























#include "test.h"
#include "testutil.h"
#include "warnless.h"
#include "memdebug.h"

#define TEST_HANG_TIMEOUT 60 * 1000

static const char uploadthis[] =
#if defined(CURL_DOES_CONVERSIONS)

"\x74\x68\x69\x73\x20\x69\x73\x20\x74\x68\x65\x20\x62\x6c\x75\x72"
"\x62\x20\x77\x65\x20\x77\x61\x6e\x74\x20\x74\x6f\x20\x75\x70\x6c"
"\x6f\x61\x64\x0a";
#else
"this is the blurb we want to upload\n";
#endif

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

if(size * nmemb > strlen(uploadthis)) {
fprintf(stderr, "READ!\n");
strcpy(ptr, uploadthis);
return strlen(uploadthis);
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


int test(char *URL)
{
int res = 0;
CURL *curl = NULL;
int counter = 0;
CURLM *m = NULL;
int running = 1;

start_test_timing();

global_init(CURL_GLOBAL_ALL);

easy_init(curl);

easy_setopt(curl, CURLOPT_URL, URL);
easy_setopt(curl, CURLOPT_VERBOSE, 1L);
easy_setopt(curl, CURLOPT_HEADER, 1L);


easy_setopt(curl, CURLOPT_IOCTLFUNCTION, ioctlcallback);
easy_setopt(curl, CURLOPT_IOCTLDATA, &counter);
easy_setopt(curl, CURLOPT_READFUNCTION, readcallback);
easy_setopt(curl, CURLOPT_READDATA, &counter);


easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)strlen(uploadthis));

easy_setopt(curl, CURLOPT_POST, 1L);
easy_setopt(curl, CURLOPT_PROXY, libtest_arg2);
easy_setopt(curl, CURLOPT_PROXYUSERPWD, libtest_arg3);
easy_setopt(curl, CURLOPT_PROXYAUTH,
(long) (CURLAUTH_NTLM | CURLAUTH_DIGEST | CURLAUTH_BASIC) );

multi_init(m);

multi_add_handle(m, curl);

while(running) {
struct timeval timeout;
fd_set fdread, fdwrite, fdexcep;
int maxfd = -99;

timeout.tv_sec = 0;
timeout.tv_usec = 100000L; 

multi_perform(m, &running);

abort_on_test_timeout();

#if defined(TPF)
sleep(1); 
#endif

if(!running)
break; 

FD_ZERO(&fdread);
FD_ZERO(&fdwrite);
FD_ZERO(&fdexcep);

multi_fdset(m, &fdread, &fdwrite, &fdexcep, &maxfd);



select_test(maxfd + 1, &fdread, &fdwrite, &fdexcep, &timeout);

abort_on_test_timeout();
}

test_cleanup:



curl_multi_remove_handle(m, curl);
curl_multi_cleanup(m);
curl_easy_cleanup(curl);
curl_global_cleanup();

return res;
}
