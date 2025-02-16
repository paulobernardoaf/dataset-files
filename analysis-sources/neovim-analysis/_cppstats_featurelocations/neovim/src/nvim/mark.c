






#include <assert.h>
#include <inttypes.h>
#include <string.h>
#include <limits.h>

#include "nvim/vim.h"
#include "nvim/ascii.h"
#include "nvim/mark.h"
#include "nvim/buffer.h"
#include "nvim/charset.h"
#include "nvim/diff.h"
#include "nvim/eval.h"
#include "nvim/ex_cmds.h"
#include "nvim/fileio.h"
#include "nvim/fold.h"
#include "nvim/extmark.h"
#include "nvim/mbyte.h"
#include "nvim/memline.h"
#include "nvim/memory.h"
#include "nvim/message.h"
#include "nvim/normal.h"
#include "nvim/option.h"
#include "nvim/path.h"
#include "nvim/quickfix.h"
#include "nvim/search.h"
#include "nvim/sign.h"
#include "nvim/strings.h"
#include "nvim/ui.h"
#include "nvim/os/os.h"
#include "nvim/os/time.h"
#include "nvim/os/input.h"














static xfmark_T namedfm[NGLOBALMARKS];

#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "mark.c.generated.h"
#endif




int setmark(int c)
{
return setmark_pos(c, &curwin->w_cursor, curbuf->b_fnum);
}


void free_fmark(fmark_T fm)
{
tv_dict_unref(fm.additional_data);
}


void free_xfmark(xfmark_T fm)
{
xfree(fm.fname);
free_fmark(fm.fmark);
}


void clear_fmark(fmark_T *fm)
FUNC_ATTR_NONNULL_ALL
{
free_fmark(*fm);
memset(fm, 0, sizeof(*fm));
}






int setmark_pos(int c, pos_T *pos, int fnum)
{
int i;


if (c < 0)
return FAIL;

if (c == '\'' || c == '`') {
if (pos == &curwin->w_cursor) {
setpcmark();

curwin->w_prev_pcmark = curwin->w_pcmark;
} else
curwin->w_pcmark = *pos;
return OK;
}


buf_T *buf = buflist_findnr(fnum);
if (buf == NULL) {
return FAIL;
}

if (c == '"') {
RESET_FMARK(&buf->b_last_cursor, *pos, buf->b_fnum);
return OK;
}



if (c == '[') {
buf->b_op_start = *pos;
return OK;
}
if (c == ']') {
buf->b_op_end = *pos;
return OK;
}

if (c == '<' || c == '>') {
if (c == '<') {
buf->b_visual.vi_start = *pos;
} else {
buf->b_visual.vi_end = *pos;
}
if (buf->b_visual.vi_mode == NUL) {

buf->b_visual.vi_mode = 'v';
}
return OK;
}

if (ASCII_ISLOWER(c)) {
i = c - 'a';
RESET_FMARK(buf->b_namedm + i, *pos, fnum);
return OK;
}
if (ASCII_ISUPPER(c) || ascii_isdigit(c)) {
if (ascii_isdigit(c)) {
i = c - '0' + NMARKS;
} else {
i = c - 'A';
}
RESET_XFMARK(namedfm + i, *pos, fnum, NULL);
return OK;
}
return FAIL;
}





void setpcmark(void)
{
xfmark_T *fm;


if (global_busy || listcmd_busy || cmdmod.keepjumps)
return;

curwin->w_prev_pcmark = curwin->w_pcmark;
curwin->w_pcmark = curwin->w_cursor;

if (curwin->w_pcmark.lnum == 0) {
curwin->w_pcmark.lnum = 1;
}

if (jop_flags & JOP_STACK) {


if (curwin->w_jumplistidx < curwin->w_jumplistlen - 1) {


curwin->w_jumplistlen = curwin->w_jumplistidx + 1;
}
}


if (++curwin->w_jumplistlen > JUMPLISTSIZE) {
curwin->w_jumplistlen = JUMPLISTSIZE;
free_xfmark(curwin->w_jumplist[0]);
memmove(&curwin->w_jumplist[0], &curwin->w_jumplist[1],
(JUMPLISTSIZE - 1) * sizeof(curwin->w_jumplist[0]));
}
curwin->w_jumplistidx = curwin->w_jumplistlen;
fm = &curwin->w_jumplist[curwin->w_jumplistlen - 1];

SET_XFMARK(fm, curwin->w_pcmark, curbuf->b_fnum, NULL);
}







void checkpcmark(void)
{
if (curwin->w_prev_pcmark.lnum != 0
&& (equalpos(curwin->w_pcmark, curwin->w_cursor)
|| curwin->w_pcmark.lnum == 0)) {
curwin->w_pcmark = curwin->w_prev_pcmark;
curwin->w_prev_pcmark.lnum = 0; 
}
}




pos_T *movemark(int count)
{
pos_T *pos;
xfmark_T *jmp;

cleanup_jumplist(curwin, true);

if (curwin->w_jumplistlen == 0) 
return (pos_T *)NULL;

for (;; ) {
if (curwin->w_jumplistidx + count < 0
|| curwin->w_jumplistidx + count >= curwin->w_jumplistlen)
return (pos_T *)NULL;






if (curwin->w_jumplistidx == curwin->w_jumplistlen) {
setpcmark();
--curwin->w_jumplistidx; 
if (curwin->w_jumplistidx + count < 0)
return (pos_T *)NULL;
}

curwin->w_jumplistidx += count;

jmp = curwin->w_jumplist + curwin->w_jumplistidx;
if (jmp->fmark.fnum == 0)
fname2fnum(jmp);
if (jmp->fmark.fnum != curbuf->b_fnum) {

if (buflist_findnr(jmp->fmark.fnum) == NULL) { 
count += count < 0 ? -1 : 1;
continue;
}
if (buflist_getfile(jmp->fmark.fnum, jmp->fmark.mark.lnum,
0, FALSE) == FAIL)
return (pos_T *)NULL;

curwin->w_cursor = jmp->fmark.mark;
pos = (pos_T *)-1;
} else
pos = &(jmp->fmark.mark);
return pos;
}
}




pos_T *movechangelist(int count)
{
int n;

if (curbuf->b_changelistlen == 0) 
return (pos_T *)NULL;

n = curwin->w_changelistidx;
if (n + count < 0) {
if (n == 0)
return (pos_T *)NULL;
n = 0;
} else if (n + count >= curbuf->b_changelistlen) {
if (n == curbuf->b_changelistlen - 1)
return (pos_T *)NULL;
n = curbuf->b_changelistlen - 1;
} else
n += count;
curwin->w_changelistidx = n;
return &(curbuf->b_changelist[n].mark);
}












pos_T *getmark_buf(buf_T *buf, int c, bool changefile)
{
return getmark_buf_fnum(buf, c, changefile, NULL);
}

pos_T *getmark(int c, bool changefile)
{
return getmark_buf_fnum(curbuf, c, changefile, NULL);
}

pos_T *getmark_buf_fnum(buf_T *buf, int c, bool changefile, int *fnum)
{
pos_T *posp;
pos_T *startp, *endp;
static pos_T pos_copy;

posp = NULL;



if (c < 0)
return posp;
if (c > '~') { 
} else if (c == '\'' || c == '`') { 
pos_copy = curwin->w_pcmark; 
posp = &pos_copy; 
} else if (c == '"') { 
posp = &(buf->b_last_cursor.mark);
} else if (c == '^') { 
posp = &(buf->b_last_insert.mark);
} else if (c == '.') { 
posp = &(buf->b_last_change.mark);
} else if (c == '[') { 
posp = &(buf->b_op_start);
} else if (c == ']') { 
posp = &(buf->b_op_end);
} else if (c == '{' || c == '}') { 
pos_T pos;
oparg_T oa;
int slcb = listcmd_busy;

pos = curwin->w_cursor;
listcmd_busy = TRUE; 
if (findpar(&oa.inclusive,
c == '}' ? FORWARD : BACKWARD, 1L, NUL, FALSE)) {
pos_copy = curwin->w_cursor;
posp = &pos_copy;
}
curwin->w_cursor = pos;
listcmd_busy = slcb;
} else if (c == '(' || c == ')') { 
pos_T pos;
int slcb = listcmd_busy;

pos = curwin->w_cursor;
listcmd_busy = TRUE; 
if (findsent(c == ')' ? FORWARD : BACKWARD, 1L)) {
pos_copy = curwin->w_cursor;
posp = &pos_copy;
}
curwin->w_cursor = pos;
listcmd_busy = slcb;
} else if (c == '<' || c == '>') { 
startp = &buf->b_visual.vi_start;
endp = &buf->b_visual.vi_end;
if (((c == '<') == lt(*startp, *endp) || endp->lnum == 0)
&& startp->lnum != 0) {
posp = startp;
} else {
posp = endp;
}


if (buf->b_visual.vi_mode == 'V') {
pos_copy = *posp;
posp = &pos_copy;
if (c == '<')
pos_copy.col = 0;
else
pos_copy.col = MAXCOL;
pos_copy.coladd = 0;
}
} else if (ASCII_ISLOWER(c)) { 
posp = &(buf->b_namedm[c - 'a'].mark);
} else if (ASCII_ISUPPER(c) || ascii_isdigit(c)) { 
if (ascii_isdigit(c))
c = c - '0' + NMARKS;
else
c -= 'A';
posp = &(namedfm[c].fmark.mark);

if (namedfm[c].fmark.fnum == 0) {
fname2fnum(&namedfm[c]);
}

if (fnum != NULL)
*fnum = namedfm[c].fmark.fnum;
else if (namedfm[c].fmark.fnum != buf->b_fnum) {

posp = &pos_copy;

if (namedfm[c].fmark.mark.lnum != 0
&& changefile && namedfm[c].fmark.fnum) {
if (buflist_getfile(namedfm[c].fmark.fnum,
(linenr_T)1, GETF_SETMARK, FALSE) == OK) {

curwin->w_cursor = namedfm[c].fmark.mark;
return (pos_T *)-1;
}
pos_copy.lnum = -1; 
} else
pos_copy.lnum = 0; 

}
}

return posp;
}






pos_T *
getnextmark (
pos_T *startpos, 
int dir, 
int begin_line
)
{
int i;
pos_T *result = NULL;
pos_T pos;

pos = *startpos;





if (dir == BACKWARD && begin_line)
pos.col = 0;
else if (dir == FORWARD && begin_line)
pos.col = MAXCOL;

for (i = 0; i < NMARKS; i++) {
if (curbuf->b_namedm[i].mark.lnum > 0) {
if (dir == FORWARD) {
if ((result == NULL || lt(curbuf->b_namedm[i].mark, *result))
&& lt(pos, curbuf->b_namedm[i].mark))
result = &curbuf->b_namedm[i].mark;
} else {
if ((result == NULL || lt(*result, curbuf->b_namedm[i].mark))
&& lt(curbuf->b_namedm[i].mark, pos))
result = &curbuf->b_namedm[i].mark;
}
}
}

return result;
}






static void fname2fnum(xfmark_T *fm)
{
char_u *p;

if (fm->fname != NULL) {




if (fm->fname[0] == '~' && (fm->fname[1] == '/'
#if defined(BACKSLASH_IN_FILENAME)
|| fm->fname[1] == '\\'
#endif
)) {
int len;

expand_env((char_u *)"~/", NameBuff, MAXPATHL);
len = (int)STRLEN(NameBuff);
STRLCPY(NameBuff + len, fm->fname + 2, MAXPATHL - len);
} else
STRLCPY(NameBuff, fm->fname, MAXPATHL);


os_dirname(IObuff, IOSIZE);
p = path_shorten_fname(NameBuff, IObuff);


(void)buflist_new(NameBuff, p, (linenr_T)1, 0);
}
}






void fmarks_check_names(buf_T *buf)
{
char_u *name = buf->b_ffname;
int i;

if (buf->b_ffname == NULL)
return;

for (i = 0; i < NGLOBALMARKS; ++i)
fmarks_check_one(&namedfm[i], name, buf);

FOR_ALL_WINDOWS_IN_TAB(wp, curtab) {
for (i = 0; i < wp->w_jumplistlen; ++i) {
fmarks_check_one(&wp->w_jumplist[i], name, buf);
}
}
}

static void fmarks_check_one(xfmark_T *fm, char_u *name, buf_T *buf)
{
if (fm->fmark.fnum == 0
&& fm->fname != NULL
&& fnamecmp(name, fm->fname) == 0) {
fm->fmark.fnum = buf->b_fnum;
XFREE_CLEAR(fm->fname);
}
}





int check_mark(pos_T *pos)
{
if (pos == NULL) {
EMSG(_(e_umark));
return FAIL;
}
if (pos->lnum <= 0) {


if (pos->lnum == 0)
EMSG(_(e_marknotset));
return FAIL;
}
if (pos->lnum > curbuf->b_ml.ml_line_count) {
EMSG(_(e_markinval));
return FAIL;
}
return OK;
}






void clrallmarks(buf_T *const buf)
FUNC_ATTR_NONNULL_ALL
{
for (size_t i = 0; i < NMARKS; i++) {
clear_fmark(&buf->b_namedm[i]);
}
clear_fmark(&buf->b_last_cursor);
buf->b_last_cursor.mark.lnum = 1;
clear_fmark(&buf->b_last_insert);
clear_fmark(&buf->b_last_change);
buf->b_op_start.lnum = 0; 
buf->b_op_end.lnum = 0;
for (int i = 0; i < buf->b_changelistlen; i++) {
clear_fmark(&buf->b_changelist[i]);
}
buf->b_changelistlen = 0;
}






char_u *fm_getname(fmark_T *fmark, int lead_len)
{
if (fmark->fnum == curbuf->b_fnum) 
return mark_line(&(fmark->mark), lead_len);
return buflist_nr2name(fmark->fnum, FALSE, TRUE);
}





static char_u *mark_line(pos_T *mp, int lead_len)
{
char_u *s, *p;
int len;

if (mp->lnum == 0 || mp->lnum > curbuf->b_ml.ml_line_count)
return vim_strsave((char_u *)"-invalid-");
assert(Columns >= 0 && (size_t)Columns <= SIZE_MAX);

s = vim_strnsave(skipwhite(ml_get(mp->lnum)), (size_t)Columns * 5);


len = 0;
for (p = s; *p != NUL; MB_PTR_ADV(p)) {
len += ptr2cells(p);
if (len >= Columns - lead_len)
break;
}
*p = NUL;
return s;
}




void ex_marks(exarg_T *eap)
{
char_u *arg = eap->arg;
int i;
char_u *name;

if (arg != NULL && *arg == NUL)
arg = NULL;

show_one_mark('\'', arg, &curwin->w_pcmark, NULL, true);
for (i = 0; i < NMARKS; ++i)
show_one_mark(i + 'a', arg, &curbuf->b_namedm[i].mark, NULL, true);
for (i = 0; i < NGLOBALMARKS; ++i) {
if (namedfm[i].fmark.fnum != 0)
name = fm_getname(&namedfm[i].fmark, 15);
else
name = namedfm[i].fname;
if (name != NULL) {
show_one_mark(i >= NMARKS ? i - NMARKS + '0' : i + 'A',
arg, &namedfm[i].fmark.mark, name,
namedfm[i].fmark.fnum == curbuf->b_fnum);
if (namedfm[i].fmark.fnum != 0)
xfree(name);
}
}
show_one_mark('"', arg, &curbuf->b_last_cursor.mark, NULL, true);
show_one_mark('[', arg, &curbuf->b_op_start, NULL, true);
show_one_mark(']', arg, &curbuf->b_op_end, NULL, true);
show_one_mark('^', arg, &curbuf->b_last_insert.mark, NULL, true);
show_one_mark('.', arg, &curbuf->b_last_change.mark, NULL, true);
show_one_mark('<', arg, &curbuf->b_visual.vi_start, NULL, true);
show_one_mark('>', arg, &curbuf->b_visual.vi_end, NULL, true);
show_one_mark(-1, arg, NULL, NULL, false);
}

static void
show_one_mark(
int c,
char_u *arg,
pos_T *p,
char_u *name_arg,
int current 
)
{
static bool did_title = false;
bool mustfree = false;
char_u *name = name_arg;

if (c == -1) { 
if (did_title) {
did_title = false;
} else {
if (arg == NULL) {
MSG(_("No marks set"));
} else {
EMSG2(_("E283: No marks matching \"%s\""), arg);
}
}
} else if (!got_int
&& (arg == NULL || vim_strchr(arg, c) != NULL)
&& p->lnum != 0) {

if (name == NULL && current) {
name = mark_line(p, 15);
mustfree = true;
}
if (!message_filtered(name)) {
if (!did_title) {

msg_puts_title(_("\nmark line col file/text"));
did_title = true;
}
msg_putchar('\n');
if (!got_int) {
snprintf((char *)IObuff, IOSIZE, " %c %6ld %4d ", c, p->lnum, p->col);
msg_outtrans(IObuff);
if (name != NULL) {
msg_outtrans_attr(name, current ? HL_ATTR(HLF_D) : 0);
}
}
ui_flush(); 
}
if (mustfree) {
xfree(name);
}
}
}




void ex_delmarks(exarg_T *eap)
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
EMSG(_(e_invarg));
else if (*eap->arg == NUL)
EMSG(_(e_argreq));
else {

for (p = eap->arg; *p != NUL; ++p) {
lower = ASCII_ISLOWER(*p);
digit = ascii_isdigit(*p);
if (lower || digit || ASCII_ISUPPER(*p)) {
if (p[1] == '-') {

from = *p;
to = p[2];
if (!(lower ? ASCII_ISLOWER(p[2])
: (digit ? ascii_isdigit(p[2])
: ASCII_ISUPPER(p[2])))
|| to < from) {
EMSG2(_(e_invarg2), p);
return;
}
p += 2;
} else

from = to = *p;

for (i = from; i <= to; ++i) {
if (lower) {
curbuf->b_namedm[i - 'a'].mark.lnum = 0;
} else {
if (digit) {
n = i - '0' + NMARKS;
} else {
n = i - 'A';
}
namedfm[n].fmark.mark.lnum = 0;
XFREE_CLEAR(namedfm[n].fname);
}
}
} else
switch (*p) {
case '"': CLEAR_FMARK(&curbuf->b_last_cursor); break;
case '^': CLEAR_FMARK(&curbuf->b_last_insert); break;
case '.': CLEAR_FMARK(&curbuf->b_last_change); break;
case '[': curbuf->b_op_start.lnum = 0; break;
case ']': curbuf->b_op_end.lnum = 0; break;
case '<': curbuf->b_visual.vi_start.lnum = 0; break;
case '>': curbuf->b_visual.vi_end.lnum = 0; break;
case ' ': break;
default: EMSG2(_(e_invarg2), p);
return;
}
}
}
}




void ex_jumps(exarg_T *eap)
{
int i;
char_u *name;

cleanup_jumplist(curwin, true);

MSG_PUTS_TITLE(_("\n jump line col file/text"));
for (i = 0; i < curwin->w_jumplistlen && !got_int; ++i) {
if (curwin->w_jumplist[i].fmark.mark.lnum != 0) {
name = fm_getname(&curwin->w_jumplist[i].fmark, 16);


if (name == NULL || message_filtered(name)) {
xfree(name);
continue;
}

msg_putchar('\n');
if (got_int) {
xfree(name);
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
xfree(name);
os_breakcheck();
}
ui_flush();
}
if (curwin->w_jumplistidx == curwin->w_jumplistlen)
MSG_PUTS("\n>");
}

void ex_clearjumps(exarg_T *eap)
{
free_jumplist(curwin);
curwin->w_jumplistlen = 0;
curwin->w_jumplistidx = 0;
}




void ex_changes(exarg_T *eap)
{
int i;
char_u *name;


MSG_PUTS_TITLE(_("\nchange line col text"));

for (i = 0; i < curbuf->b_changelistlen && !got_int; ++i) {
if (curbuf->b_changelist[i].mark.lnum != 0) {
msg_putchar('\n');
if (got_int)
break;
sprintf((char *)IObuff, "%c %3d %5ld %4d ",
i == curwin->w_changelistidx ? '>' : ' ',
i > curwin->w_changelistidx ? i - curwin->w_changelistidx
: curwin->w_changelistidx - i,
(long)curbuf->b_changelist[i].mark.lnum,
curbuf->b_changelist[i].mark.col);
msg_outtrans(IObuff);
name = mark_line(&curbuf->b_changelist[i].mark, 17);
msg_outtrans_attr(name, HL_ATTR(HLF_D));
xfree(name);
os_breakcheck();
}
ui_flush();
}
if (curwin->w_changelistidx == curbuf->b_changelistlen)
MSG_PUTS("\n>");
}

#define one_adjust(add) { lp = add; if (*lp >= line1 && *lp <= line2) { if (amount == MAXLNUM) *lp = 0; else *lp += amount; } else if (amount_after && *lp > line2) *lp += amount_after; }














#define one_adjust_nodel(add) { lp = add; if (*lp >= line1 && *lp <= line2) { if (amount == MAXLNUM) *lp = line1; else *lp += amount; } else if (amount_after && *lp > line2) *lp += amount_after; }
























void mark_adjust(linenr_T line1,
linenr_T line2,
long amount,
long amount_after,
ExtmarkOp op)
{
mark_adjust_internal(line1, line2, amount, amount_after, true, op);
}






void mark_adjust_nofold(linenr_T line1, linenr_T line2, long amount,
long amount_after,
ExtmarkOp op)
{
mark_adjust_internal(line1, line2, amount, amount_after, false, op);
}

static void mark_adjust_internal(linenr_T line1, linenr_T line2,
long amount, long amount_after,
bool adjust_folds,
ExtmarkOp op)
{
int i;
int fnum = curbuf->b_fnum;
linenr_T *lp;
static pos_T initpos = { 1, 0, 0 };

if (line2 < line1 && amount_after == 0L) 
return;

if (!cmdmod.lockmarks) {

for (i = 0; i < NMARKS; i++) {
one_adjust(&(curbuf->b_namedm[i].mark.lnum));
if (namedfm[i].fmark.fnum == fnum)
one_adjust_nodel(&(namedfm[i].fmark.mark.lnum));
}
for (i = NMARKS; i < NGLOBALMARKS; i++) {
if (namedfm[i].fmark.fnum == fnum)
one_adjust_nodel(&(namedfm[i].fmark.mark.lnum));
}


one_adjust(&(curbuf->b_last_insert.mark.lnum));


one_adjust(&(curbuf->b_last_change.mark.lnum));


if (!equalpos(curbuf->b_last_cursor.mark, initpos))
one_adjust(&(curbuf->b_last_cursor.mark.lnum));



for (i = 0; i < curbuf->b_changelistlen; ++i)
one_adjust_nodel(&(curbuf->b_changelist[i].mark.lnum));


one_adjust_nodel(&(curbuf->b_visual.vi_start.lnum));
one_adjust_nodel(&(curbuf->b_visual.vi_end.lnum));


if (!qf_mark_adjust(NULL, line1, line2, amount, amount_after)) {
curbuf->b_has_qf_entry &= ~BUF_HAS_QF_ENTRY;
}

bool found_one = false;
FOR_ALL_TAB_WINDOWS(tab, win) {
found_one |= qf_mark_adjust(win, line1, line2, amount, amount_after);
}
if (!found_one) {
curbuf->b_has_qf_entry &= ~BUF_HAS_LL_ENTRY;
}

sign_mark_adjust(line1, line2, amount, amount_after);
if (op != kExtmarkNOOP) {
extmark_adjust(curbuf, line1, line2, amount, amount_after, op);
}
}


one_adjust(&(curwin->w_pcmark.lnum));


one_adjust(&(curwin->w_prev_pcmark.lnum));


if (saved_cursor.lnum != 0)
one_adjust_nodel(&(saved_cursor.lnum));




FOR_ALL_TAB_WINDOWS(tab, win) {
if (!cmdmod.lockmarks) {


for (i = 0; i < win->w_jumplistlen; ++i) {
if (win->w_jumplist[i].fmark.fnum == fnum) {
one_adjust_nodel(&(win->w_jumplist[i].fmark.mark.lnum));
}
}
}

if (win->w_buffer == curbuf) {
if (!cmdmod.lockmarks) {

for (i = 0; i < win->w_tagstacklen; i++) {
if (win->w_tagstack[i].fmark.fnum == fnum) {
one_adjust_nodel(&(win->w_tagstack[i].fmark.mark.lnum));
}
}
}


if (win->w_old_cursor_lnum != 0) {
one_adjust_nodel(&(win->w_old_cursor_lnum));
one_adjust_nodel(&(win->w_old_visual_lnum));
}



if (win != curwin) {
if (win->w_topline >= line1 && win->w_topline <= line2) {
if (amount == MAXLNUM) { 
if (line1 <= 1) {
win->w_topline = 1;
} else {
win->w_topline = line1 - 1;
}
} else { 
win->w_topline += amount;
}
win->w_topfill = 0;
} else if (amount_after && win->w_topline > line2) {
win->w_topline += amount_after;
win->w_topfill = 0;
}
if (win->w_cursor.lnum >= line1 && win->w_cursor.lnum <= line2) {
if (amount == MAXLNUM) { 
if (line1 <= 1) {
win->w_cursor.lnum = 1;
} else {
win->w_cursor.lnum = line1 - 1;
}
win->w_cursor.col = 0;
} else { 
win->w_cursor.lnum += amount;
}
} else if (amount_after && win->w_cursor.lnum > line2) {
win->w_cursor.lnum += amount_after;
}
}

if (adjust_folds) {
foldMarkAdjust(win, line1, line2, amount, amount_after);
}
}
}


diff_mark_adjust(line1, line2, amount, amount_after);
}


#define col_adjust(pp) { posp = pp; if (posp->lnum == lnum && posp->col >= mincol) { posp->lnum += lnum_amount; assert(col_amount > INT_MIN && col_amount <= INT_MAX); if (col_amount < 0 && posp->col <= (colnr_T)-col_amount) { posp->col = 0; } else if (posp->col < spaces_removed) { posp->col = (int)col_amount + spaces_removed; } else { posp->col += (colnr_T)col_amount; } } }





















void mark_col_adjust(
linenr_T lnum, colnr_T mincol, long lnum_amount, long col_amount,
int spaces_removed)
{
int i;
int fnum = curbuf->b_fnum;
pos_T *posp;

if ((col_amount == 0L && lnum_amount == 0L) || cmdmod.lockmarks)
return; 


for (i = 0; i < NMARKS; i++) {
col_adjust(&(curbuf->b_namedm[i].mark));
if (namedfm[i].fmark.fnum == fnum)
col_adjust(&(namedfm[i].fmark.mark));
}
for (i = NMARKS; i < NGLOBALMARKS; i++) {
if (namedfm[i].fmark.fnum == fnum)
col_adjust(&(namedfm[i].fmark.mark));
}


col_adjust(&(curbuf->b_last_insert.mark));


col_adjust(&(curbuf->b_last_change.mark));


for (i = 0; i < curbuf->b_changelistlen; ++i)
col_adjust(&(curbuf->b_changelist[i].mark));


col_adjust(&(curbuf->b_visual.vi_start));
col_adjust(&(curbuf->b_visual.vi_end));


col_adjust(&(curwin->w_pcmark));


col_adjust(&(curwin->w_prev_pcmark));


col_adjust(&saved_cursor);




FOR_ALL_WINDOWS_IN_TAB(win, curtab) {

for (i = 0; i < win->w_jumplistlen; ++i) {
if (win->w_jumplist[i].fmark.fnum == fnum) {
col_adjust(&(win->w_jumplist[i].fmark.mark));
}
}

if (win->w_buffer == curbuf) {

for (i = 0; i < win->w_tagstacklen; i++) {
if (win->w_tagstack[i].fmark.fnum == fnum) {
col_adjust(&(win->w_tagstack[i].fmark.mark));
}
}


if (win != curwin) {
col_adjust(&win->w_cursor);
}
}
}
}




void cleanup_jumplist(win_T *wp, bool checktail)
{
int i;




for (i = 0; i < wp->w_jumplistlen; i++) {
if ((wp->w_jumplist[i].fmark.fnum == 0)
&& (wp->w_jumplist[i].fmark.mark.lnum != 0)) {
fname2fnum(&wp->w_jumplist[i]);
}
}

int to = 0;
for (int from = 0; from < wp->w_jumplistlen; from++) {
if (wp->w_jumplistidx == from) {
wp->w_jumplistidx = to;
}
for (i = from + 1; i < wp->w_jumplistlen; i++) {
if (wp->w_jumplist[i].fmark.fnum
== wp->w_jumplist[from].fmark.fnum
&& wp->w_jumplist[from].fmark.fnum != 0
&& wp->w_jumplist[i].fmark.mark.lnum
== wp->w_jumplist[from].fmark.mark.lnum) {
break;
}
}

bool mustfree;
if (i >= wp->w_jumplistlen) { 
mustfree = false;
} else if (i > from + 1) { 

mustfree = !(jop_flags & JOP_STACK);
} else { 
mustfree = true;
}

if (mustfree) {
xfree(wp->w_jumplist[from].fname);
} else {
if (to != from) {



wp->w_jumplist[to] = wp->w_jumplist[from];
}
to++;
}
}
if (wp->w_jumplistidx == wp->w_jumplistlen) {
wp->w_jumplistidx = to;
}
wp->w_jumplistlen = to;



if (checktail && wp->w_jumplistlen
&& wp->w_jumplistidx == wp->w_jumplistlen) {
const xfmark_T *fm_last = &wp->w_jumplist[wp->w_jumplistlen - 1];
if (fm_last->fmark.fnum == curbuf->b_fnum
&& fm_last->fmark.mark.lnum == wp->w_cursor.lnum) {
xfree(fm_last->fname);
wp->w_jumplistlen--;
wp->w_jumplistidx--;
}
}
}




void copy_jumplist(win_T *from, win_T *to)
{
int i;

for (i = 0; i < from->w_jumplistlen; ++i) {
to->w_jumplist[i] = from->w_jumplist[i];
if (from->w_jumplist[i].fname != NULL)
to->w_jumplist[i].fname = vim_strsave(from->w_jumplist[i].fname);
}
to->w_jumplistlen = from->w_jumplistlen;
to->w_jumplistidx = from->w_jumplistidx;
}












const void *mark_jumplist_iter(const void *const iter, const win_T *const win,
xfmark_T *const fm)
FUNC_ATTR_NONNULL_ARG(2, 3) FUNC_ATTR_WARN_UNUSED_RESULT
{
if (iter == NULL && win->w_jumplistlen == 0) {
*fm = (xfmark_T) {{{0, 0, 0}, 0, 0, NULL}, NULL};
return NULL;
}
const xfmark_T *const iter_mark =
(iter == NULL
? &(win->w_jumplist[0])
: (const xfmark_T *const) iter);
*fm = *iter_mark;
if (iter_mark == &(win->w_jumplist[win->w_jumplistlen - 1])) {
return NULL;
} else {
return iter_mark + 1;
}
}












const void *mark_global_iter(const void *const iter, char *const name,
xfmark_T *const fm)
FUNC_ATTR_NONNULL_ARG(2, 3) FUNC_ATTR_WARN_UNUSED_RESULT
{
*name = NUL;
const xfmark_T *iter_mark = (iter == NULL
? &(namedfm[0])
: (const xfmark_T *const) iter);
while ((size_t) (iter_mark - &(namedfm[0])) < ARRAY_SIZE(namedfm)
&& !iter_mark->fmark.mark.lnum) {
iter_mark++;
}
if ((size_t) (iter_mark - &(namedfm[0])) == ARRAY_SIZE(namedfm)
|| !iter_mark->fmark.mark.lnum) {
return NULL;
}
size_t iter_off = (size_t) (iter_mark - &(namedfm[0]));
*name = (char) (iter_off < NMARKS
? 'A' + (char) iter_off
: '0' + (char) (iter_off - NMARKS));
*fm = *iter_mark;
while ((size_t) (++iter_mark - &(namedfm[0])) < ARRAY_SIZE(namedfm)) {
if (iter_mark->fmark.mark.lnum) {
return (const void *) iter_mark;
}
}
return NULL;
}












static inline const fmark_T *next_buffer_mark(const buf_T *const buf,
char *const mark_name)
FUNC_ATTR_NONNULL_ALL FUNC_ATTR_WARN_UNUSED_RESULT
{
switch (*mark_name) {
case NUL: {
*mark_name = '"';
return &(buf->b_last_cursor);
}
case '"': {
*mark_name = '^';
return &(buf->b_last_insert);
}
case '^': {
*mark_name = '.';
return &(buf->b_last_change);
}
case '.': {
*mark_name = 'a';
return &(buf->b_namedm[0]);
}
case 'z': {
return NULL;
}
default: {
(*mark_name)++;
return &(buf->b_namedm[*mark_name - 'a']);
}
}
}













const void *mark_buffer_iter(const void *const iter, const buf_T *const buf,
char *const name, fmark_T *const fm)
FUNC_ATTR_NONNULL_ARG(2, 3, 4) FUNC_ATTR_WARN_UNUSED_RESULT
{
*name = NUL;
char mark_name = (char) (iter == NULL
? NUL
: (iter == &(buf->b_last_cursor)
? '"'
: (iter == &(buf->b_last_insert)
? '^'
: (iter == &(buf->b_last_change)
? '.'
: 'a' + (char) ((const fmark_T *)iter
- &(buf->b_namedm[0]))))));
const fmark_T *iter_mark = next_buffer_mark(buf, &mark_name);
while (iter_mark != NULL && iter_mark->mark.lnum == 0) {
iter_mark = next_buffer_mark(buf, &mark_name);
}
if (iter_mark == NULL) {
return NULL;
}
size_t iter_off = (size_t) (iter_mark - &(buf->b_namedm[0]));
if (mark_name) {
*name = mark_name;
} else {
*name = (char) ('a' + (char) iter_off);
}
*fm = *iter_mark;
return (const void *) iter_mark;
}









bool mark_set_global(const char name, const xfmark_T fm, const bool update)
{
const int idx = mark_global_index(name);
if (idx == -1) {
return false;
}
xfmark_T *const fm_tgt = &(namedfm[idx]);
if (update && fm.fmark.timestamp <= fm_tgt->fmark.timestamp) {
return false;
}
if (fm_tgt->fmark.mark.lnum != 0) {
free_xfmark(*fm_tgt);
}
*fm_tgt = fm;
return true;
}










bool mark_set_local(const char name, buf_T *const buf,
const fmark_T fm, const bool update)
FUNC_ATTR_NONNULL_ALL
{
fmark_T *fm_tgt = NULL;
if (ASCII_ISLOWER(name)) {
fm_tgt = &(buf->b_namedm[name - 'a']);
} else if (name == '"') {
fm_tgt = &(buf->b_last_cursor);
} else if (name == '^') {
fm_tgt = &(buf->b_last_insert);
} else if (name == '.') {
fm_tgt = &(buf->b_last_change);
} else {
return false;
}
if (update && fm.timestamp <= fm_tgt->timestamp) {
return false;
}
if (fm_tgt->mark.lnum != 0) {
free_fmark(*fm_tgt);
}
*fm_tgt = fm;
return true;
}




void free_jumplist(win_T *wp)
{
int i;

for (i = 0; i < wp->w_jumplistlen; ++i) {
free_xfmark(wp->w_jumplist[i]);
}
wp->w_jumplistlen = 0;
}

void set_last_cursor(win_T *win)
{
if (win->w_buffer != NULL) {
RESET_FMARK(&win->w_buffer->b_last_cursor, win->w_cursor, 0);
}
}

#if defined(EXITFREE)
void free_all_marks(void)
{
int i;

for (i = 0; i < NGLOBALMARKS; i++) {
if (namedfm[i].fmark.mark.lnum != 0) {
free_xfmark(namedfm[i]);
}
}
memset(&namedfm[0], 0, sizeof(namedfm));
}
#endif







void mark_mb_adjustpos(buf_T *buf, pos_T *lp)
FUNC_ATTR_NONNULL_ALL
{
if (lp->col > 0 || lp->coladd > 1) {
const char_u *const p = ml_get_buf(buf, lp->lnum, false);
if (*p == NUL || (int)STRLEN(p) < lp->col) {
lp->col = 0;
} else {
lp->col -= utf_head_off(p, p + lp->col);
}


if (lp->coladd == 1
&& p[lp->col] != TAB
&& vim_isprintc(utf_ptr2char(p + lp->col))
&& ptr2cells(p + lp->col) > 1) {
lp->coladd = 0;
}
}
}
