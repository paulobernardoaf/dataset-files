













#include "tomcrypt_private.h"

#if defined(LTC_CHACHA)








int chacha_keystream(chacha_state *st, unsigned char *out, unsigned long outlen)
{
if (outlen == 0) return CRYPT_OK; 
LTC_ARGCHK(out != NULL);
XMEMSET(out, 0, outlen);
return chacha_crypt(st, out, outlen, out);
}

#endif




