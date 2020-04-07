




















#include "test.h"

#include "testutil.h"
#include "warnless.h"
#include "memdebug.h"

struct headerinfo {
size_t largest;
};

static size_t header(void *ptr, size_t size, size_t nmemb, void *stream)
{
size_t headersize = size * nmemb;
struct headerinfo *info = (struct headerinfo *)stream;
(void)ptr;

if(headersize > info->largest)

info->largest = headersize;

return nmemb * size;
}

int test(char *URL)
{
CURLcode code;
CURL *curl = NULL;
int res = 0;
struct headerinfo info = {0};

global_init(CURL_GLOBAL_ALL);

easy_init(curl);

easy_setopt(curl, CURLOPT_HEADERFUNCTION, header);
easy_setopt(curl, CURLOPT_HEADERDATA, &info);
easy_setopt(curl, CURLOPT_VERBOSE, 1L);
easy_setopt(curl, CURLOPT_URL, URL);

code = curl_easy_perform(curl);
if(CURLE_OK != code) {
fprintf(stderr, "%s:%d curl_easy_perform() failed, "
"with code %d (%s)\n",
__FILE__, __LINE__, (int)code, curl_easy_strerror(code));
res = TEST_ERR_MAJOR_BAD;
goto test_cleanup;
}

printf("Max: %ld\n", (long)info.largest);

test_cleanup:

curl_easy_cleanup(curl);
curl_global_cleanup();

return res;
}
