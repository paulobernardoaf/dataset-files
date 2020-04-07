#include <fiddle.h>
VALUE mFiddle;
VALUE rb_eFiddleError;
#if !defined(TYPE_SSIZE_T)
#if SIZEOF_SIZE_T == SIZEOF_INT
#define TYPE_SSIZE_T TYPE_INT
#elif SIZEOF_SIZE_T == SIZEOF_LONG
#define TYPE_SSIZE_T TYPE_LONG
#elif defined HAVE_LONG_LONG && SIZEOF_SIZE_T == SIZEOF_LONG_LONG
#define TYPE_SSIZE_T TYPE_LONG_LONG
#endif
#endif
#define TYPE_SIZE_T (-1*SIGNEDNESS_OF_SIZE_T*TYPE_SSIZE_T)
#if !defined(TYPE_PTRDIFF_T)
#if SIZEOF_PTRDIFF_T == SIZEOF_INT
#define TYPE_PTRDIFF_T TYPE_INT
#elif SIZEOF_PTRDIFF_T == SIZEOF_LONG
#define TYPE_PTRDIFF_T TYPE_LONG
#elif defined HAVE_LONG_LONG && SIZEOF_PTRDIFF_T == SIZEOF_LONG_LONG
#define TYPE_PTRDIFF_T TYPE_LONG_LONG
#endif
#endif
#if !defined(TYPE_INTPTR_T)
#if SIZEOF_INTPTR_T == SIZEOF_INT
#define TYPE_INTPTR_T TYPE_INT
#elif SIZEOF_INTPTR_T == SIZEOF_LONG
#define TYPE_INTPTR_T TYPE_LONG
#elif defined HAVE_LONG_LONG && SIZEOF_INTPTR_T == SIZEOF_LONG_LONG
#define TYPE_INTPTR_T TYPE_LONG_LONG
#endif
#endif
#define TYPE_UINTPTR_T (-TYPE_INTPTR_T)
void Init_fiddle_pointer(void);
static VALUE
rb_fiddle_malloc(VALUE self, VALUE size)
{
void *ptr;
ptr = (void*)ruby_xmalloc(NUM2SIZET(size));
return PTR2NUM(ptr);
}
static VALUE
rb_fiddle_realloc(VALUE self, VALUE addr, VALUE size)
{
void *ptr = NUM2PTR(addr);
ptr = (void*)ruby_xrealloc(ptr, NUM2SIZET(size));
return PTR2NUM(ptr);
}
VALUE
rb_fiddle_free(VALUE self, VALUE addr)
{
void *ptr = NUM2PTR(addr);
ruby_xfree(ptr);
return Qnil;
}
VALUE
rb_fiddle_ptr2value(VALUE self, VALUE addr)
{
return (VALUE)NUM2PTR(addr);
}
static VALUE
rb_fiddle_value2ptr(VALUE self, VALUE val)
{
return PTR2NUM((void*)val);
}
void Init_fiddle_handle(void);
void
Init_fiddle(void)
{
mFiddle = rb_define_module("Fiddle");
rb_eFiddleError = rb_define_class_under(mFiddle, "DLError", rb_eStandardError);
rb_define_const(mFiddle, "TYPE_VOID", INT2NUM(TYPE_VOID));
rb_define_const(mFiddle, "TYPE_VOIDP", INT2NUM(TYPE_VOIDP));
rb_define_const(mFiddle, "TYPE_CHAR", INT2NUM(TYPE_CHAR));
rb_define_const(mFiddle, "TYPE_SHORT", INT2NUM(TYPE_SHORT));
rb_define_const(mFiddle, "TYPE_INT", INT2NUM(TYPE_INT));
rb_define_const(mFiddle, "TYPE_LONG", INT2NUM(TYPE_LONG));
#if HAVE_LONG_LONG
rb_define_const(mFiddle, "TYPE_LONG_LONG", INT2NUM(TYPE_LONG_LONG));
#endif
rb_define_const(mFiddle, "TYPE_FLOAT", INT2NUM(TYPE_FLOAT));
rb_define_const(mFiddle, "TYPE_DOUBLE", INT2NUM(TYPE_DOUBLE));
rb_define_const(mFiddle, "TYPE_SIZE_T", INT2NUM(TYPE_SIZE_T));
rb_define_const(mFiddle, "TYPE_SSIZE_T", INT2NUM(TYPE_SSIZE_T));
rb_define_const(mFiddle, "TYPE_PTRDIFF_T", INT2NUM(TYPE_PTRDIFF_T));
rb_define_const(mFiddle, "TYPE_INTPTR_T", INT2NUM(TYPE_INTPTR_T));
rb_define_const(mFiddle, "TYPE_UINTPTR_T", INT2NUM(TYPE_UINTPTR_T));
rb_define_const(mFiddle, "ALIGN_VOIDP", INT2NUM(ALIGN_VOIDP));
rb_define_const(mFiddle, "ALIGN_CHAR", INT2NUM(ALIGN_CHAR));
rb_define_const(mFiddle, "ALIGN_SHORT", INT2NUM(ALIGN_SHORT));
rb_define_const(mFiddle, "ALIGN_INT", INT2NUM(ALIGN_INT));
rb_define_const(mFiddle, "ALIGN_LONG", INT2NUM(ALIGN_LONG));
#if HAVE_LONG_LONG
rb_define_const(mFiddle, "ALIGN_LONG_LONG", INT2NUM(ALIGN_LONG_LONG));
#endif
rb_define_const(mFiddle, "ALIGN_FLOAT", INT2NUM(ALIGN_FLOAT));
rb_define_const(mFiddle, "ALIGN_DOUBLE",INT2NUM(ALIGN_DOUBLE));
rb_define_const(mFiddle, "ALIGN_SIZE_T", INT2NUM(ALIGN_OF(size_t)));
rb_define_const(mFiddle, "ALIGN_SSIZE_T", INT2NUM(ALIGN_OF(size_t))); 
rb_define_const(mFiddle, "ALIGN_PTRDIFF_T", INT2NUM(ALIGN_OF(ptrdiff_t)));
rb_define_const(mFiddle, "ALIGN_INTPTR_T", INT2NUM(ALIGN_OF(intptr_t)));
rb_define_const(mFiddle, "ALIGN_UINTPTR_T", INT2NUM(ALIGN_OF(uintptr_t)));
#if defined(_WIN32)
rb_define_const(mFiddle, "WINDOWS", Qtrue);
#else
rb_define_const(mFiddle, "WINDOWS", Qfalse);
#endif
rb_define_const(mFiddle, "SIZEOF_VOIDP", INT2NUM(sizeof(void*)));
rb_define_const(mFiddle, "SIZEOF_CHAR", INT2NUM(sizeof(char)));
rb_define_const(mFiddle, "SIZEOF_SHORT", INT2NUM(sizeof(short)));
rb_define_const(mFiddle, "SIZEOF_INT", INT2NUM(sizeof(int)));
rb_define_const(mFiddle, "SIZEOF_LONG", INT2NUM(sizeof(long)));
#if HAVE_LONG_LONG
rb_define_const(mFiddle, "SIZEOF_LONG_LONG", INT2NUM(sizeof(LONG_LONG)));
#endif
rb_define_const(mFiddle, "SIZEOF_FLOAT", INT2NUM(sizeof(float)));
rb_define_const(mFiddle, "SIZEOF_DOUBLE",INT2NUM(sizeof(double)));
rb_define_const(mFiddle, "SIZEOF_SIZE_T", INT2NUM(sizeof(size_t)));
rb_define_const(mFiddle, "SIZEOF_SSIZE_T", INT2NUM(sizeof(size_t))); 
rb_define_const(mFiddle, "SIZEOF_PTRDIFF_T", INT2NUM(sizeof(ptrdiff_t)));
rb_define_const(mFiddle, "SIZEOF_INTPTR_T", INT2NUM(sizeof(intptr_t)));
rb_define_const(mFiddle, "SIZEOF_UINTPTR_T", INT2NUM(sizeof(uintptr_t)));
rb_define_const(mFiddle, "RUBY_FREE", PTR2NUM(ruby_xfree));
rb_define_const(mFiddle, "BUILD_RUBY_PLATFORM", rb_str_new2(RUBY_PLATFORM));
rb_define_module_function(mFiddle, "dlwrap", rb_fiddle_value2ptr, 1);
rb_define_module_function(mFiddle, "dlunwrap", rb_fiddle_ptr2value, 1);
rb_define_module_function(mFiddle, "malloc", rb_fiddle_malloc, 1);
rb_define_module_function(mFiddle, "realloc", rb_fiddle_realloc, 2);
rb_define_module_function(mFiddle, "free", rb_fiddle_free, 1);
Init_fiddle_function();
Init_fiddle_closure();
Init_fiddle_handle();
Init_fiddle_pointer();
}
