#define USING_FLOAT_STUFF
#include "vim.h"
#if defined(FEAT_EVAL) || defined(PROTO)
#if defined(VMS)
#include <float.h>
#endif
#define DEFINE_VIM9_GLOBALS
#include "vim9.h"
typedef struct endlabel_S endlabel_T;
struct endlabel_S {
endlabel_T *el_next; 
int el_end_label; 
};
typedef struct {
int is_if_label; 
endlabel_T *is_end_label; 
} ifscope_T;
typedef struct {
int ws_top_label; 
endlabel_T *ws_end_label; 
} whilescope_T;
typedef struct {
int fs_top_label; 
endlabel_T *fs_end_label; 
} forscope_T;
typedef struct {
int ts_try_label; 
endlabel_T *ts_end_label; 
int ts_catch_label; 
int ts_caught_all; 
} tryscope_T;
typedef enum {
NO_SCOPE,
IF_SCOPE,
WHILE_SCOPE,
FOR_SCOPE,
TRY_SCOPE,
BLOCK_SCOPE
} scopetype_T;
typedef struct scope_S scope_T;
struct scope_S {
scope_T *se_outer; 
scopetype_T se_type;
int se_local_count; 
union {
ifscope_T se_if;
whilescope_T se_while;
forscope_T se_for;
tryscope_T se_try;
} se_u;
};
typedef struct {
char_u *lv_name;
type_T *lv_type;
int lv_const; 
int lv_arg; 
} lvar_T;
struct cctx_S {
ufunc_T *ctx_ufunc; 
int ctx_lnum; 
garray_T ctx_instr; 
garray_T ctx_locals; 
int ctx_max_local; 
garray_T ctx_imports; 
int ctx_skip; 
scope_T *ctx_scope; 
garray_T ctx_type_stack; 
garray_T *ctx_type_list; 
};
static char e_var_notfound[] = N_("E1001: variable not found: %s");
static char e_syntax_at[] = N_("E1002: Syntax error at %s");
static int compile_expr1(char_u **arg, cctx_T *cctx);
static int compile_expr2(char_u **arg, cctx_T *cctx);
static int compile_expr3(char_u **arg, cctx_T *cctx);
static void delete_def_function_contents(dfunc_T *dfunc);
static int
lookup_local(char_u *name, size_t len, cctx_T *cctx)
{
int idx;
if (len == 0)
return -1;
for (idx = 0; idx < cctx->ctx_locals.ga_len; ++idx)
{
lvar_T *lvar = ((lvar_T *)cctx->ctx_locals.ga_data) + idx;
if (STRNCMP(name, lvar->lv_name, len) == 0
&& STRLEN(lvar->lv_name) == len)
return idx;
}
return -1;
}
static int
lookup_arg(char_u *name, size_t len, cctx_T *cctx)
{
int idx;
if (len == 0)
return -1;
for (idx = 0; idx < cctx->ctx_ufunc->uf_args.ga_len; ++idx)
{
char_u *arg = FUNCARG(cctx->ctx_ufunc, idx);
if (STRNCMP(name, arg, len) == 0 && STRLEN(arg) == len)
return idx;
}
return -1;
}
static int
lookup_vararg(char_u *name, size_t len, cctx_T *cctx)
{
char_u *va_name = cctx->ctx_ufunc->uf_va_name;
return len > 0 && va_name != NULL
&& STRNCMP(name, va_name, len) == 0 && STRLEN(va_name) == len;
}
static int
lookup_script(char_u *name, size_t len)
{
int cc;
hashtab_T *ht = &SCRIPT_VARS(current_sctx.sc_sid);
dictitem_T *di;
cc = name[len];
name[len] = NUL;
di = find_var_in_ht(ht, 0, name, TRUE);
name[len] = cc;
return di == NULL ? FAIL: OK;
}
int
check_defined(char_u *p, int len, cctx_T *cctx)
{
if (lookup_script(p, len) == OK
|| (cctx != NULL
&& (lookup_local(p, len, cctx) >= 0
|| find_imported(p, len, cctx) != NULL)))
{
semsg("E1073: imported name already defined: %s", p);
return FAIL;
}
return OK;
}
static type_T *
get_list_type(type_T *member_type, garray_T *type_list)
{
type_T *type;
if (member_type->tt_type == VAR_UNKNOWN)
return &t_list_any;
if (member_type->tt_type == VAR_VOID)
return &t_list_empty;
if (member_type->tt_type == VAR_BOOL)
return &t_list_bool;
if (member_type->tt_type == VAR_NUMBER)
return &t_list_number;
if (member_type->tt_type == VAR_STRING)
return &t_list_string;
if (ga_grow(type_list, 1) == FAIL)
return &t_any;
type = ((type_T *)type_list->ga_data) + type_list->ga_len;
++type_list->ga_len;
type->tt_type = VAR_LIST;
type->tt_member = member_type;
return type;
}
static type_T *
get_dict_type(type_T *member_type, garray_T *type_list)
{
type_T *type;
if (member_type->tt_type == VAR_UNKNOWN)
return &t_dict_any;
if (member_type->tt_type == VAR_VOID)
return &t_dict_empty;
if (member_type->tt_type == VAR_BOOL)
return &t_dict_bool;
if (member_type->tt_type == VAR_NUMBER)
return &t_dict_number;
if (member_type->tt_type == VAR_STRING)
return &t_dict_string;
if (ga_grow(type_list, 1) == FAIL)
return &t_any;
type = ((type_T *)type_list->ga_data) + type_list->ga_len;
++type_list->ga_len;
type->tt_type = VAR_DICT;
type->tt_member = member_type;
return type;
}
static type_T *
typval2type(typval_T *tv)
{
if (tv->v_type == VAR_NUMBER)
return &t_number;
if (tv->v_type == VAR_BOOL)
return &t_bool;
if (tv->v_type == VAR_STRING)
return &t_string;
if (tv->v_type == VAR_LIST) 
return &t_list_string;
if (tv->v_type == VAR_DICT) 
return &t_dict_any;
return &t_any;
}
#define RETURN_NULL_IF_SKIP(cctx) if (cctx->ctx_skip == TRUE) return NULL
#define RETURN_OK_IF_SKIP(cctx) if (cctx->ctx_skip == TRUE) return OK
static isn_T *
generate_instr(cctx_T *cctx, isntype_T isn_type)
{
garray_T *instr = &cctx->ctx_instr;
isn_T *isn;
RETURN_NULL_IF_SKIP(cctx);
if (ga_grow(instr, 1) == FAIL)
return NULL;
isn = ((isn_T *)instr->ga_data) + instr->ga_len;
isn->isn_type = isn_type;
isn->isn_lnum = cctx->ctx_lnum + 1;
++instr->ga_len;
return isn;
}
static isn_T *
generate_instr_drop(cctx_T *cctx, isntype_T isn_type, int drop)
{
garray_T *stack = &cctx->ctx_type_stack;
RETURN_NULL_IF_SKIP(cctx);
stack->ga_len -= drop;
return generate_instr(cctx, isn_type);
}
static isn_T *
generate_instr_type(cctx_T *cctx, isntype_T isn_type, type_T *type)
{
isn_T *isn;
garray_T *stack = &cctx->ctx_type_stack;
if ((isn = generate_instr(cctx, isn_type)) == NULL)
return NULL;
if (ga_grow(stack, 1) == FAIL)
return NULL;
((type_T **)stack->ga_data)[stack->ga_len] = type;
++stack->ga_len;
return isn;
}
static int
may_generate_2STRING(int offset, cctx_T *cctx)
{
isn_T *isn;
garray_T *stack = &cctx->ctx_type_stack;
type_T **type = ((type_T **)stack->ga_data) + stack->ga_len + offset;
if ((*type)->tt_type == VAR_STRING)
return OK;
*type = &t_string;
if ((isn = generate_instr(cctx, ISN_2STRING)) == NULL)
return FAIL;
isn->isn_arg.number = offset;
return OK;
}
static int
check_number_or_float(vartype_T type1, vartype_T type2, char_u *op)
{
if (!((type1 == VAR_NUMBER || type1 == VAR_FLOAT || type1 == VAR_UNKNOWN)
&& (type2 == VAR_NUMBER || type2 == VAR_FLOAT
|| type2 == VAR_UNKNOWN)))
{
if (*op == '+')
emsg(_("E1035: wrong argument type for +"));
else
semsg(_("E1036: %c requires number or float arguments"), *op);
return FAIL;
}
return OK;
}
static int
generate_two_op(cctx_T *cctx, char_u *op)
{
garray_T *stack = &cctx->ctx_type_stack;
type_T *type1;
type_T *type2;
vartype_T vartype;
isn_T *isn;
RETURN_OK_IF_SKIP(cctx);
type1 = ((type_T **)stack->ga_data)[stack->ga_len - 2];
type2 = ((type_T **)stack->ga_data)[stack->ga_len - 1];
vartype = VAR_UNKNOWN;
if (type1->tt_type == type2->tt_type
&& (type1->tt_type == VAR_NUMBER
|| type1->tt_type == VAR_LIST
#if defined(FEAT_FLOAT)
|| type1->tt_type == VAR_FLOAT
#endif
|| type1->tt_type == VAR_BLOB))
vartype = type1->tt_type;
switch (*op)
{
case '+': if (vartype != VAR_LIST && vartype != VAR_BLOB
&& type1->tt_type != VAR_UNKNOWN
&& type2->tt_type != VAR_UNKNOWN
&& check_number_or_float(
type1->tt_type, type2->tt_type, op) == FAIL)
return FAIL;
isn = generate_instr_drop(cctx,
vartype == VAR_NUMBER ? ISN_OPNR
: vartype == VAR_LIST ? ISN_ADDLIST
: vartype == VAR_BLOB ? ISN_ADDBLOB
#if defined(FEAT_FLOAT)
: vartype == VAR_FLOAT ? ISN_OPFLOAT
#endif
: ISN_OPANY, 1);
if (isn != NULL)
isn->isn_arg.op.op_type = EXPR_ADD;
break;
case '-':
case '*':
case '/': if (check_number_or_float(type1->tt_type, type2->tt_type,
op) == FAIL)
return FAIL;
if (vartype == VAR_NUMBER)
isn = generate_instr_drop(cctx, ISN_OPNR, 1);
#if defined(FEAT_FLOAT)
else if (vartype == VAR_FLOAT)
isn = generate_instr_drop(cctx, ISN_OPFLOAT, 1);
#endif
else
isn = generate_instr_drop(cctx, ISN_OPANY, 1);
if (isn != NULL)
isn->isn_arg.op.op_type = *op == '*'
? EXPR_MULT : *op == '/'? EXPR_DIV : EXPR_SUB;
break;
case '%': if ((type1->tt_type != VAR_UNKNOWN
&& type1->tt_type != VAR_NUMBER)
|| (type2->tt_type != VAR_UNKNOWN
&& type2->tt_type != VAR_NUMBER))
{
emsg(_("E1035: % requires number arguments"));
return FAIL;
}
isn = generate_instr_drop(cctx,
vartype == VAR_NUMBER ? ISN_OPNR : ISN_OPANY, 1);
if (isn != NULL)
isn->isn_arg.op.op_type = EXPR_REM;
break;
}
if (vartype == VAR_UNKNOWN)
{
type_T *type = &t_any;
#if defined(FEAT_FLOAT)
if ((type1->tt_type == VAR_NUMBER || type1->tt_type == VAR_FLOAT)
&& (type2->tt_type == VAR_NUMBER || type2->tt_type == VAR_FLOAT))
type = &t_float;
#endif
((type_T **)stack->ga_data)[stack->ga_len - 1] = type;
}
return OK;
}
static int
generate_COMPARE(cctx_T *cctx, exptype_T exptype, int ic)
{
isntype_T isntype = ISN_DROP;
isn_T *isn;
garray_T *stack = &cctx->ctx_type_stack;
vartype_T type1;
vartype_T type2;
RETURN_OK_IF_SKIP(cctx);
type1 = ((type_T **)stack->ga_data)[stack->ga_len - 2]->tt_type;
type2 = ((type_T **)stack->ga_data)[stack->ga_len - 1]->tt_type;
if (type1 == type2)
{
switch (type1)
{
case VAR_BOOL: isntype = ISN_COMPAREBOOL; break;
case VAR_SPECIAL: isntype = ISN_COMPARESPECIAL; break;
case VAR_NUMBER: isntype = ISN_COMPARENR; break;
case VAR_FLOAT: isntype = ISN_COMPAREFLOAT; break;
case VAR_STRING: isntype = ISN_COMPARESTRING; break;
case VAR_BLOB: isntype = ISN_COMPAREBLOB; break;
case VAR_LIST: isntype = ISN_COMPARELIST; break;
case VAR_DICT: isntype = ISN_COMPAREDICT; break;
case VAR_FUNC: isntype = ISN_COMPAREFUNC; break;
case VAR_PARTIAL: isntype = ISN_COMPAREPARTIAL; break;
default: isntype = ISN_COMPAREANY; break;
}
}
else if (type1 == VAR_UNKNOWN || type2 == VAR_UNKNOWN
|| ((type1 == VAR_NUMBER || type1 == VAR_FLOAT)
&& (type2 == VAR_NUMBER || type2 ==VAR_FLOAT)))
isntype = ISN_COMPAREANY;
if ((exptype == EXPR_IS || exptype == EXPR_ISNOT)
&& (isntype == ISN_COMPAREBOOL
|| isntype == ISN_COMPARESPECIAL
|| isntype == ISN_COMPARENR
|| isntype == ISN_COMPAREFLOAT))
{
semsg(_("E1037: Cannot use \"%s\" with %s"),
exptype == EXPR_IS ? "is" : "isnot" , vartype_name(type1));
return FAIL;
}
if (isntype == ISN_DROP
|| ((exptype != EXPR_EQUAL && exptype != EXPR_NEQUAL
&& (type1 == VAR_BOOL || type1 == VAR_SPECIAL
|| type2 == VAR_BOOL || type2 == VAR_SPECIAL)))
|| ((exptype != EXPR_EQUAL && exptype != EXPR_NEQUAL
&& exptype != EXPR_IS && exptype != EXPR_ISNOT
&& (type1 == VAR_BLOB || type2 == VAR_BLOB
|| type1 == VAR_LIST || type2 == VAR_LIST))))
{
semsg(_("E1072: Cannot compare %s with %s"),
vartype_name(type1), vartype_name(type2));
return FAIL;
}
if ((isn = generate_instr(cctx, isntype)) == NULL)
return FAIL;
isn->isn_arg.op.op_type = exptype;
isn->isn_arg.op.op_ic = ic;
if (stack->ga_len >= 2)
{
--stack->ga_len;
((type_T **)stack->ga_data)[stack->ga_len - 1] = &t_bool;
}
return OK;
}
static int
generate_2BOOL(cctx_T *cctx, int invert)
{
isn_T *isn;
garray_T *stack = &cctx->ctx_type_stack;
RETURN_OK_IF_SKIP(cctx);
if ((isn = generate_instr(cctx, ISN_2BOOL)) == NULL)
return FAIL;
isn->isn_arg.number = invert;
((type_T **)stack->ga_data)[stack->ga_len - 1] = &t_bool;
return OK;
}
static int
generate_TYPECHECK(cctx_T *cctx, type_T *vartype, int offset)
{
isn_T *isn;
garray_T *stack = &cctx->ctx_type_stack;
RETURN_OK_IF_SKIP(cctx);
if ((isn = generate_instr(cctx, ISN_CHECKTYPE)) == NULL)
return FAIL;
isn->isn_arg.type.ct_type = vartype->tt_type; 
isn->isn_arg.type.ct_off = offset;
((type_T **)stack->ga_data)[stack->ga_len - 1] = vartype;
return OK;
}
static int
generate_PUSHNR(cctx_T *cctx, varnumber_T number)
{
isn_T *isn;
RETURN_OK_IF_SKIP(cctx);
if ((isn = generate_instr_type(cctx, ISN_PUSHNR, &t_number)) == NULL)
return FAIL;
isn->isn_arg.number = number;
return OK;
}
static int
generate_PUSHBOOL(cctx_T *cctx, varnumber_T number)
{
isn_T *isn;
RETURN_OK_IF_SKIP(cctx);
if ((isn = generate_instr_type(cctx, ISN_PUSHBOOL, &t_bool)) == NULL)
return FAIL;
isn->isn_arg.number = number;
return OK;
}
static int
generate_PUSHSPEC(cctx_T *cctx, varnumber_T number)
{
isn_T *isn;
RETURN_OK_IF_SKIP(cctx);
if ((isn = generate_instr_type(cctx, ISN_PUSHSPEC, &t_special)) == NULL)
return FAIL;
isn->isn_arg.number = number;
return OK;
}
#if defined(FEAT_FLOAT)
static int
generate_PUSHF(cctx_T *cctx, float_T fnumber)
{
isn_T *isn;
RETURN_OK_IF_SKIP(cctx);
if ((isn = generate_instr_type(cctx, ISN_PUSHF, &t_float)) == NULL)
return FAIL;
isn->isn_arg.fnumber = fnumber;
return OK;
}
#endif
static int
generate_PUSHS(cctx_T *cctx, char_u *str)
{
isn_T *isn;
RETURN_OK_IF_SKIP(cctx);
if ((isn = generate_instr_type(cctx, ISN_PUSHS, &t_string)) == NULL)
return FAIL;
isn->isn_arg.string = str;
return OK;
}
static int
generate_PUSHCHANNEL(cctx_T *cctx, channel_T *channel)
{
isn_T *isn;
RETURN_OK_IF_SKIP(cctx);
if ((isn = generate_instr_type(cctx, ISN_PUSHCHANNEL, &t_channel)) == NULL)
return FAIL;
isn->isn_arg.channel = channel;
return OK;
}
static int
generate_PUSHJOB(cctx_T *cctx, job_T *job)
{
isn_T *isn;
RETURN_OK_IF_SKIP(cctx);
if ((isn = generate_instr_type(cctx, ISN_PUSHJOB, &t_channel)) == NULL)
return FAIL;
isn->isn_arg.job = job;
return OK;
}
static int
generate_PUSHBLOB(cctx_T *cctx, blob_T *blob)
{
isn_T *isn;
RETURN_OK_IF_SKIP(cctx);
if ((isn = generate_instr_type(cctx, ISN_PUSHBLOB, &t_blob)) == NULL)
return FAIL;
isn->isn_arg.blob = blob;
return OK;
}
static int
generate_PUSHFUNC(cctx_T *cctx, char_u *name)
{
isn_T *isn;
RETURN_OK_IF_SKIP(cctx);
if ((isn = generate_instr_type(cctx, ISN_PUSHFUNC, &t_func_void)) == NULL)
return FAIL;
isn->isn_arg.string = name;
return OK;
}
static int
generate_PUSHPARTIAL(cctx_T *cctx, partial_T *part)
{
isn_T *isn;
RETURN_OK_IF_SKIP(cctx);
if ((isn = generate_instr_type(cctx, ISN_PUSHPARTIAL,
&t_partial_any)) == NULL)
return FAIL;
isn->isn_arg.partial = part;
return OK;
}
static int
generate_STORE(cctx_T *cctx, isntype_T isn_type, int idx, char_u *name)
{
isn_T *isn;
RETURN_OK_IF_SKIP(cctx);
if ((isn = generate_instr_drop(cctx, isn_type, 1)) == NULL)
return FAIL;
if (name != NULL)
isn->isn_arg.string = vim_strsave(name);
else
isn->isn_arg.number = idx;
return OK;
}
static int
generate_STORENR(cctx_T *cctx, int idx, varnumber_T value)
{
isn_T *isn;
RETURN_OK_IF_SKIP(cctx);
if ((isn = generate_instr(cctx, ISN_STORENR)) == NULL)
return FAIL;
isn->isn_arg.storenr.stnr_idx = idx;
isn->isn_arg.storenr.stnr_val = value;
return OK;
}
static int
generate_STOREOPT(cctx_T *cctx, char_u *name, int opt_flags)
{
isn_T *isn;
RETURN_OK_IF_SKIP(cctx);
if ((isn = generate_instr(cctx, ISN_STOREOPT)) == NULL)
return FAIL;
isn->isn_arg.storeopt.so_name = vim_strsave(name);
isn->isn_arg.storeopt.so_flags = opt_flags;
return OK;
}
static int
generate_LOAD(
cctx_T *cctx,
isntype_T isn_type,
int idx,
char_u *name,
type_T *type)
{
isn_T *isn;
RETURN_OK_IF_SKIP(cctx);
if ((isn = generate_instr_type(cctx, isn_type, type)) == NULL)
return FAIL;
if (name != NULL)
isn->isn_arg.string = vim_strsave(name);
else
isn->isn_arg.number = idx;
return OK;
}
static int
generate_LOADV(
cctx_T *cctx,
char_u *name,
int error)
{
int vidx = find_vim_var(name);
RETURN_OK_IF_SKIP(cctx);
if (vidx < 0)
{
if (error)
semsg(_(e_var_notfound), name);
return FAIL;
}
return generate_LOAD(cctx, ISN_LOADV, vidx, NULL, &t_any);
}
static int
generate_OLDSCRIPT(
cctx_T *cctx,
isntype_T isn_type,
char_u *name,
int sid,
type_T *type)
{
isn_T *isn;
RETURN_OK_IF_SKIP(cctx);
if (isn_type == ISN_LOADS)
isn = generate_instr_type(cctx, isn_type, type);
else
isn = generate_instr_drop(cctx, isn_type, 1);
if (isn == NULL)
return FAIL;
isn->isn_arg.loadstore.ls_name = vim_strsave(name);
isn->isn_arg.loadstore.ls_sid = sid;
return OK;
}
static int
generate_VIM9SCRIPT(
cctx_T *cctx,
isntype_T isn_type,
int sid,
int idx,
type_T *type)
{
isn_T *isn;
RETURN_OK_IF_SKIP(cctx);
if (isn_type == ISN_LOADSCRIPT)
isn = generate_instr_type(cctx, isn_type, type);
else
isn = generate_instr_drop(cctx, isn_type, 1);
if (isn == NULL)
return FAIL;
isn->isn_arg.script.script_sid = sid;
isn->isn_arg.script.script_idx = idx;
return OK;
}
static int
generate_NEWLIST(cctx_T *cctx, int count)
{
isn_T *isn;
garray_T *stack = &cctx->ctx_type_stack;
garray_T *type_list = cctx->ctx_type_list;
type_T *type;
type_T *member;
RETURN_OK_IF_SKIP(cctx);
if ((isn = generate_instr(cctx, ISN_NEWLIST)) == NULL)
return FAIL;
isn->isn_arg.number = count;
stack->ga_len -= count;
if (count > 0)
member = ((type_T **)stack->ga_data)[stack->ga_len];
else
member = &t_void;
type = get_list_type(member, type_list);
if (ga_grow(stack, 1) == FAIL)
return FAIL;
((type_T **)stack->ga_data)[stack->ga_len] = type;
++stack->ga_len;
return OK;
}
static int
generate_NEWDICT(cctx_T *cctx, int count)
{
isn_T *isn;
garray_T *stack = &cctx->ctx_type_stack;
garray_T *type_list = cctx->ctx_type_list;
type_T *type;
type_T *member;
RETURN_OK_IF_SKIP(cctx);
if ((isn = generate_instr(cctx, ISN_NEWDICT)) == NULL)
return FAIL;
isn->isn_arg.number = count;
stack->ga_len -= 2 * count;
if (count > 0)
member = ((type_T **)stack->ga_data)[stack->ga_len + 1];
else
member = &t_void;
type = get_dict_type(member, type_list);
if (ga_grow(stack, 1) == FAIL)
return FAIL;
((type_T **)stack->ga_data)[stack->ga_len] = type;
++stack->ga_len;
return OK;
}
static int
generate_FUNCREF(cctx_T *cctx, int dfunc_idx)
{
isn_T *isn;
garray_T *stack = &cctx->ctx_type_stack;
RETURN_OK_IF_SKIP(cctx);
if ((isn = generate_instr(cctx, ISN_FUNCREF)) == NULL)
return FAIL;
isn->isn_arg.number = dfunc_idx;
if (ga_grow(stack, 1) == FAIL)
return FAIL;
((type_T **)stack->ga_data)[stack->ga_len] = &t_partial_any;
++stack->ga_len;
return OK;
}
static int
generate_JUMP(cctx_T *cctx, jumpwhen_T when, int where)
{
isn_T *isn;
garray_T *stack = &cctx->ctx_type_stack;
RETURN_OK_IF_SKIP(cctx);
if ((isn = generate_instr(cctx, ISN_JUMP)) == NULL)
return FAIL;
isn->isn_arg.jump.jump_when = when;
isn->isn_arg.jump.jump_where = where;
if (when != JUMP_ALWAYS && stack->ga_len > 0)
--stack->ga_len;
return OK;
}
static int
generate_FOR(cctx_T *cctx, int loop_idx)
{
isn_T *isn;
garray_T *stack = &cctx->ctx_type_stack;
RETURN_OK_IF_SKIP(cctx);
if ((isn = generate_instr(cctx, ISN_FOR)) == NULL)
return FAIL;
isn->isn_arg.forloop.for_idx = loop_idx;
if (ga_grow(stack, 1) == FAIL)
return FAIL;
((type_T **)stack->ga_data)[stack->ga_len] = &t_any;
++stack->ga_len;
return OK;
}
static int
generate_BCALL(cctx_T *cctx, int func_idx, int argcount)
{
isn_T *isn;
garray_T *stack = &cctx->ctx_type_stack;
type_T *argtypes[MAX_FUNC_ARGS];
int i;
RETURN_OK_IF_SKIP(cctx);
if (check_internal_func(func_idx, argcount) == FAIL)
return FAIL;
if ((isn = generate_instr(cctx, ISN_BCALL)) == NULL)
return FAIL;
isn->isn_arg.bfunc.cbf_idx = func_idx;
isn->isn_arg.bfunc.cbf_argcount = argcount;
for (i = 0; i < argcount; ++i)
argtypes[i] = ((type_T **)stack->ga_data)[stack->ga_len - argcount + i];
stack->ga_len -= argcount; 
if (ga_grow(stack, 1) == FAIL)
return FAIL;
((type_T **)stack->ga_data)[stack->ga_len] =
internal_func_ret_type(func_idx, argcount, argtypes);
++stack->ga_len; 
return OK;
}
static int
generate_CALL(cctx_T *cctx, ufunc_T *ufunc, int pushed_argcount)
{
isn_T *isn;
garray_T *stack = &cctx->ctx_type_stack;
int regular_args = ufunc->uf_args.ga_len;
int argcount = pushed_argcount;
RETURN_OK_IF_SKIP(cctx);
if (argcount > regular_args && !has_varargs(ufunc))
{
semsg(_(e_toomanyarg), ufunc->uf_name);
return FAIL;
}
if (argcount < regular_args - ufunc->uf_def_args.ga_len)
{
semsg(_(e_toofewarg), ufunc->uf_name);
return FAIL;
}
if (ufunc->uf_va_name != NULL)
{
int count = argcount - regular_args;
if (count >= 0)
{
generate_NEWLIST(cctx, count);
argcount = regular_args + 1;
}
}
if ((isn = generate_instr(cctx,
ufunc->uf_dfunc_idx >= 0 ? ISN_DCALL : ISN_UCALL)) == NULL)
return FAIL;
if (ufunc->uf_dfunc_idx >= 0)
{
isn->isn_arg.dfunc.cdf_idx = ufunc->uf_dfunc_idx;
isn->isn_arg.dfunc.cdf_argcount = argcount;
}
else
{
isn->isn_arg.ufunc.cuf_name = vim_strsave(ufunc->uf_name);
isn->isn_arg.ufunc.cuf_argcount = argcount;
}
stack->ga_len -= argcount; 
if (ga_grow(stack, 1) == FAIL)
return FAIL;
((type_T **)stack->ga_data)[stack->ga_len] = ufunc->uf_ret_type;
++stack->ga_len;
return OK;
}
static int
generate_UCALL(cctx_T *cctx, char_u *name, int argcount)
{
isn_T *isn;
garray_T *stack = &cctx->ctx_type_stack;
RETURN_OK_IF_SKIP(cctx);
if ((isn = generate_instr(cctx, ISN_UCALL)) == NULL)
return FAIL;
isn->isn_arg.ufunc.cuf_name = vim_strsave(name);
isn->isn_arg.ufunc.cuf_argcount = argcount;
stack->ga_len -= argcount; 
if (ga_grow(stack, 1) == FAIL)
return FAIL;
((type_T **)stack->ga_data)[stack->ga_len] = &t_any;
++stack->ga_len;
return OK;
}
static int
generate_PCALL(cctx_T *cctx, int argcount, int at_top)
{
isn_T *isn;
garray_T *stack = &cctx->ctx_type_stack;
RETURN_OK_IF_SKIP(cctx);
if ((isn = generate_instr(cctx, ISN_PCALL)) == NULL)
return FAIL;
isn->isn_arg.pfunc.cpf_top = at_top;
isn->isn_arg.pfunc.cpf_argcount = argcount;
stack->ga_len -= argcount; 
((type_T **)stack->ga_data)[stack->ga_len - 1] = &t_any;
if (at_top && generate_instr(cctx, ISN_PCALL_END) == NULL)
return FAIL;
return OK;
}
static int
generate_MEMBER(cctx_T *cctx, char_u *name, size_t len)
{
isn_T *isn;
garray_T *stack = &cctx->ctx_type_stack;
type_T *type;
RETURN_OK_IF_SKIP(cctx);
if ((isn = generate_instr(cctx, ISN_MEMBER)) == NULL)
return FAIL;
isn->isn_arg.string = vim_strnsave(name, (int)len);
type = ((type_T **)stack->ga_data)[stack->ga_len - 1];
if (type->tt_type != VAR_DICT && type != &t_any)
{
emsg(_(e_dictreq));
return FAIL;
}
if (type->tt_type == VAR_DICT)
((type_T **)stack->ga_data)[stack->ga_len - 1] = type->tt_member;
return OK;
}
static int
generate_ECHO(cctx_T *cctx, int with_white, int count)
{
isn_T *isn;
RETURN_OK_IF_SKIP(cctx);
if ((isn = generate_instr_drop(cctx, ISN_ECHO, count)) == NULL)
return FAIL;
isn->isn_arg.echo.echo_with_white = with_white;
isn->isn_arg.echo.echo_count = count;
return OK;
}
static int
generate_EXECUTE(cctx_T *cctx, int count)
{
isn_T *isn;
if ((isn = generate_instr_drop(cctx, ISN_EXECUTE, count)) == NULL)
return FAIL;
isn->isn_arg.number = count;
return OK;
}
static int
generate_EXEC(cctx_T *cctx, char_u *line)
{
isn_T *isn;
RETURN_OK_IF_SKIP(cctx);
if ((isn = generate_instr(cctx, ISN_EXEC)) == NULL)
return FAIL;
isn->isn_arg.string = vim_strsave(line);
return OK;
}
static char e_white_both[] =
N_("E1004: white space required before and after '%s'");
static int
reserve_local(cctx_T *cctx, char_u *name, size_t len, int isConst, type_T *type)
{
int idx;
lvar_T *lvar;
if (lookup_arg(name, len, cctx) >= 0 || lookup_vararg(name, len, cctx))
{
emsg_namelen(_("E1006: %s is used as an argument"), name, (int)len);
return -1;
}
if (ga_grow(&cctx->ctx_locals, 1) == FAIL)
return -1;
idx = cctx->ctx_locals.ga_len;
if (cctx->ctx_max_local < idx + 1)
cctx->ctx_max_local = idx + 1;
++cctx->ctx_locals.ga_len;
lvar = ((lvar_T *)cctx->ctx_locals.ga_data) + idx;
lvar->lv_name = vim_strnsave(name, (int)(len == 0 ? STRLEN(name) : len));
lvar->lv_const = isConst;
lvar->lv_type = type;
return idx;
}
static void
unwind_locals(cctx_T *cctx, int new_top)
{
if (cctx->ctx_locals.ga_len > new_top)
{
int idx;
lvar_T *lvar;
for (idx = new_top; idx < cctx->ctx_locals.ga_len; ++idx)
{
lvar = ((lvar_T *)cctx->ctx_locals.ga_data) + idx;
vim_free(lvar->lv_name);
}
}
cctx->ctx_locals.ga_len = new_top;
}
static void
free_local(cctx_T *cctx)
{
unwind_locals(cctx, 0);
ga_clear(&cctx->ctx_locals);
}
char_u *
skip_type(char_u *start)
{
char_u *p = start;
while (ASCII_ISALNUM(*p) || *p == '_')
++p;
if (*skipwhite(p) == '<')
{
p = skipwhite(p);
p = skip_type(skipwhite(p + 1));
p = skipwhite(p);
if (*p == '>')
++p;
}
return p;
}
static type_T *
parse_type_member(char_u **arg, type_T *type, garray_T *type_list)
{
type_T *member_type;
int prev_called_emsg = called_emsg;
if (**arg != '<')
{
if (*skipwhite(*arg) == '<')
emsg(_("E1007: No white space allowed before <"));
else
emsg(_("E1008: Missing <type>"));
return type;
}
*arg = skipwhite(*arg + 1);
member_type = parse_type(arg, type_list);
*arg = skipwhite(*arg);
if (**arg != '>' && called_emsg == prev_called_emsg)
{
emsg(_("E1009: Missing > after type"));
return type;
}
++*arg;
if (type->tt_type == VAR_LIST)
return get_list_type(member_type, type_list);
return get_dict_type(member_type, type_list);
}
type_T *
parse_type(char_u **arg, garray_T *type_list)
{
char_u *p = *arg;
size_t len;
while (ASCII_ISALNUM(*p) || *p == '_')
++p;
len = p - *arg;
switch (**arg)
{
case 'a':
if (len == 3 && STRNCMP(*arg, "any", len) == 0)
{
*arg += len;
return &t_any;
}
break;
case 'b':
if (len == 4 && STRNCMP(*arg, "bool", len) == 0)
{
*arg += len;
return &t_bool;
}
if (len == 4 && STRNCMP(*arg, "blob", len) == 0)
{
*arg += len;
return &t_blob;
}
break;
case 'c':
if (len == 7 && STRNCMP(*arg, "channel", len) == 0)
{
*arg += len;
return &t_channel;
}
break;
case 'd':
if (len == 4 && STRNCMP(*arg, "dict", len) == 0)
{
*arg += len;
return parse_type_member(arg, &t_dict_any, type_list);
}
break;
case 'f':
if (len == 5 && STRNCMP(*arg, "float", len) == 0)
{
#if defined(FEAT_FLOAT)
*arg += len;
return &t_float;
#else
emsg(_("E1055: This Vim is not compiled with float support"));
return &t_any;
#endif
}
if (len == 4 && STRNCMP(*arg, "func", len) == 0)
{
*arg += len;
return &t_func_any;
}
break;
case 'j':
if (len == 3 && STRNCMP(*arg, "job", len) == 0)
{
*arg += len;
return &t_job;
}
break;
case 'l':
if (len == 4 && STRNCMP(*arg, "list", len) == 0)
{
*arg += len;
return parse_type_member(arg, &t_list_any, type_list);
}
break;
case 'n':
if (len == 6 && STRNCMP(*arg, "number", len) == 0)
{
*arg += len;
return &t_number;
}
break;
case 'p':
if (len == 7 && STRNCMP(*arg, "partial", len) == 0)
{
*arg += len;
return &t_partial_any;
}
break;
case 's':
if (len == 6 && STRNCMP(*arg, "string", len) == 0)
{
*arg += len;
return &t_string;
}
break;
case 'v':
if (len == 4 && STRNCMP(*arg, "void", len) == 0)
{
*arg += len;
return &t_void;
}
break;
}
semsg(_("E1010: Type not recognized: %s"), *arg);
return &t_any;
}
static int
equal_type(type_T *type1, type_T *type2)
{
if (type1->tt_type != type2->tt_type)
return FALSE;
switch (type1->tt_type)
{
case VAR_VOID:
case VAR_UNKNOWN:
case VAR_SPECIAL:
case VAR_BOOL:
case VAR_NUMBER:
case VAR_FLOAT:
case VAR_STRING:
case VAR_BLOB:
case VAR_JOB:
case VAR_CHANNEL:
break; 
case VAR_LIST:
case VAR_DICT:
return equal_type(type1->tt_member, type2->tt_member);
case VAR_FUNC:
case VAR_PARTIAL:
return equal_type(type1->tt_member, type2->tt_member)
&& type1->tt_argcount == type2->tt_argcount;
}
return TRUE;
}
static void
common_type(type_T *type1, type_T *type2, type_T **dest, garray_T *type_list)
{
if (equal_type(type1, type2))
{
*dest = type1;
return;
}
if (type1->tt_type == type2->tt_type)
{
if (type1->tt_type == VAR_LIST || type2->tt_type == VAR_DICT)
{
type_T *common;
common_type(type1->tt_member, type2->tt_member, &common, type_list);
if (type1->tt_type == VAR_LIST)
*dest = get_list_type(common, type_list);
else
*dest = get_dict_type(common, type_list);
return;
}
*dest = type1;
}
*dest = &t_any;
}
char *
vartype_name(vartype_T type)
{
switch (type)
{
case VAR_UNKNOWN: break;
case VAR_VOID: return "void";
case VAR_SPECIAL: return "special";
case VAR_BOOL: return "bool";
case VAR_NUMBER: return "number";
case VAR_FLOAT: return "float";
case VAR_STRING: return "string";
case VAR_BLOB: return "blob";
case VAR_JOB: return "job";
case VAR_CHANNEL: return "channel";
case VAR_LIST: return "list";
case VAR_DICT: return "dict";
case VAR_FUNC: return "func";
case VAR_PARTIAL: return "partial";
}
return "any";
}
char *
type_name(type_T *type, char **tofree)
{
char *name = vartype_name(type->tt_type);
*tofree = NULL;
if (type->tt_type == VAR_LIST || type->tt_type == VAR_DICT)
{
char *member_free;
char *member_name = type_name(type->tt_member, &member_free);
size_t len;
len = STRLEN(name) + STRLEN(member_name) + 3;
*tofree = alloc(len);
if (*tofree != NULL)
{
vim_snprintf(*tofree, len, "%s<%s>", name, member_name);
vim_free(member_free);
return *tofree;
}
}
return name;
}
int
get_script_item_idx(int sid, char_u *name, int check_writable)
{
hashtab_T *ht;
dictitem_T *di;
scriptitem_T *si = SCRIPT_ITEM(sid);
int idx;
if (sid <= 0 || sid > script_items.ga_len)
return -1;
ht = &SCRIPT_VARS(sid);
di = find_var_in_ht(ht, 0, name, TRUE);
if (di == NULL)
return -2;
for (idx = 0; idx < si->sn_var_vals.ga_len; ++idx)
{
svar_T *sv = ((svar_T *)si->sn_var_vals.ga_data) + idx;
if (sv->sv_tv == &di->di_tv)
{
if (check_writable && sv->sv_const)
semsg(_(e_readonlyvar), name);
return idx;
}
}
return -1;
}
imported_T *
find_imported(char_u *name, size_t len, cctx_T *cctx)
{
scriptitem_T *si = SCRIPT_ITEM(current_sctx.sc_sid);
int idx;
if (cctx != NULL)
for (idx = 0; idx < cctx->ctx_imports.ga_len; ++idx)
{
imported_T *import = ((imported_T *)cctx->ctx_imports.ga_data)
+ idx;
if (len == 0 ? STRCMP(name, import->imp_name) == 0
: STRLEN(import->imp_name) == len
&& STRNCMP(name, import->imp_name, len) == 0)
return import;
}
for (idx = 0; idx < si->sn_imports.ga_len; ++idx)
{
imported_T *import = ((imported_T *)si->sn_imports.ga_data) + idx;
if (len == 0 ? STRCMP(name, import->imp_name) == 0
: STRLEN(import->imp_name) == len
&& STRNCMP(name, import->imp_name, len) == 0)
return import;
}
return NULL;
}
static void
free_imported(cctx_T *cctx)
{
int idx;
for (idx = 0; idx < cctx->ctx_imports.ga_len; ++idx)
{
imported_T *import = ((imported_T *)cctx->ctx_imports.ga_data) + idx;
vim_free(import->imp_name);
}
ga_clear(&cctx->ctx_imports);
}
static int
compile_load_scriptvar(
cctx_T *cctx,
char_u *name, 
char_u *start, 
char_u **end, 
int error) 
{
scriptitem_T *si = SCRIPT_ITEM(current_sctx.sc_sid);
int idx = get_script_item_idx(current_sctx.sc_sid, name, FALSE);
imported_T *import;
if (idx == -1 || si->sn_version != SCRIPT_VERSION_VIM9)
{
return generate_OLDSCRIPT(cctx, ISN_LOADS, name, current_sctx.sc_sid,
&t_any);
}
if (idx >= 0)
{
svar_T *sv = ((svar_T *)si->sn_var_vals.ga_data) + idx;
generate_VIM9SCRIPT(cctx, ISN_LOADSCRIPT,
current_sctx.sc_sid, idx, sv->sv_type);
return OK;
}
import = find_imported(name, 0, cctx);
if (import != NULL)
{
if (import->imp_all)
{
char_u *p = skipwhite(*end);
int name_len;
ufunc_T *ufunc;
type_T *type;
if (*p != '.')
{
semsg(_("E1060: expected dot after name: %s"), start);
return FAIL;
}
++p;
if (VIM_ISWHITE(*p))
{
emsg(_("E1074: no white space allowed after dot"));
return FAIL;
}
idx = find_exported(import->imp_sid, &p, &name_len, &ufunc, &type);
if (idx < 0)
return FAIL;
*end = p;
generate_VIM9SCRIPT(cctx, ISN_LOADSCRIPT,
import->imp_sid,
idx,
type);
}
else
{
generate_VIM9SCRIPT(cctx, ISN_LOADSCRIPT,
import->imp_sid,
import->imp_var_vals_idx,
import->imp_type);
}
return OK;
}
if (error)
semsg(_("E1050: Item not found: %s"), name);
return FAIL;
}
static int
compile_load(char_u **arg, char_u *end_arg, cctx_T *cctx, int error)
{
type_T *type;
char_u *name;
char_u *end = end_arg;
int res = FAIL;
int prev_called_emsg = called_emsg;
if (*(*arg + 1) == ':')
{
if (end <= *arg + 2)
name = vim_strsave((char_u *)"[empty]");
else
name = vim_strnsave(*arg + 2, end - (*arg + 2));
if (name == NULL)
return FAIL;
if (**arg == 'v')
{
res = generate_LOADV(cctx, name, error);
}
else if (**arg == 'g')
{
res = generate_LOAD(cctx, ISN_LOADG, 0, name, &t_any);
}
else if (**arg == 's')
{
res = compile_load_scriptvar(cctx, name, NULL, NULL, error);
}
else if (**arg == 'b')
{
semsg("Namespace b: not supported yet: %s", *arg);
goto theend;
}
else if (**arg == 'w')
{
semsg("Namespace w: not supported yet: %s", *arg);
goto theend;
}
else if (**arg == 't')
{
semsg("Namespace t: not supported yet: %s", *arg);
goto theend;
}
else
{
semsg("E1075: Namespace not supported: %s", *arg);
goto theend;
}
}
else
{
size_t len = end - *arg;
int idx;
int gen_load = FALSE;
name = vim_strnsave(*arg, end - *arg);
if (name == NULL)
return FAIL;
idx = lookup_arg(*arg, len, cctx);
if (idx >= 0)
{
if (cctx->ctx_ufunc->uf_arg_types != NULL)
type = cctx->ctx_ufunc->uf_arg_types[idx];
else
type = &t_any;
idx -= cctx->ctx_ufunc->uf_args.ga_len + STACK_FRAME_SIZE;
if (cctx->ctx_ufunc->uf_va_name != NULL)
--idx;
gen_load = TRUE;
}
else if (lookup_vararg(*arg, len, cctx))
{
idx = -STACK_FRAME_SIZE - 1;
type = cctx->ctx_ufunc->uf_va_type;
gen_load = TRUE;
}
else
{
idx = lookup_local(*arg, len, cctx);
if (idx >= 0)
{
type = (((lvar_T *)cctx->ctx_locals.ga_data) + idx)->lv_type;
gen_load = TRUE;
}
else
{
if ((len == 4 && STRNCMP("true", *arg, 4) == 0)
|| (len == 5 && STRNCMP("false", *arg, 5) == 0))
res = generate_PUSHBOOL(cctx, **arg == 't'
? VVAL_TRUE : VVAL_FALSE);
else if (SCRIPT_ITEM(current_sctx.sc_sid)->sn_version
== SCRIPT_VERSION_VIM9)
res = compile_load_scriptvar(cctx, name, *arg, &end, error);
}
}
if (gen_load)
res = generate_LOAD(cctx, ISN_LOAD, idx, NULL, type);
}
*arg = end;
theend:
if (res == FAIL && error && called_emsg == prev_called_emsg)
semsg(_(e_var_notfound), name);
vim_free(name);
return res;
}
static int
compile_arguments(char_u **arg, cctx_T *cctx, int *argcount)
{
char_u *p = *arg;
while (*p != NUL && *p != ')')
{
if (compile_expr1(&p, cctx) == FAIL)
return FAIL;
++*argcount;
if (*p != ',' && *skipwhite(p) == ',')
{
emsg(_("E1068: No white space allowed before ,"));
p = skipwhite(p);
}
if (*p == ',')
{
++p;
if (!VIM_ISWHITE(*p))
emsg(_("E1069: white space required after ,"));
}
p = skipwhite(p);
}
p = skipwhite(p);
if (*p != ')')
{
emsg(_(e_missing_close));
return FAIL;
}
*arg = p + 1;
return OK;
}
static int
compile_call(char_u **arg, size_t varlen, cctx_T *cctx, int argcount_init)
{
char_u *name = *arg;
char_u *p;
int argcount = argcount_init;
char_u namebuf[100];
char_u fname_buf[FLEN_FIXED + 1];
char_u *tofree = NULL;
int error = FCERR_NONE;
ufunc_T *ufunc;
int res = FAIL;
if (varlen >= sizeof(namebuf))
{
semsg(_("E1011: name too long: %s"), name);
return FAIL;
}
vim_strncpy(namebuf, *arg, varlen);
name = fname_trans_sid(namebuf, fname_buf, &tofree, &error);
*arg = skipwhite(*arg + varlen + 1);
if (compile_arguments(arg, cctx, &argcount) == FAIL)
goto theend;
if (ASCII_ISLOWER(*name) && name[1] != ':')
{
int idx;
idx = find_internal_func(name);
if (idx >= 0)
{
res = generate_BCALL(cctx, idx, argcount);
goto theend;
}
semsg(_(e_unknownfunc), namebuf);
}
ufunc = find_func(name, cctx);
if (ufunc != NULL)
{
res = generate_CALL(cctx, ufunc, argcount);
goto theend;
}
p = namebuf;
if (compile_load(&p, namebuf + varlen, cctx, FALSE) == OK)
{
res = generate_PCALL(cctx, argcount, FALSE);
goto theend;
}
res = generate_UCALL(cctx, name, argcount);
theend:
vim_free(tofree);
return res;
}
#define VIM9_NAMESPACE_CHAR (char_u *)"bgstvw"
static char_u *
to_name_end(char_u *arg, int namespace)
{
char_u *p;
if (!eval_isnamec1(*arg))
return arg;
for (p = arg + 1; *p != NUL && eval_isnamec(*p); MB_PTR_ADV(p))
if (*p == ':' && (p != arg + 1
|| !namespace
|| vim_strchr(VIM9_NAMESPACE_CHAR, *arg) == NULL))
break;
return p;
}
char_u *
to_name_const_end(char_u *arg)
{
char_u *p = to_name_end(arg, TRUE);
typval_T rettv;
if (p == arg && *arg == '[')
{
if (get_list_tv(&p, &rettv, FALSE, FALSE) == FAIL)
p = arg;
}
else if (p == arg && *arg == '#' && arg[1] == '{')
{
++p;
if (eval_dict(&p, &rettv, FALSE, TRUE) == FAIL)
p = arg;
}
else if (p == arg && *arg == '{')
{
int ret = get_lambda_tv(&p, &rettv, FALSE);
if (ret == NOTDONE)
ret = eval_dict(&p, &rettv, FALSE, FALSE);
if (ret != OK)
p = arg;
}
return p;
}
static void
type_mismatch(type_T *expected, type_T *actual)
{
char *tofree1, *tofree2;
semsg(_("E1013: type mismatch, expected %s but got %s"),
type_name(expected, &tofree1), type_name(actual, &tofree2));
vim_free(tofree1);
vim_free(tofree2);
}
static int
check_type(type_T *expected, type_T *actual, int give_msg)
{
if (expected->tt_type != VAR_UNKNOWN)
{
if (expected->tt_type != actual->tt_type)
{
if (give_msg)
type_mismatch(expected, actual);
return FAIL;
}
if (expected->tt_type == VAR_DICT || expected->tt_type == VAR_LIST)
{
int ret;
if (actual->tt_member == &t_void)
ret = OK;
else
ret = check_type(expected->tt_member, actual->tt_member, FALSE);
if (ret == FAIL && give_msg)
type_mismatch(expected, actual);
return ret;
}
}
return OK;
}
static int
need_type(type_T *actual, type_T *expected, int offset, cctx_T *cctx)
{
if (check_type(expected, actual, FALSE))
return OK;
if (actual->tt_type != VAR_UNKNOWN)
{
type_mismatch(expected, actual);
return FAIL;
}
generate_TYPECHECK(cctx, expected, offset);
return OK;
}
static int
compile_list(char_u **arg, cctx_T *cctx)
{
char_u *p = skipwhite(*arg + 1);
int count = 0;
while (*p != ']')
{
if (*p == NUL)
{
semsg(_(e_list_end), *arg);
return FAIL;
}
if (compile_expr1(&p, cctx) == FAIL)
break;
++count;
if (*p == ',')
++p;
p = skipwhite(p);
}
*arg = p + 1;
generate_NEWLIST(cctx, count);
return OK;
}
static int
compile_lambda(char_u **arg, cctx_T *cctx)
{
garray_T *instr = &cctx->ctx_instr;
typval_T rettv;
ufunc_T *ufunc;
if (get_lambda_tv(arg, &rettv, TRUE) != OK)
return FAIL;
ufunc = rettv.vval.v_partial->pt_func;
++ufunc->uf_refcount;
clear_tv(&rettv);
compile_def_function(ufunc, TRUE);
if (ufunc->uf_dfunc_idx >= 0)
{
if (ga_grow(instr, 1) == FAIL)
return FAIL;
generate_FUNCREF(cctx, ufunc->uf_dfunc_idx);
return OK;
}
return FAIL;
}
static int
compile_lambda_call(char_u **arg, cctx_T *cctx)
{
ufunc_T *ufunc;
typval_T rettv;
int argcount = 1;
int ret = FAIL;
if (get_lambda_tv(arg, &rettv, TRUE) == FAIL)
return FAIL;
if (**arg != '(')
{
if (*skipwhite(*arg) == '(')
emsg(_(e_nowhitespace));
else
semsg(_(e_missing_paren), "lambda");
clear_tv(&rettv);
return FAIL;
}
ufunc = rettv.vval.v_partial->pt_func;
++ufunc->uf_refcount;
clear_tv(&rettv);
compile_def_function(ufunc, TRUE);
*arg = skipwhite(*arg + 1);
if (compile_arguments(arg, cctx, &argcount) == OK)
ret = generate_CALL(cctx, ufunc, argcount);
return ret;
}
static int
compile_dict(char_u **arg, cctx_T *cctx, int literal)
{
garray_T *instr = &cctx->ctx_instr;
int count = 0;
dict_T *d = dict_alloc();
dictitem_T *item;
if (d == NULL)
return FAIL;
*arg = skipwhite(*arg + 1);
while (**arg != '}' && **arg != NUL)
{
char_u *key = NULL;
if (literal)
{
char_u *p = to_name_end(*arg, !literal);
if (p == *arg)
{
semsg(_("E1014: Invalid key: %s"), *arg);
return FAIL;
}
key = vim_strnsave(*arg, p - *arg);
if (generate_PUSHS(cctx, key) == FAIL)
return FAIL;
*arg = p;
}
else
{
isn_T *isn;
if (compile_expr1(arg, cctx) == FAIL)
return FAIL;
isn = ((isn_T *)instr->ga_data) + instr->ga_len - 1;
if (isn->isn_type == ISN_PUSHS)
key = isn->isn_arg.string;
}
if (key != NULL)
{
item = dict_find(d, key, -1);
if (item != NULL)
{
semsg(_(e_duplicate_key), key);
goto failret;
}
item = dictitem_alloc(key);
if (item != NULL)
{
item->di_tv.v_type = VAR_UNKNOWN;
item->di_tv.v_lock = 0;
if (dict_add(d, item) == FAIL)
dictitem_free(item);
}
}
*arg = skipwhite(*arg);
if (**arg != ':')
{
semsg(_(e_missing_dict_colon), *arg);
return FAIL;
}
*arg = skipwhite(*arg + 1);
if (compile_expr1(arg, cctx) == FAIL)
return FAIL;
++count;
if (**arg == '}')
break;
if (**arg != ',')
{
semsg(_(e_missing_dict_comma), *arg);
goto failret;
}
*arg = skipwhite(*arg + 1);
}
if (**arg != '}')
{
semsg(_(e_missing_dict_end), *arg);
goto failret;
}
*arg = *arg + 1;
dict_unref(d);
return generate_NEWDICT(cctx, count);
failret:
dict_unref(d);
return FAIL;
}
static int
compile_get_option(char_u **arg, cctx_T *cctx)
{
typval_T rettv;
char_u *start = *arg;
int ret;
rettv.v_type = VAR_UNKNOWN;
ret = get_option_tv(arg, &rettv, TRUE);
if (ret == OK)
{
char_u *name = vim_strnsave(start, *arg - start);
type_T *type = rettv.v_type == VAR_NUMBER ? &t_number : &t_string;
ret = generate_LOAD(cctx, ISN_LOADOPT, 0, name, type);
vim_free(name);
}
clear_tv(&rettv);
return ret;
}
static int
compile_get_env(char_u **arg, cctx_T *cctx)
{
char_u *start = *arg;
int len;
int ret;
char_u *name;
++*arg;
len = get_env_len(arg);
if (len == 0)
{
semsg(_(e_syntax_at), start - 1);
return FAIL;
}
name = vim_strnsave(start, len + 1);
ret = generate_LOAD(cctx, ISN_LOADENV, 0, name, &t_string);
vim_free(name);
return ret;
}
static int
compile_get_register(char_u **arg, cctx_T *cctx)
{
int ret;
++*arg;
if (**arg == NUL)
{
semsg(_(e_syntax_at), *arg - 1);
return FAIL;
}
if (!valid_yank_reg(**arg, TRUE))
{
emsg_invreg(**arg);
return FAIL;
}
ret = generate_LOAD(cctx, ISN_LOADREG, **arg, NULL, &t_string);
++*arg;
return ret;
}
static int
apply_leader(typval_T *rettv, char_u *start, char_u *end)
{
char_u *p = end;
while (p > start)
{
--p;
if (*p == '-' || *p == '+')
{
#if defined(FEAT_FLOAT)
if (rettv->v_type == VAR_FLOAT)
{
if (*p == '-')
rettv->vval.v_float = -rettv->vval.v_float;
}
else
#endif
{
varnumber_T val;
int error = FALSE;
if (check_not_string(rettv) == FAIL)
return FAIL;
val = tv_get_number_chk(rettv, &error);
clear_tv(rettv);
if (error)
return FAIL;
if (*p == '-')
val = -val;
rettv->v_type = VAR_NUMBER;
rettv->vval.v_number = val;
}
}
else
{
int v = tv2bool(rettv);
clear_tv(rettv);
rettv->v_type = VAR_BOOL;
rettv->vval.v_number = v ? VVAL_FALSE : VVAL_TRUE;
}
}
return OK;
}
static void
get_vim_constant(char_u **arg, typval_T *rettv)
{
if (STRNCMP(*arg, "v:true", 6) == 0)
{
rettv->v_type = VAR_BOOL;
rettv->vval.v_number = VVAL_TRUE;
*arg += 6;
}
else if (STRNCMP(*arg, "v:false", 7) == 0)
{
rettv->v_type = VAR_BOOL;
rettv->vval.v_number = VVAL_FALSE;
*arg += 7;
}
else if (STRNCMP(*arg, "v:null", 6) == 0)
{
rettv->v_type = VAR_SPECIAL;
rettv->vval.v_number = VVAL_NULL;
*arg += 6;
}
else if (STRNCMP(*arg, "v:none", 6) == 0)
{
rettv->v_type = VAR_SPECIAL;
rettv->vval.v_number = VVAL_NONE;
*arg += 6;
}
}
static int
compile_leader(cctx_T *cctx, char_u *start, char_u *end)
{
char_u *p = end;
while (p > start)
{
--p;
if (*p == '-' || *p == '+')
{
int negate = *p == '-';
isn_T *isn;
while (p > start && (p[-1] == '-' || p[-1] == '+'))
{
--p;
if (*p == '-')
negate = !negate;
}
if (negate)
isn = generate_instr(cctx, ISN_NEGATENR);
else
isn = generate_instr(cctx, ISN_CHECKNR);
if (isn == NULL)
return FAIL;
}
else
{
int invert = TRUE;
while (p > start && p[-1] == '!')
{
--p;
invert = !invert;
}
if (generate_2BOOL(cctx, invert) == FAIL)
return FAIL;
}
}
return OK;
}
static int
compile_subscript(
char_u **arg,
cctx_T *cctx,
char_u **start_leader,
char_u *end_leader)
{
for (;;)
{
if (**arg == '(')
{
int argcount = 0;
*arg = skipwhite(*arg + 1);
if (compile_arguments(arg, cctx, &argcount) == FAIL)
return FAIL;
if (generate_PCALL(cctx, argcount, TRUE) == FAIL)
return FAIL;
}
else if (**arg == '-' && (*arg)[1] == '>')
{
char_u *p;
if (compile_leader(cctx, *start_leader, end_leader) == FAIL)
return FAIL;
*start_leader = end_leader; 
*arg = skipwhite(*arg + 2);
if (**arg == '{')
{
if (compile_lambda_call(arg, cctx) == FAIL)
return FAIL;
}
else
{
p = *arg;
if (ASCII_ISALPHA(*p) && p[1] == ':')
p += 2;
for ( ; eval_isnamec1(*p); ++p)
;
if (*p != '(')
{
semsg(_(e_missing_paren), *arg);
return FAIL;
}
if (compile_call(arg, p - *arg, cctx, 1) == FAIL)
return FAIL;
}
}
else if (**arg == '[')
{
garray_T *stack;
type_T **typep;
*arg = skipwhite(*arg + 1);
if (compile_expr1(arg, cctx) == FAIL)
return FAIL;
if (**arg != ']')
{
emsg(_(e_missbrac));
return FAIL;
}
*arg = *arg + 1;
if (generate_instr_drop(cctx, ISN_INDEX, 1) == FAIL)
return FAIL;
stack = &cctx->ctx_type_stack;
typep = ((type_T **)stack->ga_data) + stack->ga_len - 1;
if ((*typep)->tt_type != VAR_LIST && *typep != &t_any)
{
emsg(_(e_listreq));
return FAIL;
}
if ((*typep)->tt_type == VAR_LIST)
*typep = (*typep)->tt_member;
}
else if (**arg == '.' && (*arg)[1] != '.')
{
char_u *p;
++*arg;
p = *arg;
if (eval_isnamec1(*p))
while (eval_isnamec(*p))
MB_PTR_ADV(p);
if (p == *arg)
{
semsg(_(e_syntax_at), *arg);
return FAIL;
}
if (generate_MEMBER(cctx, *arg, p - *arg) == FAIL)
return FAIL;
*arg = p;
}
else
break;
}
return OK;
}
static int
compile_expr7(char_u **arg, cctx_T *cctx)
{
typval_T rettv;
char_u *start_leader, *end_leader;
int ret = OK;
start_leader = *arg;
while (**arg == '!' || **arg == '-' || **arg == '+')
*arg = skipwhite(*arg + 1);
end_leader = *arg;
rettv.v_type = VAR_UNKNOWN;
switch (**arg)
{
case '0': 
case '1':
case '2':
case '3':
case '4':
case '5':
case '6':
case '7':
case '8':
case '9':
case '.': if (get_number_tv(arg, &rettv, TRUE, FALSE) == FAIL)
return FAIL;
break;
case '"': if (get_string_tv(arg, &rettv, TRUE) == FAIL)
return FAIL;
break;
case '\'': if (get_lit_string_tv(arg, &rettv, TRUE) == FAIL)
return FAIL;
break;
case 'v': get_vim_constant(arg, &rettv);
ret = NOTDONE;
break;
case '[': ret = compile_list(arg, cctx);
break;
case '#': if ((*arg)[1] == '{')
{
++*arg;
ret = compile_dict(arg, cctx, TRUE);
}
else
ret = NOTDONE;
break;
case '{': {
char_u *start = skipwhite(*arg + 1);
ret = get_function_args(&start, '-', NULL,
NULL, NULL, NULL, TRUE);
if (ret != FAIL && *start == '>')
ret = compile_lambda(arg, cctx);
else
ret = compile_dict(arg, cctx, FALSE);
}
break;
case '&': ret = compile_get_option(arg, cctx);
break;
case '$': ret = compile_get_env(arg, cctx);
break;
case '@': ret = compile_get_register(arg, cctx);
break;
case '(': *arg = skipwhite(*arg + 1);
ret = compile_expr1(arg, cctx); 
*arg = skipwhite(*arg);
if (**arg == ')')
++*arg;
else if (ret == OK)
{
emsg(_(e_missing_close));
ret = FAIL;
}
break;
default: ret = NOTDONE;
break;
}
if (ret == FAIL)
return FAIL;
if (rettv.v_type != VAR_UNKNOWN)
{
if (apply_leader(&rettv, start_leader, end_leader) == FAIL)
{
clear_tv(&rettv);
return FAIL;
}
start_leader = end_leader; 
switch (rettv.v_type)
{
case VAR_BOOL:
generate_PUSHBOOL(cctx, rettv.vval.v_number);
break;
case VAR_SPECIAL:
generate_PUSHSPEC(cctx, rettv.vval.v_number);
break;
case VAR_NUMBER:
generate_PUSHNR(cctx, rettv.vval.v_number);
break;
#if defined(FEAT_FLOAT)
case VAR_FLOAT:
generate_PUSHF(cctx, rettv.vval.v_float);
break;
#endif
case VAR_BLOB:
generate_PUSHBLOB(cctx, rettv.vval.v_blob);
rettv.vval.v_blob = NULL;
break;
case VAR_STRING:
generate_PUSHS(cctx, rettv.vval.v_string);
rettv.vval.v_string = NULL;
break;
default:
iemsg("constant type missing");
return FAIL;
}
}
else if (ret == NOTDONE)
{
char_u *p;
int r;
if (!eval_isnamec1(**arg))
{
semsg(_("E1015: Name expected: %s"), *arg);
return FAIL;
}
p = to_name_end(*arg, TRUE);
if (*p == '(')
r = compile_call(arg, p - *arg, cctx, 0);
else
r = compile_load(arg, p, cctx, TRUE);
if (r == FAIL)
return FAIL;
}
if (compile_subscript(arg, cctx, &start_leader, end_leader) == FAIL)
return FAIL;
return compile_leader(cctx, start_leader, end_leader);
}
static int
compile_expr6(char_u **arg, cctx_T *cctx)
{
char_u *op;
if (compile_expr7(arg, cctx) == FAIL)
return FAIL;
for (;;)
{
op = skipwhite(*arg);
if (*op != '*' && *op != '/' && *op != '%')
break;
if (!VIM_ISWHITE(**arg) || !VIM_ISWHITE(op[1]))
{
char_u buf[3];
vim_strncpy(buf, op, 1);
semsg(_(e_white_both), buf);
}
*arg = skipwhite(op + 1);
if (compile_expr7(arg, cctx) == FAIL)
return FAIL;
generate_two_op(cctx, op);
}
return OK;
}
static int
compile_expr5(char_u **arg, cctx_T *cctx)
{
char_u *op;
int oplen;
if (compile_expr6(arg, cctx) == FAIL)
return FAIL;
for (;;)
{
op = skipwhite(*arg);
if (*op != '+' && *op != '-' && !(*op == '.' && (*(*arg + 1) == '.')))
break;
oplen = (*op == '.' ? 2 : 1);
if (!VIM_ISWHITE(**arg) || !VIM_ISWHITE(op[oplen]))
{
char_u buf[3];
vim_strncpy(buf, op, oplen);
semsg(_(e_white_both), buf);
}
*arg = skipwhite(op + oplen);
if (compile_expr6(arg, cctx) == FAIL)
return FAIL;
if (*op == '.')
{
if (may_generate_2STRING(-2, cctx) == FAIL
|| may_generate_2STRING(-1, cctx) == FAIL)
return FAIL;
generate_instr_drop(cctx, ISN_CONCAT, 1);
}
else
generate_two_op(cctx, op);
}
return OK;
}
static exptype_T
get_compare_type(char_u *p, int *len, int *type_is)
{
exptype_T type = EXPR_UNKNOWN;
int i;
switch (p[0])
{
case '=': if (p[1] == '=')
type = EXPR_EQUAL;
else if (p[1] == '~')
type = EXPR_MATCH;
break;
case '!': if (p[1] == '=')
type = EXPR_NEQUAL;
else if (p[1] == '~')
type = EXPR_NOMATCH;
break;
case '>': if (p[1] != '=')
{
type = EXPR_GREATER;
*len = 1;
}
else
type = EXPR_GEQUAL;
break;
case '<': if (p[1] != '=')
{
type = EXPR_SMALLER;
*len = 1;
}
else
type = EXPR_SEQUAL;
break;
case 'i': if (p[1] == 's')
{
if (p[2] == 'n' && p[3] == 'o' && p[4] == 't')
*len = 5;
i = p[*len];
if (!isalnum(i) && i != '_')
{
type = *len == 2 ? EXPR_IS : EXPR_ISNOT;
*type_is = TRUE;
}
}
break;
}
return type;
}
static int
compile_expr4(char_u **arg, cctx_T *cctx)
{
exptype_T type = EXPR_UNKNOWN;
char_u *p;
int len = 2;
int type_is = FALSE;
if (compile_expr5(arg, cctx) == FAIL)
return FAIL;
p = skipwhite(*arg);
type = get_compare_type(p, &len, &type_is);
if (type != EXPR_UNKNOWN)
{
int ic = FALSE; 
if (type_is && (p[len] == '?' || p[len] == '#'))
{
semsg(_(e_invexpr2), *arg);
return FAIL;
}
if (p[len] == '?')
{
ic = TRUE;
++len;
}
else if (p[len] == '#')
++len;
if (!VIM_ISWHITE(**arg) || !VIM_ISWHITE(p[len]))
{
char_u buf[7];
vim_strncpy(buf, p, len);
semsg(_(e_white_both), buf);
}
*arg = skipwhite(p + len);
if (compile_expr5(arg, cctx) == FAIL)
return FAIL;
generate_COMPARE(cctx, type, ic);
}
return OK;
}
static int
compile_and_or(char_u **arg, cctx_T *cctx, char *op)
{
char_u *p = skipwhite(*arg);
int opchar = *op;
if (p[0] == opchar && p[1] == opchar)
{
garray_T *instr = &cctx->ctx_instr;
garray_T end_ga;
ga_init2(&end_ga, sizeof(int), 10);
while (p[0] == opchar && p[1] == opchar)
{
if (!VIM_ISWHITE(**arg) || !VIM_ISWHITE(p[2]))
semsg(_(e_white_both), op);
if (ga_grow(&end_ga, 1) == FAIL)
{
ga_clear(&end_ga);
return FAIL;
}
*(((int *)end_ga.ga_data) + end_ga.ga_len) = instr->ga_len;
++end_ga.ga_len;
generate_JUMP(cctx, opchar == '|'
? JUMP_AND_KEEP_IF_TRUE : JUMP_AND_KEEP_IF_FALSE, 0);
*arg = skipwhite(p + 2);
if ((opchar == '|' ? compile_expr3(arg, cctx)
: compile_expr4(arg, cctx)) == FAIL)
{
ga_clear(&end_ga);
return FAIL;
}
p = skipwhite(*arg);
}
while (end_ga.ga_len > 0)
{
isn_T *isn;
--end_ga.ga_len;
isn = ((isn_T *)instr->ga_data)
+ *(((int *)end_ga.ga_data) + end_ga.ga_len);
isn->isn_arg.jump.jump_where = instr->ga_len;
}
ga_clear(&end_ga);
}
return OK;
}
static int
compile_expr3(char_u **arg, cctx_T *cctx)
{
if (compile_expr4(arg, cctx) == FAIL)
return FAIL;
return compile_and_or(arg, cctx, "&&");
}
static int
compile_expr2(char_u **arg, cctx_T *cctx)
{
if (compile_expr3(arg, cctx) == FAIL)
return FAIL;
return compile_and_or(arg, cctx, "||");
}
static int
compile_expr1(char_u **arg, cctx_T *cctx)
{
char_u *p;
if (compile_expr2(arg, cctx) == FAIL)
return FAIL;
p = skipwhite(*arg);
if (*p == '?')
{
garray_T *instr = &cctx->ctx_instr;
garray_T *stack = &cctx->ctx_type_stack;
int alt_idx = instr->ga_len;
int end_idx;
isn_T *isn;
type_T *type1;
type_T *type2;
if (!VIM_ISWHITE(**arg) || !VIM_ISWHITE(p[1]))
semsg(_(e_white_both), "?");
generate_JUMP(cctx, JUMP_IF_FALSE, 0);
*arg = skipwhite(p + 1);
if (compile_expr1(arg, cctx) == FAIL)
return FAIL;
--stack->ga_len;
type1 = ((type_T **)stack->ga_data)[stack->ga_len];
end_idx = instr->ga_len;
generate_JUMP(cctx, JUMP_ALWAYS, 0);
isn = ((isn_T *)instr->ga_data) + alt_idx;
isn->isn_arg.jump.jump_where = instr->ga_len;
p = skipwhite(*arg);
if (*p != ':')
{
emsg(_(e_missing_colon));
return FAIL;
}
if (!VIM_ISWHITE(**arg) || !VIM_ISWHITE(p[1]))
semsg(_(e_white_both), ":");
*arg = skipwhite(p + 1);
if (compile_expr1(arg, cctx) == FAIL)
return FAIL;
type2 = ((type_T **)stack->ga_data)[stack->ga_len - 1];
common_type(type1, type2, &type2, cctx->ctx_type_list);
isn = ((isn_T *)instr->ga_data) + end_idx;
isn->isn_arg.jump.jump_where = instr->ga_len;
}
return OK;
}
static char_u *
compile_return(char_u *arg, int set_return_type, cctx_T *cctx)
{
char_u *p = arg;
garray_T *stack = &cctx->ctx_type_stack;
type_T *stack_type;
if (*p != NUL && *p != '|' && *p != '\n')
{
if (compile_expr1(&p, cctx) == FAIL)
return NULL;
stack_type = ((type_T **)stack->ga_data)[stack->ga_len - 1];
if (set_return_type)
cctx->ctx_ufunc->uf_ret_type = stack_type;
else if (need_type(stack_type, cctx->ctx_ufunc->uf_ret_type, -1, cctx)
== FAIL)
return NULL;
}
else
{
if (cctx->ctx_ufunc->uf_ret_type->tt_type != VAR_VOID)
{
emsg(_("E1003: Missing return value"));
return NULL;
}
generate_PUSHNR(cctx, 0);
}
if (generate_instr(cctx, ISN_RETURN) == NULL)
return NULL;
return skipwhite(p);
}
int
assignment_len(char_u *p, int *heredoc)
{
if (*p == '=')
{
if (p[1] == '<' && p[2] == '<')
{
*heredoc = TRUE;
return 3;
}
return 1;
}
if (vim_strchr((char_u *)"+-*/%", *p) != NULL && p[1] == '=')
return 2;
if (STRNCMP(p, "..=", 3) == 0)
return 3;
return 0;
}
static char *reserved[] = {
"true",
"false",
NULL
};
static char_u *
heredoc_getline(
int c UNUSED,
void *cookie,
int indent UNUSED,
int do_concat UNUSED)
{
cctx_T *cctx = (cctx_T *)cookie;
if (cctx->ctx_lnum == cctx->ctx_ufunc->uf_lines.ga_len)
{
iemsg("Heredoc got to end");
return NULL;
}
++cctx->ctx_lnum;
return vim_strsave(((char_u **)cctx->ctx_ufunc->uf_lines.ga_data)
[cctx->ctx_lnum]);
}
typedef enum {
dest_local,
dest_option,
dest_env,
dest_global,
dest_vimvar,
dest_script,
dest_reg,
} assign_dest_T;
static char_u *
compile_assignment(char_u *arg, exarg_T *eap, cmdidx_T cmdidx, cctx_T *cctx)
{
char_u *p;
char_u *ret = NULL;
int var_count = 0;
int semicolon = 0;
size_t varlen;
garray_T *instr = &cctx->ctx_instr;
int idx = -1;
int new_local = FALSE;
char_u *op;
int opt_type;
assign_dest_T dest = dest_local;
int opt_flags = 0;
int vimvaridx = -1;
int oplen = 0;
int heredoc = FALSE;
type_T *type = &t_any;
lvar_T *lvar;
char_u *name;
char_u *sp;
int has_type = FALSE;
int is_decl = cmdidx == CMD_let || cmdidx == CMD_const;
int instr_count = -1;
p = skip_var_list(arg, FALSE, &var_count, &semicolon);
if (p == NULL)
return NULL;
if (var_count > 0)
{
emsg("Cannot handle a list yet");
return NULL;
}
if (is_decl && p == arg + 2 && p[-1] == ':')
--p;
varlen = p - arg;
name = vim_strnsave(arg, (int)varlen);
if (name == NULL)
return NULL;
if (cctx->ctx_skip != TRUE)
{
if (*arg == '&')
{
int cc;
long numval;
dest = dest_option;
if (cmdidx == CMD_const)
{
emsg(_(e_const_option));
goto theend;
}
if (is_decl)
{
semsg(_("E1052: Cannot declare an option: %s"), arg);
goto theend;
}
p = arg;
p = find_option_end(&p, &opt_flags);
if (p == NULL)
{
emsg(_(e_letunexp));
goto theend;
}
cc = *p;
*p = NUL;
opt_type = get_option_value(arg + 1, &numval, NULL, opt_flags);
*p = cc;
if (opt_type == -3)
{
semsg(_(e_unknown_option), arg);
goto theend;
}
if (opt_type == -2 || opt_type == 0)
type = &t_string;
else
type = &t_number; 
}
else if (*arg == '$')
{
dest = dest_env;
type = &t_string;
if (is_decl)
{
semsg(_("E1065: Cannot declare an environment variable: %s"), name);
goto theend;
}
}
else if (*arg == '@')
{
if (!valid_yank_reg(arg[1], TRUE))
{
emsg_invreg(arg[1]);
goto theend;
}
dest = dest_reg;
type = &t_string;
if (is_decl)
{
semsg(_("E1066: Cannot declare a register: %s"), name);
goto theend;
}
}
else if (STRNCMP(arg, "g:", 2) == 0)
{
dest = dest_global;
if (is_decl)
{
semsg(_("E1016: Cannot declare a global variable: %s"), name);
goto theend;
}
}
else if (STRNCMP(arg, "v:", 2) == 0)
{
typval_T *vtv;
vimvaridx = find_vim_var(name + 2);
if (vimvaridx < 0)
{
semsg(_(e_var_notfound), arg);
goto theend;
}
dest = dest_vimvar;
vtv = get_vim_var_tv(vimvaridx);
type = typval2type(vtv);
if (is_decl)
{
semsg(_("E1064: Cannot declare a v: variable: %s"), name);
goto theend;
}
}
else
{
for (idx = 0; reserved[idx] != NULL; ++idx)
if (STRCMP(reserved[idx], name) == 0)
{
semsg(_("E1034: Cannot use reserved name %s"), name);
goto theend;
}
idx = lookup_local(arg, varlen, cctx);
if (idx >= 0)
{
if (is_decl)
{
semsg(_("E1017: Variable already declared: %s"), name);
goto theend;
}
else
{
lvar = ((lvar_T *)cctx->ctx_locals.ga_data) + idx;
if (lvar->lv_const)
{
semsg(_("E1018: Cannot assign to a constant: %s"), name);
goto theend;
}
}
}
else if (STRNCMP(arg, "s:", 2) == 0
|| lookup_script(arg, varlen) == OK
|| find_imported(arg, varlen, cctx) != NULL)
{
dest = dest_script;
if (is_decl)
{
semsg(_("E1054: Variable already declared in the script: %s"),
name);
goto theend;
}
}
}
}
if (dest != dest_option)
{
if (is_decl && *p == ':')
{
p = skipwhite(p + 1);
type = parse_type(&p, cctx->ctx_type_list);
has_type = TRUE;
}
else if (idx >= 0)
{
lvar = ((lvar_T *)cctx->ctx_locals.ga_data) + idx;
type = lvar->lv_type;
}
}
sp = p;
p = skipwhite(p);
op = p;
oplen = assignment_len(p, &heredoc);
if (oplen > 0 && (!VIM_ISWHITE(*sp) || !VIM_ISWHITE(op[oplen])))
{
char_u buf[4];
vim_strncpy(buf, op, oplen);
semsg(_(e_white_both), buf);
}
if (oplen == 3 && !heredoc && dest != dest_global
&& type->tt_type != VAR_STRING && type->tt_type != VAR_UNKNOWN)
{
emsg(_("E1019: Can only concatenate to string"));
goto theend;
}
if (idx < 0 && dest == dest_local && cctx->ctx_skip != TRUE)
{
if (oplen > 1 && !heredoc)
{
semsg(_("E1020: cannot use an operator on a new variable: %s"),
name);
goto theend;
}
idx = reserve_local(cctx, arg, varlen, cmdidx == CMD_const, type);
if (idx < 0)
goto theend;
new_local = TRUE;
}
if (heredoc)
{
list_T *l;
listitem_T *li;
eap->getline = heredoc_getline;
eap->cookie = cctx;
l = heredoc_get(eap, op + 3);
for (li = l->lv_first; li != NULL; li = li->li_next)
{
generate_PUSHS(cctx, li->li_tv.vval.v_string);
li->li_tv.vval.v_string = NULL;
}
generate_NEWLIST(cctx, l->lv_len);
type = &t_list_string;
list_free(l);
p += STRLEN(p);
}
else if (oplen > 0)
{
int r;
type_T *stacktype;
garray_T *stack;
if (*op != '=')
{
switch (dest)
{
case dest_option:
generate_LOAD(cctx, ISN_LOADOPT, 0, name, type);
break;
case dest_global:
generate_LOAD(cctx, ISN_LOADG, 0, name + 2, type);
break;
case dest_script:
compile_load_scriptvar(cctx,
name + (name[1] == ':' ? 2 : 0), NULL, NULL, TRUE);
break;
case dest_env:
generate_LOAD(cctx, ISN_LOADENV, 0, name, type);
break;
case dest_reg:
generate_LOAD(cctx, ISN_LOADREG, arg[1], NULL, &t_string);
break;
case dest_vimvar:
generate_LOADV(cctx, name + 2, TRUE);
break;
case dest_local:
generate_LOAD(cctx, ISN_LOAD, idx, NULL, type);
break;
}
}
if (new_local)
--cctx->ctx_locals.ga_len;
instr_count = instr->ga_len;
p = skipwhite(p + oplen);
r = compile_expr1(&p, cctx);
if (new_local)
++cctx->ctx_locals.ga_len;
if (r == FAIL)
goto theend;
stack = &cctx->ctx_type_stack;
stacktype = stack->ga_len == 0 ? &t_void
: ((type_T **)stack->ga_data)[stack->ga_len - 1];
if (idx >= 0 && (is_decl || !has_type))
{
lvar = ((lvar_T *)cctx->ctx_locals.ga_data) + idx;
if (!has_type)
{
if (stacktype->tt_type == VAR_VOID)
{
emsg(_("E1031: Cannot use void value"));
goto theend;
}
else
lvar->lv_type = stacktype;
}
else
if (check_type(lvar->lv_type, stacktype, TRUE) == FAIL)
goto theend;
}
else if (*p != '=' && check_type(type, stacktype, TRUE) == FAIL)
goto theend;
}
else if (cmdidx == CMD_const)
{
emsg(_("E1021: const requires a value"));
goto theend;
}
else if (!has_type || dest == dest_option)
{
emsg(_("E1022: type or initialization required"));
goto theend;
}
else
{
if (ga_grow(instr, 1) == FAIL)
goto theend;
switch (type->tt_type)
{
case VAR_BOOL:
generate_PUSHBOOL(cctx, VVAL_FALSE);
break;
case VAR_FLOAT:
#if defined(FEAT_FLOAT)
generate_PUSHF(cctx, 0.0);
#endif
break;
case VAR_STRING:
generate_PUSHS(cctx, NULL);
break;
case VAR_BLOB:
generate_PUSHBLOB(cctx, NULL);
break;
case VAR_FUNC:
generate_PUSHFUNC(cctx, NULL);
break;
case VAR_PARTIAL:
generate_PUSHPARTIAL(cctx, NULL);
break;
case VAR_LIST:
generate_NEWLIST(cctx, 0);
break;
case VAR_DICT:
generate_NEWDICT(cctx, 0);
break;
case VAR_JOB:
generate_PUSHJOB(cctx, NULL);
break;
case VAR_CHANNEL:
generate_PUSHCHANNEL(cctx, NULL);
break;
case VAR_NUMBER:
case VAR_UNKNOWN:
case VAR_VOID:
case VAR_SPECIAL: 
generate_PUSHNR(cctx, 0);
break;
}
}
if (oplen > 0 && *op != '=')
{
type_T *expected = &t_number;
garray_T *stack = &cctx->ctx_type_stack;
type_T *stacktype;
if (*op == '.')
expected = &t_string;
stacktype = ((type_T **)stack->ga_data)[stack->ga_len - 1];
if (need_type(stacktype, expected, -1, cctx) == FAIL)
goto theend;
if (*op == '.')
generate_instr_drop(cctx, ISN_CONCAT, 1);
else
{
isn_T *isn = generate_instr_drop(cctx, ISN_OPNR, 1);
if (isn == NULL)
goto theend;
switch (*op)
{
case '+': isn->isn_arg.op.op_type = EXPR_ADD; break;
case '-': isn->isn_arg.op.op_type = EXPR_SUB; break;
case '*': isn->isn_arg.op.op_type = EXPR_MULT; break;
case '/': isn->isn_arg.op.op_type = EXPR_DIV; break;
case '%': isn->isn_arg.op.op_type = EXPR_REM; break;
}
}
}
switch (dest)
{
case dest_option:
generate_STOREOPT(cctx, name + 1, opt_flags);
break;
case dest_global:
generate_STORE(cctx, ISN_STOREG, 0, name);
break;
case dest_env:
generate_STORE(cctx, ISN_STOREENV, 0, name + 1);
break;
case dest_reg:
generate_STORE(cctx, ISN_STOREREG, name[1], NULL);
break;
case dest_vimvar:
generate_STORE(cctx, ISN_STOREV, vimvaridx, NULL);
break;
case dest_script:
{
char_u *rawname = name + (name[1] == ':' ? 2 : 0);
imported_T *import = NULL;
int sid = current_sctx.sc_sid;
if (name[1] != ':')
{
import = find_imported(name, 0, cctx);
if (import != NULL)
sid = import->imp_sid;
}
idx = get_script_item_idx(sid, rawname, TRUE);
if (idx < 0)
generate_OLDSCRIPT(cctx, ISN_STORES, name, sid, &t_any);
else
generate_VIM9SCRIPT(cctx, ISN_STORESCRIPT,
sid, idx, &t_any);
}
break;
case dest_local:
{
isn_T *isn = ((isn_T *)instr->ga_data) + instr->ga_len - 1;
if (instr->ga_len == instr_count + 1
&& isn->isn_type == ISN_PUSHNR)
{
varnumber_T val = isn->isn_arg.number;
garray_T *stack = &cctx->ctx_type_stack;
isn->isn_type = ISN_STORENR;
isn->isn_arg.storenr.stnr_idx = idx;
isn->isn_arg.storenr.stnr_val = val;
if (stack->ga_len > 0)
--stack->ga_len;
}
else
generate_STORE(cctx, ISN_STORE, idx, NULL);
}
break;
}
ret = p;
theend:
vim_free(name);
return ret;
}
static char_u *
compile_import(char_u *arg, cctx_T *cctx)
{
return handle_import(arg, &cctx->ctx_imports, 0, cctx);
}
static int
compile_jump_to_end(endlabel_T **el, jumpwhen_T when, cctx_T *cctx)
{
garray_T *instr = &cctx->ctx_instr;
endlabel_T *endlabel = ALLOC_CLEAR_ONE(endlabel_T);
if (endlabel == NULL)
return FAIL;
endlabel->el_next = *el;
*el = endlabel;
endlabel->el_end_label = instr->ga_len;
generate_JUMP(cctx, when, 0);
return OK;
}
static void
compile_fill_jump_to_end(endlabel_T **el, cctx_T *cctx)
{
garray_T *instr = &cctx->ctx_instr;
while (*el != NULL)
{
endlabel_T *cur = (*el);
isn_T *isn;
isn = ((isn_T *)instr->ga_data) + cur->el_end_label;
isn->isn_arg.jump.jump_where = instr->ga_len;
*el = cur->el_next;
vim_free(cur);
}
}
static void
compile_free_jump_to_end(endlabel_T **el)
{
while (*el != NULL)
{
endlabel_T *cur = (*el);
*el = cur->el_next;
vim_free(cur);
}
}
static scope_T *
new_scope(cctx_T *cctx, scopetype_T type)
{
scope_T *scope = ALLOC_CLEAR_ONE(scope_T);
if (scope == NULL)
return NULL;
scope->se_outer = cctx->ctx_scope;
cctx->ctx_scope = scope;
scope->se_type = type;
scope->se_local_count = cctx->ctx_locals.ga_len;
return scope;
}
static void
drop_scope(cctx_T *cctx)
{
scope_T *scope = cctx->ctx_scope;
if (scope == NULL)
{
iemsg("calling drop_scope() without a scope");
return;
}
cctx->ctx_scope = scope->se_outer;
switch (scope->se_type)
{
case IF_SCOPE:
compile_free_jump_to_end(&scope->se_u.se_if.is_end_label); break;
case FOR_SCOPE:
compile_free_jump_to_end(&scope->se_u.se_for.fs_end_label); break;
case WHILE_SCOPE:
compile_free_jump_to_end(&scope->se_u.se_while.ws_end_label); break;
case TRY_SCOPE:
compile_free_jump_to_end(&scope->se_u.se_try.ts_end_label); break;
case NO_SCOPE:
case BLOCK_SCOPE:
break;
}
vim_free(scope);
}
static int
evaluate_const_expr7(char_u **arg, cctx_T *cctx UNUSED, typval_T *tv)
{
typval_T argvars[2];
char_u *start_leader, *end_leader;
int has_call = FALSE;
start_leader = *arg;
while (**arg == '!')
*arg = skipwhite(*arg + 1);
end_leader = *arg;
if (STRNCMP("true", *arg, 4) == 0 && !ASCII_ISALNUM((*arg)[4]))
{
tv->v_type = VAR_SPECIAL;
tv->vval.v_number = VVAL_TRUE;
*arg += 4;
return OK;
}
if (STRNCMP("false", *arg, 5) == 0 && !ASCII_ISALNUM((*arg)[5]))
{
tv->v_type = VAR_SPECIAL;
tv->vval.v_number = VVAL_FALSE;
*arg += 5;
return OK;
}
if (STRNCMP("has(", *arg, 4) == 0)
{
has_call = TRUE;
*arg = skipwhite(*arg + 4);
}
if (**arg == '"')
{
if (get_string_tv(arg, tv, TRUE) == FAIL)
return FAIL;
}
else if (**arg == '\'')
{
if (get_lit_string_tv(arg, tv, TRUE) == FAIL)
return FAIL;
}
else
return FAIL;
if (has_call)
{
*arg = skipwhite(*arg);
if (**arg != ')')
return FAIL;
*arg = *arg + 1;
argvars[0] = *tv;
argvars[1].v_type = VAR_UNKNOWN;
tv->v_type = VAR_NUMBER;
tv->vval.v_number = 0;
f_has(argvars, tv);
clear_tv(&argvars[0]);
while (start_leader < end_leader)
{
if (*start_leader == '!')
tv->vval.v_number = !tv->vval.v_number;
++start_leader;
}
}
return OK;
}
static int
evaluate_const_expr4(char_u **arg, cctx_T *cctx UNUSED, typval_T *tv)
{
exptype_T type = EXPR_UNKNOWN;
char_u *p;
int len = 2;
int type_is = FALSE;
if (evaluate_const_expr7(arg, cctx, tv) == FAIL)
return FAIL;
p = skipwhite(*arg);
type = get_compare_type(p, &len, &type_is);
if (type != EXPR_UNKNOWN)
{
typval_T tv2;
char_u *s1, *s2;
char_u buf1[NUMBUFLEN], buf2[NUMBUFLEN];
int n;
if (tv->v_type != VAR_STRING)
return FAIL;
if (type != EXPR_EQUAL)
return FAIL;
init_tv(&tv2);
*arg = skipwhite(p + len);
if (evaluate_const_expr7(arg, cctx, &tv2) == FAIL
|| tv2.v_type != VAR_STRING)
{
clear_tv(&tv2);
return FAIL;
}
s1 = tv_get_string_buf(tv, buf1);
s2 = tv_get_string_buf(&tv2, buf2);
n = STRCMP(s1, s2);
clear_tv(tv);
clear_tv(&tv2);
tv->v_type = VAR_BOOL;
tv->vval.v_number = n == 0 ? VVAL_TRUE : VVAL_FALSE;
}
return OK;
}
static int evaluate_const_expr3(char_u **arg, cctx_T *cctx, typval_T *tv);
static int
evaluate_const_and_or(char_u **arg, cctx_T *cctx, char *op, typval_T *tv)
{
char_u *p = skipwhite(*arg);
int opchar = *op;
if (p[0] == opchar && p[1] == opchar)
{
int val = tv2bool(tv);
while (p[0] == opchar && p[1] == opchar)
{
typval_T tv2;
if (!VIM_ISWHITE(**arg) || !VIM_ISWHITE(p[2]))
return FAIL;
*arg = skipwhite(p + 2);
tv2.v_type = VAR_UNKNOWN;
tv2.v_lock = 0;
if ((opchar == '|' ? evaluate_const_expr3(arg, cctx, &tv2)
: evaluate_const_expr4(arg, cctx, &tv2)) == FAIL)
{
clear_tv(&tv2);
return FAIL;
}
if ((opchar == '&') == val)
{
clear_tv(tv);
*tv = tv2;
val = tv2bool(tv);
}
else
clear_tv(&tv2);
p = skipwhite(*arg);
}
}
return OK;
}
static int
evaluate_const_expr3(char_u **arg, cctx_T *cctx, typval_T *tv)
{
if (evaluate_const_expr4(arg, cctx, tv) == FAIL)
return FAIL;
return evaluate_const_and_or(arg, cctx, "&&", tv);
}
static int
evaluate_const_expr2(char_u **arg, cctx_T *cctx, typval_T *tv)
{
if (evaluate_const_expr3(arg, cctx, tv) == FAIL)
return FAIL;
return evaluate_const_and_or(arg, cctx, "||", tv);
}
static int
evaluate_const_expr1(char_u **arg, cctx_T *cctx, typval_T *tv)
{
char_u *p;
if (evaluate_const_expr2(arg, cctx, tv) == FAIL)
return FAIL;
p = skipwhite(*arg);
if (*p == '?')
{
int val = tv2bool(tv);
typval_T tv2;
if (!VIM_ISWHITE(**arg) || !VIM_ISWHITE(p[1]))
return FAIL;
clear_tv(tv);
*arg = skipwhite(p + 1);
if (evaluate_const_expr1(arg, cctx, tv) == FAIL)
return FAIL;
p = skipwhite(*arg);
if (*p != ':' || !VIM_ISWHITE(**arg) || !VIM_ISWHITE(p[1]))
return FAIL;
*arg = skipwhite(p + 1);
tv2.v_type = VAR_UNKNOWN;
if (evaluate_const_expr1(arg, cctx, &tv2) == FAIL)
{
clear_tv(&tv2);
return FAIL;
}
if (val)
{
clear_tv(&tv2);
}
else
{
clear_tv(tv);
*tv = tv2;
}
}
return OK;
}
static char_u *
compile_if(char_u *arg, cctx_T *cctx)
{
char_u *p = arg;
garray_T *instr = &cctx->ctx_instr;
scope_T *scope;
typval_T tv;
tv.v_type = VAR_UNKNOWN;
if (evaluate_const_expr1(&p, cctx, &tv) == OK)
cctx->ctx_skip = tv2bool(&tv) ? FALSE : TRUE;
else
cctx->ctx_skip = MAYBE;
clear_tv(&tv);
if (cctx->ctx_skip == MAYBE)
{
p = arg;
if (compile_expr1(&p, cctx) == FAIL)
return NULL;
}
scope = new_scope(cctx, IF_SCOPE);
if (scope == NULL)
return NULL;
if (cctx->ctx_skip == MAYBE)
{
scope->se_u.se_if.is_if_label = instr->ga_len;
generate_JUMP(cctx, JUMP_IF_FALSE, 0);
}
else
scope->se_u.se_if.is_if_label = -1;
return p;
}
static char_u *
compile_elseif(char_u *arg, cctx_T *cctx)
{
char_u *p = arg;
garray_T *instr = &cctx->ctx_instr;
isn_T *isn;
scope_T *scope = cctx->ctx_scope;
typval_T tv;
if (scope == NULL || scope->se_type != IF_SCOPE)
{
emsg(_(e_elseif_without_if));
return NULL;
}
unwind_locals(cctx, scope->se_local_count);
if (cctx->ctx_skip == MAYBE)
{
if (compile_jump_to_end(&scope->se_u.se_if.is_end_label,
JUMP_ALWAYS, cctx) == FAIL)
return NULL;
isn = ((isn_T *)instr->ga_data) + scope->se_u.se_if.is_if_label;
isn->isn_arg.jump.jump_where = instr->ga_len;
}
tv.v_type = VAR_UNKNOWN;
if (evaluate_const_expr1(&p, cctx, &tv) == OK)
cctx->ctx_skip = tv2bool(&tv) ? FALSE : TRUE;
else
cctx->ctx_skip = MAYBE;
clear_tv(&tv);
if (cctx->ctx_skip == MAYBE)
{
p = arg;
if (compile_expr1(&p, cctx) == FAIL)
return NULL;
scope->se_u.se_if.is_if_label = instr->ga_len;
generate_JUMP(cctx, JUMP_IF_FALSE, 0);
}
else
scope->se_u.se_if.is_if_label = -1;
return p;
}
static char_u *
compile_else(char_u *arg, cctx_T *cctx)
{
char_u *p = arg;
garray_T *instr = &cctx->ctx_instr;
isn_T *isn;
scope_T *scope = cctx->ctx_scope;
if (scope == NULL || scope->se_type != IF_SCOPE)
{
emsg(_(e_else_without_if));
return NULL;
}
unwind_locals(cctx, scope->se_local_count);
if (cctx->ctx_skip == MAYBE)
{
if (compile_jump_to_end(&scope->se_u.se_if.is_end_label,
JUMP_ALWAYS, cctx) == FAIL)
return NULL;
}
if (cctx->ctx_skip == MAYBE)
{
if (scope->se_u.se_if.is_if_label >= 0)
{
isn = ((isn_T *)instr->ga_data) + scope->se_u.se_if.is_if_label;
isn->isn_arg.jump.jump_where = instr->ga_len;
scope->se_u.se_if.is_if_label = -1;
}
}
if (cctx->ctx_skip != MAYBE)
cctx->ctx_skip = !cctx->ctx_skip;
return p;
}
static char_u *
compile_endif(char_u *arg, cctx_T *cctx)
{
scope_T *scope = cctx->ctx_scope;
ifscope_T *ifscope;
garray_T *instr = &cctx->ctx_instr;
isn_T *isn;
if (scope == NULL || scope->se_type != IF_SCOPE)
{
emsg(_(e_endif_without_if));
return NULL;
}
ifscope = &scope->se_u.se_if;
unwind_locals(cctx, scope->se_local_count);
if (scope->se_u.se_if.is_if_label >= 0)
{
isn = ((isn_T *)instr->ga_data) + scope->se_u.se_if.is_if_label;
isn->isn_arg.jump.jump_where = instr->ga_len;
}
compile_fill_jump_to_end(&ifscope->is_end_label, cctx);
cctx->ctx_skip = FALSE;
drop_scope(cctx);
return arg;
}
static char_u *
compile_for(char_u *arg, cctx_T *cctx)
{
char_u *p;
size_t varlen;
garray_T *instr = &cctx->ctx_instr;
garray_T *stack = &cctx->ctx_type_stack;
scope_T *scope;
int loop_idx; 
int var_idx; 
type_T *vartype;
for (p = arg; eval_isnamec1(*p); ++p)
;
varlen = p - arg;
var_idx = lookup_local(arg, varlen, cctx);
if (var_idx >= 0)
{
semsg(_("E1023: variable already defined: %s"), arg);
return NULL;
}
p = skipwhite(p);
if (STRNCMP(p, "in", 2) != 0 || !VIM_ISWHITE(p[2]))
{
emsg(_(e_missing_in));
return NULL;
}
p = skipwhite(p + 2);
scope = new_scope(cctx, FOR_SCOPE);
if (scope == NULL)
return NULL;
loop_idx = reserve_local(cctx, (char_u *)"", 0, FALSE, &t_number);
if (loop_idx < 0)
{
drop_scope(cctx);
return NULL;
}
var_idx = reserve_local(cctx, arg, varlen, FALSE, &t_any);
if (var_idx < 0)
{
drop_scope(cctx);
return NULL;
}
generate_STORENR(cctx, loop_idx, -1);
arg = p;
if (compile_expr1(&arg, cctx) == FAIL)
{
drop_scope(cctx);
return NULL;
}
vartype = ((type_T **)stack->ga_data)[stack->ga_len - 1];
if (vartype->tt_type != VAR_LIST)
{
emsg(_("E1024: need a List to iterate over"));
drop_scope(cctx);
return NULL;
}
if (vartype->tt_member->tt_type != VAR_UNKNOWN)
{
lvar_T *lvar = ((lvar_T *)cctx->ctx_locals.ga_data) + var_idx;
lvar->lv_type = vartype->tt_member;
}
scope->se_u.se_for.fs_top_label = instr->ga_len;
generate_FOR(cctx, loop_idx);
generate_STORE(cctx, ISN_STORE, var_idx, NULL);
return arg;
}
static char_u *
compile_endfor(char_u *arg, cctx_T *cctx)
{
garray_T *instr = &cctx->ctx_instr;
scope_T *scope = cctx->ctx_scope;
forscope_T *forscope;
isn_T *isn;
if (scope == NULL || scope->se_type != FOR_SCOPE)
{
emsg(_(e_for));
return NULL;
}
forscope = &scope->se_u.se_for;
cctx->ctx_scope = scope->se_outer;
unwind_locals(cctx, scope->se_local_count);
generate_JUMP(cctx, JUMP_ALWAYS, forscope->fs_top_label);
isn = ((isn_T *)instr->ga_data) + forscope->fs_top_label;
isn->isn_arg.forloop.for_end = instr->ga_len;
compile_fill_jump_to_end(&forscope->fs_end_label, cctx);
if (generate_instr_drop(cctx, ISN_DROP, 1) == NULL)
return NULL;
vim_free(scope);
return arg;
}
static char_u *
compile_while(char_u *arg, cctx_T *cctx)
{
char_u *p = arg;
garray_T *instr = &cctx->ctx_instr;
scope_T *scope;
scope = new_scope(cctx, WHILE_SCOPE);
if (scope == NULL)
return NULL;
scope->se_u.se_while.ws_top_label = instr->ga_len;
if (compile_expr1(&p, cctx) == FAIL)
return NULL;
if (compile_jump_to_end(&scope->se_u.se_while.ws_end_label,
JUMP_IF_FALSE, cctx) == FAIL)
return FAIL;
return p;
}
static char_u *
compile_endwhile(char_u *arg, cctx_T *cctx)
{
scope_T *scope = cctx->ctx_scope;
if (scope == NULL || scope->se_type != WHILE_SCOPE)
{
emsg(_(e_while));
return NULL;
}
cctx->ctx_scope = scope->se_outer;
unwind_locals(cctx, scope->se_local_count);
generate_JUMP(cctx, JUMP_ALWAYS, scope->se_u.se_while.ws_top_label);
compile_fill_jump_to_end(&scope->se_u.se_while.ws_end_label, cctx);
vim_free(scope);
return arg;
}
static char_u *
compile_continue(char_u *arg, cctx_T *cctx)
{
scope_T *scope = cctx->ctx_scope;
for (;;)
{
if (scope == NULL)
{
emsg(_(e_continue));
return NULL;
}
if (scope->se_type == FOR_SCOPE || scope->se_type == WHILE_SCOPE)
break;
scope = scope->se_outer;
}
generate_JUMP(cctx, JUMP_ALWAYS,
scope->se_type == FOR_SCOPE ? scope->se_u.se_for.fs_top_label
: scope->se_u.se_while.ws_top_label);
return arg;
}
static char_u *
compile_break(char_u *arg, cctx_T *cctx)
{
scope_T *scope = cctx->ctx_scope;
endlabel_T **el;
for (;;)
{
if (scope == NULL)
{
emsg(_(e_break));
return NULL;
}
if (scope->se_type == FOR_SCOPE || scope->se_type == WHILE_SCOPE)
break;
scope = scope->se_outer;
}
if (scope->se_type == FOR_SCOPE)
el = &scope->se_u.se_for.fs_end_label;
else
el = &scope->se_u.se_while.ws_end_label;
if (compile_jump_to_end(el, JUMP_ALWAYS, cctx) == FAIL)
return FAIL;
return arg;
}
static char_u *
compile_block(char_u *arg, cctx_T *cctx)
{
if (new_scope(cctx, BLOCK_SCOPE) == NULL)
return NULL;
return skipwhite(arg + 1);
}
static void
compile_endblock(cctx_T *cctx)
{
scope_T *scope = cctx->ctx_scope;
cctx->ctx_scope = scope->se_outer;
unwind_locals(cctx, scope->se_local_count);
vim_free(scope);
}
static char_u *
compile_try(char_u *arg, cctx_T *cctx)
{
garray_T *instr = &cctx->ctx_instr;
scope_T *try_scope;
scope_T *scope;
try_scope = new_scope(cctx, TRY_SCOPE);
if (try_scope == NULL)
return NULL;
try_scope->se_u.se_try.ts_try_label = instr->ga_len;
if (generate_instr(cctx, ISN_TRY) == NULL)
return NULL;
scope = new_scope(cctx, BLOCK_SCOPE);
if (scope == NULL)
return NULL;
return arg;
}
static char_u *
compile_catch(char_u *arg, cctx_T *cctx UNUSED)
{
scope_T *scope = cctx->ctx_scope;
garray_T *instr = &cctx->ctx_instr;
char_u *p;
isn_T *isn;
if (scope != NULL && scope->se_type == BLOCK_SCOPE)
compile_endblock(cctx);
scope = cctx->ctx_scope;
if (scope == NULL || scope->se_type != TRY_SCOPE)
{
emsg(_(e_catch));
return NULL;
}
if (scope->se_u.se_try.ts_caught_all)
{
emsg(_("E1033: catch unreachable after catch-all"));
return NULL;
}
if (compile_jump_to_end(&scope->se_u.se_try.ts_end_label,
JUMP_ALWAYS, cctx) == FAIL)
return NULL;
isn = ((isn_T *)instr->ga_data) + scope->se_u.se_try.ts_try_label;
if (isn->isn_arg.try.try_catch == 0)
isn->isn_arg.try.try_catch = instr->ga_len;
if (scope->se_u.se_try.ts_catch_label != 0)
{
isn = ((isn_T *)instr->ga_data) + scope->se_u.se_try.ts_catch_label;
isn->isn_arg.jump.jump_where = instr->ga_len;
}
p = skipwhite(arg);
if (ends_excmd(*p))
{
scope->se_u.se_try.ts_caught_all = TRUE;
scope->se_u.se_try.ts_catch_label = 0;
}
else
{
char_u *end;
char_u *pat;
char_u *tofree = NULL;
int dropped = 0;
int len;
generate_instr_type(cctx, ISN_PUSHEXC, &t_string);
end = skip_regexp_ex(p + 1, *p, TRUE, &tofree, &dropped);
if (*end != *p)
{
semsg(_("E1067: Separator mismatch: %s"), p);
vim_free(tofree);
return FAIL;
}
if (tofree == NULL)
len = (int)(end - (p + 1));
else
len = (int)(end - tofree);
pat = vim_strnsave(tofree == NULL ? p + 1 : tofree, len);
vim_free(tofree);
p += len + 2 + dropped;
if (pat == NULL)
return FAIL;
if (generate_PUSHS(cctx, pat) == FAIL)
return FAIL;
if (generate_COMPARE(cctx, EXPR_MATCH, FALSE) == FAIL)
return NULL;
scope->se_u.se_try.ts_catch_label = instr->ga_len;
if (generate_JUMP(cctx, JUMP_IF_FALSE, 0) == FAIL)
return NULL;
}
if (generate_instr(cctx, ISN_CATCH) == NULL)
return NULL;
if (new_scope(cctx, BLOCK_SCOPE) == NULL)
return NULL;
return p;
}
static char_u *
compile_finally(char_u *arg, cctx_T *cctx)
{
scope_T *scope = cctx->ctx_scope;
garray_T *instr = &cctx->ctx_instr;
isn_T *isn;
if (scope != NULL && scope->se_type == BLOCK_SCOPE)
compile_endblock(cctx);
scope = cctx->ctx_scope;
if (scope == NULL || scope->se_type != TRY_SCOPE)
{
emsg(_(e_finally));
return NULL;
}
isn = ((isn_T *)instr->ga_data) + scope->se_u.se_try.ts_try_label;
if (isn->isn_arg.try.try_finally != 0)
{
emsg(_(e_finally_dup));
return NULL;
}
compile_fill_jump_to_end(&scope->se_u.se_try.ts_end_label, cctx);
isn->isn_arg.try.try_finally = instr->ga_len;
if (scope->se_u.se_try.ts_catch_label != 0)
{
isn = ((isn_T *)instr->ga_data) + scope->se_u.se_try.ts_catch_label;
isn->isn_arg.jump.jump_where = instr->ga_len;
}
return arg;
}
static char_u *
compile_endtry(char_u *arg, cctx_T *cctx)
{
scope_T *scope = cctx->ctx_scope;
garray_T *instr = &cctx->ctx_instr;
isn_T *isn;
if (scope != NULL && scope->se_type == BLOCK_SCOPE)
compile_endblock(cctx);
scope = cctx->ctx_scope;
if (scope == NULL || scope->se_type != TRY_SCOPE)
{
if (scope == NULL)
emsg(_(e_no_endtry));
else if (scope->se_type == WHILE_SCOPE)
emsg(_(e_endwhile));
else if (scope->se_type == FOR_SCOPE)
emsg(_(e_endfor));
else
emsg(_(e_endif));
return NULL;
}
isn = ((isn_T *)instr->ga_data) + scope->se_u.se_try.ts_try_label;
if (isn->isn_arg.try.try_catch == 0 && isn->isn_arg.try.try_finally == 0)
{
emsg(_("E1032: missing :catch or :finally"));
return NULL;
}
compile_fill_jump_to_end(&scope->se_u.se_try.ts_end_label, cctx);
if (isn->isn_arg.try.try_finally == 0)
isn->isn_arg.try.try_finally = instr->ga_len;
compile_endblock(cctx);
if (generate_instr(cctx, ISN_ENDTRY) == NULL)
return NULL;
return arg;
}
static char_u *
compile_throw(char_u *arg, cctx_T *cctx UNUSED)
{
char_u *p = skipwhite(arg);
if (ends_excmd(*p))
{
emsg(_(e_argreq));
return NULL;
}
if (compile_expr1(&p, cctx) == FAIL)
return NULL;
if (may_generate_2STRING(-1, cctx) == FAIL)
return NULL;
if (generate_instr_drop(cctx, ISN_THROW, 1) == NULL)
return NULL;
return p;
}
static char_u *
compile_echo(char_u *arg, int with_white, cctx_T *cctx)
{
char_u *p = arg;
int count = 0;
for (;;)
{
if (compile_expr1(&p, cctx) == FAIL)
return NULL;
++count;
p = skipwhite(p);
if (ends_excmd(*p))
break;
}
generate_ECHO(cctx, with_white, count);
return p;
}
static char_u *
compile_execute(char_u *arg, cctx_T *cctx)
{
char_u *p = arg;
int count = 0;
for (;;)
{
if (compile_expr1(&p, cctx) == FAIL)
return NULL;
++count;
p = skipwhite(p);
if (ends_excmd(*p))
break;
}
generate_EXECUTE(cctx, count);
return p;
}
void
compile_def_function(ufunc_T *ufunc, int set_return_type)
{
char_u *line = NULL;
char_u *p;
exarg_T ea;
char *errormsg = NULL; 
int had_return = FALSE;
cctx_T cctx;
garray_T *instr;
int called_emsg_before = called_emsg;
int ret = FAIL;
sctx_T save_current_sctx = current_sctx;
int emsg_before = called_emsg;
{
dfunc_T *dfunc; 
if (ufunc->uf_dfunc_idx >= 0)
{
dfunc = ((dfunc_T *)def_functions.ga_data) + ufunc->uf_dfunc_idx;
delete_def_function_contents(dfunc);
}
else
{
if (ga_grow(&def_functions, 1) == FAIL)
return;
dfunc = ((dfunc_T *)def_functions.ga_data) + def_functions.ga_len;
vim_memset(dfunc, 0, sizeof(dfunc_T));
dfunc->df_idx = def_functions.ga_len;
ufunc->uf_dfunc_idx = dfunc->df_idx;
dfunc->df_ufunc = ufunc;
++def_functions.ga_len;
}
}
vim_memset(&cctx, 0, sizeof(cctx));
cctx.ctx_ufunc = ufunc;
cctx.ctx_lnum = -1;
ga_init2(&cctx.ctx_locals, sizeof(lvar_T), 10);
ga_init2(&cctx.ctx_type_stack, sizeof(type_T *), 50);
ga_init2(&cctx.ctx_imports, sizeof(imported_T), 10);
cctx.ctx_type_list = &ufunc->uf_type_list;
ga_init2(&cctx.ctx_instr, sizeof(isn_T), 50);
instr = &cctx.ctx_instr;
current_sctx.sc_version = SCRIPT_VERSION_VIM9;
if (ufunc->uf_def_args.ga_len > 0)
{
int count = ufunc->uf_def_args.ga_len;
int i;
char_u *arg;
int off = STACK_FRAME_SIZE + (ufunc->uf_va_name != NULL ? 1 : 0);
ufunc->uf_def_arg_idx = ALLOC_CLEAR_MULT(int, count + 1);
if (ufunc->uf_def_arg_idx == NULL)
goto erret;
for (i = 0; i < count; ++i)
{
ufunc->uf_def_arg_idx[i] = instr->ga_len;
arg = ((char_u **)(ufunc->uf_def_args.ga_data))[i];
if (compile_expr1(&arg, &cctx) == FAIL
|| generate_STORE(&cctx, ISN_STORE,
i - count - off, NULL) == FAIL)
goto erret;
}
if (ufunc->uf_va_name != NULL)
{
if (generate_NEWLIST(&cctx, 0) == FAIL
|| generate_STORE(&cctx, ISN_STORE, -off, NULL) == FAIL)
goto erret;
}
ufunc->uf_def_arg_idx[count] = instr->ga_len;
}
for (;;)
{
int is_ex_command;
if (line != NULL && *line == '|')
++line;
else if (line != NULL && *line != NUL)
{
if (emsg_before == called_emsg)
semsg(_("E488: Trailing characters: %s"), line);
goto erret;
}
else
{
do
{
++cctx.ctx_lnum;
if (cctx.ctx_lnum == ufunc->uf_lines.ga_len)
break;
line = ((char_u **)ufunc->uf_lines.ga_data)[cctx.ctx_lnum];
} while (line == NULL);
if (cctx.ctx_lnum == ufunc->uf_lines.ga_len)
break;
SOURCING_LNUM = ufunc->uf_script_ctx.sc_lnum + cctx.ctx_lnum + 1;
}
emsg_before = called_emsg;
had_return = FALSE;
vim_memset(&ea, 0, sizeof(ea));
ea.cmdlinep = &line;
ea.cmd = skipwhite(line);
if (*ea.cmd == '}')
{
scopetype_T stype = cctx.ctx_scope == NULL
? NO_SCOPE : cctx.ctx_scope->se_type;
if (stype == BLOCK_SCOPE)
{
compile_endblock(&cctx);
line = ea.cmd;
}
else
{
emsg(_("E1025: using } outside of a block scope"));
goto erret;
}
if (line != NULL)
line = skipwhite(ea.cmd + 1);
continue;
}
if (*ea.cmd == '{' && ends_excmd(*skipwhite(ea.cmd + 1)))
{
line = compile_block(ea.cmd, &cctx);
continue;
}
is_ex_command = *ea.cmd == ':';
if (parse_command_modifiers(&ea, &errormsg, FALSE) == FAIL)
{
if (errormsg != NULL)
goto erret;
line = (char_u *)"";
continue;
}
if (checkforcmd(&ea.cmd, "call", 3))
ea.cmd = skipwhite(ea.cmd);
if (!is_ex_command)
{
p = (*ea.cmd == '&' || *ea.cmd == '$' || *ea.cmd == '@')
? ea.cmd + 1 : ea.cmd;
p = to_name_end(p, TRUE);
if (p > ea.cmd && *p != NUL)
{
int oplen;
int heredoc;
oplen = assignment_len(skipwhite(p), &heredoc);
if (oplen > 0)
{
if (*ea.cmd == '&'
|| *ea.cmd == '$'
|| *ea.cmd == '@'
|| ((p - ea.cmd) > 2 && ea.cmd[1] == ':')
|| lookup_local(ea.cmd, p - ea.cmd, &cctx) >= 0
|| lookup_script(ea.cmd, p - ea.cmd) == OK
|| find_imported(ea.cmd, p - ea.cmd, &cctx) != NULL)
{
line = compile_assignment(ea.cmd, &ea, CMD_SIZE, &cctx);
if (line == NULL)
goto erret;
continue;
}
}
}
}
ea.cmd = skip_range(ea.cmd, NULL);
p = find_ex_command(&ea, NULL, is_ex_command ? NULL : lookup_local,
&cctx);
if (p == ea.cmd && ea.cmdidx != CMD_SIZE)
{
if (cctx.ctx_skip == TRUE)
{
line += STRLEN(line);
continue;
}
if (ea.cmdidx == CMD_eval)
{
p = ea.cmd;
if (compile_expr1(&p, &cctx) == FAIL)
goto erret;
generate_instr_drop(&cctx, ISN_DROP, 1);
line = p;
continue;
}
iemsg("Command from find_ex_command() not handled");
goto erret;
}
p = skipwhite(p);
if (cctx.ctx_skip == TRUE
&& ea.cmdidx != CMD_elseif
&& ea.cmdidx != CMD_else
&& ea.cmdidx != CMD_endif)
{
line += STRLEN(line);
continue;
}
switch (ea.cmdidx)
{
case CMD_def:
case CMD_function:
emsg("Nested function not implemented yet");
goto erret;
case CMD_return:
line = compile_return(p, set_return_type, &cctx);
had_return = TRUE;
break;
case CMD_let:
case CMD_const:
line = compile_assignment(p, &ea, ea.cmdidx, &cctx);
break;
case CMD_import:
line = compile_import(p, &cctx);
break;
case CMD_if:
line = compile_if(p, &cctx);
break;
case CMD_elseif:
line = compile_elseif(p, &cctx);
break;
case CMD_else:
line = compile_else(p, &cctx);
break;
case CMD_endif:
line = compile_endif(p, &cctx);
break;
case CMD_while:
line = compile_while(p, &cctx);
break;
case CMD_endwhile:
line = compile_endwhile(p, &cctx);
break;
case CMD_for:
line = compile_for(p, &cctx);
break;
case CMD_endfor:
line = compile_endfor(p, &cctx);
break;
case CMD_continue:
line = compile_continue(p, &cctx);
break;
case CMD_break:
line = compile_break(p, &cctx);
break;
case CMD_try:
line = compile_try(p, &cctx);
break;
case CMD_catch:
line = compile_catch(p, &cctx);
break;
case CMD_finally:
line = compile_finally(p, &cctx);
break;
case CMD_endtry:
line = compile_endtry(p, &cctx);
break;
case CMD_throw:
line = compile_throw(p, &cctx);
break;
case CMD_echo:
line = compile_echo(p, TRUE, &cctx);
break;
case CMD_echon:
line = compile_echo(p, FALSE, &cctx);
break;
case CMD_execute:
line = compile_execute(p, &cctx);
break;
default:
generate_EXEC(&cctx, line);
line = (char_u *)"";
break;
}
if (line == NULL)
goto erret;
line = skipwhite(line);
if (cctx.ctx_type_stack.ga_len < 0)
{
iemsg("Type stack underflow");
goto erret;
}
}
if (cctx.ctx_scope != NULL)
{
if (cctx.ctx_scope->se_type == IF_SCOPE)
emsg(_(e_endif));
else if (cctx.ctx_scope->se_type == WHILE_SCOPE)
emsg(_(e_endwhile));
else if (cctx.ctx_scope->se_type == FOR_SCOPE)
emsg(_(e_endfor));
else
emsg(_("E1026: Missing }"));
goto erret;
}
if (!had_return)
{
if (ufunc->uf_ret_type->tt_type != VAR_VOID)
{
emsg(_("E1027: Missing return statement"));
goto erret;
}
generate_PUSHNR(&cctx, 0);
generate_instr(&cctx, ISN_RETURN);
}
{
dfunc_T *dfunc = ((dfunc_T *)def_functions.ga_data)
+ ufunc->uf_dfunc_idx;
dfunc->df_deleted = FALSE;
dfunc->df_instr = instr->ga_data;
dfunc->df_instr_count = instr->ga_len;
dfunc->df_varcount = cctx.ctx_max_local;
}
ret = OK;
erret:
if (ret == FAIL)
{
int idx;
dfunc_T *dfunc = ((dfunc_T *)def_functions.ga_data)
+ ufunc->uf_dfunc_idx;
for (idx = 0; idx < instr->ga_len; ++idx)
delete_instr(((isn_T *)instr->ga_data) + idx);
ga_clear(instr);
ufunc->uf_dfunc_idx = -1;
if (!dfunc->df_deleted)
--def_functions.ga_len;
while (cctx.ctx_scope != NULL)
drop_scope(&cctx);
ga_clear_strings(&ufunc->uf_lines);
if (errormsg != NULL)
emsg(errormsg);
else if (called_emsg == called_emsg_before)
emsg(_("E1028: compile_def_function failed"));
}
current_sctx = save_current_sctx;
free_imported(&cctx);
free_local(&cctx);
ga_clear(&cctx.ctx_type_stack);
}
void
delete_instr(isn_T *isn)
{
switch (isn->isn_type)
{
case ISN_EXEC:
case ISN_LOADENV:
case ISN_LOADG:
case ISN_LOADOPT:
case ISN_MEMBER:
case ISN_PUSHEXC:
case ISN_PUSHS:
case ISN_STOREENV:
case ISN_STOREG:
case ISN_PUSHFUNC:
vim_free(isn->isn_arg.string);
break;
case ISN_LOADS:
case ISN_STORES:
vim_free(isn->isn_arg.loadstore.ls_name);
break;
case ISN_STOREOPT:
vim_free(isn->isn_arg.storeopt.so_name);
break;
case ISN_PUSHBLOB: 
blob_unref(isn->isn_arg.blob);
break;
case ISN_PUSHPARTIAL:
partial_unref(isn->isn_arg.partial);
break;
case ISN_PUSHJOB:
#if defined(FEAT_JOB_CHANNEL)
job_unref(isn->isn_arg.job);
#endif
break;
case ISN_PUSHCHANNEL:
#if defined(FEAT_JOB_CHANNEL)
channel_unref(isn->isn_arg.channel);
#endif
break;
case ISN_UCALL:
vim_free(isn->isn_arg.ufunc.cuf_name);
break;
case ISN_2BOOL:
case ISN_2STRING:
case ISN_ADDBLOB:
case ISN_ADDLIST:
case ISN_BCALL:
case ISN_CATCH:
case ISN_CHECKNR:
case ISN_CHECKTYPE:
case ISN_COMPAREANY:
case ISN_COMPAREBLOB:
case ISN_COMPAREBOOL:
case ISN_COMPAREDICT:
case ISN_COMPAREFLOAT:
case ISN_COMPAREFUNC:
case ISN_COMPARELIST:
case ISN_COMPARENR:
case ISN_COMPAREPARTIAL:
case ISN_COMPARESPECIAL:
case ISN_COMPARESTRING:
case ISN_CONCAT:
case ISN_DCALL:
case ISN_DROP:
case ISN_ECHO:
case ISN_EXECUTE:
case ISN_ENDTRY:
case ISN_FOR:
case ISN_FUNCREF:
case ISN_INDEX:
case ISN_JUMP:
case ISN_LOAD:
case ISN_LOADSCRIPT:
case ISN_LOADREG:
case ISN_LOADV:
case ISN_NEGATENR:
case ISN_NEWDICT:
case ISN_NEWLIST:
case ISN_OPNR:
case ISN_OPFLOAT:
case ISN_OPANY:
case ISN_PCALL:
case ISN_PCALL_END:
case ISN_PUSHF:
case ISN_PUSHNR:
case ISN_PUSHBOOL:
case ISN_PUSHSPEC:
case ISN_RETURN:
case ISN_STORE:
case ISN_STOREV:
case ISN_STORENR:
case ISN_STOREREG:
case ISN_STORESCRIPT:
case ISN_THROW:
case ISN_TRY:
break;
}
}
static void
delete_def_function_contents(dfunc_T *dfunc)
{
int idx;
ga_clear(&dfunc->df_def_args_isn);
if (dfunc->df_instr != NULL)
{
for (idx = 0; idx < dfunc->df_instr_count; ++idx)
delete_instr(dfunc->df_instr + idx);
VIM_CLEAR(dfunc->df_instr);
}
dfunc->df_deleted = TRUE;
}
void
delete_def_function(ufunc_T *ufunc)
{
if (ufunc->uf_dfunc_idx >= 0)
{
dfunc_T *dfunc = ((dfunc_T *)def_functions.ga_data)
+ ufunc->uf_dfunc_idx;
delete_def_function_contents(dfunc);
}
}
#if defined(EXITFREE) || defined(PROTO)
void
free_def_functions(void)
{
int idx;
for (idx = 0; idx < def_functions.ga_len; ++idx)
{
dfunc_T *dfunc = ((dfunc_T *)def_functions.ga_data) + idx;
delete_def_function_contents(dfunc);
}
ga_clear(&def_functions);
}
#endif
#endif 
