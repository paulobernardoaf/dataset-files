#include "ruby/ruby.h" 
#include "ruby/io.h" 
void ruby_set_inplace_mode(const char *);
void rb_stdio_set_default_encoding(void);
VALUE rb_io_flush_raw(VALUE, int);
size_t rb_io_memsize(const rb_io_t *);
int rb_stderr_tty_p(void);
void rb_io_fptr_finalize_internal(void *ptr);
#if defined(rb_io_fptr_finalize)
#undef rb_io_fptr_finalize
#endif
#define rb_io_fptr_finalize rb_io_fptr_finalize_internal
RUBY_SYMBOL_EXPORT_BEGIN
void rb_maygvl_fd_fix_cloexec(int fd);
int rb_gc_for_fd(int err);
void rb_write_error_str(VALUE mesg);
RUBY_SYMBOL_EXPORT_END
