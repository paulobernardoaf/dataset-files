#include "ruby/ruby.h" 
struct rb_thread_struct; 
struct rb_fiber_struct; 
VALUE rb_obj_is_fiber(VALUE);
void rb_fiber_reset_root_local_storage(struct rb_thread_struct *);
void ruby_register_rollback_func_for_ensure(VALUE (*ensure_func)(VALUE), VALUE (*rollback_func)(VALUE));
void rb_fiber_init_mjit_cont(struct rb_fiber_struct *fiber);
