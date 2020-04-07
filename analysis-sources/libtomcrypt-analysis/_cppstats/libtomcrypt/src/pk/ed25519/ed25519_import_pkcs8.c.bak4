







#include "tomcrypt_private.h"






#if defined(LTC_CURVE25519)










int ed25519_import_pkcs8(const unsigned char *in, unsigned long inlen,
const void *pwd, unsigned long pwdlen,
curve25519_key *key)
{
return ec25519_import_pkcs8(in, inlen, pwd, pwdlen, PKA_ED25519, tweetnacl_crypto_sk_to_pk, key);
}

#endif




