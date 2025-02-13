#include "tomcrypt_private.h"
#if defined(LTC_MECC)
int ecc_make_key(prng_state *prng, int wprng, int keysize, ecc_key *key)
{
int err;
if ((err = ecc_set_curve_by_size(keysize, key)) != CRYPT_OK) { return err; }
if ((err = ecc_generate_key(prng, wprng, key)) != CRYPT_OK) { return err; }
return CRYPT_OK;
}
int ecc_make_key_ex(prng_state *prng, int wprng, ecc_key *key, const ltc_ecc_curve *cu)
{
int err;
if ((err = ecc_set_curve(cu, key)) != CRYPT_OK) { return err; }
if ((err = ecc_generate_key(prng, wprng, key)) != CRYPT_OK) { return err; }
return CRYPT_OK;
}
int ecc_generate_key(prng_state *prng, int wprng, ecc_key *key)
{
int err;
LTC_ARGCHK(ltc_mp.name != NULL);
LTC_ARGCHK(key != NULL);
LTC_ARGCHK(key->dp.size > 0);
if ((err = rand_bn_upto(key->k, key->dp.order, prng, wprng)) != CRYPT_OK) {
goto error;
}
if ((err = ltc_mp.ecc_ptmul(key->k, &key->dp.base, &key->pubkey, key->dp.A, key->dp.prime, 1)) != CRYPT_OK) {
goto error;
}
key->type = PK_PRIVATE;
err = CRYPT_OK;
goto cleanup;
error:
ecc_free(key);
cleanup:
return err;
}
#endif
