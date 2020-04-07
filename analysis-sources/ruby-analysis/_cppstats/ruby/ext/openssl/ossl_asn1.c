#include "ossl.h"
static VALUE ossl_asn1_decode0(unsigned char **pp, long length, long *offset,
int depth, int yield, long *num_read);
static VALUE ossl_asn1_initialize(int argc, VALUE *argv, VALUE self);
VALUE
asn1time_to_time(const ASN1_TIME *time)
{
struct tm tm;
VALUE argv[6];
int count;
memset(&tm, 0, sizeof(struct tm));
switch (time->type) {
case V_ASN1_UTCTIME:
count = sscanf((const char *)time->data, "%2d%2d%2d%2d%2d%2dZ",
&tm.tm_year, &tm.tm_mon, &tm.tm_mday, &tm.tm_hour, &tm.tm_min,
&tm.tm_sec);
if (count == 5) {
tm.tm_sec = 0;
} else if (count != 6) {
ossl_raise(rb_eTypeError, "bad UTCTIME format: \"%s\"",
time->data);
}
if (tm.tm_year < 69) {
tm.tm_year += 2000;
} else {
tm.tm_year += 1900;
}
break;
case V_ASN1_GENERALIZEDTIME:
count = sscanf((const char *)time->data, "%4d%2d%2d%2d%2d%2dZ",
&tm.tm_year, &tm.tm_mon, &tm.tm_mday, &tm.tm_hour, &tm.tm_min,
&tm.tm_sec);
if (count == 5) {
tm.tm_sec = 0;
}
else if (count != 6) {
ossl_raise(rb_eTypeError, "bad GENERALIZEDTIME format: \"%s\"",
time->data);
}
break;
default:
rb_warning("unknown time format");
return Qnil;
}
argv[0] = INT2NUM(tm.tm_year);
argv[1] = INT2NUM(tm.tm_mon);
argv[2] = INT2NUM(tm.tm_mday);
argv[3] = INT2NUM(tm.tm_hour);
argv[4] = INT2NUM(tm.tm_min);
argv[5] = INT2NUM(tm.tm_sec);
return rb_funcall2(rb_cTime, rb_intern("utc"), 6, argv);
}
void
ossl_time_split(VALUE time, time_t *sec, int *days)
{
VALUE num = rb_Integer(time);
if (FIXNUM_P(num)) {
time_t t = FIX2LONG(num);
*sec = t % 86400;
*days = rb_long2int(t / 86400);
}
else {
*days = NUM2INT(rb_funcall(num, rb_intern("/"), 1, INT2FIX(86400)));
*sec = NUM2TIMET(rb_funcall(num, rb_intern("%"), 1, INT2FIX(86400)));
}
}
VALUE
asn1str_to_str(const ASN1_STRING *str)
{
return rb_str_new((const char *)str->data, str->length);
}
VALUE
asn1integer_to_num(const ASN1_INTEGER *ai)
{
BIGNUM *bn;
VALUE num;
if (!ai) {
ossl_raise(rb_eTypeError, "ASN1_INTEGER is NULL!");
}
if (ai->type == V_ASN1_ENUMERATED)
bn = ASN1_ENUMERATED_to_BN((ASN1_ENUMERATED *)ai, NULL);
else
bn = ASN1_INTEGER_to_BN(ai, NULL);
if (!bn)
ossl_raise(eOSSLError, NULL);
num = ossl_bn_new(bn);
BN_free(bn);
return num;
}
ASN1_INTEGER *
num_to_asn1integer(VALUE obj, ASN1_INTEGER *ai)
{
BIGNUM *bn;
if (NIL_P(obj))
ossl_raise(rb_eTypeError, "Can't convert nil into Integer");
bn = GetBNPtr(obj);
if (!(ai = BN_to_ASN1_INTEGER(bn, ai)))
ossl_raise(eOSSLError, NULL);
return ai;
}
#define ossl_asn1_get_value(o) rb_attr_get((o),sivVALUE)
#define ossl_asn1_get_tag(o) rb_attr_get((o),sivTAG)
#define ossl_asn1_get_tagging(o) rb_attr_get((o),sivTAGGING)
#define ossl_asn1_get_tag_class(o) rb_attr_get((o),sivTAG_CLASS)
#define ossl_asn1_get_indefinite_length(o) rb_attr_get((o),sivINDEFINITE_LENGTH)
#define ossl_asn1_set_value(o,v) rb_ivar_set((o),sivVALUE,(v))
#define ossl_asn1_set_tag(o,v) rb_ivar_set((o),sivTAG,(v))
#define ossl_asn1_set_tagging(o,v) rb_ivar_set((o),sivTAGGING,(v))
#define ossl_asn1_set_tag_class(o,v) rb_ivar_set((o),sivTAG_CLASS,(v))
#define ossl_asn1_set_indefinite_length(o,v) rb_ivar_set((o),sivINDEFINITE_LENGTH,(v))
VALUE mASN1;
VALUE eASN1Error;
VALUE cASN1Data;
VALUE cASN1Primitive;
VALUE cASN1Constructive;
VALUE cASN1EndOfContent;
VALUE cASN1Boolean; 
VALUE cASN1Integer, cASN1Enumerated; 
VALUE cASN1BitString; 
VALUE cASN1OctetString, cASN1UTF8String; 
VALUE cASN1NumericString, cASN1PrintableString;
VALUE cASN1T61String, cASN1VideotexString;
VALUE cASN1IA5String, cASN1GraphicString;
VALUE cASN1ISO64String, cASN1GeneralString;
VALUE cASN1UniversalString, cASN1BMPString;
VALUE cASN1Null; 
VALUE cASN1ObjectId; 
VALUE cASN1UTCTime, cASN1GeneralizedTime; 
VALUE cASN1Sequence, cASN1Set; 
static VALUE sym_IMPLICIT, sym_EXPLICIT;
static VALUE sym_UNIVERSAL, sym_APPLICATION, sym_CONTEXT_SPECIFIC, sym_PRIVATE;
static ID sivVALUE, sivTAG, sivTAG_CLASS, sivTAGGING, sivINDEFINITE_LENGTH, sivUNUSED_BITS;
static ID id_each;
static ASN1_BOOLEAN
obj_to_asn1bool(VALUE obj)
{
if (NIL_P(obj))
ossl_raise(rb_eTypeError, "Can't convert nil into Boolean");
return RTEST(obj) ? 0xff : 0x0;
}
static ASN1_INTEGER*
obj_to_asn1int(VALUE obj)
{
return num_to_asn1integer(obj, NULL);
}
static ASN1_BIT_STRING*
obj_to_asn1bstr(VALUE obj, long unused_bits)
{
ASN1_BIT_STRING *bstr;
if (unused_bits < 0 || unused_bits > 7)
ossl_raise(eASN1Error, "unused_bits for a bitstring value must be in "\
"the range 0 to 7");
StringValue(obj);
if(!(bstr = ASN1_BIT_STRING_new()))
ossl_raise(eASN1Error, NULL);
ASN1_BIT_STRING_set(bstr, (unsigned char *)RSTRING_PTR(obj), RSTRING_LENINT(obj));
bstr->flags &= ~(ASN1_STRING_FLAG_BITS_LEFT|0x07); 
bstr->flags |= ASN1_STRING_FLAG_BITS_LEFT | unused_bits;
return bstr;
}
static ASN1_STRING*
obj_to_asn1str(VALUE obj)
{
ASN1_STRING *str;
StringValue(obj);
if(!(str = ASN1_STRING_new()))
ossl_raise(eASN1Error, NULL);
ASN1_STRING_set(str, RSTRING_PTR(obj), RSTRING_LENINT(obj));
return str;
}
static ASN1_NULL*
obj_to_asn1null(VALUE obj)
{
ASN1_NULL *null;
if(!NIL_P(obj))
ossl_raise(eASN1Error, "nil expected");
if(!(null = ASN1_NULL_new()))
ossl_raise(eASN1Error, NULL);
return null;
}
static ASN1_OBJECT*
obj_to_asn1obj(VALUE obj)
{
ASN1_OBJECT *a1obj;
StringValueCStr(obj);
a1obj = OBJ_txt2obj(RSTRING_PTR(obj), 0);
if(!a1obj) a1obj = OBJ_txt2obj(RSTRING_PTR(obj), 1);
if(!a1obj) ossl_raise(eASN1Error, "invalid OBJECT ID %"PRIsVALUE, obj);
return a1obj;
}
static ASN1_UTCTIME *
obj_to_asn1utime(VALUE time)
{
time_t sec;
ASN1_UTCTIME *t;
int off_days;
ossl_time_split(time, &sec, &off_days);
if (!(t = ASN1_UTCTIME_adj(NULL, sec, off_days, 0)))
ossl_raise(eASN1Error, NULL);
return t;
}
static ASN1_GENERALIZEDTIME *
obj_to_asn1gtime(VALUE time)
{
time_t sec;
ASN1_GENERALIZEDTIME *t;
int off_days;
ossl_time_split(time, &sec, &off_days);
if (!(t = ASN1_GENERALIZEDTIME_adj(NULL, sec, off_days, 0)))
ossl_raise(eASN1Error, NULL);
return t;
}
static ASN1_STRING*
obj_to_asn1derstr(VALUE obj)
{
ASN1_STRING *a1str;
VALUE str;
str = ossl_to_der(obj);
if(!(a1str = ASN1_STRING_new()))
ossl_raise(eASN1Error, NULL);
ASN1_STRING_set(a1str, RSTRING_PTR(str), RSTRING_LENINT(str));
return a1str;
}
static VALUE
decode_bool(unsigned char* der, long length)
{
const unsigned char *p = der;
if (length != 3)
ossl_raise(eASN1Error, "invalid length for BOOLEAN");
if (p[0] != 1 || p[1] != 1)
ossl_raise(eASN1Error, "invalid BOOLEAN");
return p[2] ? Qtrue : Qfalse;
}
static VALUE
decode_int(unsigned char* der, long length)
{
ASN1_INTEGER *ai;
const unsigned char *p;
VALUE ret;
int status = 0;
p = der;
if(!(ai = d2i_ASN1_INTEGER(NULL, &p, length)))
ossl_raise(eASN1Error, NULL);
ret = rb_protect((VALUE (*)(VALUE))asn1integer_to_num,
(VALUE)ai, &status);
ASN1_INTEGER_free(ai);
if(status) rb_jump_tag(status);
return ret;
}
static VALUE
decode_bstr(unsigned char* der, long length, long *unused_bits)
{
ASN1_BIT_STRING *bstr;
const unsigned char *p;
long len;
VALUE ret;
p = der;
if(!(bstr = d2i_ASN1_BIT_STRING(NULL, &p, length)))
ossl_raise(eASN1Error, NULL);
len = bstr->length;
*unused_bits = 0;
if(bstr->flags & ASN1_STRING_FLAG_BITS_LEFT)
*unused_bits = bstr->flags & 0x07;
ret = rb_str_new((const char *)bstr->data, len);
ASN1_BIT_STRING_free(bstr);
return ret;
}
static VALUE
decode_enum(unsigned char* der, long length)
{
ASN1_ENUMERATED *ai;
const unsigned char *p;
VALUE ret;
int status = 0;
p = der;
if(!(ai = d2i_ASN1_ENUMERATED(NULL, &p, length)))
ossl_raise(eASN1Error, NULL);
ret = rb_protect((VALUE (*)(VALUE))asn1integer_to_num,
(VALUE)ai, &status);
ASN1_ENUMERATED_free(ai);
if(status) rb_jump_tag(status);
return ret;
}
static VALUE
decode_null(unsigned char* der, long length)
{
ASN1_NULL *null;
const unsigned char *p;
p = der;
if(!(null = d2i_ASN1_NULL(NULL, &p, length)))
ossl_raise(eASN1Error, NULL);
ASN1_NULL_free(null);
return Qnil;
}
static VALUE
decode_obj(unsigned char* der, long length)
{
ASN1_OBJECT *obj;
const unsigned char *p;
VALUE ret;
int nid;
BIO *bio;
p = der;
if(!(obj = d2i_ASN1_OBJECT(NULL, &p, length)))
ossl_raise(eASN1Error, NULL);
if((nid = OBJ_obj2nid(obj)) != NID_undef){
ASN1_OBJECT_free(obj);
ret = rb_str_new2(OBJ_nid2sn(nid));
}
else{
if(!(bio = BIO_new(BIO_s_mem()))){
ASN1_OBJECT_free(obj);
ossl_raise(eASN1Error, NULL);
}
i2a_ASN1_OBJECT(bio, obj);
ASN1_OBJECT_free(obj);
ret = ossl_membio2str(bio);
}
return ret;
}
static VALUE
decode_time(unsigned char* der, long length)
{
ASN1_TIME *time;
const unsigned char *p;
VALUE ret;
int status = 0;
p = der;
if(!(time = d2i_ASN1_TIME(NULL, &p, length)))
ossl_raise(eASN1Error, NULL);
ret = rb_protect((VALUE (*)(VALUE))asn1time_to_time,
(VALUE)time, &status);
ASN1_TIME_free(time);
if(status) rb_jump_tag(status);
return ret;
}
static VALUE
decode_eoc(unsigned char *der, long length)
{
if (length != 2 || !(der[0] == 0x00 && der[1] == 0x00))
ossl_raise(eASN1Error, NULL);
return rb_str_new("", 0);
}
typedef struct {
const char *name;
VALUE *klass;
} ossl_asn1_info_t;
static const ossl_asn1_info_t ossl_asn1_info[] = {
{ "EOC", &cASN1EndOfContent, }, 
{ "BOOLEAN", &cASN1Boolean, }, 
{ "INTEGER", &cASN1Integer, }, 
{ "BIT_STRING", &cASN1BitString, }, 
{ "OCTET_STRING", &cASN1OctetString, }, 
{ "NULL", &cASN1Null, }, 
{ "OBJECT", &cASN1ObjectId, }, 
{ "OBJECT_DESCRIPTOR", NULL, }, 
{ "EXTERNAL", NULL, }, 
{ "REAL", NULL, }, 
{ "ENUMERATED", &cASN1Enumerated, }, 
{ "EMBEDDED_PDV", NULL, }, 
{ "UTF8STRING", &cASN1UTF8String, }, 
{ "RELATIVE_OID", NULL, }, 
{ "[UNIVERSAL 14]", NULL, }, 
{ "[UNIVERSAL 15]", NULL, }, 
{ "SEQUENCE", &cASN1Sequence, }, 
{ "SET", &cASN1Set, }, 
{ "NUMERICSTRING", &cASN1NumericString, }, 
{ "PRINTABLESTRING", &cASN1PrintableString, }, 
{ "T61STRING", &cASN1T61String, }, 
{ "VIDEOTEXSTRING", &cASN1VideotexString, }, 
{ "IA5STRING", &cASN1IA5String, }, 
{ "UTCTIME", &cASN1UTCTime, }, 
{ "GENERALIZEDTIME", &cASN1GeneralizedTime, }, 
{ "GRAPHICSTRING", &cASN1GraphicString, }, 
{ "ISO64STRING", &cASN1ISO64String, }, 
{ "GENERALSTRING", &cASN1GeneralString, }, 
{ "UNIVERSALSTRING", &cASN1UniversalString, }, 
{ "CHARACTER_STRING", NULL, }, 
{ "BMPSTRING", &cASN1BMPString, }, 
};
enum {ossl_asn1_info_size = (sizeof(ossl_asn1_info)/sizeof(ossl_asn1_info[0]))};
static VALUE class_tag_map;
static int ossl_asn1_default_tag(VALUE obj);
ASN1_TYPE*
ossl_asn1_get_asn1type(VALUE obj)
{
ASN1_TYPE *ret;
VALUE value, rflag;
void *ptr;
void (*free_func)();
int tag;
tag = ossl_asn1_default_tag(obj);
value = ossl_asn1_get_value(obj);
switch(tag){
case V_ASN1_BOOLEAN:
ptr = (void*)(VALUE)obj_to_asn1bool(value);
free_func = NULL;
break;
case V_ASN1_INTEGER: 
case V_ASN1_ENUMERATED:
ptr = obj_to_asn1int(value);
free_func = ASN1_INTEGER_free;
break;
case V_ASN1_BIT_STRING:
rflag = rb_attr_get(obj, sivUNUSED_BITS);
ptr = obj_to_asn1bstr(value, NUM2INT(rflag));
free_func = ASN1_BIT_STRING_free;
break;
case V_ASN1_NULL:
ptr = obj_to_asn1null(value);
free_func = ASN1_NULL_free;
break;
case V_ASN1_OCTET_STRING: 
case V_ASN1_UTF8STRING: 
case V_ASN1_NUMERICSTRING: 
case V_ASN1_PRINTABLESTRING: 
case V_ASN1_T61STRING: 
case V_ASN1_VIDEOTEXSTRING: 
case V_ASN1_IA5STRING: 
case V_ASN1_GRAPHICSTRING: 
case V_ASN1_ISO64STRING: 
case V_ASN1_GENERALSTRING: 
case V_ASN1_UNIVERSALSTRING: 
case V_ASN1_BMPSTRING:
ptr = obj_to_asn1str(value);
free_func = ASN1_STRING_free;
break;
case V_ASN1_OBJECT:
ptr = obj_to_asn1obj(value);
free_func = ASN1_OBJECT_free;
break;
case V_ASN1_UTCTIME:
ptr = obj_to_asn1utime(value);
free_func = ASN1_TIME_free;
break;
case V_ASN1_GENERALIZEDTIME:
ptr = obj_to_asn1gtime(value);
free_func = ASN1_TIME_free;
break;
case V_ASN1_SET: 
case V_ASN1_SEQUENCE:
ptr = obj_to_asn1derstr(obj);
free_func = ASN1_STRING_free;
break;
default:
ossl_raise(eASN1Error, "unsupported ASN.1 type");
}
if(!(ret = OPENSSL_malloc(sizeof(ASN1_TYPE)))){
if(free_func) free_func(ptr);
ossl_raise(eASN1Error, "ASN1_TYPE alloc failure");
}
memset(ret, 0, sizeof(ASN1_TYPE));
ASN1_TYPE_set(ret, tag, ptr);
return ret;
}
static int
ossl_asn1_default_tag(VALUE obj)
{
VALUE tmp_class, tag;
tmp_class = CLASS_OF(obj);
while (!NIL_P(tmp_class)) {
tag = rb_hash_lookup(class_tag_map, tmp_class);
if (tag != Qnil)
return NUM2INT(tag);
tmp_class = rb_class_superclass(tmp_class);
}
return -1;
}
static int
ossl_asn1_tag(VALUE obj)
{
VALUE tag;
tag = ossl_asn1_get_tag(obj);
if(NIL_P(tag))
ossl_raise(eASN1Error, "tag number not specified");
return NUM2INT(tag);
}
static int
ossl_asn1_tag_class(VALUE obj)
{
VALUE s;
s = ossl_asn1_get_tag_class(obj);
if (NIL_P(s) || s == sym_UNIVERSAL)
return V_ASN1_UNIVERSAL;
else if (s == sym_APPLICATION)
return V_ASN1_APPLICATION;
else if (s == sym_CONTEXT_SPECIFIC)
return V_ASN1_CONTEXT_SPECIFIC;
else if (s == sym_PRIVATE)
return V_ASN1_PRIVATE;
else
ossl_raise(eASN1Error, "invalid tag class");
}
static VALUE
ossl_asn1_class2sym(int tc)
{
if((tc & V_ASN1_PRIVATE) == V_ASN1_PRIVATE)
return sym_PRIVATE;
else if((tc & V_ASN1_CONTEXT_SPECIFIC) == V_ASN1_CONTEXT_SPECIFIC)
return sym_CONTEXT_SPECIFIC;
else if((tc & V_ASN1_APPLICATION) == V_ASN1_APPLICATION)
return sym_APPLICATION;
else
return sym_UNIVERSAL;
}
static VALUE
ossl_asn1data_initialize(VALUE self, VALUE value, VALUE tag, VALUE tag_class)
{
if(!SYMBOL_P(tag_class))
ossl_raise(eASN1Error, "invalid tag class");
ossl_asn1_set_tag(self, tag);
ossl_asn1_set_value(self, value);
ossl_asn1_set_tag_class(self, tag_class);
ossl_asn1_set_indefinite_length(self, Qfalse);
return self;
}
static VALUE
to_der_internal(VALUE self, int constructed, int indef_len, VALUE body)
{
int encoding = constructed ? indef_len ? 2 : 1 : 0;
int tag_class = ossl_asn1_tag_class(self);
int tag_number = ossl_asn1_tag(self);
int default_tag_number = ossl_asn1_default_tag(self);
int body_length, total_length;
VALUE str;
unsigned char *p;
body_length = RSTRING_LENINT(body);
if (ossl_asn1_get_tagging(self) == sym_EXPLICIT) {
int inner_length, e_encoding = indef_len ? 2 : 1;
if (default_tag_number == -1)
ossl_raise(eASN1Error, "explicit tagging of unknown tag");
inner_length = ASN1_object_size(encoding, body_length, default_tag_number);
total_length = ASN1_object_size(e_encoding, inner_length, tag_number);
str = rb_str_new(NULL, total_length);
p = (unsigned char *)RSTRING_PTR(str);
ASN1_put_object(&p, e_encoding, inner_length, tag_number, tag_class);
ASN1_put_object(&p, encoding, body_length, default_tag_number, V_ASN1_UNIVERSAL);
memcpy(p, RSTRING_PTR(body), body_length);
p += body_length;
if (indef_len) {
ASN1_put_eoc(&p); 
ASN1_put_eoc(&p); 
}
}
else {
total_length = ASN1_object_size(encoding, body_length, tag_number);
str = rb_str_new(NULL, total_length);
p = (unsigned char *)RSTRING_PTR(str);
ASN1_put_object(&p, encoding, body_length, tag_number, tag_class);
memcpy(p, RSTRING_PTR(body), body_length);
p += body_length;
if (indef_len)
ASN1_put_eoc(&p);
}
assert(p - (unsigned char *)RSTRING_PTR(str) == total_length);
return str;
}
static VALUE ossl_asn1prim_to_der(VALUE);
static VALUE ossl_asn1cons_to_der(VALUE);
static VALUE
ossl_asn1data_to_der(VALUE self)
{
VALUE value = ossl_asn1_get_value(self);
if (rb_obj_is_kind_of(value, rb_cArray))
return ossl_asn1cons_to_der(self);
else {
if (RTEST(ossl_asn1_get_indefinite_length(self)))
ossl_raise(eASN1Error, "indefinite length form cannot be used " \
"with primitive encoding");
return ossl_asn1prim_to_der(self);
}
}
static VALUE
int_ossl_asn1_decode0_prim(unsigned char **pp, long length, long hlen, int tag,
VALUE tc, long *num_read)
{
VALUE value, asn1data;
unsigned char *p;
long flag = 0;
p = *pp;
if(tc == sym_UNIVERSAL && tag < ossl_asn1_info_size) {
switch(tag){
case V_ASN1_EOC:
value = decode_eoc(p, hlen+length);
break;
case V_ASN1_BOOLEAN:
value = decode_bool(p, hlen+length);
break;
case V_ASN1_INTEGER:
value = decode_int(p, hlen+length);
break;
case V_ASN1_BIT_STRING:
value = decode_bstr(p, hlen+length, &flag);
break;
case V_ASN1_NULL:
value = decode_null(p, hlen+length);
break;
case V_ASN1_ENUMERATED:
value = decode_enum(p, hlen+length);
break;
case V_ASN1_OBJECT:
value = decode_obj(p, hlen+length);
break;
case V_ASN1_UTCTIME: 
case V_ASN1_GENERALIZEDTIME:
value = decode_time(p, hlen+length);
break;
default:
p += hlen;
value = rb_str_new((const char *)p, length);
break;
}
}
else {
p += hlen;
value = rb_str_new((const char *)p, length);
}
*pp += hlen + length;
*num_read = hlen + length;
if (tc == sym_UNIVERSAL &&
tag < ossl_asn1_info_size && ossl_asn1_info[tag].klass) {
VALUE klass = *ossl_asn1_info[tag].klass;
VALUE args[4];
args[0] = value;
args[1] = INT2NUM(tag);
args[2] = Qnil;
args[3] = tc;
asn1data = rb_obj_alloc(klass);
ossl_asn1_initialize(4, args, asn1data);
if(tag == V_ASN1_BIT_STRING){
rb_ivar_set(asn1data, sivUNUSED_BITS, LONG2NUM(flag));
}
}
else {
asn1data = rb_obj_alloc(cASN1Data);
ossl_asn1data_initialize(asn1data, value, INT2NUM(tag), tc);
}
return asn1data;
}
static VALUE
int_ossl_asn1_decode0_cons(unsigned char **pp, long max_len, long length,
long *offset, int depth, int yield, int j,
int tag, VALUE tc, long *num_read)
{
VALUE value, asn1data, ary;
int indefinite;
long available_len, off = *offset;
indefinite = (j == 0x21);
ary = rb_ary_new();
available_len = indefinite ? max_len : length;
while (available_len > 0) {
long inner_read = 0;
value = ossl_asn1_decode0(pp, available_len, &off, depth + 1, yield, &inner_read);
*num_read += inner_read;
available_len -= inner_read;
if (indefinite &&
ossl_asn1_tag(value) == V_ASN1_EOC &&
ossl_asn1_get_tag_class(value) == sym_UNIVERSAL) {
break;
}
rb_ary_push(ary, value);
}
if (tc == sym_UNIVERSAL) {
VALUE args[4];
if (tag == V_ASN1_SEQUENCE || tag == V_ASN1_SET)
asn1data = rb_obj_alloc(*ossl_asn1_info[tag].klass);
else
asn1data = rb_obj_alloc(cASN1Constructive);
args[0] = ary;
args[1] = INT2NUM(tag);
args[2] = Qnil;
args[3] = tc;
ossl_asn1_initialize(4, args, asn1data);
}
else {
asn1data = rb_obj_alloc(cASN1Data);
ossl_asn1data_initialize(asn1data, ary, INT2NUM(tag), tc);
}
if (indefinite)
ossl_asn1_set_indefinite_length(asn1data, Qtrue);
else
ossl_asn1_set_indefinite_length(asn1data, Qfalse);
*offset = off;
return asn1data;
}
static VALUE
ossl_asn1_decode0(unsigned char **pp, long length, long *offset, int depth,
int yield, long *num_read)
{
unsigned char *start, *p;
const unsigned char *p0;
long len = 0, inner_read = 0, off = *offset, hlen;
int tag, tc, j;
VALUE asn1data, tag_class;
p = *pp;
start = p;
p0 = p;
j = ASN1_get_object(&p0, &len, &tag, &tc, length);
p = (unsigned char *)p0;
if(j & 0x80) ossl_raise(eASN1Error, NULL);
if(len > length) ossl_raise(eASN1Error, "value is too short");
if((tc & V_ASN1_PRIVATE) == V_ASN1_PRIVATE)
tag_class = sym_PRIVATE;
else if((tc & V_ASN1_CONTEXT_SPECIFIC) == V_ASN1_CONTEXT_SPECIFIC)
tag_class = sym_CONTEXT_SPECIFIC;
else if((tc & V_ASN1_APPLICATION) == V_ASN1_APPLICATION)
tag_class = sym_APPLICATION;
else
tag_class = sym_UNIVERSAL;
hlen = p - start;
if(yield) {
VALUE arg = rb_ary_new();
rb_ary_push(arg, LONG2NUM(depth));
rb_ary_push(arg, LONG2NUM(*offset));
rb_ary_push(arg, LONG2NUM(hlen));
rb_ary_push(arg, LONG2NUM(len));
rb_ary_push(arg, (j & V_ASN1_CONSTRUCTED) ? Qtrue : Qfalse);
rb_ary_push(arg, ossl_asn1_class2sym(tc));
rb_ary_push(arg, INT2NUM(tag));
rb_yield(arg);
}
if(j & V_ASN1_CONSTRUCTED) {
*pp += hlen;
off += hlen;
asn1data = int_ossl_asn1_decode0_cons(pp, length - hlen, len, &off, depth, yield, j, tag, tag_class, &inner_read);
inner_read += hlen;
}
else {
if ((j & 0x01) && (len == 0))
ossl_raise(eASN1Error, "indefinite length for primitive value");
asn1data = int_ossl_asn1_decode0_prim(pp, len, hlen, tag, tag_class, &inner_read);
off += hlen + len;
}
if (num_read)
*num_read = inner_read;
if (len != 0 && inner_read != hlen + len) {
ossl_raise(eASN1Error,
"Type mismatch. Bytes read: %ld Bytes available: %ld",
inner_read, hlen + len);
}
*offset = off;
return asn1data;
}
static void
int_ossl_decode_sanity_check(long len, long read, long offset)
{
if (len != 0 && (read != len || offset != len)) {
ossl_raise(eASN1Error,
"Type mismatch. Total bytes read: %ld Bytes available: %ld Offset: %ld",
read, len, offset);
}
}
static VALUE
ossl_asn1_traverse(VALUE self, VALUE obj)
{
unsigned char *p;
VALUE tmp;
long len, read = 0, offset = 0;
obj = ossl_to_der_if_possible(obj);
tmp = rb_str_new4(StringValue(obj));
p = (unsigned char *)RSTRING_PTR(tmp);
len = RSTRING_LEN(tmp);
ossl_asn1_decode0(&p, len, &offset, 0, 1, &read);
RB_GC_GUARD(tmp);
int_ossl_decode_sanity_check(len, read, offset);
return Qnil;
}
static VALUE
ossl_asn1_decode(VALUE self, VALUE obj)
{
VALUE ret;
unsigned char *p;
VALUE tmp;
long len, read = 0, offset = 0;
obj = ossl_to_der_if_possible(obj);
tmp = rb_str_new4(StringValue(obj));
p = (unsigned char *)RSTRING_PTR(tmp);
len = RSTRING_LEN(tmp);
ret = ossl_asn1_decode0(&p, len, &offset, 0, 0, &read);
RB_GC_GUARD(tmp);
int_ossl_decode_sanity_check(len, read, offset);
return ret;
}
static VALUE
ossl_asn1_decode_all(VALUE self, VALUE obj)
{
VALUE ary, val;
unsigned char *p;
long len, tmp_len = 0, read = 0, offset = 0;
VALUE tmp;
obj = ossl_to_der_if_possible(obj);
tmp = rb_str_new4(StringValue(obj));
p = (unsigned char *)RSTRING_PTR(tmp);
len = RSTRING_LEN(tmp);
tmp_len = len;
ary = rb_ary_new();
while (tmp_len > 0) {
long tmp_read = 0;
val = ossl_asn1_decode0(&p, tmp_len, &offset, 0, 0, &tmp_read);
rb_ary_push(ary, val);
read += tmp_read;
tmp_len -= tmp_read;
}
RB_GC_GUARD(tmp);
int_ossl_decode_sanity_check(len, read, offset);
return ary;
}
static VALUE
ossl_asn1_initialize(int argc, VALUE *argv, VALUE self)
{
VALUE value, tag, tagging, tag_class;
int default_tag;
rb_scan_args(argc, argv, "13", &value, &tag, &tagging, &tag_class);
default_tag = ossl_asn1_default_tag(self);
if (default_tag == -1 || argc > 1) {
if(NIL_P(tag))
ossl_raise(eASN1Error, "must specify tag number");
if(!NIL_P(tagging) && !SYMBOL_P(tagging))
ossl_raise(eASN1Error, "invalid tagging method");
if(NIL_P(tag_class)) {
if (NIL_P(tagging))
tag_class = sym_UNIVERSAL;
else
tag_class = sym_CONTEXT_SPECIFIC;
}
if(!SYMBOL_P(tag_class))
ossl_raise(eASN1Error, "invalid tag class");
}
else{
tag = INT2NUM(default_tag);
tagging = Qnil;
tag_class = sym_UNIVERSAL;
}
ossl_asn1_set_tag(self, tag);
ossl_asn1_set_value(self, value);
ossl_asn1_set_tagging(self, tagging);
ossl_asn1_set_tag_class(self, tag_class);
ossl_asn1_set_indefinite_length(self, Qfalse);
if (default_tag == V_ASN1_BIT_STRING)
rb_ivar_set(self, sivUNUSED_BITS, INT2FIX(0));
return self;
}
static VALUE
ossl_asn1eoc_initialize(VALUE self) {
VALUE tag, tagging, tag_class, value;
tag = INT2FIX(0);
tagging = Qnil;
tag_class = sym_UNIVERSAL;
value = rb_str_new("", 0);
ossl_asn1_set_tag(self, tag);
ossl_asn1_set_value(self, value);
ossl_asn1_set_tagging(self, tagging);
ossl_asn1_set_tag_class(self, tag_class);
ossl_asn1_set_indefinite_length(self, Qfalse);
return self;
}
static VALUE
ossl_asn1eoc_to_der(VALUE self)
{
return rb_str_new("\0\0", 2);
}
static VALUE
ossl_asn1prim_to_der(VALUE self)
{
ASN1_TYPE *asn1;
long alllen, bodylen;
unsigned char *p0, *p1;
int j, tag, tc, state;
VALUE str;
if (ossl_asn1_default_tag(self) == -1) {
str = ossl_asn1_get_value(self);
return to_der_internal(self, 0, 0, StringValue(str));
}
asn1 = ossl_asn1_get_asn1type(self);
alllen = i2d_ASN1_TYPE(asn1, NULL);
if (alllen < 0) {
ASN1_TYPE_free(asn1);
ossl_raise(eASN1Error, "i2d_ASN1_TYPE");
}
str = ossl_str_new(NULL, alllen, &state);
if (state) {
ASN1_TYPE_free(asn1);
rb_jump_tag(state);
}
p0 = p1 = (unsigned char *)RSTRING_PTR(str);
i2d_ASN1_TYPE(asn1, &p0);
ASN1_TYPE_free(asn1);
assert(p0 - p1 == alllen);
j = ASN1_get_object((const unsigned char **)&p1, &bodylen, &tag, &tc, alllen);
if (j & 0x80)
ossl_raise(eASN1Error, "ASN1_get_object"); 
return to_der_internal(self, 0, 0, rb_str_drop_bytes(str, alllen - bodylen));
}
static VALUE
ossl_asn1cons_to_der(VALUE self)
{
VALUE ary, str;
long i;
int indef_len;
indef_len = RTEST(ossl_asn1_get_indefinite_length(self));
ary = rb_convert_type(ossl_asn1_get_value(self), T_ARRAY, "Array", "to_a");
str = rb_str_new(NULL, 0);
for (i = 0; i < RARRAY_LEN(ary); i++) {
VALUE item = RARRAY_AREF(ary, i);
if (indef_len && rb_obj_is_kind_of(item, cASN1EndOfContent)) {
if (i != RARRAY_LEN(ary) - 1)
ossl_raise(eASN1Error, "illegal EOC octets in value");
break;
}
item = ossl_to_der_if_possible(item);
StringValue(item);
rb_str_append(str, item);
}
return to_der_internal(self, 1, indef_len, str);
}
static VALUE
ossl_asn1cons_each(VALUE self)
{
rb_block_call(ossl_asn1_get_value(self), id_each, 0, 0, 0, 0);
return self;
}
static VALUE
ossl_asn1obj_s_register(VALUE self, VALUE oid, VALUE sn, VALUE ln)
{
StringValueCStr(oid);
StringValueCStr(sn);
StringValueCStr(ln);
if(!OBJ_create(RSTRING_PTR(oid), RSTRING_PTR(sn), RSTRING_PTR(ln)))
ossl_raise(eASN1Error, NULL);
return Qtrue;
}
static VALUE
ossl_asn1obj_get_sn(VALUE self)
{
VALUE val, ret = Qnil;
int nid;
val = ossl_asn1_get_value(self);
if ((nid = OBJ_txt2nid(StringValueCStr(val))) != NID_undef)
ret = rb_str_new2(OBJ_nid2sn(nid));
return ret;
}
static VALUE
ossl_asn1obj_get_ln(VALUE self)
{
VALUE val, ret = Qnil;
int nid;
val = ossl_asn1_get_value(self);
if ((nid = OBJ_txt2nid(StringValueCStr(val))) != NID_undef)
ret = rb_str_new2(OBJ_nid2ln(nid));
return ret;
}
static VALUE
ossl_asn1obj_eq(VALUE self, VALUE other)
{
VALUE valSelf, valOther;
int nidSelf, nidOther;
valSelf = ossl_asn1_get_value(self);
valOther = ossl_asn1_get_value(other);
if ((nidSelf = OBJ_txt2nid(StringValueCStr(valSelf))) == NID_undef)
ossl_raise(eASN1Error, "OBJ_txt2nid");
if ((nidOther = OBJ_txt2nid(StringValueCStr(valOther))) == NID_undef)
ossl_raise(eASN1Error, "OBJ_txt2nid");
return nidSelf == nidOther ? Qtrue : Qfalse;
}
static VALUE
asn1obj_get_oid_i(VALUE vobj)
{
ASN1_OBJECT *a1obj = (void *)vobj;
VALUE str;
int len;
str = rb_usascii_str_new(NULL, 127);
len = OBJ_obj2txt(RSTRING_PTR(str), RSTRING_LENINT(str), a1obj, 1);
if (len <= 0 || len == INT_MAX)
ossl_raise(eASN1Error, "OBJ_obj2txt");
if (len > RSTRING_LEN(str)) {
rb_str_resize(str, len + 1);
len = OBJ_obj2txt(RSTRING_PTR(str), len + 1, a1obj, 1);
if (len <= 0)
ossl_raise(eASN1Error, "OBJ_obj2txt");
}
rb_str_set_len(str, len);
return str;
}
static VALUE
ossl_asn1obj_get_oid(VALUE self)
{
VALUE str;
ASN1_OBJECT *a1obj;
int state;
a1obj = obj_to_asn1obj(ossl_asn1_get_value(self));
str = rb_protect(asn1obj_get_oid_i, (VALUE)a1obj, &state);
ASN1_OBJECT_free(a1obj);
if (state)
rb_jump_tag(state);
return str;
}
#define OSSL_ASN1_IMPL_FACTORY_METHOD(klass) static VALUE ossl_asn1_##klass(int argc, VALUE *argv, VALUE self){ return rb_funcall3(cASN1##klass, rb_intern("new"), argc, argv); }
OSSL_ASN1_IMPL_FACTORY_METHOD(Boolean)
OSSL_ASN1_IMPL_FACTORY_METHOD(Integer)
OSSL_ASN1_IMPL_FACTORY_METHOD(Enumerated)
OSSL_ASN1_IMPL_FACTORY_METHOD(BitString)
OSSL_ASN1_IMPL_FACTORY_METHOD(OctetString)
OSSL_ASN1_IMPL_FACTORY_METHOD(UTF8String)
OSSL_ASN1_IMPL_FACTORY_METHOD(NumericString)
OSSL_ASN1_IMPL_FACTORY_METHOD(PrintableString)
OSSL_ASN1_IMPL_FACTORY_METHOD(T61String)
OSSL_ASN1_IMPL_FACTORY_METHOD(VideotexString)
OSSL_ASN1_IMPL_FACTORY_METHOD(IA5String)
OSSL_ASN1_IMPL_FACTORY_METHOD(GraphicString)
OSSL_ASN1_IMPL_FACTORY_METHOD(ISO64String)
OSSL_ASN1_IMPL_FACTORY_METHOD(GeneralString)
OSSL_ASN1_IMPL_FACTORY_METHOD(UniversalString)
OSSL_ASN1_IMPL_FACTORY_METHOD(BMPString)
OSSL_ASN1_IMPL_FACTORY_METHOD(Null)
OSSL_ASN1_IMPL_FACTORY_METHOD(ObjectId)
OSSL_ASN1_IMPL_FACTORY_METHOD(UTCTime)
OSSL_ASN1_IMPL_FACTORY_METHOD(GeneralizedTime)
OSSL_ASN1_IMPL_FACTORY_METHOD(Sequence)
OSSL_ASN1_IMPL_FACTORY_METHOD(Set)
OSSL_ASN1_IMPL_FACTORY_METHOD(EndOfContent)
void
Init_ossl_asn1(void)
{
#undef rb_intern
VALUE ary;
int i;
#if 0
mOSSL = rb_define_module("OpenSSL");
eOSSLError = rb_define_class_under(mOSSL, "OpenSSLError", rb_eStandardError);
#endif
sym_UNIVERSAL = ID2SYM(rb_intern_const("UNIVERSAL"));
sym_CONTEXT_SPECIFIC = ID2SYM(rb_intern_const("CONTEXT_SPECIFIC"));
sym_APPLICATION = ID2SYM(rb_intern_const("APPLICATION"));
sym_PRIVATE = ID2SYM(rb_intern_const("PRIVATE"));
sym_EXPLICIT = ID2SYM(rb_intern_const("EXPLICIT"));
sym_IMPLICIT = ID2SYM(rb_intern_const("IMPLICIT"));
sivVALUE = rb_intern("@value");
sivTAG = rb_intern("@tag");
sivTAGGING = rb_intern("@tagging");
sivTAG_CLASS = rb_intern("@tag_class");
sivINDEFINITE_LENGTH = rb_intern("@indefinite_length");
sivUNUSED_BITS = rb_intern("@unused_bits");
mASN1 = rb_define_module_under(mOSSL, "ASN1");
eASN1Error = rb_define_class_under(mASN1, "ASN1Error", eOSSLError);
rb_define_module_function(mASN1, "traverse", ossl_asn1_traverse, 1);
rb_define_module_function(mASN1, "decode", ossl_asn1_decode, 1);
rb_define_module_function(mASN1, "decode_all", ossl_asn1_decode_all, 1);
ary = rb_ary_new();
rb_define_const(mASN1, "UNIVERSAL_TAG_NAME", ary);
for(i = 0; i < ossl_asn1_info_size; i++){
if(ossl_asn1_info[i].name[0] == '[') continue;
rb_define_const(mASN1, ossl_asn1_info[i].name, INT2NUM(i));
rb_ary_store(ary, i, rb_str_new2(ossl_asn1_info[i].name));
}
cASN1Data = rb_define_class_under(mASN1, "ASN1Data", rb_cObject);
rb_attr(cASN1Data, rb_intern("value"), 1, 1, 0);
rb_attr(cASN1Data, rb_intern("tag"), 1, 1, 0);
rb_attr(cASN1Data, rb_intern("tag_class"), 1, 1, 0);
rb_attr(cASN1Data, rb_intern("indefinite_length"), 1, 1, 0);
rb_define_alias(cASN1Data, "infinite_length", "indefinite_length");
rb_define_alias(cASN1Data, "infinite_length=", "indefinite_length=");
rb_define_method(cASN1Data, "initialize", ossl_asn1data_initialize, 3);
rb_define_method(cASN1Data, "to_der", ossl_asn1data_to_der, 0);
cASN1Primitive = rb_define_class_under(mASN1, "Primitive", cASN1Data);
rb_attr(cASN1Primitive, rb_intern("tagging"), 1, 1, Qtrue);
rb_undef_method(cASN1Primitive, "indefinite_length=");
rb_undef_method(cASN1Primitive, "infinite_length=");
rb_define_method(cASN1Primitive, "initialize", ossl_asn1_initialize, -1);
rb_define_method(cASN1Primitive, "to_der", ossl_asn1prim_to_der, 0);
cASN1Constructive = rb_define_class_under(mASN1,"Constructive", cASN1Data);
rb_include_module(cASN1Constructive, rb_mEnumerable);
rb_attr(cASN1Constructive, rb_intern("tagging"), 1, 1, Qtrue);
rb_define_method(cASN1Constructive, "initialize", ossl_asn1_initialize, -1);
rb_define_method(cASN1Constructive, "to_der", ossl_asn1cons_to_der, 0);
rb_define_method(cASN1Constructive, "each", ossl_asn1cons_each, 0);
#define OSSL_ASN1_DEFINE_CLASS(name, super) do{cASN1##name = rb_define_class_under(mASN1, #name, cASN1##super);rb_define_module_function(mASN1, #name, ossl_asn1_##name, -1);}while(0)
OSSL_ASN1_DEFINE_CLASS(Boolean, Primitive);
OSSL_ASN1_DEFINE_CLASS(Integer, Primitive);
OSSL_ASN1_DEFINE_CLASS(Enumerated, Primitive);
OSSL_ASN1_DEFINE_CLASS(BitString, Primitive);
OSSL_ASN1_DEFINE_CLASS(OctetString, Primitive);
OSSL_ASN1_DEFINE_CLASS(UTF8String, Primitive);
OSSL_ASN1_DEFINE_CLASS(NumericString, Primitive);
OSSL_ASN1_DEFINE_CLASS(PrintableString, Primitive);
OSSL_ASN1_DEFINE_CLASS(T61String, Primitive);
OSSL_ASN1_DEFINE_CLASS(VideotexString, Primitive);
OSSL_ASN1_DEFINE_CLASS(IA5String, Primitive);
OSSL_ASN1_DEFINE_CLASS(GraphicString, Primitive);
OSSL_ASN1_DEFINE_CLASS(ISO64String, Primitive);
OSSL_ASN1_DEFINE_CLASS(GeneralString, Primitive);
OSSL_ASN1_DEFINE_CLASS(UniversalString, Primitive);
OSSL_ASN1_DEFINE_CLASS(BMPString, Primitive);
OSSL_ASN1_DEFINE_CLASS(Null, Primitive);
OSSL_ASN1_DEFINE_CLASS(ObjectId, Primitive);
OSSL_ASN1_DEFINE_CLASS(UTCTime, Primitive);
OSSL_ASN1_DEFINE_CLASS(GeneralizedTime, Primitive);
OSSL_ASN1_DEFINE_CLASS(Sequence, Constructive);
OSSL_ASN1_DEFINE_CLASS(Set, Constructive);
OSSL_ASN1_DEFINE_CLASS(EndOfContent, Data);
#if 0
cASN1ObjectId = rb_define_class_under(mASN1, "ObjectId", cASN1Primitive); 
#endif
rb_define_singleton_method(cASN1ObjectId, "register", ossl_asn1obj_s_register, 3);
rb_define_method(cASN1ObjectId, "sn", ossl_asn1obj_get_sn, 0);
rb_define_method(cASN1ObjectId, "ln", ossl_asn1obj_get_ln, 0);
rb_define_method(cASN1ObjectId, "oid", ossl_asn1obj_get_oid, 0);
rb_define_alias(cASN1ObjectId, "short_name", "sn");
rb_define_alias(cASN1ObjectId, "long_name", "ln");
rb_define_method(cASN1ObjectId, "==", ossl_asn1obj_eq, 1);
rb_attr(cASN1BitString, rb_intern("unused_bits"), 1, 1, 0);
rb_define_method(cASN1EndOfContent, "initialize", ossl_asn1eoc_initialize, 0);
rb_define_method(cASN1EndOfContent, "to_der", ossl_asn1eoc_to_der, 0);
class_tag_map = rb_hash_new();
rb_gc_register_mark_object(class_tag_map);
rb_hash_aset(class_tag_map, cASN1EndOfContent, INT2NUM(V_ASN1_EOC));
rb_hash_aset(class_tag_map, cASN1Boolean, INT2NUM(V_ASN1_BOOLEAN));
rb_hash_aset(class_tag_map, cASN1Integer, INT2NUM(V_ASN1_INTEGER));
rb_hash_aset(class_tag_map, cASN1BitString, INT2NUM(V_ASN1_BIT_STRING));
rb_hash_aset(class_tag_map, cASN1OctetString, INT2NUM(V_ASN1_OCTET_STRING));
rb_hash_aset(class_tag_map, cASN1Null, INT2NUM(V_ASN1_NULL));
rb_hash_aset(class_tag_map, cASN1ObjectId, INT2NUM(V_ASN1_OBJECT));
rb_hash_aset(class_tag_map, cASN1Enumerated, INT2NUM(V_ASN1_ENUMERATED));
rb_hash_aset(class_tag_map, cASN1UTF8String, INT2NUM(V_ASN1_UTF8STRING));
rb_hash_aset(class_tag_map, cASN1Sequence, INT2NUM(V_ASN1_SEQUENCE));
rb_hash_aset(class_tag_map, cASN1Set, INT2NUM(V_ASN1_SET));
rb_hash_aset(class_tag_map, cASN1NumericString, INT2NUM(V_ASN1_NUMERICSTRING));
rb_hash_aset(class_tag_map, cASN1PrintableString, INT2NUM(V_ASN1_PRINTABLESTRING));
rb_hash_aset(class_tag_map, cASN1T61String, INT2NUM(V_ASN1_T61STRING));
rb_hash_aset(class_tag_map, cASN1VideotexString, INT2NUM(V_ASN1_VIDEOTEXSTRING));
rb_hash_aset(class_tag_map, cASN1IA5String, INT2NUM(V_ASN1_IA5STRING));
rb_hash_aset(class_tag_map, cASN1UTCTime, INT2NUM(V_ASN1_UTCTIME));
rb_hash_aset(class_tag_map, cASN1GeneralizedTime, INT2NUM(V_ASN1_GENERALIZEDTIME));
rb_hash_aset(class_tag_map, cASN1GraphicString, INT2NUM(V_ASN1_GRAPHICSTRING));
rb_hash_aset(class_tag_map, cASN1ISO64String, INT2NUM(V_ASN1_ISO64STRING));
rb_hash_aset(class_tag_map, cASN1GeneralString, INT2NUM(V_ASN1_GENERALSTRING));
rb_hash_aset(class_tag_map, cASN1UniversalString, INT2NUM(V_ASN1_UNIVERSALSTRING));
rb_hash_aset(class_tag_map, cASN1BMPString, INT2NUM(V_ASN1_BMPSTRING));
id_each = rb_intern_const("each");
}
