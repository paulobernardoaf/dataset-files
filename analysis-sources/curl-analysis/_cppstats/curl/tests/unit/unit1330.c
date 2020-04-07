#include "curlcheck.h"
#include "memdebug.h"
static CURLcode unit_setup(void)
{
return CURLE_OK;
}
static void unit_stop(void)
{
}
UNITTEST_START
char *ptr = malloc(1330);
Curl_safefree(ptr);
UNITTEST_STOP
