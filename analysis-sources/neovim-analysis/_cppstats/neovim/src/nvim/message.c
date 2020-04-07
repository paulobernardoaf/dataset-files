#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include "nvim/vim.h"
#include "nvim/ascii.h"
#include "nvim/assert.h"
#include "nvim/message.h"
#include "nvim/charset.h"
#include "nvim/eval.h"
#include "nvim/ex_eval.h"
#include "nvim/ex_docmd.h"
#include "nvim/fileio.h"
#include "nvim/func_attr.h"
#include "nvim/getchar.h"
#include "nvim/main.h"
#include "nvim/mbyte.h"
#include "nvim/memory.h"
#include "nvim/misc1.h"
#include "nvim/keymap.h"
#include "nvim/garray.h"
#include "nvim/ops.h"
#include "nvim/option.h"
#include "nvim/normal.h"
#include "nvim/regexp.h"
#include "nvim/screen.h"
#include "nvim/strings.h"
#include "nvim/syntax.h"
#include "nvim/highlight.h"
#include "nvim/ui.h"
#include "nvim/ui_compositor.h"
#include "nvim/mouse.h"
#include "nvim/os/os.h"
#include "nvim/os/input.h"
#include "nvim/os/time.h"
#include "nvim/api/private/helpers.h"
typedef struct msgchunk_S msgchunk_T;
struct msgchunk_S {
msgchunk_T *sb_next;
msgchunk_T *sb_prev;
char sb_eol; 
int sb_msg_col; 
int sb_attr; 
char_u sb_text[1]; 
};
#define DLG_BUTTON_SEP '\n'
#define DLG_HOTKEY_CHAR '&'
static int confirm_msg_used = FALSE; 
#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "message.c.generated.h"
#endif
static char_u *confirm_msg = NULL; 
static char_u *confirm_msg_tail; 
MessageHistoryEntry *first_msg_hist = NULL;
MessageHistoryEntry *last_msg_hist = NULL;
static int msg_hist_len = 0;
static FILE *verbose_fd = NULL;
static int verbose_did_open = FALSE;
static const char *msg_ext_kind = NULL;
static Array msg_ext_chunks = ARRAY_DICT_INIT;
static garray_T msg_ext_last_chunk = GA_INIT(sizeof(char), 40);
static sattr_T msg_ext_last_attr = -1;
static size_t msg_ext_cur_len = 0;
static bool msg_ext_overwrite = false; 
static int msg_ext_visible = 0; 
static bool msg_ext_keep_after_cmdline = false;
static int msg_grid_pos_at_flush = 0;
static int msg_grid_scroll_discount = 0;
static void ui_ext_msg_set_pos(int row, bool scrolled)
{
char buf[MAX_MCO];
size_t size = utf_char2bytes(curwin->w_p_fcs_chars.msgsep, (char_u *)buf);
buf[size] = '\0';
ui_call_msg_set_pos(msg_grid.handle, row, scrolled,
(String){ .data = buf, .size = size });
}
void msg_grid_set_pos(int row, bool scrolled)
{
if (!msg_grid.throttled) {
ui_ext_msg_set_pos(row, scrolled);
msg_grid_pos_at_flush = row;
}
msg_grid_pos = row;
if (msg_grid.chars) {
msg_grid_adj.row_offset = -row;
}
}
bool msg_use_grid(void)
{
return default_grid.chars && msg_use_msgsep()
&& !ui_has(kUIMessages);
}
void msg_grid_validate(void)
{
grid_assign_handle(&msg_grid);
bool should_alloc = msg_use_grid();
if (should_alloc && (msg_grid.Rows != Rows || msg_grid.Columns != Columns
|| !msg_grid.chars)) {
grid_alloc(&msg_grid, Rows, Columns, false, true);
xfree(msg_grid.dirty_col);
msg_grid.dirty_col = xcalloc(Rows, sizeof(*msg_grid.dirty_col));
int pos = msg_scrolled ? msg_grid_pos : Rows - p_ch;
ui_comp_put_grid(&msg_grid, pos, 0, msg_grid.Rows, msg_grid.Columns,
false, true);
ui_call_grid_resize(msg_grid.handle, msg_grid.Columns, msg_grid.Rows);
msg_grid.throttled = false; 
msg_scrolled_at_flush = msg_scrolled;
msg_grid.focusable = false;
if (!msg_scrolled) {
msg_grid_set_pos(Rows - p_ch, false);
}
} else if (!should_alloc && msg_grid.chars) {
ui_comp_remove_grid(&msg_grid);
grid_free(&msg_grid);
XFREE_CLEAR(msg_grid.dirty_col);
ui_call_grid_destroy(msg_grid.handle);
msg_grid.throttled = false;
msg_grid_adj.row_offset = 0;
redraw_cmdline = true;
} else if (msg_grid.chars && !msg_scrolled && msg_grid_pos != Rows - p_ch) {
msg_grid_set_pos(Rows - p_ch, false);
}
if (msg_grid.chars && cmdline_row < msg_grid_pos) {
cmdline_row = msg_grid_pos;
}
}
int msg(char_u *s)
{
return msg_attr_keep(s, 0, false, false);
}
int verb_msg(char *s)
{
verbose_enter();
int n = msg_attr_keep((char_u *)s, 0, false, false);
verbose_leave();
return n;
}
int msg_attr(const char *s, const int attr)
FUNC_ATTR_NONNULL_ARG(1)
{
return msg_attr_keep((char_u *)s, attr, false, false);
}
void msg_multiline_attr(const char *s, int attr,
bool check_int, bool *need_clear)
FUNC_ATTR_NONNULL_ALL
{
const char *next_spec = s;
while (next_spec != NULL) {
if (check_int && got_int) {
return;
}
next_spec = strpbrk(s, "\t\n\r");
if (next_spec != NULL) {
msg_outtrans_len_attr((const char_u *)s, next_spec - s, attr);
if (*next_spec != TAB && *need_clear) {
msg_clr_eos();
*need_clear = false;
}
msg_putchar_attr((uint8_t)(*next_spec), attr);
s = next_spec + 1;
}
}
if (*s != NUL) {
msg_outtrans_attr((char_u *)s, attr);
}
return;
}
bool msg_attr_keep(char_u *s, int attr, bool keep, bool multiline)
FUNC_ATTR_NONNULL_ALL
{
static int entered = 0;
int retval;
char_u *buf = NULL;
if (keep && multiline) {
abort();
}
if (!emsg_on_display && message_filtered(s)) {
return true;
}
if (attr == 0) {
set_vim_var_string(VV_STATUSMSG, (char *) s, -1);
}
if (entered >= 3)
return TRUE;
++entered;
if (s != keep_msg
|| (*s != '<'
&& last_msg_hist != NULL
&& last_msg_hist->msg != NULL
&& STRCMP(s, last_msg_hist->msg))) {
add_msg_hist((const char *)s, -1, attr, multiline);
}
if (s == keep_msg)
keep_msg = NULL;
msg_start();
buf = msg_strtrunc(s, FALSE);
if (buf != NULL)
s = buf;
bool need_clear = true;
if (multiline) {
msg_multiline_attr((char *)s, attr, false, &need_clear);
} else {
msg_outtrans_attr(s, attr);
}
if (need_clear) {
msg_clr_eos();
}
retval = msg_end();
if (keep && retval && vim_strsize(s) < (int)(Rows - cmdline_row - 1)
* Columns + sc_col) {
set_keep_msg(s, 0);
}
xfree(buf);
--entered;
return retval;
}
char_u *
msg_strtrunc (
char_u *s,
int force 
)
{
char_u *buf = NULL;
int len;
int room;
if ((!msg_scroll && !need_wait_return && shortmess(SHM_TRUNCALL)
&& !exmode_active && msg_silent == 0 && !ui_has(kUIMessages))
|| force) {
len = vim_strsize(s);
if (msg_scrolled != 0)
room = (int)(Rows - msg_row) * Columns - 1;
else
room = (int)(Rows - msg_row - 1) * Columns + sc_col - 1;
if (len > room && room > 0) {
len = (room + 2) * 18;
buf = xmalloc(len);
trunc_string(s, buf, room, len);
}
}
return buf;
}
void trunc_string(char_u *s, char_u *buf, int room_in, int buflen)
{
size_t room = room_in - 3; 
size_t half;
size_t len = 0;
int e;
int i;
int n;
if (room_in < 3) {
room = 0;
}
half = room / 2;
for (e = 0; len < half && e < buflen; ++e) {
if (s[e] == NUL) {
buf[e] = NUL;
return;
}
n = ptr2cells(s + e);
if (len + n > half) {
break;
}
len += n;
buf[e] = s[e];
if (has_mbyte)
for (n = (*mb_ptr2len)(s + e); --n > 0; ) {
if (++e == buflen)
break;
buf[e] = s[e];
}
}
half = i = (int)STRLEN(s);
for (;;) {
do {
half = half - utf_head_off(s, s + half - 1) - 1;
} while (half > 0 && utf_iscomposing(utf_ptr2char(s + half)));
n = ptr2cells(s + half);
if (len + n > room || half == 0) {
break;
}
len += n;
i = half;
}
if (i <= e + 3) {
if (s != buf) {
len = STRLEN(s);
if (len >= (size_t)buflen) {
len = buflen - 1;
}
len = len - e + 1;
if (len < 1) {
buf[e - 1] = NUL;
} else {
memmove(buf + e, s + e, len);
}
}
} else if (e + 3 < buflen) {
memmove(buf + e, "...", (size_t)3);
len = STRLEN(s + i) + 1;
if (len >= (size_t)buflen - e - 3) {
len = buflen - e - 3 - 1;
}
memmove(buf + e + 3, s + i, len);
buf[e + 3 + len - 1] = NUL;
} else {
buf[e - 1] = NUL;
}
}
int smsg(char *s, ...)
FUNC_ATTR_PRINTF(1, 2)
{
va_list arglist;
va_start(arglist, s);
vim_vsnprintf((char *)IObuff, IOSIZE, s, arglist);
va_end(arglist);
return msg(IObuff);
}
int smsg_attr(int attr, char *s, ...)
FUNC_ATTR_PRINTF(2, 3)
{
va_list arglist;
va_start(arglist, s);
vim_vsnprintf((char *)IObuff, IOSIZE, s, arglist);
va_end(arglist);
return msg_attr((const char *)IObuff, attr);
}
int smsg_attr_keep(int attr, char *s, ...)
FUNC_ATTR_PRINTF(2, 3)
{
va_list arglist;
va_start(arglist, s);
vim_vsnprintf((char *)IObuff, IOSIZE, s, arglist);
va_end(arglist);
return msg_attr_keep(IObuff, attr, true, false);
}
static int last_sourcing_lnum = 0;
static char_u *last_sourcing_name = NULL;
void reset_last_sourcing(void)
{
XFREE_CLEAR(last_sourcing_name);
last_sourcing_lnum = 0;
}
static int other_sourcing_name(void)
{
if (sourcing_name != NULL) {
if (last_sourcing_name != NULL)
return STRCMP(sourcing_name, last_sourcing_name) != 0;
return TRUE;
}
return FALSE;
}
static char *get_emsg_source(void)
FUNC_ATTR_MALLOC FUNC_ATTR_WARN_UNUSED_RESULT
{
if (sourcing_name != NULL && other_sourcing_name()) {
const char *const p = _("Error detected while processing %s:");
const size_t buf_len = STRLEN(sourcing_name) + strlen(p) + 1;
char *const buf = xmalloc(buf_len);
snprintf(buf, buf_len, p, sourcing_name);
return buf;
}
return NULL;
}
static char *get_emsg_lnum(void)
FUNC_ATTR_MALLOC FUNC_ATTR_WARN_UNUSED_RESULT
{
if (sourcing_name != NULL
&& (other_sourcing_name() || sourcing_lnum != last_sourcing_lnum)
&& sourcing_lnum != 0) {
const char *const p = _("line %4ld:");
const size_t buf_len = 20 + strlen(p);
char *const buf = xmalloc(buf_len);
snprintf(buf, buf_len, p, (long)sourcing_lnum);
return buf;
}
return NULL;
}
void msg_source(int attr)
{
no_wait_return++;
char *p = get_emsg_source();
if (p != NULL) {
msg_attr(p, attr);
xfree(p);
}
p = get_emsg_lnum();
if (p != NULL) {
msg_attr(p, HL_ATTR(HLF_N));
xfree(p);
last_sourcing_lnum = sourcing_lnum; 
}
if (sourcing_name == NULL || other_sourcing_name()) {
xfree(last_sourcing_name);
if (sourcing_name == NULL)
last_sourcing_name = NULL;
else
last_sourcing_name = vim_strsave(sourcing_name);
}
--no_wait_return;
}
int emsg_not_now(void)
{
if ((emsg_off > 0 && vim_strchr(p_debug, 'm') == NULL
&& vim_strchr(p_debug, 't') == NULL)
|| emsg_skip > 0
)
return TRUE;
return FALSE;
}
static bool emsg_multiline(const char *s, bool multiline)
{
int attr;
int ignore = false;
int severe;
if (emsg_not_now()) {
return true;
}
called_emsg = true;
severe = emsg_severe;
emsg_severe = false;
if (!emsg_off || vim_strchr(p_debug, 't') != NULL) {
if (cause_errthrow((char_u *)s, severe, &ignore) == true) {
if (!ignore) {
did_emsg++;
}
return true;
}
set_vim_var_string(VV_ERRMSG, s, -1);
if (emsg_silent != 0) {
if (!emsg_noredir) {
msg_start();
char *p = get_emsg_source();
if (p != NULL) {
const size_t p_len = strlen(p);
p[p_len] = '\n';
redir_write(p, p_len + 1);
xfree(p);
}
p = get_emsg_lnum();
if (p != NULL) {
const size_t p_len = strlen(p);
p[p_len] = '\n';
redir_write(p, p_len + 1);
xfree(p);
}
redir_write(s, strlen(s));
}
if (sourcing_name != NULL && sourcing_lnum != 0) {
DLOG("(:silent) %s (%s (line %ld))",
s, sourcing_name, (long)sourcing_lnum);
} else {
DLOG("(:silent) %s", s);
}
return true;
}
if (sourcing_name != NULL && sourcing_lnum != 0) {
ILOG("%s (%s (line %ld))", s, sourcing_name, (long)sourcing_lnum);
} else {
ILOG("%s", s);
}
ex_exitval = 1;
msg_silent = 0;
cmd_silent = false;
if (global_busy) { 
global_busy++;
}
if (p_eb) {
beep_flush(); 
} else {
flush_buffers(FLUSH_MINIMAL); 
}
did_emsg++; 
}
emsg_on_display = true; 
msg_scroll++; 
attr = HL_ATTR(HLF_E); 
if (msg_scrolled != 0) {
need_wait_return = true; 
} 
if (msg_ext_kind == NULL) {
msg_ext_set_kind("emsg");
}
msg_source(attr);
msg_nowait = false; 
return msg_attr_keep((char_u *)s, attr, false, multiline);
}
bool emsg(const char_u *s)
{
return emsg_multiline((const char *)s, false);
}
void emsg_invreg(int name)
{
EMSG2(_("E354: Invalid register name: '%s'"), transchar(name));
}
bool emsgf(const char *const fmt, ...)
FUNC_ATTR_PRINTF(1, 2)
{
bool ret;
va_list ap;
va_start(ap, fmt);
ret = emsgfv(fmt, ap);
va_end(ap);
return ret;
}
#define MULTILINE_BUFSIZE 8192
bool emsgf_multiline(const char *const fmt, ...)
{
bool ret;
va_list ap;
static char errbuf[MULTILINE_BUFSIZE];
if (emsg_not_now()) {
return true;
}
va_start(ap, fmt);
vim_vsnprintf(errbuf, sizeof(errbuf), fmt, ap);
va_end(ap);
ret = emsg_multiline(errbuf, true);
return ret;
}
static bool emsgfv(const char *fmt, va_list ap)
{
static char errbuf[IOSIZE];
if (emsg_not_now()) {
return true;
}
vim_vsnprintf(errbuf, sizeof(errbuf), fmt, ap);
return emsg((const char_u *)errbuf);
}
void iemsg(const char *s)
{
emsg((char_u *)s);
#if defined(ABORT_ON_INTERNAL_ERROR)
abort();
#endif
}
void iemsgf(const char *s, ...)
{
va_list ap;
va_start(ap, s);
(void)emsgfv(s, ap);
va_end(ap);
#if defined(ABORT_ON_INTERNAL_ERROR)
abort();
#endif
}
void internal_error(char *where)
{
IEMSG2(_(e_intern2), where);
}
static void msg_emsgf_event(void **argv)
{
char *s = argv[0];
(void)emsg((char_u *)s);
xfree(s);
}
void msg_schedule_emsgf(const char *const fmt, ...)
FUNC_ATTR_PRINTF(1, 2)
{
va_list ap;
va_start(ap, fmt);
vim_vsnprintf((char *)IObuff, IOSIZE, fmt, ap);
va_end(ap);
char *s = xstrdup((char *)IObuff);
multiqueue_put(main_loop.events, msg_emsgf_event, 1, s);
}
char_u *msg_trunc_attr(char_u *s, int force, int attr)
{
int n;
add_msg_hist((const char *)s, -1, attr, false);
s = msg_may_trunc(force, s);
msg_hist_off = true;
n = msg_attr((const char *)s, attr);
msg_hist_off = false;
if (n)
return s;
return NULL;
}
char_u *msg_may_trunc(int force, char_u *s)
{
int n;
int room;
room = (int)(Rows - cmdline_row - 1) * Columns + sc_col - 1;
if ((force || (shortmess(SHM_TRUNC) && !exmode_active))
&& (n = (int)STRLEN(s) - room) > 0) {
if (has_mbyte) {
int size = vim_strsize(s);
if (size <= room)
return s;
for (n = 0; size >= room; ) {
size -= utf_ptr2cells(s + n);
n += utfc_ptr2len(s + n);
}
--n;
}
s += n;
*s = '<';
}
return s;
}
static void add_msg_hist(const char *s, int len, int attr, bool multiline)
{
if (msg_hist_off || msg_silent != 0)
return;
while (msg_hist_len > MAX_MSG_HIST_LEN)
(void)delete_first_msg();
struct msg_hist *p = xmalloc(sizeof(struct msg_hist));
if (len < 0)
len = (int)STRLEN(s);
while (len > 0 && *s == '\n') {
++s;
--len;
}
while (len > 0 && s[len - 1] == '\n') {
len--;
}
p->msg = (char_u *)xmemdupz(s, (size_t)len);
p->next = NULL;
p->attr = attr;
p->multiline = multiline;
p->kind = msg_ext_kind;
if (last_msg_hist != NULL) {
last_msg_hist->next = p;
}
last_msg_hist = p;
if (first_msg_hist == NULL) {
first_msg_hist = last_msg_hist;
}
msg_hist_len++;
}
int delete_first_msg(void)
{
struct msg_hist *p;
if (msg_hist_len <= 0)
return FAIL;
p = first_msg_hist;
first_msg_hist = p->next;
if (first_msg_hist == NULL) { 
assert(msg_hist_len == 1);
last_msg_hist = NULL;
}
xfree(p->msg);
xfree(p);
--msg_hist_len;
return OK;
}
void ex_messages(void *const eap_p)
FUNC_ATTR_NONNULL_ALL
{
const exarg_T *const eap = (const exarg_T *)eap_p;
struct msg_hist *p;
int c = 0;
if (STRCMP(eap->arg, "clear") == 0) {
int keep = eap->addr_count == 0 ? 0 : eap->line2;
while (msg_hist_len > keep) {
(void)delete_first_msg();
}
return;
}
if (*eap->arg != NUL) {
EMSG(_(e_invarg));
return;
}
p = first_msg_hist;
if (eap->addr_count != 0) {
for (; p != NULL && !got_int; p = p->next) {
c++;
}
c -= eap->line2;
for (p = first_msg_hist; p != NULL && !got_int && c > 0; p = p->next, c--) {
}
}
if (ui_has(kUIMessages)) {
Array entries = ARRAY_DICT_INIT;
for (; p != NULL; p = p->next) {
if (p->msg != NULL && p->msg[0] != NUL) {
Array entry = ARRAY_DICT_INIT;
ADD(entry, STRING_OBJ(cstr_to_string(p->kind)));
Array content_entry = ARRAY_DICT_INIT;
ADD(content_entry, INTEGER_OBJ(p->attr));
ADD(content_entry, STRING_OBJ(cstr_to_string((char *)(p->msg))));
Array content = ARRAY_DICT_INIT;
ADD(content, ARRAY_OBJ(content_entry));
ADD(entry, ARRAY_OBJ(content));
ADD(entries, ARRAY_OBJ(entry));
}
}
ui_call_msg_history_show(entries);
} else {
msg_hist_off = true;
for (; p != NULL && !got_int; p = p->next) {
if (p->msg != NULL) {
msg_attr_keep(p->msg, p->attr, false, p->multiline);
}
}
msg_hist_off = false;
}
}
void msg_end_prompt(void)
{
msg_ext_clear_later();
need_wait_return = false;
emsg_on_display = false;
cmdline_row = msg_row;
msg_col = 0;
msg_clr_eos();
lines_left = -1;
}
void wait_return(int redraw)
{
int c;
int oldState;
int tmpState;
int had_got_int;
FILE *save_scriptout;
if (redraw == true) {
redraw_all_later(NOT_VALID);
}
if (msg_silent != 0)
return;
if (vgetc_busy > 0)
return;
need_wait_return = TRUE;
if (no_wait_return) {
if (!exmode_active)
cmdline_row = msg_row;
return;
}
redir_off = TRUE; 
oldState = State;
if (quit_more) {
c = CAR; 
quit_more = FALSE;
got_int = FALSE;
} else if (exmode_active) {
MSG_PUTS(" "); 
c = CAR; 
got_int = FALSE;
} else {
screenalloc();
State = HITRETURN;
setmouse();
cmdline_row = msg_row;
if (need_check_timestamps) {
check_timestamps(false);
}
hit_return_msg();
do {
had_got_int = got_int;
no_mapping++;
const int save_reg_recording = reg_recording;
save_scriptout = scriptout;
reg_recording = 0;
scriptout = NULL;
c = safe_vgetc();
if (had_got_int && !global_busy) {
got_int = false;
}
no_mapping--;
reg_recording = save_reg_recording;
scriptout = save_scriptout;
if (p_more) {
if (c == 'b' || c == 'k' || c == 'u' || c == 'g'
|| c == K_UP || c == K_PAGEUP) {
if (msg_scrolled > Rows)
do_more_prompt(c);
else {
msg_didout = FALSE;
c = K_IGNORE;
msg_col =
cmdmsg_rl ? Columns - 1 :
0;
}
if (quit_more) {
c = CAR; 
quit_more = FALSE;
got_int = FALSE;
} else if (c != K_IGNORE) {
c = K_IGNORE;
hit_return_msg();
}
} else if (msg_scrolled > Rows - 2
&& (c == 'j' || c == 'd' || c == 'f'
|| c == K_DOWN || c == K_PAGEDOWN))
c = K_IGNORE;
}
} while ((had_got_int && c == Ctrl_C)
|| c == K_IGNORE
|| c == K_LEFTDRAG || c == K_LEFTRELEASE
|| c == K_MIDDLEDRAG || c == K_MIDDLERELEASE
|| c == K_RIGHTDRAG || c == K_RIGHTRELEASE
|| c == K_MOUSELEFT || c == K_MOUSERIGHT
|| c == K_MOUSEDOWN || c == K_MOUSEUP
|| (!mouse_has(MOUSE_RETURN)
&& mouse_row < msg_row
&& (c == K_LEFTMOUSE
|| c == K_MIDDLEMOUSE
|| c == K_RIGHTMOUSE
|| c == K_X1MOUSE
|| c == K_X2MOUSE))
);
os_breakcheck();
if (c == K_LEFTMOUSE || c == K_MIDDLEMOUSE || c == K_RIGHTMOUSE
|| c == K_X1MOUSE || c == K_X2MOUSE)
(void)jump_to_mouse(MOUSE_SETPOS, NULL, 0);
else if (vim_strchr((char_u *)"\r\n ", c) == NULL && c != Ctrl_C) {
ins_char_typebuf(c);
do_redraw = true; 
}
}
redir_off = false;
if (c == ':' || c == '?' || c == '/') {
if (!exmode_active)
cmdline_row = msg_row;
skip_redraw = true; 
do_redraw = false;
msg_ext_keep_after_cmdline = true;
}
tmpState = State;
State = oldState; 
setmouse();
msg_check();
need_wait_return = false;
did_wait_return = true;
emsg_on_display = false; 
lines_left = -1; 
reset_last_sourcing();
if (keep_msg != NULL && vim_strsize(keep_msg) >=
(Rows - cmdline_row - 1) * Columns + sc_col) {
XFREE_CLEAR(keep_msg); 
}
if (tmpState == SETWSIZE) { 
ui_refresh();
} else if (!skip_redraw) {
if (redraw == true || (msg_scrolled != 0 && redraw != -1)) {
redraw_later(VALID);
}
if (ui_has(kUIMessages)) {
msg_ext_clear(true);
}
}
}
static void hit_return_msg(void)
{
int save_p_more = p_more;
p_more = FALSE; 
if (msg_didout) 
msg_putchar('\n');
msg_ext_set_kind("return_prompt");
if (got_int) {
MSG_PUTS(_("Interrupt: "));
}
MSG_PUTS_ATTR(_("Press ENTER or type command to continue"), HL_ATTR(HLF_R));
if (!msg_use_printf()) {
msg_clr_eos();
}
p_more = save_p_more;
}
void set_keep_msg(char_u *s, int attr)
{
xfree(keep_msg);
if (s != NULL && msg_silent == 0)
keep_msg = vim_strsave(s);
else
keep_msg = NULL;
keep_msg_more = FALSE;
keep_msg_attr = attr;
}
void msg_ext_set_kind(const char *msg_kind)
{
msg_ext_ui_flush();
msg_ext_kind = msg_kind;
}
void msg_start(void)
{
int did_return = false;
if (!msg_silent) {
XFREE_CLEAR(keep_msg); 
}
if (need_clr_eos) {
need_clr_eos = false;
msg_clr_eos();
}
if (!msg_scroll && full_screen) { 
msg_row = cmdline_row;
msg_col =
cmdmsg_rl ? Columns - 1 :
0;
} else if (msg_didout) { 
msg_putchar('\n');
did_return = TRUE;
if (exmode_active != EXMODE_NORMAL)
cmdline_row = msg_row;
}
if (!msg_didany || lines_left < 0)
msg_starthere();
if (msg_silent == 0) {
msg_didout = false; 
}
if (ui_has(kUIMessages)) {
msg_ext_ui_flush();
if (!msg_scroll && msg_ext_visible) {
msg_ext_overwrite = true;
}
}
if (!did_return) {
redir_write("\n", 1);
}
}
void msg_starthere(void)
{
lines_left = cmdline_row;
msg_didany = FALSE;
}
void msg_putchar(int c)
{
msg_putchar_attr(c, 0);
}
void msg_putchar_attr(int c, int attr)
{
char buf[MB_MAXBYTES + 1];
if (IS_SPECIAL(c)) {
buf[0] = (char)K_SPECIAL;
buf[1] = (char)K_SECOND(c);
buf[2] = (char)K_THIRD(c);
buf[3] = NUL;
} else {
buf[utf_char2bytes(c, (char_u *)buf)] = NUL;
}
msg_puts_attr(buf, attr);
}
void msg_outnum(long n)
{
char buf[20];
snprintf(buf, sizeof(buf), "%ld", n);
msg_puts(buf);
}
void msg_home_replace(char_u *fname)
{
msg_home_replace_attr(fname, 0);
}
void msg_home_replace_hl(char_u *fname)
{
msg_home_replace_attr(fname, HL_ATTR(HLF_D));
}
static void msg_home_replace_attr(char_u *fname, int attr)
{
char_u *name;
name = home_replace_save(NULL, fname);
msg_outtrans_attr(name, attr);
xfree(name);
}
int msg_outtrans(char_u *str)
{
return msg_outtrans_attr(str, 0);
}
int msg_outtrans_attr(const char_u *str, int attr)
{
return msg_outtrans_len_attr(str, (int)STRLEN(str), attr);
}
int msg_outtrans_len(const char_u *str, int len)
{
return msg_outtrans_len_attr(str, len, 0);
}
char_u *msg_outtrans_one(char_u *p, int attr)
{
int l;
if ((l = utfc_ptr2len(p)) > 1) {
msg_outtrans_len_attr(p, l, attr);
return p + l;
}
msg_puts_attr((const char *)transchar_byte(*p), attr);
return p + 1;
}
int msg_outtrans_len_attr(const char_u *msgstr, int len, int attr)
{
int retval = 0;
const char *str = (const char *)msgstr;
const char *plain_start = (const char *)msgstr;
char_u *s;
int mb_l;
int c;
if (attr & MSG_HIST) {
add_msg_hist(str, len, attr, false);
attr &= ~MSG_HIST;
}
if (enc_utf8 && utf_iscomposing(utf_ptr2char(msgstr))) {
msg_puts_attr(" ", attr);
}
while (--len >= 0) {
mb_l = utfc_ptr2len_len((char_u *)str, len + 1);
if (mb_l > 1) {
c = utf_ptr2char((char_u *)str);
if (vim_isprintc(c)) {
retval += utf_ptr2cells((char_u *)str);
} else {
if (str > plain_start) {
msg_puts_attr_len(plain_start, str - plain_start, attr);
}
plain_start = str + mb_l;
msg_puts_attr((const char *)transchar(c),
(attr == 0 ? HL_ATTR(HLF_8) : attr));
retval += char2cells(c);
}
len -= mb_l - 1;
str += mb_l;
} else {
s = transchar_byte((uint8_t)(*str));
if (s[1] != NUL) {
if (str > plain_start) {
msg_puts_attr_len(plain_start, str - plain_start, attr);
}
plain_start = str + 1;
msg_puts_attr((const char *)s, attr == 0 ? HL_ATTR(HLF_8) : attr);
retval += (int)STRLEN(s);
} else {
retval++;
}
str++;
}
}
if (str > plain_start) {
msg_puts_attr_len(plain_start, str - plain_start, attr);
}
return retval;
}
void msg_make(char_u *arg)
{
int i;
static char_u *str = (char_u *)"eeffoc", *rs = (char_u *)"Plon#dqg#vxjduB";
arg = skipwhite(arg);
for (i = 5; *arg && i >= 0; --i)
if (*arg++ != str[i])
break;
if (i < 0) {
msg_putchar('\n');
for (i = 0; rs[i]; ++i)
msg_putchar(rs[i] - 3);
}
}
int msg_outtrans_special(
const char_u *strstart,
bool from, 
int maxlen 
)
{
if (strstart == NULL) {
return 0; 
}
const char_u *str = strstart;
int retval = 0;
int attr = HL_ATTR(HLF_8);
while (*str != NUL) {
const char *string;
if ((str == strstart || str[1] == NUL) && *str == ' ') {
string = "<Space>";
str++;
} else {
string = str2special((const char **)&str, from, false);
}
const int len = vim_strsize((char_u *)string);
if (maxlen > 0 && retval + len >= maxlen) {
break;
}
msg_puts_attr(string, (len > 1
&& (*mb_ptr2len)((char_u *)string) <= 1
? attr : 0));
retval += len;
}
return retval;
}
char *str2special_save(const char *const str, const bool replace_spaces,
const bool replace_lt)
FUNC_ATTR_NONNULL_ALL FUNC_ATTR_WARN_UNUSED_RESULT FUNC_ATTR_MALLOC
FUNC_ATTR_NONNULL_RET
{
garray_T ga;
ga_init(&ga, 1, 40);
const char *p = str;
while (*p != NUL) {
ga_concat(&ga, (const char_u *)str2special(&p, replace_spaces, replace_lt));
}
ga_append(&ga, NUL);
return (char *)ga.ga_data;
}
const char *str2special(const char **const sp, const bool replace_spaces,
const bool replace_lt)
FUNC_ATTR_NONNULL_ALL FUNC_ATTR_WARN_UNUSED_RESULT FUNC_ATTR_NONNULL_RET
{
static char buf[7];
const char *const p = mb_unescape(sp);
if (p != NULL) {
return p;
}
const char *str = *sp;
int c = (uint8_t)(*str);
int modifiers = 0;
bool special = false;
if (c == K_SPECIAL && str[1] != NUL && str[2] != NUL) {
if ((uint8_t)str[1] == KS_MODIFIER) {
modifiers = (uint8_t)str[2];
str += 3;
c = (uint8_t)(*str);
}
if (c == K_SPECIAL && str[1] != NUL && str[2] != NUL) {
c = TO_SPECIAL((uint8_t)str[1], (uint8_t)str[2]);
str += 2;
}
if (IS_SPECIAL(c) || modifiers) { 
special = true;
}
}
if (!IS_SPECIAL(c)) {
const int len = utf_ptr2len((const char_u *)str);
if (MB_BYTE2LEN((uint8_t)(*str)) > len) {
transchar_nonprint((char_u *)buf, c);
*sp = str + 1;
return buf;
}
c = utf_ptr2char((const char_u *)str);
*sp = str + len;
} else {
*sp = str + 1;
}
if (special
|| char2cells(c) > 1
|| (replace_spaces && c == ' ')
|| (replace_lt && c == '<')) {
return (const char *)get_special_key_name(c, modifiers);
}
buf[0] = (char)c;
buf[1] = NUL;
return buf;
}
void str2specialbuf(const char *sp, char *buf, size_t len)
FUNC_ATTR_NONNULL_ALL
{
while (*sp) {
const char *s = str2special(&sp, false, false);
const size_t s_len = strlen(s);
if (len <= s_len) {
break;
}
memcpy(buf, s, s_len);
buf += s_len;
len -= s_len;
}
*buf = NUL;
}
void msg_prt_line(char_u *s, int list)
{
int c;
int col = 0;
int n_extra = 0;
int c_extra = 0;
int c_final = 0;
char_u *p_extra = NULL; 
int n;
int attr = 0;
char_u *trail = NULL;
int l;
if (curwin->w_p_list) {
list = true;
}
if (list && curwin->w_p_lcs_chars.trail) {
trail = s + STRLEN(s);
while (trail > s && ascii_iswhite(trail[-1])) {
trail--;
}
}
if (*s == NUL && !(list && curwin->w_p_lcs_chars.eol != NUL)) {
msg_putchar(' ');
}
while (!got_int) {
if (n_extra > 0) {
n_extra--;
if (n_extra == 0 && c_final) {
c = c_final;
} else if (c_extra) {
c = c_extra;
} else {
assert(p_extra != NULL);
c = *p_extra++;
}
} else if ((l = utfc_ptr2len(s)) > 1) {
col += utf_ptr2cells(s);
char buf[MB_MAXBYTES + 1];
if (curwin->w_p_lcs_chars.nbsp != NUL && list
&& (utf_ptr2char(s) == 160 || utf_ptr2char(s) == 0x202f)) {
utf_char2bytes(curwin->w_p_lcs_chars.nbsp, (char_u *)buf);
buf[utfc_ptr2len((char_u *)buf)] = NUL;
} else {
memmove(buf, s, (size_t)l);
buf[l] = NUL;
}
msg_puts(buf);
s += l;
continue;
} else {
attr = 0;
c = *s++;
if (c == TAB && (!list || curwin->w_p_lcs_chars.tab1)) {
n_extra = curbuf->b_p_ts - col % curbuf->b_p_ts - 1;
if (!list) {
c = ' ';
c_extra = ' ';
c_final = NUL;
} else {
c = (n_extra == 0 && curwin->w_p_lcs_chars.tab3)
? curwin->w_p_lcs_chars.tab3
: curwin->w_p_lcs_chars.tab1;
c_extra = curwin->w_p_lcs_chars.tab2;
c_final = curwin->w_p_lcs_chars.tab3;
attr = HL_ATTR(HLF_8);
}
} else if (c == 160 && list && curwin->w_p_lcs_chars.nbsp != NUL) {
c = curwin->w_p_lcs_chars.nbsp;
attr = HL_ATTR(HLF_8);
} else if (c == NUL && list && curwin->w_p_lcs_chars.eol != NUL) {
p_extra = (char_u *)"";
c_extra = NUL;
c_final = NUL;
n_extra = 1;
c = curwin->w_p_lcs_chars.eol;
attr = HL_ATTR(HLF_AT);
s--;
} else if (c != NUL && (n = byte2cells(c)) > 1) {
n_extra = n - 1;
p_extra = transchar_byte(c);
c_extra = NUL;
c_final = NUL;
c = *p_extra++;
attr = HL_ATTR(HLF_8);
} else if (c == ' ' && trail != NULL && s > trail) {
c = curwin->w_p_lcs_chars.trail;
attr = HL_ATTR(HLF_8);
} else if (c == ' ' && list && curwin->w_p_lcs_chars.space != NUL) {
c = curwin->w_p_lcs_chars.space;
attr = HL_ATTR(HLF_8);
}
}
if (c == NUL)
break;
msg_putchar_attr(c, attr);
col++;
}
msg_clr_eos();
}
static char_u *screen_puts_mbyte(char_u *s, int l, int attr)
{
int cw;
attr = hl_combine_attr(HL_ATTR(HLF_MSG), attr);
msg_didout = true; 
cw = utf_ptr2cells(s);
if (cw > 1
&& (cmdmsg_rl ? msg_col <= 1 : msg_col == Columns - 1)) {
msg_screen_putchar('>', HL_ATTR(HLF_AT));
return s;
}
grid_puts_len(&msg_grid_adj, s, l, msg_row, msg_col, attr);
if (cmdmsg_rl) {
msg_col -= cw;
if (msg_col == 0) {
msg_col = Columns;
++msg_row;
}
} else {
msg_col += cw;
if (msg_col >= Columns) {
msg_col = 0;
++msg_row;
}
}
return s + l;
}
void msg_puts(const char *s)
{
msg_puts_attr(s, 0);
}
void msg_puts_title(const char *s)
{
msg_puts_attr(s, HL_ATTR(HLF_T));
}
void msg_puts_long_attr(char_u *longstr, int attr)
{
msg_puts_long_len_attr(longstr, (int)STRLEN(longstr), attr);
}
void msg_puts_long_len_attr(char_u *longstr, int len, int attr)
{
int slen = len;
int room;
room = Columns - msg_col;
if (len > room && room >= 20) {
slen = (room - 3) / 2;
msg_outtrans_len_attr(longstr, slen, attr);
msg_puts_attr("...", HL_ATTR(HLF_8));
}
msg_outtrans_len_attr(longstr + len - slen, slen, attr);
}
void msg_puts_attr(const char *const s, const int attr)
{
msg_puts_attr_len(s, -1, attr);
}
void msg_puts_attr_len(const char *const str, const ptrdiff_t len, int attr)
FUNC_ATTR_NONNULL_ALL
{
assert(len < 0 || memchr(str, 0, len) == NULL);
redir_write(str, len);
if (msg_silent != 0) {
return;
}
if (attr & MSG_HIST) {
add_msg_hist(str, (int)len, attr, false);
attr &= ~MSG_HIST;
}
bool overflow = false;
if (ui_has(kUIMessages)) {
int count = msg_ext_visible + (msg_ext_overwrite ? 0 : 1);
if (count > 1) {
overflow = true;
}
} else {
overflow = msg_scrolled != 0;
}
if (overflow && !msg_scrolled_ign) {
need_wait_return = true;
}
msg_didany = true; 
if (msg_use_printf()) {
int saved_msg_col = msg_col;
msg_puts_printf(str, len);
if (headless_mode) {
msg_col = saved_msg_col;
}
}
if (!msg_use_printf() || (headless_mode && default_grid.chars)) {
msg_puts_display((const char_u *)str, len, attr, false);
}
}
void msg_printf_attr(const int attr, const char *const fmt, ...)
FUNC_ATTR_NONNULL_ARG(2) FUNC_ATTR_PRINTF(2, 3)
{
static char msgbuf[IOSIZE];
va_list ap;
va_start(ap, fmt);
const size_t len = vim_vsnprintf(msgbuf, sizeof(msgbuf), fmt, ap);
va_end(ap);
msg_scroll = true;
msg_puts_attr_len(msgbuf, (ptrdiff_t)len, attr);
}
static void msg_ext_emit_chunk(void)
{
if (msg_ext_last_attr == -1) {
return; 
}
Array chunk = ARRAY_DICT_INIT;
ADD(chunk, INTEGER_OBJ(msg_ext_last_attr));
msg_ext_last_attr = -1;
String text = ga_take_string(&msg_ext_last_chunk);
ADD(chunk, STRING_OBJ(text));
ADD(msg_ext_chunks, ARRAY_OBJ(chunk));
}
static void msg_puts_display(const char_u *str, int maxlen, int attr,
int recurse)
{
const char_u *s = str;
const char_u *t_s = str; 
int t_col = 0; 
int l;
int cw;
const char_u *sb_str = str;
int sb_col = msg_col;
int wrap;
int did_last_char;
did_wait_return = false;
if (ui_has(kUIMessages)) {
if (attr != msg_ext_last_attr) {
msg_ext_emit_chunk();
msg_ext_last_attr = attr;
}
size_t len = strnlen((char *)str, maxlen); 
ga_concat_len(&msg_ext_last_chunk, (char *)str, len);
msg_ext_cur_len += len;
return;
}
msg_grid_validate();
cmdline_was_last_drawn = redrawing_cmdline;
while ((maxlen < 0 || (int)(s - str) < maxlen) && *s != NUL) {
if (!recurse && msg_row >= Rows - 1
&& (*s == '\n' || (cmdmsg_rl
? (msg_col <= 1
|| (*s == TAB && msg_col <= 7)
|| (utf_ptr2cells(s) > 1
&& msg_col <= 2))
: (msg_col + t_col >= Columns - 1
|| (*s == TAB
&& msg_col + t_col >= ((Columns - 1) & ~7))
|| (utf_ptr2cells(s) > 1
&& msg_col + t_col >= Columns - 2))))) {
if (t_col > 0) {
t_puts(&t_col, t_s, s, attr);
}
if (msg_no_more && lines_left == 0)
break;
bool has_last_char = (*s >= ' ' && !cmdmsg_rl);
msg_scroll_up(!has_last_char);
msg_row = Rows - 2;
if (msg_col >= Columns) 
msg_col = Columns - 1;
if (has_last_char) {
if (maxlen >= 0) {
l = utfc_ptr2len_len(s, (int)((str + maxlen) - s));
} else {
l = utfc_ptr2len(s);
}
s = screen_puts_mbyte((char_u *)s, l, attr);
did_last_char = true;
} else {
did_last_char = false;
}
if (has_last_char && msg_do_throttle()) {
if (!msg_grid.throttled) {
msg_grid_scroll_discount++;
}
msg_grid.throttled = true;
}
if (p_more) {
store_sb_text((char_u **)&sb_str, (char_u *)s, attr, &sb_col, true);
}
inc_msg_scrolled();
need_wait_return = true; 
redraw_cmdline = true;
if (cmdline_row > 0 && !exmode_active) {
cmdline_row--;
}
if (lines_left > 0)
--lines_left;
if (p_more && lines_left == 0 && State != HITRETURN
&& !msg_no_more && !exmode_active) {
if (do_more_prompt(NUL))
s = confirm_msg_tail;
if (quit_more)
return;
}
if (did_last_char)
continue;
}
wrap = *s == '\n'
|| msg_col + t_col >= Columns
|| (utf_ptr2cells(s) > 1
&& msg_col + t_col >= Columns - 1)
;
if (t_col > 0 && (wrap || *s == '\r' || *s == '\b'
|| *s == '\t' || *s == BELL)) {
t_puts(&t_col, t_s, s, attr);
}
if (wrap && p_more && !recurse) {
store_sb_text((char_u **)&sb_str, (char_u *)s, attr, &sb_col, true);
}
if (*s == '\n') { 
msg_didout = FALSE; 
if (cmdmsg_rl)
msg_col = Columns - 1;
else
msg_col = 0;
if (++msg_row >= Rows) 
msg_row = Rows - 1;
} else if (*s == '\r') { 
msg_col = 0;
} else if (*s == '\b') { 
if (msg_col)
--msg_col;
} else if (*s == TAB) { 
do {
msg_screen_putchar(' ', attr);
} while (msg_col & 7);
} else if (*s == BELL) { 
vim_beep(BO_SH);
} else if (*s >= 0x20) { 
cw = utf_ptr2cells(s);
if (maxlen >= 0) {
l = utfc_ptr2len_len(s, (int)((str + maxlen) - s));
} else {
l = utfc_ptr2len(s);
}
if (cmdmsg_rl || (cw > 1 && msg_col + t_col >= Columns - 1)) {
if (l > 1) {
s = screen_puts_mbyte((char_u *)s, l, attr) - 1;
} else {
msg_screen_putchar(*s, attr);
}
} else {
if (t_col == 0)
t_s = s;
t_col += cw;
s += l - 1;
}
}
++s;
}
if (t_col > 0) {
t_puts(&t_col, t_s, s, attr);
}
if (p_more && !recurse) {
store_sb_text((char_u **)&sb_str, (char_u *)s, attr, &sb_col, false);
}
msg_check();
}
bool message_filtered(char_u *msg)
{
if (cmdmod.filter_regmatch.regprog == NULL) {
return false;
}
bool match = vim_regexec(&cmdmod.filter_regmatch, msg, (colnr_T)0);
return cmdmod.filter_force ? match : !match;
}
int msg_scrollsize(void)
{
return msg_scrolled + p_ch + 1;
}
bool msg_use_msgsep(void)
{
return ((dy_flags & DY_MSGSEP) || ui_has(kUIMultigrid));
}
bool msg_do_throttle(void)
{
return msg_use_grid() && !(rdb_flags & RDB_NOTHROTTLE);
}
void msg_scroll_up(bool may_throttle)
{
if (may_throttle && msg_do_throttle()) {
msg_grid.throttled = true;
}
msg_did_scroll = true;
if (msg_use_msgsep()) {
if (msg_grid_pos > 0) {
msg_grid_set_pos(msg_grid_pos-1, true);
} else {
grid_del_lines(&msg_grid, 0, 1, msg_grid.Rows, 0, msg_grid.Columns);
memmove(msg_grid.dirty_col, msg_grid.dirty_col+1,
(msg_grid.Rows-1) * sizeof(*msg_grid.dirty_col));
msg_grid.dirty_col[msg_grid.Rows-1] = 0;
}
} else {
grid_del_lines(&msg_grid_adj, 0, 1, Rows, 0, Columns);
}
grid_fill(&msg_grid_adj, Rows-1, Rows, 0, Columns, ' ', ' ',
HL_ATTR(HLF_MSG));
}
void msg_scroll_flush(void)
{
if (msg_grid.throttled) {
msg_grid.throttled = false;
int pos_delta = msg_grid_pos_at_flush - msg_grid_pos;
assert(pos_delta >= 0);
int delta = MIN(msg_scrolled - msg_scrolled_at_flush, msg_grid.Rows);
if (pos_delta > 0) {
ui_ext_msg_set_pos(msg_grid_pos, true);
}
int to_scroll = delta-pos_delta-msg_grid_scroll_discount;
assert(to_scroll >= 0);
if (to_scroll > 0 && msg_grid_pos == 0) {
ui_call_grid_scroll(msg_grid.handle, 0, Rows, 0, Columns, to_scroll, 0);
}
for (int i = MAX(Rows-MAX(delta, 1), 0); i < Rows; i++) {
int row = i-msg_grid_pos;
assert(row >= 0);
ui_line(&msg_grid, row, 0, msg_grid.dirty_col[row], msg_grid.Columns,
HL_ATTR(HLF_MSG), false);
msg_grid.dirty_col[row] = 0;
}
}
msg_scrolled_at_flush = msg_scrolled;
msg_grid_scroll_discount = 0;
msg_grid_pos_at_flush = msg_grid_pos;
}
void msg_reset_scroll(void)
{
if (ui_has(kUIMessages)) {
msg_ext_clear(true);
return;
}
if (msg_use_grid()) {
msg_grid.throttled = false;
msg_grid_set_pos(Rows - p_ch, false);
clear_cmdline = true;
if (msg_grid.chars) {
for (int i = 0; i < MIN(msg_scrollsize(), msg_grid.Rows); i++) {
grid_clear_line(&msg_grid, msg_grid.line_offset[i],
(int)msg_grid.Columns, false);
}
}
} else {
redraw_all_later(NOT_VALID);
}
msg_scrolled = 0;
msg_scrolled_at_flush = 0;
}
static void inc_msg_scrolled(void)
{
if (*get_vim_var_str(VV_SCROLLSTART) == NUL) {
char *p = (char *) sourcing_name;
char *tofree = NULL;
if (p == NULL) {
p = _("Unknown");
} else {
size_t len = strlen(p) + 40;
tofree = xmalloc(len);
vim_snprintf(tofree, len, _("%s line %" PRId64),
p, (int64_t) sourcing_lnum);
p = tofree;
}
set_vim_var_string(VV_SCROLLSTART, p, -1);
xfree(tofree);
}
msg_scrolled++;
if (must_redraw < VALID) {
must_redraw = VALID;
}
}
static msgchunk_T *last_msgchunk = NULL; 
typedef enum {
SB_CLEAR_NONE = 0,
SB_CLEAR_ALL,
SB_CLEAR_CMDLINE_BUSY,
SB_CLEAR_CMDLINE_DONE
} sb_clear_T;
static sb_clear_T do_clear_sb_text = SB_CLEAR_NONE;
static void store_sb_text(
char_u **sb_str, 
char_u *s, 
int attr,
int *sb_col,
int finish 
)
{
msgchunk_T *mp;
if (do_clear_sb_text == SB_CLEAR_ALL
|| do_clear_sb_text == SB_CLEAR_CMDLINE_DONE) {
clear_sb_text(do_clear_sb_text == SB_CLEAR_ALL);
do_clear_sb_text = SB_CLEAR_NONE;
}
if (s > *sb_str) {
mp = xmalloc((sizeof(msgchunk_T) + (s - *sb_str)));
mp->sb_eol = finish;
mp->sb_msg_col = *sb_col;
mp->sb_attr = attr;
memcpy(mp->sb_text, *sb_str, s - *sb_str);
mp->sb_text[s - *sb_str] = NUL;
if (last_msgchunk == NULL) {
last_msgchunk = mp;
mp->sb_prev = NULL;
} else {
mp->sb_prev = last_msgchunk;
last_msgchunk->sb_next = mp;
last_msgchunk = mp;
}
mp->sb_next = NULL;
} else if (finish && last_msgchunk != NULL)
last_msgchunk->sb_eol = TRUE;
*sb_str = s;
*sb_col = 0;
}
void may_clear_sb_text(void)
{
do_clear_sb_text = SB_CLEAR_ALL;
}
void sb_text_start_cmdline(void)
{
do_clear_sb_text = SB_CLEAR_CMDLINE_BUSY;
msg_sb_eol();
}
void sb_text_end_cmdline(void)
{
do_clear_sb_text = SB_CLEAR_CMDLINE_DONE;
}
void clear_sb_text(int all)
{
msgchunk_T *mp;
msgchunk_T **lastp;
if (all) {
lastp = &last_msgchunk;
} else {
if (last_msgchunk == NULL) {
return;
}
lastp = &last_msgchunk->sb_prev;
}
while (*lastp != NULL) {
mp = (*lastp)->sb_prev;
xfree(*lastp);
*lastp = mp;
}
}
void show_sb_text(void)
{
msgchunk_T *mp;
mp = msg_sb_start(last_msgchunk);
if (mp == NULL || mp->sb_prev == NULL) {
vim_beep(BO_MESS);
} else {
do_more_prompt('G');
wait_return(FALSE);
}
}
static msgchunk_T *msg_sb_start(msgchunk_T *mps)
{
msgchunk_T *mp = mps;
while (mp != NULL && mp->sb_prev != NULL && !mp->sb_prev->sb_eol)
mp = mp->sb_prev;
return mp;
}
void msg_sb_eol(void)
{
if (last_msgchunk != NULL)
last_msgchunk->sb_eol = TRUE;
}
static msgchunk_T *disp_sb_line(int row, msgchunk_T *smp)
{
msgchunk_T *mp = smp;
char_u *p;
for (;; ) {
msg_row = row;
msg_col = mp->sb_msg_col;
p = mp->sb_text;
if (*p == '\n') 
++p;
msg_puts_display(p, -1, mp->sb_attr, TRUE);
if (mp->sb_eol || mp->sb_next == NULL)
break;
mp = mp->sb_next;
}
return mp->sb_next;
}
static void t_puts(int *t_col, const char_u *t_s, const char_u *s, int attr)
{
attr = hl_combine_attr(HL_ATTR(HLF_MSG), attr);
msg_didout = true; 
grid_puts_len(&msg_grid_adj, (char_u *)t_s, (int)(s - t_s), msg_row, msg_col,
attr);
msg_col += *t_col;
*t_col = 0;
if (enc_utf8 && utf_iscomposing(utf_ptr2char(t_s)))
--msg_col;
if (msg_col >= Columns) {
msg_col = 0;
++msg_row;
}
}
int msg_use_printf(void)
{
return !embedded_mode && !ui_active();
}
static void msg_puts_printf(const char *str, const ptrdiff_t maxlen)
{
const char *s = str;
char buf[7];
char *p;
while ((maxlen < 0 || s - str < maxlen) && *s != NUL) {
int len = utf_ptr2len((const char_u *)s);
if (!(silent_mode && p_verbose == 0)) {
p = &buf[0];
if (*s == '\n' && !info_message) {
*p++ = '\r';
}
memcpy(p, s, len);
*(p + len) = '\0';
if (info_message) {
mch_msg(buf);
} else {
mch_errmsg(buf);
}
}
int cw = utf_char2cells(utf_ptr2char((const char_u *)s));
if (cmdmsg_rl) {
if (*s == '\r' || *s == '\n') {
msg_col = Columns - 1;
} else {
msg_col -= cw;
}
} else {
if (*s == '\r' || *s == '\n') {
msg_col = 0;
} else {
msg_col += cw;
}
}
s += len;
}
msg_didout = true; 
}
static int do_more_prompt(int typed_char)
{
static bool entered = false;
int used_typed_char = typed_char;
int oldState = State;
int c;
int retval = FALSE;
int toscroll;
bool to_redraw = false;
msgchunk_T *mp_last = NULL;
msgchunk_T *mp;
int i;
if (entered || (State == HITRETURN && typed_char == 0)) {
return false;
}
entered = true;
if (typed_char == 'G') {
mp_last = msg_sb_start(last_msgchunk);
for (i = 0; i < Rows - 2 && mp_last != NULL
&& mp_last->sb_prev != NULL; ++i)
mp_last = msg_sb_start(mp_last->sb_prev);
}
State = ASKMORE;
setmouse();
if (typed_char == NUL)
msg_moremsg(FALSE);
for (;; ) {
if (used_typed_char != NUL) {
c = used_typed_char; 
used_typed_char = NUL;
} else {
c = get_keystroke(resize_events);
}
toscroll = 0;
switch (c) {
case BS: 
case K_BS:
case 'k':
case K_UP:
toscroll = -1;
break;
case CAR: 
case NL:
case 'j':
case K_DOWN:
toscroll = 1;
break;
case 'u': 
toscroll = -(Rows / 2);
break;
case 'd': 
toscroll = Rows / 2;
break;
case 'b': 
case K_PAGEUP:
toscroll = -(Rows - 1);
break;
case ' ': 
case 'f':
case K_PAGEDOWN:
case K_LEFTMOUSE:
toscroll = Rows - 1;
break;
case 'g': 
toscroll = -999999;
break;
case 'G': 
toscroll = 999999;
lines_left = 999999;
break;
case ':': 
if (!confirm_msg_used) {
typeahead_noflush(':');
cmdline_row = Rows - 1; 
skip_redraw = TRUE; 
need_wait_return = FALSE; 
}
FALLTHROUGH;
case 'q': 
case Ctrl_C:
case ESC:
if (confirm_msg_used) {
retval = TRUE;
} else {
got_int = TRUE;
quit_more = TRUE;
}
lines_left = Rows - 1;
break;
case K_EVENT:
multiqueue_process_events(resize_events);
to_redraw = true;
break;
default: 
msg_moremsg(TRUE);
continue;
}
assert((toscroll == 0) || !to_redraw);
if (toscroll != 0 || to_redraw) {
if (toscroll < 0 || to_redraw) {
if (mp_last == NULL) {
mp = msg_sb_start(last_msgchunk);
} else if (mp_last->sb_prev != NULL) {
mp = msg_sb_start(mp_last->sb_prev);
} else {
mp = NULL;
}
for (i = 0; i < Rows - 2 && mp != NULL && mp->sb_prev != NULL;
++i)
mp = msg_sb_start(mp->sb_prev);
if (mp != NULL && (mp->sb_prev != NULL || to_redraw)) {
for (i = 0; i > toscroll; i--) {
if (mp == NULL || mp->sb_prev == NULL) {
break;
}
mp = msg_sb_start(mp->sb_prev);
if (mp_last == NULL)
mp_last = msg_sb_start(last_msgchunk);
else
mp_last = msg_sb_start(mp_last->sb_prev);
}
if (toscroll == -1 && !to_redraw) {
grid_ins_lines(&msg_grid_adj, 0, 1, Rows, 0, Columns);
grid_fill(&msg_grid_adj, 0, 1, 0, Columns, ' ', ' ',
HL_ATTR(HLF_MSG));
(void)disp_sb_line(0, mp);
} else {
grid_fill(&msg_grid_adj, 0, Rows, 0, Columns, ' ', ' ',
HL_ATTR(HLF_MSG));
for (i = 0; mp != NULL && i < Rows - 1; i++) {
mp = disp_sb_line(i, mp);
++msg_scrolled;
}
to_redraw = false;
}
toscroll = 0;
}
} else {
while (toscroll > 0 && mp_last != NULL) {
if (msg_do_throttle() && !msg_grid.throttled) {
msg_scrolled_at_flush--;
msg_grid_scroll_discount++;
}
msg_scroll_up(true);
inc_msg_scrolled();
grid_fill(&msg_grid_adj, Rows-2, Rows-1, 0, Columns, ' ', ' ',
HL_ATTR(HLF_MSG));
mp_last = disp_sb_line(Rows - 2, mp_last);
toscroll--;
}
}
if (toscroll <= 0) {
grid_fill(&msg_grid_adj, Rows - 1, Rows, 0, Columns, ' ', ' ',
HL_ATTR(HLF_MSG));
msg_moremsg(false);
continue;
}
lines_left = toscroll;
}
break;
}
grid_fill(&msg_grid_adj, Rows - 1, Rows, 0, Columns, ' ', ' ',
HL_ATTR(HLF_MSG));
redraw_cmdline = true;
clear_cmdline = false;
mode_displayed = false;
State = oldState;
setmouse();
if (quit_more) {
msg_row = Rows - 1;
msg_col = 0;
} else if (cmdmsg_rl) {
msg_col = Columns - 1;
}
entered = false;
return retval;
}
#if defined(WIN32)
void mch_errmsg(char *str)
{
assert(str != NULL);
wchar_t *utf16str;
int r = utf8_to_utf16(str, -1, &utf16str);
if (r != 0) {
fprintf(stderr, "utf8_to_utf16 failed: %d", r);
} else {
fwprintf(stderr, L"%ls", utf16str);
xfree(utf16str);
}
}
void mch_msg(char *str)
{
assert(str != NULL);
wchar_t *utf16str;
int r = utf8_to_utf16(str, -1, &utf16str);
if (r != 0) {
fprintf(stderr, "utf8_to_utf16 failed: %d", r);
} else {
wprintf(L"%ls", utf16str);
xfree(utf16str);
}
}
#endif 
static void msg_screen_putchar(int c, int attr)
{
attr = hl_combine_attr(HL_ATTR(HLF_MSG), attr);
msg_didout = true; 
grid_putchar(&msg_grid_adj, c, msg_row, msg_col, attr);
if (cmdmsg_rl) {
if (--msg_col == 0) {
msg_col = Columns;
++msg_row;
}
} else {
if (++msg_col >= Columns) {
msg_col = 0;
++msg_row;
}
}
}
void msg_moremsg(int full)
{
int attr;
char_u *s = (char_u *)_("-- More --");
attr = hl_combine_attr(HL_ATTR(HLF_MSG), HL_ATTR(HLF_M));
grid_puts(&msg_grid_adj, s, Rows - 1, 0, attr);
if (full) {
grid_puts(&msg_grid_adj, (char_u *)
_(" SPACE/d/j: screen/page/line down, b/u/k: up, q: quit "),
Rows - 1, vim_strsize(s), attr);
}
}
void repeat_message(void)
{
if (State == ASKMORE) {
msg_moremsg(TRUE); 
msg_row = Rows - 1;
} else if (State == CONFIRM) {
display_confirm_msg(); 
msg_row = Rows - 1;
} else if (State == EXTERNCMD) {
ui_cursor_goto(msg_row, msg_col); 
} else if (State == HITRETURN || State == SETWSIZE) {
if (msg_row == Rows - 1) {
msg_didout = FALSE;
msg_col = 0;
msg_clr_eos();
}
hit_return_msg();
msg_row = Rows - 1;
}
}
void msg_clr_eos(void)
{
if (msg_silent == 0)
msg_clr_eos_force();
}
void msg_clr_eos_force(void)
{
if (ui_has(kUIMessages)) {
return;
}
int msg_startcol = (cmdmsg_rl) ? 0 : msg_col;
int msg_endcol = (cmdmsg_rl) ? msg_col + 1 : Columns;
if (msg_grid.chars && msg_row < msg_grid_pos) {
msg_row = msg_grid_pos;
}
grid_fill(&msg_grid_adj, msg_row, msg_row + 1, msg_startcol, msg_endcol, ' ',
' ', HL_ATTR(HLF_MSG));
grid_fill(&msg_grid_adj, msg_row + 1, Rows, 0, Columns, ' ', ' ',
HL_ATTR(HLF_MSG));
redraw_cmdline = true; 
if (msg_row < Rows-1 || msg_col == (cmdmsg_rl ? Columns : 0)) {
clear_cmdline = false; 
mode_displayed = false; 
}
}
void msg_clr_cmdline(void)
{
msg_row = cmdline_row;
msg_col = 0;
msg_clr_eos_force();
}
int msg_end(void)
{
if (!exiting && need_wait_return && !(State & CMDLINE)) {
wait_return(FALSE);
return FALSE;
}
msg_ext_ui_flush();
return true;
}
void msg_ext_ui_flush(void)
{
if (!ui_has(kUIMessages)) {
return;
}
msg_ext_emit_chunk();
if (msg_ext_chunks.size > 0) {
ui_call_msg_show(cstr_to_string(msg_ext_kind),
msg_ext_chunks, msg_ext_overwrite);
if (!msg_ext_overwrite) {
msg_ext_visible++;
}
msg_ext_kind = NULL;
msg_ext_chunks = (Array)ARRAY_DICT_INIT;
msg_ext_cur_len = 0;
msg_ext_overwrite = false;
}
}
void msg_ext_flush_showmode(void)
{
if (ui_has(kUIMessages)) {
msg_ext_emit_chunk();
ui_call_msg_showmode(msg_ext_chunks);
msg_ext_chunks = (Array)ARRAY_DICT_INIT;
msg_ext_cur_len = 0;
}
}
void msg_ext_clear(bool force)
{
if (msg_ext_visible && (!msg_ext_keep_after_cmdline || force)) {
ui_call_msg_clear();
msg_ext_visible = 0;
msg_ext_overwrite = false; 
}
msg_ext_keep_after_cmdline = false;
}
void msg_ext_clear_later(void)
{
if (msg_ext_is_visible()) {
msg_ext_need_clear = true;
if (must_redraw < VALID) {
must_redraw = VALID;
}
}
}
void msg_ext_check_clear(void)
{
if (msg_ext_need_clear) {
msg_ext_clear(true);
msg_ext_need_clear = false;
}
}
bool msg_ext_is_visible(void)
{
return ui_has(kUIMessages) && msg_ext_visible > 0;
}
void msg_check(void)
{
if (ui_has(kUIMessages)) {
return;
}
if (msg_row == Rows - 1 && msg_col >= sc_col) {
need_wait_return = TRUE;
redraw_cmdline = TRUE;
}
}
static void redir_write(const char *const str, const ptrdiff_t maxlen)
{
const char_u *s = (char_u *)str;
static int cur_col = 0;
if (maxlen == 0) {
return;
}
if (redir_off)
return;
if (*p_vfile != NUL && verbose_fd == NULL)
verbose_open();
if (redirecting()) {
if (*s != '\n' && *s != '\r') {
while (cur_col < msg_col) {
if (capture_ga) {
ga_concat_len(capture_ga, " ", 1);
}
if (redir_reg) {
write_reg_contents(redir_reg, (char_u *)" ", 1, true);
} else if (redir_vname) {
var_redir_str((char_u *)" ", -1);
} else if (redir_fd != NULL) {
fputs(" ", redir_fd);
}
if (verbose_fd != NULL) {
fputs(" ", verbose_fd);
}
cur_col++;
}
}
size_t len = maxlen == -1 ? STRLEN(s) : (size_t)maxlen;
if (capture_ga) {
ga_concat_len(capture_ga, (const char *)str, len);
}
if (redir_reg) {
write_reg_contents(redir_reg, s, len, true);
}
if (redir_vname) {
var_redir_str((char_u *)s, maxlen);
}
while (*s != NUL
&& (maxlen < 0 || (int)(s - (const char_u *)str) < maxlen)) {
if (!redir_reg && !redir_vname && !capture_ga) {
if (redir_fd != NULL) {
putc(*s, redir_fd);
}
}
if (verbose_fd != NULL) {
putc(*s, verbose_fd);
}
if (*s == '\r' || *s == '\n') {
cur_col = 0;
} else if (*s == '\t') {
cur_col += (8 - cur_col % 8);
} else {
cur_col++;
}
s++;
}
if (msg_silent != 0) 
msg_col = cur_col;
}
}
int redirecting(void)
{
return redir_fd != NULL || *p_vfile != NUL
|| redir_reg || redir_vname || capture_ga != NULL;
}
void verbose_enter(void)
{
if (*p_vfile != NUL)
++msg_silent;
}
void verbose_leave(void)
{
if (*p_vfile != NUL)
if (--msg_silent < 0)
msg_silent = 0;
}
void verbose_enter_scroll(void)
{
if (*p_vfile != NUL)
++msg_silent;
else
msg_scroll = TRUE;
}
void verbose_leave_scroll(void)
{
if (*p_vfile != NUL) {
if (--msg_silent < 0)
msg_silent = 0;
} else
cmdline_row = msg_row;
}
void verbose_stop(void)
{
if (verbose_fd != NULL) {
fclose(verbose_fd);
verbose_fd = NULL;
}
verbose_did_open = FALSE;
}
int verbose_open(void)
{
if (verbose_fd == NULL && !verbose_did_open) {
verbose_did_open = TRUE;
verbose_fd = os_fopen((char *)p_vfile, "a");
if (verbose_fd == NULL) {
EMSG2(_(e_notopen), p_vfile);
return FAIL;
}
}
return OK;
}
void give_warning(char_u *message, bool hl) FUNC_ATTR_NONNULL_ARG(1)
{
if (msg_silent != 0) {
return;
}
no_wait_return++;
set_vim_var_string(VV_WARNINGMSG, (char *)message, -1);
XFREE_CLEAR(keep_msg);
if (hl) {
keep_msg_attr = HL_ATTR(HLF_W);
} else {
keep_msg_attr = 0;
}
if (msg_ext_kind == NULL) {
msg_ext_set_kind("wmsg");
}
if (msg_attr((const char *)message, keep_msg_attr) && msg_scrolled == 0) {
set_keep_msg(message, keep_msg_attr);
}
msg_didout = false; 
msg_nowait = true; 
msg_col = 0;
no_wait_return--;
}
void give_warning2(char_u *const message, char_u *const a1, bool hl)
{
vim_snprintf((char *)IObuff, IOSIZE, (char *)message, a1);
give_warning(IObuff, hl);
}
void msg_advance(int col)
{
if (msg_silent != 0) { 
msg_col = col; 
return;
}
if (ui_has(kUIMessages)) {
while (msg_ext_cur_len < (size_t)col) {
msg_putchar(' ');
}
return;
}
if (col >= Columns) 
col = Columns - 1;
if (cmdmsg_rl)
while (msg_col > Columns - col)
msg_putchar(' ');
else
while (msg_col < col)
msg_putchar(' ');
}
int
do_dialog (
int type,
char_u *title,
char_u *message,
char_u *buttons,
int dfltbutton,
char_u *textfield, 
int ex_cmd 
)
{
int retval = 0;
char_u *hotkeys;
int c;
int i;
if (silent_mode 
|| !ui_active() 
) {
return dfltbutton; 
}
int save_msg_silent = msg_silent;
int oldState = State;
msg_silent = 0; 
State = CONFIRM;
setmouse();
++no_wait_return;
hotkeys = msg_show_console_dialog(message, buttons, dfltbutton);
for (;; ) {
c = get_keystroke(NULL);
switch (c) {
case CAR: 
case NL:
retval = dfltbutton;
break;
case Ctrl_C: 
case ESC:
retval = 0;
break;
default: 
if (c < 0) { 
continue;
}
if (c == ':' && ex_cmd) {
retval = dfltbutton;
ins_char_typebuf(':');
break;
}
c = mb_tolower(c);
retval = 1;
for (i = 0; hotkeys[i]; i++) {
if (utf_ptr2char(hotkeys + i) == c) {
break;
}
i += utfc_ptr2len(hotkeys + i) - 1;
retval++;
}
if (hotkeys[i])
break;
continue;
}
break;
}
xfree(hotkeys);
msg_silent = save_msg_silent;
State = oldState;
setmouse();
--no_wait_return;
msg_end_prompt();
return retval;
}
static int
copy_char (
char_u *from,
char_u *to,
int lowercase 
)
{
if (lowercase) {
int c = mb_tolower(utf_ptr2char(from));
return utf_char2bytes(c, to);
}
int len = utfc_ptr2len(from);
memmove(to, from, (size_t)len);
return len;
}
#define HAS_HOTKEY_LEN 30
#define HOTK_LEN (has_mbyte ? MB_MAXBYTES : 1)
static char_u * console_dialog_alloc(const char_u *message,
char_u *buttons,
bool has_hotkey[])
{
int lenhotkey = HOTK_LEN; 
has_hotkey[0] = false;
int len = 0;
int idx = 0;
char_u *r = buttons;
while (*r) {
if (*r == DLG_BUTTON_SEP) {
len += 3; 
lenhotkey += HOTK_LEN; 
if (idx < HAS_HOTKEY_LEN - 1) {
has_hotkey[++idx] = false;
}
} else if (*r == DLG_HOTKEY_CHAR) {
r++;
len++; 
if (idx < HAS_HOTKEY_LEN - 1) {
has_hotkey[idx] = true;
}
}
MB_PTR_ADV(r);
}
len += (int)(STRLEN(message)
+ 2 
+ STRLEN(buttons)
+ 3); 
lenhotkey++; 
if (!has_hotkey[0]) {
len += 2; 
}
xfree(confirm_msg);
confirm_msg = xmalloc(len);
*confirm_msg = NUL;
return xmalloc(lenhotkey);
}
static char_u *msg_show_console_dialog(char_u *message, char_u *buttons, int dfltbutton)
FUNC_ATTR_NONNULL_RET
{
bool has_hotkey[HAS_HOTKEY_LEN] = {false};
char_u *hotk = console_dialog_alloc(message, buttons, has_hotkey);
copy_hotkeys_and_msg(message, buttons, dfltbutton, has_hotkey, hotk);
display_confirm_msg();
return hotk;
}
static void copy_hotkeys_and_msg(const char_u *message, char_u *buttons,
int default_button_idx, const bool has_hotkey[],
char_u *hotkeys_ptr)
{
*confirm_msg = '\n';
STRCPY(confirm_msg + 1, message);
char_u *msgp = confirm_msg + 1 + STRLEN(message);
hotkeys_ptr[copy_char(buttons, hotkeys_ptr, TRUE)] = NUL;
confirm_msg_tail = msgp;
*msgp++ = '\n';
bool first_hotkey = false; 
if (!has_hotkey[0]) {
first_hotkey = true; 
}
int idx = 0;
char_u *r = buttons;
while (*r) {
if (*r == DLG_BUTTON_SEP) {
*msgp++ = ',';
*msgp++ = ' '; 
hotkeys_ptr += (has_mbyte) ? STRLEN(hotkeys_ptr): 1;
hotkeys_ptr[copy_char(r + 1, hotkeys_ptr, TRUE)] = NUL;
if (default_button_idx) {
default_button_idx--;
}
if (idx < HAS_HOTKEY_LEN - 1 && !has_hotkey[++idx]) {
first_hotkey = true;
}
} else if (*r == DLG_HOTKEY_CHAR || first_hotkey) {
if (*r == DLG_HOTKEY_CHAR) {
++r;
}
first_hotkey = false;
if (*r == DLG_HOTKEY_CHAR) { 
*msgp++ = *r;
} else {
*msgp++ = (default_button_idx == 1) ? '[' : '(';
msgp += copy_char(r, msgp, FALSE);
*msgp++ = (default_button_idx == 1) ? ']' : ')';
hotkeys_ptr[copy_char(r, hotkeys_ptr, TRUE)] = NUL;
}
} else {
msgp += copy_char(r, msgp, FALSE);
}
MB_PTR_ADV(r);
}
*msgp++ = ':';
*msgp++ = ' ';
*msgp = NUL;
}
void display_confirm_msg(void)
{
confirm_msg_used++;
if (confirm_msg != NULL) {
msg_ext_set_kind("confirm");
msg_puts_attr((const char *)confirm_msg, HL_ATTR(HLF_M));
}
confirm_msg_used--;
}
int vim_dialog_yesno(int type, char_u *title, char_u *message, int dflt)
{
if (do_dialog(type,
title == NULL ? (char_u *)_("Question") : title,
message,
(char_u *)_("&Yes\n&No"), dflt, NULL, FALSE) == 1)
return VIM_YES;
return VIM_NO;
}
int vim_dialog_yesnocancel(int type, char_u *title, char_u *message, int dflt)
{
switch (do_dialog(type,
title == NULL ? (char_u *)_("Question") : title,
message,
(char_u *)_("&Yes\n&No\n&Cancel"), dflt, NULL, FALSE)) {
case 1: return VIM_YES;
case 2: return VIM_NO;
}
return VIM_CANCEL;
}
int vim_dialog_yesnoallcancel(int type, char_u *title, char_u *message, int dflt)
{
switch (do_dialog(type,
title == NULL ? (char_u *)"Question" : title,
message,
(char_u *)_("&Yes\n&No\nSave &All\n&Discard All\n&Cancel"),
dflt, NULL, FALSE)) {
case 1: return VIM_YES;
case 2: return VIM_NO;
case 3: return VIM_ALL;
case 4: return VIM_DISCARDALL;
}
return VIM_CANCEL;
}
