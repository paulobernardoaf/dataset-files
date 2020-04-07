






#include <assert.h>
#include <float.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <math.h>

#include "nvim/api/private/defs.h"
#include "nvim/api/vim.h"
#include "nvim/api/buffer.h"
#include "nvim/log.h"
#include "nvim/vim.h"
#include "nvim/ascii.h"
#include "nvim/ex_cmds.h"
#include "nvim/buffer.h"
#include "nvim/change.h"
#include "nvim/charset.h"
#include "nvim/cursor.h"
#include "nvim/diff.h"
#include "nvim/digraph.h"
#include "nvim/edit.h"
#include "nvim/eval.h"
#include "nvim/ex_cmds2.h"
#include "nvim/ex_docmd.h"
#include "nvim/ex_eval.h"
#include "nvim/ex_getln.h"
#include "nvim/fileio.h"
#include "nvim/fold.h"
#include "nvim/getchar.h"
#include "nvim/highlight.h"
#include "nvim/indent.h"
#include "nvim/buffer_updates.h"
#include "nvim/main.h"
#include "nvim/mark.h"
#include "nvim/extmark.h"
#include "nvim/mbyte.h"
#include "nvim/memline.h"
#include "nvim/message.h"
#include "nvim/misc1.h"
#include "nvim/garray.h"
#include "nvim/memory.h"
#include "nvim/move.h"
#include "nvim/mouse.h"
#include "nvim/normal.h"
#include "nvim/ops.h"
#include "nvim/option.h"
#include "nvim/os_unix.h"
#include "nvim/path.h"
#include "nvim/quickfix.h"
#include "nvim/regexp.h"
#include "nvim/screen.h"
#include "nvim/search.h"
#include "nvim/spell.h"
#include "nvim/strings.h"
#include "nvim/syntax.h"
#include "nvim/tag.h"
#include "nvim/ui.h"
#include "nvim/undo.h"
#include "nvim/window.h"
#include "nvim/os/os.h"
#include "nvim/os/shell.h"
#include "nvim/os/input.h"
#include "nvim/os/time.h"



typedef enum {
kSubHonorOptions = 0, 
kSubIgnoreCase, 
kSubMatchCase, 
} SubIgnoreType;


typedef struct {
bool do_all; 
bool do_ask; 
bool do_count; 
bool do_error; 
bool do_print; 
bool do_list; 
bool do_number; 
SubIgnoreType do_ic; 
} subflags_T;



typedef struct {
lpos_T start; 
lpos_T end; 
linenr_T pre_match; 
} SubResult;



typedef struct {
kvec_t(SubResult) subresults;
linenr_T lines_needed; 
} PreviewLines;

#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "ex_cmds.c.generated.h"
#endif


void do_ascii(const exarg_T *const eap)
{
char_u *dig;
int cc[MAX_MCO];
int c = utfc_ptr2char(get_cursor_pos_ptr(), cc);
if (c == NUL) {
MSG("NUL");
return;
}

size_t iobuff_len = 0;

int ci = 0;
if (c < 0x80) {
if (c == NL) { 
c = NUL;
}
const int cval = (c == CAR && get_fileformat(curbuf) == EOL_MAC
? NL 
: c);
char buf1[20];
if (vim_isprintc_strict(c) && (c < ' ' || c > '~')) {
char_u buf3[7];
transchar_nonprint(buf3, c);
vim_snprintf(buf1, sizeof(buf1), " <%s>", (char *)buf3);
} else {
buf1[0] = NUL;
}
char buf2[20];
buf2[0] = NUL;

dig = get_digraph_for_char(cval);
if (dig != NULL) {
iobuff_len += (
vim_snprintf((char *)IObuff + iobuff_len,
sizeof(IObuff) - iobuff_len,
_("<%s>%s%s %d, Hex %02x, Oct %03o, Digr %s"),
transchar(c), buf1, buf2, cval, cval, cval, dig));
} else {
iobuff_len += (
vim_snprintf((char *)IObuff + iobuff_len,
sizeof(IObuff) - iobuff_len,
_("<%s>%s%s %d, Hex %02x, Octal %03o"),
transchar(c), buf1, buf2, cval, cval, cval));
}

c = cc[ci++];
}

#define SPACE_FOR_DESC (1 + 1 + 1 + MB_MAXBYTES + 16 + 4 + 3 + 3 + 1)




















while (c >= 0x80 && iobuff_len < sizeof(IObuff) - SPACE_FOR_DESC) {

if (iobuff_len > 0) {
IObuff[iobuff_len++] = ' ';
}
IObuff[iobuff_len++] = '<';
if (utf_iscomposing(c)) {
IObuff[iobuff_len++] = ' '; 
}
iobuff_len += utf_char2bytes(c, IObuff + iobuff_len);

dig = get_digraph_for_char(c);
if (dig != NULL) {
iobuff_len += (
vim_snprintf((char *)IObuff + iobuff_len,
sizeof(IObuff) - iobuff_len,
(c < 0x10000
? _("> %d, Hex %04x, Oct %o, Digr %s")
: _("> %d, Hex %08x, Oct %o, Digr %s")),
c, c, c, dig));
} else {
iobuff_len += (
vim_snprintf((char *)IObuff + iobuff_len,
sizeof(IObuff) - iobuff_len,
(c < 0x10000
? _("> %d, Hex %04x, Octal %o")
: _("> %d, Hex %08x, Octal %o")),
c, c, c));
}
if (ci == MAX_MCO) {
break;
}
c = cc[ci++];
}
if (ci != MAX_MCO && c != 0) {
xstrlcpy((char *)IObuff + iobuff_len, " ...", sizeof(IObuff) - iobuff_len);
}

msg(IObuff);
}




void ex_align(exarg_T *eap)
{
pos_T save_curpos;
int len;
int indent = 0;
int new_indent;
int has_tab;
int width;

if (curwin->w_p_rl) {

if (eap->cmdidx == CMD_right)
eap->cmdidx = CMD_left;
else if (eap->cmdidx == CMD_left)
eap->cmdidx = CMD_right;
}

width = atoi((char *)eap->arg);
save_curpos = curwin->w_cursor;
if (eap->cmdidx == CMD_left) { 
if (width >= 0)
indent = width;
} else {





if (width <= 0)
width = curbuf->b_p_tw;
if (width == 0 && curbuf->b_p_wm > 0) {
width = curwin->w_width_inner - curbuf->b_p_wm;
}
if (width <= 0) {
width = 80;
}
}

if (u_save((linenr_T)(eap->line1 - 1), (linenr_T)(eap->line2 + 1)) == FAIL)
return;

for (curwin->w_cursor.lnum = eap->line1;
curwin->w_cursor.lnum <= eap->line2; ++curwin->w_cursor.lnum) {
if (eap->cmdidx == CMD_left) 
new_indent = indent;
else {
has_tab = FALSE; 
len = linelen(eap->cmdidx == CMD_right ? &has_tab
: NULL) - get_indent();

if (len <= 0) 
continue;

if (eap->cmdidx == CMD_center)
new_indent = (width - len) / 2;
else {
new_indent = width - len; 





if (has_tab)
while (new_indent > 0) {
(void)set_indent(new_indent, 0);
if (linelen(NULL) <= width) {




do
(void)set_indent(++new_indent, 0);
while (linelen(NULL) <= width);
--new_indent;
break;
}
--new_indent;
}
}
}
if (new_indent < 0)
new_indent = 0;
(void)set_indent(new_indent, 0); 
}
changed_lines(eap->line1, 0, eap->line2 + 1, 0L, true);
curwin->w_cursor = save_curpos;
beginline(BL_WHITE | BL_FIX);
}




static int linelen(int *has_tab)
{
char_u *line;
char_u *first;
char_u *last;
int save;
int len;


line = get_cursor_line_ptr();
first = skipwhite(line);


for (last = first + STRLEN(first);
last > first && ascii_iswhite(last[-1]); --last)
;
save = *last;
*last = NUL;

len = linetabsize(line);

if (has_tab != NULL) {
*has_tab = vim_strchr(first, TAB) != NULL;
}
*last = save;

return len;
}



static char_u *sortbuf1;
static char_u *sortbuf2;

static int sort_ic; 
static int sort_nr; 
static int sort_rx; 
static int sort_flt; 

static int sort_abort; 


typedef struct {
linenr_T lnum; 
union {
struct {
varnumber_T start_col_nr; 
varnumber_T end_col_nr; 
} line;
struct {
varnumber_T value; 
bool is_number; 
} num;
float_T value_flt; 
} st_u;
} sorti_T;


static int sort_compare(const void *s1, const void *s2)
{
sorti_T l1 = *(sorti_T *)s1;
sorti_T l2 = *(sorti_T *)s2;
int result = 0;




if (sort_abort)
return 0;
fast_breakcheck();
if (got_int)
sort_abort = TRUE;



if (sort_nr) {
if (l1.st_u.num.is_number != l2.st_u.num.is_number) {
result = l1.st_u.num.is_number - l2.st_u.num.is_number;
} else {
result = l1.st_u.num.value == l2.st_u.num.value
? 0
: l1.st_u.num.value > l2.st_u.num.value
? 1
: -1;
}
} else if (sort_flt) {
result = l1.st_u.value_flt == l2.st_u.value_flt
? 0 : l1.st_u.value_flt > l2.st_u.value_flt
? 1 : -1;
} else {



memcpy(sortbuf1, ml_get(l1.lnum) + l1.st_u.line.start_col_nr,
l1.st_u.line.end_col_nr - l1.st_u.line.start_col_nr + 1);
sortbuf1[l1.st_u.line.end_col_nr - l1.st_u.line.start_col_nr] = NUL;
memcpy(sortbuf2, ml_get(l2.lnum) + l2.st_u.line.start_col_nr,
l2.st_u.line.end_col_nr - l2.st_u.line.start_col_nr + 1);
sortbuf2[l2.st_u.line.end_col_nr - l2.st_u.line.start_col_nr] = NUL;

result = sort_ic ? STRICMP(sortbuf1, sortbuf2)
: STRCMP(sortbuf1, sortbuf2);
}


if (result == 0)
return (int)(l1.lnum - l2.lnum);
return result;
}


void ex_sort(exarg_T *eap)
{
regmatch_T regmatch;
int len;
linenr_T lnum;
long maxlen = 0;
size_t count = (size_t)(eap->line2 - eap->line1 + 1);
size_t i;
char_u *p;
char_u *s;
char_u *s2;
char_u c; 
bool unique = false;
long deleted;
colnr_T start_col;
colnr_T end_col;
int sort_what = 0;


if (count <= 1) {
return;
}

if (u_save((linenr_T)(eap->line1 - 1), (linenr_T)(eap->line2 + 1)) == FAIL) {
return;
}
sortbuf1 = NULL;
sortbuf2 = NULL;
regmatch.regprog = NULL;
sorti_T *nrs = xmalloc(count * sizeof(sorti_T));

sort_abort = sort_ic = sort_rx = sort_nr = sort_flt = 0;
size_t format_found = 0;
bool change_occurred = false; 

for (p = eap->arg; *p != NUL; ++p) {
if (ascii_iswhite(*p)) {
} else if (*p == 'i') {
sort_ic = true;
} else if (*p == 'r') {
sort_rx = true;
} else if (*p == 'n') {
sort_nr = 1;
format_found++;
} else if (*p == 'f') {
sort_flt = 1;
format_found++;
} else if (*p == 'b') {
sort_what = STR2NR_BIN + STR2NR_FORCE;
format_found++;
} else if (*p == 'o') {
sort_what = STR2NR_OCT + STR2NR_FORCE;
format_found++;
} else if (*p == 'x') {
sort_what = STR2NR_HEX + STR2NR_FORCE;
format_found++;
} else if (*p == 'u') {
unique = true;
} else if (*p == '"') {

break;
} else if (check_nextcmd(p) != NULL) {
eap->nextcmd = check_nextcmd(p);
break;
} else if (!ASCII_ISALPHA(*p) && regmatch.regprog == NULL) {
s = skip_regexp(p + 1, *p, true, NULL);
if (*s != *p) {
EMSG(_(e_invalpat));
goto sortend;
}
*s = NUL;

if (s == p + 1) {
if (last_search_pat() == NULL) {
EMSG(_(e_noprevre));
goto sortend;
}
regmatch.regprog = vim_regcomp(last_search_pat(), RE_MAGIC);
} else {
regmatch.regprog = vim_regcomp(p + 1, RE_MAGIC);
}
if (regmatch.regprog == NULL) {
goto sortend;
}
p = s; 
regmatch.rm_ic = p_ic;
} else {
EMSG2(_(e_invarg2), p);
goto sortend;
}
}


if (format_found > 1) {
EMSG(_(e_invarg));
goto sortend;
}



sort_nr += sort_what;







for (lnum = eap->line1; lnum <= eap->line2; ++lnum) {
s = ml_get(lnum);
len = (int)STRLEN(s);
if (maxlen < len) {
maxlen = len;
}

start_col = 0;
end_col = len;
if (regmatch.regprog != NULL && vim_regexec(&regmatch, s, 0)) {
if (sort_rx) {
start_col = (colnr_T)(regmatch.startp[0] - s);
end_col = (colnr_T)(regmatch.endp[0] - s);
} else {
start_col = (colnr_T)(regmatch.endp[0] - s);
}
} else if (regmatch.regprog != NULL) {
end_col = 0;
}

if (sort_nr || sort_flt) {


s2 = s + end_col;
c = *s2;
*s2 = NUL;

p = s + start_col;
if (sort_nr) {
if (sort_what & STR2NR_HEX) {
s = skiptohex(p);
} else if (sort_what & STR2NR_BIN) {
s = (char_u *)skiptobin((char *)p);
} else {
s = skiptodigit(p);
}
if (s > p && s[-1] == '-') {
s--; 
}
if (*s == NUL) {

nrs[lnum - eap->line1].st_u.num.is_number = false;
nrs[lnum - eap->line1].st_u.num.value = 0;
} else {
nrs[lnum - eap->line1].st_u.num.is_number = true;
vim_str2nr(s, NULL, NULL, sort_what,
&nrs[lnum - eap->line1].st_u.num.value, NULL, 0);
}
} else {
s = skipwhite(p);
if (*s == '+') {
s = skipwhite(s + 1);
}

if (*s == NUL) {

nrs[lnum - eap->line1].st_u.value_flt = -DBL_MAX;
} else {
nrs[lnum - eap->line1].st_u.value_flt = strtod((char *)s, NULL);
}
}
*s2 = c;
} else {

nrs[lnum - eap->line1].st_u.line.start_col_nr = start_col;
nrs[lnum - eap->line1].st_u.line.end_col_nr = end_col;
}

nrs[lnum - eap->line1].lnum = lnum;

if (regmatch.regprog != NULL)
fast_breakcheck();
if (got_int)
goto sortend;
}


sortbuf1 = xmalloc(maxlen + 1);
sortbuf2 = xmalloc(maxlen + 1);


qsort((void *)nrs, count, sizeof(sorti_T), sort_compare);

if (sort_abort)
goto sortend;


lnum = eap->line2;
for (i = 0; i < count; i++) {
const linenr_T get_lnum = nrs[eap->forceit ? count - i - 1 : i].lnum;



if (get_lnum + ((linenr_T)count - 1) != lnum) {
change_occurred = true;
}

s = ml_get(get_lnum);
if (!unique || i == 0
|| (sort_ic ? STRICMP(s, sortbuf1) : STRCMP(s, sortbuf1)) != 0) {


STRCPY(sortbuf1, s);
if (ml_append(lnum++, sortbuf1, (colnr_T)0, false) == FAIL) {
break;
}
}
fast_breakcheck();
if (got_int)
goto sortend;
}


if (i == count) {
for (i = 0; i < count; ++i) {
ml_delete(eap->line1, false);
}
} else {
count = 0;
}


deleted = (long)(count - (lnum - eap->line2));
if (deleted > 0) {
mark_adjust(eap->line2 - deleted, eap->line2, (long)MAXLNUM, -deleted,
kExtmarkUndo);
msgmore(-deleted);
} else if (deleted < 0) {
mark_adjust(eap->line2, MAXLNUM, -deleted, 0L, kExtmarkUndo);
}
if (change_occurred || deleted != 0) {
changed_lines(eap->line1, 0, eap->line2 + 1, -deleted, true);
}

curwin->w_cursor.lnum = eap->line1;
beginline(BL_WHITE | BL_FIX);

sortend:
xfree(nrs);
xfree(sortbuf1);
xfree(sortbuf2);
vim_regfree(regmatch.regprog);
if (got_int) {
EMSG(_(e_interr));
}
}




void ex_retab(exarg_T *eap)
{
linenr_T lnum;
int got_tab = FALSE;
long num_spaces = 0;
long num_tabs;
long len;
long col;
long vcol;
long start_col = 0; 
long start_vcol = 0; 
int temp;
long old_len;
char_u *ptr;
char_u *new_line = (char_u *)1; 
int did_undo; 
int new_ts;
int save_list;
linenr_T first_line = 0; 
linenr_T last_line = 0; 

save_list = curwin->w_p_list;
curwin->w_p_list = 0; 

new_ts = getdigits_int(&(eap->arg), false, -1);
if (new_ts < 0) {
EMSG(_(e_positive));
return;
}
if (new_ts == 0)
new_ts = curbuf->b_p_ts;
for (lnum = eap->line1; !got_int && lnum <= eap->line2; ++lnum) {
ptr = ml_get(lnum);
col = 0;
vcol = 0;
did_undo = FALSE;
for (;; ) {
if (ascii_iswhite(ptr[col])) {
if (!got_tab && num_spaces == 0) {

start_vcol = vcol;
start_col = col;
}
if (ptr[col] == ' ')
num_spaces++;
else
got_tab = TRUE;
} else {
if (got_tab || (eap->forceit && num_spaces > 1)) {



len = num_spaces = vcol - start_vcol;
num_tabs = 0;
if (!curbuf->b_p_et) {
temp = new_ts - (start_vcol % new_ts);
if (num_spaces >= temp) {
num_spaces -= temp;
num_tabs++;
}
num_tabs += num_spaces / new_ts;
num_spaces -= (num_spaces / new_ts) * new_ts;
}
if (curbuf->b_p_et || got_tab
|| (num_spaces + num_tabs < len)) {
if (did_undo == false) {
did_undo = true;
if (u_save((linenr_T)(lnum - 1),
(linenr_T)(lnum + 1)) == FAIL) {
new_line = NULL; 
break;
}
}


len = num_spaces + num_tabs;
old_len = (long)STRLEN(ptr);
new_line = xmalloc(old_len - col + start_col + len + 1);

if (start_col > 0)
memmove(new_line, ptr, (size_t)start_col);
memmove(new_line + start_col + len,
ptr + col, (size_t)(old_len - col + 1));
ptr = new_line + start_col;
for (col = 0; col < len; col++) {
ptr[col] = (col < num_tabs) ? '\t' : ' ';
}
ml_replace(lnum, new_line, false);
if (first_line == 0) {
first_line = lnum;
}
last_line = lnum;
ptr = new_line;
col = start_col + len;
}
}
got_tab = FALSE;
num_spaces = 0;
}
if (ptr[col] == NUL)
break;
vcol += chartabsize(ptr + col, (colnr_T)vcol);
if (has_mbyte)
col += (*mb_ptr2len)(ptr + col);
else
++col;
}
if (new_line == NULL) 
break;
line_breakcheck();
}
if (got_int)
EMSG(_(e_interr));

if (curbuf->b_p_ts != new_ts)
redraw_curbuf_later(NOT_VALID);
if (first_line != 0) {
changed_lines(first_line, 0, last_line + 1, 0L, true);
}

curwin->w_p_list = save_list; 

curbuf->b_p_ts = new_ts;
coladvance(curwin->w_curswant);

u_clearline();
}






int do_move(linenr_T line1, linenr_T line2, linenr_T dest)
{
char_u *str;
linenr_T l;
linenr_T extra; 
linenr_T num_lines; 
linenr_T last_line; 

if (dest >= line1 && dest < line2) {
EMSG(_("E134: Cannot move a range of lines into itself"));
return FAIL;
}



if (dest == line1 - 1 || dest == line2) {


if (dest >= line1) {
curwin->w_cursor.lnum = dest;
} else {
curwin->w_cursor.lnum = dest + (line2 - line1) + 1;
}
return OK;
}

num_lines = line2 - line1 + 1;





if (u_save(dest, dest + 1) == FAIL)
return FAIL;
for (extra = 0, l = line1; l <= line2; l++) {
str = vim_strsave(ml_get(l + extra));
ml_append(dest + l - line1, str, (colnr_T)0, false);
xfree(str);
if (dest < line1)
extra++;
}
















last_line = curbuf->b_ml.ml_line_count;
mark_adjust_nofold(line1, line2, last_line - line2, 0L, kExtmarkNOOP);
changed_lines(last_line - num_lines + 1, 0, last_line + 1, num_lines, false);
if (dest >= line2) {
mark_adjust_nofold(line2 + 1, dest, -num_lines, 0L, kExtmarkNOOP);
FOR_ALL_TAB_WINDOWS(tab, win) {
if (win->w_buffer == curbuf) {
foldMoveRange(&win->w_folds, line1, line2, dest);
}
}
curbuf->b_op_start.lnum = dest - num_lines + 1;
curbuf->b_op_end.lnum = dest;
} else {
mark_adjust_nofold(dest + 1, line1 - 1, num_lines, 0L, kExtmarkNOOP);
FOR_ALL_TAB_WINDOWS(tab, win) {
if (win->w_buffer == curbuf) {
foldMoveRange(&win->w_folds, dest + 1, line1 - 1, line2);
}
}
curbuf->b_op_start.lnum = dest + 1;
curbuf->b_op_end.lnum = dest + num_lines;
}
curbuf->b_op_start.col = curbuf->b_op_end.col = 0;
mark_adjust_nofold(last_line - num_lines + 1, last_line,
-(last_line - dest - extra), 0L, kExtmarkNOOP);


int size = line2-line1+1;
int off = dest >= line2 ? -size : 0;
extmark_move_region(curbuf, line1-1, 0,
line2-line1+1, 0,
dest+off, 0, kExtmarkUndo);

changed_lines(last_line - num_lines + 1, 0, last_line + 1, -extra, false);


buf_updates_send_changes(curbuf, dest + 1, num_lines, 0, true);




if (u_save(line1 + extra - 1, line2 + extra + 1) == FAIL)
return FAIL;

for (l = line1; l <= line2; l++) {
ml_delete(line1 + extra, true);
}
if (!global_busy && num_lines > p_report) {
if (num_lines == 1)
MSG(_("1 line moved"));
else
smsg(_("%" PRId64 " lines moved"), (int64_t)num_lines);
}




if (dest >= line1)
curwin->w_cursor.lnum = dest;
else
curwin->w_cursor.lnum = dest + (line2 - line1) + 1;

if (line1 < dest) {
dest += num_lines + 1;
last_line = curbuf->b_ml.ml_line_count;
if (dest > last_line + 1)
dest = last_line + 1;
changed_lines(line1, 0, dest, 0L, false);
} else {
changed_lines(dest + 1, 0, line1 + num_lines, 0L, false);
}


buf_updates_send_changes(curbuf, line1 + extra, 0, num_lines, true);

return OK;
}




void ex_copy(linenr_T line1, linenr_T line2, linenr_T n)
{
linenr_T count;
char_u *p;

count = line2 - line1 + 1;
curbuf->b_op_start.lnum = n + 1;
curbuf->b_op_end.lnum = n + count;
curbuf->b_op_start.col = curbuf->b_op_end.col = 0;












if (u_save(n, n + 1) == FAIL)
return;

curwin->w_cursor.lnum = n;
while (line1 <= line2) {


p = vim_strsave(ml_get(line1));
ml_append(curwin->w_cursor.lnum, p, (colnr_T)0, false);
xfree(p);


if (line1 == n)
line1 = curwin->w_cursor.lnum;
++line1;
if (curwin->w_cursor.lnum < line1)
++line1;
if (curwin->w_cursor.lnum < line2)
++line2;
++curwin->w_cursor.lnum;
}

appended_lines_mark(n, count);

msgmore((long)count);
}

static char_u *prevcmd = NULL; 

#if defined(EXITFREE)
void free_prev_shellcmd(void)
{
xfree(prevcmd);
}

#endif






void do_bang(int addr_count, exarg_T *eap, int forceit, int do_in, int do_out)
{
char_u *arg = eap->arg; 
linenr_T line1 = eap->line1; 
linenr_T line2 = eap->line2; 
char_u *newcmd = NULL; 
int free_newcmd = FALSE; 
int ins_prevcmd;
char_u *t;
char_u *p;
char_u *trailarg;
int len;
int scroll_save = msg_scroll;






if (check_restricted() || check_secure())
return;

if (addr_count == 0) { 
msg_scroll = FALSE; 
autowrite_all();
msg_scroll = scroll_save;
}





ins_prevcmd = forceit;
trailarg = arg;
do {
len = (int)STRLEN(trailarg) + 1;
if (newcmd != NULL)
len += (int)STRLEN(newcmd);
if (ins_prevcmd) {
if (prevcmd == NULL) {
EMSG(_(e_noprev));
xfree(newcmd);
return;
}
len += (int)STRLEN(prevcmd);
}
t = xmalloc(len);
*t = NUL;
if (newcmd != NULL)
STRCAT(t, newcmd);
if (ins_prevcmd)
STRCAT(t, prevcmd);
p = t + STRLEN(t);
STRCAT(t, trailarg);
xfree(newcmd);
newcmd = t;





trailarg = NULL;
while (*p) {
if (*p == '!') {
if (p > newcmd && p[-1] == '\\')
STRMOVE(p - 1, p);
else {
trailarg = p;
*trailarg++ = NUL;
ins_prevcmd = TRUE;
break;
}
}
++p;
}
} while (trailarg != NULL);

xfree(prevcmd);
prevcmd = newcmd;

if (bangredo) { 



char_u *cmd = vim_strsave_escaped(prevcmd, (char_u *)"%#");

AppendToRedobuffLit(cmd, -1);
xfree(cmd);
AppendToRedobuff("\n");
bangredo = false;
}



if (*p_shq != NUL) {
newcmd = xmalloc(STRLEN(prevcmd) + 2 * STRLEN(p_shq) + 1);
STRCPY(newcmd, p_shq);
STRCAT(newcmd, prevcmd);
STRCAT(newcmd, p_shq);
free_newcmd = TRUE;
}
if (addr_count == 0) { 

msg_start();
msg_putchar(':');
msg_putchar('!');
msg_outtrans(newcmd);
msg_clr_eos();
ui_cursor_goto(msg_row, msg_col);

do_shell(newcmd, 0);
} else { 


do_filter(line1, line2, eap, newcmd, do_in, do_out);
apply_autocmds(EVENT_SHELLFILTERPOST, NULL, NULL, FALSE, curbuf);
}
if (free_newcmd)
xfree(newcmd);
}














static void do_filter(
linenr_T line1,
linenr_T line2,
exarg_T *eap, 
char_u *cmd,
int do_in,
int do_out)
{
char_u *itmp = NULL;
char_u *otmp = NULL;
linenr_T linecount;
linenr_T read_linecount;
pos_T cursor_save;
char_u *cmd_buf;
buf_T *old_curbuf = curbuf;
int shell_flags = 0;

if (*cmd == NUL) 
return;


cursor_save = curwin->w_cursor;
linecount = line2 - line1 + 1;
curwin->w_cursor.lnum = line1;
curwin->w_cursor.col = 0;
changed_line_abv_curs();
invalidate_botline();














if (do_out)
shell_flags |= kShellOptDoOut;

if (!do_in && do_out && !p_stmp) {

shell_flags |= kShellOptRead;
curwin->w_cursor.lnum = line2;
} else if (do_in && !do_out && !p_stmp) {

shell_flags |= kShellOptWrite;
curbuf->b_op_start.lnum = line1;
curbuf->b_op_end.lnum = line2;
} else if (do_in && do_out && !p_stmp) {


shell_flags |= kShellOptRead | kShellOptWrite;
curbuf->b_op_start.lnum = line1;
curbuf->b_op_end.lnum = line2;
curwin->w_cursor.lnum = line2;
} else if ((do_in && (itmp = vim_tempname()) == NULL)
|| (do_out && (otmp = vim_tempname()) == NULL)) {
EMSG(_(e_notmp));
goto filterend;
}





++no_wait_return; 
if (itmp != NULL && buf_write(curbuf, itmp, NULL, line1, line2, eap,
false, false, false, true) == FAIL) {
msg_putchar('\n'); 
no_wait_return--;
if (!aborting()) {
EMSG2(_("E482: Can't create file %s"), itmp); 
}
goto filterend;
}
if (curbuf != old_curbuf)
goto filterend;

if (!do_out)
msg_putchar('\n');


cmd_buf = make_filter_cmd(cmd, itmp, otmp);
ui_cursor_goto(Rows - 1, 0);

if (do_out) {
if (u_save((linenr_T)(line2), (linenr_T)(line2 + 1)) == FAIL) {
xfree(cmd_buf);
goto error;
}
redraw_curbuf_later(VALID);
}
read_linecount = curbuf->b_ml.ml_line_count;


call_shell(cmd_buf, kShellOptFilter | shell_flags, NULL);
xfree(cmd_buf);

did_check_timestamps = FALSE;
need_check_timestamps = TRUE;




os_breakcheck();
got_int = FALSE;

if (do_out) {
if (otmp != NULL) {
if (readfile(otmp, NULL, line2, (linenr_T)0, (linenr_T)MAXLNUM, eap,
READ_FILTER) != OK) {
if (!aborting()) {
msg_putchar('\n');
EMSG2(_(e_notread), otmp);
}
goto error;
}
if (curbuf != old_curbuf)
goto filterend;
}

read_linecount = curbuf->b_ml.ml_line_count - read_linecount;

if (shell_flags & kShellOptRead) {
curbuf->b_op_start.lnum = line2 + 1;
curbuf->b_op_end.lnum = curwin->w_cursor.lnum;
appended_lines_mark(line2, read_linecount);
}

if (do_in) {
if (cmdmod.keepmarks || vim_strchr(p_cpo, CPO_REMMARK) == NULL) {



if (read_linecount >= linecount) {

mark_adjust(line1, line2, linecount, 0L, kExtmarkNOOP);
} else {


mark_adjust(line1, line1 + read_linecount - 1, linecount, 0L,
kExtmarkNOOP);
mark_adjust(line1 + read_linecount, line2, MAXLNUM, 0L,
kExtmarkNOOP);
}
}





curwin->w_cursor.lnum = line1;
del_lines(linecount, TRUE);
curbuf->b_op_start.lnum -= linecount; 
curbuf->b_op_end.lnum -= linecount; 
write_lnum_adjust(-linecount); 

foldUpdate(curwin, curbuf->b_op_start.lnum, curbuf->b_op_end.lnum);
} else {



linecount = curbuf->b_op_end.lnum - curbuf->b_op_start.lnum + 1;
curwin->w_cursor.lnum = curbuf->b_op_end.lnum;
}

beginline(BL_WHITE | BL_FIX); 
--no_wait_return;

if (linecount > p_report) {
if (do_in) {
vim_snprintf((char *)msg_buf, sizeof(msg_buf),
_("%" PRId64 " lines filtered"), (int64_t)linecount);
if (msg(msg_buf) && !msg_scroll)

set_keep_msg(msg_buf, 0);
} else
msgmore((long)linecount);
}
} else {
error:

curwin->w_cursor = cursor_save;
--no_wait_return;
wait_return(FALSE);
}

filterend:

if (curbuf != old_curbuf) {
--no_wait_return;
EMSG(_("E135: *Filter* Autocommands must not change current buffer"));
}
if (itmp != NULL)
os_remove((char *)itmp);
if (otmp != NULL)
os_remove((char *)otmp);
xfree(itmp);
xfree(otmp);
}



void
do_shell(
char_u *cmd,
int flags 
)
{



if (check_restricted() || check_secure()) {
msg_end();
return;
}






msg_putchar('\r'); 
msg_putchar('\n'); 


if (p_warn
&& !autocmd_busy
&& msg_silent == 0)
FOR_ALL_BUFFERS(buf) {
if (bufIsChanged(buf)) {
MSG_PUTS(_("[No write since last change]\n"));
break;
}
}



ui_cursor_goto(msg_row, msg_col);
(void)call_shell(cmd, flags, NULL);
msg_didout = true;
did_check_timestamps = false;
need_check_timestamps = true;



msg_row = Rows - 1;
msg_col = 0;

apply_autocmds(EVENT_SHELLCMDPOST, NULL, NULL, FALSE, curbuf);
}

#if !defined(UNIX)
static char *find_pipe(const char *cmd)
{
bool inquote = false;

for (const char *p = cmd; *p != NUL; p++) {
if (!inquote && *p == '|') {
return p;
}
if (*p == '"') {
inquote = !inquote;
} else if (rem_backslash((const char_u *)p)) {
p++;
}
}
return NULL;
}
#endif








char_u *make_filter_cmd(char_u *cmd, char_u *itmp, char_u *otmp)
{
bool is_fish_shell =
#if defined(UNIX)
STRNCMP(invocation_path_tail(p_sh, NULL), "fish", 4) == 0;
#else
false;
#endif

size_t len = STRLEN(cmd) + 1; 

len += is_fish_shell ? sizeof("begin; ""; end") - 1
: sizeof("("")") - 1;

if (itmp != NULL) {
len += STRLEN(itmp) + sizeof(" { "" < "" } ") - 1;
}
if (otmp != NULL) {
len += STRLEN(otmp) + STRLEN(p_srr) + 2; // two extra spaces (" "),
}
char *const buf = xmalloc(len);

#if defined(UNIX)
// Put delimiters around the command (for concatenated commands) when
// redirecting input and/or output.
if (itmp != NULL || otmp != NULL) {
char *fmt = is_fish_shell ? "begin; %s; end"
: "(%s)";
vim_snprintf(buf, len, fmt, (char *)cmd);
} else {
xstrlcpy(buf, (char *)cmd, len);
}

if (itmp != NULL) {
xstrlcat(buf, " < ", len - 1);
xstrlcat(buf, (const char *)itmp, len - 1);
}
#else
// For shells that don't understand braces around commands, at least allow
// the use of commands in a pipe.
xstrlcpy(buf, (char *)cmd, len);
if (itmp != NULL) {
// If there is a pipe, we have to put the '<' in front of it.
// Don't do this when 'shellquote' is not empty, otherwise the
// redirection would be inside the quotes.
if (*p_shq == NUL) {
char *const p = find_pipe(buf);
if (p != NULL) {
*p = NUL;
}
}
xstrlcat(buf, " < ", len);
xstrlcat(buf, (const char *)itmp, len);
if (*p_shq == NUL) {
const char *const p = find_pipe((const char *)cmd);
if (p != NULL) {
xstrlcat(buf, " ", len - 1); // Insert a space before the '|' for DOS
xstrlcat(buf, p, len - 1);
}
}
}
#endif
if (otmp != NULL) {
append_redir(buf, len, (char *) p_srr, (char *) otmp);
}
return (char_u *) buf;
}

/// Append output redirection for the given file to the end of the buffer
///
/// @param[out] buf Buffer to append to.
/// @param[in] buflen Buffer length.
/// @param[in] opt Separator or format string to append: will append
/// `printf(' ' . opt, fname)` if `%s` is found in `opt` or
/// a space, opt, a space and then fname if `%s` is not found
/// there.
/// @param[in] fname File name to append.
void append_redir(char *const buf, const size_t buflen,
const char *const opt, const char *const fname)
{
char *const end = buf + strlen(buf);
// find "%s"
const char *p = opt;
for (; (p = strchr(p, '%')) != NULL; p++) {
if (p[1] == 's') { // found %s
break;
} else if (p[1] == '%') { // skip %%
p++;
}
}
if (p != NULL) {
*end = ' '; // not really needed? Not with sh, ksh or bash
vim_snprintf(end + 1, (size_t) (buflen - (end + 1 - buf)), opt, fname);
} else {
vim_snprintf(end, (size_t) (buflen - (end - buf)), " %s %s", opt, fname);
}
}

void print_line_no_prefix(linenr_T lnum, int use_number, int list)
{
char numbuf[30];

if (curwin->w_p_nu || use_number) {
vim_snprintf(numbuf, sizeof(numbuf), "%*" PRIdLINENR " ",
number_width(curwin), lnum);
msg_puts_attr(numbuf, HL_ATTR(HLF_N)); // Highlight line nrs.
}
msg_prt_line(ml_get(lnum), list);
}

/*
* Print a text line. Also in silent mode ("ex -s").
*/
void print_line(linenr_T lnum, int use_number, int list)
{
int save_silent = silent_mode;

// apply :filter /pat/
if (message_filtered(ml_get(lnum))) {
return;
}

msg_start();
silent_mode = FALSE;
info_message = TRUE; /* use mch_msg(), not mch_errmsg() */
print_line_no_prefix(lnum, use_number, list);
if (save_silent) {
msg_putchar('\n');
ui_flush();
silent_mode = save_silent;
}
info_message = FALSE;
}

int rename_buffer(char_u *new_fname)
{
char_u *fname, *sfname, *xfname;
buf_T *buf;

buf = curbuf;
apply_autocmds(EVENT_BUFFILEPRE, NULL, NULL, FALSE, curbuf);
/* buffer changed, don't change name now */
if (buf != curbuf)
return FAIL;
if (aborting()) /* autocmds may abort script processing */
return FAIL;
/*
* The name of the current buffer will be changed.
* A new (unlisted) buffer entry needs to be made to hold the old file
* name, which will become the alternate file name.
* But don't set the alternate file name if the buffer didn't have a
* name.
*/
fname = curbuf->b_ffname;
sfname = curbuf->b_sfname;
xfname = curbuf->b_fname;
curbuf->b_ffname = NULL;
curbuf->b_sfname = NULL;
if (setfname(curbuf, new_fname, NULL, true) == FAIL) {
curbuf->b_ffname = fname;
curbuf->b_sfname = sfname;
return FAIL;
}
curbuf->b_flags |= BF_NOTEDITED;
if (xfname != NULL && *xfname != NUL) {
buf = buflist_new(fname, xfname, curwin->w_cursor.lnum, 0);
if (buf != NULL && !cmdmod.keepalt) {
curwin->w_alt_fnum = buf->b_fnum;
}
}
xfree(fname);
xfree(sfname);
apply_autocmds(EVENT_BUFFILEPOST, NULL, NULL, FALSE, curbuf);
/* Change directories when the 'acd' option is set. */
do_autochdir();
return OK;
}

/*
* ":file[!] [fname]".
*/
void ex_file(exarg_T *eap)
{
/* ":0file" removes the file name. Check for illegal uses ":3file",
* "0file name", etc. */
if (eap->addr_count > 0
&& (*eap->arg != NUL
|| eap->line2 > 0
|| eap->addr_count > 1)) {
EMSG(_(e_invarg));
return;
}

if (*eap->arg != NUL || eap->addr_count == 1) {
if (rename_buffer(eap->arg) == FAIL) {
return;
}
redraw_tabline = true;
}

// print file name if no argument or 'F' is not in 'shortmess'
if (*eap->arg == NUL || !shortmess(SHM_FILEINFO)) {
fileinfo(false, false, eap->forceit);
}
}

/*
* ":update".
*/
void ex_update(exarg_T *eap)
{
if (curbufIsChanged())
(void)do_write(eap);
}

/*
* ":write" and ":saveas".
*/
void ex_write(exarg_T *eap)
{
if (eap->usefilter) /* input lines to shell command */
do_bang(1, eap, FALSE, TRUE, FALSE);
else
(void)do_write(eap);
}

/*
* write current buffer to file 'eap->arg'
* if 'eap->append' is TRUE, append to the file
*
* if *eap->arg == NUL write to current file
*
* return FAIL for failure, OK otherwise
*/
int do_write(exarg_T *eap)
{
int other;
char_u *fname = NULL; /* init to shut up gcc */
char_u *ffname;
int retval = FAIL;
char_u *free_fname = NULL;
buf_T *alt_buf = NULL;
int name_was_missing;

if (not_writing()) /* check 'write' option */
return FAIL;

ffname = eap->arg;
if (*ffname == NUL) {
if (eap->cmdidx == CMD_saveas) {
EMSG(_(e_argreq));
goto theend;
}
other = FALSE;
} else {
fname = ffname;
free_fname = (char_u *)fix_fname((char *)ffname);
/*
* When out-of-memory, keep unexpanded file name, because we MUST be
* able to write the file in this situation.
*/
if (free_fname != NULL)
ffname = free_fname;
other = otherfile(ffname);
}

/*
* If we have a new file, put its name in the list of alternate file names.
*/
if (other) {
if (vim_strchr(p_cpo, CPO_ALTWRITE) != NULL
|| eap->cmdidx == CMD_saveas)
alt_buf = setaltfname(ffname, fname, (linenr_T)1);
else
alt_buf = buflist_findname(ffname);
if (alt_buf != NULL && alt_buf->b_ml.ml_mfp != NULL) {
/* Overwriting a file that is loaded in another buffer is not a
* good idea. */
EMSG(_(e_bufloaded));
goto theend;
}
}

// Writing to the current file is not allowed in readonly mode
// and a file name is required.
// "nofile" and "nowrite" buffers cannot be written implicitly either.
if (!other && (bt_dontwrite_msg(curbuf)
|| check_fname() == FAIL
|| check_readonly(&eap->forceit, curbuf))) {
goto theend;
}

if (!other) {
ffname = curbuf->b_ffname;
fname = curbuf->b_fname;
/*
* Not writing the whole file is only allowed with '!'.
*/
if ( (eap->line1 != 1
|| eap->line2 != curbuf->b_ml.ml_line_count)
&& !eap->forceit
&& !eap->append
&& !p_wa) {
if (p_confirm || cmdmod.confirm) {
if (vim_dialog_yesno(VIM_QUESTION, NULL,
(char_u *)_("Write partial file?"), 2) != VIM_YES)
goto theend;
eap->forceit = TRUE;
} else {
EMSG(_("E140: Use ! to write partial buffer"));
goto theend;
}
}
}

if (check_overwrite(eap, curbuf, fname, ffname, other) == OK) {
if (eap->cmdidx == CMD_saveas && alt_buf != NULL) {
buf_T *was_curbuf = curbuf;

apply_autocmds(EVENT_BUFFILEPRE, NULL, NULL, FALSE, curbuf);
apply_autocmds(EVENT_BUFFILEPRE, NULL, NULL, FALSE, alt_buf);
if (curbuf != was_curbuf || aborting()) {
/* buffer changed, don't change name now */
retval = FAIL;
goto theend;
}
/* Exchange the file names for the current and the alternate
* buffer. This makes it look like we are now editing the buffer
* under the new name. Must be done before buf_write(), because
* if there is no file name and 'cpo' contains 'F', it will set
* the file name. */
fname = alt_buf->b_fname;
alt_buf->b_fname = curbuf->b_fname;
curbuf->b_fname = fname;
fname = alt_buf->b_ffname;
alt_buf->b_ffname = curbuf->b_ffname;
curbuf->b_ffname = fname;
fname = alt_buf->b_sfname;
alt_buf->b_sfname = curbuf->b_sfname;
curbuf->b_sfname = fname;
buf_name_changed(curbuf);
apply_autocmds(EVENT_BUFFILEPOST, NULL, NULL, FALSE, curbuf);
apply_autocmds(EVENT_BUFFILEPOST, NULL, NULL, FALSE, alt_buf);
if (!alt_buf->b_p_bl) {
alt_buf->b_p_bl = TRUE;
apply_autocmds(EVENT_BUFADD, NULL, NULL, FALSE, alt_buf);
}
if (curbuf != was_curbuf || aborting()) {
/* buffer changed, don't write the file */
retval = FAIL;
goto theend;
}

// If 'filetype' was empty try detecting it now.
if (*curbuf->b_p_ft == NUL) {
if (au_has_group((char_u *)"filetypedetect")) {
(void)do_doautocmd((char_u *)"filetypedetect BufRead", true, NULL);
}
do_modelines(0);
}

/* Autocommands may have changed buffer names, esp. when
* 'autochdir' is set. */
fname = curbuf->b_sfname;
}

name_was_missing = curbuf->b_ffname == NULL;
retval = buf_write(curbuf, ffname, fname, eap->line1, eap->line2,
eap, eap->append, eap->forceit, TRUE, FALSE);

/* After ":saveas fname" reset 'readonly'. */
if (eap->cmdidx == CMD_saveas) {
if (retval == OK) {
curbuf->b_p_ro = FALSE;
redraw_tabline = TRUE;
}
}

// Change directories when the 'acd' option is set and the file name
// got changed or set.
if (eap->cmdidx == CMD_saveas || name_was_missing) {
do_autochdir();
}
}

theend:
xfree(free_fname);
return retval;
}

/*
* Check if it is allowed to overwrite a file. If b_flags has BF_NOTEDITED,
* BF_NEW or BF_READERR, check for overwriting current file.
* May set eap->forceit if a dialog says it's OK to overwrite.
* Return OK if it's OK, FAIL if it is not.
*/
int
check_overwrite(
exarg_T *eap,
buf_T *buf,
char_u *fname, // file name to be used (can differ from
// buf->ffname)
char_u *ffname, // full path version of fname
int other // writing under other name
)
{
/*
* write to other file or b_flags set or not writing the whole file:
* overwriting only allowed with '!'
*/
if ((other
|| (buf->b_flags & BF_NOTEDITED)
|| ((buf->b_flags & BF_NEW)
&& vim_strchr(p_cpo, CPO_OVERNEW) == NULL)
|| (buf->b_flags & BF_READERR))
&& !p_wa
&& !bt_nofile(buf)
&& os_path_exists(ffname)) {
if (!eap->forceit && !eap->append) {
#if defined(UNIX)
// It is possible to open a directory on Unix.
if (os_isdir(ffname)) {
EMSG2(_(e_isadir2), ffname);
return FAIL;
}
#endif
if (p_confirm || cmdmod.confirm) {
char_u buff[DIALOG_MSG_SIZE];

dialog_msg(buff, _("Overwrite existing file \"%s\"?"), fname);
if (vim_dialog_yesno(VIM_QUESTION, NULL, buff, 2) != VIM_YES)
return FAIL;
eap->forceit = TRUE;
} else {
EMSG(_(e_exists));
return FAIL;
}
}


if (other && !emsg_silent) {
char_u *dir;
char_u *p;
char_u *swapname;






if (*p_dir == NUL) {
dir = xmalloc(5);
STRCPY(dir, ".");
} else {
dir = xmalloc(MAXPATHL);
p = p_dir;
copy_option_part(&p, dir, MAXPATHL, ",");
}
swapname = makeswapname(fname, ffname, curbuf, dir);
xfree(dir);
if (os_path_exists(swapname)) {
if (p_confirm || cmdmod.confirm) {
char_u buff[DIALOG_MSG_SIZE];

dialog_msg(buff,
_("Swap file \"%s\" exists, overwrite anyway?"),
swapname);
if (vim_dialog_yesno(VIM_QUESTION, NULL, buff, 2)
!= VIM_YES) {
xfree(swapname);
return FAIL;
}
eap->forceit = TRUE;
} else {
EMSG2(_("E768: Swap file exists: %s (:silent! overrides)"),
swapname);
xfree(swapname);
return FAIL;
}
}
xfree(swapname);
}
}
return OK;
}




void ex_wnext(exarg_T *eap)
{
int i;

if (eap->cmd[1] == 'n')
i = curwin->w_arg_idx + (int)eap->line2;
else
i = curwin->w_arg_idx - (int)eap->line2;
eap->line1 = 1;
eap->line2 = curbuf->b_ml.ml_line_count;
if (do_write(eap) != FAIL)
do_argfile(eap, i);
}




void do_wqall(exarg_T *eap)
{
int error = 0;
int save_forceit = eap->forceit;

if (eap->cmdidx == CMD_xall || eap->cmdidx == CMD_wqall) {
exiting = true;
}

FOR_ALL_BUFFERS(buf) {
if (!bufIsChanged(buf) || bt_dontwrite(buf)) {
continue;
}







if (not_writing()) {
++error;
break;
}
if (buf->b_ffname == NULL) {
EMSGN(_("E141: No file name for buffer %" PRId64), buf->b_fnum);
++error;
} else if (check_readonly(&eap->forceit, buf)
|| check_overwrite(eap, buf, buf->b_fname, buf->b_ffname,
FALSE) == FAIL) {
++error;
} else {
bufref_T bufref;
set_bufref(&bufref, buf);
if (buf_write_all(buf, eap->forceit) == FAIL) {
error++;
}

if (!bufref_valid(&bufref)) {
buf = firstbuf;
}
}
eap->forceit = save_forceit; 
}
if (exiting) {
if (!error)
getout(0); 
not_exiting();
}
}





int not_writing(void)
{
if (p_write)
return FALSE;
EMSG(_("E142: File not written: Writing is disabled by 'write' option"));
return TRUE;
}






static int check_readonly(int *forceit, buf_T *buf)
{


if (!*forceit && (buf->b_p_ro
|| (os_path_exists(buf->b_ffname)
&& !os_file_is_writable((char *)buf->b_ffname)))) {
if ((p_confirm || cmdmod.confirm) && buf->b_fname != NULL) {
char_u buff[DIALOG_MSG_SIZE];

if (buf->b_p_ro)
dialog_msg(buff,
_(
"'readonly' option is set for \"%s\".\nDo you wish to write anyway?"),
buf->b_fname);
else
dialog_msg(buff,
_(
"File permissions of \"%s\" are read-only.\nIt may still be possible to write it.\nDo you wish to try?"),
buf->b_fname);

if (vim_dialog_yesno(VIM_QUESTION, NULL, buff, 2) == VIM_YES) {

*forceit = TRUE;
return FALSE;
} else
return TRUE;
} else if (buf->b_p_ro)
EMSG(_(e_readonly));
else
EMSG2(_("E505: \"%s\" is read-only (add ! to override)"),
buf->b_fname);
return TRUE;
}

return FALSE;
}












int getfile(int fnum, char_u *ffname, char_u *sfname, int setpm, linenr_T lnum, int forceit)
{
int other;
int retval;
char_u *free_me = NULL;

if (text_locked()) {
return GETFILE_ERROR;
}
if (curbuf_locked()) {
return GETFILE_ERROR;
}

if (fnum == 0) {

fname_expand(curbuf, &ffname, &sfname);
other = otherfile(ffname);
free_me = ffname; 
} else
other = (fnum != curbuf->b_fnum);

if (other) {
no_wait_return++; 
}
if (other && !forceit && curbuf->b_nwindows == 1 && !buf_hide(curbuf)
&& curbufIsChanged() && autowrite(curbuf, forceit) == FAIL) {
if (p_confirm && p_write) {
dialog_changed(curbuf, false);
}
if (curbufIsChanged()) {
no_wait_return--;
no_write_message();
retval = GETFILE_NOT_WRITTEN; 
goto theend;
}
}
if (other)
--no_wait_return;
if (setpm)
setpcmark();
if (!other) {
if (lnum != 0) {
curwin->w_cursor.lnum = lnum;
}
check_cursor_lnum();
beginline(BL_SOL | BL_FIX);
retval = GETFILE_SAME_FILE; 
} else if (do_ecmd(fnum, ffname, sfname, NULL, lnum,
(buf_hide(curbuf) ? ECMD_HIDE : 0)
+ (forceit ? ECMD_FORCEIT : 0), curwin) == OK) {
retval = GETFILE_OPEN_OTHER; 
} else {
retval = GETFILE_ERROR; 
}

theend:
xfree(free_me);
return retval;
}




























int do_ecmd(
int fnum,
char_u *ffname,
char_u *sfname,
exarg_T *eap, 
linenr_T newlnum,
int flags,
win_T *oldwin
)
{
int other_file; 
int oldbuf; 
int auto_buf = FALSE; 

char_u *new_name = NULL;
int did_set_swapcommand = FALSE;
buf_T *buf;
bufref_T bufref;
bufref_T old_curbuf;
char_u *free_fname = NULL;
int retval = FAIL;
long n;
pos_T orig_pos;
linenr_T topline = 0;
int newcol = -1;
int solcol = -1;
pos_T *pos;
char_u *command = NULL;
int did_get_winopts = FALSE;
int readfile_flags = 0;
bool did_inc_redrawing_disabled = false;
long *so_ptr = curwin->w_p_so >= 0 ? &curwin->w_p_so : &p_so;

if (eap != NULL)
command = eap->do_ecmd_cmd;

set_bufref(&old_curbuf, curbuf);

if (fnum != 0) {
if (fnum == curbuf->b_fnum) 
return OK; 
other_file = TRUE;
} else {

if (sfname == NULL)
sfname = ffname;
#if defined(USE_FNAME_CASE)
if (sfname != NULL)
path_fix_case(sfname); 
#endif

if ((flags & ECMD_ADDBUF) && (ffname == NULL || *ffname == NUL))
goto theend;

if (ffname == NULL)
other_file = TRUE;

else if (*ffname == NUL && curbuf->b_ffname == NULL)
other_file = FALSE;
else {
if (*ffname == NUL) { 
ffname = curbuf->b_ffname;
sfname = curbuf->b_fname;
}
free_fname = (char_u *)fix_fname((char *)ffname); 
if (free_fname != NULL)
ffname = free_fname;
other_file = otherfile(ffname);
}
}


if (!other_file && curbuf->terminal) {
check_arg_idx(curwin); 
maketitle(); 
retval = OK;
goto theend;
}






if ( ((!other_file && !(flags & ECMD_OLDBUF))
|| (curbuf->b_nwindows == 1
&& !(flags & (ECMD_HIDE | ECMD_ADDBUF))))
&& check_changed(curbuf, (p_awa ? CCGD_AW : 0)
| (other_file ? 0 : CCGD_MULTWIN)
| ((flags & ECMD_FORCEIT) ? CCGD_FORCEIT : 0)
| (eap == NULL ? 0 : CCGD_EXCMD))) {
if (fnum == 0 && other_file && ffname != NULL)
(void)setaltfname(ffname, sfname, newlnum < 0 ? 0 : newlnum);
goto theend;
}





reset_VIsual();

if ((command != NULL || newlnum > (linenr_T)0)
&& *get_vim_var_str(VV_SWAPCOMMAND) == NUL) {

const size_t len = (command != NULL) ? STRLEN(command) + 3 : 30;
char *const p = xmalloc(len);
if (command != NULL) {
vim_snprintf(p, len, ":%s\r", command);
} else {
vim_snprintf(p, len, "%" PRId64 "G", (int64_t)newlnum);
}
set_vim_var_string(VV_SWAPCOMMAND, p, -1);
did_set_swapcommand = TRUE;
xfree(p);
}





if (other_file) {
if (!(flags & ECMD_ADDBUF)) {
if (!cmdmod.keepalt)
curwin->w_alt_fnum = curbuf->b_fnum;
if (oldwin != NULL)
buflist_altfpos(oldwin);
}

if (fnum) {
buf = buflist_findnr(fnum);
} else {
if (flags & ECMD_ADDBUF) {
linenr_T tlnum = 1L;

if (command != NULL) {
tlnum = atol((char *)command);
if (tlnum <= 0)
tlnum = 1L;
}
(void)buflist_new(ffname, sfname, tlnum, BLN_LISTED);
goto theend;
}
buf = buflist_new(ffname, sfname, 0L,
BLN_CURBUF | (flags & ECMD_SET_HELP ? 0 : BLN_LISTED));

if (oldwin != NULL) {
oldwin = curwin;
}
set_bufref(&old_curbuf, curbuf);
}
if (buf == NULL)
goto theend;
if (buf->b_ml.ml_mfp == NULL) {

oldbuf = false;
} else {

oldbuf = true;
set_bufref(&bufref, buf);
(void)buf_check_timestamp(buf, false);


if (!bufref_valid(&bufref) || curbuf != old_curbuf.br_buf) {
goto theend;
}
if (aborting()) {

goto theend;
}
}



if ((oldbuf && newlnum == ECMD_LASTL) || newlnum == ECMD_LAST) {
pos = buflist_findfpos(buf);
newlnum = pos->lnum;
solcol = pos->col;
}







if (buf != curbuf) {










if (buf->b_fname != NULL) {
new_name = vim_strsave(buf->b_fname);
}
set_bufref(&au_new_curbuf, buf);
apply_autocmds(EVENT_BUFLEAVE, NULL, NULL, false, curbuf);
if (!bufref_valid(&au_new_curbuf)) {

delbuf_msg(new_name); 
goto theend;
}
if (aborting()) { 
xfree(new_name);
goto theend;
}
if (buf == curbuf) { 
auto_buf = true;
} else {
win_T *the_curwin = curwin;



the_curwin->w_closing = true;
buf->b_locked++;

if (curbuf == old_curbuf.br_buf) {
buf_copy_options(buf, BCO_ENTER);
}



u_sync(false);
close_buffer(oldwin, curbuf,
(flags & ECMD_HIDE) || curbuf->terminal ? 0 : DOBUF_UNLOAD,
false);

the_curwin->w_closing = false;
buf->b_locked--;


if (aborting() && curwin->w_buffer != NULL) {
xfree(new_name);
goto theend;
}

if (!bufref_valid(&au_new_curbuf)) {

delbuf_msg(new_name); 
goto theend;
}
if (buf == curbuf) { 
auto_buf = true;
} else {


if (curwin->w_buffer == NULL
|| curwin->w_s == &(curwin->w_buffer->b_s)) {
curwin->w_s = &(buf->b_s);
}

curwin->w_buffer = buf;
curbuf = buf;
++curbuf->b_nwindows;


if (!oldbuf && eap != NULL) {
set_file_options(TRUE, eap);
set_forced_fenc(eap);
}
}





get_winopts(curbuf);
did_get_winopts = TRUE;

}
xfree(new_name);
au_new_curbuf.br_buf = NULL;
au_new_curbuf.br_buf_free_count = 0;
}

curwin->w_pcmark.lnum = 1;
curwin->w_pcmark.col = 0;
} else { 
if ((flags & ECMD_ADDBUF)
|| check_fname() == FAIL) {
goto theend;
}
oldbuf = (flags & ECMD_OLDBUF);
}



RedrawingDisabled++;
did_inc_redrawing_disabled = true;

buf = curbuf;
if ((flags & ECMD_SET_HELP) || keep_help_flag) {
prepare_help_buffer();
} else if (!curbuf->b_help) {


set_buflisted(TRUE);
}



if (buf != curbuf)
goto theend;
if (aborting()) 
goto theend;




did_filetype = FALSE;








if (!other_file && !oldbuf) { 
set_last_cursor(curwin); 
if (newlnum == ECMD_LAST || newlnum == ECMD_LASTL) {
newlnum = curwin->w_cursor.lnum;
solcol = curwin->w_cursor.col;
}
buf = curbuf;
if (buf->b_fname != NULL) {
new_name = vim_strsave(buf->b_fname);
} else {
new_name = NULL;
}
set_bufref(&bufref, buf);
if (p_ur < 0 || curbuf->b_ml.ml_line_count <= p_ur) {


u_sync(false);
if (u_savecommon(0, curbuf->b_ml.ml_line_count + 1, 0, true)
== FAIL) {
xfree(new_name);
goto theend;
}
u_unchanged(curbuf);
buf_updates_unregister_all(curbuf);
buf_freeall(curbuf, BFA_KEEP_UNDO);


readfile_flags = READ_KEEP_UNDO;
} else {
buf_updates_unregister_all(curbuf);
buf_freeall(curbuf, 0); 
}


if (!bufref_valid(&bufref)) {
delbuf_msg(new_name); 
goto theend;
}
xfree(new_name);




if (buf != curbuf)
goto theend;
if (aborting()) 
goto theend;
buf_clear_file(curbuf);
curbuf->b_op_start.lnum = 0; 
curbuf->b_op_end.lnum = 0;
}






retval = OK;




check_arg_idx(curwin);

if (!auto_buf) {





curwin_init();



FOR_ALL_TAB_WINDOWS(tp, win) {
if (win->w_buffer == curbuf) {
foldUpdateAll(win);
}
}


do_autochdir();





orig_pos = curwin->w_cursor;
topline = curwin->w_topline;
if (!oldbuf) { 
swap_exists_action = SEA_DIALOG;
curbuf->b_flags |= BF_CHECK_RO; 




if (should_abort(open_buffer(FALSE, eap, readfile_flags)))
retval = FAIL;

if (swap_exists_action == SEA_QUIT)
retval = FAIL;
handle_swap_exists(&old_curbuf);
} else {



do_modelines(OPT_WINONLY);

apply_autocmds_retval(EVENT_BUFENTER, NULL, NULL, FALSE, curbuf,
&retval);
apply_autocmds_retval(EVENT_BUFWINENTER, NULL, NULL, FALSE, curbuf,
&retval);
}
check_arg_idx(curwin);




if (!equalpos(curwin->w_cursor, orig_pos)) {
const char_u *text = get_cursor_line_ptr();

if (curwin->w_cursor.lnum != orig_pos.lnum
|| curwin->w_cursor.col != (int)(skipwhite(text) - text)) {
newlnum = curwin->w_cursor.lnum;
newcol = curwin->w_cursor.col;
}
}
if (curwin->w_topline == topline)
topline = 0;


changed_line_abv_curs();

maketitle();
}




if (curwin->w_p_diff) {
diff_buf_add(curbuf);
diff_invalidate(curbuf);
}



if (did_get_winopts && curwin->w_p_spell && *curwin->w_s->b_p_spl != NUL)
(void)did_set_spelllang(curwin);

if (command == NULL) {
if (newcol >= 0) { 
curwin->w_cursor.lnum = newlnum;
curwin->w_cursor.col = newcol;
check_cursor();
} else if (newlnum > 0) { 
curwin->w_cursor.lnum = newlnum;
check_cursor_lnum();
if (solcol >= 0 && !p_sol) {

curwin->w_cursor.col = solcol;
check_cursor_col();
curwin->w_cursor.coladd = 0;
curwin->w_set_curswant = TRUE;
} else
beginline(BL_SOL | BL_FIX);
} else { 
if (exmode_active)
curwin->w_cursor.lnum = curbuf->b_ml.ml_line_count;
beginline(BL_WHITE | BL_FIX);
}
}


check_lnums(FALSE);





if (oldbuf
&& !auto_buf
) {
int msg_scroll_save = msg_scroll;



if (shortmess(SHM_OVERALL) && !exiting && p_verbose == 0)
msg_scroll = FALSE;
if (!msg_scroll) 
check_for_delay(FALSE);
msg_start();
msg_scroll = msg_scroll_save;
msg_scrolled_ign = TRUE;

if (!shortmess(SHM_FILEINFO)) {
fileinfo(false, true, false);
}

msg_scrolled_ign = FALSE;
}

if (command != NULL)
do_cmdline(command, NULL, NULL, DOCMD_VERBOSE);

if (curbuf->b_kmap_state & KEYMAP_INIT)
(void)keymap_init();

RedrawingDisabled--;
did_inc_redrawing_disabled = false;
if (!skip_redraw) {
n = *so_ptr;
if (topline == 0 && command == NULL) {
*so_ptr = 999; 
}
update_topline();
curwin->w_scbind_pos = curwin->w_topline;
*so_ptr = n;
redraw_curbuf_later(NOT_VALID); 
}

if (p_im)
need_start_insertmode = TRUE;


do_autochdir();


theend:
if (bufref_valid(&old_curbuf) && old_curbuf.br_buf->terminal != NULL) {
terminal_check_size(old_curbuf.br_buf->terminal);
}

if (did_inc_redrawing_disabled) {
RedrawingDisabled--;
}
if (did_set_swapcommand) {
set_vim_var_string(VV_SWAPCOMMAND, NULL, -1);
}
xfree(free_fname);
return retval;
}

static void delbuf_msg(char_u *name)
{
EMSG2(_("E143: Autocommands unexpectedly deleted new buffer %s"),
name == NULL ? (char_u *)"" : name);
xfree(name);
au_new_curbuf.br_buf = NULL;
au_new_curbuf.br_buf_free_count = 0;
}

static int append_indent = 0; 




void ex_append(exarg_T *eap)
{
char_u *theline;
bool did_undo = false;
linenr_T lnum = eap->line2;
int indent = 0;
char_u *p;
int vcol;
int empty = (curbuf->b_ml.ml_flags & ML_EMPTY);


if (eap->forceit)
curbuf->b_p_ai = !curbuf->b_p_ai;


if (eap->cmdidx != CMD_change && curbuf->b_p_ai && lnum > 0)
append_indent = get_indent_lnum(lnum);

if (eap->cmdidx != CMD_append)
--lnum;


if (empty && lnum == 1)
lnum = 0;

State = INSERT; 
if (curbuf->b_p_iminsert == B_IMODE_LMAP)
State |= LANGMAP;

for (;; ) {
msg_scroll = TRUE;
need_wait_return = FALSE;
if (curbuf->b_p_ai) {
if (append_indent >= 0) {
indent = append_indent;
append_indent = -1;
} else if (lnum > 0)
indent = get_indent_lnum(lnum);
}
ex_keep_indent = FALSE;
if (eap->getline == NULL) {


if (eap->nextcmd == NULL || *eap->nextcmd == NUL)
break;
p = vim_strchr(eap->nextcmd, NL);
if (p == NULL)
p = eap->nextcmd + STRLEN(eap->nextcmd);
theline = vim_strnsave(eap->nextcmd, (int)(p - eap->nextcmd));
if (*p != NUL)
++p;
eap->nextcmd = p;
} else {


int save_State = State;
State = CMDLINE;
theline = eap->getline(
eap->cstack->cs_looplevel > 0 ? -1 :
NUL, eap->cookie, indent, true);
State = save_State;
}
lines_left = Rows - 1;
if (theline == NULL)
break;


if (ex_keep_indent)
append_indent = indent;


vcol = 0;
for (p = theline; indent > vcol; ++p) {
if (*p == ' ')
++vcol;
else if (*p == TAB)
vcol += 8 - vcol % 8;
else
break;
}
if ((p[0] == '.' && p[1] == NUL)
|| (!did_undo && u_save(lnum, lnum + 1 + (empty ? 1 : 0))
== FAIL)) {
xfree(theline);
break;
}


if (p[0] == NUL)
theline[0] = NUL;

did_undo = true;
ml_append(lnum, theline, (colnr_T)0, false);
appended_lines_mark(lnum + (empty ? 1 : 0), 1L);

xfree(theline);
++lnum;

if (empty) {
ml_delete(2L, false);
empty = 0;
}
}
State = NORMAL;

if (eap->forceit)
curbuf->b_p_ai = !curbuf->b_p_ai;





curbuf->b_op_start.lnum = (eap->line2 < curbuf->b_ml.ml_line_count) ?
eap->line2 + 1 : curbuf->b_ml.ml_line_count;
if (eap->cmdidx != CMD_append)
--curbuf->b_op_start.lnum;
curbuf->b_op_end.lnum = (eap->line2 < lnum)
? lnum : curbuf->b_op_start.lnum;
curbuf->b_op_start.col = curbuf->b_op_end.col = 0;
curwin->w_cursor.lnum = lnum;
check_cursor_lnum();
beginline(BL_SOL | BL_FIX);

need_wait_return = FALSE; 
ex_no_reprint = TRUE;
}




void ex_change(exarg_T *eap)
{
linenr_T lnum;

if (eap->line2 >= eap->line1
&& u_save(eap->line1 - 1, eap->line2 + 1) == FAIL)
return;


if (eap->forceit ? !curbuf->b_p_ai : curbuf->b_p_ai)
append_indent = get_indent_lnum(eap->line1);

for (lnum = eap->line2; lnum >= eap->line1; --lnum) {
if (curbuf->b_ml.ml_flags & ML_EMPTY) 
break;
ml_delete(eap->line1, false);
}


check_cursor_lnum();
deleted_lines_mark(eap->line1, (long)(eap->line2 - lnum));


eap->line2 = eap->line1;
ex_append(eap);
}

void ex_z(exarg_T *eap)
{
char_u *x;
int64_t bigness;
char_u *kind;
int minus = 0;
linenr_T start, end, curs, i;
int j;
linenr_T lnum = eap->line2;



if (eap->forceit) {
bigness = curwin->w_height_inner;
} else if (ONE_WINDOW) {
bigness = curwin->w_p_scr * 2;
} else {
bigness = curwin->w_height_inner - 3;
}
if (bigness < 1) {
bigness = 1;
}

x = eap->arg;
kind = x;
if (*kind == '-' || *kind == '+' || *kind == '='
|| *kind == '^' || *kind == '.')
++x;
while (*x == '-' || *x == '+')
++x;

if (*x != 0) {
if (!ascii_isdigit(*x)) {
EMSG(_("E144: non-numeric argument to :z"));
return;
}
bigness = atol((char *)x);


if (bigness > 2 * curbuf->b_ml.ml_line_count || bigness < 0) {
bigness = 2 * curbuf->b_ml.ml_line_count;
}

p_window = bigness;
if (*kind == '=') {
bigness += 2;
}
}


if (*kind == '-' || *kind == '+')
for (x = kind + 1; *x == *kind; ++x)
;

switch (*kind) {
case '-':
start = lnum - bigness * (linenr_T)(x - kind) + 1;
end = start + bigness - 1;
curs = end;
break;

case '=':
start = lnum - (bigness + 1) / 2 + 1;
end = lnum + (bigness + 1) / 2 - 1;
curs = lnum;
minus = 1;
break;

case '^':
start = lnum - bigness * 2;
end = lnum - bigness;
curs = lnum - bigness;
break;

case '.':
start = lnum - (bigness + 1) / 2 + 1;
end = lnum + (bigness + 1) / 2 - 1;
curs = end;
break;

default: 
start = lnum;
if (*kind == '+')
start += bigness * (linenr_T)(x - kind - 1) + 1;
else if (eap->addr_count == 0)
++start;
end = start + bigness - 1;
curs = end;
break;
}

if (start < 1)
start = 1;

if (end > curbuf->b_ml.ml_line_count)
end = curbuf->b_ml.ml_line_count;

if (curs > curbuf->b_ml.ml_line_count) {
curs = curbuf->b_ml.ml_line_count;
} else if (curs < 1) {
curs = 1;
}

for (i = start; i <= end; i++) {
if (minus && i == lnum) {
msg_putchar('\n');

for (j = 1; j < Columns; j++)
msg_putchar('-');
}

print_line(i, eap->flags & EXFLAG_NR, eap->flags & EXFLAG_LIST);

if (minus && i == lnum) {
msg_putchar('\n');

for (j = 1; j < Columns; j++)
msg_putchar('-');
}
}

if (curwin->w_cursor.lnum != curs) {
curwin->w_cursor.lnum = curs;
curwin->w_cursor.col = 0;
}
ex_no_reprint = true;
}




bool check_restricted(void)
FUNC_ATTR_PURE FUNC_ATTR_WARN_UNUSED_RESULT
{
if (restricted) {
EMSG(_("E145: Shell commands and some functionality not allowed"
" in restricted mode"));
return true;
}
return false;
}






int check_secure(void)
{
if (secure) {
secure = 2;
EMSG(_(e_curdir));
return TRUE;
}



if (sandbox != 0) {
EMSG(_(e_sandbox));
return TRUE;
}
return FALSE;
}


static SubReplacementString old_sub = {NULL, 0, NULL};

static int global_need_beginline; 




void sub_get_replacement(SubReplacementString *const ret_sub)
FUNC_ATTR_NONNULL_ALL
{
*ret_sub = old_sub;
}






void sub_set_replacement(SubReplacementString sub)
{
xfree(old_sub.sub);
if (sub.additional_elements != old_sub.additional_elements) {
tv_list_unref(old_sub.additional_elements);
}
old_sub = sub;
}











static bool sub_joining_lines(exarg_T *eap, char_u *pat, char_u *sub,
char_u *cmd, bool save)
FUNC_ATTR_NONNULL_ARG(1, 3, 4)
{


if (pat != NULL
&& strcmp((const char *)pat, "\\n") == 0
&& *sub == NUL
&& (*cmd == NUL || (cmd[1] == NUL
&& (*cmd == 'g'
|| *cmd == 'l'
|| *cmd == 'p'
|| *cmd == '#')))) {
curwin->w_cursor.lnum = eap->line1;
if (*cmd == 'l') {
eap->flags = EXFLAG_LIST;
} else if (*cmd == '#') {
eap->flags = EXFLAG_NR;
} else if (*cmd == 'p') {
eap->flags = EXFLAG_PRINT;
}


linenr_T joined_lines_count = eap->line2 - eap->line1 + 1

+ (eap->line2 < curbuf->b_ml.ml_line_count ? 1 : 0);
if (joined_lines_count > 1) {
do_join(joined_lines_count, FALSE, TRUE, FALSE, true);
sub_nsubs = joined_lines_count - 1;
sub_nlines = 1;
do_sub_msg(false);
ex_may_print(eap);
}

if (save) {
if (!cmdmod.keeppatterns) {
save_re_pat(RE_SUBST, pat, p_magic);
}
add_to_history(HIST_SEARCH, pat, true, NUL);
}

return true;
}

return false;
}










static char_u *sub_grow_buf(char_u **new_start, int needed_len)
FUNC_ATTR_NONNULL_ARG(1) FUNC_ATTR_NONNULL_RET
{
int new_start_len = 0;
char_u *new_end;
if (*new_start == NULL) {



new_start_len = needed_len + 50;
*new_start = xmalloc(new_start_len);
**new_start = NUL;
new_end = *new_start;
} else {



size_t len = STRLEN(*new_start);
needed_len += len;
if (needed_len > new_start_len) {
new_start_len = needed_len + 50;
*new_start = xrealloc(*new_start, new_start_len);
}
new_end = *new_start + len;
}

return new_end;
}








static char_u *sub_parse_flags(char_u *cmd, subflags_T *subflags,
int *which_pat)
FUNC_ATTR_NONNULL_ALL FUNC_ATTR_NONNULL_RET
{

if (*cmd == '&') {
cmd++;
} else {
subflags->do_all = p_gd;
subflags->do_ask = false;
subflags->do_error = true;
subflags->do_print = false;
subflags->do_list = false;
subflags->do_count = false;
subflags->do_number = false;
subflags->do_ic = kSubHonorOptions;
}
while (*cmd) {


if (*cmd == 'g') {
subflags->do_all = !subflags->do_all;
} else if (*cmd == 'c') {
subflags->do_ask = !subflags->do_ask;
} else if (*cmd == 'n') {
subflags->do_count = true;
} else if (*cmd == 'e') {
subflags->do_error = !subflags->do_error;
} else if (*cmd == 'r') { 
*which_pat = RE_LAST;
} else if (*cmd == 'p') {
subflags->do_print = true;
} else if (*cmd == '#') {
subflags->do_print = true;
subflags->do_number = true;
} else if (*cmd == 'l') {
subflags->do_print = true;
subflags->do_list = true;
} else if (*cmd == 'i') { 
subflags->do_ic = kSubIgnoreCase;
} else if (*cmd == 'I') { 
subflags->do_ic = kSubMatchCase;
} else {
break;
}
cmd++;
}
if (subflags->do_count) {
subflags->do_ask = false;
}

return cmd;
}











static buf_T *do_sub(exarg_T *eap, proftime_T timeout,
bool do_buf_event)
{
long i = 0;
regmmatch_T regmatch;
static subflags_T subflags = {
.do_all = false,
.do_ask = false,
.do_count = false,
.do_error = true,
.do_print = false,
.do_list = false,
.do_number = false,
.do_ic = kSubHonorOptions
};
char_u *pat = NULL, *sub = NULL; 
int delimiter;
bool has_second_delim = false;
int sublen;
int got_quit = false;
int got_match = false;
int which_pat;
char_u *cmd = eap->arg;
linenr_T first_line = 0; 
linenr_T last_line= 0; 
linenr_T old_line_count = curbuf->b_ml.ml_line_count;
char_u *sub_firstline; 
bool endcolumn = false; 
PreviewLines preview_lines = { KV_INITIAL_VALUE, 0 };
static int pre_src_id = 0; 
static int pre_hl_id = 0;
buf_T *orig_buf = curbuf; 
pos_T old_cursor = curwin->w_cursor;
int start_nsubs;
int save_ma = 0;
int save_b_changed = curbuf->b_changed;
bool preview = (State & CMDPREVIEW);


if (!preview) {

u_save_cursor();
}

if (!global_busy) {
sub_nsubs = 0;
sub_nlines = 0;
}
start_nsubs = sub_nsubs;

if (eap->cmdidx == CMD_tilde)
which_pat = RE_LAST; 
else
which_pat = RE_SUBST; 


if (eap->cmd[0] == 's' && *cmd != NUL && !ascii_iswhite(*cmd)
&& vim_strchr((char_u *)"0123456789cegriIp|\"", *cmd) == NULL) {

if (isalpha(*cmd)) {
EMSG(_("E146: Regular expressions can't be delimited by letters"));
return NULL;
}





if (*cmd == '\\') {
++cmd;
if (vim_strchr((char_u *)"/?&", *cmd) == NULL) {
EMSG(_(e_backslash));
return NULL;
}
if (*cmd != '&') {
which_pat = RE_SEARCH; 
}
pat = (char_u *)""; 
delimiter = *cmd++; 
has_second_delim = true;
} else { 
which_pat = RE_LAST; 
delimiter = *cmd++; 
pat = cmd; 
cmd = skip_regexp(cmd, delimiter, p_magic, &eap->arg);
if (cmd[0] == delimiter) { 
*cmd++ = NUL; 
has_second_delim = true;
}
}





sub = cmd; 

while (cmd[0]) {
if (cmd[0] == delimiter) { 
*cmd++ = NUL; 
break;
}
if (cmd[0] == '\\' && cmd[1] != 0) { 
cmd++;
}
MB_PTR_ADV(cmd);
}

if (!eap->skip && !preview) {
sub_set_replacement((SubReplacementString) {
.sub = xstrdup((char *) sub),
.timestamp = os_time(),
.additional_elements = NULL,
});
}
} else if (!eap->skip) { 
if (old_sub.sub == NULL) { 
EMSG(_(e_nopresub));
return NULL;
}
pat = NULL; 
sub = (char_u *) old_sub.sub;



endcolumn = (curwin->w_curswant == MAXCOL);
}

if (sub != NULL && sub_joining_lines(eap, pat, sub, cmd, !preview)) {
return NULL;
}

cmd = sub_parse_flags(cmd, &subflags, &which_pat);

bool save_do_all = subflags.do_all; 
bool save_do_ask = subflags.do_ask; 


cmd = skipwhite(cmd);
if (ascii_isdigit(*cmd)) {
i = getdigits_long(&cmd, true, 0);
if (i <= 0 && !eap->skip && subflags.do_error) {
EMSG(_(e_zerocount));
return NULL;
}
eap->line1 = eap->line2;
eap->line2 += i - 1;
if (eap->line2 > curbuf->b_ml.ml_line_count)
eap->line2 = curbuf->b_ml.ml_line_count;
}




cmd = skipwhite(cmd);
if (*cmd && *cmd != '"') { 
eap->nextcmd = check_nextcmd(cmd);
if (eap->nextcmd == NULL) {
EMSG(_(e_trailing));
return NULL;
}
}

if (eap->skip) { 
return NULL;
}

if (!subflags.do_count && !MODIFIABLE(curbuf)) {

EMSG(_(e_modifiable));
return NULL;
}

if (search_regcomp(pat, RE_SUBST, which_pat, (preview ? 0 : SEARCH_HIS),
&regmatch) == FAIL) {
if (subflags.do_error) {
EMSG(_(e_invcmd));
}
return NULL;
}


if (subflags.do_ic == kSubIgnoreCase) {
regmatch.rmm_ic = true;
} else if (subflags.do_ic == kSubMatchCase) {
regmatch.rmm_ic = false;
}

sub_firstline = NULL;






if (!(sub[0] == '\\' && sub[1] == '='))
sub = regtilde(sub, p_magic);



linenr_T line2 = eap->line2;

for (linenr_T lnum = eap->line1;
lnum <= line2 && !got_quit && !aborting()
&& (!preview || preview_lines.lines_needed <= (linenr_T)p_cwh
|| lnum <= curwin->w_botline);
lnum++) {
long nmatch = vim_regexec_multi(&regmatch, curwin, curbuf, lnum,
(colnr_T)0, NULL, NULL);
if (nmatch) {
colnr_T copycol;
colnr_T matchcol;
colnr_T prev_matchcol = MAXCOL;
char_u *new_end, *new_start = NULL;
char_u *p1;
int did_sub = FALSE;
int lastone;
long nmatch_tl = 0; 
int do_again; 
int skip_match = false;
linenr_T sub_firstlnum; 













































sub_firstlnum = lnum;
copycol = 0;
matchcol = 0;


if (!got_match) {
setpcmark();
got_match = TRUE;
}









for (;; ) {
SubResult current_match = {
.start = { 0, 0 },
.end = { 0, 0 },
.pre_match = 0,
};






if (regmatch.startpos[0].lnum > 0) {
current_match.pre_match = lnum;
lnum += regmatch.startpos[0].lnum;
sub_firstlnum += regmatch.startpos[0].lnum;
nmatch -= regmatch.startpos[0].lnum;
XFREE_CLEAR(sub_firstline);
}



current_match.start.lnum = sub_firstlnum;



if (lnum > curbuf->b_ml.ml_line_count) {
break;
}
if (sub_firstline == NULL) {
sub_firstline = vim_strsave(ml_get(sub_firstlnum));
}



curwin->w_cursor.lnum = lnum;
do_again = FALSE;






if (matchcol == prev_matchcol
&& regmatch.endpos[0].lnum == 0
&& matchcol == regmatch.endpos[0].col) {
if (sub_firstline[matchcol] == NUL)


skip_match = TRUE;
else {

if (has_mbyte)
matchcol += mb_ptr2len(sub_firstline + matchcol);
else
++matchcol;
}

current_match.start.col = matchcol;
current_match.end.lnum = sub_firstlnum;
current_match.end.col = matchcol;
goto skip;
}



matchcol = regmatch.endpos[0].col;
prev_matchcol = matchcol;



if (subflags.do_count) {




if (nmatch > 1) {
matchcol = (colnr_T)STRLEN(sub_firstline);
nmatch = 1;
skip_match = TRUE;
}
sub_nsubs++;
did_sub = TRUE;


if (!(sub[0] == '\\' && sub[1] == '='))
goto skip;
}

if (subflags.do_ask && !preview) {
int typed = 0;



int save_State = State;
State = CONFIRM;
setmouse(); 
curwin->w_cursor.col = regmatch.startpos[0].col;

if (curwin->w_p_crb) {
do_check_cursorbind();
}



if (vim_strchr(p_cpo, CPO_UNDO) != NULL)
++no_u_sync;




while (subflags.do_ask) {
if (exmode_active) {
char_u *resp;
colnr_T sc, ec;

print_line_no_prefix(lnum, subflags.do_number, subflags.do_list);

getvcol(curwin, &curwin->w_cursor, &sc, NULL, NULL);
curwin->w_cursor.col = regmatch.endpos[0].col - 1;
if (curwin->w_cursor.col < 0) {
curwin->w_cursor.col = 0;
}
getvcol(curwin, &curwin->w_cursor, NULL, NULL, &ec);
if (subflags.do_number || curwin->w_p_nu) {
int numw = number_width(curwin) + 1;
sc += numw;
ec += numw;
}
msg_start();
for (i = 0; i < (long)sc; ++i)
msg_putchar(' ');
for (; i <= (long)ec; ++i)
msg_putchar('^');

resp = getexmodeline('?', NULL, 0, true);
if (resp != NULL) {
typed = *resp;
xfree(resp);
}
} else {
char_u *orig_line = NULL;
int len_change = 0;
int save_p_fen = curwin->w_p_fen;

curwin->w_p_fen = FALSE;


int temp = RedrawingDisabled;
RedrawingDisabled = 0;

if (new_start != NULL) {





orig_line = vim_strsave(ml_get(lnum));
char_u *new_line = concat_str(new_start, sub_firstline + copycol);




len_change = (int)STRLEN(new_line) - (int)STRLEN(orig_line);
curwin->w_cursor.col += len_change;
ml_replace(lnum, new_line, false);
}

search_match_lines = regmatch.endpos[0].lnum
- regmatch.startpos[0].lnum;
search_match_endcol = regmatch.endpos[0].col
+ len_change;
highlight_match = TRUE;

update_topline();
validate_cursor();
update_screen(SOME_VALID);
highlight_match = FALSE;
redraw_later(SOME_VALID);

curwin->w_p_fen = save_p_fen;
if (msg_row == Rows - 1)
msg_didout = FALSE; 
msg_starthere();
i = msg_scroll;
msg_scroll = 0; 

msg_no_more = true;
msg_ext_set_kind("confirm_sub");
smsg_attr(HL_ATTR(HLF_R), 
_("replace with %s (y/n/a/q/l/^E/^Y)?"), sub);
msg_no_more = FALSE;
msg_scroll = i;
showruler(TRUE);
ui_cursor_goto(msg_row, msg_col);
RedrawingDisabled = temp;

no_mapping++; 
typed = plain_vgetc();
no_mapping--;


msg_didout = FALSE; 
msg_col = 0;
gotocmdline(TRUE);


if (orig_line != NULL) {
ml_replace(lnum, orig_line, false);
}
}

need_wait_return = false; 
if (typed == 'q' || typed == ESC || typed == Ctrl_C) {
got_quit = true;
break;
}
if (typed == 'n')
break;
if (typed == 'y')
break;
if (typed == 'l') {

subflags.do_all = false;
line2 = lnum;
break;
}
if (typed == 'a') {
subflags.do_ask = false;
break;
}
if (typed == Ctrl_E)
scrollup_clamp();
else if (typed == Ctrl_Y)
scrolldown_clamp();
}
State = save_State;
setmouse();
if (vim_strchr(p_cpo, CPO_UNDO) != NULL)
--no_u_sync;

if (typed == 'n') {





if (nmatch > 1) {
matchcol = (colnr_T)STRLEN(sub_firstline);
skip_match = TRUE;
}
goto skip;
}
if (got_quit)
goto skip;
}



curwin->w_cursor.col = regmatch.startpos[0].col;



if (nmatch > curbuf->b_ml.ml_line_count - sub_firstlnum + 1) {
nmatch = curbuf->b_ml.ml_line_count - sub_firstlnum + 1;
current_match.end.lnum = sub_firstlnum + nmatch;
skip_match = true;
}

#define ADJUST_SUB_FIRSTLNUM() do { if (nmatch > 1) { sub_firstlnum += nmatch - 1; xfree(sub_firstline); sub_firstline = vim_strsave(ml_get(sub_firstlnum)); if (sub_firstlnum <= line2) { do_again = true; } else { subflags.do_all = false; } } if (skip_match) { xfree(sub_firstline); sub_firstline = vim_strsave((char_u *)""); copycol = 0; } } while (0)


























if (preview && !has_second_delim) {
current_match.start.col = regmatch.startpos[0].col;
if (current_match.end.lnum == 0) {
current_match.end.lnum = sub_firstlnum + nmatch - 1;
}
current_match.end.col = regmatch.endpos[0].col;

ADJUST_SUB_FIRSTLNUM();
lnum += nmatch - 1;

goto skip;
}




if (!preview || has_second_delim) {
long lnum_start = lnum; 
save_ma = curbuf->b_p_ma;
if (subflags.do_count) {

curbuf->b_p_ma = false;
sandbox++;
}


subflags_T subflags_save = subflags;

sublen = vim_regsub_multi(&regmatch,
sub_firstlnum - regmatch.startpos[0].lnum,
sub, sub_firstline, false, p_magic, true);



subflags = subflags_save;
if (aborting() || subflags.do_count) {
curbuf->b_p_ma = save_ma;
if (sandbox > 0) {
sandbox--;
}
goto skip;
}






if (nmatch == 1) {
p1 = sub_firstline;
} else {
p1 = ml_get(sub_firstlnum + nmatch - 1);
nmatch_tl += nmatch - 1;
}
size_t copy_len = regmatch.startpos[0].col - copycol;
new_end = sub_grow_buf(&new_start,
(STRLEN(p1) - regmatch.endpos[0].col)
+ copy_len + sublen + 1);


memmove(new_end, sub_firstline + copycol, (size_t)copy_len);
new_end += copy_len;



int start_col = new_end - new_start;
current_match.start.col = start_col;

(void)vim_regsub_multi(&regmatch,
sub_firstlnum - regmatch.startpos[0].lnum,
sub, new_end, true, p_magic, true);
sub_nsubs++;
did_sub = true;



curwin->w_cursor.col = 0;


copycol = regmatch.endpos[0].col;

ADJUST_SUB_FIRSTLNUM();








for (p1 = new_end; *p1; p1++) {
if (p1[0] == '\\' && p1[1] != NUL) { 
STRMOVE(p1, p1 + 1);
} else if (*p1 == CAR) {
if (u_inssub(lnum) == OK) { 
*p1 = NUL; 
ml_append(lnum - 1, new_start,
(colnr_T)(p1 - new_start + 1), false);
mark_adjust(lnum + 1, (linenr_T)MAXLNUM, 1L, 0L, kExtmarkNOOP);

if (subflags.do_ask) {
appended_lines(lnum - 1, 1L);
} else {
if (first_line == 0) {
first_line = lnum;
}
last_line = lnum + 1;
}

sub_firstlnum++;
lnum++;
line2++;

curwin->w_cursor.lnum++;

STRMOVE(new_start, p1 + 1);
p1 = new_start - 1;
}
} else if (has_mbyte) {
p1 += (*mb_ptr2len)(p1) - 1;
}
}
size_t new_endcol = STRLEN(new_start);
current_match.end.col = new_endcol;
current_match.end.lnum = lnum;



if (!preview) {
lpos_T start = regmatch.startpos[0], end = regmatch.endpos[0];
int matchcols = end.col - ((end.lnum == start.lnum)
? start.col : 0);
int subcols = new_endcol - ((lnum == lnum_start) ? start_col : 0);
extmark_splice(curbuf, lnum_start-1, start_col,
end.lnum-start.lnum, matchcols,
lnum-lnum_start, subcols, kExtmarkUndo);
}
}






skip:





lastone = (skip_match
|| got_int
|| got_quit
|| lnum > line2
|| !(subflags.do_all || do_again)
|| (sub_firstline[matchcol] == NUL && nmatch <= 1
&& !re_multiline(regmatch.regprog)));
nmatch = -1;










if (lastone
|| nmatch_tl > 0
|| (nmatch = vim_regexec_multi(&regmatch, curwin,
curbuf, sub_firstlnum,
matchcol, NULL, NULL)) == 0
|| regmatch.startpos[0].lnum > 0) {
if (new_start != NULL) {







STRCAT(new_start, sub_firstline + copycol);
matchcol = (colnr_T)STRLEN(sub_firstline) - matchcol;
prev_matchcol = (colnr_T)STRLEN(sub_firstline)
- prev_matchcol;

if (u_savesub(lnum) != OK) {
break;
}
ml_replace(lnum, new_start, true);

if (nmatch_tl > 0) {






++lnum;
if (u_savedel(lnum, nmatch_tl) != OK)
break;
for (i = 0; i < nmatch_tl; i++) {
ml_delete(lnum, false);
}
mark_adjust(lnum, lnum + nmatch_tl - 1,
(long)MAXLNUM, -nmatch_tl, kExtmarkNOOP);
if (subflags.do_ask) {
deleted_lines(lnum, nmatch_tl);
}
lnum--;
line2 -= nmatch_tl; 
nmatch_tl = 0;
}



if (subflags.do_ask) {
changed_bytes(lnum, 0);
} else {
if (first_line == 0) {
first_line = lnum;
}
last_line = lnum + 1;
}

sub_firstlnum = lnum;
xfree(sub_firstline); 
sub_firstline = new_start;
new_start = NULL;
matchcol = (colnr_T)STRLEN(sub_firstline) - matchcol;
prev_matchcol = (colnr_T)STRLEN(sub_firstline)
- prev_matchcol;
copycol = 0;
}
if (nmatch == -1 && !lastone)
nmatch = vim_regexec_multi(&regmatch, curwin, curbuf,
sub_firstlnum, matchcol, NULL, NULL);




if (nmatch <= 0) {



if (nmatch == -1)
lnum -= regmatch.startpos[0].lnum;

#define PUSH_PREVIEW_LINES() do { linenr_T match_lines = current_match.end.lnum - current_match.start.lnum +1; if (preview_lines.subresults.size > 0) { linenr_T last = kv_last(preview_lines.subresults).end.lnum; if (last == current_match.start.lnum) { preview_lines.lines_needed += match_lines - 1; } } else { preview_lines.lines_needed += match_lines; } kv_push(preview_lines.subresults, current_match); } while (0)















PUSH_PREVIEW_LINES();

break;
}
}

PUSH_PREVIEW_LINES();

line_breakcheck();
}

if (did_sub) {
sub_nlines++;
}
xfree(new_start); 
XFREE_CLEAR(sub_firstline); 
}

line_breakcheck();

if (profile_passed_limit(timeout)) {
got_quit = true;
}
}

if (first_line != 0) {



i = curbuf->b_ml.ml_line_count - old_line_count;
changed_lines(first_line, 0, last_line - i, i, false);

int64_t num_added = last_line - first_line;
int64_t num_removed = num_added - i;
buf_updates_send_changes(curbuf, first_line, num_added, num_removed,
do_buf_event);
}

xfree(sub_firstline); 


if (subflags.do_count) {
curwin->w_cursor = old_cursor;
}

if (sub_nsubs > start_nsubs) {

curbuf->b_op_start.lnum = eap->line1;
curbuf->b_op_end.lnum = line2;
curbuf->b_op_start.col = curbuf->b_op_end.col = 0;

if (!global_busy) {

if (!subflags.do_ask) {
if (endcolumn) {
coladvance((colnr_T)MAXCOL);
} else {
beginline(BL_WHITE | BL_FIX);
}
}
if (!preview && !do_sub_msg(subflags.do_count) && subflags.do_ask) {
MSG("");
}
} else {
global_need_beginline = true;
}
if (subflags.do_print) {
print_line(curwin->w_cursor.lnum, subflags.do_number, subflags.do_list);
}
} else if (!global_busy) {
if (got_int) {

EMSG(_(e_interr));
} else if (got_match) {

MSG("");
} else if (subflags.do_error) {

EMSG2(_(e_patnotf2), get_search_pat());
}
}

if (subflags.do_ask && hasAnyFolding(curwin)) {

changed_window_setting();
}

vim_regfree(regmatch.regprog);


subflags.do_all = save_do_all;
subflags.do_ask = save_do_ask;


buf_T *preview_buf = NULL;
size_t subsize = preview_lines.subresults.size;
if (preview && !aborting()) {
if (got_quit || profile_passed_limit(timeout)) { 
set_string_option_direct((char_u *)"icm", -1, (char_u *)"", OPT_FREE,
SID_NONE);
} else if (*p_icm != NUL && pat != NULL) {
if (pre_src_id == 0) {

pre_src_id = (int)nvim_create_namespace((String)STRING_INIT);
}
if (pre_hl_id == 0) {
pre_hl_id = syn_check_group((char_u *)S_LEN("Substitute"));
}
curbuf->b_changed = save_b_changed; 
preview_buf = show_sub(eap, old_cursor, &preview_lines,
pre_hl_id, pre_src_id);
if (subsize > 0) {
extmark_clear(orig_buf, pre_src_id, eap->line1-1, 0,
kv_last(preview_lines.subresults).end.lnum-1, MAXCOL);
}
}
}

kv_destroy(preview_lines.subresults);

return preview_buf;
#undef ADJUST_SUB_FIRSTLNUM
#undef PUSH_PREVIEW_LINES
} 






bool
do_sub_msg (
bool count_only 
)
{






if (((sub_nsubs > p_report && (KeyTyped || sub_nlines > 1 || p_report < 1))
|| count_only)
&& messaging()) {
if (got_int)
STRCPY(msg_buf, _("(Interrupted) "));
else
*msg_buf = NUL;
if (sub_nsubs == 1)
vim_snprintf_add((char *)msg_buf, sizeof(msg_buf),
"%s", count_only ? _("1 match") : _("1 substitution"));
else
vim_snprintf_add((char *)msg_buf, sizeof(msg_buf),
count_only ? _("%" PRId64 " matches")
: _("%" PRId64 " substitutions"),
(int64_t)sub_nsubs);
if (sub_nlines == 1)
vim_snprintf_add((char *)msg_buf, sizeof(msg_buf),
"%s", _(" on 1 line"));
else
vim_snprintf_add((char *)msg_buf, sizeof(msg_buf),
_(" on %" PRId64 " lines"), (int64_t)sub_nlines);
if (msg(msg_buf))

set_keep_msg(msg_buf, 0);
return true;
}
if (got_int) {
EMSG(_(e_interr));
return true;
}
return false;
}

static void global_exe_one(char_u *const cmd, const linenr_T lnum)
{
curwin->w_cursor.lnum = lnum;
curwin->w_cursor.col = 0;
if (*cmd == NUL || *cmd == '\n') {
do_cmdline((char_u *)"p", NULL, NULL, DOCMD_NOWAIT);
} else {
do_cmdline(cmd, NULL, NULL, DOCMD_NOWAIT);
}
}

















void ex_global(exarg_T *eap)
{
linenr_T lnum; 
int ndone = 0;
int type; 
char_u *cmd; 

char_u delim; 
char_u *pat;
regmmatch_T regmatch;
int match;
int which_pat;



if (global_busy && (eap->line1 != 1
|| eap->line2 != curbuf->b_ml.ml_line_count)) {

EMSG(_("E147: Cannot do :global recursive with a range"));
return;
}

if (eap->forceit) 
type = 'v';
else
type = *eap->cmd;
cmd = eap->arg;
which_pat = RE_LAST; 






if (*cmd == '\\') {
++cmd;
if (vim_strchr((char_u *)"/?&", *cmd) == NULL) {
EMSG(_(e_backslash));
return;
}
if (*cmd == '&')
which_pat = RE_SUBST; 
else
which_pat = RE_SEARCH; 
++cmd;
pat = (char_u *)"";
} else if (*cmd == NUL) {
EMSG(_("E148: Regular expression missing from global"));
return;
} else {
delim = *cmd; 
if (delim)
++cmd; 
pat = cmd; 
cmd = skip_regexp(cmd, delim, p_magic, &eap->arg);
if (cmd[0] == delim) 
*cmd++ = NUL; 
}

if (search_regcomp(pat, RE_BOTH, which_pat, SEARCH_HIS, &regmatch) == FAIL) {
EMSG(_(e_invcmd));
return;
}

if (global_busy) {
lnum = curwin->w_cursor.lnum;
match = vim_regexec_multi(&regmatch, curwin, curbuf, lnum,
(colnr_T)0, NULL, NULL);
if ((type == 'g' && match) || (type == 'v' && !match)) {
global_exe_one(cmd, lnum);
}
} else {

for (lnum = eap->line1; lnum <= eap->line2 && !got_int; lnum++) {

match = vim_regexec_multi(&regmatch, curwin, curbuf, lnum,
(colnr_T)0, NULL, NULL);
if ((type == 'g' && match) || (type == 'v' && !match)) {
ml_setmarked(lnum);
ndone++;
}
line_breakcheck();
}


if (got_int) {
MSG(_(e_interr));
} else if (ndone == 0) {
if (type == 'v') {
smsg(_("Pattern found in every line: %s"), pat);
} else {
smsg(_("Pattern not found: %s"), pat);
}
} else {
global_exe(cmd);
}
ml_clearmarked(); 
}
vim_regfree(regmatch.regprog);
}


void global_exe(char_u *cmd)
{
linenr_T old_lcount; 
buf_T *old_buf = curbuf; 
linenr_T lnum; 
int save_mapped_ctrl_c = mapped_ctrl_c;




setpcmark();


msg_didout = true;

mapped_ctrl_c = 0;

sub_nsubs = 0;
sub_nlines = 0;
global_need_beginline = false;
global_busy = 1;
old_lcount = curbuf->b_ml.ml_line_count;

while (!got_int && (lnum = ml_firstmarked()) != 0 && global_busy == 1) {
global_exe_one(cmd, lnum);
os_breakcheck();
}

mapped_ctrl_c = save_mapped_ctrl_c;
global_busy = 0;
if (global_need_beginline) {
beginline(BL_WHITE | BL_FIX);
} else {
check_cursor(); 
}



changed_line_abv_curs();



if (msg_col == 0 && msg_scrolled == 0) {
msg_didout = false;
}





if (!do_sub_msg(false) && curbuf == old_buf) {
msgmore(curbuf->b_ml.ml_line_count - old_lcount);
}
}

#if defined(EXITFREE)
void free_old_sub(void)
{
sub_set_replacement((SubReplacementString) {NULL, 0, NULL});
}

#endif





bool
prepare_tagpreview (
bool undo_sync 
)
{



if (!curwin->w_p_pvw) {
bool found_win = false;
FOR_ALL_WINDOWS_IN_TAB(wp, curtab) {
if (wp->w_p_pvw) {
win_enter(wp, undo_sync);
found_win = true;
break;
}
}
if (!found_win) {



if (win_split(g_do_tagpreview > 0 ? g_do_tagpreview : 0, 0)
== FAIL)
return false;
curwin->w_p_pvw = TRUE;
curwin->w_p_wfh = TRUE;
RESET_BINDING(curwin); 

curwin->w_p_diff = false; 
set_string_option_direct((char_u *)"fdc", -1, 
(char_u *)"0", OPT_FREE, SID_NONE);
return true;
}
}
return false;
}






void ex_help(exarg_T *eap)
{
char_u *arg;
char_u *tag;
FILE *helpfd; 
int n;
int i;
win_T *wp;
int num_matches;
char_u **matches;
char_u *p;
int empty_fnum = 0;
int alt_fnum = 0;
buf_T *buf;
int len;
char_u *lang;
const bool old_KeyTyped = KeyTyped;

if (eap != NULL) {




for (arg = eap->arg; *arg; ++arg) {
if (*arg == '\n' || *arg == '\r'
|| (*arg == '|' && arg[1] != NUL && arg[1] != '|')) {
*arg++ = NUL;
eap->nextcmd = arg;
break;
}
}
arg = eap->arg;

if (eap->forceit && *arg == NUL && !curbuf->b_help) {
EMSG(_("E478: Don't panic!"));
return;
}

if (eap->skip) 
return;
} else
arg = (char_u *)"";


p = arg + STRLEN(arg) - 1;
while (p > arg && ascii_iswhite(*p) && p[-1] != '\\')
*p-- = NUL;


lang = check_help_lang(arg);


if (*arg == NUL)
arg = (char_u *)"help.txt";




n = find_help_tags(arg, &num_matches, &matches,
eap != NULL && eap->forceit);

i = 0;
if (n != FAIL && lang != NULL)

for (i = 0; i < num_matches; ++i) {
len = (int)STRLEN(matches[i]);
if (len > 3 && matches[i][len - 3] == '@'
&& STRICMP(matches[i] + len - 2, lang) == 0)
break;
}
if (i >= num_matches || n == FAIL) {
if (lang != NULL)
EMSG3(_("E661: Sorry, no '%s' help for %s"), lang, arg);
else
EMSG2(_("E149: Sorry, no help for %s"), arg);
if (n != FAIL)
FreeWild(num_matches, matches);
return;
}


tag = vim_strsave(matches[i]);
FreeWild(num_matches, matches);





if (!bt_help(curwin->w_buffer)
|| cmdmod.tab != 0
) {
if (cmdmod.tab != 0) {
wp = NULL;
} else {
wp = NULL;
FOR_ALL_WINDOWS_IN_TAB(wp2, curtab) {
if (bt_help(wp2->w_buffer)) {
wp = wp2;
break;
}
}
}
if (wp != NULL && wp->w_buffer->b_nwindows > 0) {
win_enter(wp, true);
} else {


if ((helpfd = os_fopen((char *)p_hf, READBIN)) == NULL) {
smsg(_("Sorry, help file \"%s\" not found"), p_hf);
goto erret;
}
fclose(helpfd);




n = WSP_HELP;
if (cmdmod.split == 0 && curwin->w_width != Columns
&& curwin->w_width < 80)
n |= WSP_TOP;
if (win_split(0, n) == FAIL)
goto erret;

if (curwin->w_height < p_hh)
win_setheight((int)p_hh);






alt_fnum = curbuf->b_fnum;
(void)do_ecmd(0, NULL, NULL, NULL, ECMD_LASTL,
ECMD_HIDE + ECMD_SET_HELP,
NULL 
);
if (!cmdmod.keepalt)
curwin->w_alt_fnum = alt_fnum;
empty_fnum = curbuf->b_fnum;
}
}

if (!p_im)
restart_edit = 0; 



KeyTyped = old_KeyTyped;

do_tag(tag, DT_HELP, 1, FALSE, TRUE);




if (empty_fnum != 0 && curbuf->b_fnum != empty_fnum) {
buf = buflist_findnr(empty_fnum);
if (buf != NULL && buf->b_nwindows == 0)
wipe_buffer(buf, TRUE);
}


if (alt_fnum != 0 && curwin->w_alt_fnum == empty_fnum && !cmdmod.keepalt)
curwin->w_alt_fnum = alt_fnum;

erret:
xfree(tag);
}







char_u *check_help_lang(char_u *arg)
{
int len = (int)STRLEN(arg);

if (len >= 3 && arg[len - 3] == '@' && ASCII_ISALPHA(arg[len - 2])
&& ASCII_ISALPHA(arg[len - 1])) {
arg[len - 3] = NUL; 
return arg + len - 2;
}
return NULL;
}












int
help_heuristic(
char_u *matched_string,
int offset, 
int wrong_case 
)
{
int num_letters;
char_u *p;

num_letters = 0;
for (p = matched_string; *p; p++)
if (ASCII_ISALNUM(*p))
num_letters++;










if (offset > 0
&& ASCII_ISALNUM(matched_string[offset])
&& ASCII_ISALNUM(matched_string[offset - 1])) {
offset += 10000;
} else if (offset > 2) {
offset *= 200;
}
if (wrong_case) {
offset += 5000;
}


if (matched_string[0] == '+' && matched_string[1] != NUL) {
offset += 100;
}
return (int)(100 * num_letters + STRLEN(matched_string) + offset);
}





static int help_compare(const void *s1, const void *s2)
{
char *p1;
char *p2;

p1 = *(char **)s1 + strlen(*(char **)s1) + 1;
p2 = *(char **)s2 + strlen(*(char **)s2) + 1;
return strcmp(p1, p2);
}





int find_help_tags(const char_u *arg, int *num_matches, char_u ***matches,
bool keep_lang)
{
int i;
static const char *(mtable[]) = {
"*", "g*", "[*", "]*",
"/*", "/\\*", "\"*", "**",
"/\\(\\)", "/\\%(\\)",
"?", ":?", "?<CR>", "g?", "g?g?", "g??",
"-?", "q?", "v_g?",
"/\\?", "/\\z(\\)", "\\=", ":s\\=",
"[count]", "[quotex]",
"[range]", ":[range]",
"[pattern]", "\\|", "\\%$",
"s/\\~", "s/\\U", "s/\\L",
"s/\\1", "s/\\2", "s/\\3", "s/\\9"
};
static const char *(rtable[]) = {
"star", "gstar", "[star", "]star",
"/star", "/\\\\star", "quotestar", "starstar",
"/\\\\(\\\\)", "/\\\\%(\\\\)",
"?", ":?", "?<CR>", "g?", "g?g?", "g??",
"-?", "q?", "v_g?",
"/\\\\?", "/\\\\z(\\\\)", "\\\\=", ":s\\\\=",
"\\[count]", "\\[quotex]",
"\\[range]", ":\\[range]",
"\\[pattern]", "\\\\bar", "/\\\\%\\$",
"s/\\\\\\~", "s/\\\\U", "s/\\\\L",
"s/\\\\1", "s/\\\\2", "s/\\\\3", "s/\\\\9"
};
static const char *(expr_table[]) = {
"!=?", "!~?", "<=?", "<?", "==?", "=~?",
">=?", ">?", "is?", "isnot?"
};
char_u *d = IObuff; 

if (STRNICMP(arg, "expr-", 5) == 0) {



for (i = (int)ARRAY_SIZE(expr_table); --i >= 0; ) {
if (STRCMP(arg + 5, expr_table[i]) == 0) {
for (int si = 0, di = 0; ; si++) {
if (arg[si] == '~') {
d[di++] = '\\';
}
d[di++] = arg[si];
if (arg[si] == NUL) {
break;
}
}
break;
}
}
} else {


for (i = (int)ARRAY_SIZE(mtable); --i >= 0; ) {
if (STRCMP(arg, mtable[i]) == 0) {
STRCPY(d, rtable[i]);
break;
}
}
}

if (i < 0) { 





if (arg[0] == '\\'
&& ((arg[1] != NUL && arg[2] == NUL)
|| (vim_strchr((char_u *)"%_z@", arg[1]) != NULL
&& arg[2] != NUL))) {
STRCPY(d, "/\\\\");
STRCPY(d + 3, arg + 1);

if (d[3] == '_' && d[4] == '$')
STRCPY(d + 4, "\\$");
} else {





if ((arg[0] == '[' && (arg[1] == ':'
|| (arg[1] == '+' && arg[2] == '+')))
|| (arg[0] == '\\' && arg[1] == '{'))
*d++ = '\\';


if (*arg == '(' && arg[1] == '\'') {
arg++;
}
for (const char_u *s = arg; *s; s++) {






if (d - IObuff > IOSIZE - 10) { 
break;
}
switch (*s) {
case '|': STRCPY(d, "bar");
d += 3;
continue;
case '"': STRCPY(d, "quote");
d += 5;
continue;
case '*': *d++ = '.';
break;
case '?': *d++ = '.';
continue;
case '$':
case '.':
case '~': *d++ = '\\';
break;
}






if (*s < ' ' || (*s == '^' && s[1] && (ASCII_ISALPHA(s[1])
|| vim_strchr((char_u *)
"?@[\\]^",
s[1]) != NULL))) {
if (d > IObuff && d[-1] != '_' && d[-1] != '\\')
*d++ = '_'; 
STRCPY(d, "CTRL-");
d += 5;
if (*s < ' ') {
*d++ = *s + '@';
if (d[-1] == '\\')
*d++ = '\\'; 
} else
*d++ = *++s;
if (s[1] != NUL && s[1] != '_')
*d++ = '_'; 
continue;
} else if (*s == '^') 
*d++ = '\\';





else if (s[0] == '\\' && s[1] != '\\'
&& *arg == '/' && s == arg + 1)
*d++ = '\\';



if (STRNICMP(s, "CTRL-\\_", 7) == 0) {
STRCPY(d, "CTRL-\\\\");
d += 7;
s += 6;
}

*d++ = *s;



if (*s == '(' && (s[1] == '{' || s[1] =='[')) {
break;
}



if (*s == '\'' && s > arg && *arg == '\'') {
break;
}

if (*s == '}' && s > arg && *arg == '{') {
break;
}
}
*d = NUL;

if (*IObuff == '`') {
if (d > IObuff + 2 && d[-1] == '`') {

memmove(IObuff, IObuff + 1, STRLEN(IObuff));
d[-2] = NUL;
} else if (d > IObuff + 3 && d[-2] == '`' && d[-1] == ',') {

memmove(IObuff, IObuff + 1, STRLEN(IObuff));
d[-3] = NUL;
} else if (d > IObuff + 4 && d[-3] == '`'
&& d[-2] == '\\' && d[-1] == '.') {

memmove(IObuff, IObuff + 1, STRLEN(IObuff));
d[-4] = NUL;
}
}
}
}

*matches = NULL;
*num_matches = 0;
int flags = TAG_HELP | TAG_REGEXP | TAG_NAMES | TAG_VERBOSE | TAG_NO_TAGFUNC;
if (keep_lang) {
flags |= TAG_KEEP_LANG;
}
if (find_tags(IObuff, num_matches, matches, flags, (int)MAXCOL, NULL) == OK
&& *num_matches > 0) {


qsort((void *)*matches, (size_t)*num_matches,
sizeof(char_u *), help_compare);

while (*num_matches > TAG_MANY)
xfree((*matches)[--*num_matches]);
}
return OK;
}


static void prepare_help_buffer(void)
{
curbuf->b_help = true;
set_string_option_direct((char_u *)"buftype", -1, (char_u *)"help",
OPT_FREE|OPT_LOCAL, 0);






char_u *p = (char_u *)"!-~,^*,^|,^\",192-255";
if (STRCMP(curbuf->b_p_isk, p) != 0) {
set_string_option_direct((char_u *)"isk", -1, p, OPT_FREE|OPT_LOCAL, 0);
check_buf_options(curbuf);
(void)buf_init_chartab(curbuf, FALSE);
}


set_string_option_direct((char_u *)"fdm", -1, (char_u *)"manual",
OPT_FREE|OPT_LOCAL, 0);

curbuf->b_p_ts = 8; 
curwin->w_p_list = FALSE; 

curbuf->b_p_ma = FALSE; 
curbuf->b_p_bin = FALSE; 
curwin->w_p_nu = 0; 
curwin->w_p_rnu = 0; 
RESET_BINDING(curwin); 
curwin->w_p_arab = FALSE; 
curwin->w_p_rl = FALSE; 
curwin->w_p_fen = FALSE; 
curwin->w_p_diff = FALSE; 
curwin->w_p_spell = FALSE; 

set_buflisted(FALSE);
}





void fix_help_buffer(void)
{
linenr_T lnum;
char_u *line;
bool in_example = false;


if (STRCMP(curbuf->b_p_ft, "help") != 0) {
curbuf_lock++;
set_option_value("ft", 0L, "help", OPT_LOCAL);
curbuf_lock--;
}

if (!syntax_present(curwin)) {
for (lnum = 1; lnum <= curbuf->b_ml.ml_line_count; lnum++) {
line = ml_get_buf(curbuf, lnum, false);
const size_t len = STRLEN(line);
if (in_example && len > 0 && !ascii_iswhite(line[0])) {

if (line[0] == '<') {

line = ml_get_buf(curbuf, lnum, TRUE);
line[0] = ' ';
}
in_example = false;
}
if (!in_example && len > 0) {
if (line[len - 1] == '>' && (len == 1 || line[len - 2] == ' ')) {

line = ml_get_buf(curbuf, lnum, TRUE);
line[len - 1] = ' ';
in_example = true;
} else if (line[len - 1] == '~') {

line = ml_get_buf(curbuf, lnum, TRUE);
line[len - 1] = ' ';
}
}
}
}





char_u *const fname = path_tail(curbuf->b_fname);
if (fnamecmp(fname, "help.txt") == 0
|| (fnamencmp(fname, "help.", 5) == 0
&& ASCII_ISALPHA(fname[5])
&& ASCII_ISALPHA(fname[6])
&& TOLOWER_ASC(fname[7]) == 'x'
&& fname[8] == NUL)
) {
for (lnum = 1; lnum < curbuf->b_ml.ml_line_count; ++lnum) {
line = ml_get_buf(curbuf, lnum, FALSE);
if (strstr((char *)line, "*local-additions*") == NULL)
continue;



char_u *p = p_rtp;
while (*p != NUL) {
copy_option_part(&p, NameBuff, MAXPATHL, ",");
char_u *const rt = (char_u *)vim_getenv("VIMRUNTIME");
if (rt != NULL
&& path_full_compare(rt, NameBuff, false, true) != kEqualFiles) {
int fcount;
char_u **fnames;
char_u *s;
vimconv_T vc;
char_u *cp;


if (!add_pathsep((char *)NameBuff)
|| STRLCAT(NameBuff, "doc/*.??[tx]",
sizeof(NameBuff)) >= MAXPATHL) {
EMSG(_(e_fnametoolong));
continue;
}



char_u *buff_list[1] = {NameBuff};
if (gen_expand_wildcards(1, buff_list, &fcount,
&fnames, EW_FILE|EW_SILENT) == OK
&& fcount > 0) {


for (int i1 = 0; i1 < fcount; i1++) {
for (int i2 = 0; i2 < fcount; i2++) {
if (i1 == i2) {
continue;
}
if (fnames[i1] == NULL || fnames[i2] == NULL) {
continue;
}
const char_u *const f1 = fnames[i1];
const char_u *const f2 = fnames[i2];
const char_u *const t1 = path_tail(f1);
const char_u *const t2 = path_tail(f2);
const char_u *const e1 = STRRCHR(t1, '.');
const char_u *const e2 = STRRCHR(t2, '.');
if (e1 == NULL || e2 == NULL) {
continue;
}
if (fnamecmp(e1, ".txt") != 0
&& fnamecmp(e1, fname + 4) != 0) {

XFREE_CLEAR(fnames[i1]);
continue;
}
if (e1 - f1 != e2 - f2
|| fnamencmp(f1, f2, e1 - f1) != 0) {
continue;
}
if (fnamecmp(e1, ".txt") == 0
&& fnamecmp(e2, fname + 4) == 0) {

XFREE_CLEAR(fnames[i1]);
}
}
}
for (int fi = 0; fi < fcount; fi++) {
if (fnames[fi] == NULL) {
continue;
}

FILE *const fd = os_fopen((char *)fnames[fi], "r");
if (fd == NULL) {
continue;
}
vim_fgets(IObuff, IOSIZE, fd);
if (IObuff[0] == '*'
&& (s = vim_strchr(IObuff + 1, '*'))
!= NULL) {
TriState this_utf = kNone;


IObuff[0] = '|';
*s = '|';
while (*s != NUL) {
if (*s == '\r' || *s == '\n')
*s = NUL;




if (*s >= 0x80 && this_utf != kFalse) {
this_utf = kTrue;
const int l = utf_ptr2len(s);
if (l == 1) {
this_utf = kFalse;
}
s += l - 1;
}
++s;
}



vc.vc_type = CONV_NONE;
convert_setup(
&vc,
(char_u *)(this_utf == kTrue ? "utf-8" : "latin1"),
p_enc);
if (vc.vc_type == CONV_NONE) {

cp = IObuff;
} else {


cp = string_convert(&vc, IObuff, NULL);
if (cp == NULL) {
cp = IObuff;
}
}
convert_setup(&vc, NULL, NULL);

ml_append(lnum, cp, (colnr_T)0, false);
if (cp != IObuff) {
xfree(cp);
}
lnum++;
}
fclose(fd);
}
FreeWild(fcount, fnames);
}
}
xfree(rt);
}
break;
}
}
}




void ex_exusage(exarg_T *eap)
{
do_cmdline_cmd("help ex-cmd-index");
}




void ex_viusage(exarg_T *eap)
{
do_cmdline_cmd("help normal-index");
}









static void helptags_one(char_u *const dir, const char_u *const ext,
const char_u *const tagfname, const bool add_help_tags)
{
garray_T ga;
int filecount;
char_u **files;
char_u *p1, *p2;
char_u *s;
TriState utf8 = kNone;
bool mix = false; 


size_t dirlen = STRLCPY(NameBuff, dir, sizeof(NameBuff));
if (dirlen >= MAXPATHL
|| STRLCAT(NameBuff, "/**/*", sizeof(NameBuff)) >= MAXPATHL 
|| STRLCAT(NameBuff, ext, sizeof(NameBuff)) >= MAXPATHL) {
EMSG(_(e_fnametoolong));
return;
}



char_u *buff_list[1] = {NameBuff};
if (gen_expand_wildcards(1, buff_list, &filecount, &files,
EW_FILE|EW_SILENT) == FAIL
|| filecount == 0) {
if (!got_int) {
EMSG2(_("E151: No match: %s"), NameBuff);
}
return;
}





memcpy(NameBuff, dir, dirlen + 1);
if (!add_pathsep((char *)NameBuff)
|| STRLCAT(NameBuff, tagfname, sizeof(NameBuff)) >= MAXPATHL) {
EMSG(_(e_fnametoolong));
return;
}

FILE *const fd_tags = os_fopen((char *)NameBuff, "w");
if (fd_tags == NULL) {
EMSG2(_("E152: Cannot open %s for writing"), NameBuff);
FreeWild(filecount, files);
return;
}



ga_init(&ga, (int)sizeof(char_u *), 100);
if (add_help_tags
|| path_full_compare((char_u *)"$VIMRUNTIME/doc",
dir, false, true) == kEqualFiles) {
s = xmalloc(18 + STRLEN(tagfname));
sprintf((char *)s, "help-tags\t%s\t1\n", tagfname);
GA_APPEND(char_u *, &ga, s);
}


for (int fi = 0; fi < filecount && !got_int; fi++) {
FILE *const fd = os_fopen((char *)files[fi], "r");
if (fd == NULL) {
EMSG2(_("E153: Unable to open %s for reading"), files[fi]);
continue;
}
const char_u *const fname = files[fi] + dirlen + 1;

bool firstline = true;
while (!vim_fgets(IObuff, IOSIZE, fd) && !got_int) {
if (firstline) {

TriState this_utf8 = kNone;
for (s = IObuff; *s != NUL; s++) {
if (*s >= 0x80) {
this_utf8 = kTrue;
const int l = utf_ptr2len(s);
if (l == 1) {

this_utf8 = kFalse;
break;
}
s += l - 1;
}
}
if (this_utf8 == kNone) { 
this_utf8 = kFalse;
}
if (utf8 == kNone) { 
utf8 = this_utf8;
} else if (utf8 != this_utf8) {
EMSG2(_(
"E670: Mix of help file encodings within a language: %s"),
files[fi]);
mix = !got_int;
got_int = TRUE;
}
firstline = false;
}
p1 = vim_strchr(IObuff, '*'); 
while (p1 != NULL) {
p2 = (char_u *)strchr((const char *)p1 + 1, '*'); 
if (p2 != NULL && p2 > p1 + 1) { 
for (s = p1 + 1; s < p2; s++) {
if (*s == ' ' || *s == '\t' || *s == '|') {
break;
}
}




if (s == p2
&& (p1 == IObuff || p1[-1] == ' ' || p1[-1] == '\t')
&& (vim_strchr((char_u *)" \t\n\r", s[1]) != NULL
|| s[1] == '\0')) {
*p2 = '\0';
++p1;
s = xmalloc((p2 - p1) + STRLEN(fname) + 2);
GA_APPEND(char_u *, &ga, s);
sprintf((char *)s, "%s\t%s", p1, fname);


p2 = vim_strchr(p2 + 1, '*');
}
}
p1 = p2;
}
line_breakcheck();
}

fclose(fd);
}

FreeWild(filecount, files);

if (!got_int && ga.ga_data != NULL) {

sort_strings((char_u **)ga.ga_data, ga.ga_len);


for (int i = 1; i < ga.ga_len; i++) {
p1 = ((char_u **)ga.ga_data)[i - 1];
p2 = ((char_u **)ga.ga_data)[i];
while (*p1 == *p2) {
if (*p2 == '\t') {
*p2 = NUL;
vim_snprintf((char *)NameBuff, MAXPATHL,
_("E154: Duplicate tag \"%s\" in file %s/%s"),
((char_u **)ga.ga_data)[i], dir, p2 + 1);
EMSG(NameBuff);
*p2 = '\t';
break;
}
++p1;
++p2;
}
}

if (utf8 == kTrue) {
fprintf(fd_tags, "!_TAG_FILE_ENCODING\tutf-8\t//\n");
}


for (int i = 0; i < ga.ga_len; i++) {
s = ((char_u **)ga.ga_data)[i];
if (STRNCMP(s, "help-tags\t", 10) == 0) {

fputs((char *)s, fd_tags);
} else {
fprintf(fd_tags, "%s\t/" "*", s);
for (p1 = s; *p1 != '\t'; p1++) {

if (*p1 == '\\' || *p1 == '/') {
putc('\\', fd_tags);
}
putc(*p1, fd_tags);
}
fprintf(fd_tags, "*\n");
}
}
}
if (mix) {
got_int = false; 
}

GA_DEEP_CLEAR_PTR(&ga);
fclose(fd_tags); 
}


static void do_helptags(char_u *dirname, bool add_help_tags)
{
int len;
garray_T ga;
char_u lang[2];
char_u ext[5];
char_u fname[8];
int filecount;
char_u **files;


STRLCPY(NameBuff, dirname, sizeof(NameBuff));
if (!add_pathsep((char *)NameBuff)
|| STRLCAT(NameBuff, "**", sizeof(NameBuff)) >= MAXPATHL) {
EMSG(_(e_fnametoolong));
return;
}



char_u *buff_list[1] = {NameBuff};
if (gen_expand_wildcards(1, buff_list, &filecount, &files,
EW_FILE|EW_SILENT) == FAIL
|| filecount == 0) {
EMSG2(_("E151: No match: %s"), NameBuff);
return;
}



int j;
ga_init(&ga, 1, 10);
for (int i = 0; i < filecount; i++) {
len = (int)STRLEN(files[i]);
if (len <= 4) {
continue;
}
if (STRICMP(files[i] + len - 4, ".txt") == 0) {

lang[0] = 'e';
lang[1] = 'n';
} else if (files[i][len - 4] == '.'
&& ASCII_ISALPHA(files[i][len - 3])
&& ASCII_ISALPHA(files[i][len - 2])
&& TOLOWER_ASC(files[i][len - 1]) == 'x') {

lang[0] = TOLOWER_ASC(files[i][len - 3]);
lang[1] = TOLOWER_ASC(files[i][len - 2]);
} else
continue;


for (j = 0; j < ga.ga_len; j += 2) {
if (STRNCMP(lang, ((char_u *)ga.ga_data) + j, 2) == 0) {
break;
}
}
if (j == ga.ga_len) {

ga_grow(&ga, 2);
((char_u *)ga.ga_data)[ga.ga_len++] = lang[0];
((char_u *)ga.ga_data)[ga.ga_len++] = lang[1];
}
}




for (j = 0; j < ga.ga_len; j += 2) {
STRCPY(fname, "tags-xx");
fname[5] = ((char_u *)ga.ga_data)[j];
fname[6] = ((char_u *)ga.ga_data)[j + 1];
if (fname[5] == 'e' && fname[6] == 'n') {

fname[4] = NUL;
STRCPY(ext, ".txt");
} else {

STRCPY(ext, ".xxx");
ext[1] = fname[5];
ext[2] = fname[6];
}
helptags_one(dirname, ext, fname, add_help_tags);
}

ga_clear(&ga);
FreeWild(filecount, files);
}

static void
helptags_cb(char_u *fname, void *cookie)
{
do_helptags(fname, *(bool *)cookie);
}




void ex_helptags(exarg_T *eap)
{
expand_T xpc;
char_u *dirname;
bool add_help_tags = false;


if (STRNCMP(eap->arg, "++t", 3) == 0 && ascii_iswhite(eap->arg[3])) {
add_help_tags = true;
eap->arg = skipwhite(eap->arg + 3);
}

if (STRCMP(eap->arg, "ALL") == 0) {
do_in_path(p_rtp, (char_u *)"doc", DIP_ALL + DIP_DIR,
helptags_cb, &add_help_tags);
} else {
ExpandInit(&xpc);
xpc.xp_context = EXPAND_DIRECTORIES;
dirname = ExpandOne(&xpc, eap->arg, NULL,
WILD_LIST_NOTFOUND|WILD_SILENT, WILD_EXPAND_FREE);
if (dirname == NULL || !os_isdir(dirname)) {
EMSG2(_("E150: Not a directory: %s"), eap->arg);
} else {
do_helptags(dirname, add_help_tags);
}
xfree(dirname);
}
}




void ex_helpclose(exarg_T *eap)
{
FOR_ALL_WINDOWS_IN_TAB(win, curtab) {
if (bt_help(win->w_buffer)) {
win_close(win, false);
return;
}
}
}



static buf_T *show_sub(exarg_T *eap, pos_T old_cusr,
PreviewLines *preview_lines, int hl_id, int src_id)
FUNC_ATTR_NONNULL_ALL
{
static handle_T bufnr = 0; 

win_T *save_curwin = curwin;
cmdmod_T save_cmdmod = cmdmod;
char_u *save_shm_p = vim_strsave(p_shm);
PreviewLines lines = *preview_lines;
buf_T *orig_buf = curbuf;


buf_T *preview_buf = bufnr ? buflist_findnr(bufnr) : 0;
cmdmod.split = 0; 
cmdmod.tab = 0; 
cmdmod.noswapfile = true; 

set_string_option_direct((char_u *)"shm", -1, (char_u *)"F", OPT_FREE,
SID_NONE);

bool outside_curline = (eap->line1 != old_cusr.lnum
|| eap->line2 != old_cusr.lnum);
bool split = outside_curline && (*p_icm != 'n');
if (preview_buf == curbuf) { 
split = false;
preview_buf = NULL;
}


for (size_t i = 0; i < lines.subresults.size; i++) {
SubResult curres = lines.subresults.items[i];
if (curres.start.lnum >= old_cusr.lnum) {
curwin->w_cursor.lnum = curres.start.lnum;
curwin->w_cursor.col = curres.start.col;
break;
} 
}


linenr_T highest_num_line = 0;
int col_width = 0;

if (split && win_split((int)p_cwh, WSP_BOT) != FAIL) {
buf_open_scratch(preview_buf ? bufnr : 0, "[Preview]");
buf_clear();
preview_buf = curbuf;
bufnr = preview_buf->handle;
curbuf->b_p_bl = false;
curbuf->b_p_ma = true;
curbuf->b_p_ul = -1;
curbuf->b_p_tw = 0; 
curwin->w_p_cul = false;
curwin->w_p_cuc = false;
curwin->w_p_spell = false;
curwin->w_p_fen = false;

if (lines.subresults.size > 0) {
highest_num_line = kv_last(lines.subresults).end.lnum;
col_width = log10(highest_num_line) + 1 + 3;
}
} else {

preview_buf = NULL;
}

char *str = NULL; 
size_t old_line_size = 0;
size_t line_size = 0;
linenr_T linenr_preview = 0; 
linenr_T linenr_origbuf = 0; 
linenr_T next_linenr = 0; 

for (size_t matchidx = 0; matchidx < lines.subresults.size; matchidx++) {
SubResult match = lines.subresults.items[matchidx];

if (preview_buf) {
lpos_T p_start = { 0, match.start.col }; 
lpos_T p_end = { 0, match.end.col }; 

if (match.pre_match == 0) {
next_linenr = match.start.lnum;
} else {
next_linenr = match.pre_match;
}

if (next_linenr == linenr_origbuf) {
next_linenr++;
p_start.lnum = linenr_preview; 
p_end.lnum = linenr_preview; 
}

for (; next_linenr <= match.end.lnum; next_linenr++) {
if (next_linenr == match.start.lnum) {
p_start.lnum = linenr_preview + 1;
}
if (next_linenr == match.end.lnum) {
p_end.lnum = linenr_preview + 1;
}
char *line;
if (next_linenr == orig_buf->b_ml.ml_line_count + 1) {
line = "";
} else {
line = (char *)ml_get_buf(orig_buf, next_linenr, false);
line_size = strlen(line) + col_width + 1;


if (line_size > old_line_size) {
str = xrealloc(str, line_size * sizeof(char));
old_line_size = line_size;
}
}

snprintf(str, line_size, "|%*ld| %s", col_width - 3,
next_linenr, line);
if (linenr_preview == 0) {
ml_replace(1, (char_u *)str, true);
} else {
ml_append(linenr_preview, (char_u *)str, (colnr_T)line_size, false);
}
linenr_preview += 1;
}
linenr_origbuf = match.end.lnum;

bufhl_add_hl_pos_offset(preview_buf, src_id, hl_id, p_start,
p_end, col_width);
}
bufhl_add_hl_pos_offset(orig_buf, src_id, hl_id, match.start,
match.end, 0);
}
xfree(str);

redraw_later(SOME_VALID);
win_enter(save_curwin, false); 
update_topline();


int save_rd = RedrawingDisabled;
RedrawingDisabled = 0;
update_screen(SOME_VALID);
RedrawingDisabled = save_rd;

set_string_option_direct((char_u *)"shm", -1, save_shm_p, OPT_FREE, SID_NONE);
xfree(save_shm_p);

cmdmod = save_cmdmod;

return preview_buf;
}






void ex_substitute(exarg_T *eap)
{
bool preview = (State & CMDPREVIEW);
if (*p_icm == NUL || !preview) { 
(void)do_sub(eap, profile_zero(), true);
return;
}

block_autocmds(); 

char_u *save_eap = eap->arg;
garray_T save_view;
win_size_save(&save_view); 
save_search_patterns();
int save_changedtick = buf_get_changedtick(curbuf);
time_t save_b_u_time_cur = curbuf->b_u_time_cur;
u_header_T *save_b_u_newhead = curbuf->b_u_newhead;
long save_b_p_ul = curbuf->b_p_ul;
int save_w_p_cul = curwin->w_p_cul;
int save_w_p_cuc = curwin->w_p_cuc;

curbuf->b_p_ul = LONG_MAX; 
curwin->w_p_cul = false; 
curwin->w_p_cuc = false; 


bool save_hls = p_hls;
p_hls = false;
buf_T *preview_buf = do_sub(eap, profile_setlimit(p_rdt), false);
p_hls = save_hls;

if (save_changedtick != buf_get_changedtick(curbuf)) {

if (!u_undo_and_forget(1)) { abort(); }


curbuf->b_u_newhead = save_b_u_newhead;
curbuf->b_u_time_cur = save_b_u_time_cur;
buf_set_changedtick(curbuf, save_changedtick);
}
if (buf_valid(preview_buf)) {

close_windows(preview_buf, false);
}
curbuf->b_p_ul = save_b_p_ul;
curwin->w_p_cul = save_w_p_cul; 
curwin->w_p_cuc = save_w_p_cuc; 
eap->arg = save_eap;
restore_search_patterns();
win_size_restore(&save_view);
ga_clear(&save_view);
unblock_autocmds();
}






char_u *skip_vimgrep_pat(char_u *p, char_u **s, int *flags)
{
int c;

if (vim_isIDc(*p)) {

if (s != NULL) {
*s = p;
}
p = skiptowhite(p);
if (s != NULL && *p != NUL) {
*p++ = NUL;
}
} else {

if (s != NULL) {
*s = p + 1;
}
c = *p;
p = skip_regexp(p + 1, c, true, NULL);
if (*p != c) {
return NULL;
}


if (s != NULL) {
*p = NUL;
}
p++;


while (*p == 'g' || *p == 'j') {
if (flags != NULL) {
if (*p == 'g') {
*flags |= VGR_GLOBAL;
} else {
*flags |= VGR_NOJUMP;
}
}
p++;
}
}
return p;
}


void ex_oldfiles(exarg_T *eap)
{
list_T *l = get_vim_var_list(VV_OLDFILES);
long nr = 0;

if (l == NULL) {
msg((char_u *)_("No old files"));
} else {
msg_start();
msg_scroll = true;
TV_LIST_ITER(l, li, {
if (got_int) {
break;
}
nr++;
const char *fname = tv_get_string(TV_LIST_ITEM_TV(li));
if (!message_filtered((char_u *)fname)) {
msg_outnum(nr);
MSG_PUTS(": ");
msg_outtrans((char_u *)tv_get_string(TV_LIST_ITEM_TV(li)));
msg_clr_eos();
msg_putchar('\n');
ui_flush(); 
os_breakcheck();
}
});


got_int = false;


if (cmdmod.browse) {
quit_more = false;
nr = prompt_for_number(false);
msg_starthere();
if (nr > 0 && nr <= tv_list_len(l)) {
const char *const p = tv_list_find_str(l, nr - 1);
if (p == NULL) {
return;
}
char *const s = (char *)expand_env_save((char_u *)p);
eap->arg = (char_u *)s;
eap->cmdidx = CMD_edit;
cmdmod.browse = false;
do_exedit(eap, NULL);
xfree(s);
}
}
}
}
