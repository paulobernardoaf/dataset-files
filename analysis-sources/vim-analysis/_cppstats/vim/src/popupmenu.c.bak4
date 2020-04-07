











#include "vim.h"

static pumitem_T *pum_array = NULL; 
static int pum_size; 
static int pum_selected; 
static int pum_first = 0; 

static int call_update_screen = FALSE;

static int pum_height; 
static int pum_width; 
static int pum_base_width; 
static int pum_kind_width; 
static int pum_extra_width; 
static int pum_scrollbar; 

static int pum_row; 
static int pum_col; 

static win_T *pum_window = NULL;
static int pum_win_row;
static int pum_win_height;
static int pum_win_col;
static int pum_win_wcol;
static int pum_win_width;



static int pum_pretend_not_visible = FALSE;



static int pum_will_redraw = FALSE;

static int pum_set_selected(int n, int repeat);

#define PUM_DEF_HEIGHT 10

static void
pum_compute_size(void)
{
int i;
int w;


pum_base_width = 0;
pum_kind_width = 0;
pum_extra_width = 0;
for (i = 0; i < pum_size; ++i)
{
w = vim_strsize(pum_array[i].pum_text);
if (pum_base_width < w)
pum_base_width = w;
if (pum_array[i].pum_kind != NULL)
{
w = vim_strsize(pum_array[i].pum_kind) + 1;
if (pum_kind_width < w)
pum_kind_width = w;
}
if (pum_array[i].pum_extra != NULL)
{
w = vim_strsize(pum_array[i].pum_extra) + 1;
if (pum_extra_width < w)
pum_extra_width = w;
}
}
}







void
pum_display(
pumitem_T *array,
int size,
int selected) 

{
int def_width;
int max_width;
int context_lines;
int cursor_col;
int above_row;
int below_row;
int redo_count = 0;
#if defined(FEAT_QUICKFIX)
win_T *pvwin;
#endif

do
{
def_width = p_pw;
above_row = 0;
below_row = cmdline_row;



pum_array = (pumitem_T *)1;
validate_cursor_col();
pum_array = NULL;



pum_window = curwin;
pum_win_row = curwin->w_wrow + W_WINROW(curwin);
pum_win_height = curwin->w_height;
pum_win_col = curwin->w_wincol;
pum_win_wcol = curwin->w_wcol;
pum_win_width = curwin->w_width;

#if defined(FEAT_QUICKFIX)
FOR_ALL_WINDOWS(pvwin)
if (pvwin->w_p_pvw)
break;
if (pvwin != NULL)
{
if (W_WINROW(pvwin) < W_WINROW(curwin))
above_row = W_WINROW(pvwin) + pvwin->w_height;
else if (W_WINROW(pvwin) > W_WINROW(curwin) + curwin->w_height)
below_row = W_WINROW(pvwin);
}
#endif




if (size < PUM_DEF_HEIGHT)
pum_height = size;
else
pum_height = PUM_DEF_HEIGHT;
if (p_ph > 0 && pum_height > p_ph)
pum_height = p_ph;



if (pum_win_row + 2 >= below_row - pum_height
&& pum_win_row - above_row > (below_row - above_row) / 2)
{



if (curwin->w_wrow - curwin->w_cline_row >= 2)
context_lines = 2;
else
context_lines = curwin->w_wrow - curwin->w_cline_row;

if (pum_win_row >= size + context_lines)
{
pum_row = pum_win_row - size - context_lines;
pum_height = size;
}
else
{
pum_row = 0;
pum_height = pum_win_row - context_lines;
}
if (p_ph > 0 && pum_height > p_ph)
{
pum_row += pum_height - p_ph;
pum_height = p_ph;
}
}
else
{



validate_cheight();
if (curwin->w_cline_row
+ curwin->w_cline_height - curwin->w_wrow >= 3)
context_lines = 3;
else
context_lines = curwin->w_cline_row
+ curwin->w_cline_height - curwin->w_wrow;

pum_row = pum_win_row + context_lines;
if (size > below_row - pum_row)
pum_height = below_row - pum_row;
else
pum_height = size;
if (p_ph > 0 && pum_height > p_ph)
pum_height = p_ph;
}


if (pum_height < 1 || (pum_height == 1 && size > 1))
return;

#if defined(FEAT_QUICKFIX)

if (pvwin != NULL && pum_row < above_row && pum_height > above_row)
{
pum_row = above_row;
pum_height = pum_win_row - above_row;
}
#endif

pum_array = array;
pum_size = size;
pum_compute_size();
max_width = pum_base_width;


#if defined(FEAT_RIGHTLEFT)
if (curwin->w_p_rl)
cursor_col = curwin->w_wincol + curwin->w_width
- curwin->w_wcol - 1;
else
#endif
cursor_col = curwin->w_wincol + curwin->w_wcol;


if (pum_height < size)
{
pum_scrollbar = 1;
++max_width;
}
else
pum_scrollbar = 0;

if (def_width < max_width)
def_width = max_width;

if (((cursor_col < Columns - p_pw
|| cursor_col < Columns - max_width)
#if defined(FEAT_RIGHTLEFT)
&& !curwin->w_p_rl)
|| (curwin->w_p_rl
&& (cursor_col > p_pw || cursor_col > max_width)
#endif
))
{

pum_col = cursor_col;


#if defined(FEAT_RIGHTLEFT)
if (curwin->w_p_rl)
pum_width = pum_col - pum_scrollbar + 1;
else
#endif
pum_width = Columns - pum_col - pum_scrollbar;

if (pum_width > max_width + pum_kind_width + pum_extra_width + 1
&& pum_width > p_pw)
{


pum_width = max_width + pum_kind_width + pum_extra_width + 1;
if (pum_width < p_pw)
pum_width = p_pw;
}
else if (((cursor_col > p_pw || cursor_col > max_width)
#if defined(FEAT_RIGHTLEFT)
&& !curwin->w_p_rl)
|| (curwin->w_p_rl && (cursor_col < Columns - p_pw
|| cursor_col < Columns - max_width)
#endif
))
{

#if defined(FEAT_RIGHTLEFT)
if (curwin->w_p_rl
&& W_ENDCOL(curwin) < max_width + pum_scrollbar + 1)
{
pum_col = cursor_col + max_width + pum_scrollbar + 1;
if (pum_col >= Columns)
pum_col = Columns - 1;
}
else if (!curwin->w_p_rl)
#endif
{
if (curwin->w_wincol > Columns - max_width - pum_scrollbar
&& max_width <= p_pw)
{

pum_col = Columns - max_width - pum_scrollbar;
if (pum_col < 0)
pum_col = 0;
}
}

#if defined(FEAT_RIGHTLEFT)
if (curwin->w_p_rl)
pum_width = pum_col - pum_scrollbar + 1;
else
#endif
pum_width = Columns - pum_col - pum_scrollbar;

if (pum_width < p_pw)
{
pum_width = p_pw;
#if defined(FEAT_RIGHTLEFT)
if (curwin->w_p_rl)
{
if (pum_width > pum_col)
pum_width = pum_col;
}
else
#endif
{
if (pum_width >= Columns - pum_col)
pum_width = Columns - pum_col - 1;
}
}
else if (pum_width > max_width + pum_kind_width
+ pum_extra_width + 1
&& pum_width > p_pw)
{
pum_width = max_width + pum_kind_width
+ pum_extra_width + 1;
if (pum_width < p_pw)
pum_width = p_pw;
}
}

}
else if (Columns < def_width)
{

#if defined(FEAT_RIGHTLEFT)
if (curwin->w_p_rl)
pum_col = Columns - 1;
else
#endif
pum_col = 0;
pum_width = Columns - 1;
}
else
{
if (max_width > p_pw)
max_width = p_pw; 
#if defined(FEAT_RIGHTLEFT)
if (curwin->w_p_rl)
pum_col = max_width - 1;
else
#endif
pum_col = Columns - max_width;
pum_width = max_width - pum_scrollbar;
}




} while (pum_set_selected(selected, redo_count) && ++redo_count <= 2);
}





void
pum_call_update_screen()
{
call_update_screen = TRUE;




curwin->w_valid &= ~(VALID_CROW|VALID_CHEIGHT);
validate_cursor();
}





int
pum_under_menu(int row, int col)
{
return pum_will_redraw
&& row >= pum_row
&& row < pum_row + pum_height
&& col >= pum_col - 1
&& col < pum_col + pum_width;
}




void
pum_redraw(void)
{
int row = pum_row;
int col;
int attr_norm = highlight_attr[HLF_PNI];
int attr_select = highlight_attr[HLF_PSI];
int attr_scroll = highlight_attr[HLF_PSB];
int attr_thumb = highlight_attr[HLF_PST];
int attr;
int i;
int idx;
char_u *s;
char_u *p = NULL;
int totwidth, width, w;
int thumb_pos = 0;
int thumb_height = 1;
int round;
int n;

if (call_update_screen)
{
call_update_screen = FALSE;


pum_will_redraw = TRUE;
update_screen(0);
pum_will_redraw = FALSE;
}


if (pum_first > pum_size - pum_height)
pum_first = pum_size - pum_height;

if (pum_scrollbar)
{
thumb_height = pum_height * pum_height / pum_size;
if (thumb_height == 0)
thumb_height = 1;
thumb_pos = (pum_first * (pum_height - thumb_height)
+ (pum_size - pum_height) / 2)
/ (pum_size - pum_height);
}

#if defined(FEAT_PROP_POPUP)


screen_zindex = POPUPMENU_ZINDEX;
#endif

for (i = 0; i < pum_height; ++i)
{
idx = i + pum_first;
attr = (idx == pum_selected) ? attr_select : attr_norm;


#if defined(FEAT_RIGHTLEFT)
if (curwin->w_p_rl)
{
if (pum_col < curwin->w_wincol + curwin->w_width - 1)
screen_putchar(' ', row, pum_col + 1, attr);
}
else
#endif
if (pum_col > 0)
screen_putchar(' ', row, pum_col - 1, attr);



col = pum_col;
totwidth = 0;
for (round = 1; round <= 3; ++round)
{
width = 0;
s = NULL;
switch (round)
{
case 1: p = pum_array[idx].pum_text; break;
case 2: p = pum_array[idx].pum_kind; break;
case 3: p = pum_array[idx].pum_extra; break;
}
if (p != NULL)
for ( ; ; MB_PTR_ADV(p))
{
if (s == NULL)
s = p;
w = ptr2cells(p);
if (*p == NUL || *p == TAB || totwidth + w > pum_width)
{


char_u *st;
int saved = *p;

if (saved != NUL)
*p = NUL;
st = transstr(s);
if (saved != NUL)
*p = saved;
#if defined(FEAT_RIGHTLEFT)
if (curwin->w_p_rl)
{
if (st != NULL)
{
char_u *rt = reverse_text(st);

if (rt != NULL)
{
char_u *rt_start = rt;
int size;

size = vim_strsize(rt);
if (size > pum_width)
{
do
{
size -= has_mbyte
? (*mb_ptr2cells)(rt) : 1;
MB_PTR_ADV(rt);
} while (size > pum_width);

if (size < pum_width)
{





*(--rt) = '<';
size++;
}
}
screen_puts_len(rt, (int)STRLEN(rt),
row, col - size + 1, attr);
vim_free(rt_start);
}
vim_free(st);
}
col -= width;
}
else
#endif
{
if (st != NULL)
{
screen_puts_len(st, (int)STRLEN(st), row, col,
attr);
vim_free(st);
}
col += width;
}

if (*p != TAB)
break;


#if defined(FEAT_RIGHTLEFT)
if (curwin->w_p_rl)
{
screen_puts_len((char_u *)" ", 2, row, col - 1,
attr);
col -= 2;
}
else
#endif
{
screen_puts_len((char_u *)" ", 2, row, col, attr);
col += 2;
}
totwidth += 2;
s = NULL; 
width = 0;
}
else
width += w;
}

if (round > 1)
n = pum_kind_width + 1;
else
n = 1;


if (round == 3
|| (round == 2 && pum_array[idx].pum_extra == NULL)
|| (round == 1 && pum_array[idx].pum_kind == NULL
&& pum_array[idx].pum_extra == NULL)
|| pum_base_width + n >= pum_width)
break;
#if defined(FEAT_RIGHTLEFT)
if (curwin->w_p_rl)
{
screen_fill(row, row + 1, pum_col - pum_base_width - n + 1,
col + 1, ' ', ' ', attr);
col = pum_col - pum_base_width - n + 1;
}
else
#endif
{
screen_fill(row, row + 1, col, pum_col + pum_base_width + n,
' ', ' ', attr);
col = pum_col + pum_base_width + n;
}
totwidth = pum_base_width + n;
}

#if defined(FEAT_RIGHTLEFT)
if (curwin->w_p_rl)
screen_fill(row, row + 1, pum_col - pum_width + 1, col + 1, ' ',
' ', attr);
else
#endif
screen_fill(row, row + 1, col, pum_col + pum_width, ' ', ' ',
attr);
if (pum_scrollbar > 0)
{
#if defined(FEAT_RIGHTLEFT)
if (curwin->w_p_rl)
screen_putchar(' ', row, pum_col - pum_width,
i >= thumb_pos && i < thumb_pos + thumb_height
? attr_thumb : attr_scroll);
else
#endif
screen_putchar(' ', row, pum_col + pum_width,
i >= thumb_pos && i < thumb_pos + thumb_height
? attr_thumb : attr_scroll);
}

++row;
}

#if defined(FEAT_PROP_POPUP)
screen_zindex = 0;
#endif
}

#if (defined(FEAT_PROP_POPUP) && defined(FEAT_QUICKFIX)) || defined(PROTO)



void
pum_position_info_popup(win_T *wp)
{
int col = pum_col + pum_width + pum_scrollbar + 1;
int row = pum_row;
int botpos = POPPOS_BOTLEFT;

wp->w_popup_pos = POPPOS_TOPLEFT;
if (Columns - col < 20 && Columns - col < pum_col)
{
col = pum_col - 1;
wp->w_popup_pos = POPPOS_TOPRIGHT;
botpos = POPPOS_BOTRIGHT;
wp->w_maxwidth = pum_col - 1;
}
else
wp->w_maxwidth = Columns - col + 1;
wp->w_maxwidth -= popup_extra_width(wp);

row -= popup_top_extra(wp);
if (wp->w_popup_flags & POPF_INFO_MENU)
{
if (pum_row < pum_win_row)
{

row += pum_height;
wp->w_popup_pos = botpos;
}
else

row += 1;
}
else

row += pum_selected - pum_first + 1;

wp->w_popup_flags &= ~POPF_HIDDEN;
if (wp->w_maxwidth < 10)


wp->w_popup_flags |= POPF_HIDDEN;
else
popup_set_wantpos_rowcol(wp, row, col);
}
#endif











static int
pum_set_selected(int n, int repeat UNUSED)
{
int resized = FALSE;
int context = pum_height / 2;
#if defined(FEAT_QUICKFIX)
int prev_selected = pum_selected;
#endif
#if defined(FEAT_PROP_POPUP) && defined(FEAT_QUICKFIX)
int has_info = FALSE;
#endif

pum_selected = n;

if (pum_selected >= 0 && pum_selected < pum_size)
{
if (pum_first > pum_selected - 4)
{


if (pum_first > pum_selected - 2)
{
pum_first -= pum_height - 2;
if (pum_first < 0)
pum_first = 0;
else if (pum_first > pum_selected)
pum_first = pum_selected;
}
else
pum_first = pum_selected;
}
else if (pum_first < pum_selected - pum_height + 5)
{


if (pum_first < pum_selected - pum_height + 1 + 2)
{
pum_first += pum_height - 2;
if (pum_first < pum_selected - pum_height + 1)
pum_first = pum_selected - pum_height + 1;
}
else
pum_first = pum_selected - pum_height + 1;
}


if (context > 3)
context = 3;
if (pum_height > 2)
{
if (pum_first > pum_selected - context)
{

pum_first = pum_selected - context;
if (pum_first < 0)
pum_first = 0;
}
else if (pum_first < pum_selected + context - pum_height + 1)
{

pum_first = pum_selected + context - pum_height + 1;
}
}

if (pum_first > pum_size - pum_height)
pum_first = pum_size - pum_height;

#if defined(FEAT_QUICKFIX)







if (pum_array[pum_selected].pum_info != NULL
&& Rows > 10
&& repeat <= 1
&& vim_strchr(p_cot, 'p') != NULL)
{
win_T *curwin_save = curwin;
tabpage_T *curtab_save = curtab;
int res = OK;
#if defined(FEAT_PROP_POPUP)
use_popup_T use_popup;
#else
#define use_popup USEPOPUP_NONE
#endif
#if defined(FEAT_PROP_POPUP)
has_info = TRUE;
if (strstr((char *)p_cot, "popuphidden") != NULL)
use_popup = USEPOPUP_HIDDEN;
else if (strstr((char *)p_cot, "popup") != NULL)
use_popup = USEPOPUP_NORMAL;
else
use_popup = USEPOPUP_NONE;
#endif


g_do_tagpreview = 3;
if (p_pvh > 0 && p_pvh < g_do_tagpreview)
g_do_tagpreview = p_pvh;
++RedrawingDisabled;


++no_u_sync;
resized = prepare_tagpreview(FALSE, FALSE, use_popup);
--no_u_sync;
--RedrawingDisabled;
g_do_tagpreview = 0;

if (curwin->w_p_pvw
#if defined(FEAT_PROP_POPUP)
|| (curwin->w_popup_flags & POPF_INFO)
#endif
)
{
if (!resized
&& curbuf->b_nwindows == 1
&& curbuf->b_fname == NULL
&& bt_nofile(curbuf)
&& curbuf->b_p_bh[0] == 'w')
{

while (!BUFEMPTY())
ml_delete((linenr_T)1, FALSE);
}
else
{

++no_u_sync;
res = do_ecmd(0, NULL, NULL, NULL, ECMD_ONE, 0, NULL);
--no_u_sync;
if (res == OK)
{


set_option_value((char_u *)"swf", 0L, NULL, OPT_LOCAL);
set_option_value((char_u *)"bt", 0L,
(char_u *)"nofile", OPT_LOCAL);
set_option_value((char_u *)"bh", 0L,
(char_u *)"wipe", OPT_LOCAL);
set_option_value((char_u *)"diff", 0L,
NULL, OPT_LOCAL);
}
}
if (res == OK)
{
char_u *p, *e;
linenr_T lnum = 0;

for (p = pum_array[pum_selected].pum_info; *p != NUL; )
{
e = vim_strchr(p, '\n');
if (e == NULL)
{
ml_append(lnum++, p, 0, FALSE);
break;
}
else
{
*e = NUL;
ml_append(lnum++, p, (int)(e - p + 1), FALSE);
*e = '\n';
p = e + 1;
}
}

ml_delete(curbuf->b_ml.ml_line_count, FALSE);



if (repeat == 0 && use_popup == USEPOPUP_NONE)
{
if (lnum > p_pvh)
lnum = p_pvh;
if (curwin->w_height < lnum)
{
win_setheight((int)lnum);
resized = TRUE;
}
}

curbuf->b_changed = 0;
curbuf->b_p_ma = FALSE;
if (pum_selected != prev_selected)
{
#if defined(FEAT_PROP_POPUP)
curwin->w_firstline = 1;
#endif
curwin->w_topline = 1;
}
else if (curwin->w_topline > curbuf->b_ml.ml_line_count)
curwin->w_topline = curbuf->b_ml.ml_line_count;
curwin->w_cursor.lnum = curwin->w_topline;
curwin->w_cursor.col = 0;
#if defined(FEAT_PROP_POPUP)
if (use_popup != USEPOPUP_NONE)
{
pum_position_info_popup(curwin);
if (win_valid(curwin_save))
redraw_win_later(curwin_save, SOME_VALID);
}
#endif
if ((curwin != curwin_save && win_valid(curwin_save))
|| (curtab != curtab_save
&& valid_tabpage(curtab_save)))
{
if (curtab != curtab_save && valid_tabpage(curtab_save))
goto_tabpage_tp(curtab_save, FALSE, FALSE);




if (ins_compl_active() && !resized)
curwin->w_redr_status = FALSE;


validate_cursor();
redraw_later(SOME_VALID);





if (resized && win_valid(curwin_save))
{
++no_u_sync;
win_enter(curwin_save, TRUE);
--no_u_sync;
update_topline();
}



pum_pretend_not_visible = TRUE;



pum_will_redraw = !resized;
update_screen(0);
pum_pretend_not_visible = FALSE;
pum_will_redraw = FALSE;

if (!resized && win_valid(curwin_save))
{
#if defined(FEAT_PROP_POPUP)
win_T *wp = curwin;
#endif
++no_u_sync;
win_enter(curwin_save, TRUE);
--no_u_sync;
#if defined(FEAT_PROP_POPUP)
if (use_popup == USEPOPUP_HIDDEN && win_valid(wp))
popup_hide(wp);
#endif
}



pum_pretend_not_visible = TRUE;
pum_will_redraw = !resized;
update_screen(0);
pum_pretend_not_visible = FALSE;
pum_will_redraw = FALSE;
call_update_screen = FALSE;
}
}
}
#if defined(FEAT_PROP_POPUP) && defined(FEAT_QUICKFIX)
if (WIN_IS_POPUP(curwin))

win_enter(firstwin, TRUE);
#endif
}
#endif
}
#if defined(FEAT_PROP_POPUP) && defined(FEAT_QUICKFIX)
if (!has_info)

popup_hide_info();
#endif

if (!resized)
pum_redraw();

return resized;
}




void
pum_undisplay(void)
{
pum_array = NULL;
redraw_all_later(NOT_VALID);
redraw_tabline = TRUE;
status_redraw_all();
#if defined(FEAT_PROP_POPUP) && defined(FEAT_QUICKFIX)

popup_hide_info();
#endif
}





void
pum_clear(void)
{
pum_first = 0;
}






int
pum_visible(void)
{
return !pum_pretend_not_visible && pum_array != NULL;
}




void
pum_may_redraw(void)
{
pumitem_T *array = pum_array;
int len = pum_size;
int selected = pum_selected;

if (!pum_visible() || pum_will_redraw)
return; 

if (pum_window != curwin
|| (pum_win_row == curwin->w_wrow + W_WINROW(curwin)
&& pum_win_height == curwin->w_height
&& pum_win_col == curwin->w_wincol
&& pum_win_width == curwin->w_width))
{

pum_redraw();
}
else
{
int wcol = curwin->w_wcol;




pum_undisplay();
curwin->w_wcol = pum_win_wcol;
curwin->w_valid |= VALID_WCOL;
pum_display(array, len, selected);
curwin->w_wcol = wcol;
}
}





int
pum_get_height(void)
{
return pum_height;
}

#if defined(FEAT_EVAL) || defined(PROTO)



void
pum_set_event_info(dict_T *dict)
{
if (!pum_visible())
return;
(void)dict_add_number(dict, "height", pum_height);
(void)dict_add_number(dict, "width", pum_width);
(void)dict_add_number(dict, "row", pum_row);
(void)dict_add_number(dict, "col", pum_col);
(void)dict_add_number(dict, "size", pum_size);
(void)dict_add_bool(dict, "scrollbar",
pum_scrollbar ? VVAL_TRUE : VVAL_FALSE);
}
#endif

#if defined(FEAT_BEVAL_TERM) || defined(FEAT_TERM_POPUP_MENU) || defined(PROTO)
static void
pum_position_at_mouse(int min_width)
{
if (Rows - mouse_row > pum_size)
{

pum_row = mouse_row + 1;
if (pum_height > Rows - pum_row)
pum_height = Rows - pum_row;
}
else
{

pum_row = mouse_row - pum_size;
if (pum_row < 0)
{
pum_height += pum_row;
pum_row = 0;
}
}
if (Columns - mouse_col >= pum_base_width
|| Columns - mouse_col > min_width)

pum_col = mouse_col;
else

pum_col = Columns - (pum_base_width > min_width
? min_width : pum_base_width);

pum_width = Columns - pum_col;
if (pum_width > pum_base_width + 1)
pum_width = pum_base_width + 1;


pum_window = NULL;
}

#endif

#if defined(FEAT_BEVAL_TERM) || defined(PROTO)
static pumitem_T *balloon_array = NULL;
static int balloon_arraysize;

#define BALLOON_MIN_WIDTH 50
#define BALLOON_MIN_HEIGHT 10

typedef struct {
char_u *start;
int bytelen;
int cells;
int indent;
} balpart_T;







int
split_message(char_u *mesg, pumitem_T **array)
{
garray_T ga;
char_u *p;
balpart_T *item;
int quoted = FALSE;
int height;
int line;
int item_idx;
int indent = 0;
int max_cells = 0;
int max_height = Rows / 2 - 1;
int long_item_count = 0;
int split_long_items = FALSE;

ga_init2(&ga, sizeof(balpart_T), 20);
p = mesg;

while (*p != NUL)
{
if (ga_grow(&ga, 1) == FAIL)
goto failed;
item = ((balpart_T *)ga.ga_data) + ga.ga_len;
item->start = p;
item->indent = indent;
item->cells = indent * 2;
++ga.ga_len;
while (*p != NUL)
{
if (*p == '"')
quoted = !quoted;
else if (*p == '\n')
break;
else if (*p == '\\' && p[1] != NUL)
++p;
else if (!quoted)
{
if ((*p == ',' && p[1] == ' ') || *p == '{' || *p == '}')
{

if (*p == '{')
++indent;
else if (*p == '}' && indent > 0)
--indent;
++item->cells;
p = skipwhite(p + 1);
break;
}
}
item->cells += ptr2cells(p);
p += mb_ptr2len(p);
}
item->bytelen = p - item->start;
if (*p == '\n')
++p;
if (item->cells > max_cells)
max_cells = item->cells;
long_item_count += (item->cells - 1) / BALLOON_MIN_WIDTH;
}

height = 2 + ga.ga_len;


if (long_item_count > 0 && height + long_item_count <= max_height)
{
split_long_items = TRUE;
height += long_item_count;
}



if (height > max_height)
height = max_height;
*array = ALLOC_CLEAR_MULT(pumitem_T, height);
if (*array == NULL)
goto failed;


(*array)->pum_text = vim_strsave((char_u *)"");
(*array + height - 1)->pum_text = vim_strsave((char_u *)"");

for (line = 1, item_idx = 0; line < height - 1; ++item_idx)
{
int skip;
int thislen;
int copylen;
int ind;
int cells;

item = ((balpart_T *)ga.ga_data) + item_idx;
if (item->bytelen == 0)
(*array)[line++].pum_text = vim_strsave((char_u *)"");
else
for (skip = 0; skip < item->bytelen; skip += thislen)
{
if (split_long_items && item->cells >= BALLOON_MIN_WIDTH)
{
cells = item->indent * 2;
for (p = item->start + skip;
p < item->start + item->bytelen;
p += mb_ptr2len(p))
if ((cells += ptr2cells(p)) > BALLOON_MIN_WIDTH)
break;
thislen = p - (item->start + skip);
}
else
thislen = item->bytelen;


p = alloc(thislen + item->indent * 2 + 1);
if (p == NULL)
{
for (line = 0; line <= height - 1; ++line)
vim_free((*array)[line].pum_text);
vim_free(*array);
goto failed;
}
for (ind = 0; ind < item->indent * 2; ++ind)
p[ind] = ' ';


for (copylen = thislen; copylen > 0; --copylen)
if (item->start[skip + copylen - 1] != ' ')
break;

vim_strncpy(p + ind, item->start + skip, copylen);
(*array)[line].pum_text = p;
item->indent = 0; 
++line;
}
}
ga_clear(&ga);
return height;

failed:
ga_clear(&ga);
return 0;
}

void
ui_remove_balloon(void)
{
if (balloon_array != NULL)
{
pum_undisplay();
while (balloon_arraysize > 0)
vim_free(balloon_array[--balloon_arraysize].pum_text);
VIM_CLEAR(balloon_array);
}
}




void
ui_post_balloon(char_u *mesg, list_T *list)
{
ui_remove_balloon();

if (mesg == NULL && list == NULL)
{
pum_undisplay();
return;
}
if (list != NULL)
{
listitem_T *li;
int idx;

balloon_arraysize = list->lv_len;
balloon_array = ALLOC_CLEAR_MULT(pumitem_T, list->lv_len);
if (balloon_array == NULL)
return;
range_list_materialize(list);
for (idx = 0, li = list->lv_first; li != NULL; li = li->li_next, ++idx)
{
char_u *text = tv_get_string_chk(&li->li_tv);

balloon_array[idx].pum_text = vim_strsave(
text == NULL ? (char_u *)"" : text);
}
}
else
balloon_arraysize = split_message(mesg, &balloon_array);

if (balloon_arraysize > 0)
{
pum_array = balloon_array;
pum_size = balloon_arraysize;
pum_compute_size();
pum_scrollbar = 0;
pum_height = balloon_arraysize;

pum_position_at_mouse(BALLOON_MIN_WIDTH);
pum_selected = -1;
pum_first = 0;
pum_redraw();
}
}




void
ui_may_remove_balloon(void)
{


ui_remove_balloon();
}
#endif

#if defined(FEAT_TERM_POPUP_MENU) || defined(PROTO)



static void
pum_select_mouse_pos(void)
{
int idx = mouse_row - pum_row;

if (idx < 0 || idx >= pum_size)
pum_selected = -1;
else if (*pum_array[idx].pum_text != NUL)
pum_selected = idx;
}




static void
pum_execute_menu(vimmenu_T *menu, int mode)
{
vimmenu_T *mp;
int idx = 0;
exarg_T ea;

for (mp = menu->children; mp != NULL; mp = mp->next)
if ((mp->modes & mp->enabled & mode) && idx++ == pum_selected)
{
vim_memset(&ea, 0, sizeof(ea));
execute_menu(&ea, mp, -1);
break;
}
}





void
pum_show_popupmenu(vimmenu_T *menu)
{
vimmenu_T *mp;
int idx = 0;
pumitem_T *array;
#if defined(FEAT_BEVAL_TERM)
int save_bevalterm = p_bevalterm;
#endif
int mode;

pum_undisplay();
pum_size = 0;
mode = get_menu_mode_flag();

for (mp = menu->children; mp != NULL; mp = mp->next)
if (menu_is_separator(mp->dname)
|| (mp->modes & mp->enabled & mode))
++pum_size;



if (pum_size <= 0)
{
emsg(e_menuothermode);
return;
}

array = ALLOC_CLEAR_MULT(pumitem_T, pum_size);
if (array == NULL)
return;

for (mp = menu->children; mp != NULL; mp = mp->next)
if (menu_is_separator(mp->dname))
array[idx++].pum_text = (char_u *)"";
else if (mp->modes & mp->enabled & mode)
array[idx++].pum_text = mp->dname;

pum_array = array;
pum_compute_size();
pum_scrollbar = 0;
pum_height = pum_size;
pum_position_at_mouse(20);

pum_selected = -1;
pum_first = 0;
#if defined(FEAT_BEVAL_TERM)
p_bevalterm = TRUE; 
mch_setmouse(TRUE);
#endif

for (;;)
{
int c;

pum_redraw();
setcursor_mayforce(TRUE);
out_flush();

c = vgetc();



if (c == ESC || c == Ctrl_C || pum_array == NULL)
break;
else if (c == CAR || c == NL)
{

pum_execute_menu(menu, mode);
break;
}
else if (c == 'k' || c == K_UP || c == K_MOUSEUP)
{

while (pum_selected > 0)
{
--pum_selected;
if (*array[pum_selected].pum_text != NUL)
break;
}
}
else if (c == 'j' || c == K_DOWN || c == K_MOUSEDOWN)
{

while (pum_selected < pum_size - 1)
{
++pum_selected;
if (*array[pum_selected].pum_text != NUL)
break;
}
}
else if (c == K_RIGHTMOUSE)
{

vungetc(c);
break;
}
else if (c == K_LEFTDRAG || c == K_RIGHTDRAG || c == K_MOUSEMOVE)
{

pum_select_mouse_pos();
}
else if (c == K_LEFTMOUSE || c == K_LEFTMOUSE_NM || c == K_RIGHTRELEASE)
{


pum_select_mouse_pos();
if (pum_selected >= 0)
{
pum_execute_menu(menu, mode);
break;
}
if (c == K_LEFTMOUSE || c == K_LEFTMOUSE_NM)
break;
}
}

vim_free(array);
pum_undisplay();
#if defined(FEAT_BEVAL_TERM)
p_bevalterm = save_bevalterm;
mch_setmouse(TRUE);
#endif
}

void
pum_make_popup(char_u *path_name, int use_mouse_pos)
{
vimmenu_T *menu;

if (!use_mouse_pos)
{


mouse_row = curwin->w_winrow + curwin->w_wrow;
mouse_col = curwin->w_wincol + curwin->w_wcol;
}

menu = gui_find_menu(path_name);
if (menu != NULL)
pum_show_popupmenu(menu);
}
#endif
