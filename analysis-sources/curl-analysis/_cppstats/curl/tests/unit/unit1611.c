#include "curlcheck.h"
#include "curl_md4.h"
static CURLcode unit_setup(void)
{
return CURLE_OK;
}
static void unit_stop(void)
{
}
UNITTEST_START
#if !defined(CURL_DISABLE_CRYPTO_AUTH)
const char string1[] = "1";
const char string2[] = "hello-you-fool";
unsigned char output[MD4_DIGEST_LENGTH];
unsigned char *testp = output;
Curl_md4it(output, (const unsigned char *) string1, strlen(string1));
verify_memory(testp,
"\x8b\xe1\xec\x69\x7b\x14\xad\x3a\x53\xb3\x71\x43\x61\x20\x64"
"\x1d", MD4_DIGEST_LENGTH);
Curl_md4it(output, (const unsigned char *) string2, strlen(string2));
verify_memory(testp,
"\xa7\x16\x1c\xad\x7e\xbe\xdb\xbc\xf8\xc7\x23\x10\x2d\x2c\xe2"
"\x0b", MD4_DIGEST_LENGTH);
#endif
UNITTEST_STOP
