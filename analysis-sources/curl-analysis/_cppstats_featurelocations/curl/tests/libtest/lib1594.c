























#include "test.h"

#include "memdebug.h"

int test(char *URL)
{
struct curl_slist *header = NULL;
curl_off_t retry;
CURL *curl = NULL;
int res = 0;

global_init(CURL_GLOBAL_ALL);

easy_init(curl);

easy_setopt(curl, CURLOPT_URL, URL);

res = curl_easy_perform(curl);
if(res)
goto test_cleanup;

res = curl_easy_getinfo(curl, CURLINFO_RETRY_AFTER, &retry);
if(res)
goto test_cleanup;

#if defined(LIB1596)


retry += time(NULL);
retry /= 10000;
#endif
printf("Retry-After: %" CURL_FORMAT_CURL_OFF_T "\n", retry);

test_cleanup:


curl_easy_cleanup(curl);
curl_slist_free_all(header);
curl_global_cleanup();

return res;
}
