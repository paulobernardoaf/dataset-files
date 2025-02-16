#include "debug_counter.h"
#include "id.h"
#include "internal.h"
#include "internal/array.h"
#include "internal/compar.h"
#include "internal/enum.h"
#include "internal/gc.h"
#include "internal/hash.h"
#include "internal/numeric.h"
#include "internal/object.h"
#include "internal/proc.h"
#include "internal/rational.h"
#include "internal/vm.h"
#include "probes.h"
#include "ruby/encoding.h"
#include "ruby/st.h"
#include "ruby/util.h"
#include "transient_heap.h"
#include "builtin.h"
#if !ARRAY_DEBUG
#define NDEBUG
#endif
#include "ruby_assert.h"
VALUE rb_cArray;
#define id_cmp idCmp
#define ARY_DEFAULT_SIZE 16
#define ARY_MAX_SIZE (LONG_MAX / (int)sizeof(VALUE))
#define SMALL_ARRAY_LEN 16
static int
should_be_T_ARRAY(VALUE ary)
{
return RB_TYPE_P(ary, T_ARRAY);
}
static int
should_not_be_shared_and_embedded(VALUE ary)
{
return !FL_TEST((ary), ELTS_SHARED) || !FL_TEST((ary), RARRAY_EMBED_FLAG);
}
#define ARY_SHARED_P(ary) (assert(should_be_T_ARRAY((VALUE)(ary))), assert(should_not_be_shared_and_embedded((VALUE)ary)), FL_TEST_RAW((ary),ELTS_SHARED)!=0)
#define ARY_EMBED_P(ary) (assert(should_be_T_ARRAY((VALUE)(ary))), assert(should_not_be_shared_and_embedded((VALUE)ary)), FL_TEST_RAW((ary), RARRAY_EMBED_FLAG) != 0)
#define ARY_HEAP_PTR(a) (assert(!ARY_EMBED_P(a)), RARRAY(a)->as.heap.ptr)
#define ARY_HEAP_LEN(a) (assert(!ARY_EMBED_P(a)), RARRAY(a)->as.heap.len)
#define ARY_HEAP_CAPA(a) (assert(!ARY_EMBED_P(a)), assert(!ARY_SHARED_ROOT_P(a)), RARRAY(a)->as.heap.aux.capa)
#define ARY_EMBED_PTR(a) (assert(ARY_EMBED_P(a)), RARRAY(a)->as.ary)
#define ARY_EMBED_LEN(a) (assert(ARY_EMBED_P(a)), (long)((RBASIC(a)->flags >> RARRAY_EMBED_LEN_SHIFT) & (RARRAY_EMBED_LEN_MASK >> RARRAY_EMBED_LEN_SHIFT)))
#define ARY_HEAP_SIZE(a) (assert(!ARY_EMBED_P(a)), assert(ARY_OWNS_HEAP_P(a)), ARY_CAPA(a) * sizeof(VALUE))
#define ARY_OWNS_HEAP_P(a) (assert(should_be_T_ARRAY((VALUE)(a))), !FL_TEST_RAW((a), ELTS_SHARED|RARRAY_EMBED_FLAG))
#define FL_SET_EMBED(a) do { assert(!ARY_SHARED_P(a)); FL_SET((a), RARRAY_EMBED_FLAG); RARY_TRANSIENT_UNSET(a); ary_verify(a); } while (0)
#define FL_UNSET_EMBED(ary) FL_UNSET((ary), RARRAY_EMBED_FLAG|RARRAY_EMBED_LEN_MASK)
#define FL_SET_SHARED(ary) do { assert(!ARY_EMBED_P(ary)); FL_SET((ary), ELTS_SHARED); } while (0)
#define FL_UNSET_SHARED(ary) FL_UNSET((ary), ELTS_SHARED)
#define ARY_SET_PTR(ary, p) do { assert(!ARY_EMBED_P(ary)); assert(!OBJ_FROZEN(ary)); RARRAY(ary)->as.heap.ptr = (p); } while (0)
#define ARY_SET_EMBED_LEN(ary, n) do { long tmp_n = (n); assert(ARY_EMBED_P(ary)); assert(!OBJ_FROZEN(ary)); RBASIC(ary)->flags &= ~RARRAY_EMBED_LEN_MASK; RBASIC(ary)->flags |= (tmp_n) << RARRAY_EMBED_LEN_SHIFT; } while (0)
#define ARY_SET_HEAP_LEN(ary, n) do { assert(!ARY_EMBED_P(ary)); RARRAY(ary)->as.heap.len = (n); } while (0)
#define ARY_SET_LEN(ary, n) do { if (ARY_EMBED_P(ary)) { ARY_SET_EMBED_LEN((ary), (n)); } else { ARY_SET_HEAP_LEN((ary), (n)); } assert(RARRAY_LEN(ary) == (n)); } while (0)
#define ARY_INCREASE_PTR(ary, n) do { assert(!ARY_EMBED_P(ary)); assert(!OBJ_FROZEN(ary)); RARRAY(ary)->as.heap.ptr += (n); } while (0)
#define ARY_INCREASE_LEN(ary, n) do { assert(!OBJ_FROZEN(ary)); if (ARY_EMBED_P(ary)) { ARY_SET_EMBED_LEN((ary), RARRAY_LEN(ary)+(n)); } else { RARRAY(ary)->as.heap.len += (n); } } while (0)
#define ARY_CAPA(ary) (ARY_EMBED_P(ary) ? RARRAY_EMBED_LEN_MAX : ARY_SHARED_ROOT_P(ary) ? RARRAY_LEN(ary) : ARY_HEAP_CAPA(ary))
#define ARY_SET_CAPA(ary, n) do { assert(!ARY_EMBED_P(ary)); assert(!ARY_SHARED_P(ary)); assert(!OBJ_FROZEN(ary)); RARRAY(ary)->as.heap.aux.capa = (n); } while (0)
#define ARY_SHARED_ROOT(ary) (assert(ARY_SHARED_P(ary)), RARRAY(ary)->as.heap.aux.shared_root)
#define ARY_SET_SHARED(ary, value) do { const VALUE _ary_ = (ary); const VALUE _value_ = (value); assert(!ARY_EMBED_P(_ary_)); assert(ARY_SHARED_P(_ary_)); assert(ARY_SHARED_ROOT_P(_value_)); RB_OBJ_WRITE(_ary_, &RARRAY(_ary_)->as.heap.aux.shared_root, _value_); } while (0)
#define RARRAY_SHARED_ROOT_FLAG FL_USER5
#define ARY_SHARED_ROOT_P(ary) (assert(should_be_T_ARRAY((VALUE)(ary))), FL_TEST_RAW((ary), RARRAY_SHARED_ROOT_FLAG))
#define ARY_SHARED_ROOT_REFCNT(ary) (assert(ARY_SHARED_ROOT_P(ary)), RARRAY(ary)->as.heap.aux.capa)
#define ARY_SHARED_ROOT_OCCUPIED(ary) (ARY_SHARED_ROOT_REFCNT(ary) == 1)
#define ARY_SET_SHARED_ROOT_REFCNT(ary, value) do { assert(ARY_SHARED_ROOT_P(ary)); RARRAY(ary)->as.heap.aux.capa = (value); } while (0)
#define FL_SET_SHARED_ROOT(ary) do { assert(!ARY_EMBED_P(ary)); assert(!RARRAY_TRANSIENT_P(ary)); FL_SET((ary), RARRAY_SHARED_ROOT_FLAG); } while (0)
static inline void
ARY_SET(VALUE a, long i, VALUE v)
{
assert(!ARY_SHARED_P(a));
assert(!OBJ_FROZEN(a));
RARRAY_ASET(a, i, v);
}
#undef RARRAY_ASET
#if ARRAY_DEBUG
#define ary_verify(ary) ary_verify_(ary, __FILE__, __LINE__)
static VALUE
ary_verify_(VALUE ary, const char *file, int line)
{
assert(RB_TYPE_P(ary, T_ARRAY));
if (FL_TEST(ary, ELTS_SHARED)) {
VALUE root = RARRAY(ary)->as.heap.aux.shared_root;
const VALUE *ptr = ARY_HEAP_PTR(ary);
const VALUE *root_ptr = RARRAY_CONST_PTR_TRANSIENT(root);
long len = ARY_HEAP_LEN(ary), root_len = RARRAY_LEN(root);
assert(FL_TEST(root, RARRAY_SHARED_ROOT_FLAG));
assert(root_ptr <= ptr && ptr + len <= root_ptr + root_len);
ary_verify(root);
}
else if (ARY_EMBED_P(ary)) {
assert(!RARRAY_TRANSIENT_P(ary));
assert(!ARY_SHARED_P(ary));
assert(RARRAY_LEN(ary) <= RARRAY_EMBED_LEN_MAX);
}
else {
#if 1
const VALUE *ptr = RARRAY_CONST_PTR_TRANSIENT(ary);
long i, len = RARRAY_LEN(ary);
volatile VALUE v;
if (len > 1) len = 1; 
for (i=0; i<len; i++) {
v = ptr[i]; 
}
v = v;
#endif
}
#if USE_TRANSIENT_HEAP
if (RARRAY_TRANSIENT_P(ary)) {
assert(rb_transient_heap_managed_ptr_p(RARRAY_CONST_PTR_TRANSIENT(ary)));
}
#endif
rb_transient_heap_verify();
return ary;
}
void
rb_ary_verify(VALUE ary)
{
ary_verify(ary);
}
#else
#define ary_verify(ary) ((void)0)
#endif
VALUE *
rb_ary_ptr_use_start(VALUE ary)
{
#if ARRAY_DEBUG
FL_SET_RAW(ary, RARRAY_PTR_IN_USE_FLAG);
#endif
return (VALUE *)RARRAY_CONST_PTR_TRANSIENT(ary);
}
void
rb_ary_ptr_use_end(VALUE ary)
{
#if ARRAY_DEBUG
FL_UNSET_RAW(ary, RARRAY_PTR_IN_USE_FLAG);
#endif
}
void
rb_mem_clear(VALUE *mem, long size)
{
while (size--) {
*mem++ = Qnil;
}
}
static void
ary_mem_clear(VALUE ary, long beg, long size)
{
RARRAY_PTR_USE_TRANSIENT(ary, ptr, {
rb_mem_clear(ptr + beg, size);
});
}
static inline void
memfill(register VALUE *mem, register long size, register VALUE val)
{
while (size--) {
*mem++ = val;
}
}
static void
ary_memfill(VALUE ary, long beg, long size, VALUE val)
{
RARRAY_PTR_USE_TRANSIENT(ary, ptr, {
memfill(ptr + beg, size, val);
RB_OBJ_WRITTEN(ary, Qundef, val);
});
}
static void
ary_memcpy0(VALUE ary, long beg, long argc, const VALUE *argv, VALUE buff_owner_ary)
{
assert(!ARY_SHARED_P(buff_owner_ary));
if (argc > (int)(128/sizeof(VALUE)) ) {
rb_gc_writebarrier_remember(buff_owner_ary);
RARRAY_PTR_USE_TRANSIENT(ary, ptr, {
MEMCPY(ptr+beg, argv, VALUE, argc);
});
}
else {
int i;
RARRAY_PTR_USE_TRANSIENT(ary, ptr, {
for (i=0; i<argc; i++) {
RB_OBJ_WRITE(buff_owner_ary, &ptr[i+beg], argv[i]);
}
});
}
}
static void
ary_memcpy(VALUE ary, long beg, long argc, const VALUE *argv)
{
ary_memcpy0(ary, beg, argc, argv, ary);
}
static VALUE *
ary_heap_alloc(VALUE ary, size_t capa)
{
VALUE *ptr = rb_transient_heap_alloc(ary, sizeof(VALUE) * capa);
if (ptr != NULL) {
RARY_TRANSIENT_SET(ary);
}
else {
RARY_TRANSIENT_UNSET(ary);
ptr = ALLOC_N(VALUE, capa);
}
return ptr;
}
static void
ary_heap_free_ptr(VALUE ary, const VALUE *ptr, long size)
{
if (RARRAY_TRANSIENT_P(ary)) {
}
else {
ruby_sized_xfree((void *)ptr, size);
}
}
static void
ary_heap_free(VALUE ary)
{
if (RARRAY_TRANSIENT_P(ary)) {
RARY_TRANSIENT_UNSET(ary);
}
else {
ary_heap_free_ptr(ary, ARY_HEAP_PTR(ary), ARY_HEAP_SIZE(ary));
}
}
static void
ary_heap_realloc(VALUE ary, size_t new_capa)
{
size_t old_capa = ARY_HEAP_CAPA(ary);
if (RARRAY_TRANSIENT_P(ary)) {
if (new_capa <= old_capa) {
}
else {
VALUE *new_ptr = rb_transient_heap_alloc(ary, sizeof(VALUE) * new_capa);
if (new_ptr == NULL) {
new_ptr = ALLOC_N(VALUE, new_capa);
RARY_TRANSIENT_UNSET(ary);
}
MEMCPY(new_ptr, ARY_HEAP_PTR(ary), VALUE, old_capa);
ARY_SET_PTR(ary, new_ptr);
}
}
else {
SIZED_REALLOC_N(RARRAY(ary)->as.heap.ptr, VALUE, new_capa, old_capa);
}
ary_verify(ary);
}
#if USE_TRANSIENT_HEAP
static inline void
rb_ary_transient_heap_evacuate_(VALUE ary, int transient, int promote)
{
if (transient) {
VALUE *new_ptr;
const VALUE *old_ptr = ARY_HEAP_PTR(ary);
long capa = ARY_HEAP_CAPA(ary);
long len = ARY_HEAP_LEN(ary);
if (ARY_SHARED_ROOT_P(ary)) {
capa = len;
}
assert(ARY_OWNS_HEAP_P(ary));
assert(RARRAY_TRANSIENT_P(ary));
assert(!ARY_PTR_USING_P(ary));
if (promote) {
new_ptr = ALLOC_N(VALUE, capa);
RARY_TRANSIENT_UNSET(ary);
}
else {
new_ptr = ary_heap_alloc(ary, capa);
}
MEMCPY(new_ptr, old_ptr, VALUE, capa);
RARRAY(ary)->as.heap.ptr = new_ptr;
}
ary_verify(ary);
}
void
rb_ary_transient_heap_evacuate(VALUE ary, int promote)
{
rb_ary_transient_heap_evacuate_(ary, RARRAY_TRANSIENT_P(ary), promote);
}
void
rb_ary_detransient(VALUE ary)
{
assert(RARRAY_TRANSIENT_P(ary));
rb_ary_transient_heap_evacuate_(ary, TRUE, TRUE);
}
#else
void
rb_ary_detransient(VALUE ary)
{
}
#endif
static void
ary_resize_capa(VALUE ary, long capacity)
{
assert(RARRAY_LEN(ary) <= capacity);
assert(!OBJ_FROZEN(ary));
assert(!ARY_SHARED_P(ary));
if (capacity > RARRAY_EMBED_LEN_MAX) {
if (ARY_EMBED_P(ary)) {
long len = ARY_EMBED_LEN(ary);
VALUE *ptr = ary_heap_alloc(ary, capacity);
MEMCPY(ptr, ARY_EMBED_PTR(ary), VALUE, len);
FL_UNSET_EMBED(ary);
ARY_SET_PTR(ary, ptr);
ARY_SET_HEAP_LEN(ary, len);
}
else {
ary_heap_realloc(ary, capacity);
}
ARY_SET_CAPA(ary, capacity);
}
else {
if (!ARY_EMBED_P(ary)) {
long len = ARY_HEAP_LEN(ary);
long old_capa = ARY_HEAP_CAPA(ary);
const VALUE *ptr = ARY_HEAP_PTR(ary);
if (len > capacity) len = capacity;
MEMCPY((VALUE *)RARRAY(ary)->as.ary, ptr, VALUE, len);
ary_heap_free_ptr(ary, ptr, old_capa);
FL_SET_EMBED(ary);
ARY_SET_LEN(ary, len);
}
}
ary_verify(ary);
}
static inline void
ary_shrink_capa(VALUE ary)
{
long capacity = ARY_HEAP_LEN(ary);
long old_capa = ARY_HEAP_CAPA(ary);
assert(!ARY_SHARED_P(ary));
assert(old_capa >= capacity);
if (old_capa > capacity) ary_heap_realloc(ary, capacity);
ary_verify(ary);
}
static void
ary_double_capa(VALUE ary, long min)
{
long new_capa = ARY_CAPA(ary) / 2;
if (new_capa < ARY_DEFAULT_SIZE) {
new_capa = ARY_DEFAULT_SIZE;
}
if (new_capa >= ARY_MAX_SIZE - min) {
new_capa = (ARY_MAX_SIZE - min) / 2;
}
new_capa += min;
ary_resize_capa(ary, new_capa);
ary_verify(ary);
}
static void
rb_ary_decrement_share(VALUE shared_root)
{
if (shared_root) {
long num = ARY_SHARED_ROOT_REFCNT(shared_root) - 1;
if (num == 0) {
rb_ary_free(shared_root);
rb_gc_force_recycle(shared_root);
}
else if (num > 0) {
ARY_SET_SHARED_ROOT_REFCNT(shared_root, num);
}
}
}
static void
rb_ary_unshare(VALUE ary)
{
VALUE shared_root = RARRAY(ary)->as.heap.aux.shared_root;
rb_ary_decrement_share(shared_root);
FL_UNSET_SHARED(ary);
}
static inline void
rb_ary_unshare_safe(VALUE ary)
{
if (ARY_SHARED_P(ary) && !ARY_EMBED_P(ary)) {
rb_ary_unshare(ary);
}
}
static VALUE
rb_ary_increment_share(VALUE shared_root)
{
long num = ARY_SHARED_ROOT_REFCNT(shared_root);
if (num >= 0) {
ARY_SET_SHARED_ROOT_REFCNT(shared_root, num + 1);
}
return shared_root;
}
static void
rb_ary_set_shared(VALUE ary, VALUE shared_root)
{
rb_ary_increment_share(shared_root);
FL_SET_SHARED(ary);
RB_DEBUG_COUNTER_INC(obj_ary_shared_create);
ARY_SET_SHARED(ary, shared_root);
}
static inline void
rb_ary_modify_check(VALUE ary)
{
rb_check_frozen(ary);
ary_verify(ary);
}
void
rb_ary_modify(VALUE ary)
{
rb_ary_modify_check(ary);
if (ARY_SHARED_P(ary)) {
long shared_len, len = RARRAY_LEN(ary);
VALUE shared_root = ARY_SHARED_ROOT(ary);
ary_verify(shared_root);
if (len <= RARRAY_EMBED_LEN_MAX) {
const VALUE *ptr = ARY_HEAP_PTR(ary);
FL_UNSET_SHARED(ary);
FL_SET_EMBED(ary);
MEMCPY((VALUE *)ARY_EMBED_PTR(ary), ptr, VALUE, len);
rb_ary_decrement_share(shared_root);
ARY_SET_EMBED_LEN(ary, len);
}
else if (ARY_SHARED_ROOT_OCCUPIED(shared_root) && len > ((shared_len = RARRAY_LEN(shared_root))>>1)) {
long shift = RARRAY_CONST_PTR_TRANSIENT(ary) - RARRAY_CONST_PTR_TRANSIENT(shared_root);
FL_UNSET_SHARED(ary);
ARY_SET_PTR(ary, RARRAY_CONST_PTR_TRANSIENT(shared_root));
ARY_SET_CAPA(ary, shared_len);
RARRAY_PTR_USE_TRANSIENT(ary, ptr, {
MEMMOVE(ptr, ptr+shift, VALUE, len);
});
FL_SET_EMBED(shared_root);
rb_ary_decrement_share(shared_root);
}
else {
VALUE *ptr = ary_heap_alloc(ary, len);
MEMCPY(ptr, ARY_HEAP_PTR(ary), VALUE, len);
rb_ary_unshare(ary);
ARY_SET_CAPA(ary, len);
ARY_SET_PTR(ary, ptr);
}
rb_gc_writebarrier_remember(ary);
}
ary_verify(ary);
}
static VALUE
ary_ensure_room_for_push(VALUE ary, long add_len)
{
long old_len = RARRAY_LEN(ary);
long new_len = old_len + add_len;
long capa;
if (old_len > ARY_MAX_SIZE - add_len) {
rb_raise(rb_eIndexError, "index %ld too big", new_len);
}
if (ARY_SHARED_P(ary)) {
if (new_len > RARRAY_EMBED_LEN_MAX) {
VALUE shared_root = ARY_SHARED_ROOT(ary);
if (ARY_SHARED_ROOT_OCCUPIED(shared_root)) {
if (ARY_HEAP_PTR(ary) - RARRAY_CONST_PTR_TRANSIENT(shared_root) + new_len <= RARRAY_LEN(shared_root)) {
rb_ary_modify_check(ary);
ary_verify(ary);
ary_verify(shared_root);
return shared_root;
}
else {
rb_ary_modify(ary);
capa = ARY_CAPA(ary);
if (new_len > capa - (capa >> 6)) {
ary_double_capa(ary, new_len);
}
ary_verify(ary);
return ary;
}
}
}
ary_verify(ary);
rb_ary_modify(ary);
}
else {
rb_ary_modify_check(ary);
}
capa = ARY_CAPA(ary);
if (new_len > capa) {
ary_double_capa(ary, new_len);
}
ary_verify(ary);
return ary;
}
VALUE
rb_ary_freeze(VALUE ary)
{
return rb_obj_freeze(ary);
}
VALUE
rb_ary_shared_with_p(VALUE ary1, VALUE ary2)
{
if (!ARY_EMBED_P(ary1) && ARY_SHARED_P(ary1) &&
!ARY_EMBED_P(ary2) && ARY_SHARED_P(ary2) &&
RARRAY(ary1)->as.heap.aux.shared_root == RARRAY(ary2)->as.heap.aux.shared_root &&
RARRAY(ary1)->as.heap.len == RARRAY(ary2)->as.heap.len) {
return Qtrue;
}
return Qfalse;
}
static VALUE
ary_alloc(VALUE klass)
{
NEWOBJ_OF(ary, struct RArray, klass, T_ARRAY | RARRAY_EMBED_FLAG | (RGENGC_WB_PROTECTED_ARRAY ? FL_WB_PROTECTED : 0));
return (VALUE)ary;
}
static VALUE
empty_ary_alloc(VALUE klass)
{
RUBY_DTRACE_CREATE_HOOK(ARRAY, 0);
return ary_alloc(klass);
}
static VALUE
ary_new(VALUE klass, long capa)
{
VALUE ary,*ptr;
if (capa < 0) {
rb_raise(rb_eArgError, "negative array size (or size too big)");
}
if (capa > ARY_MAX_SIZE) {
rb_raise(rb_eArgError, "array size too big");
}
RUBY_DTRACE_CREATE_HOOK(ARRAY, capa);
ary = ary_alloc(klass);
if (capa > RARRAY_EMBED_LEN_MAX) {
ptr = ary_heap_alloc(ary, capa);
FL_UNSET_EMBED(ary);
ARY_SET_PTR(ary, ptr);
ARY_SET_CAPA(ary, capa);
ARY_SET_HEAP_LEN(ary, 0);
}
return ary;
}
VALUE
rb_ary_new_capa(long capa)
{
return ary_new(rb_cArray, capa);
}
VALUE
rb_ary_new(void)
{
return rb_ary_new2(RARRAY_EMBED_LEN_MAX);
}
VALUE
(rb_ary_new_from_args)(long n, ...)
{
va_list ar;
VALUE ary;
long i;
ary = rb_ary_new2(n);
va_start(ar, n);
for (i=0; i<n; i++) {
ARY_SET(ary, i, va_arg(ar, VALUE));
}
va_end(ar);
ARY_SET_LEN(ary, n);
return ary;
}
MJIT_FUNC_EXPORTED VALUE
rb_ary_tmp_new_from_values(VALUE klass, long n, const VALUE *elts)
{
VALUE ary;
ary = ary_new(klass, n);
if (n > 0 && elts) {
ary_memcpy(ary, 0, n, elts);
ARY_SET_LEN(ary, n);
}
return ary;
}
VALUE
rb_ary_new_from_values(long n, const VALUE *elts)
{
return rb_ary_tmp_new_from_values(rb_cArray, n, elts);
}
VALUE
rb_ary_tmp_new(long capa)
{
VALUE ary = ary_new(0, capa);
rb_ary_transient_heap_evacuate(ary, TRUE);
return ary;
}
VALUE
rb_ary_tmp_new_fill(long capa)
{
VALUE ary = ary_new(0, capa);
ary_memfill(ary, 0, capa, Qnil);
ARY_SET_LEN(ary, capa);
rb_ary_transient_heap_evacuate(ary, TRUE);
return ary;
}
void
rb_ary_free(VALUE ary)
{
if (ARY_OWNS_HEAP_P(ary)) {
if (USE_DEBUG_COUNTER &&
!ARY_SHARED_ROOT_P(ary) &&
ARY_HEAP_CAPA(ary) > RARRAY_LEN(ary)) {
RB_DEBUG_COUNTER_INC(obj_ary_extracapa);
}
if (RARRAY_TRANSIENT_P(ary)) {
RB_DEBUG_COUNTER_INC(obj_ary_transient);
}
else {
RB_DEBUG_COUNTER_INC(obj_ary_ptr);
ary_heap_free(ary);
}
}
else {
RB_DEBUG_COUNTER_INC(obj_ary_embed);
}
if (ARY_SHARED_P(ary)) {
RB_DEBUG_COUNTER_INC(obj_ary_shared);
}
if (ARY_SHARED_ROOT_P(ary) && ARY_SHARED_ROOT_OCCUPIED(ary)) {
RB_DEBUG_COUNTER_INC(obj_ary_shared_root_occupied);
}
}
RUBY_FUNC_EXPORTED size_t
rb_ary_memsize(VALUE ary)
{
if (ARY_OWNS_HEAP_P(ary)) {
return ARY_CAPA(ary) * sizeof(VALUE);
}
else {
return 0;
}
}
static inline void
ary_discard(VALUE ary)
{
rb_ary_free(ary);
RBASIC(ary)->flags |= RARRAY_EMBED_FLAG;
RBASIC(ary)->flags &= ~(RARRAY_EMBED_LEN_MASK | RARRAY_TRANSIENT_FLAG);
}
static VALUE
ary_make_shared(VALUE ary)
{
assert(!ARY_EMBED_P(ary));
ary_verify(ary);
if (ARY_SHARED_P(ary)) {
return ARY_SHARED_ROOT(ary);
}
else if (ARY_SHARED_ROOT_P(ary)) {
return ary;
}
else if (OBJ_FROZEN(ary)) {
rb_ary_transient_heap_evacuate(ary, TRUE);
ary_shrink_capa(ary);
FL_SET_SHARED_ROOT(ary);
ARY_SET_SHARED_ROOT_REFCNT(ary, 1);
return ary;
}
else {
long capa = ARY_CAPA(ary), len = RARRAY_LEN(ary);
const VALUE *ptr;
NEWOBJ_OF(shared, struct RArray, 0, T_ARRAY | (RGENGC_WB_PROTECTED_ARRAY ? FL_WB_PROTECTED : 0));
rb_ary_transient_heap_evacuate(ary, TRUE);
ptr = ARY_HEAP_PTR(ary);
FL_UNSET_EMBED(shared);
ARY_SET_LEN((VALUE)shared, capa);
ARY_SET_PTR((VALUE)shared, ptr);
ary_mem_clear((VALUE)shared, len, capa - len);
FL_SET_SHARED_ROOT(shared);
ARY_SET_SHARED_ROOT_REFCNT((VALUE)shared, 1);
FL_SET_SHARED(ary);
RB_DEBUG_COUNTER_INC(obj_ary_shared_create);
ARY_SET_SHARED(ary, (VALUE)shared);
OBJ_FREEZE(shared);
ary_verify((VALUE)shared);
ary_verify(ary);
return (VALUE)shared;
}
}
static VALUE
ary_make_substitution(VALUE ary)
{
long len = RARRAY_LEN(ary);
if (len <= RARRAY_EMBED_LEN_MAX) {
VALUE subst = rb_ary_new2(len);
ary_memcpy(subst, 0, len, RARRAY_CONST_PTR_TRANSIENT(ary));
ARY_SET_EMBED_LEN(subst, len);
return subst;
}
else {
return rb_ary_increment_share(ary_make_shared(ary));
}
}
VALUE
rb_assoc_new(VALUE car, VALUE cdr)
{
return rb_ary_new3(2, car, cdr);
}
VALUE
rb_to_array_type(VALUE ary)
{
return rb_convert_type_with_id(ary, T_ARRAY, "Array", idTo_ary);
}
#define to_ary rb_to_array_type
VALUE
rb_check_array_type(VALUE ary)
{
return rb_check_convert_type_with_id(ary, T_ARRAY, "Array", idTo_ary);
}
MJIT_FUNC_EXPORTED VALUE
rb_check_to_array(VALUE ary)
{
return rb_check_convert_type_with_id(ary, T_ARRAY, "Array", idTo_a);
}
static VALUE
rb_ary_s_try_convert(VALUE dummy, VALUE ary)
{
return rb_check_array_type(ary);
}
static VALUE
rb_ary_initialize(int argc, VALUE *argv, VALUE ary)
{
long len;
VALUE size, val;
rb_ary_modify(ary);
if (argc == 0) {
if (ARY_OWNS_HEAP_P(ary) && ARY_HEAP_PTR(ary) != NULL) {
ary_heap_free(ary);
}
rb_ary_unshare_safe(ary);
FL_SET_EMBED(ary);
ARY_SET_EMBED_LEN(ary, 0);
if (rb_block_given_p()) {
rb_warning("given block not used");
}
return ary;
}
rb_scan_args(argc, argv, "02", &size, &val);
if (argc == 1 && !FIXNUM_P(size)) {
val = rb_check_array_type(size);
if (!NIL_P(val)) {
rb_ary_replace(ary, val);
return ary;
}
}
len = NUM2LONG(size);
if (len < 0) {
rb_raise(rb_eArgError, "negative array size");
}
if (len > ARY_MAX_SIZE) {
rb_raise(rb_eArgError, "array size too big");
}
rb_ary_modify(ary);
ary_resize_capa(ary, len);
if (rb_block_given_p()) {
long i;
if (argc == 2) {
rb_warn("block supersedes default value argument");
}
for (i=0; i<len; i++) {
rb_ary_store(ary, i, rb_yield(LONG2NUM(i)));
ARY_SET_LEN(ary, i + 1);
}
}
else {
ary_memfill(ary, 0, len, val);
ARY_SET_LEN(ary, len);
}
return ary;
}
static VALUE
rb_ary_s_create(int argc, VALUE *argv, VALUE klass)
{
VALUE ary = ary_new(klass, argc);
if (argc > 0 && argv) {
ary_memcpy(ary, 0, argc, argv);
ARY_SET_LEN(ary, argc);
}
return ary;
}
void
rb_ary_store(VALUE ary, long idx, VALUE val)
{
long len = RARRAY_LEN(ary);
if (idx < 0) {
idx += len;
if (idx < 0) {
rb_raise(rb_eIndexError, "index %ld too small for array; minimum: %ld",
idx - len, -len);
}
}
else if (idx >= ARY_MAX_SIZE) {
rb_raise(rb_eIndexError, "index %ld too big", idx);
}
rb_ary_modify(ary);
if (idx >= ARY_CAPA(ary)) {
ary_double_capa(ary, idx);
}
if (idx > len) {
ary_mem_clear(ary, len, idx - len + 1);
}
if (idx >= len) {
ARY_SET_LEN(ary, idx + 1);
}
ARY_SET(ary, idx, val);
}
static VALUE
ary_make_partial(VALUE ary, VALUE klass, long offset, long len)
{
assert(offset >= 0);
assert(len >= 0);
assert(offset+len <= RARRAY_LEN(ary));
if (len <= RARRAY_EMBED_LEN_MAX) {
VALUE result = ary_alloc(klass);
ary_memcpy(result, 0, len, RARRAY_CONST_PTR_TRANSIENT(ary) + offset);
ARY_SET_EMBED_LEN(result, len);
return result;
}
else {
VALUE shared, result = ary_alloc(klass);
FL_UNSET_EMBED(result);
shared = ary_make_shared(ary);
ARY_SET_PTR(result, RARRAY_CONST_PTR_TRANSIENT(ary));
ARY_SET_LEN(result, RARRAY_LEN(ary));
rb_ary_set_shared(result, shared);
ARY_INCREASE_PTR(result, offset);
ARY_SET_LEN(result, len);
ary_verify(shared);
ary_verify(result);
return result;
}
}
static VALUE
ary_make_shared_copy(VALUE ary)
{
return ary_make_partial(ary, rb_obj_class(ary), 0, RARRAY_LEN(ary));
}
enum ary_take_pos_flags
{
ARY_TAKE_FIRST = 0,
ARY_TAKE_LAST = 1
};
static VALUE
ary_take_first_or_last(int argc, const VALUE *argv, VALUE ary, enum ary_take_pos_flags last)
{
long n;
long len;
long offset = 0;
argc = rb_check_arity(argc, 0, 1);
RUBY_ASSERT_ALWAYS(argc == 1);
n = NUM2LONG(argv[0]);
len = RARRAY_LEN(ary);
if (n > len) {
n = len;
}
else if (n < 0) {
rb_raise(rb_eArgError, "negative array size");
}
if (last) {
offset = len - n;
}
return ary_make_partial(ary, rb_cArray, offset, n);
}
VALUE
rb_ary_push(VALUE ary, VALUE item)
{
long idx = RARRAY_LEN((ary_verify(ary), ary));
VALUE target_ary = ary_ensure_room_for_push(ary, 1);
RARRAY_PTR_USE_TRANSIENT(ary, ptr, {
RB_OBJ_WRITE(target_ary, &ptr[idx], item);
});
ARY_SET_LEN(ary, idx + 1);
ary_verify(ary);
return ary;
}
VALUE
rb_ary_cat(VALUE ary, const VALUE *argv, long len)
{
long oldlen = RARRAY_LEN(ary);
VALUE target_ary = ary_ensure_room_for_push(ary, len);
ary_memcpy0(ary, oldlen, len, argv, target_ary);
ARY_SET_LEN(ary, oldlen + len);
return ary;
}
static VALUE
rb_ary_push_m(int argc, VALUE *argv, VALUE ary)
{
return rb_ary_cat(ary, argv, argc);
}
VALUE
rb_ary_pop(VALUE ary)
{
long n;
rb_ary_modify_check(ary);
n = RARRAY_LEN(ary);
if (n == 0) return Qnil;
if (ARY_OWNS_HEAP_P(ary) &&
n * 3 < ARY_CAPA(ary) &&
ARY_CAPA(ary) > ARY_DEFAULT_SIZE)
{
ary_resize_capa(ary, n * 2);
}
--n;
ARY_SET_LEN(ary, n);
ary_verify(ary);
return RARRAY_AREF(ary, n);
}
static VALUE
rb_ary_pop_m(int argc, VALUE *argv, VALUE ary)
{
VALUE result;
if (argc == 0) {
return rb_ary_pop(ary);
}
rb_ary_modify_check(ary);
result = ary_take_first_or_last(argc, argv, ary, ARY_TAKE_LAST);
ARY_INCREASE_LEN(ary, -RARRAY_LEN(result));
ary_verify(ary);
return result;
}
VALUE
rb_ary_shift(VALUE ary)
{
VALUE top;
long len = RARRAY_LEN(ary);
rb_ary_modify_check(ary);
if (len == 0) return Qnil;
top = RARRAY_AREF(ary, 0);
if (!ARY_SHARED_P(ary)) {
if (len < ARY_DEFAULT_SIZE) {
RARRAY_PTR_USE_TRANSIENT(ary, ptr, {
MEMMOVE(ptr, ptr+1, VALUE, len-1);
}); 
ARY_INCREASE_LEN(ary, -1);
ary_verify(ary);
return top;
}
assert(!ARY_EMBED_P(ary)); 
ARY_SET(ary, 0, Qnil);
ary_make_shared(ary);
}
else if (ARY_SHARED_ROOT_OCCUPIED(ARY_SHARED_ROOT(ary))) {
RARRAY_PTR_USE_TRANSIENT(ary, ptr, ptr[0] = Qnil);
}
ARY_INCREASE_PTR(ary, 1); 
ARY_INCREASE_LEN(ary, -1);
ary_verify(ary);
return top;
}
static VALUE
rb_ary_shift_m(int argc, VALUE *argv, VALUE ary)
{
VALUE result;
long n;
if (argc == 0) {
return rb_ary_shift(ary);
}
rb_ary_modify_check(ary);
result = ary_take_first_or_last(argc, argv, ary, ARY_TAKE_FIRST);
n = RARRAY_LEN(result);
rb_ary_behead(ary,n);
return result;
}
MJIT_FUNC_EXPORTED VALUE
rb_ary_behead(VALUE ary, long n)
{
if (n<=0) return ary;
rb_ary_modify_check(ary);
if (ARY_SHARED_P(ary)) {
if (ARY_SHARED_ROOT_OCCUPIED(ARY_SHARED_ROOT(ary))) {
setup_occupied_shared:
ary_mem_clear(ary, 0, n);
}
ARY_INCREASE_PTR(ary, n);
}
else {
if (RARRAY_LEN(ary) < ARY_DEFAULT_SIZE) {
RARRAY_PTR_USE_TRANSIENT(ary, ptr, {
MEMMOVE(ptr, ptr+n, VALUE, RARRAY_LEN(ary)-n);
}); 
}
else {
ary_make_shared(ary);
goto setup_occupied_shared;
}
}
ARY_INCREASE_LEN(ary, -n);
ary_verify(ary);
return ary;
}
static VALUE
ary_ensure_room_for_unshift(VALUE ary, int argc)
{
long len = RARRAY_LEN(ary);
long new_len = len + argc;
long capa;
const VALUE *head, *sharedp;
if (len > ARY_MAX_SIZE - argc) {
rb_raise(rb_eIndexError, "index %ld too big", new_len);
}
if (ARY_SHARED_P(ary)) {
VALUE shared_root = ARY_SHARED_ROOT(ary);
capa = RARRAY_LEN(shared_root);
if (ARY_SHARED_ROOT_OCCUPIED(shared_root) && capa > new_len) {
rb_ary_modify_check(ary);
head = RARRAY_CONST_PTR_TRANSIENT(ary);
sharedp = RARRAY_CONST_PTR_TRANSIENT(shared_root);
goto makeroom_if_need;
}
}
rb_ary_modify(ary);
capa = ARY_CAPA(ary);
if (capa - (capa >> 6) <= new_len) {
ary_double_capa(ary, new_len);
}
if (new_len > ARY_DEFAULT_SIZE * 4) {
ary_verify(ary);
capa = ARY_CAPA(ary);
ary_make_shared(ary);
head = sharedp = RARRAY_CONST_PTR_TRANSIENT(ary);
goto makeroom;
makeroom_if_need:
if (head - sharedp < argc) {
long room;
makeroom:
room = capa - new_len;
room -= room >> 4;
MEMMOVE((VALUE *)sharedp + argc + room, head, VALUE, len);
head = sharedp + argc + room;
}
ARY_SET_PTR(ary, head - argc);
assert(ARY_SHARED_ROOT_OCCUPIED(ARY_SHARED_ROOT(ary)));
ary_verify(ary);
return ARY_SHARED_ROOT(ary);
}
else {
RARRAY_PTR_USE_TRANSIENT(ary, ptr, {
MEMMOVE(ptr + argc, ptr, VALUE, len);
});
ary_verify(ary);
return ary;
}
}
static VALUE
rb_ary_unshift_m(int argc, VALUE *argv, VALUE ary)
{
long len = RARRAY_LEN(ary);
VALUE target_ary;
if (argc == 0) {
rb_ary_modify_check(ary);
return ary;
}
target_ary = ary_ensure_room_for_unshift(ary, argc);
ary_memcpy0(ary, 0, argc, argv, target_ary);
ARY_SET_LEN(ary, len + argc);
return ary;
}
VALUE
rb_ary_unshift(VALUE ary, VALUE item)
{
return rb_ary_unshift_m(1,&item,ary);
}
static inline VALUE
rb_ary_elt(VALUE ary, long offset)
{
long len = RARRAY_LEN(ary);
if (len == 0) return Qnil;
if (offset < 0 || len <= offset) {
return Qnil;
}
return RARRAY_AREF(ary, offset);
}
VALUE
rb_ary_entry(VALUE ary, long offset)
{
return rb_ary_entry_internal(ary, offset);
}
VALUE
rb_ary_subseq(VALUE ary, long beg, long len)
{
VALUE klass;
long alen = RARRAY_LEN(ary);
if (beg > alen) return Qnil;
if (beg < 0 || len < 0) return Qnil;
if (alen < len || alen < beg + len) {
len = alen - beg;
}
klass = rb_obj_class(ary);
if (len == 0) return ary_new(klass, 0);
return ary_make_partial(ary, klass, beg, len);
}
static VALUE rb_ary_aref2(VALUE ary, VALUE b, VALUE e);
VALUE
rb_ary_aref(int argc, const VALUE *argv, VALUE ary)
{
rb_check_arity(argc, 1, 2);
if (argc == 2) {
return rb_ary_aref2(ary, argv[0], argv[1]);
}
return rb_ary_aref1(ary, argv[0]);
}
VALUE
rb_ary_aref2(VALUE ary, VALUE b, VALUE e)
{
long beg = NUM2LONG(b);
long len = NUM2LONG(e);
if (beg < 0) {
beg += RARRAY_LEN(ary);
}
return rb_ary_subseq(ary, beg, len);
}
MJIT_FUNC_EXPORTED VALUE
rb_ary_aref1(VALUE ary, VALUE arg)
{
long beg, len;
if (FIXNUM_P(arg)) {
return rb_ary_entry(ary, FIX2LONG(arg));
}
switch (rb_range_beg_len(arg, &beg, &len, RARRAY_LEN(ary), 0)) {
case Qfalse:
break;
case Qnil:
return Qnil;
default:
return rb_ary_subseq(ary, beg, len);
}
return rb_ary_entry(ary, NUM2LONG(arg));
}
VALUE
rb_ary_at(VALUE ary, VALUE pos)
{
return rb_ary_entry(ary, NUM2LONG(pos));
}
static VALUE
rb_ary_first(int argc, VALUE *argv, VALUE ary)
{
if (argc == 0) {
if (RARRAY_LEN(ary) == 0) return Qnil;
return RARRAY_AREF(ary, 0);
}
else {
return ary_take_first_or_last(argc, argv, ary, ARY_TAKE_FIRST);
}
}
VALUE
rb_ary_last(int argc, const VALUE *argv, VALUE ary)
{
if (argc == 0) {
long len = RARRAY_LEN(ary);
if (len == 0) return Qnil;
return RARRAY_AREF(ary, len-1);
}
else {
return ary_take_first_or_last(argc, argv, ary, ARY_TAKE_LAST);
}
}
static VALUE
rb_ary_fetch(int argc, VALUE *argv, VALUE ary)
{
VALUE pos, ifnone;
long block_given;
long idx;
rb_scan_args(argc, argv, "11", &pos, &ifnone);
block_given = rb_block_given_p();
if (block_given && argc == 2) {
rb_warn("block supersedes default value argument");
}
idx = NUM2LONG(pos);
if (idx < 0) {
idx += RARRAY_LEN(ary);
}
if (idx < 0 || RARRAY_LEN(ary) <= idx) {
if (block_given) return rb_yield(pos);
if (argc == 1) {
rb_raise(rb_eIndexError, "index %ld outside of array bounds: %ld...%ld",
idx - (idx < 0 ? RARRAY_LEN(ary) : 0), -RARRAY_LEN(ary), RARRAY_LEN(ary));
}
return ifnone;
}
return RARRAY_AREF(ary, idx);
}
static VALUE
rb_ary_index(int argc, VALUE *argv, VALUE ary)
{
VALUE val;
long i;
if (argc == 0) {
RETURN_ENUMERATOR(ary, 0, 0);
for (i=0; i<RARRAY_LEN(ary); i++) {
if (RTEST(rb_yield(RARRAY_AREF(ary, i)))) {
return LONG2NUM(i);
}
}
return Qnil;
}
rb_check_arity(argc, 0, 1);
val = argv[0];
if (rb_block_given_p())
rb_warn("given block not used");
for (i=0; i<RARRAY_LEN(ary); i++) {
VALUE e = RARRAY_AREF(ary, i);
if (rb_equal(e, val)) {
return LONG2NUM(i);
}
}
return Qnil;
}
static VALUE
rb_ary_rindex(int argc, VALUE *argv, VALUE ary)
{
VALUE val;
long i = RARRAY_LEN(ary), len;
if (argc == 0) {
RETURN_ENUMERATOR(ary, 0, 0);
while (i--) {
if (RTEST(rb_yield(RARRAY_AREF(ary, i))))
return LONG2NUM(i);
if (i > (len = RARRAY_LEN(ary))) {
i = len;
}
}
return Qnil;
}
rb_check_arity(argc, 0, 1);
val = argv[0];
if (rb_block_given_p())
rb_warn("given block not used");
while (i--) {
VALUE e = RARRAY_AREF(ary, i);
if (rb_equal(e, val)) {
return LONG2NUM(i);
}
if (i > RARRAY_LEN(ary)) {
break;
}
}
return Qnil;
}
VALUE
rb_ary_to_ary(VALUE obj)
{
VALUE tmp = rb_check_array_type(obj);
if (!NIL_P(tmp)) return tmp;
return rb_ary_new3(1, obj);
}
static void
rb_ary_splice(VALUE ary, long beg, long len, const VALUE *rptr, long rlen)
{
long olen;
long rofs;
if (len < 0) rb_raise(rb_eIndexError, "negative length (%ld)", len);
olen = RARRAY_LEN(ary);
if (beg < 0) {
beg += olen;
if (beg < 0) {
rb_raise(rb_eIndexError, "index %ld too small for array; minimum: %ld",
beg - olen, -olen);
}
}
if (olen < len || olen < beg + len) {
len = olen - beg;
}
{
const VALUE *optr = RARRAY_CONST_PTR_TRANSIENT(ary);
rofs = (rptr >= optr && rptr < optr + olen) ? rptr - optr : -1;
}
if (beg >= olen) {
VALUE target_ary;
if (beg > ARY_MAX_SIZE - rlen) {
rb_raise(rb_eIndexError, "index %ld too big", beg);
}
target_ary = ary_ensure_room_for_push(ary, rlen-len); 
len = beg + rlen;
ary_mem_clear(ary, olen, beg - olen);
if (rlen > 0) {
if (rofs != -1) rptr = RARRAY_CONST_PTR_TRANSIENT(ary) + rofs;
ary_memcpy0(ary, beg, rlen, rptr, target_ary);
}
ARY_SET_LEN(ary, len);
}
else {
long alen;
if (olen - len > ARY_MAX_SIZE - rlen) {
rb_raise(rb_eIndexError, "index %ld too big", olen + rlen - len);
}
rb_ary_modify(ary);
alen = olen + rlen - len;
if (alen >= ARY_CAPA(ary)) {
ary_double_capa(ary, alen);
}
if (len != rlen) {
RARRAY_PTR_USE_TRANSIENT(ary, ptr,
MEMMOVE(ptr + beg + rlen, ptr + beg + len,
VALUE, olen - (beg + len)));
ARY_SET_LEN(ary, alen);
}
if (rlen > 0) {
if (rofs != -1) rptr = RARRAY_CONST_PTR_TRANSIENT(ary) + rofs;
RB_OBJ_WB_UNPROTECT_FOR(ARRAY, ary);
RARRAY_PTR_USE_TRANSIENT(ary, ptr,
MEMMOVE(ptr + beg, rptr, VALUE, rlen));
}
}
}
void
rb_ary_set_len(VALUE ary, long len)
{
long capa;
rb_ary_modify_check(ary);
if (ARY_SHARED_P(ary)) {
rb_raise(rb_eRuntimeError, "can't set length of shared ");
}
if (len > (capa = (long)ARY_CAPA(ary))) {
rb_bug("probable buffer overflow: %ld for %ld", len, capa);
}
ARY_SET_LEN(ary, len);
}
VALUE
rb_ary_resize(VALUE ary, long len)
{
long olen;
rb_ary_modify(ary);
olen = RARRAY_LEN(ary);
if (len == olen) return ary;
if (len > ARY_MAX_SIZE) {
rb_raise(rb_eIndexError, "index %ld too big", len);
}
if (len > olen) {
if (len >= ARY_CAPA(ary)) {
ary_double_capa(ary, len);
}
ary_mem_clear(ary, olen, len - olen);
ARY_SET_LEN(ary, len);
}
else if (ARY_EMBED_P(ary)) {
ARY_SET_EMBED_LEN(ary, len);
}
else if (len <= RARRAY_EMBED_LEN_MAX) {
VALUE tmp[RARRAY_EMBED_LEN_MAX];
MEMCPY(tmp, ARY_HEAP_PTR(ary), VALUE, len);
ary_discard(ary);
MEMCPY((VALUE *)ARY_EMBED_PTR(ary), tmp, VALUE, len); 
ARY_SET_EMBED_LEN(ary, len);
}
else {
if (olen > len + ARY_DEFAULT_SIZE) {
ary_heap_realloc(ary, len);
ARY_SET_CAPA(ary, len);
}
ARY_SET_HEAP_LEN(ary, len);
}
ary_verify(ary);
return ary;
}
static VALUE
rb_ary_aset(int argc, VALUE *argv, VALUE ary)
{
long offset, beg, len;
VALUE rpl;
if (argc == 3) {
rb_ary_modify_check(ary);
beg = NUM2LONG(argv[0]);
len = NUM2LONG(argv[1]);
goto range;
}
rb_check_arity(argc, 2, 2);
rb_ary_modify_check(ary);
if (FIXNUM_P(argv[0])) {
offset = FIX2LONG(argv[0]);
goto fixnum;
}
if (rb_range_beg_len(argv[0], &beg, &len, RARRAY_LEN(ary), 1)) {
range:
rpl = rb_ary_to_ary(argv[argc-1]);
rb_ary_splice(ary, beg, len, RARRAY_CONST_PTR_TRANSIENT(rpl), RARRAY_LEN(rpl));
RB_GC_GUARD(rpl);
return argv[argc-1];
}
offset = NUM2LONG(argv[0]);
fixnum:
rb_ary_store(ary, offset, argv[1]);
return argv[1];
}
static VALUE
rb_ary_insert(int argc, VALUE *argv, VALUE ary)
{
long pos;
rb_check_arity(argc, 1, UNLIMITED_ARGUMENTS);
rb_ary_modify_check(ary);
pos = NUM2LONG(argv[0]);
if (argc == 1) return ary;
if (pos == -1) {
pos = RARRAY_LEN(ary);
}
else if (pos < 0) {
long minpos = -RARRAY_LEN(ary) - 1;
if (pos < minpos) {
rb_raise(rb_eIndexError, "index %ld too small for array; minimum: %ld",
pos, minpos);
}
pos++;
}
rb_ary_splice(ary, pos, 0, argv + 1, argc - 1);
return ary;
}
static VALUE
rb_ary_length(VALUE ary);
static VALUE
ary_enum_length(VALUE ary, VALUE args, VALUE eobj)
{
return rb_ary_length(ary);
}
VALUE
rb_ary_each(VALUE ary)
{
long i;
ary_verify(ary);
RETURN_SIZED_ENUMERATOR(ary, 0, 0, ary_enum_length);
for (i=0; i<RARRAY_LEN(ary); i++) {
rb_yield(RARRAY_AREF(ary, i));
}
return ary;
}
static VALUE
rb_ary_each_index(VALUE ary)
{
long i;
RETURN_SIZED_ENUMERATOR(ary, 0, 0, ary_enum_length);
for (i=0; i<RARRAY_LEN(ary); i++) {
rb_yield(LONG2NUM(i));
}
return ary;
}
static VALUE
rb_ary_reverse_each(VALUE ary)
{
long len;
RETURN_SIZED_ENUMERATOR(ary, 0, 0, ary_enum_length);
len = RARRAY_LEN(ary);
while (len--) {
long nlen;
rb_yield(RARRAY_AREF(ary, len));
nlen = RARRAY_LEN(ary);
if (nlen < len) {
len = nlen;
}
}
return ary;
}
static VALUE
rb_ary_length(VALUE ary)
{
long len = RARRAY_LEN(ary);
return LONG2NUM(len);
}
static VALUE
rb_ary_empty_p(VALUE ary)
{
if (RARRAY_LEN(ary) == 0)
return Qtrue;
return Qfalse;
}
VALUE
rb_ary_dup(VALUE ary)
{
long len = RARRAY_LEN(ary);
VALUE dup = rb_ary_new2(len);
ary_memcpy(dup, 0, len, RARRAY_CONST_PTR_TRANSIENT(ary));
ARY_SET_LEN(dup, len);
ary_verify(ary);
ary_verify(dup);
return dup;
}
VALUE
rb_ary_resurrect(VALUE ary)
{
return ary_make_partial(ary, rb_cArray, 0, RARRAY_LEN(ary));
}
extern VALUE rb_output_fs;
static void ary_join_1(VALUE obj, VALUE ary, VALUE sep, long i, VALUE result, int *first);
static VALUE
recursive_join(VALUE obj, VALUE argp, int recur)
{
VALUE *arg = (VALUE *)argp;
VALUE ary = arg[0];
VALUE sep = arg[1];
VALUE result = arg[2];
int *first = (int *)arg[3];
if (recur) {
rb_raise(rb_eArgError, "recursive array join");
}
else {
ary_join_1(obj, ary, sep, 0, result, first);
}
return Qnil;
}
static long
ary_join_0(VALUE ary, VALUE sep, long max, VALUE result)
{
long i;
VALUE val;
if (max > 0) rb_enc_copy(result, RARRAY_AREF(ary, 0));
for (i=0; i<max; i++) {
val = RARRAY_AREF(ary, i);
if (!RB_TYPE_P(val, T_STRING)) break;
if (i > 0 && !NIL_P(sep))
rb_str_buf_append(result, sep);
rb_str_buf_append(result, val);
}
return i;
}
static void
ary_join_1(VALUE obj, VALUE ary, VALUE sep, long i, VALUE result, int *first)
{
VALUE val, tmp;
for (; i<RARRAY_LEN(ary); i++) {
if (i > 0 && !NIL_P(sep))
rb_str_buf_append(result, sep);
val = RARRAY_AREF(ary, i);
if (RB_TYPE_P(val, T_STRING)) {
str_join:
rb_str_buf_append(result, val);
if (*first) {
rb_enc_copy(result, val);
*first = FALSE;
}
}
else if (RB_TYPE_P(val, T_ARRAY)) {
obj = val;
ary_join:
if (val == ary) {
rb_raise(rb_eArgError, "recursive array join");
}
else {
VALUE args[4];
*first = FALSE;
args[0] = val;
args[1] = sep;
args[2] = result;
args[3] = (VALUE)first;
rb_exec_recursive(recursive_join, obj, (VALUE)args);
}
}
else {
tmp = rb_check_string_type(val);
if (!NIL_P(tmp)) {
val = tmp;
goto str_join;
}
tmp = rb_check_array_type(val);
if (!NIL_P(tmp)) {
obj = val;
val = tmp;
goto ary_join;
}
val = rb_obj_as_string(val);
goto str_join;
}
}
}
VALUE
rb_ary_join(VALUE ary, VALUE sep)
{
long len = 1, i;
VALUE val, tmp, result;
if (RARRAY_LEN(ary) == 0) return rb_usascii_str_new(0, 0);
if (!NIL_P(sep)) {
StringValue(sep);
len += RSTRING_LEN(sep) * (RARRAY_LEN(ary) - 1);
}
for (i=0; i<RARRAY_LEN(ary); i++) {
val = RARRAY_AREF(ary, i);
tmp = rb_check_string_type(val);
if (NIL_P(tmp) || tmp != val) {
int first;
long n = RARRAY_LEN(ary);
if (i > n) i = n;
result = rb_str_buf_new(len + (n-i)*10);
rb_enc_associate(result, rb_usascii_encoding());
i = ary_join_0(ary, sep, i, result);
first = i == 0;
ary_join_1(ary, ary, sep, i, result, &first);
return result;
}
len += RSTRING_LEN(tmp);
}
result = rb_str_new(0, len);
rb_str_set_len(result, 0);
ary_join_0(ary, sep, RARRAY_LEN(ary), result);
return result;
}
static VALUE
rb_ary_join_m(int argc, VALUE *argv, VALUE ary)
{
VALUE sep;
if (rb_check_arity(argc, 0, 1) == 0 || NIL_P(sep = argv[0])) {
sep = rb_output_fs;
if (!NIL_P(sep)) {
rb_warn("$, is set to non-nil value");
}
}
return rb_ary_join(ary, sep);
}
static VALUE
inspect_ary(VALUE ary, VALUE dummy, int recur)
{
long i;
VALUE s, str;
if (recur) return rb_usascii_str_new_cstr("[...]");
str = rb_str_buf_new2("[");
for (i=0; i<RARRAY_LEN(ary); i++) {
s = rb_inspect(RARRAY_AREF(ary, i));
if (i > 0) rb_str_buf_cat2(str, ", ");
else rb_enc_copy(str, s);
rb_str_buf_append(str, s);
}
rb_str_buf_cat2(str, "]");
return str;
}
static VALUE
rb_ary_inspect(VALUE ary)
{
if (RARRAY_LEN(ary) == 0) return rb_usascii_str_new2("[]");
return rb_exec_recursive(inspect_ary, ary, 0);
}
VALUE
rb_ary_to_s(VALUE ary)
{
return rb_ary_inspect(ary);
}
static VALUE
rb_ary_to_a(VALUE ary)
{
if (rb_obj_class(ary) != rb_cArray) {
VALUE dup = rb_ary_new2(RARRAY_LEN(ary));
rb_ary_replace(dup, ary);
return dup;
}
return ary;
}
static VALUE
rb_ary_to_h(VALUE ary)
{
long i;
VALUE hash = rb_hash_new_with_size(RARRAY_LEN(ary));
int block_given = rb_block_given_p();
for (i=0; i<RARRAY_LEN(ary); i++) {
const VALUE e = rb_ary_elt(ary, i);
const VALUE elt = block_given ? rb_yield_force_blockarg(e) : e;
const VALUE key_value_pair = rb_check_array_type(elt);
if (NIL_P(key_value_pair)) {
rb_raise(rb_eTypeError, "wrong element type %"PRIsVALUE" at %ld (expected array)",
rb_obj_class(elt), i);
}
if (RARRAY_LEN(key_value_pair) != 2) {
rb_raise(rb_eArgError, "wrong array length at %ld (expected 2, was %ld)",
i, RARRAY_LEN(key_value_pair));
}
rb_hash_aset(hash, RARRAY_AREF(key_value_pair, 0), RARRAY_AREF(key_value_pair, 1));
}
return hash;
}
static VALUE
rb_ary_to_ary_m(VALUE ary)
{
return ary;
}
static void
ary_reverse(VALUE *p1, VALUE *p2)
{
while (p1 < p2) {
VALUE tmp = *p1;
*p1++ = *p2;
*p2-- = tmp;
}
}
VALUE
rb_ary_reverse(VALUE ary)
{
VALUE *p2;
long len = RARRAY_LEN(ary);
rb_ary_modify(ary);
if (len > 1) {
RARRAY_PTR_USE_TRANSIENT(ary, p1, {
p2 = p1 + len - 1; 
ary_reverse(p1, p2);
}); 
}
return ary;
}
static VALUE
rb_ary_reverse_bang(VALUE ary)
{
return rb_ary_reverse(ary);
}
static VALUE
rb_ary_reverse_m(VALUE ary)
{
long len = RARRAY_LEN(ary);
VALUE dup = rb_ary_new2(len);
if (len > 0) {
const VALUE *p1 = RARRAY_CONST_PTR_TRANSIENT(ary);
VALUE *p2 = (VALUE *)RARRAY_CONST_PTR_TRANSIENT(dup) + len - 1;
do *p2-- = *p1++; while (--len > 0);
}
ARY_SET_LEN(dup, RARRAY_LEN(ary));
return dup;
}
static inline long
rotate_count(long cnt, long len)
{
return (cnt < 0) ? (len - (~cnt % len) - 1) : (cnt % len);
}
static void
ary_rotate_ptr(VALUE *ptr, long len, long cnt)
{
if (cnt == 1) {
VALUE tmp = *ptr;
memmove(ptr, ptr + 1, sizeof(VALUE)*(len - 1));
*(ptr + len - 1) = tmp;
} else if (cnt == len - 1) {
VALUE tmp = *(ptr + len - 1);
memmove(ptr + 1, ptr, sizeof(VALUE)*(len - 1));
*ptr = tmp;
} else {
--len;
if (cnt < len) ary_reverse(ptr + cnt, ptr + len);
if (--cnt > 0) ary_reverse(ptr, ptr + cnt);
if (len > 0) ary_reverse(ptr, ptr + len);
}
}
VALUE
rb_ary_rotate(VALUE ary, long cnt)
{
rb_ary_modify(ary);
if (cnt != 0) {
long len = RARRAY_LEN(ary);
if (len > 1 && (cnt = rotate_count(cnt, len)) > 0) {
RARRAY_PTR_USE_TRANSIENT(ary, ptr, ary_rotate_ptr(ptr, len, cnt));
return ary;
}
}
return Qnil;
}
static VALUE
rb_ary_rotate_bang(int argc, VALUE *argv, VALUE ary)
{
long n = (rb_check_arity(argc, 0, 1) ? NUM2LONG(argv[0]) : 1);
rb_ary_rotate(ary, n);
return ary;
}
static VALUE
rb_ary_rotate_m(int argc, VALUE *argv, VALUE ary)
{
VALUE rotated;
const VALUE *ptr;
long len;
long cnt = (rb_check_arity(argc, 0, 1) ? NUM2LONG(argv[0]) : 1);
len = RARRAY_LEN(ary);
rotated = rb_ary_new2(len);
if (len > 0) {
cnt = rotate_count(cnt, len);
ptr = RARRAY_CONST_PTR_TRANSIENT(ary);
len -= cnt;
ary_memcpy(rotated, 0, len, ptr + cnt);
ary_memcpy(rotated, len, cnt, ptr);
}
ARY_SET_LEN(rotated, RARRAY_LEN(ary));
return rotated;
}
struct ary_sort_data {
VALUE ary;
struct cmp_opt_data cmp_opt;
};
static VALUE
sort_reentered(VALUE ary)
{
if (RBASIC(ary)->klass) {
rb_raise(rb_eRuntimeError, "sort reentered");
}
return Qnil;
}
static int
sort_1(const void *ap, const void *bp, void *dummy)
{
struct ary_sort_data *data = dummy;
VALUE retval = sort_reentered(data->ary);
VALUE a = *(const VALUE *)ap, b = *(const VALUE *)bp;
VALUE args[2];
int n;
args[0] = a;
args[1] = b;
retval = rb_yield_values2(2, args);
n = rb_cmpint(retval, a, b);
sort_reentered(data->ary);
return n;
}
static int
sort_2(const void *ap, const void *bp, void *dummy)
{
struct ary_sort_data *data = dummy;
VALUE retval = sort_reentered(data->ary);
VALUE a = *(const VALUE *)ap, b = *(const VALUE *)bp;
int n;
if (FIXNUM_P(a) && FIXNUM_P(b) && CMP_OPTIMIZABLE(data->cmp_opt, Integer)) {
if ((long)a > (long)b) return 1;
if ((long)a < (long)b) return -1;
return 0;
}
if (STRING_P(a) && STRING_P(b) && CMP_OPTIMIZABLE(data->cmp_opt, String)) {
return rb_str_cmp(a, b);
}
if (RB_FLOAT_TYPE_P(a) && CMP_OPTIMIZABLE(data->cmp_opt, Float)) {
return rb_float_cmp(a, b);
}
retval = rb_funcallv(a, id_cmp, 1, &b);
n = rb_cmpint(retval, a, b);
sort_reentered(data->ary);
return n;
}
VALUE
rb_ary_sort_bang(VALUE ary)
{
rb_ary_modify(ary);
assert(!ARY_SHARED_P(ary));
if (RARRAY_LEN(ary) > 1) {
VALUE tmp = ary_make_substitution(ary); 
struct ary_sort_data data;
long len = RARRAY_LEN(ary);
RBASIC_CLEAR_CLASS(tmp);
data.ary = tmp;
data.cmp_opt.opt_methods = 0;
data.cmp_opt.opt_inited = 0;
RARRAY_PTR_USE(tmp, ptr, {
ruby_qsort(ptr, len, sizeof(VALUE),
rb_block_given_p()?sort_1:sort_2, &data);
}); 
rb_ary_modify(ary);
if (ARY_EMBED_P(tmp)) {
if (ARY_SHARED_P(ary)) { 
rb_ary_unshare(ary);
FL_SET_EMBED(ary);
}
ary_memcpy(ary, 0, ARY_EMBED_LEN(tmp), ARY_EMBED_PTR(tmp));
ARY_SET_LEN(ary, ARY_EMBED_LEN(tmp));
}
else {
if (!ARY_EMBED_P(ary) && ARY_HEAP_PTR(ary) == ARY_HEAP_PTR(tmp)) {
FL_UNSET_SHARED(ary);
ARY_SET_CAPA(ary, RARRAY_LEN(tmp));
}
else {
assert(!ARY_SHARED_P(tmp));
if (ARY_EMBED_P(ary)) {
FL_UNSET_EMBED(ary);
}
else if (ARY_SHARED_P(ary)) {
rb_ary_unshare(ary);
}
else {
ary_heap_free(ary);
}
ARY_SET_PTR(ary, ARY_HEAP_PTR(tmp));
ARY_SET_HEAP_LEN(ary, len);
ARY_SET_CAPA(ary, ARY_HEAP_LEN(tmp));
}
FL_UNSET(tmp, FL_FREEZE);
FL_SET_EMBED(tmp);
ARY_SET_EMBED_LEN(tmp, 0);
FL_SET(tmp, FL_FREEZE);
}
RBASIC_SET_CLASS_RAW(tmp, rb_cArray); 
}
ary_verify(ary);
return ary;
}
VALUE
rb_ary_sort(VALUE ary)
{
ary = rb_ary_dup(ary);
rb_ary_sort_bang(ary);
return ary;
}
static VALUE rb_ary_bsearch_index(VALUE ary);
static VALUE
rb_ary_bsearch(VALUE ary)
{
VALUE index_result = rb_ary_bsearch_index(ary);
if (FIXNUM_P(index_result)) {
return rb_ary_entry(ary, FIX2LONG(index_result));
}
return index_result;
}
static VALUE
rb_ary_bsearch_index(VALUE ary)
{
long low = 0, high = RARRAY_LEN(ary), mid;
int smaller = 0, satisfied = 0;
VALUE v, val;
RETURN_ENUMERATOR(ary, 0, 0);
while (low < high) {
mid = low + ((high - low) / 2);
val = rb_ary_entry(ary, mid);
v = rb_yield(val);
if (FIXNUM_P(v)) {
if (v == INT2FIX(0)) return INT2FIX(mid);
smaller = (SIGNED_VALUE)v < 0; 
}
else if (v == Qtrue) {
satisfied = 1;
smaller = 1;
}
else if (v == Qfalse || v == Qnil) {
smaller = 0;
}
else if (rb_obj_is_kind_of(v, rb_cNumeric)) {
const VALUE zero = INT2FIX(0);
switch (rb_cmpint(rb_funcallv(v, id_cmp, 1, &zero), v, zero)) {
case 0: return INT2FIX(mid);
case 1: smaller = 1; break;
case -1: smaller = 0;
}
}
else {
rb_raise(rb_eTypeError, "wrong argument type %"PRIsVALUE
" (must be numeric, true, false or nil)",
rb_obj_class(v));
}
if (smaller) {
high = mid;
}
else {
low = mid + 1;
}
}
if (!satisfied) return Qnil;
return INT2FIX(low);
}
static VALUE
sort_by_i(RB_BLOCK_CALL_FUNC_ARGLIST(i, dummy))
{
return rb_yield(i);
}
static VALUE
rb_ary_sort_by_bang(VALUE ary)
{
VALUE sorted;
RETURN_SIZED_ENUMERATOR(ary, 0, 0, ary_enum_length);
rb_ary_modify(ary);
sorted = rb_block_call(ary, rb_intern("sort_by"), 0, 0, sort_by_i, 0);
rb_ary_replace(ary, sorted);
return ary;
}
static VALUE
rb_ary_collect(VALUE ary)
{
long i;
VALUE collect;
RETURN_SIZED_ENUMERATOR(ary, 0, 0, ary_enum_length);
collect = rb_ary_new2(RARRAY_LEN(ary));
for (i = 0; i < RARRAY_LEN(ary); i++) {
rb_ary_push(collect, rb_yield(RARRAY_AREF(ary, i)));
}
return collect;
}
static VALUE
rb_ary_collect_bang(VALUE ary)
{
long i;
RETURN_SIZED_ENUMERATOR(ary, 0, 0, ary_enum_length);
rb_ary_modify(ary);
for (i = 0; i < RARRAY_LEN(ary); i++) {
rb_ary_store(ary, i, rb_yield(RARRAY_AREF(ary, i)));
}
return ary;
}
VALUE
rb_get_values_at(VALUE obj, long olen, int argc, const VALUE *argv, VALUE (*func) (VALUE, long))
{
VALUE result = rb_ary_new2(argc);
long beg, len, i, j;
for (i=0; i<argc; i++) {
if (FIXNUM_P(argv[i])) {
rb_ary_push(result, (*func)(obj, FIX2LONG(argv[i])));
continue;
}
if (rb_range_beg_len(argv[i], &beg, &len, olen, 1)) {
long end = olen < beg+len ? olen : beg+len;
for (j = beg; j < end; j++) {
rb_ary_push(result, (*func)(obj, j));
}
if (beg + len > j)
rb_ary_resize(result, RARRAY_LEN(result) + (beg + len) - j);
continue;
}
rb_ary_push(result, (*func)(obj, NUM2LONG(argv[i])));
}
return result;
}
static VALUE
append_values_at_single(VALUE result, VALUE ary, long olen, VALUE idx)
{
long beg, len;
if (FIXNUM_P(idx)) {
beg = FIX2LONG(idx);
}
else if (rb_range_beg_len(idx, &beg, &len, olen, 1)) {
if (len > 0) {
const VALUE *const src = RARRAY_CONST_PTR_TRANSIENT(ary);
const long end = beg + len;
const long prevlen = RARRAY_LEN(result);
if (beg < olen) {
rb_ary_cat(result, src + beg, end > olen ? olen-beg : len);
}
if (end > olen) {
rb_ary_store(result, prevlen + len - 1, Qnil);
}
}
return result;
}
else {
beg = NUM2LONG(idx);
}
return rb_ary_push(result, rb_ary_entry(ary, beg));
}
static VALUE
rb_ary_values_at(int argc, VALUE *argv, VALUE ary)
{
long i, olen = RARRAY_LEN(ary);
VALUE result = rb_ary_new_capa(argc);
for (i = 0; i < argc; ++i) {
append_values_at_single(result, ary, olen, argv[i]);
}
RB_GC_GUARD(ary);
return result;
}
static VALUE
rb_ary_select(VALUE ary)
{
VALUE result;
long i;
RETURN_SIZED_ENUMERATOR(ary, 0, 0, ary_enum_length);
result = rb_ary_new2(RARRAY_LEN(ary));
for (i = 0; i < RARRAY_LEN(ary); i++) {
if (RTEST(rb_yield(RARRAY_AREF(ary, i)))) {
rb_ary_push(result, rb_ary_elt(ary, i));
}
}
return result;
}
struct select_bang_arg {
VALUE ary;
long len[2];
};
static VALUE
select_bang_i(VALUE a)
{
volatile struct select_bang_arg *arg = (void *)a;
VALUE ary = arg->ary;
long i1, i2;
for (i1 = i2 = 0; i1 < RARRAY_LEN(ary); arg->len[0] = ++i1) {
VALUE v = RARRAY_AREF(ary, i1);
if (!RTEST(rb_yield(v))) continue;
if (i1 != i2) {
rb_ary_store(ary, i2, v);
}
arg->len[1] = ++i2;
}
return (i1 == i2) ? Qnil : ary;
}
static VALUE
select_bang_ensure(VALUE a)
{
volatile struct select_bang_arg *arg = (void *)a;
VALUE ary = arg->ary;
long len = RARRAY_LEN(ary);
long i1 = arg->len[0], i2 = arg->len[1];
if (i2 < len && i2 < i1) {
long tail = 0;
if (i1 < len) {
tail = len - i1;
RARRAY_PTR_USE_TRANSIENT(ary, ptr, {
MEMMOVE(ptr + i2, ptr + i1, VALUE, tail);
});
}
ARY_SET_LEN(ary, i2 + tail);
}
return ary;
}
static VALUE
rb_ary_select_bang(VALUE ary)
{
struct select_bang_arg args;
RETURN_SIZED_ENUMERATOR(ary, 0, 0, ary_enum_length);
rb_ary_modify(ary);
args.ary = ary;
args.len[0] = args.len[1] = 0;
return rb_ensure(select_bang_i, (VALUE)&args, select_bang_ensure, (VALUE)&args);
}
static VALUE
rb_ary_keep_if(VALUE ary)
{
RETURN_SIZED_ENUMERATOR(ary, 0, 0, ary_enum_length);
rb_ary_select_bang(ary);
return ary;
}
static void
ary_resize_smaller(VALUE ary, long len)
{
rb_ary_modify(ary);
if (RARRAY_LEN(ary) > len) {
ARY_SET_LEN(ary, len);
if (len * 2 < ARY_CAPA(ary) &&
ARY_CAPA(ary) > ARY_DEFAULT_SIZE) {
ary_resize_capa(ary, len * 2);
}
}
}
VALUE
rb_ary_delete(VALUE ary, VALUE item)
{
VALUE v = item;
long i1, i2;
for (i1 = i2 = 0; i1 < RARRAY_LEN(ary); i1++) {
VALUE e = RARRAY_AREF(ary, i1);
if (rb_equal(e, item)) {
v = e;
continue;
}
if (i1 != i2) {
rb_ary_store(ary, i2, e);
}
i2++;
}
if (RARRAY_LEN(ary) == i2) {
if (rb_block_given_p()) {
return rb_yield(item);
}
return Qnil;
}
ary_resize_smaller(ary, i2);
ary_verify(ary);
return v;
}
void
rb_ary_delete_same(VALUE ary, VALUE item)
{
long i1, i2;
for (i1 = i2 = 0; i1 < RARRAY_LEN(ary); i1++) {
VALUE e = RARRAY_AREF(ary, i1);
if (e == item) {
continue;
}
if (i1 != i2) {
rb_ary_store(ary, i2, e);
}
i2++;
}
if (RARRAY_LEN(ary) == i2) {
return;
}
ary_resize_smaller(ary, i2);
}
VALUE
rb_ary_delete_at(VALUE ary, long pos)
{
long len = RARRAY_LEN(ary);
VALUE del;
if (pos >= len) return Qnil;
if (pos < 0) {
pos += len;
if (pos < 0) return Qnil;
}
rb_ary_modify(ary);
del = RARRAY_AREF(ary, pos);
RARRAY_PTR_USE_TRANSIENT(ary, ptr, {
MEMMOVE(ptr+pos, ptr+pos+1, VALUE, len-pos-1);
});
ARY_INCREASE_LEN(ary, -1);
ary_verify(ary);
return del;
}
static VALUE
rb_ary_delete_at_m(VALUE ary, VALUE pos)
{
return rb_ary_delete_at(ary, NUM2LONG(pos));
}
static VALUE
rb_ary_slice_bang(int argc, VALUE *argv, VALUE ary)
{
VALUE arg1, arg2;
long pos, len, orig_len;
rb_ary_modify_check(ary);
if (argc == 2) {
pos = NUM2LONG(argv[0]);
len = NUM2LONG(argv[1]);
delete_pos_len:
if (len < 0) return Qnil;
orig_len = RARRAY_LEN(ary);
if (pos < 0) {
pos += orig_len;
if (pos < 0) return Qnil;
}
else if (orig_len < pos) return Qnil;
if (orig_len < pos + len) {
len = orig_len - pos;
}
if (len == 0) return rb_ary_new2(0);
arg2 = rb_ary_new4(len, RARRAY_CONST_PTR_TRANSIENT(ary)+pos);
RBASIC_SET_CLASS(arg2, rb_obj_class(ary));
rb_ary_splice(ary, pos, len, 0, 0);
return arg2;
}
rb_check_arity(argc, 1, 2);
arg1 = argv[0];
if (!FIXNUM_P(arg1)) {
switch (rb_range_beg_len(arg1, &pos, &len, RARRAY_LEN(ary), 0)) {
case Qtrue:
goto delete_pos_len;
case Qnil:
return Qnil;
default:
break;
}
}
return rb_ary_delete_at(ary, NUM2LONG(arg1));
}
static VALUE
ary_reject(VALUE orig, VALUE result)
{
long i;
for (i = 0; i < RARRAY_LEN(orig); i++) {
VALUE v = RARRAY_AREF(orig, i);
if (!RTEST(rb_yield(v))) {
rb_ary_push(result, v);
}
}
return result;
}
static VALUE
reject_bang_i(VALUE a)
{
volatile struct select_bang_arg *arg = (void *)a;
VALUE ary = arg->ary;
long i1, i2;
for (i1 = i2 = 0; i1 < RARRAY_LEN(ary); arg->len[0] = ++i1) {
VALUE v = RARRAY_AREF(ary, i1);
if (RTEST(rb_yield(v))) continue;
if (i1 != i2) {
rb_ary_store(ary, i2, v);
}
arg->len[1] = ++i2;
}
return (i1 == i2) ? Qnil : ary;
}
static VALUE
ary_reject_bang(VALUE ary)
{
struct select_bang_arg args;
rb_ary_modify_check(ary);
args.ary = ary;
args.len[0] = args.len[1] = 0;
return rb_ensure(reject_bang_i, (VALUE)&args, select_bang_ensure, (VALUE)&args);
}
static VALUE
rb_ary_reject_bang(VALUE ary)
{
RETURN_SIZED_ENUMERATOR(ary, 0, 0, ary_enum_length);
rb_ary_modify(ary);
return ary_reject_bang(ary);
}
static VALUE
rb_ary_reject(VALUE ary)
{
VALUE rejected_ary;
RETURN_SIZED_ENUMERATOR(ary, 0, 0, ary_enum_length);
rejected_ary = rb_ary_new();
ary_reject(ary, rejected_ary);
return rejected_ary;
}
static VALUE
rb_ary_delete_if(VALUE ary)
{
ary_verify(ary);
RETURN_SIZED_ENUMERATOR(ary, 0, 0, ary_enum_length);
ary_reject_bang(ary);
return ary;
}
static VALUE
take_i(RB_BLOCK_CALL_FUNC_ARGLIST(val, cbarg))
{
VALUE *args = (VALUE *)cbarg;
if (args[1] == 0) rb_iter_break();
else args[1]--;
if (argc > 1) val = rb_ary_new4(argc, argv);
rb_ary_push(args[0], val);
return Qnil;
}
static VALUE
take_items(VALUE obj, long n)
{
VALUE result = rb_check_array_type(obj);
VALUE args[2];
if (!NIL_P(result)) return rb_ary_subseq(result, 0, n);
result = rb_ary_new2(n);
args[0] = result; args[1] = (VALUE)n;
if (rb_check_block_call(obj, idEach, 0, 0, take_i, (VALUE)args) == Qundef)
rb_raise(rb_eTypeError, "wrong argument type %"PRIsVALUE" (must respond to :each)",
rb_obj_class(obj));
return result;
}
static VALUE
rb_ary_zip(int argc, VALUE *argv, VALUE ary)
{
int i, j;
long len = RARRAY_LEN(ary);
VALUE result = Qnil;
for (i=0; i<argc; i++) {
argv[i] = take_items(argv[i], len);
}
if (rb_block_given_p()) {
int arity = rb_block_arity();
if (arity > 1) {
VALUE work, *tmp;
tmp = ALLOCV_N(VALUE, work, argc+1);
for (i=0; i<RARRAY_LEN(ary); i++) {
tmp[0] = RARRAY_AREF(ary, i);
for (j=0; j<argc; j++) {
tmp[j+1] = rb_ary_elt(argv[j], i);
}
rb_yield_values2(argc+1, tmp);
}
if (work) ALLOCV_END(work);
}
else {
for (i=0; i<RARRAY_LEN(ary); i++) {
VALUE tmp = rb_ary_new2(argc+1);
rb_ary_push(tmp, RARRAY_AREF(ary, i));
for (j=0; j<argc; j++) {
rb_ary_push(tmp, rb_ary_elt(argv[j], i));
}
rb_yield(tmp);
}
}
}
else {
result = rb_ary_new_capa(len);
for (i=0; i<len; i++) {
VALUE tmp = rb_ary_new_capa(argc+1);
rb_ary_push(tmp, RARRAY_AREF(ary, i));
for (j=0; j<argc; j++) {
rb_ary_push(tmp, rb_ary_elt(argv[j], i));
}
rb_ary_push(result, tmp);
}
}
return result;
}
static VALUE
rb_ary_transpose(VALUE ary)
{
long elen = -1, alen, i, j;
VALUE tmp, result = 0;
alen = RARRAY_LEN(ary);
if (alen == 0) return rb_ary_dup(ary);
for (i=0; i<alen; i++) {
tmp = to_ary(rb_ary_elt(ary, i));
if (elen < 0) { 
elen = RARRAY_LEN(tmp);
result = rb_ary_new2(elen);
for (j=0; j<elen; j++) {
rb_ary_store(result, j, rb_ary_new2(alen));
}
}
else if (elen != RARRAY_LEN(tmp)) {
rb_raise(rb_eIndexError, "element size differs (%ld should be %ld)",
RARRAY_LEN(tmp), elen);
}
for (j=0; j<elen; j++) {
rb_ary_store(rb_ary_elt(result, j), i, rb_ary_elt(tmp, j));
}
}
return result;
}
VALUE
rb_ary_replace(VALUE copy, VALUE orig)
{
rb_ary_modify_check(copy);
orig = to_ary(orig);
if (copy == orig) return copy;
if (RARRAY_LEN(orig) <= RARRAY_EMBED_LEN_MAX) {
VALUE shared_root = 0;
if (ARY_OWNS_HEAP_P(copy)) {
ary_heap_free(copy);
}
else if (ARY_SHARED_P(copy)) {
shared_root = ARY_SHARED_ROOT(copy);
FL_UNSET_SHARED(copy);
}
FL_SET_EMBED(copy);
ary_memcpy(copy, 0, RARRAY_LEN(orig), RARRAY_CONST_PTR_TRANSIENT(orig));
if (shared_root) {
rb_ary_decrement_share(shared_root);
}
ARY_SET_LEN(copy, RARRAY_LEN(orig));
}
else {
VALUE shared_root = ary_make_shared(orig);
if (ARY_OWNS_HEAP_P(copy)) {
ary_heap_free(copy);
}
else {
rb_ary_unshare_safe(copy);
}
FL_UNSET_EMBED(copy);
ARY_SET_PTR(copy, ARY_HEAP_PTR(orig));
ARY_SET_LEN(copy, ARY_HEAP_LEN(orig));
rb_ary_set_shared(copy, shared_root);
}
ary_verify(copy);
return copy;
}
VALUE
rb_ary_clear(VALUE ary)
{
rb_ary_modify_check(ary);
if (ARY_SHARED_P(ary)) {
if (!ARY_EMBED_P(ary)) {
rb_ary_unshare(ary);
FL_SET_EMBED(ary);
ARY_SET_EMBED_LEN(ary, 0);
}
}
else {
ARY_SET_LEN(ary, 0);
if (ARY_DEFAULT_SIZE * 2 < ARY_CAPA(ary)) {
ary_resize_capa(ary, ARY_DEFAULT_SIZE * 2);
}
}
ary_verify(ary);
return ary;
}
static VALUE
rb_ary_fill(int argc, VALUE *argv, VALUE ary)
{
VALUE item = Qundef, arg1, arg2;
long beg = 0, end = 0, len = 0;
if (rb_block_given_p()) {
rb_scan_args(argc, argv, "02", &arg1, &arg2);
argc += 1; 
}
else {
rb_scan_args(argc, argv, "12", &item, &arg1, &arg2);
}
switch (argc) {
case 1:
beg = 0;
len = RARRAY_LEN(ary);
break;
case 2:
if (rb_range_beg_len(arg1, &beg, &len, RARRAY_LEN(ary), 1)) {
break;
}
case 3:
beg = NIL_P(arg1) ? 0 : NUM2LONG(arg1);
if (beg < 0) {
beg = RARRAY_LEN(ary) + beg;
if (beg < 0) beg = 0;
}
len = NIL_P(arg2) ? RARRAY_LEN(ary) - beg : NUM2LONG(arg2);
break;
}
rb_ary_modify(ary);
if (len < 0) {
return ary;
}
if (beg >= ARY_MAX_SIZE || len > ARY_MAX_SIZE - beg) {
rb_raise(rb_eArgError, "argument too big");
}
end = beg + len;
if (RARRAY_LEN(ary) < end) {
if (end >= ARY_CAPA(ary)) {
ary_resize_capa(ary, end);
}
ary_mem_clear(ary, RARRAY_LEN(ary), end - RARRAY_LEN(ary));
ARY_SET_LEN(ary, end);
}
if (item == Qundef) {
VALUE v;
long i;
for (i=beg; i<end; i++) {
v = rb_yield(LONG2NUM(i));
if (i>=RARRAY_LEN(ary)) break;
ARY_SET(ary, i, v);
}
}
else {
ary_memfill(ary, beg, len, item);
}
return ary;
}
VALUE
rb_ary_plus(VALUE x, VALUE y)
{
VALUE z;
long len, xlen, ylen;
y = to_ary(y);
xlen = RARRAY_LEN(x);
ylen = RARRAY_LEN(y);
len = xlen + ylen;
z = rb_ary_new2(len);
ary_memcpy(z, 0, xlen, RARRAY_CONST_PTR_TRANSIENT(x));
ary_memcpy(z, xlen, ylen, RARRAY_CONST_PTR_TRANSIENT(y));
ARY_SET_LEN(z, len);
return z;
}
static VALUE
ary_append(VALUE x, VALUE y)
{
long n = RARRAY_LEN(y);
if (n > 0) {
rb_ary_splice(x, RARRAY_LEN(x), 0, RARRAY_CONST_PTR_TRANSIENT(y), n);
}
return x;
}
static VALUE
rb_ary_concat_multi(int argc, VALUE *argv, VALUE ary)
{
rb_ary_modify_check(ary);
if (argc == 1) {
rb_ary_concat(ary, argv[0]);
}
else if (argc > 1) {
int i;
VALUE args = rb_ary_tmp_new(argc);
for (i = 0; i < argc; i++) {
rb_ary_concat(args, argv[i]);
}
ary_append(ary, args);
}
ary_verify(ary);
return ary;
}
VALUE
rb_ary_concat(VALUE x, VALUE y)
{
return ary_append(x, to_ary(y));
}
static VALUE
rb_ary_times(VALUE ary, VALUE times)
{
VALUE ary2, tmp;
const VALUE *ptr;
long t, len;
tmp = rb_check_string_type(times);
if (!NIL_P(tmp)) {
return rb_ary_join(ary, tmp);
}
len = NUM2LONG(times);
if (len == 0) {
ary2 = ary_new(rb_obj_class(ary), 0);
goto out;
}
if (len < 0) {
rb_raise(rb_eArgError, "negative argument");
}
if (ARY_MAX_SIZE/len < RARRAY_LEN(ary)) {
rb_raise(rb_eArgError, "argument too big");
}
len *= RARRAY_LEN(ary);
ary2 = ary_new(rb_obj_class(ary), len);
ARY_SET_LEN(ary2, len);
ptr = RARRAY_CONST_PTR_TRANSIENT(ary);
t = RARRAY_LEN(ary);
if (0 < t) {
ary_memcpy(ary2, 0, t, ptr);
while (t <= len/2) {
ary_memcpy(ary2, t, t, RARRAY_CONST_PTR_TRANSIENT(ary2));
t *= 2;
}
if (t < len) {
ary_memcpy(ary2, t, len-t, RARRAY_CONST_PTR_TRANSIENT(ary2));
}
}
out:
return ary2;
}
VALUE
rb_ary_assoc(VALUE ary, VALUE key)
{
long i;
VALUE v;
for (i = 0; i < RARRAY_LEN(ary); ++i) {
v = rb_check_array_type(RARRAY_AREF(ary, i));
if (!NIL_P(v) && RARRAY_LEN(v) > 0 &&
rb_equal(RARRAY_AREF(v, 0), key))
return v;
}
return Qnil;
}
VALUE
rb_ary_rassoc(VALUE ary, VALUE value)
{
long i;
VALUE v;
for (i = 0; i < RARRAY_LEN(ary); ++i) {
v = RARRAY_AREF(ary, i);
if (RB_TYPE_P(v, T_ARRAY) &&
RARRAY_LEN(v) > 1 &&
rb_equal(RARRAY_AREF(v, 1), value))
return v;
}
return Qnil;
}
static VALUE
recursive_equal(VALUE ary1, VALUE ary2, int recur)
{
long i, len1;
const VALUE *p1, *p2;
if (recur) return Qtrue; 
p1 = RARRAY_CONST_PTR(ary1);
p2 = RARRAY_CONST_PTR(ary2);
len1 = RARRAY_LEN(ary1);
for (i = 0; i < len1; i++) {
if (*p1 != *p2) {
if (rb_equal(*p1, *p2)) {
len1 = RARRAY_LEN(ary1);
if (len1 != RARRAY_LEN(ary2))
return Qfalse;
if (len1 < i)
return Qtrue;
p1 = RARRAY_CONST_PTR(ary1) + i;
p2 = RARRAY_CONST_PTR(ary2) + i;
}
else {
return Qfalse;
}
}
p1++;
p2++;
}
return Qtrue;
}
static VALUE
rb_ary_equal(VALUE ary1, VALUE ary2)
{
if (ary1 == ary2) return Qtrue;
if (!RB_TYPE_P(ary2, T_ARRAY)) {
if (!rb_respond_to(ary2, idTo_ary)) {
return Qfalse;
}
return rb_equal(ary2, ary1);
}
if (RARRAY_LEN(ary1) != RARRAY_LEN(ary2)) return Qfalse;
if (RARRAY_CONST_PTR_TRANSIENT(ary1) == RARRAY_CONST_PTR_TRANSIENT(ary2)) return Qtrue;
return rb_exec_recursive_paired(recursive_equal, ary1, ary2, ary2);
}
static VALUE
recursive_eql(VALUE ary1, VALUE ary2, int recur)
{
long i;
if (recur) return Qtrue; 
for (i=0; i<RARRAY_LEN(ary1); i++) {
if (!rb_eql(rb_ary_elt(ary1, i), rb_ary_elt(ary2, i)))
return Qfalse;
}
return Qtrue;
}
static VALUE
rb_ary_eql(VALUE ary1, VALUE ary2)
{
if (ary1 == ary2) return Qtrue;
if (!RB_TYPE_P(ary2, T_ARRAY)) return Qfalse;
if (RARRAY_LEN(ary1) != RARRAY_LEN(ary2)) return Qfalse;
if (RARRAY_CONST_PTR_TRANSIENT(ary1) == RARRAY_CONST_PTR_TRANSIENT(ary2)) return Qtrue;
return rb_exec_recursive_paired(recursive_eql, ary1, ary2, ary2);
}
static VALUE
rb_ary_hash(VALUE ary)
{
long i;
st_index_t h;
VALUE n;
h = rb_hash_start(RARRAY_LEN(ary));
h = rb_hash_uint(h, (st_index_t)rb_ary_hash);
for (i=0; i<RARRAY_LEN(ary); i++) {
n = rb_hash(RARRAY_AREF(ary, i));
h = rb_hash_uint(h, NUM2LONG(n));
}
h = rb_hash_end(h);
return ST2FIX(h);
}
VALUE
rb_ary_includes(VALUE ary, VALUE item)
{
long i;
VALUE e;
for (i=0; i<RARRAY_LEN(ary); i++) {
e = RARRAY_AREF(ary, i);
if (rb_equal(e, item)) {
return Qtrue;
}
}
return Qfalse;
}
static VALUE
rb_ary_includes_by_eql(VALUE ary, VALUE item)
{
long i;
VALUE e;
for (i=0; i<RARRAY_LEN(ary); i++) {
e = RARRAY_AREF(ary, i);
if (rb_eql(item, e)) {
return Qtrue;
}
}
return Qfalse;
}
static VALUE
recursive_cmp(VALUE ary1, VALUE ary2, int recur)
{
long i, len;
if (recur) return Qundef; 
len = RARRAY_LEN(ary1);
if (len > RARRAY_LEN(ary2)) {
len = RARRAY_LEN(ary2);
}
for (i=0; i<len; i++) {
VALUE e1 = rb_ary_elt(ary1, i), e2 = rb_ary_elt(ary2, i);
VALUE v = rb_funcallv(e1, id_cmp, 1, &e2);
if (v != INT2FIX(0)) {
return v;
}
}
return Qundef;
}
VALUE
rb_ary_cmp(VALUE ary1, VALUE ary2)
{
long len;
VALUE v;
ary2 = rb_check_array_type(ary2);
if (NIL_P(ary2)) return Qnil;
if (ary1 == ary2) return INT2FIX(0);
v = rb_exec_recursive_paired(recursive_cmp, ary1, ary2, ary2);
if (v != Qundef) return v;
len = RARRAY_LEN(ary1) - RARRAY_LEN(ary2);
if (len == 0) return INT2FIX(0);
if (len > 0) return INT2FIX(1);
return INT2FIX(-1);
}
static VALUE
ary_add_hash(VALUE hash, VALUE ary)
{
long i;
for (i=0; i<RARRAY_LEN(ary); i++) {
VALUE elt = RARRAY_AREF(ary, i);
rb_hash_add_new_element(hash, elt, elt);
}
return hash;
}
static inline VALUE
ary_tmp_hash_new(VALUE ary)
{
long size = RARRAY_LEN(ary);
VALUE hash = rb_hash_new_with_size(size);
RBASIC_CLEAR_CLASS(hash);
return hash;
}
static VALUE
ary_make_hash(VALUE ary)
{
VALUE hash = ary_tmp_hash_new(ary);
return ary_add_hash(hash, ary);
}
static VALUE
ary_add_hash_by(VALUE hash, VALUE ary)
{
long i;
for (i = 0; i < RARRAY_LEN(ary); ++i) {
VALUE v = rb_ary_elt(ary, i), k = rb_yield(v);
rb_hash_add_new_element(hash, k, v);
}
return hash;
}
static VALUE
ary_make_hash_by(VALUE ary)
{
VALUE hash = ary_tmp_hash_new(ary);
return ary_add_hash_by(hash, ary);
}
static inline void
ary_recycle_hash(VALUE hash)
{
assert(RBASIC_CLASS(hash) == 0);
if (RHASH_ST_TABLE_P(hash)) {
st_table *tbl = RHASH_ST_TABLE(hash);
st_free_table(tbl);
RHASH_ST_CLEAR(hash);
}
}
static VALUE
rb_ary_diff(VALUE ary1, VALUE ary2)
{
VALUE ary3;
VALUE hash;
long i;
ary2 = to_ary(ary2);
ary3 = rb_ary_new();
if (RARRAY_LEN(ary1) <= SMALL_ARRAY_LEN || RARRAY_LEN(ary2) <= SMALL_ARRAY_LEN) {
for (i=0; i<RARRAY_LEN(ary1); i++) {
VALUE elt = rb_ary_elt(ary1, i);
if (rb_ary_includes_by_eql(ary2, elt)) continue;
rb_ary_push(ary3, elt);
}
return ary3;
}
hash = ary_make_hash(ary2);
for (i=0; i<RARRAY_LEN(ary1); i++) {
if (rb_hash_stlike_lookup(hash, RARRAY_AREF(ary1, i), NULL)) continue;
rb_ary_push(ary3, rb_ary_elt(ary1, i));
}
ary_recycle_hash(hash);
return ary3;
}
static VALUE
rb_ary_difference_multi(int argc, VALUE *argv, VALUE ary)
{
VALUE ary_diff;
long i, length;
volatile VALUE t0;
bool *is_hash = ALLOCV_N(bool, t0, argc);
ary_diff = rb_ary_new();
length = RARRAY_LEN(ary);
for (i = 0; i < argc; i++) {
argv[i] = to_ary(argv[i]);
is_hash[i] = (length > SMALL_ARRAY_LEN && RARRAY_LEN(argv[i]) > SMALL_ARRAY_LEN);
if (is_hash[i]) argv[i] = ary_make_hash(argv[i]);
}
for (i = 0; i < RARRAY_LEN(ary); i++) {
int j;
VALUE elt = rb_ary_elt(ary, i);
for (j = 0; j < argc; j++) {
if (is_hash[j]) {
if (rb_hash_stlike_lookup(argv[j], RARRAY_AREF(ary, i), NULL))
break;
}
else {
if (rb_ary_includes_by_eql(argv[j], elt)) break;
}
}
if (j == argc) rb_ary_push(ary_diff, elt);
}
ALLOCV_END(t0);
return ary_diff;
}
static VALUE
rb_ary_and(VALUE ary1, VALUE ary2)
{
VALUE hash, ary3, v;
st_data_t vv;
long i;
ary2 = to_ary(ary2);
ary3 = rb_ary_new();
if (RARRAY_LEN(ary1) == 0 || RARRAY_LEN(ary2) == 0) return ary3;
if (RARRAY_LEN(ary1) <= SMALL_ARRAY_LEN && RARRAY_LEN(ary2) <= SMALL_ARRAY_LEN) {
for (i=0; i<RARRAY_LEN(ary1); i++) {
v = RARRAY_AREF(ary1, i);
if (!rb_ary_includes_by_eql(ary2, v)) continue;
if (rb_ary_includes_by_eql(ary3, v)) continue;
rb_ary_push(ary3, v);
}
return ary3;
}
hash = ary_make_hash(ary2);
for (i=0; i<RARRAY_LEN(ary1); i++) {
v = RARRAY_AREF(ary1, i);
vv = (st_data_t)v;
if (rb_hash_stlike_delete(hash, &vv, 0)) {
rb_ary_push(ary3, v);
}
}
ary_recycle_hash(hash);
return ary3;
}
static VALUE
rb_ary_intersection_multi(int argc, VALUE *argv, VALUE ary)
{
VALUE result = rb_ary_dup(ary);
int i;
for (i = 0; i < argc; i++) {
result = rb_ary_and(result, argv[i]);
}
return result;
}
static int
ary_hash_orset(st_data_t *key, st_data_t *value, st_data_t arg, int existing)
{
if (existing) return ST_STOP;
*key = *value = (VALUE)arg;
return ST_CONTINUE;
}
static void
rb_ary_union(VALUE ary_union, VALUE ary)
{
long i;
for (i = 0; i < RARRAY_LEN(ary); i++) {
VALUE elt = rb_ary_elt(ary, i);
if (rb_ary_includes_by_eql(ary_union, elt)) continue;
rb_ary_push(ary_union, elt);
}
}
static void
rb_ary_union_hash(VALUE hash, VALUE ary2)
{
long i;
for (i = 0; i < RARRAY_LEN(ary2); i++) {
VALUE elt = RARRAY_AREF(ary2, i);
if (!rb_hash_stlike_update(hash, (st_data_t)elt, ary_hash_orset, (st_data_t)elt)) {
RB_OBJ_WRITTEN(hash, Qundef, elt);
}
}
}
static VALUE
rb_ary_or(VALUE ary1, VALUE ary2)
{
VALUE hash, ary3;
ary2 = to_ary(ary2);
if (RARRAY_LEN(ary1) + RARRAY_LEN(ary2) <= SMALL_ARRAY_LEN) {
ary3 = rb_ary_new();
rb_ary_union(ary3, ary1);
rb_ary_union(ary3, ary2);
return ary3;
}
hash = ary_make_hash(ary1);
rb_ary_union_hash(hash, ary2);
ary3 = rb_hash_values(hash);
ary_recycle_hash(hash);
return ary3;
}
static VALUE
rb_ary_union_multi(int argc, VALUE *argv, VALUE ary)
{
int i;
long sum;
VALUE hash, ary_union;
sum = RARRAY_LEN(ary);
for (i = 0; i < argc; i++) {
argv[i] = to_ary(argv[i]);
sum += RARRAY_LEN(argv[i]);
}
if (sum <= SMALL_ARRAY_LEN) {
ary_union = rb_ary_new();
rb_ary_union(ary_union, ary);
for (i = 0; i < argc; i++) rb_ary_union(ary_union, argv[i]);
return ary_union;
}
hash = ary_make_hash(ary);
for (i = 0; i < argc; i++) rb_ary_union_hash(hash, argv[i]);
ary_union = rb_hash_values(hash);
ary_recycle_hash(hash);
return ary_union;
}
static VALUE
rb_ary_max(int argc, VALUE *argv, VALUE ary)
{
struct cmp_opt_data cmp_opt = { 0, 0 };
VALUE result = Qundef, v;
VALUE num;
long i;
if (rb_check_arity(argc, 0, 1) && !NIL_P(num = argv[0]))
return rb_nmin_run(ary, num, 0, 1, 1);
if (rb_block_given_p()) {
for (i = 0; i < RARRAY_LEN(ary); i++) {
v = RARRAY_AREF(ary, i);
if (result == Qundef || rb_cmpint(rb_yield_values(2, v, result), v, result) > 0) {
result = v;
}
}
}
else {
for (i = 0; i < RARRAY_LEN(ary); i++) {
v = RARRAY_AREF(ary, i);
if (result == Qundef || OPTIMIZED_CMP(v, result, cmp_opt) > 0) {
result = v;
}
}
}
if (result == Qundef) return Qnil;
return result;
}
static VALUE
rb_ary_min(int argc, VALUE *argv, VALUE ary)
{
struct cmp_opt_data cmp_opt = { 0, 0 };
VALUE result = Qundef, v;
VALUE num;
long i;
if (rb_check_arity(argc, 0, 1) && !NIL_P(num = argv[0]))
return rb_nmin_run(ary, num, 0, 0, 1);
if (rb_block_given_p()) {
for (i = 0; i < RARRAY_LEN(ary); i++) {
v = RARRAY_AREF(ary, i);
if (result == Qundef || rb_cmpint(rb_yield_values(2, v, result), v, result) < 0) {
result = v;
}
}
}
else {
for (i = 0; i < RARRAY_LEN(ary); i++) {
v = RARRAY_AREF(ary, i);
if (result == Qundef || OPTIMIZED_CMP(v, result, cmp_opt) < 0) {
result = v;
}
}
}
if (result == Qundef) return Qnil;
return result;
}
static VALUE
rb_ary_minmax(VALUE ary)
{
if (rb_block_given_p()) {
return rb_call_super(0, NULL);
}
return rb_assoc_new(rb_ary_min(0, 0, ary), rb_ary_max(0, 0, ary));
}
static int
push_value(st_data_t key, st_data_t val, st_data_t ary)
{
rb_ary_push((VALUE)ary, (VALUE)val);
return ST_CONTINUE;
}
static VALUE
rb_ary_uniq_bang(VALUE ary)
{
VALUE hash;
long hash_size;
rb_ary_modify_check(ary);
if (RARRAY_LEN(ary) <= 1)
return Qnil;
if (rb_block_given_p())
hash = ary_make_hash_by(ary);
else
hash = ary_make_hash(ary);
hash_size = RHASH_SIZE(hash);
if (RARRAY_LEN(ary) == hash_size) {
return Qnil;
}
rb_ary_modify_check(ary);
ARY_SET_LEN(ary, 0);
if (ARY_SHARED_P(ary) && !ARY_EMBED_P(ary)) {
rb_ary_unshare(ary);
FL_SET_EMBED(ary);
}
ary_resize_capa(ary, hash_size);
rb_hash_foreach(hash, push_value, ary);
ary_recycle_hash(hash);
return ary;
}
static VALUE
rb_ary_uniq(VALUE ary)
{
VALUE hash, uniq;
if (RARRAY_LEN(ary) <= 1) {
hash = 0;
uniq = rb_ary_dup(ary);
}
else if (rb_block_given_p()) {
hash = ary_make_hash_by(ary);
uniq = rb_hash_values(hash);
}
else {
hash = ary_make_hash(ary);
uniq = rb_hash_values(hash);
}
RBASIC_SET_CLASS(uniq, rb_obj_class(ary));
if (hash) {
ary_recycle_hash(hash);
}
return uniq;
}
static VALUE
rb_ary_compact_bang(VALUE ary)
{
VALUE *p, *t, *end;
long n;
rb_ary_modify(ary);
p = t = (VALUE *)RARRAY_CONST_PTR_TRANSIENT(ary); 
end = p + RARRAY_LEN(ary);
while (t < end) {
if (NIL_P(*t)) t++;
else *p++ = *t++;
}
n = p - RARRAY_CONST_PTR_TRANSIENT(ary);
if (RARRAY_LEN(ary) == n) {
return Qnil;
}
ary_resize_smaller(ary, n);
return ary;
}
static VALUE
rb_ary_compact(VALUE ary)
{
ary = rb_ary_dup(ary);
rb_ary_compact_bang(ary);
return ary;
}
static VALUE
rb_ary_count(int argc, VALUE *argv, VALUE ary)
{
long i, n = 0;
if (rb_check_arity(argc, 0, 1) == 0) {
VALUE v;
if (!rb_block_given_p())
return LONG2NUM(RARRAY_LEN(ary));
for (i = 0; i < RARRAY_LEN(ary); i++) {
v = RARRAY_AREF(ary, i);
if (RTEST(rb_yield(v))) n++;
}
}
else {
VALUE obj = argv[0];
if (rb_block_given_p()) {
rb_warn("given block not used");
}
for (i = 0; i < RARRAY_LEN(ary); i++) {
if (rb_equal(RARRAY_AREF(ary, i), obj)) n++;
}
}
return LONG2NUM(n);
}
static VALUE
flatten(VALUE ary, int level)
{
long i;
VALUE stack, result, tmp, elt, vmemo;
st_table *memo;
st_data_t id;
for (i = 0; i < RARRAY_LEN(ary); i++) {
elt = RARRAY_AREF(ary, i);
tmp = rb_check_array_type(elt);
if (!NIL_P(tmp)) {
break;
}
}
if (i == RARRAY_LEN(ary)) {
return ary;
} else if (tmp == ary) {
rb_raise(rb_eArgError, "tried to flatten recursive array");
}
result = ary_new(0, RARRAY_LEN(ary));
ary_memcpy(result, 0, i, RARRAY_CONST_PTR_TRANSIENT(ary));
ARY_SET_LEN(result, i);
stack = ary_new(0, ARY_DEFAULT_SIZE);
rb_ary_push(stack, ary);
rb_ary_push(stack, LONG2NUM(i + 1));
vmemo = rb_hash_new();
RBASIC_CLEAR_CLASS(vmemo);
memo = st_init_numtable();
rb_hash_st_table_set(vmemo, memo);
st_insert(memo, (st_data_t)ary, (st_data_t)Qtrue);
st_insert(memo, (st_data_t)tmp, (st_data_t)Qtrue);
ary = tmp;
i = 0;
while (1) {
while (i < RARRAY_LEN(ary)) {
elt = RARRAY_AREF(ary, i++);
if (level >= 0 && RARRAY_LEN(stack) / 2 >= level) {
rb_ary_push(result, elt);
continue;
}
tmp = rb_check_array_type(elt);
if (RBASIC(result)->klass) {
RB_GC_GUARD(vmemo);
st_clear(memo);
rb_raise(rb_eRuntimeError, "flatten reentered");
}
if (NIL_P(tmp)) {
rb_ary_push(result, elt);
}
else {
id = (st_data_t)tmp;
if (st_is_member(memo, id)) {
st_clear(memo);
rb_raise(rb_eArgError, "tried to flatten recursive array");
}
st_insert(memo, id, (st_data_t)Qtrue);
rb_ary_push(stack, ary);
rb_ary_push(stack, LONG2NUM(i));
ary = tmp;
i = 0;
}
}
if (RARRAY_LEN(stack) == 0) {
break;
}
id = (st_data_t)ary;
st_delete(memo, &id, 0);
tmp = rb_ary_pop(stack);
i = NUM2LONG(tmp);
ary = rb_ary_pop(stack);
}
st_clear(memo);
RBASIC_SET_CLASS(result, rb_obj_class(ary));
return result;
}
static VALUE
rb_ary_flatten_bang(int argc, VALUE *argv, VALUE ary)
{
int mod = 0, level = -1;
VALUE result, lv;
lv = (rb_check_arity(argc, 0, 1) ? argv[0] : Qnil);
rb_ary_modify_check(ary);
if (!NIL_P(lv)) level = NUM2INT(lv);
if (level == 0) return Qnil;
result = flatten(ary, level);
if (result == ary) {
return Qnil;
}
if (!(mod = ARY_EMBED_P(result))) rb_obj_freeze(result);
rb_ary_replace(ary, result);
if (mod) ARY_SET_EMBED_LEN(result, 0);
return ary;
}
static VALUE
rb_ary_flatten(int argc, VALUE *argv, VALUE ary)
{
int level = -1;
VALUE result;
if (rb_check_arity(argc, 0, 1) && !NIL_P(argv[0])) {
level = NUM2INT(argv[0]);
if (level == 0) return ary_make_shared_copy(ary);
}
result = flatten(ary, level);
if (result == ary) {
result = ary_make_shared_copy(ary);
}
return result;
}
#define RAND_UPTO(max) (long)rb_random_ulong_limited((randgen), (max)-1)
static VALUE
rb_ary_shuffle_bang(rb_execution_context_t *ec, VALUE ary, VALUE randgen)
{
long i, len;
rb_ary_modify(ary);
i = len = RARRAY_LEN(ary);
RARRAY_PTR_USE(ary, ptr, {
while (i) {
long j = RAND_UPTO(i);
VALUE tmp;
if (len != RARRAY_LEN(ary) || ptr != RARRAY_CONST_PTR_TRANSIENT(ary)) {
rb_raise(rb_eRuntimeError, "modified during shuffle");
}
tmp = ptr[--i];
ptr[i] = ptr[j];
ptr[j] = tmp;
}
}); 
return ary;
}
static VALUE
rb_ary_shuffle(rb_execution_context_t *ec, VALUE ary, VALUE randgen)
{
ary = rb_ary_dup(ary);
rb_ary_shuffle_bang(ec, ary, randgen);
return ary;
}
static VALUE
rb_ary_sample(rb_execution_context_t *ec, VALUE ary, VALUE randgen, VALUE nv, VALUE to_array)
{
VALUE result;
long n, len, i, j, k, idx[10];
long rnds[numberof(idx)];
long memo_threshold;
len = RARRAY_LEN(ary);
if (!to_array) {
if (len < 2)
i = 0;
else
i = RAND_UPTO(len);
return rb_ary_elt(ary, i);
}
n = NUM2LONG(nv);
if (n < 0) rb_raise(rb_eArgError, "negative sample number");
if (n > len) n = len;
if (n <= numberof(idx)) {
for (i = 0; i < n; ++i) {
rnds[i] = RAND_UPTO(len - i);
}
}
k = len;
len = RARRAY_LEN(ary);
if (len < k && n <= numberof(idx)) {
for (i = 0; i < n; ++i) {
if (rnds[i] >= len) return rb_ary_new_capa(0);
}
}
if (n > len) n = len;
switch (n) {
case 0:
return rb_ary_new_capa(0);
case 1:
i = rnds[0];
return rb_ary_new_from_values(1, &RARRAY_AREF(ary, i));
case 2:
i = rnds[0];
j = rnds[1];
if (j >= i) j++;
return rb_ary_new_from_args(2, RARRAY_AREF(ary, i), RARRAY_AREF(ary, j));
case 3:
i = rnds[0];
j = rnds[1];
k = rnds[2];
{
long l = j, g = i;
if (j >= i) l = i, g = ++j;
if (k >= l && (++k >= g)) ++k;
}
return rb_ary_new_from_args(3, RARRAY_AREF(ary, i), RARRAY_AREF(ary, j), RARRAY_AREF(ary, k));
}
memo_threshold =
len < 2560 ? len / 128 :
len < 5120 ? len / 64 :
len < 10240 ? len / 32 :
len / 16;
if (n <= numberof(idx)) {
long sorted[numberof(idx)];
sorted[0] = idx[0] = rnds[0];
for (i=1; i<n; i++) {
k = rnds[i];
for (j = 0; j < i; ++j) {
if (k < sorted[j]) break;
++k;
}
memmove(&sorted[j+1], &sorted[j], sizeof(sorted[0])*(i-j));
sorted[j] = idx[i] = k;
}
result = rb_ary_new_capa(n);
RARRAY_PTR_USE_TRANSIENT(result, ptr_result, {
for (i=0; i<n; i++) {
ptr_result[i] = RARRAY_AREF(ary, idx[i]);
}
});
}
else if (n <= memo_threshold / 2) {
long max_idx = 0;
#undef RUBY_UNTYPED_DATA_WARNING
#define RUBY_UNTYPED_DATA_WARNING 0
VALUE vmemo = Data_Wrap_Struct(0, 0, st_free_table, 0);
st_table *memo = st_init_numtable_with_size(n);
DATA_PTR(vmemo) = memo;
result = rb_ary_new_capa(n);
RARRAY_PTR_USE(result, ptr_result, {
for (i=0; i<n; i++) {
long r = RAND_UPTO(len-i) + i;
ptr_result[i] = r;
if (r > max_idx) max_idx = r;
}
len = RARRAY_LEN(ary);
if (len <= max_idx) n = 0;
else if (n > len) n = len;
RARRAY_PTR_USE_TRANSIENT(ary, ptr_ary, {
for (i=0; i<n; i++) {
long j2 = j = ptr_result[i];
long i2 = i;
st_data_t value;
if (st_lookup(memo, (st_data_t)i, &value)) i2 = (long)value;
if (st_lookup(memo, (st_data_t)j, &value)) j2 = (long)value;
st_insert(memo, (st_data_t)j, (st_data_t)i2);
ptr_result[i] = ptr_ary[j2];
}
});
});
DATA_PTR(vmemo) = 0;
st_free_table(memo);
}
else {
result = rb_ary_dup(ary);
RBASIC_CLEAR_CLASS(result);
RB_GC_GUARD(ary);
RARRAY_PTR_USE(result, ptr_result, {
for (i=0; i<n; i++) {
j = RAND_UPTO(len-i) + i;
nv = ptr_result[j];
ptr_result[j] = ptr_result[i];
ptr_result[i] = nv;
}
});
RBASIC_SET_CLASS_RAW(result, rb_cArray);
}
ARY_SET_LEN(result, n);
return result;
}
static VALUE
rb_ary_cycle_size(VALUE self, VALUE args, VALUE eobj)
{
long mul;
VALUE n = Qnil;
if (args && (RARRAY_LEN(args) > 0)) {
n = RARRAY_AREF(args, 0);
}
if (RARRAY_LEN(self) == 0) return INT2FIX(0);
if (n == Qnil) return DBL2NUM(HUGE_VAL);
mul = NUM2LONG(n);
if (mul <= 0) return INT2FIX(0);
n = LONG2FIX(mul);
return rb_fix_mul_fix(rb_ary_length(self), n);
}
static VALUE
rb_ary_cycle(int argc, VALUE *argv, VALUE ary)
{
long n, i;
rb_check_arity(argc, 0, 1);
RETURN_SIZED_ENUMERATOR(ary, argc, argv, rb_ary_cycle_size);
if (argc == 0 || NIL_P(argv[0])) {
n = -1;
}
else {
n = NUM2LONG(argv[0]);
if (n <= 0) return Qnil;
}
while (RARRAY_LEN(ary) > 0 && (n < 0 || 0 < n--)) {
for (i=0; i<RARRAY_LEN(ary); i++) {
rb_yield(RARRAY_AREF(ary, i));
}
}
return Qnil;
}
#define tmpary(n) rb_ary_tmp_new(n)
#define tmpary_discard(a) (ary_discard(a), RBASIC_SET_CLASS_RAW(a, rb_cArray))
static int
yield_indexed_values(const VALUE values, const long r, const long *const p)
{
const VALUE result = rb_ary_new2(r);
long i;
for (i = 0; i < r; i++) ARY_SET(result, i, RARRAY_AREF(values, p[i]));
ARY_SET_LEN(result, r);
rb_yield(result);
return !RBASIC(values)->klass;
}
static void
permute0(const long n, const long r, long *const p, char *const used, const VALUE values)
{
long i = 0, index = 0;
for (;;) {
const char *const unused = memchr(&used[i], 0, n-i);
if (!unused) {
if (!index) break;
i = p[--index]; 
used[i++] = 0; 
}
else {
i = unused - used;
p[index] = i;
used[i] = 1; 
++index;
if (index < r-1) { 
p[index] = i = 0;
continue;
}
for (i = 0; i < n; ++i) {
if (used[i]) continue;
p[index] = i;
if (!yield_indexed_values(values, r, p)) {
rb_raise(rb_eRuntimeError, "permute reentered");
}
}
i = p[--index]; 
used[i] = 0; 
p[index] = ++i;
}
}
}
static VALUE
descending_factorial(long from, long how_many)
{
VALUE cnt;
if (how_many > 0) {
cnt = LONG2FIX(from);
while (--how_many > 0) {
long v = --from;
cnt = rb_int_mul(cnt, LONG2FIX(v));
}
}
else {
cnt = LONG2FIX(how_many == 0);
}
return cnt;
}
static VALUE
binomial_coefficient(long comb, long size)
{
VALUE r;
long i;
if (comb > size-comb) {
comb = size-comb;
}
if (comb < 0) {
return LONG2FIX(0);
}
else if (comb == 0) {
return LONG2FIX(1);
}
r = LONG2FIX(size);
for (i = 1; i < comb; ++i) {
r = rb_int_mul(r, LONG2FIX(size - i));
r = rb_int_idiv(r, LONG2FIX(i + 1));
}
return r;
}
static VALUE
rb_ary_permutation_size(VALUE ary, VALUE args, VALUE eobj)
{
long n = RARRAY_LEN(ary);
long k = (args && (RARRAY_LEN(args) > 0)) ? NUM2LONG(RARRAY_AREF(args, 0)) : n;
return descending_factorial(n, k);
}
static VALUE
rb_ary_permutation(int argc, VALUE *argv, VALUE ary)
{
long r, n, i;
n = RARRAY_LEN(ary); 
RETURN_SIZED_ENUMERATOR(ary, argc, argv, rb_ary_permutation_size); 
r = n;
if (rb_check_arity(argc, 0, 1) && !NIL_P(argv[0]))
r = NUM2LONG(argv[0]); 
if (r < 0 || n < r) {
}
else if (r == 0) { 
rb_yield(rb_ary_new2(0));
}
else if (r == 1) { 
for (i = 0; i < RARRAY_LEN(ary); i++) {
rb_yield(rb_ary_new3(1, RARRAY_AREF(ary, i)));
}
}
else { 
volatile VALUE t0;
long *p = ALLOCV_N(long, t0, r+roomof(n, sizeof(long)));
char *used = (char*)(p + r);
VALUE ary0 = ary_make_shared_copy(ary); 
RBASIC_CLEAR_CLASS(ary0);
MEMZERO(used, char, n); 
permute0(n, r, p, used, ary0); 
ALLOCV_END(t0);
RBASIC_SET_CLASS_RAW(ary0, rb_cArray);
}
return ary;
}
static void
combinate0(const long len, const long n, long *const stack, const VALUE values)
{
long lev = 0;
MEMZERO(stack+1, long, n);
stack[0] = -1;
for (;;) {
for (lev++; lev < n; lev++) {
stack[lev+1] = stack[lev]+1;
}
if (!yield_indexed_values(values, n, stack+1)) {
rb_raise(rb_eRuntimeError, "combination reentered");
}
do {
if (lev == 0) return;
stack[lev--]++;
} while (stack[lev+1]+n == len+lev+1);
}
}
static VALUE
rb_ary_combination_size(VALUE ary, VALUE args, VALUE eobj)
{
long n = RARRAY_LEN(ary);
long k = NUM2LONG(RARRAY_AREF(args, 0));
return binomial_coefficient(k, n);
}
static VALUE
rb_ary_combination(VALUE ary, VALUE num)
{
long i, n, len;
n = NUM2LONG(num);
RETURN_SIZED_ENUMERATOR(ary, 1, &num, rb_ary_combination_size);
len = RARRAY_LEN(ary);
if (n < 0 || len < n) {
}
else if (n == 0) {
rb_yield(rb_ary_new2(0));
}
else if (n == 1) {
for (i = 0; i < RARRAY_LEN(ary); i++) {
rb_yield(rb_ary_new3(1, RARRAY_AREF(ary, i)));
}
}
else {
VALUE ary0 = ary_make_shared_copy(ary); 
volatile VALUE t0;
long *stack = ALLOCV_N(long, t0, n+1);
RBASIC_CLEAR_CLASS(ary0);
combinate0(len, n, stack, ary0);
ALLOCV_END(t0);
RBASIC_SET_CLASS_RAW(ary0, rb_cArray);
}
return ary;
}
static void
rpermute0(const long n, const long r, long *const p, const VALUE values)
{
long i = 0, index = 0;
p[index] = i;
for (;;) {
if (++index < r-1) {
p[index] = i = 0;
continue;
}
for (i = 0; i < n; ++i) {
p[index] = i;
if (!yield_indexed_values(values, r, p)) {
rb_raise(rb_eRuntimeError, "repeated permute reentered");
}
}
do {
if (index <= 0) return;
} while ((i = ++p[--index]) >= n);
}
}
static VALUE
rb_ary_repeated_permutation_size(VALUE ary, VALUE args, VALUE eobj)
{
long n = RARRAY_LEN(ary);
long k = NUM2LONG(RARRAY_AREF(args, 0));
if (k < 0) {
return LONG2FIX(0);
}
if (n <= 0) {
return LONG2FIX(!k);
}
return rb_int_positive_pow(n, (unsigned long)k);
}
static VALUE
rb_ary_repeated_permutation(VALUE ary, VALUE num)
{
long r, n, i;
n = RARRAY_LEN(ary); 
RETURN_SIZED_ENUMERATOR(ary, 1, &num, rb_ary_repeated_permutation_size); 
r = NUM2LONG(num); 
if (r < 0) {
}
else if (r == 0) { 
rb_yield(rb_ary_new2(0));
}
else if (r == 1) { 
for (i = 0; i < RARRAY_LEN(ary); i++) {
rb_yield(rb_ary_new3(1, RARRAY_AREF(ary, i)));
}
}
else { 
volatile VALUE t0;
long *p = ALLOCV_N(long, t0, r);
VALUE ary0 = ary_make_shared_copy(ary); 
RBASIC_CLEAR_CLASS(ary0);
rpermute0(n, r, p, ary0); 
ALLOCV_END(t0);
RBASIC_SET_CLASS_RAW(ary0, rb_cArray);
}
return ary;
}
static void
rcombinate0(const long n, const long r, long *const p, const long rest, const VALUE values)
{
long i = 0, index = 0;
p[index] = i;
for (;;) {
if (++index < r-1) {
p[index] = i;
continue;
}
for (; i < n; ++i) {
p[index] = i;
if (!yield_indexed_values(values, r, p)) {
rb_raise(rb_eRuntimeError, "repeated combination reentered");
}
}
do {
if (index <= 0) return;
} while ((i = ++p[--index]) >= n);
}
}
static VALUE
rb_ary_repeated_combination_size(VALUE ary, VALUE args, VALUE eobj)
{
long n = RARRAY_LEN(ary);
long k = NUM2LONG(RARRAY_AREF(args, 0));
if (k == 0) {
return LONG2FIX(1);
}
return binomial_coefficient(k, n + k - 1);
}
static VALUE
rb_ary_repeated_combination(VALUE ary, VALUE num)
{
long n, i, len;
n = NUM2LONG(num); 
RETURN_SIZED_ENUMERATOR(ary, 1, &num, rb_ary_repeated_combination_size); 
len = RARRAY_LEN(ary);
if (n < 0) {
}
else if (n == 0) {
rb_yield(rb_ary_new2(0));
}
else if (n == 1) {
for (i = 0; i < RARRAY_LEN(ary); i++) {
rb_yield(rb_ary_new3(1, RARRAY_AREF(ary, i)));
}
}
else if (len == 0) {
}
else {
volatile VALUE t0;
long *p = ALLOCV_N(long, t0, n);
VALUE ary0 = ary_make_shared_copy(ary); 
RBASIC_CLEAR_CLASS(ary0);
rcombinate0(len, n, p, n, ary0); 
ALLOCV_END(t0);
RBASIC_SET_CLASS_RAW(ary0, rb_cArray);
}
return ary;
}
static VALUE
rb_ary_product(int argc, VALUE *argv, VALUE ary)
{
int n = argc+1; 
volatile VALUE t0 = tmpary(n);
volatile VALUE t1 = Qundef;
VALUE *arrays = RARRAY_PTR(t0); 
int *counters = ALLOCV_N(int, t1, n); 
VALUE result = Qnil; 
long i,j;
long resultlen = 1;
RBASIC_CLEAR_CLASS(t0);
ARY_SET_LEN(t0, n);
arrays[0] = ary;
for (i = 1; i < n; i++) arrays[i] = Qnil;
for (i = 1; i < n; i++) arrays[i] = to_ary(argv[i-1]);
for (i = 0; i < n; i++) counters[i] = 0;
if (rb_block_given_p()) {
for (i = 0; i < n; i++) {
if (RARRAY_LEN(arrays[i]) == 0) goto done;
arrays[i] = ary_make_shared_copy(arrays[i]);
}
}
else {
for (i = 0; i < n; i++) {
long k = RARRAY_LEN(arrays[i]);
if (k == 0) {
result = rb_ary_new2(0);
goto done;
}
if (MUL_OVERFLOW_LONG_P(resultlen, k))
rb_raise(rb_eRangeError, "too big to product");
resultlen *= k;
}
result = rb_ary_new2(resultlen);
}
for (;;) {
int m;
VALUE subarray = rb_ary_new2(n);
for (j = 0; j < n; j++) {
rb_ary_push(subarray, rb_ary_entry(arrays[j], counters[j]));
}
if (NIL_P(result)) {
FL_SET(t0, FL_USER5);
rb_yield(subarray);
if (! FL_TEST(t0, FL_USER5)) {
rb_raise(rb_eRuntimeError, "product reentered");
}
else {
FL_UNSET(t0, FL_USER5);
}
}
else {
rb_ary_push(result, subarray);
}
m = n-1;
counters[m]++;
while (counters[m] == RARRAY_LEN(arrays[m])) {
counters[m] = 0;
if (--m < 0) goto done;
counters[m]++;
}
}
done:
tmpary_discard(t0);
ALLOCV_END(t1);
return NIL_P(result) ? ary : result;
}
static VALUE
rb_ary_take(VALUE obj, VALUE n)
{
long len = NUM2LONG(n);
if (len < 0) {
rb_raise(rb_eArgError, "attempt to take negative size");
}
return rb_ary_subseq(obj, 0, len);
}
static VALUE
rb_ary_take_while(VALUE ary)
{
long i;
RETURN_ENUMERATOR(ary, 0, 0);
for (i = 0; i < RARRAY_LEN(ary); i++) {
if (!RTEST(rb_yield(RARRAY_AREF(ary, i)))) break;
}
return rb_ary_take(ary, LONG2FIX(i));
}
static VALUE
rb_ary_drop(VALUE ary, VALUE n)
{
VALUE result;
long pos = NUM2LONG(n);
if (pos < 0) {
rb_raise(rb_eArgError, "attempt to drop negative size");
}
result = rb_ary_subseq(ary, pos, RARRAY_LEN(ary));
if (result == Qnil) result = rb_ary_new();
return result;
}
static VALUE
rb_ary_drop_while(VALUE ary)
{
long i;
RETURN_ENUMERATOR(ary, 0, 0);
for (i = 0; i < RARRAY_LEN(ary); i++) {
if (!RTEST(rb_yield(RARRAY_AREF(ary, i)))) break;
}
return rb_ary_drop(ary, LONG2FIX(i));
}
static VALUE
rb_ary_any_p(int argc, VALUE *argv, VALUE ary)
{
long i, len = RARRAY_LEN(ary);
rb_check_arity(argc, 0, 1);
if (!len) return Qfalse;
if (argc) {
if (rb_block_given_p()) {
rb_warn("given block not used");
}
for (i = 0; i < RARRAY_LEN(ary); ++i) {
if (RTEST(rb_funcall(argv[0], idEqq, 1, RARRAY_AREF(ary, i)))) return Qtrue;
}
}
else if (!rb_block_given_p()) {
for (i = 0; i < len; ++i) {
if (RTEST(RARRAY_AREF(ary, i))) return Qtrue;
}
}
else {
for (i = 0; i < RARRAY_LEN(ary); ++i) {
if (RTEST(rb_yield(RARRAY_AREF(ary, i)))) return Qtrue;
}
}
return Qfalse;
}
static VALUE
rb_ary_all_p(int argc, VALUE *argv, VALUE ary)
{
long i, len = RARRAY_LEN(ary);
rb_check_arity(argc, 0, 1);
if (!len) return Qtrue;
if (argc) {
if (rb_block_given_p()) {
rb_warn("given block not used");
}
for (i = 0; i < RARRAY_LEN(ary); ++i) {
if (!RTEST(rb_funcall(argv[0], idEqq, 1, RARRAY_AREF(ary, i)))) return Qfalse;
}
}
else if (!rb_block_given_p()) {
for (i = 0; i < len; ++i) {
if (!RTEST(RARRAY_AREF(ary, i))) return Qfalse;
}
}
else {
for (i = 0; i < RARRAY_LEN(ary); ++i) {
if (!RTEST(rb_yield(RARRAY_AREF(ary, i)))) return Qfalse;
}
}
return Qtrue;
}
static VALUE
rb_ary_none_p(int argc, VALUE *argv, VALUE ary)
{
long i, len = RARRAY_LEN(ary);
rb_check_arity(argc, 0, 1);
if (!len) return Qtrue;
if (argc) {
if (rb_block_given_p()) {
rb_warn("given block not used");
}
for (i = 0; i < RARRAY_LEN(ary); ++i) {
if (RTEST(rb_funcall(argv[0], idEqq, 1, RARRAY_AREF(ary, i)))) return Qfalse;
}
}
else if (!rb_block_given_p()) {
for (i = 0; i < len; ++i) {
if (RTEST(RARRAY_AREF(ary, i))) return Qfalse;
}
}
else {
for (i = 0; i < RARRAY_LEN(ary); ++i) {
if (RTEST(rb_yield(RARRAY_AREF(ary, i)))) return Qfalse;
}
}
return Qtrue;
}
static VALUE
rb_ary_one_p(int argc, VALUE *argv, VALUE ary)
{
long i, len = RARRAY_LEN(ary);
VALUE result = Qfalse;
rb_check_arity(argc, 0, 1);
if (!len) return Qfalse;
if (argc) {
if (rb_block_given_p()) {
rb_warn("given block not used");
}
for (i = 0; i < RARRAY_LEN(ary); ++i) {
if (RTEST(rb_funcall(argv[0], idEqq, 1, RARRAY_AREF(ary, i)))) {
if (result) return Qfalse;
result = Qtrue;
}
}
}
else if (!rb_block_given_p()) {
for (i = 0; i < len; ++i) {
if (RTEST(RARRAY_AREF(ary, i))) {
if (result) return Qfalse;
result = Qtrue;
}
}
}
else {
for (i = 0; i < RARRAY_LEN(ary); ++i) {
if (RTEST(rb_yield(RARRAY_AREF(ary, i)))) {
if (result) return Qfalse;
result = Qtrue;
}
}
}
return result;
}
static VALUE
rb_ary_dig(int argc, VALUE *argv, VALUE self)
{
rb_check_arity(argc, 1, UNLIMITED_ARGUMENTS);
self = rb_ary_at(self, *argv);
if (!--argc) return self;
++argv;
return rb_obj_dig(argc, argv, self, Qnil);
}
static inline VALUE
finish_exact_sum(long n, VALUE r, VALUE v, int z)
{
if (n != 0)
v = rb_fix_plus(LONG2FIX(n), v);
if (r != Qundef) {
if (FIXNUM_P(r))
v = rb_fix_plus(r, v);
else if (RB_TYPE_P(r, T_BIGNUM))
v = rb_big_plus(r, v);
else
v = rb_rational_plus(r, v);
}
else if (!n && z) {
v = rb_fix_plus(LONG2FIX(0), v);
}
return v;
}
static VALUE
rb_ary_sum(int argc, VALUE *argv, VALUE ary)
{
VALUE e, v, r;
long i, n;
int block_given;
v = (rb_check_arity(argc, 0, 1) ? argv[0] : LONG2FIX(0));
block_given = rb_block_given_p();
if (RARRAY_LEN(ary) == 0)
return v;
n = 0;
r = Qundef;
for (i = 0; i < RARRAY_LEN(ary); i++) {
e = RARRAY_AREF(ary, i);
if (block_given)
e = rb_yield(e);
if (FIXNUM_P(e)) {
n += FIX2LONG(e); 
if (!FIXABLE(n)) {
v = rb_big_plus(LONG2NUM(n), v);
n = 0;
}
}
else if (RB_TYPE_P(e, T_BIGNUM))
v = rb_big_plus(e, v);
else if (RB_TYPE_P(e, T_RATIONAL)) {
if (r == Qundef)
r = e;
else
r = rb_rational_plus(r, e);
}
else
goto not_exact;
}
v = finish_exact_sum(n, r, v, argc!=0);
return v;
not_exact:
v = finish_exact_sum(n, r, v, i!=0);
if (RB_FLOAT_TYPE_P(e)) {
double f, c;
double x, t;
f = NUM2DBL(v);
c = 0.0;
goto has_float_value;
for (; i < RARRAY_LEN(ary); i++) {
e = RARRAY_AREF(ary, i);
if (block_given)
e = rb_yield(e);
if (RB_FLOAT_TYPE_P(e))
has_float_value:
x = RFLOAT_VALUE(e);
else if (FIXNUM_P(e))
x = FIX2LONG(e);
else if (RB_TYPE_P(e, T_BIGNUM))
x = rb_big2dbl(e);
else if (RB_TYPE_P(e, T_RATIONAL))
x = rb_num2dbl(e);
else
goto not_float;
if (isnan(f)) continue;
if (isnan(x)) {
f = x;
continue;
}
if (isinf(x)) {
if (isinf(f) && signbit(x) != signbit(f))
f = NAN;
else
f = x;
continue;
}
if (isinf(f)) continue;
t = f + x;
if (fabs(f) >= fabs(x))
c += ((f - t) + x);
else
c += ((x - t) + f);
f = t;
}
f += c;
return DBL2NUM(f);
not_float:
v = DBL2NUM(f);
}
goto has_some_value;
for (; i < RARRAY_LEN(ary); i++) {
e = RARRAY_AREF(ary, i);
if (block_given)
e = rb_yield(e);
has_some_value:
v = rb_funcall(v, idPLUS, 1, e);
}
return v;
}
static VALUE
rb_ary_deconstruct(VALUE ary)
{
return ary;
}
void
Init_Array(void)
{
#undef rb_intern
#define rb_intern(str) rb_intern_const(str)
rb_cArray = rb_define_class("Array", rb_cObject);
rb_include_module(rb_cArray, rb_mEnumerable);
rb_define_alloc_func(rb_cArray, empty_ary_alloc);
rb_define_singleton_method(rb_cArray, "[]", rb_ary_s_create, -1);
rb_define_singleton_method(rb_cArray, "try_convert", rb_ary_s_try_convert, 1);
rb_define_method(rb_cArray, "initialize", rb_ary_initialize, -1);
rb_define_method(rb_cArray, "initialize_copy", rb_ary_replace, 1);
rb_define_method(rb_cArray, "inspect", rb_ary_inspect, 0);
rb_define_alias(rb_cArray, "to_s", "inspect");
rb_define_method(rb_cArray, "to_a", rb_ary_to_a, 0);
rb_define_method(rb_cArray, "to_h", rb_ary_to_h, 0);
rb_define_method(rb_cArray, "to_ary", rb_ary_to_ary_m, 0);
rb_define_method(rb_cArray, "==", rb_ary_equal, 1);
rb_define_method(rb_cArray, "eql?", rb_ary_eql, 1);
rb_define_method(rb_cArray, "hash", rb_ary_hash, 0);
rb_define_method(rb_cArray, "[]", rb_ary_aref, -1);
rb_define_method(rb_cArray, "[]=", rb_ary_aset, -1);
rb_define_method(rb_cArray, "at", rb_ary_at, 1);
rb_define_method(rb_cArray, "fetch", rb_ary_fetch, -1);
rb_define_method(rb_cArray, "first", rb_ary_first, -1);
rb_define_method(rb_cArray, "last", rb_ary_last, -1);
rb_define_method(rb_cArray, "concat", rb_ary_concat_multi, -1);
rb_define_method(rb_cArray, "union", rb_ary_union_multi, -1);
rb_define_method(rb_cArray, "difference", rb_ary_difference_multi, -1);
rb_define_method(rb_cArray, "intersection", rb_ary_intersection_multi, -1);
rb_define_method(rb_cArray, "<<", rb_ary_push, 1);
rb_define_method(rb_cArray, "push", rb_ary_push_m, -1);
rb_define_alias(rb_cArray, "append", "push");
rb_define_method(rb_cArray, "pop", rb_ary_pop_m, -1);
rb_define_method(rb_cArray, "shift", rb_ary_shift_m, -1);
rb_define_method(rb_cArray, "unshift", rb_ary_unshift_m, -1);
rb_define_alias(rb_cArray, "prepend", "unshift");
rb_define_method(rb_cArray, "insert", rb_ary_insert, -1);
rb_define_method(rb_cArray, "each", rb_ary_each, 0);
rb_define_method(rb_cArray, "each_index", rb_ary_each_index, 0);
rb_define_method(rb_cArray, "reverse_each", rb_ary_reverse_each, 0);
rb_define_method(rb_cArray, "length", rb_ary_length, 0);
rb_define_alias(rb_cArray, "size", "length");
rb_define_method(rb_cArray, "empty?", rb_ary_empty_p, 0);
rb_define_method(rb_cArray, "find_index", rb_ary_index, -1);
rb_define_method(rb_cArray, "index", rb_ary_index, -1);
rb_define_method(rb_cArray, "rindex", rb_ary_rindex, -1);
rb_define_method(rb_cArray, "join", rb_ary_join_m, -1);
rb_define_method(rb_cArray, "reverse", rb_ary_reverse_m, 0);
rb_define_method(rb_cArray, "reverse!", rb_ary_reverse_bang, 0);
rb_define_method(rb_cArray, "rotate", rb_ary_rotate_m, -1);
rb_define_method(rb_cArray, "rotate!", rb_ary_rotate_bang, -1);
rb_define_method(rb_cArray, "sort", rb_ary_sort, 0);
rb_define_method(rb_cArray, "sort!", rb_ary_sort_bang, 0);
rb_define_method(rb_cArray, "sort_by!", rb_ary_sort_by_bang, 0);
rb_define_method(rb_cArray, "collect", rb_ary_collect, 0);
rb_define_method(rb_cArray, "collect!", rb_ary_collect_bang, 0);
rb_define_method(rb_cArray, "map", rb_ary_collect, 0);
rb_define_method(rb_cArray, "map!", rb_ary_collect_bang, 0);
rb_define_method(rb_cArray, "select", rb_ary_select, 0);
rb_define_method(rb_cArray, "select!", rb_ary_select_bang, 0);
rb_define_method(rb_cArray, "filter", rb_ary_select, 0);
rb_define_method(rb_cArray, "filter!", rb_ary_select_bang, 0);
rb_define_method(rb_cArray, "keep_if", rb_ary_keep_if, 0);
rb_define_method(rb_cArray, "values_at", rb_ary_values_at, -1);
rb_define_method(rb_cArray, "delete", rb_ary_delete, 1);
rb_define_method(rb_cArray, "delete_at", rb_ary_delete_at_m, 1);
rb_define_method(rb_cArray, "delete_if", rb_ary_delete_if, 0);
rb_define_method(rb_cArray, "reject", rb_ary_reject, 0);
rb_define_method(rb_cArray, "reject!", rb_ary_reject_bang, 0);
rb_define_method(rb_cArray, "zip", rb_ary_zip, -1);
rb_define_method(rb_cArray, "transpose", rb_ary_transpose, 0);
rb_define_method(rb_cArray, "replace", rb_ary_replace, 1);
rb_define_method(rb_cArray, "clear", rb_ary_clear, 0);
rb_define_method(rb_cArray, "fill", rb_ary_fill, -1);
rb_define_method(rb_cArray, "include?", rb_ary_includes, 1);
rb_define_method(rb_cArray, "<=>", rb_ary_cmp, 1);
rb_define_method(rb_cArray, "slice", rb_ary_aref, -1);
rb_define_method(rb_cArray, "slice!", rb_ary_slice_bang, -1);
rb_define_method(rb_cArray, "assoc", rb_ary_assoc, 1);
rb_define_method(rb_cArray, "rassoc", rb_ary_rassoc, 1);
rb_define_method(rb_cArray, "+", rb_ary_plus, 1);
rb_define_method(rb_cArray, "*", rb_ary_times, 1);
rb_define_method(rb_cArray, "-", rb_ary_diff, 1);
rb_define_method(rb_cArray, "&", rb_ary_and, 1);
rb_define_method(rb_cArray, "|", rb_ary_or, 1);
rb_define_method(rb_cArray, "max", rb_ary_max, -1);
rb_define_method(rb_cArray, "min", rb_ary_min, -1);
rb_define_method(rb_cArray, "minmax", rb_ary_minmax, 0);
rb_define_method(rb_cArray, "uniq", rb_ary_uniq, 0);
rb_define_method(rb_cArray, "uniq!", rb_ary_uniq_bang, 0);
rb_define_method(rb_cArray, "compact", rb_ary_compact, 0);
rb_define_method(rb_cArray, "compact!", rb_ary_compact_bang, 0);
rb_define_method(rb_cArray, "flatten", rb_ary_flatten, -1);
rb_define_method(rb_cArray, "flatten!", rb_ary_flatten_bang, -1);
rb_define_method(rb_cArray, "count", rb_ary_count, -1);
rb_define_method(rb_cArray, "cycle", rb_ary_cycle, -1);
rb_define_method(rb_cArray, "permutation", rb_ary_permutation, -1);
rb_define_method(rb_cArray, "combination", rb_ary_combination, 1);
rb_define_method(rb_cArray, "repeated_permutation", rb_ary_repeated_permutation, 1);
rb_define_method(rb_cArray, "repeated_combination", rb_ary_repeated_combination, 1);
rb_define_method(rb_cArray, "product", rb_ary_product, -1);
rb_define_method(rb_cArray, "take", rb_ary_take, 1);
rb_define_method(rb_cArray, "take_while", rb_ary_take_while, 0);
rb_define_method(rb_cArray, "drop", rb_ary_drop, 1);
rb_define_method(rb_cArray, "drop_while", rb_ary_drop_while, 0);
rb_define_method(rb_cArray, "bsearch", rb_ary_bsearch, 0);
rb_define_method(rb_cArray, "bsearch_index", rb_ary_bsearch_index, 0);
rb_define_method(rb_cArray, "any?", rb_ary_any_p, -1);
rb_define_method(rb_cArray, "all?", rb_ary_all_p, -1);
rb_define_method(rb_cArray, "none?", rb_ary_none_p, -1);
rb_define_method(rb_cArray, "one?", rb_ary_one_p, -1);
rb_define_method(rb_cArray, "dig", rb_ary_dig, -1);
rb_define_method(rb_cArray, "sum", rb_ary_sum, -1);
rb_define_method(rb_cArray, "deconstruct", rb_ary_deconstruct, 0);
}
#include "array.rbinc"
