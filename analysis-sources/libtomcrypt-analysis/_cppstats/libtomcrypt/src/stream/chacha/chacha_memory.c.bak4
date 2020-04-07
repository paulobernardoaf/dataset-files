








#include "tomcrypt_private.h"

#if defined(LTC_CHACHA)













int chacha_memory(const unsigned char *key, unsigned long keylen, unsigned long rounds,
const unsigned char *iv, unsigned long ivlen, ulong64 counter,
const unsigned char *datain, unsigned long datalen, unsigned char *dataout)
{
chacha_state st;
int err;

LTC_ARGCHK(ivlen <= 8 || counter < 4294967296); 

if ((err = chacha_setup(&st, key, keylen, rounds)) != CRYPT_OK) goto WIPE_KEY;
if (ivlen > 8) {
if ((err = chacha_ivctr32(&st, iv, ivlen, (ulong32)counter)) != CRYPT_OK) goto WIPE_KEY;
} else {
if ((err = chacha_ivctr64(&st, iv, ivlen, counter)) != CRYPT_OK) goto WIPE_KEY;
}
err = chacha_crypt(&st, datain, datalen, dataout);
WIPE_KEY:
chacha_done(&st);
return err;
}

#endif 




