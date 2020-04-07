#include "tomcrypt_private.h"
#if defined(LTC_CHACHA)
int chacha_done(chacha_state *st)
{
LTC_ARGCHK(st != NULL);
zeromem(st, sizeof(chacha_state));
return CRYPT_OK;
}
#endif
