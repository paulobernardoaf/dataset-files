#include "tomcrypt_private.h"
#if defined(LTC_SOSEMANUK)
int sosemanuk_memory(const unsigned char *key, unsigned long keylen,
const unsigned char *iv, unsigned long ivlen,
const unsigned char *datain, unsigned long datalen,
unsigned char *dataout)
{
sosemanuk_state st;
int err;
if ((err = sosemanuk_setup(&st, key, keylen)) != CRYPT_OK) goto WIPE_KEY;
if ((err = sosemanuk_setiv(&st, iv, ivlen)) != CRYPT_OK) goto WIPE_KEY;
err = sosemanuk_crypt(&st, datain, datalen, dataout);
WIPE_KEY:
sosemanuk_done(&st);
return err;
}
#endif 
