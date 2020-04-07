



























#include "test.h"

#include "memdebug.h"

static int progressKiller(void *arg,
double dltotal,
double dlnow,
double ultotal,
double ulnow)
{
(void)arg;
(void)dltotal;
(void)dlnow;
(void)ultotal;
(void)ulnow;
printf("PROGRESSFUNCTION called\n");
return 1;
}

int test(char *URL)
{
CURL *curl;
int res = 0;

global_init(CURL_GLOBAL_ALL);

easy_init(curl);

easy_setopt(curl, CURLOPT_URL, URL);
easy_setopt(curl, CURLOPT_TIMEOUT, (long)7);
easy_setopt(curl, CURLOPT_NOSIGNAL, (long)1);
easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progressKiller);
easy_setopt(curl, CURLOPT_PROGRESSDATA, NULL);
easy_setopt(curl, CURLOPT_NOPROGRESS, (long)0);

res = curl_easy_perform(curl);

test_cleanup:



curl_easy_cleanup(curl);
curl_global_cleanup();

return res;
}
