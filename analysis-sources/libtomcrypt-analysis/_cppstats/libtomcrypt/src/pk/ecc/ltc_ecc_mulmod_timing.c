#include "tomcrypt_private.h"
#if defined(LTC_MECC)
#if defined(LTC_ECC_TIMING_RESISTANT)
int ltc_ecc_mulmod(void *k, const ecc_point *G, ecc_point *R, void *a, void *modulus, int map)
{
ecc_point *tG, *M[3];
int i, j, err, inf;
void *mp = NULL, *mu = NULL, *ma = NULL, *a_plus3 = NULL;
ltc_mp_digit buf;
int bitcnt, mode, digidx;
LTC_ARGCHK(k != NULL);
LTC_ARGCHK(G != NULL);
LTC_ARGCHK(R != NULL);
LTC_ARGCHK(modulus != NULL);
if ((err = ltc_ecc_is_point_at_infinity(G, modulus, &inf)) != CRYPT_OK) return err;
if (inf) {
return ltc_ecc_set_point_xyz(1, 1, 0, R);
}
if ((err = mp_montgomery_setup(modulus, &mp)) != CRYPT_OK) { goto error; }
if ((err = mp_init(&mu)) != CRYPT_OK) { goto error; }
if ((err = mp_montgomery_normalization(mu, modulus)) != CRYPT_OK) { goto error; }
if ((err = mp_init(&a_plus3)) != CRYPT_OK) { goto error; }
if ((err = mp_add_d(a, 3, a_plus3)) != CRYPT_OK) { goto error; }
if (mp_cmp(a_plus3, modulus) != LTC_MP_EQ) {
if ((err = mp_init(&ma)) != CRYPT_OK) { goto error; }
if ((err = mp_mulmod(a, mu, modulus, ma)) != CRYPT_OK) { goto error; }
}
for (i = 0; i < 3; i++) {
M[i] = ltc_ecc_new_point();
if (M[i] == NULL) {
for (j = 0; j < i; j++) {
ltc_ecc_del_point(M[j]);
}
mp_clear(mu);
mp_montgomery_free(mp);
return CRYPT_MEM;
}
}
tG = ltc_ecc_new_point();
if (tG == NULL) { err = CRYPT_MEM; goto done; }
if ((err = mp_mulmod(G->x, mu, modulus, tG->x)) != CRYPT_OK) { goto done; }
if ((err = mp_mulmod(G->y, mu, modulus, tG->y)) != CRYPT_OK) { goto done; }
if ((err = mp_mulmod(G->z, mu, modulus, tG->z)) != CRYPT_OK) { goto done; }
mp_clear(mu);
mu = NULL;
if ((err = ltc_ecc_copy_point(tG, M[0])) != CRYPT_OK) { goto done; }
if ((err = ltc_mp.ecc_ptdbl(tG, M[1], ma, modulus, mp)) != CRYPT_OK) { goto done; }
mode = 0;
bitcnt = 1;
buf = 0;
digidx = mp_get_digit_count(k) - 1;
for (;;) {
if (--bitcnt == 0) {
if (digidx == -1) {
break;
}
buf = mp_get_digit(k, digidx);
bitcnt = (int) MP_DIGIT_BIT;
--digidx;
}
i = (int)((buf >> (MP_DIGIT_BIT - 1)) & 1);
buf <<= 1;
if (mode == 0 && i == 0) {
if ((err = ltc_mp.ecc_ptadd(M[0], M[1], M[2], ma, modulus, mp)) != CRYPT_OK) { goto done; }
if ((err = ltc_mp.ecc_ptdbl(M[1], M[2], ma, modulus, mp)) != CRYPT_OK) { goto done; }
continue;
}
if (mode == 0 && i == 1) {
mode = 1;
if ((err = ltc_mp.ecc_ptadd(M[0], M[1], M[2], ma, modulus, mp)) != CRYPT_OK) { goto done; }
if ((err = ltc_mp.ecc_ptdbl(M[1], M[2], ma, modulus, mp)) != CRYPT_OK) { goto done; }
continue;
}
if ((err = ltc_mp.ecc_ptadd(M[0], M[1], M[i^1], ma, modulus, mp)) != CRYPT_OK) { goto done; }
if ((err = ltc_mp.ecc_ptdbl(M[i], M[i], ma, modulus, mp)) != CRYPT_OK) { goto done; }
}
if ((err = ltc_ecc_copy_point(M[0], R)) != CRYPT_OK) { goto done; }
if (map) {
err = ltc_ecc_map(R, modulus, mp);
} else {
err = CRYPT_OK;
}
done:
ltc_ecc_del_point(tG);
for (i = 0; i < 3; i++) {
ltc_ecc_del_point(M[i]);
}
error:
if (ma != NULL) mp_clear(ma);
if (a_plus3 != NULL) mp_clear(a_plus3);
if (mu != NULL) mp_clear(mu);
if (mp != NULL) mp_montgomery_free(mp);
return err;
}
#endif
#endif
