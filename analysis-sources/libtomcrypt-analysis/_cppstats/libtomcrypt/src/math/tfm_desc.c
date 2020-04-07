#define DESC_DEF_ONLY
#include "tomcrypt_private.h"
#if defined(TFM_DESC)
#include <tfm.h>
static const struct {
int tfm_code, ltc_code;
} tfm_to_ltc_codes[] = {
{ FP_OKAY , CRYPT_OK},
{ FP_MEM , CRYPT_MEM},
{ FP_VAL , CRYPT_INVALID_ARG},
};
static int tfm_to_ltc_error(int err)
{
int x;
for (x = 0; x < (int)(sizeof(tfm_to_ltc_codes)/sizeof(tfm_to_ltc_codes[0])); x++) {
if (err == tfm_to_ltc_codes[x].tfm_code) {
return tfm_to_ltc_codes[x].ltc_code;
}
}
return CRYPT_ERROR;
}
static int init(void **a)
{
LTC_ARGCHK(a != NULL);
*a = XCALLOC(1, sizeof(fp_int));
if (*a == NULL) {
return CRYPT_MEM;
}
fp_init(*a);
return CRYPT_OK;
}
static void deinit(void *a)
{
LTC_ARGCHKVD(a != NULL);
XFREE(a);
}
static int neg(void *a, void *b)
{
LTC_ARGCHK(a != NULL);
LTC_ARGCHK(b != NULL);
fp_neg(((fp_int*)a), ((fp_int*)b));
return CRYPT_OK;
}
static int copy(void *a, void *b)
{
LTC_ARGCHK(a != NULL);
LTC_ARGCHK(b != NULL);
fp_copy(a, b);
return CRYPT_OK;
}
static int init_copy(void **a, void *b)
{
if (init(a) != CRYPT_OK) {
return CRYPT_MEM;
}
return copy(b, *a);
}
static int set_int(void *a, ltc_mp_digit b)
{
LTC_ARGCHK(a != NULL);
fp_set(a, b);
return CRYPT_OK;
}
static unsigned long get_int(void *a)
{
fp_int *A;
LTC_ARGCHK(a != NULL);
A = a;
return A->used > 0 ? A->dp[0] : 0;
}
static ltc_mp_digit get_digit(void *a, int n)
{
fp_int *A;
LTC_ARGCHK(a != NULL);
A = a;
return (n >= A->used || n < 0) ? 0 : A->dp[n];
}
static int get_digit_count(void *a)
{
fp_int *A;
LTC_ARGCHK(a != NULL);
A = a;
return A->used;
}
static int compare(void *a, void *b)
{
int ret;
LTC_ARGCHK(a != NULL);
LTC_ARGCHK(b != NULL);
ret = fp_cmp(a, b);
switch (ret) {
case FP_LT: return LTC_MP_LT;
case FP_EQ: return LTC_MP_EQ;
case FP_GT: return LTC_MP_GT;
}
return 0;
}
static int compare_d(void *a, ltc_mp_digit b)
{
int ret;
LTC_ARGCHK(a != NULL);
ret = fp_cmp_d(a, b);
switch (ret) {
case FP_LT: return LTC_MP_LT;
case FP_EQ: return LTC_MP_EQ;
case FP_GT: return LTC_MP_GT;
}
return 0;
}
static int count_bits(void *a)
{
LTC_ARGCHK(a != NULL);
return fp_count_bits(a);
}
static int count_lsb_bits(void *a)
{
LTC_ARGCHK(a != NULL);
return fp_cnt_lsb(a);
}
static int twoexpt(void *a, int n)
{
LTC_ARGCHK(a != NULL);
fp_2expt(a, n);
return CRYPT_OK;
}
static int read_radix(void *a, const char *b, int radix)
{
LTC_ARGCHK(a != NULL);
LTC_ARGCHK(b != NULL);
return tfm_to_ltc_error(fp_read_radix(a, (char *)b, radix));
}
static int write_radix(void *a, char *b, int radix)
{
LTC_ARGCHK(a != NULL);
LTC_ARGCHK(b != NULL);
return tfm_to_ltc_error(fp_toradix(a, b, radix));
}
static unsigned long unsigned_size(void *a)
{
LTC_ARGCHK(a != NULL);
return fp_unsigned_bin_size(a);
}
static int unsigned_write(void *a, unsigned char *b)
{
LTC_ARGCHK(a != NULL);
LTC_ARGCHK(b != NULL);
fp_to_unsigned_bin(a, b);
return CRYPT_OK;
}
static int unsigned_read(void *a, unsigned char *b, unsigned long len)
{
LTC_ARGCHK(a != NULL);
LTC_ARGCHK(b != NULL);
fp_read_unsigned_bin(a, b, len);
return CRYPT_OK;
}
static int add(void *a, void *b, void *c)
{
LTC_ARGCHK(a != NULL);
LTC_ARGCHK(b != NULL);
LTC_ARGCHK(c != NULL);
fp_add(a, b, c);
return CRYPT_OK;
}
static int addi(void *a, ltc_mp_digit b, void *c)
{
LTC_ARGCHK(a != NULL);
LTC_ARGCHK(c != NULL);
fp_add_d(a, b, c);
return CRYPT_OK;
}
static int sub(void *a, void *b, void *c)
{
LTC_ARGCHK(a != NULL);
LTC_ARGCHK(b != NULL);
LTC_ARGCHK(c != NULL);
fp_sub(a, b, c);
return CRYPT_OK;
}
static int subi(void *a, ltc_mp_digit b, void *c)
{
LTC_ARGCHK(a != NULL);
LTC_ARGCHK(c != NULL);
fp_sub_d(a, b, c);
return CRYPT_OK;
}
static int mul(void *a, void *b, void *c)
{
LTC_ARGCHK(a != NULL);
LTC_ARGCHK(b != NULL);
LTC_ARGCHK(c != NULL);
fp_mul(a, b, c);
return CRYPT_OK;
}
static int muli(void *a, ltc_mp_digit b, void *c)
{
LTC_ARGCHK(a != NULL);
LTC_ARGCHK(c != NULL);
fp_mul_d(a, b, c);
return CRYPT_OK;
}
static int sqr(void *a, void *b)
{
LTC_ARGCHK(a != NULL);
LTC_ARGCHK(b != NULL);
fp_sqr(a, b);
return CRYPT_OK;
}
static int divide(void *a, void *b, void *c, void *d)
{
LTC_ARGCHK(a != NULL);
LTC_ARGCHK(b != NULL);
return tfm_to_ltc_error(fp_div(a, b, c, d));
}
static int div_2(void *a, void *b)
{
LTC_ARGCHK(a != NULL);
LTC_ARGCHK(b != NULL);
fp_div_2(a, b);
return CRYPT_OK;
}
static int modi(void *a, ltc_mp_digit b, ltc_mp_digit *c)
{
fp_digit tmp;
int err;
LTC_ARGCHK(a != NULL);
LTC_ARGCHK(c != NULL);
if ((err = tfm_to_ltc_error(fp_mod_d(a, b, &tmp))) != CRYPT_OK) {
return err;
}
*c = tmp;
return CRYPT_OK;
}
static int gcd(void *a, void *b, void *c)
{
LTC_ARGCHK(a != NULL);
LTC_ARGCHK(b != NULL);
LTC_ARGCHK(c != NULL);
fp_gcd(a, b, c);
return CRYPT_OK;
}
static int lcm(void *a, void *b, void *c)
{
LTC_ARGCHK(a != NULL);
LTC_ARGCHK(b != NULL);
LTC_ARGCHK(c != NULL);
fp_lcm(a, b, c);
return CRYPT_OK;
}
static int addmod(void *a, void *b, void *c, void *d)
{
LTC_ARGCHK(a != NULL);
LTC_ARGCHK(b != NULL);
LTC_ARGCHK(c != NULL);
LTC_ARGCHK(d != NULL);
return tfm_to_ltc_error(fp_addmod(a,b,c,d));
}
static int submod(void *a, void *b, void *c, void *d)
{
LTC_ARGCHK(a != NULL);
LTC_ARGCHK(b != NULL);
LTC_ARGCHK(c != NULL);
LTC_ARGCHK(d != NULL);
return tfm_to_ltc_error(fp_submod(a,b,c,d));
}
static int mulmod(void *a, void *b, void *c, void *d)
{
LTC_ARGCHK(a != NULL);
LTC_ARGCHK(b != NULL);
LTC_ARGCHK(c != NULL);
LTC_ARGCHK(d != NULL);
return tfm_to_ltc_error(fp_mulmod(a,b,c,d));
}
static int sqrmod(void *a, void *b, void *c)
{
LTC_ARGCHK(a != NULL);
LTC_ARGCHK(b != NULL);
LTC_ARGCHK(c != NULL);
return tfm_to_ltc_error(fp_sqrmod(a,b,c));
}
static int invmod(void *a, void *b, void *c)
{
LTC_ARGCHK(a != NULL);
LTC_ARGCHK(b != NULL);
LTC_ARGCHK(c != NULL);
return tfm_to_ltc_error(fp_invmod(a, b, c));
}
static int montgomery_setup(void *a, void **b)
{
int err;
LTC_ARGCHK(a != NULL);
LTC_ARGCHK(b != NULL);
*b = XCALLOC(1, sizeof(fp_digit));
if (*b == NULL) {
return CRYPT_MEM;
}
if ((err = tfm_to_ltc_error(fp_montgomery_setup(a, (fp_digit *)*b))) != CRYPT_OK) {
XFREE(*b);
}
return err;
}
static int montgomery_normalization(void *a, void *b)
{
LTC_ARGCHK(a != NULL);
LTC_ARGCHK(b != NULL);
fp_montgomery_calc_normalization(a, b);
return CRYPT_OK;
}
static int montgomery_reduce(void *a, void *b, void *c)
{
LTC_ARGCHK(a != NULL);
LTC_ARGCHK(b != NULL);
LTC_ARGCHK(c != NULL);
fp_montgomery_reduce(a, b, *((fp_digit *)c));
return CRYPT_OK;
}
static void montgomery_deinit(void *a)
{
XFREE(a);
}
static int exptmod(void *a, void *b, void *c, void *d)
{
LTC_ARGCHK(a != NULL);
LTC_ARGCHK(b != NULL);
LTC_ARGCHK(c != NULL);
LTC_ARGCHK(d != NULL);
return tfm_to_ltc_error(fp_exptmod(a,b,c,d));
}
static int isprime(void *a, int b, int *c)
{
LTC_ARGCHK(a != NULL);
LTC_ARGCHK(c != NULL);
if (b == 0) {
b = LTC_MILLER_RABIN_REPS;
} 
*c = (fp_isprime_ex(a, b) == FP_YES) ? LTC_MP_YES : LTC_MP_NO;
return CRYPT_OK;
}
#if defined(LTC_MECC) && defined(LTC_MECC_ACCEL)
static int tfm_ecc_projective_dbl_point(const ecc_point *P, ecc_point *R, void *ma, void *modulus, void *Mp)
{
fp_int t1, t2;
fp_digit mp;
int err, inf;
LTC_ARGCHK(P != NULL);
LTC_ARGCHK(R != NULL);
LTC_ARGCHK(modulus != NULL);
LTC_ARGCHK(Mp != NULL);
mp = *((fp_digit*)Mp);
fp_init(&t1);
fp_init(&t2);
if (P != R) {
fp_copy(P->x, R->x);
fp_copy(P->y, R->y);
fp_copy(P->z, R->z);
}
if ((err = ltc_ecc_is_point_at_infinity(P, modulus, &inf)) != CRYPT_OK) return err;
if (inf) {
ltc_mp.set_int(R->x, 1);
ltc_mp.set_int(R->y, 1);
ltc_mp.set_int(R->z, 0);
return CRYPT_OK;
}
fp_sqr(R->z, &t1);
fp_montgomery_reduce(&t1, modulus, mp);
fp_mul(R->z, R->y, R->z);
fp_montgomery_reduce(R->z, modulus, mp);
fp_add(R->z, R->z, R->z);
if (fp_cmp(R->z, modulus) != FP_LT) {
fp_sub(R->z, modulus, R->z);
}
if (ma == NULL) { 
fp_sub(R->x, &t1, &t2);
if (fp_cmp_d(&t2, 0) == LTC_MP_LT) {
fp_add(&t2, modulus, &t2);
}
fp_add(&t1, R->x, &t1);
if (fp_cmp(&t1, modulus) != FP_LT) {
fp_sub(&t1, modulus, &t1);
}
fp_mul(&t1, &t2, &t2);
fp_montgomery_reduce(&t2, modulus, mp);
fp_add(&t2, &t2, &t1);
if (fp_cmp(&t1, modulus) != FP_LT) {
fp_sub(&t1, modulus, &t1);
}
fp_add(&t1, &t2, &t1);
if (fp_cmp(&t1, modulus) != FP_LT) {
fp_sub(&t1, modulus, &t1);
}
}
else {
fp_sqr(&t1, &t2);
fp_montgomery_reduce(&t2, modulus, mp);
fp_mul(&t2, ma, &t1);
fp_montgomery_reduce(&t1, modulus, mp);
fp_sqr(R->x, &t2);
fp_montgomery_reduce(&t2, modulus, mp);
fp_add(&t1, &t2, &t1);
if (fp_cmp(&t1, modulus) != FP_LT) {
fp_sub(&t1, modulus, &t1);
}
fp_add(&t1, &t2, &t1);
if (fp_cmp(&t1, modulus) != FP_LT) {
fp_sub(&t1, modulus, &t1);
}
fp_add(&t1, &t2, &t1);
if (fp_cmp(&t1, modulus) != FP_LT) {
fp_sub(&t1, modulus, &t1);
}
}
fp_add(R->y, R->y, R->y);
if (fp_cmp(R->y, modulus) != FP_LT) {
fp_sub(R->y, modulus, R->y);
}
fp_sqr(R->y, R->y);
fp_montgomery_reduce(R->y, modulus, mp);
fp_sqr(R->y, &t2);
fp_montgomery_reduce(&t2, modulus, mp);
if (fp_isodd(&t2)) {
fp_add(&t2, modulus, &t2);
}
fp_div_2(&t2, &t2);
fp_mul(R->y, R->x, R->y);
fp_montgomery_reduce(R->y, modulus, mp);
fp_sqr(&t1, R->x);
fp_montgomery_reduce(R->x, modulus, mp);
fp_sub(R->x, R->y, R->x);
if (fp_cmp_d(R->x, 0) == FP_LT) {
fp_add(R->x, modulus, R->x);
}
fp_sub(R->x, R->y, R->x);
if (fp_cmp_d(R->x, 0) == FP_LT) {
fp_add(R->x, modulus, R->x);
}
fp_sub(R->y, R->x, R->y);
if (fp_cmp_d(R->y, 0) == FP_LT) {
fp_add(R->y, modulus, R->y);
}
fp_mul(R->y, &t1, R->y);
fp_montgomery_reduce(R->y, modulus, mp);
fp_sub(R->y, &t2, R->y);
if (fp_cmp_d(R->y, 0) == FP_LT) {
fp_add(R->y, modulus, R->y);
}
return CRYPT_OK;
}
static int tfm_ecc_projective_add_point(const ecc_point *P, const ecc_point *Q, ecc_point *R, void *ma, void *modulus, void *Mp)
{
fp_int t1, t2, x, y, z;
fp_digit mp;
int err, inf;
LTC_ARGCHK(P != NULL);
LTC_ARGCHK(Q != NULL);
LTC_ARGCHK(R != NULL);
LTC_ARGCHK(modulus != NULL);
LTC_ARGCHK(Mp != NULL);
mp = *((fp_digit*)Mp);
fp_init(&t1);
fp_init(&t2);
fp_init(&x);
fp_init(&y);
fp_init(&z);
if ((err = ltc_ecc_is_point_at_infinity(P, modulus, &inf)) != CRYPT_OK) return err;
if (inf) {
ltc_mp.copy(Q->x, R->x);
ltc_mp.copy(Q->y, R->y);
ltc_mp.copy(Q->z, R->z);
return CRYPT_OK;
}
if ((err = ltc_ecc_is_point_at_infinity(Q, modulus, &inf)) != CRYPT_OK) return err;
if (inf) {
ltc_mp.copy(P->x, R->x);
ltc_mp.copy(P->y, R->y);
ltc_mp.copy(P->z, R->z);
return CRYPT_OK;
}
fp_sub(modulus, Q->y, &t1);
if ( (fp_cmp(P->x, Q->x) == FP_EQ) &&
(Q->z != NULL && fp_cmp(P->z, Q->z) == FP_EQ) &&
(fp_cmp(P->y, Q->y) == FP_EQ || fp_cmp(P->y, &t1) == FP_EQ)) {
return tfm_ecc_projective_dbl_point(P, R, ma, modulus, Mp);
}
fp_copy(P->x, &x);
fp_copy(P->y, &y);
fp_copy(P->z, &z);
if (Q->z != NULL) {
fp_sqr(Q->z, &t1);
fp_montgomery_reduce(&t1, modulus, mp);
fp_mul(&t1, &x, &x);
fp_montgomery_reduce(&x, modulus, mp);
fp_mul(Q->z, &t1, &t1);
fp_montgomery_reduce(&t1, modulus, mp);
fp_mul(&t1, &y, &y);
fp_montgomery_reduce(&y, modulus, mp);
}
fp_sqr(&z, &t1);
fp_montgomery_reduce(&t1, modulus, mp);
fp_mul(Q->x, &t1, &t2);
fp_montgomery_reduce(&t2, modulus, mp);
fp_mul(&z, &t1, &t1);
fp_montgomery_reduce(&t1, modulus, mp);
fp_mul(Q->y, &t1, &t1);
fp_montgomery_reduce(&t1, modulus, mp);
fp_sub(&y, &t1, &y);
if (fp_cmp_d(&y, 0) == FP_LT) {
fp_add(&y, modulus, &y);
}
fp_add(&t1, &t1, &t1);
if (fp_cmp(&t1, modulus) != FP_LT) {
fp_sub(&t1, modulus, &t1);
}
fp_add(&t1, &y, &t1);
if (fp_cmp(&t1, modulus) != FP_LT) {
fp_sub(&t1, modulus, &t1);
}
fp_sub(&x, &t2, &x);
if (fp_cmp_d(&x, 0) == FP_LT) {
fp_add(&x, modulus, &x);
}
fp_add(&t2, &t2, &t2);
if (fp_cmp(&t2, modulus) != FP_LT) {
fp_sub(&t2, modulus, &t2);
}
fp_add(&t2, &x, &t2);
if (fp_cmp(&t2, modulus) != FP_LT) {
fp_sub(&t2, modulus, &t2);
}
if (Q->z != NULL) {
fp_mul(&z, Q->z, &z);
fp_montgomery_reduce(&z, modulus, mp);
}
fp_mul(&z, &x, &z);
fp_montgomery_reduce(&z, modulus, mp);
fp_mul(&t1, &x, &t1);
fp_montgomery_reduce(&t1, modulus, mp);
fp_sqr(&x, &x);
fp_montgomery_reduce(&x, modulus, mp);
fp_mul(&t2, &x, &t2);
fp_montgomery_reduce(&t2, modulus, mp);
fp_mul(&t1, &x, &t1);
fp_montgomery_reduce(&t1, modulus, mp);
fp_sqr(&y, &x);
fp_montgomery_reduce(&x, modulus, mp);
fp_sub(&x, &t2, &x);
if (fp_cmp_d(&x, 0) == FP_LT) {
fp_add(&x, modulus, &x);
}
fp_sub(&t2, &x, &t2);
if (fp_cmp_d(&t2, 0) == FP_LT) {
fp_add(&t2, modulus, &t2);
}
fp_sub(&t2, &x, &t2);
if (fp_cmp_d(&t2, 0) == FP_LT) {
fp_add(&t2, modulus, &t2);
}
fp_mul(&t2, &y, &t2);
fp_montgomery_reduce(&t2, modulus, mp);
fp_sub(&t2, &t1, &y);
if (fp_cmp_d(&y, 0) == FP_LT) {
fp_add(&y, modulus, &y);
}
if (fp_isodd(&y)) {
fp_add(&y, modulus, &y);
}
fp_div_2(&y, &y);
fp_copy(&x, R->x);
fp_copy(&y, R->y);
fp_copy(&z, R->z);
return CRYPT_OK;
}
#endif
static int set_rand(void *a, int size)
{
LTC_ARGCHK(a != NULL);
fp_rand(a, size);
return CRYPT_OK;
}
const ltc_math_descriptor tfm_desc = {
"TomsFastMath",
(int)DIGIT_BIT,
&init,
&init_copy,
&deinit,
&neg,
&copy,
&set_int,
&get_int,
&get_digit,
&get_digit_count,
&compare,
&compare_d,
&count_bits,
&count_lsb_bits,
&twoexpt,
&read_radix,
&write_radix,
&unsigned_size,
&unsigned_write,
&unsigned_read,
&add,
&addi,
&sub,
&subi,
&mul,
&muli,
&sqr,
NULL, 
&divide,
&div_2,
&modi,
&gcd,
&lcm,
&mulmod,
&sqrmod,
&invmod,
&montgomery_setup,
&montgomery_normalization,
&montgomery_reduce,
&montgomery_deinit,
&exptmod,
&isprime,
#if defined(LTC_MECC)
#if defined(LTC_MECC_FP)
&ltc_ecc_fp_mulmod,
#else
&ltc_ecc_mulmod,
#endif 
#if defined(LTC_MECC_ACCEL)
&tfm_ecc_projective_add_point,
&tfm_ecc_projective_dbl_point,
#else
&ltc_ecc_projective_add_point,
&ltc_ecc_projective_dbl_point,
#endif 
&ltc_ecc_map,
#if defined(LTC_ECC_SHAMIR)
#if defined(LTC_MECC_FP)
&ltc_ecc_fp_mul2add,
#else
&ltc_ecc_mul2add,
#endif 
#else
NULL,
#endif 
#else
NULL, NULL, NULL, NULL, NULL,
#endif 
#if defined(LTC_MRSA)
&rsa_make_key,
&rsa_exptmod,
#else
NULL, NULL,
#endif
&addmod,
&submod,
set_rand,
};
#endif
