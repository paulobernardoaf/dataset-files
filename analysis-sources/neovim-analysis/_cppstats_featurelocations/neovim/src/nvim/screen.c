





























































#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <string.h>

#include "nvim/log.h"
#include "nvim/vim.h"
#include "nvim/ascii.h"
#include "nvim/arabic.h"
#include "nvim/screen.h"
#include "nvim/buffer.h"
#include "nvim/charset.h"
#include "nvim/cursor.h"
#include "nvim/cursor_shape.h"
#include "nvim/diff.h"
#include "nvim/eval.h"
#include "nvim/ex_cmds.h"
#include "nvim/ex_cmds2.h"
#include "nvim/ex_getln.h"
#include "nvim/edit.h"
#include "nvim/fileio.h"
#include "nvim/fold.h"
#include "nvim/indent.h"
#include "nvim/getchar.h"
#include "nvim/highlight.h"
#include "nvim/main.h"
#include "nvim/mark.h"
#include "nvim/extmark.h"
#include "nvim/mbyte.h"
#include "nvim/memline.h"
#include "nvim/memory.h"
#include "nvim/menu.h"
#include "nvim/message.h"
#include "nvim/misc1.h"
#include "nvim/garray.h"
#include "nvim/move.h"
#include "nvim/normal.h"
#include "nvim/option.h"
#include "nvim/os_unix.h"
#include "nvim/path.h"
#include "nvim/popupmnu.h"
#include "nvim/quickfix.h"
#include "nvim/regexp.h"
#include "nvim/search.h"
#include "nvim/sign.h"
#include "nvim/spell.h"
#include "nvim/state.h"
#include "nvim/strings.h"
#include "nvim/syntax.h"
#include "nvim/terminal.h"
#include "nvim/ui.h"
#include "nvim/ui_compositor.h"
#include "nvim/undo.h"
#include "nvim/version.h"
#include "nvim/window.h"
#include "nvim/os/time.h"
#include "nvim/api/private/helpers.h"
#include "nvim/api/vim.h"
#include "nvim/lua/executor.h"

#define MB_FILLER_CHAR '<' 






static size_t linebuf_size = 0;
static schar_T *linebuf_char = NULL;
static sattr_T *linebuf_attr = NULL;

static match_T search_hl; 

static foldinfo_T win_foldinfo; 

StlClickDefinition *tab_page_click_defs = NULL;

long tab_page_click_defs_size = 0;



typedef struct {
const char_u *p;
int prev_c; 
int prev_c1; 
} LineState;
#define LINE_STATE(p) { p, 0, 0 }


static bool send_grid_resize = false;

static bool conceal_cursor_used = false;

static bool redraw_popupmenu = false;
static bool msg_grid_invalid = false;

static bool resizing = false;

#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "screen.c.generated.h"
#endif
#define SEARCH_HL_PRIORITY 0






void redraw_later(int type)
{
redraw_win_later(curwin, type);
}

void redraw_win_later(win_T *wp, int type)
FUNC_ATTR_NONNULL_ALL
{
if (!exiting && wp->w_redr_type < type) {
wp->w_redr_type = type;
if (type >= NOT_VALID)
wp->w_lines_valid = 0;
if (must_redraw < type) 
must_redraw = type;
}
}




void redraw_all_later(int type)
{
FOR_ALL_WINDOWS_IN_TAB(wp, curtab) {
redraw_win_later(wp, type);
}

if (must_redraw < type) {
must_redraw = type;
}
}

void screen_invalidate_highlights(void)
{
FOR_ALL_WINDOWS_IN_TAB(wp, curtab) {
redraw_win_later(wp, NOT_VALID);
wp->w_grid.valid = false;
}
}




void redraw_curbuf_later(int type)
{
redraw_buf_later(curbuf, type);
}

void redraw_buf_later(buf_T *buf, int type)
{
FOR_ALL_WINDOWS_IN_TAB(wp, curtab) {
if (wp->w_buffer == buf) {
redraw_win_later(wp, type);
}
}
}

void redraw_buf_line_later(buf_T *buf, linenr_T line)
{
FOR_ALL_WINDOWS_IN_TAB(wp, curtab) {
if (wp->w_buffer == buf
&& line >= wp->w_topline && line < wp->w_botline) {
redrawWinline(wp, line);
}
}
}

void redraw_buf_range_later(buf_T *buf, linenr_T firstline, linenr_T lastline)
{
FOR_ALL_WINDOWS_IN_TAB(wp, curtab) {
if (wp->w_buffer == buf
&& lastline >= wp->w_topline && firstline < wp->w_botline) {
if (wp->w_redraw_top == 0 || wp->w_redraw_top > firstline) {
wp->w_redraw_top = firstline;
}
if (wp->w_redraw_bot == 0 || wp->w_redraw_bot < lastline) {
wp->w_redraw_bot = lastline;
}
redraw_win_later(wp, VALID);
}
}
}









void
redrawWinline(
win_T *wp,
linenr_T lnum
)
FUNC_ATTR_NONNULL_ALL
{
if (lnum >= wp->w_topline
&& lnum < wp->w_botline) {
if (wp->w_redraw_top == 0 || wp->w_redraw_top > lnum) {
wp->w_redraw_top = lnum;
}
if (wp->w_redraw_bot == 0 || wp->w_redraw_bot < lnum) {
wp->w_redraw_bot = lnum;
}
redraw_win_later(wp, VALID);
}
}




void update_curbuf(int type)
{
redraw_curbuf_later(type);
update_screen(type);
}







int update_screen(int type)
{
static int did_intro = FALSE;
int did_one;




if (!default_grid.chars || resizing) {
return FAIL;
}


if (need_diff_redraw) {
diff_redraw(true);
}

if (must_redraw) {
if (type < must_redraw) 
type = must_redraw;





must_redraw = 0;
}


if (curwin->w_lines_valid == 0 && type < NOT_VALID)
type = NOT_VALID;



if (!redrawing() || updating_screen) {
redraw_later(type); 
must_redraw = type;
if (type > INVERTED_ALL) {
curwin->w_lines_valid = 0; 
}
return FAIL;
}
updating_screen = 1;

display_tick++; 




if (msg_did_scroll) {
msg_did_scroll = false;
msg_scrolled_at_flush = 0;
}

if (type >= CLEAR || !default_grid.valid) {
ui_comp_set_screen_valid(false);
}


if (msg_scrolled || msg_grid_invalid) {
clear_cmdline = true;
int valid = MAX(Rows - msg_scrollsize(), 0);
if (msg_grid.chars) {

for (int i = 0; i < MIN(msg_scrollsize(), msg_grid.Rows); i++) {
grid_clear_line(&msg_grid, msg_grid.line_offset[i],
(int)msg_grid.Columns, false);
}
}
if (msg_use_msgsep()) {
msg_grid.throttled = false;

if (type == NOT_VALID && !ui_has(kUIMultigrid) && msg_scrolled) {
ui_comp_set_screen_valid(false);
for (int i = valid; i < Rows-p_ch; i++) {
grid_clear_line(&default_grid, default_grid.line_offset[i],
Columns, false);
}
FOR_ALL_WINDOWS_IN_TAB(wp, curtab) {
if (wp->w_floating) {
continue;
}
if (W_ENDROW(wp) > valid) {
wp->w_redr_type = MAX(wp->w_redr_type, NOT_VALID);
}
if (W_ENDROW(wp) + wp->w_status_height > valid) {
wp->w_redr_status = true;
}
}
}
msg_grid_set_pos(Rows-p_ch, false);
msg_grid_invalid = false;
} else if (msg_scrolled > Rows - 5) { 
type = CLEAR;
} else if (type != CLEAR) {
check_for_delay(false);
grid_ins_lines(&default_grid, 0, msg_scrolled, Rows, 0, Columns);
FOR_ALL_WINDOWS_IN_TAB(wp, curtab) {
if (wp->w_floating) {
continue;
}
if (wp->w_winrow < msg_scrolled) {
if (W_ENDROW(wp) > msg_scrolled
&& wp->w_redr_type < REDRAW_TOP
&& wp->w_lines_valid > 0
&& wp->w_topline == wp->w_lines[0].wl_lnum) {
wp->w_upd_rows = msg_scrolled - wp->w_winrow;
wp->w_redr_type = REDRAW_TOP;
} else {
wp->w_redr_type = NOT_VALID;
if (W_ENDROW(wp) + wp->w_status_height
<= msg_scrolled) {
wp->w_redr_status = TRUE;
}
}
}
}
redraw_cmdline = TRUE;
redraw_tabline = TRUE;
}
msg_scrolled = 0;
msg_scrolled_at_flush = 0;
need_wait_return = false;
}

win_ui_flush();
msg_ext_check_clear();


compute_cmdrow();


if (need_highlight_changed)
highlight_changed();

if (type == CLEAR) { 
screenclear(); 
cmdline_screen_cleared(); 
type = NOT_VALID;

must_redraw = 0;
} else if (!default_grid.valid) {
grid_invalidate(&default_grid);
default_grid.valid = true;
}



if (type == NOT_VALID && (msg_use_grid() || msg_grid.chars)) {
grid_fill(&default_grid, Rows-p_ch, Rows, 0, Columns, ' ', ' ', 0);
}

ui_comp_set_screen_valid(true);

if (clear_cmdline) 
check_for_delay(FALSE);



if (curwin->w_redr_type < NOT_VALID
&& curwin->w_nrwidth != ((curwin->w_p_nu || curwin->w_p_rnu)
? number_width(curwin) : 0))
curwin->w_redr_type = NOT_VALID;




if (type == INVERTED)
update_curswant();
if (curwin->w_redr_type < type
&& !((type == VALID
&& curwin->w_lines[0].wl_valid
&& curwin->w_topfill == curwin->w_old_topfill
&& curwin->w_botfill == curwin->w_old_botfill
&& curwin->w_topline == curwin->w_lines[0].wl_lnum)
|| (type == INVERTED
&& VIsual_active
&& curwin->w_old_cursor_lnum == curwin->w_cursor.lnum
&& curwin->w_old_visual_mode == VIsual_mode
&& (curwin->w_valid & VALID_VIRTCOL)
&& curwin->w_old_curswant == curwin->w_curswant)
))
curwin->w_redr_type = type;


if (redraw_tabline || type >= NOT_VALID) {
update_window_hl(curwin, type >= NOT_VALID);
FOR_ALL_TABS(tp) {
if (tp != curtab) {
update_window_hl(tp->tp_curwin, type >= NOT_VALID);
}
}
draw_tabline();
}





FOR_ALL_WINDOWS_IN_TAB(wp, curtab) {
update_window_hl(wp, type >= NOT_VALID);

if (wp->w_buffer->b_mod_set) {
win_T *wwp;


for (wwp = firstwin; wwp != wp; wwp = wwp->w_next) {
if (wwp->w_buffer == wp->w_buffer) {
break;
}
}
if (wwp == wp && syntax_present(wp)) {
syn_stack_apply_changes(wp->w_buffer);
}

buf_T *buf = wp->w_buffer;
if (buf->b_luahl && buf->b_luahl_window != LUA_NOREF) {
Error err = ERROR_INIT;
FIXED_TEMP_ARRAY(args, 2);
args.items[0] = BUFFER_OBJ(buf->handle);
args.items[1] = INTEGER_OBJ(display_tick);
executor_exec_lua_cb(buf->b_luahl_start, "start", args, false, &err);
if (ERROR_SET(&err)) {
ELOG("error in luahl start: %s", err.msg);
api_clear_error(&err);
}
}
}
}





did_one = FALSE;
search_hl.rm.regprog = NULL;


FOR_ALL_WINDOWS_IN_TAB(wp, curtab) {
if (wp->w_redr_type == CLEAR && wp->w_floating && wp->w_grid.chars) {
grid_invalidate(&wp->w_grid);
wp->w_redr_type = NOT_VALID;
}

if (wp->w_redr_type != 0) {
if (!did_one) {
did_one = TRUE;
start_search_hl();
}
win_update(wp);
}


if (wp->w_redr_status) {
win_redr_status(wp);
}
}

end_search_hl();


if (pum_drawn() && must_redraw_pum) {
pum_redraw();
}

send_grid_resize = false;



FOR_ALL_WINDOWS_IN_TAB(wp, curtab) {
wp->w_buffer->b_mod_set = false;
}

updating_screen = 0;



if (clear_cmdline || redraw_cmdline) {
showmode();
}


if (!did_intro)
maybe_intro_message();
did_intro = TRUE;


cmdline_was_last_drawn = false;
return OK;
}



bool conceal_cursor_line(const win_T *wp)
FUNC_ATTR_NONNULL_ALL
{
int c;

if (*wp->w_p_cocu == NUL) {
return false;
}
if (get_real_state() & VISUAL) {
c = 'v';
} else if (State & INSERT) {
c = 'i';
} else if (State & NORMAL) {
c = 'n';
} else if (State & CMDLINE) {
c = 'c';
} else {
return false;
}
return vim_strchr(wp->w_p_cocu, c) != NULL;
}




void conceal_check_cursor_line(void)
{
bool should_conceal = conceal_cursor_line(curwin);
if (curwin->w_p_cole > 0 && (conceal_cursor_used != should_conceal)) {
redrawWinline(curwin, curwin->w_cursor.lnum);


curs_columns(true);
}
}







bool win_cursorline_standout(const win_T *wp)
FUNC_ATTR_NONNULL_ALL
{
return wp->w_p_cul
|| (wp->w_p_cole > 0 && (VIsual_active || !conceal_cursor_line(wp)));
}

static DecorationRedrawState decorations;
bool decorations_active = false;

void decorations_add_luahl_attr(int attr_id,
int start_row, int start_col,
int end_row, int end_col)
{
kv_push(decorations.active,
((HlRange){ start_row, start_col, end_row, end_col, attr_id, NULL }));
}




























static void win_update(win_T *wp)
{
buf_T *buf = wp->w_buffer;
int type;
int top_end = 0; 

int mid_start = 999; 

int mid_end = 0; 

int bot_start = 999; 

int scrolled_down = FALSE; 

bool top_to_mod = false; 

int row; 
linenr_T lnum; 
int idx; 
int srow; 

int eof = FALSE; 
int didline = FALSE; 
int i;
long j;
static int recursive = FALSE; 
int old_botline = wp->w_botline;
long fold_count;

#define DID_NONE 1 
#define DID_LINE 2 
#define DID_FOLD 3 
int did_update = DID_NONE;
linenr_T syntax_last_parsed = 0; 
linenr_T mod_top = 0;
linenr_T mod_bot = 0;
int save_got_int;




buf_signcols(buf);

type = wp->w_redr_type;

win_grid_alloc(wp);

if (type >= NOT_VALID) {
wp->w_redr_status = true;
wp->w_lines_valid = 0;
}


if (wp->w_grid.Rows == 0) {
wp->w_redr_type = 0;
return;
}


if (wp->w_grid.Columns == 0) {

draw_vsep_win(wp, 0);
wp->w_redr_type = 0;
return;
}

init_search_hl(wp);



i = (wp->w_p_nu || wp->w_p_rnu) ? number_width(wp) : 0;
if (wp->w_nrwidth != i) {
type = NOT_VALID;
wp->w_nrwidth = i;

if (buf->terminal) {
terminal_check_size(buf->terminal);
}
} else if (buf->b_mod_set
&& buf->b_mod_xlines != 0
&& wp->w_redraw_top != 0) {



type = NOT_VALID;
} else {




mod_top = wp->w_redraw_top;
if (wp->w_redraw_bot != 0)
mod_bot = wp->w_redraw_bot + 1;
else
mod_bot = 0;
if (buf->b_mod_set) {
if (mod_top == 0 || mod_top > buf->b_mod_top) {
mod_top = buf->b_mod_top;


if (syntax_present(wp)) {
mod_top -= buf->b_s.b_syn_sync_linebreaks;
if (mod_top < 1)
mod_top = 1;
}
}
if (mod_bot == 0 || mod_bot < buf->b_mod_bot)
mod_bot = buf->b_mod_bot;






if (search_hl.rm.regprog != NULL
&& re_multiline(search_hl.rm.regprog)) {
top_to_mod = true;
} else {
const matchitem_T *cur = wp->w_match_head;
while (cur != NULL) {
if (cur->match.regprog != NULL
&& re_multiline(cur->match.regprog)) {
top_to_mod = true;
break;
}
cur = cur->next;
}
}
}
if (mod_top != 0 && hasAnyFolding(wp)) {
linenr_T lnumt, lnumb;













lnumt = wp->w_topline;
lnumb = MAXLNUM;
for (i = 0; i < wp->w_lines_valid; ++i)
if (wp->w_lines[i].wl_valid) {
if (wp->w_lines[i].wl_lastlnum < mod_top)
lnumt = wp->w_lines[i].wl_lastlnum + 1;
if (lnumb == MAXLNUM && wp->w_lines[i].wl_lnum >= mod_bot) {
lnumb = wp->w_lines[i].wl_lnum;


if (compute_foldcolumn(wp, 0) > 0) {
lnumb++;
}
}
}

(void)hasFoldingWin(wp, mod_top, &mod_top, NULL, true, NULL);
if (mod_top > lnumt) {
mod_top = lnumt;
}


mod_bot--;
(void)hasFoldingWin(wp, mod_bot, NULL, &mod_bot, true, NULL);
mod_bot++;
if (mod_bot < lnumb) {
mod_bot = lnumb;
}
}





if (mod_top != 0 && mod_top < wp->w_topline) {
if (mod_bot > wp->w_topline)
mod_top = wp->w_topline;
else if (syntax_present(wp))
top_end = 1;
}



if (mod_top != 0 && buf->b_mod_xlines != 0 && wp->w_p_nu)
mod_bot = MAXLNUM;
}
wp->w_redraw_top = 0; 
wp->w_redraw_bot = 0;





if (type == REDRAW_TOP) {
j = 0;
for (i = 0; i < wp->w_lines_valid; ++i) {
j += wp->w_lines[i].wl_size;
if (j >= wp->w_upd_rows) {
top_end = j;
break;
}
}
if (top_end == 0)

type = NOT_VALID;
else

type = VALID;
}









if ((type == VALID || type == SOME_VALID
|| type == INVERTED || type == INVERTED_ALL)
&& !wp->w_botfill && !wp->w_old_botfill
) {
if (mod_top != 0 && wp->w_topline == mod_top) {




} else if (wp->w_lines[0].wl_valid
&& (wp->w_topline < wp->w_lines[0].wl_lnum
|| (wp->w_topline == wp->w_lines[0].wl_lnum
&& wp->w_topfill > wp->w_old_topfill)
)) {



if (hasAnyFolding(wp)) {
linenr_T ln;



j = 0;
for (ln = wp->w_topline; ln < wp->w_lines[0].wl_lnum; ln++) {
j++;
if (j >= wp->w_grid.Rows - 2) {
break;
}
(void)hasFoldingWin(wp, ln, NULL, &ln, true, NULL);
}
} else
j = wp->w_lines[0].wl_lnum - wp->w_topline;
if (j < wp->w_grid.Rows - 2) { 
i = plines_m_win(wp, wp->w_topline, wp->w_lines[0].wl_lnum - 1);

if (wp->w_lines[0].wl_lnum != wp->w_topline)
i += diff_check_fill(wp, wp->w_lines[0].wl_lnum)
- wp->w_old_topfill;
if (i != 0 && i < wp->w_grid.Rows - 2) { 



win_scroll_lines(wp, 0, i);
if (wp->w_lines_valid != 0) {


top_end = i;
scrolled_down = true;



if ((wp->w_lines_valid += j) > wp->w_grid.Rows) {
wp->w_lines_valid = wp->w_grid.Rows;
}
for (idx = wp->w_lines_valid; idx - j >= 0; idx--) {
wp->w_lines[idx] = wp->w_lines[idx - j];
}
while (idx >= 0) {
wp->w_lines[idx--].wl_valid = false;
}
}
} else {
mid_start = 0; 
}
} else {
mid_start = 0; 
}
} else {







j = -1;
row = 0;
for (i = 0; i < wp->w_lines_valid; i++) {
if (wp->w_lines[i].wl_valid
&& wp->w_lines[i].wl_lnum == wp->w_topline) {
j = i;
break;
}
row += wp->w_lines[i].wl_size;
}
if (j == -1) {


mid_start = 0;
} else {






if (wp->w_lines[0].wl_lnum == wp->w_topline)
row += wp->w_old_topfill;
else
row += diff_check_fill(wp, wp->w_topline);

row -= wp->w_topfill;
if (row > 0) {
win_scroll_lines(wp, 0, -row);
bot_start = wp->w_grid.Rows - row;
}
if ((row == 0 || bot_start < 999) && wp->w_lines_valid != 0) {






bot_start = 0;
idx = 0;
for (;; ) {
wp->w_lines[idx] = wp->w_lines[j];


if (row > 0 && bot_start + row
+ (int)wp->w_lines[j].wl_size > wp->w_grid.Rows) {
wp->w_lines_valid = idx + 1;
break;
}
bot_start += wp->w_lines[idx++].wl_size;


if (++j >= wp->w_lines_valid) {
wp->w_lines_valid = idx;
break;
}
}


if (wp->w_p_diff && bot_start > 0)
wp->w_lines[0].wl_size =
plines_win_nofill(wp, wp->w_topline, true)
+ wp->w_topfill;
}
}
}


if (mid_start == 0) {
mid_end = wp->w_grid.Rows;
}
} else {

mid_start = 0;
mid_end = wp->w_grid.Rows;
}

if (type == SOME_VALID) {

mid_start = 0;
mid_end = wp->w_grid.Rows;
type = NOT_VALID;
}


if ((VIsual_active && buf == curwin->w_buffer)
|| (wp->w_old_cursor_lnum != 0 && type != NOT_VALID)) {
linenr_T from, to;

if (VIsual_active) {
if (VIsual_mode != wp->w_old_visual_mode || type == INVERTED_ALL) {



if (curwin->w_cursor.lnum < VIsual.lnum) {
from = curwin->w_cursor.lnum;
to = VIsual.lnum;
} else {
from = VIsual.lnum;
to = curwin->w_cursor.lnum;
}

if (wp->w_old_cursor_lnum < from)
from = wp->w_old_cursor_lnum;
if (wp->w_old_cursor_lnum > to)
to = wp->w_old_cursor_lnum;
if (wp->w_old_visual_lnum < from)
from = wp->w_old_visual_lnum;
if (wp->w_old_visual_lnum > to)
to = wp->w_old_visual_lnum;
} else {





if (curwin->w_cursor.lnum < wp->w_old_cursor_lnum) {
from = curwin->w_cursor.lnum;
to = wp->w_old_cursor_lnum;
} else {
from = wp->w_old_cursor_lnum;
to = curwin->w_cursor.lnum;
if (from == 0) 
from = to;
}

if (VIsual.lnum != wp->w_old_visual_lnum
|| VIsual.col != wp->w_old_visual_col) {
if (wp->w_old_visual_lnum < from
&& wp->w_old_visual_lnum != 0)
from = wp->w_old_visual_lnum;
if (wp->w_old_visual_lnum > to)
to = wp->w_old_visual_lnum;
if (VIsual.lnum < from)
from = VIsual.lnum;
if (VIsual.lnum > to)
to = VIsual.lnum;
}
}






if (VIsual_mode == Ctrl_V) {
colnr_T fromc, toc;
int save_ve_flags = ve_flags;

if (curwin->w_p_lbr)
ve_flags = VE_ALL;

getvcols(wp, &VIsual, &curwin->w_cursor, &fromc, &toc);
ve_flags = save_ve_flags;
++toc;
if (curwin->w_curswant == MAXCOL)
toc = MAXCOL;

if (fromc != wp->w_old_cursor_fcol
|| toc != wp->w_old_cursor_lcol) {
if (from > VIsual.lnum)
from = VIsual.lnum;
if (to < VIsual.lnum)
to = VIsual.lnum;
}
wp->w_old_cursor_fcol = fromc;
wp->w_old_cursor_lcol = toc;
}
} else {

if (wp->w_old_cursor_lnum < wp->w_old_visual_lnum) {
from = wp->w_old_cursor_lnum;
to = wp->w_old_visual_lnum;
} else {
from = wp->w_old_visual_lnum;
to = wp->w_old_cursor_lnum;
}
}




if (from < wp->w_topline)
from = wp->w_topline;





if (wp->w_valid & VALID_BOTLINE) {
if (from >= wp->w_botline)
from = wp->w_botline - 1;
if (to >= wp->w_botline)
to = wp->w_botline - 1;
}










if (mid_start > 0) {
lnum = wp->w_topline;
idx = 0;
srow = 0;
if (scrolled_down)
mid_start = top_end;
else
mid_start = 0;
while (lnum < from && idx < wp->w_lines_valid) { 
if (wp->w_lines[idx].wl_valid)
mid_start += wp->w_lines[idx].wl_size;
else if (!scrolled_down)
srow += wp->w_lines[idx].wl_size;
++idx;
if (idx < wp->w_lines_valid && wp->w_lines[idx].wl_valid)
lnum = wp->w_lines[idx].wl_lnum;
else
++lnum;
}
srow += mid_start;
mid_end = wp->w_grid.Rows;
for (; idx < wp->w_lines_valid; idx++) { 
if (wp->w_lines[idx].wl_valid
&& wp->w_lines[idx].wl_lnum >= to + 1) {

mid_end = srow;
break;
}
srow += wp->w_lines[idx].wl_size;
}
}
}

if (VIsual_active && buf == curwin->w_buffer) {
wp->w_old_visual_mode = VIsual_mode;
wp->w_old_cursor_lnum = curwin->w_cursor.lnum;
wp->w_old_visual_lnum = VIsual.lnum;
wp->w_old_visual_col = VIsual.col;
wp->w_old_curswant = curwin->w_curswant;
} else {
wp->w_old_visual_mode = 0;
wp->w_old_cursor_lnum = 0;
wp->w_old_visual_lnum = 0;
wp->w_old_visual_col = 0;
}


save_got_int = got_int;
got_int = 0;

proftime_T syntax_tm = profile_setlimit(p_rdt);
syn_set_timeout(&syntax_tm);
win_foldinfo.fi_level = 0;




idx = 0; 
row = 0;
srow = 0;
lnum = wp->w_topline; 

decorations_active = decorations_redraw_reset(buf, &decorations);

if (buf->b_luahl && buf->b_luahl_window != LUA_NOREF) {
Error err = ERROR_INIT;
FIXED_TEMP_ARRAY(args, 4);
linenr_T knownmax = ((wp->w_valid & VALID_BOTLINE)
? wp->w_botline
: (wp->w_topline + wp->w_height_inner));
args.items[0] = WINDOW_OBJ(wp->handle);
args.items[1] = BUFFER_OBJ(buf->handle);

args.items[2] = INTEGER_OBJ(wp->w_topline-1);
args.items[3] = INTEGER_OBJ(knownmax);


executor_exec_lua_cb(buf->b_luahl_window, "window", args, false, &err);
if (ERROR_SET(&err)) {
ELOG("error in luahl window: %s", err.msg);
api_clear_error(&err);
}
}


for (;; ) {


if (row == wp->w_grid.Rows) {
didline = true;
break;
}


if (lnum > buf->b_ml.ml_line_count) {
eof = TRUE;
break;
}



srow = row;








if (row < top_end
|| (row >= mid_start && row < mid_end)
|| top_to_mod
|| idx >= wp->w_lines_valid
|| (row + wp->w_lines[idx].wl_size > bot_start)
|| (mod_top != 0
&& (lnum == mod_top
|| (lnum >= mod_top
&& (lnum < mod_bot
|| did_update == DID_FOLD
|| (did_update == DID_LINE
&& syntax_present(wp)
&& ((foldmethodIsSyntax(wp)
&& hasAnyFolding(wp))
|| syntax_check_changed(lnum)))


|| (wp->w_match_head != NULL
&& buf->b_mod_xlines != 0)))))) {
if (lnum == mod_top) {
top_to_mod = false;
}







if (lnum == mod_top
&& mod_bot != MAXLNUM
&& !(dollar_vcol >= 0 && mod_bot == mod_top + 1)) {
int old_rows = 0;
int new_rows = 0;
int xtra_rows;
linenr_T l;




for (i = idx; i < wp->w_lines_valid; ++i) {


if (wp->w_lines[i].wl_valid
&& wp->w_lines[i].wl_lnum == mod_bot)
break;
old_rows += wp->w_lines[i].wl_size;
if (wp->w_lines[i].wl_valid
&& wp->w_lines[i].wl_lastlnum + 1 == mod_bot) {


++i;
while (i < wp->w_lines_valid
&& !wp->w_lines[i].wl_valid)
old_rows += wp->w_lines[i++].wl_size;
break;
}
}

if (i >= wp->w_lines_valid) {



bot_start = 0;
} else {


j = idx;
for (l = lnum; l < mod_bot; l++) {
if (hasFoldingWin(wp, l, NULL, &l, true, NULL)) {
new_rows++;
} else if (l == wp->w_topline) {
new_rows += plines_win_nofill(wp, l, true) + wp->w_topfill;
} else {
new_rows += plines_win(wp, l, true);
}
j++;
if (new_rows > wp->w_grid.Rows - row - 2) {

new_rows = 9999;
break;
}
}
xtra_rows = new_rows - old_rows;
if (xtra_rows < 0) {




if (row - xtra_rows >= wp->w_grid.Rows - 2) {
mod_bot = MAXLNUM;
} else {
win_scroll_lines(wp, row, xtra_rows);
bot_start = wp->w_grid.Rows + xtra_rows;
}
} else if (xtra_rows > 0) {



if (row + xtra_rows >= wp->w_grid.Rows - 2) {
mod_bot = MAXLNUM;
} else {
win_scroll_lines(wp, row + old_rows, xtra_rows);
if (top_end > row + old_rows) {


top_end += xtra_rows;
}
}
}



if (mod_bot != MAXLNUM && i != j) {
if (j < i) {
int x = row + new_rows;


for (;; ) {

if (i >= wp->w_lines_valid) {
wp->w_lines_valid = j;
break;
}
wp->w_lines[j] = wp->w_lines[i];

if (x + (int)wp->w_lines[j].wl_size
> wp->w_grid.Rows) {
wp->w_lines_valid = j + 1;
break;
}
x += wp->w_lines[j++].wl_size;
++i;
}
if (bot_start > x)
bot_start = x;
} else { 

j -= i;
wp->w_lines_valid += j;
if (wp->w_lines_valid > wp->w_grid.Rows) {
wp->w_lines_valid = wp->w_grid.Rows;
}
for (i = wp->w_lines_valid; i - j >= idx; i--) {
wp->w_lines[i] = wp->w_lines[i - j];
}




while (i >= idx) {
wp->w_lines[i].wl_size = 0;
wp->w_lines[i--].wl_valid = FALSE;
}
}
}
}
}






fold_count = foldedCount(wp, lnum, &win_foldinfo);
if (fold_count != 0) {
fold_line(wp, fold_count, &win_foldinfo, lnum, row);
++row;
--fold_count;
wp->w_lines[idx].wl_folded = TRUE;
wp->w_lines[idx].wl_lastlnum = lnum + fold_count;
did_update = DID_FOLD;
} else if (idx < wp->w_lines_valid
&& wp->w_lines[idx].wl_valid
&& wp->w_lines[idx].wl_lnum == lnum
&& lnum > wp->w_topline
&& !(dy_flags & (DY_LASTLINE | DY_TRUNCATE))
&& srow + wp->w_lines[idx].wl_size > wp->w_grid.Rows
&& diff_check_fill(wp, lnum) == 0
) {


row = wp->w_grid.Rows + 1;
} else {
prepare_search_hl(wp, lnum);

if (syntax_last_parsed != 0 && syntax_last_parsed + 1 < lnum
&& syntax_present(wp))
syntax_end_parsing(syntax_last_parsed + 1);




row = win_line(wp, lnum, srow, wp->w_grid.Rows, mod_top == 0, false);

wp->w_lines[idx].wl_folded = FALSE;
wp->w_lines[idx].wl_lastlnum = lnum;
did_update = DID_LINE;
syntax_last_parsed = lnum;
}

wp->w_lines[idx].wl_lnum = lnum;
wp->w_lines[idx].wl_valid = true;

if (row > wp->w_grid.Rows) { 

if (dollar_vcol == -1) {
wp->w_lines[idx].wl_size = plines_win(wp, lnum, true);
}
idx++;
break;
}
if (dollar_vcol == -1)
wp->w_lines[idx].wl_size = row - srow;
++idx;
lnum += fold_count + 1;
} else {
if (wp->w_p_rnu) {


fold_count = foldedCount(wp, lnum, &win_foldinfo);
if (fold_count != 0) {
fold_line(wp, fold_count, &win_foldinfo, lnum, row);
} else {
(void)win_line(wp, lnum, srow, wp->w_grid.Rows, true, true);
}
}


row += wp->w_lines[idx++].wl_size;
if (row > wp->w_grid.Rows) { 
break;
}
lnum = wp->w_lines[idx - 1].wl_lastlnum + 1;
did_update = DID_NONE;
}

if (lnum > buf->b_ml.ml_line_count) {
eof = TRUE;
break;
}
}





if (idx > wp->w_lines_valid)
wp->w_lines_valid = idx;




if (syntax_last_parsed != 0 && syntax_present(wp))
syntax_end_parsing(syntax_last_parsed + 1);





wp->w_empty_rows = 0;
wp->w_filler_rows = 0;
if (!eof && !didline) {
int at_attr = hl_combine_attr(wp->w_hl_attr_normal,
win_hl_attr(wp, HLF_AT));
if (lnum == wp->w_topline) {




wp->w_botline = lnum + 1;
} else if (diff_check_fill(wp, lnum) >= wp->w_grid.Rows - srow) {

wp->w_botline = lnum;
wp->w_filler_rows = wp->w_grid.Rows - srow;
} else if (dy_flags & DY_TRUNCATE) { 
int scr_row = wp->w_grid.Rows - 1;


grid_puts_len(&wp->w_grid, (char_u *)"@@", 2, scr_row, 0, at_attr);

grid_fill(&wp->w_grid, scr_row, scr_row + 1, 2, wp->w_grid.Columns,
'@', ' ', at_attr);
set_empty_rows(wp, srow);
wp->w_botline = lnum;
} else if (dy_flags & DY_LASTLINE) { 

grid_fill(&wp->w_grid, wp->w_grid.Rows - 1, wp->w_grid.Rows,
wp->w_grid.Columns - 3, wp->w_grid.Columns, '@', '@', at_attr);
set_empty_rows(wp, srow);
wp->w_botline = lnum;
} else {
win_draw_end(wp, '@', ' ', true, srow, wp->w_grid.Rows, HLF_AT);
wp->w_botline = lnum;
}
} else {
if (eof) { 
wp->w_botline = buf->b_ml.ml_line_count + 1;
j = diff_check_fill(wp, wp->w_botline);
if (j > 0 && !wp->w_botfill) {

if (char2cells(wp->w_p_fcs_chars.diff) > 1) {
i = '-';
} else {
i = wp->w_p_fcs_chars.diff;
}
if (row + j > wp->w_grid.Rows) {
j = wp->w_grid.Rows - row;
}
win_draw_end(wp, i, i, true, row, row + (int)j, HLF_DED);
row += j;
}
} else if (dollar_vcol == -1)
wp->w_botline = lnum;



win_draw_end(wp, wp->w_p_fcs_chars.eob, ' ', false, row, wp->w_grid.Rows,
HLF_EOB);
}

if (wp->w_redr_type >= REDRAW_TOP) {
draw_vsep_win(wp, 0);
}
syn_set_timeout(NULL);


wp->w_redr_type = 0;
wp->w_old_topfill = wp->w_topfill;
wp->w_old_botfill = wp->w_botfill;

if (dollar_vcol == -1) {












wp->w_valid |= VALID_BOTLINE;
wp->w_viewport_invalid = true;
if (wp == curwin && wp->w_botline != old_botline && !recursive) {
recursive = TRUE;
curwin->w_valid &= ~VALID_TOPLINE;
update_topline(); 
if (must_redraw != 0) {

i = curbuf->b_mod_set;
curbuf->b_mod_set = false;
win_update(curwin);
must_redraw = 0;
curbuf->b_mod_set = i;
}
recursive = FALSE;
}
}


if (!got_int)
got_int = save_got_int;
} 










int win_signcol_width(win_T *wp)
{

return 2;
}



static int win_fill_end(win_T *wp, int c1, int c2, int off, int width, int row,
int endrow, int attr)
{
int nn = off + width;

if (nn > wp->w_grid.Columns) {
nn = wp->w_grid.Columns;
}

if (wp->w_p_rl) {
grid_fill(&wp->w_grid, row, endrow, W_ENDCOL(wp) - nn, W_ENDCOL(wp) - off,
c1, c2, attr);
} else {
grid_fill(&wp->w_grid, row, endrow, off, nn, c1, c2, attr);
}

return nn;
}




static void win_draw_end(win_T *wp, int c1, int c2, bool draw_margin, int row,
int endrow, hlf_T hl)
{
assert(hl >= 0 && hl < HLF_COUNT);
int n = 0;

if (draw_margin) {

int fdc = compute_foldcolumn(wp, 0);
if (fdc > 0) {
n = win_fill_end(wp, ' ', ' ', n, fdc, row, endrow,
win_hl_attr(wp, HLF_FC));
}

int count = win_signcol_count(wp);
if (count > 0) {
n = win_fill_end(wp, ' ', ' ', n, win_signcol_width(wp) * count, row,
endrow, win_hl_attr(wp, HLF_SC));
}

if ((wp->w_p_nu || wp->w_p_rnu) && vim_strchr(p_cpo, CPO_NUMCOL) == NULL) {
n = win_fill_end(wp, ' ', ' ', n, number_width(wp) + 1, row, endrow,
win_hl_attr(wp, HLF_N));
}
}

int attr = hl_combine_attr(wp->w_hl_attr_normal, win_hl_attr(wp, hl));

if (wp->w_p_rl) {
grid_fill(&wp->w_grid, row, endrow, wp->w_wincol, W_ENDCOL(wp) - 1 - n,
c2, c2, attr);
grid_fill(&wp->w_grid, row, endrow, W_ENDCOL(wp) - 1 - n, W_ENDCOL(wp) - n,
c1, c2, attr);
} else {
grid_fill(&wp->w_grid, row, endrow, n, wp->w_grid.Columns, c1, c2, attr);
}

set_empty_rows(wp, row);
}





static int advance_color_col(int vcol, int **color_cols)
{
while (**color_cols >= 0 && vcol > **color_cols)
++*color_cols;
return **color_cols >= 0;
}



static int compute_foldcolumn(win_T *wp, int col)
{
int fdc = win_fdccol_count(wp);
int wmw = wp == curwin && p_wmw == 0 ? 1 : p_wmw;
int wwidth = wp->w_grid.Columns;

if (fdc > wwidth - (col + wmw)) {
fdc = wwidth - (col + wmw);
}
return fdc;
}




static int line_putchar(LineState *s, schar_T *dest, int maxcells, bool rl)
{
const char_u *p = s->p;
int cells = utf_ptr2cells(p);
int c_len = utfc_ptr2len(p);
int u8c, u8cc[MAX_MCO];
if (cells > maxcells) {
return -1;
}
u8c = utfc_ptr2char(p, u8cc);
if (*p < 0x80 && u8cc[0] == 0) {
schar_from_ascii(dest[0], *p);
s->prev_c = u8c;
} else {
if (p_arshape && !p_tbidi && arabic_char(u8c)) {

int pc, pc1, nc;
int pcc[MAX_MCO];
int firstbyte = *p;



if (rl) {
pc = s->prev_c;
pc1 = s->prev_c1;
nc = utf_ptr2char(p + c_len);
s->prev_c1 = u8cc[0];
} else {
pc = utfc_ptr2char(p + c_len, pcc);
nc = s->prev_c;
pc1 = pcc[0];
}
s->prev_c = u8c;

u8c = arabic_shape(u8c, &firstbyte, &u8cc[0], pc, pc1, nc);
} else {
s->prev_c = u8c;
}
schar_from_cc(dest[0], u8c, u8cc);
}
if (cells > 1) {
dest[1][0] = 0;
}
s->p += c_len;
return cells;
}




static void fold_line(win_T *wp, long fold_count, foldinfo_T *foldinfo, linenr_T lnum, int row)
{
char_u buf[FOLD_TEXT_LEN];
pos_T *top, *bot;
linenr_T lnume = lnum + fold_count - 1;
int len;
char_u *text;
int fdc;
int col;
int txtcol;
int off;









col = 0;
off = 0;





if (cmdwin_type != 0 && wp == curwin) {
schar_from_ascii(linebuf_char[off], cmdwin_type);
linebuf_attr[off] = win_hl_attr(wp, HLF_AT);
col++;
}

#define RL_MEMSET(p, v, l) do { if (wp->w_p_rl) { for (int ri = 0; ri < l; ri++) { linebuf_attr[off + (wp->w_grid.Columns - (p) - (l)) + ri] = v; } } else { for (int ri = 0; ri < l; ri++) { linebuf_attr[off + (p) + ri] = v; } } } while (0)














fdc = compute_foldcolumn(wp, col);
if (fdc > 0) {
fill_foldcolumn(buf, wp, true, lnum);
const char_u *it = &buf[0];
for (int i = 0; i < fdc; i++) {
int mb_c = mb_ptr2char_adv(&it);
if (wp->w_p_rl) {
schar_from_char(linebuf_char[off + wp->w_grid.Columns - i - 1 - col],
mb_c);
} else {
schar_from_char(linebuf_char[off + col + i], mb_c);
}
}
RL_MEMSET(col, win_hl_attr(wp, HLF_FC), fdc);
col += fdc;
}



RL_MEMSET(col, win_hl_attr(wp, HLF_FL), wp->w_grid.Columns - col);


if (win_signcol_count(wp) > 0) {
len = wp->w_grid.Columns - col;
if (len > 0) {
int len_max = win_signcol_width(wp) * win_signcol_count(wp);
if (len > len_max) {
len = len_max;
}
char_u space_buf[18] = " ";
assert((size_t)len_max <= sizeof(space_buf));
copy_text_attr(off + col, space_buf, len,
win_hl_attr(wp, HLF_FL));
col += len;
}
}




if (wp->w_p_nu || wp->w_p_rnu) {
len = wp->w_grid.Columns - col;
if (len > 0) {
int w = number_width(wp);
long num;
char *fmt = "%*ld ";

if (len > w + 1)
len = w + 1;

if (wp->w_p_nu && !wp->w_p_rnu)

num = (long)lnum;
else {

num = labs((long)get_cursor_rel_lnum(wp, lnum));
if (num == 0 && wp->w_p_nu && wp->w_p_rnu) {


num = lnum;
fmt = "%-*ld ";
}
}

snprintf((char *)buf, FOLD_TEXT_LEN, fmt, w, num);
if (wp->w_p_rl) {

copy_text_attr(off + wp->w_grid.Columns - len - col, buf, len,
win_hl_attr(wp, HLF_FL));
} else {
copy_text_attr(off + col, buf, len, win_hl_attr(wp, HLF_FL));
}
col += len;
}
}




text = get_foldtext(wp, lnum, lnume, foldinfo, buf);

txtcol = col; 




int idx;

if (wp->w_p_rl) {
idx = off;
} else {
idx = off + col;
}

LineState s = LINE_STATE(text);

while (*s.p != NUL) {




int maxcells = wp->w_grid.Columns - col - (wp->w_p_rl ? col : 0);
int cells = line_putchar(&s, &linebuf_char[idx], maxcells, wp->w_p_rl);
if (cells == -1) {
break;
}
col += cells;
idx += cells;
}


if (wp->w_p_rl)
col -= txtcol;

schar_T sc;
schar_from_char(sc, wp->w_p_fcs_chars.fold);
while (col < wp->w_grid.Columns
- (wp->w_p_rl ? txtcol : 0)
) {
schar_copy(linebuf_char[off+col++], sc);
}

if (text != buf)
xfree(text);





if (VIsual_active && wp->w_buffer == curwin->w_buffer) {
if (ltoreq(curwin->w_cursor, VIsual)) {

top = &curwin->w_cursor;
bot = &VIsual;
} else {

top = &VIsual;
bot = &curwin->w_cursor;
}
if (lnum >= top->lnum
&& lnume <= bot->lnum
&& (VIsual_mode != 'v'
|| ((lnum > top->lnum
|| (lnum == top->lnum
&& top->col == 0))
&& (lnume < bot->lnum
|| (lnume == bot->lnum
&& (bot->col - (*p_sel == 'e'))
>= (colnr_T)STRLEN(ml_get_buf(wp->w_buffer, lnume,
FALSE))))))) {
if (VIsual_mode == Ctrl_V) {

if (wp->w_old_cursor_fcol + txtcol < (colnr_T)wp->w_grid.Columns) {
if (wp->w_old_cursor_lcol != MAXCOL
&& wp->w_old_cursor_lcol + txtcol
< (colnr_T)wp->w_grid.Columns) {
len = wp->w_old_cursor_lcol;
} else {
len = wp->w_grid.Columns - txtcol;
}
RL_MEMSET(wp->w_old_cursor_fcol + txtcol, win_hl_attr(wp, HLF_V),
len - (int)wp->w_old_cursor_fcol);
}
} else {

RL_MEMSET(txtcol, win_hl_attr(wp, HLF_V), wp->w_grid.Columns - txtcol);
}
}
}


if (wp->w_p_cc_cols) {
int i = 0;
int j = wp->w_p_cc_cols[i];
int old_txtcol = txtcol;

while (j > -1) {
txtcol += j;
if (wp->w_p_wrap) {
txtcol -= wp->w_skipcol;
} else {
txtcol -= wp->w_leftcol;
}
if (txtcol >= 0 && txtcol < wp->w_grid.Columns) {
linebuf_attr[off + txtcol] =
hl_combine_attr(linebuf_attr[off + txtcol], win_hl_attr(wp, HLF_MC));
}
txtcol = old_txtcol;
j = wp->w_p_cc_cols[++i];
}
}


if (wp->w_p_cuc) {
txtcol += wp->w_virtcol;
if (wp->w_p_wrap)
txtcol -= wp->w_skipcol;
else
txtcol -= wp->w_leftcol;
if (txtcol >= 0 && txtcol < wp->w_grid.Columns) {
linebuf_attr[off + txtcol] = hl_combine_attr(
linebuf_attr[off + txtcol], win_hl_attr(wp, HLF_CUC));
}
}

grid_put_linebuf(&wp->w_grid, row, 0, wp->w_grid.Columns, wp->w_grid.Columns,
false, wp, wp->w_hl_attr_normal, false);





if (wp == curwin
&& lnum <= curwin->w_cursor.lnum
&& lnume >= curwin->w_cursor.lnum) {
curwin->w_cline_row = row;
curwin->w_cline_height = 1;
curwin->w_cline_folded = true;
curwin->w_valid |= (VALID_CHEIGHT|VALID_CROW);
conceal_cursor_used = conceal_cursor_line(curwin);
}
}





static void copy_text_attr(int off, char_u *buf, int len, int attr)
{
int i;

for (i = 0; i < len; i++) {
schar_from_ascii(linebuf_char[off + i], buf[i]);
linebuf_attr[off + i] = attr;
}
}










static size_t
fill_foldcolumn(
char_u *p,
win_T *wp,
int closed,
linenr_T lnum
)
{
int i = 0;
int level;
int first_level;
int fdc = compute_foldcolumn(wp, 0); 
size_t char_counter = 0;
int symbol = 0;
int len = 0;

memset(p, ' ', MAX_MCO * fdc + 1);

level = win_foldinfo.fi_level;



first_level = level - fdc - closed + 1;
if (first_level < 1) {
first_level = 1;
}

for (i = 0; i < MIN(fdc, level); i++) {
if (win_foldinfo.fi_lnum == lnum
&& first_level + i >= win_foldinfo.fi_low_level) {
symbol = wp->w_p_fcs_chars.foldopen;
} else if (first_level == 1) {
symbol = wp->w_p_fcs_chars.foldsep;
} else if (first_level + i <= 9) {
symbol = '0' + first_level + i;
} else {
symbol = '>';
}

len = utf_char2bytes(symbol, &p[char_counter]);
char_counter += len;
if (first_level + i >= level) {
i++;
break;
}
}

if (closed) {
if (symbol != 0) {

char_counter -= len;
memset(&p[char_counter], ' ', len);
}
len = utf_char2bytes(wp->w_p_fcs_chars.foldclosed, &p[char_counter]);
char_counter += len;
}

return MAX(char_counter + (fdc-i), (size_t)fdc);
}








static int
win_line (
win_T *wp,
linenr_T lnum,
int startrow,
int endrow,
bool nochange, 
bool number_only 
)
{
int c = 0; 
long vcol = 0; 
long vcol_sbr = -1; 
long vcol_prev = -1; 
char_u *line; 
char_u *ptr; 
int row; 
ScreenGrid *grid = &wp->w_grid; 

char_u extra[57]; 

int n_extra = 0; 
char_u *p_extra = NULL; 
char_u *p_extra_free = NULL; 
int c_extra = NUL; 
int c_final = NUL; 
int extra_attr = 0; 
static char_u *at_end_str = (char_u *)""; 


int lcs_eol_one = wp->w_p_lcs_chars.eol; 
int lcs_prec_todo = wp->w_p_lcs_chars.prec; 


int saved_n_extra = 0;
char_u *saved_p_extra = NULL;
int saved_c_extra = 0;
int saved_c_final = 0;
int saved_char_attr = 0;

int n_attr = 0; 
int saved_attr2 = 0; 
int n_attr3 = 0; 
int saved_attr3 = 0; 

int n_skip = 0; 

int fromcol = -10; 
int tocol = MAXCOL; 
int fromcol_prev = -2; 
bool noinvcur = false; 
pos_T *top, *bot;
int lnum_in_visual_area = false;
pos_T pos;
long v;

int char_attr = 0; 
int attr_pri = FALSE; 
int area_highlighting = FALSE; 

int attr = 0; 
int area_attr = 0; 
int search_attr = 0; 
int vcol_save_attr = 0; 
int syntax_attr = 0; 
int has_syntax = FALSE; 
int save_did_emsg;
int eol_hl_off = 0; 
int draw_color_col = false; 
int *color_cols = NULL; 
bool has_spell = false; 
#define SPWORDLEN 150
char_u nextline[SPWORDLEN * 2]; 
int nextlinecol = 0; 
int nextline_idx = 0; 

int spell_attr = 0; 
int word_end = 0; 
static linenr_T checked_lnum = 0; 
static int checked_col = 0; 

static int cap_col = -1; 
static linenr_T capcol_lnum = 0; 
int cur_checked_col = 0; 
int extra_check = 0; 
int multi_attr = 0; 
int mb_l = 1; 
int mb_c = 0; 
bool mb_utf8 = false; 
int u8cc[MAX_MCO]; 
int filler_lines; 
int filler_todo; 
hlf_T diff_hlf = (hlf_T)0; 
int change_start = MAXCOL; 
int change_end = -1; 
colnr_T trailcol = MAXCOL; 
bool need_showbreak = false; 
int line_attr = 0; 
int line_attr_lowprio = 0; 
matchitem_T *cur; 
match_T *shl; 
int shl_flag; 

bool prevcol_hl_flag; 


int prev_c = 0; 
int prev_c1 = 0; 

bool search_attr_from_match = false; 
bool has_decorations = false; 
bool do_virttext = false; 


#define WL_START 0 
#define WL_CMDLINE WL_START + 1 
#define WL_FOLD WL_CMDLINE + 1 
#define WL_SIGN WL_FOLD + 1 
#define WL_NR WL_SIGN + 1 
#define WL_BRI WL_NR + 1 
#define WL_SBR WL_BRI + 1 
#define WL_LINE WL_SBR + 1 
int draw_state = WL_START; 

int syntax_flags = 0;
int syntax_seqnr = 0;
int prev_syntax_id = 0;
int conceal_attr = win_hl_attr(wp, HLF_CONCEAL);
int is_concealing = false;
int boguscols = 0; 

int vcol_off = 0; 
int did_wcol = false;
int match_conc = 0; 
int old_boguscols = 0;
#define VCOL_HLC (vcol - vcol_off)
#define FIX_FOR_BOGUSCOLS { n_extra += vcol_off; vcol -= vcol_off; vcol_off = 0; col -= boguscols; old_boguscols = boguscols; boguscols = 0; }









if (startrow > endrow) 
return startrow;

row = startrow;

char *luatext = NULL;

buf_T *buf = wp->w_buffer;

if (!number_only) {


extra_check = wp->w_p_lbr;
if (syntax_present(wp) && !wp->w_s->b_syn_error && !wp->w_s->b_syn_slow) {


save_did_emsg = did_emsg;
did_emsg = false;
syntax_start(wp, lnum);
if (did_emsg) {
wp->w_s->b_syn_error = true;
} else {
did_emsg = save_did_emsg;
if (!wp->w_s->b_syn_slow) {
has_syntax = true;
extra_check = true;
}
}
}

if (decorations_active) {
if (buf->b_luahl && buf->b_luahl_line != LUA_NOREF) {
Error err = ERROR_INIT;
FIXED_TEMP_ARRAY(args, 3);
args.items[0] = WINDOW_OBJ(wp->handle);
args.items[1] = BUFFER_OBJ(buf->handle);
args.items[2] = INTEGER_OBJ(lnum-1);
lua_attr_active = true;
extra_check = true;
Object o = executor_exec_lua_cb(buf->b_luahl_line, "line",
args, true, &err);
lua_attr_active = false;
if (o.type == kObjectTypeString) {


luatext = o.data.string.data;
do_virttext = true;
} else if (ERROR_SET(&err)) {
ELOG("error in luahl line: %s", err.msg);
luatext = err.msg;
do_virttext = true;
}
}

has_decorations = decorations_redraw_line(wp->w_buffer, lnum-1,
&decorations);
if (has_decorations) {
extra_check = true;
}
}


color_cols = wp->w_buffer->terminal ? NULL : wp->w_p_cc_cols;
if (color_cols != NULL) {
draw_color_col = advance_color_col(VCOL_HLC, &color_cols);
}

if (wp->w_p_spell
&& *wp->w_s->b_p_spl != NUL
&& !GA_EMPTY(&wp->w_s->b_langp)
&& *(char **)(wp->w_s->b_langp.ga_data) != NULL) {

has_spell = true;
extra_check = true;




nextline[SPWORDLEN] = NUL;
if (lnum < wp->w_buffer->b_ml.ml_line_count) {
line = ml_get_buf(wp->w_buffer, lnum + 1, false);
spell_cat_line(nextline + SPWORDLEN, line, SPWORDLEN);
}



if (lnum == checked_lnum) {
cur_checked_col = checked_col;
}
checked_lnum = 0;




if (lnum != capcol_lnum) {
cap_col = -1;
}
if (lnum == 1) {
cap_col = 0;
}
capcol_lnum = 0;
}


if (VIsual_active && wp->w_buffer == curwin->w_buffer) {
if (ltoreq(curwin->w_cursor, VIsual)) {

top = &curwin->w_cursor;
bot = &VIsual;
} else {

top = &VIsual;
bot = &curwin->w_cursor;
}
lnum_in_visual_area = (lnum >= top->lnum && lnum <= bot->lnum);
if (VIsual_mode == Ctrl_V) {

if (lnum_in_visual_area) {
fromcol = wp->w_old_cursor_fcol;
tocol = wp->w_old_cursor_lcol;
}
} else {

if (lnum > top->lnum && lnum <= bot->lnum) {
fromcol = 0;
} else if (lnum == top->lnum) {
if (VIsual_mode == 'V') { 
fromcol = 0;
} else {
getvvcol(wp, top, (colnr_T *)&fromcol, NULL, NULL);
if (gchar_pos(top) == NUL) {
tocol = fromcol + 1;
}
}
}
if (VIsual_mode != 'V' && lnum == bot->lnum) {
if (*p_sel == 'e' && bot->col == 0
&& bot->coladd == 0) {
fromcol = -10;
tocol = MAXCOL;
} else if (bot->col == MAXCOL) {
tocol = MAXCOL;
} else {
pos = *bot;
if (*p_sel == 'e') {
getvvcol(wp, &pos, (colnr_T *)&tocol, NULL, NULL);
} else {
getvvcol(wp, &pos, NULL, NULL, (colnr_T *)&tocol);
tocol++;
}
}
}
}


if (!highlight_match && lnum == curwin->w_cursor.lnum && wp == curwin
&& cursor_is_block_during_visual(*p_sel == 'e')) {
noinvcur = true;
}


if (fromcol >= 0) {
area_highlighting = true;
attr = win_hl_attr(wp, HLF_V);
}

} else if (highlight_match
&& wp == curwin
&& lnum >= curwin->w_cursor.lnum
&& lnum <= curwin->w_cursor.lnum + search_match_lines) {
if (lnum == curwin->w_cursor.lnum) {
getvcol(curwin, &(curwin->w_cursor),
(colnr_T *)&fromcol, NULL, NULL);
} else {
fromcol = 0;
}
if (lnum == curwin->w_cursor.lnum + search_match_lines) {
pos.lnum = lnum;
pos.col = search_match_endcol;
getvcol(curwin, &pos, (colnr_T *)&tocol, NULL, NULL);
}

if (fromcol == tocol) {
tocol = fromcol + 1;
}
area_highlighting = true;
attr = win_hl_attr(wp, HLF_I);
}
}

filler_lines = diff_check(wp, lnum);
if (filler_lines < 0) {
if (filler_lines == -1) {
if (diff_find_change(wp, lnum, &change_start, &change_end))
diff_hlf = HLF_ADD; 
else if (change_start == 0)
diff_hlf = HLF_TXD; 
else
diff_hlf = HLF_CHD; 
} else
diff_hlf = HLF_ADD; 
filler_lines = 0;
area_highlighting = TRUE;
}
if (lnum == wp->w_topline)
filler_lines = wp->w_topfill;
filler_todo = filler_lines;


if (lnum == wp->w_cursor.lnum) {


if (wp->w_p_cul && !(wp == curwin && VIsual_active)) {
int cul_attr = win_hl_attr(wp, HLF_CUL);
HlAttrs ae = syn_attr2entry(cul_attr);




if (ae.rgb_fg_color == -1 && ae.cterm_fg_color == 0) {
line_attr_lowprio = cul_attr;
} else {
if (!(State & INSERT) && bt_quickfix(wp->w_buffer)
&& qf_current_entry(wp) == lnum) {
line_attr = hl_combine_attr(cul_attr, line_attr);
} else {
line_attr = cul_attr;
}
}
}

wp->w_last_cursorline = wp->w_cursor.lnum;
}


v = buf_getsigntype(wp->w_buffer, lnum, SIGN_LINEHL, 0, 1);
if (v != 0) {
line_attr = sign_get_attr((int)v, SIGN_LINEHL);
}


if (bt_quickfix(wp->w_buffer) && qf_current_entry(wp) == lnum) {
line_attr = win_hl_attr(wp, HLF_QFL);
}

if (line_attr_lowprio || line_attr) {
area_highlighting = true;
}

line = ml_get_buf(wp->w_buffer, lnum, FALSE);
ptr = line;

if (has_spell && !number_only) {

if (cap_col == 0) {
cap_col = (int)getwhitecols(line);
}




if (nextline[SPWORDLEN] == NUL) {

nextlinecol = MAXCOL;
nextline_idx = 0;
} else {
v = (long)STRLEN(line);
if (v < SPWORDLEN) {


nextlinecol = 0;
memmove(nextline, line, (size_t)v);
STRMOVE(nextline + v, nextline + SPWORDLEN);
nextline_idx = v + 1;
} else {

nextlinecol = v - SPWORDLEN;
memmove(nextline, line + nextlinecol, SPWORDLEN); 
nextline_idx = SPWORDLEN + 1;
}
}
}

if (wp->w_p_list) {
if (curwin->w_p_lcs_chars.space
|| wp->w_p_lcs_chars.trail
|| wp->w_p_lcs_chars.nbsp) {
extra_check = true;
}

if (wp->w_p_lcs_chars.trail) {
trailcol = (colnr_T)STRLEN(ptr);
while (trailcol > (colnr_T)0 && ascii_iswhite(ptr[trailcol - 1])) {
trailcol--;
}
trailcol += (colnr_T) (ptr - line);
}
}





if (wp->w_p_wrap)
v = wp->w_skipcol;
else
v = wp->w_leftcol;
if (v > 0 && !number_only) {
char_u *prev_ptr = ptr;
while (vcol < v && *ptr != NUL) {
c = win_lbr_chartabsize(wp, line, ptr, (colnr_T)vcol, NULL);
vcol += c;
prev_ptr = ptr;
MB_PTR_ADV(ptr);
}







if (vcol < v && (wp->w_p_cuc
|| draw_color_col
|| virtual_active()
|| (VIsual_active && wp->w_buffer == curwin->w_buffer))) {
vcol = v;
}



if (vcol > v) {
vcol -= c;
ptr = prev_ptr;


if (utf_ptr2cells(ptr) >= c || *ptr == TAB) {
n_skip = v - vcol;
}
}





if (tocol <= vcol)
fromcol = 0;
else if (fromcol >= 0 && fromcol < vcol)
fromcol = vcol;


if (wp->w_p_wrap) {
need_showbreak = true;
}


if (has_spell) {
size_t len;
colnr_T linecol = (colnr_T)(ptr - line);
hlf_T spell_hlf = HLF_COUNT;

pos = wp->w_cursor;
wp->w_cursor.lnum = lnum;
wp->w_cursor.col = linecol;
len = spell_move_to(wp, FORWARD, TRUE, TRUE, &spell_hlf);


line = ml_get_buf(wp->w_buffer, lnum, FALSE);
ptr = line + linecol;

if (len == 0 || (int)wp->w_cursor.col > ptr - line) {


spell_hlf = HLF_COUNT;
word_end = (int)(spell_to_word_end(ptr, wp) - line + 1);
} else {

assert(len <= INT_MAX);
word_end = wp->w_cursor.col + (int)len + 1;


if (spell_hlf != HLF_COUNT)
spell_attr = highlight_attr[spell_hlf];
}
wp->w_cursor = pos;


if (has_syntax) {
syntax_start(wp, lnum);
}
}
}





if (fromcol >= 0) {
if (noinvcur) {
if ((colnr_T)fromcol == wp->w_virtcol) {


fromcol_prev = fromcol;
fromcol = -1;
} else if ((colnr_T)fromcol < wp->w_virtcol)

fromcol_prev = wp->w_virtcol;
}
if (fromcol >= tocol)
fromcol = -1;
}





cur = wp->w_match_head;
shl_flag = false;
while ((cur != NULL || !shl_flag) && !number_only) {
if (!shl_flag) {
shl = &search_hl;
shl_flag = true;
} else {
shl = &cur->hl; 
}
shl->startcol = MAXCOL;
shl->endcol = MAXCOL;
shl->attr_cur = 0;
shl->is_addpos = false;
v = (long)(ptr - line);
if (cur != NULL) {
cur->pos.cur = 0;
}
next_search_hl(wp, shl, lnum, (colnr_T)v,
shl == &search_hl ? NULL : cur);
if (wp->w_s->b_syn_slow) {
has_syntax = false;
}



line = ml_get_buf(wp->w_buffer, lnum, false);
ptr = line + v;

if (shl->lnum != 0 && shl->lnum <= lnum) {
if (shl->lnum == lnum) {
shl->startcol = shl->rm.startpos[0].col;
} else {
shl->startcol = 0;
}
if (lnum == shl->lnum + shl->rm.endpos[0].lnum
- shl->rm.startpos[0].lnum) {
shl->endcol = shl->rm.endpos[0].col;
} else {
shl->endcol = MAXCOL;
}

if (shl->startcol == shl->endcol) {
if (line[shl->endcol] != NUL) {
shl->endcol += (*mb_ptr2len)(line + shl->endcol);
} else {
++shl->endcol;
}
}
if ((long)shl->startcol < v) { 
shl->attr_cur = shl->attr;
search_attr = shl->attr;
search_attr_from_match = shl != &search_hl;
}
area_highlighting = true;
}
if (shl != &search_hl && cur != NULL)
cur = cur->next;
}

unsigned off = 0; 
int col = 0; 
if (wp->w_p_rl) {



col = grid->Columns - 1;
off += col;
}


int term_attrs[1024] = {0};
if (wp->w_buffer->terminal) {
terminal_get_line_attributes(wp->w_buffer->terminal, wp, lnum, term_attrs);
extra_check = true;
}

int sign_idx = 0;

for (;; ) {
int has_match_conc = 0; 
bool did_decrement_ptr = false;

if (draw_state != WL_LINE) {
if (draw_state == WL_CMDLINE - 1 && n_extra == 0) {
draw_state = WL_CMDLINE;
if (cmdwin_type != 0 && wp == curwin) {

n_extra = 1;
c_extra = cmdwin_type;
c_final = NUL;
char_attr = win_hl_attr(wp, HLF_AT);
}
}

if (draw_state == WL_FOLD - 1 && n_extra == 0) {
int fdc = compute_foldcolumn(wp, 0);

draw_state = WL_FOLD;
if (fdc > 0) {


xfree(p_extra_free);
p_extra_free = xmalloc(MAX_MCO * fdc + 1);
n_extra = fill_foldcolumn(p_extra_free, wp, false, lnum);
p_extra_free[n_extra] = NUL;
p_extra = p_extra_free;
c_extra = NUL;
c_final = NUL;
char_attr = win_hl_attr(wp, HLF_FC);
}
}


if (draw_state == WL_SIGN - 1 && n_extra == 0) {
draw_state = WL_SIGN;


int count = win_signcol_count(wp);
if (count > 0) {
int text_sign;

c_extra = ' ';
c_final = NUL;
char_attr = win_hl_attr(wp, HLF_SC);
n_extra = win_signcol_width(wp);

if (row == startrow + filler_lines && filler_todo <= 0) {
text_sign = buf_getsigntype(wp->w_buffer, lnum, SIGN_TEXT,
sign_idx, count);
if (text_sign != 0) {
p_extra = sign_get_text(text_sign);
if (p_extra != NULL) {
int symbol_blen = (int)STRLEN(p_extra);

c_extra = NUL;
c_final = NUL;



assert((size_t)win_signcol_width(wp)
>= mb_string2cells(p_extra));

n_extra = symbol_blen +
(win_signcol_width(wp) - mb_string2cells(p_extra));

assert(sizeof(extra) > (size_t)symbol_blen);
memset(extra, ' ', sizeof(extra));
memcpy(extra, p_extra, symbol_blen);

p_extra = extra;
p_extra[n_extra] = NUL;
}
char_attr = sign_get_attr(text_sign, SIGN_TEXT);
}
}

sign_idx++;
if (sign_idx < count) {
draw_state = WL_SIGN - 1;
}
}
}

if (draw_state == WL_NR - 1 && n_extra == 0) {
draw_state = WL_NR;


if ((wp->w_p_nu || wp->w_p_rnu)
&& (row == startrow
+ filler_lines
|| vim_strchr(p_cpo, CPO_NUMCOL) == NULL)) {

if (row == startrow
+ filler_lines
) {
long num;
char *fmt = "%*ld ";

if (wp->w_p_nu && !wp->w_p_rnu)

num = (long)lnum;
else {

num = labs((long)get_cursor_rel_lnum(wp, lnum));
if (num == 0 && wp->w_p_nu && wp->w_p_rnu) {

num = lnum;
fmt = "%-*ld ";
}
}

sprintf((char *)extra, fmt,
number_width(wp), num);
if (wp->w_skipcol > 0)
for (p_extra = extra; *p_extra == ' '; ++p_extra)
*p_extra = '-';
if (wp->w_p_rl) { 

char_u *p2 = skiptowhite(extra) - 1;
for (char_u *p1 = extra; p1 < p2; p1++, p2--) {
const int t = *p1;
*p1 = *p2;
*p2 = t;
}
}
p_extra = extra;
c_extra = NUL;
c_final = NUL;
} else {
c_extra = ' ';
c_final = NUL;
}
n_extra = number_width(wp) + 1;
char_attr = win_hl_attr(wp, HLF_N);

int num_sign = buf_getsigntype(wp->w_buffer, lnum, SIGN_NUMHL,
0, 1);
if (num_sign != 0) {

char_attr = sign_get_attr(num_sign, SIGN_NUMHL);
} else if ((wp->w_p_cul || wp->w_p_rnu)
&& lnum == wp->w_cursor.lnum) {




char_attr = win_hl_attr(wp, HLF_CLN);
}
}
}

if (wp->w_p_brisbr && draw_state == WL_BRI - 1
&& n_extra == 0 && *p_sbr != NUL) {

draw_state = WL_BRI;
} else if (wp->w_p_brisbr && draw_state == WL_SBR && n_extra == 0) {

draw_state = WL_BRI - 1;
}


if (draw_state == WL_BRI - 1 && n_extra == 0) {
draw_state = WL_BRI;

if (wp->w_p_bri && (row != startrow || need_showbreak)
&& filler_lines == 0) {
char_attr = 0;

if (diff_hlf != (hlf_T)0) {
char_attr = win_hl_attr(wp, diff_hlf);
if (wp->w_p_cul && lnum == wp->w_cursor.lnum) {
char_attr = hl_combine_attr(char_attr, win_hl_attr(wp, HLF_CUL));
}
}
p_extra = NULL;
c_extra = ' ';
c_final = NUL;
n_extra =
get_breakindent_win(wp, ml_get_buf(wp->w_buffer, lnum, false));
if (wp->w_skipcol > 0 && wp->w_p_wrap) {
need_showbreak = false;
}


if (tocol == vcol) {
tocol += n_extra;
}
}
}

if (draw_state == WL_SBR - 1 && n_extra == 0) {
draw_state = WL_SBR;
if (filler_todo > 0) {

if (char2cells(wp->w_p_fcs_chars.diff) > 1) {
c_extra = '-';
c_final = NUL;
} else {
c_extra = wp->w_p_fcs_chars.diff;
c_final = NUL;
}
if (wp->w_p_rl) {
n_extra = col + 1;
} else {
n_extra = grid->Columns - col;
}
char_attr = win_hl_attr(wp, HLF_DED);
}
if (*p_sbr != NUL && need_showbreak) {

p_extra = p_sbr;
c_extra = NUL;
c_final = NUL;
n_extra = (int)STRLEN(p_sbr);
char_attr = win_hl_attr(wp, HLF_AT);
if (wp->w_skipcol == 0 || !wp->w_p_wrap) {
need_showbreak = false;
}
vcol_sbr = vcol + MB_CHARLEN(p_sbr);


if (tocol == vcol)
tocol += n_extra;

if (wp->w_p_cul && lnum == wp->w_cursor.lnum) {
char_attr = hl_combine_attr(char_attr, win_hl_attr(wp, HLF_CUL));
}
}
}

if (draw_state == WL_LINE - 1 && n_extra == 0) {
sign_idx = 0;
draw_state = WL_LINE;
if (saved_n_extra) {

n_extra = saved_n_extra;
c_extra = saved_c_extra;
c_final = saved_c_final;
p_extra = saved_p_extra;
char_attr = saved_char_attr;
} else {
char_attr = 0;
}
}
}


if ((dollar_vcol >= 0 && wp == curwin
&& lnum == wp->w_cursor.lnum && vcol >= (long)wp->w_virtcol
&& filler_todo <= 0)
|| (number_only && draw_state > WL_NR)) {
grid_put_linebuf(grid, row, 0, col, -grid->Columns, wp->w_p_rl, wp,
wp->w_hl_attr_normal, false);


if (wp->w_p_cuc) {
row = wp->w_cline_row + wp->w_cline_height;
} else {
row = grid->Rows;
}
break;
}

if (draw_state == WL_LINE && (area_highlighting || has_spell)) {

if (vcol == fromcol
|| (vcol + 1 == fromcol && n_extra == 0
&& utf_ptr2cells(ptr) > 1)
|| ((int)vcol_prev == fromcol_prev
&& vcol_prev < vcol 
&& vcol < tocol)) {
area_attr = attr; 
} else if (area_attr != 0 && (vcol == tocol
|| (noinvcur
&& (colnr_T)vcol == wp->w_virtcol))) {
area_attr = 0; 
}

if (!n_extra) {








v = (long)(ptr - line);
cur = wp->w_match_head;
shl_flag = FALSE;
while (cur != NULL || shl_flag == FALSE) {
if (shl_flag == FALSE
&& ((cur != NULL
&& cur->priority > SEARCH_HL_PRIORITY)
|| cur == NULL)) {
shl = &search_hl;
shl_flag = TRUE;
} else
shl = &cur->hl;
if (cur != NULL) {
cur->pos.cur = 0;
}
bool pos_inprogress = true; 

while (shl->rm.regprog != NULL
|| (cur != NULL && pos_inprogress)) {
if (shl->startcol != MAXCOL
&& v >= (long)shl->startcol
&& v < (long)shl->endcol) {
int tmp_col = v + utfc_ptr2len(ptr);

if (shl->endcol < tmp_col) {
shl->endcol = tmp_col;
}
shl->attr_cur = shl->attr;


if (cur != NULL
&& shl != &search_hl
&& syn_name2id((char_u *)"Conceal") == cur->hlg_id) {
has_match_conc = v == (long)shl->startcol ? 2 : 1;
match_conc = cur->conceal_char;
} else {
has_match_conc = match_conc = 0;
}
} else if (v == (long)shl->endcol) {
shl->attr_cur = 0;

next_search_hl(wp, shl, lnum, (colnr_T)v,
shl == &search_hl ? NULL : cur);
pos_inprogress = !(cur == NULL || cur->pos.cur == 0);



line = ml_get_buf(wp->w_buffer, lnum, FALSE);
ptr = line + v;

if (shl->lnum == lnum) {
shl->startcol = shl->rm.startpos[0].col;
if (shl->rm.endpos[0].lnum == 0)
shl->endcol = shl->rm.endpos[0].col;
else
shl->endcol = MAXCOL;

if (shl->startcol == shl->endcol) {

shl->endcol += (*mb_ptr2len)(line + shl->endcol);
}



continue;
}
}
break;
}
if (shl != &search_hl && cur != NULL)
cur = cur->next;
}



search_attr_from_match = false;
search_attr = search_hl.attr_cur;
cur = wp->w_match_head;
shl_flag = FALSE;
while (cur != NULL || shl_flag == FALSE) {
if (shl_flag == FALSE
&& ((cur != NULL
&& cur->priority > SEARCH_HL_PRIORITY)
|| cur == NULL)) {
shl = &search_hl;
shl_flag = TRUE;
} else
shl = &cur->hl;
if (shl->attr_cur != 0) {
search_attr = shl->attr_cur;
search_attr_from_match = shl != &search_hl;
}
if (shl != &search_hl && cur != NULL)
cur = cur->next;
}

if (*ptr == NUL
&& (wp->w_p_list && lcs_eol_one == -1)) {
search_attr = 0;
}
}

if (diff_hlf != (hlf_T)0) {
if (diff_hlf == HLF_CHD && ptr - line >= change_start
&& n_extra == 0) {
diff_hlf = HLF_TXD; 
}
if (diff_hlf == HLF_TXD && ptr - line > change_end
&& n_extra == 0) {
diff_hlf = HLF_CHD; 
}
line_attr = win_hl_attr(wp, diff_hlf);

if (wp->w_p_cul && lnum == wp->w_cursor.lnum) {
line_attr = 0 != line_attr_lowprio 
? hl_combine_attr(hl_combine_attr(win_hl_attr(wp, HLF_CUL),
line_attr),
hl_get_underline())
: hl_combine_attr(line_attr, win_hl_attr(wp, HLF_CUL));
}
}


attr_pri = true;

if (area_attr != 0) {
char_attr = hl_combine_attr(line_attr, area_attr);
} else if (search_attr != 0) {
char_attr = hl_combine_attr(line_attr, search_attr);
}


else if (line_attr != 0 && ((fromcol == -10 && tocol == MAXCOL)
|| vcol < fromcol || vcol_prev < fromcol_prev
|| vcol >= tocol)) {
char_attr = line_attr;
} else {
attr_pri = false;
if (has_syntax) {
char_attr = syntax_attr;
} else {
char_attr = 0;
}
}
}










if (n_extra > 0) {
if (c_extra != NUL || (n_extra == 1 && c_final != NUL)) {
c = (n_extra == 1 && c_final != NUL) ? c_final : c_extra;
mb_c = c; 
if (utf_char2len(c) > 1) {
mb_utf8 = true;
u8cc[0] = 0;
c = 0xc0;
} else {
mb_utf8 = false;
}
} else {
c = *p_extra;
mb_c = c;


mb_l = utfc_ptr2len(p_extra);
mb_utf8 = false;
if (mb_l > n_extra) {
mb_l = 1;
} else if (mb_l > 1) {
mb_c = utfc_ptr2char(p_extra, u8cc);
mb_utf8 = true;
c = 0xc0;
}
if (mb_l == 0) { 
mb_l = 1;
}


if ((wp->w_p_rl ? (col <= 0) : (col >= grid->Columns - 1))
&& (*mb_char2cells)(mb_c) == 2) {
c = '>';
mb_c = c;
mb_l = 1;
(void)mb_l;
multi_attr = win_hl_attr(wp, HLF_AT);



n_extra++;
p_extra--;
} else {
n_extra -= mb_l - 1;
p_extra += mb_l - 1;
}
p_extra++;
}
n_extra--;
} else {
int c0;

XFREE_CLEAR(p_extra_free);


c0 = c = *ptr;
mb_c = c;


mb_l = utfc_ptr2len(ptr);
mb_utf8 = false;
if (mb_l > 1) {
mb_c = utfc_ptr2char(ptr, u8cc);


if (mb_c < 0x80) {
c0 = c = mb_c;
}
mb_utf8 = true;



if (utf_iscomposing(mb_c)) {
int i;

for (i = MAX_MCO - 1; i > 0; i--) {
u8cc[i] = u8cc[i - 1];
}
u8cc[0] = mb_c;
mb_c = ' ';
}
}

if ((mb_l == 1 && c >= 0x80)
|| (mb_l >= 1 && mb_c == 0)
|| (mb_l > 1 && (!vim_isprintc(mb_c)))) {


transchar_hex((char *)extra, mb_c);
if (wp->w_p_rl) { 
rl_mirror(extra);
}

p_extra = extra;
c = *p_extra;
mb_c = mb_ptr2char_adv((const char_u **)&p_extra);
mb_utf8 = (c >= 0x80);
n_extra = (int)STRLEN(p_extra);
c_extra = NUL;
c_final = NUL;
if (area_attr == 0 && search_attr == 0) {
n_attr = n_extra + 1;
extra_attr = win_hl_attr(wp, HLF_8);
saved_attr2 = char_attr; 
}
} else if (mb_l == 0) { 
mb_l = 1;
} else if (p_arshape && !p_tbidi && arabic_char(mb_c)) {

int pc, pc1, nc;
int pcc[MAX_MCO];



if (wp->w_p_rl) {
pc = prev_c;
pc1 = prev_c1;
nc = utf_ptr2char(ptr + mb_l);
prev_c1 = u8cc[0];
} else {
pc = utfc_ptr2char(ptr + mb_l, pcc);
nc = prev_c;
pc1 = pcc[0];
}
prev_c = mb_c;

mb_c = arabic_shape(mb_c, &c, &u8cc[0], pc, pc1, nc);
} else {
prev_c = mb_c;
}



if ((wp->w_p_rl ? (col <= 0) :
(col >= grid->Columns - 1))
&& (*mb_char2cells)(mb_c) == 2) {
c = '>';
mb_c = c;
mb_utf8 = false;
mb_l = 1;
multi_attr = win_hl_attr(wp, HLF_AT);


ptr--;
did_decrement_ptr = true;
} else if (*ptr != NUL) {
ptr += mb_l - 1;
}



if (n_skip > 0 && mb_l > 1 && n_extra == 0) {
n_extra = 1;
c_extra = MB_FILLER_CHAR;
c_final = NUL;
c = ' ';
if (area_attr == 0 && search_attr == 0) {
n_attr = n_extra + 1;
extra_attr = win_hl_attr(wp, HLF_AT);
saved_attr2 = char_attr; 
}
mb_c = c;
mb_utf8 = false;
mb_l = 1;
}
ptr++;

if (extra_check) {
bool can_spell = true;



v = (long)(ptr - line);
if (has_syntax && v > 0) {


save_did_emsg = did_emsg;
did_emsg = FALSE;

syntax_attr = get_syntax_attr((colnr_T)v - 1,
has_spell ? &can_spell : NULL, false);

if (did_emsg) {
wp->w_s->b_syn_error = TRUE;
has_syntax = FALSE;
} else
did_emsg = save_did_emsg;



line = ml_get_buf(wp->w_buffer, lnum, FALSE);
ptr = line + v;

if (!attr_pri) {
char_attr = syntax_attr;
} else {
char_attr = hl_combine_attr(syntax_attr, char_attr);
}


if (c == NUL) {
syntax_flags = 0;
} else {
syntax_flags = get_syntax_info(&syntax_seqnr);
}
} else if (!attr_pri) {
char_attr = 0;
}





if (has_spell && v >= word_end && v > cur_checked_col) {
spell_attr = 0;
if (!attr_pri) {
char_attr = syntax_attr;
}
if (c != 0 && (!has_syntax || can_spell)) {
char_u *prev_ptr;
char_u *p;
int len;
hlf_T spell_hlf = HLF_COUNT;
prev_ptr = ptr - mb_l;
v -= mb_l - 1;



if ((prev_ptr - line) - nextlinecol >= 0) {
p = nextline + ((prev_ptr - line) - nextlinecol);
} else {
p = prev_ptr;
}
cap_col -= (int)(prev_ptr - line);
size_t tmplen = spell_check(wp, p, &spell_hlf, &cap_col, nochange);
assert(tmplen <= INT_MAX);
len = (int)tmplen;
word_end = v + len;



if (spell_hlf != HLF_COUNT
&& (State & INSERT) != 0
&& wp->w_cursor.lnum == lnum
&& wp->w_cursor.col >=
(colnr_T)(prev_ptr - line)
&& wp->w_cursor.col < (colnr_T)word_end) {
spell_hlf = HLF_COUNT;
spell_redraw_lnum = lnum;
}

if (spell_hlf == HLF_COUNT && p != prev_ptr
&& (p - nextline) + len > nextline_idx) {


checked_lnum = lnum + 1;
checked_col = (int)((p - nextline) + len - nextline_idx);
}


if (spell_hlf != HLF_COUNT)
spell_attr = highlight_attr[spell_hlf];

if (cap_col > 0) {
if (p != prev_ptr
&& (p - nextline) + cap_col >= nextline_idx) {


capcol_lnum = lnum + 1;
cap_col = (int)((p - nextline) + cap_col
- nextline_idx);
} else

cap_col += (int)(prev_ptr - line);
}
}
}
if (spell_attr != 0) {
if (!attr_pri)
char_attr = hl_combine_attr(char_attr, spell_attr);
else
char_attr = hl_combine_attr(spell_attr, char_attr);
}

if (has_decorations && v > 0) {
int extmark_attr = decorations_redraw_col(wp->w_buffer, (colnr_T)v-1,
&decorations);
if (extmark_attr != 0) {
if (!attr_pri) {
char_attr = hl_combine_attr(char_attr, extmark_attr);
} else {
char_attr = hl_combine_attr(extmark_attr, char_attr);
}
}
}

if (wp->w_buffer->terminal) {
char_attr = hl_combine_attr(term_attrs[vcol], char_attr);
}


if (wp->w_p_lbr && c0 == c && vim_isbreak(c)
&& !vim_isbreak((int)(*ptr))) {
int mb_off = utf_head_off(line, ptr - 1);
char_u *p = ptr - (mb_off + 1);

n_extra = win_lbr_chartabsize(wp, line, p, (colnr_T)vcol, NULL) - 1;
if (c == TAB && n_extra + col > grid->Columns) {
n_extra = (int)wp->w_buffer->b_p_ts
- vcol % (int)wp->w_buffer->b_p_ts - 1;
}
c_extra = mb_off > 0 ? MB_FILLER_CHAR : ' ';
c_final = NUL;
if (ascii_iswhite(c)) {
if (c == TAB)

FIX_FOR_BOGUSCOLS;
if (!wp->w_p_list) {
c = ' ';
}
}
}


if (wp->w_p_list
&& (((c == 160
|| (mb_utf8 && (mb_c == 160 || mb_c == 0x202f)))
&& curwin->w_p_lcs_chars.nbsp)
|| (c == ' ' && curwin->w_p_lcs_chars.space
&& ptr - line <= trailcol))) {
c = (c == ' ') ? wp->w_p_lcs_chars.space : wp->w_p_lcs_chars.nbsp;
n_attr = 1;
extra_attr = win_hl_attr(wp, HLF_0);
saved_attr2 = char_attr; 
mb_c = c;
if (utf_char2len(c) > 1) {
mb_utf8 = true;
u8cc[0] = 0;
c = 0xc0;
} else {
mb_utf8 = false;
}
}

if (trailcol != MAXCOL && ptr > line + trailcol && c == ' ') {
c = wp->w_p_lcs_chars.trail;
n_attr = 1;
extra_attr = win_hl_attr(wp, HLF_0);
saved_attr2 = char_attr; 
mb_c = c;
if (utf_char2len(c) > 1) {
mb_utf8 = true;
u8cc[0] = 0;
c = 0xc0;
} else {
mb_utf8 = false;
}
}
}




if (!vim_isprintc(c)) {


if (c == TAB && (!wp->w_p_list || wp->w_p_lcs_chars.tab1)) {
int tab_len = 0;
long vcol_adjusted = vcol; 


if (*p_sbr != NUL && vcol == vcol_sbr && wp->w_p_wrap) {
vcol_adjusted = vcol - MB_CHARLEN(p_sbr);
}

tab_len = (int)wp->w_buffer->b_p_ts
- vcol_adjusted % (int)wp->w_buffer->b_p_ts - 1;

if (!wp->w_p_lbr || !wp->w_p_list) {
n_extra = tab_len;
} else {
char_u *p;
int i;
int saved_nextra = n_extra;

if (vcol_off > 0) {

tab_len += vcol_off;
}

if (wp->w_p_lcs_chars.tab1 && old_boguscols > 0
&& n_extra > tab_len) {
tab_len += n_extra - tab_len;
}




int len = (tab_len * mb_char2len(wp->w_p_lcs_chars.tab2));
if (n_extra > 0) {
len += n_extra - tab_len;
}
c = wp->w_p_lcs_chars.tab1;
p = xmalloc(len + 1);
memset(p, ' ', len);
p[len] = NUL;
xfree(p_extra_free);
p_extra_free = p;
for (i = 0; i < tab_len; i++) {
int lcs = wp->w_p_lcs_chars.tab2;



if (wp->w_p_lcs_chars.tab3 && i == tab_len - 1) {
lcs = wp->w_p_lcs_chars.tab3;
}
utf_char2bytes(lcs, p);
p += mb_char2len(lcs);
n_extra += mb_char2len(lcs) - (saved_nextra > 0 ? 1 : 0);
}
p_extra = p_extra_free;



if (vcol_off > 0) {
n_extra -= vcol_off;
}
}

{
int vc_saved = vcol_off;







FIX_FOR_BOGUSCOLS;




if (n_extra == tab_len + vc_saved && wp->w_p_list
&& wp->w_p_lcs_chars.tab1) {
tab_len += vc_saved;
}
}

mb_utf8 = false; 
if (wp->w_p_list) {
c = (n_extra == 0 && wp->w_p_lcs_chars.tab3)
? wp->w_p_lcs_chars.tab3
: wp->w_p_lcs_chars.tab1;
if (wp->w_p_lbr) {
c_extra = NUL; 
} else {
c_extra = wp->w_p_lcs_chars.tab2;
}
c_final = wp->w_p_lcs_chars.tab3;
n_attr = tab_len + 1;
extra_attr = win_hl_attr(wp, HLF_0);
saved_attr2 = char_attr; 
mb_c = c;
if (utf_char2len(c) > 1) {
mb_utf8 = true;
u8cc[0] = 0;
c = 0xc0;
}
} else {
c_final = NUL;
c_extra = ' ';
c = ' ';
}
} else if (c == NUL
&& (wp->w_p_list
|| ((fromcol >= 0 || fromcol_prev >= 0)
&& tocol > vcol
&& VIsual_mode != Ctrl_V
&& (wp->w_p_rl ? (col >= 0) : (col < grid->Columns))
&& !(noinvcur
&& lnum == wp->w_cursor.lnum
&& (colnr_T)vcol == wp->w_virtcol)))
&& lcs_eol_one > 0) {



if (diff_hlf == (hlf_T)0
&& line_attr == 0
&& line_attr_lowprio == 0) {

if (area_highlighting && virtual_active()
&& tocol != MAXCOL && vcol < tocol) {
n_extra = 0;
} else {
p_extra = at_end_str;
n_extra = 1;
c_extra = NUL;
c_final = NUL;
}
}
if (wp->w_p_list && wp->w_p_lcs_chars.eol > 0) {
c = wp->w_p_lcs_chars.eol;
} else {
c = ' ';
}
lcs_eol_one = -1;
ptr--; 
extra_attr = win_hl_attr(wp, HLF_AT);
n_attr = 1;
mb_c = c;
if (utf_char2len(c) > 1) {
mb_utf8 = true;
u8cc[0] = 0;
c = 0xc0;
} else {
mb_utf8 = false; 
}
} else if (c != NUL) {
p_extra = transchar(c);
if (n_extra == 0) {
n_extra = byte2cells(c) - 1;
}
if ((dy_flags & DY_UHEX) && wp->w_p_rl)
rl_mirror(p_extra); 
c_extra = NUL;
c_final = NUL;
if (wp->w_p_lbr) {
char_u *p;

c = *p_extra;
p = xmalloc(n_extra + 1);
memset(p, ' ', n_extra);
STRNCPY(p, p_extra + 1, STRLEN(p_extra) - 1);
p[n_extra] = NUL;
xfree(p_extra_free);
p_extra_free = p_extra = p;
} else {
n_extra = byte2cells(c) - 1;
c = *p_extra++;
}
n_attr = n_extra + 1;
extra_attr = win_hl_attr(wp, HLF_8);
saved_attr2 = char_attr; 
mb_utf8 = false; 
} else if (VIsual_active
&& (VIsual_mode == Ctrl_V || VIsual_mode == 'v')
&& virtual_active()
&& tocol != MAXCOL
&& vcol < tocol
&& (wp->w_p_rl ? (col >= 0) : (col < grid->Columns))) {
c = ' ';
ptr--; 
}
}

if (wp->w_p_cole > 0
&& (wp != curwin || lnum != wp->w_cursor.lnum
|| conceal_cursor_line(wp))
&& ((syntax_flags & HL_CONCEAL) != 0 || has_match_conc > 0)
&& !(lnum_in_visual_area
&& vim_strchr(wp->w_p_cocu, 'v') == NULL)) {
char_attr = conceal_attr;
if ((prev_syntax_id != syntax_seqnr || has_match_conc > 1)
&& (syn_get_sub_char() != NUL || match_conc
|| wp->w_p_cole == 1)
&& wp->w_p_cole != 3) {


if (match_conc) {
c = match_conc;
} else if (syn_get_sub_char() != NUL) {
c = syn_get_sub_char();
} else if (wp->w_p_lcs_chars.conceal != NUL) {
c = wp->w_p_lcs_chars.conceal;
} else {
c = ' ';
}

prev_syntax_id = syntax_seqnr;

if (n_extra > 0)
vcol_off += n_extra;
vcol += n_extra;
if (wp->w_p_wrap && n_extra > 0) {
if (wp->w_p_rl) {
col -= n_extra;
boguscols -= n_extra;
} else {
boguscols += n_extra;
col += n_extra;
}
}
n_extra = 0;
n_attr = 0;
} else if (n_skip == 0) {
is_concealing = TRUE;
n_skip = 1;
}
mb_c = c;
if (utf_char2len(c) > 1) {
mb_utf8 = true;
u8cc[0] = 0;
c = 0xc0;
} else {
mb_utf8 = false; 
}
} else {
prev_syntax_id = 0;
is_concealing = FALSE;
}

if (n_skip > 0 && did_decrement_ptr) {

ptr++;
}
}



if (!did_wcol && draw_state == WL_LINE
&& wp == curwin && lnum == wp->w_cursor.lnum
&& conceal_cursor_line(wp)
&& (int)wp->w_virtcol <= vcol + n_skip) {
if (wp->w_p_rl) {
wp->w_wcol = grid->Columns - col + boguscols - 1;
} else {
wp->w_wcol = col - boguscols;
}
wp->w_wrow = row;
did_wcol = true;
curwin->w_valid |= VALID_WCOL|VALID_WROW|VALID_VIRTCOL;
}


if (n_attr > 0 && draw_state == WL_LINE && !search_attr_from_match) {
char_attr = hl_combine_attr(char_attr, extra_attr);
}




if (lcs_prec_todo != NUL
&& wp->w_p_list
&& (wp->w_p_wrap ? (wp->w_skipcol > 0 && row == 0) : wp->w_leftcol > 0)
&& filler_todo <= 0
&& draw_state > WL_NR
&& c != NUL) {
c = wp->w_p_lcs_chars.prec;
lcs_prec_todo = NUL;
if ((*mb_char2cells)(mb_c) > 1) {


c_extra = MB_FILLER_CHAR;
c_final = NUL;
n_extra = 1;
n_attr = 2;
extra_attr = win_hl_attr(wp, HLF_AT);
}
mb_c = c;
if (utf_char2len(c) > 1) {
mb_utf8 = true;
u8cc[0] = 0;
c = 0xc0;
} else {
mb_utf8 = false; 
}
saved_attr3 = char_attr; 
char_attr = win_hl_attr(wp, HLF_AT); 
n_attr3 = 1;
}


if (c == NUL) {
long prevcol = (long)(ptr - line) - 1;


if ((long)(wp->w_p_wrap ? wp->w_skipcol : wp->w_leftcol) > prevcol) {
prevcol++;
}





prevcol_hl_flag = false;
if (!search_hl.is_addpos && prevcol == (long)search_hl.startcol) {
prevcol_hl_flag = true;
} else {
cur = wp->w_match_head;
while (cur != NULL) {
if (!cur->hl.is_addpos && prevcol == (long)cur->hl.startcol) {
prevcol_hl_flag = true;
break;
}
cur = cur->next;
}
}
if (wp->w_p_lcs_chars.eol == lcs_eol_one
&& ((area_attr != 0 && vcol == fromcol
&& (VIsual_mode != Ctrl_V
|| lnum == VIsual.lnum
|| lnum == curwin->w_cursor.lnum))

|| prevcol_hl_flag)) {
int n = 0;

if (wp->w_p_rl) {
if (col < 0)
n = 1;
} else {
if (col >= grid->Columns) {
n = -1;
}
}
if (n != 0) {


off += n;
col += n;
} else {

schar_from_ascii(linebuf_char[off], ' ');
}
if (area_attr == 0) {


char_attr = search_hl.attr;
cur = wp->w_match_head;
shl_flag = FALSE;
while (cur != NULL || shl_flag == FALSE) {
if (shl_flag == FALSE
&& ((cur != NULL
&& cur->priority > SEARCH_HL_PRIORITY)
|| cur == NULL)) {
shl = &search_hl;
shl_flag = TRUE;
} else
shl = &cur->hl;
if ((ptr - line) - 1 == (long)shl->startcol
&& (shl == &search_hl || !shl->is_addpos)) {
char_attr = shl->attr;
}
if (shl != &search_hl && cur != NULL) {
cur = cur->next;
}
}
}

int eol_attr = char_attr;
if (wp->w_p_cul && lnum == wp->w_cursor.lnum) {
eol_attr = hl_combine_attr(win_hl_attr(wp, HLF_CUL), eol_attr);
}
linebuf_attr[off] = eol_attr;
if (wp->w_p_rl) {
--col;
--off;
} else {
++col;
++off;
}
++vcol;
eol_hl_off = 1;
}

if (wp->w_p_wrap) {
v = wp->w_skipcol;
} else {
v = wp->w_leftcol;
}


if (vcol < v + col - win_col_off(wp))
vcol = v + col - win_col_off(wp);


col -= boguscols;


if (draw_color_col)
draw_color_col = advance_color_col(VCOL_HLC, &color_cols);

VirtText virt_text = KV_INITIAL_VALUE;
if (luatext) {
kv_push(virt_text, ((VirtTextChunk){ .text = luatext, .hl_id = 0 }));
do_virttext = true;
} else if (has_decorations) {
VirtText *vp = decorations_redraw_virt_text(wp->w_buffer, &decorations);
if (vp) {
virt_text = *vp;
do_virttext = true;
}
}

if (((wp->w_p_cuc
&& (int)wp->w_virtcol >= VCOL_HLC - eol_hl_off
&& (int)wp->w_virtcol <
grid->Columns * (row - startrow + 1) + v
&& lnum != wp->w_cursor.lnum)
|| draw_color_col || line_attr_lowprio || line_attr
|| diff_hlf != (hlf_T)0 || do_virttext)) {
int rightmost_vcol = 0;
int i;

size_t virt_pos = 0;
LineState s = LINE_STATE((char_u *)"");
int virt_attr = 0;



bool delay_virttext = wp->w_p_lcs_chars.eol == lcs_eol_one
&& eol_hl_off == 0;

if (wp->w_p_cuc) {
rightmost_vcol = wp->w_virtcol;
}

if (draw_color_col) {

for (i = 0; color_cols[i] >= 0; i++) {
if (rightmost_vcol < color_cols[i]) {
rightmost_vcol = color_cols[i];
}
}
}

int cuc_attr = win_hl_attr(wp, HLF_CUC);
int mc_attr = win_hl_attr(wp, HLF_MC);

int diff_attr = 0;
if (diff_hlf == HLF_TXD) {
diff_hlf = HLF_CHD;
}
if (diff_hlf != 0) {
diff_attr = win_hl_attr(wp, diff_hlf);
}

int base_attr = hl_combine_attr(line_attr_lowprio, diff_attr);
if (base_attr || line_attr) {
rightmost_vcol = INT_MAX;
}

int col_stride = wp->w_p_rl ? -1 : 1;

while (wp->w_p_rl ? col >= 0 : col < grid->Columns) {
int cells = -1;
if (do_virttext && !delay_virttext) {
if (*s.p == NUL) {
if (virt_pos < virt_text.size) {
s.p = (char_u *)kv_A(virt_text, virt_pos).text;
int hl_id = kv_A(virt_text, virt_pos).hl_id;
virt_attr = hl_id > 0 ? syn_id2attr(hl_id) : 0;
virt_pos++;
} else {
do_virttext = false;
}
}
if (*s.p != NUL) {
cells = line_putchar(&s, &linebuf_char[off], grid->Columns - col,
false);
}
}
delay_virttext = false;

if (cells == -1) {
schar_from_ascii(linebuf_char[off], ' ');
cells = 1;
}
col += cells * col_stride;
if (draw_color_col) {
draw_color_col = advance_color_col(VCOL_HLC, &color_cols);
}

int col_attr = base_attr;

if (wp->w_p_cuc && VCOL_HLC == (long)wp->w_virtcol) {
col_attr = cuc_attr;
} else if (draw_color_col && VCOL_HLC == *color_cols) {
col_attr = mc_attr;
}

if (do_virttext) {
col_attr = hl_combine_attr(col_attr, virt_attr);
}

col_attr = hl_combine_attr(col_attr, line_attr);

linebuf_attr[off] = col_attr;
if (cells == 2) {
linebuf_attr[off+1] = col_attr;
}
off += cells * col_stride;

if (VCOL_HLC >= rightmost_vcol && *s.p == NUL
&& virt_pos >= virt_text.size) {
break;
}

vcol += cells;
}
}


if (wp->w_buffer->terminal) {


int n = wp->w_p_rl ? -1 : 1;
while (col >= 0 && col < grid->Columns) {
schar_from_ascii(linebuf_char[off], ' ');
linebuf_attr[off] = term_attrs[vcol];
off += n;
vcol += n;
col += n;
}
}
grid_put_linebuf(grid, row, 0, col, grid->Columns, wp->w_p_rl, wp,
wp->w_hl_attr_normal, false);
row++;





if (wp == curwin && lnum == curwin->w_cursor.lnum) {
curwin->w_cline_row = startrow;
curwin->w_cline_height = row - startrow;
curwin->w_cline_folded = false;
curwin->w_valid |= (VALID_CHEIGHT|VALID_CROW);
conceal_cursor_used = conceal_cursor_line(curwin);
}

break;
}



if (wp->w_p_lcs_chars.ext != NUL
&& wp->w_p_list
&& !wp->w_p_wrap
&& filler_todo <= 0
&& (wp->w_p_rl ? col == 0 : col == grid->Columns - 1)
&& (*ptr != NUL
|| lcs_eol_one > 0
|| (n_extra && (c_extra != NUL || *p_extra != NUL)))) {
c = wp->w_p_lcs_chars.ext;
char_attr = win_hl_attr(wp, HLF_AT);
mb_c = c;
if (utf_char2len(c) > 1) {
mb_utf8 = true;
u8cc[0] = 0;
c = 0xc0;
} else {
mb_utf8 = false;
}
}


if (draw_color_col) {
draw_color_col = advance_color_col(VCOL_HLC, &color_cols);
}





vcol_save_attr = -1;
if (draw_state == WL_LINE && !lnum_in_visual_area
&& search_attr == 0 && area_attr == 0) {
if (wp->w_p_cuc && VCOL_HLC == (long)wp->w_virtcol
&& lnum != wp->w_cursor.lnum) {
vcol_save_attr = char_attr;
char_attr = hl_combine_attr(win_hl_attr(wp, HLF_CUC), char_attr);
} else if (draw_color_col && VCOL_HLC == *color_cols) {
vcol_save_attr = char_attr;
char_attr = hl_combine_attr(win_hl_attr(wp, HLF_MC), char_attr);
}
}


if (draw_state == WL_LINE) {
char_attr = hl_combine_attr(line_attr_lowprio, char_attr);
}



vcol_prev = vcol;
if (draw_state < WL_LINE || n_skip <= 0) {



if (wp->w_p_rl && (*mb_char2cells)(mb_c) > 1) {

off--;
col--;
}
if (mb_utf8) {
schar_from_cc(linebuf_char[off], mb_c, u8cc);
} else {
schar_from_ascii(linebuf_char[off], c);
}
if (multi_attr) {
linebuf_attr[off] = multi_attr;
multi_attr = 0;
} else {
linebuf_attr[off] = char_attr;
}

if ((*mb_char2cells)(mb_c) > 1) {

off++;
col++;

linebuf_char[off][0] = 0;
if (draw_state > WL_NR && filler_todo <= 0) {
vcol++;
}


if (tocol == vcol) {
tocol++;
}
if (wp->w_p_rl) {

--off;
--col;
}
}
if (wp->w_p_rl) {
--off;
--col;
} else {
++off;
++col;
}
} else if (wp->w_p_cole > 0 && is_concealing) {
--n_skip;
++vcol_off;
if (n_extra > 0)
vcol_off += n_extra;
if (wp->w_p_wrap) {













if (n_extra > 0) {
vcol += n_extra;
if (wp->w_p_rl) {
col -= n_extra;
boguscols -= n_extra;
} else {
col += n_extra;
boguscols += n_extra;
}
n_extra = 0;
n_attr = 0;
}


if ((*mb_char2cells)(mb_c) > 1) {

if (wp->w_p_rl) {
--boguscols;
--col;
} else {
++boguscols;
++col;
}
}

if (wp->w_p_rl) {
--boguscols;
--col;
} else {
++boguscols;
++col;
}
} else {
if (n_extra > 0) {
vcol += n_extra;
n_extra = 0;
n_attr = 0;
}
}

} else
--n_skip;



if (draw_state > WL_NR
&& filler_todo <= 0
)
++vcol;

if (vcol_save_attr >= 0)
char_attr = vcol_save_attr;


if (draw_state > WL_NR && n_attr3 > 0 && --n_attr3 == 0)
char_attr = saved_attr3;


if (n_attr > 0 && draw_state == WL_LINE && --n_attr == 0)
char_attr = saved_attr2;





if ((wp->w_p_rl ? (col < 0) : (col >= grid->Columns))
&& (*ptr != NUL
|| filler_todo > 0
|| (wp->w_p_list && wp->w_p_lcs_chars.eol != NUL
&& p_extra != at_end_str)
|| (n_extra != 0 && (c_extra != NUL || *p_extra != NUL)))
) {
bool wrap = wp->w_p_wrap 
&& filler_todo <= 0 
&& lcs_eol_one != -1 
&& row != endrow - 1 
&& (grid->Columns == Columns 
|| ui_has(kUIMultigrid)) 
&& !wp->w_p_rl; 
grid_put_linebuf(grid, row, 0, col - boguscols, grid->Columns, wp->w_p_rl,
wp, wp->w_hl_attr_normal, wrap);
if (wrap) {
ScreenGrid *current_grid = grid;
int current_row = row, dummy_col = 0; 
screen_adjust_grid(&current_grid, &current_row, &dummy_col);


current_grid->attrs[current_grid->line_offset[current_row+1]] = -1;


current_grid->line_wraps[current_row] = true;
}

boguscols = 0;
row++;



if ((!wp->w_p_wrap
&& filler_todo <= 0
) || lcs_eol_one == -1)
break;


if (draw_state != WL_LINE && filler_todo <= 0) {
win_draw_end(wp, '@', ' ', true, row, wp->w_grid.Rows, HLF_AT);
row = endrow;
}


if (row == endrow) {
++row;
break;
}

col = 0;
off = 0;
if (wp->w_p_rl) {
col = grid->Columns - 1; 
off += col;
}


draw_state = WL_START;
saved_n_extra = n_extra;
saved_p_extra = p_extra;
saved_c_extra = c_extra;
saved_c_final = c_final;
saved_char_attr = char_attr;
n_extra = 0;
lcs_prec_todo = wp->w_p_lcs_chars.prec;
if (filler_todo <= 0) {
need_showbreak = true;
}
filler_todo--;


if (filler_todo == 0 && wp->w_botfill) {
break;
}
}

} 


if (*skipwhite(line) == NUL) {
capcol_lnum = lnum + 1;
cap_col = 0;
}

xfree(p_extra_free);
xfree(luatext);
return row;
}











void screen_adjust_grid(ScreenGrid **grid, int *row_off, int *col_off)
{
if (!(*grid)->chars && *grid != &default_grid) {
*row_off += (*grid)->row_offset;
*col_off += (*grid)->col_offset;
if (*grid == &msg_grid_adj && msg_grid.chars) {
*grid = &msg_grid;
} else {
*grid = &default_grid;
}
}
}









static int grid_char_needs_redraw(ScreenGrid *grid, int off_from, int off_to,
int cols)
{
return (cols > 0
&& ((schar_cmp(linebuf_char[off_from], grid->chars[off_to])
|| linebuf_attr[off_from] != grid->attrs[off_to]
|| (line_off2cells(linebuf_char, off_from, off_from + cols) > 1
&& schar_cmp(linebuf_char[off_from + 1],
grid->chars[off_to + 1])))
|| rdb_flags & RDB_NODELTA));
}












static void grid_put_linebuf(ScreenGrid *grid, int row, int coloff, int endcol,
int clear_width, int rlflag, win_T *wp,
int bg_attr, bool wrap)
{
unsigned off_from;
unsigned off_to;
unsigned max_off_from;
unsigned max_off_to;
int col = 0;
bool redraw_this; 
bool redraw_next; 
bool clear_next = false;
int char_cells; 

int start_dirty = -1, end_dirty = 0;



if (row >= grid->Rows) {
row = grid->Rows - 1;
}
if (endcol > grid->Columns) {
endcol = grid->Columns;
}

screen_adjust_grid(&grid, &row, &coloff);


if (grid->chars == NULL || row >= grid->Rows || coloff >= grid->Columns) {
DLOG("invalid state, skipped");
return;
}

off_from = 0;
off_to = grid->line_offset[row] + coloff;
max_off_from = linebuf_size;
max_off_to = grid->line_offset[row] + grid->Columns;

if (rlflag) {

if (clear_width > 0) {
while (col <= endcol && grid->chars[off_to][0] == ' '
&& grid->chars[off_to][1] == NUL
&& grid->attrs[off_to] == bg_attr
) {
++off_to;
++col;
}
if (col <= endcol) {
grid_fill(grid, row, row + 1, col + coloff, endcol + coloff + 1,
' ', ' ', bg_attr);
}
}
col = endcol + 1;
off_to = grid->line_offset[row] + col + coloff;
off_from += col;
endcol = (clear_width > 0 ? clear_width : -clear_width);
}

if (bg_attr) {
for (int c = col; c < endcol; c++) {
linebuf_attr[off_from+c] =
hl_combine_attr(bg_attr, linebuf_attr[off_from+c]);
}
}

redraw_next = grid_char_needs_redraw(grid, off_from, off_to, endcol - col);

while (col < endcol) {
char_cells = 1;
if (col + 1 < endcol) {
char_cells = line_off2cells(linebuf_char, off_from, max_off_from);
}
redraw_this = redraw_next;
redraw_next = grid_char_needs_redraw(grid, off_from + char_cells,
off_to + char_cells,
endcol - col - char_cells);

if (redraw_this) {
if (start_dirty == -1) {
start_dirty = col;
}
end_dirty = col + char_cells;





if (col + char_cells == endcol
&& ((char_cells == 1
&& grid_off2cells(grid, off_to, max_off_to) > 1)
|| (char_cells == 2
&& grid_off2cells(grid, off_to, max_off_to) == 1
&& grid_off2cells(grid, off_to + 1, max_off_to) > 1))) {
clear_next = true;
}

schar_copy(grid->chars[off_to], linebuf_char[off_from]);
if (char_cells == 2) {
schar_copy(grid->chars[off_to+1], linebuf_char[off_from+1]);
}

grid->attrs[off_to] = linebuf_attr[off_from];


if (char_cells == 2) {
grid->attrs[off_to + 1] = linebuf_attr[off_from];
}
}

off_to += char_cells;
off_from += char_cells;
col += char_cells;
}

if (clear_next) {


schar_from_ascii(grid->chars[off_to], ' ');
end_dirty++;
}

int clear_end = -1;
if (clear_width > 0 && !rlflag) {


while (col < clear_width) {
if (grid->chars[off_to][0] != ' '
|| grid->chars[off_to][1] != NUL
|| grid->attrs[off_to] != bg_attr) {
grid->chars[off_to][0] = ' ';
grid->chars[off_to][1] = NUL;
grid->attrs[off_to] = bg_attr;
if (start_dirty == -1) {
start_dirty = col;
end_dirty = col;
} else if (clear_end == -1) {
end_dirty = endcol;
}
clear_end = col+1;
}
col++;
off_to++;
}
}

if (clear_width > 0 || wp->w_width != grid->Columns) {


grid->line_wraps[row] = false;
}

if (clear_end < end_dirty) {
clear_end = end_dirty;
}
if (start_dirty == -1) {
start_dirty = end_dirty;
}
if (clear_end > start_dirty) {
ui_line(grid, row, coloff+start_dirty, coloff+end_dirty, coloff+clear_end,
bg_attr, wrap);
}
}





void rl_mirror(char_u *str)
{
char_u *p1, *p2;
int t;

for (p1 = str, p2 = str + STRLEN(str) - 1; p1 < p2; ++p1, --p2) {
t = *p1;
*p1 = *p2;
*p2 = t;
}
}




void status_redraw_all(void)
{

FOR_ALL_WINDOWS_IN_TAB(wp, curtab) {
if (wp->w_status_height) {
wp->w_redr_status = TRUE;
redraw_later(VALID);
}
}
}


void status_redraw_curbuf(void)
{
status_redraw_buf(curbuf);
}


void status_redraw_buf(buf_T *buf)
{
FOR_ALL_WINDOWS_IN_TAB(wp, curtab) {
if (wp->w_status_height != 0 && wp->w_buffer == buf) {
wp->w_redr_status = true;
redraw_later(VALID);
}
}
}




void redraw_statuslines(void)
{
FOR_ALL_WINDOWS_IN_TAB(wp, curtab) {
if (wp->w_redr_status) {
win_redr_status(wp);
}
}
if (redraw_tabline)
draw_tabline();
}




void win_redraw_last_status(const frame_T *frp)
FUNC_ATTR_NONNULL_ARG(1)
{
if (frp->fr_layout == FR_LEAF) {
frp->fr_win->w_redr_status = true;
} else if (frp->fr_layout == FR_ROW) {
FOR_ALL_FRAMES(frp, frp->fr_child) {
win_redraw_last_status(frp);
}
} else {
assert(frp->fr_layout == FR_COL);
frp = frp->fr_child;
while (frp->fr_next != NULL) {
frp = frp->fr_next;
}
win_redraw_last_status(frp);
}
}




static void draw_vsep_win(win_T *wp, int row)
{
int hl;
int c;

if (wp->w_vsep_width) {

c = fillchar_vsep(wp, &hl);
grid_fill(&default_grid, wp->w_winrow + row, W_ENDROW(wp),
W_ENDCOL(wp), W_ENDCOL(wp) + 1, c, ' ', hl);
}
}





static int status_match_len(expand_T *xp, char_u *s)
{
int len = 0;

int emenu = (xp->xp_context == EXPAND_MENUS
|| xp->xp_context == EXPAND_MENUNAMES);


if (emenu && menu_is_separator(s))
return 1;

while (*s != NUL) {
s += skip_status_match_char(xp, s);
len += ptr2cells(s);
MB_PTR_ADV(s);
}

return len;
}





static int skip_status_match_char(expand_T *xp, char_u *s)
{
if ((rem_backslash(s) && xp->xp_context != EXPAND_HELP)
|| ((xp->xp_context == EXPAND_MENUS
|| xp->xp_context == EXPAND_MENUNAMES)
&& (s[0] == '\t' || (s[0] == '\\' && s[1] != NUL)))
) {
#if !defined(BACKSLASH_IN_FILENAME)
if (xp->xp_shell && csh_like_shell() && s[1] == '\\' && s[2] == '!')
return 2;
#endif
return 1;
}
return 0;
}








void
win_redr_status_matches (
expand_T *xp,
int num_matches,
char_u **matches, 
int match,
int showtail
)
{
#define L_MATCH(m) (showtail ? sm_gettail(matches[m], false) : matches[m])
int row;
char_u *buf;
int len;
int clen; 
int fillchar;
int attr;
int i;
int highlight = TRUE;
char_u *selstart = NULL;
int selstart_col = 0;
char_u *selend = NULL;
static int first_match = 0;
int add_left = FALSE;
char_u *s;
int emenu;
int l;

if (matches == NULL) 
return;

buf = xmalloc(Columns * MB_MAXBYTES + 1);

if (match == -1) { 
match = 0;
highlight = FALSE;
}

clen = status_match_len(xp, L_MATCH(match)) + 3;
if (match == 0)
first_match = 0;
else if (match < first_match) {

first_match = match;
add_left = TRUE;
} else {

for (i = first_match; i < match; ++i)
clen += status_match_len(xp, L_MATCH(i)) + 2;
if (first_match > 0)
clen += 2;

if ((long)clen > Columns) {
first_match = match;

clen = 2;
for (i = match; i < num_matches; ++i) {
clen += status_match_len(xp, L_MATCH(i)) + 2;
if ((long)clen >= Columns) {
break;
}
}
if (i == num_matches)
add_left = TRUE;
}
}
if (add_left)
while (first_match > 0) {
clen += status_match_len(xp, L_MATCH(first_match - 1)) + 2;
if ((long)clen >= Columns) {
break;
}
first_match--;
}

fillchar = fillchar_status(&attr, curwin);

if (first_match == 0) {
*buf = NUL;
len = 0;
} else {
STRCPY(buf, "< ");
len = 2;
}
clen = len;

i = first_match;
while ((long)(clen + status_match_len(xp, L_MATCH(i)) + 2) < Columns) {
if (i == match) {
selstart = buf + len;
selstart_col = clen;
}

s = L_MATCH(i);

emenu = (xp->xp_context == EXPAND_MENUS
|| xp->xp_context == EXPAND_MENUNAMES);
if (emenu && menu_is_separator(s)) {
STRCPY(buf + len, transchar('|'));
l = (int)STRLEN(buf + len);
len += l;
clen += l;
} else
for (; *s != NUL; ++s) {
s += skip_status_match_char(xp, s);
clen += ptr2cells(s);
if ((l = (*mb_ptr2len)(s)) > 1) {
STRNCPY(buf + len, s, l); 
s += l - 1;
len += l;
} else {
STRCPY(buf + len, transchar_byte(*s));
len += (int)STRLEN(buf + len);
}
}
if (i == match)
selend = buf + len;

*(buf + len++) = ' ';
*(buf + len++) = ' ';
clen += 2;
if (++i == num_matches)
break;
}

if (i != num_matches) {
*(buf + len++) = '>';
++clen;
}

buf[len] = NUL;

row = cmdline_row - 1;
if (row >= 0) {
if (wild_menu_showing == 0 || wild_menu_showing == WM_LIST) {
if (msg_scrolled > 0) {


if (cmdline_row == Rows - 1) {
msg_scroll_up(false);
msg_scrolled++;
} else {
cmdline_row++;
row++;
}
wild_menu_showing = WM_SCROLLED;
} else {



if (lastwin->w_status_height == 0) {
save_p_ls = p_ls;
save_p_wmh = p_wmh;
p_ls = 2;
p_wmh = 0;
last_status(FALSE);
}
wild_menu_showing = WM_SHOWN;
}
}



ScreenGrid *grid = (wild_menu_showing == WM_SCROLLED)
? &msg_grid_adj : &default_grid;

grid_puts(grid, buf, row, 0, attr);
if (selstart != NULL && highlight) {
*selend = NUL;
grid_puts(grid, selstart, row, selstart_col, HL_ATTR(HLF_WM));
}

grid_fill(grid, row, row + 1, clen, Columns,
fillchar, fillchar, attr);
}

win_redraw_last_status(topframe);
xfree(buf);
}




static void win_redr_status(win_T *wp)
{
int row;
char_u *p;
int len;
int fillchar;
int attr;
int this_ru_col;
static int busy = FALSE;



if (busy

|| (wild_menu_showing != 0 && !ui_has(kUIWildmenu))) {
return;
}
busy = true;

wp->w_redr_status = FALSE;
if (wp->w_status_height == 0) {

redraw_cmdline = true;
} else if (!redrawing()) {


wp->w_redr_status = true;
} else if (*p_stl != NUL || *wp->w_p_stl != NUL) {

redraw_custom_statusline(wp);
} else {
fillchar = fillchar_status(&attr, wp);

get_trans_bufname(wp->w_buffer);
p = NameBuff;
len = (int)STRLEN(p);

if (bt_help(wp->w_buffer)
|| wp->w_p_pvw
|| bufIsChanged(wp->w_buffer)
|| wp->w_buffer->b_p_ro) {
*(p + len++) = ' ';
}
if (bt_help(wp->w_buffer)) {
STRCPY(p + len, _("[Help]"));
len += (int)STRLEN(p + len);
}
if (wp->w_p_pvw) {
STRCPY(p + len, _("[Preview]"));
len += (int)STRLEN(p + len);
}
if (bufIsChanged(wp->w_buffer)) {
STRCPY(p + len, "[+]");
len += 3;
}
if (wp->w_buffer->b_p_ro) {
STRCPY(p + len, _("[RO]"));

}

this_ru_col = ru_col - (Columns - wp->w_width);
if (this_ru_col < (wp->w_width + 1) / 2) {
this_ru_col = (wp->w_width + 1) / 2;
}
if (this_ru_col <= 1) {
p = (char_u *)"<"; 
len = 1;
} else {
int clen = 0, i;


clen = (int)mb_string2cells(p);



for (i = 0; p[i] != NUL && clen >= this_ru_col - 1;
i += utfc_ptr2len(p + i)) {
clen -= utf_ptr2cells(p + i);
}
len = clen;
if (i > 0) {
p = p + i - 1;
*p = '<';
++len;
}
}

row = W_ENDROW(wp);
grid_puts(&default_grid, p, row, wp->w_wincol, attr);
grid_fill(&default_grid, row, row + 1, len + wp->w_wincol,
this_ru_col + wp->w_wincol, fillchar, fillchar, attr);

if (get_keymap_str(wp, (char_u *)"<%s>", NameBuff, MAXPATHL)
&& this_ru_col - len > (int)(STRLEN(NameBuff) + 1))
grid_puts(&default_grid, NameBuff, row,
(int)(this_ru_col - STRLEN(NameBuff) - 1), attr);

win_redr_ruler(wp, TRUE);
}




if (wp->w_vsep_width != 0 && wp->w_status_height != 0 && redrawing()) {
if (stl_connected(wp)) {
fillchar = fillchar_status(&attr, wp);
} else {
fillchar = fillchar_vsep(wp, &attr);
}
grid_putchar(&default_grid, fillchar, W_ENDROW(wp), W_ENDCOL(wp), attr);
}
busy = FALSE;
}





static void redraw_custom_statusline(win_T *wp)
{
static int entered = false;
int saved_did_emsg = did_emsg;



if (entered)
return;
entered = TRUE;

did_emsg = false;
win_redr_custom(wp, false);
if (did_emsg) {



set_string_option_direct((char_u *)"statusline", -1,
(char_u *)"", OPT_FREE | (*wp->w_p_stl != NUL
? OPT_LOCAL : OPT_GLOBAL), SID_ERROR);
}
did_emsg |= saved_did_emsg;
entered = false;
}






int stl_connected(win_T *wp)
{
frame_T *fr;

fr = wp->w_frame;
while (fr->fr_parent != NULL) {
if (fr->fr_parent->fr_layout == FR_COL) {
if (fr->fr_next != NULL)
break;
} else {
if (fr->fr_next != NULL)
return TRUE;
}
fr = fr->fr_parent;
}
return FALSE;
}





int
get_keymap_str (
win_T *wp,
char_u *fmt, 
char_u *buf, 
int len 
)
{
char_u *p;

if (wp->w_buffer->b_p_iminsert != B_IMODE_LMAP)
return FALSE;

{
buf_T *old_curbuf = curbuf;
win_T *old_curwin = curwin;
char_u *s;

curbuf = wp->w_buffer;
curwin = wp;
STRCPY(buf, "b:keymap_name"); 
++emsg_skip;
s = p = eval_to_string(buf, NULL, FALSE);
--emsg_skip;
curbuf = old_curbuf;
curwin = old_curwin;
if (p == NULL || *p == NUL) {
if (wp->w_buffer->b_kmap_state & KEYMAP_LOADED) {
p = wp->w_buffer->b_p_keymap;
} else {
p = (char_u *)"lang";
}
}
if (vim_snprintf((char *)buf, len, (char *)fmt, p) > len - 1) {
buf[0] = NUL;
}
xfree(s);
}
return buf[0] != NUL;
}





static void
win_redr_custom (
win_T *wp,
int draw_ruler 
)
{
static int entered = FALSE;
int attr;
int curattr;
int row;
int col = 0;
int maxwidth;
int width;
int n;
int len;
int fillchar;
char_u buf[MAXPATHL];
char_u *stl;
char_u *p;
struct stl_hlrec hltab[STL_MAX_ITEM];
StlClickRecord tabtab[STL_MAX_ITEM];
int use_sandbox = false;
win_T *ewp;
int p_crb_save;

ScreenGrid *grid = &default_grid;




if (entered)
return;
entered = TRUE;


if (wp == NULL) {

stl = p_tal;
row = 0;
fillchar = ' ';
attr = HL_ATTR(HLF_TPF);
maxwidth = Columns;
use_sandbox = was_set_insecurely((char_u *)"tabline", 0);
} else {
row = W_ENDROW(wp);
fillchar = fillchar_status(&attr, wp);
maxwidth = wp->w_width;

if (draw_ruler) {
stl = p_ruf;

if (*stl == '%') {
if (*++stl == '-')
stl++;
if (atoi((char *)stl))
while (ascii_isdigit(*stl))
stl++;
if (*stl++ != '(')
stl = p_ruf;
}
col = ru_col - (Columns - wp->w_width);
if (col < (wp->w_width + 1) / 2) {
col = (wp->w_width + 1) / 2;
}
maxwidth = wp->w_width - col;
if (!wp->w_status_height) {
grid = &msg_grid_adj;
row = Rows - 1;
maxwidth--; 
fillchar = ' ';
attr = HL_ATTR(HLF_MSG);
}

use_sandbox = was_set_insecurely((char_u *)"rulerformat", 0);
} else {
if (*wp->w_p_stl != NUL)
stl = wp->w_p_stl;
else
stl = p_stl;
use_sandbox = was_set_insecurely((char_u *)"statusline",
*wp->w_p_stl == NUL ? 0 : OPT_LOCAL);
}

col += wp->w_wincol;
}

if (maxwidth <= 0)
goto theend;



ewp = wp == NULL ? curwin : wp;
p_crb_save = ewp->w_p_crb;
ewp->w_p_crb = FALSE;



stl = vim_strsave(stl);
width = build_stl_str_hl(ewp, buf, sizeof(buf),
stl, use_sandbox,
fillchar, maxwidth, hltab, tabtab);
xfree(stl);
ewp->w_p_crb = p_crb_save;


p = (char_u *)transstr((const char *)buf);
len = STRLCPY(buf, p, sizeof(buf));
len = (size_t)len < sizeof(buf) ? len : (int)sizeof(buf) - 1;
xfree(p);


while (width < maxwidth && len < (int)sizeof(buf) - 1) {
len += utf_char2bytes(fillchar, buf + len);
width++;
}
buf[len] = NUL;




grid_puts_line_start(grid, row);

curattr = attr;
p = buf;
for (n = 0; hltab[n].start != NULL; n++) {
int textlen = (int)(hltab[n].start - p);
grid_puts_len(grid, p, textlen, row, col, curattr);
col += vim_strnsize(p, textlen);
p = hltab[n].start;

if (hltab[n].userhl == 0)
curattr = attr;
else if (hltab[n].userhl < 0)
curattr = syn_id2attr(-hltab[n].userhl);
else if (wp != NULL && wp != curwin && wp->w_status_height != 0)
curattr = highlight_stlnc[hltab[n].userhl - 1];
else
curattr = highlight_user[hltab[n].userhl - 1];
}

grid_puts(grid, p >= buf + len ? (char_u *)"" : p, row, col,
curattr);

grid_puts_line_flush(false);

if (wp == NULL) {

col = 0;
len = 0;
p = buf;
StlClickDefinition cur_click_def = {
.type = kStlClickDisabled,
};
for (n = 0; tabtab[n].start != NULL; n++) {
len += vim_strnsize(p, (int)(tabtab[n].start - (char *) p));
while (col < len) {
tab_page_click_defs[col++] = cur_click_def;
}
p = (char_u *) tabtab[n].start;
cur_click_def = tabtab[n].def;
}
while (col < Columns) {
tab_page_click_defs[col++] = cur_click_def;
}
}

theend:
entered = FALSE;
}





static void schar_from_ascii(char_u *p, const char c)
{
p[0] = c;
p[1] = 0;
}


static int schar_from_char(char_u *p, int c)
{
int len = utf_char2bytes(c, p);
p[len] = NUL;
return len;
}


static int schar_from_cc(char_u *p, int c, int u8cc[MAX_MCO])
{
int len = utf_char2bytes(c, p);
for (int i = 0; i < MAX_MCO; i++) {
if (u8cc[i] == 0) {
break;
}
len += utf_char2bytes(u8cc[i], p + len);
}
p[len] = 0;
return len;
}


static int schar_cmp(char_u *sc1, char_u *sc2)
{
return STRNCMP(sc1, sc2, sizeof(schar_T));
}


static void schar_copy(char_u *sc1, char_u *sc2)
{
STRLCPY(sc1, sc2, sizeof(schar_T));
}

static int line_off2cells(schar_T *line, size_t off, size_t max_off)
{
return (off + 1 < max_off && line[off + 1][0] == 0) ? 2 : 1;
}



static int grid_off2cells(ScreenGrid *grid, size_t off, size_t max_off)
{
return line_off2cells(grid->chars, off, max_off);
}





bool grid_lefthalve(ScreenGrid *grid, int row, int col)
{
screen_adjust_grid(&grid, &row, &col);

return grid_off2cells(grid, grid->line_offset[row] + col,
grid->line_offset[row] + grid->Columns) > 1;
}



int grid_fix_col(ScreenGrid *grid, int col, int row)
{
int coloff = 0;
screen_adjust_grid(&grid, &row, &coloff);

col += coloff;
if (grid->chars != NULL && col > 0
&& grid->chars[grid->line_offset[row] + col][0] == 0) {
return col - 1 - coloff;
}
return col - coloff;
}


void grid_putchar(ScreenGrid *grid, int c, int row, int col, int attr)
{
char_u buf[MB_MAXBYTES + 1];

buf[utf_char2bytes(c, buf)] = NUL;
grid_puts(grid, buf, row, col, attr);
}



void grid_getbytes(ScreenGrid *grid, int row, int col, char_u *bytes,
int *attrp)
{
unsigned off;

screen_adjust_grid(&grid, &row, &col);


if (grid->chars != NULL && row < grid->Rows && col < grid->Columns) {
off = grid->line_offset[row] + col;
*attrp = grid->attrs[off];
schar_copy(bytes, grid->chars[off]);
}
}






void grid_puts(ScreenGrid *grid, char_u *text, int row, int col, int attr)
{
grid_puts_len(grid, text, -1, row, col, attr);
}

static ScreenGrid *put_dirty_grid = NULL;
static int put_dirty_row = -1;
static int put_dirty_first = INT_MAX;
static int put_dirty_last = 0;





void grid_puts_line_start(ScreenGrid *grid, int row)
{
int col = 0; 
screen_adjust_grid(&grid, &row, &col);
assert(put_dirty_row == -1);
put_dirty_row = row;
put_dirty_grid = grid;
}



void grid_puts_len(ScreenGrid *grid, char_u *text, int textlen, int row,
int col, int attr)
{
unsigned off;
char_u *ptr = text;
int len = textlen;
int c;
unsigned max_off;
int mbyte_blen = 1;
int mbyte_cells = 1;
int u8c = 0;
int u8cc[MAX_MCO];
int clear_next_cell = FALSE;
int prev_c = 0; 
int pc, nc, nc1;
int pcc[MAX_MCO];
int need_redraw;
bool do_flush = false;

screen_adjust_grid(&grid, &row, &col);



if (grid->chars == NULL
|| row >= grid->Rows || row < 0
|| col >= grid->Columns || col < 0) {
return;
}

if (put_dirty_row == -1) {
grid_puts_line_start(grid, row);
do_flush = true;
} else {
if (grid != put_dirty_grid || row != put_dirty_row) {
abort();
}
}
off = grid->line_offset[row] + col;



if (grid != &default_grid && col == 0 && grid_invalid_row(grid, row)) {

put_dirty_first = -1;
put_dirty_last = MAX(put_dirty_last, 1);
}

max_off = grid->line_offset[row] + grid->Columns;
while (col < grid->Columns
&& (len < 0 || (int)(ptr - text) < len)
&& *ptr != NUL) {
c = *ptr;

if (len > 0) {
mbyte_blen = utfc_ptr2len_len(ptr, (int)((text + len) - ptr));
} else {
mbyte_blen = utfc_ptr2len(ptr);
}
if (len >= 0) {
u8c = utfc_ptr2char_len(ptr, u8cc, (int)((text + len) - ptr));
} else {
u8c = utfc_ptr2char(ptr, u8cc);
}
mbyte_cells = utf_char2cells(u8c);
if (p_arshape && !p_tbidi && arabic_char(u8c)) {

if (len >= 0 && (int)(ptr - text) + mbyte_blen >= len) {

nc = NUL;
nc1 = NUL;
} else {
nc = utfc_ptr2char_len(ptr + mbyte_blen, pcc,
(int)((text + len) - ptr - mbyte_blen));
nc1 = pcc[0];
}
pc = prev_c;
prev_c = u8c;
u8c = arabic_shape(u8c, &c, &u8cc[0], nc, nc1, pc);
} else {
prev_c = u8c;
}
if (col + mbyte_cells > grid->Columns) {


c = '>';
mbyte_cells = 1;
}

schar_T buf;
schar_from_cc(buf, u8c, u8cc);


need_redraw = schar_cmp(grid->chars[off], buf)
|| (mbyte_cells == 2 && grid->chars[off + 1][0] != 0)
|| grid->attrs[off] != attr
|| exmode_active;

if (need_redraw) {





if (clear_next_cell) {
clear_next_cell = false;
} else if ((len < 0 ? ptr[mbyte_blen] == NUL
: ptr + mbyte_blen >= text + len)
&& ((mbyte_cells == 1
&& grid_off2cells(grid, off, max_off) > 1)
|| (mbyte_cells == 2
&& grid_off2cells(grid, off, max_off) == 1
&& grid_off2cells(grid, off + 1, max_off) > 1))) {
clear_next_cell = true;
}

schar_copy(grid->chars[off], buf);
grid->attrs[off] = attr;
if (mbyte_cells == 2) {
grid->chars[off + 1][0] = 0;
grid->attrs[off + 1] = attr;
}
put_dirty_first = MIN(put_dirty_first, col);
put_dirty_last = MAX(put_dirty_last, col+mbyte_cells);
}

off += mbyte_cells;
col += mbyte_cells;
ptr += mbyte_blen;
if (clear_next_cell) {

ptr = (char_u *)" ";
len = -1;
}
}

if (do_flush) {
grid_puts_line_flush(true);
}
}







void grid_puts_line_flush(bool set_cursor)
{
assert(put_dirty_row != -1);
if (put_dirty_first < put_dirty_last) {
if (set_cursor) {
ui_grid_cursor_goto(put_dirty_grid->handle, put_dirty_row,
MIN(put_dirty_last, put_dirty_grid->Columns-1));
}
if (!put_dirty_grid->throttled) {
ui_line(put_dirty_grid, put_dirty_row, put_dirty_first, put_dirty_last,
put_dirty_last, 0, false);
} else if (put_dirty_grid->dirty_col) {
if (put_dirty_last > put_dirty_grid->dirty_col[put_dirty_row]) {
put_dirty_grid->dirty_col[put_dirty_row] = put_dirty_last;
}
}
put_dirty_first = INT_MAX;
put_dirty_last = 0;
}
put_dirty_row = -1;
put_dirty_grid = NULL;
}




static void start_search_hl(void)
{
if (p_hls && !no_hlsearch) {
last_pat_prog(&search_hl.rm);

search_hl.tm = profile_setlimit(p_rdt);
}
}




static void end_search_hl(void)
{
if (search_hl.rm.regprog != NULL) {
vim_regfree(search_hl.rm.regprog);
search_hl.rm.regprog = NULL;
}
}





static void init_search_hl(win_T *wp)
{
matchitem_T *cur;



cur = wp->w_match_head;
while (cur != NULL) {
cur->hl.rm = cur->match;
if (cur->hlg_id == 0)
cur->hl.attr = 0;
else
cur->hl.attr = syn_id2attr(cur->hlg_id);
cur->hl.buf = wp->w_buffer;
cur->hl.lnum = 0;
cur->hl.first_lnum = 0;

cur->hl.tm = profile_setlimit(p_rdt);
cur = cur->next;
}
search_hl.buf = wp->w_buffer;
search_hl.lnum = 0;
search_hl.first_lnum = 0;
search_hl.attr = win_hl_attr(wp, HLF_L);


}




static void prepare_search_hl(win_T *wp, linenr_T lnum)
{
matchitem_T *cur; 
match_T *shl; 
int shl_flag; 

int n;






cur = wp->w_match_head;
shl_flag = false;
while (cur != NULL || shl_flag == false) {
if (shl_flag == false) {
shl = &search_hl;
shl_flag = true;
} else {
shl = &cur->hl; 
}
if (shl->rm.regprog != NULL
&& shl->lnum == 0
&& re_multiline(shl->rm.regprog)) {
if (shl->first_lnum == 0) {
for (shl->first_lnum = lnum;
shl->first_lnum > wp->w_topline;
shl->first_lnum--) {
if (hasFoldingWin(wp, shl->first_lnum - 1, NULL, NULL, true, NULL)) {
break;
}
}
}
if (cur != NULL) {
cur->pos.cur = 0;
}
bool pos_inprogress = true; 

n = 0;
while (shl->first_lnum < lnum && (shl->rm.regprog != NULL
|| (cur != NULL && pos_inprogress))) {
next_search_hl(wp, shl, shl->first_lnum, (colnr_T)n,
shl == &search_hl ? NULL : cur);
pos_inprogress = !(cur == NULL || cur->pos.cur == 0);
if (shl->lnum != 0) {
shl->first_lnum = shl->lnum
+ shl->rm.endpos[0].lnum
- shl->rm.startpos[0].lnum;
n = shl->rm.endpos[0].col;
} else {
++shl->first_lnum;
n = 0;
}
}
}
if (shl != &search_hl && cur != NULL)
cur = cur->next;
}
}









static void
next_search_hl (
win_T *win,
match_T *shl, 
linenr_T lnum,
colnr_T mincol, 
matchitem_T *cur 
)
{
linenr_T l;
colnr_T matchcol;
long nmatched = 0;
int save_called_emsg = called_emsg;

if (shl->lnum != 0) {





l = shl->lnum + shl->rm.endpos[0].lnum - shl->rm.startpos[0].lnum;
if (lnum > l)
shl->lnum = 0;
else if (lnum < l || shl->rm.endpos[0].col > mincol)
return;
}





called_emsg = FALSE;
for (;; ) {

if (profile_passed_limit(shl->tm)) {
shl->lnum = 0; 
break;
}






if (shl->lnum == 0)
matchcol = 0;
else if (vim_strchr(p_cpo, CPO_SEARCH) == NULL
|| (shl->rm.endpos[0].lnum == 0
&& shl->rm.endpos[0].col <= shl->rm.startpos[0].col)) {
char_u *ml;

matchcol = shl->rm.startpos[0].col;
ml = ml_get_buf(shl->buf, lnum, FALSE) + matchcol;
if (*ml == NUL) {
++matchcol;
shl->lnum = 0;
break;
}
matchcol += mb_ptr2len(ml);
} else {
matchcol = shl->rm.endpos[0].col;
}

shl->lnum = lnum;
if (shl->rm.regprog != NULL) {


bool regprog_is_copy = (shl != &search_hl
&& cur != NULL
&& shl == &cur->hl
&& cur->match.regprog == cur->hl.rm.regprog);
int timed_out = false;

nmatched = vim_regexec_multi(&shl->rm, win, shl->buf, lnum, matchcol,
&(shl->tm), &timed_out);

if (regprog_is_copy) {
cur->match.regprog = cur->hl.rm.regprog;
}
if (called_emsg || got_int || timed_out) {

if (shl == &search_hl) {

vim_regfree(shl->rm.regprog);
set_no_hlsearch(true);
}
shl->rm.regprog = NULL;
shl->lnum = 0;
got_int = FALSE; 
break;
}
} else if (cur != NULL) {
nmatched = next_search_hl_pos(shl, lnum, &(cur->pos), matchcol);
}
if (nmatched == 0) {
shl->lnum = 0; 
break;
}
if (shl->rm.startpos[0].lnum > 0
|| shl->rm.startpos[0].col >= mincol
|| nmatched > 1
|| shl->rm.endpos[0].col > mincol) {
shl->lnum += shl->rm.startpos[0].lnum;
break; 
}


called_emsg = save_called_emsg;
}
}



static int
next_search_hl_pos(
match_T *shl, 
linenr_T lnum,
posmatch_T *posmatch, 
colnr_T mincol 
)
{
int i;
int found = -1;

shl->lnum = 0;
for (i = posmatch->cur; i < MAXPOSMATCH; i++) {
llpos_T *pos = &posmatch->pos[i];

if (pos->lnum == 0) {
break;
}
if (pos->len == 0 && pos->col < mincol) {
continue;
}
if (pos->lnum == lnum) {
if (found >= 0) {


if (pos->col < posmatch->pos[found].col) {
llpos_T tmp = *pos;

*pos = posmatch->pos[found];
posmatch->pos[found] = tmp;
}
} else {
found = i;
}
}
}
posmatch->cur = 0;
if (found >= 0) {
colnr_T start = posmatch->pos[found].col == 0
? 0: posmatch->pos[found].col - 1;
colnr_T end = posmatch->pos[found].col == 0
? MAXCOL : start + posmatch->pos[found].len;

shl->lnum = lnum;
shl->rm.startpos[0].lnum = 0;
shl->rm.startpos[0].col = start;
shl->rm.endpos[0].lnum = 0;
shl->rm.endpos[0].col = end;
shl->is_addpos = true;
posmatch->cur = found + 1;
return 1;
}
return 0;
}





void grid_fill(ScreenGrid *grid, int start_row, int end_row, int start_col,
int end_col, int c1, int c2, int attr)
{
schar_T sc;

int row_off = 0, col_off = 0;
screen_adjust_grid(&grid, &row_off, &col_off);
start_row += row_off;
end_row += row_off;
start_col += col_off;
end_col += col_off;


if (end_row > grid->Rows) {
end_row = grid->Rows;
}
if (end_col > grid->Columns) {
end_col = grid->Columns;
}


if (start_row >= end_row || start_col >= end_col) {
return;
}

for (int row = start_row; row < end_row; row++) {




if (start_col > 0 && grid_fix_col(grid, start_col, row) != start_col) {
grid_puts_len(grid, (char_u *)" ", 1, row, start_col - 1, 0);
}
if (end_col < grid->Columns
&& grid_fix_col(grid, end_col, row) != end_col) {
grid_puts_len(grid, (char_u *)" ", 1, row, end_col, 0);
}




int dirty_first = INT_MAX;
int dirty_last = 0;

int col = start_col;
schar_from_char(sc, c1);
int lineoff = grid->line_offset[row];
for (col = start_col; col < end_col; col++) {
int off = lineoff + col;
if (schar_cmp(grid->chars[off], sc)
|| grid->attrs[off] != attr) {
schar_copy(grid->chars[off], sc);
grid->attrs[off] = attr;
if (dirty_first == INT_MAX) {
dirty_first = col;
}
dirty_last = col+1;
}
if (col == start_col) {
schar_from_char(sc, c2);
}
}
if (dirty_last > dirty_first) {

if (put_dirty_row == row) {
put_dirty_first = MIN(put_dirty_first, dirty_first);
put_dirty_last = MAX(put_dirty_last, dirty_last);
} else if (grid->throttled) {

assert(grid == &msg_grid);
int dirty = 0;
if (attr != HL_ATTR(HLF_MSG) || c2 != ' ') {
dirty = dirty_last;
} else if (c1 != ' ') {
dirty = dirty_first + 1;
}
if (grid->dirty_col && dirty > grid->dirty_col[row]) {
grid->dirty_col[row] = dirty;
}
} else {
int last = c2 != ' ' ? dirty_last : dirty_first + (c1 != ' ');
ui_line(grid, row, dirty_first, last, dirty_last, attr, false);
}
}

if (end_col == grid->Columns) {
grid->line_wraps[row] = false;
}
}
}





void check_for_delay(int check_msg_scroll)
{
if ((emsg_on_display || (check_msg_scroll && msg_scroll))
&& !did_wait_return
&& emsg_silent == 0) {
ui_flush();
os_delay(1000L, true);
emsg_on_display = FALSE;
if (check_msg_scroll)
msg_scroll = FALSE;
}
}






void win_grid_alloc(win_T *wp)
{
ScreenGrid *grid = &wp->w_grid;

int rows = wp->w_height_inner;
int cols = wp->w_width_inner;

bool want_allocation = ui_has(kUIMultigrid) || wp->w_floating;
bool has_allocation = (grid->chars != NULL);

if (grid->Rows != rows) {
wp->w_lines_valid = 0;
xfree(wp->w_lines);
wp->w_lines = xcalloc(rows+1, sizeof(wline_T));
}

int was_resized = false;
if ((has_allocation != want_allocation)
|| grid->Rows != rows
|| grid->Columns != cols) {
if (want_allocation) {
grid_alloc(grid, rows, cols, wp->w_grid.valid, wp->w_grid.valid);
grid->valid = true;
} else {


grid_free(grid);
grid->Rows = rows;
grid->Columns = cols;
grid->valid = false;
}
was_resized = true;
} else if (want_allocation && has_allocation && !wp->w_grid.valid) {
grid_invalidate(grid);
grid->valid = true;
}

grid->row_offset = wp->w_winrow;
grid->col_offset = wp->w_wincol;





if ((send_grid_resize || was_resized) && want_allocation) {
ui_call_grid_resize(grid->handle, grid->Columns, grid->Rows);
}
}


void grid_assign_handle(ScreenGrid *grid)
{
static int last_grid_handle = DEFAULT_GRID_HANDLE;


if (grid->handle == 0) {
grid->handle = ++last_grid_handle;
}
}











void screenalloc(void)
{



if (resizing) {
return;
}
resizing = true;

int retry_count = 0;

retry:



if ((default_grid.chars != NULL
&& Rows == default_grid.Rows
&& Columns == default_grid.Columns
)
|| Rows == 0
|| Columns == 0
|| (!full_screen && default_grid.chars == NULL)) {
resizing = false;
return;
}





++RedrawingDisabled;



ui_comp_set_screen_valid(false);
if (msg_grid.chars) {
msg_grid_invalid = true;
}

win_new_shellsize(); 

comp_col(); 











grid_alloc(&default_grid, Rows, Columns, true, true);
StlClickDefinition *new_tab_page_click_defs = xcalloc(
(size_t)Columns, sizeof(*new_tab_page_click_defs));

clear_tab_page_click_defs(tab_page_click_defs, tab_page_click_defs_size);
xfree(tab_page_click_defs);

tab_page_click_defs = new_tab_page_click_defs;
tab_page_click_defs_size = Columns;

default_grid.row_offset = 0;
default_grid.col_offset = 0;
default_grid.handle = DEFAULT_GRID_HANDLE;

must_redraw = CLEAR; 

RedrawingDisabled--;





if (starting == 0 && ++retry_count <= 3) {
apply_autocmds(EVENT_VIMRESIZED, NULL, NULL, FALSE, curbuf);


goto retry;
}

resizing = false;
}

void grid_alloc(ScreenGrid *grid, int rows, int columns, bool copy, bool valid)
{
int new_row;
ScreenGrid new = *grid;
assert(rows >= 0 && columns >= 0);
size_t ncells = (size_t)rows * columns;
new.chars = xmalloc(ncells * sizeof(schar_T));
new.attrs = xmalloc(ncells * sizeof(sattr_T));
new.line_offset = xmalloc((size_t)(rows * sizeof(unsigned)));
new.line_wraps = xmalloc((size_t)(rows * sizeof(char_u)));

new.Rows = rows;
new.Columns = columns;

for (new_row = 0; new_row < new.Rows; new_row++) {
new.line_offset[new_row] = new_row * new.Columns;
new.line_wraps[new_row] = false;

grid_clear_line(&new, new.line_offset[new_row], columns, valid);

if (copy) {




if (new_row < grid->Rows && grid->chars != NULL) {
int len = MIN(grid->Columns, new.Columns);
memmove(new.chars + new.line_offset[new_row],
grid->chars + grid->line_offset[new_row],
(size_t)len * sizeof(schar_T));
memmove(new.attrs + new.line_offset[new_row],
grid->attrs + grid->line_offset[new_row],
(size_t)len * sizeof(sattr_T));
}
}
}
grid_free(grid);
*grid = new;



if (linebuf_size < (size_t)columns) {
xfree(linebuf_char);
xfree(linebuf_attr);
linebuf_char = xmalloc(columns * sizeof(schar_T));
linebuf_attr = xmalloc(columns * sizeof(sattr_T));
linebuf_size = columns;
}
}

void grid_free(ScreenGrid *grid)
{
xfree(grid->chars);
xfree(grid->attrs);
xfree(grid->line_offset);
xfree(grid->line_wraps);

grid->chars = NULL;
grid->attrs = NULL;
grid->line_offset = NULL;
grid->line_wraps = NULL;
}


void screen_free_all_mem(void)
{
grid_free(&default_grid);
xfree(linebuf_char);
xfree(linebuf_attr);
}





void clear_tab_page_click_defs(StlClickDefinition *const tpcd,
const long tpcd_size)
{
if (tpcd != NULL) {
for (long i = 0; i < tpcd_size; i++) {
if (i == 0 || tpcd[i].func != tpcd[i - 1].func) {
xfree(tpcd[i].func);
}
}
memset(tpcd, 0, (size_t) tpcd_size * sizeof(tpcd[0]));
}
}

void screenclear(void)
{
check_for_delay(false);
screenalloc(); 

int i;

if (starting == NO_SCREEN || default_grid.chars == NULL) {
return;
}


for (i = 0; i < default_grid.Rows; i++) {
grid_clear_line(&default_grid, default_grid.line_offset[i],
(int)default_grid.Columns, true);
default_grid.line_wraps[i] = false;
}

ui_call_grid_clear(1); 
ui_comp_set_screen_valid(true);

clear_cmdline = false;
mode_displayed = false;

redraw_all_later(NOT_VALID);
redraw_cmdline = true;
redraw_tabline = true;
redraw_popupmenu = true;
pum_invalidate();
FOR_ALL_WINDOWS_IN_TAB(wp, curtab) {
if (wp->w_floating) {
wp->w_redr_type = CLEAR;
}
}
if (must_redraw == CLEAR) {
must_redraw = NOT_VALID; 
}
compute_cmdrow();
msg_row = cmdline_row; 
msg_col = 0;
msg_scrolled = 0; 
msg_didany = false;
msg_didout = false;
if (HL_ATTR(HLF_MSG) > 0 && msg_use_grid() && msg_grid.chars) {
grid_invalidate(&msg_grid);
msg_grid_validate();
msg_grid_invalid = false;
clear_cmdline = true;
}
}



void grid_clear_line(ScreenGrid *grid, unsigned off, int width, bool valid)
{
for (int col = 0; col < width; col++) {
schar_from_ascii(grid->chars[off + col], ' ');
}
int fill = valid ? 0 : -1;
(void)memset(grid->attrs + off, fill, (size_t)width * sizeof(sattr_T));
}

void grid_invalidate(ScreenGrid *grid)
{
(void)memset(grid->attrs, -1, grid->Rows * grid->Columns * sizeof(sattr_T));
}

bool grid_invalid_row(ScreenGrid *grid, int row)
{
return grid->attrs[grid->line_offset[row]] < 0;
}




static void linecopy(ScreenGrid *grid, int to, int from, int col, int width)
{
unsigned off_to = grid->line_offset[to] + col;
unsigned off_from = grid->line_offset[from] + col;

memmove(grid->chars + off_to, grid->chars + off_from,
width * sizeof(schar_T));
memmove(grid->attrs + off_to, grid->attrs + off_from,
width * sizeof(sattr_T));
}




void setcursor(void)
{
if (redrawing()) {
validate_cursor();

ScreenGrid *grid = &curwin->w_grid;
int row = curwin->w_wrow;
int col = curwin->w_wcol;
if (curwin->w_p_rl) {


col = curwin->w_width_inner - curwin->w_wcol
- ((utf_ptr2cells(get_cursor_pos_ptr()) == 2
&& vim_isprintc(gchar_cursor())) ? 2 : 1);
}

screen_adjust_grid(&grid, &row, &col);
ui_grid_cursor_goto(grid->handle, row, col);
}
}





void win_scroll_lines(win_T *wp, int row, int line_count)
{
if (!redrawing() || line_count == 0) {
return;
}


if (row + abs(line_count) >= wp->w_grid.Rows) {
return;
}

if (line_count < 0) {
grid_del_lines(&wp->w_grid, row, -line_count,
wp->w_grid.Rows, 0, wp->w_grid.Columns);
} else {
grid_ins_lines(&wp->w_grid, row, line_count,
wp->w_grid.Rows, 0, wp->w_grid.Columns);
}
}


















void grid_ins_lines(ScreenGrid *grid, int row, int line_count, int end, int col,
int width)
{
int i;
int j;
unsigned temp;

int row_off = 0;
screen_adjust_grid(&grid, &row_off, &col);
row += row_off;
end += row_off;

if (line_count <= 0) {
return;
}



for (i = 0; i < line_count; i++) {
if (width != grid->Columns) {

j = end - 1 - i;
while ((j -= line_count) >= row) {
linecopy(grid, j + line_count, j, col, width);
}
j += line_count;
grid_clear_line(grid, grid->line_offset[j] + col, width, false);
grid->line_wraps[j] = false;
} else {
j = end - 1 - i;
temp = grid->line_offset[j];
while ((j -= line_count) >= row) {
grid->line_offset[j + line_count] = grid->line_offset[j];
grid->line_wraps[j + line_count] = grid->line_wraps[j];
}
grid->line_offset[j + line_count] = temp;
grid->line_wraps[j + line_count] = false;
grid_clear_line(grid, temp, (int)grid->Columns, false);
}
}

if (!grid->throttled) {
ui_call_grid_scroll(grid->handle, row, end, col, col+width, -line_count, 0);
}

return;
}





void grid_del_lines(ScreenGrid *grid, int row, int line_count, int end, int col,
int width)
{
int j;
int i;
unsigned temp;

int row_off = 0;
screen_adjust_grid(&grid, &row_off, &col);
row += row_off;
end += row_off;

if (line_count <= 0) {
return;
}



for (i = 0; i < line_count; i++) {
if (width != grid->Columns) {

j = row + i;
while ((j += line_count) <= end - 1) {
linecopy(grid, j - line_count, j, col, width);
}
j -= line_count;
grid_clear_line(grid, grid->line_offset[j] + col, width, false);
grid->line_wraps[j] = false;
} else {

j = row + i;
temp = grid->line_offset[j];
while ((j += line_count) <= end - 1) {
grid->line_offset[j - line_count] = grid->line_offset[j];
grid->line_wraps[j - line_count] = grid->line_wraps[j];
}
grid->line_offset[j - line_count] = temp;
grid->line_wraps[j - line_count] = false;
grid_clear_line(grid, temp, (int)grid->Columns, false);
}
}

if (!grid->throttled) {
ui_call_grid_scroll(grid->handle, row, end, col, col+width, line_count, 0);
}

return;
}








int showmode(void)
{
int need_clear;
int length = 0;
int do_mode;
int attr;
int nwr_save;
int sub_attr;

if (ui_has(kUIMessages) && clear_cmdline) {
msg_ext_clear(true);
}


msg_ext_ui_flush();

msg_grid_validate();

do_mode = ((p_smd && msg_silent == 0)
&& ((State & TERM_FOCUS)
|| (State & INSERT)
|| restart_edit
|| VIsual_active));
if (do_mode || reg_recording != 0) {



if (!redrawing() || (char_avail() && !KeyTyped) || msg_silent != 0) {
redraw_cmdline = TRUE; 
return 0;
}

nwr_save = need_wait_return;


check_for_delay(FALSE);


need_clear = clear_cmdline;
if (clear_cmdline && cmdline_row < Rows - 1) {
msg_clr_cmdline(); 
}


msg_pos_mode();
attr = HL_ATTR(HLF_CM); 



msg_no_more = true;
int save_lines_left = lines_left;
lines_left = 0;

if (do_mode) {
MSG_PUTS_ATTR("--", attr);

if (edit_submode != NULL && !shortmess(SHM_COMPLETIONMENU)) {



if (ui_has(kUIMessages)) {
length = INT_MAX;
} else {
length = (Rows - msg_row) * Columns - 3;
}
if (edit_submode_extra != NULL) {
length -= vim_strsize(edit_submode_extra);
}
if (length > 0) {
if (edit_submode_pre != NULL)
length -= vim_strsize(edit_submode_pre);
if (length - vim_strsize(edit_submode) > 0) {
if (edit_submode_pre != NULL) {
msg_puts_attr((const char *)edit_submode_pre, attr);
}
msg_puts_attr((const char *)edit_submode, attr);
}
if (edit_submode_extra != NULL) {
MSG_PUTS_ATTR(" ", attr); 
if ((int)edit_submode_highl < (int)HLF_COUNT) {
sub_attr = win_hl_attr(curwin, edit_submode_highl);
} else {
sub_attr = attr;
}
msg_puts_attr((const char *)edit_submode_extra, sub_attr);
}
}
} else {
if (State & TERM_FOCUS) {
MSG_PUTS_ATTR(_(" TERMINAL"), attr);
} else if (State & VREPLACE_FLAG)
MSG_PUTS_ATTR(_(" VREPLACE"), attr);
else if (State & REPLACE_FLAG)
MSG_PUTS_ATTR(_(" REPLACE"), attr);
else if (State & INSERT) {
if (p_ri)
MSG_PUTS_ATTR(_(" REVERSE"), attr);
MSG_PUTS_ATTR(_(" INSERT"), attr);
} else if (restart_edit == 'I' || restart_edit == 'i'
|| restart_edit == 'a') {
MSG_PUTS_ATTR(_(" (insert)"), attr);
} else if (restart_edit == 'R') {
MSG_PUTS_ATTR(_(" (replace)"), attr);
} else if (restart_edit == 'V') {
MSG_PUTS_ATTR(_(" (vreplace)"), attr);
}
if (p_hkmap) {
MSG_PUTS_ATTR(_(" Hebrew"), attr);
}
if (State & LANGMAP) {
if (curwin->w_p_arab) {
MSG_PUTS_ATTR(_(" Arabic"), attr);
} else if (get_keymap_str(curwin, (char_u *)" (%s)",
NameBuff, MAXPATHL)) {
MSG_PUTS_ATTR(NameBuff, attr);
}
}
if ((State & INSERT) && p_paste)
MSG_PUTS_ATTR(_(" (paste)"), attr);

if (VIsual_active) {
char *p;



switch ((VIsual_select ? 4 : 0)
+ (VIsual_mode == Ctrl_V) * 2
+ (VIsual_mode == 'V')) {
case 0: p = N_(" VISUAL"); break;
case 1: p = N_(" VISUAL LINE"); break;
case 2: p = N_(" VISUAL BLOCK"); break;
case 4: p = N_(" SELECT"); break;
case 5: p = N_(" SELECT LINE"); break;
default: p = N_(" SELECT BLOCK"); break;
}
MSG_PUTS_ATTR(_(p), attr);
}
MSG_PUTS_ATTR(" --", attr);
}

need_clear = TRUE;
}
if (reg_recording != 0
&& edit_submode == NULL 
) {
recording_mode(attr);
need_clear = true;
}

mode_displayed = TRUE;
if (need_clear || clear_cmdline)
msg_clr_eos();
msg_didout = FALSE; 
length = msg_col;
msg_col = 0;
msg_no_more = false;
lines_left = save_lines_left;
need_wait_return = nwr_save; 
} else if (clear_cmdline && msg_silent == 0) {

msg_clr_cmdline();
}


msg_ext_flush_showmode();


if (VIsual_active)
clear_showcmd();



win_T *last = lastwin_nofloating();
if (redrawing() && last->w_status_height == 0) {
win_redr_ruler(last, true);
}
redraw_cmdline = false;
clear_cmdline = false;

return length;
}




static void msg_pos_mode(void)
{
msg_col = 0;
msg_row = Rows - 1;
}




void unshowmode(bool force)
{

if (!redrawing() || (!force && char_avail() && !KeyTyped)) {
redraw_cmdline = true; 
} else {
clearmode();
}
}


void clearmode(void)
{
const int save_msg_row = msg_row;
const int save_msg_col = msg_col;

msg_ext_ui_flush();
msg_pos_mode();
if (reg_recording != 0) {
recording_mode(HL_ATTR(HLF_CM));
}
msg_clr_eos();
msg_ext_flush_showmode();

msg_col = save_msg_col;
msg_row = save_msg_row;
}

static void recording_mode(int attr)
{
MSG_PUTS_ATTR(_("recording"), attr);
if (!shortmess(SHM_RECORDING)) {
char_u s[4];
snprintf((char *)s, ARRAY_SIZE(s), " @%c", reg_recording);
MSG_PUTS_ATTR(s, attr);
}
}




void draw_tabline(void)
{
int tabcount = 0;
int tabwidth = 0;
int col = 0;
int scol = 0;
int attr;
win_T *wp;
win_T *cwp;
int wincount;
int modified;
int c;
int len;
int attr_nosel = HL_ATTR(HLF_TP);
int attr_fill = HL_ATTR(HLF_TPF);
char_u *p;
int room;
int use_sep_chars = (t_colors < 8
);

if (default_grid.chars == NULL) {
return;
}
redraw_tabline = false;

if (ui_has(kUITabline)) {
ui_ext_tabline_update();
return;
}

if (tabline_height() < 1)
return;



assert(Columns == tab_page_click_defs_size);
clear_tab_page_click_defs(tab_page_click_defs, tab_page_click_defs_size);


if (*p_tal != NUL) {
int saved_did_emsg = did_emsg;



did_emsg = false;
win_redr_custom(NULL, false);
if (did_emsg) {
set_string_option_direct((char_u *)"tabline", -1,
(char_u *)"", OPT_FREE, SID_ERROR);
}
did_emsg |= saved_did_emsg;
} else {
FOR_ALL_TABS(tp) {
++tabcount;
}

if (tabcount > 0) {
tabwidth = (Columns - 1 + tabcount / 2) / tabcount;
}

if (tabwidth < 6) {
tabwidth = 6;
}

attr = attr_nosel;
tabcount = 0;

FOR_ALL_TABS(tp) {
if (col >= Columns - 4) {
break;
}

scol = col;

if (tp == curtab) {
cwp = curwin;
wp = firstwin;
} else {
cwp = tp->tp_curwin;
wp = tp->tp_firstwin;
}


if (tp->tp_topframe == topframe) {
attr = win_hl_attr(cwp, HLF_TPS);
}
if (use_sep_chars && col > 0) {
grid_putchar(&default_grid, '|', 0, col++, attr);
}

if (tp->tp_topframe != topframe) {
attr = win_hl_attr(cwp, HLF_TP);
}

grid_putchar(&default_grid, ' ', 0, col++, attr);

modified = false;

for (wincount = 0; wp != NULL; wp = wp->w_next, ++wincount) {
if (bufIsChanged(wp->w_buffer)) {
modified = true;
}
}


if (modified || wincount > 1) {
if (wincount > 1) {
vim_snprintf((char *)NameBuff, MAXPATHL, "%d", wincount);
len = (int)STRLEN(NameBuff);
if (col + len >= Columns - 3) {
break;
}
grid_puts_len(&default_grid, NameBuff, len, 0, col,
hl_combine_attr(attr, win_hl_attr(cwp, HLF_T)));
col += len;
}
if (modified) {
grid_puts_len(&default_grid, (char_u *)"+", 1, 0, col++, attr);
}
grid_putchar(&default_grid, ' ', 0, col++, attr);
}

room = scol - col + tabwidth - 1;
if (room > 0) {

get_trans_bufname(cwp->w_buffer);
(void)shorten_dir(NameBuff);
len = vim_strsize(NameBuff);
p = NameBuff;
while (len > room) {
len -= ptr2cells(p);
MB_PTR_ADV(p);
}
if (len > Columns - col - 1) {
len = Columns - col - 1;
}

grid_puts_len(&default_grid, p, (int)STRLEN(p), 0, col, attr);
col += len;
}
grid_putchar(&default_grid, ' ', 0, col++, attr);



tabcount++;
while (scol < col) {
tab_page_click_defs[scol++] = (StlClickDefinition) {
.type = kStlClickTabSwitch,
.tabnr = tabcount,
.func = NULL,
};
}
}

if (use_sep_chars)
c = '_';
else
c = ' ';
grid_fill(&default_grid, 0, 1, col, Columns, c, c, attr_fill);


if (first_tabpage->tp_next != NULL) {
grid_putchar(&default_grid, 'X', 0, Columns - 1, attr_nosel);
tab_page_click_defs[Columns - 1] = (StlClickDefinition) {
.type = kStlClickTabClose,
.tabnr = 999,
.func = NULL,
};
}
}



redraw_tabline = FALSE;
}

void ui_ext_tabline_update(void)
{
Array tabs = ARRAY_DICT_INIT;
FOR_ALL_TABS(tp) {
Dictionary tab_info = ARRAY_DICT_INIT;
PUT(tab_info, "tab", TABPAGE_OBJ(tp->handle));

win_T *cwp = (tp == curtab) ? curwin : tp->tp_curwin;
get_trans_bufname(cwp->w_buffer);
PUT(tab_info, "name", STRING_OBJ(cstr_to_string((char *)NameBuff)));

ADD(tabs, DICTIONARY_OBJ(tab_info));
}
ui_call_tabline_update(curtab->handle, tabs);
}





void get_trans_bufname(buf_T *buf)
{
if (buf_spname(buf) != NULL)
STRLCPY(NameBuff, buf_spname(buf), MAXPATHL);
else
home_replace(buf, buf->b_fname, NameBuff, MAXPATHL, TRUE);
trans_characters(NameBuff, MAXPATHL);
}




static int fillchar_status(int *attr, win_T *wp)
{
int fill;
bool is_curwin = (wp == curwin);
if (is_curwin) {
*attr = win_hl_attr(wp, HLF_S);
fill = wp->w_p_fcs_chars.stl;
} else {
*attr = win_hl_attr(wp, HLF_SNC);
fill = wp->w_p_fcs_chars.stlnc;
}



if (*attr != 0 && ((win_hl_attr(wp, HLF_S) != win_hl_attr(wp, HLF_SNC)
|| !is_curwin || ONE_WINDOW)
|| (wp->w_p_fcs_chars.stl != wp->w_p_fcs_chars.stlnc))) {
return fill;
}
if (is_curwin) {
return '^';
}
return '=';
}





static int fillchar_vsep(win_T *wp, int *attr)
{
*attr = win_hl_attr(wp, HLF_C);
return wp->w_p_fcs_chars.vert;
}




int redrawing(void)
{
return !RedrawingDisabled
&& !(p_lz && char_avail() && !KeyTyped && !do_redraw);
}




int messaging(void)
{
return !(p_lz && char_avail() && !KeyTyped);
}





void showruler(int always)
{
if (!always && !redrawing())
return;
if ((*p_stl != NUL || *curwin->w_p_stl != NUL) && curwin->w_status_height) {
redraw_custom_statusline(curwin);
} else {
win_redr_ruler(curwin, always);
}

if (need_maketitle
|| (p_icon && (stl_syntax & STL_IN_ICON))
|| (p_title && (stl_syntax & STL_IN_TITLE))
)
maketitle();

if (redraw_tabline)
draw_tabline();
}

static void win_redr_ruler(win_T *wp, int always)
{
static bool did_show_ext_ruler = false;


if (!p_ru) {
return;
}





if (wp->w_cursor.lnum > wp->w_buffer->b_ml.ml_line_count)
return;



if (wp == lastwin && lastwin->w_status_height == 0)
if (edit_submode != NULL)
return;

if (*p_ruf) {
int save_called_emsg = called_emsg;

called_emsg = FALSE;
win_redr_custom(wp, TRUE);
if (called_emsg)
set_string_option_direct((char_u *)"rulerformat", -1,
(char_u *)"", OPT_FREE, SID_ERROR);
called_emsg |= save_called_emsg;
return;
}




int empty_line = FALSE;
if (!(State & INSERT)
&& *ml_get_buf(wp->w_buffer, wp->w_cursor.lnum, FALSE) == NUL)
empty_line = TRUE;




validate_virtcol_win(wp);
if ( redraw_cmdline
|| always
|| wp->w_cursor.lnum != wp->w_ru_cursor.lnum
|| wp->w_cursor.col != wp->w_ru_cursor.col
|| wp->w_virtcol != wp->w_ru_virtcol
|| wp->w_cursor.coladd != wp->w_ru_cursor.coladd
|| wp->w_topline != wp->w_ru_topline
|| wp->w_buffer->b_ml.ml_line_count != wp->w_ru_line_count
|| wp->w_topfill != wp->w_ru_topfill
|| empty_line != wp->w_ru_empty) {

int width;
int row;
int fillchar;
int attr;
int off;
bool part_of_status = false;

if (wp->w_status_height) {
row = W_ENDROW(wp);
fillchar = fillchar_status(&attr, wp);
off = wp->w_wincol;
width = wp->w_width;
part_of_status = true;
} else {
row = Rows - 1;
fillchar = ' ';
attr = HL_ATTR(HLF_MSG);
width = Columns;
off = 0;
}


colnr_T virtcol = wp->w_virtcol;
if (wp->w_p_list && wp->w_p_lcs_chars.tab1 == NUL) {
wp->w_p_list = false;
getvvcol(wp, &wp->w_cursor, NULL, &virtcol, NULL);
wp->w_p_list = true;
}

#define RULER_BUF_LEN 70
char_u buffer[RULER_BUF_LEN];





vim_snprintf((char *)buffer, RULER_BUF_LEN, "%" PRId64 ",",
(wp->w_buffer->b_ml.ml_flags & ML_EMPTY) ? (int64_t)0L
: (int64_t)wp->w_cursor.lnum);
size_t len = STRLEN(buffer);
col_print(buffer + len, RULER_BUF_LEN - len,
empty_line ? 0 : (int)wp->w_cursor.col + 1,
(int)virtcol + 1);






int i = (int)STRLEN(buffer);
get_rel_pos(wp, buffer + i + 1, RULER_BUF_LEN - i - 1);
int o = i + vim_strsize(buffer + i + 1);
if (wp->w_status_height == 0) { 
o++;
}
int this_ru_col = ru_col - (Columns - width);
if (this_ru_col < 0) {
this_ru_col = 0;
}


if (this_ru_col < (width + 1) / 2) {
this_ru_col = (width + 1) / 2;
}
if (this_ru_col + o < width) {

while (this_ru_col + o < width && RULER_BUF_LEN > i + 4) {
i += utf_char2bytes(fillchar, buffer + i);
o++;
}
get_rel_pos(wp, buffer + i, RULER_BUF_LEN - i);
}

if (ui_has(kUIMessages) && !part_of_status) {
Array content = ARRAY_DICT_INIT;
Array chunk = ARRAY_DICT_INIT;
ADD(chunk, INTEGER_OBJ(attr));
ADD(chunk, STRING_OBJ(cstr_to_string((char *)buffer)));
ADD(content, ARRAY_OBJ(chunk));
ui_call_msg_ruler(content);
did_show_ext_ruler = true;
} else {
if (did_show_ext_ruler) {
ui_call_msg_ruler((Array)ARRAY_DICT_INIT);
did_show_ext_ruler = false;
}

o = 0;
for (i = 0; buffer[i] != NUL; i += utfc_ptr2len(buffer + i)) {
o += utf_ptr2cells(buffer + i);
if (this_ru_col + o > width) {
buffer[i] = NUL;
break;
}
}

ScreenGrid *grid = part_of_status ? &default_grid : &msg_grid_adj;
grid_puts(grid, buffer, row, this_ru_col + off, attr);
grid_fill(grid, row, row + 1,
this_ru_col + off + (int)STRLEN(buffer), off + width, fillchar,
fillchar, attr);
}

wp->w_ru_cursor = wp->w_cursor;
wp->w_ru_virtcol = wp->w_virtcol;
wp->w_ru_empty = empty_line;
wp->w_ru_topline = wp->w_topline;
wp->w_ru_line_count = wp->w_buffer->b_ml.ml_line_count;
wp->w_ru_topfill = wp->w_topfill;
}
}






int number_width(win_T *wp)
{
int n;
linenr_T lnum;

if (wp->w_p_rnu && !wp->w_p_nu) {

lnum = wp->w_height_inner;
} else {

lnum = wp->w_buffer->b_ml.ml_line_count;
}

if (lnum == wp->w_nrwidth_line_count)
return wp->w_nrwidth_width;
wp->w_nrwidth_line_count = lnum;

n = 0;
do {
lnum /= 10;
++n;
} while (lnum > 0);


if (n < wp->w_p_nuw - 1)
n = wp->w_p_nuw - 1;

wp->w_nrwidth_width = n;
return n;
}


void screen_resize(int width, int height)
{
static int busy = FALSE;



if (updating_screen || busy) {
return;
}

if (width < 0 || height < 0) 
return;

if (State == HITRETURN || State == SETWSIZE) {

State = SETWSIZE;
return;
}





if (curwin->w_buffer == NULL)
return;

++busy;

Rows = height;
Columns = width;
check_shellsize();
height = Rows;
width = Columns;
p_lines = Rows;
p_columns = Columns;
ui_call_grid_resize(1, width, height);

send_grid_resize = true;





if (State != ASKMORE && State != EXTERNCMD && State != CONFIRM) {
screenclear();
}

if (starting != NO_SCREEN) {
maketitle();
changed_line_abv_curs();
invalidate_botline();











if (State == ASKMORE || State == EXTERNCMD || State == CONFIRM
|| exmode_active) {
screenalloc();
if (msg_grid.chars) {
msg_grid_validate();
}


ui_comp_set_screen_valid(true);
repeat_message();
} else {
if (curwin->w_p_scb)
do_check_scrollbind(TRUE);
if (State & CMDLINE) {
redraw_popupmenu = false;
update_screen(NOT_VALID);
redrawcmdline();
if (pum_drawn()) {
cmdline_pum_display(false);
}
} else {
update_topline();
if (pum_drawn()) {



redraw_popupmenu = false;
ins_compl_show_pum();
}
update_screen(NOT_VALID);
if (redrawing()) {
setcursor();
}
}
}
ui_flush();
}
busy--;
}



void check_shellsize(void)
{
if (Rows < min_rows()) {

Rows = min_rows();
}
limit_screen_size();
}


void limit_screen_size(void)
{
if (Columns < MIN_COLUMNS) {
Columns = MIN_COLUMNS;
} else if (Columns > 10000) {
Columns = 10000;
}

if (Rows > 1000) {
Rows = 1000;
}
}

void win_new_shellsize(void)
{
static long old_Rows = 0;
static long old_Columns = 0;

if (old_Rows != Rows) {

if (p_window == old_Rows - 1 || old_Rows == 0) {
p_window = Rows - 1;
}
old_Rows = Rows;
shell_new_rows(); 
}
if (old_Columns != Columns) {
old_Columns = Columns;
shell_new_columns(); 
}
}

win_T *get_win_by_grid_handle(handle_T handle)
{
FOR_ALL_WINDOWS_IN_TAB(wp, curtab) {
if (wp->w_grid.handle == handle) {
return wp;
}
}
return NULL;
}
