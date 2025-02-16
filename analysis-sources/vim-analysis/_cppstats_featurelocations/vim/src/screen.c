






































#include "vim.h"




static int screen_attr = 0;

static void screen_char_2(unsigned off, int row, int col);
static void screenclear2(void);
static void lineclear(unsigned off, int width, int attr);
static void lineinvalid(unsigned off, int width);
static int win_do_lines(win_T *wp, int row, int line_count, int mayclear, int del, int clear_attr);
static void win_rest_invalid(win_T *wp);
static void msg_pos_mode(void);
static void recording_mode(int attr);


static int screen_char_attr = 0;

#if defined(FEAT_CONCEAL) || defined(PROTO)




int
conceal_cursor_line(win_T *wp)
{
int c;

if (*wp->w_p_cocu == NUL)
return FALSE;
if (get_real_state() & VISUAL)
c = 'v';
else if (State & INSERT)
c = 'i';
else if (State & NORMAL)
c = 'n';
else if (State & CMDLINE)
c = 'c';
else
return FALSE;
return vim_strchr(wp->w_p_cocu, c) != NULL;
}




void
conceal_check_cursor_line(void)
{
if (curwin->w_p_cole > 0 && conceal_cursor_line(curwin))
{
need_cursor_line_redraw = TRUE;


curs_columns(TRUE);
}
}
#endif





int
get_wcr_attr(win_T *wp)
{
int wcr_attr = 0;

if (*wp->w_p_wcr != NUL)
wcr_attr = syn_name2attr(wp->w_p_wcr);
#if defined(FEAT_PROP_POPUP)
else if (WIN_IS_POPUP(wp))
{
if (wp->w_popup_flags & POPF_INFO)
wcr_attr = HL_ATTR(HLF_PSI); 
else
wcr_attr = HL_ATTR(HLF_PNI); 
}
#endif
return wcr_attr;
}





static int
screen_fill_end(
win_T *wp,
int c1,
int c2,
int off,
int width,
int row,
int endrow,
int attr)
{
int nn = off + width;

if (nn > wp->w_width)
nn = wp->w_width;
#if defined(FEAT_RIGHTLEFT)
if (wp->w_p_rl)
{
screen_fill(W_WINROW(wp) + row, W_WINROW(wp) + endrow,
W_ENDCOL(wp) - nn, (int)W_ENDCOL(wp) - off,
c1, c2, attr);
}
else
#endif
screen_fill(W_WINROW(wp) + row, W_WINROW(wp) + endrow,
wp->w_wincol + off, (int)wp->w_wincol + nn,
c1, c2, attr);
return nn;
}






void
win_draw_end(
win_T *wp,
int c1,
int c2,
int draw_margin,
int row,
int endrow,
hlf_T hl)
{
int n = 0;
int attr = HL_ATTR(hl);
int wcr_attr = get_wcr_attr(wp);

attr = hl_combine_attr(wcr_attr, attr);

if (draw_margin)
{
#if defined(FEAT_FOLDING)
int fdc = compute_foldcolumn(wp, 0);

if (fdc > 0)

n = screen_fill_end(wp, ' ', ' ', n, fdc,
row, endrow, hl_combine_attr(wcr_attr, HL_ATTR(HLF_FC)));
#endif
#if defined(FEAT_SIGNS)
if (signcolumn_on(wp))

n = screen_fill_end(wp, ' ', ' ', n, 2,
row, endrow, hl_combine_attr(wcr_attr, HL_ATTR(HLF_SC)));
#endif
if ((wp->w_p_nu || wp->w_p_rnu)
&& vim_strchr(p_cpo, CPO_NUMCOL) == NULL)

n = screen_fill_end(wp, ' ', ' ', n, number_width(wp) + 1,
row, endrow, hl_combine_attr(wcr_attr, HL_ATTR(HLF_N)));
}

#if defined(FEAT_RIGHTLEFT)
if (wp->w_p_rl)
{
screen_fill(W_WINROW(wp) + row, W_WINROW(wp) + endrow,
wp->w_wincol, W_ENDCOL(wp) - 1 - n,
c2, c2, attr);
screen_fill(W_WINROW(wp) + row, W_WINROW(wp) + endrow,
W_ENDCOL(wp) - 1 - n, W_ENDCOL(wp) - n,
c1, c2, attr);
}
else
#endif
{
screen_fill(W_WINROW(wp) + row, W_WINROW(wp) + endrow,
wp->w_wincol + n, (int)W_ENDCOL(wp),
c1, c2, attr);
}

set_empty_rows(wp, row);
}

#if defined(FEAT_FOLDING) || defined(PROTO)




int
compute_foldcolumn(win_T *wp, int col)
{
int fdc = wp->w_p_fdc;
int wmw = wp == curwin && p_wmw == 0 ? 1 : p_wmw;
int wwidth = wp->w_width;

if (fdc > wwidth - (col + wmw))
fdc = wwidth - (col + wmw);
return fdc;
}





void
fill_foldcolumn(
char_u *p,
win_T *wp,
int closed, 
linenr_T lnum) 
{
int i = 0;
int level;
int first_level;
int empty;
int fdc = compute_foldcolumn(wp, 0);


vim_memset(p, ' ', (size_t)fdc);

level = win_foldinfo.fi_level;
if (level > 0)
{

empty = (fdc == 1) ? 0 : 1;



first_level = level - fdc - closed + 1 + empty;
if (first_level < 1)
first_level = 1;

for (i = 0; i + empty < fdc; ++i)
{
if (win_foldinfo.fi_lnum == lnum
&& first_level + i >= win_foldinfo.fi_low_level)
p[i] = '-';
else if (first_level == 1)
p[i] = '|';
else if (first_level + i <= 9)
p[i] = '0' + first_level + i;
else
p[i] = '>';
if (first_level + i == level)
break;
}
}
if (closed)
p[i >= fdc ? i - 1 : i] = '+';
}
#endif 





static int
comp_char_differs(int off_from, int off_to)
{
int i;

for (i = 0; i < Screen_mco; ++i)
{
if (ScreenLinesC[i][off_from] != ScreenLinesC[i][off_to])
return TRUE;
if (ScreenLinesC[i][off_from] == 0)
break;
}
return FALSE;
}








static int
char_needs_redraw(int off_from, int off_to, int cols)
{
if (cols > 0
&& ((ScreenLines[off_from] != ScreenLines[off_to]
|| ScreenAttrs[off_from] != ScreenAttrs[off_to])
|| (enc_dbcs != 0
&& MB_BYTE2LEN(ScreenLines[off_from]) > 1
&& (enc_dbcs == DBCS_JPNU && ScreenLines[off_from] == 0x8e
? ScreenLines2[off_from] != ScreenLines2[off_to]
: (cols > 1 && ScreenLines[off_from + 1]
!= ScreenLines[off_to + 1])))
|| (enc_utf8
&& (ScreenLinesUC[off_from] != ScreenLinesUC[off_to]
|| (ScreenLinesUC[off_from] != 0
&& comp_char_differs(off_from, off_to))
|| ((*mb_off2cells)(off_from, off_from + cols) > 1
&& ScreenLines[off_from + 1]
!= ScreenLines[off_to + 1])))))
return TRUE;
return FALSE;
}

#if defined(FEAT_TERMINAL) || defined(PROTO)



int
screen_get_current_line_off()
{
return (int)(current_ScreenLine - ScreenLines);
}
#endif

#if defined(FEAT_PROP_POPUP)




static int
blocked_by_popup(int row, int col)
{
int off;

if (!popup_visible)
return FALSE;
off = row * screen_Columns + col;
return popup_mask[off] > screen_zindex || popup_transparent[off];
}
#endif




void
reset_screen_attr(void)
{
#if defined(FEAT_GUI)
if (gui.in_use)


screen_attr = HL_ALL + 1;
else
#endif

screen_attr = HL_BOLD | HL_UNDERLINE | HL_INVERSE | HL_STRIKETHROUGH;
}














void
screen_line(
int row,
int coloff,
int endcol,
int clear_width,
int flags UNUSED)
{
unsigned off_from;
unsigned off_to;
unsigned max_off_from;
unsigned max_off_to;
int col = 0;
int hl;
int force = FALSE; 
int redraw_this 
#if defined(FEAT_GUI)
= TRUE 
#endif
;
int redraw_next; 
int clear_next = FALSE;
int char_cells; 

#define CHAR_CELLS char_cells


if (row >= Rows)
row = Rows - 1;
if (endcol > Columns)
endcol = Columns;

#if defined(FEAT_CLIPBOARD)
clip_may_clear_selection(row, row);
#endif

off_from = (unsigned)(current_ScreenLine - ScreenLines);
off_to = LineOffset[row] + coloff;
max_off_from = off_from + screen_Columns;
max_off_to = LineOffset[row] + screen_Columns;

#if defined(FEAT_RIGHTLEFT)
if (flags & SLF_RIGHTLEFT)
{

if (clear_width > 0)
{
while (col <= endcol && ScreenLines[off_to] == ' '
&& ScreenAttrs[off_to] == 0
&& (!enc_utf8 || ScreenLinesUC[off_to] == 0))
{
++off_to;
++col;
}
if (col <= endcol)
screen_fill(row, row + 1, col + coloff,
endcol + coloff + 1, ' ', ' ', 0);
}
col = endcol + 1;
off_to = LineOffset[row] + col + coloff;
off_from += col;
endcol = (clear_width > 0 ? clear_width : -clear_width);
}
#endif 

#if defined(FEAT_PROP_POPUP)



if (coloff > 0 && ScreenLines[off_to] == 0)
{
ScreenLines[off_to - 1] = ' ';
ScreenLinesUC[off_to - 1] = 0;
screen_char(off_to - 1, row, col + coloff - 1);
}
#endif

redraw_next = char_needs_redraw(off_from, off_to, endcol - col);

while (col < endcol)
{
if (has_mbyte && (col + 1 < endcol))
char_cells = (*mb_off2cells)(off_from, max_off_from);
else
char_cells = 1;

redraw_this = redraw_next;
redraw_next = force || char_needs_redraw(off_from + CHAR_CELLS,
off_to + CHAR_CELLS, endcol - col - CHAR_CELLS);

#if defined(FEAT_GUI)



if (redraw_next && gui.in_use)
{
hl = ScreenAttrs[off_to + CHAR_CELLS];
if (hl > HL_ALL)
hl = syn_attr2attr(hl);
if (hl & HL_BOLD)
redraw_this = TRUE;
}
#endif
#if defined(FEAT_PROP_POPUP)
if (blocked_by_popup(row, col + coloff))
redraw_this = FALSE;
#endif
if (redraw_this)
{












if ( p_wiv
&& !force
#if defined(FEAT_GUI)
&& !gui.in_use
#endif
&& ScreenAttrs[off_to] != 0
&& ScreenAttrs[off_from] != ScreenAttrs[off_to])
{



windgoto(row, col + coloff);
out_str(T_CE); 
screen_start(); 
force = TRUE; 
redraw_next = TRUE; 





if (col + coloff > 0 && ScreenAttrs[off_to - 1] != 0)
{
screen_attr = ScreenAttrs[off_to - 1];
term_windgoto(row, col + coloff);
screen_stop_highlight();
}
else
screen_attr = 0; 
}
if (enc_dbcs != 0)
{




if (char_cells == 1
&& col + 1 < endcol
&& (*mb_off2cells)(off_to, max_off_to) > 1)
{


ScreenLines[off_to + 1] = 0;
redraw_next = TRUE;
}
else if (char_cells == 2
&& col + 2 < endcol
&& (*mb_off2cells)(off_to, max_off_to) == 1
&& (*mb_off2cells)(off_to + 1, max_off_to) > 1)
{



ScreenLines[off_to + 2] = 0;
redraw_next = TRUE;
}

if (enc_dbcs == DBCS_JPNU)
ScreenLines2[off_to] = ScreenLines2[off_from];
}





if (has_mbyte && col + char_cells == endcol
&& ((char_cells == 1
&& (*mb_off2cells)(off_to, max_off_to) > 1)
|| (char_cells == 2
&& (*mb_off2cells)(off_to, max_off_to) == 1
&& (*mb_off2cells)(off_to + 1, max_off_to) > 1)))
clear_next = TRUE;

ScreenLines[off_to] = ScreenLines[off_from];
if (enc_utf8)
{
ScreenLinesUC[off_to] = ScreenLinesUC[off_from];
if (ScreenLinesUC[off_from] != 0)
{
int i;

for (i = 0; i < Screen_mco; ++i)
ScreenLinesC[i][off_to] = ScreenLinesC[i][off_from];
}
}
if (char_cells == 2)
ScreenLines[off_to + 1] = ScreenLines[off_from + 1];

#if defined(FEAT_GUI) || defined(UNIX)




if (
#if defined(FEAT_GUI)
gui.in_use
#endif
#if defined(FEAT_GUI) && defined(UNIX)
||
#endif
#if defined(UNIX)
term_is_xterm
#endif
)
{
hl = ScreenAttrs[off_to];
if (hl > HL_ALL)
hl = syn_attr2attr(hl);
if (hl & HL_BOLD)
redraw_next = TRUE;
}
#endif
ScreenAttrs[off_to] = ScreenAttrs[off_from];



if (char_cells == 2)
ScreenAttrs[off_to + 1] = ScreenAttrs[off_from];

if (enc_dbcs != 0 && char_cells == 2)
screen_char_2(off_to, row, col + coloff);
else
screen_char(off_to, row, col + coloff);
}
else if ( p_wiv
#if defined(FEAT_GUI)
&& !gui.in_use
#endif
&& col + coloff > 0)
{
if (ScreenAttrs[off_to] == ScreenAttrs[off_to - 1])
{




screen_attr = 0;
}
else if (screen_attr != 0)
screen_stop_highlight();
}

off_to += CHAR_CELLS;
off_from += CHAR_CELLS;
col += CHAR_CELLS;
}

if (clear_next)
{


ScreenLines[off_to] = ' ';
if (enc_utf8)
ScreenLinesUC[off_to] = 0;
screen_char(off_to, row, col + coloff);
}

if (clear_width > 0
#if defined(FEAT_RIGHTLEFT)
&& !(flags & SLF_RIGHTLEFT)
#endif
)
{
#if defined(FEAT_GUI)
int startCol = col;
#endif


while (col < clear_width && ScreenLines[off_to] == ' '
&& ScreenAttrs[off_to] == 0
&& (!enc_utf8 || ScreenLinesUC[off_to] == 0))
{
++off_to;
++col;
}
if (col < clear_width)
{
#if defined(FEAT_GUI)







if (gui.in_use && (col > startCol || !redraw_this))
{
hl = ScreenAttrs[off_to];
if (hl > HL_ALL || (hl & HL_BOLD))
{
int prev_cells = 1;

if (enc_utf8)


prev_cells = ScreenLines[off_to - 1] == 0 ? 2 : 1;
else if (enc_dbcs != 0)
{


unsigned off = LineOffset[row];
unsigned max_off = LineOffset[row] + screen_Columns;

while (off < off_to)
{
prev_cells = (*mb_off2cells)(off, max_off);
off += prev_cells;
}
}

if (enc_dbcs != 0 && prev_cells > 1)
screen_char_2(off_to - prev_cells, row,
col + coloff - prev_cells);
else
screen_char(off_to - prev_cells, row,
col + coloff - prev_cells);
}
}
#endif
screen_fill(row, row + 1, col + coloff, clear_width + coloff,
' ', ' ', 0);
off_to += clear_width - col;
col = clear_width;
}
}

if (clear_width > 0
#if defined(FEAT_PROP_POPUP)
&& !(flags & SLF_POPUP) 
#endif
)
{


if (coloff + col < Columns)
{
#if defined(FEAT_PROP_POPUP)
if (!blocked_by_popup(row, col + coloff))
#endif
{
int c;

c = fillchar_vsep(&hl);
if (ScreenLines[off_to] != (schar_T)c
|| (enc_utf8 && (int)ScreenLinesUC[off_to]
!= (c >= 0x80 ? c : 0))
|| ScreenAttrs[off_to] != hl)
{
ScreenLines[off_to] = c;
ScreenAttrs[off_to] = hl;
if (enc_utf8)
{
if (c >= 0x80)
{
ScreenLinesUC[off_to] = c;
ScreenLinesC[0][off_to] = 0;
}
else
ScreenLinesUC[off_to] = 0;
}
screen_char(off_to, row, col + coloff);
}
}
}
else
LineWraps[row] = FALSE;
}
}

#if defined(FEAT_RIGHTLEFT) || defined(PROTO)




void
rl_mirror(char_u *str)
{
char_u *p1, *p2;
int t;

for (p1 = str, p2 = str + STRLEN(str) - 1; p1 < p2; ++p1, --p2)
{
t = *p1;
*p1 = *p2;
*p2 = t;
}
}
#endif




void
draw_vsep_win(win_T *wp, int row)
{
int hl;
int c;

if (wp->w_vsep_width)
{

c = fillchar_vsep(&hl);
screen_fill(W_WINROW(wp) + row, W_WINROW(wp) + wp->w_height,
W_ENDCOL(wp), W_ENDCOL(wp) + 1,
c, ' ', hl);
}
}

#if defined(FEAT_WILDMENU)
static int skip_status_match_char(expand_T *xp, char_u *s);




static int
status_match_len(expand_T *xp, char_u *s)
{
int len = 0;

#if defined(FEAT_MENU)
int emenu = (xp->xp_context == EXPAND_MENUS
|| xp->xp_context == EXPAND_MENUNAMES);


if (emenu && menu_is_separator(s))
return 1;
#endif

while (*s != NUL)
{
s += skip_status_match_char(xp, s);
len += ptr2cells(s);
MB_PTR_ADV(s);
}

return len;
}





static int
skip_status_match_char(expand_T *xp, char_u *s)
{
if ((rem_backslash(s) && xp->xp_context != EXPAND_HELP)
#if defined(FEAT_MENU)
|| ((xp->xp_context == EXPAND_MENUS
|| xp->xp_context == EXPAND_MENUNAMES)
&& (s[0] == '\t' || (s[0] == '\\' && s[1] != NUL)))
#endif
)
{
#if !defined(BACKSLASH_IN_FILENAME)
if (xp->xp_shell && csh_like_shell() && s[1] == '\\' && s[2] == '!')
return 2;
#endif
return 1;
}
return 0;
}








void
win_redr_status_matches(
expand_T *xp,
int num_matches,
char_u **matches, 
int match,
int showtail)
{
#define L_MATCH(m) (showtail ? sm_gettail(matches[m]) : matches[m])
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
#if defined(FEAT_MENU)
int emenu;
#endif
int l;

if (matches == NULL) 
return;

if (has_mbyte)
buf = alloc(Columns * MB_MAXBYTES + 1);
else
buf = alloc(Columns + 1);
if (buf == NULL)
return;

if (match == -1) 
{
match = 0;
highlight = FALSE;
}

clen = status_match_len(xp, L_MATCH(match)) + 3;
if (match == 0)
first_match = 0;
else if (match < first_match)
{

first_match = match;
add_left = TRUE;
}
else
{

for (i = first_match; i < match; ++i)
clen += status_match_len(xp, L_MATCH(i)) + 2;
if (first_match > 0)
clen += 2;

if ((long)clen > Columns)
{
first_match = match;

clen = 2;
for (i = match; i < num_matches; ++i)
{
clen += status_match_len(xp, L_MATCH(i)) + 2;
if ((long)clen >= Columns)
break;
}
if (i == num_matches)
add_left = TRUE;
}
}
if (add_left)
while (first_match > 0)
{
clen += status_match_len(xp, L_MATCH(first_match - 1)) + 2;
if ((long)clen >= Columns)
break;
--first_match;
}

fillchar = fillchar_status(&attr, curwin);

if (first_match == 0)
{
*buf = NUL;
len = 0;
}
else
{
STRCPY(buf, "< ");
len = 2;
}
clen = len;

i = first_match;
while ((long)(clen + status_match_len(xp, L_MATCH(i)) + 2) < Columns)
{
if (i == match)
{
selstart = buf + len;
selstart_col = clen;
}

s = L_MATCH(i);

#if defined(FEAT_MENU)
emenu = (xp->xp_context == EXPAND_MENUS
|| xp->xp_context == EXPAND_MENUNAMES);
if (emenu && menu_is_separator(s))
{
STRCPY(buf + len, transchar('|'));
l = (int)STRLEN(buf + len);
len += l;
clen += l;
}
else
#endif
for ( ; *s != NUL; ++s)
{
s += skip_status_match_char(xp, s);
clen += ptr2cells(s);
if (has_mbyte && (l = (*mb_ptr2len)(s)) > 1)
{
STRNCPY(buf + len, s, l);
s += l - 1;
len += l;
}
else
{
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

if (i != num_matches)
{
*(buf + len++) = '>';
++clen;
}

buf[len] = NUL;

row = cmdline_row - 1;
if (row >= 0)
{
if (wild_menu_showing == 0)
{
if (msg_scrolled > 0)
{


if (cmdline_row == Rows - 1)
{
screen_del_lines(0, 0, 1, (int)Rows, TRUE, 0, NULL);
++msg_scrolled;
}
else
{
++cmdline_row;
++row;
}
wild_menu_showing = WM_SCROLLED;
}
else
{



if (lastwin->w_status_height == 0)
{
save_p_ls = p_ls;
save_p_wmh = p_wmh;
p_ls = 2;
p_wmh = 0;
last_status(FALSE);
}
wild_menu_showing = WM_SHOWN;
}
}

screen_puts(buf, row, 0, attr);
if (selstart != NULL && highlight)
{
*selend = NUL;
screen_puts(selstart, row, selstart_col, HL_ATTR(HLF_WM));
}

screen_fill(row, row + 1, clen, (int)Columns, fillchar, fillchar, attr);
}

win_redraw_last_status(topframe);
vim_free(buf);
}
#endif






int
stl_connected(win_T *wp)
{
frame_T *fr;

fr = wp->w_frame;
while (fr->fr_parent != NULL)
{
if (fr->fr_parent->fr_layout == FR_COL)
{
if (fr->fr_next != NULL)
break;
}
else
{
if (fr->fr_next != NULL)
return TRUE;
}
fr = fr->fr_parent;
}
return FALSE;
}





int
get_keymap_str(
win_T *wp,
char_u *fmt, 
char_u *buf, 
int len) 
{
char_u *p;

if (wp->w_buffer->b_p_iminsert != B_IMODE_LMAP)
return FALSE;

{
#if defined(FEAT_EVAL)
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
if (p == NULL || *p == NUL)
#endif
{
#if defined(FEAT_KEYMAP)
if (wp->w_buffer->b_kmap_state & KEYMAP_LOADED)
p = wp->w_buffer->b_p_keymap;
else
#endif
p = (char_u *)"lang";
}
if (vim_snprintf((char *)buf, len, (char *)fmt, p) > len - 1)
buf[0] = NUL;
#if defined(FEAT_EVAL)
vim_free(s);
#endif
}
return buf[0] != NUL;
}

#if defined(FEAT_STL_OPT) || defined(PROTO)




void
win_redr_custom(
win_T *wp,
int draw_ruler) 
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
struct stl_hlrec tabtab[STL_MAX_ITEM];
int use_sandbox = FALSE;
win_T *ewp;
int p_crb_save;




if (entered)
return;
entered = TRUE;


if (wp == NULL)
{

stl = p_tal;
row = 0;
fillchar = ' ';
attr = HL_ATTR(HLF_TPF);
maxwidth = Columns;
#if defined(FEAT_EVAL)
use_sandbox = was_set_insecurely((char_u *)"tabline", 0);
#endif
}
else
{
row = W_WINROW(wp) + wp->w_height;
fillchar = fillchar_status(&attr, wp);
maxwidth = wp->w_width;

if (draw_ruler)
{
stl = p_ruf;

if (*stl == '%')
{
if (*++stl == '-')
stl++;
if (atoi((char *)stl))
while (VIM_ISDIGIT(*stl))
stl++;
if (*stl++ != '(')
stl = p_ruf;
}
col = ru_col - (Columns - wp->w_width);
if (col < (wp->w_width + 1) / 2)
col = (wp->w_width + 1) / 2;
maxwidth = wp->w_width - col;
if (!wp->w_status_height)
{
row = Rows - 1;
--maxwidth; 
fillchar = ' ';
attr = 0;
}

#if defined(FEAT_EVAL)
use_sandbox = was_set_insecurely((char_u *)"rulerformat", 0);
#endif
}
else
{
if (*wp->w_p_stl != NUL)
stl = wp->w_p_stl;
else
stl = p_stl;
#if defined(FEAT_EVAL)
use_sandbox = was_set_insecurely((char_u *)"statusline",
*wp->w_p_stl == NUL ? 0 : OPT_LOCAL);
#endif
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
vim_free(stl);
ewp->w_p_crb = p_crb_save;


p = transstr(buf);
if (p != NULL)
{
vim_strncpy(buf, p, sizeof(buf) - 1);
vim_free(p);
}


len = (int)STRLEN(buf);
while (width < maxwidth && len < (int)sizeof(buf) - 1)
{
len += (*mb_char2bytes)(fillchar, buf + len);
++width;
}
buf[len] = NUL;




curattr = attr;
p = buf;
for (n = 0; hltab[n].start != NULL; n++)
{
len = (int)(hltab[n].start - p);
screen_puts_len(p, len, row, col, curattr);
col += vim_strnsize(p, len);
p = hltab[n].start;

if (hltab[n].userhl == 0)
curattr = attr;
else if (hltab[n].userhl < 0)
curattr = syn_id2attr(-hltab[n].userhl);
#if defined(FEAT_TERMINAL)
else if (wp != NULL && wp != curwin && bt_terminal(wp->w_buffer)
&& wp->w_status_height != 0)
curattr = highlight_stltermnc[hltab[n].userhl - 1];
else if (wp != NULL && bt_terminal(wp->w_buffer)
&& wp->w_status_height != 0)
curattr = highlight_stlterm[hltab[n].userhl - 1];
#endif
else if (wp != NULL && wp != curwin && wp->w_status_height != 0)
curattr = highlight_stlnc[hltab[n].userhl - 1];
else
curattr = highlight_user[hltab[n].userhl - 1];
}
screen_puts(p, row, col, curattr);

if (wp == NULL)
{

col = 0;
len = 0;
p = buf;
fillchar = 0;
for (n = 0; tabtab[n].start != NULL; n++)
{
len += vim_strnsize(p, (int)(tabtab[n].start - p));
while (col < len)
TabPageIdxs[col++] = fillchar;
p = tabtab[n].start;
fillchar = tabtab[n].userhl;
}
while (col < Columns)
TabPageIdxs[col++] = fillchar;
}

theend:
entered = FALSE;
}

#endif 




void
screen_putchar(int c, int row, int col, int attr)
{
char_u buf[MB_MAXBYTES + 1];

if (has_mbyte)
buf[(*mb_char2bytes)(c, buf)] = NUL;
else
{
buf[0] = c;
buf[1] = NUL;
}
screen_puts(buf, row, col, attr);
}





void
screen_getbytes(int row, int col, char_u *bytes, int *attrp)
{
unsigned off;


if (ScreenLines != NULL && row < screen_Rows && col < screen_Columns)
{
off = LineOffset[row] + col;
*attrp = ScreenAttrs[off];
bytes[0] = ScreenLines[off];
bytes[1] = NUL;

if (enc_utf8 && ScreenLinesUC[off] != 0)
bytes[utfc_char2bytes(off, bytes)] = NUL;
else if (enc_dbcs == DBCS_JPNU && ScreenLines[off] == 0x8e)
{
bytes[0] = ScreenLines[off];
bytes[1] = ScreenLines2[off];
bytes[2] = NUL;
}
else if (enc_dbcs && MB_BYTE2LEN(bytes[0]) > 1)
{
bytes[1] = ScreenLines[off + 1];
bytes[2] = NUL;
}
}
}






static int
screen_comp_differs(int off, int *u8cc)
{
int i;

for (i = 0; i < Screen_mco; ++i)
{
if (ScreenLinesC[i][off] != (u8char_T)u8cc[i])
return TRUE;
if (u8cc[i] == 0)
break;
}
return FALSE;
}







void
screen_puts(
char_u *text,
int row,
int col,
int attr)
{
screen_puts_len(text, -1, row, col, attr);
}





void
screen_puts_len(
char_u *text,
int textlen,
int row,
int col,
int attr)
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
#if defined(FEAT_ARABIC)
int prev_c = 0; 
int pc, nc, nc1;
int pcc[MAX_MCO];
#endif
int force_redraw_this;
int force_redraw_next = FALSE;
int need_redraw;



if (ScreenLines == NULL
|| row >= screen_Rows || row < 0
|| col >= screen_Columns || col < 0)
return;
off = LineOffset[row] + col;



if (has_mbyte && col > 0 && col < screen_Columns
#if defined(FEAT_GUI)
&& !gui.in_use
#endif
&& mb_fix_col(col, row) != col)
{
ScreenLines[off - 1] = ' ';
ScreenAttrs[off - 1] = 0;
if (enc_utf8)
{
ScreenLinesUC[off - 1] = 0;
ScreenLinesC[0][off - 1] = 0;
}

screen_char(off - 1, row, col - 1);

force_redraw_next = TRUE;
}

max_off = LineOffset[row] + screen_Columns;
while (col < screen_Columns
&& (len < 0 || (int)(ptr - text) < len)
&& *ptr != NUL)
{
c = *ptr;

if (has_mbyte)
{
if (enc_utf8 && len > 0)
mbyte_blen = utfc_ptr2len_len(ptr, (int)((text + len) - ptr));
else
mbyte_blen = (*mb_ptr2len)(ptr);
if (enc_dbcs == DBCS_JPNU && c == 0x8e)
mbyte_cells = 1;
else if (enc_dbcs != 0)
mbyte_cells = mbyte_blen;
else 
{
if (len >= 0)
u8c = utfc_ptr2char_len(ptr, u8cc,
(int)((text + len) - ptr));
else
u8c = utfc_ptr2char(ptr, u8cc);
mbyte_cells = utf_char2cells(u8c);
#if defined(FEAT_ARABIC)
if (p_arshape && !p_tbidi && ARABIC_CHAR(u8c))
{

if (len >= 0 && (int)(ptr - text) + mbyte_blen >= len)
{

nc = NUL;
nc1 = NUL;
}
else
{
nc = utfc_ptr2char_len(ptr + mbyte_blen, pcc,
(int)((text + len) - ptr - mbyte_blen));
nc1 = pcc[0];
}
pc = prev_c;
prev_c = u8c;
u8c = arabic_shape(u8c, &c, &u8cc[0], nc, nc1, pc);
}
else
prev_c = u8c;
#endif
if (col + mbyte_cells > screen_Columns)
{


c = '>';
mbyte_cells = 1;
}
}
}

force_redraw_this = force_redraw_next;
force_redraw_next = FALSE;

need_redraw = ScreenLines[off] != c
|| (mbyte_cells == 2
&& ScreenLines[off + 1] != (enc_dbcs ? ptr[1] : 0))
|| (enc_dbcs == DBCS_JPNU
&& c == 0x8e
&& ScreenLines2[off] != ptr[1])
|| (enc_utf8
&& (ScreenLinesUC[off] !=
(u8char_T)(c < 0x80 && u8cc[0] == 0 ? 0 : u8c)
|| (ScreenLinesUC[off] != 0
&& screen_comp_differs(off, u8cc))))
|| ScreenAttrs[off] != attr
|| exmode_active;

if ((need_redraw || force_redraw_this)
#if defined(FEAT_PROP_POPUP)
&& !blocked_by_popup(row, col)
#endif
)
{
#if defined(FEAT_GUI) || defined(UNIX)




if (need_redraw && ScreenLines[off] != ' ' && (
#if defined(FEAT_GUI)
gui.in_use
#endif
#if defined(FEAT_GUI) && defined(UNIX)
||
#endif
#if defined(UNIX)
term_is_xterm
#endif
))
{
int n = ScreenAttrs[off];

if (n > HL_ALL)
n = syn_attr2attr(n);
if (n & HL_BOLD)
force_redraw_next = TRUE;
}
#endif





if (clear_next_cell)
clear_next_cell = FALSE;
else if (has_mbyte
&& (len < 0 ? ptr[mbyte_blen] == NUL
: ptr + mbyte_blen >= text + len)
&& ((mbyte_cells == 1 && (*mb_off2cells)(off, max_off) > 1)
|| (mbyte_cells == 2
&& (*mb_off2cells)(off, max_off) == 1
&& (*mb_off2cells)(off + 1, max_off) > 1)))
clear_next_cell = TRUE;



if (enc_dbcs
&& ((mbyte_cells == 1 && (*mb_off2cells)(off, max_off) > 1)
|| (mbyte_cells == 2
&& (*mb_off2cells)(off, max_off) == 1
&& (*mb_off2cells)(off + 1, max_off) > 1)))
ScreenLines[off + mbyte_blen] = 0;
ScreenLines[off] = c;
ScreenAttrs[off] = attr;
if (enc_utf8)
{
if (c < 0x80 && u8cc[0] == 0)
ScreenLinesUC[off] = 0;
else
{
int i;

ScreenLinesUC[off] = u8c;
for (i = 0; i < Screen_mco; ++i)
{
ScreenLinesC[i][off] = u8cc[i];
if (u8cc[i] == 0)
break;
}
}
if (mbyte_cells == 2)
{
ScreenLines[off + 1] = 0;
ScreenAttrs[off + 1] = attr;
}
screen_char(off, row, col);
}
else if (mbyte_cells == 2)
{
ScreenLines[off + 1] = ptr[1];
ScreenAttrs[off + 1] = attr;
screen_char_2(off, row, col);
}
else if (enc_dbcs == DBCS_JPNU && c == 0x8e)
{
ScreenLines2[off] = ptr[1];
screen_char(off, row, col);
}
else
screen_char(off, row, col);
}
if (has_mbyte)
{
off += mbyte_cells;
col += mbyte_cells;
ptr += mbyte_blen;
if (clear_next_cell)
{

ptr = (char_u *)" ";
len = -1;
}
}
else
{
++off;
++col;
++ptr;
}
}



if (force_redraw_next && col < screen_Columns)
{
if (enc_dbcs != 0 && dbcs_off2cells(off, max_off) > 1)
screen_char_2(off, row, col);
else
screen_char(off, row, col);
}
}

#if defined(FEAT_SEARCH_EXTRA) || defined(PROTO)



void
start_search_hl(void)
{
if (p_hls && !no_hlsearch)
{
last_pat_prog(&screen_search_hl.rm);
screen_search_hl.attr = HL_ATTR(HLF_L);
#if defined(FEAT_RELTIME)

profile_setlimit(p_rdt, &screen_search_hl.tm);
#endif
}
}




void
end_search_hl(void)
{
if (screen_search_hl.rm.regprog != NULL)
{
vim_regfree(screen_search_hl.rm.regprog);
screen_search_hl.rm.regprog = NULL;
}
}
#endif

static void
screen_start_highlight(int attr)
{
attrentry_T *aep = NULL;

screen_attr = attr;
if (full_screen
#if defined(MSWIN)
&& termcap_active
#endif
)
{
#if defined(FEAT_GUI)
if (gui.in_use)
{
char buf[20];


sprintf(buf, IF_EB("\033|%dh", ESC_STR "|%dh"), attr);
OUT_STR(buf);
}
else
#endif
{
if (attr > HL_ALL) 
{
if (IS_CTERM)
aep = syn_cterm_attr2entry(attr);
else
aep = syn_term_attr2entry(attr);
if (aep == NULL) 
attr = 0;
else
attr = aep->ae_attr;
}
#if defined(FEAT_VTP) && defined(FEAT_TERMGUICOLORS)
if (use_vtp())
{
guicolor_T defguifg, defguibg;
int defctermfg, defctermbg;



get_default_console_color(&defctermfg, &defctermbg, &defguifg,
&defguibg);

if (p_tgc)
{
if (aep == NULL || COLOR_INVALID(aep->ae_u.cterm.fg_rgb))
term_fg_rgb_color(defguifg);
if (aep == NULL || COLOR_INVALID(aep->ae_u.cterm.bg_rgb))
term_bg_rgb_color(defguibg);
}
else if (t_colors >= 256)
{
if (aep == NULL || aep->ae_u.cterm.fg_color == 0)
term_fg_color(defctermfg);
if (aep == NULL || aep->ae_u.cterm.bg_color == 0)
term_bg_color(defctermbg);
}
}
#endif
if ((attr & HL_BOLD) && *T_MD != NUL) 
out_str(T_MD);
else if (aep != NULL && cterm_normal_fg_bold && (
#if defined(FEAT_TERMGUICOLORS)
p_tgc && aep->ae_u.cterm.fg_rgb != CTERMCOLOR
? aep->ae_u.cterm.fg_rgb != INVALCOLOR
:
#endif
t_colors > 1 && aep->ae_u.cterm.fg_color))


out_str(T_ME);
if ((attr & HL_STANDOUT) && *T_SO != NUL) 
out_str(T_SO);
if ((attr & HL_UNDERCURL) && *T_UCS != NUL) 
out_str(T_UCS);
if (((attr & HL_UNDERLINE) 
|| ((attr & HL_UNDERCURL) && *T_UCS == NUL))
&& *T_US != NUL)
out_str(T_US);
if ((attr & HL_ITALIC) && *T_CZH != NUL) 
out_str(T_CZH);
if ((attr & HL_INVERSE) && *T_MR != NUL) 
out_str(T_MR);
if ((attr & HL_STRIKETHROUGH) && *T_STS != NUL) 
out_str(T_STS);





if (aep != NULL)
{
#if defined(FEAT_TERMGUICOLORS)



if (p_tgc && aep->ae_u.cterm.fg_rgb != CTERMCOLOR)
{
if (aep->ae_u.cterm.fg_rgb != INVALCOLOR)
term_fg_rgb_color(aep->ae_u.cterm.fg_rgb);
}
else
#endif
if (t_colors > 1)
{
if (aep->ae_u.cterm.fg_color)
term_fg_color(aep->ae_u.cterm.fg_color - 1);
}
#if defined(FEAT_TERMGUICOLORS)
if (p_tgc && aep->ae_u.cterm.bg_rgb != CTERMCOLOR)
{
if (aep->ae_u.cterm.bg_rgb != INVALCOLOR)
term_bg_rgb_color(aep->ae_u.cterm.bg_rgb);
}
else
#endif
if (t_colors > 1)
{
if (aep->ae_u.cterm.bg_color)
term_bg_color(aep->ae_u.cterm.bg_color - 1);
}

if (!IS_CTERM)
{
if (aep->ae_u.term.start != NULL)
out_str(aep->ae_u.term.start);
}
}
}
}
}

void
screen_stop_highlight(void)
{
int do_ME = FALSE; 

if (screen_attr != 0
#if defined(MSWIN)
&& termcap_active
#endif
)
{
#if defined(FEAT_GUI)
if (gui.in_use)
{
char buf[20];


sprintf(buf, IF_EB("\033|%dH", ESC_STR "|%dH"), screen_attr);
OUT_STR(buf);
}
else
#endif
{
if (screen_attr > HL_ALL) 
{
attrentry_T *aep;

if (IS_CTERM)
{



aep = syn_cterm_attr2entry(screen_attr);
if (aep != NULL && ((
#if defined(FEAT_TERMGUICOLORS)
p_tgc && aep->ae_u.cterm.fg_rgb != CTERMCOLOR
? aep->ae_u.cterm.fg_rgb != INVALCOLOR
:
#endif
aep->ae_u.cterm.fg_color) || (
#if defined(FEAT_TERMGUICOLORS)
p_tgc && aep->ae_u.cterm.bg_rgb != CTERMCOLOR
? aep->ae_u.cterm.bg_rgb != INVALCOLOR
:
#endif
aep->ae_u.cterm.bg_color)))
do_ME = TRUE;
}
else
{
aep = syn_term_attr2entry(screen_attr);
if (aep != NULL && aep->ae_u.term.stop != NULL)
{
if (STRCMP(aep->ae_u.term.stop, T_ME) == 0)
do_ME = TRUE;
else
out_str(aep->ae_u.term.stop);
}
}
if (aep == NULL) 
screen_attr = 0;
else
screen_attr = aep->ae_attr;
}





if (screen_attr & HL_STANDOUT)
{
if (STRCMP(T_SE, T_ME) == 0)
do_ME = TRUE;
else
out_str(T_SE);
}
if ((screen_attr & HL_UNDERCURL) && *T_UCE != NUL)
{
if (STRCMP(T_UCE, T_ME) == 0)
do_ME = TRUE;
else
out_str(T_UCE);
}
if ((screen_attr & HL_UNDERLINE)
|| ((screen_attr & HL_UNDERCURL) && *T_UCE == NUL))
{
if (STRCMP(T_UE, T_ME) == 0)
do_ME = TRUE;
else
out_str(T_UE);
}
if (screen_attr & HL_ITALIC)
{
if (STRCMP(T_CZR, T_ME) == 0)
do_ME = TRUE;
else
out_str(T_CZR);
}
if (screen_attr & HL_STRIKETHROUGH)
{
if (STRCMP(T_STE, T_ME) == 0)
do_ME = TRUE;
else
out_str(T_STE);
}
if (do_ME || (screen_attr & (HL_BOLD | HL_INVERSE)))
out_str(T_ME);

#if defined(FEAT_TERMGUICOLORS)
if (p_tgc)
{
if (cterm_normal_fg_gui_color != INVALCOLOR)
term_fg_rgb_color(cterm_normal_fg_gui_color);
if (cterm_normal_bg_gui_color != INVALCOLOR)
term_bg_rgb_color(cterm_normal_bg_gui_color);
}
else
#endif
{
if (t_colors > 1)
{

if (cterm_normal_fg_color != 0)
term_fg_color(cterm_normal_fg_color - 1);
if (cterm_normal_bg_color != 0)
term_bg_color(cterm_normal_bg_color - 1);
if (cterm_normal_fg_bold)
out_str(T_MD);
}
}
}
}
screen_attr = 0;
}





void
reset_cterm_colors(void)
{
if (IS_CTERM)
{

#if defined(FEAT_TERMGUICOLORS)
if (p_tgc ? (cterm_normal_fg_gui_color != INVALCOLOR
|| cterm_normal_bg_gui_color != INVALCOLOR)
: (cterm_normal_fg_color > 0 || cterm_normal_bg_color > 0))
#else
if (cterm_normal_fg_color > 0 || cterm_normal_bg_color > 0)
#endif
{
out_str(T_OP);
screen_attr = -1;
}
if (cterm_normal_fg_bold)
{
out_str(T_ME);
screen_attr = -1;
}
}
}





void
screen_char(unsigned off, int row, int col)
{
int attr;



if (row >= screen_Rows || col >= screen_Columns)
return;



if (pum_under_menu(row, col)
#if defined(FEAT_PROP_POPUP)
&& screen_zindex <= POPUPMENU_ZINDEX
#endif
)
return;
#if defined(FEAT_PROP_POPUP)
if (blocked_by_popup(row, col))
return;
#endif




if (*T_XN == NUL
&& row == screen_Rows - 1 && col == screen_Columns - 1
#if defined(FEAT_RIGHTLEFT)

&& !cmdmsg_rl
#endif
)
{
ScreenAttrs[off] = (sattr_T)-1;
return;
}




if (screen_char_attr != 0)
attr = screen_char_attr;
else
attr = ScreenAttrs[off];
if (screen_attr != attr)
screen_stop_highlight();

windgoto(row, col);

if (screen_attr != attr)
screen_start_highlight(attr);

if (enc_utf8 && ScreenLinesUC[off] != 0)
{
char_u buf[MB_MAXBYTES + 1];

if (utf_ambiguous_width(ScreenLinesUC[off]))
{
if (*p_ambw == 'd'
#if defined(FEAT_GUI)
&& !gui.in_use
#endif
)
{


out_str((char_u *)" ");
term_windgoto(row, col);
}


screen_cur_col = 9999;
}
else if (utf_char2cells(ScreenLinesUC[off]) > 1)
++screen_cur_col;


buf[utfc_char2bytes(off, buf)] = NUL;
out_str(buf);
}
else
{
out_flush_check();
out_char(ScreenLines[off]);

if (enc_dbcs == DBCS_JPNU && ScreenLines[off] == 0x8e)
out_char(ScreenLines2[off]);
}

screen_cur_col++;
}







static void
screen_char_2(unsigned off, int row, int col)
{

if (off + 1 >= (unsigned)(screen_Rows * screen_Columns))
return;



if (row == screen_Rows - 1 && col >= screen_Columns - 2)
{
ScreenAttrs[off] = (sattr_T)-1;
return;
}



screen_char(off, row, col);
out_char(ScreenLines[off + 1]);
++screen_cur_col;
}





void
screen_draw_rectangle(
int row,
int col,
int height,
int width,
int invert)
{
int r, c;
int off;
int max_off;


if (ScreenLines == NULL)
return;

if (invert)
screen_char_attr = HL_INVERSE;
for (r = row; r < row + height; ++r)
{
off = LineOffset[r];
max_off = off + screen_Columns;
for (c = col; c < col + width; ++c)
{
if (enc_dbcs != 0 && dbcs_off2cells(off + c, max_off) > 1)
{
screen_char_2(off + c, r, c);
++c;
}
else
{
screen_char(off + c, r, c);
if (utf_off2cells(off + c, max_off) > 1)
++c;
}
}
}
screen_char_attr = 0;
}




static void
redraw_block(int row, int end, win_T *wp)
{
int col;
int width;

#if defined(FEAT_CLIPBOARD)
clip_may_clear_selection(row, end - 1);
#endif

if (wp == NULL)
{
col = 0;
width = Columns;
}
else
{
col = wp->w_wincol;
width = wp->w_width;
}
screen_draw_rectangle(row, col, end - row, width, FALSE);
}

void
space_to_screenline(int off, int attr)
{
ScreenLines[off] = ' ';
ScreenAttrs[off] = attr;
if (enc_utf8)
ScreenLinesUC[off] = 0;
}






void
screen_fill(
int start_row,
int end_row,
int start_col,
int end_col,
int c1,
int c2,
int attr)
{
int row;
int col;
int off;
int end_off;
int did_delete;
int c;
int norm_term;
#if defined(FEAT_GUI) || defined(UNIX)
int force_next = FALSE;
#endif

if (end_row > screen_Rows) 
end_row = screen_Rows;
if (end_col > screen_Columns) 
end_col = screen_Columns;
if (ScreenLines == NULL
|| start_row >= end_row
|| start_col >= end_col) 
return;


norm_term = (
#if defined(FEAT_GUI)
!gui.in_use &&
#endif
!IS_CTERM);
for (row = start_row; row < end_row; ++row)
{
if (has_mbyte
#if defined(FEAT_GUI)
&& !gui.in_use
#endif
)
{




if (start_col > 0 && mb_fix_col(start_col, row) != start_col)
screen_puts_len((char_u *)" ", 1, row, start_col - 1, 0);
if (end_col < screen_Columns && mb_fix_col(end_col, row) != end_col)
screen_puts_len((char_u *)" ", 1, row, end_col, 0);
}





did_delete = FALSE;
if (c2 == ' '
&& end_col == Columns
&& can_clear(T_CE)
&& (attr == 0
|| (norm_term
&& attr <= HL_ALL
&& ((attr & ~(HL_BOLD | HL_ITALIC)) == 0))))
{



col = start_col;
if (c1 != ' ') 
++col;

off = LineOffset[row] + col;
end_off = LineOffset[row] + end_col;


if (enc_utf8)
while (off < end_off && ScreenLines[off] == ' '
&& ScreenAttrs[off] == 0 && ScreenLinesUC[off] == 0)
++off;
else
while (off < end_off && ScreenLines[off] == ' '
&& ScreenAttrs[off] == 0)
++off;
if (off < end_off) 
{
col = off - LineOffset[row];
screen_stop_highlight();
term_windgoto(row, col);
out_str(T_CE);
screen_start(); 
col = end_col - col;
while (col--) 
{
space_to_screenline(off, 0);
++off;
}
}
did_delete = TRUE; 
}

off = LineOffset[row] + start_col;
c = c1;
for (col = start_col; col < end_col; ++col)
{
if ((ScreenLines[off] != c
|| (enc_utf8 && (int)ScreenLinesUC[off]
!= (c >= 0x80 ? c : 0))
|| ScreenAttrs[off] != attr
#if defined(FEAT_GUI) || defined(UNIX)
|| force_next
#endif
)
#if defined(FEAT_PROP_POPUP)

&& !blocked_by_popup(row, col)
#endif
)
{
#if defined(FEAT_GUI) || defined(UNIX)




if (
#if defined(FEAT_GUI)
gui.in_use
#endif
#if defined(FEAT_GUI) && defined(UNIX)
||
#endif
#if defined(UNIX)
term_is_xterm
#endif
)
{
if (ScreenLines[off] != ' '
&& (ScreenAttrs[off] > HL_ALL
|| ScreenAttrs[off] & HL_BOLD))
force_next = TRUE;
else
force_next = FALSE;
}
#endif
ScreenLines[off] = c;
if (enc_utf8)
{
if (c >= 0x80)
{
ScreenLinesUC[off] = c;
ScreenLinesC[0][off] = 0;
}
else
ScreenLinesUC[off] = 0;
}
ScreenAttrs[off] = attr;
if (!did_delete || c != ' ')
screen_char(off, row, col);
}
++off;
if (col == start_col)
{
if (did_delete)
break;
c = c2;
}
}
if (end_col == Columns)
LineWraps[row] = FALSE;
if (row == Rows - 1) 
{
redraw_cmdline = TRUE;
if (start_col == 0 && end_col == Columns
&& c1 == ' ' && c2 == ' ' && attr == 0)
clear_cmdline = FALSE; 
if (start_col == 0)
mode_displayed = FALSE; 
}
}
}





void
check_for_delay(int check_msg_scroll)
{
if ((emsg_on_display || (check_msg_scroll && msg_scroll))
&& !did_wait_return
&& emsg_silent == 0)
{
out_flush();
ui_delay(1006L, TRUE);
emsg_on_display = FALSE;
if (check_msg_scroll)
msg_scroll = FALSE;
}
}




static void
clear_TabPageIdxs(void)
{
int scol;

for (scol = 0; scol < Columns; ++scol)
TabPageIdxs[scol] = 0;
}







int
screen_valid(int doclear)
{
screenalloc(doclear); 
return (ScreenLines != NULL);
}











void
screenalloc(int doclear)
{
int new_row, old_row;
#if defined(FEAT_GUI)
int old_Rows;
#endif
win_T *wp;
int outofmem = FALSE;
int len;
schar_T *new_ScreenLines;
u8char_T *new_ScreenLinesUC = NULL;
u8char_T *new_ScreenLinesC[MAX_MCO];
schar_T *new_ScreenLines2 = NULL;
int i;
sattr_T *new_ScreenAttrs;
unsigned *new_LineOffset;
char_u *new_LineWraps;
short *new_TabPageIdxs;
#if defined(FEAT_PROP_POPUP)
short *new_popup_mask;
short *new_popup_mask_next;
char *new_popup_transparent;
#endif
tabpage_T *tp;
static int entered = FALSE; 
static int done_outofmem_msg = FALSE; 
int retry_count = 0;

retry:





if ((ScreenLines != NULL
&& Rows == screen_Rows
&& Columns == screen_Columns
&& enc_utf8 == (ScreenLinesUC != NULL)
&& (enc_dbcs == DBCS_JPNU) == (ScreenLines2 != NULL)
&& p_mco == Screen_mco)
|| Rows == 0
|| Columns == 0
|| (!full_screen && ScreenLines == NULL))
return;






if (entered)
return;
entered = TRUE;





++RedrawingDisabled;

win_new_shellsize(); 

#if defined(FEAT_GUI_HAIKU)
vim_lock_screen(); 
#endif

comp_col(); 












FOR_ALL_TAB_WINDOWS(tp, wp)
win_free_lsize(wp);
if (aucmd_win != NULL)
win_free_lsize(aucmd_win);
#if defined(FEAT_PROP_POPUP)

FOR_ALL_POPUPWINS(wp)
win_free_lsize(wp);

FOR_ALL_TABPAGES(tp)
FOR_ALL_POPUPWINS_IN_TAB(tp, wp)
win_free_lsize(wp);
#endif

new_ScreenLines = LALLOC_MULT(schar_T, (Rows + 1) * Columns);
vim_memset(new_ScreenLinesC, 0, sizeof(u8char_T *) * MAX_MCO);
if (enc_utf8)
{
new_ScreenLinesUC = LALLOC_MULT(u8char_T, (Rows + 1) * Columns);
for (i = 0; i < p_mco; ++i)
new_ScreenLinesC[i] = LALLOC_CLEAR_MULT(u8char_T,
(Rows + 1) * Columns);
}
if (enc_dbcs == DBCS_JPNU)
new_ScreenLines2 = LALLOC_MULT(schar_T, (Rows + 1) * Columns);
new_ScreenAttrs = LALLOC_MULT(sattr_T, (Rows + 1) * Columns);
new_LineOffset = LALLOC_MULT(unsigned, Rows);
new_LineWraps = LALLOC_MULT(char_u, Rows);
new_TabPageIdxs = LALLOC_MULT(short, Columns);
#if defined(FEAT_PROP_POPUP)
new_popup_mask = LALLOC_MULT(short, Rows * Columns);
new_popup_mask_next = LALLOC_MULT(short, Rows * Columns);
new_popup_transparent = LALLOC_MULT(char, Rows * Columns);
#endif

FOR_ALL_TAB_WINDOWS(tp, wp)
{
if (win_alloc_lines(wp) == FAIL)
{
outofmem = TRUE;
goto give_up;
}
}
if (aucmd_win != NULL && aucmd_win->w_lines == NULL
&& win_alloc_lines(aucmd_win) == FAIL)
outofmem = TRUE;
#if defined(FEAT_PROP_POPUP)

FOR_ALL_POPUPWINS(wp)
if (win_alloc_lines(wp) == FAIL)
{
outofmem = TRUE;
goto give_up;
}

FOR_ALL_TABPAGES(tp)
FOR_ALL_POPUPWINS_IN_TAB(tp, wp)
if (win_alloc_lines(wp) == FAIL)
{
outofmem = TRUE;
goto give_up;
}
#endif

give_up:

for (i = 0; i < p_mco; ++i)
if (new_ScreenLinesC[i] == NULL)
break;
if (new_ScreenLines == NULL
|| (enc_utf8 && (new_ScreenLinesUC == NULL || i != p_mco))
|| (enc_dbcs == DBCS_JPNU && new_ScreenLines2 == NULL)
|| new_ScreenAttrs == NULL
|| new_LineOffset == NULL
|| new_LineWraps == NULL
|| new_TabPageIdxs == NULL
#if defined(FEAT_PROP_POPUP)
|| new_popup_mask == NULL
|| new_popup_mask_next == NULL
|| new_popup_transparent == NULL
#endif
|| outofmem)
{
if (ScreenLines != NULL || !done_outofmem_msg)
{

do_outofmem_msg((long_u)((Rows + 1) * Columns));



done_outofmem_msg = TRUE;
}
VIM_CLEAR(new_ScreenLines);
VIM_CLEAR(new_ScreenLinesUC);
for (i = 0; i < p_mco; ++i)
VIM_CLEAR(new_ScreenLinesC[i]);
VIM_CLEAR(new_ScreenLines2);
VIM_CLEAR(new_ScreenAttrs);
VIM_CLEAR(new_LineOffset);
VIM_CLEAR(new_LineWraps);
VIM_CLEAR(new_TabPageIdxs);
#if defined(FEAT_PROP_POPUP)
VIM_CLEAR(new_popup_mask);
VIM_CLEAR(new_popup_mask_next);
VIM_CLEAR(new_popup_transparent);
#endif
}
else
{
done_outofmem_msg = FALSE;

for (new_row = 0; new_row < Rows; ++new_row)
{
new_LineOffset[new_row] = new_row * Columns;
new_LineWraps[new_row] = FALSE;







if (!doclear)
{
(void)vim_memset(new_ScreenLines + new_row * Columns,
' ', (size_t)Columns * sizeof(schar_T));
if (enc_utf8)
{
(void)vim_memset(new_ScreenLinesUC + new_row * Columns,
0, (size_t)Columns * sizeof(u8char_T));
for (i = 0; i < p_mco; ++i)
(void)vim_memset(new_ScreenLinesC[i]
+ new_row * Columns,
0, (size_t)Columns * sizeof(u8char_T));
}
if (enc_dbcs == DBCS_JPNU)
(void)vim_memset(new_ScreenLines2 + new_row * Columns,
0, (size_t)Columns * sizeof(schar_T));
(void)vim_memset(new_ScreenAttrs + new_row * Columns,
0, (size_t)Columns * sizeof(sattr_T));
old_row = new_row + (screen_Rows - Rows);
if (old_row >= 0 && ScreenLines != NULL)
{
if (screen_Columns < Columns)
len = screen_Columns;
else
len = Columns;


if (!(enc_utf8 && ScreenLinesUC == NULL)
&& p_mco == Screen_mco)
mch_memmove(new_ScreenLines + new_LineOffset[new_row],
ScreenLines + LineOffset[old_row],
(size_t)len * sizeof(schar_T));
if (enc_utf8 && ScreenLinesUC != NULL
&& p_mco == Screen_mco)
{
mch_memmove(new_ScreenLinesUC + new_LineOffset[new_row],
ScreenLinesUC + LineOffset[old_row],
(size_t)len * sizeof(u8char_T));
for (i = 0; i < p_mco; ++i)
mch_memmove(new_ScreenLinesC[i]
+ new_LineOffset[new_row],
ScreenLinesC[i] + LineOffset[old_row],
(size_t)len * sizeof(u8char_T));
}
if (enc_dbcs == DBCS_JPNU && ScreenLines2 != NULL)
mch_memmove(new_ScreenLines2 + new_LineOffset[new_row],
ScreenLines2 + LineOffset[old_row],
(size_t)len * sizeof(schar_T));
mch_memmove(new_ScreenAttrs + new_LineOffset[new_row],
ScreenAttrs + LineOffset[old_row],
(size_t)len * sizeof(sattr_T));
}
}
}

current_ScreenLine = new_ScreenLines + Rows * Columns;

#if defined(FEAT_PROP_POPUP)
vim_memset(new_popup_mask, 0, Rows * Columns * sizeof(short));
vim_memset(new_popup_transparent, 0, Rows * Columns * sizeof(char));
#endif
}

free_screenlines();


ScreenLines = new_ScreenLines;
ScreenLinesUC = new_ScreenLinesUC;
for (i = 0; i < p_mco; ++i)
ScreenLinesC[i] = new_ScreenLinesC[i];
Screen_mco = p_mco;
ScreenLines2 = new_ScreenLines2;
ScreenAttrs = new_ScreenAttrs;
LineOffset = new_LineOffset;
LineWraps = new_LineWraps;
TabPageIdxs = new_TabPageIdxs;
#if defined(FEAT_PROP_POPUP)
popup_mask = new_popup_mask;
popup_mask_next = new_popup_mask_next;
popup_transparent = new_popup_transparent;
popup_mask_refresh = TRUE;
#endif



#if defined(FEAT_GUI)
old_Rows = screen_Rows;
#endif
screen_Rows = Rows;
screen_Columns = Columns;

must_redraw = CLEAR; 
if (doclear)
screenclear2();
#if defined(FEAT_GUI)
else if (gui.in_use
&& !gui.starting
&& ScreenLines != NULL
&& old_Rows != Rows)
{
gui_redraw_block(0, 0, (int)Rows - 1, (int)Columns - 1, 0);



if (msg_row >= Rows) 
msg_row = Rows - 1; 
else if (Rows > old_Rows) 
msg_row += Rows - old_Rows; 
if (msg_col >= Columns) 
msg_col = Columns - 1; 
}
#endif
clear_TabPageIdxs();

#if defined(FEAT_GUI_HAIKU)
vim_unlock_screen();
#endif

entered = FALSE;
--RedrawingDisabled;





if (starting == 0 && ++retry_count <= 3)
{
apply_autocmds(EVENT_VIMRESIZED, NULL, NULL, FALSE, curbuf);


goto retry;
}
}

void
free_screenlines(void)
{
int i;

VIM_CLEAR(ScreenLinesUC);
for (i = 0; i < Screen_mco; ++i)
VIM_CLEAR(ScreenLinesC[i]);
VIM_CLEAR(ScreenLines2);
VIM_CLEAR(ScreenLines);
VIM_CLEAR(ScreenAttrs);
VIM_CLEAR(LineOffset);
VIM_CLEAR(LineWraps);
VIM_CLEAR(TabPageIdxs);
#if defined(FEAT_PROP_POPUP)
VIM_CLEAR(popup_mask);
VIM_CLEAR(popup_mask_next);
VIM_CLEAR(popup_transparent);
#endif
}

void
screenclear(void)
{
check_for_delay(FALSE);
screenalloc(FALSE); 
screenclear2(); 
}

static void
screenclear2(void)
{
int i;

if (starting == NO_SCREEN || ScreenLines == NULL
#if defined(FEAT_GUI)
|| (gui.in_use && gui.starting)
#endif
)
return;

#if defined(FEAT_GUI)
if (!gui.in_use)
#endif
screen_attr = -1; 
screen_stop_highlight(); 

#if defined(FEAT_CLIPBOARD)

clip_scroll_selection(9999);
#endif


for (i = 0; i < Rows; ++i)
{
lineclear(LineOffset[i], (int)Columns, 0);
LineWraps[i] = FALSE;
}

if (can_clear(T_CL))
{
out_str(T_CL); 
clear_cmdline = FALSE;
mode_displayed = FALSE;
}
else
{

for (i = 0; i < Rows; ++i)
lineinvalid(LineOffset[i], (int)Columns);
clear_cmdline = TRUE;
}

screen_cleared = TRUE; 

win_rest_invalid(firstwin);
redraw_cmdline = TRUE;
redraw_tabline = TRUE;
if (must_redraw == CLEAR) 
must_redraw = NOT_VALID;
compute_cmdrow();
msg_row = cmdline_row; 
msg_col = 0;
screen_start(); 
msg_scrolled = 0; 
msg_didany = FALSE;
msg_didout = FALSE;
}




static void
lineclear(unsigned off, int width, int attr)
{
(void)vim_memset(ScreenLines + off, ' ', (size_t)width * sizeof(schar_T));
if (enc_utf8)
(void)vim_memset(ScreenLinesUC + off, 0,
(size_t)width * sizeof(u8char_T));
(void)vim_memset(ScreenAttrs + off, attr, (size_t)width * sizeof(sattr_T));
}





static void
lineinvalid(unsigned off, int width)
{
(void)vim_memset(ScreenAttrs + off, -1, (size_t)width * sizeof(sattr_T));
}




static void
linecopy(int to, int from, win_T *wp)
{
unsigned off_to = LineOffset[to] + wp->w_wincol;
unsigned off_from = LineOffset[from] + wp->w_wincol;

mch_memmove(ScreenLines + off_to, ScreenLines + off_from,
wp->w_width * sizeof(schar_T));
if (enc_utf8)
{
int i;

mch_memmove(ScreenLinesUC + off_to, ScreenLinesUC + off_from,
wp->w_width * sizeof(u8char_T));
for (i = 0; i < p_mco; ++i)
mch_memmove(ScreenLinesC[i] + off_to, ScreenLinesC[i] + off_from,
wp->w_width * sizeof(u8char_T));
}
if (enc_dbcs == DBCS_JPNU)
mch_memmove(ScreenLines2 + off_to, ScreenLines2 + off_from,
wp->w_width * sizeof(schar_T));
mch_memmove(ScreenAttrs + off_to, ScreenAttrs + off_from,
wp->w_width * sizeof(sattr_T));
}






int
can_clear(char_u *p)
{
return (*p != NUL && (t_colors <= 1
#if defined(FEAT_GUI)
|| gui.in_use
#endif
#if defined(FEAT_TERMGUICOLORS)
|| (p_tgc && cterm_normal_bg_gui_color == INVALCOLOR)
|| (!p_tgc && cterm_normal_bg_color == 0)
#else
|| cterm_normal_bg_color == 0
#endif
|| *T_UT != NUL)
#if defined(FEAT_PROP_POPUP)
&& !(p == T_CE && popup_visible)
#endif
);
}






void
screen_start(void)
{
screen_cur_row = screen_cur_col = 9999;
}






void
windgoto(int row, int col)
{
sattr_T *p;
int i;
int plan;
int cost;
int wouldbe_col;
int noinvcurs;
char_u *bs;
int goto_cost;
int attr;

#define GOTO_COST 7 
#define HIGHL_COST 5 

#define PLAN_LE 1
#define PLAN_CR 2
#define PLAN_NL 3
#define PLAN_WRITE 4

if (ScreenLines == NULL)
return;

if (col != screen_cur_col || row != screen_cur_row)
{

if (row < 0) 
row = 0;
if (row >= screen_Rows)
row = screen_Rows - 1;
if (col >= screen_Columns)
col = screen_Columns - 1;


if (screen_attr && *T_MS == NUL)
noinvcurs = HIGHL_COST;
else
noinvcurs = 0;
goto_cost = GOTO_COST + noinvcurs;














if (row >= screen_cur_row && screen_cur_col < Columns)
{




bs = NULL; 
attr = screen_attr;
if (row == screen_cur_row && col < screen_cur_col)
{

if (*T_LE)
bs = T_LE; 
else
bs = T_BC; 
if (*bs)
cost = (screen_cur_col - col) * (int)STRLEN(bs);
else
cost = 999;
if (col + 1 < cost) 
{
plan = PLAN_CR;
wouldbe_col = 0;
cost = 1; 
}
else
{
plan = PLAN_LE;
wouldbe_col = col;
}
if (noinvcurs) 
{
cost += noinvcurs;
attr = 0;
}
}




else if (row > screen_cur_row)
{
plan = PLAN_NL;
wouldbe_col = 0;
cost = (row - screen_cur_row) * 2; 
if (noinvcurs) 
{
cost += noinvcurs;
attr = 0;
}
}




else
{
plan = PLAN_WRITE;
wouldbe_col = screen_cur_col;
cost = 0;
}





i = col - wouldbe_col;
if (i > 0)
cost += i;
if (cost < goto_cost && i > 0)
{




p = ScreenAttrs + LineOffset[row] + wouldbe_col;
while (i && *p++ == attr)
--i;
if (i != 0)
{



if (*--p == 0)
{
cost += noinvcurs;
while (i && *p++ == 0)
--i;
}
if (i != 0)
cost = 999; 
}
if (enc_utf8)
{

for (i = wouldbe_col; i < col; ++i)
if (ScreenLinesUC[LineOffset[row] + i] != 0)
{
cost = 999;
break;
}
}
}




if (cost < goto_cost)
{
if (plan == PLAN_LE)
{
if (noinvcurs)
screen_stop_highlight();
while (screen_cur_col > col)
{
out_str(bs);
--screen_cur_col;
}
}
else if (plan == PLAN_CR)
{
if (noinvcurs)
screen_stop_highlight();
out_char('\r');
screen_cur_col = 0;
}
else if (plan == PLAN_NL)
{
if (noinvcurs)
screen_stop_highlight();
while (screen_cur_row < row)
{
out_char('\n');
++screen_cur_row;
}
screen_cur_col = 0;
}

i = col - screen_cur_col;
if (i > 0)
{





if (T_ND[0] != NUL && T_ND[1] == NUL)
{
while (i-- > 0)
out_char(*T_ND);
}
else
{
int off;

off = LineOffset[row] + screen_cur_col;
while (i-- > 0)
{
if (ScreenAttrs[off] != screen_attr)
screen_stop_highlight();
out_flush_check();
out_char(ScreenLines[off]);
if (enc_dbcs == DBCS_JPNU
&& ScreenLines[off] == 0x8e)
out_char(ScreenLines2[off]);
++off;
}
}
}
}
}
else
cost = 999;

if (cost >= goto_cost)
{
if (noinvcurs)
screen_stop_highlight();
if (row == screen_cur_row && (col > screen_cur_col)
&& *T_CRI != NUL)
term_cursor_right(col - screen_cur_col);
else
term_windgoto(row, col);
}
screen_cur_row = row;
screen_cur_col = col;
}
}




void
setcursor(void)
{
setcursor_mayforce(FALSE);
}





void
setcursor_mayforce(int force)
{
if (force || redrawing())
{
validate_cursor();
windgoto(W_WINROW(curwin) + curwin->w_wrow,
curwin->w_wincol + (
#if defined(FEAT_RIGHTLEFT)


curwin->w_p_rl ? ((int)curwin->w_width - curwin->w_wcol
- ((has_mbyte
&& (*mb_ptr2cells)(ml_get_cursor()) == 2
&& vim_isprintc(gchar_cursor())) ? 2 : 1)) :
#endif
curwin->w_wcol));
}
}









int
win_ins_lines(
win_T *wp,
int row,
int line_count,
int invalid,
int mayclear)
{
int did_delete;
int nextrow;
int lastrow;
int retval;

if (invalid)
wp->w_lines_valid = 0;

if (wp->w_height < 5)
return FAIL;

if (line_count > wp->w_height - row)
line_count = wp->w_height - row;

retval = win_do_lines(wp, row, line_count, mayclear, FALSE, 0);
if (retval != MAYBE)
return retval;







did_delete = FALSE;
if (wp->w_next != NULL || wp->w_status_height)
{
if (screen_del_lines(0, W_WINROW(wp) + wp->w_height - line_count,
line_count, (int)Rows, FALSE, 0, NULL) == OK)
did_delete = TRUE;
else if (wp->w_next)
return FAIL;
}



if (!did_delete)
{
wp->w_redr_status = TRUE;
redraw_cmdline = TRUE;
nextrow = W_WINROW(wp) + wp->w_height + wp->w_status_height;
lastrow = nextrow + line_count;
if (lastrow > Rows)
lastrow = Rows;
screen_fill(nextrow - line_count, lastrow - line_count,
wp->w_wincol, (int)W_ENDCOL(wp),
' ', ' ', 0);
}

if (screen_ins_lines(0, W_WINROW(wp) + row, line_count, (int)Rows, 0, NULL)
== FAIL)
{

if (did_delete)
{
wp->w_redr_status = TRUE;
win_rest_invalid(W_NEXT(wp));
}
return FAIL;
}

return OK;
}








int
win_del_lines(
win_T *wp,
int row,
int line_count,
int invalid,
int mayclear,
int clear_attr) 
{
int retval;

if (invalid)
wp->w_lines_valid = 0;

if (line_count > wp->w_height - row)
line_count = wp->w_height - row;

retval = win_do_lines(wp, row, line_count, mayclear, TRUE, clear_attr);
if (retval != MAYBE)
return retval;

if (screen_del_lines(0, W_WINROW(wp) + row, line_count,
(int)Rows, FALSE, clear_attr, NULL) == FAIL)
return FAIL;





if (wp->w_next || wp->w_status_height || cmdline_row < Rows - 1)
{
if (screen_ins_lines(0, W_WINROW(wp) + wp->w_height - line_count,
line_count, (int)Rows, clear_attr, NULL) == FAIL)
{
wp->w_redr_status = TRUE;
win_rest_invalid(wp->w_next);
}
}




else
redraw_cmdline = TRUE;
return OK;
}






static int
win_do_lines(
win_T *wp,
int row,
int line_count,
int mayclear,
int del,
int clear_attr)
{
int retval;

if (!redrawing() || line_count <= 0)
return FAIL;



if (no_win_do_lines_ins && !del)
return FAIL;


if (mayclear && Rows - line_count < 5 && wp->w_width == Columns)
{
if (!no_win_do_lines_ins)
screenclear(); 
return FAIL;
}

#if defined(FEAT_PROP_POPUP)

if (popup_visible)
return FAIL;
#endif


if (row + line_count >= wp->w_height)
{
screen_fill(W_WINROW(wp) + row, W_WINROW(wp) + wp->w_height,
wp->w_wincol, (int)W_ENDCOL(wp),
' ', ' ', 0);
return OK;
}






if (!no_win_do_lines_ins)
clear_cmdline = TRUE;










if (scroll_region || wp->w_width != Columns)
{
if (scroll_region && (wp->w_width == Columns || *T_CSV != NUL))
scroll_region_set(wp, row);
if (del)
retval = screen_del_lines(W_WINROW(wp) + row, 0, line_count,
wp->w_height - row, FALSE, clear_attr, wp);
else
retval = screen_ins_lines(W_WINROW(wp) + row, 0, line_count,
wp->w_height - row, clear_attr, wp);
if (scroll_region && (wp->w_width == Columns || *T_CSV != NUL))
scroll_region_reset();
return retval;
}

if (wp->w_next != NULL && p_tf) 
return FAIL;

return MAYBE;
}




static void
win_rest_invalid(win_T *wp)
{
while (wp != NULL)
{
redraw_win_later(wp, NOT_VALID);
wp->w_redr_status = TRUE;
wp = wp->w_next;
}
redraw_cmdline = TRUE;
}














#define USE_T_CAL 1
#define USE_T_CDL 2
#define USE_T_AL 3
#define USE_T_CE 4
#define USE_T_DL 5
#define USE_T_SR 6
#define USE_NL 7
#define USE_T_CD 8
#define USE_REDRAW 9









int
screen_ins_lines(
int off,
int row,
int line_count,
int end,
int clear_attr,
win_T *wp) 
{
int i;
int j;
unsigned temp;
int cursor_row;
int cursor_col = 0;
int type;
int result_empty;
int can_ce = can_clear(T_CE);










if (!screen_valid(TRUE)
|| line_count <= 0 || line_count > p_ttyscroll
#if defined(FEAT_CLIPBOARD)
|| (clip_star.state != SELECT_CLEARED
&& redrawing_for_callback > 0)
#endif
#if defined(FEAT_PROP_POPUP)
|| popup_visible
#endif
)
return FAIL;


























result_empty = (row + line_count >= end);
if (wp != NULL && wp->w_width != Columns && *T_CSV == NUL)
type = USE_REDRAW;
else if (can_clear(T_CD) && result_empty)
type = USE_T_CD;
else if (*T_CAL != NUL && (line_count > 1 || *T_AL == NUL))
type = USE_T_CAL;
else if (*T_CDL != NUL && result_empty && (line_count > 1 || !can_ce))
type = USE_T_CDL;
else if (*T_AL != NUL)
type = USE_T_AL;
else if (can_ce && result_empty)
type = USE_T_CE;
else if (*T_DL != NUL && result_empty)
type = USE_T_DL;
else if (*T_SR != NUL && row == 0 && (*T_DA == NUL || can_ce))
type = USE_T_SR;
else
return FAIL;





if (type == USE_T_CD || type == USE_T_CDL ||
type == USE_T_CE || type == USE_T_DL)
return screen_del_lines(off, row, line_count, end, FALSE, 0, wp);






if (*T_DB)
screen_del_lines(off, end - line_count, line_count, end, FALSE, 0, wp);

#if defined(FEAT_CLIPBOARD)


if (off + row > 0 || (wp != NULL && wp->w_width != Columns))
clip_clear_selection(&clip_star);
else
clip_scroll_selection(-line_count);
#endif

#if defined(FEAT_GUI_HAIKU)
vim_lock_screen();
#endif

#if defined(FEAT_GUI)


gui_dont_update_cursor(row + off <= gui.cursor_row);
#endif

if (wp != NULL && wp->w_wincol != 0 && *T_CSV != NUL && *T_CCS == NUL)
cursor_col = wp->w_wincol;

if (*T_CCS != NUL) 
cursor_row = row;
else
cursor_row = row + off;





row += off;
end += off;
for (i = 0; i < line_count; ++i)
{
if (wp != NULL && wp->w_width != Columns)
{

j = end - 1 - i;
while ((j -= line_count) >= row)
linecopy(j + line_count, j, wp);
j += line_count;
if (can_clear((char_u *)" "))
lineclear(LineOffset[j] + wp->w_wincol, wp->w_width,
clear_attr);
else
lineinvalid(LineOffset[j] + wp->w_wincol, wp->w_width);
LineWraps[j] = FALSE;
}
else
{
j = end - 1 - i;
temp = LineOffset[j];
while ((j -= line_count) >= row)
{
LineOffset[j + line_count] = LineOffset[j];
LineWraps[j + line_count] = LineWraps[j];
}
LineOffset[j + line_count] = temp;
LineWraps[j + line_count] = FALSE;
if (can_clear((char_u *)" "))
lineclear(temp, (int)Columns, clear_attr);
else
lineinvalid(temp, (int)Columns);
}
}

#if defined(FEAT_GUI_HAIKU)
vim_unlock_screen();
#endif

screen_stop_highlight();
windgoto(cursor_row, cursor_col);
if (clear_attr != 0)
screen_start_highlight(clear_attr);


if (type == USE_REDRAW)
redraw_block(row, end, wp);
else if (type == USE_T_CAL)
{
term_append_lines(line_count);
screen_start(); 
}
else
{
for (i = 0; i < line_count; i++)
{
if (type == USE_T_AL)
{
if (i && cursor_row != 0)
windgoto(cursor_row, cursor_col);
out_str(T_AL);
}
else 
out_str(T_SR);
screen_start(); 
}
}





if (type == USE_T_SR && *T_DA)
{
for (i = 0; i < line_count; ++i)
{
windgoto(off + i, cursor_col);
out_str(T_CE);
screen_start(); 
}
}

#if defined(FEAT_GUI)
gui_can_update_cursor();
if (gui.in_use)
out_flush(); 
#endif
return OK;
}









int
screen_del_lines(
int off,
int row,
int line_count,
int end,
int force, 
int clear_attr, 
win_T *wp UNUSED) 
{
int j;
int i;
unsigned temp;
int cursor_row;
int cursor_col = 0;
int cursor_end;
int result_empty; 
int can_delete; 
int type;









if (!screen_valid(TRUE) || line_count <= 0
|| (!force && line_count > p_ttyscroll)
#if defined(FEAT_CLIPBOARD)
|| (clip_star.state != SELECT_CLEARED
&& redrawing_for_callback > 0)
#endif
)
return FAIL;




result_empty = row + line_count >= end;





can_delete = (*T_DB == NUL || can_clear(T_CE));













if (wp != NULL && wp->w_width != Columns && *T_CSV == NUL)
type = USE_REDRAW;
else if (can_clear(T_CD) && result_empty)
type = USE_T_CD;
#if defined(__BEOS__) && defined(BEOS_DR8)









else if (row == 0 && T_DB == empty_option
&& (line_count == 1 || *T_CDL == NUL))
#else
else if (row == 0 && (
#if !defined(AMIGA)


line_count == 1 ||
#endif
*T_CDL == NUL))
#endif
type = USE_NL;
else if (*T_CDL != NUL && line_count > 1 && can_delete)
type = USE_T_CDL;
else if (can_clear(T_CE) && result_empty
&& (wp == NULL || wp->w_width == Columns))
type = USE_T_CE;
else if (*T_DL != NUL && can_delete)
type = USE_T_DL;
else if (*T_CDL != NUL && can_delete)
type = USE_T_CDL;
else
return FAIL;

#if defined(FEAT_CLIPBOARD)


if (off + row > 0 || (wp != NULL && wp->w_width != Columns))
clip_clear_selection(&clip_star);
else
clip_scroll_selection(line_count);
#endif

#if defined(FEAT_GUI)


gui_dont_update_cursor(gui.cursor_row >= row + off
&& gui.cursor_row < end + off);
#endif

if (wp != NULL && wp->w_wincol != 0 && *T_CSV != NUL && *T_CCS == NUL)
cursor_col = wp->w_wincol;

if (*T_CCS != NUL) 
{
cursor_row = row;
cursor_end = end;
}
else
{
cursor_row = row + off;
cursor_end = end + off;
}





row += off;
end += off;
for (i = 0; i < line_count; ++i)
{
if (wp != NULL && wp->w_width != Columns)
{

j = row + i;
while ((j += line_count) <= end - 1)
linecopy(j - line_count, j, wp);
j -= line_count;
if (can_clear((char_u *)" "))
lineclear(LineOffset[j] + wp->w_wincol, wp->w_width,
clear_attr);
else
lineinvalid(LineOffset[j] + wp->w_wincol, wp->w_width);
LineWraps[j] = FALSE;
}
else
{

j = row + i;
temp = LineOffset[j];
while ((j += line_count) <= end - 1)
{
LineOffset[j - line_count] = LineOffset[j];
LineWraps[j - line_count] = LineWraps[j];
}
LineOffset[j - line_count] = temp;
LineWraps[j - line_count] = FALSE;
if (can_clear((char_u *)" "))
lineclear(temp, (int)Columns, clear_attr);
else
lineinvalid(temp, (int)Columns);
}
}

#if defined(FEAT_GUI_HAIKU)
vim_unlock_screen();
#endif

if (screen_attr != clear_attr)
screen_stop_highlight();
if (clear_attr != 0)
screen_start_highlight(clear_attr);


if (type == USE_REDRAW)
redraw_block(row, end, wp);
else if (type == USE_T_CD) 
{
windgoto(cursor_row, cursor_col);
out_str(T_CD);
screen_start(); 
}
else if (type == USE_T_CDL)
{
windgoto(cursor_row, cursor_col);
term_delete_lines(line_count);
screen_start(); 
}





else if (type == USE_NL)
{
windgoto(cursor_end - 1, cursor_col);
for (i = line_count; --i >= 0; )
out_char('\n'); 
}
else
{
for (i = line_count; --i >= 0; )
{
if (type == USE_T_DL)
{
windgoto(cursor_row, cursor_col);
out_str(T_DL); 
}
else 
{
windgoto(cursor_row + i, cursor_col);
out_str(T_CE); 
}
screen_start(); 
}
}





if (*T_DB && (type == USE_T_DL || type == USE_T_CDL))
{
for (i = line_count; i > 0; --i)
{
windgoto(cursor_end - i, cursor_col);
out_str(T_CE); 
screen_start(); 
}
}

#if defined(FEAT_GUI)
gui_can_update_cursor();
if (gui.in_use)
out_flush(); 
#endif

return OK;
}





int
skip_showmode()
{


if (global_busy
|| msg_silent != 0
|| !redrawing()
|| (char_avail() && !KeyTyped))
{
redraw_mode = TRUE; 
return TRUE;
}
return FALSE;
}










int
showmode(void)
{
int need_clear;
int length = 0;
int do_mode;
int attr;
int nwr_save;
int sub_attr;

do_mode = ((p_smd && msg_silent == 0)
&& ((State & INSERT)
|| restart_edit != NUL
|| VIsual_active));
if (do_mode || reg_recording != 0)
{
if (skip_showmode())
return 0; 

nwr_save = need_wait_return;


check_for_delay(FALSE);


need_clear = clear_cmdline;
if (clear_cmdline && cmdline_row < Rows - 1)
msg_clr_cmdline(); 


msg_pos_mode();
cursor_off();
attr = HL_ATTR(HLF_CM); 
if (do_mode)
{
msg_puts_attr("--", attr);
#if defined(FEAT_XIM)
if (
#if defined(FEAT_GUI_GTK)
preedit_get_status()
#else
im_get_status()
#endif
)
#if defined(FEAT_GUI_GTK)
msg_puts_attr(" IM", attr);
#else
msg_puts_attr(" XIM", attr);
#endif
#endif

if (edit_submode != NULL && !shortmess(SHM_COMPLETIONMENU))
{


length = (Rows - msg_row) * Columns - 3;
if (edit_submode_extra != NULL)
length -= vim_strsize(edit_submode_extra);
if (length > 0)
{
if (edit_submode_pre != NULL)
length -= vim_strsize(edit_submode_pre);
if (length - vim_strsize(edit_submode) > 0)
{
if (edit_submode_pre != NULL)
msg_puts_attr((char *)edit_submode_pre, attr);
msg_puts_attr((char *)edit_submode, attr);
}
if (edit_submode_extra != NULL)
{
msg_puts_attr(" ", attr); 
if ((int)edit_submode_highl < (int)HLF_COUNT)
sub_attr = HL_ATTR(edit_submode_highl);
else
sub_attr = attr;
msg_puts_attr((char *)edit_submode_extra, sub_attr);
}
}
}
else
{
if (State & VREPLACE_FLAG)
msg_puts_attr(_(" VREPLACE"), attr);
else if (State & REPLACE_FLAG)
msg_puts_attr(_(" REPLACE"), attr);
else if (State & INSERT)
{
#if defined(FEAT_RIGHTLEFT)
if (p_ri)
msg_puts_attr(_(" REVERSE"), attr);
#endif
msg_puts_attr(_(" INSERT"), attr);
}
else if (restart_edit == 'I' || restart_edit == 'A')
msg_puts_attr(_(" (insert)"), attr);
else if (restart_edit == 'R')
msg_puts_attr(_(" (replace)"), attr);
else if (restart_edit == 'V')
msg_puts_attr(_(" (vreplace)"), attr);
#if defined(FEAT_RIGHTLEFT)
if (p_hkmap)
msg_puts_attr(_(" Hebrew"), attr);
#endif
#if defined(FEAT_KEYMAP)
if (State & LANGMAP)
{
#if defined(FEAT_ARABIC)
if (curwin->w_p_arab)
msg_puts_attr(_(" Arabic"), attr);
else
#endif
if (get_keymap_str(curwin, (char_u *)" (%s)",
NameBuff, MAXPATHL))
msg_puts_attr((char *)NameBuff, attr);
}
#endif
if ((State & INSERT) && p_paste)
msg_puts_attr(_(" (paste)"), attr);

if (VIsual_active)
{
char *p;



switch ((VIsual_select ? 4 : 0)
+ (VIsual_mode == Ctrl_V) * 2
+ (VIsual_mode == 'V'))
{
case 0: p = N_(" VISUAL"); break;
case 1: p = N_(" VISUAL LINE"); break;
case 2: p = N_(" VISUAL BLOCK"); break;
case 4: p = N_(" SELECT"); break;
case 5: p = N_(" SELECT LINE"); break;
default: p = N_(" SELECT BLOCK"); break;
}
msg_puts_attr(_(p), attr);
}
msg_puts_attr(" --", attr);
}

need_clear = TRUE;
}
if (reg_recording != 0
&& edit_submode == NULL) 
{
recording_mode(attr);
need_clear = TRUE;
}

mode_displayed = TRUE;
if (need_clear || clear_cmdline || redraw_mode)
msg_clr_eos();
msg_didout = FALSE; 
length = msg_col;
msg_col = 0;
need_wait_return = nwr_save; 
}
else if (clear_cmdline && msg_silent == 0)

msg_clr_cmdline();
else if (redraw_mode)
{
msg_pos_mode();
msg_clr_eos();
}

#if defined(FEAT_CMDL_INFO)

if (VIsual_active)
clear_showcmd();



if (redrawing() && lastwin->w_status_height == 0)
win_redr_ruler(lastwin, TRUE, FALSE);
#endif
redraw_cmdline = FALSE;
redraw_mode = FALSE;
clear_cmdline = FALSE;

return length;
}




static void
msg_pos_mode(void)
{
msg_col = 0;
msg_row = Rows - 1;
}






void
unshowmode(int force)
{



if (!redrawing() || (!force && char_avail() && !KeyTyped))
redraw_cmdline = TRUE; 
else
clearmode();
}




void
clearmode(void)
{
int save_msg_row = msg_row;
int save_msg_col = msg_col;

msg_pos_mode();
if (reg_recording != 0)
recording_mode(HL_ATTR(HLF_CM));
msg_clr_eos();

msg_col = save_msg_col;
msg_row = save_msg_row;
}

static void
recording_mode(int attr)
{
msg_puts_attr(_("recording"), attr);
if (!shortmess(SHM_RECORDING))
{
char s[4];

sprintf(s, " @%c", reg_recording);
msg_puts_attr(s, attr);
}
}




void
draw_tabline(void)
{
int tabcount = 0;
tabpage_T *tp;
int tabwidth;
int col = 0;
int scol = 0;
int attr;
win_T *wp;
win_T *cwp;
int wincount;
int modified;
int c;
int len;
int attr_sel = HL_ATTR(HLF_TPS);
int attr_nosel = HL_ATTR(HLF_TP);
int attr_fill = HL_ATTR(HLF_TPF);
char_u *p;
int room;
int use_sep_chars = (t_colors < 8
#if defined(FEAT_GUI)
&& !gui.in_use
#endif
#if defined(FEAT_TERMGUICOLORS)
&& !p_tgc
#endif
);

if (ScreenLines == NULL)
return;
redraw_tabline = FALSE;

#if defined(FEAT_GUI_TABLINE)

if (gui_use_tabline())
{
gui_update_tabline();
return;
}
#endif

if (tabline_height() < 1)
return;

#if defined(FEAT_STL_OPT)
clear_TabPageIdxs();


if (*p_tal != NUL)
{
int saved_did_emsg = did_emsg;



did_emsg = FALSE;
win_redr_custom(NULL, FALSE);
if (did_emsg)
set_string_option_direct((char_u *)"tabline", -1,
(char_u *)"", OPT_FREE, SID_ERROR);
did_emsg |= saved_did_emsg;
}
else
#endif
{
FOR_ALL_TABPAGES(tp)
++tabcount;

tabwidth = (Columns - 1 + tabcount / 2) / tabcount;
if (tabwidth < 6)
tabwidth = 6;

attr = attr_nosel;
tabcount = 0;
for (tp = first_tabpage; tp != NULL && col < Columns - 4;
tp = tp->tp_next)
{
scol = col;

if (tp->tp_topframe == topframe)
attr = attr_sel;
if (use_sep_chars && col > 0)
screen_putchar('|', 0, col++, attr);

if (tp->tp_topframe != topframe)
attr = attr_nosel;

screen_putchar(' ', 0, col++, attr);

if (tp == curtab)
{
cwp = curwin;
wp = firstwin;
}
else
{
cwp = tp->tp_curwin;
wp = tp->tp_firstwin;
}

modified = FALSE;
for (wincount = 0; wp != NULL; wp = wp->w_next, ++wincount)
if (bufIsChanged(wp->w_buffer))
modified = TRUE;
if (modified || wincount > 1)
{
if (wincount > 1)
{
vim_snprintf((char *)NameBuff, MAXPATHL, "%d", wincount);
len = (int)STRLEN(NameBuff);
if (col + len >= Columns - 3)
break;
screen_puts_len(NameBuff, len, 0, col,
#if defined(FEAT_SYN_HL)
hl_combine_attr(attr, HL_ATTR(HLF_T))
#else
attr
#endif
);
col += len;
}
if (modified)
screen_puts_len((char_u *)"+", 1, 0, col++, attr);
screen_putchar(' ', 0, col++, attr);
}

room = scol - col + tabwidth - 1;
if (room > 0)
{

get_trans_bufname(cwp->w_buffer);
shorten_dir(NameBuff);
len = vim_strsize(NameBuff);
p = NameBuff;
if (has_mbyte)
while (len > room)
{
len -= ptr2cells(p);
MB_PTR_ADV(p);
}
else if (len > room)
{
p += len - room;
len = room;
}
if (len > Columns - col - 1)
len = Columns - col - 1;

screen_puts_len(p, (int)STRLEN(p), 0, col, attr);
col += len;
}
screen_putchar(' ', 0, col++, attr);



++tabcount;
while (scol < col)
TabPageIdxs[scol++] = tabcount;
}

if (use_sep_chars)
c = '_';
else
c = ' ';
screen_fill(0, 1, col, (int)Columns, c, c, attr_fill);


if (first_tabpage->tp_next != NULL)
{
screen_putchar('X', 0, (int)Columns - 1, attr_nosel);
TabPageIdxs[Columns - 1] = -999;
}
}



redraw_tabline = FALSE;
}





void
get_trans_bufname(buf_T *buf)
{
if (buf_spname(buf) != NULL)
vim_strncpy(NameBuff, buf_spname(buf), MAXPATHL - 1);
else
home_replace(buf, buf->b_fname, NameBuff, MAXPATHL, TRUE);
trans_characters(NameBuff, MAXPATHL);
}




int
fillchar_status(int *attr, win_T *wp)
{
int fill;

#if defined(FEAT_TERMINAL)
if (bt_terminal(wp->w_buffer))
{
if (wp == curwin)
{
*attr = HL_ATTR(HLF_ST);
fill = fill_stl;
}
else
{
*attr = HL_ATTR(HLF_STNC);
fill = fill_stlnc;
}
}
else
#endif
if (wp == curwin)
{
*attr = HL_ATTR(HLF_S);
fill = fill_stl;
}
else
{
*attr = HL_ATTR(HLF_SNC);
fill = fill_stlnc;
}



if (*attr != 0 && ((HL_ATTR(HLF_S) != HL_ATTR(HLF_SNC)
|| wp != curwin || ONE_WINDOW)
|| (fill_stl != fill_stlnc)))
return fill;
if (wp == curwin)
return '^';
return '=';
}





int
fillchar_vsep(int *attr)
{
*attr = HL_ATTR(HLF_C);
if (*attr == 0 && fill_vert == ' ')
return '|';
else
return fill_vert;
}




int
redrawing(void)
{
#if defined(FEAT_EVAL)
if (disable_redraw_for_testing)
return 0;
else
#endif
return ((!RedrawingDisabled
#if defined(FEAT_EVAL)
|| ignore_redraw_flag_for_testing
#endif
) && !(p_lz && char_avail() && !KeyTyped && !do_redraw));
}




int
messaging(void)
{
return (!(p_lz && char_avail() && !KeyTyped));
}








#define COL_RULER 17 

void
comp_col(void)
{
#if defined(FEAT_CMDL_INFO)
int last_has_status = (p_ls == 2 || (p_ls == 1 && !ONE_WINDOW));

sc_col = 0;
ru_col = 0;
if (p_ru)
{
#if defined(FEAT_STL_OPT)
ru_col = (ru_wid ? ru_wid : COL_RULER) + 1;
#else
ru_col = COL_RULER + 1;
#endif

if (!last_has_status)
sc_col = ru_col;
}
if (p_sc)
{
sc_col += SHOWCMD_COLS;
if (!p_ru || last_has_status) 
++sc_col;
}
sc_col = Columns - sc_col;
ru_col = Columns - ru_col;
if (sc_col <= 0) 
sc_col = 1;
if (ru_col <= 0)
ru_col = 1;
#else
sc_col = Columns;
ru_col = Columns;
#endif
#if defined(FEAT_EVAL)
set_vim_var_nr(VV_ECHOSPACE, sc_col - 1);
#endif
}

#if defined(FEAT_LINEBREAK) || defined(PROTO)





int
number_width(win_T *wp)
{
int n;
linenr_T lnum;

if (wp->w_p_rnu && !wp->w_p_nu)

lnum = wp->w_height;
else

lnum = wp->w_buffer->b_ml.ml_line_count;

if (lnum == wp->w_nrwidth_line_count && wp->w_nuw_cached == wp->w_p_nuw)
return wp->w_nrwidth_width;
wp->w_nrwidth_line_count = lnum;

n = 0;
do
{
lnum /= 10;
++n;
} while (lnum > 0);


if (n < wp->w_p_nuw - 1)
n = wp->w_p_nuw - 1;

#if defined(FEAT_SIGNS)


if (n < 2 && get_first_valid_sign(wp) != NULL
&& (*wp->w_p_scl == 'n' && *(wp->w_p_scl + 1) == 'u'))
n = 2;
#endif

wp->w_nrwidth_width = n;
wp->w_nuw_cached = wp->w_p_nuw;
return n;
}
#endif

#if defined(FEAT_EVAL) || defined(PROTO)




int
screen_screencol(void)
{
return screen_cur_col;
}





int
screen_screenrow(void)
{
return screen_cur_row;
}
#endif





char *
set_chars_option(char_u **varp)
{
int round, i, len, entries;
char_u *p, *s;
int c1 = 0, c2 = 0, c3 = 0;
struct charstab
{
int *cp;
char *name;
};
static struct charstab filltab[] =
{
{&fill_stl, "stl"},
{&fill_stlnc, "stlnc"},
{&fill_vert, "vert"},
{&fill_fold, "fold"},
{&fill_diff, "diff"},
};
static struct charstab lcstab[] =
{
{&lcs_eol, "eol"},
{&lcs_ext, "extends"},
{&lcs_nbsp, "nbsp"},
{&lcs_prec, "precedes"},
{&lcs_space, "space"},
{&lcs_tab2, "tab"},
{&lcs_trail, "trail"},
#if defined(FEAT_CONCEAL)
{&lcs_conceal, "conceal"},
#else
{NULL, "conceal"},
#endif
};
struct charstab *tab;

if (varp == &p_lcs)
{
tab = lcstab;
entries = sizeof(lcstab) / sizeof(struct charstab);
}
else
{
tab = filltab;
entries = sizeof(filltab) / sizeof(struct charstab);
}


for (round = 0; round <= 1; ++round)
{
if (round > 0)
{


for (i = 0; i < entries; ++i)
if (tab[i].cp != NULL)
*(tab[i].cp) = (varp == &p_lcs ? NUL : ' ');

if (varp == &p_lcs)
{
lcs_tab1 = NUL;
lcs_tab3 = NUL;
}
else
fill_diff = '-';
}
p = *varp;
while (*p)
{
for (i = 0; i < entries; ++i)
{
len = (int)STRLEN(tab[i].name);
if (STRNCMP(p, tab[i].name, len) == 0
&& p[len] == ':'
&& p[len + 1] != NUL)
{
c2 = c3 = 0;
s = p + len + 1;
c1 = mb_ptr2char_adv(&s);
if (mb_char2cells(c1) > 1)
continue;
if (tab[i].cp == &lcs_tab2)
{
if (*s == NUL)
continue;
c2 = mb_ptr2char_adv(&s);
if (mb_char2cells(c2) > 1)
continue;
if (!(*s == ',' || *s == NUL))
{
c3 = mb_ptr2char_adv(&s);
if (mb_char2cells(c3) > 1)
continue;
}
}

if (*s == ',' || *s == NUL)
{
if (round)
{
if (tab[i].cp == &lcs_tab2)
{
lcs_tab1 = c1;
lcs_tab2 = c2;
lcs_tab3 = c3;
}
else if (tab[i].cp != NULL)
*(tab[i].cp) = c1;

}
p = s;
break;
}
}
}

if (i == entries)
return e_invarg;
if (*p == ',')
++p;
}
}

return NULL; 
}

