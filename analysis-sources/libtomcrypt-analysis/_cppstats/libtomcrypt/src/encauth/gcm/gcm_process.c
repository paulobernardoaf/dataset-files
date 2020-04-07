#include "tomcrypt_private.h"
#if defined(LTC_GCM_MODE)
int gcm_process(gcm_state *gcm,
unsigned char *pt, unsigned long ptlen,
unsigned char *ct,
int direction)
{
unsigned long x;
int y, err;
unsigned char b;
LTC_ARGCHK(gcm != NULL);
if (ptlen > 0) {
LTC_ARGCHK(pt != NULL);
LTC_ARGCHK(ct != NULL);
}
if (gcm->buflen > 16 || gcm->buflen < 0) {
return CRYPT_INVALID_ARG;
}
if ((err = cipher_is_valid(gcm->cipher)) != CRYPT_OK) {
return err;
}
if (gcm->pttotlen / 8 + (ulong64)gcm->buflen + (ulong64)ptlen >= CONST64(0xFFFFFFFE0)) {
return CRYPT_INVALID_ARG;
}
if (gcm->mode == LTC_GCM_MODE_IV) {
if ((err = gcm_add_aad(gcm, NULL, 0)) != CRYPT_OK) return err;
}
if (gcm->mode == LTC_GCM_MODE_AAD) {
if (gcm->buflen) {
gcm->totlen += gcm->buflen * CONST64(8);
gcm_mult_h(gcm, gcm->X);
}
for (y = 15; y >= 12; y--) {
if (++gcm->Y[y] & 255) { break; }
}
if ((err = cipher_descriptor[gcm->cipher].ecb_encrypt(gcm->Y, gcm->buf, &gcm->K)) != CRYPT_OK) {
return err;
}
gcm->buflen = 0;
gcm->mode = LTC_GCM_MODE_TEXT;
}
if (gcm->mode != LTC_GCM_MODE_TEXT) {
return CRYPT_INVALID_ARG;
}
x = 0;
#if defined(LTC_FAST)
if (gcm->buflen == 0) {
if (direction == GCM_ENCRYPT) {
for (x = 0; x < (ptlen & ~15); x += 16) {
for (y = 0; y < 16; y += sizeof(LTC_FAST_TYPE)) {
*(LTC_FAST_TYPE_PTR_CAST(&ct[x + y])) = *(LTC_FAST_TYPE_PTR_CAST(&pt[x+y])) ^ *(LTC_FAST_TYPE_PTR_CAST(&gcm->buf[y]));
*(LTC_FAST_TYPE_PTR_CAST(&gcm->X[y])) ^= *(LTC_FAST_TYPE_PTR_CAST(&ct[x+y]));
}
gcm->pttotlen += 128;
gcm_mult_h(gcm, gcm->X);
for (y = 15; y >= 12; y--) {
if (++gcm->Y[y] & 255) { break; }
}
if ((err = cipher_descriptor[gcm->cipher].ecb_encrypt(gcm->Y, gcm->buf, &gcm->K)) != CRYPT_OK) {
return err;
}
}
} else {
for (x = 0; x < (ptlen & ~15); x += 16) {
for (y = 0; y < 16; y += sizeof(LTC_FAST_TYPE)) {
*(LTC_FAST_TYPE_PTR_CAST(&gcm->X[y])) ^= *(LTC_FAST_TYPE_PTR_CAST(&ct[x+y]));
*(LTC_FAST_TYPE_PTR_CAST(&pt[x + y])) = *(LTC_FAST_TYPE_PTR_CAST(&ct[x+y])) ^ *(LTC_FAST_TYPE_PTR_CAST(&gcm->buf[y]));
}
gcm->pttotlen += 128;
gcm_mult_h(gcm, gcm->X);
for (y = 15; y >= 12; y--) {
if (++gcm->Y[y] & 255) { break; }
}
if ((err = cipher_descriptor[gcm->cipher].ecb_encrypt(gcm->Y, gcm->buf, &gcm->K)) != CRYPT_OK) {
return err;
}
}
}
}
#endif
for (; x < ptlen; x++) {
if (gcm->buflen == 16) {
gcm->pttotlen += 128;
gcm_mult_h(gcm, gcm->X);
for (y = 15; y >= 12; y--) {
if (++gcm->Y[y] & 255) { break; }
}
if ((err = cipher_descriptor[gcm->cipher].ecb_encrypt(gcm->Y, gcm->buf, &gcm->K)) != CRYPT_OK) {
return err;
}
gcm->buflen = 0;
}
if (direction == GCM_ENCRYPT) {
b = ct[x] = pt[x] ^ gcm->buf[gcm->buflen];
} else {
b = ct[x];
pt[x] = ct[x] ^ gcm->buf[gcm->buflen];
}
gcm->X[gcm->buflen++] ^= b;
}
return CRYPT_OK;
}
#endif
