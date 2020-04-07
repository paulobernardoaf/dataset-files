




















#include "test.h"

#include "testutil.h"
#include "warnless.h"
#include "memdebug.h"


int test(char *URL)
{
CURL *curls = NULL;
int res = 0;
curl_mimepart *field = NULL;
curl_mime *mime = NULL;

global_init(CURL_GLOBAL_ALL);
easy_init(curls);

mime = curl_mime_init(curls);
field = curl_mime_addpart(mime);
curl_mime_name(field, "name");
curl_mime_data(field, "short value", CURL_ZERO_TERMINATED);

easy_setopt(curls, CURLOPT_URL, URL);
easy_setopt(curls, CURLOPT_HEADER, 1L);
easy_setopt(curls, CURLOPT_VERBOSE, 1L);
easy_setopt(curls, CURLOPT_MIMEPOST, mime);
easy_setopt(curls, CURLOPT_NOPROGRESS, 1L);

res = curl_easy_perform(curls);
if(res)
goto test_cleanup;


curl_mime_data(field, "long value for length change", CURL_ZERO_TERMINATED);
res = curl_easy_perform(curls);

test_cleanup:
curl_mime_free(mime);
curl_easy_cleanup(curls);
curl_global_cleanup();
return (int) res; 
}
