#include "ruby/ruby.h" 
struct rb_iseq_struct; 
VALUE rb_parser_set_yydebug(VALUE, VALUE);
void *rb_parser_load_file(VALUE parser, VALUE name);
RUBY_SYMBOL_EXPORT_BEGIN
VALUE rb_parser_set_context(VALUE, const struct rb_iseq_struct *, int);
RUBY_SYMBOL_EXPORT_END
