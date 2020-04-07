#include <tomcrypt_test.h>
int misc_test(void)
{
#if defined(LTC_BCRYPT)
DO(bcrypt_test());
#endif
#if defined(LTC_HKDF)
DO(hkdf_test());
#endif
#if defined(LTC_PKCS_5)
DO(pkcs_5_test());
#endif
#if defined(LTC_PADDING)
DO(padding_test());
#endif
#if defined(LTC_BASE64)
DO(base64_test());
#endif
#if defined(LTC_BASE32)
DO(base32_test());
#endif
#if defined(LTC_BASE16)
DO(base16_test());
#endif
#if defined(LTC_ADLER32)
DO(adler32_test());
#endif
#if defined(LTC_CRC32)
DO(crc32_test());
#endif
#if defined(LTC_SSH)
ssh_test();
#endif
return 0;
}
