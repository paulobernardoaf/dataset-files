#include "ruby/ruby.h" 
VALUE rb_get_expanded_load_path(void);
int rb_require_internal(VALUE fname);
NORETURN(void rb_load_fail(VALUE, const char*));
