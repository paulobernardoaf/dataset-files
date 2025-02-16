




















#include "test.h"

#if defined(HAVE_LOCALE_H)
#include <locale.h> 
#endif

#if defined(HAVE_IO_H)
#include <io.h> 
#endif

#if defined(HAVE_FCNTL_H)
#include <fcntl.h> 
#endif

#if defined(USE_NSS)
#include <nspr.h>
#endif

#if defined(CURLDEBUG)
#define MEMDEBUG_NODEFINES
#include "memdebug.h"
#endif

int select_wrapper(int nfds, fd_set *rd, fd_set *wr, fd_set *exc,
struct timeval *tv)
{
if(nfds < 0) {
SET_SOCKERRNO(EINVAL);
return -1;
}
#if defined(USE_WINSOCK)





if(!nfds) {
Sleep((1000*tv->tv_sec) + (DWORD)(((double)tv->tv_usec)/1000.0));
return 0;
}
#endif
return select(nfds, rd, wr, exc, tv);
}

void wait_ms(int ms)
{
struct timeval t;
t.tv_sec = ms/1000;
ms -= (int)t.tv_sec * 1000;
t.tv_usec = ms * 1000;
select_wrapper(0, NULL, NULL, NULL, &t);
}

char *libtest_arg2 = NULL;
char *libtest_arg3 = NULL;
int test_argc;
char **test_argv;

struct timeval tv_test_start; 

#if defined(UNITTESTS)
int unitfail; 
#endif

#if defined(CURLDEBUG)
static void memory_tracking_init(void)
{
char *env;

env = curl_getenv("CURL_MEMDEBUG");
if(env) {

char fname[CURL_MT_LOGFNAME_BUFSIZE];
if(strlen(env) >= CURL_MT_LOGFNAME_BUFSIZE)
env[CURL_MT_LOGFNAME_BUFSIZE-1] = '\0';
strcpy(fname, env);
curl_free(env);
curl_dbg_memdebug(fname);



}

env = curl_getenv("CURL_MEMLIMIT");
if(env) {
char *endptr;
long num = strtol(env, &endptr, 10);
if((endptr != env) && (endptr == env + strlen(env)) && (num > 0))
curl_dbg_memlimit(num);
curl_free(env);
}
}
#else
#define memory_tracking_init() Curl_nop_stmt
#endif


char *hexdump(const unsigned char *buffer, size_t len)
{
static char dump[200 * 3 + 1];
char *p = dump;
size_t i;
if(len > 200)
return NULL;
for(i = 0; i<len; i++, p += 3)
msnprintf(p, 4, "%02x ", buffer[i]);
return dump;
}


int main(int argc, char **argv)
{
char *URL;
int result;

#if defined(O_BINARY)
#if defined(__HIGHC__)
_setmode(stdout, O_BINARY);
#else
setmode(fileno(stdout), O_BINARY);
#endif
#endif

memory_tracking_init();






#if defined(HAVE_SETLOCALE)
setlocale(LC_ALL, "");
#endif

if(argc< 2) {
fprintf(stderr, "Pass URL as argument please\n");
return 1;
}

test_argc = argc;
test_argv = argv;

if(argc>2)
libtest_arg2 = argv[2];

if(argc>3)
libtest_arg3 = argv[3];

URL = argv[1]; 

fprintf(stderr, "URL: %s\n", URL);

result = test(URL);

#if defined(USE_NSS)
if(PR_Initialized())

PR_Cleanup();
#endif

return result;
}
