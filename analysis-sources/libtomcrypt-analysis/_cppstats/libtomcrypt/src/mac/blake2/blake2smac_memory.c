#include "tomcrypt_private.h"
#if defined(LTC_BLAKE2SMAC)
int blake2smac_memory(const unsigned char *key, unsigned long keylen, const unsigned char *in, unsigned long inlen, unsigned char *mac, unsigned long *maclen)
{
blake2smac_state st;
int err;
LTC_ARGCHK(key != NULL);
LTC_ARGCHK(in != NULL);
LTC_ARGCHK(mac != NULL);
LTC_ARGCHK(maclen != NULL);
if ((err = blake2smac_init(&st, *maclen, key, keylen)) != CRYPT_OK) { goto LBL_ERR; }
if ((err = blake2smac_process(&st, in, inlen)) != CRYPT_OK) { goto LBL_ERR; }
err = blake2smac_done(&st, mac, maclen);
LBL_ERR:
#if defined(LTC_CLEAN_STACK)
zeromem(&st, sizeof(blake2smac_state));
#endif
return err;
}
#endif
