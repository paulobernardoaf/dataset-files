







#include "tomcrypt_private.h"






#if defined(LTC_CURVE25519)








int x25519_make_key(prng_state *prng, int wprng, curve25519_key *key)
{
int err;

LTC_ARGCHK(prng != NULL);
LTC_ARGCHK(key != NULL);

if ((err = prng_is_valid(wprng)) != CRYPT_OK) {
return err;
}

if (prng_descriptor[wprng].read(key->priv, sizeof(key->priv), prng) != sizeof(key->priv)) {
return CRYPT_ERROR_READPRNG;
}

tweetnacl_crypto_scalarmult_base(key->pub, key->priv);

key->type = PK_PRIVATE;
key->algo = PKA_X25519;

return err;
}

#endif




