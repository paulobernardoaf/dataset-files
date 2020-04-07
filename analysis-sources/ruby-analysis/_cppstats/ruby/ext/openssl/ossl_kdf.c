#include "ossl.h"
#if OPENSSL_VERSION_NUMBER >= 0x10100000 && !defined(LIBRESSL_VERSION_NUMBER)
#include <openssl/kdf.h>
#endif
static VALUE mKDF, eKDF;
static VALUE
kdf_pbkdf2_hmac(int argc, VALUE *argv, VALUE self)
{
VALUE pass, salt, opts, kwargs[4], str;
static ID kwargs_ids[4];
int iters, len;
const EVP_MD *md;
if (!kwargs_ids[0]) {
kwargs_ids[0] = rb_intern_const("salt");
kwargs_ids[1] = rb_intern_const("iterations");
kwargs_ids[2] = rb_intern_const("length");
kwargs_ids[3] = rb_intern_const("hash");
}
rb_scan_args(argc, argv, "1:", &pass, &opts);
rb_get_kwargs(opts, kwargs_ids, 4, 0, kwargs);
StringValue(pass);
salt = StringValue(kwargs[0]);
iters = NUM2INT(kwargs[1]);
len = NUM2INT(kwargs[2]);
md = ossl_evp_get_digestbyname(kwargs[3]);
str = rb_str_new(0, len);
if (!PKCS5_PBKDF2_HMAC(RSTRING_PTR(pass), RSTRING_LENINT(pass),
(unsigned char *)RSTRING_PTR(salt),
RSTRING_LENINT(salt), iters, md, len,
(unsigned char *)RSTRING_PTR(str)))
ossl_raise(eKDF, "PKCS5_PBKDF2_HMAC");
return str;
}
#if defined(HAVE_EVP_PBE_SCRYPT)
static VALUE
kdf_scrypt(int argc, VALUE *argv, VALUE self)
{
VALUE pass, salt, opts, kwargs[5], str;
static ID kwargs_ids[5];
size_t len;
uint64_t N, r, p, maxmem;
if (!kwargs_ids[0]) {
kwargs_ids[0] = rb_intern_const("salt");
kwargs_ids[1] = rb_intern_const("N");
kwargs_ids[2] = rb_intern_const("r");
kwargs_ids[3] = rb_intern_const("p");
kwargs_ids[4] = rb_intern_const("length");
}
rb_scan_args(argc, argv, "1:", &pass, &opts);
rb_get_kwargs(opts, kwargs_ids, 5, 0, kwargs);
StringValue(pass);
salt = StringValue(kwargs[0]);
N = NUM2UINT64T(kwargs[1]);
r = NUM2UINT64T(kwargs[2]);
p = NUM2UINT64T(kwargs[3]);
len = NUM2LONG(kwargs[4]);
maxmem = SIZE_MAX;
str = rb_str_new(0, len);
if (!EVP_PBE_scrypt(RSTRING_PTR(pass), RSTRING_LEN(pass),
(unsigned char *)RSTRING_PTR(salt), RSTRING_LEN(salt),
N, r, p, maxmem, (unsigned char *)RSTRING_PTR(str), len))
ossl_raise(eKDF, "EVP_PBE_scrypt");
return str;
}
#endif
#if OPENSSL_VERSION_NUMBER >= 0x10100000 && !defined(LIBRESSL_VERSION_NUMBER)
static VALUE
kdf_hkdf(int argc, VALUE *argv, VALUE self)
{
VALUE ikm, salt, info, opts, kwargs[4], str;
static ID kwargs_ids[4];
int saltlen, ikmlen, infolen;
size_t len;
const EVP_MD *md;
EVP_PKEY_CTX *pctx;
if (!kwargs_ids[0]) {
kwargs_ids[0] = rb_intern_const("salt");
kwargs_ids[1] = rb_intern_const("info");
kwargs_ids[2] = rb_intern_const("length");
kwargs_ids[3] = rb_intern_const("hash");
}
rb_scan_args(argc, argv, "1:", &ikm, &opts);
rb_get_kwargs(opts, kwargs_ids, 4, 0, kwargs);
StringValue(ikm);
ikmlen = RSTRING_LENINT(ikm);
salt = StringValue(kwargs[0]);
saltlen = RSTRING_LENINT(salt);
info = StringValue(kwargs[1]);
infolen = RSTRING_LENINT(info);
len = (size_t)NUM2LONG(kwargs[2]);
if (len > LONG_MAX)
rb_raise(rb_eArgError, "length must be non-negative");
md = ossl_evp_get_digestbyname(kwargs[3]);
str = rb_str_new(NULL, (long)len);
pctx = EVP_PKEY_CTX_new_id(EVP_PKEY_HKDF, NULL);
if (!pctx)
ossl_raise(eKDF, "EVP_PKEY_CTX_new_id");
if (EVP_PKEY_derive_init(pctx) <= 0) {
EVP_PKEY_CTX_free(pctx);
ossl_raise(eKDF, "EVP_PKEY_derive_init");
}
if (EVP_PKEY_CTX_set_hkdf_md(pctx, md) <= 0) {
EVP_PKEY_CTX_free(pctx);
ossl_raise(eKDF, "EVP_PKEY_CTX_set_hkdf_md");
}
if (EVP_PKEY_CTX_set1_hkdf_salt(pctx, (unsigned char *)RSTRING_PTR(salt),
saltlen) <= 0) {
EVP_PKEY_CTX_free(pctx);
ossl_raise(eKDF, "EVP_PKEY_CTX_set_hkdf_salt");
}
if (EVP_PKEY_CTX_set1_hkdf_key(pctx, (unsigned char *)RSTRING_PTR(ikm),
ikmlen) <= 0) {
EVP_PKEY_CTX_free(pctx);
ossl_raise(eKDF, "EVP_PKEY_CTX_set_hkdf_key");
}
if (EVP_PKEY_CTX_add1_hkdf_info(pctx, (unsigned char *)RSTRING_PTR(info),
infolen) <= 0) {
EVP_PKEY_CTX_free(pctx);
ossl_raise(eKDF, "EVP_PKEY_CTX_set_hkdf_info");
}
if (EVP_PKEY_derive(pctx, (unsigned char *)RSTRING_PTR(str), &len) <= 0) {
EVP_PKEY_CTX_free(pctx);
ossl_raise(eKDF, "EVP_PKEY_derive");
}
rb_str_set_len(str, (long)len);
EVP_PKEY_CTX_free(pctx);
return str;
}
#endif
void
Init_ossl_kdf(void)
{
#if 0
mOSSL = rb_define_module("OpenSSL");
eOSSLError = rb_define_class_under(mOSSL, "OpenSSLError", rb_eStandardError);
#endif
mKDF = rb_define_module_under(mOSSL, "KDF");
eKDF = rb_define_class_under(mKDF, "KDFError", eOSSLError);
rb_define_module_function(mKDF, "pbkdf2_hmac", kdf_pbkdf2_hmac, -1);
#if defined(HAVE_EVP_PBE_SCRYPT)
rb_define_module_function(mKDF, "scrypt", kdf_scrypt, -1);
#endif
#if OPENSSL_VERSION_NUMBER >= 0x10100000 && !defined(LIBRESSL_VERSION_NUMBER)
rb_define_module_function(mKDF, "hkdf", kdf_hkdf, -1);
#endif
}
