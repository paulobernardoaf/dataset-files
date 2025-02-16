#include "curlcheck.h"
#include "curl_hmac.h"
#include "curl_md5.h"
static CURLcode unit_setup(void)
{
return CURLE_OK;
}
static void unit_stop(void)
{
}
UNITTEST_START
#if !defined(CURL_DISABLE_CRYPTO_AUTH)
const char password[] = "Pa55worD";
const char string1[] = "1";
const char string2[] = "hello-you-fool";
unsigned char output[HMAC_MD5_LENGTH];
unsigned char *testp = output;
Curl_hmacit(Curl_HMAC_MD5,
(const unsigned char *) password, strlen(password),
(const unsigned char *) string1, strlen(string1),
output);
verify_memory(testp,
"\xd1\x29\x75\x43\x58\xdc\xab\x78\xdf\xcd\x7f\x2b\x29\x31\x13"
"\x37", HMAC_MD5_LENGTH);
Curl_hmacit(Curl_HMAC_MD5,
(const unsigned char *) password, strlen(password),
(const unsigned char *) string2, strlen(string2),
output);
verify_memory(testp,
"\x75\xf1\xa7\xb9\xf5\x40\xe5\xa4\x98\x83\x9f\x64\x5a\x27\x6d"
"\xd0", HMAC_MD5_LENGTH);
#endif
UNITTEST_STOP
