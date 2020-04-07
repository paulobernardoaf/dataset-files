#include "test.h"
#include "testutil.h"
#include "warnless.h"
#include "memdebug.h"
static int dload_progress_cb(void *a, curl_off_t b, curl_off_t c,
curl_off_t d, curl_off_t e)
{
(void)a;
(void)b;
(void)c;
(void)d;
(void)e;
return 0;
}
static size_t write_cb(char *d, size_t n, size_t l, void *p)
{
(void)d;
(void)p;
return n*l;
}
static CURLcode run(CURL *hnd, long limit, long time)
{
curl_easy_setopt(hnd, CURLOPT_LOW_SPEED_LIMIT, limit);
curl_easy_setopt(hnd, CURLOPT_LOW_SPEED_TIME, time);
return curl_easy_perform(hnd);
}
int test(char *URL)
{
CURLcode ret;
CURL *hnd = curl_easy_init();
char buffer[CURL_ERROR_SIZE];
curl_easy_setopt(hnd, CURLOPT_URL, URL);
curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, write_cb);
curl_easy_setopt(hnd, CURLOPT_ERRORBUFFER, buffer);
curl_easy_setopt(hnd, CURLOPT_NOPROGRESS, 0L);
curl_easy_setopt(hnd, CURLOPT_XFERINFOFUNCTION, dload_progress_cb);
printf("Start: %d\n", time(NULL));
ret = run(hnd, 1, 2);
if(ret)
fprintf(stderr, "error %d: %s\n", ret, buffer);
ret = run(hnd, 12000, 1);
if(ret != CURLE_OPERATION_TIMEDOUT)
fprintf(stderr, "error %d: %s\n", ret, buffer);
else
ret = 0;
printf("End: %d\n", time(NULL));
curl_easy_cleanup(hnd);
return (int)ret;
}
