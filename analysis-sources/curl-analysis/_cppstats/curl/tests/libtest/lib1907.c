#include "test.h"
#include "testutil.h"
#include "warnless.h"
#include "memdebug.h"
int test(char *URL)
{
char *url_after;
CURL *curl;
CURLcode curl_code;
char error_buffer[CURL_ERROR_SIZE] = "";
curl_global_init(CURL_GLOBAL_DEFAULT);
curl = curl_easy_init();
curl_easy_setopt(curl, CURLOPT_URL, URL);
curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, error_buffer);
curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
curl_code = curl_easy_perform(curl);
if(!curl_code)
fprintf(stderr, "failure expected, "
"curl_easy_perform returned %ld: <%s>, <%s>\n",
(long) curl_code, curl_easy_strerror(curl_code), error_buffer);
if(!curl_easy_getinfo(curl, CURLINFO_EFFECTIVE_URL, &url_after))
printf("Effective URL: %s\n", url_after);
curl_easy_cleanup(curl);
curl_global_cleanup();
return 0;
}
