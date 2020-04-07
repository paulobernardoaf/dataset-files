








#include "tomcrypt_private.h"

#if defined(LTC_SALSA20)






int salsa20_done(salsa20_state *st)
{
LTC_ARGCHK(st != NULL);
zeromem(st, sizeof(salsa20_state));
return CRYPT_OK;
}

#endif




