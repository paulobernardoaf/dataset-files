







#include "tomcrypt_private.h"






#if defined(LTC_CURVE25519)










int ed25519_import_raw(const unsigned char *in, unsigned long inlen, int which, curve25519_key *key)
{
LTC_ARGCHK(in != NULL);
LTC_ARGCHK(inlen == 32uL);
LTC_ARGCHK(key != NULL);

if (which == PK_PRIVATE) {
XMEMCPY(key->priv, in, sizeof(key->priv));
tweetnacl_crypto_sk_to_pk(key->pub, key->priv);
} else if (which == PK_PUBLIC) {
XMEMCPY(key->pub, in, sizeof(key->pub));
} else {
return CRYPT_INVALID_ARG;
}
key->algo = PKA_ED25519;
key->type = which;

return CRYPT_OK;
}

#endif




