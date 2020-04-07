#include "ruby/ruby.h" 
#include "ruby/encoding.h" 
#include "internal/compilers.h" 
#if defined(rb_sym_intern_ascii_cstr)
#undef rb_sym_intern_ascii_cstr
#endif
VALUE rb_to_symbol_type(VALUE obj);
VALUE rb_sym_intern(const char *ptr, long len, rb_encoding *enc);
VALUE rb_sym_intern_ascii(const char *ptr, long len);
VALUE rb_sym_intern_ascii_cstr(const char *ptr);
int rb_is_const_name(VALUE name);
int rb_is_class_name(VALUE name);
int rb_is_instance_name(VALUE name);
int rb_is_local_name(VALUE name);
PUREFUNC(int rb_is_const_sym(VALUE sym));
PUREFUNC(int rb_is_attrset_sym(VALUE sym));
ID rb_make_internal_id(void);
void rb_gc_free_dsymbol(VALUE);
#if __has_builtin(__builtin_constant_p)
#define rb_sym_intern_ascii_cstr(ptr) (__builtin_constant_p(ptr) ? rb_sym_intern_ascii((ptr), (long)strlen(ptr)) : rb_sym_intern_ascii_cstr(ptr))
#endif
