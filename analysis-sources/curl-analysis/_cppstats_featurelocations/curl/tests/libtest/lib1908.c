




















#include "test.h"

#include "testutil.h"
#include "warnless.h"
#include "memdebug.h"

int test(char *URL)
{
CURLcode ret = CURLE_OK;
CURL *hnd;
start_test_timing();

curl_global_init(CURL_GLOBAL_ALL);

hnd = curl_easy_init();
if(hnd) {
curl_easy_setopt(hnd, CURLOPT_URL, URL);
curl_easy_setopt(hnd, CURLOPT_NOPROGRESS, 1L);
curl_easy_setopt(hnd, CURLOPT_ALTSVC, "log/altsvc-1908");
ret = curl_easy_perform(hnd);
curl_easy_reset(hnd);
curl_easy_cleanup(hnd);
}
curl_global_cleanup();
return (int)ret;
}
