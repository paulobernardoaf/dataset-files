#include "ossl.h"
VALUE mRandom;
VALUE eRandomError;
static VALUE
ossl_rand_seed(VALUE self, VALUE str)
{
StringValue(str);
RAND_seed(RSTRING_PTR(str), RSTRING_LENINT(str));
return str;
}
static VALUE
ossl_rand_add(VALUE self, VALUE str, VALUE entropy)
{
StringValue(str);
RAND_add(RSTRING_PTR(str), RSTRING_LENINT(str), NUM2DBL(entropy));
return self;
}
static VALUE
ossl_rand_load_file(VALUE self, VALUE filename)
{
if(!RAND_load_file(StringValueCStr(filename), -1)) {
ossl_raise(eRandomError, NULL);
}
return Qtrue;
}
static VALUE
ossl_rand_write_file(VALUE self, VALUE filename)
{
if (RAND_write_file(StringValueCStr(filename)) == -1) {
ossl_raise(eRandomError, NULL);
}
return Qtrue;
}
static VALUE
ossl_rand_bytes(VALUE self, VALUE len)
{
VALUE str;
int n = NUM2INT(len);
int ret;
str = rb_str_new(0, n);
ret = RAND_bytes((unsigned char *)RSTRING_PTR(str), n);
if (ret == 0) {
ossl_raise(eRandomError, "RAND_bytes");
} else if (ret == -1) {
ossl_raise(eRandomError, "RAND_bytes is not supported");
}
return str;
}
#if defined(HAVE_RAND_EGD)
static VALUE
ossl_rand_egd(VALUE self, VALUE filename)
{
if (RAND_egd(StringValueCStr(filename)) == -1) {
ossl_raise(eRandomError, NULL);
}
return Qtrue;
}
static VALUE
ossl_rand_egd_bytes(VALUE self, VALUE filename, VALUE len)
{
int n = NUM2INT(len);
if (RAND_egd_bytes(StringValueCStr(filename), n) == -1) {
ossl_raise(eRandomError, NULL);
}
return Qtrue;
}
#endif 
static VALUE
ossl_rand_status(VALUE self)
{
return RAND_status() ? Qtrue : Qfalse;
}
void
Init_ossl_rand(void)
{
#if 0
mOSSL = rb_define_module("OpenSSL");
eOSSLError = rb_define_class_under(mOSSL, "OpenSSLError", rb_eStandardError);
#endif
mRandom = rb_define_module_under(mOSSL, "Random");
eRandomError = rb_define_class_under(mRandom, "RandomError", eOSSLError);
rb_define_module_function(mRandom, "seed", ossl_rand_seed, 1);
rb_define_module_function(mRandom, "random_add", ossl_rand_add, 2);
rb_define_module_function(mRandom, "load_random_file", ossl_rand_load_file, 1);
rb_define_module_function(mRandom, "write_random_file", ossl_rand_write_file, 1);
rb_define_module_function(mRandom, "random_bytes", ossl_rand_bytes, 1);
#if OPENSSL_VERSION_NUMBER < 0x10101000 || defined(LIBRESSL_VERSION_NUMBER)
rb_define_alias(rb_singleton_class(mRandom), "pseudo_bytes", "random_bytes");
#endif
#if defined(HAVE_RAND_EGD)
rb_define_module_function(mRandom, "egd", ossl_rand_egd, 1);
rb_define_module_function(mRandom, "egd_bytes", ossl_rand_egd_bytes, 2);
#endif 
rb_define_module_function(mRandom, "status?", ossl_rand_status, 0);
}
