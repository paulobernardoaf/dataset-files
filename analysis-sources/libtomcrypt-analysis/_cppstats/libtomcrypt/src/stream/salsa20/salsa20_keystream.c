#include "tomcrypt_private.h"
#if defined(LTC_SALSA20)
int salsa20_keystream(salsa20_state *st, unsigned char *out, unsigned long outlen)
{
if (outlen == 0) return CRYPT_OK; 
LTC_ARGCHK(out != NULL);
XMEMSET(out, 0, outlen);
return salsa20_crypt(st, out, outlen, out);
}
#endif
