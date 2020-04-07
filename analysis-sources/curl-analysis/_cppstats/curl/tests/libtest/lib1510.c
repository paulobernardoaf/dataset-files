#include "test.h"
#include "testutil.h"
#include "warnless.h"
#include "memdebug.h"
#define TEST_HANG_TIMEOUT 60 * 1000
#define NUM_URLS 4
int test(char *URL)
{
int res = 0;
CURL *curl = NULL;
int i;
char target_url[256];
char dnsentry[256];
struct curl_slist *slist = NULL, *slist2;
char *port = libtest_arg3;
char *address = libtest_arg2;
(void)URL;
for(i = 0; i < NUM_URLS; i++) {
msnprintf(dnsentry, sizeof(dnsentry), "server%d.example.com:%s:%s", i + 1,
port, address);
printf("%s\n", dnsentry);
slist2 = curl_slist_append(slist, dnsentry);
if(!slist2) {
fprintf(stderr, "curl_slist_append() failed\n");
goto test_cleanup;
}
slist = slist2;
}
start_test_timing();
global_init(CURL_GLOBAL_ALL);
easy_init(curl);
easy_setopt(curl, CURLOPT_VERBOSE, 1L);
easy_setopt(curl, CURLOPT_HEADER, 1L);
easy_setopt(curl, CURLOPT_RESOLVE, slist);
easy_setopt(curl, CURLOPT_MAXCONNECTS, 3L);
for(i = 0; i < NUM_URLS; i++) {
msnprintf(target_url, sizeof(target_url),
"http://server%d.example.com:%s/path/1510%04i",
i + 1, port, i + 1);
target_url[sizeof(target_url) - 1] = '\0';
easy_setopt(curl, CURLOPT_URL, target_url);
res = curl_easy_perform(curl);
abort_on_test_timeout();
}
test_cleanup:
curl_easy_cleanup(curl);
curl_slist_free_all(slist);
curl_global_cleanup();
return res;
}
