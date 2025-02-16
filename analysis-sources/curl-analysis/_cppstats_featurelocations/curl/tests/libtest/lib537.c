




















#include "test.h"

#if defined(HAVE_SYS_RESOURCE_H)
#include <sys/resource.h>
#endif
#if defined(HAVE_FCNTL_H)
#include <fcntl.h>
#endif
#include <limits.h>

#include "warnless.h"
#include "memdebug.h"

#if !defined(HAVE_POLL_FINE) && !defined(USE_WINSOCK) && !defined(TPF) && !defined(FD_SETSIZE)



#error "this test requires FD_SETSIZE"
#endif

#define SAFETY_MARGIN (11)

#if defined(WIN32) || defined(_WIN32) || defined(MSDOS)
#define DEV_NULL "NUL"
#else
#define DEV_NULL "/dev/null"
#endif

#if defined(HAVE_GETRLIMIT) && defined(HAVE_SETRLIMIT)

static int *fd = NULL;
static struct rlimit num_open;
static char msgbuff[256];

static void store_errmsg(const char *msg, int err)
{
if(!err)
msnprintf(msgbuff, sizeof(msgbuff), "%s", msg);
else
msnprintf(msgbuff, sizeof(msgbuff), "%s, errno %d, %s", msg, err,
strerror(err));
}

static void close_file_descriptors(void)
{
for(num_open.rlim_cur = 0;
num_open.rlim_cur < num_open.rlim_max;
num_open.rlim_cur++)
if(fd[num_open.rlim_cur] > 0)
close(fd[num_open.rlim_cur]);
free(fd);
fd = NULL;
}

static int fopen_works(void)
{
FILE *fpa[3];
int i;
int ret = 1;

for(i = 0; i < 3; i++) {
fpa[i] = NULL;
}
for(i = 0; i < 3; i++) {
fpa[i] = fopen(DEV_NULL, FOPEN_READTEXT);
if(fpa[i] == NULL) {
store_errmsg("fopen failed", errno);
fprintf(stderr, "%s\n", msgbuff);
ret = 0;
break;
}
}
for(i = 0; i < 3; i++) {
if(fpa[i] != NULL)
fclose(fpa[i]);
}
return ret;
}

static int rlimit(int keep_open)
{
int *tmpfd;
rlim_t nitems, i;
int *memchunk = NULL;
char *fmt;
struct rlimit rl;
char strbuff[256];
char strbuff1[81];
char fmt_u[] = "%u";
char fmt_lu[] = "%lu";
#if defined(HAVE_LONGLONG)
char fmt_llu[] = "%llu";

if(sizeof(rl.rlim_max) > sizeof(long))
fmt = fmt_llu;
else
#endif
fmt = (sizeof(rl.rlim_max) < sizeof(long))?fmt_u:fmt_lu;



if(getrlimit(RLIMIT_NOFILE, &rl) != 0) {
store_errmsg("getrlimit() failed", errno);
fprintf(stderr, "%s\n", msgbuff);
return -1;
}



#if defined(RLIM_INFINITY)
if(rl.rlim_cur == RLIM_INFINITY)
strcpy(strbuff, "INFINITY");
else
#endif
msnprintf(strbuff, sizeof(strbuff), fmt, rl.rlim_cur);
fprintf(stderr, "initial soft limit: %s\n", strbuff);

#if defined(RLIM_INFINITY)
if(rl.rlim_max == RLIM_INFINITY)
strcpy(strbuff, "INFINITY");
else
#endif
msnprintf(strbuff, sizeof(strbuff), fmt, rl.rlim_max);
fprintf(stderr, "initial hard limit: %s\n", strbuff);










if(rl.rlim_cur != rl.rlim_max) {

#if defined(OPEN_MAX)
if((rl.rlim_cur > 0) &&
(rl.rlim_cur < OPEN_MAX)) {
fprintf(stderr, "raising soft limit up to OPEN_MAX\n");
rl.rlim_cur = OPEN_MAX;
if(setrlimit(RLIMIT_NOFILE, &rl) != 0) {

store_errmsg("setrlimit() failed", errno);
fprintf(stderr, "%s\n", msgbuff);
msgbuff[0] = '\0';
}
}
#endif

fprintf(stderr, "raising soft limit up to hard limit\n");
rl.rlim_cur = rl.rlim_max;
if(setrlimit(RLIMIT_NOFILE, &rl) != 0) {

store_errmsg("setrlimit() failed", errno);
fprintf(stderr, "%s\n", msgbuff);
msgbuff[0] = '\0';
}



if(getrlimit(RLIMIT_NOFILE, &rl) != 0) {
store_errmsg("getrlimit() failed", errno);
fprintf(stderr, "%s\n", msgbuff);
return -3;
}



#if defined(RLIM_INFINITY)
if(rl.rlim_cur == RLIM_INFINITY)
strcpy(strbuff, "INFINITY");
else
#endif
msnprintf(strbuff, sizeof(strbuff), fmt, rl.rlim_cur);
fprintf(stderr, "current soft limit: %s\n", strbuff);

#if defined(RLIM_INFINITY)
if(rl.rlim_max == RLIM_INFINITY)
strcpy(strbuff, "INFINITY");
else
#endif
msnprintf(strbuff, sizeof(strbuff), fmt, rl.rlim_max);
fprintf(stderr, "current hard limit: %s\n", strbuff);

} 




















for(nitems = i = 1; nitems <= i; i *= 2)
nitems = i;
if(nitems > 0x7fff)
nitems = 0x40000;
do {
num_open.rlim_max = sizeof(*memchunk) * nitems;
msnprintf(strbuff, sizeof(strbuff), fmt, num_open.rlim_max);
fprintf(stderr, "allocating memchunk %s byte array\n", strbuff);
memchunk = malloc(sizeof(*memchunk) * (size_t)nitems);
if(!memchunk) {
fprintf(stderr, "memchunk, malloc() failed\n");
nitems /= 2;
}
} while(nitems && !memchunk);
if(!memchunk) {
store_errmsg("memchunk, malloc() failed", errno);
fprintf(stderr, "%s\n", msgbuff);
return -4;
}



fprintf(stderr, "initializing memchunk array\n");

for(i = 0; i < nitems; i++)
memchunk[i] = -1;



#if defined(RLIM_INFINITY)
if((rl.rlim_cur > 0) && (rl.rlim_cur != RLIM_INFINITY)) {
#else
if(rl.rlim_cur > 0) {
#endif

num_open.rlim_max = rl.rlim_cur - SAFETY_MARGIN;
}
else {

for(nitems = i = 1; nitems <= i; i *= 2)
nitems = i;
if(nitems > 0x7fff)
nitems = 0x40000;
num_open.rlim_max = nitems;
}



if((size_t)(num_open.rlim_max) > ((size_t)-1) / sizeof(*fd)) {
msnprintf(strbuff1, sizeof(strbuff1), fmt, num_open.rlim_max);
msnprintf(strbuff, sizeof(strbuff), "unable to allocate an array for %s "
"file descriptors, would overflow size_t", strbuff1);
store_errmsg(strbuff, 0);
fprintf(stderr, "%s\n", msgbuff);
free(memchunk);
return -5;
}



do {
msnprintf(strbuff, sizeof(strbuff), fmt, num_open.rlim_max);
fprintf(stderr, "allocating array for %s file descriptors\n", strbuff);
fd = malloc(sizeof(*fd) * (size_t)(num_open.rlim_max));
if(!fd) {
fprintf(stderr, "fd, malloc() failed\n");
num_open.rlim_max /= 2;
}
} while(num_open.rlim_max && !fd);
if(!fd) {
store_errmsg("fd, malloc() failed", errno);
fprintf(stderr, "%s\n", msgbuff);
free(memchunk);
return -6;
}



fprintf(stderr, "initializing fd array\n");

for(num_open.rlim_cur = 0;
num_open.rlim_cur < num_open.rlim_max;
num_open.rlim_cur++)
fd[num_open.rlim_cur] = -1;

msnprintf(strbuff, sizeof(strbuff), fmt, num_open.rlim_max);
fprintf(stderr, "trying to open %s file descriptors\n", strbuff);



fd[0] = open(DEV_NULL, O_RDONLY);
if(fd[0] < 0) {
msnprintf(strbuff, sizeof(strbuff), "opening of %s failed", DEV_NULL);
store_errmsg(strbuff, errno);
fprintf(stderr, "%s\n", msgbuff);
free(fd);
fd = NULL;
free(memchunk);
return -7;
}



for(num_open.rlim_cur = 1;
num_open.rlim_cur < num_open.rlim_max;
num_open.rlim_cur++) {

fd[num_open.rlim_cur] = dup(fd[0]);

if(fd[num_open.rlim_cur] < 0) {

fd[num_open.rlim_cur] = -1;

msnprintf(strbuff1, sizeof(strbuff1), fmt, num_open.rlim_cur);
msnprintf(strbuff, sizeof(strbuff), "dup() attempt %s failed", strbuff1);
fprintf(stderr, "%s\n", strbuff);

msnprintf(strbuff1, sizeof(strbuff1), fmt, num_open.rlim_cur);
msnprintf(strbuff, sizeof(strbuff), "fds system limit seems close to %s",
strbuff1);
fprintf(stderr, "%s\n", strbuff);

num_open.rlim_max = num_open.rlim_cur - SAFETY_MARGIN;

num_open.rlim_cur -= num_open.rlim_max;
msnprintf(strbuff1, sizeof(strbuff1), fmt, num_open.rlim_cur);
msnprintf(strbuff, sizeof(strbuff), "closing %s file descriptors",
strbuff1);
fprintf(stderr, "%s\n", strbuff);

for(num_open.rlim_cur = num_open.rlim_max;
fd[num_open.rlim_cur] >= 0;
num_open.rlim_cur++) {
close(fd[num_open.rlim_cur]);
fd[num_open.rlim_cur] = -1;
}

msnprintf(strbuff, sizeof(strbuff), fmt, num_open.rlim_max);
fprintf(stderr, "shrinking array for %s file descriptors\n", strbuff);



tmpfd = realloc(fd, sizeof(*fd) * (size_t)(num_open.rlim_max));
if(tmpfd) {
fd = tmpfd;
tmpfd = NULL;
}

break;

}

}

msnprintf(strbuff, sizeof(strbuff), fmt, num_open.rlim_max);
fprintf(stderr, "%s file descriptors open\n", strbuff);

#if !defined(HAVE_POLL_FINE) && !defined(USE_WINSOCK) && !defined(TPF)














num_open.rlim_cur = FD_SETSIZE - SAFETY_MARGIN;
if(num_open.rlim_max > num_open.rlim_cur) {
msnprintf(strbuff, sizeof(strbuff), "select limit is FD_SETSIZE %d",
FD_SETSIZE);
store_errmsg(strbuff, 0);
fprintf(stderr, "%s\n", msgbuff);
close_file_descriptors();
free(memchunk);
return -8;
}

num_open.rlim_cur = FD_SETSIZE - SAFETY_MARGIN;
for(rl.rlim_cur = 0;
rl.rlim_cur < num_open.rlim_max;
rl.rlim_cur++) {
if((fd[rl.rlim_cur] > 0) &&
((unsigned int)fd[rl.rlim_cur] > num_open.rlim_cur)) {
msnprintf(strbuff, sizeof(strbuff), "select limit is FD_SETSIZE %d",
FD_SETSIZE);
store_errmsg(strbuff, 0);
fprintf(stderr, "%s\n", msgbuff);
close_file_descriptors();
free(memchunk);
return -9;
}
}

#endif 










if(!fopen_works()) {
msnprintf(strbuff1, sizeof(strbuff1), fmt, num_open.rlim_max);
msnprintf(strbuff, sizeof(strbuff), "fopen fails with %s fds open",
strbuff1);
fprintf(stderr, "%s\n", msgbuff);
msnprintf(strbuff, sizeof(strbuff), "fopen fails with lots of fds open");
store_errmsg(strbuff, 0);
close_file_descriptors();
free(memchunk);
return -10;
}




free(memchunk);



if(!keep_open) {
close_file_descriptors();
}

return 0;
}

int test(char *URL)
{
CURLcode res;
CURL *curl;

if(!strcmp(URL, "check")) {

if(rlimit(FALSE)) {
fprintf(stdout, "rlimit problem: %s\n", msgbuff);
return 1;
}
return 0; 
}

if(rlimit(TRUE)) {

return TEST_ERR_MAJOR_BAD;
}




if(curl_global_init(CURL_GLOBAL_ALL) != CURLE_OK) {
fprintf(stderr, "curl_global_init() failed\n");
close_file_descriptors();
return TEST_ERR_MAJOR_BAD;
}

curl = curl_easy_init();
if(!curl) {
fprintf(stderr, "curl_easy_init() failed\n");
close_file_descriptors();
curl_global_cleanup();
return TEST_ERR_MAJOR_BAD;
}

test_setopt(curl, CURLOPT_URL, URL);
test_setopt(curl, CURLOPT_HEADER, 1L);

res = curl_easy_perform(curl);

test_cleanup:

close_file_descriptors();
curl_easy_cleanup(curl);
curl_global_cleanup();

return (int)res;
}

#else 

int test(char *URL)
{
(void)URL;
printf("system lacks necessary system function(s)");
return 1; 
}

#endif 
