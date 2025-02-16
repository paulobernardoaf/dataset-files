#if !defined(INTERNAL_PROC_H)
#define INTERNAL_PROC_H









#include "ruby/ruby.h" 
#include "ruby/st.h" 
struct rb_block; 
struct rb_iseq_struct; 


VALUE rb_proc_location(VALUE self);
st_index_t rb_hash_proc(st_index_t hash, VALUE proc);
int rb_block_pair_yield_optimizable(void);
int rb_block_arity(void);
int rb_block_min_max_arity(int *max);
VALUE rb_block_to_s(VALUE self, const struct rb_block *block, const char *additional_info);

MJIT_SYMBOL_EXPORT_BEGIN
VALUE rb_func_proc_new(rb_block_call_func_t func, VALUE val);
VALUE rb_func_lambda_new(rb_block_call_func_t func, VALUE val, int min_argc, int max_argc);
VALUE rb_iseq_location(const struct rb_iseq_struct *iseq);
VALUE rb_sym_to_proc(VALUE sym);
MJIT_SYMBOL_EXPORT_END

#endif 
