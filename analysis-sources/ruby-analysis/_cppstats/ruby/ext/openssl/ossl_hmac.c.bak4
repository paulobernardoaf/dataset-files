








#if !defined(OPENSSL_NO_HMAC)

#include "ossl.h"

#define NewHMAC(klass) TypedData_Wrap_Struct((klass), &ossl_hmac_type, 0)

#define GetHMAC(obj, ctx) do { TypedData_Get_Struct((obj), HMAC_CTX, &ossl_hmac_type, (ctx)); if (!(ctx)) { ossl_raise(rb_eRuntimeError, "HMAC wasn't initialized"); } } while (0)









VALUE cHMAC;
VALUE eHMACError;








static void
ossl_hmac_free(void *ctx)
{
HMAC_CTX_free(ctx);
}

static const rb_data_type_t ossl_hmac_type = {
"OpenSSL/HMAC",
{
0, ossl_hmac_free,
},
0, 0, RUBY_TYPED_FREE_IMMEDIATELY,
};

static VALUE
ossl_hmac_alloc(VALUE klass)
{
VALUE obj;
HMAC_CTX *ctx;

obj = NewHMAC(klass);
ctx = HMAC_CTX_new();
if (!ctx)
ossl_raise(eHMACError, NULL);
RTYPEDDATA_DATA(obj) = ctx;

return obj;
}































static VALUE
ossl_hmac_initialize(VALUE self, VALUE key, VALUE digest)
{
HMAC_CTX *ctx;

StringValue(key);
GetHMAC(self, ctx);
HMAC_Init_ex(ctx, RSTRING_PTR(key), RSTRING_LENINT(key),
ossl_evp_get_digestbyname(digest), NULL);

return self;
}

static VALUE
ossl_hmac_copy(VALUE self, VALUE other)
{
HMAC_CTX *ctx1, *ctx2;

rb_check_frozen(self);
if (self == other) return self;

GetHMAC(self, ctx1);
GetHMAC(other, ctx2);

if (!HMAC_CTX_copy(ctx1, ctx2))
ossl_raise(eHMACError, "HMAC_CTX_copy");
return self;
}



















static VALUE
ossl_hmac_update(VALUE self, VALUE data)
{
HMAC_CTX *ctx;

StringValue(data);
GetHMAC(self, ctx);
HMAC_Update(ctx, (unsigned char *)RSTRING_PTR(data), RSTRING_LEN(data));

return self;
}

static void
hmac_final(HMAC_CTX *ctx, unsigned char *buf, unsigned int *buf_len)
{
HMAC_CTX *final;

final = HMAC_CTX_new();
if (!final)
ossl_raise(eHMACError, "HMAC_CTX_new");

if (!HMAC_CTX_copy(final, ctx)) {
HMAC_CTX_free(final);
ossl_raise(eHMACError, "HMAC_CTX_copy");
}

HMAC_Final(final, buf, buf_len);
HMAC_CTX_free(final);
}













static VALUE
ossl_hmac_digest(VALUE self)
{
HMAC_CTX *ctx;
unsigned int buf_len;
VALUE ret;

GetHMAC(self, ctx);
ret = rb_str_new(NULL, EVP_MAX_MD_SIZE);
hmac_final(ctx, (unsigned char *)RSTRING_PTR(ret), &buf_len);
assert(buf_len <= EVP_MAX_MD_SIZE);
rb_str_set_len(ret, buf_len);

return ret;
}








static VALUE
ossl_hmac_hexdigest(VALUE self)
{
HMAC_CTX *ctx;
unsigned char buf[EVP_MAX_MD_SIZE];
unsigned int buf_len;
VALUE ret;

GetHMAC(self, ctx);
hmac_final(ctx, buf, &buf_len);
ret = rb_str_new(NULL, buf_len * 2);
ossl_bin2hex(buf, RSTRING_PTR(ret), buf_len);

return ret;
}




















static VALUE
ossl_hmac_reset(VALUE self)
{
HMAC_CTX *ctx;

GetHMAC(self, ctx);
HMAC_Init_ex(ctx, NULL, 0, NULL, NULL);

return self;
}


















static VALUE
ossl_hmac_s_digest(VALUE klass, VALUE digest, VALUE key, VALUE data)
{
unsigned char *buf;
unsigned int buf_len;

StringValue(key);
StringValue(data);
buf = HMAC(ossl_evp_get_digestbyname(digest), RSTRING_PTR(key),
RSTRING_LENINT(key), (unsigned char *)RSTRING_PTR(data),
RSTRING_LEN(data), NULL, &buf_len);

return rb_str_new((const char *)buf, buf_len);
}


















static VALUE
ossl_hmac_s_hexdigest(VALUE klass, VALUE digest, VALUE key, VALUE data)
{
unsigned char buf[EVP_MAX_MD_SIZE];
unsigned int buf_len;
VALUE ret;

StringValue(key);
StringValue(data);

if (!HMAC(ossl_evp_get_digestbyname(digest), RSTRING_PTR(key),
RSTRING_LENINT(key), (unsigned char *)RSTRING_PTR(data),
RSTRING_LEN(data), buf, &buf_len))
ossl_raise(eHMACError, "HMAC");

ret = rb_str_new(NULL, buf_len * 2);
ossl_bin2hex(buf, RSTRING_PTR(ret), buf_len);

return ret;
}




void
Init_ossl_hmac(void)
{
#if 0
mOSSL = rb_define_module("OpenSSL");
eOSSLError = rb_define_class_under(mOSSL, "OpenSSLError", rb_eStandardError);
#endif





























eHMACError = rb_define_class_under(mOSSL, "HMACError", eOSSLError);

cHMAC = rb_define_class_under(mOSSL, "HMAC", rb_cObject);

rb_define_alloc_func(cHMAC, ossl_hmac_alloc);
rb_define_singleton_method(cHMAC, "digest", ossl_hmac_s_digest, 3);
rb_define_singleton_method(cHMAC, "hexdigest", ossl_hmac_s_hexdigest, 3);

rb_define_method(cHMAC, "initialize", ossl_hmac_initialize, 2);
rb_define_method(cHMAC, "initialize_copy", ossl_hmac_copy, 1);

rb_define_method(cHMAC, "reset", ossl_hmac_reset, 0);
rb_define_method(cHMAC, "update", ossl_hmac_update, 1);
rb_define_alias(cHMAC, "<<", "update");
rb_define_method(cHMAC, "digest", ossl_hmac_digest, 0);
rb_define_method(cHMAC, "hexdigest", ossl_hmac_hexdigest, 0);
rb_define_alias(cHMAC, "inspect", "hexdigest");
rb_define_alias(cHMAC, "to_s", "hexdigest");
}

#else 
#warning >>> OpenSSL is compiled without HMAC support <<<
void
Init_ossl_hmac(void)
{
rb_warning("HMAC is not available: OpenSSL is compiled without HMAC.");
}
#endif 
