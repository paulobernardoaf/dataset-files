








#include "ossl.h"

#define NewX509(klass) TypedData_Wrap_Struct((klass), &ossl_x509_type, 0)

#define SetX509(obj, x509) do { if (!(x509)) { ossl_raise(rb_eRuntimeError, "CERT wasn't initialized!"); } RTYPEDDATA_DATA(obj) = (x509); } while (0)





#define GetX509(obj, x509) do { TypedData_Get_Struct((obj), X509, &ossl_x509_type, (x509)); if (!(x509)) { ossl_raise(rb_eRuntimeError, "CERT wasn't initialized!"); } } while (0)









VALUE cX509Cert;
VALUE eX509CertError;

static void
ossl_x509_free(void *ptr)
{
X509_free(ptr);
}

static const rb_data_type_t ossl_x509_type = {
"OpenSSL/X509",
{
0, ossl_x509_free,
},
0, 0, RUBY_TYPED_FREE_IMMEDIATELY,
};




VALUE
ossl_x509_new(X509 *x509)
{
X509 *new;
VALUE obj;

obj = NewX509(cX509Cert);
if (!x509) {
new = X509_new();
} else {
new = X509_dup(x509);
}
if (!new) {
ossl_raise(eX509CertError, NULL);
}
SetX509(obj, new);

return obj;
}

X509 *
GetX509CertPtr(VALUE obj)
{
X509 *x509;

GetX509(obj, x509);

return x509;
}

X509 *
DupX509CertPtr(VALUE obj)
{
X509 *x509;

GetX509(obj, x509);

X509_up_ref(x509);

return x509;
}




static VALUE
ossl_x509_alloc(VALUE klass)
{
X509 *x509;
VALUE obj;

obj = NewX509(klass);
x509 = X509_new();
if (!x509) ossl_raise(eX509CertError, NULL);
SetX509(obj, x509);

return obj;
}






static VALUE
ossl_x509_initialize(int argc, VALUE *argv, VALUE self)
{
BIO *in;
X509 *x509, *x = DATA_PTR(self);
VALUE arg;

if (rb_scan_args(argc, argv, "01", &arg) == 0) {

return self;
}
arg = ossl_to_der_if_possible(arg);
in = ossl_obj2bio(&arg);
x509 = PEM_read_bio_X509(in, &x, NULL, NULL);
DATA_PTR(self) = x;
if (!x509) {
OSSL_BIO_reset(in);
x509 = d2i_X509_bio(in, &x);
DATA_PTR(self) = x;
}
BIO_free(in);
if (!x509) ossl_raise(eX509CertError, NULL);

return self;
}

static VALUE
ossl_x509_copy(VALUE self, VALUE other)
{
X509 *a, *b, *x509;

rb_check_frozen(self);
if (self == other) return self;

GetX509(self, a);
GetX509(other, b);

x509 = X509_dup(b);
if (!x509) ossl_raise(eX509CertError, NULL);

DATA_PTR(self) = x509;
X509_free(a);

return self;
}





static VALUE
ossl_x509_to_der(VALUE self)
{
X509 *x509;
VALUE str;
long len;
unsigned char *p;

GetX509(self, x509);
if ((len = i2d_X509(x509, NULL)) <= 0)
ossl_raise(eX509CertError, NULL);
str = rb_str_new(0, len);
p = (unsigned char *)RSTRING_PTR(str);
if (i2d_X509(x509, &p) <= 0)
ossl_raise(eX509CertError, NULL);
ossl_str_adjust(str, p);

return str;
}





static VALUE
ossl_x509_to_pem(VALUE self)
{
X509 *x509;
BIO *out;
VALUE str;

GetX509(self, x509);
out = BIO_new(BIO_s_mem());
if (!out) ossl_raise(eX509CertError, NULL);

if (!PEM_write_bio_X509(out, x509)) {
BIO_free(out);
ossl_raise(eX509CertError, NULL);
}
str = ossl_membio2str(out);

return str;
}





static VALUE
ossl_x509_to_text(VALUE self)
{
X509 *x509;
BIO *out;
VALUE str;

GetX509(self, x509);

out = BIO_new(BIO_s_mem());
if (!out) ossl_raise(eX509CertError, NULL);

if (!X509_print(out, x509)) {
BIO_free(out);
ossl_raise(eX509CertError, NULL);
}
str = ossl_membio2str(out);

return str;
}

#if 0



static VALUE
ossl_x509_to_req(VALUE self)
{
X509 *x509;
X509_REQ *req;
VALUE obj;

GetX509(self, x509);
if (!(req = X509_to_X509_REQ(x509, NULL, EVP_md5()))) {
ossl_raise(eX509CertError, NULL);
}
obj = ossl_x509req_new(req);
X509_REQ_free(req);

return obj;
}
#endif





static VALUE
ossl_x509_get_version(VALUE self)
{
X509 *x509;

GetX509(self, x509);

return LONG2NUM(X509_get_version(x509));
}





static VALUE
ossl_x509_set_version(VALUE self, VALUE version)
{
X509 *x509;
long ver;

if ((ver = NUM2LONG(version)) < 0) {
ossl_raise(eX509CertError, "version must be >= 0!");
}
GetX509(self, x509);
if (!X509_set_version(x509, ver)) {
ossl_raise(eX509CertError, NULL);
}

return version;
}





static VALUE
ossl_x509_get_serial(VALUE self)
{
X509 *x509;

GetX509(self, x509);

return asn1integer_to_num(X509_get_serialNumber(x509));
}





static VALUE
ossl_x509_set_serial(VALUE self, VALUE num)
{
X509 *x509;

GetX509(self, x509);
X509_set_serialNumber(x509, num_to_asn1integer(num, X509_get_serialNumber(x509)));

return num;
}





static VALUE
ossl_x509_get_signature_algorithm(VALUE self)
{
X509 *x509;
BIO *out;
VALUE str;

GetX509(self, x509);
out = BIO_new(BIO_s_mem());
if (!out) ossl_raise(eX509CertError, NULL);

if (!i2a_ASN1_OBJECT(out, X509_get0_tbs_sigalg(x509)->algorithm)) {
BIO_free(out);
ossl_raise(eX509CertError, NULL);
}
str = ossl_membio2str(out);

return str;
}





static VALUE
ossl_x509_get_subject(VALUE self)
{
X509 *x509;
X509_NAME *name;

GetX509(self, x509);
if (!(name = X509_get_subject_name(x509))) { 
ossl_raise(eX509CertError, NULL);
}

return ossl_x509name_new(name);
}





static VALUE
ossl_x509_set_subject(VALUE self, VALUE subject)
{
X509 *x509;

GetX509(self, x509);
if (!X509_set_subject_name(x509, GetX509NamePtr(subject))) { 
ossl_raise(eX509CertError, NULL);
}

return subject;
}





static VALUE
ossl_x509_get_issuer(VALUE self)
{
X509 *x509;
X509_NAME *name;

GetX509(self, x509);
if(!(name = X509_get_issuer_name(x509))) { 
ossl_raise(eX509CertError, NULL);
}

return ossl_x509name_new(name);
}





static VALUE
ossl_x509_set_issuer(VALUE self, VALUE issuer)
{
X509 *x509;

GetX509(self, x509);
if (!X509_set_issuer_name(x509, GetX509NamePtr(issuer))) { 
ossl_raise(eX509CertError, NULL);
}

return issuer;
}





static VALUE
ossl_x509_get_not_before(VALUE self)
{
X509 *x509;
const ASN1_TIME *asn1time;

GetX509(self, x509);
if (!(asn1time = X509_get0_notBefore(x509))) {
ossl_raise(eX509CertError, NULL);
}

return asn1time_to_time(asn1time);
}





static VALUE
ossl_x509_set_not_before(VALUE self, VALUE time)
{
X509 *x509;
ASN1_TIME *asn1time;

GetX509(self, x509);
asn1time = ossl_x509_time_adjust(NULL, time);
if (!X509_set1_notBefore(x509, asn1time)) {
ASN1_TIME_free(asn1time);
ossl_raise(eX509CertError, "X509_set_notBefore");
}
ASN1_TIME_free(asn1time);

return time;
}





static VALUE
ossl_x509_get_not_after(VALUE self)
{
X509 *x509;
const ASN1_TIME *asn1time;

GetX509(self, x509);
if (!(asn1time = X509_get0_notAfter(x509))) {
ossl_raise(eX509CertError, NULL);
}

return asn1time_to_time(asn1time);
}





static VALUE
ossl_x509_set_not_after(VALUE self, VALUE time)
{
X509 *x509;
ASN1_TIME *asn1time;

GetX509(self, x509);
asn1time = ossl_x509_time_adjust(NULL, time);
if (!X509_set1_notAfter(x509, asn1time)) {
ASN1_TIME_free(asn1time);
ossl_raise(eX509CertError, "X509_set_notAfter");
}
ASN1_TIME_free(asn1time);

return time;
}





static VALUE
ossl_x509_get_public_key(VALUE self)
{
X509 *x509;
EVP_PKEY *pkey;

GetX509(self, x509);
if (!(pkey = X509_get_pubkey(x509))) { 
ossl_raise(eX509CertError, NULL);
}

return ossl_pkey_new(pkey); 
}





static VALUE
ossl_x509_set_public_key(VALUE self, VALUE key)
{
X509 *x509;
EVP_PKEY *pkey;

GetX509(self, x509);
pkey = GetPKeyPtr(key);
ossl_pkey_check_public_key(pkey);
if (!X509_set_pubkey(x509, pkey))
ossl_raise(eX509CertError, "X509_set_pubkey");
return key;
}





static VALUE
ossl_x509_sign(VALUE self, VALUE key, VALUE digest)
{
X509 *x509;
EVP_PKEY *pkey;
const EVP_MD *md;

pkey = GetPrivPKeyPtr(key); 
md = ossl_evp_get_digestbyname(digest);
GetX509(self, x509);
if (!X509_sign(x509, pkey, md)) {
ossl_raise(eX509CertError, NULL);
}

return self;
}








static VALUE
ossl_x509_verify(VALUE self, VALUE key)
{
X509 *x509;
EVP_PKEY *pkey;

GetX509(self, x509);
pkey = GetPKeyPtr(key);
ossl_pkey_check_public_key(pkey);
switch (X509_verify(x509, pkey)) {
case 1:
return Qtrue;
case 0:
ossl_clear_error();
return Qfalse;
default:
ossl_raise(eX509CertError, NULL);
}
}








static VALUE
ossl_x509_check_private_key(VALUE self, VALUE key)
{
X509 *x509;
EVP_PKEY *pkey;


pkey = GetPrivPKeyPtr(key); 
GetX509(self, x509);
if (!X509_check_private_key(x509, pkey)) {
ossl_clear_error();
return Qfalse;
}

return Qtrue;
}





static VALUE
ossl_x509_get_extensions(VALUE self)
{
X509 *x509;
int count, i;
X509_EXTENSION *ext;
VALUE ary;

GetX509(self, x509);
count = X509_get_ext_count(x509);
if (count < 0) {
return rb_ary_new();
}
ary = rb_ary_new2(count);
for (i=0; i<count; i++) {
ext = X509_get_ext(x509, i); 
rb_ary_push(ary, ossl_x509ext_new(ext));
}

return ary;
}





static VALUE
ossl_x509_set_extensions(VALUE self, VALUE ary)
{
X509 *x509;
X509_EXTENSION *ext;
long i;

Check_Type(ary, T_ARRAY);

for (i=0; i<RARRAY_LEN(ary); i++) {
OSSL_Check_Kind(RARRAY_AREF(ary, i), cX509Ext);
}
GetX509(self, x509);
while ((ext = X509_delete_ext(x509, 0)))
X509_EXTENSION_free(ext);
for (i=0; i<RARRAY_LEN(ary); i++) {
ext = GetX509ExtPtr(RARRAY_AREF(ary, i));
if (!X509_add_ext(x509, ext, -1)) { 
ossl_raise(eX509CertError, NULL);
}
}

return ary;
}





static VALUE
ossl_x509_add_extension(VALUE self, VALUE extension)
{
X509 *x509;
X509_EXTENSION *ext;

GetX509(self, x509);
ext = GetX509ExtPtr(extension);
if (!X509_add_ext(x509, ext, -1)) { 
ossl_raise(eX509CertError, NULL);
}

return extension;
}

static VALUE
ossl_x509_inspect(VALUE self)
{
return rb_sprintf("#<%"PRIsVALUE": subject=%+"PRIsVALUE", "
"issuer=%+"PRIsVALUE", serial=%+"PRIsVALUE", "
"not_before=%+"PRIsVALUE", not_after=%+"PRIsVALUE">",
rb_obj_class(self),
ossl_x509_get_subject(self),
ossl_x509_get_issuer(self),
ossl_x509_get_serial(self),
ossl_x509_get_not_before(self),
ossl_x509_get_not_after(self));
}








static VALUE
ossl_x509_eq(VALUE self, VALUE other)
{
X509 *a, *b;

GetX509(self, a);
if (!rb_obj_is_kind_of(other, cX509Cert))
return Qfalse;
GetX509(other, b);

return !X509_cmp(a, b) ? Qtrue : Qfalse;
}




void
Init_ossl_x509cert(void)
{
#if 0
mOSSL = rb_define_module("OpenSSL");
eOSSLError = rb_define_class_under(mOSSL, "OpenSSLError", rb_eStandardError);
mX509 = rb_define_module_under(mOSSL, "X509");
#endif

eX509CertError = rb_define_class_under(mX509, "CertificateError", eOSSLError);





























































































cX509Cert = rb_define_class_under(mX509, "Certificate", rb_cObject);

rb_define_alloc_func(cX509Cert, ossl_x509_alloc);
rb_define_method(cX509Cert, "initialize", ossl_x509_initialize, -1);
rb_define_method(cX509Cert, "initialize_copy", ossl_x509_copy, 1);

rb_define_method(cX509Cert, "to_der", ossl_x509_to_der, 0);
rb_define_method(cX509Cert, "to_pem", ossl_x509_to_pem, 0);
rb_define_alias(cX509Cert, "to_s", "to_pem");
rb_define_method(cX509Cert, "to_text", ossl_x509_to_text, 0);
rb_define_method(cX509Cert, "version", ossl_x509_get_version, 0);
rb_define_method(cX509Cert, "version=", ossl_x509_set_version, 1);
rb_define_method(cX509Cert, "signature_algorithm", ossl_x509_get_signature_algorithm, 0);
rb_define_method(cX509Cert, "serial", ossl_x509_get_serial, 0);
rb_define_method(cX509Cert, "serial=", ossl_x509_set_serial, 1);
rb_define_method(cX509Cert, "subject", ossl_x509_get_subject, 0);
rb_define_method(cX509Cert, "subject=", ossl_x509_set_subject, 1);
rb_define_method(cX509Cert, "issuer", ossl_x509_get_issuer, 0);
rb_define_method(cX509Cert, "issuer=", ossl_x509_set_issuer, 1);
rb_define_method(cX509Cert, "not_before", ossl_x509_get_not_before, 0);
rb_define_method(cX509Cert, "not_before=", ossl_x509_set_not_before, 1);
rb_define_method(cX509Cert, "not_after", ossl_x509_get_not_after, 0);
rb_define_method(cX509Cert, "not_after=", ossl_x509_set_not_after, 1);
rb_define_method(cX509Cert, "public_key", ossl_x509_get_public_key, 0);
rb_define_method(cX509Cert, "public_key=", ossl_x509_set_public_key, 1);
rb_define_method(cX509Cert, "sign", ossl_x509_sign, 2);
rb_define_method(cX509Cert, "verify", ossl_x509_verify, 1);
rb_define_method(cX509Cert, "check_private_key", ossl_x509_check_private_key, 1);
rb_define_method(cX509Cert, "extensions", ossl_x509_get_extensions, 0);
rb_define_method(cX509Cert, "extensions=", ossl_x509_set_extensions, 1);
rb_define_method(cX509Cert, "add_extension", ossl_x509_add_extension, 1);
rb_define_method(cX509Cert, "inspect", ossl_x509_inspect, 0);
rb_define_method(cX509Cert, "==", ossl_x509_eq, 1);
}
