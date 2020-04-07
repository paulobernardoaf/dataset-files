








#include "tomcrypt_private.h"








#if defined(LTC_BASE16)









int base16_decode(const char *in, unsigned long inlen,
unsigned char *out, unsigned long *outlen)
{
unsigned long pos, out_len;
unsigned char idx0, idx1;
char in0, in1;

const unsigned char hashmap[] = {
0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 
0x08, 0x09, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0xff, 
};

LTC_ARGCHK(in != NULL);
LTC_ARGCHK(out != NULL);
LTC_ARGCHK(outlen != NULL);

if ((inlen % 2) == 1) return CRYPT_INVALID_PACKET;
out_len = *outlen * 2;
for (pos = 0; ((pos + 1 < out_len) && (pos + 1 < inlen)); pos += 2) {
in0 = in[pos + 0];
in1 = in[pos + 1];

if ((in0 < '0') || (in0 > 'g')) return CRYPT_INVALID_PACKET;
if ((in1 < '0') || (in1 > 'g')) return CRYPT_INVALID_PACKET;

idx0 = (unsigned char) (in0 & 0x1F) ^ 0x10;
idx1 = (unsigned char) (in1 & 0x1F) ^ 0x10;

if (hashmap[idx0] == 0xff) return CRYPT_INVALID_PACKET;
if (hashmap[idx1] == 0xff) return CRYPT_INVALID_PACKET;

out[pos / 2] = (unsigned char) (hashmap[idx0] << 4) | hashmap[idx1];
}
*outlen = pos / 2;
return CRYPT_OK;
}

#endif




