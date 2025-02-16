









#include "vim.h"

#if defined(FEAT_BEVAL) || defined(FEAT_PROP_POPUP) || defined(PROTO)







int
find_word_under_cursor(
int mouserow,
int mousecol,
int getword,
int flags, 
win_T **winp, 
linenr_T *lnump, 
char_u **textp,
int *colp, 
int *startcolp) 
{
int row = mouserow;
int col = mousecol;
int scol;
win_T *wp;
char_u *lbuf;
linenr_T lnum;

*textp = NULL;
wp = mouse_find_win(&row, &col, FAIL_POPUP);
if (wp != NULL && row >= 0 && row < wp->w_height && col < wp->w_width)
{


if (!mouse_comp_pos(wp, &row, &col, &lnum, NULL))
{

lbuf = ml_get_buf(wp->w_buffer, lnum, FALSE);
if (col <= win_linetabsize(wp, lbuf, (colnr_T)MAXCOL))
{

if (getword)
{


int len;
pos_T *spos = NULL, *epos = NULL;

if (VIsual_active)
{
if (LT_POS(VIsual, curwin->w_cursor))
{
spos = &VIsual;
epos = &curwin->w_cursor;
}
else
{
spos = &curwin->w_cursor;
epos = &VIsual;
}
}

col = vcol2col(wp, lnum, col);
scol = col;

if (VIsual_active
&& wp->w_buffer == curwin->w_buffer
&& (lnum == spos->lnum
? col >= (int)spos->col
: lnum > spos->lnum)
&& (lnum == epos->lnum
? col <= (int)epos->col
: lnum < epos->lnum))
{



if (spos->lnum != epos->lnum || spos->col == epos->col)
return FAIL;

lbuf = ml_get_buf(curwin->w_buffer, VIsual.lnum, FALSE);
len = epos->col - spos->col;
if (*p_sel != 'e')
len += mb_ptr2len(lbuf + epos->col);
lbuf = vim_strnsave(lbuf + spos->col, len);
lnum = spos->lnum;
col = spos->col;
scol = col;
}
else
{

++emsg_off;
len = find_ident_at_pos(wp, lnum, (colnr_T)col,
&lbuf, &scol, flags);
--emsg_off;
if (len == 0)
return FAIL;
lbuf = vim_strnsave(lbuf, len);
}
}

if (winp != NULL)
*winp = wp;
if (lnump != NULL)
*lnump = lnum;
*textp = lbuf;
if (colp != NULL)
*colp = col;
if (startcolp != NULL)
*startcolp = scol;
return OK;
}
}
}
return FAIL;
}
#endif

#if defined(FEAT_BEVAL) || defined(PROTO)







int
get_beval_info(
BalloonEval *beval,
int getword,
win_T **winp,
linenr_T *lnump,
char_u **textp,
int *colp)
{
int row = mouse_row;
int col = mouse_col;

#if defined(FEAT_GUI)
if (gui.in_use)
{
row = Y_2_ROW(beval->y);
col = X_2_COL(beval->x);
}
#endif
if (find_word_under_cursor(row, col, getword,
FIND_IDENT + FIND_STRING + FIND_EVAL,
winp, lnump, textp, colp, NULL) == OK)
{
#if defined(FEAT_VARTABS)
vim_free(beval->vts);
beval->vts = tabstop_copy((*winp)->w_buffer->b_p_vts_array);
if ((*winp)->w_buffer->b_p_vts_array != NULL && beval->vts == NULL)
{
if (getword)
vim_free(*textp);
return FAIL;
}
#endif
beval->ts = (*winp)->w_buffer->b_p_ts;
return OK;
}

return FAIL;
}





void
post_balloon(BalloonEval *beval UNUSED, char_u *mesg, list_T *list UNUSED)
{
#if defined(FEAT_BEVAL_TERM)
#if defined(FEAT_GUI)
if (!gui.in_use)
#endif
ui_post_balloon(mesg, list);
#endif
#if defined(FEAT_BEVAL_GUI)
if (gui.in_use)

gui_mch_post_balloon(beval, mesg);
#endif
}






int
can_use_beval(void)
{
return (0
#if defined(FEAT_BEVAL_GUI)
|| (gui.in_use && p_beval)
#endif
#if defined(FEAT_BEVAL_TERM)
|| (
#if defined(FEAT_GUI)
!gui.in_use &&
#endif
p_bevalterm)
#endif
) && msg_scrolled == 0;
}




void
general_beval_cb(BalloonEval *beval, int state UNUSED)
{
#if defined(FEAT_EVAL)
win_T *wp;
int col;
int use_sandbox;
linenr_T lnum;
char_u *text;
static char_u *result = NULL;
long winnr = 0;
char_u *bexpr;
buf_T *save_curbuf;
size_t len;
win_T *cw;
#endif
static int recursive = FALSE;



if (!can_use_beval() || beval == NULL)
return;



if (recursive)
return;
recursive = TRUE;

#if defined(FEAT_EVAL)
if (get_beval_info(beval, TRUE, &wp, &lnum, &text, &col) == OK)
{
bexpr = (*wp->w_buffer->b_p_bexpr == NUL) ? p_bexpr
: wp->w_buffer->b_p_bexpr;
if (*bexpr != NUL)
{

for (cw = firstwin; cw != wp; cw = cw->w_next)
++winnr;

set_vim_var_nr(VV_BEVAL_BUFNR, (long)wp->w_buffer->b_fnum);
set_vim_var_nr(VV_BEVAL_WINNR, winnr);
set_vim_var_nr(VV_BEVAL_WINID, wp->w_id);
set_vim_var_nr(VV_BEVAL_LNUM, (long)lnum);
set_vim_var_nr(VV_BEVAL_COL, (long)(col + 1));
set_vim_var_string(VV_BEVAL_TEXT, text, -1);
vim_free(text);





save_curbuf = curbuf;
curbuf = wp->w_buffer;
use_sandbox = was_set_insecurely((char_u *)"balloonexpr",
*curbuf->b_p_bexpr == NUL ? 0 : OPT_LOCAL);
curbuf = save_curbuf;
if (use_sandbox)
++sandbox;
++textlock;

vim_free(result);
result = eval_to_string(bexpr, NULL, TRUE);




if (result != NULL)
{
len = STRLEN(result);
if (len > 0 && result[len - 1] == NL)
result[len - 1] = NUL;
}

if (use_sandbox)
--sandbox;
--textlock;

set_vim_var_string(VV_BEVAL_TEXT, NULL, -1);
if (result != NULL && result[0] != NUL)
post_balloon(beval, result, NULL);



if (must_redraw)
redraw_after_callback(FALSE);

recursive = FALSE;
return;
}
}
#endif
#if defined(FEAT_NETBEANS_INTG)
if (bevalServers & BEVAL_NETBEANS)
netbeans_beval_cb(beval, state);
#endif

recursive = FALSE;
}

#endif
