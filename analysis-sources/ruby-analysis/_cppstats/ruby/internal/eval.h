#include "ruby/ruby.h" 
#define id_signo ruby_static_id_signo
#define id_status ruby_static_id_status
extern ID ruby_static_id_signo;
extern ID ruby_static_id_status;
VALUE rb_refinement_module_get_refined_class(VALUE module);
void rb_class_modify_check(VALUE);
NORETURN(VALUE rb_f_raise(int argc, VALUE *argv));
VALUE rb_get_backtrace(VALUE info);
void rb_call_end_proc(VALUE data);
void rb_mark_end_proc(void);
