#include "tomcrypt_private.h"
#if defined(LTC_DER)
int der_decode_object_identifier(const unsigned char *in, unsigned long inlen,
unsigned long *words, unsigned long *outlen)
{
unsigned long x, y, t, len;
int err;
LTC_ARGCHK(in != NULL);
LTC_ARGCHK(words != NULL);
LTC_ARGCHK(outlen != NULL);
if (inlen < 3) {
return CRYPT_INVALID_PACKET;
}
if (*outlen < 2) {
*outlen = 2;
return CRYPT_BUFFER_OVERFLOW;
}
x = 0;
if ((in[x++] & 0x1F) != 0x06) {
return CRYPT_INVALID_PACKET;
}
y = inlen - x;
if ((err = der_decode_asn1_length(in + x, &y, &len)) != CRYPT_OK) {
return err;
}
x += y;
if ((len == 0) || (len > (inlen - x))) {
return CRYPT_INVALID_PACKET;
}
y = 0;
t = 0;
while (len--) {
t = (t << 7) | (in[x] & 0x7F);
if (!(in[x++] & 0x80)) {
if (y >= *outlen) {
y++;
} else {
if (y == 0) {
if (t <= 79) {
words[0] = t / 40;
words[1] = t % 40;
} else {
words[0] = 2;
words[1] = t - 80;
}
y = 2;
} else {
words[y++] = t;
}
}
t = 0;
}
}
if (y > *outlen) {
err = CRYPT_BUFFER_OVERFLOW;
} else {
err = CRYPT_OK;
}
*outlen = y;
return err;
}
#endif
