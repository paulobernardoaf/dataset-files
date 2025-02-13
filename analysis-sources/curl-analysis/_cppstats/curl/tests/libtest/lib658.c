#include "test.h"
#include "testutil.h"
#include "warnless.h"
#include "memdebug.h"
int test(char *URL)
{
CURL *handle = NULL;
CURLcode res = 0;
CURLU *urlp = NULL;
CURLUcode uc = 0;
global_init(CURL_GLOBAL_ALL);
easy_init(handle);
urlp = curl_url();
if(!urlp) {
fprintf(stderr, "problem init URL api.");
goto test_cleanup;
}
uc = curl_url_set(urlp, CURLUPART_URL, URL, 0);
if(uc) {
fprintf(stderr, "problem setting CURLUPART_URL.");
goto test_cleanup;
}
easy_setopt(handle, CURLOPT_URL, "http://www.example.com");
easy_setopt(handle, CURLOPT_CURLU, urlp);
easy_setopt(handle, CURLOPT_VERBOSE, 1L);
res = curl_easy_perform(handle);
if(res) {
fprintf(stderr, "%s:%d curl_easy_perform() failed with code %d (%s)\n",
__FILE__, __LINE__, res, curl_easy_strerror(res));
goto test_cleanup;
}
test_cleanup:
curl_url_cleanup(urlp);
curl_easy_cleanup(handle);
curl_global_cleanup();
return res;
}
