#include "nvim/assert.h"
#include "nvim/buffer.h"
#include "nvim/buffer_updates.h"
#include "nvim/change.h"
#include "nvim/charset.h"
#include "nvim/cursor.h"
#include "nvim/diff.h"
#include "nvim/edit.h"
#include "nvim/eval.h"
#include "nvim/fileio.h"
#include "nvim/fold.h"
#include "nvim/indent.h"
#include "nvim/indent_c.h"
#include "nvim/mark.h"
#include "nvim/extmark.h"
#include "nvim/memline.h"
#include "nvim/misc1.h"
#include "nvim/move.h"
#include "nvim/option.h"
#include "nvim/screen.h"
#include "nvim/search.h"
#include "nvim/state.h"
#include "nvim/ui.h"
#include "nvim/undo.h"
#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "change.c.generated.h"
#endif
void change_warning(int col)
{
static char *w_readonly = N_("W10: Warning: Changing a readonly file");
if (curbuf->b_did_warn == false
&& curbufIsChanged() == 0
&& !autocmd_busy
&& curbuf->b_p_ro) {
curbuf_lock++;
apply_autocmds(EVENT_FILECHANGEDRO, NULL, NULL, false, curbuf);
curbuf_lock--;
if (!curbuf->b_p_ro) {
return;
}
msg_start();
if (msg_row == Rows - 1) {
msg_col = col;
}
msg_source(HL_ATTR(HLF_W));
msg_ext_set_kind("wmsg");
MSG_PUTS_ATTR(_(w_readonly), HL_ATTR(HLF_W) | MSG_HIST);
set_vim_var_string(VV_WARNINGMSG, _(w_readonly), -1);
msg_clr_eos();
(void)msg_end();
if (msg_silent == 0 && !silent_mode && ui_active()) {
ui_flush();
os_delay(1000L, true); 
}
curbuf->b_did_warn = true;
redraw_cmdline = false; 
if (msg_row < Rows - 1) {
showmode();
}
}
}
void changed(void)
{
if (!curbuf->b_changed) {
int save_msg_scroll = msg_scroll;
change_warning(0);
if (curbuf->b_may_swap
&& !bt_dontwrite(curbuf)
) {
int save_need_wait_return = need_wait_return;
need_wait_return = false;
ml_open_file(curbuf);
if (need_wait_return && emsg_silent == 0) {
ui_flush();
os_delay(2000L, true);
wait_return(true);
msg_scroll = save_msg_scroll;
} else {
need_wait_return = save_need_wait_return;
}
}
changed_internal();
}
buf_inc_changedtick(curbuf);
highlight_match = false;
}
void changed_internal(void)
{
curbuf->b_changed = true;
ml_setflags(curbuf);
check_status(curbuf);
redraw_tabline = true;
need_maketitle = true; 
}
static void changed_common(linenr_T lnum, colnr_T col, linenr_T lnume,
long xtra)
{
int i;
int cols;
pos_T *p;
int add;
changed();
if (curwin->w_p_diff && diff_internal()) {
curtab->tp_diff_update = true;
}
if (!cmdmod.keepjumps) {
RESET_FMARK(&curbuf->b_last_change, ((pos_T) { lnum, col, 0 }), 0);
if (curbuf->b_new_change || curbuf->b_changelistlen == 0) {
if (curbuf->b_changelistlen == 0) {
add = true;
} else {
p = &curbuf->b_changelist[curbuf->b_changelistlen - 1].mark;
if (p->lnum != lnum) {
add = true;
} else {
cols = comp_textwidth(false);
if (cols == 0) {
cols = 79;
}
add = (p->col + cols < col || col + cols < p->col);
}
}
if (add) {
curbuf->b_new_change = false;
if (curbuf->b_changelistlen == JUMPLISTSIZE) {
curbuf->b_changelistlen = JUMPLISTSIZE - 1;
memmove(curbuf->b_changelist, curbuf->b_changelist + 1,
sizeof(curbuf->b_changelist[0]) * (JUMPLISTSIZE - 1));
FOR_ALL_TAB_WINDOWS(tp, wp) {
if (wp->w_buffer == curbuf && wp->w_changelistidx > 0) {
wp->w_changelistidx--;
}
}
}
FOR_ALL_TAB_WINDOWS(tp, wp) {
if (wp->w_buffer == curbuf
&& wp->w_changelistidx == curbuf->b_changelistlen) {
wp->w_changelistidx++;
}
}
curbuf->b_changelistlen++;
}
}
curbuf->b_changelist[curbuf->b_changelistlen - 1] =
curbuf->b_last_change;
curwin->w_changelistidx = curbuf->b_changelistlen;
}
FOR_ALL_TAB_WINDOWS(tp, wp) {
if (wp->w_buffer == curbuf) {
if (wp->w_redr_type < VALID) {
wp->w_redr_type = VALID;
}
foldUpdate(wp, lnum, lnume + xtra - 1);
bool folded = hasFoldingWin(wp, lnum, &lnum, NULL, false, NULL);
if (wp->w_cursor.lnum == lnum) {
wp->w_cline_folded = folded;
}
folded = hasFoldingWin(wp, lnume, NULL, &lnume, false, NULL);
if (wp->w_cursor.lnum == lnume) {
wp->w_cline_folded = folded;
}
if (wp->w_cursor.lnum <= lnum) {
i = find_wl_entry(wp, lnum);
if (i >= 0 && wp->w_cursor.lnum > wp->w_lines[i].wl_lnum) {
changed_line_abv_curs_win(wp);
}
}
if (wp->w_cursor.lnum > lnum) {
changed_line_abv_curs_win(wp);
} else if (wp->w_cursor.lnum == lnum && wp->w_cursor.col >= col) {
changed_cline_bef_curs_win(wp);
}
if (wp->w_botline >= lnum) {
approximate_botline_win(wp);
}
for (i = 0; i < wp->w_lines_valid; i++) {
if (wp->w_lines[i].wl_valid) {
if (wp->w_lines[i].wl_lnum >= lnum) {
if (wp->w_lines[i].wl_lnum < lnume) {
wp->w_lines[i].wl_valid = false;
} else if (xtra != 0) {
wp->w_lines[i].wl_lnum += xtra;
wp->w_lines[i].wl_lastlnum += xtra;
}
} else if (wp->w_lines[i].wl_lastlnum >= lnum) {
wp->w_lines[i].wl_valid = false;
}
}
}
if (hasAnyFolding(wp)) {
set_topline(wp, wp->w_topline);
}
if (wp->w_p_rnu
|| (wp->w_p_cul && lnum <= wp->w_last_cursorline)) {
redraw_win_later(wp, SOME_VALID);
}
}
}
if (must_redraw < VALID) {
must_redraw = VALID;
}
if (lnum <= curwin->w_cursor.lnum
&& lnume + (xtra < 0 ? -xtra : xtra) > curwin->w_cursor.lnum) {
curwin->w_last_cursormoved.lnum = 0;
}
}
static void changedOneline(buf_T *buf, linenr_T lnum)
{
if (buf->b_mod_set) {
if (lnum < buf->b_mod_top) {
buf->b_mod_top = lnum;
} else if (lnum >= buf->b_mod_bot) {
buf->b_mod_bot = lnum + 1;
}
} else {
buf->b_mod_set = true;
buf->b_mod_top = lnum;
buf->b_mod_bot = lnum + 1;
buf->b_mod_xlines = 0;
}
}
void changed_bytes(linenr_T lnum, colnr_T col)
{
changedOneline(curbuf, lnum);
changed_common(lnum, col, lnum + 1, 0L);
buf_updates_send_changes(curbuf, lnum, 1, 1, true);
if (curwin->w_p_diff) {
linenr_T wlnum;
FOR_ALL_WINDOWS_IN_TAB(wp, curtab) {
if (wp->w_p_diff && wp != curwin) {
redraw_win_later(wp, VALID);
wlnum = diff_lnum_win(lnum, wp);
if (wlnum > 0) {
changedOneline(wp->w_buffer, wlnum);
}
}
}
}
}
static void inserted_bytes(linenr_T lnum, colnr_T col, int old, int new)
{
if (curbuf_splice_pending == 0) {
extmark_splice(curbuf, (int)lnum-1, col, 0, old, 0, new, kExtmarkUndo);
}
changed_bytes(lnum, col);
}
void appended_lines(linenr_T lnum, long count)
{
changed_lines(lnum + 1, 0, lnum + 1, count, true);
}
void appended_lines_mark(linenr_T lnum, long count)
{
if (lnum + count < curbuf->b_ml.ml_line_count || curwin->w_p_diff) {
mark_adjust(lnum + 1, (linenr_T)MAXLNUM, count, 0L, kExtmarkUndo);
} else {
extmark_adjust(curbuf, lnum + 1, (linenr_T)MAXLNUM, count, 0L,
kExtmarkUndo);
}
changed_lines(lnum + 1, 0, lnum + 1, count, true);
}
void deleted_lines(linenr_T lnum, long count)
{
changed_lines(lnum, 0, lnum + count, -count, true);
}
void deleted_lines_mark(linenr_T lnum, long count)
{
mark_adjust(lnum, (linenr_T)(lnum + count - 1), (long)MAXLNUM, -count,
kExtmarkUndo);
changed_lines(lnum, 0, lnum + count, -count, true);
}
void changed_lines_buf(buf_T *buf, linenr_T lnum, linenr_T lnume, long xtra)
{
if (buf->b_mod_set) {
if (lnum < buf->b_mod_top) {
buf->b_mod_top = lnum;
}
if (lnum < buf->b_mod_bot) {
buf->b_mod_bot += xtra;
if (buf->b_mod_bot < lnum) {
buf->b_mod_bot = lnum;
}
}
if (lnume + xtra > buf->b_mod_bot) {
buf->b_mod_bot = lnume + xtra;
}
buf->b_mod_xlines += xtra;
} else {
buf->b_mod_set = true;
buf->b_mod_top = lnum;
buf->b_mod_bot = lnume + xtra;
buf->b_mod_xlines = xtra;
}
}
void
changed_lines(
linenr_T lnum, 
colnr_T col, 
linenr_T lnume, 
long xtra, 
bool do_buf_event 
)
{
changed_lines_buf(curbuf, lnum, lnume, xtra);
if (xtra == 0 && curwin->w_p_diff && !diff_internal()) {
linenr_T wlnum;
FOR_ALL_WINDOWS_IN_TAB(wp, curtab) {
if (wp->w_p_diff && wp != curwin) {
redraw_win_later(wp, VALID);
wlnum = diff_lnum_win(lnum, wp);
if (wlnum > 0) {
changed_lines_buf(wp->w_buffer, wlnum,
lnume - lnum + wlnum, 0L);
}
}
}
}
changed_common(lnum, col, lnume, xtra);
if (do_buf_event) {
int64_t num_added = (int64_t)(lnume + xtra - lnum);
int64_t num_removed = lnume - lnum;
buf_updates_send_changes(curbuf, lnum, num_added, num_removed, true);
}
}
void unchanged(buf_T *buf, int ff, bool always_inc_changedtick)
{
if (buf->b_changed || (ff && file_ff_differs(buf, false))) {
buf->b_changed = false;
ml_setflags(buf);
if (ff) {
save_file_ff(buf);
}
check_status(buf);
redraw_tabline = true;
need_maketitle = true; 
buf_inc_changedtick(buf);
} else if (always_inc_changedtick) {
buf_inc_changedtick(buf);
}
}
void ins_bytes(char_u *p)
{
ins_bytes_len(p, STRLEN(p));
}
void ins_bytes_len(char_u *p, size_t len)
{
size_t n;
for (size_t i = 0; i < len; i += n) {
if (enc_utf8) {
n = (size_t)utfc_ptr2len_len(p + i, (int)(len - i));
} else {
n = (size_t)(*mb_ptr2len)(p + i);
}
ins_char_bytes(p + i, n);
}
}
void ins_char(int c)
{
char_u buf[MB_MAXBYTES + 1];
size_t n = (size_t)utf_char2bytes(c, buf);
if (buf[0] == 0) {
buf[0] = '\n';
}
ins_char_bytes(buf, n);
}
void ins_char_bytes(char_u *buf, size_t charlen)
{
if (virtual_active() && curwin->w_cursor.coladd > 0) {
coladvance_force(getviscol());
}
size_t col = (size_t)curwin->w_cursor.col;
linenr_T lnum = curwin->w_cursor.lnum;
char_u *oldp = ml_get(lnum);
size_t linelen = STRLEN(oldp) + 1; 
size_t oldlen = 0; 
size_t newlen = charlen; 
if (State & REPLACE_FLAG) {
if (State & VREPLACE_FLAG) {
int old_list = curwin->w_p_list;
if (old_list && vim_strchr(p_cpo, CPO_LISTWM) == NULL) {
curwin->w_p_list = false;
}
colnr_T vcol;
getvcol(curwin, &curwin->w_cursor, NULL, &vcol, NULL);
colnr_T new_vcol = vcol + chartabsize(buf, vcol);
while (oldp[col + oldlen] != NUL && vcol < new_vcol) {
vcol += chartabsize(oldp + col + oldlen, vcol);
if (vcol > new_vcol && oldp[col + oldlen] == TAB) {
break;
}
oldlen += (size_t)(*mb_ptr2len)(oldp + col + oldlen);
if (vcol > new_vcol) {
newlen += (size_t)(vcol - new_vcol);
}
}
curwin->w_p_list = old_list;
} else if (oldp[col] != NUL) {
oldlen = (size_t)(*mb_ptr2len)(oldp + col);
}
replace_push(NUL);
for (size_t i = 0; i < oldlen; i++) {
i += (size_t)replace_push_mb(oldp + col + i) - 1;
}
}
char_u *newp = xmalloc((size_t)(linelen + newlen - oldlen));
if (col > 0) {
memmove(newp, oldp, (size_t)col);
}
char_u *p = newp + col;
if (linelen > col + oldlen) {
memmove(p + newlen, oldp + col + oldlen,
(size_t)(linelen - col - oldlen));
}
memmove(p, buf, charlen);
for (size_t i = charlen; i < newlen; i++) {
p[i] = ' ';
}
ml_replace(lnum, newp, false);
inserted_bytes(lnum, (colnr_T)col, (int)oldlen, (int)newlen);
if (p_sm && (State & INSERT)
&& msg_silent == 0
&& !ins_compl_active()
) {
showmatch(utf_ptr2char(buf));
}
if (!p_ri || (State & REPLACE_FLAG)) {
curwin->w_cursor.col += (int)charlen;
}
}
void ins_str(char_u *s)
{
char_u *oldp, *newp;
int newlen = (int)STRLEN(s);
int oldlen;
colnr_T col;
linenr_T lnum = curwin->w_cursor.lnum;
if (virtual_active() && curwin->w_cursor.coladd > 0) {
coladvance_force(getviscol());
}
col = curwin->w_cursor.col;
oldp = ml_get(lnum);
oldlen = (int)STRLEN(oldp);
newp = (char_u *)xmalloc((size_t)oldlen + (size_t)newlen + 1);
if (col > 0) {
memmove(newp, oldp, (size_t)col);
}
memmove(newp + col, s, (size_t)newlen);
int bytes = oldlen - col + 1;
assert(bytes >= 0);
memmove(newp + col + newlen, oldp + col, (size_t)bytes);
ml_replace(lnum, newp, false);
inserted_bytes(lnum, col, 0, newlen);
curwin->w_cursor.col += newlen;
}
int del_char(bool fixpos)
{
mb_adjust_cursor();
if (*get_cursor_pos_ptr() == NUL) {
return FAIL;
}
return del_chars(1L, fixpos);
}
int del_chars(long count, int fixpos)
{
int bytes = 0;
long i;
char_u *p;
int l;
p = get_cursor_pos_ptr();
for (i = 0; i < count && *p != NUL; i++) {
l = (*mb_ptr2len)(p);
bytes += l;
p += l;
}
return del_bytes(bytes, fixpos, true);
}
int del_bytes(colnr_T count, bool fixpos_arg, bool use_delcombine)
{
linenr_T lnum = curwin->w_cursor.lnum;
colnr_T col = curwin->w_cursor.col;
bool fixpos = fixpos_arg;
char_u *oldp = ml_get(lnum);
colnr_T oldlen = (colnr_T)STRLEN(oldp);
if (col >= oldlen) {
return FAIL;
}
if (count == 0) {
return OK;
}
if (count < 1) {
IEMSGN("E292: Invalid count for del_bytes(): %ld", count);
return FAIL;
}
if (p_deco && use_delcombine && enc_utf8
&& utfc_ptr2len(oldp + col) >= count) {
int cc[MAX_MCO];
int n;
(void)utfc_ptr2char(oldp + col, cc);
if (cc[0] != NUL) {
n = col;
do {
col = n;
count = utf_ptr2len(oldp + n);
n += count;
} while (UTF_COMPOSINGLIKE(oldp + col, oldp + n));
fixpos = false;
}
}
int movelen = oldlen - col - count + 1; 
if (movelen <= 1) {
if (col > 0 && fixpos && restart_edit == 0
&& (ve_flags & VE_ONEMORE) == 0
) {
curwin->w_cursor.col--;
curwin->w_cursor.coladd = 0;
curwin->w_cursor.col -= utf_head_off(oldp, oldp + curwin->w_cursor.col);
}
count = oldlen - col;
movelen = 1;
}
bool was_alloced = ml_line_alloced(); 
char_u *newp;
if (was_alloced) {
ml_add_deleted_len(curbuf->b_ml.ml_line_ptr, oldlen);
newp = oldp; 
} else { 
newp = xmalloc((size_t)(oldlen + 1 - count));
memmove(newp, oldp, (size_t)col);
}
memmove(newp + col, oldp + col + count, (size_t)movelen);
if (!was_alloced) {
ml_replace(lnum, newp, false);
}
inserted_bytes(lnum, col, count, 0);
return OK;
}
int copy_indent(int size, char_u *src)
{
char_u *p = NULL;
char_u *line = NULL;
char_u *s;
int todo;
int ind_len;
int line_len = 0;
int tab_pad;
int ind_done;
int round;
for (round = 1; round <= 2; round++) {
todo = size;
ind_len = 0;
ind_done = 0;
s = src;
while (todo > 0 && ascii_iswhite(*s)) {
if (*s == TAB) {
tab_pad = (int)curbuf->b_p_ts
- (ind_done % (int)curbuf->b_p_ts);
if (todo < tab_pad) {
break;
}
todo -= tab_pad;
ind_done += tab_pad;
} else {
todo--;
ind_done++;
}
ind_len++;
if (p != NULL) {
*p++ = *s;
}
s++;
}
tab_pad = (int)curbuf->b_p_ts - (ind_done % (int)curbuf->b_p_ts);
if ((todo >= tab_pad) && !curbuf->b_p_et) {
todo -= tab_pad;
ind_len++;
if (p != NULL) {
*p++ = TAB;
}
}
while (todo >= (int)curbuf->b_p_ts && !curbuf->b_p_et) {
todo -= (int)curbuf->b_p_ts;
ind_len++;
if (p != NULL) {
*p++ = TAB;
}
}
while (todo > 0) {
todo--;
ind_len++;
if (p != NULL) {
*p++ = ' ';
}
}
if (p == NULL) {
line_len = (int)STRLEN(get_cursor_line_ptr()) + 1;
assert(ind_len + line_len >= 0);
size_t line_size;
STRICT_ADD(ind_len, line_len, &line_size, size_t);
line = xmalloc(line_size);
p = line;
}
}
memmove(p, get_cursor_line_ptr(), (size_t)line_len);
ml_replace(curwin->w_cursor.lnum, line, false);
curwin->w_cursor.col = ind_len;
return true;
}
int open_line(
int dir, 
int flags,
int second_line_indent
)
{
char_u *next_line = NULL; 
char_u *p_extra = NULL; 
colnr_T less_cols = 0; 
colnr_T less_cols_off = 0; 
pos_T old_cursor; 
colnr_T newcol = 0; 
int newindent = 0; 
bool trunc_line = false; 
bool retval = false; 
int extra_len = 0; 
int lead_len; 
char_u *lead_flags; 
char_u *leader = NULL; 
char_u *allocated = NULL; 
char_u *p;
char_u saved_char = NUL; 
pos_T *pos;
bool do_si = (!p_paste && curbuf->b_p_si && !curbuf->b_p_cin
&& *curbuf->b_p_inde == NUL);
bool no_si = false; 
int first_char = NUL; 
int vreplace_mode;
bool did_append; 
int saved_pi = curbuf->b_p_pi; 
linenr_T lnum = curwin->w_cursor.lnum;
colnr_T mincol = curwin->w_cursor.col + 1;
char_u *saved_line = vim_strsave(get_cursor_line_ptr());
if (State & VREPLACE_FLAG) {
if (curwin->w_cursor.lnum < orig_line_count) {
next_line = vim_strsave(ml_get(curwin->w_cursor.lnum + 1));
} else {
next_line = vim_strsave((char_u *)"");
}
replace_push(NUL); 
replace_push(NUL);
p = saved_line + curwin->w_cursor.col;
while (*p != NUL) {
p += replace_push_mb(p);
}
saved_line[curwin->w_cursor.col] = NUL;
}
if ((State & INSERT)
&& !(State & VREPLACE_FLAG)
) {
p_extra = saved_line + curwin->w_cursor.col;
if (do_si) { 
p = skipwhite(p_extra);
first_char = *p;
}
extra_len = (int)STRLEN(p_extra);
saved_char = *p_extra;
*p_extra = NUL;
}
u_clearline(); 
did_si = false;
ai_col = 0;
if (dir == FORWARD && did_ai) {
trunc_line = true;
}
if (curbuf->b_p_ai
|| do_si
) {
newindent = get_indent_str(saved_line, (int)curbuf->b_p_ts, false);
if (newindent == 0 && !(flags & OPENLINE_COM_LIST)) {
newindent = second_line_indent; 
}
if (!trunc_line && do_si && *saved_line != NUL
&& (p_extra == NULL || first_char != '{')) {
char_u *ptr;
char_u last_char;
old_cursor = curwin->w_cursor;
ptr = saved_line;
if (flags & OPENLINE_DO_COM) {
lead_len = get_leader_len(ptr, NULL, false, true);
} else {
lead_len = 0;
}
if (dir == FORWARD) {
if (lead_len == 0 && ptr[0] == '#') {
while (ptr[0] == '#' && curwin->w_cursor.lnum > 1) {
ptr = ml_get(--curwin->w_cursor.lnum);
}
newindent = get_indent();
}
if (flags & OPENLINE_DO_COM) {
lead_len = get_leader_len(ptr, NULL, false, true);
} else {
lead_len = 0;
}
if (lead_len > 0) {
p = skipwhite(ptr);
if (p[0] == '/' && p[1] == '*') {
p++;
}
if (p[0] == '*') {
for (p++; *p; p++) {
if (p[0] == '/' && p[-1] == '*') {
curwin->w_cursor.col = (colnr_T)(p - ptr);
if ((pos = findmatch(NULL, NUL)) != NULL) {
curwin->w_cursor.lnum = pos->lnum;
newindent = get_indent();
}
}
}
}
} else { 
p = ptr + STRLEN(ptr) - 1;
while (p > ptr && ascii_iswhite(*p)) {
p--;
}
last_char = *p;
if (last_char == '{' || last_char == ';') {
if (p > ptr) {
p--;
}
while (p > ptr && ascii_iswhite(*p)) {
p--;
}
}
if (*p == ')') {
curwin->w_cursor.col = (colnr_T)(p - ptr);
if ((pos = findmatch(NULL, '(')) != NULL) {
curwin->w_cursor.lnum = pos->lnum;
newindent = get_indent();
ptr = get_cursor_line_ptr();
}
}
if (last_char == '{') {
did_si = true; 
no_si = true; 
} else if (last_char != ';' && last_char != '}'
&& cin_is_cinword(ptr)) {
did_si = true;
}
}
} else { 
if (lead_len == 0 && ptr[0] == '#') {
bool was_backslashed = false;
while ((ptr[0] == '#' || was_backslashed)
&& curwin->w_cursor.lnum < curbuf->b_ml.ml_line_count) {
if (*ptr && ptr[STRLEN(ptr) - 1] == '\\') {
was_backslashed = true;
} else {
was_backslashed = false;
}
ptr = ml_get(++curwin->w_cursor.lnum);
}
if (was_backslashed) {
newindent = 0; 
} else {
newindent = get_indent();
}
}
p = skipwhite(ptr);
if (*p == '}') { 
did_si = true;
} else { 
can_si_back = true;
}
}
curwin->w_cursor = old_cursor;
}
if (do_si) {
can_si = true;
}
did_ai = true;
}
end_comment_pending = NUL;
if (flags & OPENLINE_DO_COM) {
lead_len = get_leader_len(saved_line, &lead_flags, dir == BACKWARD, true);
} else {
lead_len = 0;
}
if (lead_len > 0) {
char_u *lead_repl = NULL; 
int lead_repl_len = 0; 
char_u lead_middle[COM_MAX_LEN]; 
char_u lead_end[COM_MAX_LEN]; 
char_u *comment_end = NULL; 
int extra_space = false; 
int current_flag;
int require_blank = false; 
char_u *p2;
for (p = lead_flags; *p && *p != ':'; p++) {
if (*p == COM_BLANK) {
require_blank = true;
continue;
}
if (*p == COM_START || *p == COM_MIDDLE) {
current_flag = *p;
if (*p == COM_START) {
if (dir == BACKWARD) {
lead_len = 0;
break;
}
(void)copy_option_part(&p, lead_middle, COM_MAX_LEN, ",");
require_blank = false;
}
while (*p && p[-1] != ':') { 
if (*p == COM_BLANK) {
require_blank = true;
}
p++;
}
(void)copy_option_part(&p, lead_middle, COM_MAX_LEN, ",");
while (*p && p[-1] != ':') { 
if (*p == COM_AUTO_END) {
end_comment_pending = -1; 
}
p++;
}
size_t n = copy_option_part(&p, lead_end, COM_MAX_LEN, ",");
if (end_comment_pending == -1) { 
end_comment_pending = lead_end[n - 1];
}
if (dir == FORWARD) {
for (p = saved_line + lead_len; *p; p++) {
if (STRNCMP(p, lead_end, n) == 0) {
comment_end = p;
lead_len = 0;
break;
}
}
}
if (lead_len > 0) {
if (current_flag == COM_START) {
lead_repl = lead_middle;
lead_repl_len = (int)STRLEN(lead_middle);
}
if (!ascii_iswhite(saved_line[lead_len - 1])
&& ((p_extra != NULL
&& (int)curwin->w_cursor.col == lead_len)
|| (p_extra == NULL
&& saved_line[lead_len] == NUL)
|| require_blank)) {
extra_space = true;
}
}
break;
}
if (*p == COM_END) {
if (dir == FORWARD) {
comment_end = skipwhite(saved_line);
lead_len = 0;
break;
}
while (p > curbuf->b_p_com && *p != ',') {
p--;
}
for (lead_repl = p; lead_repl > curbuf->b_p_com
&& lead_repl[-1] != ':'; lead_repl--) {
}
lead_repl_len = (int)(p - lead_repl);
extra_space = true;
for (p2 = p; *p2 && *p2 != ':'; p2++) {
if (*p2 == COM_AUTO_END) {
end_comment_pending = -1; 
}
}
if (end_comment_pending == -1) {
while (*p2 && *p2 != ',') {
p2++;
}
end_comment_pending = p2[-1];
}
break;
}
if (*p == COM_FIRST) {
if (dir == BACKWARD) {
lead_len = 0;
} else {
lead_repl = (char_u *)"";
lead_repl_len = 0;
}
break;
}
}
if (lead_len > 0) {
int bytes = lead_len
+ lead_repl_len
+ extra_space
+ extra_len
+ (second_line_indent > 0 ? second_line_indent : 0)
+ 1;
assert(bytes >= 0);
leader = xmalloc((size_t)bytes);
allocated = leader; 
STRLCPY(leader, saved_line, lead_len + 1);
if (lead_repl != NULL) {
int c = 0;
int off = 0;
for (p = lead_flags; *p != NUL && *p != ':'; ) {
if (*p == COM_RIGHT || *p == COM_LEFT) {
c = *p++;
} else if (ascii_isdigit(*p) || *p == '-') {
off = getdigits_int(&p, true, 0);
} else {
p++;
}
}
if (c == COM_RIGHT) { 
for (p = leader + lead_len - 1; p > leader
&& ascii_iswhite(*p); p--) {
}
p++;
{
int repl_size = vim_strnsize(lead_repl,
lead_repl_len);
int old_size = 0;
char_u *endp = p;
int l;
while (old_size < repl_size && p > leader) {
MB_PTR_BACK(leader, p);
old_size += ptr2cells(p);
}
l = lead_repl_len - (int)(endp - p);
if (l != 0) {
memmove(endp + l, endp,
(size_t)((leader + lead_len) - endp));
}
lead_len += l;
}
memmove(p, lead_repl, (size_t)lead_repl_len);
if (p + lead_repl_len > leader + lead_len) {
p[lead_repl_len] = NUL;
}
while (--p >= leader) {
int l = utf_head_off(leader, p);
if (l > 1) {
p -= l;
if (ptr2cells(p) > 1) {
p[1] = ' ';
l--;
}
memmove(p + 1, p + l + 1,
(size_t)((leader + lead_len) - (p + l + 1)));
lead_len -= l;
*p = ' ';
} else if (!ascii_iswhite(*p)) {
*p = ' ';
}
}
} else { 
p = skipwhite(leader);
{
int repl_size = vim_strnsize(lead_repl,
lead_repl_len);
int i;
int l;
for (i = 0; i < lead_len && p[i] != NUL; i += l) {
l = (*mb_ptr2len)(p + i);
if (vim_strnsize(p, i + l) > repl_size) {
break;
}
}
if (i != lead_repl_len) {
memmove(p + lead_repl_len, p + i,
(size_t)(lead_len - i - (p - leader)));
lead_len += lead_repl_len - i;
}
}
memmove(p, lead_repl, (size_t)lead_repl_len);
for (p += lead_repl_len; p < leader + lead_len; p++) {
if (!ascii_iswhite(*p)) {
if (p + 1 < leader + lead_len && p[1] == TAB) {
lead_len--;
memmove(p, p + 1, (size_t)(leader + lead_len - p));
} else {
int l = (*mb_ptr2len)(p);
if (l > 1) {
if (ptr2cells(p) > 1) {
l--;
*p++ = ' ';
}
memmove(p + 1, p + l, (size_t)(leader + lead_len - p));
lead_len -= l - 1;
}
*p = ' ';
}
}
}
*p = NUL;
}
if (curbuf->b_p_ai
|| do_si
) {
newindent = get_indent_str(leader, (int)curbuf->b_p_ts, false);
}
if (newindent + off < 0) {
off = -newindent;
newindent = 0;
} else {
newindent += off;
}
while (off > 0 && lead_len > 0
&& leader[lead_len - 1] == ' ') {
if (vim_strchr(skipwhite(leader), '\t') != NULL) {
break;
}
lead_len--;
off--;
}
if (lead_len > 0 && ascii_iswhite(leader[lead_len - 1])) {
extra_space = false;
}
leader[lead_len] = NUL;
}
if (extra_space) {
leader[lead_len++] = ' ';
leader[lead_len] = NUL;
}
newcol = lead_len;
if (newindent
|| did_si
) {
while (lead_len && ascii_iswhite(*leader)) {
lead_len--;
newcol--;
leader++;
}
}
did_si = can_si = false;
} else if (comment_end != NULL) {
if (comment_end[0] == '*' && comment_end[1] == '/'
&& (curbuf->b_p_ai || do_si)) {
old_cursor = curwin->w_cursor;
curwin->w_cursor.col = (colnr_T)(comment_end - saved_line);
if ((pos = findmatch(NULL, NUL)) != NULL) {
curwin->w_cursor.lnum = pos->lnum;
newindent = get_indent();
}
curwin->w_cursor = old_cursor;
}
}
}
if (p_extra != NULL) {
*p_extra = saved_char; 
if (REPLACE_NORMAL(State)) {
replace_push(NUL); 
}
if (curbuf->b_p_ai || (flags & OPENLINE_DELSPACES)) {
while ((*p_extra == ' ' || *p_extra == '\t')
&& !utf_iscomposing(utf_ptr2char(p_extra + 1))) {
if (REPLACE_NORMAL(State)) {
replace_push(*p_extra);
}
p_extra++;
less_cols_off++;
}
}
less_cols = (int)(p_extra - saved_line);
}
if (p_extra == NULL) {
p_extra = (char_u *)""; 
}
if (lead_len > 0) {
if (flags & OPENLINE_COM_LIST && second_line_indent > 0) {
int i;
int padding = second_line_indent
- (newindent + (int)STRLEN(leader));
for (i = 0; i < padding; i++) {
STRCAT(leader, " ");
less_cols--;
newcol++;
}
}
STRCAT(leader, p_extra);
p_extra = leader;
did_ai = true; 
less_cols -= lead_len;
} else {
end_comment_pending = NUL; 
}
curbuf_splice_pending++;
old_cursor = curwin->w_cursor;
if (dir == BACKWARD) {
curwin->w_cursor.lnum--;
}
if (!(State & VREPLACE_FLAG) || old_cursor.lnum >= orig_line_count) {
if (ml_append(curwin->w_cursor.lnum, p_extra, (colnr_T)0, false) == FAIL) {
goto theend;
}
if (curwin->w_cursor.lnum + 1 < curbuf->b_ml.ml_line_count
|| curwin->w_p_diff) {
mark_adjust(curwin->w_cursor.lnum + 1, (linenr_T)MAXLNUM, 1L, 0L,
kExtmarkUndo);
}
did_append = true;
} else {
curwin->w_cursor.lnum++;
if (curwin->w_cursor.lnum >= Insstart.lnum + vr_lines_changed) {
(void)u_save_cursor(); 
vr_lines_changed++;
}
ml_replace(curwin->w_cursor.lnum, p_extra, true);
changed_bytes(curwin->w_cursor.lnum, 0);
curwin->w_cursor.lnum--;
did_append = false;
}
inhibit_delete_count++;
if (newindent
|| did_si
) {
curwin->w_cursor.lnum++;
if (did_si) {
int sw = get_sw_value(curbuf);
if (p_sr) {
newindent -= newindent % sw;
}
newindent += sw;
}
if (curbuf->b_p_ci) {
(void)copy_indent(newindent, saved_line);
curbuf->b_p_pi = true;
} else {
(void)set_indent(newindent, SIN_INSERT|SIN_NOMARK);
}
less_cols -= curwin->w_cursor.col;
ai_col = curwin->w_cursor.col;
if (REPLACE_NORMAL(State)) {
for (colnr_T n = 0; n < curwin->w_cursor.col; n++) {
replace_push(NUL);
}
}
newcol += curwin->w_cursor.col;
if (no_si) {
did_si = false;
}
}
inhibit_delete_count--;
if (REPLACE_NORMAL(State)) {
while (lead_len-- > 0) {
replace_push(NUL);
}
}
curwin->w_cursor = old_cursor;
if (dir == FORWARD) {
if (trunc_line || (State & INSERT)) {
saved_line[curwin->w_cursor.col] = NUL;
if (trunc_line && !(flags & OPENLINE_KEEPTRAIL)) {
truncate_spaces(saved_line);
}
ml_replace(curwin->w_cursor.lnum, saved_line, false);
saved_line = NULL;
if (did_append) {
changed_lines(curwin->w_cursor.lnum, curwin->w_cursor.col,
curwin->w_cursor.lnum + 1, 1L, true);
did_append = false;
if (flags & OPENLINE_MARKFIX) {
mark_col_adjust(curwin->w_cursor.lnum,
curwin->w_cursor.col + less_cols_off,
1L, (long)-less_cols, 0);
}
int cols_added = mincol-1+less_cols_off-less_cols;
extmark_splice(curbuf, (int)lnum-1, mincol-1, 0, less_cols_off,
1, cols_added, kExtmarkUndo);
} else {
changed_bytes(curwin->w_cursor.lnum, curwin->w_cursor.col);
}
}
curwin->w_cursor.lnum = old_cursor.lnum + 1;
}
if (did_append) {
changed_lines(curwin->w_cursor.lnum, 0, curwin->w_cursor.lnum, 1L, true);
extmark_splice(curbuf, (int)curwin->w_cursor.lnum-1,
0, 0, 0, 1, 0, kExtmarkUndo);
}
curbuf_splice_pending--;
curwin->w_cursor.col = newcol;
curwin->w_cursor.coladd = 0;
if (State & VREPLACE_FLAG) {
vreplace_mode = State; 
State = INSERT;
} else {
vreplace_mode = 0;
}
if (!p_paste
&& leader == NULL
&& curbuf->b_p_lisp
&& curbuf->b_p_ai) {
fixthisline(get_lisp_indent);
ai_col = (colnr_T)getwhitecols_curline();
}
if (!p_paste
&& (curbuf->b_p_cin
|| *curbuf->b_p_inde != NUL
)
&& in_cinkeys(dir == FORWARD
? KEY_OPEN_FORW
: KEY_OPEN_BACK, ' ', linewhite(curwin->w_cursor.lnum))) {
do_c_expr_indent();
ai_col = (colnr_T)getwhitecols_curline();
}
if (vreplace_mode != 0) {
State = vreplace_mode;
}
if (State & VREPLACE_FLAG) {
p_extra = vim_strsave(get_cursor_line_ptr());
ml_replace(curwin->w_cursor.lnum, next_line, false);
curwin->w_cursor.col = 0;
curwin->w_cursor.coladd = 0;
ins_bytes(p_extra); 
xfree(p_extra);
next_line = NULL;
}
retval = true; 
theend:
curbuf->b_p_pi = saved_pi;
xfree(saved_line);
xfree(next_line);
xfree(allocated);
return retval;
} 
void truncate_line(int fixpos)
{
char_u *newp;
linenr_T lnum = curwin->w_cursor.lnum;
colnr_T col = curwin->w_cursor.col;
if (col == 0) {
newp = vim_strsave((char_u *)"");
} else {
newp = vim_strnsave(ml_get(lnum), (size_t)col);
}
ml_replace(lnum, newp, false);
changed_bytes(lnum, curwin->w_cursor.col);
if (fixpos && curwin->w_cursor.col > 0) {
curwin->w_cursor.col--;
}
}
void del_lines(long nlines, int undo)
{
long n;
linenr_T first = curwin->w_cursor.lnum;
if (nlines <= 0) {
return;
}
if (undo && u_savedel(first, nlines) == FAIL) {
return;
}
for (n = 0; n < nlines; ) {
if (curbuf->b_ml.ml_flags & ML_EMPTY) { 
break;
}
ml_delete(first, true);
n++;
if (first > curbuf->b_ml.ml_line_count) {
break;
}
}
curwin->w_cursor.col = 0;
check_cursor_lnum();
deleted_lines_mark(first, n);
}
