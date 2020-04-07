








#include "tomcrypt_private.h"
#include <stdarg.h>

#if defined(LTC_BLAKE2BMAC)












int blake2bmac_memory_multi(const unsigned char *key, unsigned long keylen, unsigned char *mac, unsigned long *maclen, const unsigned char *in, unsigned long inlen, ...)
{
blake2bmac_state st;
int err;
va_list args;
const unsigned char *curptr;
unsigned long curlen;

LTC_ARGCHK(key != NULL);
LTC_ARGCHK(in != NULL);
LTC_ARGCHK(mac != NULL);
LTC_ARGCHK(maclen != NULL);

va_start(args, inlen);
curptr = in;
curlen = inlen;
if ((err = blake2bmac_init(&st, *maclen, key, keylen)) != CRYPT_OK) { goto LBL_ERR; }
for (;;) {
if ((err = blake2bmac_process(&st, curptr, curlen)) != CRYPT_OK) { goto LBL_ERR; }
curptr = va_arg(args, const unsigned char*);
if (curptr == NULL) break;
curlen = va_arg(args, unsigned long);
}
err = blake2bmac_done(&st, mac, maclen);
LBL_ERR:
#if defined(LTC_CLEAN_STACK)
zeromem(&st, sizeof(blake2bmac_state));
#endif
va_end(args);
return err;
}

#endif




