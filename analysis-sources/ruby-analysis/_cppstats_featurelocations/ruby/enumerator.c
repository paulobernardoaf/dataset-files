













#include "ruby/config.h"

#if defined(HAVE_FLOAT_H)
#include <float.h>
#endif

#include "id.h"
#include "internal.h"
#include "internal/enumerator.h"
#include "internal/error.h"
#include "internal/hash.h"
#include "internal/imemo.h"
#include "internal/numeric.h"
#include "internal/range.h"
#include "ruby/ruby.h"























































































VALUE rb_cEnumerator;
static VALUE rb_cLazy;
static ID id_rewind, id_new, id_to_enum;
static ID id_next, id_result, id_receiver, id_arguments, id_memo, id_method, id_force;
static ID id_begin, id_end, id_step, id_exclude_end;
static VALUE sym_each, sym_cycle, sym_yield;

static VALUE lazy_use_super_method;

#define id_call idCall
#define id_each idEach
#define id_eqq idEqq
#define id_initialize idInitialize
#define id_size idSize

VALUE rb_eStopIteration;

struct enumerator {
VALUE obj;
ID meth;
VALUE args;
VALUE fib;
VALUE dst;
VALUE lookahead;
VALUE feedvalue;
VALUE stop_exc;
VALUE size;
VALUE procs;
rb_enumerator_size_func *size_fn;
int kw_splat;
};

static VALUE rb_cGenerator, rb_cYielder, rb_cEnumProducer;

struct generator {
VALUE proc;
VALUE obj;
};

struct yielder {
VALUE proc;
};

struct producer {
VALUE init;
VALUE proc;
};

typedef struct MEMO *lazyenum_proc_func(VALUE, struct MEMO *, VALUE, long);
typedef VALUE lazyenum_size_func(VALUE, VALUE);
typedef struct {
lazyenum_proc_func *proc;
lazyenum_size_func *size;
} lazyenum_funcs;

struct proc_entry {
VALUE proc;
VALUE memo;
const lazyenum_funcs *fn;
};

static VALUE generator_allocate(VALUE klass);
static VALUE generator_init(VALUE obj, VALUE proc);

static VALUE rb_cEnumChain;

struct enum_chain {
VALUE enums;
long pos;
};

VALUE rb_cArithSeq;




static void
enumerator_mark(void *p)
{
struct enumerator *ptr = p;
rb_gc_mark_movable(ptr->obj);
rb_gc_mark_movable(ptr->args);
rb_gc_mark_movable(ptr->fib);
rb_gc_mark_movable(ptr->dst);
rb_gc_mark_movable(ptr->lookahead);
rb_gc_mark_movable(ptr->feedvalue);
rb_gc_mark_movable(ptr->stop_exc);
rb_gc_mark_movable(ptr->size);
rb_gc_mark_movable(ptr->procs);
}

static void
enumerator_compact(void *p)
{
struct enumerator *ptr = p;
ptr->obj = rb_gc_location(ptr->obj);
ptr->args = rb_gc_location(ptr->args);
ptr->fib = rb_gc_location(ptr->fib);
ptr->dst = rb_gc_location(ptr->dst);
ptr->lookahead = rb_gc_location(ptr->lookahead);
ptr->feedvalue = rb_gc_location(ptr->feedvalue);
ptr->stop_exc = rb_gc_location(ptr->stop_exc);
ptr->size = rb_gc_location(ptr->size);
ptr->procs = rb_gc_location(ptr->procs);
}

#define enumerator_free RUBY_TYPED_DEFAULT_FREE

static size_t
enumerator_memsize(const void *p)
{
return sizeof(struct enumerator);
}

static const rb_data_type_t enumerator_data_type = {
"enumerator",
{
enumerator_mark,
enumerator_free,
enumerator_memsize,
enumerator_compact,
},
0, 0, RUBY_TYPED_FREE_IMMEDIATELY
};

static struct enumerator *
enumerator_ptr(VALUE obj)
{
struct enumerator *ptr;

TypedData_Get_Struct(obj, struct enumerator, &enumerator_data_type, ptr);
if (!ptr || ptr->obj == Qundef) {
rb_raise(rb_eArgError, "uninitialized enumerator");
}
return ptr;
}

static void
proc_entry_mark(void *p)
{
struct proc_entry *ptr = p;
rb_gc_mark_movable(ptr->proc);
rb_gc_mark_movable(ptr->memo);
}

static void
proc_entry_compact(void *p)
{
struct proc_entry *ptr = p;
ptr->proc = rb_gc_location(ptr->proc);
ptr->memo = rb_gc_location(ptr->memo);
}

#define proc_entry_free RUBY_TYPED_DEFAULT_FREE

static size_t
proc_entry_memsize(const void *p)
{
return p ? sizeof(struct proc_entry) : 0;
}

static const rb_data_type_t proc_entry_data_type = {
"proc_entry",
{
proc_entry_mark,
proc_entry_free,
proc_entry_memsize,
proc_entry_compact,
},
};

static struct proc_entry *
proc_entry_ptr(VALUE proc_entry)
{
struct proc_entry *ptr;

TypedData_Get_Struct(proc_entry, struct proc_entry, &proc_entry_data_type, ptr);

return ptr;
}






























































static VALUE
obj_to_enum(int argc, VALUE *argv, VALUE obj)
{
VALUE enumerator, meth = sym_each;

if (argc > 0) {
--argc;
meth = *argv++;
}
enumerator = rb_enumeratorize_with_size(obj, meth, argc, argv, 0);
if (rb_block_given_p()) {
enumerator_ptr(enumerator)->size = rb_block_proc();
}
return enumerator;
}

static VALUE
enumerator_allocate(VALUE klass)
{
struct enumerator *ptr;
VALUE enum_obj;

enum_obj = TypedData_Make_Struct(klass, struct enumerator, &enumerator_data_type, ptr);
ptr->obj = Qundef;

return enum_obj;
}

static VALUE
enumerator_init(VALUE enum_obj, VALUE obj, VALUE meth, int argc, const VALUE *argv, rb_enumerator_size_func *size_fn, VALUE size, int kw_splat)
{
struct enumerator *ptr;

rb_check_frozen(enum_obj);
TypedData_Get_Struct(enum_obj, struct enumerator, &enumerator_data_type, ptr);

if (!ptr) {
rb_raise(rb_eArgError, "unallocated enumerator");
}

ptr->obj = obj;
ptr->meth = rb_to_id(meth);
if (argc) ptr->args = rb_ary_new4(argc, argv);
ptr->fib = 0;
ptr->dst = Qnil;
ptr->lookahead = Qundef;
ptr->feedvalue = Qundef;
ptr->stop_exc = Qfalse;
ptr->size = size;
ptr->size_fn = size_fn;
ptr->kw_splat = kw_splat;

return enum_obj;
}







































static VALUE
enumerator_initialize(int argc, VALUE *argv, VALUE obj)
{
VALUE recv, meth = sym_each;
VALUE size = Qnil;
int kw_splat = 0;

if (rb_block_given_p()) {
rb_check_arity(argc, 0, 1);
recv = generator_init(generator_allocate(rb_cGenerator), rb_block_proc());
if (argc) {
if (NIL_P(argv[0]) || rb_respond_to(argv[0], id_call) ||
(RB_TYPE_P(argv[0], T_FLOAT) && RFLOAT_VALUE(argv[0]) == HUGE_VAL)) {
size = argv[0];
}
else {
size = rb_to_int(argv[0]);
}
argc = 0;
}
}
else {
rb_check_arity(argc, 1, UNLIMITED_ARGUMENTS);
rb_warn_deprecated("Enumerator.new without a block", "Object#to_enum");
recv = *argv++;
if (--argc) {
meth = *argv++;
--argc;
}
kw_splat = rb_keyword_given_p();
}

return enumerator_init(obj, recv, meth, argc, argv, 0, size, kw_splat);
}


static VALUE
enumerator_init_copy(VALUE obj, VALUE orig)
{
struct enumerator *ptr0, *ptr1;

if (!OBJ_INIT_COPY(obj, orig)) return obj;
ptr0 = enumerator_ptr(orig);
if (ptr0->fib) {

rb_raise(rb_eTypeError, "can't copy execution context");
}

TypedData_Get_Struct(obj, struct enumerator, &enumerator_data_type, ptr1);

if (!ptr1) {
rb_raise(rb_eArgError, "unallocated enumerator");
}

ptr1->obj = ptr0->obj;
ptr1->meth = ptr0->meth;
ptr1->args = ptr0->args;
ptr1->fib = 0;
ptr1->lookahead = Qundef;
ptr1->feedvalue = Qundef;
ptr1->size = ptr0->size;
ptr1->size_fn = ptr0->size_fn;

return obj;
}




VALUE
rb_enumeratorize(VALUE obj, VALUE meth, int argc, const VALUE *argv)
{
return rb_enumeratorize_with_size(obj, meth, argc, argv, 0);
}

static VALUE
lazy_to_enum_i(VALUE self, VALUE meth, int argc, const VALUE *argv, rb_enumerator_size_func *size_fn, int kw_splat);

VALUE
rb_enumeratorize_with_size(VALUE obj, VALUE meth, int argc, const VALUE *argv, rb_enumerator_size_func *size_fn)
{


if (RTEST(rb_obj_is_kind_of(obj, rb_cLazy)))
return lazy_to_enum_i(obj, meth, argc, argv, size_fn, rb_keyword_given_p());
else
return enumerator_init(enumerator_allocate(rb_cEnumerator),
obj, meth, argc, argv, size_fn, Qnil, rb_keyword_given_p());
}

VALUE
rb_enumeratorize_with_size_kw(VALUE obj, VALUE meth, int argc, const VALUE *argv, rb_enumerator_size_func *size_fn, int kw_splat)
{


if (RTEST(rb_obj_is_kind_of(obj, rb_cLazy)))
return lazy_to_enum_i(obj, meth, argc, argv, size_fn, kw_splat);
else
return enumerator_init(enumerator_allocate(rb_cEnumerator),
obj, meth, argc, argv, size_fn, Qnil, kw_splat);
}

static VALUE
enumerator_block_call(VALUE obj, rb_block_call_func *func, VALUE arg)
{
int argc = 0;
const VALUE *argv = 0;
const struct enumerator *e = enumerator_ptr(obj);
ID meth = e->meth;

if (e->args) {
argc = RARRAY_LENINT(e->args);
argv = RARRAY_CONST_PTR(e->args);
}
return rb_block_call_kw(e->obj, meth, argc, argv, func, arg, e->kw_splat);
}





































static VALUE
enumerator_each(int argc, VALUE *argv, VALUE obj)
{
if (argc > 0) {
struct enumerator *e = enumerator_ptr(obj = rb_obj_dup(obj));
VALUE args = e->args;
if (args) {
#if SIZEOF_INT < SIZEOF_LONG

rb_long2int(RARRAY_LEN(args) + argc);
#endif
args = rb_ary_dup(args);
rb_ary_cat(args, argv, argc);
}
else {
args = rb_ary_new4(argc, argv);
}
e->args = args;
e->size = Qnil;
e->size_fn = 0;
}
if (!rb_block_given_p()) return obj;
return enumerator_block_call(obj, 0, obj);
}

static VALUE
enumerator_with_index_i(RB_BLOCK_CALL_FUNC_ARGLIST(val, m))
{
struct MEMO *memo = (struct MEMO *)m;
VALUE idx = memo->v1;
MEMO_V1_SET(memo, rb_int_succ(idx));

if (argc <= 1)
return rb_yield_values(2, val, idx);

return rb_yield_values(2, rb_ary_new4(argc, argv), idx);
}

static VALUE
enumerator_size(VALUE obj);

static VALUE
enumerator_enum_size(VALUE obj, VALUE args, VALUE eobj)
{
return enumerator_size(obj);
}













static VALUE
enumerator_with_index(int argc, VALUE *argv, VALUE obj)
{
VALUE memo;

rb_check_arity(argc, 0, 1);
RETURN_SIZED_ENUMERATOR(obj, argc, argv, enumerator_enum_size);
memo = (!argc || NIL_P(memo = argv[0])) ? INT2FIX(0) : rb_to_int(memo);
return enumerator_block_call(obj, enumerator_with_index_i, (VALUE)MEMO_NEW(memo, 0, 0));
}











static VALUE
enumerator_each_with_index(VALUE obj)
{
return enumerator_with_index(0, NULL, obj);
}

static VALUE
enumerator_with_object_i(RB_BLOCK_CALL_FUNC_ARGLIST(val, memo))
{
if (argc <= 1)
return rb_yield_values(2, val, memo);

return rb_yield_values(2, rb_ary_new4(argc, argv), memo);
}






























static VALUE
enumerator_with_object(VALUE obj, VALUE memo)
{
RETURN_SIZED_ENUMERATOR(obj, 1, &memo, enumerator_enum_size);
enumerator_block_call(obj, enumerator_with_object_i, memo);

return memo;
}

static VALUE
next_ii(RB_BLOCK_CALL_FUNC_ARGLIST(i, obj))
{
struct enumerator *e = enumerator_ptr(obj);
VALUE feedvalue = Qnil;
VALUE args = rb_ary_new4(argc, argv);
rb_fiber_yield(1, &args);
if (e->feedvalue != Qundef) {
feedvalue = e->feedvalue;
e->feedvalue = Qundef;
}
return feedvalue;
}

static VALUE
next_i(RB_BLOCK_CALL_FUNC_ARGLIST(_, obj))
{
struct enumerator *e = enumerator_ptr(obj);
VALUE nil = Qnil;
VALUE result;

result = rb_block_call(obj, id_each, 0, 0, next_ii, obj);
e->stop_exc = rb_exc_new2(rb_eStopIteration, "iteration reached an end");
rb_ivar_set(e->stop_exc, id_result, result);
return rb_fiber_yield(1, &nil);
}

static void
next_init(VALUE obj, struct enumerator *e)
{
VALUE curr = rb_fiber_current();
e->dst = curr;
e->fib = rb_fiber_new(next_i, obj);
e->lookahead = Qundef;
}

static VALUE
get_next_values(VALUE obj, struct enumerator *e)
{
VALUE curr, vs;

if (e->stop_exc)
rb_exc_raise(e->stop_exc);

curr = rb_fiber_current();

if (!e->fib || !rb_fiber_alive_p(e->fib)) {
next_init(obj, e);
}

vs = rb_fiber_resume(e->fib, 1, &curr);
if (e->stop_exc) {
e->fib = 0;
e->dst = Qnil;
e->lookahead = Qundef;
e->feedvalue = Qundef;
rb_exc_raise(e->stop_exc);
}
return vs;
}
















































static VALUE
enumerator_next_values(VALUE obj)
{
struct enumerator *e = enumerator_ptr(obj);
VALUE vs;

if (e->lookahead != Qundef) {
vs = e->lookahead;
e->lookahead = Qundef;
return vs;
}

return get_next_values(obj, e);
}

static VALUE
ary2sv(VALUE args, int dup)
{
if (!RB_TYPE_P(args, T_ARRAY))
return args;

switch (RARRAY_LEN(args)) {
case 0:
return Qnil;

case 1:
return RARRAY_AREF(args, 0);

default:
if (dup)
return rb_ary_dup(args);
return args;
}
}























static VALUE
enumerator_next(VALUE obj)
{
VALUE vs = enumerator_next_values(obj);
return ary2sv(vs, 0);
}

static VALUE
enumerator_peek_values(VALUE obj)
{
struct enumerator *e = enumerator_ptr(obj);

if (e->lookahead == Qundef) {
e->lookahead = get_next_values(obj, e);
}
return e->lookahead;
}





























static VALUE
enumerator_peek_values_m(VALUE obj)
{
return rb_ary_dup(enumerator_peek_values(obj));
}























static VALUE
enumerator_peek(VALUE obj)
{
VALUE vs = enumerator_peek_values(obj);
return ary2sv(vs, 1);
}















































static VALUE
enumerator_feed(VALUE obj, VALUE v)
{
struct enumerator *e = enumerator_ptr(obj);

if (e->feedvalue != Qundef) {
rb_raise(rb_eTypeError, "feed value already set");
}
e->feedvalue = v;

return Qnil;
}










static VALUE
enumerator_rewind(VALUE obj)
{
struct enumerator *e = enumerator_ptr(obj);

rb_check_funcall(e->obj, id_rewind, 0, 0);

e->fib = 0;
e->dst = Qnil;
e->lookahead = Qundef;
e->feedvalue = Qundef;
e->stop_exc = Qfalse;
return obj;
}

static struct generator *generator_ptr(VALUE obj);
static VALUE append_method(VALUE obj, VALUE str, ID default_method, VALUE default_args);

static VALUE
inspect_enumerator(VALUE obj, VALUE dummy, int recur)
{
struct enumerator *e;
VALUE eobj, str, cname;

TypedData_Get_Struct(obj, struct enumerator, &enumerator_data_type, e);

cname = rb_obj_class(obj);

if (!e || e->obj == Qundef) {
return rb_sprintf("#<%"PRIsVALUE": uninitialized>", rb_class_path(cname));
}

if (recur) {
str = rb_sprintf("#<%"PRIsVALUE": ...>", rb_class_path(cname));
return str;
}

if (e->procs) {
long i;

eobj = generator_ptr(e->obj)->obj;

if (rb_obj_class(eobj) == cname) {
str = rb_inspect(eobj);
}
else {
str = rb_sprintf("#<%"PRIsVALUE": %+"PRIsVALUE">", rb_class_path(cname), eobj);
}
for (i = 0; i < RARRAY_LEN(e->procs); i++) {
str = rb_sprintf("#<%"PRIsVALUE": %"PRIsVALUE, cname, str);
append_method(RARRAY_AREF(e->procs, i), str, e->meth, e->args);
rb_str_buf_cat2(str, ">");
}
return str;
}

eobj = rb_attr_get(obj, id_receiver);
if (NIL_P(eobj)) {
eobj = e->obj;
}


str = rb_sprintf("#<%"PRIsVALUE": %+"PRIsVALUE, rb_class_path(cname), eobj);
append_method(obj, str, e->meth, e->args);

rb_str_buf_cat2(str, ">");

return str;
}

static int
key_symbol_p(VALUE key, VALUE val, VALUE arg)
{
if (SYMBOL_P(key)) return ST_CONTINUE;
*(int *)arg = FALSE;
return ST_STOP;
}

static int
kwd_append(VALUE key, VALUE val, VALUE str)
{
if (!SYMBOL_P(key)) rb_raise(rb_eRuntimeError, "non-symbol key inserted");
rb_str_catf(str, "% "PRIsVALUE": %"PRIsVALUE", ", key, val);
return ST_CONTINUE;
}

static VALUE
append_method(VALUE obj, VALUE str, ID default_method, VALUE default_args)
{
VALUE method, eargs;

method = rb_attr_get(obj, id_method);
if (method != Qfalse) {
if (!NIL_P(method)) {
Check_Type(method, T_SYMBOL);
method = rb_sym2str(method);
}
else {
method = rb_id2str(default_method);
}
rb_str_buf_cat2(str, ":");
rb_str_buf_append(str, method);
}

eargs = rb_attr_get(obj, id_arguments);
if (NIL_P(eargs)) {
eargs = default_args;
}
if (eargs != Qfalse) {
long argc = RARRAY_LEN(eargs);
const VALUE *argv = RARRAY_CONST_PTR(eargs); 

if (argc > 0) {
VALUE kwds = Qnil;

rb_str_buf_cat2(str, "(");

if (RB_TYPE_P(argv[argc-1], T_HASH) && !RHASH_EMPTY_P(argv[argc-1])) {
int all_key = TRUE;
rb_hash_foreach(argv[argc-1], key_symbol_p, (VALUE)&all_key);
if (all_key) kwds = argv[--argc];
}

while (argc--) {
VALUE arg = *argv++;

rb_str_append(str, rb_inspect(arg));
rb_str_buf_cat2(str, ", ");
}
if (!NIL_P(kwds)) {
rb_hash_foreach(kwds, kwd_append, str);
}
rb_str_set_len(str, RSTRING_LEN(str)-2);
rb_str_buf_cat2(str, ")");
}
}

return str;
}








static VALUE
enumerator_inspect(VALUE obj)
{
return rb_exec_recursive(inspect_enumerator, obj, 0);
}












static VALUE
enumerator_size(VALUE obj)
{
struct enumerator *e = enumerator_ptr(obj);
int argc = 0;
const VALUE *argv = NULL;
VALUE size;

if (e->procs) {
struct generator *g = generator_ptr(e->obj);
VALUE receiver = rb_check_funcall(g->obj, id_size, 0, 0);
long i = 0;

for (i = 0; i < RARRAY_LEN(e->procs); i++) {
VALUE proc = RARRAY_AREF(e->procs, i);
struct proc_entry *entry = proc_entry_ptr(proc);
lazyenum_size_func *size_fn = entry->fn->size;
if (!size_fn) {
return Qnil;
}
receiver = (*size_fn)(proc, receiver);
}
return receiver;
}

if (e->size_fn) {
return (*e->size_fn)(e->obj, e->args, obj);
}
if (e->args) {
argc = (int)RARRAY_LEN(e->args);
argv = RARRAY_CONST_PTR(e->args);
}
size = rb_check_funcall_kw(e->size, id_call, argc, argv, e->kw_splat);
if (size != Qundef) return size;
return e->size;
}




static void
yielder_mark(void *p)
{
struct yielder *ptr = p;
rb_gc_mark_movable(ptr->proc);
}

static void
yielder_compact(void *p)
{
struct yielder *ptr = p;
ptr->proc = rb_gc_location(ptr->proc);
}

#define yielder_free RUBY_TYPED_DEFAULT_FREE

static size_t
yielder_memsize(const void *p)
{
return sizeof(struct yielder);
}

static const rb_data_type_t yielder_data_type = {
"yielder",
{
yielder_mark,
yielder_free,
yielder_memsize,
yielder_compact,
},
0, 0, RUBY_TYPED_FREE_IMMEDIATELY
};

static struct yielder *
yielder_ptr(VALUE obj)
{
struct yielder *ptr;

TypedData_Get_Struct(obj, struct yielder, &yielder_data_type, ptr);
if (!ptr || ptr->proc == Qundef) {
rb_raise(rb_eArgError, "uninitialized yielder");
}
return ptr;
}


static VALUE
yielder_allocate(VALUE klass)
{
struct yielder *ptr;
VALUE obj;

obj = TypedData_Make_Struct(klass, struct yielder, &yielder_data_type, ptr);
ptr->proc = Qundef;

return obj;
}

static VALUE
yielder_init(VALUE obj, VALUE proc)
{
struct yielder *ptr;

TypedData_Get_Struct(obj, struct yielder, &yielder_data_type, ptr);

if (!ptr) {
rb_raise(rb_eArgError, "unallocated yielder");
}

ptr->proc = proc;

return obj;
}


static VALUE
yielder_initialize(VALUE obj)
{
rb_need_block();

return yielder_init(obj, rb_block_proc());
}


static VALUE
yielder_yield(VALUE obj, VALUE args)
{
struct yielder *ptr = yielder_ptr(obj);

return rb_proc_call_kw(ptr->proc, args, RB_PASS_CALLED_KEYWORDS);
}


static VALUE
yielder_yield_push(VALUE obj, VALUE arg)
{
struct yielder *ptr = yielder_ptr(obj);

rb_proc_call_with_block(ptr->proc, 1, &arg, Qnil);

return obj;
}













static VALUE
yielder_to_proc(VALUE obj)
{
VALUE method = rb_obj_method(obj, sym_yield);

return rb_funcall(method, idTo_proc, 0);
}

static VALUE
yielder_yield_i(RB_BLOCK_CALL_FUNC_ARGLIST(obj, memo))
{
return rb_yield_values_kw(argc, argv, RB_PASS_CALLED_KEYWORDS);
}

static VALUE
yielder_new(void)
{
return yielder_init(yielder_allocate(rb_cYielder), rb_proc_new(yielder_yield_i, 0));
}




static void
generator_mark(void *p)
{
struct generator *ptr = p;
rb_gc_mark_movable(ptr->proc);
rb_gc_mark_movable(ptr->obj);
}

static void
generator_compact(void *p)
{
struct generator *ptr = p;
ptr->proc = rb_gc_location(ptr->proc);
ptr->obj = rb_gc_location(ptr->obj);
}

#define generator_free RUBY_TYPED_DEFAULT_FREE

static size_t
generator_memsize(const void *p)
{
return sizeof(struct generator);
}

static const rb_data_type_t generator_data_type = {
"generator",
{
generator_mark,
generator_free,
generator_memsize,
generator_compact,
},
0, 0, RUBY_TYPED_FREE_IMMEDIATELY
};

static struct generator *
generator_ptr(VALUE obj)
{
struct generator *ptr;

TypedData_Get_Struct(obj, struct generator, &generator_data_type, ptr);
if (!ptr || ptr->proc == Qundef) {
rb_raise(rb_eArgError, "uninitialized generator");
}
return ptr;
}


static VALUE
generator_allocate(VALUE klass)
{
struct generator *ptr;
VALUE obj;

obj = TypedData_Make_Struct(klass, struct generator, &generator_data_type, ptr);
ptr->proc = Qundef;

return obj;
}

static VALUE
generator_init(VALUE obj, VALUE proc)
{
struct generator *ptr;

rb_check_frozen(obj);
TypedData_Get_Struct(obj, struct generator, &generator_data_type, ptr);

if (!ptr) {
rb_raise(rb_eArgError, "unallocated generator");
}

ptr->proc = proc;

return obj;
}


static VALUE
generator_initialize(int argc, VALUE *argv, VALUE obj)
{
VALUE proc;

if (argc == 0) {
rb_need_block();

proc = rb_block_proc();
}
else {
rb_scan_args(argc, argv, "1", &proc);

if (!rb_obj_is_proc(proc))
rb_raise(rb_eTypeError,
"wrong argument type %"PRIsVALUE" (expected Proc)",
rb_obj_class(proc));

if (rb_block_given_p()) {
rb_warn("given block not used");
}
}

return generator_init(obj, proc);
}


static VALUE
generator_init_copy(VALUE obj, VALUE orig)
{
struct generator *ptr0, *ptr1;

if (!OBJ_INIT_COPY(obj, orig)) return obj;

ptr0 = generator_ptr(orig);

TypedData_Get_Struct(obj, struct generator, &generator_data_type, ptr1);

if (!ptr1) {
rb_raise(rb_eArgError, "unallocated generator");
}

ptr1->proc = ptr0->proc;

return obj;
}


static VALUE
generator_each(int argc, VALUE *argv, VALUE obj)
{
struct generator *ptr = generator_ptr(obj);
VALUE args = rb_ary_new2(argc + 1);

rb_ary_push(args, yielder_new());
if (argc > 0) {
rb_ary_cat(args, argv, argc);
}

return rb_proc_call_kw(ptr->proc, args, RB_PASS_CALLED_KEYWORDS);
}


static VALUE
enum_size(VALUE self)
{
VALUE r = rb_check_funcall(self, id_size, 0, 0);
return (r == Qundef) ? Qnil : r;
}

static VALUE
lazyenum_size(VALUE self, VALUE args, VALUE eobj)
{
return enum_size(self);
}

static VALUE
lazy_size(VALUE self)
{
return enum_size(rb_ivar_get(self, id_receiver));
}

static VALUE
lazy_receiver_size(VALUE generator, VALUE args, VALUE lazy)
{
return lazy_size(lazy);
}

static VALUE
lazy_init_iterator(RB_BLOCK_CALL_FUNC_ARGLIST(val, m))
{
VALUE result;
if (argc == 1) {
VALUE args[2];
args[0] = m;
args[1] = val;
result = rb_yield_values2(2, args);
}
else {
VALUE args;
int len = rb_long2int((long)argc + 1);
VALUE *nargv = ALLOCV_N(VALUE, args, len);

nargv[0] = m;
if (argc > 0) {
MEMCPY(nargv + 1, argv, VALUE, argc);
}
result = rb_yield_values2(len, nargv);
ALLOCV_END(args);
}
if (result == Qundef) rb_iter_break();
return Qnil;
}

static VALUE
lazy_init_block_i(RB_BLOCK_CALL_FUNC_ARGLIST(val, m))
{
rb_block_call(m, id_each, argc-1, argv+1, lazy_init_iterator, val);
return Qnil;
}

#define memo_value v2
#define memo_flags u3.state
#define LAZY_MEMO_BREAK 1
#define LAZY_MEMO_PACKED 2
#define LAZY_MEMO_BREAK_P(memo) ((memo)->memo_flags & LAZY_MEMO_BREAK)
#define LAZY_MEMO_PACKED_P(memo) ((memo)->memo_flags & LAZY_MEMO_PACKED)
#define LAZY_MEMO_SET_BREAK(memo) ((memo)->memo_flags |= LAZY_MEMO_BREAK)
#define LAZY_MEMO_SET_VALUE(memo, value) MEMO_V2_SET(memo, value)
#define LAZY_MEMO_SET_PACKED(memo) ((memo)->memo_flags |= LAZY_MEMO_PACKED)
#define LAZY_MEMO_RESET_PACKED(memo) ((memo)->memo_flags &= ~LAZY_MEMO_PACKED)

static VALUE
lazy_init_yielder(RB_BLOCK_CALL_FUNC_ARGLIST(_, m))
{
VALUE yielder = RARRAY_AREF(m, 0);
VALUE procs_array = RARRAY_AREF(m, 1);
VALUE memos = rb_attr_get(yielder, id_memo);
long i = 0;
struct MEMO *result;
int cont = 1;

result = MEMO_NEW(Qnil, rb_enum_values_pack(argc, argv),
argc > 1 ? LAZY_MEMO_PACKED : 0);

for (i = 0; i < RARRAY_LEN(procs_array); i++) {
VALUE proc = RARRAY_AREF(procs_array, i);
struct proc_entry *entry = proc_entry_ptr(proc);
if (!(*entry->fn->proc)(proc, result, memos, i)) {
cont = 0;
break;
}
}

if (cont) {
rb_funcall2(yielder, idLTLT, 1, &(result->memo_value));
}
if (LAZY_MEMO_BREAK_P(result)) {
rb_iter_break();
}
return result->memo_value;
}

static VALUE
lazy_init_block(RB_BLOCK_CALL_FUNC_ARGLIST(val, m))
{
VALUE procs = RARRAY_AREF(m, 1);

rb_ivar_set(val, id_memo, rb_ary_new2(RARRAY_LEN(procs)));
rb_block_call(RARRAY_AREF(m, 0), id_each, 0, 0,
lazy_init_yielder, rb_ary_new3(2, val, procs));
return Qnil;
}

static VALUE
lazy_generator_init(VALUE enumerator, VALUE procs)
{
VALUE generator;
VALUE obj;
struct generator *gen_ptr;
struct enumerator *e = enumerator_ptr(enumerator);

if (RARRAY_LEN(procs) > 0) {
struct generator *old_gen_ptr = generator_ptr(e->obj);
obj = old_gen_ptr->obj;
}
else {
obj = enumerator;
}

generator = generator_allocate(rb_cGenerator);

rb_block_call(generator, id_initialize, 0, 0,
lazy_init_block, rb_ary_new3(2, obj, procs));

gen_ptr = generator_ptr(generator);
gen_ptr->obj = obj;

return generator;
}


























































































static VALUE
lazy_initialize(int argc, VALUE *argv, VALUE self)
{
VALUE obj, size = Qnil;
VALUE generator;

rb_check_arity(argc, 1, 2);
if (!rb_block_given_p()) {
rb_raise(rb_eArgError, "tried to call lazy new without a block");
}
obj = argv[0];
if (argc > 1) {
size = argv[1];
}
generator = generator_allocate(rb_cGenerator);
rb_block_call(generator, id_initialize, 0, 0, lazy_init_block_i, obj);
enumerator_init(self, generator, sym_each, 0, 0, 0, size, 0);
rb_ivar_set(self, id_receiver, obj);

return self;
}

#if 0 








static VALUE lazy_to_a(VALUE self)
{
}
#endif

static void
lazy_set_args(VALUE lazy, VALUE args)
{
ID id = rb_frame_this_func();
rb_ivar_set(lazy, id_method, ID2SYM(id));
if (NIL_P(args)) {

rb_ivar_set(lazy, id_arguments, Qfalse);
}
else {
rb_ivar_set(lazy, id_arguments, args);
}
}

static VALUE
lazy_set_method(VALUE lazy, VALUE args, rb_enumerator_size_func *size_fn)
{
struct enumerator *e = enumerator_ptr(lazy);
lazy_set_args(lazy, args);
e->size_fn = size_fn;
return lazy;
}

static VALUE
lazy_add_method(VALUE obj, int argc, VALUE *argv, VALUE args, VALUE memo,
const lazyenum_funcs *fn)
{
struct enumerator *new_e;
VALUE new_obj;
VALUE new_generator;
VALUE new_procs;
struct enumerator *e = enumerator_ptr(obj);
struct proc_entry *entry;
VALUE entry_obj = TypedData_Make_Struct(rb_cObject, struct proc_entry,
&proc_entry_data_type, entry);
if (rb_block_given_p()) {
entry->proc = rb_block_proc();
}
entry->fn = fn;
entry->memo = args;

lazy_set_args(entry_obj, memo);

new_procs = RTEST(e->procs) ? rb_ary_dup(e->procs) : rb_ary_new();
new_generator = lazy_generator_init(obj, new_procs);
rb_ary_push(new_procs, entry_obj);

new_obj = enumerator_init_copy(enumerator_allocate(rb_cLazy), obj);
new_e = DATA_PTR(new_obj);
new_e->obj = new_generator;
new_e->procs = new_procs;

if (argc > 0) {
new_e->meth = rb_to_id(*argv++);
--argc;
}
else {
new_e->meth = id_each;
}
new_e->args = rb_ary_new4(argc, argv);
return new_obj;
}






























static VALUE
enumerable_lazy(VALUE obj)
{
VALUE result = lazy_to_enum_i(obj, sym_each, 0, 0, lazyenum_size, rb_keyword_given_p());

rb_ivar_set(result, id_method, Qfalse);
return result;
}

static VALUE
lazy_to_enum_i(VALUE obj, VALUE meth, int argc, const VALUE *argv, rb_enumerator_size_func *size_fn, int kw_splat)
{
return enumerator_init(enumerator_allocate(rb_cLazy),
obj, meth, argc, argv, size_fn, Qnil, kw_splat);
}
























static VALUE
lazy_to_enum(int argc, VALUE *argv, VALUE self)
{
VALUE lazy, meth = sym_each, super_meth;

if (argc > 0) {
--argc;
meth = *argv++;
}
if (RTEST((super_meth = rb_hash_aref(lazy_use_super_method, meth)))) {
meth = super_meth;
}
lazy = lazy_to_enum_i(self, meth, argc, argv, 0, rb_keyword_given_p());
if (rb_block_given_p()) {
enumerator_ptr(lazy)->size = rb_block_proc();
}
return lazy;
}

static VALUE
lazy_eager_size(VALUE self, VALUE args, VALUE eobj)
{
return enum_size(self);
}








static VALUE
lazy_eager(VALUE self)
{
return enumerator_init(enumerator_allocate(rb_cEnumerator),
self, sym_each, 0, 0, lazy_eager_size, Qnil, 0);
}

static VALUE
lazyenum_yield(VALUE proc_entry, struct MEMO *result)
{
struct proc_entry *entry = proc_entry_ptr(proc_entry);
return rb_proc_call_with_block(entry->proc, 1, &result->memo_value, Qnil);
}

static VALUE
lazyenum_yield_values(VALUE proc_entry, struct MEMO *result)
{
struct proc_entry *entry = proc_entry_ptr(proc_entry);
int argc = 1;
const VALUE *argv = &result->memo_value;
if (LAZY_MEMO_PACKED_P(result)) {
const VALUE args = *argv;
argc = RARRAY_LENINT(args);
argv = RARRAY_CONST_PTR(args);
}
return rb_proc_call_with_block(entry->proc, argc, argv, Qnil);
}

static struct MEMO *
lazy_map_proc(VALUE proc_entry, struct MEMO *result, VALUE memos, long memo_index)
{
VALUE value = lazyenum_yield_values(proc_entry, result);
LAZY_MEMO_SET_VALUE(result, value);
LAZY_MEMO_RESET_PACKED(result);
return result;
}

static VALUE
lazy_map_size(VALUE entry, VALUE receiver)
{
return receiver;
}

static const lazyenum_funcs lazy_map_funcs = {
lazy_map_proc, lazy_map_size,
};














static VALUE
lazy_map(VALUE obj)
{
if (!rb_block_given_p()) {
rb_raise(rb_eArgError, "tried to call lazy map without a block");
}

return lazy_add_method(obj, 0, 0, Qnil, Qnil, &lazy_map_funcs);
}

static VALUE
lazy_flat_map_i(RB_BLOCK_CALL_FUNC_ARGLIST(i, yielder))
{
VALUE arg = rb_enum_values_pack(argc, argv);

return rb_funcallv(yielder, idLTLT, 1, &arg);
}

static VALUE
lazy_flat_map_each(VALUE obj, VALUE yielder)
{
rb_block_call(obj, id_each, 0, 0, lazy_flat_map_i, yielder);
return Qnil;
}

static VALUE
lazy_flat_map_to_ary(VALUE obj, VALUE yielder)
{
VALUE ary = rb_check_array_type(obj);
if (NIL_P(ary)) {
rb_funcall(yielder, idLTLT, 1, obj);
}
else {
long i;
for (i = 0; i < RARRAY_LEN(ary); i++) {
rb_funcall(yielder, idLTLT, 1, RARRAY_AREF(ary, i));
}
}
return Qnil;
}

static VALUE
lazy_flat_map_proc(RB_BLOCK_CALL_FUNC_ARGLIST(val, m))
{
VALUE result = rb_yield_values2(argc - 1, &argv[1]);
if (RB_TYPE_P(result, T_ARRAY)) {
long i;
for (i = 0; i < RARRAY_LEN(result); i++) {
rb_funcall(argv[0], idLTLT, 1, RARRAY_AREF(result, i));
}
}
else {
if (rb_respond_to(result, id_force) && rb_respond_to(result, id_each)) {
lazy_flat_map_each(result, argv[0]);
}
else {
lazy_flat_map_to_ary(result, argv[0]);
}
}
return Qnil;
}
























static VALUE
lazy_flat_map(VALUE obj)
{
if (!rb_block_given_p()) {
rb_raise(rb_eArgError, "tried to call lazy flat_map without a block");
}

return lazy_set_method(rb_block_call(rb_cLazy, id_new, 1, &obj,
lazy_flat_map_proc, 0),
Qnil, 0);
}

static struct MEMO *
lazy_select_proc(VALUE proc_entry, struct MEMO *result, VALUE memos, long memo_index)
{
VALUE chain = lazyenum_yield(proc_entry, result);
if (!RTEST(chain)) return 0;
return result;
}

static const lazyenum_funcs lazy_select_funcs = {
lazy_select_proc, 0,
};









static VALUE
lazy_select(VALUE obj)
{
if (!rb_block_given_p()) {
rb_raise(rb_eArgError, "tried to call lazy select without a block");
}

return lazy_add_method(obj, 0, 0, Qnil, Qnil, &lazy_select_funcs);
}

static struct MEMO *
lazy_filter_map_proc(VALUE proc_entry, struct MEMO *result, VALUE memos, long memo_index)
{
VALUE value = lazyenum_yield_values(proc_entry, result);
if (!RTEST(value)) return 0;
LAZY_MEMO_SET_VALUE(result, value);
LAZY_MEMO_RESET_PACKED(result);
return result;
}

static const lazyenum_funcs lazy_filter_map_funcs = {
lazy_filter_map_proc, 0,
};











static VALUE
lazy_filter_map(VALUE obj)
{
if (!rb_block_given_p()) {
rb_raise(rb_eArgError, "tried to call lazy filter_map without a block");
}

return lazy_add_method(obj, 0, 0, Qnil, Qnil, &lazy_filter_map_funcs);
}

static struct MEMO *
lazy_reject_proc(VALUE proc_entry, struct MEMO *result, VALUE memos, long memo_index)
{
VALUE chain = lazyenum_yield(proc_entry, result);
if (RTEST(chain)) return 0;
return result;
}

static const lazyenum_funcs lazy_reject_funcs = {
lazy_reject_proc, 0,
};








static VALUE
lazy_reject(VALUE obj)
{
if (!rb_block_given_p()) {
rb_raise(rb_eArgError, "tried to call lazy reject without a block");
}

return lazy_add_method(obj, 0, 0, Qnil, Qnil, &lazy_reject_funcs);
}

static struct MEMO *
lazy_grep_proc(VALUE proc_entry, struct MEMO *result, VALUE memos, long memo_index)
{
struct proc_entry *entry = proc_entry_ptr(proc_entry);
VALUE chain = rb_funcall(entry->memo, id_eqq, 1, result->memo_value);
if (!RTEST(chain)) return 0;
return result;
}

static struct MEMO *
lazy_grep_iter_proc(VALUE proc_entry, struct MEMO *result, VALUE memos, long memo_index)
{
struct proc_entry *entry = proc_entry_ptr(proc_entry);
VALUE value, chain = rb_funcall(entry->memo, id_eqq, 1, result->memo_value);

if (!RTEST(chain)) return 0;
value = rb_proc_call_with_block(entry->proc, 1, &(result->memo_value), Qnil);
LAZY_MEMO_SET_VALUE(result, value);
LAZY_MEMO_RESET_PACKED(result);

return result;
}

static const lazyenum_funcs lazy_grep_iter_funcs = {
lazy_grep_iter_proc, 0,
};

static const lazyenum_funcs lazy_grep_funcs = {
lazy_grep_proc, 0,
};









static VALUE
lazy_grep(VALUE obj, VALUE pattern)
{
const lazyenum_funcs *const funcs = rb_block_given_p() ?
&lazy_grep_iter_funcs : &lazy_grep_funcs;
return lazy_add_method(obj, 0, 0, pattern, rb_ary_new3(1, pattern), funcs);
}

static struct MEMO *
lazy_grep_v_proc(VALUE proc_entry, struct MEMO *result, VALUE memos, long memo_index)
{
struct proc_entry *entry = proc_entry_ptr(proc_entry);
VALUE chain = rb_funcall(entry->memo, id_eqq, 1, result->memo_value);
if (RTEST(chain)) return 0;
return result;
}

static struct MEMO *
lazy_grep_v_iter_proc(VALUE proc_entry, struct MEMO *result, VALUE memos, long memo_index)
{
struct proc_entry *entry = proc_entry_ptr(proc_entry);
VALUE value, chain = rb_funcall(entry->memo, id_eqq, 1, result->memo_value);

if (RTEST(chain)) return 0;
value = rb_proc_call_with_block(entry->proc, 1, &(result->memo_value), Qnil);
LAZY_MEMO_SET_VALUE(result, value);
LAZY_MEMO_RESET_PACKED(result);

return result;
}

static const lazyenum_funcs lazy_grep_v_iter_funcs = {
lazy_grep_v_iter_proc, 0,
};

static const lazyenum_funcs lazy_grep_v_funcs = {
lazy_grep_v_proc, 0,
};









static VALUE
lazy_grep_v(VALUE obj, VALUE pattern)
{
const lazyenum_funcs *const funcs = rb_block_given_p() ?
&lazy_grep_v_iter_funcs : &lazy_grep_v_funcs;
return lazy_add_method(obj, 0, 0, pattern, rb_ary_new3(1, pattern), funcs);
}

static VALUE
call_next(VALUE obj)
{
return rb_funcall(obj, id_next, 0);
}

static VALUE
next_stopped(VALUE obj, VALUE _)
{
return Qnil;
}

static VALUE
lazy_zip_arrays_func(RB_BLOCK_CALL_FUNC_ARGLIST(val, arrays))
{
VALUE yielder, ary, memo;
long i, count;

yielder = argv[0];
memo = rb_attr_get(yielder, id_memo);
count = NIL_P(memo) ? 0 : NUM2LONG(memo);

ary = rb_ary_new2(RARRAY_LEN(arrays) + 1);
rb_ary_push(ary, argv[1]);
for (i = 0; i < RARRAY_LEN(arrays); i++) {
rb_ary_push(ary, rb_ary_entry(RARRAY_AREF(arrays, i), count));
}
rb_funcall(yielder, idLTLT, 1, ary);
rb_ivar_set(yielder, id_memo, LONG2NUM(++count));
return Qnil;
}

static VALUE
lazy_zip_func(RB_BLOCK_CALL_FUNC_ARGLIST(val, zip_args))
{
VALUE yielder, ary, arg, v;
long i;

yielder = argv[0];
arg = rb_attr_get(yielder, id_memo);
if (NIL_P(arg)) {
arg = rb_ary_new2(RARRAY_LEN(zip_args));
for (i = 0; i < RARRAY_LEN(zip_args); i++) {
rb_ary_push(arg, rb_funcall(RARRAY_AREF(zip_args, i), id_to_enum, 0));
}
rb_ivar_set(yielder, id_memo, arg);
}

ary = rb_ary_new2(RARRAY_LEN(arg) + 1);
v = Qnil;
if (--argc > 0) {
++argv;
v = argc > 1 ? rb_ary_new_from_values(argc, argv) : *argv;
}
rb_ary_push(ary, v);
for (i = 0; i < RARRAY_LEN(arg); i++) {
v = rb_rescue2(call_next, RARRAY_AREF(arg, i), next_stopped, 0,
rb_eStopIteration, (VALUE)0);
rb_ary_push(ary, v);
}
rb_funcall(yielder, idLTLT, 1, ary);
return Qnil;
}









static VALUE
lazy_zip(int argc, VALUE *argv, VALUE obj)
{
VALUE ary, v;
long i;
rb_block_call_func *func = lazy_zip_arrays_func;

if (rb_block_given_p()) {
return rb_call_super(argc, argv);
}

ary = rb_ary_new2(argc);
for (i = 0; i < argc; i++) {
v = rb_check_array_type(argv[i]);
if (NIL_P(v)) {
for (; i < argc; i++) {
if (!rb_respond_to(argv[i], id_each)) {
rb_raise(rb_eTypeError, "wrong argument type %"PRIsVALUE" (must respond to :each)",
rb_obj_class(argv[i]));
}
}
ary = rb_ary_new4(argc, argv);
func = lazy_zip_func;
break;
}
rb_ary_push(ary, v);
}

return lazy_set_method(rb_block_call(rb_cLazy, id_new, 1, &obj,
func, ary),
ary, lazy_receiver_size);
}

static struct MEMO *
lazy_take_proc(VALUE proc_entry, struct MEMO *result, VALUE memos, long memo_index)
{
long remain;
struct proc_entry *entry = proc_entry_ptr(proc_entry);
VALUE memo = rb_ary_entry(memos, memo_index);

if (NIL_P(memo)) {
memo = entry->memo;
}

remain = NUM2LONG(memo);
if (remain == 0) {
LAZY_MEMO_SET_BREAK(result);
}
else {
if (--remain == 0) LAZY_MEMO_SET_BREAK(result);
rb_ary_store(memos, memo_index, LONG2NUM(remain));
}
return result;
}

static VALUE
lazy_take_size(VALUE entry, VALUE receiver)
{
long len = NUM2LONG(RARRAY_AREF(rb_ivar_get(entry, id_arguments), 0));
if (NIL_P(receiver) || (FIXNUM_P(receiver) && FIX2LONG(receiver) < len))
return receiver;
return LONG2NUM(len);
}

static const lazyenum_funcs lazy_take_funcs = {
lazy_take_proc, lazy_take_size,
};








static VALUE
lazy_take(VALUE obj, VALUE n)
{
long len = NUM2LONG(n);
int argc = 0;
VALUE argv[2];

if (len < 0) {
rb_raise(rb_eArgError, "attempt to take negative size");
}

if (len == 0) {
argv[0] = sym_cycle;
argv[1] = INT2NUM(0);
argc = 2;
}

return lazy_add_method(obj, argc, argv, n, rb_ary_new3(1, n), &lazy_take_funcs);
}

static struct MEMO *
lazy_take_while_proc(VALUE proc_entry, struct MEMO *result, VALUE memos, long memo_index)
{
VALUE take = lazyenum_yield_values(proc_entry, result);
if (!RTEST(take)) {
LAZY_MEMO_SET_BREAK(result);
return 0;
}
return result;
}

static const lazyenum_funcs lazy_take_while_funcs = {
lazy_take_while_proc, 0,
};








static VALUE
lazy_take_while(VALUE obj)
{
if (!rb_block_given_p()) {
rb_raise(rb_eArgError, "tried to call lazy take_while without a block");
}

return lazy_add_method(obj, 0, 0, Qnil, Qnil, &lazy_take_while_funcs);
}

static VALUE
lazy_drop_size(VALUE proc_entry, VALUE receiver)
{
long len = NUM2LONG(RARRAY_AREF(rb_ivar_get(proc_entry, id_arguments), 0));
if (NIL_P(receiver))
return receiver;
if (FIXNUM_P(receiver)) {
len = FIX2LONG(receiver) - len;
return LONG2FIX(len < 0 ? 0 : len);
}
return rb_funcall(receiver, '-', 1, LONG2NUM(len));
}

static struct MEMO *
lazy_drop_proc(VALUE proc_entry, struct MEMO *result, VALUE memos, long memo_index)
{
long remain;
struct proc_entry *entry = proc_entry_ptr(proc_entry);
VALUE memo = rb_ary_entry(memos, memo_index);

if (NIL_P(memo)) {
memo = entry->memo;
}
remain = NUM2LONG(memo);
if (remain > 0) {
--remain;
rb_ary_store(memos, memo_index, LONG2NUM(remain));
return 0;
}

return result;
}

static const lazyenum_funcs lazy_drop_funcs = {
lazy_drop_proc, lazy_drop_size,
};








static VALUE
lazy_drop(VALUE obj, VALUE n)
{
long len = NUM2LONG(n);
VALUE argv[2];
argv[0] = sym_each;
argv[1] = n;

if (len < 0) {
rb_raise(rb_eArgError, "attempt to drop negative size");
}

return lazy_add_method(obj, 2, argv, n, rb_ary_new3(1, n), &lazy_drop_funcs);
}

static struct MEMO *
lazy_drop_while_proc(VALUE proc_entry, struct MEMO* result, VALUE memos, long memo_index)
{
struct proc_entry *entry = proc_entry_ptr(proc_entry);
VALUE memo = rb_ary_entry(memos, memo_index);

if (NIL_P(memo)) {
memo = entry->memo;
}

if (!RTEST(memo)) {
VALUE drop = lazyenum_yield_values(proc_entry, result);
if (RTEST(drop)) return 0;
rb_ary_store(memos, memo_index, Qtrue);
}
return result;
}

static const lazyenum_funcs lazy_drop_while_funcs = {
lazy_drop_while_proc, 0,
};








static VALUE
lazy_drop_while(VALUE obj)
{
if (!rb_block_given_p()) {
rb_raise(rb_eArgError, "tried to call lazy drop_while without a block");
}

return lazy_add_method(obj, 0, 0, Qfalse, Qnil, &lazy_drop_while_funcs);
}

static int
lazy_uniq_check(VALUE chain, VALUE memos, long memo_index)
{
VALUE hash = rb_ary_entry(memos, memo_index);

if (NIL_P(hash)) {
hash = rb_obj_hide(rb_hash_new());
rb_ary_store(memos, memo_index, hash);
}

return rb_hash_add_new_element(hash, chain, Qfalse);
}

static struct MEMO *
lazy_uniq_proc(VALUE proc_entry, struct MEMO *result, VALUE memos, long memo_index)
{
if (lazy_uniq_check(result->memo_value, memos, memo_index)) return 0;
return result;
}

static struct MEMO *
lazy_uniq_iter_proc(VALUE proc_entry, struct MEMO *result, VALUE memos, long memo_index)
{
VALUE chain = lazyenum_yield(proc_entry, result);

if (lazy_uniq_check(chain, memos, memo_index)) return 0;
return result;
}

static const lazyenum_funcs lazy_uniq_iter_funcs = {
lazy_uniq_iter_proc, 0,
};

static const lazyenum_funcs lazy_uniq_funcs = {
lazy_uniq_proc, 0,
};









static VALUE
lazy_uniq(VALUE obj)
{
const lazyenum_funcs *const funcs =
rb_block_given_p() ? &lazy_uniq_iter_funcs : &lazy_uniq_funcs;
return lazy_add_method(obj, 0, 0, Qnil, Qnil, funcs);
}

static struct MEMO *
lazy_with_index_proc(VALUE proc_entry, struct MEMO* result, VALUE memos, long memo_index)
{
struct proc_entry *entry = proc_entry_ptr(proc_entry);
VALUE memo = rb_ary_entry(memos, memo_index);
VALUE argv[2];

if (NIL_P(memo)) {
memo = entry->memo;
}

argv[0] = result->memo_value;
argv[1] = memo;
if (entry->proc) {
rb_proc_call_with_block(entry->proc, 2, argv, Qnil);
LAZY_MEMO_RESET_PACKED(result);
} else {
LAZY_MEMO_SET_VALUE(result, rb_ary_new_from_values(2, argv));
LAZY_MEMO_SET_PACKED(result);
}
rb_ary_store(memos, memo_index, LONG2NUM(NUM2LONG(memo) + 1));
return result;
}

static const lazyenum_funcs lazy_with_index_funcs = {
lazy_with_index_proc, 0,
};

















static VALUE
lazy_with_index(int argc, VALUE *argv, VALUE obj)
{
VALUE memo;

rb_scan_args(argc, argv, "01", &memo);
if (NIL_P(memo))
memo = LONG2NUM(0);

return lazy_add_method(obj, 0, 0, memo, rb_ary_new_from_values(1, &memo), &lazy_with_index_funcs);
}

#if 0 







static VALUE lazy_chunk(VALUE self)
{
}







static VALUE lazy_chunk_while(VALUE self)
{
}








static VALUE lazy_slice_after(VALUE self)
{
}








static VALUE lazy_slice_before(VALUE self)
{
}







static VALUE lazy_slice_when(VALUE self)
{
}
#endif

static VALUE
lazy_super(int argc, VALUE *argv, VALUE lazy)
{
return enumerable_lazy(rb_call_super(argc, argv));
}








static VALUE
lazy_lazy(VALUE obj)
{
return obj;
}
















































static VALUE
stop_result(VALUE self)
{
return rb_attr_get(self, id_result);
}





static void
producer_mark(void *p)
{
struct producer *ptr = p;
rb_gc_mark_movable(ptr->init);
rb_gc_mark_movable(ptr->proc);
}

static void
producer_compact(void *p)
{
struct producer *ptr = p;
ptr->init = rb_gc_location(ptr->init);
ptr->proc = rb_gc_location(ptr->proc);
}

#define producer_free RUBY_TYPED_DEFAULT_FREE

static size_t
producer_memsize(const void *p)
{
return sizeof(struct producer);
}

static const rb_data_type_t producer_data_type = {
"producer",
{
producer_mark,
producer_free,
producer_memsize,
producer_compact,
},
0, 0, RUBY_TYPED_FREE_IMMEDIATELY
};

static struct producer *
producer_ptr(VALUE obj)
{
struct producer *ptr;

TypedData_Get_Struct(obj, struct producer, &producer_data_type, ptr);
if (!ptr || ptr->proc == Qundef) {
rb_raise(rb_eArgError, "uninitialized producer");
}
return ptr;
}


static VALUE
producer_allocate(VALUE klass)
{
struct producer *ptr;
VALUE obj;

obj = TypedData_Make_Struct(klass, struct producer, &producer_data_type, ptr);
ptr->init = Qundef;
ptr->proc = Qundef;

return obj;
}

static VALUE
producer_init(VALUE obj, VALUE init, VALUE proc)
{
struct producer *ptr;

TypedData_Get_Struct(obj, struct producer, &producer_data_type, ptr);

if (!ptr) {
rb_raise(rb_eArgError, "unallocated producer");
}

ptr->init = init;
ptr->proc = proc;

return obj;
}

static VALUE
producer_each_stop(VALUE dummy, VALUE exc)
{
return rb_attr_get(exc, id_result);
}

static VALUE
producer_each_i(VALUE obj)
{
struct producer *ptr;
VALUE init, proc, curr;

ptr = producer_ptr(obj);
init = ptr->init;
proc = ptr->proc;

if (init == Qundef) {
curr = Qnil;
} else {
rb_yield(init);
curr = init;
}

for (;;) {
curr = rb_funcall(proc, id_call, 1, curr);
rb_yield(curr);
}

return Qnil;
}


static VALUE
producer_each(VALUE obj)
{
rb_need_block();

return rb_rescue2(producer_each_i, obj, producer_each_stop, (VALUE)0, rb_eStopIteration, (VALUE)0);
}

static VALUE
producer_size(VALUE obj, VALUE args, VALUE eobj)
{
return DBL2NUM(HUGE_VAL);
}




































static VALUE
enumerator_s_produce(int argc, VALUE *argv, VALUE klass)
{
VALUE init, producer;

if (!rb_block_given_p()) rb_raise(rb_eArgError, "no block given");

if (rb_scan_args(argc, argv, "01", &init) == 0) {
init = Qundef;
}

producer = producer_init(producer_allocate(rb_cEnumProducer), init, rb_block_proc());

return rb_enumeratorize_with_size_kw(producer, sym_each, 0, 0, producer_size, RB_NO_KEYWORDS);
}











static void
enum_chain_mark(void *p)
{
struct enum_chain *ptr = p;
rb_gc_mark_movable(ptr->enums);
}

static void
enum_chain_compact(void *p)
{
struct enum_chain *ptr = p;
ptr->enums = rb_gc_location(ptr->enums);
}

#define enum_chain_free RUBY_TYPED_DEFAULT_FREE

static size_t
enum_chain_memsize(const void *p)
{
return sizeof(struct enum_chain);
}

static const rb_data_type_t enum_chain_data_type = {
"chain",
{
enum_chain_mark,
enum_chain_free,
enum_chain_memsize,
enum_chain_compact,
},
0, 0, RUBY_TYPED_FREE_IMMEDIATELY
};

static struct enum_chain *
enum_chain_ptr(VALUE obj)
{
struct enum_chain *ptr;

TypedData_Get_Struct(obj, struct enum_chain, &enum_chain_data_type, ptr);
if (!ptr || ptr->enums == Qundef) {
rb_raise(rb_eArgError, "uninitialized chain");
}
return ptr;
}


static VALUE
enum_chain_allocate(VALUE klass)
{
struct enum_chain *ptr;
VALUE obj;

obj = TypedData_Make_Struct(klass, struct enum_chain, &enum_chain_data_type, ptr);
ptr->enums = Qundef;
ptr->pos = -1;

return obj;
}












static VALUE
enum_chain_initialize(VALUE obj, VALUE enums)
{
struct enum_chain *ptr;

rb_check_frozen(obj);
TypedData_Get_Struct(obj, struct enum_chain, &enum_chain_data_type, ptr);

if (!ptr) rb_raise(rb_eArgError, "unallocated chain");

ptr->enums = rb_obj_freeze(enums);
ptr->pos = -1;

return obj;
}


static VALUE
enum_chain_init_copy(VALUE obj, VALUE orig)
{
struct enum_chain *ptr0, *ptr1;

if (!OBJ_INIT_COPY(obj, orig)) return obj;
ptr0 = enum_chain_ptr(orig);

TypedData_Get_Struct(obj, struct enum_chain, &enum_chain_data_type, ptr1);

if (!ptr1) rb_raise(rb_eArgError, "unallocated chain");

ptr1->enums = ptr0->enums;
ptr1->pos = ptr0->pos;

return obj;
}

static VALUE
enum_chain_total_size(VALUE enums)
{
VALUE total = INT2FIX(0);
long i;

for (i = 0; i < RARRAY_LEN(enums); i++) {
VALUE size = enum_size(RARRAY_AREF(enums, i));

if (NIL_P(size) || (RB_TYPE_P(size, T_FLOAT) && isinf(NUM2DBL(size)))) {
return size;
}
if (!RB_INTEGER_TYPE_P(size)) {
return Qnil;
}

total = rb_funcall(total, '+', 1, size);
}

return total;
}










static VALUE
enum_chain_size(VALUE obj)
{
return enum_chain_total_size(enum_chain_ptr(obj)->enums);
}

static VALUE
enum_chain_enum_size(VALUE obj, VALUE args, VALUE eobj)
{
return enum_chain_size(obj);
}

static VALUE
enum_chain_enum_no_size(VALUE obj, VALUE args, VALUE eobj)
{
return Qnil;
}













static VALUE
enum_chain_each(int argc, VALUE *argv, VALUE obj)
{
VALUE enums, block;
struct enum_chain *objptr;
long i;

RETURN_SIZED_ENUMERATOR(obj, argc, argv, argc > 0 ? enum_chain_enum_no_size : enum_chain_enum_size);

objptr = enum_chain_ptr(obj);
enums = objptr->enums;
block = rb_block_proc();

for (i = 0; i < RARRAY_LEN(enums); i++) {
objptr->pos = i;
rb_funcall_with_block(RARRAY_AREF(enums, i), id_each, argc, argv, block);
}

return obj;
}









static VALUE
enum_chain_rewind(VALUE obj)
{
struct enum_chain *objptr = enum_chain_ptr(obj);
VALUE enums = objptr->enums;
long i;

for (i = objptr->pos; 0 <= i && i < RARRAY_LEN(enums); objptr->pos = --i) {
rb_check_funcall(RARRAY_AREF(enums, i), id_rewind, 0, 0);
}

return obj;
}

static VALUE
inspect_enum_chain(VALUE obj, VALUE dummy, int recur)
{
VALUE klass = rb_obj_class(obj);
struct enum_chain *ptr;

TypedData_Get_Struct(obj, struct enum_chain, &enum_chain_data_type, ptr);

if (!ptr || ptr->enums == Qundef) {
return rb_sprintf("#<%"PRIsVALUE": uninitialized>", rb_class_path(klass));
}

if (recur) {
return rb_sprintf("#<%"PRIsVALUE": ...>", rb_class_path(klass));
}

return rb_sprintf("#<%"PRIsVALUE": %+"PRIsVALUE">", rb_class_path(klass), ptr->enums);
}







static VALUE
enum_chain_inspect(VALUE obj)
{
return rb_exec_recursive(inspect_enum_chain, obj, 0);
}











static VALUE
enum_chain(int argc, VALUE *argv, VALUE obj)
{
VALUE enums = rb_ary_new_from_values(1, &obj);
rb_ary_cat(enums, argv, argc);

return enum_chain_initialize(enum_chain_allocate(rb_cEnumChain), enums);
}











static VALUE
enumerator_plus(VALUE obj, VALUE eobj)
{
VALUE enums = rb_ary_new_from_args(2, obj, eobj);

return enum_chain_initialize(enum_chain_allocate(rb_cEnumChain), enums);
}










VALUE
rb_arith_seq_new(VALUE obj, VALUE meth, int argc, VALUE const *argv,
rb_enumerator_size_func *size_fn,
VALUE beg, VALUE end, VALUE step, int excl)
{
VALUE aseq = enumerator_init(enumerator_allocate(rb_cArithSeq),
obj, meth, argc, argv, size_fn, Qnil, rb_keyword_given_p());
rb_ivar_set(aseq, id_begin, beg);
rb_ivar_set(aseq, id_end, end);
rb_ivar_set(aseq, id_step, step);
rb_ivar_set(aseq, id_exclude_end, excl ? Qtrue : Qfalse);
return aseq;
}







static inline VALUE
arith_seq_begin(VALUE self)
{
return rb_ivar_get(self, id_begin);
}






static inline VALUE
arith_seq_end(VALUE self)
{
return rb_ivar_get(self, id_end);
}







static inline VALUE
arith_seq_step(VALUE self)
{
return rb_ivar_get(self, id_step);
}






static inline VALUE
arith_seq_exclude_end(VALUE self)
{
return rb_ivar_get(self, id_exclude_end);
}

static inline int
arith_seq_exclude_end_p(VALUE self)
{
return RTEST(arith_seq_exclude_end(self));
}

int
rb_arithmetic_sequence_extract(VALUE obj, rb_arithmetic_sequence_components_t *component)
{
if (rb_obj_is_kind_of(obj, rb_cArithSeq)) {
component->begin = arith_seq_begin(obj);
component->end = arith_seq_end(obj);
component->step = arith_seq_step(obj);
component->exclude_end = arith_seq_exclude_end_p(obj);
return 1;
}
else if (rb_obj_is_kind_of(obj, rb_cRange)) {
component->begin = RANGE_BEG(obj);
component->end = RANGE_END(obj);
component->step = INT2FIX(1);
component->exclude_end = RTEST(RANGE_EXCL(obj));
return 1;
}

return 0;
}









static VALUE
arith_seq_first(int argc, VALUE *argv, VALUE self)
{
VALUE b, e, s, ary;
long n;
int x;

rb_check_arity(argc, 0, 1);

b = arith_seq_begin(self);
e = arith_seq_end(self);
s = arith_seq_step(self);
if (argc == 0) {
if (NIL_P(b)) {
return Qnil;
}
if (!NIL_P(e)) {
VALUE zero = INT2FIX(0);
int r = rb_cmpint(rb_num_coerce_cmp(s, zero, idCmp), s, zero);
if (r > 0 && RTEST(rb_funcall(b, '>', 1, e))) {
return Qnil;
}
if (r < 0 && RTEST(rb_funcall(b, '<', 1, e))) {
return Qnil;
}
}
return b;
}



n = NUM2LONG(argv[0]);
if (n < 0) {
rb_raise(rb_eArgError, "attempt to take negative size");
}
if (n == 0) {
return rb_ary_new_capa(0);
}

x = arith_seq_exclude_end_p(self);

if (FIXNUM_P(b) && NIL_P(e) && FIXNUM_P(s)) {
long i = FIX2LONG(b), unit = FIX2LONG(s);
ary = rb_ary_new_capa(n);
while (n > 0 && FIXABLE(i)) {
rb_ary_push(ary, LONG2FIX(i));
i += unit; 
--n;
}
if (n > 0) {
b = LONG2NUM(i);
while (n > 0) {
rb_ary_push(ary, b);
b = rb_big_plus(b, s);
--n;
}
}
return ary;
}
else if (FIXNUM_P(b) && FIXNUM_P(e) && FIXNUM_P(s)) {
long i = FIX2LONG(b);
long end = FIX2LONG(e);
long unit = FIX2LONG(s);
long len;

if (unit >= 0) {
if (!x) end += 1;

len = end - i;
if (len < 0) len = 0;
ary = rb_ary_new_capa((n < len) ? n : len);
while (n > 0 && i < end) {
rb_ary_push(ary, LONG2FIX(i));
if (i + unit < i) break;
i += unit;
--n;
}
}
else {
if (!x) end -= 1;

len = i - end;
if (len < 0) len = 0;
ary = rb_ary_new_capa((n < len) ? n : len);
while (n > 0 && i > end) {
rb_ary_push(ary, LONG2FIX(i));
if (i + unit > i) break;
i += unit;
--n;
}
}
return ary;
}
else if (RB_FLOAT_TYPE_P(b) || RB_FLOAT_TYPE_P(e) || RB_FLOAT_TYPE_P(s)) {


double unit = NUM2DBL(s);
double beg = NUM2DBL(b);
double end = NIL_P(e) ? (unit < 0 ? -1 : 1)*HUGE_VAL : NUM2DBL(e);
double len = ruby_float_step_size(beg, end, unit, x);
long i;

if (n > len)
n = (long)len;

if (isinf(unit)) {
if (len > 0) {
ary = rb_ary_new_capa(1);
rb_ary_push(ary, DBL2NUM(beg));
}
else {
ary = rb_ary_new_capa(0);
}
}
else if (unit == 0) {
VALUE val = DBL2NUM(beg);
ary = rb_ary_new_capa(n);
for (i = 0; i < len; ++i) {
rb_ary_push(ary, val);
}
}
else {
ary = rb_ary_new_capa(n);
for (i = 0; i < n; ++i) {
double d = i*unit+beg;
if (unit >= 0 ? end < d : d < end) d = end;
rb_ary_push(ary, DBL2NUM(d));
}
}

return ary;
}

return rb_call_super(argc, argv);
}









static VALUE
arith_seq_last(int argc, VALUE *argv, VALUE self)
{
VALUE b, e, s, len_1, len, last, nv, ary;
int last_is_adjusted;
long n;

e = arith_seq_end(self);
if (NIL_P(e)) {
rb_raise(rb_eRangeError,
"cannot get the last element of endless arithmetic sequence");
}

b = arith_seq_begin(self);
s = arith_seq_step(self);

len_1 = rb_int_idiv(rb_int_minus(e, b), s);
if (rb_num_negative_int_p(len_1)) {
if (argc == 0) {
return Qnil;
}
return rb_ary_new_capa(0);
}

last = rb_int_plus(b, rb_int_mul(s, len_1));
if ((last_is_adjusted = arith_seq_exclude_end_p(self) && rb_equal(last, e))) {
last = rb_int_minus(last, s);
}

if (argc == 0) {
return last;
}

if (last_is_adjusted) {
len = len_1;
}
else {
len = rb_int_plus(len_1, INT2FIX(1));
}

rb_scan_args(argc, argv, "1", &nv);
if (!RB_INTEGER_TYPE_P(nv)) {
nv = rb_to_int(nv);
}
if (RTEST(rb_int_gt(nv, len))) {
nv = len;
}
n = NUM2LONG(nv);
if (n < 0) {
rb_raise(rb_eArgError, "negative array size");
}

ary = rb_ary_new_capa(n);
b = rb_int_minus(last, rb_int_mul(s, nv));
while (n) {
b = rb_int_plus(b, s);
rb_ary_push(ary, b);
--n;
}

return ary;
}







static VALUE
arith_seq_inspect(VALUE self)
{
struct enumerator *e;
VALUE eobj, str, eargs;
int range_p;

TypedData_Get_Struct(self, struct enumerator, &enumerator_data_type, e);

eobj = rb_attr_get(self, id_receiver);
if (NIL_P(eobj)) {
eobj = e->obj;
}

range_p = RTEST(rb_obj_is_kind_of(eobj, rb_cRange));
str = rb_sprintf("(%s%"PRIsVALUE"%s.", range_p ? "(" : "", eobj, range_p ? ")" : "");

rb_str_buf_append(str, rb_id2str(e->meth));

eargs = rb_attr_get(eobj, id_arguments);
if (NIL_P(eargs)) {
eargs = e->args;
}
if (eargs != Qfalse) {
long argc = RARRAY_LEN(eargs);
const VALUE *argv = RARRAY_CONST_PTR(eargs); 

if (argc > 0) {
VALUE kwds = Qnil;

rb_str_buf_cat2(str, "(");

if (RB_TYPE_P(argv[argc-1], T_HASH)) {
int all_key = TRUE;
rb_hash_foreach(argv[argc-1], key_symbol_p, (VALUE)&all_key);
if (all_key) kwds = argv[--argc];
}

while (argc--) {
VALUE arg = *argv++;

rb_str_append(str, rb_inspect(arg));
rb_str_buf_cat2(str, ", ");
}
if (!NIL_P(kwds)) {
rb_hash_foreach(kwds, kwd_append, str);
}
rb_str_set_len(str, RSTRING_LEN(str)-2); 
rb_str_buf_cat2(str, ")");
}
}

rb_str_buf_cat2(str, ")");

return str;
}








static VALUE
arith_seq_eq(VALUE self, VALUE other)
{
if (!RTEST(rb_obj_is_kind_of(other, rb_cArithSeq))) {
return Qfalse;
}

if (!rb_equal(arith_seq_begin(self), arith_seq_begin(other))) {
return Qfalse;
}

if (!rb_equal(arith_seq_end(self), arith_seq_end(other))) {
return Qfalse;
}

if (!rb_equal(arith_seq_step(self), arith_seq_step(other))) {
return Qfalse;
}

if (arith_seq_exclude_end_p(self) != arith_seq_exclude_end_p(other)) {
return Qfalse;
}

return Qtrue;
}











static VALUE
arith_seq_hash(VALUE self)
{
st_index_t hash;
VALUE v;

hash = rb_hash_start(arith_seq_exclude_end_p(self));
v = rb_hash(arith_seq_begin(self));
hash = rb_hash_uint(hash, NUM2LONG(v));
v = rb_hash(arith_seq_end(self));
hash = rb_hash_uint(hash, NUM2LONG(v));
v = rb_hash(arith_seq_step(self));
hash = rb_hash_uint(hash, NUM2LONG(v));
hash = rb_hash_end(hash);

return ST2FIX(hash);
}

#define NUM_GE(x, y) RTEST(rb_num_coerce_relop((x), (y), idGE))

struct arith_seq_gen {
VALUE current;
VALUE end;
VALUE step;
int excl;
};






static VALUE
arith_seq_each(VALUE self)
{
VALUE c, e, s, len_1, last;
int x;

if (!rb_block_given_p()) return self;

c = arith_seq_begin(self);
e = arith_seq_end(self);
s = arith_seq_step(self);
x = arith_seq_exclude_end_p(self);

if (!RB_TYPE_P(s, T_COMPLEX) && ruby_float_step(c, e, s, x, TRUE)) {
return self;
}

if (NIL_P(e)) {
while (1) {
rb_yield(c);
c = rb_int_plus(c, s);
}

return self;
}

if (rb_equal(s, INT2FIX(0))) {
while (1) {
rb_yield(c);
}

return self;
}

len_1 = rb_int_idiv(rb_int_minus(e, c), s);
last = rb_int_plus(c, rb_int_mul(s, len_1));
if (x && rb_equal(last, e)) {
last = rb_int_minus(last, s);
}

if (rb_num_negative_int_p(s)) {
while (NUM_GE(c, last)) {
rb_yield(c);
c = rb_int_plus(c, s);
}
}
else {
while (NUM_GE(last, c)) {
rb_yield(c);
c = rb_int_plus(c, s);
}
}

return self;
}








static VALUE
arith_seq_size(VALUE self)
{
VALUE b, e, s, len_1, len, last;
int x;

b = arith_seq_begin(self);
e = arith_seq_end(self);
s = arith_seq_step(self);
x = arith_seq_exclude_end_p(self);

if (RB_FLOAT_TYPE_P(b) || RB_FLOAT_TYPE_P(e) || RB_FLOAT_TYPE_P(s)) {
double ee, n;

if (NIL_P(e)) {
if (rb_num_negative_int_p(s)) {
ee = -HUGE_VAL;
}
else {
ee = HUGE_VAL;
}
}
else {
ee = NUM2DBL(e);
}

n = ruby_float_step_size(NUM2DBL(b), ee, NUM2DBL(s), x);
if (isinf(n)) return DBL2NUM(n);
if (POSFIXABLE(n)) return LONG2FIX(n);
return rb_dbl2big(n);
}

if (NIL_P(e)) {
return DBL2NUM(HUGE_VAL);
}

if (!rb_obj_is_kind_of(s, rb_cNumeric)) {
s = rb_to_int(s);
}

if (rb_equal(s, INT2FIX(0))) {
return DBL2NUM(HUGE_VAL);
}

len_1 = rb_int_idiv(rb_int_minus(e, b), s);
if (rb_num_negative_int_p(len_1)) {
return INT2FIX(0);
}

last = rb_int_plus(b, rb_int_mul(s, len_1));
if (x && rb_equal(last, e)) {
len = len_1;
}
else {
len = rb_int_plus(len_1, INT2FIX(1));
}

return len;
}

void
InitVM_Enumerator(void)
{
ID id_private = rb_intern("private");

rb_define_method(rb_mKernel, "to_enum", obj_to_enum, -1);
rb_define_method(rb_mKernel, "enum_for", obj_to_enum, -1);

rb_cEnumerator = rb_define_class("Enumerator", rb_cObject);
rb_include_module(rb_cEnumerator, rb_mEnumerable);

rb_define_alloc_func(rb_cEnumerator, enumerator_allocate);
rb_define_method(rb_cEnumerator, "initialize", enumerator_initialize, -1);
rb_define_method(rb_cEnumerator, "initialize_copy", enumerator_init_copy, 1);
rb_define_method(rb_cEnumerator, "each", enumerator_each, -1);
rb_define_method(rb_cEnumerator, "each_with_index", enumerator_each_with_index, 0);
rb_define_method(rb_cEnumerator, "each_with_object", enumerator_with_object, 1);
rb_define_method(rb_cEnumerator, "with_index", enumerator_with_index, -1);
rb_define_method(rb_cEnumerator, "with_object", enumerator_with_object, 1);
rb_define_method(rb_cEnumerator, "next_values", enumerator_next_values, 0);
rb_define_method(rb_cEnumerator, "peek_values", enumerator_peek_values_m, 0);
rb_define_method(rb_cEnumerator, "next", enumerator_next, 0);
rb_define_method(rb_cEnumerator, "peek", enumerator_peek, 0);
rb_define_method(rb_cEnumerator, "feed", enumerator_feed, 1);
rb_define_method(rb_cEnumerator, "rewind", enumerator_rewind, 0);
rb_define_method(rb_cEnumerator, "inspect", enumerator_inspect, 0);
rb_define_method(rb_cEnumerator, "size", enumerator_size, 0);
rb_define_method(rb_cEnumerator, "+", enumerator_plus, 1);
rb_define_method(rb_mEnumerable, "chain", enum_chain, -1);


rb_cLazy = rb_define_class_under(rb_cEnumerator, "Lazy", rb_cEnumerator);
rb_define_method(rb_mEnumerable, "lazy", enumerable_lazy, 0);

rb_define_alias(rb_cLazy, "_enumerable_map", "map");
rb_define_alias(rb_cLazy, "_enumerable_collect", "collect");
rb_define_alias(rb_cLazy, "_enumerable_flat_map", "flat_map");
rb_define_alias(rb_cLazy, "_enumerable_collect_concat", "collect_concat");
rb_define_alias(rb_cLazy, "_enumerable_select", "select");
rb_define_alias(rb_cLazy, "_enumerable_find_all", "find_all");
rb_define_alias(rb_cLazy, "_enumerable_filter", "filter");
rb_define_alias(rb_cLazy, "_enumerable_filter_map", "filter_map");
rb_define_alias(rb_cLazy, "_enumerable_reject", "reject");
rb_define_alias(rb_cLazy, "_enumerable_grep", "grep");
rb_define_alias(rb_cLazy, "_enumerable_grep_v", "grep_v");
rb_define_alias(rb_cLazy, "_enumerable_zip", "zip");
rb_define_alias(rb_cLazy, "_enumerable_take", "take");
rb_define_alias(rb_cLazy, "_enumerable_take_while", "take_while");
rb_define_alias(rb_cLazy, "_enumerable_drop", "drop");
rb_define_alias(rb_cLazy, "_enumerable_drop_while", "drop_while");
rb_define_alias(rb_cLazy, "_enumerable_uniq", "uniq");
rb_define_private_method(rb_cLazy, "_enumerable_with_index", enumerator_with_index, -1);

rb_funcall(rb_cLazy, id_private, 1, ID2SYM(rb_intern("_enumerable_map")));
rb_funcall(rb_cLazy, id_private, 1, ID2SYM(rb_intern("_enumerable_collect")));
rb_funcall(rb_cLazy, id_private, 1, ID2SYM(rb_intern("_enumerable_flat_map")));
rb_funcall(rb_cLazy, id_private, 1, ID2SYM(rb_intern("_enumerable_collect_concat")));
rb_funcall(rb_cLazy, id_private, 1, ID2SYM(rb_intern("_enumerable_select")));
rb_funcall(rb_cLazy, id_private, 1, ID2SYM(rb_intern("_enumerable_find_all")));
rb_funcall(rb_cLazy, id_private, 1, ID2SYM(rb_intern("_enumerable_filter")));
rb_funcall(rb_cLazy, id_private, 1, ID2SYM(rb_intern("_enumerable_filter_map")));
rb_funcall(rb_cLazy, id_private, 1, ID2SYM(rb_intern("_enumerable_reject")));
rb_funcall(rb_cLazy, id_private, 1, ID2SYM(rb_intern("_enumerable_grep")));
rb_funcall(rb_cLazy, id_private, 1, ID2SYM(rb_intern("_enumerable_grep_v")));
rb_funcall(rb_cLazy, id_private, 1, ID2SYM(rb_intern("_enumerable_zip")));
rb_funcall(rb_cLazy, id_private, 1, ID2SYM(rb_intern("_enumerable_take")));
rb_funcall(rb_cLazy, id_private, 1, ID2SYM(rb_intern("_enumerable_take_while")));
rb_funcall(rb_cLazy, id_private, 1, ID2SYM(rb_intern("_enumerable_drop")));
rb_funcall(rb_cLazy, id_private, 1, ID2SYM(rb_intern("_enumerable_drop_while")));
rb_funcall(rb_cLazy, id_private, 1, ID2SYM(rb_intern("_enumerable_uniq")));

rb_define_method(rb_cLazy, "initialize", lazy_initialize, -1);
rb_define_method(rb_cLazy, "to_enum", lazy_to_enum, -1);
rb_define_method(rb_cLazy, "enum_for", lazy_to_enum, -1);
rb_define_method(rb_cLazy, "eager", lazy_eager, 0);
rb_define_method(rb_cLazy, "map", lazy_map, 0);
rb_define_method(rb_cLazy, "collect", lazy_map, 0);
rb_define_method(rb_cLazy, "flat_map", lazy_flat_map, 0);
rb_define_method(rb_cLazy, "collect_concat", lazy_flat_map, 0);
rb_define_method(rb_cLazy, "select", lazy_select, 0);
rb_define_method(rb_cLazy, "find_all", lazy_select, 0);
rb_define_method(rb_cLazy, "filter", lazy_select, 0);
rb_define_method(rb_cLazy, "filter_map", lazy_filter_map, 0);
rb_define_method(rb_cLazy, "reject", lazy_reject, 0);
rb_define_method(rb_cLazy, "grep", lazy_grep, 1);
rb_define_method(rb_cLazy, "grep_v", lazy_grep_v, 1);
rb_define_method(rb_cLazy, "zip", lazy_zip, -1);
rb_define_method(rb_cLazy, "take", lazy_take, 1);
rb_define_method(rb_cLazy, "take_while", lazy_take_while, 0);
rb_define_method(rb_cLazy, "drop", lazy_drop, 1);
rb_define_method(rb_cLazy, "drop_while", lazy_drop_while, 0);
rb_define_method(rb_cLazy, "lazy", lazy_lazy, 0);
rb_define_method(rb_cLazy, "chunk", lazy_super, -1);
rb_define_method(rb_cLazy, "slice_before", lazy_super, -1);
rb_define_method(rb_cLazy, "slice_after", lazy_super, -1);
rb_define_method(rb_cLazy, "slice_when", lazy_super, -1);
rb_define_method(rb_cLazy, "chunk_while", lazy_super, -1);
rb_define_method(rb_cLazy, "uniq", lazy_uniq, 0);
rb_define_method(rb_cLazy, "with_index", lazy_with_index, -1);

lazy_use_super_method = rb_hash_new_with_size(18);
rb_hash_aset(lazy_use_super_method, ID2SYM(rb_intern("map")), ID2SYM(rb_intern("_enumerable_map")));
rb_hash_aset(lazy_use_super_method, ID2SYM(rb_intern("collect")), ID2SYM(rb_intern("_enumerable_collect")));
rb_hash_aset(lazy_use_super_method, ID2SYM(rb_intern("flat_map")), ID2SYM(rb_intern("_enumerable_flat_map")));
rb_hash_aset(lazy_use_super_method, ID2SYM(rb_intern("collect_concat")), ID2SYM(rb_intern("_enumerable_collect_concat")));
rb_hash_aset(lazy_use_super_method, ID2SYM(rb_intern("select")), ID2SYM(rb_intern("_enumerable_select")));
rb_hash_aset(lazy_use_super_method, ID2SYM(rb_intern("find_all")), ID2SYM(rb_intern("_enumerable_find_all")));
rb_hash_aset(lazy_use_super_method, ID2SYM(rb_intern("filter")), ID2SYM(rb_intern("_enumerable_filter")));
rb_hash_aset(lazy_use_super_method, ID2SYM(rb_intern("filter_map")), ID2SYM(rb_intern("_enumerable_filter_map")));
rb_hash_aset(lazy_use_super_method, ID2SYM(rb_intern("reject")), ID2SYM(rb_intern("_enumerable_reject")));
rb_hash_aset(lazy_use_super_method, ID2SYM(rb_intern("grep")), ID2SYM(rb_intern("_enumerable_grep")));
rb_hash_aset(lazy_use_super_method, ID2SYM(rb_intern("grep_v")), ID2SYM(rb_intern("_enumerable_grep_v")));
rb_hash_aset(lazy_use_super_method, ID2SYM(rb_intern("zip")), ID2SYM(rb_intern("_enumerable_zip")));
rb_hash_aset(lazy_use_super_method, ID2SYM(rb_intern("take")), ID2SYM(rb_intern("_enumerable_take")));
rb_hash_aset(lazy_use_super_method, ID2SYM(rb_intern("take_while")), ID2SYM(rb_intern("_enumerable_take_while")));
rb_hash_aset(lazy_use_super_method, ID2SYM(rb_intern("drop")), ID2SYM(rb_intern("_enumerable_drop")));
rb_hash_aset(lazy_use_super_method, ID2SYM(rb_intern("drop_while")), ID2SYM(rb_intern("_enumerable_drop_while")));
rb_hash_aset(lazy_use_super_method, ID2SYM(rb_intern("uniq")), ID2SYM(rb_intern("_enumerable_uniq")));
rb_hash_aset(lazy_use_super_method, ID2SYM(rb_intern("with_index")), ID2SYM(rb_intern("_enumerable_with_index")));
rb_obj_freeze(lazy_use_super_method);
rb_gc_register_mark_object(lazy_use_super_method);

#if 0 
rb_define_method(rb_cLazy, "to_a", lazy_to_a, 0);
rb_define_method(rb_cLazy, "chunk", lazy_chunk, 0);
rb_define_method(rb_cLazy, "chunk_while", lazy_chunk_while, 0);
rb_define_method(rb_cLazy, "slice_after", lazy_slice_after, 0);
rb_define_method(rb_cLazy, "slice_before", lazy_slice_before, 0);
rb_define_method(rb_cLazy, "slice_when", lazy_slice_when, 0);
#endif
rb_define_alias(rb_cLazy, "force", "to_a");

rb_eStopIteration = rb_define_class("StopIteration", rb_eIndexError);
rb_define_method(rb_eStopIteration, "result", stop_result, 0);


rb_cGenerator = rb_define_class_under(rb_cEnumerator, "Generator", rb_cObject);
rb_include_module(rb_cGenerator, rb_mEnumerable);
rb_define_alloc_func(rb_cGenerator, generator_allocate);
rb_define_method(rb_cGenerator, "initialize", generator_initialize, -1);
rb_define_method(rb_cGenerator, "initialize_copy", generator_init_copy, 1);
rb_define_method(rb_cGenerator, "each", generator_each, -1);


rb_cYielder = rb_define_class_under(rb_cEnumerator, "Yielder", rb_cObject);
rb_define_alloc_func(rb_cYielder, yielder_allocate);
rb_define_method(rb_cYielder, "initialize", yielder_initialize, 0);
rb_define_method(rb_cYielder, "yield", yielder_yield, -2);
rb_define_method(rb_cYielder, "<<", yielder_yield_push, 1);
rb_define_method(rb_cYielder, "to_proc", yielder_to_proc, 0);


rb_cEnumProducer = rb_define_class_under(rb_cEnumerator, "Producer", rb_cObject);
rb_define_alloc_func(rb_cEnumProducer, producer_allocate);
rb_define_method(rb_cEnumProducer, "each", producer_each, 0);
rb_define_singleton_method(rb_cEnumerator, "produce", enumerator_s_produce, -1);


rb_cEnumChain = rb_define_class_under(rb_cEnumerator, "Chain", rb_cEnumerator);
rb_define_alloc_func(rb_cEnumChain, enum_chain_allocate);
rb_define_method(rb_cEnumChain, "initialize", enum_chain_initialize, -2);
rb_define_method(rb_cEnumChain, "initialize_copy", enum_chain_init_copy, 1);
rb_define_method(rb_cEnumChain, "each", enum_chain_each, -1);
rb_define_method(rb_cEnumChain, "size", enum_chain_size, 0);
rb_define_method(rb_cEnumChain, "rewind", enum_chain_rewind, 0);
rb_define_method(rb_cEnumChain, "inspect", enum_chain_inspect, 0);


rb_cArithSeq = rb_define_class_under(rb_cEnumerator, "ArithmeticSequence", rb_cEnumerator);
rb_undef_alloc_func(rb_cArithSeq);
rb_undef_method(CLASS_OF(rb_cArithSeq), "new");
rb_define_method(rb_cArithSeq, "begin", arith_seq_begin, 0);
rb_define_method(rb_cArithSeq, "end", arith_seq_end, 0);
rb_define_method(rb_cArithSeq, "exclude_end?", arith_seq_exclude_end, 0);
rb_define_method(rb_cArithSeq, "step", arith_seq_step, 0);
rb_define_method(rb_cArithSeq, "first", arith_seq_first, -1);
rb_define_method(rb_cArithSeq, "last", arith_seq_last, -1);
rb_define_method(rb_cArithSeq, "inspect", arith_seq_inspect, 0);
rb_define_method(rb_cArithSeq, "==", arith_seq_eq, 1);
rb_define_method(rb_cArithSeq, "===", arith_seq_eq, 1);
rb_define_method(rb_cArithSeq, "eql?", arith_seq_eq, 1);
rb_define_method(rb_cArithSeq, "hash", arith_seq_hash, 0);
rb_define_method(rb_cArithSeq, "each", arith_seq_each, 0);
rb_define_method(rb_cArithSeq, "size", arith_seq_size, 0);

rb_provide("enumerator.so"); 
}

#undef rb_intern
void
Init_Enumerator(void)
{
id_rewind = rb_intern("rewind");
id_new = rb_intern("new");
id_next = rb_intern("next");
id_result = rb_intern("result");
id_receiver = rb_intern("receiver");
id_arguments = rb_intern("arguments");
id_memo = rb_intern("memo");
id_method = rb_intern("method");
id_force = rb_intern("force");
id_to_enum = rb_intern("to_enum");
id_begin = rb_intern("begin");
id_end = rb_intern("end");
id_step = rb_intern("step");
id_exclude_end = rb_intern("exclude_end");
sym_each = ID2SYM(id_each);
sym_cycle = ID2SYM(rb_intern("cycle"));
sym_yield = ID2SYM(rb_intern("yield"));

InitVM(Enumerator);
}
