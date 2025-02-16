#if !defined(RUBY_RUBY_BACKWARD_H)
#define RUBY_RUBY_BACKWARD_H 1

#define RClass RClassDeprecated
#if !defined(__cplusplus)
DEPRECATED_TYPE(("RClass is internal use only"),
struct RClass {
struct RBasic basic;
});
#endif

#define DECLARE_DEPRECATED_FEATURE(ver, func) NORETURN(ERRORFUNC(("deprecated since "#ver), DEPRECATED(void func(void))))



DECLARE_DEPRECATED_FEATURE(2.2, rb_disable_super);
DECLARE_DEPRECATED_FEATURE(2.2, rb_enable_super);


DECLARE_DEPRECATED_FEATURE(2.2, rb_hash_iter_lev);
DECLARE_DEPRECATED_FEATURE(2.2, rb_hash_ifnone);


DECLARE_DEPRECATED_FEATURE(2.2, rb_str_associate);
DECLARE_DEPRECATED_FEATURE(2.2, rb_str_associated);


DEPRECATED(void rb_autoload(VALUE, ID, const char*));


DECLARE_DEPRECATED_FEATURE(2.2, rb_clear_cache);
DECLARE_DEPRECATED_FEATURE(2.2, rb_frame_pop);

#define DECLARE_DEPRECATED_INTERNAL_FEATURE(func) NORETURN(ERRORFUNC(("deprecated internal function"), DEPRECATED(void func(void))))



NORETURN(ERRORFUNC(("internal function"), void rb_frozen_class_p(VALUE)));
DECLARE_DEPRECATED_INTERNAL_FEATURE(rb_exec_end_proc);


DECLARE_DEPRECATED_INTERNAL_FEATURE(rb_compile_error);
DECLARE_DEPRECATED_INTERNAL_FEATURE(rb_compile_error_with_enc);
DECLARE_DEPRECATED_INTERNAL_FEATURE(rb_compile_error_append);


DECLARE_DEPRECATED_INTERNAL_FEATURE(rb_gc_call_finalizer_at_exit);


DECLARE_DEPRECATED_INTERNAL_FEATURE(rb_trap_exit);


DECLARE_DEPRECATED_INTERNAL_FEATURE(rb_struct_ptr);


DECLARE_DEPRECATED_INTERNAL_FEATURE(rb_clear_trace_func);


DECLARE_DEPRECATED_INTERNAL_FEATURE(rb_generic_ivar_table);
NORETURN(ERRORFUNC(("internal function"), VALUE rb_mod_const_missing(VALUE, VALUE)));


#if !defined(RUBY_SHOW_COPYRIGHT_TO_DIE)
#define RUBY_SHOW_COPYRIGHT_TO_DIE 1
#endif
#if RUBY_SHOW_COPYRIGHT_TO_DIE

DEPRECATED(static inline int ruby_show_copyright_to_die(int));
static inline int
ruby_show_copyright_to_die(int exitcode)
{
ruby_show_copyright();
return exitcode;
}
#define ruby_show_copyright() (exit(ruby_show_copyright_to_die(EXIT_SUCCESS)))

#endif

#endif 
