#define LTC_MP_LT -1
#define LTC_MP_EQ 0
#define LTC_MP_GT 1
#define LTC_MP_NO 0
#define LTC_MP_YES 1
#if !defined(LTC_MECC)
typedef void ecc_point;
#endif
#if !defined(LTC_MRSA)
typedef void rsa_key;
#endif
#if !defined(LTC_MILLER_RABIN_REPS)
#define LTC_MILLER_RABIN_REPS 40
#endif
int radix_to_bin(const void *in, int radix, void *out, unsigned long *len);
typedef struct {
const char *name;
int bits_per_digit;
int (*init)(void **a);
int (*init_copy)(void **dst, void *src);
void (*deinit)(void *a);
int (*neg)(void *src, void *dst);
int (*copy)(void *src, void *dst);
int (*set_int)(void *a, ltc_mp_digit n);
unsigned long (*get_int)(void *a);
ltc_mp_digit (*get_digit)(void *a, int n);
int (*get_digit_count)(void *a);
int (*compare)(void *a, void *b);
int (*compare_d)(void *a, ltc_mp_digit n);
int (*count_bits)(void * a);
int (*count_lsb_bits)(void *a);
int (*twoexpt)(void *a , int n);
int (*read_radix)(void *a, const char *str, int radix);
int (*write_radix)(void *a, char *str, int radix);
unsigned long (*unsigned_size)(void *a);
int (*unsigned_write)(void *src, unsigned char *dst);
int (*unsigned_read)( void *dst,
unsigned char *src,
unsigned long len);
int (*add)(void *a, void *b, void *c);
int (*addi)(void *a, ltc_mp_digit b, void *c);
int (*sub)(void *a, void *b, void *c);
int (*subi)(void *a, ltc_mp_digit b, void *c);
int (*mul)(void *a, void *b, void *c);
int (*muli)(void *a, ltc_mp_digit b, void *c);
int (*sqr)(void *a, void *b);
int (*sqrtmod_prime)(void *a, void *b, void *c);
int (*mpdiv)(void *a, void *b, void *c, void *d);
int (*div_2)(void *a, void *b);
int (*modi)(void *a, ltc_mp_digit b, ltc_mp_digit *c);
int (*gcd)(void *a, void *b, void *c);
int (*lcm)(void *a, void *b, void *c);
int (*mulmod)(void *a, void *b, void *c, void *d);
int (*sqrmod)(void *a, void *b, void *c);
int (*invmod)(void *, void *, void *);
int (*montgomery_setup)(void *a, void **b);
int (*montgomery_normalization)(void *a, void *b);
int (*montgomery_reduce)(void *a, void *b, void *c);
void (*montgomery_deinit)(void *a);
int (*exptmod)(void *a, void *b, void *c, void *d);
int (*isprime)(void *a, int b, int *c);
int (*ecc_ptmul)( void *k,
const ecc_point *G,
ecc_point *R,
void *a,
void *modulus,
int map);
int (*ecc_ptadd)(const ecc_point *P,
const ecc_point *Q,
ecc_point *R,
void *ma,
void *modulus,
void *mp);
int (*ecc_ptdbl)(const ecc_point *P,
ecc_point *R,
void *ma,
void *modulus,
void *mp);
int (*ecc_map)(ecc_point *P, void *modulus, void *mp);
int (*ecc_mul2add)(const ecc_point *A, void *kA,
const ecc_point *B, void *kB,
ecc_point *C,
void *ma,
void *modulus);
int (*rsa_keygen)(prng_state *prng,
int wprng,
int size,
long e,
rsa_key *key);
int (*rsa_me)(const unsigned char *in, unsigned long inlen,
unsigned char *out, unsigned long *outlen, int which,
const rsa_key *key);
int (*addmod)(void *a, void *b, void *c, void *d);
int (*submod)(void *a, void *b, void *c, void *d);
int (*rand)(void *a, int size);
} ltc_math_descriptor;
extern ltc_math_descriptor ltc_mp;
int ltc_init_multi(void **a, ...);
void ltc_deinit_multi(void *a, ...);
void ltc_cleanup_multi(void **a, ...);
#if defined(LTM_DESC)
extern const ltc_math_descriptor ltm_desc;
#endif
#if defined(TFM_DESC)
extern const ltc_math_descriptor tfm_desc;
#endif
#if defined(GMP_DESC)
extern const ltc_math_descriptor gmp_desc;
#endif
