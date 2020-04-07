#include "test.h"
#include "testutil.h"
#include "warnless.h"
#include "memdebug.h"
int test(char *URL)
{
CURLcode res = 0;
CURL *curl = NULL;
long protocol = 0;
global_init(CURL_GLOBAL_ALL);
easy_init(curl);
easy_setopt(curl, CURLOPT_URL, URL);
res = curl_easy_perform(curl);
if(res) {
fprintf(stderr, "curl_easy_perform() returned %d (%s)\n",
res, curl_easy_strerror(res));
goto test_cleanup;
}
res = curl_easy_getinfo(curl, CURLINFO_PROTOCOL, &protocol);
if(res) {
fprintf(stderr, "curl_easy_getinfo() returned %d (%s)\n",
res, curl_easy_strerror(res));
goto test_cleanup;
}
printf("Protocol: %x\n", protocol);
curl_easy_cleanup(curl);
curl_global_cleanup();
return 0;
test_cleanup:
curl_easy_cleanup(curl);
curl_global_cleanup();
return res; 
}
