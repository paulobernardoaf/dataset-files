







#include "tomcrypt_private.h"






#if defined(LTC_MDSA)










int dsa_make_key(prng_state *prng, int wprng, int group_size, int modulus_size, dsa_key *key)
{
int err;

if ((err = dsa_generate_pqg(prng, wprng, group_size, modulus_size, key)) != CRYPT_OK) { return err; }
if ((err = dsa_generate_key(prng, wprng, key)) != CRYPT_OK) { return err; }

return CRYPT_OK;
}

#endif




