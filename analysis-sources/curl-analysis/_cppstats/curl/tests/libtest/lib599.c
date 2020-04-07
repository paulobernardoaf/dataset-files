#include "test.h"
#include "memdebug.h"
static int progress_callback(void *clientp, double dltotal,
double dlnow, double ultotal, double ulnow)
{
(void)clientp;
(void)ulnow;
(void)ultotal;
if((dltotal > 0.0) && (dlnow > dltotal)) {
printf("%.0f > %.0f !!\n", dltotal, dlnow);
return -1;
}
return 0;
}
int test(char *URL)
{
CURL *curl;
CURLcode res = CURLE_OK;
double content_length = 0.0;
if(curl_global_init(CURL_GLOBAL_ALL) != CURLE_OK) {
fprintf(stderr, "curl_global_init() failed\n");
return TEST_ERR_MAJOR_BAD;
}
curl = curl_easy_init();
if(!curl) {
fprintf(stderr, "curl_easy_init() failed\n");
curl_global_cleanup();
return TEST_ERR_MAJOR_BAD;
}
test_setopt(curl, CURLOPT_URL, URL);
test_setopt(curl, CURLOPT_NOPROGRESS, 0L);
test_setopt(curl, CURLOPT_PROGRESSFUNCTION, progress_callback);
test_setopt(curl, CURLOPT_VERBOSE, 1L);
test_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
test_setopt(curl, CURLOPT_HEADER, 1L);
res = curl_easy_perform(curl);
if(!res) {
FILE *moo;
res = curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD,
&content_length);
moo = fopen(libtest_arg2, "wb");
if(moo) {
fprintf(moo, "CL: %.0f\n", content_length);
fclose(moo);
}
}
test_cleanup:
curl_easy_cleanup(curl);
curl_global_cleanup();
return res;
}
