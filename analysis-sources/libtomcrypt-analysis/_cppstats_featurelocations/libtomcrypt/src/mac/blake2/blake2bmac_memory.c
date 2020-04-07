








#include "tomcrypt_private.h"

#if defined(LTC_BLAKE2BMAC)











int blake2bmac_memory(const unsigned char *key, unsigned long keylen, const unsigned char *in, unsigned long inlen, unsigned char *mac, unsigned long *maclen)
{
blake2bmac_state st;
int err;

LTC_ARGCHK(key != NULL);
LTC_ARGCHK(in != NULL);
LTC_ARGCHK(mac != NULL);
LTC_ARGCHK(maclen != NULL);

if ((err = blake2bmac_init(&st, *maclen, key, keylen)) != CRYPT_OK) { goto LBL_ERR; }
if ((err = blake2bmac_process(&st, in, inlen)) != CRYPT_OK) { goto LBL_ERR; }
err = blake2bmac_done(&st, mac, maclen);
LBL_ERR:
#if defined(LTC_CLEAN_STACK)
zeromem(&st, sizeof(blake2bmac_state));
#endif
return err;
}

#endif




