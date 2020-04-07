







#include "tomcrypt_private.h"







#if defined(LTC_DER)









int der_encode_octet_string(const unsigned char *in, unsigned long inlen,
unsigned char *out, unsigned long *outlen)
{
unsigned long x, y, len;
int err;

LTC_ARGCHK(in != NULL);
LTC_ARGCHK(out != NULL);
LTC_ARGCHK(outlen != NULL);


if ((err = der_length_octet_string(inlen, &len)) != CRYPT_OK) {
return err;
}


if (len > *outlen) {
*outlen = len;
return CRYPT_BUFFER_OVERFLOW;
}


x = 0;
out[x++] = 0x04;
len = *outlen - x;
if ((err = der_encode_asn1_length(inlen, out + x, &len)) != CRYPT_OK) {
return err;
}
x += len;


for (y = 0; y < inlen; y++) {
out[x++] = in[y];
}


*outlen = x;

return CRYPT_OK;
}

#endif




