#include "id.h"
#include "internal.h"
#include "internal/compar.h"
#include "internal/enum.h"
#include "internal/hash.h"
#include "internal/imemo.h"
#include "internal/numeric.h"
#include "internal/object.h"
#include "internal/proc.h"
#include "internal/rational.h"
#include "ruby/util.h"
#include "ruby_assert.h"
#include "symbol.h"
VALUE rb_mEnumerable;
static ID id_next;
#define id_div idDiv
#define id_each idEach
#define id_eqq idEqq
#define id_cmp idCmp
#define id_lshift idLTLT
#define id_call idCall
#define id_size idSize
VALUE
rb_enum_values_pack(int argc, const VALUE *argv)
{
if (argc == 0) return Qnil;
if (argc == 1) return argv[0];
return rb_ary_new4(argc, argv);
}
#define ENUM_WANT_SVALUE() do { i = rb_enum_values_pack(argc, argv); } while (0)
static VALUE
enum_yield(int argc, VALUE ary)
{
if (argc > 1)
return rb_yield_force_blockarg(ary);
if (argc == 1)
return rb_yield(ary);
return rb_yield_values2(0, 0);
}
static VALUE
enum_yield_array(VALUE ary)
{
long len = RARRAY_LEN(ary);
if (len > 1)
return rb_yield_force_blockarg(ary);
if (len == 1)
return rb_yield(RARRAY_AREF(ary, 0));
return rb_yield_values2(0, 0);
}
static VALUE
grep_i(RB_BLOCK_CALL_FUNC_ARGLIST(i, args))
{
struct MEMO *memo = MEMO_CAST(args);
ENUM_WANT_SVALUE();
if (RTEST(rb_funcallv(memo->v1, id_eqq, 1, &i)) == RTEST(memo->u3.value)) {
rb_ary_push(memo->v2, i);
}
return Qnil;
}
static VALUE
grep_iter_i(RB_BLOCK_CALL_FUNC_ARGLIST(i, args))
{
struct MEMO *memo = MEMO_CAST(args);
ENUM_WANT_SVALUE();
if (RTEST(rb_funcallv(memo->v1, id_eqq, 1, &i)) == RTEST(memo->u3.value)) {
rb_ary_push(memo->v2, enum_yield(argc, i));
}
return Qnil;
}
static VALUE
enum_grep(VALUE obj, VALUE pat)
{
VALUE ary = rb_ary_new();
struct MEMO *memo = MEMO_NEW(pat, ary, Qtrue);
rb_block_call(obj, id_each, 0, 0, rb_block_given_p() ? grep_iter_i : grep_i, (VALUE)memo);
return ary;
}
static VALUE
enum_grep_v(VALUE obj, VALUE pat)
{
VALUE ary = rb_ary_new();
struct MEMO *memo = MEMO_NEW(pat, ary, Qfalse);
rb_block_call(obj, id_each, 0, 0, rb_block_given_p() ? grep_iter_i : grep_i, (VALUE)memo);
return ary;
}
#define COUNT_BIGNUM IMEMO_FL_USER0
#define MEMO_V3_SET(m, v) RB_OBJ_WRITE((m), &(m)->u3.value, (v))
static void
imemo_count_up(struct MEMO *memo)
{
if (memo->flags & COUNT_BIGNUM) {
MEMO_V3_SET(memo, rb_int_succ(memo->u3.value));
}
else if (++memo->u3.cnt == 0) {
unsigned long buf[2] = {0, 1};
MEMO_V3_SET(memo, rb_big_unpack(buf, 2));
memo->flags |= COUNT_BIGNUM;
}
}
static VALUE
imemo_count_value(struct MEMO *memo)
{
if (memo->flags & COUNT_BIGNUM) {
return memo->u3.value;
}
else {
return ULONG2NUM(memo->u3.cnt);
}
}
static VALUE
count_i(RB_BLOCK_CALL_FUNC_ARGLIST(i, memop))
{
struct MEMO *memo = MEMO_CAST(memop);
ENUM_WANT_SVALUE();
if (rb_equal(i, memo->v1)) {
imemo_count_up(memo);
}
return Qnil;
}
static VALUE
count_iter_i(RB_BLOCK_CALL_FUNC_ARGLIST(i, memop))
{
struct MEMO *memo = MEMO_CAST(memop);
if (RTEST(rb_yield_values2(argc, argv))) {
imemo_count_up(memo);
}
return Qnil;
}
static VALUE
count_all_i(RB_BLOCK_CALL_FUNC_ARGLIST(i, memop))
{
struct MEMO *memo = MEMO_CAST(memop);
imemo_count_up(memo);
return Qnil;
}
static VALUE
enum_count(int argc, VALUE *argv, VALUE obj)
{
VALUE item = Qnil;
struct MEMO *memo;
rb_block_call_func *func;
if (argc == 0) {
if (rb_block_given_p()) {
func = count_iter_i;
}
else {
func = count_all_i;
}
}
else {
rb_scan_args(argc, argv, "1", &item);
if (rb_block_given_p()) {
rb_warn("given block not used");
}
func = count_i;
}
memo = MEMO_NEW(item, 0, 0);
rb_block_call(obj, id_each, 0, 0, func, (VALUE)memo);
return imemo_count_value(memo);
}
static VALUE
find_i(RB_BLOCK_CALL_FUNC_ARGLIST(i, memop))
{
ENUM_WANT_SVALUE();
if (RTEST(enum_yield(argc, i))) {
struct MEMO *memo = MEMO_CAST(memop);
MEMO_V1_SET(memo, i);
memo->u3.cnt = 1;
rb_iter_break();
}
return Qnil;
}
static VALUE
enum_find(int argc, VALUE *argv, VALUE obj)
{
struct MEMO *memo;
VALUE if_none;
if_none = rb_check_arity(argc, 0, 1) ? argv[0] : Qnil;
RETURN_ENUMERATOR(obj, argc, argv);
memo = MEMO_NEW(Qundef, 0, 0);
rb_block_call(obj, id_each, 0, 0, find_i, (VALUE)memo);
if (memo->u3.cnt) {
return memo->v1;
}
if (!NIL_P(if_none)) {
return rb_funcallv(if_none, id_call, 0, 0);
}
return Qnil;
}
static VALUE
find_index_i(RB_BLOCK_CALL_FUNC_ARGLIST(i, memop))
{
struct MEMO *memo = MEMO_CAST(memop);
ENUM_WANT_SVALUE();
if (rb_equal(i, memo->v2)) {
MEMO_V1_SET(memo, imemo_count_value(memo));
rb_iter_break();
}
imemo_count_up(memo);
return Qnil;
}
static VALUE
find_index_iter_i(RB_BLOCK_CALL_FUNC_ARGLIST(i, memop))
{
struct MEMO *memo = MEMO_CAST(memop);
if (RTEST(rb_yield_values2(argc, argv))) {
MEMO_V1_SET(memo, imemo_count_value(memo));
rb_iter_break();
}
imemo_count_up(memo);
return Qnil;
}
static VALUE
enum_find_index(int argc, VALUE *argv, VALUE obj)
{
struct MEMO *memo; 
VALUE condition_value = Qnil;
rb_block_call_func *func;
if (argc == 0) {
RETURN_ENUMERATOR(obj, 0, 0);
func = find_index_iter_i;
}
else {
rb_scan_args(argc, argv, "1", &condition_value);
if (rb_block_given_p()) {
rb_warn("given block not used");
}
func = find_index_i;
}
memo = MEMO_NEW(Qnil, condition_value, 0);
rb_block_call(obj, id_each, 0, 0, func, (VALUE)memo);
return memo->v1;
}
static VALUE
find_all_i(RB_BLOCK_CALL_FUNC_ARGLIST(i, ary))
{
ENUM_WANT_SVALUE();
if (RTEST(enum_yield(argc, i))) {
rb_ary_push(ary, i);
}
return Qnil;
}
static VALUE
enum_size(VALUE self, VALUE args, VALUE eobj)
{
return rb_check_funcall_default(self, id_size, 0, 0, Qnil);
}
static long
limit_by_enum_size(VALUE obj, long n)
{
unsigned long limit;
VALUE size = rb_check_funcall(obj, id_size, 0, 0);
if (!FIXNUM_P(size)) return n;
limit = FIX2ULONG(size);
return ((unsigned long)n > limit) ? (long)limit : n;
}
static int
enum_size_over_p(VALUE obj, long n)
{
VALUE size = rb_check_funcall(obj, id_size, 0, 0);
if (!FIXNUM_P(size)) return 0;
return ((unsigned long)n > FIX2ULONG(size));
}
static VALUE
enum_find_all(VALUE obj)
{
VALUE ary;
RETURN_SIZED_ENUMERATOR(obj, 0, 0, enum_size);
ary = rb_ary_new();
rb_block_call(obj, id_each, 0, 0, find_all_i, ary);
return ary;
}
static VALUE
filter_map_i(RB_BLOCK_CALL_FUNC_ARGLIST(i, ary))
{
i = rb_yield_values2(argc, argv);
if (RTEST(i)) {
rb_ary_push(ary, i);
}
return Qnil;
}
static VALUE
enum_filter_map(VALUE obj)
{
VALUE ary;
RETURN_SIZED_ENUMERATOR(obj, 0, 0, enum_size);
ary = rb_ary_new();
rb_block_call(obj, id_each, 0, 0, filter_map_i, ary);
return ary;
}
static VALUE
reject_i(RB_BLOCK_CALL_FUNC_ARGLIST(i, ary))
{
ENUM_WANT_SVALUE();
if (!RTEST(enum_yield(argc, i))) {
rb_ary_push(ary, i);
}
return Qnil;
}
static VALUE
enum_reject(VALUE obj)
{
VALUE ary;
RETURN_SIZED_ENUMERATOR(obj, 0, 0, enum_size);
ary = rb_ary_new();
rb_block_call(obj, id_each, 0, 0, reject_i, ary);
return ary;
}
static VALUE
collect_i(RB_BLOCK_CALL_FUNC_ARGLIST(i, ary))
{
rb_ary_push(ary, rb_yield_values2(argc, argv));
return Qnil;
}
static VALUE
collect_all(RB_BLOCK_CALL_FUNC_ARGLIST(i, ary))
{
rb_ary_push(ary, rb_enum_values_pack(argc, argv));
return Qnil;
}
static VALUE
enum_collect(VALUE obj)
{
VALUE ary;
int min_argc, max_argc;
RETURN_SIZED_ENUMERATOR(obj, 0, 0, enum_size);
ary = rb_ary_new();
min_argc = rb_block_min_max_arity(&max_argc);
rb_lambda_call(obj, id_each, 0, 0, collect_i, min_argc, max_argc, ary);
return ary;
}
static VALUE
flat_map_i(RB_BLOCK_CALL_FUNC_ARGLIST(i, ary))
{
VALUE tmp;
i = rb_yield_values2(argc, argv);
tmp = rb_check_array_type(i);
if (NIL_P(tmp)) {
rb_ary_push(ary, i);
}
else {
rb_ary_concat(ary, tmp);
}
return Qnil;
}
static VALUE
enum_flat_map(VALUE obj)
{
VALUE ary;
RETURN_SIZED_ENUMERATOR(obj, 0, 0, enum_size);
ary = rb_ary_new();
rb_block_call(obj, id_each, 0, 0, flat_map_i, ary);
return ary;
}
static VALUE
enum_to_a(int argc, VALUE *argv, VALUE obj)
{
VALUE ary = rb_ary_new();
rb_block_call(obj, id_each, argc, argv, collect_all, ary);
return ary;
}
static VALUE
enum_hashify(VALUE obj, int argc, const VALUE *argv, rb_block_call_func *iter)
{
VALUE hash = rb_hash_new();
rb_block_call(obj, id_each, argc, argv, iter, hash);
return hash;
}
static VALUE
enum_to_h_i(RB_BLOCK_CALL_FUNC_ARGLIST(i, hash))
{
ENUM_WANT_SVALUE();
return rb_hash_set_pair(hash, i);
}
static VALUE
enum_to_h_ii(RB_BLOCK_CALL_FUNC_ARGLIST(i, hash))
{
return rb_hash_set_pair(hash, rb_yield_values2(argc, argv));
}
static VALUE
enum_to_h(int argc, VALUE *argv, VALUE obj)
{
rb_block_call_func *iter = rb_block_given_p() ? enum_to_h_ii : enum_to_h_i;
return enum_hashify(obj, argc, argv, iter);
}
static VALUE
inject_i(RB_BLOCK_CALL_FUNC_ARGLIST(i, p))
{
struct MEMO *memo = MEMO_CAST(p);
ENUM_WANT_SVALUE();
if (memo->v1 == Qundef) {
MEMO_V1_SET(memo, i);
}
else {
MEMO_V1_SET(memo, rb_yield_values(2, memo->v1, i));
}
return Qnil;
}
static VALUE
inject_op_i(RB_BLOCK_CALL_FUNC_ARGLIST(i, p))
{
struct MEMO *memo = MEMO_CAST(p);
VALUE name;
ENUM_WANT_SVALUE();
if (memo->v1 == Qundef) {
MEMO_V1_SET(memo, i);
}
else if (SYMBOL_P(name = memo->u3.value)) {
const ID mid = SYM2ID(name);
MEMO_V1_SET(memo, rb_funcallv(memo->v1, mid, 1, &i));
}
else {
VALUE args[2];
args[0] = name;
args[1] = i;
MEMO_V1_SET(memo, rb_f_send(numberof(args), args, memo->v1));
}
return Qnil;
}
static VALUE
ary_inject_op(VALUE ary, VALUE init, VALUE op)
{
ID id;
VALUE v, e;
long i, n;
if (RARRAY_LEN(ary) == 0)
return init == Qundef ? Qnil : init;
if (init == Qundef) {
v = RARRAY_AREF(ary, 0);
i = 1;
if (RARRAY_LEN(ary) == 1)
return v;
}
else {
v = init;
i = 0;
}
id = SYM2ID(op);
if (id == idPLUS) {
if (RB_INTEGER_TYPE_P(v) &&
rb_method_basic_definition_p(rb_cInteger, idPLUS) &&
rb_obj_respond_to(v, idPLUS, FALSE)) {
n = 0;
for (; i < RARRAY_LEN(ary); i++) {
e = RARRAY_AREF(ary, i);
if (FIXNUM_P(e)) {
n += FIX2LONG(e); 
if (!FIXABLE(n)) {
v = rb_big_plus(ULONG2NUM(n), v);
n = 0;
}
}
else if (RB_TYPE_P(e, T_BIGNUM))
v = rb_big_plus(e, v);
else
goto not_integer;
}
if (n != 0)
v = rb_fix_plus(LONG2FIX(n), v);
return v;
not_integer:
if (n != 0)
v = rb_fix_plus(LONG2FIX(n), v);
}
}
for (; i < RARRAY_LEN(ary); i++) {
VALUE arg = RARRAY_AREF(ary, i);
v = rb_funcallv_public(v, id, 1, &arg);
}
return v;
}
static VALUE
enum_inject(int argc, VALUE *argv, VALUE obj)
{
struct MEMO *memo;
VALUE init, op;
rb_block_call_func *iter = inject_i;
ID id;
switch (rb_scan_args(argc, argv, "02", &init, &op)) {
case 0:
init = Qundef;
break;
case 1:
if (rb_block_given_p()) {
break;
}
id = rb_check_id(&init);
op = id ? ID2SYM(id) : init;
init = Qundef;
iter = inject_op_i;
break;
case 2:
if (rb_block_given_p()) {
rb_warning("given block not used");
}
id = rb_check_id(&op);
if (id) op = ID2SYM(id);
iter = inject_op_i;
break;
}
if (iter == inject_op_i &&
SYMBOL_P(op) &&
RB_TYPE_P(obj, T_ARRAY) &&
rb_method_basic_definition_p(CLASS_OF(obj), id_each)) {
return ary_inject_op(obj, init, op);
}
memo = MEMO_NEW(init, Qnil, op);
rb_block_call(obj, id_each, 0, 0, iter, (VALUE)memo);
if (memo->v1 == Qundef) return Qnil;
return memo->v1;
}
static VALUE
partition_i(RB_BLOCK_CALL_FUNC_ARGLIST(i, arys))
{
struct MEMO *memo = MEMO_CAST(arys);
VALUE ary;
ENUM_WANT_SVALUE();
if (RTEST(enum_yield(argc, i))) {
ary = memo->v1;
}
else {
ary = memo->v2;
}
rb_ary_push(ary, i);
return Qnil;
}
static VALUE
enum_partition(VALUE obj)
{
struct MEMO *memo;
RETURN_SIZED_ENUMERATOR(obj, 0, 0, enum_size);
memo = MEMO_NEW(rb_ary_new(), rb_ary_new(), 0);
rb_block_call(obj, id_each, 0, 0, partition_i, (VALUE)memo);
return rb_assoc_new(memo->v1, memo->v2);
}
static VALUE
group_by_i(RB_BLOCK_CALL_FUNC_ARGLIST(i, hash))
{
VALUE group;
VALUE values;
ENUM_WANT_SVALUE();
group = enum_yield(argc, i);
values = rb_hash_aref(hash, group);
if (!RB_TYPE_P(values, T_ARRAY)) {
values = rb_ary_new3(1, i);
rb_hash_aset(hash, group, values);
}
else {
rb_ary_push(values, i);
}
return Qnil;
}
static VALUE
enum_group_by(VALUE obj)
{
RETURN_SIZED_ENUMERATOR(obj, 0, 0, enum_size);
return enum_hashify(obj, 0, 0, group_by_i);
}
static void
tally_up(VALUE hash, VALUE group)
{
VALUE tally = rb_hash_aref(hash, group);
if (NIL_P(tally)) {
tally = INT2FIX(1);
}
else if (FIXNUM_P(tally) && tally < INT2FIX(FIXNUM_MAX)) {
tally += INT2FIX(1) & ~FIXNUM_FLAG;
}
else {
tally = rb_big_plus(tally, INT2FIX(1));
}
rb_hash_aset(hash, group, tally);
}
static VALUE
tally_i(RB_BLOCK_CALL_FUNC_ARGLIST(i, hash))
{
ENUM_WANT_SVALUE();
tally_up(hash, i);
return Qnil;
}
static VALUE
enum_tally(VALUE obj)
{
return enum_hashify(obj, 0, 0, tally_i);
}
static VALUE
first_i(RB_BLOCK_CALL_FUNC_ARGLIST(i, params))
{
struct MEMO *memo = MEMO_CAST(params);
ENUM_WANT_SVALUE();
MEMO_V1_SET(memo, i);
rb_iter_break();
UNREACHABLE_RETURN(Qnil);
}
static VALUE enum_take(VALUE obj, VALUE n);
static VALUE
enum_first(int argc, VALUE *argv, VALUE obj)
{
struct MEMO *memo;
rb_check_arity(argc, 0, 1);
if (argc > 0) {
return enum_take(obj, argv[0]);
}
else {
memo = MEMO_NEW(Qnil, 0, 0);
rb_block_call(obj, id_each, 0, 0, first_i, (VALUE)memo);
return memo->v1;
}
}
static VALUE
enum_sort(VALUE obj)
{
return rb_ary_sort_bang(enum_to_a(0, 0, obj));
}
#define SORT_BY_BUFSIZE 16
struct sort_by_data {
const VALUE ary;
const VALUE buf;
long n;
};
static VALUE
sort_by_i(RB_BLOCK_CALL_FUNC_ARGLIST(i, _data))
{
struct sort_by_data *data = (struct sort_by_data *)&MEMO_CAST(_data)->v1;
VALUE ary = data->ary;
VALUE v;
ENUM_WANT_SVALUE();
v = enum_yield(argc, i);
if (RBASIC(ary)->klass) {
rb_raise(rb_eRuntimeError, "sort_by reentered");
}
if (RARRAY_LEN(data->buf) != SORT_BY_BUFSIZE*2) {
rb_raise(rb_eRuntimeError, "sort_by reentered");
}
RARRAY_ASET(data->buf, data->n*2, v);
RARRAY_ASET(data->buf, data->n*2+1, i);
data->n++;
if (data->n == SORT_BY_BUFSIZE) {
rb_ary_concat(ary, data->buf);
data->n = 0;
}
return Qnil;
}
static int
sort_by_cmp(const void *ap, const void *bp, void *data)
{
struct cmp_opt_data cmp_opt = { 0, 0 };
VALUE a;
VALUE b;
VALUE ary = (VALUE)data;
if (RBASIC(ary)->klass) {
rb_raise(rb_eRuntimeError, "sort_by reentered");
}
a = *(VALUE *)ap;
b = *(VALUE *)bp;
return OPTIMIZED_CMP(a, b, cmp_opt);
}
static VALUE
enum_sort_by(VALUE obj)
{
VALUE ary, buf;
struct MEMO *memo;
long i;
struct sort_by_data *data;
RETURN_SIZED_ENUMERATOR(obj, 0, 0, enum_size);
if (RB_TYPE_P(obj, T_ARRAY) && RARRAY_LEN(obj) <= LONG_MAX/2) {
ary = rb_ary_new2(RARRAY_LEN(obj)*2);
}
else {
ary = rb_ary_new();
}
RBASIC_CLEAR_CLASS(ary);
buf = rb_ary_tmp_new(SORT_BY_BUFSIZE*2);
rb_ary_store(buf, SORT_BY_BUFSIZE*2-1, Qnil);
memo = MEMO_NEW(0, 0, 0);
data = (struct sort_by_data *)&memo->v1;
RB_OBJ_WRITE(memo, &data->ary, ary);
RB_OBJ_WRITE(memo, &data->buf, buf);
data->n = 0;
rb_block_call(obj, id_each, 0, 0, sort_by_i, (VALUE)memo);
ary = data->ary;
buf = data->buf;
if (data->n) {
rb_ary_resize(buf, data->n*2);
rb_ary_concat(ary, buf);
}
if (RARRAY_LEN(ary) > 2) {
RARRAY_PTR_USE(ary, ptr,
ruby_qsort(ptr, RARRAY_LEN(ary)/2, 2*sizeof(VALUE),
sort_by_cmp, (void *)ary));
}
if (RBASIC(ary)->klass) {
rb_raise(rb_eRuntimeError, "sort_by reentered");
}
for (i=1; i<RARRAY_LEN(ary); i+=2) {
RARRAY_ASET(ary, i/2, RARRAY_AREF(ary, i));
}
rb_ary_resize(ary, RARRAY_LEN(ary)/2);
RBASIC_SET_CLASS_RAW(ary, rb_cArray);
return ary;
}
#define ENUMFUNC(name) argc ? name##_eqq : rb_block_given_p() ? name##_iter_i : name##_i
#define MEMO_ENUM_NEW(v1) (rb_check_arity(argc, 0, 1), MEMO_NEW((v1), (argc ? *argv : 0), 0))
#define DEFINE_ENUMFUNCS(name) static VALUE enum_##name##_func(VALUE result, struct MEMO *memo); static VALUE name##_i(RB_BLOCK_CALL_FUNC_ARGLIST(i, memo)) { return enum_##name##_func(rb_enum_values_pack(argc, argv), MEMO_CAST(memo)); } static VALUE name##_iter_i(RB_BLOCK_CALL_FUNC_ARGLIST(i, memo)) { return enum_##name##_func(rb_yield_values2(argc, argv), MEMO_CAST(memo)); } static VALUE name##_eqq(RB_BLOCK_CALL_FUNC_ARGLIST(i, memo)) { ENUM_WANT_SVALUE(); return enum_##name##_func(rb_funcallv(MEMO_CAST(memo)->v2, id_eqq, 1, &i), MEMO_CAST(memo)); } static VALUE enum_##name##_func(VALUE result, struct MEMO *memo)
#define WARN_UNUSED_BLOCK(argc) do { if ((argc) > 0 && rb_block_given_p()) { rb_warn("given block not used"); } } while (0)
DEFINE_ENUMFUNCS(all)
{
if (!RTEST(result)) {
MEMO_V1_SET(memo, Qfalse);
rb_iter_break();
}
return Qnil;
}
static VALUE
enum_all(int argc, VALUE *argv, VALUE obj)
{
struct MEMO *memo = MEMO_ENUM_NEW(Qtrue);
WARN_UNUSED_BLOCK(argc);
rb_block_call(obj, id_each, 0, 0, ENUMFUNC(all), (VALUE)memo);
return memo->v1;
}
DEFINE_ENUMFUNCS(any)
{
if (RTEST(result)) {
MEMO_V1_SET(memo, Qtrue);
rb_iter_break();
}
return Qnil;
}
static VALUE
enum_any(int argc, VALUE *argv, VALUE obj)
{
struct MEMO *memo = MEMO_ENUM_NEW(Qfalse);
WARN_UNUSED_BLOCK(argc);
rb_block_call(obj, id_each, 0, 0, ENUMFUNC(any), (VALUE)memo);
return memo->v1;
}
DEFINE_ENUMFUNCS(one)
{
if (RTEST(result)) {
if (memo->v1 == Qundef) {
MEMO_V1_SET(memo, Qtrue);
}
else if (memo->v1 == Qtrue) {
MEMO_V1_SET(memo, Qfalse);
rb_iter_break();
}
}
return Qnil;
}
struct nmin_data {
long n;
long bufmax;
long curlen;
VALUE buf;
VALUE limit;
int (*cmpfunc)(const void *, const void *, void *);
int rev: 1; 
int by: 1; 
};
static VALUE
cmpint_reenter_check(struct nmin_data *data, VALUE val)
{
if (RBASIC(data->buf)->klass) {
rb_raise(rb_eRuntimeError, "%s%s reentered",
data->rev ? "max" : "min",
data->by ? "_by" : "");
}
return val;
}
static int
nmin_cmp(const void *ap, const void *bp, void *_data)
{
struct cmp_opt_data cmp_opt = { 0, 0 };
struct nmin_data *data = (struct nmin_data *)_data;
VALUE a = *(const VALUE *)ap, b = *(const VALUE *)bp;
#define rb_cmpint(cmp, a, b) rb_cmpint(cmpint_reenter_check(data, (cmp)), a, b)
return OPTIMIZED_CMP(a, b, cmp_opt);
#undef rb_cmpint
}
static int
nmin_block_cmp(const void *ap, const void *bp, void *_data)
{
struct nmin_data *data = (struct nmin_data *)_data;
VALUE a = *(const VALUE *)ap, b = *(const VALUE *)bp;
VALUE cmp = rb_yield_values(2, a, b);
cmpint_reenter_check(data, cmp);
return rb_cmpint(cmp, a, b);
}
static void
nmin_filter(struct nmin_data *data)
{
long n;
VALUE *beg;
int eltsize;
long numelts;
long left, right;
long store_index;
long i, j;
if (data->curlen <= data->n)
return;
n = data->n;
beg = RARRAY_PTR(data->buf);
eltsize = data->by ? 2 : 1;
numelts = data->curlen;
left = 0;
right = numelts-1;
#define GETPTR(i) (beg+(i)*eltsize)
#define SWAP(i, j) do { VALUE tmp[2]; memcpy(tmp, GETPTR(i), sizeof(VALUE)*eltsize); memcpy(GETPTR(i), GETPTR(j), sizeof(VALUE)*eltsize); memcpy(GETPTR(j), tmp, sizeof(VALUE)*eltsize); } while (0)
while (1) {
long pivot_index = left + (right-left)/2;
long num_pivots = 1;
SWAP(pivot_index, right);
pivot_index = right;
store_index = left;
i = left;
while (i <= right-num_pivots) {
int c = data->cmpfunc(GETPTR(i), GETPTR(pivot_index), data);
if (data->rev)
c = -c;
if (c == 0) {
SWAP(i, right-num_pivots);
num_pivots++;
continue;
}
if (c < 0) {
SWAP(i, store_index);
store_index++;
}
i++;
}
j = store_index;
for (i = right; right-num_pivots < i; i--) {
if (i <= j)
break;
SWAP(j, i);
j++;
}
if (store_index <= n && n <= store_index+num_pivots)
break;
if (n < store_index) {
right = store_index-1;
}
else {
left = store_index+num_pivots;
}
}
#undef GETPTR
#undef SWAP
data->limit = RARRAY_AREF(data->buf, store_index*eltsize); 
data->curlen = data->n;
rb_ary_resize(data->buf, data->n * eltsize);
}
static VALUE
nmin_i(RB_BLOCK_CALL_FUNC_ARGLIST(i, _data))
{
struct nmin_data *data = (struct nmin_data *)_data;
VALUE cmpv;
ENUM_WANT_SVALUE();
if (data->by)
cmpv = enum_yield(argc, i);
else
cmpv = i;
if (data->limit != Qundef) {
int c = data->cmpfunc(&cmpv, &data->limit, data);
if (data->rev)
c = -c;
if (c >= 0)
return Qnil;
}
if (data->by)
rb_ary_push(data->buf, cmpv);
rb_ary_push(data->buf, i);
data->curlen++;
if (data->curlen == data->bufmax) {
nmin_filter(data);
}
return Qnil;
}
VALUE
rb_nmin_run(VALUE obj, VALUE num, int by, int rev, int ary)
{
VALUE result;
struct nmin_data data;
data.n = NUM2LONG(num);
if (data.n < 0)
rb_raise(rb_eArgError, "negative size (%ld)", data.n);
if (data.n == 0)
return rb_ary_new2(0);
if (LONG_MAX/4/(by ? 2 : 1) < data.n)
rb_raise(rb_eArgError, "too big size");
data.bufmax = data.n * 4;
data.curlen = 0;
data.buf = rb_ary_tmp_new(data.bufmax * (by ? 2 : 1));
data.limit = Qundef;
data.cmpfunc = by ? nmin_cmp :
rb_block_given_p() ? nmin_block_cmp :
nmin_cmp;
data.rev = rev;
data.by = by;
if (ary) {
long i;
for (i = 0; i < RARRAY_LEN(obj); i++) {
VALUE args[1];
args[0] = RARRAY_AREF(obj, i);
nmin_i(obj, (VALUE)&data, 1, args, Qundef);
}
}
else {
rb_block_call(obj, id_each, 0, 0, nmin_i, (VALUE)&data);
}
nmin_filter(&data);
result = data.buf;
if (by) {
long i;
RARRAY_PTR_USE(result, ptr, {
ruby_qsort(ptr,
RARRAY_LEN(result)/2,
sizeof(VALUE)*2,
data.cmpfunc, (void *)&data);
for (i=1; i<RARRAY_LEN(result); i+=2) {
ptr[i/2] = ptr[i];
}
});
rb_ary_resize(result, RARRAY_LEN(result)/2);
}
else {
RARRAY_PTR_USE(result, ptr, {
ruby_qsort(ptr, RARRAY_LEN(result), sizeof(VALUE),
data.cmpfunc, (void *)&data);
});
}
if (rev) {
rb_ary_reverse(result);
}
RBASIC_SET_CLASS(result, rb_cArray);
return result;
}
static VALUE
enum_one(int argc, VALUE *argv, VALUE obj)
{
struct MEMO *memo = MEMO_ENUM_NEW(Qundef);
VALUE result;
WARN_UNUSED_BLOCK(argc);
rb_block_call(obj, id_each, 0, 0, ENUMFUNC(one), (VALUE)memo);
result = memo->v1;
if (result == Qundef) return Qfalse;
return result;
}
DEFINE_ENUMFUNCS(none)
{
if (RTEST(result)) {
MEMO_V1_SET(memo, Qfalse);
rb_iter_break();
}
return Qnil;
}
static VALUE
enum_none(int argc, VALUE *argv, VALUE obj)
{
struct MEMO *memo = MEMO_ENUM_NEW(Qtrue);
WARN_UNUSED_BLOCK(argc);
rb_block_call(obj, id_each, 0, 0, ENUMFUNC(none), (VALUE)memo);
return memo->v1;
}
struct min_t {
VALUE min;
struct cmp_opt_data cmp_opt;
};
static VALUE
min_i(RB_BLOCK_CALL_FUNC_ARGLIST(i, args))
{
struct min_t *memo = MEMO_FOR(struct min_t, args);
ENUM_WANT_SVALUE();
if (memo->min == Qundef) {
memo->min = i;
}
else {
if (OPTIMIZED_CMP(i, memo->min, memo->cmp_opt) < 0) {
memo->min = i;
}
}
return Qnil;
}
static VALUE
min_ii(RB_BLOCK_CALL_FUNC_ARGLIST(i, args))
{
VALUE cmp;
struct min_t *memo = MEMO_FOR(struct min_t, args);
ENUM_WANT_SVALUE();
if (memo->min == Qundef) {
memo->min = i;
}
else {
cmp = rb_yield_values(2, i, memo->min);
if (rb_cmpint(cmp, i, memo->min) < 0) {
memo->min = i;
}
}
return Qnil;
}
static VALUE
enum_min(int argc, VALUE *argv, VALUE obj)
{
VALUE memo;
struct min_t *m = NEW_CMP_OPT_MEMO(struct min_t, memo);
VALUE result;
VALUE num;
if (rb_check_arity(argc, 0, 1) && !NIL_P(num = argv[0]))
return rb_nmin_run(obj, num, 0, 0, 0);
m->min = Qundef;
m->cmp_opt.opt_methods = 0;
m->cmp_opt.opt_inited = 0;
if (rb_block_given_p()) {
rb_block_call(obj, id_each, 0, 0, min_ii, memo);
}
else {
rb_block_call(obj, id_each, 0, 0, min_i, memo);
}
result = m->min;
if (result == Qundef) return Qnil;
return result;
}
struct max_t {
VALUE max;
struct cmp_opt_data cmp_opt;
};
static VALUE
max_i(RB_BLOCK_CALL_FUNC_ARGLIST(i, args))
{
struct max_t *memo = MEMO_FOR(struct max_t, args);
ENUM_WANT_SVALUE();
if (memo->max == Qundef) {
memo->max = i;
}
else {
if (OPTIMIZED_CMP(i, memo->max, memo->cmp_opt) > 0) {
memo->max = i;
}
}
return Qnil;
}
static VALUE
max_ii(RB_BLOCK_CALL_FUNC_ARGLIST(i, args))
{
struct max_t *memo = MEMO_FOR(struct max_t, args);
VALUE cmp;
ENUM_WANT_SVALUE();
if (memo->max == Qundef) {
memo->max = i;
}
else {
cmp = rb_yield_values(2, i, memo->max);
if (rb_cmpint(cmp, i, memo->max) > 0) {
memo->max = i;
}
}
return Qnil;
}
static VALUE
enum_max(int argc, VALUE *argv, VALUE obj)
{
VALUE memo;
struct max_t *m = NEW_CMP_OPT_MEMO(struct max_t, memo);
VALUE result;
VALUE num;
if (rb_check_arity(argc, 0, 1) && !NIL_P(num = argv[0]))
return rb_nmin_run(obj, num, 0, 1, 0);
m->max = Qundef;
m->cmp_opt.opt_methods = 0;
m->cmp_opt.opt_inited = 0;
if (rb_block_given_p()) {
rb_block_call(obj, id_each, 0, 0, max_ii, (VALUE)memo);
}
else {
rb_block_call(obj, id_each, 0, 0, max_i, (VALUE)memo);
}
result = m->max;
if (result == Qundef) return Qnil;
return result;
}
struct minmax_t {
VALUE min;
VALUE max;
VALUE last;
struct cmp_opt_data cmp_opt;
};
static void
minmax_i_update(VALUE i, VALUE j, struct minmax_t *memo)
{
int n;
if (memo->min == Qundef) {
memo->min = i;
memo->max = j;
}
else {
n = OPTIMIZED_CMP(i, memo->min, memo->cmp_opt);
if (n < 0) {
memo->min = i;
}
n = OPTIMIZED_CMP(j, memo->max, memo->cmp_opt);
if (n > 0) {
memo->max = j;
}
}
}
static VALUE
minmax_i(RB_BLOCK_CALL_FUNC_ARGLIST(i, _memo))
{
struct minmax_t *memo = MEMO_FOR(struct minmax_t, _memo);
int n;
VALUE j;
ENUM_WANT_SVALUE();
if (memo->last == Qundef) {
memo->last = i;
return Qnil;
}
j = memo->last;
memo->last = Qundef;
n = OPTIMIZED_CMP(j, i, memo->cmp_opt);
if (n == 0)
i = j;
else if (n < 0) {
VALUE tmp;
tmp = i;
i = j;
j = tmp;
}
minmax_i_update(i, j, memo);
return Qnil;
}
static void
minmax_ii_update(VALUE i, VALUE j, struct minmax_t *memo)
{
int n;
if (memo->min == Qundef) {
memo->min = i;
memo->max = j;
}
else {
n = rb_cmpint(rb_yield_values(2, i, memo->min), i, memo->min);
if (n < 0) {
memo->min = i;
}
n = rb_cmpint(rb_yield_values(2, j, memo->max), j, memo->max);
if (n > 0) {
memo->max = j;
}
}
}
static VALUE
minmax_ii(RB_BLOCK_CALL_FUNC_ARGLIST(i, _memo))
{
struct minmax_t *memo = MEMO_FOR(struct minmax_t, _memo);
int n;
VALUE j;
ENUM_WANT_SVALUE();
if (memo->last == Qundef) {
memo->last = i;
return Qnil;
}
j = memo->last;
memo->last = Qundef;
n = rb_cmpint(rb_yield_values(2, j, i), j, i);
if (n == 0)
i = j;
else if (n < 0) {
VALUE tmp;
tmp = i;
i = j;
j = tmp;
}
minmax_ii_update(i, j, memo);
return Qnil;
}
static VALUE
enum_minmax(VALUE obj)
{
VALUE memo;
struct minmax_t *m = NEW_CMP_OPT_MEMO(struct minmax_t, memo);
m->min = Qundef;
m->last = Qundef;
m->cmp_opt.opt_methods = 0;
m->cmp_opt.opt_inited = 0;
if (rb_block_given_p()) {
rb_block_call(obj, id_each, 0, 0, minmax_ii, memo);
if (m->last != Qundef)
minmax_ii_update(m->last, m->last, m);
}
else {
rb_block_call(obj, id_each, 0, 0, minmax_i, memo);
if (m->last != Qundef)
minmax_i_update(m->last, m->last, m);
}
if (m->min != Qundef) {
return rb_assoc_new(m->min, m->max);
}
return rb_assoc_new(Qnil, Qnil);
}
static VALUE
min_by_i(RB_BLOCK_CALL_FUNC_ARGLIST(i, args))
{
struct cmp_opt_data cmp_opt = { 0, 0 };
struct MEMO *memo = MEMO_CAST(args);
VALUE v;
ENUM_WANT_SVALUE();
v = enum_yield(argc, i);
if (memo->v1 == Qundef) {
MEMO_V1_SET(memo, v);
MEMO_V2_SET(memo, i);
}
else if (OPTIMIZED_CMP(v, memo->v1, cmp_opt) < 0) {
MEMO_V1_SET(memo, v);
MEMO_V2_SET(memo, i);
}
return Qnil;
}
static VALUE
enum_min_by(int argc, VALUE *argv, VALUE obj)
{
struct MEMO *memo;
VALUE num;
rb_check_arity(argc, 0, 1);
RETURN_SIZED_ENUMERATOR(obj, argc, argv, enum_size);
if (argc && !NIL_P(num = argv[0]))
return rb_nmin_run(obj, num, 1, 0, 0);
memo = MEMO_NEW(Qundef, Qnil, 0);
rb_block_call(obj, id_each, 0, 0, min_by_i, (VALUE)memo);
return memo->v2;
}
static VALUE
max_by_i(RB_BLOCK_CALL_FUNC_ARGLIST(i, args))
{
struct cmp_opt_data cmp_opt = { 0, 0 };
struct MEMO *memo = MEMO_CAST(args);
VALUE v;
ENUM_WANT_SVALUE();
v = enum_yield(argc, i);
if (memo->v1 == Qundef) {
MEMO_V1_SET(memo, v);
MEMO_V2_SET(memo, i);
}
else if (OPTIMIZED_CMP(v, memo->v1, cmp_opt) > 0) {
MEMO_V1_SET(memo, v);
MEMO_V2_SET(memo, i);
}
return Qnil;
}
static VALUE
enum_max_by(int argc, VALUE *argv, VALUE obj)
{
struct MEMO *memo;
VALUE num;
rb_check_arity(argc, 0, 1);
RETURN_SIZED_ENUMERATOR(obj, argc, argv, enum_size);
if (argc && !NIL_P(num = argv[0]))
return rb_nmin_run(obj, num, 1, 1, 0);
memo = MEMO_NEW(Qundef, Qnil, 0);
rb_block_call(obj, id_each, 0, 0, max_by_i, (VALUE)memo);
return memo->v2;
}
struct minmax_by_t {
VALUE min_bv;
VALUE max_bv;
VALUE min;
VALUE max;
VALUE last_bv;
VALUE last;
};
static void
minmax_by_i_update(VALUE v1, VALUE v2, VALUE i1, VALUE i2, struct minmax_by_t *memo)
{
struct cmp_opt_data cmp_opt = { 0, 0 };
if (memo->min_bv == Qundef) {
memo->min_bv = v1;
memo->max_bv = v2;
memo->min = i1;
memo->max = i2;
}
else {
if (OPTIMIZED_CMP(v1, memo->min_bv, cmp_opt) < 0) {
memo->min_bv = v1;
memo->min = i1;
}
if (OPTIMIZED_CMP(v2, memo->max_bv, cmp_opt) > 0) {
memo->max_bv = v2;
memo->max = i2;
}
}
}
static VALUE
minmax_by_i(RB_BLOCK_CALL_FUNC_ARGLIST(i, _memo))
{
struct cmp_opt_data cmp_opt = { 0, 0 };
struct minmax_by_t *memo = MEMO_FOR(struct minmax_by_t, _memo);
VALUE vi, vj, j;
int n;
ENUM_WANT_SVALUE();
vi = enum_yield(argc, i);
if (memo->last_bv == Qundef) {
memo->last_bv = vi;
memo->last = i;
return Qnil;
}
vj = memo->last_bv;
j = memo->last;
memo->last_bv = Qundef;
n = OPTIMIZED_CMP(vj, vi, cmp_opt);
if (n == 0) {
i = j;
vi = vj;
}
else if (n < 0) {
VALUE tmp;
tmp = i;
i = j;
j = tmp;
tmp = vi;
vi = vj;
vj = tmp;
}
minmax_by_i_update(vi, vj, i, j, memo);
return Qnil;
}
static VALUE
enum_minmax_by(VALUE obj)
{
VALUE memo;
struct minmax_by_t *m = NEW_MEMO_FOR(struct minmax_by_t, memo);
RETURN_SIZED_ENUMERATOR(obj, 0, 0, enum_size);
m->min_bv = Qundef;
m->max_bv = Qundef;
m->min = Qnil;
m->max = Qnil;
m->last_bv = Qundef;
m->last = Qundef;
rb_block_call(obj, id_each, 0, 0, minmax_by_i, memo);
if (m->last_bv != Qundef)
minmax_by_i_update(m->last_bv, m->last_bv, m->last, m->last, m);
m = MEMO_FOR(struct minmax_by_t, memo);
return rb_assoc_new(m->min, m->max);
}
static VALUE
member_i(RB_BLOCK_CALL_FUNC_ARGLIST(iter, args))
{
struct MEMO *memo = MEMO_CAST(args);
if (rb_equal(rb_enum_values_pack(argc, argv), memo->v1)) {
MEMO_V2_SET(memo, Qtrue);
rb_iter_break();
}
return Qnil;
}
static VALUE
enum_member(VALUE obj, VALUE val)
{
struct MEMO *memo = MEMO_NEW(val, Qfalse, 0);
rb_block_call(obj, id_each, 0, 0, member_i, (VALUE)memo);
return memo->v2;
}
static VALUE
each_with_index_i(RB_BLOCK_CALL_FUNC_ARGLIST(i, memo))
{
struct MEMO *m = MEMO_CAST(memo);
VALUE n = imemo_count_value(m);
imemo_count_up(m);
return rb_yield_values(2, rb_enum_values_pack(argc, argv), n);
}
static VALUE
enum_each_with_index(int argc, VALUE *argv, VALUE obj)
{
struct MEMO *memo;
RETURN_SIZED_ENUMERATOR(obj, argc, argv, enum_size);
memo = MEMO_NEW(0, 0, 0);
rb_block_call(obj, id_each, argc, argv, each_with_index_i, (VALUE)memo);
return obj;
}
static VALUE
enum_reverse_each(int argc, VALUE *argv, VALUE obj)
{
VALUE ary;
long len;
RETURN_SIZED_ENUMERATOR(obj, argc, argv, enum_size);
ary = enum_to_a(argc, argv, obj);
len = RARRAY_LEN(ary);
while (len--) {
long nlen;
rb_yield(RARRAY_AREF(ary, len));
nlen = RARRAY_LEN(ary);
if (nlen < len) {
len = nlen;
}
}
return obj;
}
static VALUE
each_val_i(RB_BLOCK_CALL_FUNC_ARGLIST(i, p))
{
ENUM_WANT_SVALUE();
enum_yield(argc, i);
return Qnil;
}
static VALUE
enum_each_entry(int argc, VALUE *argv, VALUE obj)
{
RETURN_SIZED_ENUMERATOR(obj, argc, argv, enum_size);
rb_block_call(obj, id_each, argc, argv, each_val_i, 0);
return obj;
}
static VALUE
add_int(VALUE x, long n)
{
const VALUE y = LONG2NUM(n);
if (RB_INTEGER_TYPE_P(x)) return rb_int_plus(x, y);
return rb_funcallv(x, '+', 1, &y);
}
static VALUE
div_int(VALUE x, long n)
{
const VALUE y = LONG2NUM(n);
if (RB_INTEGER_TYPE_P(x)) return rb_int_idiv(x, y);
return rb_funcallv(x, id_div, 1, &y);
}
#define dont_recycle_block_arg(arity) ((arity) == 1 || (arity) < 0)
static VALUE
each_slice_i(RB_BLOCK_CALL_FUNC_ARGLIST(i, m))
{
struct MEMO *memo = MEMO_CAST(m);
VALUE ary = memo->v1;
VALUE v = Qnil;
long size = memo->u3.cnt;
ENUM_WANT_SVALUE();
rb_ary_push(ary, i);
if (RARRAY_LEN(ary) == size) {
v = rb_yield(ary);
if (memo->v2) {
MEMO_V1_SET(memo, rb_ary_new2(size));
}
else {
rb_ary_clear(ary);
}
}
return v;
}
static VALUE
enum_each_slice_size(VALUE obj, VALUE args, VALUE eobj)
{
VALUE n, size;
long slice_size = NUM2LONG(RARRAY_AREF(args, 0));
ID infinite_p;
CONST_ID(infinite_p, "infinite?");
if (slice_size <= 0) rb_raise(rb_eArgError, "invalid slice size");
size = enum_size(obj, 0, 0);
if (size == Qnil) return Qnil;
if (RB_FLOAT_TYPE_P(size) && RTEST(rb_funcall(size, infinite_p, 0))) {
return size;
}
n = add_int(size, slice_size-1);
return div_int(n, slice_size);
}
static VALUE
enum_each_slice(VALUE obj, VALUE n)
{
long size = NUM2LONG(n);
VALUE ary;
struct MEMO *memo;
int arity;
if (size <= 0) rb_raise(rb_eArgError, "invalid slice size");
RETURN_SIZED_ENUMERATOR(obj, 1, &n, enum_each_slice_size);
size = limit_by_enum_size(obj, size);
ary = rb_ary_new2(size);
arity = rb_block_arity();
memo = MEMO_NEW(ary, dont_recycle_block_arg(arity), size);
rb_block_call(obj, id_each, 0, 0, each_slice_i, (VALUE)memo);
ary = memo->v1;
if (RARRAY_LEN(ary) > 0) rb_yield(ary);
return Qnil;
}
static VALUE
each_cons_i(RB_BLOCK_CALL_FUNC_ARGLIST(i, args))
{
struct MEMO *memo = MEMO_CAST(args);
VALUE ary = memo->v1;
VALUE v = Qnil;
long size = memo->u3.cnt;
ENUM_WANT_SVALUE();
if (RARRAY_LEN(ary) == size) {
rb_ary_shift(ary);
}
rb_ary_push(ary, i);
if (RARRAY_LEN(ary) == size) {
if (memo->v2) {
ary = rb_ary_dup(ary);
}
v = rb_yield(ary);
}
return v;
}
static VALUE
enum_each_cons_size(VALUE obj, VALUE args, VALUE eobj)
{
struct cmp_opt_data cmp_opt = { 0, 0 };
const VALUE zero = LONG2FIX(0);
VALUE n, size;
long cons_size = NUM2LONG(RARRAY_AREF(args, 0));
if (cons_size <= 0) rb_raise(rb_eArgError, "invalid size");
size = enum_size(obj, 0, 0);
if (size == Qnil) return Qnil;
n = add_int(size, 1 - cons_size);
return (OPTIMIZED_CMP(n, zero, cmp_opt) == -1) ? zero : n;
}
static VALUE
enum_each_cons(VALUE obj, VALUE n)
{
long size = NUM2LONG(n);
struct MEMO *memo;
int arity;
if (size <= 0) rb_raise(rb_eArgError, "invalid size");
RETURN_SIZED_ENUMERATOR(obj, 1, &n, enum_each_cons_size);
arity = rb_block_arity();
if (enum_size_over_p(obj, size)) return Qnil;
memo = MEMO_NEW(rb_ary_new2(size), dont_recycle_block_arg(arity), size);
rb_block_call(obj, id_each, 0, 0, each_cons_i, (VALUE)memo);
return Qnil;
}
static VALUE
each_with_object_i(RB_BLOCK_CALL_FUNC_ARGLIST(i, memo))
{
ENUM_WANT_SVALUE();
return rb_yield_values(2, i, memo);
}
static VALUE
enum_each_with_object(VALUE obj, VALUE memo)
{
RETURN_SIZED_ENUMERATOR(obj, 1, &memo, enum_size);
rb_block_call(obj, id_each, 0, 0, each_with_object_i, memo);
return memo;
}
static VALUE
zip_ary(RB_BLOCK_CALL_FUNC_ARGLIST(val, memoval))
{
struct MEMO *memo = (struct MEMO *)memoval;
VALUE result = memo->v1;
VALUE args = memo->v2;
long n = memo->u3.cnt++;
VALUE tmp;
int i;
tmp = rb_ary_new2(RARRAY_LEN(args) + 1);
rb_ary_store(tmp, 0, rb_enum_values_pack(argc, argv));
for (i=0; i<RARRAY_LEN(args); i++) {
VALUE e = RARRAY_AREF(args, i);
if (RARRAY_LEN(e) <= n) {
rb_ary_push(tmp, Qnil);
}
else {
rb_ary_push(tmp, RARRAY_AREF(e, n));
}
}
if (NIL_P(result)) {
enum_yield_array(tmp);
}
else {
rb_ary_push(result, tmp);
}
RB_GC_GUARD(args);
return Qnil;
}
static VALUE
call_next(VALUE w)
{
VALUE *v = (VALUE *)w;
return v[0] = rb_funcallv(v[1], id_next, 0, 0);
}
static VALUE
call_stop(VALUE w, VALUE _)
{
VALUE *v = (VALUE *)w;
return v[0] = Qundef;
}
static VALUE
zip_i(RB_BLOCK_CALL_FUNC_ARGLIST(val, memoval))
{
struct MEMO *memo = (struct MEMO *)memoval;
VALUE result = memo->v1;
VALUE args = memo->v2;
VALUE tmp;
int i;
tmp = rb_ary_new2(RARRAY_LEN(args) + 1);
rb_ary_store(tmp, 0, rb_enum_values_pack(argc, argv));
for (i=0; i<RARRAY_LEN(args); i++) {
if (NIL_P(RARRAY_AREF(args, i))) {
rb_ary_push(tmp, Qnil);
}
else {
VALUE v[2];
v[1] = RARRAY_AREF(args, i);
rb_rescue2(call_next, (VALUE)v, call_stop, (VALUE)v, rb_eStopIteration, (VALUE)0);
if (v[0] == Qundef) {
RARRAY_ASET(args, i, Qnil);
v[0] = Qnil;
}
rb_ary_push(tmp, v[0]);
}
}
if (NIL_P(result)) {
enum_yield_array(tmp);
}
else {
rb_ary_push(result, tmp);
}
RB_GC_GUARD(args);
return Qnil;
}
static VALUE
enum_zip(int argc, VALUE *argv, VALUE obj)
{
int i;
ID conv;
struct MEMO *memo;
VALUE result = Qnil;
VALUE args = rb_ary_new4(argc, argv);
int allary = TRUE;
argv = RARRAY_PTR(args);
for (i=0; i<argc; i++) {
VALUE ary = rb_check_array_type(argv[i]);
if (NIL_P(ary)) {
allary = FALSE;
break;
}
argv[i] = ary;
}
if (!allary) {
static const VALUE sym_each = STATIC_ID2SYM(id_each);
CONST_ID(conv, "to_enum");
for (i=0; i<argc; i++) {
if (!rb_respond_to(argv[i], id_each)) {
rb_raise(rb_eTypeError, "wrong argument type %"PRIsVALUE" (must respond to :each)",
rb_obj_class(argv[i]));
}
argv[i] = rb_funcallv(argv[i], conv, 1, &sym_each);
}
}
if (!rb_block_given_p()) {
result = rb_ary_new();
}
memo = MEMO_NEW(result, args, 0);
rb_block_call(obj, id_each, 0, 0, allary ? zip_ary : zip_i, (VALUE)memo);
return result;
}
static VALUE
take_i(RB_BLOCK_CALL_FUNC_ARGLIST(i, args))
{
struct MEMO *memo = MEMO_CAST(args);
rb_ary_push(memo->v1, rb_enum_values_pack(argc, argv));
if (--memo->u3.cnt == 0) rb_iter_break();
return Qnil;
}
static VALUE
enum_take(VALUE obj, VALUE n)
{
struct MEMO *memo;
VALUE result;
long len = NUM2LONG(n);
if (len < 0) {
rb_raise(rb_eArgError, "attempt to take negative size");
}
if (len == 0) return rb_ary_new2(0);
result = rb_ary_new2(len);
memo = MEMO_NEW(result, 0, len);
rb_block_call(obj, id_each, 0, 0, take_i, (VALUE)memo);
return result;
}
static VALUE
take_while_i(RB_BLOCK_CALL_FUNC_ARGLIST(i, ary))
{
if (!RTEST(rb_yield_values2(argc, argv))) rb_iter_break();
rb_ary_push(ary, rb_enum_values_pack(argc, argv));
return Qnil;
}
static VALUE
enum_take_while(VALUE obj)
{
VALUE ary;
RETURN_ENUMERATOR(obj, 0, 0);
ary = rb_ary_new();
rb_block_call(obj, id_each, 0, 0, take_while_i, ary);
return ary;
}
static VALUE
drop_i(RB_BLOCK_CALL_FUNC_ARGLIST(i, args))
{
struct MEMO *memo = MEMO_CAST(args);
if (memo->u3.cnt == 0) {
rb_ary_push(memo->v1, rb_enum_values_pack(argc, argv));
}
else {
memo->u3.cnt--;
}
return Qnil;
}
static VALUE
enum_drop(VALUE obj, VALUE n)
{
VALUE result;
struct MEMO *memo;
long len = NUM2LONG(n);
if (len < 0) {
rb_raise(rb_eArgError, "attempt to drop negative size");
}
result = rb_ary_new();
memo = MEMO_NEW(result, 0, len);
rb_block_call(obj, id_each, 0, 0, drop_i, (VALUE)memo);
return result;
}
static VALUE
drop_while_i(RB_BLOCK_CALL_FUNC_ARGLIST(i, args))
{
struct MEMO *memo = MEMO_CAST(args);
ENUM_WANT_SVALUE();
if (!memo->u3.state && !RTEST(enum_yield(argc, i))) {
memo->u3.state = TRUE;
}
if (memo->u3.state) {
rb_ary_push(memo->v1, i);
}
return Qnil;
}
static VALUE
enum_drop_while(VALUE obj)
{
VALUE result;
struct MEMO *memo;
RETURN_ENUMERATOR(obj, 0, 0);
result = rb_ary_new();
memo = MEMO_NEW(result, 0, FALSE);
rb_block_call(obj, id_each, 0, 0, drop_while_i, (VALUE)memo);
return result;
}
static VALUE
cycle_i(RB_BLOCK_CALL_FUNC_ARGLIST(i, ary))
{
ENUM_WANT_SVALUE();
rb_ary_push(ary, argc > 1 ? i : rb_ary_new_from_values(argc, argv));
enum_yield(argc, i);
return Qnil;
}
static VALUE
enum_cycle_size(VALUE self, VALUE args, VALUE eobj)
{
long mul = 0;
VALUE n = Qnil;
VALUE size;
if (args && (RARRAY_LEN(args) > 0)) {
n = RARRAY_AREF(args, 0);
if (!NIL_P(n)) mul = NUM2LONG(n);
}
size = enum_size(self, args, 0);
if (NIL_P(size) || FIXNUM_ZERO_P(size)) return size;
if (NIL_P(n)) return DBL2NUM(HUGE_VAL);
if (mul <= 0) return INT2FIX(0);
n = LONG2FIX(mul);
return rb_funcallv(size, '*', 1, &n);
}
static VALUE
enum_cycle(int argc, VALUE *argv, VALUE obj)
{
VALUE ary;
VALUE nv = Qnil;
long n, i, len;
rb_check_arity(argc, 0, 1);
RETURN_SIZED_ENUMERATOR(obj, argc, argv, enum_cycle_size);
if (!argc || NIL_P(nv = argv[0])) {
n = -1;
}
else {
n = NUM2LONG(nv);
if (n <= 0) return Qnil;
}
ary = rb_ary_new();
RBASIC_CLEAR_CLASS(ary);
rb_block_call(obj, id_each, 0, 0, cycle_i, ary);
len = RARRAY_LEN(ary);
if (len == 0) return Qnil;
while (n < 0 || 0 < --n) {
for (i=0; i<len; i++) {
enum_yield_array(RARRAY_AREF(ary, i));
}
}
return Qnil;
}
struct chunk_arg {
VALUE categorize;
VALUE prev_value;
VALUE prev_elts;
VALUE yielder;
};
static VALUE
chunk_ii(RB_BLOCK_CALL_FUNC_ARGLIST(i, _argp))
{
struct chunk_arg *argp = MEMO_FOR(struct chunk_arg, _argp);
VALUE v, s;
VALUE alone = ID2SYM(rb_intern("_alone"));
VALUE separator = ID2SYM(rb_intern("_separator"));
ENUM_WANT_SVALUE();
v = rb_funcallv(argp->categorize, id_call, 1, &i);
if (v == alone) {
if (!NIL_P(argp->prev_value)) {
s = rb_assoc_new(argp->prev_value, argp->prev_elts);
rb_funcallv(argp->yielder, id_lshift, 1, &s);
argp->prev_value = argp->prev_elts = Qnil;
}
v = rb_assoc_new(v, rb_ary_new3(1, i));
rb_funcallv(argp->yielder, id_lshift, 1, &v);
}
else if (NIL_P(v) || v == separator) {
if (!NIL_P(argp->prev_value)) {
v = rb_assoc_new(argp->prev_value, argp->prev_elts);
rb_funcallv(argp->yielder, id_lshift, 1, &v);
argp->prev_value = argp->prev_elts = Qnil;
}
}
else if (SYMBOL_P(v) && (s = rb_sym2str(v), RSTRING_PTR(s)[0] == '_')) {
rb_raise(rb_eRuntimeError, "symbols beginning with an underscore are reserved");
}
else {
if (NIL_P(argp->prev_value)) {
argp->prev_value = v;
argp->prev_elts = rb_ary_new3(1, i);
}
else {
if (rb_equal(argp->prev_value, v)) {
rb_ary_push(argp->prev_elts, i);
}
else {
s = rb_assoc_new(argp->prev_value, argp->prev_elts);
rb_funcallv(argp->yielder, id_lshift, 1, &s);
argp->prev_value = v;
argp->prev_elts = rb_ary_new3(1, i);
}
}
}
return Qnil;
}
static VALUE
chunk_i(RB_BLOCK_CALL_FUNC_ARGLIST(yielder, enumerator))
{
VALUE enumerable;
VALUE arg;
struct chunk_arg *memo = NEW_MEMO_FOR(struct chunk_arg, arg);
enumerable = rb_ivar_get(enumerator, rb_intern("chunk_enumerable"));
memo->categorize = rb_ivar_get(enumerator, rb_intern("chunk_categorize"));
memo->prev_value = Qnil;
memo->prev_elts = Qnil;
memo->yielder = yielder;
rb_block_call(enumerable, id_each, 0, 0, chunk_ii, arg);
memo = MEMO_FOR(struct chunk_arg, arg);
if (!NIL_P(memo->prev_elts)) {
arg = rb_assoc_new(memo->prev_value, memo->prev_elts);
rb_funcallv(memo->yielder, id_lshift, 1, &arg);
}
return Qnil;
}
static VALUE
enum_chunk(VALUE enumerable)
{
VALUE enumerator;
RETURN_SIZED_ENUMERATOR(enumerable, 0, 0, enum_size);
enumerator = rb_obj_alloc(rb_cEnumerator);
rb_ivar_set(enumerator, rb_intern("chunk_enumerable"), enumerable);
rb_ivar_set(enumerator, rb_intern("chunk_categorize"), rb_block_proc());
rb_block_call(enumerator, idInitialize, 0, 0, chunk_i, enumerator);
return enumerator;
}
struct slicebefore_arg {
VALUE sep_pred;
VALUE sep_pat;
VALUE prev_elts;
VALUE yielder;
};
static VALUE
slicebefore_ii(RB_BLOCK_CALL_FUNC_ARGLIST(i, _argp))
{
struct slicebefore_arg *argp = MEMO_FOR(struct slicebefore_arg, _argp);
VALUE header_p;
ENUM_WANT_SVALUE();
if (!NIL_P(argp->sep_pat))
header_p = rb_funcallv(argp->sep_pat, id_eqq, 1, &i);
else
header_p = rb_funcallv(argp->sep_pred, id_call, 1, &i);
if (RTEST(header_p)) {
if (!NIL_P(argp->prev_elts))
rb_funcallv(argp->yielder, id_lshift, 1, &argp->prev_elts);
argp->prev_elts = rb_ary_new3(1, i);
}
else {
if (NIL_P(argp->prev_elts))
argp->prev_elts = rb_ary_new3(1, i);
else
rb_ary_push(argp->prev_elts, i);
}
return Qnil;
}
static VALUE
slicebefore_i(RB_BLOCK_CALL_FUNC_ARGLIST(yielder, enumerator))
{
VALUE enumerable;
VALUE arg;
struct slicebefore_arg *memo = NEW_MEMO_FOR(struct slicebefore_arg, arg);
enumerable = rb_ivar_get(enumerator, rb_intern("slicebefore_enumerable"));
memo->sep_pred = rb_attr_get(enumerator, rb_intern("slicebefore_sep_pred"));
memo->sep_pat = NIL_P(memo->sep_pred) ? rb_ivar_get(enumerator, rb_intern("slicebefore_sep_pat")) : Qnil;
memo->prev_elts = Qnil;
memo->yielder = yielder;
rb_block_call(enumerable, id_each, 0, 0, slicebefore_ii, arg);
memo = MEMO_FOR(struct slicebefore_arg, arg);
if (!NIL_P(memo->prev_elts))
rb_funcallv(memo->yielder, id_lshift, 1, &memo->prev_elts);
return Qnil;
}
static VALUE
enum_slice_before(int argc, VALUE *argv, VALUE enumerable)
{
VALUE enumerator;
if (rb_block_given_p()) {
if (argc != 0)
rb_error_arity(argc, 0, 0);
enumerator = rb_obj_alloc(rb_cEnumerator);
rb_ivar_set(enumerator, rb_intern("slicebefore_sep_pred"), rb_block_proc());
}
else {
VALUE sep_pat;
rb_scan_args(argc, argv, "1", &sep_pat);
enumerator = rb_obj_alloc(rb_cEnumerator);
rb_ivar_set(enumerator, rb_intern("slicebefore_sep_pat"), sep_pat);
}
rb_ivar_set(enumerator, rb_intern("slicebefore_enumerable"), enumerable);
rb_block_call(enumerator, idInitialize, 0, 0, slicebefore_i, enumerator);
return enumerator;
}
struct sliceafter_arg {
VALUE pat;
VALUE pred;
VALUE prev_elts;
VALUE yielder;
};
static VALUE
sliceafter_ii(RB_BLOCK_CALL_FUNC_ARGLIST(i, _memo))
{
#define UPDATE_MEMO ((void)(memo = MEMO_FOR(struct sliceafter_arg, _memo)))
struct sliceafter_arg *memo;
int split_p;
UPDATE_MEMO;
ENUM_WANT_SVALUE();
if (NIL_P(memo->prev_elts)) {
memo->prev_elts = rb_ary_new3(1, i);
}
else {
rb_ary_push(memo->prev_elts, i);
}
if (NIL_P(memo->pred)) {
split_p = RTEST(rb_funcallv(memo->pat, id_eqq, 1, &i));
UPDATE_MEMO;
}
else {
split_p = RTEST(rb_funcallv(memo->pred, id_call, 1, &i));
UPDATE_MEMO;
}
if (split_p) {
rb_funcallv(memo->yielder, id_lshift, 1, &memo->prev_elts);
UPDATE_MEMO;
memo->prev_elts = Qnil;
}
return Qnil;
#undef UPDATE_MEMO
}
static VALUE
sliceafter_i(RB_BLOCK_CALL_FUNC_ARGLIST(yielder, enumerator))
{
VALUE enumerable;
VALUE arg;
struct sliceafter_arg *memo = NEW_MEMO_FOR(struct sliceafter_arg, arg);
enumerable = rb_ivar_get(enumerator, rb_intern("sliceafter_enum"));
memo->pat = rb_ivar_get(enumerator, rb_intern("sliceafter_pat"));
memo->pred = rb_attr_get(enumerator, rb_intern("sliceafter_pred"));
memo->prev_elts = Qnil;
memo->yielder = yielder;
rb_block_call(enumerable, id_each, 0, 0, sliceafter_ii, arg);
memo = MEMO_FOR(struct sliceafter_arg, arg);
if (!NIL_P(memo->prev_elts))
rb_funcallv(memo->yielder, id_lshift, 1, &memo->prev_elts);
return Qnil;
}
static VALUE
enum_slice_after(int argc, VALUE *argv, VALUE enumerable)
{
VALUE enumerator;
VALUE pat = Qnil, pred = Qnil;
if (rb_block_given_p()) {
if (0 < argc)
rb_raise(rb_eArgError, "both pattern and block are given");
pred = rb_block_proc();
}
else {
rb_scan_args(argc, argv, "1", &pat);
}
enumerator = rb_obj_alloc(rb_cEnumerator);
rb_ivar_set(enumerator, rb_intern("sliceafter_enum"), enumerable);
rb_ivar_set(enumerator, rb_intern("sliceafter_pat"), pat);
rb_ivar_set(enumerator, rb_intern("sliceafter_pred"), pred);
rb_block_call(enumerator, idInitialize, 0, 0, sliceafter_i, enumerator);
return enumerator;
}
struct slicewhen_arg {
VALUE pred;
VALUE prev_elt;
VALUE prev_elts;
VALUE yielder;
int inverted; 
};
static VALUE
slicewhen_ii(RB_BLOCK_CALL_FUNC_ARGLIST(i, _memo))
{
#define UPDATE_MEMO ((void)(memo = MEMO_FOR(struct slicewhen_arg, _memo)))
struct slicewhen_arg *memo;
int split_p;
UPDATE_MEMO;
ENUM_WANT_SVALUE();
if (memo->prev_elt == Qundef) {
memo->prev_elt = i;
memo->prev_elts = rb_ary_new3(1, i);
}
else {
VALUE args[2];
args[0] = memo->prev_elt;
args[1] = i;
split_p = RTEST(rb_funcallv(memo->pred, id_call, 2, args));
UPDATE_MEMO;
if (memo->inverted)
split_p = !split_p;
if (split_p) {
rb_funcallv(memo->yielder, id_lshift, 1, &memo->prev_elts);
UPDATE_MEMO;
memo->prev_elts = rb_ary_new3(1, i);
}
else {
rb_ary_push(memo->prev_elts, i);
}
memo->prev_elt = i;
}
return Qnil;
#undef UPDATE_MEMO
}
static VALUE
slicewhen_i(RB_BLOCK_CALL_FUNC_ARGLIST(yielder, enumerator))
{
VALUE enumerable;
VALUE arg;
struct slicewhen_arg *memo =
NEW_PARTIAL_MEMO_FOR(struct slicewhen_arg, arg, inverted);
enumerable = rb_ivar_get(enumerator, rb_intern("slicewhen_enum"));
memo->pred = rb_attr_get(enumerator, rb_intern("slicewhen_pred"));
memo->prev_elt = Qundef;
memo->prev_elts = Qnil;
memo->yielder = yielder;
memo->inverted = RTEST(rb_attr_get(enumerator, rb_intern("slicewhen_inverted")));
rb_block_call(enumerable, id_each, 0, 0, slicewhen_ii, arg);
memo = MEMO_FOR(struct slicewhen_arg, arg);
if (!NIL_P(memo->prev_elts))
rb_funcallv(memo->yielder, id_lshift, 1, &memo->prev_elts);
return Qnil;
}
static VALUE
enum_slice_when(VALUE enumerable)
{
VALUE enumerator;
VALUE pred;
pred = rb_block_proc();
enumerator = rb_obj_alloc(rb_cEnumerator);
rb_ivar_set(enumerator, rb_intern("slicewhen_enum"), enumerable);
rb_ivar_set(enumerator, rb_intern("slicewhen_pred"), pred);
rb_ivar_set(enumerator, rb_intern("slicewhen_inverted"), Qfalse);
rb_block_call(enumerator, idInitialize, 0, 0, slicewhen_i, enumerator);
return enumerator;
}
static VALUE
enum_chunk_while(VALUE enumerable)
{
VALUE enumerator;
VALUE pred;
pred = rb_block_proc();
enumerator = rb_obj_alloc(rb_cEnumerator);
rb_ivar_set(enumerator, rb_intern("slicewhen_enum"), enumerable);
rb_ivar_set(enumerator, rb_intern("slicewhen_pred"), pred);
rb_ivar_set(enumerator, rb_intern("slicewhen_inverted"), Qtrue);
rb_block_call(enumerator, idInitialize, 0, 0, slicewhen_i, enumerator);
return enumerator;
}
struct enum_sum_memo {
VALUE v, r;
long n;
double f, c;
int block_given;
int float_value;
};
static void
sum_iter(VALUE i, struct enum_sum_memo *memo)
{
const int unused = (assert(memo != NULL), 0);
long n = memo->n;
VALUE v = memo->v;
VALUE r = memo->r;
double f = memo->f;
double c = memo->c;
if (memo->block_given)
i = rb_yield(i);
if (memo->float_value)
goto float_value;
if (FIXNUM_P(v) || RB_TYPE_P(v, T_BIGNUM) || RB_TYPE_P(v, T_RATIONAL)) {
if (FIXNUM_P(i)) {
n += FIX2LONG(i); 
if (!FIXABLE(n)) {
v = rb_big_plus(LONG2NUM(n), v);
n = 0;
}
}
else if (RB_TYPE_P(i, T_BIGNUM))
v = rb_big_plus(i, v);
else if (RB_TYPE_P(i, T_RATIONAL)) {
if (r == Qundef)
r = i;
else
r = rb_rational_plus(r, i);
}
else {
if (n != 0) {
v = rb_fix_plus(LONG2FIX(n), v);
n = 0;
}
if (r != Qundef) {
if (FIXNUM_P(r))
v = rb_fix_plus(r, v);
else if (RB_TYPE_P(r, T_BIGNUM))
v = rb_big_plus(r, v);
else
v = rb_rational_plus(r, v);
r = Qundef;
}
if (RB_FLOAT_TYPE_P(i)) {
f = NUM2DBL(v);
c = 0.0;
memo->float_value = 1;
goto float_value;
}
else
goto some_value;
}
}
else if (RB_FLOAT_TYPE_P(v)) {
double x, t;
float_value:
if (RB_FLOAT_TYPE_P(i))
x = RFLOAT_VALUE(i);
else if (FIXNUM_P(i))
x = FIX2LONG(i);
else if (RB_TYPE_P(i, T_BIGNUM))
x = rb_big2dbl(i);
else if (RB_TYPE_P(i, T_RATIONAL))
x = rb_num2dbl(i);
else {
v = DBL2NUM(f);
memo->float_value = 0;
goto some_value;
}
if (isnan(f)) return;
if (isnan(x)) {
memo->v = i;
memo->f = x;
return;
}
if (isinf(x)) {
if (isinf(f) && signbit(x) != signbit(f)) {
memo->f = NAN;
memo->v = DBL2NUM(f);
}
else {
memo->f = x;
memo->v = i;
}
return;
}
if (isinf(f)) return;
t = f + x;
if (fabs(f) >= fabs(x))
c += ((f - t) + x);
else
c += ((x - t) + f);
f = t;
}
else {
some_value:
v = rb_funcallv(v, idPLUS, 1, &i);
}
memo->v = v;
memo->n = n;
memo->r = r;
memo->f = f;
memo->c = c;
(void)unused;
}
static VALUE
enum_sum_i(RB_BLOCK_CALL_FUNC_ARGLIST(i, args))
{
ENUM_WANT_SVALUE();
sum_iter(i, (struct enum_sum_memo *) args);
return Qnil;
}
static int
hash_sum_i(VALUE key, VALUE value, VALUE arg)
{
sum_iter(rb_assoc_new(key, value), (struct enum_sum_memo *) arg);
return ST_CONTINUE;
}
static void
hash_sum(VALUE hash, struct enum_sum_memo *memo)
{
assert(RB_TYPE_P(hash, T_HASH));
assert(memo != NULL);
rb_hash_foreach(hash, hash_sum_i, (VALUE)memo);
}
static VALUE
int_range_sum(VALUE beg, VALUE end, int excl, VALUE init)
{
if (excl) {
if (FIXNUM_P(end))
end = LONG2FIX(FIX2LONG(end) - 1);
else
end = rb_big_minus(end, LONG2FIX(1));
}
if (rb_int_ge(end, beg)) {
VALUE a;
a = rb_int_plus(rb_int_minus(end, beg), LONG2FIX(1));
a = rb_int_mul(a, rb_int_plus(end, beg));
a = rb_int_idiv(a, LONG2FIX(2));
return rb_int_plus(init, a);
}
return init;
}
static VALUE
enum_sum(int argc, VALUE* argv, VALUE obj)
{
struct enum_sum_memo memo;
VALUE beg, end;
int excl;
memo.v = (rb_check_arity(argc, 0, 1) == 0) ? LONG2FIX(0) : argv[0];
memo.block_given = rb_block_given_p();
memo.n = 0;
memo.r = Qundef;
if ((memo.float_value = RB_FLOAT_TYPE_P(memo.v))) {
memo.f = RFLOAT_VALUE(memo.v);
memo.c = 0.0;
}
else {
memo.f = 0.0;
memo.c = 0.0;
}
if (RTEST(rb_range_values(obj, &beg, &end, &excl))) {
if (!memo.block_given && !memo.float_value &&
(FIXNUM_P(beg) || RB_TYPE_P(beg, T_BIGNUM)) &&
(FIXNUM_P(end) || RB_TYPE_P(end, T_BIGNUM))) {
return int_range_sum(beg, end, excl, memo.v);
}
}
if (RB_TYPE_P(obj, T_HASH) &&
rb_method_basic_definition_p(CLASS_OF(obj), id_each))
hash_sum(obj, &memo);
else
rb_block_call(obj, id_each, 0, 0, enum_sum_i, (VALUE)&memo);
if (memo.float_value) {
return DBL2NUM(memo.f + memo.c);
}
else {
if (memo.n != 0)
memo.v = rb_fix_plus(LONG2FIX(memo.n), memo.v);
if (memo.r != Qundef) {
if (FIXNUM_P(memo.r))
memo.v = rb_fix_plus(memo.r, memo.v);
else if (RB_TYPE_P(memo.r, T_BIGNUM))
memo.v = rb_big_plus(memo.r, memo.v);
else
memo.v = rb_rational_plus(memo.r, memo.v);
}
return memo.v;
}
}
static VALUE
uniq_func(RB_BLOCK_CALL_FUNC_ARGLIST(i, hash))
{
ENUM_WANT_SVALUE();
rb_hash_add_new_element(hash, i, i);
return Qnil;
}
static VALUE
uniq_iter(RB_BLOCK_CALL_FUNC_ARGLIST(i, hash))
{
ENUM_WANT_SVALUE();
rb_hash_add_new_element(hash, rb_yield_values2(argc, argv), i);
return Qnil;
}
static VALUE
enum_uniq(VALUE obj)
{
VALUE hash, ret;
rb_block_call_func *const func =
rb_block_given_p() ? uniq_iter : uniq_func;
hash = rb_obj_hide(rb_hash_new());
rb_block_call(obj, id_each, 0, 0, func, hash);
ret = rb_hash_values(hash);
rb_hash_clear(hash);
return ret;
}
void
Init_Enumerable(void)
{
#undef rb_intern
#define rb_intern(str) rb_intern_const(str)
rb_mEnumerable = rb_define_module("Enumerable");
rb_define_method(rb_mEnumerable, "to_a", enum_to_a, -1);
rb_define_method(rb_mEnumerable, "entries", enum_to_a, -1);
rb_define_method(rb_mEnumerable, "to_h", enum_to_h, -1);
rb_define_method(rb_mEnumerable, "sort", enum_sort, 0);
rb_define_method(rb_mEnumerable, "sort_by", enum_sort_by, 0);
rb_define_method(rb_mEnumerable, "grep", enum_grep, 1);
rb_define_method(rb_mEnumerable, "grep_v", enum_grep_v, 1);
rb_define_method(rb_mEnumerable, "count", enum_count, -1);
rb_define_method(rb_mEnumerable, "find", enum_find, -1);
rb_define_method(rb_mEnumerable, "detect", enum_find, -1);
rb_define_method(rb_mEnumerable, "find_index", enum_find_index, -1);
rb_define_method(rb_mEnumerable, "find_all", enum_find_all, 0);
rb_define_method(rb_mEnumerable, "select", enum_find_all, 0);
rb_define_method(rb_mEnumerable, "filter", enum_find_all, 0);
rb_define_method(rb_mEnumerable, "filter_map", enum_filter_map, 0);
rb_define_method(rb_mEnumerable, "reject", enum_reject, 0);
rb_define_method(rb_mEnumerable, "collect", enum_collect, 0);
rb_define_method(rb_mEnumerable, "map", enum_collect, 0);
rb_define_method(rb_mEnumerable, "flat_map", enum_flat_map, 0);
rb_define_method(rb_mEnumerable, "collect_concat", enum_flat_map, 0);
rb_define_method(rb_mEnumerable, "inject", enum_inject, -1);
rb_define_method(rb_mEnumerable, "reduce", enum_inject, -1);
rb_define_method(rb_mEnumerable, "partition", enum_partition, 0);
rb_define_method(rb_mEnumerable, "group_by", enum_group_by, 0);
rb_define_method(rb_mEnumerable, "tally", enum_tally, 0);
rb_define_method(rb_mEnumerable, "first", enum_first, -1);
rb_define_method(rb_mEnumerable, "all?", enum_all, -1);
rb_define_method(rb_mEnumerable, "any?", enum_any, -1);
rb_define_method(rb_mEnumerable, "one?", enum_one, -1);
rb_define_method(rb_mEnumerable, "none?", enum_none, -1);
rb_define_method(rb_mEnumerable, "min", enum_min, -1);
rb_define_method(rb_mEnumerable, "max", enum_max, -1);
rb_define_method(rb_mEnumerable, "minmax", enum_minmax, 0);
rb_define_method(rb_mEnumerable, "min_by", enum_min_by, -1);
rb_define_method(rb_mEnumerable, "max_by", enum_max_by, -1);
rb_define_method(rb_mEnumerable, "minmax_by", enum_minmax_by, 0);
rb_define_method(rb_mEnumerable, "member?", enum_member, 1);
rb_define_method(rb_mEnumerable, "include?", enum_member, 1);
rb_define_method(rb_mEnumerable, "each_with_index", enum_each_with_index, -1);
rb_define_method(rb_mEnumerable, "reverse_each", enum_reverse_each, -1);
rb_define_method(rb_mEnumerable, "each_entry", enum_each_entry, -1);
rb_define_method(rb_mEnumerable, "each_slice", enum_each_slice, 1);
rb_define_method(rb_mEnumerable, "each_cons", enum_each_cons, 1);
rb_define_method(rb_mEnumerable, "each_with_object", enum_each_with_object, 1);
rb_define_method(rb_mEnumerable, "zip", enum_zip, -1);
rb_define_method(rb_mEnumerable, "take", enum_take, 1);
rb_define_method(rb_mEnumerable, "take_while", enum_take_while, 0);
rb_define_method(rb_mEnumerable, "drop", enum_drop, 1);
rb_define_method(rb_mEnumerable, "drop_while", enum_drop_while, 0);
rb_define_method(rb_mEnumerable, "cycle", enum_cycle, -1);
rb_define_method(rb_mEnumerable, "chunk", enum_chunk, 0);
rb_define_method(rb_mEnumerable, "slice_before", enum_slice_before, -1);
rb_define_method(rb_mEnumerable, "slice_after", enum_slice_after, -1);
rb_define_method(rb_mEnumerable, "slice_when", enum_slice_when, 0);
rb_define_method(rb_mEnumerable, "chunk_while", enum_chunk_while, 0);
rb_define_method(rb_mEnumerable, "sum", enum_sum, -1);
rb_define_method(rb_mEnumerable, "uniq", enum_uniq, 0);
id_next = rb_intern("next");
}
