#include <stdbool.h>
#include <string.h>
#include <inttypes.h>
#include <assert.h>
#include "nvim/api/private/handle.h"
#include "nvim/api/private/helpers.h"
#include "nvim/api/vim.h"
#include "nvim/ascii.h"
#include "nvim/assert.h"
#include "nvim/channel.h"
#include "nvim/vim.h"
#include "nvim/buffer.h"
#include "nvim/change.h"
#include "nvim/charset.h"
#include "nvim/cursor.h"
#include "nvim/diff.h"
#include "nvim/digraph.h"
#include "nvim/eval.h"
#include "nvim/ex_cmds2.h"
#include "nvim/ex_cmds.h"
#include "nvim/ex_docmd.h"
#include "nvim/ex_eval.h"
#include "nvim/ex_getln.h"
#include "nvim/fileio.h"
#include "nvim/file_search.h"
#include "nvim/fold.h"
#include "nvim/getchar.h"
#include "nvim/hashtab.h"
#include "nvim/highlight.h"
#include "nvim/indent.h"
#include "nvim/indent_c.h"
#include "nvim/main.h"
#include "nvim/mark.h"
#include "nvim/extmark.h"
#include "nvim/mbyte.h"
#include "nvim/memline.h"
#include "nvim/memory.h"
#include "nvim/message.h"
#include "nvim/misc1.h"
#include "nvim/garray.h"
#include "nvim/move.h"
#include "nvim/option.h"
#include "nvim/os_unix.h"
#include "nvim/path.h"
#include "nvim/quickfix.h"
#include "nvim/regexp.h"
#include "nvim/screen.h"
#include "nvim/sign.h"
#include "nvim/spell.h"
#include "nvim/strings.h"
#include "nvim/syntax.h"
#include "nvim/ui.h"
#include "nvim/undo.h"
#include "nvim/version.h"
#include "nvim/window.h"
#include "nvim/shada.h"
#include "nvim/os/os.h"
#include "nvim/os/time.h"
#include "nvim/os/input.h"
#include "nvim/buffer_updates.h"
#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "buffer.c.generated.h"
#endif
static char *msg_loclist = N_("[Location List]");
static char *msg_qflist = N_("[Quickfix List]");
static char *e_auabort = N_("E855: Autocommands caused command to abort");
static int buf_free_count = 0;
typedef enum {
kBffClearWinInfo = 1,
kBffInitChangedtick = 2,
} BufFreeFlags;
static int
read_buffer(
int read_stdin, 
exarg_T *eap, 
int flags) 
{
int retval = OK;
linenr_T line_count;
line_count = curbuf->b_ml.ml_line_count;
retval = readfile(
read_stdin ? NULL : curbuf->b_ffname,
read_stdin ? NULL : curbuf->b_fname,
(linenr_T)line_count, (linenr_T)0, (linenr_T)MAXLNUM, eap,
flags | READ_BUFFER);
if (retval == OK) {
while (--line_count >= 0) {
ml_delete((linenr_T)1, false);
}
} else {
while (curbuf->b_ml.ml_line_count > line_count) {
ml_delete(line_count, false);
}
}
curwin->w_cursor.lnum = 1;
curwin->w_cursor.col = 0;
if (read_stdin) {
if (!readonlymode && !BUFEMPTY()) {
changed();
} else if (retval != FAIL) {
unchanged(curbuf, false, true);
}
apply_autocmds_retval(EVENT_STDINREADPOST, NULL, NULL, false,
curbuf, &retval);
}
return retval;
}
int open_buffer(
int read_stdin, 
exarg_T *eap, 
int flags 
)
{
int retval = OK;
bufref_T old_curbuf;
long old_tw = curbuf->b_p_tw;
int read_fifo = false;
if (readonlymode && curbuf->b_ffname != NULL
&& (curbuf->b_flags & BF_NEVERLOADED))
curbuf->b_p_ro = true;
if (ml_open(curbuf) == FAIL) {
close_buffer(NULL, curbuf, 0, false);
curbuf = NULL;
FOR_ALL_BUFFERS(buf) {
if (buf->b_ml.ml_mfp != NULL) {
curbuf = buf;
break;
}
}
if (curbuf == NULL) {
EMSG(_("E82: Cannot allocate any buffer, exiting..."));
getout(2);
}
EMSG(_("E83: Cannot allocate buffer, using other one..."));
enter_buffer(curbuf);
if (old_tw != curbuf->b_p_tw) {
check_colorcolumn(curwin);
}
return FAIL;
}
set_bufref(&old_curbuf, curbuf);
modified_was_set = false;
curwin->w_valid = 0;
if (curbuf->b_ffname != NULL) {
int old_msg_silent = msg_silent;
#if defined(UNIX)
int save_bin = curbuf->b_p_bin;
int perm;
perm = os_getperm((const char *)curbuf->b_ffname);
if (perm >= 0 && (0
#if defined(S_ISFIFO)
|| S_ISFIFO(perm)
#endif
#if defined(S_ISSOCK)
|| S_ISSOCK(perm)
#endif
#if defined(OPEN_CHR_FILES)
|| (S_ISCHR(perm)
&& is_dev_fd_file(curbuf->b_ffname))
#endif
)
) {
read_fifo = true;
}
if (read_fifo) {
curbuf->b_p_bin = true;
}
#endif
if (shortmess(SHM_FILEINFO)) {
msg_silent = 1;
}
retval = readfile(curbuf->b_ffname, curbuf->b_fname,
(linenr_T)0, (linenr_T)0, (linenr_T)MAXLNUM, eap,
flags | READ_NEW | (read_fifo ? READ_FIFO : 0));
#if defined(UNIX)
if (read_fifo) {
curbuf->b_p_bin = save_bin;
if (retval == OK) {
retval = read_buffer(false, eap, flags);
}
}
#endif
msg_silent = old_msg_silent;
if (bt_help(curbuf)) {
fix_help_buffer();
}
} else if (read_stdin) {
int save_bin = curbuf->b_p_bin;
curbuf->b_p_bin = true;
retval = readfile(NULL, NULL, (linenr_T)0,
(linenr_T)0, (linenr_T)MAXLNUM, NULL,
flags | (READ_NEW + READ_STDIN));
curbuf->b_p_bin = save_bin;
if (retval == OK) {
retval = read_buffer(true, eap, flags);
}
}
if (curbuf->b_flags & BF_NEVERLOADED) {
(void)buf_init_chartab(curbuf, false);
parse_cino(curbuf);
}
if ((got_int && vim_strchr(p_cpo, CPO_INTMOD) != NULL)
|| modified_was_set 
|| (aborting() && vim_strchr(p_cpo, CPO_INTMOD) != NULL)) {
changed();
} else if (retval != FAIL && !read_stdin && !read_fifo) {
unchanged(curbuf, false, true);
}
save_file_ff(curbuf); 
curbuf->b_last_changedtick = buf_get_changedtick(curbuf);
curbuf->b_last_changedtick_pum = buf_get_changedtick(curbuf);
if (aborting()) {
curbuf->b_flags |= BF_READERR;
}
foldUpdateAll(curwin);
if (!(curwin->w_valid & VALID_TOPLINE)) {
curwin->w_topline = 1;
curwin->w_topfill = 0;
}
apply_autocmds_retval(EVENT_BUFENTER, NULL, NULL, false, curbuf, &retval);
if (retval == FAIL) {
return FAIL;
}
if (bufref_valid(&old_curbuf) && old_curbuf.br_buf->b_ml.ml_mfp != NULL) {
aco_save_T aco;
aucmd_prepbuf(&aco, old_curbuf.br_buf);
do_modelines(0);
curbuf->b_flags &= ~(BF_CHECK_RO | BF_NEVERLOADED);
apply_autocmds_retval(EVENT_BUFWINENTER, NULL, NULL, false, curbuf,
&retval);
aucmd_restbuf(&aco);
}
return retval;
}
void set_bufref(bufref_T *bufref, buf_T *buf)
{
bufref->br_buf = buf;
bufref->br_fnum = buf == NULL ? 0 : buf->b_fnum;
bufref->br_buf_free_count = buf_free_count;
}
bool bufref_valid(bufref_T *bufref)
{
return bufref->br_buf_free_count == buf_free_count
? true
: buf_valid(bufref->br_buf) && bufref->br_fnum == bufref->br_buf->b_fnum;
}
bool buf_valid(buf_T *buf)
FUNC_ATTR_PURE FUNC_ATTR_WARN_UNUSED_RESULT
{
if (buf == NULL) {
return false;
}
for (buf_T *bp = lastbuf; bp != NULL; bp = bp->b_prev) {
if (bp == buf) {
return true;
}
}
return false;
}
void close_buffer(win_T *win, buf_T *buf, int action, bool abort_if_last)
{
bool unload_buf = (action != 0);
bool del_buf = (action == DOBUF_DEL || action == DOBUF_WIPE);
bool wipe_buf = (action == DOBUF_WIPE);
bool is_curwin = (curwin != NULL && curwin->w_buffer == buf);
win_T *the_curwin = curwin;
tabpage_T *the_curtab = curtab;
if (!buf->terminal) {
if (buf->b_p_bh[0] == 'd') { 
del_buf = true;
unload_buf = true;
} else if (buf->b_p_bh[0] == 'w') { 
del_buf = true;
unload_buf = true;
wipe_buf = true;
} else if (buf->b_p_bh[0] == 'u') 
unload_buf = true;
}
if (buf->terminal && (unload_buf || del_buf || wipe_buf)) {
unload_buf = true;
del_buf = true;
wipe_buf = true;
}
if (buf->b_locked > 0 && (del_buf || wipe_buf)) {
EMSG(_("E937: Attempt to delete a buffer that is in use"));
return;
}
if (win != NULL 
&& win_valid_any_tab(win)) {
if (buf->b_nwindows == 1) {
set_last_cursor(win);
}
buflist_setfpos(buf, win,
win->w_cursor.lnum == 1 ? 0 : win->w_cursor.lnum,
win->w_cursor.col, true);
}
bufref_T bufref;
set_bufref(&bufref, buf);
if (buf->b_nwindows == 1) {
buf->b_locked++;
if (apply_autocmds(EVENT_BUFWINLEAVE, buf->b_fname, buf->b_fname, false,
buf) && !bufref_valid(&bufref)) {
EMSG(_(e_auabort));
return;
}
buf->b_locked--;
if (abort_if_last && last_nonfloat(win)) {
EMSG(_(e_auabort));
return;
}
if (!unload_buf) {
buf->b_locked++;
if (apply_autocmds(EVENT_BUFHIDDEN, buf->b_fname, buf->b_fname, false,
buf) && !bufref_valid(&bufref)) {
EMSG(_(e_auabort));
return;
}
buf->b_locked--;
if (abort_if_last && last_nonfloat(win)) {
EMSG(_(e_auabort));
return;
}
}
if (aborting()) { 
return;
}
}
if (is_curwin && curwin != the_curwin && win_valid_any_tab(the_curwin)) {
block_autocmds();
goto_tabpage_win(the_curtab, the_curwin);
unblock_autocmds();
}
int nwindows = buf->b_nwindows;
if (buf->b_nwindows > 0) {
buf->b_nwindows--;
}
if (diffopt_hiddenoff() && !unload_buf && buf->b_nwindows == 0) {
diff_buf_delete(buf); 
}
if (buf->b_nwindows > 0 || !unload_buf) {
return;
}
if (buf->terminal) {
terminal_close(buf->terminal, NULL);
}
if (buf->b_ffname == NULL) {
del_buf = true;
}
bool is_curbuf = (buf == curbuf);
if (is_curbuf && VIsual_active
#if defined(EXITFREE)
&& !entered_free_all_mem
#endif
) {
end_visual_mode();
}
buf->b_nwindows = nwindows;
buf_freeall(buf, (del_buf ? BFA_DEL : 0) + (wipe_buf ? BFA_WIPE : 0));
if (!bufref_valid(&bufref)) {
return;
}
if (aborting()) {
return;
}
if (buf == curbuf && !is_curbuf) {
return;
}
if (win != NULL 
&& win_valid_any_tab(win)
&& win->w_buffer == buf) {
win->w_buffer = NULL; 
}
if (buf->b_nwindows > 0) {
buf->b_nwindows--;
}
do_autochdir();
buf_updates_unregister_all(buf);
if (wipe_buf) {
xfree(buf->b_ffname);
xfree(buf->b_sfname);
if (buf->b_prev == NULL) {
firstbuf = buf->b_next;
} else {
buf->b_prev->b_next = buf->b_next;
}
if (buf->b_next == NULL) {
lastbuf = buf->b_prev;
} else {
buf->b_next->b_prev = buf->b_prev;
}
free_buffer(buf);
} else {
if (del_buf) {
free_buffer_stuff(buf, kBffClearWinInfo | kBffInitChangedtick);
buf->b_flags = BF_CHECK_RO | BF_NEVERLOADED;
buf->b_p_initialized = false;
}
buf_clear_file(buf);
if (del_buf) {
buf->b_p_bl = false;
}
}
}
void buf_clear_file(buf_T *buf)
{
buf->b_ml.ml_line_count = 1;
unchanged(buf, true, true);
buf->b_p_eol = true;
buf->b_start_eol = true;
buf->b_p_bomb = false;
buf->b_start_bomb = false;
buf->b_ml.ml_mfp = NULL;
buf->b_ml.ml_flags = ML_EMPTY; 
}
void buf_clear(void)
{
linenr_T line_count = curbuf->b_ml.ml_line_count;
while (!(curbuf->b_ml.ml_flags & ML_EMPTY)) {
ml_delete((linenr_T)1, false);
}
deleted_lines_mark(1, line_count); 
ml_close(curbuf, true); 
buf_clear_file(curbuf);
}
void buf_freeall(buf_T *buf, int flags)
{
bool is_curbuf = (buf == curbuf);
int is_curwin = (curwin != NULL && curwin->w_buffer == buf);
win_T *the_curwin = curwin;
tabpage_T *the_curtab = curtab;
buf->b_locked++;
bufref_T bufref;
set_bufref(&bufref, buf);
if ((buf->b_ml.ml_mfp != NULL)
&& apply_autocmds(EVENT_BUFUNLOAD, buf->b_fname, buf->b_fname, false, buf)
&& !bufref_valid(&bufref)) {
return;
}
if ((flags & BFA_DEL)
&& buf->b_p_bl
&& apply_autocmds(EVENT_BUFDELETE, buf->b_fname, buf->b_fname, false, buf)
&& !bufref_valid(&bufref)) {
return;
}
if ((flags & BFA_WIPE)
&& apply_autocmds(EVENT_BUFWIPEOUT, buf->b_fname, buf->b_fname, false,
buf)
&& !bufref_valid(&bufref)) {
return;
}
buf->b_locked--;
if (is_curwin && curwin != the_curwin && win_valid_any_tab(the_curwin)) {
block_autocmds();
goto_tabpage_win(the_curtab, the_curwin);
unblock_autocmds();
}
if (aborting()) { 
return;
}
if (buf == curbuf && !is_curbuf) {
return;
}
diff_buf_delete(buf); 
if (curwin != NULL && curwin->w_buffer == buf) {
reset_synblock(curwin);
}
FOR_ALL_TAB_WINDOWS(tp, win) {
if (win->w_buffer == buf) {
clearFolding(win);
}
}
ml_close(buf, true); 
buf->b_ml.ml_line_count = 0; 
if ((flags & BFA_KEEP_UNDO) == 0) {
u_blockfree(buf); 
u_clearall(buf); 
}
syntax_clear(&buf->b_s); 
buf->b_flags &= ~BF_READERR; 
}
static void free_buffer(buf_T *buf)
{
handle_unregister_buffer(buf);
buf_free_count++;
free_buffer_stuff(buf, kBffClearWinInfo);
if (buf->b_vars->dv_refcount > DO_NOT_FREE_CNT) {
tv_dict_add(buf->b_vars,
tv_dict_item_copy((dictitem_T *)(&buf->changedtick_di)));
}
unref_var_dict(buf->b_vars);
aubuflocal_remove(buf);
tv_dict_unref(buf->additional_data);
xfree(buf->b_prompt_text);
callback_free(&buf->b_prompt_callback);
callback_free(&buf->b_prompt_interrupt);
clear_fmark(&buf->b_last_cursor);
clear_fmark(&buf->b_last_insert);
clear_fmark(&buf->b_last_change);
for (size_t i = 0; i < NMARKS; i++) {
free_fmark(buf->b_namedm[i]);
}
for (int i = 0; i < buf->b_changelistlen; i++) {
free_fmark(buf->b_changelist[i]);
}
if (autocmd_busy) {
memset(&buf->b_namedm[0], 0, sizeof(buf->b_namedm));
memset(&buf->b_changelist[0], 0, sizeof(buf->b_changelist));
buf->b_next = au_pending_free_buf;
au_pending_free_buf = buf;
} else {
xfree(buf);
}
}
static void free_buffer_stuff(buf_T *buf, int free_flags)
{
if (free_flags & kBffClearWinInfo) {
clear_wininfo(buf); 
free_buf_options(buf, true);
ga_clear(&buf->b_s.b_langp);
}
{
hashitem_T *const changedtick_hi = hash_find(
&buf->b_vars->dv_hashtab, (const char_u *)"changedtick");
assert(changedtick_hi != NULL);
hash_remove(&buf->b_vars->dv_hashtab, changedtick_hi);
}
vars_clear(&buf->b_vars->dv_hashtab); 
hash_init(&buf->b_vars->dv_hashtab);
if (free_flags & kBffInitChangedtick) {
buf_init_changedtick(buf);
}
uc_clear(&buf->b_ucmds); 
buf_delete_signs(buf, (char_u *)"*"); 
extmark_free_all(buf); 
map_clear_int(buf, MAP_ALL_MODES, true, false); 
map_clear_int(buf, MAP_ALL_MODES, true, true); 
XFREE_CLEAR(buf->b_start_fenc);
buf_updates_unregister_all(buf);
}
static void clear_wininfo(buf_T *buf)
{
wininfo_T *wip;
while (buf->b_wininfo != NULL) {
wip = buf->b_wininfo;
buf->b_wininfo = wip->wi_next;
if (wip->wi_optset) {
clear_winopt(&wip->wi_opt);
deleteFoldRecurse(&wip->wi_folds);
}
xfree(wip);
}
}
void goto_buffer(exarg_T *eap, int start, int dir, int count)
{
bufref_T old_curbuf;
set_bufref(&old_curbuf, curbuf);
swap_exists_action = SEA_DIALOG;
(void)do_buffer(*eap->cmd == 's' ? DOBUF_SPLIT : DOBUF_GOTO,
start, dir, count, eap->forceit);
if (swap_exists_action == SEA_QUIT && *eap->cmd == 's') {
cleanup_T cs;
enter_cleanup(&cs);
win_close(curwin, true);
swap_exists_action = SEA_NONE;
swap_exists_did_quit = true;
leave_cleanup(&cs);
} else {
handle_swap_exists(&old_curbuf);
}
}
void handle_swap_exists(bufref_T *old_curbuf)
{
cleanup_T cs;
long old_tw = curbuf->b_p_tw;
buf_T *buf;
if (swap_exists_action == SEA_QUIT) {
enter_cleanup(&cs);
swap_exists_action = SEA_NONE; 
swap_exists_did_quit = true;
close_buffer(curwin, curbuf, DOBUF_UNLOAD, false);
if (old_curbuf == NULL
|| !bufref_valid(old_curbuf)
|| old_curbuf->br_buf == curbuf) {
buf = buflist_new(NULL, NULL, 1L, BLN_CURBUF | BLN_LISTED);
} else {
buf = old_curbuf->br_buf;
}
if (buf != NULL) {
int old_msg_silent = msg_silent;
if (shortmess(SHM_FILEINFO)) {
msg_silent = 1; 
}
enter_buffer(buf);
msg_silent = old_msg_silent;
if (old_tw != curbuf->b_p_tw) {
check_colorcolumn(curwin);
}
}
leave_cleanup(&cs);
} else if (swap_exists_action == SEA_RECOVER) {
enter_cleanup(&cs);
msg_scroll = true;
ml_recover(false);
MSG_PUTS("\n"); 
cmdline_row = msg_row;
do_modelines(0);
leave_cleanup(&cs);
}
swap_exists_action = SEA_NONE; 
}
char_u *
do_bufdel(
int command,
char_u *arg, 
int addr_count,
int start_bnr, 
int end_bnr, 
int forceit
)
{
int do_current = 0; 
int deleted = 0; 
char_u *errormsg = NULL; 
int bnr; 
char_u *p;
if (addr_count == 0) {
(void)do_buffer(command, DOBUF_CURRENT, FORWARD, 0, forceit);
} else {
if (addr_count == 2) {
if (*arg) { 
return (char_u *)_(e_trailing);
}
bnr = start_bnr;
} else { 
bnr = end_bnr;
}
for (; !got_int; os_breakcheck()) {
if (bnr == curbuf->b_fnum) {
do_current = bnr;
} else if (do_buffer(command, DOBUF_FIRST, FORWARD, bnr,
forceit) == OK) {
deleted++;
}
if (addr_count == 2) {
if (++bnr > end_bnr) {
break;
}
} else { 
arg = skipwhite(arg);
if (*arg == NUL) {
break;
}
if (!ascii_isdigit(*arg)) {
p = skiptowhite_esc(arg);
bnr = buflist_findpat(arg, p, command == DOBUF_WIPE,
false, false);
if (bnr < 0) { 
break;
}
arg = p;
} else {
bnr = getdigits_int(&arg, false, 0);
}
}
}
if (!got_int && do_current
&& do_buffer(command, DOBUF_FIRST,
FORWARD, do_current, forceit) == OK) {
deleted++;
}
if (deleted == 0) {
if (command == DOBUF_UNLOAD) {
STRCPY(IObuff, _("E515: No buffers were unloaded"));
} else if (command == DOBUF_DEL) {
STRCPY(IObuff, _("E516: No buffers were deleted"));
} else {
STRCPY(IObuff, _("E517: No buffers were wiped out"));
}
errormsg = IObuff;
} else if (deleted >= p_report) {
if (command == DOBUF_UNLOAD) {
if (deleted == 1) {
MSG(_("1 buffer unloaded"));
} else {
smsg(_("%d buffers unloaded"), deleted);
}
} else if (command == DOBUF_DEL) {
if (deleted == 1) {
MSG(_("1 buffer deleted"));
} else {
smsg(_("%d buffers deleted"), deleted);
}
} else {
if (deleted == 1) {
MSG(_("1 buffer wiped out"));
} else {
smsg(_("%d buffers wiped out"), deleted);
}
}
}
}
return errormsg;
}
static int empty_curbuf(int close_others, int forceit, int action)
{
int retval;
buf_T *buf = curbuf;
if (action == DOBUF_UNLOAD) {
EMSG(_("E90: Cannot unload last buffer"));
return FAIL;
}
bufref_T bufref;
set_bufref(&bufref, buf);
if (close_others) {
close_windows(buf, true);
}
setpcmark();
retval = do_ecmd(0, NULL, NULL, NULL, ECMD_ONE,
forceit ? ECMD_FORCEIT : 0, curwin);
if (buf != curbuf && bufref_valid(&bufref) && buf->b_nwindows == 0) {
close_buffer(NULL, buf, action, false);
}
if (!close_others) {
need_fileinfo = false;
}
return retval;
}
int
do_buffer(
int action,
int start,
int dir, 
int count, 
int forceit 
)
{
buf_T *buf;
buf_T *bp;
int unload = (action == DOBUF_UNLOAD || action == DOBUF_DEL
|| action == DOBUF_WIPE);
switch (start) {
case DOBUF_FIRST: buf = firstbuf; break;
case DOBUF_LAST: buf = lastbuf; break;
default: buf = curbuf; break;
}
if (start == DOBUF_MOD) { 
while (count-- > 0) {
do {
buf = buf->b_next;
if (buf == NULL) {
buf = firstbuf;
}
} while (buf != curbuf && !bufIsChanged(buf));
}
if (!bufIsChanged(buf)) {
EMSG(_("E84: No modified buffer found"));
return FAIL;
}
} else if (start == DOBUF_FIRST && count) { 
while (buf != NULL && buf->b_fnum != count) {
buf = buf->b_next;
}
} else {
bp = NULL;
while (count > 0 || (!unload && !buf->b_p_bl && bp != buf)) {
if (bp == NULL) {
bp = buf;
}
if (dir == FORWARD) {
buf = buf->b_next;
if (buf == NULL) {
buf = firstbuf;
}
} else {
buf = buf->b_prev;
if (buf == NULL) {
buf = lastbuf;
}
}
if (unload || buf->b_p_bl) {
count--;
bp = NULL; 
}
if (bp == buf) {
EMSG(_("E85: There is no listed buffer"));
return FAIL;
}
}
}
if (buf == NULL) { 
if (start == DOBUF_FIRST) {
if (!unload) {
EMSGN(_(e_nobufnr), count);
}
} else if (dir == FORWARD) {
EMSG(_("E87: Cannot go beyond last buffer"));
} else {
EMSG(_("E88: Cannot go before first buffer"));
}
return FAIL;
}
if (unload) {
int forward;
bufref_T bufref;
set_bufref(&bufref, buf);
if (action != DOBUF_WIPE && buf->b_ml.ml_mfp == NULL && !buf->b_p_bl) {
return FAIL;
}
if (!forceit && (buf->terminal || bufIsChanged(buf))) {
if ((p_confirm || cmdmod.confirm) && p_write && !buf->terminal) {
dialog_changed(buf, false);
if (!bufref_valid(&bufref)) {
return FAIL;
}
if (bufIsChanged(buf)) {
return FAIL;
}
} else {
if (buf->terminal) {
if (p_confirm || cmdmod.confirm) {
if (!dialog_close_terminal(buf)) {
return FAIL;
}
} else {
EMSG2(_("E89: %s will be killed (add ! to override)"),
(char *)buf->b_fname);
return FAIL;
}
} else {
EMSGN(_("E89: No write since last change for buffer %" PRId64
" (add ! to override)"),
buf->b_fnum);
return FAIL;
}
}
}
if (buf == curbuf && VIsual_active) {
end_visual_mode();
}
bp = NULL;
FOR_ALL_BUFFERS(bp2) {
if (bp2->b_p_bl && bp2 != buf) {
bp = bp2;
break;
}
}
if (bp == NULL && buf == curbuf) {
return empty_curbuf(true, forceit, action);
}
while (buf == curbuf
&& !(curwin->w_closing || curwin->w_buffer->b_locked > 0)
&& (!ONE_WINDOW || first_tabpage->tp_next != NULL)) {
if (win_close(curwin, false) == FAIL) {
break;
}
}
if (buf != curbuf) {
close_windows(buf, false);
if (buf != curbuf && bufref_valid(&bufref) && buf->b_nwindows <= 0) {
close_buffer(NULL, buf, action, false);
}
return OK;
}
buf = NULL; 
bp = NULL; 
if (au_new_curbuf.br_buf != NULL && bufref_valid(&au_new_curbuf)) {
buf = au_new_curbuf.br_buf;
} else if (curwin->w_jumplistlen > 0) {
int jumpidx;
jumpidx = curwin->w_jumplistidx - 1;
if (jumpidx < 0) {
jumpidx = curwin->w_jumplistlen - 1;
}
forward = jumpidx;
while (jumpidx != curwin->w_jumplistidx) {
buf = buflist_findnr(curwin->w_jumplist[jumpidx].fmark.fnum);
if (buf != NULL) {
if (buf == curbuf || !buf->b_p_bl) {
buf = NULL; 
} else if (buf->b_ml.ml_mfp == NULL) {
if (bp == NULL) {
bp = buf;
}
buf = NULL;
}
}
if (buf != NULL) { 
break;
}
if (!jumpidx && curwin->w_jumplistidx == curwin->w_jumplistlen) {
break;
}
if (--jumpidx < 0) {
jumpidx = curwin->w_jumplistlen - 1;
}
if (jumpidx == forward) { 
break;
}
}
}
if (buf == NULL) { 
forward = true;
buf = curbuf->b_next;
for (;; ) {
if (buf == NULL) {
if (!forward) { 
break;
}
buf = curbuf->b_prev;
forward = false;
continue;
}
if (buf->b_help == curbuf->b_help && buf->b_p_bl) {
if (buf->b_ml.ml_mfp != NULL) { 
break;
}
if (bp == NULL) { 
bp = buf;
}
}
if (forward) {
buf = buf->b_next;
} else {
buf = buf->b_prev;
}
}
}
if (buf == NULL) { 
buf = bp;
}
if (buf == NULL) { 
FOR_ALL_BUFFERS(buf2) {
if (buf2->b_p_bl && buf2 != curbuf) {
buf = buf2;
break;
}
}
}
if (buf == NULL) { 
if (curbuf->b_next != NULL) {
buf = curbuf->b_next;
} else {
buf = curbuf->b_prev;
}
}
}
if (buf == NULL) {
return empty_curbuf(false, forceit, action);
}
if (action == DOBUF_SPLIT) { 
if ((swb_flags & SWB_USEOPEN) && buf_jump_open_win(buf)) {
return OK;
}
if ((swb_flags & SWB_USETAB) && buf_jump_open_tab(buf)) {
return OK;
}
if (win_split(0, 0) == FAIL) {
return FAIL;
}
}
if (buf == curbuf) {
return OK;
}
if (action == DOBUF_GOTO && !can_abandon(curbuf, forceit)) {
if ((p_confirm || cmdmod.confirm) && p_write) {
bufref_T bufref;
set_bufref(&bufref, buf);
dialog_changed(curbuf, false);
if (!bufref_valid(&bufref)) {
return FAIL;
}
}
if (bufIsChanged(curbuf)) {
no_write_message();
return FAIL;
}
}
set_curbuf(buf, action);
if (action == DOBUF_SPLIT) {
RESET_BINDING(curwin); 
}
if (aborting()) { 
return FAIL;
}
return OK;
}
void set_curbuf(buf_T *buf, int action)
{
buf_T *prevbuf;
int unload = (action == DOBUF_UNLOAD || action == DOBUF_DEL
|| action == DOBUF_WIPE);
long old_tw = curbuf->b_p_tw;
setpcmark();
if (!cmdmod.keepalt) {
curwin->w_alt_fnum = curbuf->b_fnum; 
}
buflist_altfpos(curwin); 
VIsual_reselect = false;
prevbuf = curbuf;
bufref_T newbufref;
bufref_T prevbufref;
set_bufref(&prevbufref, prevbuf);
set_bufref(&newbufref, buf);
if (!apply_autocmds(EVENT_BUFLEAVE, NULL, NULL, false, curbuf)
|| (bufref_valid(&prevbufref) && bufref_valid(&newbufref)
&& !aborting())) {
if (prevbuf == curwin->w_buffer) {
reset_synblock(curwin);
}
if (unload) {
close_windows(prevbuf, false);
}
if (bufref_valid(&prevbufref) && !aborting()) {
win_T *previouswin = curwin;
if (prevbuf == curbuf) {
u_sync(false);
}
close_buffer(prevbuf == curwin->w_buffer ? curwin : NULL,
prevbuf,
unload
? action
: (action == DOBUF_GOTO && !buf_hide(prevbuf)
&& !bufIsChanged(prevbuf)) ? DOBUF_UNLOAD : 0,
false);
if (curwin != previouswin && win_valid(previouswin)) {
curwin = previouswin;
}
}
}
if ((buf_valid(buf) && buf != curbuf
&& !aborting()
) || curwin->w_buffer == NULL
) {
enter_buffer(buf);
if (old_tw != curbuf->b_p_tw) {
check_colorcolumn(curwin);
}
}
if (bufref_valid(&prevbufref) && prevbuf->terminal != NULL) {
terminal_check_size(prevbuf->terminal);
}
}
void enter_buffer(buf_T *buf)
{
buf_copy_options(buf, BCO_ENTER | BCO_NOHELP);
if (!buf->b_help) {
get_winopts(buf);
} else {
clearFolding(curwin);
}
foldUpdateAll(curwin); 
curwin->w_buffer = buf;
curbuf = buf;
curbuf->b_nwindows++;
if (curwin->w_p_diff) {
diff_buf_add(curbuf);
}
curwin->w_s = &(curbuf->b_s);
curwin->w_cursor.lnum = 1;
curwin->w_cursor.col = 0;
curwin->w_cursor.coladd = 0;
curwin->w_set_curswant = true;
curwin->w_topline_was_set = false;
curwin->w_valid = 0;
if (curbuf->b_ml.ml_mfp == NULL) { 
if (*curbuf->b_p_ft == NUL) {
did_filetype = false;
}
open_buffer(false, NULL, 0);
} else {
if (!msg_silent && !shortmess(SHM_FILEINFO)) {
need_fileinfo = true; 
}
(void)buf_check_timestamp(curbuf, false);
curwin->w_topline = 1;
curwin->w_topfill = 0;
apply_autocmds(EVENT_BUFENTER, NULL, NULL, false, curbuf);
apply_autocmds(EVENT_BUFWINENTER, NULL, NULL, false, curbuf);
}
if (curwin->w_cursor.lnum == 1 && inindent(0)) {
buflist_getfpos();
}
check_arg_idx(curwin); 
maketitle();
if (curwin->w_topline == 1 && !curwin->w_topline_was_set) {
scroll_cursor_halfway(false); 
}
do_autochdir();
if (curbuf->b_kmap_state & KEYMAP_INIT) {
(void)keymap_init();
}
if (!curbuf->b_help && curwin->w_p_spell && *curwin->w_s->b_p_spl != NUL) {
(void)did_set_spelllang(curwin);
}
redraw_later(NOT_VALID);
}
void do_autochdir(void)
{
if (p_acd) {
if (starting == 0
&& curbuf->b_ffname != NULL
&& vim_chdirfile(curbuf->b_ffname) == OK) {
post_chdir(kCdScopeGlobal, false);
shorten_fnames(true);
}
}
}
void no_write_message(void)
{
EMSG(_("E37: No write since last change (add ! to override)"));
}
void no_write_message_nobang(void)
{
EMSG(_("E37: No write since last change"));
}
static int top_file_num = 1; 
static inline void buf_init_changedtick(buf_T *const buf)
FUNC_ATTR_ALWAYS_INLINE FUNC_ATTR_NONNULL_ALL
{
STATIC_ASSERT(sizeof("changedtick") <= sizeof(buf->changedtick_di.di_key),
"buf->changedtick_di cannot hold large enough keys");
buf->changedtick_di = (ChangedtickDictItem) {
.di_flags = DI_FLAGS_RO|DI_FLAGS_FIX, 
.di_tv = (typval_T) {
.v_type = VAR_NUMBER,
.v_lock = VAR_FIXED,
.vval.v_number = buf_get_changedtick(buf),
},
.di_key = "changedtick",
};
tv_dict_add(buf->b_vars, (dictitem_T *)&buf->changedtick_di);
}
buf_T * buflist_new(char_u *ffname, char_u *sfname, linenr_T lnum, int flags)
{
buf_T *buf;
fname_expand(curbuf, &ffname, &sfname); 
FileID file_id;
bool file_id_valid = (sfname != NULL
&& os_fileid((char *)sfname, &file_id));
if (ffname != NULL && !(flags & (BLN_DUMMY | BLN_NEW))
&& (buf = buflist_findname_file_id(ffname, &file_id,
file_id_valid)) != NULL) {
xfree(ffname);
if (lnum != 0) {
buflist_setfpos(buf, curwin, lnum, (colnr_T)0, false);
}
if ((flags & BLN_NOOPT) == 0) {
buf_copy_options(buf, 0);
}
if ((flags & BLN_LISTED) && !buf->b_p_bl) {
buf->b_p_bl = true;
bufref_T bufref;
set_bufref(&bufref, buf);
if (!(flags & BLN_DUMMY)) {
if (apply_autocmds(EVENT_BUFADD, NULL, NULL, false, buf)
&& !bufref_valid(&bufref)) {
return NULL;
}
}
}
return buf;
}
buf = NULL;
if ((flags & BLN_CURBUF) && curbuf_reusable()) {
assert(curbuf != NULL);
buf = curbuf;
if (curbuf->b_p_bl) {
apply_autocmds(EVENT_BUFDELETE, NULL, NULL, false, curbuf);
}
if (buf == curbuf) {
apply_autocmds(EVENT_BUFWIPEOUT, NULL, NULL, false, curbuf);
}
if (aborting()) { 
return NULL;
}
if (buf == curbuf) {
clear_string_option(&buf->b_p_bh);
clear_string_option(&buf->b_p_bt);
}
}
if (buf != curbuf || curbuf == NULL) {
buf = xcalloc(1, sizeof(buf_T));
buf->b_vars = tv_dict_alloc();
buf->b_signcols_max = -1;
init_var_dict(buf->b_vars, &buf->b_bufvar, VAR_SCOPE);
buf_init_changedtick(buf);
}
if (ffname != NULL) {
buf->b_ffname = ffname;
buf->b_sfname = vim_strsave(sfname);
}
clear_wininfo(buf);
buf->b_wininfo = xcalloc(1, sizeof(wininfo_T));
if (ffname != NULL && (buf->b_ffname == NULL || buf->b_sfname == NULL)) {
XFREE_CLEAR(buf->b_ffname);
XFREE_CLEAR(buf->b_sfname);
if (buf != curbuf) {
free_buffer(buf);
}
return NULL;
}
if (buf == curbuf) {
buf_freeall(buf, 0);
if (buf != curbuf) { 
return NULL;
}
if (aborting()) { 
return NULL;
}
free_buffer_stuff(buf, kBffInitChangedtick); 
buf->b_p_initialized = false;
buf_copy_options(buf, BCO_ENTER);
curbuf->b_kmap_state |= KEYMAP_INIT;
} else {
buf->b_next = NULL;
if (firstbuf == NULL) { 
buf->b_prev = NULL;
firstbuf = buf;
} else { 
lastbuf->b_next = buf;
buf->b_prev = lastbuf;
}
lastbuf = buf;
buf->b_fnum = top_file_num++;
handle_register_buffer(buf);
if (top_file_num < 0) { 
EMSG(_("W14: Warning: List of file names overflow"));
if (emsg_silent == 0) {
ui_flush();
os_delay(3000L, true); 
}
top_file_num = 1;
}
buf_copy_options(buf, BCO_ALWAYS);
}
buf->b_wininfo->wi_fpos.lnum = lnum;
buf->b_wininfo->wi_win = curwin;
hash_init(&buf->b_s.b_keywtab);
hash_init(&buf->b_s.b_keywtab_ic);
buf->b_fname = buf->b_sfname;
if (!file_id_valid) {
buf->file_id_valid = false;
} else {
buf->file_id_valid = true;
buf->file_id = file_id;
}
buf->b_u_synced = true;
buf->b_flags = BF_CHECK_RO | BF_NEVERLOADED;
if (flags & BLN_DUMMY) {
buf->b_flags |= BF_DUMMY;
}
buf_clear_file(buf);
clrallmarks(buf); 
fmarks_check_names(buf); 
buf->b_p_bl = (flags & BLN_LISTED) ? true : false; 
kv_destroy(buf->update_channels);
kv_init(buf->update_channels);
kv_destroy(buf->update_callbacks);
kv_init(buf->update_callbacks);
if (!(flags & BLN_DUMMY)) {
bufref_T bufref;
set_bufref(&bufref, buf);
if (apply_autocmds(EVENT_BUFNEW, NULL, NULL, false, buf)
&& !bufref_valid(&bufref)) {
return NULL;
}
if ((flags & BLN_LISTED)
&& apply_autocmds(EVENT_BUFADD, NULL, NULL, false, buf)
&& !bufref_valid(&bufref)) {
return NULL;
}
if (aborting()) {
return NULL;
}
}
buf->b_prompt_callback.type = kCallbackNone;
buf->b_prompt_interrupt.type = kCallbackNone;
buf->b_prompt_text = NULL;
return buf;
}
bool curbuf_reusable(void)
{
return (curbuf != NULL
&& curbuf->b_ffname == NULL
&& curbuf->b_nwindows <= 1
&& (curbuf->b_ml.ml_mfp == NULL || BUFEMPTY())
&& !bt_quickfix(curbuf)
&& !curbufIsChanged());
}
void free_buf_options(buf_T *buf, int free_p_ff)
{
if (free_p_ff) {
clear_string_option(&buf->b_p_fenc);
clear_string_option(&buf->b_p_ff);
clear_string_option(&buf->b_p_bh);
clear_string_option(&buf->b_p_bt);
}
clear_string_option(&buf->b_p_def);
clear_string_option(&buf->b_p_inc);
clear_string_option(&buf->b_p_inex);
clear_string_option(&buf->b_p_inde);
clear_string_option(&buf->b_p_indk);
clear_string_option(&buf->b_p_fp);
clear_string_option(&buf->b_p_fex);
clear_string_option(&buf->b_p_kp);
clear_string_option(&buf->b_p_mps);
clear_string_option(&buf->b_p_fo);
clear_string_option(&buf->b_p_flp);
clear_string_option(&buf->b_p_isk);
clear_string_option(&buf->b_p_keymap);
keymap_ga_clear(&buf->b_kmap_ga);
ga_clear(&buf->b_kmap_ga);
clear_string_option(&buf->b_p_com);
clear_string_option(&buf->b_p_cms);
clear_string_option(&buf->b_p_nf);
clear_string_option(&buf->b_p_syn);
clear_string_option(&buf->b_s.b_syn_isk);
clear_string_option(&buf->b_s.b_p_spc);
clear_string_option(&buf->b_s.b_p_spf);
vim_regfree(buf->b_s.b_cap_prog);
buf->b_s.b_cap_prog = NULL;
clear_string_option(&buf->b_s.b_p_spl);
clear_string_option(&buf->b_p_sua);
clear_string_option(&buf->b_p_ft);
clear_string_option(&buf->b_p_cink);
clear_string_option(&buf->b_p_cino);
clear_string_option(&buf->b_p_cinw);
clear_string_option(&buf->b_p_cpt);
clear_string_option(&buf->b_p_cfu);
clear_string_option(&buf->b_p_ofu);
clear_string_option(&buf->b_p_gp);
clear_string_option(&buf->b_p_mp);
clear_string_option(&buf->b_p_efm);
clear_string_option(&buf->b_p_ep);
clear_string_option(&buf->b_p_path);
clear_string_option(&buf->b_p_tags);
clear_string_option(&buf->b_p_tc);
clear_string_option(&buf->b_p_tfu);
clear_string_option(&buf->b_p_dict);
clear_string_option(&buf->b_p_tsr);
clear_string_option(&buf->b_p_qe);
buf->b_p_ar = -1;
buf->b_p_ul = NO_LOCAL_UNDOLEVEL;
clear_string_option(&buf->b_p_lw);
clear_string_option(&buf->b_p_bkc);
clear_string_option(&buf->b_p_menc);
}
int buflist_getfile(int n, linenr_T lnum, int options, int forceit)
{
buf_T *buf;
win_T *wp = NULL;
pos_T *fpos;
colnr_T col;
buf = buflist_findnr(n);
if (buf == NULL) {
if ((options & GETF_ALT) && n == 0) {
EMSG(_(e_noalt));
} else {
EMSGN(_("E92: Buffer %" PRId64 " not found"), n);
}
return FAIL;
}
if (buf == curbuf) {
return OK;
}
if (text_locked()) {
text_locked_msg();
return FAIL;
}
if (curbuf_locked()) {
return FAIL;
}
if (lnum == 0) {
fpos = buflist_findfpos(buf);
lnum = fpos->lnum;
col = fpos->col;
} else
col = 0;
if (options & GETF_SWITCH) {
if (swb_flags & SWB_USEOPEN) {
wp = buf_jump_open_win(buf);
}
if (wp == NULL && (swb_flags & SWB_USETAB)) {
wp = buf_jump_open_tab(buf);
}
if (wp == NULL && (swb_flags & (SWB_VSPLIT | SWB_SPLIT | SWB_NEWTAB))
&& !BUFEMPTY()) {
if (swb_flags & SWB_NEWTAB) {
tabpage_new();
} else if (win_split(0, (swb_flags & SWB_VSPLIT) ? WSP_VERT : 0)
== FAIL) {
return FAIL;
}
RESET_BINDING(curwin);
}
}
RedrawingDisabled++;
if (GETFILE_SUCCESS(getfile(buf->b_fnum, NULL, NULL,
(options & GETF_SETMARK), lnum, forceit))) {
RedrawingDisabled--;
if (!p_sol && col != 0) {
curwin->w_cursor.col = col;
check_cursor_col();
curwin->w_cursor.coladd = 0;
curwin->w_set_curswant = true;
}
return OK;
}
RedrawingDisabled--;
return FAIL;
}
void buflist_getfpos(void)
{
pos_T *fpos;
fpos = buflist_findfpos(curbuf);
curwin->w_cursor.lnum = fpos->lnum;
check_cursor_lnum();
if (p_sol) {
curwin->w_cursor.col = 0;
} else {
curwin->w_cursor.col = fpos->col;
check_cursor_col();
curwin->w_cursor.coladd = 0;
curwin->w_set_curswant = true;
}
}
buf_T *buflist_findname_exp(char_u *fname)
{
char_u *ffname;
buf_T *buf = NULL;
ffname = (char_u *)FullName_save((char *)fname,
#if defined(UNIX)
true
#else
false
#endif
);
if (ffname != NULL) {
buf = buflist_findname(ffname);
xfree(ffname);
}
return buf;
}
buf_T *buflist_findname(char_u *ffname)
{
FileID file_id;
bool file_id_valid = os_fileid((char *)ffname, &file_id);
return buflist_findname_file_id(ffname, &file_id, file_id_valid);
}
static buf_T *buflist_findname_file_id(char_u *ffname, FileID *file_id,
bool file_id_valid)
{
FOR_ALL_BUFFERS_BACKWARDS(buf) {
if ((buf->b_flags & BF_DUMMY) == 0
&& !otherfile_buf(buf, ffname, file_id, file_id_valid)) {
return buf;
}
}
return NULL;
}
int buflist_findpat(
const char_u *pattern,
const char_u *pattern_end, 
int unlisted, 
int diffmode, 
int curtab_only 
)
{
int match = -1;
int find_listed;
char_u *pat;
char_u *patend;
int attempt;
char_u *p;
int toggledollar;
if (pattern_end == pattern + 1 && (*pattern == '%' || *pattern == '#')) {
if (*pattern == '%') {
match = curbuf->b_fnum;
} else {
match = curwin->w_alt_fnum;
}
buf_T *found_buf = buflist_findnr(match);
if (diffmode && !(found_buf && diff_mode_buf(found_buf))) {
match = -1;
}
} else {
pat = file_pat_to_reg_pat(pattern, pattern_end, NULL, false);
if (pat == NULL) {
return -1;
}
patend = pat + STRLEN(pat) - 1;
toggledollar = (patend > pat && *patend == '$');
find_listed = true;
for (;; ) {
for (attempt = 0; attempt <= 3; attempt++) {
if (toggledollar) {
*patend = (attempt < 2) ? NUL : '$'; 
}
p = pat;
if (*p == '^' && !(attempt & 1)) { 
p++;
}
regmatch_T regmatch;
regmatch.regprog = vim_regcomp(p, p_magic ? RE_MAGIC : 0);
if (regmatch.regprog == NULL) {
xfree(pat);
return -1;
}
FOR_ALL_BUFFERS_BACKWARDS(buf) {
if (buf->b_p_bl == find_listed
&& (!diffmode || diff_mode_buf(buf))
&& buflist_match(&regmatch, buf, false) != NULL) {
if (curtab_only) {
bool found_window = false;
FOR_ALL_WINDOWS_IN_TAB(wp, curtab) {
if (wp->w_buffer == buf) {
found_window = true;
break;
}
}
if (!found_window) {
continue;
}
}
if (match >= 0) { 
match = -2;
break;
}
match = buf->b_fnum; 
}
}
vim_regfree(regmatch.regprog);
if (match >= 0) { 
break;
}
}
if (!unlisted || !find_listed || match != -1) {
break;
}
find_listed = false;
}
xfree(pat);
}
if (match == -2) {
EMSG2(_("E93: More than one match for %s"), pattern);
} else if (match < 0) {
EMSG2(_("E94: No matching buffer for %s"), pattern);
}
return match;
}
int ExpandBufnames(char_u *pat, int *num_file, char_u ***file, int options)
{
int count = 0;
int round;
char_u *p;
int attempt;
char_u *patc;
*num_file = 0; 
*file = NULL;
if (*pat == '^') {
patc = xmalloc(STRLEN(pat) + 11);
STRCPY(patc, "\\(^\\|[\\/]\\)");
STRCPY(patc + 11, pat + 1);
} else
patc = pat;
for (attempt = 0; attempt <= 1; attempt++) {
if (attempt > 0 && patc == pat) {
break; 
}
regmatch_T regmatch;
regmatch.regprog = vim_regcomp(patc + attempt * 11, RE_MAGIC);
if (regmatch.regprog == NULL) {
if (patc != pat) {
xfree(patc);
}
return FAIL;
}
for (round = 1; round <= 2; round++) {
count = 0;
FOR_ALL_BUFFERS(buf) {
if (!buf->b_p_bl) { 
continue;
}
p = buflist_match(&regmatch, buf, p_wic);
if (p != NULL) {
if (round == 1) {
count++;
} else {
if (options & WILD_HOME_REPLACE) {
p = home_replace_save(buf, p);
} else {
p = vim_strsave(p);
}
(*file)[count++] = p;
}
}
}
if (count == 0) { 
break;
}
if (round == 1) {
*file = xmalloc((size_t)count * sizeof(**file));
}
}
vim_regfree(regmatch.regprog);
if (count) { 
break;
}
}
if (patc != pat) {
xfree(patc);
}
*num_file = count;
return count == 0 ? FAIL : OK;
}
static char_u *buflist_match(regmatch_T *rmp, buf_T *buf, bool ignore_case)
{
char_u *match = fname_match(rmp, buf->b_sfname, ignore_case);
if (match == NULL) {
match = fname_match(rmp, buf->b_ffname, ignore_case);
}
return match;
}
static char_u *fname_match(regmatch_T *rmp, char_u *name, bool ignore_case)
{
char_u *match = NULL;
char_u *p;
if (name != NULL) {
rmp->rm_ic = p_fic || ignore_case;
if (vim_regexec(rmp, name, (colnr_T)0)) {
match = name;
} else {
p = home_replace_save(NULL, name);
if (vim_regexec(rmp, p, (colnr_T)0)) {
match = name;
}
xfree(p);
}
}
return match;
}
buf_T *buflist_findnr(int nr)
{
if (nr == 0) {
nr = curwin->w_alt_fnum;
}
return handle_get_buffer((handle_T)nr);
}
char_u *
buflist_nr2name(
int n,
int fullname,
int helptail 
)
{
buf_T *buf;
buf = buflist_findnr(n);
if (buf == NULL) {
return NULL;
}
return home_replace_save(helptail ? buf : NULL,
fullname ? buf->b_ffname : buf->b_fname);
}
void buflist_setfpos(buf_T *const buf, win_T *const win,
linenr_T lnum, colnr_T col,
bool copy_options)
FUNC_ATTR_NONNULL_ALL
{
wininfo_T *wip;
for (wip = buf->b_wininfo; wip != NULL; wip = wip->wi_next) {
if (wip->wi_win == win) {
break;
}
}
if (wip == NULL) {
wip = xcalloc(1, sizeof(wininfo_T));
wip->wi_win = win;
if (lnum == 0) { 
lnum = 1;
}
} else {
if (wip->wi_prev) {
wip->wi_prev->wi_next = wip->wi_next;
} else {
buf->b_wininfo = wip->wi_next;
}
if (wip->wi_next) {
wip->wi_next->wi_prev = wip->wi_prev;
}
if (copy_options && wip->wi_optset) {
clear_winopt(&wip->wi_opt);
deleteFoldRecurse(&wip->wi_folds);
}
}
if (lnum != 0) {
wip->wi_fpos.lnum = lnum;
wip->wi_fpos.col = col;
}
if (copy_options) {
copy_winopt(&win->w_onebuf_opt, &wip->wi_opt);
wip->wi_fold_manual = win->w_fold_manual;
cloneFoldGrowArray(&win->w_folds, &wip->wi_folds);
wip->wi_optset = true;
}
wip->wi_next = buf->b_wininfo;
buf->b_wininfo = wip;
wip->wi_prev = NULL;
if (wip->wi_next) {
wip->wi_next->wi_prev = wip;
}
return;
}
static bool wininfo_other_tab_diff(wininfo_T *wip)
FUNC_ATTR_PURE FUNC_ATTR_WARN_UNUSED_RESULT FUNC_ATTR_NONNULL_ALL
{
if (wip->wi_opt.wo_diff) {
FOR_ALL_WINDOWS_IN_TAB(wp, curtab) {
if (wip->wi_win == wp) {
return false;
}
}
return true;
}
return false;
}
static wininfo_T *find_wininfo(buf_T *buf, int skip_diff_buffer)
{
wininfo_T *wip;
for (wip = buf->b_wininfo; wip != NULL; wip = wip->wi_next)
if (wip->wi_win == curwin
&& (!skip_diff_buffer || !wininfo_other_tab_diff(wip))
)
break;
if (wip == NULL) {
if (skip_diff_buffer) {
for (wip = buf->b_wininfo; wip != NULL; wip = wip->wi_next) {
if (!wininfo_other_tab_diff(wip)) {
break;
}
}
} else {
wip = buf->b_wininfo;
}
}
return wip;
}
void get_winopts(buf_T *buf)
{
wininfo_T *wip;
clear_winopt(&curwin->w_onebuf_opt);
clearFolding(curwin);
wip = find_wininfo(buf, true);
if (wip != NULL && wip->wi_win != curwin && wip->wi_win != NULL
&& wip->wi_win->w_buffer == buf) {
win_T *wp = wip->wi_win;
copy_winopt(&wp->w_onebuf_opt, &curwin->w_onebuf_opt);
curwin->w_fold_manual = wp->w_fold_manual;
curwin->w_foldinvalid = true;
cloneFoldGrowArray(&wp->w_folds, &curwin->w_folds);
} else if (wip != NULL && wip->wi_optset) {
copy_winopt(&wip->wi_opt, &curwin->w_onebuf_opt);
curwin->w_fold_manual = wip->wi_fold_manual;
curwin->w_foldinvalid = true;
cloneFoldGrowArray(&wip->wi_folds, &curwin->w_folds);
} else
copy_winopt(&curwin->w_allbuf_opt, &curwin->w_onebuf_opt);
if (curwin->w_float_config.style == kWinStyleMinimal) {
didset_window_options(curwin);
win_set_minimal_style(curwin);
}
if (p_fdls >= 0) {
curwin->w_p_fdl = p_fdls;
}
didset_window_options(curwin);
}
pos_T *buflist_findfpos(buf_T *buf)
{
static pos_T no_position = { 1, 0, 0 };
wininfo_T *wip = find_wininfo(buf, false);
return (wip == NULL) ? &no_position : &(wip->wi_fpos);
}
linenr_T buflist_findlnum(buf_T *buf)
{
return buflist_findfpos(buf)->lnum;
}
void buflist_list(exarg_T *eap)
{
buf_T *buf;
int len;
int i;
for (buf = firstbuf; buf != NULL && !got_int; buf = buf->b_next) {
if ((!buf->b_p_bl && !eap->forceit && !strchr((char *)eap->arg, 'u'))
|| (strchr((char *)eap->arg, 'u') && buf->b_p_bl)
|| (strchr((char *)eap->arg, '+')
&& ((buf->b_flags & BF_READERR) || !bufIsChanged(buf)))
|| (strchr((char *)eap->arg, 'a')
&& (buf->b_ml.ml_mfp == NULL || buf->b_nwindows == 0))
|| (strchr((char *)eap->arg, 'h')
&& (buf->b_ml.ml_mfp == NULL || buf->b_nwindows != 0))
|| (strchr((char *)eap->arg, '-') && buf->b_p_ma)
|| (strchr((char *)eap->arg, '=') && !buf->b_p_ro)
|| (strchr((char *)eap->arg, 'x') && !(buf->b_flags & BF_READERR))
|| (strchr((char *)eap->arg, '%') && buf != curbuf)
|| (strchr((char *)eap->arg, '#')
&& (buf == curbuf || curwin->w_alt_fnum != buf->b_fnum))) {
continue;
}
if (buf_spname(buf) != NULL) {
STRLCPY(NameBuff, buf_spname(buf), MAXPATHL);
} else {
home_replace(buf, buf->b_fname, NameBuff, MAXPATHL, true);
}
if (message_filtered(NameBuff)) {
continue;
}
const int changed_char = (buf->b_flags & BF_READERR)
? 'x'
: (bufIsChanged(buf) ? '+' : ' ');
int ro_char = !MODIFIABLE(buf) ? '-' : (buf->b_p_ro ? '=' : ' ');
if (buf->terminal) {
ro_char = channel_job_running((uint64_t)buf->b_p_channel) ? 'R' : 'F';
}
msg_putchar('\n');
len = vim_snprintf(
(char *)IObuff, IOSIZE - 20, "%3d%c%c%c%c%c \"%s\"",
buf->b_fnum,
buf->b_p_bl ? ' ' : 'u',
buf == curbuf ? '%' : (curwin->w_alt_fnum == buf->b_fnum ? '#' : ' '),
buf->b_ml.ml_mfp == NULL ? ' ' : (buf->b_nwindows == 0 ? 'h' : 'a'),
ro_char,
changed_char,
NameBuff);
if (len > IOSIZE - 20) {
len = IOSIZE - 20;
}
i = 40 - vim_strsize(IObuff);
do {
IObuff[len++] = ' ';
} while (--i > 0 && len < IOSIZE - 18);
vim_snprintf((char *)IObuff + len, (size_t)(IOSIZE - len),
_("line %" PRId64),
buf == curbuf ? (int64_t)curwin->w_cursor.lnum
: (int64_t)buflist_findlnum(buf));
msg_outtrans(IObuff);
line_breakcheck();
}
}
int buflist_name_nr(int fnum, char_u **fname, linenr_T *lnum)
{
buf_T *buf;
buf = buflist_findnr(fnum);
if (buf == NULL || buf->b_fname == NULL) {
return FAIL;
}
*fname = buf->b_fname;
*lnum = buflist_findlnum(buf);
return OK;
}
int
setfname(
buf_T *buf,
char_u *ffname,
char_u *sfname,
bool message 
)
{
buf_T *obuf = NULL;
FileID file_id;
bool file_id_valid = false;
if (ffname == NULL || *ffname == NUL) {
XFREE_CLEAR(buf->b_ffname);
XFREE_CLEAR(buf->b_sfname);
} else {
fname_expand(buf, &ffname, &sfname); 
if (ffname == NULL) { 
return FAIL;
}
file_id_valid = os_fileid((char *)ffname, &file_id);
if (!(buf->b_flags & BF_DUMMY)) {
obuf = buflist_findname_file_id(ffname, &file_id, file_id_valid);
}
if (obuf != NULL && obuf != buf) {
if (obuf->b_ml.ml_mfp != NULL) { 
if (message) {
EMSG(_("E95: Buffer with this name already exists"));
}
xfree(ffname);
return FAIL;
}
close_buffer(NULL, obuf, DOBUF_WIPE, false);
}
sfname = vim_strsave(sfname);
#if defined(USE_FNAME_CASE)
path_fix_case(sfname); 
#endif
xfree(buf->b_ffname);
xfree(buf->b_sfname);
buf->b_ffname = ffname;
buf->b_sfname = sfname;
}
buf->b_fname = buf->b_sfname;
if (!file_id_valid) {
buf->file_id_valid = false;
} else {
buf->file_id_valid = true;
buf->file_id = file_id;
}
buf_name_changed(buf);
return OK;
}
void buf_set_name(int fnum, char_u *name)
{
buf_T *buf;
buf = buflist_findnr(fnum);
if (buf != NULL) {
xfree(buf->b_sfname);
xfree(buf->b_ffname);
buf->b_ffname = vim_strsave(name);
buf->b_sfname = NULL;
fname_expand(buf, &buf->b_ffname, &buf->b_sfname);
buf->b_fname = buf->b_sfname;
}
}
void buf_name_changed(buf_T *buf)
{
if (buf->b_ml.ml_mfp != NULL) {
ml_setname(buf);
}
if (curwin->w_buffer == buf) {
check_arg_idx(curwin); 
}
maketitle(); 
status_redraw_all(); 
fmarks_check_names(buf); 
ml_timestamp(buf); 
}
buf_T *setaltfname(char_u *ffname, char_u *sfname, linenr_T lnum)
{
buf_T *buf;
buf = buflist_new(ffname, sfname, lnum, 0);
if (buf != NULL && !cmdmod.keepalt) {
curwin->w_alt_fnum = buf->b_fnum;
}
return buf;
}
char_u * getaltfname(
bool errmsg 
)
{
char_u *fname;
linenr_T dummy;
if (buflist_name_nr(0, &fname, &dummy) == FAIL) {
if (errmsg) {
EMSG(_(e_noalt));
}
return NULL;
}
return fname;
}
int buflist_add(char_u *fname, int flags)
{
buf_T *buf;
buf = buflist_new(fname, NULL, (linenr_T)0, flags);
if (buf != NULL) {
return buf->b_fnum;
}
return 0;
}
#if defined(BACKSLASH_IN_FILENAME)
void buflist_slash_adjust(void)
{
FOR_ALL_BUFFERS(bp) {
if (bp->b_ffname != NULL) {
slash_adjust(bp->b_ffname);
}
if (bp->b_sfname != NULL) {
slash_adjust(bp->b_sfname);
}
}
}
#endif
void buflist_altfpos(win_T *win)
{
buflist_setfpos(curbuf, win, win->w_cursor.lnum, win->w_cursor.col, true);
}
bool otherfile(char_u *ffname)
FUNC_ATTR_PURE FUNC_ATTR_WARN_UNUSED_RESULT FUNC_ATTR_NONNULL_ALL
{
return otherfile_buf(curbuf, ffname, NULL, false);
}
static bool otherfile_buf(buf_T *buf, char_u *ffname, FileID *file_id_p,
bool file_id_valid)
FUNC_ATTR_PURE FUNC_ATTR_WARN_UNUSED_RESULT
{
if (ffname == NULL || *ffname == NUL || buf->b_ffname == NULL) {
return true;
}
if (fnamecmp(ffname, buf->b_ffname) == 0) {
return false;
}
{
FileID file_id;
if (file_id_p == NULL) {
file_id_p = &file_id;
file_id_valid = os_fileid((char *)ffname, file_id_p);
}
if (!file_id_valid) {
return true;
}
if (buf_same_file_id(buf, file_id_p)) {
buf_set_file_id(buf);
if (buf_same_file_id(buf, file_id_p)) {
return false;
}
}
}
return true;
}
void buf_set_file_id(buf_T *buf)
{
FileID file_id;
if (buf->b_fname != NULL
&& os_fileid((char *)buf->b_fname, &file_id)) {
buf->file_id_valid = true;
buf->file_id = file_id;
} else {
buf->file_id_valid = false;
}
}
static bool buf_same_file_id(buf_T *buf, FileID *file_id)
FUNC_ATTR_PURE FUNC_ATTR_WARN_UNUSED_RESULT FUNC_ATTR_NONNULL_ALL
{
return buf->file_id_valid && os_fileid_equal(&(buf->file_id), file_id);
}
void
fileinfo(
int fullname, 
int shorthelp,
int dont_truncate
)
{
char_u *name;
int n;
char_u *p;
char_u *buffer;
size_t len;
buffer = xmalloc(IOSIZE);
if (fullname > 1) { 
vim_snprintf((char *)buffer, IOSIZE, "buf %d: ", curbuf->b_fnum);
p = buffer + STRLEN(buffer);
} else
p = buffer;
*p++ = '"';
if (buf_spname(curbuf) != NULL) {
STRLCPY(p, buf_spname(curbuf), IOSIZE - (p - buffer));
} else {
if (!fullname && curbuf->b_fname != NULL) {
name = curbuf->b_fname;
} else {
name = curbuf->b_ffname;
}
home_replace(shorthelp ? curbuf : NULL, name, p,
(size_t)(IOSIZE - (p - buffer)), true);
}
vim_snprintf_add((char *)buffer, IOSIZE, "\"%s%s%s%s%s%s",
curbufIsChanged() ? (shortmess(SHM_MOD)
? " [+]" : _(" [Modified]")) : " ",
(curbuf->b_flags & BF_NOTEDITED)
&& !bt_dontwrite(curbuf)
? _("[Not edited]") : "",
(curbuf->b_flags & BF_NEW)
&& !bt_dontwrite(curbuf)
? _("[New file]") : "",
(curbuf->b_flags & BF_READERR) ? _("[Read errors]") : "",
curbuf->b_p_ro ? (shortmess(SHM_RO) ? _("[RO]")
: _("[readonly]")) : "",
(curbufIsChanged() || (curbuf->b_flags & BF_WRITE_MASK)
|| curbuf->b_p_ro) ?
" " : "");
if (curwin->w_cursor.lnum > 1000000L)
n = (int)(((long)curwin->w_cursor.lnum) /
((long)curbuf->b_ml.ml_line_count / 100L));
else
n = (int)(((long)curwin->w_cursor.lnum * 100L) /
(long)curbuf->b_ml.ml_line_count);
if (curbuf->b_ml.ml_flags & ML_EMPTY) {
vim_snprintf_add((char *)buffer, IOSIZE, "%s", _(no_lines_msg));
} else if (p_ru) {
if (curbuf->b_ml.ml_line_count == 1) {
vim_snprintf_add((char *)buffer, IOSIZE, _("1 line --%d%%--"), n);
} else {
vim_snprintf_add((char *)buffer, IOSIZE, _("%" PRId64 " lines --%d%%--"),
(int64_t)curbuf->b_ml.ml_line_count, n);
}
} else {
vim_snprintf_add((char *)buffer, IOSIZE,
_("line %" PRId64 " of %" PRId64 " --%d%%-- col "),
(int64_t)curwin->w_cursor.lnum,
(int64_t)curbuf->b_ml.ml_line_count,
n);
validate_virtcol();
len = STRLEN(buffer);
col_print(buffer + len, IOSIZE - len,
(int)curwin->w_cursor.col + 1, (int)curwin->w_virtcol + 1);
}
(void)append_arg_number(curwin, buffer, IOSIZE, !shortmess(SHM_FILE));
if (dont_truncate) {
msg_start();
n = msg_scroll;
msg_scroll = true;
msg(buffer);
msg_scroll = n;
} else {
p = msg_trunc_attr(buffer, false, 0);
if (restart_edit != 0 || (msg_scrolled && !need_wait_return)) {
set_keep_msg(p, 0);
}
}
xfree(buffer);
}
void col_print(char_u *buf, size_t buflen, int col, int vcol)
{
if (col == vcol) {
vim_snprintf((char *)buf, buflen, "%d", col);
} else {
vim_snprintf((char *)buf, buflen, "%d-%d", col, vcol);
}
}
static char_u *lasttitle = NULL;
static char_u *lasticon = NULL;
void maketitle(void)
{
char_u *title_str = NULL;
char_u *icon_str = NULL;
int maxlen = 0;
int len;
int mustset;
char buf[IOSIZE];
if (!redrawing()) {
need_maketitle = true;
return;
}
need_maketitle = false;
if (!p_title && !p_icon && lasttitle == NULL && lasticon == NULL) {
return; 
}
if (p_title) {
if (p_titlelen > 0) {
maxlen = (int)(p_titlelen * Columns / 100);
if (maxlen < 10) {
maxlen = 10;
}
}
if (*p_titlestring != NUL) {
if (stl_syntax & STL_IN_TITLE) {
int use_sandbox = false;
int save_called_emsg = called_emsg;
use_sandbox = was_set_insecurely((char_u *)"titlestring", 0);
called_emsg = false;
build_stl_str_hl(curwin, (char_u *)buf, sizeof(buf),
p_titlestring, use_sandbox,
0, maxlen, NULL, NULL);
title_str = (char_u *)buf;
if (called_emsg) {
set_string_option_direct((char_u *)"titlestring", -1, (char_u *)"",
OPT_FREE, SID_ERROR);
}
called_emsg |= save_called_emsg;
} else {
title_str = p_titlestring;
}
} else {
#define SPACE_FOR_FNAME (sizeof(buf) - 100)
#define SPACE_FOR_DIR (sizeof(buf) - 20)
#define SPACE_FOR_ARGNR (sizeof(buf) - 10) 
char *buf_p = buf;
if (curbuf->b_fname == NULL) {
const size_t size = xstrlcpy(buf_p, _("[No Name]"),
SPACE_FOR_FNAME + 1);
buf_p += MIN(size, SPACE_FOR_FNAME);
} else {
buf_p += transstr_buf((const char *)path_tail(curbuf->b_fname),
buf_p, SPACE_FOR_FNAME + 1);
}
switch (bufIsChanged(curbuf)
| (curbuf->b_p_ro << 1)
| (!MODIFIABLE(curbuf) << 2)) {
case 0: break;
case 1: buf_p = strappend(buf_p, " +"); break;
case 2: buf_p = strappend(buf_p, " ="); break;
case 3: buf_p = strappend(buf_p, " =+"); break;
case 4:
case 6: buf_p = strappend(buf_p, " -"); break;
case 5:
case 7: buf_p = strappend(buf_p, " -+"); break;
default: assert(false);
}
if (curbuf->b_fname != NULL) {
*buf_p++ = ' ';
*buf_p++ = '(';
home_replace(curbuf, curbuf->b_ffname, (char_u *)buf_p,
(SPACE_FOR_DIR - (size_t)(buf_p - buf)), true);
#if defined(BACKSLASH_IN_FILENAME)
if (isalpha((uint8_t)buf_p) && *(buf_p + 1) == ':') {
buf_p += 2;
}
#endif
char *p = (char *)path_tail_with_sep((char_u *)buf_p);
if (p == buf_p) {
xstrlcpy(buf_p, _("help"), SPACE_FOR_DIR - (size_t)(buf_p - buf));
} else {
*p = NUL;
}
if ((size_t)(buf_p - buf) < SPACE_FOR_DIR) {
char *const tbuf = transstr(buf_p);
const size_t free_space = SPACE_FOR_DIR - (size_t)(buf_p - buf) + 1;
const size_t dir_len = xstrlcpy(buf_p, tbuf, free_space);
buf_p += MIN(dir_len, free_space - 1);
xfree(tbuf);
} else {
const size_t free_space = SPACE_FOR_ARGNR - (size_t)(buf_p - buf) + 1;
const size_t dots_len = xstrlcpy(buf_p, "...", free_space);
buf_p += MIN(dots_len, free_space - 1);
}
*buf_p++ = ')';
*buf_p = NUL;
} else {
*buf_p = NUL;
}
append_arg_number(curwin, (char_u *)buf_p,
(int)(SPACE_FOR_ARGNR - (size_t)(buf_p - buf)), false);
xstrlcat(buf_p, " - NVIM", (sizeof(buf) - (size_t)(buf_p - buf)));
if (maxlen > 0) {
if (vim_strsize((char_u *)buf) > maxlen) {
trunc_string((char_u *)buf, (char_u *)buf, maxlen, sizeof(buf));
}
}
title_str = (char_u *)buf;
#undef SPACE_FOR_FNAME
#undef SPACE_FOR_DIR
#undef SPACE_FOR_ARGNR
}
}
mustset = value_change(title_str, &lasttitle);
if (p_icon) {
icon_str = (char_u *)buf;
if (*p_iconstring != NUL) {
if (stl_syntax & STL_IN_ICON) {
int use_sandbox = false;
int save_called_emsg = called_emsg;
use_sandbox = was_set_insecurely((char_u *)"iconstring", 0);
called_emsg = false;
build_stl_str_hl(curwin, icon_str, sizeof(buf),
p_iconstring, use_sandbox,
0, 0, NULL, NULL);
if (called_emsg) {
set_string_option_direct((char_u *)"iconstring", -1,
(char_u *)"", OPT_FREE, SID_ERROR);
}
called_emsg |= save_called_emsg;
} else {
icon_str = p_iconstring;
}
} else {
char_u *buf_p;
if (buf_spname(curbuf) != NULL) {
buf_p = buf_spname(curbuf);
} else { 
buf_p = path_tail(curbuf->b_ffname);
}
*icon_str = NUL;
len = (int)STRLEN(buf_p);
if (len > 100) {
len -= 100;
if (has_mbyte) {
len += (*mb_tail_off)(buf_p, buf_p + len) + 1;
}
buf_p += len;
}
STRCPY(icon_str, buf_p);
trans_characters(icon_str, IOSIZE);
}
}
mustset |= value_change(icon_str, &lasticon);
if (mustset) {
resettitle();
}
}
static bool value_change(char_u *str, char_u **last)
FUNC_ATTR_WARN_UNUSED_RESULT
{
if ((str == NULL) != (*last == NULL)
|| (str != NULL && *last != NULL && STRCMP(str, *last) != 0)) {
xfree(*last);
if (str == NULL) {
*last = NULL;
resettitle();
} else {
*last = vim_strsave(str);
return true;
}
}
return false;
}
void resettitle(void)
{
ui_call_set_icon(cstr_as_string((char *)lasticon));
ui_call_set_title(cstr_as_string((char *)lasttitle));
ui_flush();
}
#if defined(EXITFREE)
void free_titles(void)
{
xfree(lasttitle);
xfree(lasticon);
}
#endif
typedef enum {
kNumBaseDecimal = 10,
kNumBaseHexadecimal = 16
} NumberBase;
int build_stl_str_hl(
win_T *wp,
char_u *out,
size_t outlen,
char_u *fmt,
int use_sandbox,
char_u fillchar,
int maxwidth,
struct stl_hlrec *hltab,
StlClickRecord *tabtab
)
{
int groupitems[STL_MAX_ITEM];
struct stl_item {
char_u *start;
char *cmd;
int minwid;
int maxwid;
enum {
Normal,
Empty,
Group,
Separate,
Highlight,
TabPage,
ClickFunc,
Trunc
} type;
} items[STL_MAX_ITEM];
#define TMPLEN 70
char_u tmp[TMPLEN];
char_u *usefmt = fmt;
const int save_must_redraw = must_redraw;
const int save_redr_type = curwin->w_redr_type;
if (fmt[0] == '%' && fmt[1] == '!') {
usefmt = eval_to_string_safe(fmt + 2, NULL, use_sandbox);
if (usefmt == NULL) {
usefmt = fmt;
}
}
if (fillchar == 0) {
fillchar = ' ';
} else if (mb_char2len(fillchar) > 1) {
fillchar = '-';
}
linenr_T lnum = wp->w_cursor.lnum;
if (lnum > wp->w_buffer->b_ml.ml_line_count) {
lnum = wp->w_buffer->b_ml.ml_line_count;
wp->w_cursor.lnum = lnum;
}
const char_u *line_ptr = ml_get_buf(wp->w_buffer, lnum, false);
bool empty_line = (*line_ptr == NUL);
int byteval;
const size_t len = STRLEN(line_ptr);
if (wp->w_cursor.col > (colnr_T)len) {
wp->w_cursor.col = (colnr_T)len;
wp->w_cursor.coladd = 0;
byteval = 0;
} else {
byteval = utf_ptr2char(line_ptr + wp->w_cursor.col);
}
int groupdepth = 0;
int curitem = 0;
bool prevchar_isflag = true;
bool prevchar_isitem = false;
char_u *out_p = out;
char_u *out_end_p = (out + outlen) - 1;
for (char_u *fmt_p = usefmt; *fmt_p; ) {
if (curitem == STL_MAX_ITEM) {
if (out_p + 5 < out_end_p) {
memmove(out_p, " E541", (size_t)5);
out_p += 5;
}
break;
}
if (*fmt_p != NUL && *fmt_p != '%') {
prevchar_isflag = prevchar_isitem = false;
}
while (*fmt_p != NUL && *fmt_p != '%' && out_p < out_end_p)
*out_p++ = *fmt_p++;
if (*fmt_p == NUL || out_p >= out_end_p) {
break;
}
fmt_p++;
if (*fmt_p == NUL) {
break;
}
if (*fmt_p == '%') {
*out_p++ = *fmt_p++;
prevchar_isflag = prevchar_isitem = false;
continue;
}
if (*fmt_p == STL_SEPARATE) {
fmt_p++;
if (groupdepth > 0) {
continue;
}
items[curitem].type = Separate;
items[curitem++].start = out_p;
continue;
}
if (*fmt_p == STL_TRUNCMARK) {
fmt_p++;
items[curitem].type = Trunc;
items[curitem++].start = out_p;
continue;
}
if (*fmt_p == ')') {
fmt_p++;
if (groupdepth < 1) {
continue;
}
groupdepth--;
char_u *t = items[groupitems[groupdepth]].start;
*out_p = NUL;
long group_len = vim_strsize(t);
if (curitem > groupitems[groupdepth] + 1
&& items[groupitems[groupdepth]].minwid == 0) {
int group_start_userhl = 0;
int group_end_userhl = 0;
int n;
for (n = groupitems[groupdepth] - 1; n >= 0; n--) {
if (items[n].type == Highlight) {
group_start_userhl = group_end_userhl = items[n].minwid;
break;
}
}
for (n = groupitems[groupdepth] + 1; n < curitem; n++) {
if (items[n].type == Normal) {
break;
}
if (items[n].type == Highlight) {
group_end_userhl = items[n].minwid;
}
}
if (n == curitem && group_start_userhl == group_end_userhl) {
out_p = t;
group_len = 0;
for (n = groupitems[groupdepth] + 1; n < curitem; n++) {
if (items[n].type == Highlight) {
items[n].type = Empty;
}
}
}
}
if (group_len > items[groupitems[groupdepth]].maxwid) {
long n;
if (has_mbyte) {
n = 0;
while (group_len >= items[groupitems[groupdepth]].maxwid) {
group_len -= ptr2cells(t + n);
n += (*mb_ptr2len)(t + n);
}
} else {
n = (long)(out_p - t) - items[groupitems[groupdepth]].maxwid + 1;
}
*t = '<';
memmove(t + 1, t + n, (size_t)(out_p - (t + n)));
out_p = out_p - n + 1;
while (++group_len < items[groupitems[groupdepth]].minwid) {
*out_p++ = fillchar;
}
for (int idx = groupitems[groupdepth] + 1; idx < curitem; idx++) {
items[idx].start -= n;
if (items[idx].start < t) {
items[idx].start = t;
}
}
} else if (abs(items[groupitems[groupdepth]].minwid) > group_len) {
long min_group_width = items[groupitems[groupdepth]].minwid;
if (min_group_width < 0) {
min_group_width = 0 - min_group_width;
while (group_len++ < min_group_width && out_p < out_end_p)
*out_p++ = fillchar;
} else {
memmove(t + min_group_width - group_len, t, (size_t)(out_p - t));
group_len = min_group_width - group_len;
if (out_p + group_len >= (out_end_p + 1)) {
group_len = (long)(out_end_p - out_p);
}
out_p += group_len;
for (int n = groupitems[groupdepth] + 1; n < curitem; n++) {
items[n].start += group_len;
}
for (; group_len > 0; group_len--) {
*t++ = fillchar;
}
}
}
continue;
}
int minwid = 0;
int maxwid = 9999;
bool left_align = false;
bool zeropad = (*fmt_p == '0');
if (zeropad) {
fmt_p++;
}
if (*fmt_p == '-') {
fmt_p++;
left_align = true;
}
if (ascii_isdigit(*fmt_p)) {
minwid = getdigits_int(&fmt_p, false, 0);
}
if (*fmt_p == STL_USER_HL) {
items[curitem].type = Highlight;
items[curitem].start = out_p;
items[curitem].minwid = minwid > 9 ? 1 : minwid;
fmt_p++;
curitem++;
continue;
}
if (*fmt_p == STL_TABPAGENR || *fmt_p == STL_TABCLOSENR) {
if (*fmt_p == STL_TABCLOSENR) {
if (minwid == 0) {
for (long n = curitem - 1; n >= 0; n--) {
if (items[n].type == TabPage && items[n].minwid >= 0) {
minwid = items[n].minwid;
break;
}
}
} else {
minwid = -minwid;
}
}
items[curitem].type = TabPage;
items[curitem].start = out_p;
items[curitem].minwid = minwid;
fmt_p++;
curitem++;
continue;
}
if (*fmt_p == STL_CLICK_FUNC) {
fmt_p++;
char *t = (char *) fmt_p;
while (*fmt_p != STL_CLICK_FUNC && *fmt_p) {
fmt_p++;
}
if (*fmt_p != STL_CLICK_FUNC) {
break;
}
items[curitem].type = ClickFunc;
items[curitem].start = out_p;
items[curitem].cmd = xmemdupz(t, (size_t)(((char *)fmt_p - t)));
items[curitem].minwid = minwid;
fmt_p++;
curitem++;
continue;
}
if (*fmt_p == '.') {
fmt_p++;
if (ascii_isdigit(*fmt_p)) {
maxwid = getdigits_int(&fmt_p, false, 50);
}
}
minwid = (minwid > 50 ? 50 : minwid) * (left_align ? -1 : 1);
if (*fmt_p == '(') {
groupitems[groupdepth++] = curitem;
items[curitem].type = Group;
items[curitem].start = out_p;
items[curitem].minwid = minwid;
items[curitem].maxwid = maxwid;
fmt_p++;
curitem++;
continue;
}
if (vim_strchr(STL_ALL, *fmt_p) == NULL) {
fmt_p++;
continue;
}
char_u opt = *fmt_p++;
NumberBase base = kNumBaseDecimal;
bool itemisflag = false;
bool fillable = true;
long num = -1;
char_u *str = NULL;
switch (opt) {
case STL_FILEPATH:
case STL_FULLPATH:
case STL_FILENAME:
{
fillable = false;
if (buf_spname(wp->w_buffer) != NULL) {
STRLCPY(NameBuff, buf_spname(wp->w_buffer), MAXPATHL);
} else {
char_u *t = (opt == STL_FULLPATH) ? wp->w_buffer->b_ffname
: wp->w_buffer->b_fname;
home_replace(wp->w_buffer, t, NameBuff, MAXPATHL, true);
}
trans_characters(NameBuff, MAXPATHL);
if (opt != STL_FILENAME) {
str = NameBuff;
} else {
str = path_tail(NameBuff);
}
break;
}
case STL_VIM_EXPR: 
{
itemisflag = true;
char_u *t = out_p;
while (*fmt_p != '}' && *fmt_p != NUL && out_p < out_end_p)
*out_p++ = *fmt_p++;
if (*fmt_p != '}') { 
break;
}
fmt_p++;
*out_p = 0;
out_p = t;
vim_snprintf((char *)tmp, sizeof(tmp), "%d", curbuf->b_fnum);
set_internal_string_var((char_u *)"g:actual_curbuf", tmp);
buf_T *const save_curbuf = curbuf;
win_T *const save_curwin = curwin;
const int save_VIsual_active = VIsual_active;
curwin = wp;
curbuf = wp->w_buffer;
if (curwin != save_curwin) {
VIsual_active = false;
}
str = eval_to_string_safe(out_p, &t, use_sandbox);
curwin = save_curwin;
curbuf = save_curbuf;
VIsual_active = save_VIsual_active;
do_unlet(S_LEN("g:actual_curbuf"), true);
if (str != NULL && *str != 0) {
if (*skipdigits(str) == NUL) {
num = atoi((char *)str);
XFREE_CLEAR(str);
itemisflag = false;
}
}
break;
}
case STL_LINE:
num = (wp->w_buffer->b_ml.ml_flags & ML_EMPTY)
? 0L : (long)(wp->w_cursor.lnum);
break;
case STL_NUMLINES:
num = wp->w_buffer->b_ml.ml_line_count;
break;
case STL_COLUMN:
num = !(State & INSERT) && empty_line
? 0 : (int)wp->w_cursor.col + 1;
break;
case STL_VIRTCOL:
case STL_VIRTCOL_ALT:
{
colnr_T virtcol = wp->w_virtcol;
if (wp->w_p_list && wp->w_p_lcs_chars.tab1 == NUL) {
wp->w_p_list = false;
getvcol(wp, &wp->w_cursor, NULL, &virtcol, NULL);
wp->w_p_list = true;
}
virtcol++;
if (opt == STL_VIRTCOL_ALT
&& (virtcol == (colnr_T)(!(State & INSERT) && empty_line
? 0 : (int)wp->w_cursor.col + 1)))
break;
num = (long)virtcol;
break;
}
case STL_PERCENTAGE:
num = (int)(((long)wp->w_cursor.lnum * 100L) /
(long)wp->w_buffer->b_ml.ml_line_count);
break;
case STL_ALTPERCENT:
get_rel_pos(wp, tmp, TMPLEN);
str = tmp;
break;
case STL_ARGLISTSTAT:
fillable = false;
tmp[0] = 0;
if (append_arg_number(wp, tmp, (int)sizeof(tmp), false)) {
str = tmp;
}
break;
case STL_KEYMAP:
fillable = false;
if (get_keymap_str(wp, (char_u *)"<%s>", tmp, TMPLEN)) {
str = tmp;
}
break;
case STL_PAGENUM:
num = printer_page_num;
break;
case STL_BUFNO:
num = wp->w_buffer->b_fnum;
break;
case STL_OFFSET_X:
base = kNumBaseHexadecimal;
FALLTHROUGH;
case STL_OFFSET:
{
long l = ml_find_line_or_offset(wp->w_buffer, wp->w_cursor.lnum, NULL,
false);
num = (wp->w_buffer->b_ml.ml_flags & ML_EMPTY) || l < 0 ?
0L : l + 1 + (!(State & INSERT) && empty_line ?
0 : (int)wp->w_cursor.col);
break;
}
case STL_BYTEVAL_X:
base = kNumBaseHexadecimal;
FALLTHROUGH;
case STL_BYTEVAL:
num = byteval;
if (num == NL) {
num = 0;
} else if (num == CAR && get_fileformat(wp->w_buffer) == EOL_MAC) {
num = NL;
}
break;
case STL_ROFLAG:
case STL_ROFLAG_ALT:
itemisflag = true;
if (wp->w_buffer->b_p_ro) {
str = (char_u *)((opt == STL_ROFLAG_ALT) ? ",RO" : _("[RO]"));
}
break;
case STL_HELPFLAG:
case STL_HELPFLAG_ALT:
itemisflag = true;
if (wp->w_buffer->b_help)
str = (char_u *)((opt == STL_HELPFLAG_ALT) ? ",HLP"
: _("[Help]"));
break;
case STL_FILETYPE:
if (*wp->w_buffer->b_p_ft != NUL
&& STRLEN(wp->w_buffer->b_p_ft) < TMPLEN - 3) {
vim_snprintf((char *)tmp, sizeof(tmp), "[%s]",
wp->w_buffer->b_p_ft);
str = tmp;
}
break;
case STL_FILETYPE_ALT:
{
itemisflag = true;
if (*wp->w_buffer->b_p_ft != NUL
&& STRLEN(wp->w_buffer->b_p_ft) < TMPLEN - 2) {
vim_snprintf((char *)tmp, sizeof(tmp), ",%s",
wp->w_buffer->b_p_ft);
for (char_u *t = tmp; *t != 0; t++) {
*t = (char_u)TOUPPER_LOC(*t);
}
str = tmp;
}
break;
}
case STL_PREVIEWFLAG:
case STL_PREVIEWFLAG_ALT:
itemisflag = true;
if (wp->w_p_pvw)
str = (char_u *)((opt == STL_PREVIEWFLAG_ALT) ? ",PRV"
: _("[Preview]"));
break;
case STL_QUICKFIX:
if (bt_quickfix(wp->w_buffer))
str = (char_u *)(wp->w_llist_ref
? _(msg_loclist)
: _(msg_qflist));
break;
case STL_MODIFIED:
case STL_MODIFIED_ALT:
itemisflag = true;
switch ((opt == STL_MODIFIED_ALT)
+ bufIsChanged(wp->w_buffer) * 2
+ (!MODIFIABLE(wp->w_buffer)) * 4) {
case 2: str = (char_u *)"[+]"; break;
case 3: str = (char_u *)",+"; break;
case 4: str = (char_u *)"[-]"; break;
case 5: str = (char_u *)",-"; break;
case 6: str = (char_u *)"[+-]"; break;
case 7: str = (char_u *)",+-"; break;
}
break;
case STL_HIGHLIGHT:
{
char_u *t = fmt_p;
while (*fmt_p != '#' && *fmt_p != NUL) {
fmt_p++;
}
if (*fmt_p == '#') {
items[curitem].type = Highlight;
items[curitem].start = out_p;
items[curitem].minwid = -syn_namen2id(t, (int)(fmt_p - t));
curitem++;
fmt_p++;
}
continue;
}
}
items[curitem].start = out_p;
items[curitem].type = Normal;
if (str != NULL && *str) {
char_u *t = str;
if (itemisflag) {
if ((t[0] && t[1])
&& ((!prevchar_isitem && *t == ',')
|| (prevchar_isflag && *t == ' ')))
t++;
prevchar_isflag = true;
}
long l = vim_strsize(t);
if (l > 0) {
prevchar_isitem = true;
}
if (l > maxwid) {
while (l >= maxwid)
if (has_mbyte) {
l -= ptr2cells(t);
t += (*mb_ptr2len)(t);
} else {
l -= byte2cells(*t++);
}
if (out_p >= out_end_p) {
break;
}
*out_p++ = '<';
}
if (minwid > 0) {
for (; l < minwid && out_p < out_end_p; l++) {
if (l + 1 == minwid && fillchar == '-' && ascii_isdigit(*t)) {
*out_p++ = ' ';
} else {
*out_p++ = fillchar;
}
}
minwid = 0;
} else {
minwid *= -1;
}
while (*t && out_p < out_end_p) {
*out_p++ = *t++;
if (fillable && out_p[-1] == ' '
&& (!ascii_isdigit(*t) || fillchar != '-'))
out_p[-1] = fillchar;
}
for (; l < minwid && out_p < out_end_p; l++) {
*out_p++ = fillchar;
}
} else if (num >= 0) {
if (out_p + 20 > out_end_p) {
break; 
}
prevchar_isitem = true;
char_u nstr[20];
char_u *t = nstr;
if (opt == STL_VIRTCOL_ALT) {
*t++ = '-';
minwid--;
}
*t++ = '%';
if (zeropad) {
*t++ = '0';
}
*t++ = '*';
*t++ = (char_u)(base == kNumBaseHexadecimal ? 'X' : 'd');
*t = 0;
long num_chars = 1;
for (long n = num; n >= (int) base; n /= (int) base) {
num_chars++;
}
if (opt == STL_VIRTCOL_ALT) {
num_chars++;
}
assert(out_end_p >= out_p);
size_t remaining_buf_len = (size_t)(out_end_p - out_p) + 1;
if (num_chars > maxwid) {
num_chars += 2;
long n = num_chars - maxwid;
while (num_chars-- > maxwid) {
num /= (long)base;
}
*t++ = '>';
*t++ = '%';
*t = t[-3];
*++t = 0;
vim_snprintf((char *)out_p, remaining_buf_len, (char *)nstr,
0, num, n);
} else {
vim_snprintf((char *)out_p, remaining_buf_len, (char *)nstr,
minwid, num);
}
out_p += STRLEN(out_p);
} else {
items[curitem].type = Empty;
}
if (opt == STL_VIM_EXPR) {
xfree(str);
}
if (num >= 0 || (!itemisflag && str && *str)) {
prevchar_isflag = false; 
}
curitem++;
}
*out_p = NUL;
int itemcnt = curitem;
if (usefmt != fmt) {
xfree(usefmt);
}
int width = vim_strsize(out);
if (maxwidth > 0 && width > maxwidth) {
int item_idx = 0;
char_u *trunc_p;
if (itemcnt == 0) {
trunc_p = out;
} else {
trunc_p = items[0].start;
item_idx = 0;
for (int i = 0; i < itemcnt; i++) {
if (items[i].type == Trunc) {
trunc_p = items[i].start;
item_idx = i;
break;
}
}
}
if (width - vim_strsize(trunc_p) >= maxwidth) {
if (has_mbyte) {
trunc_p = out;
width = 0;
for (;; ) {
width += ptr2cells(trunc_p);
if (width >= maxwidth) {
break;
}
trunc_p += (*mb_ptr2len)(trunc_p);
}
} else {
trunc_p = out + maxwidth - 1;
}
for (int i = 0; i < itemcnt; i++) {
if (items[i].start > trunc_p) {
itemcnt = i;
break;
}
}
*trunc_p++ = '>';
*trunc_p = 0;
} else {
long trunc_len;
if (has_mbyte) {
trunc_len = 0;
while (width >= maxwidth) {
width -= ptr2cells(trunc_p + trunc_len);
trunc_len += (*mb_ptr2len)(trunc_p + trunc_len);
}
} else {
trunc_len = (width - maxwidth) + 1;
}
char_u *trunc_end_p = trunc_p + trunc_len;
STRMOVE(trunc_p + 1, trunc_end_p);
*trunc_p = '<';
if (width + 1 < maxwidth) {
trunc_p = trunc_p + STRLEN(trunc_p);
}
while (++width < maxwidth) {
*trunc_p++ = fillchar;
*trunc_p = NUL;
}
long item_offset = trunc_len - 1;
for (int i = item_idx; i < itemcnt; i++) {
if (items[i].start >= trunc_end_p) {
items[i].start -= item_offset;
} else {
items[i].start = trunc_p;
}
}
}
width = maxwidth;
} else if (width < maxwidth
&& STRLEN(out) + (size_t)(maxwidth - width) + 1 < outlen) {
int num_separators = 0;
for (int i = 0; i < itemcnt; i++) {
if (items[i].type == Separate) {
num_separators++;
}
}
if (num_separators) {
int separator_locations[STL_MAX_ITEM];
int index = 0;
for (int i = 0; i < itemcnt; i++) {
if (items[i].type == Separate) {
separator_locations[index] = i;
index++;
}
}
int standard_spaces = (maxwidth - width) / num_separators;
int final_spaces = (maxwidth - width) -
standard_spaces * (num_separators - 1);
for (int i = 0; i < num_separators; i++) {
int dislocation = (i == (num_separators - 1))
? final_spaces : standard_spaces;
char_u *seploc = items[separator_locations[i]].start + dislocation;
STRMOVE(seploc, items[separator_locations[i]].start);
for (char_u *s = items[separator_locations[i]].start; s < seploc; s++) {
*s = fillchar;
}
for (int item_idx = separator_locations[i] + 1;
item_idx < itemcnt;
item_idx++) {
items[item_idx].start += dislocation;
}
}
width = maxwidth;
}
}
if (hltab != NULL) {
struct stl_hlrec *sp = hltab;
for (long l = 0; l < itemcnt; l++) {
if (items[l].type == Highlight) {
sp->start = items[l].start;
sp->userhl = items[l].minwid;
sp++;
}
}
sp->start = NULL;
sp->userhl = 0;
}
if (tabtab != NULL) {
StlClickRecord *cur_tab_rec = tabtab;
for (long l = 0; l < itemcnt; l++) {
if (items[l].type == TabPage) {
cur_tab_rec->start = (char *)items[l].start;
if (items[l].minwid == 0) {
cur_tab_rec->def.type = kStlClickDisabled;
cur_tab_rec->def.tabnr = 0;
} else {
int tabnr = items[l].minwid;
if (items[l].minwid > 0) {
cur_tab_rec->def.type = kStlClickTabSwitch;
} else {
cur_tab_rec->def.type = kStlClickTabClose;
tabnr = -tabnr;
}
cur_tab_rec->def.tabnr = tabnr;
}
cur_tab_rec->def.func = NULL;
cur_tab_rec++;
} else if (items[l].type == ClickFunc) {
cur_tab_rec->start = (char *)items[l].start;
cur_tab_rec->def.type = kStlClickFuncRun;
cur_tab_rec->def.tabnr = items[l].minwid;
cur_tab_rec->def.func = items[l].cmd;
cur_tab_rec++;
}
}
cur_tab_rec->start = NULL;
cur_tab_rec->def.type = kStlClickDisabled;
cur_tab_rec->def.tabnr = 0;
cur_tab_rec->def.func = NULL;
}
if (updating_screen) {
must_redraw = save_must_redraw;
curwin->w_redr_type = save_redr_type;
}
return width;
}
void get_rel_pos(win_T *wp, char_u *buf, int buflen)
{
if (buflen < 3) {
return;
}
long above; 
long below; 
above = wp->w_topline - 1;
above += diff_check_fill(wp, wp->w_topline) - wp->w_topfill;
if (wp->w_topline == 1 && wp->w_topfill >= 1) {
above = 0;
}
below = wp->w_buffer->b_ml.ml_line_count - wp->w_botline + 1;
if (below <= 0) {
STRLCPY(buf, (above == 0 ? _("All") : _("Bot")), buflen);
} else if (above <= 0) {
STRLCPY(buf, _("Top"), buflen);
} else {
vim_snprintf((char *)buf, (size_t)buflen, "%2d%%", above > 1000000L
? (int)(above / ((above + below) / 100L))
: (int)(above * 100L / (above + below)));
}
}
static bool append_arg_number(win_T *wp, char_u *buf, int buflen, bool add_file)
FUNC_ATTR_NONNULL_ALL
{
if (ARGCOUNT <= 1) {
return false;
}
char_u *p = buf + STRLEN(buf); 
if (p - buf + 35 >= buflen) {
return false;
}
*p++ = ' ';
*p++ = '(';
if (add_file) {
STRCPY(p, "file ");
p += 5;
}
vim_snprintf((char *)p, (size_t)(buflen - (p - buf)),
wp->w_arg_idx_invalid
? "(%d) of %d)"
: "%d of %d)", wp->w_arg_idx + 1, ARGCOUNT);
return true;
}
void fname_expand(buf_T *buf, char_u **ffname, char_u **sfname)
{
if (*ffname == NULL) { 
return;
}
if (*sfname == NULL) { 
*sfname = *ffname;
}
*ffname = (char_u *)fix_fname((char *)(*ffname)); 
#if defined(WIN32)
if (!buf->b_p_bin) {
char *rfname = os_resolve_shortcut((const char *)(*ffname));
if (rfname != NULL) {
xfree(*ffname);
*ffname = (char_u *)rfname;
*sfname = (char_u *)rfname;
}
}
#endif
}
char_u *alist_name(aentry_T *aep)
{
buf_T *bp;
bp = buflist_findnr(aep->ae_fnum);
if (bp == NULL || bp->b_fname == NULL) {
return aep->ae_fname;
}
return bp->b_fname;
}
void
do_arg_all(
int count,
int forceit, 
int keep_tabs 
)
{
int i;
char_u *opened; 
int opened_len; 
int use_firstwin = false; 
int split_ret = OK;
bool p_ea_save;
alist_T *alist; 
buf_T *buf;
tabpage_T *tpnext;
int had_tab = cmdmod.tab;
win_T *old_curwin, *last_curwin;
tabpage_T *old_curtab, *last_curtab;
win_T *new_curwin = NULL;
tabpage_T *new_curtab = NULL;
assert(firstwin != NULL); 
if (ARGCOUNT <= 0) {
return;
}
setpcmark();
opened_len = ARGCOUNT;
opened = xcalloc((size_t)opened_len, 1);
alist = curwin->w_alist;
alist->al_refcount++;
old_curwin = curwin;
old_curtab = curtab;
if (had_tab > 0) {
goto_tabpage_tp(first_tabpage, true, true);
}
for (;; ) {
win_T *wpnext = NULL;
tpnext = curtab->tp_next;
for (win_T *wp = firstwin; wp != NULL; wp = wpnext) {
wpnext = wp->w_next;
buf = wp->w_buffer;
if (buf->b_ffname == NULL
|| (!keep_tabs && (buf->b_nwindows > 1 || wp->w_width != Columns))) {
i = opened_len;
} else {
for (i = 0; i < opened_len; i++) {
if (i < alist->al_ga.ga_len
&& (AARGLIST(alist)[i].ae_fnum == buf->b_fnum
|| path_full_compare(alist_name(&AARGLIST(alist)[i]),
buf->b_ffname,
true, true) & kEqualFiles)) {
int weight = 1;
if (old_curtab == curtab) {
weight++;
if (old_curwin == wp) {
weight++;
}
}
if (weight > (int)opened[i]) {
opened[i] = (char_u)weight;
if (i == 0) {
if (new_curwin != NULL) {
new_curwin->w_arg_idx = opened_len;
}
new_curwin = wp;
new_curtab = curtab;
}
} else if (keep_tabs) {
i = opened_len;
}
if (wp->w_alist != alist) {
alist_unlink(wp->w_alist);
wp->w_alist = alist;
wp->w_alist->al_refcount++;
}
break;
}
}
}
wp->w_arg_idx = i;
if (i == opened_len && !keep_tabs) { 
if (buf_hide(buf) || forceit || buf->b_nwindows > 1
|| !bufIsChanged(buf)) {
if (!buf_hide(buf) && buf->b_nwindows <= 1 && bufIsChanged(buf)) {
bufref_T bufref;
set_bufref(&bufref, buf);
(void)autowrite(buf, false);
if (!win_valid(wp) || !bufref_valid(&bufref)) {
wpnext = firstwin; 
continue;
}
}
if (ONE_WINDOW
&& (first_tabpage->tp_next == NULL || !had_tab)) {
use_firstwin = true;
} else {
win_close(wp, !buf_hide(buf) && !bufIsChanged(buf));
if (!win_valid(wpnext)) {
wpnext = firstwin;
}
}
}
}
}
if (had_tab == 0 || tpnext == NULL) {
break;
}
if (!valid_tabpage(tpnext)) {
tpnext = first_tabpage; 
}
goto_tabpage_tp(tpnext, true, true);
}
if (count > opened_len || count <= 0) {
count = opened_len;
}
autocmd_no_enter++;
autocmd_no_leave++;
last_curwin = curwin;
last_curtab = curtab;
win_enter(lastwin, false);
if (keep_tabs && BUFEMPTY() && curbuf->b_nwindows == 1
&& curbuf->b_ffname == NULL && !curbuf->b_changed) {
use_firstwin = true;
}
for (i = 0; i < count && i < opened_len && !got_int; i++) {
if (alist == &global_alist && i == global_alist.al_ga.ga_len - 1) {
arg_had_last = true;
}
if (opened[i] > 0) {
if (curwin->w_arg_idx != i) {
FOR_ALL_WINDOWS_IN_TAB(wp, curtab) {
if (wp->w_arg_idx == i) {
if (keep_tabs) {
new_curwin = wp;
new_curtab = curtab;
} else {
win_move_after(wp, curwin);
}
break;
}
}
}
} else if (split_ret == OK) {
if (!use_firstwin) { 
p_ea_save = p_ea;
p_ea = true; 
split_ret = win_split(0, WSP_ROOM | WSP_BELOW);
p_ea = p_ea_save;
if (split_ret == FAIL) {
continue;
}
} else { 
autocmd_no_leave--;
}
curwin->w_arg_idx = i;
if (i == 0) {
new_curwin = curwin;
new_curtab = curtab;
}
(void)do_ecmd(0, alist_name(&AARGLIST(alist)[i]), NULL, NULL, ECMD_ONE,
((buf_hide(curwin->w_buffer)
|| bufIsChanged(curwin->w_buffer))
? ECMD_HIDE : 0) + ECMD_OLDBUF,
curwin);
if (use_firstwin) {
autocmd_no_leave++;
}
use_firstwin = false;
}
os_breakcheck();
if (had_tab > 0 && tabpage_index(NULL) <= p_tpm) {
cmdmod.tab = 9999;
}
}
alist_unlink(alist);
autocmd_no_enter--;
if (last_curtab != new_curtab) {
if (valid_tabpage(last_curtab)) {
goto_tabpage_tp(last_curtab, true, true);
}
if (win_valid(last_curwin)) {
win_enter(last_curwin, false);
}
}
if (valid_tabpage(new_curtab)) {
goto_tabpage_tp(new_curtab, true, true);
}
if (win_valid(new_curwin)) {
win_enter(new_curwin, false);
}
autocmd_no_leave--;
xfree(opened);
}
int bt_prompt(buf_T *buf)
{
return buf != NULL && buf->b_p_bt[0] == 'p';
}
void ex_buffer_all(exarg_T *eap)
{
buf_T *buf;
win_T *wp, *wpnext;
int split_ret = OK;
bool p_ea_save;
int open_wins = 0;
int r;
long count; 
int all; 
int had_tab = cmdmod.tab;
tabpage_T *tpnext;
if (eap->addr_count == 0) { 
count = 9999;
} else {
count = eap->line2; 
}
if (eap->cmdidx == CMD_unhide || eap->cmdidx == CMD_sunhide) {
all = false;
} else {
all = true;
}
setpcmark();
if (had_tab > 0) {
goto_tabpage_tp(first_tabpage, true, true);
}
for (;; ) {
tpnext = curtab->tp_next;
for (wp = firstwin; wp != NULL; wp = wpnext) {
wpnext = wp->w_next;
if ((wp->w_buffer->b_nwindows > 1
|| ((cmdmod.split & WSP_VERT)
? wp->w_height + wp->w_status_height < Rows - p_ch
- tabline_height()
: wp->w_width != Columns)
|| (had_tab > 0 && wp != firstwin))
&& !ONE_WINDOW
&& !(wp->w_closing || wp->w_buffer->b_locked > 0)
) {
win_close(wp, false);
wpnext = firstwin; 
tpnext = first_tabpage; 
open_wins = 0;
} else {
open_wins++;
}
}
if (had_tab == 0 || tpnext == NULL) {
break;
}
goto_tabpage_tp(tpnext, true, true);
}
autocmd_no_enter++;
win_enter(lastwin, false);
autocmd_no_leave++;
for (buf = firstbuf; buf != NULL && open_wins < count; buf = buf->b_next) {
if ((!all && buf->b_ml.ml_mfp == NULL) || !buf->b_p_bl) {
continue;
}
if (had_tab != 0) {
if (buf->b_nwindows > 0) {
wp = lastwin; 
} else {
wp = NULL;
}
} else {
for (wp = firstwin; wp != NULL; wp = wp->w_next) {
if (wp->w_buffer == buf) {
break;
}
}
if (wp != NULL) {
win_move_after(wp, curwin);
}
}
if (wp == NULL && split_ret == OK) {
bufref_T bufref;
set_bufref(&bufref, buf);
p_ea_save = p_ea;
p_ea = true; 
split_ret = win_split(0, WSP_ROOM | WSP_BELOW);
open_wins++;
p_ea = p_ea_save;
if (split_ret == FAIL) {
continue;
}
swap_exists_action = SEA_DIALOG;
set_curbuf(buf, DOBUF_GOTO);
if (!bufref_valid(&bufref)) {
swap_exists_action = SEA_NONE;
break;
}
if (swap_exists_action == SEA_QUIT) {
cleanup_T cs;
enter_cleanup(&cs);
win_close(curwin, true);
open_wins--;
swap_exists_action = SEA_NONE;
swap_exists_did_quit = true;
leave_cleanup(&cs);
} else
handle_swap_exists(NULL);
}
os_breakcheck();
if (got_int) {
(void)vgetc(); 
break;
}
if (aborting()) {
break;
}
if (had_tab > 0 && tabpage_index(NULL) <= p_tpm) {
cmdmod.tab = 9999;
}
}
autocmd_no_enter--;
win_enter(firstwin, false); 
autocmd_no_leave--;
for (wp = lastwin; open_wins > count; ) {
r = (buf_hide(wp->w_buffer) || !bufIsChanged(wp->w_buffer)
|| autowrite(wp->w_buffer, false) == OK);
if (!win_valid(wp)) {
wp = lastwin;
} else if (r) {
win_close(wp, !buf_hide(wp->w_buffer));
open_wins--;
wp = lastwin;
} else {
wp = wp->w_prev;
if (wp == NULL) {
break;
}
}
}
}
void do_modelines(int flags)
{
linenr_T lnum;
int nmlines;
static int entered = 0;
if (!curbuf->b_p_ml || (nmlines = (int)p_mls) == 0) {
return;
}
if (entered) {
return;
}
entered++;
for (lnum = 1; lnum <= curbuf->b_ml.ml_line_count && lnum <= nmlines;
lnum++) {
if (chk_modeline(lnum, flags) == FAIL) {
nmlines = 0;
}
}
for (lnum = curbuf->b_ml.ml_line_count; lnum > 0 && lnum > nmlines
&& lnum > curbuf->b_ml.ml_line_count - nmlines; lnum--) {
if (chk_modeline(lnum, flags) == FAIL) {
nmlines = 0;
}
}
entered--;
}
static int
chk_modeline(
linenr_T lnum,
int flags 
)
{
char_u *s;
char_u *e;
char_u *linecopy; 
int prev;
intmax_t vers;
int end;
int retval = OK;
char_u *save_sourcing_name;
linenr_T save_sourcing_lnum;
prev = -1;
for (s = ml_get(lnum); *s != NUL; s++) {
if (prev == -1 || ascii_isspace(prev)) {
if ((prev != -1 && STRNCMP(s, "ex:", (size_t)3) == 0)
|| STRNCMP(s, "vi:", (size_t)3) == 0)
break;
if ((s[0] == 'v' || s[0] == 'V') && s[1] == 'i' && s[2] == 'm') {
if (s[3] == '<' || s[3] == '=' || s[3] == '>') {
e = s + 4;
} else {
e = s + 3;
}
if (!try_getdigits(&e, &vers)) {
continue;
}
if (*e == ':'
&& (s[0] != 'V'
|| STRNCMP(skipwhite(e + 1), "set", 3) == 0)
&& (s[3] == ':'
|| (VIM_VERSION_100 >= vers && isdigit(s[3]))
|| (VIM_VERSION_100 < vers && s[3] == '<')
|| (VIM_VERSION_100 > vers && s[3] == '>')
|| (VIM_VERSION_100 == vers && s[3] == '='))) {
break;
}
}
}
prev = *s;
}
if (!*s) {
return retval;
}
do { 
s++;
} while (s[-1] != ':');
s = linecopy = vim_strsave(s); 
save_sourcing_lnum = sourcing_lnum;
save_sourcing_name = sourcing_name;
sourcing_lnum = lnum; 
sourcing_name = (char_u *)"modelines";
end = false;
while (end == false) {
s = skipwhite(s);
if (*s == NUL) {
break;
}
for (e = s; *e != ':' && *e != NUL; e++) {
if (e[0] == '\\' && e[1] == ':') {
STRMOVE(e, e + 1);
}
}
if (*e == NUL) {
end = true;
}
if (STRNCMP(s, "set ", (size_t)4) == 0
|| STRNCMP(s, "se ", (size_t)3) == 0) {
if (*e != ':') { 
break;
}
end = true;
s = vim_strchr(s, ' ') + 1;
}
*e = NUL; 
if (*s != NUL) { 
const int secure_save = secure;
const sctx_T save_current_sctx = current_sctx;
current_sctx.sc_sid = SID_MODELINE;
current_sctx.sc_seq = 0;
current_sctx.sc_lnum = 0;
secure = 1;
retval = do_set(s, OPT_MODELINE | OPT_LOCAL | flags);
secure = secure_save;
current_sctx = save_current_sctx;
if (retval == FAIL) { 
break;
}
}
s = e + 1; 
}
sourcing_lnum = save_sourcing_lnum;
sourcing_name = save_sourcing_name;
xfree(linecopy);
return retval;
}
bool bt_help(const buf_T *const buf)
FUNC_ATTR_PURE FUNC_ATTR_WARN_UNUSED_RESULT
{
return buf != NULL && buf->b_help;
}
bool bt_normal(const buf_T *const buf)
FUNC_ATTR_PURE FUNC_ATTR_WARN_UNUSED_RESULT
{
return buf != NULL && buf->b_p_bt[0] == NUL;
}
bool bt_quickfix(const buf_T *const buf)
FUNC_ATTR_PURE FUNC_ATTR_WARN_UNUSED_RESULT
{
return buf != NULL && buf->b_p_bt[0] == 'q';
}
bool bt_terminal(const buf_T *const buf)
FUNC_ATTR_PURE FUNC_ATTR_WARN_UNUSED_RESULT
{
return buf != NULL && buf->b_p_bt[0] == 't';
}
bool bt_nofile(const buf_T *const buf)
FUNC_ATTR_PURE FUNC_ATTR_WARN_UNUSED_RESULT
{
return buf != NULL && ((buf->b_p_bt[0] == 'n' && buf->b_p_bt[2] == 'f')
|| buf->b_p_bt[0] == 'a'
|| buf->terminal
|| buf->b_p_bt[0] == 'p');
}
bool bt_dontwrite(const buf_T *const buf)
FUNC_ATTR_PURE FUNC_ATTR_WARN_UNUSED_RESULT
{
return buf != NULL && (buf->b_p_bt[0] == 'n'
|| buf->terminal
|| buf->b_p_bt[0] == 'p');
}
bool bt_dontwrite_msg(const buf_T *const buf)
FUNC_ATTR_PURE FUNC_ATTR_WARN_UNUSED_RESULT
{
if (bt_dontwrite(buf)) {
EMSG(_("E382: Cannot write, 'buftype' option is set"));
return true;
}
return false;
}
bool buf_hide(const buf_T *const buf)
{
switch (buf->b_p_bh[0]) {
case 'u': 
case 'w': 
case 'd': return false; 
case 'h': return true; 
}
return p_hid || cmdmod.hide;
}
char_u *buf_spname(buf_T *buf)
{
if (bt_quickfix(buf)) {
win_T *win;
tabpage_T *tp;
if (find_win_for_buf(buf, &win, &tp) && win->w_llist_ref != NULL) {
return (char_u *)_(msg_loclist);
} else {
return (char_u *)_(msg_qflist);
}
}
if (bt_nofile(buf)) {
if (buf->b_fname != NULL) {
return buf->b_fname;
}
if (bt_prompt(buf)) {
return (char_u *)_("[Prompt]");
}
return (char_u *)_("[Scratch]");
}
if (buf->b_fname == NULL) {
return (char_u *)_("[No Name]");
}
return NULL;
}
bool find_win_for_buf(buf_T *buf, win_T **wp, tabpage_T **tp)
{
*wp = NULL;
*tp = NULL;
FOR_ALL_TAB_WINDOWS(tp2, wp2) {
if (wp2->w_buffer == buf) {
*tp = tp2;
*wp = wp2;
return true;
}
}
return false;
}
int buf_signcols(buf_T *buf)
{
if (buf->b_signcols_max == -1) {
signlist_T *sign; 
buf->b_signcols_max = 0;
int linesum = 0;
linenr_T curline = 0;
FOR_ALL_SIGNS_IN_BUF(buf, sign) {
if (sign->lnum > curline) {
if (linesum > buf->b_signcols_max) {
buf->b_signcols_max = linesum;
}
curline = sign->lnum;
linesum = 0;
}
linesum++;
}
if (linesum > buf->b_signcols_max) {
buf->b_signcols_max = linesum;
}
if (buf->b_signcols_max != buf->b_signcols) {
buf->b_signcols = buf->b_signcols_max;
redraw_buf_later(buf, NOT_VALID);
}
}
return buf->b_signcols;
}
void set_buflisted(int on)
{
if (on != curbuf->b_p_bl) {
curbuf->b_p_bl = on;
if (on) {
apply_autocmds(EVENT_BUFADD, NULL, NULL, false, curbuf);
} else {
apply_autocmds(EVENT_BUFDELETE, NULL, NULL, false, curbuf);
}
}
}
bool buf_contents_changed(buf_T *buf)
FUNC_ATTR_NONNULL_ALL
{
bool differ = true;
buf_T *newbuf = buflist_new(NULL, NULL, (linenr_T)1, BLN_DUMMY);
if (newbuf == NULL) {
return true;
}
exarg_T ea;
prep_exarg(&ea, buf);
aco_save_T aco;
aucmd_prepbuf(&aco, newbuf);
if (ml_open(curbuf) == OK
&& readfile(buf->b_ffname, buf->b_fname,
(linenr_T)0, (linenr_T)0, (linenr_T)MAXLNUM,
&ea, READ_NEW | READ_DUMMY) == OK) {
if (buf->b_ml.ml_line_count == curbuf->b_ml.ml_line_count) {
differ = false;
for (linenr_T lnum = 1; lnum <= curbuf->b_ml.ml_line_count; lnum++) {
if (STRCMP(ml_get_buf(buf, lnum, false), ml_get(lnum)) != 0) {
differ = true;
break;
}
}
}
}
xfree(ea.cmd);
aucmd_restbuf(&aco);
if (curbuf != newbuf) { 
wipe_buffer(newbuf, false);
}
return differ;
}
void
wipe_buffer(
buf_T *buf,
int aucmd 
)
{
if (!aucmd) {
block_autocmds();
}
close_buffer(NULL, buf, DOBUF_WIPE, false);
if (!aucmd) {
unblock_autocmds();
}
}
void buf_open_scratch(handle_T bufnr, char *bufname)
{
(void)do_ecmd((int)bufnr, NULL, NULL, NULL, ECMD_ONE, ECMD_HIDE, NULL);
(void)setfname(curbuf, (char_u *)bufname, NULL, true);
set_option_value("bh", 0L, "hide", OPT_LOCAL);
set_option_value("bt", 0L, "nofile", OPT_LOCAL);
set_option_value("swf", 0L, NULL, OPT_LOCAL);
RESET_BINDING(curwin);
}
