




















#include "test.h"

#include "testutil.h"
#include "warnless.h"
#include "memdebug.h"





int test(char *URL)
{
CURL *handle = NULL;
CURLcode res = 0;
CURLU *urlp = NULL;

global_init(CURL_GLOBAL_ALL);
easy_init(handle);

urlp = curl_url();

if(!urlp) {
fprintf(stderr, "problem init URL api.");
goto test_cleanup;
}


if(curl_url_set(urlp, CURLUPART_HOST, "www.example.com", 0) ||
curl_url_set(urlp, CURLUPART_SCHEME, "http", 0) ||
curl_url_set(urlp, CURLUPART_PORT, "80", 0)) {
fprintf(stderr, "problem setting CURLUPART");
goto test_cleanup;
}

easy_setopt(handle, CURLOPT_CURLU, urlp);
easy_setopt(handle, CURLOPT_VERBOSE, 1L);
easy_setopt(handle, CURLOPT_PROXY, URL);

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
