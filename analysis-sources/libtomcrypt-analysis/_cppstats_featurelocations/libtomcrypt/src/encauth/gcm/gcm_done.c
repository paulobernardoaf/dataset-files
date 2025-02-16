












#include "tomcrypt_private.h"

#if defined(LTC_GCM_MODE)








int gcm_done(gcm_state *gcm,
unsigned char *tag, unsigned long *taglen)
{
unsigned long x;
int err;

LTC_ARGCHK(gcm != NULL);
LTC_ARGCHK(tag != NULL);
LTC_ARGCHK(taglen != NULL);

if (gcm->buflen > 16 || gcm->buflen < 0) {
return CRYPT_INVALID_ARG;
}

if ((err = cipher_is_valid(gcm->cipher)) != CRYPT_OK) {
return err;
}

if (gcm->mode == LTC_GCM_MODE_IV) {

if ((err = gcm_add_aad(gcm, NULL, 0)) != CRYPT_OK) return err;
}

if (gcm->mode == LTC_GCM_MODE_AAD) {

if ((err = gcm_process(gcm, NULL, 0, NULL, 0)) != CRYPT_OK) return err;
}

if (gcm->mode != LTC_GCM_MODE_TEXT) {
return CRYPT_INVALID_ARG;
}


if (gcm->buflen) {
gcm->pttotlen += gcm->buflen * CONST64(8);
gcm_mult_h(gcm, gcm->X);
}


STORE64H(gcm->totlen, gcm->buf);
STORE64H(gcm->pttotlen, gcm->buf+8);
for (x = 0; x < 16; x++) {
gcm->X[x] ^= gcm->buf[x];
}
gcm_mult_h(gcm, gcm->X);


if ((err = cipher_descriptor[gcm->cipher].ecb_encrypt(gcm->Y_0, gcm->buf, &gcm->K)) != CRYPT_OK) {
return err;
}
for (x = 0; x < 16 && x < *taglen; x++) {
tag[x] = gcm->buf[x] ^ gcm->X[x];
}
*taglen = x;

cipher_descriptor[gcm->cipher].done(&gcm->K);

return CRYPT_OK;
}

#endif





