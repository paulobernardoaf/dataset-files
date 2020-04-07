












#include "tomcrypt_private.h"

#if defined(LTC_GCM_MODE)


















int gcm_memory( int cipher,
const unsigned char *key, unsigned long keylen,
const unsigned char *IV, unsigned long IVlen,
const unsigned char *adata, unsigned long adatalen,
unsigned char *pt, unsigned long ptlen,
unsigned char *ct,
unsigned char *tag, unsigned long *taglen,
int direction)
{
void *orig;
gcm_state *gcm;
int err;

if ((err = cipher_is_valid(cipher)) != CRYPT_OK) {
return err;
}

if (cipher_descriptor[cipher].accel_gcm_memory != NULL) {
return cipher_descriptor[cipher].accel_gcm_memory
(key, keylen,
IV, IVlen,
adata, adatalen,
pt, ptlen,
ct,
tag, taglen,
direction);
}



#if !defined(LTC_GCM_TABLES_SSE2)
orig = gcm = XMALLOC(sizeof(*gcm));
#else
orig = gcm = XMALLOC(sizeof(*gcm) + 16);
#endif
if (gcm == NULL) {
return CRYPT_MEM;
}





#if defined(LTC_GCM_TABLES_SSE2)
if ((unsigned long)gcm & 15) {
gcm = (gcm_state *)((unsigned long)gcm + (16 - ((unsigned long)gcm & 15)));
}
#endif

if ((err = gcm_init(gcm, cipher, key, keylen)) != CRYPT_OK) {
goto LTC_ERR;
}
if ((err = gcm_add_iv(gcm, IV, IVlen)) != CRYPT_OK) {
goto LTC_ERR;
}
if ((err = gcm_add_aad(gcm, adata, adatalen)) != CRYPT_OK) {
goto LTC_ERR;
}
if ((err = gcm_process(gcm, pt, ptlen, ct, direction)) != CRYPT_OK) {
goto LTC_ERR;
}
if (direction == GCM_ENCRYPT) {
if ((err = gcm_done(gcm, tag, taglen)) != CRYPT_OK) {
goto LTC_ERR;
}
}
else if (direction == GCM_DECRYPT) {
unsigned char buf[MAXBLOCKSIZE];
unsigned long buflen = sizeof(buf);
if ((err = gcm_done(gcm, buf, &buflen)) != CRYPT_OK) {
goto LTC_ERR;
}
if (buflen != *taglen || XMEM_NEQ(buf, tag, buflen) != 0) {
err = CRYPT_ERROR;
}
}
else {
err = CRYPT_INVALID_ARG;
}
LTC_ERR:
XFREE(orig);
return err;
}
#endif





