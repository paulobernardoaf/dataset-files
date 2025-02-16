#include "ruby/config.h"
#include <stddef.h> 
#include "internal/array.h" 
#include "internal/gc.h" 
#include "internal/stdbool.h" 
#include "ruby/ruby.h" 
#if !defined(IMEMO_DEBUG)
#define IMEMO_DEBUG 0
#endif
#define IMEMO_MASK 0x0f
#define IMEMO_FL_USHIFT (FL_USHIFT + 4)
#define IMEMO_FL_USER0 FL_USER4
#define IMEMO_FL_USER1 FL_USER5
#define IMEMO_FL_USER2 FL_USER6
#define IMEMO_FL_USER3 FL_USER7
#define IMEMO_FL_USER4 FL_USER8
#define IMEMO_FL_USER5 FL_USER9
enum imemo_type {
imemo_env = 0,
imemo_cref = 1, 
imemo_svar = 2, 
imemo_throw_data = 3,
imemo_ifunc = 4, 
imemo_memo = 5,
imemo_ment = 6,
imemo_iseq = 7,
imemo_tmpbuf = 8,
imemo_ast = 9,
imemo_parser_strterm = 10,
imemo_callinfo = 11,
imemo_callcache = 12,
};
struct vm_svar {
VALUE flags;
const VALUE cref_or_me; 
const VALUE lastline;
const VALUE backref;
const VALUE others;
};
struct vm_throw_data {
VALUE flags;
VALUE reserved;
const VALUE throw_obj;
const struct rb_control_frame_struct *catch_frame;
int throw_state;
};
#define THROW_DATA_CONSUMED IMEMO_FL_USER0
struct vm_ifunc_argc {
#if SIZEOF_INT * 2 > SIZEOF_VALUE
signed int min: (SIZEOF_VALUE * CHAR_BIT) / 2;
signed int max: (SIZEOF_VALUE * CHAR_BIT) / 2;
#else
int min, max;
#endif
};
struct vm_ifunc {
VALUE flags;
VALUE reserved;
rb_block_call_func_t func;
const void *data;
struct vm_ifunc_argc argc;
};
struct rb_imemo_tmpbuf_struct {
VALUE flags;
VALUE reserved;
VALUE *ptr; 
struct rb_imemo_tmpbuf_struct *next; 
size_t cnt; 
};
struct MEMO {
VALUE flags;
VALUE reserved;
const VALUE v1;
const VALUE v2;
union {
long cnt;
long state;
const VALUE value;
void (*func)(void);
} u3;
};
#define THROW_DATA_P(err) imemo_throw_data_p((VALUE)err)
#define MEMO_CAST(m) (R_CAST(MEMO)(m))
#define MEMO_NEW(a, b, c) ((struct MEMO *)rb_imemo_new(imemo_memo, (VALUE)(a), (VALUE)(b), (VALUE)(c), 0))
#define MEMO_FOR(type, value) ((type *)RARRAY_PTR(value))
#define NEW_MEMO_FOR(type, value) ((value) = rb_ary_tmp_new_fill(type_roomof(type, VALUE)), MEMO_FOR(type, value))
#define NEW_PARTIAL_MEMO_FOR(type, value, member) ((value) = rb_ary_tmp_new_fill(type_roomof(type, VALUE)), rb_ary_set_len((value), offsetof(type, member) / sizeof(VALUE)), MEMO_FOR(type, value))
typedef struct rb_imemo_tmpbuf_struct rb_imemo_tmpbuf_t;
VALUE rb_imemo_new(enum imemo_type type, VALUE v1, VALUE v2, VALUE v3, VALUE v0);
rb_imemo_tmpbuf_t *rb_imemo_tmpbuf_parser_heap(void *buf, rb_imemo_tmpbuf_t *old_heap, size_t cnt);
struct vm_ifunc *rb_vm_ifunc_new(rb_block_call_func_t func, const void *data, int min_argc, int max_argc);
void rb_strterm_mark(VALUE obj);
static inline enum imemo_type imemo_type(VALUE imemo);
static inline int imemo_type_p(VALUE imemo, enum imemo_type imemo_type);
static inline bool imemo_throw_data_p(VALUE imemo);
static inline struct vm_ifunc *rb_vm_ifunc_proc_new(rb_block_call_func_t func, const void *data);
static inline VALUE rb_imemo_tmpbuf_auto_free_pointer(void);
static inline void *RB_IMEMO_TMPBUF_PTR(VALUE v);
static inline void *rb_imemo_tmpbuf_set_ptr(VALUE v, void *ptr);
static inline VALUE rb_imemo_tmpbuf_auto_free_pointer_new_from_an_RString(VALUE str);
static inline void MEMO_V1_SET(struct MEMO *m, VALUE v);
static inline void MEMO_V2_SET(struct MEMO *m, VALUE v);
RUBY_SYMBOL_EXPORT_BEGIN
#if IMEMO_DEBUG
VALUE rb_imemo_new_debug(enum imemo_type type, VALUE v1, VALUE v2, VALUE v3, VALUE v0, const char *file, int line);
#define rb_imemo_new(type, v1, v2, v3, v0) rb_imemo_new_debug(type, v1, v2, v3, v0, __FILE__, __LINE__)
#else
VALUE rb_imemo_new(enum imemo_type type, VALUE v1, VALUE v2, VALUE v3, VALUE v0);
#endif
RUBY_SYMBOL_EXPORT_END
static inline enum imemo_type
imemo_type(VALUE imemo)
{
return (RBASIC(imemo)->flags >> FL_USHIFT) & IMEMO_MASK;
}
static inline int
imemo_type_p(VALUE imemo, enum imemo_type imemo_type)
{
if (LIKELY(!RB_SPECIAL_CONST_P(imemo))) {
const VALUE mask = (IMEMO_MASK << FL_USHIFT) | RUBY_T_MASK;
const VALUE expected_type = (imemo_type << FL_USHIFT) | T_IMEMO;
return expected_type == (RBASIC(imemo)->flags & mask);
}
else {
return 0;
}
}
#define IMEMO_TYPE_P(v, t) imemo_type_p((VALUE)v, t)
static inline bool
imemo_throw_data_p(VALUE imemo)
{
return RB_TYPE_P(imemo, T_IMEMO);
}
static inline struct vm_ifunc *
rb_vm_ifunc_proc_new(rb_block_call_func_t func, const void *data)
{
return rb_vm_ifunc_new(func, data, 0, UNLIMITED_ARGUMENTS);
}
static inline VALUE
rb_imemo_tmpbuf_auto_free_pointer(void)
{
return rb_imemo_new(imemo_tmpbuf, 0, 0, 0, 0);
}
static inline void *
RB_IMEMO_TMPBUF_PTR(VALUE v)
{
const struct rb_imemo_tmpbuf_struct *p = (const void *)v;
return p->ptr;
}
static inline void *
rb_imemo_tmpbuf_set_ptr(VALUE v, void *ptr)
{
return ((rb_imemo_tmpbuf_t *)v)->ptr = ptr;
}
static inline VALUE
rb_imemo_tmpbuf_auto_free_pointer_new_from_an_RString(VALUE str)
{
const void *src;
VALUE imemo;
rb_imemo_tmpbuf_t *tmpbuf;
void *dst;
size_t len;
SafeStringValue(str);
imemo = rb_imemo_tmpbuf_auto_free_pointer();
tmpbuf = (rb_imemo_tmpbuf_t *)imemo;
len = RSTRING_LEN(str);
src = RSTRING_PTR(str);
dst = ruby_xmalloc(len);
memcpy(dst, src, len);
tmpbuf->ptr = dst;
return imemo;
}
static inline void
MEMO_V1_SET(struct MEMO *m, VALUE v)
{
RB_OBJ_WRITE(m, &m->v1, v);
}
static inline void
MEMO_V2_SET(struct MEMO *m, VALUE v)
{
RB_OBJ_WRITE(m, &m->v2, v);
}
