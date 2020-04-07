#include "tomcrypt_private.h"
#if defined(LTC_BLAKE2SMAC)
int blake2smac_init(blake2smac_state *st, unsigned long outlen, const unsigned char *key, unsigned long keylen)
{
LTC_ARGCHK(st != NULL);
LTC_ARGCHK(key != NULL);
return blake2s_init(st, outlen, key, keylen);
}
int blake2smac_process(blake2smac_state *st, const unsigned char *in, unsigned long inlen)
{
if (inlen == 0) return CRYPT_OK; 
LTC_ARGCHK(st != NULL);
LTC_ARGCHK(in != NULL);
return blake2s_process(st, in, inlen);
}
int blake2smac_done(blake2smac_state *st, unsigned char *mac, unsigned long *maclen)
{
LTC_ARGCHK(st != NULL);
LTC_ARGCHK(mac != NULL);
LTC_ARGCHK(maclen != NULL);
LTC_ARGCHK(*maclen >= st->blake2s.outlen);
*maclen = st->blake2s.outlen;
return blake2s_done(st, mac);
}
#endif
