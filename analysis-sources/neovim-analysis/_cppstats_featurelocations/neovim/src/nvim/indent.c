


#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>

#include "nvim/ascii.h"
#include "nvim/assert.h"
#include "nvim/change.h"
#include "nvim/indent.h"
#include "nvim/eval.h"
#include "nvim/charset.h"
#include "nvim/cursor.h"
#include "nvim/mark.h"
#include "nvim/extmark.h"
#include "nvim/memline.h"
#include "nvim/memory.h"
#include "nvim/misc1.h"
#include "nvim/move.h"
#include "nvim/option.h"
#include "nvim/regexp.h"
#include "nvim/screen.h"
#include "nvim/search.h"
#include "nvim/strings.h"
#include "nvim/undo.h"
#include "nvim/buffer.h"


#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "indent.c.generated.h"
#endif


int get_indent(void)
{
return get_indent_str(get_cursor_line_ptr(), (int)curbuf->b_p_ts, false);
}



int get_indent_lnum(linenr_T lnum)
{
return get_indent_str(ml_get(lnum), (int)curbuf->b_p_ts, false);
}




int get_indent_buf(buf_T *buf, linenr_T lnum)
{
return get_indent_str(ml_get_buf(buf, lnum, false), (int)buf->b_p_ts, false);
}





int get_indent_str(char_u *ptr, int ts, int list)
{
int count = 0;

for (; *ptr; ++ptr) {

if (*ptr == TAB) {
if (!list || curwin->w_p_lcs_chars.tab1) {

count += ts - (count % ts);
} else {


count += ptr2cells(ptr);
}
} else if (*ptr == ' ') {

count++;
} else {
break;
}
}
return count;
}












int set_indent(int size, int flags)
{
char_u *p;
char_u *newline;
char_u *oldline;
char_u *s;
int todo;
int ind_len; 
int line_len;
int doit = false;
int ind_done = 0; 
int tab_pad;
int retval = false;


int orig_char_len = -1;



todo = size;
ind_len = 0;
p = oldline = get_cursor_line_ptr();






if (!curbuf->b_p_et || (!(flags & SIN_INSERT) && curbuf->b_p_pi)) {


if (!(flags & SIN_INSERT) && curbuf->b_p_pi) {
ind_done = 0;


while (todo > 0 && ascii_iswhite(*p)) {
if (*p == TAB) {
tab_pad = (int)curbuf->b_p_ts - (ind_done % (int)curbuf->b_p_ts);


if (todo < tab_pad) {
break;
}
todo -= tab_pad;
ind_len++;
ind_done += tab_pad;
} else {
todo--;
ind_len++;
ind_done++;
}
p++;
}



if (curbuf->b_p_et) {
orig_char_len = ind_len;
}


tab_pad = (int)curbuf->b_p_ts - (ind_done % (int)curbuf->b_p_ts);

if ((todo >= tab_pad) && (orig_char_len == -1)) {
doit = true;
todo -= tab_pad;
ind_len++;


}
}


while (todo >= (int)curbuf->b_p_ts) {
if (*p != TAB) {
doit = true;
} else {
p++;
}
todo -= (int)curbuf->b_p_ts;
ind_len++;


}
}


while (todo > 0) {
if (*p != ' ') {
doit = true;
} else {
p++;
}
todo--;
ind_len++;


}


if (!doit && !ascii_iswhite(*p) && !(flags & SIN_INSERT)) {
return false;
}


if (flags & SIN_INSERT) {
p = oldline;
} else {
p = skipwhite(p);
}
line_len = (int)STRLEN(p) + 1;




int skipcols = 0; 
if (orig_char_len != -1) {
int newline_size; 
STRICT_ADD(orig_char_len, size, &newline_size, int);
STRICT_SUB(newline_size, ind_done, &newline_size, int);
STRICT_ADD(newline_size, line_len, &newline_size, int);
assert(newline_size >= 0);
newline = xmalloc((size_t)newline_size);
todo = size - ind_done;



ind_len = orig_char_len + todo;
p = oldline;
s = newline;
skipcols = orig_char_len;

while (orig_char_len > 0) {
*s++ = *p++;
orig_char_len--;
}



while (ascii_iswhite(*p)) {
p++;
}
} else {
todo = size;
assert(ind_len + line_len >= 0);
size_t newline_size;
STRICT_ADD(ind_len, line_len, &newline_size, size_t);
newline = xmalloc(newline_size);
s = newline;
}



if (!curbuf->b_p_et) {


if (!(flags & SIN_INSERT) && curbuf->b_p_pi) {
p = oldline;
ind_done = 0;

while (todo > 0 && ascii_iswhite(*p)) {
if (*p == TAB) {
tab_pad = (int)curbuf->b_p_ts - (ind_done % (int)curbuf->b_p_ts);


if (todo < tab_pad) {
break;
}
todo -= tab_pad;
ind_done += tab_pad;
} else {
todo--;
ind_done++;
}
*s++ = *p++;
skipcols++;
}


tab_pad = (int)curbuf->b_p_ts - (ind_done % (int)curbuf->b_p_ts);

if (todo >= tab_pad) {
*s++ = TAB;
todo -= tab_pad;
}
p = skipwhite(p);
}

while (todo >= (int)curbuf->b_p_ts) {
*s++ = TAB;
todo -= (int)curbuf->b_p_ts;
}
}

while (todo > 0) {
*s++ = ' ';
todo--;
}
memmove(s, p, (size_t)line_len);


if (!(flags & SIN_UNDO) || (u_savesub(curwin->w_cursor.lnum) == OK)) {
ml_replace(curwin->w_cursor.lnum, newline, false);
if (!(flags & SIN_NOMARK)) {
extmark_splice(curbuf,
(int)curwin->w_cursor.lnum-1, skipcols,
0, (int)(p-oldline) - skipcols,
0, (int)(s-newline) - skipcols,
kExtmarkUndo);
}

if (flags & SIN_CHANGED) {
changed_bytes(curwin->w_cursor.lnum, 0);
}


if (saved_cursor.lnum == curwin->w_cursor.lnum) {
if (saved_cursor.col >= (colnr_T)(p - oldline)) {


saved_cursor.col += ind_len - (colnr_T)(p - oldline);

} else if (saved_cursor.col >= (colnr_T)(s - newline)) {


saved_cursor.col = (colnr_T)(s - newline);
}
}
retval = true;
} else {
xfree(newline);
}
curwin->w_cursor.col = ind_len;
return retval;
}





int get_number_indent(linenr_T lnum)
{
colnr_T col;
pos_T pos;
regmatch_T regmatch;
int lead_len = 0; 

if (lnum > curbuf->b_ml.ml_line_count) {
return -1;
}
pos.lnum = 0;


if ((State & INSERT) || has_format_option(FO_Q_COMS)) {
lead_len = get_leader_len(ml_get(lnum), NULL, false, true);
}
regmatch.regprog = vim_regcomp(curbuf->b_p_flp, RE_MAGIC);

if (regmatch.regprog != NULL) {
regmatch.rm_ic = false;



if (vim_regexec(&regmatch, ml_get(lnum) + lead_len, (colnr_T)0)) {
pos.lnum = lnum;
pos.col = (colnr_T)(*regmatch.endp - ml_get(lnum));
pos.coladd = 0;
}
vim_regfree(regmatch.regprog);
}

if ((pos.lnum == 0) || (*ml_get_pos(&pos) == NUL)) {
return -1;
}
getvcol(curwin, &pos, &col, NULL, NULL);
return (int)col;
}






int get_breakindent_win(win_T *wp, char_u *line)
FUNC_ATTR_NONNULL_ARG(1)
{
static int prev_indent = 0; 
static long prev_ts = 0; 
static char_u *prev_line = NULL; 
static varnumber_T prev_tick = 0; 
int bri = 0;

const int eff_wwidth = wp->w_width_inner
- ((wp->w_p_nu || wp->w_p_rnu)
&& (vim_strchr(p_cpo, CPO_NUMCOL) == NULL)
? number_width(wp) + 1 : 0);


if (prev_line != line || prev_ts != wp->w_buffer->b_p_ts
|| prev_tick != buf_get_changedtick(wp->w_buffer)) {
prev_line = line;
prev_ts = wp->w_buffer->b_p_ts;
prev_tick = buf_get_changedtick(wp->w_buffer);
prev_indent = get_indent_str(line, (int)wp->w_buffer->b_p_ts, wp->w_p_list);
}
bri = prev_indent + wp->w_p_brishift;


if (wp->w_p_brisbr)
bri -= vim_strsize(p_sbr);


bri += win_col_off2(wp);


if (bri < 0)
bri = 0;


else if (bri > eff_wwidth - wp->w_p_brimin)
bri = (eff_wwidth - wp->w_p_brimin < 0)
? 0 : eff_wwidth - wp->w_p_brimin;

return bri;
}





int inindent(int extra)
{
char_u *ptr;
colnr_T col;

for (col = 0, ptr = get_cursor_line_ptr(); ascii_iswhite(*ptr); ++col) {
ptr++;
}

if (col >= curwin->w_cursor.col + extra) {
return true;
} else {
return false;
}
}



int get_expr_indent(void)
{
int indent = -1;
pos_T save_pos;
colnr_T save_curswant;
int save_set_curswant;
int save_State;
int use_sandbox = was_set_insecurely((char_u *)"indentexpr", OPT_LOCAL);



save_pos = curwin->w_cursor;
save_curswant = curwin->w_curswant;
save_set_curswant = curwin->w_set_curswant;
set_vim_var_nr(VV_LNUM, (varnumber_T) curwin->w_cursor.lnum);

if (use_sandbox) {
sandbox++;
}
textlock++;



char_u *inde_copy = vim_strsave(curbuf->b_p_inde);
indent = (int)eval_to_number(inde_copy);
xfree(inde_copy);

if (use_sandbox) {
sandbox--;
}
textlock--;




save_State = State;
State = INSERT;
curwin->w_cursor = save_pos;
curwin->w_curswant = save_curswant;
curwin->w_set_curswant = save_set_curswant;
check_cursor();
State = save_State;


if (indent < 0) {
indent = get_indent();
}

return indent;
}
















int get_lisp_indent(void)
{
pos_T *pos, realpos, paren;
int amount;
char_u *that;
colnr_T col;
colnr_T firsttry;
int parencount;
int quotecount;
int vi_lisp;


vi_lisp = (vim_strchr(p_cpo, CPO_LISP) != NULL);

realpos = curwin->w_cursor;
curwin->w_cursor.col = 0;

if ((pos = findmatch(NULL, '(')) == NULL) {
pos = findmatch(NULL, '[');
} else {
paren = *pos;
pos = findmatch(NULL, '[');

if ((pos == NULL) || lt(*pos, paren)) {
pos = &paren;
}
}

if (pos != NULL) {


amount = -1;
parencount = 0;

while (--curwin->w_cursor.lnum >= pos->lnum) {
if (linewhite(curwin->w_cursor.lnum)) {
continue;
}

for (that = get_cursor_line_ptr(); *that != NUL; ++that) {
if (*that == ';') {
while (*(that + 1) != NUL) {
that++;
}
continue;
}

if (*that == '\\') {
if (*(that + 1) != NUL) {
that++;
}
continue;
}

if ((*that == '"') && (*(that + 1) != NUL)) {
while (*++that && *that != '"') {

if (*that == '\\') {
if (*++that == NUL) {
break;
}
if (that[1] == NUL) {
that++;
break;
}
}
}
}
if ((*that == '(') || (*that == '[')) {
parencount++;
} else if ((*that == ')') || (*that == ']')) {
parencount--;
}
}

if (parencount == 0) {
amount = get_indent();
break;
}
}

if (amount == -1) {
curwin->w_cursor.lnum = pos->lnum;
curwin->w_cursor.col = pos->col;
col = pos->col;

that = get_cursor_line_ptr();

if (vi_lisp && (get_indent() == 0)) {
amount = 2;
} else {
char_u *line = that;

amount = 0;

while (*that && col) {
amount += lbr_chartabsize_adv(line, &that, (colnr_T)amount);
col--;
}





if (!vi_lisp && ((*that == '(') || (*that == '['))
&& lisp_match(that + 1)) {
amount += 2;
} else {
that++;
amount++;
firsttry = amount;

while (ascii_iswhite(*that)) {
amount += lbr_chartabsize(line, that, (colnr_T)amount);
that++;
}

if (*that && (*that != ';')) {



if (!vi_lisp && (*that != '(') && (*that != '[')) {
firsttry++;
}

parencount = 0;
quotecount = 0;

if (vi_lisp || ((*that != '"') && (*that != '\'')
&& (*that != '#') && ((*that < '0') || (*that > '9')))) {
while (*that
&& (!ascii_iswhite(*that) || quotecount || parencount)
&& (!((*that == '(' || *that == '[')
&& !quotecount && !parencount && vi_lisp))) {
if (*that == '"') {
quotecount = !quotecount;
}
if (((*that == '(') || (*that == '[')) && !quotecount) {
parencount++;
}
if (((*that == ')') || (*that == ']')) && !quotecount) {
parencount--;
}
if ((*that == '\\') && (*(that + 1) != NUL)) {
amount += lbr_chartabsize_adv(line, &that, (colnr_T)amount);
}

amount += lbr_chartabsize_adv(line, &that, (colnr_T)amount);
}
}

while (ascii_iswhite(*that)) {
amount += lbr_chartabsize(line, that, (colnr_T)amount);
that++;
}

if (!*that || (*that == ';')) {
amount = firsttry;
}
}
}
}
}
} else {
amount = 0; 
}
curwin->w_cursor = realpos;

return amount;
}


static int lisp_match(char_u *p)
{
char_u buf[LSIZE];
int len;
char_u *word = *curbuf->b_p_lw != NUL ? curbuf->b_p_lw : p_lispwords;

while (*word != NUL) {
(void)copy_option_part(&word, buf, LSIZE, ",");
len = (int)STRLEN(buf);

if ((STRNCMP(buf, p, len) == 0) && (p[len] == ' ')) {
return true;
}
}
return false;
}
