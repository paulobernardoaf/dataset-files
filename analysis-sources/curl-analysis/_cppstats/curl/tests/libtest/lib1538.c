#include "test.h"
#include "memdebug.h"
int test(char *URL)
{
int res = 0;
CURLcode easyret;
CURLMcode multiret;
CURLSHcode shareret;
(void)URL;
curl_easy_strerror(INT_MAX);
curl_multi_strerror(INT_MAX);
curl_share_strerror(INT_MAX);
curl_easy_strerror(-INT_MAX);
curl_multi_strerror(-INT_MAX);
curl_share_strerror(-INT_MAX);
for(easyret = CURLE_OK; easyret <= CURL_LAST; easyret++) {
printf("e%d: %s\n", (int)easyret, curl_easy_strerror(easyret));
}
for(multiret = CURLM_CALL_MULTI_PERFORM; multiret <= CURLM_LAST;
multiret++) {
printf("m%d: %s\n", (int)multiret, curl_multi_strerror(multiret));
}
for(shareret = CURLSHE_OK; shareret <= CURLSHE_LAST; shareret++) {
printf("s%d: %s\n", (int)shareret, curl_share_strerror(shareret));
}
return (int)res;
}
