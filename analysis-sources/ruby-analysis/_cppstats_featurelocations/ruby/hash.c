












#include "ruby/config.h"

#include <errno.h>

#if defined(__APPLE__)
#if defined(HAVE_CRT_EXTERNS_H)
#include <crt_externs.h>
#else
#include "missing/crt_externs.h"
#endif
#endif

#include "debug_counter.h"
#include "id.h"
#include "internal.h"
#include "internal/array.h"
#include "internal/bignum.h"
#include "internal/class.h"
#include "internal/cont.h"
#include "internal/error.h"
#include "internal/hash.h"
#include "internal/object.h"
#include "internal/proc.h"
#include "internal/symbol.h"
#include "internal/time.h"
#include "internal/vm.h"
#include "probes.h"
#include "ruby/st.h"
#include "ruby/util.h"
#include "ruby_assert.h"
#include "symbol.h"
#include "transient_heap.h"

#if !defined(HASH_DEBUG)
#define HASH_DEBUG 0
#endif

#if HASH_DEBUG
#include "gc.h"
#endif

#define HAS_EXTRA_STATES(hash, klass) ( ((klass = has_extra_methods(rb_obj_class(hash))) != 0) || FL_TEST((hash), FL_EXIVAR|RHASH_PROC_DEFAULT) || !NIL_P(RHASH_IFNONE(hash)))




#define SET_DEFAULT(hash, ifnone) ( FL_UNSET_RAW(hash, RHASH_PROC_DEFAULT), RHASH_SET_IFNONE(hash, ifnone))



#define SET_PROC_DEFAULT(hash, proc) set_proc_default(hash, proc)

#define COPY_DEFAULT(hash, hash2) copy_default(RHASH(hash), RHASH(hash2))

static inline void
copy_default(struct RHash *hash, const struct RHash *hash2)
{
hash->basic.flags &= ~RHASH_PROC_DEFAULT;
hash->basic.flags |= hash2->basic.flags & RHASH_PROC_DEFAULT;
RHASH_SET_IFNONE(hash, RHASH_IFNONE((VALUE)hash2));
}

static VALUE
has_extra_methods(VALUE klass)
{
const VALUE base = rb_cHash;
VALUE c = klass;
while (c != base) {
if (rb_class_has_methods(c)) return klass;
c = RCLASS_SUPER(c);
}
return 0;
}

static VALUE rb_hash_s_try_convert(VALUE, VALUE);











VALUE
rb_hash_freeze(VALUE hash)
{
return rb_obj_freeze(hash);
}

VALUE rb_cHash;

static VALUE envtbl;
static ID id_hash, id_default, id_flatten_bang;
static ID id_hash_iter_lev;

VALUE
rb_hash_set_ifnone(VALUE hash, VALUE ifnone)
{
RB_OBJ_WRITE(hash, (&RHASH(hash)->ifnone), ifnone);
return hash;
}

static int
rb_any_cmp(VALUE a, VALUE b)
{
if (a == b) return 0;
if (RB_TYPE_P(a, T_STRING) && RBASIC(a)->klass == rb_cString &&
RB_TYPE_P(b, T_STRING) && RBASIC(b)->klass == rb_cString) {
return rb_str_hash_cmp(a, b);
}
if (a == Qundef || b == Qundef) return -1;
if (SYMBOL_P(a) && SYMBOL_P(b)) {
return a != b;
}

return !rb_eql(a, b);
}

static VALUE
hash_recursive(VALUE obj, VALUE arg, int recurse)
{
if (recurse) return INT2FIX(0);
return rb_funcallv(obj, id_hash, 0, 0);
}

VALUE
rb_hash(VALUE obj)
{
VALUE hval = rb_exec_recursive_outer(hash_recursive, obj, 0);

while (!FIXNUM_P(hval)) {
if (RB_TYPE_P(hval, T_BIGNUM)) {
int sign;
unsigned long ul;
sign = rb_integer_pack(hval, &ul, 1, sizeof(ul), 0,
INTEGER_PACK_NATIVE_BYTE_ORDER);
ul &= (1UL << (sizeof(long)*CHAR_BIT-1)) - 1;
if (sign < 0)
return LONG2FIX(-(long)ul);
return LONG2FIX((long)ul);
}
hval = rb_to_int(hval);
}
return hval;
}

static long rb_objid_hash(st_index_t index);

static st_index_t
dbl_to_index(double d)
{
union {double d; st_index_t i;} u;
u.d = d;
return u.i;
}

long
rb_dbl_long_hash(double d)
{

if (d == 0.0) d = 0.0;
#if SIZEOF_INT == SIZEOF_VOIDP
return rb_memhash(&d, sizeof(d));
#else
return rb_objid_hash(dbl_to_index(d));
#endif
}

static inline long
any_hash(VALUE a, st_index_t (*other_func)(VALUE))
{
VALUE hval;
st_index_t hnum;

if (SPECIAL_CONST_P(a)) {
if (STATIC_SYM_P(a)) {
hnum = a >> (RUBY_SPECIAL_SHIFT + ID_SCOPE_SHIFT);
hnum = rb_hash_start(hnum);
goto out;
}
else if (FLONUM_P(a)) {

goto flt;
}
hnum = rb_objid_hash((st_index_t)a);
}
else if (BUILTIN_TYPE(a) == T_STRING) {
hnum = rb_str_hash(a);
}
else if (BUILTIN_TYPE(a) == T_SYMBOL) {
hnum = RSYMBOL(a)->hashval;
}
else if (BUILTIN_TYPE(a) == T_BIGNUM) {
hval = rb_big_hash(a);
hnum = FIX2LONG(hval);
}
else if (BUILTIN_TYPE(a) == T_FLOAT) {
flt:
hnum = rb_dbl_long_hash(rb_float_value(a));
}
else {
hnum = other_func(a);
}
out:
#if SIZEOF_LONG < SIZEOF_ST_INDEX_T
if (hnum > 0)
hnum &= (unsigned long)-1 >> 2;
else
hnum |= ~((unsigned long)-1 >> 2);
#else
hnum <<= 1;
hnum = RSHIFT(hnum, 1);
#endif
return (long)hnum;
}

static st_index_t
obj_any_hash(VALUE obj)
{
obj = rb_hash(obj);
return FIX2LONG(obj);
}

static st_index_t
rb_any_hash(VALUE a)
{
return any_hash(a, obj_any_hash);
}






static const uint64_t prime1 = ((uint64_t)0x2e0bb864 << 32) | 0xe9ea7df5;
static const uint32_t prime2 = 0x830fcab9;


static inline uint64_t
mult_and_mix(uint64_t m1, uint64_t m2)
{
#if defined HAVE_UINT128_T
uint128_t r = (uint128_t) m1 * (uint128_t) m2;
return (uint64_t) (r >> 64) ^ (uint64_t) r;
#else
uint64_t hm1 = m1 >> 32, hm2 = m2 >> 32;
uint64_t lm1 = m1, lm2 = m2;
uint64_t v64_128 = hm1 * hm2;
uint64_t v32_96 = hm1 * lm2 + lm1 * hm2;
uint64_t v1_32 = lm1 * lm2;

return (v64_128 + (v32_96 >> 32)) ^ ((v32_96 << 32) + v1_32);
#endif
}

static inline uint64_t
key64_hash(uint64_t key, uint32_t seed)
{
return mult_and_mix(key + seed, prime1);
}


#define st_index_hash(index) key64_hash(rb_hash_start(index), prime2)

static long
rb_objid_hash(st_index_t index)
{
return (long)st_index_hash(index);
}

static st_index_t
objid_hash(VALUE obj)
{
VALUE object_id = rb_obj_id(obj);
if (!FIXNUM_P(object_id))
object_id = rb_big_hash(object_id);

#if SIZEOF_LONG == SIZEOF_VOIDP
return (st_index_t)st_index_hash((st_index_t)NUM2LONG(object_id));
#elif SIZEOF_LONG_LONG == SIZEOF_VOIDP
return (st_index_t)st_index_hash((st_index_t)NUM2LL(object_id));
#endif
}




















VALUE
rb_obj_hash(VALUE obj)
{
long hnum = any_hash(obj, objid_hash);
return ST2FIX(hnum);
}

static const struct st_hash_type objhash = {
rb_any_cmp,
rb_any_hash,
};

#define rb_ident_cmp st_numcmp

static st_index_t
rb_ident_hash(st_data_t n)
{
#if defined(USE_FLONUM)





if (FLONUM_P(n)) {
n ^= dbl_to_index(rb_float_value(n));
}
#endif

return (st_index_t)st_index_hash((st_index_t)n);
}

#define identhash rb_hashtype_ident
const struct st_hash_type rb_hashtype_ident = {
rb_ident_cmp,
rb_ident_hash,
};

typedef st_index_t st_hash_t;











#define RHASH_AR_TABLE_MAX_BOUND RHASH_AR_TABLE_MAX_SIZE

#define RHASH_AR_TABLE_REF(hash, n) (&RHASH_AR_TABLE(hash)->pairs[n])
#define RHASH_AR_CLEARED_HINT 0xff

typedef struct ar_table_pair_struct {
VALUE key;
VALUE val;
} ar_table_pair;

typedef struct ar_table_struct {

ar_table_pair pairs[RHASH_AR_TABLE_MAX_SIZE];
} ar_table;

size_t
rb_hash_ar_table_size(void)
{
return sizeof(ar_table);
}

static inline st_hash_t
ar_do_hash(st_data_t key)
{
return (st_hash_t)rb_any_hash(key);
}

static inline ar_hint_t
ar_do_hash_hint(st_hash_t hash_value)
{
return (ar_hint_t)hash_value;
}

static inline ar_hint_t
ar_hint(VALUE hash, unsigned int index)
{
return RHASH(hash)->ar_hint.ary[index];
}

static inline void
ar_hint_set_hint(VALUE hash, unsigned int index, ar_hint_t hint)
{
RHASH(hash)->ar_hint.ary[index] = hint;
}

static inline void
ar_hint_set(VALUE hash, unsigned int index, st_hash_t hash_value)
{
ar_hint_set_hint(hash, index, ar_do_hash_hint(hash_value));
}

static inline void
ar_clear_entry(VALUE hash, unsigned int index)
{
ar_table_pair *pair = RHASH_AR_TABLE_REF(hash, index);
pair->key = Qundef;
ar_hint_set_hint(hash, index, RHASH_AR_CLEARED_HINT);
}

static inline int
ar_cleared_entry(VALUE hash, unsigned int index)
{
if (ar_hint(hash, index) == RHASH_AR_CLEARED_HINT) {



ar_table_pair *pair = RHASH_AR_TABLE_REF(hash, index);
return pair->key == Qundef;
}
else {
return FALSE;
}
}

static inline void
ar_set_entry(VALUE hash, unsigned int index, st_data_t key, st_data_t val, st_hash_t hash_value)
{
ar_table_pair *pair = RHASH_AR_TABLE_REF(hash, index);
pair->key = key;
pair->val = val;
ar_hint_set(hash, index, hash_value);
}

#define RHASH_AR_TABLE_SIZE(h) (HASH_ASSERT(RHASH_AR_TABLE_P(h)), RHASH_AR_TABLE_SIZE_RAW(h))


#define RHASH_AR_TABLE_BOUND_RAW(h) ((unsigned int)((RBASIC(h)->flags >> RHASH_AR_TABLE_BOUND_SHIFT) & (RHASH_AR_TABLE_BOUND_MASK >> RHASH_AR_TABLE_BOUND_SHIFT)))



#define RHASH_AR_TABLE_BOUND(h) (HASH_ASSERT(RHASH_AR_TABLE_P(h)), RHASH_AR_TABLE_BOUND_RAW(h))


#define RHASH_ST_TABLE_SET(h, s) rb_hash_st_table_set(h, s)
#define RHASH_TYPE(hash) (RHASH_AR_TABLE_P(hash) ? &objhash : RHASH_ST_TABLE(hash)->type)

#define HASH_ASSERT(expr) RUBY_ASSERT_MESG_WHEN(HASH_DEBUG, expr, #expr)

#if HASH_DEBUG
#define hash_verify(hash) hash_verify_(hash, __FILE__, __LINE__)

void
rb_hash_dump(VALUE hash)
{
rb_obj_info_dump(hash);

if (RHASH_AR_TABLE_P(hash)) {
unsigned i, n = 0, bound = RHASH_AR_TABLE_BOUND(hash);

fprintf(stderr, " size:%u bound:%u\n",
RHASH_AR_TABLE_SIZE(hash), RHASH_AR_TABLE_BOUND(hash));

for (i=0; i<bound; i++) {
st_data_t k, v;

if (!ar_cleared_entry(hash, i)) {
char b1[0x100], b2[0x100];
ar_table_pair *pair = RHASH_AR_TABLE_REF(hash, i);
k = pair->key;
v = pair->val;
fprintf(stderr, " %d key:%s val:%s hint:%02x\n", i,
rb_raw_obj_info(b1, 0x100, k),
rb_raw_obj_info(b2, 0x100, v),
ar_hint(hash, i));
n++;
}
else {
fprintf(stderr, " %d empty\n", i);
}
}
}
}

static VALUE
hash_verify_(VALUE hash, const char *file, int line)
{
HASH_ASSERT(RB_TYPE_P(hash, T_HASH));

if (RHASH_AR_TABLE_P(hash)) {
unsigned i, n = 0, bound = RHASH_AR_TABLE_BOUND(hash);

for (i=0; i<bound; i++) {
st_data_t k, v;
if (!ar_cleared_entry(hash, i)) {
ar_table_pair *pair = RHASH_AR_TABLE_REF(hash, i);
k = pair->key;
v = pair->val;
HASH_ASSERT(k != Qundef);
HASH_ASSERT(v != Qundef);
n++;
}
}
if (n != RHASH_AR_TABLE_SIZE(hash)) {
rb_bug("n:%u, RHASH_AR_TABLE_SIZE:%u", n, RHASH_AR_TABLE_SIZE(hash));
}
}
else {
HASH_ASSERT(RHASH_ST_TABLE(hash) != NULL);
HASH_ASSERT(RHASH_AR_TABLE_SIZE_RAW(hash) == 0);
HASH_ASSERT(RHASH_AR_TABLE_BOUND_RAW(hash) == 0);
}

#if USE_TRANSIENT_HEP
if (RHASH_TRANSIENT_P(hash)) {
volatile st_data_t MAYBE_UNUSED(key) = RHASH_AR_TABLE_REF(hash, 0)->key; 
HASH_ASSERT(RHASH_AR_TABLE(hash) != NULL);
HASH_ASSERT(rb_transient_heap_managed_ptr_p(RHASH_AR_TABLE(hash)));
}
#endif
return hash;
}

#else
#define hash_verify(h) ((void)0)
#endif

static inline int
RHASH_TABLE_NULL_P(VALUE hash)
{
if (RHASH(hash)->as.ar == NULL) {
HASH_ASSERT(RHASH_AR_TABLE_P(hash));
return TRUE;
}
else {
return FALSE;
}
}

static inline int
RHASH_TABLE_EMPTY_P(VALUE hash)
{
return RHASH_SIZE(hash) == 0;
}

int
rb_hash_ar_table_p(VALUE hash)
{
if (FL_TEST_RAW((hash), RHASH_ST_TABLE_FLAG)) {
HASH_ASSERT(RHASH(hash)->as.st != NULL);
return FALSE;
}
else {
return TRUE;
}
}

ar_table *
rb_hash_ar_table(VALUE hash)
{
HASH_ASSERT(RHASH_AR_TABLE_P(hash));
return RHASH(hash)->as.ar;
}

st_table *
rb_hash_st_table(VALUE hash)
{
HASH_ASSERT(!RHASH_AR_TABLE_P(hash));
return RHASH(hash)->as.st;
}

void
rb_hash_st_table_set(VALUE hash, st_table *st)
{
HASH_ASSERT(st != NULL);
FL_SET_RAW((hash), RHASH_ST_TABLE_FLAG);
RHASH(hash)->as.st = st;
}

static void
hash_ar_table_set(VALUE hash, ar_table *ar)
{
HASH_ASSERT(RHASH_AR_TABLE_P(hash));
HASH_ASSERT((RHASH_TRANSIENT_P(hash) && ar == NULL) ? FALSE : TRUE);
RHASH(hash)->as.ar = ar;
hash_verify(hash);
}

#define RHASH_SET_ST_FLAG(h) FL_SET_RAW(h, RHASH_ST_TABLE_FLAG)
#define RHASH_UNSET_ST_FLAG(h) FL_UNSET_RAW(h, RHASH_ST_TABLE_FLAG)

static inline void
RHASH_AR_TABLE_BOUND_SET(VALUE h, st_index_t n)
{
HASH_ASSERT(RHASH_AR_TABLE_P(h));
HASH_ASSERT(n <= RHASH_AR_TABLE_MAX_BOUND);

RBASIC(h)->flags &= ~RHASH_AR_TABLE_BOUND_MASK;
RBASIC(h)->flags |= n << RHASH_AR_TABLE_BOUND_SHIFT;
}

static inline void
RHASH_AR_TABLE_SIZE_SET(VALUE h, st_index_t n)
{
HASH_ASSERT(RHASH_AR_TABLE_P(h));
HASH_ASSERT(n <= RHASH_AR_TABLE_MAX_SIZE);

RBASIC(h)->flags &= ~RHASH_AR_TABLE_SIZE_MASK;
RBASIC(h)->flags |= n << RHASH_AR_TABLE_SIZE_SHIFT;
}

static inline void
HASH_AR_TABLE_SIZE_ADD(VALUE h, st_index_t n)
{
HASH_ASSERT(RHASH_AR_TABLE_P(h));

RHASH_AR_TABLE_SIZE_SET(h, RHASH_AR_TABLE_SIZE(h) + n);

hash_verify(h);
}

#define RHASH_AR_TABLE_SIZE_INC(h) HASH_AR_TABLE_SIZE_ADD(h, 1)

static inline void
RHASH_AR_TABLE_SIZE_DEC(VALUE h)
{
HASH_ASSERT(RHASH_AR_TABLE_P(h));
int new_size = RHASH_AR_TABLE_SIZE(h) - 1;

if (new_size != 0) {
RHASH_AR_TABLE_SIZE_SET(h, new_size);
}
else {
RHASH_AR_TABLE_SIZE_SET(h, 0);
RHASH_AR_TABLE_BOUND_SET(h, 0);
}
hash_verify(h);
}

static inline void
RHASH_AR_TABLE_CLEAR(VALUE h)
{
RBASIC(h)->flags &= ~RHASH_AR_TABLE_SIZE_MASK;
RBASIC(h)->flags &= ~RHASH_AR_TABLE_BOUND_MASK;

hash_ar_table_set(h, NULL);
}

static ar_table*
ar_alloc_table(VALUE hash)
{
ar_table *tab = (ar_table*)rb_transient_heap_alloc(hash, sizeof(ar_table));

if (tab != NULL) {
RHASH_SET_TRANSIENT_FLAG(hash);
}
else {
RHASH_UNSET_TRANSIENT_FLAG(hash);
tab = (ar_table*)ruby_xmalloc(sizeof(ar_table));
}

RHASH_AR_TABLE_SIZE_SET(hash, 0);
RHASH_AR_TABLE_BOUND_SET(hash, 0);
hash_ar_table_set(hash, tab);

return tab;
}

NOINLINE(static int ar_equal(VALUE x, VALUE y));

static int
ar_equal(VALUE x, VALUE y)
{
return rb_any_cmp(x, y) == 0;
}

static unsigned
ar_find_entry_hint(VALUE hash, ar_hint_t hint, st_data_t key)
{
unsigned i, bound = RHASH_AR_TABLE_BOUND(hash);
const ar_hint_t *hints = RHASH(hash)->ar_hint.ary;



for (i = 0; i < bound; i++) {
if (hints[i] == hint) {
ar_table_pair *pair = RHASH_AR_TABLE_REF(hash, i);
if (ar_equal(key, pair->key)) {
RB_DEBUG_COUNTER_INC(artable_hint_hit);
return i;
}
else {
#if 0
static int pid;
static char fname[256];
static FILE *fp;

if (pid != getpid()) {
snprintf(fname, sizeof(fname), "/tmp/ruby-armiss.%d", pid = getpid());
if ((fp = fopen(fname, "w")) == NULL) rb_bug("fopen");
}

st_hash_t h1 = ar_do_hash(key);
st_hash_t h2 = ar_do_hash(pair->key);

fprintf(fp, "miss: hash_eq:%d hints[%d]:%02x hint:%02x\n"
" key :%016lx %s\n"
" pair->key:%016lx %s\n",
h1 == h2, i, hints[i], hint,
h1, rb_obj_info(key), h2, rb_obj_info(pair->key));
#endif
RB_DEBUG_COUNTER_INC(artable_hint_miss);
}
}
}
RB_DEBUG_COUNTER_INC(artable_hint_notfound);
return RHASH_AR_TABLE_MAX_BOUND;
}

static unsigned
ar_find_entry(VALUE hash, st_hash_t hash_value, st_data_t key)
{
ar_hint_t hint = ar_do_hash_hint(hash_value);
return ar_find_entry_hint(hash, hint, key);
}

static inline void
ar_free_and_clear_table(VALUE hash)
{
ar_table *tab = RHASH_AR_TABLE(hash);

if (tab) {
if (RHASH_TRANSIENT_P(hash)) {
RHASH_UNSET_TRANSIENT_FLAG(hash);
}
else {
ruby_xfree(RHASH_AR_TABLE(hash));
}
RHASH_AR_TABLE_CLEAR(hash);
}
HASH_ASSERT(RHASH_AR_TABLE_SIZE(hash) == 0);
HASH_ASSERT(RHASH_AR_TABLE_BOUND(hash) == 0);
HASH_ASSERT(RHASH_TRANSIENT_P(hash) == 0);
}

static void
ar_try_convert_table(VALUE hash)
{
if (!RHASH_AR_TABLE_P(hash)) return;

const unsigned size = RHASH_AR_TABLE_SIZE(hash);

st_table *new_tab;
st_index_t i;

if (size < RHASH_AR_TABLE_MAX_SIZE) {
return;
}

new_tab = st_init_table_with_size(&objhash, size * 2);

for (i = 0; i < RHASH_AR_TABLE_MAX_BOUND; i++) {
ar_table_pair *pair = RHASH_AR_TABLE_REF(hash, i);
st_add_direct(new_tab, pair->key, pair->val);
}
ar_free_and_clear_table(hash);
RHASH_ST_TABLE_SET(hash, new_tab);
return;
}

static st_table *
ar_force_convert_table(VALUE hash, const char *file, int line)
{
st_table *new_tab;

if (RHASH_ST_TABLE_P(hash)) {
return RHASH_ST_TABLE(hash);
}

if (RHASH_AR_TABLE(hash)) {
unsigned i, bound = RHASH_AR_TABLE_BOUND(hash);

#if RHASH_CONVERT_TABLE_DEBUG
rb_obj_info_dump(hash);
fprintf(stderr, "force_convert: %s:%d\n", file, line);
RB_DEBUG_COUNTER_INC(obj_hash_force_convert);
#endif

new_tab = st_init_table_with_size(&objhash, RHASH_AR_TABLE_SIZE(hash));

for (i = 0; i < bound; i++) {
if (ar_cleared_entry(hash, i)) continue;

ar_table_pair *pair = RHASH_AR_TABLE_REF(hash, i);
st_add_direct(new_tab, pair->key, pair->val);
}
ar_free_and_clear_table(hash);
}
else {
new_tab = st_init_table(&objhash);
}
RHASH_ST_TABLE_SET(hash, new_tab);

return new_tab;
}

static ar_table *
hash_ar_table(VALUE hash)
{
if (RHASH_TABLE_NULL_P(hash)) {
ar_alloc_table(hash);
}
return RHASH_AR_TABLE(hash);
}

static int
ar_compact_table(VALUE hash)
{
const unsigned bound = RHASH_AR_TABLE_BOUND(hash);
const unsigned size = RHASH_AR_TABLE_SIZE(hash);

if (size == bound) {
return size;
}
else {
unsigned i, j=0;
ar_table_pair *pairs = RHASH_AR_TABLE(hash)->pairs;

for (i=0; i<bound; i++) {
if (ar_cleared_entry(hash, i)) {
if (j <= i) j = i+1;
for (; j<bound; j++) {
if (!ar_cleared_entry(hash, j)) {
pairs[i] = pairs[j];
ar_hint_set_hint(hash, i, (st_hash_t)ar_hint(hash, j));
ar_clear_entry(hash, j);
j++;
goto found;
}
}

goto done;
found:;
}
}
done:
HASH_ASSERT(i<=bound);

RHASH_AR_TABLE_BOUND_SET(hash, size);
hash_verify(hash);
return size;
}
}

static int
ar_add_direct_with_hash(VALUE hash, st_data_t key, st_data_t val, st_hash_t hash_value)
{
unsigned bin = RHASH_AR_TABLE_BOUND(hash);

if (RHASH_AR_TABLE_SIZE(hash) >= RHASH_AR_TABLE_MAX_SIZE) {
return 1;
}
else {
if (UNLIKELY(bin >= RHASH_AR_TABLE_MAX_BOUND)) {
bin = ar_compact_table(hash);
hash_ar_table(hash);
}
HASH_ASSERT(bin < RHASH_AR_TABLE_MAX_BOUND);

ar_set_entry(hash, bin, key, val, hash_value);
RHASH_AR_TABLE_BOUND_SET(hash, bin+1);
RHASH_AR_TABLE_SIZE_INC(hash);
return 0;
}
}

static int
ar_general_foreach(VALUE hash, st_foreach_check_callback_func *func, st_update_callback_func *replace, st_data_t arg)
{
if (RHASH_AR_TABLE_SIZE(hash) > 0) {
unsigned i, bound = RHASH_AR_TABLE_BOUND(hash);

for (i = 0; i < bound; i++) {
if (ar_cleared_entry(hash, i)) continue;

ar_table_pair *pair = RHASH_AR_TABLE_REF(hash, i);
enum st_retval retval = (*func)(pair->key, pair->val, arg, 0);


switch (retval) {
case ST_CONTINUE:
break;
case ST_CHECK:
case ST_STOP:
return 0;
case ST_REPLACE:
if (replace) {
VALUE key = pair->key;
VALUE val = pair->val;
retval = (*replace)(&key, &val, arg, TRUE);


ar_table_pair *pair = RHASH_AR_TABLE_REF(hash, i);
pair->key = key;
pair->val = val;
}
break;
case ST_DELETE:
ar_clear_entry(hash, i);
RHASH_AR_TABLE_SIZE_DEC(hash);
break;
}
}
}
return 0;
}

static int
ar_foreach_with_replace(VALUE hash, st_foreach_check_callback_func *func, st_update_callback_func *replace, st_data_t arg)
{
return ar_general_foreach(hash, func, replace, arg);
}

struct functor {
st_foreach_callback_func *func;
st_data_t arg;
};

static int
apply_functor(st_data_t k, st_data_t v, st_data_t d, int _)
{
const struct functor *f = (void *)d;
return f->func(k, v, f->arg);
}

static int
ar_foreach(VALUE hash, st_foreach_callback_func *func, st_data_t arg)
{
const struct functor f = { func, arg };
return ar_general_foreach(hash, apply_functor, NULL, (st_data_t)&f);
}

static int
ar_foreach_check(VALUE hash, st_foreach_check_callback_func *func, st_data_t arg,
st_data_t never)
{
if (RHASH_AR_TABLE_SIZE(hash) > 0) {
unsigned i, ret = 0, bound = RHASH_AR_TABLE_BOUND(hash);
enum st_retval retval;
st_data_t key;
ar_table_pair *pair;
ar_hint_t hint;

for (i = 0; i < bound; i++) {
if (ar_cleared_entry(hash, i)) continue;

pair = RHASH_AR_TABLE_REF(hash, i);
key = pair->key;
hint = ar_hint(hash, i);

retval = (*func)(key, pair->val, arg, 0);
hash_verify(hash);

switch (retval) {
case ST_CHECK: {
pair = RHASH_AR_TABLE_REF(hash, i);
if (pair->key == never) break;
ret = ar_find_entry_hint(hash, hint, key);
if (ret == RHASH_AR_TABLE_MAX_BOUND) {
retval = (*func)(0, 0, arg, 1);
return 2;
}
}
case ST_CONTINUE:
break;
case ST_STOP:
case ST_REPLACE:
return 0;
case ST_DELETE: {
if (!ar_cleared_entry(hash, i)) {
ar_clear_entry(hash, i);
RHASH_AR_TABLE_SIZE_DEC(hash);
}
break;
}
}
}
}
return 0;
}

static int
ar_update(VALUE hash, st_data_t key,
st_update_callback_func *func, st_data_t arg)
{
int retval, existing;
unsigned bin = RHASH_AR_TABLE_MAX_BOUND;
st_data_t value = 0, old_key;
st_hash_t hash_value = ar_do_hash(key);

if (UNLIKELY(!RHASH_AR_TABLE_P(hash))) {

return -1;
}

if (RHASH_AR_TABLE_SIZE(hash) > 0) {
bin = ar_find_entry(hash, hash_value, key);
existing = (bin != RHASH_AR_TABLE_MAX_BOUND) ? TRUE : FALSE;
}
else {
hash_ar_table(hash); 
existing = FALSE;
}

if (existing) {
ar_table_pair *pair = RHASH_AR_TABLE_REF(hash, bin);
key = pair->key;
value = pair->val;
}
old_key = key;
retval = (*func)(&key, &value, arg, existing);


switch (retval) {
case ST_CONTINUE:
if (!existing) {
if (ar_add_direct_with_hash(hash, key, value, hash_value)) {
return -1;
}
}
else {
ar_table_pair *pair = RHASH_AR_TABLE_REF(hash, bin);
if (old_key != key) {
pair->key = key;
}
pair->val = value;
}
break;
case ST_DELETE:
if (existing) {
ar_clear_entry(hash, bin);
RHASH_AR_TABLE_SIZE_DEC(hash);
}
break;
}
return existing;
}

static int
ar_insert(VALUE hash, st_data_t key, st_data_t value)
{
unsigned bin = RHASH_AR_TABLE_BOUND(hash);
st_hash_t hash_value = ar_do_hash(key);

if (UNLIKELY(!RHASH_AR_TABLE_P(hash))) {

return -1;
}

hash_ar_table(hash); 

bin = ar_find_entry(hash, hash_value, key);
if (bin == RHASH_AR_TABLE_MAX_BOUND) {
if (RHASH_AR_TABLE_SIZE(hash) >= RHASH_AR_TABLE_MAX_SIZE) {
return -1;
}
else if (bin >= RHASH_AR_TABLE_MAX_BOUND) {
bin = ar_compact_table(hash);
hash_ar_table(hash);
}
HASH_ASSERT(bin < RHASH_AR_TABLE_MAX_BOUND);

ar_set_entry(hash, bin, key, value, hash_value);
RHASH_AR_TABLE_BOUND_SET(hash, bin+1);
RHASH_AR_TABLE_SIZE_INC(hash);
return 0;
}
else {
RHASH_AR_TABLE_REF(hash, bin)->val = value;
return 1;
}
}

static int
ar_lookup(VALUE hash, st_data_t key, st_data_t *value)
{
if (RHASH_AR_TABLE_SIZE(hash) == 0) {
return 0;
}
else {
st_hash_t hash_value = ar_do_hash(key);
if (UNLIKELY(!RHASH_AR_TABLE_P(hash))) {

return st_lookup(RHASH_ST_TABLE(hash), key, value);
}
unsigned bin = ar_find_entry(hash, hash_value, key);

if (bin == RHASH_AR_TABLE_MAX_BOUND) {
return 0;
}
else {
HASH_ASSERT(bin < RHASH_AR_TABLE_MAX_BOUND);
if (value != NULL) {
*value = RHASH_AR_TABLE_REF(hash, bin)->val;
}
return 1;
}
}
}

static int
ar_delete(VALUE hash, st_data_t *key, st_data_t *value)
{
unsigned bin;
st_hash_t hash_value = ar_do_hash(*key);

if (UNLIKELY(!RHASH_AR_TABLE_P(hash))) {

return st_delete(RHASH_ST_TABLE(hash), key, value);
}

bin = ar_find_entry(hash, hash_value, *key);

if (bin == RHASH_AR_TABLE_MAX_BOUND) {
if (value != 0) *value = 0;
return 0;
}
else {
if (value != 0) {
ar_table_pair *pair = RHASH_AR_TABLE_REF(hash, bin);
*value = pair->val;
}
ar_clear_entry(hash, bin);
RHASH_AR_TABLE_SIZE_DEC(hash);
return 1;
}
}

static int
ar_shift(VALUE hash, st_data_t *key, st_data_t *value)
{
if (RHASH_AR_TABLE_SIZE(hash) > 0) {
unsigned i, bound = RHASH_AR_TABLE_BOUND(hash);

for (i = 0; i < bound; i++) {
if (!ar_cleared_entry(hash, i)) {
ar_table_pair *pair = RHASH_AR_TABLE_REF(hash, i);
if (value != 0) *value = pair->val;
*key = pair->key;
ar_clear_entry(hash, i);
RHASH_AR_TABLE_SIZE_DEC(hash);
return 1;
}
}
}
if (value != NULL) *value = 0;
return 0;
}

static long
ar_keys(VALUE hash, st_data_t *keys, st_index_t size)
{
unsigned i, bound = RHASH_AR_TABLE_BOUND(hash);
st_data_t *keys_start = keys, *keys_end = keys + size;

for (i = 0; i < bound; i++) {
if (keys == keys_end) {
break;
}
else {
if (!ar_cleared_entry(hash, i)) {
*keys++ = RHASH_AR_TABLE_REF(hash, i)->key;
}
}
}

return keys - keys_start;
}

static long
ar_values(VALUE hash, st_data_t *values, st_index_t size)
{
unsigned i, bound = RHASH_AR_TABLE_BOUND(hash);
st_data_t *values_start = values, *values_end = values + size;

for (i = 0; i < bound; i++) {
if (values == values_end) {
break;
}
else {
if (!ar_cleared_entry(hash, i)) {
*values++ = RHASH_AR_TABLE_REF(hash, i)->val;
}
}
}

return values - values_start;
}

static ar_table*
ar_copy(VALUE hash1, VALUE hash2)
{
ar_table *old_tab = RHASH_AR_TABLE(hash2);

if (old_tab != NULL) {
ar_table *new_tab = RHASH_AR_TABLE(hash1);
if (new_tab == NULL) {
new_tab = (ar_table*) rb_transient_heap_alloc(hash1, sizeof(ar_table));
if (new_tab != NULL) {
RHASH_SET_TRANSIENT_FLAG(hash1);
}
else {
RHASH_UNSET_TRANSIENT_FLAG(hash1);
new_tab = (ar_table*)ruby_xmalloc(sizeof(ar_table));
}
}
*new_tab = *old_tab;
RHASH(hash1)->ar_hint.word = RHASH(hash2)->ar_hint.word;
RHASH_AR_TABLE_BOUND_SET(hash1, RHASH_AR_TABLE_BOUND(hash2));
RHASH_AR_TABLE_SIZE_SET(hash1, RHASH_AR_TABLE_SIZE(hash2));
hash_ar_table_set(hash1, new_tab);

rb_gc_writebarrier_remember(hash1);
return new_tab;
}
else {
RHASH_AR_TABLE_BOUND_SET(hash1, RHASH_AR_TABLE_BOUND(hash2));
RHASH_AR_TABLE_SIZE_SET(hash1, RHASH_AR_TABLE_SIZE(hash2));

if (RHASH_TRANSIENT_P(hash1)) {
RHASH_UNSET_TRANSIENT_FLAG(hash1);
}
else if (RHASH_AR_TABLE(hash1)) {
ruby_xfree(RHASH_AR_TABLE(hash1));
}

hash_ar_table_set(hash1, NULL);

rb_gc_writebarrier_remember(hash1);
return old_tab;
}
}

static void
ar_clear(VALUE hash)
{
if (RHASH_AR_TABLE(hash) != NULL) {
RHASH_AR_TABLE_SIZE_SET(hash, 0);
RHASH_AR_TABLE_BOUND_SET(hash, 0);
}
else {
HASH_ASSERT(RHASH_AR_TABLE_SIZE(hash) == 0);
HASH_ASSERT(RHASH_AR_TABLE_BOUND(hash) == 0);
}
}

#if USE_TRANSIENT_HEAP
void
rb_hash_transient_heap_evacuate(VALUE hash, int promote)
{
if (RHASH_TRANSIENT_P(hash)) {
ar_table *new_tab;
ar_table *old_tab = RHASH_AR_TABLE(hash);

if (UNLIKELY(old_tab == NULL)) {
rb_gc_force_recycle(hash);
return;
}
HASH_ASSERT(old_tab != NULL);
if (promote) {
promote:
new_tab = ruby_xmalloc(sizeof(ar_table));
RHASH_UNSET_TRANSIENT_FLAG(hash);
}
else {
new_tab = rb_transient_heap_alloc(hash, sizeof(ar_table));
if (new_tab == NULL) goto promote;
}
*new_tab = *old_tab;
hash_ar_table_set(hash, new_tab);
}
hash_verify(hash);
}
#endif

typedef int st_foreach_func(st_data_t, st_data_t, st_data_t);

struct foreach_safe_arg {
st_table *tbl;
st_foreach_func *func;
st_data_t arg;
};

static int
foreach_safe_i(st_data_t key, st_data_t value, st_data_t args, int error)
{
int status;
struct foreach_safe_arg *arg = (void *)args;

if (error) return ST_STOP;
status = (*arg->func)(key, value, arg->arg);
if (status == ST_CONTINUE) {
return ST_CHECK;
}
return status;
}

void
st_foreach_safe(st_table *table, st_foreach_func *func, st_data_t a)
{
struct foreach_safe_arg arg;

arg.tbl = table;
arg.func = (st_foreach_func *)func;
arg.arg = a;
if (st_foreach_check(table, foreach_safe_i, (st_data_t)&arg, 0)) {
rb_raise(rb_eRuntimeError, "hash modified during iteration");
}
}

typedef int rb_foreach_func(VALUE, VALUE, VALUE);

struct hash_foreach_arg {
VALUE hash;
rb_foreach_func *func;
VALUE arg;
};

static int
hash_ar_foreach_iter(st_data_t key, st_data_t value, st_data_t argp, int error)
{
struct hash_foreach_arg *arg = (struct hash_foreach_arg *)argp;
int status;

if (error) return ST_STOP;
status = (*arg->func)((VALUE)key, (VALUE)value, arg->arg);


switch (status) {
case ST_DELETE:
return ST_DELETE;
case ST_CONTINUE:
break;
case ST_STOP:
return ST_STOP;
}
return ST_CHECK;
}

static int
hash_foreach_iter(st_data_t key, st_data_t value, st_data_t argp, int error)
{
struct hash_foreach_arg *arg = (struct hash_foreach_arg *)argp;
int status;
st_table *tbl;

if (error) return ST_STOP;
tbl = RHASH_ST_TABLE(arg->hash);
status = (*arg->func)((VALUE)key, (VALUE)value, arg->arg);
if (RHASH_ST_TABLE(arg->hash) != tbl) {
rb_raise(rb_eRuntimeError, "rehash occurred during iteration");
}
switch (status) {
case ST_DELETE:
return ST_DELETE;
case ST_CONTINUE:
break;
case ST_STOP:
return ST_STOP;
}
return ST_CHECK;
}

static int
iter_lev_in_ivar(VALUE hash)
{
VALUE levval = rb_ivar_get(hash, id_hash_iter_lev);
HASH_ASSERT(FIXNUM_P(levval));
return FIX2INT(levval);
}

void rb_ivar_set_internal(VALUE obj, ID id, VALUE val);

static void
iter_lev_in_ivar_set(VALUE hash, int lev)
{
rb_ivar_set_internal(hash, id_hash_iter_lev, INT2FIX(lev));
}

static int
iter_lev_in_flags(VALUE hash)
{
unsigned int u = (unsigned int)((RBASIC(hash)->flags >> RHASH_LEV_SHIFT) & RHASH_LEV_MAX);
return (int)u;
}

static int
RHASH_ITER_LEV(VALUE hash)
{
int lev = iter_lev_in_flags(hash);

if (lev == RHASH_LEV_MAX) {
return iter_lev_in_ivar(hash);
}
else {
return lev;
}
}

static void
hash_iter_lev_inc(VALUE hash)
{
int lev = iter_lev_in_flags(hash);
if (lev == RHASH_LEV_MAX) {
lev = iter_lev_in_ivar(hash);
iter_lev_in_ivar_set(hash, lev+1);
}
else {
lev += 1;
RBASIC(hash)->flags = ((RBASIC(hash)->flags & ~RHASH_LEV_MASK) | ((VALUE)lev << RHASH_LEV_SHIFT));
if (lev == RHASH_LEV_MAX) {
iter_lev_in_ivar_set(hash, lev);
}
}
}

static void
hash_iter_lev_dec(VALUE hash)
{
int lev = iter_lev_in_flags(hash);
if (lev == RHASH_LEV_MAX) {
lev = iter_lev_in_ivar(hash);
HASH_ASSERT(lev > 0);
iter_lev_in_ivar_set(hash, lev-1);
}
else {
HASH_ASSERT(lev > 0);
RBASIC(hash)->flags = ((RBASIC(hash)->flags & ~RHASH_LEV_MASK) | ((lev-1) << RHASH_LEV_SHIFT));
}
}

static VALUE
hash_foreach_ensure_rollback(VALUE hash)
{
hash_iter_lev_inc(hash);
return 0;
}

static VALUE
hash_foreach_ensure(VALUE hash)
{
hash_iter_lev_dec(hash);
return 0;
}

int
rb_hash_stlike_foreach(VALUE hash, st_foreach_callback_func *func, st_data_t arg)
{
if (RHASH_AR_TABLE_P(hash)) {
return ar_foreach(hash, func, arg);
}
else {
return st_foreach(RHASH_ST_TABLE(hash), func, arg);
}
}

int
rb_hash_stlike_foreach_with_replace(VALUE hash, st_foreach_check_callback_func *func, st_update_callback_func *replace, st_data_t arg)
{
if (RHASH_AR_TABLE_P(hash)) {
return ar_foreach_with_replace(hash, func, replace, arg);
}
else {
return st_foreach_with_replace(RHASH_ST_TABLE(hash), func, replace, arg);
}
}

static VALUE
hash_foreach_call(VALUE arg)
{
VALUE hash = ((struct hash_foreach_arg *)arg)->hash;
int ret = 0;
if (RHASH_AR_TABLE_P(hash)) {
ret = ar_foreach_check(hash, hash_ar_foreach_iter,
(st_data_t)arg, (st_data_t)Qundef);
}
else if (RHASH_ST_TABLE_P(hash)) {
ret = st_foreach_check(RHASH_ST_TABLE(hash), hash_foreach_iter,
(st_data_t)arg, (st_data_t)Qundef);
}
if (ret) {
rb_raise(rb_eRuntimeError, "ret: %d, hash modified during iteration", ret);
}
return Qnil;
}

void
rb_hash_foreach(VALUE hash, rb_foreach_func *func, VALUE farg)
{
struct hash_foreach_arg arg;

if (RHASH_TABLE_EMPTY_P(hash))
return;
hash_iter_lev_inc(hash);
arg.hash = hash;
arg.func = (rb_foreach_func *)func;
arg.arg = farg;
rb_ensure(hash_foreach_call, (VALUE)&arg, hash_foreach_ensure, hash);
hash_verify(hash);
}

static VALUE
hash_alloc_flags(VALUE klass, VALUE flags, VALUE ifnone)
{
const VALUE wb = (RGENGC_WB_PROTECTED_HASH ? FL_WB_PROTECTED : 0);
NEWOBJ_OF(hash, struct RHash, klass, T_HASH | wb | flags);

RHASH_SET_IFNONE((VALUE)hash, ifnone);

return (VALUE)hash;
}

static VALUE
hash_alloc(VALUE klass)
{
return hash_alloc_flags(klass, 0, Qnil);
}

static VALUE
empty_hash_alloc(VALUE klass)
{
RUBY_DTRACE_CREATE_HOOK(HASH, 0);

return hash_alloc(klass);
}

VALUE
rb_hash_new(void)
{
return hash_alloc(rb_cHash);
}

MJIT_FUNC_EXPORTED VALUE
rb_hash_new_with_size(st_index_t size)
{
VALUE ret = rb_hash_new();
if (size == 0) {

}
else if (size <= RHASH_AR_TABLE_MAX_SIZE) {
ar_alloc_table(ret);
}
else {
RHASH_ST_TABLE_SET(ret, st_init_table_with_size(&objhash, size));
}
return ret;
}

static VALUE
hash_copy(VALUE ret, VALUE hash)
{
if (!RHASH_EMPTY_P(hash)) {
if (RHASH_AR_TABLE_P(hash))
ar_copy(ret, hash);
else if (RHASH_ST_TABLE_P(hash))
RHASH_ST_TABLE_SET(ret, st_copy(RHASH_ST_TABLE(hash)));
}
return ret;
}

static VALUE
hash_dup(VALUE hash, VALUE klass, VALUE flags)
{
return hash_copy(hash_alloc_flags(klass, flags, RHASH_IFNONE(hash)),
hash);
}

VALUE
rb_hash_dup(VALUE hash)
{
const VALUE flags = RBASIC(hash)->flags;
VALUE ret = hash_dup(hash, rb_obj_class(hash),
flags & (FL_EXIVAR|RHASH_PROC_DEFAULT));
if (flags & FL_EXIVAR)
rb_copy_generic_ivar(ret, hash);
return ret;
}

MJIT_FUNC_EXPORTED VALUE
rb_hash_resurrect(VALUE hash)
{
VALUE ret = hash_dup(hash, rb_cHash, 0);
return ret;
}

static void
rb_hash_modify_check(VALUE hash)
{
rb_check_frozen(hash);
}

MJIT_FUNC_EXPORTED struct st_table *
#if RHASH_CONVERT_TABLE_DEBUG
rb_hash_tbl_raw(VALUE hash, const char *file, int line)
{
return ar_force_convert_table(hash, file, line);
}
#else
rb_hash_tbl_raw(VALUE hash)
{
return ar_force_convert_table(hash, NULL, 0);
}
#endif

struct st_table *
rb_hash_tbl(VALUE hash, const char *file, int line)
{
OBJ_WB_UNPROTECT(hash);
return RHASH_TBL_RAW(hash);
}

static void
rb_hash_modify(VALUE hash)
{
rb_hash_modify_check(hash);
}

NORETURN(static void no_new_key(void));
static void
no_new_key(void)
{
rb_raise(rb_eRuntimeError, "can't add a new key into hash during iteration");
}

struct update_callback_arg {
VALUE hash;
st_data_t arg;
};

#define NOINSERT_UPDATE_CALLBACK(func) static int func##_noinsert(st_data_t *key, st_data_t *val, st_data_t arg, int existing) { if (!existing) no_new_key(); return func(key, val, (struct update_arg *)arg, existing); } static int func##_insert(st_data_t *key, st_data_t *val, st_data_t arg, int existing) { return func(key, val, (struct update_arg *)arg, existing); }













struct update_arg {
st_data_t arg;
VALUE hash;
VALUE new_key;
VALUE old_key;
VALUE new_value;
VALUE old_value;
};

typedef int (*tbl_update_func)(st_data_t *, st_data_t *, st_data_t, int);

int
rb_hash_stlike_update(VALUE hash, st_data_t key, st_update_callback_func *func, st_data_t arg)
{
if (RHASH_AR_TABLE_P(hash)) {
int result = ar_update(hash, (st_data_t)key, func, arg);
if (result == -1) {
ar_try_convert_table(hash);
}
else {
return result;
}
}

return st_update(RHASH_ST_TABLE(hash), (st_data_t)key, func, arg);
}

static int
tbl_update(VALUE hash, VALUE key, tbl_update_func func, st_data_t optional_arg)
{
struct update_arg arg;
int result;

arg.arg = optional_arg;
arg.hash = hash;
arg.new_key = 0;
arg.old_key = Qundef;
arg.new_value = 0;
arg.old_value = Qundef;

result = rb_hash_stlike_update(hash, key, func, (st_data_t)&arg);


if (arg.new_key) RB_OBJ_WRITTEN(hash, arg.old_key, arg.new_key);
if (arg.new_value) RB_OBJ_WRITTEN(hash, arg.old_value, arg.new_value);

return result;
}

#define UPDATE_CALLBACK(iter_lev, func) ((iter_lev) > 0 ? func##_noinsert : func##_insert)

#define RHASH_UPDATE_ITER(h, iter_lev, key, func, a) do { tbl_update((h), (key), UPDATE_CALLBACK((iter_lev), func), (st_data_t)(a)); } while (0)



#define RHASH_UPDATE(hash, key, func, arg) RHASH_UPDATE_ITER(hash, RHASH_ITER_LEV(hash), key, func, arg)


static void
set_proc_default(VALUE hash, VALUE proc)
{
if (rb_proc_lambda_p(proc)) {
int n = rb_proc_arity(proc);

if (n != 2 && (n >= 0 || n < -3)) {
if (n < 0) n = -n-1;
rb_raise(rb_eTypeError, "default_proc takes two arguments (2 for %d)", n);
}
}

FL_SET_RAW(hash, RHASH_PROC_DEFAULT);
RHASH_SET_IFNONE(hash, proc);
}




































static VALUE
rb_hash_initialize(int argc, VALUE *argv, VALUE hash)
{
VALUE ifnone;

rb_hash_modify(hash);
if (rb_block_given_p()) {
rb_check_arity(argc, 0, 0);
ifnone = rb_block_proc();
SET_PROC_DEFAULT(hash, ifnone);
}
else {
rb_check_arity(argc, 0, 1);
ifnone = argc == 0 ? Qnil : argv[0];
RHASH_SET_IFNONE(hash, ifnone);
}

return hash;
}





















static VALUE
rb_hash_s_create(int argc, VALUE *argv, VALUE klass)
{
VALUE hash, tmp;

if (argc == 1) {
tmp = rb_hash_s_try_convert(Qnil, argv[0]);
if (!NIL_P(tmp)) {
hash = hash_alloc(klass);
hash_copy(hash, tmp);
return hash;
}

tmp = rb_check_array_type(argv[0]);
if (!NIL_P(tmp)) {
long i;

hash = hash_alloc(klass);
for (i = 0; i < RARRAY_LEN(tmp); ++i) {
VALUE e = RARRAY_AREF(tmp, i);
VALUE v = rb_check_array_type(e);
VALUE key, val = Qnil;

if (NIL_P(v)) {
rb_raise(rb_eArgError, "wrong element type %s at %ld (expected array)",
rb_builtin_class_name(e), i);
}
switch (RARRAY_LEN(v)) {
default:
rb_raise(rb_eArgError, "invalid number of elements (%ld for 1..2)",
RARRAY_LEN(v));
case 2:
val = RARRAY_AREF(v, 1);
case 1:
key = RARRAY_AREF(v, 0);
rb_hash_aset(hash, key, val);
}
}
return hash;
}
}
if (argc % 2 != 0) {
rb_raise(rb_eArgError, "odd number of arguments for Hash");
}

hash = hash_alloc(klass);
rb_hash_bulk_insert(argc, argv, hash);
hash_verify(hash);
return hash;
}

MJIT_FUNC_EXPORTED VALUE
rb_to_hash_type(VALUE hash)
{
return rb_convert_type_with_id(hash, T_HASH, "Hash", idTo_hash);
}
#define to_hash rb_to_hash_type

VALUE
rb_check_hash_type(VALUE hash)
{
return rb_check_convert_type_with_id(hash, T_HASH, "Hash", idTo_hash);
}












static VALUE
rb_hash_s_try_convert(VALUE dummy, VALUE hash)
{
return rb_check_hash_type(hash);
}
















static VALUE
rb_hash_s_ruby2_keywords_hash_p(VALUE dummy, VALUE hash)
{
Check_Type(hash, T_HASH);
return (RHASH(hash)->basic.flags & RHASH_PASS_AS_KEYWORDS) ? Qtrue : Qfalse;
}
















static VALUE
rb_hash_s_ruby2_keywords_hash(VALUE dummy, VALUE hash)
{
Check_Type(hash, T_HASH);
hash = rb_hash_dup(hash);
RHASH(hash)->basic.flags |= RHASH_PASS_AS_KEYWORDS;
return hash;
}

struct rehash_arg {
VALUE hash;
st_table *tbl;
};

static int
rb_hash_rehash_i(VALUE key, VALUE value, VALUE arg)
{
if (RHASH_AR_TABLE_P(arg)) {
ar_insert(arg, (st_data_t)key, (st_data_t)value);
}
else {
st_insert(RHASH_ST_TABLE(arg), (st_data_t)key, (st_data_t)value);
}
return ST_CONTINUE;
}





















VALUE
rb_hash_rehash(VALUE hash)
{
VALUE tmp;
st_table *tbl;

if (RHASH_ITER_LEV(hash) > 0) {
rb_raise(rb_eRuntimeError, "rehash during iteration");
}
rb_hash_modify_check(hash);
if (RHASH_AR_TABLE_P(hash)) {
tmp = hash_alloc(0);
ar_alloc_table(tmp);
rb_hash_foreach(hash, rb_hash_rehash_i, (VALUE)tmp);
ar_free_and_clear_table(hash);
ar_copy(hash, tmp);
ar_free_and_clear_table(tmp);
}
else if (RHASH_ST_TABLE_P(hash)) {
st_table *old_tab = RHASH_ST_TABLE(hash);
tmp = hash_alloc(0);
tbl = st_init_table_with_size(old_tab->type, old_tab->num_entries);
RHASH_ST_TABLE_SET(tmp, tbl);
rb_hash_foreach(hash, rb_hash_rehash_i, (VALUE)tmp);
st_free_table(old_tab);
RHASH_ST_TABLE_SET(hash, tbl);
RHASH_ST_CLEAR(tmp);
}
hash_verify(hash);
return hash;
}

static VALUE
call_default_proc(VALUE proc, VALUE hash, VALUE key)
{
VALUE args[2] = {hash, key};
return rb_proc_call_with_block(proc, 2, args, Qnil);
}

VALUE
rb_hash_default_value(VALUE hash, VALUE key)
{
if (LIKELY(rb_method_basic_definition_p(CLASS_OF(hash), id_default))) {
VALUE ifnone = RHASH_IFNONE(hash);
if (!FL_TEST(hash, RHASH_PROC_DEFAULT)) return ifnone;
if (key == Qundef) return Qnil;
return call_default_proc(ifnone, hash, key);
}
else {
return rb_funcall(hash, id_default, 1, key);
}
}

static inline int
hash_stlike_lookup(VALUE hash, st_data_t key, st_data_t *pval)
{
hash_verify(hash);

if (RHASH_AR_TABLE_P(hash)) {
return ar_lookup(hash, key, pval);
}
else {
return st_lookup(RHASH_ST_TABLE(hash), key, pval);
}
}

MJIT_FUNC_EXPORTED int
rb_hash_stlike_lookup(VALUE hash, st_data_t key, st_data_t *pval)
{
return hash_stlike_lookup(hash, key, pval);
}















VALUE
rb_hash_aref(VALUE hash, VALUE key)
{
st_data_t val;

if (hash_stlike_lookup(hash, key, &val)) {
return (VALUE)val;
}
else {
return rb_hash_default_value(hash, key);
}
}

VALUE
rb_hash_lookup2(VALUE hash, VALUE key, VALUE def)
{
st_data_t val;

if (hash_stlike_lookup(hash, key, &val)) {
return (VALUE)val;
}
else {
return def; 
}
}

VALUE
rb_hash_lookup(VALUE hash, VALUE key)
{
return rb_hash_lookup2(hash, key, Qnil);
}






























static VALUE
rb_hash_fetch_m(int argc, VALUE *argv, VALUE hash)
{
VALUE key;
st_data_t val;
long block_given;

rb_check_arity(argc, 1, 2);
key = argv[0];

block_given = rb_block_given_p();
if (block_given && argc == 2) {
rb_warn("block supersedes default value argument");
}

if (hash_stlike_lookup(hash, key, &val)) {
return (VALUE)val;
}
else {
if (block_given) {
return rb_yield(key);
}
else if (argc == 1) {
VALUE desc = rb_protect(rb_inspect, key, 0);
if (NIL_P(desc)) {
desc = rb_any_to_s(key);
}
desc = rb_str_ellipsize(desc, 65);
rb_key_err_raise(rb_sprintf("key not found: %"PRIsVALUE, desc), hash, key);
}
else {
return argv[1];
}
}
}

VALUE
rb_hash_fetch(VALUE hash, VALUE key)
{
return rb_hash_fetch_m(1, &key, hash);
}






















static VALUE
rb_hash_default(int argc, VALUE *argv, VALUE hash)
{
VALUE ifnone;

rb_check_arity(argc, 0, 1);
ifnone = RHASH_IFNONE(hash);
if (FL_TEST(hash, RHASH_PROC_DEFAULT)) {
if (argc == 0) return Qnil;
return call_default_proc(ifnone, hash, argv[0]);
}
return ifnone;
}





















static VALUE
rb_hash_set_default(VALUE hash, VALUE ifnone)
{
rb_hash_modify_check(hash);
SET_DEFAULT(hash, ifnone);
return ifnone;
}
















static VALUE
rb_hash_default_proc(VALUE hash)
{
if (FL_TEST(hash, RHASH_PROC_DEFAULT)) {
return RHASH_IFNONE(hash);
}
return Qnil;
}














VALUE
rb_hash_set_default_proc(VALUE hash, VALUE proc)
{
VALUE b;

rb_hash_modify_check(hash);
if (NIL_P(proc)) {
SET_DEFAULT(hash, proc);
return proc;
}
b = rb_check_convert_type_with_id(proc, T_DATA, "Proc", idTo_proc);
if (NIL_P(b) || !rb_obj_is_proc(b)) {
rb_raise(rb_eTypeError,
"wrong default_proc type %s (expected Proc)",
rb_obj_classname(proc));
}
proc = b;
SET_PROC_DEFAULT(hash, proc);
return proc;
}

static int
key_i(VALUE key, VALUE value, VALUE arg)
{
VALUE *args = (VALUE *)arg;

if (rb_equal(value, args[0])) {
args[1] = key;
return ST_STOP;
}
return ST_CONTINUE;
}















static VALUE
rb_hash_key(VALUE hash, VALUE value)
{
VALUE args[2];

args[0] = value;
args[1] = Qnil;

rb_hash_foreach(hash, key_i, (VALUE)args);

return args[1];
}


static VALUE
rb_hash_index(VALUE hash, VALUE value)
{
rb_warn_deprecated("Hash#index", "Hash#key");
return rb_hash_key(hash, value);
}

int
rb_hash_stlike_delete(VALUE hash, st_data_t *pkey, st_data_t *pval)
{
if (RHASH_AR_TABLE_P(hash)) {
return ar_delete(hash, pkey, pval);
}
else {
return st_delete(RHASH_ST_TABLE(hash), pkey, pval);
}
}






VALUE
rb_hash_delete_entry(VALUE hash, VALUE key)
{
st_data_t ktmp = (st_data_t)key, val;

if (rb_hash_stlike_delete(hash, &ktmp, &val)) {
return (VALUE)val;
}
else {
return Qundef;
}
}






VALUE
rb_hash_delete(VALUE hash, VALUE key)
{
VALUE deleted_value = rb_hash_delete_entry(hash, key);

if (deleted_value != Qundef) { 
return deleted_value;
}
else {
return Qnil;
}
}



















static VALUE
rb_hash_delete_m(VALUE hash, VALUE key)
{
VALUE val;

rb_hash_modify_check(hash);
val = rb_hash_delete_entry(hash, key);

if (val != Qundef) {
return val;
}
else {
if (rb_block_given_p()) {
return rb_yield(key);
}
else {
return Qnil;
}
}
}

struct shift_var {
VALUE key;
VALUE val;
};

static int
shift_i_safe(VALUE key, VALUE value, VALUE arg)
{
struct shift_var *var = (struct shift_var *)arg;

var->key = key;
var->val = value;
return ST_STOP;
}














static VALUE
rb_hash_shift(VALUE hash)
{
struct shift_var var;

rb_hash_modify_check(hash);
if (RHASH_AR_TABLE_P(hash)) {
var.key = Qundef;
if (RHASH_ITER_LEV(hash) == 0) {
if (ar_shift(hash, &var.key, &var.val)) {
return rb_assoc_new(var.key, var.val);
}
}
else {
rb_hash_foreach(hash, shift_i_safe, (VALUE)&var);
if (var.key != Qundef) {
rb_hash_delete_entry(hash, var.key);
return rb_assoc_new(var.key, var.val);
}
}
}
if (RHASH_ST_TABLE_P(hash)) {
var.key = Qundef;
if (RHASH_ITER_LEV(hash) == 0) {
if (st_shift(RHASH_ST_TABLE(hash), &var.key, &var.val)) {
return rb_assoc_new(var.key, var.val);
}
}
else {
rb_hash_foreach(hash, shift_i_safe, (VALUE)&var);
if (var.key != Qundef) {
rb_hash_delete_entry(hash, var.key);
return rb_assoc_new(var.key, var.val);
}
}
}
return rb_hash_default_value(hash, Qnil);
}

static int
delete_if_i(VALUE key, VALUE value, VALUE hash)
{
if (RTEST(rb_yield_values(2, key, value))) {
return ST_DELETE;
}
return ST_CONTINUE;
}

static VALUE
hash_enum_size(VALUE hash, VALUE args, VALUE eobj)
{
return rb_hash_size(hash);
}
















VALUE
rb_hash_delete_if(VALUE hash)
{
RETURN_SIZED_ENUMERATOR(hash, 0, 0, hash_enum_size);
rb_hash_modify_check(hash);
if (!RHASH_TABLE_EMPTY_P(hash)) {
rb_hash_foreach(hash, delete_if_i, hash);
}
return hash;
}










VALUE
rb_hash_reject_bang(VALUE hash)
{
st_index_t n;

RETURN_SIZED_ENUMERATOR(hash, 0, 0, hash_enum_size);
rb_hash_modify(hash);
n = RHASH_SIZE(hash);
if (!n) return Qnil;
rb_hash_foreach(hash, delete_if_i, hash);
if (n == RHASH_SIZE(hash)) return Qnil;
return hash;
}

static int
reject_i(VALUE key, VALUE value, VALUE result)
{
if (!RTEST(rb_yield_values(2, key, value))) {
rb_hash_aset(result, key, value);
}
return ST_CONTINUE;
}















VALUE
rb_hash_reject(VALUE hash)
{
VALUE result;

RETURN_SIZED_ENUMERATOR(hash, 0, 0, hash_enum_size);
if (RTEST(ruby_verbose)) {
VALUE klass;
if (HAS_EXTRA_STATES(hash, klass)) {
rb_warn("extra states are no longer copied: %+"PRIsVALUE, hash);
}
}
result = rb_hash_new();
if (!RHASH_EMPTY_P(hash)) {
rb_hash_foreach(hash, reject_i, result);
}
return result;
}












static VALUE
rb_hash_slice(int argc, VALUE *argv, VALUE hash)
{
int i;
VALUE key, value, result;

if (argc == 0 || RHASH_EMPTY_P(hash)) {
return rb_hash_new();
}
result = rb_hash_new_with_size(argc);

for (i = 0; i < argc; i++) {
key = argv[i];
value = rb_hash_lookup2(hash, key, Qundef);
if (value != Qundef)
rb_hash_aset(result, key, value);
}

return result;
}












VALUE
rb_hash_values_at(int argc, VALUE *argv, VALUE hash)
{
VALUE result = rb_ary_new2(argc);
long i;

for (i=0; i<argc; i++) {
rb_ary_push(result, rb_hash_aref(hash, argv[i]));
}
return result;
}

















VALUE
rb_hash_fetch_values(int argc, VALUE *argv, VALUE hash)
{
VALUE result = rb_ary_new2(argc);
long i;

for (i=0; i<argc; i++) {
rb_ary_push(result, rb_hash_fetch(hash, argv[i]));
}
return result;
}

static int
select_i(VALUE key, VALUE value, VALUE result)
{
if (RTEST(rb_yield_values(2, key, value))) {
rb_hash_aset(result, key, value);
}
return ST_CONTINUE;
}



















VALUE
rb_hash_select(VALUE hash)
{
VALUE result;

RETURN_SIZED_ENUMERATOR(hash, 0, 0, hash_enum_size);
result = rb_hash_new();
if (!RHASH_EMPTY_P(hash)) {
rb_hash_foreach(hash, select_i, result);
}
return result;
}

static int
keep_if_i(VALUE key, VALUE value, VALUE hash)
{
if (!RTEST(rb_yield_values(2, key, value))) {
return ST_DELETE;
}
return ST_CONTINUE;
}














VALUE
rb_hash_select_bang(VALUE hash)
{
st_index_t n;

RETURN_SIZED_ENUMERATOR(hash, 0, 0, hash_enum_size);
rb_hash_modify_check(hash);
n = RHASH_SIZE(hash);
if (!n) return Qnil;
rb_hash_foreach(hash, keep_if_i, hash);
if (n == RHASH_SIZE(hash)) return Qnil;
return hash;
}














VALUE
rb_hash_keep_if(VALUE hash)
{
RETURN_SIZED_ENUMERATOR(hash, 0, 0, hash_enum_size);
rb_hash_modify_check(hash);
if (!RHASH_TABLE_EMPTY_P(hash)) {
rb_hash_foreach(hash, keep_if_i, hash);
}
return hash;
}

static int
clear_i(VALUE key, VALUE value, VALUE dummy)
{
return ST_DELETE;
}












VALUE
rb_hash_clear(VALUE hash)
{
rb_hash_modify_check(hash);

if (RHASH_ITER_LEV(hash) > 0) {
rb_hash_foreach(hash, clear_i, 0);
}
else if (RHASH_AR_TABLE_P(hash)) {
ar_clear(hash);
}
else {
st_clear(RHASH_ST_TABLE(hash));
}

return hash;
}

static int
hash_aset(st_data_t *key, st_data_t *val, struct update_arg *arg, int existing)
{
if (existing) {
arg->new_value = arg->arg;
arg->old_value = *val;
}
else {
arg->new_key = *key;
arg->new_value = arg->arg;
}
*val = arg->arg;
return ST_CONTINUE;
}

VALUE
rb_hash_key_str(VALUE key)
{
if (!RB_FL_ANY_RAW(key, FL_EXIVAR) && RBASIC_CLASS(key) == rb_cString) {
return rb_fstring(key);
}
else {
return rb_str_new_frozen(key);
}
}

static int
hash_aset_str(st_data_t *key, st_data_t *val, struct update_arg *arg, int existing)
{
if (!existing && !RB_OBJ_FROZEN(*key)) {
*key = rb_hash_key_str(*key);
}
return hash_aset(key, val, arg, existing);
}

NOINSERT_UPDATE_CALLBACK(hash_aset)
NOINSERT_UPDATE_CALLBACK(hash_aset_str)




























VALUE
rb_hash_aset(VALUE hash, VALUE key, VALUE val)
{
int iter_lev = RHASH_ITER_LEV(hash);

rb_hash_modify(hash);

if (RHASH_TABLE_NULL_P(hash)) {
if (iter_lev > 0) no_new_key();
ar_alloc_table(hash);
}

if (RHASH_TYPE(hash) == &identhash || rb_obj_class(key) != rb_cString) {
RHASH_UPDATE_ITER(hash, iter_lev, key, hash_aset, val);
}
else {
RHASH_UPDATE_ITER(hash, iter_lev, key, hash_aset_str, val);
}
return val;
}













static VALUE
rb_hash_replace(VALUE hash, VALUE hash2)
{
rb_hash_modify_check(hash);
if (hash == hash2) return hash;
if (RHASH_ITER_LEV(hash) > 0) {
rb_raise(rb_eRuntimeError, "can't replace hash during iteration");
}
hash2 = to_hash(hash2);

COPY_DEFAULT(hash, hash2);

if (RHASH_AR_TABLE_P(hash)) {
if (RHASH_AR_TABLE_P(hash2)) {
ar_clear(hash);
}
else {
ar_free_and_clear_table(hash);
RHASH_ST_TABLE_SET(hash, st_init_table_with_size(RHASH_TYPE(hash2), RHASH_SIZE(hash2)));
}
}
else {
if (RHASH_AR_TABLE_P(hash2)) {
st_free_table(RHASH_ST_TABLE(hash));
RHASH_ST_CLEAR(hash);
}
else {
st_clear(RHASH_ST_TABLE(hash));
RHASH_TBL_RAW(hash)->type = RHASH_ST_TABLE(hash2)->type;
}
}
rb_hash_foreach(hash2, rb_hash_rehash_i, (VALUE)hash);

rb_gc_writebarrier_remember(hash);

return hash;
}

















VALUE
rb_hash_size(VALUE hash)
{
return INT2FIX(RHASH_SIZE(hash));
}

size_t
rb_hash_size_num(VALUE hash)
{
return (long)RHASH_SIZE(hash);
}











static VALUE
rb_hash_empty_p(VALUE hash)
{
return RHASH_EMPTY_P(hash) ? Qtrue : Qfalse;
}

static int
each_value_i(VALUE key, VALUE value, VALUE _)
{
rb_yield(value);
return ST_CONTINUE;
}




















static VALUE
rb_hash_each_value(VALUE hash)
{
RETURN_SIZED_ENUMERATOR(hash, 0, 0, hash_enum_size);
rb_hash_foreach(hash, each_value_i, 0);
return hash;
}

static int
each_key_i(VALUE key, VALUE value, VALUE _)
{
rb_yield(key);
return ST_CONTINUE;
}



















static VALUE
rb_hash_each_key(VALUE hash)
{
RETURN_SIZED_ENUMERATOR(hash, 0, 0, hash_enum_size);
rb_hash_foreach(hash, each_key_i, 0);
return hash;
}

static int
each_pair_i(VALUE key, VALUE value, VALUE _)
{
rb_yield(rb_assoc_new(key, value));
return ST_CONTINUE;
}

static int
each_pair_i_fast(VALUE key, VALUE value, VALUE _)
{
VALUE argv[2];
argv[0] = key;
argv[1] = value;
rb_yield_values2(2, argv);
return ST_CONTINUE;
}























static VALUE
rb_hash_each_pair(VALUE hash)
{
RETURN_SIZED_ENUMERATOR(hash, 0, 0, hash_enum_size);
if (rb_block_pair_yield_optimizable())
rb_hash_foreach(hash, each_pair_i_fast, 0);
else
rb_hash_foreach(hash, each_pair_i, 0);
return hash;
}

struct transform_keys_args{
VALUE trans;
VALUE result;
int block_given;
};

static int
transform_keys_hash_i(VALUE key, VALUE value, VALUE transarg)
{
struct transform_keys_args *p = (void *)transarg;
VALUE trans = p->trans, result = p->result;
VALUE new_key = rb_hash_lookup2(trans, key, Qundef);
if (new_key == Qundef) {
if (p->block_given)
new_key = rb_yield(key);
else
new_key = key;
}
rb_hash_aset(result, new_key, value);
return ST_CONTINUE;
}

static int
transform_keys_i(VALUE key, VALUE value, VALUE result)
{
VALUE new_key = rb_yield(key);
rb_hash_aset(result, new_key, value);
return ST_CONTINUE;
}


















static VALUE
rb_hash_transform_keys(int argc, VALUE *argv, VALUE hash)
{
VALUE result;
struct transform_keys_args transarg = {0};

argc = rb_check_arity(argc, 0, 1);
if (argc > 0) {
transarg.trans = to_hash(argv[0]);
transarg.block_given = rb_block_given_p();
}
else {
RETURN_SIZED_ENUMERATOR(hash, 0, 0, hash_enum_size);
}
result = rb_hash_new();
if (!RHASH_EMPTY_P(hash)) {
if (transarg.trans) {
transarg.result = result;
rb_hash_foreach(hash, transform_keys_hash_i, (VALUE)&transarg);
}
else {
rb_hash_foreach(hash, transform_keys_i, result);
}
}

return result;
}

static VALUE rb_hash_flatten(int argc, VALUE *argv, VALUE hash);


















static VALUE
rb_hash_transform_keys_bang(int argc, VALUE *argv, VALUE hash)
{
VALUE trans = 0;
int block_given = 0;

argc = rb_check_arity(argc, 0, 1);
if (argc > 0) {
trans = to_hash(argv[0]);
block_given = rb_block_given_p();
}
else {
RETURN_SIZED_ENUMERATOR(hash, 0, 0, hash_enum_size);
}
rb_hash_modify_check(hash);
if (!RHASH_TABLE_EMPTY_P(hash)) {
long i;
VALUE pairs = rb_hash_flatten(0, NULL, hash);
rb_hash_clear(hash);
for (i = 0; i < RARRAY_LEN(pairs); i += 2) {
VALUE key = RARRAY_AREF(pairs, i), new_key, val;

if (!trans) {
new_key = rb_yield(key);
}
else if ((new_key = rb_hash_lookup2(trans, key, Qundef)) != Qundef) {

}
else if (block_given) {
new_key = rb_yield(key);
}
else {
new_key = key;
}
val = RARRAY_AREF(pairs, i+1);
rb_hash_aset(hash, new_key, val);
}
}
return hash;
}

static int
transform_values_foreach_func(st_data_t key, st_data_t value, st_data_t argp, int error)
{
return ST_REPLACE;
}

static int
transform_values_foreach_replace(st_data_t *key, st_data_t *value, st_data_t argp, int existing)
{
VALUE new_value = rb_yield((VALUE)*value);
VALUE hash = (VALUE)argp;
RB_OBJ_WRITE(hash, value, new_value);
return ST_CONTINUE;
}


















static VALUE
rb_hash_transform_values(VALUE hash)
{
VALUE result;

RETURN_SIZED_ENUMERATOR(hash, 0, 0, hash_enum_size);
result = hash_copy(hash_alloc(rb_cHash), hash);

if (!RHASH_EMPTY_P(hash)) {
rb_hash_stlike_foreach_with_replace(result, transform_values_foreach_func, transform_values_foreach_replace, result);
}

return result;
}


















static VALUE
rb_hash_transform_values_bang(VALUE hash)
{
RETURN_SIZED_ENUMERATOR(hash, 0, 0, hash_enum_size);
rb_hash_modify_check(hash);

if (!RHASH_TABLE_EMPTY_P(hash)) {
rb_hash_stlike_foreach_with_replace(hash, transform_values_foreach_func, transform_values_foreach_replace, hash);
}

return hash;
}

static int
to_a_i(VALUE key, VALUE value, VALUE ary)
{
rb_ary_push(ary, rb_assoc_new(key, value));
return ST_CONTINUE;
}












static VALUE
rb_hash_to_a(VALUE hash)
{
VALUE ary;

ary = rb_ary_new_capa(RHASH_SIZE(hash));
rb_hash_foreach(hash, to_a_i, ary);

return ary;
}

static int
inspect_i(VALUE key, VALUE value, VALUE str)
{
VALUE str2;

str2 = rb_inspect(key);
if (RSTRING_LEN(str) > 1) {
rb_str_buf_cat_ascii(str, ", ");
}
else {
rb_enc_copy(str, str2);
}
rb_str_buf_append(str, str2);
rb_str_buf_cat_ascii(str, "=>");
str2 = rb_inspect(value);
rb_str_buf_append(str, str2);

return ST_CONTINUE;
}

static VALUE
inspect_hash(VALUE hash, VALUE dummy, int recur)
{
VALUE str;

if (recur) return rb_usascii_str_new2("{...}");
str = rb_str_buf_new2("{");
rb_hash_foreach(hash, inspect_i, str);
rb_str_buf_cat2(str, "}");

return str;
}












static VALUE
rb_hash_inspect(VALUE hash)
{
if (RHASH_EMPTY_P(hash))
return rb_usascii_str_new2("{}");
return rb_exec_recursive(inspect_hash, hash, 0);
}








static VALUE
rb_hash_to_hash(VALUE hash)
{
return hash;
}

VALUE
rb_hash_set_pair(VALUE hash, VALUE arg)
{
VALUE pair;

pair = rb_check_array_type(arg);
if (NIL_P(pair)) {
rb_raise(rb_eTypeError, "wrong element type %s (expected array)",
rb_builtin_class_name(arg));
}
if (RARRAY_LEN(pair) != 2) {
rb_raise(rb_eArgError, "element has wrong array length (expected 2, was %ld)",
RARRAY_LEN(pair));
}
rb_hash_aset(hash, RARRAY_AREF(pair, 0), RARRAY_AREF(pair, 1));
return hash;
}

static int
to_h_i(VALUE key, VALUE value, VALUE hash)
{
rb_hash_set_pair(hash, rb_yield_values(2, key, value));
return ST_CONTINUE;
}

static VALUE
rb_hash_to_h_block(VALUE hash)
{
VALUE h = rb_hash_new_with_size(RHASH_SIZE(hash));
rb_hash_foreach(hash, to_h_i, h);
return h;
}













static VALUE
rb_hash_to_h(VALUE hash)
{
if (rb_block_given_p()) {
return rb_hash_to_h_block(hash);
}
if (rb_obj_class(hash) != rb_cHash) {
const VALUE flags = RBASIC(hash)->flags;
hash = hash_dup(hash, rb_cHash, flags & RHASH_PROC_DEFAULT);
}
return hash;
}

static int
keys_i(VALUE key, VALUE value, VALUE ary)
{
rb_ary_push(ary, key);
return ST_CONTINUE;
}













MJIT_FUNC_EXPORTED VALUE
rb_hash_keys(VALUE hash)
{
st_index_t size = RHASH_SIZE(hash);
VALUE keys = rb_ary_new_capa(size);

if (size == 0) return keys;

if (ST_DATA_COMPATIBLE_P(VALUE)) {
RARRAY_PTR_USE_TRANSIENT(keys, ptr, {
if (RHASH_AR_TABLE_P(hash)) {
size = ar_keys(hash, ptr, size);
}
else {
st_table *table = RHASH_ST_TABLE(hash);
size = st_keys(table, ptr, size);
}
});
rb_gc_writebarrier_remember(keys);
rb_ary_set_len(keys, size);
}
else {
rb_hash_foreach(hash, keys_i, keys);
}

return keys;
}

static int
values_i(VALUE key, VALUE value, VALUE ary)
{
rb_ary_push(ary, value);
return ST_CONTINUE;
}













VALUE
rb_hash_values(VALUE hash)
{
VALUE values;
st_index_t size = RHASH_SIZE(hash);

values = rb_ary_new_capa(size);
if (size == 0) return values;

if (ST_DATA_COMPATIBLE_P(VALUE)) {
if (RHASH_AR_TABLE_P(hash)) {
rb_gc_writebarrier_remember(values);
RARRAY_PTR_USE_TRANSIENT(values, ptr, {
size = ar_values(hash, ptr, size);
});
}
else if (RHASH_ST_TABLE_P(hash)) {
st_table *table = RHASH_ST_TABLE(hash);
rb_gc_writebarrier_remember(values);
RARRAY_PTR_USE_TRANSIENT(values, ptr, {
size = st_values(table, ptr, size);
});
}
rb_ary_set_len(values, size);
}
else {
rb_hash_foreach(hash, values_i, values);
}

return values;
}




















MJIT_FUNC_EXPORTED VALUE
rb_hash_has_key(VALUE hash, VALUE key)
{
if (hash_stlike_lookup(hash, key, NULL)) {
return Qtrue;
}
else {
return Qfalse;
}
}

static int
rb_hash_search_value(VALUE key, VALUE value, VALUE arg)
{
VALUE *data = (VALUE *)arg;

if (rb_equal(value, data[1])) {
data[0] = Qtrue;
return ST_STOP;
}
return ST_CONTINUE;
}














static VALUE
rb_hash_has_value(VALUE hash, VALUE val)
{
VALUE data[2];

data[0] = Qfalse;
data[1] = val;
rb_hash_foreach(hash, rb_hash_search_value, (VALUE)data);
return data[0];
}

struct equal_data {
VALUE result;
VALUE hash;
int eql;
};

static int
eql_i(VALUE key, VALUE val1, VALUE arg)
{
struct equal_data *data = (struct equal_data *)arg;
st_data_t val2;

if (!hash_stlike_lookup(data->hash, key, &val2)) {
data->result = Qfalse;
return ST_STOP;
}
else {
if (!(data->eql ? rb_eql(val1, (VALUE)val2) : (int)rb_equal(val1, (VALUE)val2))) {
data->result = Qfalse;
return ST_STOP;
}
return ST_CONTINUE;
}
}

static VALUE
recursive_eql(VALUE hash, VALUE dt, int recur)
{
struct equal_data *data;

if (recur) return Qtrue; 
data = (struct equal_data*)dt;
data->result = Qtrue;
rb_hash_foreach(hash, eql_i, dt);

return data->result;
}

static VALUE
hash_equal(VALUE hash1, VALUE hash2, int eql)
{
struct equal_data data;

if (hash1 == hash2) return Qtrue;
if (!RB_TYPE_P(hash2, T_HASH)) {
if (!rb_respond_to(hash2, idTo_hash)) {
return Qfalse;
}
if (eql) {
if (rb_eql(hash2, hash1)) {
return Qtrue;
}
else {
return Qfalse;
}
}
else {
return rb_equal(hash2, hash1);
}
}
if (RHASH_SIZE(hash1) != RHASH_SIZE(hash2))
return Qfalse;
if (!RHASH_TABLE_EMPTY_P(hash1) && !RHASH_TABLE_EMPTY_P(hash2)) {
if (RHASH_TYPE(hash1) != RHASH_TYPE(hash2)) {
return Qfalse;
}
else {
data.hash = hash2;
data.eql = eql;
return rb_exec_recursive_paired(recursive_eql, hash1, hash2, (VALUE)&data);
}
}

#if 0
if (!(rb_equal(RHASH_IFNONE(hash1), RHASH_IFNONE(hash2)) &&
FL_TEST(hash1, RHASH_PROC_DEFAULT) == FL_TEST(hash2, RHASH_PROC_DEFAULT)))
return Qfalse;
#endif
return Qtrue;
}

























static VALUE
rb_hash_equal(VALUE hash1, VALUE hash2)
{
return hash_equal(hash1, hash2, FALSE);
}










static VALUE
rb_hash_eql(VALUE hash1, VALUE hash2)
{
return hash_equal(hash1, hash2, TRUE);
}

static int
hash_i(VALUE key, VALUE val, VALUE arg)
{
st_index_t *hval = (st_index_t *)arg;
st_index_t hdata[2];

hdata[0] = rb_hash(key);
hdata[1] = rb_hash(val);
*hval ^= st_hash(hdata, sizeof(hdata), 0);
return ST_CONTINUE;
}











static VALUE
rb_hash_hash(VALUE hash)
{
st_index_t size = RHASH_SIZE(hash);
st_index_t hval = rb_hash_start(size);
hval = rb_hash_uint(hval, (st_index_t)rb_hash_hash);
if (size) {
rb_hash_foreach(hash, hash_i, (VALUE)&hval);
}
hval = rb_hash_end(hval);
return ST2FIX(hval);
}

static int
rb_hash_invert_i(VALUE key, VALUE value, VALUE hash)
{
rb_hash_aset(hash, value, key);
return ST_CONTINUE;
}































static VALUE
rb_hash_invert(VALUE hash)
{
VALUE h = rb_hash_new_with_size(RHASH_SIZE(hash));

rb_hash_foreach(hash, rb_hash_invert_i, h);
return h;
}

static int
rb_hash_update_callback(st_data_t *key, st_data_t *value, struct update_arg *arg, int existing)
{
if (existing) {
arg->old_value = *value;
arg->new_value = arg->arg;
}
else {
arg->new_key = *key;
arg->new_value = arg->arg;
}
*value = arg->arg;
return ST_CONTINUE;
}

NOINSERT_UPDATE_CALLBACK(rb_hash_update_callback)

static int
rb_hash_update_i(VALUE key, VALUE value, VALUE hash)
{
RHASH_UPDATE(hash, key, rb_hash_update_callback, value);
return ST_CONTINUE;
}

static int
rb_hash_update_block_callback(st_data_t *key, st_data_t *value, struct update_arg *arg, int existing)
{
VALUE newvalue = (VALUE)arg->arg;

if (existing) {
newvalue = rb_yield_values(3, (VALUE)*key, (VALUE)*value, newvalue);
arg->old_value = *value;
}
else {
arg->new_key = *key;
}
arg->new_value = newvalue;
*value = newvalue;
return ST_CONTINUE;
}

NOINSERT_UPDATE_CALLBACK(rb_hash_update_block_callback)

static int
rb_hash_update_block_i(VALUE key, VALUE value, VALUE hash)
{
RHASH_UPDATE(hash, key, rb_hash_update_block_callback, value);
return ST_CONTINUE;
}












































static VALUE
rb_hash_update(int argc, VALUE *argv, VALUE self)
{
int i;
bool block_given = rb_block_given_p();

rb_hash_modify(self);
for (i = 0; i < argc; i++){
VALUE hash = to_hash(argv[i]);
if (block_given) {
rb_hash_foreach(hash, rb_hash_update_block_i, self);
}
else {
rb_hash_foreach(hash, rb_hash_update_i, self);
}
}
return self;
}

struct update_func_arg {
VALUE hash;
VALUE value;
rb_hash_update_func *func;
};

static int
rb_hash_update_func_callback(st_data_t *key, st_data_t *value, struct update_arg *arg, int existing)
{
struct update_func_arg *uf_arg = (struct update_func_arg *)arg->arg;
VALUE newvalue = uf_arg->value;

if (existing) {
newvalue = (*uf_arg->func)((VALUE)*key, (VALUE)*value, newvalue);
arg->old_value = *value;
}
else {
arg->new_key = *key;
}
arg->new_value = newvalue;
*value = newvalue;
return ST_CONTINUE;
}

NOINSERT_UPDATE_CALLBACK(rb_hash_update_func_callback)

static int
rb_hash_update_func_i(VALUE key, VALUE value, VALUE arg0)
{
struct update_func_arg *arg = (struct update_func_arg *)arg0;
VALUE hash = arg->hash;

arg->value = value;
RHASH_UPDATE(hash, key, rb_hash_update_func_callback, (VALUE)arg);
return ST_CONTINUE;
}

VALUE
rb_hash_update_by(VALUE hash1, VALUE hash2, rb_hash_update_func *func)
{
rb_hash_modify(hash1);
hash2 = to_hash(hash2);
if (func) {
struct update_func_arg arg;
arg.hash = hash1;
arg.func = func;
rb_hash_foreach(hash2, rb_hash_update_func_i, (VALUE)&arg);
}
else {
rb_hash_foreach(hash2, rb_hash_update_i, hash1);
}
return hash1;
}
































static VALUE
rb_hash_merge(int argc, VALUE *argv, VALUE self)
{
return rb_hash_update(argc, argv, rb_hash_dup(self));
}

static int
assoc_cmp(VALUE a, VALUE b)
{
return !RTEST(rb_equal(a, b));
}

static VALUE
lookup2_call(VALUE arg)
{
VALUE *args = (VALUE *)arg;
return rb_hash_lookup2(args[0], args[1], Qundef);
}

struct reset_hash_type_arg {
VALUE hash;
const struct st_hash_type *orighash;
};

static VALUE
reset_hash_type(VALUE arg)
{
struct reset_hash_type_arg *p = (struct reset_hash_type_arg *)arg;
HASH_ASSERT(RHASH_ST_TABLE_P(p->hash));
RHASH_ST_TABLE(p->hash)->type = p->orighash;
return Qundef;
}

static int
assoc_i(VALUE key, VALUE val, VALUE arg)
{
VALUE *args = (VALUE *)arg;

if (RTEST(rb_equal(args[0], key))) {
args[1] = rb_assoc_new(key, val);
return ST_STOP;
}
return ST_CONTINUE;
}















VALUE
rb_hash_assoc(VALUE hash, VALUE key)
{
st_table *table;
const struct st_hash_type *orighash;
VALUE args[2];

if (RHASH_EMPTY_P(hash)) return Qnil;

ar_force_convert_table(hash, __FILE__, __LINE__);
HASH_ASSERT(RHASH_ST_TABLE_P(hash));
table = RHASH_ST_TABLE(hash);
orighash = table->type;

if (orighash != &identhash) {
VALUE value;
struct reset_hash_type_arg ensure_arg;
struct st_hash_type assochash;

assochash.compare = assoc_cmp;
assochash.hash = orighash->hash;
table->type = &assochash;
args[0] = hash;
args[1] = key;
ensure_arg.hash = hash;
ensure_arg.orighash = orighash;
value = rb_ensure(lookup2_call, (VALUE)&args, reset_hash_type, (VALUE)&ensure_arg);
if (value != Qundef) return rb_assoc_new(key, value);
}

args[0] = key;
args[1] = Qnil;
rb_hash_foreach(hash, assoc_i, (VALUE)args);
return args[1];
}

static int
rassoc_i(VALUE key, VALUE val, VALUE arg)
{
VALUE *args = (VALUE *)arg;

if (RTEST(rb_equal(args[0], val))) {
args[1] = rb_assoc_new(key, val);
return ST_STOP;
}
return ST_CONTINUE;
}














VALUE
rb_hash_rassoc(VALUE hash, VALUE obj)
{
VALUE args[2];

args[0] = obj;
args[1] = Qnil;
rb_hash_foreach(hash, rassoc_i, (VALUE)args);
return args[1];
}

static int
flatten_i(VALUE key, VALUE val, VALUE ary)
{
VALUE pair[2];

pair[0] = key;
pair[1] = val;
rb_ary_cat(ary, pair, 2);

return ST_CONTINUE;
}

















static VALUE
rb_hash_flatten(int argc, VALUE *argv, VALUE hash)
{
VALUE ary;

rb_check_arity(argc, 0, 1);

if (argc) {
int level = NUM2INT(argv[0]);

if (level == 0) return rb_hash_to_a(hash);

ary = rb_ary_new_capa(RHASH_SIZE(hash) * 2);
rb_hash_foreach(hash, flatten_i, ary);
level--;

if (level > 0) {
VALUE ary_flatten_level = INT2FIX(level);
rb_funcallv(ary, id_flatten_bang, 1, &ary_flatten_level);
}
else if (level < 0) {

rb_funcallv(ary, id_flatten_bang, 0, 0);
}
}
else {
ary = rb_ary_new_capa(RHASH_SIZE(hash) * 2);
rb_hash_foreach(hash, flatten_i, ary);
}

return ary;
}

static int
delete_if_nil(VALUE key, VALUE value, VALUE hash)
{
if (NIL_P(value)) {
return ST_DELETE;
}
return ST_CONTINUE;
}

static int
set_if_not_nil(VALUE key, VALUE value, VALUE hash)
{
if (!NIL_P(value)) {
rb_hash_aset(hash, key, value);
}
return ST_CONTINUE;
}













static VALUE
rb_hash_compact(VALUE hash)
{
VALUE result = rb_hash_new();
if (!RHASH_EMPTY_P(hash)) {
rb_hash_foreach(hash, set_if_not_nil, result);
}
return result;
}













static VALUE
rb_hash_compact_bang(VALUE hash)
{
st_index_t n;
rb_hash_modify_check(hash);
n = RHASH_SIZE(hash);
if (n) {
rb_hash_foreach(hash, delete_if_nil, hash);
if (n != RHASH_SIZE(hash))
return hash;
}
return Qnil;
}

static st_table *rb_init_identtable_with_size(st_index_t size);

















static VALUE
rb_hash_compare_by_id(VALUE hash)
{
VALUE tmp;
st_table *identtable;

if (rb_hash_compare_by_id_p(hash)) return hash;

rb_hash_modify_check(hash);
ar_force_convert_table(hash, __FILE__, __LINE__);
HASH_ASSERT(RHASH_ST_TABLE_P(hash));

tmp = hash_alloc(0);
identtable = rb_init_identtable_with_size(RHASH_SIZE(hash));
RHASH_ST_TABLE_SET(tmp, identtable);
rb_hash_foreach(hash, rb_hash_rehash_i, (VALUE)tmp);
st_free_table(RHASH_ST_TABLE(hash));
RHASH_ST_TABLE_SET(hash, identtable);
RHASH_ST_CLEAR(tmp);
rb_gc_force_recycle(tmp);

return hash;
}










MJIT_FUNC_EXPORTED VALUE
rb_hash_compare_by_id_p(VALUE hash)
{
if (RHASH_ST_TABLE_P(hash) && RHASH_ST_TABLE(hash)->type == &identhash) {
return Qtrue;
}
else {
return Qfalse;
}
}

VALUE
rb_ident_hash_new(void)
{
VALUE hash = rb_hash_new();
RHASH_ST_TABLE_SET(hash, st_init_table(&identhash));
return hash;
}

st_table *
rb_init_identtable(void)
{
return st_init_table(&identhash);
}

static st_table *
rb_init_identtable_with_size(st_index_t size)
{
return st_init_table_with_size(&identhash, size);
}

static int
any_p_i(VALUE key, VALUE value, VALUE arg)
{
VALUE ret = rb_yield(rb_assoc_new(key, value));
if (RTEST(ret)) {
*(VALUE *)arg = Qtrue;
return ST_STOP;
}
return ST_CONTINUE;
}

static int
any_p_i_fast(VALUE key, VALUE value, VALUE arg)
{
VALUE ret = rb_yield_values(2, key, value);
if (RTEST(ret)) {
*(VALUE *)arg = Qtrue;
return ST_STOP;
}
return ST_CONTINUE;
}

static int
any_p_i_pattern(VALUE key, VALUE value, VALUE arg)
{
VALUE ret = rb_funcall(((VALUE *)arg)[1], idEqq, 1, rb_assoc_new(key, value));
if (RTEST(ret)) {
*(VALUE *)arg = Qtrue;
return ST_STOP;
}
return ST_CONTINUE;
}









static VALUE
rb_hash_any_p(int argc, VALUE *argv, VALUE hash)
{
VALUE args[2];
args[0] = Qfalse;

rb_check_arity(argc, 0, 1);
if (RHASH_EMPTY_P(hash)) return Qfalse;
if (argc) {
if (rb_block_given_p()) {
rb_warn("given block not used");
}
args[1] = argv[0];

rb_hash_foreach(hash, any_p_i_pattern, (VALUE)args);
}
else {
if (!rb_block_given_p()) {

return Qtrue;
}
if (rb_block_pair_yield_optimizable())
rb_hash_foreach(hash, any_p_i_fast, (VALUE)args);
else
rb_hash_foreach(hash, any_p_i, (VALUE)args);
}
return args[0];
}




















static VALUE
rb_hash_dig(int argc, VALUE *argv, VALUE self)
{
rb_check_arity(argc, 1, UNLIMITED_ARGUMENTS);
self = rb_hash_aref(self, *argv);
if (!--argc) return self;
++argv;
return rb_obj_dig(argc, argv, self, Qnil);
}

static int
hash_le_i(VALUE key, VALUE value, VALUE arg)
{
VALUE *args = (VALUE *)arg;
VALUE v = rb_hash_lookup2(args[0], key, Qundef);
if (v != Qundef && rb_equal(value, v)) return ST_CONTINUE;
args[1] = Qfalse;
return ST_STOP;
}

static VALUE
hash_le(VALUE hash1, VALUE hash2)
{
VALUE args[2];
args[0] = hash2;
args[1] = Qtrue;
rb_hash_foreach(hash1, hash_le_i, (VALUE)args);
return args[1];
}














static VALUE
rb_hash_le(VALUE hash, VALUE other)
{
other = to_hash(other);
if (RHASH_SIZE(hash) > RHASH_SIZE(other)) return Qfalse;
return hash_le(hash, other);
}














static VALUE
rb_hash_lt(VALUE hash, VALUE other)
{
other = to_hash(other);
if (RHASH_SIZE(hash) >= RHASH_SIZE(other)) return Qfalse;
return hash_le(hash, other);
}














static VALUE
rb_hash_ge(VALUE hash, VALUE other)
{
other = to_hash(other);
if (RHASH_SIZE(hash) < RHASH_SIZE(other)) return Qfalse;
return hash_le(other, hash);
}














static VALUE
rb_hash_gt(VALUE hash, VALUE other)
{
other = to_hash(other);
if (RHASH_SIZE(hash) <= RHASH_SIZE(other)) return Qfalse;
return hash_le(other, hash);
}

static VALUE
hash_proc_call(RB_BLOCK_CALL_FUNC_ARGLIST(key, hash))
{
rb_check_arity(argc, 1, 1);
return rb_hash_aref(hash, *argv);
}














static VALUE
rb_hash_to_proc(VALUE hash)
{
return rb_func_proc_new(hash_proc_call, hash);
}

static VALUE
rb_hash_deconstruct_keys(VALUE hash, VALUE keys)
{
return hash;
}

static int
add_new_i(st_data_t *key, st_data_t *val, st_data_t arg, int existing)
{
VALUE *args = (VALUE *)arg;
if (existing) return ST_STOP;
RB_OBJ_WRITTEN(args[0], Qundef, (VALUE)*key);
RB_OBJ_WRITE(args[0], (VALUE *)val, args[1]);
return ST_CONTINUE;
}





int
rb_hash_add_new_element(VALUE hash, VALUE key, VALUE val)
{
st_table *tbl;
int ret = 0;
VALUE args[2];
args[0] = hash;
args[1] = val;

if (RHASH_AR_TABLE_P(hash)) {
hash_ar_table(hash);

ret = ar_update(hash, (st_data_t)key, add_new_i, (st_data_t)args);
if (ret != -1) {
return ret;
}
ar_try_convert_table(hash);
}
tbl = RHASH_TBL_RAW(hash);
return st_update(tbl, (st_data_t)key, add_new_i, (st_data_t)args);

}

static st_data_t
key_stringify(VALUE key)
{
return (rb_obj_class(key) == rb_cString && !RB_OBJ_FROZEN(key)) ?
rb_hash_key_str(key) : key;
}

static void
ar_bulk_insert(VALUE hash, long argc, const VALUE *argv)
{
long i;
for (i = 0; i < argc; ) {
st_data_t k = key_stringify(argv[i++]);
st_data_t v = argv[i++];
ar_insert(hash, k, v);
RB_OBJ_WRITTEN(hash, Qundef, k);
RB_OBJ_WRITTEN(hash, Qundef, v);
}
}

void
rb_hash_bulk_insert(long argc, const VALUE *argv, VALUE hash)
{
HASH_ASSERT(argc % 2 == 0);
if (argc > 0) {
st_index_t size = argc / 2;

if (RHASH_TABLE_NULL_P(hash)) {
if (size <= RHASH_AR_TABLE_MAX_SIZE) {
hash_ar_table(hash);
}
else {
RHASH_TBL_RAW(hash);
}
}

if (RHASH_AR_TABLE_P(hash) &&
(RHASH_AR_TABLE_SIZE(hash) + size <= RHASH_AR_TABLE_MAX_SIZE)) {
ar_bulk_insert(hash, argc, argv);
}
else {
rb_hash_bulk_insert_into_st_table(argc, argv, hash);
}
}
}

static char **origenviron;
#if defined(_WIN32)
#define GET_ENVIRON(e) ((e) = rb_w32_get_environ())
#define FREE_ENVIRON(e) rb_w32_free_environ(e)
static char **my_environ;
#undef environ
#define environ my_environ
#undef getenv
static char *(*w32_getenv)(const char*);
static char *
w32_getenv_unknown(const char *name)
{
char *(*func)(const char*);
if (rb_locale_encindex() == rb_ascii8bit_encindex()) {
func = rb_w32_getenv;
}
else {
func = rb_w32_ugetenv;
}

return (w32_getenv = func)(name);
}
static char *(*w32_getenv)(const char*) = w32_getenv_unknown;
#define getenv(n) w32_getenv(n)
#elif defined(__APPLE__)
#undef environ
#define environ (*_NSGetEnviron())
#define GET_ENVIRON(e) (e)
#define FREE_ENVIRON(e)
#else
extern char **environ;
#define GET_ENVIRON(e) (e)
#define FREE_ENVIRON(e)
#endif
#if defined(ENV_IGNORECASE)
#define ENVMATCH(s1, s2) (STRCASECMP((s1), (s2)) == 0)
#define ENVNMATCH(s1, s2, n) (STRNCASECMP((s1), (s2), (n)) == 0)
#else
#define ENVMATCH(n1, n2) (strcmp((n1), (n2)) == 0)
#define ENVNMATCH(s1, s2, n) (memcmp((s1), (s2), (n)) == 0)
#endif

static VALUE
env_enc_str_new(const char *ptr, long len, rb_encoding *enc)
{
#if defined(_WIN32)
rb_encoding *internal = rb_default_internal_encoding();
const int ecflags = ECONV_INVALID_REPLACE | ECONV_UNDEF_REPLACE;
rb_encoding *utf8 = rb_utf8_encoding();
VALUE str = rb_enc_str_new(NULL, 0, (internal ? internal : enc));
if (NIL_P(rb_str_cat_conv_enc_opts(str, 0, ptr, len, utf8, ecflags, Qnil))) {
rb_str_initialize(str, ptr, len, NULL);
}
#else
VALUE str = rb_external_str_new_with_enc(ptr, len, enc);
#endif

rb_obj_freeze(str);
return str;
}

static VALUE
env_enc_str_new_cstr(const char *ptr, rb_encoding *enc)
{
return env_enc_str_new(ptr, strlen(ptr), enc);
}

static VALUE
env_str_new(const char *ptr, long len)
{
return env_enc_str_new(ptr, len, rb_locale_encoding());
}

static VALUE
env_str_new2(const char *ptr)
{
if (!ptr) return Qnil;
return env_str_new(ptr, strlen(ptr));
}

static const char TZ_ENV[] = "TZ";

static rb_encoding *
env_encoding_for(const char *name, const char *ptr)
{
if (ENVMATCH(name, PATH_ENV)) {
return rb_filesystem_encoding();
}
else {
return rb_locale_encoding();
}
}

static VALUE
env_name_new(const char *name, const char *ptr)
{
return env_enc_str_new_cstr(ptr, env_encoding_for(name, ptr));
}

static void *
get_env_cstr(
#if defined(_WIN32)
volatile VALUE *pstr,
#else
VALUE str,
#endif
const char *name)
{
#if defined(_WIN32)
VALUE str = *pstr;
#endif
char *var;
rb_encoding *enc = rb_enc_get(str);
if (!rb_enc_asciicompat(enc)) {
rb_raise(rb_eArgError, "bad environment variable %s: ASCII incompatible encoding: %s",
name, rb_enc_name(enc));
}
#if defined(_WIN32)
if (!rb_enc_str_asciionly_p(str)) {
*pstr = str = rb_str_conv_enc(str, NULL, rb_utf8_encoding());
}
#endif
var = RSTRING_PTR(str);
if (memchr(var, '\0', RSTRING_LEN(str))) {
rb_raise(rb_eArgError, "bad environment variable %s: contains null byte", name);
}
return rb_str_fill_terminator(str, 1); 
}

#if defined(_WIN32)
#define get_env_ptr(var, val) (var = get_env_cstr(&(val), #var))

#else
#define get_env_ptr(var, val) (var = get_env_cstr(val, #var))

#endif

static inline const char *
env_name(volatile VALUE *s)
{
const char *name;
SafeStringValue(*s);
get_env_ptr(name, *s);
return name;
}

#define env_name(s) env_name(&(s))

static VALUE env_aset(VALUE nm, VALUE val);

static VALUE
env_delete(VALUE name)
{
const char *nam, *val;

nam = env_name(name);
val = getenv(nam);







if (ENVMATCH(nam, TZ_ENV)) {
ruby_reset_timezone();
}

if (val) {
VALUE value = env_str_new2(val);

ruby_setenv(nam, 0);
if (ENVMATCH(nam, PATH_ENV)) {
RB_GC_GUARD(name);
}
return value;
}
return Qnil;
}






















static VALUE
env_delete_m(VALUE obj, VALUE name)
{
VALUE val;

val = env_delete(name);
if (NIL_P(val) && rb_block_given_p()) rb_yield(name);
return val;
}














static VALUE
rb_f_getenv(VALUE obj, VALUE name)
{
const char *nam, *env;

nam = env_name(name);
env = getenv(nam);
if (env) {
return env_name_new(nam, env);
}
return Qnil;
}


























static VALUE
env_fetch(int argc, VALUE *argv, VALUE _)
{
VALUE key;
long block_given;
const char *nam, *env;

rb_check_arity(argc, 1, 2);
key = argv[0];
block_given = rb_block_given_p();
if (block_given && argc == 2) {
rb_warn("block supersedes default value argument");
}
nam = env_name(key);
env = getenv(nam);
if (!env) {
if (block_given) return rb_yield(key);
if (argc == 1) {
rb_key_err_raise(rb_sprintf("key not found: \"%"PRIsVALUE"\"", key), envtbl, key);
}
return argv[1];
}
return env_name_new(nam, env);
}

int
rb_env_path_tainted(void)
{
rb_warn_deprecated_to_remove("rb_env_path_tainted", "3.2");
return 0;
}

#if defined(_WIN32) || (defined(HAVE_SETENV) && defined(HAVE_UNSETENV))
#elif defined __sun
static int
in_origenv(const char *str)
{
char **env;
for (env = origenviron; *env; ++env) {
if (*env == str) return 1;
}
return 0;
}
#else
static int
envix(const char *nam)
{
register int i, len = strlen(nam);
char **env;

env = GET_ENVIRON(environ);
for (i = 0; env[i]; i++) {
if (ENVNMATCH(env[i],nam,len) && env[i][len] == '=')
break; 
} 
FREE_ENVIRON(environ);
return i;
}
#endif

#if defined(_WIN32)
static size_t
getenvsize(const WCHAR* p)
{
const WCHAR* porg = p;
while (*p++) p += lstrlenW(p) + 1;
return p - porg + 1;
}

static size_t
getenvblocksize(void)
{
#if defined(_MAX_ENV)
return _MAX_ENV;
#else
return 32767;
#endif
}

static int
check_envsize(size_t n)
{
if (_WIN32_WINNT < 0x0600 && rb_w32_osver() < 6) {



WCHAR* p = GetEnvironmentStringsW();
if (!p) return -1; 
n += getenvsize(p);
FreeEnvironmentStringsW(p);
if (n >= getenvblocksize()) {
return -1;
}
}
return 0;
}
#endif

#if defined(_WIN32) || (defined(__sun) && !(defined(HAVE_SETENV) && defined(HAVE_UNSETENV)))


NORETURN(static void invalid_envname(const char *name));

static void
invalid_envname(const char *name)
{
rb_syserr_fail_str(EINVAL, rb_sprintf("ruby_setenv(%s)", name));
}

static const char *
check_envname(const char *name)
{
if (strchr(name, '=')) {
invalid_envname(name);
}
return name;
}
#endif

void
ruby_setenv(const char *name, const char *value)
{
#if defined(_WIN32)
#if defined(MINGW_HAS_SECURE_API) || RUBY_MSVCRT_VERSION >= 80
#define HAVE__WPUTENV_S 1
#endif
VALUE buf;
WCHAR *wname;
WCHAR *wvalue = 0;
int failed = 0;
int len;
check_envname(name);
len = MultiByteToWideChar(CP_UTF8, 0, name, -1, NULL, 0);
if (value) {
int len2;
len2 = MultiByteToWideChar(CP_UTF8, 0, value, -1, NULL, 0);
if (check_envsize((size_t)len + len2)) { 
goto fail; 
}
wname = ALLOCV_N(WCHAR, buf, len + len2);
wvalue = wname + len;
MultiByteToWideChar(CP_UTF8, 0, name, -1, wname, len);
MultiByteToWideChar(CP_UTF8, 0, value, -1, wvalue, len2);
#if !defined(HAVE__WPUTENV_S)
wname[len-1] = L'=';
#endif
}
else {
wname = ALLOCV_N(WCHAR, buf, len + 1);
MultiByteToWideChar(CP_UTF8, 0, name, -1, wname, len);
wvalue = wname + len;
*wvalue = L'\0';
#if !defined(HAVE__WPUTENV_S)
wname[len-1] = L'=';
#endif
}
#if !defined(HAVE__WPUTENV_S)
failed = _wputenv(wname);
#else
failed = _wputenv_s(wname, wvalue);
#endif
ALLOCV_END(buf);


if (!value || !*value) {

if (!SetEnvironmentVariable(name, value) &&
GetLastError() != ERROR_ENVVAR_NOT_FOUND) goto fail;
}
if (failed) {
fail:
invalid_envname(name);
}
#elif defined(HAVE_SETENV) && defined(HAVE_UNSETENV)
if (value) {
if (setenv(name, value, 1))
rb_sys_fail_str(rb_sprintf("setenv(%s)", name));
}
else {
#if defined(VOID_UNSETENV)
unsetenv(name);
#else
if (unsetenv(name))
rb_sys_fail_str(rb_sprintf("unsetenv(%s)", name));
#endif
}
#elif defined __sun




size_t len, mem_size;
char **env_ptr, *str, *mem_ptr;

check_envname(name);
len = strlen(name);
if (value) {
mem_size = len + strlen(value) + 2;
mem_ptr = malloc(mem_size);
if (mem_ptr == NULL)
rb_sys_fail_str(rb_sprintf("malloc("PRIuSIZE")", mem_size));
snprintf(mem_ptr, mem_size, "%s=%s", name, value);
}
for (env_ptr = GET_ENVIRON(environ); (str = *env_ptr) != 0; ++env_ptr) {
if (!strncmp(str, name, len) && str[len] == '=') {
if (!in_origenv(str)) free(str);
while ((env_ptr[0] = env_ptr[1]) != 0) env_ptr++;
break;
}
}
if (value) {
if (putenv(mem_ptr)) {
free(mem_ptr);
rb_sys_fail_str(rb_sprintf("putenv(%s)", name));
}
}
#else 
size_t len;
int i;

i=envix(name); 

if (environ == origenviron) { 
int j;
int max;
char **tmpenv;

for (max = i; environ[max]; max++) ;
tmpenv = ALLOC_N(char*, max+2);
for (j=0; j<max; j++) 
tmpenv[j] = ruby_strdup(environ[j]);
tmpenv[max] = 0;
environ = tmpenv; 
}
if (environ[i]) {
char **envp = origenviron;
while (*envp && *envp != environ[i]) envp++;
if (!*envp)
xfree(environ[i]);
if (!value) {
while (environ[i]) {
environ[i] = environ[i+1];
i++;
}
return;
}
}
else { 
if (!value) return;
REALLOC_N(environ, char*, i+2); 
environ[i+1] = 0; 
}
len = strlen(name) + strlen(value) + 2;
environ[i] = ALLOC_N(char, len);
snprintf(environ[i],len,"%s=%s",name,value); 
#endif 
}

void
ruby_unsetenv(const char *name)
{
ruby_setenv(name, 0);
}













































static VALUE
env_aset_m(VALUE obj, VALUE nm, VALUE val)
{
return env_aset(nm, val);
}

static VALUE
env_aset(VALUE nm, VALUE val)
{
char *name, *value;

if (NIL_P(val)) {
env_delete(nm);
return Qnil;
}
SafeStringValue(nm);
SafeStringValue(val);


get_env_ptr(name, nm);
get_env_ptr(value, val);

ruby_setenv(name, value);
if (ENVMATCH(name, PATH_ENV)) {
RB_GC_GUARD(nm);
}
else if (ENVMATCH(name, TZ_ENV)) {
ruby_reset_timezone();
}
return val;
}

static VALUE
env_keys(void)
{
char **env;
VALUE ary;

ary = rb_ary_new();
env = GET_ENVIRON(environ);
while (*env) {
char *s = strchr(*env, '=');
if (s) {
rb_ary_push(ary, env_str_new(*env, s-*env));
}
env++;
}
FREE_ENVIRON(environ);
return ary;
}
















static VALUE
env_f_keys(VALUE _)
{
return env_keys();
}

static VALUE
rb_env_size(VALUE ehash, VALUE args, VALUE eobj)
{
char **env;
long cnt = 0;

env = GET_ENVIRON(environ);
for (; *env ; ++env) {
if (strchr(*env, '=')) {
cnt++;
}
}
FREE_ENVIRON(environ);
return LONG2FIX(cnt);
}


















static VALUE
env_each_key(VALUE ehash)
{
VALUE keys;
long i;

RETURN_SIZED_ENUMERATOR(ehash, 0, 0, rb_env_size);
keys = env_keys();
for (i=0; i<RARRAY_LEN(keys); i++) {
rb_yield(RARRAY_AREF(keys, i));
}
return ehash;
}

static VALUE
env_values(void)
{
VALUE ary;
char **env;

ary = rb_ary_new();
env = GET_ENVIRON(environ);
while (*env) {
char *s = strchr(*env, '=');
if (s) {
rb_ary_push(ary, env_str_new2(s+1));
}
env++;
}
FREE_ENVIRON(environ);
return ary;
}















static VALUE
env_f_values(VALUE _)
{
return env_values();
}


















static VALUE
env_each_value(VALUE ehash)
{
VALUE values;
long i;

RETURN_SIZED_ENUMERATOR(ehash, 0, 0, rb_env_size);
values = env_values();
for (i=0; i<RARRAY_LEN(values); i++) {
rb_yield(RARRAY_AREF(values, i));
}
return ehash;
}



















static VALUE
env_each_pair(VALUE ehash)
{
char **env;
VALUE ary;
long i;

RETURN_SIZED_ENUMERATOR(ehash, 0, 0, rb_env_size);

ary = rb_ary_new();
env = GET_ENVIRON(environ);
while (*env) {
char *s = strchr(*env, '=');
if (s) {
rb_ary_push(ary, env_str_new(*env, s-*env));
rb_ary_push(ary, env_str_new2(s+1));
}
env++;
}
FREE_ENVIRON(environ);

if (rb_block_pair_yield_optimizable()) {
for (i=0; i<RARRAY_LEN(ary); i+=2) {
rb_yield_values(2, RARRAY_AREF(ary, i), RARRAY_AREF(ary, i+1));
}
}
else {
for (i=0; i<RARRAY_LEN(ary); i+=2) {
rb_yield(rb_assoc_new(RARRAY_AREF(ary, i), RARRAY_AREF(ary, i+1)));
}
}
return ehash;
}























static VALUE
env_reject_bang(VALUE ehash)
{
VALUE keys;
long i;
int del = 0;

RETURN_SIZED_ENUMERATOR(ehash, 0, 0, rb_env_size);
keys = env_keys();
RBASIC_CLEAR_CLASS(keys);
for (i=0; i<RARRAY_LEN(keys); i++) {
VALUE val = rb_f_getenv(Qnil, RARRAY_AREF(keys, i));
if (!NIL_P(val)) {
if (RTEST(rb_yield_values(2, RARRAY_AREF(keys, i), val))) {
env_delete(RARRAY_AREF(keys, i));
del++;
}
}
}
RB_GC_GUARD(keys);
if (del == 0) return Qnil;
return envtbl;
}





















static VALUE
env_delete_if(VALUE ehash)
{
RETURN_SIZED_ENUMERATOR(ehash, 0, 0, rb_env_size);
env_reject_bang(ehash);
return envtbl;
}



















static VALUE
env_values_at(int argc, VALUE *argv, VALUE _)
{
VALUE result;
long i;

result = rb_ary_new();
for (i=0; i<argc; i++) {
rb_ary_push(result, rb_f_getenv(Qnil, argv[i]));
}
return result;
}






















static VALUE
env_select(VALUE ehash)
{
VALUE result;
VALUE keys;
long i;

RETURN_SIZED_ENUMERATOR(ehash, 0, 0, rb_env_size);
result = rb_hash_new();
keys = env_keys();
for (i = 0; i < RARRAY_LEN(keys); ++i) {
VALUE key = RARRAY_AREF(keys, i);
VALUE val = rb_f_getenv(Qnil, key);
if (!NIL_P(val)) {
if (RTEST(rb_yield_values(2, key, val))) {
rb_hash_aset(result, key, val);
}
}
}
RB_GC_GUARD(keys);

return result;
}






































static VALUE
env_select_bang(VALUE ehash)
{
VALUE keys;
long i;
int del = 0;

RETURN_SIZED_ENUMERATOR(ehash, 0, 0, rb_env_size);
keys = env_keys();
RBASIC_CLEAR_CLASS(keys);
for (i=0; i<RARRAY_LEN(keys); i++) {
VALUE val = rb_f_getenv(Qnil, RARRAY_AREF(keys, i));
if (!NIL_P(val)) {
if (!RTEST(rb_yield_values(2, RARRAY_AREF(keys, i), val))) {
env_delete(RARRAY_AREF(keys, i));
del++;
}
}
}
RB_GC_GUARD(keys);
if (del == 0) return Qnil;
return envtbl;
}



















static VALUE
env_keep_if(VALUE ehash)
{
RETURN_SIZED_ENUMERATOR(ehash, 0, 0, rb_env_size);
env_select_bang(ehash);
return envtbl;
}













static VALUE
env_slice(int argc, VALUE *argv, VALUE _)
{
int i;
VALUE key, value, result;

if (argc == 0) {
return rb_hash_new();
}
result = rb_hash_new_with_size(argc);

for (i = 0; i < argc; i++) {
key = argv[i];
value = rb_f_getenv(Qnil, key);
if (value != Qnil)
rb_hash_aset(result, key, value);
}

return result;
}

VALUE
rb_env_clear(void)
{
VALUE keys;
long i;

keys = env_keys();
for (i=0; i<RARRAY_LEN(keys); i++) {
VALUE val = rb_f_getenv(Qnil, RARRAY_AREF(keys, i));
if (!NIL_P(val)) {
env_delete(RARRAY_AREF(keys, i));
}
}
RB_GC_GUARD(keys);
return envtbl;
}











static VALUE
env_clear(VALUE _)
{
return rb_env_clear();
}








static VALUE
env_to_s(VALUE _)
{
return rb_usascii_str_new2("ENV");
}









static VALUE
env_inspect(VALUE _)
{
char **env;
VALUE str, i;

str = rb_str_buf_new2("{");
env = GET_ENVIRON(environ);
while (*env) {
char *s = strchr(*env, '=');

if (env != environ) {
rb_str_buf_cat2(str, ", ");
}
if (s) {
rb_str_buf_cat2(str, "\"");
rb_str_buf_cat(str, *env, s-*env);
rb_str_buf_cat2(str, "\"=>");
i = rb_inspect(rb_str_new2(s+1));
rb_str_buf_append(str, i);
}
env++;
}
FREE_ENVIRON(environ);
rb_str_buf_cat2(str, "}");

return str;
}










static VALUE
env_to_a(VALUE _)
{
char **env;
VALUE ary;

ary = rb_ary_new();
env = GET_ENVIRON(environ);
while (*env) {
char *s = strchr(*env, '=');
if (s) {
rb_ary_push(ary, rb_assoc_new(env_str_new(*env, s-*env),
env_str_new2(s+1)));
}
env++;
}
FREE_ENVIRON(environ);
return ary;
}












static VALUE
env_none(VALUE _)
{
return Qnil;
}











static VALUE
env_size(VALUE _)
{
int i;
char **env;

env = GET_ENVIRON(environ);
for (i=0; env[i]; i++)
;
FREE_ENVIRON(environ);
return INT2FIX(i);
}











static VALUE
env_empty_p(VALUE _)
{
char **env;

env = GET_ENVIRON(environ);
if (env[0] == 0) {
FREE_ENVIRON(environ);
return Qtrue;
}
FREE_ENVIRON(environ);
return Qfalse;
}


























static VALUE
env_has_key(VALUE env, VALUE key)
{
const char *s;

s = env_name(key);
if (getenv(s)) return Qtrue;
return Qfalse;
}






















static VALUE
env_assoc(VALUE env, VALUE key)
{
const char *s, *e;

s = env_name(key);
e = getenv(s);
if (e) return rb_assoc_new(key, env_str_new2(e));
return Qnil;
}













static VALUE
env_has_value(VALUE dmy, VALUE obj)
{
char **env;

obj = rb_check_string_type(obj);
if (NIL_P(obj)) return Qnil;
env = GET_ENVIRON(environ);
while (*env) {
char *s = strchr(*env, '=');
if (s++) {
long len = strlen(s);
if (RSTRING_LEN(obj) == len && strncmp(s, RSTRING_PTR(obj), len) == 0) {
FREE_ENVIRON(environ);
return Qtrue;
}
}
env++;
}
FREE_ENVIRON(environ);
return Qfalse;
}





















static VALUE
env_rassoc(VALUE dmy, VALUE obj)
{
char **env;

obj = rb_check_string_type(obj);
if (NIL_P(obj)) return Qnil;
env = GET_ENVIRON(environ);
while (*env) {
char *s = strchr(*env, '=');
if (s++) {
long len = strlen(s);
if (RSTRING_LEN(obj) == len && strncmp(s, RSTRING_PTR(obj), len) == 0) {
VALUE result = rb_assoc_new(rb_str_new(*env, s-*env-1), obj);
FREE_ENVIRON(environ);
return result;
}
}
env++;
}
FREE_ENVIRON(environ);
return Qnil;
}

















static VALUE
env_key(VALUE dmy, VALUE value)
{
char **env;
VALUE str;

SafeStringValue(value);
env = GET_ENVIRON(environ);
while (*env) {
char *s = strchr(*env, '=');
if (s++) {
long len = strlen(s);
if (RSTRING_LEN(value) == len && strncmp(s, RSTRING_PTR(value), len) == 0) {
str = env_str_new(*env, s-*env-1);
FREE_ENVIRON(environ);
return str;
}
}
env++;
}
FREE_ENVIRON(environ);
return Qnil;
}







static VALUE
env_index(VALUE dmy, VALUE value)
{
rb_warn_deprecated("ENV.index", "ENV.key");
return env_key(dmy, value);
}

static VALUE
env_to_hash(void)
{
char **env;
VALUE hash;

hash = rb_hash_new();
env = GET_ENVIRON(environ);
while (*env) {
char *s = strchr(*env, '=');
if (s) {
rb_hash_aset(hash, env_str_new(*env, s-*env),
env_str_new2(s+1));
}
env++;
}
FREE_ENVIRON(environ);
return hash;
}










static VALUE
env_f_to_hash(VALUE _)
{
return env_to_hash();
}



















static VALUE
env_to_h(VALUE _)
{
VALUE hash = env_to_hash();
if (rb_block_given_p()) {
hash = rb_hash_to_h_block(hash);
}
return hash;
}
















static VALUE
env_reject(VALUE _)
{
return rb_hash_delete_if(env_to_hash());
}








static VALUE
env_freeze(VALUE self)
{
rb_raise(rb_eTypeError, "cannot freeze ENV");
return self; 
}


















static VALUE
env_shift(VALUE _)
{
char **env;
VALUE result = Qnil;

env = GET_ENVIRON(environ);
if (*env) {
char *s = strchr(*env, '=');
if (s) {
VALUE key = env_str_new(*env, s-*env);
VALUE val = env_str_new2(getenv(RSTRING_PTR(key)));
env_delete(key);
result = rb_assoc_new(key, val);
}
}
FREE_ENVIRON(environ);
return result;
}
















static VALUE
env_invert(VALUE _)
{
return rb_hash_invert(env_to_hash());
}

static int
env_replace_i(VALUE key, VALUE val, VALUE keys)
{
env_aset(key, val);
if (rb_ary_includes(keys, key)) {
rb_ary_delete(keys, key);
}
return ST_CONTINUE;
}



















static VALUE
env_replace(VALUE env, VALUE hash)
{
VALUE keys;
long i;

keys = env_keys();
if (env == hash) return env;
hash = to_hash(hash);
rb_hash_foreach(hash, env_replace_i, keys);

for (i=0; i<RARRAY_LEN(keys); i++) {
env_delete(RARRAY_AREF(keys, i));
}
RB_GC_GUARD(keys);
return env;
}

static int
env_update_i(VALUE key, VALUE val, VALUE _)
{
env_aset(key, val);
return ST_CONTINUE;
}

static int
env_update_block_i(VALUE key, VALUE val, VALUE _)
{
VALUE oldval = rb_f_getenv(Qnil, key);
if (!NIL_P(oldval)) {
val = rb_yield_values(3, key, oldval, val);
}
env_aset(key, val);
return ST_CONTINUE;
}





































static VALUE
env_update(VALUE env, VALUE hash)
{
if (env == hash) return env;
hash = to_hash(hash);
rb_foreach_func *func = rb_block_given_p() ?
env_update_block_i : env_update_i;
rb_hash_foreach(hash, func, 0);
return env;
}























































































































































































































void
Init_Hash(void)
{
#undef rb_intern
#define rb_intern(str) rb_intern_const(str)
id_hash = rb_intern("hash");
id_default = rb_intern("default");
id_flatten_bang = rb_intern("flatten!");
id_hash_iter_lev = rb_make_internal_id();

rb_cHash = rb_define_class("Hash", rb_cObject);

rb_include_module(rb_cHash, rb_mEnumerable);

rb_define_alloc_func(rb_cHash, empty_hash_alloc);
rb_define_singleton_method(rb_cHash, "[]", rb_hash_s_create, -1);
rb_define_singleton_method(rb_cHash, "try_convert", rb_hash_s_try_convert, 1);
rb_define_method(rb_cHash, "initialize", rb_hash_initialize, -1);
rb_define_method(rb_cHash, "initialize_copy", rb_hash_replace, 1);
rb_define_method(rb_cHash, "rehash", rb_hash_rehash, 0);

rb_define_method(rb_cHash, "to_hash", rb_hash_to_hash, 0);
rb_define_method(rb_cHash, "to_h", rb_hash_to_h, 0);
rb_define_method(rb_cHash, "to_a", rb_hash_to_a, 0);
rb_define_method(rb_cHash, "inspect", rb_hash_inspect, 0);
rb_define_alias(rb_cHash, "to_s", "inspect");
rb_define_method(rb_cHash, "to_proc", rb_hash_to_proc, 0);

rb_define_method(rb_cHash, "==", rb_hash_equal, 1);
rb_define_method(rb_cHash, "[]", rb_hash_aref, 1);
rb_define_method(rb_cHash, "hash", rb_hash_hash, 0);
rb_define_method(rb_cHash, "eql?", rb_hash_eql, 1);
rb_define_method(rb_cHash, "fetch", rb_hash_fetch_m, -1);
rb_define_method(rb_cHash, "[]=", rb_hash_aset, 2);
rb_define_method(rb_cHash, "store", rb_hash_aset, 2);
rb_define_method(rb_cHash, "default", rb_hash_default, -1);
rb_define_method(rb_cHash, "default=", rb_hash_set_default, 1);
rb_define_method(rb_cHash, "default_proc", rb_hash_default_proc, 0);
rb_define_method(rb_cHash, "default_proc=", rb_hash_set_default_proc, 1);
rb_define_method(rb_cHash, "key", rb_hash_key, 1);
rb_define_method(rb_cHash, "index", rb_hash_index, 1);
rb_define_method(rb_cHash, "size", rb_hash_size, 0);
rb_define_method(rb_cHash, "length", rb_hash_size, 0);
rb_define_method(rb_cHash, "empty?", rb_hash_empty_p, 0);

rb_define_method(rb_cHash, "each_value", rb_hash_each_value, 0);
rb_define_method(rb_cHash, "each_key", rb_hash_each_key, 0);
rb_define_method(rb_cHash, "each_pair", rb_hash_each_pair, 0);
rb_define_method(rb_cHash, "each", rb_hash_each_pair, 0);

rb_define_method(rb_cHash, "transform_keys", rb_hash_transform_keys, -1);
rb_define_method(rb_cHash, "transform_keys!", rb_hash_transform_keys_bang, -1);
rb_define_method(rb_cHash, "transform_values", rb_hash_transform_values, 0);
rb_define_method(rb_cHash, "transform_values!", rb_hash_transform_values_bang, 0);

rb_define_method(rb_cHash, "keys", rb_hash_keys, 0);
rb_define_method(rb_cHash, "values", rb_hash_values, 0);
rb_define_method(rb_cHash, "values_at", rb_hash_values_at, -1);
rb_define_method(rb_cHash, "fetch_values", rb_hash_fetch_values, -1);

rb_define_method(rb_cHash, "shift", rb_hash_shift, 0);
rb_define_method(rb_cHash, "delete", rb_hash_delete_m, 1);
rb_define_method(rb_cHash, "delete_if", rb_hash_delete_if, 0);
rb_define_method(rb_cHash, "keep_if", rb_hash_keep_if, 0);
rb_define_method(rb_cHash, "select", rb_hash_select, 0);
rb_define_method(rb_cHash, "select!", rb_hash_select_bang, 0);
rb_define_method(rb_cHash, "filter", rb_hash_select, 0);
rb_define_method(rb_cHash, "filter!", rb_hash_select_bang, 0);
rb_define_method(rb_cHash, "reject", rb_hash_reject, 0);
rb_define_method(rb_cHash, "reject!", rb_hash_reject_bang, 0);
rb_define_method(rb_cHash, "slice", rb_hash_slice, -1);
rb_define_method(rb_cHash, "clear", rb_hash_clear, 0);
rb_define_method(rb_cHash, "invert", rb_hash_invert, 0);
rb_define_method(rb_cHash, "update", rb_hash_update, -1);
rb_define_method(rb_cHash, "replace", rb_hash_replace, 1);
rb_define_method(rb_cHash, "merge!", rb_hash_update, -1);
rb_define_method(rb_cHash, "merge", rb_hash_merge, -1);
rb_define_method(rb_cHash, "assoc", rb_hash_assoc, 1);
rb_define_method(rb_cHash, "rassoc", rb_hash_rassoc, 1);
rb_define_method(rb_cHash, "flatten", rb_hash_flatten, -1);
rb_define_method(rb_cHash, "compact", rb_hash_compact, 0);
rb_define_method(rb_cHash, "compact!", rb_hash_compact_bang, 0);

rb_define_method(rb_cHash, "include?", rb_hash_has_key, 1);
rb_define_method(rb_cHash, "member?", rb_hash_has_key, 1);
rb_define_method(rb_cHash, "has_key?", rb_hash_has_key, 1);
rb_define_method(rb_cHash, "has_value?", rb_hash_has_value, 1);
rb_define_method(rb_cHash, "key?", rb_hash_has_key, 1);
rb_define_method(rb_cHash, "value?", rb_hash_has_value, 1);

rb_define_method(rb_cHash, "compare_by_identity", rb_hash_compare_by_id, 0);
rb_define_method(rb_cHash, "compare_by_identity?", rb_hash_compare_by_id_p, 0);

rb_define_method(rb_cHash, "any?", rb_hash_any_p, -1);
rb_define_method(rb_cHash, "dig", rb_hash_dig, -1);

rb_define_method(rb_cHash, "<=", rb_hash_le, 1);
rb_define_method(rb_cHash, "<", rb_hash_lt, 1);
rb_define_method(rb_cHash, ">=", rb_hash_ge, 1);
rb_define_method(rb_cHash, ">", rb_hash_gt, 1);

rb_define_method(rb_cHash, "deconstruct_keys", rb_hash_deconstruct_keys, 1);

rb_define_singleton_method(rb_cHash, "ruby2_keywords_hash?", rb_hash_s_ruby2_keywords_hash_p, 1);
rb_define_singleton_method(rb_cHash, "ruby2_keywords_hash", rb_hash_s_ruby2_keywords_hash, 1);





















































































origenviron = environ;
envtbl = rb_obj_alloc(rb_cObject);
rb_extend_object(envtbl, rb_mEnumerable);

rb_define_singleton_method(envtbl, "[]", rb_f_getenv, 1);
rb_define_singleton_method(envtbl, "fetch", env_fetch, -1);
rb_define_singleton_method(envtbl, "[]=", env_aset_m, 2);
rb_define_singleton_method(envtbl, "store", env_aset_m, 2);
rb_define_singleton_method(envtbl, "each", env_each_pair, 0);
rb_define_singleton_method(envtbl, "each_pair", env_each_pair, 0);
rb_define_singleton_method(envtbl, "each_key", env_each_key, 0);
rb_define_singleton_method(envtbl, "each_value", env_each_value, 0);
rb_define_singleton_method(envtbl, "delete", env_delete_m, 1);
rb_define_singleton_method(envtbl, "delete_if", env_delete_if, 0);
rb_define_singleton_method(envtbl, "keep_if", env_keep_if, 0);
rb_define_singleton_method(envtbl, "slice", env_slice, -1);
rb_define_singleton_method(envtbl, "clear", env_clear, 0);
rb_define_singleton_method(envtbl, "reject", env_reject, 0);
rb_define_singleton_method(envtbl, "reject!", env_reject_bang, 0);
rb_define_singleton_method(envtbl, "select", env_select, 0);
rb_define_singleton_method(envtbl, "select!", env_select_bang, 0);
rb_define_singleton_method(envtbl, "filter", env_select, 0);
rb_define_singleton_method(envtbl, "filter!", env_select_bang, 0);
rb_define_singleton_method(envtbl, "shift", env_shift, 0);
rb_define_singleton_method(envtbl, "freeze", env_freeze, 0);
rb_define_singleton_method(envtbl, "invert", env_invert, 0);
rb_define_singleton_method(envtbl, "replace", env_replace, 1);
rb_define_singleton_method(envtbl, "update", env_update, 1);
rb_define_singleton_method(envtbl, "merge!", env_update, 1);
rb_define_singleton_method(envtbl, "inspect", env_inspect, 0);
rb_define_singleton_method(envtbl, "rehash", env_none, 0);
rb_define_singleton_method(envtbl, "to_a", env_to_a, 0);
rb_define_singleton_method(envtbl, "to_s", env_to_s, 0);
rb_define_singleton_method(envtbl, "key", env_key, 1);
rb_define_singleton_method(envtbl, "index", env_index, 1);
rb_define_singleton_method(envtbl, "size", env_size, 0);
rb_define_singleton_method(envtbl, "length", env_size, 0);
rb_define_singleton_method(envtbl, "empty?", env_empty_p, 0);
rb_define_singleton_method(envtbl, "keys", env_f_keys, 0);
rb_define_singleton_method(envtbl, "values", env_f_values, 0);
rb_define_singleton_method(envtbl, "values_at", env_values_at, -1);
rb_define_singleton_method(envtbl, "include?", env_has_key, 1);
rb_define_singleton_method(envtbl, "member?", env_has_key, 1);
rb_define_singleton_method(envtbl, "has_key?", env_has_key, 1);
rb_define_singleton_method(envtbl, "has_value?", env_has_value, 1);
rb_define_singleton_method(envtbl, "key?", env_has_key, 1);
rb_define_singleton_method(envtbl, "value?", env_has_value, 1);
rb_define_singleton_method(envtbl, "to_hash", env_f_to_hash, 0);
rb_define_singleton_method(envtbl, "to_h", env_to_h, 0);
rb_define_singleton_method(envtbl, "assoc", env_assoc, 1);
rb_define_singleton_method(envtbl, "rassoc", env_rassoc, 1);






rb_define_global_const("ENV", envtbl);


ruby_register_rollback_func_for_ensure(hash_foreach_ensure, hash_foreach_ensure_rollback);

HASH_ASSERT(sizeof(ar_hint_t) * RHASH_AR_TABLE_MAX_SIZE == sizeof(VALUE));
}
