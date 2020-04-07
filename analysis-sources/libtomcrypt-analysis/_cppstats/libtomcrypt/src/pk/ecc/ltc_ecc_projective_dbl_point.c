#include "tomcrypt_private.h"
#if defined(LTC_MECC) && (!defined(LTC_MECC_ACCEL) || defined(LTM_DESC))
int ltc_ecc_projective_dbl_point(const ecc_point *P, ecc_point *R, void *ma, void *modulus, void *mp)
{
void *t1, *t2;
int err, inf;
LTC_ARGCHK(P != NULL);
LTC_ARGCHK(R != NULL);
LTC_ARGCHK(modulus != NULL);
LTC_ARGCHK(mp != NULL);
if ((err = mp_init_multi(&t1, &t2, NULL)) != CRYPT_OK) {
return err;
}
if (P != R) {
if ((err = ltc_ecc_copy_point(P, R)) != CRYPT_OK) { goto done; }
}
if ((err = ltc_ecc_is_point_at_infinity(P, modulus, &inf)) != CRYPT_OK) return err;
if (inf) {
err = ltc_ecc_set_point_xyz(1, 1, 0, R);
goto done;
}
if ((err = mp_sqr(R->z, t1)) != CRYPT_OK) { goto done; }
if ((err = mp_montgomery_reduce(t1, modulus, mp)) != CRYPT_OK) { goto done; }
if ((err = mp_mul(R->z, R->y, R->z)) != CRYPT_OK) { goto done; }
if ((err = mp_montgomery_reduce(R->z, modulus, mp)) != CRYPT_OK) { goto done; }
if ((err = mp_add(R->z, R->z, R->z)) != CRYPT_OK) { goto done; }
if (mp_cmp(R->z, modulus) != LTC_MP_LT) {
if ((err = mp_sub(R->z, modulus, R->z)) != CRYPT_OK) { goto done; }
}
if (ma == NULL) { 
if ((err = mp_sub(R->x, t1, t2)) != CRYPT_OK) { goto done; }
if (mp_cmp_d(t2, 0) == LTC_MP_LT) {
if ((err = mp_add(t2, modulus, t2)) != CRYPT_OK) { goto done; }
}
if ((err = mp_add(t1, R->x, t1)) != CRYPT_OK) { goto done; }
if (mp_cmp(t1, modulus) != LTC_MP_LT) {
if ((err = mp_sub(t1, modulus, t1)) != CRYPT_OK) { goto done; }
}
if ((err = mp_mul(t1, t2, t2)) != CRYPT_OK) { goto done; }
if ((err = mp_montgomery_reduce(t2, modulus, mp)) != CRYPT_OK) { goto done; }
if ((err = mp_add(t2, t2, t1)) != CRYPT_OK) { goto done; }
if (mp_cmp(t1, modulus) != LTC_MP_LT) {
if ((err = mp_sub(t1, modulus, t1)) != CRYPT_OK) { goto done; }
}
if ((err = mp_add(t1, t2, t1)) != CRYPT_OK) { goto done; }
if (mp_cmp(t1, modulus) != LTC_MP_LT) {
if ((err = mp_sub(t1, modulus, t1)) != CRYPT_OK) { goto done; }
}
}
else {
if ((err = mp_sqr(t1, t2)) != CRYPT_OK) { goto done; }
if ((err = mp_montgomery_reduce(t2, modulus, mp)) != CRYPT_OK) { goto done; }
if ((err = mp_mul(t2, ma, t1)) != CRYPT_OK) { goto done; }
if ((err = mp_montgomery_reduce(t1, modulus, mp)) != CRYPT_OK) { goto done; }
if ((err = mp_sqr(R->x, t2)) != CRYPT_OK) { goto done; }
if ((err = mp_montgomery_reduce(t2, modulus, mp)) != CRYPT_OK) { goto done; }
if ((err = mp_add(t1, t2, t1)) != CRYPT_OK) { goto done; }
if (mp_cmp(t1, modulus) != LTC_MP_LT) {
if ((err = mp_sub(t1, modulus, t1)) != CRYPT_OK) { goto done; }
}
if ((err = mp_add(t1, t2, t1)) != CRYPT_OK) { goto done; }
if (mp_cmp(t1, modulus) != LTC_MP_LT) {
if ((err = mp_sub(t1, modulus, t1)) != CRYPT_OK) { goto done; }
}
if ((err = mp_add(t1, t2, t1)) != CRYPT_OK) { goto done; }
if (mp_cmp(t1, modulus) != LTC_MP_LT) {
if ((err = mp_sub(t1, modulus, t1)) != CRYPT_OK) { goto done; }
}
}
if ((err = mp_add(R->y, R->y, R->y)) != CRYPT_OK) { goto done; }
if (mp_cmp(R->y, modulus) != LTC_MP_LT) {
if ((err = mp_sub(R->y, modulus, R->y)) != CRYPT_OK) { goto done; }
}
if ((err = mp_sqr(R->y, R->y)) != CRYPT_OK) { goto done; }
if ((err = mp_montgomery_reduce(R->y, modulus, mp)) != CRYPT_OK) { goto done; }
if ((err = mp_sqr(R->y, t2)) != CRYPT_OK) { goto done; }
if ((err = mp_montgomery_reduce(t2, modulus, mp)) != CRYPT_OK) { goto done; }
if (mp_isodd(t2)) {
if ((err = mp_add(t2, modulus, t2)) != CRYPT_OK) { goto done; }
}
if ((err = mp_div_2(t2, t2)) != CRYPT_OK) { goto done; }
if ((err = mp_mul(R->y, R->x, R->y)) != CRYPT_OK) { goto done; }
if ((err = mp_montgomery_reduce(R->y, modulus, mp)) != CRYPT_OK) { goto done; }
if ((err = mp_sqr(t1, R->x)) != CRYPT_OK) { goto done; }
if ((err = mp_montgomery_reduce(R->x, modulus, mp)) != CRYPT_OK) { goto done; }
if ((err = mp_sub(R->x, R->y, R->x)) != CRYPT_OK) { goto done; }
if (mp_cmp_d(R->x, 0) == LTC_MP_LT) {
if ((err = mp_add(R->x, modulus, R->x)) != CRYPT_OK) { goto done; }
}
if ((err = mp_sub(R->x, R->y, R->x)) != CRYPT_OK) { goto done; }
if (mp_cmp_d(R->x, 0) == LTC_MP_LT) {
if ((err = mp_add(R->x, modulus, R->x)) != CRYPT_OK) { goto done; }
}
if ((err = mp_sub(R->y, R->x, R->y)) != CRYPT_OK) { goto done; }
if (mp_cmp_d(R->y, 0) == LTC_MP_LT) {
if ((err = mp_add(R->y, modulus, R->y)) != CRYPT_OK) { goto done; }
}
if ((err = mp_mul(R->y, t1, R->y)) != CRYPT_OK) { goto done; }
if ((err = mp_montgomery_reduce(R->y, modulus, mp)) != CRYPT_OK) { goto done; }
if ((err = mp_sub(R->y, t2, R->y)) != CRYPT_OK) { goto done; }
if (mp_cmp_d(R->y, 0) == LTC_MP_LT) {
if ((err = mp_add(R->y, modulus, R->y)) != CRYPT_OK) { goto done; }
}
err = CRYPT_OK;
done:
mp_clear_multi(t2, t1, NULL);
return err;
}
#endif
