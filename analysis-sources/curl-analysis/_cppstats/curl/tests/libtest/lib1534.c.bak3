




















#include "test.h"

#include "memdebug.h"



int test(char *URL)
{
CURL *curl, *dupe = NULL;
long filetime;
int res = CURLE_OK;

global_init(CURL_GLOBAL_ALL);

easy_init(curl);




res = curl_easy_getinfo(curl, CURLINFO_FILETIME, &filetime);
if(res) {
fprintf(stderr, "%s:%d curl_easy_getinfo() failed with code %d (%s)\n",
__FILE__, __LINE__, res, curl_easy_strerror(res));
goto test_cleanup;
}
if(filetime != -1) {
fprintf(stderr, "%s:%d filetime init failed; expected -1 but is %ld\n",
__FILE__, __LINE__, filetime);
res = CURLE_FAILED_INIT;
goto test_cleanup;
}

easy_setopt(curl, CURLOPT_URL, URL);
easy_setopt(curl, CURLOPT_FILETIME, 1L);

res = curl_easy_perform(curl);
if(res) {
fprintf(stderr, "%s:%d curl_easy_perform() failed with code %d (%s)\n",
__FILE__, __LINE__, res, curl_easy_strerror(res));
goto test_cleanup;
}




res = curl_easy_getinfo(curl, CURLINFO_FILETIME, &filetime);
if(res) {
fprintf(stderr, "%s:%d curl_easy_getinfo() failed with code %d (%s)\n",
__FILE__, __LINE__, res, curl_easy_strerror(res));
goto test_cleanup;
}
if(filetime != 30) {
fprintf(stderr, "%s:%d filetime of http resource is incorrect; "
"expected 30 but is %ld\n",
__FILE__, __LINE__, filetime);
res = CURLE_HTTP_RETURNED_ERROR;
goto test_cleanup;
}




dupe = curl_easy_duphandle(curl);
if(!dupe) {
fprintf(stderr, "%s:%d curl_easy_duphandle() failed\n",
__FILE__, __LINE__);
res = CURLE_FAILED_INIT;
goto test_cleanup;
}

res = curl_easy_getinfo(dupe, CURLINFO_FILETIME, &filetime);
if(res) {
fprintf(stderr, "%s:%d curl_easy_getinfo() failed with code %d (%s)\n",
__FILE__, __LINE__, res, curl_easy_strerror(res));
goto test_cleanup;
}
if(filetime != -1) {
fprintf(stderr, "%s:%d filetime init failed; expected -1 but is %ld\n",
__FILE__, __LINE__, filetime);
res = CURLE_FAILED_INIT;
goto test_cleanup;
}





curl_easy_reset(curl);

res = curl_easy_getinfo(curl, CURLINFO_FILETIME, &filetime);
if(res) {
fprintf(stderr, "%s:%d curl_easy_getinfo() failed with code %d (%s)\n",
__FILE__, __LINE__, res, curl_easy_strerror(res));
goto test_cleanup;
}
if(filetime != -1) {
fprintf(stderr, "%s:%d filetime init failed; expected -1 but is %ld\n",
__FILE__, __LINE__, filetime);
res = CURLE_FAILED_INIT;
goto test_cleanup;
}

test_cleanup:
curl_easy_cleanup(curl);
curl_easy_cleanup(dupe);
curl_global_cleanup();
return res;
}
