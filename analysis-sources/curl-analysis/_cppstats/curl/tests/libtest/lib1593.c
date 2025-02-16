#include "test.h"
#include "memdebug.h"
int test(char *URL)
{
struct curl_slist *header = NULL;
long unmet;
CURL *curl = NULL;
int res = 0;
global_init(CURL_GLOBAL_ALL);
easy_init(curl);
easy_setopt(curl, CURLOPT_URL, URL);
easy_setopt(curl, CURLOPT_TIMECONDITION, (long)CURL_TIMECOND_IFMODSINCE);
easy_setopt(curl, CURLOPT_TIMEVALUE, 1566210680L);
header = curl_slist_append(NULL, "If-Modified-Since:");
if(!header) {
res = TEST_ERR_MAJOR_BAD;
goto test_cleanup;
}
easy_setopt(curl, CURLOPT_HTTPHEADER, header);
res = curl_easy_perform(curl);
if(res)
goto test_cleanup;
res = curl_easy_getinfo(curl, CURLINFO_CONDITION_UNMET, &unmet);
if(res)
goto test_cleanup;
if(unmet != 1L) {
res = TEST_ERR_FAILURE;
goto test_cleanup;
}
test_cleanup:
curl_easy_cleanup(curl);
curl_slist_free_all(header);
curl_global_cleanup();
return res;
}
