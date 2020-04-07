#include "tomcrypt_private.h"
#if defined(LTC_XTS_MODE)
static int _tweak_uncrypt(const unsigned char *C, unsigned char *P, unsigned char *T, const symmetric_xts *xts)
{
unsigned long x;
int err;
#if defined(LTC_FAST)
for (x = 0; x < 16; x += sizeof(LTC_FAST_TYPE)) {
*(LTC_FAST_TYPE_PTR_CAST(&P[x])) = *(LTC_FAST_TYPE_PTR_CAST(&C[x])) ^ *(LTC_FAST_TYPE_PTR_CAST(&T[x]));
}
#else
for (x = 0; x < 16; x++) {
P[x] = C[x] ^ T[x];
}
#endif
err = cipher_descriptor[xts->cipher].ecb_decrypt(P, P, &xts->key1);
#if defined(LTC_FAST)
for (x = 0; x < 16; x += sizeof(LTC_FAST_TYPE)) {
*(LTC_FAST_TYPE_PTR_CAST(&P[x])) ^= *(LTC_FAST_TYPE_PTR_CAST(&T[x]));
}
#else
for (x = 0; x < 16; x++) {
P[x] = P[x] ^ T[x];
}
#endif
xts_mult_x(T);
return err;
}
int xts_decrypt(const unsigned char *ct, unsigned long ptlen, unsigned char *pt, unsigned char *tweak,
const symmetric_xts *xts)
{
unsigned char PP[16], CC[16], T[16];
unsigned long i, m, mo, lim;
int err;
LTC_ARGCHK(pt != NULL);
LTC_ARGCHK(ct != NULL);
LTC_ARGCHK(tweak != NULL);
LTC_ARGCHK(xts != NULL);
if ((err = cipher_is_valid(xts->cipher)) != CRYPT_OK) {
return err;
}
m = ptlen >> 4;
mo = ptlen & 15;
if (m == 0) {
return CRYPT_INVALID_ARG;
}
if (mo == 0) {
lim = m;
} else {
lim = m - 1;
}
if (cipher_descriptor[xts->cipher].accel_xts_decrypt && lim > 0) {
if ((err = cipher_descriptor[xts->cipher].accel_xts_decrypt(ct, pt, lim, tweak, &xts->key1, &xts->key2)) !=
CRYPT_OK) {
return err;
}
ct += lim * 16;
pt += lim * 16;
XMEMCPY(T, tweak, sizeof(T));
} else {
if ((err = cipher_descriptor[xts->cipher].ecb_encrypt(tweak, T, &xts->key2)) != CRYPT_OK) {
return err;
}
for (i = 0; i < lim; i++) {
if ((err = _tweak_uncrypt(ct, pt, T, xts)) != CRYPT_OK) {
return err;
}
ct += 16;
pt += 16;
}
}
if (mo > 0) {
XMEMCPY(CC, T, 16);
xts_mult_x(CC);
if ((err = _tweak_uncrypt(ct, PP, CC, xts)) != CRYPT_OK) {
return err;
}
for (i = 0; i < mo; i++) {
CC[i] = ct[16 + i];
pt[16 + i] = PP[i];
}
for (; i < 16; i++) {
CC[i] = PP[i];
}
if ((err = _tweak_uncrypt(CC, pt, T, xts)) != CRYPT_OK) {
return err;
}
}
if ((err = cipher_descriptor[xts->cipher].ecb_decrypt(T, tweak, &xts->key2)) != CRYPT_OK) {
return err;
}
return CRYPT_OK;
}
#endif
