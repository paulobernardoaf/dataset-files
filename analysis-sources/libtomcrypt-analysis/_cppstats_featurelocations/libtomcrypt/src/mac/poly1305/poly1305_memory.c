













#include "tomcrypt_private.h"

#if defined(LTC_POLY1305)











int poly1305_memory(const unsigned char *key, unsigned long keylen, const unsigned char *in, unsigned long inlen, unsigned char *mac, unsigned long *maclen)
{
poly1305_state st;
int err;

LTC_ARGCHK(key != NULL);
LTC_ARGCHK(in != NULL);
LTC_ARGCHK(mac != NULL);
LTC_ARGCHK(maclen != NULL);

if ((err = poly1305_init(&st, key, keylen)) != CRYPT_OK) { goto LBL_ERR; }
if ((err = poly1305_process(&st, in, inlen)) != CRYPT_OK) { goto LBL_ERR; }
err = poly1305_done(&st, mac, maclen);
LBL_ERR:
#if defined(LTC_CLEAN_STACK)
zeromem(&st, sizeof(poly1305_state));
#endif
return err;
}

#endif




