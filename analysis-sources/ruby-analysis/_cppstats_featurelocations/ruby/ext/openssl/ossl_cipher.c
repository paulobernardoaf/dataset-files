








#include "ossl.h"

#define NewCipher(klass) TypedData_Wrap_Struct((klass), &ossl_cipher_type, 0)

#define AllocCipher(obj, ctx) do { (ctx) = EVP_CIPHER_CTX_new(); if (!(ctx)) ossl_raise(rb_eRuntimeError, NULL); RTYPEDDATA_DATA(obj) = (ctx); } while (0)





#define GetCipherInit(obj, ctx) do { TypedData_Get_Struct((obj), EVP_CIPHER_CTX, &ossl_cipher_type, (ctx)); } while (0)


#define GetCipher(obj, ctx) do { GetCipherInit((obj), (ctx)); if (!(ctx)) { ossl_raise(rb_eRuntimeError, "Cipher not initialized!"); } } while (0)









VALUE cCipher;
VALUE eCipherError;
static ID id_auth_tag_len, id_key_set;

static VALUE ossl_cipher_alloc(VALUE klass);
static void ossl_cipher_free(void *ptr);

static const rb_data_type_t ossl_cipher_type = {
"OpenSSL/Cipher",
{
0, ossl_cipher_free,
},
0, 0, RUBY_TYPED_FREE_IMMEDIATELY,
};




const EVP_CIPHER *
ossl_evp_get_cipherbyname(VALUE obj)
{
if (rb_obj_is_kind_of(obj, cCipher)) {
EVP_CIPHER_CTX *ctx;

GetCipher(obj, ctx);

return EVP_CIPHER_CTX_cipher(ctx);
}
else {
const EVP_CIPHER *cipher;

StringValueCStr(obj);
cipher = EVP_get_cipherbyname(RSTRING_PTR(obj));
if (!cipher)
ossl_raise(rb_eArgError,
"unsupported cipher algorithm: %"PRIsVALUE, obj);

return cipher;
}
}

VALUE
ossl_cipher_new(const EVP_CIPHER *cipher)
{
VALUE ret;
EVP_CIPHER_CTX *ctx;

ret = ossl_cipher_alloc(cCipher);
AllocCipher(ret, ctx);
if (EVP_CipherInit_ex(ctx, cipher, NULL, NULL, NULL, -1) != 1)
ossl_raise(eCipherError, NULL);

return ret;
}




static void
ossl_cipher_free(void *ptr)
{
EVP_CIPHER_CTX_free(ptr);
}

static VALUE
ossl_cipher_alloc(VALUE klass)
{
return NewCipher(klass);
}









static VALUE
ossl_cipher_initialize(VALUE self, VALUE str)
{
EVP_CIPHER_CTX *ctx;
const EVP_CIPHER *cipher;
char *name;

name = StringValueCStr(str);
GetCipherInit(self, ctx);
if (ctx) {
ossl_raise(rb_eRuntimeError, "Cipher already initialized!");
}
AllocCipher(self, ctx);
if (!(cipher = EVP_get_cipherbyname(name))) {
ossl_raise(rb_eRuntimeError, "unsupported cipher algorithm (%"PRIsVALUE")", str);
}
if (EVP_CipherInit_ex(ctx, cipher, NULL, NULL, NULL, -1) != 1)
ossl_raise(eCipherError, NULL);

return self;
}

static VALUE
ossl_cipher_copy(VALUE self, VALUE other)
{
EVP_CIPHER_CTX *ctx1, *ctx2;

rb_check_frozen(self);
if (self == other) return self;

GetCipherInit(self, ctx1);
if (!ctx1) {
AllocCipher(self, ctx1);
}
GetCipher(other, ctx2);
if (EVP_CIPHER_CTX_copy(ctx1, ctx2) != 1)
ossl_raise(eCipherError, NULL);

return self;
}

static void*
add_cipher_name_to_ary(const OBJ_NAME *name, VALUE ary)
{
rb_ary_push(ary, rb_str_new2(name->name));
return NULL;
}







static VALUE
ossl_s_ciphers(VALUE self)
{
VALUE ary;

ary = rb_ary_new();
OBJ_NAME_do_all_sorted(OBJ_NAME_TYPE_CIPHER_METH,
(void(*)(const OBJ_NAME*,void*))add_cipher_name_to_ary,
(void*)ary);

return ary;
}










static VALUE
ossl_cipher_reset(VALUE self)
{
EVP_CIPHER_CTX *ctx;

GetCipher(self, ctx);
if (EVP_CipherInit_ex(ctx, NULL, NULL, NULL, NULL, -1) != 1)
ossl_raise(eCipherError, NULL);

return self;
}

static VALUE
ossl_cipher_init(int argc, VALUE *argv, VALUE self, int mode)
{
EVP_CIPHER_CTX *ctx;
unsigned char key[EVP_MAX_KEY_LENGTH], *p_key = NULL;
unsigned char iv[EVP_MAX_IV_LENGTH], *p_iv = NULL;
VALUE pass, init_v;

if(rb_scan_args(argc, argv, "02", &pass, &init_v) > 0){





VALUE cname = rb_class_path(rb_obj_class(self));
rb_warn("arguments for %"PRIsVALUE"#encrypt and %"PRIsVALUE"#decrypt were deprecated; "
"use %"PRIsVALUE"#pkcs5_keyivgen to derive key and IV",
cname, cname, cname);
StringValue(pass);
GetCipher(self, ctx);
if (NIL_P(init_v)) memcpy(iv, "OpenSSL for Ruby rulez!", sizeof(iv));
else{
StringValue(init_v);
if (EVP_MAX_IV_LENGTH > RSTRING_LEN(init_v)) {
memset(iv, 0, EVP_MAX_IV_LENGTH);
memcpy(iv, RSTRING_PTR(init_v), RSTRING_LEN(init_v));
}
else memcpy(iv, RSTRING_PTR(init_v), sizeof(iv));
}
EVP_BytesToKey(EVP_CIPHER_CTX_cipher(ctx), EVP_md5(), iv,
(unsigned char *)RSTRING_PTR(pass), RSTRING_LENINT(pass), 1, key, NULL);
p_key = key;
p_iv = iv;
}
else {
GetCipher(self, ctx);
}
if (EVP_CipherInit_ex(ctx, NULL, NULL, p_key, p_iv, mode) != 1) {
ossl_raise(eCipherError, NULL);
}

rb_ivar_set(self, id_key_set, p_key ? Qtrue : Qfalse);

return self;
}













static VALUE
ossl_cipher_encrypt(int argc, VALUE *argv, VALUE self)
{
return ossl_cipher_init(argc, argv, self, 1);
}













static VALUE
ossl_cipher_decrypt(int argc, VALUE *argv, VALUE self)
{
return ossl_cipher_init(argc, argv, self, 0);
}





















static VALUE
ossl_cipher_pkcs5_keyivgen(int argc, VALUE *argv, VALUE self)
{
EVP_CIPHER_CTX *ctx;
const EVP_MD *digest;
VALUE vpass, vsalt, viter, vdigest;
unsigned char key[EVP_MAX_KEY_LENGTH], iv[EVP_MAX_IV_LENGTH], *salt = NULL;
int iter;

rb_scan_args(argc, argv, "13", &vpass, &vsalt, &viter, &vdigest);
StringValue(vpass);
if(!NIL_P(vsalt)){
StringValue(vsalt);
if(RSTRING_LEN(vsalt) != PKCS5_SALT_LEN)
ossl_raise(eCipherError, "salt must be an 8-octet string");
salt = (unsigned char *)RSTRING_PTR(vsalt);
}
iter = NIL_P(viter) ? 2048 : NUM2INT(viter);
if (iter <= 0)
rb_raise(rb_eArgError, "iterations must be a positive integer");
digest = NIL_P(vdigest) ? EVP_md5() : ossl_evp_get_digestbyname(vdigest);
GetCipher(self, ctx);
EVP_BytesToKey(EVP_CIPHER_CTX_cipher(ctx), digest, salt,
(unsigned char *)RSTRING_PTR(vpass), RSTRING_LENINT(vpass), iter, key, iv);
if (EVP_CipherInit_ex(ctx, NULL, NULL, key, iv, -1) != 1)
ossl_raise(eCipherError, NULL);
OPENSSL_cleanse(key, sizeof key);
OPENSSL_cleanse(iv, sizeof iv);

rb_ivar_set(self, id_key_set, Qtrue);

return Qnil;
}

static int
ossl_cipher_update_long(EVP_CIPHER_CTX *ctx, unsigned char *out, long *out_len_ptr,
const unsigned char *in, long in_len)
{
int out_part_len;
int limit = INT_MAX / 2 + 1;
long out_len = 0;

do {
int in_part_len = in_len > limit ? limit : (int)in_len;

if (!EVP_CipherUpdate(ctx, out ? (out + out_len) : 0,
&out_part_len, in, in_part_len))
return 0;

out_len += out_part_len;
in += in_part_len;
} while ((in_len -= limit) > 0);

if (out_len_ptr)
*out_len_ptr = out_len;

return 1;
}













static VALUE
ossl_cipher_update(int argc, VALUE *argv, VALUE self)
{
EVP_CIPHER_CTX *ctx;
unsigned char *in;
long in_len, out_len;
VALUE data, str;

rb_scan_args(argc, argv, "11", &data, &str);

if (!RTEST(rb_attr_get(self, id_key_set)))
ossl_raise(eCipherError, "key not set");

StringValue(data);
in = (unsigned char *)RSTRING_PTR(data);
if ((in_len = RSTRING_LEN(data)) == 0)
ossl_raise(rb_eArgError, "data must not be empty");
GetCipher(self, ctx);
out_len = in_len+EVP_CIPHER_CTX_block_size(ctx);
if (out_len <= 0) {
ossl_raise(rb_eRangeError,
"data too big to make output buffer: %ld bytes", in_len);
}

if (NIL_P(str)) {
str = rb_str_new(0, out_len);
} else {
StringValue(str);
rb_str_resize(str, out_len);
}

if (!ossl_cipher_update_long(ctx, (unsigned char *)RSTRING_PTR(str), &out_len, in, in_len))
ossl_raise(eCipherError, NULL);
assert(out_len < RSTRING_LEN(str));
rb_str_set_len(str, out_len);

return str;
}















static VALUE
ossl_cipher_final(VALUE self)
{
EVP_CIPHER_CTX *ctx;
int out_len;
VALUE str;

GetCipher(self, ctx);
str = rb_str_new(0, EVP_CIPHER_CTX_block_size(ctx));
if (!EVP_CipherFinal_ex(ctx, (unsigned char *)RSTRING_PTR(str), &out_len))
ossl_raise(eCipherError, NULL);
assert(out_len <= RSTRING_LEN(str));
rb_str_set_len(str, out_len);

return str;
}








static VALUE
ossl_cipher_name(VALUE self)
{
EVP_CIPHER_CTX *ctx;

GetCipher(self, ctx);

return rb_str_new2(EVP_CIPHER_name(EVP_CIPHER_CTX_cipher(ctx)));
}












static VALUE
ossl_cipher_set_key(VALUE self, VALUE key)
{
EVP_CIPHER_CTX *ctx;
int key_len;

StringValue(key);
GetCipher(self, ctx);

key_len = EVP_CIPHER_CTX_key_length(ctx);
if (RSTRING_LEN(key) != key_len)
ossl_raise(rb_eArgError, "key must be %d bytes", key_len);

if (EVP_CipherInit_ex(ctx, NULL, NULL, (unsigned char *)RSTRING_PTR(key), NULL, -1) != 1)
ossl_raise(eCipherError, NULL);

rb_ivar_set(self, id_key_set, Qtrue);

return key;
}













static VALUE
ossl_cipher_set_iv(VALUE self, VALUE iv)
{
EVP_CIPHER_CTX *ctx;
int iv_len = 0;

StringValue(iv);
GetCipher(self, ctx);

if (EVP_CIPHER_flags(EVP_CIPHER_CTX_cipher(ctx)) & EVP_CIPH_FLAG_AEAD_CIPHER)
iv_len = (int)(VALUE)EVP_CIPHER_CTX_get_app_data(ctx);
if (!iv_len)
iv_len = EVP_CIPHER_CTX_iv_length(ctx);
if (RSTRING_LEN(iv) != iv_len)
ossl_raise(rb_eArgError, "iv must be %d bytes", iv_len);

if (EVP_CipherInit_ex(ctx, NULL, NULL, NULL, (unsigned char *)RSTRING_PTR(iv), -1) != 1)
ossl_raise(eCipherError, NULL);

return iv;
}








static VALUE
ossl_cipher_is_authenticated(VALUE self)
{
EVP_CIPHER_CTX *ctx;

GetCipher(self, ctx);

return (EVP_CIPHER_flags(EVP_CIPHER_CTX_cipher(ctx)) & EVP_CIPH_FLAG_AEAD_CIPHER) ? Qtrue : Qfalse;
}



















static VALUE
ossl_cipher_set_auth_data(VALUE self, VALUE data)
{
EVP_CIPHER_CTX *ctx;
unsigned char *in;
long in_len, out_len;

StringValue(data);

in = (unsigned char *) RSTRING_PTR(data);
in_len = RSTRING_LEN(data);

GetCipher(self, ctx);
if (!(EVP_CIPHER_flags(EVP_CIPHER_CTX_cipher(ctx)) & EVP_CIPH_FLAG_AEAD_CIPHER))
ossl_raise(eCipherError, "AEAD not supported by this cipher");

if (!ossl_cipher_update_long(ctx, NULL, &out_len, in, in_len))
ossl_raise(eCipherError, "couldn't set additional authenticated data");

return data;
}
















static VALUE
ossl_cipher_get_auth_tag(int argc, VALUE *argv, VALUE self)
{
VALUE vtag_len, ret;
EVP_CIPHER_CTX *ctx;
int tag_len = 16;

rb_scan_args(argc, argv, "01", &vtag_len);
if (NIL_P(vtag_len))
vtag_len = rb_attr_get(self, id_auth_tag_len);
if (!NIL_P(vtag_len))
tag_len = NUM2INT(vtag_len);

GetCipher(self, ctx);

if (!(EVP_CIPHER_flags(EVP_CIPHER_CTX_cipher(ctx)) & EVP_CIPH_FLAG_AEAD_CIPHER))
ossl_raise(eCipherError, "authentication tag not supported by this cipher");

ret = rb_str_new(NULL, tag_len);
if (!EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_AEAD_GET_TAG, tag_len, RSTRING_PTR(ret)))
ossl_raise(eCipherError, "retrieving the authentication tag failed");

return ret;
}














static VALUE
ossl_cipher_set_auth_tag(VALUE self, VALUE vtag)
{
EVP_CIPHER_CTX *ctx;
unsigned char *tag;
int tag_len;

StringValue(vtag);
tag = (unsigned char *) RSTRING_PTR(vtag);
tag_len = RSTRING_LENINT(vtag);

GetCipher(self, ctx);
if (!(EVP_CIPHER_flags(EVP_CIPHER_CTX_cipher(ctx)) & EVP_CIPH_FLAG_AEAD_CIPHER))
ossl_raise(eCipherError, "authentication tag not supported by this cipher");

if (!EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_AEAD_SET_TAG, tag_len, tag))
ossl_raise(eCipherError, "unable to set AEAD tag");

return vtag;
}












static VALUE
ossl_cipher_set_auth_tag_len(VALUE self, VALUE vlen)
{
int tag_len = NUM2INT(vlen);
EVP_CIPHER_CTX *ctx;

GetCipher(self, ctx);
if (!(EVP_CIPHER_flags(EVP_CIPHER_CTX_cipher(ctx)) & EVP_CIPH_FLAG_AEAD_CIPHER))
ossl_raise(eCipherError, "AEAD not supported by this cipher");

if (!EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_AEAD_SET_TAG, tag_len, NULL))
ossl_raise(eCipherError, "unable to set authentication tag length");


rb_ivar_set(self, id_auth_tag_len, INT2NUM(tag_len));

return vlen;
}









static VALUE
ossl_cipher_set_iv_length(VALUE self, VALUE iv_length)
{
int len = NUM2INT(iv_length);
EVP_CIPHER_CTX *ctx;

GetCipher(self, ctx);
if (!(EVP_CIPHER_flags(EVP_CIPHER_CTX_cipher(ctx)) & EVP_CIPH_FLAG_AEAD_CIPHER))
ossl_raise(eCipherError, "cipher does not support AEAD");

if (!EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_AEAD_SET_IVLEN, len, NULL))
ossl_raise(eCipherError, "unable to set IV length");





EVP_CIPHER_CTX_set_app_data(ctx, (void *)(VALUE)len);

return iv_length;
}













static VALUE
ossl_cipher_set_key_length(VALUE self, VALUE key_length)
{
int len = NUM2INT(key_length);
EVP_CIPHER_CTX *ctx;

GetCipher(self, ctx);
if (EVP_CIPHER_CTX_set_key_length(ctx, len) != 1)
ossl_raise(eCipherError, NULL);

return key_length;
}











static VALUE
ossl_cipher_set_padding(VALUE self, VALUE padding)
{
EVP_CIPHER_CTX *ctx;
int pad = NUM2INT(padding);

GetCipher(self, ctx);
if (EVP_CIPHER_CTX_set_padding(ctx, pad) != 1)
ossl_raise(eCipherError, NULL);
return padding;
}







static VALUE
ossl_cipher_key_length(VALUE self)
{
EVP_CIPHER_CTX *ctx;

GetCipher(self, ctx);

return INT2NUM(EVP_CIPHER_CTX_key_length(ctx));
}







static VALUE
ossl_cipher_iv_length(VALUE self)
{
EVP_CIPHER_CTX *ctx;
int len = 0;

GetCipher(self, ctx);
if (EVP_CIPHER_flags(EVP_CIPHER_CTX_cipher(ctx)) & EVP_CIPH_FLAG_AEAD_CIPHER)
len = (int)(VALUE)EVP_CIPHER_CTX_get_app_data(ctx);
if (!len)
len = EVP_CIPHER_CTX_iv_length(ctx);

return INT2NUM(len);
}







static VALUE
ossl_cipher_block_size(VALUE self)
{
EVP_CIPHER_CTX *ctx;

GetCipher(self, ctx);

return INT2NUM(EVP_CIPHER_CTX_block_size(ctx));
}




void
Init_ossl_cipher(void)
{
#if 0
mOSSL = rb_define_module("OpenSSL");
eOSSLError = rb_define_class_under(mOSSL, "OpenSSLError", rb_eStandardError);
#endif

















































































































































































































cCipher = rb_define_class_under(mOSSL, "Cipher", rb_cObject);
eCipherError = rb_define_class_under(cCipher, "CipherError", eOSSLError);

rb_define_alloc_func(cCipher, ossl_cipher_alloc);
rb_define_method(cCipher, "initialize_copy", ossl_cipher_copy, 1);
rb_define_module_function(cCipher, "ciphers", ossl_s_ciphers, 0);
rb_define_method(cCipher, "initialize", ossl_cipher_initialize, 1);
rb_define_method(cCipher, "reset", ossl_cipher_reset, 0);
rb_define_method(cCipher, "encrypt", ossl_cipher_encrypt, -1);
rb_define_method(cCipher, "decrypt", ossl_cipher_decrypt, -1);
rb_define_method(cCipher, "pkcs5_keyivgen", ossl_cipher_pkcs5_keyivgen, -1);
rb_define_method(cCipher, "update", ossl_cipher_update, -1);
rb_define_method(cCipher, "final", ossl_cipher_final, 0);
rb_define_method(cCipher, "name", ossl_cipher_name, 0);
rb_define_method(cCipher, "key=", ossl_cipher_set_key, 1);
rb_define_method(cCipher, "auth_data=", ossl_cipher_set_auth_data, 1);
rb_define_method(cCipher, "auth_tag=", ossl_cipher_set_auth_tag, 1);
rb_define_method(cCipher, "auth_tag", ossl_cipher_get_auth_tag, -1);
rb_define_method(cCipher, "auth_tag_len=", ossl_cipher_set_auth_tag_len, 1);
rb_define_method(cCipher, "authenticated?", ossl_cipher_is_authenticated, 0);
rb_define_method(cCipher, "key_len=", ossl_cipher_set_key_length, 1);
rb_define_method(cCipher, "key_len", ossl_cipher_key_length, 0);
rb_define_method(cCipher, "iv=", ossl_cipher_set_iv, 1);
rb_define_method(cCipher, "iv_len=", ossl_cipher_set_iv_length, 1);
rb_define_method(cCipher, "iv_len", ossl_cipher_iv_length, 0);
rb_define_method(cCipher, "block_size", ossl_cipher_block_size, 0);
rb_define_method(cCipher, "padding=", ossl_cipher_set_padding, 1);

id_auth_tag_len = rb_intern_const("auth_tag_len");
id_key_set = rb_intern_const("key_set");
}
