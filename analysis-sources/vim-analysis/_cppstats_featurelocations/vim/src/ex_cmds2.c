












#include "vim.h"
#include "version.h"







int
autowrite(buf_T *buf, int forceit)
{
int r;
bufref_T bufref;

if (!(p_aw || p_awa) || !p_write
#if defined(FEAT_QUICKFIX)

|| bt_dontwrite(buf)
#endif
|| (!forceit && buf->b_p_ro) || buf->b_ffname == NULL)
return FAIL;
set_bufref(&bufref, buf);
r = buf_write_all(buf, forceit);



if (bufref_valid(&bufref) && bufIsChanged(buf))
r = FAIL;
return r;
}




void
autowrite_all(void)
{
buf_T *buf;

if (!(p_aw || p_awa) || !p_write)
return;
FOR_ALL_BUFFERS(buf)
if (bufIsChanged(buf) && !buf->b_p_ro && !bt_dontwrite(buf))
{
bufref_T bufref;

set_bufref(&bufref, buf);

(void)buf_write_all(buf, FALSE);


if (!bufref_valid(&bufref))
buf = firstbuf;
}
}





int
check_changed(buf_T *buf, int flags)
{
int forceit = (flags & CCGD_FORCEIT);
bufref_T bufref;

set_bufref(&bufref, buf);

if ( !forceit
&& bufIsChanged(buf)
&& ((flags & CCGD_MULTWIN) || buf->b_nwindows <= 1)
&& (!(flags & CCGD_AW) || autowrite(buf, forceit) == FAIL))
{
#if defined(FEAT_GUI_DIALOG) || defined(FEAT_CON_DIALOG)
if ((p_confirm || cmdmod.confirm) && p_write)
{
buf_T *buf2;
int count = 0;

if (flags & CCGD_ALLBUF)
FOR_ALL_BUFFERS(buf2)
if (bufIsChanged(buf2)
&& (buf2->b_ffname != NULL
#if defined(FEAT_BROWSE)
|| cmdmod.browse
#endif
))
++count;
if (!bufref_valid(&bufref))

return FALSE;

dialog_changed(buf, count > 1);

if (!bufref_valid(&bufref))

return FALSE;
return bufIsChanged(buf);
}
#endif
if (flags & CCGD_EXCMD)
no_write_message();
else
no_write_message_nobang(curbuf);
return TRUE;
}
return FALSE;
}

#if defined(FEAT_GUI_DIALOG) || defined(FEAT_CON_DIALOG) || defined(PROTO)

#if defined(FEAT_BROWSE) || defined(PROTO)



void
browse_save_fname(buf_T *buf)
{
if (buf->b_fname == NULL)
{
char_u *fname;

fname = do_browse(BROWSE_SAVE, (char_u *)_("Save As"),
NULL, NULL, NULL, NULL, buf);
if (fname != NULL)
{
if (setfname(buf, fname, NULL, TRUE) == OK)
buf->b_flags |= BF_NOTEDITED;
vim_free(fname);
}
}
}
#endif





void
dialog_changed(
buf_T *buf,
int checkall) 
{
char_u buff[DIALOG_MSG_SIZE];
int ret;
buf_T *buf2;
exarg_T ea;

dialog_msg(buff, _("Save changes to \"%s\"?"), buf->b_fname);
if (checkall)
ret = vim_dialog_yesnoallcancel(VIM_QUESTION, NULL, buff, 1);
else
ret = vim_dialog_yesnocancel(VIM_QUESTION, NULL, buff, 1);



vim_memset(&ea, 0, sizeof(ea));

if (ret == VIM_YES)
{
#if defined(FEAT_BROWSE)

browse_save_fname(buf);
#endif
if (buf->b_fname != NULL && check_overwrite(&ea, buf,
buf->b_fname, buf->b_ffname, FALSE) == OK)

(void)buf_write_all(buf, FALSE);
}
else if (ret == VIM_NO)
{
unchanged(buf, TRUE, FALSE);
}
else if (ret == VIM_ALL)
{





FOR_ALL_BUFFERS(buf2)
{
if (bufIsChanged(buf2)
&& (buf2->b_ffname != NULL
#if defined(FEAT_BROWSE)
|| cmdmod.browse
#endif
)
&& !buf2->b_p_ro)
{
bufref_T bufref;

set_bufref(&bufref, buf2);
#if defined(FEAT_BROWSE)

browse_save_fname(buf2);
#endif
if (buf2->b_fname != NULL && check_overwrite(&ea, buf2,
buf2->b_fname, buf2->b_ffname, FALSE) == OK)

(void)buf_write_all(buf2, FALSE);


if (!bufref_valid(&bufref))
buf2 = firstbuf;
}
}
}
else if (ret == VIM_DISCARDALL)
{



FOR_ALL_BUFFERS(buf2)
unchanged(buf2, TRUE, FALSE);
}
}
#endif





int
can_abandon(buf_T *buf, int forceit)
{
return ( buf_hide(buf)
|| !bufIsChanged(buf)
|| buf->b_nwindows > 1
|| autowrite(buf, forceit) == OK
|| forceit);
}




static void
add_bufnum(int *bufnrs, int *bufnump, int nr)
{
int i;

for (i = 0; i < *bufnump; ++i)
if (bufnrs[i] == nr)
return;
bufnrs[*bufnump] = nr;
*bufnump = *bufnump + 1;
}







int
check_changed_any(
int hidden, 
int unload)
{
int ret = FALSE;
buf_T *buf;
int save;
int i;
int bufnum = 0;
int bufcount = 0;
int *bufnrs;
tabpage_T *tp;
win_T *wp;


FOR_ALL_BUFFERS(buf)
++bufcount;

if (bufcount == 0)
return FALSE;

bufnrs = ALLOC_MULT(int, bufcount);
if (bufnrs == NULL)
return FALSE;


bufnrs[bufnum++] = curbuf->b_fnum;


FOR_ALL_WINDOWS(wp)
if (wp->w_buffer != curbuf)
add_bufnum(bufnrs, &bufnum, wp->w_buffer->b_fnum);


FOR_ALL_TABPAGES(tp)
if (tp != curtab)
FOR_ALL_WINDOWS_IN_TAB(tp, wp)
add_bufnum(bufnrs, &bufnum, wp->w_buffer->b_fnum);


FOR_ALL_BUFFERS(buf)
add_bufnum(bufnrs, &bufnum, buf->b_fnum);

for (i = 0; i < bufnum; ++i)
{
buf = buflist_findnr(bufnrs[i]);
if (buf == NULL)
continue;
if ((!hidden || buf->b_nwindows == 0) && bufIsChanged(buf))
{
bufref_T bufref;

set_bufref(&bufref, buf);
#if defined(FEAT_TERMINAL)
if (term_job_running(buf->b_term))
{
if (term_try_stop_job(buf) == FAIL)
break;
}
else
#endif


if (check_changed(buf, (p_awa ? CCGD_AW : 0)
| CCGD_MULTWIN
| CCGD_ALLBUF) && bufref_valid(&bufref))
break; 
}
}

if (i >= bufnum)
goto theend;


ret = TRUE;
exiting = FALSE;
#if defined(FEAT_GUI_DIALOG) || defined(FEAT_CON_DIALOG)



if (!(p_confirm || cmdmod.confirm))
#endif
{




if (vgetc_busy > 0)
{
msg_row = cmdline_row;
msg_col = 0;
msg_didout = FALSE;
}
if (
#if defined(FEAT_TERMINAL)
term_job_running(buf->b_term)
? semsg(_("E947: Job still running in buffer \"%s\""),
buf->b_fname)
:
#endif
semsg(_("E162: No write since last change for buffer \"%s\""),
buf_spname(buf) != NULL ? buf_spname(buf) : buf->b_fname))
{
save = no_wait_return;
no_wait_return = FALSE;
wait_return(FALSE);
no_wait_return = save;
}
}


if (buf != curbuf)
FOR_ALL_TAB_WINDOWS(tp, wp)
if (wp->w_buffer == buf)
{
bufref_T bufref;

set_bufref(&bufref, buf);

goto_tabpage_win(tp, wp);


if (!bufref_valid(&bufref))
goto theend;
goto buf_found;
}
buf_found:


if (buf != curbuf)
set_curbuf(buf, unload ? DOBUF_UNLOAD : DOBUF_GOTO);

theend:
vim_free(bufnrs);
return ret;
}





int
check_fname(void)
{
if (curbuf->b_ffname == NULL)
{
emsg(_(e_noname));
return FAIL;
}
return OK;
}






int
buf_write_all(buf_T *buf, int forceit)
{
int retval;
buf_T *old_curbuf = curbuf;

retval = (buf_write(buf, buf->b_ffname, buf->b_fname,
(linenr_T)1, buf->b_ml.ml_line_count, NULL,
FALSE, forceit, TRUE, FALSE));
if (curbuf != old_curbuf)
{
msg_source(HL_ATTR(HLF_W));
msg(_("Warning: Entered other buffer unexpectedly (check autocommands)"));
}
return retval;
}




void
ex_listdo(exarg_T *eap)
{
int i;
win_T *wp;
tabpage_T *tp;
buf_T *buf = curbuf;
int next_fnum = 0;
#if defined(FEAT_SYN_HL)
char_u *save_ei = NULL;
#endif
char_u *p_shm_save;
#if defined(FEAT_QUICKFIX)
int qf_size = 0;
int qf_idx;
#endif

#if !defined(FEAT_QUICKFIX)
if (eap->cmdidx == CMD_cdo || eap->cmdidx == CMD_ldo ||
eap->cmdidx == CMD_cfdo || eap->cmdidx == CMD_lfdo)
{
ex_ni(eap);
return;
}
#endif

#if defined(FEAT_SYN_HL)
if (eap->cmdidx != CMD_windo && eap->cmdidx != CMD_tabdo)
{


save_ei = au_event_disable(",Syntax");

FOR_ALL_BUFFERS(buf)
buf->b_flags &= ~BF_SYN_SET;
buf = curbuf;
}
#endif
#if defined(FEAT_CLIPBOARD)
start_global_changes();
#endif

if (eap->cmdidx == CMD_windo
|| eap->cmdidx == CMD_tabdo
|| buf_hide(curbuf)
|| !check_changed(curbuf, CCGD_AW
| (eap->forceit ? CCGD_FORCEIT : 0)
| CCGD_EXCMD))
{
i = 0;

wp = firstwin;
tp = first_tabpage;
switch (eap->cmdidx)
{
case CMD_windo:
for ( ; wp != NULL && i + 1 < eap->line1; wp = wp->w_next)
i++;
break;
case CMD_tabdo:
for( ; tp != NULL && i + 1 < eap->line1; tp = tp->tp_next)
i++;
break;
case CMD_argdo:
i = eap->line1 - 1;
break;
default:
break;
}

if (eap->cmdidx == CMD_bufdo)
{

for (buf = firstbuf; buf != NULL && (buf->b_fnum < eap->line1
|| !buf->b_p_bl); buf = buf->b_next)
if (buf->b_fnum > eap->line2)
{
buf = NULL;
break;
}
if (buf != NULL)
goto_buffer(eap, DOBUF_FIRST, FORWARD, buf->b_fnum);
}
#if defined(FEAT_QUICKFIX)
else if (eap->cmdidx == CMD_cdo || eap->cmdidx == CMD_ldo
|| eap->cmdidx == CMD_cfdo || eap->cmdidx == CMD_lfdo)
{
qf_size = qf_get_valid_size(eap);
if (qf_size <= 0 || eap->line1 > qf_size)
buf = NULL;
else
{
ex_cc(eap);

buf = curbuf;
i = eap->line1 - 1;
if (eap->addr_count <= 0)

eap->line2 = qf_size;
}
}
#endif
else
setpcmark();
listcmd_busy = TRUE; 

while (!got_int && buf != NULL)
{
if (eap->cmdidx == CMD_argdo)
{

if (i == ARGCOUNT)
break;


if (curwin->w_arg_idx != i || !editing_arg_idx(curwin))
{


p_shm_save = vim_strsave(p_shm);
set_option_value((char_u *)"shm", 0L, (char_u *)"", 0);
do_argfile(eap, i);
set_option_value((char_u *)"shm", 0L, p_shm_save, 0);
vim_free(p_shm_save);
}
if (curwin->w_arg_idx != i)
break;
}
else if (eap->cmdidx == CMD_windo)
{

if (!win_valid(wp))
break;
win_goto(wp);
if (curwin != wp)
break; 
wp = curwin->w_next;
}
else if (eap->cmdidx == CMD_tabdo)
{

if (!valid_tabpage(tp))
break;
goto_tabpage_tp(tp, TRUE, TRUE);
tp = tp->tp_next;
}
else if (eap->cmdidx == CMD_bufdo)
{


next_fnum = -1;
for (buf = curbuf->b_next; buf != NULL; buf = buf->b_next)
if (buf->b_p_bl)
{
next_fnum = buf->b_fnum;
break;
}
}

++i;


do_cmdline(eap->arg, eap->getline, eap->cookie,
DOCMD_VERBOSE + DOCMD_NOWAIT);

if (eap->cmdidx == CMD_bufdo)
{

if (next_fnum < 0 || next_fnum > eap->line2)
break;

FOR_ALL_BUFFERS(buf)
if (buf->b_fnum == next_fnum)
break;
if (buf == NULL)
break;



p_shm_save = vim_strsave(p_shm);
set_option_value((char_u *)"shm", 0L, (char_u *)"", 0);
goto_buffer(eap, DOBUF_FIRST, FORWARD, next_fnum);
set_option_value((char_u *)"shm", 0L, p_shm_save, 0);
vim_free(p_shm_save);


if (curbuf->b_fnum != next_fnum)
break;
}

#if defined(FEAT_QUICKFIX)
if (eap->cmdidx == CMD_cdo || eap->cmdidx == CMD_ldo
|| eap->cmdidx == CMD_cfdo || eap->cmdidx == CMD_lfdo)
{
if (i >= qf_size || i >= eap->line2)
break;

qf_idx = qf_get_cur_idx(eap);

ex_cnext(eap);


if (qf_get_cur_idx(eap) == qf_idx)
break;
}
#endif

if (eap->cmdidx == CMD_windo)
{
validate_cursor(); 


if (curwin->w_p_scb)
do_check_scrollbind(TRUE);
}

if (eap->cmdidx == CMD_windo || eap->cmdidx == CMD_tabdo)
if (i+1 > eap->line2)
break;
if (eap->cmdidx == CMD_argdo && i >= eap->line2)
break;
}
listcmd_busy = FALSE;
}

#if defined(FEAT_SYN_HL)
if (save_ei != NULL)
{
buf_T *bnext;
aco_save_T aco;

au_event_restore(save_ei);

for (buf = firstbuf; buf != NULL; buf = bnext)
{
bnext = buf->b_next;
if (buf->b_nwindows > 0 && (buf->b_flags & BF_SYN_SET))
{
buf->b_flags &= ~BF_SYN_SET;



if (buf == curbuf)
apply_autocmds(EVENT_SYNTAX, curbuf->b_p_syn,
curbuf->b_fname, TRUE, curbuf);
else
{
aucmd_prepbuf(&aco, buf);
apply_autocmds(EVENT_SYNTAX, buf->b_p_syn,
buf->b_fname, TRUE, buf);
aucmd_restbuf(&aco);
}


bnext = firstbuf;
}
}
}
#endif
#if defined(FEAT_CLIPBOARD)
end_global_changes();
#endif
}

#if defined(FEAT_EVAL)



void
ex_compiler(exarg_T *eap)
{
char_u *buf;
char_u *old_cur_comp = NULL;
char_u *p;

if (*eap->arg == NUL)
{

do_cmdline_cmd((char_u *)"echo globpath(&rtp, 'compiler/*.vim')");

}
else
{
buf = alloc(STRLEN(eap->arg) + 14);
if (buf != NULL)
{
if (eap->forceit)
{

do_cmdline_cmd((char_u *)
"command -nargs=* CompilerSet set <args>");
}
else
{






old_cur_comp = get_var_value((char_u *)"g:current_compiler");
if (old_cur_comp != NULL)
old_cur_comp = vim_strsave(old_cur_comp);
do_cmdline_cmd((char_u *)
"command -nargs=* CompilerSet setlocal <args>");
}
do_unlet((char_u *)"g:current_compiler", TRUE);
do_unlet((char_u *)"b:current_compiler", TRUE);

sprintf((char *)buf, "compiler/%s.vim", eap->arg);
if (source_runtime(buf, DIP_ALL) == FAIL)
semsg(_("E666: compiler not supported: %s"), eap->arg);
vim_free(buf);

do_cmdline_cmd((char_u *)":delcommand CompilerSet");


p = get_var_value((char_u *)"g:current_compiler");
if (p != NULL)
set_internal_string_var((char_u *)"b:current_compiler", p);


if (!eap->forceit)
{
if (old_cur_comp != NULL)
{
set_internal_string_var((char_u *)"g:current_compiler",
old_cur_comp);
vim_free(old_cur_comp);
}
else
do_unlet((char_u *)"g:current_compiler", TRUE);
}
}
}
}
#endif

#if defined(FEAT_PYTHON3) || defined(FEAT_PYTHON) || defined(PROTO)

#if (defined(FEAT_PYTHON) && defined(FEAT_PYTHON3)) || defined(PROTO)



void
init_pyxversion(void)
{
if (p_pyx == 0)
{
if (python3_enabled(FALSE))
p_pyx = 3;
else if (python_enabled(FALSE))
p_pyx = 2;
}
}
#endif










static int
requires_py_version(char_u *filename)
{
FILE *file;
int requires_py_version = 0;
int i, lines;

lines = (int)p_mls;
if (lines < 0)
lines = 5;

file = mch_fopen((char *)filename, "r");
if (file != NULL)
{
for (i = 0; i < lines; i++)
{
if (vim_fgets(IObuff, IOSIZE, file))
break;
if (i == 0 && IObuff[0] == '#' && IObuff[1] == '!')
{

if (strstr((char *)IObuff + 2, "python2") != NULL)
{
requires_py_version = 2;
break;
}
if (strstr((char *)IObuff + 2, "python3") != NULL)
{
requires_py_version = 3;
break;
}
}
IObuff[21] = '\0';
if (STRCMP("#requires python 2.x", IObuff) == 0)
{
requires_py_version = 2;
break;
}
if (STRCMP("#requires python 3.x", IObuff) == 0)
{
requires_py_version = 3;
break;
}
}
fclose(file);
}
return requires_py_version;
}





static void
source_pyx_file(exarg_T *eap, char_u *fname)
{
exarg_T ex;
int v = requires_py_version(fname);

#if defined(FEAT_PYTHON) && defined(FEAT_PYTHON3)
init_pyxversion();
#endif
if (v == 0)
{
#if defined(FEAT_PYTHON) && defined(FEAT_PYTHON3)

v = p_pyx;
#elif defined(FEAT_PYTHON)
v = 2;
#elif defined(FEAT_PYTHON3)
v = 3;
#endif
}





if (eap == NULL)
vim_memset(&ex, 0, sizeof(ex));
else
ex = *eap;
ex.arg = fname;
ex.cmd = (char_u *)(v == 2 ? "pyfile" : "pyfile3");

if (v == 2)
{
#if defined(FEAT_PYTHON)
ex_pyfile(&ex);
#else
vim_snprintf((char *)IObuff, IOSIZE,
_("W20: Required python version 2.x not supported, ignoring file: %s"),
fname);
msg((char *)IObuff);
#endif
return;
}
else
{
#if defined(FEAT_PYTHON3)
ex_py3file(&ex);
#else
vim_snprintf((char *)IObuff, IOSIZE,
_("W21: Required python version 3.x not supported, ignoring file: %s"),
fname);
msg((char *)IObuff);
#endif
return;
}
}




void
ex_pyxfile(exarg_T *eap)
{
source_pyx_file(eap, eap->arg);
}




void
ex_pyx(exarg_T *eap)
{
#if defined(FEAT_PYTHON) && defined(FEAT_PYTHON3)
init_pyxversion();
if (p_pyx == 2)
ex_python(eap);
else
ex_py3(eap);
#elif defined(FEAT_PYTHON)
ex_python(eap);
#elif defined(FEAT_PYTHON3)
ex_py3(eap);
#endif
}




void
ex_pyxdo(exarg_T *eap)
{
#if defined(FEAT_PYTHON) && defined(FEAT_PYTHON3)
init_pyxversion();
if (p_pyx == 2)
ex_pydo(eap);
else
ex_py3do(eap);
#elif defined(FEAT_PYTHON)
ex_pydo(eap);
#elif defined(FEAT_PYTHON3)
ex_py3do(eap);
#endif
}

#endif




void
ex_checktime(exarg_T *eap)
{
buf_T *buf;
int save_no_check_timestamps = no_check_timestamps;

no_check_timestamps = 0;
if (eap->addr_count == 0) 
check_timestamps(FALSE);
else
{
buf = buflist_findnr((int)eap->line2);
if (buf != NULL) 
(void)buf_check_timestamp(buf, FALSE);
}
no_check_timestamps = save_no_check_timestamps;
}

#if (defined(HAVE_LOCALE_H) || defined(X_LOCALE)) && (defined(FEAT_EVAL) || defined(FEAT_MULTI_LANG))

#define HAVE_GET_LOCALE_VAL
static char_u *
get_locale_val(int what)
{
char_u *loc;


loc = (char_u *)setlocale(what, NULL);

#if defined(MSWIN)
if (loc != NULL)
{
char_u *p;



p = vim_strchr(loc, '=');
if (p != NULL)
{
loc = ++p;
while (*p != NUL) 
{
if (*p < ' ' || *p == ';')
{
*p = NUL;
break;
}
++p;
}
}
}
#endif

return loc;
}
#endif


#if defined(MSWIN)





static char_u *
gettext_lang(char_u *name)
{
int i;
static char *(mtable[]) = {
"afrikaans", "af",
"czech", "cs",
"dutch", "nl",
"german", "de",
"english_united kingdom", "en_GB",
"spanish", "es",
"french", "fr",
"italian", "it",
"japanese", "ja",
"korean", "ko",
"norwegian", "no",
"polish", "pl",
"russian", "ru",
"slovak", "sk",
"swedish", "sv",
"ukrainian", "uk",
"chinese_china", "zh_CN",
"chinese_taiwan", "zh_TW",
NULL};

for (i = 0; mtable[i] != NULL; i += 2)
if (STRNICMP(mtable[i], name, STRLEN(mtable[i])) == 0)
return (char_u *)mtable[i + 1];
return name;
}
#endif

#if defined(FEAT_MULTI_LANG) || defined(PROTO)




static int
is_valid_mess_lang(char_u *lang)
{
return lang != NULL && ASCII_ISALPHA(lang[0]) && ASCII_ISALPHA(lang[1]);
}





char_u *
get_mess_lang(void)
{
char_u *p;

#if defined(HAVE_GET_LOCALE_VAL)
#if defined(LC_MESSAGES)
p = get_locale_val(LC_MESSAGES);
#else




p = get_locale_val(LC_COLLATE);
#endif
#else
p = mch_getenv((char_u *)"LC_ALL");
if (!is_valid_mess_lang(p))
{
p = mch_getenv((char_u *)"LC_MESSAGES");
if (!is_valid_mess_lang(p))
p = mch_getenv((char_u *)"LANG");
}
#endif
#if defined(MSWIN)
p = gettext_lang(p);
#endif
return is_valid_mess_lang(p) ? p : NULL;
}
#endif


#if (defined(FEAT_EVAL) && !((defined(HAVE_LOCALE_H) || defined(X_LOCALE)) && defined(LC_MESSAGES))) || ((defined(HAVE_LOCALE_H) || defined(X_LOCALE)) && !defined(LC_MESSAGES))






static char_u *
get_mess_env(void)
{
char_u *p;

p = mch_getenv((char_u *)"LC_ALL");
if (p == NULL || *p == NUL)
{
p = mch_getenv((char_u *)"LC_MESSAGES");
if (p == NULL || *p == NUL)
{
p = mch_getenv((char_u *)"LANG");
if (p != NULL && VIM_ISDIGIT(*p))
p = NULL; 
#if defined(HAVE_GET_LOCALE_VAL)
if (p == NULL || *p == NUL)
p = get_locale_val(LC_CTYPE);
#endif
}
}
return p;
}
#endif

#if defined(FEAT_EVAL) || defined(PROTO)





void
set_lang_var(void)
{
char_u *loc;

#if defined(HAVE_GET_LOCALE_VAL)
loc = get_locale_val(LC_CTYPE);
#else

loc = (char_u *)"C";
#endif
set_vim_var_string(VV_CTYPE, loc, -1);



#if defined(HAVE_GET_LOCALE_VAL) && defined(LC_MESSAGES)
loc = get_locale_val(LC_MESSAGES);
#else
loc = get_mess_env();
#endif
set_vim_var_string(VV_LANG, loc, -1);

#if defined(HAVE_GET_LOCALE_VAL)
loc = get_locale_val(LC_TIME);
#endif
set_vim_var_string(VV_LC_TIME, loc, -1);
}
#endif

#if defined(HAVE_LOCALE_H) || defined(X_LOCALE) 



void
ex_language(exarg_T *eap)
{
char *loc;
char_u *p;
char_u *name;
int what = LC_ALL;
char *whatstr = "";
#if defined(LC_MESSAGES)
#define VIM_LC_MESSAGES LC_MESSAGES
#else
#define VIM_LC_MESSAGES 6789
#endif

name = eap->arg;




p = skiptowhite(eap->arg);
if ((*p == NUL || VIM_ISWHITE(*p)) && p - eap->arg >= 3)
{
if (STRNICMP(eap->arg, "messages", p - eap->arg) == 0)
{
what = VIM_LC_MESSAGES;
name = skipwhite(p);
whatstr = "messages ";
}
else if (STRNICMP(eap->arg, "ctype", p - eap->arg) == 0)
{
what = LC_CTYPE;
name = skipwhite(p);
whatstr = "ctype ";
}
else if (STRNICMP(eap->arg, "time", p - eap->arg) == 0)
{
what = LC_TIME;
name = skipwhite(p);
whatstr = "time ";
}
}

if (*name == NUL)
{
#if !defined(LC_MESSAGES)
if (what == VIM_LC_MESSAGES)
p = get_mess_env();
else
#endif
p = (char_u *)setlocale(what, NULL);
if (p == NULL || *p == NUL)
p = (char_u *)"Unknown";
smsg(_("Current %slanguage: \"%s\""), whatstr, p);
}
else
{
#if !defined(LC_MESSAGES)
if (what == VIM_LC_MESSAGES)
loc = "";
else
#endif
{
loc = setlocale(what, (char *)name);
#if defined(FEAT_FLOAT) && defined(LC_NUMERIC)

setlocale(LC_NUMERIC, "C");
#endif
}
if (loc == NULL)
semsg(_("E197: Cannot set language to \"%s\""), name);
else
{
#if defined(HAVE_NL_MSG_CAT_CNTR)


extern int _nl_msg_cat_cntr;

++_nl_msg_cat_cntr;
#endif

vim_setenv((char_u *)"LC_ALL", (char_u *)"");

if (what != LC_TIME)
{




if (what == LC_ALL)
{
vim_setenv((char_u *)"LANG", name);


vim_setenv((char_u *)"LANGUAGE", (char_u *)"");
#if defined(MSWIN)



setlocale(LC_CTYPE, "C");
#endif
}
if (what != LC_CTYPE)
{
char_u *mname;
#if defined(MSWIN)
mname = gettext_lang(name);
#else
mname = name;
#endif
vim_setenv((char_u *)"LC_MESSAGES", mname);
#if defined(FEAT_MULTI_LANG)
set_helplang_default(mname);
#endif
}
}

#if defined(FEAT_EVAL)

set_lang_var();
#endif
#if defined(FEAT_TITLE)
maketitle();
#endif
}
}
}

static char_u **locales = NULL; 

#if !defined(MSWIN)
static int did_init_locales = FALSE;





static char_u **
find_locales(void)
{
garray_T locales_ga;
char_u *loc;



char_u *locale_a = get_cmd_output((char_u *)"locale -a",
NULL, SHELL_SILENT, NULL);
if (locale_a == NULL)
return NULL;
ga_init2(&locales_ga, sizeof(char_u *), 20);



loc = (char_u *)strtok((char *)locale_a, "\n");

while (loc != NULL)
{
if (ga_grow(&locales_ga, 1) == FAIL)
break;
loc = vim_strsave(loc);
if (loc == NULL)
break;

((char_u **)locales_ga.ga_data)[locales_ga.ga_len++] = loc;
loc = (char_u *)strtok(NULL, "\n");
}
vim_free(locale_a);
if (ga_grow(&locales_ga, 1) == FAIL)
{
ga_clear(&locales_ga);
return NULL;
}
((char_u **)locales_ga.ga_data)[locales_ga.ga_len] = NULL;
return (char_u **)locales_ga.ga_data;
}
#endif




static void
init_locales(void)
{
#if !defined(MSWIN)
if (!did_init_locales)
{
did_init_locales = TRUE;
locales = find_locales();
}
#endif
}

#if defined(EXITFREE) || defined(PROTO)
void
free_locales(void)
{
int i;
if (locales != NULL)
{
for (i = 0; locales[i] != NULL; i++)
vim_free(locales[i]);
VIM_CLEAR(locales);
}
}
#endif





char_u *
get_lang_arg(expand_T *xp UNUSED, int idx)
{
if (idx == 0)
return (char_u *)"messages";
if (idx == 1)
return (char_u *)"ctype";
if (idx == 2)
return (char_u *)"time";

init_locales();
if (locales == NULL)
return NULL;
return locales[idx - 3];
}




char_u *
get_locales(expand_T *xp UNUSED, int idx)
{
init_locales();
if (locales == NULL)
return NULL;
return locales[idx];
}

#endif
