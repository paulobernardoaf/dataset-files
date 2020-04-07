#include "gc.h"
#include "internal.h"
#include "internal/class.h"
#include "internal/compilers.h"
#include "internal/hash.h"
#include "internal/imemo.h"
#include "node.h"
#include "ruby/io.h"
#include "ruby/re.h"
#include "ruby/st.h"
#include "symbol.h"
static VALUE
memsize_of_m(VALUE self, VALUE obj)
{
return SIZET2NUM(rb_obj_memsize_of(obj));
}
struct total_data {
size_t total;
VALUE klass;
};
static int
total_i(void *vstart, void *vend, size_t stride, void *ptr)
{
VALUE v;
struct total_data *data = (struct total_data *)ptr;
for (v = (VALUE)vstart; v != (VALUE)vend; v += stride) {
if (RBASIC(v)->flags) {
switch (BUILTIN_TYPE(v)) {
case T_NONE:
case T_IMEMO:
case T_ICLASS:
case T_NODE:
case T_ZOMBIE:
continue;
default:
if (data->klass == 0 || rb_obj_is_kind_of(v, data->klass)) {
data->total += rb_obj_memsize_of(v);
}
}
}
}
return 0;
}
static VALUE
memsize_of_all_m(int argc, VALUE *argv, VALUE self)
{
struct total_data data = {0, 0};
if (argc > 0) {
rb_scan_args(argc, argv, "01", &data.klass);
}
rb_objspace_each_objects(total_i, &data);
return SIZET2NUM(data.total);
}
static int
set_zero_i(st_data_t key, st_data_t val, st_data_t arg)
{
VALUE k = (VALUE)key;
VALUE hash = (VALUE)arg;
rb_hash_aset(hash, k, INT2FIX(0));
return ST_CONTINUE;
}
static VALUE
setup_hash(int argc, VALUE *argv)
{
VALUE hash;
if (rb_scan_args(argc, argv, "01", &hash) == 1) {
if (!RB_TYPE_P(hash, T_HASH))
rb_raise(rb_eTypeError, "non-hash given");
}
if (hash == Qnil) {
hash = rb_hash_new();
}
else if (!RHASH_EMPTY_P(hash)) {
st_foreach(RHASH_TBL(hash), set_zero_i, hash);
}
return hash;
}
static int
cos_i(void *vstart, void *vend, size_t stride, void *data)
{
size_t *counts = (size_t *)data;
VALUE v = (VALUE)vstart;
for (;v != (VALUE)vend; v += stride) {
if (RBASIC(v)->flags) {
counts[BUILTIN_TYPE(v)] += rb_obj_memsize_of(v);
}
}
return 0;
}
static VALUE
type2sym(enum ruby_value_type i)
{
VALUE type;
switch (i) {
#define CASE_TYPE(t) case t: type = ID2SYM(rb_intern(#t)); break;
CASE_TYPE(T_NONE);
CASE_TYPE(T_OBJECT);
CASE_TYPE(T_CLASS);
CASE_TYPE(T_MODULE);
CASE_TYPE(T_FLOAT);
CASE_TYPE(T_STRING);
CASE_TYPE(T_REGEXP);
CASE_TYPE(T_ARRAY);
CASE_TYPE(T_HASH);
CASE_TYPE(T_STRUCT);
CASE_TYPE(T_BIGNUM);
CASE_TYPE(T_FILE);
CASE_TYPE(T_DATA);
CASE_TYPE(T_MATCH);
CASE_TYPE(T_COMPLEX);
CASE_TYPE(T_RATIONAL);
CASE_TYPE(T_NIL);
CASE_TYPE(T_TRUE);
CASE_TYPE(T_FALSE);
CASE_TYPE(T_SYMBOL);
CASE_TYPE(T_FIXNUM);
CASE_TYPE(T_UNDEF);
CASE_TYPE(T_IMEMO);
CASE_TYPE(T_NODE);
CASE_TYPE(T_ICLASS);
CASE_TYPE(T_MOVED);
CASE_TYPE(T_ZOMBIE);
#undef CASE_TYPE
default: rb_bug("type2sym: unknown type (%d)", i);
}
return type;
}
static VALUE
count_objects_size(int argc, VALUE *argv, VALUE os)
{
size_t counts[T_MASK+1];
size_t total = 0;
enum ruby_value_type i;
VALUE hash = setup_hash(argc, argv);
for (i = 0; i <= T_MASK; i++) {
counts[i] = 0;
}
rb_objspace_each_objects(cos_i, &counts[0]);
for (i = 0; i <= T_MASK; i++) {
if (counts[i]) {
VALUE type = type2sym(i);
total += counts[i];
rb_hash_aset(hash, type, SIZET2NUM(counts[i]));
}
}
rb_hash_aset(hash, ID2SYM(rb_intern("TOTAL")), SIZET2NUM(total));
return hash;
}
struct dynamic_symbol_counts {
size_t mortal;
size_t immortal;
};
static int
cs_i(void *vstart, void *vend, size_t stride, void *n)
{
struct dynamic_symbol_counts *counts = (struct dynamic_symbol_counts *)n;
VALUE v = (VALUE)vstart;
for (; v != (VALUE)vend; v += stride) {
if (RBASIC(v)->flags && BUILTIN_TYPE(v) == T_SYMBOL) {
ID id = RSYMBOL(v)->id;
if ((id & ~ID_SCOPE_MASK) == 0) {
counts->mortal++;
}
else {
counts->immortal++;
}
}
}
return 0;
}
size_t rb_sym_immortal_count(void);
static VALUE
count_symbols(int argc, VALUE *argv, VALUE os)
{
struct dynamic_symbol_counts dynamic_counts = {0, 0};
VALUE hash = setup_hash(argc, argv);
size_t immortal_symbols = rb_sym_immortal_count();
rb_objspace_each_objects(cs_i, &dynamic_counts);
rb_hash_aset(hash, ID2SYM(rb_intern("mortal_dynamic_symbol")), SIZET2NUM(dynamic_counts.mortal));
rb_hash_aset(hash, ID2SYM(rb_intern("immortal_dynamic_symbol")), SIZET2NUM(dynamic_counts.immortal));
rb_hash_aset(hash, ID2SYM(rb_intern("immortal_static_symbol")), SIZET2NUM(immortal_symbols - dynamic_counts.immortal));
rb_hash_aset(hash, ID2SYM(rb_intern("immortal_symbol")), SIZET2NUM(immortal_symbols));
return hash;
}
static int
cn_i(void *vstart, void *vend, size_t stride, void *n)
{
size_t *nodes = (size_t *)n;
VALUE v = (VALUE)vstart;
for (; v != (VALUE)vend; v += stride) {
if (RBASIC(v)->flags && BUILTIN_TYPE(v) == T_NODE) {
size_t s = nd_type((NODE *)v);
nodes[s]++;
}
}
return 0;
}
static VALUE
count_nodes(int argc, VALUE *argv, VALUE os)
{
size_t nodes[NODE_LAST+1];
enum node_type i;
VALUE hash = setup_hash(argc, argv);
for (i = 0; i <= NODE_LAST; i++) {
nodes[i] = 0;
}
rb_objspace_each_objects(cn_i, &nodes[0]);
for (i=0; i<NODE_LAST; i++) {
if (nodes[i] != 0) {
VALUE node;
switch (i) {
#define COUNT_NODE(n) case n: node = ID2SYM(rb_intern(#n)); goto set
COUNT_NODE(NODE_SCOPE);
COUNT_NODE(NODE_BLOCK);
COUNT_NODE(NODE_IF);
COUNT_NODE(NODE_UNLESS);
COUNT_NODE(NODE_CASE);
COUNT_NODE(NODE_CASE2);
COUNT_NODE(NODE_CASE3);
COUNT_NODE(NODE_WHEN);
COUNT_NODE(NODE_IN);
COUNT_NODE(NODE_WHILE);
COUNT_NODE(NODE_UNTIL);
COUNT_NODE(NODE_ITER);
COUNT_NODE(NODE_FOR);
COUNT_NODE(NODE_FOR_MASGN);
COUNT_NODE(NODE_BREAK);
COUNT_NODE(NODE_NEXT);
COUNT_NODE(NODE_REDO);
COUNT_NODE(NODE_RETRY);
COUNT_NODE(NODE_BEGIN);
COUNT_NODE(NODE_RESCUE);
COUNT_NODE(NODE_RESBODY);
COUNT_NODE(NODE_ENSURE);
COUNT_NODE(NODE_AND);
COUNT_NODE(NODE_OR);
COUNT_NODE(NODE_MASGN);
COUNT_NODE(NODE_LASGN);
COUNT_NODE(NODE_DASGN);
COUNT_NODE(NODE_DASGN_CURR);
COUNT_NODE(NODE_GASGN);
COUNT_NODE(NODE_IASGN);
COUNT_NODE(NODE_CDECL);
COUNT_NODE(NODE_CVASGN);
COUNT_NODE(NODE_OP_ASGN1);
COUNT_NODE(NODE_OP_ASGN2);
COUNT_NODE(NODE_OP_ASGN_AND);
COUNT_NODE(NODE_OP_ASGN_OR);
COUNT_NODE(NODE_OP_CDECL);
COUNT_NODE(NODE_CALL);
COUNT_NODE(NODE_OPCALL);
COUNT_NODE(NODE_FCALL);
COUNT_NODE(NODE_VCALL);
COUNT_NODE(NODE_QCALL);
COUNT_NODE(NODE_SUPER);
COUNT_NODE(NODE_ZSUPER);
COUNT_NODE(NODE_LIST);
COUNT_NODE(NODE_ZLIST);
COUNT_NODE(NODE_VALUES);
COUNT_NODE(NODE_HASH);
COUNT_NODE(NODE_RETURN);
COUNT_NODE(NODE_YIELD);
COUNT_NODE(NODE_LVAR);
COUNT_NODE(NODE_DVAR);
COUNT_NODE(NODE_GVAR);
COUNT_NODE(NODE_IVAR);
COUNT_NODE(NODE_CONST);
COUNT_NODE(NODE_CVAR);
COUNT_NODE(NODE_NTH_REF);
COUNT_NODE(NODE_BACK_REF);
COUNT_NODE(NODE_MATCH);
COUNT_NODE(NODE_MATCH2);
COUNT_NODE(NODE_MATCH3);
COUNT_NODE(NODE_LIT);
COUNT_NODE(NODE_STR);
COUNT_NODE(NODE_DSTR);
COUNT_NODE(NODE_XSTR);
COUNT_NODE(NODE_DXSTR);
COUNT_NODE(NODE_EVSTR);
COUNT_NODE(NODE_DREGX);
COUNT_NODE(NODE_ONCE);
COUNT_NODE(NODE_ARGS);
COUNT_NODE(NODE_ARGS_AUX);
COUNT_NODE(NODE_OPT_ARG);
COUNT_NODE(NODE_KW_ARG);
COUNT_NODE(NODE_POSTARG);
COUNT_NODE(NODE_ARGSCAT);
COUNT_NODE(NODE_ARGSPUSH);
COUNT_NODE(NODE_SPLAT);
COUNT_NODE(NODE_BLOCK_PASS);
COUNT_NODE(NODE_DEFN);
COUNT_NODE(NODE_DEFS);
COUNT_NODE(NODE_ALIAS);
COUNT_NODE(NODE_VALIAS);
COUNT_NODE(NODE_UNDEF);
COUNT_NODE(NODE_CLASS);
COUNT_NODE(NODE_MODULE);
COUNT_NODE(NODE_SCLASS);
COUNT_NODE(NODE_COLON2);
COUNT_NODE(NODE_COLON3);
COUNT_NODE(NODE_DOT2);
COUNT_NODE(NODE_DOT3);
COUNT_NODE(NODE_FLIP2);
COUNT_NODE(NODE_FLIP3);
COUNT_NODE(NODE_SELF);
COUNT_NODE(NODE_NIL);
COUNT_NODE(NODE_TRUE);
COUNT_NODE(NODE_FALSE);
COUNT_NODE(NODE_ERRINFO);
COUNT_NODE(NODE_DEFINED);
COUNT_NODE(NODE_POSTEXE);
COUNT_NODE(NODE_DSYM);
COUNT_NODE(NODE_ATTRASGN);
COUNT_NODE(NODE_LAMBDA);
COUNT_NODE(NODE_ARYPTN);
COUNT_NODE(NODE_HSHPTN);
#undef COUNT_NODE
case NODE_LAST: break;
}
UNREACHABLE;
set:
rb_hash_aset(hash, node, SIZET2NUM(nodes[i]));
}
}
return hash;
}
static int
cto_i(void *vstart, void *vend, size_t stride, void *data)
{
VALUE hash = (VALUE)data;
VALUE v = (VALUE)vstart;
for (; v != (VALUE)vend; v += stride) {
if (RBASIC(v)->flags && BUILTIN_TYPE(v) == T_DATA) {
VALUE counter;
VALUE key = RBASIC(v)->klass;
if (key == 0) {
const char *name = rb_objspace_data_type_name(v);
if (name == 0) name = "unknown";
key = ID2SYM(rb_intern(name));
}
counter = rb_hash_aref(hash, key);
if (NIL_P(counter)) {
counter = INT2FIX(1);
}
else {
counter = INT2FIX(FIX2INT(counter) + 1);
}
rb_hash_aset(hash, key, counter);
}
}
return 0;
}
static VALUE
count_tdata_objects(int argc, VALUE *argv, VALUE self)
{
VALUE hash = setup_hash(argc, argv);
rb_objspace_each_objects(cto_i, (void *)hash);
return hash;
}
static ID imemo_type_ids[IMEMO_MASK+1];
static int
count_imemo_objects_i(void *vstart, void *vend, size_t stride, void *data)
{
VALUE hash = (VALUE)data;
VALUE v = (VALUE)vstart;
for (; v != (VALUE)vend; v += stride) {
if (RBASIC(v)->flags && BUILTIN_TYPE(v) == T_IMEMO) {
VALUE counter;
VALUE key = ID2SYM(imemo_type_ids[imemo_type(v)]);
counter = rb_hash_aref(hash, key);
if (NIL_P(counter)) {
counter = INT2FIX(1);
}
else {
counter = INT2FIX(FIX2INT(counter) + 1);
}
rb_hash_aset(hash, key, counter);
}
}
return 0;
}
static VALUE
count_imemo_objects(int argc, VALUE *argv, VALUE self)
{
VALUE hash = setup_hash(argc, argv);
if (imemo_type_ids[0] == 0) {
imemo_type_ids[0] = rb_intern("imemo_env");
imemo_type_ids[1] = rb_intern("imemo_cref");
imemo_type_ids[2] = rb_intern("imemo_svar");
imemo_type_ids[3] = rb_intern("imemo_throw_data");
imemo_type_ids[4] = rb_intern("imemo_ifunc");
imemo_type_ids[5] = rb_intern("imemo_memo");
imemo_type_ids[6] = rb_intern("imemo_ment");
imemo_type_ids[7] = rb_intern("imemo_iseq");
imemo_type_ids[8] = rb_intern("imemo_tmpbuf");
imemo_type_ids[9] = rb_intern("imemo_ast");
imemo_type_ids[10] = rb_intern("imemo_parser_strterm");
imemo_type_ids[11] = rb_intern("imemo_callinfo");
imemo_type_ids[12] = rb_intern("imemo_callcache");
}
rb_objspace_each_objects(count_imemo_objects_i, (void *)hash);
return hash;
}
static void
iow_mark(void *ptr)
{
rb_gc_mark((VALUE)ptr);
}
static size_t
iow_size(const void *ptr)
{
VALUE obj = (VALUE)ptr;
return rb_obj_memsize_of(obj);
}
static const rb_data_type_t iow_data_type = {
"ObjectSpace::InternalObjectWrapper",
{iow_mark, 0, iow_size,},
0, 0, RUBY_TYPED_FREE_IMMEDIATELY
};
static VALUE rb_cInternalObjectWrapper;
static VALUE
iow_newobj(VALUE obj)
{
return TypedData_Wrap_Struct(rb_cInternalObjectWrapper, &iow_data_type, (void *)obj);
}
static VALUE
iow_type(VALUE self)
{
VALUE obj = (VALUE)DATA_PTR(self);
return type2sym(BUILTIN_TYPE(obj));
}
static VALUE
iow_inspect(VALUE self)
{
VALUE obj = (VALUE)DATA_PTR(self);
VALUE type = type2sym(BUILTIN_TYPE(obj));
return rb_sprintf("#<InternalObject:%p %"PRIsVALUE">", (void *)obj, rb_sym2str(type));
}
static VALUE
iow_internal_object_id(VALUE self)
{
VALUE obj = (VALUE)DATA_PTR(self);
return rb_obj_id(obj);
}
struct rof_data {
st_table *refs;
VALUE internals;
};
static void
reachable_object_from_i(VALUE obj, void *data_ptr)
{
struct rof_data *data = (struct rof_data *)data_ptr;
VALUE key = obj;
VALUE val = obj;
if (rb_objspace_markable_object_p(obj)) {
if (rb_objspace_internal_object_p(obj)) {
val = iow_newobj(obj);
rb_ary_push(data->internals, val);
}
st_insert(data->refs, key, val);
}
}
static int
collect_values(st_data_t key, st_data_t value, st_data_t data)
{
VALUE ary = (VALUE)data;
rb_ary_push(ary, (VALUE)value);
return ST_CONTINUE;
}
static VALUE
reachable_objects_from(VALUE self, VALUE obj)
{
if (rb_objspace_markable_object_p(obj)) {
VALUE ret = rb_ary_new();
struct rof_data data;
if (rb_typeddata_is_kind_of(obj, &iow_data_type)) {
obj = (VALUE)DATA_PTR(obj);
}
data.refs = st_init_numtable();
data.internals = rb_ary_new();
rb_objspace_reachable_objects_from(obj, reachable_object_from_i, &data);
st_foreach(data.refs, collect_values, (st_data_t)ret);
return ret;
}
else {
return Qnil;
}
}
struct rofr_data {
VALUE categories;
const char *last_category;
VALUE last_category_str;
VALUE last_category_objects;
};
static void
reachable_object_from_root_i(const char *category, VALUE obj, void *ptr)
{
struct rofr_data *data = (struct rofr_data *)ptr;
VALUE category_str;
VALUE category_objects;
if (category == data->last_category) {
category_str = data->last_category_str;
category_objects = data->last_category_objects;
}
else {
data->last_category = category;
category_str = data->last_category_str = rb_str_new2(category);
category_objects = data->last_category_objects = rb_ident_hash_new();
if (!NIL_P(rb_hash_lookup(data->categories, category_str))) {
rb_bug("reachable_object_from_root_i: category should insert at once");
}
rb_hash_aset(data->categories, category_str, category_objects);
}
if (rb_objspace_markable_object_p(obj) &&
obj != data->categories &&
obj != data->last_category_objects) {
if (rb_objspace_internal_object_p(obj)) {
obj = iow_newobj(obj);
}
rb_hash_aset(category_objects, obj, obj);
}
}
static int
collect_values_of_values(VALUE category, VALUE category_objects, VALUE categories)
{
VALUE ary = rb_ary_new();
rb_hash_foreach(category_objects, collect_values, ary);
rb_hash_aset(categories, category, ary);
return ST_CONTINUE;
}
static VALUE
reachable_objects_from_root(VALUE self)
{
struct rofr_data data;
VALUE hash = data.categories = rb_ident_hash_new();
data.last_category = 0;
rb_objspace_reachable_objects_from_root(reachable_object_from_root_i, &data);
rb_hash_foreach(hash, collect_values_of_values, hash);
return hash;
}
static VALUE
wrap_klass_iow(VALUE klass)
{
if (!RTEST(klass)) {
return Qnil;
}
else if (RB_TYPE_P(klass, T_ICLASS) ||
CLASS_OF(klass) == Qfalse ) {
return iow_newobj(klass);
}
else {
return klass;
}
}
static VALUE
objspace_internal_class_of(VALUE self, VALUE obj)
{
VALUE klass;
if (rb_typeddata_is_kind_of(obj, &iow_data_type)) {
obj = (VALUE)DATA_PTR(obj);
}
klass = CLASS_OF(obj);
return wrap_klass_iow(klass);
}
static VALUE
objspace_internal_super_of(VALUE self, VALUE obj)
{
VALUE super;
if (rb_typeddata_is_kind_of(obj, &iow_data_type)) {
obj = (VALUE)DATA_PTR(obj);
}
switch (OBJ_BUILTIN_TYPE(obj)) {
case T_MODULE:
case T_CLASS:
case T_ICLASS:
super = RCLASS_SUPER(obj);
break;
default:
rb_raise(rb_eArgError, "class or module is expected");
}
return wrap_klass_iow(super);
}
void Init_object_tracing(VALUE rb_mObjSpace);
void Init_objspace_dump(VALUE rb_mObjSpace);
void
Init_objspace(void)
{
#undef rb_intern
VALUE rb_mObjSpace;
#if 0
rb_mObjSpace = rb_define_module("ObjectSpace"); 
#endif
rb_mObjSpace = rb_const_get(rb_cObject, rb_intern("ObjectSpace"));
rb_define_module_function(rb_mObjSpace, "memsize_of", memsize_of_m, 1);
rb_define_module_function(rb_mObjSpace, "memsize_of_all", memsize_of_all_m, -1);
rb_define_module_function(rb_mObjSpace, "count_objects_size", count_objects_size, -1);
rb_define_module_function(rb_mObjSpace, "count_symbols", count_symbols, -1);
rb_define_module_function(rb_mObjSpace, "count_nodes", count_nodes, -1);
rb_define_module_function(rb_mObjSpace, "count_tdata_objects", count_tdata_objects, -1);
rb_define_module_function(rb_mObjSpace, "count_imemo_objects", count_imemo_objects, -1);
rb_define_module_function(rb_mObjSpace, "reachable_objects_from", reachable_objects_from, 1);
rb_define_module_function(rb_mObjSpace, "reachable_objects_from_root", reachable_objects_from_root, 0);
rb_define_module_function(rb_mObjSpace, "internal_class_of", objspace_internal_class_of, 1);
rb_define_module_function(rb_mObjSpace, "internal_super_of", objspace_internal_super_of, 1);
rb_cInternalObjectWrapper = rb_define_class_under(rb_mObjSpace, "InternalObjectWrapper", rb_cObject);
rb_define_method(rb_cInternalObjectWrapper, "type", iow_type, 0);
rb_define_method(rb_cInternalObjectWrapper, "inspect", iow_inspect, 0);
rb_define_method(rb_cInternalObjectWrapper, "internal_object_id", iow_internal_object_id, 0);
Init_object_tracing(rb_mObjSpace);
Init_objspace_dump(rb_mObjSpace);
}
