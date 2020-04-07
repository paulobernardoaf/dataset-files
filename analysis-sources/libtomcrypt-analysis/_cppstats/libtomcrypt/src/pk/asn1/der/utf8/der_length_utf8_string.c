#include "tomcrypt_private.h"
#if defined(LTC_DER)
unsigned long der_utf8_charsize(const wchar_t c)
{
if (c <= 0x7F) {
return 1;
}
if (c <= 0x7FF) {
return 2;
}
#if LTC_WCHAR_MAX == 0xFFFF
return 3;
#else
if (c <= 0xFFFF) {
return 3;
}
return 4;
#endif
}
int der_utf8_valid_char(const wchar_t c)
{
LTC_UNUSED_PARAM(c);
#if !defined(LTC_WCHAR_MAX) || LTC_WCHAR_MAX > 0xFFFF
if (c > 0x10FFFF) return 0;
#endif
#if LTC_WCHAR_MAX != 0xFFFF && LTC_WCHAR_MAX != 0xFFFFFFFF
if (c < 0) return 0;
#endif
return 1;
}
int der_length_utf8_string(const wchar_t *in, unsigned long noctets, unsigned long *outlen)
{
unsigned long x, len;
int err;
LTC_ARGCHK(in != NULL);
LTC_ARGCHK(outlen != NULL);
len = 0;
for (x = 0; x < noctets; x++) {
if (!der_utf8_valid_char(in[x])) return CRYPT_INVALID_ARG;
len += der_utf8_charsize(in[x]);
}
if ((err = der_length_asn1_length(len, &x)) != CRYPT_OK) {
return err;
}
*outlen = 1 + x + len;
return CRYPT_OK;
}
#endif
