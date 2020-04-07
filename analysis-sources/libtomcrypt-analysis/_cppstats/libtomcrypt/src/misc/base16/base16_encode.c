#include "tomcrypt_private.h"
#if defined(LTC_BASE16)
int base16_encode(const unsigned char *in, unsigned long inlen,
char *out, unsigned long *outlen,
unsigned int options)
{
unsigned long i, x;
const char *alphabet;
const char *alphabets[2] = {
"0123456789abcdef",
"0123456789ABCDEF",
};
LTC_ARGCHK(in != NULL);
LTC_ARGCHK(out != NULL);
LTC_ARGCHK(outlen != NULL);
x = inlen * 2 + 1;
if (x < inlen) return CRYPT_OVERFLOW;
if (*outlen < x) {
*outlen = x;
return CRYPT_BUFFER_OVERFLOW;
}
x--;
*outlen = x; 
if (options == 0) {
alphabet = alphabets[0];
} else {
alphabet = alphabets[1];
}
for (i = 0; i < x; i += 2) {
out[i] = alphabet[(in[i/2] >> 4) & 0x0f];
out[i+1] = alphabet[in[i/2] & 0x0f];
}
out[x] = '\0';
return CRYPT_OK;
}
#endif
