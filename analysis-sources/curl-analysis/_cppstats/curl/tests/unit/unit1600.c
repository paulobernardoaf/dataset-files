#include "curlcheck.h"
#include "urldata.h"
#include "curl_ntlm_core.h"
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
#if defined(USE_NTLM) && (!defined(USE_WINDOWS_SSPI) || defined(USE_WIN32_CRYPTO))
unsigned char output[21];
unsigned char *testp = output;
Curl_ntlm_core_mk_nt_hash(easy, "1", output);
verify_memory(testp,
"\x69\x94\x3c\x5e\x63\xb4\xd2\xc1\x04\xdb"
"\xbc\xc1\x51\x38\xb7\x2b\x00\x00\x00\x00\x00", 21);
Curl_ntlm_core_mk_nt_hash(easy, "hello-you-fool", output);
verify_memory(testp,
"\x39\xaf\x87\xa6\x75\x0a\x7a\x00\xba\xa0"
"\xd3\x4f\x04\x9e\xc1\xd0\x00\x00\x00\x00\x00", 21);
Curl_ntlm_core_mk_nt_hash(easy, "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", output);
verify_memory(testp,
"\x36\x9d\xae\x06\x84\x7e\xe1\xc1\x4a\x94\x39\xea\x6f\x44\x8c\x65\x00\x00\x00\x00\x00", 21);
#endif
UNITTEST_STOP
