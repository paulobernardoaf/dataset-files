#include "tomcrypt_private.h"
#if defined(LTC_MECC)
int ltc_ecc_verify_key(const ecc_key *key)
{
int err, inf;
ecc_point *point;
void *prime = key->dp.prime;
void *order = key->dp.order;
void *a = key->dp.A;
if (ltc_mp.compare_d(key->pubkey.z, 1) == LTC_MP_EQ) {
if ((ltc_mp.compare(key->pubkey.x, prime) != LTC_MP_LT) ||
(ltc_mp.compare(key->pubkey.y, prime) != LTC_MP_LT) ||
(ltc_mp.compare_d(key->pubkey.x, 0) == LTC_MP_LT) ||
(ltc_mp.compare_d(key->pubkey.y, 0) == LTC_MP_LT) ||
(mp_iszero(key->pubkey.x) && mp_iszero(key->pubkey.y))
)
{
err = CRYPT_INVALID_PACKET;
goto done2;
}
}
if ((err = ltc_ecc_is_point(&key->dp, key->pubkey.x, key->pubkey.y)) != CRYPT_OK) { goto done2; }
point = ltc_ecc_new_point();
if ((err = ltc_ecc_mulmod(order, &(key->pubkey), point, a, prime, 1)) != CRYPT_OK) { goto done1; }
err = ltc_ecc_is_point_at_infinity(point, prime, &inf);
if (err != CRYPT_OK || inf) {
err = CRYPT_ERROR;
}
else {
err = CRYPT_OK;
}
done1:
ltc_ecc_del_point(point);
done2:
return err;
}
#endif
