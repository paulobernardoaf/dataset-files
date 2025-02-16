#include <math.h>
#if defined(HAVE_LOCALE_H)
#include <locale.h>
#endif
#include "nvim/ascii.h"
#include "nvim/buffer.h"
#include "nvim/change.h"
#include "nvim/channel.h"
#include "nvim/charset.h"
#include "nvim/cursor.h"
#include "nvim/edit.h"
#include "nvim/eval.h"
#include "nvim/eval/encode.h"
#include "nvim/eval/executor.h"
#include "nvim/eval/gc.h"
#include "nvim/eval/typval.h"
#include "nvim/ex_cmds2.h"
#include "nvim/ex_docmd.h"
#include "nvim/ex_getln.h"
#include "nvim/ex_session.h"
#include "nvim/fileio.h"
#include "nvim/getchar.h"
#include "nvim/lua/executor.h"
#include "nvim/mark.h"
#include "nvim/memline.h"
#include "nvim/misc1.h"
#include "nvim/move.h"
#include "nvim/ops.h"
#include "nvim/option.h"
#include "nvim/os/input.h"
#include "nvim/os/os.h"
#include "nvim/os/shell.h"
#include "nvim/path.h"
#include "nvim/quickfix.h"
#include "nvim/regexp.h"
#include "nvim/screen.h"
#include "nvim/search.h"
#include "nvim/sign.h"
#include "nvim/syntax.h"
#include "nvim/ui.h"
#include "nvim/undo.h"
#include "nvim/version.h"
#include "nvim/window.h"
#define DICT_MAXNEST 100 
#define AUTOLOAD_CHAR '#'
static char *e_letunexp = N_("E18: Unexpected characters in :let");
static char *e_missbrac = N_("E111: Missing ']'");
static char *e_funcexts = N_(
"E122: Function %s already exists, add ! to replace it");
static char *e_funcdict = N_("E717: Dictionary entry already exists");
static char *e_funcref = N_("E718: Funcref required");
static char *e_dictrange = N_("E719: Cannot use [:] with a Dictionary");
static char *e_nofunc = N_("E130: Unknown function: %s");
static char *e_illvar = N_("E461: Illegal variable name: %s");
static char *e_cannot_mod = N_("E995: Cannot modify existing variable");
static char *e_letwrong = N_("E734: Wrong variable type for %s=");
static char_u * const namespace_char = (char_u *)"abglstvw";
static ScopeDictDictItem globvars_var;
#define globvarht globvardict.dv_hashtab
static hashtab_T compat_hashtab;
hashtab_T func_hashtab;
static int *eval_lavars_used = NULL;
typedef struct {
ScopeDictDictItem sv_var;
dict_T sv_dict;
} scriptvar_T;
static garray_T ga_scripts = {0, 0, sizeof(scriptvar_T *), 4, NULL};
#define SCRIPT_SV(id) (((scriptvar_T **)ga_scripts.ga_data)[(id) - 1])
#define SCRIPT_VARS(id) (SCRIPT_SV(id)->sv_dict.dv_hashtab)
static int echo_attr = 0; 
#define FC_ABORT 0x01 
#define FC_RANGE 0x02 
#define FC_DICT 0x04 
#define FC_CLOSURE 0x08 
#define FC_DELETED 0x10 
#define FC_REMOVED 0x20 
#define FC_SANDBOX 0x40 
static garray_T ga_loaded = { 0, 0, sizeof(char_u *), 4, NULL };
#define FUNCARG(fp, j) ((char_u **)(fp->uf_args.ga_data))[j]
#define FUNCLINE(fp, j) ((char_u **)(fp->uf_lines.ga_data))[j]
#define VAR_SHORT_LEN 20
#define FIXVAR_CNT 12
struct funccall_S {
ufunc_T *func; 
int linenr; 
int returned; 
TV_DICTITEM_STRUCT(VAR_SHORT_LEN + 1) fixvar[FIXVAR_CNT];
dict_T l_vars; 
ScopeDictDictItem l_vars_var; 
dict_T l_avars; 
ScopeDictDictItem l_avars_var; 
list_T l_varlist; 
listitem_T l_listitems[MAX_FUNC_ARGS]; 
typval_T *rettv; 
linenr_T breakpoint; 
int dbg_tick; 
int level; 
proftime_T prof_child; 
funccall_T *caller; 
int fc_refcount; 
int fc_copyID; 
garray_T fc_funcs; 
};
typedef struct {
int fi_semicolon; 
int fi_varcount; 
listwatch_T fi_lw; 
list_T *fi_list; 
} forinfo_T;
#define VV_COMPAT 1 
#define VV_RO 2 
#define VV_RO_SBX 4 
#define VV(idx, name, type, flags) [idx] = { .vv_name = name, .vv_di = { .di_tv = { .v_type = type }, .di_flags = 0, .di_key = { 0 }, }, .vv_flags = flags, }
static struct vimvar {
char *vv_name; 
TV_DICTITEM_STRUCT(17) vv_di; 
char vv_flags; 
} vimvars[] =
{
VV(VV_COUNT, "count", VAR_NUMBER, VV_RO),
VV(VV_COUNT1, "count1", VAR_NUMBER, VV_RO),
VV(VV_PREVCOUNT, "prevcount", VAR_NUMBER, VV_RO),
VV(VV_ERRMSG, "errmsg", VAR_STRING, 0),
VV(VV_WARNINGMSG, "warningmsg", VAR_STRING, 0),
VV(VV_STATUSMSG, "statusmsg", VAR_STRING, 0),
VV(VV_SHELL_ERROR, "shell_error", VAR_NUMBER, VV_RO),
VV(VV_THIS_SESSION, "this_session", VAR_STRING, 0),
VV(VV_VERSION, "version", VAR_NUMBER, VV_COMPAT+VV_RO),
VV(VV_LNUM, "lnum", VAR_NUMBER, VV_RO_SBX),
VV(VV_TERMRESPONSE, "termresponse", VAR_STRING, VV_RO),
VV(VV_FNAME, "fname", VAR_STRING, VV_RO),
VV(VV_LANG, "lang", VAR_STRING, VV_RO),
VV(VV_LC_TIME, "lc_time", VAR_STRING, VV_RO),
VV(VV_CTYPE, "ctype", VAR_STRING, VV_RO),
VV(VV_CC_FROM, "charconvert_from", VAR_STRING, VV_RO),
VV(VV_CC_TO, "charconvert_to", VAR_STRING, VV_RO),
VV(VV_FNAME_IN, "fname_in", VAR_STRING, VV_RO),
VV(VV_FNAME_OUT, "fname_out", VAR_STRING, VV_RO),
VV(VV_FNAME_NEW, "fname_new", VAR_STRING, VV_RO),
VV(VV_FNAME_DIFF, "fname_diff", VAR_STRING, VV_RO),
VV(VV_CMDARG, "cmdarg", VAR_STRING, VV_RO),
VV(VV_FOLDSTART, "foldstart", VAR_NUMBER, VV_RO_SBX),
VV(VV_FOLDEND, "foldend", VAR_NUMBER, VV_RO_SBX),
VV(VV_FOLDDASHES, "folddashes", VAR_STRING, VV_RO_SBX),
VV(VV_FOLDLEVEL, "foldlevel", VAR_NUMBER, VV_RO_SBX),
VV(VV_PROGNAME, "progname", VAR_STRING, VV_RO),
VV(VV_SEND_SERVER, "servername", VAR_STRING, VV_RO),
VV(VV_DYING, "dying", VAR_NUMBER, VV_RO),
VV(VV_EXCEPTION, "exception", VAR_STRING, VV_RO),
VV(VV_THROWPOINT, "throwpoint", VAR_STRING, VV_RO),
VV(VV_STDERR, "stderr", VAR_NUMBER, VV_RO),
VV(VV_REG, "register", VAR_STRING, VV_RO),
VV(VV_CMDBANG, "cmdbang", VAR_NUMBER, VV_RO),
VV(VV_INSERTMODE, "insertmode", VAR_STRING, VV_RO),
VV(VV_VAL, "val", VAR_UNKNOWN, VV_RO),
VV(VV_KEY, "key", VAR_UNKNOWN, VV_RO),
VV(VV_PROFILING, "profiling", VAR_NUMBER, VV_RO),
VV(VV_FCS_REASON, "fcs_reason", VAR_STRING, VV_RO),
VV(VV_FCS_CHOICE, "fcs_choice", VAR_STRING, 0),
VV(VV_BEVAL_BUFNR, "beval_bufnr", VAR_NUMBER, VV_RO),
VV(VV_BEVAL_WINNR, "beval_winnr", VAR_NUMBER, VV_RO),
VV(VV_BEVAL_WINID, "beval_winid", VAR_NUMBER, VV_RO),
VV(VV_BEVAL_LNUM, "beval_lnum", VAR_NUMBER, VV_RO),
VV(VV_BEVAL_COL, "beval_col", VAR_NUMBER, VV_RO),
VV(VV_BEVAL_TEXT, "beval_text", VAR_STRING, VV_RO),
VV(VV_SCROLLSTART, "scrollstart", VAR_STRING, 0),
VV(VV_SWAPNAME, "swapname", VAR_STRING, VV_RO),
VV(VV_SWAPCHOICE, "swapchoice", VAR_STRING, 0),
VV(VV_SWAPCOMMAND, "swapcommand", VAR_STRING, VV_RO),
VV(VV_CHAR, "char", VAR_STRING, 0),
VV(VV_MOUSE_WIN, "mouse_win", VAR_NUMBER, 0),
VV(VV_MOUSE_WINID, "mouse_winid", VAR_NUMBER, 0),
VV(VV_MOUSE_LNUM, "mouse_lnum", VAR_NUMBER, 0),
VV(VV_MOUSE_COL, "mouse_col", VAR_NUMBER, 0),
VV(VV_OP, "operator", VAR_STRING, VV_RO),
VV(VV_SEARCHFORWARD, "searchforward", VAR_NUMBER, 0),
VV(VV_HLSEARCH, "hlsearch", VAR_NUMBER, 0),
VV(VV_OLDFILES, "oldfiles", VAR_LIST, 0),
VV(VV_WINDOWID, "windowid", VAR_NUMBER, VV_RO_SBX),
VV(VV_PROGPATH, "progpath", VAR_STRING, VV_RO),
VV(VV_COMPLETED_ITEM, "completed_item", VAR_DICT, VV_RO),
VV(VV_OPTION_NEW, "option_new", VAR_STRING, VV_RO),
VV(VV_OPTION_OLD, "option_old", VAR_STRING, VV_RO),
VV(VV_OPTION_TYPE, "option_type", VAR_STRING, VV_RO),
VV(VV_ERRORS, "errors", VAR_LIST, 0),
VV(VV_MSGPACK_TYPES, "msgpack_types", VAR_DICT, VV_RO),
VV(VV_EVENT, "event", VAR_DICT, VV_RO),
VV(VV_FALSE, "false", VAR_SPECIAL, VV_RO),
VV(VV_TRUE, "true", VAR_SPECIAL, VV_RO),
VV(VV_NULL, "null", VAR_SPECIAL, VV_RO),
VV(VV__NULL_LIST, "_null_list", VAR_LIST, VV_RO),
VV(VV__NULL_DICT, "_null_dict", VAR_DICT, VV_RO),
VV(VV_VIM_DID_ENTER, "vim_did_enter", VAR_NUMBER, VV_RO),
VV(VV_TESTING, "testing", VAR_NUMBER, 0),
VV(VV_TYPE_NUMBER, "t_number", VAR_NUMBER, VV_RO),
VV(VV_TYPE_STRING, "t_string", VAR_NUMBER, VV_RO),
VV(VV_TYPE_FUNC, "t_func", VAR_NUMBER, VV_RO),
VV(VV_TYPE_LIST, "t_list", VAR_NUMBER, VV_RO),
VV(VV_TYPE_DICT, "t_dict", VAR_NUMBER, VV_RO),
VV(VV_TYPE_FLOAT, "t_float", VAR_NUMBER, VV_RO),
VV(VV_TYPE_BOOL, "t_bool", VAR_NUMBER, VV_RO),
VV(VV_ECHOSPACE, "echospace", VAR_NUMBER, VV_RO),
VV(VV_EXITING, "exiting", VAR_NUMBER, VV_RO),
VV(VV_LUA, "lua", VAR_PARTIAL, VV_RO),
};
#undef VV
#define vv_type vv_di.di_tv.v_type
#define vv_nr vv_di.di_tv.vval.v_number
#define vv_special vv_di.di_tv.vval.v_special
#define vv_float vv_di.di_tv.vval.v_float
#define vv_str vv_di.di_tv.vval.v_string
#define vv_list vv_di.di_tv.vval.v_list
#define vv_dict vv_di.di_tv.vval.v_dict
#define vv_partial vv_di.di_tv.vval.v_partial
#define vv_tv vv_di.di_tv
static ScopeDictDictItem vimvars_var;
static partial_T *vvlua_partial;
#define vimvarht vimvardict.dv_hashtab
#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "eval.c.generated.h"
#endif
static uint64_t last_timer_id = 1;
static PMap(uint64_t) *timers = NULL;
static const char *const msgpack_type_names[] = {
[kMPNil] = "nil",
[kMPBoolean] = "boolean",
[kMPInteger] = "integer",
[kMPFloat] = "float",
[kMPString] = "string",
[kMPBinary] = "binary",
[kMPArray] = "array",
[kMPMap] = "map",
[kMPExt] = "ext",
};
const list_T *eval_msgpack_type_lists[] = {
[kMPNil] = NULL,
[kMPBoolean] = NULL,
[kMPInteger] = NULL,
[kMPFloat] = NULL,
[kMPString] = NULL,
[kMPBinary] = NULL,
[kMPArray] = NULL,
[kMPMap] = NULL,
[kMPExt] = NULL,
};
varnumber_T num_divide(varnumber_T n1, varnumber_T n2)
FUNC_ATTR_CONST FUNC_ATTR_WARN_UNUSED_RESULT
{
varnumber_T result;
if (n2 == 0) { 
if (n1 == 0) {
result = VARNUMBER_MIN; 
} else if (n1 < 0) {
result = -VARNUMBER_MAX;
} else {
result = VARNUMBER_MAX;
}
} else {
result = n1 / n2;
}
return result;
}
varnumber_T num_modulus(varnumber_T n1, varnumber_T n2)
FUNC_ATTR_CONST FUNC_ATTR_WARN_UNUSED_RESULT
{
return (n2 == 0) ? 0 : (n1 % n2);
}
void eval_init(void)
{
vimvars[VV_VERSION].vv_nr = VIM_VERSION_100;
timers = pmap_new(uint64_t)();
struct vimvar *p;
init_var_dict(&globvardict, &globvars_var, VAR_DEF_SCOPE);
init_var_dict(&vimvardict, &vimvars_var, VAR_SCOPE);
vimvardict.dv_lock = VAR_FIXED;
hash_init(&compat_hashtab);
hash_init(&func_hashtab);
for (size_t i = 0; i < ARRAY_SIZE(vimvars); i++) {
p = &vimvars[i];
assert(STRLEN(p->vv_name) <= 16);
STRCPY(p->vv_di.di_key, p->vv_name);
if (p->vv_flags & VV_RO)
p->vv_di.di_flags = DI_FLAGS_RO | DI_FLAGS_FIX;
else if (p->vv_flags & VV_RO_SBX)
p->vv_di.di_flags = DI_FLAGS_RO_SBX | DI_FLAGS_FIX;
else
p->vv_di.di_flags = DI_FLAGS_FIX;
if (p->vv_type != VAR_UNKNOWN)
hash_add(&vimvarht, p->vv_di.di_key);
if (p->vv_flags & VV_COMPAT)
hash_add(&compat_hashtab, p->vv_di.di_key);
}
vimvars[VV_VERSION].vv_nr = VIM_VERSION_100;
dict_T *const msgpack_types_dict = tv_dict_alloc();
for (size_t i = 0; i < ARRAY_SIZE(msgpack_type_names); i++) {
list_T *const type_list = tv_list_alloc(0);
tv_list_set_lock(type_list, VAR_FIXED);
tv_list_ref(type_list);
dictitem_T *const di = tv_dict_item_alloc(msgpack_type_names[i]);
di->di_flags |= DI_FLAGS_RO|DI_FLAGS_FIX;
di->di_tv = (typval_T) {
.v_type = VAR_LIST,
.vval = { .v_list = type_list, },
};
eval_msgpack_type_lists[i] = type_list;
if (tv_dict_add(msgpack_types_dict, di) == FAIL) {
assert(false);
}
}
msgpack_types_dict->dv_lock = VAR_FIXED;
set_vim_var_dict(VV_MSGPACK_TYPES, msgpack_types_dict);
set_vim_var_dict(VV_COMPLETED_ITEM, tv_dict_alloc());
dict_T *v_event = tv_dict_alloc();
v_event->dv_lock = VAR_FIXED;
set_vim_var_dict(VV_EVENT, v_event);
set_vim_var_list(VV_ERRORS, tv_list_alloc(kListLenUnknown));
set_vim_var_nr(VV_STDERR, CHAN_STDERR);
set_vim_var_nr(VV_SEARCHFORWARD, 1L);
set_vim_var_nr(VV_HLSEARCH, 1L);
set_vim_var_nr(VV_COUNT1, 1);
set_vim_var_nr(VV_TYPE_NUMBER, VAR_TYPE_NUMBER);
set_vim_var_nr(VV_TYPE_STRING, VAR_TYPE_STRING);
set_vim_var_nr(VV_TYPE_FUNC, VAR_TYPE_FUNC);
set_vim_var_nr(VV_TYPE_LIST, VAR_TYPE_LIST);
set_vim_var_nr(VV_TYPE_DICT, VAR_TYPE_DICT);
set_vim_var_nr(VV_TYPE_FLOAT, VAR_TYPE_FLOAT);
set_vim_var_nr(VV_TYPE_BOOL, VAR_TYPE_BOOL);
set_vim_var_special(VV_FALSE, kSpecialVarFalse);
set_vim_var_special(VV_TRUE, kSpecialVarTrue);
set_vim_var_special(VV_NULL, kSpecialVarNull);
set_vim_var_special(VV_EXITING, kSpecialVarNull);
set_vim_var_nr(VV_ECHOSPACE, sc_col - 1);
vimvars[VV_LUA].vv_type = VAR_PARTIAL;
vvlua_partial = xcalloc(1, sizeof(partial_T));
vimvars[VV_LUA].vv_partial = vvlua_partial;
vvlua_partial->pt_name = xmallocz(0);
vvlua_partial->pt_refcount++;
set_reg_var(0); 
}
#if defined(EXITFREE)
void eval_clear(void)
{
struct vimvar *p;
for (size_t i = 0; i < ARRAY_SIZE(vimvars); i++) {
p = &vimvars[i];
if (p->vv_di.di_tv.v_type == VAR_STRING) {
XFREE_CLEAR(p->vv_str);
} else if (p->vv_di.di_tv.v_type == VAR_LIST) {
tv_list_unref(p->vv_list);
p->vv_list = NULL;
}
}
hash_clear(&vimvarht);
hash_init(&vimvarht); 
hash_clear(&compat_hashtab);
free_scriptnames();
free_locales();
vars_clear(&globvarht);
ga_clear_strings(&ga_loaded);
for (int i = 1; i <= ga_scripts.ga_len; ++i)
vars_clear(&SCRIPT_VARS(i));
for (int i = 1; i <= ga_scripts.ga_len; ++i)
xfree(SCRIPT_SV(i));
ga_clear(&ga_scripts);
(void)garbage_collect(false);
free_all_functions();
}
#endif
char_u *func_name(void *cookie)
{
return ((funccall_T *)cookie)->func->uf_name;
}
linenr_T *func_breakpoint(void *cookie)
{
return &((funccall_T *)cookie)->breakpoint;
}
int *func_dbg_tick(void *cookie)
{
return &((funccall_T *)cookie)->dbg_tick;
}
int func_level(void *cookie)
{
return ((funccall_T *)cookie)->level;
}
funccall_T *current_funccal = NULL;
funccall_T *previous_funccal = NULL;
int current_func_returned(void)
{
return current_funccal->returned;
}
void set_internal_string_var(char_u *name, char_u *value)
{
const typval_T tv = {
.v_type = VAR_STRING,
.vval.v_string = value,
};
set_var((const char *)name, STRLEN(name), (typval_T *)&tv, true);
}
static lval_T *redir_lval = NULL;
static garray_T redir_ga; 
static char_u *redir_endp = NULL;
static char_u *redir_varname = NULL;
int
var_redir_start(
char_u *name,
int append 
)
{
int save_emsg;
int err;
typval_T tv;
if (!eval_isnamec1(*name)) {
EMSG(_(e_invarg));
return FAIL;
}
redir_varname = vim_strsave(name);
redir_lval = xcalloc(1, sizeof(lval_T));
ga_init(&redir_ga, (int)sizeof(char), 500);
redir_endp = (char_u *)get_lval(redir_varname, NULL, redir_lval, false, false,
0, FNE_CHECK_START);
if (redir_endp == NULL || redir_lval->ll_name == NULL
|| *redir_endp != NUL) {
clear_lval(redir_lval);
if (redir_endp != NULL && *redir_endp != NUL)
EMSG(_(e_trailing));
else
EMSG(_(e_invarg));
redir_endp = NULL; 
var_redir_stop();
return FAIL;
}
save_emsg = did_emsg;
did_emsg = FALSE;
tv.v_type = VAR_STRING;
tv.vval.v_string = (char_u *)"";
if (append) {
set_var_lval(redir_lval, redir_endp, &tv, true, false, (char_u *)".");
} else {
set_var_lval(redir_lval, redir_endp, &tv, true, false, (char_u *)"=");
}
clear_lval(redir_lval);
err = did_emsg;
did_emsg |= save_emsg;
if (err) {
redir_endp = NULL; 
var_redir_stop();
return FAIL;
}
return OK;
}
void var_redir_str(char_u *value, int value_len)
{
int len;
if (redir_lval == NULL)
return;
if (value_len == -1)
len = (int)STRLEN(value); 
else
len = value_len; 
ga_grow(&redir_ga, len);
memmove((char *)redir_ga.ga_data + redir_ga.ga_len, value, len);
redir_ga.ga_len += len;
}
void var_redir_stop(void)
{
typval_T tv;
if (redir_lval != NULL) {
if (redir_endp != NULL) {
ga_append(&redir_ga, NUL); 
tv.v_type = VAR_STRING;
tv.vval.v_string = redir_ga.ga_data;
redir_endp = (char_u *)get_lval(redir_varname, NULL, redir_lval,
false, false, 0, FNE_CHECK_START);
if (redir_endp != NULL && redir_lval->ll_name != NULL) {
set_var_lval(redir_lval, redir_endp, &tv, false, false, (char_u *)".");
}
clear_lval(redir_lval);
}
XFREE_CLEAR(redir_ga.ga_data);
XFREE_CLEAR(redir_lval);
}
XFREE_CLEAR(redir_varname);
}
int eval_charconvert(const char *const enc_from, const char *const enc_to,
const char *const fname_from, const char *const fname_to)
{
bool err = false;
set_vim_var_string(VV_CC_FROM, enc_from, -1);
set_vim_var_string(VV_CC_TO, enc_to, -1);
set_vim_var_string(VV_FNAME_IN, fname_from, -1);
set_vim_var_string(VV_FNAME_OUT, fname_to, -1);
if (eval_to_bool(p_ccv, &err, NULL, false)) {
err = true;
}
set_vim_var_string(VV_CC_FROM, NULL, -1);
set_vim_var_string(VV_CC_TO, NULL, -1);
set_vim_var_string(VV_FNAME_IN, NULL, -1);
set_vim_var_string(VV_FNAME_OUT, NULL, -1);
if (err) {
return FAIL;
}
return OK;
}
int eval_printexpr(const char *const fname, const char *const args)
{
bool err = false;
set_vim_var_string(VV_FNAME_IN, fname, -1);
set_vim_var_string(VV_CMDARG, args, -1);
if (eval_to_bool(p_pexpr, &err, NULL, false)) {
err = true;
}
set_vim_var_string(VV_FNAME_IN, NULL, -1);
set_vim_var_string(VV_CMDARG, NULL, -1);
if (err) {
os_remove(fname);
return FAIL;
}
return OK;
}
void eval_diff(const char *const origfile, const char *const newfile,
const char *const outfile)
{
bool err = false;
set_vim_var_string(VV_FNAME_IN, origfile, -1);
set_vim_var_string(VV_FNAME_NEW, newfile, -1);
set_vim_var_string(VV_FNAME_OUT, outfile, -1);
(void)eval_to_bool(p_dex, &err, NULL, FALSE);
set_vim_var_string(VV_FNAME_IN, NULL, -1);
set_vim_var_string(VV_FNAME_NEW, NULL, -1);
set_vim_var_string(VV_FNAME_OUT, NULL, -1);
}
void eval_patch(const char *const origfile, const char *const difffile,
const char *const outfile)
{
bool err = false;
set_vim_var_string(VV_FNAME_IN, origfile, -1);
set_vim_var_string(VV_FNAME_DIFF, difffile, -1);
set_vim_var_string(VV_FNAME_OUT, outfile, -1);
(void)eval_to_bool(p_pex, &err, NULL, FALSE);
set_vim_var_string(VV_FNAME_IN, NULL, -1);
set_vim_var_string(VV_FNAME_DIFF, NULL, -1);
set_vim_var_string(VV_FNAME_OUT, NULL, -1);
}
int
eval_to_bool(
char_u *arg,
bool *error,
char_u **nextcmd,
int skip 
)
{
typval_T tv;
bool retval = false;
if (skip) {
emsg_skip++;
}
if (eval0(arg, &tv, nextcmd, !skip) == FAIL) {
*error = true;
} else {
*error = false;
if (!skip) {
retval = (tv_get_number_chk(&tv, error) != 0);
tv_clear(&tv);
}
}
if (skip) {
emsg_skip--;
}
return retval;
}
static int eval1_emsg(char_u **arg, typval_T *rettv, bool evaluate)
FUNC_ATTR_NONNULL_ARG(1, 2)
{
const char_u *const start = *arg;
const int did_emsg_before = did_emsg;
const int called_emsg_before = called_emsg;
const int ret = eval1(arg, rettv, evaluate);
if (ret == FAIL) {
if (!aborting()
&& did_emsg == did_emsg_before
&& called_emsg == called_emsg_before) {
emsgf(_(e_invexpr2), start);
}
}
return ret;
}
int eval_expr_typval(const typval_T *expr, typval_T *argv,
int argc, typval_T *rettv)
FUNC_ATTR_NONNULL_ARG(1, 2, 4)
{
int dummy;
if (expr->v_type == VAR_FUNC) {
const char_u *const s = expr->vval.v_string;
if (s == NULL || *s == NUL) {
return FAIL;
}
if (call_func(s, (int)STRLEN(s), rettv, argc, argv, NULL,
0L, 0L, &dummy, true, NULL, NULL) == FAIL) {
return FAIL;
}
} else if (expr->v_type == VAR_PARTIAL) {
partial_T *const partial = expr->vval.v_partial;
const char_u *const s = partial_name(partial);
if (s == NULL || *s == NUL) {
return FAIL;
}
if (call_func(s, (int)STRLEN(s), rettv, argc, argv, NULL,
0L, 0L, &dummy, true, partial, NULL) == FAIL) {
return FAIL;
}
} else {
char buf[NUMBUFLEN];
char_u *s = (char_u *)tv_get_string_buf_chk(expr, buf);
if (s == NULL) {
return FAIL;
}
s = skipwhite(s);
if (eval1_emsg(&s, rettv, true) == FAIL) {
return FAIL;
}
if (*s != NUL) { 
tv_clear(rettv);
emsgf(_(e_invexpr2), s);
return FAIL;
}
}
return OK;
}
bool eval_expr_to_bool(const typval_T *expr, bool *error)
FUNC_ATTR_NONNULL_ARG(1, 2)
{
typval_T argv, rettv;
if (eval_expr_typval(expr, &argv, 0, &rettv) == FAIL) {
*error = true;
return false;
}
const bool res = (tv_get_number_chk(&rettv, error) != 0);
tv_clear(&rettv);
return res;
}
char *eval_to_string_skip(const char *arg, const char **nextcmd,
const bool skip)
FUNC_ATTR_MALLOC FUNC_ATTR_NONNULL_ARG(1) FUNC_ATTR_WARN_UNUSED_RESULT
{
typval_T tv;
char *retval;
if (skip) {
emsg_skip++;
}
if (eval0((char_u *)arg, &tv, (char_u **)nextcmd, !skip) == FAIL || skip) {
retval = NULL;
} else {
retval = xstrdup(tv_get_string(&tv));
tv_clear(&tv);
}
if (skip) {
emsg_skip--;
}
return retval;
}
int skip_expr(char_u **pp)
{
typval_T rettv;
*pp = skipwhite(*pp);
return eval1(pp, &rettv, FALSE);
}
char_u *eval_to_string(char_u *arg, char_u **nextcmd, int convert)
{
typval_T tv;
char *retval;
garray_T ga;
if (eval0(arg, &tv, nextcmd, true) == FAIL) {
retval = NULL;
} else {
if (convert && tv.v_type == VAR_LIST) {
ga_init(&ga, (int)sizeof(char), 80);
if (tv.vval.v_list != NULL) {
tv_list_join(&ga, tv.vval.v_list, "\n");
if (tv_list_len(tv.vval.v_list) > 0) {
ga_append(&ga, NL);
}
}
ga_append(&ga, NUL);
retval = (char *)ga.ga_data;
} else if (convert && tv.v_type == VAR_FLOAT) {
char numbuf[NUMBUFLEN];
vim_snprintf(numbuf, NUMBUFLEN, "%g", tv.vval.v_float);
retval = xstrdup(numbuf);
} else {
retval = xstrdup(tv_get_string(&tv));
}
tv_clear(&tv);
}
return (char_u *)retval;
}
char_u *eval_to_string_safe(char_u *arg, char_u **nextcmd, int use_sandbox)
{
char_u *retval;
void *save_funccalp;
save_funccalp = save_funccal();
if (use_sandbox)
++sandbox;
++textlock;
retval = eval_to_string(arg, nextcmd, FALSE);
if (use_sandbox)
--sandbox;
--textlock;
restore_funccal(save_funccalp);
return retval;
}
varnumber_T eval_to_number(char_u *expr)
{
typval_T rettv;
varnumber_T retval;
char_u *p = skipwhite(expr);
++emsg_off;
if (eval1(&p, &rettv, true) == FAIL) {
retval = -1;
} else {
retval = tv_get_number_chk(&rettv, NULL);
tv_clear(&rettv);
}
--emsg_off;
return retval;
}
static void prepare_vimvar(int idx, typval_T *save_tv)
{
*save_tv = vimvars[idx].vv_tv;
if (vimvars[idx].vv_type == VAR_UNKNOWN)
hash_add(&vimvarht, vimvars[idx].vv_di.di_key);
}
static void restore_vimvar(int idx, typval_T *save_tv)
{
hashitem_T *hi;
vimvars[idx].vv_tv = *save_tv;
if (vimvars[idx].vv_type == VAR_UNKNOWN) {
hi = hash_find(&vimvarht, vimvars[idx].vv_di.di_key);
if (HASHITEM_EMPTY(hi)) {
internal_error("restore_vimvar()");
} else {
hash_remove(&vimvarht, hi);
}
}
}
void find_win_for_curbuf(void)
{
for (wininfo_T *wip = curbuf->b_wininfo; wip != NULL; wip = wip->wi_next) {
if (wip->wi_win != NULL) {
curwin = wip->wi_win;
break;
}
}
}
list_T *eval_spell_expr(char_u *badword, char_u *expr)
{
typval_T save_val;
typval_T rettv;
list_T *list = NULL;
char_u *p = skipwhite(expr);
prepare_vimvar(VV_VAL, &save_val);
vimvars[VV_VAL].vv_type = VAR_STRING;
vimvars[VV_VAL].vv_str = badword;
if (p_verbose == 0)
++emsg_off;
if (eval1(&p, &rettv, true) == OK) {
if (rettv.v_type != VAR_LIST) {
tv_clear(&rettv);
} else {
list = rettv.vval.v_list;
}
}
if (p_verbose == 0)
--emsg_off;
restore_vimvar(VV_VAL, &save_val);
return list;
}
int get_spellword(list_T *const list, const char **ret_word)
{
if (tv_list_len(list) != 2) {
EMSG(_("E5700: Expression from 'spellsuggest' must yield lists with "
"exactly two values"));
return -1;
}
*ret_word = tv_list_find_str(list, 0);
if (*ret_word == NULL) {
return -1;
}
return tv_list_find_nr(list, -1, NULL);
}
int call_vim_function(
const char_u *func,
int argc,
typval_T *argv,
typval_T *rettv
)
FUNC_ATTR_NONNULL_ALL
{
int doesrange;
int ret;
int len = (int)STRLEN(func);
partial_T *pt = NULL;
if (len >= 6 && !memcmp(func, "v:lua.", 6)) {
func += 6;
len = check_luafunc_name((const char *)func, false);
if (len == 0) {
ret = FAIL;
goto fail;
}
pt = vvlua_partial;
}
rettv->v_type = VAR_UNKNOWN; 
ret = call_func(func, len, rettv, argc, argv, NULL,
curwin->w_cursor.lnum, curwin->w_cursor.lnum,
&doesrange, true, pt, NULL);
fail:
if (ret == FAIL) {
tv_clear(rettv);
}
return ret;
}
varnumber_T call_func_retnr(const char_u *func, int argc,
typval_T *argv)
FUNC_ATTR_NONNULL_ALL
{
typval_T rettv;
varnumber_T retval;
if (call_vim_function(func, argc, argv, &rettv) == FAIL) {
return -1;
}
retval = tv_get_number_chk(&rettv, NULL);
tv_clear(&rettv);
return retval;
}
char *call_func_retstr(const char *const func, int argc,
typval_T *argv)
FUNC_ATTR_NONNULL_ALL FUNC_ATTR_WARN_UNUSED_RESULT FUNC_ATTR_MALLOC
{
typval_T rettv;
if (call_vim_function((const char_u *)func, argc, argv, &rettv)
== FAIL) {
return NULL;
}
char *const retval = xstrdup(tv_get_string(&rettv));
tv_clear(&rettv);
return retval;
}
void *call_func_retlist(const char_u *func, int argc, typval_T *argv)
FUNC_ATTR_NONNULL_ALL
{
typval_T rettv;
if (call_vim_function(func, argc, argv, &rettv) == FAIL) {
return NULL;
}
if (rettv.v_type != VAR_LIST) {
tv_clear(&rettv);
return NULL;
}
return rettv.vval.v_list;
}
void *save_funccal(void)
{
funccall_T *fc = current_funccal;
current_funccal = NULL;
return (void *)fc;
}
void restore_funccal(void *vfc)
{
funccall_T *fc = (funccall_T *)vfc;
current_funccal = fc;
}
void prof_child_enter(proftime_T *tm 
)
{
funccall_T *fc = current_funccal;
if (fc != NULL && fc->func->uf_profiling) {
fc->prof_child = profile_start();
}
script_prof_save(tm);
}
void prof_child_exit(proftime_T *tm 
)
{
funccall_T *fc = current_funccal;
if (fc != NULL && fc->func->uf_profiling) {
fc->prof_child = profile_end(fc->prof_child);
fc->prof_child = profile_sub_wait(*tm, fc->prof_child);
fc->func->uf_tm_children =
profile_add(fc->func->uf_tm_children, fc->prof_child);
fc->func->uf_tml_children =
profile_add(fc->func->uf_tml_children, fc->prof_child);
}
script_prof_restore(tm);
}
int eval_foldexpr(char_u *arg, int *cp)
{
typval_T tv;
varnumber_T retval;
char_u *s;
int use_sandbox = was_set_insecurely((char_u *)"foldexpr",
OPT_LOCAL);
++emsg_off;
if (use_sandbox)
++sandbox;
++textlock;
*cp = NUL;
if (eval0(arg, &tv, NULL, TRUE) == FAIL)
retval = 0;
else {
if (tv.v_type == VAR_NUMBER)
retval = tv.vval.v_number;
else if (tv.v_type != VAR_STRING || tv.vval.v_string == NULL)
retval = 0;
else {
s = tv.vval.v_string;
if (!ascii_isdigit(*s) && *s != '-')
*cp = *s++;
retval = atol((char *)s);
}
tv_clear(&tv);
}
--emsg_off;
if (use_sandbox)
--sandbox;
--textlock;
return (int)retval;
}
void ex_const(exarg_T *eap)
{
ex_let_const(eap, true);
}
static list_T *
heredoc_get(exarg_T *eap, char_u *cmd)
{
char_u *marker;
char_u *p;
int marker_indent_len = 0;
int text_indent_len = 0;
char_u *text_indent = NULL;
if (eap->getline == NULL) {
EMSG(_("E991: cannot use =<< here"));
return NULL;
}
cmd = skipwhite(cmd);
if (STRNCMP(cmd, "trim", 4) == 0
&& (cmd[4] == NUL || ascii_iswhite(cmd[4]))) {
cmd = skipwhite(cmd + 4);
p = *eap->cmdlinep;
while (ascii_iswhite(*p)) {
p++;
marker_indent_len++;
}
text_indent_len = -1;
}
if (*cmd != NUL && *cmd != '"') {
marker = skipwhite(cmd);
p = skiptowhite(marker);
if (*skipwhite(p) != NUL && *skipwhite(p) != '"') {
EMSG(_(e_trailing));
return NULL;
}
*p = NUL;
if (islower(*marker)) {
EMSG(_("E221: Marker cannot start with lower case letter"));
return NULL;
}
} else {
EMSG(_("E172: Missing marker"));
return NULL;
}
list_T *l = tv_list_alloc(0);
for (;;) {
int mi = 0;
int ti = 0;
char_u *theline = eap->getline(NUL, eap->cookie, 0, false);
if (theline == NULL) {
EMSG2(_("E990: Missing end marker '%s'"), marker);
break;
}
if (marker_indent_len > 0
&& STRNCMP(theline, *eap->cmdlinep, marker_indent_len) == 0) {
mi = marker_indent_len;
}
if (STRCMP(marker, theline + mi) == 0) {
xfree(theline);
break;
}
if (text_indent_len == -1 && *theline != NUL) {
p = theline;
text_indent_len = 0;
while (ascii_iswhite(*p)) {
p++;
text_indent_len++;
}
text_indent = vim_strnsave(theline, text_indent_len);
}
if (text_indent != NULL) {
for (ti = 0; ti < text_indent_len; ti++) {
if (theline[ti] != text_indent[ti]) {
break;
}
}
}
tv_list_append_string(l, (char *)(theline + ti), -1);
xfree(theline);
}
xfree(text_indent);
return l;
}
void ex_let(exarg_T *eap)
{
ex_let_const(eap, false);
}
static void ex_let_const(exarg_T *eap, const bool is_const)
{
char_u *arg = eap->arg;
char_u *expr = NULL;
typval_T rettv;
int i;
int var_count = 0;
int semicolon = 0;
char_u op[2];
char_u *argend;
int first = TRUE;
argend = (char_u *)skip_var_list(arg, &var_count, &semicolon);
if (argend == NULL) {
return;
}
if (argend > arg && argend[-1] == '.') { 
argend--;
}
expr = skipwhite(argend);
if (*expr != '=' && !((vim_strchr((char_u *)"+-*/%.", *expr) != NULL
&& expr[1] == '=') || STRNCMP(expr, "..=", 3) == 0)) {
if (*arg == '[') {
EMSG(_(e_invarg));
} else if (!ends_excmd(*arg)) {
arg = (char_u *)list_arg_vars(eap, (const char *)arg, &first);
} else if (!eap->skip) {
list_glob_vars(&first);
list_buf_vars(&first);
list_win_vars(&first);
list_tab_vars(&first);
list_script_vars(&first);
list_func_vars(&first);
list_vim_vars(&first);
}
eap->nextcmd = check_nextcmd(arg);
} else if (expr[0] == '=' && expr[1] == '<' && expr[2] == '<') {
list_T *l = heredoc_get(eap, expr + 3);
if (l != NULL) {
tv_list_set_ret(&rettv, l);
if (!eap->skip) {
op[0] = '=';
op[1] = NUL;
(void)ex_let_vars(eap->arg, &rettv, false, semicolon, var_count,
is_const, op);
}
tv_clear(&rettv);
}
} else {
op[0] = '=';
op[1] = NUL;
if (*expr != '=') {
if (vim_strchr((char_u *)"+-*/%.", *expr) != NULL) {
op[0] = *expr; 
if (expr[0] == '.' && expr[1] == '.') { 
expr++;
}
}
expr = skipwhite(expr + 2);
} else {
expr = skipwhite(expr + 1);
}
if (eap->skip)
++emsg_skip;
i = eval0(expr, &rettv, &eap->nextcmd, !eap->skip);
if (eap->skip) {
if (i != FAIL) {
tv_clear(&rettv);
}
emsg_skip--;
} else if (i != FAIL) {
(void)ex_let_vars(eap->arg, &rettv, false, semicolon, var_count,
is_const, op);
tv_clear(&rettv);
}
}
}
static int
ex_let_vars(
char_u *arg_start,
typval_T *tv,
int copy, 
int semicolon, 
int var_count, 
int is_const, 
char_u *nextchars
)
{
char_u *arg = arg_start;
typval_T ltv;
if (*arg != '[') {
if (ex_let_one(arg, tv, copy, is_const, nextchars, nextchars) == NULL) {
return FAIL;
}
return OK;
}
if (tv->v_type != VAR_LIST) {
EMSG(_(e_listreq));
return FAIL;
}
list_T *const l = tv->vval.v_list;
const int len = tv_list_len(l);
if (semicolon == 0 && var_count < len) {
EMSG(_("E687: Less targets than List items"));
return FAIL;
}
if (var_count - semicolon > len) {
EMSG(_("E688: More targets than List items"));
return FAIL;
}
assert(l != NULL);
listitem_T *item = tv_list_first(l);
size_t rest_len = tv_list_len(l);
while (*arg != ']') {
arg = skipwhite(arg + 1);
arg = ex_let_one(arg, TV_LIST_ITEM_TV(item), true, is_const,
(const char_u *)",;]", nextchars);
if (arg == NULL) {
return FAIL;
}
rest_len--;
item = TV_LIST_ITEM_NEXT(l, item);
arg = skipwhite(arg);
if (*arg == ';') {
list_T *const rest_list = tv_list_alloc(rest_len);
while (item != NULL) {
tv_list_append_tv(rest_list, TV_LIST_ITEM_TV(item));
item = TV_LIST_ITEM_NEXT(l, item);
}
ltv.v_type = VAR_LIST;
ltv.v_lock = VAR_UNLOCKED;
ltv.vval.v_list = rest_list;
tv_list_ref(rest_list);
arg = ex_let_one(skipwhite(arg + 1), &ltv, false, is_const,
(char_u *)"]", nextchars);
tv_clear(&ltv);
if (arg == NULL) {
return FAIL;
}
break;
} else if (*arg != ',' && *arg != ']') {
internal_error("ex_let_vars()");
return FAIL;
}
}
return OK;
}
static const char_u *skip_var_list(const char_u *arg, int *var_count,
int *semicolon)
{
const char_u *p;
const char_u *s;
if (*arg == '[') {
p = arg;
for (;; ) {
p = skipwhite(p + 1); 
s = skip_var_one(p);
if (s == p) {
EMSG2(_(e_invarg2), p);
return NULL;
}
++*var_count;
p = skipwhite(s);
if (*p == ']')
break;
else if (*p == ';') {
if (*semicolon == 1) {
EMSG(_("Double ; in list of variables"));
return NULL;
}
*semicolon = 1;
} else if (*p != ',') {
EMSG2(_(e_invarg2), p);
return NULL;
}
}
return p + 1;
} else
return skip_var_one(arg);
}
static const char_u *skip_var_one(const char_u *arg)
{
if (*arg == '@' && arg[1] != NUL)
return arg + 2;
return find_name_end(*arg == '$' || *arg == '&' ? arg + 1 : arg,
NULL, NULL, FNE_INCL_BR | FNE_CHECK_START);
}
static void list_hashtable_vars(hashtab_T *ht, const char *prefix, int empty,
int *first)
{
hashitem_T *hi;
dictitem_T *di;
int todo;
todo = (int)ht->ht_used;
for (hi = ht->ht_array; todo > 0 && !got_int; ++hi) {
if (!HASHITEM_EMPTY(hi)) {
todo--;
di = TV_DICT_HI2DI(hi);
char buf[IOSIZE];
xstrlcpy(buf, prefix, IOSIZE - 1);
xstrlcat(buf, (char *)di->di_key, IOSIZE);
if (message_filtered((char_u *)buf)) {
continue;
}
if (empty || di->di_tv.v_type != VAR_STRING
|| di->di_tv.vval.v_string != NULL) {
list_one_var(di, prefix, first);
}
}
}
}
static void list_glob_vars(int *first)
{
list_hashtable_vars(&globvarht, "", true, first);
}
static void list_buf_vars(int *first)
{
list_hashtable_vars(&curbuf->b_vars->dv_hashtab, "b:", true, first);
}
static void list_win_vars(int *first)
{
list_hashtable_vars(&curwin->w_vars->dv_hashtab, "w:", true, first);
}
static void list_tab_vars(int *first)
{
list_hashtable_vars(&curtab->tp_vars->dv_hashtab, "t:", true, first);
}
static void list_vim_vars(int *first)
{
list_hashtable_vars(&vimvarht, "v:", false, first);
}
static void list_script_vars(int *first)
{
if (current_sctx.sc_sid > 0 && current_sctx.sc_sid <= ga_scripts.ga_len) {
list_hashtable_vars(&SCRIPT_VARS(current_sctx.sc_sid), "s:", false, first);
}
}
static void list_func_vars(int *first)
{
if (current_funccal != NULL) {
list_hashtable_vars(&current_funccal->l_vars.dv_hashtab, "l:", false,
first);
}
}
static const char *list_arg_vars(exarg_T *eap, const char *arg, int *first)
{
int error = FALSE;
int len;
const char *name;
const char *name_start;
typval_T tv;
while (!ends_excmd(*arg) && !got_int) {
if (error || eap->skip) {
arg = (const char *)find_name_end((char_u *)arg, NULL, NULL,
FNE_INCL_BR | FNE_CHECK_START);
if (!ascii_iswhite(*arg) && !ends_excmd(*arg)) {
emsg_severe = TRUE;
EMSG(_(e_trailing));
break;
}
} else {
name_start = name = arg;
char *tofree;
len = get_name_len(&arg, &tofree, true, true);
if (len <= 0) {
if (len < 0 && !aborting()) {
emsg_severe = TRUE;
EMSG2(_(e_invarg2), arg);
break;
}
error = TRUE;
} else {
if (tofree != NULL) {
name = tofree;
}
if (get_var_tv((const char *)name, len, &tv, NULL, true, false)
== FAIL) {
error = true;
} else {
const char *const arg_subsc = arg;
if (handle_subscript(&arg, &tv, true, true) == FAIL) {
error = true;
} else {
if (arg == arg_subsc && len == 2 && name[1] == ':') {
switch (*name) {
case 'g': list_glob_vars(first); break;
case 'b': list_buf_vars(first); break;
case 'w': list_win_vars(first); break;
case 't': list_tab_vars(first); break;
case 'v': list_vim_vars(first); break;
case 's': list_script_vars(first); break;
case 'l': list_func_vars(first); break;
default:
EMSG2(_("E738: Can't list variables for %s"), name);
}
} else {
char *const s = encode_tv2echo(&tv, NULL);
const char *const used_name = (arg == arg_subsc
? name
: name_start);
const ptrdiff_t name_size = (used_name == tofree
? (ptrdiff_t)strlen(used_name)
: (arg - used_name));
list_one_var_a("", used_name, name_size,
tv.v_type, s == NULL ? "" : s, first);
xfree(s);
}
tv_clear(&tv);
}
}
}
xfree(tofree);
}
arg = (const char *)skipwhite((const char_u *)arg);
}
return arg;
}
static char_u *ex_let_one(char_u *arg, typval_T *const tv,
const bool copy, const bool is_const,
const char_u *const endchars, const char_u *const op)
FUNC_ATTR_NONNULL_ARG(1, 2) FUNC_ATTR_WARN_UNUSED_RESULT
{
char_u *arg_end = NULL;
int len;
int opt_flags;
char_u *tofree = NULL;
if (*arg == '$') {
if (is_const) {
EMSG(_("E996: Cannot lock an environment variable"));
return NULL;
}
arg++;
char *name = (char *)arg;
len = get_env_len((const char_u **)&arg);
if (len == 0) {
EMSG2(_(e_invarg2), name - 1);
} else {
if (op != NULL && vim_strchr((char_u *)"+-*/%", *op) != NULL) {
EMSG2(_(e_letwrong), op);
} else if (endchars != NULL
&& vim_strchr(endchars, *skipwhite(arg)) == NULL) {
EMSG(_(e_letunexp));
} else if (!check_secure()) {
const char c1 = name[len];
name[len] = NUL;
const char *p = tv_get_string_chk(tv);
if (p != NULL && op != NULL && *op == '.') {
char *s = vim_getenv(name);
if (s != NULL) {
tofree = concat_str((const char_u *)s, (const char_u *)p);
p = (const char *)tofree;
xfree(s);
}
}
if (p != NULL) {
os_setenv(name, p, 1);
if (STRICMP(name, "HOME") == 0) {
init_homedir();
} else if (didset_vim && STRICMP(name, "VIM") == 0) {
didset_vim = false;
} else if (didset_vimruntime
&& STRICMP(name, "VIMRUNTIME") == 0) {
didset_vimruntime = false;
}
arg_end = arg;
}
name[len] = c1;
xfree(tofree);
}
}
} else if (*arg == '&') {
if (is_const) {
EMSG(_("E996: Cannot lock an option"));
return NULL;
}
char *const p = (char *)find_option_end((const char **)&arg, &opt_flags);
if (p == NULL
|| (endchars != NULL
&& vim_strchr(endchars, *skipwhite((const char_u *)p)) == NULL)) {
EMSG(_(e_letunexp));
} else {
int opt_type;
long numval;
char *stringval = NULL;
const char c1 = *p;
*p = NUL;
varnumber_T n = tv_get_number(tv);
const char *s = tv_get_string_chk(tv); 
if (s != NULL && op != NULL && *op != '=') {
opt_type = get_option_value(arg, &numval, (char_u **)&stringval,
opt_flags);
if ((opt_type == 1 && *op == '.')
|| (opt_type == 0 && *op != '.')) {
EMSG2(_(e_letwrong), op);
s = NULL; 
} else {
if (opt_type == 1) { 
switch (*op) {
case '+': n = numval + n; break;
case '-': n = numval - n; break;
case '*': n = numval * n; break;
case '/': n = num_divide(numval, n); break;
case '%': n = num_modulus(numval, n); break;
}
} else if (opt_type == 0 && stringval != NULL) { 
char *const oldstringval = stringval;
stringval = (char *)concat_str((const char_u *)stringval,
(const char_u *)s);
xfree(oldstringval);
s = stringval;
}
}
}
if (s != NULL) {
set_option_value((const char *)arg, n, s, opt_flags);
arg_end = (char_u *)p;
}
*p = c1;
xfree(stringval);
}
} else if (*arg == '@') {
if (is_const) {
EMSG(_("E996: Cannot lock a register"));
return NULL;
}
arg++;
if (op != NULL && vim_strchr((char_u *)"+-*/%", *op) != NULL) {
emsgf(_(e_letwrong), op);
} else if (endchars != NULL
&& vim_strchr(endchars, *skipwhite(arg + 1)) == NULL) {
EMSG(_(e_letunexp));
} else {
char_u *s;
char_u *ptofree = NULL;
const char *p = tv_get_string_chk(tv);
if (p != NULL && op != NULL && *op == '.') {
s = get_reg_contents(*arg == '@' ? '"' : *arg, kGRegExprSrc);
if (s != NULL) {
ptofree = concat_str(s, (const char_u *)p);
p = (const char *)ptofree;
xfree(s);
}
}
if (p != NULL) {
write_reg_contents(*arg == '@' ? '"' : *arg,
(const char_u *)p, STRLEN(p), false);
arg_end = arg + 1;
}
xfree(ptofree);
}
}
else if (eval_isnamec1(*arg) || *arg == '{') {
lval_T lv;
char_u *const p = get_lval(arg, tv, &lv, false, false, 0, FNE_CHECK_START);
if (p != NULL && lv.ll_name != NULL) {
if (endchars != NULL && vim_strchr(endchars, *skipwhite(p)) == NULL) {
EMSG(_(e_letunexp));
} else {
set_var_lval(&lv, p, tv, copy, is_const, op);
arg_end = p;
}
}
clear_lval(&lv);
} else
EMSG2(_(e_invarg2), arg);
return arg_end;
}
char_u *get_lval(char_u *const name, typval_T *const rettv,
lval_T *const lp, const bool unlet, const bool skip,
const int flags, const int fne_flags)
FUNC_ATTR_NONNULL_ARG(1, 3)
{
dictitem_T *v;
typval_T var1;
typval_T var2;
int empty1 = FALSE;
listitem_T *ni;
hashtab_T *ht;
int quiet = flags & GLV_QUIET;
memset(lp, 0, sizeof(lval_T));
if (skip) {
lp->ll_name = (const char *)name;
return (char_u *)find_name_end((const char_u *)name, NULL, NULL,
FNE_INCL_BR | fne_flags);
}
char_u *expr_start;
char_u *expr_end;
char_u *p = (char_u *)find_name_end(name,
(const char_u **)&expr_start,
(const char_u **)&expr_end,
fne_flags);
if (expr_start != NULL) {
if (unlet && !ascii_iswhite(*p) && !ends_excmd(*p)
&& *p != '[' && *p != '.') {
EMSG(_(e_trailing));
return NULL;
}
lp->ll_exp_name = (char *)make_expanded_name(name, expr_start, expr_end,
(char_u *)p);
lp->ll_name = lp->ll_exp_name;
if (lp->ll_exp_name == NULL) {
if (!aborting() && !quiet) {
emsg_severe = TRUE;
EMSG2(_(e_invarg2), name);
return NULL;
}
lp->ll_name_len = 0;
} else {
lp->ll_name_len = strlen(lp->ll_name);
}
} else {
lp->ll_name = (const char *)name;
lp->ll_name_len = (size_t)((const char *)p - lp->ll_name);
}
if ((*p != '[' && *p != '.') || lp->ll_name == NULL) {
return p;
}
v = find_var(lp->ll_name, lp->ll_name_len,
(flags & GLV_READ_ONLY) ? NULL : &ht,
flags & GLV_NO_AUTOLOAD);
if (v == NULL && !quiet) {
emsgf(_("E121: Undefined variable: %.*s"),
(int)lp->ll_name_len, lp->ll_name);
}
if (v == NULL) {
return NULL;
}
lp->ll_tv = &v->di_tv;
var1.v_type = VAR_UNKNOWN;
var2.v_type = VAR_UNKNOWN;
while (*p == '[' || (*p == '.' && lp->ll_tv->v_type == VAR_DICT)) {
if (!(lp->ll_tv->v_type == VAR_LIST && lp->ll_tv->vval.v_list != NULL)
&& !(lp->ll_tv->v_type == VAR_DICT
&& lp->ll_tv->vval.v_dict != NULL)) {
if (!quiet)
EMSG(_("E689: Can only index a List or Dictionary"));
return NULL;
}
if (lp->ll_range) {
if (!quiet)
EMSG(_("E708: [:] must come last"));
return NULL;
}
int len = -1;
char_u *key = NULL;
if (*p == '.') {
key = p + 1;
for (len = 0; ASCII_ISALNUM(key[len]) || key[len] == '_'; len++) {
}
if (len == 0) {
if (!quiet) {
EMSG(_("E713: Cannot use empty key after ."));
}
return NULL;
}
p = key + len;
} else {
p = skipwhite(p + 1);
if (*p == ':') {
empty1 = true;
} else {
empty1 = false;
if (eval1(&p, &var1, true) == FAIL) { 
return NULL;
}
if (!tv_check_str(&var1)) {
tv_clear(&var1);
return NULL;
}
}
if (*p == ':') {
if (lp->ll_tv->v_type == VAR_DICT) {
if (!quiet) {
EMSG(_(e_dictrange));
}
tv_clear(&var1);
return NULL;
}
if (rettv != NULL && (rettv->v_type != VAR_LIST
|| rettv->vval.v_list == NULL)) {
if (!quiet) {
EMSG(_("E709: [:] requires a List value"));
}
tv_clear(&var1);
return NULL;
}
p = skipwhite(p + 1);
if (*p == ']') {
lp->ll_empty2 = true;
} else {
lp->ll_empty2 = false;
if (eval1(&p, &var2, true) == FAIL) { 
tv_clear(&var1);
return NULL;
}
if (!tv_check_str(&var2)) {
tv_clear(&var1);
tv_clear(&var2);
return NULL;
}
}
lp->ll_range = TRUE;
} else
lp->ll_range = FALSE;
if (*p != ']') {
if (!quiet) {
EMSG(_(e_missbrac));
}
tv_clear(&var1);
tv_clear(&var2);
return NULL;
}
++p;
}
if (lp->ll_tv->v_type == VAR_DICT) {
if (len == -1) {
key = (char_u *)tv_get_string(&var1); 
}
lp->ll_list = NULL;
lp->ll_dict = lp->ll_tv->vval.v_dict;
lp->ll_di = tv_dict_find(lp->ll_dict, (const char *)key, len);
if (rettv != NULL && lp->ll_dict->dv_scope != 0) {
int prevval;
int wrong;
if (len != -1) {
prevval = key[len];
key[len] = NUL;
} else {
prevval = 0; 
}
wrong = ((lp->ll_dict->dv_scope == VAR_DEF_SCOPE
&& tv_is_func(*rettv)
&& !var_check_func_name((const char *)key, lp->ll_di == NULL))
|| !valid_varname((const char *)key));
if (len != -1) {
key[len] = prevval;
}
if (wrong) {
return NULL;
}
}
if (lp->ll_di != NULL && tv_is_luafunc(&lp->ll_di->di_tv)
&& len == -1 && rettv == NULL) {
tv_clear(&var1);
EMSG2(e_illvar, "v:['lua']");
return NULL;
}
if (lp->ll_di == NULL) {
if (lp->ll_dict == &vimvardict
|| &lp->ll_dict->dv_hashtab == get_funccal_args_ht()) {
EMSG2(_(e_illvar), name);
tv_clear(&var1);
return NULL;
}
if (*p == '[' || *p == '.' || unlet) {
if (!quiet) {
emsgf(_(e_dictkey), key);
}
tv_clear(&var1);
return NULL;
}
if (len == -1) {
lp->ll_newkey = vim_strsave(key);
} else {
lp->ll_newkey = vim_strnsave(key, len);
}
tv_clear(&var1);
break;
} else if (!(flags & GLV_READ_ONLY) && var_check_ro(lp->ll_di->di_flags,
(const char *)name,
(size_t)(p - name))) {
tv_clear(&var1);
return NULL;
}
tv_clear(&var1);
lp->ll_tv = &lp->ll_di->di_tv;
} else {
if (empty1) {
lp->ll_n1 = 0;
} else {
lp->ll_n1 = (long)tv_get_number(&var1);
}
tv_clear(&var1);
lp->ll_dict = NULL;
lp->ll_list = lp->ll_tv->vval.v_list;
lp->ll_li = tv_list_find(lp->ll_list, lp->ll_n1);
if (lp->ll_li == NULL) {
if (lp->ll_n1 < 0) {
lp->ll_n1 = 0;
lp->ll_li = tv_list_find(lp->ll_list, lp->ll_n1);
}
}
if (lp->ll_li == NULL) {
tv_clear(&var2);
if (!quiet) {
EMSGN(_(e_listidx), lp->ll_n1);
}
return NULL;
}
if (lp->ll_range && !lp->ll_empty2) {
lp->ll_n2 = (long)tv_get_number(&var2); 
tv_clear(&var2);
if (lp->ll_n2 < 0) {
ni = tv_list_find(lp->ll_list, lp->ll_n2);
if (ni == NULL) {
if (!quiet)
EMSGN(_(e_listidx), lp->ll_n2);
return NULL;
}
lp->ll_n2 = tv_list_idx_of_item(lp->ll_list, ni);
}
if (lp->ll_n1 < 0) {
lp->ll_n1 = tv_list_idx_of_item(lp->ll_list, lp->ll_li);
}
if (lp->ll_n2 < lp->ll_n1) {
if (!quiet) {
EMSGN(_(e_listidx), lp->ll_n2);
}
return NULL;
}
}
lp->ll_tv = TV_LIST_ITEM_TV(lp->ll_li);
}
}
tv_clear(&var1);
return p;
}
void clear_lval(lval_T *lp)
{
xfree(lp->ll_exp_name);
xfree(lp->ll_newkey);
}
static void set_var_lval(lval_T *lp, char_u *endp, typval_T *rettv,
int copy, const bool is_const, const char_u *op)
{
int cc;
listitem_T *ri;
dictitem_T *di;
if (lp->ll_tv == NULL) {
cc = *endp;
*endp = NUL;
if (op != NULL && *op != '=') {
typval_T tv;
if (is_const) {
EMSG(_(e_cannot_mod));
*endp = cc;
return;
}
di = NULL;
if (get_var_tv((const char *)lp->ll_name, (int)STRLEN(lp->ll_name),
&tv, &di, true, false) == OK) {
if ((di == NULL
|| (!var_check_ro(di->di_flags, (const char *)lp->ll_name,
TV_CSTRING)
&& !tv_check_lock(di->di_tv.v_lock, (const char *)lp->ll_name,
TV_CSTRING)))
&& eexe_mod_op(&tv, rettv, (const char *)op) == OK) {
set_var(lp->ll_name, lp->ll_name_len, &tv, false);
}
tv_clear(&tv);
}
} else {
set_var_const(lp->ll_name, lp->ll_name_len, rettv, copy, is_const);
}
*endp = cc;
} else if (tv_check_lock(lp->ll_newkey == NULL
? lp->ll_tv->v_lock
: lp->ll_tv->vval.v_dict->dv_lock,
(const char *)lp->ll_name, TV_CSTRING)) {
} else if (lp->ll_range) {
listitem_T *ll_li = lp->ll_li;
int ll_n1 = lp->ll_n1;
if (is_const) {
EMSG(_("E996: Cannot lock a range"));
return;
}
for (ri = tv_list_first(rettv->vval.v_list);
ri != NULL && ll_li != NULL; ) {
if (tv_check_lock(TV_LIST_ITEM_TV(ll_li)->v_lock,
(const char *)lp->ll_name,
TV_CSTRING)) {
return;
}
ri = TV_LIST_ITEM_NEXT(rettv->vval.v_list, ri);
if (ri == NULL || (!lp->ll_empty2 && lp->ll_n2 == ll_n1)) {
break;
}
ll_li = TV_LIST_ITEM_NEXT(lp->ll_list, ll_li);
ll_n1++;
}
for (ri = tv_list_first(rettv->vval.v_list); ri != NULL; ) {
if (op != NULL && *op != '=') {
eexe_mod_op(TV_LIST_ITEM_TV(lp->ll_li), TV_LIST_ITEM_TV(ri),
(const char *)op);
} else {
tv_clear(TV_LIST_ITEM_TV(lp->ll_li));
tv_copy(TV_LIST_ITEM_TV(ri), TV_LIST_ITEM_TV(lp->ll_li));
}
ri = TV_LIST_ITEM_NEXT(rettv->vval.v_list, ri);
if (ri == NULL || (!lp->ll_empty2 && lp->ll_n2 == lp->ll_n1)) {
break;
}
assert(lp->ll_li != NULL);
if (TV_LIST_ITEM_NEXT(lp->ll_list, lp->ll_li) == NULL) {
tv_list_append_number(lp->ll_list, 0);
lp->ll_li = tv_list_last(lp->ll_list); 
} else {
lp->ll_li = TV_LIST_ITEM_NEXT(lp->ll_list, lp->ll_li);
}
lp->ll_n1++;
}
if (ri != NULL) {
EMSG(_("E710: List value has more items than target"));
} else if (lp->ll_empty2
? (lp->ll_li != NULL
&& TV_LIST_ITEM_NEXT(lp->ll_list, lp->ll_li) != NULL)
: lp->ll_n1 != lp->ll_n2) {
EMSG(_("E711: List value has not enough items"));
}
} else {
typval_T oldtv = TV_INITIAL_VALUE;
dict_T *dict = lp->ll_dict;
bool watched = tv_dict_is_watched(dict);
if (is_const) {
EMSG(_("E996: Cannot lock a list or dict"));
return;
}
if (lp->ll_newkey != NULL) {
if (op != NULL && *op != '=') {
EMSG2(_(e_letwrong), op);
return;
}
di = tv_dict_item_alloc((const char *)lp->ll_newkey);
if (tv_dict_add(lp->ll_tv->vval.v_dict, di) == FAIL) {
xfree(di);
return;
}
lp->ll_tv = &di->di_tv;
} else {
if (watched) {
tv_copy(lp->ll_tv, &oldtv);
}
if (op != NULL && *op != '=') {
eexe_mod_op(lp->ll_tv, rettv, (const char *)op);
goto notify;
} else {
tv_clear(lp->ll_tv);
}
}
if (copy) {
tv_copy(rettv, lp->ll_tv);
} else {
*lp->ll_tv = *rettv;
lp->ll_tv->v_lock = 0;
tv_init(rettv);
}
notify:
if (watched) {
if (oldtv.v_type == VAR_UNKNOWN) {
assert(lp->ll_newkey != NULL);
tv_dict_watcher_notify(dict, (char *)lp->ll_newkey, lp->ll_tv, NULL);
} else {
dictitem_T *di_ = lp->ll_di;
assert(di_->di_key != NULL);
tv_dict_watcher_notify(dict, (char *)di_->di_key, lp->ll_tv, &oldtv);
tv_clear(&oldtv);
}
}
}
}
void *eval_for_line(const char_u *arg, bool *errp, char_u **nextcmdp, int skip)
{
forinfo_T *fi = xcalloc(1, sizeof(forinfo_T));
const char_u *expr;
typval_T tv;
list_T *l;
*errp = true; 
expr = skip_var_list(arg, &fi->fi_varcount, &fi->fi_semicolon);
if (expr == NULL)
return fi;
expr = skipwhite(expr);
if (expr[0] != 'i' || expr[1] != 'n' || !ascii_iswhite(expr[2])) {
EMSG(_("E690: Missing \"in\" after :for"));
return fi;
}
if (skip)
++emsg_skip;
if (eval0(skipwhite(expr + 2), &tv, nextcmdp, !skip) == OK) {
*errp = false;
if (!skip) {
l = tv.vval.v_list;
if (tv.v_type != VAR_LIST) {
EMSG(_(e_listreq));
tv_clear(&tv);
} else if (l == NULL) {
tv_clear(&tv);
} else {
fi->fi_list = l;
tv_list_watch_add(l, &fi->fi_lw);
fi->fi_lw.lw_item = tv_list_first(l);
}
}
}
if (skip)
--emsg_skip;
return fi;
}
bool next_for_item(void *fi_void, char_u *arg)
{
forinfo_T *fi = (forinfo_T *)fi_void;
listitem_T *item = fi->fi_lw.lw_item;
if (item == NULL) {
return false;
} else {
fi->fi_lw.lw_item = TV_LIST_ITEM_NEXT(fi->fi_list, item);
return (ex_let_vars(arg, TV_LIST_ITEM_TV(item), true,
fi->fi_semicolon, fi->fi_varcount, false, NULL) == OK);
}
}
void free_for_info(void *fi_void)
{
forinfo_T *fi = (forinfo_T *)fi_void;
if (fi != NULL && fi->fi_list != NULL) {
tv_list_watch_remove(fi->fi_list, &fi->fi_lw);
tv_list_unref(fi->fi_list);
}
xfree(fi);
}
void set_context_for_expression(expand_T *xp, char_u *arg, cmdidx_T cmdidx)
{
int got_eq = FALSE;
int c;
char_u *p;
if (cmdidx == CMD_let || cmdidx == CMD_const) {
xp->xp_context = EXPAND_USER_VARS;
if (vim_strpbrk(arg, (char_u *)"\"'+-*/%.=!?~|&$([<>,#") == NULL) {
for (p = arg + STRLEN(arg); p >= arg; ) {
xp->xp_pattern = p;
MB_PTR_BACK(arg, p);
if (ascii_iswhite(*p)) {
break;
}
}
return;
}
} else
xp->xp_context = cmdidx == CMD_call ? EXPAND_FUNCTIONS
: EXPAND_EXPRESSION;
while ((xp->xp_pattern = vim_strpbrk(arg,
(char_u *)"\"'+-*/%.=!?~|&$([<>,#")) != NULL) {
c = *xp->xp_pattern;
if (c == '&') {
c = xp->xp_pattern[1];
if (c == '&') {
++xp->xp_pattern;
xp->xp_context = cmdidx != CMD_let || got_eq
? EXPAND_EXPRESSION : EXPAND_NOTHING;
} else if (c != ' ') {
xp->xp_context = EXPAND_SETTINGS;
if ((c == 'l' || c == 'g') && xp->xp_pattern[2] == ':')
xp->xp_pattern += 2;
}
} else if (c == '$') {
xp->xp_context = EXPAND_ENV_VARS;
} else if (c == '=') {
got_eq = TRUE;
xp->xp_context = EXPAND_EXPRESSION;
} else if (c == '#'
&& xp->xp_context == EXPAND_EXPRESSION) {
break;
} else if ((c == '<' || c == '#')
&& xp->xp_context == EXPAND_FUNCTIONS
&& vim_strchr(xp->xp_pattern, '(') == NULL) {
break;
} else if (cmdidx != CMD_let || got_eq) {
if (c == '"') { 
while ((c = *++xp->xp_pattern) != NUL && c != '"')
if (c == '\\' && xp->xp_pattern[1] != NUL)
++xp->xp_pattern;
xp->xp_context = EXPAND_NOTHING;
} else if (c == '\'') { 
while ((c = *++xp->xp_pattern) != NUL && c != '\'')
;
xp->xp_context = EXPAND_NOTHING;
} else if (c == '|') {
if (xp->xp_pattern[1] == '|') {
++xp->xp_pattern;
xp->xp_context = EXPAND_EXPRESSION;
} else
xp->xp_context = EXPAND_COMMANDS;
} else
xp->xp_context = EXPAND_EXPRESSION;
} else
xp->xp_context = EXPAND_EXPRESSION;
arg = xp->xp_pattern;
if (*arg != NUL)
while ((c = *++arg) != NUL && (c == ' ' || c == '\t'))
;
}
xp->xp_pattern = arg;
}
void ex_call(exarg_T *eap)
{
char_u *arg = eap->arg;
char_u *startarg;
char_u *name;
char_u *tofree;
int len;
typval_T rettv;
linenr_T lnum;
int doesrange;
bool failed = false;
funcdict_T fudi;
partial_T *partial = NULL;
if (eap->skip) {
emsg_skip++;
if (eval0(eap->arg, &rettv, &eap->nextcmd, false) != FAIL) {
tv_clear(&rettv);
}
emsg_skip--;
return;
}
tofree = trans_function_name(&arg, false, TFN_INT, &fudi, &partial);
if (fudi.fd_newkey != NULL) {
EMSG2(_(e_dictkey), fudi.fd_newkey);
xfree(fudi.fd_newkey);
}
if (tofree == NULL) {
return;
}
if (fudi.fd_dict != NULL) {
fudi.fd_dict->dv_refcount++;
}
len = (int)STRLEN(tofree);
name = deref_func_name((const char *)tofree, &len,
partial != NULL ? NULL : &partial, false);
startarg = skipwhite(arg);
rettv.v_type = VAR_UNKNOWN; 
if (*startarg != '(') {
EMSG2(_("E107: Missing parentheses: %s"), eap->arg);
goto end;
}
lnum = eap->line1;
for (; lnum <= eap->line2; lnum++) {
if (eap->addr_count > 0) { 
if (lnum > curbuf->b_ml.ml_line_count) {
EMSG(_(e_invrange));
break;
}
curwin->w_cursor.lnum = lnum;
curwin->w_cursor.col = 0;
curwin->w_cursor.coladd = 0;
}
arg = startarg;
if (get_func_tv(name, (int)STRLEN(name), &rettv, &arg,
eap->line1, eap->line2, &doesrange,
true, partial, fudi.fd_dict) == FAIL) {
failed = true;
break;
}
if (handle_subscript((const char **)&arg, &rettv, true, true)
== FAIL) {
failed = true;
break;
}
tv_clear(&rettv);
if (doesrange) {
break;
}
if (aborting()) {
break;
}
}
if (!failed || eap->cstack->cs_trylevel > 0) {
if (!ends_excmd(*arg)) {
emsg_severe = TRUE;
EMSG(_(e_trailing));
} else {
eap->nextcmd = check_nextcmd(arg);
}
}
end:
tv_dict_unref(fudi.fd_dict);
xfree(tofree);
}
void ex_unlet(exarg_T *eap)
{
ex_unletlock(eap, eap->arg, 0);
}
void ex_lockvar(exarg_T *eap)
{
char_u *arg = eap->arg;
int deep = 2;
if (eap->forceit) {
deep = -1;
} else if (ascii_isdigit(*arg)) {
deep = getdigits_int(&arg, false, -1);
arg = skipwhite(arg);
}
ex_unletlock(eap, arg, deep);
}
static void ex_unletlock(exarg_T *eap, char_u *argstart, int deep)
{
char_u *arg = argstart;
bool error = false;
lval_T lv;
do {
if (*arg == '$') {
const char *name = (char *)++arg;
if (get_env_len((const char_u **)&arg) == 0) {
EMSG2(_(e_invarg2), name - 1);
return;
}
os_unsetenv(name);
arg = skipwhite(arg);
continue;
}
char_u *const name_end = (char_u *)get_lval(arg, NULL, &lv, true,
eap->skip || error,
0, FNE_CHECK_START);
if (lv.ll_name == NULL) {
error = true; 
}
if (name_end == NULL || (!ascii_iswhite(*name_end)
&& !ends_excmd(*name_end))) {
if (name_end != NULL) {
emsg_severe = TRUE;
EMSG(_(e_trailing));
}
if (!(eap->skip || error))
clear_lval(&lv);
break;
}
if (!error && !eap->skip) {
if (eap->cmdidx == CMD_unlet) {
if (do_unlet_var(&lv, name_end, eap->forceit) == FAIL)
error = TRUE;
} else {
if (do_lock_var(&lv, name_end, deep,
eap->cmdidx == CMD_lockvar) == FAIL) {
error = true;
}
}
}
if (!eap->skip)
clear_lval(&lv);
arg = skipwhite(name_end);
} while (!ends_excmd(*arg));
eap->nextcmd = check_nextcmd(arg);
}
static int do_unlet_var(lval_T *const lp, char_u *const name_end, int forceit)
{
int ret = OK;
int cc;
if (lp->ll_tv == NULL) {
cc = *name_end;
*name_end = NUL;
if (do_unlet(lp->ll_name, lp->ll_name_len, forceit) == FAIL) {
ret = FAIL;
}
*name_end = cc;
} else if ((lp->ll_list != NULL
&& tv_check_lock(tv_list_locked(lp->ll_list),
(const char *)lp->ll_name,
lp->ll_name_len))
|| (lp->ll_dict != NULL
&& tv_check_lock(lp->ll_dict->dv_lock,
(const char *)lp->ll_name,
lp->ll_name_len))) {
return FAIL;
} else if (lp->ll_range) {
assert(lp->ll_list != NULL);
listitem_T *const first_li = lp->ll_li;
listitem_T *last_li = first_li;
for (;;) {
listitem_T *const li = TV_LIST_ITEM_NEXT(lp->ll_list, lp->ll_li);
if (tv_check_lock(TV_LIST_ITEM_TV(lp->ll_li)->v_lock,
(const char *)lp->ll_name,
lp->ll_name_len)) {
return false;
}
lp->ll_li = li;
lp->ll_n1++;
if (lp->ll_li == NULL || (!lp->ll_empty2 && lp->ll_n2 < lp->ll_n1)) {
break;
} else {
last_li = lp->ll_li;
}
}
tv_list_remove_items(lp->ll_list, first_li, last_li);
} else {
if (lp->ll_list != NULL) {
tv_list_item_remove(lp->ll_list, lp->ll_li);
} else {
dict_T *d = lp->ll_dict;
assert(d != NULL);
dictitem_T *di = lp->ll_di;
bool watched = tv_dict_is_watched(d);
char *key = NULL;
typval_T oldtv;
if (watched) {
tv_copy(&di->di_tv, &oldtv);
key = xstrdup((char *)di->di_key);
}
tv_dict_item_remove(d, di);
if (watched) {
tv_dict_watcher_notify(d, key, NULL, &oldtv);
tv_clear(&oldtv);
xfree(key);
}
}
}
return ret;
}
int do_unlet(const char *const name, const size_t name_len, const int forceit)
FUNC_ATTR_NONNULL_ALL
{
const char *varname;
dict_T *dict;
hashtab_T *ht = find_var_ht_dict(name, name_len, &varname, &dict);
if (ht != NULL && *varname != NUL) {
dict_T *d;
if (ht == &globvarht) {
d = &globvardict;
} else if (current_funccal != NULL
&& ht == &current_funccal->l_vars.dv_hashtab) {
d = &current_funccal->l_vars;
} else if (ht == &compat_hashtab) {
d = &vimvardict;
} else {
dictitem_T *const di = find_var_in_ht(ht, *name, "", 0, false);
d = di->di_tv.vval.v_dict;
}
if (d == NULL) {
internal_error("do_unlet()");
return FAIL;
}
hashitem_T *hi = hash_find(ht, (const char_u *)varname);
if (HASHITEM_EMPTY(hi)) {
hi = find_hi_in_scoped_ht((const char *)name, &ht);
}
if (hi != NULL && !HASHITEM_EMPTY(hi)) {
dictitem_T *const di = TV_DICT_HI2DI(hi);
if (var_check_fixed(di->di_flags, (const char *)name, TV_CSTRING)
|| var_check_ro(di->di_flags, (const char *)name, TV_CSTRING)
|| tv_check_lock(d->dv_lock, (const char *)name, TV_CSTRING)) {
return FAIL;
}
if (tv_check_lock(d->dv_lock, (const char *)name, TV_CSTRING)) {
return FAIL;
}
typval_T oldtv;
bool watched = tv_dict_is_watched(dict);
if (watched) {
tv_copy(&di->di_tv, &oldtv);
}
delete_var(ht, hi);
if (watched) {
tv_dict_watcher_notify(dict, varname, NULL, &oldtv);
tv_clear(&oldtv);
}
return OK;
}
}
if (forceit)
return OK;
EMSG2(_("E108: No such variable: \"%s\""), name);
return FAIL;
}
static int do_lock_var(lval_T *lp, char_u *const name_end, const int deep,
const bool lock)
{
int ret = OK;
if (deep == 0) { 
return OK;
}
if (lp->ll_tv == NULL) {
dictitem_T *const di = find_var(
(const char *)lp->ll_name, lp->ll_name_len, NULL,
true);
if (di == NULL) {
ret = FAIL;
} else if ((di->di_flags & DI_FLAGS_FIX)
&& di->di_tv.v_type != VAR_DICT
&& di->di_tv.v_type != VAR_LIST) {
emsgf(_("E940: Cannot lock or unlock variable %s"), lp->ll_name);
} else {
if (lock) {
di->di_flags |= DI_FLAGS_LOCK;
} else {
di->di_flags &= ~DI_FLAGS_LOCK;
}
tv_item_lock(&di->di_tv, deep, lock);
}
} else if (lp->ll_range) {
listitem_T *li = lp->ll_li;
while (li != NULL && (lp->ll_empty2 || lp->ll_n2 >= lp->ll_n1)) {
tv_item_lock(TV_LIST_ITEM_TV(li), deep, lock);
li = TV_LIST_ITEM_NEXT(lp->ll_list, li);
lp->ll_n1++;
}
} else if (lp->ll_list != NULL) {
tv_item_lock(TV_LIST_ITEM_TV(lp->ll_li), deep, lock);
} else {
tv_item_lock(&lp->ll_di->di_tv, deep, lock);
}
return ret;
}
void del_menutrans_vars(void)
{
hash_lock(&globvarht);
HASHTAB_ITER(&globvarht, hi, {
if (STRNCMP(hi->hi_key, "menutrans_", 10) == 0) {
delete_var(&globvarht, hi);
}
});
hash_unlock(&globvarht);
}
static char_u *varnamebuf = NULL;
static size_t varnamebuflen = 0;
static char_u *cat_prefix_varname(int prefix, char_u *name)
{
size_t len = STRLEN(name) + 3;
if (len > varnamebuflen) {
xfree(varnamebuf);
len += 10; 
varnamebuf = xmalloc(len);
varnamebuflen = len;
}
*varnamebuf = prefix;
varnamebuf[1] = ':';
STRCPY(varnamebuf + 2, name);
return varnamebuf;
}
char_u *get_user_var_name(expand_T *xp, int idx)
{
static size_t gdone;
static size_t bdone;
static size_t wdone;
static size_t tdone;
static size_t vidx;
static hashitem_T *hi;
hashtab_T *ht;
if (idx == 0) {
gdone = bdone = wdone = vidx = 0;
tdone = 0;
}
if (gdone < globvarht.ht_used) {
if (gdone++ == 0)
hi = globvarht.ht_array;
else
++hi;
while (HASHITEM_EMPTY(hi))
++hi;
if (STRNCMP("g:", xp->xp_pattern, 2) == 0)
return cat_prefix_varname('g', hi->hi_key);
return hi->hi_key;
}
ht = &curbuf->b_vars->dv_hashtab;
if (bdone < ht->ht_used) {
if (bdone++ == 0)
hi = ht->ht_array;
else
++hi;
while (HASHITEM_EMPTY(hi))
++hi;
return cat_prefix_varname('b', hi->hi_key);
}
ht = &curwin->w_vars->dv_hashtab;
if (wdone < ht->ht_used) {
if (wdone++ == 0)
hi = ht->ht_array;
else
++hi;
while (HASHITEM_EMPTY(hi))
++hi;
return cat_prefix_varname('w', hi->hi_key);
}
ht = &curtab->tp_vars->dv_hashtab;
if (tdone < ht->ht_used) {
if (tdone++ == 0)
hi = ht->ht_array;
else
++hi;
while (HASHITEM_EMPTY(hi))
++hi;
return cat_prefix_varname('t', hi->hi_key);
}
if (vidx < ARRAY_SIZE(vimvars)) {
return cat_prefix_varname('v', (char_u *)vimvars[vidx++].vv_name);
}
XFREE_CLEAR(varnamebuf);
varnamebuflen = 0;
return NULL;
}
static int pattern_match(char_u *pat, char_u *text, int ic)
{
int matches = 0;
regmatch_T regmatch;
char_u *save_cpo = p_cpo;
p_cpo = (char_u *)"";
regmatch.regprog = vim_regcomp(pat, RE_MAGIC + RE_STRING);
if (regmatch.regprog != NULL) {
regmatch.rm_ic = ic;
matches = vim_regexec_nl(&regmatch, text, (colnr_T)0);
vim_regfree(regmatch.regprog);
}
p_cpo = save_cpo;
return matches;
}
typedef enum {
TYPE_UNKNOWN = 0,
TYPE_EQUAL, 
TYPE_NEQUAL, 
TYPE_GREATER, 
TYPE_GEQUAL, 
TYPE_SMALLER, 
TYPE_SEQUAL, 
TYPE_MATCH, 
TYPE_NOMATCH, 
} exptype_T;
int eval0(char_u *arg, typval_T *rettv, char_u **nextcmd, int evaluate)
{
int ret;
char_u *p;
p = skipwhite(arg);
ret = eval1(&p, rettv, evaluate);
if (ret == FAIL || !ends_excmd(*p)) {
if (ret != FAIL) {
tv_clear(rettv);
}
if (!aborting()) {
emsgf(_(e_invexpr2), arg);
}
ret = FAIL;
}
if (nextcmd != NULL)
*nextcmd = check_nextcmd(p);
return ret;
}
int eval1(char_u **arg, typval_T *rettv, int evaluate)
{
int result;
typval_T var2;
if (eval2(arg, rettv, evaluate) == FAIL)
return FAIL;
if ((*arg)[0] == '?') {
result = FALSE;
if (evaluate) {
bool error = false;
if (tv_get_number_chk(rettv, &error) != 0) {
result = true;
}
tv_clear(rettv);
if (error) {
return FAIL;
}
}
*arg = skipwhite(*arg + 1);
if (eval1(arg, rettv, evaluate && result) == FAIL) 
return FAIL;
if ((*arg)[0] != ':') {
EMSG(_("E109: Missing ':' after '?'"));
if (evaluate && result) {
tv_clear(rettv);
}
return FAIL;
}
*arg = skipwhite(*arg + 1);
if (eval1(arg, &var2, evaluate && !result) == FAIL) { 
if (evaluate && result) {
tv_clear(rettv);
}
return FAIL;
}
if (evaluate && !result)
*rettv = var2;
}
return OK;
}
static int eval2(char_u **arg, typval_T *rettv, int evaluate)
{
typval_T var2;
long result;
int first;
bool error = false;
if (eval3(arg, rettv, evaluate) == FAIL)
return FAIL;
first = TRUE;
result = FALSE;
while ((*arg)[0] == '|' && (*arg)[1] == '|') {
if (evaluate && first) {
if (tv_get_number_chk(rettv, &error) != 0) {
result = true;
}
tv_clear(rettv);
if (error) {
return FAIL;
}
first = false;
}
*arg = skipwhite(*arg + 2);
if (eval3(arg, &var2, evaluate && !result) == FAIL)
return FAIL;
if (evaluate && !result) {
if (tv_get_number_chk(&var2, &error) != 0) {
result = true;
}
tv_clear(&var2);
if (error) {
return FAIL;
}
}
if (evaluate) {
rettv->v_type = VAR_NUMBER;
rettv->vval.v_number = result;
}
}
return OK;
}
static int eval3(char_u **arg, typval_T *rettv, int evaluate)
{
typval_T var2;
long result;
int first;
bool error = false;
if (eval4(arg, rettv, evaluate) == FAIL)
return FAIL;
first = TRUE;
result = TRUE;
while ((*arg)[0] == '&' && (*arg)[1] == '&') {
if (evaluate && first) {
if (tv_get_number_chk(rettv, &error) == 0) {
result = false;
}
tv_clear(rettv);
if (error) {
return FAIL;
}
first = false;
}
*arg = skipwhite(*arg + 2);
if (eval4(arg, &var2, evaluate && result) == FAIL)
return FAIL;
if (evaluate && result) {
if (tv_get_number_chk(&var2, &error) == 0) {
result = false;
}
tv_clear(&var2);
if (error) {
return FAIL;
}
}
if (evaluate) {
rettv->v_type = VAR_NUMBER;
rettv->vval.v_number = result;
}
}
return OK;
}
static int eval4(char_u **arg, typval_T *rettv, int evaluate)
{
typval_T var2;
char_u *p;
int i;
exptype_T type = TYPE_UNKNOWN;
int type_is = FALSE; 
int len = 2;
varnumber_T n1, n2;
int ic;
if (eval5(arg, rettv, evaluate) == FAIL)
return FAIL;
p = *arg;
switch (p[0]) {
case '=': if (p[1] == '=')
type = TYPE_EQUAL;
else if (p[1] == '~')
type = TYPE_MATCH;
break;
case '!': if (p[1] == '=')
type = TYPE_NEQUAL;
else if (p[1] == '~')
type = TYPE_NOMATCH;
break;
case '>': if (p[1] != '=') {
type = TYPE_GREATER;
len = 1;
} else
type = TYPE_GEQUAL;
break;
case '<': if (p[1] != '=') {
type = TYPE_SMALLER;
len = 1;
} else
type = TYPE_SEQUAL;
break;
case 'i': if (p[1] == 's') {
if (p[2] == 'n' && p[3] == 'o' && p[4] == 't') {
len = 5;
}
if (!isalnum(p[len]) && p[len] != '_') {
type = len == 2 ? TYPE_EQUAL : TYPE_NEQUAL;
type_is = TRUE;
}
}
break;
}
if (type != TYPE_UNKNOWN) {
if (p[len] == '?') {
ic = TRUE;
++len;
}
else if (p[len] == '#') {
ic = FALSE;
++len;
}
else
ic = p_ic;
*arg = skipwhite(p + len);
if (eval5(arg, &var2, evaluate) == FAIL) {
tv_clear(rettv);
return FAIL;
}
if (evaluate) {
if (type_is && rettv->v_type != var2.v_type) {
n1 = (type == TYPE_NEQUAL);
} else if (rettv->v_type == VAR_LIST || var2.v_type == VAR_LIST) {
if (type_is) {
n1 = (rettv->v_type == var2.v_type
&& rettv->vval.v_list == var2.vval.v_list);
if (type == TYPE_NEQUAL)
n1 = !n1;
} else if (rettv->v_type != var2.v_type
|| (type != TYPE_EQUAL && type != TYPE_NEQUAL)) {
if (rettv->v_type != var2.v_type) {
EMSG(_("E691: Can only compare List with List"));
} else {
EMSG(_("E692: Invalid operation for List"));
}
tv_clear(rettv);
tv_clear(&var2);
return FAIL;
} else {
n1 = tv_list_equal(rettv->vval.v_list, var2.vval.v_list, ic, false);
if (type == TYPE_NEQUAL) {
n1 = !n1;
}
}
} else if (rettv->v_type == VAR_DICT || var2.v_type == VAR_DICT) {
if (type_is) {
n1 = (rettv->v_type == var2.v_type
&& rettv->vval.v_dict == var2.vval.v_dict);
if (type == TYPE_NEQUAL)
n1 = !n1;
} else if (rettv->v_type != var2.v_type
|| (type != TYPE_EQUAL && type != TYPE_NEQUAL)) {
if (rettv->v_type != var2.v_type)
EMSG(_("E735: Can only compare Dictionary with Dictionary"));
else
EMSG(_("E736: Invalid operation for Dictionary"));
tv_clear(rettv);
tv_clear(&var2);
return FAIL;
} else {
n1 = tv_dict_equal(rettv->vval.v_dict, var2.vval.v_dict,
ic, false);
if (type == TYPE_NEQUAL) {
n1 = !n1;
}
}
} else if (tv_is_func(*rettv) || tv_is_func(var2)) {
if (type != TYPE_EQUAL && type != TYPE_NEQUAL) {
EMSG(_("E694: Invalid operation for Funcrefs"));
tv_clear(rettv);
tv_clear(&var2);
return FAIL;
}
if ((rettv->v_type == VAR_PARTIAL
&& rettv->vval.v_partial == NULL)
|| (var2.v_type == VAR_PARTIAL
&& var2.vval.v_partial == NULL)) {
n1 = false;
} else if (type_is) {
if (rettv->v_type == VAR_FUNC && var2.v_type == VAR_FUNC) {
n1 = tv_equal(rettv, &var2, ic, false);
} else if (rettv->v_type == VAR_PARTIAL
&& var2.v_type == VAR_PARTIAL) {
n1 = (rettv->vval.v_partial == var2.vval.v_partial);
} else {
n1 = false;
}
} else {
n1 = tv_equal(rettv, &var2, ic, false);
}
if (type == TYPE_NEQUAL) {
n1 = !n1;
}
}
else if ((rettv->v_type == VAR_FLOAT || var2.v_type == VAR_FLOAT)
&& type != TYPE_MATCH && type != TYPE_NOMATCH) {
float_T f1, f2;
if (rettv->v_type == VAR_FLOAT) {
f1 = rettv->vval.v_float;
} else {
f1 = tv_get_number(rettv);
}
if (var2.v_type == VAR_FLOAT) {
f2 = var2.vval.v_float;
} else {
f2 = tv_get_number(&var2);
}
n1 = false;
switch (type) {
case TYPE_EQUAL: n1 = (f1 == f2); break;
case TYPE_NEQUAL: n1 = (f1 != f2); break;
case TYPE_GREATER: n1 = (f1 > f2); break;
case TYPE_GEQUAL: n1 = (f1 >= f2); break;
case TYPE_SMALLER: n1 = (f1 < f2); break;
case TYPE_SEQUAL: n1 = (f1 <= f2); break;
case TYPE_UNKNOWN:
case TYPE_MATCH:
case TYPE_NOMATCH: break;
}
}
else if ((rettv->v_type == VAR_NUMBER || var2.v_type == VAR_NUMBER)
&& type != TYPE_MATCH && type != TYPE_NOMATCH) {
n1 = tv_get_number(rettv);
n2 = tv_get_number(&var2);
switch (type) {
case TYPE_EQUAL: n1 = (n1 == n2); break;
case TYPE_NEQUAL: n1 = (n1 != n2); break;
case TYPE_GREATER: n1 = (n1 > n2); break;
case TYPE_GEQUAL: n1 = (n1 >= n2); break;
case TYPE_SMALLER: n1 = (n1 < n2); break;
case TYPE_SEQUAL: n1 = (n1 <= n2); break;
case TYPE_UNKNOWN:
case TYPE_MATCH:
case TYPE_NOMATCH: break;
}
} else {
char buf1[NUMBUFLEN];
char buf2[NUMBUFLEN];
const char *const s1 = tv_get_string_buf(rettv, buf1);
const char *const s2 = tv_get_string_buf(&var2, buf2);
if (type != TYPE_MATCH && type != TYPE_NOMATCH) {
i = mb_strcmp_ic((bool)ic, s1, s2);
} else {
i = 0;
}
n1 = false;
switch (type) {
case TYPE_EQUAL: n1 = (i == 0); break;
case TYPE_NEQUAL: n1 = (i != 0); break;
case TYPE_GREATER: n1 = (i > 0); break;
case TYPE_GEQUAL: n1 = (i >= 0); break;
case TYPE_SMALLER: n1 = (i < 0); break;
case TYPE_SEQUAL: n1 = (i <= 0); break;
case TYPE_MATCH:
case TYPE_NOMATCH: {
n1 = pattern_match((char_u *)s2, (char_u *)s1, ic);
if (type == TYPE_NOMATCH) {
n1 = !n1;
}
break;
}
case TYPE_UNKNOWN: break; 
}
}
tv_clear(rettv);
tv_clear(&var2);
rettv->v_type = VAR_NUMBER;
rettv->vval.v_number = n1;
}
}
return OK;
}
static int eval5(char_u **arg, typval_T *rettv, int evaluate)
{
typval_T var2;
typval_T var3;
int op;
varnumber_T n1, n2;
float_T f1 = 0, f2 = 0;
char_u *p;
if (eval6(arg, rettv, evaluate, FALSE) == FAIL)
return FAIL;
for (;; ) {
op = **arg;
if (op != '+' && op != '-' && op != '.')
break;
if ((op != '+' || rettv->v_type != VAR_LIST)
&& (op == '.' || rettv->v_type != VAR_FLOAT)) {
if (evaluate && !tv_check_str(rettv)) {
tv_clear(rettv);
return FAIL;
}
}
if (op == '.' && *(*arg + 1) == '.') { 
(*arg)++;
}
*arg = skipwhite(*arg + 1);
if (eval6(arg, &var2, evaluate, op == '.') == FAIL) {
tv_clear(rettv);
return FAIL;
}
if (evaluate) {
if (op == '.') {
char buf1[NUMBUFLEN];
char buf2[NUMBUFLEN];
const char *const s1 = tv_get_string_buf(rettv, buf1);
const char *const s2 = tv_get_string_buf_chk(&var2, buf2);
if (s2 == NULL) { 
tv_clear(rettv);
tv_clear(&var2);
return FAIL;
}
p = concat_str((const char_u *)s1, (const char_u *)s2);
tv_clear(rettv);
rettv->v_type = VAR_STRING;
rettv->vval.v_string = p;
} else if (op == '+' && rettv->v_type == VAR_LIST
&& var2.v_type == VAR_LIST) {
if (tv_list_concat(rettv->vval.v_list, var2.vval.v_list, &var3)
== FAIL) {
tv_clear(rettv);
tv_clear(&var2);
return FAIL;
}
tv_clear(rettv);
*rettv = var3;
} else {
bool error = false;
if (rettv->v_type == VAR_FLOAT) {
f1 = rettv->vval.v_float;
n1 = 0;
} else {
n1 = tv_get_number_chk(rettv, &error);
if (error) {
tv_clear(rettv);
return FAIL;
}
if (var2.v_type == VAR_FLOAT)
f1 = n1;
}
if (var2.v_type == VAR_FLOAT) {
f2 = var2.vval.v_float;
n2 = 0;
} else {
n2 = tv_get_number_chk(&var2, &error);
if (error) {
tv_clear(rettv);
tv_clear(&var2);
return FAIL;
}
if (rettv->v_type == VAR_FLOAT)
f2 = n2;
}
tv_clear(rettv);
if (rettv->v_type == VAR_FLOAT || var2.v_type == VAR_FLOAT) {
if (op == '+')
f1 = f1 + f2;
else
f1 = f1 - f2;
rettv->v_type = VAR_FLOAT;
rettv->vval.v_float = f1;
} else {
if (op == '+')
n1 = n1 + n2;
else
n1 = n1 - n2;
rettv->v_type = VAR_NUMBER;
rettv->vval.v_number = n1;
}
}
tv_clear(&var2);
}
}
return OK;
}
static int eval6(char_u **arg, typval_T *rettv, int evaluate, int want_string)
FUNC_ATTR_NO_SANITIZE_UNDEFINED
{
typval_T var2;
int op;
varnumber_T n1, n2;
bool use_float = false;
float_T f1 = 0, f2 = 0;
bool error = false;
if (eval7(arg, rettv, evaluate, want_string) == FAIL)
return FAIL;
for (;; ) {
op = **arg;
if (op != '*' && op != '/' && op != '%')
break;
if (evaluate) {
if (rettv->v_type == VAR_FLOAT) {
f1 = rettv->vval.v_float;
use_float = true;
n1 = 0;
} else {
n1 = tv_get_number_chk(rettv, &error);
}
tv_clear(rettv);
if (error) {
return FAIL;
}
} else {
n1 = 0;
}
*arg = skipwhite(*arg + 1);
if (eval7(arg, &var2, evaluate, FALSE) == FAIL)
return FAIL;
if (evaluate) {
if (var2.v_type == VAR_FLOAT) {
if (!use_float) {
f1 = n1;
use_float = true;
}
f2 = var2.vval.v_float;
n2 = 0;
} else {
n2 = tv_get_number_chk(&var2, &error);
tv_clear(&var2);
if (error) {
return FAIL;
}
if (use_float) {
f2 = n2;
}
}
if (use_float) {
if (op == '*') {
f1 = f1 * f2;
} else if (op == '/') {
f1 = (f2 == 0
? (
#if defined(NAN)
f1 == 0
? NAN
:
#endif
(f1 > 0
? INFINITY
: -INFINITY)
)
: f1 / f2);
} else {
EMSG(_("E804: Cannot use '%' with Float"));
return FAIL;
}
rettv->v_type = VAR_FLOAT;
rettv->vval.v_float = f1;
} else {
if (op == '*') {
n1 = n1 * n2;
} else if (op == '/') {
n1 = num_divide(n1, n2);
} else {
n1 = num_modulus(n1, n2);
}
rettv->v_type = VAR_NUMBER;
rettv->vval.v_number = n1;
}
}
}
return OK;
}
static int eval7(
char_u **arg,
typval_T *rettv,
int evaluate,
int want_string 
)
{
varnumber_T n;
int len;
char_u *s;
char_u *start_leader, *end_leader;
int ret = OK;
char_u *alias;
rettv->v_type = VAR_UNKNOWN;
start_leader = *arg;
while (**arg == '!' || **arg == '-' || **arg == '+') {
*arg = skipwhite(*arg + 1);
}
end_leader = *arg;
switch (**arg) {
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
{
char_u *p = skipdigits(*arg + 1);
int get_float = false;
if (!want_string && p[0] == '.' && ascii_isdigit(p[1])) {
get_float = true;
p = skipdigits(p + 2);
if (*p == 'e' || *p == 'E') {
++p;
if (*p == '-' || *p == '+') {
++p;
}
if (!ascii_isdigit(*p)) {
get_float = false;
} else {
p = skipdigits(p + 1);
}
}
if (ASCII_ISALPHA(*p) || *p == '.') {
get_float = false;
}
}
if (get_float) {
float_T f;
*arg += string2float((char *) *arg, &f);
if (evaluate) {
rettv->v_type = VAR_FLOAT;
rettv->vval.v_float = f;
}
} else {
vim_str2nr(*arg, NULL, &len, STR2NR_ALL, &n, NULL, 0);
*arg += len;
if (evaluate) {
rettv->v_type = VAR_NUMBER;
rettv->vval.v_number = n;
}
}
break;
}
case '"': ret = get_string_tv(arg, rettv, evaluate);
break;
case '\'': ret = get_lit_string_tv(arg, rettv, evaluate);
break;
case '[': ret = get_list_tv(arg, rettv, evaluate);
break;
case '{': ret = get_lambda_tv(arg, rettv, evaluate);
if (ret == NOTDONE) {
ret = dict_get_tv(arg, rettv, evaluate);
}
break;
case '&': {
ret = get_option_tv((const char **)arg, rettv, evaluate);
break;
}
case '$': ret = get_env_tv(arg, rettv, evaluate);
break;
case '@': ++*arg;
if (evaluate) {
rettv->v_type = VAR_STRING;
rettv->vval.v_string = get_reg_contents(**arg, kGRegExprSrc);
}
if (**arg != NUL) {
++*arg;
}
break;
case '(': *arg = skipwhite(*arg + 1);
ret = eval1(arg, rettv, evaluate); 
if (**arg == ')') {
++*arg;
} else if (ret == OK) {
EMSG(_("E110: Missing ')'"));
tv_clear(rettv);
ret = FAIL;
}
break;
default: ret = NOTDONE;
break;
}
if (ret == NOTDONE) {
s = *arg;
len = get_name_len((const char **)arg, (char **)&alias, evaluate, true);
if (alias != NULL) {
s = alias;
}
if (len <= 0) {
ret = FAIL;
} else {
if (**arg == '(') { 
partial_T *partial;
if (!evaluate) {
check_vars((const char *)s, len);
}
s = deref_func_name((const char *)s, &len, &partial, !evaluate);
s = xmemdupz(s, len);
ret = get_func_tv(s, len, rettv, arg,
curwin->w_cursor.lnum, curwin->w_cursor.lnum,
&len, evaluate, partial, NULL);
xfree(s);
if (rettv->v_type == VAR_UNKNOWN && !evaluate && **arg == '(') {
rettv->vval.v_string = (char_u *)tv_empty_string;
rettv->v_type = VAR_FUNC;
}
if (evaluate && aborting()) {
if (ret == OK) {
tv_clear(rettv);
}
ret = FAIL;
}
} else if (evaluate) {
ret = get_var_tv((const char *)s, len, rettv, NULL, true, false);
} else {
check_vars((const char *)s, len);
ret = OK;
}
}
xfree(alias);
}
*arg = skipwhite(*arg);
if (ret == OK) {
ret = handle_subscript((const char **)arg, rettv, evaluate, true);
}
if (ret == OK && evaluate && end_leader > start_leader) {
bool error = false;
varnumber_T val = 0;
float_T f = 0.0;
if (rettv->v_type == VAR_FLOAT) {
f = rettv->vval.v_float;
} else {
val = tv_get_number_chk(rettv, &error);
}
if (error) {
tv_clear(rettv);
ret = FAIL;
} else {
while (end_leader > start_leader) {
--end_leader;
if (*end_leader == '!') {
if (rettv->v_type == VAR_FLOAT) {
f = !f;
} else {
val = !val;
}
} else if (*end_leader == '-') {
if (rettv->v_type == VAR_FLOAT) {
f = -f;
} else {
val = -val;
}
}
}
if (rettv->v_type == VAR_FLOAT) {
tv_clear(rettv);
rettv->vval.v_float = f;
} else {
tv_clear(rettv);
rettv->v_type = VAR_NUMBER;
rettv->vval.v_number = val;
}
}
}
return ret;
}
static int
eval_index(
char_u **arg,
typval_T *rettv,
int evaluate,
int verbose 
)
{
bool empty1 = false;
bool empty2 = false;
long n1, n2 = 0;
ptrdiff_t len = -1;
int range = false;
char_u *key = NULL;
switch (rettv->v_type) {
case VAR_FUNC:
case VAR_PARTIAL: {
if (verbose) {
EMSG(_("E695: Cannot index a Funcref"));
}
return FAIL;
}
case VAR_FLOAT: {
if (verbose) {
EMSG(_(e_float_as_string));
}
return FAIL;
}
case VAR_SPECIAL: {
if (verbose) {
EMSG(_("E909: Cannot index a special variable"));
}
return FAIL;
}
case VAR_UNKNOWN: {
if (evaluate) {
return FAIL;
}
FALLTHROUGH;
}
case VAR_STRING:
case VAR_NUMBER:
case VAR_LIST:
case VAR_DICT: {
break;
}
}
typval_T var1 = TV_INITIAL_VALUE;
typval_T var2 = TV_INITIAL_VALUE;
if (**arg == '.') {
key = *arg + 1;
for (len = 0; ASCII_ISALNUM(key[len]) || key[len] == '_'; ++len)
;
if (len == 0)
return FAIL;
*arg = skipwhite(key + len);
} else {
*arg = skipwhite(*arg + 1);
if (**arg == ':') {
empty1 = true;
} else if (eval1(arg, &var1, evaluate) == FAIL) { 
return FAIL;
} else if (evaluate && !tv_check_str(&var1)) {
tv_clear(&var1);
return FAIL;
}
if (**arg == ':') {
range = TRUE;
*arg = skipwhite(*arg + 1);
if (**arg == ']') {
empty2 = true;
} else if (eval1(arg, &var2, evaluate) == FAIL) { 
if (!empty1) {
tv_clear(&var1);
}
return FAIL;
} else if (evaluate && !tv_check_str(&var2)) {
if (!empty1) {
tv_clear(&var1);
}
tv_clear(&var2);
return FAIL;
}
}
if (**arg != ']') {
if (verbose) {
EMSG(_(e_missbrac));
}
tv_clear(&var1);
if (range) {
tv_clear(&var2);
}
return FAIL;
}
*arg = skipwhite(*arg + 1); 
}
if (evaluate) {
n1 = 0;
if (!empty1 && rettv->v_type != VAR_DICT && !tv_is_luafunc(rettv)) {
n1 = tv_get_number(&var1);
tv_clear(&var1);
}
if (range) {
if (empty2) {
n2 = -1;
} else {
n2 = tv_get_number(&var2);
tv_clear(&var2);
}
}
switch (rettv->v_type) {
case VAR_NUMBER:
case VAR_STRING: {
const char *const s = tv_get_string(rettv);
char *v;
len = (ptrdiff_t)strlen(s);
if (range) {
if (n1 < 0) {
n1 = len + n1;
if (n1 < 0) {
n1 = 0;
}
}
if (n2 < 0) {
n2 = len + n2;
} else if (n2 >= len) {
n2 = len;
}
if (n1 >= len || n2 < 0 || n1 > n2) {
v = NULL;
} else {
v = xmemdupz(s + n1, (size_t)(n2 - n1 + 1));
}
} else {
if (n1 >= len || n1 < 0) {
v = NULL;
} else {
v = xmemdupz(s + n1, 1);
}
}
tv_clear(rettv);
rettv->v_type = VAR_STRING;
rettv->vval.v_string = (char_u *)v;
break;
}
case VAR_LIST: {
len = tv_list_len(rettv->vval.v_list);
if (n1 < 0) {
n1 = len + n1;
}
if (!empty1 && (n1 < 0 || n1 >= len)) {
if (!range) {
if (verbose) {
EMSGN(_(e_listidx), n1);
}
return FAIL;
}
n1 = len;
}
if (range) {
list_T *l;
listitem_T *item;
if (n2 < 0) {
n2 = len + n2;
} else if (n2 >= len) {
n2 = len - 1;
}
if (!empty2 && (n2 < 0 || n2 + 1 < n1)) {
n2 = -1;
}
l = tv_list_alloc(n2 - n1 + 1);
item = tv_list_find(rettv->vval.v_list, n1);
while (n1++ <= n2) {
tv_list_append_tv(l, TV_LIST_ITEM_TV(item));
item = TV_LIST_ITEM_NEXT(rettv->vval.v_list, item);
}
tv_clear(rettv);
tv_list_set_ret(rettv, l);
} else {
tv_copy(TV_LIST_ITEM_TV(tv_list_find(rettv->vval.v_list, n1)), &var1);
tv_clear(rettv);
*rettv = var1;
}
break;
}
case VAR_DICT: {
if (range) {
if (verbose) {
EMSG(_(e_dictrange));
}
if (len == -1) {
tv_clear(&var1);
}
return FAIL;
}
if (len == -1) {
key = (char_u *)tv_get_string_chk(&var1);
if (key == NULL) {
tv_clear(&var1);
return FAIL;
}
}
dictitem_T *const item = tv_dict_find(rettv->vval.v_dict,
(const char *)key, len);
if (item == NULL && verbose) {
emsgf(_(e_dictkey), key);
}
if (len == -1) {
tv_clear(&var1);
}
if (item == NULL || tv_is_luafunc(&item->di_tv)) {
return FAIL;
}
tv_copy(&item->di_tv, &var1);
tv_clear(rettv);
*rettv = var1;
break;
}
case VAR_SPECIAL:
case VAR_FUNC:
case VAR_FLOAT:
case VAR_PARTIAL:
case VAR_UNKNOWN: {
break; 
}
}
}
return OK;
}
int get_option_tv(const char **const arg, typval_T *const rettv,
const bool evaluate)
FUNC_ATTR_NONNULL_ARG(1)
{
long numval;
char_u *stringval;
int opt_type;
int c;
bool working = (**arg == '+'); 
int ret = OK;
int opt_flags;
char *option_end = (char *)find_option_end(arg, &opt_flags);
if (option_end == NULL) {
if (rettv != NULL) {
EMSG2(_("E112: Option name missing: %s"), *arg);
}
return FAIL;
}
if (!evaluate) {
*arg = option_end;
return OK;
}
c = *option_end;
*option_end = NUL;
opt_type = get_option_value((char_u *)(*arg), &numval,
rettv == NULL ? NULL : &stringval, opt_flags);
if (opt_type == -3) { 
if (rettv != NULL)
EMSG2(_("E113: Unknown option: %s"), *arg);
ret = FAIL;
} else if (rettv != NULL) {
if (opt_type == -2) { 
rettv->v_type = VAR_STRING;
rettv->vval.v_string = NULL;
} else if (opt_type == -1) { 
rettv->v_type = VAR_NUMBER;
rettv->vval.v_number = 0;
} else if (opt_type == 1) { 
rettv->v_type = VAR_NUMBER;
rettv->vval.v_number = numval;
} else { 
rettv->v_type = VAR_STRING;
rettv->vval.v_string = stringval;
}
} else if (working && (opt_type == -2 || opt_type == -1))
ret = FAIL;
*option_end = c; 
*arg = option_end;
return ret;
}
static int get_string_tv(char_u **arg, typval_T *rettv, int evaluate)
{
char_u *p;
char_u *name;
unsigned int extra = 0;
for (p = *arg + 1; *p != NUL && *p != '"'; MB_PTR_ADV(p)) {
if (*p == '\\' && p[1] != NUL) {
++p;
if (*p == '<')
extra += 2;
}
}
if (*p != '"') {
EMSG2(_("E114: Missing quote: %s"), *arg);
return FAIL;
}
if (!evaluate) {
*arg = p + 1;
return OK;
}
name = xmalloc(p - *arg + extra);
rettv->v_type = VAR_STRING;
rettv->vval.v_string = name;
for (p = *arg + 1; *p != NUL && *p != '"'; ) {
if (*p == '\\') {
switch (*++p) {
case 'b': *name++ = BS; ++p; break;
case 'e': *name++ = ESC; ++p; break;
case 'f': *name++ = FF; ++p; break;
case 'n': *name++ = NL; ++p; break;
case 'r': *name++ = CAR; ++p; break;
case 't': *name++ = TAB; ++p; break;
case 'X': 
case 'x':
case 'u': 
case 'U':
if (ascii_isxdigit(p[1])) {
int n, nr;
int c = toupper(*p);
if (c == 'X') {
n = 2;
} else if (*p == 'u') {
n = 4;
} else {
n = 8;
}
nr = 0;
while (--n >= 0 && ascii_isxdigit(p[1])) {
++p;
nr = (nr << 4) + hex2nr(*p);
}
++p;
if (c != 'X') {
name += utf_char2bytes(nr, name);
} else {
*name++ = nr;
}
}
break;
case '0':
case '1':
case '2':
case '3':
case '4':
case '5':
case '6':
case '7': *name = *p++ - '0';
if (*p >= '0' && *p <= '7') {
*name = (*name << 3) + *p++ - '0';
if (*p >= '0' && *p <= '7')
*name = (*name << 3) + *p++ - '0';
}
++name;
break;
case '<':
extra = trans_special((const char_u **)&p, STRLEN(p), name, true, true);
if (extra != 0) {
name += extra;
break;
}
FALLTHROUGH;
default: MB_COPY_CHAR(p, name);
break;
}
} else
MB_COPY_CHAR(p, name);
}
*name = NUL;
if (*p != NUL) { 
p++;
}
*arg = p;
return OK;
}
static int get_lit_string_tv(char_u **arg, typval_T *rettv, int evaluate)
{
char_u *p;
char_u *str;
int reduce = 0;
for (p = *arg + 1; *p != NUL; MB_PTR_ADV(p)) {
if (*p == '\'') {
if (p[1] != '\'')
break;
++reduce;
++p;
}
}
if (*p != '\'') {
EMSG2(_("E115: Missing quote: %s"), *arg);
return FAIL;
}
if (!evaluate) {
*arg = p + 1;
return OK;
}
str = xmalloc((p - *arg) - reduce);
rettv->v_type = VAR_STRING;
rettv->vval.v_string = str;
for (p = *arg + 1; *p != NUL; ) {
if (*p == '\'') {
if (p[1] != '\'')
break;
++p;
}
MB_COPY_CHAR(p, str);
}
*str = NUL;
*arg = p + 1;
return OK;
}
char_u *partial_name(partial_T *pt)
{
if (pt->pt_name != NULL) {
return pt->pt_name;
}
return pt->pt_func->uf_name;
}
static void partial_free(partial_T *pt)
{
for (int i = 0; i < pt->pt_argc; i++) {
tv_clear(&pt->pt_argv[i]);
}
xfree(pt->pt_argv);
tv_dict_unref(pt->pt_dict);
if (pt->pt_name != NULL) {
func_unref(pt->pt_name);
xfree(pt->pt_name);
} else {
func_ptr_unref(pt->pt_func);
}
xfree(pt);
}
void partial_unref(partial_T *pt)
{
if (pt != NULL && --pt->pt_refcount <= 0) {
partial_free(pt);
}
}
static int get_list_tv(char_u **arg, typval_T *rettv, int evaluate)
{
list_T *l = NULL;
if (evaluate) {
l = tv_list_alloc(kListLenShouldKnow);
}
*arg = skipwhite(*arg + 1);
while (**arg != ']' && **arg != NUL) {
typval_T tv;
if (eval1(arg, &tv, evaluate) == FAIL) { 
goto failret;
}
if (evaluate) {
tv.v_lock = VAR_UNLOCKED;
tv_list_append_owned_tv(l, tv);
}
if (**arg == ']') {
break;
}
if (**arg != ',') {
emsgf(_("E696: Missing comma in List: %s"), *arg);
goto failret;
}
*arg = skipwhite(*arg + 1);
}
if (**arg != ']') {
emsgf(_("E697: Missing end of List ']': %s"), *arg);
failret:
if (evaluate) {
tv_list_free(l);
}
return FAIL;
}
*arg = skipwhite(*arg + 1);
if (evaluate) {
tv_list_set_ret(rettv, l);
}
return OK;
}
bool func_equal(
typval_T *tv1,
typval_T *tv2,
bool ic 
) {
char_u *s1, *s2;
dict_T *d1, *d2;
int a1, a2;
s1 = tv1->v_type == VAR_FUNC ? tv1->vval.v_string
: partial_name(tv1->vval.v_partial);
if (s1 != NULL && *s1 == NUL) {
s1 = NULL;
}
s2 = tv2->v_type == VAR_FUNC ? tv2->vval.v_string
: partial_name(tv2->vval.v_partial);
if (s2 != NULL && *s2 == NUL) {
s2 = NULL;
}
if (s1 == NULL || s2 == NULL) {
if (s1 != s2) {
return false;
}
} else if (STRCMP(s1, s2) != 0) {
return false;
}
d1 = tv1->v_type == VAR_FUNC ? NULL : tv1->vval.v_partial->pt_dict;
d2 = tv2->v_type == VAR_FUNC ? NULL : tv2->vval.v_partial->pt_dict;
if (d1 == NULL || d2 == NULL) {
if (d1 != d2) {
return false;
}
} else if (!tv_dict_equal(d1, d2, ic, true)) {
return false;
}
a1 = tv1->v_type == VAR_FUNC ? 0 : tv1->vval.v_partial->pt_argc;
a2 = tv2->v_type == VAR_FUNC ? 0 : tv2->vval.v_partial->pt_argc;
if (a1 != a2) {
return false;
}
for (int i = 0; i < a1; i++) {
if (!tv_equal(tv1->vval.v_partial->pt_argv + i,
tv2->vval.v_partial->pt_argv + i, ic, true)) {
return false;
}
}
return true;
}
int get_copyID(void)
FUNC_ATTR_WARN_UNUSED_RESULT
{
static int current_copyID = 0;
current_copyID += COPYID_INC;
return current_copyID;
}
static garray_T funcargs = GA_EMPTY_INIT_VALUE;
bool garbage_collect(bool testing)
{
bool abort = false;
#define ABORTING(func) abort = abort || func
if (!testing) {
want_garbage_collect = false;
may_garbage_collect = false;
garbage_collect_at_exit = false;
}
const int copyID = get_copyID();
for (funccall_T *fc = previous_funccal; fc != NULL; fc = fc->caller) {
fc->fc_copyID = copyID + 1;
ABORTING(set_ref_in_ht)(&fc->l_vars.dv_hashtab, copyID + 1, NULL);
ABORTING(set_ref_in_ht)(&fc->l_avars.dv_hashtab, copyID + 1, NULL);
}
for (int i = 1; i <= ga_scripts.ga_len; ++i) {
ABORTING(set_ref_in_ht)(&SCRIPT_VARS(i), copyID, NULL);
}
FOR_ALL_BUFFERS(buf) {
ABORTING(set_ref_in_item)(&buf->b_bufvar.di_tv, copyID, NULL, NULL);
ABORTING(set_ref_in_fmark)(buf->b_last_cursor, copyID);
ABORTING(set_ref_in_fmark)(buf->b_last_insert, copyID);
ABORTING(set_ref_in_fmark)(buf->b_last_change, copyID);
for (size_t i = 0; i < NMARKS; i++) {
ABORTING(set_ref_in_fmark)(buf->b_namedm[i], copyID);
}
for (int i = 0; i < buf->b_changelistlen; i++) {
ABORTING(set_ref_in_fmark)(buf->b_changelist[i], copyID);
}
ABORTING(set_ref_dict)(buf->additional_data, copyID);
set_ref_in_callback(&buf->b_prompt_callback, copyID, NULL, NULL);
set_ref_in_callback(&buf->b_prompt_interrupt, copyID, NULL, NULL);
}
FOR_ALL_TAB_WINDOWS(tp, wp) {
ABORTING(set_ref_in_item)(&wp->w_winvar.di_tv, copyID, NULL, NULL);
for (int i = 0; i < wp->w_jumplistlen; i++) {
ABORTING(set_ref_in_fmark)(wp->w_jumplist[i].fmark, copyID);
}
}
if (aucmd_win != NULL) {
ABORTING(set_ref_in_item)(&aucmd_win->w_winvar.di_tv, copyID, NULL, NULL);
}
{
const void *reg_iter = NULL;
do {
yankreg_T reg;
char name = NUL;
bool is_unnamed = false;
reg_iter = op_global_reg_iter(reg_iter, &name, &reg, &is_unnamed);
if (name != NUL) {
ABORTING(set_ref_dict)(reg.additional_data, copyID);
}
} while (reg_iter != NULL);
}
{
const void *mark_iter = NULL;
do {
xfmark_T fm;
char name = NUL;
mark_iter = mark_global_iter(mark_iter, &name, &fm);
if (name != NUL) {
ABORTING(set_ref_dict)(fm.fmark.additional_data, copyID);
}
} while (mark_iter != NULL);
}
FOR_ALL_TABS(tp) {
ABORTING(set_ref_in_item)(&tp->tp_winvar.di_tv, copyID, NULL, NULL);
}
ABORTING(set_ref_in_ht)(&globvarht, copyID, NULL);
for (funccall_T *fc = current_funccal; fc != NULL; fc = fc->caller) {
fc->fc_copyID = copyID;
ABORTING(set_ref_in_ht)(&fc->l_vars.dv_hashtab, copyID, NULL);
ABORTING(set_ref_in_ht)(&fc->l_avars.dv_hashtab, copyID, NULL);
}
ABORTING(set_ref_in_functions(copyID));
{
Channel *data;
map_foreach_value(channels, data, {
set_ref_in_callback_reader(&data->on_data, copyID, NULL, NULL);
set_ref_in_callback_reader(&data->on_stderr, copyID, NULL, NULL);
set_ref_in_callback(&data->on_exit, copyID, NULL, NULL);
})
}
{
timer_T *timer;
map_foreach_value(timers, timer, {
set_ref_in_callback(&timer->callback, copyID, NULL, NULL);
})
}
for (int i = 0; i < funcargs.ga_len; i++) {
ABORTING(set_ref_in_item)(((typval_T **)funcargs.ga_data)[i],
copyID, NULL, NULL);
}
ABORTING(set_ref_in_ht)(&vimvarht, copyID, NULL);
if (p_hi) {
for (uint8_t i = 0; i < HIST_COUNT; i++) {
const void *iter = NULL;
do {
histentry_T hist;
iter = hist_iter(iter, i, false, &hist);
if (hist.hisstr != NULL) {
ABORTING(set_ref_list)(hist.additional_elements, copyID);
}
} while (iter != NULL);
}
}
{
SearchPattern pat;
get_search_pattern(&pat);
ABORTING(set_ref_dict)(pat.additional_data, copyID);
get_substitute_pattern(&pat);
ABORTING(set_ref_dict)(pat.additional_data, copyID);
}
{
SubReplacementString sub;
sub_get_replacement(&sub);
ABORTING(set_ref_list)(sub.additional_elements, copyID);
}
ABORTING(set_ref_in_quickfix)(copyID);
bool did_free = false;
if (!abort) {
did_free = free_unref_items(copyID);
bool did_free_funccal = false;
for (funccall_T **pfc = &previous_funccal; *pfc != NULL;) {
if (can_free_funccal(*pfc, copyID)) {
funccall_T *fc = *pfc;
*pfc = fc->caller;
free_funccal(fc, true);
did_free = true;
did_free_funccal = true;
} else {
pfc = &(*pfc)->caller;
}
}
if (did_free_funccal) {
(void)garbage_collect(testing);
}
} else if (p_verbose > 0) {
verb_msg(_(
"Not enough memory to set references, garbage collection aborted!"));
}
#undef ABORTING
return did_free;
}
static int free_unref_items(int copyID)
{
bool did_free = false;
tv_in_free_unref_items = true;
for (dict_T *dd = gc_first_dict; dd != NULL; dd = dd->dv_used_next) {
if ((dd->dv_copyID & COPYID_MASK) != (copyID & COPYID_MASK)) {
tv_dict_free_contents(dd);
did_free = true;
}
}
for (list_T *ll = gc_first_list; ll != NULL; ll = ll->lv_used_next) {
if ((tv_list_copyid(ll) & COPYID_MASK) != (copyID & COPYID_MASK)
&& !tv_list_has_watchers(ll)) {
tv_list_free_contents(ll);
did_free = true;
}
}
dict_T *dd_next;
for (dict_T *dd = gc_first_dict; dd != NULL; dd = dd_next) {
dd_next = dd->dv_used_next;
if ((dd->dv_copyID & COPYID_MASK) != (copyID & COPYID_MASK)) {
tv_dict_free_dict(dd);
}
}
list_T *ll_next;
for (list_T *ll = gc_first_list; ll != NULL; ll = ll_next) {
ll_next = ll->lv_used_next;
if ((ll->lv_copyID & COPYID_MASK) != (copyID & COPYID_MASK)
&& !tv_list_has_watchers(ll)) {
tv_list_free_list(ll);
}
}
tv_in_free_unref_items = false;
return did_free;
}
bool set_ref_in_ht(hashtab_T *ht, int copyID, list_stack_T **list_stack)
FUNC_ATTR_WARN_UNUSED_RESULT
{
bool abort = false;
ht_stack_T *ht_stack = NULL;
hashtab_T *cur_ht = ht;
for (;;) {
if (!abort) {
HASHTAB_ITER(cur_ht, hi, {
abort = abort || set_ref_in_item(
&TV_DICT_HI2DI(hi)->di_tv, copyID, &ht_stack, list_stack);
});
}
if (ht_stack == NULL) {
break;
}
cur_ht = ht_stack->ht;
ht_stack_T *tempitem = ht_stack;
ht_stack = ht_stack->prev;
xfree(tempitem);
}
return abort;
}
bool set_ref_in_list(list_T *l, int copyID, ht_stack_T **ht_stack)
FUNC_ATTR_WARN_UNUSED_RESULT
{
bool abort = false;
list_stack_T *list_stack = NULL;
list_T *cur_l = l;
for (;;) {
TV_LIST_ITER(cur_l, li, {
if (abort) {
break;
}
abort = set_ref_in_item(TV_LIST_ITEM_TV(li), copyID, ht_stack,
&list_stack);
});
if (list_stack == NULL) {
break;
}
cur_l = list_stack->list;
list_stack_T *tempitem = list_stack;
list_stack = list_stack->prev;
xfree(tempitem);
}
return abort;
}
bool set_ref_in_item(typval_T *tv, int copyID, ht_stack_T **ht_stack,
list_stack_T **list_stack)
FUNC_ATTR_WARN_UNUSED_RESULT
{
bool abort = false;
switch (tv->v_type) {
case VAR_DICT: {
dict_T *dd = tv->vval.v_dict;
if (dd != NULL && dd->dv_copyID != copyID) {
dd->dv_copyID = copyID;
if (ht_stack == NULL) {
abort = set_ref_in_ht(&dd->dv_hashtab, copyID, list_stack);
} else {
ht_stack_T *newitem = try_malloc(sizeof(ht_stack_T));
if (newitem == NULL) {
abort = true;
} else {
newitem->ht = &dd->dv_hashtab;
newitem->prev = *ht_stack;
*ht_stack = newitem;
}
}
QUEUE *w = NULL;
DictWatcher *watcher = NULL;
QUEUE_FOREACH(w, &dd->watchers) {
watcher = tv_dict_watcher_node_data(w);
set_ref_in_callback(&watcher->callback, copyID, ht_stack, list_stack);
}
}
break;
}
case VAR_LIST: {
list_T *ll = tv->vval.v_list;
if (ll != NULL && ll->lv_copyID != copyID) {
ll->lv_copyID = copyID;
if (list_stack == NULL) {
abort = set_ref_in_list(ll, copyID, ht_stack);
} else {
list_stack_T *newitem = try_malloc(sizeof(list_stack_T));
if (newitem == NULL) {
abort = true;
} else {
newitem->list = ll;
newitem->prev = *list_stack;
*list_stack = newitem;
}
}
}
break;
}
case VAR_PARTIAL: {
partial_T *pt = tv->vval.v_partial;
if (pt != NULL) {
abort = set_ref_in_func(pt->pt_name, pt->pt_func, copyID);
if (pt->pt_dict != NULL) {
typval_T dtv;
dtv.v_type = VAR_DICT;
dtv.vval.v_dict = pt->pt_dict;
abort = abort || set_ref_in_item(&dtv, copyID, ht_stack, list_stack);
}
for (int i = 0; i < pt->pt_argc; i++) {
abort = abort || set_ref_in_item(&pt->pt_argv[i], copyID,
ht_stack, list_stack);
}
}
break;
}
case VAR_FUNC:
abort = set_ref_in_func(tv->vval.v_string, NULL, copyID);
break;
case VAR_UNKNOWN:
case VAR_SPECIAL:
case VAR_FLOAT:
case VAR_NUMBER:
case VAR_STRING: {
break;
}
}
return abort;
}
bool set_ref_in_functions(int copyID)
{
int todo;
hashitem_T *hi = NULL;
bool abort = false;
ufunc_T *fp;
todo = (int)func_hashtab.ht_used;
for (hi = func_hashtab.ht_array; todo > 0 && !got_int; hi++) {
if (!HASHITEM_EMPTY(hi)) {
todo--;
fp = HI2UF(hi);
if (!func_name_refcount(fp->uf_name)) {
abort = abort || set_ref_in_func(NULL, fp, copyID);
}
}
}
return abort;
}
static inline bool set_ref_in_fmark(fmark_T fm, int copyID)
FUNC_ATTR_WARN_UNUSED_RESULT
{
if (fm.additional_data != NULL
&& fm.additional_data->dv_copyID != copyID) {
fm.additional_data->dv_copyID = copyID;
return set_ref_in_ht(&fm.additional_data->dv_hashtab, copyID, NULL);
}
return false;
}
static inline bool set_ref_list(list_T *list, int copyID)
FUNC_ATTR_WARN_UNUSED_RESULT
{
if (list != NULL) {
typval_T tv = (typval_T) {
.v_type = VAR_LIST,
.vval = { .v_list = list }
};
return set_ref_in_item(&tv, copyID, NULL, NULL);
}
return false;
}
static inline bool set_ref_dict(dict_T *dict, int copyID)
FUNC_ATTR_WARN_UNUSED_RESULT
{
if (dict != NULL) {
typval_T tv = (typval_T) {
.v_type = VAR_DICT,
.vval = { .v_dict = dict }
};
return set_ref_in_item(&tv, copyID, NULL, NULL);
}
return false;
}
static bool set_ref_in_funccal(funccall_T *fc, int copyID)
{
bool abort = false;
if (fc->fc_copyID != copyID) {
fc->fc_copyID = copyID;
abort = abort || set_ref_in_ht(&fc->l_vars.dv_hashtab, copyID, NULL);
abort = abort || set_ref_in_ht(&fc->l_avars.dv_hashtab, copyID, NULL);
abort = abort || set_ref_in_func(NULL, fc->func, copyID);
}
return abort;
}
static int dict_get_tv(char_u **arg, typval_T *rettv, int evaluate)
{
dict_T *d = NULL;
typval_T tvkey;
typval_T tv;
char_u *key = NULL;
dictitem_T *item;
char_u *start = skipwhite(*arg + 1);
char buf[NUMBUFLEN];
if (*start != '}') {
if (eval1(&start, &tv, FALSE) == FAIL) 
return FAIL;
if (*start == '}')
return NOTDONE;
}
if (evaluate) {
d = tv_dict_alloc();
}
tvkey.v_type = VAR_UNKNOWN;
tv.v_type = VAR_UNKNOWN;
*arg = skipwhite(*arg + 1);
while (**arg != '}' && **arg != NUL) {
if (eval1(arg, &tvkey, evaluate) == FAIL) 
goto failret;
if (**arg != ':') {
EMSG2(_("E720: Missing colon in Dictionary: %s"), *arg);
tv_clear(&tvkey);
goto failret;
}
if (evaluate) {
key = (char_u *)tv_get_string_buf_chk(&tvkey, buf);
if (key == NULL) {
tv_clear(&tvkey);
goto failret;
}
}
*arg = skipwhite(*arg + 1);
if (eval1(arg, &tv, evaluate) == FAIL) { 
if (evaluate) {
tv_clear(&tvkey);
}
goto failret;
}
if (evaluate) {
item = tv_dict_find(d, (const char *)key, -1);
if (item != NULL) {
EMSG2(_("E721: Duplicate key in Dictionary: \"%s\""), key);
tv_clear(&tvkey);
tv_clear(&tv);
goto failret;
}
item = tv_dict_item_alloc((const char *)key);
item->di_tv = tv;
item->di_tv.v_lock = 0;
if (tv_dict_add(d, item) == FAIL) {
tv_dict_item_free(item);
}
}
tv_clear(&tvkey);
if (**arg == '}')
break;
if (**arg != ',') {
EMSG2(_("E722: Missing comma in Dictionary: %s"), *arg);
goto failret;
}
*arg = skipwhite(*arg + 1);
}
if (**arg != '}') {
EMSG2(_("E723: Missing end of Dictionary '}': %s"), *arg);
failret:
if (d != NULL) {
tv_dict_free(d);
}
return FAIL;
}
*arg = skipwhite(*arg + 1);
if (evaluate) {
tv_dict_set_ret(rettv, d);
}
return OK;
}
static int get_function_args(char_u **argp, char_u endchar, garray_T *newargs,
int *varargs, bool skip)
{
bool mustend = false;
char_u *arg = *argp;
char_u *p = arg;
int c;
int i;
if (newargs != NULL) {
ga_init(newargs, (int)sizeof(char_u *), 3);
}
if (varargs != NULL) {
*varargs = false;
}
while (*p != endchar) {
if (p[0] == '.' && p[1] == '.' && p[2] == '.') {
if (varargs != NULL) {
*varargs = true;
}
p += 3;
mustend = true;
} else {
arg = p;
while (ASCII_ISALNUM(*p) || *p == '_') {
p++;
}
if (arg == p || isdigit(*arg)
|| (p - arg == 9 && STRNCMP(arg, "firstline", 9) == 0)
|| (p - arg == 8 && STRNCMP(arg, "lastline", 8) == 0)) {
if (!skip) {
EMSG2(_("E125: Illegal argument: %s"), arg);
}
break;
}
if (newargs != NULL) {
ga_grow(newargs, 1);
c = *p;
*p = NUL;
arg = vim_strsave(arg);
for (i = 0; i < newargs->ga_len; i++) {
if (STRCMP(((char_u **)(newargs->ga_data))[i], arg) == 0) {
EMSG2(_("E853: Duplicate argument name: %s"), arg);
xfree(arg);
goto err_ret;
}
}
((char_u **)(newargs->ga_data))[newargs->ga_len] = arg;
newargs->ga_len++;
*p = c;
}
if (*p == ',') {
p++;
} else {
mustend = true;
}
}
p = skipwhite(p);
if (mustend && *p != endchar) {
if (!skip) {
EMSG2(_(e_invarg2), *argp);
}
break;
}
}
if (*p != endchar) {
goto err_ret;
}
p++; 
*argp = p;
return OK;
err_ret:
if (newargs != NULL) {
ga_clear_strings(newargs);
}
return FAIL;
}
static void register_closure(ufunc_T *fp)
{
if (fp->uf_scoped == current_funccal) {
return;
}
funccal_unref(fp->uf_scoped, fp, false);
fp->uf_scoped = current_funccal;
current_funccal->fc_refcount++;
ga_grow(&current_funccal->fc_funcs, 1);
((ufunc_T **)current_funccal->fc_funcs.ga_data)
[current_funccal->fc_funcs.ga_len++] = fp;
}
static int get_lambda_tv(char_u **arg, typval_T *rettv, bool evaluate)
{
garray_T newargs = GA_EMPTY_INIT_VALUE;
garray_T *pnewargs;
ufunc_T *fp = NULL;
int varargs;
int ret;
char_u *start = skipwhite(*arg + 1);
char_u *s, *e;
static int lambda_no = 0;
int *old_eval_lavars = eval_lavars_used;
int eval_lavars = false;
ret = get_function_args(&start, '-', NULL, NULL, true);
if (ret == FAIL || *start != '>') {
return NOTDONE;
}
if (evaluate) {
pnewargs = &newargs;
} else {
pnewargs = NULL;
}
*arg = skipwhite(*arg + 1);
ret = get_function_args(arg, '-', pnewargs, &varargs, false);
if (ret == FAIL || **arg != '>') {
goto errret;
}
if (evaluate) {
eval_lavars_used = &eval_lavars;
}
*arg = skipwhite(*arg + 1);
s = *arg;
ret = skip_expr(arg);
if (ret == FAIL) {
goto errret;
}
e = *arg;
*arg = skipwhite(*arg);
if (**arg != '}') {
goto errret;
}
(*arg)++;
if (evaluate) {
int len, flags = 0;
char_u *p;
char_u name[20];
partial_T *pt;
garray_T newlines;
lambda_no++;
snprintf((char *)name, sizeof(name), "<lambda>%d", lambda_no);
fp = xcalloc(1, offsetof(ufunc_T, uf_name) + STRLEN(name) + 1);
pt = xcalloc(1, sizeof(partial_T));
ga_init(&newlines, (int)sizeof(char_u *), 1);
ga_grow(&newlines, 1);
len = 7 + e - s + 1;
p = (char_u *)xmalloc(len);
((char_u **)(newlines.ga_data))[newlines.ga_len++] = p;
STRCPY(p, "return ");
STRLCPY(p + 7, s, e - s + 1);
fp->uf_refcount = 1;
STRCPY(fp->uf_name, name);
hash_add(&func_hashtab, UF2HIKEY(fp));
fp->uf_args = newargs;
fp->uf_lines = newlines;
if (current_funccal != NULL && eval_lavars) {
flags |= FC_CLOSURE;
register_closure(fp);
} else {
fp->uf_scoped = NULL;
}
if (prof_def_func()) {
func_do_profile(fp);
}
if (sandbox) {
flags |= FC_SANDBOX;
}
fp->uf_varargs = true;
fp->uf_flags = flags;
fp->uf_calls = 0;
fp->uf_script_ctx = current_sctx;
fp->uf_script_ctx.sc_lnum += sourcing_lnum - newlines.ga_len;
pt->pt_func = fp;
pt->pt_refcount = 1;
rettv->vval.v_partial = pt;
rettv->v_type = VAR_PARTIAL;
}
eval_lavars_used = old_eval_lavars;
return OK;
errret:
ga_clear_strings(&newargs);
xfree(fp);
eval_lavars_used = old_eval_lavars;
return FAIL;
}
size_t string2float(const char *const text, float_T *const ret_value)
FUNC_ATTR_NONNULL_ALL
{
char *s = NULL;
if (STRNICMP(text, "inf", 3) == 0) {
*ret_value = INFINITY;
return 3;
}
if (STRNICMP(text, "-inf", 3) == 0) {
*ret_value = -INFINITY;
return 4;
}
if (STRNICMP(text, "nan", 3) == 0) {
*ret_value = NAN;
return 3;
}
*ret_value = strtod(text, &s);
return (size_t) (s - text);
}
static int get_env_tv(char_u **arg, typval_T *rettv, int evaluate)
{
char_u *name;
char_u *string = NULL;
int len;
int cc;
++*arg;
name = *arg;
len = get_env_len((const char_u **)arg);
if (evaluate) {
if (len == 0) {
return FAIL; 
}
cc = name[len];
name[len] = NUL;
string = (char_u *)vim_getenv((char *)name);
if (string == NULL || *string == NUL) {
xfree(string);
string = expand_env_save(name - 1);
if (string != NULL && *string == '$') {
XFREE_CLEAR(string);
}
}
name[len] = cc;
rettv->v_type = VAR_STRING;
rettv->vval.v_string = string;
}
return OK;
}
static char_u *deref_func_name(const char *name, int *lenp,
partial_T **const partialp, bool no_autoload)
FUNC_ATTR_NONNULL_ARG(1, 2)
{
if (partialp != NULL) {
*partialp = NULL;
}
dictitem_T *const v = find_var(name, (size_t)(*lenp), NULL, no_autoload);
if (v != NULL && v->di_tv.v_type == VAR_FUNC) {
if (v->di_tv.vval.v_string == NULL) { 
*lenp = 0;
return (char_u *)"";
}
*lenp = (int)STRLEN(v->di_tv.vval.v_string);
return v->di_tv.vval.v_string;
}
if (v != NULL && v->di_tv.v_type == VAR_PARTIAL) {
partial_T *const pt = v->di_tv.vval.v_partial;
if (pt == NULL) { 
*lenp = 0;
return (char_u *)"";
}
if (partialp != NULL) {
*partialp = pt;
}
char_u *s = partial_name(pt);
*lenp = (int)STRLEN(s);
return s;
}
return (char_u *)name;
}
static int
get_func_tv(
const char_u *name, 
int len, 
typval_T *rettv,
char_u **arg, 
linenr_T firstline, 
linenr_T lastline, 
int *doesrange, 
int evaluate,
partial_T *partial, 
dict_T *selfdict 
)
{
char_u *argp;
int ret = OK;
typval_T argvars[MAX_FUNC_ARGS + 1]; 
int argcount = 0; 
argp = *arg;
while (argcount < MAX_FUNC_ARGS - (partial == NULL ? 0 : partial->pt_argc)) {
argp = skipwhite(argp + 1); 
if (*argp == ')' || *argp == ',' || *argp == NUL) {
break;
}
if (eval1(&argp, &argvars[argcount], evaluate) == FAIL) {
ret = FAIL;
break;
}
++argcount;
if (*argp != ',')
break;
}
if (*argp == ')')
++argp;
else
ret = FAIL;
if (ret == OK) {
int i = 0;
if (get_vim_var_nr(VV_TESTING)) {
if (funcargs.ga_itemsize == 0) {
ga_init(&funcargs, (int)sizeof(typval_T *), 50);
}
for (i = 0; i < argcount; i++) {
ga_grow(&funcargs, 1);
((typval_T **)funcargs.ga_data)[funcargs.ga_len++] = &argvars[i];
}
}
ret = call_func(name, len, rettv, argcount, argvars, NULL,
firstline, lastline, doesrange, evaluate,
partial, selfdict);
funcargs.ga_len -= i;
} else if (!aborting()) {
if (argcount == MAX_FUNC_ARGS) {
emsg_funcname(N_("E740: Too many arguments for function %s"), name);
} else {
emsg_funcname(N_("E116: Invalid arguments for function %s"), name);
}
}
while (--argcount >= 0) {
tv_clear(&argvars[argcount]);
}
*arg = skipwhite(argp);
return ret;
}
typedef enum {
ERROR_UNKNOWN = 0,
ERROR_TOOMANY,
ERROR_TOOFEW,
ERROR_SCRIPT,
ERROR_DICT,
ERROR_NONE,
ERROR_OTHER,
ERROR_BOTH,
ERROR_DELETED,
} FnameTransError;
#define FLEN_FIXED 40
static char_u *fname_trans_sid(const char_u *const name,
char_u *const fname_buf,
char_u **const tofree, int *const error)
FUNC_ATTR_NONNULL_ALL FUNC_ATTR_WARN_UNUSED_RESULT
{
char_u *fname;
const int llen = eval_fname_script((const char *)name);
if (llen > 0) {
fname_buf[0] = K_SPECIAL;
fname_buf[1] = KS_EXTRA;
fname_buf[2] = (int)KE_SNR;
int i = 3;
if (eval_fname_sid((const char *)name)) { 
if (current_sctx.sc_sid <= 0) {
*error = ERROR_SCRIPT;
} else {
snprintf((char *)fname_buf + 3, FLEN_FIXED + 1, "%" PRId64 "_",
(int64_t)current_sctx.sc_sid);
i = (int)STRLEN(fname_buf);
}
}
if (i + STRLEN(name + llen) < FLEN_FIXED) {
STRCPY(fname_buf + i, name + llen);
fname = fname_buf;
} else {
fname = xmalloc(i + STRLEN(name + llen) + 1);
*tofree = fname;
memmove(fname, fname_buf, (size_t)i);
STRCPY(fname + i, name + llen);
}
} else {
fname = (char_u *)name;
}
return fname;
}
bool set_ref_in_func(char_u *name, ufunc_T *fp_in, int copyID)
{
ufunc_T *fp = fp_in;
funccall_T *fc;
int error = ERROR_NONE;
char_u fname_buf[FLEN_FIXED + 1];
char_u *tofree = NULL;
char_u *fname;
bool abort = false;
if (name == NULL && fp_in == NULL) {
return false;
}
if (fp_in == NULL) {
fname = fname_trans_sid(name, fname_buf, &tofree, &error);
fp = find_func(fname);
}
if (fp != NULL) {
for (fc = fp->uf_scoped; fc != NULL; fc = fc->func->uf_scoped) {
abort = abort || set_ref_in_funccal(fc, copyID);
}
}
xfree(tofree);
return abort;
}
int
call_func(
const char_u *funcname, 
int len, 
typval_T *rettv, 
int argcount_in, 
typval_T *argvars_in, 
ArgvFunc argv_func, 
linenr_T firstline, 
linenr_T lastline, 
int *doesrange, 
bool evaluate,
partial_T *partial, 
dict_T *selfdict_in 
)
FUNC_ATTR_NONNULL_ARG(1, 3, 5, 9)
{
int ret = FAIL;
int error = ERROR_NONE;
ufunc_T *fp;
char_u fname_buf[FLEN_FIXED + 1];
char_u *tofree = NULL;
char_u *fname;
char_u *name;
int argcount = argcount_in;
typval_T *argvars = argvars_in;
dict_T *selfdict = selfdict_in;
typval_T argv[MAX_FUNC_ARGS + 1]; 
int argv_clear = 0;
rettv->v_type = VAR_UNKNOWN;
name = vim_strnsave(funcname, len);
fname = fname_trans_sid(name, fname_buf, &tofree, &error);
*doesrange = false;
if (partial != NULL) {
if (partial->pt_dict != NULL
&& (selfdict_in == NULL || !partial->pt_auto)) {
selfdict = partial->pt_dict;
}
if (error == ERROR_NONE && partial->pt_argc > 0) {
for (argv_clear = 0; argv_clear < partial->pt_argc; argv_clear++) {
tv_copy(&partial->pt_argv[argv_clear], &argv[argv_clear]);
}
for (int i = 0; i < argcount_in; i++) {
argv[i + argv_clear] = argvars_in[i];
}
argvars = argv;
argcount = partial->pt_argc + argcount_in;
}
}
if (error == ERROR_NONE && evaluate) {
char_u *rfname = fname;
if (fname[0] == 'g' && fname[1] == ':') {
rfname = fname + 2;
}
rettv->v_type = VAR_NUMBER; 
rettv->vval.v_number = 0;
error = ERROR_UNKNOWN;
if (partial == vvlua_partial) {
if (len > 0) {
error = ERROR_NONE;
executor_call_lua((const char *)funcname, len,
argvars, argcount, rettv);
}
} else if (!builtin_function((const char *)rfname, -1)) {
if (partial != NULL && partial->pt_func != NULL) {
fp = partial->pt_func;
} else {
fp = find_func(rfname);
}
if (fp == NULL
&& apply_autocmds(EVENT_FUNCUNDEFINED, rfname, rfname, TRUE, NULL)
&& !aborting()) {
fp = find_func(rfname);
}
if (fp == NULL && script_autoload((const char *)rfname, STRLEN(rfname),
true) && !aborting()) {
fp = find_func(rfname);
}
if (fp != NULL && (fp->uf_flags & FC_DELETED)) {
error = ERROR_DELETED;
} else if (fp != NULL) {
if (argv_func != NULL) {
argcount = argv_func(argcount, argvars, fp->uf_args.ga_len);
}
if (fp->uf_flags & FC_RANGE) {
*doesrange = true;
}
if (argcount < fp->uf_args.ga_len) {
error = ERROR_TOOFEW;
} else if (!fp->uf_varargs && argcount > fp->uf_args.ga_len) {
error = ERROR_TOOMANY;
} else if ((fp->uf_flags & FC_DICT) && selfdict == NULL) {
error = ERROR_DICT;
} else {
call_user_func(fp, argcount, argvars, rettv, firstline, lastline,
(fp->uf_flags & FC_DICT) ? selfdict : NULL);
error = ERROR_NONE;
}
}
} else {
const VimLFuncDef *const fdef = find_internal_func((const char *)fname);
if (fdef != NULL) {
if (argcount < fdef->min_argc) {
error = ERROR_TOOFEW;
} else if (argcount > fdef->max_argc) {
error = ERROR_TOOMANY;
} else {
argvars[argcount].v_type = VAR_UNKNOWN;
fdef->func(argvars, rettv, fdef->data);
error = ERROR_NONE;
}
}
}
update_force_abort();
}
if (error == ERROR_NONE)
ret = OK;
if (!aborting()) {
switch (error) {
case ERROR_UNKNOWN:
emsg_funcname(N_("E117: Unknown function: %s"), name);
break;
case ERROR_DELETED:
emsg_funcname(N_("E933: Function was deleted: %s"), name);
break;
case ERROR_TOOMANY:
emsg_funcname(_(e_toomanyarg), name);
break;
case ERROR_TOOFEW:
emsg_funcname(N_("E119: Not enough arguments for function: %s"),
name);
break;
case ERROR_SCRIPT:
emsg_funcname(N_("E120: Using <SID> not in a script context: %s"),
name);
break;
case ERROR_DICT:
emsg_funcname(N_("E725: Calling dict function without Dictionary: %s"),
name);
break;
}
}
while (argv_clear > 0) {
tv_clear(&argv[--argv_clear]);
}
xfree(tofree);
xfree(name);
return ret;
}
static void emsg_funcname(char *ermsg, const char_u *name)
{
char_u *p;
if (*name == K_SPECIAL) {
p = concat_str((char_u *)"<SNR>", name + 3);
} else {
p = (char_u *)name;
}
EMSG2(_(ermsg), p);
if (p != name) {
xfree(p);
}
}
void get_arglist_as_rettv(aentry_T *arglist, int argcount,
typval_T *rettv)
{
tv_list_alloc_ret(rettv, argcount);
if (arglist != NULL) {
for (int idx = 0; idx < argcount; idx++) {
tv_list_append_string(rettv->vval.v_list,
(const char *)alist_name(&arglist[idx]), -1);
}
}
}
void prepare_assert_error(garray_T *gap)
{
char buf[NUMBUFLEN];
ga_init(gap, 1, 100);
if (sourcing_name != NULL) {
ga_concat(gap, sourcing_name);
if (sourcing_lnum > 0) {
ga_concat(gap, (char_u *)" ");
}
}
if (sourcing_lnum > 0) {
vim_snprintf(buf, ARRAY_SIZE(buf), "line %" PRId64, (int64_t)sourcing_lnum);
ga_concat(gap, (char_u *)buf);
}
if (sourcing_name != NULL || sourcing_lnum > 0) {
ga_concat(gap, (char_u *)": ");
}
}
static void ga_concat_esc(garray_T *gap, char_u *str)
{
char_u *p;
char_u buf[NUMBUFLEN];
if (str == NULL) {
ga_concat(gap, (char_u *)"NULL");
return;
}
for (p = str; *p != NUL; p++) {
switch (*p) {
case BS: ga_concat(gap, (char_u *)"\\b"); break;
case ESC: ga_concat(gap, (char_u *)"\\e"); break;
case FF: ga_concat(gap, (char_u *)"\\f"); break;
case NL: ga_concat(gap, (char_u *)"\\n"); break;
case TAB: ga_concat(gap, (char_u *)"\\t"); break;
case CAR: ga_concat(gap, (char_u *)"\\r"); break;
case '\\': ga_concat(gap, (char_u *)"\\\\"); break;
default:
if (*p < ' ') {
vim_snprintf((char *)buf, NUMBUFLEN, "\\x%02x", *p);
ga_concat(gap, buf);
} else {
ga_append(gap, *p);
}
break;
}
}
}
void fill_assert_error(garray_T *gap, typval_T *opt_msg_tv,
char_u *exp_str, typval_T *exp_tv,
typval_T *got_tv, assert_type_T atype)
{
char_u *tofree;
if (opt_msg_tv->v_type != VAR_UNKNOWN) {
tofree = (char_u *)encode_tv2echo(opt_msg_tv, NULL);
ga_concat(gap, tofree);
xfree(tofree);
ga_concat(gap, (char_u *)": ");
}
if (atype == ASSERT_MATCH || atype == ASSERT_NOTMATCH) {
ga_concat(gap, (char_u *)"Pattern ");
} else if (atype == ASSERT_NOTEQUAL) {
ga_concat(gap, (char_u *)"Expected not equal to ");
} else {
ga_concat(gap, (char_u *)"Expected ");
}
if (exp_str == NULL) {
tofree = (char_u *)encode_tv2string(exp_tv, NULL);
ga_concat_esc(gap, tofree);
xfree(tofree);
} else {
ga_concat_esc(gap, exp_str);
}
if (atype != ASSERT_NOTEQUAL) {
if (atype == ASSERT_MATCH) {
ga_concat(gap, (char_u *)" does not match ");
} else if (atype == ASSERT_NOTMATCH) {
ga_concat(gap, (char_u *)" does match ");
} else {
ga_concat(gap, (char_u *)" but got ");
}
tofree = (char_u *)encode_tv2string(got_tv, NULL);
ga_concat_esc(gap, tofree);
xfree(tofree);
}
}
void assert_error(garray_T *gap)
{
struct vimvar *vp = &vimvars[VV_ERRORS];
if (vp->vv_type != VAR_LIST || vimvars[VV_ERRORS].vv_list == NULL) {
set_vim_var_list(VV_ERRORS, tv_list_alloc(1));
}
tv_list_append_string(vimvars[VV_ERRORS].vv_list,
(const char *)gap->ga_data, (ptrdiff_t)gap->ga_len);
}
int assert_equal_common(typval_T *argvars, assert_type_T atype)
FUNC_ATTR_NONNULL_ALL
{
garray_T ga;
if (tv_equal(&argvars[0], &argvars[1], false, false)
!= (atype == ASSERT_EQUAL)) {
prepare_assert_error(&ga);
fill_assert_error(&ga, &argvars[2], NULL,
&argvars[0], &argvars[1], atype);
assert_error(&ga);
ga_clear(&ga);
return 1;
}
return 0;
}
int assert_equalfile(typval_T *argvars)
FUNC_ATTR_NONNULL_ALL
{
char buf1[NUMBUFLEN];
char buf2[NUMBUFLEN];
const char *const fname1 = tv_get_string_buf_chk(&argvars[0], buf1);
const char *const fname2 = tv_get_string_buf_chk(&argvars[1], buf2);
garray_T ga;
if (fname1 == NULL || fname2 == NULL) {
return 0;
}
IObuff[0] = NUL;
FILE *const fd1 = os_fopen(fname1, READBIN);
if (fd1 == NULL) {
snprintf((char *)IObuff, IOSIZE, (char *)e_notread, fname1);
} else {
FILE *const fd2 = os_fopen(fname2, READBIN);
if (fd2 == NULL) {
fclose(fd1);
snprintf((char *)IObuff, IOSIZE, (char *)e_notread, fname2);
} else {
for (int64_t count = 0; ; count++) {
const int c1 = fgetc(fd1);
const int c2 = fgetc(fd2);
if (c1 == EOF) {
if (c2 != EOF) {
STRCPY(IObuff, "first file is shorter");
}
break;
} else if (c2 == EOF) {
STRCPY(IObuff, "second file is shorter");
break;
} else if (c1 != c2) {
snprintf((char *)IObuff, IOSIZE,
"difference at byte %" PRId64, count);
break;
}
}
fclose(fd1);
fclose(fd2);
}
}
if (IObuff[0] != NUL) {
prepare_assert_error(&ga);
ga_concat(&ga, IObuff);
assert_error(&ga);
ga_clear(&ga);
return 1;
}
return 0;
}
int assert_inrange(typval_T *argvars)
FUNC_ATTR_NONNULL_ALL
{
bool error = false;
const varnumber_T lower = tv_get_number_chk(&argvars[0], &error);
const varnumber_T upper = tv_get_number_chk(&argvars[1], &error);
const varnumber_T actual = tv_get_number_chk(&argvars[2], &error);
if (error) {
return 0;
}
if (actual < lower || actual > upper) {
garray_T ga;
prepare_assert_error(&ga);
char msg[55];
vim_snprintf(msg, sizeof(msg),
"range %" PRIdVARNUMBER " - %" PRIdVARNUMBER ",",
lower, upper);
fill_assert_error(&ga, &argvars[3], (char_u *)msg, NULL, &argvars[2],
ASSERT_INRANGE);
assert_error(&ga);
ga_clear(&ga);
return 1;
}
return 0;
}
int assert_bool(typval_T *argvars, bool is_true)
FUNC_ATTR_NONNULL_ALL
{
bool error = false;
garray_T ga;
if ((argvars[0].v_type != VAR_NUMBER
|| (tv_get_number_chk(&argvars[0], &error) == 0) == is_true
|| error)
&& (argvars[0].v_type != VAR_SPECIAL
|| (argvars[0].vval.v_special
!= (SpecialVarValue) (is_true
? kSpecialVarTrue
: kSpecialVarFalse)))) {
prepare_assert_error(&ga);
fill_assert_error(&ga, &argvars[1],
(char_u *)(is_true ? "True" : "False"),
NULL, &argvars[0], ASSERT_OTHER);
assert_error(&ga);
ga_clear(&ga);
return 1;
}
return 0;
}
int assert_exception(typval_T *argvars)
FUNC_ATTR_NONNULL_ALL
{
garray_T ga;
const char *const error = tv_get_string_chk(&argvars[0]);
if (vimvars[VV_EXCEPTION].vv_str == NULL) {
prepare_assert_error(&ga);
ga_concat(&ga, (char_u *)"v:exception is not set");
assert_error(&ga);
ga_clear(&ga);
return 1;
} else if (error != NULL
&& strstr((char *)vimvars[VV_EXCEPTION].vv_str, error) == NULL) {
prepare_assert_error(&ga);
fill_assert_error(&ga, &argvars[1], NULL, &argvars[0],
&vimvars[VV_EXCEPTION].vv_tv, ASSERT_OTHER);
assert_error(&ga);
ga_clear(&ga);
return 1;
}
return 0;
}
int assert_fails(typval_T *argvars)
FUNC_ATTR_NONNULL_ALL
{
const char *const cmd = tv_get_string_chk(&argvars[0]);
garray_T ga;
int ret = 0;
int save_trylevel = trylevel;
trylevel = 0;
called_emsg = false;
suppress_errthrow = true;
emsg_silent = true;
do_cmdline_cmd(cmd);
if (!called_emsg) {
prepare_assert_error(&ga);
ga_concat(&ga, (const char_u *)"command did not fail: ");
if (argvars[1].v_type != VAR_UNKNOWN
&& argvars[2].v_type != VAR_UNKNOWN) {
char *const tofree = encode_tv2echo(&argvars[2], NULL);
ga_concat(&ga, (char_u *)tofree);
xfree(tofree);
} else {
ga_concat(&ga, (const char_u *)cmd);
}
assert_error(&ga);
ga_clear(&ga);
ret = 1;
} else if (argvars[1].v_type != VAR_UNKNOWN) {
char buf[NUMBUFLEN];
const char *const error = tv_get_string_buf_chk(&argvars[1], buf);
if (error == NULL
|| strstr((char *)vimvars[VV_ERRMSG].vv_str, error) == NULL) {
prepare_assert_error(&ga);
fill_assert_error(&ga, &argvars[2], NULL, &argvars[1],
&vimvars[VV_ERRMSG].vv_tv, ASSERT_OTHER);
assert_error(&ga);
ga_clear(&ga);
ret = 1;
}
}
trylevel = save_trylevel;
called_emsg = false;
suppress_errthrow = false;
emsg_silent = false;
emsg_on_display = false;
set_vim_var_string(VV_ERRMSG, NULL, 0);
return ret;
}
int assert_match_common(typval_T *argvars, assert_type_T atype)
FUNC_ATTR_NONNULL_ALL
{
char buf1[NUMBUFLEN];
char buf2[NUMBUFLEN];
const char *const pat = tv_get_string_buf_chk(&argvars[0], buf1);
const char *const text = tv_get_string_buf_chk(&argvars[1], buf2);
if (pat == NULL || text == NULL) {
EMSG(_(e_invarg));
} else if (pattern_match((char_u *)pat, (char_u *)text, false)
!= (atype == ASSERT_MATCH)) {
garray_T ga;
prepare_assert_error(&ga);
fill_assert_error(&ga, &argvars[2], NULL, &argvars[0], &argvars[1], atype);
assert_error(&ga);
ga_clear(&ga);
return 1;
}
return 0;
}
int func_call(char_u *name, typval_T *args, partial_T *partial,
dict_T *selfdict, typval_T *rettv)
{
typval_T argv[MAX_FUNC_ARGS + 1];
int argc = 0;
int dummy;
int r = 0;
TV_LIST_ITER(args->vval.v_list, item, {
if (argc == MAX_FUNC_ARGS - (partial == NULL ? 0 : partial->pt_argc)) {
EMSG(_("E699: Too many arguments"));
goto func_call_skip_call;
}
tv_copy(TV_LIST_ITEM_TV(item), &argv[argc++]);
});
r = call_func(name, (int)STRLEN(name), rettv, argc, argv, NULL,
curwin->w_cursor.lnum, curwin->w_cursor.lnum,
&dummy, true, partial, selfdict);
func_call_skip_call:
while (argc > 0) {
tv_clear(&argv[--argc]);
}
return r;
}
win_T * find_win_by_nr_or_id(typval_T *vp)
{
int nr = (int)tv_get_number_chk(vp, NULL);
if (nr >= LOWEST_WIN_ID) {
return win_id2wp(vp);
}
return find_win_by_nr(vp, NULL);
}
void filter_map(typval_T *argvars, typval_T *rettv, int map)
{
typval_T *expr;
list_T *l = NULL;
dictitem_T *di;
hashtab_T *ht;
hashitem_T *hi;
dict_T *d = NULL;
typval_T save_val;
typval_T save_key;
int rem = false;
int todo;
char_u *ermsg = (char_u *)(map ? "map()" : "filter()");
const char *const arg_errmsg = (map
? N_("map() argument")
: N_("filter() argument"));
int save_did_emsg;
int idx = 0;
if (argvars[0].v_type == VAR_LIST) {
tv_copy(&argvars[0], rettv);
if ((l = argvars[0].vval.v_list) == NULL
|| (!map && tv_check_lock(tv_list_locked(l), arg_errmsg,
TV_TRANSLATE))) {
return;
}
} else if (argvars[0].v_type == VAR_DICT) {
tv_copy(&argvars[0], rettv);
if ((d = argvars[0].vval.v_dict) == NULL
|| (!map && tv_check_lock(d->dv_lock, arg_errmsg, TV_TRANSLATE))) {
return;
}
} else {
EMSG2(_(e_listdictarg), ermsg);
return;
}
expr = &argvars[1];
if (expr->v_type != VAR_UNKNOWN) {
prepare_vimvar(VV_VAL, &save_val);
save_did_emsg = did_emsg;
did_emsg = FALSE;
prepare_vimvar(VV_KEY, &save_key);
if (argvars[0].v_type == VAR_DICT) {
vimvars[VV_KEY].vv_type = VAR_STRING;
ht = &d->dv_hashtab;
hash_lock(ht);
todo = (int)ht->ht_used;
for (hi = ht->ht_array; todo > 0; ++hi) {
if (!HASHITEM_EMPTY(hi)) {
--todo;
di = TV_DICT_HI2DI(hi);
if (map
&& (tv_check_lock(di->di_tv.v_lock, arg_errmsg, TV_TRANSLATE)
|| var_check_ro(di->di_flags, arg_errmsg, TV_TRANSLATE))) {
break;
}
vimvars[VV_KEY].vv_str = vim_strsave(di->di_key);
int r = filter_map_one(&di->di_tv, expr, map, &rem);
tv_clear(&vimvars[VV_KEY].vv_tv);
if (r == FAIL || did_emsg) {
break;
}
if (!map && rem) {
if (var_check_fixed(di->di_flags, arg_errmsg, TV_TRANSLATE)
|| var_check_ro(di->di_flags, arg_errmsg, TV_TRANSLATE)) {
break;
}
tv_dict_item_remove(d, di);
}
}
}
hash_unlock(ht);
} else {
assert(argvars[0].v_type == VAR_LIST);
vimvars[VV_KEY].vv_type = VAR_NUMBER;
for (listitem_T *li = tv_list_first(l); li != NULL;) {
if (map
&& tv_check_lock(TV_LIST_ITEM_TV(li)->v_lock, arg_errmsg,
TV_TRANSLATE)) {
break;
}
vimvars[VV_KEY].vv_nr = idx;
if (filter_map_one(TV_LIST_ITEM_TV(li), expr, map, &rem) == FAIL
|| did_emsg) {
break;
}
if (!map && rem) {
li = tv_list_item_remove(l, li);
} else {
li = TV_LIST_ITEM_NEXT(l, li);
}
idx++;
}
}
restore_vimvar(VV_KEY, &save_key);
restore_vimvar(VV_VAL, &save_val);
did_emsg |= save_did_emsg;
}
}
static int filter_map_one(typval_T *tv, typval_T *expr, int map, int *remp)
FUNC_ATTR_NONNULL_ARG(1, 2)
{
typval_T rettv;
typval_T argv[3];
int retval = FAIL;
tv_copy(tv, &vimvars[VV_VAL].vv_tv);
argv[0] = vimvars[VV_KEY].vv_tv;
argv[1] = vimvars[VV_VAL].vv_tv;
if (eval_expr_typval(expr, argv, 2, &rettv) == FAIL) {
goto theend;
}
if (map) {
tv_clear(tv);
rettv.v_lock = 0;
*tv = rettv;
} else {
bool error = false;
*remp = (tv_get_number_chk(&rettv, &error) == 0);
tv_clear(&rettv);
if (error) {
goto theend;
}
}
retval = OK;
theend:
tv_clear(&vimvars[VV_VAL].vv_tv);
return retval;
}
void common_function(typval_T *argvars, typval_T *rettv,
bool is_funcref, FunPtr fptr)
{
char_u *s;
char_u *name;
bool use_string = false;
partial_T *arg_pt = NULL;
char_u *trans_name = NULL;
if (argvars[0].v_type == VAR_FUNC) {
s = argvars[0].vval.v_string;
} else if (argvars[0].v_type == VAR_PARTIAL
&& argvars[0].vval.v_partial != NULL) {
arg_pt = argvars[0].vval.v_partial;
s = partial_name(arg_pt);
} else {
s = (char_u *)tv_get_string(&argvars[0]);
use_string = true;
}
if ((use_string && vim_strchr(s, AUTOLOAD_CHAR) == NULL) || is_funcref) {
name = s;
trans_name = trans_function_name(&name, false,
TFN_INT | TFN_QUIET | TFN_NO_AUTOLOAD
| TFN_NO_DEREF, NULL, NULL);
if (*name != NUL) {
s = NULL;
}
}
if (s == NULL || *s == NUL || (use_string && ascii_isdigit(*s))
|| (is_funcref && trans_name == NULL)) {
emsgf(_(e_invarg2), (use_string
? tv_get_string(&argvars[0])
: (const char *)s));
} else if (trans_name != NULL
&& (is_funcref ? find_func(trans_name) == NULL
: !translated_function_exists((const char *)trans_name))) {
emsgf(_("E700: Unknown function: %s"), s);
} else {
int dict_idx = 0;
int arg_idx = 0;
list_T *list = NULL;
if (STRNCMP(s, "s:", 2) == 0 || STRNCMP(s, "<SID>", 5) == 0) {
char sid_buf[25];
int off = *s == 's' ? 2 : 5;
snprintf(sid_buf, sizeof(sid_buf), "<SNR>%" PRId64 "_",
(int64_t)current_sctx.sc_sid);
name = xmalloc(STRLEN(sid_buf) + STRLEN(s + off) + 1);
STRCPY(name, sid_buf);
STRCAT(name, s + off);
} else {
name = vim_strsave(s);
}
if (argvars[1].v_type != VAR_UNKNOWN) {
if (argvars[2].v_type != VAR_UNKNOWN) {
arg_idx = 1;
dict_idx = 2;
} else if (argvars[1].v_type == VAR_DICT) {
dict_idx = 1;
} else {
arg_idx = 1;
}
if (dict_idx > 0) {
if (argvars[dict_idx].v_type != VAR_DICT) {
EMSG(_("E922: expected a dict"));
xfree(name);
goto theend;
}
if (argvars[dict_idx].vval.v_dict == NULL) {
dict_idx = 0;
}
}
if (arg_idx > 0) {
if (argvars[arg_idx].v_type != VAR_LIST) {
EMSG(_("E923: Second argument of function() must be "
"a list or a dict"));
xfree(name);
goto theend;
}
list = argvars[arg_idx].vval.v_list;
if (tv_list_len(list) == 0) {
arg_idx = 0;
}
}
}
if (dict_idx > 0 || arg_idx > 0 || arg_pt != NULL || is_funcref) {
partial_T *const pt = xcalloc(1, sizeof(*pt));
if (arg_idx > 0 || (arg_pt != NULL && arg_pt->pt_argc > 0)) {
const int arg_len = (arg_pt == NULL ? 0 : arg_pt->pt_argc);
const int lv_len = tv_list_len(list);
pt->pt_argc = arg_len + lv_len;
pt->pt_argv = xmalloc(sizeof(pt->pt_argv[0]) * pt->pt_argc);
int i = 0;
for (; i < arg_len; i++) {
tv_copy(&arg_pt->pt_argv[i], &pt->pt_argv[i]);
}
if (lv_len > 0) {
TV_LIST_ITER(list, li, {
tv_copy(TV_LIST_ITEM_TV(li), &pt->pt_argv[i++]);
});
}
}
if (dict_idx > 0) {
pt->pt_dict = argvars[dict_idx].vval.v_dict;
(pt->pt_dict->dv_refcount)++;
} else if (arg_pt != NULL) {
pt->pt_dict = arg_pt->pt_dict;
pt->pt_auto = arg_pt->pt_auto;
if (pt->pt_dict != NULL) {
(pt->pt_dict->dv_refcount)++;
}
}
pt->pt_refcount = 1;
if (arg_pt != NULL && arg_pt->pt_func != NULL) {
pt->pt_func = arg_pt->pt_func;
func_ptr_ref(pt->pt_func);
xfree(name);
} else if (is_funcref) {
pt->pt_func = find_func(trans_name);
func_ptr_ref(pt->pt_func);
xfree(name);
} else {
pt->pt_name = name;
func_ref(name);
}
rettv->v_type = VAR_PARTIAL;
rettv->vval.v_partial = pt;
} else {
rettv->v_type = VAR_FUNC;
rettv->vval.v_string = name;
func_ref(name);
}
}
theend:
xfree(trans_name);
}
dict_T *get_buffer_info(buf_T *buf)
{
dict_T *const dict = tv_dict_alloc();
tv_dict_add_nr(dict, S_LEN("bufnr"), buf->b_fnum);
tv_dict_add_str(dict, S_LEN("name"),
buf->b_ffname != NULL ? (const char *)buf->b_ffname : "");
tv_dict_add_nr(dict, S_LEN("lnum"),
buf == curbuf ? curwin->w_cursor.lnum : buflist_findlnum(buf));
tv_dict_add_nr(dict, S_LEN("linecount"), buf->b_ml.ml_line_count);
tv_dict_add_nr(dict, S_LEN("loaded"), buf->b_ml.ml_mfp != NULL);
tv_dict_add_nr(dict, S_LEN("listed"), buf->b_p_bl);
tv_dict_add_nr(dict, S_LEN("changed"), bufIsChanged(buf));
tv_dict_add_nr(dict, S_LEN("changedtick"), buf_get_changedtick(buf));
tv_dict_add_nr(dict, S_LEN("hidden"),
buf->b_ml.ml_mfp != NULL && buf->b_nwindows == 0);
tv_dict_add_dict(dict, S_LEN("variables"), buf->b_vars);
list_T *const windows = tv_list_alloc(kListLenMayKnow);
FOR_ALL_TAB_WINDOWS(tp, wp) {
if (wp->w_buffer == buf) {
tv_list_append_number(windows, (varnumber_T)wp->handle);
}
}
tv_dict_add_list(dict, S_LEN("windows"), windows);
if (buf->b_signlist != NULL) {
tv_dict_add_list(dict, S_LEN("signs"), get_buffer_signs(buf));
}
return dict;
}
linenr_T tv_get_lnum_buf(const typval_T *const tv,
const buf_T *const buf)
FUNC_ATTR_NONNULL_ARG(1) FUNC_ATTR_WARN_UNUSED_RESULT
{
if (tv->v_type == VAR_STRING
&& tv->vval.v_string != NULL
&& tv->vval.v_string[0] == '$'
&& buf != NULL) {
return buf->b_ml.ml_line_count;
}
return tv_get_number_chk(tv, NULL);
}
void get_qf_loc_list(int is_qf, win_T *wp, typval_T *what_arg,
typval_T *rettv)
{
if (what_arg->v_type == VAR_UNKNOWN) {
tv_list_alloc_ret(rettv, kListLenMayKnow);
if (is_qf || wp != NULL) {
(void)get_errorlist(NULL, wp, -1, rettv->vval.v_list);
}
} else {
tv_dict_alloc_ret(rettv);
if (is_qf || wp != NULL) {
if (what_arg->v_type == VAR_DICT) {
dict_T *d = what_arg->vval.v_dict;
if (d != NULL) {
qf_get_properties(wp, d, rettv->vval.v_dict);
}
} else {
EMSG(_(e_dictreq));
}
}
}
}
dict_T *get_tabpage_info(tabpage_T *tp, int tp_idx)
{
dict_T *const dict = tv_dict_alloc();
tv_dict_add_nr(dict, S_LEN("tabnr"), tp_idx);
list_T *const l = tv_list_alloc(kListLenMayKnow);
FOR_ALL_WINDOWS_IN_TAB(wp, tp) {
tv_list_append_number(l, (varnumber_T)wp->handle);
}
tv_dict_add_list(dict, S_LEN("windows"), l);
tv_dict_add_dict(dict, S_LEN("variables"), tp->tp_vars);
return dict;
}
dict_T *get_win_info(win_T *wp, int16_t tpnr, int16_t winnr)
{
dict_T *const dict = tv_dict_alloc();
tv_dict_add_nr(dict, S_LEN("tabnr"), tpnr);
tv_dict_add_nr(dict, S_LEN("winnr"), winnr);
tv_dict_add_nr(dict, S_LEN("winid"), wp->handle);
tv_dict_add_nr(dict, S_LEN("height"), wp->w_height);
tv_dict_add_nr(dict, S_LEN("winrow"), wp->w_winrow + 1);
tv_dict_add_nr(dict, S_LEN("topline"), wp->w_topline);
tv_dict_add_nr(dict, S_LEN("botline"), wp->w_botline - 1);
tv_dict_add_nr(dict, S_LEN("width"), wp->w_width);
tv_dict_add_nr(dict, S_LEN("bufnr"), wp->w_buffer->b_fnum);
tv_dict_add_nr(dict, S_LEN("wincol"), wp->w_wincol + 1);
tv_dict_add_nr(dict, S_LEN("terminal"), bt_terminal(wp->w_buffer));
tv_dict_add_nr(dict, S_LEN("quickfix"), bt_quickfix(wp->w_buffer));
tv_dict_add_nr(dict, S_LEN("loclist"),
(bt_quickfix(wp->w_buffer) && wp->w_llist_ref != NULL));
tv_dict_add_dict(dict, S_LEN("variables"), wp->w_vars);
return dict;
}
win_T *
find_win_by_nr(
typval_T *vp,
tabpage_T *tp 
)
{
int nr = (int)tv_get_number_chk(vp, NULL);
if (nr < 0) {
return NULL;
}
if (nr == 0) {
return curwin;
}
if (tp == NULL) {
tp = curtab;
}
FOR_ALL_WINDOWS_IN_TAB(wp, tp) {
if (nr >= LOWEST_WIN_ID) {
if (wp->handle == nr) {
return wp;
}
} else if (--nr <= 0) {
return wp;
}
}
return NULL;
}
win_T *find_tabwin(typval_T *wvp, typval_T *tvp)
{
win_T *wp = NULL;
tabpage_T *tp = NULL;
if (wvp->v_type != VAR_UNKNOWN) {
if (tvp->v_type != VAR_UNKNOWN) {
long n = tv_get_number(tvp);
if (n >= 0) {
tp = find_tabpage(n);
}
} else {
tp = curtab;
}
if (tp != NULL) {
wp = find_win_by_nr(wvp, tp);
}
} else {
wp = curwin;
}
return wp;
}
void
getwinvar(
typval_T *argvars,
typval_T *rettv,
int off 
)
{
win_T *win, *oldcurwin;
dictitem_T *v;
tabpage_T *tp = NULL;
tabpage_T *oldtabpage = NULL;
bool done = false;
if (off == 1) {
tp = find_tabpage((int)tv_get_number_chk(&argvars[0], NULL));
} else {
tp = curtab;
}
win = find_win_by_nr(&argvars[off], tp);
const char *varname = tv_get_string_chk(&argvars[off + 1]);
rettv->v_type = VAR_STRING;
rettv->vval.v_string = NULL;
emsg_off++;
if (win != NULL && varname != NULL) {
bool need_switch_win = tp != curtab || win != curwin;
if (!need_switch_win
|| switch_win(&oldcurwin, &oldtabpage, win, tp, true) == OK) {
if (*varname == '&') {
if (varname[1] == NUL) {
dict_T *opts = get_winbuf_options(false);
if (opts != NULL) {
tv_dict_set_ret(rettv, opts);
done = true;
}
} else if (get_option_tv(&varname, rettv, 1) == OK) {
done = true;
}
} else {
v = find_var_in_ht(&win->w_vars->dv_hashtab, 'w', varname,
strlen(varname), false);
if (v != NULL) {
tv_copy(&v->di_tv, rettv);
done = true;
}
}
}
if (need_switch_win) {
restore_win(oldcurwin, oldtabpage, true);
}
}
emsg_off--;
if (!done && argvars[off + 2].v_type != VAR_UNKNOWN) {
tv_copy(&argvars[off + 2], rettv);
}
}
void get_user_input(const typval_T *const argvars,
typval_T *const rettv,
const bool inputdialog,
const bool secret)
FUNC_ATTR_NONNULL_ALL
{
rettv->v_type = VAR_STRING;
rettv->vval.v_string = NULL;
const char *prompt = "";
const char *defstr = "";
const char *cancelreturn = NULL;
const char *xp_name = NULL;
Callback input_callback = { .type = kCallbackNone };
char prompt_buf[NUMBUFLEN];
char defstr_buf[NUMBUFLEN];
char cancelreturn_buf[NUMBUFLEN];
char xp_name_buf[NUMBUFLEN];
char def[1] = { 0 };
if (argvars[0].v_type == VAR_DICT) {
if (argvars[1].v_type != VAR_UNKNOWN) {
EMSG(_("E5050: {opts} must be the only argument"));
return;
}
dict_T *const dict = argvars[0].vval.v_dict;
prompt = tv_dict_get_string_buf_chk(dict, S_LEN("prompt"), prompt_buf, "");
if (prompt == NULL) {
return;
}
defstr = tv_dict_get_string_buf_chk(dict, S_LEN("default"), defstr_buf, "");
if (defstr == NULL) {
return;
}
cancelreturn = tv_dict_get_string_buf_chk(dict, S_LEN("cancelreturn"),
cancelreturn_buf, def);
if (cancelreturn == NULL) { 
return;
}
if (*cancelreturn == NUL) {
cancelreturn = NULL;
}
xp_name = tv_dict_get_string_buf_chk(dict, S_LEN("completion"),
xp_name_buf, def);
if (xp_name == NULL) { 
return;
}
if (xp_name == def) { 
xp_name = NULL;
}
if (!tv_dict_get_callback(dict, S_LEN("highlight"), &input_callback)) {
return;
}
} else {
prompt = tv_get_string_buf_chk(&argvars[0], prompt_buf);
if (prompt == NULL) {
return;
}
if (argvars[1].v_type != VAR_UNKNOWN) {
defstr = tv_get_string_buf_chk(&argvars[1], defstr_buf);
if (defstr == NULL) {
return;
}
if (argvars[2].v_type != VAR_UNKNOWN) {
const char *const arg2 = tv_get_string_buf_chk(&argvars[2],
cancelreturn_buf);
if (arg2 == NULL) {
return;
}
if (inputdialog) {
cancelreturn = arg2;
} else {
xp_name = arg2;
}
}
}
}
int xp_type = EXPAND_NOTHING;
char *xp_arg = NULL;
if (xp_name != NULL) {
const int xp_namelen = (int)strlen(xp_name);
uint32_t argt;
if (parse_compl_arg((char_u *)xp_name, xp_namelen, &xp_type,
&argt, (char_u **)&xp_arg) == FAIL) {
return;
}
}
const bool cmd_silent_save = cmd_silent;
cmd_silent = false; 
const char *p = prompt;
if (!ui_has(kUICmdline)) {
const char *lastnl = strrchr(prompt, '\n');
if (lastnl != NULL) {
p = lastnl+1;
msg_start();
msg_clr_eos();
msg_puts_attr_len(prompt, p - prompt, echo_attr);
msg_didout = false;
msg_starthere();
}
}
cmdline_row = msg_row;
stuffReadbuffSpec(defstr);
const int save_ex_normal_busy = ex_normal_busy;
ex_normal_busy = 0;
rettv->vval.v_string =
(char_u *)getcmdline_prompt(secret ? NUL : '@', p, echo_attr,
xp_type, xp_arg, input_callback);
ex_normal_busy = save_ex_normal_busy;
callback_free(&input_callback);
if (rettv->vval.v_string == NULL && cancelreturn != NULL) {
rettv->vval.v_string = (char_u *)xstrdup(cancelreturn);
}
xfree(xp_arg);
need_wait_return = false;
msg_didout = false;
cmd_silent = cmd_silent_save;
}
void dict_list(typval_T *const tv, typval_T *const rettv,
const DictListType what)
{
if (tv->v_type != VAR_DICT) {
EMSG(_(e_dictreq));
return;
}
if (tv->vval.v_dict == NULL) {
return;
}
tv_list_alloc_ret(rettv, tv_dict_len(tv->vval.v_dict));
TV_DICT_ITER(tv->vval.v_dict, di, {
typval_T tv_item = { .v_lock = VAR_UNLOCKED };
switch (what) {
case kDictListKeys: {
tv_item.v_type = VAR_STRING;
tv_item.vval.v_string = vim_strsave(di->di_key);
break;
}
case kDictListValues: {
tv_copy(&di->di_tv, &tv_item);
break;
}
case kDictListItems: {
list_T *const sub_l = tv_list_alloc(2);
tv_item.v_type = VAR_LIST;
tv_item.vval.v_list = sub_l;
tv_list_ref(sub_l);
tv_list_append_owned_tv(sub_l, (typval_T) {
.v_type = VAR_STRING,
.v_lock = VAR_UNLOCKED,
.vval.v_string = (char_u *)xstrdup((const char *)di->di_key),
});
tv_list_append_tv(sub_l, &di->di_tv);
break;
}
}
tv_list_append_owned_tv(rettv->vval.v_list, tv_item);
});
}
char **tv_to_argv(typval_T *cmd_tv, const char **cmd, bool *executable)
{
if (cmd_tv->v_type == VAR_STRING) { 
const char *cmd_str = tv_get_string(cmd_tv);
if (cmd) {
*cmd = cmd_str;
}
return shell_build_argv(cmd_str, NULL);
}
if (cmd_tv->v_type != VAR_LIST) {
EMSG2(_(e_invarg2), "expected String or List");
return NULL;
}
list_T *argl = cmd_tv->vval.v_list;
int argc = tv_list_len(argl);
if (!argc) {
EMSG(_(e_invarg)); 
return NULL;
}
const char *arg0 = tv_get_string_chk(TV_LIST_ITEM_TV(tv_list_first(argl)));
char *exe_resolved = NULL;
if (!arg0 || !os_can_exe(arg0, &exe_resolved, true)) {
if (arg0 && executable) {
char buf[IOSIZE];
snprintf(buf, sizeof(buf), "'%s' is not executable", arg0);
EMSG3(_(e_invargNval), "cmd", buf);
*executable = false;
}
return NULL;
}
if (cmd) {
*cmd = exe_resolved;
}
int i = 0;
char **argv = xcalloc(argc + 1, sizeof(char *));
TV_LIST_ITER_CONST(argl, arg, {
const char *a = tv_get_string_chk(TV_LIST_ITEM_TV(arg));
if (!a) {
shell_free_argv(argv);
xfree(exe_resolved);
return NULL;
}
argv[i++] = xstrdup(a);
});
xfree(argv[0]);
argv[0] = exe_resolved;
return argv;
}
void mapblock_fill_dict(dict_T *const dict,
const mapblock_T *const mp,
long buffer_value,
bool compatible)
FUNC_ATTR_NONNULL_ALL
{
char *const lhs = str2special_save((const char *)mp->m_keys,
compatible, !compatible);
char *const mapmode = map_mode_to_chars(mp->m_mode);
varnumber_T noremap_value;
if (compatible) {
noremap_value = !!mp->m_noremap;
} else {
noremap_value = mp->m_noremap == REMAP_SCRIPT ? 2 : !!mp->m_noremap;
}
if (compatible) {
tv_dict_add_str(dict, S_LEN("rhs"), (const char *)mp->m_orig_str);
} else {
tv_dict_add_allocated_str(dict, S_LEN("rhs"),
str2special_save((const char *)mp->m_str, false,
true));
}
tv_dict_add_allocated_str(dict, S_LEN("lhs"), lhs);
tv_dict_add_nr(dict, S_LEN("noremap"), noremap_value);
tv_dict_add_nr(dict, S_LEN("expr"), mp->m_expr ? 1 : 0);
tv_dict_add_nr(dict, S_LEN("silent"), mp->m_silent ? 1 : 0);
tv_dict_add_nr(dict, S_LEN("sid"), (varnumber_T)mp->m_script_ctx.sc_sid);
tv_dict_add_nr(dict, S_LEN("lnum"), (varnumber_T)mp->m_script_ctx.sc_lnum);
tv_dict_add_nr(dict, S_LEN("buffer"), (varnumber_T)buffer_value);
tv_dict_add_nr(dict, S_LEN("nowait"), mp->m_nowait ? 1 : 0);
tv_dict_add_allocated_str(dict, S_LEN("mode"), mapmode);
}
int matchadd_dict_arg(typval_T *tv, const char **conceal_char,
win_T **win)
{
dictitem_T *di;
if (tv->v_type != VAR_DICT) {
EMSG(_(e_dictreq));
return FAIL;
}
if ((di = tv_dict_find(tv->vval.v_dict, S_LEN("conceal"))) != NULL) {
*conceal_char = tv_get_string(&di->di_tv);
}
if ((di = tv_dict_find(tv->vval.v_dict, S_LEN("window"))) != NULL) {
*win = find_win_by_nr_or_id(&di->di_tv);
if (*win == NULL) {
EMSG(_("E957: Invalid window number"));
return FAIL;
}
}
return OK;
}
void return_register(int regname, typval_T *rettv)
{
char_u buf[2] = { regname, 0 };
rettv->v_type = VAR_STRING;
rettv->vval.v_string = vim_strsave(buf);
}
void screenchar_adjust_grid(ScreenGrid **grid, int *row, int *col)
{
msg_scroll_flush();
if (msg_grid.chars && msg_grid.comp_index > 0 && *row >= msg_grid.comp_row
&& *row < (msg_grid.Rows + msg_grid.comp_row)
&& *col < msg_grid.Columns) {
*grid = &msg_grid;
*row -= msg_grid.comp_row;
}
}
void set_buffer_lines(buf_T *buf, linenr_T lnum_arg, bool append,
const typval_T *lines, typval_T *rettv)
FUNC_ATTR_NONNULL_ARG(4, 5)
{
linenr_T lnum = lnum_arg + (append ? 1 : 0);
const char *line = NULL;
list_T *l = NULL;
listitem_T *li = NULL;
long added = 0;
linenr_T append_lnum;
buf_T *curbuf_save = NULL;
win_T *curwin_save = NULL;
const bool is_curbuf = buf == curbuf;
if (buf == NULL || (!is_curbuf && buf->b_ml.ml_mfp == NULL) || lnum < 1) {
rettv->vval.v_number = 1; 
return;
}
if (!is_curbuf) {
curbuf_save = curbuf;
curwin_save = curwin;
curbuf = buf;
find_win_for_curbuf();
}
if (append) {
append_lnum = lnum - 1;
} else {
append_lnum = curbuf->b_ml.ml_line_count;
}
if (lines->v_type == VAR_LIST) {
l = lines->vval.v_list;
li = tv_list_first(l);
} else {
line = tv_get_string_chk(lines);
}
for (;; ) {
if (lines->v_type == VAR_LIST) {
if (li == NULL) {
break;
}
line = tv_get_string_chk(TV_LIST_ITEM_TV(li));
li = TV_LIST_ITEM_NEXT(l, li);
}
rettv->vval.v_number = 1; 
if (line == NULL || lnum > curbuf->b_ml.ml_line_count + 1) {
break;
}
if (u_sync_once == 2) {
u_sync_once = 1; 
u_sync(true);
}
if (!append && lnum <= curbuf->b_ml.ml_line_count) {
if (u_savesub(lnum) == OK
&& ml_replace(lnum, (char_u *)line, true) == OK) {
changed_bytes(lnum, 0);
if (is_curbuf && lnum == curwin->w_cursor.lnum) {
check_cursor_col();
}
rettv->vval.v_number = 0; 
}
} else if (added > 0 || u_save(lnum - 1, lnum) == OK) {
added++;
if (ml_append(lnum - 1, (char_u *)line, 0, false) == OK) {
rettv->vval.v_number = 0; 
}
}
if (l == NULL) { 
break;
}
lnum++;
}
if (added > 0) {
appended_lines_mark(append_lnum, added);
FOR_ALL_TAB_WINDOWS(tp, wp) {
if (wp->w_buffer == buf
&& (wp->w_buffer != curbuf || wp == curwin)
&& wp->w_cursor.lnum > append_lnum) {
wp->w_cursor.lnum += added;
}
}
check_cursor_col();
update_topline();
}
if (!is_curbuf) {
curbuf = curbuf_save;
curwin = curwin_save;
}
}
void setwinvar(typval_T *argvars, typval_T *rettv, int off)
{
if (check_secure()) {
return;
}
tabpage_T *tp = NULL;
if (off == 1) {
tp = find_tabpage((int)tv_get_number_chk(&argvars[0], NULL));
} else {
tp = curtab;
}
win_T *const win = find_win_by_nr(&argvars[off], tp);
const char *varname = tv_get_string_chk(&argvars[off + 1]);
typval_T *varp = &argvars[off + 2];
if (win != NULL && varname != NULL && varp != NULL) {
win_T *save_curwin;
tabpage_T *save_curtab;
bool need_switch_win = tp != curtab || win != curwin;
if (!need_switch_win
|| switch_win(&save_curwin, &save_curtab, win, tp, true) == OK) {
if (*varname == '&') {
long numval;
bool error = false;
varname++;
numval = tv_get_number_chk(varp, &error);
char nbuf[NUMBUFLEN];
const char *const strval = tv_get_string_buf_chk(varp, nbuf);
if (!error && strval != NULL) {
set_option_value(varname, numval, strval, OPT_LOCAL);
}
} else {
const size_t varname_len = strlen(varname);
char *const winvarname = xmalloc(varname_len + 3);
memcpy(winvarname, "w:", 2);
memcpy(winvarname + 2, varname, varname_len + 1);
set_var(winvarname, varname_len + 2, varp, true);
xfree(winvarname);
}
}
if (need_switch_win) {
restore_win(save_curwin, save_curtab, true);
}
}
}
void get_xdg_var_list(const XDGVarType xdg, typval_T *rettv)
FUNC_ATTR_NONNULL_ALL
{
const void *iter = NULL;
list_T *const list = tv_list_alloc(kListLenShouldKnow);
rettv->v_type = VAR_LIST;
rettv->vval.v_list = list;
tv_list_ref(list);
char *const dirs = stdpaths_get_xdg_var(xdg);
if (dirs == NULL) {
return;
}
do {
size_t dir_len;
const char *dir;
iter = vim_env_iter(':', dirs, iter, &dir, &dir_len);
if (dir != NULL && dir_len > 0) {
char *dir_with_nvim = xmemdupz(dir, dir_len);
dir_with_nvim = concat_fnames_realloc(dir_with_nvim, "nvim", true);
tv_list_append_string(list, dir_with_nvim, strlen(dir_with_nvim));
xfree(dir_with_nvim);
}
} while (iter != NULL);
xfree(dirs);
}
static list_T *string_to_list(const char *str, size_t len, const bool keepempty)
{
if (!keepempty && str[len - 1] == NL) {
len--;
}
list_T *const list = tv_list_alloc(kListLenMayKnow);
encode_list_write(list, str, len);
return list;
}
void get_system_output_as_rettv(typval_T *argvars, typval_T *rettv,
bool retlist)
{
proftime_T wait_time;
bool profiling = do_profiling == PROF_YES;
rettv->v_type = VAR_STRING;
rettv->vval.v_string = NULL;
if (check_restricted() || check_secure()) {
return;
}
ptrdiff_t input_len;
char *input = save_tv_as_string(&argvars[1], &input_len, false);
if (input_len < 0) {
assert(input == NULL);
return;
}
bool executable = true;
char **argv = tv_to_argv(&argvars[0], NULL, &executable);
if (!argv) {
if (!executable) {
set_vim_var_nr(VV_SHELL_ERROR, (long)-1);
}
xfree(input);
return; 
}
if (p_verbose > 3) {
char *cmdstr = shell_argv_to_str(argv);
verbose_enter_scroll();
smsg(_("Executing command: \"%s\""), cmdstr);
msg_puts("\n\n");
verbose_leave_scroll();
xfree(cmdstr);
}
if (profiling) {
prof_child_enter(&wait_time);
}
size_t nread = 0;
char *res = NULL;
int status = os_system(argv, input, input_len, &res, &nread);
if (profiling) {
prof_child_exit(&wait_time);
}
xfree(input);
set_vim_var_nr(VV_SHELL_ERROR, (long) status);
if (res == NULL) {
if (retlist) {
tv_list_alloc_ret(rettv, 0);
} else {
rettv->vval.v_string = (char_u *) xstrdup("");
}
return;
}
if (retlist) {
int keepempty = 0;
if (argvars[1].v_type != VAR_UNKNOWN && argvars[2].v_type != VAR_UNKNOWN) {
keepempty = tv_get_number(&argvars[2]);
}
rettv->vval.v_list = string_to_list(res, nread, (bool)keepempty);
tv_list_ref(rettv->vval.v_list);
rettv->v_type = VAR_LIST;
xfree(res);
} else {
memchrsub(res, NUL, 1, nread);
#if defined(USE_CRNL)
char *d = res;
for (char *s = res; *s; ++s) {
if (s[0] == CAR && s[1] == NL) {
++s;
}
*d++ = *s;
}
*d = NUL;
#endif
rettv->vval.v_string = (char_u *) res;
}
}
bool callback_from_typval(Callback *const callback, typval_T *const arg)
FUNC_ATTR_NONNULL_ALL FUNC_ATTR_WARN_UNUSED_RESULT
{
int r = OK;
if (arg->v_type == VAR_PARTIAL && arg->vval.v_partial != NULL) {
callback->data.partial = arg->vval.v_partial;
callback->data.partial->pt_refcount++;
callback->type = kCallbackPartial;
} else if (arg->v_type == VAR_STRING
&& arg->vval.v_string != NULL
&& ascii_isdigit(*arg->vval.v_string)) {
r = FAIL;
} else if (arg->v_type == VAR_FUNC || arg->v_type == VAR_STRING) {
char_u *name = arg->vval.v_string;
func_ref(name);
callback->data.funcref = vim_strsave(name);
callback->type = kCallbackFuncref;
} else if (arg->v_type == VAR_NUMBER && arg->vval.v_number == 0) {
callback->type = kCallbackNone;
} else {
r = FAIL;
}
if (r == FAIL) {
EMSG(_("E921: Invalid callback argument"));
return false;
}
return true;
}
bool callback_call(Callback *const callback, const int argcount_in,
typval_T *const argvars_in, typval_T *const rettv)
FUNC_ATTR_NONNULL_ALL
{
partial_T *partial;
char_u *name;
switch (callback->type) {
case kCallbackFuncref:
name = callback->data.funcref;
partial = NULL;
break;
case kCallbackPartial:
partial = callback->data.partial;
name = partial_name(partial);
break;
case kCallbackNone:
return false;
break;
default:
abort();
}
int dummy;
return call_func(name, (int)STRLEN(name), rettv, argcount_in, argvars_in,
NULL, curwin->w_cursor.lnum, curwin->w_cursor.lnum, &dummy,
true, partial, NULL);
}
static bool set_ref_in_callback(Callback *callback, int copyID,
ht_stack_T **ht_stack,
list_stack_T **list_stack)
{
typval_T tv;
switch (callback->type) {
case kCallbackFuncref:
case kCallbackNone:
break;
case kCallbackPartial:
tv.v_type = VAR_PARTIAL;
tv.vval.v_partial = callback->data.partial;
return set_ref_in_item(&tv, copyID, ht_stack, list_stack);
break;
default:
abort();
}
return false;
}
static bool set_ref_in_callback_reader(CallbackReader *reader, int copyID,
ht_stack_T **ht_stack,
list_stack_T **list_stack)
{
if (set_ref_in_callback(&reader->cb, copyID, ht_stack, list_stack)) {
return true;
}
if (reader->self) {
typval_T tv;
tv.v_type = VAR_DICT;
tv.vval.v_dict = reader->self;
return set_ref_in_item(&tv, copyID, ht_stack, list_stack);
}
return false;
}
timer_T *find_timer_by_nr(varnumber_T xx)
{
return pmap_get(uint64_t)(timers, xx);
}
void add_timer_info(typval_T *rettv, timer_T *timer)
{
list_T *list = rettv->vval.v_list;
dict_T *dict = tv_dict_alloc();
tv_list_append_dict(list, dict);
tv_dict_add_nr(dict, S_LEN("id"), timer->timer_id);
tv_dict_add_nr(dict, S_LEN("time"), timer->timeout);
tv_dict_add_nr(dict, S_LEN("paused"), timer->paused);
tv_dict_add_nr(dict, S_LEN("repeat"),
(timer->repeat_count < 0 ? -1 : timer->repeat_count));
dictitem_T *di = tv_dict_item_alloc("callback");
if (tv_dict_add(dict, di) == FAIL) {
xfree(di);
return;
}
if (timer->callback.type == kCallbackPartial) {
di->di_tv.v_type = VAR_PARTIAL;
di->di_tv.vval.v_partial = timer->callback.data.partial;
timer->callback.data.partial->pt_refcount++;
} else if (timer->callback.type == kCallbackFuncref) {
di->di_tv.v_type = VAR_FUNC;
di->di_tv.vval.v_string = vim_strsave(timer->callback.data.funcref);
}
}
void add_timer_info_all(typval_T *rettv)
{
tv_list_alloc_ret(rettv, timers->table->n_occupied);
timer_T *timer;
map_foreach_value(timers, timer, {
if (!timer->stopped) {
add_timer_info(rettv, timer);
}
})
}
void timer_due_cb(TimeWatcher *tw, void *data)
{
timer_T *timer = (timer_T *)data;
int save_did_emsg = did_emsg;
int save_called_emsg = called_emsg;
const bool save_ex_pressedreturn = get_pressedreturn();
if (timer->stopped || timer->paused) {
return;
}
timer->refcount++;
if (timer->repeat_count >= 0 && --timer->repeat_count == 0) {
timer_stop(timer);
}
typval_T argv[2] = { TV_INITIAL_VALUE, TV_INITIAL_VALUE };
argv[0].v_type = VAR_NUMBER;
argv[0].vval.v_number = timer->timer_id;
typval_T rettv = TV_INITIAL_VALUE;
called_emsg = false;
callback_call(&timer->callback, 1, argv, &rettv);
if (called_emsg && did_emsg) {
timer->emsg_count++;
if (current_exception != NULL) {
discard_current_exception();
}
}
did_emsg = save_did_emsg;
called_emsg = save_called_emsg;
set_pressedreturn(save_ex_pressedreturn);
if (timer->emsg_count >= 3) {
timer_stop(timer);
}
tv_clear(&rettv);
if (!timer->stopped && timer->timeout == 0) {
time_watcher_start(&timer->tw, timer_due_cb, 0, 0);
}
timer_decref(timer);
}
uint64_t timer_start(const long timeout,
const int repeat_count,
const Callback *const callback)
{
timer_T *timer = xmalloc(sizeof *timer);
timer->refcount = 1;
timer->stopped = false;
timer->paused = false;
timer->emsg_count = 0;
timer->repeat_count = repeat_count;
timer->timeout = timeout;
timer->timer_id = last_timer_id++;
timer->callback = *callback;
time_watcher_init(&main_loop, &timer->tw, timer);
timer->tw.events = multiqueue_new_child(main_loop.events);
timer->tw.blockable = true;
time_watcher_start(&timer->tw, timer_due_cb, timeout, timeout);
pmap_put(uint64_t)(timers, timer->timer_id, timer);
return timer->timer_id;
}
void timer_stop(timer_T *timer)
{
if (timer->stopped) {
return;
}
timer->stopped = true;
time_watcher_stop(&timer->tw);
time_watcher_close(&timer->tw, timer_close_cb);
}
static void timer_close_cb(TimeWatcher *tw, void *data)
{
timer_T *timer = (timer_T *)data;
multiqueue_free(timer->tw.events);
callback_free(&timer->callback);
pmap_del(uint64_t)(timers, timer->timer_id);
timer_decref(timer);
}
static void timer_decref(timer_T *timer)
{
if (--timer->refcount == 0) {
xfree(timer);
}
}
void timer_stop_all(void)
{
timer_T *timer;
map_foreach_value(timers, timer, {
timer_stop(timer);
})
}
void timer_teardown(void)
{
timer_stop_all();
}
bool write_list(FileDescriptor *const fp, const list_T *const list,
const bool binary)
FUNC_ATTR_NONNULL_ARG(1)
{
int error = 0;
TV_LIST_ITER_CONST(list, li, {
const char *const s = tv_get_string_chk(TV_LIST_ITEM_TV(li));
if (s == NULL) {
return false;
}
const char *hunk_start = s;
for (const char *p = hunk_start;; p++) {
if (*p == NUL || *p == NL) {
if (p != hunk_start) {
const ptrdiff_t written = file_write(fp, hunk_start,
(size_t)(p - hunk_start));
if (written < 0) {
error = (int)written;
goto write_list_error;
}
}
if (*p == NUL) {
break;
} else {
hunk_start = p + 1;
const ptrdiff_t written = file_write(fp, (char[]){ NUL }, 1);
if (written < 0) {
error = (int)written;
break;
}
}
}
}
if (!binary || TV_LIST_ITEM_NEXT(list, li) != NULL) {
const ptrdiff_t written = file_write(fp, "\n", 1);
if (written < 0) {
error = (int)written;
goto write_list_error;
}
}
});
if ((error = file_flush(fp)) != 0) {
goto write_list_error;
}
return true;
write_list_error:
emsgf(_("E80: Error while writing: %s"), os_strerror(error));
return false;
}
char *save_tv_as_string(typval_T *tv, ptrdiff_t *const len, bool endnl)
FUNC_ATTR_MALLOC FUNC_ATTR_NONNULL_ALL
{
*len = 0;
if (tv->v_type == VAR_UNKNOWN) {
return NULL;
}
if (tv->v_type != VAR_LIST && tv->v_type != VAR_NUMBER) {
const char *ret = tv_get_string_chk(tv);
if (ret) {
*len = strlen(ret);
return xmemdupz(ret, (size_t)(*len));
} else {
*len = -1;
return NULL;
}
}
if (tv->v_type == VAR_NUMBER) { 
buf_T *buf = buflist_findnr(tv->vval.v_number);
if (buf) {
for (linenr_T lnum = 1; lnum <= buf->b_ml.ml_line_count; lnum++) {
for (char_u *p = ml_get_buf(buf, lnum, false); *p != NUL; p++) {
*len += 1;
}
*len += 1;
}
} else {
EMSGN(_(e_nobufnr), tv->vval.v_number);
*len = -1;
return NULL;
}
if (*len == 0) {
return NULL;
}
char *ret = xmalloc(*len + 1);
char *end = ret;
for (linenr_T lnum = 1; lnum <= buf->b_ml.ml_line_count; lnum++) {
for (char_u *p = ml_get_buf(buf, lnum, false); *p != NUL; p++) {
*end++ = (*p == '\n') ? NUL : *p;
}
*end++ = '\n';
}
*end = NUL;
*len = end - ret;
return ret;
}
assert(tv->v_type == VAR_LIST);
list_T *list = tv->vval.v_list;
TV_LIST_ITER_CONST(list, li, {
*len += strlen(tv_get_string(TV_LIST_ITEM_TV(li))) + 1;
});
if (*len == 0) {
return NULL;
}
char *ret = xmalloc(*len + endnl);
char *end = ret;
TV_LIST_ITER_CONST(list, li, {
for (const char *s = tv_get_string(TV_LIST_ITEM_TV(li)); *s != NUL; s++) {
*end++ = (*s == '\n') ? NUL : *s;
}
if (endnl || TV_LIST_ITEM_NEXT(list, li) != NULL) {
*end++ = '\n';
}
});
*end = NUL;
*len = end - ret;
return ret;
}
pos_T *var2fpos(const typval_T *const tv, const int dollar_lnum,
int *const ret_fnum)
FUNC_ATTR_WARN_UNUSED_RESULT FUNC_ATTR_NONNULL_ALL
{
static pos_T pos;
pos_T *pp;
if (tv->v_type == VAR_LIST) {
list_T *l;
int len;
bool error = false;
listitem_T *li;
l = tv->vval.v_list;
if (l == NULL) {
return NULL;
}
pos.lnum = tv_list_find_nr(l, 0L, &error);
if (error || pos.lnum <= 0 || pos.lnum > curbuf->b_ml.ml_line_count) {
return NULL;
}
pos.col = tv_list_find_nr(l, 1L, &error);
if (error) {
return NULL;
}
len = (long)STRLEN(ml_get(pos.lnum));
li = tv_list_find(l, 1L);
if (li != NULL && TV_LIST_ITEM_TV(li)->v_type == VAR_STRING
&& TV_LIST_ITEM_TV(li)->vval.v_string != NULL
&& STRCMP(TV_LIST_ITEM_TV(li)->vval.v_string, "$") == 0) {
pos.col = len + 1;
}
if (pos.col == 0 || (int)pos.col > len + 1) {
return NULL;
}
pos.col--;
pos.coladd = tv_list_find_nr(l, 2L, &error);
if (error) {
pos.coladd = 0;
}
return &pos;
}
const char *const name = tv_get_string_chk(tv);
if (name == NULL) {
return NULL;
}
if (name[0] == '.') { 
return &curwin->w_cursor;
}
if (name[0] == 'v' && name[1] == NUL) { 
if (VIsual_active) {
return &VIsual;
}
return &curwin->w_cursor;
}
if (name[0] == '\'') { 
pp = getmark_buf_fnum(curbuf, (uint8_t)name[1], false, ret_fnum);
if (pp == NULL || pp == (pos_T *)-1 || pp->lnum <= 0) {
return NULL;
}
return pp;
}
pos.coladd = 0;
if (name[0] == 'w' && dollar_lnum) {
pos.col = 0;
if (name[1] == '0') { 
update_topline();
pos.lnum = curwin->w_topline > 0 ? curwin->w_topline : 1;
return &pos;
} else if (name[1] == '$') { 
validate_botline();
pos.lnum = curwin->w_botline > 0 ? curwin->w_botline - 1 : 0;
return &pos;
}
} else if (name[0] == '$') { 
if (dollar_lnum) {
pos.lnum = curbuf->b_ml.ml_line_count;
pos.col = 0;
} else {
pos.lnum = curwin->w_cursor.lnum;
pos.col = (colnr_T)STRLEN(get_cursor_line_ptr());
}
return &pos;
}
return NULL;
}
int list2fpos(typval_T *arg, pos_T *posp, int *fnump, colnr_T *curswantp)
{
list_T *l;
long i = 0;
long n;
if (arg->v_type != VAR_LIST
|| (l = arg->vval.v_list) == NULL
|| tv_list_len(l) < (fnump == NULL ? 2 : 3)
|| tv_list_len(l) > (fnump == NULL ? 4 : 5)) {
return FAIL;
}
if (fnump != NULL) {
n = tv_list_find_nr(l, i++, NULL); 
if (n < 0) {
return FAIL;
}
if (n == 0) {
n = curbuf->b_fnum; 
}
*fnump = n;
}
n = tv_list_find_nr(l, i++, NULL); 
if (n < 0) {
return FAIL;
}
posp->lnum = n;
n = tv_list_find_nr(l, i++, NULL); 
if (n < 0) {
return FAIL;
}
posp->col = n;
n = tv_list_find_nr(l, i, NULL); 
if (n < 0) {
posp->coladd = 0;
} else {
posp->coladd = n;
}
if (curswantp != NULL) {
*curswantp = tv_list_find_nr(l, i + 1, NULL); 
}
return OK;
}
static int get_env_len(const char_u **arg)
{
int len;
const char_u *p;
for (p = *arg; vim_isIDc(*p); p++) {
}
if (p == *arg) { 
return 0;
}
len = (int)(p - *arg);
*arg = p;
return len;
}
static int get_id_len(const char **const arg)
{
int len;
const char *p;
for (p = *arg; eval_isnamec(*p); p++) {
if (*p == ':') {
len = (int)(p - *arg);
if (len > 1
|| (len == 1 && vim_strchr(namespace_char, **arg) == NULL)) {
break;
}
}
}
if (p == *arg) { 
return 0;
}
len = (int)(p - *arg);
*arg = (const char *)skipwhite((const char_u *)p);
return len;
}
int get_name_len(const char **const arg,
char **alias,
int evaluate,
int verbose)
{
int len;
*alias = NULL; 
if ((*arg)[0] == (char)K_SPECIAL && (*arg)[1] == (char)KS_EXTRA
&& (*arg)[2] == (char)KE_SNR) {
*arg += 3;
return get_id_len(arg) + 3;
}
len = eval_fname_script(*arg);
if (len > 0) {
*arg += len;
}
char_u *expr_start;
char_u *expr_end;
const char *p = (const char *)find_name_end((char_u *)(*arg),
(const char_u **)&expr_start,
(const char_u **)&expr_end,
len > 0 ? 0 : FNE_CHECK_START);
if (expr_start != NULL) {
if (!evaluate) {
len += (int)(p - *arg);
*arg = (const char *)skipwhite((const char_u *)p);
return len;
}
char_u *temp_string = make_expanded_name((char_u *)(*arg) - len, expr_start,
expr_end, (char_u *)p);
if (temp_string == NULL) {
return -1;
}
*alias = (char *)temp_string;
*arg = (const char *)skipwhite((const char_u *)p);
return (int)STRLEN(temp_string);
}
len += get_id_len(arg);
if (len == 0 && verbose && **arg != NUL) {
EMSG2(_(e_invexpr2), *arg);
}
return len;
}
static const char_u *find_name_end(const char_u *arg, const char_u **expr_start,
const char_u **expr_end, int flags)
{
int mb_nest = 0;
int br_nest = 0;
int len;
if (expr_start != NULL) {
*expr_start = NULL;
*expr_end = NULL;
}
if ((flags & FNE_CHECK_START) && !eval_isnamec1(*arg) && *arg != '{') {
return arg;
}
const char_u *p;
for (p = arg; *p != NUL
&& (eval_isnamec(*p)
|| *p == '{'
|| ((flags & FNE_INCL_BR) && (*p == '[' || *p == '.'))
|| mb_nest != 0
|| br_nest != 0); MB_PTR_ADV(p)) {
if (*p == '\'') {
for (p = p + 1; *p != NUL && *p != '\''; MB_PTR_ADV(p)) {
}
if (*p == NUL) {
break;
}
} else if (*p == '"') {
for (p = p + 1; *p != NUL && *p != '"'; MB_PTR_ADV(p)) {
if (*p == '\\' && p[1] != NUL) {
++p;
}
}
if (*p == NUL) {
break;
}
} else if (br_nest == 0 && mb_nest == 0 && *p == ':') {
len = (int)(p - arg);
if ((len > 1 && p[-1] != '}')
|| (len == 1 && vim_strchr(namespace_char, *arg) == NULL)) {
break;
}
}
if (mb_nest == 0) {
if (*p == '[') {
++br_nest;
} else if (*p == ']') {
--br_nest;
}
}
if (br_nest == 0) {
if (*p == '{') {
mb_nest++;
if (expr_start != NULL && *expr_start == NULL) {
*expr_start = p;
}
} else if (*p == '}') {
mb_nest--;
if (expr_start != NULL && mb_nest == 0 && *expr_end == NULL) {
*expr_end = p;
}
}
}
}
return p;
}
static char_u *make_expanded_name(const char_u *in_start, char_u *expr_start,
char_u *expr_end, char_u *in_end)
{
char_u c1;
char_u *retval = NULL;
char_u *temp_result;
char_u *nextcmd = NULL;
if (expr_end == NULL || in_end == NULL)
return NULL;
*expr_start = NUL;
*expr_end = NUL;
c1 = *in_end;
*in_end = NUL;
temp_result = eval_to_string(expr_start + 1, &nextcmd, FALSE);
if (temp_result != NULL && nextcmd == NULL) {
retval = xmalloc(STRLEN(temp_result) + (expr_start - in_start)
+ (in_end - expr_end) + 1);
STRCPY(retval, in_start);
STRCAT(retval, temp_result);
STRCAT(retval, expr_end + 1);
}
xfree(temp_result);
*in_end = c1; 
*expr_start = '{';
*expr_end = '}';
if (retval != NULL) {
temp_result = (char_u *)find_name_end(retval,
(const char_u **)&expr_start,
(const char_u **)&expr_end, 0);
if (expr_start != NULL) {
temp_result = make_expanded_name(retval, expr_start,
expr_end, temp_result);
xfree(retval);
retval = temp_result;
}
}
return retval;
}
static int eval_isnamec(int c)
{
return ASCII_ISALNUM(c) || c == '_' || c == ':' || c == AUTOLOAD_CHAR;
}
static int eval_isnamec1(int c)
{
return ASCII_ISALPHA(c) || c == '_';
}
varnumber_T get_vim_var_nr(int idx) FUNC_ATTR_PURE
{
return vimvars[idx].vv_nr;
}
char_u *get_vim_var_str(int idx) FUNC_ATTR_PURE FUNC_ATTR_NONNULL_RET
{
return (char_u *)tv_get_string(&vimvars[idx].vv_tv);
}
list_T *get_vim_var_list(int idx) FUNC_ATTR_PURE
{
return vimvars[idx].vv_list;
}
dict_T *get_vim_var_dict(int idx) FUNC_ATTR_PURE
{
return vimvars[idx].vv_dict;
}
void set_vim_var_char(int c)
{
char buf[MB_MAXBYTES + 1];
buf[utf_char2bytes(c, (char_u *)buf)] = NUL;
set_vim_var_string(VV_CHAR, buf, -1);
}
void set_vcount(long count, long count1, int set_prevcount)
{
if (set_prevcount)
vimvars[VV_PREVCOUNT].vv_nr = vimvars[VV_COUNT].vv_nr;
vimvars[VV_COUNT].vv_nr = count;
vimvars[VV_COUNT1].vv_nr = count1;
}
void set_vim_var_nr(const VimVarIndex idx, const varnumber_T val)
{
tv_clear(&vimvars[idx].vv_tv);
vimvars[idx].vv_type = VAR_NUMBER;
vimvars[idx].vv_nr = val;
}
void set_vim_var_special(const VimVarIndex idx, const SpecialVarValue val)
{
tv_clear(&vimvars[idx].vv_tv);
vimvars[idx].vv_type = VAR_SPECIAL;
vimvars[idx].vv_special = val;
}
void set_vim_var_string(const VimVarIndex idx, const char *const val,
const ptrdiff_t len)
{
tv_clear(&vimvars[idx].vv_di.di_tv);
vimvars[idx].vv_type = VAR_STRING;
if (val == NULL) {
vimvars[idx].vv_str = NULL;
} else if (len == -1) {
vimvars[idx].vv_str = (char_u *) xstrdup(val);
} else {
vimvars[idx].vv_str = (char_u *) xstrndup(val, (size_t) len);
}
}
void set_vim_var_list(const VimVarIndex idx, list_T *const val)
{
tv_clear(&vimvars[idx].vv_di.di_tv);
vimvars[idx].vv_type = VAR_LIST;
vimvars[idx].vv_list = val;
if (val != NULL) {
tv_list_ref(val);
}
}
void set_vim_var_dict(const VimVarIndex idx, dict_T *const val)
{
tv_clear(&vimvars[idx].vv_di.di_tv);
vimvars[idx].vv_type = VAR_DICT;
vimvars[idx].vv_dict = val;
if (val != NULL) {
val->dv_refcount++;
tv_dict_set_keys_readonly(val);
}
}
void set_reg_var(int c)
{
char regname;
if (c == 0 || c == ' ') {
regname = '"';
} else {
regname = c;
}
if (vimvars[VV_REG].vv_str == NULL || vimvars[VV_REG].vv_str[0] != c) {
set_vim_var_string(VV_REG, &regname, 1);
}
}
char_u *v_exception(char_u *oldval)
{
if (oldval == NULL)
return vimvars[VV_EXCEPTION].vv_str;
vimvars[VV_EXCEPTION].vv_str = oldval;
return NULL;
}
char_u *v_throwpoint(char_u *oldval)
{
if (oldval == NULL)
return vimvars[VV_THROWPOINT].vv_str;
vimvars[VV_THROWPOINT].vv_str = oldval;
return NULL;
}
char_u *set_cmdarg(exarg_T *eap, char_u *oldarg)
{
char_u *oldval = vimvars[VV_CMDARG].vv_str;
if (eap == NULL) {
xfree(oldval);
vimvars[VV_CMDARG].vv_str = oldarg;
return NULL;
}
size_t len = 0;
if (eap->force_bin == FORCE_BIN)
len = 6;
else if (eap->force_bin == FORCE_NOBIN)
len = 8;
if (eap->read_edit)
len += 7;
if (eap->force_ff != 0) {
len += 10; 
}
if (eap->force_enc != 0) {
len += STRLEN(eap->cmd + eap->force_enc) + 7;
}
if (eap->bad_char != 0) {
len += 7 + 4; 
}
const size_t newval_len = len + 1;
char_u *newval = xmalloc(newval_len);
if (eap->force_bin == FORCE_BIN)
sprintf((char *)newval, " ++bin");
else if (eap->force_bin == FORCE_NOBIN)
sprintf((char *)newval, " ++nobin");
else
*newval = NUL;
if (eap->read_edit)
STRCAT(newval, " ++edit");
if (eap->force_ff != 0) {
snprintf((char *)newval + STRLEN(newval), newval_len, " ++ff=%s",
eap->force_ff == 'u' ? "unix" :
eap->force_ff == 'd' ? "dos" : "mac");
}
if (eap->force_enc != 0) {
snprintf((char *)newval + STRLEN(newval), newval_len, " ++enc=%s",
eap->cmd + eap->force_enc);
}
if (eap->bad_char == BAD_KEEP) {
STRCPY(newval + STRLEN(newval), " ++bad=keep");
} else if (eap->bad_char == BAD_DROP) {
STRCPY(newval + STRLEN(newval), " ++bad=drop");
} else if (eap->bad_char != 0) {
snprintf((char *)newval + STRLEN(newval), newval_len, " ++bad=%c",
eap->bad_char);
}
vimvars[VV_CMDARG].vv_str = newval;
return oldval;
}
int get_var_tv(
const char *name,
int len, 
typval_T *rettv, 
dictitem_T **dip, 
int verbose, 
int no_autoload 
)
{
int ret = OK;
typval_T *tv = NULL;
dictitem_T *v;
v = find_var(name, (size_t)len, NULL, no_autoload);
if (v != NULL) {
tv = &v->di_tv;
if (dip != NULL) {
*dip = v;
}
}
if (tv == NULL) {
if (rettv != NULL && verbose) {
emsgf(_("E121: Undefined variable: %.*s"), len, name);
}
ret = FAIL;
} else if (rettv != NULL) {
tv_copy(tv, rettv);
}
return ret;
}
static void check_vars(const char *name, size_t len)
{
if (eval_lavars_used == NULL) {
return;
}
const char *varname;
hashtab_T *ht = find_var_ht(name, len, &varname);
if (ht == get_funccal_local_ht() || ht == get_funccal_args_ht()) {
if (find_var(name, len, NULL, true) != NULL) {
*eval_lavars_used = true;
}
}
}
static bool tv_is_luafunc(typval_T *tv)
{
return tv->v_type == VAR_PARTIAL && tv->vval.v_partial == vvlua_partial;
}
static int check_luafunc_name(const char *str, bool paren)
{
const char *p = str;
while (ASCII_ISALNUM(*p) || *p == '_' || *p == '.') {
p++;
}
if (*p != (paren ? '(' : NUL)) {
return 0;
} else {
return (int)(p-str);
}
}
int
handle_subscript(
const char **const arg,
typval_T *rettv,
int evaluate, 
int verbose 
)
{
int ret = OK;
dict_T *selfdict = NULL;
const char_u *s;
int len;
typval_T functv;
int slen = 0;
bool lua = false;
if (tv_is_luafunc(rettv)) {
if (**arg != '.') {
tv_clear(rettv);
ret = FAIL;
} else {
(*arg)++;
lua = true;
s = (char_u *)(*arg);
slen = check_luafunc_name(*arg, true);
if (slen == 0) {
tv_clear(rettv);
ret = FAIL;
}
(*arg) += slen;
}
}
while (ret == OK
&& (**arg == '['
|| (**arg == '.' && rettv->v_type == VAR_DICT)
|| (**arg == '(' && (!evaluate || tv_is_func(*rettv))))
&& !ascii_iswhite(*(*arg - 1))) {
if (**arg == '(') {
partial_T *pt = NULL;
if (evaluate) {
functv = *rettv;
rettv->v_type = VAR_UNKNOWN;
if (functv.v_type == VAR_PARTIAL) {
pt = functv.vval.v_partial;
if (!lua) {
s = partial_name(pt);
}
} else {
s = functv.vval.v_string;
}
} else {
s = (char_u *)"";
}
ret = get_func_tv(s, lua ? slen : (int)STRLEN(s), rettv, (char_u **)arg,
curwin->w_cursor.lnum, curwin->w_cursor.lnum,
&len, evaluate, pt, selfdict);
if (evaluate) {
tv_clear(&functv);
}
if (aborting()) {
if (ret == OK) {
tv_clear(rettv);
}
ret = FAIL;
}
tv_dict_unref(selfdict);
selfdict = NULL;
} else { 
tv_dict_unref(selfdict);
if (rettv->v_type == VAR_DICT) {
selfdict = rettv->vval.v_dict;
if (selfdict != NULL)
++selfdict->dv_refcount;
} else
selfdict = NULL;
if (eval_index((char_u **)arg, rettv, evaluate, verbose) == FAIL) {
tv_clear(rettv);
ret = FAIL;
}
}
}
if (selfdict != NULL && tv_is_func(*rettv)) {
set_selfdict(rettv, selfdict);
}
tv_dict_unref(selfdict);
return ret;
}
void set_selfdict(typval_T *rettv, dict_T *selfdict)
{
if (rettv->v_type == VAR_PARTIAL && !rettv->vval.v_partial->pt_auto
&& rettv->vval.v_partial->pt_dict != NULL) {
return;
}
char_u *fname;
char_u *tofree = NULL;
ufunc_T *fp;
char_u fname_buf[FLEN_FIXED + 1];
int error;
if (rettv->v_type == VAR_PARTIAL && rettv->vval.v_partial->pt_func != NULL) {
fp = rettv->vval.v_partial->pt_func;
} else {
fname = rettv->v_type == VAR_FUNC || rettv->v_type == VAR_STRING
? rettv->vval.v_string
: rettv->vval.v_partial->pt_name;
fname = fname_trans_sid(fname, fname_buf, &tofree, &error);
fp = find_func(fname);
xfree(tofree);
}
if (fp != NULL && (fp->uf_flags & FC_DICT)) {
partial_T *pt = (partial_T *)xcalloc(1, sizeof(partial_T));
pt->pt_refcount = 1;
pt->pt_dict = selfdict;
(selfdict->dv_refcount)++;
pt->pt_auto = true;
if (rettv->v_type == VAR_FUNC || rettv->v_type == VAR_STRING) {
pt->pt_name = rettv->vval.v_string;
} else {
partial_T *ret_pt = rettv->vval.v_partial;
int i;
if (ret_pt->pt_name != NULL) {
pt->pt_name = vim_strsave(ret_pt->pt_name);
func_ref(pt->pt_name);
} else {
pt->pt_func = ret_pt->pt_func;
func_ptr_ref(pt->pt_func);
}
if (ret_pt->pt_argc > 0) {
size_t arg_size = sizeof(typval_T) * ret_pt->pt_argc;
pt->pt_argv = (typval_T *)xmalloc(arg_size);
pt->pt_argc = ret_pt->pt_argc;
for (i = 0; i < pt->pt_argc; i++) {
tv_copy(&ret_pt->pt_argv[i], &pt->pt_argv[i]);
}
}
partial_unref(ret_pt);
}
rettv->v_type = VAR_PARTIAL;
rettv->vval.v_partial = pt;
}
}
static const char *tv_stringify(typval_T *varp, char *buf)
FUNC_ATTR_NONNULL_ALL
{
if (varp->v_type == VAR_LIST
|| varp->v_type == VAR_DICT
|| varp->v_type == VAR_FUNC
|| varp->v_type == VAR_PARTIAL
|| varp->v_type == VAR_FLOAT) {
typval_T tmp;
f_string(varp, &tmp, NULL);
const char *const res = tv_get_string_buf(&tmp, buf);
tv_clear(varp);
*varp = tmp;
return res;
}
return tv_get_string_buf(varp, buf);
}
dictitem_T *find_var(const char *const name, const size_t name_len,
hashtab_T **htp, int no_autoload)
{
const char *varname;
hashtab_T *const ht = find_var_ht(name, name_len, &varname);
if (htp != NULL) {
*htp = ht;
}
if (ht == NULL) {
return NULL;
}
dictitem_T *const ret = find_var_in_ht(ht, *name,
varname,
name_len - (size_t)(varname - name),
no_autoload || htp != NULL);
if (ret != NULL) {
return ret;
}
return find_var_in_scoped_ht(name, name_len, no_autoload || htp != NULL);
}
dictitem_T *find_var_in_ht(hashtab_T *const ht,
int htname,
const char *const varname,
const size_t varname_len,
int no_autoload)
FUNC_ATTR_WARN_UNUSED_RESULT FUNC_ATTR_NONNULL_ALL
{
hashitem_T *hi;
if (varname_len == 0) {
switch (htname) {
case 's': return (dictitem_T *)&SCRIPT_SV(current_sctx.sc_sid)->sv_var;
case 'g': return (dictitem_T *)&globvars_var;
case 'v': return (dictitem_T *)&vimvars_var;
case 'b': return (dictitem_T *)&curbuf->b_bufvar;
case 'w': return (dictitem_T *)&curwin->w_winvar;
case 't': return (dictitem_T *)&curtab->tp_winvar;
case 'l': return (current_funccal == NULL
? NULL : (dictitem_T *)&current_funccal->l_vars_var);
case 'a': return (current_funccal == NULL
? NULL : (dictitem_T *)&get_funccal()->l_avars_var);
}
return NULL;
}
hi = hash_find_len(ht, varname, varname_len);
if (HASHITEM_EMPTY(hi)) {
if (ht == &globvarht && !no_autoload) {
if (!script_autoload(varname, varname_len, false) || aborting()) {
return NULL;
}
hi = hash_find_len(ht, varname, varname_len);
}
if (HASHITEM_EMPTY(hi)) {
return NULL;
}
}
return TV_DICT_HI2DI(hi);
}
static funccall_T *get_funccal(void)
{
funccall_T *funccal = current_funccal;
if (debug_backtrace_level > 0) {
for (int i = 0; i < debug_backtrace_level; i++) {
funccall_T *temp_funccal = funccal->caller;
if (temp_funccal) {
funccal = temp_funccal;
} else {
debug_backtrace_level = i;
}
}
}
return funccal;
}
static hashtab_T *get_funccal_args_ht(void)
{
if (current_funccal == NULL) {
return NULL;
}
return &get_funccal()->l_avars.dv_hashtab;
}
static hashtab_T *get_funccal_local_ht(void)
{
if (current_funccal == NULL) {
return NULL;
}
return &get_funccal()->l_vars.dv_hashtab;
}
static hashtab_T *find_var_ht_dict(const char *name, const size_t name_len,
const char **varname, dict_T **d)
{
hashitem_T *hi;
*d = NULL;
if (name_len == 0) {
return NULL;
}
if (name_len == 1 || name[1] != ':') {
if (name[0] == ':' || name[0] == AUTOLOAD_CHAR) {
return NULL;
}
*varname = name;
hi = hash_find_len(&compat_hashtab, name, name_len);
if (!HASHITEM_EMPTY(hi)) {
return &compat_hashtab;
}
if (current_funccal == NULL) {
*d = &globvardict;
} else {
*d = &get_funccal()->l_vars; 
}
goto end;
}
*varname = name + 2;
if (*name == 'g') { 
*d = &globvardict;
} else if (name_len > 2
&& (memchr(name + 2, ':', name_len - 2) != NULL
|| memchr(name + 2, AUTOLOAD_CHAR, name_len - 2) != NULL)) {
return NULL;
}
if (*name == 'b') { 
*d = curbuf->b_vars;
} else if (*name == 'w') { 
*d = curwin->w_vars;
} else if (*name == 't') { 
*d = curtab->tp_vars;
} else if (*name == 'v') { 
*d = &vimvardict;
} else if (*name == 'a' && current_funccal != NULL) { 
*d = &get_funccal()->l_avars;
} else if (*name == 'l' && current_funccal != NULL) { 
*d = &get_funccal()->l_vars;
} else if (*name == 's' 
&& current_sctx.sc_sid > 0
&& current_sctx.sc_sid <= ga_scripts.ga_len) {
*d = &SCRIPT_SV(current_sctx.sc_sid)->sv_dict;
}
end:
return *d ? &(*d)->dv_hashtab : NULL;
}
static hashtab_T *find_var_ht(const char *name, const size_t name_len,
const char **varname)
{
dict_T *d;
return find_var_ht_dict(name, name_len, varname, &d);
}
char_u *get_var_value(const char *const name)
{
dictitem_T *v;
v = find_var(name, strlen(name), NULL, false);
if (v == NULL) {
return NULL;
}
return (char_u *)tv_get_string(&v->di_tv);
}
void new_script_vars(scid_T id)
{
hashtab_T *ht;
scriptvar_T *sv;
ga_grow(&ga_scripts, (int)(id - ga_scripts.ga_len));
{
for (int i = 1; i <= ga_scripts.ga_len; ++i) {
ht = &SCRIPT_VARS(i);
if (ht->ht_mask == HT_INIT_SIZE - 1)
ht->ht_array = ht->ht_smallarray;
sv = SCRIPT_SV(i);
sv->sv_var.di_tv.vval.v_dict = &sv->sv_dict;
}
while (ga_scripts.ga_len < id) {
sv = SCRIPT_SV(ga_scripts.ga_len + 1) = xcalloc(1, sizeof(scriptvar_T));
init_var_dict(&sv->sv_dict, &sv->sv_var, VAR_SCOPE);
++ga_scripts.ga_len;
}
}
}
void init_var_dict(dict_T *dict, ScopeDictDictItem *dict_var, int scope)
{
hash_init(&dict->dv_hashtab);
dict->dv_lock = VAR_UNLOCKED;
dict->dv_scope = scope;
dict->dv_refcount = DO_NOT_FREE_CNT;
dict->dv_copyID = 0;
dict_var->di_tv.vval.v_dict = dict;
dict_var->di_tv.v_type = VAR_DICT;
dict_var->di_tv.v_lock = VAR_FIXED;
dict_var->di_flags = DI_FLAGS_RO | DI_FLAGS_FIX;
dict_var->di_key[0] = NUL;
QUEUE_INIT(&dict->watchers);
}
void unref_var_dict(dict_T *dict)
{
dict->dv_refcount -= DO_NOT_FREE_CNT - 1;
tv_dict_unref(dict);
}
void vars_clear(hashtab_T *ht)
{
vars_clear_ext(ht, TRUE);
}
static void vars_clear_ext(hashtab_T *ht, int free_val)
{
int todo;
hashitem_T *hi;
dictitem_T *v;
hash_lock(ht);
todo = (int)ht->ht_used;
for (hi = ht->ht_array; todo > 0; ++hi) {
if (!HASHITEM_EMPTY(hi)) {
--todo;
v = TV_DICT_HI2DI(hi);
if (free_val) {
tv_clear(&v->di_tv);
}
if (v->di_flags & DI_FLAGS_ALLOC) {
xfree(v);
}
}
}
hash_clear(ht);
ht->ht_used = 0;
}
static void delete_var(hashtab_T *ht, hashitem_T *hi)
{
dictitem_T *di = TV_DICT_HI2DI(hi);
hash_remove(ht, hi);
tv_clear(&di->di_tv);
xfree(di);
}
static void list_one_var(dictitem_T *v, const char *prefix, int *first)
{
char *const s = encode_tv2echo(&v->di_tv, NULL);
list_one_var_a(prefix, (const char *)v->di_key, STRLEN(v->di_key),
v->di_tv.v_type, (s == NULL ? "" : s), first);
xfree(s);
}
static void list_one_var_a(const char *prefix, const char *name,
const ptrdiff_t name_len, const int type,
const char *string, int *first)
{
msg_start();
msg_puts(prefix);
if (name != NULL) { 
msg_puts_attr_len(name, name_len, 0);
}
msg_putchar(' ');
msg_advance(22);
if (type == VAR_NUMBER) {
msg_putchar('#');
} else if (type == VAR_FUNC || type == VAR_PARTIAL) {
msg_putchar('*');
} else if (type == VAR_LIST) {
msg_putchar('[');
if (*string == '[')
++string;
} else if (type == VAR_DICT) {
msg_putchar('{');
if (*string == '{')
++string;
} else
msg_putchar(' ');
msg_outtrans((char_u *)string);
if (type == VAR_FUNC || type == VAR_PARTIAL) {
msg_puts("()");
}
if (*first) {
msg_clr_eos();
*first = FALSE;
}
}
void set_var(const char *name, const size_t name_len, typval_T *const tv,
const bool copy)
FUNC_ATTR_NONNULL_ALL
{
set_var_const(name, name_len, tv, copy, false);
}
static void set_var_const(const char *name, const size_t name_len,
typval_T *const tv, const bool copy,
const bool is_const)
FUNC_ATTR_NONNULL_ALL
{
dictitem_T *v;
hashtab_T *ht;
dict_T *dict;
const char *varname;
ht = find_var_ht_dict(name, name_len, &varname, &dict);
const bool watched = tv_dict_is_watched(dict);
if (ht == NULL || *varname == NUL) {
EMSG2(_(e_illvar), name);
return;
}
v = find_var_in_ht(ht, 0, varname, name_len - (size_t)(varname - name), true);
if (v == NULL) {
v = find_var_in_scoped_ht((const char *)name, name_len, true);
}
if (tv_is_func(*tv) && !var_check_func_name(name, v == NULL)) {
return;
}
typval_T oldtv = TV_INITIAL_VALUE;
if (v != NULL) {
if (is_const) {
EMSG(_(e_cannot_mod));
return;
}
if (var_check_ro(v->di_flags, name, name_len)
|| tv_check_lock(v->di_tv.v_lock, name, name_len)) {
return;
}
if (ht == &vimvarht) {
if (v->di_tv.v_type == VAR_STRING) {
XFREE_CLEAR(v->di_tv.vval.v_string);
if (copy || tv->v_type != VAR_STRING) {
const char *const val = tv_get_string(tv);
if (v->di_tv.vval.v_string == NULL) {
v->di_tv.vval.v_string = (char_u *)xstrdup(val);
}
} else {
v->di_tv.vval.v_string = tv->vval.v_string;
tv->vval.v_string = NULL;
}
return;
} else if (v->di_tv.v_type == VAR_NUMBER) {
v->di_tv.vval.v_number = tv_get_number(tv);
if (strcmp(varname, "searchforward") == 0) {
set_search_direction(v->di_tv.vval.v_number ? '/' : '?');
} else if (strcmp(varname, "hlsearch") == 0) {
no_hlsearch = !v->di_tv.vval.v_number;
redraw_all_later(SOME_VALID);
}
return;
} else if (v->di_tv.v_type != tv->v_type) {
EMSG2(_("E963: setting %s to value with wrong type"), name);
return;
}
}
if (watched) {
tv_copy(&v->di_tv, &oldtv);
}
tv_clear(&v->di_tv);
} else { 
if (ht == &vimvarht || ht == get_funccal_args_ht()) {
emsgf(_(e_illvar), name);
return;
}
if (!valid_varname(varname)) {
return;
}
assert(dict != NULL);
v = xmalloc(sizeof(dictitem_T) + strlen(varname));
STRCPY(v->di_key, varname);
if (tv_dict_add(dict, v) == FAIL) {
xfree(v);
return;
}
v->di_flags = DI_FLAGS_ALLOC;
if (is_const) {
v->di_flags |= DI_FLAGS_LOCK;
}
}
if (copy || tv->v_type == VAR_NUMBER || tv->v_type == VAR_FLOAT) {
tv_copy(tv, &v->di_tv);
} else {
v->di_tv = *tv;
v->di_tv.v_lock = 0;
tv_init(tv);
}
if (watched) {
if (oldtv.v_type == VAR_UNKNOWN) {
tv_dict_watcher_notify(dict, (char *)v->di_key, &v->di_tv, NULL);
} else {
tv_dict_watcher_notify(dict, (char *)v->di_key, &v->di_tv, &oldtv);
tv_clear(&oldtv);
}
}
if (is_const) {
v->di_tv.v_lock |= VAR_LOCKED;
}
}
bool var_check_ro(const int flags, const char *name,
size_t name_len)
FUNC_ATTR_WARN_UNUSED_RESULT FUNC_ATTR_NONNULL_ALL
{
const char *error_message = NULL;
if (flags & DI_FLAGS_RO) {
error_message = _(e_readonlyvar);
} else if ((flags & DI_FLAGS_RO_SBX) && sandbox) {
error_message = N_("E794: Cannot set variable in the sandbox: \"%.*s\"");
}
if (error_message == NULL) {
return false;
}
if (name_len == TV_TRANSLATE) {
name = _(name);
name_len = strlen(name);
} else if (name_len == TV_CSTRING) {
name_len = strlen(name);
}
emsgf(_(error_message), (int)name_len, name);
return true;
}
bool var_check_fixed(const int flags, const char *name,
size_t name_len)
FUNC_ATTR_WARN_UNUSED_RESULT FUNC_ATTR_NONNULL_ALL
{
if (flags & DI_FLAGS_FIX) {
if (name_len == TV_TRANSLATE) {
name = _(name);
name_len = strlen(name);
} else if (name_len == TV_CSTRING) {
name_len = strlen(name);
}
EMSG3(_("E795: Cannot delete variable %.*s"), (int)name_len, name);
return true;
}
return false;
}
bool var_check_func_name(const char *const name, const bool new_var)
FUNC_ATTR_NONNULL_ALL FUNC_ATTR_WARN_UNUSED_RESULT
{
if (!(vim_strchr((char_u *)"wbst", name[0]) != NULL && name[1] == ':')
&& !ASCII_ISUPPER((name[0] != NUL && name[1] == ':') ? name[2]
: name[0])) {
EMSG2(_("E704: Funcref variable name must start with a capital: %s"), name);
return false;
}
if (new_var && function_exists((const char *)name, false)) {
EMSG2(_("E705: Variable name conflicts with existing function: %s"),
name);
return false;
}
return true;
}
bool valid_varname(const char *varname)
FUNC_ATTR_NONNULL_ALL FUNC_ATTR_WARN_UNUSED_RESULT
{
for (const char *p = varname; *p != NUL; p++) {
if (!eval_isnamec1((int)(uint8_t)(*p))
&& (p == varname || !ascii_isdigit(*p))
&& *p != AUTOLOAD_CHAR) {
emsgf(_(e_illvar), varname);
return false;
}
}
return true;
}
int var_item_copy(const vimconv_T *const conv,
typval_T *const from,
typval_T *const to,
const bool deep,
const int copyID)
FUNC_ATTR_NONNULL_ARG(2, 3)
{
static int recurse = 0;
int ret = OK;
if (recurse >= DICT_MAXNEST) {
EMSG(_("E698: variable nested too deep for making a copy"));
return FAIL;
}
++recurse;
switch (from->v_type) {
case VAR_NUMBER:
case VAR_FLOAT:
case VAR_FUNC:
case VAR_PARTIAL:
case VAR_SPECIAL:
tv_copy(from, to);
break;
case VAR_STRING:
if (conv == NULL || conv->vc_type == CONV_NONE
|| from->vval.v_string == NULL) {
tv_copy(from, to);
} else {
to->v_type = VAR_STRING;
to->v_lock = 0;
if ((to->vval.v_string = string_convert((vimconv_T *)conv,
from->vval.v_string,
NULL))
== NULL) {
to->vval.v_string = (char_u *) xstrdup((char *) from->vval.v_string);
}
}
break;
case VAR_LIST:
to->v_type = VAR_LIST;
to->v_lock = 0;
if (from->vval.v_list == NULL) {
to->vval.v_list = NULL;
} else if (copyID != 0 && tv_list_copyid(from->vval.v_list) == copyID) {
to->vval.v_list = tv_list_latest_copy(from->vval.v_list);
tv_list_ref(to->vval.v_list);
} else {
to->vval.v_list = tv_list_copy(conv, from->vval.v_list, deep, copyID);
}
if (to->vval.v_list == NULL && from->vval.v_list != NULL) {
ret = FAIL;
}
break;
case VAR_DICT:
to->v_type = VAR_DICT;
to->v_lock = 0;
if (from->vval.v_dict == NULL)
to->vval.v_dict = NULL;
else if (copyID != 0 && from->vval.v_dict->dv_copyID == copyID) {
to->vval.v_dict = from->vval.v_dict->dv_copydict;
++to->vval.v_dict->dv_refcount;
} else {
to->vval.v_dict = tv_dict_copy(conv, from->vval.v_dict, deep, copyID);
}
if (to->vval.v_dict == NULL && from->vval.v_dict != NULL) {
ret = FAIL;
}
break;
case VAR_UNKNOWN:
internal_error("var_item_copy(UNKNOWN)");
ret = FAIL;
}
--recurse;
return ret;
}
void ex_echo(exarg_T *eap)
{
char_u *arg = eap->arg;
typval_T rettv;
bool atstart = true;
bool need_clear = true;
const int did_emsg_before = did_emsg;
if (eap->skip)
++emsg_skip;
while (*arg != NUL && *arg != '|' && *arg != '\n' && !got_int) {
need_clr_eos = true;
{
char_u *p = arg;
if (eval1(&arg, &rettv, !eap->skip) == FAIL) {
if (!aborting() && did_emsg == did_emsg_before) {
EMSG2(_(e_invexpr2), p);
}
need_clr_eos = false;
break;
}
need_clr_eos = false;
}
if (!eap->skip) {
if (atstart) {
atstart = false;
if (eap->cmdidx == CMD_echo) {
msg_sb_eol();
msg_start();
}
} else if (eap->cmdidx == CMD_echo) {
msg_puts_attr(" ", echo_attr);
}
char *tofree = encode_tv2echo(&rettv, NULL);
if (*tofree != NUL) {
msg_ext_set_kind("echo");
msg_multiline_attr(tofree, echo_attr, true, &need_clear);
}
xfree(tofree);
}
tv_clear(&rettv);
arg = skipwhite(arg);
}
eap->nextcmd = check_nextcmd(arg);
if (eap->skip) {
emsg_skip--;
} else {
if (need_clear) {
msg_clr_eos();
}
if (eap->cmdidx == CMD_echo) {
msg_end();
}
}
}
void ex_echohl(exarg_T *eap)
{
int id;
id = syn_name2id(eap->arg);
if (id == 0)
echo_attr = 0;
else
echo_attr = syn_id2attr(id);
}
void ex_execute(exarg_T *eap)
{
char_u *arg = eap->arg;
typval_T rettv;
int ret = OK;
garray_T ga;
int save_did_emsg;
ga_init(&ga, 1, 80);
if (eap->skip)
++emsg_skip;
while (*arg != NUL && *arg != '|' && *arg != '\n') {
ret = eval1_emsg(&arg, &rettv, !eap->skip);
if (ret == FAIL) {
break;
}
if (!eap->skip) {
char buf[NUMBUFLEN];
const char *const argstr = eap->cmdidx == CMD_execute
? tv_get_string_buf(&rettv, buf)
: tv_stringify(&rettv, buf);
const size_t len = strlen(argstr);
ga_grow(&ga, len + 2);
if (!GA_EMPTY(&ga)) {
((char_u *)(ga.ga_data))[ga.ga_len++] = ' ';
}
memcpy((char_u *)(ga.ga_data) + ga.ga_len, argstr, len + 1);
ga.ga_len += len;
}
tv_clear(&rettv);
arg = skipwhite(arg);
}
if (ret != FAIL && ga.ga_data != NULL) {
if (eap->cmdidx == CMD_echomsg || eap->cmdidx == CMD_echoerr) {
msg_sb_eol();
}
if (eap->cmdidx == CMD_echomsg) {
msg_ext_set_kind("echomsg");
MSG_ATTR(ga.ga_data, echo_attr);
ui_flush();
} else if (eap->cmdidx == CMD_echoerr) {
save_did_emsg = did_emsg;
msg_ext_set_kind("echoerr");
EMSG((char_u *)ga.ga_data);
if (!force_abort)
did_emsg = save_did_emsg;
} else if (eap->cmdidx == CMD_execute)
do_cmdline((char_u *)ga.ga_data,
eap->getline, eap->cookie, DOCMD_NOWAIT|DOCMD_VERBOSE);
}
ga_clear(&ga);
if (eap->skip)
--emsg_skip;
eap->nextcmd = check_nextcmd(arg);
}
static const char *find_option_end(const char **const arg, int *const opt_flags)
{
const char *p = *arg;
++p;
if (*p == 'g' && p[1] == ':') {
*opt_flags = OPT_GLOBAL;
p += 2;
} else if (*p == 'l' && p[1] == ':') {
*opt_flags = OPT_LOCAL;
p += 2;
} else {
*opt_flags = 0;
}
if (!ASCII_ISALPHA(*p)) {
return NULL;
}
*arg = p;
if (p[0] == 't' && p[1] == '_' && p[2] != NUL && p[3] != NUL) {
p += 4; 
} else {
while (ASCII_ISALPHA(*p)) {
p++;
}
}
return p;
}
void ex_function(exarg_T *eap)
{
char_u *theline;
char_u *line_to_free = NULL;
int c;
int saved_did_emsg;
int saved_wait_return = need_wait_return;
char_u *name = NULL;
char_u *p;
char_u *arg;
char_u *line_arg = NULL;
garray_T newargs;
garray_T newlines;
int varargs = false;
int flags = 0;
ufunc_T *fp;
bool overwrite = false;
int indent;
int nesting;
dictitem_T *v;
funcdict_T fudi;
static int func_nr = 0; 
int paren;
hashtab_T *ht;
int todo;
hashitem_T *hi;
linenr_T sourcing_lnum_off;
linenr_T sourcing_lnum_top;
bool is_heredoc = false;
char_u *skip_until = NULL;
char_u *heredoc_trimmed = NULL;
bool show_block = false;
bool do_concat = true;
if (ends_excmd(*eap->arg)) {
if (!eap->skip) {
todo = (int)func_hashtab.ht_used;
for (hi = func_hashtab.ht_array; todo > 0 && !got_int; ++hi) {
if (!HASHITEM_EMPTY(hi)) {
--todo;
fp = HI2UF(hi);
if (message_filtered(fp->uf_name)) {
continue;
}
if (!func_name_refcount(fp->uf_name)) {
list_func_head(fp, false, false);
}
}
}
}
eap->nextcmd = check_nextcmd(eap->arg);
return;
}
if (*eap->arg == '/') {
p = skip_regexp(eap->arg + 1, '/', TRUE, NULL);
if (!eap->skip) {
regmatch_T regmatch;
c = *p;
*p = NUL;
regmatch.regprog = vim_regcomp(eap->arg + 1, RE_MAGIC);
*p = c;
if (regmatch.regprog != NULL) {
regmatch.rm_ic = p_ic;
todo = (int)func_hashtab.ht_used;
for (hi = func_hashtab.ht_array; todo > 0 && !got_int; ++hi) {
if (!HASHITEM_EMPTY(hi)) {
--todo;
fp = HI2UF(hi);
if (!isdigit(*fp->uf_name)
&& vim_regexec(&regmatch, fp->uf_name, 0))
list_func_head(fp, false, false);
}
}
vim_regfree(regmatch.regprog);
}
}
if (*p == '/')
++p;
eap->nextcmd = check_nextcmd(p);
return;
}
p = eap->arg;
name = trans_function_name(&p, eap->skip, TFN_NO_AUTOLOAD, &fudi, NULL);
paren = (vim_strchr(p, '(') != NULL);
if (name == NULL && (fudi.fd_dict == NULL || !paren) && !eap->skip) {
if (!aborting()) {
if (fudi.fd_newkey != NULL) {
EMSG2(_(e_dictkey), fudi.fd_newkey);
}
xfree(fudi.fd_newkey);
return;
} else
eap->skip = TRUE;
}
saved_did_emsg = did_emsg;
did_emsg = FALSE;
if (!paren) {
if (!ends_excmd(*skipwhite(p))) {
EMSG(_(e_trailing));
goto ret_free;
}
eap->nextcmd = check_nextcmd(p);
if (eap->nextcmd != NULL)
*p = NUL;
if (!eap->skip && !got_int) {
fp = find_func(name);
if (fp != NULL) {
list_func_head(fp, !eap->forceit, eap->forceit);
for (int j = 0; j < fp->uf_lines.ga_len && !got_int; j++) {
if (FUNCLINE(fp, j) == NULL) {
continue;
}
msg_putchar('\n');
if (!eap->forceit) {
msg_outnum((long)j + 1);
if (j < 9) {
msg_putchar(' ');
}
if (j < 99) {
msg_putchar(' ');
}
}
msg_prt_line(FUNCLINE(fp, j), false);
ui_flush(); 
os_breakcheck();
}
if (!got_int) {
msg_putchar('\n');
msg_puts(eap->forceit ? "endfunction" : " endfunction");
}
} else
emsg_funcname(N_("E123: Undefined function: %s"), name);
}
goto ret_free;
}
p = skipwhite(p);
if (*p != '(') {
if (!eap->skip) {
EMSG2(_("E124: Missing '(': %s"), eap->arg);
goto ret_free;
}
if (vim_strchr(p, '(') != NULL)
p = vim_strchr(p, '(');
}
p = skipwhite(p + 1);
ga_init(&newargs, (int)sizeof(char_u *), 3);
ga_init(&newlines, (int)sizeof(char_u *), 3);
if (!eap->skip) {
if (name != NULL)
arg = name;
else
arg = fudi.fd_newkey;
if (arg != NULL && (fudi.fd_di == NULL || !tv_is_func(fudi.fd_di->di_tv))) {
int j = (*arg == K_SPECIAL) ? 3 : 0;
while (arg[j] != NUL && (j == 0 ? eval_isnamec1(arg[j])
: eval_isnamec(arg[j])))
++j;
if (arg[j] != NUL)
emsg_funcname((char *)e_invarg2, arg);
}
if (fudi.fd_dict != NULL && fudi.fd_dict->dv_scope == VAR_DEF_SCOPE)
EMSG(_("E862: Cannot use g: here"));
}
if (get_function_args(&p, ')', &newargs, &varargs, eap->skip) == FAIL) {
goto errret_2;
}
if (KeyTyped && ui_has(kUICmdline)) {
show_block = true;
ui_ext_cmdline_block_append(0, (const char *)eap->cmd);
}
for (;; ) {
p = skipwhite(p);
if (STRNCMP(p, "range", 5) == 0) {
flags |= FC_RANGE;
p += 5;
} else if (STRNCMP(p, "dict", 4) == 0) {
flags |= FC_DICT;
p += 4;
} else if (STRNCMP(p, "abort", 5) == 0) {
flags |= FC_ABORT;
p += 5;
} else if (STRNCMP(p, "closure", 7) == 0) {
flags |= FC_CLOSURE;
p += 7;
if (current_funccal == NULL) {
emsg_funcname(N_
("E932: Closure function should not be at top level: %s"),
name == NULL ? (char_u *)"" : name);
goto erret;
}
} else {
break;
}
}
if (*p == '\n') {
line_arg = p + 1;
} else if (*p != NUL && *p != '"' && !eap->skip && !did_emsg) {
EMSG(_(e_trailing));
}
if (KeyTyped) {
if (!eap->skip && !eap->forceit) {
if (fudi.fd_dict != NULL && fudi.fd_newkey == NULL)
EMSG(_(e_funcdict));
else if (name != NULL && find_func(name) != NULL)
emsg_funcname(e_funcexts, name);
}
if (!eap->skip && did_emsg)
goto erret;
if (!ui_has(kUICmdline)) {
msg_putchar('\n'); 
}
cmdline_row = msg_row;
}
sourcing_lnum_top = sourcing_lnum;
indent = 2;
nesting = 0;
for (;; ) {
if (KeyTyped) {
msg_scroll = true;
saved_wait_return = false;
}
need_wait_return = false;
if (line_arg != NULL) {
theline = line_arg;
p = vim_strchr(theline, '\n');
if (p == NULL)
line_arg += STRLEN(line_arg);
else {
*p = NUL;
line_arg = p + 1;
}
} else {
xfree(line_to_free);
if (eap->getline == NULL) {
theline = getcmdline(':', 0L, indent, do_concat);
} else {
theline = eap->getline(':', eap->cookie, indent, do_concat);
}
line_to_free = theline;
}
if (KeyTyped) {
lines_left = Rows - 1;
}
if (theline == NULL) {
EMSG(_("E126: Missing :endfunction"));
goto erret;
}
if (show_block) {
assert(indent >= 0);
ui_ext_cmdline_block_append((size_t)indent, (const char *)theline);
}
sourcing_lnum_off = get_sourced_lnum(eap->getline, eap->cookie);
if (sourcing_lnum < sourcing_lnum_off) {
sourcing_lnum_off -= sourcing_lnum;
} else {
sourcing_lnum_off = 0;
}
if (skip_until != NULL) {
if (heredoc_trimmed == NULL
|| (is_heredoc && skipwhite(theline) == theline)
|| STRNCMP(theline, heredoc_trimmed,
STRLEN(heredoc_trimmed)) == 0) {
if (heredoc_trimmed == NULL) {
p = theline;
} else if (is_heredoc) {
p = skipwhite(theline) == theline
? theline : theline + STRLEN(heredoc_trimmed);
} else {
p = theline + STRLEN(heredoc_trimmed);
}
if (STRCMP(p, skip_until) == 0) {
XFREE_CLEAR(skip_until);
XFREE_CLEAR(heredoc_trimmed);
do_concat = true;
is_heredoc = false;
}
}
} else {
for (p = theline; ascii_iswhite(*p) || *p == ':'; ++p)
;
if (checkforcmd(&p, "endfunction", 4) && nesting-- == 0) {
if (*p == '!') {
p++;
}
char_u *nextcmd = NULL;
if (*p == '|') {
nextcmd = p + 1;
} else if (line_arg != NULL && *skipwhite(line_arg) != NUL) {
nextcmd = line_arg;
} else if (*p != NUL && *p != '"' && p_verbose > 0) {
give_warning2((char_u *)_("W22: Text found after :endfunction: %s"),
p, true);
}
if (nextcmd != NULL) {
eap->nextcmd = nextcmd;
if (line_to_free != NULL) {
xfree(*eap->cmdlinep);
*eap->cmdlinep = line_to_free;
line_to_free = NULL;
}
}
break;
}
if (indent > 2 && STRNCMP(p, "end", 3) == 0)
indent -= 2;
else if (STRNCMP(p, "if", 2) == 0
|| STRNCMP(p, "wh", 2) == 0
|| STRNCMP(p, "for", 3) == 0
|| STRNCMP(p, "try", 3) == 0)
indent += 2;
if (checkforcmd(&p, "function", 2)) {
if (*p == '!') {
p = skipwhite(p + 1);
}
p += eval_fname_script((const char *)p);
xfree(trans_function_name(&p, true, 0, NULL, NULL));
if (*skipwhite(p) == '(') {
nesting++;
indent += 2;
}
}
p = skip_range(p, NULL);
if ((p[0] == 'a' && (!ASCII_ISALPHA(p[1]) || p[1] == 'p'))
|| (p[0] == 'c'
&& (!ASCII_ISALPHA(p[1])
|| (p[1] == 'h' && (!ASCII_ISALPHA(p[2])
|| (p[2] == 'a'
&& (STRNCMP(&p[3], "nge", 3) != 0
|| !ASCII_ISALPHA(p[6])))))))
|| (p[0] == 'i'
&& (!ASCII_ISALPHA(p[1]) || (p[1] == 'n'
&& (!ASCII_ISALPHA(p[2])
|| (p[2] == 's')))))) {
skip_until = vim_strsave((char_u *)".");
}
arg = skipwhite(skiptowhite(p));
if (arg[0] == '<' && arg[1] =='<'
&& ((p[0] == 'p' && p[1] == 'y'
&& (!ASCII_ISALNUM(p[2]) || p[2] == 't'
|| ((p[2] == '3' || p[2] == 'x')
&& !ASCII_ISALPHA(p[3]))))
|| (p[0] == 'p' && p[1] == 'e'
&& (!ASCII_ISALPHA(p[2]) || p[2] == 'r'))
|| (p[0] == 't' && p[1] == 'c'
&& (!ASCII_ISALPHA(p[2]) || p[2] == 'l'))
|| (p[0] == 'l' && p[1] == 'u' && p[2] == 'a'
&& !ASCII_ISALPHA(p[3]))
|| (p[0] == 'r' && p[1] == 'u' && p[2] == 'b'
&& (!ASCII_ISALPHA(p[3]) || p[3] == 'y'))
|| (p[0] == 'm' && p[1] == 'z'
&& (!ASCII_ISALPHA(p[2]) || p[2] == 's'))
)) {
p = skipwhite(arg + 2);
if (*p == NUL)
skip_until = vim_strsave((char_u *)".");
else
skip_until = vim_strsave(p);
}
arg = skipwhite(skiptowhite(p));
if (*arg == '[') {
arg = vim_strchr(arg, ']');
}
if (arg != NULL) {
arg = skipwhite(skiptowhite(arg));
if (arg[0] == '='
&& arg[1] == '<'
&& arg[2] =='<'
&& (p[0] == 'l'
&& p[1] == 'e'
&& (!ASCII_ISALNUM(p[2])
|| (p[2] == 't' && !ASCII_ISALNUM(p[3]))))) {
p = skipwhite(arg + 3);
if (STRNCMP(p, "trim", 4) == 0) {
p = skipwhite(p + 4);
heredoc_trimmed =
vim_strnsave(theline, (int)(skipwhite(theline) - theline));
}
skip_until = vim_strnsave(p, (int)(skiptowhite(p) - p));
do_concat = false;
is_heredoc = true;
}
}
}
ga_grow(&newlines, 1 + sourcing_lnum_off);
p = vim_strsave(theline);
((char_u **)(newlines.ga_data))[newlines.ga_len++] = p;
while (sourcing_lnum_off-- > 0)
((char_u **)(newlines.ga_data))[newlines.ga_len++] = NULL;
if (line_arg != NULL && *line_arg == NUL)
line_arg = NULL;
}
if (eap->skip || did_emsg)
goto erret;
if (fudi.fd_dict == NULL) {
v = find_var((const char *)name, STRLEN(name), &ht, false);
if (v != NULL && v->di_tv.v_type == VAR_FUNC) {
emsg_funcname(N_("E707: Function name conflicts with variable: %s"),
name);
goto erret;
}
fp = find_func(name);
if (fp != NULL) {
if (!eap->forceit
&& (fp->uf_script_ctx.sc_sid != current_sctx.sc_sid
|| fp->uf_script_ctx.sc_seq == current_sctx.sc_seq)) {
emsg_funcname(e_funcexts, name);
goto erret;
}
if (fp->uf_calls > 0) {
emsg_funcname(N_("E127: Cannot redefine function %s: It is in use"),
name);
goto erret;
}
if (fp->uf_refcount > 1) {
(fp->uf_refcount)--;
fp->uf_flags |= FC_REMOVED;
fp = NULL;
overwrite = true;
} else {
XFREE_CLEAR(name);
func_clear_items(fp);
fp->uf_profiling = false;
fp->uf_prof_initialized = false;
}
}
} else {
char numbuf[20];
fp = NULL;
if (fudi.fd_newkey == NULL && !eap->forceit) {
EMSG(_(e_funcdict));
goto erret;
}
if (fudi.fd_di == NULL) {
if (tv_check_lock(fudi.fd_dict->dv_lock, (const char *)eap->arg,
TV_CSTRING)) {
goto erret;
}
} else if (tv_check_lock(fudi.fd_di->di_tv.v_lock, (const char *)eap->arg,
TV_CSTRING)) {
goto erret;
}
xfree(name);
sprintf(numbuf, "%d", ++func_nr);
name = vim_strsave((char_u *)numbuf);
}
if (fp == NULL) {
if (fudi.fd_dict == NULL && vim_strchr(name, AUTOLOAD_CHAR) != NULL) {
int slen, plen;
char_u *scriptname;
int j = FAIL;
if (sourcing_name != NULL) {
scriptname = (char_u *)autoload_name((const char *)name, STRLEN(name));
p = vim_strchr(scriptname, '/');
plen = (int)STRLEN(p);
slen = (int)STRLEN(sourcing_name);
if (slen > plen && fnamecmp(p,
sourcing_name + slen - plen) == 0)
j = OK;
xfree(scriptname);
}
if (j == FAIL) {
EMSG2(_(
"E746: Function name does not match script file name: %s"),
name);
goto erret;
}
}
fp = xcalloc(1, offsetof(ufunc_T, uf_name) + STRLEN(name) + 1);
if (fudi.fd_dict != NULL) {
if (fudi.fd_di == NULL) {
fudi.fd_di = tv_dict_item_alloc((const char *)fudi.fd_newkey);
if (tv_dict_add(fudi.fd_dict, fudi.fd_di) == FAIL) {
xfree(fudi.fd_di);
xfree(fp);
goto erret;
}
} else {
tv_clear(&fudi.fd_di->di_tv);
}
fudi.fd_di->di_tv.v_type = VAR_FUNC;
fudi.fd_di->di_tv.vval.v_string = vim_strsave(name);
flags |= FC_DICT;
}
STRCPY(fp->uf_name, name);
if (overwrite) {
hi = hash_find(&func_hashtab, name);
hi->hi_key = UF2HIKEY(fp);
} else if (hash_add(&func_hashtab, UF2HIKEY(fp)) == FAIL) {
xfree(fp);
goto erret;
}
fp->uf_refcount = 1;
}
fp->uf_args = newargs;
fp->uf_lines = newlines;
if ((flags & FC_CLOSURE) != 0) {
register_closure(fp);
} else {
fp->uf_scoped = NULL;
}
if (prof_def_func()) {
func_do_profile(fp);
}
fp->uf_varargs = varargs;
if (sandbox) {
flags |= FC_SANDBOX;
}
fp->uf_flags = flags;
fp->uf_calls = 0;
fp->uf_script_ctx = current_sctx;
fp->uf_script_ctx.sc_lnum += sourcing_lnum_top;
goto ret_free;
erret:
ga_clear_strings(&newargs);
errret_2:
ga_clear_strings(&newlines);
ret_free:
xfree(skip_until);
xfree(line_to_free);
xfree(fudi.fd_newkey);
xfree(name);
did_emsg |= saved_did_emsg;
need_wait_return |= saved_wait_return;
if (show_block) {
ui_ext_cmdline_block_leave();
}
} 
static char_u *
trans_function_name(
char_u **pp,
bool skip, 
int flags,
funcdict_T *fdp, 
partial_T **partial 
)
FUNC_ATTR_NONNULL_ARG(1)
{
char_u *name = NULL;
const char_u *start;
const char_u *end;
int lead;
int len;
lval_T lv;
if (fdp != NULL)
memset(fdp, 0, sizeof(funcdict_T));
start = *pp;
if ((*pp)[0] == K_SPECIAL && (*pp)[1] == KS_EXTRA
&& (*pp)[2] == (int)KE_SNR) {
*pp += 3;
len = get_id_len((const char **)pp) + 3;
return (char_u *)xmemdupz(start, len);
}
lead = eval_fname_script((const char *)start);
if (lead > 2) {
start += lead;
}
end = get_lval((char_u *)start, NULL, &lv, false, skip, flags | GLV_READ_ONLY,
lead > 2 ? 0 : FNE_CHECK_START);
if (end == start) {
if (!skip)
EMSG(_("E129: Function name required"));
goto theend;
}
if (end == NULL || (lv.ll_tv != NULL && (lead > 2 || lv.ll_range))) {
if (!aborting()) {
if (end != NULL) {
emsgf(_(e_invarg2), start);
}
} else {
*pp = (char_u *)find_name_end(start, NULL, NULL, FNE_INCL_BR);
}
goto theend;
}
if (lv.ll_tv != NULL) {
if (fdp != NULL) {
fdp->fd_dict = lv.ll_dict;
fdp->fd_newkey = lv.ll_newkey;
lv.ll_newkey = NULL;
fdp->fd_di = lv.ll_di;
}
if (lv.ll_tv->v_type == VAR_FUNC && lv.ll_tv->vval.v_string != NULL) {
name = vim_strsave(lv.ll_tv->vval.v_string);
*pp = (char_u *)end;
} else if (lv.ll_tv->v_type == VAR_PARTIAL
&& lv.ll_tv->vval.v_partial != NULL) {
if (lv.ll_tv->vval.v_partial == vvlua_partial && *end == '.') {
len = check_luafunc_name((const char *)end+1, true);
if (len == 0) {
EMSG2(e_invexpr2, "v:lua");
goto theend;
}
name = xmallocz(len);
memcpy(name, end+1, len);
*pp = (char_u *)end+1+len;
} else {
name = vim_strsave(partial_name(lv.ll_tv->vval.v_partial));
*pp = (char_u *)end;
}
if (partial != NULL) {
*partial = lv.ll_tv->vval.v_partial;
}
} else {
if (!skip && !(flags & TFN_QUIET) && (fdp == NULL
|| lv.ll_dict == NULL
|| fdp->fd_newkey == NULL)) {
EMSG(_(e_funcref));
} else {
*pp = (char_u *)end;
}
name = NULL;
}
goto theend;
}
if (lv.ll_name == NULL) {
*pp = (char_u *)end;
goto theend;
}
if (lv.ll_exp_name != NULL) {
len = (int)strlen(lv.ll_exp_name);
name = deref_func_name(lv.ll_exp_name, &len, partial,
flags & TFN_NO_AUTOLOAD);
if ((const char *)name == lv.ll_exp_name) {
name = NULL;
}
} else if (!(flags & TFN_NO_DEREF)) {
len = (int)(end - *pp);
name = deref_func_name((const char *)(*pp), &len, partial,
flags & TFN_NO_AUTOLOAD);
if (name == *pp) {
name = NULL;
}
}
if (name != NULL) {
name = vim_strsave(name);
*pp = (char_u *)end;
if (strncmp((char *)name, "<SNR>", 5) == 0) {
name[0] = K_SPECIAL;
name[1] = KS_EXTRA;
name[2] = (int)KE_SNR;
memmove(name + 3, name + 5, strlen((char *)name + 5) + 1);
}
goto theend;
}
if (lv.ll_exp_name != NULL) {
len = (int)strlen(lv.ll_exp_name);
if (lead <= 2 && lv.ll_name == lv.ll_exp_name
&& lv.ll_name_len >= 2 && memcmp(lv.ll_name, "s:", 2) == 0) {
lv.ll_name += 2;
lv.ll_name_len -= 2;
len -= 2;
lead = 2;
}
} else {
if (lead == 2 || (lv.ll_name[0] == 'g' && lv.ll_name[1] == ':')) {
lv.ll_name += 2;
lv.ll_name_len -= 2;
}
len = (int)((const char *)end - lv.ll_name);
}
size_t sid_buf_len = 0;
char sid_buf[20];
if (skip) {
lead = 0; 
} else if (lead > 0) {
lead = 3;
if ((lv.ll_exp_name != NULL && eval_fname_sid(lv.ll_exp_name))
|| eval_fname_sid((const char *)(*pp))) {
if (current_sctx.sc_sid <= 0) {
EMSG(_(e_usingsid));
goto theend;
}
sid_buf_len = snprintf(sid_buf, sizeof(sid_buf),
"%" PRIdSCID "_", current_sctx.sc_sid);
lead += sid_buf_len;
}
} else if (!(flags & TFN_INT)
&& builtin_function(lv.ll_name, lv.ll_name_len)) {
EMSG2(_("E128: Function name must start with a capital or \"s:\": %s"),
start);
goto theend;
}
if (!skip && !(flags & TFN_QUIET) && !(flags & TFN_NO_DEREF)) {
char_u *cp = xmemrchr(lv.ll_name, ':', lv.ll_name_len);
if (cp != NULL && cp < end) {
EMSG2(_("E884: Function name cannot contain a colon: %s"), start);
goto theend;
}
}
name = xmalloc(len + lead + 1);
if (!skip && lead > 0) {
name[0] = K_SPECIAL;
name[1] = KS_EXTRA;
name[2] = (int)KE_SNR;
if (sid_buf_len > 0) { 
memcpy(name + 3, sid_buf, sid_buf_len);
}
}
memmove(name + lead, lv.ll_name, len);
name[lead + len] = NUL;
*pp = (char_u *)end;
theend:
clear_lval(&lv);
return name;
}
static int eval_fname_script(const char *const p)
{
if (p[0] == '<'
&& (mb_strnicmp((char_u *)p + 1, (char_u *)"SID>", 4) == 0
|| mb_strnicmp((char_u *)p + 1, (char_u *)"SNR>", 4) == 0)) {
return 5;
}
if (p[0] == 's' && p[1] == ':') {
return 2;
}
return 0;
}
static inline bool eval_fname_sid(const char *const name)
FUNC_ATTR_PURE FUNC_ATTR_ALWAYS_INLINE FUNC_ATTR_WARN_UNUSED_RESULT
FUNC_ATTR_NONNULL_ALL
{
return *name == 's' || TOUPPER_ASC(name[2]) == 'I';
}
static void list_func_head(ufunc_T *fp, int indent, bool force)
{
msg_start();
if (indent)
MSG_PUTS(" ");
MSG_PUTS(force ? "function! " : "function ");
if (fp->uf_name[0] == K_SPECIAL) {
MSG_PUTS_ATTR("<SNR>", HL_ATTR(HLF_8));
msg_puts((const char *)fp->uf_name + 3);
} else {
msg_puts((const char *)fp->uf_name);
}
msg_putchar('(');
int j;
for (j = 0; j < fp->uf_args.ga_len; j++) {
if (j) {
msg_puts(", ");
}
msg_puts((const char *)FUNCARG(fp, j));
}
if (fp->uf_varargs) {
if (j) {
msg_puts(", ");
}
msg_puts("...");
}
msg_putchar(')');
if (fp->uf_flags & FC_ABORT) {
msg_puts(" abort");
}
if (fp->uf_flags & FC_RANGE) {
msg_puts(" range");
}
if (fp->uf_flags & FC_DICT) {
msg_puts(" dict");
}
if (fp->uf_flags & FC_CLOSURE) {
msg_puts(" closure");
}
msg_clr_eos();
if (p_verbose > 0) {
last_set_msg(fp->uf_script_ctx);
}
}
static ufunc_T *find_func(const char_u *name)
{
hashitem_T *hi;
hi = hash_find(&func_hashtab, name);
if (!HASHITEM_EMPTY(hi))
return HI2UF(hi);
return NULL;
}
#if defined(EXITFREE)
void free_all_functions(void)
{
hashitem_T *hi;
ufunc_T *fp;
uint64_t skipped = 0;
uint64_t todo = 1;
uint64_t used;
while (current_funccal != NULL) {
tv_clear(current_funccal->rettv);
cleanup_function_call(current_funccal);
}
while (todo > 0) {
todo = func_hashtab.ht_used;
for (hi = func_hashtab.ht_array; todo > 0; hi++) {
if (!HASHITEM_EMPTY(hi)) {
fp = HI2UF(hi);
if (func_name_refcount(fp->uf_name)) {
skipped++;
} else {
used = func_hashtab.ht_used;
func_clear(fp, true);
if (used != func_hashtab.ht_used) {
skipped = 0;
break;
}
}
todo--;
}
}
}
skipped = 0;
while (func_hashtab.ht_used > skipped) {
todo = func_hashtab.ht_used;
for (hi = func_hashtab.ht_array; todo > 0; hi++) {
if (!HASHITEM_EMPTY(hi)) {
todo--;
fp = HI2UF(hi);
if (func_name_refcount(fp->uf_name)) {
skipped++;
} else {
func_free(fp);
skipped = 0;
break;
}
}
}
}
if (skipped == 0) {
hash_clear(&func_hashtab);
}
}
#endif
bool translated_function_exists(const char *name)
{
if (builtin_function(name, -1)) {
return find_internal_func((char *)name) != NULL;
}
return find_func((const char_u *)name) != NULL;
}
bool function_exists(const char *const name, bool no_deref)
{
const char_u *nm = (const char_u *)name;
bool n = false;
int flag = TFN_INT | TFN_QUIET | TFN_NO_AUTOLOAD;
if (no_deref) {
flag |= TFN_NO_DEREF;
}
char *const p = (char *)trans_function_name((char_u **)&nm, false, flag, NULL,
NULL);
nm = skipwhite(nm);
if (p != NULL && (*nm == NUL || *nm == '(')) {
n = translated_function_exists(p);
}
xfree(p);
return n;
}
static bool builtin_function(const char *name, int len)
{
if (!ASCII_ISLOWER(name[0])) {
return false;
}
const char *p = (len == -1
? strchr(name, AUTOLOAD_CHAR)
: memchr(name, AUTOLOAD_CHAR, (size_t)len));
return p == NULL;
}
static void func_do_profile(ufunc_T *fp)
{
int len = fp->uf_lines.ga_len;
if (!fp->uf_prof_initialized) {
if (len == 0) {
len = 1; 
}
fp->uf_tm_count = 0;
fp->uf_tm_self = profile_zero();
fp->uf_tm_total = profile_zero();
if (fp->uf_tml_count == NULL) {
fp->uf_tml_count = xcalloc(len, sizeof(int));
}
if (fp->uf_tml_total == NULL) {
fp->uf_tml_total = xcalloc(len, sizeof(proftime_T));
}
if (fp->uf_tml_self == NULL) {
fp->uf_tml_self = xcalloc(len, sizeof(proftime_T));
}
fp->uf_tml_idx = -1;
fp->uf_prof_initialized = true;
}
fp->uf_profiling = TRUE;
}
void func_dump_profile(FILE *fd)
{
hashitem_T *hi;
int todo;
ufunc_T *fp;
ufunc_T **sorttab;
int st_len = 0;
todo = (int)func_hashtab.ht_used;
if (todo == 0)
return; 
sorttab = xmalloc(sizeof(ufunc_T *) * todo);
for (hi = func_hashtab.ht_array; todo > 0; ++hi) {
if (!HASHITEM_EMPTY(hi)) {
--todo;
fp = HI2UF(hi);
if (fp->uf_prof_initialized) {
sorttab[st_len++] = fp;
if (fp->uf_name[0] == K_SPECIAL) {
fprintf(fd, "FUNCTION <SNR>%s()\n", fp->uf_name + 3);
} else {
fprintf(fd, "FUNCTION %s()\n", fp->uf_name);
}
if (fp->uf_script_ctx.sc_sid != 0) {
bool should_free;
const LastSet last_set = (LastSet){
.script_ctx = fp->uf_script_ctx,
.channel_id = 0,
};
char_u *p = get_scriptname(last_set, &should_free);
fprintf(fd, " Defined: %s:%" PRIdLINENR "\n",
p, fp->uf_script_ctx.sc_lnum);
if (should_free) {
xfree(p);
}
}
if (fp->uf_tm_count == 1) {
fprintf(fd, "Called 1 time\n");
} else {
fprintf(fd, "Called %d times\n", fp->uf_tm_count);
}
fprintf(fd, "Total time: %s\n", profile_msg(fp->uf_tm_total));
fprintf(fd, " Self time: %s\n", profile_msg(fp->uf_tm_self));
fprintf(fd, "\n");
fprintf(fd, "count total (s) self (s)\n");
for (int i = 0; i < fp->uf_lines.ga_len; ++i) {
if (FUNCLINE(fp, i) == NULL)
continue;
prof_func_line(fd, fp->uf_tml_count[i],
&fp->uf_tml_total[i], &fp->uf_tml_self[i], TRUE);
fprintf(fd, "%s\n", FUNCLINE(fp, i));
}
fprintf(fd, "\n");
}
}
}
if (st_len > 0) {
qsort((void *)sorttab, (size_t)st_len, sizeof(ufunc_T *),
prof_total_cmp);
prof_sort_list(fd, sorttab, st_len, "TOTAL", FALSE);
qsort((void *)sorttab, (size_t)st_len, sizeof(ufunc_T *),
prof_self_cmp);
prof_sort_list(fd, sorttab, st_len, "SELF", TRUE);
}
xfree(sorttab);
}
static void
prof_sort_list(
FILE *fd,
ufunc_T **sorttab,
int st_len,
char *title,
int prefer_self 
)
{
int i;
ufunc_T *fp;
fprintf(fd, "FUNCTIONS SORTED ON %s TIME\n", title);
fprintf(fd, "count total (s) self (s) function\n");
for (i = 0; i < 20 && i < st_len; ++i) {
fp = sorttab[i];
prof_func_line(fd, fp->uf_tm_count, &fp->uf_tm_total, &fp->uf_tm_self,
prefer_self);
if (fp->uf_name[0] == K_SPECIAL)
fprintf(fd, " <SNR>%s()\n", fp->uf_name + 3);
else
fprintf(fd, " %s()\n", fp->uf_name);
}
fprintf(fd, "\n");
}
static void prof_func_line(
FILE *fd,
int count,
proftime_T *total,
proftime_T *self,
int prefer_self 
)
{
if (count > 0) {
fprintf(fd, "%5d ", count);
if (prefer_self && profile_equal(*total, *self))
fprintf(fd, " ");
else
fprintf(fd, "%s ", profile_msg(*total));
if (!prefer_self && profile_equal(*total, *self))
fprintf(fd, " ");
else
fprintf(fd, "%s ", profile_msg(*self));
} else
fprintf(fd, " ");
}
static int prof_total_cmp(const void *s1, const void *s2)
{
ufunc_T *p1 = *(ufunc_T **)s1;
ufunc_T *p2 = *(ufunc_T **)s2;
return profile_cmp(p1->uf_tm_total, p2->uf_tm_total);
}
static int prof_self_cmp(const void *s1, const void *s2)
{
ufunc_T *p1 = *(ufunc_T **)s1;
ufunc_T *p2 = *(ufunc_T **)s2;
return profile_cmp(p1->uf_tm_self, p2->uf_tm_self);
}
static bool script_autoload(const char *const name, const size_t name_len,
const bool reload)
{
const char *p = memchr(name, AUTOLOAD_CHAR, name_len);
if (p == NULL || p == name) {
return false;
}
bool ret = false;
char *tofree = autoload_name(name, name_len);
char *scriptname = tofree;
int i = 0;
for (; i < ga_loaded.ga_len; i++) {
if (STRCMP(((char **)ga_loaded.ga_data)[i] + 9, scriptname + 9) == 0) {
break;
}
}
if (!reload && i < ga_loaded.ga_len) {
ret = false; 
} else {
if (i == ga_loaded.ga_len) {
GA_APPEND(char *, &ga_loaded, scriptname);
tofree = NULL;
}
if (source_runtime((char_u *)scriptname, 0) == OK) {
ret = true;
}
}
xfree(tofree);
return ret;
}
static char *autoload_name(const char *const name, const size_t name_len)
FUNC_ATTR_MALLOC FUNC_ATTR_WARN_UNUSED_RESULT
{
char *const scriptname = xmalloc(name_len + sizeof("autoload/.vim"));
memcpy(scriptname, "autoload/", sizeof("autoload/") - 1);
memcpy(scriptname + sizeof("autoload/") - 1, name, name_len);
size_t auchar_idx = 0;
for (size_t i = sizeof("autoload/") - 1;
i - sizeof("autoload/") + 1 < name_len;
i++) {
if (scriptname[i] == AUTOLOAD_CHAR) {
scriptname[i] = '/';
auchar_idx = i;
}
}
memcpy(scriptname + auchar_idx, ".vim", sizeof(".vim"));
return scriptname;
}
char_u *get_user_func_name(expand_T *xp, int idx)
{
static size_t done;
static hashitem_T *hi;
ufunc_T *fp;
if (idx == 0) {
done = 0;
hi = func_hashtab.ht_array;
}
assert(hi);
if (done < func_hashtab.ht_used) {
if (done++ > 0)
++hi;
while (HASHITEM_EMPTY(hi))
++hi;
fp = HI2UF(hi);
if ((fp->uf_flags & FC_DICT)
|| STRNCMP(fp->uf_name, "<lambda>", 8) == 0) {
return (char_u *)""; 
}
if (STRLEN(fp->uf_name) + 4 >= IOSIZE) {
return fp->uf_name; 
}
cat_func_name(IObuff, fp);
if (xp->xp_context != EXPAND_USER_FUNC) {
STRCAT(IObuff, "(");
if (!fp->uf_varargs && GA_EMPTY(&fp->uf_args))
STRCAT(IObuff, ")");
}
return IObuff;
}
return NULL;
}
static void cat_func_name(char_u *buf, ufunc_T *fp)
{
if (fp->uf_name[0] == K_SPECIAL) {
STRCPY(buf, "<SNR>");
STRCAT(buf, fp->uf_name + 3);
} else
STRCPY(buf, fp->uf_name);
}
static bool func_name_refcount(char_u *name)
{
return isdigit(*name) || *name == '<';
}
void ex_delfunction(exarg_T *eap)
{
ufunc_T *fp = NULL;
char_u *p;
char_u *name;
funcdict_T fudi;
p = eap->arg;
name = trans_function_name(&p, eap->skip, 0, &fudi, NULL);
xfree(fudi.fd_newkey);
if (name == NULL) {
if (fudi.fd_dict != NULL && !eap->skip)
EMSG(_(e_funcref));
return;
}
if (!ends_excmd(*skipwhite(p))) {
xfree(name);
EMSG(_(e_trailing));
return;
}
eap->nextcmd = check_nextcmd(p);
if (eap->nextcmd != NULL)
*p = NUL;
if (!eap->skip)
fp = find_func(name);
xfree(name);
if (!eap->skip) {
if (fp == NULL) {
if (!eap->forceit) {
EMSG2(_(e_nofunc), eap->arg);
}
return;
}
if (fp->uf_calls > 0) {
EMSG2(_("E131: Cannot delete function %s: It is in use"), eap->arg);
return;
}
if (fp->uf_refcount > 2) {
EMSG2(_("Cannot delete function %s: It is being used internally"),
eap->arg);
return;
}
if (fudi.fd_dict != NULL) {
tv_dict_item_remove(fudi.fd_dict, fudi.fd_di);
} else {
if (fp->uf_refcount > (func_name_refcount(fp->uf_name) ? 0 : 1)) {
if (func_remove(fp)) {
fp->uf_refcount--;
}
fp->uf_flags |= FC_DELETED;
} else {
func_clear_free(fp, false);
}
}
}
}
static bool func_remove(ufunc_T *fp)
{
hashitem_T *hi = hash_find(&func_hashtab, UF2HIKEY(fp));
if (!HASHITEM_EMPTY(hi)) {
hash_remove(&func_hashtab, hi);
return true;
}
return false;
}
static void func_clear_items(ufunc_T *fp)
{
ga_clear_strings(&(fp->uf_args));
ga_clear_strings(&(fp->uf_lines));
XFREE_CLEAR(fp->uf_tml_count);
XFREE_CLEAR(fp->uf_tml_total);
XFREE_CLEAR(fp->uf_tml_self);
}
static void func_clear(ufunc_T *fp, bool force)
{
if (fp->uf_cleared) {
return;
}
fp->uf_cleared = true;
func_clear_items(fp);
funccal_unref(fp->uf_scoped, fp, force);
}
static void func_free(ufunc_T *fp)
{
if ((fp->uf_flags & (FC_DELETED | FC_REMOVED)) == 0) {
func_remove(fp);
}
xfree(fp);
}
static void func_clear_free(ufunc_T *fp, bool force)
{
func_clear(fp, force);
func_free(fp);
}
void func_unref(char_u *name)
{
ufunc_T *fp = NULL;
if (name == NULL || !func_name_refcount(name)) {
return;
}
fp = find_func(name);
if (fp == NULL && isdigit(*name)) {
#if defined(EXITFREE)
if (!entered_free_all_mem) {
internal_error("func_unref()");
abort();
}
#else
internal_error("func_unref()");
abort();
#endif
}
func_ptr_unref(fp);
}
void func_ptr_unref(ufunc_T *fp)
{
if (fp != NULL && --fp->uf_refcount <= 0) {
if (fp->uf_calls == 0) {
func_clear_free(fp, false);
}
}
}
void func_ref(char_u *name)
{
ufunc_T *fp;
if (name == NULL || !func_name_refcount(name)) {
return;
}
fp = find_func(name);
if (fp != NULL) {
(fp->uf_refcount)++;
} else if (isdigit(*name)) {
internal_error("func_ref()");
}
}
void func_ptr_ref(ufunc_T *fp)
{
if (fp != NULL) {
(fp->uf_refcount)++;
}
}
static inline bool fc_referenced(const funccall_T *const fc)
FUNC_ATTR_ALWAYS_INLINE FUNC_ATTR_PURE FUNC_ATTR_WARN_UNUSED_RESULT
FUNC_ATTR_NONNULL_ALL
{
return ((fc->l_varlist.lv_refcount 
!= DO_NOT_FREE_CNT)
|| fc->l_vars.dv_refcount != DO_NOT_FREE_CNT
|| fc->l_avars.dv_refcount != DO_NOT_FREE_CNT
|| fc->fc_refcount > 0);
}
void call_user_func(ufunc_T *fp, int argcount, typval_T *argvars,
typval_T *rettv, linenr_T firstline, linenr_T lastline,
dict_T *selfdict)
FUNC_ATTR_NONNULL_ARG(1, 3, 4)
{
char_u *save_sourcing_name;
linenr_T save_sourcing_lnum;
bool using_sandbox = false;
funccall_T *fc;
int save_did_emsg;
static int depth = 0;
dictitem_T *v;
int fixvar_idx = 0; 
int ai;
bool islambda = false;
char_u numbuf[NUMBUFLEN];
char_u *name;
proftime_T wait_start;
proftime_T call_start;
int started_profiling = false;
bool did_save_redo = false;
save_redo_T save_redo;
if (depth >= p_mfd) {
EMSG(_("E132: Function call depth is higher than 'maxfuncdepth'"));
rettv->v_type = VAR_NUMBER;
rettv->vval.v_number = -1;
return;
}
++depth;
save_search_patterns();
if (!ins_compl_active()) {
saveRedobuff(&save_redo);
did_save_redo = true;
}
++fp->uf_calls;
line_breakcheck();
fc = xmalloc(sizeof(funccall_T));
fc->caller = current_funccal;
current_funccal = fc;
fc->func = fp;
fc->rettv = rettv;
rettv->vval.v_number = 0;
fc->linenr = 0;
fc->returned = FALSE;
fc->level = ex_nesting_level;
fc->breakpoint = dbg_find_breakpoint(FALSE, fp->uf_name, (linenr_T)0);
fc->dbg_tick = debug_tick;
fc->fc_refcount = 0;
fc->fc_copyID = 0;
ga_init(&fc->fc_funcs, sizeof(ufunc_T *), 1);
func_ptr_ref(fp);
if (STRNCMP(fp->uf_name, "<lambda>", 8) == 0) {
islambda = true;
}
init_var_dict(&fc->l_vars, &fc->l_vars_var, VAR_DEF_SCOPE);
if (selfdict != NULL) {
v = (dictitem_T *)&fc->fixvar[fixvar_idx++];
#if !defined(__clang_analyzer__)
name = v->di_key;
STRCPY(name, "self");
#endif
v->di_flags = DI_FLAGS_RO | DI_FLAGS_FIX;
tv_dict_add(&fc->l_vars, v);
v->di_tv.v_type = VAR_DICT;
v->di_tv.v_lock = 0;
v->di_tv.vval.v_dict = selfdict;
++selfdict->dv_refcount;
}
init_var_dict(&fc->l_avars, &fc->l_avars_var, VAR_SCOPE);
add_nr_var(&fc->l_avars, (dictitem_T *)&fc->fixvar[fixvar_idx++], "0",
(varnumber_T)(argcount - fp->uf_args.ga_len));
fc->l_avars.dv_lock = VAR_FIXED;
v = (dictitem_T *)&fc->fixvar[fixvar_idx++];
#if !defined(__clang_analyzer__)
name = v->di_key;
STRCPY(name, "000");
#endif
v->di_flags = DI_FLAGS_RO | DI_FLAGS_FIX;
tv_dict_add(&fc->l_avars, v);
v->di_tv.v_type = VAR_LIST;
v->di_tv.v_lock = VAR_FIXED;
v->di_tv.vval.v_list = &fc->l_varlist;
tv_list_init_static(&fc->l_varlist);
tv_list_set_lock(&fc->l_varlist, VAR_FIXED);
add_nr_var(&fc->l_avars, (dictitem_T *)&fc->fixvar[fixvar_idx++],
"firstline", (varnumber_T)firstline);
add_nr_var(&fc->l_avars, (dictitem_T *)&fc->fixvar[fixvar_idx++],
"lastline", (varnumber_T)lastline);
for (int i = 0; i < argcount; i++) {
bool addlocal = false;
ai = i - fp->uf_args.ga_len;
if (ai < 0) {
name = FUNCARG(fp, i);
if (islambda) {
addlocal = true;
}
} else {
snprintf((char *)numbuf, sizeof(numbuf), "%d", ai + 1);
name = numbuf;
}
if (fixvar_idx < FIXVAR_CNT && STRLEN(name) <= VAR_SHORT_LEN) {
v = (dictitem_T *)&fc->fixvar[fixvar_idx++];
v->di_flags = DI_FLAGS_RO | DI_FLAGS_FIX;
} else {
v = xmalloc(sizeof(dictitem_T) + STRLEN(name));
v->di_flags = DI_FLAGS_RO | DI_FLAGS_FIX | DI_FLAGS_ALLOC;
}
STRCPY(v->di_key, name);
v->di_tv = argvars[i];
v->di_tv.v_lock = VAR_FIXED;
if (addlocal) {
tv_copy(&v->di_tv, &v->di_tv);
tv_dict_add(&fc->l_vars, v);
} else {
tv_dict_add(&fc->l_avars, v);
}
if (ai >= 0 && ai < MAX_FUNC_ARGS) {
tv_list_append(&fc->l_varlist, &fc->l_listitems[ai]);
*TV_LIST_ITEM_TV(&fc->l_listitems[ai]) = argvars[i];
TV_LIST_ITEM_TV(&fc->l_listitems[ai])->v_lock = VAR_FIXED;
}
}
++RedrawingDisabled;
save_sourcing_name = sourcing_name;
save_sourcing_lnum = sourcing_lnum;
sourcing_lnum = 1;
if (fp->uf_flags & FC_SANDBOX) {
using_sandbox = true;
sandbox++;
}
size_t len = (save_sourcing_name == NULL ? 0 : STRLEN(save_sourcing_name))
+ STRLEN(fp->uf_name) + 27;
sourcing_name = xmalloc(len);
{
if (save_sourcing_name != NULL
&& STRNCMP(save_sourcing_name, "function ", 9) == 0) {
vim_snprintf((char *)sourcing_name,
len,
"%s[%" PRId64 "]..",
save_sourcing_name,
(int64_t)save_sourcing_lnum);
} else {
STRCPY(sourcing_name, "function ");
}
cat_func_name(sourcing_name + STRLEN(sourcing_name), fp);
if (p_verbose >= 12) {
++no_wait_return;
verbose_enter_scroll();
smsg(_("calling %s"), sourcing_name);
if (p_verbose >= 14) {
msg_puts("(");
for (int i = 0; i < argcount; i++) {
if (i > 0) {
msg_puts(", ");
}
if (argvars[i].v_type == VAR_NUMBER) {
msg_outnum((long)argvars[i].vval.v_number);
} else {
emsg_off++;
char *tofree = encode_tv2string(&argvars[i], NULL);
emsg_off--;
if (tofree != NULL) {
char *s = tofree;
char buf[MSG_BUF_LEN];
if (vim_strsize((char_u *)s) > MSG_BUF_CLEN) {
trunc_string((char_u *)s, (char_u *)buf, MSG_BUF_CLEN,
sizeof(buf));
s = buf;
}
msg_puts(s);
xfree(tofree);
}
}
}
msg_puts(")");
}
msg_puts("\n"); 
verbose_leave_scroll();
--no_wait_return;
}
}
const bool do_profiling_yes = do_profiling == PROF_YES;
bool func_not_yet_profiling_but_should =
do_profiling_yes
&& !fp->uf_profiling && has_profiling(false, fp->uf_name, NULL);
if (func_not_yet_profiling_but_should) {
started_profiling = true;
func_do_profile(fp);
}
bool func_or_func_caller_profiling =
do_profiling_yes
&& (fp->uf_profiling
|| (fc->caller != NULL && fc->caller->func->uf_profiling));
if (func_or_func_caller_profiling) {
++fp->uf_tm_count;
call_start = profile_start();
fp->uf_tm_children = profile_zero();
}
if (do_profiling_yes) {
script_prof_save(&wait_start);
}
const sctx_T save_current_sctx = current_sctx;
current_sctx = fp->uf_script_ctx;
save_did_emsg = did_emsg;
did_emsg = FALSE;
do_cmdline(NULL, get_func_line, (void *)fc,
DOCMD_NOWAIT|DOCMD_VERBOSE|DOCMD_REPEAT);
--RedrawingDisabled;
if ((did_emsg
&& (fp->uf_flags & FC_ABORT)) || rettv->v_type == VAR_UNKNOWN) {
tv_clear(rettv);
rettv->v_type = VAR_NUMBER;
rettv->vval.v_number = -1;
}
if (func_or_func_caller_profiling) {
call_start = profile_end(call_start);
call_start = profile_sub_wait(wait_start, call_start); 
fp->uf_tm_total = profile_add(fp->uf_tm_total, call_start);
fp->uf_tm_self = profile_self(fp->uf_tm_self, call_start,
fp->uf_tm_children);
if (fc->caller != NULL && fc->caller->func->uf_profiling) {
fc->caller->func->uf_tm_children =
profile_add(fc->caller->func->uf_tm_children, call_start);
fc->caller->func->uf_tml_children =
profile_add(fc->caller->func->uf_tml_children, call_start);
}
if (started_profiling) {
fp->uf_profiling = false;
}
}
if (p_verbose >= 12) {
++no_wait_return;
verbose_enter_scroll();
if (aborting())
smsg(_("%s aborted"), sourcing_name);
else if (fc->rettv->v_type == VAR_NUMBER)
smsg(_("%s returning #%" PRId64 ""),
sourcing_name, (int64_t)fc->rettv->vval.v_number);
else {
char_u buf[MSG_BUF_LEN];
emsg_off++;
char_u *s = (char_u *) encode_tv2string(fc->rettv, NULL);
char_u *tofree = s;
emsg_off--;
if (s != NULL) {
if (vim_strsize(s) > MSG_BUF_CLEN) {
trunc_string(s, buf, MSG_BUF_CLEN, MSG_BUF_LEN);
s = buf;
}
smsg(_("%s returning %s"), sourcing_name, s);
xfree(tofree);
}
}
msg_puts("\n"); 
verbose_leave_scroll();
--no_wait_return;
}
xfree(sourcing_name);
sourcing_name = save_sourcing_name;
sourcing_lnum = save_sourcing_lnum;
current_sctx = save_current_sctx;
if (do_profiling_yes) {
script_prof_restore(&wait_start);
}
if (using_sandbox) {
sandbox--;
}
if (p_verbose >= 12 && sourcing_name != NULL) {
++no_wait_return;
verbose_enter_scroll();
smsg(_("continuing in %s"), sourcing_name);
msg_puts("\n"); 
verbose_leave_scroll();
--no_wait_return;
}
did_emsg |= save_did_emsg;
depth--;
cleanup_function_call(fc);
if (--fp->uf_calls <= 0 && fp->uf_refcount <= 0) {
func_clear_free(fp, false);
}
if (did_save_redo) {
restoreRedobuff(&save_redo);
}
restore_search_patterns();
}
static void funccal_unref(funccall_T *fc, ufunc_T *fp, bool force)
{
funccall_T **pfc;
int i;
if (fc == NULL) {
return;
}
fc->fc_refcount--;
if (force ? fc->fc_refcount <= 0 : !fc_referenced(fc)) {
for (pfc = &previous_funccal; *pfc != NULL; pfc = &(*pfc)->caller) {
if (fc == *pfc) {
*pfc = fc->caller;
free_funccal(fc, true);
return;
}
}
}
for (i = 0; i < fc->fc_funcs.ga_len; i++) {
if (((ufunc_T **)(fc->fc_funcs.ga_data))[i] == fp) {
((ufunc_T **)(fc->fc_funcs.ga_data))[i] = NULL;
}
}
}
static int can_free_funccal(funccall_T *fc, int copyID)
{
return fc->l_varlist.lv_copyID != copyID
&& fc->l_vars.dv_copyID != copyID
&& fc->l_avars.dv_copyID != copyID
&& fc->fc_copyID != copyID;
}
static void
free_funccal(
funccall_T *fc,
int free_val 
)
{
for (int i = 0; i < fc->fc_funcs.ga_len; i++) {
ufunc_T *fp = ((ufunc_T **)(fc->fc_funcs.ga_data))[i];
if (fp != NULL && fp->uf_scoped == fc) {
fp->uf_scoped = NULL;
}
}
ga_clear(&fc->fc_funcs);
vars_clear_ext(&fc->l_avars.dv_hashtab, free_val);
vars_clear(&fc->l_vars.dv_hashtab);
if (free_val) {
TV_LIST_ITER(&fc->l_varlist, li, {
tv_clear(TV_LIST_ITEM_TV(li));
});
}
func_ptr_unref(fc->func);
xfree(fc);
}
static void cleanup_function_call(funccall_T *fc)
{
current_funccal = fc->caller;
if (!fc_referenced(fc)) {
free_funccal(fc, false);
} else {
fc->caller = previous_funccal;
previous_funccal = fc;
TV_DICT_ITER(&fc->l_avars, di, {
tv_copy(&di->di_tv, &di->di_tv);
});
TV_LIST_ITER(&fc->l_varlist, li, {
tv_copy(TV_LIST_ITEM_TV(li), TV_LIST_ITEM_TV(li));
});
}
}
static void add_nr_var(dict_T *dp, dictitem_T *v, char *name, varnumber_T nr)
{
#if !defined(__clang_analyzer__)
STRCPY(v->di_key, name);
#endif
v->di_flags = DI_FLAGS_RO | DI_FLAGS_FIX;
tv_dict_add(dp, v);
v->di_tv.v_type = VAR_NUMBER;
v->di_tv.v_lock = VAR_FIXED;
v->di_tv.vval.v_number = nr;
}
void ex_return(exarg_T *eap)
{
char_u *arg = eap->arg;
typval_T rettv;
int returning = FALSE;
if (current_funccal == NULL) {
EMSG(_("E133: :return not inside a function"));
return;
}
if (eap->skip)
++emsg_skip;
eap->nextcmd = NULL;
if ((*arg != NUL && *arg != '|' && *arg != '\n')
&& eval0(arg, &rettv, &eap->nextcmd, !eap->skip) != FAIL) {
if (!eap->skip) {
returning = do_return(eap, false, true, &rettv);
} else {
tv_clear(&rettv);
}
} else if (!eap->skip) { 
update_force_abort();
if (!aborting()) {
returning = do_return(eap, false, true, NULL);
}
}
if (returning)
eap->nextcmd = NULL;
else if (eap->nextcmd == NULL) 
eap->nextcmd = check_nextcmd(arg);
if (eap->skip)
--emsg_skip;
}
int do_return(exarg_T *eap, int reanimate, int is_cmd, void *rettv)
{
int idx;
cstack_T *const cstack = eap->cstack;
if (reanimate)
current_funccal->returned = FALSE;
idx = cleanup_conditionals(eap->cstack, 0, TRUE);
if (idx >= 0) {
cstack->cs_pending[idx] = CSTP_RETURN;
if (!is_cmd && !reanimate)
cstack->cs_rettv[idx] = rettv;
else {
if (reanimate) {
assert(current_funccal->rettv);
rettv = current_funccal->rettv;
}
if (rettv != NULL) {
cstack->cs_rettv[idx] = xcalloc(1, sizeof(typval_T));
*(typval_T *)cstack->cs_rettv[idx] = *(typval_T *)rettv;
} else
cstack->cs_rettv[idx] = NULL;
if (reanimate) {
current_funccal->rettv->v_type = VAR_NUMBER;
current_funccal->rettv->vval.v_number = 0;
}
}
report_make_pending(CSTP_RETURN, rettv);
} else {
current_funccal->returned = TRUE;
if (!reanimate && rettv != NULL) {
tv_clear(current_funccal->rettv);
*current_funccal->rettv = *(typval_T *)rettv;
if (!is_cmd)
xfree(rettv);
}
}
return idx < 0;
}
char_u *get_return_cmd(void *rettv)
{
char_u *s = NULL;
char_u *tofree = NULL;
if (rettv != NULL) {
tofree = s = (char_u *) encode_tv2echo((typval_T *) rettv, NULL);
}
if (s == NULL) {
s = (char_u *)"";
}
STRCPY(IObuff, ":return ");
STRLCPY(IObuff + 8, s, IOSIZE - 8);
if (STRLEN(s) + 8 >= IOSIZE)
STRCPY(IObuff + IOSIZE - 4, "...");
xfree(tofree);
return vim_strsave(IObuff);
}
char_u *get_func_line(int c, void *cookie, int indent, bool do_concat)
{
funccall_T *fcp = (funccall_T *)cookie;
ufunc_T *fp = fcp->func;
char_u *retval;
garray_T *gap; 
if (fcp->dbg_tick != debug_tick) {
fcp->breakpoint = dbg_find_breakpoint(FALSE, fp->uf_name,
sourcing_lnum);
fcp->dbg_tick = debug_tick;
}
if (do_profiling == PROF_YES)
func_line_end(cookie);
gap = &fp->uf_lines;
if (((fp->uf_flags & FC_ABORT) && did_emsg && !aborted_in_try())
|| fcp->returned)
retval = NULL;
else {
while (fcp->linenr < gap->ga_len
&& ((char_u **)(gap->ga_data))[fcp->linenr] == NULL)
++fcp->linenr;
if (fcp->linenr >= gap->ga_len)
retval = NULL;
else {
retval = vim_strsave(((char_u **)(gap->ga_data))[fcp->linenr++]);
sourcing_lnum = fcp->linenr;
if (do_profiling == PROF_YES)
func_line_start(cookie);
}
}
if (fcp->breakpoint != 0 && fcp->breakpoint <= sourcing_lnum) {
dbg_breakpoint(fp->uf_name, sourcing_lnum);
fcp->breakpoint = dbg_find_breakpoint(FALSE, fp->uf_name,
sourcing_lnum);
fcp->dbg_tick = debug_tick;
}
return retval;
}
void func_line_start(void *cookie)
{
funccall_T *fcp = (funccall_T *)cookie;
ufunc_T *fp = fcp->func;
if (fp->uf_profiling && sourcing_lnum >= 1
&& sourcing_lnum <= fp->uf_lines.ga_len) {
fp->uf_tml_idx = sourcing_lnum - 1;
while (fp->uf_tml_idx > 0 && FUNCLINE(fp, fp->uf_tml_idx) == NULL)
--fp->uf_tml_idx;
fp->uf_tml_execed = FALSE;
fp->uf_tml_start = profile_start();
fp->uf_tml_children = profile_zero();
fp->uf_tml_wait = profile_get_wait();
}
}
void func_line_exec(void *cookie)
{
funccall_T *fcp = (funccall_T *)cookie;
ufunc_T *fp = fcp->func;
if (fp->uf_profiling && fp->uf_tml_idx >= 0)
fp->uf_tml_execed = TRUE;
}
void func_line_end(void *cookie)
{
funccall_T *fcp = (funccall_T *)cookie;
ufunc_T *fp = fcp->func;
if (fp->uf_profiling && fp->uf_tml_idx >= 0) {
if (fp->uf_tml_execed) {
++fp->uf_tml_count[fp->uf_tml_idx];
fp->uf_tml_start = profile_end(fp->uf_tml_start);
fp->uf_tml_start = profile_sub_wait(fp->uf_tml_wait, fp->uf_tml_start);
fp->uf_tml_total[fp->uf_tml_idx] =
profile_add(fp->uf_tml_total[fp->uf_tml_idx], fp->uf_tml_start);
fp->uf_tml_self[fp->uf_tml_idx] =
profile_self(fp->uf_tml_self[fp->uf_tml_idx], fp->uf_tml_start,
fp->uf_tml_children);
}
fp->uf_tml_idx = -1;
}
}
int func_has_ended(void *cookie)
{
funccall_T *fcp = (funccall_T *)cookie;
return ((fcp->func->uf_flags & FC_ABORT) && did_emsg && !aborted_in_try())
|| fcp->returned;
}
int func_has_abort(void *cookie)
{
return ((funccall_T *)cookie)->func->uf_flags & FC_ABORT;
}
static var_flavour_T var_flavour(char_u *varname)
{
char_u *p = varname;
if (ASCII_ISUPPER(*p)) {
while (*(++p))
if (ASCII_ISLOWER(*p)) {
return VAR_FLAVOUR_SESSION;
}
return VAR_FLAVOUR_SHADA;
} else {
return VAR_FLAVOUR_DEFAULT;
}
}
hashitem_T *find_hi_in_scoped_ht(const char *name, hashtab_T **pht)
{
if (current_funccal == NULL || current_funccal->func->uf_scoped == NULL) {
return NULL;
}
funccall_T *old_current_funccal = current_funccal;
hashitem_T *hi = NULL;
const size_t namelen = strlen(name);
const char *varname;
current_funccal = current_funccal->func->uf_scoped;
while (current_funccal != NULL) {
hashtab_T *ht = find_var_ht(name, namelen, &varname);
if (ht != NULL && *varname != NUL) {
hi = hash_find_len(ht, varname, namelen - (varname - name));
if (!HASHITEM_EMPTY(hi)) {
*pht = ht;
break;
}
}
if (current_funccal == current_funccal->func->uf_scoped) {
break;
}
current_funccal = current_funccal->func->uf_scoped;
}
current_funccal = old_current_funccal;
return hi;
}
dictitem_T *find_var_in_scoped_ht(const char *name, const size_t namelen,
int no_autoload)
{
if (current_funccal == NULL || current_funccal->func->uf_scoped == NULL) {
return NULL;
}
dictitem_T *v = NULL;
funccall_T *old_current_funccal = current_funccal;
const char *varname;
current_funccal = current_funccal->func->uf_scoped;
while (current_funccal) {
hashtab_T *ht = find_var_ht(name, namelen, &varname);
if (ht != NULL && *varname != NUL) {
v = find_var_in_ht(ht, *name, varname,
namelen - (size_t)(varname - name), no_autoload);
if (v != NULL) {
break;
}
}
if (current_funccal == current_funccal->func->uf_scoped) {
break;
}
current_funccal = current_funccal->func->uf_scoped;
}
current_funccal = old_current_funccal;
return v;
}
const void *var_shada_iter(const void *const iter, const char **const name,
typval_T *rettv, var_flavour_T flavour)
FUNC_ATTR_WARN_UNUSED_RESULT FUNC_ATTR_NONNULL_ARG(2, 3)
{
const hashitem_T *hi;
const hashitem_T *hifirst = globvarht.ht_array;
const size_t hinum = (size_t) globvarht.ht_mask + 1;
*name = NULL;
if (iter == NULL) {
hi = globvarht.ht_array;
while ((size_t) (hi - hifirst) < hinum
&& (HASHITEM_EMPTY(hi)
|| !(var_flavour(hi->hi_key) & flavour))) {
hi++;
}
if ((size_t) (hi - hifirst) == hinum) {
return NULL;
}
} else {
hi = (const hashitem_T *) iter;
}
*name = (char *)TV_DICT_HI2DI(hi)->di_key;
tv_copy(&TV_DICT_HI2DI(hi)->di_tv, rettv);
while ((size_t)(++hi - hifirst) < hinum) {
if (!HASHITEM_EMPTY(hi) && (var_flavour(hi->hi_key) & flavour)) {
return hi;
}
}
return NULL;
}
void var_set_global(const char *const name, typval_T vartv)
{
funccall_T *const saved_current_funccal = current_funccal;
current_funccal = NULL;
set_var(name, strlen(name), &vartv, false);
current_funccal = saved_current_funccal;
}
int store_session_globals(FILE *fd)
{
TV_DICT_ITER(&globvardict, this_var, {
if ((this_var->di_tv.v_type == VAR_NUMBER
|| this_var->di_tv.v_type == VAR_STRING)
&& var_flavour(this_var->di_key) == VAR_FLAVOUR_SESSION) {
char_u *const p = vim_strsave_escaped(
(const char_u *)tv_get_string(&this_var->di_tv),
(const char_u *)"\\\"\n\r");
for (char_u *t = p; *t != NUL; t++) {
if (*t == '\n') {
*t = 'n';
} else if (*t == '\r') {
*t = 'r';
}
}
if ((fprintf(fd, "let %s = %c%s%c",
this_var->di_key,
((this_var->di_tv.v_type == VAR_STRING) ? '"'
: ' '),
p,
((this_var->di_tv.v_type == VAR_STRING) ? '"'
: ' ')) < 0)
|| put_eol(fd) == FAIL) {
xfree(p);
return FAIL;
}
xfree(p);
} else if (this_var->di_tv.v_type == VAR_FLOAT
&& var_flavour(this_var->di_key) == VAR_FLAVOUR_SESSION) {
float_T f = this_var->di_tv.vval.v_float;
int sign = ' ';
if (f < 0) {
f = -f;
sign = '-';
}
if ((fprintf(fd, "let %s = %c%f", this_var->di_key, sign, f) < 0)
|| put_eol(fd) == FAIL) {
return FAIL;
}
}
});
return OK;
}
void last_set_msg(sctx_T script_ctx)
{
const LastSet last_set = (LastSet){
.script_ctx = script_ctx,
.channel_id = 0,
};
option_last_set_msg(last_set);
}
void option_last_set_msg(LastSet last_set)
{
if (last_set.script_ctx.sc_sid != 0) {
bool should_free;
char_u *p = get_scriptname(last_set, &should_free);
verbose_enter();
MSG_PUTS(_("\n\tLast set from "));
MSG_PUTS(p);
if (last_set.script_ctx.sc_lnum > 0) {
MSG_PUTS(_(line_msg));
msg_outnum((long)last_set.script_ctx.sc_lnum);
}
if (should_free) {
xfree(p);
}
verbose_leave();
}
}
void reset_v_option_vars(void)
{
set_vim_var_string(VV_OPTION_NEW, NULL, -1);
set_vim_var_string(VV_OPTION_OLD, NULL, -1);
set_vim_var_string(VV_OPTION_TYPE, NULL, -1);
}
int
modify_fname(
char_u *src, 
bool tilde_file, 
size_t *usedlen, 
char_u **fnamep, 
char_u **bufp, 
size_t *fnamelen 
)
{
int valid = 0;
char_u *tail;
char_u *s, *p, *pbuf;
char_u dirname[MAXPATHL];
int c;
int has_fullname = 0;
repeat:
if (src[*usedlen] == ':' && src[*usedlen + 1] == 'p') {
has_fullname = 1;
valid |= VALID_PATH;
*usedlen += 2;
if ((*fnamep)[0] == '~'
#if !defined(UNIX)
&& ((*fnamep)[1] == '/'
#if defined(BACKSLASH_IN_FILENAME)
|| (*fnamep)[1] == '\\'
#endif
|| (*fnamep)[1] == NUL)
#endif
&& !(tilde_file && (*fnamep)[1] == NUL)
) {
*fnamep = expand_env_save(*fnamep);
xfree(*bufp); 
*bufp = *fnamep;
if (*fnamep == NULL)
return -1;
}
for (p = *fnamep; *p != NUL; MB_PTR_ADV(p)) {
if (vim_ispathsep(*p)
&& p[1] == '.'
&& (p[2] == NUL
|| vim_ispathsep(p[2])
|| (p[2] == '.'
&& (p[3] == NUL || vim_ispathsep(p[3]))))) {
break;
}
}
if (*p != NUL || !vim_isAbsName(*fnamep)) {
*fnamep = (char_u *)FullName_save((char *)*fnamep, *p != NUL);
xfree(*bufp); 
*bufp = *fnamep;
if (*fnamep == NULL)
return -1;
}
if (os_isdir(*fnamep)) {
*fnamep = vim_strnsave(*fnamep, STRLEN(*fnamep) + 2);
xfree(*bufp); 
*bufp = *fnamep;
if (*fnamep == NULL)
return -1;
add_pathsep((char *)*fnamep);
}
}
while (src[*usedlen] == ':'
&& ((c = src[*usedlen + 1]) == '.' || c == '~' || c == '8')) {
*usedlen += 2;
if (c == '8') {
continue;
}
pbuf = NULL;
if (!has_fullname) {
if (c == '.' && **fnamep == '~')
p = pbuf = expand_env_save(*fnamep);
else
p = pbuf = (char_u *)FullName_save((char *)*fnamep, FALSE);
} else
p = *fnamep;
has_fullname = 0;
if (p != NULL) {
if (c == '.') {
os_dirname(dirname, MAXPATHL);
s = path_shorten_fname(p, dirname);
if (s != NULL) {
*fnamep = s;
if (pbuf != NULL) {
xfree(*bufp); 
*bufp = pbuf;
pbuf = NULL;
}
}
} else {
home_replace(NULL, p, dirname, MAXPATHL, TRUE);
if (*dirname == '~') {
s = vim_strsave(dirname);
*fnamep = s;
xfree(*bufp);
*bufp = s;
}
}
xfree(pbuf);
}
}
tail = path_tail(*fnamep);
*fnamelen = STRLEN(*fnamep);
while (src[*usedlen] == ':' && src[*usedlen + 1] == 'h') {
valid |= VALID_HEAD;
*usedlen += 2;
s = get_past_head(*fnamep);
while (tail > s && after_pathsep((char *)s, (char *)tail)) {
MB_PTR_BACK(*fnamep, tail);
}
*fnamelen = (size_t)(tail - *fnamep);
if (*fnamelen == 0) {
xfree(*bufp);
*bufp = *fnamep = tail = vim_strsave((char_u *)".");
*fnamelen = 1;
} else {
while (tail > s && !after_pathsep((char *)s, (char *)tail)) {
MB_PTR_BACK(*fnamep, tail);
}
}
}
if (src[*usedlen] == ':' && src[*usedlen + 1] == '8') {
*usedlen += 2;
}
if (src[*usedlen] == ':' && src[*usedlen + 1] == 't') {
*usedlen += 2;
*fnamelen -= (size_t)(tail - *fnamep);
*fnamep = tail;
}
while (src[*usedlen] == ':'
&& (src[*usedlen + 1] == 'e' || src[*usedlen + 1] == 'r')) {
const bool is_second_e = *fnamep > tail;
if (src[*usedlen + 1] == 'e' && is_second_e) {
s = *fnamep - 2;
} else {
s = *fnamep + *fnamelen - 1;
}
for (; s > tail; s--) {
if (s[0] == '.') {
break;
}
}
if (src[*usedlen + 1] == 'e') {
if (s > tail || (0 && is_second_e && s == tail)) {
char_u *newstart = s + 1;
size_t distance_stepped_back = *fnamep - newstart;
*fnamelen += distance_stepped_back;
*fnamep = newstart;
} else if (*fnamep <= tail) {
*fnamelen = 0;
}
} else {
if (s > MAX(tail, *fnamep)) {
*fnamelen = (size_t)(s - *fnamep);
}
}
*usedlen += 2;
}
if (src[*usedlen] == ':'
&& (src[*usedlen + 1] == 's'
|| (src[*usedlen + 1] == 'g' && src[*usedlen + 2] == 's'))) {
char_u *str;
char_u *pat;
char_u *sub;
int sep;
char_u *flags;
int didit = FALSE;
flags = (char_u *)"";
s = src + *usedlen + 2;
if (src[*usedlen + 1] == 'g') {
flags = (char_u *)"g";
++s;
}
sep = *s++;
if (sep) {
p = vim_strchr(s, sep);
if (p != NULL) {
pat = vim_strnsave(s, (int)(p - s));
s = p + 1;
p = vim_strchr(s, sep);
if (p != NULL) {
sub = vim_strnsave(s, (int)(p - s));
str = vim_strnsave(*fnamep, *fnamelen);
*usedlen = (size_t)(p + 1 - src);
s = do_string_sub(str, pat, sub, NULL, flags);
*fnamep = s;
*fnamelen = STRLEN(s);
xfree(*bufp);
*bufp = s;
didit = TRUE;
xfree(sub);
xfree(str);
}
xfree(pat);
}
if (didit)
goto repeat;
}
}
if (src[*usedlen] == ':' && src[*usedlen + 1] == 'S') {
c = (*fnamep)[*fnamelen];
if (c != NUL) {
(*fnamep)[*fnamelen] = NUL;
}
p = vim_strsave_shellescape(*fnamep, false, false);
if (c != NUL) {
(*fnamep)[*fnamelen] = c;
}
xfree(*bufp);
*bufp = *fnamep = p;
*fnamelen = STRLEN(p);
*usedlen += 2;
}
return valid;
}
char_u *do_string_sub(char_u *str, char_u *pat, char_u *sub,
typval_T *expr, char_u *flags)
{
int sublen;
regmatch_T regmatch;
int do_all;
char_u *tail;
char_u *end;
garray_T ga;
char_u *save_cpo;
char_u *zero_width = NULL;
save_cpo = p_cpo;
p_cpo = empty_option;
ga_init(&ga, 1, 200);
do_all = (flags[0] == 'g');
regmatch.rm_ic = p_ic;
regmatch.regprog = vim_regcomp(pat, RE_MAGIC + RE_STRING);
if (regmatch.regprog != NULL) {
tail = str;
end = str + STRLEN(str);
while (vim_regexec_nl(&regmatch, str, (colnr_T)(tail - str))) {
if (regmatch.startp[0] == regmatch.endp[0]) {
if (zero_width == regmatch.startp[0]) {
int i = utfc_ptr2len(tail);
memmove((char_u *)ga.ga_data + ga.ga_len, tail, (size_t)i);
ga.ga_len += i;
tail += i;
continue;
}
zero_width = regmatch.startp[0];
}
sublen = vim_regsub(&regmatch, sub, expr, tail, false, true, false);
ga_grow(&ga, (int)((end - tail) + sublen -
(regmatch.endp[0] - regmatch.startp[0])));
int i = (int)(regmatch.startp[0] - tail);
memmove((char_u *)ga.ga_data + ga.ga_len, tail, (size_t)i);
(void)vim_regsub(&regmatch, sub, expr, (char_u *)ga.ga_data
+ ga.ga_len + i, true, true, false);
ga.ga_len += i + sublen - 1;
tail = regmatch.endp[0];
if (*tail == NUL)
break;
if (!do_all)
break;
}
if (ga.ga_data != NULL)
STRCPY((char *)ga.ga_data + ga.ga_len, tail);
vim_regfree(regmatch.regprog);
}
char_u *ret = vim_strsave(ga.ga_data == NULL ? str : (char_u *)ga.ga_data);
ga_clear(&ga);
if (p_cpo == empty_option) {
p_cpo = save_cpo;
} else {
free_string_option(save_cpo);
}
return ret;
}
bool common_job_callbacks(dict_T *vopts,
CallbackReader *on_stdout,
CallbackReader *on_stderr,
Callback *on_exit)
{
if (tv_dict_get_callback(vopts, S_LEN("on_stdout"), &on_stdout->cb)
&&tv_dict_get_callback(vopts, S_LEN("on_stderr"), &on_stderr->cb)
&& tv_dict_get_callback(vopts, S_LEN("on_exit"), on_exit)) {
on_stdout->buffered = tv_dict_get_number(vopts, "stdout_buffered");
on_stderr->buffered = tv_dict_get_number(vopts, "stderr_buffered");
if (on_stdout->buffered && on_stdout->cb.type == kCallbackNone) {
on_stdout->self = vopts;
}
if (on_stderr->buffered && on_stderr->cb.type == kCallbackNone) {
on_stderr->self = vopts;
}
vopts->dv_refcount++;
return true;
}
callback_reader_free(on_stdout);
callback_reader_free(on_stderr);
callback_free(on_exit);
return false;
}
Channel *find_job(uint64_t id, bool show_error)
{
Channel *data = find_channel(id);
if (!data || data->streamtype != kChannelStreamProc
|| process_is_stopped(&data->stream.proc)) {
if (show_error) {
if (data && data->streamtype != kChannelStreamProc) {
EMSG(_(e_invchanjob));
} else {
EMSG(_(e_invchan));
}
}
return NULL;
}
return data;
}
void script_host_eval(char *name, typval_T *argvars, typval_T *rettv)
{
if (check_restricted() || check_secure()) {
return;
}
if (argvars[0].v_type != VAR_STRING) {
EMSG(_(e_invarg));
return;
}
list_T *args = tv_list_alloc(1);
tv_list_append_string(args, (const char *)argvars[0].vval.v_string, -1);
*rettv = eval_call_provider(name, "eval", args);
}
typval_T eval_call_provider(char *provider, char *method, list_T *arguments)
{
if (!eval_has_provider(provider)) {
emsgf("E319: No \"%s\" provider found. Run \":checkhealth provider\"",
provider);
return (typval_T){
.v_type = VAR_NUMBER,
.v_lock = VAR_UNLOCKED,
.vval.v_number = (varnumber_T)0
};
}
char func[256];
int name_len = snprintf(func, sizeof(func), "provider#%s#Call", provider);
struct caller_scope saved_provider_caller_scope = provider_caller_scope;
provider_caller_scope = (struct caller_scope) {
.script_ctx = current_sctx,
.sourcing_name = sourcing_name,
.sourcing_lnum = sourcing_lnum,
.autocmd_fname = autocmd_fname,
.autocmd_match = autocmd_match,
.autocmd_bufnr = autocmd_bufnr,
.funccalp = save_funccal()
};
provider_call_nesting++;
typval_T argvars[3] = {
{.v_type = VAR_STRING, .vval.v_string = (uint8_t *)method, .v_lock = 0},
{.v_type = VAR_LIST, .vval.v_list = arguments, .v_lock = 0},
{.v_type = VAR_UNKNOWN}
};
typval_T rettv = { .v_type = VAR_UNKNOWN, .v_lock = VAR_UNLOCKED };
tv_list_ref(arguments);
int dummy;
(void)call_func((const char_u *)func,
name_len,
&rettv,
2,
argvars,
NULL,
curwin->w_cursor.lnum,
curwin->w_cursor.lnum,
&dummy,
true,
NULL,
NULL);
tv_list_unref(arguments);
restore_funccal(provider_caller_scope.funccalp);
provider_caller_scope = saved_provider_caller_scope;
provider_call_nesting--;
assert(provider_call_nesting >= 0);
return rettv;
}
bool eval_has_provider(const char *feat)
{
if (!strequal(feat, "clipboard")
&& !strequal(feat, "python")
&& !strequal(feat, "python3")
&& !strequal(feat, "python_compiled")
&& !strequal(feat, "python_dynamic")
&& !strequal(feat, "python3_compiled")
&& !strequal(feat, "python3_dynamic")
&& !strequal(feat, "perl")
&& !strequal(feat, "ruby")
&& !strequal(feat, "node")) {
return false;
}
char name[32]; 
snprintf(name, sizeof(name), "%s", feat);
strchrsub(name, '_', '\0'); 
char buf[256];
typval_T tv;
int len = snprintf(buf, sizeof(buf), "g:loaded_%s_provider", name);
if (get_var_tv(buf, len, &tv, NULL, false, true) == FAIL) {
len = snprintf(buf, sizeof(buf), "provider#%s#bogus", name);
script_autoload(buf, len, false);
len = snprintf(buf, sizeof(buf), "g:loaded_%s_provider", name);
if (get_var_tv(buf, len, &tv, NULL, false, true) == FAIL) {
snprintf(buf, sizeof(buf), "provider#%s#Call", name);
if (!!find_func((char_u *)buf) && p_lpl) {
emsgf("provider: %s: missing required variable g:loaded_%s_provider",
name, name);
}
return false;
}
}
bool ok = (tv.v_type == VAR_NUMBER)
? 2 == tv.vval.v_number 
: false;
if (ok) {
snprintf(buf, sizeof(buf), "provider#%s#Call", name);
if (!find_func((char_u *)buf)) {
emsgf("provider: %s: g:loaded_%s_provider=2 but %s is not defined",
name, name, buf);
ok = false;
}
}
return ok;
}
void eval_fmt_source_name_line(char *buf, size_t bufsize)
{
if (sourcing_name) {
snprintf(buf, bufsize, "%s:%" PRIdLINENR, sourcing_name, sourcing_lnum);
} else {
snprintf(buf, bufsize, "?");
}
}
void ex_checkhealth(exarg_T *eap)
{
bool found = !!find_func((char_u *)"health#check");
if (!found
&& script_autoload("health#check", sizeof("health#check") - 1, false)) {
found = !!find_func((char_u *)"health#check");
}
if (!found) {
const char *vimruntime_env = os_getenv("VIMRUNTIME");
if (vimruntime_env == NULL) {
EMSG(_("E5009: $VIMRUNTIME is empty or unset"));
} else {
bool rtp_ok = NULL != strstr((char *)p_rtp, vimruntime_env);
if (rtp_ok) {
EMSG2(_("E5009: Invalid $VIMRUNTIME: %s"), vimruntime_env);
} else {
EMSG(_("E5009: Invalid 'runtimepath'"));
}
}
return;
}
size_t bufsize = STRLEN(eap->arg) + sizeof("call health#check('')");
char *buf = xmalloc(bufsize);
snprintf(buf, bufsize, "call health#check('%s')", eap->arg);
do_cmdline_cmd(buf);
xfree(buf);
}
void invoke_prompt_callback(void)
{
typval_T rettv;
typval_T argv[2];
char_u *text;
char_u *prompt;
linenr_T lnum = curbuf->b_ml.ml_line_count;
ml_append(lnum, (char_u *)"", 0, false);
curwin->w_cursor.lnum = lnum + 1;
curwin->w_cursor.col = 0;
if (curbuf->b_prompt_callback.type == kCallbackNone) {
return;
}
text = ml_get(lnum);
prompt = prompt_text();
if (STRLEN(text) >= STRLEN(prompt)) {
text += STRLEN(prompt);
}
argv[0].v_type = VAR_STRING;
argv[0].vval.v_string = vim_strsave(text);
argv[1].v_type = VAR_UNKNOWN;
callback_call(&curbuf->b_prompt_callback, 1, argv, &rettv);
tv_clear(&argv[0]);
tv_clear(&rettv);
}
bool invoke_prompt_interrupt(void)
{
typval_T rettv;
typval_T argv[1];
if (curbuf->b_prompt_interrupt.type == kCallbackNone) {
return false;
}
argv[0].v_type = VAR_UNKNOWN;
got_int = false; 
callback_call(&curbuf->b_prompt_interrupt, 0, argv, &rettv);
tv_clear(&rettv);
return true;
}
