#include "tomcrypt_private.h"
#if defined(LTC_RC4_STREAM)
int rc4_stream_memory(const unsigned char *key, unsigned long keylen,
const unsigned char *datain, unsigned long datalen,
unsigned char *dataout)
{
rc4_state st;
int err;
if ((err = rc4_stream_setup(&st, key, keylen)) != CRYPT_OK) goto WIPE_KEY;
err = rc4_stream_crypt(&st, datain, datalen, dataout);
WIPE_KEY:
rc4_stream_done(&st);
return err;
}
#endif 
