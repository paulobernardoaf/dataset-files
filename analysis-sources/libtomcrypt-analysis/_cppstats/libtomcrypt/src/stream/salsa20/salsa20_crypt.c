#include "tomcrypt_private.h"
#if defined(LTC_SALSA20)
#define QUARTERROUND(a,b,c,d) x[b] ^= (ROL((x[a] + x[d]), 7)); x[c] ^= (ROL((x[b] + x[a]), 9)); x[d] ^= (ROL((x[c] + x[b]), 13)); x[a] ^= (ROL((x[d] + x[c]), 18));
static void _salsa20_block(unsigned char *output, const ulong32 *input, int rounds)
{
ulong32 x[16];
int i;
XMEMCPY(x, input, sizeof(x));
for (i = rounds; i > 0; i -= 2) {
QUARTERROUND( 0, 4, 8,12)
QUARTERROUND( 5, 9,13, 1)
QUARTERROUND(10,14, 2, 6)
QUARTERROUND(15, 3, 7,11)
QUARTERROUND( 0, 1, 2, 3)
QUARTERROUND( 5, 6, 7, 4)
QUARTERROUND(10,11, 8, 9)
QUARTERROUND(15,12,13,14)
}
for (i = 0; i < 16; ++i) {
x[i] += input[i];
STORE32L(x[i], output + 4 * i);
}
}
int salsa20_crypt(salsa20_state *st, const unsigned char *in, unsigned long inlen, unsigned char *out)
{
unsigned char buf[64];
unsigned long i, j;
if (inlen == 0) return CRYPT_OK; 
LTC_ARGCHK(st != NULL);
LTC_ARGCHK(in != NULL);
LTC_ARGCHK(out != NULL);
LTC_ARGCHK(st->ivlen == 8 || st->ivlen == 24);
if (st->ksleft > 0) {
j = MIN(st->ksleft, inlen);
for (i = 0; i < j; ++i, st->ksleft--) out[i] = in[i] ^ st->kstream[64 - st->ksleft];
inlen -= j;
if (inlen == 0) return CRYPT_OK;
out += j;
in += j;
}
for (;;) {
_salsa20_block(buf, st->input, st->rounds);
if (0 == ++st->input[8] && 0 == ++st->input[9]) return CRYPT_OVERFLOW;
if (inlen <= 64) {
for (i = 0; i < inlen; ++i) out[i] = in[i] ^ buf[i];
st->ksleft = 64 - inlen;
for (i = inlen; i < 64; ++i) st->kstream[i] = buf[i];
return CRYPT_OK;
}
for (i = 0; i < 64; ++i) out[i] = in[i] ^ buf[i];
inlen -= 64;
out += 64;
in += 64;
}
}
#endif
