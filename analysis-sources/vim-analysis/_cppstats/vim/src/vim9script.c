#include "vim.h"
#if defined(FEAT_EVAL) || defined(PROTO)
#include "vim9.h"
static char e_needs_vim9[] = N_("E1042: import/export can only be used in vim9script");
int
in_vim9script(void)
{
return current_sctx.sc_version == SCRIPT_VERSION_VIM9;
}
void
ex_vim9script(exarg_T *eap)
{
scriptitem_T *si = SCRIPT_ITEM(current_sctx.sc_sid);
if (!getline_equal(eap->getline, eap->cookie, getsourceline))
{
emsg(_("E1038: vim9script can only be used in a script"));
return;
}
if (si->sn_had_command)
{
emsg(_("E1039: vim9script must be the first command in a script"));
return;
}
current_sctx.sc_version = SCRIPT_VERSION_VIM9;
si->sn_version = SCRIPT_VERSION_VIM9;
si->sn_had_command = TRUE;
if (STRCMP(p_cpo, CPO_VIM) != 0)
{
si->sn_save_cpo = p_cpo;
p_cpo = vim_strsave((char_u *)CPO_VIM);
}
}
void
ex_export(exarg_T *eap UNUSED)
{
if (current_sctx.sc_version != SCRIPT_VERSION_VIM9)
{
emsg(_(e_needs_vim9));
return;
}
eap->cmd = eap->arg;
(void)find_ex_command(eap, NULL, lookup_scriptvar, NULL);
switch (eap->cmdidx)
{
case CMD_let:
case CMD_const:
case CMD_def:
is_export = TRUE;
do_cmdline(eap->cmd, eap->getline, eap->cookie,
DOCMD_VERBOSE + DOCMD_NOWAIT);
if (is_export)
{
emsg(_("E1044: export with invalid argument"));
is_export = FALSE;
}
break;
default:
emsg(_("E1043: Invalid command after :export"));
break;
}
}
static imported_T *
new_imported(garray_T *gap)
{
if (ga_grow(gap, 1) == OK)
return ((imported_T *)gap->ga_data + gap->ga_len++);
return NULL;
}
void
free_imports(int sid)
{
scriptitem_T *si = SCRIPT_ITEM(sid);
int idx;
for (idx = 0; idx < si->sn_imports.ga_len; ++idx)
{
imported_T *imp = ((imported_T *)si->sn_imports.ga_data) + idx;
vim_free(imp->imp_name);
}
ga_clear(&si->sn_imports);
ga_clear(&si->sn_var_vals);
ga_clear(&si->sn_type_list);
}
void
ex_import(exarg_T *eap)
{
if (current_sctx.sc_version != SCRIPT_VERSION_VIM9)
emsg(_(e_needs_vim9));
else
{
char_u *cmd_end = handle_import(eap->arg, NULL,
current_sctx.sc_sid, NULL);
if (cmd_end != NULL)
eap->nextcmd = check_nextcmd(cmd_end);
}
}
int
find_exported(
int sid,
char_u **argp,
int *name_len,
ufunc_T **ufunc,
type_T **type)
{
char_u *name = *argp;
char_u *arg = *argp;
int cc;
int idx = -1;
svar_T *sv;
scriptitem_T *script = SCRIPT_ITEM(sid);
while (eval_isnamec(*arg))
++arg;
*name_len = (int)(arg - name);
cc = *arg;
*arg = NUL;
idx = get_script_item_idx(sid, name, FALSE);
if (idx >= 0)
{
sv = ((svar_T *)script->sn_var_vals.ga_data) + idx;
if (!sv->sv_export)
{
semsg(_("E1049: Item not exported in script: %s"), name);
*arg = cc;
return -1;
}
*type = sv->sv_type;
*ufunc = NULL;
}
else
{
char_u buffer[200];
char_u *funcname;
if (STRLEN(name) < sizeof(buffer) - 10)
funcname = buffer;
else
{
funcname = alloc(STRLEN(name) + 10);
if (funcname == NULL)
{
*arg = cc;
return -1;
}
}
funcname[0] = K_SPECIAL;
funcname[1] = KS_EXTRA;
funcname[2] = (int)KE_SNR;
sprintf((char *)funcname + 3, "%ld_%s", (long)sid, name);
*ufunc = find_func(funcname, NULL);
if (funcname != buffer)
vim_free(funcname);
if (*ufunc == NULL)
{
semsg(_("E1048: Item not found in script: %s"), name);
*arg = cc;
return -1;
}
}
*arg = cc;
arg = skipwhite(arg);
*argp = arg;
return idx;
}
char_u *
handle_import(char_u *arg_start, garray_T *gap, int import_sid, void *cctx)
{
char_u *arg = arg_start;
char_u *cmd_end;
char_u *as_ptr = NULL;
char_u *from_ptr;
int as_len = 0;
int ret = FAIL;
typval_T tv;
int sid = -1;
int res;
if (*arg == '{')
{
while (*arg != NUL && *arg != '}')
++arg;
if (*arg == '}')
arg = skipwhite(arg + 1);
}
else
{
if (*arg == '*')
arg = skipwhite(arg + 1);
else if (eval_isnamec1(*arg))
{
while (eval_isnamec(*arg))
++arg;
arg = skipwhite(arg);
}
if (STRNCMP("as", arg, 2) == 0 && VIM_ISWHITE(arg[2]))
{
arg = skipwhite(arg + 2);
as_ptr = arg;
if (eval_isnamec1(*arg))
while (eval_isnamec(*arg))
++arg;
as_len = (int)(arg - as_ptr);
arg = skipwhite(arg);
if (check_defined(as_ptr, as_len, cctx) == FAIL)
return NULL;
}
else if (*arg_start == '*')
{
emsg(_("E1045: Missing \"as\" after *"));
return NULL;
}
}
if (STRNCMP("from", arg, 4) != 0 || !VIM_ISWHITE(arg[4]))
{
emsg(_("E1070: Missing \"from\""));
return NULL;
}
from_ptr = arg;
arg = skipwhite(arg + 4);
tv.v_type = VAR_UNKNOWN;
if (*arg == '\'')
ret = get_lit_string_tv(&arg, &tv, TRUE);
else if (*arg == '"')
ret = get_string_tv(&arg, &tv, TRUE);
if (ret == FAIL || tv.vval.v_string == NULL || *tv.vval.v_string == NUL)
{
emsg(_("E1071: Invalid string after \"from\""));
return NULL;
}
cmd_end = arg;
if (*tv.vval.v_string == '.')
{
size_t len;
scriptitem_T *si = SCRIPT_ITEM(current_sctx.sc_sid);
char_u *tail = gettail(si->sn_name);
char_u *from_name;
len = STRLEN(si->sn_name) - STRLEN(tail) + STRLEN(tv.vval.v_string) + 2;
from_name = alloc((int)len);
if (from_name == NULL)
{
clear_tv(&tv);
return NULL;
}
vim_strncpy(from_name, si->sn_name, tail - si->sn_name);
add_pathsep(from_name);
STRCAT(from_name, tv.vval.v_string);
simplify_filename(from_name);
res = do_source(from_name, FALSE, DOSO_NONE, &sid);
vim_free(from_name);
}
else if (mch_isFullName(tv.vval.v_string))
{
res = do_source(tv.vval.v_string, FALSE, DOSO_NONE, &sid);
}
else
{
size_t len = 7 + STRLEN(tv.vval.v_string) + 1;
char_u *from_name;
from_name = alloc((int)len);
if (from_name == NULL)
{
clear_tv(&tv);
return NULL;
}
vim_snprintf((char *)from_name, len, "import/%s", tv.vval.v_string);
res = source_in_path(p_rtp, from_name, DIP_NOAFTER, &sid);
vim_free(from_name);
}
if (res == FAIL || sid <= 0)
{
semsg(_("E1053: Could not import \"%s\""), tv.vval.v_string);
clear_tv(&tv);
return NULL;
}
clear_tv(&tv);
if (*arg_start == '*')
{
imported_T *imported = new_imported(gap != NULL ? gap
: &SCRIPT_ITEM(import_sid)->sn_imports);
if (imported == NULL)
return NULL;
imported->imp_name = vim_strnsave(as_ptr, as_len);
imported->imp_sid = sid;
imported->imp_all = TRUE;
}
else
{
arg = arg_start;
if (*arg == '{')
arg = skipwhite(arg + 1);
for (;;)
{
char_u *name = arg;
int name_len;
int idx;
imported_T *imported;
ufunc_T *ufunc = NULL;
type_T *type;
idx = find_exported(sid, &arg, &name_len, &ufunc, &type);
if (idx < 0 && ufunc == NULL)
return NULL;
if (check_defined(name, name_len, cctx) == FAIL)
return NULL;
imported = new_imported(gap != NULL ? gap
: &SCRIPT_ITEM(import_sid)->sn_imports);
if (imported == NULL)
return NULL;
imported->imp_name = vim_strnsave(name, name_len);
imported->imp_sid = sid;
if (idx >= 0)
{
imported->imp_type = type;
imported->imp_var_vals_idx = idx;
}
else
imported->imp_funcname = ufunc->uf_name;
arg = skipwhite(arg);
if (*arg_start != '{')
break;
if (*arg == '}')
{
arg = skipwhite(arg + 1);
break;
}
if (*arg != ',')
{
emsg(_("E1046: Missing comma in import"));
return NULL;
}
arg = skipwhite(arg + 1);
}
if (arg != from_ptr)
{
emsg(_("E1047: syntax error in import"));
return NULL;
}
}
return cmd_end;
}
#endif 
