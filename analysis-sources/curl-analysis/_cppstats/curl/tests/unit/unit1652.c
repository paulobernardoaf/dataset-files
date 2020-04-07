#include "curlcheck.h"
#include "urldata.h"
#include "sendf.h"
static struct Curl_easy *data;
static char input[4096];
static char result[4096];
int debugf_cb(CURL *handle, curl_infotype type, char *buf, size_t size,
void *userptr);
int
debugf_cb(CURL *handle, curl_infotype type, char *buf, size_t size,
void *userptr)
{
(void)handle;
(void)type;
(void)userptr;
memset(result, '\0', sizeof(result));
memcpy(result, buf, size);
return 0;
}
static CURLcode
unit_setup(void)
{
int res = 0;
global_init(CURL_GLOBAL_ALL);
data = curl_easy_init();
if(!data)
return CURLE_OUT_OF_MEMORY;
curl_easy_setopt(data, CURLOPT_DEBUGFUNCTION, debugf_cb);
curl_easy_setopt(data, CURLOPT_VERBOSE, 1L);
return CURLE_OK;
}
static void
unit_stop(void)
{
curl_easy_cleanup(data);
curl_global_cleanup();
}
UNITTEST_START
msnprintf(input, sizeof(input), "Simple Test");
Curl_infof(data, "%s", input);
fail_unless(strcmp(result, input) == 0, "Simple string test");
Curl_infof(data, "%s %u testing %lu\n", input, 42, 43L);
fail_unless(strcmp(result, "Simple Test 42 testing 43\n") == 0,
"Format string");
Curl_infof(data, "");
fail_unless(strlen(result) == 0, "Empty string");
Curl_infof(data, "%s", NULL);
fail_unless(strcmp(result, "(nil)") == 0, "Passing NULL as string");
memset(input, '\0', sizeof(input));
memset(input, 'A', 2048);
Curl_infof(data, "%s", input);
fail_unless(strlen(result) == 2048, "No truncation of infof input");
fail_unless(strcmp(result, input) == 0, "No truncation of infof input");
fail_unless(result[sizeof(result) - 1] == '\0',
"No truncation of infof input");
memset(input + 2047, 'A', 4);
Curl_infof(data, "%s", input);
fail_unless(strlen(result) == 2048, "Truncation of infof input 1");
fail_unless(result[sizeof(result) - 1] == '\0', "Truncation of infof input 1");
fail_unless(strncmp(result + 2045, "...", 3) == 0,
"Truncation of infof input 1");
memset(input + 2047, 'A', 4);
memset(input + 2047 + 4, '\n', 1);
Curl_infof(data, "%s\n", input);
fail_unless(strlen(result) == 2048, "Truncation of infof input 2");
fail_unless(result[sizeof(result) - 1] == '\0', "Truncation of infof input 2");
fail_unless(strncmp(result + 2044, "...", 3) == 0,
"Truncation of infof input 2");
memset(input, '\0', sizeof(input));
memset(input, 'A', sizeof(input) - 1);
Curl_infof(data, "%s\n", input);
fail_unless(strlen(result) == 2048, "Truncation of infof input 3");
fail_unless(result[sizeof(result) - 1] == '\0', "Truncation of infof input 3");
fail_unless(strncmp(result + 2044, "...", 3) == 0,
"Truncation of infof input 3");
UNITTEST_STOP
