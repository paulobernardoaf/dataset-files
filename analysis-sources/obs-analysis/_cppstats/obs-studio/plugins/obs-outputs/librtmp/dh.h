#if defined(USE_MBEDTLS)
#include <mbedtls/dhm.h>
#include <mbedtls/bignum.h>
typedef mbedtls_mpi* MP_t;
#define MP_new(m) m = malloc(sizeof(mbedtls_mpi)); mbedtls_mpi_init(m)
#define MP_set_w(mpi, w) mbedtls_mpi_lset(mpi, w)
#define MP_cmp(u, v) mbedtls_mpi_cmp_mpi(u, v)
#define MP_set(u, v) mbedtls_mpi_copy(u, v)
#define MP_sub_w(mpi, w) mbedtls_mpi_sub_int(mpi, mpi, w)
#define MP_cmp_1(mpi) mbedtls_mpi_cmp_int(mpi, 1)
#define MP_modexp(r, y, q, p) mbedtls_mpi_exp_mod(r, y, q, p, NULL)
#define MP_free(mpi) mbedtls_mpi_free(mpi); free(mpi)
#define MP_gethex(u, hex, res) MP_new(u); res = mbedtls_mpi_read_string(u, 16, hex) == 0
#define MP_bytes(u) mbedtls_mpi_size(u)
#define MP_setbin(u,buf,len) mbedtls_mpi_write_binary(u,buf,len)
#define MP_getbin(u,buf,len) MP_new(u); mbedtls_mpi_read_binary(u,buf,len)
typedef struct MDH
{
MP_t p;
MP_t g;
MP_t pub_key;
MP_t priv_key;
long length;
mbedtls_dhm_context ctx;
} MDH;
#define MDH_new() calloc(1,sizeof(MDH))
#define MDH_free(vp) {MDH *_dh = vp; mbedtls_dhm_free(&_dh->ctx); MP_free(_dh->p); MP_free(_dh->g); MP_free(_dh->pub_key); MP_free(_dh->priv_key); free(_dh);}
static int MDH_generate_key(RTMP *r, MDH *dh)
{
unsigned char out[2];
MP_set(&dh->ctx.P, dh->p);
MP_set(&dh->ctx.G, dh->g);
dh->ctx.len = 128;
mbedtls_dhm_make_public(&dh->ctx, 1024, out, 1, mbedtls_ctr_drbg_random, &r->RTMP_TLS_ctx->ctr_drbg);
MP_new(dh->pub_key);
MP_new(dh->priv_key);
MP_set(dh->pub_key, &dh->ctx.GX);
MP_set(dh->priv_key, &dh->ctx.X);
return 1;
}
static int MDH_compute_key(uint8_t *secret, size_t len, MP_t pub, MDH *dh)
{
MP_set(&dh->ctx.GY, pub);
size_t olen;
mbedtls_dhm_calc_secret(&dh->ctx, secret, len, &olen, NULL, NULL);
return 0;
}
#elif defined(USE_POLARSSL)
#include <polarssl/dhm.h>
typedef mpi * MP_t;
#define MP_new(m) m = malloc(sizeof(mpi)); mpi_init(m)
#define MP_set_w(mpi, w) mpi_lset(mpi, w)
#define MP_cmp(u, v) mpi_cmp_mpi(u, v)
#define MP_set(u, v) mpi_copy(u, v)
#define MP_sub_w(mpi, w) mpi_sub_int(mpi, mpi, w)
#define MP_cmp_1(mpi) mpi_cmp_int(mpi, 1)
#define MP_modexp(r, y, q, p) mpi_exp_mod(r, y, q, p, NULL)
#define MP_free(mpi) mpi_free(mpi); free(mpi)
#define MP_gethex(u, hex, res) MP_new(u); res = mpi_read_string(u, 16, hex) == 0
#define MP_bytes(u) mpi_size(u)
#define MP_setbin(u,buf,len) mpi_write_binary(u,buf,len)
#define MP_getbin(u,buf,len) MP_new(u); mpi_read_binary(u,buf,len)
typedef struct MDH
{
MP_t p;
MP_t g;
MP_t pub_key;
MP_t priv_key;
long length;
dhm_context ctx;
} MDH;
#define MDH_new() calloc(1,sizeof(MDH))
#define MDH_free(vp) {MDH *_dh = vp; dhm_free(&_dh->ctx); MP_free(_dh->p); MP_free(_dh->g); MP_free(_dh->pub_key); MP_free(_dh->priv_key); free(_dh);}
static int MDH_generate_key(MDH *dh)
{
unsigned char out[2];
MP_set(&dh->ctx.P, dh->p);
MP_set(&dh->ctx.G, dh->g);
dh->ctx.len = 128;
dhm_make_public(&dh->ctx, 1024, out, 1, havege_random, &RTMP_TLS_ctx->hs);
MP_new(dh->pub_key);
MP_new(dh->priv_key);
MP_set(dh->pub_key, &dh->ctx.GX);
MP_set(dh->priv_key, &dh->ctx.X);
return 1;
}
static int MDH_compute_key(uint8_t *secret, size_t len, MP_t pub, MDH *dh)
{
MP_set(&dh->ctx.GY, pub);
dhm_calc_secret(&dh->ctx, secret, &len);
return 0;
}
#elif defined(USE_GNUTLS)
#include <gmp.h>
#include <nettle/bignum.h>
typedef mpz_ptr MP_t;
#define MP_new(m) m = malloc(sizeof(*m)); mpz_init2(m, 1)
#define MP_set_w(mpi, w) mpz_set_ui(mpi, w)
#define MP_cmp(u, v) mpz_cmp(u, v)
#define MP_set(u, v) mpz_set(u, v)
#define MP_sub_w(mpi, w) mpz_sub_ui(mpi, mpi, w)
#define MP_cmp_1(mpi) mpz_cmp_ui(mpi, 1)
#define MP_modexp(r, y, q, p) mpz_powm(r, y, q, p)
#define MP_free(mpi) mpz_clear(mpi); free(mpi)
#define MP_gethex(u, hex, res) u = malloc(sizeof(*u)); mpz_init2(u, 1); res = (mpz_set_str(u, hex, 16) == 0)
#define MP_bytes(u) (mpz_sizeinbase(u, 2) + 7) / 8
#define MP_setbin(u,buf,len) nettle_mpz_get_str_256(len,buf,u)
#define MP_getbin(u,buf,len) u = malloc(sizeof(*u)); mpz_init2(u, 1); nettle_mpz_set_str_256_u(u,len,buf)
typedef struct MDH
{
MP_t p;
MP_t g;
MP_t pub_key;
MP_t priv_key;
long length;
} MDH;
#define MDH_new() calloc(1,sizeof(MDH))
#define MDH_free(dh) do {MP_free(((MDH*)(dh))->p); MP_free(((MDH*)(dh))->g); MP_free(((MDH*)(dh))->pub_key); MP_free(((MDH*)(dh))->priv_key); free(dh);} while(0)
extern MP_t gnutls_calc_dh_secret(MP_t *priv, MP_t g, MP_t p);
extern MP_t gnutls_calc_dh_key(MP_t y, MP_t x, MP_t p);
#define MDH_generate_key(dh) (dh->pub_key = gnutls_calc_dh_secret(&dh->priv_key, dh->g, dh->p))
static int MDH_compute_key(uint8_t *secret, size_t len, MP_t pub, MDH *dh)
{
MP_t sec = gnutls_calc_dh_key(pub, dh->priv_key, dh->p);
if (sec)
{
MP_setbin(sec, secret, len);
MP_free(sec);
return 0;
}
else
return -1;
}
#else 
#include <openssl/bn.h>
#include <openssl/dh.h>
typedef BIGNUM * MP_t;
#define MP_new(m) m = BN_new()
#define MP_set_w(mpi, w) BN_set_word(mpi, w)
#define MP_cmp(u, v) BN_cmp(u, v)
#define MP_set(u, v) BN_copy(u, v)
#define MP_sub_w(mpi, w) BN_sub_word(mpi, w)
#define MP_cmp_1(mpi) BN_cmp(mpi, BN_value_one())
#define MP_modexp(r, y, q, p) do {BN_CTX *ctx = BN_CTX_new(); BN_mod_exp(r, y, q, p, ctx); BN_CTX_free(ctx);} while(0)
#define MP_free(mpi) BN_free(mpi)
#define MP_gethex(u, hex, res) res = BN_hex2bn(&u, hex)
#define MP_bytes(u) BN_num_bytes(u)
#define MP_setbin(u,buf,len) BN_bn2bin(u,buf)
#define MP_getbin(u,buf,len) u = BN_bin2bn(buf,len,0)
#define MDH DH
#define MDH_new() DH_new()
#define MDH_free(dh) DH_free(dh)
#define MDH_generate_key(dh) DH_generate_key(dh)
#define MDH_compute_key(secret, seclen, pub, dh) DH_compute_key(secret, pub, dh)
#endif
#include "log.h"
#include "dhgroups.h"
static int
isValidPublicKey(MP_t y, MP_t p, MP_t q)
{
int ret = TRUE;
MP_t bn;
assert(y);
MP_new(bn);
assert(bn);
MP_set_w(bn, 1);
if (MP_cmp(y, bn) < 0)
{
RTMP_Log(RTMP_LOGERROR, "DH public key must be at least 2");
ret = FALSE;
goto failed;
}
MP_set(bn, p);
MP_sub_w(bn, 1);
if (MP_cmp(y, bn) > 0)
{
RTMP_Log(RTMP_LOGERROR, "DH public key must be at most p-2");
ret = FALSE;
goto failed;
}
if (q)
{
MP_modexp(bn, y, q, p);
if (MP_cmp_1(bn) != 0)
{
RTMP_Log(RTMP_LOGWARNING, "DH public key does not fulfill y^q mod p = 1");
}
}
failed:
MP_free(bn);
return ret;
}
static MDH *
DHInit(int nKeyBits)
{
size_t res;
MDH *dh = MDH_new();
if (!dh)
goto failed;
MP_new(dh->g);
if (!dh->g)
goto failed;
MP_gethex(dh->p, P1024, res); 
if (!res)
{
goto failed;
}
MP_set_w(dh->g, 2); 
dh->length = nKeyBits;
return dh;
failed:
if (dh)
MDH_free(dh);
return 0;
}
static int
DHGenerateKey(RTMP *r)
{
MDH *dh = r->Link.dh;
size_t res = 0;
if (!dh)
return 0;
while (!res)
{
MP_t q1 = NULL;
if (!MDH_generate_key(r, dh))
return 0;
MP_gethex(q1, Q1024, res);
assert(res);
res = isValidPublicKey(dh->pub_key, dh->p, q1);
if (!res)
{
MP_free(dh->pub_key);
MP_free(dh->priv_key);
dh->pub_key = dh->priv_key = 0;
}
MP_free(q1);
}
return 1;
}
static int
DHGetPublicKey(MDH *dh, uint8_t *pubkey, size_t nPubkeyLen)
{
int len;
if (!dh || !dh->pub_key)
return 0;
len = (int)MP_bytes(dh->pub_key);
if (len <= 0 || len > (int) nPubkeyLen)
return 0;
memset(pubkey, 0, nPubkeyLen);
MP_setbin(dh->pub_key, pubkey + (nPubkeyLen - len), len);
return 1;
}
#if 0 
static int
DHGetPrivateKey(MDH *dh, uint8_t *privkey, size_t nPrivkeyLen)
{
if (!dh || !dh->priv_key)
return 0;
int len = MP_bytes(dh->priv_key);
if (len <= 0 || len > (int) nPrivkeyLen)
return 0;
memset(privkey, 0, nPrivkeyLen);
MP_setbin(dh->priv_key, privkey + (nPrivkeyLen - len), len);
return 1;
}
#endif
static int
DHComputeSharedSecretKey(MDH *dh, uint8_t *pubkey, size_t nPubkeyLen,
uint8_t *secret)
{
MP_t q1 = NULL, pubkeyBn = NULL;
size_t len;
int res;
if (!dh || !secret || nPubkeyLen >= INT_MAX)
return -1;
MP_getbin(pubkeyBn, pubkey, nPubkeyLen);
if (!pubkeyBn)
return -1;
MP_gethex(q1, Q1024, len);
assert(len);
if (isValidPublicKey(pubkeyBn, dh->p, q1))
res = MDH_compute_key(secret, nPubkeyLen, pubkeyBn, dh);
else
res = -1;
MP_free(q1);
MP_free(pubkeyBn);
return res;
}
