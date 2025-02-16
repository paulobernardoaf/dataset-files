
























#include "test.h"

#include "memdebug.h"

static CURL *curl;

static int progressCallback(void *arg,
double dltotal,
double dlnow,
double ultotal,
double ulnow)
{
CURLcode res = 0;
char buffer[256];
size_t n = 0;
(void)arg;
(void)dltotal;
(void)dlnow;
(void)ultotal;
(void)ulnow;
res = curl_easy_recv(curl, buffer, 256, &n);
printf("curl_easy_recv returned %d\n", res);
res = curl_easy_send(curl, buffer, n, &n);
printf("curl_easy_send returned %d\n", res);

return 1;
}

int test(char *URL)
{
int res = 0;

global_init(CURL_GLOBAL_ALL);

easy_init(curl);

easy_setopt(curl, CURLOPT_URL, URL);
easy_setopt(curl, CURLOPT_TIMEOUT, (long)7);
easy_setopt(curl, CURLOPT_NOSIGNAL, (long)1);
easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progressCallback);
easy_setopt(curl, CURLOPT_PROGRESSDATA, NULL);
easy_setopt(curl, CURLOPT_NOPROGRESS, (long)0);

res = curl_easy_perform(curl);

test_cleanup:



curl_easy_cleanup(curl);
curl_global_cleanup();

return res;
}
