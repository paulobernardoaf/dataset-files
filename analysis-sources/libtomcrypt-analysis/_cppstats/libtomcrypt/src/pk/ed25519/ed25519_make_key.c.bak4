







#include "tomcrypt_private.h"






#if defined(LTC_CURVE25519)








int ed25519_make_key(prng_state *prng, int wprng, curve25519_key *key)
{
int err;

LTC_ARGCHK(prng != NULL);
LTC_ARGCHK(key != NULL);

if ((err = tweetnacl_crypto_sign_keypair(prng, wprng, key->pub, key->priv)) != CRYPT_OK) {
return err;
}

key->type = PK_PRIVATE;
key->algo = PKA_ED25519;

return err;
}

#endif




