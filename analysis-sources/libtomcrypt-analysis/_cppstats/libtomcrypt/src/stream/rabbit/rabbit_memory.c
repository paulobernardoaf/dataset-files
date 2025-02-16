#include "tomcrypt_private.h"
#if defined(LTC_RABBIT)
int rabbit_memory(const unsigned char *key, unsigned long keylen,
const unsigned char *iv, unsigned long ivlen,
const unsigned char *datain, unsigned long datalen,
unsigned char *dataout)
{
rabbit_state st;
int err;
if ((err = rabbit_setup(&st, key, keylen)) != CRYPT_OK) goto WIPE_KEY;
if ((err = rabbit_setiv(&st, iv, ivlen)) != CRYPT_OK) goto WIPE_KEY;
err = rabbit_crypt(&st, datain, datalen, dataout);
WIPE_KEY:
rabbit_done(&st);
return err;
}
#endif 
