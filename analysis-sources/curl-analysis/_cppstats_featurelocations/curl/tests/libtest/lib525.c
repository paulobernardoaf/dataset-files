




















#include "test.h"

#include <fcntl.h>

#include "testutil.h"
#include "warnless.h"
#include "memdebug.h"

#define TEST_HANG_TIMEOUT 60 * 1000

int test(char *URL)
{
int res = 0;
CURL *curl = NULL;
FILE *hd_src = NULL;
int hd;
struct_stat file_info;
CURLM *m = NULL;
int running;

start_test_timing();

if(!libtest_arg2) {
#if defined(LIB529)

fprintf(stderr, "Usage: lib529 [url] [uploadfile]\n");
#else

fprintf(stderr, "Usage: lib525 [url] [uploadfile]\n");
#endif
return TEST_ERR_USAGE;
}

hd_src = fopen(libtest_arg2, "rb");
if(NULL == hd_src) {
fprintf(stderr, "fopen failed with error: %d (%s)\n",
errno, strerror(errno));
fprintf(stderr, "Error opening file: (%s)\n", libtest_arg2);
return TEST_ERR_FOPEN;
}


hd = fstat(fileno(hd_src), &file_info);
if(hd == -1) {

fprintf(stderr, "fstat() failed with error: %d (%s)\n",
errno, strerror(errno));
fprintf(stderr, "ERROR: cannot open file (%s)\n", libtest_arg2);
fclose(hd_src);
return TEST_ERR_FSTAT;
}

res_global_init(CURL_GLOBAL_ALL);
if(res) {
fclose(hd_src);
return res;
}

easy_init(curl);


easy_setopt(curl, CURLOPT_UPLOAD, 1L);


easy_setopt(curl, CURLOPT_URL, URL);


easy_setopt(curl, CURLOPT_VERBOSE, 1L);


easy_setopt(curl, CURLOPT_FTPPORT, "-");


easy_setopt(curl, CURLOPT_READDATA, hd_src);










easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)file_info.st_size);

multi_init(m);

multi_add_handle(m, curl);

for(;;) {
struct timeval interval;
fd_set rd, wr, exc;
int maxfd = -99;

interval.tv_sec = 1;
interval.tv_usec = 0;

multi_perform(m, &running);

abort_on_test_timeout();

if(!running)
break; 

FD_ZERO(&rd);
FD_ZERO(&wr);
FD_ZERO(&exc);

multi_fdset(m, &rd, &wr, &exc, &maxfd);



select_test(maxfd + 1, &rd, &wr, &exc, &interval);

abort_on_test_timeout();
}

test_cleanup:

#if defined(LIB529)


curl_multi_remove_handle(m, curl);
curl_multi_cleanup(m);
curl_easy_cleanup(curl);
curl_global_cleanup();
#else


curl_multi_remove_handle(m, curl);
curl_easy_cleanup(curl);
curl_multi_cleanup(m);
curl_global_cleanup();
#endif


fclose(hd_src);

return res;
}
