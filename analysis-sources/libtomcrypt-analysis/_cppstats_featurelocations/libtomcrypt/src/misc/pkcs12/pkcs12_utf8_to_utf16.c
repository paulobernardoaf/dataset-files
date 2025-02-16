








#include "tomcrypt_private.h"

#if defined(LTC_PKCS_12)

int pkcs12_utf8_to_utf16(const unsigned char *in, unsigned long inlen,
unsigned char *out, unsigned long *outlen) {
unsigned long len = 0;
const unsigned char* in_end = in + inlen;
const ulong32 offset[6] = {
0x00000000UL, 0x00003080UL, 0x000E2080UL,
0x03C82080UL, 0xFA082080UL, 0x82082080UL
};
int err = CRYPT_ERROR;

LTC_ARGCHK(in != NULL);
LTC_ARGCHK(out != NULL);
LTC_ARGCHK(outlen != NULL);

while (in < in_end) {
ulong32 ch = 0;
unsigned short extra = 0; 
if (*in >= 192) extra++; 
if (*in >= 224) extra++; 
if (*in >= 240) extra++; 
if (*in >= 248) extra++; 
if (*in >= 252) extra++; 
if (in + extra >= in_end) goto ERROR;
switch (extra) {
case 5: ch += *in++; ch <<= 6;

case 4: ch += *in++; ch <<= 6;

case 3: ch += *in++; ch <<= 6;

case 2: ch += *in++; ch <<= 6;

case 1: ch += *in++; ch <<= 6;

case 0: ch += *in++;
}
ch -= offset[extra];
if (ch > 0xFFFF) goto ERROR;
if (*outlen >= len + 2) {
out[len] = (unsigned short)((ch >> 8) & 0xFF);
out[len + 1] = (unsigned char)(ch & 0xFF);
}
len += 2;
}

err = len > *outlen ? CRYPT_BUFFER_OVERFLOW : CRYPT_OK;
*outlen = len;
ERROR:
return err;
}

#endif




