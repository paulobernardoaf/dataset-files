














#include "vim.h"

#if defined(FEAT_SYN_HL)



static int
advance_color_col(int vcol, int **color_cols)
{
while (**color_cols >= 0 && vcol > **color_cols)
++*color_cols;
return (**color_cols >= 0);
}
#endif

#if defined(FEAT_SYN_HL)




static void
margin_columns_win(win_T *wp, int *left_col, int *right_col)
{

static int saved_w_virtcol;
static win_T *prev_wp;
static int prev_left_col;
static int prev_right_col;
static int prev_col_off;

int cur_col_off = win_col_off(wp);
int width1;
int width2;

if (saved_w_virtcol == wp->w_virtcol
&& prev_wp == wp && prev_col_off == cur_col_off)
{
*right_col = prev_right_col;
*left_col = prev_left_col;
return;
}

width1 = wp->w_width - cur_col_off;
width2 = width1 + win_col_off2(wp);

*left_col = 0;
*right_col = width1;

if (wp->w_virtcol >= (colnr_T)width1)
*right_col = width1 + ((wp->w_virtcol - width1) / width2 + 1) * width2;
if (wp->w_virtcol >= (colnr_T)width1 && width2 > 0)
*left_col = (wp->w_virtcol - width1) / width2 * width2 + width1;


prev_left_col = *left_col;
prev_right_col = *right_col;
prev_wp = wp;
saved_w_virtcol = wp->w_virtcol;
prev_col_off = cur_col_off;
}
#endif

#if defined(FEAT_SIGNS)





static void
get_sign_display_info(
int nrcol,
win_T *wp,
linenr_T lnum UNUSED,
sign_attrs_T *sattr,
int wcr_attr,
int row,
int startrow,
int filler_lines UNUSED,
int filler_todo UNUSED,
int *c_extrap,
int *c_finalp,
char_u *extra,
char_u **pp_extra,
int *n_extrap,
int *char_attrp)
{
int text_sign;
#if defined(FEAT_SIGN_ICONS)
int icon_sign;
#endif


*c_extrap = ' ';
*c_finalp = NUL;
if (nrcol)
*n_extrap = number_width(wp) + 1;
else
{
*char_attrp = hl_combine_attr(wcr_attr, HL_ATTR(HLF_SC));
*n_extrap = 2;
}

if (row == startrow
#if defined(FEAT_DIFF)
+ filler_lines && filler_todo <= 0
#endif
)
{
text_sign = (sattr->sat_text != NULL) ? sattr->sat_typenr : 0;
#if defined(FEAT_SIGN_ICONS)
icon_sign = (sattr->sat_icon != NULL) ? sattr->sat_typenr : 0;
if (gui.in_use && icon_sign != 0)
{

if (nrcol)
{
*c_extrap = NUL;
sprintf((char *)extra, "%-*c ", number_width(wp), SIGN_BYTE);
*pp_extra = extra;
*n_extrap = (int)STRLEN(*pp_extra);
}
else
*c_extrap = SIGN_BYTE;
#if defined(FEAT_NETBEANS_INTG)
if (netbeans_active() && (buf_signcount(wp->w_buffer, lnum) > 1))
{
if (nrcol)
{
*c_extrap = NUL;
sprintf((char *)extra, "%-*c ", number_width(wp),
MULTISIGN_BYTE);
*pp_extra = extra;
*n_extrap = (int)STRLEN(*pp_extra);
}
else
*c_extrap = MULTISIGN_BYTE;
}
#endif
*c_finalp = NUL;
*char_attrp = icon_sign;
}
else
#endif
if (text_sign != 0)
{
*pp_extra = sattr->sat_text;
if (*pp_extra != NULL)
{
if (nrcol)
{
int n, width = number_width(wp) - 2;

for (n = 0; n < width; n++)
extra[n] = ' ';
extra[n] = 0;
STRCAT(extra, *pp_extra);
STRCAT(extra, " ");
*pp_extra = extra;
}
*c_extrap = NUL;
*c_finalp = NUL;
*n_extrap = (int)STRLEN(*pp_extra);
}
*char_attrp = sattr->sat_texthl;
}
}
}
#endif

#if defined(FEAT_PROP_POPUP)
static textprop_T *current_text_props = NULL;
static buf_T *current_buf = NULL;

static int
text_prop_compare(const void *s1, const void *s2)
{
int idx1, idx2;
proptype_T *pt1, *pt2;
colnr_T col1, col2;

idx1 = *(int *)s1;
idx2 = *(int *)s2;
pt1 = text_prop_type_by_id(current_buf, current_text_props[idx1].tp_type);
pt2 = text_prop_type_by_id(current_buf, current_text_props[idx2].tp_type);
if (pt1 == pt2)
return 0;
if (pt1 == NULL)
return -1;
if (pt2 == NULL)
return 1;
if (pt1->pt_priority != pt2->pt_priority)
return pt1->pt_priority > pt2->pt_priority ? 1 : -1;
col1 = current_text_props[idx1].tp_col;
col2 = current_text_props[idx2].tp_col;
return col1 == col2 ? 0 : col1 > col2 ? 1 : -1;
}
#endif








int
win_line(
win_T *wp,
linenr_T lnum,
int startrow,
int endrow,
int nochange UNUSED, 
int number_only) 
{
int col = 0; 
unsigned off; 
int c = 0; 
long vcol = 0; 
#if defined(FEAT_LINEBREAK)
long vcol_sbr = -1; 
#endif
long vcol_prev = -1; 
char_u *line; 
char_u *ptr; 
int row; 
int screen_row; 

char_u extra[21]; 
int n_extra = 0; 
char_u *p_extra = NULL; 
char_u *p_extra_free = NULL; 
int c_extra = NUL; 
int c_final = NUL; 
int extra_attr = 0; 
static char_u *at_end_str = (char_u *)""; 

int lcs_eol_one = lcs_eol; 
int lcs_prec_todo = lcs_prec; 


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
int noinvcur = FALSE; 
int lnum_in_visual_area = FALSE;
pos_T pos;
long v;

int char_attr = 0; 
int attr_pri = FALSE; 
int area_highlighting = FALSE; 

int vi_attr = 0; 

int wcr_attr = 0; 
int win_attr = 0; 

int area_attr = 0; 
int search_attr = 0; 
#if defined(FEAT_SYN_HL)
int vcol_save_attr = 0; 
int syntax_attr = 0; 
int prev_syntax_col = -1; 
int prev_syntax_attr = 0; 
int has_syntax = FALSE; 
int save_did_emsg;
int draw_color_col = FALSE; 
int *color_cols = NULL; 
#endif
int eol_hl_off = 0; 
#if defined(FEAT_PROP_POPUP)
int text_prop_count;
int text_prop_next = 0; 
textprop_T *text_props = NULL;
int *text_prop_idxs = NULL;
int text_props_active = 0;
proptype_T *text_prop_type = NULL;
int text_prop_attr = 0;
int text_prop_combine = FALSE;
#endif
#if defined(FEAT_SPELL)
int has_spell = FALSE; 
int can_spell;
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
#endif
int extra_check = 0; 
int multi_attr = 0; 
int mb_l = 1; 
int mb_c = 0; 
int mb_utf8 = FALSE; 
int u8cc[MAX_MCO]; 
#if defined(FEAT_DIFF) || defined(FEAT_SIGNS)
int filler_lines = 0; 
int filler_todo = 0; 
#endif
#if defined(FEAT_DIFF)
hlf_T diff_hlf = (hlf_T)0; 
int change_start = MAXCOL; 
int change_end = -1; 
#endif
colnr_T trailcol = MAXCOL; 
#if defined(FEAT_LINEBREAK)
int need_showbreak = FALSE; 

#endif
#if defined(FEAT_SIGNS) || defined(FEAT_QUICKFIX) || defined(FEAT_SYN_HL) || defined(FEAT_DIFF)

#define LINE_ATTR
int line_attr = 0; 
int line_attr_save;
#endif
#if defined(FEAT_SIGNS)
int sign_present = FALSE;
sign_attrs_T sattr;
#endif
#if defined(FEAT_ARABIC)
int prev_c = 0; 
int prev_c1 = 0; 
#endif
#if defined(LINE_ATTR)
int did_line_attr = 0;
#endif
#if defined(FEAT_TERMINAL)
int get_term_attr = FALSE;
#endif
#if defined(FEAT_SYN_HL)
int cul_attr = 0; 


int cul_screenline = FALSE;



int left_curline_col = 0;
int right_curline_col = 0;
#endif


#define WL_START 0 
#if defined(FEAT_CMDWIN)
#define WL_CMDLINE WL_START + 1 
#else
#define WL_CMDLINE WL_START
#endif
#if defined(FEAT_FOLDING)
#define WL_FOLD WL_CMDLINE + 1 
#else
#define WL_FOLD WL_CMDLINE
#endif
#if defined(FEAT_SIGNS)
#define WL_SIGN WL_FOLD + 1 
#else
#define WL_SIGN WL_FOLD 
#endif
#define WL_NR WL_SIGN + 1 
#if defined(FEAT_LINEBREAK)
#define WL_BRI WL_NR + 1 
#else
#define WL_BRI WL_NR
#endif
#if defined(FEAT_LINEBREAK) || defined(FEAT_DIFF)
#define WL_SBR WL_BRI + 1 
#else
#define WL_SBR WL_BRI
#endif
#define WL_LINE WL_SBR + 1 
int draw_state = WL_START; 
#if defined(FEAT_XIM) && defined(FEAT_GUI_GTK)
int feedback_col = 0;
int feedback_old_attr = -1;
#endif
int screen_line_flags = 0;

#if defined(FEAT_CONCEAL) || defined(FEAT_SEARCH_EXTRA)
int match_conc = 0; 
#endif
#if defined(FEAT_CONCEAL)
int syntax_flags = 0;
int syntax_seqnr = 0;
int prev_syntax_id = 0;
int conceal_attr = HL_ATTR(HLF_CONCEAL);
int is_concealing = FALSE;
int boguscols = 0; 

int vcol_off = 0; 
int did_wcol = FALSE;
int old_boguscols = 0;
#define VCOL_HLC (vcol - vcol_off)
#define FIX_FOR_BOGUSCOLS { n_extra += vcol_off; vcol -= vcol_off; vcol_off = 0; col -= boguscols; old_boguscols = boguscols; boguscols = 0; }








#else
#define VCOL_HLC (vcol)
#endif

if (startrow > endrow) 
return startrow;

row = startrow;
screen_row = row + W_WINROW(wp);

if (!number_only)
{


#if defined(FEAT_LINEBREAK)
extra_check = wp->w_p_lbr;
#endif
#if defined(FEAT_SYN_HL)
if (syntax_present(wp) && !wp->w_s->b_syn_error
#if defined(SYN_TIME_LIMIT)
&& !wp->w_s->b_syn_slow
#endif
)
{


save_did_emsg = did_emsg;
did_emsg = FALSE;
syntax_start(wp, lnum);
if (did_emsg)
wp->w_s->b_syn_error = TRUE;
else
{
did_emsg = save_did_emsg;
#if defined(SYN_TIME_LIMIT)
if (!wp->w_s->b_syn_slow)
#endif
{
has_syntax = TRUE;
extra_check = TRUE;
}
}
}


color_cols = wp->w_p_cc_cols;
if (color_cols != NULL)
draw_color_col = advance_color_col(VCOL_HLC, &color_cols);
#endif

#if defined(FEAT_TERMINAL)
if (term_show_buffer(wp->w_buffer))
{
extra_check = TRUE;
get_term_attr = TRUE;
win_attr = term_get_attr(wp, lnum, -1);
}
#endif

#if defined(FEAT_SPELL)
if (wp->w_p_spell
&& *wp->w_s->b_p_spl != NUL
&& wp->w_s->b_langp.ga_len > 0
&& *(char **)(wp->w_s->b_langp.ga_data) != NULL)
{

has_spell = TRUE;
extra_check = TRUE;




nextline[SPWORDLEN] = NUL;
if (lnum < wp->w_buffer->b_ml.ml_line_count)
{
line = ml_get_buf(wp->w_buffer, lnum + 1, FALSE);
spell_cat_line(nextline + SPWORDLEN, line, SPWORDLEN);
}



if (lnum == checked_lnum)
cur_checked_col = checked_col;
checked_lnum = 0;




if (lnum != capcol_lnum)
cap_col = -1;
if (lnum == 1)
cap_col = 0;
capcol_lnum = 0;
}
#endif


if (VIsual_active && wp->w_buffer == curwin->w_buffer)
{
pos_T *top, *bot;

if (LTOREQ_POS(curwin->w_cursor, VIsual))
{

top = &curwin->w_cursor;
bot = &VIsual;
}
else
{

top = &VIsual;
bot = &curwin->w_cursor;
}
lnum_in_visual_area = (lnum >= top->lnum && lnum <= bot->lnum);
if (VIsual_mode == Ctrl_V)
{

if (lnum_in_visual_area)
{
fromcol = wp->w_old_cursor_fcol;
tocol = wp->w_old_cursor_lcol;
}
}
else
{

if (lnum > top->lnum && lnum <= bot->lnum)
fromcol = 0;
else if (lnum == top->lnum)
{
if (VIsual_mode == 'V') 
fromcol = 0;
else
{
getvvcol(wp, top, (colnr_T *)&fromcol, NULL, NULL);
if (gchar_pos(top) == NUL)
tocol = fromcol + 1;
}
}
if (VIsual_mode != 'V' && lnum == bot->lnum)
{
if (*p_sel == 'e' && bot->col == 0 && bot->coladd == 0)
{
fromcol = -10;
tocol = MAXCOL;
}
else if (bot->col == MAXCOL)
tocol = MAXCOL;
else
{
pos = *bot;
if (*p_sel == 'e')
getvvcol(wp, &pos, (colnr_T *)&tocol, NULL, NULL);
else
{
getvvcol(wp, &pos, NULL, NULL, (colnr_T *)&tocol);
++tocol;
}
}
}
}


if (!highlight_match && lnum == curwin->w_cursor.lnum
&& wp == curwin
#if defined(FEAT_GUI)
&& !gui.in_use
#endif
)
noinvcur = TRUE;


if (fromcol >= 0)
{
area_highlighting = TRUE;
vi_attr = HL_ATTR(HLF_V);
#if defined(FEAT_CLIPBOARD) && defined(FEAT_X11)
if ((clip_star.available && !clip_star.owned
&& clip_isautosel_star())
|| (clip_plus.available && !clip_plus.owned
&& clip_isautosel_plus()))
vi_attr = HL_ATTR(HLF_VNC);
#endif
}
}


else if (highlight_match
&& wp == curwin
&& lnum >= curwin->w_cursor.lnum
&& lnum <= curwin->w_cursor.lnum + search_match_lines)
{
if (lnum == curwin->w_cursor.lnum)
getvcol(curwin, &(curwin->w_cursor),
(colnr_T *)&fromcol, NULL, NULL);
else
fromcol = 0;
if (lnum == curwin->w_cursor.lnum + search_match_lines)
{
pos.lnum = lnum;
pos.col = search_match_endcol;
getvcol(curwin, &pos, (colnr_T *)&tocol, NULL, NULL);
}
else
tocol = MAXCOL;

if (fromcol == tocol)
tocol = fromcol + 1;
area_highlighting = TRUE;
vi_attr = HL_ATTR(HLF_I);
}
}

#if defined(FEAT_DIFF)
filler_lines = diff_check(wp, lnum);
if (filler_lines < 0)
{
if (filler_lines == -1)
{
if (diff_find_change(wp, lnum, &change_start, &change_end))
diff_hlf = HLF_ADD; 
else if (change_start == 0)
diff_hlf = HLF_TXD; 
else
diff_hlf = HLF_CHD; 
}
else
diff_hlf = HLF_ADD; 
filler_lines = 0;
area_highlighting = TRUE;
}
if (lnum == wp->w_topline)
filler_lines = wp->w_topfill;
filler_todo = filler_lines;
#endif

#if defined(FEAT_SIGNS)
sign_present = buf_get_signattrs(wp, lnum, &sattr);
#endif

#if defined(LINE_ATTR)
#if defined(FEAT_SIGNS)

if (sign_present)
line_attr = sattr.sat_linehl;
#endif
#if defined(FEAT_QUICKFIX)

if (bt_quickfix(wp->w_buffer) && qf_current_entry(wp) == lnum)
line_attr = HL_ATTR(HLF_QFL);
#endif
if (line_attr != 0)
area_highlighting = TRUE;
#endif

line = ml_get_buf(wp->w_buffer, lnum, FALSE);
ptr = line;

#if defined(FEAT_SPELL)
if (has_spell && !number_only)
{

if (cap_col == 0)
cap_col = getwhitecols(line);




if (nextline[SPWORDLEN] == NUL)
{

nextlinecol = MAXCOL;
nextline_idx = 0;
}
else
{
v = (long)STRLEN(line);
if (v < SPWORDLEN)
{


nextlinecol = 0;
mch_memmove(nextline, line, (size_t)v);
STRMOVE(nextline + v, nextline + SPWORDLEN);
nextline_idx = v + 1;
}
else
{

nextlinecol = v - SPWORDLEN;
mch_memmove(nextline, line + nextlinecol, SPWORDLEN);
nextline_idx = SPWORDLEN + 1;
}
}
}
#endif

if (wp->w_p_list)
{
if (lcs_space || lcs_trail || lcs_nbsp)
extra_check = TRUE;

if (lcs_trail)
{
trailcol = (colnr_T)STRLEN(ptr);
while (trailcol > (colnr_T)0 && VIM_ISWHITE(ptr[trailcol - 1]))
--trailcol;
trailcol += (colnr_T) (ptr - line);
}
}

wcr_attr = get_wcr_attr(wp);
if (wcr_attr != 0)
{
win_attr = wcr_attr;
area_highlighting = TRUE;
}

#if defined(FEAT_PROP_POPUP)
if (WIN_IS_POPUP(wp))
screen_line_flags |= SLF_POPUP;
#endif



if (wp->w_p_wrap)
v = wp->w_skipcol;
else
v = wp->w_leftcol;
if (v > 0 && !number_only)
{
char_u *prev_ptr = ptr;

while (vcol < v && *ptr != NUL)
{
c = win_lbr_chartabsize(wp, line, ptr, (colnr_T)vcol, NULL);
vcol += c;
prev_ptr = ptr;
MB_PTR_ADV(ptr);
}







if (vcol < v && (
#if defined(FEAT_SYN_HL)
wp->w_p_cuc || draw_color_col ||
#endif
virtual_active() ||
(VIsual_active && wp->w_buffer == curwin->w_buffer)))
vcol = v;



if (vcol > v)
{
vcol -= c;
ptr = prev_ptr;


if (( (*mb_ptr2cells)(ptr) >= c || *ptr == TAB) && col == 0)
n_skip = v - vcol;
}



if (tocol <= vcol)
fromcol = 0;
else if (fromcol >= 0 && fromcol < vcol)
fromcol = vcol;

#if defined(FEAT_LINEBREAK)

if (wp->w_p_wrap)
need_showbreak = TRUE;
#endif
#if defined(FEAT_SPELL)


if (has_spell)
{
int len;
colnr_T linecol = (colnr_T)(ptr - line);
hlf_T spell_hlf = HLF_COUNT;

pos = wp->w_cursor;
wp->w_cursor.lnum = lnum;
wp->w_cursor.col = linecol;
len = spell_move_to(wp, FORWARD, TRUE, TRUE, &spell_hlf);


line = ml_get_buf(wp->w_buffer, lnum, FALSE);
ptr = line + linecol;

if (len == 0 || (int)wp->w_cursor.col > ptr - line)
{


spell_hlf = HLF_COUNT;
word_end = (int)(spell_to_word_end(ptr, wp) - line + 1);
}
else
{

word_end = wp->w_cursor.col + len + 1;


if (spell_hlf != HLF_COUNT)
spell_attr = highlight_attr[spell_hlf];
}
wp->w_cursor = pos;

#if defined(FEAT_SYN_HL)

if (has_syntax)
syntax_start(wp, lnum);
#endif
}
#endif
}



if (fromcol >= 0)
{
if (noinvcur)
{
if ((colnr_T)fromcol == wp->w_virtcol)
{


fromcol_prev = fromcol;
fromcol = -1;
}
else if ((colnr_T)fromcol < wp->w_virtcol)

fromcol_prev = wp->w_virtcol;
}
if (fromcol >= tocol)
fromcol = -1;
}

#if defined(FEAT_SEARCH_EXTRA)
if (!number_only)
{
v = (long)(ptr - line);
area_highlighting |= prepare_search_hl_line(wp, lnum, (colnr_T)v,
&line, &screen_search_hl,
&search_attr);
ptr = line + v; 
}
#endif

#if defined(FEAT_SYN_HL)

if (wp->w_p_cul && lnum == wp->w_cursor.lnum)
{



if (!(wp == curwin && VIsual_active)
&& wp->w_p_culopt_flags != CULOPT_NBR)
{
cul_screenline = (wp->w_p_wrap
&& (wp->w_p_culopt_flags & CULOPT_SCRLINE));



if (!cul_screenline)
{
cul_attr = HL_ATTR(HLF_CUL);
line_attr = cul_attr;
wp->w_last_cursorline = wp->w_cursor.lnum;
}
else
{
line_attr_save = line_attr;
wp->w_last_cursorline = 0;
margin_columns_win(wp, &left_curline_col, &right_curline_col);
}
area_highlighting = TRUE;
}
else
wp->w_last_cursorline = wp->w_cursor.lnum;
}
#endif

#if defined(FEAT_PROP_POPUP)
{
char_u *prop_start;

text_prop_count = get_text_props(wp->w_buffer, lnum,
&prop_start, FALSE);
if (text_prop_count > 0)
{


text_props = ALLOC_MULT(textprop_T, text_prop_count);
if (text_props != NULL)
mch_memmove(text_props, prop_start,
text_prop_count * sizeof(textprop_T));


text_prop_idxs = ALLOC_MULT(int, text_prop_count);
area_highlighting = TRUE;
extra_check = TRUE;
}
}
#endif

off = (unsigned)(current_ScreenLine - ScreenLines);
col = 0;

#if defined(FEAT_RIGHTLEFT)
if (wp->w_p_rl)
{



col = wp->w_width - 1;
off += col;
screen_line_flags |= SLF_RIGHTLEFT;
}
#endif


for (;;)
{
#if defined(FEAT_CONCEAL) || defined(FEAT_SEARCH_EXTRA)
int has_match_conc = 0; 
#endif
#if defined(FEAT_CONCEAL)
int did_decrement_ptr = FALSE;
#endif

if (draw_state != WL_LINE)
{
#if defined(FEAT_CMDWIN)
if (draw_state == WL_CMDLINE - 1 && n_extra == 0)
{
draw_state = WL_CMDLINE;
if (cmdwin_type != 0 && wp == curwin)
{

n_extra = 1;
c_extra = cmdwin_type;
c_final = NUL;
char_attr = hl_combine_attr(wcr_attr, HL_ATTR(HLF_AT));
}
}
#endif

#if defined(FEAT_FOLDING)
if (draw_state == WL_FOLD - 1 && n_extra == 0)
{
int fdc = compute_foldcolumn(wp, 0);

draw_state = WL_FOLD;
if (fdc > 0)
{


vim_free(p_extra_free);
p_extra_free = alloc(12 + 1);

if (p_extra_free != NULL)
{
fill_foldcolumn(p_extra_free, wp, FALSE, lnum);
n_extra = fdc;
p_extra_free[n_extra] = NUL;
p_extra = p_extra_free;
c_extra = NUL;
c_final = NUL;
char_attr = hl_combine_attr(wcr_attr, HL_ATTR(HLF_FC));
}
}
}
#endif

#if defined(FEAT_SIGNS)
if (draw_state == WL_SIGN - 1 && n_extra == 0)
{
draw_state = WL_SIGN;


if (signcolumn_on(wp))
get_sign_display_info(FALSE, wp, lnum, &sattr, wcr_attr,
row, startrow, filler_lines, filler_todo, &c_extra,
&c_final, extra, &p_extra, &n_extra, &char_attr);
}
#endif

if (draw_state == WL_NR - 1 && n_extra == 0)
{
draw_state = WL_NR;


if ((wp->w_p_nu || wp->w_p_rnu)
&& (row == startrow
#if defined(FEAT_DIFF)
+ filler_lines
#endif
|| vim_strchr(p_cpo, CPO_NUMCOL) == NULL))
{
#if defined(FEAT_SIGNS)



if ((*wp->w_p_scl == 'n' && *(wp->w_p_scl + 1) == 'u')
&& sign_present)
get_sign_display_info(TRUE, wp, lnum, &sattr, wcr_attr,
row, startrow, filler_lines, filler_todo,
&c_extra, &c_final, extra, &p_extra, &n_extra,
&char_attr);
else
#endif
{

if (row == startrow
#if defined(FEAT_DIFF)
+ filler_lines
#endif
)
{
long num;
char *fmt = "%*ld ";

if (wp->w_p_nu && !wp->w_p_rnu)

num = (long)lnum;
else
{

num = labs((long)get_cursor_rel_lnum(wp, lnum));
if (num == 0 && wp->w_p_nu && wp->w_p_rnu)
{

num = lnum;
fmt = "%-*ld ";
}
}

sprintf((char *)extra, fmt,
number_width(wp), num);
if (wp->w_skipcol > 0)
for (p_extra = extra; *p_extra == ' '; ++p_extra)
*p_extra = '-';
#if defined(FEAT_RIGHTLEFT)
if (wp->w_p_rl) 
{
char_u *p1, *p2;
int t;


p2 = skiptowhite(extra) - 1;
for (p1 = extra; p1 < p2; ++p1, --p2)
{
t = *p1;
*p1 = *p2;
*p2 = t;
}
}
#endif
p_extra = extra;
c_extra = NUL;
c_final = NUL;
}
else
{
c_extra = ' ';
c_final = NUL;
}
n_extra = number_width(wp) + 1;
char_attr = hl_combine_attr(wcr_attr, HL_ATTR(HLF_N));
#if defined(FEAT_SYN_HL)






if (wp->w_p_cul
&& lnum == wp->w_cursor.lnum
&& (wp->w_p_culopt_flags & CULOPT_NBR)
&& (row == startrow
|| wp->w_p_culopt_flags & CULOPT_LINE))
char_attr = hl_combine_attr(wcr_attr, HL_ATTR(HLF_CLN));
#endif
if (wp->w_p_rnu && lnum < wp->w_cursor.lnum
&& HL_ATTR(HLF_LNA) != 0)

char_attr = hl_combine_attr(wcr_attr,
HL_ATTR(HLF_LNA));
if (wp->w_p_rnu && lnum > wp->w_cursor.lnum
&& HL_ATTR(HLF_LNB) != 0)

char_attr = hl_combine_attr(wcr_attr,
HL_ATTR(HLF_LNB));
}
}
}

#if defined(FEAT_LINEBREAK)
if (wp->w_briopt_sbr && draw_state == WL_BRI - 1
&& n_extra == 0 && *get_showbreak_value(wp) != NUL)

draw_state = WL_BRI;
else if (wp->w_briopt_sbr && draw_state == WL_SBR && n_extra == 0)

draw_state = WL_BRI - 1;


if (draw_state == WL_BRI - 1 && n_extra == 0)
{
draw_state = WL_BRI;

if (wp->w_p_bri && n_extra == 0
&& (row != startrow || need_showbreak)
#if defined(FEAT_DIFF)
&& filler_lines == 0
#endif
)
{
char_attr = 0;
#if defined(FEAT_DIFF)
if (diff_hlf != (hlf_T)0)
{
char_attr = HL_ATTR(diff_hlf);
#if defined(FEAT_SYN_HL)
if (cul_attr != 0)
char_attr = hl_combine_attr(char_attr, cul_attr);
#endif
}
#endif
p_extra = NULL;
c_extra = ' ';
c_final = NUL;
n_extra = get_breakindent_win(wp,
ml_get_buf(wp->w_buffer, lnum, FALSE));
if (wp->w_skipcol > 0 && wp->w_p_wrap && wp->w_briopt_sbr)
need_showbreak = FALSE;


if (tocol == vcol)
tocol += n_extra;
}
}
#endif

#if defined(FEAT_LINEBREAK) || defined(FEAT_DIFF)
if (draw_state == WL_SBR - 1 && n_extra == 0)
{
char_u *sbr;

draw_state = WL_SBR;
#if defined(FEAT_DIFF)
if (filler_todo > 0)
{

if (char2cells(fill_diff) > 1)
{
c_extra = '-';
c_final = NUL;
}
else
{
c_extra = fill_diff;
c_final = NUL;
}
#if defined(FEAT_RIGHTLEFT)
if (wp->w_p_rl)
n_extra = col + 1;
else
#endif
n_extra = wp->w_width - col;
char_attr = HL_ATTR(HLF_DED);
}
#endif
#if defined(FEAT_LINEBREAK)
sbr = get_showbreak_value(wp);
if (*sbr != NUL && need_showbreak)
{

p_extra = sbr;
c_extra = NUL;
c_final = NUL;
n_extra = (int)STRLEN(sbr);
if (wp->w_skipcol == 0 || !wp->w_p_wrap)
need_showbreak = FALSE;
vcol_sbr = vcol + MB_CHARLEN(sbr);


if (tocol == vcol)
tocol += n_extra;

char_attr = hl_combine_attr(win_attr, HL_ATTR(HLF_AT));
#if defined(FEAT_SYN_HL)

if (cul_attr != 0)
char_attr = hl_combine_attr(char_attr, cul_attr);
#endif
}
#endif
}
#endif

if (draw_state == WL_LINE - 1 && n_extra == 0)
{
draw_state = WL_LINE;
if (saved_n_extra)
{

n_extra = saved_n_extra;
c_extra = saved_c_extra;
c_final = saved_c_final;
p_extra = saved_p_extra;
char_attr = saved_char_attr;
}
else
char_attr = win_attr;
}
}
#if defined(FEAT_SYN_HL)
if (cul_screenline)
{
if (draw_state == WL_LINE
&& vcol >= left_curline_col
&& vcol < right_curline_col)
{
cul_attr = HL_ATTR(HLF_CUL);
line_attr = cul_attr;
}
else
{
cul_attr = 0;
line_attr = line_attr_save;
}
}
#endif




if ((dollar_vcol >= 0 && wp == curwin
&& lnum == wp->w_cursor.lnum && vcol >= (long)wp->w_virtcol
#if defined(FEAT_DIFF)
&& filler_todo <= 0
#endif
)
|| (number_only && draw_state > WL_NR))
{
screen_line(screen_row, wp->w_wincol, col, -(int)wp->w_width,
screen_line_flags);


#if defined(FEAT_SYN_HL)
if (wp->w_p_cuc)
row = wp->w_cline_row + wp->w_cline_height;
else
#endif
row = wp->w_height;
break;
}

if (draw_state == WL_LINE && (area_highlighting || extra_check))
{

if (vcol == fromcol
|| (has_mbyte && vcol + 1 == fromcol && n_extra == 0
&& (*mb_ptr2cells)(ptr) > 1)
|| ((int)vcol_prev == fromcol_prev
&& vcol_prev < vcol 
&& vcol < tocol))
area_attr = vi_attr; 
else if (area_attr != 0
&& (vcol == tocol
|| (noinvcur && (colnr_T)vcol == wp->w_virtcol)))
area_attr = 0; 

#if defined(FEAT_SEARCH_EXTRA)
if (!n_extra)
{



v = (long)(ptr - line);
search_attr = update_search_hl(wp, lnum, (colnr_T)v, &line,
&screen_search_hl, &has_match_conc,
&match_conc, did_line_attr, lcs_eol_one);
ptr = line + v; 
}
#endif

#if defined(FEAT_DIFF)
if (diff_hlf != (hlf_T)0)
{
if (diff_hlf == HLF_CHD && ptr - line >= change_start
&& n_extra == 0)
diff_hlf = HLF_TXD; 
if (diff_hlf == HLF_TXD && ptr - line > change_end
&& n_extra == 0)
diff_hlf = HLF_CHD; 
line_attr = HL_ATTR(diff_hlf);
if (wp->w_p_cul && lnum == wp->w_cursor.lnum
&& wp->w_p_culopt_flags != CULOPT_NBR
&& (!cul_screenline || (vcol >= left_curline_col
&& vcol <= right_curline_col)))
line_attr = hl_combine_attr(
line_attr, HL_ATTR(HLF_CUL));
}
#endif

#if defined(FEAT_PROP_POPUP)
if (text_props != NULL)
{
int pi;
int bcol = (int)(ptr - line);

if (n_extra > 0)
--bcol; 


for (pi = 0; pi < text_props_active; ++pi)
{
int tpi = text_prop_idxs[pi];

if (bcol >= text_props[tpi].tp_col - 1
+ text_props[tpi].tp_len)
{
if (pi + 1 < text_props_active)
mch_memmove(text_prop_idxs + pi,
text_prop_idxs + pi + 1,
sizeof(int)
* (text_props_active - (pi + 1)));
--text_props_active;
--pi;
}
}


while (text_prop_next < text_prop_count
&& bcol >= text_props[text_prop_next].tp_col - 1)
text_prop_idxs[text_props_active++] = text_prop_next++;

text_prop_attr = 0;
text_prop_combine = FALSE;
text_prop_type = NULL;
if (text_props_active > 0)
{


current_text_props = text_props;
current_buf = wp->w_buffer;
qsort((void *)text_prop_idxs, (size_t)text_props_active,
sizeof(int), text_prop_compare);

for (pi = 0; pi < text_props_active; ++pi)
{
int tpi = text_prop_idxs[pi];
proptype_T *pt = text_prop_type_by_id(
wp->w_buffer, text_props[tpi].tp_type);

if (pt != NULL && pt->pt_hl_id > 0)
{
int pt_attr = syn_id2attr(pt->pt_hl_id);

text_prop_type = pt;
text_prop_attr =
hl_combine_attr(text_prop_attr, pt_attr);
text_prop_combine = pt->pt_flags & PT_FLAG_COMBINE;
}
}
}
}
#endif

#if defined(FEAT_SYN_HL)
if (extra_check && n_extra == 0)
{
syntax_attr = 0;
#if defined(FEAT_TERMINAL)
if (get_term_attr)
syntax_attr = term_get_attr(wp, lnum, vcol);
#endif

if (has_syntax)
{


save_did_emsg = did_emsg;
did_emsg = FALSE;

v = (long)(ptr - line);
if (v == prev_syntax_col)

syntax_attr = prev_syntax_attr;
else
{
#if defined(FEAT_SPELL)
can_spell = TRUE;
#endif
syntax_attr = get_syntax_attr((colnr_T)v,
#if defined(FEAT_SPELL)
has_spell ? &can_spell :
#endif
NULL, FALSE);
prev_syntax_col = v;
prev_syntax_attr = syntax_attr;
}

if (did_emsg)
{
wp->w_s->b_syn_error = TRUE;
has_syntax = FALSE;
syntax_attr = 0;
}
else
did_emsg = save_did_emsg;
#if defined(SYN_TIME_LIMIT)
if (wp->w_s->b_syn_slow)
has_syntax = FALSE;
#endif



line = ml_get_buf(wp->w_buffer, lnum, FALSE);
ptr = line + v;
#if defined(FEAT_CONCEAL)


if (*ptr == NUL)
syntax_flags = 0;
else
syntax_flags = get_syntax_info(&syntax_seqnr);
#endif
}
}
#if defined(FEAT_PROP_POPUP)

if (text_prop_type != NULL)
{
if (text_prop_combine)
syntax_attr = hl_combine_attr(syntax_attr, text_prop_attr);
else
syntax_attr = text_prop_attr;
}
#endif
#endif


attr_pri = TRUE;
#if defined(LINE_ATTR)
if (area_attr != 0)
{
char_attr = hl_combine_attr(line_attr, area_attr);
#if defined(FEAT_SYN_HL)
char_attr = hl_combine_attr(syntax_attr, char_attr);
#endif
}
else if (search_attr != 0)
{
char_attr = hl_combine_attr(line_attr, search_attr);
#if defined(FEAT_SYN_HL)
char_attr = hl_combine_attr(syntax_attr, char_attr);
#endif
}
else if (line_attr != 0 && ((fromcol == -10 && tocol == MAXCOL)
|| vcol < fromcol || vcol_prev < fromcol_prev
|| vcol >= tocol))
{


#if defined(FEAT_SYN_HL)
char_attr = hl_combine_attr(syntax_attr, line_attr);
#else
char_attr = line_attr;
#endif
attr_pri = FALSE;
}
#else
if (area_attr != 0)
char_attr = area_attr;
else if (search_attr != 0)
char_attr = search_attr;
#endif
else
{
attr_pri = FALSE;
#if defined(FEAT_SYN_HL)
char_attr = syntax_attr;
#else
char_attr = 0;
#endif
}
}


if (win_attr != 0)
{
if (char_attr == 0)
char_attr = win_attr;
else
char_attr = hl_combine_attr(win_attr, char_attr);
}










if (n_extra > 0)
{
if (c_extra != NUL || (n_extra == 1 && c_final != NUL))
{
c = (n_extra == 1 && c_final != NUL) ? c_final : c_extra;
mb_c = c; 
if (enc_utf8 && utf_char2len(c) > 1)
{
mb_utf8 = TRUE;
u8cc[0] = 0;
c = 0xc0;
}
else
mb_utf8 = FALSE;
}
else
{
c = *p_extra;
if (has_mbyte)
{
mb_c = c;
if (enc_utf8)
{


mb_l = utfc_ptr2len(p_extra);
mb_utf8 = FALSE;
if (mb_l > n_extra)
mb_l = 1;
else if (mb_l > 1)
{
mb_c = utfc_ptr2char(p_extra, u8cc);
mb_utf8 = TRUE;
c = 0xc0;
}
}
else
{

mb_l = MB_BYTE2LEN(c);
if (mb_l >= n_extra)
mb_l = 1;
else if (mb_l > 1)
mb_c = (c << 8) + p_extra[1];
}
if (mb_l == 0) 
mb_l = 1;



if ((
#if defined(FEAT_RIGHTLEFT)
wp->w_p_rl ? (col <= 0) :
#endif
(col >= wp->w_width - 1))
&& (*mb_char2cells)(mb_c) == 2)
{
c = '>';
mb_c = c;
mb_l = 1;
mb_utf8 = FALSE;
multi_attr = HL_ATTR(HLF_AT);
#if defined(FEAT_SYN_HL)
if (cul_attr)
multi_attr = hl_combine_attr(multi_attr, cul_attr);
#endif
multi_attr = hl_combine_attr(win_attr, multi_attr);



++n_extra;
--p_extra;
}
else
{
n_extra -= mb_l - 1;
p_extra += mb_l - 1;
}
}
++p_extra;
}
--n_extra;
}
else
{
#if defined(FEAT_LINEBREAK)
int c0;
#endif
VIM_CLEAR(p_extra_free);


c = *ptr;
#if defined(FEAT_LINEBREAK)
c0 = *ptr;
#endif
if (has_mbyte)
{
mb_c = c;
if (enc_utf8)
{


mb_l = utfc_ptr2len(ptr);
mb_utf8 = FALSE;
if (mb_l > 1)
{
mb_c = utfc_ptr2char(ptr, u8cc);


if (mb_c < 0x80)
{
c = mb_c;
#if defined(FEAT_LINEBREAK)
c0 = mb_c;
#endif
}
mb_utf8 = TRUE;



if (utf_iscomposing(mb_c))
{
int i;

for (i = Screen_mco - 1; i > 0; --i)
u8cc[i] = u8cc[i - 1];
u8cc[0] = mb_c;
mb_c = ' ';
}
}

if ((mb_l == 1 && c >= 0x80)
|| (mb_l >= 1 && mb_c == 0)
|| (mb_l > 1 && (!vim_isprintc(mb_c))))
{


transchar_hex(extra, mb_c);
#if defined(FEAT_RIGHTLEFT)
if (wp->w_p_rl) 
rl_mirror(extra);
#endif
p_extra = extra;
c = *p_extra;
mb_c = mb_ptr2char_adv(&p_extra);
mb_utf8 = (c >= 0x80);
n_extra = (int)STRLEN(p_extra);
c_extra = NUL;
c_final = NUL;
if (area_attr == 0 && search_attr == 0)
{
n_attr = n_extra + 1;
extra_attr = hl_combine_attr(
win_attr, HL_ATTR(HLF_8));
saved_attr2 = char_attr; 
}
}
else if (mb_l == 0) 
mb_l = 1;
#if defined(FEAT_ARABIC)
else if (p_arshape && !p_tbidi && ARABIC_CHAR(mb_c))
{

int pc, pc1, nc;
int pcc[MAX_MCO];



if (wp->w_p_rl)
{
pc = prev_c;
pc1 = prev_c1;
nc = utf_ptr2char(ptr + mb_l);
prev_c1 = u8cc[0];
}
else
{
pc = utfc_ptr2char(ptr + mb_l, pcc);
nc = prev_c;
pc1 = pcc[0];
}
prev_c = mb_c;

mb_c = arabic_shape(mb_c, &c, &u8cc[0], pc, pc1, nc);
}
else
prev_c = mb_c;
#endif
}
else 
{
mb_l = MB_BYTE2LEN(c);
if (mb_l == 0) 
mb_l = 1;
else if (mb_l > 1)
{


if (ptr[1] >= 32)
mb_c = (c << 8) + ptr[1];
else
{
if (ptr[1] == NUL)
{

mb_l = 1;
transchar_nonprint(extra, c);
}
else
{

mb_l = 2;
STRCPY(extra, "XX");
}
p_extra = extra;
n_extra = (int)STRLEN(extra) - 1;
c_extra = NUL;
c_final = NUL;
c = *p_extra++;
if (area_attr == 0 && search_attr == 0)
{
n_attr = n_extra + 1;
extra_attr = hl_combine_attr(
win_attr, HL_ATTR(HLF_8));
saved_attr2 = char_attr; 
}
mb_c = c;
}
}
}



if ((
#if defined(FEAT_RIGHTLEFT)
wp->w_p_rl ? (col <= 0) :
#endif
(col >= wp->w_width - 1))
&& (*mb_char2cells)(mb_c) == 2)
{
c = '>';
mb_c = c;
mb_utf8 = FALSE;
mb_l = 1;
multi_attr = hl_combine_attr(win_attr, HL_ATTR(HLF_AT));


--ptr;
#if defined(FEAT_CONCEAL)
did_decrement_ptr = TRUE;
#endif
}
else if (*ptr != NUL)
ptr += mb_l - 1;




if (n_skip > 0 && mb_l > 1 && n_extra == 0)
{
n_extra = 1;
c_extra = MB_FILLER_CHAR;
c_final = NUL;
c = ' ';
if (area_attr == 0 && search_attr == 0)
{
n_attr = n_extra + 1;
extra_attr = hl_combine_attr(win_attr, HL_ATTR(HLF_AT));
saved_attr2 = char_attr; 
}
mb_c = c;
mb_utf8 = FALSE;
mb_l = 1;
}

}
++ptr;

if (extra_check)
{
#if defined(FEAT_SPELL)




v = (long)(ptr - line);
if (has_spell && v >= word_end && v > cur_checked_col)
{
spell_attr = 0;
if (c != 0 && (
#if defined(FEAT_SYN_HL)
!has_syntax ||
#endif
can_spell))
{
char_u *prev_ptr, *p;
int len;
hlf_T spell_hlf = HLF_COUNT;
if (has_mbyte)
{
prev_ptr = ptr - mb_l;
v -= mb_l - 1;
}
else
prev_ptr = ptr - 1;



if ((prev_ptr - line) - nextlinecol >= 0)
p = nextline + (prev_ptr - line) - nextlinecol;
else
p = prev_ptr;
cap_col -= (int)(prev_ptr - line);
len = spell_check(wp, p, &spell_hlf, &cap_col,
nochange);
word_end = v + len;



if (spell_hlf != HLF_COUNT
&& (State & INSERT) != 0
&& wp->w_cursor.lnum == lnum
&& wp->w_cursor.col >=
(colnr_T)(prev_ptr - line)
&& wp->w_cursor.col < (colnr_T)word_end)
{
spell_hlf = HLF_COUNT;
spell_redraw_lnum = lnum;
}

if (spell_hlf == HLF_COUNT && p != prev_ptr
&& (p - nextline) + len > nextline_idx)
{


checked_lnum = lnum + 1;
checked_col = (int)((p - nextline)
+ len - nextline_idx);
}


if (spell_hlf != HLF_COUNT)
spell_attr = highlight_attr[spell_hlf];

if (cap_col > 0)
{
if (p != prev_ptr
&& (p - nextline) + cap_col >= nextline_idx)
{


capcol_lnum = lnum + 1;
cap_col = (int)((p - nextline) + cap_col
- nextline_idx);
}
else

cap_col += (int)(prev_ptr - line);
}
}
}
if (spell_attr != 0)
{
if (!attr_pri)
char_attr = hl_combine_attr(char_attr, spell_attr);
else
char_attr = hl_combine_attr(spell_attr, char_attr);
}
#endif
#if defined(FEAT_LINEBREAK)

if (wp->w_p_lbr && c0 == c
&& VIM_ISBREAK(c) && !VIM_ISBREAK((int)*ptr))
{
int mb_off = has_mbyte ? (*mb_head_off)(line, ptr - 1) : 0;
char_u *p = ptr - (mb_off + 1);


n_extra = win_lbr_chartabsize(wp, line, p, (colnr_T)vcol,
NULL) - 1;
if (c == TAB && n_extra + col > wp->w_width)
#if defined(FEAT_VARTABS)
n_extra = tabstop_padding(vcol, wp->w_buffer->b_p_ts,
wp->w_buffer->b_p_vts_array) - 1;
#else
n_extra = (int)wp->w_buffer->b_p_ts
- vcol % (int)wp->w_buffer->b_p_ts - 1;
#endif

c_extra = mb_off > 0 ? MB_FILLER_CHAR : ' ';
c_final = NUL;
if (VIM_ISWHITE(c))
{
#if defined(FEAT_CONCEAL)
if (c == TAB)

FIX_FOR_BOGUSCOLS;
#endif
if (!wp->w_p_list)
c = ' ';
}
}
#endif




if (wp->w_p_list
&& ((((c == 160 && mb_l == 1)
|| (mb_utf8
&& ((mb_c == 160 && mb_l == 2)
|| (mb_c == 0x202f && mb_l == 3))))
&& lcs_nbsp)
|| (c == ' '
&& mb_l == 1
&& lcs_space
&& ptr - line <= trailcol)))
{
c = (c == ' ') ? lcs_space : lcs_nbsp;
if (area_attr == 0 && search_attr == 0)
{
n_attr = 1;
extra_attr = hl_combine_attr(win_attr, HL_ATTR(HLF_8));
saved_attr2 = char_attr; 
}
mb_c = c;
if (enc_utf8 && utf_char2len(c) > 1)
{
mb_utf8 = TRUE;
u8cc[0] = 0;
c = 0xc0;
}
else
mb_utf8 = FALSE;
}

if (trailcol != MAXCOL && ptr > line + trailcol && c == ' ')
{
c = lcs_trail;
if (!attr_pri)
{
n_attr = 1;
extra_attr = hl_combine_attr(win_attr, HL_ATTR(HLF_8));
saved_attr2 = char_attr; 
}
mb_c = c;
if (enc_utf8 && utf_char2len(c) > 1)
{
mb_utf8 = TRUE;
u8cc[0] = 0;
c = 0xc0;
}
else
mb_utf8 = FALSE;
}
}


if (!vim_isprintc(c))
{



if (c == TAB && (!wp->w_p_list || lcs_tab1))
{
int tab_len = 0;
long vcol_adjusted = vcol; 
#if defined(FEAT_LINEBREAK)
char_u *sbr = get_showbreak_value(wp);



if (*sbr != NUL && vcol == vcol_sbr && wp->w_p_wrap)
vcol_adjusted = vcol - MB_CHARLEN(sbr);
#endif

#if defined(FEAT_VARTABS)
tab_len = tabstop_padding(vcol_adjusted,
wp->w_buffer->b_p_ts,
wp->w_buffer->b_p_vts_array) - 1;
#else
tab_len = (int)wp->w_buffer->b_p_ts
- vcol_adjusted % (int)wp->w_buffer->b_p_ts - 1;
#endif

#if defined(FEAT_LINEBREAK)
if (!wp->w_p_lbr || !wp->w_p_list)
#endif

n_extra = tab_len;
#if defined(FEAT_LINEBREAK)
else
{
char_u *p;
int len;
int i;
int saved_nextra = n_extra;

#if defined(FEAT_CONCEAL)
if (vcol_off > 0)

tab_len += vcol_off;

if (wp->w_p_list && lcs_tab1 && old_boguscols > 0
&& n_extra > tab_len)
tab_len += n_extra - tab_len;
#endif




len = (tab_len * mb_char2len(lcs_tab2));
if (n_extra > 0)
len += n_extra - tab_len;
c = lcs_tab1;
p = alloc(len + 1);
vim_memset(p, ' ', len);
p[len] = NUL;
vim_free(p_extra_free);
p_extra_free = p;
for (i = 0; i < tab_len; i++)
{
int lcs = lcs_tab2;

if (*p == NUL)
{
tab_len = i;
break;
}



if (lcs_tab3 && i == tab_len - 1)
lcs = lcs_tab3;
mb_char2bytes(lcs, p);
p += mb_char2len(lcs);
n_extra += mb_char2len(lcs)
- (saved_nextra > 0 ? 1 : 0);
}
p_extra = p_extra_free;
#if defined(FEAT_CONCEAL)


if (vcol_off > 0)
n_extra -= vcol_off;
#endif
}
#endif
#if defined(FEAT_CONCEAL)
{
int vc_saved = vcol_off;







FIX_FOR_BOGUSCOLS;




if (n_extra == tab_len + vc_saved && wp->w_p_list
&& lcs_tab1)
tab_len += vc_saved;
}
#endif
mb_utf8 = FALSE; 
if (wp->w_p_list)
{
c = (n_extra == 0 && lcs_tab3) ? lcs_tab3 : lcs_tab1;
#if defined(FEAT_LINEBREAK)
if (wp->w_p_lbr)
c_extra = NUL; 
else
#endif
c_extra = lcs_tab2;
c_final = lcs_tab3;
n_attr = tab_len + 1;
extra_attr = hl_combine_attr(win_attr, HL_ATTR(HLF_8));
saved_attr2 = char_attr; 
mb_c = c;
if (enc_utf8 && utf_char2len(c) > 1)
{
mb_utf8 = TRUE;
u8cc[0] = 0;
c = 0xc0;
}
}
else
{
c_final = NUL;
c_extra = ' ';
c = ' ';
}
}
else if (c == NUL
&& (wp->w_p_list
|| ((fromcol >= 0 || fromcol_prev >= 0)
&& tocol > vcol
&& VIsual_mode != Ctrl_V
&& (
#if defined(FEAT_RIGHTLEFT)
wp->w_p_rl ? (col >= 0) :
#endif
(col < wp->w_width))
&& !(noinvcur
&& lnum == wp->w_cursor.lnum
&& (colnr_T)vcol == wp->w_virtcol)))
&& lcs_eol_one > 0)
{


#if defined(FEAT_DIFF) || defined(LINE_ATTR)


if (
#if defined(FEAT_DIFF)
diff_hlf == (hlf_T)0
#if defined(LINE_ATTR)
&&
#endif
#endif
#if defined(LINE_ATTR)
line_attr == 0
#endif
)
#endif
{


if (area_highlighting && virtual_active()
&& tocol != MAXCOL && vcol < tocol)
n_extra = 0;
else
{
p_extra = at_end_str;
n_extra = 1;
c_extra = NUL;
c_final = NUL;
}
}
if (wp->w_p_list && lcs_eol > 0)
c = lcs_eol;
else
c = ' ';
lcs_eol_one = -1;
--ptr; 
if (!attr_pri)
{
extra_attr = hl_combine_attr(win_attr, HL_ATTR(HLF_AT));
n_attr = 1;
}
mb_c = c;
if (enc_utf8 && utf_char2len(c) > 1)
{
mb_utf8 = TRUE;
u8cc[0] = 0;
c = 0xc0;
}
else
mb_utf8 = FALSE; 
}
else if (c != NUL)
{
p_extra = transchar(c);
if (n_extra == 0)
n_extra = byte2cells(c) - 1;
#if defined(FEAT_RIGHTLEFT)
if ((dy_flags & DY_UHEX) && wp->w_p_rl)
rl_mirror(p_extra); 
#endif
c_extra = NUL;
c_final = NUL;
#if defined(FEAT_LINEBREAK)
if (wp->w_p_lbr)
{
char_u *p;

c = *p_extra;
p = alloc(n_extra + 1);
vim_memset(p, ' ', n_extra);
STRNCPY(p, p_extra + 1, STRLEN(p_extra) - 1);
p[n_extra] = NUL;
vim_free(p_extra_free);
p_extra_free = p_extra = p;
}
else
#endif
{
n_extra = byte2cells(c) - 1;
c = *p_extra++;
}
if (!attr_pri)
{
n_attr = n_extra + 1;
extra_attr = hl_combine_attr(win_attr, HL_ATTR(HLF_8));
saved_attr2 = char_attr; 
}
mb_utf8 = FALSE; 
}
else if (VIsual_active
&& (VIsual_mode == Ctrl_V
|| VIsual_mode == 'v')
&& virtual_active()
&& tocol != MAXCOL
&& vcol < tocol
&& (
#if defined(FEAT_RIGHTLEFT)
wp->w_p_rl ? (col >= 0) :
#endif
(col < wp->w_width)))
{
c = ' ';
--ptr; 
}
#if defined(LINE_ATTR)
else if ((
#if defined(FEAT_DIFF)
diff_hlf != (hlf_T)0 ||
#endif
#if defined(FEAT_TERMINAL)
win_attr != 0 ||
#endif
line_attr != 0
) && (
#if defined(FEAT_RIGHTLEFT)
wp->w_p_rl ? (col >= 0) :
#endif
(col
#if defined(FEAT_CONCEAL)
- boguscols
#endif
< wp->w_width)))
{

c = ' ';
--ptr; 


++did_line_attr;


if (line_attr != 0 && char_attr == search_attr
&& (did_line_attr > 1
|| (wp->w_p_list && lcs_eol > 0)))
char_attr = line_attr;
#if defined(FEAT_DIFF)
if (diff_hlf == HLF_TXD)
{
diff_hlf = HLF_CHD;
if (vi_attr == 0 || char_attr != vi_attr)
{
char_attr = HL_ATTR(diff_hlf);
if (wp->w_p_cul && lnum == wp->w_cursor.lnum
&& wp->w_p_culopt_flags != CULOPT_NBR
&& (!cul_screenline
|| (vcol >= left_curline_col
&& vcol <= right_curline_col)))
char_attr = hl_combine_attr(
char_attr, HL_ATTR(HLF_CUL));
}
}
#endif
#if defined(FEAT_TERMINAL)
if (win_attr != 0)
{
char_attr = win_attr;
if (wp->w_p_cul && lnum == wp->w_cursor.lnum
&& wp->w_p_culopt_flags != CULOPT_NBR)
{
if (!cul_screenline || (vcol >= left_curline_col
&& vcol <= right_curline_col))
char_attr = hl_combine_attr(
char_attr, HL_ATTR(HLF_CUL));
}
else if (line_attr)
char_attr = hl_combine_attr(char_attr, line_attr);
}
#endif
}
#endif
}

#if defined(FEAT_CONCEAL)
if ( wp->w_p_cole > 0
&& (wp != curwin || lnum != wp->w_cursor.lnum ||
conceal_cursor_line(wp))
&& ((syntax_flags & HL_CONCEAL) != 0 || has_match_conc > 0)
&& !(lnum_in_visual_area
&& vim_strchr(wp->w_p_cocu, 'v') == NULL))
{
char_attr = conceal_attr;
if ((prev_syntax_id != syntax_seqnr || has_match_conc > 1)
&& (syn_get_sub_char() != NUL || match_conc
|| wp->w_p_cole == 1)
&& wp->w_p_cole != 3)
{


if (match_conc)
c = match_conc;
else if (syn_get_sub_char() != NUL)
c = syn_get_sub_char();
else if (lcs_conceal != NUL)
c = lcs_conceal;
else
c = ' ';

prev_syntax_id = syntax_seqnr;

if (n_extra > 0)
vcol_off += n_extra;
vcol += n_extra;
if (wp->w_p_wrap && n_extra > 0)
{
#if defined(FEAT_RIGHTLEFT)
if (wp->w_p_rl)
{
col -= n_extra;
boguscols -= n_extra;
}
else
#endif
{
boguscols += n_extra;
col += n_extra;
}
}
n_extra = 0;
n_attr = 0;
}
else if (n_skip == 0)
{
is_concealing = TRUE;
n_skip = 1;
}
mb_c = c;
if (enc_utf8 && utf_char2len(c) > 1)
{
mb_utf8 = TRUE;
u8cc[0] = 0;
c = 0xc0;
}
else
mb_utf8 = FALSE; 
}
else
{
prev_syntax_id = 0;
is_concealing = FALSE;
}

if (n_skip > 0 && did_decrement_ptr)

++ptr;

#endif 
}

#if defined(FEAT_CONCEAL)


if (!did_wcol && draw_state == WL_LINE
&& wp == curwin && lnum == wp->w_cursor.lnum
&& conceal_cursor_line(wp)
&& (int)wp->w_virtcol <= vcol + n_skip)
{
#if defined(FEAT_RIGHTLEFT)
if (wp->w_p_rl)
wp->w_wcol = wp->w_width - col + boguscols - 1;
else
#endif
wp->w_wcol = col - boguscols;
wp->w_wrow = row;
did_wcol = TRUE;
curwin->w_valid |= VALID_WCOL|VALID_WROW|VALID_VIRTCOL;
}
#endif


if (n_attr > 0
&& draw_state == WL_LINE
&& !attr_pri)
{
#if defined(LINE_ATTR)
if (line_attr)
char_attr = hl_combine_attr(extra_attr, line_attr);
else
#endif
char_attr = extra_attr;
}

#if defined(FEAT_XIM) && defined(FEAT_GUI_GTK)



if (p_imst == IM_ON_THE_SPOT
&& xic != NULL
&& lnum == wp->w_cursor.lnum
&& (State & INSERT)
&& !p_imdisable
&& im_is_preediting()
&& draw_state == WL_LINE)
{
colnr_T tcol;

if (preedit_end_col == MAXCOL)
getvcol(curwin, &(wp->w_cursor), &tcol, NULL, NULL);
else
tcol = preedit_end_col;
if ((long)preedit_start_col <= vcol && vcol < (long)tcol)
{
if (feedback_old_attr < 0)
{
feedback_col = 0;
feedback_old_attr = char_attr;
}
char_attr = im_get_feedback_attr(feedback_col);
if (char_attr < 0)
char_attr = feedback_old_attr;
feedback_col++;
}
else if (feedback_old_attr >= 0)
{
char_attr = feedback_old_attr;
feedback_old_attr = -1;
feedback_col = 0;
}
}
#endif



if (lcs_prec_todo != NUL
&& wp->w_p_list
&& (wp->w_p_wrap ?
(wp->w_skipcol > 0 && row == 0) :
wp->w_leftcol > 0)
#if defined(FEAT_DIFF)
&& filler_todo <= 0
#endif
&& draw_state > WL_NR
&& c != NUL)
{
c = lcs_prec;
lcs_prec_todo = NUL;
if (has_mbyte && (*mb_char2cells)(mb_c) > 1)
{


c_extra = MB_FILLER_CHAR;
c_final = NUL;
n_extra = 1;
n_attr = 2;
extra_attr = hl_combine_attr(win_attr, HL_ATTR(HLF_AT));
}
mb_c = c;
if (enc_utf8 && utf_char2len(c) > 1)
{
mb_utf8 = TRUE;
u8cc[0] = 0;
c = 0xc0;
}
else
mb_utf8 = FALSE; 
if (!attr_pri)
{
saved_attr3 = char_attr; 
char_attr = hl_combine_attr(win_attr, HL_ATTR(HLF_AT));
n_attr3 = 1;
}
}


if ((c == NUL
#if defined(LINE_ATTR)
|| did_line_attr == 1
#endif
) && eol_hl_off == 0)
{
#if defined(FEAT_SEARCH_EXTRA)


int prevcol_hl_flag = get_prevcol_hl_flag(wp, &screen_search_hl,
(long)(ptr - line) - (c == NUL));
#endif




if (lcs_eol == lcs_eol_one
&& ((area_attr != 0 && vcol == fromcol
&& (VIsual_mode != Ctrl_V
|| lnum == VIsual.lnum
|| lnum == curwin->w_cursor.lnum)
&& c == NUL)
#if defined(FEAT_SEARCH_EXTRA)

|| (prevcol_hl_flag
#if defined(FEAT_SYN_HL)
&& !(wp->w_p_cul && lnum == wp->w_cursor.lnum
&& !(wp == curwin && VIsual_active))
#endif
#if defined(FEAT_DIFF)
&& diff_hlf == (hlf_T)0
#endif
#if defined(LINE_ATTR)
&& did_line_attr <= 1
#endif
)
#endif
))
{
int n = 0;

#if defined(FEAT_RIGHTLEFT)
if (wp->w_p_rl)
{
if (col < 0)
n = 1;
}
else
#endif
{
if (col >= wp->w_width)
n = -1;
}
if (n != 0)
{


off += n;
col += n;
}
else
{

ScreenLines[off] = ' ';
if (enc_utf8)
ScreenLinesUC[off] = 0;
}
#if defined(FEAT_SEARCH_EXTRA)
if (area_attr == 0)
{


get_search_match_hl(wp, &screen_search_hl,
(long)(ptr - line), &char_attr);
}
#endif
ScreenAttrs[off] = char_attr;
#if defined(FEAT_RIGHTLEFT)
if (wp->w_p_rl)
{
--col;
--off;
}
else
#endif
{
++col;
++off;
}
++vcol;
eol_hl_off = 1;
}
}


if (c == NUL)
{
#if defined(FEAT_SYN_HL)

if (wp->w_p_wrap)
v = wp->w_skipcol;
else
v = wp->w_leftcol;


if (vcol < v + col - win_col_off(wp))
vcol = v + col - win_col_off(wp);
#if defined(FEAT_CONCEAL)


col -= boguscols;
boguscols = 0;
#endif

if (draw_color_col)
draw_color_col = advance_color_col(VCOL_HLC, &color_cols);

if (((wp->w_p_cuc
&& (int)wp->w_virtcol >= VCOL_HLC - eol_hl_off
&& (int)wp->w_virtcol <
wp->w_width * (row - startrow + 1) + v
&& lnum != wp->w_cursor.lnum)
|| draw_color_col
|| win_attr != 0)
#if defined(FEAT_RIGHTLEFT)
&& !wp->w_p_rl
#endif
)
{
int rightmost_vcol = 0;
int i;

if (wp->w_p_cuc)
rightmost_vcol = wp->w_virtcol;
if (draw_color_col)

for (i = 0; color_cols[i] >= 0; ++i)
if (rightmost_vcol < color_cols[i])
rightmost_vcol = color_cols[i];

while (col < wp->w_width)
{
ScreenLines[off] = ' ';
if (enc_utf8)
ScreenLinesUC[off] = 0;
++col;
if (draw_color_col)
draw_color_col = advance_color_col(VCOL_HLC,
&color_cols);

if (wp->w_p_cuc && VCOL_HLC == (long)wp->w_virtcol)
ScreenAttrs[off++] = HL_ATTR(HLF_CUC);
else if (draw_color_col && VCOL_HLC == *color_cols)
ScreenAttrs[off++] = HL_ATTR(HLF_MC);
else
ScreenAttrs[off++] = win_attr;

if (VCOL_HLC >= rightmost_vcol && win_attr == 0)
break;

++vcol;
}
}
#endif

screen_line(screen_row, wp->w_wincol, col,
(int)wp->w_width, screen_line_flags);
row++;



if (wp == curwin && lnum == curwin->w_cursor.lnum)
{
curwin->w_cline_row = startrow;
curwin->w_cline_height = row - startrow;
#if defined(FEAT_FOLDING)
curwin->w_cline_folded = FALSE;
#endif
curwin->w_valid |= (VALID_CHEIGHT|VALID_CROW);
}

break;
}



if (lcs_ext != NUL
&& wp->w_p_list
&& !wp->w_p_wrap
#if defined(FEAT_DIFF)
&& filler_todo <= 0
#endif
&& (
#if defined(FEAT_RIGHTLEFT)
wp->w_p_rl ? col == 0 :
#endif
col == wp->w_width - 1)
&& (*ptr != NUL
|| (wp->w_p_list && lcs_eol_one > 0)
|| (n_extra && (c_extra != NUL || *p_extra != NUL))))
{
c = lcs_ext;
char_attr = hl_combine_attr(win_attr, HL_ATTR(HLF_AT));
mb_c = c;
if (enc_utf8 && utf_char2len(c) > 1)
{
mb_utf8 = TRUE;
u8cc[0] = 0;
c = 0xc0;
}
else
mb_utf8 = FALSE;
}

#if defined(FEAT_SYN_HL)

if (draw_color_col)
draw_color_col = advance_color_col(VCOL_HLC, &color_cols);





vcol_save_attr = -1;
if (draw_state == WL_LINE && !lnum_in_visual_area
&& search_attr == 0 && area_attr == 0)
{
if (wp->w_p_cuc && VCOL_HLC == (long)wp->w_virtcol
&& lnum != wp->w_cursor.lnum)
{
vcol_save_attr = char_attr;
char_attr = hl_combine_attr(char_attr, HL_ATTR(HLF_CUC));
}
else if (draw_color_col && VCOL_HLC == *color_cols)
{
vcol_save_attr = char_attr;
char_attr = hl_combine_attr(char_attr, HL_ATTR(HLF_MC));
}
}
#endif



vcol_prev = vcol;
if (draw_state < WL_LINE || n_skip <= 0)
{

#if defined(FEAT_RIGHTLEFT)
if (has_mbyte && wp->w_p_rl && (*mb_char2cells)(mb_c) > 1)
{

--off;
--col;
}
#endif
ScreenLines[off] = c;
if (enc_dbcs == DBCS_JPNU)
{
if ((mb_c & 0xff00) == 0x8e00)
ScreenLines[off] = 0x8e;
ScreenLines2[off] = mb_c & 0xff;
}
else if (enc_utf8)
{
if (mb_utf8)
{
int i;

ScreenLinesUC[off] = mb_c;
if ((c & 0xff) == 0)
ScreenLines[off] = 0x80; 
for (i = 0; i < Screen_mco; ++i)
{
ScreenLinesC[i][off] = u8cc[i];
if (u8cc[i] == 0)
break;
}
}
else
ScreenLinesUC[off] = 0;
}
if (multi_attr)
{
ScreenAttrs[off] = multi_attr;
multi_attr = 0;
}
else
ScreenAttrs[off] = char_attr;

if (has_mbyte && (*mb_char2cells)(mb_c) > 1)
{

++off;
++col;
if (enc_utf8)

ScreenLines[off] = 0;
else

ScreenLines[off] = mb_c & 0xff;
if (draw_state > WL_NR
#if defined(FEAT_DIFF)
&& filler_todo <= 0
#endif
)
++vcol;


if (tocol == vcol)
++tocol;
#if defined(FEAT_RIGHTLEFT)
if (wp->w_p_rl)
{

--off;
--col;
}
#endif
}
#if defined(FEAT_RIGHTLEFT)
if (wp->w_p_rl)
{
--off;
--col;
}
else
#endif
{
++off;
++col;
}
}
#if defined(FEAT_CONCEAL)
else if (wp->w_p_cole > 0 && is_concealing)
{
--n_skip;
++vcol_off;
if (n_extra > 0)
vcol_off += n_extra;
if (wp->w_p_wrap)
{











if (n_extra > 0)
{
vcol += n_extra;
#if defined(FEAT_RIGHTLEFT)
if (wp->w_p_rl)
{
col -= n_extra;
boguscols -= n_extra;
}
else
#endif
{
col += n_extra;
boguscols += n_extra;
}
n_extra = 0;
n_attr = 0;
}


if (has_mbyte && (*mb_char2cells)(mb_c) > 1)
{

#if defined(FEAT_RIGHTLEFT)
if (wp->w_p_rl)
{
--boguscols;
--col;
}
else
#endif
{
++boguscols;
++col;
}
}

#if defined(FEAT_RIGHTLEFT)
if (wp->w_p_rl)
{
--boguscols;
--col;
}
else
#endif
{
++boguscols;
++col;
}
}
else
{
if (n_extra > 0)
{
vcol += n_extra;
n_extra = 0;
n_attr = 0;
}
}

}
#endif 
else
--n_skip;



if (draw_state > WL_NR
#if defined(FEAT_DIFF)
&& filler_todo <= 0
#endif
)
++vcol;

#if defined(FEAT_SYN_HL)
if (vcol_save_attr >= 0)
char_attr = vcol_save_attr;
#endif


if (draw_state > WL_NR && n_attr3 > 0 && --n_attr3 == 0)
char_attr = saved_attr3;


if (n_attr > 0 && draw_state == WL_LINE && --n_attr == 0)
char_attr = saved_attr2;



if ((
#if defined(FEAT_RIGHTLEFT)
wp->w_p_rl ? (col < 0) :
#endif
(col >= wp->w_width))
&& (*ptr != NUL
#if defined(FEAT_DIFF)
|| filler_todo > 0
#endif
|| (wp->w_p_list && lcs_eol != NUL && p_extra != at_end_str)
|| (n_extra != 0 && (c_extra != NUL || *p_extra != NUL)))
)
{
#if defined(FEAT_CONCEAL)
screen_line(screen_row, wp->w_wincol, col - boguscols,
(int)wp->w_width, screen_line_flags);
boguscols = 0;
#else
screen_line(screen_row, wp->w_wincol, col,
(int)wp->w_width, screen_line_flags);
#endif
++row;
++screen_row;



if ((!wp->w_p_wrap
#if defined(FEAT_DIFF)
&& filler_todo <= 0
#endif
) || lcs_eol_one == -1)
break;


if (draw_state != WL_LINE
#if defined(FEAT_DIFF)
&& filler_todo <= 0
#endif
)
{
win_draw_end(wp, '@', ' ', TRUE, row, wp->w_height, HLF_AT);
draw_vsep_win(wp, row);
row = endrow;
}


if (row == endrow)
{
++row;
break;
}

if (screen_cur_row == screen_row - 1
#if defined(FEAT_DIFF)
&& filler_todo <= 0
#endif
&& wp->w_width == Columns)
{

LineWraps[screen_row - 1] = TRUE;











if (p_tf
#if defined(FEAT_GUI)
&& !gui.in_use
#endif
&& !(has_mbyte
&& ((*mb_off2cells)(LineOffset[screen_row],
LineOffset[screen_row] + screen_Columns)
== 2
|| (*mb_off2cells)(LineOffset[screen_row - 1]
+ (int)Columns - 2,
LineOffset[screen_row] + screen_Columns)
== 2)))
{




if (screen_cur_col != wp->w_width)
screen_char(LineOffset[screen_row - 1]
+ (unsigned)Columns - 1,
screen_row - 1, (int)(Columns - 1));



if (has_mbyte && MB_BYTE2LEN(ScreenLines[LineOffset[
screen_row - 1] + (Columns - 1)]) > 1)
out_char(' ');
else
out_char(ScreenLines[LineOffset[screen_row - 1]
+ (Columns - 1)]);

ScreenAttrs[LineOffset[screen_row]] = (sattr_T)-1;
screen_start(); 
}
}

col = 0;
off = (unsigned)(current_ScreenLine - ScreenLines);
#if defined(FEAT_RIGHTLEFT)
if (wp->w_p_rl)
{
col = wp->w_width - 1; 
off += col;
}
#endif


draw_state = WL_START;
saved_n_extra = n_extra;
saved_p_extra = p_extra;
saved_c_extra = c_extra;
saved_c_final = c_final;
#if defined(FEAT_SYN_HL)
if (!(cul_screenline
#if defined(FEAT_DIFF)
&& diff_hlf == (hlf_T)0)
#endif
)
saved_char_attr = char_attr;
else
#endif
saved_char_attr = 0;
n_extra = 0;
lcs_prec_todo = lcs_prec;
#if defined(FEAT_LINEBREAK)
#if defined(FEAT_DIFF)
if (filler_todo <= 0)
#endif
need_showbreak = TRUE;
#endif
#if defined(FEAT_DIFF)
--filler_todo;


if (filler_todo == 0 && wp->w_botfill)
break;
#endif
}

} 

#if defined(FEAT_SPELL)

if (*skipwhite(line) == NUL)
{
capcol_lnum = lnum + 1;
cap_col = 0;
}
#endif
#if defined(FEAT_PROP_POPUP)
vim_free(text_props);
vim_free(text_prop_idxs);
#endif

vim_free(p_extra_free);
return row;
}
