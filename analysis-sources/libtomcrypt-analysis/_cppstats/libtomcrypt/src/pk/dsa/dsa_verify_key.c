#include "tomcrypt_private.h"
#if defined(LTC_MDSA)
int dsa_verify_key(const dsa_key *key, int *stat)
{
int err;
err = dsa_int_validate_primes(key, stat);
if (err != CRYPT_OK || *stat == 0) return err;
err = dsa_int_validate_pqg(key, stat);
if (err != CRYPT_OK || *stat == 0) return err;
return dsa_int_validate_xy(key, stat);
}
int dsa_int_validate_pqg(const dsa_key *key, int *stat)
{
void *tmp1, *tmp2;
int err;
LTC_ARGCHK(key != NULL);
LTC_ARGCHK(stat != NULL);
*stat = 0;
if ( key->qord >= LTC_MDSA_MAX_GROUP || key->qord <= 15 ||
(unsigned long)key->qord >= mp_unsigned_bin_size(key->p) ||
(mp_unsigned_bin_size(key->p) - key->qord) >= LTC_MDSA_DELTA ) {
return CRYPT_OK;
}
if (mp_cmp_d(key->g, 1) != LTC_MP_GT || mp_cmp(key->g, key->p) != LTC_MP_LT) {
return CRYPT_OK;
}
if ((err = mp_init_multi(&tmp1, &tmp2, NULL)) != CRYPT_OK) { return err; }
if ((err = mp_sub_d(key->p, 1, tmp1)) != CRYPT_OK) { goto error; }
if ((err = mp_div(tmp1, key->q, tmp1, tmp2)) != CRYPT_OK) { goto error; }
if (mp_iszero(tmp2) != LTC_MP_YES) {
err = CRYPT_OK;
goto error;
}
if ((err = mp_exptmod(key->g, key->q, key->p, tmp1)) != CRYPT_OK) { goto error; }
if (mp_cmp_d(tmp1, 1) != LTC_MP_EQ) {
err = CRYPT_OK;
goto error;
}
err = CRYPT_OK;
*stat = 1;
error:
mp_clear_multi(tmp2, tmp1, NULL);
return err;
}
int dsa_int_validate_primes(const dsa_key *key, int *stat)
{
int err, res;
*stat = 0;
LTC_ARGCHK(key != NULL);
LTC_ARGCHK(stat != NULL);
if ((err = mp_prime_is_prime(key->q, LTC_MILLER_RABIN_REPS, &res)) != CRYPT_OK) {
return err;
}
if (res == LTC_MP_NO) {
return CRYPT_OK;
}
if ((err = mp_prime_is_prime(key->p, LTC_MILLER_RABIN_REPS, &res)) != CRYPT_OK) {
return err;
}
if (res == LTC_MP_NO) {
return CRYPT_OK;
}
*stat = 1;
return CRYPT_OK;
}
int dsa_int_validate_xy(const dsa_key *key, int *stat)
{
void *tmp;
int err;
*stat = 0;
LTC_ARGCHK(key != NULL);
LTC_ARGCHK(stat != NULL);
if ((err = mp_init(&tmp)) != CRYPT_OK) {
return err;
}
if ((err = mp_sub_d(key->p, 1, tmp)) != CRYPT_OK) {
goto error;
}
if (mp_cmp_d(key->y, 1) != LTC_MP_GT || mp_cmp(key->y, tmp) != LTC_MP_LT) {
err = CRYPT_OK;
goto error;
}
if (key->type == PK_PRIVATE) {
if (mp_cmp_d(key->x, 0) != LTC_MP_GT || mp_cmp(key->x, key->q) != LTC_MP_LT) {
err = CRYPT_OK;
goto error;
}
if ((err = mp_exptmod(key->g, key->x, key->p, tmp)) != CRYPT_OK) {
goto error;
}
if (mp_cmp(tmp, key->y) != LTC_MP_EQ) {
err = CRYPT_OK;
goto error;
}
}
else {
if ((err = mp_exptmod(key->y, key->q, key->p, tmp)) != CRYPT_OK) {
goto error;
}
if (mp_cmp_d(tmp, 1) != LTC_MP_EQ) {
err = CRYPT_OK;
goto error;
}
}
err = CRYPT_OK;
*stat = 1;
error:
mp_clear(tmp);
return err;
}
#endif
