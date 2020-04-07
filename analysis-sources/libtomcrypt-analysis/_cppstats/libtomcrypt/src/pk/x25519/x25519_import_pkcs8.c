#include "tomcrypt_private.h"
#if defined(LTC_CURVE25519)
int x25519_import_pkcs8(const unsigned char *in, unsigned long inlen,
const void *pwd, unsigned long pwdlen,
curve25519_key *key)
{
return ec25519_import_pkcs8(in, inlen, pwd, pwdlen, PKA_X25519, tweetnacl_crypto_scalarmult_base, key);
}
#endif
