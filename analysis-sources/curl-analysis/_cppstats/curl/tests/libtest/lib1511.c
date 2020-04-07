#include "test.h"
#include "memdebug.h"
int test(char *URL)
{
long unmet;
CURL *curl = NULL;
int res = 0;
global_init(CURL_GLOBAL_ALL);
easy_init(curl);
easy_setopt(curl, CURLOPT_URL, URL);
easy_setopt(curl, CURLOPT_HEADER, 1L);
easy_setopt(curl, CURLOPT_TIMECONDITION, (long)CURL_TIMECOND_IFMODSINCE);
easy_setopt(curl, CURLOPT_TIMEVALUE, 1566210680L);
res = curl_easy_perform(curl);
if(res)
goto test_cleanup;
curl_easy_getinfo(curl, CURLINFO_CONDITION_UNMET, &unmet);
if(unmet != 1L) {
res = TEST_ERR_FAILURE; 
goto test_cleanup;
}
easy_setopt(curl, CURLOPT_TIMEVALUE, 1L);
res = curl_easy_perform(curl);
if(res)
goto test_cleanup;
curl_easy_getinfo(curl, CURLINFO_CONDITION_UNMET, &unmet);
if(unmet != 0L) {
res = TEST_ERR_FAILURE; 
goto test_cleanup;
}
res = TEST_ERR_SUCCESS; 
test_cleanup:
curl_easy_cleanup(curl);
curl_global_cleanup();
return res;
}
