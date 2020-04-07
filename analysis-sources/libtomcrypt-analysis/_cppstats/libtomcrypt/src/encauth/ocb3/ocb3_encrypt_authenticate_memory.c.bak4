












#include "tomcrypt_private.h"

#if defined(LTC_OCB3_MODE)

















int ocb3_encrypt_authenticate_memory(int cipher,
const unsigned char *key, unsigned long keylen,
const unsigned char *nonce, unsigned long noncelen,
const unsigned char *adata, unsigned long adatalen,
const unsigned char *pt, unsigned long ptlen,
unsigned char *ct,
unsigned char *tag, unsigned long *taglen)
{
int err;
ocb3_state *ocb;

LTC_ARGCHK(taglen != NULL);


ocb = XMALLOC(sizeof(ocb3_state));
if (ocb == NULL) {
return CRYPT_MEM;
}

if ((err = ocb3_init(ocb, cipher, key, keylen, nonce, noncelen, *taglen)) != CRYPT_OK) {
goto LBL_ERR;
}

if (adata != NULL || adatalen != 0) {
if ((err = ocb3_add_aad(ocb, adata, adatalen)) != CRYPT_OK) {
goto LBL_ERR;
}
}

if ((err = ocb3_encrypt_last(ocb, pt, ptlen, ct)) != CRYPT_OK) {
goto LBL_ERR;
}

err = ocb3_done(ocb, tag, taglen);

LBL_ERR:
#if defined(LTC_CLEAN_STACK)
zeromem(ocb, sizeof(ocb3_state));
#endif

XFREE(ocb);
return err;
}

#endif




