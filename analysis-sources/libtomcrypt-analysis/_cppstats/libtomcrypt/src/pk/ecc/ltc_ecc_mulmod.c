#include "tomcrypt_private.h"
#if defined(LTC_MECC)
#if !defined(LTC_ECC_TIMING_RESISTANT)
#define WINSIZE 4
int ltc_ecc_mulmod(void *k, const ecc_point *G, ecc_point *R, void *a, void *modulus, int map)
{
ecc_point *tG, *M[8];
int i, j, err, inf;
void *mp = NULL, *mu = NULL, *ma = NULL, *a_plus3 = NULL;
ltc_mp_digit buf;
int first, bitbuf, bitcpy, bitcnt, mode, digidx;
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
for (i = 0; i < 8; i++) {
M[i] = ltc_ecc_new_point();
if (M[i] == NULL) {
for (j = 0; j < i; j++) {
ltc_ecc_del_point(M[j]);
}
err = CRYPT_MEM;
goto error;
}
}
tG = ltc_ecc_new_point();
if (tG == NULL) { err = CRYPT_MEM; goto done; }
if (mp_cmp_d(mu, 1) == LTC_MP_EQ) {
if ((err = ltc_ecc_copy_point(G, tG)) != CRYPT_OK) { goto done; }
} else {
if ((err = mp_mulmod(G->x, mu, modulus, tG->x)) != CRYPT_OK) { goto done; }
if ((err = mp_mulmod(G->y, mu, modulus, tG->y)) != CRYPT_OK) { goto done; }
if ((err = mp_mulmod(G->z, mu, modulus, tG->z)) != CRYPT_OK) { goto done; }
}
mp_clear(mu);
mu = NULL;
if ((err = ltc_mp.ecc_ptdbl(tG, M[0], ma, modulus, mp)) != CRYPT_OK) { goto done; }
if ((err = ltc_mp.ecc_ptdbl(M[0], M[0], ma, modulus, mp)) != CRYPT_OK) { goto done; }
if ((err = ltc_mp.ecc_ptdbl(M[0], M[0], ma, modulus, mp)) != CRYPT_OK) { goto done; }
for (j = 9; j < 16; j++) {
if ((err = ltc_mp.ecc_ptadd(M[j-9], tG, M[j-8], ma, modulus, mp)) != CRYPT_OK) { goto done; }
}
mode = 0;
bitcnt = 1;
buf = 0;
digidx = mp_get_digit_count(k) - 1;
bitcpy = bitbuf = 0;
first = 1;
for (;;) {
if (--bitcnt == 0) {
if (digidx == -1) {
break;
}
buf = mp_get_digit(k, digidx);
bitcnt = (int) ltc_mp.bits_per_digit;
--digidx;
}
i = (buf >> (ltc_mp.bits_per_digit - 1)) & 1;
buf <<= 1;
if (mode == 0 && i == 0) {
continue;
}
if (mode == 1 && i == 0) {
if ((err = ltc_mp.ecc_ptdbl(R, R, ma, modulus, mp)) != CRYPT_OK) { goto done; }
continue;
}
bitbuf |= (i << (WINSIZE - ++bitcpy));
mode = 2;
if (bitcpy == WINSIZE) {
if (first == 1) {
if ((err = ltc_ecc_copy_point(M[bitbuf-8], R)) != CRYPT_OK) { goto done; }
first = 0;
} else {
for (j = 0; j < WINSIZE; j++) {
if ((err = ltc_mp.ecc_ptdbl(R, R, ma, modulus, mp)) != CRYPT_OK) { goto done; }
}
if ((err = ltc_mp.ecc_ptadd(R, M[bitbuf-8], R, ma, modulus, mp)) != CRYPT_OK) { goto done; }
}
bitcpy = bitbuf = 0;
mode = 1;
}
}
if (mode == 2 && bitcpy > 0) {
for (j = 0; j < bitcpy; j++) {
if (first == 0) {
if ((err = ltc_mp.ecc_ptdbl(R, R, ma, modulus, mp)) != CRYPT_OK) { goto done; }
}
bitbuf <<= 1;
if ((bitbuf & (1 << WINSIZE)) != 0) {
if (first == 1){
if ((err = ltc_ecc_copy_point(tG, R)) != CRYPT_OK) { goto done; }
first = 0;
} else {
if ((err = ltc_mp.ecc_ptadd(R, tG, R, ma, modulus, mp)) != CRYPT_OK) { goto done; }
}
}
}
}
if (map) {
err = ltc_ecc_map(R, modulus, mp);
} else {
err = CRYPT_OK;
}
done:
ltc_ecc_del_point(tG);
for (i = 0; i < 8; i++) {
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
#undef WINSIZE
#endif
