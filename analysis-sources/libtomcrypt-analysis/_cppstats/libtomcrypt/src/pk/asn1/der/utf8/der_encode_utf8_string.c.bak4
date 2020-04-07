







#include "tomcrypt_private.h"







#if defined(LTC_DER)









int der_encode_utf8_string(const wchar_t *in, unsigned long inlen,
unsigned char *out, unsigned long *outlen)
{
unsigned long x, y, len;
int err;

LTC_ARGCHK(in != NULL);
LTC_ARGCHK(out != NULL);
LTC_ARGCHK(outlen != NULL);


for (x = len = 0; x < inlen; x++) {
if (!der_utf8_valid_char(in[x])) return CRYPT_INVALID_ARG;
len += der_utf8_charsize(in[x]);
}
if ((err = der_length_asn1_length(len, &x)) != CRYPT_OK) {
return err;
}
x += len + 1;


if (x > *outlen) {
*outlen = x;
return CRYPT_BUFFER_OVERFLOW;
}


x = 0;
out[x++] = 0x0C;

y = *outlen - x;
if ((err = der_encode_asn1_length(len, out + x, &y)) != CRYPT_OK) {
return err;
}
x += y;


for (y = 0; y < inlen; y++) {
switch (der_utf8_charsize(in[y])) {
case 1: out[x++] = (unsigned char)in[y]; break;
case 2: out[x++] = 0xC0 | ((in[y] >> 6) & 0x1F); out[x++] = 0x80 | (in[y] & 0x3F); break;
case 3: out[x++] = 0xE0 | ((in[y] >> 12) & 0x0F); out[x++] = 0x80 | ((in[y] >> 6) & 0x3F); out[x++] = 0x80 | (in[y] & 0x3F); break;
#if !defined(LTC_WCHAR_MAX) || LTC_WCHAR_MAX > 0xFFFF
case 4: out[x++] = 0xF0 | ((in[y] >> 18) & 0x07); out[x++] = 0x80 | ((in[y] >> 12) & 0x3F); out[x++] = 0x80 | ((in[y] >> 6) & 0x3F); out[x++] = 0x80 | (in[y] & 0x3F); break;
#endif
}
}


*outlen = x;

return CRYPT_OK;
}

#endif




