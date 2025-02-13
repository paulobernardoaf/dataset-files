#include <tomcrypt_test.h>
int cipher_hash_test(void)
{
int x;
for (x = 0; cipher_descriptor[x].name != NULL; x++) {
DOX(cipher_descriptor[x].test(), cipher_descriptor[x].name);
}
#if defined(LTC_CHACHA)
DO(chacha_test());
#endif
#if defined(LTC_SALSA20)
DO(salsa20_test());
#endif
#if defined(LTC_XSALSA20)
DO(xsalsa20_test());
#endif
#if defined(LTC_SOSEMANUK)
DO(sosemanuk_test());
#endif
#if defined(LTC_RABBIT)
DO(rabbit_test());
#endif
#if defined(LTC_RC4_STREAM)
DO(rc4_stream_test());
#endif
#if defined(LTC_SOBER128_STREAM)
DO(sober128_stream_test());
#endif
for (x = 0; hash_descriptor[x].name != NULL; x++) {
DOX(hash_descriptor[x].test(), hash_descriptor[x].name);
}
#if defined(LTC_SHA3)
DOX(sha3_shake_test(), "sha3_shake");
#endif
return 0;
}
