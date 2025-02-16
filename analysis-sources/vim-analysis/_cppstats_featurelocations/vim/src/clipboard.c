












#include "vim.h"

#if defined(FEAT_CYGWIN_WIN32_CLIPBOARD)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "winclip.pro"
#endif











#if defined(FEAT_CLIPBOARD) || defined(PROTO)












void
clip_init(int can_use)
{
Clipboard_T *cb;

cb = &clip_star;
for (;;)
{
cb->available = can_use;
cb->owned = FALSE;
cb->start.lnum = 0;
cb->start.col = 0;
cb->end.lnum = 0;
cb->end.col = 0;
cb->state = SELECT_CLEARED;

if (cb == &clip_plus)
break;
cb = &clip_plus;
}
}








void
clip_update_selection(Clipboard_T *clip)
{
pos_T start, end;


if (!redo_VIsual_busy && VIsual_active && (State & NORMAL))
{
if (LT_POS(VIsual, curwin->w_cursor))
{
start = VIsual;
end = curwin->w_cursor;
if (has_mbyte)
end.col += (*mb_ptr2len)(ml_get_cursor()) - 1;
}
else
{
start = curwin->w_cursor;
end = VIsual;
}
if (!EQUAL_POS(clip->start, start)
|| !EQUAL_POS(clip->end, end)
|| clip->vmode != VIsual_mode)
{
clip_clear_selection(clip);
clip->start = start;
clip->end = end;
clip->vmode = VIsual_mode;
clip_free_selection(clip);
clip_own_selection(clip);
clip_gen_set_selection(clip);
}
}
}

static int
clip_gen_own_selection(Clipboard_T *cbd)
{
#if defined(FEAT_XCLIPBOARD)
#if defined(FEAT_GUI)
if (gui.in_use)
return clip_mch_own_selection(cbd);
else
#endif
return clip_xterm_own_selection(cbd);
#else
return clip_mch_own_selection(cbd);
#endif
}

void
clip_own_selection(Clipboard_T *cbd)
{




#if defined(FEAT_X11)


if (cbd->available)
{
int was_owned = cbd->owned;

cbd->owned = (clip_gen_own_selection(cbd) == OK);
if (!was_owned && (cbd == &clip_star || cbd == &clip_plus))
{



if (cbd->owned
&& (get_real_state() == VISUAL
|| get_real_state() == SELECTMODE)
&& (cbd == &clip_star ? clip_isautosel_star()
: clip_isautosel_plus())
&& HL_ATTR(HLF_V) != HL_ATTR(HLF_VNC))
redraw_curbuf_later(INVERTED_ALL);
}
}
#else

if (!cbd->owned && cbd->available)
cbd->owned = (clip_gen_own_selection(cbd) == OK);
#endif
}

static void
clip_gen_lose_selection(Clipboard_T *cbd)
{
#if defined(FEAT_XCLIPBOARD)
#if defined(FEAT_GUI)
if (gui.in_use)
clip_mch_lose_selection(cbd);
else
#endif
clip_xterm_lose_selection(cbd);
#else
clip_mch_lose_selection(cbd);
#endif
}

void
clip_lose_selection(Clipboard_T *cbd)
{
#if defined(FEAT_X11)
int was_owned = cbd->owned;
#endif
int visual_selection = FALSE;

if (cbd == &clip_star || cbd == &clip_plus)
visual_selection = TRUE;

clip_free_selection(cbd);
cbd->owned = FALSE;
if (visual_selection)
clip_clear_selection(cbd);
clip_gen_lose_selection(cbd);
#if defined(FEAT_X11)
if (visual_selection)
{



if (was_owned
&& (get_real_state() == VISUAL
|| get_real_state() == SELECTMODE)
&& (cbd == &clip_star ?
clip_isautosel_star() : clip_isautosel_plus())
&& HL_ATTR(HLF_V) != HL_ATTR(HLF_VNC))
{
update_curbuf(INVERTED_ALL);
setcursor();
cursor_on();
out_flush_cursor(TRUE, FALSE);
}
}
#endif
}

static void
clip_copy_selection(Clipboard_T *clip)
{
if (VIsual_active && (State & NORMAL) && clip->available)
{
clip_update_selection(clip);
clip_free_selection(clip);
clip_own_selection(clip);
if (clip->owned)
clip_get_selection(clip);
clip_gen_set_selection(clip);
}
}






static int global_change_count = 0; 
static int clipboard_needs_update = FALSE; 
static int clip_did_set_selection = TRUE;




void
start_global_changes(void)
{
if (++global_change_count > 1)
return;
clip_unnamed_saved = clip_unnamed;
clipboard_needs_update = FALSE;

if (clip_did_set_selection)
{
clip_unnamed = 0;
clip_did_set_selection = FALSE;
}
}





static int
is_clipboard_needs_update()
{
return clipboard_needs_update;
}




void
end_global_changes(void)
{
if (--global_change_count > 0)

return;
if (!clip_did_set_selection)
{
clip_did_set_selection = TRUE;
clip_unnamed = clip_unnamed_saved;
clip_unnamed_saved = 0;
if (clipboard_needs_update)
{


if (clip_unnamed & CLIP_UNNAMED)
{
clip_own_selection(&clip_star);
clip_gen_set_selection(&clip_star);
}
if (clip_unnamed & CLIP_UNNAMED_PLUS)
{
clip_own_selection(&clip_plus);
clip_gen_set_selection(&clip_plus);
}
}
}
clipboard_needs_update = FALSE;
}




void
clip_auto_select(void)
{
if (clip_isautosel_star())
clip_copy_selection(&clip_star);
if (clip_isautosel_plus())
clip_copy_selection(&clip_plus);
}





int
clip_isautosel_star(void)
{
return (
#if defined(FEAT_GUI)
gui.in_use ? (vim_strchr(p_go, GO_ASEL) != NULL) :
#endif
clip_autoselect_star);
}





int
clip_isautosel_plus(void)
{
return (
#if defined(FEAT_GUI)
gui.in_use ? (vim_strchr(p_go, GO_ASELPLUS) != NULL) :
#endif
clip_autoselect_plus);
}









static int
clip_compare_pos(
int row1,
int col1,
int row2,
int col2)
{
if (row1 > row2) return(1);
if (row1 < row2) return(-1);
if (col1 > col2) return(1);
if (col1 < col2) return(-1);
return(0);
}


#define CLIP_CLEAR 1
#define CLIP_SET 2
#define CLIP_TOGGLE 3





static void
clip_invert_rectangle(
Clipboard_T *cbd UNUSED,
int row_arg,
int col_arg,
int height_arg,
int width_arg,
int invert)
{
int row = row_arg;
int col = col_arg;
int height = height_arg;
int width = width_arg;

#if defined(FEAT_PROP_POPUP)

screen_zindex = CLIP_ZINDEX;

if (col < cbd->min_col)
{
width -= cbd->min_col - col;
col = cbd->min_col;
}
if (width > cbd->max_col - col)
width = cbd->max_col - col;
if (row < cbd->min_row)
{
height -= cbd->min_row - row;
row = cbd->min_row;
}
if (height > cbd->max_row - row + 1)
height = cbd->max_row - row + 1;
#endif
#if defined(FEAT_GUI)
if (gui.in_use)
gui_mch_invert_rectangle(row, col, height, width);
else
#endif
screen_draw_rectangle(row, col, height, width, invert);
#if defined(FEAT_PROP_POPUP)
screen_zindex = 0;
#endif
}









static void
clip_invert_area(
Clipboard_T *cbd,
int row1,
int col1,
int row2,
int col2,
int how)
{
int invert = FALSE;
int max_col;

#if defined(FEAT_PROP_POPUP)
max_col = cbd->max_col - 1;
#else
max_col = Columns - 1;
#endif

if (how == CLIP_SET)
invert = TRUE;


if (clip_compare_pos(row1, col1, row2, col2) > 0)
{
int tmp_row, tmp_col;

tmp_row = row1;
tmp_col = col1;
row1 = row2;
col1 = col2;
row2 = tmp_row;
col2 = tmp_col;
}
else if (how == CLIP_TOGGLE)
invert = TRUE;


if (row1 == row2)
{
clip_invert_rectangle(cbd, row1, col1, 1, col2 - col1, invert);
}
else
{

if (col1 > 0)
{
clip_invert_rectangle(cbd, row1, col1, 1,
(int)Columns - col1, invert);
row1++;
}


if (col2 < max_col)
{
clip_invert_rectangle(cbd, row2, 0, 1, col2, invert);
row2--;
}


if (row2 >= row1)
clip_invert_rectangle(cbd, row1, 0, row2 - row1 + 1,
(int)Columns, invert);
}
}





void
clip_modeless(int button, int is_click, int is_drag)
{
int repeat;

repeat = ((clip_star.mode == SELECT_MODE_CHAR
|| clip_star.mode == SELECT_MODE_LINE)
&& (mod_mask & MOD_MASK_2CLICK))
|| (clip_star.mode == SELECT_MODE_WORD
&& (mod_mask & MOD_MASK_3CLICK));
if (is_click && button == MOUSE_RIGHT)
{


if (clip_star.state == SELECT_CLEARED)
clip_start_selection(mouse_col, mouse_row, FALSE);
clip_process_selection(button, mouse_col, mouse_row, repeat);
}
else if (is_click)
clip_start_selection(mouse_col, mouse_row, repeat);
else if (is_drag)
{


if (clip_star.state != SELECT_CLEARED)
clip_process_selection(button, mouse_col, mouse_row, repeat);
}
else 
clip_process_selection(MOUSE_RELEASE, mouse_col, mouse_row, FALSE);
}





static void
clip_update_modeless_selection(
Clipboard_T *cb,
int row1,
int col1,
int row2,
int col2)
{

if (row1 != cb->start.lnum || col1 != (int)cb->start.col)
{
clip_invert_area(cb, row1, col1, (int)cb->start.lnum, cb->start.col,
CLIP_TOGGLE);
cb->start.lnum = row1;
cb->start.col = col1;
}


if (row2 != cb->end.lnum || col2 != (int)cb->end.col)
{
clip_invert_area(cb, (int)cb->end.lnum, cb->end.col, row2, col2,
CLIP_TOGGLE);
cb->end.lnum = row2;
cb->end.col = col2;
}
}





#define CHAR_CLASS(c) (c <= ' ' ? ' ' : vim_iswordc(c))

static void
clip_get_word_boundaries(Clipboard_T *cb, int row, int col)
{
int start_class;
int temp_col;
char_u *p;
int mboff;

if (row >= screen_Rows || col >= screen_Columns || ScreenLines == NULL)
return;

p = ScreenLines + LineOffset[row];

if (enc_dbcs != 0)
col -= dbcs_screen_head_off(p, p + col);
else if (enc_utf8 && p[col] == 0)
--col;
start_class = CHAR_CLASS(p[col]);

temp_col = col;
for ( ; temp_col > 0; temp_col--)
if (enc_dbcs != 0
&& (mboff = dbcs_screen_head_off(p, p + temp_col - 1)) > 0)
temp_col -= mboff;
else if (CHAR_CLASS(p[temp_col - 1]) != start_class
&& !(enc_utf8 && p[temp_col - 1] == 0))
break;
cb->word_start_col = temp_col;

temp_col = col;
for ( ; temp_col < screen_Columns; temp_col++)
if (enc_dbcs != 0 && dbcs_ptr2cells(p + temp_col) == 2)
++temp_col;
else if (CHAR_CLASS(p[temp_col]) != start_class
&& !(enc_utf8 && p[temp_col] == 0))
break;
cb->word_end_col = temp_col;
}





static int
clip_get_line_end(Clipboard_T *cbd UNUSED, int row)
{
int i;

if (row >= screen_Rows || ScreenLines == NULL)
return 0;
for (i =
#if defined(FEAT_PROP_POPUP)
cbd->max_col;
#else
screen_Columns;
#endif
i > 0; i--)
if (ScreenLines[LineOffset[row] + i - 1] != ' ')
break;
return i;
}




void
clip_start_selection(int col, int row, int repeated_click)
{
Clipboard_T *cb = &clip_star;
#if defined(FEAT_PROP_POPUP)
win_T *wp;
int row_cp = row;
int col_cp = col;

wp = mouse_find_win(&row_cp, &col_cp, FIND_POPUP);
if (wp != NULL && WIN_IS_POPUP(wp)
&& popup_is_in_scrollbar(wp, row_cp, col_cp))

return;
#endif

if (cb->state == SELECT_DONE)
clip_clear_selection(cb);

row = check_row(row);
col = check_col(col);
col = mb_fix_col(col, row);

cb->start.lnum = row;
cb->start.col = col;
cb->end = cb->start;
cb->origin_row = (short_u)cb->start.lnum;
cb->state = SELECT_IN_PROGRESS;
#if defined(FEAT_PROP_POPUP)
if (wp != NULL && WIN_IS_POPUP(wp))
{


cb->min_col = wp->w_wincol + wp->w_popup_border[3];
cb->max_col = wp->w_wincol + popup_width(wp)
- wp->w_popup_border[1] - wp->w_has_scrollbar;
if (cb->max_col > screen_Columns)
cb->max_col = screen_Columns;
cb->min_row = wp->w_winrow + wp->w_popup_border[0];
cb->max_row = wp->w_winrow + popup_height(wp) - 1
- wp->w_popup_border[2];
}
else
{
cb->min_col = 0;
cb->max_col = screen_Columns;
cb->min_row = 0;
cb->max_row = screen_Rows;
}
#endif

if (repeated_click)
{
if (++cb->mode > SELECT_MODE_LINE)
cb->mode = SELECT_MODE_CHAR;
}
else
cb->mode = SELECT_MODE_CHAR;

#if defined(FEAT_GUI)

if (gui.in_use)
gui_undraw_cursor();
#endif

switch (cb->mode)
{
case SELECT_MODE_CHAR:
cb->origin_start_col = cb->start.col;
cb->word_end_col = clip_get_line_end(cb, (int)cb->start.lnum);
break;

case SELECT_MODE_WORD:
clip_get_word_boundaries(cb, (int)cb->start.lnum, cb->start.col);
cb->origin_start_col = cb->word_start_col;
cb->origin_end_col = cb->word_end_col;

clip_invert_area(cb, (int)cb->start.lnum, cb->word_start_col,
(int)cb->end.lnum, cb->word_end_col, CLIP_SET);
cb->start.col = cb->word_start_col;
cb->end.col = cb->word_end_col;
break;

case SELECT_MODE_LINE:
clip_invert_area(cb, (int)cb->start.lnum, 0, (int)cb->start.lnum,
(int)Columns, CLIP_SET);
cb->start.col = 0;
cb->end.col = Columns;
break;
}

cb->prev = cb->start;

#if defined(DEBUG_SELECTION)
printf("Selection started at (%ld,%d)\n", cb->start.lnum, cb->start.col);
#endif
}




void
clip_process_selection(
int button,
int col,
int row,
int_u repeated_click)
{
Clipboard_T *cb = &clip_star;
int diff;
int slen = 1; 

if (button == MOUSE_RELEASE)
{
if (cb->state != SELECT_IN_PROGRESS)
return;


if (cb->start.lnum == cb->end.lnum && cb->start.col == cb->end.col)
{
#if defined(FEAT_GUI)
if (gui.in_use)
gui_update_cursor(FALSE, FALSE);
#endif
cb->state = SELECT_CLEARED;
return;
}

#if defined(DEBUG_SELECTION)
printf("Selection ended: (%ld,%d) to (%ld,%d)\n", cb->start.lnum,
cb->start.col, cb->end.lnum, cb->end.col);
#endif
if (clip_isautosel_star()
|| (
#if defined(FEAT_GUI)
gui.in_use ? (vim_strchr(p_go, GO_ASELML) != NULL) :
#endif
clip_autoselectml))
clip_copy_modeless_selection(FALSE);
#if defined(FEAT_GUI)
if (gui.in_use)
gui_update_cursor(FALSE, FALSE);
#endif

cb->state = SELECT_DONE;
return;
}

row = check_row(row);
col = check_col(col);
col = mb_fix_col(col, row);

if (col == (int)cb->prev.col && row == cb->prev.lnum && !repeated_click)
return;





if (cb->state == SELECT_DONE && button == MOUSE_RIGHT)
{





if (clip_compare_pos(row, col, (int)cb->start.lnum, cb->start.col) < 0
|| (clip_compare_pos(row, col,
(int)cb->end.lnum, cb->end.col) < 0
&& (((cb->start.lnum == cb->end.lnum
&& cb->end.col - col > col - cb->start.col))
|| ((diff = (cb->end.lnum - row) -
(row - cb->start.lnum)) > 0
|| (diff == 0 && col < (int)(cb->start.col +
cb->end.col) / 2)))))
{
cb->origin_row = (short_u)cb->end.lnum;
cb->origin_start_col = cb->end.col - 1;
cb->origin_end_col = cb->end.col;
}
else
{
cb->origin_row = (short_u)cb->start.lnum;
cb->origin_start_col = cb->start.col;
cb->origin_end_col = cb->start.col;
}
if (cb->mode == SELECT_MODE_WORD && !repeated_click)
cb->mode = SELECT_MODE_CHAR;
}


cb->state = SELECT_IN_PROGRESS;

#if defined(DEBUG_SELECTION)
printf("Selection extending to (%d,%d)\n", row, col);
#endif

if (repeated_click && ++cb->mode > SELECT_MODE_LINE)
cb->mode = SELECT_MODE_CHAR;

switch (cb->mode)
{
case SELECT_MODE_CHAR:

if (row != cb->prev.lnum)
cb->word_end_col = clip_get_line_end(cb, row);


if (clip_compare_pos(row, col, cb->origin_row,
cb->origin_start_col) >= 0)
{
if (col >= (int)cb->word_end_col)
clip_update_modeless_selection(cb, cb->origin_row,
cb->origin_start_col, row, (int)Columns);
else
{
if (has_mbyte && mb_lefthalve(row, col))
slen = 2;
clip_update_modeless_selection(cb, cb->origin_row,
cb->origin_start_col, row, col + slen);
}
}
else
{
if (has_mbyte
&& mb_lefthalve(cb->origin_row, cb->origin_start_col))
slen = 2;
if (col >= (int)cb->word_end_col)
clip_update_modeless_selection(cb, row, cb->word_end_col,
cb->origin_row, cb->origin_start_col + slen);
else
clip_update_modeless_selection(cb, row, col,
cb->origin_row, cb->origin_start_col + slen);
}
break;

case SELECT_MODE_WORD:

if (row == cb->prev.lnum && col >= (int)cb->word_start_col
&& col < (int)cb->word_end_col && !repeated_click)
return;


clip_get_word_boundaries(cb, row, col);


if (clip_compare_pos(row, col, cb->origin_row,
cb->origin_start_col) >= 0)
clip_update_modeless_selection(cb, cb->origin_row,
cb->origin_start_col, row, cb->word_end_col);
else
clip_update_modeless_selection(cb, row, cb->word_start_col,
cb->origin_row, cb->origin_end_col);
break;

case SELECT_MODE_LINE:
if (row == cb->prev.lnum && !repeated_click)
return;

if (clip_compare_pos(row, col, cb->origin_row,
cb->origin_start_col) >= 0)
clip_update_modeless_selection(cb, cb->origin_row, 0, row,
(int)Columns);
else
clip_update_modeless_selection(cb, row, 0, cb->origin_row,
(int)Columns);
break;
}

cb->prev.lnum = row;
cb->prev.col = col;

#if defined(DEBUG_SELECTION)
printf("Selection is: (%ld,%d) to (%ld,%d)\n", cb->start.lnum,
cb->start.col, cb->end.lnum, cb->end.col);
#endif
}

#if defined(FEAT_GUI) || defined(PROTO)




void
clip_may_redraw_selection(int row, int col, int len)
{
int start = col;
int end = col + len;

if (clip_star.state != SELECT_CLEARED
&& row >= clip_star.start.lnum
&& row <= clip_star.end.lnum)
{
if (row == clip_star.start.lnum && start < (int)clip_star.start.col)
start = clip_star.start.col;
if (row == clip_star.end.lnum && end > (int)clip_star.end.col)
end = clip_star.end.col;
if (end > start)
clip_invert_area(&clip_star, row, start, row, end, 0);
}
}
#endif




void
clip_clear_selection(Clipboard_T *cbd)
{

if (cbd->state == SELECT_CLEARED)
return;

clip_invert_area(cbd, (int)cbd->start.lnum, cbd->start.col,
(int)cbd->end.lnum, cbd->end.col, CLIP_CLEAR);
cbd->state = SELECT_CLEARED;
}




void
clip_may_clear_selection(int row1, int row2)
{
if (clip_star.state == SELECT_DONE
&& row2 >= clip_star.start.lnum
&& row1 <= clip_star.end.lnum)
clip_clear_selection(&clip_star);
}





void
clip_scroll_selection(
int rows) 
{
int lnum;

if (clip_star.state == SELECT_CLEARED)
return;

lnum = clip_star.start.lnum - rows;
if (lnum <= 0)
clip_star.start.lnum = 0;
else if (lnum >= screen_Rows) 
clip_star.state = SELECT_CLEARED;
else
clip_star.start.lnum = lnum;

lnum = clip_star.end.lnum - rows;
if (lnum < 0) 
clip_star.state = SELECT_CLEARED;
else if (lnum >= screen_Rows)
clip_star.end.lnum = screen_Rows - 1;
else
clip_star.end.lnum = lnum;
}






void
clip_copy_modeless_selection(int both UNUSED)
{
char_u *buffer;
char_u *bufp;
int row;
int start_col;
int end_col;
int line_end_col;
int add_newline_flag = FALSE;
int len;
char_u *p;
int row1 = clip_star.start.lnum;
int col1 = clip_star.start.col;
int row2 = clip_star.end.lnum;
int col2 = clip_star.end.col;


if (ScreenLines == NULL)
return;




if (row1 > row2)
{
row = row1; row1 = row2; row2 = row;
row = col1; col1 = col2; col2 = row;
}
else if (row1 == row2 && col1 > col2)
{
row = col1; col1 = col2; col2 = row;
}
#if defined(FEAT_PROP_POPUP)
if (col1 < clip_star.min_col)
col1 = clip_star.min_col;
if (col2 > clip_star.max_col)
col2 = clip_star.max_col;
if (row1 > clip_star.max_row || row2 < clip_star.min_row)
return;
if (row1 < clip_star.min_row)
row1 = clip_star.min_row;
if (row2 > clip_star.max_row)
row2 = clip_star.max_row;
#endif

p = ScreenLines + LineOffset[row1];
if (enc_dbcs != 0)
col1 -= (*mb_head_off)(p, p + col1);
else if (enc_utf8 && p[col1] == 0)
--col1;


len = (row2 - row1 + 1) * Columns + 1;
if (enc_dbcs != 0)
len *= 2; 
else if (enc_utf8)
len *= MB_MAXBYTES;
buffer = alloc(len);
if (buffer == NULL) 
return;


for (bufp = buffer, row = row1; row <= row2; row++)
{
if (row == row1)
start_col = col1;
else
#if defined(FEAT_PROP_POPUP)
start_col = clip_star.min_col;
#else
start_col = 0;
#endif

if (row == row2)
end_col = col2;
else
#if defined(FEAT_PROP_POPUP)
end_col = clip_star.max_col;
#else
end_col = Columns;
#endif

line_end_col = clip_get_line_end(&clip_star, row);


if (end_col >=
#if defined(FEAT_PROP_POPUP)
clip_star.max_col
#else
Columns
#endif
&& (row < row2 || end_col > line_end_col))
{

end_col = line_end_col;
if (end_col < start_col)
end_col = start_col;


if (row == row2)
add_newline_flag = TRUE;
}


if (row > row1 && !LineWraps[row - 1])
*bufp++ = NL;


if (row < screen_Rows && end_col <= screen_Columns)
{
if (enc_dbcs != 0)
{
int i;

p = ScreenLines + LineOffset[row];
for (i = start_col; i < end_col; ++i)
if (enc_dbcs == DBCS_JPNU && p[i] == 0x8e)
{

*bufp++ = 0x8e;
*bufp++ = ScreenLines2[LineOffset[row] + i];
}
else
{
*bufp++ = p[i];
if (MB_BYTE2LEN(p[i]) == 2)
*bufp++ = p[++i];
}
}
else if (enc_utf8)
{
int off;
int i;
int ci;

off = LineOffset[row];
for (i = start_col; i < end_col; ++i)
{


if (ScreenLinesUC[off + i] == 0)
*bufp++ = ScreenLines[off + i];
else
{
bufp += utf_char2bytes(ScreenLinesUC[off + i], bufp);
for (ci = 0; ci < Screen_mco; ++ci)
{

if (ScreenLinesC[ci][off + i] == 0)
break;
bufp += utf_char2bytes(ScreenLinesC[ci][off + i],
bufp);
}
}

if (ScreenLines[off + i + 1] == 0)
++i;
}
}
else
{
STRNCPY(bufp, ScreenLines + LineOffset[row] + start_col,
end_col - start_col);
bufp += end_col - start_col;
}
}
}


if (add_newline_flag)
*bufp++ = NL;


clip_free_selection(&clip_star);
clip_own_selection(&clip_star);


clip_yank_selection(MCHAR, buffer, (long)(bufp - buffer), &clip_star);


clip_gen_set_selection(&clip_star);

#if defined(FEAT_X11)
if (both)
{

clip_free_selection(&clip_plus);
clip_own_selection(&clip_plus);
clip_yank_selection(MCHAR, buffer, (long)(bufp - buffer), &clip_plus);
clip_gen_set_selection(&clip_plus);
}
#endif
vim_free(buffer);
}

void
clip_gen_set_selection(Clipboard_T *cbd)
{
if (!clip_did_set_selection)
{


if ((cbd == &clip_plus && (clip_unnamed_saved & CLIP_UNNAMED_PLUS))
|| (cbd == &clip_star && (clip_unnamed_saved & CLIP_UNNAMED)))
{
clipboard_needs_update = TRUE;
return;
}
}
#if defined(FEAT_XCLIPBOARD)
#if defined(FEAT_GUI)
if (gui.in_use)
clip_mch_set_selection(cbd);
else
#endif
clip_xterm_set_selection(cbd);
#else
clip_mch_set_selection(cbd);
#endif
}

static void
clip_gen_request_selection(Clipboard_T *cbd)
{
#if defined(FEAT_XCLIPBOARD)
#if defined(FEAT_GUI)
if (gui.in_use)
clip_mch_request_selection(cbd);
else
#endif
clip_xterm_request_selection(cbd);
#else
clip_mch_request_selection(cbd);
#endif
}

#if (defined(FEAT_X11) && defined(FEAT_XCLIPBOARD) && defined(USE_SYSTEM)) || defined(PROTO)

static int
clip_x11_owner_exists(Clipboard_T *cbd)
{
return XGetSelectionOwner(X_DISPLAY, cbd->sel_atom) != None;
}
#endif

#if (defined(FEAT_X11) && defined(USE_SYSTEM)) || defined(PROTO)
int
clip_gen_owner_exists(Clipboard_T *cbd UNUSED)
{
#if defined(FEAT_XCLIPBOARD)
#if defined(FEAT_GUI_GTK)
if (gui.in_use)
return clip_gtk_owner_exists(cbd);
else
#endif
return clip_x11_owner_exists(cbd);
#else
return TRUE;
#endif
}
#endif





char *
check_clipboard_option(void)
{
int new_unnamed = 0;
int new_autoselect_star = FALSE;
int new_autoselect_plus = FALSE;
int new_autoselectml = FALSE;
int new_html = FALSE;
regprog_T *new_exclude_prog = NULL;
char *errmsg = NULL;
char_u *p;

for (p = p_cb; *p != NUL; )
{
if (STRNCMP(p, "unnamed", 7) == 0 && (p[7] == ',' || p[7] == NUL))
{
new_unnamed |= CLIP_UNNAMED;
p += 7;
}
else if (STRNCMP(p, "unnamedplus", 11) == 0
&& (p[11] == ',' || p[11] == NUL))
{
new_unnamed |= CLIP_UNNAMED_PLUS;
p += 11;
}
else if (STRNCMP(p, "autoselect", 10) == 0
&& (p[10] == ',' || p[10] == NUL))
{
new_autoselect_star = TRUE;
p += 10;
}
else if (STRNCMP(p, "autoselectplus", 14) == 0
&& (p[14] == ',' || p[14] == NUL))
{
new_autoselect_plus = TRUE;
p += 14;
}
else if (STRNCMP(p, "autoselectml", 12) == 0
&& (p[12] == ',' || p[12] == NUL))
{
new_autoselectml = TRUE;
p += 12;
}
else if (STRNCMP(p, "html", 4) == 0 && (p[4] == ',' || p[4] == NUL))
{
new_html = TRUE;
p += 4;
}
else if (STRNCMP(p, "exclude:", 8) == 0 && new_exclude_prog == NULL)
{
p += 8;
new_exclude_prog = vim_regcomp(p, RE_MAGIC);
if (new_exclude_prog == NULL)
errmsg = e_invarg;
break;
}
else
{
errmsg = e_invarg;
break;
}
if (*p == ',')
++p;
}
if (errmsg == NULL)
{
clip_unnamed = new_unnamed;
clip_autoselect_star = new_autoselect_star;
clip_autoselect_plus = new_autoselect_plus;
clip_autoselectml = new_autoselectml;
clip_html = new_html;
vim_regfree(clip_exclude_prog);
clip_exclude_prog = new_exclude_prog;
#if defined(FEAT_GUI_GTK)
if (gui.in_use)
{
gui_gtk_set_selection_targets();
gui_gtk_set_dnd_targets();
}
#endif
}
else
vim_regfree(new_exclude_prog);

return errmsg;
}





#if defined(FEAT_XCLIPBOARD) || defined(FEAT_GUI_X11) || defined(PROTO)
#include <X11/Xatom.h>
#include <X11/Intrinsic.h>





void
open_app_context(void)
{
if (app_context == NULL)
{
XtToolkitInitialize();
app_context = XtCreateApplicationContext();
}
}

static Atom vim_atom; 
static Atom vimenc_atom; 
static Atom utf8_atom;
static Atom compound_text_atom;
static Atom text_atom;
static Atom targets_atom;
static Atom timestamp_atom; 

void
x11_setup_atoms(Display *dpy)
{
vim_atom = XInternAtom(dpy, VIM_ATOM_NAME, False);
vimenc_atom = XInternAtom(dpy, VIMENC_ATOM_NAME,False);
utf8_atom = XInternAtom(dpy, "UTF8_STRING", False);
compound_text_atom = XInternAtom(dpy, "COMPOUND_TEXT", False);
text_atom = XInternAtom(dpy, "TEXT", False);
targets_atom = XInternAtom(dpy, "TARGETS", False);
clip_star.sel_atom = XA_PRIMARY;
clip_plus.sel_atom = XInternAtom(dpy, "CLIPBOARD", False);
timestamp_atom = XInternAtom(dpy, "TIMESTAMP", False);
}





static Boolean
clip_x11_convert_selection_cb(
Widget w UNUSED,
Atom *sel_atom,
Atom *target,
Atom *type,
XtPointer *value,
long_u *length,
int *format)
{
static char_u *save_result = NULL;
static long_u save_length = 0;
char_u *string;
int motion_type;
Clipboard_T *cbd;
int i;

if (*sel_atom == clip_plus.sel_atom)
cbd = &clip_plus;
else
cbd = &clip_star;

if (!cbd->owned)
return False; 


if (*target == targets_atom)
{
static Atom array[7];

*value = (XtPointer)array;
i = 0;
array[i++] = targets_atom;
array[i++] = vimenc_atom;
array[i++] = vim_atom;
if (enc_utf8)
array[i++] = utf8_atom;
array[i++] = XA_STRING;
array[i++] = text_atom;
array[i++] = compound_text_atom;

*type = XA_ATOM;


*format = 32;
*length = i;
return True;
}

if ( *target != XA_STRING
&& *target != vimenc_atom
&& (*target != utf8_atom || !enc_utf8)
&& *target != vim_atom
&& *target != text_atom
&& *target != compound_text_atom)
return False;

clip_get_selection(cbd);
motion_type = clip_convert_selection(&string, length, cbd);
if (motion_type < 0)
return False;


if (*target == vim_atom)
(*length)++;


if (*target == vimenc_atom)
*length += STRLEN(p_enc) + 2;

if (save_length < *length || save_length / 2 >= *length)
*value = XtRealloc((char *)save_result, (Cardinal)*length + 1);
else
*value = save_result;
if (*value == NULL)
{
vim_free(string);
return False;
}
save_result = (char_u *)*value;
save_length = *length;

if (*target == XA_STRING || (*target == utf8_atom && enc_utf8))
{
mch_memmove(save_result, string, (size_t)(*length));
*type = *target;
}
else if (*target == compound_text_atom || *target == text_atom)
{
XTextProperty text_prop;
char *string_nt = (char *)save_result;
int conv_result;


mch_memmove(string_nt, string, (size_t)*length);
string_nt[*length] = NUL;
conv_result = XmbTextListToTextProperty(X_DISPLAY, (char **)&string_nt,
1, XCompoundTextStyle, &text_prop);
if (conv_result != Success)
{
vim_free(string);
return False;
}
*value = (XtPointer)(text_prop.value); 
*length = text_prop.nitems;
*type = compound_text_atom;
XtFree((char *)save_result);
save_result = (char_u *)*value;
save_length = *length;
}
else if (*target == vimenc_atom)
{
int l = STRLEN(p_enc);

save_result[0] = motion_type;
STRCPY(save_result + 1, p_enc);
mch_memmove(save_result + l + 2, string, (size_t)(*length - l - 2));
*type = vimenc_atom;
}
else
{
save_result[0] = motion_type;
mch_memmove(save_result + 1, string, (size_t)(*length - 1));
*type = vim_atom;
}
*format = 8; 
vim_free(string);
return True;
}

static void
clip_x11_lose_ownership_cb(Widget w UNUSED, Atom *sel_atom)
{
if (*sel_atom == clip_plus.sel_atom)
clip_lose_selection(&clip_plus);
else
clip_lose_selection(&clip_star);
}

static void
clip_x11_notify_cb(Widget w UNUSED, Atom *sel_atom UNUSED, Atom *target UNUSED)
{

}




static void
clip_x11_timestamp_cb(
Widget w,
XtPointer n UNUSED,
XEvent *event,
Boolean *cont UNUSED)
{
Atom actual_type;
int format;
unsigned long nitems, bytes_after;
unsigned char *prop=NULL;
XPropertyEvent *xproperty=&event->xproperty;



if (event->type != PropertyNotify || xproperty->state
|| (xproperty->atom != clip_star.sel_atom
&& xproperty->atom != clip_plus.sel_atom))
return;

if (XGetWindowProperty(xproperty->display, xproperty->window,
xproperty->atom, 0, 0, False, timestamp_atom, &actual_type, &format,
&nitems, &bytes_after, &prop))
return;

if (prop)
XFree(prop);


if (actual_type != timestamp_atom || format != 32)
return;


if (XtOwnSelection(w, xproperty->atom, xproperty->time,
clip_x11_convert_selection_cb, clip_x11_lose_ownership_cb,
clip_x11_notify_cb) == OK)
{


if (xproperty->atom == clip_plus.sel_atom)
clip_plus.owned = TRUE;
else
clip_star.owned = TRUE;
}
}

void
x11_setup_selection(Widget w)
{
XtAddEventHandler(w, PropertyChangeMask, False,
clip_x11_timestamp_cb, (XtPointer)NULL);
}

static void
clip_x11_request_selection_cb(
Widget w UNUSED,
XtPointer success,
Atom *sel_atom,
Atom *type,
XtPointer value,
long_u *length,
int *format)
{
int motion_type = MAUTO;
long_u len;
char_u *p;
char **text_list = NULL;
Clipboard_T *cbd;
char_u *tmpbuf = NULL;

if (*sel_atom == clip_plus.sel_atom)
cbd = &clip_plus;
else
cbd = &clip_star;

if (value == NULL || *length == 0)
{
clip_free_selection(cbd); 
*(int *)success = FALSE;
return;
}
p = (char_u *)value;
len = *length;
if (*type == vim_atom)
{
motion_type = *p++;
len--;
}

else if (*type == vimenc_atom)
{
char_u *enc;
vimconv_T conv;
int convlen;

motion_type = *p++;
--len;

enc = p;
p += STRLEN(p) + 1;
len -= p - enc;



conv.vc_type = CONV_NONE;
convert_setup(&conv, enc, p_enc);
if (conv.vc_type != CONV_NONE)
{
convlen = len; 
tmpbuf = string_convert(&conv, p, &convlen);
len = convlen;
if (tmpbuf != NULL)
p = tmpbuf;
convert_setup(&conv, NULL, NULL);
}
}

else if (*type == compound_text_atom
|| *type == utf8_atom
|| (enc_dbcs != 0 && *type == text_atom))
{
XTextProperty text_prop;
int n_text = 0;
int status;

text_prop.value = (unsigned char *)value;
text_prop.encoding = *type;
text_prop.format = *format;
text_prop.nitems = len;
#if defined(X_HAVE_UTF8_STRING)
if (*type == utf8_atom)
status = Xutf8TextPropertyToTextList(X_DISPLAY, &text_prop,
&text_list, &n_text);
else
#endif
status = XmbTextPropertyToTextList(X_DISPLAY, &text_prop,
&text_list, &n_text);
if (status != Success || n_text < 1)
{
*(int *)success = FALSE;
return;
}
p = (char_u *)text_list[0];
len = STRLEN(p);
}
clip_yank_selection(motion_type, p, (long)len, cbd);

if (text_list != NULL)
XFreeStringList(text_list);
vim_free(tmpbuf);
XtFree((char *)value);
*(int *)success = TRUE;
}

void
clip_x11_request_selection(
Widget myShell,
Display *dpy,
Clipboard_T *cbd)
{
XEvent event;
Atom type;
static int success;
int i;
time_t start_time;
int timed_out = FALSE;

for (i = 0; i < 6; i++)
{
switch (i)
{
case 0: type = vimenc_atom; break;
case 1: type = vim_atom; break;
case 2: type = utf8_atom; break;
case 3: type = compound_text_atom; break;
case 4: type = text_atom; break;
default: type = XA_STRING;
}
if (type == utf8_atom
#if defined(X_HAVE_UTF8_STRING)
&& !enc_utf8
#endif
)


continue;
success = MAYBE;
XtGetSelectionValue(myShell, cbd->sel_atom, type,
clip_x11_request_selection_cb, (XtPointer)&success, CurrentTime);



XFlush(dpy);






start_time = time(NULL);
while (success == MAYBE)
{
if (XCheckTypedEvent(dpy, PropertyNotify, &event)
|| XCheckTypedEvent(dpy, SelectionNotify, &event)
|| XCheckTypedEvent(dpy, SelectionRequest, &event))
{






XtDispatchEvent(&event);
continue;
}






if (time(NULL) > start_time + 2)
{
timed_out = TRUE;
break;
}


XSync(dpy, False);


ui_delay(1L, TRUE);
}

if (success == TRUE)
return;



if (timed_out)
break;
}


yank_cut_buffer0(dpy, cbd);
}

void
clip_x11_lose_selection(Widget myShell, Clipboard_T *cbd)
{
XtDisownSelection(myShell, cbd->sel_atom,
XtLastTimestampProcessed(XtDisplay(myShell)));
}

int
clip_x11_own_selection(Widget myShell, Clipboard_T *cbd)
{




#if defined(FEAT_GUI)
if (gui.in_use)
{
if (XtOwnSelection(myShell, cbd->sel_atom,
XtLastTimestampProcessed(XtDisplay(myShell)),
clip_x11_convert_selection_cb, clip_x11_lose_ownership_cb,
clip_x11_notify_cb) == False)
return FAIL;
}
else
#endif
{
if (!XChangeProperty(XtDisplay(myShell), XtWindow(myShell),
cbd->sel_atom, timestamp_atom, 32, PropModeAppend, NULL, 0))
return FAIL;
}

XFlush(XtDisplay(myShell));
return OK;
}





void
clip_x11_set_selection(Clipboard_T *cbd UNUSED)
{
}

#endif

#if defined(FEAT_XCLIPBOARD) || defined(FEAT_GUI_X11) || defined(FEAT_GUI_GTK) || defined(PROTO)




void
yank_cut_buffer0(Display *dpy, Clipboard_T *cbd)
{
int nbytes = 0;
char_u *buffer = (char_u *)XFetchBuffer(dpy, &nbytes, 0);

if (nbytes > 0)
{
int done = FALSE;





if (has_mbyte)
{
char_u *conv_buf;
vimconv_T vc;

vc.vc_type = CONV_NONE;
if (convert_setup(&vc, (char_u *)"latin1", p_enc) == OK)
{
conv_buf = string_convert(&vc, buffer, &nbytes);
if (conv_buf != NULL)
{
clip_yank_selection(MCHAR, conv_buf, (long)nbytes, cbd);
vim_free(conv_buf);
done = TRUE;
}
convert_setup(&vc, NULL, NULL);
}
}
if (!done) 
clip_yank_selection(MCHAR, buffer, (long)nbytes, cbd);
XFree((void *)buffer);
if (p_verbose > 0)
{
verbose_enter();
verb_msg(_("Used CUT_BUFFER0 instead of empty selection"));
verbose_leave();
}
}
}
#endif




























#if (defined(FEAT_X11) && defined(FEAT_CLIPBOARD)) || defined(PROTO)
void
x11_export_final_selection(void)
{
Display *dpy;
char_u *str = NULL;
long_u len = 0;
int motion_type = -1;

#if defined(FEAT_GUI)
if (gui.in_use)
dpy = X_DISPLAY;
else
#endif
#if defined(FEAT_XCLIPBOARD)
dpy = xterm_dpy;
#else
return;
#endif


if (clip_plus.owned)
motion_type = clip_convert_selection(&str, &len, &clip_plus);
else if (clip_star.owned)
motion_type = clip_convert_selection(&str, &len, &clip_star);


if (dpy != NULL && str != NULL && motion_type >= 0
&& len < 1024*1024 && len > 0)
{
int ok = TRUE;




if (has_mbyte)
{
vimconv_T vc;

vc.vc_type = CONV_NONE;
if (convert_setup(&vc, p_enc, (char_u *)"latin1") == OK)
{
int intlen = len;
char_u *conv_str;

vc.vc_fail = TRUE;
conv_str = string_convert(&vc, str, &intlen);
len = intlen;
if (conv_str != NULL)
{
vim_free(str);
str = conv_str;
}
else
{
ok = FALSE;
}
convert_setup(&vc, NULL, NULL);
}
else
{
ok = FALSE;
}
}



if (ok)
{
XStoreBuffer(dpy, (char *)str, (int)len, 0);
XFlush(dpy);
}
}

vim_free(str);
}
#endif

void
clip_free_selection(Clipboard_T *cbd)
{
yankreg_T *y_ptr = get_y_current();

if (cbd == &clip_plus)
set_y_current(get_y_register(PLUS_REGISTER));
else
set_y_current(get_y_register(STAR_REGISTER));
free_yank_all();
get_y_current()->y_size = 0;
set_y_current(y_ptr);
}




void
clip_get_selection(Clipboard_T *cbd)
{
yankreg_T *old_y_previous, *old_y_current;
pos_T old_cursor;
pos_T old_visual;
int old_visual_mode;
colnr_T old_curswant;
int old_set_curswant;
pos_T old_op_start, old_op_end;
oparg_T oa;
cmdarg_T ca;

if (cbd->owned)
{
if ((cbd == &clip_plus
&& get_y_register(PLUS_REGISTER)->y_array != NULL)
|| (cbd == &clip_star
&& get_y_register(STAR_REGISTER)->y_array != NULL))
return;


old_y_previous = get_y_previous();
old_y_current = get_y_current();
old_cursor = curwin->w_cursor;
old_curswant = curwin->w_curswant;
old_set_curswant = curwin->w_set_curswant;
old_op_start = curbuf->b_op_start;
old_op_end = curbuf->b_op_end;
old_visual = VIsual;
old_visual_mode = VIsual_mode;
clear_oparg(&oa);
oa.regname = (cbd == &clip_plus ? '+' : '*');
oa.op_type = OP_YANK;
vim_memset(&ca, 0, sizeof(ca));
ca.oap = &oa;
ca.cmdchar = 'y';
ca.count1 = 1;
ca.retval = CA_NO_ADJ_OP_END;
do_pending_operator(&ca, 0, TRUE);
set_y_previous(old_y_previous);
set_y_current(old_y_current);
curwin->w_cursor = old_cursor;
changed_cline_bef_curs(); 
curwin->w_curswant = old_curswant;
curwin->w_set_curswant = old_set_curswant;
curbuf->b_op_start = old_op_start;
curbuf->b_op_end = old_op_end;
VIsual = old_visual;
VIsual_mode = old_visual_mode;
}
else if (!is_clipboard_needs_update())
{
clip_free_selection(cbd);


clip_gen_request_selection(cbd);
}
}




void
clip_yank_selection(
int type,
char_u *str,
long len,
Clipboard_T *cbd)
{
yankreg_T *y_ptr;

if (cbd == &clip_plus)
y_ptr = get_y_register(PLUS_REGISTER);
else
y_ptr = get_y_register(STAR_REGISTER);

clip_free_selection(cbd);

str_to_reg(y_ptr, type, str, len, 0L, FALSE);
}






int
clip_convert_selection(char_u **str, long_u *len, Clipboard_T *cbd)
{
char_u *p;
int lnum;
int i, j;
int_u eolsize;
yankreg_T *y_ptr;

if (cbd == &clip_plus)
y_ptr = get_y_register(PLUS_REGISTER);
else
y_ptr = get_y_register(STAR_REGISTER);

#if defined(USE_CRNL)
eolsize = 2;
#else
eolsize = 1;
#endif

*str = NULL;
*len = 0;
if (y_ptr->y_array == NULL)
return -1;

for (i = 0; i < y_ptr->y_size; i++)
*len += (long_u)STRLEN(y_ptr->y_array[i]) + eolsize;


if (y_ptr->y_type == MCHAR && *len >= eolsize)
*len -= eolsize;

p = *str = alloc(*len + 1); 
if (p == NULL)
return -1;
lnum = 0;
for (i = 0, j = 0; i < (int)*len; i++, j++)
{
if (y_ptr->y_array[lnum][j] == '\n')
p[i] = NUL;
else if (y_ptr->y_array[lnum][j] == NUL)
{
#if defined(USE_CRNL)
p[i++] = '\r';
#endif
p[i] = '\n';
lnum++;
j = -1;
}
else
p[i] = y_ptr->y_array[lnum][j];
}
return y_ptr->y_type;
}





int
may_get_selection(int regname)
{
if (regname == '*')
{
if (!clip_star.available)
regname = 0;
else
clip_get_selection(&clip_star);
}
else if (regname == '+')
{
if (!clip_plus.available)
regname = 0;
else
clip_get_selection(&clip_plus);
}
return regname;
}




void
may_set_selection(void)
{
if ((get_y_current() == get_y_register(STAR_REGISTER))
&& clip_star.available)
{
clip_own_selection(&clip_star);
clip_gen_set_selection(&clip_star);
}
else if ((get_y_current() == get_y_register(PLUS_REGISTER))
&& clip_plus.available)
{
clip_own_selection(&clip_plus);
clip_gen_set_selection(&clip_plus);
}
}





void
adjust_clip_reg(int *rp)
{


if (*rp == 0 && (clip_unnamed != 0 || clip_unnamed_saved != 0))
{
if (clip_unnamed != 0)
*rp = ((clip_unnamed & CLIP_UNNAMED_PLUS) && clip_plus.available)
? '+' : '*';
else
*rp = ((clip_unnamed_saved & CLIP_UNNAMED_PLUS)
&& clip_plus.available) ? '+' : '*';
}
if (!clip_star.available && *rp == '*')
*rp = 0;
if (!clip_plus.available && *rp == '+')
*rp = 0;
}

#endif 
