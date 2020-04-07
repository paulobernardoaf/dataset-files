#include "tomcrypt_private.h"
#if defined(LTC_SOBER128_STREAM)
int sober128_stream_memory(const unsigned char *key, unsigned long keylen,
const unsigned char *iv, unsigned long ivlen,
const unsigned char *datain, unsigned long datalen,
unsigned char *dataout)
{
sober128_state st;
int err;
if ((err = sober128_stream_setup(&st, key, keylen)) != CRYPT_OK) goto WIPE_KEY;
if ((err = sober128_stream_setiv(&st, iv, ivlen)) != CRYPT_OK) goto WIPE_KEY;
err = sober128_stream_crypt(&st, datain, datalen, dataout);
WIPE_KEY:
sober128_stream_done(&st);
return err;
}
#endif 
