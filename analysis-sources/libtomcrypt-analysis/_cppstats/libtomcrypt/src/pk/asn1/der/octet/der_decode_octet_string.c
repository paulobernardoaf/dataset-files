#include "tomcrypt_private.h"
#if defined(LTC_DER)
int der_decode_octet_string(const unsigned char *in, unsigned long inlen,
unsigned char *out, unsigned long *outlen)
{
unsigned long x, y, len;
int err;
LTC_ARGCHK(in != NULL);
LTC_ARGCHK(out != NULL);
LTC_ARGCHK(outlen != NULL);
if (inlen < 2) {
return CRYPT_INVALID_PACKET;
}
if ((in[0] & 0x1F) != 0x04) {
return CRYPT_INVALID_PACKET;
}
x = 1;
y = inlen - x;
if ((err = der_decode_asn1_length(in + x, &y, &len)) != CRYPT_OK) {
return err;
}
x += y;
if (len > *outlen) {
*outlen = len;
return CRYPT_BUFFER_OVERFLOW;
}
if (len > (inlen - x)) {
return CRYPT_INVALID_PACKET;
}
for (y = 0; y < len; y++) {
out[y] = in[x++];
}
*outlen = y;
return CRYPT_OK;
}
#endif
