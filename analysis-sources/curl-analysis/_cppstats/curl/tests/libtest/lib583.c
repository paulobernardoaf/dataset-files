#include "test.h"
#include <sys/stat.h>
#include "memdebug.h"
int test(char *URL)
{
int stillRunning;
CURLM *multiHandle = NULL;
CURL *curl = NULL;
CURLMcode res = CURLM_OK;
global_init(CURL_GLOBAL_ALL);
multi_init(multiHandle);
easy_init(curl);
easy_setopt(curl, CURLOPT_USERPWD, libtest_arg2);
easy_setopt(curl, CURLOPT_SSH_PUBLIC_KEYFILE, "curl_client_key.pub");
easy_setopt(curl, CURLOPT_SSH_PRIVATE_KEYFILE, "curl_client_key");
easy_setopt(curl, CURLOPT_UPLOAD, 1L);
easy_setopt(curl, CURLOPT_VERBOSE, 1L);
easy_setopt(curl, CURLOPT_URL, URL);
easy_setopt(curl, CURLOPT_INFILESIZE, (long)5);
multi_add_handle(multiHandle, curl);
fprintf(stderr, "curl_multi_perform()...\n");
multi_perform(multiHandle, &stillRunning);
fprintf(stderr, "curl_multi_perform() succeeded\n");
fprintf(stderr, "curl_multi_remove_handle()...\n");
res = curl_multi_remove_handle(multiHandle, curl);
if(res)
fprintf(stderr, "curl_multi_remove_handle() failed, "
"with code %d\n", (int)res);
else
fprintf(stderr, "curl_multi_remove_handle() succeeded\n");
test_cleanup:
curl_easy_cleanup(curl);
curl_multi_cleanup(multiHandle);
curl_global_cleanup();
return (int)res;
}
