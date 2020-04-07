#include "tomcrypt_private.h"
#if defined(LTC_BASE32)
int base32_decode(const char *in, unsigned long inlen,
unsigned char *out, unsigned long *outlen,
base32_alphabet id)
{
unsigned long x;
int y = 0;
ulong64 t = 0;
char c;
const unsigned char *map;
const unsigned char tables[4][43] = {
{ 
99,99,26,27,28,29,30,31,99,99,
99,99,99,99,99,99,99,
0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,
13,14,15,16,17,18,19,20,21,22,23,24,25
},
{ 
0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
99,99,99,99,99,99,99,
10,11,12,13,14,15,16,17,18,19,20,21,22,
23,24,25,26,27,28,29,30,31,99,99,99,99
},
{ 
99,18,99,25,26,27,30,29, 7,31,
99,99,99,99,99,99,99,
24, 1,12, 3, 8, 5, 6,28,21, 9,10,99,11,
2,16,13,14, 4,22,17,19,99,20,15, 0,23
},
{ 
0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
99,99,99,99,99,99,99,
10,11,12,13,14,15,16,17, 1,18,19, 1,20,
21, 0,22,23,24,25,26,99,27,28,29,30,31
}
};
LTC_ARGCHK(in != NULL);
LTC_ARGCHK(out != NULL);
LTC_ARGCHK(outlen != NULL);
LTC_ARGCHK(id >= BASE32_RFC4648);
LTC_ARGCHK(id <= BASE32_CROCKFORD);
while (inlen > 0 && in[inlen-1] == '=') inlen--;
if (inlen == 0) {
*outlen = 0;
return CRYPT_OK;
}
x = (inlen * 5) / 8;
if (*outlen < x) {
*outlen = x;
return CRYPT_BUFFER_OVERFLOW;
}
*outlen = x;
x = inlen % 8;
if (x == 1 || x == 3 || x == 6) {
return CRYPT_INVALID_PACKET;
}
map = tables[id];
for (x = 0; x < inlen; x++) {
c = in[x];
if ((c >= 'a') && (c <= 'z')) c -= 32;
if (c < '0' || c > 'Z' || map[c-'0'] > 31) {
return CRYPT_INVALID_PACKET;
}
t = (t<<5) | map[c-'0'];
if (++y == 8) {
*out++ = (unsigned char)((t>>32) & 255);
*out++ = (unsigned char)((t>>24) & 255);
*out++ = (unsigned char)((t>>16) & 255);
*out++ = (unsigned char)((t>> 8) & 255);
*out++ = (unsigned char)( t & 255);
y = 0;
t = 0;
}
}
if (y > 0) {
t = t << (5 * (8 - y));
if (y >= 2) *out++ = (unsigned char)((t>>32) & 255);
if (y >= 4) *out++ = (unsigned char)((t>>24) & 255);
if (y >= 5) *out++ = (unsigned char)((t>>16) & 255);
if (y >= 7) *out++ = (unsigned char)((t>> 8) & 255);
}
return CRYPT_OK;
}
#endif
