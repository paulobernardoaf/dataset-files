#include "curlcheck.h"
#include "llist.h"
static CURL *easy;
static CURLcode unit_setup(void)
{
int res = CURLE_OK;
global_init(CURL_GLOBAL_ALL);
easy = curl_easy_init();
if(!easy)
return CURLE_OUT_OF_MEMORY;
return res;
}
static void unit_stop(void)
{
curl_easy_cleanup(easy);
curl_global_cleanup();
}
UNITTEST_START
int len;
char *esc;
esc = curl_easy_escape(easy, "", -1);
fail_unless(esc == NULL, "negative string length can't work");
esc = curl_easy_unescape(easy, "%41%41%41%41", -1, &len);
fail_unless(esc == NULL, "negative string length can't work");
UNITTEST_STOP
