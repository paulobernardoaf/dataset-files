








#include "ossl.h"

#define GetDigest(obj, ctx) do { TypedData_Get_Struct((obj), EVP_MD_CTX, &ossl_digest_type, (ctx)); if (!(ctx)) { ossl_raise(rb_eRuntimeError, "Digest CTX wasn't initialized!"); } } while (0)









VALUE cDigest;
VALUE eDigestError;

static VALUE ossl_digest_alloc(VALUE klass);

static void
ossl_digest_free(void *ctx)
{
EVP_MD_CTX_destroy(ctx);
}

static const rb_data_type_t ossl_digest_type = {
"OpenSSL/Digest",
{
0, ossl_digest_free,
},
0, 0, RUBY_TYPED_FREE_IMMEDIATELY,
};




const EVP_MD *
ossl_evp_get_digestbyname(VALUE obj)
{
const EVP_MD *md;
ASN1_OBJECT *oid = NULL;

if (RB_TYPE_P(obj, T_STRING)) {
const char *name = StringValueCStr(obj);

md = EVP_get_digestbyname(name);
if (!md) {
oid = OBJ_txt2obj(name, 0);
md = EVP_get_digestbyobj(oid);
ASN1_OBJECT_free(oid);
}
if(!md)
ossl_raise(rb_eRuntimeError, "Unsupported digest algorithm (%"PRIsVALUE").", obj);
} else {
EVP_MD_CTX *ctx;

GetDigest(obj, ctx);

md = EVP_MD_CTX_md(ctx);
}

return md;
}

VALUE
ossl_digest_new(const EVP_MD *md)
{
VALUE ret;
EVP_MD_CTX *ctx;

ret = ossl_digest_alloc(cDigest);
ctx = EVP_MD_CTX_new();
if (!ctx)
ossl_raise(eDigestError, "EVP_MD_CTX_new");
RTYPEDDATA_DATA(ret) = ctx;

if (!EVP_DigestInit_ex(ctx, md, NULL))
ossl_raise(eDigestError, "Digest initialization failed");

return ret;
}




static VALUE
ossl_digest_alloc(VALUE klass)
{
return TypedData_Wrap_Struct(klass, &ossl_digest_type, 0);
}

VALUE ossl_digest_update(VALUE, VALUE);


















static VALUE
ossl_digest_initialize(int argc, VALUE *argv, VALUE self)
{
EVP_MD_CTX *ctx;
const EVP_MD *md;
VALUE type, data;

rb_scan_args(argc, argv, "11", &type, &data);
md = ossl_evp_get_digestbyname(type);
if (!NIL_P(data)) StringValue(data);

TypedData_Get_Struct(self, EVP_MD_CTX, &ossl_digest_type, ctx);
if (!ctx) {
RTYPEDDATA_DATA(self) = ctx = EVP_MD_CTX_new();
if (!ctx)
ossl_raise(eDigestError, "EVP_MD_CTX_new");
}

if (!EVP_DigestInit_ex(ctx, md, NULL))
ossl_raise(eDigestError, "Digest initialization failed");

if (!NIL_P(data)) return ossl_digest_update(self, data);
return self;
}

static VALUE
ossl_digest_copy(VALUE self, VALUE other)
{
EVP_MD_CTX *ctx1, *ctx2;

rb_check_frozen(self);
if (self == other) return self;

TypedData_Get_Struct(self, EVP_MD_CTX, &ossl_digest_type, ctx1);
if (!ctx1) {
RTYPEDDATA_DATA(self) = ctx1 = EVP_MD_CTX_new();
if (!ctx1)
ossl_raise(eDigestError, "EVP_MD_CTX_new");
}
GetDigest(other, ctx2);

if (!EVP_MD_CTX_copy(ctx1, ctx2)) {
ossl_raise(eDigestError, NULL);
}
return self;
}









static VALUE
ossl_digest_reset(VALUE self)
{
EVP_MD_CTX *ctx;

GetDigest(self, ctx);
if (EVP_DigestInit_ex(ctx, EVP_MD_CTX_md(ctx), NULL) != 1) {
ossl_raise(eDigestError, "Digest initialization failed.");
}

return self;
}
















VALUE
ossl_digest_update(VALUE self, VALUE data)
{
EVP_MD_CTX *ctx;

StringValue(data);
GetDigest(self, ctx);

if (!EVP_DigestUpdate(ctx, RSTRING_PTR(data), RSTRING_LEN(data)))
ossl_raise(eDigestError, "EVP_DigestUpdate");

return self;
}






static VALUE
ossl_digest_finish(int argc, VALUE *argv, VALUE self)
{
EVP_MD_CTX *ctx;
VALUE str;
int out_len;

GetDigest(self, ctx);
rb_scan_args(argc, argv, "01", &str);
out_len = EVP_MD_CTX_size(ctx);

if (NIL_P(str)) {
str = rb_str_new(NULL, out_len);
} else {
StringValue(str);
rb_str_resize(str, out_len);
}

if (!EVP_DigestFinal_ex(ctx, (unsigned char *)RSTRING_PTR(str), NULL))
ossl_raise(eDigestError, "EVP_DigestFinal_ex");

return str;
}












static VALUE
ossl_digest_name(VALUE self)
{
EVP_MD_CTX *ctx;

GetDigest(self, ctx);

return rb_str_new2(EVP_MD_name(EVP_MD_CTX_md(ctx)));
}













static VALUE
ossl_digest_size(VALUE self)
{
EVP_MD_CTX *ctx;

GetDigest(self, ctx);

return INT2NUM(EVP_MD_CTX_size(ctx));
}














static VALUE
ossl_digest_block_length(VALUE self)
{
EVP_MD_CTX *ctx;

GetDigest(self, ctx);

return INT2NUM(EVP_MD_CTX_block_size(ctx));
}




void
Init_ossl_digest(void)
{
rb_require("digest");

#if 0
mOSSL = rb_define_module("OpenSSL");
eOSSLError = rb_define_class_under(mOSSL, "OpenSSLError", rb_eStandardError);
#endif
















































































































cDigest = rb_define_class_under(mOSSL, "Digest", rb_path2class("Digest::Class"));





eDigestError = rb_define_class_under(cDigest, "DigestError", eOSSLError);

rb_define_alloc_func(cDigest, ossl_digest_alloc);

rb_define_method(cDigest, "initialize", ossl_digest_initialize, -1);
rb_define_method(cDigest, "initialize_copy", ossl_digest_copy, 1);
rb_define_method(cDigest, "reset", ossl_digest_reset, 0);
rb_define_method(cDigest, "update", ossl_digest_update, 1);
rb_define_alias(cDigest, "<<", "update");
rb_define_private_method(cDigest, "finish", ossl_digest_finish, -1);
rb_define_method(cDigest, "digest_length", ossl_digest_size, 0);
rb_define_method(cDigest, "block_length", ossl_digest_block_length, 0);

rb_define_method(cDigest, "name", ossl_digest_name, 0);
}
