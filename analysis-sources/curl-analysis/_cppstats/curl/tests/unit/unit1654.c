#include "curlcheck.h"
#include "urldata.h"
#include "altsvc.h"
static CURLcode
unit_setup(void)
{
return CURLE_OK;
}
static void
unit_stop(void)
{
curl_global_cleanup();
}
#if defined(CURL_DISABLE_HTTP) || !defined(USE_ALTSVC)
UNITTEST_START
{
return 0; 
}
UNITTEST_STOP
#else
UNITTEST_START
{
char outname[256];
CURL *curl;
CURLcode result;
struct altsvcinfo *asi = Curl_altsvc_init();
if(!asi)
return 1;
result = Curl_altsvc_load(asi, arg);
if(result) {
Curl_altsvc_cleanup(asi);
return result;
}
curl = curl_easy_init();
if(!curl)
goto fail;
fail_unless(asi->num == 4, "wrong number of entries");
msnprintf(outname, sizeof(outname), "%s-out", arg);
result = Curl_altsvc_parse(curl, asi, "h2=\"example.com:8080\"",
ALPN_h1, "example.org", 8080);
if(result) {
fprintf(stderr, "Curl_altsvc_parse() failed!\n");
unitfail++;
}
fail_unless(asi->num == 5, "wrong number of entries");
result = Curl_altsvc_parse(curl, asi, "h3=\":8080\"",
ALPN_h1, "2.example.org", 8080);
if(result) {
fprintf(stderr, "Curl_altsvc_parse(2) failed!\n");
unitfail++;
}
fail_unless(asi->num == 6, "wrong number of entries");
result = Curl_altsvc_parse(curl, asi,
"h2=\"example.com:8080\", h3=\"yesyes.com\"",
ALPN_h1, "3.example.org", 8080);
if(result) {
fprintf(stderr, "Curl_altsvc_parse(3) failed!\n");
unitfail++;
}
fail_unless(asi->num == 8, "wrong number of entries");
result = Curl_altsvc_parse(curl, asi, "h2=\"example.com:443\"; ma = 120;",
ALPN_h2, "example.org", 80);
if(result) {
fprintf(stderr, "Curl_altsvc_parse(4) failed!\n");
unitfail++;
}
fail_unless(asi->num == 9, "wrong number of entries");
result = Curl_altsvc_parse(curl, asi, "h2=\"example.net:443\"; ma=\"180\";",
ALPN_h2, "example.net", 80);
if(result) {
fprintf(stderr, "Curl_altsvc_parse(4) failed!\n");
unitfail++;
}
fail_unless(asi->num == 10, "wrong number of entries");
result = Curl_altsvc_parse(curl, asi,
"h2=\":443\", h3=\":443\"; ma = 120; persist = 1",
ALPN_h1, "curl.haxx.se", 80);
if(result) {
fprintf(stderr, "Curl_altsvc_parse(5) failed!\n");
unitfail++;
}
fail_unless(asi->num == 12, "wrong number of entries");
result = Curl_altsvc_parse(curl, asi, "clear;",
ALPN_h1, "curl.haxx.se", 80);
if(result) {
fprintf(stderr, "Curl_altsvc_parse(6) failed!\n");
unitfail++;
}
fail_unless(asi->num == 10, "wrong number of entries");
Curl_altsvc_save(curl, asi, outname);
curl_easy_cleanup(curl);
fail:
Curl_altsvc_cleanup(asi);
return unitfail;
}
UNITTEST_STOP
#endif
