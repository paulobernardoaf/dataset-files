







#include "tomcrypt_private.h"






#if defined(LTC_DER)








int der_encode_object_identifier(const unsigned long *words, unsigned long nwords,
unsigned char *out, unsigned long *outlen)
{
unsigned long i, x, y, z, t, mask, wordbuf;
int err;

LTC_ARGCHK(words != NULL);
LTC_ARGCHK(out != NULL);
LTC_ARGCHK(outlen != NULL);


if ((err = der_length_object_identifier(words, nwords, &x)) != CRYPT_OK) {
return err;
}
if (x > *outlen) {
*outlen = x;
return CRYPT_BUFFER_OVERFLOW;
}


z = 0;
wordbuf = words[0] * 40 + words[1];
for (y = 1; y < nwords; y++) {
t = der_object_identifier_bits(wordbuf);
z += t/7 + ((t%7) ? 1 : 0) + (wordbuf == 0 ? 1 : 0);
if (y < nwords - 1) {
wordbuf = words[y + 1];
}
}


x = 0;
out[x++] = 0x06;
y = *outlen - x;
if ((err = der_encode_asn1_length(z, out + x, &y)) != CRYPT_OK) {
return err;
}
x += y;


wordbuf = words[0] * 40 + words[1];
for (i = 1; i < nwords; i++) {

t = wordbuf & 0xFFFFFFFF;
if (t) {
y = x;
mask = 0;
while (t) {
out[x++] = (unsigned char)((t & 0x7F) | mask);
t >>= 7;
mask |= 0x80; 
}

z = x - 1;
while (y < z) {
t = out[y]; out[y] = out[z]; out[z] = (unsigned char)t;
++y;
--z;
}
} else {

out[x++] = 0x00;
}

if (i < nwords - 1) {
wordbuf = words[i + 1];
}
}

*outlen = x;
return CRYPT_OK;
}

#endif




