














#include "digest.h"

static VALUE rb_mDigest;
static VALUE rb_mDigest_Instance;
static VALUE rb_cDigest_Class;
static VALUE rb_cDigest_Base;

static ID id_reset, id_update, id_finish, id_digest, id_hexdigest, id_digest_length;
static ID id_metadata;

RUBY_EXTERN void Init_digest_base(void);






































































static VALUE
hexencode_str_new(VALUE str_digest)
{
char *digest;
size_t digest_len;
size_t i;
VALUE str;
char *p;
static const char hex[] = {
'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
'a', 'b', 'c', 'd', 'e', 'f'
};

StringValue(str_digest);
digest = RSTRING_PTR(str_digest);
digest_len = RSTRING_LEN(str_digest);

if (LONG_MAX / 2 < digest_len) {
rb_raise(rb_eRuntimeError, "digest string too long");
}

str = rb_usascii_str_new(0, digest_len * 2);

for (i = 0, p = RSTRING_PTR(str); i < digest_len; i++) {
unsigned char byte = digest[i];

p[i + i] = hex[byte >> 4];
p[i + i + 1] = hex[byte & 0x0f];
}

RB_GC_GUARD(str_digest);

return str;
}







static VALUE
rb_digest_s_hexencode(VALUE klass, VALUE str)
{
return hexencode_str_new(str);
}

NORETURN(static void rb_digest_instance_method_unimpl(VALUE self, const char *method));








static void
rb_digest_instance_method_unimpl(VALUE self, const char *method)
{
rb_raise(rb_eRuntimeError, "%s does not implement %s()",
rb_obj_classname(self), method);
}












static VALUE
rb_digest_instance_update(VALUE self, VALUE str)
{
rb_digest_instance_method_unimpl(self, "update");

UNREACHABLE;
}













static VALUE
rb_digest_instance_finish(VALUE self)
{
rb_digest_instance_method_unimpl(self, "finish");

UNREACHABLE;
}









static VALUE
rb_digest_instance_reset(VALUE self)
{
rb_digest_instance_method_unimpl(self, "reset");

UNREACHABLE;
}








static VALUE
rb_digest_instance_new(VALUE self)
{
VALUE clone = rb_obj_clone(self);
rb_funcall(clone, id_reset, 0);
return clone;
}













static VALUE
rb_digest_instance_digest(int argc, VALUE *argv, VALUE self)
{
VALUE str, value;

if (rb_scan_args(argc, argv, "01", &str) > 0) {
rb_funcall(self, id_reset, 0);
rb_funcall(self, id_update, 1, str);
value = rb_funcall(self, id_finish, 0);
rb_funcall(self, id_reset, 0);
} else {
value = rb_funcall(rb_obj_clone(self), id_finish, 0);
}

return value;
}








static VALUE
rb_digest_instance_digest_bang(VALUE self)
{
VALUE value = rb_funcall(self, id_finish, 0);
rb_funcall(self, id_reset, 0);

return value;
}













static VALUE
rb_digest_instance_hexdigest(int argc, VALUE *argv, VALUE self)
{
VALUE str, value;

if (rb_scan_args(argc, argv, "01", &str) > 0) {
rb_funcall(self, id_reset, 0);
rb_funcall(self, id_update, 1, str);
value = rb_funcall(self, id_finish, 0);
rb_funcall(self, id_reset, 0);
} else {
value = rb_funcall(rb_obj_clone(self), id_finish, 0);
}

return hexencode_str_new(value);
}








static VALUE
rb_digest_instance_hexdigest_bang(VALUE self)
{
VALUE value = rb_funcall(self, id_finish, 0);
rb_funcall(self, id_reset, 0);

return hexencode_str_new(value);
}







static VALUE
rb_digest_instance_to_s(VALUE self)
{
return rb_funcall(self, id_hexdigest, 0);
}







static VALUE
rb_digest_instance_inspect(VALUE self)
{
VALUE str;
size_t digest_len = 32; 
const char *cname;

cname = rb_obj_classname(self);


str = rb_str_buf_new(2 + strlen(cname) + 2 + digest_len * 2 + 1);
rb_str_buf_cat2(str, "#<");
rb_str_buf_cat2(str, cname);
rb_str_buf_cat2(str, ": ");
rb_str_buf_append(str, rb_digest_instance_hexdigest(0, 0, self));
rb_str_buf_cat2(str, ">");
return str;
}











static VALUE
rb_digest_instance_equal(VALUE self, VALUE other)
{
VALUE str1, str2;

if (rb_obj_is_kind_of(other, rb_mDigest_Instance) == Qtrue) {
str1 = rb_digest_instance_digest(0, 0, self);
str2 = rb_digest_instance_digest(0, 0, other);
} else {
str1 = rb_digest_instance_to_s(self);
str2 = rb_check_string_type(other);
if (NIL_P(str2)) return Qfalse;
}


StringValue(str1);
StringValue(str2);

if (RSTRING_LEN(str1) == RSTRING_LEN(str2) &&
rb_str_cmp(str1, str2) == 0) {
return Qtrue;
}
return Qfalse;
}










static VALUE
rb_digest_instance_digest_length(VALUE self)
{

VALUE digest = rb_digest_instance_digest(0, 0, self);


StringValue(digest);
return INT2NUM(RSTRING_LEN(digest));
}








static VALUE
rb_digest_instance_length(VALUE self)
{
return rb_funcall(self, id_digest_length, 0);
}









static VALUE
rb_digest_instance_block_length(VALUE self)
{
rb_digest_instance_method_unimpl(self, "block_length");

UNREACHABLE;
}

















static VALUE
rb_digest_class_s_digest(int argc, VALUE *argv, VALUE klass)
{
VALUE str;
volatile VALUE obj;

if (argc < 1) {
rb_raise(rb_eArgError, "no data given");
}

str = *argv++;
argc--;

StringValue(str);

obj = rb_obj_alloc(klass);
rb_obj_call_init(obj, argc, argv);

return rb_funcall(obj, id_digest, 1, str);
}









static VALUE
rb_digest_class_s_hexdigest(int argc, VALUE *argv, VALUE klass)
{
return hexencode_str_new(rb_funcallv(klass, id_digest, argc, argv));
}


static VALUE
rb_digest_class_init(VALUE self)
{
return self;
}










































static rb_digest_metadata_t *
get_digest_base_metadata(VALUE klass)
{
VALUE p;
VALUE obj;
rb_digest_metadata_t *algo;

for (p = klass; !NIL_P(p); p = rb_class_superclass(p)) {
if (rb_ivar_defined(p, id_metadata)) {
obj = rb_ivar_get(p, id_metadata);
break;
}
}

if (NIL_P(p))
rb_raise(rb_eRuntimeError, "Digest::Base cannot be directly inherited in Ruby");

if (!RB_TYPE_P(obj, T_DATA) || RTYPEDDATA_P(obj)) {
wrong:
if (p == klass)
rb_raise(rb_eTypeError, "%"PRIsVALUE"::metadata is not initialized properly",
klass);
else
rb_raise(rb_eTypeError, "%"PRIsVALUE"(%"PRIsVALUE")::metadata is not initialized properly",
klass, p);
}

#undef RUBY_UNTYPED_DATA_WARNING
#define RUBY_UNTYPED_DATA_WARNING 0
Data_Get_Struct(obj, rb_digest_metadata_t, algo);

if (!algo) goto wrong;

switch (algo->api_version) {
case 3:
break;





default:
rb_raise(rb_eRuntimeError, "Incompatible digest API version");
}

return algo;
}

static rb_digest_metadata_t *
get_digest_obj_metadata(VALUE obj)
{
return get_digest_base_metadata(rb_obj_class(obj));
}

static const rb_data_type_t digest_type = {
"digest",
{0, RUBY_TYPED_DEFAULT_FREE, 0,},
0, 0,
(RUBY_TYPED_FREE_IMMEDIATELY|RUBY_TYPED_WB_PROTECTED),
};

static inline void
algo_init(const rb_digest_metadata_t *algo, void *pctx)
{
if (algo->init_func(pctx) != 1) {
rb_raise(rb_eRuntimeError, "Digest initialization failed.");
}
}

static VALUE
rb_digest_base_alloc(VALUE klass)
{
rb_digest_metadata_t *algo;
VALUE obj;
void *pctx;

if (klass == rb_cDigest_Base) {
rb_raise(rb_eNotImpError, "Digest::Base is an abstract class");
}

algo = get_digest_base_metadata(klass);

obj = rb_data_typed_object_zalloc(klass, algo->ctx_size, &digest_type);
pctx = RTYPEDDATA_DATA(obj);
algo_init(algo, pctx);

return obj;
}


static VALUE
rb_digest_base_copy(VALUE copy, VALUE obj)
{
rb_digest_metadata_t *algo;
void *pctx1, *pctx2;

if (copy == obj) return copy;

rb_check_frozen(copy);

algo = get_digest_obj_metadata(copy);
if (algo != get_digest_obj_metadata(obj))
rb_raise(rb_eTypeError, "different algorithms");

TypedData_Get_Struct(obj, void, &digest_type, pctx1);
TypedData_Get_Struct(copy, void, &digest_type, pctx2);
memcpy(pctx2, pctx1, algo->ctx_size);

return copy;
}






static VALUE
rb_digest_base_reset(VALUE self)
{
rb_digest_metadata_t *algo;
void *pctx;

algo = get_digest_obj_metadata(self);

TypedData_Get_Struct(self, void, &digest_type, pctx);

algo_init(algo, pctx);

return self;
}








static VALUE
rb_digest_base_update(VALUE self, VALUE str)
{
rb_digest_metadata_t *algo;
void *pctx;

algo = get_digest_obj_metadata(self);

TypedData_Get_Struct(self, void, &digest_type, pctx);

StringValue(str);
algo->update_func(pctx, (unsigned char *)RSTRING_PTR(str), RSTRING_LEN(str));
RB_GC_GUARD(str);

return self;
}


static VALUE
rb_digest_base_finish(VALUE self)
{
rb_digest_metadata_t *algo;
void *pctx;
VALUE str;

algo = get_digest_obj_metadata(self);

TypedData_Get_Struct(self, void, &digest_type, pctx);

str = rb_str_new(0, algo->digest_len);
algo->finish_func(pctx, (unsigned char *)RSTRING_PTR(str));


algo_init(algo, pctx);

return str;
}






static VALUE
rb_digest_base_digest_length(VALUE self)
{
rb_digest_metadata_t *algo;

algo = get_digest_obj_metadata(self);

return INT2NUM(algo->digest_len);
}






static VALUE
rb_digest_base_block_length(VALUE self)
{
rb_digest_metadata_t *algo;

algo = get_digest_obj_metadata(self);

return INT2NUM(algo->block_len);
}

void
Init_digest(void)
{
#undef rb_intern
id_reset = rb_intern("reset");
id_update = rb_intern("update");
id_finish = rb_intern("finish");
id_digest = rb_intern("digest");
id_hexdigest = rb_intern("hexdigest");
id_digest_length = rb_intern("digest_length");
id_metadata = rb_id_metadata();
InitVM(digest);
}

void
InitVM_digest(void)
{



rb_mDigest = rb_define_module("Digest");


rb_define_module_function(rb_mDigest, "hexencode", rb_digest_s_hexencode, 1);




rb_mDigest_Instance = rb_define_module_under(rb_mDigest, "Instance");


rb_define_method(rb_mDigest_Instance, "update", rb_digest_instance_update, 1);
rb_define_method(rb_mDigest_Instance, "<<", rb_digest_instance_update, 1);
rb_define_private_method(rb_mDigest_Instance, "finish", rb_digest_instance_finish, 0);
rb_define_method(rb_mDigest_Instance, "reset", rb_digest_instance_reset, 0);
rb_define_method(rb_mDigest_Instance, "digest_length", rb_digest_instance_digest_length, 0);
rb_define_method(rb_mDigest_Instance, "block_length", rb_digest_instance_block_length, 0);


rb_define_method(rb_mDigest_Instance, "==", rb_digest_instance_equal, 1);
rb_define_method(rb_mDigest_Instance, "inspect", rb_digest_instance_inspect, 0);


rb_define_method(rb_mDigest_Instance, "new", rb_digest_instance_new, 0);
rb_define_method(rb_mDigest_Instance, "digest", rb_digest_instance_digest, -1);
rb_define_method(rb_mDigest_Instance, "digest!", rb_digest_instance_digest_bang, 0);
rb_define_method(rb_mDigest_Instance, "hexdigest", rb_digest_instance_hexdigest, -1);
rb_define_method(rb_mDigest_Instance, "hexdigest!", rb_digest_instance_hexdigest_bang, 0);
rb_define_method(rb_mDigest_Instance, "to_s", rb_digest_instance_to_s, 0);
rb_define_method(rb_mDigest_Instance, "length", rb_digest_instance_length, 0);
rb_define_method(rb_mDigest_Instance, "size", rb_digest_instance_length, 0);




rb_cDigest_Class = rb_define_class_under(rb_mDigest, "Class", rb_cObject);
rb_define_method(rb_cDigest_Class, "initialize", rb_digest_class_init, 0);
rb_include_module(rb_cDigest_Class, rb_mDigest_Instance);


rb_define_singleton_method(rb_cDigest_Class, "digest", rb_digest_class_s_digest, -1);
rb_define_singleton_method(rb_cDigest_Class, "hexdigest", rb_digest_class_s_hexdigest, -1);


rb_cDigest_Base = rb_define_class_under(rb_mDigest, "Base", rb_cDigest_Class);

rb_define_alloc_func(rb_cDigest_Base, rb_digest_base_alloc);

rb_define_method(rb_cDigest_Base, "initialize_copy", rb_digest_base_copy, 1);
rb_define_method(rb_cDigest_Base, "reset", rb_digest_base_reset, 0);
rb_define_method(rb_cDigest_Base, "update", rb_digest_base_update, 1);
rb_define_method(rb_cDigest_Base, "<<", rb_digest_base_update, 1);
rb_define_private_method(rb_cDigest_Base, "finish", rb_digest_base_finish, 0);
rb_define_method(rb_cDigest_Base, "digest_length", rb_digest_base_digest_length, 0);
rb_define_method(rb_cDigest_Base, "block_length", rb_digest_base_block_length, 0);
}
