







#include "tomcrypt_private.h"






#if defined(LTC_PELICAN)

#define __LTC_AES_TAB_C__
#define ENCRYPT_ONLY
#define PELI_TAB
#include "../../ciphers/aes/aes_tab.c"








int pelican_init(pelican_state *pelmac, const unsigned char *key, unsigned long keylen)
{
int err;

LTC_ARGCHK(pelmac != NULL);
LTC_ARGCHK(key != NULL);

#if defined(LTC_FAST)
if (16 % sizeof(LTC_FAST_TYPE)) {
return CRYPT_INVALID_ARG;
}
#endif

if ((err = aes_setup(key, keylen, 0, &pelmac->K)) != CRYPT_OK) {
return err;
}

zeromem(pelmac->state, 16);
aes_ecb_encrypt(pelmac->state, pelmac->state, &pelmac->K);
pelmac->buflen = 0;

return CRYPT_OK;
}

static void _four_rounds(pelican_state *pelmac)
{
ulong32 s0, s1, s2, s3, t0, t1, t2, t3;
int r;

LOAD32H(s0, pelmac->state );
LOAD32H(s1, pelmac->state + 4);
LOAD32H(s2, pelmac->state + 8);
LOAD32H(s3, pelmac->state + 12);
for (r = 0; r < 4; r++) {
t0 =
Te0(LTC_BYTE(s0, 3)) ^
Te1(LTC_BYTE(s1, 2)) ^
Te2(LTC_BYTE(s2, 1)) ^
Te3(LTC_BYTE(s3, 0));
t1 =
Te0(LTC_BYTE(s1, 3)) ^
Te1(LTC_BYTE(s2, 2)) ^
Te2(LTC_BYTE(s3, 1)) ^
Te3(LTC_BYTE(s0, 0));
t2 =
Te0(LTC_BYTE(s2, 3)) ^
Te1(LTC_BYTE(s3, 2)) ^
Te2(LTC_BYTE(s0, 1)) ^
Te3(LTC_BYTE(s1, 0));
t3 =
Te0(LTC_BYTE(s3, 3)) ^
Te1(LTC_BYTE(s0, 2)) ^
Te2(LTC_BYTE(s1, 1)) ^
Te3(LTC_BYTE(s2, 0));
s0 = t0; s1 = t1; s2 = t2; s3 = t3;
}
STORE32H(s0, pelmac->state );
STORE32H(s1, pelmac->state + 4);
STORE32H(s2, pelmac->state + 8);
STORE32H(s3, pelmac->state + 12);
}








int pelican_process(pelican_state *pelmac, const unsigned char *in, unsigned long inlen)
{

LTC_ARGCHK(pelmac != NULL);
LTC_ARGCHK(in != NULL);


if (pelmac->buflen < 0 || pelmac->buflen > 15) {
return CRYPT_INVALID_ARG;
}

#if defined(LTC_FAST)
if (pelmac->buflen == 0) {
while (inlen & ~15) {
int x;
for (x = 0; x < 16; x += sizeof(LTC_FAST_TYPE)) {
*(LTC_FAST_TYPE_PTR_CAST((unsigned char *)pelmac->state + x)) ^= *(LTC_FAST_TYPE_PTR_CAST((unsigned char *)in + x));
}
_four_rounds(pelmac);
in += 16;
inlen -= 16;
}
}
#endif

while (inlen--) {
pelmac->state[pelmac->buflen++] ^= *in++;
if (pelmac->buflen == 16) {
_four_rounds(pelmac);
pelmac->buflen = 0;
}
}
return CRYPT_OK;
}







int pelican_done(pelican_state *pelmac, unsigned char *out)
{
LTC_ARGCHK(pelmac != NULL);
LTC_ARGCHK(out != NULL);


if (pelmac->buflen < 0 || pelmac->buflen > 16) {
return CRYPT_INVALID_ARG;
}

if (pelmac->buflen == 16) {
_four_rounds(pelmac);
pelmac->buflen = 0;
}
pelmac->state[pelmac->buflen++] ^= 0x80;
aes_ecb_encrypt(pelmac->state, out, &pelmac->K);
aes_done(&pelmac->K);
return CRYPT_OK;
}

#endif




