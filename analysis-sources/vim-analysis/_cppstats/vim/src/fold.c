#include "vim.h"
#if defined(FEAT_FOLDING) || defined(PROTO)
typedef struct
{
linenr_T fd_top; 
linenr_T fd_len; 
garray_T fd_nested; 
char fd_flags; 
char fd_small; 
} fold_T;
#define FD_OPEN 0 
#define FD_CLOSED 1 
#define FD_LEVEL 2 
#define MAX_LEVEL 20 
static void newFoldLevelWin(win_T *wp);
static int checkCloseRec(garray_T *gap, linenr_T lnum, int level);
static int foldFind(garray_T *gap, linenr_T lnum, fold_T **fpp);
static int foldLevelWin(win_T *wp, linenr_T lnum);
static void checkupdate(win_T *wp);
static void setFoldRepeat(linenr_T lnum, long count, int do_open);
static linenr_T setManualFold(linenr_T lnum, int opening, int recurse, int *donep);
static linenr_T setManualFoldWin(win_T *wp, linenr_T lnum, int opening, int recurse, int *donep);
static void foldOpenNested(fold_T *fpr);
static void deleteFoldEntry(garray_T *gap, int idx, int recursive);
static void foldMarkAdjustRecurse(garray_T *gap, linenr_T line1, linenr_T line2, long amount, long amount_after);
static int getDeepestNestingRecurse(garray_T *gap);
static int check_closed(win_T *win, fold_T *fp, int *use_levelp, int level, int *maybe_smallp, linenr_T lnum_off);
static void checkSmall(win_T *wp, fold_T *fp, linenr_T lnum_off);
static void setSmallMaybe(garray_T *gap);
static void foldCreateMarkers(linenr_T start, linenr_T end);
static void foldAddMarker(linenr_T lnum, char_u *marker, int markerlen);
static void deleteFoldMarkers(fold_T *fp, int recursive, linenr_T lnum_off);
static void foldDelMarker(linenr_T lnum, char_u *marker, int markerlen);
static void foldUpdateIEMS(win_T *wp, linenr_T top, linenr_T bot);
static void parseMarker(win_T *wp);
static char *e_nofold = N_("E490: No fold found");
static linenr_T invalid_top = (linenr_T)0;
static linenr_T invalid_bot = (linenr_T)0;
static linenr_T prev_lnum = 0;
static int prev_lnum_lvl = -1;
#define DONE_NOTHING 0
#define DONE_ACTION 1 
#define DONE_FOLD 2 
static int foldstartmarkerlen;
static char_u *foldendmarker;
static int foldendmarkerlen;
void
copyFoldingState(win_T *wp_from, win_T *wp_to)
{
wp_to->w_fold_manual = wp_from->w_fold_manual;
wp_to->w_foldinvalid = wp_from->w_foldinvalid;
cloneFoldGrowArray(&wp_from->w_folds, &wp_to->w_folds);
}
int
hasAnyFolding(win_T *win)
{
return (win->w_p_fen
&& (!foldmethodIsManual(win) || win->w_folds.ga_len > 0));
}
int
hasFolding(linenr_T lnum, linenr_T *firstp, linenr_T *lastp)
{
return hasFoldingWin(curwin, lnum, firstp, lastp, TRUE, NULL);
}
int
hasFoldingWin(
win_T *win,
linenr_T lnum,
linenr_T *firstp,
linenr_T *lastp,
int cache, 
foldinfo_T *infop) 
{
int had_folded = FALSE;
linenr_T first = 0;
linenr_T last = 0;
linenr_T lnum_rel = lnum;
int x;
fold_T *fp;
int level = 0;
int use_level = FALSE;
int maybe_small = FALSE;
garray_T *gap;
int low_level = 0;
checkupdate(win);
if (!hasAnyFolding(win))
{
if (infop != NULL)
infop->fi_level = 0;
return FALSE;
}
if (cache)
{
x = find_wl_entry(win, lnum);
if (x >= 0)
{
first = win->w_lines[x].wl_lnum;
last = win->w_lines[x].wl_lastlnum;
had_folded = win->w_lines[x].wl_folded;
}
}
if (first == 0)
{
gap = &win->w_folds;
for (;;)
{
if (!foldFind(gap, lnum_rel, &fp))
break;
if (lnum_rel == fp->fd_top && low_level == 0)
low_level = level + 1;
first += fp->fd_top;
last += fp->fd_top;
had_folded = check_closed(win, fp, &use_level, level,
&maybe_small, lnum - lnum_rel);
if (had_folded)
{
last += fp->fd_len - 1;
break;
}
gap = &fp->fd_nested;
lnum_rel -= fp->fd_top;
++level;
}
}
if (!had_folded)
{
if (infop != NULL)
{
infop->fi_level = level;
infop->fi_lnum = lnum - lnum_rel;
infop->fi_low_level = low_level == 0 ? level : low_level;
}
return FALSE;
}
if (last > win->w_buffer->b_ml.ml_line_count)
last = win->w_buffer->b_ml.ml_line_count;
if (lastp != NULL)
*lastp = last;
if (firstp != NULL)
*firstp = first;
if (infop != NULL)
{
infop->fi_level = level + 1;
infop->fi_lnum = first;
infop->fi_low_level = low_level == 0 ? level + 1 : low_level;
}
return TRUE;
}
#if defined(FEAT_EVAL)
static int
foldLevel(linenr_T lnum)
{
if (invalid_top == (linenr_T)0)
checkupdate(curwin);
else if (lnum == prev_lnum && prev_lnum_lvl >= 0)
return prev_lnum_lvl;
else if (lnum >= invalid_top && lnum <= invalid_bot)
return -1;
if (!hasAnyFolding(curwin))
return 0;
return foldLevelWin(curwin, lnum);
}
#endif
int
lineFolded(win_T *win, linenr_T lnum)
{
return foldedCount(win, lnum, NULL) != 0;
}
long
foldedCount(win_T *win, linenr_T lnum, foldinfo_T *infop)
{
linenr_T last;
if (hasFoldingWin(win, lnum, NULL, &last, FALSE, infop))
return (long)(last - lnum + 1);
return 0;
}
int
foldmethodIsManual(win_T *wp)
{
return (wp->w_p_fdm[3] == 'u');
}
int
foldmethodIsIndent(win_T *wp)
{
return (wp->w_p_fdm[0] == 'i');
}
int
foldmethodIsExpr(win_T *wp)
{
return (wp->w_p_fdm[1] == 'x');
}
int
foldmethodIsMarker(win_T *wp)
{
return (wp->w_p_fdm[2] == 'r');
}
int
foldmethodIsSyntax(win_T *wp)
{
return (wp->w_p_fdm[0] == 's');
}
int
foldmethodIsDiff(win_T *wp)
{
return (wp->w_p_fdm[0] == 'd');
}
void
closeFold(linenr_T lnum, long count)
{
setFoldRepeat(lnum, count, FALSE);
}
void
closeFoldRecurse(linenr_T lnum)
{
(void)setManualFold(lnum, FALSE, TRUE, NULL);
}
void
opFoldRange(
linenr_T first,
linenr_T last,
int opening, 
int recurse, 
int had_visual) 
{
int done = DONE_NOTHING; 
linenr_T lnum;
linenr_T lnum_next;
for (lnum = first; lnum <= last; lnum = lnum_next + 1)
{
lnum_next = lnum;
if (opening && !recurse)
(void)hasFolding(lnum, NULL, &lnum_next);
(void)setManualFold(lnum, opening, recurse, &done);
if (!opening && !recurse)
(void)hasFolding(lnum, NULL, &lnum_next);
}
if (done == DONE_NOTHING)
emsg(_(e_nofold));
if (had_visual)
redraw_curbuf_later(INVERTED);
}
void
openFold(linenr_T lnum, long count)
{
setFoldRepeat(lnum, count, TRUE);
}
void
openFoldRecurse(linenr_T lnum)
{
(void)setManualFold(lnum, TRUE, TRUE, NULL);
}
void
foldOpenCursor(void)
{
int done;
checkupdate(curwin);
if (hasAnyFolding(curwin))
for (;;)
{
done = DONE_NOTHING;
(void)setManualFold(curwin->w_cursor.lnum, TRUE, FALSE, &done);
if (!(done & DONE_ACTION))
break;
}
}
void
newFoldLevel(void)
{
newFoldLevelWin(curwin);
#if defined(FEAT_DIFF)
if (foldmethodIsDiff(curwin) && curwin->w_p_scb)
{
win_T *wp;
FOR_ALL_WINDOWS(wp)
{
if (wp != curwin && foldmethodIsDiff(wp) && wp->w_p_scb)
{
wp->w_p_fdl = curwin->w_p_fdl;
newFoldLevelWin(wp);
}
}
}
#endif
}
static void
newFoldLevelWin(win_T *wp)
{
fold_T *fp;
int i;
checkupdate(wp);
if (wp->w_fold_manual)
{
fp = (fold_T *)wp->w_folds.ga_data;
for (i = 0; i < wp->w_folds.ga_len; ++i)
fp[i].fd_flags = FD_LEVEL;
wp->w_fold_manual = FALSE;
}
changed_window_setting_win(wp);
}
void
foldCheckClose(void)
{
if (*p_fcl != NUL) 
{
checkupdate(curwin);
if (checkCloseRec(&curwin->w_folds, curwin->w_cursor.lnum,
(int)curwin->w_p_fdl))
changed_window_setting();
}
}
static int
checkCloseRec(garray_T *gap, linenr_T lnum, int level)
{
fold_T *fp;
int retval = FALSE;
int i;
fp = (fold_T *)gap->ga_data;
for (i = 0; i < gap->ga_len; ++i)
{
if (fp[i].fd_flags == FD_OPEN)
{
if (level <= 0 && (lnum < fp[i].fd_top
|| lnum >= fp[i].fd_top + fp[i].fd_len))
{
fp[i].fd_flags = FD_LEVEL;
retval = TRUE;
}
else
retval |= checkCloseRec(&fp[i].fd_nested, lnum - fp[i].fd_top,
level - 1);
}
}
return retval;
}
int
foldManualAllowed(int create)
{
if (foldmethodIsManual(curwin) || foldmethodIsMarker(curwin))
return TRUE;
if (create)
emsg(_("E350: Cannot create fold with current 'foldmethod'"));
else
emsg(_("E351: Cannot delete fold with current 'foldmethod'"));
return FALSE;
}
void
foldCreate(linenr_T start, linenr_T end)
{
fold_T *fp;
garray_T *gap;
garray_T fold_ga;
int i, j;
int cont;
int use_level = FALSE;
int closed = FALSE;
int level = 0;
linenr_T start_rel = start;
linenr_T end_rel = end;
if (start > end)
{
end = start_rel;
start = end_rel;
start_rel = start;
end_rel = end;
}
if (foldmethodIsMarker(curwin))
{
foldCreateMarkers(start, end);
return;
}
checkupdate(curwin);
gap = &curwin->w_folds;
for (;;)
{
if (!foldFind(gap, start_rel, &fp))
break;
if (fp->fd_top + fp->fd_len > end_rel)
{
gap = &fp->fd_nested;
start_rel -= fp->fd_top;
end_rel -= fp->fd_top;
if (use_level || fp->fd_flags == FD_LEVEL)
{
use_level = TRUE;
if (level >= curwin->w_p_fdl)
closed = TRUE;
}
else if (fp->fd_flags == FD_CLOSED)
closed = TRUE;
++level;
}
else
{
break;
}
}
i = (int)(fp - (fold_T *)gap->ga_data);
if (ga_grow(gap, 1) == OK)
{
fp = (fold_T *)gap->ga_data + i;
ga_init2(&fold_ga, (int)sizeof(fold_T), 10);
for (cont = 0; i + cont < gap->ga_len; ++cont)
if (fp[cont].fd_top > end_rel)
break;
if (cont > 0 && ga_grow(&fold_ga, cont) == OK)
{
if (start_rel > fp->fd_top)
start_rel = fp->fd_top;
if (end_rel < fp[cont - 1].fd_top + fp[cont - 1].fd_len - 1)
end_rel = fp[cont - 1].fd_top + fp[cont - 1].fd_len - 1;
mch_memmove(fold_ga.ga_data, fp, sizeof(fold_T) * cont);
fold_ga.ga_len += cont;
i += cont;
for (j = 0; j < cont; ++j)
((fold_T *)fold_ga.ga_data)[j].fd_top -= start_rel;
}
if (i < gap->ga_len)
mch_memmove(fp + 1, (fold_T *)gap->ga_data + i,
sizeof(fold_T) * (gap->ga_len - i));
gap->ga_len = gap->ga_len + 1 - cont;
fp->fd_nested = fold_ga;
fp->fd_top = start_rel;
fp->fd_len = end_rel - start_rel + 1;
if (use_level && !closed && level < curwin->w_p_fdl)
closeFold(start, 1L);
if (!use_level)
curwin->w_fold_manual = TRUE;
fp->fd_flags = FD_CLOSED;
fp->fd_small = MAYBE;
changed_window_setting();
}
}
void
deleteFold(
linenr_T start,
linenr_T end,
int recursive,
int had_visual) 
{
garray_T *gap;
fold_T *fp;
garray_T *found_ga;
fold_T *found_fp = NULL;
linenr_T found_off = 0;
int use_level;
int maybe_small = FALSE;
int level = 0;
linenr_T lnum = start;
linenr_T lnum_off;
int did_one = FALSE;
linenr_T first_lnum = MAXLNUM;
linenr_T last_lnum = 0;
checkupdate(curwin);
while (lnum <= end)
{
gap = &curwin->w_folds;
found_ga = NULL;
lnum_off = 0;
use_level = FALSE;
for (;;)
{
if (!foldFind(gap, lnum - lnum_off, &fp))
break;
found_ga = gap;
found_fp = fp;
found_off = lnum_off;
if (check_closed(curwin, fp, &use_level, level,
&maybe_small, lnum_off))
break;
gap = &fp->fd_nested;
lnum_off += fp->fd_top;
++level;
}
if (found_ga == NULL)
{
++lnum;
}
else
{
lnum = found_fp->fd_top + found_fp->fd_len + found_off;
if (foldmethodIsManual(curwin))
deleteFoldEntry(found_ga,
(int)(found_fp - (fold_T *)found_ga->ga_data), recursive);
else
{
if (first_lnum > found_fp->fd_top + found_off)
first_lnum = found_fp->fd_top + found_off;
if (last_lnum < lnum)
last_lnum = lnum;
if (!did_one)
parseMarker(curwin);
deleteFoldMarkers(found_fp, recursive, found_off);
}
did_one = TRUE;
changed_window_setting();
}
}
if (!did_one)
{
emsg(_(e_nofold));
if (had_visual)
redraw_curbuf_later(INVERTED);
}
else
check_cursor_col();
if (last_lnum > 0)
changed_lines(first_lnum, (colnr_T)0, last_lnum, 0L);
}
void
clearFolding(win_T *win)
{
deleteFoldRecurse(&win->w_folds);
win->w_foldinvalid = FALSE;
}
void
foldUpdate(win_T *wp, linenr_T top, linenr_T bot)
{
fold_T *fp;
if (disable_fold_update > 0)
return;
#if defined(FEAT_DIFF)
if (need_diff_redraw)
return;
#endif
(void)foldFind(&wp->w_folds, top, &fp);
while (fp < (fold_T *)wp->w_folds.ga_data + wp->w_folds.ga_len
&& fp->fd_top < bot)
{
fp->fd_small = MAYBE;
++fp;
}
if (foldmethodIsIndent(wp)
|| foldmethodIsExpr(wp)
|| foldmethodIsMarker(wp)
#if defined(FEAT_DIFF)
|| foldmethodIsDiff(wp)
#endif
|| foldmethodIsSyntax(wp))
{
int save_got_int = got_int;
got_int = FALSE;
foldUpdateIEMS(wp, top, bot);
got_int |= save_got_int;
}
}
void
foldUpdateAll(win_T *win)
{
win->w_foldinvalid = TRUE;
redraw_win_later(win, NOT_VALID);
}
int
foldMoveTo(
int updown,
int dir, 
long count)
{
long n;
int retval = FAIL;
linenr_T lnum_off;
linenr_T lnum_found;
linenr_T lnum;
int use_level;
int maybe_small;
garray_T *gap;
fold_T *fp;
int level;
int last;
checkupdate(curwin);
for (n = 0; n < count; ++n)
{
lnum_off = 0;
gap = &curwin->w_folds;
use_level = FALSE;
maybe_small = FALSE;
lnum_found = curwin->w_cursor.lnum;
level = 0;
last = FALSE;
for (;;)
{
if (!foldFind(gap, curwin->w_cursor.lnum - lnum_off, &fp))
{
if (!updown)
break;
if (dir == FORWARD)
{
if (fp - (fold_T *)gap->ga_data >= gap->ga_len)
break;
--fp;
}
else
{
if (fp == (fold_T *)gap->ga_data)
break;
}
last = TRUE;
}
if (!last)
{
if (check_closed(curwin, fp, &use_level, level,
&maybe_small, lnum_off))
last = TRUE;
if (last && !updown)
break;
}
if (updown)
{
if (dir == FORWARD)
{
if (fp + 1 - (fold_T *)gap->ga_data < gap->ga_len)
{
lnum = fp[1].fd_top + lnum_off;
if (lnum > curwin->w_cursor.lnum)
lnum_found = lnum;
}
}
else
{
if (fp > (fold_T *)gap->ga_data)
{
lnum = fp[-1].fd_top + lnum_off + fp[-1].fd_len - 1;
if (lnum < curwin->w_cursor.lnum)
lnum_found = lnum;
}
}
}
else
{
if (dir == FORWARD)
{
lnum = fp->fd_top + lnum_off + fp->fd_len - 1;
if (lnum > curwin->w_cursor.lnum)
lnum_found = lnum;
}
else
{
lnum = fp->fd_top + lnum_off;
if (lnum < curwin->w_cursor.lnum)
lnum_found = lnum;
}
}
if (last)
break;
gap = &fp->fd_nested;
lnum_off += fp->fd_top;
++level;
}
if (lnum_found != curwin->w_cursor.lnum)
{
if (retval == FAIL)
setpcmark();
curwin->w_cursor.lnum = lnum_found;
curwin->w_cursor.col = 0;
retval = OK;
}
else
break;
}
return retval;
}
void
foldInitWin(win_T *new_win)
{
ga_init2(&new_win->w_folds, (int)sizeof(fold_T), 10);
}
int
find_wl_entry(win_T *win, linenr_T lnum)
{
int i;
for (i = 0; i < win->w_lines_valid; ++i)
if (win->w_lines[i].wl_valid)
{
if (lnum < win->w_lines[i].wl_lnum)
return -1;
if (lnum <= win->w_lines[i].wl_lastlnum)
return i;
}
return -1;
}
void
foldAdjustVisual(void)
{
pos_T *start, *end;
char_u *ptr;
if (!VIsual_active || !hasAnyFolding(curwin))
return;
if (LTOREQ_POS(VIsual, curwin->w_cursor))
{
start = &VIsual;
end = &curwin->w_cursor;
}
else
{
start = &curwin->w_cursor;
end = &VIsual;
}
if (hasFolding(start->lnum, &start->lnum, NULL))
start->col = 0;
if (hasFolding(end->lnum, NULL, &end->lnum))
{
ptr = ml_get(end->lnum);
end->col = (colnr_T)STRLEN(ptr);
if (end->col > 0 && *p_sel == 'o')
--end->col;
if (has_mbyte)
mb_adjust_cursor();
}
}
void
foldAdjustCursor(void)
{
(void)hasFolding(curwin->w_cursor.lnum, &curwin->w_cursor.lnum, NULL);
}
void
cloneFoldGrowArray(garray_T *from, garray_T *to)
{
int i;
fold_T *from_p;
fold_T *to_p;
ga_init2(to, from->ga_itemsize, from->ga_growsize);
if (from->ga_len == 0 || ga_grow(to, from->ga_len) == FAIL)
return;
from_p = (fold_T *)from->ga_data;
to_p = (fold_T *)to->ga_data;
for (i = 0; i < from->ga_len; i++)
{
to_p->fd_top = from_p->fd_top;
to_p->fd_len = from_p->fd_len;
to_p->fd_flags = from_p->fd_flags;
to_p->fd_small = from_p->fd_small;
cloneFoldGrowArray(&from_p->fd_nested, &to_p->fd_nested);
++to->ga_len;
++from_p;
++to_p;
}
}
static int
foldFind(garray_T *gap, linenr_T lnum, fold_T **fpp)
{
linenr_T low, high;
fold_T *fp;
int i;
fp = (fold_T *)gap->ga_data;
low = 0;
high = gap->ga_len - 1;
while (low <= high)
{
i = (low + high) / 2;
if (fp[i].fd_top > lnum)
high = i - 1;
else if (fp[i].fd_top + fp[i].fd_len <= lnum)
low = i + 1;
else
{
*fpp = fp + i;
return TRUE;
}
}
*fpp = fp + low;
return FALSE;
}
static int
foldLevelWin(win_T *wp, linenr_T lnum)
{
fold_T *fp;
linenr_T lnum_rel = lnum;
int level = 0;
garray_T *gap;
gap = &wp->w_folds;
for (;;)
{
if (!foldFind(gap, lnum_rel, &fp))
break;
gap = &fp->fd_nested;
lnum_rel -= fp->fd_top;
++level;
}
return level;
}
static void
checkupdate(win_T *wp)
{
if (wp->w_foldinvalid)
{
foldUpdate(wp, (linenr_T)1, (linenr_T)MAXLNUM); 
wp->w_foldinvalid = FALSE;
}
}
static void
setFoldRepeat(linenr_T lnum, long count, int do_open)
{
int done;
long n;
for (n = 0; n < count; ++n)
{
done = DONE_NOTHING;
(void)setManualFold(lnum, do_open, FALSE, &done);
if (!(done & DONE_ACTION))
{
if (n == 0 && !(done & DONE_FOLD))
emsg(_(e_nofold));
break;
}
}
}
static linenr_T
setManualFold(
linenr_T lnum,
int opening, 
int recurse, 
int *donep)
{
#if defined(FEAT_DIFF)
if (foldmethodIsDiff(curwin) && curwin->w_p_scb)
{
win_T *wp;
linenr_T dlnum;
FOR_ALL_WINDOWS(wp)
{
if (wp != curwin && foldmethodIsDiff(wp) && wp->w_p_scb)
{
dlnum = diff_lnum_win(curwin->w_cursor.lnum, wp);
if (dlnum != 0)
(void)setManualFoldWin(wp, dlnum, opening, recurse, NULL);
}
}
}
#endif
return setManualFoldWin(curwin, lnum, opening, recurse, donep);
}
static linenr_T
setManualFoldWin(
win_T *wp,
linenr_T lnum,
int opening, 
int recurse, 
int *donep)
{
fold_T *fp;
fold_T *fp2;
fold_T *found = NULL;
int j;
int level = 0;
int use_level = FALSE;
int found_fold = FALSE;
garray_T *gap;
linenr_T next = MAXLNUM;
linenr_T off = 0;
int done = 0;
checkupdate(wp);
gap = &wp->w_folds;
for (;;)
{
if (!foldFind(gap, lnum, &fp))
{
if (fp < (fold_T *)gap->ga_data + gap->ga_len)
next = fp->fd_top + off;
break;
}
found_fold = TRUE;
if (fp + 1 < (fold_T *)gap->ga_data + gap->ga_len)
next = fp[1].fd_top + off;
if (use_level || fp->fd_flags == FD_LEVEL)
{
use_level = TRUE;
if (level >= wp->w_p_fdl)
fp->fd_flags = FD_CLOSED;
else
fp->fd_flags = FD_OPEN;
fp2 = (fold_T *)fp->fd_nested.ga_data;
for (j = 0; j < fp->fd_nested.ga_len; ++j)
fp2[j].fd_flags = FD_LEVEL;
}
if (!opening && recurse)
{
if (fp->fd_flags != FD_CLOSED)
{
done |= DONE_ACTION;
fp->fd_flags = FD_CLOSED;
}
}
else if (fp->fd_flags == FD_CLOSED)
{
if (opening)
{
fp->fd_flags = FD_OPEN;
done |= DONE_ACTION;
if (recurse)
foldOpenNested(fp);
}
break;
}
found = fp;
gap = &fp->fd_nested;
lnum -= fp->fd_top;
off += fp->fd_top;
++level;
}
if (found_fold)
{
if (!opening && found != NULL)
{
found->fd_flags = FD_CLOSED;
done |= DONE_ACTION;
}
wp->w_fold_manual = TRUE;
if (done & DONE_ACTION)
changed_window_setting_win(wp);
done |= DONE_FOLD;
}
else if (donep == NULL && wp == curwin)
emsg(_(e_nofold));
if (donep != NULL)
*donep |= done;
return next;
}
static void
foldOpenNested(fold_T *fpr)
{
int i;
fold_T *fp;
fp = (fold_T *)fpr->fd_nested.ga_data;
for (i = 0; i < fpr->fd_nested.ga_len; ++i)
{
foldOpenNested(&fp[i]);
fp[i].fd_flags = FD_OPEN;
}
}
static void
deleteFoldEntry(garray_T *gap, int idx, int recursive)
{
fold_T *fp;
int i;
long moved;
fold_T *nfp;
fp = (fold_T *)gap->ga_data + idx;
if (recursive || fp->fd_nested.ga_len == 0)
{
deleteFoldRecurse(&fp->fd_nested);
--gap->ga_len;
if (idx < gap->ga_len)
mch_memmove(fp, fp + 1, sizeof(fold_T) * (gap->ga_len - idx));
}
else
{
moved = fp->fd_nested.ga_len;
if (ga_grow(gap, (int)(moved - 1)) == OK)
{
fp = (fold_T *)gap->ga_data + idx;
nfp = (fold_T *)fp->fd_nested.ga_data;
for (i = 0; i < moved; ++i)
{
nfp[i].fd_top += fp->fd_top;
if (fp->fd_flags == FD_LEVEL)
nfp[i].fd_flags = FD_LEVEL;
if (fp->fd_small == MAYBE)
nfp[i].fd_small = MAYBE;
}
if (idx + 1 < gap->ga_len)
mch_memmove(fp + moved, fp + 1,
sizeof(fold_T) * (gap->ga_len - (idx + 1)));
mch_memmove(fp, nfp, (size_t)(sizeof(fold_T) * moved));
vim_free(nfp);
gap->ga_len += moved - 1;
}
}
}
void
deleteFoldRecurse(garray_T *gap)
{
int i;
for (i = 0; i < gap->ga_len; ++i)
deleteFoldRecurse(&(((fold_T *)(gap->ga_data))[i].fd_nested));
ga_clear(gap);
}
void
foldMarkAdjust(
win_T *wp,
linenr_T line1,
linenr_T line2,
long amount,
long amount_after)
{
if (amount == MAXLNUM && line2 >= line1 && line2 - line1 >= -amount_after)
line2 = line1 - amount_after - 1;
if ((State & INSERT) && amount == (linenr_T)1 && line2 == MAXLNUM)
--line1;
foldMarkAdjustRecurse(&wp->w_folds, line1, line2, amount, amount_after);
}
static void
foldMarkAdjustRecurse(
garray_T *gap,
linenr_T line1,
linenr_T line2,
long amount,
long amount_after)
{
fold_T *fp;
int i;
linenr_T last;
linenr_T top;
if ((State & INSERT) && amount == (linenr_T)1 && line2 == MAXLNUM)
top = line1 + 1;
else
top = line1;
(void)foldFind(gap, line1, &fp);
for (i = (int)(fp - (fold_T *)gap->ga_data); i < gap->ga_len; ++i, ++fp)
{
last = fp->fd_top + fp->fd_len - 1; 
if (last < line1)
continue;
if (fp->fd_top > line2)
{
if (amount_after == 0)
break;
fp->fd_top += amount_after;
}
else
{
if (fp->fd_top >= top && last <= line2)
{
if (amount == MAXLNUM)
{
deleteFoldEntry(gap, i, TRUE);
--i; 
--fp;
}
else
fp->fd_top += amount;
}
else
{
if (fp->fd_top < top)
{
foldMarkAdjustRecurse(&fp->fd_nested, line1 - fp->fd_top,
line2 - fp->fd_top, amount, amount_after);
if (last <= line2)
{
if (amount == MAXLNUM)
fp->fd_len = line1 - fp->fd_top;
else
fp->fd_len += amount;
}
else
{
fp->fd_len += amount_after;
}
}
else
{
if (amount == MAXLNUM)
{
foldMarkAdjustRecurse(&fp->fd_nested,
line1 - fp->fd_top,
line2 - fp->fd_top,
amount,
amount_after + (fp->fd_top - top));
fp->fd_len -= line2 - fp->fd_top + 1;
fp->fd_top = line1;
}
else
{
foldMarkAdjustRecurse(&fp->fd_nested,
line1 - fp->fd_top,
line2 - fp->fd_top,
amount,
amount_after - amount);
fp->fd_len += amount_after - amount;
fp->fd_top += amount;
}
}
}
}
}
}
int
getDeepestNesting(void)
{
checkupdate(curwin);
return getDeepestNestingRecurse(&curwin->w_folds);
}
static int
getDeepestNestingRecurse(garray_T *gap)
{
int i;
int level;
int maxlevel = 0;
fold_T *fp;
fp = (fold_T *)gap->ga_data;
for (i = 0; i < gap->ga_len; ++i)
{
level = getDeepestNestingRecurse(&fp[i].fd_nested) + 1;
if (level > maxlevel)
maxlevel = level;
}
return maxlevel;
}
static int
check_closed(
win_T *win,
fold_T *fp,
int *use_levelp, 
int level, 
int *maybe_smallp, 
linenr_T lnum_off) 
{
int closed = FALSE;
if (*use_levelp || fp->fd_flags == FD_LEVEL)
{
*use_levelp = TRUE;
if (level >= win->w_p_fdl)
closed = TRUE;
}
else if (fp->fd_flags == FD_CLOSED)
closed = TRUE;
if (fp->fd_small == MAYBE)
*maybe_smallp = TRUE;
if (closed)
{
if (*maybe_smallp)
fp->fd_small = MAYBE;
checkSmall(win, fp, lnum_off);
if (fp->fd_small == TRUE)
closed = FALSE;
}
return closed;
}
static void
checkSmall(
win_T *wp,
fold_T *fp,
linenr_T lnum_off) 
{
int count;
int n;
if (fp->fd_small == MAYBE)
{
setSmallMaybe(&fp->fd_nested);
if (fp->fd_len > curwin->w_p_fml)
fp->fd_small = FALSE;
else
{
count = 0;
for (n = 0; n < fp->fd_len; ++n)
{
count += plines_win_nofold(wp, fp->fd_top + lnum_off + n);
if (count > curwin->w_p_fml)
{
fp->fd_small = FALSE;
return;
}
}
fp->fd_small = TRUE;
}
}
}
static void
setSmallMaybe(garray_T *gap)
{
int i;
fold_T *fp;
fp = (fold_T *)gap->ga_data;
for (i = 0; i < gap->ga_len; ++i)
fp[i].fd_small = MAYBE;
}
static void
foldCreateMarkers(linenr_T start, linenr_T end)
{
if (!curbuf->b_p_ma)
{
emsg(_(e_modifiable));
return;
}
parseMarker(curwin);
foldAddMarker(start, curwin->w_p_fmr, foldstartmarkerlen);
foldAddMarker(end, foldendmarker, foldendmarkerlen);
changed_lines(start, (colnr_T)0, end, 0L);
}
static void
foldAddMarker(linenr_T lnum, char_u *marker, int markerlen)
{
char_u *cms = curbuf->b_p_cms;
char_u *line;
int line_len;
char_u *newline;
char_u *p = (char_u *)strstr((char *)curbuf->b_p_cms, "%s");
int line_is_comment = FALSE;
line = ml_get(lnum);
line_len = (int)STRLEN(line);
if (u_save(lnum - 1, lnum + 1) == OK)
{
(void)skip_comment(line, FALSE, FALSE, &line_is_comment);
newline = alloc(line_len + markerlen + STRLEN(cms) + 1);
if (newline == NULL)
return;
STRCPY(newline, line);
if (p == NULL || line_is_comment)
vim_strncpy(newline + line_len, marker, markerlen);
else
{
STRCPY(newline + line_len, cms);
STRNCPY(newline + line_len + (p - cms), marker, markerlen);
STRCPY(newline + line_len + (p - cms) + markerlen, p + 2);
}
ml_replace(lnum, newline, FALSE);
}
}
static void
deleteFoldMarkers(
fold_T *fp,
int recursive,
linenr_T lnum_off) 
{
int i;
if (recursive)
for (i = 0; i < fp->fd_nested.ga_len; ++i)
deleteFoldMarkers((fold_T *)fp->fd_nested.ga_data + i, TRUE,
lnum_off + fp->fd_top);
foldDelMarker(fp->fd_top + lnum_off, curwin->w_p_fmr, foldstartmarkerlen);
foldDelMarker(fp->fd_top + lnum_off + fp->fd_len - 1,
foldendmarker, foldendmarkerlen);
}
static void
foldDelMarker(linenr_T lnum, char_u *marker, int markerlen)
{
char_u *line;
char_u *newline;
char_u *p;
int len;
char_u *cms = curbuf->b_p_cms;
char_u *cms2;
if (lnum > curbuf->b_ml.ml_line_count)
return;
line = ml_get(lnum);
for (p = line; *p != NUL; ++p)
if (STRNCMP(p, marker, markerlen) == 0)
{
len = markerlen;
if (VIM_ISDIGIT(p[len]))
++len;
if (*cms != NUL)
{
cms2 = (char_u *)strstr((char *)cms, "%s");
if (p - line >= cms2 - cms
&& STRNCMP(p - (cms2 - cms), cms, cms2 - cms) == 0
&& STRNCMP(p + len, cms2 + 2, STRLEN(cms2 + 2)) == 0)
{
p -= cms2 - cms;
len += (int)STRLEN(cms) - 2;
}
}
if (u_save(lnum - 1, lnum + 1) == OK)
{
newline = alloc(STRLEN(line) - len + 1);
if (newline != NULL)
{
STRNCPY(newline, line, p - line);
STRCPY(newline + (p - line), p + len);
ml_replace(lnum, newline, FALSE);
}
}
break;
}
}
char_u *
get_foldtext(
win_T *wp,
linenr_T lnum,
linenr_T lnume,
foldinfo_T *foldinfo,
char_u *buf)
{
char_u *text = NULL;
#if defined(FEAT_EVAL)
static int got_fdt_error = FALSE;
int save_did_emsg = did_emsg;
static win_T *last_wp = NULL;
static linenr_T last_lnum = 0;
if (last_wp != wp || last_wp == NULL
|| last_lnum > lnum || last_lnum == 0)
got_fdt_error = FALSE;
if (!got_fdt_error)
did_emsg = FALSE;
if (*wp->w_p_fdt != NUL)
{
char_u dashes[MAX_LEVEL + 2];
win_T *save_curwin;
int level;
char_u *p;
set_vim_var_nr(VV_FOLDSTART, lnum);
set_vim_var_nr(VV_FOLDEND, lnume);
level = foldinfo->fi_level;
if (level > (int)sizeof(dashes) - 1)
level = (int)sizeof(dashes) - 1;
vim_memset(dashes, '-', (size_t)level);
dashes[level] = NUL;
set_vim_var_string(VV_FOLDDASHES, dashes, -1);
set_vim_var_nr(VV_FOLDLEVEL, (long)level);
if (!got_fdt_error)
{
save_curwin = curwin;
curwin = wp;
curbuf = wp->w_buffer;
++emsg_silent; 
text = eval_to_string_safe(wp->w_p_fdt, NULL,
was_set_insecurely((char_u *)"foldtext", OPT_LOCAL));
--emsg_silent;
if (text == NULL || did_emsg)
got_fdt_error = TRUE;
curwin = save_curwin;
curbuf = curwin->w_buffer;
}
last_lnum = lnum;
last_wp = wp;
set_vim_var_string(VV_FOLDDASHES, NULL, -1);
if (!did_emsg && save_did_emsg)
did_emsg = save_did_emsg;
if (text != NULL)
{
for (p = text; *p != NUL; ++p)
{
int len;
if (has_mbyte && (len = (*mb_ptr2len)(p)) > 1)
{
if (!vim_isprintc((*mb_ptr2char)(p)))
break;
p += len - 1;
}
else
if (*p == TAB)
*p = ' ';
else if (ptr2cells(p) > 1)
break;
}
if (*p != NUL)
{
p = transstr(text);
vim_free(text);
text = p;
}
}
}
if (text == NULL)
#endif
{
long count = (long)(lnume - lnum + 1);
vim_snprintf((char *)buf, FOLD_TEXT_LEN,
NGETTEXT("+--%3ld line folded ",
"+--%3ld lines folded ", count),
count);
text = buf;
}
return text;
}
#if defined(FEAT_EVAL)
static void
foldtext_cleanup(char_u *str)
{
char_u *cms_start; 
int cms_slen = 0; 
char_u *cms_end; 
int cms_elen = 0; 
char_u *s;
char_u *p;
int len;
int did1 = FALSE;
int did2 = FALSE;
cms_start = skipwhite(curbuf->b_p_cms);
cms_slen = (int)STRLEN(cms_start);
while (cms_slen > 0 && VIM_ISWHITE(cms_start[cms_slen - 1]))
--cms_slen;
cms_end = (char_u *)strstr((char *)cms_start, "%s");
if (cms_end != NULL)
{
cms_elen = cms_slen - (int)(cms_end - cms_start);
cms_slen = (int)(cms_end - cms_start);
while (cms_slen > 0 && VIM_ISWHITE(cms_start[cms_slen - 1]))
--cms_slen;
s = skipwhite(cms_end + 2);
cms_elen -= (int)(s - cms_end);
cms_end = s;
}
parseMarker(curwin);
for (s = str; *s != NUL; )
{
len = 0;
if (STRNCMP(s, curwin->w_p_fmr, foldstartmarkerlen) == 0)
len = foldstartmarkerlen;
else if (STRNCMP(s, foldendmarker, foldendmarkerlen) == 0)
len = foldendmarkerlen;
if (len > 0)
{
if (VIM_ISDIGIT(s[len]))
++len;
for (p = s; p > str && VIM_ISWHITE(p[-1]); --p)
;
if (p >= str + cms_slen
&& STRNCMP(p - cms_slen, cms_start, cms_slen) == 0)
{
len += (int)(s - p) + cms_slen;
s = p - cms_slen;
}
}
else if (cms_end != NULL)
{
if (!did1 && cms_slen > 0 && STRNCMP(s, cms_start, cms_slen) == 0)
{
len = cms_slen;
did1 = TRUE;
}
else if (!did2 && cms_elen > 0
&& STRNCMP(s, cms_end, cms_elen) == 0)
{
len = cms_elen;
did2 = TRUE;
}
}
if (len != 0)
{
while (VIM_ISWHITE(s[len]))
++len;
STRMOVE(s, s + len);
}
else
{
MB_PTR_ADV(s);
}
}
}
#endif
typedef struct
{
win_T *wp; 
linenr_T lnum; 
linenr_T off; 
linenr_T lnum_save; 
int lvl; 
int lvl_next; 
int start; 
int end; 
int had_end; 
} fline_T;
static int fold_changed;
static linenr_T foldUpdateIEMSRecurse(garray_T *gap, int level, linenr_T startlnum, fline_T *flp, void (*getlevel)(fline_T *), linenr_T bot, int topflags);
static int foldInsert(garray_T *gap, int i);
static void foldSplit(garray_T *gap, int i, linenr_T top, linenr_T bot);
static void foldRemove(garray_T *gap, linenr_T top, linenr_T bot);
static void foldMerge(fold_T *fp1, garray_T *gap, fold_T *fp2);
static void foldlevelIndent(fline_T *flp);
#if defined(FEAT_DIFF)
static void foldlevelDiff(fline_T *flp);
#endif
static void foldlevelExpr(fline_T *flp);
static void foldlevelMarker(fline_T *flp);
static void foldlevelSyntax(fline_T *flp);
static void
foldUpdateIEMS(win_T *wp, linenr_T top, linenr_T bot)
{
linenr_T start;
linenr_T end;
fline_T fline;
void (*getlevel)(fline_T *);
int level;
fold_T *fp;
if (invalid_top != (linenr_T)0)
return;
if (wp->w_foldinvalid)
{
top = 1;
bot = wp->w_buffer->b_ml.ml_line_count;
wp->w_foldinvalid = FALSE;
setSmallMaybe(&wp->w_folds);
}
#if defined(FEAT_DIFF)
if (foldmethodIsDiff(wp))
{
if (top > diff_context)
top -= diff_context;
else
top = 1;
bot += diff_context;
}
#endif
if (top > wp->w_buffer->b_ml.ml_line_count)
top = wp->w_buffer->b_ml.ml_line_count;
fold_changed = FALSE;
fline.wp = wp;
fline.off = 0;
fline.lvl = 0;
fline.lvl_next = -1;
fline.start = 0;
fline.end = MAX_LEVEL + 1;
fline.had_end = MAX_LEVEL + 1;
invalid_top = top;
invalid_bot = bot;
if (foldmethodIsMarker(wp))
{
getlevel = foldlevelMarker;
parseMarker(wp);
if (top > 1)
{
level = foldLevelWin(wp, top - 1);
fline.lnum = top - 1;
fline.lvl = level;
getlevel(&fline);
if (fline.lvl > level)
fline.lvl = level - (fline.lvl - fline.lvl_next);
else
fline.lvl = fline.lvl_next;
}
fline.lnum = top;
getlevel(&fline);
}
else
{
fline.lnum = top;
if (foldmethodIsExpr(wp))
{
getlevel = foldlevelExpr;
if (top > 1)
--fline.lnum;
}
else if (foldmethodIsSyntax(wp))
getlevel = foldlevelSyntax;
#if defined(FEAT_DIFF)
else if (foldmethodIsDiff(wp))
getlevel = foldlevelDiff;
#endif
else
getlevel = foldlevelIndent;
fline.lvl = -1;
for ( ; !got_int; --fline.lnum)
{
fline.lvl_next = -1;
getlevel(&fline);
if (fline.lvl >= 0)
break;
}
}
if (foldlevelSyntax == getlevel)
{
garray_T *gap = &wp->w_folds;
fold_T *fpn = NULL;
int current_fdl = 0;
linenr_T fold_start_lnum = 0;
linenr_T lnum_rel = fline.lnum;
while (current_fdl < fline.lvl)
{
if (!foldFind(gap, lnum_rel, &fpn))
break;
++current_fdl;
fold_start_lnum += fpn->fd_top;
gap = &fpn->fd_nested;
lnum_rel -= fpn->fd_top;
}
if (fpn != NULL && current_fdl == fline.lvl)
{
linenr_T fold_end_lnum = fold_start_lnum + fpn->fd_len;
if (fold_end_lnum > bot)
bot = fold_end_lnum;
}
}
start = fline.lnum;
end = bot;
if (start > end && end < wp->w_buffer->b_ml.ml_line_count)
end = start;
while (!got_int)
{
if (fline.lnum > wp->w_buffer->b_ml.ml_line_count)
break;
if (fline.lnum > end)
{
if (getlevel != foldlevelMarker
&& getlevel != foldlevelSyntax
&& getlevel != foldlevelExpr)
break;
if ((start <= end
&& foldFind(&wp->w_folds, end, &fp)
&& fp->fd_top + fp->fd_len - 1 > end)
|| (fline.lvl == 0
&& foldFind(&wp->w_folds, fline.lnum, &fp)
&& fp->fd_top < fline.lnum))
end = fp->fd_top + fp->fd_len - 1;
else if (getlevel == foldlevelSyntax
&& foldLevelWin(wp, fline.lnum) != fline.lvl)
end = fline.lnum;
else
break;
}
if (fline.lvl > 0)
{
invalid_top = fline.lnum;
invalid_bot = end;
end = foldUpdateIEMSRecurse(&wp->w_folds,
1, start, &fline, getlevel, end, FD_LEVEL);
start = fline.lnum;
}
else
{
if (fline.lnum == wp->w_buffer->b_ml.ml_line_count)
break;
++fline.lnum;
fline.lvl = fline.lvl_next;
getlevel(&fline);
}
}
foldRemove(&wp->w_folds, start, end);
if (fold_changed && wp->w_p_fen)
changed_window_setting_win(wp);
if (end != bot)
{
if (wp->w_redraw_top == 0 || wp->w_redraw_top > top)
wp->w_redraw_top = top;
if (wp->w_redraw_bot < end)
wp->w_redraw_bot = end;
}
invalid_top = (linenr_T)0;
}
static linenr_T
foldUpdateIEMSRecurse(
garray_T *gap,
int level,
linenr_T startlnum,
fline_T *flp,
void (*getlevel)(fline_T *),
linenr_T bot,
int topflags) 
{
linenr_T ll;
fold_T *fp = NULL;
fold_T *fp2;
int lvl = level;
linenr_T startlnum2 = startlnum;
linenr_T firstlnum = flp->lnum; 
int i;
int finish = FALSE;
linenr_T linecount = flp->wp->w_buffer->b_ml.ml_line_count - flp->off;
int concat;
if (getlevel == foldlevelMarker && flp->start <= flp->lvl - level
&& flp->lvl > 0)
{
(void)foldFind(gap, startlnum - 1, &fp);
if (fp >= ((fold_T *)gap->ga_data) + gap->ga_len
|| fp->fd_top >= startlnum)
fp = NULL;
}
flp->lnum_save = flp->lnum;
while (!got_int)
{
line_breakcheck();
lvl = flp->lvl;
if (lvl > MAX_LEVEL)
lvl = MAX_LEVEL;
if (flp->lnum > firstlnum
&& (level > lvl - flp->start || level >= flp->had_end))
lvl = 0;
if (flp->lnum > bot && !finish && fp != NULL)
{
if (getlevel != foldlevelMarker
&& getlevel != foldlevelExpr
&& getlevel != foldlevelSyntax)
break;
i = 0;
fp2 = fp;
if (lvl >= level)
{
ll = flp->lnum - fp->fd_top;
while (foldFind(&fp2->fd_nested, ll, &fp2))
{
++i;
ll -= fp2->fd_top;
}
}
if (lvl < level + i)
{
(void)foldFind(&fp->fd_nested, flp->lnum - fp->fd_top, &fp2);
if (fp2 != NULL)
bot = fp2->fd_top + fp2->fd_len - 1 + fp->fd_top;
}
else if (fp->fd_top + fp->fd_len <= flp->lnum && lvl >= level)
finish = TRUE;
else
break;
}
if (fp == NULL
&& (lvl != level
|| flp->lnum_save >= bot
|| flp->start != 0
|| flp->had_end <= MAX_LEVEL
|| flp->lnum == linecount))
{
while (!got_int)
{
if (flp->start != 0 || flp->had_end <= MAX_LEVEL)
concat = 0;
else
concat = 1;
if (foldFind(gap, startlnum, &fp)
|| (fp < ((fold_T *)gap->ga_data) + gap->ga_len
&& fp->fd_top <= firstlnum)
|| foldFind(gap, firstlnum - concat, &fp)
|| (fp < ((fold_T *)gap->ga_data) + gap->ga_len
&& ((lvl < level && fp->fd_top < flp->lnum)
|| (lvl >= level
&& fp->fd_top <= flp->lnum_save))))
{
if (fp->fd_top + fp->fd_len + concat > firstlnum)
{
if (fp->fd_top == firstlnum)
{
}
else if (fp->fd_top >= startlnum)
{
if (fp->fd_top > firstlnum)
foldMarkAdjustRecurse(&fp->fd_nested,
(linenr_T)0, (linenr_T)MAXLNUM,
(long)(fp->fd_top - firstlnum), 0L);
else
foldMarkAdjustRecurse(&fp->fd_nested,
(linenr_T)0,
(long)(firstlnum - fp->fd_top - 1),
(linenr_T)MAXLNUM,
(long)(fp->fd_top - firstlnum));
fp->fd_len += fp->fd_top - firstlnum;
fp->fd_top = firstlnum;
fold_changed = TRUE;
}
else if ((flp->start != 0 && lvl == level)
|| firstlnum != startlnum)
{
linenr_T breakstart;
linenr_T breakend;
if (firstlnum != startlnum)
{
breakstart = startlnum;
breakend = firstlnum;
}
else
{
breakstart = flp->lnum;
breakend = flp->lnum;
}
foldRemove(&fp->fd_nested, breakstart - fp->fd_top,
breakend - fp->fd_top);
i = (int)(fp - (fold_T *)gap->ga_data);
foldSplit(gap, i, breakstart, breakend - 1);
fp = (fold_T *)gap->ga_data + i + 1;
if (getlevel == foldlevelMarker
|| getlevel == foldlevelExpr
|| getlevel == foldlevelSyntax)
finish = TRUE;
}
if (fp->fd_top == startlnum && concat)
{
i = (int)(fp - (fold_T *)gap->ga_data);
if (i != 0)
{
fp2 = fp - 1;
if (fp2->fd_top + fp2->fd_len == fp->fd_top)
{
foldMerge(fp2, gap, fp);
fp = fp2;
}
}
}
break;
}
if (fp->fd_top >= startlnum)
{
deleteFoldEntry(gap,
(int)(fp - (fold_T *)gap->ga_data), TRUE);
}
else
{
fp->fd_len = startlnum - fp->fd_top;
foldMarkAdjustRecurse(&fp->fd_nested,
(linenr_T)fp->fd_len, (linenr_T)MAXLNUM,
(linenr_T)MAXLNUM, 0L);
fold_changed = TRUE;
}
}
else
{
i = (int)(fp - (fold_T *)gap->ga_data);
if (foldInsert(gap, i) != OK)
return bot;
fp = (fold_T *)gap->ga_data + i;
fp->fd_top = firstlnum;
fp->fd_len = bot - firstlnum + 1;
if (topflags == FD_OPEN)
{
flp->wp->w_fold_manual = TRUE;
fp->fd_flags = FD_OPEN;
}
else if (i <= 0)
{
fp->fd_flags = topflags;
if (topflags != FD_LEVEL)
flp->wp->w_fold_manual = TRUE;
}
else
fp->fd_flags = (fp - 1)->fd_flags;
fp->fd_small = MAYBE;
if (getlevel == foldlevelMarker
|| getlevel == foldlevelExpr
|| getlevel == foldlevelSyntax)
finish = TRUE;
fold_changed = TRUE;
break;
}
}
}
if (lvl < level || flp->lnum > linecount)
{
break;
}
if (lvl > level && fp != NULL)
{
if (bot < flp->lnum)
bot = flp->lnum;
flp->lnum = flp->lnum_save - fp->fd_top;
flp->off += fp->fd_top;
i = (int)(fp - (fold_T *)gap->ga_data);
bot = foldUpdateIEMSRecurse(&fp->fd_nested, level + 1,
startlnum2 - fp->fd_top, flp, getlevel,
bot - fp->fd_top, fp->fd_flags);
fp = (fold_T *)gap->ga_data + i;
flp->lnum += fp->fd_top;
flp->lnum_save += fp->fd_top;
flp->off -= fp->fd_top;
bot += fp->fd_top;
startlnum2 = flp->lnum;
}
else
{
flp->lnum = flp->lnum_save;
ll = flp->lnum + 1;
while (!got_int)
{
prev_lnum = flp->lnum;
prev_lnum_lvl = flp->lvl;
if (++flp->lnum > linecount)
break;
flp->lvl = flp->lvl_next;
getlevel(flp);
if (flp->lvl >= 0 || flp->had_end <= MAX_LEVEL)
break;
}
prev_lnum = 0;
if (flp->lnum > linecount)
break;
flp->lnum_save = flp->lnum;
flp->lnum = ll;
}
}
if (fp == NULL) 
return bot;
if (fp->fd_len < flp->lnum - fp->fd_top)
{
fp->fd_len = flp->lnum - fp->fd_top;
fp->fd_small = MAYBE;
fold_changed = TRUE;
}
else if (fp->fd_top + fp->fd_len > linecount)
fp->fd_len = linecount - fp->fd_top + 1;
foldRemove(&fp->fd_nested, startlnum2 - fp->fd_top,
flp->lnum - 1 - fp->fd_top);
if (lvl < level)
{
if (fp->fd_len != flp->lnum - fp->fd_top)
{
if (fp->fd_top + fp->fd_len - 1 > bot)
{
if (getlevel == foldlevelMarker
|| getlevel == foldlevelExpr
|| getlevel == foldlevelSyntax)
{
bot = fp->fd_top + fp->fd_len - 1;
fp->fd_len = flp->lnum - fp->fd_top;
}
else
{
i = (int)(fp - (fold_T *)gap->ga_data);
foldSplit(gap, i, flp->lnum, bot);
fp = (fold_T *)gap->ga_data + i;
}
}
else
fp->fd_len = flp->lnum - fp->fd_top;
fold_changed = TRUE;
}
}
for (;;)
{
fp2 = fp + 1;
if (fp2 >= (fold_T *)gap->ga_data + gap->ga_len
|| fp2->fd_top > flp->lnum)
break;
if (fp2->fd_top + fp2->fd_len > flp->lnum)
{
if (fp2->fd_top < flp->lnum)
{
foldMarkAdjustRecurse(&fp2->fd_nested,
(linenr_T)0, (long)(flp->lnum - fp2->fd_top - 1),
(linenr_T)MAXLNUM, (long)(fp2->fd_top - flp->lnum));
fp2->fd_len -= flp->lnum - fp2->fd_top;
fp2->fd_top = flp->lnum;
fold_changed = TRUE;
}
if (lvl >= level)
{
foldMerge(fp, gap, fp2);
}
break;
}
fold_changed = TRUE;
deleteFoldEntry(gap, (int)(fp2 - (fold_T *)gap->ga_data), TRUE);
}
if (bot < flp->lnum - 1)
bot = flp->lnum - 1;
return bot;
}
static int
foldInsert(garray_T *gap, int i)
{
fold_T *fp;
if (ga_grow(gap, 1) != OK)
return FAIL;
fp = (fold_T *)gap->ga_data + i;
if (i < gap->ga_len)
mch_memmove(fp + 1, fp, sizeof(fold_T) * (gap->ga_len - i));
++gap->ga_len;
ga_init2(&fp->fd_nested, (int)sizeof(fold_T), 10);
return OK;
}
static void
foldSplit(
garray_T *gap,
int i,
linenr_T top,
linenr_T bot)
{
fold_T *fp;
fold_T *fp2;
garray_T *gap1;
garray_T *gap2;
int idx;
int len;
if (foldInsert(gap, i + 1) == FAIL)
return;
fp = (fold_T *)gap->ga_data + i;
fp[1].fd_top = bot + 1;
fp[1].fd_len = fp->fd_len - (fp[1].fd_top - fp->fd_top);
fp[1].fd_flags = fp->fd_flags;
fp[1].fd_small = MAYBE;
fp->fd_small = MAYBE;
gap1 = &fp->fd_nested;
gap2 = &fp[1].fd_nested;
(void)(foldFind(gap1, bot + 1 - fp->fd_top, &fp2));
len = (int)((fold_T *)gap1->ga_data + gap1->ga_len - fp2);
if (len > 0 && ga_grow(gap2, len) == OK)
{
for (idx = 0; idx < len; ++idx)
{
((fold_T *)gap2->ga_data)[idx] = fp2[idx];
((fold_T *)gap2->ga_data)[idx].fd_top
-= fp[1].fd_top - fp->fd_top;
}
gap2->ga_len = len;
gap1->ga_len -= len;
}
fp->fd_len = top - fp->fd_top;
fold_changed = TRUE;
}
static void
foldRemove(garray_T *gap, linenr_T top, linenr_T bot)
{
fold_T *fp = NULL;
if (bot < top)
return; 
for (;;)
{
if (foldFind(gap, top, &fp) && fp->fd_top < top)
{
foldRemove(&fp->fd_nested, top - fp->fd_top, bot - fp->fd_top);
if (fp->fd_top + fp->fd_len - 1 > bot)
{
foldSplit(gap, (int)(fp - (fold_T *)gap->ga_data), top, bot);
}
else
{
fp->fd_len = top - fp->fd_top;
}
fold_changed = TRUE;
continue;
}
if (fp >= (fold_T *)(gap->ga_data) + gap->ga_len
|| fp->fd_top > bot)
{
break;
}
if (fp->fd_top >= top)
{
fold_changed = TRUE;
if (fp->fd_top + fp->fd_len - 1 > bot)
{
foldMarkAdjustRecurse(&fp->fd_nested,
(linenr_T)0, (long)(bot - fp->fd_top),
(linenr_T)MAXLNUM, (long)(fp->fd_top - bot - 1));
fp->fd_len -= bot - fp->fd_top + 1;
fp->fd_top = bot + 1;
break;
}
deleteFoldEntry(gap, (int)(fp - (fold_T *)gap->ga_data), TRUE);
}
}
}
static void
foldReverseOrder(garray_T *gap, linenr_T start_arg, linenr_T end_arg)
{
fold_T *left, *right;
fold_T tmp;
linenr_T start = start_arg;
linenr_T end = end_arg;
for (; start < end; start++, end--)
{
left = (fold_T *)gap->ga_data + start;
right = (fold_T *)gap->ga_data + end;
tmp = *left;
*left = *right;
*right = tmp;
}
}
static void
truncate_fold(fold_T *fp, linenr_T end)
{
end += 1;
foldRemove(&fp->fd_nested, end - fp->fd_top, MAXLNUM);
fp->fd_len = end - fp->fd_top;
}
#define fold_end(fp) ((fp)->fd_top + (fp)->fd_len - 1)
#define valid_fold(fp, gap) ((fp) < ((fold_T *)(gap)->ga_data + (gap)->ga_len))
#define fold_index(fp, gap) ((size_t)(fp - ((fold_T *)(gap)->ga_data)))
void
foldMoveRange(garray_T *gap, linenr_T line1, linenr_T line2, linenr_T dest)
{
fold_T *fp;
linenr_T range_len = line2 - line1 + 1;
linenr_T move_len = dest - line2;
int at_start = foldFind(gap, line1 - 1, &fp);
size_t move_start = 0, move_end = 0, dest_index = 0;
if (at_start)
{
if (fold_end(fp) > dest)
{
foldMoveRange(&fp->fd_nested, line1 - fp->fd_top, line2 -
fp->fd_top, dest - fp->fd_top);
return;
}
else if (fold_end(fp) > line2)
{
foldMarkAdjustRecurse(&fp->fd_nested, line1 - fp->fd_top, line2 -
fp->fd_top, MAXLNUM, -range_len);
fp->fd_len -= range_len;
}
else
truncate_fold(fp, line1 - 1);
fp = fp + 1;
}
if (!valid_fold(fp, gap) || fp->fd_top > dest)
{
return;
}
else if (fp->fd_top > line2)
{
for (; valid_fold(fp, gap) && fold_end(fp) <= dest; fp++)
fp->fd_top -= range_len;
if (valid_fold(fp, gap) && fp->fd_top <= dest)
{
truncate_fold(fp, dest);
fp->fd_top -= range_len;
}
return;
}
else if (fold_end(fp) > dest)
{
foldMarkAdjustRecurse(&fp->fd_nested, line2 + 1 - fp->fd_top, dest -
fp->fd_top, MAXLNUM, -move_len);
fp->fd_len -= move_len;
fp->fd_top += move_len;
return;
}
move_start = fold_index(fp, gap);
for (; valid_fold(fp, gap) && fp->fd_top <= dest; fp++)
{
if (fp->fd_top <= line2)
{
if (fold_end(fp) > line2)
truncate_fold(fp, line2);
fp->fd_top += move_len;
continue;
}
if (move_end == 0)
move_end = fold_index(fp, gap);
if (fold_end(fp) > dest)
truncate_fold(fp, dest);
fp->fd_top -= range_len;
}
dest_index = fold_index(fp, gap);
if (move_end == 0)
return;
foldReverseOrder(gap, (linenr_T)move_start, (linenr_T)dest_index - 1);
foldReverseOrder(gap, (linenr_T)move_start,
(linenr_T)(move_start + dest_index - move_end - 1));
foldReverseOrder(gap, (linenr_T)(move_start + dest_index - move_end),
(linenr_T)(dest_index - 1));
}
#undef fold_end
#undef valid_fold
#undef fold_index
static void
foldMerge(fold_T *fp1, garray_T *gap, fold_T *fp2)
{
fold_T *fp3;
fold_T *fp4;
int idx;
garray_T *gap1 = &fp1->fd_nested;
garray_T *gap2 = &fp2->fd_nested;
if (foldFind(gap1, fp1->fd_len - 1L, &fp3) && foldFind(gap2, 0L, &fp4))
foldMerge(fp3, gap2, fp4);
if (gap2->ga_len > 0 && ga_grow(gap1, gap2->ga_len) == OK)
{
for (idx = 0; idx < gap2->ga_len; ++idx)
{
((fold_T *)gap1->ga_data)[gap1->ga_len]
= ((fold_T *)gap2->ga_data)[idx];
((fold_T *)gap1->ga_data)[gap1->ga_len].fd_top += fp1->fd_len;
++gap1->ga_len;
}
gap2->ga_len = 0;
}
fp1->fd_len += fp2->fd_len;
deleteFoldEntry(gap, (int)(fp2 - (fold_T *)gap->ga_data), TRUE);
fold_changed = TRUE;
}
static void
foldlevelIndent(fline_T *flp)
{
char_u *s;
buf_T *buf;
linenr_T lnum = flp->lnum + flp->off;
buf = flp->wp->w_buffer;
s = skipwhite(ml_get_buf(buf, lnum, FALSE));
if (*s == NUL || vim_strchr(flp->wp->w_p_fdi, *s) != NULL)
{
if (lnum == 1 || lnum == buf->b_ml.ml_line_count)
flp->lvl = 0;
else
flp->lvl = -1;
}
else
flp->lvl = get_indent_buf(buf, lnum) / get_sw_value(buf);
if (flp->lvl > flp->wp->w_p_fdn)
{
flp->lvl = flp->wp->w_p_fdn;
if (flp->lvl < 0)
flp->lvl = 0;
}
}
#if defined(FEAT_DIFF)
static void
foldlevelDiff(fline_T *flp)
{
if (diff_infold(flp->wp, flp->lnum + flp->off))
flp->lvl = 1;
else
flp->lvl = 0;
}
#endif
static void
foldlevelExpr(fline_T *flp)
{
#if !defined(FEAT_EVAL)
flp->start = FALSE;
flp->lvl = 0;
#else
win_T *win;
int n;
int c;
linenr_T lnum = flp->lnum + flp->off;
int save_keytyped;
win = curwin;
curwin = flp->wp;
curbuf = flp->wp->w_buffer;
set_vim_var_nr(VV_LNUM, lnum);
flp->start = 0;
flp->had_end = flp->end;
flp->end = MAX_LEVEL + 1;
if (lnum <= 1)
flp->lvl = 0;
save_keytyped = KeyTyped;
n = (int)eval_foldexpr(flp->wp->w_p_fde, &c);
KeyTyped = save_keytyped;
switch (c)
{
case 'a': if (flp->lvl >= 0)
{
flp->lvl += n;
flp->lvl_next = flp->lvl;
}
flp->start = n;
break;
case 's': if (flp->lvl >= 0)
{
if (n > flp->lvl)
flp->lvl_next = 0;
else
flp->lvl_next = flp->lvl - n;
flp->end = flp->lvl_next + 1;
}
break;
case '>': flp->lvl = n;
flp->lvl_next = n;
flp->start = 1;
break;
case '<': flp->lvl_next = n - 1;
flp->end = n;
break;
case '=': flp->lvl_next = flp->lvl;
break;
default: if (n < 0)
flp->lvl_next = flp->lvl;
else
flp->lvl_next = n;
flp->lvl = n;
break;
}
if (flp->lvl < 0)
{
if (lnum <= 1)
{
flp->lvl = 0;
flp->lvl_next = 0;
}
if (lnum == curbuf->b_ml.ml_line_count)
flp->lvl_next = 0;
}
curwin = win;
curbuf = curwin->w_buffer;
#endif
}
static void
parseMarker(win_T *wp)
{
foldendmarker = vim_strchr(wp->w_p_fmr, ',');
foldstartmarkerlen = (int)(foldendmarker++ - wp->w_p_fmr);
foldendmarkerlen = (int)STRLEN(foldendmarker);
}
static void
foldlevelMarker(fline_T *flp)
{
char_u *startmarker;
int cstart;
int cend;
int start_lvl = flp->lvl;
char_u *s;
int n;
startmarker = flp->wp->w_p_fmr;
cstart = *startmarker;
++startmarker;
cend = *foldendmarker;
flp->start = 0;
flp->lvl_next = flp->lvl;
s = ml_get_buf(flp->wp->w_buffer, flp->lnum + flp->off, FALSE);
while (*s)
{
if (*s == cstart
&& STRNCMP(s + 1, startmarker, foldstartmarkerlen - 1) == 0)
{
s += foldstartmarkerlen;
if (VIM_ISDIGIT(*s))
{
n = atoi((char *)s);
if (n > 0)
{
flp->lvl = n;
flp->lvl_next = n;
if (n <= start_lvl)
flp->start = 1;
else
flp->start = n - start_lvl;
}
}
else
{
++flp->lvl;
++flp->lvl_next;
++flp->start;
}
}
else if (*s == cend
&& STRNCMP(s + 1, foldendmarker + 1, foldendmarkerlen - 1) == 0)
{
s += foldendmarkerlen;
if (VIM_ISDIGIT(*s))
{
n = atoi((char *)s);
if (n > 0)
{
flp->lvl = n;
flp->lvl_next = n - 1;
if (flp->lvl_next > start_lvl)
flp->lvl_next = start_lvl;
}
}
else
--flp->lvl_next;
}
else
MB_PTR_ADV(s);
}
if (flp->lvl_next < 0)
flp->lvl_next = 0;
}
static void
foldlevelSyntax(fline_T *flp)
{
#if !defined(FEAT_SYN_HL)
flp->start = 0;
flp->lvl = 0;
#else
linenr_T lnum = flp->lnum + flp->off;
int n;
flp->lvl = syn_get_foldlevel(flp->wp, lnum);
flp->start = 0;
if (lnum < flp->wp->w_buffer->b_ml.ml_line_count)
{
n = syn_get_foldlevel(flp->wp, lnum + 1);
if (n > flp->lvl)
{
flp->start = n - flp->lvl; 
flp->lvl = n;
}
}
#endif
}
#if defined(FEAT_SESSION) || defined(PROTO)
static int put_folds_recurse(FILE *fd, garray_T *gap, linenr_T off);
static int put_foldopen_recurse(FILE *fd, win_T *wp, garray_T *gap, linenr_T off);
static int put_fold_open_close(FILE *fd, fold_T *fp, linenr_T off);
int
put_folds(FILE *fd, win_T *wp)
{
if (foldmethodIsManual(wp))
{
if (put_line(fd, "silent! normal! zE") == FAIL
|| put_folds_recurse(fd, &wp->w_folds, (linenr_T)0) == FAIL)
return FAIL;
}
if (wp->w_fold_manual)
return put_foldopen_recurse(fd, wp, &wp->w_folds, (linenr_T)0);
return OK;
}
static int
put_folds_recurse(FILE *fd, garray_T *gap, linenr_T off)
{
int i;
fold_T *fp;
fp = (fold_T *)gap->ga_data;
for (i = 0; i < gap->ga_len; i++)
{
if (put_folds_recurse(fd, &fp->fd_nested, off + fp->fd_top) == FAIL)
return FAIL;
if (fprintf(fd, "%ld,%ldfold", fp->fd_top + off,
fp->fd_top + off + fp->fd_len - 1) < 0
|| put_eol(fd) == FAIL)
return FAIL;
++fp;
}
return OK;
}
static int
put_foldopen_recurse(
FILE *fd,
win_T *wp,
garray_T *gap,
linenr_T off)
{
int i;
int level;
fold_T *fp;
fp = (fold_T *)gap->ga_data;
for (i = 0; i < gap->ga_len; i++)
{
if (fp->fd_flags != FD_LEVEL)
{
if (fp->fd_nested.ga_len > 0)
{
if (fprintf(fd, "%ld", fp->fd_top + off) < 0
|| put_eol(fd) == FAIL
|| put_line(fd, "normal! zo") == FAIL)
return FAIL;
if (put_foldopen_recurse(fd, wp, &fp->fd_nested,
off + fp->fd_top)
== FAIL)
return FAIL;
if (fp->fd_flags == FD_CLOSED)
{
if (put_fold_open_close(fd, fp, off) == FAIL)
return FAIL;
}
}
else
{
level = foldLevelWin(wp, off + fp->fd_top);
if ((fp->fd_flags == FD_CLOSED && wp->w_p_fdl >= level)
|| (fp->fd_flags != FD_CLOSED && wp->w_p_fdl < level))
if (put_fold_open_close(fd, fp, off) == FAIL)
return FAIL;
}
}
++fp;
}
return OK;
}
static int
put_fold_open_close(FILE *fd, fold_T *fp, linenr_T off)
{
if (fprintf(fd, "%ld", fp->fd_top + off) < 0
|| put_eol(fd) == FAIL
|| fprintf(fd, "normal! z%c",
fp->fd_flags == FD_CLOSED ? 'c' : 'o') < 0
|| put_eol(fd) == FAIL)
return FAIL;
return OK;
}
#endif 
#endif 
#if defined(FEAT_EVAL) || defined(PROTO)
static void
foldclosed_both(
typval_T *argvars UNUSED,
typval_T *rettv,
int end UNUSED)
{
#if defined(FEAT_FOLDING)
linenr_T lnum;
linenr_T first, last;
lnum = tv_get_lnum(argvars);
if (lnum >= 1 && lnum <= curbuf->b_ml.ml_line_count)
{
if (hasFoldingWin(curwin, lnum, &first, &last, FALSE, NULL))
{
if (end)
rettv->vval.v_number = (varnumber_T)last;
else
rettv->vval.v_number = (varnumber_T)first;
return;
}
}
#endif
rettv->vval.v_number = -1;
}
void
f_foldclosed(typval_T *argvars, typval_T *rettv)
{
foldclosed_both(argvars, rettv, FALSE);
}
void
f_foldclosedend(typval_T *argvars, typval_T *rettv)
{
foldclosed_both(argvars, rettv, TRUE);
}
void
f_foldlevel(typval_T *argvars UNUSED, typval_T *rettv UNUSED)
{
#if defined(FEAT_FOLDING)
linenr_T lnum;
lnum = tv_get_lnum(argvars);
if (lnum >= 1 && lnum <= curbuf->b_ml.ml_line_count)
rettv->vval.v_number = foldLevel(lnum);
#endif
}
void
f_foldtext(typval_T *argvars UNUSED, typval_T *rettv)
{
#if defined(FEAT_FOLDING)
linenr_T foldstart;
linenr_T foldend;
char_u *dashes;
linenr_T lnum;
char_u *s;
char_u *r;
int len;
char *txt;
long count;
#endif
rettv->v_type = VAR_STRING;
rettv->vval.v_string = NULL;
#if defined(FEAT_FOLDING)
foldstart = (linenr_T)get_vim_var_nr(VV_FOLDSTART);
foldend = (linenr_T)get_vim_var_nr(VV_FOLDEND);
dashes = get_vim_var_str(VV_FOLDDASHES);
if (foldstart > 0 && foldend <= curbuf->b_ml.ml_line_count
&& dashes != NULL)
{
for (lnum = foldstart; lnum < foldend; ++lnum)
if (!linewhite(lnum))
break;
s = skipwhite(ml_get(lnum));
if (s[0] == '/' && (s[1] == '*' || s[1] == '/'))
{
s = skipwhite(s + 2);
if (*skipwhite(s) == NUL
&& lnum + 1 < (linenr_T)get_vim_var_nr(VV_FOLDEND))
{
s = skipwhite(ml_get(lnum + 1));
if (*s == '*')
s = skipwhite(s + 1);
}
}
count = (long)(foldend - foldstart + 1);
txt = NGETTEXT("+-%s%3ld line: ", "+-%s%3ld lines: ", count);
r = alloc(STRLEN(txt)
+ STRLEN(dashes) 
+ 20 
+ STRLEN(s)); 
if (r != NULL)
{
sprintf((char *)r, txt, dashes, count);
len = (int)STRLEN(r);
STRCAT(r, s);
foldtext_cleanup(r + len);
rettv->vval.v_string = r;
}
}
#endif
}
void
f_foldtextresult(typval_T *argvars UNUSED, typval_T *rettv)
{
#if defined(FEAT_FOLDING)
linenr_T lnum;
char_u *text;
char_u buf[FOLD_TEXT_LEN];
foldinfo_T foldinfo;
int fold_count;
static int entered = FALSE;
#endif
rettv->v_type = VAR_STRING;
rettv->vval.v_string = NULL;
#if defined(FEAT_FOLDING)
if (entered)
return; 
entered = TRUE;
lnum = tv_get_lnum(argvars);
if (lnum < 0)
lnum = 0;
fold_count = foldedCount(curwin, lnum, &foldinfo);
if (fold_count > 0)
{
text = get_foldtext(curwin, lnum, lnum + fold_count - 1,
&foldinfo, buf);
if (text == buf)
text = vim_strsave(text);
rettv->vval.v_string = text;
}
entered = FALSE;
#endif
}
#endif 
