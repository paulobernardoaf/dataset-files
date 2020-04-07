












#include "vim.h"












static xfmark_T namedfm[NMARKS + EXTRA_MARKS]; 

static void fname2fnum(xfmark_T *fm);
static void fmarks_check_one(xfmark_T *fm, char_u *name, buf_T *buf);
static char_u *mark_line(pos_T *mp, int lead_len);
static void show_one_mark(int, char_u *, pos_T *, char_u *, int current);
static void mark_adjust_internal(linenr_T line1, linenr_T line2, long amount,
long amount_after, int adjust_folds);





int
setmark(int c)
{
return setmark_pos(c, &curwin->w_cursor, curbuf->b_fnum);
}






int
setmark_pos(int c, pos_T *pos, int fnum)
{
int i;
buf_T *buf;


if (c < 0)
return FAIL;

if (c == '\'' || c == '`')
{
if (pos == &curwin->w_cursor)
{
setpcmark();

curwin->w_prev_pcmark = curwin->w_pcmark;
}
else
curwin->w_pcmark = *pos;
return OK;
}

buf = buflist_findnr(fnum);
if (buf == NULL)
return FAIL;

if (c == '"')
{
buf->b_last_cursor = *pos;
return OK;
}



if (c == '[')
{
buf->b_op_start = *pos;
return OK;
}
if (c == ']')
{
buf->b_op_end = *pos;
return OK;
}

if (c == '<' || c == '>')
{
if (c == '<')
buf->b_visual.vi_start = *pos;
else
buf->b_visual.vi_end = *pos;
if (buf->b_visual.vi_mode == NUL)

buf->b_visual.vi_mode = 'v';
return OK;
}

if (ASCII_ISLOWER(c))
{
i = c - 'a';
buf->b_namedm[i] = *pos;
return OK;
}
if (ASCII_ISUPPER(c) || VIM_ISDIGIT(c))
{
if (VIM_ISDIGIT(c))
i = c - '0' + NMARKS;
else
i = c - 'A';
namedfm[i].fmark.mark = *pos;
namedfm[i].fmark.fnum = fnum;
VIM_CLEAR(namedfm[i].fname);
#if defined(FEAT_VIMINFO)
namedfm[i].time_set = vim_time();
#endif
return OK;
}
return FAIL;
}





void
setpcmark(void)
{
#if defined(FEAT_JUMPLIST)
int i;
xfmark_T *fm;
#endif
#if defined(JUMPLIST_ROTATE)
xfmark_T tempmark;
#endif


if (global_busy || listcmd_busy || cmdmod.keepjumps)
return;

curwin->w_prev_pcmark = curwin->w_pcmark;
curwin->w_pcmark = curwin->w_cursor;

#if defined(FEAT_JUMPLIST)
#if defined(JUMPLIST_ROTATE)





if (curwin->w_jumplistidx < curwin->w_jumplistlen)
++curwin->w_jumplistidx;
while (curwin->w_jumplistidx < curwin->w_jumplistlen)
{
tempmark = curwin->w_jumplist[curwin->w_jumplistlen - 1];
for (i = curwin->w_jumplistlen - 1; i > 0; --i)
curwin->w_jumplist[i] = curwin->w_jumplist[i - 1];
curwin->w_jumplist[0] = tempmark;
++curwin->w_jumplistidx;
}
#endif


if (++curwin->w_jumplistlen > JUMPLISTSIZE)
{
curwin->w_jumplistlen = JUMPLISTSIZE;
vim_free(curwin->w_jumplist[0].fname);
for (i = 1; i < JUMPLISTSIZE; ++i)
curwin->w_jumplist[i - 1] = curwin->w_jumplist[i];
}
curwin->w_jumplistidx = curwin->w_jumplistlen;
fm = &curwin->w_jumplist[curwin->w_jumplistlen - 1];

fm->fmark.mark = curwin->w_pcmark;
fm->fmark.fnum = curbuf->b_fnum;
fm->fname = NULL;
#if defined(FEAT_VIMINFO)
fm->time_set = vim_time();
#endif
#endif
}







void
checkpcmark(void)
{
if (curwin->w_prev_pcmark.lnum != 0
&& (EQUAL_POS(curwin->w_pcmark, curwin->w_cursor)
|| curwin->w_pcmark.lnum == 0))
{
curwin->w_pcmark = curwin->w_prev_pcmark;
curwin->w_prev_pcmark.lnum = 0; 
}
}

#if defined(FEAT_JUMPLIST) || defined(PROTO)



pos_T *
movemark(int count)
{
pos_T *pos;
xfmark_T *jmp;

cleanup_jumplist(curwin, TRUE);

if (curwin->w_jumplistlen == 0) 
return (pos_T *)NULL;

for (;;)
{
if (curwin->w_jumplistidx + count < 0
|| curwin->w_jumplistidx + count >= curwin->w_jumplistlen)
return (pos_T *)NULL;






if (curwin->w_jumplistidx == curwin->w_jumplistlen)
{
setpcmark();
--curwin->w_jumplistidx; 
if (curwin->w_jumplistidx + count < 0)
return (pos_T *)NULL;
}

curwin->w_jumplistidx += count;

jmp = curwin->w_jumplist + curwin->w_jumplistidx;
if (jmp->fmark.fnum == 0)
fname2fnum(jmp);
if (jmp->fmark.fnum != curbuf->b_fnum)
{

if (buflist_findnr(jmp->fmark.fnum) == NULL)
{ 
count += count < 0 ? -1 : 1;
continue;
}
if (buflist_getfile(jmp->fmark.fnum, jmp->fmark.mark.lnum,
0, FALSE) == FAIL)
return (pos_T *)NULL;

curwin->w_cursor = jmp->fmark.mark;
pos = (pos_T *)-1;
}
else
pos = &(jmp->fmark.mark);
return pos;
}
}




pos_T *
movechangelist(int count)
{
int n;

if (curbuf->b_changelistlen == 0) 
return (pos_T *)NULL;

n = curwin->w_changelistidx;
if (n + count < 0)
{
if (n == 0)
return (pos_T *)NULL;
n = 0;
}
else if (n + count >= curbuf->b_changelistlen)
{
if (n == curbuf->b_changelistlen - 1)
return (pos_T *)NULL;
n = curbuf->b_changelistlen - 1;
}
else
n += count;
curwin->w_changelistidx = n;
return curbuf->b_changelist + n;
}
#endif












pos_T *
getmark_buf(buf_T *buf, int c, int changefile)
{
return getmark_buf_fnum(buf, c, changefile, NULL);
}

pos_T *
getmark(int c, int changefile)
{
return getmark_buf_fnum(curbuf, c, changefile, NULL);
}

pos_T *
getmark_buf_fnum(
buf_T *buf,
int c,
int changefile,
int *fnum)
{
pos_T *posp;
pos_T *startp, *endp;
static pos_T pos_copy;

posp = NULL;



if (c < 0)
return posp;
#if !defined(EBCDIC)
if (c > '~') 
;
else
#endif
if (c == '\'' || c == '`') 
{
pos_copy = curwin->w_pcmark; 
posp = &pos_copy; 
}
else if (c == '"') 
posp = &(buf->b_last_cursor);
else if (c == '^') 
posp = &(buf->b_last_insert);
else if (c == '.') 
posp = &(buf->b_last_change);
else if (c == '[') 
posp = &(buf->b_op_start);
else if (c == ']') 
posp = &(buf->b_op_end);
else if (c == '{' || c == '}') 
{
pos_T pos;
oparg_T oa;
int slcb = listcmd_busy;

pos = curwin->w_cursor;
listcmd_busy = TRUE; 
if (findpar(&oa.inclusive,
c == '}' ? FORWARD : BACKWARD, 1L, NUL, FALSE))
{
pos_copy = curwin->w_cursor;
posp = &pos_copy;
}
curwin->w_cursor = pos;
listcmd_busy = slcb;
}
else if (c == '(' || c == ')') 
{
pos_T pos;
int slcb = listcmd_busy;

pos = curwin->w_cursor;
listcmd_busy = TRUE; 
if (findsent(c == ')' ? FORWARD : BACKWARD, 1L))
{
pos_copy = curwin->w_cursor;
posp = &pos_copy;
}
curwin->w_cursor = pos;
listcmd_busy = slcb;
}
else if (c == '<' || c == '>') 
{
startp = &buf->b_visual.vi_start;
endp = &buf->b_visual.vi_end;
if (((c == '<') == LT_POS(*startp, *endp) || endp->lnum == 0)
&& startp->lnum != 0)
posp = startp;
else
posp = endp;



if (buf->b_visual.vi_mode == 'V')
{
pos_copy = *posp;
posp = &pos_copy;
if (c == '<')
pos_copy.col = 0;
else
pos_copy.col = MAXCOL;
pos_copy.coladd = 0;
}
}
else if (ASCII_ISLOWER(c)) 
{
posp = &(buf->b_namedm[c - 'a']);
}
else if (ASCII_ISUPPER(c) || VIM_ISDIGIT(c)) 
{
if (VIM_ISDIGIT(c))
c = c - '0' + NMARKS;
else
c -= 'A';
posp = &(namedfm[c].fmark.mark);

if (namedfm[c].fmark.fnum == 0)
fname2fnum(&namedfm[c]);

if (fnum != NULL)
*fnum = namedfm[c].fmark.fnum;
else if (namedfm[c].fmark.fnum != buf->b_fnum)
{

posp = &pos_copy;

if (namedfm[c].fmark.mark.lnum != 0
&& changefile && namedfm[c].fmark.fnum)
{
if (buflist_getfile(namedfm[c].fmark.fnum,
(linenr_T)1, GETF_SETMARK, FALSE) == OK)
{

curwin->w_cursor = namedfm[c].fmark.mark;
return (pos_T *)-1;
}
pos_copy.lnum = -1; 
}
else
pos_copy.lnum = 0; 

}
}

return posp;
}






pos_T *
getnextmark(
pos_T *startpos, 
int dir, 
int begin_line)
{
int i;
pos_T *result = NULL;
pos_T pos;

pos = *startpos;





if (dir == BACKWARD && begin_line)
pos.col = 0;
else if (dir == FORWARD && begin_line)
pos.col = MAXCOL;

for (i = 0; i < NMARKS; i++)
{
if (curbuf->b_namedm[i].lnum > 0)
{
if (dir == FORWARD)
{
if ((result == NULL || LT_POS(curbuf->b_namedm[i], *result))
&& LT_POS(pos, curbuf->b_namedm[i]))
result = &curbuf->b_namedm[i];
}
else
{
if ((result == NULL || LT_POS(*result, curbuf->b_namedm[i]))
&& LT_POS(curbuf->b_namedm[i], pos))
result = &curbuf->b_namedm[i];
}
}
}

return result;
}






static void
fname2fnum(xfmark_T *fm)
{
char_u *p;

if (fm->fname != NULL)
{




if (fm->fname[0] == '~' && (fm->fname[1] == '/'
#if defined(BACKSLASH_IN_FILENAME)
|| fm->fname[1] == '\\'
#endif
))
{
int len;

expand_env((char_u *)"~/", NameBuff, MAXPATHL);
len = (int)STRLEN(NameBuff);
vim_strncpy(NameBuff + len, fm->fname + 2, MAXPATHL - len - 1);
}
else
vim_strncpy(NameBuff, fm->fname, MAXPATHL - 1);


mch_dirname(IObuff, IOSIZE);
p = shorten_fname(NameBuff, IObuff);


(void)buflist_new(NameBuff, p, (linenr_T)1, 0);
}
}






void
fmarks_check_names(buf_T *buf)
{
char_u *name;
int i;
#if defined(FEAT_JUMPLIST)
win_T *wp;
#endif

if (buf->b_ffname == NULL)
return;

name = home_replace_save(buf, buf->b_ffname);
if (name == NULL)
return;

for (i = 0; i < NMARKS + EXTRA_MARKS; ++i)
fmarks_check_one(&namedfm[i], name, buf);

#if defined(FEAT_JUMPLIST)
FOR_ALL_WINDOWS(wp)
{
for (i = 0; i < wp->w_jumplistlen; ++i)
fmarks_check_one(&wp->w_jumplist[i], name, buf);
}
#endif

vim_free(name);
}

static void
fmarks_check_one(xfmark_T *fm, char_u *name, buf_T *buf)
{
if (fm->fmark.fnum == 0
&& fm->fname != NULL
&& fnamecmp(name, fm->fname) == 0)
{
fm->fmark.fnum = buf->b_fnum;
VIM_CLEAR(fm->fname);
}
}





int
check_mark(pos_T *pos)
{
if (pos == NULL)
{
emsg(_(e_umark));
return FAIL;
}
if (pos->lnum <= 0)
{


if (pos->lnum == 0)
emsg(_(e_marknotset));
return FAIL;
}
if (pos->lnum > curbuf->b_ml.ml_line_count)
{
emsg(_(e_markinval));
return FAIL;
}
return OK;
}






void
clrallmarks(buf_T *buf)
{
static int i = -1;

if (i == -1) 
for (i = 0; i < NMARKS + 1; i++)
{
namedfm[i].fmark.mark.lnum = 0;
namedfm[i].fname = NULL;
#if defined(FEAT_VIMINFO)
namedfm[i].time_set = 0;
#endif
}

for (i = 0; i < NMARKS; i++)
buf->b_namedm[i].lnum = 0;
buf->b_op_start.lnum = 0; 
buf->b_op_end.lnum = 0;
buf->b_last_cursor.lnum = 1; 
buf->b_last_cursor.col = 0;
buf->b_last_cursor.coladd = 0;
buf->b_last_insert.lnum = 0; 
buf->b_last_change.lnum = 0; 
#if defined(FEAT_JUMPLIST)
buf->b_changelistlen = 0;
#endif
}






char_u *
fm_getname(fmark_T *fmark, int lead_len)
{
if (fmark->fnum == curbuf->b_fnum) 
return mark_line(&(fmark->mark), lead_len);
return buflist_nr2name(fmark->fnum, FALSE, TRUE);
}





static char_u *
mark_line(pos_T *mp, int lead_len)
{
char_u *s, *p;
int len;

if (mp->lnum == 0 || mp->lnum > curbuf->b_ml.ml_line_count)
return vim_strsave((char_u *)"-invalid-");

s = vim_strnsave(skipwhite(ml_get(mp->lnum)), (int)Columns * 5);
if (s == NULL)
return NULL;

len = 0;
for (p = s; *p != NUL; MB_PTR_ADV(p))
{
len += ptr2cells(p);
if (len >= Columns - lead_len)
break;
}
*p = NUL;
return s;
}




void
ex_marks(exarg_T *eap)
{
char_u *arg = eap->arg;
int i;
char_u *name;

if (arg != NULL && *arg == NUL)
arg = NULL;

show_one_mark('\'', arg, &curwin->w_pcmark, NULL, TRUE);
for (i = 0; i < NMARKS; ++i)
show_one_mark(i + 'a', arg, &curbuf->b_namedm[i], NULL, TRUE);
for (i = 0; i < NMARKS + EXTRA_MARKS; ++i)
{
if (namedfm[i].fmark.fnum != 0)
name = fm_getname(&namedfm[i].fmark, 15);
else
name = namedfm[i].fname;
if (name != NULL)
{
show_one_mark(i >= NMARKS ? i - NMARKS + '0' : i + 'A',
arg, &namedfm[i].fmark.mark, name,
namedfm[i].fmark.fnum == curbuf->b_fnum);
if (namedfm[i].fmark.fnum != 0)
vim_free(name);
}
}
show_one_mark('"', arg, &curbuf->b_last_cursor, NULL, TRUE);
show_one_mark('[', arg, &curbuf->b_op_start, NULL, TRUE);
show_one_mark(']', arg, &curbuf->b_op_end, NULL, TRUE);
show_one_mark('^', arg, &curbuf->b_last_insert, NULL, TRUE);
show_one_mark('.', arg, &curbuf->b_last_change, NULL, TRUE);
show_one_mark('<', arg, &curbuf->b_visual.vi_start, NULL, TRUE);
show_one_mark('>', arg, &curbuf->b_visual.vi_end, NULL, TRUE);
show_one_mark(-1, arg, NULL, NULL, FALSE);
}

static void
show_one_mark(
int c,
char_u *arg,
pos_T *p,
char_u *name_arg,
int current) 
{
static int did_title = FALSE;
int mustfree = FALSE;
char_u *name = name_arg;

if (c == -1) 
{
if (did_title)
did_title = FALSE;
else
{
if (arg == NULL)
msg(_("No marks set"));
else
semsg(_("E283: No marks matching \"%s\""), arg);
}
}

else if (!got_int
&& (arg == NULL || vim_strchr(arg, c) != NULL)
&& p->lnum != 0)
{
if (name == NULL && current)
{
name = mark_line(p, 15);
mustfree = TRUE;
}
if (!message_filtered(name))
{
if (!did_title)
{

msg_puts_title(_("\nmark line col file/text"));
did_title = TRUE;
}
msg_putchar('\n');
if (!got_int)
{
sprintf((char *)IObuff, " %c %6ld %4d ", c, p->lnum, p->col);
msg_outtrans(IObuff);
if (name != NULL)
{
msg_outtrans_attr(name, current ? HL_ATTR(HLF_D) : 0);
}
}
out_flush(); 
}
if (mustfree)
vim_free(name);
}
}




void
ex_delmarks(exarg_T *eap)
{
char_u *p;
int from, to;
int i;
int lower;
int digit;
int n;

if (*eap->arg == NUL && eap->forceit)

clrallmarks(curbuf);
else if (eap->forceit)
emsg(_(e_invarg));
else if (*eap->arg == NUL)
emsg(_(e_argreq));
else
{

for (p = eap->arg; *p != NUL; ++p)
{
lower = ASCII_ISLOWER(*p);
digit = VIM_ISDIGIT(*p);
if (lower || digit || ASCII_ISUPPER(*p))
{
if (p[1] == '-')
{

from = *p;
to = p[2];
if (!(lower ? ASCII_ISLOWER(p[2])
: (digit ? VIM_ISDIGIT(p[2])
: ASCII_ISUPPER(p[2])))
|| to < from)
{
semsg(_(e_invarg2), p);
return;
}
p += 2;
}
else

from = to = *p;

for (i = from; i <= to; ++i)
{
if (lower)
curbuf->b_namedm[i - 'a'].lnum = 0;
else
{
if (digit)
n = i - '0' + NMARKS;
else
n = i - 'A';
namedfm[n].fmark.mark.lnum = 0;
namedfm[n].fmark.fnum = 0;
VIM_CLEAR(namedfm[n].fname);
#if defined(FEAT_VIMINFO)
namedfm[n].time_set = digit ? 0 : vim_time();
#endif
}
}
}
else
switch (*p)
{
case '"': curbuf->b_last_cursor.lnum = 0; break;
case '^': curbuf->b_last_insert.lnum = 0; break;
case '.': curbuf->b_last_change.lnum = 0; break;
case '[': curbuf->b_op_start.lnum = 0; break;
case ']': curbuf->b_op_end.lnum = 0; break;
case '<': curbuf->b_visual.vi_start.lnum = 0; break;
case '>': curbuf->b_visual.vi_end.lnum = 0; break;
case ' ': break;
default: semsg(_(e_invarg2), p);
return;
}
}
}
}

#if defined(FEAT_JUMPLIST) || defined(PROTO)



void
ex_jumps(exarg_T *eap UNUSED)
{
int i;
char_u *name;

cleanup_jumplist(curwin, TRUE);


msg_puts_title(_("\n jump line col file/text"));
for (i = 0; i < curwin->w_jumplistlen && !got_int; ++i)
{
if (curwin->w_jumplist[i].fmark.mark.lnum != 0)
{
name = fm_getname(&curwin->w_jumplist[i].fmark, 16);


if (name == NULL || message_filtered(name))
{
vim_free(name);
continue;
}

msg_putchar('\n');
if (got_int)
{
vim_free(name);
break;
}
sprintf((char *)IObuff, "%c %2d %5ld %4d ",
i == curwin->w_jumplistidx ? '>' : ' ',
i > curwin->w_jumplistidx ? i - curwin->w_jumplistidx
: curwin->w_jumplistidx - i,
curwin->w_jumplist[i].fmark.mark.lnum,
curwin->w_jumplist[i].fmark.mark.col);
msg_outtrans(IObuff);
msg_outtrans_attr(name,
curwin->w_jumplist[i].fmark.fnum == curbuf->b_fnum
? HL_ATTR(HLF_D) : 0);
vim_free(name);
ui_breakcheck();
}
out_flush();
}
if (curwin->w_jumplistidx == curwin->w_jumplistlen)
msg_puts("\n>");
}

void
ex_clearjumps(exarg_T *eap UNUSED)
{
free_jumplist(curwin);
curwin->w_jumplistlen = 0;
curwin->w_jumplistidx = 0;
}




void
ex_changes(exarg_T *eap UNUSED)
{
int i;
char_u *name;


msg_puts_title(_("\nchange line col text"));

for (i = 0; i < curbuf->b_changelistlen && !got_int; ++i)
{
if (curbuf->b_changelist[i].lnum != 0)
{
msg_putchar('\n');
if (got_int)
break;
sprintf((char *)IObuff, "%c %3d %5ld %4d ",
i == curwin->w_changelistidx ? '>' : ' ',
i > curwin->w_changelistidx ? i - curwin->w_changelistidx
: curwin->w_changelistidx - i,
(long)curbuf->b_changelist[i].lnum,
curbuf->b_changelist[i].col);
msg_outtrans(IObuff);
name = mark_line(&curbuf->b_changelist[i], 17);
if (name == NULL)
break;
msg_outtrans_attr(name, HL_ATTR(HLF_D));
vim_free(name);
ui_breakcheck();
}
out_flush();
}
if (curwin->w_changelistidx == curbuf->b_changelistlen)
msg_puts("\n>");
}
#endif

#define one_adjust(add) { lp = add; if (*lp >= line1 && *lp <= line2) { if (amount == MAXLNUM) *lp = 0; else *lp += amount; } else if (amount_after && *lp > line2) *lp += amount_after; }














#define one_adjust_nodel(add) { lp = add; if (*lp >= line1 && *lp <= line2) { if (amount == MAXLNUM) *lp = line1; else *lp += amount; } else if (amount_after && *lp > line2) *lp += amount_after; }
























void
mark_adjust(
linenr_T line1,
linenr_T line2,
long amount,
long amount_after)
{
mark_adjust_internal(line1, line2, amount, amount_after, TRUE);
}

void
mark_adjust_nofold(
linenr_T line1,
linenr_T line2,
long amount,
long amount_after)
{
mark_adjust_internal(line1, line2, amount, amount_after, FALSE);
}

static void
mark_adjust_internal(
linenr_T line1,
linenr_T line2,
long amount,
long amount_after,
int adjust_folds UNUSED)
{
int i;
int fnum = curbuf->b_fnum;
linenr_T *lp;
win_T *win;
tabpage_T *tab;
static pos_T initpos = {1, 0, 0};

if (line2 < line1 && amount_after == 0L) 
return;

if (!cmdmod.lockmarks)
{

for (i = 0; i < NMARKS; i++)
{
one_adjust(&(curbuf->b_namedm[i].lnum));
if (namedfm[i].fmark.fnum == fnum)
one_adjust_nodel(&(namedfm[i].fmark.mark.lnum));
}
for (i = NMARKS; i < NMARKS + EXTRA_MARKS; i++)
{
if (namedfm[i].fmark.fnum == fnum)
one_adjust_nodel(&(namedfm[i].fmark.mark.lnum));
}


one_adjust(&(curbuf->b_last_insert.lnum));


one_adjust(&(curbuf->b_last_change.lnum));


if (!EQUAL_POS(curbuf->b_last_cursor, initpos))
one_adjust(&(curbuf->b_last_cursor.lnum));


#if defined(FEAT_JUMPLIST)

for (i = 0; i < curbuf->b_changelistlen; ++i)
one_adjust_nodel(&(curbuf->b_changelist[i].lnum));
#endif


one_adjust_nodel(&(curbuf->b_visual.vi_start.lnum));
one_adjust_nodel(&(curbuf->b_visual.vi_end.lnum));

#if defined(FEAT_QUICKFIX)

qf_mark_adjust(NULL, line1, line2, amount, amount_after);

FOR_ALL_TAB_WINDOWS(tab, win)
qf_mark_adjust(win, line1, line2, amount, amount_after);
#endif

#if defined(FEAT_SIGNS)
sign_mark_adjust(line1, line2, amount, amount_after);
#endif
}


one_adjust(&(curwin->w_pcmark.lnum));


one_adjust(&(curwin->w_prev_pcmark.lnum));


if (saved_cursor.lnum != 0)
one_adjust_nodel(&(saved_cursor.lnum));




FOR_ALL_TAB_WINDOWS(tab, win)
{
#if defined(FEAT_JUMPLIST)
if (!cmdmod.lockmarks)


for (i = 0; i < win->w_jumplistlen; ++i)
if (win->w_jumplist[i].fmark.fnum == fnum)
one_adjust_nodel(&(win->w_jumplist[i].fmark.mark.lnum));
#endif

if (win->w_buffer == curbuf)
{
if (!cmdmod.lockmarks)

for (i = 0; i < win->w_tagstacklen; i++)
if (win->w_tagstack[i].fmark.fnum == fnum)
one_adjust_nodel(&(win->w_tagstack[i].fmark.mark.lnum));


if (win->w_old_cursor_lnum != 0)
{
one_adjust_nodel(&(win->w_old_cursor_lnum));
one_adjust_nodel(&(win->w_old_visual_lnum));
}



if (win != curwin)
{
if (win->w_topline >= line1 && win->w_topline <= line2)
{
if (amount == MAXLNUM) 
{
if (line1 <= 1)
win->w_topline = 1;
else
win->w_topline = line1 - 1;
}
else 
win->w_topline += amount;
#if defined(FEAT_DIFF)
win->w_topfill = 0;
#endif
}
else if (amount_after && win->w_topline > line2)
{
win->w_topline += amount_after;
#if defined(FEAT_DIFF)
win->w_topfill = 0;
#endif
}
if (win->w_cursor.lnum >= line1 && win->w_cursor.lnum <= line2)
{
if (amount == MAXLNUM) 
{
if (line1 <= 1)
win->w_cursor.lnum = 1;
else
win->w_cursor.lnum = line1 - 1;
win->w_cursor.col = 0;
}
else 
win->w_cursor.lnum += amount;
}
else if (amount_after && win->w_cursor.lnum > line2)
win->w_cursor.lnum += amount_after;
}

#if defined(FEAT_FOLDING)

if (adjust_folds)
foldMarkAdjust(win, line1, line2, amount, amount_after);
#endif
}
}

#if defined(FEAT_DIFF)

diff_mark_adjust(line1, line2, amount, amount_after);
#endif
}


#define col_adjust(pp) { posp = pp; if (posp->lnum == lnum && posp->col >= mincol) { posp->lnum += lnum_amount; if (col_amount < 0 && posp->col <= (colnr_T)-col_amount) posp->col = 0; else if (posp->col < spaces_removed) posp->col = col_amount + spaces_removed; else posp->col += col_amount; } }





















void
mark_col_adjust(
linenr_T lnum,
colnr_T mincol,
long lnum_amount,
long col_amount,
int spaces_removed)
{
int i;
int fnum = curbuf->b_fnum;
win_T *win;
pos_T *posp;

if ((col_amount == 0L && lnum_amount == 0L) || cmdmod.lockmarks)
return; 


for (i = 0; i < NMARKS; i++)
{
col_adjust(&(curbuf->b_namedm[i]));
if (namedfm[i].fmark.fnum == fnum)
col_adjust(&(namedfm[i].fmark.mark));
}
for (i = NMARKS; i < NMARKS + EXTRA_MARKS; i++)
{
if (namedfm[i].fmark.fnum == fnum)
col_adjust(&(namedfm[i].fmark.mark));
}


col_adjust(&(curbuf->b_last_insert));


col_adjust(&(curbuf->b_last_change));

#if defined(FEAT_JUMPLIST)

for (i = 0; i < curbuf->b_changelistlen; ++i)
col_adjust(&(curbuf->b_changelist[i]));
#endif


col_adjust(&(curbuf->b_visual.vi_start));
col_adjust(&(curbuf->b_visual.vi_end));


col_adjust(&(curwin->w_pcmark));


col_adjust(&(curwin->w_prev_pcmark));


col_adjust(&saved_cursor);




FOR_ALL_WINDOWS(win)
{
#if defined(FEAT_JUMPLIST)

for (i = 0; i < win->w_jumplistlen; ++i)
if (win->w_jumplist[i].fmark.fnum == fnum)
col_adjust(&(win->w_jumplist[i].fmark.mark));
#endif

if (win->w_buffer == curbuf)
{

for (i = 0; i < win->w_tagstacklen; i++)
if (win->w_tagstack[i].fmark.fnum == fnum)
col_adjust(&(win->w_tagstack[i].fmark.mark));


if (win != curwin)
col_adjust(&win->w_cursor);
}
}
}

#if defined(FEAT_JUMPLIST)






void
cleanup_jumplist(win_T *wp, int loadfiles)
{
int i;
int from, to;

if (loadfiles)
{



for (i = 0; i < wp->w_jumplistlen; ++i)
{
if ((wp->w_jumplist[i].fmark.fnum == 0) &&
(wp->w_jumplist[i].fmark.mark.lnum != 0))
fname2fnum(&wp->w_jumplist[i]);
}
}

to = 0;
for (from = 0; from < wp->w_jumplistlen; ++from)
{
if (wp->w_jumplistidx == from)
wp->w_jumplistidx = to;
for (i = from + 1; i < wp->w_jumplistlen; ++i)
if (wp->w_jumplist[i].fmark.fnum
== wp->w_jumplist[from].fmark.fnum
&& wp->w_jumplist[from].fmark.fnum != 0
&& wp->w_jumplist[i].fmark.mark.lnum
== wp->w_jumplist[from].fmark.mark.lnum)
break;
if (i >= wp->w_jumplistlen) 
wp->w_jumplist[to++] = wp->w_jumplist[from];
else
vim_free(wp->w_jumplist[from].fname);
}
if (wp->w_jumplistidx == wp->w_jumplistlen)
wp->w_jumplistidx = to;
wp->w_jumplistlen = to;
}




void
copy_jumplist(win_T *from, win_T *to)
{
int i;

for (i = 0; i < from->w_jumplistlen; ++i)
{
to->w_jumplist[i] = from->w_jumplist[i];
if (from->w_jumplist[i].fname != NULL)
to->w_jumplist[i].fname = vim_strsave(from->w_jumplist[i].fname);
}
to->w_jumplistlen = from->w_jumplistlen;
to->w_jumplistidx = from->w_jumplistidx;
}




void
free_jumplist(win_T *wp)
{
int i;

for (i = 0; i < wp->w_jumplistlen; ++i)
vim_free(wp->w_jumplist[i].fname);
}
#endif 

void
set_last_cursor(win_T *win)
{
if (win->w_buffer != NULL)
win->w_buffer->b_last_cursor = win->w_cursor;
}

#if defined(EXITFREE) || defined(PROTO)
void
free_all_marks(void)
{
int i;

for (i = 0; i < NMARKS + EXTRA_MARKS; i++)
if (namedfm[i].fmark.mark.lnum != 0)
vim_free(namedfm[i].fname);
}
#endif




xfmark_T *
get_namedfm(void)
{
return namedfm;
}
