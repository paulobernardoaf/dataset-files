#include "ossl.h"
#if !defined(OPENSSL_NO_EC)
#define EXPORT_PEM 0
#define EXPORT_DER 1
static const rb_data_type_t ossl_ec_group_type;
static const rb_data_type_t ossl_ec_point_type;
#define GetPKeyEC(obj, pkey) do { GetPKey((obj), (pkey)); if (EVP_PKEY_base_id(pkey) != EVP_PKEY_EC) { ossl_raise(rb_eRuntimeError, "THIS IS NOT A EC PKEY!"); } } while (0)
#define GetEC(obj, key) do { EVP_PKEY *_pkey; GetPKeyEC(obj, _pkey); (key) = EVP_PKEY_get0_EC_KEY(_pkey); } while (0)
#define GetECGroup(obj, group) do { TypedData_Get_Struct(obj, EC_GROUP, &ossl_ec_group_type, group); if ((group) == NULL) ossl_raise(eEC_GROUP, "EC_GROUP is not initialized"); } while (0)
#define GetECPoint(obj, point) do { TypedData_Get_Struct(obj, EC_POINT, &ossl_ec_point_type, point); if ((point) == NULL) ossl_raise(eEC_POINT, "EC_POINT is not initialized"); } while (0)
#define GetECPointGroup(obj, group) do { VALUE _group = rb_attr_get(obj, id_i_group); GetECGroup(_group, group); } while (0)
VALUE cEC;
VALUE eECError;
VALUE cEC_GROUP;
VALUE eEC_GROUP;
VALUE cEC_POINT;
VALUE eEC_POINT;
static ID s_GFp;
static ID s_GFp_simple;
static ID s_GFp_mont;
static ID s_GFp_nist;
static ID s_GF2m;
static ID s_GF2m_simple;
static ID ID_uncompressed;
static ID ID_compressed;
static ID ID_hybrid;
static ID id_i_group;
static VALUE ec_group_new(const EC_GROUP *group);
static VALUE ec_point_new(const EC_POINT *point, const EC_GROUP *group);
static VALUE ec_instance(VALUE klass, EC_KEY *ec)
{
EVP_PKEY *pkey;
VALUE obj;
if (!ec) {
return Qfalse;
}
obj = NewPKey(klass);
if (!(pkey = EVP_PKEY_new())) {
return Qfalse;
}
if (!EVP_PKEY_assign_EC_KEY(pkey, ec)) {
EVP_PKEY_free(pkey);
return Qfalse;
}
SetPKey(obj, pkey);
return obj;
}
VALUE ossl_ec_new(EVP_PKEY *pkey)
{
VALUE obj;
if (!pkey) {
obj = ec_instance(cEC, EC_KEY_new());
} else {
obj = NewPKey(cEC);
if (EVP_PKEY_base_id(pkey) != EVP_PKEY_EC) {
ossl_raise(rb_eTypeError, "Not a EC key!");
}
SetPKey(obj, pkey);
}
if (obj == Qfalse) {
ossl_raise(eECError, NULL);
}
return obj;
}
static EC_KEY *
ec_key_new_from_group(VALUE arg)
{
EC_KEY *ec;
if (rb_obj_is_kind_of(arg, cEC_GROUP)) {
EC_GROUP *group;
GetECGroup(arg, group);
if (!(ec = EC_KEY_new()))
ossl_raise(eECError, NULL);
if (!EC_KEY_set_group(ec, group)) {
EC_KEY_free(ec);
ossl_raise(eECError, NULL);
}
} else {
int nid = OBJ_sn2nid(StringValueCStr(arg));
if (nid == NID_undef)
ossl_raise(eECError, "invalid curve name");
if (!(ec = EC_KEY_new_by_curve_name(nid)))
ossl_raise(eECError, NULL);
EC_KEY_set_asn1_flag(ec, OPENSSL_EC_NAMED_CURVE);
EC_KEY_set_conv_form(ec, POINT_CONVERSION_UNCOMPRESSED);
}
return ec;
}
static VALUE
ossl_ec_key_s_generate(VALUE klass, VALUE arg)
{
EC_KEY *ec;
VALUE obj;
ec = ec_key_new_from_group(arg);
obj = ec_instance(klass, ec);
if (obj == Qfalse) {
EC_KEY_free(ec);
ossl_raise(eECError, NULL);
}
if (!EC_KEY_generate_key(ec))
ossl_raise(eECError, "EC_KEY_generate_key");
return obj;
}
static VALUE ossl_ec_key_initialize(int argc, VALUE *argv, VALUE self)
{
EVP_PKEY *pkey;
EC_KEY *ec;
VALUE arg, pass;
GetPKey(self, pkey);
if (EVP_PKEY_base_id(pkey) != EVP_PKEY_NONE)
ossl_raise(eECError, "EC_KEY already initialized");
rb_scan_args(argc, argv, "02", &arg, &pass);
if (NIL_P(arg)) {
if (!(ec = EC_KEY_new()))
ossl_raise(eECError, NULL);
} else if (rb_obj_is_kind_of(arg, cEC)) {
EC_KEY *other_ec = NULL;
GetEC(arg, other_ec);
if (!(ec = EC_KEY_dup(other_ec)))
ossl_raise(eECError, NULL);
} else if (rb_obj_is_kind_of(arg, cEC_GROUP)) {
ec = ec_key_new_from_group(arg);
} else {
BIO *in;
pass = ossl_pem_passwd_value(pass);
in = ossl_obj2bio(&arg);
ec = PEM_read_bio_ECPrivateKey(in, NULL, ossl_pem_passwd_cb, (void *)pass);
if (!ec) {
OSSL_BIO_reset(in);
ec = PEM_read_bio_EC_PUBKEY(in, NULL, ossl_pem_passwd_cb, (void *)pass);
}
if (!ec) {
OSSL_BIO_reset(in);
ec = d2i_ECPrivateKey_bio(in, NULL);
}
if (!ec) {
OSSL_BIO_reset(in);
ec = d2i_EC_PUBKEY_bio(in, NULL);
}
BIO_free(in);
if (!ec) {
ossl_clear_error();
ec = ec_key_new_from_group(arg);
}
}
if (!EVP_PKEY_assign_EC_KEY(pkey, ec)) {
EC_KEY_free(ec);
ossl_raise(eECError, "EVP_PKEY_assign_EC_KEY");
}
return self;
}
static VALUE
ossl_ec_key_initialize_copy(VALUE self, VALUE other)
{
EVP_PKEY *pkey;
EC_KEY *ec, *ec_new;
GetPKey(self, pkey);
if (EVP_PKEY_base_id(pkey) != EVP_PKEY_NONE)
ossl_raise(eECError, "EC already initialized");
GetEC(other, ec);
ec_new = EC_KEY_dup(ec);
if (!ec_new)
ossl_raise(eECError, "EC_KEY_dup");
if (!EVP_PKEY_assign_EC_KEY(pkey, ec_new)) {
EC_KEY_free(ec_new);
ossl_raise(eECError, "EVP_PKEY_assign_EC_KEY");
}
return self;
}
static VALUE
ossl_ec_key_get_group(VALUE self)
{
EC_KEY *ec;
const EC_GROUP *group;
GetEC(self, ec);
group = EC_KEY_get0_group(ec);
if (!group)
return Qnil;
return ec_group_new(group);
}
static VALUE
ossl_ec_key_set_group(VALUE self, VALUE group_v)
{
EC_KEY *ec;
EC_GROUP *group;
GetEC(self, ec);
GetECGroup(group_v, group);
if (EC_KEY_set_group(ec, group) != 1)
ossl_raise(eECError, "EC_KEY_set_group");
return group_v;
}
static VALUE ossl_ec_key_get_private_key(VALUE self)
{
EC_KEY *ec;
const BIGNUM *bn;
GetEC(self, ec);
if ((bn = EC_KEY_get0_private_key(ec)) == NULL)
return Qnil;
return ossl_bn_new(bn);
}
static VALUE ossl_ec_key_set_private_key(VALUE self, VALUE private_key)
{
EC_KEY *ec;
BIGNUM *bn = NULL;
GetEC(self, ec);
if (!NIL_P(private_key))
bn = GetBNPtr(private_key);
switch (EC_KEY_set_private_key(ec, bn)) {
case 1:
break;
case 0:
if (bn == NULL)
break;
default:
ossl_raise(eECError, "EC_KEY_set_private_key");
}
return private_key;
}
static VALUE ossl_ec_key_get_public_key(VALUE self)
{
EC_KEY *ec;
const EC_POINT *point;
GetEC(self, ec);
if ((point = EC_KEY_get0_public_key(ec)) == NULL)
return Qnil;
return ec_point_new(point, EC_KEY_get0_group(ec));
}
static VALUE ossl_ec_key_set_public_key(VALUE self, VALUE public_key)
{
EC_KEY *ec;
EC_POINT *point = NULL;
GetEC(self, ec);
if (!NIL_P(public_key))
GetECPoint(public_key, point);
switch (EC_KEY_set_public_key(ec, point)) {
case 1:
break;
case 0:
if (point == NULL)
break;
default:
ossl_raise(eECError, "EC_KEY_set_public_key");
}
return public_key;
}
static VALUE ossl_ec_key_is_public(VALUE self)
{
EC_KEY *ec;
GetEC(self, ec);
return EC_KEY_get0_public_key(ec) ? Qtrue : Qfalse;
}
static VALUE ossl_ec_key_is_private(VALUE self)
{
EC_KEY *ec;
GetEC(self, ec);
return EC_KEY_get0_private_key(ec) ? Qtrue : Qfalse;
}
static VALUE ossl_ec_key_to_string(VALUE self, VALUE ciph, VALUE pass, int format)
{
EC_KEY *ec;
BIO *out;
int i = -1;
int private = 0;
VALUE str;
const EVP_CIPHER *cipher = NULL;
GetEC(self, ec);
if (EC_KEY_get0_public_key(ec) == NULL)
ossl_raise(eECError, "can't export - no public key set");
if (EC_KEY_check_key(ec) != 1)
ossl_raise(eECError, "can't export - EC_KEY_check_key failed");
if (EC_KEY_get0_private_key(ec))
private = 1;
if (!NIL_P(ciph)) {
cipher = ossl_evp_get_cipherbyname(ciph);
pass = ossl_pem_passwd_value(pass);
}
if (!(out = BIO_new(BIO_s_mem())))
ossl_raise(eECError, "BIO_new(BIO_s_mem())");
switch(format) {
case EXPORT_PEM:
if (private) {
i = PEM_write_bio_ECPrivateKey(out, ec, cipher, NULL, 0, ossl_pem_passwd_cb, (void *)pass);
} else {
i = PEM_write_bio_EC_PUBKEY(out, ec);
}
break;
case EXPORT_DER:
if (private) {
i = i2d_ECPrivateKey_bio(out, ec);
} else {
i = i2d_EC_PUBKEY_bio(out, ec);
}
break;
default:
BIO_free(out);
ossl_raise(rb_eRuntimeError, "unknown format (internal error)");
}
if (i != 1) {
BIO_free(out);
ossl_raise(eECError, "outlen=%d", i);
}
str = ossl_membio2str(out);
return str;
}
static VALUE ossl_ec_key_export(int argc, VALUE *argv, VALUE self)
{
VALUE cipher, passwd;
rb_scan_args(argc, argv, "02", &cipher, &passwd);
return ossl_ec_key_to_string(self, cipher, passwd, EXPORT_PEM);
}
static VALUE ossl_ec_key_to_der(VALUE self)
{
return ossl_ec_key_to_string(self, Qnil, Qnil, EXPORT_DER);
}
static VALUE ossl_ec_key_to_text(VALUE self)
{
EC_KEY *ec;
BIO *out;
VALUE str;
GetEC(self, ec);
if (!(out = BIO_new(BIO_s_mem()))) {
ossl_raise(eECError, "BIO_new(BIO_s_mem())");
}
if (!EC_KEY_print(out, ec, 0)) {
BIO_free(out);
ossl_raise(eECError, "EC_KEY_print");
}
str = ossl_membio2str(out);
return str;
}
static VALUE ossl_ec_key_generate_key(VALUE self)
{
EC_KEY *ec;
GetEC(self, ec);
if (EC_KEY_generate_key(ec) != 1)
ossl_raise(eECError, "EC_KEY_generate_key");
return self;
}
static VALUE ossl_ec_key_check_key(VALUE self)
{
EC_KEY *ec;
GetEC(self, ec);
if (EC_KEY_check_key(ec) != 1)
ossl_raise(eECError, "EC_KEY_check_key");
return Qtrue;
}
static VALUE ossl_ec_key_dh_compute_key(VALUE self, VALUE pubkey)
{
EC_KEY *ec;
EC_POINT *point;
int buf_len;
VALUE str;
GetEC(self, ec);
GetECPoint(pubkey, point);
buf_len = 1024;
str = rb_str_new(0, buf_len);
buf_len = ECDH_compute_key(RSTRING_PTR(str), buf_len, point, ec, NULL);
if (buf_len < 0)
ossl_raise(eECError, "ECDH_compute_key");
rb_str_resize(str, buf_len);
return str;
}
static VALUE ossl_ec_key_dsa_sign_asn1(VALUE self, VALUE data)
{
EC_KEY *ec;
unsigned int buf_len;
VALUE str;
GetEC(self, ec);
StringValue(data);
if (EC_KEY_get0_private_key(ec) == NULL)
ossl_raise(eECError, "Private EC key needed!");
str = rb_str_new(0, ECDSA_size(ec));
if (ECDSA_sign(0, (unsigned char *) RSTRING_PTR(data), RSTRING_LENINT(data), (unsigned char *) RSTRING_PTR(str), &buf_len, ec) != 1)
ossl_raise(eECError, "ECDSA_sign");
rb_str_set_len(str, buf_len);
return str;
}
static VALUE ossl_ec_key_dsa_verify_asn1(VALUE self, VALUE data, VALUE sig)
{
EC_KEY *ec;
GetEC(self, ec);
StringValue(data);
StringValue(sig);
switch (ECDSA_verify(0, (unsigned char *) RSTRING_PTR(data), RSTRING_LENINT(data), (unsigned char *) RSTRING_PTR(sig), (int)RSTRING_LEN(sig), ec)) {
case 1: return Qtrue;
case 0: return Qfalse;
default: break;
}
ossl_raise(eECError, "ECDSA_verify");
UNREACHABLE;
}
static void
ossl_ec_group_free(void *ptr)
{
EC_GROUP_clear_free(ptr);
}
static const rb_data_type_t ossl_ec_group_type = {
"OpenSSL/ec_group",
{
0, ossl_ec_group_free,
},
0, 0, RUBY_TYPED_FREE_IMMEDIATELY,
};
static VALUE
ossl_ec_group_alloc(VALUE klass)
{
return TypedData_Wrap_Struct(klass, &ossl_ec_group_type, NULL);
}
static VALUE
ec_group_new(const EC_GROUP *group)
{
VALUE obj;
EC_GROUP *group_new;
obj = ossl_ec_group_alloc(cEC_GROUP);
group_new = EC_GROUP_dup(group);
if (!group_new)
ossl_raise(eEC_GROUP, "EC_GROUP_dup");
RTYPEDDATA_DATA(obj) = group_new;
return obj;
}
static VALUE ossl_ec_group_initialize(int argc, VALUE *argv, VALUE self)
{
VALUE arg1, arg2, arg3, arg4;
EC_GROUP *group;
TypedData_Get_Struct(self, EC_GROUP, &ossl_ec_group_type, group);
if (group)
ossl_raise(rb_eRuntimeError, "EC_GROUP is already initialized");
switch (rb_scan_args(argc, argv, "13", &arg1, &arg2, &arg3, &arg4)) {
case 1:
if (SYMBOL_P(arg1)) {
const EC_METHOD *method = NULL;
ID id = SYM2ID(arg1);
if (id == s_GFp_simple) {
method = EC_GFp_simple_method();
} else if (id == s_GFp_mont) {
method = EC_GFp_mont_method();
} else if (id == s_GFp_nist) {
method = EC_GFp_nist_method();
#if !defined(OPENSSL_NO_EC2M)
} else if (id == s_GF2m_simple) {
method = EC_GF2m_simple_method();
#endif
}
if (method) {
if ((group = EC_GROUP_new(method)) == NULL)
ossl_raise(eEC_GROUP, "EC_GROUP_new");
} else {
ossl_raise(rb_eArgError, "unknown symbol, must be :GFp_simple, :GFp_mont, :GFp_nist or :GF2m_simple");
}
} else if (rb_obj_is_kind_of(arg1, cEC_GROUP)) {
const EC_GROUP *arg1_group;
GetECGroup(arg1, arg1_group);
if ((group = EC_GROUP_dup(arg1_group)) == NULL)
ossl_raise(eEC_GROUP, "EC_GROUP_dup");
} else {
BIO *in = ossl_obj2bio(&arg1);
group = PEM_read_bio_ECPKParameters(in, NULL, NULL, NULL);
if (!group) {
OSSL_BIO_reset(in);
group = d2i_ECPKParameters_bio(in, NULL);
}
BIO_free(in);
if (!group) {
const char *name = StringValueCStr(arg1);
int nid = OBJ_sn2nid(name);
ossl_clear_error(); 
if (nid == NID_undef)
ossl_raise(eEC_GROUP, "unknown curve name (%"PRIsVALUE")", arg1);
group = EC_GROUP_new_by_curve_name(nid);
if (group == NULL)
ossl_raise(eEC_GROUP, "unable to create curve (%"PRIsVALUE")", arg1);
EC_GROUP_set_asn1_flag(group, OPENSSL_EC_NAMED_CURVE);
EC_GROUP_set_point_conversion_form(group, POINT_CONVERSION_UNCOMPRESSED);
}
}
break;
case 4:
if (SYMBOL_P(arg1)) {
ID id = SYM2ID(arg1);
EC_GROUP *(*new_curve)(const BIGNUM *, const BIGNUM *, const BIGNUM *, BN_CTX *) = NULL;
const BIGNUM *p = GetBNPtr(arg2);
const BIGNUM *a = GetBNPtr(arg3);
const BIGNUM *b = GetBNPtr(arg4);
if (id == s_GFp) {
new_curve = EC_GROUP_new_curve_GFp;
#if !defined(OPENSSL_NO_EC2M)
} else if (id == s_GF2m) {
new_curve = EC_GROUP_new_curve_GF2m;
#endif
} else {
ossl_raise(rb_eArgError, "unknown symbol, must be :GFp or :GF2m");
}
if ((group = new_curve(p, a, b, ossl_bn_ctx)) == NULL)
ossl_raise(eEC_GROUP, "EC_GROUP_new_by_GF*");
} else {
ossl_raise(rb_eArgError, "unknown argument, must be :GFp or :GF2m");
}
break;
default:
ossl_raise(rb_eArgError, "wrong number of arguments");
}
if (group == NULL)
ossl_raise(eEC_GROUP, "");
RTYPEDDATA_DATA(self) = group;
return self;
}
static VALUE
ossl_ec_group_initialize_copy(VALUE self, VALUE other)
{
EC_GROUP *group, *group_new;
TypedData_Get_Struct(self, EC_GROUP, &ossl_ec_group_type, group_new);
if (group_new)
ossl_raise(eEC_GROUP, "EC::Group already initialized");
GetECGroup(other, group);
group_new = EC_GROUP_dup(group);
if (!group_new)
ossl_raise(eEC_GROUP, "EC_GROUP_dup");
RTYPEDDATA_DATA(self) = group_new;
return self;
}
static VALUE ossl_ec_group_eql(VALUE a, VALUE b)
{
EC_GROUP *group1 = NULL, *group2 = NULL;
GetECGroup(a, group1);
GetECGroup(b, group2);
if (EC_GROUP_cmp(group1, group2, ossl_bn_ctx) == 1)
return Qfalse;
return Qtrue;
}
static VALUE ossl_ec_group_get_generator(VALUE self)
{
EC_GROUP *group;
const EC_POINT *generator;
GetECGroup(self, group);
generator = EC_GROUP_get0_generator(group);
if (!generator)
return Qnil;
return ec_point_new(generator, group);
}
static VALUE ossl_ec_group_set_generator(VALUE self, VALUE generator, VALUE order, VALUE cofactor)
{
EC_GROUP *group = NULL;
const EC_POINT *point;
const BIGNUM *o, *co;
GetECGroup(self, group);
GetECPoint(generator, point);
o = GetBNPtr(order);
co = GetBNPtr(cofactor);
if (EC_GROUP_set_generator(group, point, o, co) != 1)
ossl_raise(eEC_GROUP, "EC_GROUP_set_generator");
return self;
}
static VALUE ossl_ec_group_get_order(VALUE self)
{
VALUE bn_obj;
BIGNUM *bn;
EC_GROUP *group = NULL;
GetECGroup(self, group);
bn_obj = ossl_bn_new(NULL);
bn = GetBNPtr(bn_obj);
if (EC_GROUP_get_order(group, bn, ossl_bn_ctx) != 1)
ossl_raise(eEC_GROUP, "EC_GROUP_get_order");
return bn_obj;
}
static VALUE ossl_ec_group_get_cofactor(VALUE self)
{
VALUE bn_obj;
BIGNUM *bn;
EC_GROUP *group = NULL;
GetECGroup(self, group);
bn_obj = ossl_bn_new(NULL);
bn = GetBNPtr(bn_obj);
if (EC_GROUP_get_cofactor(group, bn, ossl_bn_ctx) != 1)
ossl_raise(eEC_GROUP, "EC_GROUP_get_cofactor");
return bn_obj;
}
static VALUE ossl_ec_group_get_curve_name(VALUE self)
{
EC_GROUP *group = NULL;
int nid;
GetECGroup(self, group);
if (group == NULL)
return Qnil;
nid = EC_GROUP_get_curve_name(group);
return rb_str_new2(OBJ_nid2sn(nid));
}
static VALUE ossl_s_builtin_curves(VALUE self)
{
EC_builtin_curve *curves = NULL;
int n;
int crv_len = rb_long2int(EC_get_builtin_curves(NULL, 0));
VALUE ary, ret;
curves = ALLOCA_N(EC_builtin_curve, crv_len);
if (curves == NULL)
return Qnil;
if (!EC_get_builtin_curves(curves, crv_len))
ossl_raise(rb_eRuntimeError, "EC_get_builtin_curves");
ret = rb_ary_new2(crv_len);
for (n = 0; n < crv_len; n++) {
const char *sname = OBJ_nid2sn(curves[n].nid);
const char *comment = curves[n].comment;
ary = rb_ary_new2(2);
rb_ary_push(ary, rb_str_new2(sname));
rb_ary_push(ary, comment ? rb_str_new2(comment) : Qnil);
rb_ary_push(ret, ary);
}
return ret;
}
static VALUE ossl_ec_group_get_asn1_flag(VALUE self)
{
EC_GROUP *group = NULL;
int flag;
GetECGroup(self, group);
flag = EC_GROUP_get_asn1_flag(group);
return INT2NUM(flag);
}
static VALUE ossl_ec_group_set_asn1_flag(VALUE self, VALUE flag_v)
{
EC_GROUP *group = NULL;
GetECGroup(self, group);
EC_GROUP_set_asn1_flag(group, NUM2INT(flag_v));
return flag_v;
}
static VALUE ossl_ec_group_get_point_conversion_form(VALUE self)
{
EC_GROUP *group = NULL;
point_conversion_form_t form;
VALUE ret;
GetECGroup(self, group);
form = EC_GROUP_get_point_conversion_form(group);
switch (form) {
case POINT_CONVERSION_UNCOMPRESSED: ret = ID_uncompressed; break;
case POINT_CONVERSION_COMPRESSED: ret = ID_compressed; break;
case POINT_CONVERSION_HYBRID: ret = ID_hybrid; break;
default: ossl_raise(eEC_GROUP, "unsupported point conversion form: %d, this module should be updated", form);
}
return ID2SYM(ret);
}
static point_conversion_form_t
parse_point_conversion_form_symbol(VALUE sym)
{
ID id = SYM2ID(sym);
if (id == ID_uncompressed)
return POINT_CONVERSION_UNCOMPRESSED;
else if (id == ID_compressed)
return POINT_CONVERSION_COMPRESSED;
else if (id == ID_hybrid)
return POINT_CONVERSION_HYBRID;
else
ossl_raise(rb_eArgError, "unsupported point conversion form %+"PRIsVALUE
" (expected :compressed, :uncompressed, or :hybrid)", sym);
}
static VALUE
ossl_ec_group_set_point_conversion_form(VALUE self, VALUE form_v)
{
EC_GROUP *group;
point_conversion_form_t form;
GetECGroup(self, group);
form = parse_point_conversion_form_symbol(form_v);
EC_GROUP_set_point_conversion_form(group, form);
return form_v;
}
static VALUE ossl_ec_group_get_seed(VALUE self)
{
EC_GROUP *group = NULL;
size_t seed_len;
GetECGroup(self, group);
seed_len = EC_GROUP_get_seed_len(group);
if (seed_len == 0)
return Qnil;
return rb_str_new((const char *)EC_GROUP_get0_seed(group), seed_len);
}
static VALUE ossl_ec_group_set_seed(VALUE self, VALUE seed)
{
EC_GROUP *group = NULL;
GetECGroup(self, group);
StringValue(seed);
if (EC_GROUP_set_seed(group, (unsigned char *)RSTRING_PTR(seed), RSTRING_LEN(seed)) != (size_t)RSTRING_LEN(seed))
ossl_raise(eEC_GROUP, "EC_GROUP_set_seed");
return seed;
}
static VALUE ossl_ec_group_get_degree(VALUE self)
{
EC_GROUP *group = NULL;
GetECGroup(self, group);
return INT2NUM(EC_GROUP_get_degree(group));
}
static VALUE ossl_ec_group_to_string(VALUE self, int format)
{
EC_GROUP *group;
BIO *out;
int i = -1;
VALUE str;
GetECGroup(self, group);
if (!(out = BIO_new(BIO_s_mem())))
ossl_raise(eEC_GROUP, "BIO_new(BIO_s_mem())");
switch(format) {
case EXPORT_PEM:
i = PEM_write_bio_ECPKParameters(out, group);
break;
case EXPORT_DER:
i = i2d_ECPKParameters_bio(out, group);
break;
default:
BIO_free(out);
ossl_raise(rb_eRuntimeError, "unknown format (internal error)");
}
if (i != 1) {
BIO_free(out);
ossl_raise(eECError, NULL);
}
str = ossl_membio2str(out);
return str;
}
static VALUE ossl_ec_group_to_pem(VALUE self)
{
return ossl_ec_group_to_string(self, EXPORT_PEM);
}
static VALUE ossl_ec_group_to_der(VALUE self)
{
return ossl_ec_group_to_string(self, EXPORT_DER);
}
static VALUE ossl_ec_group_to_text(VALUE self)
{
EC_GROUP *group;
BIO *out;
VALUE str;
GetECGroup(self, group);
if (!(out = BIO_new(BIO_s_mem()))) {
ossl_raise(eEC_GROUP, "BIO_new(BIO_s_mem())");
}
if (!ECPKParameters_print(out, group, 0)) {
BIO_free(out);
ossl_raise(eEC_GROUP, NULL);
}
str = ossl_membio2str(out);
return str;
}
static void
ossl_ec_point_free(void *ptr)
{
EC_POINT_clear_free(ptr);
}
static const rb_data_type_t ossl_ec_point_type = {
"OpenSSL/EC_POINT",
{
0, ossl_ec_point_free,
},
0, 0, RUBY_TYPED_FREE_IMMEDIATELY,
};
static VALUE
ossl_ec_point_alloc(VALUE klass)
{
return TypedData_Wrap_Struct(klass, &ossl_ec_point_type, NULL);
}
static VALUE
ec_point_new(const EC_POINT *point, const EC_GROUP *group)
{
EC_POINT *point_new;
VALUE obj;
obj = ossl_ec_point_alloc(cEC_POINT);
point_new = EC_POINT_dup(point, group);
if (!point_new)
ossl_raise(eEC_POINT, "EC_POINT_dup");
RTYPEDDATA_DATA(obj) = point_new;
rb_ivar_set(obj, id_i_group, ec_group_new(group));
return obj;
}
static VALUE ossl_ec_point_initialize_copy(VALUE, VALUE);
static VALUE ossl_ec_point_initialize(int argc, VALUE *argv, VALUE self)
{
EC_POINT *point;
VALUE group_v, arg2;
const EC_GROUP *group;
TypedData_Get_Struct(self, EC_POINT, &ossl_ec_point_type, point);
if (point)
rb_raise(eEC_POINT, "EC_POINT already initialized");
rb_scan_args(argc, argv, "11", &group_v, &arg2);
if (rb_obj_is_kind_of(group_v, cEC_POINT)) {
if (argc != 1)
rb_raise(rb_eArgError, "invalid second argument");
return ossl_ec_point_initialize_copy(self, group_v);
}
GetECGroup(group_v, group);
if (argc == 1) {
point = EC_POINT_new(group);
if (!point)
ossl_raise(eEC_POINT, "EC_POINT_new");
}
else {
if (rb_obj_is_kind_of(arg2, cBN)) {
point = EC_POINT_bn2point(group, GetBNPtr(arg2), NULL, ossl_bn_ctx);
if (!point)
ossl_raise(eEC_POINT, "EC_POINT_bn2point");
}
else {
StringValue(arg2);
point = EC_POINT_new(group);
if (!point)
ossl_raise(eEC_POINT, "EC_POINT_new");
if (!EC_POINT_oct2point(group, point,
(unsigned char *)RSTRING_PTR(arg2),
RSTRING_LEN(arg2), ossl_bn_ctx)) {
EC_POINT_free(point);
ossl_raise(eEC_POINT, "EC_POINT_oct2point");
}
}
}
RTYPEDDATA_DATA(self) = point;
rb_ivar_set(self, id_i_group, group_v);
return self;
}
static VALUE
ossl_ec_point_initialize_copy(VALUE self, VALUE other)
{
EC_POINT *point, *point_new;
EC_GROUP *group;
VALUE group_v;
TypedData_Get_Struct(self, EC_POINT, &ossl_ec_point_type, point_new);
if (point_new)
ossl_raise(eEC_POINT, "EC::Point already initialized");
GetECPoint(other, point);
group_v = rb_obj_dup(rb_attr_get(other, id_i_group));
GetECGroup(group_v, group);
point_new = EC_POINT_dup(point, group);
if (!point_new)
ossl_raise(eEC_POINT, "EC_POINT_dup");
RTYPEDDATA_DATA(self) = point_new;
rb_ivar_set(self, id_i_group, group_v);
return self;
}
static VALUE ossl_ec_point_eql(VALUE a, VALUE b)
{
EC_POINT *point1, *point2;
VALUE group_v1 = rb_attr_get(a, id_i_group);
VALUE group_v2 = rb_attr_get(b, id_i_group);
const EC_GROUP *group;
if (ossl_ec_group_eql(group_v1, group_v2) == Qfalse)
return Qfalse;
GetECPoint(a, point1);
GetECPoint(b, point2);
GetECGroup(group_v1, group);
if (EC_POINT_cmp(group, point1, point2, ossl_bn_ctx) == 1)
return Qfalse;
return Qtrue;
}
static VALUE ossl_ec_point_is_at_infinity(VALUE self)
{
EC_POINT *point;
const EC_GROUP *group;
GetECPoint(self, point);
GetECPointGroup(self, group);
switch (EC_POINT_is_at_infinity(group, point)) {
case 1: return Qtrue;
case 0: return Qfalse;
default: ossl_raise(cEC_POINT, "EC_POINT_is_at_infinity");
}
UNREACHABLE;
}
static VALUE ossl_ec_point_is_on_curve(VALUE self)
{
EC_POINT *point;
const EC_GROUP *group;
GetECPoint(self, point);
GetECPointGroup(self, group);
switch (EC_POINT_is_on_curve(group, point, ossl_bn_ctx)) {
case 1: return Qtrue;
case 0: return Qfalse;
default: ossl_raise(cEC_POINT, "EC_POINT_is_on_curve");
}
UNREACHABLE;
}
static VALUE ossl_ec_point_make_affine(VALUE self)
{
EC_POINT *point;
const EC_GROUP *group;
GetECPoint(self, point);
GetECPointGroup(self, group);
if (EC_POINT_make_affine(group, point, ossl_bn_ctx) != 1)
ossl_raise(cEC_POINT, "EC_POINT_make_affine");
return self;
}
static VALUE ossl_ec_point_invert(VALUE self)
{
EC_POINT *point;
const EC_GROUP *group;
GetECPoint(self, point);
GetECPointGroup(self, group);
if (EC_POINT_invert(group, point, ossl_bn_ctx) != 1)
ossl_raise(cEC_POINT, "EC_POINT_invert");
return self;
}
static VALUE ossl_ec_point_set_to_infinity(VALUE self)
{
EC_POINT *point;
const EC_GROUP *group;
GetECPoint(self, point);
GetECPointGroup(self, group);
if (EC_POINT_set_to_infinity(group, point) != 1)
ossl_raise(cEC_POINT, "EC_POINT_set_to_infinity");
return self;
}
static VALUE
ossl_ec_point_to_octet_string(VALUE self, VALUE conversion_form)
{
EC_POINT *point;
const EC_GROUP *group;
point_conversion_form_t form;
VALUE str;
size_t len;
GetECPoint(self, point);
GetECPointGroup(self, group);
form = parse_point_conversion_form_symbol(conversion_form);
len = EC_POINT_point2oct(group, point, form, NULL, 0, ossl_bn_ctx);
if (!len)
ossl_raise(eEC_POINT, "EC_POINT_point2oct");
str = rb_str_new(NULL, (long)len);
if (!EC_POINT_point2oct(group, point, form,
(unsigned char *)RSTRING_PTR(str), len,
ossl_bn_ctx))
ossl_raise(eEC_POINT, "EC_POINT_point2oct");
return str;
}
static VALUE ossl_ec_point_add(VALUE self, VALUE other)
{
EC_POINT *point_self, *point_other, *point_result;
const EC_GROUP *group;
VALUE group_v = rb_attr_get(self, id_i_group);
VALUE result;
GetECPoint(self, point_self);
GetECPoint(other, point_other);
GetECGroup(group_v, group);
result = rb_obj_alloc(cEC_POINT);
ossl_ec_point_initialize(1, &group_v, result);
GetECPoint(result, point_result);
if (EC_POINT_add(group, point_result, point_self, point_other, ossl_bn_ctx) != 1) {
ossl_raise(eEC_POINT, "EC_POINT_add");
}
return result;
}
static VALUE ossl_ec_point_mul(int argc, VALUE *argv, VALUE self)
{
EC_POINT *point_self, *point_result;
const EC_GROUP *group;
VALUE group_v = rb_attr_get(self, id_i_group);
VALUE arg1, arg2, arg3, result;
const BIGNUM *bn_g = NULL;
GetECPoint(self, point_self);
GetECGroup(group_v, group);
result = rb_obj_alloc(cEC_POINT);
ossl_ec_point_initialize(1, &group_v, result);
GetECPoint(result, point_result);
rb_scan_args(argc, argv, "12", &arg1, &arg2, &arg3);
if (!RB_TYPE_P(arg1, T_ARRAY)) {
BIGNUM *bn = GetBNPtr(arg1);
if (!NIL_P(arg2))
bn_g = GetBNPtr(arg2);
if (EC_POINT_mul(group, point_result, bn_g, point_self, bn, ossl_bn_ctx) != 1)
ossl_raise(eEC_POINT, NULL);
} else {
long i, num;
VALUE bns_tmp, tmp_p, tmp_b;
const EC_POINT **points;
const BIGNUM **bignums;
Check_Type(arg1, T_ARRAY);
Check_Type(arg2, T_ARRAY);
if (RARRAY_LEN(arg1) != RARRAY_LEN(arg2) + 1) 
ossl_raise(rb_eArgError, "bns must be 1 longer than points; see the documentation");
num = RARRAY_LEN(arg1);
bns_tmp = rb_ary_tmp_new(num);
bignums = ALLOCV_N(const BIGNUM *, tmp_b, num);
for (i = 0; i < num; i++) {
VALUE item = RARRAY_AREF(arg1, i);
bignums[i] = GetBNPtr(item);
rb_ary_push(bns_tmp, item);
}
points = ALLOCV_N(const EC_POINT *, tmp_p, num);
points[0] = point_self; 
for (i = 0; i < num - 1; i++)
GetECPoint(RARRAY_AREF(arg2, i), points[i + 1]);
if (!NIL_P(arg3))
bn_g = GetBNPtr(arg3);
if (EC_POINTs_mul(group, point_result, bn_g, num, points, bignums, ossl_bn_ctx) != 1) {
ALLOCV_END(tmp_b);
ALLOCV_END(tmp_p);
ossl_raise(eEC_POINT, NULL);
}
ALLOCV_END(tmp_b);
ALLOCV_END(tmp_p);
}
return result;
}
void Init_ossl_ec(void)
{
#undef rb_intern
#if 0
mPKey = rb_define_module_under(mOSSL, "PKey");
cPKey = rb_define_class_under(mPKey, "PKey", rb_cObject);
eOSSLError = rb_define_class_under(mOSSL, "OpenSSLError", rb_eStandardError);
ePKeyError = rb_define_class_under(mPKey, "PKeyError", eOSSLError);
#endif
eECError = rb_define_class_under(mPKey, "ECError", ePKeyError);
cEC = rb_define_class_under(mPKey, "EC", cPKey);
cEC_GROUP = rb_define_class_under(cEC, "Group", rb_cObject);
cEC_POINT = rb_define_class_under(cEC, "Point", rb_cObject);
eEC_GROUP = rb_define_class_under(cEC_GROUP, "Error", eOSSLError);
eEC_POINT = rb_define_class_under(cEC_POINT, "Error", eOSSLError);
s_GFp = rb_intern("GFp");
s_GF2m = rb_intern("GF2m");
s_GFp_simple = rb_intern("GFp_simple");
s_GFp_mont = rb_intern("GFp_mont");
s_GFp_nist = rb_intern("GFp_nist");
s_GF2m_simple = rb_intern("GF2m_simple");
ID_uncompressed = rb_intern("uncompressed");
ID_compressed = rb_intern("compressed");
ID_hybrid = rb_intern("hybrid");
rb_define_const(cEC, "NAMED_CURVE", INT2NUM(OPENSSL_EC_NAMED_CURVE));
#if defined(OPENSSL_EC_EXPLICIT_CURVE)
rb_define_const(cEC, "EXPLICIT_CURVE", INT2NUM(OPENSSL_EC_EXPLICIT_CURVE));
#endif
rb_define_singleton_method(cEC, "builtin_curves", ossl_s_builtin_curves, 0);
rb_define_singleton_method(cEC, "generate", ossl_ec_key_s_generate, 1);
rb_define_method(cEC, "initialize", ossl_ec_key_initialize, -1);
rb_define_method(cEC, "initialize_copy", ossl_ec_key_initialize_copy, 1);
rb_define_method(cEC, "group", ossl_ec_key_get_group, 0);
rb_define_method(cEC, "group=", ossl_ec_key_set_group, 1);
rb_define_method(cEC, "private_key", ossl_ec_key_get_private_key, 0);
rb_define_method(cEC, "private_key=", ossl_ec_key_set_private_key, 1);
rb_define_method(cEC, "public_key", ossl_ec_key_get_public_key, 0);
rb_define_method(cEC, "public_key=", ossl_ec_key_set_public_key, 1);
rb_define_method(cEC, "private?", ossl_ec_key_is_private, 0);
rb_define_method(cEC, "public?", ossl_ec_key_is_public, 0);
rb_define_alias(cEC, "private_key?", "private?");
rb_define_alias(cEC, "public_key?", "public?");
rb_define_method(cEC, "generate_key!", ossl_ec_key_generate_key, 0);
rb_define_alias(cEC, "generate_key", "generate_key!");
rb_define_method(cEC, "check_key", ossl_ec_key_check_key, 0);
rb_define_method(cEC, "dh_compute_key", ossl_ec_key_dh_compute_key, 1);
rb_define_method(cEC, "dsa_sign_asn1", ossl_ec_key_dsa_sign_asn1, 1);
rb_define_method(cEC, "dsa_verify_asn1", ossl_ec_key_dsa_verify_asn1, 2);
rb_define_method(cEC, "export", ossl_ec_key_export, -1);
rb_define_alias(cEC, "to_pem", "export");
rb_define_method(cEC, "to_der", ossl_ec_key_to_der, 0);
rb_define_method(cEC, "to_text", ossl_ec_key_to_text, 0);
rb_define_alloc_func(cEC_GROUP, ossl_ec_group_alloc);
rb_define_method(cEC_GROUP, "initialize", ossl_ec_group_initialize, -1);
rb_define_method(cEC_GROUP, "initialize_copy", ossl_ec_group_initialize_copy, 1);
rb_define_method(cEC_GROUP, "eql?", ossl_ec_group_eql, 1);
rb_define_alias(cEC_GROUP, "==", "eql?");
rb_define_method(cEC_GROUP, "generator", ossl_ec_group_get_generator, 0);
rb_define_method(cEC_GROUP, "set_generator", ossl_ec_group_set_generator, 3);
rb_define_method(cEC_GROUP, "order", ossl_ec_group_get_order, 0);
rb_define_method(cEC_GROUP, "cofactor", ossl_ec_group_get_cofactor, 0);
rb_define_method(cEC_GROUP, "curve_name", ossl_ec_group_get_curve_name, 0);
rb_define_method(cEC_GROUP, "asn1_flag", ossl_ec_group_get_asn1_flag, 0);
rb_define_method(cEC_GROUP, "asn1_flag=", ossl_ec_group_set_asn1_flag, 1);
rb_define_method(cEC_GROUP, "point_conversion_form", ossl_ec_group_get_point_conversion_form, 0);
rb_define_method(cEC_GROUP, "point_conversion_form=", ossl_ec_group_set_point_conversion_form, 1);
rb_define_method(cEC_GROUP, "seed", ossl_ec_group_get_seed, 0);
rb_define_method(cEC_GROUP, "seed=", ossl_ec_group_set_seed, 1);
rb_define_method(cEC_GROUP, "degree", ossl_ec_group_get_degree, 0);
rb_define_method(cEC_GROUP, "to_pem", ossl_ec_group_to_pem, 0);
rb_define_method(cEC_GROUP, "to_der", ossl_ec_group_to_der, 0);
rb_define_method(cEC_GROUP, "to_text", ossl_ec_group_to_text, 0);
rb_define_alloc_func(cEC_POINT, ossl_ec_point_alloc);
rb_define_method(cEC_POINT, "initialize", ossl_ec_point_initialize, -1);
rb_define_method(cEC_POINT, "initialize_copy", ossl_ec_point_initialize_copy, 1);
rb_attr(cEC_POINT, rb_intern("group"), 1, 0, 0);
rb_define_method(cEC_POINT, "eql?", ossl_ec_point_eql, 1);
rb_define_alias(cEC_POINT, "==", "eql?");
rb_define_method(cEC_POINT, "infinity?", ossl_ec_point_is_at_infinity, 0);
rb_define_method(cEC_POINT, "on_curve?", ossl_ec_point_is_on_curve, 0);
rb_define_method(cEC_POINT, "make_affine!", ossl_ec_point_make_affine, 0);
rb_define_method(cEC_POINT, "invert!", ossl_ec_point_invert, 0);
rb_define_method(cEC_POINT, "set_to_infinity!", ossl_ec_point_set_to_infinity, 0);
rb_define_method(cEC_POINT, "to_octet_string", ossl_ec_point_to_octet_string, 1);
rb_define_method(cEC_POINT, "add", ossl_ec_point_add, 1);
rb_define_method(cEC_POINT, "mul", ossl_ec_point_mul, -1);
id_i_group = rb_intern("@group");
}
#else 
void Init_ossl_ec(void)
{
}
#endif 
