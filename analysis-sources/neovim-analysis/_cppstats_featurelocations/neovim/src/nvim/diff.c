











#include <inttypes.h>
#include <stdbool.h>

#include "nvim/vim.h"
#include "xdiff/xdiff.h"
#include "nvim/ascii.h"
#include "nvim/diff.h"
#include "nvim/buffer.h"
#include "nvim/change.h"
#include "nvim/charset.h"
#include "nvim/cursor.h"
#include "nvim/eval.h"
#include "nvim/ex_cmds.h"
#include "nvim/ex_docmd.h"
#include "nvim/fileio.h"
#include "nvim/fold.h"
#include "nvim/mark.h"
#include "nvim/mbyte.h"
#include "nvim/memline.h"
#include "nvim/message.h"
#include "nvim/misc1.h"
#include "nvim/memory.h"
#include "nvim/move.h"
#include "nvim/normal.h"
#include "nvim/option.h"
#include "nvim/path.h"
#include "nvim/screen.h"
#include "nvim/strings.h"
#include "nvim/undo.h"
#include "nvim/window.h"
#include "nvim/os/os.h"
#include "nvim/os/shell.h"

static int diff_busy = false; 
static bool diff_need_update = false; 


#define DIFF_FILLER 0x001 
#define DIFF_IBLANK 0x002 
#define DIFF_ICASE 0x004 
#define DIFF_IWHITE 0x008 
#define DIFF_IWHITEALL 0x010 
#define DIFF_IWHITEEOL 0x020 
#define DIFF_HORIZONTAL 0x040 
#define DIFF_VERTICAL 0x080 
#define DIFF_HIDDEN_OFF 0x100 
#define DIFF_INTERNAL 0x200 
#define DIFF_CLOSE_OFF 0x400 
#define ALL_WHITE_DIFF (DIFF_IWHITE | DIFF_IWHITEALL | DIFF_IWHITEEOL)
static int diff_flags = DIFF_INTERNAL | DIFF_FILLER | DIFF_CLOSE_OFF;

static long diff_algorithm = 0;

#define LBUFLEN 50 



static TriState diff_a_works = kNone;


typedef struct {
char_u *din_fname; 
mmfile_t din_mmfile; 
} diffin_T;


typedef struct {
char_u *dout_fname; 
garray_T dout_ga; 
} diffout_T;


typedef struct {
diffin_T dio_orig; 
diffin_T dio_new; 
diffout_T dio_diff; 
int dio_internal; 
} diffio_T;

#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "diff.c.generated.h"
#endif




void diff_buf_delete(buf_T *buf)
{
FOR_ALL_TABS(tp) {
int i = diff_buf_idx_tp(buf, tp);

if (i != DB_COUNT) {
tp->tp_diffbuf[i] = NULL;
tp->tp_diff_invalid = true;

if (tp == curtab) {
diff_redraw(true);
}
}
}
}





void diff_buf_adjust(win_T *win)
{

if (!win->w_p_diff) {


bool found_win = false;
FOR_ALL_WINDOWS_IN_TAB(wp, curtab) {
if ((wp->w_buffer == win->w_buffer) && wp->w_p_diff) {
found_win = true;
}
}

if (!found_win) {
int i = diff_buf_idx(win->w_buffer);
if (i != DB_COUNT) {
curtab->tp_diffbuf[i] = NULL;
curtab->tp_diff_invalid = true;
diff_redraw(true);
}
}
} else {
diff_buf_add(win->w_buffer);
}
}










void diff_buf_add(buf_T *buf)
{
if (diff_buf_idx(buf) != DB_COUNT) {

return;
}

int i;
for (i = 0; i < DB_COUNT; ++i) {
if (curtab->tp_diffbuf[i] == NULL) {
curtab->tp_diffbuf[i] = buf;
curtab->tp_diff_invalid = true;
diff_redraw(true);
return;
}
}

EMSGN(_("E96: Cannot diff more than %" PRId64 " buffers"), DB_COUNT);
}




static void diff_buf_clear(void)
{
for (int i = 0; i < DB_COUNT; i++) {
if (curtab->tp_diffbuf[i] != NULL) {
curtab->tp_diffbuf[i] = NULL;
curtab->tp_diff_invalid = true;
diff_redraw(true);
}
}
}






static int diff_buf_idx(buf_T *buf)
{
int idx;
for (idx = 0; idx < DB_COUNT; ++idx) {
if (curtab->tp_diffbuf[idx] == buf) {
break;
}
}
return idx;
}







static int diff_buf_idx_tp(buf_T *buf, tabpage_T *tp)
{
int idx;
for (idx = 0; idx < DB_COUNT; ++idx) {
if (tp->tp_diffbuf[idx] == buf) {
break;
}
}
return idx;
}





void diff_invalidate(buf_T *buf)
{
FOR_ALL_TABS(tp) {
int i = diff_buf_idx_tp(buf, tp);
if (i != DB_COUNT) {
tp->tp_diff_invalid = true;
if (tp == curtab) {
diff_redraw(true);
}
}
}
}







void diff_mark_adjust(linenr_T line1, linenr_T line2, long amount,
long amount_after)
{

FOR_ALL_TABS(tp) {
int idx = diff_buf_idx_tp(curbuf, tp);
if (idx != DB_COUNT) {
diff_mark_adjust_tp(tp, idx, line1, line2, amount, amount_after);
}
}
}














static void diff_mark_adjust_tp(tabpage_T *tp, int idx, linenr_T line1,
linenr_T line2, long amount, long amount_after)
{
if (diff_internal()) {




tp->tp_diff_invalid = true;
tp->tp_diff_update = true;
}

int inserted;
int deleted;
if (line2 == MAXLNUM) {

inserted = amount;
deleted = 0;
} else if (amount_after > 0) {

inserted = amount_after;
deleted = 0;
} else {

inserted = 0;
deleted = -amount_after;
}

diff_T *dprev = NULL;
diff_T *dp = tp->tp_first_diff;

linenr_T last;
linenr_T lnum_deleted = line1; 
int n;
int off;
for (;;) {



if (((dp == NULL)
|| (dp->df_lnum[idx] - 1 > line2)
|| ((line2 == MAXLNUM) && (dp->df_lnum[idx] > line1)))
&& ((dprev == NULL)
|| (dprev->df_lnum[idx] + dprev->df_count[idx] < line1))
&& !diff_busy) {
diff_T *dnext = diff_alloc_new(tp, dprev, dp);

dnext->df_lnum[idx] = line1;
dnext->df_count[idx] = inserted;
int i;
for (i = 0; i < DB_COUNT; ++i) {
if ((tp->tp_diffbuf[i] != NULL) && (i != idx)) {
if (dprev == NULL) {
dnext->df_lnum[i] = line1;
} else {
dnext->df_lnum[i] = line1
+ (dprev->df_lnum[i] + dprev->df_count[i])
- (dprev->df_lnum[idx] + dprev->df_count[idx]);
}
dnext->df_count[i] = deleted;
}
}
}


if (dp == NULL) {
break;
}













last = dp->df_lnum[idx] + dp->df_count[idx] - 1;


if (last >= line1 - 1) {


if (dp->df_lnum[idx] - (deleted + inserted != 0) > line2) {
if (amount_after == 0) {

break;
}
dp->df_lnum[idx] += amount_after;
} else {
int check_unchanged = false;


if (deleted > 0) {
if (dp->df_lnum[idx] >= line1) {
off = dp->df_lnum[idx] - lnum_deleted;

if (last <= line2) {

if ((dp->df_next != NULL)
&& (dp->df_next->df_lnum[idx] - 1 <= line2)) {


n = dp->df_next->df_lnum[idx] - lnum_deleted;
deleted -= n;
n -= dp->df_count[idx];
lnum_deleted = dp->df_next->df_lnum[idx];
} else {
n = deleted - dp->df_count[idx];
}
dp->df_count[idx] = 0;
} else {

n = off;
dp->df_count[idx] -= line2 - dp->df_lnum[idx] + 1;
check_unchanged = true;
}
dp->df_lnum[idx] = line1;
} else {
off = 0;

if (last < line2) {

dp->df_count[idx] -= last - lnum_deleted + 1;

if ((dp->df_next != NULL)
&& (dp->df_next->df_lnum[idx] - 1 <= line2)) {


n = dp->df_next->df_lnum[idx] - 1 - last;
deleted -= dp->df_next->df_lnum[idx] - lnum_deleted;
lnum_deleted = dp->df_next->df_lnum[idx];
} else {
n = line2 - last;
}
check_unchanged = true;
} else {

n = 0;
dp->df_count[idx] -= deleted;
}
}

int i;
for (i = 0; i < DB_COUNT; ++i) {
if ((tp->tp_diffbuf[i] != NULL) && (i != idx)) {
dp->df_lnum[i] -= off;
dp->df_count[i] += n;
}
}
} else {
if (dp->df_lnum[idx] <= line1) {

dp->df_count[idx] += inserted;
check_unchanged = true;
} else {

dp->df_lnum[idx] += inserted;
}
}

if (check_unchanged) {





diff_check_unchanged(tp, dp);
}
}
}


if ((dprev != NULL)
&& (dprev->df_lnum[idx] + dprev->df_count[idx] == dp->df_lnum[idx])) {
int i;
for (i = 0; i < DB_COUNT; ++i) {
if (tp->tp_diffbuf[i] != NULL) {
dprev->df_count[i] += dp->df_count[i];
}
}
dprev->df_next = dp->df_next;
xfree(dp);
dp = dprev->df_next;
} else {

dprev = dp;
dp = dp->df_next;
}
}

dprev = NULL;
dp = tp->tp_first_diff;

while (dp != NULL) {

int i;
for (i = 0; i < DB_COUNT; ++i) {
if ((tp->tp_diffbuf[i] != NULL) && (dp->df_count[i] != 0)) {
break;
}
}

if (i == DB_COUNT) {
diff_T *dnext = dp->df_next;
xfree(dp);
dp = dnext;

if (dprev == NULL) {
tp->tp_first_diff = dnext;
} else {
dprev->df_next = dnext;
}
} else {

dprev = dp;
dp = dp->df_next;
}
}

if (tp == curtab) {

need_diff_redraw = true;




diff_need_scrollbind = true;
}
}








static diff_T* diff_alloc_new(tabpage_T *tp, diff_T *dprev, diff_T *dp)
{
diff_T *dnew = xmalloc(sizeof(*dnew));

dnew->df_next = dp;
if (dprev == NULL) {
tp->tp_first_diff = dnew;
} else {
dprev->df_next = dnew;
}

return dnew;
}









static void diff_check_unchanged(tabpage_T *tp, diff_T *dp)
{


int i_org;
for (i_org = 0; i_org < DB_COUNT; ++i_org) {
if (tp->tp_diffbuf[i_org] != NULL) {
break;
}
}


if (i_org == DB_COUNT) {
return;
}

if (diff_check_sanity(tp, dp) == FAIL) {
return;
}


int off_org = 0;
int off_new = 0;
int dir = FORWARD;
for (;;) {


while (dp->df_count[i_org] > 0) {

if (dir == BACKWARD) {
off_org = dp->df_count[i_org] - 1;
}
char_u *line_org = vim_strsave(ml_get_buf(tp->tp_diffbuf[i_org],
dp->df_lnum[i_org] + off_org,
false));

int i_new;
for (i_new = i_org + 1; i_new < DB_COUNT; ++i_new) {
if (tp->tp_diffbuf[i_new] == NULL) {
continue;
}

if (dir == BACKWARD) {
off_new = dp->df_count[i_new] - 1;
}


if ((off_new < 0) || (off_new >= dp->df_count[i_new])) {
break;
}

if (diff_cmp(line_org, ml_get_buf(tp->tp_diffbuf[i_new],
dp->df_lnum[i_new] + off_new,
false)) != 0) {
break;
}
}
xfree(line_org);


if (i_new != DB_COUNT) {
break;
}


for (i_new = i_org; i_new < DB_COUNT; ++i_new) {
if (tp->tp_diffbuf[i_new] != NULL) {
if (dir == FORWARD) {
dp->df_lnum[i_new]++;
}
dp->df_count[i_new]--;
}
}
}

if (dir == BACKWARD) {
break;
}
dir = BACKWARD;
}
}








static int diff_check_sanity(tabpage_T *tp, diff_T *dp)
{
int i;
for (i = 0; i < DB_COUNT; ++i) {
if (tp->tp_diffbuf[i] != NULL) {
if (dp->df_lnum[i] + dp->df_count[i] - 1
> tp->tp_diffbuf[i]->b_ml.ml_line_count) {
return FAIL;
}
}
}
return OK;
}




void diff_redraw(bool dofold)
{
need_diff_redraw = false;
FOR_ALL_WINDOWS_IN_TAB(wp, curtab) {
if (!wp->w_p_diff) {
continue;
}
redraw_win_later(wp, SOME_VALID);
if (dofold && foldmethodIsDiff(wp)) {
foldUpdateAll(wp);
}



int n = diff_check(wp, wp->w_topline);

if (((wp != curwin) && (wp->w_topfill > 0)) || (n > 0)) {
if (wp->w_topfill > n) {
wp->w_topfill = (n < 0 ? 0 : n);
} else if ((n > 0) && (n > wp->w_topfill)) {
wp->w_topfill = n;
}
check_topfill(wp, false);
}
}
}

static void clear_diffin(diffin_T *din)
{
if (din->din_fname == NULL) {
XFREE_CLEAR(din->din_mmfile.ptr);
} else {
os_remove((char *)din->din_fname);
}
}

static void clear_diffout(diffout_T *dout)
{
if (dout->dout_fname == NULL) {
ga_clear_strings(&dout->dout_ga);
} else {
os_remove((char *)dout->dout_fname);
}
}







static int diff_write_buffer(buf_T *buf, diffin_T *din)
{
linenr_T lnum;
char_u *s;
long len = 0;
char_u *ptr;


for (lnum = 1; lnum <= buf->b_ml.ml_line_count; lnum++) {
len += (long)STRLEN(ml_get_buf(buf, lnum, false)) + 1;
}
ptr = try_malloc(len);
if (ptr == NULL) {



buf->b_diff_failed = true;
if (p_verbose > 0) {
verbose_enter();
smsg(_("Not enough memory to use internal diff for buffer \"%s\""),
buf->b_fname);
verbose_leave();
}
return FAIL;
}
din->din_mmfile.ptr = (char *)ptr;
din->din_mmfile.size = len;

len = 0;
for (lnum = 1; lnum <= buf->b_ml.ml_line_count; lnum++) {
for (s = ml_get_buf(buf, lnum, false); *s != NUL; ) {
if (diff_flags & DIFF_ICASE) {
int c;


int orig_len;
char_u cbuf[MB_MAXBYTES + 1];

c = PTR2CHAR(s);
c = utf_fold(c);
orig_len = utfc_ptr2len(s);
if (utf_char2bytes(c, cbuf) != orig_len) {

memmove(ptr + len, s, orig_len);
} else {
memmove(ptr + len, cbuf, orig_len);
}

s += orig_len;
len += orig_len;
} else {
ptr[len++] = *s++;
}
}
ptr[len++] = NL;
}
return OK;
}









static int diff_write(buf_T *buf, diffin_T *din)
{
if (din->din_fname == NULL) {
return diff_write_buffer(buf, din);
}


char_u *save_ff = buf->b_p_ff;
buf->b_p_ff = vim_strsave((char_u *)FF_UNIX);
int r = buf_write(buf, din->din_fname, NULL,
(linenr_T)1, buf->b_ml.ml_line_count,
NULL, false, false, false, true);
free_string_option(buf->b_p_ff);
buf->b_p_ff = save_ff;
return r;
}







static void diff_try_update(diffio_T *dio,
int idx_orig,
exarg_T *eap)
{
buf_T *buf;
int idx_new;

if (dio->dio_internal) {
ga_init(&dio->dio_diff.dout_ga, sizeof(char *), 1000);
} else {

dio->dio_orig.din_fname = vim_tempname();
dio->dio_new.din_fname = vim_tempname();
dio->dio_diff.dout_fname = vim_tempname();
if (dio->dio_orig.din_fname == NULL
|| dio->dio_new.din_fname == NULL
|| dio->dio_diff.dout_fname == NULL) {
goto theend;
}
}


if (!dio->dio_internal && check_external_diff(dio) == FAIL) {
goto theend;
}


if (eap != NULL && eap->forceit) {
for (idx_new = idx_orig; idx_new < DB_COUNT; idx_new++) {
buf = curtab->tp_diffbuf[idx_new];
if (buf_valid(buf)) {
buf_check_timestamp(buf, false);
}
}
}


buf = curtab->tp_diffbuf[idx_orig];
if (diff_write(buf, &dio->dio_orig) == FAIL) {
goto theend;
}


for (idx_new = idx_orig + 1; idx_new < DB_COUNT; idx_new++) {
buf = curtab->tp_diffbuf[idx_new];
if (buf == NULL || buf->b_ml.ml_mfp == NULL) {
continue; 
}


if (diff_write(buf, &dio->dio_new) == FAIL) {
continue;
}
if (diff_file(dio) == FAIL) {
continue;
}


diff_read(idx_orig, idx_new, &dio->dio_diff);

clear_diffin(&dio->dio_new);
clear_diffout(&dio->dio_diff);
}
clear_diffin(&dio->dio_orig);

theend:
xfree(dio->dio_orig.din_fname);
xfree(dio->dio_new.din_fname);
xfree(dio->dio_diff.dout_fname);
}






int diff_internal(void)
{
return (diff_flags & DIFF_INTERNAL) != 0 && *p_dex == NUL;
}




static int diff_internal_failed(void)
{
int idx;


for (idx = 0; idx < DB_COUNT; idx++) {
if (curtab->tp_diffbuf[idx] != NULL
&& curtab->tp_diffbuf[idx]->b_diff_failed) {
return true;
}
}
return false;
}








void ex_diffupdate(exarg_T *eap)
{
if (diff_busy) {
diff_need_update = true;
return;
}

int had_diffs = curtab->tp_first_diff != NULL;


diff_clear(curtab);
curtab->tp_diff_invalid = false;


int idx_orig;
for (idx_orig = 0; idx_orig < DB_COUNT; ++idx_orig) {
if (curtab->tp_diffbuf[idx_orig] != NULL) {
break;
}
}

if (idx_orig == DB_COUNT) {
goto theend;
}


int idx_new;
for (idx_new = idx_orig + 1; idx_new < DB_COUNT; ++idx_new) {
if (curtab->tp_diffbuf[idx_new] != NULL) {
break;
}
}

if (idx_new == DB_COUNT) {
goto theend;
}


diffio_T diffio;
memset(&diffio, 0, sizeof(diffio));
diffio.dio_internal = diff_internal() && !diff_internal_failed();

diff_try_update(&diffio, idx_orig, eap);
if (diffio.dio_internal && diff_internal_failed()) {

memset(&diffio, 0, sizeof(diffio));
diff_try_update(&diffio, idx_orig, eap);
}


curwin->w_valid_cursor.lnum = 0;

theend:


if (had_diffs || curtab->tp_first_diff != NULL) {
diff_redraw(true);
apply_autocmds(EVENT_DIFFUPDATED, NULL, NULL, false, curbuf);
}
}






static int check_external_diff(diffio_T *diffio)
{

int io_error = false;
TriState ok = kFalse;
for (;;) {
ok = kFalse;
FILE *fd = os_fopen((char *)diffio->dio_orig.din_fname, "w");

if (fd == NULL) {
io_error = true;
} else {
if (fwrite("line1\n", (size_t)6, (size_t)1, fd) != 1) {
io_error = true;
}
fclose(fd);
fd = os_fopen((char *)diffio->dio_new.din_fname, "w");

if (fd == NULL) {
io_error = true;
} else {
if (fwrite("line2\n", (size_t)6, (size_t)1, fd) != 1) {
io_error = true;
}
fclose(fd);
fd = NULL;
if (diff_file(diffio) == OK) {
fd = os_fopen((char *)diffio->dio_diff.dout_fname, "r");
}

if (fd == NULL) {
io_error = true;
} else {
char_u linebuf[LBUFLEN];

for (;;) {

if (vim_fgets(linebuf, LBUFLEN, fd)) {
break;
}

if (STRNCMP(linebuf, "1c1", 3) == 0) {
ok = kTrue;
}
}
fclose(fd);
}
os_remove((char *)diffio->dio_diff.dout_fname);
os_remove((char *)diffio->dio_new.din_fname);
}
os_remove((char *)diffio->dio_orig.din_fname);
}


if (*p_dex != NUL) {
break;
}


if (diff_a_works != kNone) {
break;
}
diff_a_works = ok;


if (ok) {
break;
}
}

if (!ok) {
if (io_error) {
EMSG(_("E810: Cannot read or write temp files"));
}
EMSG(_("E97: Cannot create diffs"));
diff_a_works = kNone;
return FAIL;
}
return OK;
}




static int diff_file_internal(diffio_T *diffio)
{
xpparam_t param;
xdemitconf_t emit_cfg;
xdemitcb_t emit_cb;

memset(&param, 0, sizeof(param));
memset(&emit_cfg, 0, sizeof(emit_cfg));
memset(&emit_cb, 0, sizeof(emit_cb));

param.flags = diff_algorithm;

if (diff_flags & DIFF_IWHITE) {
param.flags |= XDF_IGNORE_WHITESPACE_CHANGE;
}
if (diff_flags & DIFF_IWHITEALL) {
param.flags |= XDF_IGNORE_WHITESPACE;
}
if (diff_flags & DIFF_IWHITEEOL) {
param.flags |= XDF_IGNORE_WHITESPACE_AT_EOL;
}
if (diff_flags & DIFF_IBLANK) {
param.flags |= XDF_IGNORE_BLANK_LINES;
}

emit_cfg.ctxlen = 0; 
emit_cb.priv = &diffio->dio_diff;
emit_cb.outf = xdiff_out;
if (xdl_diff(&diffio->dio_orig.din_mmfile,
&diffio->dio_new.din_mmfile,
&param, &emit_cfg, &emit_cb) < 0) {
EMSG(_("E960: Problem creating the internal diff"));
return FAIL;
}
return OK;
}






static int diff_file(diffio_T *dio)
{
char *tmp_orig = (char *)dio->dio_orig.din_fname;
char *tmp_new = (char *)dio->dio_new.din_fname;
char *tmp_diff = (char *)dio->dio_diff.dout_fname;
if (*p_dex != NUL) {

eval_diff(tmp_orig, tmp_new, tmp_diff);
return OK;
}

if (dio->dio_internal) {
return diff_file_internal(dio);
} else {
const size_t len = (strlen(tmp_orig) + strlen(tmp_new) + strlen(tmp_diff)
+ STRLEN(p_srr) + 27);
char *const cmd = xmalloc(len);


if (os_getenv("DIFF_OPTIONS")) {
os_unsetenv("DIFF_OPTIONS");
}




vim_snprintf((char *)cmd, len, "diff %s%s%s%s%s%s%s%s %s",
diff_a_works == kFalse ? "" : "-a ",
"",
(diff_flags & DIFF_IWHITE) ? "-b " : "",
(diff_flags & DIFF_IWHITEALL) ? "-w " : "",
(diff_flags & DIFF_IWHITEEOL) ? "-Z " : "",
(diff_flags & DIFF_IBLANK) ? "-B " : "",
(diff_flags & DIFF_ICASE) ? "-i " : "",
tmp_orig, tmp_new);
append_redir(cmd, len, (char *) p_srr, tmp_diff);
block_autocmds(); 
(void)call_shell((char_u *) cmd,
kShellOptFilter | kShellOptSilent | kShellOptDoOut,
NULL);
unblock_autocmds();
xfree(cmd);
return OK;
}
}







void ex_diffpatch(exarg_T *eap)
{
char_u *buf = NULL;
win_T *old_curwin = curwin;
char_u *newname = NULL; 
char_u *esc_name = NULL;

#if defined(UNIX)
char *fullname = NULL;
#endif



char_u *tmp_orig = vim_tempname();

char_u *tmp_new = vim_tempname();

if ((tmp_orig == NULL) || (tmp_new == NULL)) {
goto theend;
}


if (buf_write(curbuf, tmp_orig, NULL,
(linenr_T)1, curbuf->b_ml.ml_line_count,
NULL, false, false, false, true) == FAIL) {
goto theend;
}

#if defined(UNIX)

fullname = FullName_save((char *)eap->arg, false);
esc_name = vim_strsave_shellescape(
(fullname != NULL ? (char_u *)fullname : eap->arg), true, true);
#else
esc_name = vim_strsave_shellescape(eap->arg, true, true);
#endif
size_t buflen = STRLEN(tmp_orig) + STRLEN(esc_name) + STRLEN(tmp_new) + 16;
buf = xmalloc(buflen);

#if defined(UNIX)
char_u dirbuf[MAXPATHL];





if ((os_dirname(dirbuf, MAXPATHL) != OK)
|| (os_chdir((char *)dirbuf) != 0)) {
dirbuf[0] = NUL;
} else {
char *tempdir = (char *)vim_gettempdir();
if (tempdir == NULL) {
tempdir = "/tmp";
}
os_chdir(tempdir);
shorten_fnames(true);
}
#endif

if (*p_pex != NUL) {

#if defined(UNIX)
eval_patch((char *)tmp_orig,
(fullname != NULL ? fullname : (char *)eap->arg),
(char *)tmp_new);
#else
eval_patch((char *)tmp_orig, (char *)eap->arg, (char *)tmp_new);
#endif
} else {

vim_snprintf((char *)buf, buflen, "patch -o %s %s < %s",
tmp_new, tmp_orig, esc_name);
block_autocmds(); 
(void)call_shell(buf, kShellOptFilter, NULL);
unblock_autocmds();
}

#if defined(UNIX)
if (dirbuf[0] != NUL) {
if (os_chdir((char *)dirbuf) != 0) {
EMSG(_(e_prev_dir));
}
shorten_fnames(true);
}
#endif


STRCPY(buf, tmp_new);
STRCAT(buf, ".orig");
os_remove((char *)buf);
STRCPY(buf, tmp_new);
STRCAT(buf, ".rej");
os_remove((char *)buf);


FileInfo file_info;
bool info_ok = os_fileinfo((char *)tmp_new, &file_info);
uint64_t filesize = os_fileinfo_size(&file_info);
if (!info_ok || filesize == 0) {
EMSG(_("E816: Cannot read patch output"));
} else {
if (curbuf->b_fname != NULL) {
newname = vim_strnsave(curbuf->b_fname,
(int)(STRLEN(curbuf->b_fname) + 4));
STRCAT(newname, ".new");
}


cmdmod.tab = 0;

if (win_split(0, (diff_flags & DIFF_VERTICAL) ? WSP_VERT : 0) != FAIL) {

eap->cmdidx = CMD_split;
eap->arg = tmp_new;
do_exedit(eap, old_curwin);


if ((curwin != old_curwin) && win_valid(old_curwin)) {

diff_win_options(curwin, true);
diff_win_options(old_curwin, true);

if (newname != NULL) {

eap->arg = newname;
ex_file(eap);


if (au_has_group((char_u *)"filetypedetect")) {
do_cmdline_cmd(":doau filetypedetect BufRead");
}
}
}
}
}

theend:
if (tmp_orig != NULL) {
os_remove((char *)tmp_orig);
}
xfree(tmp_orig);

if (tmp_new != NULL) {
os_remove((char *)tmp_new);
}
xfree(tmp_new);
xfree(newname);
xfree(buf);
#if defined(UNIX)
xfree(fullname);
#endif
xfree(esc_name);
}




void ex_diffsplit(exarg_T *eap)
{
win_T *old_curwin = curwin;
bufref_T old_curbuf;
set_bufref(&old_curbuf, curbuf);


validate_cursor();
set_fraction(curwin);


cmdmod.tab = 0;

if (win_split(0, (diff_flags & DIFF_VERTICAL) ? WSP_VERT : 0) != FAIL) {

eap->cmdidx = CMD_split;
curwin->w_p_diff = true;
do_exedit(eap, old_curwin);


if (curwin != old_curwin) {

diff_win_options(curwin, true);
if (win_valid(old_curwin)) {
diff_win_options(old_curwin, true);

if (bufref_valid(&old_curbuf)) {

curwin->w_cursor.lnum = diff_get_corresponding_line(
old_curbuf.br_buf, old_curwin->w_cursor.lnum);
}
}


scroll_to_fraction(curwin, curwin->w_height);
}
}
}


void ex_diffthis(exarg_T *eap)
{

diff_win_options(curwin, true);
}

static void set_diff_option(win_T *wp, int value)
{
win_T *old_curwin = curwin;

curwin = wp;
curbuf = curwin->w_buffer;
curbuf_lock++;
set_option_value("diff", (long)value, NULL, OPT_LOCAL);
curbuf_lock--;
curwin = old_curwin;
curbuf = curwin->w_buffer;
}





void diff_win_options(win_T *wp, int addbuf)
{
win_T *old_curwin = curwin;


curwin = wp;
newFoldLevel();
curwin = old_curwin;


if (!wp->w_p_diff) {
wp->w_p_scb_save = wp->w_p_scb;
}
wp->w_p_scb = true;

if (!wp->w_p_diff) {
wp->w_p_crb_save = wp->w_p_crb;
}
wp->w_p_crb = true;

if (!wp->w_p_diff) {
wp->w_p_wrap_save = wp->w_p_wrap;
}
wp->w_p_wrap = false;
curwin = wp; 
curbuf = curwin->w_buffer;

if (!wp->w_p_diff) {
if (wp->w_p_diff_saved) {
free_string_option(wp->w_p_fdm_save);
}
wp->w_p_fdm_save = vim_strsave(wp->w_p_fdm);
}
set_string_option_direct((char_u *)"fdm", -1, (char_u *)"diff",
OPT_LOCAL | OPT_FREE, 0);
curwin = old_curwin;
curbuf = curwin->w_buffer;

if (!wp->w_p_diff) {
wp->w_p_fen_save = wp->w_p_fen;
wp->w_p_fdl_save = wp->w_p_fdl;

if (wp->w_p_diff_saved) {
free_string_option(wp->w_p_fdc_save);
}
wp->w_p_fdc_save = vim_strsave(wp->w_p_fdc);
}
xfree(wp->w_p_fdc);
wp->w_p_fdc = (char_u *)xstrdup("2");
assert(diff_foldcolumn >= 0 && diff_foldcolumn <= 9);
snprintf((char *)wp->w_p_fdc, STRLEN(wp->w_p_fdc) + 1, "%d", diff_foldcolumn);
wp->w_p_fen = true;
wp->w_p_fdl = 0;
foldUpdateAll(wp);


changed_window_setting_win(wp);
if (vim_strchr(p_sbo, 'h') == NULL) {
do_cmdline_cmd("set sbo+=hor");
}


wp->w_p_diff_saved = true;

set_diff_option(wp, true);

if (addbuf) {
diff_buf_add(wp->w_buffer);
}
redraw_win_later(wp, NOT_VALID);
}





void ex_diffoff(exarg_T *eap)
{
int diffwin = false;

FOR_ALL_WINDOWS_IN_TAB(wp, curtab) {
if (eap->forceit ? wp->w_p_diff : (wp == curwin)) {



set_diff_option(wp, false);

if (wp->w_p_diff_saved) {
if (wp->w_p_scb) {
wp->w_p_scb = wp->w_p_scb_save;
}

if (wp->w_p_crb) {
wp->w_p_crb = wp->w_p_crb_save;
}

if (!wp->w_p_wrap) {
wp->w_p_wrap = wp->w_p_wrap_save;
}

free_string_option(wp->w_p_fdm);
wp->w_p_fdm = vim_strsave(*wp->w_p_fdm_save
? wp->w_p_fdm_save
: (char_u *)"manual");
free_string_option(wp->w_p_fdc);
wp->w_p_fdc = vim_strsave(wp->w_p_fdc_save);

if (wp->w_p_fdl == 0) {
wp->w_p_fdl = wp->w_p_fdl_save;
}


if (wp->w_p_fen) {
wp->w_p_fen = foldmethodIsManual(wp) ? false : wp->w_p_fen_save;
}

foldUpdateAll(wp);
}

wp->w_topfill = 0;



changed_window_setting_win(wp);


diff_buf_adjust(wp);
}
diffwin |= wp->w_p_diff;
}


if (eap->forceit) {
diff_buf_clear();
}

if (!diffwin) {
diff_need_update = false;
curtab->tp_diff_invalid = false;
curtab->tp_diff_update = false;
diff_clear(curtab);
}


if (!diffwin && (vim_strchr(p_sbo, 'h') != NULL)) {
do_cmdline_cmd("set sbo-=hor");
}
}






static void diff_read(int idx_orig, int idx_new, diffout_T *dout)
{
FILE *fd = NULL;
int line_idx = 0;
diff_T *dprev = NULL;
diff_T *dp = curtab->tp_first_diff;
diff_T *dn, *dpl;
char_u linebuf[LBUFLEN]; 
char_u *line;
long off;
int i;
linenr_T lnum_orig, lnum_new;
long count_orig, count_new;
int notset = true; 
enum {
DIFF_ED,
DIFF_UNIFIED,
DIFF_NONE
} diffstyle = DIFF_NONE;

if (dout->dout_fname == NULL) {
diffstyle = DIFF_UNIFIED;
} else {
fd = os_fopen((char *)dout->dout_fname, "r");
if (fd == NULL) {
EMSG(_("E98: Cannot read diff output"));
return;
}
}

for (;;) {
if (fd == NULL) {
if (line_idx >= dout->dout_ga.ga_len) {
break; 
}
line = ((char_u **)dout->dout_ga.ga_data)[line_idx++];
} else {
if (vim_fgets(linebuf, LBUFLEN, fd)) {
break; 
}
line = linebuf;
}

if (diffstyle == DIFF_NONE) {










if (isdigit(*line)) {
diffstyle = DIFF_ED;
} else if ((STRNCMP(line, "@@ ", 3) == 0)) {
diffstyle = DIFF_UNIFIED;
} else if ((STRNCMP(line, "--- ", 4) == 0) 
&& (vim_fgets(linebuf, LBUFLEN, fd) == 0) 
&& (STRNCMP(line, "+++ ", 4) == 0)
&& (vim_fgets(linebuf, LBUFLEN, fd) == 0) 
&& (STRNCMP(line, "@@ ", 3) == 0)) {
diffstyle = DIFF_UNIFIED;
} else {


continue;
}
}

if (diffstyle == DIFF_ED) {
if (!isdigit(*line)) {
continue; 
}
if (parse_diff_ed(line, &lnum_orig, &count_orig,
&lnum_new, &count_new) == FAIL) {
continue;
}
} else {
assert(diffstyle == DIFF_UNIFIED);
if (STRNCMP(line, "@@ ", 3) != 0) {
continue; 
}
if (parse_diff_unified(line, &lnum_orig, &count_orig,
&lnum_new, &count_new) == FAIL) {
continue;
}
}



while (dp != NULL
&& lnum_orig > dp->df_lnum[idx_orig] + dp->df_count[idx_orig]) {
if (notset) {
diff_copy_entry(dprev, dp, idx_orig, idx_new);
}
dprev = dp;
dp = dp->df_next;
notset = true;
}

if ((dp != NULL)
&& (lnum_orig <= dp->df_lnum[idx_orig] + dp->df_count[idx_orig])
&& (lnum_orig + count_orig >= dp->df_lnum[idx_orig])) {


for (dpl = dp; dpl->df_next != NULL; dpl = dpl->df_next) {
if (lnum_orig + count_orig < dpl->df_next->df_lnum[idx_orig]) {
break;
}
}



off = dp->df_lnum[idx_orig] - lnum_orig;

if (off > 0) {
for (i = idx_orig; i < idx_new; ++i) {
if (curtab->tp_diffbuf[i] != NULL) {
dp->df_lnum[i] -= off;
}
}
dp->df_lnum[idx_new] = lnum_new;
dp->df_count[idx_new] = count_new;
} else if (notset) {

dp->df_lnum[idx_new] = lnum_new + off;
dp->df_count[idx_new] = count_new - off;
} else {

dp->df_count[idx_new] += count_new - count_orig
+ dpl->df_lnum[idx_orig] +
dpl->df_count[idx_orig]
- (dp->df_lnum[idx_orig] +
dp->df_count[idx_orig]);
}



off = (lnum_orig + count_orig)
- (dpl->df_lnum[idx_orig] + dpl->df_count[idx_orig]);

if (off < 0) {


if (notset) {
dp->df_count[idx_new] += -off;
}
off = 0;
}

for (i = idx_orig; i < idx_new; ++i) {
if (curtab->tp_diffbuf[i] != NULL) {
dp->df_count[i] = dpl->df_lnum[i] + dpl->df_count[i]
- dp->df_lnum[i] + off;
}
}


dn = dp->df_next;
dp->df_next = dpl->df_next;

while (dn != dp->df_next) {
dpl = dn->df_next;
xfree(dn);
dn = dpl;
}
} else {

dp = diff_alloc_new(curtab, dprev, dp);

dp->df_lnum[idx_orig] = lnum_orig;
dp->df_count[idx_orig] = count_orig;
dp->df_lnum[idx_new] = lnum_new;
dp->df_count[idx_new] = count_new;




for (i = idx_orig + 1; i < idx_new; ++i) {
if (curtab->tp_diffbuf[i] != NULL) {
diff_copy_entry(dprev, dp, idx_orig, i);
}
}
}
notset = false; 
}


while (dp != NULL) {
if (notset) {
diff_copy_entry(dprev, dp, idx_orig, idx_new);
}
dprev = dp;
dp = dp->df_next;
notset = true;
}

if (fd != NULL) {
fclose(fd);
}
}







static void diff_copy_entry(diff_T *dprev, diff_T *dp, int idx_orig,
int idx_new)
{
long off;

if (dprev == NULL) {
off = 0;
} else {
off = (dprev->df_lnum[idx_orig] + dprev->df_count[idx_orig])
- (dprev->df_lnum[idx_new] + dprev->df_count[idx_new]);
}
dp->df_lnum[idx_new] = dp->df_lnum[idx_orig] - off;
dp->df_count[idx_new] = dp->df_count[idx_orig];
}




void diff_clear(tabpage_T *tp)
FUNC_ATTR_NONNULL_ALL
{
diff_T *p;
diff_T *next_p;
for (p = tp->tp_first_diff; p != NULL; p = next_p) {
next_p = p->df_next;
xfree(p);
}
tp->tp_first_diff = NULL;
}














int diff_check(win_T *wp, linenr_T lnum)
{
int idx; 
diff_T *dp;
int maxcount;
int i;
buf_T *buf = wp->w_buffer;
int cmp;

if (curtab->tp_diff_invalid) {

ex_diffupdate(NULL);
}


if ((curtab->tp_first_diff == NULL) || !wp->w_p_diff) {
return 0;
}


if ((lnum < 1) || (lnum > buf->b_ml.ml_line_count + 1)) {
return 0;
}

idx = diff_buf_idx(buf);

if (idx == DB_COUNT) {

return 0;
}


if (hasFoldingWin(wp, lnum, NULL, NULL, true, NULL)) {
return 0;
}


for (dp = curtab->tp_first_diff; dp != NULL; dp = dp->df_next) {
if (lnum <= dp->df_lnum[idx] + dp->df_count[idx]) {
break;
}
}

if ((dp == NULL) || (lnum < dp->df_lnum[idx])) {
return 0;
}

if (lnum < dp->df_lnum[idx] + dp->df_count[idx]) {
int zero = false;




cmp = false;

for (i = 0; i < DB_COUNT; ++i) {
if ((i != idx) && (curtab->tp_diffbuf[i] != NULL)) {
if (dp->df_count[i] == 0) {
zero = true;
} else {
if (dp->df_count[i] != dp->df_count[idx]) {

return -1;
}
cmp = true;
}
}
}

if (cmp) {


for (i = 0; i < DB_COUNT; ++i) {
if ((i != idx)
&& (curtab->tp_diffbuf[i] != NULL)
&& (dp->df_count[i] != 0)) {
if (!diff_equal_entry(dp, idx, i)) {
return -1;
}
}
}
}






if (zero == false) {
return 0;
}
return -2;
}


if (!(diff_flags & DIFF_FILLER)) {
return 0;
}



maxcount = 0;
for (i = 0; i < DB_COUNT; ++i) {
if ((curtab->tp_diffbuf[i] != NULL) && (dp->df_count[i] > maxcount)) {
maxcount = dp->df_count[i];
}
}
return maxcount - dp->df_count[idx];
}








static bool diff_equal_entry(diff_T *dp, int idx1, int idx2)
FUNC_ATTR_PURE FUNC_ATTR_WARN_UNUSED_RESULT FUNC_ATTR_NONNULL_ARG(1)
{
if (dp->df_count[idx1] != dp->df_count[idx2]) {
return false;
}

if (diff_check_sanity(curtab, dp) == FAIL) {
return false;
}

for (int i = 0; i < dp->df_count[idx1]; i++) {
char_u *line = vim_strsave(ml_get_buf(curtab->tp_diffbuf[idx1],
dp->df_lnum[idx1] + i, false));

int cmp = diff_cmp(line, ml_get_buf(curtab->tp_diffbuf[idx2],
dp->df_lnum[idx2] + i, false));
xfree(line);

if (cmp != 0) {
return false;
}
}
return true;
}



static bool diff_equal_char(const char_u *const p1, const char_u *const p2,
int *const len)
{
const int l = utfc_ptr2len(p1);

if (l != utfc_ptr2len(p2)) {
return false;
}
if (l > 1) {
if (STRNCMP(p1, p2, l) != 0
&& (!(diff_flags & DIFF_ICASE)
|| utf_fold(utf_ptr2char(p1)) != utf_fold(utf_ptr2char(p2)))) {
return false;
}
*len = l;
} else {
if ((*p1 != *p2)
&& (!(diff_flags & DIFF_ICASE)
|| TOLOWER_LOC(*p1) != TOLOWER_LOC(*p2))) {
return false;
}
*len = 1;
}
return true;
}








static int diff_cmp(char_u *s1, char_u *s2)
{
if ((diff_flags & DIFF_IBLANK)
&& (*skipwhite(s1) == NUL || *skipwhite(s2) == NUL)) {
return 0;
}

if ((diff_flags & (DIFF_ICASE | ALL_WHITE_DIFF)) == 0) {
return STRCMP(s1, s2);
}

if ((diff_flags & DIFF_ICASE) && !(diff_flags & ALL_WHITE_DIFF)) {
return mb_stricmp((const char *)s1, (const char *)s2);
}

char_u *p1 = s1;
char_u *p2 = s2;


while (*p1 != NUL && *p2 != NUL) {
if (((diff_flags & DIFF_IWHITE)
&& ascii_iswhite(*p1) && ascii_iswhite(*p2))
|| ((diff_flags & DIFF_IWHITEALL)
&& (ascii_iswhite(*p1) || ascii_iswhite(*p2)))) {
p1 = skipwhite(p1);
p2 = skipwhite(p2);
} else {
int l;
if (!diff_equal_char(p1, p2, &l)) {
break;
}
p1 += l;
p2 += l;
}
}


p1 = skipwhite(p1);
p2 = skipwhite(p2);

if ((*p1 != NUL) || (*p2 != NUL)) {
return 1;
}
return 0;
}







int diff_check_fill(win_T *wp, linenr_T lnum)
{

if (!(diff_flags & DIFF_FILLER)) {
return 0;
}
int n = diff_check(wp, lnum);

if (n <= 0) {
return 0;
}
return n;
}






void diff_set_topline(win_T *fromwin, win_T *towin)
{
buf_T *frombuf = fromwin->w_buffer;
linenr_T lnum = fromwin->w_topline;
diff_T *dp;
int max_count;
int i;

int fromidx = diff_buf_idx(frombuf);
if (fromidx == DB_COUNT) {

return;
}

if (curtab->tp_diff_invalid) {

ex_diffupdate(NULL);
}
towin->w_topfill = 0;


for (dp = curtab->tp_first_diff; dp != NULL; dp = dp->df_next) {
if (lnum <= dp->df_lnum[fromidx] + dp->df_count[fromidx]) {
break;
}
}

if (dp == NULL) {


towin->w_topline = towin->w_buffer->b_ml.ml_line_count
- (frombuf->b_ml.ml_line_count - lnum);
} else {

int toidx = diff_buf_idx(towin->w_buffer);

if (toidx == DB_COUNT) {

return;
}
towin->w_topline = lnum + (dp->df_lnum[toidx] - dp->df_lnum[fromidx]);

if (lnum >= dp->df_lnum[fromidx]) {


max_count = 0;

for (i = 0; i < DB_COUNT; ++i) {
if ((curtab->tp_diffbuf[i] != NULL) && (max_count < dp->df_count[i])) {
max_count = dp->df_count[i];
}
}

if (dp->df_count[toidx] == dp->df_count[fromidx]) {

towin->w_topfill = fromwin->w_topfill;
} else if (dp->df_count[toidx] > dp->df_count[fromidx]) {
if (lnum == dp->df_lnum[fromidx] + dp->df_count[fromidx]) {


if (max_count - fromwin->w_topfill >= dp->df_count[toidx]) {

towin->w_topline = dp->df_lnum[toidx] + dp->df_count[toidx];
towin->w_topfill = fromwin->w_topfill;
} else {

towin->w_topline = dp->df_lnum[toidx]
+ max_count - fromwin->w_topfill;
}
}
} else if (towin->w_topline >= dp->df_lnum[toidx]
+ dp->df_count[toidx]) {


towin->w_topline = dp->df_lnum[toidx] + dp->df_count[toidx];

if (diff_flags & DIFF_FILLER) {
if (lnum == dp->df_lnum[fromidx] + dp->df_count[fromidx]) {

towin->w_topfill = fromwin->w_topfill;
} else {

towin->w_topfill = dp->df_lnum[fromidx] + max_count - lnum;
}
}
}
}
}


towin->w_botfill = false;

if (towin->w_topline > towin->w_buffer->b_ml.ml_line_count) {
towin->w_topline = towin->w_buffer->b_ml.ml_line_count;
towin->w_botfill = true;
}

if (towin->w_topline < 1) {
towin->w_topline = 1;
towin->w_topfill = 0;
}


invalidate_botline_win(towin);
changed_line_abv_curs_win(towin);

check_topfill(towin, false);
(void)hasFoldingWin(towin, towin->w_topline, &towin->w_topline,
NULL, true, NULL);
}




int diffopt_changed(void)
{
int diff_context_new = 6;
int diff_flags_new = 0;
int diff_foldcolumn_new = 2;
long diff_algorithm_new = 0;
long diff_indent_heuristic = 0;

char_u *p = p_dip;
while (*p != NUL) {
if (STRNCMP(p, "filler", 6) == 0) {
p += 6;
diff_flags_new |= DIFF_FILLER;
} else if ((STRNCMP(p, "context:", 8) == 0) && ascii_isdigit(p[8])) {
p += 8;
diff_context_new = getdigits_int(&p, false, diff_context_new);
} else if (STRNCMP(p, "iblank", 6) == 0) {
p += 6;
diff_flags_new |= DIFF_IBLANK;
} else if (STRNCMP(p, "icase", 5) == 0) {
p += 5;
diff_flags_new |= DIFF_ICASE;
} else if (STRNCMP(p, "iwhiteall", 9) == 0) {
p += 9;
diff_flags_new |= DIFF_IWHITEALL;
} else if (STRNCMP(p, "iwhiteeol", 9) == 0) {
p += 9;
diff_flags_new |= DIFF_IWHITEEOL;
} else if (STRNCMP(p, "iwhite", 6) == 0) {
p += 6;
diff_flags_new |= DIFF_IWHITE;
} else if (STRNCMP(p, "horizontal", 10) == 0) {
p += 10;
diff_flags_new |= DIFF_HORIZONTAL;
} else if (STRNCMP(p, "vertical", 8) == 0) {
p += 8;
diff_flags_new |= DIFF_VERTICAL;
} else if ((STRNCMP(p, "foldcolumn:", 11) == 0) && ascii_isdigit(p[11])) {
p += 11;
diff_foldcolumn_new = getdigits_int(&p, false, diff_foldcolumn_new);
} else if (STRNCMP(p, "hiddenoff", 9) == 0) {
p += 9;
diff_flags_new |= DIFF_HIDDEN_OFF;
} else if (STRNCMP(p, "closeoff", 8) == 0) {
p += 8;
diff_flags_new |= DIFF_CLOSE_OFF;
} else if (STRNCMP(p, "indent-heuristic", 16) == 0) {
p += 16;
diff_indent_heuristic = XDF_INDENT_HEURISTIC;
} else if (STRNCMP(p, "internal", 8) == 0) {
p += 8;
diff_flags_new |= DIFF_INTERNAL;
} else if (STRNCMP(p, "algorithm:", 10) == 0) {
p += 10;
if (STRNCMP(p, "myers", 5) == 0) {
p += 5;
diff_algorithm_new = 0;
} else if (STRNCMP(p, "minimal", 7) == 0) {
p += 7;
diff_algorithm_new = XDF_NEED_MINIMAL;
} else if (STRNCMP(p, "patience", 8) == 0) {
p += 8;
diff_algorithm_new = XDF_PATIENCE_DIFF;
} else if (STRNCMP(p, "histogram", 9) == 0) {
p += 9;
diff_algorithm_new = XDF_HISTOGRAM_DIFF;
} else {
return FAIL;
}
}

if ((*p != ',') && (*p != NUL)) {
return FAIL;
}

if (*p == ',') {
++p;
}
}

diff_algorithm_new |= diff_indent_heuristic;


if ((diff_flags_new & DIFF_HORIZONTAL) && (diff_flags_new & DIFF_VERTICAL)) {
return FAIL;
}



if (diff_flags != diff_flags_new || diff_algorithm != diff_algorithm_new) {
FOR_ALL_TABS(tp) {
tp->tp_diff_invalid = true;
}
}

diff_flags = diff_flags_new;
diff_context = diff_context_new == 0 ? 1 : diff_context_new;
diff_foldcolumn = diff_foldcolumn_new;
diff_algorithm = diff_algorithm_new;

diff_redraw(true);



check_scrollbind((linenr_T)0, 0L);
return OK;
}


bool diffopt_horizontal(void)
FUNC_ATTR_PURE FUNC_ATTR_WARN_UNUSED_RESULT
{
return (diff_flags & DIFF_HORIZONTAL) != 0;
}


bool diffopt_hiddenoff(void)
{
return (diff_flags & DIFF_HIDDEN_OFF) != 0;
}


bool diffopt_closeoff(void)
FUNC_ATTR_PURE FUNC_ATTR_WARN_UNUSED_RESULT
{
return (diff_flags & DIFF_CLOSE_OFF) != 0;
}









bool diff_find_change(win_T *wp, linenr_T lnum, int *startp, int *endp)
FUNC_ATTR_WARN_UNUSED_RESULT FUNC_ATTR_NONNULL_ALL
{
char_u *line_new;
int si_org;
int si_new;
int ei_org;
int ei_new;
bool added = true;
int l;


char_u *line_org = vim_strsave(ml_get_buf(wp->w_buffer, lnum, false));

int idx = diff_buf_idx(wp->w_buffer);
if (idx == DB_COUNT) {

xfree(line_org);
return false;
}


diff_T *dp;
for (dp = curtab->tp_first_diff; dp != NULL; dp = dp->df_next) {
if (lnum <= dp->df_lnum[idx] + dp->df_count[idx]) {
break;
}
}

if ((dp == NULL) || (diff_check_sanity(curtab, dp) == FAIL)) {
xfree(line_org);
return false;
}

int off = lnum - dp->df_lnum[idx];
int i;
for (i = 0; i < DB_COUNT; ++i) {
if ((curtab->tp_diffbuf[i] != NULL) && (i != idx)) {

if (off >= dp->df_count[i]) {
continue;
}
added = false;
line_new = ml_get_buf(curtab->tp_diffbuf[i],
dp->df_lnum[i] + off, false);


si_org = si_new = 0;

while (line_org[si_org] != NUL) {
if (((diff_flags & DIFF_IWHITE)
&& ascii_iswhite(line_org[si_org])
&& ascii_iswhite(line_new[si_new]))
|| ((diff_flags & DIFF_IWHITEALL)
&& (ascii_iswhite(line_org[si_org])
|| ascii_iswhite(line_new[si_new])))) {
si_org = (int)(skipwhite(line_org + si_org) - line_org);
si_new = (int)(skipwhite(line_new + si_new) - line_new);
} else {
if (!diff_equal_char(line_org + si_org, line_new + si_new, &l)) {
break;
}
si_org += l;
si_new += l;
}
}



si_org -= utf_head_off(line_org, line_org + si_org);
si_new -= utf_head_off(line_new, line_new + si_new);

if (*startp > si_org) {
*startp = si_org;
}


if ((line_org[si_org] != NUL) || (line_new[si_new] != NUL)) {
ei_org = (int)STRLEN(line_org);
ei_new = (int)STRLEN(line_new);

while (ei_org >= *startp
&& ei_new >= si_new
&& ei_org >= 0
&& ei_new >= 0) {
if (((diff_flags & DIFF_IWHITE)
&& ascii_iswhite(line_org[ei_org])
&& ascii_iswhite(line_new[ei_new]))
|| ((diff_flags & DIFF_IWHITEALL)
&& (ascii_iswhite(line_org[ei_org])
|| ascii_iswhite(line_new[ei_new])))) {
while (ei_org >= *startp && ascii_iswhite(line_org[ei_org])) {
ei_org--;
}

while (ei_new >= si_new && ascii_iswhite(line_new[ei_new])) {
ei_new--;
}
} else {
const char_u *p1 = line_org + ei_org;
const char_u *p2 = line_new + ei_new;

p1 -= utf_head_off(line_org, p1);
p2 -= utf_head_off(line_new, p2);

if (!diff_equal_char(p1, p2, &l)) {
break;
}
ei_org -= l;
ei_new -= l;
}
}

if (*endp < ei_org) {
*endp = ei_org;
}
}
}
}

xfree(line_org);
return added;
}








bool diff_infold(win_T *wp, linenr_T lnum)
FUNC_ATTR_PURE FUNC_ATTR_WARN_UNUSED_RESULT FUNC_ATTR_NONNULL_ARG(1)
{
bool other = false;
diff_T *dp;


if (!wp->w_p_diff) {
return false;
}

int idx = -1;
int i;
for (i = 0; i < DB_COUNT; ++i) {
if (curtab->tp_diffbuf[i] == wp->w_buffer) {
idx = i;
} else if (curtab->tp_diffbuf[i] != NULL) {
other = true;
}
}


if ((idx == -1) || !other) {
return false;
}

if (curtab->tp_diff_invalid) {

ex_diffupdate(NULL);
}


if (curtab->tp_first_diff == NULL) {
return true;
}

for (dp = curtab->tp_first_diff; dp != NULL; dp = dp->df_next) {

if (dp->df_lnum[idx] - diff_context > lnum) {
break;
}


if (dp->df_lnum[idx] + dp->df_count[idx] + diff_context > lnum) {
return false;
}
}
return true;
}


void nv_diffgetput(bool put, size_t count)
{
exarg_T ea;
char buf[30];

if (bt_prompt(curbuf)) {
vim_beep(BO_OPER);
return;
}
if (count == 0) {
ea.arg = (char_u *)"";
} else {
vim_snprintf(buf, 30, "%zu", count);
ea.arg = (char_u *)buf;
}

if (put) {
ea.cmdidx = CMD_diffput;
} else {
ea.cmdidx = CMD_diffget;
}

ea.addr_count = 0;
ea.line1 = curwin->w_cursor.lnum;
ea.line2 = curwin->w_cursor.lnum;
ex_diffgetput(&ea);
}




void ex_diffgetput(exarg_T *eap)
{
linenr_T lnum;
int count;
linenr_T off = 0;
diff_T *dp;
diff_T *dprev;
diff_T *dfree;
int i;
int added;
char_u *p;
aco_save_T aco;
buf_T *buf;
int start_skip, end_skip;
int new_count;
int buf_empty;
int found_not_ma = false;
int idx_other;
int idx_from;
int idx_to;


int idx_cur = diff_buf_idx(curbuf);
if (idx_cur == DB_COUNT) {
EMSG(_("E99: Current buffer is not in diff mode"));
return;
}

if (*eap->arg == NUL) {

for (idx_other = 0; idx_other < DB_COUNT; ++idx_other) {
if ((curtab->tp_diffbuf[idx_other] != curbuf)
&& (curtab->tp_diffbuf[idx_other] != NULL)) {
if ((eap->cmdidx != CMD_diffput)
|| MODIFIABLE(curtab->tp_diffbuf[idx_other])) {
break;
}
found_not_ma = true;
}
}

if (idx_other == DB_COUNT) {
if (found_not_ma) {
EMSG(_("E793: No other buffer in diff mode is modifiable"));
} else {
EMSG(_("E100: No other buffer in diff mode"));
}
return;
}


for (i = idx_other + 1; i < DB_COUNT; ++i) {
if ((curtab->tp_diffbuf[i] != curbuf)
&& (curtab->tp_diffbuf[i] != NULL)
&& ((eap->cmdidx != CMD_diffput)
|| MODIFIABLE(curtab->tp_diffbuf[i]))) {
EMSG(_("E101: More than two buffers in diff mode, don't know "
"which one to use"));
return;
}
}
} else {

p = eap->arg + STRLEN(eap->arg);
while (p > eap->arg && ascii_iswhite(p[-1])) {
p--;
}

for (i = 0; ascii_isdigit(eap->arg[i]) && eap->arg + i < p; ++i) {
}

if (eap->arg + i == p) {

i = atol((char *)eap->arg);
} else {
i = buflist_findpat(eap->arg, p, false, true, false);

if (i < 0) {

return;
}
}
buf = buflist_findnr(i);

if (buf == NULL) {
EMSG2(_("E102: Can't find buffer \"%s\""), eap->arg);
return;
}

if (buf == curbuf) {

return;
}
idx_other = diff_buf_idx(buf);

if (idx_other == DB_COUNT) {
EMSG2(_("E103: Buffer \"%s\" is not in diff mode"), eap->arg);
return;
}
}

diff_busy = true;


if (eap->addr_count == 0) {


if ((eap->cmdidx == CMD_diffget)
&& (eap->line1 == curbuf->b_ml.ml_line_count)
&& (diff_check(curwin, eap->line1) == 0)
&& ((eap->line1 == 1) || (diff_check(curwin, eap->line1 - 1) == 0))) {
++eap->line2;
} else if (eap->line1 > 0) {
--eap->line1;
}
}

if (eap->cmdidx == CMD_diffget) {
idx_from = idx_other;
idx_to = idx_cur;
} else {
idx_from = idx_cur;
idx_to = idx_other;





aucmd_prepbuf(&aco, curtab->tp_diffbuf[idx_other]);
}




if (!curbuf->b_changed) {
change_warning(0);
if (diff_buf_idx(curbuf) != idx_to) {
EMSG(_("E787: Buffer changed unexpectedly"));
goto theend;
}
}

dprev = NULL;

for (dp = curtab->tp_first_diff; dp != NULL;) {
if (dp->df_lnum[idx_cur] > eap->line2 + off) {

break;
}
dfree = NULL;
lnum = dp->df_lnum[idx_to];
count = dp->df_count[idx_to];

if ((dp->df_lnum[idx_cur] + dp->df_count[idx_cur] > eap->line1 + off)
&& (u_save(lnum - 1, lnum + count) != FAIL)) {

start_skip = 0;
end_skip = 0;

if (eap->addr_count > 0) {

start_skip = eap->line1 + off - dp->df_lnum[idx_cur];
if (start_skip > 0) {

if (start_skip > count) {
lnum += count;
count = 0;
} else {
count -= start_skip;
lnum += start_skip;
}
} else {
start_skip = 0;
}

end_skip = dp->df_lnum[idx_cur] + dp->df_count[idx_cur] - 1
- (eap->line2 + off);

if (end_skip > 0) {

if (idx_cur == idx_from) {

i = dp->df_count[idx_cur] - start_skip - end_skip;

if (count > i) {
count = i;
}
} else {

count -= end_skip;
end_skip = dp->df_count[idx_from] - start_skip - count;

if (end_skip < 0) {
end_skip = 0;
}
}
} else {
end_skip = 0;
}
}

buf_empty = BUFEMPTY();
added = 0;

for (i = 0; i < count; ++i) {

buf_empty = curbuf->b_ml.ml_line_count == 1;
ml_delete(lnum, false);
added--;
}

for (i = 0; i < dp->df_count[idx_from] - start_skip - end_skip; ++i) {
linenr_T nr = dp->df_lnum[idx_from] + start_skip + i;
if (nr > curtab->tp_diffbuf[idx_from]->b_ml.ml_line_count) {
break;
}
p = vim_strsave(ml_get_buf(curtab->tp_diffbuf[idx_from], nr, false));
ml_append(lnum + i - 1, p, 0, false);
xfree(p);
added++;
if (buf_empty && (curbuf->b_ml.ml_line_count == 2)) {


buf_empty = false;
ml_delete((linenr_T)2, false);
}
}
new_count = dp->df_count[idx_to] + added;
dp->df_count[idx_to] = new_count;

if ((start_skip == 0) && (end_skip == 0)) {


for (i = 0; i < DB_COUNT; ++i) {
if ((curtab->tp_diffbuf[i] != NULL)
&& (i != idx_from)
&& (i != idx_to)
&& !diff_equal_entry(dp, idx_from, i)) {
break;
}
}

if (i == DB_COUNT) {

dfree = dp;
dp = dp->df_next;

if (dprev == NULL) {
curtab->tp_first_diff = dp;
} else {
dprev->df_next = dp;
}
}
}


if (added != 0) {
mark_adjust(lnum, lnum + count - 1, (long)MAXLNUM, (long)added,
kExtmarkUndo);
if (curwin->w_cursor.lnum >= lnum) {


if (curwin->w_cursor.lnum >= lnum + count) {
curwin->w_cursor.lnum += added;
} else if (added < 0) {
curwin->w_cursor.lnum = lnum;
}
}
}
changed_lines(lnum, 0, lnum + count, (long)added, true);

if (dfree != NULL) {

diff_fold_update(dfree, idx_to);
xfree(dfree);
} else {

dp->df_count[idx_to] = new_count;
}


if (idx_cur == idx_to) {
off += added;
}
}


if (dfree == NULL) {
dprev = dp;
dp = dp->df_next;
}
}


if (eap->cmdidx != CMD_diffget) {



if (KeyTyped) {
u_sync(false);
}
aucmd_restbuf(&aco);
}

theend:
diff_busy = false;
if (diff_need_update) {
ex_diffupdate(NULL);
}




check_cursor();
changed_line_abv_curs();

if (diff_need_update) {

diff_need_update = false;
} else {

diff_redraw(false);
apply_autocmds(EVENT_DIFFUPDATED, NULL, NULL, false, curbuf);
}
}








static void diff_fold_update(diff_T *dp, int skip_idx)
{
FOR_ALL_WINDOWS_IN_TAB(wp, curtab) {
for (int i = 0; i < DB_COUNT; ++i) {
if ((curtab->tp_diffbuf[i] == wp->w_buffer) && (i != skip_idx)) {
foldUpdate(wp, dp->df_lnum[i], dp->df_lnum[i] + dp->df_count[i]);
}
}
}
}




bool diff_mode_buf(buf_T *buf)
FUNC_ATTR_PURE FUNC_ATTR_WARN_UNUSED_RESULT FUNC_ATTR_NONNULL_ARG(1)
{
FOR_ALL_TABS(tp) {
if (diff_buf_idx_tp(buf, tp) != DB_COUNT) {
return true;
}
}
return false;
}







int diff_move_to(int dir, long count)
{
linenr_T lnum = curwin->w_cursor.lnum;
int idx = diff_buf_idx(curbuf);
if ((idx == DB_COUNT) || (curtab->tp_first_diff == NULL)) {
return FAIL;
}

if (curtab->tp_diff_invalid) {

ex_diffupdate(NULL);
}

if (curtab->tp_first_diff == NULL) {

return FAIL;
}

while (--count >= 0) {

if ((dir == BACKWARD) && (lnum <= curtab->tp_first_diff->df_lnum[idx])) {
break;
}

diff_T *dp;
for (dp = curtab->tp_first_diff;; dp = dp->df_next) {
if (dp == NULL) {
break;
}

if (((dir == FORWARD) && (lnum < dp->df_lnum[idx]))
|| ((dir == BACKWARD)
&& ((dp->df_next == NULL)
|| (lnum <= dp->df_next->df_lnum[idx])))) {
lnum = dp->df_lnum[idx];
break;
}
}
}


if (lnum > curbuf->b_ml.ml_line_count) {
lnum = curbuf->b_ml.ml_line_count;
}


if (lnum == curwin->w_cursor.lnum) {
return FAIL;
}

setpcmark();
curwin->w_cursor.lnum = lnum;
curwin->w_cursor.col = 0;

return OK;
}



static linenr_T diff_get_corresponding_line_int(buf_T *buf1, linenr_T lnum1)
{
int idx1;
int idx2;
diff_T *dp;
int baseline = 0;

idx1 = diff_buf_idx(buf1);
idx2 = diff_buf_idx(curbuf);

if ((idx1 == DB_COUNT)
|| (idx2 == DB_COUNT)
|| (curtab->tp_first_diff == NULL)) {
return lnum1;
}

if (curtab->tp_diff_invalid) {

ex_diffupdate(NULL);
}

if (curtab->tp_first_diff == NULL) {

return lnum1;
}

for (dp = curtab->tp_first_diff; dp != NULL; dp = dp->df_next) {
if (dp->df_lnum[idx1] > lnum1) {
return lnum1 - baseline;
}
if ((dp->df_lnum[idx1] + dp->df_count[idx1]) > lnum1) {

baseline = lnum1 - dp->df_lnum[idx1];

if (baseline > dp->df_count[idx2]) {
baseline = dp->df_count[idx2];
}

return dp->df_lnum[idx2] + baseline;
}
if ((dp->df_lnum[idx1] == lnum1)
&& (dp->df_count[idx1] == 0)
&& (dp->df_lnum[idx2] <= curwin->w_cursor.lnum)
&& ((dp->df_lnum[idx2] + dp->df_count[idx2])
> curwin->w_cursor.lnum)) {





return curwin->w_cursor.lnum;
}
baseline = (dp->df_lnum[idx1] + dp->df_count[idx1])
- (dp->df_lnum[idx2] + dp->df_count[idx2]);
}


return lnum1 - baseline;
}







linenr_T diff_get_corresponding_line(buf_T *buf1, linenr_T lnum1)
{
linenr_T lnum = diff_get_corresponding_line_int(buf1, lnum1);


if (lnum > curbuf->b_ml.ml_line_count) {
return curbuf->b_ml.ml_line_count;
}
return lnum;
}



linenr_T diff_lnum_win(linenr_T lnum, win_T *wp)
{
diff_T *dp;
int idx;
int i;
linenr_T n;

idx = diff_buf_idx(curbuf);

if (idx == DB_COUNT) {

return (linenr_T)0;
}

if (curtab->tp_diff_invalid) {

ex_diffupdate(NULL);
}


for (dp = curtab->tp_first_diff; dp != NULL; dp = dp->df_next) {
if (lnum <= dp->df_lnum[idx] + dp->df_count[idx]) {
break;
}
}


if (dp == NULL) {
return wp->w_buffer->b_ml.ml_line_count
- (curbuf->b_ml.ml_line_count - lnum);
}


i = diff_buf_idx(wp->w_buffer);

if (i == DB_COUNT) {

return (linenr_T)0;
}

n = lnum + (dp->df_lnum[i] - dp->df_lnum[idx]);
if (n > dp->df_lnum[i] + dp->df_count[i]) {
n = dp->df_lnum[i] + dp->df_count[i];
}
return n;
}





static int parse_diff_ed(char_u *line,
linenr_T *lnum_orig,
long *count_orig,
linenr_T *lnum_new,
long *count_new)
{
char_u *p;
long f1, l1, f2, l2;
int difftype;





p = line;
f1 = getdigits(&p, true, 0);
if (*p == ',') {
p++;
l1 = getdigits(&p, true, 0);
} else {
l1 = f1;
}
if (*p != 'a' && *p != 'c' && *p != 'd') {
return FAIL; 
}
difftype = *p++;
f2 = getdigits(&p, true, 0);
if (*p == ',') {
p++;
l2 = getdigits(&p, true, 0);
} else {
l2 = f2;
}
if (l1 < f1 || l2 < f2) {
return FAIL;
}

if (difftype == 'a') {
*lnum_orig = f1 + 1;
*count_orig = 0;
} else {
*lnum_orig = f1;
*count_orig = l1 - f1 + 1;
}
if (difftype == 'd') {
*lnum_new = f2 + 1;
*count_new = 0;
} else {
*lnum_new = f2;
*count_new = l2 - f2 + 1;
}
return OK;
}





static int parse_diff_unified(char_u *line,
linenr_T *lnum_orig,
long *count_orig,
linenr_T *lnum_new,
long *count_new)
{
char_u *p;
long oldline, oldcount, newline, newcount;



p = line;
if (*p++ == '@' && *p++ == '@' && *p++ == ' ' && *p++ == '-') {
oldline = getdigits(&p, true, 0);
if (*p == ',') {
p++;
oldcount = getdigits(&p, true, 0);
} else {
oldcount = 1;
}
if (*p++ == ' ' && *p++ == '+') {
newline = getdigits(&p, true, 0);
if (*p == ',') {
p++;
newcount = getdigits(&p, true, 0);
} else {
newcount = 1;
}
} else {
return FAIL; 
}

if (oldcount == 0) {
oldline += 1;
}
if (newcount == 0) {
newline += 1;
}
if (newline == 0) {
newline = 1;
}

*lnum_orig = oldline;
*count_orig = oldcount;
*lnum_new = newline;
*count_new = newcount;

return OK;
}

return FAIL;
}





static int xdiff_out(void *priv, mmbuffer_t *mb, int nbuf)
{
diffout_T *dout = (diffout_T *)priv;
char_u *p;



if (nbuf > 1) {
return 0;
}


if (STRNCMP(mb[0].ptr, "@@ ", 3) != 0) {
return 0;
}

ga_grow(&dout->dout_ga, 1);

p = vim_strnsave((char_u *)mb[0].ptr, mb[0].size);
((char_u **)dout->dout_ga.ga_data)[dout->dout_ga.ga_len++] = p;
return 0;
}
