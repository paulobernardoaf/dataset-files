











#include "ruby/config.h"

#if USE_MJIT

#include "internal.h"
#include "internal/compile.h"
#include "internal/hash.h"
#include "internal/variable.h"
#include "mjit.h"
#include "vm_core.h"
#include "vm_callinfo.h"
#include "vm_exec.h"
#include "vm_insnhelper.h"

#include "builtin.h"
#include "insns.inc"
#include "insns_info.inc"


#define NOT_COMPILED_STACK_SIZE -1
#define ALREADY_COMPILED_P(status, pos) (status->stack_size_for_pos[pos] != NOT_COMPILED_STACK_SIZE)


struct inlined_call_context {
int orig_argc; 
VALUE me; 
int param_size; 
int local_size; 
};




struct compile_status {
bool success; 
int *stack_size_for_pos; 


bool local_stack_p;

union iseq_inline_storage_entry *is_entries;

int cc_entries_index;

const struct rb_iseq_constant_body *compiled_iseq;

struct rb_mjit_compile_info *compile_info;

const struct rb_iseq_constant_body **inlined_iseqs;
struct inlined_call_context inline_context;
};




struct compile_branch {
unsigned int stack_size; 
bool finish_p; 
};

struct case_dispatch_var {
FILE *f;
unsigned int base_pos;
VALUE last_value;
};

static size_t
call_data_index(CALL_DATA cd, const struct rb_iseq_constant_body *body)
{
return cd - body->call_data;
}

const struct rb_callcache ** mjit_iseq_cc_entries(const struct rb_iseq_constant_body *const body);




static const struct rb_callcache **
captured_cc_entries(const struct compile_status *status)
{
VM_ASSERT(status->cc_entries_index != -1);
return mjit_iseq_cc_entries(status->compiled_iseq) + status->cc_entries_index;
}


static bool
has_valid_method_type(CALL_CACHE cc)
{
return vm_cc_cme(cc) != NULL;
}



static bool
fastpath_applied_iseq_p(const CALL_INFO ci, const CALL_CACHE cc, const rb_iseq_t *iseq)
{
extern bool rb_simple_iseq_p(const rb_iseq_t *iseq);
return iseq != NULL
&& !(vm_ci_flag(ci) & VM_CALL_KW_SPLAT) && rb_simple_iseq_p(iseq) 
&& vm_ci_argc(ci) == (unsigned int)iseq->body->param.lead_num 
&& vm_call_iseq_optimizable_p(ci, cc); 
}

static int
compile_case_dispatch_each(VALUE key, VALUE value, VALUE arg)
{
struct case_dispatch_var *var = (struct case_dispatch_var *)arg;
unsigned int offset;

if (var->last_value != value) {
offset = FIX2INT(value);
var->last_value = value;
fprintf(var->f, " case %d:\n", offset);
fprintf(var->f, " goto label_%d;\n", var->base_pos + offset);
fprintf(var->f, " break;\n");
}
return ST_CONTINUE;
}


static void
comment_id(FILE *f, ID id)
{
#if defined(MJIT_COMMENT_ID)
VALUE name = rb_id2str(id);
const char *p, *e;
char c, prev = '\0';

if (!name) return;
p = RSTRING_PTR(name);
e = RSTRING_END(name);
fputs("/* :\"", f);
for (; p < e; ++p) {
switch (c = *p) {
case '*': case '/': if (prev != (c ^ ('/' ^ '*'))) break;
case '\\': case '"': fputc('\\', f);
}
fputc(c, f);
prev = c;
}
fputs("\" */", f);
#endif
}

static void compile_insns(FILE *f, const struct rb_iseq_constant_body *body, unsigned int stack_size,
unsigned int pos, struct compile_status *status);







static unsigned int
compile_insn(FILE *f, const struct rb_iseq_constant_body *body, const int insn, const VALUE *operands,
const unsigned int pos, struct compile_status *status, struct compile_branch *b)
{
unsigned int next_pos = pos + insn_len(insn);


#include "mjit_compile.inc"




if (!b->finish_p && next_pos < body->iseq_size && ALREADY_COMPILED_P(status, next_pos)) {
fprintf(f, "goto label_%d;\n", next_pos);


if ((unsigned int)status->stack_size_for_pos[next_pos] != b->stack_size) {
if (mjit_opts.warnings || mjit_opts.verbose)
fprintf(stderr, "MJIT warning: JIT stack assumption is not the same between branches (%d != %u)\n",
status->stack_size_for_pos[next_pos], b->stack_size);
status->success = false;
}
}

return next_pos;
}



static void
compile_insns(FILE *f, const struct rb_iseq_constant_body *body, unsigned int stack_size,
unsigned int pos, struct compile_status *status)
{
int insn;
struct compile_branch branch;

branch.stack_size = stack_size;
branch.finish_p = false;

while (pos < body->iseq_size && !ALREADY_COMPILED_P(status, pos) && !branch.finish_p) {
#if OPT_DIRECT_THREADED_CODE || OPT_CALL_THREADED_CODE
insn = rb_vm_insn_addr2insn((void *)body->iseq_encoded[pos]);
#else
insn = (int)body->iseq_encoded[pos];
#endif
status->stack_size_for_pos[pos] = (int)branch.stack_size;

fprintf(f, "\nlabel_%d: /* %s */\n", pos, insn_name(insn));
pos = compile_insn(f, body, insn, body->iseq_encoded + (pos+1), pos, status, &branch);
if (status->success && branch.stack_size > body->stack_max) {
if (mjit_opts.warnings || mjit_opts.verbose)
fprintf(stderr, "MJIT warning: JIT stack size (%d) exceeded its max size (%d)\n", branch.stack_size, body->stack_max);
status->success = false;
}
if (!status->success)
break;
}
}


static void
compile_inlined_cancel_handler(FILE *f, const struct rb_iseq_constant_body *body, struct inlined_call_context *inline_context)
{
fprintf(f, "\ncancel:\n");
fprintf(f, " RB_DEBUG_COUNTER_INC(mjit_cancel);\n");
fprintf(f, " rb_mjit_iseq_compile_info(original_iseq->body)->disable_inlining = true;\n");
fprintf(f, " rb_mjit_recompile_iseq(original_iseq);\n");


fprintf(f, " const VALUE current_pc = reg_cfp->pc;\n");
fprintf(f, " const VALUE current_sp = reg_cfp->sp;\n");
fprintf(f, " reg_cfp->pc = orig_pc;\n");
fprintf(f, " reg_cfp->sp = orig_sp;\n\n");


fprintf(f, " struct rb_calling_info calling;\n");
fprintf(f, " calling.block_handler = VM_BLOCK_HANDLER_NONE;\n"); 
fprintf(f, " calling.argc = %d;\n", inline_context->orig_argc);
fprintf(f, " calling.recv = reg_cfp->self;\n");
fprintf(f, " reg_cfp->self = orig_self;\n");
fprintf(f, " vm_call_iseq_setup_normal(ec, reg_cfp, &calling, (const rb_callable_method_entry_t *)0x%"PRIxVALUE", 0, %d, %d);\n\n",
inline_context->me, inline_context->param_size, inline_context->local_size); 


fprintf(f, " reg_cfp = ec->cfp;\n"); 
fprintf(f, " reg_cfp->pc = current_pc;\n");
fprintf(f, " reg_cfp->sp = current_sp;\n");
for (unsigned int i = 0; i < body->stack_max; i++) { 
fprintf(f, " *(vm_base_ptr(reg_cfp) + %d) = stack[%d];\n", i, i);
}


fprintf(f, " return vm_exec(ec, ec->cfp);\n");
}


static void
compile_cancel_handler(FILE *f, const struct rb_iseq_constant_body *body, struct compile_status *status)
{
if (status->inlined_iseqs == NULL) { 
compile_inlined_cancel_handler(f, body, &status->inline_context);
return;
}

fprintf(f, "\nsend_cancel:\n");
fprintf(f, " RB_DEBUG_COUNTER_INC(mjit_cancel_send_inline);\n");
fprintf(f, " rb_mjit_iseq_compile_info(original_iseq->body)->disable_send_cache = true;\n");
fprintf(f, " rb_mjit_recompile_iseq(original_iseq);\n");
fprintf(f, " goto cancel;\n");

fprintf(f, "\nivar_cancel:\n");
fprintf(f, " RB_DEBUG_COUNTER_INC(mjit_cancel_ivar_inline);\n");
fprintf(f, " rb_mjit_iseq_compile_info(original_iseq->body)->disable_ivar_cache = true;\n");
fprintf(f, " rb_mjit_recompile_iseq(original_iseq);\n");
fprintf(f, " goto cancel;\n");

fprintf(f, "\nexivar_cancel:\n");
fprintf(f, " RB_DEBUG_COUNTER_INC(mjit_cancel_exivar_inline);\n");
fprintf(f, " rb_mjit_iseq_compile_info(original_iseq->body)->disable_exivar_cache = true;\n");
fprintf(f, " rb_mjit_recompile_iseq(original_iseq);\n");
fprintf(f, " goto cancel;\n");

fprintf(f, "\ncancel:\n");
fprintf(f, " RB_DEBUG_COUNTER_INC(mjit_cancel);\n");
if (status->local_stack_p) {
for (unsigned int i = 0; i < body->stack_max; i++) {
fprintf(f, " *(vm_base_ptr(reg_cfp) + %d) = stack[%d];\n", i, i);
}
}
fprintf(f, " return Qundef;\n");
}

extern int
mjit_capture_cc_entries(const struct rb_iseq_constant_body *compiled_iseq, const struct rb_iseq_constant_body *captured_iseq);

extern bool mjit_copy_cache_from_main_thread(const rb_iseq_t *iseq,
union iseq_inline_storage_entry *is_entries);

static bool
mjit_compile_body(FILE *f, const rb_iseq_t *iseq, struct compile_status *status)
{
const struct rb_iseq_constant_body *body = iseq->body;
status->success = true;
status->local_stack_p = !body->catch_except_p;

if (status->local_stack_p) {
fprintf(f, " VALUE stack[%d];\n", body->stack_max);
}
else {
fprintf(f, " VALUE *stack = reg_cfp->sp;\n");
}
if (status->inlined_iseqs != NULL) 
fprintf(f, " static const rb_iseq_t *original_iseq = (const rb_iseq_t *)0x%"PRIxVALUE";\n", (VALUE)iseq);
fprintf(f, " static const VALUE *const original_body_iseq = (VALUE *)0x%"PRIxVALUE";\n",
(VALUE)body->iseq_encoded);



if (body->param.flags.has_opt) {
int i;
fprintf(f, "\n");
fprintf(f, " switch (reg_cfp->pc - reg_cfp->iseq->body->iseq_encoded) {\n");
for (i = 0; i <= body->param.opt_num; i++) {
VALUE pc_offset = body->param.opt_table[i];
fprintf(f, " case %"PRIdVALUE":\n", pc_offset);
fprintf(f, " goto label_%"PRIdVALUE";\n", pc_offset);
}
fprintf(f, " }\n");
}

compile_insns(f, body, 0, 0, status);
compile_cancel_handler(f, body, status);
return status->success;
}


static bool
inlinable_iseq_p(const struct rb_iseq_constant_body *body)
{








if (body->catch_except_p)
return false;

unsigned int pos = 0;
while (pos < body->iseq_size) {
#if OPT_DIRECT_THREADED_CODE || OPT_CALL_THREADED_CODE
int insn = rb_vm_insn_addr2insn((void *)body->iseq_encoded[pos]);
#else
int insn = (int)body->iseq_encoded[pos];
#endif





if (insn != BIN(leave) && insn_may_depend_on_sp_or_pc(insn, body->iseq_encoded + (pos + 1)))
return false;

switch (insn) {
case BIN(getlocal):
case BIN(getlocal_WC_0):
case BIN(getlocal_WC_1):
case BIN(setlocal):
case BIN(setlocal_WC_0):
case BIN(setlocal_WC_1):
case BIN(getblockparam):
case BIN(getblockparamproxy):
case BIN(setblockparam):
return false;
}
pos += insn_len(insn);
}
return true;
}


#define INIT_COMPILE_STATUS(status, body, compile_root_p) do { status = (struct compile_status){ .stack_size_for_pos = (int *)alloca(sizeof(int) * body->iseq_size), .inlined_iseqs = compile_root_p ? alloca(sizeof(const struct rb_iseq_constant_body *) * body->iseq_size) : NULL, .is_entries = (body->is_size > 0) ? alloca(sizeof(union iseq_inline_storage_entry) * body->is_size) : NULL, .cc_entries_index = (body->ci_size > 0) ? mjit_capture_cc_entries(status.compiled_iseq, body) : -1, .compiled_iseq = status.compiled_iseq, .compile_info = compile_root_p ? rb_mjit_iseq_compile_info(body) : alloca(sizeof(struct rb_mjit_compile_info)) }; memset(status.stack_size_for_pos, NOT_COMPILED_STACK_SIZE, sizeof(int) * body->iseq_size); if (compile_root_p) memset((void *)status.inlined_iseqs, 0, sizeof(const struct rb_iseq_constant_body *) * body->iseq_size); else memset(status.compile_info, 0, sizeof(struct rb_mjit_compile_info)); } while (0)




















static bool
precompile_inlinable_iseqs(FILE *f, const rb_iseq_t *iseq, struct compile_status *status)
{
const struct rb_iseq_constant_body *body = iseq->body;
unsigned int pos = 0;
while (pos < body->iseq_size) {
#if OPT_DIRECT_THREADED_CODE || OPT_CALL_THREADED_CODE
int insn = rb_vm_insn_addr2insn((void *)body->iseq_encoded[pos]);
#else
int insn = (int)body->iseq_encoded[pos];
#endif
if (insn == BIN(opt_send_without_block)) { 
CALL_DATA cd = (CALL_DATA)body->iseq_encoded[pos + 1];
const struct rb_callinfo *ci = cd->ci;
const struct rb_callcache *cc = captured_cc_entries(status)[call_data_index(cd, body)]; 

const rb_iseq_t *child_iseq;
if (has_valid_method_type(cc) &&
!(vm_ci_flag(ci) & VM_CALL_TAILCALL) && 
vm_cc_cme(cc)->def->type == VM_METHOD_TYPE_ISEQ &&
fastpath_applied_iseq_p(ci, cc, child_iseq = def_iseq_ptr(vm_cc_cme(cc)->def)) &&

inlinable_iseq_p(child_iseq->body)) {
status->inlined_iseqs[pos] = child_iseq->body;

if (mjit_opts.verbose >= 1) 
fprintf(stderr, "JIT inline: %s@%s:%d => %s@%s:%d\n",
RSTRING_PTR(iseq->body->location.label),
RSTRING_PTR(rb_iseq_path(iseq)), FIX2INT(iseq->body->location.first_lineno),
RSTRING_PTR(child_iseq->body->location.label),
RSTRING_PTR(rb_iseq_path(child_iseq)), FIX2INT(child_iseq->body->location.first_lineno));

struct compile_status child_status = { .compiled_iseq = status->compiled_iseq };
INIT_COMPILE_STATUS(child_status, child_iseq->body, false);
child_status.inline_context = (struct inlined_call_context){
.orig_argc = vm_ci_argc(ci),
.me = (VALUE)vm_cc_cme(cc),
.param_size = child_iseq->body->param.size,
.local_size = child_iseq->body->local_table_size
};
if ((child_iseq->body->ci_size > 0 && child_status.cc_entries_index == -1)
|| (child_status.is_entries != NULL && !mjit_copy_cache_from_main_thread(child_iseq, child_status.is_entries))) {
return false;
}

fprintf(f, "ALWAYS_INLINE(static VALUE _mjit_inlined_%d(rb_execution_context_t *ec, rb_control_frame_t *reg_cfp, const VALUE orig_self, const rb_iseq_t *original_iseq));\n", pos);
fprintf(f, "static inline VALUE\n_mjit_inlined_%d(rb_execution_context_t *ec, rb_control_frame_t *reg_cfp, const VALUE orig_self, const rb_iseq_t *original_iseq)\n{\n", pos);
fprintf(f, " const VALUE *orig_pc = reg_cfp->pc;\n");
fprintf(f, " const VALUE *orig_sp = reg_cfp->sp;\n");
bool success = mjit_compile_body(f, child_iseq, &child_status);
fprintf(f, "\n} /* end of _mjit_inlined_%d */\n\n", pos);

if (!success)
return false;
}
}
pos += insn_len(insn);
}
return true;
}


bool
mjit_compile(FILE *f, const rb_iseq_t *iseq, const char *funcname)
{

if (!mjit_opts.debug) {
fprintf(f, "#undef OPT_CHECKED_RUN\n");
fprintf(f, "#define OPT_CHECKED_RUN 0\n\n");
}

struct compile_status status = { .compiled_iseq = iseq->body };
INIT_COMPILE_STATUS(status, iseq->body, true);
if ((iseq->body->ci_size > 0 && status.cc_entries_index == -1)
|| (status.is_entries != NULL && !mjit_copy_cache_from_main_thread(iseq, status.is_entries))) {
return false;
}

if (!status.compile_info->disable_send_cache && !status.compile_info->disable_inlining) {
if (!precompile_inlinable_iseqs(f, iseq, &status))
return false;
}

#if defined(_WIN32)
fprintf(f, "__declspec(dllexport)\n");
#endif
fprintf(f, "VALUE\n%s(rb_execution_context_t *ec, rb_control_frame_t *reg_cfp)\n{\n", funcname);
bool success = mjit_compile_body(f, iseq, &status);
fprintf(f, "\n} // end of %s\n", funcname);
return success;
}

#endif 
