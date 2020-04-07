







#include "tomcrypt_private.h"







#if defined(LTC_DER)

#define getbit(n, k) (((n) & ( 1 << (k) )) >> (k))









int der_encode_raw_bit_string(const unsigned char *in, unsigned long inlen,
unsigned char *out, unsigned long *outlen)
{
unsigned long len, x, y;
unsigned char buf;
int err;

LTC_ARGCHK(in != NULL);
LTC_ARGCHK(out != NULL);
LTC_ARGCHK(outlen != NULL);


if ((err = der_length_bit_string(inlen, &len)) != CRYPT_OK) {
return err;
}

if (len > *outlen) {
*outlen = len;
return CRYPT_BUFFER_OVERFLOW;
}


x = 0;
y = ((inlen + 7) >> 3) + 1;

out[x++] = 0x03;
len = *outlen - x;
if ((err = der_encode_asn1_length(y, out + x, &len)) != CRYPT_OK) {
return err;
}
x += len;


out[x++] = (unsigned char)((8 - inlen) & 7);


for (y = buf = 0; y < inlen; y++) {
buf |= (getbit(in[y/8],7-y%8)?1:0) << (7 - (y & 7));
if ((y & 7) == 7) {
out[x++] = buf;
buf = 0;
}
}

if (inlen & 7) {
out[x++] = buf;
}

*outlen = x;
return CRYPT_OK;
}

#endif




