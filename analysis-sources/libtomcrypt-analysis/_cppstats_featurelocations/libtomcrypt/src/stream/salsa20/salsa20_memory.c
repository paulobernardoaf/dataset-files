








#include "tomcrypt_private.h"

#if defined(LTC_SALSA20)













int salsa20_memory(const unsigned char *key, unsigned long keylen, unsigned long rounds,
const unsigned char *iv, unsigned long ivlen, ulong64 counter,
const unsigned char *datain, unsigned long datalen, unsigned char *dataout)
{
salsa20_state st;
int err;

if ((err = salsa20_setup(&st, key, keylen, rounds)) != CRYPT_OK) goto WIPE_KEY;
if ((err = salsa20_ivctr64(&st, iv, ivlen, counter)) != CRYPT_OK) goto WIPE_KEY;
err = salsa20_crypt(&st, datain, datalen, dataout);
WIPE_KEY:
salsa20_done(&st);
return err;
}

#endif 




