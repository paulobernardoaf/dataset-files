#include "ossl.h"
#define NewBN(klass) TypedData_Wrap_Struct((klass), &ossl_bn_type, 0)
#define SetBN(obj, bn) do { if (!(bn)) { ossl_raise(rb_eRuntimeError, "BN wasn't initialized!"); } RTYPEDDATA_DATA(obj) = (bn); } while (0)
#define GetBN(obj, bn) do { TypedData_Get_Struct((obj), BIGNUM, &ossl_bn_type, (bn)); if (!(bn)) { ossl_raise(rb_eRuntimeError, "BN wasn't initialized!"); } } while (0)
static void
ossl_bn_free(void *ptr)
{
BN_clear_free(ptr);
}
static const rb_data_type_t ossl_bn_type = {
"OpenSSL/BN",
{
0, ossl_bn_free,
},
0, 0, RUBY_TYPED_FREE_IMMEDIATELY,
};
VALUE cBN;
VALUE eBNError;
VALUE
ossl_bn_new(const BIGNUM *bn)
{
BIGNUM *newbn;
VALUE obj;
obj = NewBN(cBN);
newbn = bn ? BN_dup(bn) : BN_new();
if (!newbn) {
ossl_raise(eBNError, NULL);
}
SetBN(obj, newbn);
return obj;
}
static BIGNUM *
integer_to_bnptr(VALUE obj, BIGNUM *orig)
{
BIGNUM *bn;
if (FIXNUM_P(obj)) {
long i;
unsigned char bin[sizeof(long)];
long n = FIX2LONG(obj);
unsigned long un = labs(n);
for (i = sizeof(long) - 1; 0 <= i; i--) {
bin[i] = un & 0xff;
un >>= 8;
}
bn = BN_bin2bn(bin, sizeof(bin), orig);
if (!bn)
ossl_raise(eBNError, "BN_bin2bn");
if (n < 0)
BN_set_negative(bn, 1);
}
else { 
size_t len = rb_absint_size(obj, NULL);
unsigned char *bin;
VALUE buf;
int sign;
if (INT_MAX < len) {
rb_raise(eBNError, "bignum too long");
}
bin = (unsigned char*)ALLOCV_N(unsigned char, buf, len);
sign = rb_integer_pack(obj, bin, len, 1, 0, INTEGER_PACK_BIG_ENDIAN);
bn = BN_bin2bn(bin, (int)len, orig);
ALLOCV_END(buf);
if (!bn)
ossl_raise(eBNError, "BN_bin2bn");
if (sign < 0)
BN_set_negative(bn, 1);
}
return bn;
}
static VALUE
try_convert_to_bn(VALUE obj)
{
BIGNUM *bn;
VALUE newobj = Qnil;
if (rb_obj_is_kind_of(obj, cBN))
return obj;
if (RB_INTEGER_TYPE_P(obj)) {
newobj = NewBN(cBN); 
bn = integer_to_bnptr(obj, NULL);
SetBN(newobj, bn);
}
return newobj;
}
BIGNUM *
ossl_bn_value_ptr(volatile VALUE *ptr)
{
VALUE tmp;
BIGNUM *bn;
tmp = try_convert_to_bn(*ptr);
if (NIL_P(tmp))
ossl_raise(rb_eTypeError, "Cannot convert into OpenSSL::BN");
GetBN(tmp, bn);
*ptr = tmp;
return bn;
}
BN_CTX *ossl_bn_ctx;
static VALUE
ossl_bn_alloc(VALUE klass)
{
BIGNUM *bn;
VALUE obj = NewBN(klass);
if (!(bn = BN_new())) {
ossl_raise(eBNError, NULL);
}
SetBN(obj, bn);
return obj;
}
static VALUE
ossl_bn_initialize(int argc, VALUE *argv, VALUE self)
{
BIGNUM *bn;
VALUE str, bs;
int base = 10;
char *ptr;
if (rb_scan_args(argc, argv, "11", &str, &bs) == 2) {
base = NUM2INT(bs);
}
if (NIL_P(str)) {
ossl_raise(rb_eArgError, "invalid argument");
}
if (RB_INTEGER_TYPE_P(str)) {
GetBN(self, bn);
integer_to_bnptr(str, bn);
return self;
}
if (RTEST(rb_obj_is_kind_of(str, cBN))) {
BIGNUM *other;
GetBN(self, bn);
GetBN(str, other); 
if (!BN_copy(bn, other)) {
ossl_raise(eBNError, NULL);
}
return self;
}
GetBN(self, bn);
switch (base) {
case 0:
ptr = StringValuePtr(str);
if (!BN_mpi2bn((unsigned char *)ptr, RSTRING_LENINT(str), bn)) {
ossl_raise(eBNError, NULL);
}
break;
case 2:
ptr = StringValuePtr(str);
if (!BN_bin2bn((unsigned char *)ptr, RSTRING_LENINT(str), bn)) {
ossl_raise(eBNError, NULL);
}
break;
case 10:
if (!BN_dec2bn(&bn, StringValueCStr(str))) {
ossl_raise(eBNError, NULL);
}
break;
case 16:
if (!BN_hex2bn(&bn, StringValueCStr(str))) {
ossl_raise(eBNError, NULL);
}
break;
default:
ossl_raise(rb_eArgError, "invalid radix %d", base);
}
return self;
}
static VALUE
ossl_bn_to_s(int argc, VALUE *argv, VALUE self)
{
BIGNUM *bn;
VALUE str, bs;
int base = 10, len;
char *buf;
if (rb_scan_args(argc, argv, "01", &bs) == 1) {
base = NUM2INT(bs);
}
GetBN(self, bn);
switch (base) {
case 0:
len = BN_bn2mpi(bn, NULL);
str = rb_str_new(0, len);
if (BN_bn2mpi(bn, (unsigned char *)RSTRING_PTR(str)) != len)
ossl_raise(eBNError, NULL);
break;
case 2:
len = BN_num_bytes(bn);
str = rb_str_new(0, len);
if (BN_bn2bin(bn, (unsigned char *)RSTRING_PTR(str)) != len)
ossl_raise(eBNError, NULL);
break;
case 10:
if (!(buf = BN_bn2dec(bn))) ossl_raise(eBNError, NULL);
str = ossl_buf2str(buf, rb_long2int(strlen(buf)));
break;
case 16:
if (!(buf = BN_bn2hex(bn))) ossl_raise(eBNError, NULL);
str = ossl_buf2str(buf, rb_long2int(strlen(buf)));
break;
default:
ossl_raise(rb_eArgError, "invalid radix %d", base);
}
return str;
}
static VALUE
ossl_bn_to_i(VALUE self)
{
BIGNUM *bn;
char *txt;
VALUE num;
GetBN(self, bn);
if (!(txt = BN_bn2hex(bn))) {
ossl_raise(eBNError, NULL);
}
num = rb_cstr_to_inum(txt, 16, Qtrue);
OPENSSL_free(txt);
return num;
}
static VALUE
ossl_bn_to_bn(VALUE self)
{
return self;
}
static VALUE
ossl_bn_coerce(VALUE self, VALUE other)
{
switch(TYPE(other)) {
case T_STRING:
self = ossl_bn_to_s(0, NULL, self);
break;
case T_FIXNUM:
case T_BIGNUM:
self = ossl_bn_to_i(self);
break;
default:
if (!RTEST(rb_obj_is_kind_of(other, cBN))) {
ossl_raise(rb_eTypeError, "Don't know how to coerce");
}
}
return rb_assoc_new(other, self);
}
#define BIGNUM_BOOL1(func) static VALUE ossl_bn_##func(VALUE self) { BIGNUM *bn; GetBN(self, bn); if (BN_##func(bn)) { return Qtrue; } return Qfalse; }
BIGNUM_BOOL1(is_zero)
BIGNUM_BOOL1(is_one)
BIGNUM_BOOL1(is_odd)
static VALUE
ossl_bn_is_negative(VALUE self)
{
BIGNUM *bn;
GetBN(self, bn);
if (BN_is_zero(bn))
return Qfalse;
return BN_is_negative(bn) ? Qtrue : Qfalse;
}
#define BIGNUM_1c(func) static VALUE ossl_bn_##func(VALUE self) { BIGNUM *bn, *result; VALUE obj; GetBN(self, bn); obj = NewBN(rb_obj_class(self)); if (!(result = BN_new())) { ossl_raise(eBNError, NULL); } if (!BN_##func(result, bn, ossl_bn_ctx)) { BN_free(result); ossl_raise(eBNError, NULL); } SetBN(obj, result); return obj; }
BIGNUM_1c(sqr)
#define BIGNUM_2(func) static VALUE ossl_bn_##func(VALUE self, VALUE other) { BIGNUM *bn1, *bn2 = GetBNPtr(other), *result; VALUE obj; GetBN(self, bn1); obj = NewBN(rb_obj_class(self)); if (!(result = BN_new())) { ossl_raise(eBNError, NULL); } if (!BN_##func(result, bn1, bn2)) { BN_free(result); ossl_raise(eBNError, NULL); } SetBN(obj, result); return obj; }
BIGNUM_2(add)
BIGNUM_2(sub)
#define BIGNUM_2c(func) static VALUE ossl_bn_##func(VALUE self, VALUE other) { BIGNUM *bn1, *bn2 = GetBNPtr(other), *result; VALUE obj; GetBN(self, bn1); obj = NewBN(rb_obj_class(self)); if (!(result = BN_new())) { ossl_raise(eBNError, NULL); } if (!BN_##func(result, bn1, bn2, ossl_bn_ctx)) { BN_free(result); ossl_raise(eBNError, NULL); } SetBN(obj, result); return obj; }
BIGNUM_2c(mul)
BIGNUM_2c(mod)
BIGNUM_2c(exp)
BIGNUM_2c(gcd)
BIGNUM_2c(mod_sqr)
BIGNUM_2c(mod_inverse)
static VALUE
ossl_bn_div(VALUE self, VALUE other)
{
BIGNUM *bn1, *bn2 = GetBNPtr(other), *r1, *r2;
VALUE klass, obj1, obj2;
GetBN(self, bn1);
klass = rb_obj_class(self);
obj1 = NewBN(klass);
obj2 = NewBN(klass);
if (!(r1 = BN_new())) {
ossl_raise(eBNError, NULL);
}
if (!(r2 = BN_new())) {
BN_free(r1);
ossl_raise(eBNError, NULL);
}
if (!BN_div(r1, r2, bn1, bn2, ossl_bn_ctx)) {
BN_free(r1);
BN_free(r2);
ossl_raise(eBNError, NULL);
}
SetBN(obj1, r1);
SetBN(obj2, r2);
return rb_ary_new3(2, obj1, obj2);
}
#define BIGNUM_3c(func) static VALUE ossl_bn_##func(VALUE self, VALUE other1, VALUE other2) { BIGNUM *bn1, *bn2 = GetBNPtr(other1); BIGNUM *bn3 = GetBNPtr(other2), *result; VALUE obj; GetBN(self, bn1); obj = NewBN(rb_obj_class(self)); if (!(result = BN_new())) { ossl_raise(eBNError, NULL); } if (!BN_##func(result, bn1, bn2, bn3, ossl_bn_ctx)) { BN_free(result); ossl_raise(eBNError, NULL); } SetBN(obj, result); return obj; }
BIGNUM_3c(mod_add)
BIGNUM_3c(mod_sub)
BIGNUM_3c(mod_mul)
BIGNUM_3c(mod_exp)
#define BIGNUM_BIT(func) static VALUE ossl_bn_##func(VALUE self, VALUE bit) { BIGNUM *bn; GetBN(self, bn); if (!BN_##func(bn, NUM2INT(bit))) { ossl_raise(eBNError, NULL); } return self; }
BIGNUM_BIT(set_bit)
BIGNUM_BIT(clear_bit)
BIGNUM_BIT(mask_bits)
static VALUE
ossl_bn_is_bit_set(VALUE self, VALUE bit)
{
int b;
BIGNUM *bn;
b = NUM2INT(bit);
GetBN(self, bn);
if (BN_is_bit_set(bn, b)) {
return Qtrue;
}
return Qfalse;
}
#define BIGNUM_SHIFT(func) static VALUE ossl_bn_##func(VALUE self, VALUE bits) { BIGNUM *bn, *result; int b; VALUE obj; b = NUM2INT(bits); GetBN(self, bn); obj = NewBN(rb_obj_class(self)); if (!(result = BN_new())) { ossl_raise(eBNError, NULL); } if (!BN_##func(result, bn, b)) { BN_free(result); ossl_raise(eBNError, NULL); } SetBN(obj, result); return obj; }
BIGNUM_SHIFT(lshift)
BIGNUM_SHIFT(rshift)
#define BIGNUM_SELF_SHIFT(func) static VALUE ossl_bn_self_##func(VALUE self, VALUE bits) { BIGNUM *bn; int b; b = NUM2INT(bits); GetBN(self, bn); if (!BN_##func(bn, bn, b)) ossl_raise(eBNError, NULL); return self; }
BIGNUM_SELF_SHIFT(lshift)
BIGNUM_SELF_SHIFT(rshift)
#define BIGNUM_RAND(func) static VALUE ossl_bn_s_##func(int argc, VALUE *argv, VALUE klass) { BIGNUM *result; int bottom = 0, top = 0, b; VALUE bits, fill, odd, obj; switch (rb_scan_args(argc, argv, "12", &bits, &fill, &odd)) { case 3: bottom = (odd == Qtrue) ? 1 : 0; case 2: top = NUM2INT(fill); } b = NUM2INT(bits); obj = NewBN(klass); if (!(result = BN_new())) { ossl_raise(eBNError, NULL); } if (!BN_##func(result, b, top, bottom)) { BN_free(result); ossl_raise(eBNError, NULL); } SetBN(obj, result); return obj; }
BIGNUM_RAND(rand)
BIGNUM_RAND(pseudo_rand)
#define BIGNUM_RAND_RANGE(func) static VALUE ossl_bn_s_##func##_range(VALUE klass, VALUE range) { BIGNUM *bn = GetBNPtr(range), *result; VALUE obj = NewBN(klass); if (!(result = BN_new())) { ossl_raise(eBNError, NULL); } if (!BN_##func##_range(result, bn)) { BN_free(result); ossl_raise(eBNError, NULL); } SetBN(obj, result); return obj; }
BIGNUM_RAND_RANGE(rand)
BIGNUM_RAND_RANGE(pseudo_rand)
static VALUE
ossl_bn_s_generate_prime(int argc, VALUE *argv, VALUE klass)
{
BIGNUM *add = NULL, *rem = NULL, *result;
int safe = 1, num;
VALUE vnum, vsafe, vadd, vrem, obj;
rb_scan_args(argc, argv, "13", &vnum, &vsafe, &vadd, &vrem);
num = NUM2INT(vnum);
if (vsafe == Qfalse) {
safe = 0;
}
if (!NIL_P(vadd)) {
add = GetBNPtr(vadd);
rem = NIL_P(vrem) ? NULL : GetBNPtr(vrem);
}
obj = NewBN(klass);
if (!(result = BN_new())) {
ossl_raise(eBNError, NULL);
}
if (!BN_generate_prime_ex(result, num, safe, add, rem, NULL)) {
BN_free(result);
ossl_raise(eBNError, NULL);
}
SetBN(obj, result);
return obj;
}
#define BIGNUM_NUM(func) static VALUE ossl_bn_##func(VALUE self) { BIGNUM *bn; GetBN(self, bn); return INT2NUM(BN_##func(bn)); }
BIGNUM_NUM(num_bytes)
BIGNUM_NUM(num_bits)
static VALUE
ossl_bn_copy(VALUE self, VALUE other)
{
BIGNUM *bn1, *bn2;
rb_check_frozen(self);
if (self == other) return self;
GetBN(self, bn1);
bn2 = GetBNPtr(other);
if (!BN_copy(bn1, bn2)) {
ossl_raise(eBNError, NULL);
}
return self;
}
static VALUE
ossl_bn_uplus(VALUE self)
{
return self;
}
static VALUE
ossl_bn_uminus(VALUE self)
{
VALUE obj;
BIGNUM *bn1, *bn2;
GetBN(self, bn1);
obj = NewBN(cBN);
bn2 = BN_dup(bn1);
if (!bn2)
ossl_raise(eBNError, "BN_dup");
SetBN(obj, bn2);
BN_set_negative(bn2, !BN_is_negative(bn2));
return obj;
}
#define BIGNUM_CMP(func) static VALUE ossl_bn_##func(VALUE self, VALUE other) { BIGNUM *bn1, *bn2 = GetBNPtr(other); GetBN(self, bn1); return INT2NUM(BN_##func(bn1, bn2)); }
BIGNUM_CMP(cmp)
BIGNUM_CMP(ucmp)
static VALUE
ossl_bn_eq(VALUE self, VALUE other)
{
BIGNUM *bn1, *bn2;
GetBN(self, bn1);
other = try_convert_to_bn(other);
if (NIL_P(other))
return Qfalse;
GetBN(other, bn2);
if (!BN_cmp(bn1, bn2)) {
return Qtrue;
}
return Qfalse;
}
static VALUE
ossl_bn_eql(VALUE self, VALUE other)
{
BIGNUM *bn1, *bn2;
if (!rb_obj_is_kind_of(other, cBN))
return Qfalse;
GetBN(self, bn1);
GetBN(other, bn2);
return BN_cmp(bn1, bn2) ? Qfalse : Qtrue;
}
static VALUE
ossl_bn_hash(VALUE self)
{
BIGNUM *bn;
VALUE tmp, hash;
unsigned char *buf;
int len;
GetBN(self, bn);
len = BN_num_bytes(bn);
buf = ALLOCV(tmp, len);
if (BN_bn2bin(bn, buf) != len) {
ALLOCV_END(tmp);
ossl_raise(eBNError, "BN_bn2bin");
}
hash = ST2FIX(rb_memhash(buf, len));
ALLOCV_END(tmp);
return hash;
}
static VALUE
ossl_bn_is_prime(int argc, VALUE *argv, VALUE self)
{
BIGNUM *bn;
VALUE vchecks;
int checks = BN_prime_checks;
if (rb_scan_args(argc, argv, "01", &vchecks) == 1) {
checks = NUM2INT(vchecks);
}
GetBN(self, bn);
switch (BN_is_prime_ex(bn, checks, ossl_bn_ctx, NULL)) {
case 1:
return Qtrue;
case 0:
return Qfalse;
default:
ossl_raise(eBNError, NULL);
}
return Qnil;
}
static VALUE
ossl_bn_is_prime_fasttest(int argc, VALUE *argv, VALUE self)
{
BIGNUM *bn;
VALUE vchecks, vtrivdiv;
int checks = BN_prime_checks, do_trial_division = 1;
rb_scan_args(argc, argv, "02", &vchecks, &vtrivdiv);
if (!NIL_P(vchecks)) {
checks = NUM2INT(vchecks);
}
GetBN(self, bn);
if (vtrivdiv == Qfalse) {
do_trial_division = 0;
}
switch (BN_is_prime_fasttest_ex(bn, checks, ossl_bn_ctx, do_trial_division, NULL)) {
case 1:
return Qtrue;
case 0:
return Qfalse;
default:
ossl_raise(eBNError, NULL);
}
return Qnil;
}
void
Init_ossl_bn(void)
{
#if 0
mOSSL = rb_define_module("OpenSSL");
eOSSLError = rb_define_class_under(mOSSL, "OpenSSLError", rb_eStandardError);
#endif
if (!(ossl_bn_ctx = BN_CTX_new())) {
ossl_raise(rb_eRuntimeError, "Cannot init BN_CTX");
}
eBNError = rb_define_class_under(mOSSL, "BNError", eOSSLError);
cBN = rb_define_class_under(mOSSL, "BN", rb_cObject);
rb_define_alloc_func(cBN, ossl_bn_alloc);
rb_define_method(cBN, "initialize", ossl_bn_initialize, -1);
rb_define_method(cBN, "initialize_copy", ossl_bn_copy, 1);
rb_define_method(cBN, "copy", ossl_bn_copy, 1);
rb_define_method(cBN, "num_bytes", ossl_bn_num_bytes, 0);
rb_define_method(cBN, "num_bits", ossl_bn_num_bits, 0);
rb_define_method(cBN, "+@", ossl_bn_uplus, 0);
rb_define_method(cBN, "-@", ossl_bn_uminus, 0);
rb_define_method(cBN, "+", ossl_bn_add, 1);
rb_define_method(cBN, "-", ossl_bn_sub, 1);
rb_define_method(cBN, "*", ossl_bn_mul, 1);
rb_define_method(cBN, "sqr", ossl_bn_sqr, 0);
rb_define_method(cBN, "/", ossl_bn_div, 1);
rb_define_method(cBN, "%", ossl_bn_mod, 1);
rb_define_method(cBN, "mod_add", ossl_bn_mod_add, 2);
rb_define_method(cBN, "mod_sub", ossl_bn_mod_sub, 2);
rb_define_method(cBN, "mod_mul", ossl_bn_mod_mul, 2);
rb_define_method(cBN, "mod_sqr", ossl_bn_mod_sqr, 1);
rb_define_method(cBN, "**", ossl_bn_exp, 1);
rb_define_method(cBN, "mod_exp", ossl_bn_mod_exp, 2);
rb_define_method(cBN, "gcd", ossl_bn_gcd, 1);
rb_define_method(cBN, "cmp", ossl_bn_cmp, 1);
rb_define_alias(cBN, "<=>", "cmp");
rb_define_method(cBN, "ucmp", ossl_bn_ucmp, 1);
rb_define_method(cBN, "eql?", ossl_bn_eql, 1);
rb_define_method(cBN, "hash", ossl_bn_hash, 0);
rb_define_method(cBN, "==", ossl_bn_eq, 1);
rb_define_alias(cBN, "===", "==");
rb_define_method(cBN, "zero?", ossl_bn_is_zero, 0);
rb_define_method(cBN, "one?", ossl_bn_is_one, 0);
rb_define_method(cBN, "odd?", ossl_bn_is_odd, 0);
rb_define_method(cBN, "negative?", ossl_bn_is_negative, 0);
rb_define_singleton_method(cBN, "rand", ossl_bn_s_rand, -1);
rb_define_singleton_method(cBN, "pseudo_rand", ossl_bn_s_pseudo_rand, -1);
rb_define_singleton_method(cBN, "rand_range", ossl_bn_s_rand_range, 1);
rb_define_singleton_method(cBN, "pseudo_rand_range", ossl_bn_s_pseudo_rand_range, 1);
rb_define_singleton_method(cBN, "generate_prime", ossl_bn_s_generate_prime, -1);
rb_define_method(cBN, "prime?", ossl_bn_is_prime, -1);
rb_define_method(cBN, "prime_fasttest?", ossl_bn_is_prime_fasttest, -1);
rb_define_method(cBN, "set_bit!", ossl_bn_set_bit, 1);
rb_define_method(cBN, "clear_bit!", ossl_bn_clear_bit, 1);
rb_define_method(cBN, "bit_set?", ossl_bn_is_bit_set, 1);
rb_define_method(cBN, "mask_bits!", ossl_bn_mask_bits, 1);
rb_define_method(cBN, "<<", ossl_bn_lshift, 1);
rb_define_method(cBN, ">>", ossl_bn_rshift, 1);
rb_define_method(cBN, "lshift!", ossl_bn_self_lshift, 1);
rb_define_method(cBN, "rshift!", ossl_bn_self_rshift, 1);
rb_define_method(cBN, "to_s", ossl_bn_to_s, -1);
rb_define_method(cBN, "to_i", ossl_bn_to_i, 0);
rb_define_alias(cBN, "to_int", "to_i");
rb_define_method(cBN, "to_bn", ossl_bn_to_bn, 0);
rb_define_method(cBN, "coerce", ossl_bn_coerce, 1);
rb_define_method(cBN, "mod_inverse", ossl_bn_mod_inverse, 1);
}
