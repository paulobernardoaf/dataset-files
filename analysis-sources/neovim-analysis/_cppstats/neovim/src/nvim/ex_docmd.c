#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <inttypes.h>
#include "nvim/vim.h"
#include "nvim/ascii.h"
#include "nvim/ex_docmd.h"
#include "nvim/buffer.h"
#include "nvim/change.h"
#include "nvim/charset.h"
#include "nvim/cursor.h"
#include "nvim/diff.h"
#include "nvim/digraph.h"
#include "nvim/edit.h"
#include "nvim/eval.h"
#include "nvim/ex_cmds.h"
#include "nvim/ex_cmds2.h"
#include "nvim/ex_eval.h"
#include "nvim/ex_getln.h"
#include "nvim/fileio.h"
#include "nvim/fold.h"
#include "nvim/func_attr.h"
#include "nvim/getchar.h"
#include "nvim/hardcopy.h"
#include "nvim/if_cscope.h"
#include "nvim/main.h"
#include "nvim/mark.h"
#include "nvim/mbyte.h"
#include "nvim/memline.h"
#include "nvim/memory.h"
#include "nvim/menu.h"
#include "nvim/message.h"
#include "nvim/misc1.h"
#include "nvim/ex_session.h"
#include "nvim/keymap.h"
#include "nvim/file_search.h"
#include "nvim/garray.h"
#include "nvim/move.h"
#include "nvim/normal.h"
#include "nvim/ops.h"
#include "nvim/option.h"
#include "nvim/os_unix.h"
#include "nvim/path.h"
#include "nvim/quickfix.h"
#include "nvim/regexp.h"
#include "nvim/screen.h"
#include "nvim/search.h"
#include "nvim/sign.h"
#include "nvim/spell.h"
#include "nvim/spellfile.h"
#include "nvim/strings.h"
#include "nvim/syntax.h"
#include "nvim/tag.h"
#include "nvim/terminal.h"
#include "nvim/ui.h"
#include "nvim/undo.h"
#include "nvim/version.h"
#include "nvim/window.h"
#include "nvim/os/os.h"
#include "nvim/os/input.h"
#include "nvim/os/time.h"
#include "nvim/ex_cmds_defs.h"
#include "nvim/mouse.h"
#include "nvim/event/rstream.h"
#include "nvim/event/wstream.h"
#include "nvim/shada.h"
#include "nvim/lua/executor.h"
#include "nvim/globals.h"
#include "nvim/api/private/helpers.h"
static int quitmore = 0;
static bool ex_pressedreturn = false;
typedef struct ucmd {
char_u *uc_name; 
uint32_t uc_argt; 
char_u *uc_rep; 
long uc_def; 
int uc_compl; 
int uc_addr_type; 
sctx_T uc_script_ctx; 
char_u *uc_compl_arg; 
} ucmd_T;
#define UC_BUFFER 1 
static garray_T ucmds = {0, 0, sizeof(ucmd_T), 4, NULL};
#define USER_CMD(i) (&((ucmd_T *)(ucmds.ga_data))[i])
#define USER_CMD_GA(gap, i) (&((ucmd_T *)((gap)->ga_data))[i])
#define IS_USER_CMDIDX(idx) ((int)(idx) < 0)
typedef struct {
char_u *line; 
linenr_T lnum; 
} wcmd_T;
#define FREE_WCMD(wcmd) xfree((wcmd)->line)
struct loop_cookie {
garray_T *lines_gap; 
int current_line; 
int repeating; 
char_u *(*getline)(int, void *, int, bool);
void *cookie;
};
struct dbg_stuff {
int trylevel;
int force_abort;
except_T *caught_stack;
char_u *vv_exception;
char_u *vv_throwpoint;
int did_emsg;
int got_int;
int need_rethrow;
int check_cstack;
except_T *current_exception;
};
typedef struct {
exarg_T *eap;
char_u *parsed_upto; 
char_u *cmd; 
char_u *after_modifier;
char_u *errormsg;
cmdmod_T cmdmod;
int sandbox;
int msg_silent;
int emsg_silent;
bool ex_pressedreturn;
long p_verbose;
bool set_eventignore;
long verbose_save;
int save_msg_silent;
int did_esilent;
bool did_sandbox;
} parse_state_T;
#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "ex_docmd.c.generated.h"
#endif
#if !defined(HAVE_WORKING_LIBINTL)
#define ex_language ex_ni
#endif
#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "ex_cmds_defs.generated.h"
#endif
static char_u dollar_command[2] = {'$', 0};
static void save_dbg_stuff(struct dbg_stuff *dsp)
{
dsp->trylevel = trylevel; trylevel = 0;
dsp->force_abort = force_abort; force_abort = FALSE;
dsp->caught_stack = caught_stack; caught_stack = NULL;
dsp->vv_exception = v_exception(NULL);
dsp->vv_throwpoint = v_throwpoint(NULL);
dsp->did_emsg = did_emsg; did_emsg = false;
dsp->got_int = got_int; got_int = false;
dsp->need_rethrow = need_rethrow; need_rethrow = false;
dsp->check_cstack = check_cstack; check_cstack = false;
dsp->current_exception = current_exception; current_exception = NULL;
}
static void restore_dbg_stuff(struct dbg_stuff *dsp)
{
suppress_errthrow = FALSE;
trylevel = dsp->trylevel;
force_abort = dsp->force_abort;
caught_stack = dsp->caught_stack;
(void)v_exception(dsp->vv_exception);
(void)v_throwpoint(dsp->vv_throwpoint);
did_emsg = dsp->did_emsg;
got_int = dsp->got_int;
need_rethrow = dsp->need_rethrow;
check_cstack = dsp->check_cstack;
current_exception = dsp->current_exception;
}
void do_exmode(int improved)
{
int save_msg_scroll;
int prev_msg_row;
linenr_T prev_line;
int changedtick;
if (improved)
exmode_active = EXMODE_VIM;
else
exmode_active = EXMODE_NORMAL;
State = NORMAL;
if (global_busy)
return;
save_msg_scroll = msg_scroll;
RedrawingDisabled++; 
no_wait_return++; 
MSG(_("Entering Ex mode. Type \"visual\" to go to Normal mode."));
while (exmode_active) {
if (ex_normal_busy > 0 && typebuf.tb_len == 0) {
exmode_active = FALSE;
break;
}
msg_scroll = true;
need_wait_return = false;
ex_pressedreturn = false;
ex_no_reprint = false;
changedtick = buf_get_changedtick(curbuf);
prev_msg_row = msg_row;
prev_line = curwin->w_cursor.lnum;
cmdline_row = msg_row;
do_cmdline(NULL, getexline, NULL, 0);
lines_left = Rows - 1;
if ((prev_line != curwin->w_cursor.lnum
|| changedtick != buf_get_changedtick(curbuf)) && !ex_no_reprint) {
if (curbuf->b_ml.ml_flags & ML_EMPTY) {
EMSG(_(e_emptybuf));
} else {
if (ex_pressedreturn) {
msg_row = prev_msg_row;
if (prev_msg_row == Rows - 1)
msg_row--;
}
msg_col = 0;
print_line_no_prefix(curwin->w_cursor.lnum, FALSE, FALSE);
msg_clr_eos();
}
} else if (ex_pressedreturn && !ex_no_reprint) { 
if (curbuf->b_ml.ml_flags & ML_EMPTY)
EMSG(_(e_emptybuf));
else
EMSG(_("E501: At end-of-file"));
}
}
RedrawingDisabled--;
no_wait_return--;
redraw_all_later(NOT_VALID);
update_screen(NOT_VALID);
need_wait_return = false;
msg_scroll = save_msg_scroll;
}
int do_cmdline_cmd(const char *cmd)
{
return do_cmdline((char_u *)cmd, NULL, NULL,
DOCMD_NOWAIT|DOCMD_KEYTYPED);
}
int do_cmdline(char_u *cmdline, LineGetter fgetline,
void *cookie, 
int flags)
{
char_u *next_cmdline; 
char_u *cmdline_copy = NULL; 
int used_getline = FALSE; 
static int recursive = 0; 
int msg_didout_before_start = 0;
int count = 0; 
int did_inc = FALSE; 
int retval = OK;
cstack_T cstack; 
garray_T lines_ga; 
int current_line = 0; 
char_u *fname = NULL; 
linenr_T *breakpoint = NULL; 
int *dbg_tick = NULL; 
struct dbg_stuff debug_saved; 
int initial_trylevel;
struct msglist **saved_msg_list = NULL;
struct msglist *private_msg_list;
char_u *(*cmd_getline)(int, void *, int, bool);
void *cmd_cookie;
struct loop_cookie cmd_loop_cookie;
void *real_cookie;
int getline_is_func;
static int call_depth = 0; 
saved_msg_list = msg_list;
msg_list = &private_msg_list;
private_msg_list = NULL;
if (call_depth >= 200 && call_depth >= p_mfd) {
EMSG(_("E169: Command too recursive"));
do_errthrow((cstack_T *)NULL, (char_u *)NULL);
msg_list = saved_msg_list;
return FAIL;
}
call_depth++;
start_batch_changes();
cstack.cs_idx = -1;
cstack.cs_looplevel = 0;
cstack.cs_trylevel = 0;
cstack.cs_emsg_silent_list = NULL;
cstack.cs_lflags = 0;
ga_init(&lines_ga, (int)sizeof(wcmd_T), 10);
real_cookie = getline_cookie(fgetline, cookie);
getline_is_func = getline_equal(fgetline, cookie, get_func_line);
if (getline_is_func && ex_nesting_level == func_level(real_cookie))
++ex_nesting_level;
if (getline_is_func) {
fname = func_name(real_cookie);
breakpoint = func_breakpoint(real_cookie);
dbg_tick = func_dbg_tick(real_cookie);
} else if (getline_equal(fgetline, cookie, getsourceline)) {
fname = sourcing_name;
breakpoint = source_breakpoint(real_cookie);
dbg_tick = source_dbg_tick(real_cookie);
}
if (!recursive) {
force_abort = FALSE;
suppress_errthrow = FALSE;
}
if (flags & DOCMD_EXCRESET) {
save_dbg_stuff(&debug_saved);
} else {
memset(&debug_saved, 0, sizeof(debug_saved));
}
initial_trylevel = trylevel;
current_exception = NULL;
did_emsg = false;
if (!(flags & DOCMD_KEYTYPED)
&& !getline_equal(fgetline, cookie, getexline)) {
KeyTyped = false;
}
next_cmdline = cmdline;
do {
getline_is_func = getline_equal(fgetline, cookie, get_func_line);
if (next_cmdline == NULL
&& !force_abort
&& cstack.cs_idx < 0
&& !(getline_is_func && func_has_abort(real_cookie))
)
did_emsg = FALSE;
if (cstack.cs_looplevel > 0 && current_line < lines_ga.ga_len) {
XFREE_CLEAR(cmdline_copy);
if (getline_is_func) {
if (do_profiling == PROF_YES)
func_line_end(real_cookie);
if (func_has_ended(real_cookie)) {
retval = FAIL;
break;
}
} else if (do_profiling == PROF_YES
&& getline_equal(fgetline, cookie, getsourceline))
script_line_end();
if (source_finished(fgetline, cookie)) {
retval = FAIL;
break;
}
if (breakpoint != NULL && dbg_tick != NULL
&& *dbg_tick != debug_tick) {
*breakpoint = dbg_find_breakpoint(
getline_equal(fgetline, cookie, getsourceline),
fname, sourcing_lnum);
*dbg_tick = debug_tick;
}
next_cmdline = ((wcmd_T *)(lines_ga.ga_data))[current_line].line;
sourcing_lnum = ((wcmd_T *)(lines_ga.ga_data))[current_line].lnum;
if (breakpoint != NULL && *breakpoint != 0
&& *breakpoint <= sourcing_lnum) {
dbg_breakpoint(fname, sourcing_lnum);
*breakpoint = dbg_find_breakpoint(
getline_equal(fgetline, cookie, getsourceline),
fname, sourcing_lnum);
*dbg_tick = debug_tick;
}
if (do_profiling == PROF_YES) {
if (getline_is_func)
func_line_start(real_cookie);
else if (getline_equal(fgetline, cookie, getsourceline))
script_line_start();
}
}
if (cstack.cs_looplevel > 0) {
cmd_getline = get_loop_line;
cmd_cookie = (void *)&cmd_loop_cookie;
cmd_loop_cookie.lines_gap = &lines_ga;
cmd_loop_cookie.current_line = current_line;
cmd_loop_cookie.getline = fgetline;
cmd_loop_cookie.cookie = cookie;
cmd_loop_cookie.repeating = (current_line < lines_ga.ga_len);
} else {
cmd_getline = fgetline;
cmd_cookie = cookie;
}
if (next_cmdline == NULL) {
if (count == 1 && getline_equal(fgetline, cookie, getexline)) {
msg_didout = true;
}
if (fgetline == NULL
|| (next_cmdline = fgetline(':', cookie,
cstack.cs_idx <
0 ? 0 : (cstack.cs_idx + 1) * 2,
true)) == NULL) {
if (KeyTyped && !(flags & DOCMD_REPEAT)) {
need_wait_return = false;
}
retval = FAIL;
break;
}
used_getline = TRUE;
if (flags & DOCMD_KEEPLINE) {
xfree(repeat_cmdline);
if (count == 0)
repeat_cmdline = vim_strsave(next_cmdline);
else
repeat_cmdline = NULL;
}
}
else if (cmdline_copy == NULL) {
next_cmdline = vim_strsave(next_cmdline);
}
cmdline_copy = next_cmdline;
if (current_line == lines_ga.ga_len
&& (cstack.cs_looplevel || has_loop_cmd(next_cmdline))) {
store_loop_line(&lines_ga, next_cmdline);
}
did_endif = FALSE;
if (count++ == 0) {
if (!(flags & DOCMD_NOWAIT) && !recursive) {
msg_didout_before_start = msg_didout;
msg_didany = FALSE; 
msg_start();
msg_scroll = TRUE; 
++no_wait_return; 
++RedrawingDisabled;
did_inc = TRUE;
}
}
if (p_verbose >= 15 && sourcing_name != NULL) {
++no_wait_return;
verbose_enter_scroll();
smsg(_("line %" PRIdLINENR ": %s"), sourcing_lnum, cmdline_copy);
if (msg_silent == 0) {
msg_puts("\n"); 
}
verbose_leave_scroll();
--no_wait_return;
}
recursive++;
next_cmdline = do_one_cmd(&cmdline_copy, flags,
&cstack,
cmd_getline, cmd_cookie);
recursive--;
if (State & CMDPREVIEW) {
next_cmdline = NULL;
}
if (cmd_cookie == (void *)&cmd_loop_cookie)
current_line = cmd_loop_cookie.current_line;
if (next_cmdline == NULL) {
XFREE_CLEAR(cmdline_copy);
if (getline_equal(fgetline, cookie, getexline)
&& new_last_cmdline != NULL) {
xfree(last_cmdline);
last_cmdline = new_last_cmdline;
new_last_cmdline = NULL;
}
} else {
STRMOVE(cmdline_copy, next_cmdline);
next_cmdline = cmdline_copy;
}
if (did_emsg && !force_abort
&& getline_equal(fgetline, cookie, get_func_line)
&& !func_has_abort(real_cookie))
did_emsg = FALSE;
if (cstack.cs_looplevel > 0) {
++current_line;
if (cstack.cs_lflags & (CSL_HAD_CONT | CSL_HAD_ENDLOOP)) {
cstack.cs_lflags &= ~(CSL_HAD_CONT | CSL_HAD_ENDLOOP);
if (!did_emsg && !got_int && !current_exception
&& cstack.cs_idx >= 0
&& (cstack.cs_flags[cstack.cs_idx]
& (CSF_WHILE | CSF_FOR))
&& cstack.cs_line[cstack.cs_idx] >= 0
&& (cstack.cs_flags[cstack.cs_idx] & CSF_ACTIVE)) {
current_line = cstack.cs_line[cstack.cs_idx];
cstack.cs_lflags |= CSL_HAD_LOOP;
line_breakcheck(); 
if (breakpoint != NULL) {
*breakpoint = dbg_find_breakpoint(
getline_equal(fgetline, cookie, getsourceline),
fname,
((wcmd_T *)lines_ga.ga_data)[current_line].lnum-1);
*dbg_tick = debug_tick;
}
} else {
if (cstack.cs_idx >= 0)
rewind_conditionals(&cstack, cstack.cs_idx - 1,
CSF_WHILE | CSF_FOR, &cstack.cs_looplevel);
}
}
else if (cstack.cs_lflags & CSL_HAD_LOOP) {
cstack.cs_lflags &= ~CSL_HAD_LOOP;
cstack.cs_line[cstack.cs_idx] = current_line - 1;
}
}
if (cstack.cs_looplevel == 0) {
if (!GA_EMPTY(&lines_ga)) {
sourcing_lnum = ((wcmd_T *)lines_ga.ga_data)[lines_ga.ga_len - 1].lnum;
GA_DEEP_CLEAR(&lines_ga, wcmd_T, FREE_WCMD);
}
current_line = 0;
}
if (cstack.cs_lflags & CSL_HAD_FINA) {
cstack.cs_lflags &= ~CSL_HAD_FINA;
report_make_pending((cstack.cs_pending[cstack.cs_idx]
& (CSTP_ERROR | CSTP_INTERRUPT | CSTP_THROW)),
current_exception);
did_emsg = got_int = false;
current_exception = NULL;
cstack.cs_flags[cstack.cs_idx] |= CSF_ACTIVE | CSF_FINALLY;
}
trylevel = initial_trylevel + cstack.cs_trylevel;
if (trylevel == 0 && !did_emsg && !got_int && !current_exception) {
force_abort = false;
}
(void)do_intthrow(&cstack);
}
while (!((got_int || (did_emsg && force_abort) || current_exception)
&& cstack.cs_trylevel == 0)
&& !(did_emsg
&& (cstack.cs_trylevel == 0 || did_emsg_syntax)
&& used_getline
&& (getline_equal(fgetline, cookie, getexmodeline)
|| getline_equal(fgetline, cookie, getexline)))
&& (next_cmdline != NULL
|| cstack.cs_idx >= 0
|| (flags & DOCMD_REPEAT)));
xfree(cmdline_copy);
did_emsg_syntax = FALSE;
GA_DEEP_CLEAR(&lines_ga, wcmd_T, FREE_WCMD);
if (cstack.cs_idx >= 0) {
if (!got_int && !current_exception
&& ((getline_equal(fgetline, cookie, getsourceline)
&& !source_finished(fgetline, cookie))
|| (getline_equal(fgetline, cookie, get_func_line)
&& !func_has_ended(real_cookie)))) {
if (cstack.cs_flags[cstack.cs_idx] & CSF_TRY)
EMSG(_(e_endtry));
else if (cstack.cs_flags[cstack.cs_idx] & CSF_WHILE)
EMSG(_(e_endwhile));
else if (cstack.cs_flags[cstack.cs_idx] & CSF_FOR)
EMSG(_(e_endfor));
else
EMSG(_(e_endif));
}
do {
int idx = cleanup_conditionals(&cstack, 0, TRUE);
if (idx >= 0)
--idx; 
rewind_conditionals(&cstack, idx, CSF_WHILE | CSF_FOR,
&cstack.cs_looplevel);
} while (cstack.cs_idx >= 0);
trylevel = initial_trylevel;
}
do_errthrow(&cstack, getline_equal(fgetline, cookie, get_func_line)
? (char_u *)"endfunction" : (char_u *)NULL);
if (trylevel == 0) {
if (current_exception) {
void *p = NULL;
char_u *saved_sourcing_name;
int saved_sourcing_lnum;
struct msglist *messages = NULL;
struct msglist *next;
switch (current_exception->type) {
case ET_USER:
vim_snprintf((char *)IObuff, IOSIZE,
_("E605: Exception not caught: %s"),
current_exception->value);
p = vim_strsave(IObuff);
break;
case ET_ERROR:
messages = current_exception->messages;
current_exception->messages = NULL;
break;
case ET_INTERRUPT:
break;
}
saved_sourcing_name = sourcing_name;
saved_sourcing_lnum = sourcing_lnum;
sourcing_name = current_exception->throw_name;
sourcing_lnum = current_exception->throw_lnum;
current_exception->throw_name = NULL;
discard_current_exception(); 
suppress_errthrow = true;
force_abort = true;
msg_ext_set_kind("emsg"); 
if (messages != NULL) {
do {
next = messages->next;
emsg(messages->msg);
xfree(messages->msg);
xfree(messages);
messages = next;
} while (messages != NULL);
} else if (p != NULL) {
emsg(p);
xfree(p);
}
xfree(sourcing_name);
sourcing_name = saved_sourcing_name;
sourcing_lnum = saved_sourcing_lnum;
}
else if (got_int || (did_emsg && force_abort))
suppress_errthrow = TRUE;
}
if (current_exception) {
need_rethrow = true;
}
if ((getline_equal(fgetline, cookie, getsourceline)
&& ex_nesting_level > source_level(real_cookie))
|| (getline_equal(fgetline, cookie, get_func_line)
&& ex_nesting_level > func_level(real_cookie) + 1)) {
if (!current_exception) {
check_cstack = true;
}
} else {
if (getline_equal(fgetline, cookie, get_func_line))
--ex_nesting_level;
if ((getline_equal(fgetline, cookie, getsourceline)
|| getline_equal(fgetline, cookie, get_func_line))
&& ex_nesting_level + 1 <= debug_break_level)
do_debug(getline_equal(fgetline, cookie, getsourceline)
? (char_u *)_("End of sourced file")
: (char_u *)_("End of function"));
}
if (flags & DOCMD_EXCRESET)
restore_dbg_stuff(&debug_saved);
msg_list = saved_msg_list;
if (did_inc) {
--RedrawingDisabled;
--no_wait_return;
msg_scroll = FALSE;
if (retval == FAIL
|| (did_endif && KeyTyped && !did_emsg)
) {
need_wait_return = FALSE;
msg_didany = FALSE; 
} else if (need_wait_return) {
msg_didout |= msg_didout_before_start;
wait_return(FALSE);
}
}
did_endif = FALSE; 
call_depth--;
end_batch_changes();
return retval;
}
static char_u *get_loop_line(int c, void *cookie, int indent, bool do_concat)
{
struct loop_cookie *cp = (struct loop_cookie *)cookie;
wcmd_T *wp;
char_u *line;
if (cp->current_line + 1 >= cp->lines_gap->ga_len) {
if (cp->repeating)
return NULL; 
if (cp->getline == NULL) {
line = getcmdline(c, 0L, indent, do_concat);
} else {
line = cp->getline(c, cp->cookie, indent, do_concat);
}
if (line != NULL) {
store_loop_line(cp->lines_gap, line);
++cp->current_line;
}
return line;
}
KeyTyped = false;
cp->current_line++;
wp = (wcmd_T *)(cp->lines_gap->ga_data) + cp->current_line;
sourcing_lnum = wp->lnum;
return vim_strsave(wp->line);
}
static void store_loop_line(garray_T *gap, char_u *line)
{
wcmd_T *p = GA_APPEND_VIA_PTR(wcmd_T, gap);
p->line = vim_strsave(line);
p->lnum = sourcing_lnum;
}
int getline_equal(LineGetter fgetline,
void *cookie, 
LineGetter func)
{
LineGetter gp;
struct loop_cookie *cp;
gp = fgetline;
cp = (struct loop_cookie *)cookie;
while (gp == get_loop_line) {
gp = cp->getline;
cp = cp->cookie;
}
return gp == func;
}
void * getline_cookie(LineGetter fgetline,
void *cookie 
)
{
LineGetter gp;
struct loop_cookie *cp;
gp = fgetline;
cp = (struct loop_cookie *)cookie;
while (gp == get_loop_line) {
gp = cp->getline;
cp = cp->cookie;
}
return cp;
}
static int compute_buffer_local_count(int addr_type, int lnum, int offset)
{
buf_T *buf;
buf_T *nextbuf;
int count = offset;
buf = firstbuf;
while (buf->b_next != NULL && buf->b_fnum < lnum)
buf = buf->b_next;
while (count != 0) {
count += (count < 0) ? 1 : -1;
nextbuf = (offset < 0) ? buf->b_prev : buf->b_next;
if (nextbuf == NULL)
break;
buf = nextbuf;
if (addr_type == ADDR_LOADED_BUFFERS)
while (buf->b_ml.ml_mfp == NULL) {
nextbuf = (offset < 0) ? buf->b_prev : buf->b_next;
if (nextbuf == NULL) {
break;
}
buf = nextbuf;
}
}
if (addr_type == ADDR_LOADED_BUFFERS) {
while (buf->b_ml.ml_mfp == NULL) {
nextbuf = (offset >= 0) ? buf->b_prev : buf->b_next;
if (nextbuf == NULL)
break;
buf = nextbuf;
}
}
return buf->b_fnum;
}
static int current_win_nr(win_T *win)
{
int nr = 0;
FOR_ALL_WINDOWS_IN_TAB(wp, curtab) {
++nr;
if (wp == win)
break;
}
return nr;
}
static int current_tab_nr(tabpage_T *tab)
{
int nr = 0;
FOR_ALL_TABS(tp) {
++nr;
if (tp == tab)
break;
}
return nr;
}
#define CURRENT_WIN_NR current_win_nr(curwin)
#define LAST_WIN_NR current_win_nr(NULL)
#define CURRENT_TAB_NR current_tab_nr(curtab)
#define LAST_TAB_NR current_tab_nr(NULL)
static void get_wincmd_addr_type(char_u *arg, exarg_T *eap)
{
switch (*arg) {
case 'S':
case Ctrl_S:
case 's':
case Ctrl_N:
case 'n':
case 'j':
case Ctrl_J:
case 'k':
case Ctrl_K:
case 'T':
case Ctrl_R:
case 'r':
case 'R':
case 'K':
case 'J':
case '+':
case '-':
case Ctrl__:
case '_':
case '|':
case ']':
case Ctrl_RSB:
case 'g':
case Ctrl_G:
case Ctrl_V:
case 'v':
case 'h':
case Ctrl_H:
case 'l':
case Ctrl_L:
case 'H':
case 'L':
case '>':
case '<':
case '}':
case 'f':
case 'F':
case Ctrl_F:
case 'i':
case Ctrl_I:
case 'd':
case Ctrl_D:
eap->addr_type = ADDR_LINES; 
break;
case Ctrl_HAT:
case '^':
eap->addr_type = ADDR_BUFFERS;
break;
case Ctrl_Q:
case 'q':
case Ctrl_C:
case 'c':
case Ctrl_O:
case 'o':
case Ctrl_W:
case 'w':
case 'W':
case 'x':
case Ctrl_X:
eap->addr_type = ADDR_WINDOWS;
break;
case Ctrl_Z:
case 'z':
case 'P':
case 't':
case Ctrl_T:
case 'b':
case Ctrl_B:
case 'p':
case Ctrl_P:
case '=':
case CAR:
eap->addr_type = 0;
break;
}
}
static char_u *skip_colon_white(const char_u *p, bool skipleadingwhite)
{
if (skipleadingwhite) {
p = skipwhite(p);
}
while (*p == ':') {
p = skipwhite(p + 1);
}
return (char_u *)p;
}
static void parse_state_to_global(const parse_state_T *parse_state)
{
cmdmod = parse_state->cmdmod;
sandbox = parse_state->sandbox;
msg_silent = parse_state->msg_silent;
emsg_silent = parse_state->emsg_silent;
ex_pressedreturn = parse_state->ex_pressedreturn;
p_verbose = parse_state->p_verbose;
if (parse_state->set_eventignore) {
set_string_option_direct(
(char_u *)"ei", -1, (char_u *)"all", OPT_FREE, SID_NONE);
}
}
static void parse_state_from_global(parse_state_T *parse_state)
{
memset(parse_state, 0, sizeof(*parse_state));
parse_state->cmdmod = cmdmod;
parse_state->sandbox = sandbox;
parse_state->msg_silent = msg_silent;
parse_state->emsg_silent = emsg_silent;
parse_state->ex_pressedreturn = ex_pressedreturn;
parse_state->p_verbose = p_verbose;
}
static bool parse_one_cmd(
char_u **cmdlinep,
parse_state_T *const out,
LineGetter fgetline,
void *fgetline_cookie)
{
exarg_T ea = {
.line1 = 1,
.line2 = 1,
};
*out->eap = ea;
if ((*cmdlinep)[0] == '#' && (*cmdlinep)[1] == '!') {
return false;
}
ea.cmd = *cmdlinep;
for (;; ) {
while (*ea.cmd == ' '
|| *ea.cmd == '\t'
|| *ea.cmd == ':') {
ea.cmd++;
}
if (*ea.cmd == NUL && exmode_active
&& (getline_equal(fgetline, fgetline_cookie, getexmodeline)
|| getline_equal(fgetline, fgetline_cookie, getexline))
&& curwin->w_cursor.lnum < curbuf->b_ml.ml_line_count) {
ea.cmd = (char_u *)"+";
out->ex_pressedreturn = true;
}
if (*ea.cmd == '"') {
return false;
}
if (*ea.cmd == NUL) {
out->ex_pressedreturn = true;
return false;
}
char_u *p = skip_range(ea.cmd, NULL);
switch (*p) {
case 'a': if (!checkforcmd(&ea.cmd, "aboveleft", 3))
break;
out->cmdmod.split |= WSP_ABOVE;
continue;
case 'b': if (checkforcmd(&ea.cmd, "belowright", 3)) {
out->cmdmod.split |= WSP_BELOW;
continue;
}
if (checkforcmd(&ea.cmd, "browse", 3)) {
out->cmdmod.browse = true;
continue;
}
if (!checkforcmd(&ea.cmd, "botright", 2)) {
break;
}
out->cmdmod.split |= WSP_BOT;
continue;
case 'c': if (!checkforcmd(&ea.cmd, "confirm", 4))
break;
out->cmdmod.confirm = true;
continue;
case 'k': if (checkforcmd(&ea.cmd, "keepmarks", 3)) {
out->cmdmod.keepmarks = true;
continue;
}
if (checkforcmd(&ea.cmd, "keepalt", 5)) {
out->cmdmod.keepalt = true;
continue;
}
if (checkforcmd(&ea.cmd, "keeppatterns", 5)) {
out->cmdmod.keeppatterns = true;
continue;
}
if (!checkforcmd(&ea.cmd, "keepjumps", 5)) {
break;
}
out->cmdmod.keepjumps = true;
continue;
case 'f': { 
char_u *reg_pat;
if (!checkforcmd(&p, "filter", 4) || *p == NUL || ends_excmd(*p)) {
break;
}
if (*p == '!') {
out->cmdmod.filter_force = true;
p = skipwhite(p + 1);
if (*p == NUL || ends_excmd(*p)) {
break;
}
}
p = skip_vimgrep_pat(p, &reg_pat, NULL);
if (p == NULL || *p == NUL) {
break;
}
out->cmdmod.filter_regmatch.regprog = vim_regcomp(reg_pat, RE_MAGIC);
if (out->cmdmod.filter_regmatch.regprog == NULL) {
break;
}
ea.cmd = p;
continue;
}
case 'h': if (p != ea.cmd || !checkforcmd(&p, "hide", 3)
|| *p == NUL || ends_excmd(*p))
break;
ea.cmd = p;
out->cmdmod.hide = true;
continue;
case 'l': if (checkforcmd(&ea.cmd, "lockmarks", 3)) {
out->cmdmod.lockmarks = true;
continue;
}
if (!checkforcmd(&ea.cmd, "leftabove", 5)) {
break;
}
out->cmdmod.split |= WSP_ABOVE;
continue;
case 'n':
if (checkforcmd(&ea.cmd, "noautocmd", 3)) {
if (out->cmdmod.save_ei == NULL) {
out->cmdmod.save_ei = vim_strsave(p_ei);
out->set_eventignore = true;
}
continue;
}
if (!checkforcmd(&ea.cmd, "noswapfile", 3)) {
break;
}
out->cmdmod.noswapfile = true;
continue;
case 'r': if (!checkforcmd(&ea.cmd, "rightbelow", 6))
break;
out->cmdmod.split |= WSP_BELOW;
continue;
case 's': if (checkforcmd(&ea.cmd, "sandbox", 3)) {
if (!out->did_sandbox) {
out->sandbox++;
}
out->did_sandbox = true;
continue;
}
if (!checkforcmd(&ea.cmd, "silent", 3)) {
break;
}
if (out->save_msg_silent == -1) {
out->save_msg_silent = out->msg_silent;
}
out->msg_silent++;
if (*ea.cmd == '!' && !ascii_iswhite(ea.cmd[-1])) {
ea.cmd = skipwhite(ea.cmd + 1);
out->emsg_silent++;
out->did_esilent++;
}
continue;
case 't': if (checkforcmd(&p, "tab", 3)) {
long tabnr = get_address(
&ea, &ea.cmd, ADDR_TABS, ea.skip, false, 1);
if (tabnr == MAXLNUM) {
out->cmdmod.tab = tabpage_index(curtab) + 1;
} else {
if (tabnr < 0 || tabnr > LAST_TAB_NR) {
out->errormsg = (char_u *)_(e_invrange);
return false;
}
out->cmdmod.tab = tabnr + 1;
}
ea.cmd = p;
continue;
}
if (!checkforcmd(&ea.cmd, "topleft", 2)) {
break;
}
out->cmdmod.split |= WSP_TOP;
continue;
case 'u': if (!checkforcmd(&ea.cmd, "unsilent", 3))
break;
if (out->save_msg_silent == -1) {
out->save_msg_silent = out->msg_silent;
}
out->msg_silent = 0;
continue;
case 'v': if (checkforcmd(&ea.cmd, "vertical", 4)) {
out->cmdmod.split |= WSP_VERT;
continue;
}
if (!checkforcmd(&p, "verbose", 4))
break;
if (out->verbose_save < 0) {
out->verbose_save = out->p_verbose;
}
if (ascii_isdigit(*ea.cmd)) {
out->p_verbose = atoi((char *)ea.cmd);
} else {
out->p_verbose = 1;
}
ea.cmd = p;
continue;
}
break;
}
out->after_modifier = ea.cmd;
out->cmd = ea.cmd;
ea.cmd = skip_range(ea.cmd, NULL);
if (*ea.cmd == '*') {
ea.cmd = skipwhite(ea.cmd + 1);
}
out->parsed_upto = find_command(&ea, NULL);
*out->eap = ea;
return true;
}
static char_u * do_one_cmd(char_u **cmdlinep,
int flags,
cstack_T *cstack,
LineGetter fgetline,
void *cookie 
)
{
char_u *p;
linenr_T lnum;
long n;
char_u *errormsg = NULL; 
exarg_T ea;
int save_msg_scroll = msg_scroll;
parse_state_T parsed;
cmdmod_T save_cmdmod;
const int save_reg_executing = reg_executing;
ex_nesting_level++;
if (quitmore
&& !getline_equal(fgetline, cookie, get_func_line)
&& !getline_equal(fgetline, cookie, getnextac)
)
--quitmore;
save_cmdmod = cmdmod;
memset(&cmdmod, 0, sizeof(cmdmod));
parse_state_from_global(&parsed);
parsed.eap = &ea;
parsed.verbose_save = -1;
parsed.save_msg_silent = -1;
parsed.did_esilent = 0;
parsed.did_sandbox = false;
bool parse_success = parse_one_cmd(cmdlinep, &parsed, fgetline, cookie);
parse_state_to_global(&parsed);
errormsg = parsed.errormsg;
p = parsed.parsed_upto;
if (!parse_success) {
goto doend;
}
ea.skip = (did_emsg
|| got_int
|| current_exception
|| (cstack->cs_idx >= 0
&& !(cstack->cs_flags[cstack->cs_idx] & CSF_ACTIVE)));
if (do_profiling == PROF_YES
&& (!ea.skip || cstack->cs_idx == 0
|| (cstack->cs_idx > 0
&& (cstack->cs_flags[cstack->cs_idx - 1] & CSF_ACTIVE)))) {
int skip = did_emsg || got_int || current_exception;
if (ea.cmdidx == CMD_catch) {
skip = !skip && !(cstack->cs_idx >= 0
&& (cstack->cs_flags[cstack->cs_idx] & CSF_THROWN)
&& !(cstack->cs_flags[cstack->cs_idx] & CSF_CAUGHT));
} else if (ea.cmdidx == CMD_else || ea.cmdidx == CMD_elseif) {
skip = skip || !(cstack->cs_idx >= 0
&& !(cstack->cs_flags[cstack->cs_idx]
& (CSF_ACTIVE | CSF_TRUE)));
} else if (ea.cmdidx == CMD_finally) {
skip = false;
} else if (ea.cmdidx != CMD_endif
&& ea.cmdidx != CMD_endfor
&& ea.cmdidx != CMD_endtry
&& ea.cmdidx != CMD_endwhile) {
skip = ea.skip;
}
if (!skip) {
if (getline_equal(fgetline, cookie, get_func_line)) {
func_line_exec(getline_cookie(fgetline, cookie));
} else if (getline_equal(fgetline, cookie, getsourceline)) {
script_line_exec();
}
}
}
dbg_check_breakpoint(&ea);
if (!ea.skip && got_int) {
ea.skip = TRUE;
(void)do_intthrow(cstack);
}
if (!IS_USER_CMDIDX(ea.cmdidx)) {
if (ea.cmdidx != CMD_SIZE) {
ea.addr_type = cmdnames[(int)ea.cmdidx].cmd_addr_type;
} else {
ea.addr_type = ADDR_LINES;
}
if (ea.cmdidx == CMD_wincmd && p != NULL) {
get_wincmd_addr_type(skipwhite(p), &ea);
}
}
ea.cmd = parsed.cmd;
if (parse_cmd_address(&ea, &errormsg) == FAIL) {
goto doend;
}
ea.cmd = skip_colon_white(ea.cmd, true);
if (*ea.cmd == NUL || *ea.cmd == '"'
|| (ea.nextcmd = check_nextcmd(ea.cmd)) != NULL) {
if (ea.skip) { 
goto doend;
}
if (*ea.cmd == '|' || (exmode_active && ea.line1 != ea.line2)) {
ea.cmdidx = CMD_print;
ea.argt = RANGE | COUNT | TRLBAR;
if ((errormsg = invalid_range(&ea)) == NULL) {
correct_range(&ea);
ex_print(&ea);
}
} else if (ea.addr_count != 0) {
if (ea.line2 > curbuf->b_ml.ml_line_count) {
ea.line2 = curbuf->b_ml.ml_line_count;
}
if (ea.line2 < 0)
errormsg = (char_u *)_(e_invrange);
else {
if (ea.line2 == 0)
curwin->w_cursor.lnum = 1;
else
curwin->w_cursor.lnum = ea.line2;
beginline(BL_SOL | BL_FIX);
}
}
goto doend;
}
if (p != NULL && ea.cmdidx == CMD_SIZE && !ea.skip
&& ASCII_ISUPPER(*ea.cmd)
&& has_event(EVENT_CMDUNDEFINED)) {
p = ea.cmd;
while (ASCII_ISALNUM(*p)) {
++p;
}
p = vim_strnsave(ea.cmd, p - ea.cmd);
int ret = apply_autocmds(EVENT_CMDUNDEFINED, p, p, TRUE, NULL);
xfree(p);
p = (ret && !aborting()) ? find_command(&ea, NULL) : ea.cmd;
}
if (p == NULL) {
if (!ea.skip)
errormsg = (char_u *)_("E464: Ambiguous use of user-defined command");
goto doend;
}
if (ea.cmdidx == CMD_SIZE) {
if (!ea.skip) {
STRCPY(IObuff, _("E492: Not an editor command"));
if (!(flags & DOCMD_VERBOSE)) {
if (parsed.after_modifier != NULL) {
append_command(parsed.after_modifier);
} else {
append_command(*cmdlinep);
}
}
errormsg = IObuff;
did_emsg_syntax = TRUE;
}
goto doend;
}
const int ni = !IS_USER_CMDIDX(ea.cmdidx)
&& (cmdnames[ea.cmdidx].cmd_func == ex_ni
|| cmdnames[ea.cmdidx].cmd_func == ex_script_ni);
if (*p == '!' && ea.cmdidx != CMD_substitute
&& ea.cmdidx != CMD_smagic && ea.cmdidx != CMD_snomagic) {
p++;
ea.forceit = true;
} else {
ea.forceit = false;
}
if (!IS_USER_CMDIDX(ea.cmdidx)) {
ea.argt = cmdnames[(int)ea.cmdidx].cmd_argt;
}
if (!ea.skip) {
if (sandbox != 0 && !(ea.argt & SBOXOK)) {
errormsg = (char_u *)_(e_sandbox);
goto doend;
}
if (restricted != 0 && (ea.argt & RESTRICT)) {
errormsg = (char_u *)_("E981: Command not allowed in restricted mode");
goto doend;
}
if (!MODIFIABLE(curbuf) && (ea.argt & MODIFY)
&& (!curbuf->terminal || ea.cmdidx != CMD_put)) {
errormsg = (char_u *)_(e_modifiable);
goto doend;
}
if (text_locked() && !(ea.argt & CMDWIN)
&& !IS_USER_CMDIDX(ea.cmdidx)) {
errormsg = (char_u *)_(get_text_locked_msg());
goto doend;
}
if (!(ea.argt & CMDWIN)
&& ea.cmdidx != CMD_checktime
&& ea.cmdidx != CMD_edit
&& ea.cmdidx != CMD_file
&& !IS_USER_CMDIDX(ea.cmdidx)
&& curbuf_locked()) {
goto doend;
}
if (!ni && !(ea.argt & RANGE) && ea.addr_count > 0) {
errormsg = (char_u *)_(e_norange);
goto doend;
}
}
if (!ni && !(ea.argt & BANG) && ea.forceit) { 
errormsg = (char_u *)_(e_nobang);
goto doend;
}
if (!ea.skip && !ni) {
if (!global_busy && ea.line1 > ea.line2) {
if (msg_silent == 0) {
if ((flags & DOCMD_VERBOSE) || exmode_active) {
errormsg = (char_u *)_("E493: Backwards range given");
goto doend;
}
if (ask_yesno(_("Backwards range given, OK to swap"), false) != 'y') {
goto doend;
}
}
lnum = ea.line1;
ea.line1 = ea.line2;
ea.line2 = lnum;
}
if ((errormsg = invalid_range(&ea)) != NULL)
goto doend;
}
if ((ea.argt & NOTADR) && ea.addr_count == 0) 
ea.line2 = 1;
correct_range(&ea);
if (((ea.argt & WHOLEFOLD) || ea.addr_count >= 2) && !global_busy
&& ea.addr_type == ADDR_LINES) {
(void)hasFolding(ea.line1, &ea.line1, NULL);
(void)hasFolding(ea.line2, NULL, &ea.line2);
}
p = replace_makeprg(&ea, p, cmdlinep);
if (p == NULL)
goto doend;
if (ea.cmdidx == CMD_bang)
ea.arg = p;
else
ea.arg = skipwhite(p);
if (ea.cmdidx == CMD_file && *ea.arg != NUL && curbuf_locked()) {
goto doend;
}
if (ea.argt & ARGOPT)
while (ea.arg[0] == '+' && ea.arg[1] == '+')
if (getargopt(&ea) == FAIL && !ni) {
errormsg = (char_u *)_(e_invarg);
goto doend;
}
if (ea.cmdidx == CMD_write || ea.cmdidx == CMD_update) {
if (*ea.arg == '>') { 
if (*++ea.arg != '>') { 
errormsg = (char_u *)_("E494: Use w or w>>");
goto doend;
}
ea.arg = skipwhite(ea.arg + 1);
ea.append = TRUE;
} else if (*ea.arg == '!' && ea.cmdidx == CMD_write) { 
++ea.arg;
ea.usefilter = TRUE;
}
}
if (ea.cmdidx == CMD_read) {
if (ea.forceit) {
ea.usefilter = TRUE; 
ea.forceit = FALSE;
} else if (*ea.arg == '!') { 
++ea.arg;
ea.usefilter = TRUE;
}
}
if (ea.cmdidx == CMD_lshift || ea.cmdidx == CMD_rshift) {
ea.amount = 1;
while (*ea.arg == *ea.cmd) { 
++ea.arg;
++ea.amount;
}
ea.arg = skipwhite(ea.arg);
}
if ((ea.argt & EDITCMD) && !ea.usefilter)
ea.do_ecmd_cmd = getargcmd(&ea.arg);
if ((ea.argt & TRLBAR) && !ea.usefilter) {
separate_nextcmd(&ea);
} else if (ea.cmdidx == CMD_bang
|| ea.cmdidx == CMD_terminal
|| ea.cmdidx == CMD_global
|| ea.cmdidx == CMD_vglobal
|| ea.usefilter) {
for (p = ea.arg; *p; p++) {
if (*p == '\\' && p[1] == '\n') {
STRMOVE(p, p + 1);
} else if (*p == '\n') {
ea.nextcmd = p + 1;
*p = NUL;
break;
}
}
}
if ((ea.argt & DFLALL) && ea.addr_count == 0) {
buf_T *buf;
ea.line1 = 1;
switch (ea.addr_type) {
case ADDR_LINES:
ea.line2 = curbuf->b_ml.ml_line_count;
break;
case ADDR_LOADED_BUFFERS:
buf = firstbuf;
while (buf->b_next != NULL && buf->b_ml.ml_mfp == NULL) {
buf = buf->b_next;
}
ea.line1 = buf->b_fnum;
buf = lastbuf;
while (buf->b_prev != NULL && buf->b_ml.ml_mfp == NULL) {
buf = buf->b_prev;
}
ea.line2 = buf->b_fnum;
break;
case ADDR_BUFFERS:
ea.line1 = firstbuf->b_fnum;
ea.line2 = lastbuf->b_fnum;
break;
case ADDR_WINDOWS:
ea.line2 = LAST_WIN_NR;
break;
case ADDR_TABS:
ea.line2 = LAST_TAB_NR;
break;
case ADDR_TABS_RELATIVE:
ea.line2 = 1;
break;
case ADDR_ARGUMENTS:
if (ARGCOUNT == 0) {
ea.line1 = ea.line2 = 0;
} else {
ea.line2 = ARGCOUNT;
}
break;
case ADDR_QUICKFIX:
ea.line2 = qf_get_size(&ea);
if (ea.line2 == 0) {
ea.line2 = 1;
}
break;
}
}
if ((ea.argt & REGSTR)
&& *ea.arg != NUL
&& (!IS_USER_CMDIDX(ea.cmdidx) || *ea.arg != '=')
&& !((ea.argt & COUNT) && ascii_isdigit(*ea.arg))) {
if (valid_yank_reg(*ea.arg, (ea.cmdidx != CMD_put
&& !IS_USER_CMDIDX(ea.cmdidx)))) {
ea.regname = *ea.arg++;
if (ea.arg[-1] == '=' && ea.arg[0] != NUL) {
set_expr_line(vim_strsave(ea.arg));
ea.arg += STRLEN(ea.arg);
}
ea.arg = skipwhite(ea.arg);
}
}
if ((ea.argt & COUNT) && ascii_isdigit(*ea.arg)
&& (!(ea.argt & BUFNAME) || *(p = skipdigits(ea.arg)) == NUL
|| ascii_iswhite(*p))) {
n = getdigits_long(&ea.arg, false, -1);
ea.arg = skipwhite(ea.arg);
if (n <= 0 && !ni && (ea.argt & ZEROR) == 0) {
errormsg = (char_u *)_(e_zerocount);
goto doend;
}
if (ea.argt & NOTADR) { 
ea.line2 = n;
if (ea.addr_count == 0)
ea.addr_count = 1;
} else {
ea.line1 = ea.line2;
ea.line2 += n - 1;
++ea.addr_count;
if (ea.addr_type == ADDR_LINES
&& ea.line2 > curbuf->b_ml.ml_line_count) {
ea.line2 = curbuf->b_ml.ml_line_count;
}
}
}
if (ea.argt & EXFLAGS)
get_flags(&ea);
if (!ni && !(ea.argt & EXTRA) && *ea.arg != NUL
&& *ea.arg != '"' && (*ea.arg != '|' || (ea.argt & TRLBAR) == 0)) {
errormsg = (char_u *)_(e_trailing);
goto doend;
}
if (!ni && (ea.argt & NEEDARG) && *ea.arg == NUL) {
errormsg = (char_u *)_(e_argreq);
goto doend;
}
if (ea.skip) {
switch (ea.cmdidx) {
case CMD_while:
case CMD_endwhile:
case CMD_for:
case CMD_endfor:
case CMD_if:
case CMD_elseif:
case CMD_else:
case CMD_endif:
case CMD_try:
case CMD_catch:
case CMD_finally:
case CMD_endtry:
case CMD_function:
break;
case CMD_aboveleft:
case CMD_and:
case CMD_belowright:
case CMD_botright:
case CMD_browse:
case CMD_call:
case CMD_confirm:
case CMD_const:
case CMD_delfunction:
case CMD_djump:
case CMD_dlist:
case CMD_dsearch:
case CMD_dsplit:
case CMD_echo:
case CMD_echoerr:
case CMD_echomsg:
case CMD_echon:
case CMD_execute:
case CMD_filter:
case CMD_help:
case CMD_hide:
case CMD_ijump:
case CMD_ilist:
case CMD_isearch:
case CMD_isplit:
case CMD_keepalt:
case CMD_keepjumps:
case CMD_keepmarks:
case CMD_keeppatterns:
case CMD_leftabove:
case CMD_let:
case CMD_lockmarks:
case CMD_lockvar:
case CMD_lua:
case CMD_match:
case CMD_mzscheme:
case CMD_noautocmd:
case CMD_noswapfile:
case CMD_perl:
case CMD_psearch:
case CMD_python:
case CMD_py3:
case CMD_python3:
case CMD_pythonx:
case CMD_pyx:
case CMD_pyxdo:
case CMD_pyxfile:
case CMD_return:
case CMD_rightbelow:
case CMD_ruby:
case CMD_silent:
case CMD_smagic:
case CMD_snomagic:
case CMD_substitute:
case CMD_syntax:
case CMD_tab:
case CMD_tcl:
case CMD_throw:
case CMD_tilde:
case CMD_topleft:
case CMD_unlet:
case CMD_unlockvar:
case CMD_verbose:
case CMD_vertical:
case CMD_wincmd:
break;
default:
goto doend;
}
}
if (ea.argt & XFILE) {
if (expand_filename(&ea, cmdlinep, &errormsg) == FAIL)
goto doend;
}
if ((ea.argt & BUFNAME) && *ea.arg != NUL && ea.addr_count == 0
&& !IS_USER_CMDIDX(ea.cmdidx)
) {
if (ea.cmdidx == CMD_bdelete || ea.cmdidx == CMD_bwipeout
|| ea.cmdidx == CMD_bunload)
p = skiptowhite_esc(ea.arg);
else {
p = ea.arg + STRLEN(ea.arg);
while (p > ea.arg && ascii_iswhite(p[-1]))
--p;
}
ea.line2 = buflist_findpat(ea.arg, p, (ea.argt & BUFUNL) != 0,
FALSE, FALSE);
if (ea.line2 < 0) 
goto doend;
ea.addr_count = 1;
ea.arg = skipwhite(p);
}
if (ea.cmdidx == CMD_try && parsed.did_esilent > 0) {
emsg_silent -= parsed.did_esilent;
if (emsg_silent < 0) {
emsg_silent = 0;
}
parsed.did_esilent = 0;
}
ea.cmdlinep = cmdlinep;
ea.getline = fgetline;
ea.cookie = cookie;
ea.cstack = cstack;
if (IS_USER_CMDIDX(ea.cmdidx)) {
do_ucmd(&ea);
} else {
ea.errmsg = NULL;
(cmdnames[ea.cmdidx].cmd_func)(&ea);
if (ea.errmsg != NULL)
errormsg = (char_u *)_(ea.errmsg);
}
if (need_rethrow)
do_throw(cstack);
else if (check_cstack) {
if (source_finished(fgetline, cookie))
do_finish(&ea, TRUE);
else if (getline_equal(fgetline, cookie, get_func_line)
&& current_func_returned())
do_return(&ea, TRUE, FALSE, NULL);
}
need_rethrow = check_cstack = FALSE;
doend:
if (curwin->w_cursor.lnum == 0) {
curwin->w_cursor.lnum = 1;
curwin->w_cursor.col = 0;
}
if (errormsg != NULL && *errormsg != NUL && !did_emsg) {
if (flags & DOCMD_VERBOSE) {
if (errormsg != IObuff) {
STRCPY(IObuff, errormsg);
errormsg = IObuff;
}
append_command(*cmdlinep);
}
emsg(errormsg);
}
do_errthrow(cstack,
(ea.cmdidx != CMD_SIZE && !IS_USER_CMDIDX(ea.cmdidx))
? cmdnames[(int)ea.cmdidx].cmd_name
: (char_u *)NULL);
if (parsed.verbose_save >= 0) {
p_verbose = parsed.verbose_save;
}
if (cmdmod.save_ei != NULL) {
set_string_option_direct((char_u *)"ei", -1, cmdmod.save_ei,
OPT_FREE, SID_NONE);
free_string_option(cmdmod.save_ei);
}
if (cmdmod.filter_regmatch.regprog != NULL) {
vim_regfree(cmdmod.filter_regmatch.regprog);
}
cmdmod = save_cmdmod;
reg_executing = save_reg_executing;
if (parsed.save_msg_silent != -1) {
if (!did_emsg || msg_silent > parsed.save_msg_silent) {
msg_silent = parsed.save_msg_silent;
}
emsg_silent -= parsed.did_esilent;
if (emsg_silent < 0) {
emsg_silent = 0;
}
msg_scroll = save_msg_scroll;
if (redirecting())
msg_col = 0;
}
if (parsed.did_sandbox) {
sandbox--;
}
if (ea.nextcmd && *ea.nextcmd == NUL) 
ea.nextcmd = NULL;
--ex_nesting_level;
return ea.nextcmd;
}
int parse_cmd_address(exarg_T *eap, char_u **errormsg)
FUNC_ATTR_NONNULL_ALL
{
int address_count = 1;
linenr_T lnum;
for (;;) {
eap->line1 = eap->line2;
switch (eap->addr_type) {
case ADDR_LINES:
eap->line2 = curwin->w_cursor.lnum;
break;
case ADDR_WINDOWS:
eap->line2 = CURRENT_WIN_NR;
break;
case ADDR_ARGUMENTS:
eap->line2 = curwin->w_arg_idx + 1;
if (eap->line2 > ARGCOUNT) {
eap->line2 = ARGCOUNT;
}
break;
case ADDR_LOADED_BUFFERS:
case ADDR_BUFFERS:
eap->line2 = curbuf->b_fnum;
break;
case ADDR_TABS:
eap->line2 = CURRENT_TAB_NR;
break;
case ADDR_TABS_RELATIVE:
eap->line2 = 1;
break;
case ADDR_QUICKFIX:
eap->line2 = qf_get_cur_valid_idx(eap);
break;
}
eap->cmd = skipwhite(eap->cmd);
lnum = get_address(eap, &eap->cmd, eap->addr_type, eap->skip,
eap->addr_count == 0, address_count++);
if (eap->cmd == NULL) { 
return FAIL;
}
if (lnum == MAXLNUM) {
if (*eap->cmd == '%') { 
eap->cmd++;
switch (eap->addr_type) {
case ADDR_LINES:
eap->line1 = 1;
eap->line2 = curbuf->b_ml.ml_line_count;
break;
case ADDR_LOADED_BUFFERS: {
buf_T *buf = firstbuf;
while (buf->b_next != NULL && buf->b_ml.ml_mfp == NULL) {
buf = buf->b_next;
}
eap->line1 = buf->b_fnum;
buf = lastbuf;
while (buf->b_prev != NULL && buf->b_ml.ml_mfp == NULL) {
buf = buf->b_prev;
}
eap->line2 = buf->b_fnum;
break;
}
case ADDR_BUFFERS:
eap->line1 = firstbuf->b_fnum;
eap->line2 = lastbuf->b_fnum;
break;
case ADDR_WINDOWS:
case ADDR_TABS:
if (IS_USER_CMDIDX(eap->cmdidx)) {
eap->line1 = 1;
eap->line2 = eap->addr_type == ADDR_WINDOWS
? LAST_WIN_NR : LAST_TAB_NR;
} else {
*errormsg = (char_u *)_(e_invrange);
return FAIL;
}
break;
case ADDR_TABS_RELATIVE:
*errormsg = (char_u *)_(e_invrange);
return FAIL;
case ADDR_ARGUMENTS:
if (ARGCOUNT == 0) {
eap->line1 = eap->line2 = 0;
} else {
eap->line1 = 1;
eap->line2 = ARGCOUNT;
}
break;
case ADDR_QUICKFIX:
eap->line1 = 1;
eap->line2 = qf_get_size(eap);
if (eap->line2 == 0) {
eap->line2 = 1;
}
break;
}
eap->addr_count++;
} else if (*eap->cmd == '*') {
if (eap->addr_type != ADDR_LINES) {
*errormsg = (char_u *)_(e_invrange);
return FAIL;
}
eap->cmd++;
if (!eap->skip) {
pos_T *fp = getmark('<', false);
if (check_mark(fp) == FAIL) {
return FAIL;
}
eap->line1 = fp->lnum;
fp = getmark('>', false);
if (check_mark(fp) == FAIL) {
return FAIL;
}
eap->line2 = fp->lnum;
eap->addr_count++;
}
}
} else {
eap->line2 = lnum;
}
eap->addr_count++;
if (*eap->cmd == ';') {
if (!eap->skip) {
curwin->w_cursor.lnum = eap->line2;
check_cursor();
}
} else if (*eap->cmd != ',') {
break;
}
eap->cmd++;
}
if (eap->addr_count == 1) {
eap->line1 = eap->line2;
if (lnum == MAXLNUM) {
eap->addr_count = 0;
}
}
return OK;
}
int
checkforcmd(
char_u **pp, 
char *cmd, 
int len 
)
{
int i;
for (i = 0; cmd[i] != NUL; ++i)
if (((char_u *)cmd)[i] != (*pp)[i])
break;
if (i >= len && !isalpha((*pp)[i])) {
*pp = skipwhite(*pp + i);
return TRUE;
}
return FALSE;
}
static void append_command(char_u *cmd)
{
char_u *s = cmd;
char_u *d;
STRCAT(IObuff, ": ");
d = IObuff + STRLEN(IObuff);
while (*s != NUL && d - IObuff < IOSIZE - 7) {
if (
enc_utf8 ? (s[0] == 0xc2 && s[1] == 0xa0) :
*s == 0xa0) {
s +=
enc_utf8 ? 2 :
1;
STRCPY(d, "<a0>");
d += 4;
} else
MB_COPY_CHAR(s, d);
}
*d = NUL;
}
static char_u *find_command(exarg_T *eap, int *full)
{
int len;
char_u *p;
int i;
p = eap->cmd;
if (*p == 'k') {
eap->cmdidx = CMD_k;
++p;
} else if (p[0] == 's'
&& ((p[1] == 'c'
&& (p[2] == NUL
|| (p[2] != 's' && p[2] != 'r'
&& (p[3] == NUL
|| (p[3] != 'i' && p[4] != 'p')))))
|| p[1] == 'g'
|| (p[1] == 'i' && p[2] != 'm' && p[2] != 'l' && p[2] != 'g')
|| p[1] == 'I'
|| (p[1] == 'r' && p[2] != 'e'))) {
eap->cmdidx = CMD_substitute;
++p;
} else {
while (ASCII_ISALPHA(*p))
++p;
if (eap->cmd[0] == 'p' && eap->cmd[1] == 'y')
while (ASCII_ISALNUM(*p))
++p;
if (p == eap->cmd && vim_strchr((char_u *)"@!=><&~#", *p) != NULL)
++p;
len = (int)(p - eap->cmd);
if (*eap->cmd == 'd' && (p[-1] == 'l' || p[-1] == 'p')) {
for (i = 0; i < len; ++i)
if (eap->cmd[i] != ((char_u *)"delete")[i])
break;
if (i == len - 1) {
--len;
if (p[-1] == 'l')
eap->flags |= EXFLAG_LIST;
else
eap->flags |= EXFLAG_PRINT;
}
}
if (ASCII_ISLOWER(eap->cmd[0])) {
const int c1 = eap->cmd[0];
const int c2 = len == 1 ? NUL : eap->cmd[1];
if (command_count != (int)CMD_SIZE) {
iemsg((char *)_("E943: Command table needs to be updated, run 'make'"));
getout(1);
}
eap->cmdidx = cmdidxs1[CharOrdLow(c1)];
if (ASCII_ISLOWER(c2)) {
eap->cmdidx += cmdidxs2[CharOrdLow(c1)][CharOrdLow(c2)];
}
} else {
eap->cmdidx = CMD_bang;
}
for (; (int)eap->cmdidx < (int)CMD_SIZE;
eap->cmdidx = (cmdidx_T)((int)eap->cmdidx + 1))
if (STRNCMP(cmdnames[(int)eap->cmdidx].cmd_name, (char *)eap->cmd,
(size_t)len) == 0) {
if (full != NULL
&& cmdnames[(int)eap->cmdidx].cmd_name[len] == NUL)
*full = TRUE;
break;
}
if ((eap->cmdidx == CMD_SIZE)
&& *eap->cmd >= 'A' && *eap->cmd <= 'Z') {
while (ASCII_ISALNUM(*p))
++p;
p = find_ucmd(eap, p, full, NULL, NULL);
}
if (p == eap->cmd)
eap->cmdidx = CMD_SIZE;
}
return p;
}
static char_u *
find_ucmd (
exarg_T *eap,
char_u *p, 
int *full, 
expand_T *xp, 
int *complp 
)
{
int len = (int)(p - eap->cmd);
int j, k, matchlen = 0;
ucmd_T *uc;
int found = FALSE;
int possible = FALSE;
char_u *cp, *np; 
garray_T *gap;
int amb_local = FALSE; 
gap = &curbuf->b_ucmds;
for (;; ) {
for (j = 0; j < gap->ga_len; ++j) {
uc = USER_CMD_GA(gap, j);
cp = eap->cmd;
np = uc->uc_name;
k = 0;
while (k < len && *np != NUL && *cp++ == *np++)
k++;
if (k == len || (*np == NUL && ascii_isdigit(eap->cmd[k]))) {
if (k == len && found && *np != NUL) {
if (gap == &ucmds)
return NULL;
amb_local = TRUE;
}
if (!found || (k == len && *np == NUL)) {
if (k == len)
found = TRUE;
else
possible = TRUE;
if (gap == &ucmds)
eap->cmdidx = CMD_USER;
else
eap->cmdidx = CMD_USER_BUF;
eap->argt = uc->uc_argt;
eap->useridx = j;
eap->addr_type = uc->uc_addr_type;
if (complp != NULL) {
*complp = uc->uc_compl;
}
if (xp != NULL) {
xp->xp_arg = uc->uc_compl_arg;
xp->xp_script_ctx = uc->uc_script_ctx;
xp->xp_script_ctx.sc_lnum += sourcing_lnum;
}
matchlen = k;
if (k == len && *np == NUL) {
if (full != NULL)
*full = TRUE;
amb_local = FALSE;
break;
}
}
}
}
if (j < gap->ga_len || gap == &ucmds)
break;
gap = &ucmds;
}
if (amb_local) {
if (xp != NULL)
xp->xp_context = EXPAND_UNSUCCESSFUL;
return NULL;
}
if (found || possible)
return p + (matchlen - len);
return p;
}
static struct cmdmod {
char *name;
int minlen;
int has_count; 
} cmdmods[] = {
{ "aboveleft", 3, false },
{ "belowright", 3, false },
{ "botright", 2, false },
{ "browse", 3, false },
{ "confirm", 4, false },
{ "filter", 4, false },
{ "hide", 3, false },
{ "keepalt", 5, false },
{ "keepjumps", 5, false },
{ "keepmarks", 3, false },
{ "keeppatterns", 5, false },
{ "leftabove", 5, false },
{ "lockmarks", 3, false },
{ "noautocmd", 3, false },
{ "noswapfile", 3, false },
{ "rightbelow", 6, false },
{ "sandbox", 3, false },
{ "silent", 3, false },
{ "tab", 3, true },
{ "topleft", 2, false },
{ "unsilent", 3, false },
{ "verbose", 4, true },
{ "vertical", 4, false },
};
int modifier_len(char_u *cmd)
{
int i, j;
char_u *p = cmd;
if (ascii_isdigit(*cmd))
p = skipwhite(skipdigits(cmd));
for (i = 0; i < (int)ARRAY_SIZE(cmdmods); ++i) {
for (j = 0; p[j] != NUL; ++j)
if (p[j] != cmdmods[i].name[j])
break;
if (j >= cmdmods[i].minlen
&& !ASCII_ISALPHA(p[j])
&& (p == cmd || cmdmods[i].has_count)) {
return j + (int)(p - cmd);
}
}
return 0;
}
int cmd_exists(const char *const name)
{
exarg_T ea;
char_u *p;
for (int i = 0; i < (int)ARRAY_SIZE(cmdmods); i++) {
int j;
for (j = 0; name[j] != NUL; j++) {
if (name[j] != (char)cmdmods[i].name[j]) {
break;
}
}
if (name[j] == NUL && j >= cmdmods[i].minlen) {
return cmdmods[i].name[j] == NUL ? 2 : 1;
}
}
ea.cmd = (char_u *)((*name == '2' || *name == '3') ? name + 1 : name);
ea.cmdidx = (cmdidx_T)0;
int full = false;
p = find_command(&ea, &full);
if (p == NULL)
return 3;
if (ascii_isdigit(*name) && ea.cmdidx != CMD_match)
return 0;
if (*skipwhite(p) != NUL)
return 0; 
return ea.cmdidx == CMD_SIZE ? 0 : (full ? 2 : 1);
}
const char * set_one_cmd_context(
expand_T *xp,
const char *buff 
)
{
size_t len = 0;
exarg_T ea;
int context = EXPAND_NOTHING;
bool forceit = false;
bool usefilter = false; 
ExpandInit(xp);
xp->xp_pattern = (char_u *)buff;
xp->xp_context = EXPAND_COMMANDS; 
ea.argt = 0;
const char *cmd;
for (cmd = buff; vim_strchr((const char_u *)" \t:|", *cmd) != NULL; cmd++) {
}
xp->xp_pattern = (char_u *)cmd;
if (*cmd == NUL)
return NULL;
if (*cmd == '"') { 
xp->xp_context = EXPAND_NOTHING;
return NULL;
}
cmd = (const char *)skip_range((const char_u *)cmd, &xp->xp_context);
xp->xp_pattern = (char_u *)cmd;
if (*cmd == NUL) {
return NULL;
}
if (*cmd == '"') {
xp->xp_context = EXPAND_NOTHING;
return NULL;
}
if (*cmd == '|' || *cmd == '\n')
return cmd + 1; 
const char *p;
if (*cmd == 'k' && cmd[1] != 'e') {
ea.cmdidx = CMD_k;
p = cmd + 1;
} else {
p = cmd;
while (ASCII_ISALPHA(*p) || *p == '*') { 
p++;
}
if (ASCII_ISUPPER(cmd[0])) {
while (ASCII_ISALNUM(*p) || *p == '*') {
p++;
}
}
if (cmd[0] == 'p' && cmd[1] == 'y' && p == cmd + 2 && *p == '3') {
p++;
while (ASCII_ISALPHA(*p) || *p == '*') {
p++;
}
}
if (p == cmd && vim_strchr((const char_u *)"@*!=><&~#", *p) != NULL) {
p++;
}
len = (size_t)(p - cmd);
if (len == 0) {
xp->xp_context = EXPAND_UNSUCCESSFUL;
return NULL;
}
for (ea.cmdidx = (cmdidx_T)0; (int)ea.cmdidx < (int)CMD_SIZE;
ea.cmdidx = (cmdidx_T)((int)ea.cmdidx + 1)) {
if (STRNCMP(cmdnames[(int)ea.cmdidx].cmd_name, cmd, len) == 0) {
break;
}
}
if (cmd[0] >= 'A' && cmd[0] <= 'Z') {
while (ASCII_ISALNUM(*p) || *p == '*') { 
p++;
}
}
}
if (*p == NUL && ASCII_ISALNUM(p[-1]))
return NULL;
if (ea.cmdidx == CMD_SIZE) {
if (*cmd == 's' && vim_strchr((const char_u *)"cgriI", cmd[1]) != NULL) {
ea.cmdidx = CMD_substitute;
p = cmd + 1;
} else if (cmd[0] >= 'A' && cmd[0] <= 'Z') {
ea.cmd = (char_u *)cmd;
p = (const char *)find_ucmd(&ea, (char_u *)p, NULL, xp, &context);
if (p == NULL) {
ea.cmdidx = CMD_SIZE; 
}
}
}
if (ea.cmdidx == CMD_SIZE) {
xp->xp_context = EXPAND_UNSUCCESSFUL;
return NULL;
}
xp->xp_context = EXPAND_NOTHING; 
if (*p == '!') { 
forceit = true;
p++;
}
if (!IS_USER_CMDIDX(ea.cmdidx)) {
ea.argt = cmdnames[(int)ea.cmdidx].cmd_argt;
}
const char *arg = (const char *)skipwhite((const char_u *)p);
if (ea.cmdidx == CMD_write || ea.cmdidx == CMD_update) {
if (*arg == '>') { 
if (*++arg == '>') {
arg++;
}
arg = (const char *)skipwhite((const char_u *)arg);
} else if (*arg == '!' && ea.cmdidx == CMD_write) { 
arg++;
usefilter = true;
}
}
if (ea.cmdidx == CMD_read) {
usefilter = forceit; 
if (*arg == '!') { 
arg++;
usefilter = true;
}
}
if (ea.cmdidx == CMD_lshift || ea.cmdidx == CMD_rshift) {
while (*arg == *cmd) { 
arg++;
}
arg = (const char *)skipwhite((const char_u *)arg);
}
if ((ea.argt & EDITCMD) && !usefilter && *arg == '+') {
p = arg + 1;
arg = (const char *)skip_cmd_arg((char_u *)arg, false);
if (*arg == NUL)
return p;
arg = (const char *)skipwhite((const char_u *)arg);
}
if ((ea.argt & TRLBAR) && !usefilter) {
p = arg;
if (ea.cmdidx == CMD_redir && p[0] == '@' && p[1] == '"')
p += 2;
while (*p) {
if (*p == Ctrl_V) {
if (p[1] != NUL)
++p;
} else if ( (*p == '"' && !(ea.argt & NOTRLCOM))
|| *p == '|' || *p == '\n') {
if (*(p - 1) != '\\') {
if (*p == '|' || *p == '\n')
return p + 1;
return NULL; 
}
}
MB_PTR_ADV(p);
}
}
if (!(ea.argt & EXTRA) && *arg != NUL && strchr("|\"", *arg) == NULL) {
return NULL;
}
p = buff;
xp->xp_pattern = (char_u *)p;
len = strlen(buff);
while (*p && p < buff + len) {
if (*p == ' ' || *p == TAB) {
xp->xp_pattern = (char_u *)++p;
} else {
if (*p == '\\' && *(p + 1) != NUL) {
p++; 
}
MB_PTR_ADV(p);
}
}
if (ea.argt & XFILE) {
int c;
int in_quote = false;
const char *bow = NULL; 
xp->xp_pattern = skipwhite((const char_u *)arg);
p = (const char *)xp->xp_pattern;
while (*p != NUL) {
c = utf_ptr2char((const char_u *)p);
if (c == '\\' && p[1] != NUL) {
p++;
} else if (c == '`') {
if (!in_quote) {
xp->xp_pattern = (char_u *)p;
bow = p + 1;
}
in_quote = !in_quote;
}
else if (c == '|'
|| c == '\n'
|| c == '"'
|| ascii_iswhite(c)) {
len = 0; 
while (*p != NUL) {
c = utf_ptr2char((const char_u *)p);
if (c == '`' || vim_isfilec_or_wc(c)) {
break;
}
len = (size_t)utfc_ptr2len((const char_u *)p);
MB_PTR_ADV(p);
}
if (in_quote) {
bow = p;
} else {
xp->xp_pattern = (char_u *)p;
}
p -= len;
}
MB_PTR_ADV(p);
}
if (bow != NULL && in_quote) {
xp->xp_pattern = (char_u *)bow;
}
xp->xp_context = EXPAND_FILES;
if (usefilter || ea.cmdidx == CMD_bang || ea.cmdidx == CMD_terminal) {
#if !defined(BACKSLASH_IN_FILENAME)
xp->xp_shell = TRUE;
#endif
if (xp->xp_pattern == skipwhite((const char_u *)arg)) {
xp->xp_context = EXPAND_SHELLCMD;
}
}
if (*xp->xp_pattern == '$') {
for (p = (const char *)xp->xp_pattern + 1; *p != NUL; p++) {
if (!vim_isIDc((uint8_t)(*p))) {
break;
}
}
if (*p == NUL) {
xp->xp_context = EXPAND_ENV_VARS;
xp->xp_pattern++;
if (context != EXPAND_USER_DEFINED && context != EXPAND_USER_LIST) {
context = EXPAND_ENV_VARS;
}
}
}
if (*xp->xp_pattern == '~') {
for (p = (const char *)xp->xp_pattern + 1; *p != NUL && *p != '/'; p++) {
}
if (*p == NUL && p > (const char *)xp->xp_pattern + 1
&& match_user(xp->xp_pattern + 1) >= 1) {
xp->xp_context = EXPAND_USER;
++xp->xp_pattern;
}
}
}
switch (ea.cmdidx) {
case CMD_find:
case CMD_sfind:
case CMD_tabfind:
if (xp->xp_context == EXPAND_FILES)
xp->xp_context = EXPAND_FILES_IN_PATH;
break;
case CMD_cd:
case CMD_chdir:
case CMD_lcd:
case CMD_lchdir:
case CMD_tcd:
case CMD_tchdir:
if (xp->xp_context == EXPAND_FILES) {
xp->xp_context = EXPAND_DIRECTORIES;
}
break;
case CMD_help:
xp->xp_context = EXPAND_HELP;
xp->xp_pattern = (char_u *)arg;
break;
case CMD_aboveleft:
case CMD_argdo:
case CMD_belowright:
case CMD_botright:
case CMD_browse:
case CMD_bufdo:
case CMD_cdo:
case CMD_cfdo:
case CMD_confirm:
case CMD_debug:
case CMD_folddoclosed:
case CMD_folddoopen:
case CMD_hide:
case CMD_keepalt:
case CMD_keepjumps:
case CMD_keepmarks:
case CMD_keeppatterns:
case CMD_ldo:
case CMD_leftabove:
case CMD_lfdo:
case CMD_lockmarks:
case CMD_noautocmd:
case CMD_noswapfile:
case CMD_rightbelow:
case CMD_sandbox:
case CMD_silent:
case CMD_tab:
case CMD_tabdo:
case CMD_topleft:
case CMD_verbose:
case CMD_vertical:
case CMD_windo:
return arg;
case CMD_filter:
if (*arg != NUL) {
arg = (const char *)skip_vimgrep_pat((char_u *)arg, NULL, NULL);
}
if (arg == NULL || *arg == NUL) {
xp->xp_context = EXPAND_NOTHING;
return NULL;
}
return (const char *)skipwhite((const char_u *)arg);
case CMD_match:
if (*arg == NUL || !ends_excmd(*arg)) {
set_context_in_echohl_cmd(xp, arg);
arg = (const char *)skipwhite(skiptowhite((const char_u *)arg));
if (*arg != NUL) {
xp->xp_context = EXPAND_NOTHING;
arg = (const char *)skip_regexp((char_u *)arg + 1, (uint8_t)(*arg),
p_magic, NULL);
}
}
return (const char *)find_nextcmd((char_u *)arg);
case CMD_command:
while (*arg == '-') {
arg++; 
p = (const char *)skiptowhite((const char_u *)arg);
if (*p == NUL) {
p = strchr(arg, '=');
if (p == NULL) {
xp->xp_context = EXPAND_USER_CMD_FLAGS;
xp->xp_pattern = (char_u *)arg;
return NULL;
}
if (STRNICMP(arg, "complete", p - arg) == 0) {
xp->xp_context = EXPAND_USER_COMPLETE;
xp->xp_pattern = (char_u *)p + 1;
return NULL;
} else if (STRNICMP(arg, "nargs", p - arg) == 0) {
xp->xp_context = EXPAND_USER_NARGS;
xp->xp_pattern = (char_u *)p + 1;
return NULL;
} else if (STRNICMP(arg, "addr", p - arg) == 0) {
xp->xp_context = EXPAND_USER_ADDR_TYPE;
xp->xp_pattern = (char_u *)p + 1;
return NULL;
}
return NULL;
}
arg = (const char *)skipwhite((char_u *)p);
}
p = (const char *)skiptowhite((const char_u *)arg);
if (*p == NUL) {
xp->xp_context = EXPAND_USER_COMMANDS;
xp->xp_pattern = (char_u *)arg;
break;
}
return (const char *)skipwhite((const char_u *)p);
case CMD_delcommand:
xp->xp_context = EXPAND_USER_COMMANDS;
xp->xp_pattern = (char_u *)arg;
break;
case CMD_global:
case CMD_vglobal: {
const int delim = (uint8_t)(*arg); 
if (delim) {
arg++; 
}
while (arg[0] != NUL && (uint8_t)arg[0] != delim) {
if (arg[0] == '\\' && arg[1] != NUL) {
arg++;
}
arg++;
}
if (arg[0] != NUL)
return arg + 1;
break;
}
case CMD_and:
case CMD_substitute: {
const int delim = (uint8_t)(*arg);
if (delim) {
arg++;
arg = (const char *)skip_regexp((char_u *)arg, delim, p_magic, NULL);
}
while (arg[0] != NUL && (uint8_t)arg[0] != delim) {
if (arg[0] == '\\' && arg[1] != NUL) {
arg++;
}
arg++;
}
if (arg[0] != NUL) { 
arg++;
}
while (arg[0] && strchr("|\"#", arg[0]) == NULL) {
arg++;
}
if (arg[0] != NUL) {
return arg;
}
break;
}
case CMD_isearch:
case CMD_dsearch:
case CMD_ilist:
case CMD_dlist:
case CMD_ijump:
case CMD_psearch:
case CMD_djump:
case CMD_isplit:
case CMD_dsplit:
arg = (const char *)skipwhite(skipdigits((const char_u *)arg));
if (*arg == '/') { 
for (++arg; *arg && *arg != '/'; arg++) {
if (*arg == '\\' && arg[1] != NUL) {
arg++;
}
}
if (*arg) {
arg = (const char *)skipwhite((const char_u *)arg + 1);
if (*arg && strchr("|\"\n", *arg) == NULL) {
xp->xp_context = EXPAND_NOTHING;
} else {
return arg;
}
}
}
break;
case CMD_autocmd:
return (const char *)set_context_in_autocmd(xp, (char_u *)arg, false);
case CMD_doautocmd:
case CMD_doautoall:
return (const char *)set_context_in_autocmd(xp, (char_u *)arg, true);
case CMD_set:
set_context_in_set_cmd(xp, (char_u *)arg, 0);
break;
case CMD_setglobal:
set_context_in_set_cmd(xp, (char_u *)arg, OPT_GLOBAL);
break;
case CMD_setlocal:
set_context_in_set_cmd(xp, (char_u *)arg, OPT_LOCAL);
break;
case CMD_tag:
case CMD_stag:
case CMD_ptag:
case CMD_ltag:
case CMD_tselect:
case CMD_stselect:
case CMD_ptselect:
case CMD_tjump:
case CMD_stjump:
case CMD_ptjump:
if (wop_flags & WOP_TAGFILE) {
xp->xp_context = EXPAND_TAGS_LISTFILES;
} else {
xp->xp_context = EXPAND_TAGS;
}
xp->xp_pattern = (char_u *)arg;
break;
case CMD_augroup:
xp->xp_context = EXPAND_AUGROUP;
xp->xp_pattern = (char_u *)arg;
break;
case CMD_syntax:
set_context_in_syntax_cmd(xp, arg);
break;
case CMD_const:
case CMD_let:
case CMD_if:
case CMD_elseif:
case CMD_while:
case CMD_for:
case CMD_echo:
case CMD_echon:
case CMD_execute:
case CMD_echomsg:
case CMD_echoerr:
case CMD_call:
case CMD_return:
case CMD_cexpr:
case CMD_caddexpr:
case CMD_cgetexpr:
case CMD_lexpr:
case CMD_laddexpr:
case CMD_lgetexpr:
set_context_for_expression(xp, (char_u *)arg, ea.cmdidx);
break;
case CMD_unlet:
while ((xp->xp_pattern = (char_u *)strchr(arg, ' ')) != NULL) {
arg = (const char *)xp->xp_pattern + 1;
}
xp->xp_context = EXPAND_USER_VARS;
xp->xp_pattern = (char_u *)arg;
if (*xp->xp_pattern == '$') {
xp->xp_context = EXPAND_ENV_VARS;
xp->xp_pattern++;
}
break;
case CMD_function:
case CMD_delfunction:
xp->xp_context = EXPAND_USER_FUNC;
xp->xp_pattern = (char_u *)arg;
break;
case CMD_echohl:
set_context_in_echohl_cmd(xp, arg);
break;
case CMD_highlight:
set_context_in_highlight_cmd(xp, arg);
break;
case CMD_cscope:
case CMD_lcscope:
case CMD_scscope:
set_context_in_cscope_cmd(xp, arg, ea.cmdidx);
break;
case CMD_sign:
set_context_in_sign_cmd(xp, (char_u *)arg);
break;
case CMD_bdelete:
case CMD_bwipeout:
case CMD_bunload:
while ((xp->xp_pattern = (char_u *)strchr(arg, ' ')) != NULL) {
arg = (const char *)xp->xp_pattern + 1;
}
FALLTHROUGH;
case CMD_buffer:
case CMD_sbuffer:
case CMD_checktime:
xp->xp_context = EXPAND_BUFFERS;
xp->xp_pattern = (char_u *)arg;
break;
case CMD_USER:
case CMD_USER_BUF:
if (context != EXPAND_NOTHING) {
if (!(ea.argt & XFILE)) {
if (context == EXPAND_MENUS) {
return (const char *)set_context_in_menu_cmd(xp, (char_u *)cmd,
(char_u *)arg, forceit);
} else if (context == EXPAND_COMMANDS) {
return arg;
} else if (context == EXPAND_MAPPINGS) {
return (const char *)set_context_in_map_cmd(
xp, (char_u *)"map", (char_u *)arg, forceit, false, false,
CMD_map);
}
p = arg;
while (*p) {
if (*p == ' ') {
arg = p + 1;
} else if (*p == '\\' && *(p + 1) != NUL) {
p++; 
}
MB_PTR_ADV(p);
}
xp->xp_pattern = (char_u *)arg;
}
xp->xp_context = context;
}
break;
case CMD_map: case CMD_noremap:
case CMD_nmap: case CMD_nnoremap:
case CMD_vmap: case CMD_vnoremap:
case CMD_omap: case CMD_onoremap:
case CMD_imap: case CMD_inoremap:
case CMD_cmap: case CMD_cnoremap:
case CMD_lmap: case CMD_lnoremap:
case CMD_smap: case CMD_snoremap:
case CMD_xmap: case CMD_xnoremap:
return (const char *)set_context_in_map_cmd(
xp, (char_u *)cmd, (char_u *)arg, forceit, false, false, ea.cmdidx);
case CMD_unmap:
case CMD_nunmap:
case CMD_vunmap:
case CMD_ounmap:
case CMD_iunmap:
case CMD_cunmap:
case CMD_lunmap:
case CMD_sunmap:
case CMD_xunmap:
return (const char *)set_context_in_map_cmd(
xp, (char_u *)cmd, (char_u *)arg, forceit, false, true, ea.cmdidx);
case CMD_mapclear:
case CMD_nmapclear:
case CMD_vmapclear:
case CMD_omapclear:
case CMD_imapclear:
case CMD_cmapclear:
case CMD_lmapclear:
case CMD_smapclear:
case CMD_xmapclear:
xp->xp_context = EXPAND_MAPCLEAR;
xp->xp_pattern = (char_u *)arg;
break;
case CMD_abbreviate: case CMD_noreabbrev:
case CMD_cabbrev: case CMD_cnoreabbrev:
case CMD_iabbrev: case CMD_inoreabbrev:
return (const char *)set_context_in_map_cmd(
xp, (char_u *)cmd, (char_u *)arg, forceit, true, false, ea.cmdidx);
case CMD_unabbreviate:
case CMD_cunabbrev:
case CMD_iunabbrev:
return (const char *)set_context_in_map_cmd(
xp, (char_u *)cmd, (char_u *)arg, forceit, true, true, ea.cmdidx);
case CMD_menu: case CMD_noremenu: case CMD_unmenu:
case CMD_amenu: case CMD_anoremenu: case CMD_aunmenu:
case CMD_nmenu: case CMD_nnoremenu: case CMD_nunmenu:
case CMD_vmenu: case CMD_vnoremenu: case CMD_vunmenu:
case CMD_omenu: case CMD_onoremenu: case CMD_ounmenu:
case CMD_imenu: case CMD_inoremenu: case CMD_iunmenu:
case CMD_cmenu: case CMD_cnoremenu: case CMD_cunmenu:
case CMD_tmenu: case CMD_tunmenu:
case CMD_popup: case CMD_emenu:
return (const char *)set_context_in_menu_cmd(
xp, (char_u *)cmd, (char_u *)arg, forceit);
case CMD_colorscheme:
xp->xp_context = EXPAND_COLORS;
xp->xp_pattern = (char_u *)arg;
break;
case CMD_compiler:
xp->xp_context = EXPAND_COMPILER;
xp->xp_pattern = (char_u *)arg;
break;
case CMD_ownsyntax:
xp->xp_context = EXPAND_OWNSYNTAX;
xp->xp_pattern = (char_u *)arg;
break;
case CMD_setfiletype:
xp->xp_context = EXPAND_FILETYPE;
xp->xp_pattern = (char_u *)arg;
break;
case CMD_packadd:
xp->xp_context = EXPAND_PACKADD;
xp->xp_pattern = (char_u *)arg;
break;
#if defined(HAVE_WORKING_LIBINTL)
case CMD_language:
p = (const char *)skiptowhite((const char_u *)arg);
if (*p == NUL) {
xp->xp_context = EXPAND_LANGUAGE;
xp->xp_pattern = (char_u *)arg;
} else {
if (strncmp(arg, "messages", p - arg) == 0
|| strncmp(arg, "ctype", p - arg) == 0
|| strncmp(arg, "time", p - arg) == 0) {
xp->xp_context = EXPAND_LOCALES;
xp->xp_pattern = skipwhite((const char_u *)p);
} else {
xp->xp_context = EXPAND_NOTHING;
}
}
break;
#endif
case CMD_profile:
set_context_in_profile_cmd(xp, arg);
break;
case CMD_checkhealth:
xp->xp_context = EXPAND_CHECKHEALTH;
xp->xp_pattern = (char_u *)arg;
break;
case CMD_behave:
xp->xp_context = EXPAND_BEHAVE;
xp->xp_pattern = (char_u *)arg;
break;
case CMD_messages:
xp->xp_context = EXPAND_MESSAGES;
xp->xp_pattern = (char_u *)arg;
break;
case CMD_history:
xp->xp_context = EXPAND_HISTORY;
xp->xp_pattern = (char_u *)arg;
break;
case CMD_syntime:
xp->xp_context = EXPAND_SYNTIME;
xp->xp_pattern = (char_u *)arg;
break;
case CMD_argdelete:
while ((xp->xp_pattern = vim_strchr((const char_u *)arg, ' ')) != NULL) {
arg = (const char *)(xp->xp_pattern + 1);
}
xp->xp_context = EXPAND_ARGLIST;
xp->xp_pattern = (char_u *)arg;
break;
default:
break;
}
return NULL;
}
char_u *skip_range(
const char_u *cmd,
int *ctx 
)
{
unsigned delim;
while (vim_strchr((char_u *)" \t0123456789.$%'/?-+,;\\", *cmd) != NULL) {
if (*cmd == '\\') {
if (cmd[1] == '?' || cmd[1] == '/' || cmd[1] == '&') {
cmd++;
} else {
break;
}
} else if (*cmd == '\'') {
if (*++cmd == NUL && ctx != NULL) {
*ctx = EXPAND_NOTHING;
}
} else if (*cmd == '/' || *cmd == '?') {
delim = *cmd++;
while (*cmd != NUL && *cmd != delim)
if (*cmd++ == '\\' && *cmd != NUL)
++cmd;
if (*cmd == NUL && ctx != NULL)
*ctx = EXPAND_NOTHING;
}
if (*cmd != NUL)
++cmd;
}
cmd = skip_colon_white(cmd, false);
return (char_u *)cmd;
}
static linenr_T get_address(exarg_T *eap,
char_u **ptr,
int addr_type, 
int skip, 
int to_other_file, 
int address_count) 
{
int c;
int i;
long n;
char_u *cmd;
pos_T pos;
pos_T *fp;
linenr_T lnum;
buf_T *buf;
cmd = skipwhite(*ptr);
lnum = MAXLNUM;
do {
switch (*cmd) {
case '.': 
++cmd;
switch (addr_type) {
case ADDR_LINES:
lnum = curwin->w_cursor.lnum;
break;
case ADDR_WINDOWS:
lnum = CURRENT_WIN_NR;
break;
case ADDR_ARGUMENTS:
lnum = curwin->w_arg_idx + 1;
break;
case ADDR_LOADED_BUFFERS:
case ADDR_BUFFERS:
lnum = curbuf->b_fnum;
break;
case ADDR_TABS:
lnum = CURRENT_TAB_NR;
break;
case ADDR_TABS_RELATIVE:
EMSG(_(e_invrange));
cmd = NULL;
goto error;
break;
case ADDR_QUICKFIX:
lnum = qf_get_cur_valid_idx(eap);
break;
}
break;
case '$': 
++cmd;
switch (addr_type) {
case ADDR_LINES:
lnum = curbuf->b_ml.ml_line_count;
break;
case ADDR_WINDOWS:
lnum = LAST_WIN_NR;
break;
case ADDR_ARGUMENTS:
lnum = ARGCOUNT;
break;
case ADDR_LOADED_BUFFERS:
buf = lastbuf;
while (buf->b_ml.ml_mfp == NULL) {
if (buf->b_prev == NULL) {
break;
}
buf = buf->b_prev;
}
lnum = buf->b_fnum;
break;
case ADDR_BUFFERS:
lnum = lastbuf->b_fnum;
break;
case ADDR_TABS:
lnum = LAST_TAB_NR;
break;
case ADDR_TABS_RELATIVE:
EMSG(_(e_invrange));
cmd = NULL;
goto error;
break;
case ADDR_QUICKFIX:
lnum = qf_get_size(eap);
if (lnum == 0) {
lnum = 1;
}
break;
}
break;
case '\'': 
if (*++cmd == NUL) {
cmd = NULL;
goto error;
}
if (addr_type != ADDR_LINES) {
EMSG(_(e_invaddr));
cmd = NULL;
goto error;
}
if (skip)
++cmd;
else {
fp = getmark(*cmd, to_other_file && cmd[1] == NUL);
++cmd;
if (fp == (pos_T *)-1)
lnum = curwin->w_cursor.lnum;
else {
if (check_mark(fp) == FAIL) {
cmd = NULL;
goto error;
}
lnum = fp->lnum;
}
}
break;
case '/':
case '?': 
c = *cmd++;
if (addr_type != ADDR_LINES) {
EMSG(_(e_invaddr));
cmd = NULL;
goto error;
}
if (skip) { 
cmd = skip_regexp(cmd, c, p_magic, NULL);
if (*cmd == c)
++cmd;
} else {
pos = curwin->w_cursor; 
if (lnum != MAXLNUM)
curwin->w_cursor.lnum = lnum;
if (c == '/' && curwin->w_cursor.lnum > 0) {
curwin->w_cursor.col = MAXCOL;
} else {
curwin->w_cursor.col = 0;
}
searchcmdlen = 0;
if (!do_search(NULL, c, cmd, 1L, SEARCH_HIS | SEARCH_MSG, NULL)) {
curwin->w_cursor = pos;
cmd = NULL;
goto error;
}
lnum = curwin->w_cursor.lnum;
curwin->w_cursor = pos;
cmd += searchcmdlen;
}
break;
case '\\': 
++cmd;
if (addr_type != ADDR_LINES) {
EMSG(_(e_invaddr));
cmd = NULL;
goto error;
}
if (*cmd == '&')
i = RE_SUBST;
else if (*cmd == '?' || *cmd == '/')
i = RE_SEARCH;
else {
EMSG(_(e_backslash));
cmd = NULL;
goto error;
}
if (!skip) {
pos.lnum = (lnum != MAXLNUM) ? lnum : curwin->w_cursor.lnum;
pos.col = (*cmd != '?') ? MAXCOL : 0;
pos.coladd = 0;
if (searchit(curwin, curbuf, &pos, NULL,
*cmd == '?' ? BACKWARD : FORWARD,
(char_u *)"", 1L, SEARCH_MSG, i, NULL) != FAIL) {
lnum = pos.lnum;
} else {
cmd = NULL;
goto error;
}
}
++cmd;
break;
default:
if (ascii_isdigit(*cmd)) { 
lnum = getdigits_long(&cmd, false, 0);
}
}
for (;; ) {
cmd = skipwhite(cmd);
if (*cmd != '-' && *cmd != '+' && !ascii_isdigit(*cmd)) {
break;
}
if (lnum == MAXLNUM) {
switch (addr_type) {
case ADDR_LINES:
lnum = curwin->w_cursor.lnum; 
break;
case ADDR_WINDOWS:
lnum = CURRENT_WIN_NR;
break;
case ADDR_ARGUMENTS:
lnum = curwin->w_arg_idx + 1;
break;
case ADDR_LOADED_BUFFERS:
case ADDR_BUFFERS:
lnum = curbuf->b_fnum;
break;
case ADDR_TABS:
lnum = CURRENT_TAB_NR;
break;
case ADDR_TABS_RELATIVE:
lnum = 1;
break;
case ADDR_QUICKFIX:
lnum = qf_get_cur_valid_idx(eap);
break;
}
}
if (ascii_isdigit(*cmd)) {
i = '+'; 
} else {
i = *cmd++;
}
if (!ascii_isdigit(*cmd)) { 
n = 1;
} else {
n = getdigits(&cmd, true, 0);
}
if (addr_type == ADDR_TABS_RELATIVE) {
EMSG(_(e_invrange));
cmd = NULL;
goto error;
} else if (addr_type == ADDR_LOADED_BUFFERS || addr_type == ADDR_BUFFERS) {
lnum = compute_buffer_local_count(
addr_type, lnum, (i == '-') ? -1 * n : n);
} else {
if (addr_type == ADDR_LINES && (i == '-' || i == '+')
&& address_count >= 2) {
(void)hasFolding(lnum, NULL, &lnum);
}
if (i == '-') {
lnum -= n;
} else {
lnum += n;
}
}
}
} while (*cmd == '/' || *cmd == '?');
error:
*ptr = cmd;
return lnum;
}
static void get_flags(exarg_T *eap)
{
while (vim_strchr((char_u *)"lp#", *eap->arg) != NULL) {
if (*eap->arg == 'l')
eap->flags |= EXFLAG_LIST;
else if (*eap->arg == 'p')
eap->flags |= EXFLAG_PRINT;
else
eap->flags |= EXFLAG_NR;
eap->arg = skipwhite(eap->arg + 1);
}
}
void ex_ni(exarg_T *eap)
{
if (!eap->skip)
eap->errmsg = (char_u *)N_(
"E319: The command is not available in this version");
}
static void ex_script_ni(exarg_T *eap)
{
if (!eap->skip) {
ex_ni(eap);
} else {
size_t len;
xfree(script_get(eap, &len));
}
}
static char_u *invalid_range(exarg_T *eap)
{
buf_T *buf;
if (eap->line1 < 0 || eap->line2 < 0 || eap->line1 > eap->line2) {
return (char_u *)_(e_invrange);
}
if (eap->argt & RANGE) {
switch(eap->addr_type) {
case ADDR_LINES:
if (!(eap->argt & NOTADR)
&& eap->line2 > curbuf->b_ml.ml_line_count
+ (eap->cmdidx == CMD_diffget)) {
return (char_u *)_(e_invrange);
}
break;
case ADDR_ARGUMENTS:
if (eap->line2 > ARGCOUNT + (!ARGCOUNT)) {
return (char_u *)_(e_invrange);
}
break;
case ADDR_BUFFERS:
if (eap->line1 < firstbuf->b_fnum
|| eap->line2 > lastbuf->b_fnum) {
return (char_u *)_(e_invrange);
}
break;
case ADDR_LOADED_BUFFERS:
buf = firstbuf;
while (buf->b_ml.ml_mfp == NULL) {
if (buf->b_next == NULL) {
return (char_u *)_(e_invrange);
}
buf = buf->b_next;
}
if (eap->line1 < buf->b_fnum) {
return (char_u *)_(e_invrange);
}
buf = lastbuf;
while (buf->b_ml.ml_mfp == NULL) {
if (buf->b_prev == NULL) {
return (char_u *)_(e_invrange);
}
buf = buf->b_prev;
}
if (eap->line2 > buf->b_fnum) {
return (char_u *)_(e_invrange);
}
break;
case ADDR_WINDOWS:
if (eap->line2 > LAST_WIN_NR) {
return (char_u *)_(e_invrange);
}
break;
case ADDR_TABS:
if (eap->line2 > LAST_TAB_NR) {
return (char_u *)_(e_invrange);
}
break;
case ADDR_TABS_RELATIVE:
break;
case ADDR_QUICKFIX:
assert(eap->line2 >= 0);
if (eap->line2 != 1 && (size_t)eap->line2 > qf_get_size(eap)) {
return (char_u *)_(e_invrange);
}
break;
}
}
return NULL;
}
static void correct_range(exarg_T *eap)
{
if (!(eap->argt & ZEROR)) { 
if (eap->line1 == 0)
eap->line1 = 1;
if (eap->line2 == 0)
eap->line2 = 1;
}
}
static char_u *skip_grep_pat(exarg_T *eap)
{
char_u *p = eap->arg;
if (*p != NUL && (eap->cmdidx == CMD_vimgrep || eap->cmdidx == CMD_lvimgrep
|| eap->cmdidx == CMD_vimgrepadd
|| eap->cmdidx == CMD_lvimgrepadd
|| grep_internal(eap->cmdidx))) {
p = skip_vimgrep_pat(p, NULL, NULL);
if (p == NULL)
p = eap->arg;
}
return p;
}
static char_u *replace_makeprg(exarg_T *eap, char_u *p, char_u **cmdlinep)
{
char_u *new_cmdline;
char_u *program;
char_u *pos;
char_u *ptr;
int len;
int i;
if ((eap->cmdidx == CMD_make || eap->cmdidx == CMD_lmake
|| eap->cmdidx == CMD_grep || eap->cmdidx == CMD_lgrep
|| eap->cmdidx == CMD_grepadd
|| eap->cmdidx == CMD_lgrepadd)
&& !grep_internal(eap->cmdidx)) {
if (eap->cmdidx == CMD_grep || eap->cmdidx == CMD_lgrep
|| eap->cmdidx == CMD_grepadd || eap->cmdidx == CMD_lgrepadd) {
if (*curbuf->b_p_gp == NUL)
program = p_gp;
else
program = curbuf->b_p_gp;
} else {
if (*curbuf->b_p_mp == NUL)
program = p_mp;
else
program = curbuf->b_p_mp;
}
p = skipwhite(p);
if ((pos = (char_u *)strstr((char *)program, "$*")) != NULL) {
i = 1;
while ((pos = (char_u *)strstr((char *)pos + 2, "$*")) != NULL)
++i;
len = (int)STRLEN(p);
new_cmdline = xmalloc(STRLEN(program) + i * (len - 2) + 1);
ptr = new_cmdline;
while ((pos = (char_u *)strstr((char *)program, "$*")) != NULL) {
i = (int)(pos - program);
memcpy(ptr, program, i);
STRCPY(ptr += i, p);
ptr += len;
program = pos + 2;
}
STRCPY(ptr, program);
} else {
new_cmdline = xmalloc(STRLEN(program) + STRLEN(p) + 2);
STRCPY(new_cmdline, program);
STRCAT(new_cmdline, " ");
STRCAT(new_cmdline, p);
}
msg_make(p);
xfree(*cmdlinep);
*cmdlinep = new_cmdline;
p = new_cmdline;
}
return p;
}
int expand_filename(exarg_T *eap, char_u **cmdlinep, char_u **errormsgp)
{
int has_wildcards; 
char_u *repl;
size_t srclen;
char_u *p;
int escaped;
p = skip_grep_pat(eap);
has_wildcards = path_has_wildcard(p);
while (*p != NUL) {
if (p[0] == '`' && p[1] == '=') {
p += 2;
(void)skip_expr(&p);
if (*p == '`')
++p;
continue;
}
if (vim_strchr((char_u *)"%#<", *p) == NULL) {
++p;
continue;
}
repl = eval_vars(p, eap->arg, &srclen, &(eap->do_ecmd_lnum),
errormsgp, &escaped);
if (*errormsgp != NULL) 
return FAIL;
if (repl == NULL) { 
p += srclen;
continue;
}
if (vim_strchr(repl, '$') != NULL || vim_strchr(repl, '~') != NULL) {
char_u *l = repl;
repl = expand_env_save(repl);
xfree(l);
}
if (!eap->usefilter
&& !escaped
&& eap->cmdidx != CMD_bang
&& eap->cmdidx != CMD_grep
&& eap->cmdidx != CMD_grepadd
&& eap->cmdidx != CMD_hardcopy
&& eap->cmdidx != CMD_lgrep
&& eap->cmdidx != CMD_lgrepadd
&& eap->cmdidx != CMD_lmake
&& eap->cmdidx != CMD_make
&& eap->cmdidx != CMD_terminal
&& !(eap->argt & NOSPC)
) {
char_u *l;
#if defined(BACKSLASH_IN_FILENAME)
static char_u *nobslash = (char_u *)" \t\"|";
#define ESCAPE_CHARS nobslash
#else
#define ESCAPE_CHARS escape_chars
#endif
for (l = repl; *l; ++l)
if (vim_strchr(ESCAPE_CHARS, *l) != NULL) {
l = vim_strsave_escaped(repl, ESCAPE_CHARS);
xfree(repl);
repl = l;
break;
}
}
if ((eap->usefilter
|| eap->cmdidx == CMD_bang
|| eap->cmdidx == CMD_terminal)
&& vim_strpbrk(repl, (char_u *)"!") != NULL) {
char_u *l;
l = vim_strsave_escaped(repl, (char_u *)"!");
xfree(repl);
repl = l;
}
p = repl_cmdline(eap, p, srclen, repl, cmdlinep);
xfree(repl);
}
if ((eap->argt & NOSPC) && !eap->usefilter) {
if (has_wildcards) {
if (vim_strchr(eap->arg, '$') != NULL
|| vim_strchr(eap->arg, '~') != NULL) {
expand_env_esc(eap->arg, NameBuff, MAXPATHL,
TRUE, TRUE, NULL);
has_wildcards = path_has_wildcard(NameBuff);
p = NameBuff;
} else
p = NULL;
if (p != NULL) {
(void)repl_cmdline(eap, eap->arg, STRLEN(eap->arg), p, cmdlinep);
}
}
#if defined(UNIX)
if (!has_wildcards)
#endif
backslash_halve(eap->arg);
if (has_wildcards) {
expand_T xpc;
int options = WILD_LIST_NOTFOUND | WILD_NOERROR | WILD_ADD_SLASH;
ExpandInit(&xpc);
xpc.xp_context = EXPAND_FILES;
if (p_wic)
options += WILD_ICASE;
p = ExpandOne(&xpc, eap->arg, NULL, options, WILD_EXPAND_FREE);
if (p == NULL) {
return FAIL;
}
(void)repl_cmdline(eap, eap->arg, STRLEN(eap->arg), p, cmdlinep);
xfree(p);
}
}
return OK;
}
static char_u *repl_cmdline(exarg_T *eap, char_u *src, size_t srclen,
char_u *repl, char_u **cmdlinep)
{
size_t len = STRLEN(repl);
size_t i = (size_t)(src - *cmdlinep) + STRLEN(src + srclen) + len + 3;
if (eap->nextcmd != NULL)
i += STRLEN(eap->nextcmd); 
char_u *new_cmdline = xmalloc(i);
i = (size_t)(src - *cmdlinep); 
memmove(new_cmdline, *cmdlinep, i);
memmove(new_cmdline + i, repl, len);
i += len; 
STRCPY(new_cmdline + i, src + srclen);
src = new_cmdline + i; 
if (eap->nextcmd != NULL) { 
i = STRLEN(new_cmdline) + 1;
STRCPY(new_cmdline + i, eap->nextcmd);
eap->nextcmd = new_cmdline + i;
}
eap->cmd = new_cmdline + (eap->cmd - *cmdlinep);
eap->arg = new_cmdline + (eap->arg - *cmdlinep);
if (eap->do_ecmd_cmd != NULL && eap->do_ecmd_cmd != dollar_command)
eap->do_ecmd_cmd = new_cmdline + (eap->do_ecmd_cmd - *cmdlinep);
xfree(*cmdlinep);
*cmdlinep = new_cmdline;
return src;
}
void separate_nextcmd(exarg_T *eap)
{
char_u *p;
p = skip_grep_pat(eap);
for (; *p; MB_PTR_ADV(p)) {
if (*p == Ctrl_V) {
if (eap->argt & (USECTRLV | XFILE))
++p; 
else
STRMOVE(p, p + 1);
if (*p == NUL) 
break;
}
else if (p[0] == '`' && p[1] == '=' && (eap->argt & XFILE)) {
p += 2;
(void)skip_expr(&p);
}
else if ((*p == '"' && !(eap->argt & NOTRLCOM)
&& (eap->cmdidx != CMD_at || p != eap->arg)
&& (eap->cmdidx != CMD_redir
|| p != eap->arg + 1 || p[-1] != '@'))
|| *p == '|' || *p == '\n') {
if ((vim_strchr(p_cpo, CPO_BAR) == NULL
|| !(eap->argt & USECTRLV)) && *(p - 1) == '\\') {
STRMOVE(p - 1, p); 
--p;
} else {
eap->nextcmd = check_nextcmd(p);
*p = NUL;
break;
}
}
}
if (!(eap->argt & NOTRLCOM)) 
del_trailing_spaces(eap->arg);
}
static char_u *getargcmd(char_u **argp)
{
char_u *arg = *argp;
char_u *command = NULL;
if (*arg == '+') { 
++arg;
if (ascii_isspace(*arg) || *arg == '\0')
command = dollar_command;
else {
command = arg;
arg = skip_cmd_arg(command, TRUE);
if (*arg != NUL)
*arg++ = NUL; 
}
arg = skipwhite(arg); 
*argp = arg;
}
return command;
}
static char_u *
skip_cmd_arg (
char_u *p,
int rembs 
)
{
while (*p && !ascii_isspace(*p)) {
if (*p == '\\' && p[1] != NUL) {
if (rembs)
STRMOVE(p, p + 1);
else
++p;
}
MB_PTR_ADV(p);
}
return p;
}
int get_bad_opt(const char_u *p, exarg_T *eap)
FUNC_ATTR_NONNULL_ALL
{
if (STRICMP(p, "keep") == 0) {
eap->bad_char = BAD_KEEP;
} else if (STRICMP(p, "drop") == 0) {
eap->bad_char = BAD_DROP;
} else if (MB_BYTE2LEN(*p) == 1 && p[1] == NUL) {
eap->bad_char = *p;
} else {
return FAIL;
}
return OK;
}
static int getargopt(exarg_T *eap)
{
char_u *arg = eap->arg + 2;
int *pp = NULL;
int bad_char_idx;
char_u *p;
if (STRNCMP(arg, "bin", 3) == 0 || STRNCMP(arg, "nobin", 5) == 0) {
if (*arg == 'n') {
arg += 2;
eap->force_bin = FORCE_NOBIN;
} else
eap->force_bin = FORCE_BIN;
if (!checkforcmd(&arg, "binary", 3))
return FAIL;
eap->arg = skipwhite(arg);
return OK;
}
if (STRNCMP(arg, "edit", 4) == 0) {
eap->read_edit = TRUE;
eap->arg = skipwhite(arg + 4);
return OK;
}
if (STRNCMP(arg, "ff", 2) == 0) {
arg += 2;
pp = &eap->force_ff;
} else if (STRNCMP(arg, "fileformat", 10) == 0) {
arg += 10;
pp = &eap->force_ff;
} else if (STRNCMP(arg, "enc", 3) == 0) {
if (STRNCMP(arg, "encoding", 8) == 0)
arg += 8;
else
arg += 3;
pp = &eap->force_enc;
} else if (STRNCMP(arg, "bad", 3) == 0) {
arg += 3;
pp = &bad_char_idx;
}
if (pp == NULL || *arg != '=')
return FAIL;
++arg;
*pp = (int)(arg - eap->cmd);
arg = skip_cmd_arg(arg, FALSE);
eap->arg = skipwhite(arg);
*arg = NUL;
if (pp == &eap->force_ff) {
if (check_ff_value(eap->cmd + eap->force_ff) == FAIL) {
return FAIL;
}
eap->force_ff = eap->cmd[eap->force_ff];
} else if (pp == &eap->force_enc) {
for (p = eap->cmd + eap->force_enc; *p != NUL; ++p)
*p = TOLOWER_ASC(*p);
} else {
if (get_bad_opt(eap->cmd + bad_char_idx, eap) == FAIL) {
return FAIL;
}
}
return OK;
}
static int get_tabpage_arg(exarg_T *eap)
{
int tab_number = 0;
int unaccept_arg0 = (eap->cmdidx == CMD_tabmove) ? 0 : 1;
if (eap->arg && *eap->arg != NUL) {
char_u *p = eap->arg;
char_u *p_save;
int relative = 0; 
if (*p == '-') {
relative = -1;
p++;
} else if (*p == '+') {
relative = 1;
p++;
}
p_save = p;
tab_number = getdigits(&p, false, tab_number);
if (relative == 0) {
if (STRCMP(p, "$") == 0) {
tab_number = LAST_TAB_NR;
} else if (STRCMP(p, "#") == 0) {
tab_number = tabpage_index(lastused_tabpage);
} else if (p == p_save || *p_save == '-' || *p != NUL
|| tab_number > LAST_TAB_NR) {
eap->errmsg = e_invarg;
goto theend;
}
} else {
if (*p_save == NUL) {
tab_number = 1;
}
else if (p == p_save || *p_save == '-' || *p != NUL || tab_number == 0) {
eap->errmsg = e_invarg;
goto theend;
}
tab_number = tab_number * relative + tabpage_index(curtab);
if (!unaccept_arg0 && relative == -1) {
--tab_number;
}
}
if (tab_number < unaccept_arg0 || tab_number > LAST_TAB_NR) {
eap->errmsg = e_invarg;
}
} else if (eap->addr_count > 0) {
if (unaccept_arg0 && eap->line2 == 0) {
eap->errmsg = e_invrange;
tab_number = 0;
} else {
tab_number = eap->line2;
if (!unaccept_arg0 && *skipwhite(*eap->cmdlinep) == '-') {
tab_number--;
if (tab_number < unaccept_arg0) {
eap->errmsg = e_invarg;
}
}
}
} else {
switch (eap->cmdidx) {
case CMD_tabnext:
tab_number = tabpage_index(curtab) + 1;
if (tab_number > LAST_TAB_NR)
tab_number = 1;
break;
case CMD_tabmove:
tab_number = LAST_TAB_NR;
break;
default:
tab_number = tabpage_index(curtab);
}
}
theend:
return tab_number;
}
static void ex_abbreviate(exarg_T *eap)
{
do_exmap(eap, TRUE); 
}
static void ex_map(exarg_T *eap)
{
if (secure) {
secure = 2;
msg_outtrans(eap->cmd);
msg_putchar('\n');
}
do_exmap(eap, FALSE);
}
static void ex_unmap(exarg_T *eap)
{
do_exmap(eap, FALSE);
}
static void ex_mapclear(exarg_T *eap)
{
map_clear_mode(eap->cmd, eap->arg, eap->forceit, false);
}
static void ex_abclear(exarg_T *eap)
{
map_clear_mode(eap->cmd, eap->arg, true, true);
}
static void ex_autocmd(exarg_T *eap)
{
if (secure) {
secure = 2;
eap->errmsg = e_curdir;
} else if (eap->cmdidx == CMD_autocmd)
do_autocmd(eap->arg, eap->forceit);
else
do_augroup(eap->arg, eap->forceit);
}
static void ex_doautocmd(exarg_T *eap)
{
char_u *arg = eap->arg;
int call_do_modelines = check_nomodeline(&arg);
bool did_aucmd;
(void)do_doautocmd(arg, false, &did_aucmd);
if (call_do_modelines && did_aucmd) {
do_modelines(0);
}
}
static void ex_bunload(exarg_T *eap)
{
eap->errmsg = do_bufdel(
eap->cmdidx == CMD_bdelete ? DOBUF_DEL
: eap->cmdidx == CMD_bwipeout ? DOBUF_WIPE
: DOBUF_UNLOAD, eap->arg,
eap->addr_count, (int)eap->line1, (int)eap->line2, eap->forceit);
}
static void ex_buffer(exarg_T *eap)
{
if (*eap->arg) {
eap->errmsg = e_trailing;
} else {
if (eap->addr_count == 0) { 
goto_buffer(eap, DOBUF_CURRENT, FORWARD, 0);
} else {
goto_buffer(eap, DOBUF_FIRST, FORWARD, (int)eap->line2);
}
if (eap->do_ecmd_cmd != NULL) {
do_cmdline_cmd((char *)eap->do_ecmd_cmd);
}
}
}
static void ex_bmodified(exarg_T *eap)
{
goto_buffer(eap, DOBUF_MOD, FORWARD, (int)eap->line2);
if (eap->do_ecmd_cmd != NULL) {
do_cmdline_cmd((char *)eap->do_ecmd_cmd);
}
}
static void ex_bnext(exarg_T *eap)
{
goto_buffer(eap, DOBUF_CURRENT, FORWARD, (int)eap->line2);
if (eap->do_ecmd_cmd != NULL) {
do_cmdline_cmd((char *)eap->do_ecmd_cmd);
}
}
static void ex_bprevious(exarg_T *eap)
{
goto_buffer(eap, DOBUF_CURRENT, BACKWARD, (int)eap->line2);
if (eap->do_ecmd_cmd != NULL) {
do_cmdline_cmd((char *)eap->do_ecmd_cmd);
}
}
static void ex_brewind(exarg_T *eap)
{
goto_buffer(eap, DOBUF_FIRST, FORWARD, 0);
if (eap->do_ecmd_cmd != NULL) {
do_cmdline_cmd((char *)eap->do_ecmd_cmd);
}
}
static void ex_blast(exarg_T *eap)
{
goto_buffer(eap, DOBUF_LAST, BACKWARD, 0);
if (eap->do_ecmd_cmd != NULL) {
do_cmdline_cmd((char *)eap->do_ecmd_cmd);
}
}
int ends_excmd(int c) FUNC_ATTR_CONST
{
return c == NUL || c == '|' || c == '"' || c == '\n';
}
char_u *find_nextcmd(const char_u *p)
{
while (*p != '|' && *p != '\n') {
if (*p == NUL) {
return NULL;
}
p++;
}
return (char_u *)p + 1;
}
char_u *check_nextcmd(char_u *p)
{
char_u *s = skipwhite(p);
if (*s == '|' || *s == '\n') {
return (s + 1);
} else {
return NULL;
}
}
static int
check_more(
int message, 
int forceit
)
{
int n = ARGCOUNT - curwin->w_arg_idx - 1;
if (!forceit && only_one_window()
&& ARGCOUNT > 1 && !arg_had_last && n >= 0 && quitmore == 0) {
if (message) {
if ((p_confirm || cmdmod.confirm) && curbuf->b_fname != NULL) {
char_u buff[DIALOG_MSG_SIZE];
if (n == 1)
STRLCPY(buff, _("1 more file to edit. Quit anyway?"),
DIALOG_MSG_SIZE);
else
vim_snprintf((char *)buff, DIALOG_MSG_SIZE,
_("%d more files to edit. Quit anyway?"), n);
if (vim_dialog_yesno(VIM_QUESTION, NULL, buff, 1) == VIM_YES)
return OK;
return FAIL;
}
if (n == 1)
EMSG(_("E173: 1 more file to edit"));
else
EMSGN(_("E173: %" PRId64 " more files to edit"), n);
quitmore = 2; 
}
return FAIL;
}
return OK;
}
char_u *get_command_name(expand_T *xp, int idx)
{
if (idx >= (int)CMD_SIZE)
return get_user_command_name(idx);
return cmdnames[idx].cmd_name;
}
static int uc_add_command(char_u *name, size_t name_len, char_u *rep,
uint32_t argt, long def, int flags, int compl,
char_u *compl_arg, int addr_type, int force)
{
ucmd_T *cmd = NULL;
char_u *p;
int i;
int cmp = 1;
char_u *rep_buf = NULL;
garray_T *gap;
replace_termcodes(rep, STRLEN(rep), &rep_buf, false, false, true,
CPO_TO_CPO_FLAGS);
if (rep_buf == NULL) {
rep_buf = vim_strsave(rep);
}
if (flags & UC_BUFFER) {
gap = &curbuf->b_ucmds;
if (gap->ga_itemsize == 0)
ga_init(gap, (int)sizeof(ucmd_T), 4);
} else
gap = &ucmds;
for (i = 0; i < gap->ga_len; ++i) {
size_t len;
cmd = USER_CMD_GA(gap, i);
len = STRLEN(cmd->uc_name);
cmp = STRNCMP(name, cmd->uc_name, name_len);
if (cmp == 0) {
if (name_len < len)
cmp = -1;
else if (name_len > len)
cmp = 1;
}
if (cmp == 0) {
if (!force) {
EMSG(_("E174: Command already exists: add ! to replace it"));
goto fail;
}
XFREE_CLEAR(cmd->uc_rep);
XFREE_CLEAR(cmd->uc_compl_arg);
break;
}
if (cmp < 0)
break;
}
if (cmp != 0) {
ga_grow(gap, 1);
p = vim_strnsave(name, (int)name_len);
cmd = USER_CMD_GA(gap, i);
memmove(cmd + 1, cmd, (gap->ga_len - i) * sizeof(ucmd_T));
++gap->ga_len;
cmd->uc_name = p;
}
cmd->uc_rep = rep_buf;
cmd->uc_argt = argt;
cmd->uc_def = def;
cmd->uc_compl = compl;
cmd->uc_script_ctx = current_sctx;
cmd->uc_script_ctx.sc_lnum += sourcing_lnum;
cmd->uc_compl_arg = compl_arg;
cmd->uc_addr_type = addr_type;
return OK;
fail:
xfree(rep_buf);
xfree(compl_arg);
return FAIL;
}
static struct {
int expand;
char *name;
} addr_type_complete[] =
{
{ ADDR_ARGUMENTS, "arguments" },
{ ADDR_LINES, "lines" },
{ ADDR_LOADED_BUFFERS, "loaded_buffers" },
{ ADDR_TABS, "tabs" },
{ ADDR_BUFFERS, "buffers" },
{ ADDR_WINDOWS, "windows" },
{ ADDR_QUICKFIX, "quickfix" },
{ -1, NULL }
};
static const char *command_complete[] =
{
[EXPAND_ARGLIST] = "arglist",
[EXPAND_AUGROUP] = "augroup",
[EXPAND_BEHAVE] = "behave",
[EXPAND_BUFFERS] = "buffer",
[EXPAND_CHECKHEALTH] = "checkhealth",
[EXPAND_COLORS] = "color",
[EXPAND_COMMANDS] = "command",
[EXPAND_COMPILER] = "compiler",
[EXPAND_CSCOPE] = "cscope",
[EXPAND_USER_DEFINED] = "custom",
[EXPAND_USER_LIST] = "customlist",
[EXPAND_DIRECTORIES] = "dir",
[EXPAND_ENV_VARS] = "environment",
[EXPAND_EVENTS] = "event",
[EXPAND_EXPRESSION] = "expression",
[EXPAND_FILES] = "file",
[EXPAND_FILES_IN_PATH] = "file_in_path",
[EXPAND_FILETYPE] = "filetype",
[EXPAND_FUNCTIONS] = "function",
[EXPAND_HELP] = "help",
[EXPAND_HIGHLIGHT] = "highlight",
[EXPAND_HISTORY] = "history",
#if defined(HAVE_WORKING_LIBINTL)
[EXPAND_LOCALES] = "locale",
#endif
[EXPAND_MAPCLEAR] = "mapclear",
[EXPAND_MAPPINGS] = "mapping",
[EXPAND_MENUS] = "menu",
[EXPAND_MESSAGES] = "messages",
[EXPAND_OWNSYNTAX] = "syntax",
[EXPAND_SYNTIME] = "syntime",
[EXPAND_SETTINGS] = "option",
[EXPAND_PACKADD] = "packadd",
[EXPAND_SHELLCMD] = "shellcmd",
[EXPAND_SIGN] = "sign",
[EXPAND_TAGS] = "tag",
[EXPAND_TAGS_LISTFILES] = "tag_listfiles",
[EXPAND_USER] = "user",
[EXPAND_USER_VARS] = "var",
};
static char *get_command_complete(int arg)
{
if (arg >= (int)(ARRAY_SIZE(command_complete))) {
return NULL;
} else {
return (char *)command_complete[arg];
}
}
static void uc_list(char_u *name, size_t name_len)
{
int i, j;
bool found = false;
ucmd_T *cmd;
uint32_t a;
garray_T *gap = (cmdwin_type != 0 && get_cmdline_type() == NUL)
? &prevwin->w_buffer->b_ucmds
: &curbuf->b_ucmds;
for (;; ) {
for (i = 0; i < gap->ga_len; ++i) {
cmd = USER_CMD_GA(gap, i);
a = cmd->uc_argt;
if (STRNCMP(name, cmd->uc_name, name_len) != 0
|| message_filtered(cmd->uc_name)) {
continue;
}
if (!found) {
MSG_PUTS_TITLE(_("\n Name Args Address "
"Complete Definition"));
}
found = true;
msg_putchar('\n');
if (got_int)
break;
int len = 4;
if (a & BANG) {
msg_putchar('!');
len--;
}
if (a & REGSTR) {
msg_putchar('"');
len--;
}
if (gap != &ucmds) {
msg_putchar('b');
len--;
}
if (a & TRLBAR) {
msg_putchar('|');
len--;
}
while (len-- > 0) {
msg_putchar(' ');
}
msg_outtrans_attr(cmd->uc_name, HL_ATTR(HLF_D));
len = (int)STRLEN(cmd->uc_name) + 4;
do {
msg_putchar(' ');
len++;
} while (len < 22);
const int over = len - 22;
len = 0;
switch (a & (EXTRA|NOSPC|NEEDARG)) {
case 0:
IObuff[len++] = '0';
break;
case (EXTRA):
IObuff[len++] = '*';
break;
case (EXTRA|NOSPC):
IObuff[len++] = '?';
break;
case (EXTRA|NEEDARG):
IObuff[len++] = '+';
break;
case (EXTRA|NOSPC|NEEDARG):
IObuff[len++] = '1';
break;
}
do {
IObuff[len++] = ' ';
} while (len < 5 - over);
if (a & (RANGE|COUNT)) {
if (a & COUNT) {
snprintf((char *)IObuff + len, IOSIZE, "%" PRId64 "c",
(int64_t)cmd->uc_def);
len += (int)STRLEN(IObuff + len);
} else if (a & DFLALL) {
IObuff[len++] = '%';
} else if (cmd->uc_def >= 0) {
snprintf((char *)IObuff + len, IOSIZE, "%" PRId64 "",
(int64_t)cmd->uc_def);
len += (int)STRLEN(IObuff + len);
} else {
IObuff[len++] = '.';
}
}
do {
IObuff[len++] = ' ';
} while (len < 9 - over);
for (j = 0; addr_type_complete[j].expand != -1; j++) {
if (addr_type_complete[j].expand != ADDR_LINES
&& addr_type_complete[j].expand == cmd->uc_addr_type) {
STRCPY(IObuff + len, addr_type_complete[j].name);
len += (int)STRLEN(IObuff + len);
break;
}
}
do {
IObuff[len++] = ' ';
} while (len < 13 - over);
char *cmd_compl = get_command_complete(cmd->uc_compl);
if (cmd_compl != NULL) {
STRCPY(IObuff + len, get_command_complete(cmd->uc_compl));
len += (int)STRLEN(IObuff + len);
}
do {
IObuff[len++] = ' ';
} while (len < 24 - over);
IObuff[len] = '\0';
msg_outtrans(IObuff);
msg_outtrans_special(cmd->uc_rep, false,
name_len == 0 ? Columns - 46 : 0);
if (p_verbose > 0) {
last_set_msg(cmd->uc_script_ctx);
}
line_breakcheck();
if (got_int) {
break;
}
}
if (gap == &ucmds || i < gap->ga_len)
break;
gap = &ucmds;
}
if (!found)
MSG(_("No user-defined commands found"));
}
static int uc_scan_attr(char_u *attr, size_t len, uint32_t *argt, long *def,
int *flags, int *complp, char_u **compl_arg,
int *addr_type_arg)
{
char_u *p;
if (len == 0) {
EMSG(_("E175: No attribute specified"));
return FAIL;
}
if (STRNICMP(attr, "bang", len) == 0)
*argt |= BANG;
else if (STRNICMP(attr, "buffer", len) == 0)
*flags |= UC_BUFFER;
else if (STRNICMP(attr, "register", len) == 0)
*argt |= REGSTR;
else if (STRNICMP(attr, "bar", len) == 0)
*argt |= TRLBAR;
else {
int i;
char_u *val = NULL;
size_t vallen = 0;
size_t attrlen = len;
for (i = 0; i < (int)len; ++i) {
if (attr[i] == '=') {
val = &attr[i + 1];
vallen = len - i - 1;
attrlen = i;
break;
}
}
if (STRNICMP(attr, "nargs", attrlen) == 0) {
if (vallen == 1) {
if (*val == '0')
;
else if (*val == '1')
*argt |= (EXTRA | NOSPC | NEEDARG);
else if (*val == '*')
*argt |= EXTRA;
else if (*val == '?')
*argt |= (EXTRA | NOSPC);
else if (*val == '+')
*argt |= (EXTRA | NEEDARG);
else
goto wrong_nargs;
} else {
wrong_nargs:
EMSG(_("E176: Invalid number of arguments"));
return FAIL;
}
} else if (STRNICMP(attr, "range", attrlen) == 0) {
*argt |= RANGE;
if (vallen == 1 && *val == '%')
*argt |= DFLALL;
else if (val != NULL) {
p = val;
if (*def >= 0) {
two_count:
EMSG(_("E177: Count cannot be specified twice"));
return FAIL;
}
*def = getdigits_long(&p, true, 0);
*argt |= (ZEROR | NOTADR);
if (p != val + vallen || vallen == 0) {
invalid_count:
EMSG(_("E178: Invalid default value for count"));
return FAIL;
}
}
} else if (STRNICMP(attr, "count", attrlen) == 0) {
*argt |= (COUNT | ZEROR | RANGE | NOTADR);
if (val != NULL) {
p = val;
if (*def >= 0)
goto two_count;
*def = getdigits_long(&p, true, 0);
if (p != val + vallen)
goto invalid_count;
}
if (*def < 0)
*def = 0;
} else if (STRNICMP(attr, "complete", attrlen) == 0) {
if (val == NULL) {
EMSG(_("E179: argument required for -complete"));
return FAIL;
}
if (parse_compl_arg(val, (int)vallen, complp, argt, compl_arg)
== FAIL) {
return FAIL;
}
} else if (STRNICMP(attr, "addr", attrlen) == 0) {
*argt |= RANGE;
if (val == NULL) {
EMSG(_("E179: argument required for -addr"));
return FAIL;
}
if (parse_addr_type_arg(val, (int)vallen, argt, addr_type_arg) == FAIL) {
return FAIL;
}
if (addr_type_arg != ADDR_LINES) {
*argt |= (ZEROR | NOTADR);
}
} else {
char_u ch = attr[len];
attr[len] = '\0';
EMSG2(_("E181: Invalid attribute: %s"), attr);
attr[len] = ch;
return FAIL;
}
}
return OK;
}
static void ex_command(exarg_T *eap)
{
char_u *name;
char_u *end;
char_u *p;
uint32_t argt = 0;
long def = -1;
int flags = 0;
int compl = EXPAND_NOTHING;
char_u *compl_arg = NULL;
int addr_type_arg = ADDR_LINES;
int has_attr = (eap->arg[0] == '-');
int name_len;
p = eap->arg;
while (*p == '-') {
++p;
end = skiptowhite(p);
if (uc_scan_attr(p, end - p, &argt, &def, &flags, &compl, &compl_arg,
&addr_type_arg) == FAIL) {
return;
}
p = skipwhite(end);
}
name = p;
if (ASCII_ISALPHA(*p)) {
while (ASCII_ISALNUM(*p)) {
p++;
}
}
if (!ends_excmd(*p) && !ascii_iswhite(*p)) {
EMSG(_("E182: Invalid command name"));
return;
}
end = p;
name_len = (int)(end - name);
p = skipwhite(end);
if (!has_attr && ends_excmd(*p)) {
uc_list(name, end - name);
} else if (!ASCII_ISUPPER(*name)) {
EMSG(_("E183: User defined commands must start with an uppercase letter"));
return;
} else if (name_len <= 4 && STRNCMP(name, "Next", name_len) == 0) {
EMSG(_("E841: Reserved name, cannot be used for user defined command"));
return;
} else {
uc_add_command(name, end - name, p, argt, def, flags, compl, compl_arg,
addr_type_arg, eap->forceit);
}
}
void ex_comclear(exarg_T *eap)
{
uc_clear(&ucmds);
uc_clear(&curbuf->b_ucmds);
}
static void free_ucmd(ucmd_T* cmd) {
xfree(cmd->uc_name);
xfree(cmd->uc_rep);
xfree(cmd->uc_compl_arg);
}
void uc_clear(garray_T *gap)
{
GA_DEEP_CLEAR(gap, ucmd_T, free_ucmd);
}
static void ex_delcommand(exarg_T *eap)
{
int i = 0;
ucmd_T *cmd = NULL;
int cmp = -1;
garray_T *gap;
gap = &curbuf->b_ucmds;
for (;; ) {
for (i = 0; i < gap->ga_len; ++i) {
cmd = USER_CMD_GA(gap, i);
cmp = STRCMP(eap->arg, cmd->uc_name);
if (cmp <= 0)
break;
}
if (gap == &ucmds || cmp == 0)
break;
gap = &ucmds;
}
if (cmp != 0) {
EMSG2(_("E184: No such user-defined command: %s"), eap->arg);
return;
}
xfree(cmd->uc_name);
xfree(cmd->uc_rep);
xfree(cmd->uc_compl_arg);
--gap->ga_len;
if (i < gap->ga_len)
memmove(cmd, cmd + 1, (gap->ga_len - i) * sizeof(ucmd_T));
}
static char_u *uc_split_args(char_u *arg, size_t *lenp)
{
char_u *buf;
char_u *p;
char_u *q;
int len;
p = arg;
len = 2; 
while (*p) {
if (p[0] == '\\' && p[1] == '\\') {
len += 2;
p += 2;
} else if (p[0] == '\\' && ascii_iswhite(p[1])) {
len += 1;
p += 2;
} else if (*p == '\\' || *p == '"') {
len += 2;
p += 1;
} else if (ascii_iswhite(*p)) {
p = skipwhite(p);
if (*p == NUL)
break;
len += 3; 
} else {
int charlen = (*mb_ptr2len)(p);
len += charlen;
p += charlen;
}
}
buf = xmalloc(len + 1);
p = arg;
q = buf;
*q++ = '"';
while (*p) {
if (p[0] == '\\' && p[1] == '\\') {
*q++ = '\\';
*q++ = '\\';
p += 2;
} else if (p[0] == '\\' && ascii_iswhite(p[1])) {
*q++ = p[1];
p += 2;
} else if (*p == '\\' || *p == '"') {
*q++ = '\\';
*q++ = *p++;
} else if (ascii_iswhite(*p)) {
p = skipwhite(p);
if (*p == NUL)
break;
*q++ = '"';
*q++ = ',';
*q++ = '"';
} else {
MB_COPY_CHAR(p, q);
}
}
*q++ = '"';
*q = 0;
*lenp = len;
return buf;
}
static size_t add_cmd_modifier(char_u *buf, char *mod_str, bool *multi_mods)
{
size_t result = STRLEN(mod_str);
if (*multi_mods) {
result++;
}
if (buf != NULL) {
if (*multi_mods) {
STRCAT(buf, " ");
}
STRCAT(buf, mod_str);
}
*multi_mods = true;
return result;
}
static size_t
uc_check_code(
char_u *code,
size_t len,
char_u *buf,
ucmd_T *cmd, 
exarg_T *eap, 
char_u **split_buf,
size_t *split_len
)
{
size_t result = 0;
char_u *p = code + 1;
size_t l = len - 2;
int quote = 0;
enum {
ct_ARGS,
ct_BANG,
ct_COUNT,
ct_LINE1,
ct_LINE2,
ct_RANGE,
ct_MODS,
ct_REGISTER,
ct_LT,
ct_NONE
} type = ct_NONE;
if ((vim_strchr((char_u *)"qQfF", *p) != NULL) && p[1] == '-') {
quote = (*p == 'q' || *p == 'Q') ? 1 : 2;
p += 2;
l -= 2;
}
l++;
if (l <= 1) {
type = ct_NONE;
} else if (STRNICMP(p, "args>", l) == 0) {
type = ct_ARGS;
} else if (STRNICMP(p, "bang>", l) == 0) {
type = ct_BANG;
} else if (STRNICMP(p, "count>", l) == 0) {
type = ct_COUNT;
} else if (STRNICMP(p, "line1>", l) == 0) {
type = ct_LINE1;
} else if (STRNICMP(p, "line2>", l) == 0) {
type = ct_LINE2;
} else if (STRNICMP(p, "range>", l) == 0) {
type = ct_RANGE;
} else if (STRNICMP(p, "lt>", l) == 0) {
type = ct_LT;
} else if (STRNICMP(p, "reg>", l) == 0 || STRNICMP(p, "register>", l) == 0) {
type = ct_REGISTER;
} else if (STRNICMP(p, "mods>", l) == 0) {
type = ct_MODS;
}
switch (type) {
case ct_ARGS:
if (*eap->arg == NUL) {
if (quote == 1) {
result = 2;
if (buf != NULL)
STRCPY(buf, "''");
} else
result = 0;
break;
}
if ((eap->argt & NOSPC) && quote == 2)
quote = 1;
switch (quote) {
case 0: 
result = STRLEN(eap->arg);
if (buf != NULL)
STRCPY(buf, eap->arg);
break;
case 1: 
result = STRLEN(eap->arg) + 2;
for (p = eap->arg; *p; p++) {
if (*p == '\\' || *p == '"') {
result++;
}
}
if (buf != NULL) {
*buf++ = '"';
for (p = eap->arg; *p; p++) {
if (*p == '\\' || *p == '"') {
*buf++ = '\\';
}
*buf++ = *p;
}
*buf = '"';
}
break;
case 2: 
if (*split_buf == NULL)
*split_buf = uc_split_args(eap->arg, split_len);
result = *split_len;
if (buf != NULL && result != 0)
STRCPY(buf, *split_buf);
break;
}
break;
case ct_BANG:
result = eap->forceit ? 1 : 0;
if (quote)
result += 2;
if (buf != NULL) {
if (quote)
*buf++ = '"';
if (eap->forceit)
*buf++ = '!';
if (quote)
*buf = '"';
}
break;
case ct_LINE1:
case ct_LINE2:
case ct_RANGE:
case ct_COUNT:
{
char num_buf[20];
long num = (type == ct_LINE1) ? eap->line1 :
(type == ct_LINE2) ? eap->line2 :
(type == ct_RANGE) ? eap->addr_count :
(eap->addr_count > 0) ? eap->line2 : cmd->uc_def;
size_t num_len;
sprintf(num_buf, "%" PRId64, (int64_t)num);
num_len = STRLEN(num_buf);
result = num_len;
if (quote)
result += 2;
if (buf != NULL) {
if (quote)
*buf++ = '"';
STRCPY(buf, num_buf);
buf += num_len;
if (quote)
*buf = '"';
}
break;
}
case ct_MODS:
{
result = quote ? 2 : 0;
if (buf != NULL) {
if (quote) {
*buf++ = '"';
}
*buf = '\0';
}
bool multi_mods = false;
if (cmdmod.split & WSP_ABOVE) {
result += add_cmd_modifier(buf, "aboveleft", &multi_mods);
}
if (cmdmod.split & WSP_BELOW) {
result += add_cmd_modifier(buf, "belowright", &multi_mods);
}
if (cmdmod.split & WSP_BOT) {
result += add_cmd_modifier(buf, "botright", &multi_mods);
}
typedef struct {
bool *set;
char *name;
} mod_entry_T;
static mod_entry_T mod_entries[] = {
{ &cmdmod.browse, "browse" },
{ &cmdmod.confirm, "confirm" },
{ &cmdmod.hide, "hide" },
{ &cmdmod.keepalt, "keepalt" },
{ &cmdmod.keepjumps, "keepjumps" },
{ &cmdmod.keepmarks, "keepmarks" },
{ &cmdmod.keeppatterns, "keeppatterns" },
{ &cmdmod.lockmarks, "lockmarks" },
{ &cmdmod.noswapfile, "noswapfile" }
};
for (size_t i = 0; i < ARRAY_SIZE(mod_entries); i++) {
if (*mod_entries[i].set) {
result += add_cmd_modifier(buf, mod_entries[i].name, &multi_mods);
}
}
if (msg_silent > 0) {
result += add_cmd_modifier(buf, emsg_silent > 0 ? "silent!" : "silent",
&multi_mods);
}
if (cmdmod.tab > 0) {
result += add_cmd_modifier(buf, "tab", &multi_mods);
}
if (cmdmod.split & WSP_TOP) {
result += add_cmd_modifier(buf, "topleft", &multi_mods);
}
if (p_verbose > 0) {
result += add_cmd_modifier(buf, "verbose", &multi_mods);
}
if (cmdmod.split & WSP_VERT) {
result += add_cmd_modifier(buf, "vertical", &multi_mods);
}
if (quote && buf != NULL) {
buf += result - 2;
*buf = '"';
}
break;
}
case ct_REGISTER:
result = eap->regname ? 1 : 0;
if (quote)
result += 2;
if (buf != NULL) {
if (quote)
*buf++ = '\'';
if (eap->regname)
*buf++ = eap->regname;
if (quote)
*buf = '\'';
}
break;
case ct_LT:
result = 1;
if (buf != NULL)
*buf = '<';
break;
default:
result = (size_t)-1;
if (buf != NULL)
*buf = '<';
break;
}
return result;
}
static void do_ucmd(exarg_T *eap)
{
char_u *buf;
char_u *p;
char_u *q;
char_u *start;
char_u *end = NULL;
char_u *ksp;
size_t len, totlen;
size_t split_len = 0;
char_u *split_buf = NULL;
ucmd_T *cmd;
const sctx_T save_current_sctx = current_sctx;
if (eap->cmdidx == CMD_USER)
cmd = USER_CMD(eap->useridx);
else
cmd = USER_CMD_GA(&curbuf->b_ucmds, eap->useridx);
buf = NULL;
for (;; ) {
p = cmd->uc_rep; 
q = buf; 
totlen = 0;
for (;; ) {
start = vim_strchr(p, '<');
if (start != NULL)
end = vim_strchr(start + 1, '>');
if (buf != NULL) {
for (ksp = p; *ksp != NUL && *ksp != K_SPECIAL; ksp++) {
}
if (*ksp == K_SPECIAL
&& (start == NULL || ksp < start || end == NULL)
&& (ksp[1] == KS_SPECIAL && ksp[2] == KE_FILLER)) {
len = ksp - p;
if (len > 0) {
memmove(q, p, len);
q += len;
}
*q++ = K_SPECIAL;
p = ksp + 3;
continue;
}
}
if (start == NULL || end == NULL)
break;
++end;
len = start - p;
if (buf == NULL)
totlen += len;
else {
memmove(q, p, len);
q += len;
}
len = uc_check_code(start, end - start, q, cmd, eap,
&split_buf, &split_len);
if (len == (size_t)-1) {
p = start + 1;
len = 1;
} else
p = end;
if (buf == NULL)
totlen += len;
else
q += len;
}
if (buf != NULL) { 
STRCPY(q, p);
break;
}
totlen += STRLEN(p); 
buf = xmalloc(totlen + 1);
}
current_sctx.sc_sid = cmd->uc_script_ctx.sc_sid;
(void)do_cmdline(buf, eap->getline, eap->cookie,
DOCMD_VERBOSE|DOCMD_NOWAIT|DOCMD_KEYTYPED);
current_sctx = save_current_sctx;
xfree(buf);
xfree(split_buf);
}
static char_u *get_user_command_name(int idx)
{
return get_user_commands(NULL, idx - (int)CMD_SIZE);
}
char_u *get_user_cmd_addr_type(expand_T *xp, int idx)
{
return (char_u *)addr_type_complete[idx].name;
}
char_u *get_user_commands(expand_T *xp FUNC_ATTR_UNUSED, int idx)
FUNC_ATTR_PURE FUNC_ATTR_WARN_UNUSED_RESULT
{
const buf_T *const buf = (cmdwin_type != 0 && get_cmdline_type() == NUL)
? prevwin->w_buffer
: curbuf;
if (idx < buf->b_ucmds.ga_len) {
return USER_CMD_GA(&buf->b_ucmds, idx)->uc_name;
}
idx -= buf->b_ucmds.ga_len;
if (idx < ucmds.ga_len) {
return USER_CMD(idx)->uc_name;
}
return NULL;
}
char_u *get_user_cmd_flags(expand_T *xp, int idx)
{
static char *user_cmd_flags[] = {"addr", "bang", "bar",
"buffer", "complete", "count",
"nargs", "range", "register"};
if (idx >= (int)ARRAY_SIZE(user_cmd_flags))
return NULL;
return (char_u *)user_cmd_flags[idx];
}
char_u *get_user_cmd_nargs(expand_T *xp, int idx)
{
static char *user_cmd_nargs[] = {"0", "1", "*", "?", "+"};
if (idx >= (int)ARRAY_SIZE(user_cmd_nargs))
return NULL;
return (char_u *)user_cmd_nargs[idx];
}
char_u *get_user_cmd_complete(expand_T *xp, int idx)
{
if (idx >= (int)ARRAY_SIZE(command_complete)) {
return NULL;
}
char *cmd_compl = get_command_complete(idx);
if (cmd_compl == NULL) {
return (char_u *)"";
} else {
return (char_u *)cmd_compl;
}
}
int parse_addr_type_arg(char_u *value, int vallen, uint32_t *argt,
int *addr_type_arg)
{
int i, a, b;
for (i = 0; addr_type_complete[i].expand != -1; i++) {
a = (int)STRLEN(addr_type_complete[i].name) == vallen;
b = STRNCMP(value, addr_type_complete[i].name, vallen) == 0;
if (a && b) {
*addr_type_arg = addr_type_complete[i].expand;
break;
}
}
if (addr_type_complete[i].expand == -1) {
char_u *err = value;
for (i = 0; err[i] != NUL && !ascii_iswhite(err[i]); i++) {}
err[i] = NUL;
EMSG2(_("E180: Invalid address type value: %s"), err);
return FAIL;
}
if (*addr_type_arg != ADDR_LINES)
*argt |= NOTADR;
return OK;
}
int parse_compl_arg(const char_u *value, int vallen, int *complp,
uint32_t *argt, char_u **compl_arg)
{
const char_u *arg = NULL;
size_t arglen = 0;
int i;
int valend = vallen;
for (i = 0; i < vallen; ++i) {
if (value[i] == ',') {
arg = &value[i + 1];
arglen = vallen - i - 1;
valend = i;
break;
}
}
for (i = 0; i < (int)ARRAY_SIZE(command_complete); i++) {
if (get_command_complete(i) == NULL) {
continue;
}
if ((int)STRLEN(command_complete[i]) == valend
&& STRNCMP(value, command_complete[i], valend) == 0) {
*complp = i;
if (i == EXPAND_BUFFERS) {
*argt |= BUFNAME;
} else if (i == EXPAND_DIRECTORIES || i == EXPAND_FILES) {
*argt |= XFILE;
}
break;
}
}
if (i == (int)ARRAY_SIZE(command_complete)) {
EMSG2(_("E180: Invalid complete value: %s"), value);
return FAIL;
}
if (*complp != EXPAND_USER_DEFINED && *complp != EXPAND_USER_LIST
&& arg != NULL) {
EMSG(_("E468: Completion argument only allowed for custom completion"));
return FAIL;
}
if ((*complp == EXPAND_USER_DEFINED || *complp == EXPAND_USER_LIST)
&& arg == NULL) {
EMSG(_("E467: Custom completion requires a function argument"));
return FAIL;
}
if (arg != NULL)
*compl_arg = vim_strnsave(arg, (int)arglen);
return OK;
}
int cmdcomplete_str_to_type(char_u *complete_str)
{
for (int i = 0; i < (int)(ARRAY_SIZE(command_complete)); i++) {
char *cmd_compl = get_command_complete(i);
if (cmd_compl == NULL) {
continue;
}
if (STRCMP(complete_str, command_complete[i]) == 0) {
return i;
}
}
return EXPAND_NOTHING;
}
static void ex_colorscheme(exarg_T *eap)
{
if (*eap->arg == NUL) {
char_u *expr = vim_strsave((char_u *)"g:colors_name");
char_u *p = NULL;
++emsg_off;
p = eval_to_string(expr, NULL, FALSE);
--emsg_off;
xfree(expr);
if (p != NULL) {
MSG(p);
xfree(p);
} else
MSG("default");
} else if (load_colors(eap->arg) == FAIL)
EMSG2(_("E185: Cannot find color scheme '%s'"), eap->arg);
}
static void ex_highlight(exarg_T *eap)
{
if (*eap->arg == NUL && eap->cmd[2] == '!') {
MSG(_("Greetings, Vim user!"));
}
do_highlight((const char *)eap->arg, eap->forceit, false);
}
void not_exiting(void)
{
exiting = false;
}
static bool before_quit_autocmds(win_T *wp, bool quit_all, int forceit)
{
apply_autocmds(EVENT_QUITPRE, NULL, NULL, false, wp->w_buffer);
if (!win_valid(wp)
|| curbuf_locked()
|| (wp->w_buffer->b_nwindows == 1 && wp->w_buffer->b_locked > 0)) {
return true;
}
if (quit_all
|| (check_more(false, forceit) == OK && only_one_window())) {
apply_autocmds(EVENT_EXITPRE, NULL, NULL, false, curbuf);
if (!win_valid(wp)
|| curbuf_locked()
|| (curbuf->b_nwindows == 1 && curbuf->b_locked > 0)) {
return true;
}
}
return false;
}
static void ex_quit(exarg_T *eap)
{
if (cmdwin_type != 0) {
cmdwin_result = Ctrl_C;
return;
}
if (text_locked()) {
text_locked_msg();
return;
}
win_T *wp;
if (eap->addr_count > 0) {
int wnr = eap->line2;
for (wp = firstwin; wp->w_next != NULL; wp = wp->w_next) {
if (--wnr <= 0)
break;
}
} else {
wp = curwin;
}
if (curbuf_locked()) {
return;
}
if (before_quit_autocmds(wp, false, eap->forceit)) {
return;
}
if (check_more(false, eap->forceit) == OK && only_one_window()) {
exiting = true;
}
if ((!buf_hide(wp->w_buffer)
&& check_changed(wp->w_buffer, (p_awa ? CCGD_AW : 0)
| (eap->forceit ? CCGD_FORCEIT : 0)
| CCGD_EXCMD))
|| check_more(true, eap->forceit) == FAIL
|| (only_one_window() && check_changed_any(eap->forceit, true))) {
not_exiting();
} else {
if (only_one_window() && (ONE_WINDOW || eap->addr_count == 0)) {
getout(0);
}
not_exiting();
win_close(wp, !buf_hide(wp->w_buffer) || eap->forceit);
}
}
static void ex_cquit(exarg_T *eap)
{
getout(eap->addr_count > 0 ? (int)eap->line2 : EXIT_FAILURE);
}
static void ex_quit_all(exarg_T *eap)
{
if (cmdwin_type != 0) {
if (eap->forceit) {
cmdwin_result = K_XF1; 
} else {
cmdwin_result = K_XF2;
}
return;
}
if (text_locked()) {
text_locked_msg();
return;
}
if (before_quit_autocmds(curwin, true, eap->forceit)) {
return;
}
exiting = true;
if (eap->forceit || !check_changed_any(false, false)) {
getout(0);
}
not_exiting();
}
static void ex_close(exarg_T *eap)
{
win_T *win = NULL;
int winnr = 0;
if (cmdwin_type != 0) {
cmdwin_result = Ctrl_C;
} else if (!text_locked() && !curbuf_locked()) {
if (eap->addr_count == 0) {
ex_win_close(eap->forceit, curwin, NULL);
} else {
FOR_ALL_WINDOWS_IN_TAB(wp, curtab) {
winnr++;
if (winnr == eap->line2) {
win = wp;
break;
}
}
if (win == NULL)
win = lastwin;
ex_win_close(eap->forceit, win, NULL);
}
}
}
static void ex_pclose(exarg_T *eap)
{
FOR_ALL_WINDOWS_IN_TAB(win, curtab) {
if (win->w_p_pvw) {
ex_win_close(eap->forceit, win, NULL);
break;
}
}
}
void
ex_win_close(
int forceit,
win_T *win,
tabpage_T *tp 
)
{
int need_hide;
buf_T *buf = win->w_buffer;
need_hide = (bufIsChanged(buf) && buf->b_nwindows <= 1);
if (need_hide && !buf_hide(buf) && !forceit) {
if ((p_confirm || cmdmod.confirm) && p_write) {
bufref_T bufref;
set_bufref(&bufref, buf);
dialog_changed(buf, false);
if (bufref_valid(&bufref) && bufIsChanged(buf)) {
return;
}
need_hide = false;
} else {
no_write_message();
return;
}
}
if (tp == NULL) {
win_close(win, !need_hide && !buf_hide(buf));
} else {
win_close_othertab(win, !need_hide && !buf_hide(buf), tp);
}
}
static void ex_tabclose(exarg_T *eap)
{
tabpage_T *tp;
if (cmdwin_type != 0)
cmdwin_result = K_IGNORE;
else if (first_tabpage->tp_next == NULL)
EMSG(_("E784: Cannot close last tab page"));
else {
int tab_number = get_tabpage_arg(eap);
if (eap->errmsg == NULL) {
tp = find_tabpage(tab_number);
if (tp == NULL) {
beep_flush();
return;
}
if (tp != curtab) {
tabpage_close_other(tp, eap->forceit);
return;
} else if (!text_locked() && !curbuf_locked()) {
tabpage_close(eap->forceit);
}
}
}
}
static void ex_tabonly(exarg_T *eap)
{
if (cmdwin_type != 0) {
cmdwin_result = K_IGNORE;
} else if (first_tabpage->tp_next == NULL) {
MSG(_("Already only one tab page"));
} else {
int tab_number = get_tabpage_arg(eap);
if (eap->errmsg == NULL) {
goto_tabpage(tab_number);
for (int done = 0; done < 1000; done++) {
FOR_ALL_TAB_WINDOWS(tp, wp) {
assert(wp != aucmd_win);
}
FOR_ALL_TABS(tp) {
if (tp->tp_topframe != topframe) {
tabpage_close_other(tp, eap->forceit);
if (valid_tabpage(tp)) {
done = 1000;
}
break;
}
}
assert(first_tabpage);
if (first_tabpage->tp_next == NULL) {
break;
}
}
}
}
}
void tabpage_close(int forceit)
{
while (curwin->w_floating) {
ex_win_close(forceit, curwin, NULL);
}
if (!ONE_WINDOW) {
close_others(true, forceit);
}
if (ONE_WINDOW) {
ex_win_close(forceit, curwin, NULL);
}
}
void tabpage_close_other(tabpage_T *tp, int forceit)
{
int done = 0;
win_T *wp;
int h = tabline_height();
char_u prev_idx[NUMBUFLEN];
while (++done < 1000) {
snprintf((char *)prev_idx, sizeof(prev_idx), "%i", tabpage_index(tp));
wp = tp->tp_lastwin;
ex_win_close(forceit, wp, tp);
if (!valid_tabpage(tp) || tp->tp_firstwin == wp)
break;
}
redraw_tabline = TRUE;
if (h != tabline_height())
shell_new_rows();
}
static void ex_only(exarg_T *eap)
{
win_T *wp;
int wnr;
if (eap->addr_count > 0) {
wnr = eap->line2;
for (wp = firstwin; --wnr > 0;) {
if (wp->w_next == NULL)
break;
else
wp = wp->w_next;
}
} else {
wp = curwin;
}
if (wp != curwin) {
win_goto(wp);
}
close_others(TRUE, eap->forceit);
}
void ex_all(exarg_T *eap)
{
if (eap->addr_count == 0)
eap->line2 = 9999;
do_arg_all((int)eap->line2, eap->forceit, eap->cmdidx == CMD_drop);
}
static void ex_hide(exarg_T *eap)
{
if (!eap->skip) {
if (eap->addr_count == 0) {
win_close(curwin, false); 
} else {
int winnr = 0;
win_T *win = NULL;
FOR_ALL_WINDOWS_IN_TAB(wp, curtab) {
winnr++;
if (winnr == eap->line2) {
win = wp;
break;
}
}
if (win == NULL) {
win = lastwin;
}
win_close(win, false);
}
}
}
static void ex_stop(exarg_T *eap)
{
if (!check_restricted()) {
if (!eap->forceit) {
autowrite_all();
}
apply_autocmds(EVENT_VIMSUSPEND, NULL, NULL, false, NULL);
ui_cursor_goto(Rows - 1, 0);
ui_call_grid_scroll(1, 0, Rows, 0, Columns, 1, 0);
ui_flush();
ui_call_suspend(); 
ui_flush();
maketitle();
resettitle(); 
ui_refresh(); 
apply_autocmds(EVENT_VIMRESUME, NULL, NULL, false, NULL);
}
}
static void ex_exit(exarg_T *eap)
{
if (cmdwin_type != 0) {
cmdwin_result = Ctrl_C;
return;
}
if (text_locked()) {
text_locked_msg();
return;
}
if (before_quit_autocmds(curwin, false, eap->forceit)) {
return;
}
if (check_more(false, eap->forceit) == OK && only_one_window()) {
exiting = true;
}
if (((eap->cmdidx == CMD_wq
|| curbufIsChanged())
&& do_write(eap) == FAIL)
|| check_more(true, eap->forceit) == FAIL
|| (only_one_window() && check_changed_any(eap->forceit, false))) {
not_exiting();
} else {
if (only_one_window()) {
getout(0);
}
not_exiting();
win_close(curwin, !buf_hide(curwin->w_buffer));
}
}
static void ex_print(exarg_T *eap)
{
if (curbuf->b_ml.ml_flags & ML_EMPTY)
EMSG(_(e_emptybuf));
else {
for (; !got_int; os_breakcheck()) {
print_line(eap->line1,
(eap->cmdidx == CMD_number || eap->cmdidx == CMD_pound
|| (eap->flags & EXFLAG_NR)),
eap->cmdidx == CMD_list || (eap->flags & EXFLAG_LIST));
if (++eap->line1 > eap->line2)
break;
ui_flush(); 
}
setpcmark();
curwin->w_cursor.lnum = eap->line2;
beginline(BL_SOL | BL_FIX);
}
ex_no_reprint = TRUE;
}
static void ex_goto(exarg_T *eap)
{
goto_byte(eap->line2);
}
void alist_clear(alist_T *al)
{
#define FREE_AENTRY_FNAME(arg) xfree(arg->ae_fname)
GA_DEEP_CLEAR(&al->al_ga, aentry_T, FREE_AENTRY_FNAME);
}
void alist_init(alist_T *al)
{
ga_init(&al->al_ga, (int)sizeof(aentry_T), 5);
}
void alist_unlink(alist_T *al)
{
if (al != &global_alist && --al->al_refcount <= 0) {
alist_clear(al);
xfree(al);
}
}
void alist_new(void)
{
curwin->w_alist = xmalloc(sizeof(*curwin->w_alist));
curwin->w_alist->al_refcount = 1;
curwin->w_alist->id = ++max_alist_id;
alist_init(curwin->w_alist);
}
#if !defined(UNIX)
void alist_expand(int *fnum_list, int fnum_len)
{
char_u **old_arg_files;
int old_arg_count;
char_u **new_arg_files;
int new_arg_file_count;
char_u *save_p_su = p_su;
int i;
p_su = empty_option;
old_arg_files = xmalloc(sizeof(*old_arg_files) * GARGCOUNT);
for (i = 0; i < GARGCOUNT; ++i)
old_arg_files[i] = vim_strsave(GARGLIST[i].ae_fname);
old_arg_count = GARGCOUNT;
if (expand_wildcards(old_arg_count, old_arg_files,
&new_arg_file_count, &new_arg_files,
EW_FILE|EW_NOTFOUND|EW_ADDSLASH|EW_NOERROR) == OK
&& new_arg_file_count > 0) {
alist_set(&global_alist, new_arg_file_count, new_arg_files,
TRUE, fnum_list, fnum_len);
FreeWild(old_arg_count, old_arg_files);
}
p_su = save_p_su;
}
#endif
void alist_set(alist_T *al, int count, char_u **files, int use_curbuf, int *fnum_list, int fnum_len)
{
int i;
static int recursive = 0;
if (recursive) {
EMSG(_(e_au_recursive));
return;
}
recursive++;
alist_clear(al);
ga_grow(&al->al_ga, count);
{
for (i = 0; i < count; ++i) {
if (got_int) {
while (i < count)
xfree(files[i++]);
break;
}
if (fnum_list != NULL && i < fnum_len)
buf_set_name(fnum_list[i], files[i]);
alist_add(al, files[i], use_curbuf ? 2 : 1);
os_breakcheck();
}
xfree(files);
}
if (al == &global_alist) {
arg_had_last = false;
}
recursive--;
}
void
alist_add(
alist_T *al,
char_u *fname,
int set_fnum 
)
{
if (fname == NULL) 
return;
#if defined(BACKSLASH_IN_FILENAME)
slash_adjust(fname);
#endif
AARGLIST(al)[al->al_ga.ga_len].ae_fname = fname;
if (set_fnum > 0)
AARGLIST(al)[al->al_ga.ga_len].ae_fnum =
buflist_add(fname, BLN_LISTED | (set_fnum == 2 ? BLN_CURBUF : 0));
++al->al_ga.ga_len;
}
#if defined(BACKSLASH_IN_FILENAME)
void alist_slash_adjust(void)
{
for (int i = 0; i < GARGCOUNT; ++i) {
if (GARGLIST[i].ae_fname != NULL) {
slash_adjust(GARGLIST[i].ae_fname);
}
}
FOR_ALL_TAB_WINDOWS(tp, wp) {
if (wp->w_alist != &global_alist) {
for (int i = 0; i < WARGCOUNT(wp); ++i) {
if (WARGLIST(wp)[i].ae_fname != NULL) {
slash_adjust(WARGLIST(wp)[i].ae_fname);
}
}
}
}
}
#endif
static void ex_preserve(exarg_T *eap)
{
curbuf->b_flags |= BF_PRESERVED;
ml_preserve(curbuf, true, true);
}
static void ex_recover(exarg_T *eap)
{
recoverymode = true;
if (!check_changed(curbuf, (p_awa ? CCGD_AW : 0)
| CCGD_MULTWIN
| (eap->forceit ? CCGD_FORCEIT : 0)
| CCGD_EXCMD)
&& (*eap->arg == NUL
|| setfname(curbuf, eap->arg, NULL, true) == OK)) {
ml_recover(true);
}
recoverymode = false;
}
static void ex_wrongmodifier(exarg_T *eap)
{
eap->errmsg = e_invcmd;
}
void ex_splitview(exarg_T *eap)
{
win_T *old_curwin = curwin;
char_u *fname = NULL;
if (bt_quickfix(curbuf) && cmdmod.tab == 0) {
if (eap->cmdidx == CMD_split)
eap->cmdidx = CMD_new;
if (eap->cmdidx == CMD_vsplit)
eap->cmdidx = CMD_vnew;
}
if (eap->cmdidx == CMD_sfind || eap->cmdidx == CMD_tabfind) {
fname = find_file_in_path(eap->arg, STRLEN(eap->arg),
FNAME_MESS, TRUE, curbuf->b_ffname);
if (fname == NULL)
goto theend;
eap->arg = fname;
}
if (eap->cmdidx == CMD_tabedit
|| eap->cmdidx == CMD_tabfind
|| eap->cmdidx == CMD_tabnew) {
if (win_new_tabpage(cmdmod.tab != 0 ? cmdmod.tab : eap->addr_count == 0
? 0 : (int)eap->line2 + 1, eap->arg) != FAIL) {
do_exedit(eap, old_curwin);
apply_autocmds(EVENT_TABNEWENTERED, NULL, NULL, FALSE, curbuf);
if (curwin != old_curwin
&& win_valid(old_curwin)
&& old_curwin->w_buffer != curbuf
&& !cmdmod.keepalt)
old_curwin->w_alt_fnum = curbuf->b_fnum;
}
} else if (win_split(eap->addr_count > 0 ? (int)eap->line2 : 0,
*eap->cmd == 'v' ? WSP_VERT : 0) != FAIL) {
if (*eap->arg != NUL
) {
RESET_BINDING(curwin);
} else {
do_check_scrollbind(false);
}
do_exedit(eap, old_curwin);
}
theend:
xfree(fname);
}
void tabpage_new(void)
{
exarg_T ea;
memset(&ea, 0, sizeof(ea));
ea.cmdidx = CMD_tabnew;
ea.cmd = (char_u *)"tabn";
ea.arg = (char_u *)"";
ex_splitview(&ea);
}
static void ex_tabnext(exarg_T *eap)
{
int tab_number;
switch (eap->cmdidx) {
case CMD_tabfirst:
case CMD_tabrewind:
goto_tabpage(1);
break;
case CMD_tablast:
goto_tabpage(9999);
break;
case CMD_tabprevious:
case CMD_tabNext:
if (eap->arg && *eap->arg != NUL) {
char_u *p = eap->arg;
char_u *p_save = p;
tab_number = getdigits(&p, false, 0);
if (p == p_save || *p_save == '-' || *p_save == '+' || *p != NUL
|| tab_number == 0) {
eap->errmsg = e_invarg;
return;
}
} else {
if (eap->addr_count == 0) {
tab_number = 1;
} else {
tab_number = eap->line2;
if (tab_number < 1) {
eap->errmsg = e_invrange;
return;
}
}
}
goto_tabpage(-tab_number);
break;
default: 
tab_number = get_tabpage_arg(eap);
if (eap->errmsg == NULL) {
goto_tabpage(tab_number);
}
break;
}
}
static void ex_tabmove(exarg_T *eap)
{
int tab_number = get_tabpage_arg(eap);
if (eap->errmsg == NULL) {
tabpage_move(tab_number);
}
}
static void ex_tabs(exarg_T *eap)
{
int tabcount = 1;
msg_start();
msg_scroll = TRUE;
win_T *lastused_win = valid_tabpage(lastused_tabpage)
? lastused_tabpage->tp_curwin
: NULL;
FOR_ALL_TABS(tp) {
if (got_int) {
break;
}
msg_putchar('\n');
vim_snprintf((char *)IObuff, IOSIZE, _("Tab page %d"), tabcount++);
msg_outtrans_attr(IObuff, HL_ATTR(HLF_T));
ui_flush(); 
os_breakcheck();
FOR_ALL_WINDOWS_IN_TAB(wp, tp) {
if (got_int) {
break;
}
msg_putchar('\n');
msg_putchar(wp == curwin ? '>' : wp == lastused_win ? '#' : ' ');
msg_putchar(' ');
msg_putchar(bufIsChanged(wp->w_buffer) ? '+' : ' ');
msg_putchar(' ');
if (buf_spname(wp->w_buffer) != NULL)
STRLCPY(IObuff, buf_spname(wp->w_buffer), IOSIZE);
else
home_replace(wp->w_buffer, wp->w_buffer->b_fname,
IObuff, IOSIZE, TRUE);
msg_outtrans(IObuff);
ui_flush(); 
os_breakcheck();
}
}
}
static void ex_mode(exarg_T *eap)
{
if (*eap->arg == NUL) {
must_redraw = CLEAR;
ex_redraw(eap);
} else {
EMSG(_(e_screenmode));
}
}
static void ex_resize(exarg_T *eap)
{
int n;
win_T *wp = curwin;
if (eap->addr_count > 0) {
n = eap->line2;
for (wp = firstwin; wp->w_next != NULL && --n > 0; wp = wp->w_next)
;
}
n = atol((char *)eap->arg);
if (cmdmod.split & WSP_VERT) {
if (*eap->arg == '-' || *eap->arg == '+') {
n += curwin->w_width;
} else if (n == 0 && eap->arg[0] == NUL) { 
n = Columns;
}
win_setwidth_win(n, wp);
} else {
if (*eap->arg == '-' || *eap->arg == '+') {
n += curwin->w_height;
} else if (n == 0 && eap->arg[0] == NUL) { 
n = Rows-1;
}
win_setheight_win(n, wp);
}
}
static void ex_find(exarg_T *eap)
{
char_u *fname;
int count;
fname = find_file_in_path(eap->arg, STRLEN(eap->arg),
FNAME_MESS, TRUE, curbuf->b_ffname);
if (eap->addr_count > 0) {
count = eap->line2;
while (fname != NULL && --count > 0) {
xfree(fname);
fname = find_file_in_path(NULL, 0, FNAME_MESS, FALSE, curbuf->b_ffname);
}
}
if (fname != NULL) {
eap->arg = fname;
do_exedit(eap, NULL);
xfree(fname);
}
}
static void ex_edit(exarg_T *eap)
{
do_exedit(eap, NULL);
}
void
do_exedit(
exarg_T *eap,
win_T *old_curwin 
)
{
int n;
int need_hide;
if (exmode_active && (eap->cmdidx == CMD_visual
|| eap->cmdidx == CMD_view)) {
exmode_active = FALSE;
if (*eap->arg == NUL) {
if (global_busy) {
int rd = RedrawingDisabled;
int nwr = no_wait_return;
int ms = msg_scroll;
if (eap->nextcmd != NULL) {
stuffReadbuff((const char *)eap->nextcmd);
eap->nextcmd = NULL;
}
RedrawingDisabled = 0;
no_wait_return = 0;
need_wait_return = FALSE;
msg_scroll = 0;
redraw_all_later(NOT_VALID);
normal_enter(false, true);
RedrawingDisabled = rd;
no_wait_return = nwr;
msg_scroll = ms;
}
return;
}
}
if ((eap->cmdidx == CMD_new
|| eap->cmdidx == CMD_tabnew
|| eap->cmdidx == CMD_tabedit
|| eap->cmdidx == CMD_vnew
) && *eap->arg == NUL) {
setpcmark();
(void)do_ecmd(0, NULL, NULL, eap, ECMD_ONE,
ECMD_HIDE + (eap->forceit ? ECMD_FORCEIT : 0),
old_curwin == NULL ? curwin : NULL);
} else if ((eap->cmdidx != CMD_split && eap->cmdidx != CMD_vsplit)
|| *eap->arg != NUL) {
if (*eap->arg != NUL && curbuf_locked())
return;
n = readonlymode;
if (eap->cmdidx == CMD_view || eap->cmdidx == CMD_sview)
readonlymode = TRUE;
else if (eap->cmdidx == CMD_enew)
readonlymode = FALSE; 
setpcmark();
if (do_ecmd(0, (eap->cmdidx == CMD_enew ? NULL : eap->arg),
NULL, eap, eap->do_ecmd_lnum,
(buf_hide(curbuf) ? ECMD_HIDE : 0)
+ (eap->forceit ? ECMD_FORCEIT : 0)
+ (old_curwin != NULL ? ECMD_OLDBUF : 0)
+ (eap->cmdidx == CMD_badd ? ECMD_ADDBUF : 0)
, old_curwin == NULL ? curwin : NULL) == FAIL) {
if (old_curwin != NULL) {
need_hide = (curbufIsChanged() && curbuf->b_nwindows <= 1);
if (!need_hide || buf_hide(curbuf)) {
cleanup_T cs;
enter_cleanup(&cs);
win_close(curwin, !need_hide && !buf_hide(curbuf));
leave_cleanup(&cs);
}
}
} else if (readonlymode && curbuf->b_nwindows == 1) {
curbuf->b_p_ro = TRUE;
}
readonlymode = n;
} else {
if (eap->do_ecmd_cmd != NULL)
do_cmdline_cmd((char *)eap->do_ecmd_cmd);
n = curwin->w_arg_idx_invalid;
check_arg_idx(curwin);
if (n != curwin->w_arg_idx_invalid)
maketitle();
}
if (old_curwin != NULL
&& *eap->arg != NUL
&& curwin != old_curwin
&& win_valid(old_curwin)
&& old_curwin->w_buffer != curbuf
&& !cmdmod.keepalt)
old_curwin->w_alt_fnum = curbuf->b_fnum;
ex_no_reprint = TRUE;
}
static void ex_nogui(exarg_T *eap)
{
eap->errmsg = (char_u *)N_("E25: Nvim does not have a built-in GUI");
}
static void ex_swapname(exarg_T *eap)
{
if (curbuf->b_ml.ml_mfp == NULL || curbuf->b_ml.ml_mfp->mf_fname == NULL)
MSG(_("No swap file"));
else
msg(curbuf->b_ml.ml_mfp->mf_fname);
}
static void ex_syncbind(exarg_T *eap)
{
win_T *save_curwin = curwin;
buf_T *save_curbuf = curbuf;
long topline;
long y;
linenr_T old_linenr = curwin->w_cursor.lnum;
setpcmark();
if (curwin->w_p_scb) {
topline = curwin->w_topline;
FOR_ALL_WINDOWS_IN_TAB(wp, curtab) {
if (wp->w_p_scb && wp->w_buffer) {
y = wp->w_buffer->b_ml.ml_line_count - get_scrolloff_value();
if (topline > y) {
topline = y;
}
}
}
if (topline < 1) {
topline = 1;
}
} else {
topline = 1;
}
FOR_ALL_WINDOWS_IN_TAB(wp, curtab) {
curwin = wp;
if (curwin->w_p_scb) {
curbuf = curwin->w_buffer;
y = topline - curwin->w_topline;
if (y > 0)
scrollup(y, TRUE);
else
scrolldown(-y, TRUE);
curwin->w_scbind_pos = topline;
redraw_later(VALID);
cursor_correct();
curwin->w_redr_status = TRUE;
}
}
curwin = save_curwin;
curbuf = save_curbuf;
if (curwin->w_p_scb) {
did_syncbind = TRUE;
checkpcmark();
if (old_linenr != curwin->w_cursor.lnum) {
char_u ctrl_o[2];
ctrl_o[0] = Ctrl_O;
ctrl_o[1] = 0;
ins_typebuf(ctrl_o, REMAP_NONE, 0, TRUE, FALSE);
}
}
}
static void ex_read(exarg_T *eap)
{
int i;
int empty = (curbuf->b_ml.ml_flags & ML_EMPTY);
linenr_T lnum;
if (eap->usefilter) 
do_bang(1, eap, FALSE, FALSE, TRUE);
else {
if (u_save(eap->line2, (linenr_T)(eap->line2 + 1)) == FAIL)
return;
if (*eap->arg == NUL) {
if (check_fname() == FAIL) 
return;
i = readfile(curbuf->b_ffname, curbuf->b_fname,
eap->line2, (linenr_T)0, (linenr_T)MAXLNUM, eap, 0);
} else {
if (vim_strchr(p_cpo, CPO_ALTREAD) != NULL)
(void)setaltfname(eap->arg, eap->arg, (linenr_T)1);
i = readfile(eap->arg, NULL,
eap->line2, (linenr_T)0, (linenr_T)MAXLNUM, eap, 0);
}
if (i != OK) {
if (!aborting()) {
EMSG2(_(e_notopen), eap->arg);
}
} else {
if (empty && exmode_active) {
if (eap->line2 == 0)
lnum = curbuf->b_ml.ml_line_count;
else
lnum = 1;
if (*ml_get(lnum) == NUL && u_savedel(lnum, 1L) == OK) {
ml_delete(lnum, false);
if (curwin->w_cursor.lnum > 1
&& curwin->w_cursor.lnum >= lnum) {
curwin->w_cursor.lnum--;
}
deleted_lines_mark(lnum, 1L);
}
}
redraw_curbuf_later(VALID);
}
}
}
static char_u *prev_dir = NULL;
#if defined(EXITFREE)
void free_cd_dir(void)
{
XFREE_CLEAR(prev_dir);
XFREE_CLEAR(globaldir);
}
#endif
void post_chdir(CdScope scope, bool trigger_dirchanged)
{
XFREE_CLEAR(curwin->w_localdir);
if (scope >= kCdScopeTab) {
XFREE_CLEAR(curtab->tp_localdir);
}
if (scope < kCdScopeGlobal) {
if (globaldir == NULL && prev_dir != NULL) {
globaldir = vim_strsave(prev_dir);
}
}
char cwd[MAXPATHL];
if (os_dirname((char_u *)cwd, MAXPATHL) != OK) {
return;
}
switch (scope) {
case kCdScopeGlobal:
XFREE_CLEAR(globaldir);
break;
case kCdScopeTab:
curtab->tp_localdir = (char_u *)xstrdup(cwd);
break;
case kCdScopeWindow:
curwin->w_localdir = (char_u *)xstrdup(cwd);
break;
case kCdScopeInvalid:
assert(false);
}
shorten_fnames(true);
if (trigger_dirchanged) {
do_autocmd_dirchanged(cwd, scope);
}
}
void ex_cd(exarg_T *eap)
{
char_u *new_dir;
char_u *tofree;
new_dir = eap->arg;
#if !defined(UNIX)
if (*new_dir == NUL)
ex_pwd(NULL);
else
#endif
{
if (allbuf_locked())
return;
if (STRCMP(new_dir, "-") == 0) {
if (prev_dir == NULL) {
EMSG(_("E186: No previous directory"));
return;
}
new_dir = prev_dir;
}
tofree = prev_dir;
if (os_dirname(NameBuff, MAXPATHL) == OK)
prev_dir = vim_strsave(NameBuff);
else
prev_dir = NULL;
#if defined(UNIX)
if (*new_dir == NUL) {
expand_env((char_u *)"$HOME", NameBuff, MAXPATHL);
new_dir = NameBuff;
}
#endif
CdScope scope = kCdScopeGlobal; 
switch (eap->cmdidx) {
case CMD_tcd:
case CMD_tchdir:
scope = kCdScopeTab;
break;
case CMD_lcd:
case CMD_lchdir:
scope = kCdScopeWindow;
break;
default:
break;
}
if (vim_chdir(new_dir)) {
EMSG(_(e_failed));
} else {
post_chdir(scope, true);
if (KeyTyped || p_verbose >= 5) {
ex_pwd(eap);
}
}
xfree(tofree);
}
}
static void ex_pwd(exarg_T *eap)
{
if (os_dirname(NameBuff, MAXPATHL) == OK) {
#if defined(BACKSLASH_IN_FILENAME)
slash_adjust(NameBuff);
#endif
msg(NameBuff);
} else
EMSG(_("E187: Unknown"));
}
static void ex_equal(exarg_T *eap)
{
smsg("%" PRId64, (int64_t)eap->line2);
ex_may_print(eap);
}
static void ex_sleep(exarg_T *eap)
{
int n;
long len;
if (cursor_valid()) {
n = curwin->w_winrow + curwin->w_wrow - msg_scrolled;
if (n >= 0)
ui_cursor_goto(n, curwin->w_wincol + curwin->w_wcol);
}
len = eap->line2;
switch (*eap->arg) {
case 'm': break;
case NUL: len *= 1000L; break;
default: EMSG2(_(e_invarg2), eap->arg); return;
}
do_sleep(len);
}
void do_sleep(long msec)
{
ui_flush(); 
for (long left = msec; !got_int && left > 0; left -= 1000L) {
int next = left > 1000l ? 1000 : (int)left;
LOOP_PROCESS_EVENTS_UNTIL(&main_loop, main_loop.events, (int)next, got_int);
os_breakcheck();
}
if (got_int) {
(void)vpeekc();
}
}
static void do_exmap(exarg_T *eap, int isabbrev)
{
int mode;
char_u *cmdp;
cmdp = eap->cmd;
mode = get_map_mode(&cmdp, eap->forceit || isabbrev);
switch (do_map((*cmdp == 'n') ? 2 : (*cmdp == 'u'),
eap->arg, mode, isabbrev)) {
case 1: EMSG(_(e_invarg));
break;
case 2: EMSG(isabbrev ? _(e_noabbr) : _(e_nomap));
break;
}
}
static void ex_winsize(exarg_T *eap)
{
char_u *arg = eap->arg;
int w = getdigits_int(&arg, false, 10);
arg = skipwhite(arg);
char_u *p = arg;
int h = getdigits_int(&arg, false, 10);
if (*p != NUL && *arg == NUL) {
screen_resize(w, h);
} else {
EMSG(_("E465: :winsize requires two number arguments"));
}
}
static void ex_wincmd(exarg_T *eap)
{
int xchar = NUL;
char_u *p;
if (*eap->arg == 'g' || *eap->arg == Ctrl_G) {
if (eap->arg[1] == NUL) {
EMSG(_(e_invarg));
return;
}
xchar = eap->arg[1];
p = eap->arg + 2;
} else
p = eap->arg + 1;
eap->nextcmd = check_nextcmd(p);
p = skipwhite(p);
if (*p != NUL && *p != '"' && eap->nextcmd == NULL)
EMSG(_(e_invarg));
else if (!eap->skip) {
postponed_split_flags = cmdmod.split;
postponed_split_tab = cmdmod.tab;
do_window(*eap->arg, eap->addr_count > 0 ? eap->line2 : 0L, xchar);
postponed_split_flags = 0;
postponed_split_tab = 0;
}
}
static void ex_operators(exarg_T *eap)
{
oparg_T oa;
clear_oparg(&oa);
oa.regname = eap->regname;
oa.start.lnum = eap->line1;
oa.end.lnum = eap->line2;
oa.line_count = eap->line2 - eap->line1 + 1;
oa.motion_type = kMTLineWise;
virtual_op = kFalse;
if (eap->cmdidx != CMD_yank) { 
setpcmark();
curwin->w_cursor.lnum = eap->line1;
beginline(BL_SOL | BL_FIX);
}
if (VIsual_active)
end_visual_mode();
switch (eap->cmdidx) {
case CMD_delete:
oa.op_type = OP_DELETE;
op_delete(&oa);
break;
case CMD_yank:
oa.op_type = OP_YANK;
(void)op_yank(&oa, true, false);
break;
default: 
if (
(eap->cmdidx == CMD_rshift) ^ curwin->w_p_rl
)
oa.op_type = OP_RSHIFT;
else
oa.op_type = OP_LSHIFT;
op_shift(&oa, FALSE, eap->amount);
break;
}
virtual_op = kNone;
ex_may_print(eap);
}
static void ex_put(exarg_T *eap)
{
if (eap->line2 == 0) {
eap->line2 = 1;
eap->forceit = TRUE;
}
curwin->w_cursor.lnum = eap->line2;
do_put(eap->regname, NULL, eap->forceit ? BACKWARD : FORWARD, 1,
PUT_LINE|PUT_CURSLINE);
}
static void ex_copymove(exarg_T *eap)
{
long n = get_address(eap, &eap->arg, eap->addr_type, false, false, 1);
if (eap->arg == NULL) { 
eap->nextcmd = NULL;
return;
}
get_flags(eap);
if (n == MAXLNUM || n < 0 || n > curbuf->b_ml.ml_line_count) {
EMSG(_(e_invaddr));
return;
}
if (eap->cmdidx == CMD_move) {
if (do_move(eap->line1, eap->line2, n) == FAIL)
return;
} else
ex_copy(eap->line1, eap->line2, n);
u_clearline();
beginline(BL_SOL | BL_FIX);
ex_may_print(eap);
}
void ex_may_print(exarg_T *eap)
{
if (eap->flags != 0) {
print_line(curwin->w_cursor.lnum, (eap->flags & EXFLAG_NR),
(eap->flags & EXFLAG_LIST));
ex_no_reprint = TRUE;
}
}
static void ex_submagic(exarg_T *eap)
{
int magic_save = p_magic;
p_magic = (eap->cmdidx == CMD_smagic);
ex_substitute(eap);
p_magic = magic_save;
}
static void ex_join(exarg_T *eap)
{
curwin->w_cursor.lnum = eap->line1;
if (eap->line1 == eap->line2) {
if (eap->addr_count >= 2) 
return;
if (eap->line2 == curbuf->b_ml.ml_line_count) {
beep_flush();
return;
}
++eap->line2;
}
do_join(eap->line2 - eap->line1 + 1, !eap->forceit, TRUE, TRUE, true);
beginline(BL_WHITE | BL_FIX);
ex_may_print(eap);
}
static void ex_at(exarg_T *eap)
{
int prev_len = typebuf.tb_len;
curwin->w_cursor.lnum = eap->line2;
check_cursor_col();
int c = *eap->arg;
if (c == NUL) {
c = '@';
}
if (do_execreg(c, TRUE, vim_strchr(p_cpo, CPO_EXECBUF) != NULL, TRUE)
== FAIL) {
beep_flush();
} else {
int save_efr = exec_from_reg;
exec_from_reg = TRUE;
while (!stuff_empty() || typebuf.tb_len > prev_len)
(void)do_cmdline(NULL, getexline, NULL, DOCMD_NOWAIT|DOCMD_VERBOSE);
exec_from_reg = save_efr;
}
}
static void ex_bang(exarg_T *eap)
{
do_bang(eap->addr_count, eap, eap->forceit, TRUE, TRUE);
}
static void ex_undo(exarg_T *eap)
{
if (eap->addr_count == 1) { 
undo_time(eap->line2, false, false, true);
} else {
u_undo(1);
}
}
static void ex_wundo(exarg_T *eap)
{
char_u hash[UNDO_HASH_SIZE];
u_compute_hash(hash);
u_write_undo((char *) eap->arg, eap->forceit, curbuf, hash);
}
static void ex_rundo(exarg_T *eap)
{
char_u hash[UNDO_HASH_SIZE];
u_compute_hash(hash);
u_read_undo((char *) eap->arg, hash, NULL);
}
static void ex_redo(exarg_T *eap)
{
u_redo(1);
}
static void ex_later(exarg_T *eap)
{
long count = 0;
bool sec = false;
bool file = false;
char_u *p = eap->arg;
if (*p == NUL) {
count = 1;
} else if (isdigit(*p)) {
count = getdigits_long(&p, false, 0);
switch (*p) {
case 's': ++p; sec = true; break;
case 'm': ++p; sec = true; count *= 60; break;
case 'h': ++p; sec = true; count *= 60 * 60; break;
case 'd': ++p; sec = true; count *= 24 * 60 * 60; break;
case 'f': ++p; file = true; break;
}
}
if (*p != NUL) {
EMSG2(_(e_invarg2), eap->arg);
} else {
undo_time(eap->cmdidx == CMD_earlier ? -count : count,
sec, file, false);
}
}
static void ex_redir(exarg_T *eap)
{
char *mode;
char_u *fname;
char_u *arg = eap->arg;
if (STRICMP(eap->arg, "END") == 0)
close_redir();
else {
if (*arg == '>') {
++arg;
if (*arg == '>') {
++arg;
mode = "a";
} else
mode = "w";
arg = skipwhite(arg);
close_redir();
fname = expand_env_save(arg);
if (fname == NULL)
return;
redir_fd = open_exfile(fname, eap->forceit, mode);
xfree(fname);
} else if (*arg == '@') {
close_redir();
++arg;
if (valid_yank_reg(*arg, true) && *arg != '_') {
redir_reg = *arg++;
if (*arg == '>' && arg[1] == '>') 
arg += 2;
else {
if (*arg == '>')
arg++;
if (*arg == NUL && !isupper(redir_reg)) {
write_reg_contents(redir_reg, (char_u *)"", 0, false);
}
}
}
if (*arg != NUL) {
redir_reg = 0;
EMSG2(_(e_invarg2), eap->arg);
}
} else if (*arg == '=' && arg[1] == '>') {
int append;
close_redir();
arg += 2;
if (*arg == '>') {
++arg;
append = TRUE;
} else
append = FALSE;
if (var_redir_start(skipwhite(arg), append) == OK)
redir_vname = 1;
}
else
EMSG2(_(e_invarg2), eap->arg);
}
if (redir_fd != NULL
|| redir_reg || redir_vname
)
redir_off = FALSE;
}
static void ex_redraw(exarg_T *eap)
{
if (State & CMDPREVIEW) {
return; 
}
int r = RedrawingDisabled;
int p = p_lz;
RedrawingDisabled = 0;
p_lz = FALSE;
validate_cursor();
update_topline();
if (eap->forceit) {
redraw_all_later(NOT_VALID);
}
update_screen(eap->forceit ? NOT_VALID
: VIsual_active ? INVERTED : 0);
if (need_maketitle) {
maketitle();
}
RedrawingDisabled = r;
p_lz = p;
msg_didout = FALSE;
msg_col = 0;
need_wait_return = FALSE;
ui_flush();
}
static void ex_redrawstatus(exarg_T *eap)
{
if (State & CMDPREVIEW) {
return; 
}
int r = RedrawingDisabled;
int p = p_lz;
RedrawingDisabled = 0;
p_lz = FALSE;
if (eap->forceit)
status_redraw_all();
else
status_redraw_curbuf();
update_screen(
VIsual_active ? INVERTED :
0);
RedrawingDisabled = r;
p_lz = p;
ui_flush();
}
static void ex_redrawtabline(exarg_T *eap FUNC_ATTR_UNUSED)
{
const int r = RedrawingDisabled;
const int p = p_lz;
RedrawingDisabled = 0;
p_lz = false;
draw_tabline();
RedrawingDisabled = r;
p_lz = p;
ui_flush();
}
static void close_redir(void)
{
if (redir_fd != NULL) {
fclose(redir_fd);
redir_fd = NULL;
}
redir_reg = 0;
if (redir_vname) {
var_redir_stop();
redir_vname = 0;
}
}
int vim_mkdir_emsg(const char *const name, const int prot)
FUNC_ATTR_NONNULL_ALL
{
int ret;
if ((ret = os_mkdir(name, prot)) != 0) {
EMSG3(_(e_mkdir), name, os_strerror(ret));
return FAIL;
}
return OK;
}
FILE *
open_exfile (
char_u *fname,
int forceit,
char *mode 
)
{
FILE *fd;
#if defined(UNIX)
if (os_isdir(fname)) {
EMSG2(_(e_isadir2), fname);
return NULL;
}
#endif
if (!forceit && *mode != 'a' && os_path_exists(fname)) {
EMSG2(_("E189: \"%s\" exists (add ! to override)"), fname);
return NULL;
}
if ((fd = os_fopen((char *)fname, mode)) == NULL) {
EMSG2(_("E190: Cannot open \"%s\" for writing"), fname);
}
return fd;
}
static void ex_mark(exarg_T *eap)
{
pos_T pos;
if (*eap->arg == NUL) 
EMSG(_(e_argreq));
else if (eap->arg[1] != NUL) 
EMSG(_(e_trailing));
else {
pos = curwin->w_cursor; 
curwin->w_cursor.lnum = eap->line2;
beginline(BL_WHITE | BL_FIX);
if (setmark(*eap->arg) == FAIL) 
EMSG(_("E191: Argument must be a letter or forward/backward quote"));
curwin->w_cursor = pos; 
}
}
void update_topline_cursor(void)
{
check_cursor(); 
update_topline();
if (!curwin->w_p_wrap)
validate_cursor();
update_curswant();
}
static void ex_normal(exarg_T *eap)
{
if (curbuf->terminal && State & TERM_FOCUS) {
EMSG("Can't re-enter normal mode from terminal mode");
return;
}
int save_msg_scroll = msg_scroll;
int save_restart_edit = restart_edit;
int save_msg_didout = msg_didout;
int save_State = State;
tasave_T tabuf;
int save_insertmode = p_im;
int save_finish_op = finish_op;
long save_opcount = opcount;
const int save_reg_executing = reg_executing;
char_u *arg = NULL;
int l;
char_u *p;
if (ex_normal_lock > 0) {
EMSG(_(e_secure));
return;
}
if (ex_normal_busy >= p_mmd) {
EMSG(_("E192: Recursive use of :normal too deep"));
return;
}
++ex_normal_busy;
msg_scroll = FALSE; 
restart_edit = 0; 
p_im = FALSE; 
if (has_mbyte) {
int len = 0;
for (p = eap->arg; *p != NUL; ++p) {
for (l = (*mb_ptr2len)(p) - 1; l > 0; --l)
if (*++p == K_SPECIAL 
)
len += 2;
}
if (len > 0) {
arg = xmalloc(STRLEN(eap->arg) + len + 1);
len = 0;
for (p = eap->arg; *p != NUL; ++p) {
arg[len++] = *p;
for (l = (*mb_ptr2len)(p) - 1; l > 0; --l) {
arg[len++] = *++p;
if (*p == K_SPECIAL) {
arg[len++] = KS_SPECIAL;
arg[len++] = KE_FILLER;
}
}
arg[len] = NUL;
}
}
}
save_typeahead(&tabuf);
if (tabuf.typebuf_valid) {
do {
if (eap->addr_count != 0) {
curwin->w_cursor.lnum = eap->line1++;
curwin->w_cursor.col = 0;
check_cursor_moved(curwin);
}
exec_normal_cmd(
arg != NULL ? arg :
eap->arg, eap->forceit ? REMAP_NONE : REMAP_YES, FALSE);
} while (eap->addr_count > 0 && eap->line1 <= eap->line2 && !got_int);
}
update_topline_cursor();
restore_typeahead(&tabuf);
--ex_normal_busy;
msg_scroll = save_msg_scroll;
if (force_restart_edit) {
force_restart_edit = false;
} else {
restart_edit = save_restart_edit;
}
p_im = save_insertmode;
finish_op = save_finish_op;
opcount = save_opcount;
reg_executing = save_reg_executing;
msg_didout |= save_msg_didout; 
State = save_State;
setmouse();
ui_cursor_shape(); 
xfree(arg);
}
static void ex_startinsert(exarg_T *eap)
{
if (eap->forceit) {
if (!curwin->w_cursor.lnum) {
curwin->w_cursor.lnum = 1;
}
set_cursor_for_append_to_line();
}
if (State & INSERT) {
return;
}
if (eap->cmdidx == CMD_startinsert)
restart_edit = 'a';
else if (eap->cmdidx == CMD_startreplace)
restart_edit = 'R';
else
restart_edit = 'V';
if (!eap->forceit) {
if (eap->cmdidx == CMD_startinsert)
restart_edit = 'i';
curwin->w_curswant = 0; 
}
if (VIsual_active) {
showmode();
}
}
static void ex_stopinsert(exarg_T *eap)
{
restart_edit = 0;
stop_insert_mode = true;
clearmode();
}
void exec_normal_cmd(char_u *cmd, int remap, bool silent)
{
ins_typebuf(cmd, remap, 0, true, silent);
exec_normal(false);
}
void exec_normal(bool was_typed)
{
oparg_T oa;
clear_oparg(&oa);
finish_op = false;
while ((!stuff_empty()
|| ((was_typed || !typebuf_typed())
&& typebuf.tb_len > 0))
&& !got_int) {
update_topline_cursor();
normal_cmd(&oa, true); 
}
}
static void ex_checkpath(exarg_T *eap)
{
find_pattern_in_path(NULL, 0, 0, FALSE, FALSE, CHECK_PATH, 1L,
eap->forceit ? ACTION_SHOW_ALL : ACTION_SHOW,
(linenr_T)1, (linenr_T)MAXLNUM);
}
static void ex_psearch(exarg_T *eap)
{
g_do_tagpreview = p_pvh;
ex_findpat(eap);
g_do_tagpreview = 0;
}
static void ex_findpat(exarg_T *eap)
{
int whole = TRUE;
long n;
char_u *p;
int action;
switch (cmdnames[eap->cmdidx].cmd_name[2]) {
case 'e': 
if (cmdnames[eap->cmdidx].cmd_name[0] == 'p')
action = ACTION_GOTO;
else
action = ACTION_SHOW;
break;
case 'i': 
action = ACTION_SHOW_ALL;
break;
case 'u': 
action = ACTION_GOTO;
break;
default: 
action = ACTION_SPLIT;
break;
}
n = 1;
if (ascii_isdigit(*eap->arg)) { 
n = getdigits_long(&eap->arg, false, 0);
eap->arg = skipwhite(eap->arg);
}
if (*eap->arg == '/') { 
whole = false;
eap->arg++;
p = skip_regexp(eap->arg, '/', p_magic, NULL);
if (*p) {
*p++ = NUL;
p = skipwhite(p);
if (!ends_excmd(*p)) {
eap->errmsg = e_trailing;
} else {
eap->nextcmd = check_nextcmd(p);
}
}
}
if (!eap->skip)
find_pattern_in_path(eap->arg, 0, STRLEN(eap->arg), whole, !eap->forceit,
*eap->cmd == 'd' ? FIND_DEFINE : FIND_ANY,
n, action, eap->line1, eap->line2);
}
static void ex_ptag(exarg_T *eap)
{
g_do_tagpreview = p_pvh; 
ex_tag_cmd(eap, cmdnames[eap->cmdidx].cmd_name + 1);
}
static void ex_pedit(exarg_T *eap)
{
win_T *curwin_save = curwin;
g_do_tagpreview = p_pvh;
prepare_tagpreview(true);
do_exedit(eap, NULL);
if (curwin != curwin_save && win_valid(curwin_save)) {
validate_cursor();
redraw_later(VALID);
win_enter(curwin_save, true);
}
g_do_tagpreview = 0;
}
static void ex_stag(exarg_T *eap)
{
postponed_split = -1;
postponed_split_flags = cmdmod.split;
postponed_split_tab = cmdmod.tab;
ex_tag_cmd(eap, cmdnames[eap->cmdidx].cmd_name + 1);
postponed_split_flags = 0;
postponed_split_tab = 0;
}
static void ex_tag(exarg_T *eap)
{
ex_tag_cmd(eap, cmdnames[eap->cmdidx].cmd_name);
}
static void ex_tag_cmd(exarg_T *eap, char_u *name)
{
int cmd;
switch (name[1]) {
case 'j': cmd = DT_JUMP; 
break;
case 's': cmd = DT_SELECT; 
break;
case 'p': 
case 'N': cmd = DT_PREV; 
break;
case 'n': cmd = DT_NEXT; 
break;
case 'o': cmd = DT_POP; 
break;
case 'f': 
case 'r': cmd = DT_FIRST; 
break;
case 'l': cmd = DT_LAST; 
break;
default: 
if (p_cst && *eap->arg != NUL) {
ex_cstag(eap);
return;
}
cmd = DT_TAG;
break;
}
if (name[0] == 'l') {
cmd = DT_LTAG;
}
do_tag(eap->arg, cmd, eap->addr_count > 0 ? (int)eap->line2 : 1,
eap->forceit, TRUE);
}
ssize_t find_cmdline_var(const char_u *src, size_t *usedlen)
FUNC_ATTR_NONNULL_ALL
{
size_t len;
static char *(spec_str[]) = {
"%",
#define SPEC_PERC 0
"#",
#define SPEC_HASH (SPEC_PERC + 1)
"<cword>", 
#define SPEC_CWORD (SPEC_HASH + 1)
"<cWORD>", 
#define SPEC_CCWORD (SPEC_CWORD + 1)
"<cexpr>", 
#define SPEC_CEXPR (SPEC_CCWORD + 1)
"<cfile>", 
#define SPEC_CFILE (SPEC_CEXPR + 1)
"<sfile>", 
#define SPEC_SFILE (SPEC_CFILE + 1)
"<slnum>", 
#define SPEC_SLNUM (SPEC_SFILE + 1)
"<afile>", 
#define SPEC_AFILE (SPEC_SLNUM + 1)
"<abuf>", 
#define SPEC_ABUF (SPEC_AFILE + 1)
"<amatch>", 
#define SPEC_AMATCH (SPEC_ABUF + 1)
"<sflnum>", 
#define SPEC_SFLNUM (SPEC_AMATCH + 1)
};
for (size_t i = 0; i < ARRAY_SIZE(spec_str); ++i) {
len = STRLEN(spec_str[i]);
if (STRNCMP(src, spec_str[i], len) == 0) {
*usedlen = len;
assert(i <= SSIZE_MAX);
return (ssize_t)i;
}
}
return -1;
}
char_u *
eval_vars (
char_u *src, 
char_u *srcstart, 
size_t *usedlen, 
linenr_T *lnump, 
char_u **errormsg, 
int *escaped 
)
{
int i;
char_u *s;
char_u *result;
char_u *resultbuf = NULL;
size_t resultlen;
buf_T *buf;
int valid = VALID_HEAD | VALID_PATH; 
bool tilde_file = false;
int skip_mod = false;
char strbuf[30];
*errormsg = NULL;
if (escaped != NULL)
*escaped = FALSE;
ssize_t spec_idx = find_cmdline_var(src, usedlen);
if (spec_idx < 0) { 
*usedlen = 1;
return NULL;
}
if (src > srcstart && src[-1] == '\\') {
*usedlen = 0;
STRMOVE(src - 1, src); 
return NULL;
}
if (spec_idx == SPEC_CWORD
|| spec_idx == SPEC_CCWORD
|| spec_idx == SPEC_CEXPR) {
resultlen = find_ident_under_cursor(
&result,
spec_idx == SPEC_CWORD
? (FIND_IDENT | FIND_STRING)
: (spec_idx == SPEC_CEXPR
? (FIND_IDENT | FIND_STRING | FIND_EVAL)
: FIND_STRING));
if (resultlen == 0) {
*errormsg = (char_u *)"";
return NULL;
}
} else {
switch (spec_idx) {
case SPEC_PERC: 
if (curbuf->b_fname == NULL) {
result = (char_u *)"";
valid = 0; 
} else {
result = curbuf->b_fname;
tilde_file = STRCMP(result, "~") == 0;
}
break;
case SPEC_HASH: 
if (src[1] == '#') { 
result = arg_all();
resultbuf = result;
*usedlen = 2;
if (escaped != NULL)
*escaped = TRUE;
skip_mod = TRUE;
break;
}
s = src + 1;
if (*s == '<') { 
s++;
}
i = getdigits_int(&s, false, 0);
if (s == src + 2 && src[1] == '-') {
s--;
}
*usedlen = (size_t)(s - src); 
if (src[1] == '<' && i != 0) {
if (*usedlen < 2) {
*usedlen = 1;
return NULL;
}
result = (char_u *)tv_list_find_str(get_vim_var_list(VV_OLDFILES),
i - 1);
if (result == NULL) {
*errormsg = (char_u *)"";
return NULL;
}
} else {
if (i == 0 && src[1] == '<' && *usedlen > 1) {
*usedlen = 1;
}
buf = buflist_findnr(i);
if (buf == NULL) {
*errormsg = (char_u *)_(
"E194: No alternate file name to substitute for '#'");
return NULL;
}
if (lnump != NULL)
*lnump = ECMD_LAST;
if (buf->b_fname == NULL) {
result = (char_u *)"";
valid = 0; 
} else {
result = buf->b_fname;
tilde_file = STRCMP(result, "~") == 0;
}
}
break;
case SPEC_CFILE: 
result = file_name_at_cursor(FNAME_MESS|FNAME_HYP, 1L, NULL);
if (result == NULL) {
*errormsg = (char_u *)"";
return NULL;
}
resultbuf = result; 
break;
case SPEC_AFILE: 
if (autocmd_fname != NULL
&& !path_is_absolute(autocmd_fname)
&& !strequal("/", (char *)autocmd_fname)) {
result = (char_u *)FullName_save((char *)autocmd_fname, false);
xstrlcpy((char *)autocmd_fname, (char *)result, MAXPATHL);
xfree(result);
}
result = autocmd_fname;
if (result == NULL) {
*errormsg = (char_u *)_(
"E495: no autocommand file name to substitute for \"<afile>\"");
return NULL;
}
result = path_try_shorten_fname(result);
break;
case SPEC_ABUF: 
if (autocmd_bufnr <= 0) {
*errormsg = (char_u *)_(
"E496: no autocommand buffer number to substitute for \"<abuf>\"");
return NULL;
}
snprintf(strbuf, sizeof(strbuf), "%d", autocmd_bufnr);
result = (char_u *)strbuf;
break;
case SPEC_AMATCH: 
result = autocmd_match;
if (result == NULL) {
*errormsg = (char_u *)_(
"E497: no autocommand match name to substitute for \"<amatch>\"");
return NULL;
}
break;
case SPEC_SFILE: 
result = sourcing_name;
if (result == NULL) {
*errormsg = (char_u *)_(
"E498: no :source file name to substitute for \"<sfile>\"");
return NULL;
}
break;
case SPEC_SLNUM: 
if (sourcing_name == NULL || sourcing_lnum == 0) {
*errormsg = (char_u *)_("E842: no line number to use for \"<slnum>\"");
return NULL;
}
snprintf(strbuf, sizeof(strbuf), "%" PRIdLINENR, sourcing_lnum);
result = (char_u *)strbuf;
break;
case SPEC_SFLNUM: 
if (current_sctx.sc_lnum + sourcing_lnum == 0) {
*errormsg = (char_u *)_("E961: no line number to use for \"<sflnum>\"");
return NULL;
}
snprintf((char *)strbuf, sizeof(strbuf), "%" PRIdLINENR,
current_sctx.sc_lnum + sourcing_lnum);
result = (char_u *)strbuf;
break;
default:
*errormsg = (char_u *)"";
result = (char_u *)""; 
break;
}
resultlen = STRLEN(result);
if (src[*usedlen] == '<') {
(*usedlen)++;
if ((s = STRRCHR(result, '.')) != NULL && s >= path_tail(result)) {
resultlen = (size_t)(s - result);
}
} else if (!skip_mod) {
valid |= modify_fname(src, tilde_file, usedlen, &result,
&resultbuf, &resultlen);
if (result == NULL) {
*errormsg = (char_u *)"";
return NULL;
}
}
}
if (resultlen == 0 || valid != VALID_HEAD + VALID_PATH) {
if (valid != VALID_HEAD + VALID_PATH)
*errormsg = (char_u *)_(
"E499: Empty file name for '%' or '#', only works with \":p:h\"");
else
*errormsg = (char_u *)_("E500: Evaluates to an empty string");
result = NULL;
} else
result = vim_strnsave(result, resultlen);
xfree(resultbuf);
return result;
}
static char_u *arg_all(void)
{
int len;
int idx;
char_u *retval = NULL;
char_u *p;
for (;; ) {
len = 0;
for (idx = 0; idx < ARGCOUNT; ++idx) {
p = alist_name(&ARGLIST[idx]);
if (p == NULL) {
continue;
}
if (len > 0) {
if (retval != NULL)
retval[len] = ' ';
++len;
}
for (; *p != NUL; p++) {
if (*p == ' '
#if !defined(BACKSLASH_IN_FILENAME)
|| *p == '\\'
#endif
|| *p == '`') {
if (retval != NULL) {
retval[len] = '\\';
}
len++;
}
if (retval != NULL) {
retval[len] = *p;
}
len++;
}
}
if (retval != NULL) {
retval[len] = NUL;
break;
}
retval = xmalloc(len + 1);
}
return retval;
}
char_u *expand_sfile(char_u *arg)
{
char_u *errormsg;
size_t len;
char_u *result;
char_u *newres;
char_u *repl;
size_t srclen;
char_u *p;
result = vim_strsave(arg);
for (p = result; *p; ) {
if (STRNCMP(p, "<sfile>", 7) != 0)
++p;
else {
repl = eval_vars(p, result, &srclen, NULL, &errormsg, NULL);
if (errormsg != NULL) {
if (*errormsg)
emsg(errormsg);
xfree(result);
return NULL;
}
if (repl == NULL) { 
p += srclen;
continue;
}
len = STRLEN(result) - srclen + STRLEN(repl) + 1;
newres = xmalloc(len);
memmove(newres, result, (size_t)(p - result));
STRCPY(newres + (p - result), repl);
len = STRLEN(newres);
STRCAT(newres, p + srclen);
xfree(repl);
xfree(result);
result = newres;
p = newres + len; 
}
}
return result;
}
static void ex_shada(exarg_T *eap)
{
char_u *save_shada;
save_shada = p_shada;
if (*p_shada == NUL)
p_shada = (char_u *)"'100";
if (eap->cmdidx == CMD_rviminfo || eap->cmdidx == CMD_rshada) {
(void) shada_read_everything((char *) eap->arg, eap->forceit, false);
} else {
shada_write_file((char *) eap->arg, eap->forceit);
}
p_shada = save_shada;
}
void dialog_msg(char_u *buff, char *format, char_u *fname)
{
if (fname == NULL)
fname = (char_u *)_("Untitled");
vim_snprintf((char *)buff, DIALOG_MSG_SIZE, format, fname);
}
static void ex_behave(exarg_T *eap)
{
if (STRCMP(eap->arg, "mswin") == 0) {
set_option_value("selection", 0L, "exclusive", 0);
set_option_value("selectmode", 0L, "mouse,key", 0);
set_option_value("mousemodel", 0L, "popup", 0);
set_option_value("keymodel", 0L, "startsel,stopsel", 0);
} else if (STRCMP(eap->arg, "xterm") == 0) {
set_option_value("selection", 0L, "inclusive", 0);
set_option_value("selectmode", 0L, "", 0);
set_option_value("mousemodel", 0L, "extend", 0);
set_option_value("keymodel", 0L, "", 0);
} else {
EMSG2(_(e_invarg2), eap->arg);
}
}
char_u *get_behave_arg(expand_T *xp, int idx)
{
if (idx == 0)
return (char_u *)"mswin";
if (idx == 1)
return (char_u *)"xterm";
return NULL;
}
char_u *get_messages_arg(expand_T *xp FUNC_ATTR_UNUSED, int idx)
{
if (idx == 0) {
return (char_u *)"clear";
}
return NULL;
}
char_u *get_mapclear_arg(expand_T *xp FUNC_ATTR_UNUSED, int idx)
{
if (idx == 0) {
return (char_u *)"<buffer>";
}
return NULL;
}
static TriState filetype_detect = kNone;
static TriState filetype_plugin = kNone;
static TriState filetype_indent = kNone;
static void ex_filetype(exarg_T *eap)
{
char_u *arg = eap->arg;
bool plugin = false;
bool indent = false;
if (*eap->arg == NUL) {
smsg("filetype detection:%s plugin:%s indent:%s",
filetype_detect == kTrue ? "ON" : "OFF",
filetype_plugin == kTrue ? (filetype_detect == kTrue ? "ON" : "(on)") : "OFF", 
filetype_indent == kTrue ? (filetype_detect == kTrue ? "ON" : "(on)") : "OFF"); 
return;
}
for (;; ) {
if (STRNCMP(arg, "plugin", 6) == 0) {
plugin = true;
arg = skipwhite(arg + 6);
continue;
}
if (STRNCMP(arg, "indent", 6) == 0) {
indent = true;
arg = skipwhite(arg + 6);
continue;
}
break;
}
if (STRCMP(arg, "on") == 0 || STRCMP(arg, "detect") == 0) {
if (*arg == 'o' || !filetype_detect) {
source_runtime((char_u *)FILETYPE_FILE, DIP_ALL);
filetype_detect = kTrue;
if (plugin) {
source_runtime((char_u *)FTPLUGIN_FILE, DIP_ALL);
filetype_plugin = kTrue;
}
if (indent) {
source_runtime((char_u *)INDENT_FILE, DIP_ALL);
filetype_indent = kTrue;
}
}
if (*arg == 'd') {
(void)do_doautocmd((char_u *)"filetypedetect BufRead", true, NULL);
do_modelines(0);
}
} else if (STRCMP(arg, "off") == 0) {
if (plugin || indent) {
if (plugin) {
source_runtime((char_u *)FTPLUGOF_FILE, DIP_ALL);
filetype_plugin = kFalse;
}
if (indent) {
source_runtime((char_u *)INDOFF_FILE, DIP_ALL);
filetype_indent = kFalse;
}
} else {
source_runtime((char_u *)FTOFF_FILE, DIP_ALL);
filetype_detect = kFalse;
}
} else
EMSG2(_(e_invarg2), arg);
}
void filetype_maybe_enable(void)
{
if (filetype_detect == kNone) {
source_runtime((char_u *)FILETYPE_FILE, true);
filetype_detect = kTrue;
}
if (filetype_plugin == kNone) {
source_runtime((char_u *)FTPLUGIN_FILE, true);
filetype_plugin = kTrue;
}
if (filetype_indent == kNone) {
source_runtime((char_u *)INDENT_FILE, true);
filetype_indent = kTrue;
}
}
static void ex_setfiletype(exarg_T *eap)
{
if (!did_filetype) {
char_u *arg = eap->arg;
if (STRNCMP(arg, "FALLBACK ", 9) == 0) {
arg += 9;
}
set_option_value("filetype", 0L, (char *)arg, OPT_LOCAL);
if (arg != eap->arg) {
did_filetype = false;
}
}
}
static void ex_digraphs(exarg_T *eap)
{
if (*eap->arg != NUL) {
putdigraph(eap->arg);
} else {
listdigraphs(eap->forceit);
}
}
static void ex_set(exarg_T *eap)
{
int flags = 0;
if (eap->cmdidx == CMD_setlocal)
flags = OPT_LOCAL;
else if (eap->cmdidx == CMD_setglobal)
flags = OPT_GLOBAL;
(void)do_set(eap->arg, flags);
}
void set_no_hlsearch(bool flag)
{
no_hlsearch = flag;
set_vim_var_nr(VV_HLSEARCH, !no_hlsearch && p_hls);
}
static void ex_nohlsearch(exarg_T *eap)
{
set_no_hlsearch(true);
redraw_all_later(SOME_VALID);
}
static void ex_match(exarg_T *eap)
{
char_u *p;
char_u *g = NULL;
char_u *end;
int c;
int id;
if (eap->line2 <= 3) {
id = eap->line2;
} else {
EMSG(e_invcmd);
return;
}
if (!eap->skip) {
match_delete(curwin, id, false);
}
if (ends_excmd(*eap->arg)) {
end = eap->arg;
} else if ((STRNICMP(eap->arg, "none", 4) == 0
&& (ascii_iswhite(eap->arg[4]) || ends_excmd(eap->arg[4])))) {
end = eap->arg + 4;
} else {
p = skiptowhite(eap->arg);
if (!eap->skip) {
g = vim_strnsave(eap->arg, (int)(p - eap->arg));
}
p = skipwhite(p);
if (*p == NUL) {
xfree(g);
EMSG2(_(e_invarg2), eap->arg);
return;
}
end = skip_regexp(p + 1, *p, true, NULL);
if (!eap->skip) {
if (*end != NUL && !ends_excmd(*skipwhite(end + 1))) {
xfree(g);
eap->errmsg = e_trailing;
return;
}
if (*end != *p) {
xfree(g);
EMSG2(_(e_invarg2), p);
return;
}
c = *end;
*end = NUL;
match_add(curwin, (const char *)g, (const char *)p + 1, 10, id,
NULL, NULL);
xfree(g);
*end = c;
}
}
eap->nextcmd = find_nextcmd(end);
}
static void ex_fold(exarg_T *eap)
{
if (foldManualAllowed(TRUE))
foldCreate(eap->line1, eap->line2);
}
static void ex_foldopen(exarg_T *eap)
{
opFoldRange(eap->line1, eap->line2, eap->cmdidx == CMD_foldopen,
eap->forceit, FALSE);
}
static void ex_folddo(exarg_T *eap)
{
for (linenr_T lnum = eap->line1; lnum <= eap->line2; ++lnum) {
if (hasFolding(lnum, NULL, NULL) == (eap->cmdidx == CMD_folddoclosed)) {
ml_setmarked(lnum);
}
}
global_exe(eap->arg); 
ml_clearmarked(); 
}
bool is_loclist_cmd(int cmdidx)
FUNC_ATTR_PURE FUNC_ATTR_WARN_UNUSED_RESULT
{
if (cmdidx < 0 || cmdidx >= CMD_SIZE) {
return false;
}
return cmdnames[cmdidx].cmd_name[0] == 'l';
}
bool get_pressedreturn(void)
FUNC_ATTR_PURE FUNC_ATTR_WARN_UNUSED_RESULT
{
return ex_pressedreturn;
}
void set_pressedreturn(bool val)
{
ex_pressedreturn = val;
}
static void ex_terminal(exarg_T *eap)
{
char ex_cmd[1024];
if (*eap->arg != NUL) { 
char *name = (char *)vim_strsave_escaped(eap->arg, (char_u *)"\"\\");
snprintf(ex_cmd, sizeof(ex_cmd),
":enew%s | call termopen(\"%s\")",
eap->forceit ? "!" : "", name);
xfree(name);
} else { 
if (*p_sh == NUL) {
EMSG(_(e_shellempty));
return;
}
char **argv = shell_build_argv(NULL, NULL);
char **p = argv;
char tempstring[512];
char shell_argv[512] = { 0 };
while (*p != NULL) {
snprintf(tempstring, sizeof(tempstring), ",\"%s\"", *p);
xstrlcat(shell_argv, tempstring, sizeof(shell_argv));
p++;
}
shell_free_argv(argv);
snprintf(ex_cmd, sizeof(ex_cmd),
":enew%s | call termopen([%s])",
eap->forceit ? "!" : "", shell_argv + 1);
}
do_cmdline_cmd(ex_cmd);
}
bool cmd_can_preview(char_u *cmd)
{
if (cmd == NULL) {
return false;
}
cmd = skip_colon_white(cmd, true);
for (int len = modifier_len(cmd); len != 0; len = modifier_len(cmd)) {
cmd += len;
cmd = skip_colon_white(cmd, true);
}
exarg_T ea;
memset(&ea, 0, sizeof(ea));
ea.cmd = skip_range(cmd, NULL);
if (*ea.cmd == '*') {
ea.cmd = skipwhite(ea.cmd + 1);
}
char_u *end = find_command(&ea, NULL);
switch (ea.cmdidx) {
case CMD_substitute:
case CMD_smagic:
case CMD_snomagic:
if (*end && !ASCII_ISALNUM(*end)) {
return true;
}
break;
default:
break;
}
return false;
}
Dictionary commands_array(buf_T *buf)
{
Dictionary rv = ARRAY_DICT_INIT;
char str[20];
garray_T *gap = (buf == NULL) ? &ucmds : &buf->b_ucmds;
for (int i = 0; i < gap->ga_len; i++) {
char arg[2] = { 0, 0 };
Dictionary d = ARRAY_DICT_INIT;
ucmd_T *cmd = USER_CMD_GA(gap, i);
PUT(d, "name", STRING_OBJ(cstr_to_string((char *)cmd->uc_name)));
PUT(d, "definition", STRING_OBJ(cstr_to_string((char *)cmd->uc_rep)));
PUT(d, "script_id", INTEGER_OBJ(cmd->uc_script_ctx.sc_sid));
PUT(d, "bang", BOOLEAN_OBJ(!!(cmd->uc_argt & BANG)));
PUT(d, "bar", BOOLEAN_OBJ(!!(cmd->uc_argt & TRLBAR)));
PUT(d, "register", BOOLEAN_OBJ(!!(cmd->uc_argt & REGSTR)));
switch (cmd->uc_argt & (EXTRA|NOSPC|NEEDARG)) {
case 0: arg[0] = '0'; break;
case(EXTRA): arg[0] = '*'; break;
case(EXTRA|NOSPC): arg[0] = '?'; break;
case(EXTRA|NEEDARG): arg[0] = '+'; break;
case(EXTRA|NOSPC|NEEDARG): arg[0] = '1'; break;
}
PUT(d, "nargs", STRING_OBJ(cstr_to_string(arg)));
char *cmd_compl = get_command_complete(cmd->uc_compl);
PUT(d, "complete", (cmd_compl == NULL
? NIL : STRING_OBJ(cstr_to_string(cmd_compl))));
PUT(d, "complete_arg", cmd->uc_compl_arg == NULL
? NIL : STRING_OBJ(cstr_to_string((char *)cmd->uc_compl_arg)));
Object obj = NIL;
if (cmd->uc_argt & COUNT) {
if (cmd->uc_def >= 0) {
snprintf(str, sizeof(str), "%" PRId64, (int64_t)cmd->uc_def);
obj = STRING_OBJ(cstr_to_string(str)); 
} else {
obj = STRING_OBJ(cstr_to_string("0")); 
}
}
PUT(d, "count", obj);
obj = NIL;
if (cmd->uc_argt & RANGE) {
if (cmd->uc_argt & DFLALL) {
obj = STRING_OBJ(cstr_to_string("%")); 
} else if (cmd->uc_def >= 0) {
snprintf(str, sizeof(str), "%" PRId64, (int64_t)cmd->uc_def);
obj = STRING_OBJ(cstr_to_string(str)); 
} else {
obj = STRING_OBJ(cstr_to_string(".")); 
}
}
PUT(d, "range", obj);
obj = NIL;
for (int j = 0; addr_type_complete[j].expand != -1; j++) {
if (addr_type_complete[j].expand != ADDR_LINES
&& addr_type_complete[j].expand == cmd->uc_addr_type) {
obj = STRING_OBJ(cstr_to_string(addr_type_complete[j].name));
break;
}
}
PUT(d, "addr", obj);
PUT(rv, (char *)cmd->uc_name, DICTIONARY_OBJ(d));
}
return rv;
}
