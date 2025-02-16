


#include <stdbool.h>

#include "nvim/mouse.h"
#include "nvim/vim.h"
#include "nvim/ascii.h"
#include "nvim/window.h"
#include "nvim/state.h"
#include "nvim/strings.h"
#include "nvim/screen.h"
#include "nvim/syntax.h"
#include "nvim/ui.h"
#include "nvim/ui_compositor.h"
#include "nvim/os_unix.h"
#include "nvim/fold.h"
#include "nvim/diff.h"
#include "nvim/move.h"
#include "nvim/misc1.h"
#include "nvim/cursor.h"
#include "nvim/buffer_defs.h"
#include "nvim/memline.h"
#include "nvim/charset.h"

#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "mouse.c.generated.h"
#endif

static linenr_T orig_topline = 0;
static int orig_topfill = 0;

























int jump_to_mouse(int flags,
bool *inclusive, 
int which_button) 
{
static int on_status_line = 0; 
static int on_sep_line = 0; 
static int prev_row = -1;
static int prev_col = -1;
static win_T *dragwin = NULL; 
static int did_drag = false; 

win_T *wp, *old_curwin;
pos_T old_cursor;
int count;
bool first;
int row = mouse_row;
int col = mouse_col;
int grid = mouse_grid;
int mouse_char;
int fdc = 0;

mouse_past_bottom = false;
mouse_past_eol = false;

if (flags & MOUSE_RELEASED) {


if (dragwin != NULL && !did_drag)
flags &= ~(MOUSE_FOCUS | MOUSE_DID_MOVE);
dragwin = NULL;
did_drag = false;
}

if ((flags & MOUSE_DID_MOVE)
&& prev_row == mouse_row
&& prev_col == mouse_col) {
retnomove:


if (on_status_line)
return IN_STATUS_LINE;
if (on_sep_line)
return IN_SEP_LINE;
if (flags & MOUSE_MAY_STOP_VIS) {
end_visual_mode();
redraw_curbuf_later(INVERTED); 
}
return IN_BUFFER;
}

prev_row = mouse_row;
prev_col = mouse_col;

if (flags & MOUSE_SETPOS)
goto retnomove; 



if (row >= 0 && row < Rows && col >= 0 && col <= Columns
&& default_grid.chars != NULL) {
mouse_char = utf_ptr2char(default_grid.chars[default_grid.line_offset[row]
+ (unsigned)col]);
} else {
mouse_char = ' ';
}

old_curwin = curwin;
old_cursor = curwin->w_cursor;

if (!(flags & MOUSE_FOCUS)) {
if (row < 0 || col < 0) 
return IN_UNKNOWN;


wp = mouse_find_win(&grid, &row, &col);
if (wp == NULL) {
return IN_UNKNOWN;
}
fdc = win_fdccol_count(wp);
dragwin = NULL;

if (grid == DEFAULT_GRID_HANDLE && row >= wp->w_height) {

on_status_line = row - wp->w_height + 1;
dragwin = wp;
} else {
on_status_line = 0;
}

if (grid == DEFAULT_GRID_HANDLE && col >= wp->w_width) {

on_sep_line = col - wp->w_width + 1;
dragwin = wp;
} else {
on_sep_line = 0;
}



if (on_status_line && on_sep_line) {
if (stl_connected(wp))
on_sep_line = 0;
else
on_status_line = 0;
}



if (VIsual_active
&& (wp->w_buffer != curwin->w_buffer
|| (!on_status_line
&& !on_sep_line
&& (wp->w_p_rl
? col < wp->w_width_inner - fdc
: col >= fdc + (cmdwin_type == 0 && wp == curwin ? 0 : 1))
&& (flags & MOUSE_MAY_STOP_VIS)))) {
end_visual_mode();
redraw_curbuf_later(INVERTED); 
}
if (cmdwin_type != 0 && wp != curwin) {


on_sep_line = 0;
row = 0;
col += wp->w_wincol;
wp = curwin;
}



if (dragwin == NULL || (flags & MOUSE_RELEASED))
win_enter(wp, true); 

if (curwin != old_curwin)
set_mouse_topline(curwin);
if (on_status_line) { 

if (curwin == old_curwin)
return IN_STATUS_LINE;
else
return IN_STATUS_LINE | CURSOR_MOVED;
}
if (on_sep_line) { 

if (curwin == old_curwin)
return IN_SEP_LINE;
else
return IN_SEP_LINE | CURSOR_MOVED;
}

curwin->w_cursor.lnum = curwin->w_topline;
} else if (on_status_line && which_button == MOUSE_LEFT) {
if (dragwin != NULL) {

count = row - dragwin->w_winrow - dragwin->w_height + 1
- on_status_line;
win_drag_status_line(dragwin, count);
did_drag |= count;
}
return IN_STATUS_LINE; 
} else if (on_sep_line && which_button == MOUSE_LEFT) {
if (dragwin != NULL) {

count = col - dragwin->w_wincol - dragwin->w_width + 1
- on_sep_line;
win_drag_vsep_line(dragwin, count);
did_drag |= count;
}
return IN_SEP_LINE; 
} else {


if (flags & MOUSE_MAY_STOP_VIS) {
end_visual_mode();
redraw_curbuf_later(INVERTED); 
}


row -= curwin->w_winrow;
col -= curwin->w_wincol;



if (row < 0) {
count = 0;
for (first = true; curwin->w_topline > 1; ) {
if (curwin->w_topfill < diff_check(curwin, curwin->w_topline))
++count;
else
count += plines(curwin->w_topline - 1);
if (!first && count > -row)
break;
first = false;
(void)hasFolding(curwin->w_topline, &curwin->w_topline, NULL);
if (curwin->w_topfill < diff_check(curwin, curwin->w_topline)) {
++curwin->w_topfill;
} else {
--curwin->w_topline;
curwin->w_topfill = 0;
}
}
check_topfill(curwin, false);
curwin->w_valid &=
~(VALID_WROW|VALID_CROW|VALID_BOTLINE|VALID_BOTLINE_AP);
redraw_later(VALID);
row = 0;
} else if (row >= curwin->w_height_inner) {
count = 0;
for (first = true; curwin->w_topline < curbuf->b_ml.ml_line_count; ) {
if (curwin->w_topfill > 0) {
++count;
} else {
count += plines(curwin->w_topline);
}

if (!first && count > row - curwin->w_height_inner + 1) {
break;
}
first = false;

if (hasFolding(curwin->w_topline, NULL, &curwin->w_topline)
&& curwin->w_topline == curbuf->b_ml.ml_line_count) {
break;
}

if (curwin->w_topfill > 0) {
--curwin->w_topfill;
} else {
++curwin->w_topline;
curwin->w_topfill =
diff_check_fill(curwin, curwin->w_topline);
}
}
check_topfill(curwin, false);
redraw_later(VALID);
curwin->w_valid &=
~(VALID_WROW|VALID_CROW|VALID_BOTLINE|VALID_BOTLINE_AP);
row = curwin->w_height_inner - 1;
} else if (row == 0) {



if (mouse_dragging > 0
&& curwin->w_cursor.lnum
== curwin->w_buffer->b_ml.ml_line_count
&& curwin->w_cursor.lnum == curwin->w_topline) {
curwin->w_valid &= ~(VALID_TOPLINE);
}
}
}


if (curwin->w_p_rl ? col < curwin->w_width_inner - fdc :
col >= fdc + (cmdwin_type == 0 ? 0 : 1)) {
mouse_char = ' ';
}


if (mouse_comp_pos(curwin, &row, &col, &curwin->w_cursor.lnum)) {
mouse_past_bottom = true;
}

if (!(flags & MOUSE_RELEASED) && which_button == MOUSE_LEFT) {
col = mouse_adjust_click(curwin, row, col);
}


if ((flags & MOUSE_MAY_VIS) && !VIsual_active) {
VIsual = old_cursor;
VIsual_active = true;
VIsual_reselect = true;

may_start_select('o');
setmouse();

if (p_smd && msg_silent == 0) {
redraw_cmdline = true; 
}
}

curwin->w_curswant = col;
curwin->w_set_curswant = false; 
if (coladvance(col) == FAIL) { 
if (inclusive != NULL) {
*inclusive = true;
}
mouse_past_eol = true;
} else if (inclusive != NULL) {
*inclusive = false;
}

count = IN_BUFFER;
if (curwin != old_curwin || curwin->w_cursor.lnum != old_cursor.lnum
|| curwin->w_cursor.col != old_cursor.col) {
count |= CURSOR_MOVED; 
}

if (mouse_char == curwin->w_p_fcs_chars.foldclosed) {
count |= MOUSE_FOLD_OPEN;
} else if (mouse_char != ' ') {
count |= MOUSE_FOLD_CLOSE;
}

return count;
}




bool mouse_comp_pos(win_T *win, int *rowp, int *colp, linenr_T *lnump)
{
int col = *colp;
int row = *rowp;
linenr_T lnum;
bool retval = false;
int off;
int count;

if (win->w_p_rl) {
col = win->w_width_inner - 1 - col;
}

lnum = win->w_topline;

while (row > 0) {

if (win->w_p_diff
&& !hasFoldingWin(win, lnum, NULL, NULL, true, NULL)) {
if (lnum == win->w_topline) {
row -= win->w_topfill;
} else {
row -= diff_check_fill(win, lnum);
}
count = plines_win_nofill(win, lnum, true);
} else {
count = plines_win(win, lnum, true);
}

if (count > row) {
break; 
}

(void)hasFoldingWin(win, lnum, NULL, &lnum, true, NULL);

if (lnum == win->w_buffer->b_ml.ml_line_count) {
retval = true;
break; 
}
row -= count;
++lnum;
}

if (!retval) {

off = win_col_off(win) - win_col_off2(win);
if (col < off)
col = off;
col += row * (win->w_width_inner - off);

col += win->w_skipcol;
}

if (!win->w_p_wrap) {
col += win->w_leftcol;
}


col -= win_col_off(win);
if (col < 0) {
col = 0;
}

*colp = col;
*rowp = row;
*lnump = lnum;
return retval;
}





win_T *mouse_find_win(int *gridp, int *rowp, int *colp)
{
win_T *wp_grid = mouse_find_grid_win(gridp, rowp, colp);
if (wp_grid) {
return wp_grid;
} else if (*gridp > 1) {
return NULL;
}


frame_T *fp;

fp = topframe;
*rowp -= firstwin->w_winrow;
for (;; ) {
if (fp->fr_layout == FR_LEAF)
break;
if (fp->fr_layout == FR_ROW) {
for (fp = fp->fr_child; fp->fr_next != NULL; fp = fp->fr_next) {
if (*colp < fp->fr_width)
break;
*colp -= fp->fr_width;
}
} else { 
for (fp = fp->fr_child; fp->fr_next != NULL; fp = fp->fr_next) {
if (*rowp < fp->fr_height)
break;
*rowp -= fp->fr_height;
}
}
}


FOR_ALL_WINDOWS_IN_TAB(wp, curtab) {
if (wp == fp->fr_win) {
return wp;
}
}
return NULL;
}

static win_T *mouse_find_grid_win(int *gridp, int *rowp, int *colp)
{
if (*gridp == msg_grid.handle) {

*gridp = DEFAULT_GRID_HANDLE;
} else if (*gridp > 1) {
win_T *wp = get_win_by_grid_handle(*gridp);
if (wp && wp->w_grid.chars
&& !(wp->w_floating && !wp->w_float_config.focusable)) {
*rowp = MIN(*rowp, wp->w_grid.Rows-1);
*colp = MIN(*colp, wp->w_grid.Columns-1);
return wp;
}
} else if (*gridp == 0) {
ScreenGrid *grid = ui_comp_mouse_focus(*rowp, *colp);
FOR_ALL_WINDOWS_IN_TAB(wp, curtab) {
if (&wp->w_grid != grid) {
continue;
}
*gridp = grid->handle;
*rowp -= grid->comp_row;
*colp -= grid->comp_col;
return wp;
}



*gridp = DEFAULT_GRID_HANDLE;
}
return NULL;
}




void setmouse(void)
{
ui_cursor_shape();


if (*p_mouse == NUL) {
return;
}

int checkfor = MOUSE_NORMAL; 
if (VIsual_active) {
checkfor = MOUSE_VISUAL;
} else if (State == HITRETURN || State == ASKMORE || State == SETWSIZE) {
checkfor = MOUSE_RETURN;
} else if (State & INSERT) {
checkfor = MOUSE_INSERT;
} else if (State & CMDLINE) {
checkfor = MOUSE_COMMAND;
} else if (State == CONFIRM || State == EXTERNCMD) {
checkfor = ' '; 
}

if (mouse_has(checkfor)) {
ui_call_mouse_on();
} else {
ui_call_mouse_off();
}
}








int mouse_has(int c)
{
for (char_u *p = p_mouse; *p; ++p)
switch (*p) {
case 'a': if (vim_strchr((char_u *)MOUSE_A, c) != NULL)
return true;
break;
case MOUSE_HELP: if (c != MOUSE_RETURN && curbuf->b_help)
return true;
break;
default: if (c == *p) return true; break;
}
return false;
}



void set_mouse_topline(win_T *wp)
{
orig_topline = wp->w_topline;
orig_topfill = wp->w_topfill;
}




static colnr_T scroll_line_len(linenr_T lnum)
{
colnr_T col = 0;
char_u *line = ml_get(lnum);
if (*line != NUL) {
for (;;) {
int numchar = chartabsize(line, col);
MB_PTR_ADV(line);
if (*line == NUL) { 
break;
}
col += numchar;
}
}
return col;
}




static linenr_T find_longest_lnum(void)
{
linenr_T ret = 0;




if (curwin->w_topline <= curwin->w_cursor.lnum
&& curwin->w_botline > curwin->w_cursor.lnum
&& curwin->w_botline <= curbuf->b_ml.ml_line_count + 1) {
long max = 0;




for (linenr_T lnum = curwin->w_topline; lnum < curwin->w_botline; lnum++) {
colnr_T len = scroll_line_len(lnum);
if (len > (colnr_T)max) {
max = len;
ret = lnum;
} else if (len == (colnr_T)max
&& abs((int)(lnum - curwin->w_cursor.lnum))
< abs((int)(ret - curwin->w_cursor.lnum))) {
ret = lnum;
}
}
} else {

ret = curwin->w_cursor.lnum;
}

return ret;
}




bool mouse_scroll_horiz(int dir)
{
if (curwin->w_p_wrap) {
return false;
}

int step = 6;
if (mod_mask & (MOD_MASK_SHIFT | MOD_MASK_CTRL)) {
step = curwin->w_width_inner;
}

int leftcol = curwin->w_leftcol + (dir == MSCR_RIGHT ? -step : +step);
if (leftcol < 0) {
leftcol = 0;
}

if (curwin->w_leftcol == leftcol) {
return false;
}

curwin->w_leftcol = (colnr_T)leftcol;



if (!virtual_active()
&& (colnr_T)leftcol > scroll_line_len(curwin->w_cursor.lnum)) {
curwin->w_cursor.lnum = find_longest_lnum();
curwin->w_cursor.col = 0;
}

return leftcol_changed();
}


static int mouse_adjust_click(win_T *wp, int row, int col)
{
if (!(wp->w_p_cole > 0 && curbuf->b_p_smc > 0
&& wp->w_leftcol < curbuf->b_p_smc && conceal_cursor_line(wp))) {
return col;
}











linenr_T lnum = wp->w_cursor.lnum;
char_u *line = ml_get(lnum);
char_u *ptr = line;
char_u *ptr_end;
char_u *ptr_row_offset = line; 


int offset = wp->w_leftcol;
if (row > 0) {
offset += row * (wp->w_width_inner - win_col_off(wp) - win_col_off2(wp) -
wp->w_leftcol + wp->w_skipcol);
}

int vcol;

if (offset) {




vcol = 0;
while (vcol < offset && *ptr != NUL) {
vcol += chartabsize(ptr, vcol);
ptr += utfc_ptr2len(ptr);
}

ptr_row_offset = ptr;
}


vcol = offset;
ptr_end = ptr_row_offset;
while (vcol < col && *ptr_end != NUL) {
vcol += chartabsize(ptr_end, vcol);
ptr_end += utfc_ptr2len(ptr_end);
}

int matchid;
int prev_matchid;
int nudge = 0;
int cwidth = 0;

vcol = offset;

#define incr() nudge++; ptr_end += utfc_ptr2len(ptr_end)
#define decr() nudge--; ptr_end -= utfc_ptr2len(ptr_end)

while (ptr < ptr_end && *ptr != NUL) {
cwidth = chartabsize(ptr, vcol);
vcol += cwidth;
if (cwidth > 1 && *ptr == '\t' && nudge > 0) {

cwidth = MIN(cwidth, nudge);
while (cwidth > 0) {
decr();
cwidth--;
}
}

matchid = syn_get_concealed_id(wp, lnum, (colnr_T)(ptr - line));
if (matchid != 0) {
if (wp->w_p_cole == 3) {
incr();
} else {
if (!(row > 0 && ptr == ptr_row_offset)
&& (wp->w_p_cole == 1 || (wp->w_p_cole == 2
&& (wp->w_p_lcs_chars.conceal != NUL
|| syn_get_sub_char() != NUL)))) {

decr();
}

prev_matchid = matchid;

while (prev_matchid == matchid && *ptr != NUL) {
incr();
ptr += utfc_ptr2len(ptr);
matchid = syn_get_concealed_id(wp, lnum, (colnr_T)(ptr - line));
}

continue;
}
}

ptr += utfc_ptr2len(ptr);
}

return col + nudge;
}
