#include "ruby/config.h"
#include <stddef.h> 
#include "internal/static_assert.h" 
#include "internal/stdbool.h" 
#include "ruby/ruby.h" 
#if !defined(ARRAY_DEBUG)
#define ARRAY_DEBUG (0+RUBY_DEBUG)
#endif
#define RARRAY_PTR_IN_USE_FLAG FL_USER14
VALUE rb_ary_last(int, const VALUE *, VALUE);
void rb_ary_set_len(VALUE, long);
void rb_ary_delete_same(VALUE, VALUE);
VALUE rb_ary_tmp_new_fill(long capa);
VALUE rb_ary_at(VALUE, VALUE);
size_t rb_ary_memsize(VALUE);
VALUE rb_to_array_type(VALUE obj);
static inline VALUE rb_ary_entry_internal(VALUE ary, long offset);
static inline bool ARY_PTR_USING_P(VALUE ary);
static inline void RARY_TRANSIENT_SET(VALUE ary);
static inline void RARY_TRANSIENT_UNSET(VALUE ary);
RUBY_SYMBOL_EXPORT_BEGIN
void rb_ary_detransient(VALUE a);
VALUE *rb_ary_ptr_use_start(VALUE ary);
void rb_ary_ptr_use_end(VALUE ary);
RUBY_SYMBOL_EXPORT_END
MJIT_SYMBOL_EXPORT_BEGIN
VALUE rb_ary_tmp_new_from_values(VALUE, long, const VALUE *);
VALUE rb_check_to_array(VALUE ary);
VALUE rb_ary_behead(VALUE, long);
VALUE rb_ary_aref1(VALUE ary, VALUE i);
MJIT_SYMBOL_EXPORT_END
static inline VALUE
rb_ary_entry_internal(VALUE ary, long offset)
{
long len = RARRAY_LEN(ary);
const VALUE *ptr = RARRAY_CONST_PTR_TRANSIENT(ary);
if (len == 0) return Qnil;
if (offset < 0) {
offset += len;
if (offset < 0) return Qnil;
}
else if (len <= offset) {
return Qnil;
}
return ptr[offset];
}
static inline bool
ARY_PTR_USING_P(VALUE ary)
{
return FL_TEST_RAW(ary, RARRAY_PTR_IN_USE_FLAG);
}
static inline void
RARY_TRANSIENT_SET(VALUE ary)
{
#if USE_TRANSIENT_HEAP
FL_SET_RAW(ary, RARRAY_TRANSIENT_FLAG);
#endif
}
static inline void
RARY_TRANSIENT_UNSET(VALUE ary)
{
#if USE_TRANSIENT_HEAP
FL_UNSET_RAW(ary, RARRAY_TRANSIENT_FLAG);
#endif
}
#undef rb_ary_new_from_args
#if defined(__GNUC__) && defined(HAVE_VA_ARGS_MACRO)
#define rb_ary_new_from_args(n, ...) __extension__ ({ const VALUE args_to_new_ary[] = {__VA_ARGS__}; if (__builtin_constant_p(n)) { STATIC_ASSERT(rb_ary_new_from_args, numberof(args_to_new_ary) == (n)); } rb_ary_new_from_values(numberof(args_to_new_ary), args_to_new_ary); })
#endif
