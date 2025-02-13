












#include "tomcrypt_private.h"

#if defined(LTC_OCB3_MODE)


















int ocb3_decrypt_verify_memory(int cipher,
const unsigned char *key, unsigned long keylen,
const unsigned char *nonce, unsigned long noncelen,
const unsigned char *adata, unsigned long adatalen,
const unsigned char *ct, unsigned long ctlen,
unsigned char *pt,
const unsigned char *tag, unsigned long taglen,
int *stat)
{
int err;
ocb3_state *ocb;
unsigned char *buf;
unsigned long buflen;

LTC_ARGCHK(stat != NULL);


*stat = 0;


taglen = MIN(taglen, MAXBLOCKSIZE);


buf = XMALLOC(taglen);
ocb = XMALLOC(sizeof(ocb3_state));
if (ocb == NULL || buf == NULL) {
if (ocb != NULL) {
XFREE(ocb);
}
if (buf != NULL) {
XFREE(buf);
}
return CRYPT_MEM;
}

if ((err = ocb3_init(ocb, cipher, key, keylen, nonce, noncelen, taglen)) != CRYPT_OK) {
goto LBL_ERR;
}

if (adata != NULL || adatalen != 0) {
if ((err = ocb3_add_aad(ocb, adata, adatalen)) != CRYPT_OK) {
goto LBL_ERR;
}
}

if ((err = ocb3_decrypt_last(ocb, ct, ctlen, pt)) != CRYPT_OK) {
goto LBL_ERR;
}

buflen = taglen;
if ((err = ocb3_done(ocb, buf, &buflen)) != CRYPT_OK) {
goto LBL_ERR;
}


if (buflen >= taglen && XMEM_NEQ(buf, tag, taglen) == 0) {
*stat = 1;
}

err = CRYPT_OK;

LBL_ERR:
#if defined(LTC_CLEAN_STACK)
zeromem(ocb, sizeof(ocb3_state));
#endif

XFREE(ocb);
XFREE(buf);
return err;
}

#endif




