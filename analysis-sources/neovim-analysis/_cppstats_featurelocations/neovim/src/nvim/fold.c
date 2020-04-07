








#include <string.h>
#include <inttypes.h>

#include "nvim/vim.h"
#include "nvim/ascii.h"
#include "nvim/fold.h"
#include "nvim/change.h"
#include "nvim/charset.h"
#include "nvim/cursor.h"
#include "nvim/diff.h"
#include "nvim/eval.h"
#include "nvim/ex_docmd.h"
#include "nvim/ex_session.h"
#include "nvim/func_attr.h"
#include "nvim/indent.h"
#include "nvim/buffer_updates.h"
#include "nvim/extmark.h"
#include "nvim/mark.h"
#include "nvim/memline.h"
#include "nvim/memory.h"
#include "nvim/message.h"
#include "nvim/misc1.h"
#include "nvim/garray.h"
#include "nvim/move.h"
#include "nvim/option.h"
#include "nvim/screen.h"
#include "nvim/strings.h"
#include "nvim/syntax.h"
#include "nvim/undo.h"
#include "nvim/ops.h"









typedef struct {
linenr_T fd_top; 

linenr_T fd_len; 
garray_T fd_nested; 
char fd_flags; 
TriState fd_small; 


} fold_T;

#define FD_OPEN 0 
#define FD_CLOSED 1 
#define FD_LEVEL 2 

#define MAX_LEVEL 20 


typedef struct {
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


static bool fold_changed;


typedef void (*LevelGetter)(fline_T *);



#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "fold.c.generated.h"
#endif
static char *e_nofold = N_("E490: No fold found");





static linenr_T invalid_top = (linenr_T)0;
static linenr_T invalid_bot = (linenr_T)0;








static linenr_T prev_lnum = 0;
static int prev_lnum_lvl = -1;


#define DONE_NOTHING 0
#define DONE_ACTION 1 
#define DONE_FOLD 2 

static size_t foldstartmarkerlen;
static char_u *foldendmarker;
static size_t foldendmarkerlen;






void copyFoldingState(win_T *wp_from, win_T *wp_to)
{
wp_to->w_fold_manual = wp_from->w_fold_manual;
wp_to->w_foldinvalid = wp_from->w_foldinvalid;
cloneFoldGrowArray(&wp_from->w_folds, &wp_to->w_folds);
}





int hasAnyFolding(win_T *win)
{

return !win->w_buffer->terminal && win->w_p_fen
&& (!foldmethodIsManual(win) || !GA_EMPTY(&win->w_folds));
}








bool hasFolding(linenr_T lnum, linenr_T *firstp, linenr_T *lastp)
{
return hasFoldingWin(curwin, lnum, firstp, lastp, true, NULL);
}


bool hasFoldingWin(
win_T *const win,
const linenr_T lnum,
linenr_T *const firstp,
linenr_T *const lastp,
const bool cache, 
foldinfo_T *const infop 
)
{
bool had_folded = false;
linenr_T first = 0;
linenr_T last = 0;
linenr_T lnum_rel = lnum;
fold_T *fp;
int level = 0;
bool use_level = false;
bool maybe_small = false;
int low_level = 0;

checkupdate(win);


if (!hasAnyFolding(win)) {
if (infop != NULL)
infop->fi_level = 0;
return false;
}

if (cache) {




const int x = find_wl_entry(win, lnum);
if (x >= 0) {
first = win->w_lines[x].wl_lnum;
last = win->w_lines[x].wl_lastlnum;
had_folded = win->w_lines[x].wl_folded;
}
}

if (first == 0) {



garray_T *gap = &win->w_folds;
for (;; ) {
if (!foldFind(gap, lnum_rel, &fp))
break;


if (lnum_rel == fp->fd_top && low_level == 0)
low_level = level + 1;

first += fp->fd_top;
last += fp->fd_top;


had_folded = check_closed(win, fp, &use_level, level,
&maybe_small, lnum - lnum_rel);
if (had_folded) {

last += fp->fd_len - 1;
break;
}



gap = &fp->fd_nested;
lnum_rel -= fp->fd_top;
++level;
}
}

if (!had_folded) {
if (infop != NULL) {
infop->fi_level = level;
infop->fi_lnum = lnum - lnum_rel;
infop->fi_low_level = low_level == 0 ? level : low_level;
}
return false;
}

if (last > win->w_buffer->b_ml.ml_line_count) {
last = win->w_buffer->b_ml.ml_line_count;
}
if (lastp != NULL)
*lastp = last;
if (firstp != NULL)
*firstp = first;
if (infop != NULL) {
infop->fi_level = level + 1;
infop->fi_lnum = first;
infop->fi_low_level = low_level == 0 ? level + 1 : low_level;
}
return true;
}





int foldLevel(linenr_T lnum)
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





bool lineFolded(win_T *const win, const linenr_T lnum)
{
return foldedCount(win, lnum, NULL) != 0;
}










long foldedCount(win_T *win, linenr_T lnum, foldinfo_T *infop)
{
linenr_T last;

if (hasFoldingWin(win, lnum, NULL, &last, false, infop)) {
return (long)(last - lnum + 1);
}
return 0;
}





int foldmethodIsManual(win_T *wp)
{
return wp->w_p_fdm[3] == 'u';
}





int foldmethodIsIndent(win_T *wp)
{
return wp->w_p_fdm[0] == 'i';
}





int foldmethodIsExpr(win_T *wp)
{
return wp->w_p_fdm[1] == 'x';
}





int foldmethodIsMarker(win_T *wp)
{
return wp->w_p_fdm[2] == 'r';
}





int foldmethodIsSyntax(win_T *wp)
{
return wp->w_p_fdm[0] == 's';
}





int foldmethodIsDiff(win_T *wp)
{
return wp->w_p_fdm[0] == 'd';
}






void closeFold(linenr_T lnum, long count)
{
setFoldRepeat(lnum, count, FALSE);
}





void closeFoldRecurse(linenr_T lnum)
{
(void)setManualFold(lnum, FALSE, TRUE, NULL);
}






void
opFoldRange(
linenr_T first,
linenr_T last,
int opening, 
int recurse, 
int had_visual 
)
{
int done = DONE_NOTHING; 
linenr_T lnum;
linenr_T lnum_next;

for (lnum = first; lnum <= last; lnum = lnum_next + 1) {
lnum_next = lnum;


if (opening && !recurse)
(void)hasFolding(lnum, NULL, &lnum_next);
(void)setManualFold(lnum, opening, recurse, &done);


if (!opening && !recurse)
(void)hasFolding(lnum, NULL, &lnum_next);
}
if (done == DONE_NOTHING)
EMSG(_(e_nofold));

if (had_visual)
redraw_curbuf_later(INVERTED);
}






void openFold(linenr_T lnum, long count)
{
setFoldRepeat(lnum, count, TRUE);
}





void openFoldRecurse(linenr_T lnum)
{
(void)setManualFold(lnum, TRUE, TRUE, NULL);
}





void foldOpenCursor(void)
{
int done;

checkupdate(curwin);
if (hasAnyFolding(curwin))
for (;; ) {
done = DONE_NOTHING;
(void)setManualFold(curwin->w_cursor.lnum, TRUE, FALSE, &done);
if (!(done & DONE_ACTION))
break;
}
}





void newFoldLevel(void)
{
newFoldLevelWin(curwin);

if (foldmethodIsDiff(curwin) && curwin->w_p_scb) {



FOR_ALL_WINDOWS_IN_TAB(wp, curtab) {
if (wp != curwin && foldmethodIsDiff(wp) && wp->w_p_scb) {
wp->w_p_fdl = curwin->w_p_fdl;
newFoldLevelWin(wp);
}
}
}
}

static void newFoldLevelWin(win_T *wp)
{
fold_T *fp;

checkupdate(wp);
if (wp->w_fold_manual) {



fp = (fold_T *)wp->w_folds.ga_data;
for (int i = 0; i < wp->w_folds.ga_len; ++i)
fp[i].fd_flags = FD_LEVEL;
wp->w_fold_manual = false;
}
changed_window_setting_win(wp);
}





void foldCheckClose(void)
{
if (*p_fcl != NUL) { 
checkupdate(curwin);
if (checkCloseRec(&curwin->w_folds, curwin->w_cursor.lnum,
(int)curwin->w_p_fdl))
changed_window_setting();
}
}


static int checkCloseRec(garray_T *gap, linenr_T lnum, int level)
{
fold_T *fp;
int retval = FALSE;

fp = (fold_T *)gap->ga_data;
for (int i = 0; i < gap->ga_len; ++i) {

if (fp[i].fd_flags == FD_OPEN) {
if (level <= 0 && (lnum < fp[i].fd_top
|| lnum >= fp[i].fd_top + fp[i].fd_len)) {
fp[i].fd_flags = FD_LEVEL;
retval = TRUE;
} else
retval |= checkCloseRec(&fp[i].fd_nested, lnum - fp[i].fd_top,
level - 1);
}
}
return retval;
}






int foldManualAllowed(int create)
{
if (foldmethodIsManual(curwin) || foldmethodIsMarker(curwin))
return TRUE;
if (create)
EMSG(_("E350: Cannot create fold with current 'foldmethod'"));
else
EMSG(_("E351: Cannot delete fold with current 'foldmethod'"));
return FALSE;
}






void foldCreate(linenr_T start, linenr_T end)
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

if (start > end) {

end = start_rel;
start = end_rel;
start_rel = start;
end_rel = end;
}


if (foldmethodIsMarker(curwin)) {
foldCreateMarkers(start, end);
return;
}

checkupdate(curwin);


gap = &curwin->w_folds;
for (;; ) {
if (!foldFind(gap, start_rel, &fp))
break;
if (fp->fd_top + fp->fd_len > end_rel) {

gap = &fp->fd_nested;
start_rel -= fp->fd_top;
end_rel -= fp->fd_top;
if (use_level || fp->fd_flags == FD_LEVEL) {
use_level = TRUE;
if (level >= curwin->w_p_fdl)
closed = TRUE;
} else if (fp->fd_flags == FD_CLOSED)
closed = TRUE;
++level;
} else {


break;
}
}

i = (int)(fp - (fold_T *)gap->ga_data);
ga_grow(gap, 1);
{
fp = (fold_T *)gap->ga_data + i;
ga_init(&fold_ga, (int)sizeof(fold_T), 10);


for (cont = 0; i + cont < gap->ga_len; ++cont)
if (fp[cont].fd_top > end_rel)
break;
if (cont > 0) {
ga_grow(&fold_ga, cont);


if (start_rel > fp->fd_top)
start_rel = fp->fd_top;



if (end_rel < fp[cont - 1].fd_top + fp[cont - 1].fd_len - 1)
end_rel = fp[cont - 1].fd_top + fp[cont - 1].fd_len - 1;

memmove(fold_ga.ga_data, fp, sizeof(fold_T) * (size_t)cont);
fold_ga.ga_len += cont;
i += cont;



for (j = 0; j < cont; ++j)
((fold_T *)fold_ga.ga_data)[j].fd_top -= start_rel;
}

if (i < gap->ga_len)
memmove(fp + 1, (fold_T *)gap->ga_data + i,
sizeof(fold_T) * (size_t)(gap->ga_len - i));
gap->ga_len = gap->ga_len + 1 - cont;


fp->fd_nested = fold_ga;
fp->fd_top = start_rel;
fp->fd_len = end_rel - start_rel + 1;




if (use_level && !closed && level < curwin->w_p_fdl)
closeFold(start, 1L);
if (!use_level)
curwin->w_fold_manual = true;
fp->fd_flags = FD_CLOSED;
fp->fd_small = kNone;


changed_window_setting();
}
}








void deleteFold(
const linenr_T start,
const linenr_T end,
const int recursive,
const bool had_visual 
)
{
fold_T *fp;
fold_T *found_fp = NULL;
linenr_T found_off = 0;
bool maybe_small = false;
int level = 0;
linenr_T lnum = start;
bool did_one = false;
linenr_T first_lnum = MAXLNUM;
linenr_T last_lnum = 0;

checkupdate(curwin);

while (lnum <= end) {

garray_T *gap = &curwin->w_folds;
garray_T *found_ga = NULL;
linenr_T lnum_off = 0;
bool use_level = false;
for (;; ) {
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
if (found_ga == NULL) {
++lnum;
} else {
lnum = found_fp->fd_top + found_fp->fd_len + found_off;

if (foldmethodIsManual(curwin))
deleteFoldEntry(found_ga,
(int)(found_fp - (fold_T *)found_ga->ga_data), recursive);
else {
if (first_lnum > found_fp->fd_top + found_off)
first_lnum = found_fp->fd_top + found_off;
if (last_lnum < lnum)
last_lnum = lnum;
if (!did_one)
parseMarker(curwin);
deleteFoldMarkers(found_fp, recursive, found_off);
}
did_one = true;


changed_window_setting();
}
}
if (!did_one) {
EMSG(_(e_nofold));

if (had_visual)
redraw_curbuf_later(INVERTED);
} else

check_cursor_col();

if (last_lnum > 0) {
changed_lines(first_lnum, (colnr_T)0, last_lnum, 0L, false);






int64_t num_changed = last_lnum - first_lnum;
buf_updates_send_changes(curbuf, first_lnum, num_changed,
num_changed, true);
}
}





void clearFolding(win_T *win)
{
deleteFoldRecurse(&win->w_folds);
win->w_foldinvalid = false;
}








void foldUpdate(win_T *wp, linenr_T top, linenr_T bot)
{
if (compl_busy || State & INSERT) {
return;
}

if (need_diff_redraw) {

return;
}


fold_T *fp;
(void)foldFind(&wp->w_folds, top, &fp);
while (fp < (fold_T *)wp->w_folds.ga_data + wp->w_folds.ga_len
&& fp->fd_top < bot) {
fp->fd_small = kNone;
fp++;
}

if (foldmethodIsIndent(wp)
|| foldmethodIsExpr(wp)
|| foldmethodIsMarker(wp)
|| foldmethodIsDiff(wp)
|| foldmethodIsSyntax(wp)) {
int save_got_int = got_int;


got_int = FALSE;
foldUpdateIEMS(wp, top, bot);
got_int |= save_got_int;
}
}


void foldUpdateAfterInsert(void)
{
if (foldmethodIsManual(curwin) 

|| foldmethodIsSyntax(curwin) || foldmethodIsExpr(curwin)) {
return;
}

foldUpdateAll(curwin);
foldOpenCursor();
}








void foldUpdateAll(win_T *win)
{
win->w_foldinvalid = true;
redraw_win_later(win, NOT_VALID);
}






int foldMoveTo(
const bool updown,
const int dir, 
const long count
)
{
int retval = FAIL;
linenr_T lnum;
fold_T *fp;

checkupdate(curwin);


for (long n = 0; n < count; n++) {


linenr_T lnum_off = 0;
garray_T *gap = &curwin->w_folds;
bool use_level = false;
bool maybe_small = false;
linenr_T lnum_found = curwin->w_cursor.lnum;
int level = 0;
bool last = false;
for (;; ) {
if (!foldFind(gap, curwin->w_cursor.lnum - lnum_off, &fp)) {
if (!updown)
break;



if (dir == FORWARD) {
if (fp - (fold_T *)gap->ga_data >= gap->ga_len)
break;
--fp;
} else {
if (fp == (fold_T *)gap->ga_data)
break;
}


last = true;
}

if (!last) {

if (check_closed(curwin, fp, &use_level, level,
&maybe_small, lnum_off)) {
last = true;
}


if (last && !updown)
break;
}

if (updown) {
if (dir == FORWARD) {

if (fp + 1 - (fold_T *)gap->ga_data < gap->ga_len) {
lnum = fp[1].fd_top + lnum_off;
if (lnum > curwin->w_cursor.lnum)
lnum_found = lnum;
}
} else {

if (fp > (fold_T *)gap->ga_data) {
lnum = fp[-1].fd_top + lnum_off + fp[-1].fd_len - 1;
if (lnum < curwin->w_cursor.lnum)
lnum_found = lnum;
}
}
} else {


if (dir == FORWARD) {
lnum = fp->fd_top + lnum_off + fp->fd_len - 1;
if (lnum > curwin->w_cursor.lnum)
lnum_found = lnum;
} else {
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
if (lnum_found != curwin->w_cursor.lnum) {
if (retval == FAIL)
setpcmark();
curwin->w_cursor.lnum = lnum_found;
curwin->w_cursor.col = 0;
retval = OK;
} else
break;
}

return retval;
}





void foldInitWin(win_T *new_win)
{
ga_init(&new_win->w_folds, (int)sizeof(fold_T), 10);
}








int find_wl_entry(win_T *win, linenr_T lnum)
{
int i;

for (i = 0; i < win->w_lines_valid; ++i)
if (win->w_lines[i].wl_valid) {
if (lnum < win->w_lines[i].wl_lnum)
return -1;
if (lnum <= win->w_lines[i].wl_lastlnum)
return i;
}
return -1;
}





void foldAdjustVisual(void)
{
pos_T *start, *end;
char_u *ptr;

if (!VIsual_active || !hasAnyFolding(curwin))
return;

if (ltoreq(VIsual, curwin->w_cursor)) {
start = &VIsual;
end = &curwin->w_cursor;
} else {
start = &curwin->w_cursor;
end = &VIsual;
}
if (hasFolding(start->lnum, &start->lnum, NULL))
start->col = 0;
if (hasFolding(end->lnum, NULL, &end->lnum)) {
ptr = ml_get(end->lnum);
end->col = (colnr_T)STRLEN(ptr);
if (end->col > 0 && *p_sel == 'o')
--end->col;

if (has_mbyte)
mb_adjust_cursor();
}
}





void foldAdjustCursor(void)
{
(void)hasFolding(curwin->w_cursor.lnum, &curwin->w_cursor.lnum, NULL);
}






void cloneFoldGrowArray(garray_T *from, garray_T *to)
{
fold_T *from_p;
fold_T *to_p;

ga_init(to, from->ga_itemsize, from->ga_growsize);

if (GA_EMPTY(from))
return;

ga_grow(to, from->ga_len);

from_p = (fold_T *)from->ga_data;
to_p = (fold_T *)to->ga_data;

for (int i = 0; i < from->ga_len; i++) {
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








static int foldFind(garray_T *gap, linenr_T lnum, fold_T **fpp)
{
linenr_T low, high;
fold_T *fp;






fp = (fold_T *)gap->ga_data;
low = 0;
high = gap->ga_len - 1;
while (low <= high) {
linenr_T i = (low + high) / 2;
if (fp[i].fd_top > lnum)

high = i - 1;
else if (fp[i].fd_top + fp[i].fd_len <= lnum)

low = i + 1;
else {

*fpp = fp + i;
return TRUE;
}
}
*fpp = fp + low;
return FALSE;
}





static int foldLevelWin(win_T *wp, linenr_T lnum)
{
fold_T *fp;
linenr_T lnum_rel = lnum;
int level = 0;
garray_T *gap;


gap = &wp->w_folds;
for (;; ) {
if (!foldFind(gap, lnum_rel, &fp))
break;

gap = &fp->fd_nested;
lnum_rel -= fp->fd_top;
++level;
}

return level;
}





static void checkupdate(win_T *wp)
{
if (wp->w_foldinvalid) {
foldUpdate(wp, (linenr_T)1, (linenr_T)MAXLNUM); 
wp->w_foldinvalid = false;
}
}






static void setFoldRepeat(linenr_T lnum, long count, int do_open)
{
int done;
long n;

for (n = 0; n < count; ++n) {
done = DONE_NOTHING;
(void)setManualFold(lnum, do_open, FALSE, &done);
if (!(done & DONE_ACTION)) {

if (n == 0 && !(done & DONE_FOLD))
EMSG(_(e_nofold));
break;
}
}
}






static linenr_T
setManualFold(
linenr_T lnum,
int opening, 
int recurse, 
int *donep
)
{
if (foldmethodIsDiff(curwin) && curwin->w_p_scb) {
linenr_T dlnum;





FOR_ALL_WINDOWS_IN_TAB(wp, curtab) {
if (wp != curwin && foldmethodIsDiff(wp) && wp->w_p_scb) {
dlnum = diff_lnum_win(curwin->w_cursor.lnum, wp);
if (dlnum != 0) {
(void)setManualFoldWin(wp, dlnum, opening, recurse, NULL);
}
}
}
}

return setManualFoldWin(curwin, lnum, opening, recurse, donep);
}











static linenr_T
setManualFoldWin(
win_T *wp,
linenr_T lnum,
int opening, 
int recurse, 
int *donep
)
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
for (;; ) {
if (!foldFind(gap, lnum, &fp)) {

if (fp < (fold_T *)gap->ga_data + gap->ga_len)
next = fp->fd_top + off;
break;
}


found_fold = TRUE;


if (fp + 1 < (fold_T *)gap->ga_data + gap->ga_len)
next = fp[1].fd_top + off;


if (use_level || fp->fd_flags == FD_LEVEL) {
use_level = TRUE;
if (level >= wp->w_p_fdl)
fp->fd_flags = FD_CLOSED;
else
fp->fd_flags = FD_OPEN;
fp2 = (fold_T *)fp->fd_nested.ga_data;
for (j = 0; j < fp->fd_nested.ga_len; ++j)
fp2[j].fd_flags = FD_LEVEL;
}


if (!opening && recurse) {
if (fp->fd_flags != FD_CLOSED) {
done |= DONE_ACTION;
fp->fd_flags = FD_CLOSED;
}
} else if (fp->fd_flags == FD_CLOSED) {

if (opening) {
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
if (found_fold) {

if (!opening && found != NULL) {
found->fd_flags = FD_CLOSED;
done |= DONE_ACTION;
}
wp->w_fold_manual = true;
if (done & DONE_ACTION)
changed_window_setting_win(wp);
done |= DONE_FOLD;
} else if (donep == NULL && wp == curwin)
EMSG(_(e_nofold));

if (donep != NULL)
*donep |= done;

return next;
}





static void foldOpenNested(fold_T *fpr)
{
fold_T *fp;

fp = (fold_T *)fpr->fd_nested.ga_data;
for (int i = 0; i < fpr->fd_nested.ga_len; ++i) {
foldOpenNested(&fp[i]);
fp[i].fd_flags = FD_OPEN;
}
}





static void deleteFoldEntry(garray_T *const gap, const int idx,
const bool recursive)
{
fold_T *fp = (fold_T *)gap->ga_data + idx;
if (recursive || GA_EMPTY(&fp->fd_nested)) {

deleteFoldRecurse(&fp->fd_nested);
gap->ga_len--;
if (idx < gap->ga_len) {
memmove(fp, fp + 1, sizeof(*fp) * (size_t)(gap->ga_len - idx));
}
} else {


int moved = fp->fd_nested.ga_len;
ga_grow(gap, moved - 1);
{

fp = (fold_T *)gap->ga_data + idx;


fold_T *nfp = (fold_T *)fp->fd_nested.ga_data;
for (int i = 0; i < moved; i++) {
nfp[i].fd_top += fp->fd_top;
if (fp->fd_flags == FD_LEVEL)
nfp[i].fd_flags = FD_LEVEL;
if (fp->fd_small == kNone) {
nfp[i].fd_small = kNone;
}
}


if (idx + 1 < gap->ga_len) {
memmove(fp + moved, fp + 1,
sizeof(*fp) * (size_t)(gap->ga_len - (idx + 1)));
}

memmove(fp, nfp, sizeof(*fp) * (size_t)moved);
xfree(nfp);
gap->ga_len += moved - 1;
}
}
}





void deleteFoldRecurse(garray_T *gap)
{
#define DELETE_FOLD_NESTED(fd) deleteFoldRecurse(&((fd)->fd_nested))
GA_DEEP_CLEAR(gap, fold_T, DELETE_FOLD_NESTED);
}





void foldMarkAdjust(win_T *wp, linenr_T line1, linenr_T line2, long amount, long amount_after)
{


if (amount == MAXLNUM && line2 >= line1 && line2 - line1 >= -amount_after)
line2 = line1 - amount_after - 1;


if ((State & INSERT) && amount == (linenr_T)1 && line2 == MAXLNUM)
--line1;
foldMarkAdjustRecurse(&wp->w_folds, line1, line2, amount, amount_after);
}


static void foldMarkAdjustRecurse(garray_T *gap, linenr_T line1, linenr_T line2, long amount, long amount_after)
{
fold_T *fp;
linenr_T last;
linenr_T top;



if ((State & INSERT) && amount == (linenr_T)1 && line2 == MAXLNUM)
top = line1 + 1;
else
top = line1;


(void)foldFind(gap, line1, &fp);




for (int i = (int)(fp - (fold_T *)gap->ga_data); i < gap->ga_len; ++i, ++fp) {












last = fp->fd_top + fp->fd_len - 1; 


if (last < line1)
continue;


if (fp->fd_top > line2) {
if (amount_after == 0)
break;
fp->fd_top += amount_after;
} else {
if (fp->fd_top >= top && last <= line2) {

if (amount == MAXLNUM) {

deleteFoldEntry(gap, i, true);
i--; 
fp--;
} else {
fp->fd_top += amount;
}
} else {
if (fp->fd_top < top) {

foldMarkAdjustRecurse(&fp->fd_nested, line1 - fp->fd_top,
line2 - fp->fd_top, amount, amount_after);
if (last <= line2) {

if (amount == MAXLNUM)
fp->fd_len = line1 - fp->fd_top;
else
fp->fd_len += amount;
} else {

fp->fd_len += amount_after;
}
} else {


if (amount == MAXLNUM) {
foldMarkAdjustRecurse(&fp->fd_nested, line1 - fp->fd_top,
line2 - fp->fd_top, amount,
amount_after + (fp->fd_top - top));
fp->fd_len -= line2 - fp->fd_top + 1;
fp->fd_top = line1;
} else {
foldMarkAdjustRecurse(&fp->fd_nested, line1 - fp->fd_top,
line2 - fp->fd_top, amount,
amount_after - amount);
fp->fd_len += amount_after - amount;
fp->fd_top += amount;
}
}
}
}
}
}






int getDeepestNesting(win_T *wp)
{
checkupdate(wp);
return getDeepestNestingRecurse(&wp->w_folds);
}

static int getDeepestNestingRecurse(garray_T *gap)
{
int level;
int maxlevel = 0;
fold_T *fp;

fp = (fold_T *)gap->ga_data;
for (int i = 0; i < gap->ga_len; ++i) {
level = getDeepestNestingRecurse(&fp[i].fd_nested) + 1;
if (level > maxlevel)
maxlevel = level;
}

return maxlevel;
}





static bool check_closed(
win_T *const win,
fold_T *const fp,
bool *const use_levelp, 
const int level, 
bool *const maybe_smallp, 
const linenr_T lnum_off 
)
{
bool closed = false;



if (*use_levelp || fp->fd_flags == FD_LEVEL) {
*use_levelp = true;
if (level >= win->w_p_fdl) {
closed = true;
}
} else if (fp->fd_flags == FD_CLOSED) {
closed = true;
}


if (fp->fd_small == kNone) {
*maybe_smallp = true;
}
if (closed) {
if (*maybe_smallp) {
fp->fd_small = kNone;
}
checkSmall(win, fp, lnum_off);
if (fp->fd_small == kTrue) {
closed = false;
}
}
return closed;
}





static void
checkSmall(
win_T *const wp,
fold_T *const fp,
const linenr_T lnum_off 
)
{
if (fp->fd_small == kNone) {

setSmallMaybe(&fp->fd_nested);

if (fp->fd_len > curwin->w_p_fml) {
fp->fd_small = kFalse;
} else {
int count = 0;
for (int n = 0; n < fp->fd_len; n++) {
count += plines_win_nofold(wp, fp->fd_top + lnum_off + n);
if (count > curwin->w_p_fml) {
fp->fd_small = kFalse;
return;
}
}
fp->fd_small = kTrue;
}
}
}



static void setSmallMaybe(garray_T *gap)
{
fold_T *fp = (fold_T *)gap->ga_data;
for (int i = 0; i < gap->ga_len; i++) {
fp[i].fd_small = kNone;
}
}






static void foldCreateMarkers(linenr_T start, linenr_T end)
{
if (!MODIFIABLE(curbuf)) {
EMSG(_(e_modifiable));
return;
}
parseMarker(curwin);

foldAddMarker(start, curwin->w_p_fmr, foldstartmarkerlen);
foldAddMarker(end, foldendmarker, foldendmarkerlen);



changed_lines(start, (colnr_T)0, end, 0L, false);




int64_t num_changed = 1 + end - start;
buf_updates_send_changes(curbuf, start, num_changed, num_changed, true);
}





static void foldAddMarker(linenr_T lnum, const char_u *marker, size_t markerlen)
{
char_u *cms = curbuf->b_p_cms;
char_u *line;
char_u *newline;
char_u *p = (char_u *)strstr((char *)curbuf->b_p_cms, "%s");
bool line_is_comment = false;


line = ml_get(lnum);
size_t line_len = STRLEN(line);
size_t added = 0;

if (u_save(lnum - 1, lnum + 1) == OK) {

skip_comment(line, false, false, &line_is_comment);
newline = xmalloc(line_len + markerlen + STRLEN(cms) + 1);
STRCPY(newline, line);

if (p == NULL || line_is_comment) {
STRLCPY(newline + line_len, marker, markerlen + 1);
added = markerlen;
} else {
STRCPY(newline + line_len, cms);
memcpy(newline + line_len + (p - cms), marker, markerlen);
STRCPY(newline + line_len + (p - cms) + markerlen, p + 2);
added = markerlen + STRLEN(cms)-2;
}
ml_replace(lnum, newline, false);
if (added) {
extmark_splice(curbuf, (int)lnum-1, (int)line_len,
0, 0,
0, (int)added, kExtmarkUndo);
}
}
}





static void
deleteFoldMarkers(
fold_T *fp,
int recursive,
linenr_T lnum_off 
)
{
if (recursive) {
for (int i = 0; i < fp->fd_nested.ga_len; ++i) {
deleteFoldMarkers((fold_T *)fp->fd_nested.ga_data + i, TRUE,
lnum_off + fp->fd_top);
}
}
foldDelMarker(fp->fd_top + lnum_off, curwin->w_p_fmr, foldstartmarkerlen);
foldDelMarker(fp->fd_top + lnum_off + fp->fd_len - 1, foldendmarker,
foldendmarkerlen);
}







static void foldDelMarker(linenr_T lnum, char_u *marker, size_t markerlen)
{
char_u *newline;
char_u *cms = curbuf->b_p_cms;
char_u *cms2;


if (lnum > curbuf->b_ml.ml_line_count) {
return;
}
char_u *line = ml_get(lnum);
for (char_u *p = line; *p != NUL; ++p) {
if (STRNCMP(p, marker, markerlen) != 0) {
continue;
}

size_t len = markerlen;
if (ascii_isdigit(p[len]))
++len;
if (*cms != NUL) {

cms2 = (char_u *)strstr((char *)cms, "%s");
if (p - line >= cms2 - cms
&& STRNCMP(p - (cms2 - cms), cms, cms2 - cms) == 0
&& STRNCMP(p + len, cms2 + 2, STRLEN(cms2 + 2)) == 0) {
p -= cms2 - cms;
len += STRLEN(cms) - 2;
}
}
if (u_save(lnum - 1, lnum + 1) == OK) {

newline = xmalloc(STRLEN(line) - len + 1);
assert(p >= line);
memcpy(newline, line, (size_t)(p - line));
STRCPY(newline + (p - line), p + len);
ml_replace(lnum, newline, false);
extmark_splice(curbuf, (int)lnum-1, (int)(p - line),
0, (int)len,
0, 0, kExtmarkUndo);
}
break;
}
}





char_u *get_foldtext(win_T *wp, linenr_T lnum, linenr_T lnume,
foldinfo_T *foldinfo, char_u *buf)
FUNC_ATTR_NONNULL_ARG(1)
{
char_u *text = NULL;

static int got_fdt_error = FALSE;
int save_did_emsg = did_emsg;
static win_T *last_wp = NULL;
static linenr_T last_lnum = 0;

if (last_wp == NULL || last_wp != wp || last_lnum > lnum || last_lnum == 0)

got_fdt_error = FALSE;

if (!got_fdt_error)

did_emsg = FALSE;

if (*wp->w_p_fdt != NUL) {
char dashes[MAX_LEVEL + 2];
win_T *save_curwin;
int level;
char_u *p;


set_vim_var_nr(VV_FOLDSTART, (varnumber_T) lnum);
set_vim_var_nr(VV_FOLDEND, (varnumber_T) lnume);



level = foldinfo->fi_level;
if (level > (int)sizeof(dashes) - 1)
level = (int)sizeof(dashes) - 1;
memset(dashes, '-', (size_t)level);
dashes[level] = NUL;
set_vim_var_string(VV_FOLDDASHES, dashes, -1);
set_vim_var_nr(VV_FOLDLEVEL, (varnumber_T) level);


if (!got_fdt_error) {
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

if (text != NULL) {


for (p = text; *p != NUL; p++) {
int len = utfc_ptr2len(p);

if (len > 1) {
if (!vim_isprintc(utf_ptr2char(p))) {
break;
}
p += len - 1;
} else if (*p == TAB)
*p = ' ';
else if (ptr2cells(p) > 1)
break;
}
if (*p != NUL) {
p = (char_u *)transstr((const char *)text);
xfree(text);
text = p;
}
}
}
if (text == NULL) {
unsigned long count = (unsigned long)(lnume - lnum + 1);

vim_snprintf((char *)buf, FOLD_TEXT_LEN,
NGETTEXT("+--%3ld line folded",
"+--%3ld lines folded ", count),
count);
text = buf;
}
return text;
}





void foldtext_cleanup(char_u *str)
{
char_u *s;
char_u *p;
int did1 = FALSE;
int did2 = FALSE;


char_u *cms_start = skipwhite(curbuf->b_p_cms);
size_t cms_slen = STRLEN(cms_start);
while (cms_slen > 0 && ascii_iswhite(cms_start[cms_slen - 1]))
--cms_slen;


char_u *cms_end = (char_u *)strstr((char *)cms_start, "%s");
size_t cms_elen = 0;
if (cms_end != NULL) {
cms_elen = cms_slen - (size_t)(cms_end - cms_start);
cms_slen = (size_t)(cms_end - cms_start);


while (cms_slen > 0 && ascii_iswhite(cms_start[cms_slen - 1]))
--cms_slen;


s = skipwhite(cms_end + 2);
cms_elen -= (size_t)(s - cms_end);
cms_end = s;
}
parseMarker(curwin);

for (s = str; *s != NUL; ) {
size_t len = 0;
if (STRNCMP(s, curwin->w_p_fmr, foldstartmarkerlen) == 0)
len = foldstartmarkerlen;
else if (STRNCMP(s, foldendmarker, foldendmarkerlen) == 0)
len = foldendmarkerlen;
if (len > 0) {
if (ascii_isdigit(s[len]))
++len;



for (p = s; p > str && ascii_iswhite(p[-1]); --p)
;
if (p >= str + cms_slen
&& STRNCMP(p - cms_slen, cms_start, cms_slen) == 0) {
len += (size_t)(s - p) + cms_slen;
s = p - cms_slen;
}
} else if (cms_end != NULL) {
if (!did1 && cms_slen > 0 && STRNCMP(s, cms_start, cms_slen) == 0) {
len = cms_slen;
did1 = TRUE;
} else if (!did2 && cms_elen > 0
&& STRNCMP(s, cms_end, cms_elen) == 0) {
len = cms_elen;
did2 = TRUE;
}
}
if (len != 0) {
while (ascii_iswhite(s[len]))
++len;
STRMOVE(s, s + len);
} else {
MB_PTR_ADV(s);
}
}
}









static void foldUpdateIEMS(win_T *const wp, linenr_T top, linenr_T bot)
{
fline_T fline;
void (*getlevel)(fline_T *);
fold_T *fp;


if (invalid_top != (linenr_T)0)
return;

if (wp->w_foldinvalid) {

top = 1;
bot = wp->w_buffer->b_ml.ml_line_count;
wp->w_foldinvalid = false;


setSmallMaybe(&wp->w_folds);
}


if (foldmethodIsDiff(wp)) {
if (top > diff_context)
top -= diff_context;
else
top = 1;
bot += diff_context;
}



if (top > wp->w_buffer->b_ml.ml_line_count) {
top = wp->w_buffer->b_ml.ml_line_count;
}

fold_changed = false;
fline.wp = wp;
fline.off = 0;
fline.lvl = 0;
fline.lvl_next = -1;
fline.start = 0;
fline.end = MAX_LEVEL + 1;
fline.had_end = MAX_LEVEL + 1;

invalid_top = top;
invalid_bot = bot;

if (foldmethodIsMarker(wp)) {
getlevel = foldlevelMarker;


parseMarker(wp);



if (top > 1) {

const int level = foldLevelWin(wp, top - 1);


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
} else {
fline.lnum = top;
if (foldmethodIsExpr(wp)) {
getlevel = foldlevelExpr;


if (top > 1)
--fline.lnum;
} else if (foldmethodIsSyntax(wp))
getlevel = foldlevelSyntax;
else if (foldmethodIsDiff(wp))
getlevel = foldlevelDiff;
else
getlevel = foldlevelIndent;



fline.lvl = -1;
for (; !got_int; --fline.lnum) {


fline.lvl_next = -1;
getlevel(&fline);
if (fline.lvl >= 0)
break;
}
}








if (foldlevelSyntax == getlevel) {
garray_T *gap = &wp->w_folds;
fold_T *fpn = NULL;
int current_fdl = 0;
linenr_T fold_start_lnum = 0;
linenr_T lnum_rel = fline.lnum;

while (current_fdl < fline.lvl) {
if (!foldFind(gap, lnum_rel, &fpn))
break;
++current_fdl;

fold_start_lnum += fpn->fd_top;
gap = &fpn->fd_nested;
lnum_rel -= fpn->fd_top;
}
if (fpn != NULL && current_fdl == fline.lvl) {
linenr_T fold_end_lnum = fold_start_lnum + fpn->fd_len;

if (fold_end_lnum > bot)
bot = fold_end_lnum;
}
}

linenr_T start = fline.lnum;
linenr_T end = bot;

if (start > end && end < wp->w_buffer->b_ml.ml_line_count) {
end = start;
}
while (!got_int) {


if (fline.lnum > wp->w_buffer->b_ml.ml_line_count)
break;
if (fline.lnum > end) {



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


if (fline.lvl > 0) {
invalid_top = fline.lnum;
invalid_bot = end;
end = foldUpdateIEMSRecurse(&wp->w_folds, 1, start, &fline, getlevel, end,
FD_LEVEL);
start = fline.lnum;
} else {
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




if (end != bot) {
if (wp->w_redraw_top == 0 || wp->w_redraw_top > top)
wp->w_redraw_top = top;
if (wp->w_redraw_bot < end)
wp->w_redraw_bot = end;
}

invalid_top = (linenr_T)0;
}

























static linenr_T foldUpdateIEMSRecurse(
garray_T *const gap, const int level, const linenr_T startlnum,
fline_T *const flp, LevelGetter getlevel, linenr_T bot,
const char topflags 
)
{
linenr_T ll;
fold_T *fp = NULL;
fold_T *fp2;
int lvl = level;
linenr_T startlnum2 = startlnum;
const linenr_T firstlnum = flp->lnum; 
int i;
bool finish = false;
const linenr_T linecount = flp->wp->w_buffer->b_ml.ml_line_count - flp->off;
int concat;







if (getlevel == foldlevelMarker && flp->start <= flp->lvl - level
&& flp->lvl > 0) {
(void)foldFind(gap, startlnum - 1, &fp);
if (fp >= ((fold_T *)gap->ga_data) + gap->ga_len
|| fp->fd_top >= startlnum) {
fp = NULL;
}
}












flp->lnum_save = flp->lnum;
while (!got_int) {

line_breakcheck();





lvl = flp->lvl;
if (lvl > MAX_LEVEL)
lvl = MAX_LEVEL;
if (flp->lnum > firstlnum
&& (level > lvl - flp->start || level >= flp->had_end))
lvl = 0;

if (flp->lnum > bot && !finish && fp != NULL) {






if (getlevel != foldlevelMarker
&& getlevel != foldlevelExpr
&& getlevel != foldlevelSyntax)
break;
i = 0;
fp2 = fp;
if (lvl >= level) {



ll = flp->lnum - fp->fd_top;
while (foldFind(&fp2->fd_nested, ll, &fp2)) {
++i;
ll -= fp2->fd_top;
}
}
if (lvl < level + i) {
(void)foldFind(&fp->fd_nested, flp->lnum - fp->fd_top, &fp2);
if (fp2 != NULL) {
bot = fp2->fd_top + fp2->fd_len - 1 + fp->fd_top;
}
} else if (fp->fd_top + fp->fd_len <= flp->lnum && lvl >= level) {
finish = true;
} else {
break;
}
}




if (fp == NULL
&& (lvl != level
|| flp->lnum_save >= bot
|| flp->start != 0
|| flp->had_end <= MAX_LEVEL
|| flp->lnum == linecount)) {




while (!got_int) {


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
&& fp->fd_top <= flp->lnum_save)))) {
if (fp->fd_top + fp->fd_len + concat > firstlnum) {




if (fp->fd_top == firstlnum) {

} else if (fp->fd_top >= startlnum) {
if (fp->fd_top > firstlnum) {


foldMarkAdjustRecurse(&fp->fd_nested,
(linenr_T)0, (linenr_T)MAXLNUM,
(long)(fp->fd_top - firstlnum), 0L);
} else {

foldMarkAdjustRecurse(&fp->fd_nested,
(linenr_T)0,
(long)(firstlnum - fp->fd_top - 1),
(linenr_T)MAXLNUM,
(long)(fp->fd_top - firstlnum));
}
fp->fd_len += fp->fd_top - firstlnum;
fp->fd_top = firstlnum;
fold_changed = true;
} else if ((flp->start != 0 && lvl == level)
|| (firstlnum != startlnum)) {










linenr_T breakstart;
linenr_T breakend;
if (firstlnum != startlnum) {
breakstart = startlnum;
breakend = firstlnum;
} else {
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
|| getlevel == foldlevelSyntax) {
finish = true;
}
}
if (fp->fd_top == startlnum && concat) {
i = (int)(fp - (fold_T *)gap->ga_data);
if (i != 0) {
fp2 = fp - 1;
if (fp2->fd_top + fp2->fd_len == fp->fd_top) {
foldMerge(fp2, gap, fp);
fp = fp2;
}
}
}
break;
}
if (fp->fd_top >= startlnum) {



deleteFoldEntry(gap, (int)(fp - (fold_T *)gap->ga_data), true);
} else {


fp->fd_len = startlnum - fp->fd_top;
foldMarkAdjustRecurse(&fp->fd_nested,
(linenr_T)fp->fd_len, (linenr_T)MAXLNUM,
(linenr_T)MAXLNUM, 0L);
fold_changed = true;
}
} else {


i = (int)(fp - (fold_T *)gap->ga_data);
foldInsert(gap, i);
fp = (fold_T *)gap->ga_data + i;


fp->fd_top = firstlnum;
fp->fd_len = bot - firstlnum + 1;



if (topflags == FD_OPEN) {
flp->wp->w_fold_manual = true;
fp->fd_flags = FD_OPEN;
} else if (i <= 0) {
fp->fd_flags = topflags;
if (topflags != FD_LEVEL)
flp->wp->w_fold_manual = true;
} else
fp->fd_flags = (fp - 1)->fd_flags;
fp->fd_small = kNone;


if (getlevel == foldlevelMarker
|| getlevel == foldlevelExpr
|| getlevel == foldlevelSyntax) {
finish = true;
}
fold_changed = true;
break;
}
}
}

if (lvl < level || flp->lnum > linecount) {




break;
}





if (lvl > level && fp != NULL) {


if (bot < flp->lnum) {
bot = flp->lnum;
}



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


} else {






flp->lnum = flp->lnum_save;
ll = flp->lnum + 1;
while (!got_int) {

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








if (fp->fd_len < flp->lnum - fp->fd_top) {
fp->fd_len = flp->lnum - fp->fd_top;
fp->fd_small = kNone;
fold_changed = true;
} else if (fp->fd_top + fp->fd_len > linecount) {

fp->fd_len = linecount - fp->fd_top + 1;
}



foldRemove(&fp->fd_nested, startlnum2 - fp->fd_top,
flp->lnum - 1 - fp->fd_top);

if (lvl < level) {

if (fp->fd_len != flp->lnum - fp->fd_top) {
if (fp->fd_top + fp->fd_len - 1 > bot) {

if (getlevel == foldlevelMarker
|| getlevel == foldlevelExpr
|| getlevel == foldlevelSyntax) {


bot = fp->fd_top + fp->fd_len - 1;
fp->fd_len = flp->lnum - fp->fd_top;
} else {


i = (int)(fp - (fold_T *)gap->ga_data);
foldSplit(gap, i, flp->lnum, bot);
fp = (fold_T *)gap->ga_data + i;
}
} else {
fp->fd_len = flp->lnum - fp->fd_top;
}
fold_changed = true;
}
}


for (;; ) {
fp2 = fp + 1;
if (fp2 >= (fold_T *)gap->ga_data + gap->ga_len
|| fp2->fd_top > flp->lnum)
break;
if (fp2->fd_top + fp2->fd_len > flp->lnum) {
if (fp2->fd_top < flp->lnum) {

foldMarkAdjustRecurse(&fp2->fd_nested,
(linenr_T)0, (long)(flp->lnum - fp2->fd_top - 1),
(linenr_T)MAXLNUM, (long)(fp2->fd_top - flp->lnum));
fp2->fd_len -= flp->lnum - fp2->fd_top;
fp2->fd_top = flp->lnum;
fold_changed = true;
}

if (lvl >= level) {

foldMerge(fp, gap, fp2);
}
break;
}
fold_changed = true;
deleteFoldEntry(gap, (int)(fp2 - (fold_T *)gap->ga_data), true);
}



if (bot < flp->lnum - 1)
bot = flp->lnum - 1;

return bot;
}





static void foldInsert(garray_T *gap, int i)
{
fold_T *fp;

ga_grow(gap, 1);

fp = (fold_T *)gap->ga_data + i;
if (i < gap->ga_len)
memmove(fp + 1, fp, sizeof(fold_T) * (size_t)(gap->ga_len - i));
++gap->ga_len;
ga_init(&fp->fd_nested, (int)sizeof(fold_T), 10);
}









static void foldSplit(garray_T *const gap, const int i, const linenr_T top,
const linenr_T bot)
{
fold_T *fp2;


foldInsert(gap, i + 1);

fold_T *const fp = (fold_T *)gap->ga_data + i;
fp[1].fd_top = bot + 1;

assert(fp[1].fd_top > bot);
fp[1].fd_len = fp->fd_len - (fp[1].fd_top - fp->fd_top);
fp[1].fd_flags = fp->fd_flags;
fp[1].fd_small = kNone;
fp->fd_small = kNone;



garray_T *const gap1 = &fp->fd_nested;
garray_T *const gap2 = &fp[1].fd_nested;
(void)(foldFind(gap1, bot + 1 - fp->fd_top, &fp2));
const int len = (int)((fold_T *)gap1->ga_data + gap1->ga_len - fp2);
if (len > 0) {
ga_grow(gap2, len);
for (int idx = 0; idx < len; idx++) {
((fold_T *)gap2->ga_data)[idx] = fp2[idx];
((fold_T *)gap2->ga_data)[idx].fd_top
-= fp[1].fd_top - fp->fd_top;
}
gap2->ga_len = len;
gap1->ga_len -= len;
}
fp->fd_len = top - fp->fd_top;
fold_changed = true;
}




















static void foldRemove(garray_T *gap, linenr_T top, linenr_T bot)
{
fold_T *fp = NULL;

if (bot < top) {
return; 
}

for (;; ) {

if (foldFind(gap, top, &fp) && fp->fd_top < top) {

foldRemove(&fp->fd_nested, top - fp->fd_top, bot - fp->fd_top);
if (fp->fd_top + fp->fd_len - 1 > bot) {

foldSplit(gap, (int)(fp - (fold_T *)gap->ga_data), top, bot);
} else {

fp->fd_len = top - fp->fd_top;
}
fold_changed = true;
continue;
}
if (fp >= (fold_T *)(gap->ga_data) + gap->ga_len
|| fp->fd_top > bot) {

break;
}
if (fp->fd_top >= top) {

fold_changed = true;
if (fp->fd_top + fp->fd_len - 1 > bot) {

foldMarkAdjustRecurse(&fp->fd_nested,
(linenr_T)0, (long)(bot - fp->fd_top),
(linenr_T)MAXLNUM, (long)(fp->fd_top - bot - 1));
fp->fd_len -= bot - fp->fd_top + 1;
fp->fd_top = bot + 1;
break;
}


deleteFoldEntry(gap, (int)(fp - (fold_T *)gap->ga_data), true);
}
}
}


static void foldReverseOrder(
garray_T *gap,
const linenr_T start_arg,
const linenr_T end_arg)
{
linenr_T start = start_arg;
linenr_T end = end_arg;
for (; start < end; start++, end--) {
fold_T *left = (fold_T *)gap->ga_data + start;
fold_T *right = (fold_T *)gap->ga_data + end;
fold_T tmp = *left;
*left = *right;
*right = tmp;
}
}































static void truncate_fold(fold_T *fp, linenr_T end)
{

end += 1;
foldRemove(&fp->fd_nested, end - fp->fd_top, MAXLNUM);
fp->fd_len = end - fp->fd_top;
}

#define FOLD_END(fp) ((fp)->fd_top + (fp)->fd_len - 1)
#define VALID_FOLD(fp, gap) ((fp) < ((fold_T *)(gap)->ga_data + (gap)->ga_len))
#define FOLD_INDEX(fp, gap) ((size_t)(fp - ((fold_T *)(gap)->ga_data)))
void foldMoveRange(garray_T *gap, const linenr_T line1, const linenr_T line2,
const linenr_T dest)
{
fold_T *fp;
const linenr_T range_len = line2 - line1 + 1;
const linenr_T move_len = dest - line2;
const bool at_start = foldFind(gap, line1 - 1, &fp);

if (at_start) {
if (FOLD_END(fp) > dest) {


foldMoveRange(&fp->fd_nested, line1 - fp->fd_top, line2 -
fp->fd_top, dest - fp->fd_top);
return;
} else if (FOLD_END(fp) > line2) {



foldMarkAdjustRecurse(&fp->fd_nested, line1 - fp->fd_top,
line2 - fp->fd_top, MAXLNUM, -range_len);
fp->fd_len -= range_len;
} else {


truncate_fold(fp, line1 - 1);
}


fp = fp + 1;
}

if (!VALID_FOLD(fp, gap) || fp->fd_top > dest) {


return;
} else if (fp->fd_top > line2) {
for (; VALID_FOLD(fp, gap) && FOLD_END(fp) <= dest; fp++) {

fp->fd_top -= range_len;
}
if (VALID_FOLD(fp, gap) && fp->fd_top <= dest) {

truncate_fold(fp, dest);
fp->fd_top -= range_len;
}
return;
} else if (FOLD_END(fp) > dest) {

foldMarkAdjustRecurse(&fp->fd_nested, line2 + 1 - fp->fd_top,
dest - fp->fd_top, MAXLNUM, -move_len);
fp->fd_len -= move_len;
fp->fd_top += move_len;
return;
}



size_t move_start = FOLD_INDEX(fp, gap);
size_t move_end = 0, dest_index = 0;
for (; VALID_FOLD(fp, gap) && fp->fd_top <= dest; fp++) {
if (fp->fd_top <= line2) {

if (FOLD_END(fp) > line2) {

truncate_fold(fp, line2);
}
fp->fd_top += move_len;
continue;
}


if (move_end == 0) {
move_end = FOLD_INDEX(fp, gap);
}

if (FOLD_END(fp) > dest) {
truncate_fold(fp, dest);
}

fp->fd_top -= range_len;
}
dest_index = FOLD_INDEX(fp, gap);




if (move_end == 0) {

return;
}
foldReverseOrder(gap, (linenr_T)move_start, (linenr_T)(dest_index - 1));
foldReverseOrder(gap, (linenr_T)move_start,
(linenr_T)(move_start + dest_index - move_end - 1));
foldReverseOrder(gap, (linenr_T)(move_start + dest_index - move_end),
(linenr_T)(dest_index - 1));
}
#undef FOLD_END
#undef VALID_FOLD
#undef FOLD_INDEX









static void foldMerge(fold_T *fp1, garray_T *gap, fold_T *fp2)
{
fold_T *fp3;
fold_T *fp4;
int idx;
garray_T *gap1 = &fp1->fd_nested;
garray_T *gap2 = &fp2->fd_nested;



if (foldFind(gap1, fp1->fd_len - 1L, &fp3) && foldFind(gap2, 0L, &fp4))
foldMerge(fp3, gap2, fp4);


if (!GA_EMPTY(gap2)) {
ga_grow(gap1, gap2->ga_len);
for (idx = 0; idx < gap2->ga_len; ++idx) {
((fold_T *)gap1->ga_data)[gap1->ga_len]
= ((fold_T *)gap2->ga_data)[idx];
((fold_T *)gap1->ga_data)[gap1->ga_len].fd_top += fp1->fd_len;
++gap1->ga_len;
}
gap2->ga_len = 0;
}

fp1->fd_len += fp2->fd_len;
deleteFoldEntry(gap, (int)(fp2 - (fold_T *)gap->ga_data), true);
fold_changed = true;
}







static void foldlevelIndent(fline_T *flp)
{
char_u *s;
buf_T *buf;
linenr_T lnum = flp->lnum + flp->off;

buf = flp->wp->w_buffer;
s = skipwhite(ml_get_buf(buf, lnum, FALSE));



if (*s == NUL || vim_strchr(flp->wp->w_p_fdi, *s) != NULL) {

if (lnum == 1 || lnum == buf->b_ml.ml_line_count)
flp->lvl = 0;
else
flp->lvl = -1;
} else {
flp->lvl = get_indent_buf(buf, lnum) / get_sw_value(buf);
}
if (flp->lvl > flp->wp->w_p_fdn) {
flp->lvl = (int) MAX(0, flp->wp->w_p_fdn);
}
}






static void foldlevelDiff(fline_T *flp)
{
if (diff_infold(flp->wp, flp->lnum + flp->off))
flp->lvl = 1;
else
flp->lvl = 0;
}







static void foldlevelExpr(fline_T *flp)
{
win_T *win;
int n;
int c;
linenr_T lnum = flp->lnum + flp->off;

win = curwin;
curwin = flp->wp;
curbuf = flp->wp->w_buffer;
set_vim_var_nr(VV_LNUM, (varnumber_T) lnum);

flp->start = 0;
flp->had_end = flp->end;
flp->end = MAX_LEVEL + 1;
if (lnum <= 1)
flp->lvl = 0;



const bool save_keytyped = KeyTyped;
n = (int)eval_foldexpr(flp->wp->w_p_fde, &c);
KeyTyped = save_keytyped;

switch (c) {

case 'a': if (flp->lvl >= 0) {
flp->lvl += n;
flp->lvl_next = flp->lvl;
}
flp->start = n;
break;


case 's': if (flp->lvl >= 0) {
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



if (flp->lvl < 0) {
if (lnum <= 1) {
flp->lvl = 0;
flp->lvl_next = 0;
}
if (lnum == curbuf->b_ml.ml_line_count)
flp->lvl_next = 0;
}

curwin = win;
curbuf = curwin->w_buffer;
}







static void parseMarker(win_T *wp)
{
foldendmarker = vim_strchr(wp->w_p_fmr, ',');
foldstartmarkerlen = (size_t)(foldendmarker++ - wp->w_p_fmr);
foldendmarkerlen = STRLEN(foldendmarker);
}











static void foldlevelMarker(fline_T *flp)
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
while (*s) {
if (*s == cstart
&& STRNCMP(s + 1, startmarker, foldstartmarkerlen - 1) == 0) {

s += foldstartmarkerlen;
if (ascii_isdigit(*s)) {
n = atoi((char *)s);
if (n > 0) {
flp->lvl = n;
flp->lvl_next = n;
if (n <= start_lvl)
flp->start = 1;
else
flp->start = n - start_lvl;
}
} else {
++flp->lvl;
++flp->lvl_next;
++flp->start;
}
} else if (*s == cend && STRNCMP(s + 1, foldendmarker + 1,
foldendmarkerlen - 1) == 0) {

s += foldendmarkerlen;
if (ascii_isdigit(*s)) {
n = atoi((char *)s);
if (n > 0) {
flp->lvl = n;
flp->lvl_next = n - 1;

if (flp->lvl_next > start_lvl)
flp->lvl_next = start_lvl;
}
} else {
flp->lvl_next--;
}
} else {
MB_PTR_ADV(s);
}
}


if (flp->lvl_next < 0)
flp->lvl_next = 0;
}






static void foldlevelSyntax(fline_T *flp)
{
linenr_T lnum = flp->lnum + flp->off;
int n;


flp->lvl = syn_get_foldlevel(flp->wp, lnum);
flp->start = 0;
if (lnum < flp->wp->w_buffer->b_ml.ml_line_count) {
n = syn_get_foldlevel(flp->wp, lnum + 1);
if (n > flp->lvl) {
flp->start = n - flp->lvl; 
flp->lvl = n;
}
}
}








int put_folds(FILE *fd, win_T *wp)
{
if (foldmethodIsManual(wp)) {
if (put_line(fd, "silent! normal! zE") == FAIL
|| put_folds_recurse(fd, &wp->w_folds, (linenr_T)0) == FAIL)
return FAIL;
}


if (wp->w_fold_manual)
return put_foldopen_recurse(fd, wp, &wp->w_folds, (linenr_T)0);

return OK;
}






static int put_folds_recurse(FILE *fd, garray_T *gap, linenr_T off)
{
fold_T *fp = (fold_T *)gap->ga_data;
for (int i = 0; i < gap->ga_len; i++) {

if (put_folds_recurse(fd, &fp->fd_nested, off + fp->fd_top) == FAIL)
return FAIL;
if (fprintf(fd, "%" PRId64 ",%" PRId64 "fold",
(int64_t)(fp->fd_top + off),
(int64_t)(fp->fd_top + off + fp->fd_len - 1)) < 0
|| put_eol(fd) == FAIL)
return FAIL;
++fp;
}
return OK;
}






static int put_foldopen_recurse(FILE *fd, win_T *wp, garray_T *gap, linenr_T off)
{
int level;

fold_T *fp = (fold_T *)gap->ga_data;
for (int i = 0; i < gap->ga_len; i++) {
if (fp->fd_flags != FD_LEVEL) {
if (!GA_EMPTY(&fp->fd_nested)) {

if (fprintf(fd, "%" PRId64, (int64_t)(fp->fd_top + off)) < 0
|| put_eol(fd) == FAIL
|| put_line(fd, "normal! zo") == FAIL)
return FAIL;
if (put_foldopen_recurse(fd, wp, &fp->fd_nested,
off + fp->fd_top)
== FAIL)
return FAIL;

if (fp->fd_flags == FD_CLOSED) {
if (put_fold_open_close(fd, fp, off) == FAIL)
return FAIL;
}
} else {



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






static int put_fold_open_close(FILE *fd, fold_T *fp, linenr_T off)
{
if (fprintf(fd, "%" PRId64, (int64_t)(fp->fd_top + off)) < 0
|| put_eol(fd) == FAIL
|| fprintf(fd, "normal! z%c",
fp->fd_flags == FD_CLOSED ? 'c' : 'o') < 0
|| put_eol(fd) == FAIL)
return FAIL;

return OK;
}


