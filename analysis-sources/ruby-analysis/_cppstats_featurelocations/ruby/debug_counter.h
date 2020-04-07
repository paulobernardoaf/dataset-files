









#if !defined(USE_DEBUG_COUNTER)
#define USE_DEBUG_COUNTER 0
#endif

#if defined(RB_DEBUG_COUNTER)


RB_DEBUG_COUNTER(mc_inline_hit) 
RB_DEBUG_COUNTER(mc_inline_miss_klass) 
RB_DEBUG_COUNTER(mc_inline_miss_invalidated) 
RB_DEBUG_COUNTER(mc_cme_complement) 
RB_DEBUG_COUNTER(mc_cme_complement_hit) 

RB_DEBUG_COUNTER(mc_search) 
RB_DEBUG_COUNTER(mc_search_notfound) 
RB_DEBUG_COUNTER(mc_search_super) 


RB_DEBUG_COUNTER(ci_packed) 
RB_DEBUG_COUNTER(ci_kw) 
RB_DEBUG_COUNTER(ci_nokw) 
RB_DEBUG_COUNTER(ci_runtime) 


RB_DEBUG_COUNTER(cc_new) 
RB_DEBUG_COUNTER(cc_temp) 
RB_DEBUG_COUNTER(cc_found_ccs) 

RB_DEBUG_COUNTER(cc_ent_invalidate) 
RB_DEBUG_COUNTER(cc_cme_invalidate) 

RB_DEBUG_COUNTER(cc_invalidate_leaf) 
RB_DEBUG_COUNTER(cc_invalidate_leaf_ccs) 
RB_DEBUG_COUNTER(cc_invalidate_leaf_callable) 
RB_DEBUG_COUNTER(cc_invalidate_tree) 
RB_DEBUG_COUNTER(cc_invalidate_tree_cme) 
RB_DEBUG_COUNTER(cc_invalidate_tree_callable) 

RB_DEBUG_COUNTER(ccs_free) 
RB_DEBUG_COUNTER(ccs_maxlen) 
RB_DEBUG_COUNTER(ccs_found) 


RB_DEBUG_COUNTER(iseq_num) 
RB_DEBUG_COUNTER(iseq_cd_num) 




RB_DEBUG_COUNTER(ccf_general)
RB_DEBUG_COUNTER(ccf_iseq_setup)
RB_DEBUG_COUNTER(ccf_iseq_setup_0start)
RB_DEBUG_COUNTER(ccf_iseq_setup_tailcall_0start)
RB_DEBUG_COUNTER(ccf_iseq_fix) 
RB_DEBUG_COUNTER(ccf_iseq_opt) 
RB_DEBUG_COUNTER(ccf_iseq_kw1) 
RB_DEBUG_COUNTER(ccf_iseq_kw2) 
RB_DEBUG_COUNTER(ccf_cfunc)
RB_DEBUG_COUNTER(ccf_ivar) 
RB_DEBUG_COUNTER(ccf_attrset) 
RB_DEBUG_COUNTER(ccf_method_missing)
RB_DEBUG_COUNTER(ccf_zsuper)
RB_DEBUG_COUNTER(ccf_bmethod)
RB_DEBUG_COUNTER(ccf_opt_send)
RB_DEBUG_COUNTER(ccf_opt_call)
RB_DEBUG_COUNTER(ccf_opt_block_call)
RB_DEBUG_COUNTER(ccf_super_method)











RB_DEBUG_COUNTER(frame_push)
RB_DEBUG_COUNTER(frame_push_method)
RB_DEBUG_COUNTER(frame_push_block)
RB_DEBUG_COUNTER(frame_push_class)
RB_DEBUG_COUNTER(frame_push_top)
RB_DEBUG_COUNTER(frame_push_cfunc)
RB_DEBUG_COUNTER(frame_push_ifunc)
RB_DEBUG_COUNTER(frame_push_eval)
RB_DEBUG_COUNTER(frame_push_rescue)
RB_DEBUG_COUNTER(frame_push_dummy)

RB_DEBUG_COUNTER(frame_R2R)
RB_DEBUG_COUNTER(frame_R2C)
RB_DEBUG_COUNTER(frame_C2C)
RB_DEBUG_COUNTER(frame_C2R)












RB_DEBUG_COUNTER(ivar_get_ic_hit)
RB_DEBUG_COUNTER(ivar_get_ic_miss)
RB_DEBUG_COUNTER(ivar_get_ic_miss_serial)
RB_DEBUG_COUNTER(ivar_get_ic_miss_unset)
RB_DEBUG_COUNTER(ivar_get_ic_miss_noobject)
RB_DEBUG_COUNTER(ivar_set_ic_hit)
RB_DEBUG_COUNTER(ivar_set_ic_miss)
RB_DEBUG_COUNTER(ivar_set_ic_miss_serial)
RB_DEBUG_COUNTER(ivar_set_ic_miss_unset)
RB_DEBUG_COUNTER(ivar_set_ic_miss_oorange)
RB_DEBUG_COUNTER(ivar_set_ic_miss_noobject)
RB_DEBUG_COUNTER(ivar_get_base)
RB_DEBUG_COUNTER(ivar_set_base)








RB_DEBUG_COUNTER(lvar_get)
RB_DEBUG_COUNTER(lvar_get_dynamic)
RB_DEBUG_COUNTER(lvar_set)
RB_DEBUG_COUNTER(lvar_set_dynamic)
RB_DEBUG_COUNTER(lvar_set_slowpath)









RB_DEBUG_COUNTER(gc_count)
RB_DEBUG_COUNTER(gc_minor_newobj)
RB_DEBUG_COUNTER(gc_minor_malloc)
RB_DEBUG_COUNTER(gc_minor_method)
RB_DEBUG_COUNTER(gc_minor_capi)
RB_DEBUG_COUNTER(gc_minor_stress)
RB_DEBUG_COUNTER(gc_major_nofree)
RB_DEBUG_COUNTER(gc_major_oldgen)
RB_DEBUG_COUNTER(gc_major_shady)
RB_DEBUG_COUNTER(gc_major_force)
RB_DEBUG_COUNTER(gc_major_oldmalloc)

RB_DEBUG_COUNTER(gc_isptr_trial)
RB_DEBUG_COUNTER(gc_isptr_range)
RB_DEBUG_COUNTER(gc_isptr_align)
RB_DEBUG_COUNTER(gc_isptr_maybe)







































RB_DEBUG_COUNTER(obj_newobj)
RB_DEBUG_COUNTER(obj_newobj_slowpath)
RB_DEBUG_COUNTER(obj_newobj_wb_unprotected)
RB_DEBUG_COUNTER(obj_free)
RB_DEBUG_COUNTER(obj_promote)
RB_DEBUG_COUNTER(obj_wb_unprotect)

RB_DEBUG_COUNTER(obj_obj_embed)
RB_DEBUG_COUNTER(obj_obj_transient)
RB_DEBUG_COUNTER(obj_obj_ptr)

RB_DEBUG_COUNTER(obj_str_ptr)
RB_DEBUG_COUNTER(obj_str_embed)
RB_DEBUG_COUNTER(obj_str_shared)
RB_DEBUG_COUNTER(obj_str_nofree)
RB_DEBUG_COUNTER(obj_str_fstr)

RB_DEBUG_COUNTER(obj_ary_embed)
RB_DEBUG_COUNTER(obj_ary_transient)
RB_DEBUG_COUNTER(obj_ary_ptr)
RB_DEBUG_COUNTER(obj_ary_extracapa)






RB_DEBUG_COUNTER(obj_ary_shared_create)
RB_DEBUG_COUNTER(obj_ary_shared)
RB_DEBUG_COUNTER(obj_ary_shared_root_occupied)

RB_DEBUG_COUNTER(obj_hash_empty)
RB_DEBUG_COUNTER(obj_hash_1)
RB_DEBUG_COUNTER(obj_hash_2)
RB_DEBUG_COUNTER(obj_hash_3)
RB_DEBUG_COUNTER(obj_hash_4)
RB_DEBUG_COUNTER(obj_hash_5_8)
RB_DEBUG_COUNTER(obj_hash_g8)

RB_DEBUG_COUNTER(obj_hash_null)
RB_DEBUG_COUNTER(obj_hash_ar)
RB_DEBUG_COUNTER(obj_hash_st)
RB_DEBUG_COUNTER(obj_hash_transient)
RB_DEBUG_COUNTER(obj_hash_force_convert)

RB_DEBUG_COUNTER(obj_struct_embed)
RB_DEBUG_COUNTER(obj_struct_transient)
RB_DEBUG_COUNTER(obj_struct_ptr)

RB_DEBUG_COUNTER(obj_data_empty)
RB_DEBUG_COUNTER(obj_data_xfree)
RB_DEBUG_COUNTER(obj_data_imm_free)
RB_DEBUG_COUNTER(obj_data_zombie)

RB_DEBUG_COUNTER(obj_match_under4)
RB_DEBUG_COUNTER(obj_match_ge4)
RB_DEBUG_COUNTER(obj_match_ge8)
RB_DEBUG_COUNTER(obj_match_ptr)

RB_DEBUG_COUNTER(obj_iclass_ptr)
RB_DEBUG_COUNTER(obj_class_ptr)
RB_DEBUG_COUNTER(obj_module_ptr)

RB_DEBUG_COUNTER(obj_bignum_ptr)
RB_DEBUG_COUNTER(obj_bignum_embed)
RB_DEBUG_COUNTER(obj_float)
RB_DEBUG_COUNTER(obj_complex)
RB_DEBUG_COUNTER(obj_rational)

RB_DEBUG_COUNTER(obj_regexp_ptr)
RB_DEBUG_COUNTER(obj_file_ptr)
RB_DEBUG_COUNTER(obj_symbol)

RB_DEBUG_COUNTER(obj_imemo_ment)
RB_DEBUG_COUNTER(obj_imemo_iseq)
RB_DEBUG_COUNTER(obj_imemo_env)
RB_DEBUG_COUNTER(obj_imemo_tmpbuf)
RB_DEBUG_COUNTER(obj_imemo_ast)
RB_DEBUG_COUNTER(obj_imemo_cref)
RB_DEBUG_COUNTER(obj_imemo_svar)
RB_DEBUG_COUNTER(obj_imemo_throw_data)
RB_DEBUG_COUNTER(obj_imemo_ifunc)
RB_DEBUG_COUNTER(obj_imemo_memo)
RB_DEBUG_COUNTER(obj_imemo_parser_strterm)
RB_DEBUG_COUNTER(obj_imemo_callinfo)
RB_DEBUG_COUNTER(obj_imemo_callcache)


RB_DEBUG_COUNTER(artable_hint_hit)
RB_DEBUG_COUNTER(artable_hint_miss)
RB_DEBUG_COUNTER(artable_hint_notfound)





RB_DEBUG_COUNTER(heap_xmalloc)
RB_DEBUG_COUNTER(heap_xrealloc)
RB_DEBUG_COUNTER(heap_xfree)


RB_DEBUG_COUNTER(theap_alloc)
RB_DEBUG_COUNTER(theap_alloc_fail)
RB_DEBUG_COUNTER(theap_evacuate)


RB_DEBUG_COUNTER(mjit_exec)
RB_DEBUG_COUNTER(mjit_exec_not_added)
RB_DEBUG_COUNTER(mjit_exec_not_ready)
RB_DEBUG_COUNTER(mjit_exec_not_compiled)
RB_DEBUG_COUNTER(mjit_exec_call_func)


RB_DEBUG_COUNTER(mjit_add_iseq_to_process)
RB_DEBUG_COUNTER(mjit_unload_units)


RB_DEBUG_COUNTER(mjit_frame_VM2VM)
RB_DEBUG_COUNTER(mjit_frame_VM2JT)
RB_DEBUG_COUNTER(mjit_frame_JT2JT)
RB_DEBUG_COUNTER(mjit_frame_JT2VM)


RB_DEBUG_COUNTER(mjit_cancel)
RB_DEBUG_COUNTER(mjit_cancel_ivar_inline)
RB_DEBUG_COUNTER(mjit_cancel_exivar_inline)
RB_DEBUG_COUNTER(mjit_cancel_send_inline)
RB_DEBUG_COUNTER(mjit_cancel_opt_insn) 
RB_DEBUG_COUNTER(mjit_cancel_invalidate_all)


RB_DEBUG_COUNTER(mjit_length_unit_queue)
RB_DEBUG_COUNTER(mjit_length_active_units)
RB_DEBUG_COUNTER(mjit_length_compact_units)
RB_DEBUG_COUNTER(mjit_length_stale_units)


RB_DEBUG_COUNTER(mjit_compile_failures)






#endif

#if !defined(RUBY_DEBUG_COUNTER_H)
#define RUBY_DEBUG_COUNTER_H 1

#include "ruby/config.h"
#include <stddef.h> 
#include "ruby/ruby.h" 

#if !defined(__GNUC__) && USE_DEBUG_COUNTER
#error "USE_DEBUG_COUNTER is not supported by other than __GNUC__"
#endif

enum rb_debug_counter_type {
#define RB_DEBUG_COUNTER(name) RB_DEBUG_COUNTER_##name,
#include __FILE__
RB_DEBUG_COUNTER_MAX
#undef RB_DEBUG_COUNTER
};

#if USE_DEBUG_COUNTER
extern size_t rb_debug_counter[];

inline static int
rb_debug_counter_add(enum rb_debug_counter_type type, int add, int cond)
{
if (cond) {
rb_debug_counter[(int)type] += add;
}
return cond;
}

inline static int
rb_debug_counter_max(enum rb_debug_counter_type type, unsigned int num)
{
if (rb_debug_counter[(int)type] < num) {
rb_debug_counter[(int)type] = num;
return 1;
}
else {
return 0;
}
}

VALUE rb_debug_counter_reset(VALUE klass);
VALUE rb_debug_counter_show(VALUE klass);

#define RB_DEBUG_COUNTER_INC(type) rb_debug_counter_add(RB_DEBUG_COUNTER_##type, 1, 1)
#define RB_DEBUG_COUNTER_INC_UNLESS(type, cond) (!rb_debug_counter_add(RB_DEBUG_COUNTER_##type, 1, !(cond)))
#define RB_DEBUG_COUNTER_INC_IF(type, cond) rb_debug_counter_add(RB_DEBUG_COUNTER_##type, 1, (cond))
#define RB_DEBUG_COUNTER_ADD(type, num) rb_debug_counter_add(RB_DEBUG_COUNTER_##type, (num), 1)
#define RB_DEBUG_COUNTER_SETMAX(type, num) rb_debug_counter_max(RB_DEBUG_COUNTER_##type, (unsigned int)(num))

#else
#define RB_DEBUG_COUNTER_INC(type) ((void)0)
#define RB_DEBUG_COUNTER_INC_UNLESS(type, cond) (cond)
#define RB_DEBUG_COUNTER_INC_IF(type, cond) (cond)
#define RB_DEBUG_COUNTER_ADD(type, num) ((void)0)
#define RB_DEBUG_COUNTER_SETMAX(type, num) 0
#endif

void rb_debug_counter_show_results(const char *msg);

RUBY_SYMBOL_EXPORT_BEGIN

size_t ruby_debug_counter_get(const char **names_ptr, size_t *counters_ptr);
void ruby_debug_counter_reset(void);
void ruby_debug_counter_show_at_exit(int enable);

RUBY_SYMBOL_EXPORT_END

#endif 
