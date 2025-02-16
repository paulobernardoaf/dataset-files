#include "tomcrypt_private.h"
#if defined(LTC_BLAKE2BMAC)
int blake2bmac_init(blake2bmac_state *st, unsigned long outlen, const unsigned char *key, unsigned long keylen)
{
LTC_ARGCHK(st != NULL);
LTC_ARGCHK(key != NULL);
return blake2b_init(st, outlen, key, keylen);
}
int blake2bmac_process(blake2bmac_state *st, const unsigned char *in, unsigned long inlen)
{
if (inlen == 0) return CRYPT_OK; 
LTC_ARGCHK(st != NULL);
LTC_ARGCHK(in != NULL);
return blake2b_process(st, in, inlen);
}
int blake2bmac_done(blake2bmac_state *st, unsigned char *mac, unsigned long *maclen)
{
LTC_ARGCHK(st != NULL);
LTC_ARGCHK(mac != NULL);
LTC_ARGCHK(maclen != NULL);
LTC_ARGCHK(*maclen >= st->blake2b.outlen);
*maclen = st->blake2b.outlen;
return blake2b_done(st, mac);
}
#endif
