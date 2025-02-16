







#include "tomcrypt_private.h"







#if defined(LTC_DER)









int der_decode_utf8_string(const unsigned char *in, unsigned long inlen,
wchar_t *out, unsigned long *outlen)
{
wchar_t tmp;
unsigned long x, y, z, len;
int err;

LTC_ARGCHK(in != NULL);
LTC_ARGCHK(out != NULL);
LTC_ARGCHK(outlen != NULL);


if (inlen < 2) {
return CRYPT_INVALID_PACKET;
}


if ((in[0] & 0x1F) != 0x0C) {
return CRYPT_INVALID_PACKET;
}
x = 1;


y = inlen - x;
if ((err = der_decode_asn1_length(in + x, &y, &len)) != CRYPT_OK) {
return err;
}
x += y;

if (len > (inlen - x)) {
return CRYPT_INVALID_PACKET;
}


for (y = 0; x < inlen; ) {

tmp = in[x++];


for (z = 0; (tmp & 0x80) && (z <= 4); z++, tmp = (tmp << 1) & 0xFF);

if (z == 1 || z > 4 || (x + (z - 1) > inlen)) {
return CRYPT_INVALID_PACKET;
}


tmp >>= z;


if (z > 1) { --z; }
while (z-- != 0) {
if ((in[x] & 0xC0) != 0x80) {
return CRYPT_INVALID_PACKET;
}
tmp = (tmp << 6) | ((wchar_t)in[x++] & 0x3F);
}

if (y < *outlen) {
out[y] = tmp;
}
y++;
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




