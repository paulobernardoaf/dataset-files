

























#include <stdio.h>
#include <curl/curl.h>

#if LIBCURL_VERSION_NUM >= 0x073d00




#define TIME_IN_US 1 
#define TIMETYPE curl_off_t
#define TIMEOPT CURLINFO_TOTAL_TIME_T
#define MINIMAL_PROGRESS_FUNCTIONALITY_INTERVAL 3000000
#else
#define TIMETYPE double
#define TIMEOPT CURLINFO_TOTAL_TIME
#define MINIMAL_PROGRESS_FUNCTIONALITY_INTERVAL 3
#endif

#define STOP_DOWNLOAD_AFTER_THIS_MANY_BYTES 6000

struct myprogress {
TIMETYPE lastruntime; 
CURL *curl;
};


static int xferinfo(void *p,
curl_off_t dltotal, curl_off_t dlnow,
curl_off_t ultotal, curl_off_t ulnow)
{
struct myprogress *myp = (struct myprogress *)p;
CURL *curl = myp->curl;
TIMETYPE curtime = 0;

curl_easy_getinfo(curl, TIMEOPT, &curtime);




if((curtime - myp->lastruntime) >= MINIMAL_PROGRESS_FUNCTIONALITY_INTERVAL) {
myp->lastruntime = curtime;
#if defined(TIME_IN_US)
fprintf(stderr, "TOTAL TIME: %" CURL_FORMAT_CURL_OFF_T ".%06ld\r\n",
(curtime / 1000000), (long)(curtime % 1000000));
#else
fprintf(stderr, "TOTAL TIME: %f \r\n", curtime);
#endif
}

fprintf(stderr, "UP: %" CURL_FORMAT_CURL_OFF_T " of %" CURL_FORMAT_CURL_OFF_T
" DOWN: %" CURL_FORMAT_CURL_OFF_T " of %" CURL_FORMAT_CURL_OFF_T
"\r\n",
ulnow, ultotal, dlnow, dltotal);

if(dlnow > STOP_DOWNLOAD_AFTER_THIS_MANY_BYTES)
return 1;
return 0;
}

#if LIBCURL_VERSION_NUM < 0x072000

static int older_progress(void *p,
double dltotal, double dlnow,
double ultotal, double ulnow)
{
return xferinfo(p,
(curl_off_t)dltotal,
(curl_off_t)dlnow,
(curl_off_t)ultotal,
(curl_off_t)ulnow);
}
#endif

int main(void)
{
CURL *curl;
CURLcode res = CURLE_OK;
struct myprogress prog;

curl = curl_easy_init();
if(curl) {
prog.lastruntime = 0;
prog.curl = curl;

curl_easy_setopt(curl, CURLOPT_URL, "https://example.com/");

#if LIBCURL_VERSION_NUM >= 0x072000










curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, xferinfo);


curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &prog);
#else
curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, older_progress);

curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, &prog);
#endif

curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
res = curl_easy_perform(curl);

if(res != CURLE_OK)
fprintf(stderr, "%s\n", curl_easy_strerror(res));


curl_easy_cleanup(curl);
}
return (int)res;
}
