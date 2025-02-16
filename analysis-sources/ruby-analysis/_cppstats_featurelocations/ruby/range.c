










#include "ruby/config.h"

#include <assert.h>
#include <math.h>

#if defined(HAVE_FLOAT_H)
#include <float.h>
#endif

#include "id.h"
#include "internal.h"
#include "internal/array.h"
#include "internal/compar.h"
#include "internal/enum.h"
#include "internal/enumerator.h"
#include "internal/error.h"
#include "internal/numeric.h"
#include "internal/range.h"

VALUE rb_cRange;
static ID id_beg, id_end, id_excl;
#define id_cmp idCmp
#define id_succ idSucc

static VALUE r_cover_p(VALUE, VALUE, VALUE, VALUE);

#define RANGE_SET_BEG(r, v) (RSTRUCT_SET(r, 0, v))
#define RANGE_SET_END(r, v) (RSTRUCT_SET(r, 1, v))
#define RANGE_SET_EXCL(r, v) (RSTRUCT_SET(r, 2, v))
#define RBOOL(v) ((v) ? Qtrue : Qfalse)

#define EXCL(r) RTEST(RANGE_EXCL(r))

static void
range_init(VALUE range, VALUE beg, VALUE end, VALUE exclude_end)
{
if ((!FIXNUM_P(beg) || !FIXNUM_P(end)) && !NIL_P(beg) && !NIL_P(end)) {
VALUE v;

v = rb_funcall(beg, id_cmp, 1, end);
if (NIL_P(v))
rb_raise(rb_eArgError, "bad value for range");
}

RANGE_SET_EXCL(range, exclude_end);
RANGE_SET_BEG(range, beg);
RANGE_SET_END(range, end);
}

VALUE
rb_range_new(VALUE beg, VALUE end, int exclude_end)
{
VALUE range = rb_obj_alloc(rb_cRange);

range_init(range, beg, end, RBOOL(exclude_end));
return range;
}

static void
range_modify(VALUE range)
{
rb_check_frozen(range);

if (RANGE_EXCL(range) != Qnil) {
rb_name_err_raise("`initialize' called twice", range, ID2SYM(idInitialize));
}
}










static VALUE
range_initialize(int argc, VALUE *argv, VALUE range)
{
VALUE beg, end, flags;

rb_scan_args(argc, argv, "21", &beg, &end, &flags);
range_modify(range);
range_init(range, beg, end, RBOOL(RTEST(flags)));
return Qnil;
}


static VALUE
range_initialize_copy(VALUE range, VALUE orig)
{
range_modify(range);
rb_struct_init_copy(range, orig);
return range;
}











static VALUE
range_exclude_end_p(VALUE range)
{
return EXCL(range) ? Qtrue : Qfalse;
}

static VALUE
recursive_equal(VALUE range, VALUE obj, int recur)
{
if (recur) return Qtrue; 
if (!rb_equal(RANGE_BEG(range), RANGE_BEG(obj)))
return Qfalse;
if (!rb_equal(RANGE_END(range), RANGE_END(obj)))
return Qfalse;

if (EXCL(range) != EXCL(obj))
return Qfalse;
return Qtrue;
}
















static VALUE
range_eq(VALUE range, VALUE obj)
{
if (range == obj)
return Qtrue;
if (!rb_obj_is_kind_of(obj, rb_cRange))
return Qfalse;

return rb_exec_recursive_paired(recursive_equal, range, obj, obj);
}






static int
r_less(VALUE a, VALUE b)
{
VALUE r = rb_funcall(a, id_cmp, 1, b);

if (NIL_P(r))
return INT_MAX;
return rb_cmpint(r, a, b);
}

static VALUE
recursive_eql(VALUE range, VALUE obj, int recur)
{
if (recur) return Qtrue; 
if (!rb_eql(RANGE_BEG(range), RANGE_BEG(obj)))
return Qfalse;
if (!rb_eql(RANGE_END(range), RANGE_END(obj)))
return Qfalse;

if (EXCL(range) != EXCL(obj))
return Qfalse;
return Qtrue;
}















static VALUE
range_eql(VALUE range, VALUE obj)
{
if (range == obj)
return Qtrue;
if (!rb_obj_is_kind_of(obj, rb_cRange))
return Qfalse;
return rb_exec_recursive_paired(recursive_eql, range, obj, obj);
}












static VALUE
range_hash(VALUE range)
{
st_index_t hash = EXCL(range);
VALUE v;

hash = rb_hash_start(hash);
v = rb_hash(RANGE_BEG(range));
hash = rb_hash_uint(hash, NUM2LONG(v));
v = rb_hash(RANGE_END(range));
hash = rb_hash_uint(hash, NUM2LONG(v));
hash = rb_hash_uint(hash, EXCL(range) << 24);
hash = rb_hash_end(hash);

return ST2FIX(hash);
}

static void
range_each_func(VALUE range, int (*func)(VALUE, VALUE), VALUE arg)
{
int c;
VALUE b = RANGE_BEG(range);
VALUE e = RANGE_END(range);
VALUE v = b;

if (EXCL(range)) {
while (r_less(v, e) < 0) {
if ((*func)(v, arg)) break;
v = rb_funcallv(v, id_succ, 0, 0);
}
}
else {
while ((c = r_less(v, e)) <= 0) {
if ((*func)(v, arg)) break;
if (!c) break;
v = rb_funcallv(v, id_succ, 0, 0);
}
}
}

static int
sym_step_i(VALUE i, VALUE arg)
{
VALUE *iter = (VALUE *)arg;

if (FIXNUM_P(iter[0])) {
iter[0] -= INT2FIX(1) & ~FIXNUM_FLAG;
}
else {
iter[0] = rb_funcall(iter[0], '-', 1, INT2FIX(1));
}
if (iter[0] == INT2FIX(0)) {
rb_yield(rb_str_intern(i));
iter[0] = iter[1];
}
return 0;
}

static int
step_i(VALUE i, VALUE arg)
{
VALUE *iter = (VALUE *)arg;

if (FIXNUM_P(iter[0])) {
iter[0] -= INT2FIX(1) & ~FIXNUM_FLAG;
}
else {
iter[0] = rb_funcall(iter[0], '-', 1, INT2FIX(1));
}
if (iter[0] == INT2FIX(0)) {
rb_yield(i);
iter[0] = iter[1];
}
return 0;
}

static int
discrete_object_p(VALUE obj)
{
if (rb_obj_is_kind_of(obj, rb_cTime)) return FALSE; 
return rb_respond_to(obj, id_succ);
}

static int
linear_object_p(VALUE obj)
{
if (FIXNUM_P(obj) || FLONUM_P(obj)) return TRUE;
if (SPECIAL_CONST_P(obj)) return FALSE;
switch (BUILTIN_TYPE(obj)) {
case T_FLOAT:
case T_BIGNUM:
return TRUE;
}
if (rb_obj_is_kind_of(obj, rb_cNumeric)) return TRUE;
if (rb_obj_is_kind_of(obj, rb_cTime)) return TRUE;
return FALSE;
}

static VALUE
check_step_domain(VALUE step)
{
VALUE zero = INT2FIX(0);
int cmp;
if (!rb_obj_is_kind_of(step, rb_cNumeric)) {
step = rb_to_int(step);
}
cmp = rb_cmpint(rb_funcallv(step, idCmp, 1, &zero), step, zero);
if (cmp < 0) {
rb_raise(rb_eArgError, "step can't be negative");
}
else if (cmp == 0) {
rb_raise(rb_eArgError, "step can't be 0");
}
return step;
}

static VALUE
range_step_size(VALUE range, VALUE args, VALUE eobj)
{
VALUE b = RANGE_BEG(range), e = RANGE_END(range);
VALUE step = INT2FIX(1);
if (args) {
step = check_step_domain(RARRAY_AREF(args, 0));
}

if (rb_obj_is_kind_of(b, rb_cNumeric) && rb_obj_is_kind_of(e, rb_cNumeric)) {
return ruby_num_interval_step_size(b, e, step, EXCL(range));
}
return Qnil;
}









































static VALUE
range_step(int argc, VALUE *argv, VALUE range)
{
VALUE b, e, step, tmp;

b = RANGE_BEG(range);
e = RANGE_END(range);
step = (!rb_check_arity(argc, 0, 1) ? INT2FIX(1) : argv[0]);

if (!rb_block_given_p()) {
const VALUE b_num_p = rb_obj_is_kind_of(b, rb_cNumeric);
const VALUE e_num_p = rb_obj_is_kind_of(e, rb_cNumeric);
if ((b_num_p && (NIL_P(e) || e_num_p)) || (NIL_P(b) && e_num_p)) {
return rb_arith_seq_new(range, ID2SYM(rb_frame_this_func()), argc, argv,
range_step_size, b, e, step, EXCL(range));
}

RETURN_SIZED_ENUMERATOR(range, argc, argv, range_step_size);
}

step = check_step_domain(step);

if (FIXNUM_P(b) && NIL_P(e) && FIXNUM_P(step)) {
long i = FIX2LONG(b), unit = FIX2LONG(step);
do {
rb_yield(LONG2FIX(i));
i += unit; 
} while (FIXABLE(i));
b = LONG2NUM(i);

for (;; b = rb_big_plus(b, step))
rb_yield(b);
}
else if (FIXNUM_P(b) && FIXNUM_P(e) && FIXNUM_P(step)) { 
long end = FIX2LONG(e);
long i, unit = FIX2LONG(step);

if (!EXCL(range))
end += 1;
i = FIX2LONG(b);
while (i < end) {
rb_yield(LONG2NUM(i));
if (i + unit < i) break;
i += unit;
}

}
else if (SYMBOL_P(b) && (NIL_P(e) || SYMBOL_P(e))) { 
VALUE iter[2];
iter[0] = INT2FIX(1);
iter[1] = step;

b = rb_sym2str(b);
if (NIL_P(e)) {
rb_str_upto_endless_each(b, sym_step_i, (VALUE)iter);
}
else {
rb_str_upto_each(b, rb_sym2str(e), EXCL(range), sym_step_i, (VALUE)iter);
}
}
else if (ruby_float_step(b, e, step, EXCL(range), TRUE)) {

}
else if (rb_obj_is_kind_of(b, rb_cNumeric) ||
!NIL_P(rb_check_to_integer(b, "to_int")) ||
!NIL_P(rb_check_to_integer(e, "to_int"))) {
ID op = EXCL(range) ? '<' : idLE;
VALUE v = b;
int i = 0;

while (NIL_P(e) || RTEST(rb_funcall(v, op, 1, e))) {
rb_yield(v);
i++;
v = rb_funcall(b, '+', 1, rb_funcall(INT2NUM(i), '*', 1, step));
}
}
else {
tmp = rb_check_string_type(b);

if (!NIL_P(tmp)) {
VALUE iter[2];

b = tmp;
iter[0] = INT2FIX(1);
iter[1] = step;

if (NIL_P(e)) {
rb_str_upto_endless_each(b, step_i, (VALUE)iter);
}
else {
rb_str_upto_each(b, e, EXCL(range), step_i, (VALUE)iter);
}
}
else {
VALUE args[2];

if (!discrete_object_p(b)) {
rb_raise(rb_eTypeError, "can't iterate from %s",
rb_obj_classname(b));
}
args[0] = INT2FIX(1);
args[1] = step;
range_each_func(range, step_i, (VALUE)args);
}
}
return range;
}

static VALUE
range_percent_step(VALUE range, VALUE step)
{
return range_step(1, &step, range);
}

#if SIZEOF_DOUBLE == 8 && defined(HAVE_INT64_T)
union int64_double {
int64_t i;
double d;
};

static VALUE
int64_as_double_to_num(int64_t i)
{
union int64_double convert;
if (i < 0) {
convert.i = -i;
return DBL2NUM(-convert.d);
}
else {
convert.i = i;
return DBL2NUM(convert.d);
}
}

static int64_t
double_as_int64(double d)
{
union int64_double convert;
convert.d = fabs(d);
return d < 0 ? -convert.i : convert.i;
}
#endif

static int
is_integer_p(VALUE v)
{
ID id_integer_p;
VALUE is_int;
CONST_ID(id_integer_p, "integer?");
is_int = rb_check_funcall(v, id_integer_p, 0, 0);
return RTEST(is_int) && is_int != Qundef;
}

static VALUE
bsearch_integer_range(VALUE beg, VALUE end, int excl)
{
VALUE satisfied = Qnil;
int smaller;

#define BSEARCH_CHECK(expr) do { VALUE val = (expr); VALUE v = rb_yield(val); if (FIXNUM_P(v)) { if (v == INT2FIX(0)) return val; smaller = (SIGNED_VALUE)v < 0; } else if (v == Qtrue) { satisfied = val; smaller = 1; } else if (v == Qfalse || v == Qnil) { smaller = 0; } else if (rb_obj_is_kind_of(v, rb_cNumeric)) { int cmp = rb_cmpint(rb_funcall(v, id_cmp, 1, INT2FIX(0)), v, INT2FIX(0)); if (!cmp) return val; smaller = cmp < 0; } else { rb_raise(rb_eTypeError, "wrong argument type %"PRIsVALUE " (must be numeric, true, false or nil)", rb_obj_class(v)); } } while (0)


























VALUE low = rb_to_int(beg);
VALUE high = rb_to_int(end);
VALUE mid, org_high;
ID id_div;
CONST_ID(id_div, "div");

if (excl) high = rb_funcall(high, '-', 1, INT2FIX(1));
org_high = high;

while (rb_cmpint(rb_funcall(low, id_cmp, 1, high), low, high) < 0) {
mid = rb_funcall(rb_funcall(high, '+', 1, low), id_div, 1, INT2FIX(2));
BSEARCH_CHECK(mid);
if (smaller) {
high = mid;
}
else {
low = rb_funcall(mid, '+', 1, INT2FIX(1));
}
}
if (rb_equal(low, org_high)) {
BSEARCH_CHECK(low);
if (!smaller) return Qnil;
}
return satisfied;
}





















































static VALUE
range_bsearch(VALUE range)
{
VALUE beg, end, satisfied = Qnil;
int smaller;















#define BSEARCH(conv) do { RETURN_ENUMERATOR(range, 0, 0); if (EXCL(range)) high--; org_high = high; while (low < high) { mid = ((high < 0) == (low < 0)) ? low + ((high - low) / 2) : (low < -high) ? -((-1 - low - high)/2 + 1) : (low + high) / 2; BSEARCH_CHECK(conv(mid)); if (smaller) { high = mid; } else { low = mid + 1; } } if (low == org_high) { BSEARCH_CHECK(conv(low)); if (!smaller) return Qnil; } return satisfied; } while (0)























beg = RANGE_BEG(range);
end = RANGE_END(range);

if (FIXNUM_P(beg) && FIXNUM_P(end)) {
long low = FIX2LONG(beg);
long high = FIX2LONG(end);
long mid, org_high;
BSEARCH(INT2FIX);
}
#if SIZEOF_DOUBLE == 8 && defined(HAVE_INT64_T)
else if (RB_TYPE_P(beg, T_FLOAT) || RB_TYPE_P(end, T_FLOAT)) {
int64_t low = double_as_int64(NIL_P(beg) ? -HUGE_VAL : RFLOAT_VALUE(rb_Float(beg)));
int64_t high = double_as_int64(NIL_P(end) ? HUGE_VAL : RFLOAT_VALUE(rb_Float(end)));
int64_t mid, org_high;
BSEARCH(int64_as_double_to_num);
}
#endif
else if (is_integer_p(beg) && is_integer_p(end)) {
RETURN_ENUMERATOR(range, 0, 0);
return bsearch_integer_range(beg, end, EXCL(range));
}
else if (is_integer_p(beg) && NIL_P(end)) {
VALUE diff = LONG2FIX(1);
RETURN_ENUMERATOR(range, 0, 0);
while (1) {
VALUE mid = rb_funcall(beg, '+', 1, diff);
BSEARCH_CHECK(mid);
if (smaller) {
return bsearch_integer_range(beg, mid, 0);
}
diff = rb_funcall(diff, '*', 1, LONG2FIX(2));
}
}
else if (NIL_P(beg) && is_integer_p(end)) {
VALUE diff = LONG2FIX(-1);
RETURN_ENUMERATOR(range, 0, 0);
while (1) {
VALUE mid = rb_funcall(end, '+', 1, diff);
BSEARCH_CHECK(mid);
if (!smaller) {
return bsearch_integer_range(mid, end, 0);
}
diff = rb_funcall(diff, '*', 1, LONG2FIX(2));
}
}
else {
rb_raise(rb_eTypeError, "can't do binary search for %s", rb_obj_classname(beg));
}
return range;
}

static int
each_i(VALUE v, VALUE arg)
{
rb_yield(v);
return 0;
}

static int
sym_each_i(VALUE v, VALUE arg)
{
rb_yield(rb_str_intern(v));
return 0;
}













static VALUE
range_size(VALUE range)
{
VALUE b = RANGE_BEG(range), e = RANGE_END(range);
if (rb_obj_is_kind_of(b, rb_cNumeric)) {
if (rb_obj_is_kind_of(e, rb_cNumeric)) {
return ruby_num_interval_step_size(b, e, INT2FIX(1), EXCL(range));
}
if (NIL_P(e)) {
return DBL2NUM(HUGE_VAL);
}
}
else if (NIL_P(b)) {
return DBL2NUM(HUGE_VAL);
}

return Qnil;
}












static VALUE
range_to_a(VALUE range)
{
if (NIL_P(RANGE_END(range))) {
rb_raise(rb_eRangeError, "cannot convert endless range to an array");
}
return rb_call_super(0, 0);
}

static VALUE
range_enum_size(VALUE range, VALUE args, VALUE eobj)
{
return range_size(range);
}






















static VALUE
range_each(VALUE range)
{
VALUE beg, end;
long i, lim;

RETURN_SIZED_ENUMERATOR(range, 0, 0, range_enum_size);

beg = RANGE_BEG(range);
end = RANGE_END(range);

if (FIXNUM_P(beg) && NIL_P(end)) {
fixnum_endless:
i = FIX2LONG(beg);
while (FIXABLE(i)) {
rb_yield(LONG2FIX(i++));
}
beg = LONG2NUM(i);
bignum_endless:
for (;; beg = rb_big_plus(beg, INT2FIX(1)))
rb_yield(beg);
}
else if (FIXNUM_P(beg) && FIXNUM_P(end)) { 
fixnum_loop:
lim = FIX2LONG(end);
if (!EXCL(range))
lim += 1;
for (i = FIX2LONG(beg); i < lim; i++) {
rb_yield(LONG2FIX(i));
}
}
else if (RB_INTEGER_TYPE_P(beg) && (NIL_P(end) || RB_INTEGER_TYPE_P(end))) {
if (SPECIAL_CONST_P(end) || RBIGNUM_POSITIVE_P(end)) { 
if (!FIXNUM_P(beg)) {
if (RBIGNUM_NEGATIVE_P(beg)) {
do {
rb_yield(beg);
} while (!FIXNUM_P(beg = rb_big_plus(beg, INT2FIX(1))));
if (NIL_P(end)) goto fixnum_endless;
if (FIXNUM_P(end)) goto fixnum_loop;
}
else {
if (NIL_P(end)) goto bignum_endless;
if (FIXNUM_P(end)) return range;
}
}
if (FIXNUM_P(beg)) {
i = FIX2LONG(beg);
do {
rb_yield(LONG2FIX(i));
} while (POSFIXABLE(++i));
beg = LONG2NUM(i);
}
ASSUME(!FIXNUM_P(beg));
ASSUME(!SPECIAL_CONST_P(end));
}
if (!FIXNUM_P(beg) && RBIGNUM_SIGN(beg) == RBIGNUM_SIGN(end)) {
if (EXCL(range)) {
while (rb_big_cmp(beg, end) == INT2FIX(-1)) {
rb_yield(beg);
beg = rb_big_plus(beg, INT2FIX(1));
}
}
else {
VALUE c;
while ((c = rb_big_cmp(beg, end)) != INT2FIX(1)) {
rb_yield(beg);
if (c == INT2FIX(0)) break;
beg = rb_big_plus(beg, INT2FIX(1));
}
}
}
}
else if (SYMBOL_P(beg) && (NIL_P(end) || SYMBOL_P(end))) { 
beg = rb_sym2str(beg);
if (NIL_P(end)) {
rb_str_upto_endless_each(beg, sym_each_i, 0);
}
else {
rb_str_upto_each(beg, rb_sym2str(end), EXCL(range), sym_each_i, 0);
}
}
else {
VALUE tmp = rb_check_string_type(beg);

if (!NIL_P(tmp)) {
if (!NIL_P(end)) {
rb_str_upto_each(tmp, end, EXCL(range), each_i, 0);
}
else {
rb_str_upto_endless_each(tmp, each_i, 0);
}
}
else {
if (!discrete_object_p(beg)) {
rb_raise(rb_eTypeError, "can't iterate from %s",
rb_obj_classname(beg));
}
if (!NIL_P(end))
range_each_func(range, each_i, 0);
else
for (;; beg = rb_funcallv(beg, id_succ, 0, 0))
rb_yield(beg);
}
}
return range;
}










static VALUE
range_begin(VALUE range)
{
return RANGE_BEG(range);
}













static VALUE
range_end(VALUE range)
{
return RANGE_END(range);
}


static VALUE
first_i(RB_BLOCK_CALL_FUNC_ARGLIST(i, cbarg))
{
VALUE *ary = (VALUE *)cbarg;
long n = NUM2LONG(ary[0]);

if (n <= 0) {
rb_iter_break();
}
rb_ary_push(ary[1], i);
n--;
ary[0] = INT2NUM(n);
return Qnil;
}













static VALUE
range_first(int argc, VALUE *argv, VALUE range)
{
VALUE n, ary[2];

if (NIL_P(RANGE_BEG(range))) {
rb_raise(rb_eRangeError, "cannot get the first element of beginless range");
}
if (argc == 0) return RANGE_BEG(range);

rb_scan_args(argc, argv, "1", &n);
ary[0] = n;
ary[1] = rb_ary_new2(NUM2LONG(n));
rb_block_call(range, idEach, 0, 0, first_i, (VALUE)ary);

return ary[1];
}

static VALUE
rb_int_range_last(int argc, VALUE *argv, VALUE range)
{
static const VALUE ONE = INT2FIX(1);

VALUE b, e, len_1, len, nv, ary;
int x;
long n;

assert(argc > 0);

b = RANGE_BEG(range);
e = RANGE_END(range);
assert(RB_INTEGER_TYPE_P(b) && RB_INTEGER_TYPE_P(e));

x = EXCL(range);

len_1 = rb_int_minus(e, b);
if (FIXNUM_ZERO_P(len_1) || rb_num_negative_p(len_1)) {
return rb_ary_new_capa(0);
}

if (x) {
e = rb_int_minus(e, ONE);
len = len_1;
}
else {
len = rb_int_plus(len_1, ONE);
}

rb_scan_args(argc, argv, "1", &nv);
n = NUM2LONG(nv);
if (n < 0) {
rb_raise(rb_eArgError, "negative array size");
}

nv = LONG2NUM(n);
if (RTEST(rb_int_gt(nv, len))) {
nv = len;
n = NUM2LONG(nv);
}

ary = rb_ary_new_capa(n);
b = rb_int_minus(e, nv);
while (n) {
b = rb_int_plus(b, ONE);
rb_ary_push(ary, b);
--n;
}

return ary;
}


















static VALUE
range_last(int argc, VALUE *argv, VALUE range)
{
VALUE b, e;

if (NIL_P(RANGE_END(range))) {
rb_raise(rb_eRangeError, "cannot get the last element of endless range");
}
if (argc == 0) return RANGE_END(range);

b = RANGE_BEG(range);
e = RANGE_END(range);
if (RB_INTEGER_TYPE_P(b) && RB_INTEGER_TYPE_P(e) &&
RB_LIKELY(rb_method_basic_definition_p(rb_cRange, idEach))) {
return rb_int_range_last(argc, argv, range);
}
return rb_ary_last(argc, argv, rb_Array(range));
}




















static VALUE
range_min(int argc, VALUE *argv, VALUE range)
{
if (NIL_P(RANGE_BEG(range))) {
rb_raise(rb_eRangeError, "cannot get the minimum of beginless range");
}

if (rb_block_given_p()) {
if (NIL_P(RANGE_END(range))) {
rb_raise(rb_eRangeError, "cannot get the minimum of endless range with custom comparison method");
}
return rb_call_super(argc, argv);
}
else if (argc != 0) {
return range_first(argc, argv, range);
}
else {
struct cmp_opt_data cmp_opt = { 0, 0 };
VALUE b = RANGE_BEG(range);
VALUE e = RANGE_END(range);
int c = NIL_P(e) ? -1 : OPTIMIZED_CMP(b, e, cmp_opt);

if (c > 0 || (c == 0 && EXCL(range)))
return Qnil;
return b;
}
}


















static VALUE
range_max(int argc, VALUE *argv, VALUE range)
{
VALUE e = RANGE_END(range);
int nm = FIXNUM_P(e) || rb_obj_is_kind_of(e, rb_cNumeric);

if (NIL_P(RANGE_END(range))) {
rb_raise(rb_eRangeError, "cannot get the maximum of endless range");
}

if (rb_block_given_p() || (EXCL(range) && !nm) || argc) {
if (NIL_P(RANGE_BEG(range))) {
rb_raise(rb_eRangeError, "cannot get the maximum of beginless range with custom comparison method");
}
return rb_call_super(argc, argv);
}
else {
struct cmp_opt_data cmp_opt = { 0, 0 };
VALUE b = RANGE_BEG(range);
int c = OPTIMIZED_CMP(b, e, cmp_opt);

if (c > 0)
return Qnil;
if (EXCL(range)) {
if (!RB_INTEGER_TYPE_P(e)) {
rb_raise(rb_eTypeError, "cannot exclude non Integer end value");
}
if (c == 0) return Qnil;
if (!RB_INTEGER_TYPE_P(b)) {
rb_raise(rb_eTypeError, "cannot exclude end value with non Integer begin value");
}
if (FIXNUM_P(e)) {
return LONG2NUM(FIX2LONG(e) - 1);
}
return rb_funcall(e, '-', 1, INT2FIX(1));
}
return e;
}
}













static VALUE
range_minmax(VALUE range)
{
if (rb_block_given_p()) {
return rb_call_super(0, NULL);
}
return rb_assoc_new(range_min(0, NULL, range), range_max(0, NULL, range));
}

int
rb_range_values(VALUE range, VALUE *begp, VALUE *endp, int *exclp)
{
VALUE b, e;
int excl;

if (rb_obj_is_kind_of(range, rb_cRange)) {
b = RANGE_BEG(range);
e = RANGE_END(range);
excl = EXCL(range);
}
else if (RTEST(rb_obj_is_kind_of(range, rb_cArithSeq))) {
return (int)Qfalse;
}
else {
VALUE x;
b = rb_check_funcall(range, id_beg, 0, 0);
if (b == Qundef) return (int)Qfalse;
e = rb_check_funcall(range, id_end, 0, 0);
if (e == Qundef) return (int)Qfalse;
x = rb_check_funcall(range, rb_intern("exclude_end?"), 0, 0);
if (x == Qundef) return (int)Qfalse;
excl = RTEST(x);
}
*begp = b;
*endp = e;
*exclp = excl;
return (int)Qtrue;
}

VALUE
rb_range_beg_len(VALUE range, long *begp, long *lenp, long len, int err)
{
long beg, end, origbeg, origend;
VALUE b, e;
int excl;

if (!rb_range_values(range, &b, &e, &excl))
return Qfalse;
beg = NIL_P(b) ? 0 : NUM2LONG(b);
end = NIL_P(e) ? -1 : NUM2LONG(e);
if (NIL_P(e)) excl = 0;
origbeg = beg;
origend = end;
if (beg < 0) {
beg += len;
if (beg < 0)
goto out_of_range;
}
if (end < 0)
end += len;
if (!excl)
end++; 
if (err == 0 || err == 2) {
if (beg > len)
goto out_of_range;
if (end > len)
end = len;
}
len = end - beg;
if (len < 0)
len = 0;

*begp = beg;
*lenp = len;
return Qtrue;

out_of_range:
if (err) {
rb_raise(rb_eRangeError, "%ld..%s%ld out of range",
origbeg, excl ? "." : "", origend);
}
return Qnil;
}









static VALUE
range_to_s(VALUE range)
{
VALUE str, str2;

str = rb_obj_as_string(RANGE_BEG(range));
str2 = rb_obj_as_string(RANGE_END(range));
str = rb_str_dup(str);
rb_str_cat(str, "...", EXCL(range) ? 3 : 2);
rb_str_append(str, str2);

return str;
}

static VALUE
inspect_range(VALUE range, VALUE dummy, int recur)
{
VALUE str, str2 = Qundef;

if (recur) {
return rb_str_new2(EXCL(range) ? "(... ... ...)" : "(... .. ...)");
}
if (!NIL_P(RANGE_BEG(range)) || NIL_P(RANGE_END(range))) {
str = rb_str_dup(rb_inspect(RANGE_BEG(range)));
}
else {
str = rb_str_new(0, 0);
}
rb_str_cat(str, "...", EXCL(range) ? 3 : 2);
if (NIL_P(RANGE_BEG(range)) || !NIL_P(RANGE_END(range))) {
str2 = rb_inspect(RANGE_END(range));
}
if (str2 != Qundef) rb_str_append(str, str2);

return str;
}










static VALUE
range_inspect(VALUE range)
{
return rb_exec_recursive(inspect_range, range, 0);
}

static VALUE range_include_internal(VALUE range, VALUE val, int string_use_cover);



























static VALUE
range_eqq(VALUE range, VALUE val)
{
VALUE ret = range_include_internal(range, val, 1);
if (ret != Qundef) return ret;
return r_cover_p(range, RANGE_BEG(range), RANGE_END(range), val);
}























static VALUE
range_include(VALUE range, VALUE val)
{
VALUE ret = range_include_internal(range, val, 0);
if (ret != Qundef) return ret;
return rb_call_super(1, &val);
}

static VALUE
range_include_internal(VALUE range, VALUE val, int string_use_cover)
{
VALUE beg = RANGE_BEG(range);
VALUE end = RANGE_END(range);
int nv = FIXNUM_P(beg) || FIXNUM_P(end) ||
linear_object_p(beg) || linear_object_p(end);

if (nv ||
!NIL_P(rb_check_to_integer(beg, "to_int")) ||
!NIL_P(rb_check_to_integer(end, "to_int"))) {
return r_cover_p(range, beg, end, val);
}
else if (RB_TYPE_P(beg, T_STRING) || RB_TYPE_P(end, T_STRING)) {
if (RB_TYPE_P(beg, T_STRING) && RB_TYPE_P(end, T_STRING)) {
if (string_use_cover) {
return r_cover_p(range, beg, end, val);
}
else {
VALUE rb_str_include_range_p(VALUE beg, VALUE end, VALUE val, VALUE exclusive);
return rb_str_include_range_p(beg, end, val, RANGE_EXCL(range));
}
}
else if (NIL_P(beg)) {
VALUE r = rb_funcall(val, id_cmp, 1, end);
if (NIL_P(r)) return Qfalse;
if (rb_cmpint(r, val, end) <= 0) return Qtrue;
return Qfalse;
}
else if (NIL_P(end)) {
VALUE r = rb_funcall(beg, id_cmp, 1, val);
if (NIL_P(r)) return Qfalse;
if (rb_cmpint(r, beg, val) <= 0) return Qtrue;
return Qfalse;
}
}
return Qundef;
}

static int r_cover_range_p(VALUE range, VALUE beg, VALUE end, VALUE val);

































static VALUE
range_cover(VALUE range, VALUE val)
{
VALUE beg, end;

beg = RANGE_BEG(range);
end = RANGE_END(range);

if (rb_obj_is_kind_of(val, rb_cRange)) {
return RBOOL(r_cover_range_p(range, beg, end, val));
}
return r_cover_p(range, beg, end, val);
}

static VALUE
r_call_max(VALUE r)
{
return rb_funcallv(r, rb_intern("max"), 0, 0);
}

static int
r_cover_range_p(VALUE range, VALUE beg, VALUE end, VALUE val)
{
VALUE val_beg, val_end, val_max;
int cmp_end;

val_beg = RANGE_BEG(val);
val_end = RANGE_END(val);

if (!NIL_P(end) && NIL_P(val_end)) return FALSE;
if (!NIL_P(beg) && NIL_P(val_beg)) return FALSE;
if (!NIL_P(val_beg) && !NIL_P(val_end) && r_less(val_beg, val_end) > -EXCL(val)) return FALSE;
if (!NIL_P(val_beg) && !r_cover_p(range, beg, end, val_beg)) return FALSE;

cmp_end = r_less(end, val_end);

if (EXCL(range) == EXCL(val)) {
return cmp_end >= 0;
}
else if (EXCL(range)) {
return cmp_end > 0;
}
else if (cmp_end >= 0) {
return TRUE;
}

val_max = rb_rescue2(r_call_max, val, 0, Qnil, rb_eTypeError, (VALUE)0);
if (val_max == Qnil) return FALSE;

return r_less(end, val_max) >= 0;
}

static VALUE
r_cover_p(VALUE range, VALUE beg, VALUE end, VALUE val)
{
if (NIL_P(beg) || r_less(beg, val) <= 0) {
int excl = EXCL(range);
if (NIL_P(end) || r_less(val, end) <= -excl)
return Qtrue;
}
return Qfalse;
}

static VALUE
range_dumper(VALUE range)
{
VALUE v;
NEWOBJ_OF(m, struct RObject, rb_cObject, T_OBJECT | (RGENGC_WB_PROTECTED_OBJECT ? FL_WB_PROTECTED : 1));

v = (VALUE)m;

rb_ivar_set(v, id_excl, RANGE_EXCL(range));
rb_ivar_set(v, id_beg, RANGE_BEG(range));
rb_ivar_set(v, id_end, RANGE_END(range));
return v;
}

static VALUE
range_loader(VALUE range, VALUE obj)
{
VALUE beg, end, excl;

if (!RB_TYPE_P(obj, T_OBJECT) || RBASIC(obj)->klass != rb_cObject) {
rb_raise(rb_eTypeError, "not a dumped range object");
}

range_modify(range);
beg = rb_ivar_get(obj, id_beg);
end = rb_ivar_get(obj, id_end);
excl = rb_ivar_get(obj, id_excl);
if (!NIL_P(excl)) {
range_init(range, beg, end, RBOOL(RTEST(excl)));
}
return range;
}

static VALUE
range_alloc(VALUE klass)
{


return rb_struct_alloc_noinit(klass);
}











static VALUE
range_count(int argc, VALUE *argv, VALUE range)
{
if (argc != 0) {


return rb_call_super(argc, argv);
}
else if (rb_block_given_p()) {


return rb_call_super(argc, argv);
}
else if (NIL_P(RANGE_END(range))) {

return DBL2NUM(HUGE_VAL);
}
else if (NIL_P(RANGE_BEG(range))) {

return DBL2NUM(HUGE_VAL);
}
else {
return rb_call_super(argc, argv);
}
}






























































































void
Init_Range(void)
{
#undef rb_intern
#define rb_intern(str) rb_intern_const(str)

id_beg = rb_intern("begin");
id_end = rb_intern("end");
id_excl = rb_intern("excl");

rb_cRange = rb_struct_define_without_accessor(
"Range", rb_cObject, range_alloc,
"begin", "end", "excl", NULL);

rb_include_module(rb_cRange, rb_mEnumerable);
rb_marshal_define_compat(rb_cRange, rb_cObject, range_dumper, range_loader);
rb_define_method(rb_cRange, "initialize", range_initialize, -1);
rb_define_method(rb_cRange, "initialize_copy", range_initialize_copy, 1);
rb_define_method(rb_cRange, "==", range_eq, 1);
rb_define_method(rb_cRange, "===", range_eqq, 1);
rb_define_method(rb_cRange, "eql?", range_eql, 1);
rb_define_method(rb_cRange, "hash", range_hash, 0);
rb_define_method(rb_cRange, "each", range_each, 0);
rb_define_method(rb_cRange, "step", range_step, -1);
rb_define_method(rb_cRange, "%", range_percent_step, 1);
rb_define_method(rb_cRange, "bsearch", range_bsearch, 0);
rb_define_method(rb_cRange, "begin", range_begin, 0);
rb_define_method(rb_cRange, "end", range_end, 0);
rb_define_method(rb_cRange, "first", range_first, -1);
rb_define_method(rb_cRange, "last", range_last, -1);
rb_define_method(rb_cRange, "min", range_min, -1);
rb_define_method(rb_cRange, "max", range_max, -1);
rb_define_method(rb_cRange, "minmax", range_minmax, 0);
rb_define_method(rb_cRange, "size", range_size, 0);
rb_define_method(rb_cRange, "to_a", range_to_a, 0);
rb_define_method(rb_cRange, "entries", range_to_a, 0);
rb_define_method(rb_cRange, "to_s", range_to_s, 0);
rb_define_method(rb_cRange, "inspect", range_inspect, 0);

rb_define_method(rb_cRange, "exclude_end?", range_exclude_end_p, 0);

rb_define_method(rb_cRange, "member?", range_include, 1);
rb_define_method(rb_cRange, "include?", range_include, 1);
rb_define_method(rb_cRange, "cover?", range_cover, 1);
rb_define_method(rb_cRange, "count", range_count, -1);
}
