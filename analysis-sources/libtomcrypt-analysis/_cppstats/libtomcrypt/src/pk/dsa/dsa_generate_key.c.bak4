







#include "tomcrypt_private.h"






#if defined(LTC_MDSA)








int dsa_generate_key(prng_state *prng, int wprng, dsa_key *key)
{
int err;

LTC_ARGCHK(key != NULL);
LTC_ARGCHK(ltc_mp.name != NULL);





if ((err = rand_bn_upto(key->x, key->q, prng, wprng)) != CRYPT_OK) { return err; }
if ((err = mp_exptmod(key->g, key->x, key->p, key->y)) != CRYPT_OK) { return err; }
key->type = PK_PRIVATE;

return CRYPT_OK;
}

#endif




