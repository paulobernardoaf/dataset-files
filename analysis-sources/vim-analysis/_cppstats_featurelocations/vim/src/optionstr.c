












#include "vim.h"

static char *(p_ambw_values[]) = {"single", "double", NULL};
static char *(p_bg_values[]) = {"light", "dark", NULL};
static char *(p_bkc_values[]) = {"yes", "auto", "no", "breaksymlink", "breakhardlink", NULL};
static char *(p_bo_values[]) = {"all", "backspace", "cursor", "complete",
"copy", "ctrlg", "error", "esc", "ex",
"hangul", "insertmode", "lang", "mess",
"showmatch", "operator", "register", "shell",
"spell", "wildmode", NULL};
static char *(p_nf_values[]) = {"bin", "octal", "hex", "alpha", NULL};
static char *(p_ff_values[]) = {FF_UNIX, FF_DOS, FF_MAC, NULL};
#if defined(FEAT_CRYPT)
static char *(p_cm_values[]) = {"zip", "blowfish", "blowfish2", NULL};
#endif
static char *(p_cmp_values[]) = {"internal", "keepascii", NULL};
static char *(p_dy_values[]) = {"lastline", "truncate", "uhex", NULL};
#if defined(FEAT_FOLDING)
static char *(p_fdo_values[]) = {"all", "block", "hor", "mark", "percent",
"quickfix", "search", "tag", "insert",
"undo", "jump", NULL};
#endif
#if defined(FEAT_SESSION)

static char *(p_ssop_values[]) = {"buffers", "winpos", "resize", "winsize",
"localoptions", "options", "help", "blank", "globals", "slash", "unix",
"sesdir", "curdir", "folds", "cursor", "tabpages", "terminal", NULL};
#endif

static char *(p_swb_values[]) = {"useopen", "usetab", "split", "newtab", "vsplit", "uselast", NULL};
static char *(p_tc_values[]) = {"followic", "ignore", "match", "followscs", "smart", NULL};
#if defined(FEAT_TOOLBAR) && !defined(FEAT_GUI_MSWIN)
static char *(p_toolbar_values[]) = {"text", "icons", "tooltips", "horiz", NULL};
#endif
#if defined(FEAT_TOOLBAR) && defined(FEAT_GUI_GTK)
static char *(p_tbis_values[]) = {"tiny", "small", "medium", "large", "huge", "giant", NULL};
#endif
#if defined(UNIX) || defined(VMS)
static char *(p_ttym_values[]) = {"xterm", "xterm2", "dec", "netterm", "jsbterm", "pterm", "urxvt", "sgr", NULL};
#endif
static char *(p_ve_values[]) = {"block", "insert", "all", "onemore", NULL};
static char *(p_wop_values[]) = {"tagfile", NULL};
#if defined(FEAT_WAK)
static char *(p_wak_values[]) = {"yes", "menu", "no", NULL};
#endif
static char *(p_mousem_values[]) = {"extend", "popup", "popup_setpos", "mac", NULL};
static char *(p_sel_values[]) = {"inclusive", "exclusive", "old", NULL};
static char *(p_slm_values[]) = {"mouse", "key", "cmd", NULL};
static char *(p_km_values[]) = {"startsel", "stopsel", NULL};
#if defined(FEAT_BROWSE)
static char *(p_bsdir_values[]) = {"current", "last", "buffer", NULL};
#endif
static char *(p_scbopt_values[]) = {"ver", "hor", "jump", NULL};
static char *(p_debug_values[]) = {"msg", "throw", "beep", NULL};
static char *(p_ead_values[]) = {"both", "ver", "hor", NULL};
static char *(p_buftype_values[]) = {"nofile", "nowrite", "quickfix", "help", "terminal", "acwrite", "prompt", "popup", NULL};
static char *(p_bufhidden_values[]) = {"hide", "unload", "delete", "wipe", NULL};
static char *(p_bs_values[]) = {"indent", "eol", "start", NULL};
#if defined(FEAT_FOLDING)
static char *(p_fdm_values[]) = {"manual", "expr", "marker", "indent", "syntax",
#if defined(FEAT_DIFF)
"diff",
#endif
NULL};
static char *(p_fcl_values[]) = {"all", NULL};
#endif
static char *(p_cot_values[]) = {"menu", "menuone", "longest", "preview", "popup", "popuphidden", "noinsert", "noselect", NULL};
#if defined(BACKSLASH_IN_FILENAME)
static char *(p_csl_values[]) = {"slash", "backslash", NULL};
#endif
#if defined(FEAT_SIGNS)
static char *(p_scl_values[]) = {"yes", "no", "auto", "number", NULL};
#endif
#if defined(MSWIN) && defined(FEAT_TERMINAL)
static char *(p_twt_values[]) = {"winpty", "conpty", "", NULL};
#endif

static int check_opt_strings(char_u *val, char **values, int list);
static int opt_strings_flags(char_u *val, char **values, unsigned *flagp, int list);





void
didset_string_options(void)
{
(void)opt_strings_flags(p_cmp, p_cmp_values, &cmp_flags, TRUE);
(void)opt_strings_flags(p_bkc, p_bkc_values, &bkc_flags, TRUE);
(void)opt_strings_flags(p_bo, p_bo_values, &bo_flags, TRUE);
#if defined(FEAT_SESSION)
(void)opt_strings_flags(p_ssop, p_ssop_values, &ssop_flags, TRUE);
(void)opt_strings_flags(p_vop, p_ssop_values, &vop_flags, TRUE);
#endif
#if defined(FEAT_FOLDING)
(void)opt_strings_flags(p_fdo, p_fdo_values, &fdo_flags, TRUE);
#endif
(void)opt_strings_flags(p_dy, p_dy_values, &dy_flags, TRUE);
(void)opt_strings_flags(p_tc, p_tc_values, &tc_flags, FALSE);
(void)opt_strings_flags(p_ve, p_ve_values, &ve_flags, TRUE);
#if defined(UNIX) || defined(VMS)
(void)opt_strings_flags(p_ttym, p_ttym_values, &ttym_flags, FALSE);
#endif
#if defined(FEAT_TOOLBAR) && !defined(FEAT_GUI_MSWIN)
(void)opt_strings_flags(p_toolbar, p_toolbar_values, &toolbar_flags, TRUE);
#endif
#if defined(FEAT_TOOLBAR) && defined(FEAT_GUI_GTK)
(void)opt_strings_flags(p_tbis, p_tbis_values, &tbis_flags, FALSE);
#endif
}

#if defined(FEAT_EVAL)











void
trigger_optionsset_string(
int opt_idx,
int opt_flags,
char_u *oldval,
char_u *oldval_l,
char_u *oldval_g,
char_u *newval)
{

if (oldval != NULL && newval != NULL
&& *get_vim_var_str(VV_OPTION_TYPE) == NUL)
{
char_u buf_type[7];

sprintf((char *)buf_type, "%s",
(opt_flags & OPT_LOCAL) ? "local" : "global");
set_vim_var_string(VV_OPTION_OLD, oldval, -1);
set_vim_var_string(VV_OPTION_NEW, newval, -1);
set_vim_var_string(VV_OPTION_TYPE, buf_type, -1);
if (opt_flags & OPT_LOCAL)
{
set_vim_var_string(VV_OPTION_COMMAND, (char_u *)"setlocal", -1);
set_vim_var_string(VV_OPTION_OLDLOCAL, oldval, -1);
}
if (opt_flags & OPT_GLOBAL)
{
set_vim_var_string(VV_OPTION_COMMAND, (char_u *)"setglobal", -1);
set_vim_var_string(VV_OPTION_OLDGLOBAL, oldval, -1);
}
if ((opt_flags & (OPT_LOCAL | OPT_GLOBAL)) == 0)
{
set_vim_var_string(VV_OPTION_COMMAND, (char_u *)"set", -1);
set_vim_var_string(VV_OPTION_OLDLOCAL, oldval_l, -1);
set_vim_var_string(VV_OPTION_OLDGLOBAL, oldval_g, -1);
}
if (opt_flags & OPT_MODELINE)
{
set_vim_var_string(VV_OPTION_COMMAND, (char_u *)"modeline", -1);
set_vim_var_string(VV_OPTION_OLDLOCAL, oldval, -1);
}
apply_autocmds(EVENT_OPTIONSET,
(char_u *)get_option_fullname(opt_idx), NULL, FALSE,
NULL);
reset_v_option_vars();
}
}
#endif

static char *
illegal_char(char *errbuf, int c)
{
if (errbuf == NULL)
return "";
sprintf((char *)errbuf, _("E539: Illegal character <%s>"),
(char *)transchar(c));
return errbuf;
}




void
check_buf_options(buf_T *buf)
{
check_string_option(&buf->b_p_bh);
check_string_option(&buf->b_p_bt);
check_string_option(&buf->b_p_fenc);
check_string_option(&buf->b_p_ff);
#if defined(FEAT_FIND_ID)
check_string_option(&buf->b_p_def);
check_string_option(&buf->b_p_inc);
#if defined(FEAT_EVAL)
check_string_option(&buf->b_p_inex);
#endif
#endif
#if defined(FEAT_CINDENT) && defined(FEAT_EVAL)
check_string_option(&buf->b_p_inde);
check_string_option(&buf->b_p_indk);
#endif
#if defined(FEAT_BEVAL) && defined(FEAT_EVAL)
check_string_option(&buf->b_p_bexpr);
#endif
#if defined(FEAT_CRYPT)
check_string_option(&buf->b_p_cm);
#endif
check_string_option(&buf->b_p_fp);
#if defined(FEAT_EVAL)
check_string_option(&buf->b_p_fex);
#endif
#if defined(FEAT_CRYPT)
check_string_option(&buf->b_p_key);
#endif
check_string_option(&buf->b_p_kp);
check_string_option(&buf->b_p_mps);
check_string_option(&buf->b_p_fo);
check_string_option(&buf->b_p_flp);
check_string_option(&buf->b_p_isk);
check_string_option(&buf->b_p_com);
#if defined(FEAT_FOLDING)
check_string_option(&buf->b_p_cms);
#endif
check_string_option(&buf->b_p_nf);
#if defined(FEAT_TEXTOBJ)
check_string_option(&buf->b_p_qe);
#endif
#if defined(FEAT_SYN_HL)
check_string_option(&buf->b_p_syn);
check_string_option(&buf->b_s.b_syn_isk);
#endif
#if defined(FEAT_SPELL)
check_string_option(&buf->b_s.b_p_spc);
check_string_option(&buf->b_s.b_p_spf);
check_string_option(&buf->b_s.b_p_spl);
#endif
#if defined(FEAT_SEARCHPATH)
check_string_option(&buf->b_p_sua);
#endif
#if defined(FEAT_CINDENT)
check_string_option(&buf->b_p_cink);
check_string_option(&buf->b_p_cino);
parse_cino(buf);
#endif
check_string_option(&buf->b_p_ft);
#if defined(FEAT_SMARTINDENT) || defined(FEAT_CINDENT)
check_string_option(&buf->b_p_cinw);
#endif
check_string_option(&buf->b_p_cpt);
#if defined(FEAT_COMPL_FUNC)
check_string_option(&buf->b_p_cfu);
check_string_option(&buf->b_p_ofu);
#endif
#if defined(FEAT_EVAL)
check_string_option(&buf->b_p_tfu);
#endif
#if defined(FEAT_KEYMAP)
check_string_option(&buf->b_p_keymap);
#endif
#if defined(FEAT_QUICKFIX)
check_string_option(&buf->b_p_gp);
check_string_option(&buf->b_p_mp);
check_string_option(&buf->b_p_efm);
#endif
check_string_option(&buf->b_p_ep);
check_string_option(&buf->b_p_path);
check_string_option(&buf->b_p_tags);
check_string_option(&buf->b_p_tc);
check_string_option(&buf->b_p_dict);
check_string_option(&buf->b_p_tsr);
#if defined(FEAT_LISP)
check_string_option(&buf->b_p_lw);
#endif
check_string_option(&buf->b_p_bkc);
check_string_option(&buf->b_p_menc);
#if defined(FEAT_VARTABS)
check_string_option(&buf->b_p_vsts);
check_string_option(&buf->b_p_vts);
#endif
}








void
free_string_option(char_u *p)
{
if (p != empty_option)
vim_free(p);
}

void
clear_string_option(char_u **pp)
{
if (*pp != empty_option)
vim_free(*pp);
*pp = empty_option;
}

void
check_string_option(char_u **pp)
{
if (*pp == NULL)
*pp = empty_option;
}




static void
set_string_option_global(
int opt_idx, 
char_u **varp) 
{
char_u **p, *s;


if (is_window_local_option(opt_idx))
p = (char_u **)GLOBAL_WO(varp);
else
p = (char_u **)get_option_var(opt_idx);
if (!is_global_option(opt_idx)
&& p != varp
&& (s = vim_strsave(*varp)) != NULL)
{
free_string_option(*p);
*p = s;
}
}









void
set_string_option_direct(
char_u *name,
int opt_idx,
char_u *val,
int opt_flags, 
int set_sid UNUSED)
{
char_u *s;
char_u **varp;
int both = (opt_flags & (OPT_LOCAL | OPT_GLOBAL)) == 0;
int idx = opt_idx;

if (idx == -1) 
{
idx = findoption(name);
if (idx < 0) 
{
semsg(_(e_intern2), "set_string_option_direct()");
siemsg(_("For option %s"), name);
return;
}
}

if (is_hidden_option(idx)) 
return;

s = vim_strsave(val);
if (s != NULL)
{
varp = (char_u **)get_option_varp_scope(idx,
both ? OPT_LOCAL : opt_flags);
if ((opt_flags & OPT_FREE) && (get_option_flags(idx) & P_ALLOCED))
free_string_option(*varp);
*varp = s;


if (both)
set_string_option_global(idx, varp);

set_option_flag(idx, P_ALLOCED);



if (is_global_local_option(idx) && both)
{
free_string_option(*varp);
*varp = empty_option;
}
#if defined(FEAT_EVAL)
if (set_sid != SID_NONE)
{
sctx_T script_ctx;

if (set_sid == 0)
script_ctx = current_sctx;
else
{
script_ctx.sc_sid = set_sid;
script_ctx.sc_seq = 0;
script_ctx.sc_lnum = 0;
script_ctx.sc_version = 1;
}
set_option_sctx_idx(idx, opt_flags, script_ctx);
}
#endif
}
}





void
set_string_option_direct_in_win(
win_T *wp,
char_u *name,
int opt_idx,
char_u *val,
int opt_flags,
int set_sid)
{
win_T *save_curwin = curwin;

block_autocmds();
curwin = wp;
curbuf = curwin->w_buffer;
set_string_option_direct(name, opt_idx, val, opt_flags, set_sid);
curwin = save_curwin;
curbuf = curwin->w_buffer;
unblock_autocmds();
}





void
set_string_option_direct_in_buf(
buf_T *buf,
char_u *name,
int opt_idx,
char_u *val,
int opt_flags,
int set_sid)
{
buf_T *save_curbuf = curbuf;

block_autocmds();
curbuf = buf;
curwin->w_buffer = curbuf;
set_string_option_direct(name, opt_idx, val, opt_flags, set_sid);
curbuf = save_curbuf;
curwin->w_buffer = curbuf;
unblock_autocmds();
}






char *
set_string_option(
int opt_idx,
char_u *value,
int opt_flags) 
{
char_u *s;
char_u **varp;
char_u *oldval;
#if defined(FEAT_EVAL)
char_u *oldval_l = NULL;
char_u *oldval_g = NULL;
char_u *saved_oldval = NULL;
char_u *saved_oldval_l = NULL;
char_u *saved_oldval_g = NULL;
char_u *saved_newval = NULL;
#endif
char *r = NULL;
int value_checked = FALSE;

if (is_hidden_option(opt_idx)) 
return NULL;

s = vim_strsave(value == NULL ? (char_u *)"" : value);
if (s != NULL)
{
varp = (char_u **)get_option_varp_scope(opt_idx,
(opt_flags & (OPT_LOCAL | OPT_GLOBAL)) == 0
? (is_global_local_option(opt_idx)
? OPT_GLOBAL : OPT_LOCAL)
: opt_flags);
oldval = *varp;
#if defined(FEAT_EVAL)
if ((opt_flags & (OPT_LOCAL | OPT_GLOBAL)) == 0)
{
oldval_l = *(char_u **)get_option_varp_scope(opt_idx, OPT_LOCAL);
oldval_g = *(char_u **)get_option_varp_scope(opt_idx, OPT_GLOBAL);
}
#endif
*varp = s;

#if defined(FEAT_EVAL)
if (!starting
#if defined(FEAT_CRYPT)
&& !is_crypt_key_option(opt_idx)
#endif
)
{
if (oldval_l != NULL)
saved_oldval_l = vim_strsave(oldval_l);
if (oldval_g != NULL)
saved_oldval_g = vim_strsave(oldval_g);
saved_oldval = vim_strsave(oldval);
saved_newval = vim_strsave(s);
}
#endif
if ((r = did_set_string_option(opt_idx, varp, TRUE, oldval, NULL,
opt_flags, &value_checked)) == NULL)
did_set_option(opt_idx, opt_flags, TRUE, value_checked);

#if defined(FEAT_EVAL)

if (r == NULL)
trigger_optionsset_string(opt_idx, opt_flags,
saved_oldval, saved_oldval_l,
saved_oldval_g, saved_newval);
vim_free(saved_oldval);
vim_free(saved_oldval_l);
vim_free(saved_oldval_g);
vim_free(saved_newval);
#endif
}
return r;
}





static int
valid_filetype(char_u *val)
{
return valid_name(val, ".-_");
}

#if defined(FEAT_STL_OPT)




static char *
check_stl_option(char_u *s)
{
int itemcnt = 0;
int groupdepth = 0;
static char errbuf[80];

while (*s && itemcnt < STL_MAX_ITEM)
{

while (*s && *s != '%')
s++;
if (!*s)
break;
s++;
if (*s != '%' && *s != ')')
++itemcnt;
if (*s == '%' || *s == STL_TRUNCMARK || *s == STL_MIDDLEMARK)
{
s++;
continue;
}
if (*s == ')')
{
s++;
if (--groupdepth < 0)
break;
continue;
}
if (*s == '-')
s++;
while (VIM_ISDIGIT(*s))
s++;
if (*s == STL_USER_HL)
continue;
if (*s == '.')
{
s++;
while (*s && VIM_ISDIGIT(*s))
s++;
}
if (*s == '(')
{
groupdepth++;
continue;
}
if (vim_strchr(STL_ALL, *s) == NULL)
{
return illegal_char(errbuf, *s);
}
if (*s == '{')
{
s++;
while (*s != '}' && *s)
s++;
if (*s != '}')
return N_("E540: Unclosed expression sequence");
}
}
if (itemcnt >= STL_MAX_ITEM)
return N_("E541: too many items");
if (groupdepth != 0)
return N_("E542: unbalanced groups");
return NULL;
}
#endif





char *
did_set_string_option(
int opt_idx, 
char_u **varp, 
int new_value_alloced, 
char_u *oldval, 
char *errbuf, 
int opt_flags, 
int *value_checked) 

{
char *errmsg = NULL;
char_u *s, *p;
int did_chartab = FALSE;
char_u **gvarp;
long_u free_oldval = (get_option_flags(opt_idx) & P_ALLOCED);
#if defined(FEAT_GUI)

int redraw_gui_only = FALSE;
#endif
int value_changed = FALSE;
#if defined(FEAT_VTP) && defined(FEAT_TERMGUICOLORS)
int did_swaptcap = FALSE;
#endif



gvarp = (char_u **)get_option_varp_scope(opt_idx, OPT_GLOBAL);


if ((secure
#if defined(HAVE_SANDBOX)
|| sandbox != 0
#endif
) && (get_option_flags(opt_idx) & P_SECURE))
errmsg = e_secure;




else if (((get_option_flags(opt_idx) & P_NFNAME)
&& vim_strpbrk(*varp, (char_u *)(secure
? "/\\*?[|;&<>\r\n" : "/\\*?[<>\r\n")) != NULL)
|| ((get_option_flags(opt_idx) & P_NDNAME)
&& vim_strpbrk(*varp, (char_u *)"*?[|;&<>\r\n") != NULL))
errmsg = e_invarg;


else if (varp == &T_NAME)
{
if (T_NAME[0] == NUL)
errmsg = N_("E529: Cannot set 'term' to empty string");
#if defined(FEAT_GUI)
if (gui.in_use)
errmsg = N_("E530: Cannot change term in GUI");
else if (term_is_gui(T_NAME))
errmsg = N_("E531: Use \":gui\" to start the GUI");
#endif
else if (set_termname(T_NAME) == FAIL)
errmsg = N_("E522: Not found in termcap");
else
{

redraw_later_clear();



opt_idx = findoption((char_u *)"term");
free_oldval = (get_option_flags(opt_idx) & P_ALLOCED);
}
}


else if (gvarp == &p_bkc)
{
char_u *bkc = p_bkc;
unsigned int *flags = &bkc_flags;

if (opt_flags & OPT_LOCAL)
{
bkc = curbuf->b_p_bkc;
flags = &curbuf->b_bkc_flags;
}

if ((opt_flags & OPT_LOCAL) && *bkc == NUL)

*flags = 0;
else
{
if (opt_strings_flags(bkc, p_bkc_values, flags, TRUE) != OK)
errmsg = e_invarg;
if ((((int)*flags & BKC_AUTO) != 0)
+ (((int)*flags & BKC_YES) != 0)
+ (((int)*flags & BKC_NO) != 0) != 1)
{

(void)opt_strings_flags(oldval, p_bkc_values, flags, TRUE);
errmsg = e_invarg;
}
}
}


else if (varp == &p_bex || varp == &p_pm)
{
if (STRCMP(*p_bex == '.' ? p_bex + 1 : p_bex,
*p_pm == '.' ? p_pm + 1 : p_pm) == 0)
errmsg = N_("E589: 'backupext' and 'patchmode' are equal");
}
#if defined(FEAT_LINEBREAK)

else if (varp == &curwin->w_p_briopt)
{
if (briopt_check(curwin) == FAIL)
errmsg = e_invarg;
}
#endif




else if ( varp == &p_isi
|| varp == &(curbuf->b_p_isk)
|| varp == &p_isp
|| varp == &p_isf)
{
if (init_chartab() == FAIL)
{
did_chartab = TRUE; 
errmsg = e_invarg; 
}
}


else if (varp == &p_hf)
{

if (didset_vim)
{
vim_setenv((char_u *)"VIM", (char_u *)"");
didset_vim = FALSE;
}
if (didset_vimruntime)
{
vim_setenv((char_u *)"VIMRUNTIME", (char_u *)"");
didset_vimruntime = FALSE;
}
}

#if defined(FEAT_SYN_HL)

else if (varp == &curwin->w_p_culopt
|| gvarp == &curwin->w_allbuf_opt.wo_culopt)
{
if (**varp == NUL || fill_culopt_flags(*varp, curwin) != OK)
errmsg = e_invarg;
}


else if (varp == &curwin->w_p_cc)
errmsg = check_colorcolumn(curwin);
#endif

#if defined(FEAT_MULTI_LANG)

else if (varp == &p_hlg)
{

for (s = p_hlg; *s != NUL; s += 3)
{
if (s[1] == NUL || ((s[2] != ',' || s[3] == NUL) && s[2] != NUL))
{
errmsg = e_invarg;
break;
}
if (s[2] == NUL)
break;
}
}
#endif


else if (varp == &p_hl)
{
if (highlight_changed() == FAIL)
errmsg = e_invarg; 
}


else if (gvarp == &p_nf)
{
if (check_opt_strings(*varp, p_nf_values, TRUE) != OK)
errmsg = e_invarg;
}

#if defined(FEAT_SESSION)

else if (varp == &p_ssop)
{
if (opt_strings_flags(p_ssop, p_ssop_values, &ssop_flags, TRUE) != OK)
errmsg = e_invarg;
if ((ssop_flags & SSOP_CURDIR) && (ssop_flags & SSOP_SESDIR))
{

(void)opt_strings_flags(oldval, p_ssop_values, &ssop_flags, TRUE);
errmsg = e_invarg;
}
}

else if (varp == &p_vop)
{
if (opt_strings_flags(p_vop, p_ssop_values, &vop_flags, TRUE) != OK)
errmsg = e_invarg;
}
#endif


else if (varp == &p_sbo)
{
if (check_opt_strings(p_sbo, p_scbopt_values, TRUE) != OK)
errmsg = e_invarg;
}


else if (varp == &p_ambw || varp == &p_emoji)
{
if (check_opt_strings(p_ambw, p_ambw_values, FALSE) != OK)
errmsg = e_invarg;
else if (set_chars_option(&p_lcs) != NULL)
errmsg = _("E834: Conflicts with value of 'listchars'");
else if (set_chars_option(&p_fcs) != NULL)
errmsg = _("E835: Conflicts with value of 'fillchars'");
}


else if (varp == &p_bg)
{
if (check_opt_strings(p_bg, p_bg_values, FALSE) == OK)
{
#if defined(FEAT_EVAL)
int dark = (*p_bg == 'd');
#endif

init_highlight(FALSE, FALSE);

#if defined(FEAT_EVAL)
if (dark != (*p_bg == 'd')
&& get_var_value((char_u *)"g:colors_name") != NULL)
{



do_unlet((char_u *)"g:colors_name", TRUE);
free_string_option(p_bg);
p_bg = vim_strsave((char_u *)(dark ? "dark" : "light"));
check_string_option(&p_bg);
init_highlight(FALSE, FALSE);
}
#endif
}
else
errmsg = e_invarg;
}


else if (varp == &p_wim)
{
if (check_opt_wim() == FAIL)
errmsg = e_invarg;
}


else if (varp == &p_wop)
{
if (check_opt_strings(p_wop, p_wop_values, TRUE) != OK)
errmsg = e_invarg;
}

#if defined(FEAT_WAK)

else if (varp == &p_wak)
{
if (*p_wak == NUL
|| check_opt_strings(p_wak, p_wak_values, FALSE) != OK)
errmsg = e_invarg;
#if defined(FEAT_MENU)
#if defined(FEAT_GUI_MOTIF)
else if (gui.in_use)
gui_motif_set_mnemonics(p_wak[0] == 'y' || p_wak[0] == 'm');
#else
#if defined(FEAT_GUI_GTK)
else if (gui.in_use)
gui_gtk_set_mnemonics(p_wak[0] == 'y' || p_wak[0] == 'm');
#endif
#endif
#endif
}
#endif


else if (varp == &p_ei)
{
if (check_ei() == FAIL)
errmsg = e_invarg;
}


else if (varp == &p_enc || gvarp == &p_fenc || varp == &p_tenc
|| gvarp == &p_menc)
{
if (gvarp == &p_fenc)
{
if (!curbuf->b_p_ma && opt_flags != OPT_GLOBAL)
errmsg = e_modifiable;
else if (vim_strchr(*varp, ',') != NULL)


errmsg = e_invarg;
else
{
#if defined(FEAT_TITLE)

redraw_titles();
#endif

ml_setflags(curbuf);
}
}
if (errmsg == NULL)
{

p = enc_canonize(*varp);
if (p != NULL)
{
vim_free(*varp);
*varp = p;
}
if (varp == &p_enc)
{
errmsg = mb_init();
#if defined(FEAT_TITLE)
redraw_titles();
#endif
}
}

#if defined(FEAT_GUI_GTK)
if (errmsg == NULL && varp == &p_tenc && gui.in_use)
{

if (STRCMP(p_tenc, "utf-8") != 0)
errmsg = N_("E617: Cannot be changed in the GTK+ 2 GUI");
}
#endif

if (errmsg == NULL)
{
#if defined(FEAT_KEYMAP)


if (varp == &p_enc && *curbuf->b_p_keymap != NUL)
(void)keymap_init();
#endif




if (((varp == &p_enc && *p_tenc != NUL) || varp == &p_tenc))
{
if (convert_setup(&input_conv, p_tenc, p_enc) == FAIL
|| convert_setup(&output_conv, p_enc, p_tenc) == FAIL)
{
semsg(_("E950: Cannot convert between %s and %s"),
p_tenc, p_enc);
errmsg = e_invarg;
}
}

#if defined(MSWIN)

if (varp == &p_enc)
init_homedir();
#endif
}
}

#if defined(FEAT_POSTSCRIPT)
else if (varp == &p_penc)
{

p = enc_canonize(p_penc);
if (p != NULL)
{
vim_free(p_penc);
p_penc = p;
}
else
{

for (s = p_penc; *s != NUL; s++)
{
if (*s == '_')
*s = '-';
else
*s = TOLOWER_ASC(*s);
}
}
}
#endif

#if defined(FEAT_XIM) && defined(FEAT_GUI_GTK)
else if (varp == &p_imak)
{
if (!im_xim_isvalid_imactivate())
errmsg = e_invarg;
}
#endif

#if defined(FEAT_KEYMAP)
else if (varp == &curbuf->b_p_keymap)
{
if (!valid_filetype(*varp))
errmsg = e_invarg;
else
{
int secure_save = secure;



secure = 0;


errmsg = keymap_init();

secure = secure_save;



*value_checked = TRUE;
}

if (errmsg == NULL)
{
if (*curbuf->b_p_keymap != NUL)
{

curbuf->b_p_iminsert = B_IMODE_LMAP;
if (curbuf->b_p_imsearch != B_IMODE_USE_INSERT)
curbuf->b_p_imsearch = B_IMODE_LMAP;
}
else
{

if (curbuf->b_p_iminsert == B_IMODE_LMAP)
curbuf->b_p_iminsert = B_IMODE_NONE;
if (curbuf->b_p_imsearch == B_IMODE_LMAP)
curbuf->b_p_imsearch = B_IMODE_USE_INSERT;
}
if ((opt_flags & OPT_LOCAL) == 0)
{
set_iminsert_global();
set_imsearch_global();
}
status_redraw_curbuf();
}
}
#endif


else if (gvarp == &p_ff)
{
if (!curbuf->b_p_ma && !(opt_flags & OPT_GLOBAL))
errmsg = e_modifiable;
else if (check_opt_strings(*varp, p_ff_values, FALSE) != OK)
errmsg = e_invarg;
else
{

if (get_fileformat(curbuf) == EOL_DOS)
curbuf->b_p_tx = TRUE;
else
curbuf->b_p_tx = FALSE;
#if defined(FEAT_TITLE)
redraw_titles();
#endif

ml_setflags(curbuf);


if (get_fileformat(curbuf) == EOL_MAC || *oldval == 'm')
redraw_curbuf_later(NOT_VALID);
}
}


else if (varp == &p_ffs)
{
if (check_opt_strings(p_ffs, p_ff_values, TRUE) != OK)
errmsg = e_invarg;
else
{

if (*p_ffs == NUL)
p_ta = FALSE;
else
p_ta = TRUE;
}
}

#if defined(FEAT_CRYPT)

else if (gvarp == &p_key)
{


remove_key_from_history();

if (STRCMP(curbuf->b_p_key, oldval) != 0)

ml_set_crypt_key(curbuf, oldval,
*curbuf->b_p_cm == NUL ? p_cm : curbuf->b_p_cm);
}

else if (gvarp == &p_cm)
{
if (opt_flags & OPT_LOCAL)
p = curbuf->b_p_cm;
else
p = p_cm;
if (check_opt_strings(p, p_cm_values, TRUE) != OK)
errmsg = e_invarg;
else if (crypt_self_test() == FAIL)
errmsg = e_invarg;
else
{

if (*p_cm == NUL)
{
if (new_value_alloced)
free_string_option(p_cm);
p_cm = vim_strsave((char_u *)"zip");
new_value_alloced = TRUE;
}



if ((opt_flags & (OPT_LOCAL | OPT_GLOBAL)) == 0)
{
free_string_option(curbuf->b_p_cm);
curbuf->b_p_cm = empty_option;
}




if ((opt_flags & OPT_LOCAL) && *oldval == NUL)
s = p_cm; 
else
s = oldval;
if (*curbuf->b_p_cm == NUL)
p = p_cm; 
else
p = curbuf->b_p_cm;
if (STRCMP(s, p) != 0)
ml_set_crypt_key(curbuf, curbuf->b_p_key, s);



if ((opt_flags & OPT_GLOBAL) && STRCMP(p_cm, oldval) != 0)
{
buf_T *buf;

FOR_ALL_BUFFERS(buf)
if (buf != curbuf && *buf->b_p_cm == NUL)
ml_set_crypt_key(buf, buf->b_p_key, oldval);
}
}
}
#endif


else if (gvarp == &p_mps)
{
if (has_mbyte)
{
for (p = *varp; *p != NUL; ++p)
{
int x2 = -1;
int x3 = -1;

if (*p != NUL)
p += mb_ptr2len(p);
if (*p != NUL)
x2 = *p++;
if (*p != NUL)
{
x3 = mb_ptr2char(p);
p += mb_ptr2len(p);
}
if (x2 != ':' || x3 == -1 || (*p != NUL && *p != ','))
{
errmsg = e_invarg;
break;
}
if (*p == NUL)
break;
}
}
else
{

for (p = *varp; *p != NUL; p += 4)
{
if (p[1] != ':' || p[2] == NUL || (p[3] != NUL && p[3] != ','))
{
errmsg = e_invarg;
break;
}
if (p[3] == NUL)
break;
}
}
}


else if (gvarp == &p_com)
{
for (s = *varp; *s; )
{
while (*s && *s != ':')
{
if (vim_strchr((char_u *)COM_ALL, *s) == NULL
&& !VIM_ISDIGIT(*s) && *s != '-')
{
errmsg = illegal_char(errbuf, *s);
break;
}
++s;
}
if (*s++ == NUL)
errmsg = N_("E524: Missing colon");
else if (*s == ',' || *s == NUL)
errmsg = N_("E525: Zero length string");
if (errmsg != NULL)
break;
while (*s && *s != ',')
{
if (*s == '\\' && s[1] != NUL)
++s;
++s;
}
s = skip_to_option_part(s);
}
}


else if (varp == &p_lcs)
{
errmsg = set_chars_option(varp);
}


else if (varp == &p_fcs)
{
errmsg = set_chars_option(varp);
}

#if defined(FEAT_CMDWIN)

else if (varp == &p_cedit)
{
errmsg = check_cedit();
}
#endif


else if (varp == &p_vfile)
{
verbose_stop();
if (*p_vfile != NUL && verbose_open() == FAIL)
errmsg = e_invarg;
}

#if defined(FEAT_VIMINFO)

else if (varp == &p_viminfo)
{
for (s = p_viminfo; *s;)
{

if (vim_strchr((char_u *)"!\"%'/:<@cfhnrs", *s) == NULL)
{
errmsg = illegal_char(errbuf, *s);
break;
}
if (*s == 'n') 
break;
else if (*s == 'r') 
{
while (*++s && *s != ',')
;
}
else if (*s == '%')
{

while (vim_isdigit(*++s))
;
}
else if (*s == '!' || *s == 'h' || *s == 'c')
++s; 
else 
{
while (vim_isdigit(*++s))
;

if (!VIM_ISDIGIT(*(s - 1)))
{
if (errbuf != NULL)
{
sprintf(errbuf, _("E526: Missing number after <%s>"),
transchar_byte(*(s - 1)));
errmsg = errbuf;
}
else
errmsg = "";
break;
}
}
if (*s == ',')
++s;
else if (*s)
{
if (errbuf != NULL)
errmsg = N_("E527: Missing comma");
else
errmsg = "";
break;
}
}
if (*p_viminfo && errmsg == NULL && get_viminfo_parameter('\'') < 0)
errmsg = N_("E528: Must specify a ' value");
}
#endif 


else if (istermoption_idx(opt_idx) && full_screen)
{

if (varp == &T_CCO)
{
int colors = atoi((char *)T_CCO);




if (colors != t_colors)
{
t_colors = colors;
if (t_colors <= 1)
{
if (new_value_alloced)
vim_free(T_CCO);
T_CCO = empty_option;
}
#if defined(FEAT_VTP) && defined(FEAT_TERMGUICOLORS)
if (is_term_win32())
{
swap_tcap();
did_swaptcap = TRUE;
}
#endif

init_highlight(TRUE, FALSE);
}
}
ttest(FALSE);
if (varp == &T_ME)
{
out_str(T_ME);
redraw_later(CLEAR);
#if defined(MSWIN) && (!defined(FEAT_GUI_MSWIN) || defined(VIMDLL))



#if defined(VIMDLL)
if (!gui.in_use && !gui.starting)
#endif
mch_set_normal_colors();
#endif
}
if (varp == &T_BE && termcap_active)
{
if (*T_BE == NUL)


out_str(T_BD);
else
out_str(T_BE);
}
}

#if defined(FEAT_LINEBREAK)

else if (gvarp == &p_sbr)
{
for (s = *varp; *s; )
{
if (ptr2cells(s) != 1)
errmsg = N_("E595: 'showbreak' contains unprintable or wide character");
MB_PTR_ADV(s);
}
}
#endif

#if defined(FEAT_GUI)

else if (varp == &p_guifont)
{
if (gui.in_use)
{
p = p_guifont;
#if defined(FEAT_GUI_GTK)



if (STRCMP(p, "*") == 0)
{
p = gui_mch_font_dialog(oldval);

if (new_value_alloced)
free_string_option(p_guifont);

p_guifont = (p != NULL) ? p : vim_strsave(oldval);
new_value_alloced = TRUE;
}
#endif
if (p != NULL && gui_init_font(p_guifont, FALSE) != OK)
{
#if defined(FEAT_GUI_MSWIN) || defined(FEAT_GUI_PHOTON)
if (STRCMP(p_guifont, "*") == 0)
{


if (new_value_alloced)
free_string_option(p_guifont);
p_guifont = vim_strsave(oldval);
new_value_alloced = TRUE;
}
else
#endif
errmsg = N_("E596: Invalid font(s)");
}
}
redraw_gui_only = TRUE;
}
#if defined(FEAT_XFONTSET)
else if (varp == &p_guifontset)
{
if (STRCMP(p_guifontset, "*") == 0)
errmsg = N_("E597: can't select fontset");
else if (gui.in_use && gui_init_font(p_guifontset, TRUE) != OK)
errmsg = N_("E598: Invalid fontset");
redraw_gui_only = TRUE;
}
#endif
else if (varp == &p_guifontwide)
{
if (STRCMP(p_guifontwide, "*") == 0)
errmsg = N_("E533: can't select wide font");
else if (gui_get_wide_font() == FAIL)
errmsg = N_("E534: Invalid wide font");
redraw_gui_only = TRUE;
}
#endif

#if defined(CURSOR_SHAPE)

else if (varp == &p_guicursor)
errmsg = parse_shape_opt(SHAPE_CURSOR);
#endif

#if defined(FEAT_MOUSESHAPE)

else if (varp == &p_mouseshape)
{
errmsg = parse_shape_opt(SHAPE_MOUSE);
update_mouseshape(-1);
}
#endif

#if defined(FEAT_PRINTER)
else if (varp == &p_popt)
errmsg = parse_printoptions();
#if defined(FEAT_POSTSCRIPT)
else if (varp == &p_pmfn)
errmsg = parse_printmbfont();
#endif
#endif

#if defined(FEAT_LANGMAP)

else if (varp == &p_langmap)
langmap_set();
#endif

#if defined(FEAT_LINEBREAK)

else if (varp == &p_breakat)
fill_breakat_flags();
#endif

#if defined(FEAT_TITLE)

else if (varp == &p_titlestring || varp == &p_iconstring)
{
#if defined(FEAT_STL_OPT)
int flagval = (varp == &p_titlestring) ? STL_IN_TITLE : STL_IN_ICON;


if (vim_strchr(*varp, '%') && check_stl_option(*varp) == NULL)
stl_syntax |= flagval;
else
stl_syntax &= ~flagval;
#endif
did_set_title();
}
#endif

#if defined(FEAT_GUI)

else if (varp == &p_go)
{
gui_init_which_components(oldval);
redraw_gui_only = TRUE;
}
#endif

#if defined(FEAT_GUI_TABLINE)

else if (varp == &p_gtl)
{
redraw_tabline = TRUE;
redraw_gui_only = TRUE;
}

else if (varp == &p_gtt)
{
redraw_gui_only = TRUE;
}
#endif

#if defined(UNIX) || defined(VMS)

else if (varp == &p_ttym)
{


mch_setmouse(FALSE);
if (opt_strings_flags(p_ttym, p_ttym_values, &ttym_flags, FALSE) != OK)
errmsg = e_invarg;
else
check_mouse_termcode();
if (termcap_active)
setmouse(); 
}
#endif


else if (varp == &p_sel)
{
if (*p_sel == NUL
|| check_opt_strings(p_sel, p_sel_values, FALSE) != OK)
errmsg = e_invarg;
}


else if (varp == &p_slm)
{
if (check_opt_strings(p_slm, p_slm_values, TRUE) != OK)
errmsg = e_invarg;
}

#if defined(FEAT_BROWSE)

else if (varp == &p_bsdir)
{
if (check_opt_strings(p_bsdir, p_bsdir_values, FALSE) != OK
&& !mch_isdir(p_bsdir))
errmsg = e_invarg;
}
#endif


else if (varp == &p_km)
{
if (check_opt_strings(p_km, p_km_values, TRUE) != OK)
errmsg = e_invarg;
else
{
km_stopsel = (vim_strchr(p_km, 'o') != NULL);
km_startsel = (vim_strchr(p_km, 'a') != NULL);
}
}


else if (varp == &p_mousem)
{
if (check_opt_strings(p_mousem, p_mousem_values, FALSE) != OK)
errmsg = e_invarg;
#if defined(FEAT_GUI_MOTIF) && defined(FEAT_MENU) && (XmVersion <= 1002)
else if (*p_mousem != *oldval)


gui_motif_update_mousemodel(root_menu);
#endif
}


else if (varp == &p_swb)
{
if (opt_strings_flags(p_swb, p_swb_values, &swb_flags, TRUE) != OK)
errmsg = e_invarg;
}


else if (varp == &p_debug)
{
if (check_opt_strings(p_debug, p_debug_values, TRUE) != OK)
errmsg = e_invarg;
}


else if (varp == &p_dy)
{
if (opt_strings_flags(p_dy, p_dy_values, &dy_flags, TRUE) != OK)
errmsg = e_invarg;
else
(void)init_chartab();

}


else if (varp == &p_ead)
{
if (check_opt_strings(p_ead, p_ead_values, FALSE) != OK)
errmsg = e_invarg;
}

#if defined(FEAT_CLIPBOARD)

else if (varp == &p_cb)
errmsg = check_clipboard_option();
#endif

#if defined(FEAT_SPELL)


else if (varp == &(curwin->w_s->b_p_spl)
|| varp == &(curwin->w_s->b_p_spf))
{
int is_spellfile = varp == &(curwin->w_s->b_p_spf);

if ((is_spellfile && !valid_spellfile(*varp))
|| (!is_spellfile && !valid_spellang(*varp)))
errmsg = e_invarg;
else
errmsg = did_set_spell_option(is_spellfile);
}

else if (varp == &(curwin->w_s->b_p_spc))
{
errmsg = compile_cap_prog(curwin->w_s);
}

else if (varp == &p_sps)
{
if (spell_check_sps() != OK)
errmsg = e_invarg;
}

else if (varp == &p_msm)
{
if (spell_check_msm() != OK)
errmsg = e_invarg;
}
#endif


else if (gvarp == &p_bh)
{
if (check_opt_strings(curbuf->b_p_bh, p_bufhidden_values, FALSE) != OK)
errmsg = e_invarg;
}


else if (gvarp == &p_bt)
{
if (check_opt_strings(curbuf->b_p_bt, p_buftype_values, FALSE) != OK)
errmsg = e_invarg;
else
{
if (curwin->w_status_height)
{
curwin->w_redr_status = TRUE;
redraw_later(VALID);
}
curbuf->b_help = (curbuf->b_p_bt[0] == 'h');
#if defined(FEAT_TITLE)
redraw_titles();
#endif
}
}

#if defined(FEAT_STL_OPT)

else if (gvarp == &p_stl || varp == &p_ruf)
{
int wid;

if (varp == &p_ruf) 
ru_wid = 0;
s = *varp;
if (varp == &p_ruf && *s == '%')
{

if (*++s == '-') 
s++;
wid = getdigits(&s);
if (wid && *s == '(' && (errmsg = check_stl_option(p_ruf)) == NULL)
ru_wid = wid;
else
errmsg = check_stl_option(p_ruf);
}

else if (varp == &p_ruf || s[0] != '%' || s[1] != '!')
errmsg = check_stl_option(s);
if (varp == &p_ruf && errmsg == NULL)
comp_col();
}
#endif


else if (gvarp == &p_cpt)
{
for (s = *varp; *s;)
{
while (*s == ',' || *s == ' ')
s++;
if (!*s)
break;
if (vim_strchr((char_u *)".wbuksid]tU", *s) == NULL)
{
errmsg = illegal_char(errbuf, *s);
break;
}
if (*++s != NUL && *s != ',' && *s != ' ')
{
if (s[-1] == 'k' || s[-1] == 's')
{

while (*s && *s != ',' && *s != ' ')
{
if (*s == '\\' && s[1] != NUL)
++s;
++s;
}
}
else
{
if (errbuf != NULL)
{
sprintf((char *)errbuf,
_("E535: Illegal character after <%c>"),
*--s);
errmsg = errbuf;
}
else
errmsg = "";
break;
}
}
}
}


else if (varp == &p_cot)
{
if (check_opt_strings(p_cot, p_cot_values, TRUE) != OK)
errmsg = e_invarg;
else
completeopt_was_set();
}

#if defined(BACKSLASH_IN_FILENAME)

else if (gvarp == &p_csl)
{
if (check_opt_strings(p_csl, p_csl_values, FALSE) != OK
|| check_opt_strings(curbuf->b_p_csl, p_csl_values, FALSE) != OK)
errmsg = e_invarg;
}
#endif

#if defined(FEAT_SIGNS)

else if (varp == &curwin->w_p_scl)
{
if (check_opt_strings(*varp, p_scl_values, FALSE) != OK)
errmsg = e_invarg;


if (((*oldval == 'n' && *(oldval + 1) == 'u')
|| (*curwin->w_p_scl == 'n' && *(curwin->w_p_scl + 1) =='u'))
&& (curwin->w_p_nu || curwin->w_p_rnu))
curwin->w_nrwidth_line_count = 0;
}
#endif


#if defined(FEAT_TOOLBAR) && !defined(FEAT_GUI_MSWIN)

else if (varp == &p_toolbar)
{
if (opt_strings_flags(p_toolbar, p_toolbar_values,
&toolbar_flags, TRUE) != OK)
errmsg = e_invarg;
else
{
out_flush();
gui_mch_show_toolbar((toolbar_flags &
(TOOLBAR_TEXT | TOOLBAR_ICONS)) != 0);
}
}
#endif

#if defined(FEAT_TOOLBAR) && defined(FEAT_GUI_GTK)

else if (varp == &p_tbis)
{
if (opt_strings_flags(p_tbis, p_tbis_values, &tbis_flags, FALSE) != OK)
errmsg = e_invarg;
else
{
out_flush();
gui_mch_show_toolbar((toolbar_flags &
(TOOLBAR_TEXT | TOOLBAR_ICONS)) != 0);
}
}
#endif


else if (varp == &p_pt)
{
if (*p_pt)
{
(void)replace_termcodes(p_pt, &p,
REPTERM_FROM_PART | REPTERM_DO_LT, NULL);
if (p != NULL)
{
if (new_value_alloced)
free_string_option(p_pt);
p_pt = p;
new_value_alloced = TRUE;
}
}
}


else if (varp == &p_bs)
{
if (VIM_ISDIGIT(*p_bs))
{
if (*p_bs > '2' || p_bs[1] != NUL)
errmsg = e_invarg;
}
else if (check_opt_strings(p_bs, p_bs_values, TRUE) != OK)
errmsg = e_invarg;
}
else if (varp == &p_bo)
{
if (opt_strings_flags(p_bo, p_bo_values, &bo_flags, TRUE) != OK)
errmsg = e_invarg;
}


else if (gvarp == &p_tc)
{
unsigned int *flags;

if (opt_flags & OPT_LOCAL)
{
p = curbuf->b_p_tc;
flags = &curbuf->b_tc_flags;
}
else
{
p = p_tc;
flags = &tc_flags;
}

if ((opt_flags & OPT_LOCAL) && *p == NUL)

*flags = 0;
else if (*p == NUL
|| opt_strings_flags(p, p_tc_values, flags, FALSE) != OK)
errmsg = e_invarg;
}


else if (varp == &p_cmp)
{
if (opt_strings_flags(p_cmp, p_cmp_values, &cmp_flags, TRUE) != OK)
errmsg = e_invarg;
}

#if defined(FEAT_DIFF)

else if (varp == &p_dip)
{
if (diffopt_changed() == FAIL)
errmsg = e_invarg;
}
#endif

#if defined(FEAT_FOLDING)

else if (gvarp == &curwin->w_allbuf_opt.wo_fdm)
{
if (check_opt_strings(*varp, p_fdm_values, FALSE) != OK
|| *curwin->w_p_fdm == NUL)
errmsg = e_invarg;
else
{
foldUpdateAll(curwin);
if (foldmethodIsDiff(curwin))
newFoldLevel();
}
}
#if defined(FEAT_EVAL)

else if (varp == &curwin->w_p_fde)
{
if (foldmethodIsExpr(curwin))
foldUpdateAll(curwin);
}
#endif

else if (gvarp == &curwin->w_allbuf_opt.wo_fmr)
{
p = vim_strchr(*varp, ',');
if (p == NULL)
errmsg = N_("E536: comma required");
else if (p == *varp || p[1] == NUL)
errmsg = e_invarg;
else if (foldmethodIsMarker(curwin))
foldUpdateAll(curwin);
}

else if (gvarp == &p_cms)
{
if (**varp != NUL && strstr((char *)*varp, "%s") == NULL)
errmsg = N_("E537: 'commentstring' must be empty or contain %s");
}

else if (varp == &p_fdo)
{
if (opt_strings_flags(p_fdo, p_fdo_values, &fdo_flags, TRUE) != OK)
errmsg = e_invarg;
}

else if (varp == &p_fcl)
{
if (check_opt_strings(p_fcl, p_fcl_values, TRUE) != OK)
errmsg = e_invarg;
}

else if (gvarp == &curwin->w_allbuf_opt.wo_fdi)
{
if (foldmethodIsIndent(curwin))
foldUpdateAll(curwin);
}
#endif


else if (varp == &p_ve)
{
if (opt_strings_flags(p_ve, p_ve_values, &ve_flags, TRUE) != OK)
errmsg = e_invarg;
else if (STRCMP(p_ve, oldval) != 0)
{


validate_virtcol();
coladvance(curwin->w_virtcol);
}
}

#if defined(FEAT_CSCOPE) && defined(FEAT_QUICKFIX)
else if (varp == &p_csqf)
{
if (p_csqf != NULL)
{
p = p_csqf;
while (*p != NUL)
{
if (vim_strchr((char_u *)CSQF_CMDS, *p) == NULL
|| p[1] == NUL
|| vim_strchr((char_u *)CSQF_FLAGS, p[1]) == NULL
|| (p[2] != NUL && p[2] != ','))
{
errmsg = e_invarg;
break;
}
else if (p[2] == NUL)
break;
else
p += 3;
}
}
}
#endif

#if defined(FEAT_CINDENT)

else if (gvarp == &p_cino)
{

parse_cino(curbuf);
}
#endif

#if defined(FEAT_RENDER_OPTIONS)

else if (varp == &p_rop)
{
if (!gui_mch_set_rendering_options(p_rop))
errmsg = e_invarg;
}
#endif

else if (gvarp == &p_ft)
{
if (!valid_filetype(*varp))
errmsg = e_invarg;
else
{
value_changed = STRCMP(oldval, *varp) != 0;



*value_checked = TRUE;
}
}

#if defined(FEAT_SYN_HL)
else if (gvarp == &p_syn)
{
if (!valid_filetype(*varp))
errmsg = e_invarg;
else
{
value_changed = STRCMP(oldval, *varp) != 0;



*value_checked = TRUE;
}
}
#endif

#if defined(FEAT_TERMINAL)

else if (varp == &curwin->w_p_twk)
{
if (*curwin->w_p_twk != NUL
&& string_to_key(curwin->w_p_twk, TRUE) == 0)
errmsg = e_invarg;
}

else if (varp == &curwin->w_p_tws)
{
if (*curwin->w_p_tws != NUL)
{
p = skipdigits(curwin->w_p_tws);
if (p == curwin->w_p_tws
|| (*p != 'x' && *p != '*')
|| *skipdigits(p + 1) != NUL)
errmsg = e_invarg;
}
}

else if (varp == &curwin->w_p_wcr)
{
if (curwin->w_buffer->b_term != NULL)
term_update_colors();
}
#if defined(MSWIN)

else if (varp == &p_twt)
{
if (check_opt_strings(*varp, p_twt_values, FALSE) != OK)
errmsg = e_invarg;
}
#endif
#endif

#if defined(FEAT_VARTABS)

else if (varp == &(curbuf->b_p_vsts))
{
char_u *cp;

if (!(*varp)[0] || ((*varp)[0] == '0' && !(*varp)[1]))
{
if (curbuf->b_p_vsts_array)
{
vim_free(curbuf->b_p_vsts_array);
curbuf->b_p_vsts_array = 0;
}
}
else
{
for (cp = *varp; *cp; ++cp)
{
if (vim_isdigit(*cp))
continue;
if (*cp == ',' && cp > *varp && *(cp-1) != ',')
continue;
errmsg = e_invarg;
break;
}
if (errmsg == NULL)
{
int *oldarray = curbuf->b_p_vsts_array;
if (tabstop_set(*varp, &(curbuf->b_p_vsts_array)))
{
if (oldarray)
vim_free(oldarray);
}
else
errmsg = e_invarg;
}
}
}


else if (varp == &(curbuf->b_p_vts))
{
char_u *cp;

if (!(*varp)[0] || ((*varp)[0] == '0' && !(*varp)[1]))
{
if (curbuf->b_p_vts_array)
{
vim_free(curbuf->b_p_vts_array);
curbuf->b_p_vts_array = NULL;
}
}
else
{
for (cp = *varp; *cp; ++cp)
{
if (vim_isdigit(*cp))
continue;
if (*cp == ',' && cp > *varp && *(cp-1) != ',')
continue;
errmsg = e_invarg;
break;
}
if (errmsg == NULL)
{
int *oldarray = curbuf->b_p_vts_array;

if (tabstop_set(*varp, &(curbuf->b_p_vts_array)))
{
vim_free(oldarray);
#if defined(FEAT_FOLDING)
if (foldmethodIsIndent(curwin))
foldUpdateAll(curwin);
#endif
}
else
errmsg = e_invarg;
}
}
}
#endif

#if defined(FEAT_PROP_POPUP)

else if (varp == &p_pvp)
{
if (parse_previewpopup(NULL) == FAIL)
errmsg = e_invarg;
}
#if defined(FEAT_QUICKFIX)

else if (varp == &p_cpp)
{
if (parse_completepopup(NULL) == FAIL)
errmsg = e_invarg;
}
#endif
#endif


else
{
p = NULL;
if (varp == &p_ww) 
p = (char_u *)WW_ALL;
if (varp == &p_shm) 
p = (char_u *)SHM_ALL;
else if (varp == &(p_cpo)) 
p = (char_u *)CPO_ALL;
else if (varp == &(curbuf->b_p_fo)) 
p = (char_u *)FO_ALL;
#if defined(FEAT_CONCEAL)
else if (varp == &curwin->w_p_cocu) 
p = (char_u *)COCU_ALL;
#endif
else if (varp == &p_mouse) 
{
p = (char_u *)MOUSE_ALL;
}
#if defined(FEAT_GUI)
else if (varp == &p_go) 
p = (char_u *)GO_ALL;
#endif
if (p != NULL)
{
for (s = *varp; *s; ++s)
if (vim_strchr(p, *s) == NULL)
{
errmsg = illegal_char(errbuf, *s);
break;
}
}
}


if (errmsg != NULL)
{
if (new_value_alloced)
free_string_option(*varp);
*varp = oldval;

if (did_chartab)
(void)init_chartab();
if (varp == &p_hl)
(void)highlight_changed();
}
else
{
#if defined(FEAT_EVAL)

set_option_sctx_idx(opt_idx, opt_flags, current_sctx);
#endif



if (free_oldval)
free_string_option(oldval);
if (new_value_alloced)
set_option_flag(opt_idx, P_ALLOCED);
else
clear_option_flag(opt_idx, P_ALLOCED);

if ((opt_flags & (OPT_LOCAL | OPT_GLOBAL)) == 0
&& is_global_local_option(opt_idx))
{


p = get_option_varp_scope(opt_idx, OPT_LOCAL);
free_string_option(*(char_u **)p);
*(char_u **)p = empty_option;
}


else if (!(opt_flags & OPT_LOCAL) && opt_flags != OPT_GLOBAL)
set_string_option_global(opt_idx, varp);


#if defined(FEAT_SYN_HL)

if (varp == &(curbuf->b_p_syn))
{
static int syn_recursive = 0;

++syn_recursive;


apply_autocmds(EVENT_SYNTAX, curbuf->b_p_syn, curbuf->b_fname,
value_changed || syn_recursive == 1, curbuf);
curbuf->b_flags |= BF_SYN_SET;
--syn_recursive;
}
#endif
else if (varp == &(curbuf->b_p_ft))
{



if (!(opt_flags & OPT_MODELINE) || value_changed)
{
static int ft_recursive = 0;
int secure_save = secure;



secure = 0;

++ft_recursive;
did_filetype = TRUE;


apply_autocmds(EVENT_FILETYPE, curbuf->b_p_ft, curbuf->b_fname,
value_changed || ft_recursive == 1, curbuf);
--ft_recursive;

if (varp != &(curbuf->b_p_ft))
varp = NULL;

secure = secure_save;
}
}
#if defined(FEAT_SPELL)
if (varp == &(curwin->w_s->b_p_spl))
{
char_u fname[200];
char_u *q = curwin->w_s->b_p_spl;


if (STRNCMP(q, "cjk,", 4) == 0)
q += 4;





for (p = q; *p != NUL; ++p)
if (!ASCII_ISALNUM(*p) && *p != '-')
break;
if (p > q)
{
vim_snprintf((char *)fname, 200, "spell/%.*s.vim",
(int)(p - q), q);
source_runtime(fname, DIP_ALL);
}
}
#endif
}

if (varp == &p_mouse)
{
if (*p_mouse == NUL)
mch_setmouse(FALSE); 
else
setmouse(); 
}

if (curwin->w_curswant != MAXCOL
&& (get_option_flags(opt_idx) & (P_CURSWANT | P_RALL)) != 0)
curwin->w_set_curswant = TRUE;

#if defined(FEAT_GUI)

if (!redraw_gui_only || gui.in_use)
#endif
check_redraw(get_option_flags(opt_idx));

#if defined(FEAT_VTP) && defined(FEAT_TERMGUICOLORS)
if (did_swaptcap)
{
set_termname((char_u *)"win32");
init_highlight(TRUE, FALSE);
}
#endif

return errmsg;
}







static int
check_opt_strings(
char_u *val,
char **values,
int list) 
{
return opt_strings_flags(val, values, NULL, list);
}








static int
opt_strings_flags(
char_u *val, 
char **values, 
unsigned *flagp,
int list) 
{
int i;
int len;
unsigned new_flags = 0;

while (*val)
{
for (i = 0; ; ++i)
{
if (values[i] == NULL) 
return FAIL;

len = (int)STRLEN(values[i]);
if (STRNCMP(values[i], val, len) == 0
&& ((list && val[len] == ',') || val[len] == NUL))
{
val += len + (val[len] == ',');
new_flags |= (1 << i);
break; 
}
}
}
if (flagp != NULL)
*flagp = new_flags;

return OK;
}




int
check_ff_value(char_u *p)
{
return check_opt_strings(p, p_ff_values, FALSE);
}
