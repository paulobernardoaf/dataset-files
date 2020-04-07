#include "tomcrypt_private.h"
#if defined(LTC_DER)
int der_encode_ia5_string(const unsigned char *in, unsigned long inlen,
unsigned char *out, unsigned long *outlen)
{
unsigned long x, y, len;
int err;
LTC_ARGCHK(in != NULL);
LTC_ARGCHK(out != NULL);
LTC_ARGCHK(outlen != NULL);
if ((err = der_length_ia5_string(in, inlen, &len)) != CRYPT_OK) {
return err;
}
if (len > *outlen) {
*outlen = len;
return CRYPT_BUFFER_OVERFLOW;
}
x = 0;
out[x++] = 0x16;
len = *outlen - x;
if ((err = der_encode_asn1_length(inlen, out + x, &len)) != CRYPT_OK) {
return err;
}
x += len;
for (y = 0; y < inlen; y++) {
out[x++] = der_ia5_char_encode(in[y]);
}
*outlen = x;
return CRYPT_OK;
}
#endif
