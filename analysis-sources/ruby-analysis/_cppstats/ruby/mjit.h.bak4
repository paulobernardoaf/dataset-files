







#if !defined(RUBY_MJIT_H)
#define RUBY_MJIT_H 1

#include "ruby/config.h"

#if USE_MJIT

#include "debug_counter.h"
#include "ruby.h"
#include "vm_core.h"



enum rb_mjit_iseq_func {

NOT_ADDED_JIT_ISEQ_FUNC = 0,


NOT_READY_JIT_ISEQ_FUNC = 1,


NOT_COMPILED_JIT_ISEQ_FUNC = 2,

LAST_JIT_ISEQ_FUNC = 3
};


struct mjit_options {


char on;



char save_temps;

char warnings;


char debug;

char* debug_flags;

unsigned int wait;

unsigned int min_calls;


int verbose;


int max_cache_size;
};


struct rb_mjit_compile_info {

bool disable_ivar_cache;

bool disable_exivar_cache;

bool disable_send_cache;

bool disable_inlining;
};

typedef VALUE (*mjit_func_t)(rb_execution_context_t *, rb_control_frame_t *);

RUBY_SYMBOL_EXPORT_BEGIN
RUBY_EXTERN struct mjit_options mjit_opts;
RUBY_EXTERN bool mjit_call_p;

extern void rb_mjit_add_iseq_to_process(const rb_iseq_t *iseq);
extern VALUE rb_mjit_wait_call(rb_execution_context_t *ec, struct rb_iseq_constant_body *body);
extern struct rb_mjit_compile_info* rb_mjit_iseq_compile_info(const struct rb_iseq_constant_body *body);
extern void rb_mjit_recompile_iseq(const rb_iseq_t *iseq);
RUBY_SYMBOL_EXPORT_END

extern bool mjit_compile(FILE *f, const rb_iseq_t *iseq, const char *funcname);
extern void mjit_init(const struct mjit_options *opts);
extern void mjit_gc_start_hook(void);
extern void mjit_gc_exit_hook(void);
extern void mjit_free_iseq(const rb_iseq_t *iseq);
extern void mjit_update_references(const rb_iseq_t *iseq);
extern void mjit_mark(void);
extern struct mjit_cont *mjit_cont_new(rb_execution_context_t *ec);
extern void mjit_cont_free(struct mjit_cont *cont);
extern void mjit_add_class_serial(rb_serial_t class_serial);
extern void mjit_remove_class_serial(rb_serial_t class_serial);
extern void mjit_mark_cc_entries(const struct rb_iseq_constant_body *const body);



#define JIT_ISEQ_SIZE_THRESHOLD 1000


static inline int
mjit_target_iseq_p(struct rb_iseq_constant_body *body)
{
return (body->type == ISEQ_TYPE_METHOD || body->type == ISEQ_TYPE_BLOCK)
&& body->iseq_size < JIT_ISEQ_SIZE_THRESHOLD;
}



static inline VALUE
mjit_exec(rb_execution_context_t *ec)
{
const rb_iseq_t *iseq;
struct rb_iseq_constant_body *body;
long unsigned total_calls;
mjit_func_t func;

if (!mjit_call_p)
return Qundef;
RB_DEBUG_COUNTER_INC(mjit_exec);

iseq = ec->cfp->iseq;
body = iseq->body;
total_calls = ++body->total_calls;

func = body->jit_func;
uintptr_t func_i = (uintptr_t)func;
if (UNLIKELY(func_i <= LAST_JIT_ISEQ_FUNC)) {
#if defined(MJIT_HEADER)
RB_DEBUG_COUNTER_INC(mjit_frame_JT2VM);
#else
RB_DEBUG_COUNTER_INC(mjit_frame_VM2VM);
#endif
ASSUME(func_i <= LAST_JIT_ISEQ_FUNC);
switch ((enum rb_mjit_iseq_func)func_i) {
case NOT_ADDED_JIT_ISEQ_FUNC:
RB_DEBUG_COUNTER_INC(mjit_exec_not_added);
if (total_calls == mjit_opts.min_calls && mjit_target_iseq_p(body)) {
rb_mjit_add_iseq_to_process(iseq);
if (UNLIKELY(mjit_opts.wait)) {
return rb_mjit_wait_call(ec, body);
}
}
return Qundef;
case NOT_READY_JIT_ISEQ_FUNC:
RB_DEBUG_COUNTER_INC(mjit_exec_not_ready);
return Qundef;
case NOT_COMPILED_JIT_ISEQ_FUNC:
RB_DEBUG_COUNTER_INC(mjit_exec_not_compiled);
return Qundef;
default: 
break;
}
}

#if defined(MJIT_HEADER)
RB_DEBUG_COUNTER_INC(mjit_frame_JT2JT);
#else
RB_DEBUG_COUNTER_INC(mjit_frame_VM2JT);
#endif
RB_DEBUG_COUNTER_INC(mjit_exec_call_func);
return func(ec, ec->cfp);
}

void mjit_child_after_fork(void);

#else 
static inline struct mjit_cont *mjit_cont_new(rb_execution_context_t *ec){return NULL;}
static inline void mjit_cont_free(struct mjit_cont *cont){}
static inline void mjit_gc_start_hook(void){}
static inline void mjit_gc_exit_hook(void){}
static inline void mjit_free_iseq(const rb_iseq_t *iseq){}
static inline void mjit_mark(void){}
static inline void mjit_add_class_serial(rb_serial_t class_serial){}
static inline void mjit_remove_class_serial(rb_serial_t class_serial){}
static inline VALUE mjit_exec(rb_execution_context_t *ec) { return Qundef; }
static inline void mjit_child_after_fork(void){}

#endif 
#endif 
