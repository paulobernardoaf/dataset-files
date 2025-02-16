#if !defined(INTERNAL_FILE_H)
#define INTERNAL_FILE_H









#include "ruby/ruby.h" 
#include "ruby/encoding.h" 


extern const char ruby_null_device[];
VALUE rb_home_dir_of(VALUE user, VALUE result);
VALUE rb_default_home_dir(VALUE result);
VALUE rb_realpath_internal(VALUE basedir, VALUE path, int strict);
VALUE rb_check_realpath(VALUE basedir, VALUE path, rb_encoding *origenc);
void rb_file_const(const char*, VALUE);
int rb_file_load_ok(const char *);
VALUE rb_file_expand_path_fast(VALUE, VALUE);
VALUE rb_file_expand_path_internal(VALUE, VALUE, int, int, VALUE);
VALUE rb_get_path_check_to_string(VALUE);
VALUE rb_get_path_check_convert(VALUE);
int ruby_is_fd_loadable(int fd);

RUBY_SYMBOL_EXPORT_BEGIN

#if defined(HAVE_READLINK)
VALUE rb_readlink(VALUE path, rb_encoding *enc);
#endif
#if defined(__APPLE__)
VALUE rb_str_normalize_ospath(const char *ptr, long len);
#endif
RUBY_SYMBOL_EXPORT_END

#endif 
