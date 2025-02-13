




















#include "test.h"



#include "testutil.h"
#include "warnless.h"
#include "memdebug.h"

static char g_Data[40 * 1024]; 

static int sockopt_callback(void *clientp, curl_socket_t curlfd,
curlsocktype purpose)
{
#if defined(SOL_SOCKET) && defined(SO_SNDBUF)
int sndbufsize = 4 * 1024; 
(void) clientp;
(void) purpose;
setsockopt(curlfd, SOL_SOCKET, SO_SNDBUF,
(const char *)&sndbufsize, sizeof(sndbufsize));
#else
(void)clientp;
(void)curlfd;
(void)purpose;
#endif
return CURL_SOCKOPT_OK;
}

int test(char *URL)
{
CURLcode code;
struct curl_slist *pHeaderList = NULL;
CURL *pCurl = curl_easy_init();
memset(g_Data, 'A', sizeof(g_Data)); 

curl_easy_setopt(pCurl, CURLOPT_SOCKOPTFUNCTION, sockopt_callback);
curl_easy_setopt(pCurl, CURLOPT_URL, URL);
curl_easy_setopt(pCurl, CURLOPT_POSTFIELDS, g_Data);
curl_easy_setopt(pCurl, CURLOPT_POSTFIELDSIZE, (long)sizeof(g_Data));


pHeaderList = curl_slist_append(pHeaderList, "Expect:");

curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, pHeaderList);

code = curl_easy_perform(pCurl);

if(code == CURLE_OK) {
curl_off_t uploadSize;
curl_easy_getinfo(pCurl, CURLINFO_SIZE_UPLOAD_T, &uploadSize);

printf("uploadSize = %ld\n", (long)uploadSize);

if((size_t) uploadSize == sizeof(g_Data)) {
printf("!!!!!!!!!! PASS\n");
}
else {
printf("!!!!!!!!!! FAIL\n");
}
}
else {
printf("curl_easy_perform() failed. e = %d\n", code);
}

curl_slist_free_all(pHeaderList);
curl_easy_cleanup(pCurl);
curl_global_cleanup();

return 0;
}
