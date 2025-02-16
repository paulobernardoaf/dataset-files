#include "ruby/config.h"
#include <stddef.h> 
#include "ruby/ruby.h" 
struct rb_iseq_struct; 
int rb_dvar_defined(ID, const struct rb_iseq_struct *);
int rb_local_defined(ID, const struct rb_iseq_struct *);
const char *rb_insns_name(int i);
VALUE rb_insns_name_array(void);
int rb_vm_insn_addr2insn(const void *);
MJIT_SYMBOL_EXPORT_BEGIN
rb_event_flag_t rb_iseq_event_flags(const struct rb_iseq_struct *iseq, size_t pos);
MJIT_SYMBOL_EXPORT_END
