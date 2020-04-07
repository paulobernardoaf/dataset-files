#include "ruby/config.h"
#include <ctype.h>
#include <errno.h>
#include <float.h>
#include <math.h>
#include <stdio.h>
#include "constant.h"
#include "id.h"
#include "internal.h"
#include "internal/array.h"
#include "internal/class.h"
#include "internal/error.h"
#include "internal/eval.h"
#include "internal/inits.h"
#include "internal/numeric.h"
#include "internal/object.h"
#include "internal/struct.h"
#include "internal/symbol.h"
#include "internal/variable.h"
#include "probes.h"
#include "ruby/encoding.h"
#include "ruby/st.h"
#include "ruby/util.h"
#include "builtin.h"
VALUE rb_cBasicObject; 
VALUE rb_mKernel; 
VALUE rb_cObject; 
VALUE rb_cModule; 
VALUE rb_cClass; 
VALUE rb_cData; 
VALUE rb_cNilClass; 
VALUE rb_cTrueClass; 
VALUE rb_cFalseClass; 
static VALUE rb_cNilClass_to_s;
static VALUE rb_cTrueClass_to_s;
static VALUE rb_cFalseClass_to_s;
#define id_eq idEq
#define id_eql idEqlP
#define id_match idEqTilde
#define id_inspect idInspect
#define id_init_copy idInitialize_copy
#define id_init_clone idInitialize_clone
#define id_init_dup idInitialize_dup
#define id_const_missing idConst_missing
#define id_to_f idTo_f
#define CLASS_OR_MODULE_P(obj) (!SPECIAL_CONST_P(obj) && (BUILTIN_TYPE(obj) == T_CLASS || BUILTIN_TYPE(obj) == T_MODULE))
VALUE
rb_obj_hide(VALUE obj)
{
if (!SPECIAL_CONST_P(obj)) {
RBASIC_CLEAR_CLASS(obj);
}
return obj;
}
VALUE
rb_obj_reveal(VALUE obj, VALUE klass)
{
if (!SPECIAL_CONST_P(obj)) {
RBASIC_SET_CLASS(obj, klass);
}
return obj;
}
VALUE
rb_obj_setup(VALUE obj, VALUE klass, VALUE type)
{
RBASIC(obj)->flags = type;
RBASIC_SET_CLASS(obj, klass);
return obj;
}
VALUE
rb_equal(VALUE obj1, VALUE obj2)
{
VALUE result;
if (obj1 == obj2) return Qtrue;
result = rb_equal_opt(obj1, obj2);
if (result == Qundef) {
result = rb_funcall(obj1, id_eq, 1, obj2);
}
if (RTEST(result)) return Qtrue;
return Qfalse;
}
int
rb_eql(VALUE obj1, VALUE obj2)
{
VALUE result;
if (obj1 == obj2) return Qtrue;
result = rb_eql_opt(obj1, obj2);
if (result == Qundef) {
result = rb_funcall(obj1, id_eql, 1, obj2);
}
if (RTEST(result)) return Qtrue;
return Qfalse;
}
MJIT_FUNC_EXPORTED VALUE
rb_obj_equal(VALUE obj1, VALUE obj2)
{
if (obj1 == obj2) return Qtrue;
return Qfalse;
}
VALUE rb_obj_hash(VALUE obj);
MJIT_FUNC_EXPORTED VALUE
rb_obj_not(VALUE obj)
{
return RTEST(obj) ? Qfalse : Qtrue;
}
MJIT_FUNC_EXPORTED VALUE
rb_obj_not_equal(VALUE obj1, VALUE obj2)
{
VALUE result = rb_funcall(obj1, id_eq, 1, obj2);
return RTEST(result) ? Qfalse : Qtrue;
}
VALUE
rb_class_real(VALUE cl)
{
while (cl &&
((RBASIC(cl)->flags & FL_SINGLETON) || BUILTIN_TYPE(cl) == T_ICLASS)) {
cl = RCLASS_SUPER(cl);
}
return cl;
}
VALUE
rb_obj_class(VALUE obj)
{
return rb_class_real(CLASS_OF(obj));
}
static VALUE
rb_obj_singleton_class(VALUE obj)
{
return rb_singleton_class(obj);
}
MJIT_FUNC_EXPORTED void
rb_obj_copy_ivar(VALUE dest, VALUE obj)
{
if (!(RBASIC(dest)->flags & ROBJECT_EMBED) && ROBJECT_IVPTR(dest)) {
xfree(ROBJECT_IVPTR(dest));
ROBJECT(dest)->as.heap.ivptr = 0;
ROBJECT(dest)->as.heap.numiv = 0;
ROBJECT(dest)->as.heap.iv_index_tbl = 0;
}
if (RBASIC(obj)->flags & ROBJECT_EMBED) {
MEMCPY(ROBJECT(dest)->as.ary, ROBJECT(obj)->as.ary, VALUE, ROBJECT_EMBED_LEN_MAX);
RBASIC(dest)->flags |= ROBJECT_EMBED;
}
else {
uint32_t len = ROBJECT(obj)->as.heap.numiv;
VALUE *ptr = 0;
if (len > 0) {
ptr = ALLOC_N(VALUE, len);
MEMCPY(ptr, ROBJECT(obj)->as.heap.ivptr, VALUE, len);
}
ROBJECT(dest)->as.heap.ivptr = ptr;
ROBJECT(dest)->as.heap.numiv = len;
ROBJECT(dest)->as.heap.iv_index_tbl = ROBJECT(obj)->as.heap.iv_index_tbl;
RBASIC(dest)->flags &= ~ROBJECT_EMBED;
}
}
static void
init_copy(VALUE dest, VALUE obj)
{
if (OBJ_FROZEN(dest)) {
rb_raise(rb_eTypeError, "[bug] frozen object (%s) allocated", rb_obj_classname(dest));
}
RBASIC(dest)->flags &= ~(T_MASK|FL_EXIVAR);
RBASIC(dest)->flags |= RBASIC(obj)->flags & (T_MASK|FL_EXIVAR);
rb_copy_wb_protected_attribute(dest, obj);
rb_copy_generic_ivar(dest, obj);
rb_gc_copy_finalizer(dest, obj);
if (RB_TYPE_P(obj, T_OBJECT)) {
rb_obj_copy_ivar(dest, obj);
}
}
static VALUE freeze_opt(int argc, VALUE *argv);
static VALUE immutable_obj_clone(VALUE obj, VALUE kwfreeze);
static VALUE mutable_obj_clone(VALUE obj, VALUE kwfreeze);
PUREFUNC(static inline int special_object_p(VALUE obj)); 
static inline int
special_object_p(VALUE obj)
{
if (SPECIAL_CONST_P(obj)) return TRUE;
switch (BUILTIN_TYPE(obj)) {
case T_BIGNUM:
case T_FLOAT:
case T_SYMBOL:
case T_RATIONAL:
case T_COMPLEX:
return TRUE;
default:
return FALSE;
}
}
static VALUE
obj_freeze_opt(VALUE freeze)
{
switch(freeze) {
case Qfalse:
case Qtrue:
case Qnil:
break;
default:
rb_raise(rb_eArgError, "unexpected value for freeze: %"PRIsVALUE, rb_obj_class(freeze));
}
return freeze;
}
static VALUE
rb_obj_clone2(rb_execution_context_t *ec, VALUE obj, VALUE freeze)
{
VALUE kwfreeze = obj_freeze_opt(freeze);
if (!special_object_p(obj))
return mutable_obj_clone(obj, kwfreeze);
return immutable_obj_clone(obj, kwfreeze);
}
VALUE
rb_immutable_obj_clone(int argc, VALUE *argv, VALUE obj)
{
VALUE kwfreeze = freeze_opt(argc, argv);
return immutable_obj_clone(obj, kwfreeze);
}
static VALUE
freeze_opt(int argc, VALUE *argv)
{
static ID keyword_ids[1];
VALUE opt;
VALUE kwfreeze = Qnil;
if (!keyword_ids[0]) {
CONST_ID(keyword_ids[0], "freeze");
}
rb_scan_args(argc, argv, "0:", &opt);
if (!NIL_P(opt)) {
rb_get_kwargs(opt, keyword_ids, 0, 1, &kwfreeze);
if (kwfreeze != Qundef)
kwfreeze = obj_freeze_opt(kwfreeze);
}
return kwfreeze;
}
static VALUE
immutable_obj_clone(VALUE obj, VALUE kwfreeze)
{
if (kwfreeze == Qfalse)
rb_raise(rb_eArgError, "can't unfreeze %"PRIsVALUE,
rb_obj_class(obj));
return obj;
}
static VALUE
mutable_obj_clone(VALUE obj, VALUE kwfreeze)
{
VALUE clone, singleton;
VALUE argv[2];
clone = rb_obj_alloc(rb_obj_class(obj));
singleton = rb_singleton_class_clone_and_attach(obj, clone);
RBASIC_SET_CLASS(clone, singleton);
if (FL_TEST(singleton, FL_SINGLETON)) {
rb_singleton_class_attached(singleton, clone);
}
init_copy(clone, obj);
switch (kwfreeze) {
case Qnil:
rb_funcall(clone, id_init_clone, 1, obj);
RBASIC(clone)->flags |= RBASIC(obj)->flags & FL_FREEZE;
break;
case Qtrue:
{
static VALUE freeze_true_hash;
if (!freeze_true_hash) {
freeze_true_hash = rb_hash_new();
rb_gc_register_mark_object(freeze_true_hash);
rb_hash_aset(freeze_true_hash, ID2SYM(rb_intern("freeze")), Qtrue);
rb_obj_freeze(freeze_true_hash);
}
argv[0] = obj;
argv[1] = freeze_true_hash;
rb_funcallv_kw(clone, id_init_clone, 2, argv, RB_PASS_KEYWORDS);
RBASIC(clone)->flags |= FL_FREEZE;
break;
}
case Qfalse:
{
static VALUE freeze_false_hash;
if (!freeze_false_hash) {
freeze_false_hash = rb_hash_new();
rb_gc_register_mark_object(freeze_false_hash);
rb_hash_aset(freeze_false_hash, ID2SYM(rb_intern("freeze")), Qfalse);
rb_obj_freeze(freeze_false_hash);
}
argv[0] = obj;
argv[1] = freeze_false_hash;
rb_funcallv_kw(clone, id_init_clone, 2, argv, RB_PASS_KEYWORDS);
break;
}
default:
rb_bug("invalid kwfreeze passed to mutable_obj_clone");
}
return clone;
}
VALUE
rb_obj_clone(VALUE obj)
{
if (special_object_p(obj)) return obj;
return mutable_obj_clone(obj, Qnil);
}
VALUE
rb_obj_dup(VALUE obj)
{
VALUE dup;
if (special_object_p(obj)) {
return obj;
}
dup = rb_obj_alloc(rb_obj_class(obj));
init_copy(dup, obj);
rb_funcall(dup, id_init_dup, 1, obj);
return dup;
}
static VALUE
rb_obj_itself(VALUE obj)
{
return obj;
}
static VALUE
rb_obj_size(VALUE self, VALUE args, VALUE obj)
{
return LONG2FIX(1);
}
static VALUE
rb_obj_yield_self(VALUE obj)
{
RETURN_SIZED_ENUMERATOR(obj, 0, 0, rb_obj_size);
return rb_yield_values2(1, &obj);
}
VALUE
rb_obj_init_copy(VALUE obj, VALUE orig)
{
if (obj == orig) return obj;
rb_check_frozen(obj);
if (TYPE(obj) != TYPE(orig) || rb_obj_class(obj) != rb_obj_class(orig)) {
rb_raise(rb_eTypeError, "initialize_copy should take same class object");
}
return obj;
}
VALUE
rb_obj_init_dup_clone(VALUE obj, VALUE orig)
{
rb_funcall(obj, id_init_copy, 1, orig);
return obj;
}
static VALUE
rb_obj_init_clone(int argc, VALUE *argv, VALUE obj)
{
VALUE orig, opts;
rb_scan_args(argc, argv, "1:", &orig, &opts);
if (argc == 2) (void)freeze_opt(1, &opts);
rb_funcall(obj, id_init_copy, 1, orig);
return obj;
}
VALUE
rb_any_to_s(VALUE obj)
{
VALUE str;
VALUE cname = rb_class_name(CLASS_OF(obj));
str = rb_sprintf("#<%"PRIsVALUE":%p>", cname, (void*)obj);
return str;
}
VALUE rb_str_escape(VALUE str);
VALUE
rb_inspect(VALUE obj)
{
VALUE str = rb_obj_as_string(rb_funcallv(obj, id_inspect, 0, 0));
rb_encoding *enc = rb_default_internal_encoding();
if (enc == NULL) enc = rb_default_external_encoding();
if (!rb_enc_asciicompat(enc)) {
if (!rb_enc_str_asciionly_p(str))
return rb_str_escape(str);
return str;
}
if (rb_enc_get(str) != enc && !rb_enc_str_asciionly_p(str))
return rb_str_escape(str);
return str;
}
static int
inspect_i(st_data_t k, st_data_t v, st_data_t a)
{
ID id = (ID)k;
VALUE value = (VALUE)v;
VALUE str = (VALUE)a;
if (CLASS_OF(value) == 0) return ST_CONTINUE;
if (!rb_is_instance_id(id)) return ST_CONTINUE;
if (RSTRING_PTR(str)[0] == '-') { 
RSTRING_PTR(str)[0] = '#';
rb_str_cat2(str, " ");
}
else {
rb_str_cat2(str, ", ");
}
rb_str_catf(str, "%"PRIsVALUE"=%+"PRIsVALUE,
rb_id2str(id), value);
return ST_CONTINUE;
}
static VALUE
inspect_obj(VALUE obj, VALUE str, int recur)
{
if (recur) {
rb_str_cat2(str, " ...");
}
else {
rb_ivar_foreach(obj, inspect_i, str);
}
rb_str_cat2(str, ">");
RSTRING_PTR(str)[0] = '#';
return str;
}
static VALUE
rb_obj_inspect(VALUE obj)
{
if (rb_ivar_count(obj) > 0) {
VALUE str;
VALUE c = rb_class_name(CLASS_OF(obj));
str = rb_sprintf("-<%"PRIsVALUE":%p", c, (void*)obj);
return rb_exec_recursive(inspect_obj, obj, str);
}
else {
return rb_any_to_s(obj);
}
}
static VALUE
class_or_module_required(VALUE c)
{
if (SPECIAL_CONST_P(c)) goto not_class;
switch (BUILTIN_TYPE(c)) {
case T_MODULE:
case T_CLASS:
case T_ICLASS:
break;
default:
not_class:
rb_raise(rb_eTypeError, "class or module required");
}
return c;
}
static VALUE class_search_ancestor(VALUE cl, VALUE c);
VALUE
rb_obj_is_instance_of(VALUE obj, VALUE c)
{
c = class_or_module_required(c);
if (rb_obj_class(obj) == c) return Qtrue;
return Qfalse;
}
VALUE
rb_obj_is_kind_of(VALUE obj, VALUE c)
{
VALUE cl = CLASS_OF(obj);
c = class_or_module_required(c);
return class_search_ancestor(cl, RCLASS_ORIGIN(c)) ? Qtrue : Qfalse;
}
static VALUE
class_search_ancestor(VALUE cl, VALUE c)
{
while (cl) {
if (cl == c || RCLASS_M_TBL(cl) == RCLASS_M_TBL(c))
return cl;
cl = RCLASS_SUPER(cl);
}
return 0;
}
VALUE
rb_class_search_ancestor(VALUE cl, VALUE c)
{
cl = class_or_module_required(cl);
c = class_or_module_required(c);
return class_search_ancestor(cl, RCLASS_ORIGIN(c));
}
VALUE
rb_obj_tap(VALUE obj)
{
rb_yield(obj);
return obj;
}
static VALUE
rb_obj_dummy()
{
return Qnil;
}
static VALUE
rb_obj_dummy0(VALUE _)
{
return rb_obj_dummy();
}
static VALUE
rb_obj_dummy1(VALUE _x, VALUE _y)
{
return rb_obj_dummy();
}
VALUE
rb_obj_tainted(VALUE obj)
{
rb_warn_deprecated_to_remove("Object#tainted?", "3.2");
return Qfalse;
}
VALUE
rb_obj_taint(VALUE obj)
{
rb_warn_deprecated_to_remove("Object#taint", "3.2");
return obj;
}
VALUE
rb_obj_untaint(VALUE obj)
{
rb_warn_deprecated_to_remove("Object#untaint", "3.2");
return obj;
}
VALUE
rb_obj_untrusted(VALUE obj)
{
rb_warn_deprecated_to_remove("Object#untrusted?", "3.2");
return Qfalse;
}
VALUE
rb_obj_untrust(VALUE obj)
{
rb_warn_deprecated_to_remove("Object#untrust", "3.2");
return obj;
}
VALUE
rb_obj_trust(VALUE obj)
{
rb_warn_deprecated_to_remove("Object#trust", "3.2");
return obj;
}
void
rb_obj_infect(VALUE victim, VALUE carrier)
{
rb_warn_deprecated_to_remove("rb_obj_infect", "3.2");
}
VALUE
rb_obj_freeze(VALUE obj)
{
if (!OBJ_FROZEN(obj)) {
OBJ_FREEZE(obj);
if (SPECIAL_CONST_P(obj)) {
rb_bug("special consts should be frozen.");
}
}
return obj;
}
VALUE
rb_obj_frozen_p(VALUE obj)
{
return OBJ_FROZEN(obj) ? Qtrue : Qfalse;
}
static VALUE
nil_to_i(VALUE obj)
{
return INT2FIX(0);
}
static VALUE
nil_to_f(VALUE obj)
{
return DBL2NUM(0.0);
}
static VALUE
nil_to_s(VALUE obj)
{
return rb_cNilClass_to_s;
}
static VALUE
nil_to_a(VALUE obj)
{
return rb_ary_new2(0);
}
static VALUE
nil_to_h(VALUE obj)
{
return rb_hash_new();
}
static VALUE
nil_inspect(VALUE obj)
{
return rb_usascii_str_new2("nil");
}
static VALUE
nil_match(VALUE obj1, VALUE obj2)
{
return Qnil;
}
static VALUE
true_to_s(VALUE obj)
{
return rb_cTrueClass_to_s;
}
static VALUE
true_and(VALUE obj, VALUE obj2)
{
return RTEST(obj2)?Qtrue:Qfalse;
}
static VALUE
true_or(VALUE obj, VALUE obj2)
{
return Qtrue;
}
static VALUE
true_xor(VALUE obj, VALUE obj2)
{
return RTEST(obj2)?Qfalse:Qtrue;
}
static VALUE
false_to_s(VALUE obj)
{
return rb_cFalseClass_to_s;
}
static VALUE
false_and(VALUE obj, VALUE obj2)
{
return Qfalse;
}
static VALUE
false_or(VALUE obj, VALUE obj2)
{
return RTEST(obj2)?Qtrue:Qfalse;
}
static VALUE
false_xor(VALUE obj, VALUE obj2)
{
return RTEST(obj2)?Qtrue:Qfalse;
}
static VALUE
rb_true(VALUE obj)
{
return Qtrue;
}
MJIT_FUNC_EXPORTED VALUE
rb_false(VALUE obj)
{
return Qfalse;
}
static VALUE
rb_obj_match(VALUE obj1, VALUE obj2)
{
if (rb_warning_category_enabled_p(RB_WARN_CATEGORY_DEPRECATED)) {
rb_warn("deprecated Object#=~ is called on %"PRIsVALUE
"; it always returns nil", rb_obj_class(obj1));
}
return Qnil;
}
static VALUE
rb_obj_not_match(VALUE obj1, VALUE obj2)
{
VALUE result = rb_funcall(obj1, id_match, 1, obj2);
return RTEST(result) ? Qfalse : Qtrue;
}
static VALUE
rb_obj_cmp(VALUE obj1, VALUE obj2)
{
if (obj1 == obj2 || rb_equal(obj1, obj2))
return INT2FIX(0);
return Qnil;
}
static VALUE
rb_mod_to_s(VALUE klass)
{
ID id_defined_at;
VALUE refined_class, defined_at;
if (FL_TEST(klass, FL_SINGLETON)) {
VALUE s = rb_usascii_str_new2("#<Class:");
VALUE v = rb_ivar_get(klass, id__attached__);
if (CLASS_OR_MODULE_P(v)) {
rb_str_append(s, rb_inspect(v));
}
else {
rb_str_append(s, rb_any_to_s(v));
}
rb_str_cat2(s, ">");
return s;
}
refined_class = rb_refinement_module_get_refined_class(klass);
if (!NIL_P(refined_class)) {
VALUE s = rb_usascii_str_new2("#<refinement:");
rb_str_concat(s, rb_inspect(refined_class));
rb_str_cat2(s, "@");
CONST_ID(id_defined_at, "__defined_at__");
defined_at = rb_attr_get(klass, id_defined_at);
rb_str_concat(s, rb_inspect(defined_at));
rb_str_cat2(s, ">");
return s;
}
return rb_class_name(klass);
}
static VALUE
rb_mod_freeze(VALUE mod)
{
rb_class_name(mod);
return rb_obj_freeze(mod);
}
static VALUE
rb_mod_eqq(VALUE mod, VALUE arg)
{
return rb_obj_is_kind_of(arg, mod);
}
VALUE
rb_class_inherited_p(VALUE mod, VALUE arg)
{
if (mod == arg) return Qtrue;
if (!CLASS_OR_MODULE_P(arg) && !RB_TYPE_P(arg, T_ICLASS)) {
rb_raise(rb_eTypeError, "compared with non class/module");
}
if (class_search_ancestor(mod, RCLASS_ORIGIN(arg))) {
return Qtrue;
}
if (class_search_ancestor(arg, mod)) {
return Qfalse;
}
return Qnil;
}
static VALUE
rb_mod_lt(VALUE mod, VALUE arg)
{
if (mod == arg) return Qfalse;
return rb_class_inherited_p(mod, arg);
}
static VALUE
rb_mod_ge(VALUE mod, VALUE arg)
{
if (!CLASS_OR_MODULE_P(arg)) {
rb_raise(rb_eTypeError, "compared with non class/module");
}
return rb_class_inherited_p(arg, mod);
}
static VALUE
rb_mod_gt(VALUE mod, VALUE arg)
{
if (mod == arg) return Qfalse;
return rb_mod_ge(mod, arg);
}
static VALUE
rb_mod_cmp(VALUE mod, VALUE arg)
{
VALUE cmp;
if (mod == arg) return INT2FIX(0);
if (!CLASS_OR_MODULE_P(arg)) {
return Qnil;
}
cmp = rb_class_inherited_p(mod, arg);
if (NIL_P(cmp)) return Qnil;
if (cmp) {
return INT2FIX(-1);
}
return INT2FIX(1);
}
static VALUE
rb_module_s_alloc(VALUE klass)
{
VALUE mod = rb_module_new();
RBASIC_SET_CLASS(mod, klass);
return mod;
}
static VALUE
rb_class_s_alloc(VALUE klass)
{
return rb_class_boot(0);
}
static VALUE
rb_mod_initialize(VALUE module)
{
if (rb_block_given_p()) {
rb_mod_module_exec(1, &module, module);
}
return Qnil;
}
static VALUE
rb_mod_initialize_clone(int argc, VALUE* argv, VALUE clone)
{
VALUE ret, orig, opts;
rb_scan_args(argc, argv, "1:", &orig, &opts);
ret = rb_obj_init_clone(argc, argv, clone);
if (OBJ_FROZEN(orig))
rb_class_name(clone);
return ret;
}
static VALUE
rb_class_initialize(int argc, VALUE *argv, VALUE klass)
{
VALUE super;
if (RCLASS_SUPER(klass) != 0 || klass == rb_cBasicObject) {
rb_raise(rb_eTypeError, "already initialized class");
}
if (rb_check_arity(argc, 0, 1) == 0) {
super = rb_cObject;
}
else {
super = argv[0];
rb_check_inheritable(super);
if (super != rb_cBasicObject && !RCLASS_SUPER(super)) {
rb_raise(rb_eTypeError, "can't inherit uninitialized class");
}
}
RCLASS_SET_SUPER(klass, super);
rb_make_metaclass(klass, RBASIC(super)->klass);
rb_class_inherited(super, klass);
rb_mod_initialize(klass);
return klass;
}
void
rb_undefined_alloc(VALUE klass)
{
rb_raise(rb_eTypeError, "allocator undefined for %"PRIsVALUE,
klass);
}
static rb_alloc_func_t class_get_alloc_func(VALUE klass);
static VALUE class_call_alloc_func(rb_alloc_func_t allocator, VALUE klass);
static VALUE
rb_class_alloc_m(VALUE klass)
{
rb_alloc_func_t allocator = class_get_alloc_func(klass);
if (!rb_obj_respond_to(klass, rb_intern("allocate"), 1)) {
rb_raise(rb_eTypeError, "calling %"PRIsVALUE".allocate is prohibited",
klass);
}
return class_call_alloc_func(allocator, klass);
}
static VALUE
rb_class_alloc(VALUE klass)
{
rb_alloc_func_t allocator = class_get_alloc_func(klass);
return class_call_alloc_func(allocator, klass);
}
static rb_alloc_func_t
class_get_alloc_func(VALUE klass)
{
rb_alloc_func_t allocator;
if (RCLASS_SUPER(klass) == 0 && klass != rb_cBasicObject) {
rb_raise(rb_eTypeError, "can't instantiate uninitialized class");
}
if (FL_TEST(klass, FL_SINGLETON)) {
rb_raise(rb_eTypeError, "can't create instance of singleton class");
}
allocator = rb_get_alloc_func(klass);
if (!allocator) {
rb_undefined_alloc(klass);
}
return allocator;
}
static VALUE
class_call_alloc_func(rb_alloc_func_t allocator, VALUE klass)
{
VALUE obj;
RUBY_DTRACE_CREATE_HOOK(OBJECT, rb_class2name(klass));
obj = (*allocator)(klass);
if (rb_obj_class(obj) != rb_class_real(klass)) {
rb_raise(rb_eTypeError, "wrong instance allocation");
}
return obj;
}
VALUE
rb_obj_alloc(VALUE klass)
{
Check_Type(klass, T_CLASS);
return rb_class_alloc(klass);
}
static VALUE
rb_class_allocate_instance(VALUE klass)
{
NEWOBJ_OF(obj, struct RObject, klass, T_OBJECT | (RGENGC_WB_PROTECTED_OBJECT ? FL_WB_PROTECTED : 0));
return (VALUE)obj;
}
static VALUE
rb_class_s_new(int argc, const VALUE *argv, VALUE klass)
{
VALUE obj;
obj = rb_class_alloc(klass);
rb_obj_call_init_kw(obj, argc, argv, RB_PASS_CALLED_KEYWORDS);
return obj;
}
VALUE
rb_class_new_instance_kw(int argc, const VALUE *argv, VALUE klass, int kw_splat)
{
VALUE obj;
Check_Type(klass, T_CLASS);
obj = rb_class_alloc(klass);
rb_obj_call_init_kw(obj, argc, argv, kw_splat);
return obj;
}
VALUE
rb_class_new_instance(int argc, const VALUE *argv, VALUE klass)
{
VALUE obj;
Check_Type(klass, T_CLASS);
obj = rb_class_alloc(klass);
rb_obj_call_init_kw(obj, argc, argv, RB_NO_KEYWORDS);
return obj;
}
VALUE
rb_class_superclass(VALUE klass)
{
VALUE super = RCLASS_SUPER(klass);
if (!super) {
if (klass == rb_cBasicObject) return Qnil;
rb_raise(rb_eTypeError, "uninitialized class");
}
while (RB_TYPE_P(super, T_ICLASS)) {
super = RCLASS_SUPER(super);
}
if (!super) {
return Qnil;
}
return super;
}
VALUE
rb_class_get_superclass(VALUE klass)
{
return RCLASS(klass)->super;
}
static const char bad_instance_name[] = "`%1$s' is not allowed as an instance variable name";
static const char bad_class_name[] = "`%1$s' is not allowed as a class variable name";
static const char bad_const_name[] = "wrong constant name %1$s";
static const char bad_attr_name[] = "invalid attribute name `%1$s'";
#define wrong_constant_name bad_const_name
#define id_for_var(obj, name, type) id_for_setter(obj, name, type, bad_##type##_name)
#define id_for_setter(obj, name, type, message) check_setter_id(obj, &(name), rb_is_##type##_id, rb_is_##type##_name, message, strlen(message))
static ID
check_setter_id(VALUE obj, VALUE *pname,
int (*valid_id_p)(ID), int (*valid_name_p)(VALUE),
const char *message, size_t message_len)
{
ID id = rb_check_id(pname);
VALUE name = *pname;
if (id ? !valid_id_p(id) : !valid_name_p(name)) {
rb_name_err_raise_str(rb_fstring_new(message, message_len),
obj, name);
}
return id;
}
static int
rb_is_attr_name(VALUE name)
{
return rb_is_local_name(name) || rb_is_const_name(name);
}
static int
rb_is_attr_id(ID id)
{
return rb_is_local_id(id) || rb_is_const_id(id);
}
static ID
id_for_attr(VALUE obj, VALUE name)
{
ID id = id_for_var(obj, name, attr);
if (!id) id = rb_intern_str(name);
return id;
}
static VALUE
rb_mod_attr_reader(int argc, VALUE *argv, VALUE klass)
{
int i;
for (i=0; i<argc; i++) {
rb_attr(klass, id_for_attr(klass, argv[i]), TRUE, FALSE, TRUE);
}
return Qnil;
}
VALUE
rb_mod_attr(int argc, VALUE *argv, VALUE klass)
{
if (argc == 2 && (argv[1] == Qtrue || argv[1] == Qfalse)) {
rb_warning("optional boolean argument is obsoleted");
rb_attr(klass, id_for_attr(klass, argv[0]), 1, RTEST(argv[1]), TRUE);
return Qnil;
}
return rb_mod_attr_reader(argc, argv, klass);
}
static VALUE
rb_mod_attr_writer(int argc, VALUE *argv, VALUE klass)
{
int i;
for (i=0; i<argc; i++) {
rb_attr(klass, id_for_attr(klass, argv[i]), FALSE, TRUE, TRUE);
}
return Qnil;
}
static VALUE
rb_mod_attr_accessor(int argc, VALUE *argv, VALUE klass)
{
int i;
for (i=0; i<argc; i++) {
rb_attr(klass, id_for_attr(klass, argv[i]), TRUE, TRUE, TRUE);
}
return Qnil;
}
static VALUE
rb_mod_const_get(int argc, VALUE *argv, VALUE mod)
{
VALUE name, recur;
rb_encoding *enc;
const char *pbeg, *p, *path, *pend;
ID id;
rb_check_arity(argc, 1, 2);
name = argv[0];
recur = (argc == 1) ? Qtrue : argv[1];
if (SYMBOL_P(name)) {
if (!rb_is_const_sym(name)) goto wrong_name;
id = rb_check_id(&name);
if (!id) return rb_const_missing(mod, name);
return RTEST(recur) ? rb_const_get(mod, id) : rb_const_get_at(mod, id);
}
path = StringValuePtr(name);
enc = rb_enc_get(name);
if (!rb_enc_asciicompat(enc)) {
rb_raise(rb_eArgError, "invalid class path encoding (non ASCII)");
}
pbeg = p = path;
pend = path + RSTRING_LEN(name);
if (p >= pend || !*p) {
wrong_name:
rb_name_err_raise(wrong_constant_name, mod, name);
}
if (p + 2 < pend && p[0] == ':' && p[1] == ':') {
mod = rb_cObject;
p += 2;
pbeg = p;
}
while (p < pend) {
VALUE part;
long len, beglen;
while (p < pend && *p != ':') p++;
if (pbeg == p) goto wrong_name;
id = rb_check_id_cstr(pbeg, len = p-pbeg, enc);
beglen = pbeg-path;
if (p < pend && p[0] == ':') {
if (p + 2 >= pend || p[1] != ':') goto wrong_name;
p += 2;
pbeg = p;
}
if (!RB_TYPE_P(mod, T_MODULE) && !RB_TYPE_P(mod, T_CLASS)) {
rb_raise(rb_eTypeError, "%"PRIsVALUE" does not refer to class/module",
QUOTE(name));
}
if (!id) {
part = rb_str_subseq(name, beglen, len);
OBJ_FREEZE(part);
if (!rb_is_const_name(part)) {
name = part;
goto wrong_name;
}
else if (!rb_method_basic_definition_p(CLASS_OF(mod), id_const_missing)) {
part = rb_str_intern(part);
mod = rb_const_missing(mod, part);
continue;
}
else {
rb_mod_const_missing(mod, part);
}
}
if (!rb_is_const_id(id)) {
name = ID2SYM(id);
goto wrong_name;
}
#if 0
mod = rb_const_get_0(mod, id, beglen > 0 || !RTEST(recur), RTEST(recur), FALSE);
#else
if (!RTEST(recur)) {
mod = rb_const_get_at(mod, id);
}
else if (beglen == 0) {
mod = rb_const_get(mod, id);
}
else {
mod = rb_const_get_from(mod, id);
}
#endif
}
return mod;
}
static VALUE
rb_mod_const_set(VALUE mod, VALUE name, VALUE value)
{
ID id = id_for_var(mod, name, const);
if (!id) id = rb_intern_str(name);
rb_const_set(mod, id, value);
return value;
}
static VALUE
rb_mod_const_defined(int argc, VALUE *argv, VALUE mod)
{
VALUE name, recur;
rb_encoding *enc;
const char *pbeg, *p, *path, *pend;
ID id;
rb_check_arity(argc, 1, 2);
name = argv[0];
recur = (argc == 1) ? Qtrue : argv[1];
if (SYMBOL_P(name)) {
if (!rb_is_const_sym(name)) goto wrong_name;
id = rb_check_id(&name);
if (!id) return Qfalse;
return RTEST(recur) ? rb_const_defined(mod, id) : rb_const_defined_at(mod, id);
}
path = StringValuePtr(name);
enc = rb_enc_get(name);
if (!rb_enc_asciicompat(enc)) {
rb_raise(rb_eArgError, "invalid class path encoding (non ASCII)");
}
pbeg = p = path;
pend = path + RSTRING_LEN(name);
if (p >= pend || !*p) {
wrong_name:
rb_name_err_raise(wrong_constant_name, mod, name);
}
if (p + 2 < pend && p[0] == ':' && p[1] == ':') {
mod = rb_cObject;
p += 2;
pbeg = p;
}
while (p < pend) {
VALUE part;
long len, beglen;
while (p < pend && *p != ':') p++;
if (pbeg == p) goto wrong_name;
id = rb_check_id_cstr(pbeg, len = p-pbeg, enc);
beglen = pbeg-path;
if (p < pend && p[0] == ':') {
if (p + 2 >= pend || p[1] != ':') goto wrong_name;
p += 2;
pbeg = p;
}
if (!id) {
part = rb_str_subseq(name, beglen, len);
OBJ_FREEZE(part);
if (!rb_is_const_name(part)) {
name = part;
goto wrong_name;
}
else {
return Qfalse;
}
}
if (!rb_is_const_id(id)) {
name = ID2SYM(id);
goto wrong_name;
}
#if 0
mod = rb_const_search(mod, id, beglen > 0 || !RTEST(recur), RTEST(recur), FALSE);
if (mod == Qundef) return Qfalse;
#else
if (!RTEST(recur)) {
if (!rb_const_defined_at(mod, id))
return Qfalse;
if (p == pend) return Qtrue;
mod = rb_const_get_at(mod, id);
}
else if (beglen == 0) {
if (!rb_const_defined(mod, id))
return Qfalse;
if (p == pend) return Qtrue;
mod = rb_const_get(mod, id);
}
else {
if (!rb_const_defined_from(mod, id))
return Qfalse;
if (p == pend) return Qtrue;
mod = rb_const_get_from(mod, id);
}
#endif
if (p < pend && !RB_TYPE_P(mod, T_MODULE) && !RB_TYPE_P(mod, T_CLASS)) {
rb_raise(rb_eTypeError, "%"PRIsVALUE" does not refer to class/module",
QUOTE(name));
}
}
return Qtrue;
}
static VALUE
rb_mod_const_source_location(int argc, VALUE *argv, VALUE mod)
{
VALUE name, recur, loc = Qnil;
rb_encoding *enc;
const char *pbeg, *p, *path, *pend;
ID id;
rb_check_arity(argc, 1, 2);
name = argv[0];
recur = (argc == 1) ? Qtrue : argv[1];
if (SYMBOL_P(name)) {
if (!rb_is_const_sym(name)) goto wrong_name;
id = rb_check_id(&name);
if (!id) return Qnil;
return RTEST(recur) ? rb_const_source_location(mod, id) : rb_const_source_location_at(mod, id);
}
path = StringValuePtr(name);
enc = rb_enc_get(name);
if (!rb_enc_asciicompat(enc)) {
rb_raise(rb_eArgError, "invalid class path encoding (non ASCII)");
}
pbeg = p = path;
pend = path + RSTRING_LEN(name);
if (p >= pend || !*p) {
wrong_name:
rb_name_err_raise(wrong_constant_name, mod, name);
}
if (p + 2 < pend && p[0] == ':' && p[1] == ':') {
mod = rb_cObject;
p += 2;
pbeg = p;
}
while (p < pend) {
VALUE part;
long len, beglen;
while (p < pend && *p != ':') p++;
if (pbeg == p) goto wrong_name;
id = rb_check_id_cstr(pbeg, len = p-pbeg, enc);
beglen = pbeg-path;
if (p < pend && p[0] == ':') {
if (p + 2 >= pend || p[1] != ':') goto wrong_name;
p += 2;
pbeg = p;
}
if (!id) {
part = rb_str_subseq(name, beglen, len);
OBJ_FREEZE(part);
if (!rb_is_const_name(part)) {
name = part;
goto wrong_name;
}
else {
return Qnil;
}
}
if (!rb_is_const_id(id)) {
name = ID2SYM(id);
goto wrong_name;
}
if (p < pend) {
if (RTEST(recur)) {
mod = rb_const_get(mod, id);
}
else {
mod = rb_const_get_at(mod, id);
}
if (!RB_TYPE_P(mod, T_MODULE) && !RB_TYPE_P(mod, T_CLASS)) {
rb_raise(rb_eTypeError, "%"PRIsVALUE" does not refer to class/module",
QUOTE(name));
}
}
else {
if (RTEST(recur)) {
loc = rb_const_source_location(mod, id);
}
else {
loc = rb_const_source_location_at(mod, id);
}
break;
}
recur = Qfalse;
}
return loc;
}
static VALUE
rb_obj_ivar_get(VALUE obj, VALUE iv)
{
ID id = id_for_var(obj, iv, instance);
if (!id) {
return Qnil;
}
return rb_ivar_get(obj, id);
}
static VALUE
rb_obj_ivar_set(VALUE obj, VALUE iv, VALUE val)
{
ID id = id_for_var(obj, iv, instance);
if (!id) id = rb_intern_str(iv);
return rb_ivar_set(obj, id, val);
}
static VALUE
rb_obj_ivar_defined(VALUE obj, VALUE iv)
{
ID id = id_for_var(obj, iv, instance);
if (!id) {
return Qfalse;
}
return rb_ivar_defined(obj, id);
}
static VALUE
rb_mod_cvar_get(VALUE obj, VALUE iv)
{
ID id = id_for_var(obj, iv, class);
if (!id) {
rb_name_err_raise("uninitialized class variable %1$s in %2$s",
obj, iv);
}
return rb_cvar_get(obj, id);
}
static VALUE
rb_mod_cvar_set(VALUE obj, VALUE iv, VALUE val)
{
ID id = id_for_var(obj, iv, class);
if (!id) id = rb_intern_str(iv);
rb_cvar_set(obj, id, val);
return val;
}
static VALUE
rb_mod_cvar_defined(VALUE obj, VALUE iv)
{
ID id = id_for_var(obj, iv, class);
if (!id) {
return Qfalse;
}
return rb_cvar_defined(obj, id);
}
static VALUE
rb_mod_singleton_p(VALUE klass)
{
if (RB_TYPE_P(klass, T_CLASS) && FL_TEST(klass, FL_SINGLETON))
return Qtrue;
return Qfalse;
}
static const struct conv_method_tbl {
const char method[6];
unsigned short id;
} conv_method_names[] = {
#define M(n) {#n, (unsigned short)idTo_##n}
M(int),
M(ary),
M(str),
M(sym),
M(hash),
M(proc),
M(io),
M(a),
M(s),
M(i),
M(r),
#undef M
};
#define IMPLICIT_CONVERSIONS 7
static int
conv_method_index(const char *method)
{
static const char prefix[] = "to_";
if (strncmp(prefix, method, sizeof(prefix)-1) == 0) {
const char *const meth = &method[sizeof(prefix)-1];
int i;
for (i=0; i < numberof(conv_method_names); i++) {
if (conv_method_names[i].method[0] == meth[0] &&
strcmp(conv_method_names[i].method, meth) == 0) {
return i;
}
}
}
return numberof(conv_method_names);
}
static VALUE
convert_type_with_id(VALUE val, const char *tname, ID method, int raise, int index)
{
VALUE r = rb_check_funcall(val, method, 0, 0);
if (r == Qundef) {
if (raise) {
const char *msg =
((index < 0 ? conv_method_index(rb_id2name(method)) : index)
< IMPLICIT_CONVERSIONS) ?
"no implicit conversion of" : "can't convert";
const char *cname = NIL_P(val) ? "nil" :
val == Qtrue ? "true" :
val == Qfalse ? "false" :
NULL;
if (cname)
rb_raise(rb_eTypeError, "%s %s into %s", msg, cname, tname);
rb_raise(rb_eTypeError, "%s %"PRIsVALUE" into %s", msg,
rb_obj_class(val),
tname);
}
return Qnil;
}
return r;
}
static VALUE
convert_type(VALUE val, const char *tname, const char *method, int raise)
{
int i = conv_method_index(method);
ID m = i < numberof(conv_method_names) ?
conv_method_names[i].id : rb_intern(method);
return convert_type_with_id(val, tname, m, raise, i);
}
NORETURN(static void conversion_mismatch(VALUE, const char *, const char *, VALUE));
static void
conversion_mismatch(VALUE val, const char *tname, const char *method, VALUE result)
{
VALUE cname = rb_obj_class(val);
rb_raise(rb_eTypeError,
"can't convert %"PRIsVALUE" to %s (%"PRIsVALUE"#%s gives %"PRIsVALUE")",
cname, tname, cname, method, rb_obj_class(result));
}
VALUE
rb_convert_type(VALUE val, int type, const char *tname, const char *method)
{
VALUE v;
if (TYPE(val) == type) return val;
v = convert_type(val, tname, method, TRUE);
if (TYPE(v) != type) {
conversion_mismatch(val, tname, method, v);
}
return v;
}
VALUE
rb_convert_type_with_id(VALUE val, int type, const char *tname, ID method)
{
VALUE v;
if (TYPE(val) == type) return val;
v = convert_type_with_id(val, tname, method, TRUE, -1);
if (TYPE(v) != type) {
conversion_mismatch(val, tname, RSTRING_PTR(rb_id2str(method)), v);
}
return v;
}
VALUE
rb_check_convert_type(VALUE val, int type, const char *tname, const char *method)
{
VALUE v;
if (TYPE(val) == type && type != T_DATA) return val;
v = convert_type(val, tname, method, FALSE);
if (NIL_P(v)) return Qnil;
if (TYPE(v) != type) {
conversion_mismatch(val, tname, method, v);
}
return v;
}
MJIT_FUNC_EXPORTED VALUE
rb_check_convert_type_with_id(VALUE val, int type, const char *tname, ID method)
{
VALUE v;
if (TYPE(val) == type && type != T_DATA) return val;
v = convert_type_with_id(val, tname, method, FALSE, -1);
if (NIL_P(v)) return Qnil;
if (TYPE(v) != type) {
conversion_mismatch(val, tname, RSTRING_PTR(rb_id2str(method)), v);
}
return v;
}
#define try_to_int(val, mid, raise) convert_type_with_id(val, "Integer", mid, raise, -1)
ALWAYS_INLINE(static VALUE rb_to_integer(VALUE val, const char *method, ID mid));
static inline VALUE
rb_to_integer(VALUE val, const char *method, ID mid)
{
VALUE v;
if (RB_INTEGER_TYPE_P(val)) return val;
v = try_to_int(val, mid, TRUE);
if (!RB_INTEGER_TYPE_P(v)) {
conversion_mismatch(val, "Integer", method, v);
}
return v;
}
VALUE
rb_check_to_integer(VALUE val, const char *method)
{
VALUE v;
if (FIXNUM_P(val)) return val;
if (RB_TYPE_P(val, T_BIGNUM)) return val;
v = convert_type(val, "Integer", method, FALSE);
if (!RB_INTEGER_TYPE_P(v)) {
return Qnil;
}
return v;
}
VALUE
rb_to_int(VALUE val)
{
return rb_to_integer(val, "to_int", idTo_int);
}
VALUE
rb_check_to_int(VALUE val)
{
if (RB_INTEGER_TYPE_P(val)) return val;
val = try_to_int(val, idTo_int, FALSE);
if (RB_INTEGER_TYPE_P(val)) return val;
return Qnil;
}
static VALUE
rb_check_to_i(VALUE val)
{
if (RB_INTEGER_TYPE_P(val)) return val;
val = try_to_int(val, idTo_i, FALSE);
if (RB_INTEGER_TYPE_P(val)) return val;
return Qnil;
}
static VALUE
rb_convert_to_integer(VALUE val, int base, int raise_exception)
{
VALUE tmp;
if (RB_FLOAT_TYPE_P(val)) {
double f;
if (base != 0) goto arg_error;
f = RFLOAT_VALUE(val);
if (!raise_exception && !isfinite(f)) return Qnil;
if (FIXABLE(f)) return LONG2FIX((long)f);
return rb_dbl2big(f);
}
else if (RB_INTEGER_TYPE_P(val)) {
if (base != 0) goto arg_error;
return val;
}
else if (RB_TYPE_P(val, T_STRING)) {
return rb_str_convert_to_inum(val, base, TRUE, raise_exception);
}
else if (NIL_P(val)) {
if (base != 0) goto arg_error;
if (!raise_exception) return Qnil;
rb_raise(rb_eTypeError, "can't convert nil into Integer");
}
if (base != 0) {
tmp = rb_check_string_type(val);
if (!NIL_P(tmp)) return rb_str_convert_to_inum(tmp, base, TRUE, raise_exception);
arg_error:
if (!raise_exception) return Qnil;
rb_raise(rb_eArgError, "base specified for non string value");
}
tmp = rb_protect(rb_check_to_int, val, NULL);
if (RB_INTEGER_TYPE_P(tmp)) return tmp;
rb_set_errinfo(Qnil);
if (!raise_exception) {
VALUE result = rb_protect(rb_check_to_i, val, NULL);
rb_set_errinfo(Qnil);
return result;
}
return rb_to_integer(val, "to_i", idTo_i);
}
VALUE
rb_Integer(VALUE val)
{
return rb_convert_to_integer(val, 0, TRUE);
}
int
rb_bool_expected(VALUE obj, const char *flagname)
{
switch (obj) {
case Qtrue: case Qfalse:
break;
default:
rb_raise(rb_eArgError, "true or false is expected as %s: %+"PRIsVALUE,
flagname, obj);
}
return obj != Qfalse;
}
int
rb_opts_exception_p(VALUE opts, int default_value)
{
static ID kwds[1] = {idException};
VALUE exception;
if (rb_get_kwargs(opts, kwds, 0, 1, &exception))
return rb_bool_expected(exception, "exception");
return default_value;
}
#define opts_exception_p(opts) rb_opts_exception_p((opts), TRUE)
static VALUE
rb_f_integer(int argc, VALUE *argv, VALUE obj)
{
VALUE arg = Qnil, opts = Qnil;
int base = 0;
if (argc > 1) {
int narg = 1;
VALUE vbase = rb_check_to_int(argv[1]);
if (!NIL_P(vbase)) {
base = NUM2INT(vbase);
narg = 2;
}
if (argc > narg) {
VALUE hash = rb_check_hash_type(argv[argc-1]);
if (!NIL_P(hash)) {
opts = rb_extract_keywords(&hash);
if (!hash) --argc;
}
}
}
rb_check_arity(argc, 1, 2);
arg = argv[0];
return rb_convert_to_integer(arg, base, opts_exception_p(opts));
}
static double
rb_cstr_to_dbl_raise(const char *p, int badcheck, int raise, int *error)
{
const char *q;
char *end;
double d;
const char *ellipsis = "";
int w;
enum {max_width = 20};
#define OutOfRange() ((end - p > max_width) ? (w = max_width, ellipsis = "...") : (w = (int)(end - p), ellipsis = ""))
if (!p) return 0.0;
q = p;
while (ISSPACE(*p)) p++;
if (!badcheck && p[0] == '0' && (p[1] == 'x' || p[1] == 'X')) {
return 0.0;
}
d = strtod(p, &end);
if (errno == ERANGE) {
OutOfRange();
rb_warning("Float %.*s%s out of range", w, p, ellipsis);
errno = 0;
}
if (p == end) {
if (badcheck) {
bad:
if (raise)
rb_invalid_str(q, "Float()");
else {
if (error) *error = 1;
return 0.0;
}
}
return d;
}
if (*end) {
char buf[DBL_DIG * 4 + 10];
char *n = buf;
char *const init_e = buf + DBL_DIG * 4;
char *e = init_e;
char prev = 0;
int dot_seen = FALSE;
switch (*p) {case '+': case '-': prev = *n++ = *p++;}
if (*p == '0') {
prev = *n++ = '0';
while (*++p == '0');
}
while (p < end && n < e) prev = *n++ = *p++;
while (*p) {
if (*p == '_') {
if (n == buf || !ISDIGIT(prev) || (++p, !ISDIGIT(*p))) {
if (badcheck) goto bad;
break;
}
}
prev = *p++;
if (e == init_e && (prev == 'e' || prev == 'E' || prev == 'p' || prev == 'P')) {
e = buf + sizeof(buf) - 1;
*n++ = prev;
switch (*p) {case '+': case '-': prev = *n++ = *p++;}
if (*p == '0') {
prev = *n++ = '0';
while (*++p == '0');
}
continue;
}
else if (ISSPACE(prev)) {
while (ISSPACE(*p)) ++p;
if (*p) {
if (badcheck) goto bad;
break;
}
}
else if (prev == '.' ? dot_seen++ : !ISDIGIT(prev)) {
if (badcheck) goto bad;
break;
}
if (n < e) *n++ = prev;
}
*n = '\0';
p = buf;
if (!badcheck && p[0] == '0' && (p[1] == 'x' || p[1] == 'X')) {
return 0.0;
}
d = strtod(p, &end);
if (errno == ERANGE) {
OutOfRange();
rb_warning("Float %.*s%s out of range", w, p, ellipsis);
errno = 0;
}
if (badcheck) {
if (!end || p == end) goto bad;
while (*end && ISSPACE(*end)) end++;
if (*end) goto bad;
}
}
if (errno == ERANGE) {
errno = 0;
OutOfRange();
rb_raise(rb_eArgError, "Float %.*s%s out of range", w, q, ellipsis);
}
return d;
}
double
rb_cstr_to_dbl(const char *p, int badcheck)
{
return rb_cstr_to_dbl_raise(p, badcheck, TRUE, NULL);
}
static double
rb_str_to_dbl_raise(VALUE str, int badcheck, int raise, int *error)
{
char *s;
long len;
double ret;
VALUE v = 0;
StringValue(str);
s = RSTRING_PTR(str);
len = RSTRING_LEN(str);
if (s) {
if (badcheck && memchr(s, '\0', len)) {
if (raise)
rb_raise(rb_eArgError, "string for Float contains null byte");
else {
if (error) *error = 1;
return 0.0;
}
}
if (s[len]) { 
char *p = ALLOCV(v, (size_t)len + 1);
MEMCPY(p, s, char, len);
p[len] = '\0';
s = p;
}
}
ret = rb_cstr_to_dbl_raise(s, badcheck, raise, error);
if (v)
ALLOCV_END(v);
return ret;
}
FUNC_MINIMIZED(double rb_str_to_dbl(VALUE str, int badcheck));
double
rb_str_to_dbl(VALUE str, int badcheck)
{
return rb_str_to_dbl_raise(str, badcheck, TRUE, NULL);
}
#define fix2dbl_without_to_f(x) (double)FIX2LONG(x)
#define big2dbl_without_to_f(x) rb_big2dbl(x)
#define int2dbl_without_to_f(x) (FIXNUM_P(x) ? fix2dbl_without_to_f(x) : big2dbl_without_to_f(x))
#define num2dbl_without_to_f(x) (FIXNUM_P(x) ? fix2dbl_without_to_f(x) : RB_TYPE_P(x, T_BIGNUM) ? big2dbl_without_to_f(x) : (Check_Type(x, T_FLOAT), RFLOAT_VALUE(x)))
static inline double
rat2dbl_without_to_f(VALUE x)
{
VALUE num = rb_rational_num(x);
VALUE den = rb_rational_den(x);
return num2dbl_without_to_f(num) / num2dbl_without_to_f(den);
}
#define special_const_to_float(val, pre, post) switch (val) { case Qnil: rb_raise_static(rb_eTypeError, pre "nil" post); case Qtrue: rb_raise_static(rb_eTypeError, pre "true" post); case Qfalse: rb_raise_static(rb_eTypeError, pre "false" post); }
static inline void
conversion_to_float(VALUE val)
{
special_const_to_float(val, "can't convert ", " into Float");
}
static inline void
implicit_conversion_to_float(VALUE val)
{
special_const_to_float(val, "no implicit conversion to float from ", "");
}
static int
to_float(VALUE *valp, int raise_exception)
{
VALUE val = *valp;
if (SPECIAL_CONST_P(val)) {
if (FIXNUM_P(val)) {
*valp = DBL2NUM(fix2dbl_without_to_f(val));
return T_FLOAT;
}
else if (FLONUM_P(val)) {
return T_FLOAT;
}
else if (raise_exception) {
conversion_to_float(val);
}
}
else {
int type = BUILTIN_TYPE(val);
switch (type) {
case T_FLOAT:
return T_FLOAT;
case T_BIGNUM:
*valp = DBL2NUM(big2dbl_without_to_f(val));
return T_FLOAT;
case T_RATIONAL:
*valp = DBL2NUM(rat2dbl_without_to_f(val));
return T_FLOAT;
case T_STRING:
return T_STRING;
}
}
return T_NONE;
}
static VALUE
convert_type_to_float_protected(VALUE val)
{
return rb_convert_type_with_id(val, T_FLOAT, "Float", id_to_f);
}
static VALUE
rb_convert_to_float(VALUE val, int raise_exception)
{
switch (to_float(&val, raise_exception)) {
case T_FLOAT:
return val;
case T_STRING:
if (!raise_exception) {
int e = 0;
double x = rb_str_to_dbl_raise(val, TRUE, raise_exception, &e);
return e ? Qnil : DBL2NUM(x);
}
return DBL2NUM(rb_str_to_dbl(val, TRUE));
case T_NONE:
if (SPECIAL_CONST_P(val) && !raise_exception)
return Qnil;
}
if (!raise_exception) {
int state;
VALUE result = rb_protect(convert_type_to_float_protected, val, &state);
if (state) rb_set_errinfo(Qnil);
return result;
}
return rb_convert_type_with_id(val, T_FLOAT, "Float", id_to_f);
}
FUNC_MINIMIZED(VALUE rb_Float(VALUE val));
VALUE
rb_Float(VALUE val)
{
return rb_convert_to_float(val, TRUE);
}
static VALUE
rb_f_float(int argc, VALUE *argv, VALUE obj)
{
VALUE arg = Qnil, opts = Qnil;
rb_scan_args(argc, argv, "1:", &arg, &opts);
return rb_convert_to_float(arg, opts_exception_p(opts));
}
static VALUE
numeric_to_float(VALUE val)
{
if (!rb_obj_is_kind_of(val, rb_cNumeric)) {
rb_raise(rb_eTypeError, "can't convert %"PRIsVALUE" into Float",
rb_obj_class(val));
}
return rb_convert_type_with_id(val, T_FLOAT, "Float", id_to_f);
}
VALUE
rb_to_float(VALUE val)
{
switch (to_float(&val, TRUE)) {
case T_FLOAT:
return val;
}
return numeric_to_float(val);
}
VALUE
rb_check_to_float(VALUE val)
{
if (RB_TYPE_P(val, T_FLOAT)) return val;
if (!rb_obj_is_kind_of(val, rb_cNumeric)) {
return Qnil;
}
return rb_check_convert_type_with_id(val, T_FLOAT, "Float", id_to_f);
}
static inline int
basic_to_f_p(VALUE klass)
{
return rb_method_basic_definition_p(klass, id_to_f);
}
double
rb_num_to_dbl(VALUE val)
{
if (SPECIAL_CONST_P(val)) {
if (FIXNUM_P(val)) {
if (basic_to_f_p(rb_cInteger))
return fix2dbl_without_to_f(val);
}
else if (FLONUM_P(val)) {
return rb_float_flonum_value(val);
}
else {
conversion_to_float(val);
}
}
else {
switch (BUILTIN_TYPE(val)) {
case T_FLOAT:
return rb_float_noflonum_value(val);
case T_BIGNUM:
if (basic_to_f_p(rb_cInteger))
return big2dbl_without_to_f(val);
break;
case T_RATIONAL:
if (basic_to_f_p(rb_cRational))
return rat2dbl_without_to_f(val);
break;
}
}
val = numeric_to_float(val);
return RFLOAT_VALUE(val);
}
double
rb_num2dbl(VALUE val)
{
if (SPECIAL_CONST_P(val)) {
if (FIXNUM_P(val)) {
return fix2dbl_without_to_f(val);
}
else if (FLONUM_P(val)) {
return rb_float_flonum_value(val);
}
else {
implicit_conversion_to_float(val);
}
}
else {
switch (BUILTIN_TYPE(val)) {
case T_FLOAT:
return rb_float_noflonum_value(val);
case T_BIGNUM:
return big2dbl_without_to_f(val);
case T_RATIONAL:
return rat2dbl_without_to_f(val);
case T_STRING:
rb_raise(rb_eTypeError, "no implicit conversion to float from string");
}
}
val = rb_convert_type_with_id(val, T_FLOAT, "Float", id_to_f);
return RFLOAT_VALUE(val);
}
VALUE
rb_String(VALUE val)
{
VALUE tmp = rb_check_string_type(val);
if (NIL_P(tmp))
tmp = rb_convert_type_with_id(val, T_STRING, "String", idTo_s);
return tmp;
}
static VALUE
rb_f_string(VALUE obj, VALUE arg)
{
return rb_String(arg);
}
VALUE
rb_Array(VALUE val)
{
VALUE tmp = rb_check_array_type(val);
if (NIL_P(tmp)) {
tmp = rb_check_to_array(val);
if (NIL_P(tmp)) {
return rb_ary_new3(1, val);
}
}
return tmp;
}
static VALUE
rb_f_array(VALUE obj, VALUE arg)
{
return rb_Array(arg);
}
VALUE
rb_Hash(VALUE val)
{
VALUE tmp;
if (NIL_P(val)) return rb_hash_new();
tmp = rb_check_hash_type(val);
if (NIL_P(tmp)) {
if (RB_TYPE_P(val, T_ARRAY) && RARRAY_LEN(val) == 0)
return rb_hash_new();
rb_raise(rb_eTypeError, "can't convert %s into Hash", rb_obj_classname(val));
}
return tmp;
}
static VALUE
rb_f_hash(VALUE obj, VALUE arg)
{
return rb_Hash(arg);
}
struct dig_method {
VALUE klass;
int basic;
};
static ID id_dig;
static int
dig_basic_p(VALUE obj, struct dig_method *cache)
{
VALUE klass = RBASIC_CLASS(obj);
if (klass != cache->klass) {
cache->klass = klass;
cache->basic = rb_method_basic_definition_p(klass, id_dig);
}
return cache->basic;
}
static void
no_dig_method(int found, VALUE recv, ID mid, int argc, const VALUE *argv, VALUE data)
{
if (!found) {
rb_raise(rb_eTypeError, "%"PRIsVALUE" does not have #dig method",
CLASS_OF(data));
}
}
VALUE
rb_obj_dig(int argc, VALUE *argv, VALUE obj, VALUE notfound)
{
struct dig_method hash = {Qnil}, ary = {Qnil}, strt = {Qnil};
for (; argc > 0; ++argv, --argc) {
if (NIL_P(obj)) return notfound;
if (!SPECIAL_CONST_P(obj)) {
switch (BUILTIN_TYPE(obj)) {
case T_HASH:
if (dig_basic_p(obj, &hash)) {
obj = rb_hash_aref(obj, *argv);
continue;
}
break;
case T_ARRAY:
if (dig_basic_p(obj, &ary)) {
obj = rb_ary_at(obj, *argv);
continue;
}
break;
case T_STRUCT:
if (dig_basic_p(obj, &strt)) {
obj = rb_struct_lookup(obj, *argv);
continue;
}
break;
}
}
return rb_check_funcall_with_hook_kw(obj, id_dig, argc, argv,
no_dig_method, obj,
RB_NO_KEYWORDS);
}
return obj;
}
static VALUE
f_sprintf(int c, const VALUE *v, VALUE _)
{
return rb_f_sprintf(c, v);
}
void
InitVM_Object(void)
{
Init_class_hierarchy();
#if 0
rb_cBasicObject = rb_define_class("BasicObject", Qnil);
rb_cObject = rb_define_class("Object", rb_cBasicObject);
rb_cModule = rb_define_class("Module", rb_cObject);
rb_cClass = rb_define_class("Class", rb_cModule);
#endif
#undef rb_intern
#define rb_intern(str) rb_intern_const(str)
rb_define_private_method(rb_cBasicObject, "initialize", rb_obj_dummy0, 0);
rb_define_alloc_func(rb_cBasicObject, rb_class_allocate_instance);
rb_define_method(rb_cBasicObject, "==", rb_obj_equal, 1);
rb_define_method(rb_cBasicObject, "equal?", rb_obj_equal, 1);
rb_define_method(rb_cBasicObject, "!", rb_obj_not, 0);
rb_define_method(rb_cBasicObject, "!=", rb_obj_not_equal, 1);
rb_define_private_method(rb_cBasicObject, "singleton_method_added", rb_obj_dummy1, 1);
rb_define_private_method(rb_cBasicObject, "singleton_method_removed", rb_obj_dummy1, 1);
rb_define_private_method(rb_cBasicObject, "singleton_method_undefined", rb_obj_dummy1, 1);
rb_mKernel = rb_define_module("Kernel");
rb_include_module(rb_cObject, rb_mKernel);
rb_define_private_method(rb_cClass, "inherited", rb_obj_dummy1, 1);
rb_define_private_method(rb_cModule, "included", rb_obj_dummy1, 1);
rb_define_private_method(rb_cModule, "extended", rb_obj_dummy1, 1);
rb_define_private_method(rb_cModule, "prepended", rb_obj_dummy1, 1);
rb_define_private_method(rb_cModule, "method_added", rb_obj_dummy1, 1);
rb_define_private_method(rb_cModule, "method_removed", rb_obj_dummy1, 1);
rb_define_private_method(rb_cModule, "method_undefined", rb_obj_dummy1, 1);
rb_define_method(rb_mKernel, "nil?", rb_false, 0);
rb_define_method(rb_mKernel, "===", rb_equal, 1);
rb_define_method(rb_mKernel, "=~", rb_obj_match, 1);
rb_define_method(rb_mKernel, "!~", rb_obj_not_match, 1);
rb_define_method(rb_mKernel, "eql?", rb_obj_equal, 1);
rb_define_method(rb_mKernel, "hash", rb_obj_hash, 0); 
rb_define_method(rb_mKernel, "<=>", rb_obj_cmp, 1);
rb_define_method(rb_mKernel, "class", rb_obj_class, 0);
rb_define_method(rb_mKernel, "singleton_class", rb_obj_singleton_class, 0);
rb_define_method(rb_mKernel, "dup", rb_obj_dup, 0);
rb_define_method(rb_mKernel, "itself", rb_obj_itself, 0);
rb_define_method(rb_mKernel, "yield_self", rb_obj_yield_self, 0);
rb_define_method(rb_mKernel, "then", rb_obj_yield_self, 0);
rb_define_method(rb_mKernel, "initialize_copy", rb_obj_init_copy, 1);
rb_define_method(rb_mKernel, "initialize_dup", rb_obj_init_dup_clone, 1);
rb_define_method(rb_mKernel, "initialize_clone", rb_obj_init_clone, -1);
rb_define_method(rb_mKernel, "taint", rb_obj_taint, 0);
rb_define_method(rb_mKernel, "tainted?", rb_obj_tainted, 0);
rb_define_method(rb_mKernel, "untaint", rb_obj_untaint, 0);
rb_define_method(rb_mKernel, "untrust", rb_obj_untrust, 0);
rb_define_method(rb_mKernel, "untrusted?", rb_obj_untrusted, 0);
rb_define_method(rb_mKernel, "trust", rb_obj_trust, 0);
rb_define_method(rb_mKernel, "freeze", rb_obj_freeze, 0);
rb_define_method(rb_mKernel, "frozen?", rb_obj_frozen_p, 0);
rb_define_method(rb_mKernel, "to_s", rb_any_to_s, 0);
rb_define_method(rb_mKernel, "inspect", rb_obj_inspect, 0);
rb_define_method(rb_mKernel, "methods", rb_obj_methods, -1); 
rb_define_method(rb_mKernel, "singleton_methods", rb_obj_singleton_methods, -1); 
rb_define_method(rb_mKernel, "protected_methods", rb_obj_protected_methods, -1); 
rb_define_method(rb_mKernel, "private_methods", rb_obj_private_methods, -1); 
rb_define_method(rb_mKernel, "public_methods", rb_obj_public_methods, -1); 
rb_define_method(rb_mKernel, "instance_variables", rb_obj_instance_variables, 0); 
rb_define_method(rb_mKernel, "instance_variable_get", rb_obj_ivar_get, 1);
rb_define_method(rb_mKernel, "instance_variable_set", rb_obj_ivar_set, 2);
rb_define_method(rb_mKernel, "instance_variable_defined?", rb_obj_ivar_defined, 1);
rb_define_method(rb_mKernel, "remove_instance_variable",
rb_obj_remove_instance_variable, 1); 
rb_define_method(rb_mKernel, "instance_of?", rb_obj_is_instance_of, 1);
rb_define_method(rb_mKernel, "kind_of?", rb_obj_is_kind_of, 1);
rb_define_method(rb_mKernel, "is_a?", rb_obj_is_kind_of, 1);
rb_define_method(rb_mKernel, "tap", rb_obj_tap, 0);
rb_define_global_function("sprintf", f_sprintf, -1);
rb_define_global_function("format", f_sprintf, -1);
rb_define_global_function("Integer", rb_f_integer, -1);
rb_define_global_function("Float", rb_f_float, -1);
rb_define_global_function("String", rb_f_string, 1);
rb_define_global_function("Array", rb_f_array, 1);
rb_define_global_function("Hash", rb_f_hash, 1);
rb_cNilClass = rb_define_class("NilClass", rb_cObject);
rb_cNilClass_to_s = rb_fstring_enc_lit("", rb_usascii_encoding());
rb_gc_register_mark_object(rb_cNilClass_to_s);
rb_define_method(rb_cNilClass, "to_i", nil_to_i, 0);
rb_define_method(rb_cNilClass, "to_f", nil_to_f, 0);
rb_define_method(rb_cNilClass, "to_s", nil_to_s, 0);
rb_define_method(rb_cNilClass, "to_a", nil_to_a, 0);
rb_define_method(rb_cNilClass, "to_h", nil_to_h, 0);
rb_define_method(rb_cNilClass, "inspect", nil_inspect, 0);
rb_define_method(rb_cNilClass, "=~", nil_match, 1);
rb_define_method(rb_cNilClass, "&", false_and, 1);
rb_define_method(rb_cNilClass, "|", false_or, 1);
rb_define_method(rb_cNilClass, "^", false_xor, 1);
rb_define_method(rb_cNilClass, "===", rb_equal, 1);
rb_define_method(rb_cNilClass, "nil?", rb_true, 0);
rb_undef_alloc_func(rb_cNilClass);
rb_undef_method(CLASS_OF(rb_cNilClass), "new");
rb_define_global_const("NIL", Qnil);
rb_deprecate_constant(rb_cObject, "NIL");
rb_define_method(rb_cModule, "freeze", rb_mod_freeze, 0);
rb_define_method(rb_cModule, "===", rb_mod_eqq, 1);
rb_define_method(rb_cModule, "==", rb_obj_equal, 1);
rb_define_method(rb_cModule, "<=>", rb_mod_cmp, 1);
rb_define_method(rb_cModule, "<", rb_mod_lt, 1);
rb_define_method(rb_cModule, "<=", rb_class_inherited_p, 1);
rb_define_method(rb_cModule, ">", rb_mod_gt, 1);
rb_define_method(rb_cModule, ">=", rb_mod_ge, 1);
rb_define_method(rb_cModule, "initialize_copy", rb_mod_init_copy, 1); 
rb_define_method(rb_cModule, "to_s", rb_mod_to_s, 0);
rb_define_alias(rb_cModule, "inspect", "to_s");
rb_define_method(rb_cModule, "included_modules", rb_mod_included_modules, 0); 
rb_define_method(rb_cModule, "include?", rb_mod_include_p, 1); 
rb_define_method(rb_cModule, "name", rb_mod_name, 0); 
rb_define_method(rb_cModule, "ancestors", rb_mod_ancestors, 0); 
rb_define_method(rb_cModule, "attr", rb_mod_attr, -1);
rb_define_method(rb_cModule, "attr_reader", rb_mod_attr_reader, -1);
rb_define_method(rb_cModule, "attr_writer", rb_mod_attr_writer, -1);
rb_define_method(rb_cModule, "attr_accessor", rb_mod_attr_accessor, -1);
rb_define_alloc_func(rb_cModule, rb_module_s_alloc);
rb_define_method(rb_cModule, "initialize", rb_mod_initialize, 0);
rb_define_method(rb_cModule, "initialize_clone", rb_mod_initialize_clone, -1);
rb_define_method(rb_cModule, "instance_methods", rb_class_instance_methods, -1); 
rb_define_method(rb_cModule, "public_instance_methods",
rb_class_public_instance_methods, -1); 
rb_define_method(rb_cModule, "protected_instance_methods",
rb_class_protected_instance_methods, -1); 
rb_define_method(rb_cModule, "private_instance_methods",
rb_class_private_instance_methods, -1); 
rb_define_method(rb_cModule, "constants", rb_mod_constants, -1); 
rb_define_method(rb_cModule, "const_get", rb_mod_const_get, -1);
rb_define_method(rb_cModule, "const_set", rb_mod_const_set, 2);
rb_define_method(rb_cModule, "const_defined?", rb_mod_const_defined, -1);
rb_define_method(rb_cModule, "const_source_location", rb_mod_const_source_location, -1);
rb_define_private_method(rb_cModule, "remove_const",
rb_mod_remove_const, 1); 
rb_define_method(rb_cModule, "const_missing",
rb_mod_const_missing, 1); 
rb_define_method(rb_cModule, "class_variables",
rb_mod_class_variables, -1); 
rb_define_method(rb_cModule, "remove_class_variable",
rb_mod_remove_cvar, 1); 
rb_define_method(rb_cModule, "class_variable_get", rb_mod_cvar_get, 1);
rb_define_method(rb_cModule, "class_variable_set", rb_mod_cvar_set, 2);
rb_define_method(rb_cModule, "class_variable_defined?", rb_mod_cvar_defined, 1);
rb_define_method(rb_cModule, "public_constant", rb_mod_public_constant, -1); 
rb_define_method(rb_cModule, "private_constant", rb_mod_private_constant, -1); 
rb_define_method(rb_cModule, "deprecate_constant", rb_mod_deprecate_constant, -1); 
rb_define_method(rb_cModule, "singleton_class?", rb_mod_singleton_p, 0);
rb_define_method(rb_cClass, "allocate", rb_class_alloc_m, 0);
rb_define_method(rb_cClass, "new", rb_class_s_new, -1);
rb_define_method(rb_cClass, "initialize", rb_class_initialize, -1);
rb_define_method(rb_cClass, "superclass", rb_class_superclass, 0);
rb_define_alloc_func(rb_cClass, rb_class_s_alloc);
rb_undef_method(rb_cClass, "extend_object");
rb_undef_method(rb_cClass, "append_features");
rb_undef_method(rb_cClass, "prepend_features");
rb_cData = rb_define_class("Data", rb_cObject);
rb_undef_alloc_func(rb_cData);
rb_deprecate_constant(rb_cObject, "Data");
rb_cTrueClass = rb_define_class("TrueClass", rb_cObject);
rb_cTrueClass_to_s = rb_fstring_enc_lit("true", rb_usascii_encoding());
rb_gc_register_mark_object(rb_cTrueClass_to_s);
rb_define_method(rb_cTrueClass, "to_s", true_to_s, 0);
rb_define_alias(rb_cTrueClass, "inspect", "to_s");
rb_define_method(rb_cTrueClass, "&", true_and, 1);
rb_define_method(rb_cTrueClass, "|", true_or, 1);
rb_define_method(rb_cTrueClass, "^", true_xor, 1);
rb_define_method(rb_cTrueClass, "===", rb_equal, 1);
rb_undef_alloc_func(rb_cTrueClass);
rb_undef_method(CLASS_OF(rb_cTrueClass), "new");
rb_define_global_const("TRUE", Qtrue);
rb_deprecate_constant(rb_cObject, "TRUE");
rb_cFalseClass = rb_define_class("FalseClass", rb_cObject);
rb_cFalseClass_to_s = rb_fstring_enc_lit("false", rb_usascii_encoding());
rb_gc_register_mark_object(rb_cFalseClass_to_s);
rb_define_method(rb_cFalseClass, "to_s", false_to_s, 0);
rb_define_alias(rb_cFalseClass, "inspect", "to_s");
rb_define_method(rb_cFalseClass, "&", false_and, 1);
rb_define_method(rb_cFalseClass, "|", false_or, 1);
rb_define_method(rb_cFalseClass, "^", false_xor, 1);
rb_define_method(rb_cFalseClass, "===", rb_equal, 1);
rb_undef_alloc_func(rb_cFalseClass);
rb_undef_method(CLASS_OF(rb_cFalseClass), "new");
rb_define_global_const("FALSE", Qfalse);
rb_deprecate_constant(rb_cObject, "FALSE");
}
#include "kernel.rbinc"
void
Init_Object(void)
{
id_dig = rb_intern_const("dig");
InitVM(Object);
}
