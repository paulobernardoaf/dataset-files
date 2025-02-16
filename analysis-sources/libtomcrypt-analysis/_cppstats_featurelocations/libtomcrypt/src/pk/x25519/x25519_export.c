







#include "tomcrypt_private.h"






#if defined(LTC_CURVE25519)









int x25519_export( unsigned char *out, unsigned long *outlen,
int which,
const curve25519_key *key)
{
LTC_ARGCHK(key != NULL);

if (key->algo != PKA_X25519) return CRYPT_PK_INVALID_TYPE;

return ec25519_export(out, outlen, which, key);
}

#endif




