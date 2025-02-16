












#include "vim.h"

#if defined(FEAT_EVAL) || defined(PROTO)

#define FC_ABORT 0x01 
#define FC_RANGE 0x02 
#define FC_DICT 0x04 
#define FC_CLOSURE 0x08 
#define FC_DELETED 0x10 
#define FC_REMOVED 0x20 
#define FC_SANDBOX 0x40 
#define FC_DEAD 0x80 
#define FC_EXPORT 0x100 
#define FC_NOARGS 0x200 




static hashtab_T func_hashtab;


static garray_T funcargs = GA_EMPTY;


static funccall_T *current_funccal = NULL;



static funccall_T *previous_funccal = NULL;

static char *e_funcexts = N_("E122: Function %s already exists, add ! to replace it");
static char *e_funcdict = N_("E717: Dictionary entry already exists");
static char *e_funcref = N_("E718: Funcref required");
static char *e_nofunc = N_("E130: Unknown function: %s");

static void funccal_unref(funccall_T *fc, ufunc_T *fp, int force);

void
func_init()
{
hash_init(&func_hashtab);
}




hashtab_T *
func_tbl_get(void)
{
return &func_hashtab;
}






static char_u *
one_function_arg(char_u *arg, garray_T *newargs, garray_T *argtypes, int skip)
{
char_u *p = arg;

while (ASCII_ISALNUM(*p) || *p == '_')
++p;
if (arg == p || isdigit(*arg)
|| (p - arg == 9 && STRNCMP(arg, "firstline", 9) == 0)
|| (p - arg == 8 && STRNCMP(arg, "lastline", 8) == 0))
{
if (!skip)
semsg(_("E125: Illegal argument: %s"), arg);
return arg;
}
if (newargs != NULL && ga_grow(newargs, 1) == FAIL)
return arg;
if (newargs != NULL)
{
char_u *arg_copy;
int c;
int i;

c = *p;
*p = NUL;
arg_copy = vim_strsave(arg);
if (arg_copy == NULL)
{
*p = c;
return arg;
}


for (i = 0; i < newargs->ga_len; ++i)
if (STRCMP(((char_u **)(newargs->ga_data))[i], arg_copy) == 0)
{
semsg(_("E853: Duplicate argument name: %s"), arg_copy);
vim_free(arg_copy);
return arg;
}
((char_u **)(newargs->ga_data))[newargs->ga_len] = arg_copy;
newargs->ga_len++;

*p = c;
}


if (argtypes != NULL && ga_grow(argtypes, 1) == OK)
{
char_u *type = NULL;

if (*p == ':')
{
type = skipwhite(p + 1);
p = skip_type(type);
type = vim_strnsave(type, p - type);
}
else if (*skipwhite(p) == ':')
emsg(_("E1059: No white space allowed before :"));
((char_u **)argtypes->ga_data)[argtypes->ga_len++] = type;
}

return p;
}




int
get_function_args(
char_u **argp,
char_u endchar,
garray_T *newargs,
garray_T *argtypes, 
int *varargs,
garray_T *default_args,
int skip)
{
int mustend = FALSE;
char_u *arg = *argp;
char_u *p = arg;
int c;
int any_default = FALSE;
char_u *expr;

if (newargs != NULL)
ga_init2(newargs, (int)sizeof(char_u *), 3);
if (argtypes != NULL)
ga_init2(argtypes, (int)sizeof(char_u *), 3);
if (default_args != NULL)
ga_init2(default_args, (int)sizeof(char_u *), 3);

if (varargs != NULL)
*varargs = FALSE;




while (*p != endchar)
{
if (p[0] == '.' && p[1] == '.' && p[2] == '.')
{
if (varargs != NULL)
*varargs = TRUE;
p += 3;
mustend = TRUE;

if (argtypes != NULL)
{

if (!ASCII_ISALPHA(*p))
{
emsg(_("E1055: Missing name after ..."));
break;
}

arg = p;
p = one_function_arg(p, newargs, argtypes, skip);
if (p == arg)
break;
}
}
else
{
arg = p;
p = one_function_arg(p, newargs, argtypes, skip);
if (p == arg)
break;

if (*skipwhite(p) == '=' && default_args != NULL)
{
typval_T rettv;


any_default = TRUE;
p = skipwhite(p) + 1;
p = skipwhite(p);
expr = p;
if (eval1(&p, &rettv, FALSE) != FAIL)
{
if (ga_grow(default_args, 1) == FAIL)
goto err_ret;


while (p > expr && VIM_ISWHITE(p[-1]))
p--;
c = *p;
*p = NUL;
expr = vim_strsave(expr);
if (expr == NULL)
{
*p = c;
goto err_ret;
}
((char_u **)(default_args->ga_data))
[default_args->ga_len] = expr;
default_args->ga_len++;
*p = c;
}
else
mustend = TRUE;
}
else if (any_default)
{
emsg(_("E989: Non-default argument follows default argument"));
mustend = TRUE;
}
if (*p == ',')
++p;
else
mustend = TRUE;
}
p = skipwhite(p);
if (mustend && *p != endchar)
{
if (!skip)
semsg(_(e_invarg2), *argp);
break;
}
}
if (*p != endchar)
goto err_ret;
++p; 

*argp = p;
return OK;

err_ret:
if (newargs != NULL)
ga_clear_strings(newargs);
if (default_args != NULL)
ga_clear_strings(default_args);
return FAIL;
}




static int
register_closure(ufunc_T *fp)
{
if (fp->uf_scoped == current_funccal)

return OK;
funccal_unref(fp->uf_scoped, fp, FALSE);
fp->uf_scoped = current_funccal;
current_funccal->fc_refcount++;

if (ga_grow(&current_funccal->fc_funcs, 1) == FAIL)
return FAIL;
((ufunc_T **)current_funccal->fc_funcs.ga_data)
[current_funccal->fc_funcs.ga_len++] = fp;
return OK;
}

static void
set_ufunc_name(ufunc_T *fp, char_u *name)
{
STRCPY(fp->uf_name, name);

if (name[0] == K_SPECIAL)
{
fp->uf_name_exp = alloc(STRLEN(name) + 3);
if (fp->uf_name_exp != NULL)
{
STRCPY(fp->uf_name_exp, "<SNR>");
STRCAT(fp->uf_name_exp, fp->uf_name + 3);
}
}
}





int
get_lambda_tv(char_u **arg, typval_T *rettv, int evaluate)
{
garray_T newargs;
garray_T newlines;
garray_T *pnewargs;
ufunc_T *fp = NULL;
partial_T *pt = NULL;
int varargs;
int ret;
char_u *start = skipwhite(*arg + 1);
char_u *s, *e;
static int lambda_no = 0;
int *old_eval_lavars = eval_lavars_used;
int eval_lavars = FALSE;

ga_init(&newargs);
ga_init(&newlines);


ret = get_function_args(&start, '-', NULL, NULL, NULL, NULL, TRUE);
if (ret == FAIL || *start != '>')
return NOTDONE;


if (evaluate)
pnewargs = &newargs;
else
pnewargs = NULL;
*arg = skipwhite(*arg + 1);

ret = get_function_args(arg, '-', pnewargs, NULL, &varargs, NULL, FALSE);
if (ret == FAIL || **arg != '>')
goto errret;


if (evaluate)
eval_lavars_used = &eval_lavars;


*arg = skipwhite(*arg + 1);
s = *arg;
ret = skip_expr(arg);
if (ret == FAIL)
goto errret;
e = *arg;
*arg = skipwhite(*arg);
if (**arg != '}')
{
semsg(_("E451: Expected }: %s"), *arg);
goto errret;
}
++*arg;

if (evaluate)
{
int len, flags = 0;
char_u *p;
char_u name[20];

sprintf((char*)name, "<lambda>%d", ++lambda_no);

fp = alloc_clear(offsetof(ufunc_T, uf_name) + STRLEN(name) + 1);
if (fp == NULL)
goto errret;
fp->uf_dfunc_idx = -1;
pt = ALLOC_CLEAR_ONE(partial_T);
if (pt == NULL)
goto errret;

ga_init2(&newlines, (int)sizeof(char_u *), 1);
if (ga_grow(&newlines, 1) == FAIL)
goto errret;


len = 7 + e - s + 1;
p = alloc(len);
if (p == NULL)
goto errret;
((char_u **)(newlines.ga_data))[newlines.ga_len++] = p;
STRCPY(p, "return ");
vim_strncpy(p + 7, s, e - s);
if (strstr((char *)p + 7, "a:") == NULL)

flags |= FC_NOARGS;

fp->uf_refcount = 1;
set_ufunc_name(fp, name);
hash_add(&func_hashtab, UF2HIKEY(fp));
fp->uf_args = newargs;
ga_init(&fp->uf_def_args);
fp->uf_lines = newlines;
if (current_funccal != NULL && eval_lavars)
{
flags |= FC_CLOSURE;
if (register_closure(fp) == FAIL)
goto errret;
}
else
fp->uf_scoped = NULL;

#if defined(FEAT_PROFILE)
if (prof_def_func())
func_do_profile(fp);
#endif
if (sandbox)
flags |= FC_SANDBOX;

fp->uf_varargs = TRUE;
fp->uf_flags = flags;
fp->uf_calls = 0;
fp->uf_script_ctx = current_sctx;
fp->uf_script_ctx.sc_lnum += SOURCING_LNUM - newlines.ga_len;

pt->pt_func = fp;
pt->pt_refcount = 1;
rettv->vval.v_partial = pt;
rettv->v_type = VAR_PARTIAL;
}

eval_lavars_used = old_eval_lavars;
return OK;

errret:
ga_clear_strings(&newargs);
ga_clear_strings(&newlines);
vim_free(fp);
vim_free(pt);
eval_lavars_used = old_eval_lavars;
return FAIL;
}







char_u *
deref_func_name(char_u *name, int *lenp, partial_T **partialp, int no_autoload)
{
dictitem_T *v;
int cc;
char_u *s;

if (partialp != NULL)
*partialp = NULL;

cc = name[*lenp];
name[*lenp] = NUL;
v = find_var(name, NULL, no_autoload);
name[*lenp] = cc;
if (v != NULL && v->di_tv.v_type == VAR_FUNC)
{
if (v->di_tv.vval.v_string == NULL)
{
*lenp = 0;
return (char_u *)""; 
}
s = v->di_tv.vval.v_string;
*lenp = (int)STRLEN(s);
return s;
}

if (v != NULL && v->di_tv.v_type == VAR_PARTIAL)
{
partial_T *pt = v->di_tv.vval.v_partial;

if (pt == NULL)
{
*lenp = 0;
return (char_u *)""; 
}
if (partialp != NULL)
*partialp = pt;
s = partial_name(pt);
*lenp = (int)STRLEN(s);
return s;
}

return name;
}





void
emsg_funcname(char *ermsg, char_u *name)
{
char_u *p;

if (*name == K_SPECIAL)
p = concat_str((char_u *)"<SNR>", name + 3);
else
p = name;
semsg(_(ermsg), p);
if (p != name)
vim_free(p);
}





int
get_func_tv(
char_u *name, 
int len, 
typval_T *rettv,
char_u **arg, 
funcexe_T *funcexe) 
{
char_u *argp;
int ret = OK;
typval_T argvars[MAX_FUNC_ARGS + 1]; 
int argcount = 0; 




argp = *arg;
while (argcount < MAX_FUNC_ARGS - (funcexe->partial == NULL ? 0
: funcexe->partial->pt_argc))
{
argp = skipwhite(argp + 1); 
if (*argp == ')' || *argp == ',' || *argp == NUL)
break;
if (eval1(&argp, &argvars[argcount], funcexe->evaluate) == FAIL)
{
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

if (ret == OK)
{
int i = 0;

if (get_vim_var_nr(VV_TESTING))
{


if (funcargs.ga_itemsize == 0)
ga_init2(&funcargs, (int)sizeof(typval_T *), 50);
for (i = 0; i < argcount; ++i)
if (ga_grow(&funcargs, 1) == OK)
((typval_T **)funcargs.ga_data)[funcargs.ga_len++] =
&argvars[i];
}

ret = call_func(name, len, rettv, argcount, argvars, funcexe);

funcargs.ga_len -= i;
}
else if (!aborting())
{
if (argcount == MAX_FUNC_ARGS)
emsg_funcname(N_("E740: Too many arguments for function %s"), name);
else
emsg_funcname(N_("E116: Invalid arguments for function %s"), name);
}

while (--argcount >= 0)
clear_tv(&argvars[argcount]);

*arg = skipwhite(argp);
return ret;
}





static int
eval_fname_sid(char_u *p)
{
return (*p == 's' || TOUPPER_ASC(p[2]) == 'I');
}







char_u *
fname_trans_sid(char_u *name, char_u *fname_buf, char_u **tofree, int *error)
{
int llen;
char_u *fname;
int i;

llen = eval_fname_script(name);
if (llen > 0)
{
fname_buf[0] = K_SPECIAL;
fname_buf[1] = KS_EXTRA;
fname_buf[2] = (int)KE_SNR;
i = 3;
if (eval_fname_sid(name)) 
{
if (current_sctx.sc_sid <= 0)
*error = FCERR_SCRIPT;
else
{
sprintf((char *)fname_buf + 3, "%ld_",
(long)current_sctx.sc_sid);
i = (int)STRLEN(fname_buf);
}
}
if (i + STRLEN(name + llen) < FLEN_FIXED)
{
STRCPY(fname_buf + i, name + llen);
fname = fname_buf;
}
else
{
fname = alloc(i + STRLEN(name + llen) + 1);
if (fname == NULL)
*error = FCERR_OTHER;
else
{
*tofree = fname;
mch_memmove(fname, fname_buf, (size_t)i);
STRCPY(fname + i, name + llen);
}
}
}
else
fname = name;
return fname;
}




static ufunc_T *
find_func_with_sid(char_u *name, int sid)
{
hashitem_T *hi;
char_u buffer[200];

buffer[0] = K_SPECIAL;
buffer[1] = KS_EXTRA;
buffer[2] = (int)KE_SNR;
vim_snprintf((char *)buffer + 3, sizeof(buffer) - 3, "%ld_%s",
(long)sid, name);
hi = hash_find(&func_hashtab, buffer);
if (!HASHITEM_EMPTY(hi))
return HI2UF(hi);

return NULL;
}





static ufunc_T *
find_func_even_dead(char_u *name, cctx_T *cctx)
{
hashitem_T *hi;
ufunc_T *func;
imported_T *imported;

if (in_vim9script())
{

func = find_func_with_sid(name, current_sctx.sc_sid);
if (func != NULL)
return func;


imported = find_imported(name, 0, cctx);
if (imported != NULL && imported->imp_funcname != NULL)
{
hi = hash_find(&func_hashtab, imported->imp_funcname);
if (!HASHITEM_EMPTY(hi))
return HI2UF(hi);
}
}

hi = hash_find(&func_hashtab, name);
if (!HASHITEM_EMPTY(hi))
return HI2UF(hi);

return NULL;
}






ufunc_T *
find_func(char_u *name, cctx_T *cctx)
{
ufunc_T *fp = find_func_even_dead(name, cctx);

if (fp != NULL && (fp->uf_flags & FC_DEAD) == 0)
return fp;
return NULL;
}






static void
cat_func_name(char_u *buf, ufunc_T *fp)
{
if (fp->uf_name[0] == K_SPECIAL)
{
STRCPY(buf, "<SNR>");
STRCAT(buf, fp->uf_name + 3);
}
else
STRCPY(buf, fp->uf_name);
}




static void
add_nr_var(
dict_T *dp,
dictitem_T *v,
char *name,
varnumber_T nr)
{
STRCPY(v->di_key, name);
v->di_flags = DI_FLAGS_RO | DI_FLAGS_FIX;
hash_add(&dp->dv_hashtab, DI2HIKEY(v));
v->di_tv.v_type = VAR_NUMBER;
v->di_tv.v_lock = VAR_FIXED;
v->di_tv.vval.v_number = nr;
}




static void
free_funccal(funccall_T *fc)
{
int i;

for (i = 0; i < fc->fc_funcs.ga_len; ++i)
{
ufunc_T *fp = ((ufunc_T **)(fc->fc_funcs.ga_data))[i];





if (fp != NULL && fp->uf_scoped == fc)
fp->uf_scoped = NULL;
}
ga_clear(&fc->fc_funcs);

func_ptr_unref(fc->func);
vim_free(fc);
}






static void
free_funccal_contents(funccall_T *fc)
{
listitem_T *li;


vars_clear(&fc->l_vars.dv_hashtab);


vars_clear(&fc->l_avars.dv_hashtab);


FOR_ALL_LIST_ITEMS(&fc->l_varlist, li)
clear_tv(&li->li_tv);

free_funccal(fc);
}





static void
cleanup_function_call(funccall_T *fc)
{
int may_free_fc = fc->fc_refcount <= 0;
int free_fc = TRUE;

current_funccal = fc->caller;


if (may_free_fc && fc->l_vars.dv_refcount == DO_NOT_FREE_CNT)
vars_clear(&fc->l_vars.dv_hashtab);
else
free_fc = FALSE;




if (may_free_fc && fc->l_avars.dv_refcount == DO_NOT_FREE_CNT)
vars_clear_ext(&fc->l_avars.dv_hashtab, FALSE);
else
{
int todo;
hashitem_T *hi;
dictitem_T *di;

free_fc = FALSE;


todo = (int)fc->l_avars.dv_hashtab.ht_used;
for (hi = fc->l_avars.dv_hashtab.ht_array; todo > 0; ++hi)
{
if (!HASHITEM_EMPTY(hi))
{
--todo;
di = HI2DI(hi);
copy_tv(&di->di_tv, &di->di_tv);
}
}
}

if (may_free_fc && fc->l_varlist.lv_refcount == DO_NOT_FREE_CNT)
fc->l_varlist.lv_first = NULL;
else
{
listitem_T *li;

free_fc = FALSE;


FOR_ALL_LIST_ITEMS(&fc->l_varlist, li)
copy_tv(&li->li_tv, &li->li_tv);
}

if (free_fc)
free_funccal(fc);
else
{
static int made_copy = 0;




fc->caller = previous_funccal;
previous_funccal = fc;

if (want_garbage_collect)

made_copy = 0;
else if (++made_copy >= (int)((4096 * 1024) / sizeof(*fc)))
{




made_copy = 0;
want_garbage_collect = TRUE;
}
}
}





static void
funccal_unref(funccall_T *fc, ufunc_T *fp, int force)
{
funccall_T **pfc;
int i;

if (fc == NULL)
return;

if (--fc->fc_refcount <= 0 && (force || (
fc->l_varlist.lv_refcount == DO_NOT_FREE_CNT
&& fc->l_vars.dv_refcount == DO_NOT_FREE_CNT
&& fc->l_avars.dv_refcount == DO_NOT_FREE_CNT)))
for (pfc = &previous_funccal; *pfc != NULL; pfc = &(*pfc)->caller)
{
if (fc == *pfc)
{
*pfc = fc->caller;
free_funccal_contents(fc);
return;
}
}
for (i = 0; i < fc->fc_funcs.ga_len; ++i)
if (((ufunc_T **)(fc->fc_funcs.ga_data))[i] == fp)
((ufunc_T **)(fc->fc_funcs.ga_data))[i] = NULL;
}






static int
func_remove(ufunc_T *fp)
{
hashitem_T *hi;


if (fp->uf_flags & FC_DEAD)
return FALSE;

hi = hash_find(&func_hashtab, UF2HIKEY(fp));
if (!HASHITEM_EMPTY(hi))
{


if (fp->uf_dfunc_idx >= 0)
fp->uf_flags |= FC_DEAD;
else
hash_remove(&func_hashtab, hi);
return TRUE;
}
return FALSE;
}

static void
func_clear_items(ufunc_T *fp)
{
ga_clear_strings(&(fp->uf_args));
ga_clear_strings(&(fp->uf_def_args));
ga_clear_strings(&(fp->uf_lines));
VIM_CLEAR(fp->uf_name_exp);
VIM_CLEAR(fp->uf_arg_types);
VIM_CLEAR(fp->uf_def_arg_idx);
VIM_CLEAR(fp->uf_va_name);
ga_clear(&fp->uf_type_list);
#if defined(FEAT_PROFILE)
VIM_CLEAR(fp->uf_tml_count);
VIM_CLEAR(fp->uf_tml_total);
VIM_CLEAR(fp->uf_tml_self);
#endif
}






static void
func_clear(ufunc_T *fp, int force)
{
if (fp->uf_cleared)
return;
fp->uf_cleared = TRUE;


func_clear_items(fp);
funccal_unref(fp->uf_scoped, fp, force);
delete_def_function(fp);
}





static void
func_free(ufunc_T *fp)
{


if ((fp->uf_flags & (FC_DELETED | FC_REMOVED)) == 0)
func_remove(fp);

if ((fp->uf_flags & FC_DEAD) == 0)
vim_free(fp);
}





static void
func_clear_free(ufunc_T *fp, int force)
{
func_clear(fp, force);
func_free(fp);
}





static void
call_user_func(
ufunc_T *fp, 
int argcount, 
typval_T *argvars, 
typval_T *rettv, 
linenr_T firstline, 
linenr_T lastline, 
dict_T *selfdict) 
{
sctx_T save_current_sctx;
int using_sandbox = FALSE;
funccall_T *fc;
int save_did_emsg;
int default_arg_err = FALSE;
static int depth = 0;
dictitem_T *v;
int fixvar_idx = 0; 
int i;
int ai;
int islambda = FALSE;
char_u numbuf[NUMBUFLEN];
char_u *name;
#if defined(FEAT_PROFILE)
proftime_T wait_start;
proftime_T call_start;
int started_profiling = FALSE;
#endif
ESTACK_CHECK_DECLARATION


if (depth >= p_mfd)
{
emsg(_("E132: Function call depth is higher than 'maxfuncdepth'"));
rettv->v_type = VAR_NUMBER;
rettv->vval.v_number = -1;
return;
}
++depth;

line_breakcheck(); 

fc = ALLOC_CLEAR_ONE(funccall_T);
if (fc == NULL)
return;
fc->caller = current_funccal;
current_funccal = fc;
fc->func = fp;
fc->rettv = rettv;
fc->level = ex_nesting_level;

fc->breakpoint = dbg_find_breakpoint(FALSE, fp->uf_name, (linenr_T)0);
fc->dbg_tick = debug_tick;

ga_init2(&fc->fc_funcs, sizeof(ufunc_T *), 1);
func_ptr_ref(fp);

if (fp->uf_dfunc_idx >= 0)
{
estack_push_ufunc(ETYPE_UFUNC, fp, 1);
save_current_sctx = current_sctx;
current_sctx = fp->uf_script_ctx;


call_def_function(fp, argcount, argvars, rettv);
--depth;
current_funccal = fc->caller;

estack_pop();
current_sctx = save_current_sctx;
free_funccal(fc);
return;
}

if (STRNCMP(fp->uf_name, "<lambda>", 8) == 0)
islambda = TRUE;









init_var_dict(&fc->l_vars, &fc->l_vars_var, VAR_DEF_SCOPE);
if (selfdict != NULL)
{


v = &fc->fixvar[fixvar_idx++].var;
name = v->di_key;
STRCPY(name, "self");
v->di_flags = DI_FLAGS_RO | DI_FLAGS_FIX;
hash_add(&fc->l_vars.dv_hashtab, DI2HIKEY(v));
v->di_tv.v_type = VAR_DICT;
v->di_tv.v_lock = 0;
v->di_tv.vval.v_dict = selfdict;
++selfdict->dv_refcount;
}






init_var_dict(&fc->l_avars, &fc->l_avars_var, VAR_SCOPE);
if ((fp->uf_flags & FC_NOARGS) == 0)
add_nr_var(&fc->l_avars, &fc->fixvar[fixvar_idx++].var, "0",
(varnumber_T)(argcount >= fp->uf_args.ga_len
? argcount - fp->uf_args.ga_len : 0));
fc->l_avars.dv_lock = VAR_FIXED;
if ((fp->uf_flags & FC_NOARGS) == 0)
{


v = &fc->fixvar[fixvar_idx++].var;
name = v->di_key;
STRCPY(name, "000");
v->di_flags = DI_FLAGS_RO | DI_FLAGS_FIX;
hash_add(&fc->l_avars.dv_hashtab, DI2HIKEY(v));
v->di_tv.v_type = VAR_LIST;
v->di_tv.v_lock = VAR_FIXED;
v->di_tv.vval.v_list = &fc->l_varlist;
}
vim_memset(&fc->l_varlist, 0, sizeof(list_T));
fc->l_varlist.lv_refcount = DO_NOT_FREE_CNT;
fc->l_varlist.lv_lock = VAR_FIXED;







if ((fp->uf_flags & FC_NOARGS) == 0)
{
add_nr_var(&fc->l_avars, &fc->fixvar[fixvar_idx++].var, "firstline",
(varnumber_T)firstline);
add_nr_var(&fc->l_avars, &fc->fixvar[fixvar_idx++].var, "lastline",
(varnumber_T)lastline);
}
for (i = 0; i < argcount || i < fp->uf_args.ga_len; ++i)
{
int addlocal = FALSE;
typval_T def_rettv;
int isdefault = FALSE;

ai = i - fp->uf_args.ga_len;
if (ai < 0)
{

name = FUNCARG(fp, i);
if (islambda)
addlocal = TRUE;


isdefault = ai + fp->uf_def_args.ga_len >= 0
&& (i >= argcount || (argvars[i].v_type == VAR_SPECIAL
&& argvars[i].vval.v_number == VVAL_NONE));
if (isdefault)
{
char_u *default_expr = NULL;
def_rettv.v_type = VAR_NUMBER;
def_rettv.vval.v_number = -1;

default_expr = ((char_u **)(fp->uf_def_args.ga_data))
[ai + fp->uf_def_args.ga_len];
if (eval1(&default_expr, &def_rettv, TRUE) == FAIL)
{
default_arg_err = 1;
break;
}
}
}
else
{
if ((fp->uf_flags & FC_NOARGS) != 0)

break;


sprintf((char *)numbuf, "%d", ai + 1);
name = numbuf;
}
if (fixvar_idx < FIXVAR_CNT && STRLEN(name) <= VAR_SHORT_LEN)
{
v = &fc->fixvar[fixvar_idx++].var;
v->di_flags = DI_FLAGS_RO | DI_FLAGS_FIX;
STRCPY(v->di_key, name);
}
else
{
v = dictitem_alloc(name);
if (v == NULL)
break;
v->di_flags |= DI_FLAGS_RO | DI_FLAGS_FIX;
}



v->di_tv = isdefault ? def_rettv : argvars[i];
v->di_tv.v_lock = VAR_FIXED;

if (addlocal)
{


copy_tv(&v->di_tv, &v->di_tv);
hash_add(&fc->l_vars.dv_hashtab, DI2HIKEY(v));
}
else
hash_add(&fc->l_avars.dv_hashtab, DI2HIKEY(v));

if (ai >= 0 && ai < MAX_FUNC_ARGS)
{
listitem_T *li = &fc->l_listitems[ai];

li->li_tv = argvars[i];
li->li_tv.v_lock = VAR_FIXED;
list_append(&fc->l_varlist, li);
}
}


++RedrawingDisabled;

if (fp->uf_flags & FC_SANDBOX)
{
using_sandbox = TRUE;
++sandbox;
}

estack_push_ufunc(ETYPE_UFUNC, fp, 1);
ESTACK_CHECK_SETUP
if (p_verbose >= 12)
{
++no_wait_return;
verbose_enter_scroll();

smsg(_("calling %s"), SOURCING_NAME);
if (p_verbose >= 14)
{
char_u buf[MSG_BUF_LEN];
char_u numbuf2[NUMBUFLEN];
char_u *tofree;
char_u *s;

msg_puts("(");
for (i = 0; i < argcount; ++i)
{
if (i > 0)
msg_puts(", ");
if (argvars[i].v_type == VAR_NUMBER)
msg_outnum((long)argvars[i].vval.v_number);
else
{

++emsg_off;
s = tv2string(&argvars[i], &tofree, numbuf2, 0);
--emsg_off;
if (s != NULL)
{
if (vim_strsize(s) > MSG_BUF_CLEN)
{
trunc_string(s, buf, MSG_BUF_CLEN, MSG_BUF_LEN);
s = buf;
}
msg_puts((char *)s);
vim_free(tofree);
}
}
}
msg_puts(")");
}
msg_puts("\n"); 

verbose_leave_scroll();
--no_wait_return;
}
#if defined(FEAT_PROFILE)
if (do_profiling == PROF_YES)
{
if (!fp->uf_profiling && has_profiling(FALSE, fp->uf_name, NULL))
{
started_profiling = TRUE;
func_do_profile(fp);
}
if (fp->uf_profiling
|| (fc->caller != NULL && fc->caller->func->uf_profiling))
{
++fp->uf_tm_count;
profile_start(&call_start);
profile_zero(&fp->uf_tm_children);
}
script_prof_save(&wait_start);
}
#endif

save_current_sctx = current_sctx;
current_sctx = fp->uf_script_ctx;
save_did_emsg = did_emsg;
did_emsg = FALSE;

if (default_arg_err && (fp->uf_flags & FC_ABORT))
did_emsg = TRUE;
else if (islambda)
{
char_u *p = *(char_u **)fp->uf_lines.ga_data + 7;



++ex_nesting_level;
eval1(&p, rettv, TRUE);
--ex_nesting_level;
}
else

do_cmdline(NULL, get_func_line, (void *)fc,
DOCMD_NOWAIT|DOCMD_VERBOSE|DOCMD_REPEAT);

--RedrawingDisabled;


if ((did_emsg && (fp->uf_flags & FC_ABORT)) || rettv->v_type == VAR_UNKNOWN)
{
clear_tv(rettv);
rettv->v_type = VAR_NUMBER;
rettv->vval.v_number = -1;
}

#if defined(FEAT_PROFILE)
if (do_profiling == PROF_YES && (fp->uf_profiling
|| (fc->caller != NULL && fc->caller->func->uf_profiling)))
{
profile_end(&call_start);
profile_sub_wait(&wait_start, &call_start);
profile_add(&fp->uf_tm_total, &call_start);
profile_self(&fp->uf_tm_self, &call_start, &fp->uf_tm_children);
if (fc->caller != NULL && fc->caller->func->uf_profiling)
{
profile_add(&fc->caller->func->uf_tm_children, &call_start);
profile_add(&fc->caller->func->uf_tml_children, &call_start);
}
if (started_profiling)

fp->uf_profiling = FALSE;
}
#endif


if (p_verbose >= 12)
{
++no_wait_return;
verbose_enter_scroll();

if (aborting())
smsg(_("%s aborted"), SOURCING_NAME);
else if (fc->rettv->v_type == VAR_NUMBER)
smsg(_("%s returning #%ld"), SOURCING_NAME,
(long)fc->rettv->vval.v_number);
else
{
char_u buf[MSG_BUF_LEN];
char_u numbuf2[NUMBUFLEN];
char_u *tofree;
char_u *s;




++emsg_off;
s = tv2string(fc->rettv, &tofree, numbuf2, 0);
--emsg_off;
if (s != NULL)
{
if (vim_strsize(s) > MSG_BUF_CLEN)
{
trunc_string(s, buf, MSG_BUF_CLEN, MSG_BUF_LEN);
s = buf;
}
smsg(_("%s returning %s"), SOURCING_NAME, s);
vim_free(tofree);
}
}
msg_puts("\n"); 

verbose_leave_scroll();
--no_wait_return;
}

ESTACK_CHECK_NOW
estack_pop();
current_sctx = save_current_sctx;
#if defined(FEAT_PROFILE)
if (do_profiling == PROF_YES)
script_prof_restore(&wait_start);
#endif
if (using_sandbox)
--sandbox;

if (p_verbose >= 12 && SOURCING_NAME != NULL)
{
++no_wait_return;
verbose_enter_scroll();

smsg(_("continuing in %s"), SOURCING_NAME);
msg_puts("\n"); 

verbose_leave_scroll();
--no_wait_return;
}

did_emsg |= save_did_emsg;
--depth;

cleanup_function_call(fc);
}




int
call_user_func_check(
ufunc_T *fp,
int argcount,
typval_T *argvars,
typval_T *rettv,
funcexe_T *funcexe,
dict_T *selfdict)
{
int error;
int regular_args = fp->uf_args.ga_len;

if (fp->uf_flags & FC_RANGE && funcexe->doesrange != NULL)
*funcexe->doesrange = TRUE;
if (argcount < regular_args - fp->uf_def_args.ga_len)
error = FCERR_TOOFEW;
else if (!has_varargs(fp) && argcount > regular_args)
error = FCERR_TOOMANY;
else if ((fp->uf_flags & FC_DICT) && selfdict == NULL)
error = FCERR_DICT;
else
{
int did_save_redo = FALSE;
save_redo_T save_redo;






save_search_patterns();
if (!ins_compl_active())
{
saveRedobuff(&save_redo);
did_save_redo = TRUE;
}
++fp->uf_calls;
call_user_func(fp, argcount, argvars, rettv,
funcexe->firstline, funcexe->lastline,
(fp->uf_flags & FC_DICT) ? selfdict : NULL);
if (--fp->uf_calls <= 0 && fp->uf_refcount <= 0)

func_clear_free(fp, FALSE);
if (did_save_redo)
restoreRedobuff(&save_redo);
restore_search_patterns();
error = FCERR_NONE;
}
return error;
}









static int
func_name_refcount(char_u *name)
{
return isdigit(*name) || *name == '<';
}

static funccal_entry_T *funccal_stack = NULL;





void
save_funccal(funccal_entry_T *entry)
{
entry->top_funccal = current_funccal;
entry->next = funccal_stack;
funccal_stack = entry;
current_funccal = NULL;
}

void
restore_funccal(void)
{
if (funccal_stack == NULL)
iemsg("INTERNAL: restore_funccal()");
else
{
current_funccal = funccal_stack->top_funccal;
funccal_stack = funccal_stack->next;
}
}

funccall_T *
get_current_funccal(void)
{
return current_funccal;
}

#if defined(EXITFREE) || defined(PROTO)
void
free_all_functions(void)
{
hashitem_T *hi;
ufunc_T *fp;
long_u skipped = 0;
long_u todo = 1;
long_u used;


while (current_funccal != NULL)
{
clear_tv(current_funccal->rettv);
cleanup_function_call(current_funccal);
if (current_funccal == NULL && funccal_stack != NULL)
restore_funccal();
}




while (todo > 0)
{
todo = func_hashtab.ht_used;
for (hi = func_hashtab.ht_array; todo > 0; ++hi)
if (!HASHITEM_EMPTY(hi))
{

fp = HI2UF(hi);
fp->uf_flags &= ~FC_DEAD;
fp->uf_dfunc_idx = -1;



if (func_name_refcount(fp->uf_name))
++skipped;
else
{
used = func_hashtab.ht_used;
func_clear(fp, TRUE);
if (used != func_hashtab.ht_used)
{
skipped = 0;
break;
}
}
--todo;
}
}



skipped = 0;
while (func_hashtab.ht_used > skipped)
{
todo = func_hashtab.ht_used;
for (hi = func_hashtab.ht_array; todo > 0; ++hi)
if (!HASHITEM_EMPTY(hi))
{
--todo;


fp = HI2UF(hi);
if (func_name_refcount(fp->uf_name))
++skipped;
else
{
func_free(fp);
skipped = 0;
break;
}
}
}
if (skipped == 0)
hash_clear(&func_hashtab);

free_def_functions();
}
#endif






int
builtin_function(char_u *name, int len)
{
char_u *p;

if (!ASCII_ISLOWER(name[0]))
return FALSE;
p = vim_strchr(name, AUTOLOAD_CHAR);
return p == NULL || (len > 0 && p > name + len);
}

int
func_call(
char_u *name,
typval_T *args,
partial_T *partial,
dict_T *selfdict,
typval_T *rettv)
{
list_T *l = args->vval.v_list;
listitem_T *item;
typval_T argv[MAX_FUNC_ARGS + 1];
int argc = 0;
int r = 0;

range_list_materialize(l);
FOR_ALL_LIST_ITEMS(args->vval.v_list, item)
{
if (argc == MAX_FUNC_ARGS - (partial == NULL ? 0 : partial->pt_argc))
{
emsg(_("E699: Too many arguments"));
break;
}


copy_tv(&item->li_tv, &argv[argc++]);
}

if (item == NULL)
{
funcexe_T funcexe;

vim_memset(&funcexe, 0, sizeof(funcexe));
funcexe.firstline = curwin->w_cursor.lnum;
funcexe.lastline = curwin->w_cursor.lnum;
funcexe.evaluate = TRUE;
funcexe.partial = partial;
funcexe.selfdict = selfdict;
r = call_func(name, -1, rettv, argc, argv, &funcexe);
}


while (argc > 0)
clear_tv(&argv[--argc]);

return r;
}

static int callback_depth = 0;

int
get_callback_depth(void)
{
return callback_depth;
}




int
call_callback(
callback_T *callback,
int len, 
typval_T *rettv, 
int argcount, 
typval_T *argvars) 

{
funcexe_T funcexe;
int ret;

vim_memset(&funcexe, 0, sizeof(funcexe));
funcexe.evaluate = TRUE;
funcexe.partial = callback->cb_partial;
++callback_depth;
ret = call_func(callback->cb_name, len, rettv, argcount, argvars, &funcexe);
--callback_depth;
return ret;
}





void
user_func_error(int error, char_u *name)
{
switch (error)
{
case FCERR_UNKNOWN:
emsg_funcname(e_unknownfunc, name);
break;
case FCERR_NOTMETHOD:
emsg_funcname(
N_("E276: Cannot use function as a method: %s"), name);
break;
case FCERR_DELETED:
emsg_funcname(N_(e_func_deleted), name);
break;
case FCERR_TOOMANY:
emsg_funcname((char *)e_toomanyarg, name);
break;
case FCERR_TOOFEW:
emsg_funcname((char *)e_toofewarg, name);
break;
case FCERR_SCRIPT:
emsg_funcname(
N_("E120: Using <SID> not in a script context: %s"), name);
break;
case FCERR_DICT:
emsg_funcname(
N_("E725: Calling dict function without Dictionary: %s"),
name);
break;
}
}







int
call_func(
char_u *funcname, 
int len, 
typval_T *rettv, 
int argcount_in, 
typval_T *argvars_in, 

funcexe_T *funcexe) 
{
int ret = FAIL;
int error = FCERR_NONE;
int i;
ufunc_T *fp = NULL;
char_u fname_buf[FLEN_FIXED + 1];
char_u *tofree = NULL;
char_u *fname = NULL;
char_u *name = NULL;
int argcount = argcount_in;
typval_T *argvars = argvars_in;
dict_T *selfdict = funcexe->selfdict;
typval_T argv[MAX_FUNC_ARGS + 1]; 

int argv_clear = 0;
int argv_base = 0;
partial_T *partial = funcexe->partial;



rettv->v_type = VAR_UNKNOWN;

if (partial != NULL)
fp = partial->pt_func;
if (fp == NULL)
{


name = len > 0 ? vim_strnsave(funcname, len) : vim_strsave(funcname);
if (name == NULL)
return ret;

fname = fname_trans_sid(name, fname_buf, &tofree, &error);
}

if (funcexe->doesrange != NULL)
*funcexe->doesrange = FALSE;

if (partial != NULL)
{



if (partial->pt_dict != NULL && (selfdict == NULL || !partial->pt_auto))
selfdict = partial->pt_dict;
if (error == FCERR_NONE && partial->pt_argc > 0)
{
for (argv_clear = 0; argv_clear < partial->pt_argc; ++argv_clear)
{
if (argv_clear + argcount_in >= MAX_FUNC_ARGS)
{
error = FCERR_TOOMANY;
goto theend;
}
copy_tv(&partial->pt_argv[argv_clear], &argv[argv_clear]);
}
for (i = 0; i < argcount_in; ++i)
argv[i + argv_clear] = argvars_in[i];
argvars = argv;
argcount = partial->pt_argc + argcount_in;
}
}

if (error == FCERR_NONE && funcexe->evaluate)
{
char_u *rfname = fname;


if (fp == NULL && fname[0] == 'g' && fname[1] == ':')
rfname = fname + 2;

rettv->v_type = VAR_NUMBER; 
rettv->vval.v_number = 0;
error = FCERR_UNKNOWN;

if (fp != NULL || !builtin_function(rfname, -1))
{



if (fp == NULL)
fp = find_func(rfname, NULL);


if (fp == NULL
&& apply_autocmds(EVENT_FUNCUNDEFINED,
rfname, rfname, TRUE, NULL)
&& !aborting())
{

fp = find_func(rfname, NULL);
}

if (fp == NULL && script_autoload(rfname, TRUE) && !aborting())
{

fp = find_func(rfname, NULL);
}

if (fp != NULL && (fp->uf_flags & FC_DELETED))
error = FCERR_DELETED;
else if (fp != NULL)
{
if (funcexe->argv_func != NULL)

argcount = funcexe->argv_func(argcount, argvars, argv_clear,
fp->uf_args.ga_len);

if (funcexe->basetv != NULL)
{

mch_memmove(&argv[1], argvars, sizeof(typval_T) * argcount);
argv[0] = *funcexe->basetv;
argcount++;
argvars = argv;
argv_base = 1;
}

error = call_user_func_check(fp, argcount, argvars, rettv,
funcexe, selfdict);
}
}
else if (funcexe->basetv != NULL)
{




error = call_internal_method(fname, argcount, argvars, rettv,
funcexe->basetv);
}
else
{



error = call_internal_func(fname, argcount, argvars, rettv);
}










update_force_abort();
}
if (error == FCERR_NONE)
ret = OK;

theend:




if (!aborting())
{
user_func_error(error, (name != NULL) ? name : funcname);
}


while (argv_clear > 0)
clear_tv(&argv[--argv_clear + argv_base]);

vim_free(tofree);
vim_free(name);

return ret;
}

static char_u *
printable_func_name(ufunc_T *fp)
{
return fp->uf_name_exp != NULL ? fp->uf_name_exp : fp->uf_name;
}




static void
list_func_head(ufunc_T *fp, int indent)
{
int j;

msg_start();
if (indent)
msg_puts(" ");
if (fp->uf_dfunc_idx >= 0)
msg_puts("def ");
else
msg_puts("function ");
msg_puts((char *)printable_func_name(fp));
msg_putchar('(');
for (j = 0; j < fp->uf_args.ga_len; ++j)
{
if (j)
msg_puts(", ");
msg_puts((char *)FUNCARG(fp, j));
if (fp->uf_arg_types != NULL)
{
char *tofree;

msg_puts(": ");
msg_puts(type_name(fp->uf_arg_types[j], &tofree));
vim_free(tofree);
}
if (j >= fp->uf_args.ga_len - fp->uf_def_args.ga_len)
{
msg_puts(" = ");
msg_puts(((char **)(fp->uf_def_args.ga_data))
[j - fp->uf_args.ga_len + fp->uf_def_args.ga_len]);
}
}
if (fp->uf_varargs)
{
if (j)
msg_puts(", ");
msg_puts("...");
}
if (fp->uf_va_name != NULL)
{
if (j)
msg_puts(", ");
msg_puts("...");
msg_puts((char *)fp->uf_va_name);
if (fp->uf_va_type)
{
char *tofree;

msg_puts(": ");
msg_puts(type_name(fp->uf_va_type, &tofree));
vim_free(tofree);
}
}
msg_putchar(')');

if (fp->uf_dfunc_idx >= 0)
{
if (fp->uf_ret_type != &t_void)
{
char *tofree;

msg_puts(": ");
msg_puts(type_name(fp->uf_ret_type, &tofree));
vim_free(tofree);
}
}
else if (fp->uf_flags & FC_ABORT)
msg_puts(" abort");
if (fp->uf_flags & FC_RANGE)
msg_puts(" range");
if (fp->uf_flags & FC_DICT)
msg_puts(" dict");
if (fp->uf_flags & FC_CLOSURE)
msg_puts(" closure");
msg_clr_eos();
if (p_verbose > 0)
last_set_msg(fp->uf_script_ctx);
}












char_u *
trans_function_name(
char_u **pp,
int skip, 
int flags,
funcdict_T *fdp, 
partial_T **partial) 
{
char_u *name = NULL;
char_u *start;
char_u *end;
int lead;
char_u sid_buf[20];
int len;
int extra = 0;
lval_T lv;
int vim9script;

if (fdp != NULL)
vim_memset(fdp, 0, sizeof(funcdict_T));
start = *pp;



if ((*pp)[0] == K_SPECIAL && (*pp)[1] == KS_EXTRA
&& (*pp)[2] == (int)KE_SNR)
{
*pp += 3;
len = get_id_len(pp) + 3;
return vim_strnsave(start, len);
}



lead = eval_fname_script(start);
if (lead > 2)
start += lead;


end = get_lval(start, NULL, &lv, FALSE, skip, flags | GLV_READ_ONLY,
lead > 2 ? 0 : FNE_CHECK_START);
if (end == start)
{
if (!skip)
emsg(_("E129: Function name required"));
goto theend;
}
if (end == NULL || (lv.ll_tv != NULL && (lead > 2 || lv.ll_range)))
{





if (!aborting())
{
if (end != NULL)
semsg(_(e_invarg2), start);
}
else
*pp = find_name_end(start, NULL, NULL, FNE_INCL_BR);
goto theend;
}

if (lv.ll_tv != NULL)
{
if (fdp != NULL)
{
fdp->fd_dict = lv.ll_dict;
fdp->fd_newkey = lv.ll_newkey;
lv.ll_newkey = NULL;
fdp->fd_di = lv.ll_di;
}
if (lv.ll_tv->v_type == VAR_FUNC && lv.ll_tv->vval.v_string != NULL)
{
name = vim_strsave(lv.ll_tv->vval.v_string);
*pp = end;
}
else if (lv.ll_tv->v_type == VAR_PARTIAL
&& lv.ll_tv->vval.v_partial != NULL)
{
name = vim_strsave(partial_name(lv.ll_tv->vval.v_partial));
*pp = end;
if (partial != NULL)
*partial = lv.ll_tv->vval.v_partial;
}
else
{
if (!skip && !(flags & TFN_QUIET) && (fdp == NULL
|| lv.ll_dict == NULL || fdp->fd_newkey == NULL))
emsg(_(e_funcref));
else
*pp = end;
name = NULL;
}
goto theend;
}

if (lv.ll_name == NULL)
{

*pp = end;
goto theend;
}


if (lv.ll_exp_name != NULL)
{
len = (int)STRLEN(lv.ll_exp_name);
name = deref_func_name(lv.ll_exp_name, &len, partial,
flags & TFN_NO_AUTOLOAD);
if (name == lv.ll_exp_name)
name = NULL;
}
else if (!(flags & TFN_NO_DEREF))
{
len = (int)(end - *pp);
name = deref_func_name(*pp, &len, partial, flags & TFN_NO_AUTOLOAD);
if (name == *pp)
name = NULL;
}
if (name != NULL)
{
name = vim_strsave(name);
*pp = end;
if (STRNCMP(name, "<SNR>", 5) == 0)
{

name[0] = K_SPECIAL;
name[1] = KS_EXTRA;
name[2] = (int)KE_SNR;
mch_memmove(name + 3, name + 5, STRLEN(name + 5) + 1);
}
goto theend;
}

if (lv.ll_exp_name != NULL)
{
len = (int)STRLEN(lv.ll_exp_name);
if (lead <= 2 && lv.ll_name == lv.ll_exp_name
&& STRNCMP(lv.ll_name, "s:", 2) == 0)
{


lv.ll_name += 2;
len -= 2;
lead = 2;
}
}
else
{

if (lead == 2 || (lv.ll_name[0] == 'g' && lv.ll_name[1] == ':'))
lv.ll_name += 2;
len = (int)(end - lv.ll_name);
}


vim9script = ASCII_ISUPPER(*start)
&& current_sctx.sc_version == SCRIPT_VERSION_VIM9;






if (skip)
lead = 0; 
else if (lead > 0 || vim9script)
{
if (!vim9script)
lead = 3;
if (vim9script || (lv.ll_exp_name != NULL
&& eval_fname_sid(lv.ll_exp_name))
|| eval_fname_sid(*pp))
{

if (current_sctx.sc_sid <= 0)
{
emsg(_(e_usingsid));
goto theend;
}
sprintf((char *)sid_buf, "%ld_", (long)current_sctx.sc_sid);
if (vim9script)
extra = 3 + (int)STRLEN(sid_buf);
else
lead += (int)STRLEN(sid_buf);
}
}
else if (!(flags & TFN_INT) && builtin_function(lv.ll_name, len))
{
semsg(_("E128: Function name must start with a capital or \"s:\": %s"),
start);
goto theend;
}
if (!skip && !(flags & TFN_QUIET) && !(flags & TFN_NO_DEREF))
{
char_u *cp = vim_strchr(lv.ll_name, ':');

if (cp != NULL && cp < end)
{
semsg(_("E884: Function name cannot contain a colon: %s"), start);
goto theend;
}
}

name = alloc(len + lead + extra + 1);
if (name != NULL)
{
if (!skip && (lead > 0 || vim9script))
{
name[0] = K_SPECIAL;
name[1] = KS_EXTRA;
name[2] = (int)KE_SNR;
if (vim9script || lead > 3) 
STRCPY(name + 3, sid_buf);
}
mch_memmove(name + lead + extra, lv.ll_name, (size_t)len);
name[lead + extra + len] = NUL;
}
*pp = end;

theend:
clear_lval(&lv);
return name;
}




void
ex_function(exarg_T *eap)
{
char_u *theline;
char_u *line_to_free = NULL;
int j;
int c;
int saved_did_emsg;
int saved_wait_return = need_wait_return;
char_u *name = NULL;
char_u *p;
char_u *arg;
char_u *line_arg = NULL;
garray_T newargs;
garray_T argtypes;
garray_T default_args;
garray_T newlines;
int varargs = FALSE;
int flags = 0;
char_u *ret_type = NULL;
ufunc_T *fp;
int overwrite = FALSE;
int indent;
int nesting;
#define MAX_FUNC_NESTING 50
char nesting_def[MAX_FUNC_NESTING];
dictitem_T *v;
funcdict_T fudi;
static int func_nr = 0; 
int paren;
int todo;
hashitem_T *hi;
int do_concat = TRUE;
linenr_T sourcing_lnum_off;
linenr_T sourcing_lnum_top;
int is_heredoc = FALSE;
char_u *skip_until = NULL;
char_u *heredoc_trimmed = NULL;




if (ends_excmd(*eap->arg))
{
if (!eap->skip)
{
todo = (int)func_hashtab.ht_used;
for (hi = func_hashtab.ht_array; todo > 0 && !got_int; ++hi)
{
if (!HASHITEM_EMPTY(hi))
{
--todo;
fp = HI2UF(hi);
if ((fp->uf_flags & FC_DEAD)
|| message_filtered(fp->uf_name))
continue;
if (!func_name_refcount(fp->uf_name))
list_func_head(fp, FALSE);
}
}
}
eap->nextcmd = check_nextcmd(eap->arg);
return;
}




if (*eap->arg == '/')
{
p = skip_regexp(eap->arg + 1, '/', TRUE);
if (!eap->skip)
{
regmatch_T regmatch;

c = *p;
*p = NUL;
regmatch.regprog = vim_regcomp(eap->arg + 1, RE_MAGIC);
*p = c;
if (regmatch.regprog != NULL)
{
regmatch.rm_ic = p_ic;

todo = (int)func_hashtab.ht_used;
for (hi = func_hashtab.ht_array; todo > 0 && !got_int; ++hi)
{
if (!HASHITEM_EMPTY(hi))
{
--todo;
fp = HI2UF(hi);
if ((fp->uf_flags & FC_DEAD) == 0
&& !isdigit(*fp->uf_name)
&& vim_regexec(&regmatch, fp->uf_name, 0))
list_func_head(fp, FALSE);
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

ga_init(&newargs);
ga_init(&argtypes);
ga_init(&default_args);

















p = eap->arg;
name = trans_function_name(&p, eap->skip, TFN_NO_AUTOLOAD, &fudi, NULL);
paren = (vim_strchr(p, '(') != NULL);
if (name == NULL && (fudi.fd_dict == NULL || !paren) && !eap->skip)
{





if (!aborting())
{
if (!eap->skip && fudi.fd_newkey != NULL)
semsg(_(e_dictkey), fudi.fd_newkey);
vim_free(fudi.fd_newkey);
return;
}
else
eap->skip = TRUE;
}



saved_did_emsg = did_emsg;
did_emsg = FALSE;




if (!paren)
{
if (!ends_excmd(*skipwhite(p)))
{
emsg(_(e_trailing));
goto ret_free;
}
eap->nextcmd = check_nextcmd(p);
if (eap->nextcmd != NULL)
*p = NUL;
if (!eap->skip && !got_int)
{
fp = find_func(name, NULL);
if (fp != NULL)
{
list_func_head(fp, TRUE);
for (j = 0; j < fp->uf_lines.ga_len && !got_int; ++j)
{
if (FUNCLINE(fp, j) == NULL)
continue;
msg_putchar('\n');
msg_outnum((long)(j + 1));
if (j < 9)
msg_putchar(' ');
if (j < 99)
msg_putchar(' ');
msg_prt_line(FUNCLINE(fp, j), FALSE);
out_flush(); 
ui_breakcheck();
}
if (!got_int)
{
msg_putchar('\n');
if (fp->uf_dfunc_idx >= 0)
msg_puts(" enddef");
else
msg_puts(" endfunction");
}
}
else
emsg_funcname(N_("E123: Undefined function: %s"), name);
}
goto ret_free;
}




p = skipwhite(p);
if (*p != '(')
{
if (!eap->skip)
{
semsg(_("E124: Missing '(': %s"), eap->arg);
goto ret_free;
}

if (vim_strchr(p, '(') != NULL)
p = vim_strchr(p, '(');
}
p = skipwhite(p + 1);

ga_init2(&newlines, (int)sizeof(char_u *), 3);

if (!eap->skip)
{


if (name != NULL)
arg = name;
else
arg = fudi.fd_newkey;
if (arg != NULL && (fudi.fd_di == NULL
|| (fudi.fd_di->di_tv.v_type != VAR_FUNC
&& fudi.fd_di->di_tv.v_type != VAR_PARTIAL)))
{
if (*arg == K_SPECIAL)
j = 3;
else
j = 0;
while (arg[j] != NUL && (j == 0 ? eval_isnamec1(arg[j])
: eval_isnamec(arg[j])))
++j;
if (arg[j] != NUL)
emsg_funcname((char *)e_invarg2, arg);
}

if (fudi.fd_dict != NULL && fudi.fd_dict->dv_scope == VAR_DEF_SCOPE)
emsg(_("E862: Cannot use g: here"));
}

if (get_function_args(&p, ')', &newargs,
eap->cmdidx == CMD_def ? &argtypes : NULL,
&varargs, &default_args, eap->skip) == FAIL)
goto errret_2;

if (eap->cmdidx == CMD_def)
{

if (*p == ':')
{
ret_type = skipwhite(p + 1);
p = skip_type(ret_type);
if (p > ret_type)
p = skipwhite(p);
else
semsg(_("E1056: expected a type: %s"), ret_type);
}
}
else

for (;;)
{
p = skipwhite(p);
if (STRNCMP(p, "range", 5) == 0)
{
flags |= FC_RANGE;
p += 5;
}
else if (STRNCMP(p, "dict", 4) == 0)
{
flags |= FC_DICT;
p += 4;
}
else if (STRNCMP(p, "abort", 5) == 0)
{
flags |= FC_ABORT;
p += 5;
}
else if (STRNCMP(p, "closure", 7) == 0)
{
flags |= FC_CLOSURE;
p += 7;
if (current_funccal == NULL)
{
emsg_funcname(N_("E932: Closure function should not be at top level: %s"),
name == NULL ? (char_u *)"" : name);
goto erret;
}
}
else
break;
}



if (*p == '\n')
line_arg = p + 1;
else if (*p != NUL && *p != '"' && !eap->skip && !did_emsg)
emsg(_(e_trailing));





if (KeyTyped)
{



if (!eap->skip && !eap->forceit)
{
if (fudi.fd_dict != NULL && fudi.fd_newkey == NULL)
emsg(_(e_funcdict));
else if (name != NULL && find_func(name, NULL) != NULL)
emsg_funcname(e_funcexts, name);
}

if (!eap->skip && did_emsg)
goto erret;

msg_putchar('\n'); 
cmdline_row = msg_row;
}


sourcing_lnum_top = SOURCING_LNUM;

indent = 2;
nesting = 0;
nesting_def[nesting] = (eap->cmdidx == CMD_def);
for (;;)
{
if (KeyTyped)
{
msg_scroll = TRUE;
saved_wait_return = FALSE;
}
need_wait_return = FALSE;

if (line_arg != NULL)
{

theline = line_arg;
p = vim_strchr(theline, '\n');
if (p == NULL)
line_arg += STRLEN(line_arg);
else
{
*p = NUL;
line_arg = p + 1;
}
}
else
{
vim_free(line_to_free);
if (eap->getline == NULL)
theline = getcmdline(':', 0L, indent, do_concat);
else
theline = eap->getline(':', eap->cookie, indent, do_concat);
line_to_free = theline;
}
if (KeyTyped)
lines_left = Rows - 1;
if (theline == NULL)
{
if (eap->cmdidx == CMD_def)
emsg(_("E1057: Missing :enddef"));
else
emsg(_("E126: Missing :endfunction"));
goto erret;
}


sourcing_lnum_off = get_sourced_lnum(eap->getline, eap->cookie);
if (SOURCING_LNUM < sourcing_lnum_off)
sourcing_lnum_off -= SOURCING_LNUM;
else
sourcing_lnum_off = 0;

if (skip_until != NULL)
{




if (heredoc_trimmed == NULL
|| (is_heredoc && skipwhite(theline) == theline)
|| STRNCMP(theline, heredoc_trimmed,
STRLEN(heredoc_trimmed)) == 0)
{
if (heredoc_trimmed == NULL)
p = theline;
else if (is_heredoc)
p = skipwhite(theline) == theline
? theline : theline + STRLEN(heredoc_trimmed);
else
p = theline + STRLEN(heredoc_trimmed);
if (STRCMP(p, skip_until) == 0)
{
VIM_CLEAR(skip_until);
VIM_CLEAR(heredoc_trimmed);
do_concat = TRUE;
is_heredoc = FALSE;
}
}
}
else
{

for (p = theline; VIM_ISWHITE(*p) || *p == ':'; ++p)
;


if (checkforcmd(&p, nesting_def[nesting]
? "enddef" : "endfunction", 4) && nesting-- == 0)
{
char_u *nextcmd = NULL;

if (*p == '|')
nextcmd = p + 1;
else if (line_arg != NULL && *skipwhite(line_arg) != NUL)
nextcmd = line_arg;
else if (*p != NUL && *p != '"' && p_verbose > 0)
give_warning2(eap->cmdidx == CMD_def
? (char_u *)_("W1001: Text found after :enddef: %s")
: (char_u *)_("W22: Text found after :endfunction: %s"),
p, TRUE);
if (nextcmd != NULL)
{



eap->nextcmd = nextcmd;
if (line_to_free != NULL)
{
vim_free(*eap->cmdlinep);
*eap->cmdlinep = line_to_free;
line_to_free = NULL;
}
}
break;
}



if (indent > 2 && (*p == '}' || STRNCMP(p, "end", 3) == 0))
indent -= 2;
else if (STRNCMP(p, "if", 2) == 0
|| STRNCMP(p, "wh", 2) == 0
|| STRNCMP(p, "for", 3) == 0
|| STRNCMP(p, "try", 3) == 0)
indent += 2;




c = *p;
if (checkforcmd(&p, "function", 2)
|| (eap->cmdidx == CMD_def && checkforcmd(&p, "def", 3)))
{
if (*p == '!')
p = skipwhite(p + 1);
p += eval_fname_script(p);
vim_free(trans_function_name(&p, TRUE, 0, NULL, NULL));
if (*skipwhite(p) == '(')
{
if (nesting == MAX_FUNC_NESTING - 1)
emsg(_("E1058: function nesting too deep"));
else
{
++nesting;
nesting_def[nesting] = (c == 'd');
indent += 2;
}
}
}


p = skip_range(p, NULL);
if (eap->cmdidx != CMD_def
&& ((p[0] == 'a' && (!ASCII_ISALPHA(p[1]) || p[1] == 'p'))
|| (p[0] == 'c'
&& (!ASCII_ISALPHA(p[1]) || (p[1] == 'h'
&& (!ASCII_ISALPHA(p[2]) || (p[2] == 'a'
&& (STRNCMP(&p[3], "nge", 3) != 0
|| !ASCII_ISALPHA(p[6])))))))
|| (p[0] == 'i'
&& (!ASCII_ISALPHA(p[1]) || (p[1] == 'n'
&& (!ASCII_ISALPHA(p[2])
|| (p[2] == 's'
&& (!ASCII_ISALPHA(p[3])
|| p[3] == 'e'))))))))
skip_until = vim_strsave((char_u *)".");


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
))
{

p = skipwhite(arg + 2);
if (*p == NUL)
skip_until = vim_strsave((char_u *)".");
else
skip_until = vim_strsave(p);
}



arg = skipwhite(skiptowhite(p));
if (*arg == '[')
arg = vim_strchr(arg, ']');
if (arg != NULL)
{
arg = skipwhite(skiptowhite(arg));
if ( arg[0] == '=' && arg[1] == '<' && arg[2] =='<'
&& ((p[0] == 'l'
&& p[1] == 'e'
&& (!ASCII_ISALNUM(p[2])
|| (p[2] == 't' && !ASCII_ISALNUM(p[3]))))))
{
p = skipwhite(arg + 3);
if (STRNCMP(p, "trim", 4) == 0)
{

p = skipwhite(p + 4);
heredoc_trimmed = vim_strnsave(theline,
(int)(skipwhite(theline) - theline));
}
skip_until = vim_strnsave(p, (int)(skiptowhite(p) - p));
do_concat = FALSE;
is_heredoc = TRUE;
}
}
}


if (ga_grow(&newlines, 1 + sourcing_lnum_off) == FAIL)
goto erret;




p = vim_strsave(theline);
if (p == NULL)
goto erret;
((char_u **)(newlines.ga_data))[newlines.ga_len++] = p;



while (sourcing_lnum_off-- > 0)
((char_u **)(newlines.ga_data))[newlines.ga_len++] = NULL;


if (line_arg != NULL && *line_arg == NUL)
line_arg = NULL;
}



if (eap->skip || did_emsg)
goto erret;




if (fudi.fd_dict == NULL)
{
hashtab_T *ht;

v = find_var(name, &ht, FALSE);
if (v != NULL && v->di_tv.v_type == VAR_FUNC)
{
emsg_funcname(N_("E707: Function name conflicts with variable: %s"),
name);
goto erret;
}

fp = find_func_even_dead(name, NULL);
if (fp != NULL)
{
int dead = fp->uf_flags & FC_DEAD;



if (!dead && !eap->forceit
&& (fp->uf_script_ctx.sc_sid != current_sctx.sc_sid
|| fp->uf_script_ctx.sc_seq == current_sctx.sc_seq))
{
emsg_funcname(e_funcexts, name);
goto erret;
}
if (fp->uf_calls > 0)
{
emsg_funcname(
N_("E127: Cannot redefine function %s: It is in use"),
name);
goto erret;
}
if (fp->uf_refcount > 1)
{


--fp->uf_refcount;
fp->uf_flags |= FC_REMOVED;
fp = NULL;
overwrite = TRUE;
}
else
{
char_u *exp_name = fp->uf_name_exp;


VIM_CLEAR(name);
fp->uf_name_exp = NULL;
func_clear_items(fp);
fp->uf_name_exp = exp_name;
fp->uf_flags &= ~FC_DEAD;
#if defined(FEAT_PROFILE)
fp->uf_profiling = FALSE;
fp->uf_prof_initialized = FALSE;
#endif
}
}
}
else
{
char numbuf[20];

fp = NULL;
if (fudi.fd_newkey == NULL && !eap->forceit)
{
emsg(_(e_funcdict));
goto erret;
}
if (fudi.fd_di == NULL)
{

if (var_check_lock(fudi.fd_dict->dv_lock, eap->arg, FALSE))
goto erret;
}

else if (var_check_lock(fudi.fd_di->di_tv.v_lock, eap->arg, FALSE))
goto erret;



vim_free(name);
sprintf(numbuf, "%d", ++func_nr);
name = vim_strsave((char_u *)numbuf);
if (name == NULL)
goto erret;
}

if (fp == NULL)
{
if (fudi.fd_dict == NULL && vim_strchr(name, AUTOLOAD_CHAR) != NULL)
{
int slen, plen;
char_u *scriptname;


j = FAIL;
if (SOURCING_NAME != NULL)
{
scriptname = autoload_name(name);
if (scriptname != NULL)
{
p = vim_strchr(scriptname, '/');
plen = (int)STRLEN(p);
slen = (int)STRLEN(SOURCING_NAME);
if (slen > plen && fnamecmp(p,
SOURCING_NAME + slen - plen) == 0)
j = OK;
vim_free(scriptname);
}
}
if (j == FAIL)
{
semsg(_("E746: Function name does not match script file name: %s"), name);
goto erret;
}
}

fp = alloc_clear(offsetof(ufunc_T, uf_name) + STRLEN(name) + 1);
if (fp == NULL)
goto erret;
fp->uf_dfunc_idx = -1;

if (fudi.fd_dict != NULL)
{
if (fudi.fd_di == NULL)
{

fudi.fd_di = dictitem_alloc(fudi.fd_newkey);
if (fudi.fd_di == NULL)
{
vim_free(fp);
goto erret;
}
if (dict_add(fudi.fd_dict, fudi.fd_di) == FAIL)
{
vim_free(fudi.fd_di);
vim_free(fp);
goto erret;
}
}
else

clear_tv(&fudi.fd_di->di_tv);
fudi.fd_di->di_tv.v_type = VAR_FUNC;
fudi.fd_di->di_tv.vval.v_string = vim_strsave(name);


flags |= FC_DICT;
}


set_ufunc_name(fp, name);
if (overwrite)
{
hi = hash_find(&func_hashtab, name);
hi->hi_key = UF2HIKEY(fp);
}
else if (hash_add(&func_hashtab, UF2HIKEY(fp)) == FAIL)
{
vim_free(fp);
goto erret;
}
fp->uf_refcount = 1;
}
fp->uf_args = newargs;
fp->uf_def_args = default_args;
fp->uf_ret_type = &t_any;

if (eap->cmdidx == CMD_def)
{
int lnum_save = SOURCING_LNUM;


SOURCING_LNUM = sourcing_lnum_top;


ga_init2(&fp->uf_type_list, sizeof(type_T), 5);

if (argtypes.ga_len > 0)
{


int len = argtypes.ga_len - (varargs ? 1 : 0);

fp->uf_arg_types = ALLOC_CLEAR_MULT(type_T *, len);
if (fp->uf_arg_types != NULL)
{
int i;
type_T *type;

for (i = 0; i < len; ++ i)
{
p = ((char_u **)argtypes.ga_data)[i];
if (p == NULL)

type = &t_any;
else
type = parse_type(&p, &fp->uf_type_list);
if (type == NULL)
{
SOURCING_LNUM = lnum_save;
goto errret_2;
}
fp->uf_arg_types[i] = type;
}
}
if (varargs)
{


fp->uf_va_name = ((char_u **)fp->uf_args.ga_data)
[fp->uf_args.ga_len - 1];
--fp->uf_args.ga_len;
p = ((char_u **)argtypes.ga_data)[len];
if (p == NULL)

fp->uf_va_type = &t_any;
else
fp->uf_va_type = parse_type(&p, &fp->uf_type_list);
if (fp->uf_va_type == NULL)
{
SOURCING_LNUM = lnum_save;
goto errret_2;
}
}
varargs = FALSE;
}


if (ret_type == NULL)
fp->uf_ret_type = &t_void;
else
{
p = ret_type;
fp->uf_ret_type = parse_type(&p, &fp->uf_type_list);
}
}

fp->uf_lines = newlines;
if ((flags & FC_CLOSURE) != 0)
{
if (register_closure(fp) == FAIL)
goto erret;
}
else
fp->uf_scoped = NULL;

#if defined(FEAT_PROFILE)
if (prof_def_func())
func_do_profile(fp);
#endif
fp->uf_varargs = varargs;
if (sandbox)
flags |= FC_SANDBOX;
fp->uf_flags = flags;
fp->uf_calls = 0;
fp->uf_cleared = FALSE;
fp->uf_script_ctx = current_sctx;
fp->uf_script_ctx.sc_lnum += sourcing_lnum_top;
if (is_export)
{
fp->uf_flags |= FC_EXPORT;

is_export = FALSE;
}


if (eap->cmdidx == CMD_def)
compile_def_function(fp, FALSE);

goto ret_free;

erret:
ga_clear_strings(&newargs);
ga_clear_strings(&default_args);
errret_2:
ga_clear_strings(&newlines);
ret_free:
ga_clear_strings(&argtypes);
vim_free(skip_until);
vim_free(line_to_free);
vim_free(fudi.fd_newkey);
vim_free(name);
did_emsg |= saved_did_emsg;
need_wait_return |= saved_wait_return;
}






int
eval_fname_script(char_u *p)
{


if (p[0] == '<' && (MB_STRNICMP(p + 1, "SID>", 4) == 0
|| MB_STRNICMP(p + 1, "SNR>", 4) == 0))
return 5;
if (p[0] == 's' && p[1] == ':')
return 2;
return 0;
}

int
translated_function_exists(char_u *name)
{
if (builtin_function(name, -1))
return has_internal_func(name);
return find_func(name, NULL) != NULL;
}





int
has_varargs(ufunc_T *ufunc)
{
return ufunc->uf_varargs || ufunc->uf_va_name != NULL;
}





int
function_exists(char_u *name, int no_deref)
{
char_u *nm = name;
char_u *p;
int n = FALSE;
int flag;

flag = TFN_INT | TFN_QUIET | TFN_NO_AUTOLOAD;
if (no_deref)
flag |= TFN_NO_DEREF;
p = trans_function_name(&nm, FALSE, flag, NULL, NULL);
nm = skipwhite(nm);



if (p != NULL && (*nm == NUL || *nm == '('))
n = translated_function_exists(p);
vim_free(p);
return n;
}

#if defined(FEAT_PYTHON) || defined(FEAT_PYTHON3) || defined(PROTO)
char_u *
get_expanded_name(char_u *name, int check)
{
char_u *nm = name;
char_u *p;

p = trans_function_name(&nm, FALSE, TFN_INT|TFN_QUIET, NULL, NULL);

if (p != NULL && *nm == NUL)
if (!check || translated_function_exists(p))
return p;

vim_free(p);
return NULL;
}
#endif





char_u *
get_user_func_name(expand_T *xp, int idx)
{
static long_u done;
static hashitem_T *hi;
ufunc_T *fp;

if (idx == 0)
{
done = 0;
hi = func_hashtab.ht_array;
}
if (done < func_hashtab.ht_used)
{
if (done++ > 0)
++hi;
while (HASHITEM_EMPTY(hi))
++hi;
fp = HI2UF(hi);


if ((fp->uf_flags & FC_DEAD) || (fp->uf_flags & FC_DICT)
|| STRNCMP(fp->uf_name, "<lambda>", 8) == 0)
return (char_u *)"";

if (STRLEN(fp->uf_name) + 4 >= IOSIZE)
return fp->uf_name; 

cat_func_name(IObuff, fp);
if (xp->xp_context != EXPAND_USER_FUNC)
{
STRCAT(IObuff, "(");
if (!has_varargs(fp) && fp->uf_args.ga_len == 0)
STRCAT(IObuff, ")");
}
return IObuff;
}
return NULL;
}




void
ex_delfunction(exarg_T *eap)
{
ufunc_T *fp = NULL;
char_u *p;
char_u *name;
funcdict_T fudi;

p = eap->arg;
name = trans_function_name(&p, eap->skip, 0, &fudi, NULL);
vim_free(fudi.fd_newkey);
if (name == NULL)
{
if (fudi.fd_dict != NULL && !eap->skip)
emsg(_(e_funcref));
return;
}
if (!ends_excmd(*skipwhite(p)))
{
vim_free(name);
emsg(_(e_trailing));
return;
}
eap->nextcmd = check_nextcmd(p);
if (eap->nextcmd != NULL)
*p = NUL;

if (!eap->skip)
fp = find_func(name, NULL);
vim_free(name);

if (!eap->skip)
{
if (fp == NULL)
{
if (!eap->forceit)
semsg(_(e_nofunc), eap->arg);
return;
}
if (fp->uf_calls > 0)
{
semsg(_("E131: Cannot delete function %s: It is in use"), eap->arg);
return;
}

if (fudi.fd_dict != NULL)
{


dictitem_remove(fudi.fd_dict, fudi.fd_di);
}
else
{





if (fp->uf_refcount > (func_name_refcount(fp->uf_name) ? 0 : 1))
{


if (func_remove(fp))
fp->uf_refcount--;
fp->uf_flags |= FC_DELETED;
}
else
func_clear_free(fp, FALSE);
}
}
}





void
func_unref(char_u *name)
{
ufunc_T *fp = NULL;

if (name == NULL || !func_name_refcount(name))
return;
fp = find_func(name, NULL);
if (fp == NULL && isdigit(*name))
{
#if defined(EXITFREE)
if (!entered_free_all_mem)
#endif
internal_error("func_unref()");
}
if (fp != NULL && --fp->uf_refcount <= 0)
{


if (fp->uf_calls == 0)
func_clear_free(fp, FALSE);
}
}





void
func_ptr_unref(ufunc_T *fp)
{
if (fp != NULL && --fp->uf_refcount <= 0)
{


if (fp->uf_calls == 0)
func_clear_free(fp, FALSE);
}
}




void
func_ref(char_u *name)
{
ufunc_T *fp;

if (name == NULL || !func_name_refcount(name))
return;
fp = find_func(name, NULL);
if (fp != NULL)
++fp->uf_refcount;
else if (isdigit(*name))


internal_error("func_ref()");
}




void
func_ptr_ref(ufunc_T *fp)
{
if (fp != NULL)
++fp->uf_refcount;
}





static int
can_free_funccal(funccall_T *fc, int copyID)
{
return (fc->l_varlist.lv_copyID != copyID
&& fc->l_vars.dv_copyID != copyID
&& fc->l_avars.dv_copyID != copyID
&& fc->fc_copyID != copyID);
}




void
ex_return(exarg_T *eap)
{
char_u *arg = eap->arg;
typval_T rettv;
int returning = FALSE;

if (current_funccal == NULL)
{
emsg(_("E133: :return not inside a function"));
return;
}

if (eap->skip)
++emsg_skip;

eap->nextcmd = NULL;
if ((*arg != NUL && *arg != '|' && *arg != '\n')
&& eval0(arg, &rettv, &eap->nextcmd, !eap->skip) != FAIL)
{
if (!eap->skip)
returning = do_return(eap, FALSE, TRUE, &rettv);
else
clear_tv(&rettv);
}

else if (!eap->skip)
{

update_force_abort();





if (!aborting())
returning = do_return(eap, FALSE, TRUE, NULL);
}




if (returning)
eap->nextcmd = NULL;
else if (eap->nextcmd == NULL) 
eap->nextcmd = check_nextcmd(arg);

if (eap->skip)
--emsg_skip;
}




void
ex_call(exarg_T *eap)
{
char_u *arg = eap->arg;
char_u *startarg;
char_u *name;
char_u *tofree;
int len;
typval_T rettv;
linenr_T lnum;
int doesrange;
int failed = FALSE;
funcdict_T fudi;
partial_T *partial = NULL;

if (eap->skip)
{



++emsg_skip;
if (eval0(eap->arg, &rettv, &eap->nextcmd, FALSE) != FAIL)
clear_tv(&rettv);
--emsg_skip;
return;
}

tofree = trans_function_name(&arg, eap->skip, TFN_INT, &fudi, &partial);
if (fudi.fd_newkey != NULL)
{

semsg(_(e_dictkey), fudi.fd_newkey);
vim_free(fudi.fd_newkey);
}
if (tofree == NULL)
return;



if (fudi.fd_dict != NULL)
++fudi.fd_dict->dv_refcount;




len = (int)STRLEN(tofree);
name = deref_func_name(tofree, &len,
partial != NULL ? NULL : &partial, FALSE);



startarg = skipwhite(arg);
rettv.v_type = VAR_UNKNOWN; 

if (*startarg != '(')
{
semsg(_(e_missing_paren), eap->arg);
goto end;
}







if (eap->skip)
{
++emsg_skip;
lnum = eap->line2; 
}
else
lnum = eap->line1;
for ( ; lnum <= eap->line2; ++lnum)
{
funcexe_T funcexe;

if (!eap->skip && eap->addr_count > 0)
{
if (lnum > curbuf->b_ml.ml_line_count)
{


emsg(_(e_invrange));
break;
}
curwin->w_cursor.lnum = lnum;
curwin->w_cursor.col = 0;
curwin->w_cursor.coladd = 0;
}
arg = startarg;

vim_memset(&funcexe, 0, sizeof(funcexe));
funcexe.firstline = eap->line1;
funcexe.lastline = eap->line2;
funcexe.doesrange = &doesrange;
funcexe.evaluate = !eap->skip;
funcexe.partial = partial;
funcexe.selfdict = fudi.fd_dict;
if (get_func_tv(name, -1, &rettv, &arg, &funcexe) == FAIL)
{
failed = TRUE;
break;
}
if (has_watchexpr())
dbg_check_breakpoint(eap);


if (handle_subscript(&arg, &rettv, !eap->skip, TRUE,
name, &name) == FAIL)
{
failed = TRUE;
break;
}

clear_tv(&rettv);
if (doesrange || eap->skip)
break;





if (aborting())
break;
}
if (eap->skip)
--emsg_skip;


if (!failed || eap->cstack->cs_trylevel > 0)
{

if (!ends_excmd(*arg))
{
if (!failed)
{
emsg_severe = TRUE;
emsg(_(e_trailing));
}
}
else
eap->nextcmd = check_nextcmd(arg);
}

end:
dict_unref(fudi.fd_dict);
vim_free(tofree);
}









int
do_return(
exarg_T *eap,
int reanimate,
int is_cmd,
void *rettv)
{
int idx;
cstack_T *cstack = eap->cstack;

if (reanimate)

current_funccal->returned = FALSE;







idx = cleanup_conditionals(eap->cstack, 0, TRUE);
if (idx >= 0)
{
cstack->cs_pending[idx] = CSTP_RETURN;

if (!is_cmd && !reanimate)



cstack->cs_rettv[idx] = rettv;
else
{


if (reanimate)
rettv = current_funccal->rettv;

if (rettv != NULL)
{

if ((cstack->cs_rettv[idx] = alloc_tv()) != NULL)
*(typval_T *)cstack->cs_rettv[idx] = *(typval_T *)rettv;
else
emsg(_(e_outofmem));
}
else
cstack->cs_rettv[idx] = NULL;

if (reanimate)
{



current_funccal->rettv->v_type = VAR_NUMBER;
current_funccal->rettv->vval.v_number = 0;
}
}
report_make_pending(CSTP_RETURN, rettv);
}
else
{
current_funccal->returned = TRUE;




if (!reanimate && rettv != NULL)
{
clear_tv(current_funccal->rettv);
*current_funccal->rettv = *(typval_T *)rettv;
if (!is_cmd)
vim_free(rettv);
}
}

return idx < 0;
}




void
discard_pending_return(void *rettv)
{
free_tv((typval_T *)rettv);
}





char_u *
get_return_cmd(void *rettv)
{
char_u *s = NULL;
char_u *tofree = NULL;
char_u numbuf[NUMBUFLEN];

if (rettv != NULL)
s = echo_string((typval_T *)rettv, &tofree, numbuf, 0);
if (s == NULL)
s = (char_u *)"";

STRCPY(IObuff, ":return ");
STRNCPY(IObuff + 8, s, IOSIZE - 8);
if (STRLEN(s) + 8 >= IOSIZE)
STRCPY(IObuff + IOSIZE - 4, "...");
vim_free(tofree);
return vim_strsave(IObuff);
}






char_u *
get_func_line(
int c UNUSED,
void *cookie,
int indent UNUSED,
int do_concat UNUSED)
{
funccall_T *fcp = (funccall_T *)cookie;
ufunc_T *fp = fcp->func;
char_u *retval;
garray_T *gap; 


if (fcp->dbg_tick != debug_tick)
{
fcp->breakpoint = dbg_find_breakpoint(FALSE, fp->uf_name,
SOURCING_LNUM);
fcp->dbg_tick = debug_tick;
}
#if defined(FEAT_PROFILE)
if (do_profiling == PROF_YES)
func_line_end(cookie);
#endif

gap = &fp->uf_lines;
if (((fp->uf_flags & FC_ABORT) && did_emsg && !aborted_in_try())
|| fcp->returned)
retval = NULL;
else
{

while (fcp->linenr < gap->ga_len
&& ((char_u **)(gap->ga_data))[fcp->linenr] == NULL)
++fcp->linenr;
if (fcp->linenr >= gap->ga_len)
retval = NULL;
else
{
retval = vim_strsave(((char_u **)(gap->ga_data))[fcp->linenr++]);
SOURCING_LNUM = fcp->linenr;
#if defined(FEAT_PROFILE)
if (do_profiling == PROF_YES)
func_line_start(cookie);
#endif
}
}


if (fcp->breakpoint != 0 && fcp->breakpoint <= SOURCING_LNUM)
{
dbg_breakpoint(fp->uf_name, SOURCING_LNUM);

fcp->breakpoint = dbg_find_breakpoint(FALSE, fp->uf_name,
SOURCING_LNUM);
fcp->dbg_tick = debug_tick;
}

return retval;
}





int
func_has_ended(void *cookie)
{
funccall_T *fcp = (funccall_T *)cookie;



return (((fcp->func->uf_flags & FC_ABORT) && did_emsg && !aborted_in_try())
|| fcp->returned);
}




int
func_has_abort(
void *cookie)
{
return ((funccall_T *)cookie)->func->uf_flags & FC_ABORT;
}









dict_T *
make_partial(dict_T *selfdict_in, typval_T *rettv)
{
char_u *fname;
char_u *tofree = NULL;
ufunc_T *fp;
char_u fname_buf[FLEN_FIXED + 1];
int error;
dict_T *selfdict = selfdict_in;

if (rettv->v_type == VAR_PARTIAL && rettv->vval.v_partial->pt_func != NULL)
fp = rettv->vval.v_partial->pt_func;
else
{
fname = rettv->v_type == VAR_FUNC ? rettv->vval.v_string
: rettv->vval.v_partial->pt_name;

fname = fname_trans_sid(fname, fname_buf, &tofree, &error);
fp = find_func(fname, NULL);
vim_free(tofree);
}

if (fp != NULL && (fp->uf_flags & FC_DICT))
{
partial_T *pt = ALLOC_CLEAR_ONE(partial_T);

if (pt != NULL)
{
pt->pt_refcount = 1;
pt->pt_dict = selfdict;
pt->pt_auto = TRUE;
selfdict = NULL;
if (rettv->v_type == VAR_FUNC)
{


pt->pt_name = rettv->vval.v_string;
}
else
{
partial_T *ret_pt = rettv->vval.v_partial;
int i;




if (ret_pt->pt_name != NULL)
{
pt->pt_name = vim_strsave(ret_pt->pt_name);
func_ref(pt->pt_name);
}
else
{
pt->pt_func = ret_pt->pt_func;
func_ptr_ref(pt->pt_func);
}
if (ret_pt->pt_argc > 0)
{
pt->pt_argv = ALLOC_MULT(typval_T, ret_pt->pt_argc);
if (pt->pt_argv == NULL)

pt->pt_argc = 0;
else
{
pt->pt_argc = ret_pt->pt_argc;
for (i = 0; i < pt->pt_argc; i++)
copy_tv(&ret_pt->pt_argv[i], &pt->pt_argv[i]);
}
}
partial_unref(ret_pt);
}
rettv->v_type = VAR_PARTIAL;
rettv->vval.v_partial = pt;
}
}
return selfdict;
}




char_u *
func_name(void *cookie)
{
return ((funccall_T *)cookie)->func->uf_name;
}




linenr_T *
func_breakpoint(void *cookie)
{
return &((funccall_T *)cookie)->breakpoint;
}




int *
func_dbg_tick(void *cookie)
{
return &((funccall_T *)cookie)->dbg_tick;
}




int
func_level(void *cookie)
{
return ((funccall_T *)cookie)->level;
}




int
current_func_returned(void)
{
return current_funccal->returned;
}

int
free_unref_funccal(int copyID, int testing)
{
int did_free = FALSE;
int did_free_funccal = FALSE;
funccall_T *fc, **pfc;

for (pfc = &previous_funccal; *pfc != NULL; )
{
if (can_free_funccal(*pfc, copyID))
{
fc = *pfc;
*pfc = fc->caller;
free_funccal_contents(fc);
did_free = TRUE;
did_free_funccal = TRUE;
}
else
pfc = &(*pfc)->caller;
}
if (did_free_funccal)


(void)garbage_collect(testing);

return did_free;
}




static funccall_T *
get_funccal(void)
{
int i;
funccall_T *funccal;
funccall_T *temp_funccal;

funccal = current_funccal;
if (debug_backtrace_level > 0)
{
for (i = 0; i < debug_backtrace_level; i++)
{
temp_funccal = funccal->caller;
if (temp_funccal)
funccal = temp_funccal;
else

debug_backtrace_level = i;
}
}
return funccal;
}





hashtab_T *
get_funccal_local_ht()
{
if (current_funccal == NULL || current_funccal->l_vars.dv_refcount == 0)
return NULL;
return &get_funccal()->l_vars.dv_hashtab;
}





dictitem_T *
get_funccal_local_var()
{
if (current_funccal == NULL || current_funccal->l_vars.dv_refcount == 0)
return NULL;
return &get_funccal()->l_vars_var;
}





hashtab_T *
get_funccal_args_ht()
{
if (current_funccal == NULL || current_funccal->l_vars.dv_refcount == 0)
return NULL;
return &get_funccal()->l_avars.dv_hashtab;
}





dictitem_T *
get_funccal_args_var()
{
if (current_funccal == NULL || current_funccal->l_vars.dv_refcount == 0)
return NULL;
return &get_funccal()->l_avars_var;
}




void
list_func_vars(int *first)
{
if (current_funccal != NULL && current_funccal->l_vars.dv_refcount > 0)
list_hashtable_vars(&current_funccal->l_vars.dv_hashtab,
"l:", FALSE, first);
}






dict_T *
get_current_funccal_dict(hashtab_T *ht)
{
if (current_funccal != NULL
&& ht == &current_funccal->l_vars.dv_hashtab)
return &current_funccal->l_vars;
return NULL;
}




hashitem_T *
find_hi_in_scoped_ht(char_u *name, hashtab_T **pht)
{
funccall_T *old_current_funccal = current_funccal;
hashtab_T *ht;
hashitem_T *hi = NULL;
char_u *varname;

if (current_funccal == NULL || current_funccal->func->uf_scoped == NULL)
return NULL;


current_funccal = current_funccal->func->uf_scoped;
while (current_funccal != NULL)
{
ht = find_var_ht(name, &varname);
if (ht != NULL && *varname != NUL)
{
hi = hash_find(ht, varname);
if (!HASHITEM_EMPTY(hi))
{
*pht = ht;
break;
}
}
if (current_funccal == current_funccal->func->uf_scoped)
break;
current_funccal = current_funccal->func->uf_scoped;
}
current_funccal = old_current_funccal;

return hi;
}




dictitem_T *
find_var_in_scoped_ht(char_u *name, int no_autoload)
{
dictitem_T *v = NULL;
funccall_T *old_current_funccal = current_funccal;
hashtab_T *ht;
char_u *varname;

if (current_funccal == NULL || current_funccal->func->uf_scoped == NULL)
return NULL;


current_funccal = current_funccal->func->uf_scoped;
while (current_funccal)
{
ht = find_var_ht(name, &varname);
if (ht != NULL && *varname != NUL)
{
v = find_var_in_ht(ht, *name, varname, no_autoload);
if (v != NULL)
break;
}
if (current_funccal == current_funccal->func->uf_scoped)
break;
current_funccal = current_funccal->func->uf_scoped;
}
current_funccal = old_current_funccal;

return v;
}




int
set_ref_in_previous_funccal(int copyID)
{
int abort = FALSE;
funccall_T *fc;

for (fc = previous_funccal; !abort && fc != NULL; fc = fc->caller)
{
fc->fc_copyID = copyID + 1;
abort = abort
|| set_ref_in_ht(&fc->l_vars.dv_hashtab, copyID + 1, NULL)
|| set_ref_in_ht(&fc->l_avars.dv_hashtab, copyID + 1, NULL)
|| set_ref_in_list_items(&fc->l_varlist, copyID + 1, NULL);
}
return abort;
}

static int
set_ref_in_funccal(funccall_T *fc, int copyID)
{
int abort = FALSE;

if (fc->fc_copyID != copyID)
{
fc->fc_copyID = copyID;
abort = abort
|| set_ref_in_ht(&fc->l_vars.dv_hashtab, copyID, NULL)
|| set_ref_in_ht(&fc->l_avars.dv_hashtab, copyID, NULL)
|| set_ref_in_list_items(&fc->l_varlist, copyID, NULL)
|| set_ref_in_func(NULL, fc->func, copyID);
}
return abort;
}




int
set_ref_in_call_stack(int copyID)
{
int abort = FALSE;
funccall_T *fc;
funccal_entry_T *entry;

for (fc = current_funccal; !abort && fc != NULL; fc = fc->caller)
abort = abort || set_ref_in_funccal(fc, copyID);


for (entry = funccal_stack; !abort && entry != NULL; entry = entry->next)
for (fc = entry->top_funccal; !abort && fc != NULL; fc = fc->caller)
abort = abort || set_ref_in_funccal(fc, copyID);

return abort;
}




int
set_ref_in_functions(int copyID)
{
int todo;
hashitem_T *hi = NULL;
int abort = FALSE;
ufunc_T *fp;

todo = (int)func_hashtab.ht_used;
for (hi = func_hashtab.ht_array; todo > 0 && !got_int; ++hi)
{
if (!HASHITEM_EMPTY(hi))
{
--todo;
fp = HI2UF(hi);
if (!func_name_refcount(fp->uf_name))
abort = abort || set_ref_in_func(NULL, fp, copyID);
}
}
return abort;
}




int
set_ref_in_func_args(int copyID)
{
int i;
int abort = FALSE;

for (i = 0; i < funcargs.ga_len; ++i)
abort = abort || set_ref_in_item(((typval_T **)funcargs.ga_data)[i],
copyID, NULL, NULL);
return abort;
}





int
set_ref_in_func(char_u *name, ufunc_T *fp_in, int copyID)
{
ufunc_T *fp = fp_in;
funccall_T *fc;
int error = FCERR_NONE;
char_u fname_buf[FLEN_FIXED + 1];
char_u *tofree = NULL;
char_u *fname;
int abort = FALSE;

if (name == NULL && fp_in == NULL)
return FALSE;

if (fp_in == NULL)
{
fname = fname_trans_sid(name, fname_buf, &tofree, &error);
fp = find_func(fname, NULL);
}
if (fp != NULL)
{
for (fc = fp->uf_scoped; fc != NULL; fc = fc->func->uf_scoped)
abort = abort || set_ref_in_funccal(fc, copyID);
}
vim_free(tofree);
return abort;
}

#endif 
