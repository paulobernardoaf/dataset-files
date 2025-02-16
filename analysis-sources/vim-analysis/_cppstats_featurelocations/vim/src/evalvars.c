












#include "vim.h"

#if defined(FEAT_EVAL) || defined(PROTO)

static dictitem_T globvars_var; 
static dict_T globvardict; 
#define globvarht globvardict.dv_hashtab





static hashtab_T compat_hashtab;









#define VV_COMPAT 1 
#define VV_RO 2 
#define VV_RO_SBX 4 

#define VV_NAME(s, t) s, {{t, 0, {0}}, 0, {0}}

typedef struct vimvar vimvar_T;

static struct vimvar
{
char *vv_name; 
dictitem16_T vv_di; 
char vv_flags; 
} vimvars[VV_LEN] =
{


{VV_NAME("count", VAR_NUMBER), VV_COMPAT+VV_RO},
{VV_NAME("count1", VAR_NUMBER), VV_RO},
{VV_NAME("prevcount", VAR_NUMBER), VV_RO},
{VV_NAME("errmsg", VAR_STRING), VV_COMPAT},
{VV_NAME("warningmsg", VAR_STRING), 0},
{VV_NAME("statusmsg", VAR_STRING), 0},
{VV_NAME("shell_error", VAR_NUMBER), VV_COMPAT+VV_RO},
{VV_NAME("this_session", VAR_STRING), VV_COMPAT},
{VV_NAME("version", VAR_NUMBER), VV_COMPAT+VV_RO},
{VV_NAME("lnum", VAR_NUMBER), VV_RO_SBX},
{VV_NAME("termresponse", VAR_STRING), VV_RO},
{VV_NAME("fname", VAR_STRING), VV_RO},
{VV_NAME("lang", VAR_STRING), VV_RO},
{VV_NAME("lc_time", VAR_STRING), VV_RO},
{VV_NAME("ctype", VAR_STRING), VV_RO},
{VV_NAME("charconvert_from", VAR_STRING), VV_RO},
{VV_NAME("charconvert_to", VAR_STRING), VV_RO},
{VV_NAME("fname_in", VAR_STRING), VV_RO},
{VV_NAME("fname_out", VAR_STRING), VV_RO},
{VV_NAME("fname_new", VAR_STRING), VV_RO},
{VV_NAME("fname_diff", VAR_STRING), VV_RO},
{VV_NAME("cmdarg", VAR_STRING), VV_RO},
{VV_NAME("foldstart", VAR_NUMBER), VV_RO_SBX},
{VV_NAME("foldend", VAR_NUMBER), VV_RO_SBX},
{VV_NAME("folddashes", VAR_STRING), VV_RO_SBX},
{VV_NAME("foldlevel", VAR_NUMBER), VV_RO_SBX},
{VV_NAME("progname", VAR_STRING), VV_RO},
{VV_NAME("servername", VAR_STRING), VV_RO},
{VV_NAME("dying", VAR_NUMBER), VV_RO},
{VV_NAME("exception", VAR_STRING), VV_RO},
{VV_NAME("throwpoint", VAR_STRING), VV_RO},
{VV_NAME("register", VAR_STRING), VV_RO},
{VV_NAME("cmdbang", VAR_NUMBER), VV_RO},
{VV_NAME("insertmode", VAR_STRING), VV_RO},
{VV_NAME("val", VAR_UNKNOWN), VV_RO},
{VV_NAME("key", VAR_UNKNOWN), VV_RO},
{VV_NAME("profiling", VAR_NUMBER), VV_RO},
{VV_NAME("fcs_reason", VAR_STRING), VV_RO},
{VV_NAME("fcs_choice", VAR_STRING), 0},
{VV_NAME("beval_bufnr", VAR_NUMBER), VV_RO},
{VV_NAME("beval_winnr", VAR_NUMBER), VV_RO},
{VV_NAME("beval_winid", VAR_NUMBER), VV_RO},
{VV_NAME("beval_lnum", VAR_NUMBER), VV_RO},
{VV_NAME("beval_col", VAR_NUMBER), VV_RO},
{VV_NAME("beval_text", VAR_STRING), VV_RO},
{VV_NAME("scrollstart", VAR_STRING), 0},
{VV_NAME("swapname", VAR_STRING), VV_RO},
{VV_NAME("swapchoice", VAR_STRING), 0},
{VV_NAME("swapcommand", VAR_STRING), VV_RO},
{VV_NAME("char", VAR_STRING), 0},
{VV_NAME("mouse_win", VAR_NUMBER), 0},
{VV_NAME("mouse_winid", VAR_NUMBER), 0},
{VV_NAME("mouse_lnum", VAR_NUMBER), 0},
{VV_NAME("mouse_col", VAR_NUMBER), 0},
{VV_NAME("operator", VAR_STRING), VV_RO},
{VV_NAME("searchforward", VAR_NUMBER), 0},
{VV_NAME("hlsearch", VAR_NUMBER), 0},
{VV_NAME("oldfiles", VAR_LIST), 0},
{VV_NAME("windowid", VAR_NUMBER), VV_RO},
{VV_NAME("progpath", VAR_STRING), VV_RO},
{VV_NAME("completed_item", VAR_DICT), VV_RO},
{VV_NAME("option_new", VAR_STRING), VV_RO},
{VV_NAME("option_old", VAR_STRING), VV_RO},
{VV_NAME("option_oldlocal", VAR_STRING), VV_RO},
{VV_NAME("option_oldglobal", VAR_STRING), VV_RO},
{VV_NAME("option_command", VAR_STRING), VV_RO},
{VV_NAME("option_type", VAR_STRING), VV_RO},
{VV_NAME("errors", VAR_LIST), 0},
{VV_NAME("false", VAR_BOOL), VV_RO},
{VV_NAME("true", VAR_BOOL), VV_RO},
{VV_NAME("none", VAR_SPECIAL), VV_RO},
{VV_NAME("null", VAR_SPECIAL), VV_RO},
{VV_NAME("numbersize", VAR_NUMBER), VV_RO},
{VV_NAME("vim_did_enter", VAR_NUMBER), VV_RO},
{VV_NAME("testing", VAR_NUMBER), 0},
{VV_NAME("t_number", VAR_NUMBER), VV_RO},
{VV_NAME("t_string", VAR_NUMBER), VV_RO},
{VV_NAME("t_func", VAR_NUMBER), VV_RO},
{VV_NAME("t_list", VAR_NUMBER), VV_RO},
{VV_NAME("t_dict", VAR_NUMBER), VV_RO},
{VV_NAME("t_float", VAR_NUMBER), VV_RO},
{VV_NAME("t_bool", VAR_NUMBER), VV_RO},
{VV_NAME("t_none", VAR_NUMBER), VV_RO},
{VV_NAME("t_job", VAR_NUMBER), VV_RO},
{VV_NAME("t_channel", VAR_NUMBER), VV_RO},
{VV_NAME("t_blob", VAR_NUMBER), VV_RO},
{VV_NAME("termrfgresp", VAR_STRING), VV_RO},
{VV_NAME("termrbgresp", VAR_STRING), VV_RO},
{VV_NAME("termu7resp", VAR_STRING), VV_RO},
{VV_NAME("termstyleresp", VAR_STRING), VV_RO},
{VV_NAME("termblinkresp", VAR_STRING), VV_RO},
{VV_NAME("event", VAR_DICT), VV_RO},
{VV_NAME("versionlong", VAR_NUMBER), VV_RO},
{VV_NAME("echospace", VAR_NUMBER), VV_RO},
{VV_NAME("argv", VAR_LIST), VV_RO},
};


#define vv_type vv_di.di_tv.v_type
#define vv_nr vv_di.di_tv.vval.v_number
#define vv_float vv_di.di_tv.vval.v_float
#define vv_str vv_di.di_tv.vval.v_string
#define vv_list vv_di.di_tv.vval.v_list
#define vv_dict vv_di.di_tv.vval.v_dict
#define vv_blob vv_di.di_tv.vval.v_blob
#define vv_tv vv_di.di_tv

static dictitem_T vimvars_var; 
static dict_T vimvardict; 
#define vimvarht vimvardict.dv_hashtab


#include "version.h"

static void ex_let_const(exarg_T *eap, int is_const);
static char_u *skip_var_one(char_u *arg, int include_type);
static void list_glob_vars(int *first);
static void list_buf_vars(int *first);
static void list_win_vars(int *first);
static void list_tab_vars(int *first);
static char_u *list_arg_vars(exarg_T *eap, char_u *arg, int *first);
static char_u *ex_let_one(char_u *arg, typval_T *tv, int copy, int flags, char_u *endchars, char_u *op);
static void ex_unletlock(exarg_T *eap, char_u *argstart, int deep);
static int do_unlet_var(lval_T *lp, char_u *name_end, int forceit);
static int do_lock_var(lval_T *lp, char_u *name_end, int deep, int lock);
static void item_lock(typval_T *tv, int deep, int lock);
static void delete_var(hashtab_T *ht, hashitem_T *hi);
static void list_one_var(dictitem_T *v, char *prefix, int *first);
static void list_one_var_a(char *prefix, char_u *name, int type, char_u *string, int *first);




void
evalvars_init(void)
{
int i;
struct vimvar *p;

init_var_dict(&globvardict, &globvars_var, VAR_DEF_SCOPE);
init_var_dict(&vimvardict, &vimvars_var, VAR_SCOPE);
vimvardict.dv_lock = VAR_FIXED;
hash_init(&compat_hashtab);

for (i = 0; i < VV_LEN; ++i)
{
p = &vimvars[i];
if (STRLEN(p->vv_name) > DICTITEM16_KEY_LEN)
{
iemsg("INTERNAL: name too long, increase size of dictitem16_T");
getout(1);
}
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
vimvars[VV_VERSIONLONG].vv_nr = VIM_VERSION_100 * 10000 + highest_patch();

set_vim_var_nr(VV_SEARCHFORWARD, 1L);
set_vim_var_nr(VV_HLSEARCH, 1L);
set_vim_var_dict(VV_COMPLETED_ITEM, dict_alloc_lock(VAR_FIXED));
set_vim_var_list(VV_ERRORS, list_alloc());
set_vim_var_dict(VV_EVENT, dict_alloc_lock(VAR_FIXED));

set_vim_var_nr(VV_FALSE, VVAL_FALSE);
set_vim_var_nr(VV_TRUE, VVAL_TRUE);
set_vim_var_nr(VV_NONE, VVAL_NONE);
set_vim_var_nr(VV_NULL, VVAL_NULL);
set_vim_var_nr(VV_NUMBERSIZE, sizeof(varnumber_T) * 8);

set_vim_var_nr(VV_TYPE_NUMBER, VAR_TYPE_NUMBER);
set_vim_var_nr(VV_TYPE_STRING, VAR_TYPE_STRING);
set_vim_var_nr(VV_TYPE_FUNC, VAR_TYPE_FUNC);
set_vim_var_nr(VV_TYPE_LIST, VAR_TYPE_LIST);
set_vim_var_nr(VV_TYPE_DICT, VAR_TYPE_DICT);
set_vim_var_nr(VV_TYPE_FLOAT, VAR_TYPE_FLOAT);
set_vim_var_nr(VV_TYPE_BOOL, VAR_TYPE_BOOL);
set_vim_var_nr(VV_TYPE_NONE, VAR_TYPE_NONE);
set_vim_var_nr(VV_TYPE_JOB, VAR_TYPE_JOB);
set_vim_var_nr(VV_TYPE_CHANNEL, VAR_TYPE_CHANNEL);
set_vim_var_nr(VV_TYPE_BLOB, VAR_TYPE_BLOB);

set_vim_var_nr(VV_ECHOSPACE, sc_col - 1);

set_reg_var(0); 
}

#if defined(EXITFREE) || defined(PROTO)



void
evalvars_clear(void)
{
int i;
struct vimvar *p;

for (i = 0; i < VV_LEN; ++i)
{
p = &vimvars[i];
if (p->vv_di.di_tv.v_type == VAR_STRING)
VIM_CLEAR(p->vv_str);
else if (p->vv_di.di_tv.v_type == VAR_LIST)
{
list_unref(p->vv_list);
p->vv_list = NULL;
}
}
hash_clear(&vimvarht);
hash_init(&vimvarht); 
hash_clear(&compat_hashtab);


vars_clear(&globvarht);





for (i = 1; i <= script_items.ga_len; ++i)
vars_clear(&SCRIPT_VARS(i));
}
#endif

int
garbage_collect_globvars(int copyID)
{
return set_ref_in_ht(&globvarht, copyID, NULL);
}

int
garbage_collect_vimvars(int copyID)
{
return set_ref_in_ht(&vimvarht, copyID, NULL);
}

int
garbage_collect_scriptvars(int copyID)
{
int i;
int abort = FALSE;

for (i = 1; i <= script_items.ga_len; ++i)
abort = abort || set_ref_in_ht(&SCRIPT_VARS(i), copyID, NULL);

return abort;
}





void
set_internal_string_var(char_u *name, char_u *value)
{
char_u *val;
typval_T *tvp;

val = vim_strsave(value);
if (val != NULL)
{
tvp = alloc_string_tv(val);
if (tvp != NULL)
{
set_var(name, tvp, FALSE);
free_tv(tvp);
}
}
}

int
eval_charconvert(
char_u *enc_from,
char_u *enc_to,
char_u *fname_from,
char_u *fname_to)
{
int err = FALSE;

set_vim_var_string(VV_CC_FROM, enc_from, -1);
set_vim_var_string(VV_CC_TO, enc_to, -1);
set_vim_var_string(VV_FNAME_IN, fname_from, -1);
set_vim_var_string(VV_FNAME_OUT, fname_to, -1);
if (eval_to_bool(p_ccv, &err, NULL, FALSE))
err = TRUE;
set_vim_var_string(VV_CC_FROM, NULL, -1);
set_vim_var_string(VV_CC_TO, NULL, -1);
set_vim_var_string(VV_FNAME_IN, NULL, -1);
set_vim_var_string(VV_FNAME_OUT, NULL, -1);

if (err)
return FAIL;
return OK;
}

#if defined(FEAT_POSTSCRIPT) || defined(PROTO)
int
eval_printexpr(char_u *fname, char_u *args)
{
int err = FALSE;

set_vim_var_string(VV_FNAME_IN, fname, -1);
set_vim_var_string(VV_CMDARG, args, -1);
if (eval_to_bool(p_pexpr, &err, NULL, FALSE))
err = TRUE;
set_vim_var_string(VV_FNAME_IN, NULL, -1);
set_vim_var_string(VV_CMDARG, NULL, -1);

if (err)
{
mch_remove(fname);
return FAIL;
}
return OK;
}
#endif

#if defined(FEAT_DIFF) || defined(PROTO)
void
eval_diff(
char_u *origfile,
char_u *newfile,
char_u *outfile)
{
int err = FALSE;

set_vim_var_string(VV_FNAME_IN, origfile, -1);
set_vim_var_string(VV_FNAME_NEW, newfile, -1);
set_vim_var_string(VV_FNAME_OUT, outfile, -1);
(void)eval_to_bool(p_dex, &err, NULL, FALSE);
set_vim_var_string(VV_FNAME_IN, NULL, -1);
set_vim_var_string(VV_FNAME_NEW, NULL, -1);
set_vim_var_string(VV_FNAME_OUT, NULL, -1);
}

void
eval_patch(
char_u *origfile,
char_u *difffile,
char_u *outfile)
{
int err;

set_vim_var_string(VV_FNAME_IN, origfile, -1);
set_vim_var_string(VV_FNAME_DIFF, difffile, -1);
set_vim_var_string(VV_FNAME_OUT, outfile, -1);
(void)eval_to_bool(p_pex, &err, NULL, FALSE);
set_vim_var_string(VV_FNAME_IN, NULL, -1);
set_vim_var_string(VV_FNAME_DIFF, NULL, -1);
set_vim_var_string(VV_FNAME_OUT, NULL, -1);
}
#endif

#if defined(FEAT_SPELL) || defined(PROTO)





list_T *
eval_spell_expr(char_u *badword, char_u *expr)
{
typval_T save_val;
typval_T rettv;
list_T *list = NULL;
char_u *p = skipwhite(expr);


prepare_vimvar(VV_VAL, &save_val);
set_vim_var_string(VV_VAL, badword, -1);
if (p_verbose == 0)
++emsg_off;

if (eval1(&p, &rettv, TRUE) == OK)
{
if (rettv.v_type != VAR_LIST)
clear_tv(&rettv);
else
list = rettv.vval.v_list;
}

if (p_verbose == 0)
--emsg_off;
clear_tv(get_vim_var_tv(VV_VAL));
restore_vimvar(VV_VAL, &save_val);

return list;
}







int
get_spellword(list_T *list, char_u **pp)
{
listitem_T *li;

li = list->lv_first;
if (li == NULL)
return -1;
*pp = tv_get_string(&li->li_tv);

li = li->li_next;
if (li == NULL)
return -1;
return (int)tv_get_number(&li->li_tv);
}
#endif






void
prepare_vimvar(int idx, typval_T *save_tv)
{
*save_tv = vimvars[idx].vv_tv;
vimvars[idx].vv_str = NULL; 
if (vimvars[idx].vv_type == VAR_UNKNOWN)
hash_add(&vimvarht, vimvars[idx].vv_di.di_key);
}






void
restore_vimvar(int idx, typval_T *save_tv)
{
hashitem_T *hi;

vimvars[idx].vv_tv = *save_tv;
if (vimvars[idx].vv_type == VAR_UNKNOWN)
{
hi = hash_find(&vimvarht, vimvars[idx].vv_di.di_key);
if (HASHITEM_EMPTY(hi))
internal_error("restore_vimvar()");
else
hash_remove(&vimvarht, hi);
}
}




static void
list_vim_vars(int *first)
{
list_hashtable_vars(&vimvarht, "v:", FALSE, first);
}




static void
list_script_vars(int *first)
{
if (current_sctx.sc_sid > 0 && current_sctx.sc_sid <= script_items.ga_len)
list_hashtable_vars(&SCRIPT_VARS(current_sctx.sc_sid),
"s:", FALSE, first);
}















list_T *
heredoc_get(exarg_T *eap, char_u *cmd)
{
char_u *theline;
char_u *marker;
list_T *l;
char_u *p;
int marker_indent_len = 0;
int text_indent_len = 0;
char_u *text_indent = NULL;

if (eap->getline == NULL)
{
emsg(_("E991: cannot use =<< here"));
return NULL;
}


cmd = skipwhite(cmd);
if (STRNCMP(cmd, "trim", 4) == 0 && (cmd[4] == NUL || VIM_ISWHITE(cmd[4])))
{
cmd = skipwhite(cmd + 4);





p = *eap->cmdlinep;
while (VIM_ISWHITE(*p))
{
p++;
marker_indent_len++;
}
text_indent_len = -1;
}


if (*cmd != NUL && *cmd != '"')
{
marker = skipwhite(cmd);
p = skiptowhite(marker);
if (*skipwhite(p) != NUL && *skipwhite(p) != '"')
{
emsg(_(e_trailing));
return NULL;
}
*p = NUL;
if (vim_islower(*marker))
{
emsg(_("E221: Marker cannot start with lower case letter"));
return NULL;
}
}
else
{
emsg(_("E172: Missing marker"));
return NULL;
}

l = list_alloc();
if (l == NULL)
return NULL;

for (;;)
{
int mi = 0;
int ti = 0;

theline = eap->getline(NUL, eap->cookie, 0, FALSE);
if (theline == NULL)
{
semsg(_("E990: Missing end marker '%s'"), marker);
break;
}



if (marker_indent_len > 0
&& STRNCMP(theline, *eap->cmdlinep, marker_indent_len) == 0)
mi = marker_indent_len;
if (STRCMP(marker, theline + mi) == 0)
{
vim_free(theline);
break;
}

if (text_indent_len == -1 && *theline != NUL)
{

p = theline;
text_indent_len = 0;
while (VIM_ISWHITE(*p))
{
p++;
text_indent_len++;
}
text_indent = vim_strnsave(theline, text_indent_len);
}

if (text_indent != NULL)
for (ti = 0; ti < text_indent_len; ++ti)
if (theline[ti] != text_indent[ti])
break;

if (list_append_string(l, theline + ti, -1) == FAIL)
break;
vim_free(theline);
}
vim_free(text_indent);

return l;
}
















void
ex_let(exarg_T *eap)
{
ex_let_const(eap, FALSE);
}







void
ex_const(exarg_T *eap)
{
ex_let_const(eap, TRUE);
}

static void
ex_let_const(exarg_T *eap, int is_const)
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
int concat;
int flags = is_const ? LET_IS_CONST : 0;


if (eap->arg == eap->cmd)
flags |= LET_NO_COMMAND;

argend = skip_var_list(arg, TRUE, &var_count, &semicolon);
if (argend == NULL)
return;
if (argend > arg && argend[-1] == '.') 
--argend;
expr = skipwhite(argend);
concat = expr[0] == '.'
&& ((expr[1] == '=' && current_sctx.sc_version < 2)
|| (expr[1] == '.' && expr[2] == '='));
if (*expr != '=' && !((vim_strchr((char_u *)"+-*/%", *expr) != NULL
&& expr[1] == '=') || concat))
{

if (*arg == '[')
emsg(_(e_invarg));
else if (expr[0] == '.')
emsg(_("E985: .= is not supported with script version 2"));
else if (!ends_excmd(*arg))

arg = list_arg_vars(eap, arg, &first);
else if (!eap->skip)
{

list_glob_vars(&first);
list_buf_vars(&first);
list_win_vars(&first);
list_tab_vars(&first);
list_script_vars(&first);
list_func_vars(&first);
list_vim_vars(&first);
}
eap->nextcmd = check_nextcmd(arg);
}
else if (expr[0] == '=' && expr[1] == '<' && expr[2] == '<')
{
list_T *l;


l = heredoc_get(eap, expr + 3);
if (l != NULL)
{
rettv_list_set(&rettv, l);
if (!eap->skip)
{
op[0] = '=';
op[1] = NUL;
(void)ex_let_vars(eap->arg, &rettv, FALSE, semicolon, var_count,
flags, op);
}
clear_tv(&rettv);
}
}
else
{
op[0] = '=';
op[1] = NUL;
if (*expr != '=')
{
if (vim_strchr((char_u *)"+-*/%.", *expr) != NULL)
{
op[0] = *expr; 
if (expr[0] == '.' && expr[1] == '.') 
++expr;
}
expr = skipwhite(expr + 2);
}
else
expr = skipwhite(expr + 1);

if (eap->skip)
++emsg_skip;
i = eval0(expr, &rettv, &eap->nextcmd, !eap->skip);
if (eap->skip)
{
if (i != FAIL)
clear_tv(&rettv);
--emsg_skip;
}
else if (i != FAIL)
{
(void)ex_let_vars(eap->arg, &rettv, FALSE, semicolon, var_count,
flags, op);
clear_tv(&rettv);
}
}
}









int
ex_let_vars(
char_u *arg_start,
typval_T *tv,
int copy, 
int semicolon, 
int var_count, 
int flags, 
char_u *op)
{
char_u *arg = arg_start;
list_T *l;
int i;
listitem_T *item;
typval_T ltv;

if (*arg != '[')
{

if (ex_let_one(arg, tv, copy, flags, op, op) == NULL)
return FAIL;
return OK;
}


if (tv->v_type != VAR_LIST || (l = tv->vval.v_list) == NULL)
{
emsg(_(e_listreq));
return FAIL;
}

i = list_len(l);
if (semicolon == 0 && var_count < i)
{
emsg(_("E687: Less targets than List items"));
return FAIL;
}
if (var_count - semicolon > i)
{
emsg(_("E688: More targets than List items"));
return FAIL;
}

range_list_materialize(l);
item = l->lv_first;
while (*arg != ']')
{
arg = skipwhite(arg + 1);
arg = ex_let_one(arg, &item->li_tv, TRUE, flags, (char_u *)",;]", op);
item = item->li_next;
if (arg == NULL)
return FAIL;

arg = skipwhite(arg);
if (*arg == ';')
{


l = list_alloc();
if (l == NULL)
return FAIL;
while (item != NULL)
{
list_append_tv(l, &item->li_tv);
item = item->li_next;
}

ltv.v_type = VAR_LIST;
ltv.v_lock = 0;
ltv.vval.v_list = l;
l->lv_refcount = 1;

arg = ex_let_one(skipwhite(arg + 1), &ltv, FALSE, flags,
(char_u *)"]", op);
clear_tv(&ltv);
if (arg == NULL)
return FAIL;
break;
}
else if (*arg != ',' && *arg != ']')
{
internal_error("ex_let_vars()");
return FAIL;
}
}

return OK;
}








char_u *
skip_var_list(
char_u *arg,
int include_type,
int *var_count,
int *semicolon)
{
char_u *p, *s;

if (*arg == '[')
{

p = arg;
for (;;)
{
p = skipwhite(p + 1); 
s = skip_var_one(p, TRUE);
if (s == p)
{
semsg(_(e_invarg2), p);
return NULL;
}
++*var_count;

p = skipwhite(s);
if (*p == ']')
break;
else if (*p == ';')
{
if (*semicolon == 1)
{
emsg(_("E452: Double ; in list of variables"));
return NULL;
}
*semicolon = 1;
}
else if (*p != ',')
{
semsg(_(e_invarg2), p);
return NULL;
}
}
return p + 1;
}
else
return skip_var_one(arg, include_type);
}






static char_u *
skip_var_one(char_u *arg, int include_type)
{
char_u *end;

if (*arg == '@' && arg[1] != NUL)
return arg + 2;
end = find_name_end(*arg == '$' || *arg == '&' ? arg + 1 : arg,
NULL, NULL, FNE_INCL_BR | FNE_CHECK_START);
if (include_type && current_sctx.sc_version == SCRIPT_VERSION_VIM9
&& *end == ':')
{
end = skip_type(skipwhite(end + 1));
}
return end;
}





void
list_hashtable_vars(
hashtab_T *ht,
char *prefix,
int empty,
int *first)
{
hashitem_T *hi;
dictitem_T *di;
int todo;
char_u buf[IOSIZE];

todo = (int)ht->ht_used;
for (hi = ht->ht_array; todo > 0 && !got_int; ++hi)
{
if (!HASHITEM_EMPTY(hi))
{
--todo;
di = HI2DI(hi);


vim_strncpy((char_u *)buf, (char_u *)prefix, IOSIZE - 1);
vim_strcat((char_u *)buf, di->di_key, IOSIZE);
if (message_filtered(buf))
continue;

if (empty || di->di_tv.v_type != VAR_STRING
|| di->di_tv.vval.v_string != NULL)
list_one_var(di, prefix, first);
}
}
}




static void
list_glob_vars(int *first)
{
list_hashtable_vars(&globvarht, "", TRUE, first);
}




static void
list_buf_vars(int *first)
{
list_hashtable_vars(&curbuf->b_vars->dv_hashtab, "b:", TRUE, first);
}




static void
list_win_vars(int *first)
{
list_hashtable_vars(&curwin->w_vars->dv_hashtab, "w:", TRUE, first);
}




static void
list_tab_vars(int *first)
{
list_hashtable_vars(&curtab->tp_vars->dv_hashtab, "t:", TRUE, first);
}




static char_u *
list_arg_vars(exarg_T *eap, char_u *arg, int *first)
{
int error = FALSE;
int len;
char_u *name;
char_u *name_start;
char_u *arg_subsc;
char_u *tofree;
typval_T tv;

while (!ends_excmd(*arg) && !got_int)
{
if (error || eap->skip)
{
arg = find_name_end(arg, NULL, NULL, FNE_INCL_BR | FNE_CHECK_START);
if (!VIM_ISWHITE(*arg) && !ends_excmd(*arg))
{
emsg_severe = TRUE;
emsg(_(e_trailing));
break;
}
}
else
{

name_start = name = arg;
len = get_name_len(&arg, &tofree, TRUE, TRUE);
if (len <= 0)
{


if (len < 0 && !aborting())
{
emsg_severe = TRUE;
semsg(_(e_invarg2), arg);
break;
}
error = TRUE;
}
else
{
if (tofree != NULL)
name = tofree;
if (get_var_tv(name, len, &tv, NULL, TRUE, FALSE) == FAIL)
error = TRUE;
else
{

arg_subsc = arg;
if (handle_subscript(&arg, &tv, TRUE, TRUE,
name, &name) == FAIL)
error = TRUE;
else
{
if (arg == arg_subsc && len == 2 && name[1] == ':')
{
switch (*name)
{
case 'g': list_glob_vars(first); break;
case 'b': list_buf_vars(first); break;
case 'w': list_win_vars(first); break;
case 't': list_tab_vars(first); break;
case 'v': list_vim_vars(first); break;
case 's': list_script_vars(first); break;
case 'l': list_func_vars(first); break;
default:
semsg(_("E738: Can't list variables for %s"), name);
}
}
else
{
char_u numbuf[NUMBUFLEN];
char_u *tf;
int c;
char_u *s;

s = echo_string(&tv, &tf, numbuf, 0);
c = *arg;
*arg = NUL;
list_one_var_a("",
arg == arg_subsc ? name : name_start,
tv.v_type,
s == NULL ? (char_u *)"" : s,
first);
*arg = c;
vim_free(tf);
}
clear_tv(&tv);
}
}
}

vim_free(tofree);
}

arg = skipwhite(arg);
}

return arg;
}






static char_u *
ex_let_one(
char_u *arg, 
typval_T *tv, 
int copy, 
int flags, 
char_u *endchars, 
char_u *op) 
{
int c1;
char_u *name;
char_u *p;
char_u *arg_end = NULL;
int len;
int opt_flags;
char_u *tofree = NULL;


if (*arg == '$')
{
if (flags & LET_IS_CONST)
{
emsg(_("E996: Cannot lock an environment variable"));
return NULL;
}

++arg;
name = arg;
len = get_env_len(&arg);
if (len == 0)
semsg(_(e_invarg2), name - 1);
else
{
if (op != NULL && vim_strchr((char_u *)"+-*/%", *op) != NULL)
semsg(_(e_letwrong), op);
else if (endchars != NULL
&& vim_strchr(endchars, *skipwhite(arg)) == NULL)
emsg(_(e_letunexp));
else if (!check_secure())
{
c1 = name[len];
name[len] = NUL;
p = tv_get_string_chk(tv);
if (p != NULL && op != NULL && *op == '.')
{
int mustfree = FALSE;
char_u *s = vim_getenv(name, &mustfree);

if (s != NULL)
{
p = tofree = concat_str(s, p);
if (mustfree)
vim_free(s);
}
}
if (p != NULL)
{
vim_setenv_ext(name, p);
arg_end = arg;
}
name[len] = c1;
vim_free(tofree);
}
}
}




else if (*arg == '&')
{
if (flags & LET_IS_CONST)
{
emsg(_(e_const_option));
return NULL;
}

p = find_option_end(&arg, &opt_flags);
if (p == NULL || (endchars != NULL
&& vim_strchr(endchars, *skipwhite(p)) == NULL))
emsg(_(e_letunexp));
else
{
long n;
int opt_type;
long numval;
char_u *stringval = NULL;
char_u *s;

c1 = *p;
*p = NUL;

n = (long)tv_get_number(tv);
s = tv_get_string_chk(tv); 
if (s != NULL && op != NULL && *op != '=')
{
opt_type = get_option_value(arg, &numval,
&stringval, opt_flags);
if ((opt_type == 1 && *op == '.')
|| (opt_type == 0 && *op != '.'))
{
semsg(_(e_letwrong), op);
s = NULL; 
}
else
{
if (opt_type == 1) 
{
switch (*op)
{
case '+': n = numval + n; break;
case '-': n = numval - n; break;
case '*': n = numval * n; break;
case '/': n = (long)num_divide(numval, n); break;
case '%': n = (long)num_modulus(numval, n); break;
}
}
else if (opt_type == 0 && stringval != NULL) 
{
s = concat_str(stringval, s);
vim_free(stringval);
stringval = s;
}
}
}
if (s != NULL)
{
set_option_value(arg, n, s, opt_flags);
arg_end = p;
}
*p = c1;
vim_free(stringval);
}
}


else if (*arg == '@')
{
if (flags & LET_IS_CONST)
{
emsg(_("E996: Cannot lock a register"));
return NULL;
}
++arg;
if (op != NULL && vim_strchr((char_u *)"+-*/%", *op) != NULL)
semsg(_(e_letwrong), op);
else if (endchars != NULL
&& vim_strchr(endchars, *skipwhite(arg + 1)) == NULL)
emsg(_(e_letunexp));
else
{
char_u *ptofree = NULL;
char_u *s;

p = tv_get_string_chk(tv);
if (p != NULL && op != NULL && *op == '.')
{
s = get_reg_contents(*arg == '@' ? '"' : *arg, GREG_EXPR_SRC);
if (s != NULL)
{
p = ptofree = concat_str(s, p);
vim_free(s);
}
}
if (p != NULL)
{
write_reg_contents(*arg == '@' ? '"' : *arg, p, -1, FALSE);
arg_end = arg + 1;
}
vim_free(ptofree);
}
}




else if (eval_isnamec1(*arg) || *arg == '{')
{
lval_T lv;

p = get_lval(arg, tv, &lv, FALSE, FALSE, 0, FNE_CHECK_START);
if (p != NULL && lv.ll_name != NULL)
{
if (endchars != NULL && vim_strchr(endchars,
*skipwhite(lv.ll_name_end)) == NULL)
emsg(_(e_letunexp));
else
{
set_var_lval(&lv, p, tv, copy, flags, op);
arg_end = p;
}
}
clear_lval(&lv);
}

else
semsg(_(e_invarg2), arg);

return arg_end;
}




void
ex_unlet(exarg_T *eap)
{
ex_unletlock(eap, eap->arg, 0);
}




void
ex_lockvar(exarg_T *eap)
{
char_u *arg = eap->arg;
int deep = 2;

if (eap->forceit)
deep = -1;
else if (vim_isdigit(*arg))
{
deep = getdigits(&arg);
arg = skipwhite(arg);
}

ex_unletlock(eap, arg, deep);
}




static void
ex_unletlock(
exarg_T *eap,
char_u *argstart,
int deep)
{
char_u *arg = argstart;
char_u *name_end;
int error = FALSE;
lval_T lv;

do
{
if (*arg == '$')
{
char_u *name = ++arg;

if (get_env_len(&arg) == 0)
{
semsg(_(e_invarg2), name - 1);
return;
}
vim_unsetenv(name);
arg = skipwhite(arg);
continue;
}


name_end = get_lval(arg, NULL, &lv, TRUE, eap->skip || error, 0,
FNE_CHECK_START);
if (lv.ll_name == NULL)
error = TRUE; 
if (name_end == NULL || (!VIM_ISWHITE(*name_end)
&& !ends_excmd(*name_end)))
{
if (name_end != NULL)
{
emsg_severe = TRUE;
emsg(_(e_trailing));
}
if (!(eap->skip || error))
clear_lval(&lv);
break;
}

if (!error && !eap->skip)
{
if (eap->cmdidx == CMD_unlet)
{
if (do_unlet_var(&lv, name_end, eap->forceit) == FAIL)
error = TRUE;
}
else
{
if (do_lock_var(&lv, name_end, deep,
eap->cmdidx == CMD_lockvar) == FAIL)
error = TRUE;
}
}

if (!eap->skip)
clear_lval(&lv);

arg = skipwhite(name_end);
} while (!ends_excmd(*arg));

eap->nextcmd = check_nextcmd(arg);
}

static int
do_unlet_var(
lval_T *lp,
char_u *name_end,
int forceit)
{
int ret = OK;
int cc;

if (lp->ll_tv == NULL)
{
cc = *name_end;
*name_end = NUL;


if (do_unlet(lp->ll_name, forceit) == FAIL)
ret = FAIL;
*name_end = cc;
}
else if ((lp->ll_list != NULL
&& var_check_lock(lp->ll_list->lv_lock, lp->ll_name, FALSE))
|| (lp->ll_dict != NULL
&& var_check_lock(lp->ll_dict->dv_lock, lp->ll_name, FALSE)))
return FAIL;
else if (lp->ll_range)
{
listitem_T *li;
listitem_T *ll_li = lp->ll_li;
int ll_n1 = lp->ll_n1;

while (ll_li != NULL && (lp->ll_empty2 || lp->ll_n2 >= ll_n1))
{
li = ll_li->li_next;
if (var_check_lock(ll_li->li_tv.v_lock, lp->ll_name, FALSE))
return FAIL;
ll_li = li;
++ll_n1;
}


while (lp->ll_li != NULL && (lp->ll_empty2 || lp->ll_n2 >= lp->ll_n1))
{
li = lp->ll_li->li_next;
listitem_remove(lp->ll_list, lp->ll_li);
lp->ll_li = li;
++lp->ll_n1;
}
}
else
{
if (lp->ll_list != NULL)

listitem_remove(lp->ll_list, lp->ll_li);
else

dictitem_remove(lp->ll_dict, lp->ll_di);
}

return ret;
}





int
do_unlet(char_u *name, int forceit)
{
hashtab_T *ht;
hashitem_T *hi;
char_u *varname;
dict_T *d;
dictitem_T *di;

ht = find_var_ht(name, &varname);
if (ht != NULL && *varname != NUL)
{
d = get_current_funccal_dict(ht);
if (d == NULL)
{
if (ht == &globvarht)
d = &globvardict;
else if (ht == &compat_hashtab)
d = &vimvardict;
else
{
di = find_var_in_ht(ht, *name, (char_u *)"", FALSE);
d = di == NULL ? NULL : di->di_tv.vval.v_dict;
}
if (d == NULL)
{
internal_error("do_unlet()");
return FAIL;
}
}
hi = hash_find(ht, varname);
if (HASHITEM_EMPTY(hi))
hi = find_hi_in_scoped_ht(name, &ht);
if (hi != NULL && !HASHITEM_EMPTY(hi))
{
di = HI2DI(hi);
if (var_check_fixed(di->di_flags, name, FALSE)
|| var_check_ro(di->di_flags, name, FALSE)
|| var_check_lock(d->dv_lock, name, FALSE))
return FAIL;

delete_var(ht, hi);
return OK;
}
}
if (forceit)
return OK;
semsg(_("E108: No such variable: \"%s\""), name);
return FAIL;
}






static int
do_lock_var(
lval_T *lp,
char_u *name_end,
int deep,
int lock)
{
int ret = OK;
int cc;
dictitem_T *di;

if (deep == 0) 
return OK;

if (lp->ll_tv == NULL)
{
cc = *name_end;
*name_end = NUL;


di = find_var(lp->ll_name, NULL, TRUE);
if (di == NULL)
ret = FAIL;
else if ((di->di_flags & DI_FLAGS_FIX)
&& di->di_tv.v_type != VAR_DICT
&& di->di_tv.v_type != VAR_LIST)


semsg(_("E940: Cannot lock or unlock variable %s"), lp->ll_name);
else
{
if (lock)
di->di_flags |= DI_FLAGS_LOCK;
else
di->di_flags &= ~DI_FLAGS_LOCK;
item_lock(&di->di_tv, deep, lock);
}
*name_end = cc;
}
else if (lp->ll_range)
{
listitem_T *li = lp->ll_li;


while (li != NULL && (lp->ll_empty2 || lp->ll_n2 >= lp->ll_n1))
{
item_lock(&li->li_tv, deep, lock);
li = li->li_next;
++lp->ll_n1;
}
}
else if (lp->ll_list != NULL)

item_lock(&lp->ll_li->li_tv, deep, lock);
else

item_lock(&lp->ll_di->di_tv, deep, lock);

return ret;
}




static void
item_lock(typval_T *tv, int deep, int lock)
{
static int recurse = 0;
list_T *l;
listitem_T *li;
dict_T *d;
blob_T *b;
hashitem_T *hi;
int todo;

if (recurse >= DICT_MAXNEST)
{
emsg(_("E743: variable nested too deep for (un)lock"));
return;
}
if (deep == 0)
return;
++recurse;


if (lock)
tv->v_lock |= VAR_LOCKED;
else
tv->v_lock &= ~VAR_LOCKED;

switch (tv->v_type)
{
case VAR_UNKNOWN:
case VAR_VOID:
case VAR_NUMBER:
case VAR_BOOL:
case VAR_STRING:
case VAR_FUNC:
case VAR_PARTIAL:
case VAR_FLOAT:
case VAR_SPECIAL:
case VAR_JOB:
case VAR_CHANNEL:
break;

case VAR_BLOB:
if ((b = tv->vval.v_blob) != NULL)
{
if (lock)
b->bv_lock |= VAR_LOCKED;
else
b->bv_lock &= ~VAR_LOCKED;
}
break;
case VAR_LIST:
if ((l = tv->vval.v_list) != NULL)
{
if (lock)
l->lv_lock |= VAR_LOCKED;
else
l->lv_lock &= ~VAR_LOCKED;
if ((deep < 0 || deep > 1) && l->lv_first != &range_list_item)

FOR_ALL_LIST_ITEMS(l, li)
item_lock(&li->li_tv, deep - 1, lock);
}
break;
case VAR_DICT:
if ((d = tv->vval.v_dict) != NULL)
{
if (lock)
d->dv_lock |= VAR_LOCKED;
else
d->dv_lock &= ~VAR_LOCKED;
if (deep < 0 || deep > 1)
{

todo = (int)d->dv_hashtab.ht_used;
for (hi = d->dv_hashtab.ht_array; todo > 0; ++hi)
{
if (!HASHITEM_EMPTY(hi))
{
--todo;
item_lock(&HI2DI(hi)->di_tv, deep - 1, lock);
}
}
}
}
}
--recurse;
}

#if (defined(FEAT_MENU) && defined(FEAT_MULTI_LANG)) || defined(PROTO)



void
del_menutrans_vars(void)
{
hashitem_T *hi;
int todo;

hash_lock(&globvarht);
todo = (int)globvarht.ht_used;
for (hi = globvarht.ht_array; todo > 0 && !got_int; ++hi)
{
if (!HASHITEM_EMPTY(hi))
{
--todo;
if (STRNCMP(HI2DI(hi)->di_key, "menutrans_", 10) == 0)
delete_var(&globvarht, hi);
}
}
hash_unlock(&globvarht);
}
#endif







static char_u *varnamebuf = NULL;
static int varnamebuflen = 0;




static char_u *
cat_prefix_varname(int prefix, char_u *name)
{
int len;

len = (int)STRLEN(name) + 3;
if (len > varnamebuflen)
{
vim_free(varnamebuf);
len += 10; 
varnamebuf = alloc(len);
if (varnamebuf == NULL)
{
varnamebuflen = 0;
return NULL;
}
varnamebuflen = len;
}
*varnamebuf = prefix;
varnamebuf[1] = ':';
STRCPY(varnamebuf + 2, name);
return varnamebuf;
}





char_u *
get_user_var_name(expand_T *xp, int idx)
{
static long_u gdone;
static long_u bdone;
static long_u wdone;
static long_u tdone;
static int vidx;
static hashitem_T *hi;
hashtab_T *ht;

if (idx == 0)
{
gdone = bdone = wdone = vidx = 0;
tdone = 0;
}


if (gdone < globvarht.ht_used)
{
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
if (bdone < ht->ht_used)
{
if (bdone++ == 0)
hi = ht->ht_array;
else
++hi;
while (HASHITEM_EMPTY(hi))
++hi;
return cat_prefix_varname('b', hi->hi_key);
}


ht = &curwin->w_vars->dv_hashtab;
if (wdone < ht->ht_used)
{
if (wdone++ == 0)
hi = ht->ht_array;
else
++hi;
while (HASHITEM_EMPTY(hi))
++hi;
return cat_prefix_varname('w', hi->hi_key);
}


ht = &curtab->tp_vars->dv_hashtab;
if (tdone < ht->ht_used)
{
if (tdone++ == 0)
hi = ht->ht_array;
else
++hi;
while (HASHITEM_EMPTY(hi))
++hi;
return cat_prefix_varname('t', hi->hi_key);
}


if (vidx < VV_LEN)
return cat_prefix_varname('v', (char_u *)vimvars[vidx++].vv_name);

VIM_CLEAR(varnamebuf);
varnamebuflen = 0;
return NULL;
}

char *
get_var_special_name(int nr)
{
switch (nr)
{
case VVAL_FALSE: return "v:false";
case VVAL_TRUE: return "v:true";
case VVAL_NONE: return "v:none";
case VVAL_NULL: return "v:null";
}
internal_error("get_var_special_name()");
return "42";
}




dict_T *
get_globvar_dict(void)
{
return &globvardict;
}




hashtab_T *
get_globvar_ht(void)
{
return &globvarht;
}




dict_T *
get_vimvar_dict(void)
{
return &vimvardict;
}




int
find_vim_var(char_u *name)
{
dictitem_T *di = find_var_in_ht(&vimvarht, 0, name, TRUE);
struct vimvar *vv;

if (di == NULL)
return -1;
vv = (struct vimvar *)((char *)di - offsetof(vimvar_T, vv_di));
return (int)(vv - vimvars);
}





void
set_vim_var_type(int idx, vartype_T type)
{
vimvars[idx].vv_type = type;
}





void
set_vim_var_nr(int idx, varnumber_T val)
{
vimvars[idx].vv_nr = val;
}

char *
get_vim_var_name(int idx)
{
return vimvars[idx].vv_name;
}




typval_T *
get_vim_var_tv(int idx)
{
return &vimvars[idx].vv_tv;
}





int
set_vim_var_tv(int idx, typval_T *tv)
{
if (vimvars[idx].vv_type != tv->v_type)
{
emsg(_("E1063: type mismatch for v: variable"));
clear_tv(tv);
return FAIL;
}
clear_tv(&vimvars[idx].vv_di.di_tv);
vimvars[idx].vv_di.di_tv = *tv;
return OK;
}




varnumber_T
get_vim_var_nr(int idx)
{
return vimvars[idx].vv_nr;
}






char_u *
get_vim_var_str(int idx)
{
return tv_get_string(&vimvars[idx].vv_tv);
}





list_T *
get_vim_var_list(int idx)
{
return vimvars[idx].vv_list;
}





dict_T *
get_vim_var_dict(int idx)
{
return vimvars[idx].vv_dict;
}




void
set_vim_var_char(int c)
{
char_u buf[MB_MAXBYTES + 1];

if (has_mbyte)
buf[(*mb_char2bytes)(c, buf)] = NUL;
else
{
buf[0] = c;
buf[1] = NUL;
}
set_vim_var_string(VV_CHAR, buf, -1);
}





void
set_vcount(
long count,
long count1,
int set_prevcount)
{
if (set_prevcount)
vimvars[VV_PREVCOUNT].vv_nr = vimvars[VV_COUNT].vv_nr;
vimvars[VV_COUNT].vv_nr = count;
vimvars[VV_COUNT1].vv_nr = count1;
}





void
save_vimvars(vimvars_save_T *vvsave)
{
vvsave->vv_prevcount = vimvars[VV_PREVCOUNT].vv_nr;
vvsave->vv_count = vimvars[VV_COUNT].vv_nr;
vvsave->vv_count1 = vimvars[VV_COUNT1].vv_nr;
}




void
restore_vimvars(vimvars_save_T *vvsave)
{
vimvars[VV_PREVCOUNT].vv_nr = vvsave->vv_prevcount;
vimvars[VV_COUNT].vv_nr = vvsave->vv_count;
vimvars[VV_COUNT1].vv_nr = vvsave->vv_count1;
}





void
set_vim_var_string(
int idx,
char_u *val,
int len) 
{
clear_tv(&vimvars[idx].vv_di.di_tv);
vimvars[idx].vv_type = VAR_STRING;
if (val == NULL)
vimvars[idx].vv_str = NULL;
else if (len == -1)
vimvars[idx].vv_str = vim_strsave(val);
else
vimvars[idx].vv_str = vim_strnsave(val, len);
}




void
set_vim_var_list(int idx, list_T *val)
{
clear_tv(&vimvars[idx].vv_di.di_tv);
vimvars[idx].vv_type = VAR_LIST;
vimvars[idx].vv_list = val;
if (val != NULL)
++val->lv_refcount;
}




void
set_vim_var_dict(int idx, dict_T *val)
{
clear_tv(&vimvars[idx].vv_di.di_tv);
vimvars[idx].vv_type = VAR_DICT;
vimvars[idx].vv_dict = val;
if (val != NULL)
{
++val->dv_refcount;
dict_set_items_ro(val);
}
}




void
set_argv_var(char **argv, int argc)
{
list_T *l = list_alloc();
int i;

if (l == NULL)
getout(1);
l->lv_lock = VAR_FIXED;
for (i = 0; i < argc; ++i)
{
if (list_append_string(l, (char_u *)argv[i], -1) == FAIL)
getout(1);
l->lv_u.mat.lv_last->li_tv.v_lock = VAR_FIXED;
}
set_vim_var_list(VV_ARGV, l);
}




void
set_reg_var(int c)
{
char_u regname;

if (c == 0 || c == ' ')
regname = '"';
else
regname = c;

if (vimvars[VV_REG].vv_str == NULL || vimvars[VV_REG].vv_str[0] != c)
set_vim_var_string(VV_REG, &regname, 1);
}







char_u *
v_exception(char_u *oldval)
{
if (oldval == NULL)
return vimvars[VV_EXCEPTION].vv_str;

vimvars[VV_EXCEPTION].vv_str = oldval;
return NULL;
}







char_u *
v_throwpoint(char_u *oldval)
{
if (oldval == NULL)
return vimvars[VV_THROWPOINT].vv_str;

vimvars[VV_THROWPOINT].vv_str = oldval;
return NULL;
}







char_u *
set_cmdarg(exarg_T *eap, char_u *oldarg)
{
char_u *oldval;
char_u *newval;
unsigned len;

oldval = vimvars[VV_CMDARG].vv_str;
if (eap == NULL)
{
vim_free(oldval);
vimvars[VV_CMDARG].vv_str = oldarg;
return NULL;
}

if (eap->force_bin == FORCE_BIN)
len = 6;
else if (eap->force_bin == FORCE_NOBIN)
len = 8;
else
len = 0;

if (eap->read_edit)
len += 7;

if (eap->force_ff != 0)
len += 10; 
if (eap->force_enc != 0)
len += (unsigned)STRLEN(eap->cmd + eap->force_enc) + 7;
if (eap->bad_char != 0)
len += 7 + 4; 

newval = alloc(len + 1);
if (newval == NULL)
return NULL;

if (eap->force_bin == FORCE_BIN)
sprintf((char *)newval, " ++bin");
else if (eap->force_bin == FORCE_NOBIN)
sprintf((char *)newval, " ++nobin");
else
*newval = NUL;

if (eap->read_edit)
STRCAT(newval, " ++edit");

if (eap->force_ff != 0)
sprintf((char *)newval + STRLEN(newval), " ++ff=%s",
eap->force_ff == 'u' ? "unix"
: eap->force_ff == 'd' ? "dos"
: "mac");
if (eap->force_enc != 0)
sprintf((char *)newval + STRLEN(newval), " ++enc=%s",
eap->cmd + eap->force_enc);
if (eap->bad_char == BAD_KEEP)
STRCPY(newval + STRLEN(newval), " ++bad=keep");
else if (eap->bad_char == BAD_DROP)
STRCPY(newval + STRLEN(newval), " ++bad=drop");
else if (eap->bad_char != 0)
sprintf((char *)newval + STRLEN(newval), " ++bad=%c", eap->bad_char);
vimvars[VV_CMDARG].vv_str = newval;
return oldval;
}





int
get_var_tv(
char_u *name,
int len, 
typval_T *rettv, 
dictitem_T **dip, 
int verbose, 
int no_autoload) 
{
int ret = OK;
typval_T *tv = NULL;
dictitem_T *v;
int cc;


cc = name[len];
name[len] = NUL;


v = find_var(name, NULL, no_autoload);
if (v != NULL)
{
tv = &v->di_tv;
if (dip != NULL)
*dip = v;
}

if (tv == NULL && current_sctx.sc_version == SCRIPT_VERSION_VIM9)
{
imported_T *import = find_imported(name, 0, NULL);


if (import != NULL)
{
scriptitem_T *si = SCRIPT_ITEM(import->imp_sid);
svar_T *sv = ((svar_T *)si->sn_var_vals.ga_data)
+ import->imp_var_vals_idx;
tv = sv->sv_tv;
}
}

if (tv == NULL)
{
if (rettv != NULL && verbose)
semsg(_(e_undefvar), name);
ret = FAIL;
}
else if (rettv != NULL)
copy_tv(tv, rettv);

name[len] = cc;

return ret;
}





void
check_vars(char_u *name, int len)
{
int cc;
char_u *varname;
hashtab_T *ht;

if (eval_lavars_used == NULL)
return;


cc = name[len];
name[len] = NUL;

ht = find_var_ht(name, &varname);
if (ht == get_funccal_local_ht() || ht == get_funccal_args_ht())
{
if (find_var(name, NULL, TRUE) != NULL)
*eval_lavars_used = TRUE;
}

name[len] = cc;
}








dictitem_T *
find_var(char_u *name, hashtab_T **htp, int no_autoload)
{
char_u *varname;
hashtab_T *ht;
dictitem_T *ret = NULL;

ht = find_var_ht(name, &varname);
if (htp != NULL)
*htp = ht;
if (ht == NULL)
return NULL;
ret = find_var_in_ht(ht, *name, varname, no_autoload || htp != NULL);
if (ret != NULL)
return ret;


return find_var_in_scoped_ht(name, no_autoload || htp != NULL);
}





dictitem_T *
find_var_in_ht(
hashtab_T *ht,
int htname,
char_u *varname,
int no_autoload)
{
hashitem_T *hi;

if (*varname == NUL)
{

switch (htname)
{
case 's': return &SCRIPT_SV(current_sctx.sc_sid)->sv_var;
case 'g': return &globvars_var;
case 'v': return &vimvars_var;
case 'b': return &curbuf->b_bufvar;
case 'w': return &curwin->w_winvar;
case 't': return &curtab->tp_winvar;
case 'l': return get_funccal_local_var();
case 'a': return get_funccal_args_var();
}
return NULL;
}

hi = hash_find(ht, varname);
if (HASHITEM_EMPTY(hi))
{




if (ht == &globvarht && !no_autoload)
{


if (!script_autoload(varname, FALSE) || aborting())
return NULL;
hi = hash_find(ht, varname);
}
if (HASHITEM_EMPTY(hi))
return NULL;
}
return HI2DI(hi);
}




hashtab_T *
get_script_local_ht(void)
{
scid_T sid = current_sctx.sc_sid;

if (sid > 0 && sid <= script_items.ga_len)
return &SCRIPT_VARS(sid);
return NULL;
}





int
lookup_scriptvar(char_u *name, size_t len, cctx_T *dummy UNUSED)
{
hashtab_T *ht = get_script_local_ht();
char_u buffer[30];
char_u *p;
int res;
hashitem_T *hi;

if (ht == NULL)
return -1;
if (len < sizeof(buffer) - 1)
{
vim_strncpy(buffer, name, len);
p = buffer;
}
else
{
p = vim_strnsave(name, (int)len);
if (p == NULL)
return -1;
}

hi = hash_find(ht, p);
res = HASHITEM_EMPTY(hi) ? -1 : 1;


if (res == -1 && find_imported(p, 0, NULL) != NULL)
res = 1;

if (p != buffer)
vim_free(p);
return res;
}






hashtab_T *
find_var_ht(char_u *name, char_u **varname)
{
hashitem_T *hi;
hashtab_T *ht;

if (name[0] == NUL)
return NULL;
if (name[1] != ':')
{

if (name[0] == ':' || name[0] == AUTOLOAD_CHAR)
return NULL;
*varname = name;



if (current_sctx.sc_version < 3)
{
hi = hash_find(&compat_hashtab, name);
if (!HASHITEM_EMPTY(hi))
return &compat_hashtab;
}

ht = get_funccal_local_ht();
if (ht != NULL)
return ht; 


if (current_sctx.sc_version == SCRIPT_VERSION_VIM9)
{
ht = get_script_local_ht();
if (ht != NULL)
return ht;
}

return &globvarht; 
}
*varname = name + 2;
if (*name == 'g') 
return &globvarht;

if (vim_strchr(name + 2, ':') != NULL
|| vim_strchr(name + 2, AUTOLOAD_CHAR) != NULL)
return NULL;
if (*name == 'b') 
return &curbuf->b_vars->dv_hashtab;
if (*name == 'w') 
return &curwin->w_vars->dv_hashtab;
if (*name == 't') 
return &curtab->tp_vars->dv_hashtab;
if (*name == 'v') 
return &vimvarht;
if (get_current_funccal() != NULL
&& get_current_funccal()->func->uf_dfunc_idx < 0)
{

if (*name == 'a') 
return get_funccal_args_ht();
if (*name == 'l') 
return get_funccal_local_ht();
}
if (*name == 's') 
{
ht = get_script_local_ht();
if (ht != NULL)
return ht;
}
return NULL;
}






char_u *
get_var_value(char_u *name)
{
dictitem_T *v;

v = find_var(name, NULL, FALSE);
if (v == NULL)
return NULL;
return tv_get_string(&v->di_tv);
}





void
new_script_vars(scid_T id)
{
scriptvar_T *sv;

sv = ALLOC_CLEAR_ONE(scriptvar_T);
if (sv == NULL)
return;
init_var_dict(&sv->sv_dict, &sv->sv_var, VAR_SCOPE);
SCRIPT_ITEM(id)->sn_vars = sv;
}





void
init_var_dict(dict_T *dict, dictitem_T *dict_var, int scope)
{
hash_init(&dict->dv_hashtab);
dict->dv_lock = 0;
dict->dv_scope = scope;
dict->dv_refcount = DO_NOT_FREE_CNT;
dict->dv_copyID = 0;
dict_var->di_tv.vval.v_dict = dict;
dict_var->di_tv.v_type = VAR_DICT;
dict_var->di_tv.v_lock = VAR_FIXED;
dict_var->di_flags = DI_FLAGS_RO | DI_FLAGS_FIX;
dict_var->di_key[0] = NUL;
}




void
unref_var_dict(dict_T *dict)
{


dict->dv_refcount -= DO_NOT_FREE_CNT - 1;
dict_unref(dict);
}






void
vars_clear(hashtab_T *ht)
{
vars_clear_ext(ht, TRUE);
}




void
vars_clear_ext(hashtab_T *ht, int free_val)
{
int todo;
hashitem_T *hi;
dictitem_T *v;

hash_lock(ht);
todo = (int)ht->ht_used;
for (hi = ht->ht_array; todo > 0; ++hi)
{
if (!HASHITEM_EMPTY(hi))
{
--todo;




v = HI2DI(hi);
if (free_val)
clear_tv(&v->di_tv);
if (v->di_flags & DI_FLAGS_ALLOC)
vim_free(v);
}
}
hash_clear(ht);
ht->ht_used = 0;
}





static void
delete_var(hashtab_T *ht, hashitem_T *hi)
{
dictitem_T *di = HI2DI(hi);

hash_remove(ht, hi);
clear_tv(&di->di_tv);
vim_free(di);
}




static void
list_one_var(dictitem_T *v, char *prefix, int *first)
{
char_u *tofree;
char_u *s;
char_u numbuf[NUMBUFLEN];

s = echo_string(&v->di_tv, &tofree, numbuf, get_copyID());
list_one_var_a(prefix, v->di_key, v->di_tv.v_type,
s == NULL ? (char_u *)"" : s, first);
vim_free(tofree);
}

static void
list_one_var_a(
char *prefix,
char_u *name,
int type,
char_u *string,
int *first) 
{

msg_start();
msg_puts(prefix);
if (name != NULL) 
msg_puts((char *)name);
msg_putchar(' ');
msg_advance(22);
if (type == VAR_NUMBER)
msg_putchar('#');
else if (type == VAR_FUNC || type == VAR_PARTIAL)
msg_putchar('*');
else if (type == VAR_LIST)
{
msg_putchar('[');
if (*string == '[')
++string;
}
else if (type == VAR_DICT)
{
msg_putchar('{');
if (*string == '{')
++string;
}
else
msg_putchar(' ');

msg_outtrans(string);

if (type == VAR_FUNC || type == VAR_PARTIAL)
msg_puts("()");
if (*first)
{
msg_clr_eos();
*first = FALSE;
}
}






void
set_var(
char_u *name,
typval_T *tv,
int copy) 
{
set_var_const(name, NULL, tv, copy, 0);
}






void
set_var_const(
char_u *name,
type_T *type,
typval_T *tv,
int copy, 
int flags) 
{
dictitem_T *di;
char_u *varname;
hashtab_T *ht;
int is_script_local;

ht = find_var_ht(name, &varname);
if (ht == NULL || *varname == NUL)
{
semsg(_(e_illvar), name);
return;
}
is_script_local = ht == get_script_local_ht();

di = find_var_in_ht(ht, 0, varname, TRUE);


if (di == NULL)
di = find_var_in_scoped_ht(name, TRUE);

if ((tv->v_type == VAR_FUNC || tv->v_type == VAR_PARTIAL)
&& var_check_func_name(name, di == NULL))
return;

if (di != NULL)
{
if ((di->di_flags & DI_FLAGS_RELOAD) == 0)
{
if (flags & LET_IS_CONST)
{
emsg(_(e_cannot_mod));
return;
}

if (var_check_ro(di->di_flags, name, FALSE)
|| var_check_lock(di->di_tv.v_lock, name, FALSE))
return;

if ((flags & LET_NO_COMMAND) == 0
&& is_script_local
&& current_sctx.sc_version == SCRIPT_VERSION_VIM9)
{
semsg(_("E1041: Redefining script item %s"), name);
return;
}
}
else

di->di_flags &= ~DI_FLAGS_RELOAD;





if (ht == &vimvarht)
{
if (di->di_tv.v_type == VAR_STRING)
{
VIM_CLEAR(di->di_tv.vval.v_string);
if (copy || tv->v_type != VAR_STRING)
{
char_u *val = tv_get_string(tv);



if (di->di_tv.vval.v_string == NULL)
di->di_tv.vval.v_string = vim_strsave(val);
}
else
{

di->di_tv.vval.v_string = tv->vval.v_string;
tv->vval.v_string = NULL;
}
return;
}
else if (di->di_tv.v_type == VAR_NUMBER)
{
di->di_tv.vval.v_number = tv_get_number(tv);
if (STRCMP(varname, "searchforward") == 0)
set_search_direction(di->di_tv.vval.v_number ? '/' : '?');
#if defined(FEAT_SEARCH_EXTRA)
else if (STRCMP(varname, "hlsearch") == 0)
{
no_hlsearch = !di->di_tv.vval.v_number;
redraw_all_later(SOME_VALID);
}
#endif
return;
}
else if (di->di_tv.v_type != tv->v_type)
{
semsg(_("E963: setting %s to value with wrong type"), name);
return;
}
}

clear_tv(&di->di_tv);
}
else 
{

if (ht == &vimvarht || ht == get_funccal_args_ht())
{
semsg(_(e_illvar), name);
return;
}


if (!valid_varname(varname))
return;

di = alloc(sizeof(dictitem_T) + STRLEN(varname));
if (di == NULL)
return;
STRCPY(di->di_key, varname);
if (hash_add(ht, DI2HIKEY(di)) == FAIL)
{
vim_free(di);
return;
}
di->di_flags = DI_FLAGS_ALLOC;
if (flags & LET_IS_CONST)
di->di_flags |= DI_FLAGS_LOCK;

if (is_script_local && current_sctx.sc_version == SCRIPT_VERSION_VIM9)
{
scriptitem_T *si = SCRIPT_ITEM(current_sctx.sc_sid);



if (ga_grow(&si->sn_var_vals, 1) == OK)
{
svar_T *sv = ((svar_T *)si->sn_var_vals.ga_data)
+ si->sn_var_vals.ga_len;
sv->sv_name = di->di_key;
sv->sv_tv = &di->di_tv;
sv->sv_type = type == NULL ? &t_any : type;
sv->sv_const = (flags & LET_IS_CONST);
sv->sv_export = is_export;
++si->sn_var_vals.ga_len;


is_export = FALSE;
}
}
}

if (copy || tv->v_type == VAR_NUMBER || tv->v_type == VAR_FLOAT)
copy_tv(tv, &di->di_tv);
else
{
di->di_tv = *tv;
di->di_tv.v_lock = 0;
init_tv(tv);
}

if (flags & LET_IS_CONST)
di->di_tv.v_lock |= VAR_LOCKED;
}





int
var_check_ro(int flags, char_u *name, int use_gettext)
{
if (flags & DI_FLAGS_RO)
{
semsg(_(e_readonlyvar), use_gettext ? (char_u *)_(name) : name);
return TRUE;
}
if ((flags & DI_FLAGS_RO_SBX) && sandbox)
{
semsg(_(e_readonlysbx), use_gettext ? (char_u *)_(name) : name);
return TRUE;
}
return FALSE;
}





int
var_check_fixed(int flags, char_u *name, int use_gettext)
{
if (flags & DI_FLAGS_FIX)
{
semsg(_("E795: Cannot delete variable %s"),
use_gettext ? (char_u *)_(name) : name);
return TRUE;
}
return FALSE;
}





int
var_check_func_name(
char_u *name, 
int new_var) 
{

if (!(vim_strchr((char_u *)"wbst", name[0]) != NULL && name[1] == ':')
&& !ASCII_ISUPPER((name[0] != NUL && name[1] == ':')
? name[2] : name[0]))
{
semsg(_("E704: Funcref variable name must start with a capital: %s"),
name);
return TRUE;
}



if (new_var && function_exists(name, FALSE))
{
semsg(_("E705: Variable name conflicts with existing function: %s"),
name);
return TRUE;
}
return FALSE;
}






int
var_check_lock(int lock, char_u *name, int use_gettext)
{
if (lock & VAR_LOCKED)
{
semsg(_("E741: Value is locked: %s"),
name == NULL ? (char_u *)_("Unknown")
: use_gettext ? (char_u *)_(name)
: name);
return TRUE;
}
if (lock & VAR_FIXED)
{
semsg(_("E742: Cannot change value of %s"),
name == NULL ? (char_u *)_("Unknown")
: use_gettext ? (char_u *)_(name)
: name);
return TRUE;
}
return FALSE;
}





int
valid_varname(char_u *varname)
{
char_u *p;

for (p = varname; *p != NUL; ++p)
if (!eval_isnamec1(*p) && (p == varname || !VIM_ISDIGIT(*p))
&& *p != AUTOLOAD_CHAR)
{
semsg(_(e_illvar), varname);
return FALSE;
}
return TRUE;
}




static void
getwinvar(
typval_T *argvars,
typval_T *rettv,
int off) 
{
win_T *win;
char_u *varname;
dictitem_T *v;
tabpage_T *tp = NULL;
int done = FALSE;
win_T *oldcurwin;
tabpage_T *oldtabpage;
int need_switch_win;

if (off == 1)
tp = find_tabpage((int)tv_get_number_chk(&argvars[0], NULL));
else
tp = curtab;
win = find_win_by_nr(&argvars[off], tp);
varname = tv_get_string_chk(&argvars[off + 1]);
++emsg_off;

rettv->v_type = VAR_STRING;
rettv->vval.v_string = NULL;

if (win != NULL && varname != NULL)
{



need_switch_win = !(tp == curtab && win == curwin);
if (!need_switch_win
|| switch_win(&oldcurwin, &oldtabpage, win, tp, TRUE) == OK)
{
if (*varname == '&')
{
if (varname[1] == NUL)
{

dict_T *opts = get_winbuf_options(FALSE);

if (opts != NULL)
{
rettv_dict_set(rettv, opts);
done = TRUE;
}
}
else if (get_option_tv(&varname, rettv, 1) == OK)

done = TRUE;
}
else
{


v = find_var_in_ht(&win->w_vars->dv_hashtab, 'w',
varname, FALSE);
if (v != NULL)
{
copy_tv(&v->di_tv, rettv);
done = TRUE;
}
}
}

if (need_switch_win)

restore_win(oldcurwin, oldtabpage, TRUE);
}

if (!done && argvars[off + 2].v_type != VAR_UNKNOWN)

copy_tv(&argvars[off + 2], rettv);

--emsg_off;
}




static void
setwinvar(typval_T *argvars, int off)
{
win_T *win;
win_T *save_curwin;
tabpage_T *save_curtab;
int need_switch_win;
char_u *varname, *winvarname;
typval_T *varp;
char_u nbuf[NUMBUFLEN];
tabpage_T *tp = NULL;

if (check_secure())
return;

if (off == 1)
tp = find_tabpage((int)tv_get_number_chk(&argvars[0], NULL));
else
tp = curtab;
win = find_win_by_nr(&argvars[off], tp);
varname = tv_get_string_chk(&argvars[off + 1]);
varp = &argvars[off + 2];

if (win != NULL && varname != NULL && varp != NULL)
{
need_switch_win = !(tp == curtab && win == curwin);
if (!need_switch_win
|| switch_win(&save_curwin, &save_curtab, win, tp, TRUE) == OK)
{
if (*varname == '&')
{
long numval;
char_u *strval;
int error = FALSE;

++varname;
numval = (long)tv_get_number_chk(varp, &error);
strval = tv_get_string_buf_chk(varp, nbuf);
if (!error && strval != NULL)
set_option_value(varname, numval, strval, OPT_LOCAL);
}
else
{
winvarname = alloc(STRLEN(varname) + 3);
if (winvarname != NULL)
{
STRCPY(winvarname, "w:");
STRCPY(winvarname + 2, varname);
set_var(winvarname, varp, TRUE);
vim_free(winvarname);
}
}
}
if (need_switch_win)
restore_win(save_curwin, save_curtab, TRUE);
}
}





void
reset_v_option_vars(void)
{
set_vim_var_string(VV_OPTION_NEW, NULL, -1);
set_vim_var_string(VV_OPTION_OLD, NULL, -1);
set_vim_var_string(VV_OPTION_OLDLOCAL, NULL, -1);
set_vim_var_string(VV_OPTION_OLDGLOBAL, NULL, -1);
set_vim_var_string(VV_OPTION_TYPE, NULL, -1);
set_vim_var_string(VV_OPTION_COMMAND, NULL, -1);
}




void
assert_error(garray_T *gap)
{
struct vimvar *vp = &vimvars[VV_ERRORS];

if (vp->vv_type != VAR_LIST || vimvars[VV_ERRORS].vv_list == NULL)

set_vim_var_list(VV_ERRORS, list_alloc());
list_append_string(vimvars[VV_ERRORS].vv_list, gap->ga_data, gap->ga_len);
}

int
var_exists(char_u *var)
{
char_u *name;
char_u *tofree;
typval_T tv;
int len = 0;
int n = FALSE;


name = var;
len = get_name_len(&var, &tofree, TRUE, FALSE);
if (len > 0)
{
if (tofree != NULL)
name = tofree;
n = (get_var_tv(name, len, &tv, NULL, FALSE, TRUE) == OK);
if (n)
{

n = (handle_subscript(&var, &tv, TRUE, FALSE, name, &name) == OK);
if (n)
clear_tv(&tv);
}
}
if (*var != NUL)
n = FALSE;

vim_free(tofree);
return n;
}

static lval_T *redir_lval = NULL;
#define EVALCMD_BUSY (redir_lval == (lval_T *)&redir_lval)
static garray_T redir_ga; 
static char_u *redir_endp = NULL;
static char_u *redir_varname = NULL;






int
var_redir_start(char_u *name, int append)
{
int save_emsg;
int err;
typval_T tv;


if (!eval_isnamec1(*name))
{
emsg(_(e_invarg));
return FAIL;
}


redir_varname = vim_strsave(name);
if (redir_varname == NULL)
return FAIL;

redir_lval = ALLOC_CLEAR_ONE(lval_T);
if (redir_lval == NULL)
{
var_redir_stop();
return FAIL;
}


ga_init2(&redir_ga, (int)sizeof(char), 500);


redir_endp = get_lval(redir_varname, NULL, redir_lval, FALSE, FALSE, 0,
FNE_CHECK_START);
if (redir_endp == NULL || redir_lval->ll_name == NULL || *redir_endp != NUL)
{
clear_lval(redir_lval);
if (redir_endp != NULL && *redir_endp != NUL)

emsg(_(e_trailing));
else
emsg(_(e_invarg));
redir_endp = NULL; 
var_redir_stop();
return FAIL;
}



save_emsg = did_emsg;
did_emsg = FALSE;
tv.v_type = VAR_STRING;
tv.vval.v_string = (char_u *)"";
if (append)
set_var_lval(redir_lval, redir_endp, &tv, TRUE, 0, (char_u *)".");
else
set_var_lval(redir_lval, redir_endp, &tv, TRUE, 0, (char_u *)"=");
clear_lval(redir_lval);
err = did_emsg;
did_emsg |= save_emsg;
if (err)
{
redir_endp = NULL; 
var_redir_stop();
return FAIL;
}

return OK;
}










void
var_redir_str(char_u *value, int value_len)
{
int len;

if (redir_lval == NULL)
return;

if (value_len == -1)
len = (int)STRLEN(value); 
else
len = value_len; 

if (ga_grow(&redir_ga, len) == OK)
{
mch_memmove((char *)redir_ga.ga_data + redir_ga.ga_len, value, len);
redir_ga.ga_len += len;
}
else
var_redir_stop();
}





void
var_redir_stop(void)
{
typval_T tv;

if (EVALCMD_BUSY)
{
redir_lval = NULL;
return;
}

if (redir_lval != NULL)
{

if (redir_endp != NULL)
{
ga_append(&redir_ga, NUL); 
tv.v_type = VAR_STRING;
tv.vval.v_string = redir_ga.ga_data;


redir_endp = get_lval(redir_varname, NULL, redir_lval,
FALSE, FALSE, 0, FNE_CHECK_START);
if (redir_endp != NULL && redir_lval->ll_name != NULL)
set_var_lval(redir_lval, redir_endp, &tv, FALSE, 0,
(char_u *)".");
clear_lval(redir_lval);
}


VIM_CLEAR(redir_ga.ga_data);

VIM_CLEAR(redir_lval);
}
VIM_CLEAR(redir_varname);
}




void
f_gettabvar(typval_T *argvars, typval_T *rettv)
{
win_T *oldcurwin;
tabpage_T *tp, *oldtabpage;
dictitem_T *v;
char_u *varname;
int done = FALSE;

rettv->v_type = VAR_STRING;
rettv->vval.v_string = NULL;

varname = tv_get_string_chk(&argvars[1]);
tp = find_tabpage((int)tv_get_number_chk(&argvars[0], NULL));
if (tp != NULL && varname != NULL)
{


if (switch_win(&oldcurwin, &oldtabpage,
tp == curtab || tp->tp_firstwin == NULL ? firstwin
: tp->tp_firstwin, tp, TRUE) == OK)
{


v = find_var_in_ht(&tp->tp_vars->dv_hashtab, 't', varname, FALSE);
if (v != NULL)
{
copy_tv(&v->di_tv, rettv);
done = TRUE;
}
}


restore_win(oldcurwin, oldtabpage, TRUE);
}

if (!done && argvars[2].v_type != VAR_UNKNOWN)
copy_tv(&argvars[2], rettv);
}




void
f_gettabwinvar(typval_T *argvars, typval_T *rettv)
{
getwinvar(argvars, rettv, 1);
}




void
f_getwinvar(typval_T *argvars, typval_T *rettv)
{
getwinvar(argvars, rettv, 0);
}




void
f_getbufvar(typval_T *argvars, typval_T *rettv)
{
buf_T *buf;
char_u *varname;
dictitem_T *v;
int done = FALSE;

(void)tv_get_number(&argvars[0]); 
varname = tv_get_string_chk(&argvars[1]);
++emsg_off;
buf = tv_get_buf(&argvars[0], FALSE);

rettv->v_type = VAR_STRING;
rettv->vval.v_string = NULL;

if (buf != NULL && varname != NULL)
{
if (*varname == '&')
{
buf_T *save_curbuf = curbuf;


curbuf = buf;

if (varname[1] == NUL)
{

dict_T *opts = get_winbuf_options(TRUE);

if (opts != NULL)
{
rettv_dict_set(rettv, opts);
done = TRUE;
}
}
else if (get_option_tv(&varname, rettv, TRUE) == OK)

done = TRUE;


curbuf = save_curbuf;
}
else
{


v = find_var_in_ht(&buf->b_vars->dv_hashtab, 'b', varname, FALSE);
if (v != NULL)
{
copy_tv(&v->di_tv, rettv);
done = TRUE;
}
}
}

if (!done && argvars[2].v_type != VAR_UNKNOWN)

copy_tv(&argvars[2], rettv);

--emsg_off;
}




void
f_settabvar(typval_T *argvars, typval_T *rettv UNUSED)
{
tabpage_T *save_curtab;
tabpage_T *tp;
char_u *varname, *tabvarname;
typval_T *varp;

if (check_secure())
return;

tp = find_tabpage((int)tv_get_number_chk(&argvars[0], NULL));
varname = tv_get_string_chk(&argvars[1]);
varp = &argvars[2];

if (varname != NULL && varp != NULL && tp != NULL)
{
save_curtab = curtab;
goto_tabpage_tp(tp, FALSE, FALSE);

tabvarname = alloc(STRLEN(varname) + 3);
if (tabvarname != NULL)
{
STRCPY(tabvarname, "t:");
STRCPY(tabvarname + 2, varname);
set_var(tabvarname, varp, TRUE);
vim_free(tabvarname);
}


if (valid_tabpage(save_curtab))
goto_tabpage_tp(save_curtab, FALSE, FALSE);
}
}




void
f_settabwinvar(typval_T *argvars, typval_T *rettv UNUSED)
{
setwinvar(argvars, 1);
}




void
f_setwinvar(typval_T *argvars, typval_T *rettv UNUSED)
{
setwinvar(argvars, 0);
}




void
f_setbufvar(typval_T *argvars, typval_T *rettv UNUSED)
{
buf_T *buf;
char_u *varname, *bufvarname;
typval_T *varp;
char_u nbuf[NUMBUFLEN];

if (check_secure())
return;
(void)tv_get_number(&argvars[0]); 
varname = tv_get_string_chk(&argvars[1]);
buf = tv_get_buf(&argvars[0], FALSE);
varp = &argvars[2];

if (buf != NULL && varname != NULL && varp != NULL)
{
if (*varname == '&')
{
long numval;
char_u *strval;
int error = FALSE;
aco_save_T aco;


aucmd_prepbuf(&aco, buf);

++varname;
numval = (long)tv_get_number_chk(varp, &error);
strval = tv_get_string_buf_chk(varp, nbuf);
if (!error && strval != NULL)
set_option_value(varname, numval, strval, OPT_LOCAL);


aucmd_restbuf(&aco);
}
else
{
bufvarname = alloc(STRLEN(varname) + 3);
if (bufvarname != NULL)
{
buf_T *save_curbuf = curbuf;

curbuf = buf;
STRCPY(bufvarname, "b:");
STRCPY(bufvarname + 2, varname);
set_var(bufvarname, varp, TRUE);
vim_free(bufvarname);
curbuf = save_curbuf;
}
}
}
}







callback_T
get_callback(typval_T *arg)
{
callback_T res;
int r = OK;

res.cb_free_name = FALSE;
if (arg->v_type == VAR_PARTIAL && arg->vval.v_partial != NULL)
{
res.cb_partial = arg->vval.v_partial;
++res.cb_partial->pt_refcount;
res.cb_name = partial_name(res.cb_partial);
}
else
{
res.cb_partial = NULL;
if (arg->v_type == VAR_STRING && arg->vval.v_string != NULL
&& isdigit(*arg->vval.v_string))
r = FAIL;
else if (arg->v_type == VAR_FUNC || arg->v_type == VAR_STRING)
{

res.cb_name = arg->vval.v_string;
func_ref(res.cb_name);
}
else if (arg->v_type == VAR_NUMBER && arg->vval.v_number == 0)
res.cb_name = (char_u *)"";
else
r = FAIL;

if (r == FAIL)
{
emsg(_("E921: Invalid callback argument"));
res.cb_name = NULL;
}
}
return res;
}




void
put_callback(callback_T *cb, typval_T *tv)
{
if (cb->cb_partial != NULL)
{
tv->v_type = VAR_PARTIAL;
tv->vval.v_partial = cb->cb_partial;
++tv->vval.v_partial->pt_refcount;
}
else
{
tv->v_type = VAR_FUNC;
tv->vval.v_string = vim_strsave(cb->cb_name);
func_ref(cb->cb_name);
}
}





void
set_callback(callback_T *dest, callback_T *src)
{
if (src->cb_partial == NULL)
{

dest->cb_name = vim_strsave(src->cb_name);
dest->cb_free_name = TRUE;
}
else
{

dest->cb_name = src->cb_name;
dest->cb_free_name = FALSE;
}
dest->cb_partial = src->cb_partial;
}




void
free_callback(callback_T *callback)
{
if (callback->cb_partial != NULL)
{
partial_unref(callback->cb_partial);
callback->cb_partial = NULL;
}
else if (callback->cb_name != NULL)
func_unref(callback->cb_name);
if (callback->cb_free_name)
{
vim_free(callback->cb_name);
callback->cb_free_name = FALSE;
}
callback->cb_name = NULL;
}

#endif 
