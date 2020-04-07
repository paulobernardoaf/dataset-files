#include "tomcrypt_private.h"
#if defined(LTC_DER)
#define SETBIT(v, n) (v=((unsigned char)(v) | (1U << (unsigned char)(n))))
#define CLRBIT(v, n) (v=((unsigned char)(v) & ~(1U << (unsigned char)(n))))
int der_decode_raw_bit_string(const unsigned char *in, unsigned long inlen,
unsigned char *out, unsigned long *outlen)
{
unsigned long dlen, blen, x, y;
int err;
LTC_ARGCHK(in != NULL);
LTC_ARGCHK(out != NULL);
LTC_ARGCHK(outlen != NULL);
if (inlen < 4) {
return CRYPT_INVALID_ARG;
}
if ((in[0]&0x1F) != 0x03) {
return CRYPT_INVALID_PACKET;
}
x = 1;
y = inlen - 1;
if ((err = der_decode_asn1_length(in + x, &y, &dlen)) != CRYPT_OK) {
return err;
}
x += y;
if ((dlen == 0) || (dlen > (inlen - x))) {
return CRYPT_INVALID_PACKET;
}
blen = ((dlen - 1) << 3) - (in[x++] & 7);
if (blen > *outlen) {
*outlen = blen;
return CRYPT_BUFFER_OVERFLOW;
}
for (y = 0; y < blen; y++) {
if (in[x] & (1 << (7 - (y & 7)))) {
SETBIT(out[y/8], 7-(y%8));
} else {
CLRBIT(out[y/8], 7-(y%8));
}
if ((y & 7) == 7) {
++x;
}
}
*outlen = blen;
return CRYPT_OK;
}
#endif
