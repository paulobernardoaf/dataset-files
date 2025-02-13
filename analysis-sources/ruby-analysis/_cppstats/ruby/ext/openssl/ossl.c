#include "ossl.h"
#include <stdarg.h> 
#include <ruby/thread_native.h> 
#define OSSL_IMPL_ARY2SK(name, type, expected_class, dup) STACK_OF(type) * ossl_##name##_ary2sk0(VALUE ary) { STACK_OF(type) *sk; VALUE val; type *x; int i; Check_Type(ary, T_ARRAY); sk = sk_##type##_new_null(); if (!sk) ossl_raise(eOSSLError, NULL); for (i = 0; i < RARRAY_LEN(ary); i++) { val = rb_ary_entry(ary, i); if (!rb_obj_is_kind_of(val, expected_class)) { sk_##type##_pop_free(sk, type##_free); ossl_raise(eOSSLError, "object in array not" " of class ##type##"); } x = dup(val); sk_##type##_push(sk, x); } return sk; } STACK_OF(type) * ossl_protect_##name##_ary2sk(VALUE ary, int *status) { return (STACK_OF(type)*)rb_protect( (VALUE (*)(VALUE))ossl_##name##_ary2sk0, ary, status); } STACK_OF(type) * ossl_##name##_ary2sk(VALUE ary) { STACK_OF(type) *sk; int status = 0; sk = ossl_protect_##name##_ary2sk(ary, &status); if (status) rb_jump_tag(status); return sk; }
OSSL_IMPL_ARY2SK(x509, X509, cX509Cert, DupX509CertPtr)
#define OSSL_IMPL_SK2ARY(name, type) VALUE ossl_##name##_sk2ary(const STACK_OF(type) *sk) { type *t; int i, num; VALUE ary; if (!sk) { OSSL_Debug("empty sk!"); return Qnil; } num = sk_##type##_num(sk); if (num < 0) { OSSL_Debug("items in sk < -1???"); return rb_ary_new(); } ary = rb_ary_new2(num); for (i=0; i<num; i++) { t = sk_##type##_value(sk, i); rb_ary_push(ary, ossl_##name##_new(t)); } return ary; }
OSSL_IMPL_SK2ARY(x509, X509)
OSSL_IMPL_SK2ARY(x509crl, X509_CRL)
OSSL_IMPL_SK2ARY(x509name, X509_NAME)
static VALUE
ossl_str_new_i(VALUE size)
{
return rb_str_new(NULL, (long)size);
}
VALUE
ossl_str_new(const char *ptr, long len, int *pstate)
{
VALUE str;
int state;
str = rb_protect(ossl_str_new_i, len, &state);
if (pstate)
*pstate = state;
if (state) {
if (!pstate)
rb_set_errinfo(Qnil);
return Qnil;
}
if (ptr)
memcpy(RSTRING_PTR(str), ptr, len);
return str;
}
VALUE
ossl_buf2str(char *buf, int len)
{
VALUE str;
int state;
str = ossl_str_new(buf, len, &state);
OPENSSL_free(buf);
if (state)
rb_jump_tag(state);
return str;
}
void
ossl_bin2hex(unsigned char *in, char *out, size_t inlen)
{
const char *hex = "0123456789abcdef";
size_t i;
assert(inlen <= LONG_MAX / 2);
for (i = 0; i < inlen; i++) {
unsigned char p = in[i];
out[i * 2 + 0] = hex[p >> 4];
out[i * 2 + 1] = hex[p & 0x0f];
}
}
VALUE
ossl_pem_passwd_value(VALUE pass)
{
if (NIL_P(pass))
return Qnil;
StringValue(pass);
if (RSTRING_LEN(pass) > PEM_BUFSIZE)
ossl_raise(eOSSLError, "password must not be longer than %d bytes", PEM_BUFSIZE);
return pass;
}
static VALUE
ossl_pem_passwd_cb0(VALUE flag)
{
VALUE pass = rb_yield(flag);
if (NIL_P(pass))
return Qnil;
StringValue(pass);
return pass;
}
int
ossl_pem_passwd_cb(char *buf, int max_len, int flag, void *pwd_)
{
long len;
int status;
VALUE rflag, pass = (VALUE)pwd_;
if (RTEST(pass)) {
if (RB_TYPE_P(pass, T_STRING)) {
len = RSTRING_LEN(pass);
if (len <= max_len) {
memcpy(buf, RSTRING_PTR(pass), len);
return (int)len;
}
}
OSSL_Debug("passed data is not valid String???");
return -1;
}
if (!rb_block_given_p()) {
return PEM_def_callback(buf, max_len, flag, NULL);
}
while (1) {
rflag = flag ? Qtrue : Qfalse;
pass = rb_protect(ossl_pem_passwd_cb0, rflag, &status);
if (status) {
rb_set_errinfo(Qnil);
return -1;
}
if (NIL_P(pass))
return -1;
len = RSTRING_LEN(pass);
if (len > max_len) {
rb_warning("password must not be longer than %d bytes", max_len);
continue;
}
memcpy(buf, RSTRING_PTR(pass), len);
break;
}
return (int)len;
}
VALUE mOSSL;
VALUE eOSSLError;
static ID ossl_s_to_der;
VALUE
ossl_to_der(VALUE obj)
{
VALUE tmp;
tmp = rb_funcall(obj, ossl_s_to_der, 0);
StringValue(tmp);
return tmp;
}
VALUE
ossl_to_der_if_possible(VALUE obj)
{
if(rb_respond_to(obj, ossl_s_to_der))
return ossl_to_der(obj);
return obj;
}
static VALUE
ossl_make_error(VALUE exc, const char *fmt, va_list args)
{
VALUE str = Qnil;
unsigned long e;
if (fmt) {
str = rb_vsprintf(fmt, args);
}
e = ERR_peek_last_error();
if (e) {
const char *msg = ERR_reason_error_string(e);
if (NIL_P(str)) {
if (msg) str = rb_str_new_cstr(msg);
}
else {
if (RSTRING_LEN(str)) rb_str_cat2(str, ": ");
rb_str_cat2(str, msg ? msg : "(null)");
}
ossl_clear_error();
}
if (NIL_P(str)) str = rb_str_new(0, 0);
return rb_exc_new3(exc, str);
}
void
ossl_raise(VALUE exc, const char *fmt, ...)
{
va_list args;
VALUE err;
va_start(args, fmt);
err = ossl_make_error(exc, fmt, args);
va_end(args);
rb_exc_raise(err);
}
void
ossl_clear_error(void)
{
if (dOSSL == Qtrue) {
unsigned long e;
const char *file, *data, *errstr;
int line, flags;
while ((e = ERR_get_error_line_data(&file, &line, &data, &flags))) {
errstr = ERR_error_string(e, NULL);
if (!errstr)
errstr = "(null)";
if (flags & ERR_TXT_STRING) {
if (!data)
data = "(null)";
rb_warn("error on stack: %s (%s)", errstr, data);
}
else {
rb_warn("error on stack: %s", errstr);
}
}
}
else {
ERR_clear_error();
}
}
VALUE
ossl_get_errors(VALUE _)
{
VALUE ary;
long e;
ary = rb_ary_new();
while ((e = ERR_get_error()) != 0){
rb_ary_push(ary, rb_str_new2(ERR_error_string(e, NULL)));
}
return ary;
}
VALUE dOSSL;
#if !defined(HAVE_VA_ARGS_MACRO)
void
ossl_debug(const char *fmt, ...)
{
va_list args;
if (dOSSL == Qtrue) {
fprintf(stderr, "OSSL_DEBUG: ");
va_start(args, fmt);
vfprintf(stderr, fmt, args);
va_end(args);
fprintf(stderr, " [CONTEXT N/A]\n");
}
}
#endif
static VALUE
ossl_debug_get(VALUE self)
{
return dOSSL;
}
static VALUE
ossl_debug_set(VALUE self, VALUE val)
{
dOSSL = RTEST(val) ? Qtrue : Qfalse;
return val;
}
static VALUE
ossl_fips_mode_get(VALUE self)
{
#if defined(OPENSSL_FIPS)
VALUE enabled;
enabled = FIPS_mode() ? Qtrue : Qfalse;
return enabled;
#else
return Qfalse;
#endif
}
static VALUE
ossl_fips_mode_set(VALUE self, VALUE enabled)
{
#if defined(OPENSSL_FIPS)
if (RTEST(enabled)) {
int mode = FIPS_mode();
if(!mode && !FIPS_mode_set(1)) 
ossl_raise(eOSSLError, "Turning on FIPS mode failed");
} else {
if(!FIPS_mode_set(0)) 
ossl_raise(eOSSLError, "Turning off FIPS mode failed");
}
return enabled;
#else
if (RTEST(enabled))
ossl_raise(eOSSLError, "This version of OpenSSL does not support FIPS mode");
return enabled;
#endif
}
#if defined(OSSL_DEBUG)
#if !defined(LIBRESSL_VERSION_NUMBER) && (OPENSSL_VERSION_NUMBER >= 0x10100000 && !defined(OPENSSL_NO_CRYPTO_MDEBUG) || defined(CRYPTO_malloc_debug_init))
static VALUE
mem_check_start(VALUE self)
{
CRYPTO_mem_ctrl(CRYPTO_MEM_CHECK_ON);
return Qnil;
}
static VALUE
print_mem_leaks(VALUE self)
{
#if OPENSSL_VERSION_NUMBER >= 0x10100000
int ret;
#endif
BN_CTX_free(ossl_bn_ctx);
ossl_bn_ctx = NULL;
#if OPENSSL_VERSION_NUMBER >= 0x10100000
ret = CRYPTO_mem_leaks_fp(stderr);
if (ret < 0)
ossl_raise(eOSSLError, "CRYPTO_mem_leaks_fp");
return ret ? Qfalse : Qtrue;
#else
CRYPTO_mem_leaks_fp(stderr);
return Qnil;
#endif
}
#endif
#endif
#if !defined(HAVE_OPENSSL_110_THREADING_API)
struct CRYPTO_dynlock_value {
rb_nativethread_lock_t lock;
rb_nativethread_id_t owner;
size_t count;
};
static void
ossl_lock_init(struct CRYPTO_dynlock_value *l)
{
rb_nativethread_lock_initialize(&l->lock);
l->count = 0;
}
static void
ossl_lock_unlock(int mode, struct CRYPTO_dynlock_value *l)
{
if (mode & CRYPTO_LOCK) {
rb_nativethread_id_t tid = rb_nativethread_self();
if (l->count && l->owner == tid) {
l->count++;
return;
}
rb_nativethread_lock_lock(&l->lock);
l->owner = tid;
l->count = 1;
} else {
if (!--l->count)
rb_nativethread_lock_unlock(&l->lock);
}
}
static struct CRYPTO_dynlock_value *
ossl_dyn_create_callback(const char *file, int line)
{
struct CRYPTO_dynlock_value *dynlock =
OPENSSL_malloc(sizeof(struct CRYPTO_dynlock_value));
if (dynlock)
ossl_lock_init(dynlock);
return dynlock;
}
static void
ossl_dyn_lock_callback(int mode, struct CRYPTO_dynlock_value *l, const char *file, int line)
{
ossl_lock_unlock(mode, l);
}
static void
ossl_dyn_destroy_callback(struct CRYPTO_dynlock_value *l, const char *file, int line)
{
rb_nativethread_lock_destroy(&l->lock);
OPENSSL_free(l);
}
static void ossl_threadid_func(CRYPTO_THREADID *id)
{
CRYPTO_THREADID_set_pointer(id, (void *)rb_nativethread_self());
}
static struct CRYPTO_dynlock_value *ossl_locks;
static void
ossl_lock_callback(int mode, int type, const char *file, int line)
{
ossl_lock_unlock(mode, &ossl_locks[type]);
}
static void Init_ossl_locks(void)
{
int i;
int num_locks = CRYPTO_num_locks();
ossl_locks = ALLOC_N(struct CRYPTO_dynlock_value, num_locks);
for (i = 0; i < num_locks; i++)
ossl_lock_init(&ossl_locks[i]);
CRYPTO_THREADID_set_callback(ossl_threadid_func);
CRYPTO_set_locking_callback(ossl_lock_callback);
CRYPTO_set_dynlock_create_callback(ossl_dyn_create_callback);
CRYPTO_set_dynlock_lock_callback(ossl_dyn_lock_callback);
CRYPTO_set_dynlock_destroy_callback(ossl_dyn_destroy_callback);
}
#endif 
static VALUE
ossl_crypto_fixed_length_secure_compare(VALUE dummy, VALUE str1, VALUE str2)
{
const unsigned char *p1 = (const unsigned char *)StringValuePtr(str1);
const unsigned char *p2 = (const unsigned char *)StringValuePtr(str2);
long len1 = RSTRING_LEN(str1);
long len2 = RSTRING_LEN(str2);
if (len1 != len2) {
ossl_raise(rb_eArgError, "inputs must be of equal length");
}
switch (CRYPTO_memcmp(p1, p2, len1)) {
case 0: return Qtrue;
default: return Qfalse;
}
}
void
Init_openssl(void)
{
#undef rb_intern
#if 0
tzset();
#endif
#if !defined(LIBRESSL_VERSION_NUMBER) && OPENSSL_VERSION_NUMBER >= 0x10100000
if (!OPENSSL_init_ssl(0, NULL))
rb_raise(rb_eRuntimeError, "OPENSSL_init_ssl");
#else
OpenSSL_add_ssl_algorithms();
OpenSSL_add_all_algorithms();
ERR_load_crypto_strings();
SSL_load_error_strings();
#endif
mOSSL = rb_define_module("OpenSSL");
rb_global_variable(&mOSSL);
rb_define_singleton_method(mOSSL, "fixed_length_secure_compare", ossl_crypto_fixed_length_secure_compare, 2);
rb_define_const(mOSSL, "OPENSSL_VERSION", rb_str_new2(OPENSSL_VERSION_TEXT));
#if !defined(LIBRESSL_VERSION_NUMBER) && OPENSSL_VERSION_NUMBER >= 0x10100000
rb_define_const(mOSSL, "OPENSSL_LIBRARY_VERSION", rb_str_new2(OpenSSL_version(OPENSSL_VERSION)));
#else
rb_define_const(mOSSL, "OPENSSL_LIBRARY_VERSION", rb_str_new2(SSLeay_version(SSLEAY_VERSION)));
#endif
rb_define_const(mOSSL, "OPENSSL_VERSION_NUMBER", INT2NUM(OPENSSL_VERSION_NUMBER));
rb_define_const(mOSSL, "OPENSSL_FIPS",
#if defined(OPENSSL_FIPS)
Qtrue
#else
Qfalse
#endif
);
rb_define_module_function(mOSSL, "fips_mode", ossl_fips_mode_get, 0);
rb_define_module_function(mOSSL, "fips_mode=", ossl_fips_mode_set, 1);
eOSSLError = rb_define_class_under(mOSSL,"OpenSSLError",rb_eStandardError);
rb_global_variable(&eOSSLError);
dOSSL = Qfalse;
rb_global_variable(&dOSSL);
rb_define_module_function(mOSSL, "debug", ossl_debug_get, 0);
rb_define_module_function(mOSSL, "debug=", ossl_debug_set, 1);
rb_define_module_function(mOSSL, "errors", ossl_get_errors, 0);
ossl_s_to_der = rb_intern("to_der");
#if !defined(HAVE_OPENSSL_110_THREADING_API)
Init_ossl_locks();
#endif
Init_ossl_bn();
Init_ossl_cipher();
Init_ossl_config();
Init_ossl_digest();
Init_ossl_hmac();
Init_ossl_ns_spki();
Init_ossl_pkcs12();
Init_ossl_pkcs7();
Init_ossl_pkey();
Init_ossl_rand();
Init_ossl_ssl();
#if !defined(OPENSSL_NO_TS)
Init_ossl_ts();
#endif
Init_ossl_x509();
Init_ossl_ocsp();
Init_ossl_engine();
Init_ossl_asn1();
Init_ossl_kdf();
#if defined(OSSL_DEBUG)
#if !defined(LIBRESSL_VERSION_NUMBER) && (OPENSSL_VERSION_NUMBER >= 0x10100000 && !defined(OPENSSL_NO_CRYPTO_MDEBUG) || defined(CRYPTO_malloc_debug_init))
rb_define_module_function(mOSSL, "mem_check_start", mem_check_start, 0);
rb_define_module_function(mOSSL, "print_mem_leaks", print_mem_leaks, 0);
#if defined(CRYPTO_malloc_debug_init) 
CRYPTO_malloc_debug_init();
#endif
#if defined(V_CRYPTO_MDEBUG_ALL) 
CRYPTO_set_mem_debug_options(V_CRYPTO_MDEBUG_ALL);
#endif
#if OPENSSL_VERSION_NUMBER < 0x10100000 
{
int i;
for (i = 0; i <= 15; i++) {
if (CRYPTO_get_ex_new_index(i, 0, (void *)"ossl-mdebug-dummy", 0, 0, 0) < 0)
rb_raise(rb_eRuntimeError, "CRYPTO_get_ex_new_index for "
"class index %d failed", i);
}
}
#endif
#endif
#endif
}
