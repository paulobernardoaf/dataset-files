#include "test.h"
#include "testutil.h"
#include "warnless.h"
#include "memdebug.h"
#define EXCESSIVE 10*1000*1000
int test(char *URL)
{
CURLcode res = 0;
CURL *curl = NULL;
char *longurl = malloc(EXCESSIVE);
CURLU *u;
(void)URL;
if(!longurl)
return 1;
memset(longurl, 'a', EXCESSIVE);
longurl[EXCESSIVE-1] = 0;
global_init(CURL_GLOBAL_ALL);
easy_init(curl);
res = curl_easy_setopt(curl, CURLOPT_URL, longurl);
printf("CURLOPT_URL %d bytes URL == %d\n",
EXCESSIVE, (int)res);
res = curl_easy_setopt(curl, CURLOPT_POSTFIELDS, longurl);
printf("CURLOPT_POSTFIELDS %d bytes data == %d\n",
EXCESSIVE, (int)res);
u = curl_url();
if(u) {
CURLUcode uc = curl_url_set(u, CURLUPART_URL, longurl, 0);
printf("CURLUPART_URL %d bytes URL == %d\n",
EXCESSIVE, (int)uc);
uc = curl_url_set(u, CURLUPART_SCHEME, longurl, CURLU_NON_SUPPORT_SCHEME);
printf("CURLUPART_SCHEME %d bytes scheme == %d\n",
EXCESSIVE, (int)uc);
uc = curl_url_set(u, CURLUPART_USER, longurl, 0);
printf("CURLUPART_USER %d bytes user == %d\n",
EXCESSIVE, (int)uc);
curl_url_cleanup(u);
}
test_cleanup:
free(longurl);
curl_easy_cleanup(curl);
curl_global_cleanup();
return res; 
}
