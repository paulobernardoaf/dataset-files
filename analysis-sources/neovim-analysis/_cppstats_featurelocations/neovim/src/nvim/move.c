













#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>

#include "nvim/ascii.h"
#include "nvim/move.h"
#include "nvim/charset.h"
#include "nvim/cursor.h"
#include "nvim/diff.h"
#include "nvim/edit.h"
#include "nvim/fold.h"
#include "nvim/mbyte.h"
#include "nvim/memline.h"
#include "nvim/misc1.h"
#include "nvim/option.h"
#include "nvim/popupmnu.h"
#include "nvim/screen.h"
#include "nvim/strings.h"
#include "nvim/window.h"

typedef struct {
linenr_T lnum; 
int fill; 
int height; 
} lineoff_T;

#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "move.c.generated.h"
#endif






static void comp_botline(win_T *wp)
{
linenr_T lnum;
int done;





check_cursor_moved(wp);
if (wp->w_valid & VALID_CROW) {
lnum = wp->w_cursor.lnum;
done = wp->w_cline_row;
} else {
lnum = wp->w_topline;
done = 0;
}

for (; lnum <= wp->w_buffer->b_ml.ml_line_count; lnum++) {
linenr_T last = lnum;
bool folded;
int n = plines_win_full(wp, lnum, &last, &folded, true);
if (lnum <= wp->w_cursor.lnum && last >= wp->w_cursor.lnum) {
wp->w_cline_row = done;
wp->w_cline_height = n;
wp->w_cline_folded = folded;
redraw_for_cursorline(wp);
wp->w_valid |= (VALID_CROW|VALID_CHEIGHT);
}
if (done + n > wp->w_height_inner) {
break;
}
done += n;
lnum = last;
}


wp->w_botline = lnum;
wp->w_valid |= VALID_BOTLINE|VALID_BOTLINE_AP;
wp->w_viewport_invalid = true;

set_empty_rows(wp, done);

win_check_anchored_floats(wp);
}

void reset_cursorline(void)
{
curwin->w_last_cursorline = 0;
}


void redraw_for_cursorline(win_T *wp)
FUNC_ATTR_NONNULL_ALL
{
if ((wp->w_p_rnu || win_cursorline_standout(wp))
&& (wp->w_valid & VALID_CROW) == 0
&& !pum_visible()) {
if (wp->w_p_rnu) {

redraw_win_later(wp, VALID);
}
if (win_cursorline_standout(wp)) {
if (wp->w_redr_type <= VALID && wp->w_last_cursorline != 0) {



redrawWinline(wp, wp->w_last_cursorline);
redrawWinline(wp, wp->w_cursor.lnum);
} else {
redraw_win_later(wp, SOME_VALID);
}
}
}
}





void update_topline_redraw(void)
{
update_topline();
if (must_redraw)
update_screen(0);
}




void update_topline(void)
{
linenr_T old_topline;
int old_topfill;
bool check_topline = false;
bool check_botline = false;
long *so_ptr = curwin->w_p_so >= 0 ? &curwin->w_p_so : &p_so;
long save_so = *so_ptr;



if (!default_grid.chars || curwin->w_height_inner == 0) {
curwin->w_topline = curwin->w_cursor.lnum;
curwin->w_botline = curwin->w_topline;
curwin->w_valid |= VALID_BOTLINE|VALID_BOTLINE_AP;
curwin->w_viewport_invalid = true;
curwin->w_scbind_pos = 1;
return;
}

check_cursor_moved(curwin);
if (curwin->w_valid & VALID_TOPLINE)
return;


if (mouse_dragging > 0) {
*so_ptr = mouse_dragging - 1;
}

old_topline = curwin->w_topline;
old_topfill = curwin->w_topfill;


if (BUFEMPTY()) { 
if (curwin->w_topline != 1) {
redraw_later(NOT_VALID);
}
curwin->w_topline = 1;
curwin->w_botline = 2;
curwin->w_valid |= VALID_BOTLINE|VALID_BOTLINE_AP;
curwin->w_viewport_invalid = true;
curwin->w_scbind_pos = 1;
}




else {
if (curwin->w_topline > 1) {



if (curwin->w_cursor.lnum < curwin->w_topline)
check_topline = true;
else if (check_top_offset())
check_topline = true;
}

if (!check_topline && curwin->w_topfill > diff_check_fill(curwin,
curwin->w_topline))
check_topline = true;

if (check_topline) {
int halfheight = curwin->w_height_inner / 2 - 1;
if (halfheight < 2) {
halfheight = 2;
}
long n;
if (hasAnyFolding(curwin)) {



n = 0;
for (linenr_T lnum = curwin->w_cursor.lnum;
lnum < curwin->w_topline + *so_ptr; lnum++) {
n++;

if (lnum >= curbuf->b_ml.ml_line_count || n >= halfheight) {
break;
}
(void)hasFolding(lnum, NULL, &lnum);
}
} else {
n = curwin->w_topline + *so_ptr - curwin->w_cursor.lnum;
}




if (n >= halfheight) {
scroll_cursor_halfway(false);
} else {
scroll_cursor_top(scrolljump_value(), false);
check_botline = true;
}
} else {

(void)hasFolding(curwin->w_topline, &curwin->w_topline, NULL);
check_botline = true;
}
}









if (check_botline) {
if (!(curwin->w_valid & VALID_BOTLINE_AP))
validate_botline();

if (curwin->w_botline <= curbuf->b_ml.ml_line_count) {
if (curwin->w_cursor.lnum < curwin->w_botline) {
if (((long)curwin->w_cursor.lnum
>= (long)curwin->w_botline - *so_ptr
|| hasAnyFolding(curwin)
)) {
lineoff_T loff;




int n = curwin->w_empty_rows;
loff.lnum = curwin->w_cursor.lnum;

(void)hasFolding(loff.lnum, NULL, &loff.lnum);
loff.fill = 0;
n += curwin->w_filler_rows;
loff.height = 0;
while (loff.lnum < curwin->w_botline
&& (loff.lnum + 1 < curwin->w_botline || loff.fill == 0)
) {
n += loff.height;
if (n >= *so_ptr) {
break;
}
botline_forw(&loff);
}
if (n >= *so_ptr) {

check_botline = false;
}
} else {

check_botline = false;
}
}
if (check_botline) {
long line_count = 0;
if (hasAnyFolding(curwin)) {



for (linenr_T lnum = curwin->w_cursor.lnum;
lnum >= curwin->w_botline - *so_ptr; lnum--) {
line_count++;

if (lnum <= 0 || line_count > curwin->w_height_inner + 1) {
break;
}
(void)hasFolding(lnum, &lnum, NULL);
}
} else
line_count = curwin->w_cursor.lnum - curwin->w_botline
+ 1 + *so_ptr;
if (line_count <= curwin->w_height_inner + 1) {
scroll_cursor_bot(scrolljump_value(), false);
} else {
scroll_cursor_halfway(false);
}
}
}
}
curwin->w_valid |= VALID_TOPLINE;
curwin->w_viewport_invalid = true;
win_check_anchored_floats(curwin);




if (curwin->w_topline != old_topline
|| curwin->w_topfill != old_topfill
) {
dollar_vcol = -1;
if (curwin->w_skipcol != 0) {
curwin->w_skipcol = 0;
redraw_later(NOT_VALID);
} else
redraw_later(VALID);

if (curwin->w_cursor.lnum == curwin->w_topline)
validate_cursor();
}

*so_ptr = save_so;
}




void update_topline_win(win_T* win)
{
win_T *save_curwin;
switch_win(&save_curwin, NULL, win, NULL, true);
update_topline();
restore_win(save_curwin, NULL, true);
}






static int scrolljump_value(void)
{
long result = p_sj >= 0 ? p_sj : (curwin->w_height_inner * -p_sj) / 100;
assert(result <= INT_MAX);
return (int)result;
}





static bool check_top_offset(void)
{
long so = get_scrolloff_value();
if (curwin->w_cursor.lnum < curwin->w_topline + so
|| hasAnyFolding(curwin)
) {
lineoff_T loff;
loff.lnum = curwin->w_cursor.lnum;
loff.fill = 0;
int n = curwin->w_topfill; 

while (n < so) {
topline_back(&loff);

if (loff.lnum < curwin->w_topline
|| (loff.lnum == curwin->w_topline && loff.fill > 0)
) {
break;
}
n += loff.height;
}
if (n < so) {
return true;
}
}
return false;
}

void update_curswant(void)
{
if (curwin->w_set_curswant) {
validate_virtcol();
curwin->w_curswant = curwin->w_virtcol;
curwin->w_set_curswant = false;
}
}




void check_cursor_moved(win_T *wp)
{
if (wp->w_cursor.lnum != wp->w_valid_cursor.lnum) {
wp->w_valid &= ~(VALID_WROW|VALID_WCOL|VALID_VIRTCOL
|VALID_CHEIGHT|VALID_CROW|VALID_TOPLINE);
wp->w_valid_cursor = wp->w_cursor;
wp->w_valid_leftcol = wp->w_leftcol;
wp->w_viewport_invalid = true;
} else if (wp->w_cursor.col != wp->w_valid_cursor.col
|| wp->w_leftcol != wp->w_valid_leftcol
|| wp->w_cursor.coladd != wp->w_valid_cursor.coladd
) {
wp->w_valid &= ~(VALID_WROW|VALID_WCOL|VALID_VIRTCOL);
wp->w_valid_cursor.col = wp->w_cursor.col;
wp->w_valid_leftcol = wp->w_leftcol;
wp->w_valid_cursor.coladd = wp->w_cursor.coladd;
wp->w_viewport_invalid = true;
}
}






void changed_window_setting(void)
{
changed_window_setting_win(curwin);
}

void changed_window_setting_win(win_T *wp)
{
wp->w_lines_valid = 0;
changed_line_abv_curs_win(wp);
wp->w_valid &= ~(VALID_BOTLINE|VALID_BOTLINE_AP|VALID_TOPLINE);
redraw_win_later(wp, NOT_VALID);
}




void set_topline(win_T *wp, linenr_T lnum)
{

(void)hasFoldingWin(wp, lnum, &lnum, NULL, true, NULL);

wp->w_botline += lnum - wp->w_topline;
wp->w_topline = lnum;
wp->w_topline_was_set = true;
wp->w_topfill = 0;
wp->w_valid &= ~(VALID_WROW|VALID_CROW|VALID_BOTLINE|VALID_TOPLINE);

redraw_later(VALID);
}






void changed_cline_bef_curs(void)
{
curwin->w_valid &= ~(VALID_WROW|VALID_WCOL|VALID_VIRTCOL
|VALID_CHEIGHT|VALID_TOPLINE);
}

void changed_cline_bef_curs_win(win_T *wp)
{
wp->w_valid &= ~(VALID_WROW|VALID_WCOL|VALID_VIRTCOL
|VALID_CHEIGHT|VALID_TOPLINE);
}






void changed_line_abv_curs(void)
{
curwin->w_valid &= ~(VALID_WROW|VALID_WCOL|VALID_VIRTCOL|VALID_CROW
|VALID_CHEIGHT|VALID_TOPLINE);
}

void changed_line_abv_curs_win(win_T *wp)
{
wp->w_valid &= ~(VALID_WROW|VALID_WCOL|VALID_VIRTCOL|VALID_CROW
|VALID_CHEIGHT|VALID_TOPLINE);
}




void validate_botline(void)
{
if (!(curwin->w_valid & VALID_BOTLINE))
comp_botline(curwin);
}




void invalidate_botline(void)
{
curwin->w_valid &= ~(VALID_BOTLINE|VALID_BOTLINE_AP);
}

void invalidate_botline_win(win_T *wp)
{
wp->w_valid &= ~(VALID_BOTLINE|VALID_BOTLINE_AP);
}

void approximate_botline_win(win_T *wp)
{
wp->w_valid &= ~VALID_BOTLINE;
}




int cursor_valid(void)
{
check_cursor_moved(curwin);
return (curwin->w_valid & (VALID_WROW|VALID_WCOL)) ==
(VALID_WROW|VALID_WCOL);
}





void validate_cursor(void)
{
check_cursor_moved(curwin);
if ((curwin->w_valid & (VALID_WCOL|VALID_WROW)) != (VALID_WCOL|VALID_WROW))
curs_columns(true);
}





static void curs_rows(win_T *wp)
{

int all_invalid = (!redrawing()
|| wp->w_lines_valid == 0
|| wp->w_lines[0].wl_lnum > wp->w_topline);
int i = 0;
wp->w_cline_row = 0;
for (linenr_T lnum = wp->w_topline; lnum < wp->w_cursor.lnum; ++i) {
bool valid = false;
if (!all_invalid && i < wp->w_lines_valid) {
if (wp->w_lines[i].wl_lnum < lnum || !wp->w_lines[i].wl_valid)
continue; 
if (wp->w_lines[i].wl_lnum == lnum) {


if (!wp->w_buffer->b_mod_set
|| wp->w_lines[i].wl_lastlnum < wp->w_cursor.lnum
|| wp->w_buffer->b_mod_top
> wp->w_lines[i].wl_lastlnum + 1)
valid = true;
} else if (wp->w_lines[i].wl_lnum > lnum) {
--i; 
}
}
if (valid
&& (lnum != wp->w_topline || !wp->w_p_diff)
) {
lnum = wp->w_lines[i].wl_lastlnum + 1;

if (lnum > wp->w_cursor.lnum)
break;
wp->w_cline_row += wp->w_lines[i].wl_size;
} else {
linenr_T last = lnum;
bool folded;
int n = plines_win_full(wp, lnum, &last, &folded, false);
lnum = last + 1;
if (folded && lnum > wp->w_cursor.lnum) {
break;
}
wp->w_cline_row += n;
}
}

check_cursor_moved(wp);
if (!(wp->w_valid & VALID_CHEIGHT)) {
if (all_invalid
|| i == wp->w_lines_valid
|| (i < wp->w_lines_valid
&& (!wp->w_lines[i].wl_valid
|| wp->w_lines[i].wl_lnum != wp->w_cursor.lnum))) {
wp->w_cline_height = plines_win_full(wp, wp->w_cursor.lnum, NULL,
&wp->w_cline_folded, true);
} else if (i > wp->w_lines_valid) {

wp->w_cline_height = 0;
wp->w_cline_folded = hasFoldingWin(wp, wp->w_cursor.lnum, NULL,
NULL, true, NULL);
} else {
wp->w_cline_height = wp->w_lines[i].wl_size;
wp->w_cline_folded = wp->w_lines[i].wl_folded;
}
}

redraw_for_cursorline(curwin);
wp->w_valid |= VALID_CROW|VALID_CHEIGHT;
}




void validate_virtcol(void)
{
validate_virtcol_win(curwin);
}




void validate_virtcol_win(win_T *wp)
{
check_cursor_moved(wp);
if (!(wp->w_valid & VALID_VIRTCOL)) {
getvvcol(wp, &wp->w_cursor, NULL, &(wp->w_virtcol), NULL);
wp->w_valid |= VALID_VIRTCOL;
if (wp->w_p_cuc
&& !pum_visible()
)
redraw_win_later(wp, SOME_VALID);
}
}




static void validate_cheight(void)
{
check_cursor_moved(curwin);
if (!(curwin->w_valid & VALID_CHEIGHT)) {
curwin->w_cline_height = plines_win_full(curwin, curwin->w_cursor.lnum,
NULL, &curwin->w_cline_folded,
true);
curwin->w_valid |= VALID_CHEIGHT;
}
}




void validate_cursor_col(void)
{
validate_virtcol();
if (!(curwin->w_valid & VALID_WCOL)) {
colnr_T col = curwin->w_virtcol;
colnr_T off = curwin_col_off();
col += off;
int width = curwin->w_width_inner - off + curwin_col_off2();


if (curwin->w_p_wrap && col >= (colnr_T)curwin->w_width_inner
&& width > 0) {

col -= ((col - curwin->w_width_inner) / width + 1) * width;
}
if (col > (int)curwin->w_leftcol) {
col -= curwin->w_leftcol;
} else {
col = 0;
}
curwin->w_wcol = col;

curwin->w_valid |= VALID_WCOL;
}
}





int win_col_off(win_T *wp)
{
return ((wp->w_p_nu || wp->w_p_rnu) ? number_width(wp) + 1 : 0)
+ (cmdwin_type == 0 || wp != curwin ? 0 : 1)
+ win_fdccol_count(wp)
+ (win_signcol_count(wp) * win_signcol_width(wp));
}

int curwin_col_off(void)
{
return win_col_off(curwin);
}






int win_col_off2(win_T *wp)
{
if ((wp->w_p_nu || wp->w_p_rnu) && vim_strchr(p_cpo, CPO_NUMCOL) != NULL)
return number_width(wp) + 1;
return 0;
}

int curwin_col_off2(void)
{
return win_col_off2(curwin);
}






void curs_columns(
int may_scroll 
)
{
int n;
int width = 0;
colnr_T startcol;
colnr_T endcol;
colnr_T prev_skipcol;
long so = get_scrolloff_value();
long siso = get_sidescrolloff_value();




update_topline();


if (!(curwin->w_valid & VALID_CROW)) {
curs_rows(curwin);
}




if (curwin->w_cline_folded)

startcol = curwin->w_virtcol = endcol = curwin->w_leftcol;
else
getvvcol(curwin, &curwin->w_cursor,
&startcol, &(curwin->w_virtcol), &endcol);


if (startcol > dollar_vcol)
dollar_vcol = -1;

int extra = curwin_col_off();
curwin->w_wcol = curwin->w_virtcol + extra;
endcol += extra;




curwin->w_wrow = curwin->w_cline_row;

int textwidth = curwin->w_width_inner - extra;
if (textwidth <= 0) {

curwin->w_wcol = curwin->w_width_inner - 1;
curwin->w_wrow = curwin->w_height_inner - 1;
} else if (curwin->w_p_wrap
&& curwin->w_width_inner != 0
) {
width = textwidth + curwin_col_off2();


if (curwin->w_wcol >= curwin->w_width_inner) {

n = (curwin->w_wcol - curwin->w_width_inner) / width + 1;
curwin->w_wcol -= n * width;
curwin->w_wrow += n;




if (*p_sbr && *get_cursor_pos_ptr() == NUL
&& curwin->w_wcol == (int)vim_strsize(p_sbr))
curwin->w_wcol = 0;
}
}



else if (may_scroll
&& !curwin->w_cline_folded
) {






assert(siso <= INT_MAX);
int off_left = startcol - curwin->w_leftcol - (int)siso;
int off_right =
endcol - curwin->w_leftcol - curwin->w_width_inner + (int)siso + 1;
if (off_left < 0 || off_right > 0) {
int diff = (off_left < 0) ? -off_left: off_right;



int new_leftcol;
if (p_ss == 0 || diff >= textwidth / 2 || off_right >= off_left)
new_leftcol = curwin->w_wcol - extra - textwidth / 2;
else {
if (diff < p_ss) {
assert(p_ss <= INT_MAX);
diff = (int)p_ss;
}
if (off_left < 0)
new_leftcol = curwin->w_leftcol - diff;
else
new_leftcol = curwin->w_leftcol + diff;
}
if (new_leftcol < 0)
new_leftcol = 0;
if (new_leftcol != (int)curwin->w_leftcol) {
curwin->w_leftcol = new_leftcol;
win_check_anchored_floats(curwin);

redraw_later(NOT_VALID);
}
}
curwin->w_wcol -= curwin->w_leftcol;
} else if (curwin->w_wcol > (int)curwin->w_leftcol)
curwin->w_wcol -= curwin->w_leftcol;
else
curwin->w_wcol = 0;



if (curwin->w_cursor.lnum == curwin->w_topline)
curwin->w_wrow += curwin->w_topfill;
else
curwin->w_wrow += diff_check_fill(curwin, curwin->w_cursor.lnum);

prev_skipcol = curwin->w_skipcol;

int plines = 0;
if ((curwin->w_wrow >= curwin->w_height_inner
|| ((prev_skipcol > 0
|| curwin->w_wrow + so >= curwin->w_height_inner)
&& (plines =
plines_win_nofill(curwin, curwin->w_cursor.lnum, false)) - 1
>= curwin->w_height_inner))
&& curwin->w_height_inner != 0
&& curwin->w_cursor.lnum == curwin->w_topline
&& width > 0
&& curwin->w_width_inner != 0
) {






extra = 0;
if (curwin->w_skipcol + so * width > curwin->w_virtcol) {
extra = 1;
}


if (plines == 0) {
plines = plines_win(curwin, curwin->w_cursor.lnum, false);
}
plines--;
if (plines > curwin->w_wrow + so) {
assert(so <= INT_MAX);
n = curwin->w_wrow + (int)so;
} else {
n = plines;
}
if ((colnr_T)n >= curwin->w_height_inner + curwin->w_skipcol / width) {
extra += 2;
}

if (extra == 3 || plines < so * 2) {

n = curwin->w_virtcol / width;
if (n > curwin->w_height_inner / 2) {
n -= curwin->w_height_inner / 2;
} else {
n = 0;
}

if (n > plines - curwin->w_height_inner + 1) {
n = plines - curwin->w_height_inner + 1;
}
curwin->w_skipcol = n * width;
} else if (extra == 1) {

assert(so <= INT_MAX);
extra = (curwin->w_skipcol + (int)so * width - curwin->w_virtcol
+ width - 1) / width;
if (extra > 0) {
if ((colnr_T)(extra * width) > curwin->w_skipcol)
extra = curwin->w_skipcol / width;
curwin->w_skipcol -= extra * width;
}
} else if (extra == 2) {

endcol = (n - curwin->w_height_inner + 1) * width;
while (endcol > curwin->w_virtcol) {
endcol -= width;
}
if (endcol > curwin->w_skipcol) {
curwin->w_skipcol = endcol;
}
}

curwin->w_wrow -= curwin->w_skipcol / width;
if (curwin->w_wrow >= curwin->w_height_inner) {

extra = curwin->w_wrow - curwin->w_height_inner + 1;
curwin->w_skipcol += extra * width;
curwin->w_wrow -= extra;
}


extra = ((int)prev_skipcol - (int)curwin->w_skipcol) / width;
win_scroll_lines(curwin, 0, extra);
} else {
curwin->w_skipcol = 0;
}
if (prev_skipcol != curwin->w_skipcol)
redraw_later(NOT_VALID);


if (curwin->w_p_cuc && (curwin->w_valid & VALID_VIRTCOL) == 0
&& !pum_visible()) {
redraw_later(SOME_VALID);
}

curwin->w_valid |= VALID_WCOL|VALID_WROW|VALID_VIRTCOL;
}








void textpos2screenpos(win_T *wp, pos_T *pos, int *rowp, int *scolp,
int *ccolp, int *ecolp, bool local)
{
colnr_T scol = 0, ccol = 0, ecol = 0;
int row = 0;
int rowoff = 0;
colnr_T coloff = 0;
bool visible_row = false;

if (pos->lnum >= wp->w_topline && pos->lnum < wp->w_botline) {
row = plines_m_win(wp, wp->w_topline, pos->lnum - 1) + 1;
visible_row = true;
} else if (pos->lnum < wp->w_topline) {
row = 0;
} else {
row = wp->w_height_inner;
}

bool existing_row = (pos->lnum > 0
&& pos->lnum <= wp->w_buffer->b_ml.ml_line_count);

if ((local && existing_row) || visible_row) {
colnr_T off;
colnr_T col;
int width;

getvcol(wp, pos, &scol, &ccol, &ecol);


col = scol;
off = win_col_off(wp);
col += off;
width = wp->w_width - off + win_col_off2(wp);


if (wp->w_p_wrap && col >= (colnr_T)wp->w_width && width > 0) {

rowoff = visible_row ? ((col - wp->w_width) / width + 1) : 0;
col -= rowoff * width;
}

col -= wp->w_leftcol;

if (col >= 0 && col < width) {
coloff = col - scol + (local ? 0 : wp->w_wincol) + 1;
} else {
scol = ccol = ecol = 0;

if (local) {
coloff = col < 0 ? -1 : wp->w_width_inner + 1;
} else {
row = 0;
}
}
}
*rowp = (local ? 0 : wp->w_winrow) + row + rowoff;
*scolp = scol + coloff;
*ccolp = ccol + coloff;
*ecolp = ecol + coloff;
}




void 
scrolldown (
long line_count,
int byfold 
)
{
int done = 0; 


(void)hasFolding(curwin->w_topline, &curwin->w_topline, NULL);
validate_cursor(); 
while (line_count-- > 0) {
if (curwin->w_topfill < diff_check(curwin, curwin->w_topline)
&& curwin->w_topfill < curwin->w_height_inner - 1) {
curwin->w_topfill++;
done++;
} else {
if (curwin->w_topline == 1)
break;
--curwin->w_topline;
curwin->w_topfill = 0;

linenr_T first;
if (hasFolding(curwin->w_topline, &first, NULL)) {
++done;
if (!byfold)
line_count -= curwin->w_topline - first - 1;
curwin->w_botline -= curwin->w_topline - first;
curwin->w_topline = first;
} else
done += plines_nofill(curwin->w_topline);
}
--curwin->w_botline; 
invalidate_botline();
}
curwin->w_wrow += done; 
curwin->w_cline_row += done; 

if (curwin->w_cursor.lnum == curwin->w_topline)
curwin->w_cline_row = 0;
check_topfill(curwin, true);





int wrow = curwin->w_wrow;
if (curwin->w_p_wrap
&& curwin->w_width_inner != 0
) {
validate_virtcol();
validate_cheight();
wrow += curwin->w_cline_height - 1 -
curwin->w_virtcol / curwin->w_width_inner;
}
bool moved = false;
while (wrow >= curwin->w_height_inner && curwin->w_cursor.lnum > 1) {
linenr_T first;
if (hasFolding(curwin->w_cursor.lnum, &first, NULL)) {
--wrow;
if (first == 1)
curwin->w_cursor.lnum = 1;
else
curwin->w_cursor.lnum = first - 1;
} else
wrow -= plines(curwin->w_cursor.lnum--);
curwin->w_valid &=
~(VALID_WROW|VALID_WCOL|VALID_CHEIGHT|VALID_CROW|VALID_VIRTCOL);
moved = true;
}
if (moved) {

foldAdjustCursor();
coladvance(curwin->w_curswant);
}
}




void 
scrollup (
long line_count,
int byfold 
)
{
if ((byfold && hasAnyFolding(curwin))
|| curwin->w_p_diff) {

linenr_T lnum = curwin->w_topline;
while (line_count--) {
if (curwin->w_topfill > 0)
--curwin->w_topfill;
else {
if (byfold)
(void)hasFolding(lnum, NULL, &lnum);
if (lnum >= curbuf->b_ml.ml_line_count)
break;
++lnum;
curwin->w_topfill = diff_check_fill(curwin, lnum);
}
}

curwin->w_botline += lnum - curwin->w_topline;
curwin->w_topline = lnum;
} else {
curwin->w_topline += line_count;
curwin->w_botline += line_count; 
}

if (curwin->w_topline > curbuf->b_ml.ml_line_count)
curwin->w_topline = curbuf->b_ml.ml_line_count;
if (curwin->w_botline > curbuf->b_ml.ml_line_count + 1)
curwin->w_botline = curbuf->b_ml.ml_line_count + 1;

check_topfill(curwin, false);

if (hasAnyFolding(curwin))

(void)hasFolding(curwin->w_topline, &curwin->w_topline, NULL);

curwin->w_valid &= ~(VALID_WROW|VALID_CROW|VALID_BOTLINE);
if (curwin->w_cursor.lnum < curwin->w_topline) {
curwin->w_cursor.lnum = curwin->w_topline;
curwin->w_valid &=
~(VALID_WROW|VALID_WCOL|VALID_CHEIGHT|VALID_CROW|VALID_VIRTCOL);
coladvance(curwin->w_curswant);
}
}




void 
check_topfill (
win_T *wp,
bool down 
)
{
if (wp->w_topfill > 0) {
int n = plines_win_nofill(wp, wp->w_topline, true);
if (wp->w_topfill + n > wp->w_height_inner) {
if (down && wp->w_topline > 1) {
--wp->w_topline;
wp->w_topfill = 0;
} else {
wp->w_topfill = wp->w_height_inner - n;
if (wp->w_topfill < 0) {
wp->w_topfill = 0;
}
}
}
}
win_check_anchored_floats(curwin);
}





static void max_topfill(void)
{
int n = plines_nofill(curwin->w_topline);
if (n >= curwin->w_height_inner) {
curwin->w_topfill = 0;
} else {
curwin->w_topfill = diff_check_fill(curwin, curwin->w_topline);
if (curwin->w_topfill + n > curwin->w_height_inner) {
curwin->w_topfill = curwin->w_height_inner - n;
}
}
}





void scrolldown_clamp(void)
{
int can_fill = (curwin->w_topfill
< diff_check_fill(curwin, curwin->w_topline));

if (curwin->w_topline <= 1
&& !can_fill
)
return;

validate_cursor(); 






int end_row = curwin->w_wrow;
if (can_fill)
++end_row;
else
end_row += plines_nofill(curwin->w_topline - 1);
if (curwin->w_p_wrap
&& curwin->w_width_inner != 0
) {
validate_cheight();
validate_virtcol();
end_row += curwin->w_cline_height - 1 -
curwin->w_virtcol / curwin->w_width_inner;
}
if (end_row < curwin->w_height_inner - get_scrolloff_value()) {
if (can_fill) {
++curwin->w_topfill;
check_topfill(curwin, true);
} else {
--curwin->w_topline;
curwin->w_topfill = 0;
}
(void)hasFolding(curwin->w_topline, &curwin->w_topline, NULL);
--curwin->w_botline; 
curwin->w_valid &= ~(VALID_WROW|VALID_CROW|VALID_BOTLINE);
}
}





void scrollup_clamp(void)
{
if (curwin->w_topline == curbuf->b_ml.ml_line_count
&& curwin->w_topfill == 0
)
return;

validate_cursor(); 






int start_row = curwin->w_wrow - plines_nofill(curwin->w_topline)
- curwin->w_topfill;
if (curwin->w_p_wrap
&& curwin->w_width_inner != 0
) {
validate_virtcol();
start_row -= curwin->w_virtcol / curwin->w_width_inner;
}
if (start_row >= get_scrolloff_value()) {
if (curwin->w_topfill > 0) {
curwin->w_topfill--;
} else {
(void)hasFolding(curwin->w_topline, NULL, &curwin->w_topline);
curwin->w_topline++;
}
curwin->w_botline++; 
curwin->w_valid &= ~(VALID_WROW|VALID_CROW|VALID_BOTLINE);
}
}







static void topline_back(lineoff_T *lp)
{
if (lp->fill < diff_check_fill(curwin, lp->lnum)) {

++lp->fill;
lp->height = 1;
} else {
--lp->lnum;
lp->fill = 0;
if (lp->lnum < 1)
lp->height = MAXCOL;
else if (hasFolding(lp->lnum, &lp->lnum, NULL))

lp->height = 1;
else {
lp->height = plines_nofill(lp->lnum);
}
}
}







static void botline_forw(lineoff_T *lp)
{
if (lp->fill < diff_check_fill(curwin, lp->lnum + 1)) {

++lp->fill;
lp->height = 1;
} else {
++lp->lnum;
lp->fill = 0;
if (lp->lnum > curbuf->b_ml.ml_line_count) {
lp->height = MAXCOL;
} else if (hasFolding(lp->lnum, NULL, &lp->lnum)) {

lp->height = 1;
} else {
lp->height = plines_nofill(lp->lnum);
}
}
}






static void botline_topline(lineoff_T *lp)
{
if (lp->fill > 0) {
++lp->lnum;
lp->fill = diff_check_fill(curwin, lp->lnum) - lp->fill + 1;
}
}






static void topline_botline(lineoff_T *lp)
{
if (lp->fill > 0) {
lp->fill = diff_check_fill(curwin, lp->lnum) - lp->fill + 1;
--lp->lnum;
}
}






void scroll_cursor_top(int min_scroll, int always)
{
int scrolled = 0;
linenr_T top; 
linenr_T bot; 
linenr_T old_topline = curwin->w_topline;
linenr_T old_topfill = curwin->w_topfill;
linenr_T new_topline;
int off = (int)get_scrolloff_value();

if (mouse_dragging > 0)
off = mouse_dragging - 1;








validate_cheight();
int used = curwin->w_cline_height; 
if (curwin->w_cursor.lnum < curwin->w_topline) {
scrolled = used;
}

if (hasFolding(curwin->w_cursor.lnum, &top, &bot)) {
--top;
++bot;
} else {
top = curwin->w_cursor.lnum - 1;
bot = curwin->w_cursor.lnum + 1;
}
new_topline = top + 1;




int extra = diff_check_fill(curwin, curwin->w_cursor.lnum);





while (top > 0) {
int i = hasFolding(top, &top, NULL)
? 1 
: plines_nofill(top);
used += i;
if (extra + i <= off && bot < curbuf->b_ml.ml_line_count) {
if (hasFolding(bot, NULL, &bot))

++used;
else
used += plines(bot);
}
if (used > curwin->w_height_inner) {
break;
}
if (top < curwin->w_topline) {
scrolled += i;
}




if ((new_topline >= curwin->w_topline || scrolled > min_scroll)
&& extra >= off)
break;

extra += i;
new_topline = top;
--top;
++bot;
}






if (used > curwin->w_height_inner) {
scroll_cursor_halfway(false);
} else {




if (new_topline < curwin->w_topline || always)
curwin->w_topline = new_topline;
if (curwin->w_topline > curwin->w_cursor.lnum)
curwin->w_topline = curwin->w_cursor.lnum;
curwin->w_topfill = diff_check_fill(curwin, curwin->w_topline);
if (curwin->w_topfill > 0 && extra > off) {
curwin->w_topfill -= extra - off;
if (curwin->w_topfill < 0)
curwin->w_topfill = 0;
}
check_topfill(curwin, false);
if (curwin->w_topline != old_topline
|| curwin->w_topfill != old_topfill
)
curwin->w_valid &=
~(VALID_WROW|VALID_CROW|VALID_BOTLINE|VALID_BOTLINE_AP);
curwin->w_valid |= VALID_TOPLINE;
curwin->w_viewport_invalid = true;
}
}





void set_empty_rows(win_T *wp, int used)
{
wp->w_filler_rows = 0;
if (used == 0) {
wp->w_empty_rows = 0; 
} else {
wp->w_empty_rows = wp->w_height_inner - used;
if (wp->w_botline <= wp->w_buffer->b_ml.ml_line_count) {
wp->w_filler_rows = diff_check_fill(wp, wp->w_botline);
if (wp->w_empty_rows > wp->w_filler_rows)
wp->w_empty_rows -= wp->w_filler_rows;
else {
wp->w_filler_rows = wp->w_empty_rows;
wp->w_empty_rows = 0;
}
}
}
}







void scroll_cursor_bot(int min_scroll, int set_topbot)
{
int used;
int scrolled = 0;
int extra = 0;
lineoff_T loff;
lineoff_T boff;
int fill_below_window;
linenr_T old_topline = curwin->w_topline;
int old_topfill = curwin->w_topfill;
linenr_T old_botline = curwin->w_botline;
int old_valid = curwin->w_valid;
int old_empty_rows = curwin->w_empty_rows;
linenr_T cln = curwin->w_cursor.lnum; 
long so = get_scrolloff_value();

if (set_topbot) {
used = 0;
curwin->w_botline = cln + 1;
loff.fill = 0;
for (curwin->w_topline = curwin->w_botline;
curwin->w_topline > 1;
curwin->w_topline = loff.lnum) {
loff.lnum = curwin->w_topline;
topline_back(&loff);
if (loff.height == MAXCOL
|| used + loff.height > curwin->w_height_inner) {
break;
}
used += loff.height;
curwin->w_topfill = loff.fill;
}
set_empty_rows(curwin, used);
curwin->w_valid |= VALID_BOTLINE|VALID_BOTLINE_AP;
if (curwin->w_topline != old_topline
|| curwin->w_topfill != old_topfill
)
curwin->w_valid &= ~(VALID_WROW|VALID_CROW);
} else
validate_botline();


used = plines_nofill(cln);




if (cln >= curwin->w_botline) {
scrolled = used;
if (cln == curwin->w_botline)
scrolled -= curwin->w_empty_rows;
}








if (!hasFolding(curwin->w_cursor.lnum, &loff.lnum, &boff.lnum)) {
loff.lnum = cln;
boff.lnum = cln;
}
loff.fill = 0;
boff.fill = 0;
fill_below_window = diff_check_fill(curwin, curwin->w_botline)
- curwin->w_filler_rows;

while (loff.lnum > 1) {


if ((((scrolled <= 0 || scrolled >= min_scroll)
&& extra >= (mouse_dragging > 0 ? mouse_dragging - 1 : so))
|| boff.lnum + 1 > curbuf->b_ml.ml_line_count)
&& loff.lnum <= curwin->w_botline
&& (loff.lnum < curwin->w_botline
|| loff.fill >= fill_below_window)
) {
break;
}


topline_back(&loff);
if (loff.height == MAXCOL) {
used = MAXCOL;
} else {
used += loff.height;
}
if (used > curwin->w_height_inner) {
break;
}
if (loff.lnum >= curwin->w_botline
&& (loff.lnum > curwin->w_botline
|| loff.fill <= fill_below_window)
) {

scrolled += loff.height;
if (loff.lnum == curwin->w_botline
&& loff.fill == 0) {
scrolled -= curwin->w_empty_rows;
}
}

if (boff.lnum < curbuf->b_ml.ml_line_count) {

botline_forw(&boff);
used += boff.height;
if (used > curwin->w_height_inner) {
break;
}
if (extra < (mouse_dragging > 0 ? mouse_dragging - 1 : so)
|| scrolled < min_scroll) {
extra += boff.height;
if (boff.lnum >= curwin->w_botline
|| (boff.lnum + 1 == curwin->w_botline
&& boff.fill > curwin->w_filler_rows)
) {

scrolled += boff.height;
if (boff.lnum == curwin->w_botline
&& boff.fill == 0
)
scrolled -= curwin->w_empty_rows;
}
}
}
}

linenr_T line_count;

if (scrolled <= 0) {
line_count = 0;

} else if (used > curwin->w_height_inner) {
line_count = used;

} else {
line_count = 0;
boff.fill = curwin->w_topfill;
boff.lnum = curwin->w_topline - 1;
int i;
for (i = 0; i < scrolled && boff.lnum < curwin->w_botline; ) {
botline_forw(&boff);
i += boff.height;
++line_count;
}
if (i < scrolled) 
line_count = 9999;
}





if (line_count >= curwin->w_height_inner && line_count > min_scroll) {
scroll_cursor_halfway(false);
} else {
scrollup(line_count, true);
}






if (curwin->w_topline == old_topline && set_topbot) {
curwin->w_botline = old_botline;
curwin->w_empty_rows = old_empty_rows;
curwin->w_valid = old_valid;
}
curwin->w_valid |= VALID_TOPLINE;
curwin->w_viewport_invalid = true;
}





void scroll_cursor_halfway(int atend)
{
int above = 0;
int topfill = 0;
int below = 0;
lineoff_T loff;
lineoff_T boff;
linenr_T old_topline = curwin->w_topline;

loff.lnum = boff.lnum = curwin->w_cursor.lnum;
(void)hasFolding(loff.lnum, &loff.lnum, &boff.lnum);
int used = plines_nofill(loff.lnum);
loff.fill = 0;
boff.fill = 0;
linenr_T topline = loff.lnum;
while (topline > 1) {
if (below <= above) { 
if (boff.lnum < curbuf->b_ml.ml_line_count) {
botline_forw(&boff);
used += boff.height;
if (used > curwin->w_height_inner) {
break;
}
below += boff.height;
} else {
++below; 
if (atend)
++used;
}
}

if (below > above) { 
topline_back(&loff);
if (loff.height == MAXCOL)
used = MAXCOL;
else
used += loff.height;
if (used > curwin->w_height_inner) {
break;
}
above += loff.height;
topline = loff.lnum;
topfill = loff.fill;
}
}
if (!hasFolding(topline, &curwin->w_topline, NULL))
curwin->w_topline = topline;
curwin->w_topfill = topfill;
if (old_topline > curwin->w_topline + curwin->w_height_inner) {
curwin->w_botfill = false;
}
check_topfill(curwin, false);
curwin->w_valid &= ~(VALID_WROW|VALID_CROW|VALID_BOTLINE|VALID_BOTLINE_AP);
curwin->w_valid |= VALID_TOPLINE;
}







void cursor_correct(void)
{




int above_wanted = (int)get_scrolloff_value();
int below_wanted = (int)get_scrolloff_value();
if (mouse_dragging > 0) {
above_wanted = mouse_dragging - 1;
below_wanted = mouse_dragging - 1;
}
if (curwin->w_topline == 1) {
above_wanted = 0;
int max_off = curwin->w_height_inner / 2;
if (below_wanted > max_off) {
below_wanted = max_off;
}
}
validate_botline();
if (curwin->w_botline == curbuf->b_ml.ml_line_count + 1
&& mouse_dragging == 0) {
below_wanted = 0;
int max_off = (curwin->w_height_inner - 1) / 2;
if (above_wanted > max_off) {
above_wanted = max_off;
}
}





linenr_T cln = curwin->w_cursor.lnum; 
if (cln >= curwin->w_topline + above_wanted
&& cln < curwin->w_botline - below_wanted
&& !hasAnyFolding(curwin)
)
return;







linenr_T topline = curwin->w_topline;
linenr_T botline = curwin->w_botline - 1;

int above = curwin->w_topfill; 
int below = curwin->w_filler_rows; 
while ((above < above_wanted || below < below_wanted) && topline < botline) {
if (below < below_wanted && (below <= above || above >= above_wanted)) {
if (hasFolding(botline, &botline, NULL))
++below;
else
below += plines(botline);
--botline;
}
if (above < above_wanted && (above < below || below >= below_wanted)) {
if (hasFolding(topline, NULL, &topline))
++above;
else
above += plines_nofill(topline);


if (topline < botline)
above += diff_check_fill(curwin, topline + 1);
++topline;
}
}
if (topline == botline || botline == 0)
curwin->w_cursor.lnum = topline;
else if (topline > botline)
curwin->w_cursor.lnum = botline;
else {
if (cln < topline && curwin->w_topline > 1) {
curwin->w_cursor.lnum = topline;
curwin->w_valid &=
~(VALID_WROW|VALID_WCOL|VALID_CHEIGHT|VALID_CROW);
}
if (cln > botline && curwin->w_botline <= curbuf->b_ml.ml_line_count) {
curwin->w_cursor.lnum = botline;
curwin->w_valid &=
~(VALID_WROW|VALID_WCOL|VALID_CHEIGHT|VALID_CROW);
}
}
curwin->w_valid |= VALID_TOPLINE;
curwin->w_viewport_invalid = true;
}







int onepage(Direction dir, long count)
{
long n;
int retval = OK;
lineoff_T loff;
linenr_T old_topline = curwin->w_topline;
long so = get_scrolloff_value();

if (curbuf->b_ml.ml_line_count == 1) { 
beep_flush();
return FAIL;
}

for (; count > 0; --count) {
validate_botline();






if (dir == FORWARD
? ((curwin->w_topline >= curbuf->b_ml.ml_line_count - so)
&& curwin->w_botline > curbuf->b_ml.ml_line_count)
: (curwin->w_topline == 1
&& curwin->w_topfill ==
diff_check_fill(curwin, curwin->w_topline)
)) {
beep_flush();
retval = FAIL;
break;
}

loff.fill = 0;
if (dir == FORWARD) {
if (ONE_WINDOW && p_window > 0 && p_window < Rows - 1) {

if (p_window <= 2)
++curwin->w_topline;
else
curwin->w_topline += p_window - 2;
if (curwin->w_topline > curbuf->b_ml.ml_line_count)
curwin->w_topline = curbuf->b_ml.ml_line_count;
curwin->w_cursor.lnum = curwin->w_topline;
} else if (curwin->w_botline > curbuf->b_ml.ml_line_count) {

curwin->w_topline = curbuf->b_ml.ml_line_count;
curwin->w_topfill = 0;
curwin->w_valid &= ~(VALID_WROW|VALID_CROW);
} else {


loff.lnum = curwin->w_botline;
loff.fill = diff_check_fill(curwin, loff.lnum)
- curwin->w_filler_rows;
get_scroll_overlap(&loff, -1);
curwin->w_topline = loff.lnum;
curwin->w_topfill = loff.fill;
check_topfill(curwin, false);
curwin->w_cursor.lnum = curwin->w_topline;
curwin->w_valid &= ~(VALID_WCOL|VALID_CHEIGHT|VALID_WROW|
VALID_CROW|VALID_BOTLINE|VALID_BOTLINE_AP);
}
} else { 
if (curwin->w_topline == 1) {

max_topfill();
continue;
}
if (ONE_WINDOW && p_window > 0 && p_window < Rows - 1) {

if (p_window <= 2)
--curwin->w_topline;
else
curwin->w_topline -= p_window - 2;
if (curwin->w_topline < 1)
curwin->w_topline = 1;
curwin->w_cursor.lnum = curwin->w_topline + p_window - 1;
if (curwin->w_cursor.lnum > curbuf->b_ml.ml_line_count)
curwin->w_cursor.lnum = curbuf->b_ml.ml_line_count;
continue;
}




loff.lnum = curwin->w_topline - 1;
loff.fill = diff_check_fill(curwin, loff.lnum + 1)
- curwin->w_topfill;
get_scroll_overlap(&loff, 1);

if (loff.lnum >= curbuf->b_ml.ml_line_count) {
loff.lnum = curbuf->b_ml.ml_line_count;
loff.fill = 0;
} else {
botline_topline(&loff);
}
curwin->w_cursor.lnum = loff.lnum;



n = 0;
while (n <= curwin->w_height_inner && loff.lnum >= 1) {
topline_back(&loff);
if (loff.height == MAXCOL)
n = MAXCOL;
else
n += loff.height;
}
if (loff.lnum < 1) { 
curwin->w_topline = 1;
max_topfill();
curwin->w_valid &= ~(VALID_WROW|VALID_CROW|VALID_BOTLINE);
} else {

topline_botline(&loff);
botline_forw(&loff);
botline_forw(&loff);
botline_topline(&loff);

(void)hasFolding(loff.lnum, &loff.lnum, NULL);



if (loff.lnum >= curwin->w_topline
&& (loff.lnum > curwin->w_topline
|| loff.fill >= curwin->w_topfill)
) {


loff.fill = curwin->w_topfill;
if (curwin->w_topfill < diff_check_fill(curwin,
curwin->w_topline))
max_topfill();
if (curwin->w_topfill == loff.fill) {
--curwin->w_topline;
curwin->w_topfill = 0;
}
comp_botline(curwin);
curwin->w_cursor.lnum = curwin->w_botline - 1;
curwin->w_valid &=
~(VALID_WCOL | VALID_CHEIGHT | VALID_WROW | VALID_CROW);
} else {
curwin->w_topline = loff.lnum;
curwin->w_topfill = loff.fill;
check_topfill(curwin, false);
curwin->w_valid &= ~(VALID_WROW|VALID_CROW|VALID_BOTLINE);
}
}
}
}
foldAdjustCursor();
cursor_correct();
check_cursor_col();
if (retval == OK) {
beginline(BL_SOL | BL_FIX);
}
curwin->w_valid &= ~(VALID_WCOL|VALID_WROW|VALID_VIRTCOL);

if (retval == OK && dir == FORWARD) {



if (check_top_offset()) {
scroll_cursor_top(1, false);
if (curwin->w_topline <= old_topline
&& old_topline < curbuf->b_ml.ml_line_count) {
curwin->w_topline = old_topline + 1;
(void)hasFolding(curwin->w_topline, &curwin->w_topline, NULL);
}
} else if (curwin->w_botline > curbuf->b_ml.ml_line_count) {
(void)hasFolding(curwin->w_topline, &curwin->w_topline, NULL);
}
}

redraw_later(VALID);
return retval;
}













static void get_scroll_overlap(lineoff_T *lp, int dir)
{
int min_height = curwin->w_height_inner - 2;

if (lp->fill > 0)
lp->height = 1;
else
lp->height = plines_nofill(lp->lnum);
int h1 = lp->height;
if (h1 > min_height)
return; 

lineoff_T loff0 = *lp;
if (dir > 0)
botline_forw(lp);
else
topline_back(lp);
int h2 = lp->height;
if (h2 == MAXCOL || h2 + h1 > min_height) {
*lp = loff0; 
return;
}

lineoff_T loff1 = *lp;
if (dir > 0)
botline_forw(lp);
else
topline_back(lp);
int h3 = lp->height;
if (h3 == MAXCOL || h3 + h2 > min_height) {
*lp = loff0; 
return;
}

lineoff_T loff2 = *lp;
if (dir > 0)
botline_forw(lp);
else
topline_back(lp);
int h4 = lp->height;
if (h4 == MAXCOL || h4 + h3 + h2 > min_height || h3 + h2 + h1 > min_height)
*lp = loff1; 
else
*lp = loff2; 
return;
}


void halfpage(bool flag, linenr_T Prenum)
{
long scrolled = 0;
int i;

if (Prenum) {
curwin->w_p_scr = (Prenum > curwin->w_height_inner) ? curwin->w_height_inner
: Prenum;
}
assert(curwin->w_p_scr <= INT_MAX);
int n = curwin->w_p_scr <= curwin->w_height_inner ? (int)curwin->w_p_scr
: curwin->w_height_inner;

update_topline();
validate_botline();
int room = curwin->w_empty_rows + curwin->w_filler_rows;
if (flag) {



while (n > 0 && curwin->w_botline <= curbuf->b_ml.ml_line_count) {
if (curwin->w_topfill > 0) {
i = 1;
n--;
curwin->w_topfill--;
} else {
i = plines_nofill(curwin->w_topline);
n -= i;
if (n < 0 && scrolled > 0)
break;
(void)hasFolding(curwin->w_topline, NULL, &curwin->w_topline);
++curwin->w_topline;
curwin->w_topfill = diff_check_fill(curwin, curwin->w_topline);

if (curwin->w_cursor.lnum < curbuf->b_ml.ml_line_count) {
++curwin->w_cursor.lnum;
curwin->w_valid &=
~(VALID_VIRTCOL|VALID_CHEIGHT|VALID_WCOL);
}
}
curwin->w_valid &= ~(VALID_CROW|VALID_WROW);
scrolled += i;





if (curwin->w_p_diff)
curwin->w_valid &= ~(VALID_BOTLINE|VALID_BOTLINE_AP);
else {
room += i;
do {
i = plines(curwin->w_botline);
if (i > room)
break;
(void)hasFolding(curwin->w_botline, NULL,
&curwin->w_botline);
++curwin->w_botline;
room -= i;
} while (curwin->w_botline <= curbuf->b_ml.ml_line_count);
}
}


if (n > 0) {
if (hasAnyFolding(curwin)) {
while (--n >= 0
&& curwin->w_cursor.lnum < curbuf->b_ml.ml_line_count) {
(void)hasFolding(curwin->w_cursor.lnum, NULL,
&curwin->w_cursor.lnum);
++curwin->w_cursor.lnum;
}
} else
curwin->w_cursor.lnum += n;
check_cursor_lnum();
}
} else {



while (n > 0 && curwin->w_topline > 1) {
if (curwin->w_topfill < diff_check_fill(curwin, curwin->w_topline)) {
i = 1;
n--;
curwin->w_topfill++;
} else {
i = plines_nofill(curwin->w_topline - 1);
n -= i;
if (n < 0 && scrolled > 0)
break;
--curwin->w_topline;
(void)hasFolding(curwin->w_topline, &curwin->w_topline, NULL);
curwin->w_topfill = 0;
}
curwin->w_valid &= ~(VALID_CROW|VALID_WROW|
VALID_BOTLINE|VALID_BOTLINE_AP);
scrolled += i;
if (curwin->w_cursor.lnum > 1) {
--curwin->w_cursor.lnum;
curwin->w_valid &= ~(VALID_VIRTCOL|VALID_CHEIGHT|VALID_WCOL);
}
}


if (n > 0) {
if (curwin->w_cursor.lnum <= (linenr_T)n)
curwin->w_cursor.lnum = 1;
else if (hasAnyFolding(curwin)) {
while (--n >= 0 && curwin->w_cursor.lnum > 1) {
--curwin->w_cursor.lnum;
(void)hasFolding(curwin->w_cursor.lnum,
&curwin->w_cursor.lnum, NULL);
}
} else
curwin->w_cursor.lnum -= n;
}
}

foldAdjustCursor();
check_topfill(curwin, !flag);
cursor_correct();
beginline(BL_SOL | BL_FIX);
redraw_later(VALID);
}

void do_check_cursorbind(void)
{
linenr_T line = curwin->w_cursor.lnum;
colnr_T col = curwin->w_cursor.col;
colnr_T coladd = curwin->w_cursor.coladd;
colnr_T curswant = curwin->w_curswant;
int set_curswant = curwin->w_set_curswant;
win_T *old_curwin = curwin;
buf_T *old_curbuf = curbuf;
int old_VIsual_select = VIsual_select;
int old_VIsual_active = VIsual_active;




VIsual_select = VIsual_active = 0;
FOR_ALL_WINDOWS_IN_TAB(wp, curtab) {
curwin = wp;
curbuf = curwin->w_buffer;

if (curwin != old_curwin && curwin->w_p_crb) {
if (curwin->w_p_diff) {
curwin->w_cursor.lnum =
diff_get_corresponding_line(old_curbuf, line);
} else {
curwin->w_cursor.lnum = line;
}
curwin->w_cursor.col = col;
curwin->w_cursor.coladd = coladd;
curwin->w_curswant = curswant;
curwin->w_set_curswant = set_curswant;



{
int restart_edit_save = restart_edit;
restart_edit = true;
check_cursor();
if (win_cursorline_standout(curwin) || curwin->w_p_cuc) {
validate_cursor();
}
restart_edit = restart_edit_save;
}

mb_adjust_cursor();
redraw_later(VALID);


if (!curwin->w_p_scb) {
update_topline();
}
curwin->w_redr_status = true;
}
}




VIsual_select = old_VIsual_select;
VIsual_active = old_VIsual_active;
curwin = old_curwin;
curbuf = old_curbuf;
}

