


#include <stdbool.h>
#include <inttypes.h>

#include "nvim/assert.h"
#include "nvim/change.h"
#include "nvim/cursor.h"
#include "nvim/charset.h"
#include "nvim/fold.h"
#include "nvim/memline.h"
#include "nvim/memory.h"
#include "nvim/misc1.h"
#include "nvim/move.h"
#include "nvim/screen.h"
#include "nvim/state.h"
#include "nvim/vim.h"
#include "nvim/ascii.h"
#include "nvim/mark.h"

#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "cursor.c.generated.h"
#endif




int getviscol(void)
{
colnr_T x;

getvvcol(curwin, &curwin->w_cursor, &x, NULL, NULL);
return (int)x;
}




int getviscol2(colnr_T col, colnr_T coladd)
{
colnr_T x;
pos_T pos;

pos.lnum = curwin->w_cursor.lnum;
pos.col = col;
pos.coladd = coladd;
getvvcol(curwin, &pos, &x, NULL, NULL);
return (int)x;
}






int coladvance_force(colnr_T wcol)
{
int rc = coladvance2(&curwin->w_cursor, true, false, wcol);

if (wcol == MAXCOL) {
curwin->w_valid &= ~VALID_VIRTCOL;
} else {

curwin->w_valid |= VALID_VIRTCOL;
curwin->w_virtcol = wcol;
}
return rc;
}










int coladvance(colnr_T wcol)
{
int rc = getvpos(&curwin->w_cursor, wcol);

if (wcol == MAXCOL || rc == FAIL)
curwin->w_valid &= ~VALID_VIRTCOL;
else if (*get_cursor_pos_ptr() != TAB) {

curwin->w_valid |= VALID_VIRTCOL;
curwin->w_virtcol = wcol;
}
return rc;
}

static int coladvance2(
pos_T *pos,
bool addspaces, 
bool finetune, 
colnr_T wcol_arg 
)
{
colnr_T wcol = wcol_arg;
int idx;
char_u *ptr;
char_u *line;
colnr_T col = 0;
int csize = 0;
int one_more;
int head = 0;

one_more = (State & INSERT)
|| restart_edit != NUL
|| (VIsual_active && *p_sel != 'o')
|| ((ve_flags & VE_ONEMORE) && wcol < MAXCOL);
line = ml_get_buf(curbuf, pos->lnum, false);

if (wcol >= MAXCOL) {
idx = (int)STRLEN(line) - 1 + one_more;
col = wcol;

if ((addspaces || finetune) && !VIsual_active) {
curwin->w_curswant = linetabsize(line) + one_more;
if (curwin->w_curswant > 0)
--curwin->w_curswant;
}
} else {
int width = curwin->w_width_inner - win_col_off(curwin);

if (finetune
&& curwin->w_p_wrap
&& curwin->w_width_inner != 0
&& wcol >= (colnr_T)width) {
csize = linetabsize(line);
if (csize > 0)
csize--;

if (wcol / width > (colnr_T)csize / width
&& ((State & INSERT) == 0 || (int)wcol > csize + 1)) {




wcol = (csize / width + 1) * width - 1;
}
}

ptr = line;
while (col <= wcol && *ptr != NUL) {

csize = win_lbr_chartabsize(curwin, line, ptr, col, &head);
MB_PTR_ADV(ptr);
col += csize;
}
idx = (int)(ptr - line);






if (col > wcol || (!virtual_active() && one_more == 0)) {
idx -= 1;

csize -= head;
col -= csize;
}

if (virtual_active()
&& addspaces
&& wcol >= 0
&& ((col != wcol && col != wcol + 1) || csize > 1)) {



if (line[idx] == NUL) {

int correct = wcol - col;
size_t newline_size;
STRICT_ADD(idx, correct, &newline_size, size_t);
char_u *newline = xmallocz(newline_size);
memcpy(newline, line, (size_t)idx);
memset(newline + idx, ' ', (size_t)correct);

ml_replace(pos->lnum, newline, false);
changed_bytes(pos->lnum, (colnr_T)idx);
idx += correct;
col = wcol;
} else {

int linelen = (int)STRLEN(line);
int correct = wcol - col - csize + 1; 
char_u *newline;

if (-correct > csize)
return FAIL;

size_t n;
STRICT_ADD(linelen - 1, csize, &n, size_t);
newline = xmallocz(n);

memcpy(newline, line, (size_t)idx);

memset(newline + idx, ' ', (size_t)csize);

STRICT_SUB(linelen, idx, &n, size_t);
STRICT_SUB(n, 1, &n, size_t);
memcpy(newline + idx + csize, line + idx + 1, n);

ml_replace(pos->lnum, newline, false);
changed_bytes(pos->lnum, idx);
idx += (csize - 1 + correct);
col += correct;
}
}
}

if (idx < 0)
pos->col = 0;
else
pos->col = idx;

pos->coladd = 0;

if (finetune) {
if (wcol == MAXCOL) {

if (!one_more) {
colnr_T scol, ecol;

getvcol(curwin, pos, &scol, NULL, &ecol);
pos->coladd = ecol - scol;
}
} else {
int b = (int)wcol - (int)col;


if (b > 0 && b < (MAXCOL - 2 * curwin->w_width_inner)) {
pos->coladd = b;
}

col += b;
}
}


if (has_mbyte) {
mark_mb_adjustpos(curbuf, pos);
}

if (wcol < 0 || col < wcol) {
return FAIL;
}
return OK;
}





int getvpos(pos_T *pos, colnr_T wcol)
{
return coladvance2(pos, false, virtual_active(), wcol);
}




int inc_cursor(void)
{
return inc(&curwin->w_cursor);
}







int dec_cursor(void)
{
return dec(&curwin->w_cursor);
}






linenr_T get_cursor_rel_lnum(win_T *wp, linenr_T lnum)
{
linenr_T cursor = wp->w_cursor.lnum;
if (lnum == cursor || !hasAnyFolding(wp)) {
return lnum - cursor;
}

linenr_T from_line = lnum < cursor ? lnum : cursor;
linenr_T to_line = lnum > cursor ? lnum : cursor;
linenr_T retval = 0;


for (; from_line < to_line; from_line++, retval++) {

(void)hasFoldingWin(wp, from_line, NULL, &from_line, true, NULL);
}


if (from_line > to_line) {
retval--;
}

return (lnum < cursor) ? -retval : retval;
}



void check_pos(buf_T *buf, pos_T *pos)
{
char_u *line;
colnr_T len;

if (pos->lnum > buf->b_ml.ml_line_count) {
pos->lnum = buf->b_ml.ml_line_count;
}

if (pos->col > 0) {
line = ml_get_buf(buf, pos->lnum, false);
len = (colnr_T)STRLEN(line);
if (pos->col > len) {
pos->col = len;
}
}
}




void check_cursor_lnum(void)
{
if (curwin->w_cursor.lnum > curbuf->b_ml.ml_line_count) {


if (!hasFolding(curbuf->b_ml.ml_line_count,
&curwin->w_cursor.lnum, NULL))
curwin->w_cursor.lnum = curbuf->b_ml.ml_line_count;
}
if (curwin->w_cursor.lnum <= 0)
curwin->w_cursor.lnum = 1;
}




void check_cursor_col(void)
{
check_cursor_col_win(curwin);
}



void check_cursor_col_win(win_T *win)
{
colnr_T len;
colnr_T oldcol = win->w_cursor.col;
colnr_T oldcoladd = win->w_cursor.col + win->w_cursor.coladd;

len = (colnr_T)STRLEN(ml_get_buf(win->w_buffer, win->w_cursor.lnum, false));
if (len == 0) {
win->w_cursor.col = 0;
} else if (win->w_cursor.col >= len) {




if ((State & INSERT) || restart_edit
|| (VIsual_active && *p_sel != 'o')
|| (ve_flags & VE_ONEMORE)
|| virtual_active()) {
win->w_cursor.col = len;
} else {
win->w_cursor.col = len - 1;

if (has_mbyte) {
mark_mb_adjustpos(win->w_buffer, &win->w_cursor);
}
}
} else if (win->w_cursor.col < 0) {
win->w_cursor.col = 0;
}




if (oldcol == MAXCOL) {
win->w_cursor.coladd = 0;
} else if (ve_flags == VE_ALL) {
if (oldcoladd > win->w_cursor.col) {
win->w_cursor.coladd = oldcoladd - win->w_cursor.col;




if (win->w_cursor.col + 1 < len) {
assert(win->w_cursor.coladd > 0);
int cs, ce;

getvcol(win, &win->w_cursor, &cs, NULL, &ce);
if (win->w_cursor.coladd > ce - cs) {
win->w_cursor.coladd = ce - cs;
}
}
} else {

win->w_cursor.coladd = 0;
}
}
}




void check_cursor(void)
{
check_cursor_lnum();
check_cursor_col();
}





void adjust_cursor_col(void)
{
if (curwin->w_cursor.col > 0
&& (!VIsual_active || *p_sel == 'o')
&& gchar_cursor() == NUL)
--curwin->w_cursor.col;
}





bool leftcol_changed(void)
{


int64_t lastcol;
colnr_T s, e;
bool retval = false;

changed_cline_bef_curs();
lastcol = curwin->w_leftcol + curwin->w_width_inner - curwin_col_off() - 1;
validate_virtcol();





if (curwin->w_virtcol > (colnr_T)(lastcol - p_siso)) {
retval = true;
coladvance((colnr_T)(lastcol - p_siso));
} else if (curwin->w_virtcol < curwin->w_leftcol + p_siso) {
retval = true;
coladvance((colnr_T)(curwin->w_leftcol + p_siso));
}






getvvcol(curwin, &curwin->w_cursor, &s, NULL, &e);
if (e > (colnr_T)lastcol) {
retval = true;
coladvance(s - 1);
} else if (s < curwin->w_leftcol) {
retval = true;
if (coladvance(e + 1) == FAIL) { 
curwin->w_leftcol = s; 
changed_cline_bef_curs();
}
}

if (retval)
curwin->w_set_curswant = true;
redraw_later(NOT_VALID);
return retval;
}

int gchar_cursor(void)
{
return utf_ptr2char(get_cursor_pos_ptr());
}





void pchar_cursor(char_u c)
{
*(ml_get_buf(curbuf, curwin->w_cursor.lnum, true)
+ curwin->w_cursor.col) = c;
}




char_u *get_cursor_line_ptr(void)
{
return ml_get_buf(curbuf, curwin->w_cursor.lnum, false);
}




char_u *get_cursor_pos_ptr(void)
{
return ml_get_buf(curbuf, curwin->w_cursor.lnum, false) +
curwin->w_cursor.col;
}
