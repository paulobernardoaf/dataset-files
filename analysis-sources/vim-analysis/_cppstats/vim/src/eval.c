#define USING_FLOAT_STUFF
#include "vim.h"
#if defined(FEAT_EVAL) || defined(PROTO)
#if defined(VMS)
#include <float.h>
#endif
static char *e_dictrange = N_("E719: Cannot use [:] with a Dictionary");
#if defined(FEAT_FLOAT)
static char *e_float_as_string = N_("E806: using Float as a String");
#endif
#define NAMESPACE_CHAR (char_u *)"abglstvw"
static int current_copyID = 0;
static int echo_attr = 0; 
typedef struct
{
int fi_semicolon; 
int fi_varcount; 
listwatch_T fi_lw; 
list_T *fi_list; 
int fi_bi; 
blob_T *fi_blob; 
} forinfo_T;
static int tv_op(typval_T *tv1, typval_T *tv2, char_u *op);
static int eval2(char_u **arg, typval_T *rettv, int evaluate);
static int eval3(char_u **arg, typval_T *rettv, int evaluate);
static int eval4(char_u **arg, typval_T *rettv, int evaluate);
static int eval5(char_u **arg, typval_T *rettv, int evaluate);
static int eval6(char_u **arg, typval_T *rettv, int evaluate, int want_string);
static int eval7(char_u **arg, typval_T *rettv, int evaluate, int want_string);
static int eval7_leader(typval_T *rettv, char_u *start_leader, char_u **end_leaderp);
static int free_unref_items(int copyID);
static char_u *make_expanded_name(char_u *in_start, char_u *expr_start, char_u *expr_end, char_u *in_end);
static int tv_check_lock(typval_T *tv, char_u *name, int use_gettext);
varnumber_T
num_divide(varnumber_T n1, varnumber_T n2)
{
varnumber_T result;
if (n2 == 0) 
{
if (n1 == 0)
result = VARNUM_MIN; 
else if (n1 < 0)
result = -VARNUM_MAX;
else
result = VARNUM_MAX;
}
else
result = n1 / n2;
return result;
}
varnumber_T
num_modulus(varnumber_T n1, varnumber_T n2)
{
return (n2 == 0) ? 0 : (n1 % n2);
}
#if defined(EBCDIC) || defined(PROTO)
static int
compare_func_name(const void *s1, const void *s2)
{
struct fst *p1 = (struct fst *)s1;
struct fst *p2 = (struct fst *)s2;
return STRCMP(p1->f_name, p2->f_name);
}
static void
sortFunctions(void)
{
int funcCnt = (int)(sizeof(functions) / sizeof(struct fst)) - 1;
qsort(functions, (size_t)funcCnt, sizeof(struct fst), compare_func_name);
}
#endif
void
eval_init(void)
{
evalvars_init();
func_init();
#if defined(EBCDIC)
sortFunctions();
#endif
}
#if defined(EXITFREE) || defined(PROTO)
void
eval_clear(void)
{
evalvars_clear();
free_scriptnames(); 
free_locales();
free_autoload_scriptnames();
(void)garbage_collect(FALSE);
free_all_functions();
}
#endif
int
eval_to_bool(
char_u *arg,
int *error,
char_u **nextcmd,
int skip) 
{
typval_T tv;
varnumber_T retval = FALSE;
if (skip)
++emsg_skip;
if (eval0(arg, &tv, nextcmd, !skip) == FAIL)
*error = TRUE;
else
{
*error = FALSE;
if (!skip)
{
retval = (tv_get_number_chk(&tv, error) != 0);
clear_tv(&tv);
}
}
if (skip)
--emsg_skip;
return (int)retval;
}
static int
eval1_emsg(char_u **arg, typval_T *rettv, int evaluate)
{
char_u *start = *arg;
int ret;
int did_emsg_before = did_emsg;
int called_emsg_before = called_emsg;
ret = eval1(arg, rettv, evaluate);
if (ret == FAIL)
{
if (!aborting() && did_emsg == did_emsg_before
&& called_emsg == called_emsg_before)
semsg(_(e_invexpr2), start);
}
return ret;
}
int
eval_expr_typval(typval_T *expr, typval_T *argv, int argc, typval_T *rettv)
{
char_u *s;
char_u buf[NUMBUFLEN];
funcexe_T funcexe;
if (expr->v_type == VAR_FUNC)
{
s = expr->vval.v_string;
if (s == NULL || *s == NUL)
return FAIL;
vim_memset(&funcexe, 0, sizeof(funcexe));
funcexe.evaluate = TRUE;
if (call_func(s, -1, rettv, argc, argv, &funcexe) == FAIL)
return FAIL;
}
else if (expr->v_type == VAR_PARTIAL)
{
partial_T *partial = expr->vval.v_partial;
if (partial->pt_func != NULL && partial->pt_func->uf_dfunc_idx >= 0)
{
if (call_def_function(partial->pt_func, argc, argv, rettv) == FAIL)
return FAIL;
}
else
{
s = partial_name(partial);
if (s == NULL || *s == NUL)
return FAIL;
vim_memset(&funcexe, 0, sizeof(funcexe));
funcexe.evaluate = TRUE;
funcexe.partial = partial;
if (call_func(s, -1, rettv, argc, argv, &funcexe) == FAIL)
return FAIL;
}
}
else
{
s = tv_get_string_buf_chk(expr, buf);
if (s == NULL)
return FAIL;
s = skipwhite(s);
if (eval1_emsg(&s, rettv, TRUE) == FAIL)
return FAIL;
if (*s != NUL) 
{
clear_tv(rettv);
semsg(_(e_invexpr2), s);
return FAIL;
}
}
return OK;
}
int
eval_expr_to_bool(typval_T *expr, int *error)
{
typval_T rettv;
int res;
if (eval_expr_typval(expr, NULL, 0, &rettv) == FAIL)
{
*error = TRUE;
return FALSE;
}
res = (tv_get_number_chk(&rettv, error) != 0);
clear_tv(&rettv);
return res;
}
char_u *
eval_to_string_skip(
char_u *arg,
char_u **nextcmd,
int skip) 
{
typval_T tv;
char_u *retval;
if (skip)
++emsg_skip;
if (eval0(arg, &tv, nextcmd, !skip) == FAIL || skip)
retval = NULL;
else
{
retval = vim_strsave(tv_get_string(&tv));
clear_tv(&tv);
}
if (skip)
--emsg_skip;
return retval;
}
int
skip_expr(char_u **pp)
{
typval_T rettv;
*pp = skipwhite(*pp);
return eval1(pp, &rettv, FALSE);
}
char_u *
eval_to_string(
char_u *arg,
char_u **nextcmd,
int convert)
{
typval_T tv;
char_u *retval;
garray_T ga;
#if defined(FEAT_FLOAT)
char_u numbuf[NUMBUFLEN];
#endif
if (eval0(arg, &tv, nextcmd, TRUE) == FAIL)
retval = NULL;
else
{
if (convert && tv.v_type == VAR_LIST)
{
ga_init2(&ga, (int)sizeof(char), 80);
if (tv.vval.v_list != NULL)
{
list_join(&ga, tv.vval.v_list, (char_u *)"\n", TRUE, FALSE, 0);
if (tv.vval.v_list->lv_len > 0)
ga_append(&ga, NL);
}
ga_append(&ga, NUL);
retval = (char_u *)ga.ga_data;
}
#if defined(FEAT_FLOAT)
else if (convert && tv.v_type == VAR_FLOAT)
{
vim_snprintf((char *)numbuf, NUMBUFLEN, "%g", tv.vval.v_float);
retval = vim_strsave(numbuf);
}
#endif
else
retval = vim_strsave(tv_get_string(&tv));
clear_tv(&tv);
}
return retval;
}
char_u *
eval_to_string_safe(
char_u *arg,
char_u **nextcmd,
int use_sandbox)
{
char_u *retval;
funccal_entry_T funccal_entry;
save_funccal(&funccal_entry);
if (use_sandbox)
++sandbox;
++textlock;
retval = eval_to_string(arg, nextcmd, FALSE);
if (use_sandbox)
--sandbox;
--textlock;
restore_funccal();
return retval;
}
varnumber_T
eval_to_number(char_u *expr)
{
typval_T rettv;
varnumber_T retval;
char_u *p = skipwhite(expr);
++emsg_off;
if (eval1(&p, &rettv, TRUE) == FAIL)
retval = -1;
else
{
retval = tv_get_number_chk(&rettv, NULL);
clear_tv(&rettv);
}
--emsg_off;
return retval;
}
typval_T *
eval_expr(char_u *arg, char_u **nextcmd)
{
typval_T *tv;
tv = ALLOC_ONE(typval_T);
if (tv != NULL && eval0(arg, tv, nextcmd, TRUE) == FAIL)
VIM_CLEAR(tv);
return tv;
}
int
call_vim_function(
char_u *func,
int argc,
typval_T *argv,
typval_T *rettv)
{
int ret;
funcexe_T funcexe;
rettv->v_type = VAR_UNKNOWN; 
vim_memset(&funcexe, 0, sizeof(funcexe));
funcexe.firstline = curwin->w_cursor.lnum;
funcexe.lastline = curwin->w_cursor.lnum;
funcexe.evaluate = TRUE;
ret = call_func(func, -1, rettv, argc, argv, &funcexe);
if (ret == FAIL)
clear_tv(rettv);
return ret;
}
varnumber_T
call_func_retnr(
char_u *func,
int argc,
typval_T *argv)
{
typval_T rettv;
varnumber_T retval;
if (call_vim_function(func, argc, argv, &rettv) == FAIL)
return -1;
retval = tv_get_number_chk(&rettv, NULL);
clear_tv(&rettv);
return retval;
}
void *
call_func_retstr(
char_u *func,
int argc,
typval_T *argv)
{
typval_T rettv;
char_u *retval;
if (call_vim_function(func, argc, argv, &rettv) == FAIL)
return NULL;
retval = vim_strsave(tv_get_string(&rettv));
clear_tv(&rettv);
return retval;
}
void *
call_func_retlist(
char_u *func,
int argc,
typval_T *argv)
{
typval_T rettv;
if (call_vim_function(func, argc, argv, &rettv) == FAIL)
return NULL;
if (rettv.v_type != VAR_LIST)
{
clear_tv(&rettv);
return NULL;
}
return rettv.vval.v_list;
}
#if defined(FEAT_FOLDING)
int
eval_foldexpr(char_u *arg, int *cp)
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
else
{
if (tv.v_type == VAR_NUMBER)
retval = tv.vval.v_number;
else if (tv.v_type != VAR_STRING || tv.vval.v_string == NULL)
retval = 0;
else
{
s = tv.vval.v_string;
if (!VIM_ISDIGIT(*s) && *s != '-')
*cp = *s++;
retval = atol((char *)s);
}
clear_tv(&tv);
}
--emsg_off;
if (use_sandbox)
--sandbox;
--textlock;
return (int)retval;
}
#endif
char_u *
get_lval(
char_u *name,
typval_T *rettv,
lval_T *lp,
int unlet,
int skip,
int flags, 
int fne_flags) 
{
char_u *p;
char_u *expr_start, *expr_end;
int cc;
dictitem_T *v;
typval_T var1;
typval_T var2;
int empty1 = FALSE;
listitem_T *ni;
char_u *key = NULL;
int len;
hashtab_T *ht;
int quiet = flags & GLV_QUIET;
vim_memset(lp, 0, sizeof(lval_T));
if (skip)
{
lp->ll_name = name;
return find_name_end(name, NULL, NULL, FNE_INCL_BR | fne_flags);
}
p = find_name_end(name, &expr_start, &expr_end, fne_flags);
lp->ll_name_end = p;
if (expr_start != NULL)
{
if (unlet && !VIM_ISWHITE(*p) && !ends_excmd(*p)
&& *p != '[' && *p != '.')
{
emsg(_(e_trailing));
return NULL;
}
lp->ll_exp_name = make_expanded_name(name, expr_start, expr_end, p);
if (lp->ll_exp_name == NULL)
{
if (!aborting() && !quiet)
{
emsg_severe = TRUE;
semsg(_(e_invarg2), name);
return NULL;
}
}
lp->ll_name = lp->ll_exp_name;
}
else
{
lp->ll_name = name;
if (current_sctx.sc_version == SCRIPT_VERSION_VIM9 && *p == ':')
{
scriptitem_T *si = SCRIPT_ITEM(current_sctx.sc_sid);
char_u *tp = skipwhite(p + 1);
lp->ll_type = parse_type(&tp, &si->sn_type_list);
lp->ll_name_end = tp;
}
}
if ((*p != '[' && *p != '.') || lp->ll_name == NULL)
return p;
cc = *p;
*p = NUL;
v = find_var(lp->ll_name, (flags & GLV_READ_ONLY) ? NULL : &ht,
flags & GLV_NO_AUTOLOAD);
if (v == NULL && !quiet)
semsg(_(e_undefvar), lp->ll_name);
*p = cc;
if (v == NULL)
return NULL;
lp->ll_tv = &v->di_tv;
var1.v_type = VAR_UNKNOWN;
var2.v_type = VAR_UNKNOWN;
while (*p == '[' || (*p == '.' && lp->ll_tv->v_type == VAR_DICT))
{
if (!(lp->ll_tv->v_type == VAR_LIST && lp->ll_tv->vval.v_list != NULL)
&& !(lp->ll_tv->v_type == VAR_DICT
&& lp->ll_tv->vval.v_dict != NULL)
&& !(lp->ll_tv->v_type == VAR_BLOB
&& lp->ll_tv->vval.v_blob != NULL))
{
if (!quiet)
emsg(_("E689: Can only index a List, Dictionary or Blob"));
return NULL;
}
if (lp->ll_range)
{
if (!quiet)
emsg(_("E708: [:] must come last"));
return NULL;
}
len = -1;
if (*p == '.')
{
key = p + 1;
for (len = 0; ASCII_ISALNUM(key[len]) || key[len] == '_'; ++len)
;
if (len == 0)
{
if (!quiet)
emsg(_(e_emptykey));
return NULL;
}
p = key + len;
}
else
{
p = skipwhite(p + 1);
if (*p == ':')
empty1 = TRUE;
else
{
empty1 = FALSE;
if (eval1(&p, &var1, TRUE) == FAIL) 
return NULL;
if (tv_get_string_chk(&var1) == NULL)
{
clear_tv(&var1);
return NULL;
}
}
if (*p == ':')
{
if (lp->ll_tv->v_type == VAR_DICT)
{
if (!quiet)
emsg(_(e_dictrange));
clear_tv(&var1);
return NULL;
}
if (rettv != NULL
&& !(rettv->v_type == VAR_LIST
&& rettv->vval.v_list != NULL)
&& !(rettv->v_type == VAR_BLOB
&& rettv->vval.v_blob != NULL))
{
if (!quiet)
emsg(_("E709: [:] requires a List or Blob value"));
clear_tv(&var1);
return NULL;
}
p = skipwhite(p + 1);
if (*p == ']')
lp->ll_empty2 = TRUE;
else
{
lp->ll_empty2 = FALSE;
if (eval1(&p, &var2, TRUE) == FAIL) 
{
clear_tv(&var1);
return NULL;
}
if (tv_get_string_chk(&var2) == NULL)
{
clear_tv(&var1);
clear_tv(&var2);
return NULL;
}
}
lp->ll_range = TRUE;
}
else
lp->ll_range = FALSE;
if (*p != ']')
{
if (!quiet)
emsg(_(e_missbrac));
clear_tv(&var1);
clear_tv(&var2);
return NULL;
}
++p;
}
if (lp->ll_tv->v_type == VAR_DICT)
{
if (len == -1)
{
key = tv_get_string_chk(&var1); 
if (key == NULL)
{
clear_tv(&var1);
return NULL;
}
}
lp->ll_list = NULL;
lp->ll_dict = lp->ll_tv->vval.v_dict;
lp->ll_di = dict_find(lp->ll_dict, key, len);
if (rettv != NULL && lp->ll_dict->dv_scope != 0)
{
int prevval;
int wrong;
if (len != -1)
{
prevval = key[len];
key[len] = NUL;
}
else
prevval = 0; 
wrong = (lp->ll_dict->dv_scope == VAR_DEF_SCOPE
&& rettv->v_type == VAR_FUNC
&& var_check_func_name(key, lp->ll_di == NULL))
|| !valid_varname(key);
if (len != -1)
key[len] = prevval;
if (wrong)
return NULL;
}
if (lp->ll_di == NULL)
{
if (lp->ll_dict == get_vimvar_dict()
|| &lp->ll_dict->dv_hashtab == get_funccal_args_ht())
{
semsg(_(e_illvar), name);
clear_tv(&var1);
return NULL;
}
if (*p == '[' || *p == '.' || unlet)
{
if (!quiet)
semsg(_(e_dictkey), key);
clear_tv(&var1);
return NULL;
}
if (len == -1)
lp->ll_newkey = vim_strsave(key);
else
lp->ll_newkey = vim_strnsave(key, len);
clear_tv(&var1);
if (lp->ll_newkey == NULL)
p = NULL;
break;
}
else if ((flags & GLV_READ_ONLY) == 0
&& var_check_ro(lp->ll_di->di_flags, name, FALSE))
{
clear_tv(&var1);
return NULL;
}
clear_tv(&var1);
lp->ll_tv = &lp->ll_di->di_tv;
}
else if (lp->ll_tv->v_type == VAR_BLOB)
{
long bloblen = blob_len(lp->ll_tv->vval.v_blob);
if (empty1)
lp->ll_n1 = 0;
else
lp->ll_n1 = (long)tv_get_number(&var1);
clear_tv(&var1);
if (lp->ll_n1 < 0
|| lp->ll_n1 > bloblen
|| (lp->ll_range && lp->ll_n1 == bloblen))
{
if (!quiet)
semsg(_(e_blobidx), lp->ll_n1);
clear_tv(&var2);
return NULL;
}
if (lp->ll_range && !lp->ll_empty2)
{
lp->ll_n2 = (long)tv_get_number(&var2);
clear_tv(&var2);
if (lp->ll_n2 < 0
|| lp->ll_n2 >= bloblen
|| lp->ll_n2 < lp->ll_n1)
{
if (!quiet)
semsg(_(e_blobidx), lp->ll_n2);
return NULL;
}
}
lp->ll_blob = lp->ll_tv->vval.v_blob;
lp->ll_tv = NULL;
break;
}
else
{
if (empty1)
lp->ll_n1 = 0;
else
lp->ll_n1 = (long)tv_get_number(&var1);
clear_tv(&var1);
lp->ll_dict = NULL;
lp->ll_list = lp->ll_tv->vval.v_list;
lp->ll_li = list_find(lp->ll_list, lp->ll_n1);
if (lp->ll_li == NULL)
{
if (lp->ll_n1 < 0)
{
lp->ll_n1 = 0;
lp->ll_li = list_find(lp->ll_list, lp->ll_n1);
}
}
if (lp->ll_li == NULL)
{
clear_tv(&var2);
if (!quiet)
semsg(_(e_listidx), lp->ll_n1);
return NULL;
}
if (lp->ll_range && !lp->ll_empty2)
{
lp->ll_n2 = (long)tv_get_number(&var2);
clear_tv(&var2);
if (lp->ll_n2 < 0)
{
ni = list_find(lp->ll_list, lp->ll_n2);
if (ni == NULL)
{
if (!quiet)
semsg(_(e_listidx), lp->ll_n2);
return NULL;
}
lp->ll_n2 = list_idx_of_item(lp->ll_list, ni);
}
if (lp->ll_n1 < 0)
lp->ll_n1 = list_idx_of_item(lp->ll_list, lp->ll_li);
if (lp->ll_n2 < lp->ll_n1)
{
if (!quiet)
semsg(_(e_listidx), lp->ll_n2);
return NULL;
}
}
lp->ll_tv = &lp->ll_li->li_tv;
}
}
clear_tv(&var1);
lp->ll_name_end = p;
return p;
}
void
clear_lval(lval_T *lp)
{
vim_free(lp->ll_exp_name);
vim_free(lp->ll_newkey);
}
void
set_var_lval(
lval_T *lp,
char_u *endp,
typval_T *rettv,
int copy,
int flags, 
char_u *op)
{
int cc;
listitem_T *ri;
dictitem_T *di;
if (lp->ll_tv == NULL)
{
cc = *endp;
*endp = NUL;
if (lp->ll_blob != NULL)
{
int error = FALSE, val;
if (op != NULL && *op != '=')
{
semsg(_(e_letwrong), op);
return;
}
if (lp->ll_range && rettv->v_type == VAR_BLOB)
{
int il, ir;
if (lp->ll_empty2)
lp->ll_n2 = blob_len(lp->ll_blob) - 1;
if (lp->ll_n2 - lp->ll_n1 + 1 != blob_len(rettv->vval.v_blob))
{
emsg(_("E972: Blob value does not have the right number of bytes"));
return;
}
if (lp->ll_empty2)
lp->ll_n2 = blob_len(lp->ll_blob);
ir = 0;
for (il = lp->ll_n1; il <= lp->ll_n2; il++)
blob_set(lp->ll_blob, il,
blob_get(rettv->vval.v_blob, ir++));
}
else
{
val = (int)tv_get_number_chk(rettv, &error);
if (!error)
{
garray_T *gap = &lp->ll_blob->bv_ga;
if (lp->ll_n1 < gap->ga_len
|| (lp->ll_n1 == gap->ga_len
&& ga_grow(&lp->ll_blob->bv_ga, 1) == OK))
{
blob_set(lp->ll_blob, lp->ll_n1, val);
if (lp->ll_n1 == gap->ga_len)
++gap->ga_len;
}
}
}
}
else if (op != NULL && *op != '=')
{
typval_T tv;
if (flags & LET_IS_CONST)
{
emsg(_(e_cannot_mod));
*endp = cc;
return;
}
di = NULL;
if (get_var_tv(lp->ll_name, (int)STRLEN(lp->ll_name),
&tv, &di, TRUE, FALSE) == OK)
{
if ((di == NULL
|| (!var_check_ro(di->di_flags, lp->ll_name, FALSE)
&& !tv_check_lock(&di->di_tv, lp->ll_name, FALSE)))
&& tv_op(&tv, rettv, op) == OK)
set_var(lp->ll_name, &tv, FALSE);
clear_tv(&tv);
}
}
else
set_var_const(lp->ll_name, lp->ll_type, rettv, copy, flags);
*endp = cc;
}
else if (var_check_lock(lp->ll_newkey == NULL
? lp->ll_tv->v_lock
: lp->ll_tv->vval.v_dict->dv_lock, lp->ll_name, FALSE))
;
else if (lp->ll_range)
{
listitem_T *ll_li = lp->ll_li;
int ll_n1 = lp->ll_n1;
if (flags & LET_IS_CONST)
{
emsg(_("E996: Cannot lock a range"));
return;
}
for (ri = rettv->vval.v_list->lv_first; ri != NULL && ll_li != NULL; )
{
if (var_check_lock(ll_li->li_tv.v_lock, lp->ll_name, FALSE))
return;
ri = ri->li_next;
if (ri == NULL || (!lp->ll_empty2 && lp->ll_n2 == ll_n1))
break;
ll_li = ll_li->li_next;
++ll_n1;
}
for (ri = rettv->vval.v_list->lv_first; ri != NULL; )
{
if (op != NULL && *op != '=')
tv_op(&lp->ll_li->li_tv, &ri->li_tv, op);
else
{
clear_tv(&lp->ll_li->li_tv);
copy_tv(&ri->li_tv, &lp->ll_li->li_tv);
}
ri = ri->li_next;
if (ri == NULL || (!lp->ll_empty2 && lp->ll_n2 == lp->ll_n1))
break;
if (lp->ll_li->li_next == NULL)
{
if (list_append_number(lp->ll_list, 0) == FAIL)
{
ri = NULL;
break;
}
}
lp->ll_li = lp->ll_li->li_next;
++lp->ll_n1;
}
if (ri != NULL)
emsg(_("E710: List value has more items than target"));
else if (lp->ll_empty2
? (lp->ll_li != NULL && lp->ll_li->li_next != NULL)
: lp->ll_n1 != lp->ll_n2)
emsg(_("E711: List value has not enough items"));
}
else
{
if (flags & LET_IS_CONST)
{
emsg(_("E996: Cannot lock a list or dict"));
return;
}
if (lp->ll_newkey != NULL)
{
if (op != NULL && *op != '=')
{
semsg(_(e_letwrong), op);
return;
}
di = dictitem_alloc(lp->ll_newkey);
if (di == NULL)
return;
if (dict_add(lp->ll_tv->vval.v_dict, di) == FAIL)
{
vim_free(di);
return;
}
lp->ll_tv = &di->di_tv;
}
else if (op != NULL && *op != '=')
{
tv_op(lp->ll_tv, rettv, op);
return;
}
else
clear_tv(lp->ll_tv);
if (copy)
copy_tv(rettv, lp->ll_tv);
else
{
*lp->ll_tv = *rettv;
lp->ll_tv->v_lock = 0;
init_tv(rettv);
}
}
}
static int
tv_op(typval_T *tv1, typval_T *tv2, char_u *op)
{
varnumber_T n;
char_u numbuf[NUMBUFLEN];
char_u *s;
if (tv2->v_type != VAR_FUNC && tv2->v_type != VAR_DICT
&& tv2->v_type != VAR_BOOL && tv2->v_type != VAR_SPECIAL)
{
switch (tv1->v_type)
{
case VAR_UNKNOWN:
case VAR_VOID:
case VAR_DICT:
case VAR_FUNC:
case VAR_PARTIAL:
case VAR_BOOL:
case VAR_SPECIAL:
case VAR_JOB:
case VAR_CHANNEL:
break;
case VAR_BLOB:
if (*op != '+' || tv2->v_type != VAR_BLOB)
break;
if (tv1->vval.v_blob != NULL && tv2->vval.v_blob != NULL)
{
blob_T *b1 = tv1->vval.v_blob;
blob_T *b2 = tv2->vval.v_blob;
int i, len = blob_len(b2);
for (i = 0; i < len; i++)
ga_append(&b1->bv_ga, blob_get(b2, i));
}
return OK;
case VAR_LIST:
if (*op != '+' || tv2->v_type != VAR_LIST)
break;
if (tv1->vval.v_list != NULL && tv2->vval.v_list != NULL)
list_extend(tv1->vval.v_list, tv2->vval.v_list, NULL);
return OK;
case VAR_NUMBER:
case VAR_STRING:
if (tv2->v_type == VAR_LIST)
break;
if (vim_strchr((char_u *)"+-*/%", *op) != NULL)
{
n = tv_get_number(tv1);
#if defined(FEAT_FLOAT)
if (tv2->v_type == VAR_FLOAT)
{
float_T f = n;
if (*op == '%')
break;
switch (*op)
{
case '+': f += tv2->vval.v_float; break;
case '-': f -= tv2->vval.v_float; break;
case '*': f *= tv2->vval.v_float; break;
case '/': f /= tv2->vval.v_float; break;
}
clear_tv(tv1);
tv1->v_type = VAR_FLOAT;
tv1->vval.v_float = f;
}
else
#endif
{
switch (*op)
{
case '+': n += tv_get_number(tv2); break;
case '-': n -= tv_get_number(tv2); break;
case '*': n *= tv_get_number(tv2); break;
case '/': n = num_divide(n, tv_get_number(tv2)); break;
case '%': n = num_modulus(n, tv_get_number(tv2)); break;
}
clear_tv(tv1);
tv1->v_type = VAR_NUMBER;
tv1->vval.v_number = n;
}
}
else
{
if (tv2->v_type == VAR_FLOAT)
break;
s = tv_get_string(tv1);
s = concat_str(s, tv_get_string_buf(tv2, numbuf));
clear_tv(tv1);
tv1->v_type = VAR_STRING;
tv1->vval.v_string = s;
}
return OK;
case VAR_FLOAT:
#if defined(FEAT_FLOAT)
{
float_T f;
if (*op == '%' || *op == '.'
|| (tv2->v_type != VAR_FLOAT
&& tv2->v_type != VAR_NUMBER
&& tv2->v_type != VAR_STRING))
break;
if (tv2->v_type == VAR_FLOAT)
f = tv2->vval.v_float;
else
f = tv_get_number(tv2);
switch (*op)
{
case '+': tv1->vval.v_float += f; break;
case '-': tv1->vval.v_float -= f; break;
case '*': tv1->vval.v_float *= f; break;
case '/': tv1->vval.v_float /= f; break;
}
}
#endif
return OK;
}
}
semsg(_(e_letwrong), op);
return FAIL;
}
void *
eval_for_line(
char_u *arg,
int *errp,
char_u **nextcmdp,
int skip)
{
forinfo_T *fi;
char_u *expr;
typval_T tv;
list_T *l;
*errp = TRUE; 
fi = ALLOC_CLEAR_ONE(forinfo_T);
if (fi == NULL)
return NULL;
expr = skip_var_list(arg, TRUE, &fi->fi_varcount, &fi->fi_semicolon);
if (expr == NULL)
return fi;
expr = skipwhite(expr);
if (expr[0] != 'i' || expr[1] != 'n' || !VIM_ISWHITE(expr[2]))
{
emsg(_(e_missing_in));
return fi;
}
if (skip)
++emsg_skip;
if (eval0(skipwhite(expr + 2), &tv, nextcmdp, !skip) == OK)
{
*errp = FALSE;
if (!skip)
{
if (tv.v_type == VAR_LIST)
{
l = tv.vval.v_list;
if (l == NULL)
{
clear_tv(&tv);
}
else
{
range_list_materialize(l);
fi->fi_list = l;
list_add_watch(l, &fi->fi_lw);
fi->fi_lw.lw_item = l->lv_first;
}
}
else if (tv.v_type == VAR_BLOB)
{
fi->fi_bi = 0;
if (tv.vval.v_blob != NULL)
{
typval_T btv;
blob_copy(tv.vval.v_blob, &btv);
fi->fi_blob = btv.vval.v_blob;
}
clear_tv(&tv);
}
else
{
emsg(_(e_listreq));
clear_tv(&tv);
}
}
}
if (skip)
--emsg_skip;
return fi;
}
int
next_for_item(void *fi_void, char_u *arg)
{
forinfo_T *fi = (forinfo_T *)fi_void;
int result;
int flag = current_sctx.sc_version == SCRIPT_VERSION_VIM9 ?
LET_NO_COMMAND : 0;
listitem_T *item;
if (fi->fi_blob != NULL)
{
typval_T tv;
if (fi->fi_bi >= blob_len(fi->fi_blob))
return FALSE;
tv.v_type = VAR_NUMBER;
tv.v_lock = VAR_FIXED;
tv.vval.v_number = blob_get(fi->fi_blob, fi->fi_bi);
++fi->fi_bi;
return ex_let_vars(arg, &tv, TRUE, fi->fi_semicolon,
fi->fi_varcount, flag, NULL) == OK;
}
item = fi->fi_lw.lw_item;
if (item == NULL)
result = FALSE;
else
{
fi->fi_lw.lw_item = item->li_next;
result = (ex_let_vars(arg, &item->li_tv, TRUE, fi->fi_semicolon,
fi->fi_varcount, flag, NULL) == OK);
}
return result;
}
void
free_for_info(void *fi_void)
{
forinfo_T *fi = (forinfo_T *)fi_void;
if (fi != NULL && fi->fi_list != NULL)
{
list_rem_watch(fi->fi_list, &fi->fi_lw);
list_unref(fi->fi_list);
}
if (fi != NULL && fi->fi_blob != NULL)
blob_unref(fi->fi_blob);
vim_free(fi);
}
void
set_context_for_expression(
expand_T *xp,
char_u *arg,
cmdidx_T cmdidx)
{
int got_eq = FALSE;
int c;
char_u *p;
if (cmdidx == CMD_let || cmdidx == CMD_const)
{
xp->xp_context = EXPAND_USER_VARS;
if (vim_strpbrk(arg, (char_u *)"\"'+-*/%.=!?~|&$([<>,#") == NULL)
{
for (p = arg + STRLEN(arg); p >= arg; )
{
xp->xp_pattern = p;
MB_PTR_BACK(arg, p);
if (VIM_ISWHITE(*p))
break;
}
return;
}
}
else
xp->xp_context = cmdidx == CMD_call ? EXPAND_FUNCTIONS
: EXPAND_EXPRESSION;
while ((xp->xp_pattern = vim_strpbrk(arg,
(char_u *)"\"'+-*/%.=!?~|&$([<>,#")) != NULL)
{
c = *xp->xp_pattern;
if (c == '&')
{
c = xp->xp_pattern[1];
if (c == '&')
{
++xp->xp_pattern;
xp->xp_context = cmdidx != CMD_let || got_eq
? EXPAND_EXPRESSION : EXPAND_NOTHING;
}
else if (c != ' ')
{
xp->xp_context = EXPAND_SETTINGS;
if ((c == 'l' || c == 'g') && xp->xp_pattern[2] == ':')
xp->xp_pattern += 2;
}
}
else if (c == '$')
{
xp->xp_context = EXPAND_ENV_VARS;
}
else if (c == '=')
{
got_eq = TRUE;
xp->xp_context = EXPAND_EXPRESSION;
}
else if (c == '#'
&& xp->xp_context == EXPAND_EXPRESSION)
{
break;
}
else if ((c == '<' || c == '#')
&& xp->xp_context == EXPAND_FUNCTIONS
&& vim_strchr(xp->xp_pattern, '(') == NULL)
{
break;
}
else if (cmdidx != CMD_let || got_eq)
{
if (c == '"') 
{
while ((c = *++xp->xp_pattern) != NUL && c != '"')
if (c == '\\' && xp->xp_pattern[1] != NUL)
++xp->xp_pattern;
xp->xp_context = EXPAND_NOTHING;
}
else if (c == '\'') 
{
while ((c = *++xp->xp_pattern) != NUL && c != '\'')
;
xp->xp_context = EXPAND_NOTHING;
}
else if (c == '|')
{
if (xp->xp_pattern[1] == '|')
{
++xp->xp_pattern;
xp->xp_context = EXPAND_EXPRESSION;
}
else
xp->xp_context = EXPAND_COMMANDS;
}
else
xp->xp_context = EXPAND_EXPRESSION;
}
else
xp->xp_context = EXPAND_EXPRESSION;
arg = xp->xp_pattern;
if (*arg != NUL)
while ((c = *++arg) != NUL && (c == ' ' || c == '\t'))
;
}
xp->xp_pattern = arg;
}
int
pattern_match(char_u *pat, char_u *text, int ic)
{
int matches = FALSE;
char_u *save_cpo;
regmatch_T regmatch;
save_cpo = p_cpo;
p_cpo = (char_u *)"";
regmatch.regprog = vim_regcomp(pat, RE_MAGIC + RE_STRING);
if (regmatch.regprog != NULL)
{
regmatch.rm_ic = ic;
matches = vim_regexec_nl(&regmatch, text, (colnr_T)0);
vim_regfree(regmatch.regprog);
}
p_cpo = save_cpo;
return matches;
}
static int
eval_func(
char_u **arg, 
char_u *name,
int name_len,
typval_T *rettv,
int evaluate,
typval_T *basetv) 
{
char_u *s = name;
int len = name_len;
partial_T *partial;
int ret = OK;
if (!evaluate)
check_vars(s, len);
s = deref_func_name(s, &len, &partial, !evaluate);
s = vim_strsave(s);
if (s == NULL)
ret = FAIL;
else
{
funcexe_T funcexe;
vim_memset(&funcexe, 0, sizeof(funcexe));
funcexe.firstline = curwin->w_cursor.lnum;
funcexe.lastline = curwin->w_cursor.lnum;
funcexe.evaluate = evaluate;
funcexe.partial = partial;
funcexe.basetv = basetv;
ret = get_func_tv(s, len, rettv, arg, &funcexe);
}
vim_free(s);
if (rettv->v_type == VAR_UNKNOWN && !evaluate && **arg == '(')
{
rettv->vval.v_string = NULL;
rettv->v_type = VAR_FUNC;
}
if (evaluate && aborting())
{
if (ret == OK)
clear_tv(rettv);
ret = FAIL;
}
return ret;
}
int
eval0(
char_u *arg,
typval_T *rettv,
char_u **nextcmd,
int evaluate)
{
int ret;
char_u *p;
int did_emsg_before = did_emsg;
int called_emsg_before = called_emsg;
p = skipwhite(arg);
ret = eval1(&p, rettv, evaluate);
if (ret == FAIL || !ends_excmd(*p))
{
if (ret != FAIL)
clear_tv(rettv);
if (!aborting() && did_emsg == did_emsg_before
&& called_emsg == called_emsg_before)
semsg(_(e_invexpr2), arg);
ret = FAIL;
}
if (nextcmd != NULL)
*nextcmd = check_nextcmd(p);
return ret;
}
int
eval1(char_u **arg, typval_T *rettv, int evaluate)
{
int result;
typval_T var2;
if (eval2(arg, rettv, evaluate) == FAIL)
return FAIL;
if ((*arg)[0] == '?')
{
result = FALSE;
if (evaluate)
{
int error = FALSE;
if (tv_get_number_chk(rettv, &error) != 0)
result = TRUE;
clear_tv(rettv);
if (error)
return FAIL;
}
*arg = skipwhite(*arg + 1);
if (eval1(arg, rettv, evaluate && result) == FAIL) 
return FAIL;
if ((*arg)[0] != ':')
{
emsg(_(e_missing_colon));
if (evaluate && result)
clear_tv(rettv);
return FAIL;
}
*arg = skipwhite(*arg + 1);
if (eval1(arg, &var2, evaluate && !result) == FAIL) 
{
if (evaluate && result)
clear_tv(rettv);
return FAIL;
}
if (evaluate && !result)
*rettv = var2;
}
return OK;
}
static int
eval2(char_u **arg, typval_T *rettv, int evaluate)
{
typval_T var2;
long result;
int first;
int error = FALSE;
if (eval3(arg, rettv, evaluate) == FAIL)
return FAIL;
first = TRUE;
result = FALSE;
while ((*arg)[0] == '|' && (*arg)[1] == '|')
{
if (evaluate && first)
{
if (tv_get_number_chk(rettv, &error) != 0)
result = TRUE;
clear_tv(rettv);
if (error)
return FAIL;
first = FALSE;
}
*arg = skipwhite(*arg + 2);
if (eval3(arg, &var2, evaluate && !result) == FAIL)
return FAIL;
if (evaluate && !result)
{
if (tv_get_number_chk(&var2, &error) != 0)
result = TRUE;
clear_tv(&var2);
if (error)
return FAIL;
}
if (evaluate)
{
rettv->v_type = VAR_NUMBER;
rettv->vval.v_number = result;
}
}
return OK;
}
static int
eval3(char_u **arg, typval_T *rettv, int evaluate)
{
typval_T var2;
long result;
int first;
int error = FALSE;
if (eval4(arg, rettv, evaluate) == FAIL)
return FAIL;
first = TRUE;
result = TRUE;
while ((*arg)[0] == '&' && (*arg)[1] == '&')
{
if (evaluate && first)
{
if (tv_get_number_chk(rettv, &error) == 0)
result = FALSE;
clear_tv(rettv);
if (error)
return FAIL;
first = FALSE;
}
*arg = skipwhite(*arg + 2);
if (eval4(arg, &var2, evaluate && result) == FAIL)
return FAIL;
if (evaluate && result)
{
if (tv_get_number_chk(&var2, &error) == 0)
result = FALSE;
clear_tv(&var2);
if (error)
return FAIL;
}
if (evaluate)
{
rettv->v_type = VAR_NUMBER;
rettv->vval.v_number = result;
}
}
return OK;
}
static int
eval4(char_u **arg, typval_T *rettv, int evaluate)
{
typval_T var2;
char_u *p;
int i;
exptype_T type = EXPR_UNKNOWN;
int len = 2;
int ic;
if (eval5(arg, rettv, evaluate) == FAIL)
return FAIL;
p = *arg;
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
len = 1;
}
else
type = EXPR_GEQUAL;
break;
case '<': if (p[1] != '=')
{
type = EXPR_SMALLER;
len = 1;
}
else
type = EXPR_SEQUAL;
break;
case 'i': if (p[1] == 's')
{
if (p[2] == 'n' && p[3] == 'o' && p[4] == 't')
len = 5;
i = p[len];
if (!isalnum(i) && i != '_')
type = len == 2 ? EXPR_IS : EXPR_ISNOT;
}
break;
}
if (type != EXPR_UNKNOWN)
{
if (p[len] == '?')
{
ic = TRUE;
++len;
}
else if (p[len] == '#')
{
ic = FALSE;
++len;
}
else
ic = p_ic;
*arg = skipwhite(p + len);
if (eval5(arg, &var2, evaluate) == FAIL)
{
clear_tv(rettv);
return FAIL;
}
if (evaluate)
{
int ret = typval_compare(rettv, &var2, type, ic);
clear_tv(&var2);
return ret;
}
}
return OK;
}
void
eval_addblob(typval_T *tv1, typval_T *tv2)
{
blob_T *b1 = tv1->vval.v_blob;
blob_T *b2 = tv2->vval.v_blob;
blob_T *b = blob_alloc();
int i;
if (b != NULL)
{
for (i = 0; i < blob_len(b1); i++)
ga_append(&b->bv_ga, blob_get(b1, i));
for (i = 0; i < blob_len(b2); i++)
ga_append(&b->bv_ga, blob_get(b2, i));
clear_tv(tv1);
rettv_blob_set(tv1, b);
}
}
int
eval_addlist(typval_T *tv1, typval_T *tv2)
{
typval_T var3;
if (list_concat(tv1->vval.v_list, tv2->vval.v_list, &var3) == FAIL)
{
clear_tv(tv1);
clear_tv(tv2);
return FAIL;
}
clear_tv(tv1);
*tv1 = var3;
return OK;
}
static int
eval5(char_u **arg, typval_T *rettv, int evaluate)
{
typval_T var2;
int op;
varnumber_T n1, n2;
#if defined(FEAT_FLOAT)
float_T f1 = 0, f2 = 0;
#endif
char_u *s1, *s2;
char_u buf1[NUMBUFLEN], buf2[NUMBUFLEN];
char_u *p;
int concat;
if (eval6(arg, rettv, evaluate, FALSE) == FAIL)
return FAIL;
for (;;)
{
op = **arg;
concat = op == '.'
&& (*(*arg + 1) == '.' || current_sctx.sc_version < 2);
if (op != '+' && op != '-' && !concat)
break;
if ((op != '+' || (rettv->v_type != VAR_LIST
&& rettv->v_type != VAR_BLOB))
#if defined(FEAT_FLOAT)
&& (op == '.' || rettv->v_type != VAR_FLOAT)
#endif
)
{
if (evaluate && tv_get_string_chk(rettv) == NULL)
{
clear_tv(rettv);
return FAIL;
}
}
if (op == '.' && *(*arg + 1) == '.') 
++*arg;
*arg = skipwhite(*arg + 1);
if (eval6(arg, &var2, evaluate, op == '.') == FAIL)
{
clear_tv(rettv);
return FAIL;
}
if (evaluate)
{
if (op == '.')
{
s1 = tv_get_string_buf(rettv, buf1); 
s2 = tv_get_string_buf_chk(&var2, buf2);
if (s2 == NULL) 
{
clear_tv(rettv);
clear_tv(&var2);
return FAIL;
}
p = concat_str(s1, s2);
clear_tv(rettv);
rettv->v_type = VAR_STRING;
rettv->vval.v_string = p;
}
else if (op == '+' && rettv->v_type == VAR_BLOB
&& var2.v_type == VAR_BLOB)
eval_addblob(rettv, &var2);
else if (op == '+' && rettv->v_type == VAR_LIST
&& var2.v_type == VAR_LIST)
{
if (eval_addlist(rettv, &var2) == FAIL)
return FAIL;
}
else
{
int error = FALSE;
#if defined(FEAT_FLOAT)
if (rettv->v_type == VAR_FLOAT)
{
f1 = rettv->vval.v_float;
n1 = 0;
}
else
#endif
{
n1 = tv_get_number_chk(rettv, &error);
if (error)
{
clear_tv(rettv);
return FAIL;
}
#if defined(FEAT_FLOAT)
if (var2.v_type == VAR_FLOAT)
f1 = n1;
#endif
}
#if defined(FEAT_FLOAT)
if (var2.v_type == VAR_FLOAT)
{
f2 = var2.vval.v_float;
n2 = 0;
}
else
#endif
{
n2 = tv_get_number_chk(&var2, &error);
if (error)
{
clear_tv(rettv);
clear_tv(&var2);
return FAIL;
}
#if defined(FEAT_FLOAT)
if (rettv->v_type == VAR_FLOAT)
f2 = n2;
#endif
}
clear_tv(rettv);
#if defined(FEAT_FLOAT)
if (rettv->v_type == VAR_FLOAT || var2.v_type == VAR_FLOAT)
{
if (op == '+')
f1 = f1 + f2;
else
f1 = f1 - f2;
rettv->v_type = VAR_FLOAT;
rettv->vval.v_float = f1;
}
else
#endif
{
if (op == '+')
n1 = n1 + n2;
else
n1 = n1 - n2;
rettv->v_type = VAR_NUMBER;
rettv->vval.v_number = n1;
}
}
clear_tv(&var2);
}
}
return OK;
}
static int
eval6(
char_u **arg,
typval_T *rettv,
int evaluate,
int want_string) 
{
typval_T var2;
int op;
varnumber_T n1, n2;
#if defined(FEAT_FLOAT)
int use_float = FALSE;
float_T f1 = 0, f2 = 0;
#endif
int error = FALSE;
if (eval7(arg, rettv, evaluate, want_string) == FAIL)
return FAIL;
for (;;)
{
op = **arg;
if (op != '*' && op != '/' && op != '%')
break;
if (evaluate)
{
#if defined(FEAT_FLOAT)
if (rettv->v_type == VAR_FLOAT)
{
f1 = rettv->vval.v_float;
use_float = TRUE;
n1 = 0;
}
else
#endif
n1 = tv_get_number_chk(rettv, &error);
clear_tv(rettv);
if (error)
return FAIL;
}
else
n1 = 0;
*arg = skipwhite(*arg + 1);
if (eval7(arg, &var2, evaluate, FALSE) == FAIL)
return FAIL;
if (evaluate)
{
#if defined(FEAT_FLOAT)
if (var2.v_type == VAR_FLOAT)
{
if (!use_float)
{
f1 = n1;
use_float = TRUE;
}
f2 = var2.vval.v_float;
n2 = 0;
}
else
#endif
{
n2 = tv_get_number_chk(&var2, &error);
clear_tv(&var2);
if (error)
return FAIL;
#if defined(FEAT_FLOAT)
if (use_float)
f2 = n2;
#endif
}
#if defined(FEAT_FLOAT)
if (use_float)
{
if (op == '*')
f1 = f1 * f2;
else if (op == '/')
{
#if defined(VMS)
if (f2 == 0.0)
{
if (f1 == 0)
f1 = -1 * __F_FLT_MAX - 1L; 
else if (f1 < 0)
f1 = -1 * __F_FLT_MAX;
else
f1 = __F_FLT_MAX;
}
else
f1 = f1 / f2;
#else
f1 = f1 / f2;
#endif
}
else
{
emsg(_(e_modulus));
return FAIL;
}
rettv->v_type = VAR_FLOAT;
rettv->vval.v_float = f1;
}
else
#endif
{
if (op == '*')
n1 = n1 * n2;
else if (op == '/')
n1 = num_divide(n1, n2);
else
n1 = num_modulus(n1, n2);
rettv->v_type = VAR_NUMBER;
rettv->vval.v_number = n1;
}
}
}
return OK;
}
static int
eval7(
char_u **arg,
typval_T *rettv,
int evaluate,
int want_string) 
{
int len;
char_u *s;
char_u *start_leader, *end_leader;
int ret = OK;
char_u *alias;
rettv->v_type = VAR_UNKNOWN;
start_leader = *arg;
while (**arg == '!' || **arg == '-' || **arg == '+')
*arg = skipwhite(*arg + 1);
end_leader = *arg;
if (**arg == '.' && (!isdigit(*(*arg + 1))
#if defined(FEAT_FLOAT)
|| current_sctx.sc_version < 2
#endif
))
{
semsg(_(e_invexpr2), *arg);
++*arg;
return FAIL;
}
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
case '.': ret = get_number_tv(arg, rettv, evaluate, want_string);
break;
case '"': ret = get_string_tv(arg, rettv, evaluate);
break;
case '\'': ret = get_lit_string_tv(arg, rettv, evaluate);
break;
case '[': ret = get_list_tv(arg, rettv, evaluate, TRUE);
break;
case '#': if ((*arg)[1] == '{')
{
++*arg;
ret = eval_dict(arg, rettv, evaluate, TRUE);
}
else
ret = NOTDONE;
break;
case '{': ret = get_lambda_tv(arg, rettv, evaluate);
if (ret == NOTDONE)
ret = eval_dict(arg, rettv, evaluate, FALSE);
break;
case '&': ret = get_option_tv(arg, rettv, evaluate);
break;
case '$': ret = get_env_tv(arg, rettv, evaluate);
break;
case '@': ++*arg;
if (evaluate)
{
rettv->v_type = VAR_STRING;
rettv->vval.v_string = get_reg_contents(**arg,
GREG_EXPR_SRC);
}
if (**arg != NUL)
++*arg;
break;
case '(': *arg = skipwhite(*arg + 1);
ret = eval1(arg, rettv, evaluate); 
if (**arg == ')')
++*arg;
else if (ret == OK)
{
emsg(_(e_missing_close));
clear_tv(rettv);
ret = FAIL;
}
break;
default: ret = NOTDONE;
break;
}
if (ret == NOTDONE)
{
s = *arg;
len = get_name_len(arg, &alias, evaluate, TRUE);
if (alias != NULL)
s = alias;
if (len <= 0)
ret = FAIL;
else
{
if (**arg == '(') 
ret = eval_func(arg, s, len, rettv, evaluate, NULL);
else if (evaluate)
ret = get_var_tv(s, len, rettv, NULL, TRUE, FALSE);
else
{
check_vars(s, len);
ret = OK;
}
}
vim_free(alias);
}
*arg = skipwhite(*arg);
if (ret == OK)
ret = handle_subscript(arg, rettv, evaluate, TRUE,
start_leader, &end_leader);
if (ret == OK && evaluate && end_leader > start_leader)
ret = eval7_leader(rettv, start_leader, &end_leader);
return ret;
}
static int
eval7_leader(typval_T *rettv, char_u *start_leader, char_u **end_leaderp)
{
char_u *end_leader = *end_leaderp;
int ret = OK;
int error = FALSE;
varnumber_T val = 0;
#if defined(FEAT_FLOAT)
float_T f = 0.0;
if (rettv->v_type == VAR_FLOAT)
f = rettv->vval.v_float;
else
#endif
val = tv_get_number_chk(rettv, &error);
if (error)
{
clear_tv(rettv);
ret = FAIL;
}
else
{
while (end_leader > start_leader)
{
--end_leader;
if (*end_leader == '!')
{
#if defined(FEAT_FLOAT)
if (rettv->v_type == VAR_FLOAT)
f = !f;
else
#endif
val = !val;
}
else if (*end_leader == '-')
{
#if defined(FEAT_FLOAT)
if (rettv->v_type == VAR_FLOAT)
f = -f;
else
#endif
val = -val;
}
}
#if defined(FEAT_FLOAT)
if (rettv->v_type == VAR_FLOAT)
{
clear_tv(rettv);
rettv->vval.v_float = f;
}
else
#endif
{
clear_tv(rettv);
rettv->v_type = VAR_NUMBER;
rettv->vval.v_number = val;
}
}
*end_leaderp = end_leader;
return ret;
}
static int
call_func_rettv(
char_u **arg,
typval_T *rettv,
int evaluate,
dict_T *selfdict,
typval_T *basetv)
{
partial_T *pt = NULL;
funcexe_T funcexe;
typval_T functv;
char_u *s;
int ret;
if (evaluate)
{
functv = *rettv;
rettv->v_type = VAR_UNKNOWN;
if (functv.v_type == VAR_PARTIAL)
{
pt = functv.vval.v_partial;
s = partial_name(pt);
}
else
s = functv.vval.v_string;
}
else
s = (char_u *)"";
vim_memset(&funcexe, 0, sizeof(funcexe));
funcexe.firstline = curwin->w_cursor.lnum;
funcexe.lastline = curwin->w_cursor.lnum;
funcexe.evaluate = evaluate;
funcexe.partial = pt;
funcexe.selfdict = selfdict;
funcexe.basetv = basetv;
ret = get_func_tv(s, -1, rettv, arg, &funcexe);
if (evaluate)
clear_tv(&functv);
return ret;
}
static int
eval_lambda(
char_u **arg,
typval_T *rettv,
int evaluate,
int verbose) 
{
typval_T base = *rettv;
int ret;
*arg += 2;
rettv->v_type = VAR_UNKNOWN;
ret = get_lambda_tv(arg, rettv, evaluate);
if (ret != OK)
return FAIL;
else if (**arg != '(')
{
if (verbose)
{
if (*skipwhite(*arg) == '(')
emsg(_(e_nowhitespace));
else
semsg(_(e_missing_paren), "lambda");
}
clear_tv(rettv);
ret = FAIL;
}
else
ret = call_func_rettv(arg, rettv, evaluate, NULL, &base);
if (evaluate)
clear_tv(&base);
return ret;
}
static int
eval_method(
char_u **arg,
typval_T *rettv,
int evaluate,
int verbose) 
{
char_u *name;
long len;
char_u *alias;
typval_T base = *rettv;
int ret;
*arg += 2;
rettv->v_type = VAR_UNKNOWN;
name = *arg;
len = get_name_len(arg, &alias, evaluate, TRUE);
if (alias != NULL)
name = alias;
if (len <= 0)
{
if (verbose)
emsg(_("E260: Missing name after ->"));
ret = FAIL;
}
else
{
if (**arg != '(')
{
if (verbose)
semsg(_(e_missing_paren), name);
ret = FAIL;
}
else if (VIM_ISWHITE((*arg)[-1]))
{
if (verbose)
emsg(_(e_nowhitespace));
ret = FAIL;
}
else
ret = eval_func(arg, name, len, rettv, evaluate, &base);
}
if (evaluate)
clear_tv(&base);
return ret;
}
static int
eval_index(
char_u **arg,
typval_T *rettv,
int evaluate,
int verbose) 
{
int empty1 = FALSE, empty2 = FALSE;
typval_T var1, var2;
long i;
long n1, n2 = 0;
long len = -1;
int range = FALSE;
char_u *s;
char_u *key = NULL;
switch (rettv->v_type)
{
case VAR_FUNC:
case VAR_PARTIAL:
if (verbose)
emsg(_("E695: Cannot index a Funcref"));
return FAIL;
case VAR_FLOAT:
#if defined(FEAT_FLOAT)
if (verbose)
emsg(_(e_float_as_string));
return FAIL;
#endif
case VAR_BOOL:
case VAR_SPECIAL:
case VAR_JOB:
case VAR_CHANNEL:
if (verbose)
emsg(_("E909: Cannot index a special variable"));
return FAIL;
case VAR_UNKNOWN:
case VAR_VOID:
if (evaluate)
return FAIL;
case VAR_STRING:
case VAR_NUMBER:
case VAR_LIST:
case VAR_DICT:
case VAR_BLOB:
break;
}
init_tv(&var1);
init_tv(&var2);
if (**arg == '.')
{
key = *arg + 1;
for (len = 0; ASCII_ISALNUM(key[len]) || key[len] == '_'; ++len)
;
if (len == 0)
return FAIL;
*arg = skipwhite(key + len);
}
else
{
*arg = skipwhite(*arg + 1);
if (**arg == ':')
empty1 = TRUE;
else if (eval1(arg, &var1, evaluate) == FAIL) 
return FAIL;
else if (evaluate && tv_get_string_chk(&var1) == NULL)
{
clear_tv(&var1);
return FAIL;
}
if (**arg == ':')
{
range = TRUE;
*arg = skipwhite(*arg + 1);
if (**arg == ']')
empty2 = TRUE;
else if (eval1(arg, &var2, evaluate) == FAIL) 
{
if (!empty1)
clear_tv(&var1);
return FAIL;
}
else if (evaluate && tv_get_string_chk(&var2) == NULL)
{
if (!empty1)
clear_tv(&var1);
clear_tv(&var2);
return FAIL;
}
}
if (**arg != ']')
{
if (verbose)
emsg(_(e_missbrac));
clear_tv(&var1);
if (range)
clear_tv(&var2);
return FAIL;
}
*arg = skipwhite(*arg + 1); 
}
if (evaluate)
{
n1 = 0;
if (!empty1 && rettv->v_type != VAR_DICT)
{
n1 = tv_get_number(&var1);
clear_tv(&var1);
}
if (range)
{
if (empty2)
n2 = -1;
else
{
n2 = tv_get_number(&var2);
clear_tv(&var2);
}
}
switch (rettv->v_type)
{
case VAR_UNKNOWN:
case VAR_VOID:
case VAR_FUNC:
case VAR_PARTIAL:
case VAR_FLOAT:
case VAR_BOOL:
case VAR_SPECIAL:
case VAR_JOB:
case VAR_CHANNEL:
break; 
case VAR_NUMBER:
case VAR_STRING:
s = tv_get_string(rettv);
len = (long)STRLEN(s);
if (range)
{
if (n1 < 0)
{
n1 = len + n1;
if (n1 < 0)
n1 = 0;
}
if (n2 < 0)
n2 = len + n2;
else if (n2 >= len)
n2 = len;
if (n1 >= len || n2 < 0 || n1 > n2)
s = NULL;
else
s = vim_strnsave(s + n1, (int)(n2 - n1 + 1));
}
else
{
if (n1 >= len || n1 < 0)
s = NULL;
else
s = vim_strnsave(s + n1, 1);
}
clear_tv(rettv);
rettv->v_type = VAR_STRING;
rettv->vval.v_string = s;
break;
case VAR_BLOB:
len = blob_len(rettv->vval.v_blob);
if (range)
{
if (n1 < 0)
{
n1 = len + n1;
if (n1 < 0)
n1 = 0;
}
if (n2 < 0)
n2 = len + n2;
else if (n2 >= len)
n2 = len - 1;
if (n1 >= len || n2 < 0 || n1 > n2)
{
clear_tv(rettv);
rettv->v_type = VAR_BLOB;
rettv->vval.v_blob = NULL;
}
else
{
blob_T *blob = blob_alloc();
if (blob != NULL)
{
if (ga_grow(&blob->bv_ga, n2 - n1 + 1) == FAIL)
{
blob_free(blob);
return FAIL;
}
blob->bv_ga.ga_len = n2 - n1 + 1;
for (i = n1; i <= n2; i++)
blob_set(blob, i - n1,
blob_get(rettv->vval.v_blob, i));
clear_tv(rettv);
rettv_blob_set(rettv, blob);
}
}
}
else
{
if (n1 < 0)
n1 = len + n1;
if (n1 < len && n1 >= 0)
{
int v = blob_get(rettv->vval.v_blob, n1);
clear_tv(rettv);
rettv->v_type = VAR_NUMBER;
rettv->vval.v_number = v;
}
else
semsg(_(e_blobidx), n1);
}
break;
case VAR_LIST:
len = list_len(rettv->vval.v_list);
if (n1 < 0)
n1 = len + n1;
if (!empty1 && (n1 < 0 || n1 >= len))
{
if (!range)
{
if (verbose)
semsg(_(e_listidx), n1);
return FAIL;
}
n1 = len;
}
if (range)
{
list_T *l;
listitem_T *item;
if (n2 < 0)
n2 = len + n2;
else if (n2 >= len)
n2 = len - 1;
if (!empty2 && (n2 < 0 || n2 + 1 < n1))
n2 = -1;
l = list_alloc();
if (l == NULL)
return FAIL;
for (item = list_find(rettv->vval.v_list, n1);
n1 <= n2; ++n1)
{
if (list_append_tv(l, &item->li_tv) == FAIL)
{
list_free(l);
return FAIL;
}
item = item->li_next;
}
clear_tv(rettv);
rettv_list_set(rettv, l);
}
else
{
copy_tv(&list_find(rettv->vval.v_list, n1)->li_tv, &var1);
clear_tv(rettv);
*rettv = var1;
}
break;
case VAR_DICT:
if (range)
{
if (verbose)
emsg(_(e_dictrange));
if (len == -1)
clear_tv(&var1);
return FAIL;
}
{
dictitem_T *item;
if (len == -1)
{
key = tv_get_string_chk(&var1);
if (key == NULL)
{
clear_tv(&var1);
return FAIL;
}
}
item = dict_find(rettv->vval.v_dict, key, (int)len);
if (item == NULL && verbose)
semsg(_(e_dictkey), key);
if (len == -1)
clear_tv(&var1);
if (item == NULL)
return FAIL;
copy_tv(&item->di_tv, &var1);
clear_tv(rettv);
*rettv = var1;
}
break;
}
}
return OK;
}
int
get_option_tv(
char_u **arg,
typval_T *rettv, 
int evaluate)
{
char_u *option_end;
long numval;
char_u *stringval;
int opt_type;
int c;
int working = (**arg == '+'); 
int ret = OK;
int opt_flags;
option_end = find_option_end(arg, &opt_flags);
if (option_end == NULL)
{
if (rettv != NULL)
semsg(_("E112: Option name missing: %s"), *arg);
return FAIL;
}
if (!evaluate)
{
*arg = option_end;
return OK;
}
c = *option_end;
*option_end = NUL;
opt_type = get_option_value(*arg, &numval,
rettv == NULL ? NULL : &stringval, opt_flags);
if (opt_type == -3) 
{
if (rettv != NULL)
semsg(_(e_unknown_option), *arg);
ret = FAIL;
}
else if (rettv != NULL)
{
if (opt_type == -2) 
{
rettv->v_type = VAR_STRING;
rettv->vval.v_string = NULL;
}
else if (opt_type == -1) 
{
rettv->v_type = VAR_NUMBER;
rettv->vval.v_number = 0;
}
else if (opt_type == 1) 
{
rettv->v_type = VAR_NUMBER;
rettv->vval.v_number = numval;
}
else 
{
rettv->v_type = VAR_STRING;
rettv->vval.v_string = stringval;
}
}
else if (working && (opt_type == -2 || opt_type == -1))
ret = FAIL;
*option_end = c; 
*arg = option_end;
return ret;
}
int
get_number_tv(
char_u **arg,
typval_T *rettv,
int evaluate,
int want_string UNUSED)
{
int len;
#if defined(FEAT_FLOAT)
char_u *p;
int get_float = FALSE;
if (**arg == '.')
p = *arg;
else
p = skipdigits(*arg + 1);
if (!want_string && p[0] == '.' && vim_isdigit(p[1]))
{
get_float = TRUE;
p = skipdigits(p + 2);
if (*p == 'e' || *p == 'E')
{
++p;
if (*p == '-' || *p == '+')
++p;
if (!vim_isdigit(*p))
get_float = FALSE;
else
p = skipdigits(p + 1);
}
if (ASCII_ISALPHA(*p) || *p == '.')
get_float = FALSE;
}
if (get_float)
{
float_T f;
*arg += string2float(*arg, &f);
if (evaluate)
{
rettv->v_type = VAR_FLOAT;
rettv->vval.v_float = f;
}
}
else
#endif
if (**arg == '0' && ((*arg)[1] == 'z' || (*arg)[1] == 'Z'))
{
char_u *bp;
blob_T *blob = NULL; 
if (evaluate)
blob = blob_alloc();
for (bp = *arg + 2; vim_isxdigit(bp[0]); bp += 2)
{
if (!vim_isxdigit(bp[1]))
{
if (blob != NULL)
{
emsg(_("E973: Blob literal should have an even number of hex characters"));
ga_clear(&blob->bv_ga);
VIM_CLEAR(blob);
}
return FAIL;
}
if (blob != NULL)
ga_append(&blob->bv_ga,
(hex2nr(*bp) << 4) + hex2nr(*(bp+1)));
if (bp[2] == '.' && vim_isxdigit(bp[3]))
++bp;
}
if (blob != NULL)
rettv_blob_set(rettv, blob);
*arg = bp;
}
else
{
varnumber_T n;
vim_str2nr(*arg, NULL, &len, current_sctx.sc_version >= 4
? STR2NR_NO_OCT + STR2NR_QUOTE
: STR2NR_ALL, &n, NULL, 0, TRUE);
if (len == 0)
{
semsg(_(e_invexpr2), *arg);
return FAIL;
}
*arg += len;
if (evaluate)
{
rettv->v_type = VAR_NUMBER;
rettv->vval.v_number = n;
}
}
return OK;
}
int
get_string_tv(char_u **arg, typval_T *rettv, int evaluate)
{
char_u *p;
char_u *name;
int extra = 0;
for (p = *arg + 1; *p != NUL && *p != '"'; MB_PTR_ADV(p))
{
if (*p == '\\' && p[1] != NUL)
{
++p;
if (*p == '<')
extra += 2;
}
}
if (*p != '"')
{
semsg(_("E114: Missing quote: %s"), *arg);
return FAIL;
}
if (!evaluate)
{
*arg = p + 1;
return OK;
}
name = alloc(p - *arg + extra);
if (name == NULL)
return FAIL;
rettv->v_type = VAR_STRING;
rettv->vval.v_string = name;
for (p = *arg + 1; *p != NUL && *p != '"'; )
{
if (*p == '\\')
{
switch (*++p)
{
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
if (vim_isxdigit(p[1]))
{
int n, nr;
int c = toupper(*p);
if (c == 'X')
n = 2;
else if (*p == 'u')
n = 4;
else
n = 8;
nr = 0;
while (--n >= 0 && vim_isxdigit(p[1]))
{
++p;
nr = (nr << 4) + hex2nr(*p);
}
++p;
if (c != 'X')
name += (*mb_char2bytes)(nr, name);
else
*name++ = nr;
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
if (*p >= '0' && *p <= '7')
{
*name = (*name << 3) + *p++ - '0';
if (*p >= '0' && *p <= '7')
*name = (*name << 3) + *p++ - '0';
}
++name;
break;
case '<': extra = trans_special(&p, name, TRUE, TRUE,
TRUE, NULL);
if (extra != 0)
{
name += extra;
break;
}
default: MB_COPY_CHAR(p, name);
break;
}
}
else
MB_COPY_CHAR(p, name);
}
*name = NUL;
if (*p != NUL) 
++p;
*arg = p;
return OK;
}
int
get_lit_string_tv(char_u **arg, typval_T *rettv, int evaluate)
{
char_u *p;
char_u *str;
int reduce = 0;
for (p = *arg + 1; *p != NUL; MB_PTR_ADV(p))
{
if (*p == '\'')
{
if (p[1] != '\'')
break;
++reduce;
++p;
}
}
if (*p != '\'')
{
semsg(_("E115: Missing quote: %s"), *arg);
return FAIL;
}
if (!evaluate)
{
*arg = p + 1;
return OK;
}
str = alloc((p - *arg) - reduce);
if (str == NULL)
return FAIL;
rettv->v_type = VAR_STRING;
rettv->vval.v_string = str;
for (p = *arg + 1; *p != NUL; )
{
if (*p == '\'')
{
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
char_u *
partial_name(partial_T *pt)
{
if (pt->pt_name != NULL)
return pt->pt_name;
return pt->pt_func->uf_name;
}
static void
partial_free(partial_T *pt)
{
int i;
for (i = 0; i < pt->pt_argc; ++i)
clear_tv(&pt->pt_argv[i]);
vim_free(pt->pt_argv);
dict_unref(pt->pt_dict);
if (pt->pt_name != NULL)
{
func_unref(pt->pt_name);
vim_free(pt->pt_name);
}
else
func_ptr_unref(pt->pt_func);
vim_free(pt);
}
void
partial_unref(partial_T *pt)
{
if (pt != NULL && --pt->pt_refcount <= 0)
partial_free(pt);
}
static int tv_equal_recurse_limit;
static int
func_equal(
typval_T *tv1,
typval_T *tv2,
int ic) 
{
char_u *s1, *s2;
dict_T *d1, *d2;
int a1, a2;
int i;
s1 = tv1->v_type == VAR_FUNC ? tv1->vval.v_string
: partial_name(tv1->vval.v_partial);
if (s1 != NULL && *s1 == NUL)
s1 = NULL;
s2 = tv2->v_type == VAR_FUNC ? tv2->vval.v_string
: partial_name(tv2->vval.v_partial);
if (s2 != NULL && *s2 == NUL)
s2 = NULL;
if (s1 == NULL || s2 == NULL)
{
if (s1 != s2)
return FALSE;
}
else if (STRCMP(s1, s2) != 0)
return FALSE;
d1 = tv1->v_type == VAR_FUNC ? NULL : tv1->vval.v_partial->pt_dict;
d2 = tv2->v_type == VAR_FUNC ? NULL : tv2->vval.v_partial->pt_dict;
if (d1 == NULL || d2 == NULL)
{
if (d1 != d2)
return FALSE;
}
else if (!dict_equal(d1, d2, ic, TRUE))
return FALSE;
a1 = tv1->v_type == VAR_FUNC ? 0 : tv1->vval.v_partial->pt_argc;
a2 = tv2->v_type == VAR_FUNC ? 0 : tv2->vval.v_partial->pt_argc;
if (a1 != a2)
return FALSE;
for (i = 0; i < a1; ++i)
if (!tv_equal(tv1->vval.v_partial->pt_argv + i,
tv2->vval.v_partial->pt_argv + i, ic, TRUE))
return FALSE;
return TRUE;
}
int
tv_equal(
typval_T *tv1,
typval_T *tv2,
int ic, 
int recursive) 
{
char_u buf1[NUMBUFLEN], buf2[NUMBUFLEN];
char_u *s1, *s2;
static int recursive_cnt = 0; 
int r;
if (!recursive)
tv_equal_recurse_limit = 1000;
if (recursive_cnt >= tv_equal_recurse_limit)
{
--tv_equal_recurse_limit;
return TRUE;
}
if ((tv1->v_type == VAR_FUNC
|| (tv1->v_type == VAR_PARTIAL && tv1->vval.v_partial != NULL))
&& (tv2->v_type == VAR_FUNC
|| (tv2->v_type == VAR_PARTIAL && tv2->vval.v_partial != NULL)))
{
++recursive_cnt;
r = func_equal(tv1, tv2, ic);
--recursive_cnt;
return r;
}
if (tv1->v_type != tv2->v_type)
return FALSE;
switch (tv1->v_type)
{
case VAR_LIST:
++recursive_cnt;
r = list_equal(tv1->vval.v_list, tv2->vval.v_list, ic, TRUE);
--recursive_cnt;
return r;
case VAR_DICT:
++recursive_cnt;
r = dict_equal(tv1->vval.v_dict, tv2->vval.v_dict, ic, TRUE);
--recursive_cnt;
return r;
case VAR_BLOB:
return blob_equal(tv1->vval.v_blob, tv2->vval.v_blob);
case VAR_NUMBER:
case VAR_BOOL:
case VAR_SPECIAL:
return tv1->vval.v_number == tv2->vval.v_number;
case VAR_STRING:
s1 = tv_get_string_buf(tv1, buf1);
s2 = tv_get_string_buf(tv2, buf2);
return ((ic ? MB_STRICMP(s1, s2) : STRCMP(s1, s2)) == 0);
case VAR_FLOAT:
#if defined(FEAT_FLOAT)
return tv1->vval.v_float == tv2->vval.v_float;
#endif
case VAR_JOB:
#if defined(FEAT_JOB_CHANNEL)
return tv1->vval.v_job == tv2->vval.v_job;
#endif
case VAR_CHANNEL:
#if defined(FEAT_JOB_CHANNEL)
return tv1->vval.v_channel == tv2->vval.v_channel;
#endif
case VAR_PARTIAL:
return tv1->vval.v_partial == tv2->vval.v_partial;
case VAR_FUNC:
return tv1->vval.v_string == tv2->vval.v_string;
case VAR_UNKNOWN:
case VAR_VOID:
break;
}
return FALSE;
}
int
get_copyID(void)
{
current_copyID += COPYID_INC;
return current_copyID;
}
int
garbage_collect(int testing)
{
int copyID;
int abort = FALSE;
buf_T *buf;
win_T *wp;
int did_free = FALSE;
tabpage_T *tp;
if (!testing)
{
want_garbage_collect = FALSE;
may_garbage_collect = FALSE;
garbage_collect_at_exit = FALSE;
}
if (exestack.ga_maxlen - exestack.ga_len > 500)
{
size_t new_len;
char_u *pp;
int n;
n = exestack.ga_len / 2;
if (n < exestack.ga_growsize)
n = exestack.ga_growsize;
if (exestack.ga_len + n < exestack.ga_maxlen)
{
new_len = exestack.ga_itemsize * (exestack.ga_len + n);
pp = vim_realloc(exestack.ga_data, new_len);
if (pp == NULL)
return FAIL;
exestack.ga_maxlen = exestack.ga_len + n;
exestack.ga_data = pp;
}
}
copyID = get_copyID();
abort = abort || set_ref_in_previous_funccal(copyID);
abort = abort || garbage_collect_scriptvars(copyID);
FOR_ALL_BUFFERS(buf)
abort = abort || set_ref_in_item(&buf->b_bufvar.di_tv, copyID,
NULL, NULL);
FOR_ALL_TAB_WINDOWS(tp, wp)
abort = abort || set_ref_in_item(&wp->w_winvar.di_tv, copyID,
NULL, NULL);
if (aucmd_win != NULL)
abort = abort || set_ref_in_item(&aucmd_win->w_winvar.di_tv, copyID,
NULL, NULL);
#if defined(FEAT_PROP_POPUP)
FOR_ALL_POPUPWINS(wp)
abort = abort || set_ref_in_item(&wp->w_winvar.di_tv, copyID,
NULL, NULL);
FOR_ALL_TABPAGES(tp)
FOR_ALL_POPUPWINS_IN_TAB(tp, wp)
abort = abort || set_ref_in_item(&wp->w_winvar.di_tv, copyID,
NULL, NULL);
#endif
FOR_ALL_TABPAGES(tp)
abort = abort || set_ref_in_item(&tp->tp_winvar.di_tv, copyID,
NULL, NULL);
abort = abort || garbage_collect_globvars(copyID);
abort = abort || set_ref_in_call_stack(copyID);
abort = abort || set_ref_in_functions(copyID);
abort = abort || set_ref_in_func_args(copyID);
abort = abort || garbage_collect_vimvars(copyID);
abort = abort || set_ref_in_buffers(copyID);
#if defined(FEAT_LUA)
abort = abort || set_ref_in_lua(copyID);
#endif
#if defined(FEAT_PYTHON)
abort = abort || set_ref_in_python(copyID);
#endif
#if defined(FEAT_PYTHON3)
abort = abort || set_ref_in_python3(copyID);
#endif
#if defined(FEAT_JOB_CHANNEL)
abort = abort || set_ref_in_channel(copyID);
abort = abort || set_ref_in_job(copyID);
#endif
#if defined(FEAT_NETBEANS_INTG)
abort = abort || set_ref_in_nb_channel(copyID);
#endif
#if defined(FEAT_TIMERS)
abort = abort || set_ref_in_timer(copyID);
#endif
#if defined(FEAT_QUICKFIX)
abort = abort || set_ref_in_quickfix(copyID);
#endif
#if defined(FEAT_TERMINAL)
abort = abort || set_ref_in_term(copyID);
#endif
#if defined(FEAT_PROP_POPUP)
abort = abort || set_ref_in_popups(copyID);
#endif
if (!abort)
{
did_free = free_unref_items(copyID);
free_unref_funccal(copyID, testing);
}
else if (p_verbose > 0)
{
verb_msg(_("Not enough memory to set references, garbage collection aborted!"));
}
return did_free;
}
static int
free_unref_items(int copyID)
{
int did_free = FALSE;
in_free_unref_items = TRUE;
did_free |= dict_free_nonref(copyID);
did_free |= list_free_nonref(copyID);
#if defined(FEAT_JOB_CHANNEL)
did_free |= free_unused_jobs_contents(copyID, COPYID_MASK);
did_free |= free_unused_channels_contents(copyID, COPYID_MASK);
#endif
dict_free_items(copyID);
list_free_items(copyID);
#if defined(FEAT_JOB_CHANNEL)
free_unused_jobs(copyID, COPYID_MASK);
free_unused_channels(copyID, COPYID_MASK);
#endif
in_free_unref_items = FALSE;
return did_free;
}
int
set_ref_in_ht(hashtab_T *ht, int copyID, list_stack_T **list_stack)
{
int todo;
int abort = FALSE;
hashitem_T *hi;
hashtab_T *cur_ht;
ht_stack_T *ht_stack = NULL;
ht_stack_T *tempitem;
cur_ht = ht;
for (;;)
{
if (!abort)
{
todo = (int)cur_ht->ht_used;
for (hi = cur_ht->ht_array; todo > 0; ++hi)
if (!HASHITEM_EMPTY(hi))
{
--todo;
abort = abort || set_ref_in_item(&HI2DI(hi)->di_tv, copyID,
&ht_stack, list_stack);
}
}
if (ht_stack == NULL)
break;
cur_ht = ht_stack->ht;
tempitem = ht_stack;
ht_stack = ht_stack->prev;
free(tempitem);
}
return abort;
}
int
set_ref_in_dict(dict_T *d, int copyID)
{
if (d != NULL && d->dv_copyID != copyID)
{
d->dv_copyID = copyID;
return set_ref_in_ht(&d->dv_hashtab, copyID, NULL);
}
return FALSE;
}
int
set_ref_in_list(list_T *ll, int copyID)
{
if (ll != NULL && ll->lv_copyID != copyID)
{
ll->lv_copyID = copyID;
return set_ref_in_list_items(ll, copyID, NULL);
}
return FALSE;
}
int
set_ref_in_list_items(list_T *l, int copyID, ht_stack_T **ht_stack)
{
listitem_T *li;
int abort = FALSE;
list_T *cur_l;
list_stack_T *list_stack = NULL;
list_stack_T *tempitem;
cur_l = l;
for (;;)
{
if (!abort && cur_l->lv_first != &range_list_item)
for (li = cur_l->lv_first; !abort && li != NULL; li = li->li_next)
abort = abort || set_ref_in_item(&li->li_tv, copyID,
ht_stack, &list_stack);
if (list_stack == NULL)
break;
cur_l = list_stack->list;
tempitem = list_stack;
list_stack = list_stack->prev;
free(tempitem);
}
return abort;
}
int
set_ref_in_item(
typval_T *tv,
int copyID,
ht_stack_T **ht_stack,
list_stack_T **list_stack)
{
int abort = FALSE;
if (tv->v_type == VAR_DICT)
{
dict_T *dd = tv->vval.v_dict;
if (dd != NULL && dd->dv_copyID != copyID)
{
dd->dv_copyID = copyID;
if (ht_stack == NULL)
{
abort = set_ref_in_ht(&dd->dv_hashtab, copyID, list_stack);
}
else
{
ht_stack_T *newitem = (ht_stack_T*)malloc(sizeof(ht_stack_T));
if (newitem == NULL)
abort = TRUE;
else
{
newitem->ht = &dd->dv_hashtab;
newitem->prev = *ht_stack;
*ht_stack = newitem;
}
}
}
}
else if (tv->v_type == VAR_LIST)
{
list_T *ll = tv->vval.v_list;
if (ll != NULL && ll->lv_copyID != copyID)
{
ll->lv_copyID = copyID;
if (list_stack == NULL)
{
abort = set_ref_in_list_items(ll, copyID, ht_stack);
}
else
{
list_stack_T *newitem = (list_stack_T*)malloc(
sizeof(list_stack_T));
if (newitem == NULL)
abort = TRUE;
else
{
newitem->list = ll;
newitem->prev = *list_stack;
*list_stack = newitem;
}
}
}
}
else if (tv->v_type == VAR_FUNC)
{
abort = set_ref_in_func(tv->vval.v_string, NULL, copyID);
}
else if (tv->v_type == VAR_PARTIAL)
{
partial_T *pt = tv->vval.v_partial;
int i;
if (pt != NULL)
{
abort = set_ref_in_func(pt->pt_name, pt->pt_func, copyID);
if (pt->pt_dict != NULL)
{
typval_T dtv;
dtv.v_type = VAR_DICT;
dtv.vval.v_dict = pt->pt_dict;
set_ref_in_item(&dtv, copyID, ht_stack, list_stack);
}
for (i = 0; i < pt->pt_argc; ++i)
abort = abort || set_ref_in_item(&pt->pt_argv[i], copyID,
ht_stack, list_stack);
}
}
#if defined(FEAT_JOB_CHANNEL)
else if (tv->v_type == VAR_JOB)
{
job_T *job = tv->vval.v_job;
typval_T dtv;
if (job != NULL && job->jv_copyID != copyID)
{
job->jv_copyID = copyID;
if (job->jv_channel != NULL)
{
dtv.v_type = VAR_CHANNEL;
dtv.vval.v_channel = job->jv_channel;
set_ref_in_item(&dtv, copyID, ht_stack, list_stack);
}
if (job->jv_exit_cb.cb_partial != NULL)
{
dtv.v_type = VAR_PARTIAL;
dtv.vval.v_partial = job->jv_exit_cb.cb_partial;
set_ref_in_item(&dtv, copyID, ht_stack, list_stack);
}
}
}
else if (tv->v_type == VAR_CHANNEL)
{
channel_T *ch =tv->vval.v_channel;
ch_part_T part;
typval_T dtv;
jsonq_T *jq;
cbq_T *cq;
if (ch != NULL && ch->ch_copyID != copyID)
{
ch->ch_copyID = copyID;
for (part = PART_SOCK; part < PART_COUNT; ++part)
{
for (jq = ch->ch_part[part].ch_json_head.jq_next; jq != NULL;
jq = jq->jq_next)
set_ref_in_item(jq->jq_value, copyID, ht_stack, list_stack);
for (cq = ch->ch_part[part].ch_cb_head.cq_next; cq != NULL;
cq = cq->cq_next)
if (cq->cq_callback.cb_partial != NULL)
{
dtv.v_type = VAR_PARTIAL;
dtv.vval.v_partial = cq->cq_callback.cb_partial;
set_ref_in_item(&dtv, copyID, ht_stack, list_stack);
}
if (ch->ch_part[part].ch_callback.cb_partial != NULL)
{
dtv.v_type = VAR_PARTIAL;
dtv.vval.v_partial =
ch->ch_part[part].ch_callback.cb_partial;
set_ref_in_item(&dtv, copyID, ht_stack, list_stack);
}
}
if (ch->ch_callback.cb_partial != NULL)
{
dtv.v_type = VAR_PARTIAL;
dtv.vval.v_partial = ch->ch_callback.cb_partial;
set_ref_in_item(&dtv, copyID, ht_stack, list_stack);
}
if (ch->ch_close_cb.cb_partial != NULL)
{
dtv.v_type = VAR_PARTIAL;
dtv.vval.v_partial = ch->ch_close_cb.cb_partial;
set_ref_in_item(&dtv, copyID, ht_stack, list_stack);
}
}
}
#endif
return abort;
}
char_u *
echo_string_core(
typval_T *tv,
char_u **tofree,
char_u *numbuf,
int copyID,
int echo_style,
int restore_copyID,
int composite_val)
{
static int recurse = 0;
char_u *r = NULL;
if (recurse >= DICT_MAXNEST)
{
if (!did_echo_string_emsg)
{
did_echo_string_emsg = TRUE;
emsg(_("E724: variable nested too deep for displaying"));
}
*tofree = NULL;
return (char_u *)"{E724}";
}
++recurse;
switch (tv->v_type)
{
case VAR_STRING:
if (echo_style && !composite_val)
{
*tofree = NULL;
r = tv->vval.v_string;
if (r == NULL)
r = (char_u *)"";
}
else
{
*tofree = string_quote(tv->vval.v_string, FALSE);
r = *tofree;
}
break;
case VAR_FUNC:
if (echo_style)
{
*tofree = NULL;
r = tv->vval.v_string;
}
else
{
*tofree = string_quote(tv->vval.v_string, TRUE);
r = *tofree;
}
break;
case VAR_PARTIAL:
{
partial_T *pt = tv->vval.v_partial;
char_u *fname = string_quote(pt == NULL ? NULL
: partial_name(pt), FALSE);
garray_T ga;
int i;
char_u *tf;
ga_init2(&ga, 1, 100);
ga_concat(&ga, (char_u *)"function(");
if (fname != NULL)
{
ga_concat(&ga, fname);
vim_free(fname);
}
if (pt != NULL && pt->pt_argc > 0)
{
ga_concat(&ga, (char_u *)", [");
for (i = 0; i < pt->pt_argc; ++i)
{
if (i > 0)
ga_concat(&ga, (char_u *)", ");
ga_concat(&ga,
tv2string(&pt->pt_argv[i], &tf, numbuf, copyID));
vim_free(tf);
}
ga_concat(&ga, (char_u *)"]");
}
if (pt != NULL && pt->pt_dict != NULL)
{
typval_T dtv;
ga_concat(&ga, (char_u *)", ");
dtv.v_type = VAR_DICT;
dtv.vval.v_dict = pt->pt_dict;
ga_concat(&ga, tv2string(&dtv, &tf, numbuf, copyID));
vim_free(tf);
}
ga_concat(&ga, (char_u *)")");
*tofree = ga.ga_data;
r = *tofree;
break;
}
case VAR_BLOB:
r = blob2string(tv->vval.v_blob, tofree, numbuf);
break;
case VAR_LIST:
if (tv->vval.v_list == NULL)
{
*tofree = NULL;
r = NULL;
}
else if (copyID != 0 && tv->vval.v_list->lv_copyID == copyID
&& tv->vval.v_list->lv_len > 0)
{
*tofree = NULL;
r = (char_u *)"[...]";
}
else
{
int old_copyID = tv->vval.v_list->lv_copyID;
tv->vval.v_list->lv_copyID = copyID;
*tofree = list2string(tv, copyID, restore_copyID);
if (restore_copyID)
tv->vval.v_list->lv_copyID = old_copyID;
r = *tofree;
}
break;
case VAR_DICT:
if (tv->vval.v_dict == NULL)
{
*tofree = NULL;
r = NULL;
}
else if (copyID != 0 && tv->vval.v_dict->dv_copyID == copyID
&& tv->vval.v_dict->dv_hashtab.ht_used != 0)
{
*tofree = NULL;
r = (char_u *)"{...}";
}
else
{
int old_copyID = tv->vval.v_dict->dv_copyID;
tv->vval.v_dict->dv_copyID = copyID;
*tofree = dict2string(tv, copyID, restore_copyID);
if (restore_copyID)
tv->vval.v_dict->dv_copyID = old_copyID;
r = *tofree;
}
break;
case VAR_NUMBER:
case VAR_UNKNOWN:
case VAR_VOID:
*tofree = NULL;
r = tv_get_string_buf(tv, numbuf);
break;
case VAR_JOB:
case VAR_CHANNEL:
*tofree = NULL;
r = tv_get_string_buf(tv, numbuf);
if (composite_val)
{
*tofree = string_quote(r, FALSE);
r = *tofree;
}
break;
case VAR_FLOAT:
#if defined(FEAT_FLOAT)
*tofree = NULL;
vim_snprintf((char *)numbuf, NUMBUFLEN, "%g", tv->vval.v_float);
r = numbuf;
break;
#endif
case VAR_BOOL:
case VAR_SPECIAL:
*tofree = NULL;
r = (char_u *)get_var_special_name(tv->vval.v_number);
break;
}
if (--recurse == 0)
did_echo_string_emsg = FALSE;
return r;
}
char_u *
echo_string(
typval_T *tv,
char_u **tofree,
char_u *numbuf,
int copyID)
{
return echo_string_core(tv, tofree, numbuf, copyID, TRUE, FALSE, FALSE);
}
char_u *
tv2string(
typval_T *tv,
char_u **tofree,
char_u *numbuf,
int copyID)
{
return echo_string_core(tv, tofree, numbuf, copyID, FALSE, TRUE, FALSE);
}
char_u *
string_quote(char_u *str, int function)
{
unsigned len;
char_u *p, *r, *s;
len = (function ? 13 : 3);
if (str != NULL)
{
len += (unsigned)STRLEN(str);
for (p = str; *p != NUL; MB_PTR_ADV(p))
if (*p == '\'')
++len;
}
s = r = alloc(len);
if (r != NULL)
{
if (function)
{
STRCPY(r, "function('");
r += 10;
}
else
*r++ = '\'';
if (str != NULL)
for (p = str; *p != NUL; )
{
if (*p == '\'')
*r++ = '\'';
MB_COPY_CHAR(p, r);
}
*r++ = '\'';
if (function)
*r++ = ')';
*r++ = NUL;
}
return s;
}
#if defined(FEAT_FLOAT) || defined(PROTO)
int
string2float(
char_u *text,
float_T *value) 
{
char *s = (char *)text;
float_T f;
if (STRNICMP(text, "inf", 3) == 0)
{
*value = INFINITY;
return 3;
}
if (STRNICMP(text, "-inf", 3) == 0)
{
*value = -INFINITY;
return 4;
}
if (STRNICMP(text, "nan", 3) == 0)
{
*value = NAN;
return 3;
}
f = strtod(s, &s);
*value = f;
return (int)((char_u *)s - text);
}
#endif
int
get_env_tv(char_u **arg, typval_T *rettv, int evaluate)
{
char_u *string = NULL;
int len;
int cc;
char_u *name;
int mustfree = FALSE;
++*arg;
name = *arg;
len = get_env_len(arg);
if (evaluate)
{
if (len == 0)
return FAIL; 
cc = name[len];
name[len] = NUL;
string = vim_getenv(name, &mustfree);
if (string != NULL && *string != NUL)
{
if (!mustfree)
string = vim_strsave(string);
}
else
{
if (mustfree)
vim_free(string);
string = expand_env_save(name - 1);
if (string != NULL && *string == '$')
VIM_CLEAR(string);
}
name[len] = cc;
rettv->v_type = VAR_STRING;
rettv->vval.v_string = string;
}
return OK;
}
pos_T *
var2fpos(
typval_T *varp,
int dollar_lnum, 
int *fnum) 
{
char_u *name;
static pos_T pos;
pos_T *pp;
if (varp->v_type == VAR_LIST)
{
list_T *l;
int len;
int error = FALSE;
listitem_T *li;
l = varp->vval.v_list;
if (l == NULL)
return NULL;
pos.lnum = list_find_nr(l, 0L, &error);
if (error || pos.lnum <= 0 || pos.lnum > curbuf->b_ml.ml_line_count)
return NULL; 
pos.col = list_find_nr(l, 1L, &error);
if (error)
return NULL;
len = (long)STRLEN(ml_get(pos.lnum));
li = list_find(l, 1L);
if (li != NULL && li->li_tv.v_type == VAR_STRING
&& li->li_tv.vval.v_string != NULL
&& STRCMP(li->li_tv.vval.v_string, "$") == 0)
pos.col = len + 1;
if (pos.col == 0 || (int)pos.col > len + 1)
return NULL; 
--pos.col;
pos.coladd = list_find_nr(l, 2L, &error);
if (error)
pos.coladd = 0;
return &pos;
}
name = tv_get_string_chk(varp);
if (name == NULL)
return NULL;
if (name[0] == '.') 
return &curwin->w_cursor;
if (name[0] == 'v' && name[1] == NUL) 
{
if (VIsual_active)
return &VIsual;
return &curwin->w_cursor;
}
if (name[0] == '\'') 
{
pp = getmark_buf_fnum(curbuf, name[1], FALSE, fnum);
if (pp == NULL || pp == (pos_T *)-1 || pp->lnum <= 0)
return NULL;
return pp;
}
pos.coladd = 0;
if (name[0] == 'w' && dollar_lnum)
{
pos.col = 0;
if (name[1] == '0') 
{
update_topline();
pos.lnum = curwin->w_topline > 0 ? curwin->w_topline : 1;
return &pos;
}
else if (name[1] == '$') 
{
validate_botline();
pos.lnum = curwin->w_botline > 0 ? curwin->w_botline - 1 : 0;
return &pos;
}
}
else if (name[0] == '$') 
{
if (dollar_lnum)
{
pos.lnum = curbuf->b_ml.ml_line_count;
pos.col = 0;
}
else
{
pos.lnum = curwin->w_cursor.lnum;
pos.col = (colnr_T)STRLEN(ml_get_curline());
}
return &pos;
}
return NULL;
}
int
list2fpos(
typval_T *arg,
pos_T *posp,
int *fnump,
colnr_T *curswantp)
{
list_T *l = arg->vval.v_list;
long i = 0;
long n;
if (arg->v_type != VAR_LIST
|| l == NULL
|| l->lv_len < (fnump == NULL ? 2 : 3)
|| l->lv_len > (fnump == NULL ? 4 : 5))
return FAIL;
if (fnump != NULL)
{
n = list_find_nr(l, i++, NULL); 
if (n < 0)
return FAIL;
if (n == 0)
n = curbuf->b_fnum; 
*fnump = n;
}
n = list_find_nr(l, i++, NULL); 
if (n < 0)
return FAIL;
posp->lnum = n;
n = list_find_nr(l, i++, NULL); 
if (n < 0)
return FAIL;
posp->col = n;
n = list_find_nr(l, i, NULL); 
if (n < 0)
posp->coladd = 0;
else
posp->coladd = n;
if (curswantp != NULL)
*curswantp = list_find_nr(l, i + 1, NULL); 
return OK;
}
int
get_env_len(char_u **arg)
{
char_u *p;
int len;
for (p = *arg; vim_isIDc(*p); ++p)
;
if (p == *arg) 
return 0;
len = (int)(p - *arg);
*arg = p;
return len;
}
int
get_id_len(char_u **arg)
{
char_u *p;
int len;
for (p = *arg; eval_isnamec(*p); ++p)
{
if (*p == ':')
{
len = (int)(p - *arg);
if ((len == 1 && vim_strchr(NAMESPACE_CHAR, **arg) == NULL)
|| len > 1)
break;
}
}
if (p == *arg) 
return 0;
len = (int)(p - *arg);
*arg = skipwhite(p);
return len;
}
int
get_name_len(
char_u **arg,
char_u **alias,
int evaluate,
int verbose)
{
int len;
char_u *p;
char_u *expr_start;
char_u *expr_end;
*alias = NULL; 
if ((*arg)[0] == K_SPECIAL && (*arg)[1] == KS_EXTRA
&& (*arg)[2] == (int)KE_SNR)
{
*arg += 3;
return get_id_len(arg) + 3;
}
len = eval_fname_script(*arg);
if (len > 0)
{
*arg += len;
}
p = find_name_end(*arg, &expr_start, &expr_end,
len > 0 ? 0 : FNE_CHECK_START);
if (expr_start != NULL)
{
char_u *temp_string;
if (!evaluate)
{
len += (int)(p - *arg);
*arg = skipwhite(p);
return len;
}
temp_string = make_expanded_name(*arg - len, expr_start, expr_end, p);
if (temp_string == NULL)
return -1;
*alias = temp_string;
*arg = skipwhite(p);
return (int)STRLEN(temp_string);
}
len += get_id_len(arg);
if (len == 0 && verbose && **arg != NUL)
semsg(_(e_invexpr2), *arg);
return len;
}
char_u *
find_name_end(
char_u *arg,
char_u **expr_start,
char_u **expr_end,
int flags)
{
int mb_nest = 0;
int br_nest = 0;
char_u *p;
int len;
if (expr_start != NULL)
{
*expr_start = NULL;
*expr_end = NULL;
}
if ((flags & FNE_CHECK_START) && !eval_isnamec1(*arg) && *arg != '{')
return arg;
for (p = arg; *p != NUL
&& (eval_isnamec(*p)
|| *p == '{'
|| ((flags & FNE_INCL_BR) && (*p == '[' || *p == '.'))
|| mb_nest != 0
|| br_nest != 0); MB_PTR_ADV(p))
{
if (*p == '\'')
{
for (p = p + 1; *p != NUL && *p != '\''; MB_PTR_ADV(p))
;
if (*p == NUL)
break;
}
else if (*p == '"')
{
for (p = p + 1; *p != NUL && *p != '"'; MB_PTR_ADV(p))
if (*p == '\\' && p[1] != NUL)
++p;
if (*p == NUL)
break;
}
else if (br_nest == 0 && mb_nest == 0 && *p == ':')
{
len = (int)(p - arg);
if ((len == 1 && vim_strchr(NAMESPACE_CHAR, *arg) == NULL)
|| (len > 1 && p[-1] != '}'))
break;
}
if (mb_nest == 0)
{
if (*p == '[')
++br_nest;
else if (*p == ']')
--br_nest;
}
if (br_nest == 0)
{
if (*p == '{')
{
mb_nest++;
if (expr_start != NULL && *expr_start == NULL)
*expr_start = p;
}
else if (*p == '}')
{
mb_nest--;
if (expr_start != NULL && mb_nest == 0 && *expr_end == NULL)
*expr_end = p;
}
}
}
return p;
}
static char_u *
make_expanded_name(
char_u *in_start,
char_u *expr_start,
char_u *expr_end,
char_u *in_end)
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
if (temp_result != NULL && nextcmd == NULL)
{
retval = alloc(STRLEN(temp_result) + (expr_start - in_start)
+ (in_end - expr_end) + 1);
if (retval != NULL)
{
STRCPY(retval, in_start);
STRCAT(retval, temp_result);
STRCAT(retval, expr_end + 1);
}
}
vim_free(temp_result);
*in_end = c1; 
*expr_start = '{';
*expr_end = '}';
if (retval != NULL)
{
temp_result = find_name_end(retval, &expr_start, &expr_end, 0);
if (expr_start != NULL)
{
temp_result = make_expanded_name(retval, expr_start,
expr_end, temp_result);
vim_free(retval);
retval = temp_result;
}
}
return retval;
}
int
eval_isnamec(int c)
{
return (ASCII_ISALNUM(c) || c == '_' || c == ':' || c == AUTOLOAD_CHAR);
}
int
eval_isnamec1(int c)
{
return (ASCII_ISALPHA(c) || c == '_');
}
int
handle_subscript(
char_u **arg,
typval_T *rettv,
int evaluate, 
int verbose, 
char_u *start_leader, 
char_u **end_leaderp) 
{
int ret = OK;
dict_T *selfdict = NULL;
while (ret == OK
&& (((**arg == '['
|| (**arg == '.' && (rettv->v_type == VAR_DICT
|| (!evaluate
&& (*arg)[1] != '.'
&& current_sctx.sc_version >= 2)))
|| (**arg == '(' && (!evaluate || rettv->v_type == VAR_FUNC
|| rettv->v_type == VAR_PARTIAL)))
&& !VIM_ISWHITE(*(*arg - 1)))
|| (**arg == '-' && (*arg)[1] == '>')))
{
if (**arg == '(')
{
ret = call_func_rettv(arg, rettv, evaluate, selfdict, NULL);
if (aborting())
{
if (ret == OK)
clear_tv(rettv);
ret = FAIL;
}
dict_unref(selfdict);
selfdict = NULL;
}
else if (**arg == '-')
{
if (evaluate && *end_leaderp > start_leader)
ret = eval7_leader(rettv, start_leader, end_leaderp);
if (ret == OK)
{
if ((*arg)[2] == '{')
ret = eval_lambda(arg, rettv, evaluate, verbose);
else
ret = eval_method(arg, rettv, evaluate, verbose);
}
}
else 
{
dict_unref(selfdict);
if (rettv->v_type == VAR_DICT)
{
selfdict = rettv->vval.v_dict;
if (selfdict != NULL)
++selfdict->dv_refcount;
}
else
selfdict = NULL;
if (eval_index(arg, rettv, evaluate, verbose) == FAIL)
{
clear_tv(rettv);
ret = FAIL;
}
}
}
if (selfdict != NULL
&& (rettv->v_type == VAR_FUNC
|| (rettv->v_type == VAR_PARTIAL
&& (rettv->vval.v_partial->pt_auto
|| rettv->vval.v_partial->pt_dict == NULL))))
selfdict = make_partial(selfdict, rettv);
dict_unref(selfdict);
return ret;
}
typval_T *
alloc_tv(void)
{
return ALLOC_CLEAR_ONE(typval_T);
}
typval_T *
alloc_string_tv(char_u *s)
{
typval_T *rettv;
rettv = alloc_tv();
if (rettv != NULL)
{
rettv->v_type = VAR_STRING;
rettv->vval.v_string = s;
}
else
vim_free(s);
return rettv;
}
void
free_tv(typval_T *varp)
{
if (varp != NULL)
{
switch (varp->v_type)
{
case VAR_FUNC:
func_unref(varp->vval.v_string);
case VAR_STRING:
vim_free(varp->vval.v_string);
break;
case VAR_PARTIAL:
partial_unref(varp->vval.v_partial);
break;
case VAR_BLOB:
blob_unref(varp->vval.v_blob);
break;
case VAR_LIST:
list_unref(varp->vval.v_list);
break;
case VAR_DICT:
dict_unref(varp->vval.v_dict);
break;
case VAR_JOB:
#if defined(FEAT_JOB_CHANNEL)
job_unref(varp->vval.v_job);
break;
#endif
case VAR_CHANNEL:
#if defined(FEAT_JOB_CHANNEL)
channel_unref(varp->vval.v_channel);
break;
#endif
case VAR_NUMBER:
case VAR_FLOAT:
case VAR_UNKNOWN:
case VAR_VOID:
case VAR_BOOL:
case VAR_SPECIAL:
break;
}
vim_free(varp);
}
}
void
clear_tv(typval_T *varp)
{
if (varp != NULL)
{
switch (varp->v_type)
{
case VAR_FUNC:
func_unref(varp->vval.v_string);
case VAR_STRING:
VIM_CLEAR(varp->vval.v_string);
break;
case VAR_PARTIAL:
partial_unref(varp->vval.v_partial);
varp->vval.v_partial = NULL;
break;
case VAR_BLOB:
blob_unref(varp->vval.v_blob);
varp->vval.v_blob = NULL;
break;
case VAR_LIST:
list_unref(varp->vval.v_list);
varp->vval.v_list = NULL;
break;
case VAR_DICT:
dict_unref(varp->vval.v_dict);
varp->vval.v_dict = NULL;
break;
case VAR_NUMBER:
case VAR_BOOL:
case VAR_SPECIAL:
varp->vval.v_number = 0;
break;
case VAR_FLOAT:
#if defined(FEAT_FLOAT)
varp->vval.v_float = 0.0;
break;
#endif
case VAR_JOB:
#if defined(FEAT_JOB_CHANNEL)
job_unref(varp->vval.v_job);
varp->vval.v_job = NULL;
#endif
break;
case VAR_CHANNEL:
#if defined(FEAT_JOB_CHANNEL)
channel_unref(varp->vval.v_channel);
varp->vval.v_channel = NULL;
#endif
case VAR_UNKNOWN:
case VAR_VOID:
break;
}
varp->v_lock = 0;
}
}
void
init_tv(typval_T *varp)
{
if (varp != NULL)
vim_memset(varp, 0, sizeof(typval_T));
}
varnumber_T
tv_get_number(typval_T *varp)
{
int error = FALSE;
return tv_get_number_chk(varp, &error); 
}
varnumber_T
tv_get_number_chk(typval_T *varp, int *denote)
{
varnumber_T n = 0L;
switch (varp->v_type)
{
case VAR_NUMBER:
return varp->vval.v_number;
case VAR_FLOAT:
#if defined(FEAT_FLOAT)
emsg(_("E805: Using a Float as a Number"));
break;
#endif
case VAR_FUNC:
case VAR_PARTIAL:
emsg(_("E703: Using a Funcref as a Number"));
break;
case VAR_STRING:
if (varp->vval.v_string != NULL)
vim_str2nr(varp->vval.v_string, NULL, NULL,
STR2NR_ALL, &n, NULL, 0, FALSE);
return n;
case VAR_LIST:
emsg(_("E745: Using a List as a Number"));
break;
case VAR_DICT:
emsg(_("E728: Using a Dictionary as a Number"));
break;
case VAR_BOOL:
case VAR_SPECIAL:
return varp->vval.v_number == VVAL_TRUE ? 1 : 0;
case VAR_JOB:
#if defined(FEAT_JOB_CHANNEL)
emsg(_("E910: Using a Job as a Number"));
break;
#endif
case VAR_CHANNEL:
#if defined(FEAT_JOB_CHANNEL)
emsg(_("E913: Using a Channel as a Number"));
break;
#endif
case VAR_BLOB:
emsg(_("E974: Using a Blob as a Number"));
break;
case VAR_UNKNOWN:
case VAR_VOID:
internal_error_no_abort("tv_get_number(UNKNOWN)");
break;
}
if (denote == NULL) 
n = -1;
else
*denote = TRUE;
return n;
}
#if defined(FEAT_FLOAT)
float_T
tv_get_float(typval_T *varp)
{
switch (varp->v_type)
{
case VAR_NUMBER:
return (float_T)(varp->vval.v_number);
case VAR_FLOAT:
return varp->vval.v_float;
case VAR_FUNC:
case VAR_PARTIAL:
emsg(_("E891: Using a Funcref as a Float"));
break;
case VAR_STRING:
emsg(_("E892: Using a String as a Float"));
break;
case VAR_LIST:
emsg(_("E893: Using a List as a Float"));
break;
case VAR_DICT:
emsg(_("E894: Using a Dictionary as a Float"));
break;
case VAR_BOOL:
emsg(_("E362: Using a boolean value as a Float"));
break;
case VAR_SPECIAL:
emsg(_("E907: Using a special value as a Float"));
break;
case VAR_JOB:
#if defined(FEAT_JOB_CHANNEL)
emsg(_("E911: Using a Job as a Float"));
break;
#endif
case VAR_CHANNEL:
#if defined(FEAT_JOB_CHANNEL)
emsg(_("E914: Using a Channel as a Float"));
break;
#endif
case VAR_BLOB:
emsg(_("E975: Using a Blob as a Float"));
break;
case VAR_UNKNOWN:
case VAR_VOID:
internal_error_no_abort("tv_get_float(UNKNOWN)");
break;
}
return 0;
}
#endif
char_u *
tv_get_string(typval_T *varp)
{
static char_u mybuf[NUMBUFLEN];
return tv_get_string_buf(varp, mybuf);
}
char_u *
tv_get_string_buf(typval_T *varp, char_u *buf)
{
char_u *res = tv_get_string_buf_chk(varp, buf);
return res != NULL ? res : (char_u *)"";
}
char_u *
tv_get_string_chk(typval_T *varp)
{
static char_u mybuf[NUMBUFLEN];
return tv_get_string_buf_chk(varp, mybuf);
}
char_u *
tv_get_string_buf_chk(typval_T *varp, char_u *buf)
{
switch (varp->v_type)
{
case VAR_NUMBER:
vim_snprintf((char *)buf, NUMBUFLEN, "%lld",
(varnumber_T)varp->vval.v_number);
return buf;
case VAR_FUNC:
case VAR_PARTIAL:
emsg(_("E729: using Funcref as a String"));
break;
case VAR_LIST:
emsg(_("E730: using List as a String"));
break;
case VAR_DICT:
emsg(_("E731: using Dictionary as a String"));
break;
case VAR_FLOAT:
#if defined(FEAT_FLOAT)
emsg(_(e_float_as_string));
break;
#endif
case VAR_STRING:
if (varp->vval.v_string != NULL)
return varp->vval.v_string;
return (char_u *)"";
case VAR_BOOL:
case VAR_SPECIAL:
STRCPY(buf, get_var_special_name(varp->vval.v_number));
return buf;
case VAR_BLOB:
emsg(_("E976: using Blob as a String"));
break;
case VAR_JOB:
#if defined(FEAT_JOB_CHANNEL)
{
job_T *job = varp->vval.v_job;
char *status;
if (job == NULL)
return (char_u *)"no process";
status = job->jv_status == JOB_FAILED ? "fail"
: job->jv_status >= JOB_ENDED ? "dead"
: "run";
#if defined(UNIX)
vim_snprintf((char *)buf, NUMBUFLEN,
"process %ld %s", (long)job->jv_pid, status);
#elif defined(MSWIN)
vim_snprintf((char *)buf, NUMBUFLEN,
"process %ld %s",
(long)job->jv_proc_info.dwProcessId,
status);
#else
vim_snprintf((char *)buf, NUMBUFLEN, "process ? %s", status);
#endif
return buf;
}
#endif
break;
case VAR_CHANNEL:
#if defined(FEAT_JOB_CHANNEL)
{
channel_T *channel = varp->vval.v_channel;
char *status = channel_status(channel, -1);
if (channel == NULL)
vim_snprintf((char *)buf, NUMBUFLEN, "channel %s", status);
else
vim_snprintf((char *)buf, NUMBUFLEN,
"channel %d %s", channel->ch_id, status);
return buf;
}
#endif
break;
case VAR_UNKNOWN:
case VAR_VOID:
emsg(_(e_inval_string));
break;
}
return NULL;
}
static char_u *
tv_stringify(typval_T *varp, char_u *buf)
{
if (varp->v_type == VAR_LIST
|| varp->v_type == VAR_DICT
|| varp->v_type == VAR_BLOB
|| varp->v_type == VAR_FUNC
|| varp->v_type == VAR_PARTIAL
|| varp->v_type == VAR_FLOAT)
{
typval_T tmp;
f_string(varp, &tmp);
tv_get_string_buf(&tmp, buf);
clear_tv(varp);
*varp = tmp;
return tmp.vval.v_string;
}
return tv_get_string_buf(varp, buf);
}
static int
tv_check_lock(typval_T *tv, char_u *name, int use_gettext)
{
int lock = 0;
switch (tv->v_type)
{
case VAR_BLOB:
if (tv->vval.v_blob != NULL)
lock = tv->vval.v_blob->bv_lock;
break;
case VAR_LIST:
if (tv->vval.v_list != NULL)
lock = tv->vval.v_list->lv_lock;
break;
case VAR_DICT:
if (tv->vval.v_dict != NULL)
lock = tv->vval.v_dict->dv_lock;
break;
default:
break;
}
return var_check_lock(tv->v_lock, name, use_gettext)
|| (lock != 0 && var_check_lock(lock, name, use_gettext));
}
void
copy_tv(typval_T *from, typval_T *to)
{
to->v_type = from->v_type;
to->v_lock = 0;
switch (from->v_type)
{
case VAR_NUMBER:
case VAR_BOOL:
case VAR_SPECIAL:
to->vval.v_number = from->vval.v_number;
break;
case VAR_FLOAT:
#if defined(FEAT_FLOAT)
to->vval.v_float = from->vval.v_float;
break;
#endif
case VAR_JOB:
#if defined(FEAT_JOB_CHANNEL)
to->vval.v_job = from->vval.v_job;
if (to->vval.v_job != NULL)
++to->vval.v_job->jv_refcount;
break;
#endif
case VAR_CHANNEL:
#if defined(FEAT_JOB_CHANNEL)
to->vval.v_channel = from->vval.v_channel;
if (to->vval.v_channel != NULL)
++to->vval.v_channel->ch_refcount;
break;
#endif
case VAR_STRING:
case VAR_FUNC:
if (from->vval.v_string == NULL)
to->vval.v_string = NULL;
else
{
to->vval.v_string = vim_strsave(from->vval.v_string);
if (from->v_type == VAR_FUNC)
func_ref(to->vval.v_string);
}
break;
case VAR_PARTIAL:
if (from->vval.v_partial == NULL)
to->vval.v_partial = NULL;
else
{
to->vval.v_partial = from->vval.v_partial;
++to->vval.v_partial->pt_refcount;
}
break;
case VAR_BLOB:
if (from->vval.v_blob == NULL)
to->vval.v_blob = NULL;
else
{
to->vval.v_blob = from->vval.v_blob;
++to->vval.v_blob->bv_refcount;
}
break;
case VAR_LIST:
if (from->vval.v_list == NULL)
to->vval.v_list = NULL;
else
{
to->vval.v_list = from->vval.v_list;
++to->vval.v_list->lv_refcount;
}
break;
case VAR_DICT:
if (from->vval.v_dict == NULL)
to->vval.v_dict = NULL;
else
{
to->vval.v_dict = from->vval.v_dict;
++to->vval.v_dict->dv_refcount;
}
break;
case VAR_UNKNOWN:
case VAR_VOID:
internal_error_no_abort("copy_tv(UNKNOWN)");
break;
}
}
int
item_copy(
typval_T *from,
typval_T *to,
int deep,
int copyID)
{
static int recurse = 0;
int ret = OK;
if (recurse >= DICT_MAXNEST)
{
emsg(_("E698: variable nested too deep for making a copy"));
return FAIL;
}
++recurse;
switch (from->v_type)
{
case VAR_NUMBER:
case VAR_FLOAT:
case VAR_STRING:
case VAR_FUNC:
case VAR_PARTIAL:
case VAR_BOOL:
case VAR_SPECIAL:
case VAR_JOB:
case VAR_CHANNEL:
copy_tv(from, to);
break;
case VAR_LIST:
to->v_type = VAR_LIST;
to->v_lock = 0;
if (from->vval.v_list == NULL)
to->vval.v_list = NULL;
else if (copyID != 0 && from->vval.v_list->lv_copyID == copyID)
{
to->vval.v_list = from->vval.v_list->lv_copylist;
++to->vval.v_list->lv_refcount;
}
else
to->vval.v_list = list_copy(from->vval.v_list, deep, copyID);
if (to->vval.v_list == NULL)
ret = FAIL;
break;
case VAR_BLOB:
ret = blob_copy(from->vval.v_blob, to);
break;
case VAR_DICT:
to->v_type = VAR_DICT;
to->v_lock = 0;
if (from->vval.v_dict == NULL)
to->vval.v_dict = NULL;
else if (copyID != 0 && from->vval.v_dict->dv_copyID == copyID)
{
to->vval.v_dict = from->vval.v_dict->dv_copydict;
++to->vval.v_dict->dv_refcount;
}
else
to->vval.v_dict = dict_copy(from->vval.v_dict, deep, copyID);
if (to->vval.v_dict == NULL)
ret = FAIL;
break;
case VAR_UNKNOWN:
case VAR_VOID:
internal_error_no_abort("item_copy(UNKNOWN)");
ret = FAIL;
}
--recurse;
return ret;
}
void
echo_one(typval_T *rettv, int with_space, int *atstart, int *needclr)
{
char_u *tofree;
char_u numbuf[NUMBUFLEN];
char_u *p = echo_string(rettv, &tofree, numbuf, get_copyID());
if (*atstart)
{
*atstart = FALSE;
if (with_space)
{
msg_sb_eol();
msg_start();
}
}
else if (with_space)
msg_puts_attr(" ", echo_attr);
if (p != NULL)
for ( ; *p != NUL && !got_int; ++p)
{
if (*p == '\n' || *p == '\r' || *p == TAB)
{
if (*p != TAB && *needclr)
{
msg_clr_eos();
*needclr = FALSE;
}
msg_putchar_attr(*p, echo_attr);
}
else
{
if (has_mbyte)
{
int i = (*mb_ptr2len)(p);
(void)msg_outtrans_len_attr(p, i, echo_attr);
p += i - 1;
}
else
(void)msg_outtrans_len_attr(p, 1, echo_attr);
}
}
vim_free(tofree);
}
void
ex_echo(exarg_T *eap)
{
char_u *arg = eap->arg;
typval_T rettv;
char_u *p;
int needclr = TRUE;
int atstart = TRUE;
int did_emsg_before = did_emsg;
int called_emsg_before = called_emsg;
if (eap->skip)
++emsg_skip;
while (*arg != NUL && *arg != '|' && *arg != '\n' && !got_int)
{
need_clr_eos = needclr;
p = arg;
if (eval1(&arg, &rettv, !eap->skip) == FAIL)
{
if (!aborting() && did_emsg == did_emsg_before
&& called_emsg == called_emsg_before)
semsg(_(e_invexpr2), p);
need_clr_eos = FALSE;
break;
}
need_clr_eos = FALSE;
if (!eap->skip)
echo_one(&rettv, eap->cmdidx == CMD_echo, &atstart, &needclr);
clear_tv(&rettv);
arg = skipwhite(arg);
}
eap->nextcmd = check_nextcmd(arg);
if (eap->skip)
--emsg_skip;
else
{
if (needclr)
msg_clr_eos();
if (eap->cmdidx == CMD_echo)
msg_end();
}
}
void
ex_echohl(exarg_T *eap)
{
echo_attr = syn_name2attr(eap->arg);
}
int
get_echo_attr(void)
{
return echo_attr;
}
void
ex_execute(exarg_T *eap)
{
char_u *arg = eap->arg;
typval_T rettv;
int ret = OK;
char_u *p;
garray_T ga;
int len;
int save_did_emsg;
ga_init2(&ga, 1, 80);
if (eap->skip)
++emsg_skip;
while (*arg != NUL && *arg != '|' && *arg != '\n')
{
ret = eval1_emsg(&arg, &rettv, !eap->skip);
if (ret == FAIL)
break;
if (!eap->skip)
{
char_u buf[NUMBUFLEN];
if (eap->cmdidx == CMD_execute)
{
if (rettv.v_type == VAR_CHANNEL || rettv.v_type == VAR_JOB)
{
emsg(_(e_inval_string));
p = NULL;
}
else
p = tv_get_string_buf(&rettv, buf);
}
else
p = tv_stringify(&rettv, buf);
if (p == NULL)
{
clear_tv(&rettv);
ret = FAIL;
break;
}
len = (int)STRLEN(p);
if (ga_grow(&ga, len + 2) == FAIL)
{
clear_tv(&rettv);
ret = FAIL;
break;
}
if (ga.ga_len)
((char_u *)(ga.ga_data))[ga.ga_len++] = ' ';
STRCPY((char_u *)(ga.ga_data) + ga.ga_len, p);
ga.ga_len += len;
}
clear_tv(&rettv);
arg = skipwhite(arg);
}
if (ret != FAIL && ga.ga_data != NULL)
{
if (eap->cmdidx == CMD_echomsg || eap->cmdidx == CMD_echoerr)
{
msg_sb_eol();
}
if (eap->cmdidx == CMD_echomsg)
{
msg_attr(ga.ga_data, echo_attr);
out_flush();
}
else if (eap->cmdidx == CMD_echoerr)
{
save_did_emsg = did_emsg;
emsg(ga.ga_data);
if (!force_abort)
did_emsg = save_did_emsg;
}
else if (eap->cmdidx == CMD_execute)
do_cmdline((char_u *)ga.ga_data,
eap->getline, eap->cookie, DOCMD_NOWAIT|DOCMD_VERBOSE);
}
ga_clear(&ga);
if (eap->skip)
--emsg_skip;
eap->nextcmd = check_nextcmd(arg);
}
char_u *
find_option_end(char_u **arg, int *opt_flags)
{
char_u *p = *arg;
++p;
if (*p == 'g' && p[1] == ':')
{
*opt_flags = OPT_GLOBAL;
p += 2;
}
else if (*p == 'l' && p[1] == ':')
{
*opt_flags = OPT_LOCAL;
p += 2;
}
else
*opt_flags = 0;
if (!ASCII_ISALPHA(*p))
return NULL;
*arg = p;
if (p[0] == 't' && p[1] == '_' && p[2] != NUL && p[3] != NUL)
p += 4; 
else
while (ASCII_ISALPHA(*p))
++p;
return p;
}
void
last_set_msg(sctx_T script_ctx)
{
char_u *p;
if (script_ctx.sc_sid != 0)
{
p = home_replace_save(NULL, get_scriptname(script_ctx.sc_sid));
if (p != NULL)
{
verbose_enter();
msg_puts(_("\n\tLast set from "));
msg_puts((char *)p);
if (script_ctx.sc_lnum > 0)
{
msg_puts(_(line_msg));
msg_outnum((long)script_ctx.sc_lnum);
}
verbose_leave();
vim_free(p);
}
}
}
int
typval_compare(
typval_T *typ1, 
typval_T *typ2, 
exptype_T type, 
int ic) 
{
int i;
varnumber_T n1, n2;
char_u *s1, *s2;
char_u buf1[NUMBUFLEN], buf2[NUMBUFLEN];
int type_is = type == EXPR_IS || type == EXPR_ISNOT;
if (type_is && typ1->v_type != typ2->v_type)
{
n1 = (type == EXPR_ISNOT);
}
else if (typ1->v_type == VAR_BLOB || typ2->v_type == VAR_BLOB)
{
if (type_is)
{
n1 = (typ1->v_type == typ2->v_type
&& typ1->vval.v_blob == typ2->vval.v_blob);
if (type == EXPR_ISNOT)
n1 = !n1;
}
else if (typ1->v_type != typ2->v_type
|| (type != EXPR_EQUAL && type != EXPR_NEQUAL))
{
if (typ1->v_type != typ2->v_type)
emsg(_("E977: Can only compare Blob with Blob"));
else
emsg(_(e_invalblob));
clear_tv(typ1);
return FAIL;
}
else
{
n1 = blob_equal(typ1->vval.v_blob, typ2->vval.v_blob);
if (type == EXPR_NEQUAL)
n1 = !n1;
}
}
else if (typ1->v_type == VAR_LIST || typ2->v_type == VAR_LIST)
{
if (type_is)
{
n1 = (typ1->v_type == typ2->v_type
&& typ1->vval.v_list == typ2->vval.v_list);
if (type == EXPR_ISNOT)
n1 = !n1;
}
else if (typ1->v_type != typ2->v_type
|| (type != EXPR_EQUAL && type != EXPR_NEQUAL))
{
if (typ1->v_type != typ2->v_type)
emsg(_("E691: Can only compare List with List"));
else
emsg(_("E692: Invalid operation for List"));
clear_tv(typ1);
return FAIL;
}
else
{
n1 = list_equal(typ1->vval.v_list, typ2->vval.v_list,
ic, FALSE);
if (type == EXPR_NEQUAL)
n1 = !n1;
}
}
else if (typ1->v_type == VAR_DICT || typ2->v_type == VAR_DICT)
{
if (type_is)
{
n1 = (typ1->v_type == typ2->v_type
&& typ1->vval.v_dict == typ2->vval.v_dict);
if (type == EXPR_ISNOT)
n1 = !n1;
}
else if (typ1->v_type != typ2->v_type
|| (type != EXPR_EQUAL && type != EXPR_NEQUAL))
{
if (typ1->v_type != typ2->v_type)
emsg(_("E735: Can only compare Dictionary with Dictionary"));
else
emsg(_("E736: Invalid operation for Dictionary"));
clear_tv(typ1);
return FAIL;
}
else
{
n1 = dict_equal(typ1->vval.v_dict, typ2->vval.v_dict,
ic, FALSE);
if (type == EXPR_NEQUAL)
n1 = !n1;
}
}
else if (typ1->v_type == VAR_FUNC || typ2->v_type == VAR_FUNC
|| typ1->v_type == VAR_PARTIAL || typ2->v_type == VAR_PARTIAL)
{
if (type != EXPR_EQUAL && type != EXPR_NEQUAL
&& type != EXPR_IS && type != EXPR_ISNOT)
{
emsg(_("E694: Invalid operation for Funcrefs"));
clear_tv(typ1);
return FAIL;
}
if ((typ1->v_type == VAR_PARTIAL
&& typ1->vval.v_partial == NULL)
|| (typ2->v_type == VAR_PARTIAL
&& typ2->vval.v_partial == NULL))
n1 = FALSE;
else if (type_is)
{
if (typ1->v_type == VAR_FUNC && typ2->v_type == VAR_FUNC)
n1 = tv_equal(typ1, typ2, ic, FALSE);
else if (typ1->v_type == VAR_PARTIAL
&& typ2->v_type == VAR_PARTIAL)
n1 = (typ1->vval.v_partial == typ2->vval.v_partial);
else
n1 = FALSE;
}
else
n1 = tv_equal(typ1, typ2, ic, FALSE);
if (type == EXPR_NEQUAL || type == EXPR_ISNOT)
n1 = !n1;
}
#if defined(FEAT_FLOAT)
else if ((typ1->v_type == VAR_FLOAT || typ2->v_type == VAR_FLOAT)
&& type != EXPR_MATCH && type != EXPR_NOMATCH)
{
float_T f1, f2;
f1 = tv_get_float(typ1);
f2 = tv_get_float(typ2);
n1 = FALSE;
switch (type)
{
case EXPR_IS:
case EXPR_EQUAL: n1 = (f1 == f2); break;
case EXPR_ISNOT:
case EXPR_NEQUAL: n1 = (f1 != f2); break;
case EXPR_GREATER: n1 = (f1 > f2); break;
case EXPR_GEQUAL: n1 = (f1 >= f2); break;
case EXPR_SMALLER: n1 = (f1 < f2); break;
case EXPR_SEQUAL: n1 = (f1 <= f2); break;
case EXPR_UNKNOWN:
case EXPR_MATCH:
default: break; 
}
}
#endif
else if ((typ1->v_type == VAR_NUMBER || typ2->v_type == VAR_NUMBER)
&& type != EXPR_MATCH && type != EXPR_NOMATCH)
{
n1 = tv_get_number(typ1);
n2 = tv_get_number(typ2);
switch (type)
{
case EXPR_IS:
case EXPR_EQUAL: n1 = (n1 == n2); break;
case EXPR_ISNOT:
case EXPR_NEQUAL: n1 = (n1 != n2); break;
case EXPR_GREATER: n1 = (n1 > n2); break;
case EXPR_GEQUAL: n1 = (n1 >= n2); break;
case EXPR_SMALLER: n1 = (n1 < n2); break;
case EXPR_SEQUAL: n1 = (n1 <= n2); break;
case EXPR_UNKNOWN:
case EXPR_MATCH:
default: break; 
}
}
else
{
s1 = tv_get_string_buf(typ1, buf1);
s2 = tv_get_string_buf(typ2, buf2);
if (type != EXPR_MATCH && type != EXPR_NOMATCH)
i = ic ? MB_STRICMP(s1, s2) : STRCMP(s1, s2);
else
i = 0;
n1 = FALSE;
switch (type)
{
case EXPR_IS:
case EXPR_EQUAL: n1 = (i == 0); break;
case EXPR_ISNOT:
case EXPR_NEQUAL: n1 = (i != 0); break;
case EXPR_GREATER: n1 = (i > 0); break;
case EXPR_GEQUAL: n1 = (i >= 0); break;
case EXPR_SMALLER: n1 = (i < 0); break;
case EXPR_SEQUAL: n1 = (i <= 0); break;
case EXPR_MATCH:
case EXPR_NOMATCH:
n1 = pattern_match(s2, s1, ic);
if (type == EXPR_NOMATCH)
n1 = !n1;
break;
default: break; 
}
}
clear_tv(typ1);
typ1->v_type = VAR_NUMBER;
typ1->vval.v_number = n1;
return OK;
}
char_u *
typval_tostring(typval_T *arg)
{
char_u *tofree;
char_u numbuf[NUMBUFLEN];
char_u *ret = NULL;
if (arg == NULL)
return vim_strsave((char_u *)"(does not exist)");
ret = tv2string(arg, &tofree, numbuf, 0);
if (ret != NULL && tofree == NULL)
ret = vim_strsave(ret);
return ret;
}
#endif 
char_u *
do_string_sub(
char_u *str,
char_u *pat,
char_u *sub,
typval_T *expr,
char_u *flags)
{
int sublen;
regmatch_T regmatch;
int i;
int do_all;
char_u *tail;
char_u *end;
garray_T ga;
char_u *ret;
char_u *save_cpo;
char_u *zero_width = NULL;
save_cpo = p_cpo;
p_cpo = empty_option;
ga_init2(&ga, 1, 200);
do_all = (flags[0] == 'g');
regmatch.rm_ic = p_ic;
regmatch.regprog = vim_regcomp(pat, RE_MAGIC + RE_STRING);
if (regmatch.regprog != NULL)
{
tail = str;
end = str + STRLEN(str);
while (vim_regexec_nl(&regmatch, str, (colnr_T)(tail - str)))
{
if (regmatch.startp[0] == regmatch.endp[0])
{
if (zero_width == regmatch.startp[0])
{
i = mb_ptr2len(tail);
mch_memmove((char_u *)ga.ga_data + ga.ga_len, tail,
(size_t)i);
ga.ga_len += i;
tail += i;
continue;
}
zero_width = regmatch.startp[0];
}
sublen = vim_regsub(&regmatch, sub, expr, tail, FALSE, TRUE, FALSE);
if (ga_grow(&ga, (int)((end - tail) + sublen -
(regmatch.endp[0] - regmatch.startp[0]))) == FAIL)
{
ga_clear(&ga);
break;
}
i = (int)(regmatch.startp[0] - tail);
mch_memmove((char_u *)ga.ga_data + ga.ga_len, tail, (size_t)i);
(void)vim_regsub(&regmatch, sub, expr, (char_u *)ga.ga_data
+ ga.ga_len + i, TRUE, TRUE, FALSE);
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
ret = vim_strsave(ga.ga_data == NULL ? str : (char_u *)ga.ga_data);
ga_clear(&ga);
if (p_cpo == empty_option)
p_cpo = save_cpo;
else
free_string_option(save_cpo);
return ret;
}
