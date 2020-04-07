#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include "nvim/assert.h"
#include "nvim/log.h"
#include "nvim/vim.h"
#include "nvim/ascii.h"
#include "nvim/arabic.h"
#include "nvim/ex_getln.h"
#include "nvim/buffer.h"
#include "nvim/charset.h"
#include "nvim/cursor.h"
#include "nvim/digraph.h"
#include "nvim/edit.h"
#include "nvim/eval.h"
#include "nvim/ex_cmds.h"
#include "nvim/ex_cmds2.h"
#include "nvim/ex_docmd.h"
#include "nvim/ex_eval.h"
#include "nvim/fileio.h"
#include "nvim/func_attr.h"
#include "nvim/getchar.h"
#include "nvim/highlight.h"
#include "nvim/if_cscope.h"
#include "nvim/indent.h"
#include "nvim/main.h"
#include "nvim/mark.h"
#include "nvim/mbyte.h"
#include "nvim/memline.h"
#include "nvim/menu.h"
#include "nvim/message.h"
#include "nvim/misc1.h"
#include "nvim/memory.h"
#include "nvim/cursor_shape.h"
#include "nvim/keymap.h"
#include "nvim/garray.h"
#include "nvim/move.h"
#include "nvim/mouse.h"
#include "nvim/ops.h"
#include "nvim/option.h"
#include "nvim/os_unix.h"
#include "nvim/path.h"
#include "nvim/popupmnu.h"
#include "nvim/regexp.h"
#include "nvim/screen.h"
#include "nvim/search.h"
#include "nvim/sign.h"
#include "nvim/strings.h"
#include "nvim/state.h"
#include "nvim/syntax.h"
#include "nvim/tag.h"
#include "nvim/window.h"
#include "nvim/ui.h"
#include "nvim/os/input.h"
#include "nvim/os/os.h"
#include "nvim/event/loop.h"
#include "nvim/os/time.h"
#include "nvim/lib/kvec.h"
#include "nvim/api/private/helpers.h"
#include "nvim/highlight_defs.h"
#include "nvim/viml/parser/parser.h"
#include "nvim/viml/parser/expressions.h"
typedef struct {
int start; 
int end; 
int attr; 
} CmdlineColorChunk;
typedef kvec_t(CmdlineColorChunk) CmdlineColors;
typedef struct {
unsigned prompt_id; 
char *cmdbuff; 
CmdlineColors colors; 
} ColoredCmdline;
typedef enum {
kCmdRedrawNone,
kCmdRedrawPos,
kCmdRedrawAll,
} CmdRedraw;
struct cmdline_info {
char_u *cmdbuff; 
int cmdbufflen; 
int cmdlen; 
int cmdpos; 
int cmdspos; 
int cmdfirstc; 
int cmdindent; 
char_u *cmdprompt; 
int cmdattr; 
int overstrike; 
expand_T *xpc; 
int xp_context; 
char_u *xp_arg; 
int input_fn; 
unsigned prompt_id; 
Callback highlight_callback; 
ColoredCmdline last_colors; 
int level; 
struct cmdline_info *prev_ccline; 
char special_char; 
bool special_shift; 
CmdRedraw redraw_state; 
};
static unsigned last_prompt_id = 0;
typedef struct {
colnr_T vs_curswant;
colnr_T vs_leftcol;
linenr_T vs_topline;
int vs_topfill;
linenr_T vs_botline;
int vs_empty_rows;
} viewstate_T;
typedef struct command_line_state {
VimState state;
int firstc;
long count;
int indent;
int c;
int gotesc; 
int do_abbr; 
char_u *lookfor; 
int hiscnt; 
int save_hiscnt; 
int histype; 
pos_T search_start; 
pos_T save_cursor;
viewstate_T init_viewstate;
viewstate_T old_viewstate;
pos_T match_start;
pos_T match_end;
int did_incsearch;
int incsearch_postponed;
int did_wild_list; 
int wim_index; 
int res;
int save_msg_scroll;
int save_State; 
char_u *save_p_icm;
int some_key_typed; 
int ignore_drag_release;
int break_ctrl_c;
expand_T xpc;
long *b_im_ptr;
} CommandLineState;
typedef struct cmdline_info CmdlineInfo;
static struct cmdline_info ccline;
static int cmd_showtail; 
static int new_cmdpos; 
static Array cmdline_block = ARRAY_DICT_INIT;
typedef void *(*user_expand_func_T)(const char_u *, int, typval_T *);
static histentry_T *(history[HIST_COUNT]) = {NULL, NULL, NULL, NULL, NULL};
static int hisidx[HIST_COUNT] = {-1, -1, -1, -1, -1}; 
static int hisnum[HIST_COUNT] = {0, 0, 0, 0, 0};
static int hislen = 0; 
static bool getln_interrupted_highlight = false;
static pumitem_T *compl_match_array = NULL;
static int compl_match_arraysize;
static int compl_startcol;
static int compl_selected;
#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "ex_getln.c.generated.h"
#endif
static int cmd_hkmap = 0; 
static void save_viewstate(viewstate_T *vs)
FUNC_ATTR_NONNULL_ALL
{
vs->vs_curswant = curwin->w_curswant;
vs->vs_leftcol = curwin->w_leftcol;
vs->vs_topline = curwin->w_topline;
vs->vs_topfill = curwin->w_topfill;
vs->vs_botline = curwin->w_botline;
vs->vs_empty_rows = curwin->w_empty_rows;
}
static void restore_viewstate(viewstate_T *vs)
FUNC_ATTR_NONNULL_ALL
{
curwin->w_curswant = vs->vs_curswant;
curwin->w_leftcol = vs->vs_leftcol;
curwin->w_topline = vs->vs_topline;
curwin->w_topfill = vs->vs_topfill;
curwin->w_botline = vs->vs_botline;
curwin->w_empty_rows = vs->vs_empty_rows;
}
static uint8_t *command_line_enter(int firstc, long count, int indent)
{
static int cmdline_level = 0;
cmdline_level++;
CommandLineState state = {
.firstc = firstc,
.count = count,
.indent = indent,
.save_msg_scroll = msg_scroll,
.save_State = State,
.ignore_drag_release = true,
.match_start = curwin->w_cursor,
};
CommandLineState *s = &state;
s->save_p_icm = vim_strsave(p_icm);
save_viewstate(&state.init_viewstate);
if (s->firstc == -1) {
s->firstc = NUL;
s->break_ctrl_c = true;
}
if (s->firstc == ':' || s->firstc == '=' || s->firstc == '>') {
cmd_hkmap = 0;
}
ccline.prompt_id = last_prompt_id++;
ccline.level = cmdline_level;
ccline.overstrike = false; 
clearpos(&s->match_end);
s->save_cursor = curwin->w_cursor; 
s->search_start = curwin->w_cursor;
save_viewstate(&state.old_viewstate);
assert(indent >= 0);
ccline.cmdfirstc = (s->firstc == '@' ? 0 : s->firstc);
ccline.cmdindent = (s->firstc > 0 ? s->indent : 0);
alloc_cmdbuff(exmode_active ? 250 : s->indent + 1);
ccline.cmdlen = ccline.cmdpos = 0;
ccline.cmdbuff[0] = NUL;
ccline.last_colors = (ColoredCmdline){ .cmdbuff = NULL,
.colors = KV_INITIAL_VALUE };
sb_text_start_cmdline();
if (s->firstc <= 0) {
memset(ccline.cmdbuff, ' ', (size_t)s->indent);
ccline.cmdbuff[s->indent] = NUL;
ccline.cmdpos = s->indent;
ccline.cmdspos = s->indent;
ccline.cmdlen = s->indent;
}
ExpandInit(&s->xpc);
ccline.xpc = &s->xpc;
if (curwin->w_p_rl && *curwin->w_p_rlc == 's'
&& (s->firstc == '/' || s->firstc == '?')) {
cmdmsg_rl = true;
} else {
cmdmsg_rl = false;
}
msg_grid_validate();
redir_off = true; 
if (!cmd_silent) {
gotocmdline(true);
redrawcmdprompt(); 
ccline.cmdspos = cmd_startcol();
if (!msg_scroll) {
msg_ext_clear(false);
}
}
s->xpc.xp_context = EXPAND_NOTHING;
s->xpc.xp_backslash = XP_BS_NONE;
#if !defined(BACKSLASH_IN_FILENAME)
s->xpc.xp_shell = false;
#endif
if (ccline.input_fn) {
s->xpc.xp_context = ccline.xp_context;
s->xpc.xp_pattern = ccline.cmdbuff;
s->xpc.xp_arg = ccline.xp_arg;
}
msg_scroll = false;
State = CMDLINE;
if (s->firstc == '/' || s->firstc == '?' || s->firstc == '@') {
if (curbuf->b_p_imsearch == B_IMODE_USE_INSERT) {
s->b_im_ptr = &curbuf->b_p_iminsert;
} else {
s->b_im_ptr = &curbuf->b_p_imsearch;
}
if (*s->b_im_ptr == B_IMODE_LMAP) {
State |= LANGMAP;
}
}
setmouse();
ui_cursor_shape(); 
init_history();
s->hiscnt = hislen; 
s->histype = hist_char2type(s->firstc);
do_digraph(-1); 
if (did_emsg) {
redrawcmd();
}
if (!cmd_silent && msg_scrolled == 0) {
curwin->w_redr_status = true;
redraw_statuslines();
}
did_emsg = false;
got_int = false;
s->state.check = command_line_check;
s->state.execute = command_line_execute;
TryState tstate;
Error err = ERROR_INIT;
bool tl_ret = true;
dict_T *dict = get_vim_var_dict(VV_EVENT);
char firstcbuf[2];
firstcbuf[0] = (char)(firstc > 0 ? firstc : '-');
firstcbuf[1] = 0;
if (has_event(EVENT_CMDLINEENTER)) {
tv_dict_add_str(dict, S_LEN("cmdtype"), firstcbuf);
tv_dict_add_nr(dict, S_LEN("cmdlevel"), ccline.level);
tv_dict_set_keys_readonly(dict);
try_enter(&tstate);
apply_autocmds(EVENT_CMDLINEENTER, (char_u *)firstcbuf, (char_u *)firstcbuf,
false, curbuf);
tv_dict_clear(dict);
tl_ret = try_leave(&tstate, &err);
if (!tl_ret && ERROR_SET(&err)) {
msg_putchar('\n');
msg_printf_attr(HL_ATTR(HLF_E)|MSG_HIST, (char *)e_autocmd_err, err.msg);
api_clear_error(&err);
redrawcmd();
}
tl_ret = true;
}
state_enter(&s->state);
if (has_event(EVENT_CMDLINELEAVE)) {
tv_dict_add_str(dict, S_LEN("cmdtype"), firstcbuf);
tv_dict_add_nr(dict, S_LEN("cmdlevel"), ccline.level);
tv_dict_set_keys_readonly(dict);
tv_dict_add_special(dict, S_LEN("abort"),
s->gotesc ? kSpecialVarTrue : kSpecialVarFalse);
try_enter(&tstate);
apply_autocmds(EVENT_CMDLINELEAVE, (char_u *)firstcbuf, (char_u *)firstcbuf,
false, curbuf);
tl_ret = try_leave(&tstate, &err);
if (tv_dict_get_number(dict, "abort") != 0) {
s->gotesc = 1;
}
tv_dict_clear(dict);
}
cmdmsg_rl = false;
ExpandCleanup(&s->xpc);
ccline.xpc = NULL;
if (s->did_incsearch) {
if (s->gotesc) {
curwin->w_cursor = s->save_cursor;
} else {
if (!equalpos(s->save_cursor, s->search_start)) {
curwin->w_cursor = s->save_cursor;
setpcmark();
}
curwin->w_cursor = s->search_start; 
}
restore_viewstate(&s->old_viewstate);
highlight_match = false;
validate_cursor(); 
redraw_all_later(SOME_VALID);
}
if (ccline.cmdbuff != NULL) {
if (s->histype != HIST_INVALID
&& ccline.cmdlen
&& s->firstc != NUL
&& (s->some_key_typed || s->histype == HIST_SEARCH)) {
add_to_history(s->histype, ccline.cmdbuff, true,
s->histype == HIST_SEARCH ? s->firstc : NUL);
if (s->firstc == ':') {
xfree(new_last_cmdline);
new_last_cmdline = vim_strsave(ccline.cmdbuff);
}
}
if (s->gotesc) {
abandon_cmdline();
}
}
msg_check();
msg_scroll = s->save_msg_scroll;
redir_off = false;
if (!tl_ret && ERROR_SET(&err)) {
msg_putchar('\n');
msg_printf_attr(HL_ATTR(HLF_E)|MSG_HIST, (char *)e_autocmd_err, err.msg);
api_clear_error(&err);
}
if (s->some_key_typed && tl_ret) {
need_wait_return = false;
}
set_string_option_direct((char_u *)"icm", -1, s->save_p_icm, OPT_FREE,
SID_NONE);
State = s->save_State;
setmouse();
ui_cursor_shape(); 
xfree(s->save_p_icm);
xfree(ccline.last_colors.cmdbuff);
kv_destroy(ccline.last_colors.colors);
sb_text_end_cmdline();
char_u *p = ccline.cmdbuff;
if (ui_has(kUICmdline)) {
ui_call_cmdline_hide(ccline.level);
msg_ext_clear_later();
}
cmdline_level--;
return p;
}
static int command_line_check(VimState *state)
{
redir_off = true; 
quit_more = false; 
did_emsg = false; 
cursorcmd(); 
ui_cursor_shape();
return 1;
}
static int command_line_execute(VimState *state, int key)
{
if (key == K_IGNORE || key == K_NOP) {
return -1; 
}
CommandLineState *s = (CommandLineState *)state;
s->c = key;
if (s->c == K_EVENT || s->c == K_COMMAND) {
if (s->c == K_EVENT) {
multiqueue_process_events(main_loop.events);
} else {
do_cmdline(NULL, getcmdkeycmd, NULL, DOCMD_NOWAIT);
}
if (!cmdline_was_last_drawn) {
redrawcmdline();
}
return 1;
}
if (KeyTyped) {
s->some_key_typed = true;
if (cmd_hkmap) {
s->c = hkmap(s->c);
}
if (cmdmsg_rl && !KeyStuffed) {
switch (s->c) {
case K_RIGHT: s->c = K_LEFT; break;
case K_S_RIGHT: s->c = K_S_LEFT; break;
case K_C_RIGHT: s->c = K_C_LEFT; break;
case K_LEFT: s->c = K_RIGHT; break;
case K_S_LEFT: s->c = K_S_RIGHT; break;
case K_C_LEFT: s->c = K_C_RIGHT; break;
}
}
}
if ((s->c == Ctrl_C)
&& s->firstc != '@'
&& !s->break_ctrl_c
&& !global_busy) {
got_int = false;
}
if (s->lookfor != NULL
&& s->c != K_S_DOWN && s->c != K_S_UP
&& s->c != K_DOWN && s->c != K_UP
&& s->c != K_PAGEDOWN && s->c != K_PAGEUP
&& s->c != K_KPAGEDOWN && s->c != K_KPAGEUP
&& s->c != K_LEFT && s->c != K_RIGHT
&& (s->xpc.xp_numfiles > 0 || (s->c != Ctrl_P && s->c != Ctrl_N))) {
XFREE_CLEAR(s->lookfor);
}
if (s->c != p_wc && s->c == K_S_TAB && s->xpc.xp_numfiles > 0) {
s->c = Ctrl_P;
}
if (s->did_wild_list && p_wmnu) {
if (s->c == K_LEFT) {
s->c = Ctrl_P;
} else if (s->c == K_RIGHT) {
s->c = Ctrl_N;
}
}
if (compl_match_array || s->did_wild_list) {
if (s->c == Ctrl_E) {
s->res = nextwild(&s->xpc, WILD_CANCEL, WILD_NO_BEEP,
s->firstc != '@');
} else if (s->c == Ctrl_Y) {
s->res = nextwild(&s->xpc, WILD_APPLY, WILD_NO_BEEP,
s->firstc != '@');
s->c = Ctrl_E;
}
}
if (s->xpc.xp_context == EXPAND_MENUNAMES && p_wmnu
&& ccline.cmdpos > 1
&& ccline.cmdbuff[ccline.cmdpos - 1] == '.'
&& ccline.cmdbuff[ccline.cmdpos - 2] != '\\'
&& (s->c == '\n' || s->c == '\r' || s->c == K_KENTER)) {
s->c = K_DOWN;
}
if (!(s->c == p_wc && KeyTyped) && s->c != p_wcm
&& s->c != Ctrl_N && s->c != Ctrl_P && s->c != Ctrl_A
&& s->c != Ctrl_L) {
if (compl_match_array) {
pum_undisplay(true);
XFREE_CLEAR(compl_match_array);
}
if (s->xpc.xp_numfiles != -1) {
(void)ExpandOne(&s->xpc, NULL, NULL, 0, WILD_FREE);
}
s->did_wild_list = false;
if (!p_wmnu || (s->c != K_UP && s->c != K_DOWN)) {
s->xpc.xp_context = EXPAND_NOTHING;
}
s->wim_index = 0;
if (p_wmnu && wild_menu_showing != 0) {
const bool skt = KeyTyped;
int old_RedrawingDisabled = RedrawingDisabled;
if (ccline.input_fn) {
RedrawingDisabled = 0;
}
if (wild_menu_showing == WM_SCROLLED) {
cmdline_row--;
redrawcmd();
wild_menu_showing = 0;
} else if (save_p_ls != -1) {
p_ls = save_p_ls;
p_wmh = save_p_wmh;
last_status(false);
update_screen(VALID); 
redrawcmd();
save_p_ls = -1;
wild_menu_showing = 0;
} else if (wild_menu_showing != WM_LIST) {
win_redraw_last_status(topframe);
wild_menu_showing = 0; 
redraw_statuslines();
} else {
wild_menu_showing = 0;
}
KeyTyped = skt;
if (ccline.input_fn) {
RedrawingDisabled = old_RedrawingDisabled;
}
}
}
if (s->xpc.xp_context == EXPAND_MENUNAMES && p_wmnu) {
if (s->c == K_DOWN && ccline.cmdpos > 0
&& ccline.cmdbuff[ccline.cmdpos - 1] == '.') {
s->c = (int)p_wc;
} else if (s->c == K_UP) {
int found = false;
int j = (int)(s->xpc.xp_pattern - ccline.cmdbuff);
int i = 0;
while (--j > 0) {
if (ccline.cmdbuff[j] == ' '
&& ccline.cmdbuff[j - 1] != '\\') {
i = j + 1;
break;
}
if (ccline.cmdbuff[j] == '.'
&& ccline.cmdbuff[j - 1] != '\\') {
if (found) {
i = j + 1;
break;
} else {
found = true;
}
}
}
if (i > 0) {
cmdline_del(i);
}
s->c = (int)p_wc;
s->xpc.xp_context = EXPAND_NOTHING;
}
}
if ((s->xpc.xp_context == EXPAND_FILES
|| s->xpc.xp_context == EXPAND_DIRECTORIES
|| s->xpc.xp_context == EXPAND_SHELLCMD) && p_wmnu) {
char_u upseg[5];
upseg[0] = PATHSEP;
upseg[1] = '.';
upseg[2] = '.';
upseg[3] = PATHSEP;
upseg[4] = NUL;
if (s->c == K_DOWN
&& ccline.cmdpos > 0
&& ccline.cmdbuff[ccline.cmdpos - 1] == PATHSEP
&& (ccline.cmdpos < 3
|| ccline.cmdbuff[ccline.cmdpos - 2] != '.'
|| ccline.cmdbuff[ccline.cmdpos - 3] != '.')) {
s->c = (int)p_wc;
} else if (STRNCMP(s->xpc.xp_pattern, upseg + 1, 3) == 0
&& s->c == K_DOWN) {
int found = false;
int j = ccline.cmdpos;
int i = (int)(s->xpc.xp_pattern - ccline.cmdbuff);
while (--j > i) {
j -= utf_head_off(ccline.cmdbuff, ccline.cmdbuff + j);
if (vim_ispathsep(ccline.cmdbuff[j])) {
found = true;
break;
}
}
if (found
&& ccline.cmdbuff[j - 1] == '.'
&& ccline.cmdbuff[j - 2] == '.'
&& (vim_ispathsep(ccline.cmdbuff[j - 3]) || j == i + 2)) {
cmdline_del(j - 2);
s->c = (int)p_wc;
}
} else if (s->c == K_UP) {
int found = false;
int j = ccline.cmdpos - 1;
int i = (int)(s->xpc.xp_pattern - ccline.cmdbuff);
while (--j > i) {
j -= utf_head_off(ccline.cmdbuff, ccline.cmdbuff + j);
if (vim_ispathsep(ccline.cmdbuff[j])
#if defined(BACKSLASH_IN_FILENAME)
&& vim_strchr((const char_u *)" *?[{`$%#", ccline.cmdbuff[j + 1])
== NULL
#endif
) {
if (found) {
i = j + 1;
break;
} else {
found = true;
}
}
}
if (!found) {
j = i;
} else if (STRNCMP(ccline.cmdbuff + j, upseg, 4) == 0) {
j += 4;
} else if (STRNCMP(ccline.cmdbuff + j, upseg + 1, 3) == 0
&& j == i) {
j += 3;
} else {
j = 0;
}
if (j > 0) {
cmdline_del(j);
put_on_cmdline(upseg + 1, 3, false);
} else if (ccline.cmdpos > i) {
cmdline_del(i);
}
s->c = (int)p_wc;
KeyTyped = true;
}
}
if (s->c == Ctrl_BSL) {
no_mapping++;
s->c = plain_vgetc();
no_mapping--;
if (s->c != Ctrl_N
&& s->c != Ctrl_G
&& (s->c != 'e'
|| (ccline.cmdfirstc == '=' && KeyTyped)
|| cmdline_star > 0)) {
vungetc(s->c);
s->c = Ctrl_BSL;
} else if (s->c == 'e') {
char_u *p = NULL;
int len;
if (ccline.cmdpos == ccline.cmdlen) {
new_cmdpos = 99999; 
} else {
new_cmdpos = ccline.cmdpos;
}
s->c = get_expr_register();
if (s->c == '=') {
CmdlineInfo save_ccline;
save_cmdline(&save_ccline);
textlock++;
p = get_expr_line();
textlock--;
restore_cmdline(&save_ccline);
if (p != NULL) {
len = (int)STRLEN(p);
realloc_cmdbuff(len + 1);
ccline.cmdlen = len;
STRCPY(ccline.cmdbuff, p);
xfree(p);
if (new_cmdpos > ccline.cmdlen) {
ccline.cmdpos = ccline.cmdlen;
} else {
ccline.cmdpos = new_cmdpos;
}
KeyTyped = false; 
redrawcmd();
return command_line_changed(s);
}
}
beep_flush();
got_int = false; 
did_emsg = false;
emsg_on_display = false;
redrawcmd();
return command_line_not_changed(s);
} else {
if (s->c == Ctrl_G && p_im && restart_edit == 0) {
restart_edit = 'a';
}
s->gotesc = true; 
return 0; 
}
}
if (s->c == cedit_key || s->c == K_CMDWIN) {
if ((s->c == K_CMDWIN || ex_normal_busy == 0)
&& got_int == false) {
s->c = open_cmdwin();
s->some_key_typed = true;
}
} else {
s->c = do_digraph(s->c);
}
if (s->c == '\n'
|| s->c == '\r'
|| s->c == K_KENTER
|| (s->c == ESC
&& (!KeyTyped || vim_strchr(p_cpo, CPO_ESC) != NULL))) {
if (exmode_active
&& s->c != ESC
&& ccline.cmdpos == ccline.cmdlen
&& ccline.cmdpos > 0
&& ccline.cmdbuff[ccline.cmdpos - 1] == '\\') {
if (s->c == K_KENTER) {
s->c = '\n';
}
} else {
s->gotesc = false; 
if (ccheck_abbr(s->c + ABBR_OFF)) {
return command_line_changed(s);
}
if (!cmd_silent) {
if (!ui_has(kUICmdline)) {
cmd_cursor_goto(msg_row, 0);
}
ui_flush();
}
return 0;
}
}
if ((s->c == p_wc && !s->gotesc && KeyTyped) || s->c == p_wcm) {
if (s->xpc.xp_numfiles > 0) { 
if (s->xpc.xp_numfiles > 1
&& !s->did_wild_list
&& ((wim_flags[s->wim_index] & WIM_LIST)
|| (p_wmnu && (wim_flags[s->wim_index] & WIM_FULL) != 0))) {
(void)showmatches(&s->xpc, p_wmnu
&& ((wim_flags[s->wim_index] & WIM_LIST) == 0));
redrawcmd();
s->did_wild_list = true;
}
if (wim_flags[s->wim_index] & WIM_LONGEST) {
s->res = nextwild(&s->xpc, WILD_LONGEST, WILD_NO_BEEP,
s->firstc != '@');
} else if (wim_flags[s->wim_index] & WIM_FULL) {
s->res = nextwild(&s->xpc, WILD_NEXT, WILD_NO_BEEP,
s->firstc != '@');
} else {
s->res = OK; 
}
} else { 
s->wim_index = 0;
int j = ccline.cmdpos;
if (wim_flags[0] & WIM_LONGEST) {
s->res = nextwild(&s->xpc, WILD_LONGEST, WILD_NO_BEEP,
s->firstc != '@');
} else {
s->res = nextwild(&s->xpc, WILD_EXPAND_KEEP, WILD_NO_BEEP,
s->firstc != '@');
}
if (got_int) {
(void)vpeekc(); 
got_int = false; 
(void)ExpandOne(&s->xpc, NULL, NULL, 0, WILD_FREE);
s->xpc.xp_context = EXPAND_NOTHING;
return command_line_changed(s);
}
if (s->res == OK && s->xpc.xp_numfiles > 1) {
if (wim_flags[0] == WIM_LONGEST && ccline.cmdpos == j) {
s->wim_index = 1;
}
if ((wim_flags[s->wim_index] & WIM_LIST)
|| (p_wmnu && (wim_flags[s->wim_index] & WIM_FULL) != 0)) {
if (!(wim_flags[0] & WIM_LONGEST)) {
int p_wmnu_save = p_wmnu;
p_wmnu = 0;
nextwild(&s->xpc, WILD_PREV, 0, s->firstc != '@');
p_wmnu = p_wmnu_save;
}
(void)showmatches(&s->xpc, p_wmnu
&& ((wim_flags[s->wim_index] & WIM_LIST) == 0));
redrawcmd();
s->did_wild_list = true;
if (wim_flags[s->wim_index] & WIM_LONGEST) {
nextwild(&s->xpc, WILD_LONGEST, WILD_NO_BEEP,
s->firstc != '@');
} else if (wim_flags[s->wim_index] & WIM_FULL) {
nextwild(&s->xpc, WILD_NEXT, WILD_NO_BEEP,
s->firstc != '@');
}
} else {
vim_beep(BO_WILD);
}
} else if (s->xpc.xp_numfiles == -1) {
s->xpc.xp_context = EXPAND_NOTHING;
}
}
if (s->wim_index < 3) {
++s->wim_index;
}
if (s->c == ESC) {
s->gotesc = true;
}
if (s->res == OK) {
return command_line_changed(s);
}
}
s->gotesc = false;
if (s->c == K_S_TAB && KeyTyped) {
if (nextwild(&s->xpc, WILD_EXPAND_KEEP, 0, s->firstc != '@') == OK) {
showmatches(&s->xpc, p_wmnu
&& ((wim_flags[s->wim_index] & WIM_LIST) == 0));
nextwild(&s->xpc, WILD_PREV, 0, s->firstc != '@');
nextwild(&s->xpc, WILD_PREV, 0, s->firstc != '@');
return command_line_changed(s);
}
}
if (s->c == NUL || s->c == K_ZERO) {
s->c = NL;
}
s->do_abbr = true; 
return command_line_handle_key(s);
}
static void command_line_next_incsearch(CommandLineState *s, bool next_match)
{
ui_busy_start();
ui_flush();
pos_T t;
char_u *pat;
int search_flags = SEARCH_NOOF;
if (s->firstc == ccline.cmdbuff[0]) {
pat = last_search_pattern();
} else {
pat = ccline.cmdbuff;
}
save_last_search_pattern();
if (next_match) {
t = s->match_end;
if (lt(s->match_start, s->match_end)) {
(void)decl(&t);
}
search_flags += SEARCH_COL;
} else {
t = s->match_start;
}
if (!p_hls) {
search_flags += SEARCH_KEEP;
}
emsg_off++;
int found = searchit(curwin, curbuf, &t, NULL,
next_match ? FORWARD : BACKWARD,
pat, s->count, search_flags,
RE_SEARCH, NULL);
emsg_off--;
ui_busy_stop();
if (found) {
s->search_start = s->match_start;
s->match_end = t;
s->match_start = t;
if (!next_match && s->firstc == '/') {
s->search_start = t;
(void)decl(&s->search_start);
} else if (next_match && s->firstc == '?') {
s->search_start = t;
(void)incl(&s->search_start);
}
if (lt(t, s->search_start) && next_match) {
s->search_start = t;
if (s->firstc == '?') {
(void)incl(&s->search_start);
} else {
(void)decl(&s->search_start);
}
}
set_search_match(&s->match_end);
curwin->w_cursor = s->match_start;
changed_cline_bef_curs();
update_topline();
validate_cursor();
highlight_match = true;
save_viewstate(&s->old_viewstate);
update_screen(NOT_VALID);
redrawcmdline();
} else {
vim_beep(BO_ERROR);
}
restore_last_search_pattern();
return;
}
static void command_line_next_histidx(CommandLineState *s, bool next_match)
{
int j = (int)STRLEN(s->lookfor);
for (;; ) {
if (!next_match) {
if (s->hiscnt == hislen) {
s->hiscnt = hisidx[s->histype];
} else if (s->hiscnt == 0 && hisidx[s->histype] != hislen - 1) {
s->hiscnt = hislen - 1;
} else if (s->hiscnt != hisidx[s->histype] + 1) {
s->hiscnt--;
} else {
s->hiscnt = s->save_hiscnt;
break;
}
} else { 
if (s->hiscnt == hisidx[s->histype]) {
s->hiscnt = hislen;
break;
}
if (s->hiscnt == hislen) {
break;
}
if (s->hiscnt == hislen - 1) {
s->hiscnt = 0;
} else {
s->hiscnt++;
}
}
if (s->hiscnt < 0 || history[s->histype][s->hiscnt].hisstr == NULL) {
s->hiscnt = s->save_hiscnt;
break;
}
if ((s->c != K_UP && s->c != K_DOWN)
|| s->hiscnt == s->save_hiscnt
|| STRNCMP(history[s->histype][s->hiscnt].hisstr,
s->lookfor, (size_t)j) == 0) {
break;
}
}
}
static int command_line_handle_key(CommandLineState *s)
{
switch (s->c) {
case K_BS:
case Ctrl_H:
case K_DEL:
case K_KDEL:
case Ctrl_W:
if (s->c == K_KDEL) {
s->c = K_DEL;
}
if (s->c == K_DEL && ccline.cmdpos != ccline.cmdlen) {
++ccline.cmdpos;
}
if (s->c == K_DEL) {
ccline.cmdpos += mb_off_next(ccline.cmdbuff,
ccline.cmdbuff + ccline.cmdpos);
}
if (ccline.cmdpos > 0) {
char_u *p;
int j = ccline.cmdpos;
p = mb_prevptr(ccline.cmdbuff, ccline.cmdbuff + j);
if (s->c == Ctrl_W) {
while (p > ccline.cmdbuff && ascii_isspace(*p)) {
p = mb_prevptr(ccline.cmdbuff, p);
}
int i = mb_get_class(p);
while (p > ccline.cmdbuff && mb_get_class(p) == i) {
p = mb_prevptr(ccline.cmdbuff, p);
}
if (mb_get_class(p) != i) {
p += utfc_ptr2len(p);
}
}
ccline.cmdpos = (int)(p - ccline.cmdbuff);
ccline.cmdlen -= j - ccline.cmdpos;
int i = ccline.cmdpos;
while (i < ccline.cmdlen) {
ccline.cmdbuff[i++] = ccline.cmdbuff[j++];
}
ccline.cmdbuff[ccline.cmdlen] = NUL;
if (ccline.cmdlen == 0) {
s->search_start = s->save_cursor;
s->old_viewstate = s->init_viewstate;
}
redrawcmd();
} else if (ccline.cmdlen == 0 && s->c != Ctrl_W
&& ccline.cmdprompt == NULL && s->indent == 0) {
if (exmode_active || ccline.cmdfirstc == '>') {
return command_line_not_changed(s);
}
XFREE_CLEAR(ccline.cmdbuff); 
if (!cmd_silent && !ui_has(kUICmdline)) {
if (cmdmsg_rl) {
msg_col = Columns;
} else {
msg_col = 0;
}
msg_putchar(' '); 
}
s->search_start = s->save_cursor;
redraw_cmdline = true;
return 0; 
}
return command_line_changed(s);
case K_INS:
case K_KINS:
ccline.overstrike = !ccline.overstrike;
ui_cursor_shape(); 
return command_line_not_changed(s);
case Ctrl_HAT:
if (map_to_exists_mode("", LANGMAP, false)) {
State ^= LANGMAP;
if (s->b_im_ptr != NULL) {
if (State & LANGMAP) {
*s->b_im_ptr = B_IMODE_LMAP;
} else {
*s->b_im_ptr = B_IMODE_NONE;
}
}
}
if (s->b_im_ptr != NULL) {
if (s->b_im_ptr == &curbuf->b_p_iminsert) {
set_iminsert_global();
} else {
set_imsearch_global();
}
}
ui_cursor_shape(); 
status_redraw_curbuf();
return command_line_not_changed(s);
case Ctrl_U: {
int j = ccline.cmdpos;
ccline.cmdlen -= j;
int i = ccline.cmdpos = 0;
while (i < ccline.cmdlen) {
ccline.cmdbuff[i++] = ccline.cmdbuff[j++];
}
ccline.cmdbuff[ccline.cmdlen] = NUL;
if (ccline.cmdlen == 0) {
s->search_start = s->save_cursor;
}
redrawcmd();
return command_line_changed(s);
}
case ESC: 
case Ctrl_C:
if ((exmode_active && (ex_normal_busy == 0 || typebuf.tb_len > 0))
|| (getln_interrupted_highlight && s->c == Ctrl_C)) {
getln_interrupted_highlight = false;
return command_line_not_changed(s);
}
s->gotesc = true; 
return 0; 
case Ctrl_R: { 
putcmdline('"', true);
no_mapping++;
int i = s->c = plain_vgetc(); 
if (i == Ctrl_O) {
i = Ctrl_R; 
}
if (i == Ctrl_R) {
s->c = plain_vgetc(); 
}
--no_mapping;
new_cmdpos = -1;
if (s->c == '=') {
if (ccline.cmdfirstc == '=' 
|| cmdline_star > 0) { 
beep_flush();
s->c = ESC;
} else {
CmdlineInfo save_ccline;
save_cmdline(&save_ccline);
s->c = get_expr_register();
restore_cmdline(&save_ccline);
}
}
if (s->c != ESC) { 
cmdline_paste(s->c, i == Ctrl_R, false);
if (aborting()) {
s->gotesc = true; 
return 0; 
}
KeyTyped = false; 
if (new_cmdpos >= 0) {
if (new_cmdpos > ccline.cmdlen) {
ccline.cmdpos = ccline.cmdlen;
} else {
ccline.cmdpos = new_cmdpos;
}
}
}
ccline.special_char = NUL;
redrawcmd();
return command_line_changed(s);
}
case Ctrl_D:
if (showmatches(&s->xpc, false) == EXPAND_NOTHING) {
break; 
}
wild_menu_showing = WM_LIST;
redrawcmd();
return 1; 
case K_RIGHT:
case K_S_RIGHT:
case K_C_RIGHT:
do {
if (ccline.cmdpos >= ccline.cmdlen) {
break;
}
int cells = cmdline_charsize(ccline.cmdpos);
if (KeyTyped && ccline.cmdspos + cells >= Columns * Rows) {
break;
}
ccline.cmdspos += cells;
ccline.cmdpos += utfc_ptr2len(ccline.cmdbuff + ccline.cmdpos);
} while ((s->c == K_S_RIGHT || s->c == K_C_RIGHT
|| (mod_mask & (MOD_MASK_SHIFT|MOD_MASK_CTRL)))
&& ccline.cmdbuff[ccline.cmdpos] != ' ');
ccline.cmdspos = cmd_screencol(ccline.cmdpos);
return command_line_not_changed(s);
case K_LEFT:
case K_S_LEFT:
case K_C_LEFT:
if (ccline.cmdpos == 0) {
return command_line_not_changed(s);
}
do {
ccline.cmdpos--;
ccline.cmdpos -= utf_head_off(ccline.cmdbuff,
ccline.cmdbuff + ccline.cmdpos);
ccline.cmdspos -= cmdline_charsize(ccline.cmdpos);
} while (ccline.cmdpos > 0
&& (s->c == K_S_LEFT || s->c == K_C_LEFT
|| (mod_mask & (MOD_MASK_SHIFT|MOD_MASK_CTRL)))
&& ccline.cmdbuff[ccline.cmdpos - 1] != ' ');
ccline.cmdspos = cmd_screencol(ccline.cmdpos);
if (ccline.special_char != NUL) {
putcmdline(ccline.special_char, ccline.special_shift);
}
return command_line_not_changed(s);
case K_IGNORE:
return command_line_not_changed(s);
case K_MIDDLEDRAG:
case K_MIDDLERELEASE:
return command_line_not_changed(s); 
case K_MIDDLEMOUSE:
if (!mouse_has(MOUSE_COMMAND)) {
return command_line_not_changed(s); 
}
cmdline_paste(eval_has_provider("clipboard") ? '*' : 0, true, true);
redrawcmd();
return command_line_changed(s);
case K_LEFTDRAG:
case K_LEFTRELEASE:
case K_RIGHTDRAG:
case K_RIGHTRELEASE:
if (s->ignore_drag_release) {
return command_line_not_changed(s);
}
FALLTHROUGH;
case K_LEFTMOUSE:
case K_RIGHTMOUSE:
if (s->c == K_LEFTRELEASE || s->c == K_RIGHTRELEASE) {
s->ignore_drag_release = true;
} else {
s->ignore_drag_release = false;
}
if (!mouse_has(MOUSE_COMMAND)) {
return command_line_not_changed(s); 
}
ccline.cmdspos = cmd_startcol();
for (ccline.cmdpos = 0; ccline.cmdpos < ccline.cmdlen;
ccline.cmdpos++) {
int cells = cmdline_charsize(ccline.cmdpos);
if (mouse_row <= cmdline_row + ccline.cmdspos / Columns
&& mouse_col < ccline.cmdspos % Columns + cells) {
break;
}
correct_screencol(ccline.cmdpos, cells, &ccline.cmdspos);
ccline.cmdpos += utfc_ptr2len(ccline.cmdbuff + ccline.cmdpos) - 1;
ccline.cmdspos += cells;
}
return command_line_not_changed(s);
case K_MOUSEDOWN:
case K_MOUSEUP:
case K_MOUSELEFT:
case K_MOUSERIGHT:
case K_X1MOUSE:
case K_X1DRAG:
case K_X1RELEASE:
case K_X2MOUSE:
case K_X2DRAG:
case K_X2RELEASE:
return command_line_not_changed(s);
case K_SELECT: 
return command_line_not_changed(s);
case Ctrl_B: 
case K_HOME:
case K_KHOME:
case K_S_HOME:
case K_C_HOME:
ccline.cmdpos = 0;
ccline.cmdspos = cmd_startcol();
return command_line_not_changed(s);
case Ctrl_E: 
case K_END:
case K_KEND:
case K_S_END:
case K_C_END:
ccline.cmdpos = ccline.cmdlen;
ccline.cmdspos = cmd_screencol(ccline.cmdpos);
return command_line_not_changed(s);
case Ctrl_A: 
if (nextwild(&s->xpc, WILD_ALL, 0, s->firstc != '@') == FAIL)
break;
return command_line_changed(s);
case Ctrl_L:
if (p_is && !cmd_silent && (s->firstc == '/' || s->firstc == '?')) {
if (s->did_incsearch) {
curwin->w_cursor = s->match_end;
if (!equalpos(curwin->w_cursor, s->search_start)) {
s->c = gchar_cursor();
if (p_ic && p_scs
&& !pat_has_uppercase(ccline.cmdbuff)) {
s->c = mb_tolower(s->c);
}
if (s->c != NUL) {
if (s->c == s->firstc
|| vim_strchr((char_u *)(p_magic ? "\\~^$.*[" : "\\^$"), s->c)
!= NULL) {
stuffcharReadbuff(s->c);
s->c = '\\';
}
break;
}
}
}
return command_line_not_changed(s);
}
if (nextwild(&s->xpc, WILD_LONGEST, 0, s->firstc != '@') == FAIL) {
break;
}
return command_line_changed(s);
case Ctrl_N: 
case Ctrl_P: 
if (s->xpc.xp_numfiles > 0) {
if (nextwild(&s->xpc, (s->c == Ctrl_P) ? WILD_PREV : WILD_NEXT,
0, s->firstc != '@') == FAIL) {
break;
}
return command_line_not_changed(s);
}
FALLTHROUGH;
case K_UP:
case K_DOWN:
case K_S_UP:
case K_S_DOWN:
case K_PAGEUP:
case K_KPAGEUP:
case K_PAGEDOWN:
case K_KPAGEDOWN:
if (s->histype == HIST_INVALID || hislen == 0 || s->firstc == NUL) {
return command_line_not_changed(s);
}
s->save_hiscnt = s->hiscnt;
if (s->lookfor == NULL) {
s->lookfor = vim_strsave(ccline.cmdbuff);
s->lookfor[ccline.cmdpos] = NUL;
}
bool next_match = (s->c == K_DOWN || s->c == K_S_DOWN || s->c == Ctrl_N
|| s->c == K_PAGEDOWN || s->c == K_KPAGEDOWN);
command_line_next_histidx(s, next_match);
if (s->hiscnt != s->save_hiscnt) {
char_u *p;
int len = 0;
int old_firstc;
xfree(ccline.cmdbuff);
s->xpc.xp_context = EXPAND_NOTHING;
if (s->hiscnt == hislen) {
p = s->lookfor; 
} else {
p = history[s->histype][s->hiscnt].hisstr;
}
if (s->histype == HIST_SEARCH
&& p != s->lookfor
&& (old_firstc = p[STRLEN(p) + 1]) != s->firstc) {
for (int i = 0; i <= 1; i++) {
len = 0;
for (int j = 0; p[j] != NUL; j++) {
if (p[j] == old_firstc
&& (j == 0 || p[j - 1] != '\\')) {
if (i > 0) {
ccline.cmdbuff[len] = (char_u)s->firstc;
}
} else {
if (p[j] == s->firstc
&& (j == 0 || p[j - 1] != '\\')) {
if (i > 0) {
ccline.cmdbuff[len] = '\\';
}
++len;
}
if (i > 0) {
ccline.cmdbuff[len] = p[j];
}
}
++len;
}
if (i == 0) {
alloc_cmdbuff(len);
}
}
ccline.cmdbuff[len] = NUL;
} else {
alloc_cmdbuff((int)STRLEN(p));
STRCPY(ccline.cmdbuff, p);
}
ccline.cmdpos = ccline.cmdlen = (int)STRLEN(ccline.cmdbuff);
redrawcmd();
return command_line_changed(s);
}
beep_flush();
return command_line_not_changed(s);
case Ctrl_G: 
case Ctrl_T: 
if (p_is && !cmd_silent && (s->firstc == '/' || s->firstc == '?')) {
if (ccline.cmdlen != 0) {
command_line_next_incsearch(s, s->c == Ctrl_G);
}
return command_line_not_changed(s);
}
break;
case Ctrl_V:
case Ctrl_Q:
s->ignore_drag_release = true;
putcmdline('^', true);
s->c = get_literal(); 
s->do_abbr = false; 
ccline.special_char = NUL;
if (enc_utf8 && utf_iscomposing(s->c) && !cmd_silent) {
if (ui_has(kUICmdline)) {
unputcmdline();
} else {
draw_cmdline(ccline.cmdpos, ccline.cmdlen - ccline.cmdpos);
msg_putchar(' ');
cursorcmd();
}
}
break;
case Ctrl_K:
s->ignore_drag_release = true;
putcmdline('?', true);
s->c = get_digraph(true);
ccline.special_char = NUL;
if (s->c != NUL) {
break;
}
redrawcmd();
return command_line_not_changed(s);
case Ctrl__: 
if (!p_ari) {
break;
}
cmd_hkmap = !cmd_hkmap;
return command_line_not_changed(s);
default:
if (!IS_SPECIAL(s->c)) {
mod_mask = 0x0;
}
break;
}
if (s->do_abbr && (IS_SPECIAL(s->c) || !vim_iswordc(s->c))
&& (ccheck_abbr((has_mbyte && s->c >= 0x100) ?
(s->c + ABBR_OFF) : s->c)
|| s->c == Ctrl_RSB)) {
return command_line_changed(s);
}
if (IS_SPECIAL(s->c) || mod_mask != 0) {
put_on_cmdline(get_special_key_name(s->c, mod_mask), -1, true);
} else {
int j = utf_char2bytes(s->c, IObuff);
IObuff[j] = NUL; 
put_on_cmdline(IObuff, j, true);
}
return command_line_changed(s);
}
static int command_line_not_changed(CommandLineState *s)
{
if (!s->incsearch_postponed) {
return 1;
}
return command_line_changed(s);
}
static int empty_pattern(char_u *p)
{
size_t n = STRLEN(p);
while (n >= 2 && p[n - 2] == '\\'
&& vim_strchr((char_u *)"mMvVcCZ", p[n - 1]) != NULL) {
n -= 2;
}
return n == 0 || (n >= 2 && p[n - 2] == '\\' && p[n - 1] == '|');
}
static int command_line_changed(CommandLineState *s)
{
if (has_event(EVENT_CMDLINECHANGED)) {
TryState tstate;
Error err = ERROR_INIT;
dict_T *dict = get_vim_var_dict(VV_EVENT);
char firstcbuf[2];
firstcbuf[0] = (char)(s->firstc > 0 ? s->firstc : '-');
firstcbuf[1] = 0;
tv_dict_add_str(dict, S_LEN("cmdtype"), firstcbuf);
tv_dict_add_nr(dict, S_LEN("cmdlevel"), ccline.level);
tv_dict_set_keys_readonly(dict);
try_enter(&tstate);
apply_autocmds(EVENT_CMDLINECHANGED, (char_u *)firstcbuf,
(char_u *)firstcbuf, false, curbuf);
tv_dict_clear(dict);
bool tl_ret = try_leave(&tstate, &err);
if (!tl_ret && ERROR_SET(&err)) {
msg_putchar('\n');
msg_printf_attr(HL_ATTR(HLF_E)|MSG_HIST, (char *)e_autocmd_err, err.msg);
api_clear_error(&err);
redrawcmd();
}
}
if (p_is && !cmd_silent && (s->firstc == '/' || s->firstc == '?')) {
pos_T end_pos;
proftime_T tm;
searchit_arg_T sia;
if (char_avail()) {
s->incsearch_postponed = true;
return 1;
}
s->incsearch_postponed = false;
curwin->w_cursor = s->search_start; 
save_last_search_pattern();
int i;
if (ccline.cmdlen == 0) {
i = 0;
set_no_hlsearch(true); 
redraw_all_later(SOME_VALID);
} else {
int search_flags = SEARCH_OPT + SEARCH_NOOF + SEARCH_PEEK;
ui_busy_start();
ui_flush();
++emsg_off; 
tm = profile_setlimit(500L);
if (!p_hls) {
search_flags += SEARCH_KEEP;
}
memset(&sia, 0, sizeof(sia));
sia.sa_tm = &tm;
i = do_search(NULL, s->firstc, ccline.cmdbuff, s->count,
search_flags, &sia);
emsg_off--;
if (got_int) {
(void)vpeekc(); 
got_int = false; 
i = 0;
} else if (char_avail()) {
s->incsearch_postponed = true;
}
ui_busy_stop();
}
if (i != 0) {
highlight_match = true; 
} else {
highlight_match = false; 
}
restore_viewstate(&s->old_viewstate);
changed_cline_bef_curs();
update_topline();
if (i != 0) {
pos_T save_pos = curwin->w_cursor;
s->match_start = curwin->w_cursor;
set_search_match(&curwin->w_cursor);
validate_cursor();
end_pos = curwin->w_cursor;
s->match_end = end_pos;
curwin->w_cursor = save_pos;
} else {
end_pos = curwin->w_cursor; 
}
if (empty_pattern(ccline.cmdbuff)) {
set_no_hlsearch(true);
}
validate_cursor();
if (p_ru && curwin->w_status_height > 0) {
curwin->w_redr_status = true;
}
update_screen(SOME_VALID);
restore_last_search_pattern();
if (i != 0) {
curwin->w_cursor = end_pos;
}
msg_starthere();
redrawcmdline();
s->did_incsearch = true;
} else if (s->firstc == ':'
&& current_sctx.sc_sid == 0 
&& *p_icm != NUL 
&& curbuf->b_p_ma 
&& cmdline_star == 0 
&& cmd_can_preview(ccline.cmdbuff)
&& !vpeekc_any()) {
State |= CMDPREVIEW;
emsg_silent++; 
msg_silent++; 
do_cmdline(ccline.cmdbuff, NULL, NULL, DOCMD_KEEPLINE|DOCMD_NOWAIT);
msg_silent--; 
emsg_silent--; 
curwin->w_cursor = s->save_cursor;
restore_viewstate(&s->old_viewstate);
update_topline();
redrawcmdline();
} else if (State & CMDPREVIEW) {
State = (State & ~CMDPREVIEW);
update_screen(SOME_VALID); 
}
if (cmdmsg_rl || (p_arshape && !p_tbidi && enc_utf8)) {
if (!ui_has(kUICmdline) && vpeekc() == NUL) {
redrawcmd();
}
}
return 1;
}
static void abandon_cmdline(void)
{
XFREE_CLEAR(ccline.cmdbuff);
ccline.redraw_state = kCmdRedrawNone;
if (msg_scrolled == 0) {
compute_cmdrow();
}
MSG("");
redraw_cmdline = true;
}
char_u *
getcmdline (
int firstc,
long count, 
int indent, 
bool do_concat FUNC_ATTR_UNUSED
)
{
CmdlineInfo save_ccline;
save_cmdline(&save_ccline);
char_u *retval = command_line_enter(firstc, count, indent);
restore_cmdline(&save_ccline);
return retval;
}
char *getcmdline_prompt(const char firstc, const char *const prompt,
const int attr, const int xp_context,
const char *const xp_arg,
const Callback highlight_callback)
FUNC_ATTR_WARN_UNUSED_RESULT FUNC_ATTR_MALLOC
{
const int msg_col_save = msg_col;
CmdlineInfo save_ccline;
save_cmdline(&save_ccline);
ccline.prompt_id = last_prompt_id++;
ccline.cmdprompt = (char_u *)prompt;
ccline.cmdattr = attr;
ccline.xp_context = xp_context;
ccline.xp_arg = (char_u *)xp_arg;
ccline.input_fn = (firstc == '@');
ccline.highlight_callback = highlight_callback;
int msg_silent_saved = msg_silent;
msg_silent = 0;
char *const ret = (char *)command_line_enter(firstc, 1L, 0);
restore_cmdline(&save_ccline);
msg_silent = msg_silent_saved;
if (ccline.cmdbuff != NULL) {
msg_col = msg_col_save;
}
return ret;
}
int text_locked(void) {
if (cmdwin_type != 0)
return TRUE;
return textlock != 0;
}
void text_locked_msg(void)
{
EMSG(_(get_text_locked_msg()));
}
char_u * get_text_locked_msg(void) {
if (cmdwin_type != 0) {
return e_cmdwin;
} else {
return e_secure;
}
}
int curbuf_locked(void)
{
if (curbuf_lock > 0) {
EMSG(_("E788: Not allowed to edit another buffer now"));
return TRUE;
}
return allbuf_locked();
}
int allbuf_locked(void)
{
if (allbuf_lock > 0) {
EMSG(_("E811: Not allowed to change buffer information now"));
return TRUE;
}
return FALSE;
}
static int cmdline_charsize(int idx)
{
if (cmdline_star > 0) 
return 1;
return ptr2cells(ccline.cmdbuff + idx);
}
static int cmd_startcol(void)
{
return ccline.cmdindent + ((ccline.cmdfirstc != NUL) ? 1 : 0);
}
static int cmd_screencol(int bytepos)
{
int m; 
int col = cmd_startcol();
if (KeyTyped) {
m = Columns * Rows;
if (m < 0) 
m = MAXCOL;
} else {
m = MAXCOL;
}
for (int i = 0; i < ccline.cmdlen && i < bytepos;
i += utfc_ptr2len(ccline.cmdbuff + i)) {
int c = cmdline_charsize(i);
correct_screencol(i, c, &col);
if ((col += c) >= m) {
col -= c;
break;
}
}
return col;
}
static void correct_screencol(int idx, int cells, int *col)
{
if (utfc_ptr2len(ccline.cmdbuff + idx) > 1
&& utf_ptr2cells(ccline.cmdbuff + idx) > 1
&& (*col) % Columns + cells > Columns) {
(*col)++;
}
}
char_u *
getexline(
int c, 
void *cookie,
int indent, 
bool do_concat
)
{
if (exec_from_reg && vpeekc() == ':')
(void)vgetc();
return getcmdline(c, 1L, indent, do_concat);
}
char_u *
getexmodeline(
int promptc, 
void *cookie,
int indent, 
bool do_concat
)
{
garray_T line_ga;
char_u *pend;
int startcol = 0;
int c1 = 0;
int escaped = FALSE; 
int vcol = 0;
char_u *p;
int prev_char;
int len;
compute_cmdrow();
if ((msg_col || msg_didout) && promptc != '?')
msg_putchar('\n');
if (promptc == ':') {
if (p_prompt)
msg_putchar(':');
while (indent-- > 0)
msg_putchar(' ');
startcol = msg_col;
}
ga_init(&line_ga, 1, 30);
if (promptc <= 0) {
vcol = indent;
while (indent >= 8) {
ga_append(&line_ga, TAB);
msg_puts(" ");
indent -= 8;
}
while (indent-- > 0) {
ga_append(&line_ga, ' ');
msg_putchar(' ');
}
}
no_mapping++;
got_int = FALSE;
while (!got_int) {
ga_grow(&line_ga, 40);
prev_char = c1;
if (ex_normal_busy > 0 && typebuf.tb_len == 0) {
c1 = '\n';
} else {
c1 = vgetc();
}
if (got_int) {
msg_putchar('\n');
break;
}
if (!escaped) {
if (c1 == '\r')
c1 = '\n';
if (c1 == BS || c1 == K_BS || c1 == DEL || c1 == K_DEL || c1 == K_KDEL) {
if (!GA_EMPTY(&line_ga)) {
p = (char_u *)line_ga.ga_data;
p[line_ga.ga_len] = NUL;
len = utf_head_off(p, p + line_ga.ga_len - 1) + 1;
line_ga.ga_len -= len;
goto redraw;
}
continue;
}
if (c1 == Ctrl_U) {
msg_col = startcol;
msg_clr_eos();
line_ga.ga_len = 0;
goto redraw;
}
int num_spaces;
if (c1 == Ctrl_T) {
int sw = get_sw_value(curbuf);
p = (char_u *)line_ga.ga_data;
p[line_ga.ga_len] = NUL;
indent = get_indent_str(p, 8, FALSE);
num_spaces = sw - indent % sw;
add_indent:
if (num_spaces > 0) {
ga_grow(&line_ga, num_spaces + 1);
p = (char_u *)line_ga.ga_data;
char_u *s = skipwhite(p);
long move_len = line_ga.ga_len - (s - p) + 1;
assert(move_len >= 0);
memmove(s + num_spaces, s, (size_t)move_len);
memset(s, ' ', (size_t)num_spaces);
line_ga.ga_len += num_spaces;
}
redraw:
msg_col = startcol;
vcol = 0;
p = (char_u *)line_ga.ga_data;
p[line_ga.ga_len] = NUL;
while (p < (char_u *)line_ga.ga_data + line_ga.ga_len) {
if (*p == TAB) {
do {
msg_putchar(' ');
} while (++vcol % 8);
p++;
} else {
len = utfc_ptr2len(p);
msg_outtrans_len(p, len);
vcol += ptr2cells(p);
p += len;
}
}
msg_clr_eos();
cmd_cursor_goto(msg_row, msg_col);
continue;
}
if (c1 == Ctrl_D) {
p = (char_u *)line_ga.ga_data;
if (prev_char == '0' || prev_char == '^') {
if (prev_char == '^')
ex_keep_indent = TRUE;
indent = 0;
p[--line_ga.ga_len] = NUL;
} else {
p[line_ga.ga_len] = NUL;
indent = get_indent_str(p, 8, FALSE);
if (indent == 0) {
continue;
}
--indent;
indent -= indent % get_sw_value(curbuf);
}
char_u *from = skipwhite(p);
char_u *to = from;
int old_indent;
while ((old_indent = get_indent_str(p, 8, FALSE)) > indent) {
*--to = NUL;
}
long move_len = line_ga.ga_len - (from - p) + 1;
assert(move_len > 0);
memmove(to, from, (size_t)move_len);
line_ga.ga_len -= (int)(from - to);
num_spaces = indent - old_indent;
goto add_indent;
}
if (c1 == Ctrl_V || c1 == Ctrl_Q) {
escaped = TRUE;
continue;
}
if (IS_SPECIAL(c1)) {
continue;
}
}
if (IS_SPECIAL(c1)) {
c1 = '?';
}
len = utf_char2bytes(c1, (char_u *)line_ga.ga_data + line_ga.ga_len);
if (c1 == '\n') {
msg_putchar('\n');
} else if (c1 == TAB) {
do {
msg_putchar(' ');
} while (++vcol % 8);
} else {
msg_outtrans_len(((char_u *)line_ga.ga_data) + line_ga.ga_len, len);
vcol += char2cells(c1);
}
line_ga.ga_len += len;
escaped = FALSE;
cmd_cursor_goto(msg_row, msg_col);
pend = (char_u *)(line_ga.ga_data) + line_ga.ga_len;
if (!GA_EMPTY(&line_ga) && pend[-1] == '\n') {
int bcount = 0;
while (line_ga.ga_len - 2 >= bcount && pend[-2 - bcount] == '\\')
++bcount;
if (bcount > 0) {
line_ga.ga_len -= (bcount + 1) / 2;
pend -= (bcount + 1) / 2;
pend[-1] = '\n';
}
if ((bcount & 1) == 0) {
--line_ga.ga_len;
--pend;
*pend = NUL;
break;
}
}
}
no_mapping--;
msg_didout = FALSE;
msg_col = 0;
if (msg_row < Rows - 1)
++msg_row;
emsg_on_display = FALSE; 
if (got_int)
ga_clear(&line_ga);
return (char_u *)line_ga.ga_data;
}
bool cmdline_overstrike(void)
{
return ccline.overstrike;
}
bool cmdline_at_end(void)
{
return (ccline.cmdpos >= ccline.cmdlen);
}
static void alloc_cmdbuff(int len)
{
if (len < 80)
len = 100;
else
len += 20;
ccline.cmdbuff = xmalloc((size_t)len);
ccline.cmdbufflen = len;
}
static void realloc_cmdbuff(int len)
{
if (len < ccline.cmdbufflen) {
return; 
}
char_u *p = ccline.cmdbuff;
alloc_cmdbuff(len); 
memmove(ccline.cmdbuff, p, (size_t)ccline.cmdlen);
ccline.cmdbuff[ccline.cmdlen] = NUL;
xfree(p);
if (ccline.xpc != NULL
&& ccline.xpc->xp_pattern != NULL
&& ccline.xpc->xp_context != EXPAND_NOTHING
&& ccline.xpc->xp_context != EXPAND_UNSUCCESSFUL) {
int i = (int)(ccline.xpc->xp_pattern - p);
if (i >= 0 && i <= ccline.cmdlen)
ccline.xpc->xp_pattern = ccline.cmdbuff + i;
}
}
static char_u *arshape_buf = NULL;
#if defined(EXITFREE)
void free_arshape_buf(void)
{
xfree(arshape_buf);
}
#endif
enum { MAX_CB_ERRORS = 1 };
static void color_expr_cmdline(const CmdlineInfo *const colored_ccline,
ColoredCmdline *const ret_ccline_colors)
FUNC_ATTR_NONNULL_ALL
{
ParserLine plines[] = {
{
.data = (const char *)colored_ccline->cmdbuff,
.size = STRLEN(colored_ccline->cmdbuff),
.allocated = false,
},
{ NULL, 0, false },
};
ParserLine *plines_p = plines;
ParserHighlight colors;
kvi_init(colors);
ParserState pstate;
viml_parser_init(
&pstate, parser_simple_get_line, &plines_p, &colors);
ExprAST east = viml_pexpr_parse(&pstate, kExprFlagsDisallowEOC);
viml_pexpr_free_ast(east);
viml_parser_destroy(&pstate);
kv_resize(ret_ccline_colors->colors, kv_size(colors));
size_t prev_end = 0;
for (size_t i = 0 ; i < kv_size(colors) ; i++) {
const ParserHighlightChunk chunk = kv_A(colors, i);
assert(chunk.start.col < INT_MAX);
assert(chunk.end_col < INT_MAX);
if (chunk.start.col != prev_end) {
kv_push(ret_ccline_colors->colors, ((CmdlineColorChunk) {
.start = (int)prev_end,
.end = (int)chunk.start.col,
.attr = 0,
}));
}
const int id = syn_name2id((const char_u *)chunk.group);
const int attr = (id == 0 ? 0 : syn_id2attr(id));
kv_push(ret_ccline_colors->colors, ((CmdlineColorChunk) {
.start = (int)chunk.start.col,
.end = (int)chunk.end_col,
.attr = attr,
}));
prev_end = chunk.end_col;
}
if (prev_end < (size_t)colored_ccline->cmdlen) {
kv_push(ret_ccline_colors->colors, ((CmdlineColorChunk) {
.start = (int)prev_end,
.end = colored_ccline->cmdlen,
.attr = 0,
}));
}
kvi_destroy(colors);
}
static bool color_cmdline(CmdlineInfo *colored_ccline)
FUNC_ATTR_NONNULL_ALL FUNC_ATTR_WARN_UNUSED_RESULT
{
bool printed_errmsg = false;
#define PRINT_ERRMSG(...) do { msg_putchar('\n'); msg_printf_attr(HL_ATTR(HLF_E)|MSG_HIST, __VA_ARGS__); printed_errmsg = true; } while (0)
bool ret = true;
ColoredCmdline *ccline_colors = &colored_ccline->last_colors;
if (ccline_colors->prompt_id == colored_ccline->prompt_id
&& ccline_colors->cmdbuff != NULL
&& STRCMP(ccline_colors->cmdbuff, colored_ccline->cmdbuff) == 0) {
return ret;
}
kv_size(ccline_colors->colors) = 0;
if (colored_ccline->cmdbuff == NULL || *colored_ccline->cmdbuff == NUL) {
XFREE_CLEAR(ccline_colors->cmdbuff);
return ret;
}
bool arg_allocated = false;
typval_T arg = {
.v_type = VAR_STRING,
.vval.v_string = colored_ccline->cmdbuff,
};
typval_T tv = { .v_type = VAR_UNKNOWN };
static unsigned prev_prompt_id = UINT_MAX;
static int prev_prompt_errors = 0;
Callback color_cb = CALLBACK_NONE;
bool can_free_cb = false;
TryState tstate;
Error err = ERROR_INIT;
const char *err_errmsg = (const char *)e_intern2;
bool dgc_ret = true;
bool tl_ret = true;
if (colored_ccline->prompt_id != prev_prompt_id) {
prev_prompt_errors = 0;
prev_prompt_id = colored_ccline->prompt_id;
} else if (prev_prompt_errors >= MAX_CB_ERRORS) {
goto color_cmdline_end;
}
if (colored_ccline->highlight_callback.type != kCallbackNone) {
assert(colored_ccline->input_fn);
color_cb = colored_ccline->highlight_callback;
} else if (colored_ccline->cmdfirstc == ':') {
try_enter(&tstate);
err_errmsg = N_(
"E5408: Unable to get g:Nvim_color_cmdline callback: %s");
dgc_ret = tv_dict_get_callback(&globvardict, S_LEN("Nvim_color_cmdline"),
&color_cb);
tl_ret = try_leave(&tstate, &err);
can_free_cb = true;
} else if (colored_ccline->cmdfirstc == '=') {
color_expr_cmdline(colored_ccline, ccline_colors);
}
if (!tl_ret || !dgc_ret) {
goto color_cmdline_error;
}
if (color_cb.type == kCallbackNone) {
goto color_cmdline_end;
}
if (colored_ccline->cmdbuff[colored_ccline->cmdlen] != NUL) {
arg_allocated = true;
arg.vval.v_string = xmemdupz((const char *)colored_ccline->cmdbuff,
(size_t)colored_ccline->cmdlen);
}
getln_interrupted_highlight = false;
try_enter(&tstate);
err_errmsg = N_("E5407: Callback has thrown an exception: %s");
const int saved_msg_col = msg_col;
msg_silent++;
const bool cbcall_ret = callback_call(&color_cb, 1, &arg, &tv);
msg_silent--;
msg_col = saved_msg_col;
if (got_int) {
getln_interrupted_highlight = true;
}
if (!try_leave(&tstate, &err) || !cbcall_ret) {
goto color_cmdline_error;
}
if (tv.v_type != VAR_LIST) {
PRINT_ERRMSG("%s", _("E5400: Callback should return list"));
goto color_cmdline_error;
}
if (tv.vval.v_list == NULL) {
goto color_cmdline_end;
}
varnumber_T prev_end = 0;
int i = 0;
TV_LIST_ITER_CONST(tv.vval.v_list, li, {
if (TV_LIST_ITEM_TV(li)->v_type != VAR_LIST) {
PRINT_ERRMSG(_("E5401: List item %i is not a List"), i);
goto color_cmdline_error;
}
const list_T *const l = TV_LIST_ITEM_TV(li)->vval.v_list;
if (tv_list_len(l) != 3) {
PRINT_ERRMSG(_("E5402: List item %i has incorrect length: %d /= 3"),
i, tv_list_len(l));
goto color_cmdline_error;
}
bool error = false;
const varnumber_T start = (
tv_get_number_chk(TV_LIST_ITEM_TV(tv_list_first(l)), &error));
if (error) {
goto color_cmdline_error;
} else if (!(prev_end <= start && start < colored_ccline->cmdlen)) {
PRINT_ERRMSG(_("E5403: Chunk %i start %" PRIdVARNUMBER " not in range "
"[%" PRIdVARNUMBER ", %i)"),
i, start, prev_end, colored_ccline->cmdlen);
goto color_cmdline_error;
} else if (utf8len_tab_zero[(uint8_t)colored_ccline->cmdbuff[start]] == 0) {
PRINT_ERRMSG(_("E5405: Chunk %i start %" PRIdVARNUMBER " splits "
"multibyte character"), i, start);
goto color_cmdline_error;
}
if (start != prev_end) {
kv_push(ccline_colors->colors, ((CmdlineColorChunk) {
.start = (int)prev_end,
.end = (int)start,
.attr = 0,
}));
}
const varnumber_T end = tv_get_number_chk(
TV_LIST_ITEM_TV(TV_LIST_ITEM_NEXT(l, tv_list_first(l))), &error);
if (error) {
goto color_cmdline_error;
} else if (!(start < end && end <= colored_ccline->cmdlen)) {
PRINT_ERRMSG(_("E5404: Chunk %i end %" PRIdVARNUMBER " not in range "
"(%" PRIdVARNUMBER ", %i]"),
i, end, start, colored_ccline->cmdlen);
goto color_cmdline_error;
} else if (end < colored_ccline->cmdlen
&& (utf8len_tab_zero[(uint8_t)colored_ccline->cmdbuff[end]]
== 0)) {
PRINT_ERRMSG(_("E5406: Chunk %i end %" PRIdVARNUMBER " splits multibyte "
"character"), i, end);
goto color_cmdline_error;
}
prev_end = end;
const char *const group = tv_get_string_chk(
TV_LIST_ITEM_TV(tv_list_last(l)));
if (group == NULL) {
goto color_cmdline_error;
}
const int id = syn_name2id((char_u *)group);
const int attr = (id == 0 ? 0 : syn_id2attr(id));
kv_push(ccline_colors->colors, ((CmdlineColorChunk) {
.start = (int)start,
.end = (int)end,
.attr = attr,
}));
i++;
});
if (prev_end < colored_ccline->cmdlen) {
kv_push(ccline_colors->colors, ((CmdlineColorChunk) {
.start = (int)prev_end,
.end = colored_ccline->cmdlen,
.attr = 0,
}));
}
prev_prompt_errors = 0;
color_cmdline_end:
assert(!ERROR_SET(&err));
if (can_free_cb) {
callback_free(&color_cb);
}
xfree(ccline_colors->cmdbuff);
ccline_colors->prompt_id = colored_ccline->prompt_id;
if (arg_allocated) {
ccline_colors->cmdbuff = (char *)arg.vval.v_string;
} else {
ccline_colors->cmdbuff = xmemdupz((const char *)colored_ccline->cmdbuff,
(size_t)colored_ccline->cmdlen);
}
tv_clear(&tv);
return ret;
color_cmdline_error:
if (ERROR_SET(&err)) {
PRINT_ERRMSG(_(err_errmsg), err.msg);
api_clear_error(&err);
}
assert(printed_errmsg);
(void)printed_errmsg;
prev_prompt_errors++;
kv_size(ccline_colors->colors) = 0;
redrawcmdline();
ret = false;
goto color_cmdline_end;
#undef PRINT_ERRMSG
}
static void draw_cmdline(int start, int len)
{
if (!color_cmdline(&ccline)) {
return;
}
if (ui_has(kUICmdline)) {
ccline.special_char = NUL;
ccline.redraw_state = kCmdRedrawAll;
return;
}
if (cmdline_star > 0) {
for (int i = 0; i < len; i++) {
msg_putchar('*');
if (has_mbyte) {
i += (*mb_ptr2len)(ccline.cmdbuff + start + i) - 1;
}
}
} else if (p_arshape && !p_tbidi && enc_utf8 && len > 0) {
bool do_arabicshape = false;
int mb_l;
for (int i = start; i < start + len; i += mb_l) {
char_u *p = ccline.cmdbuff + i;
int u8cc[MAX_MCO];
int u8c = utfc_ptr2char_len(p, u8cc, start + len - i);
mb_l = utfc_ptr2len_len(p, start + len - i);
if (arabic_char(u8c)) {
do_arabicshape = true;
break;
}
}
if (!do_arabicshape) {
goto draw_cmdline_no_arabicshape;
}
static size_t buflen = 0;
assert(len >= 0);
if ((size_t)len * 2 + 2 > buflen) {
xfree(arshape_buf);
buflen = (size_t)len * 2 + 2;
arshape_buf = xmalloc(buflen);
}
int newlen = 0;
if (utf_iscomposing(utf_ptr2char(ccline.cmdbuff + start))) {
arshape_buf[0] = ' ';
newlen = 1;
}
int prev_c = 0;
int prev_c1 = 0;
for (int i = start; i < start + len; i += mb_l) {
char_u *p = ccline.cmdbuff + i;
int u8cc[MAX_MCO];
int u8c = utfc_ptr2char_len(p, u8cc, start + len - i);
mb_l = utfc_ptr2len_len(p, start + len - i);
if (arabic_char(u8c)) {
int pc;
int pc1 = 0;
int nc = 0;
if (cmdmsg_rl) {
pc = prev_c;
pc1 = prev_c1;
prev_c1 = u8cc[0];
if (i + mb_l >= start + len) {
nc = NUL;
} else {
nc = utf_ptr2char(p + mb_l);
}
} else {
if (i + mb_l >= start + len) {
pc = NUL;
} else {
int pcc[MAX_MCO];
pc = utfc_ptr2char_len(p + mb_l, pcc, start + len - i - mb_l);
pc1 = pcc[0];
}
nc = prev_c;
}
prev_c = u8c;
u8c = arabic_shape(u8c, NULL, &u8cc[0], pc, pc1, nc);
newlen += utf_char2bytes(u8c, arshape_buf + newlen);
if (u8cc[0] != 0) {
newlen += utf_char2bytes(u8cc[0], arshape_buf + newlen);
if (u8cc[1] != 0) {
newlen += utf_char2bytes(u8cc[1], arshape_buf + newlen);
}
}
} else {
prev_c = u8c;
memmove(arshape_buf + newlen, p, (size_t)mb_l);
newlen += mb_l;
}
}
msg_outtrans_len(arshape_buf, newlen);
} else {
draw_cmdline_no_arabicshape:
if (kv_size(ccline.last_colors.colors)) {
for (size_t i = 0; i < kv_size(ccline.last_colors.colors); i++) {
CmdlineColorChunk chunk = kv_A(ccline.last_colors.colors, i);
if (chunk.end <= start) {
continue;
}
const int chunk_start = MAX(chunk.start, start);
msg_outtrans_len_attr(ccline.cmdbuff + chunk_start,
chunk.end - chunk_start,
chunk.attr);
}
} else {
msg_outtrans_len(ccline.cmdbuff + start, len);
}
}
}
static void ui_ext_cmdline_show(CmdlineInfo *line)
{
Array content = ARRAY_DICT_INIT;
if (cmdline_star) {
size_t len = 0;
for (char_u *p = ccline.cmdbuff; *p; MB_PTR_ADV(p)) {
len++;
}
char *buf = xmallocz(len);
memset(buf, '*', len);
Array item = ARRAY_DICT_INIT;
ADD(item, INTEGER_OBJ(0));
ADD(item, STRING_OBJ(((String) { .data = buf, .size = len })));
ADD(content, ARRAY_OBJ(item));
} else if (kv_size(line->last_colors.colors)) {
for (size_t i = 0; i < kv_size(line->last_colors.colors); i++) {
CmdlineColorChunk chunk = kv_A(line->last_colors.colors, i);
Array item = ARRAY_DICT_INIT;
ADD(item, INTEGER_OBJ(chunk.attr));
assert(chunk.end >= chunk.start);
ADD(item, STRING_OBJ(cbuf_to_string((char *)line->cmdbuff + chunk.start,
(size_t)(chunk.end-chunk.start))));
ADD(content, ARRAY_OBJ(item));
}
} else {
Array item = ARRAY_DICT_INIT;
ADD(item, INTEGER_OBJ(0));
ADD(item, STRING_OBJ(cstr_to_string((char *)(line->cmdbuff))));
ADD(content, ARRAY_OBJ(item));
}
ui_call_cmdline_show(content, line->cmdpos,
cchar_to_string((char)line->cmdfirstc),
cstr_to_string((char *)(line->cmdprompt)),
line->cmdindent,
line->level);
if (line->special_char) {
ui_call_cmdline_special_char(cchar_to_string((char)(line->special_char)),
line->special_shift,
line->level);
}
}
void ui_ext_cmdline_block_append(size_t indent, const char *line)
{
char *buf = xmallocz(indent + strlen(line));
memset(buf, ' ', indent);
memcpy(buf + indent, line, strlen(line)); 
Array item = ARRAY_DICT_INIT;
ADD(item, INTEGER_OBJ(0));
ADD(item, STRING_OBJ(cstr_as_string(buf)));
Array content = ARRAY_DICT_INIT;
ADD(content, ARRAY_OBJ(item));
ADD(cmdline_block, ARRAY_OBJ(content));
if (cmdline_block.size > 1) {
ui_call_cmdline_block_append(copy_array(content));
} else {
ui_call_cmdline_block_show(copy_array(cmdline_block));
}
}
void ui_ext_cmdline_block_leave(void)
{
api_free_array(cmdline_block);
cmdline_block = (Array)ARRAY_DICT_INIT;
ui_call_cmdline_block_hide();
}
void cmdline_screen_cleared(void)
{
if (!ui_has(kUICmdline)) {
return;
}
if (cmdline_block.size) {
ui_call_cmdline_block_show(copy_array(cmdline_block));
}
int prev_level = ccline.level-1;
CmdlineInfo *line = ccline.prev_ccline;
while (prev_level > 0 && line) {
if (line->level == prev_level) {
if (prev_level != cmdwin_level) {
line->redraw_state = kCmdRedrawAll;
}
prev_level--;
}
line = line->prev_ccline;
}
}
void cmdline_ui_flush(void)
{
if (!ui_has(kUICmdline)) {
return;
}
int level = ccline.level;
CmdlineInfo *line = &ccline;
while (level > 0 && line) {
if (line->level == level) {
if (line->redraw_state == kCmdRedrawAll) {
ui_ext_cmdline_show(line);
} else if (line->redraw_state == kCmdRedrawPos) {
ui_call_cmdline_pos(line->cmdpos, line->level);
}
line->redraw_state = kCmdRedrawNone;
level--;
}
line = line->prev_ccline;
}
}
void putcmdline(char c, int shift)
{
if (cmd_silent) {
return;
}
if (!ui_has(kUICmdline)) {
msg_no_more = true;
msg_putchar(c);
if (shift) {
draw_cmdline(ccline.cmdpos, ccline.cmdlen - ccline.cmdpos);
}
msg_no_more = false;
} else if (ccline.redraw_state != kCmdRedrawAll) {
ui_call_cmdline_special_char(cchar_to_string((char)(c)), shift,
ccline.level);
}
cursorcmd();
ccline.special_char = c;
ccline.special_shift = shift;
ui_cursor_shape();
}
void unputcmdline(void)
{
if (cmd_silent) {
return;
}
msg_no_more = true;
if (ccline.cmdlen == ccline.cmdpos && !ui_has(kUICmdline)) {
msg_putchar(' ');
} else {
draw_cmdline(ccline.cmdpos, mb_ptr2len(ccline.cmdbuff + ccline.cmdpos));
}
msg_no_more = false;
cursorcmd();
ccline.special_char = NUL;
ui_cursor_shape();
}
void put_on_cmdline(char_u *str, int len, int redraw)
{
int i;
int m;
int c;
if (len < 0)
len = (int)STRLEN(str);
realloc_cmdbuff(ccline.cmdlen + len + 1);
if (!ccline.overstrike) {
memmove(ccline.cmdbuff + ccline.cmdpos + len,
ccline.cmdbuff + ccline.cmdpos,
(size_t)(ccline.cmdlen - ccline.cmdpos));
ccline.cmdlen += len;
} else {
if (has_mbyte) {
m = 0;
for (i = 0; i < len; i += (*mb_ptr2len)(str + i))
++m;
for (i = ccline.cmdpos; i < ccline.cmdlen && m > 0;
i += (*mb_ptr2len)(ccline.cmdbuff + i))
--m;
if (i < ccline.cmdlen) {
memmove(ccline.cmdbuff + ccline.cmdpos + len,
ccline.cmdbuff + i, (size_t)(ccline.cmdlen - i));
ccline.cmdlen += ccline.cmdpos + len - i;
} else
ccline.cmdlen = ccline.cmdpos + len;
} else if (ccline.cmdpos + len > ccline.cmdlen)
ccline.cmdlen = ccline.cmdpos + len;
}
memmove(ccline.cmdbuff + ccline.cmdpos, str, (size_t)len);
ccline.cmdbuff[ccline.cmdlen] = NUL;
if (enc_utf8) {
i = 0;
c = utf_ptr2char(ccline.cmdbuff + ccline.cmdpos);
while (ccline.cmdpos > 0 && utf_iscomposing(c)) {
i = utf_head_off(ccline.cmdbuff, ccline.cmdbuff + ccline.cmdpos - 1) + 1;
ccline.cmdpos -= i;
len += i;
c = utf_ptr2char(ccline.cmdbuff + ccline.cmdpos);
}
if (i == 0 && ccline.cmdpos > 0 && arabic_maycombine(c)) {
i = utf_head_off(ccline.cmdbuff, ccline.cmdbuff + ccline.cmdpos - 1) + 1;
if (arabic_combine(utf_ptr2char(ccline.cmdbuff + ccline.cmdpos - i), c)) {
ccline.cmdpos -= i;
len += i;
} else
i = 0;
}
if (i != 0) {
i = ptr2cells(ccline.cmdbuff + ccline.cmdpos);
ccline.cmdspos -= i;
msg_col -= i;
if (msg_col < 0) {
msg_col += Columns;
--msg_row;
}
}
}
if (redraw && !cmd_silent) {
msg_no_more = TRUE;
i = cmdline_row;
cursorcmd();
draw_cmdline(ccline.cmdpos, ccline.cmdlen - ccline.cmdpos);
if (cmdline_row != i || ccline.overstrike)
msg_clr_eos();
msg_no_more = FALSE;
}
if (KeyTyped) {
m = Columns * Rows;
if (m < 0) { 
m = MAXCOL;
}
} else {
m = MAXCOL;
}
for (i = 0; i < len; i++) {
c = cmdline_charsize(ccline.cmdpos);
if (has_mbyte) {
correct_screencol(ccline.cmdpos, c, &ccline.cmdspos);
}
if (ccline.cmdspos + c < m) {
ccline.cmdspos += c;
}
if (has_mbyte) {
c = (*mb_ptr2len)(ccline.cmdbuff + ccline.cmdpos) - 1;
if (c > len - i - 1) {
c = len - i - 1;
}
ccline.cmdpos += c;
i += c;
}
ccline.cmdpos++;
}
if (redraw) {
msg_check();
}
}
static void save_cmdline(struct cmdline_info *ccp)
{
*ccp = ccline;
ccline.prev_ccline = ccp;
ccline.cmdbuff = NULL;
ccline.cmdprompt = NULL;
ccline.xpc = NULL;
ccline.special_char = NUL;
ccline.level = 0;
}
static void restore_cmdline(struct cmdline_info *ccp)
{
ccline = *ccp;
}
char_u *save_cmdline_alloc(void)
{
struct cmdline_info *p = xmalloc(sizeof(struct cmdline_info));
save_cmdline(p);
return (char_u *)p;
}
void restore_cmdline_alloc(char_u *p)
{
restore_cmdline((struct cmdline_info *)p);
xfree(p);
}
static bool cmdline_paste(int regname, bool literally, bool remcr)
{
char_u *arg;
char_u *p;
bool allocated;
struct cmdline_info save_ccline;
if (regname != Ctrl_F && regname != Ctrl_P && regname != Ctrl_W
&& regname != Ctrl_A && regname != Ctrl_L
&& !valid_yank_reg(regname, false)) {
return FAIL;
}
line_breakcheck();
if (got_int)
return FAIL;
save_cmdline(&save_ccline);
textlock++;
const bool i = get_spec_reg(regname, &arg, &allocated, true);
textlock--;
restore_cmdline(&save_ccline);
if (i) {
if (arg == NULL)
return FAIL;
p = arg;
if (p_is && regname == Ctrl_W) {
char_u *w;
int len;
for (w = ccline.cmdbuff + ccline.cmdpos; w > ccline.cmdbuff; ) {
len = utf_head_off(ccline.cmdbuff, w - 1) + 1;
if (!vim_iswordc(utf_ptr2char(w - len))) {
break;
}
w -= len;
}
len = (int)((ccline.cmdbuff + ccline.cmdpos) - w);
if (p_ic ? STRNICMP(w, arg, len) == 0 : STRNCMP(w, arg, len) == 0)
p += len;
}
cmdline_paste_str(p, literally);
if (allocated)
xfree(arg);
return OK;
}
return cmdline_paste_reg(regname, literally, remcr);
}
void cmdline_paste_str(char_u *s, int literally)
{
int c, cv;
if (literally)
put_on_cmdline(s, -1, TRUE);
else
while (*s != NUL) {
cv = *s;
if (cv == Ctrl_V && s[1]) {
s++;
}
if (has_mbyte) {
c = mb_cptr2char_adv((const char_u **)&s);
} else {
c = *s++;
}
if (cv == Ctrl_V || c == ESC || c == Ctrl_C
|| c == CAR || c == NL || c == Ctrl_L
|| (c == Ctrl_BSL && *s == Ctrl_N)) {
stuffcharReadbuff(Ctrl_V);
}
stuffcharReadbuff(c);
}
}
static void cmdline_del(int from)
{
assert(ccline.cmdpos <= ccline.cmdlen);
memmove(ccline.cmdbuff + from, ccline.cmdbuff + ccline.cmdpos,
(size_t)ccline.cmdlen - (size_t)ccline.cmdpos + 1);
ccline.cmdlen -= ccline.cmdpos - from;
ccline.cmdpos = from;
}
void redrawcmdline(void)
{
if (cmd_silent)
return;
need_wait_return = FALSE;
compute_cmdrow();
redrawcmd();
cursorcmd();
ui_cursor_shape();
}
static void redrawcmdprompt(void)
{
int i;
if (cmd_silent)
return;
if (ui_has(kUICmdline)) {
ccline.redraw_state = kCmdRedrawAll;
return;
}
if (ccline.cmdfirstc != NUL) {
msg_putchar(ccline.cmdfirstc);
}
if (ccline.cmdprompt != NULL) {
msg_puts_attr((const char *)ccline.cmdprompt, ccline.cmdattr);
ccline.cmdindent = msg_col + (msg_row - cmdline_row) * Columns;
if (ccline.cmdfirstc != NUL) {
ccline.cmdindent--;
}
} else {
for (i = ccline.cmdindent; i > 0; i--) {
msg_putchar(' ');
}
}
}
void redrawcmd(void)
{
if (cmd_silent)
return;
if (ui_has(kUICmdline)) {
draw_cmdline(0, ccline.cmdlen);
return;
}
if (ccline.cmdbuff == NULL) {
cmd_cursor_goto(cmdline_row, 0);
msg_clr_eos();
return;
}
redrawing_cmdline = true;
msg_start();
redrawcmdprompt();
msg_no_more = TRUE;
draw_cmdline(0, ccline.cmdlen);
msg_clr_eos();
msg_no_more = FALSE;
ccline.cmdspos = cmd_screencol(ccline.cmdpos);
if (ccline.special_char != NUL) {
putcmdline(ccline.special_char, ccline.special_shift);
}
msg_scroll = FALSE; 
skip_redraw = false;
redrawing_cmdline = false;
}
void compute_cmdrow(void)
{
if (exmode_active || msg_scrolled != 0) {
cmdline_row = Rows - 1;
} else {
win_T *wp = lastwin_nofloating();
cmdline_row = wp->w_winrow + wp->w_height
+ wp->w_status_height;
}
lines_left = cmdline_row;
}
static void cursorcmd(void)
{
if (cmd_silent)
return;
if (ui_has(kUICmdline)) {
if (ccline.redraw_state < kCmdRedrawPos) {
ccline.redraw_state = kCmdRedrawPos;
}
setcursor();
return;
}
if (cmdmsg_rl) {
msg_row = cmdline_row + (ccline.cmdspos / (Columns - 1));
msg_col = Columns - (ccline.cmdspos % (Columns - 1)) - 1;
if (msg_row <= 0) {
msg_row = Rows - 1;
}
} else {
msg_row = cmdline_row + (ccline.cmdspos / Columns);
msg_col = ccline.cmdspos % Columns;
if (msg_row >= Rows) {
msg_row = Rows - 1;
}
}
cmd_cursor_goto(msg_row, msg_col);
}
static void cmd_cursor_goto(int row, int col)
{
ScreenGrid *grid = &msg_grid_adj;
screen_adjust_grid(&grid, &row, &col);
ui_grid_cursor_goto(grid->handle, row, col);
}
void gotocmdline(int clr)
{
if (ui_has(kUICmdline)) {
return;
}
msg_start();
if (cmdmsg_rl) {
msg_col = Columns - 1;
} else {
msg_col = 0; 
}
if (clr) { 
msg_clr_eos(); 
}
cmd_cursor_goto(cmdline_row, 0);
}
static int ccheck_abbr(int c)
{
int spos = 0;
if (p_paste || no_abbr) { 
return false;
}
while (spos < ccline.cmdlen && ascii_iswhite(ccline.cmdbuff[spos])) {
spos++;
}
if (ccline.cmdlen - spos > 5
&& ccline.cmdbuff[spos] == '\''
&& ccline.cmdbuff[spos + 2] == ','
&& ccline.cmdbuff[spos + 3] == '\'') {
spos += 5;
} else {
spos = 0;
}
return check_abbr(c, ccline.cmdbuff, ccline.cmdpos, spos);
}
static int sort_func_compare(const void *s1, const void *s2)
{
char_u *p1 = *(char_u **)s1;
char_u *p2 = *(char_u **)s2;
if (*p1 != '<' && *p2 == '<') return -1;
if (*p1 == '<' && *p2 != '<') return 1;
return STRCMP(p1, p2);
}
static int 
nextwild (
expand_T *xp,
int type,
int options, 
int escape 
)
{
int i, j;
char_u *p1;
char_u *p2;
int difflen;
if (xp->xp_numfiles == -1) {
set_expand_context(xp);
cmd_showtail = expand_showtail(xp);
}
if (xp->xp_context == EXPAND_UNSUCCESSFUL) {
beep_flush();
return OK; 
}
if (xp->xp_context == EXPAND_NOTHING) {
return FAIL;
}
if (!(ui_has(kUICmdline) || ui_has(kUIWildmenu))) {
MSG_PUTS("..."); 
ui_flush();
}
i = (int)(xp->xp_pattern - ccline.cmdbuff);
assert(ccline.cmdpos >= i);
xp->xp_pattern_len = (size_t)ccline.cmdpos - (size_t)i;
if (type == WILD_NEXT || type == WILD_PREV) {
p2 = ExpandOne(xp, NULL, NULL, 0, type);
} else {
p1 = addstar(xp->xp_pattern, xp->xp_pattern_len, xp->xp_context);
const int use_options = (
options
| WILD_HOME_REPLACE
| WILD_ADD_SLASH
| WILD_SILENT
| (escape ? WILD_ESCAPE : 0)
| (p_wic ? WILD_ICASE : 0));
p2 = ExpandOne(xp, p1, vim_strnsave(&ccline.cmdbuff[i], xp->xp_pattern_len),
use_options, type);
xfree(p1);
if (p2 != NULL && type == WILD_LONGEST) {
for (j = 0; (size_t)j < xp->xp_pattern_len; j++) {
if (ccline.cmdbuff[i + j] == '*'
|| ccline.cmdbuff[i + j] == '?') {
break;
}
}
if ((int)STRLEN(p2) < j) {
XFREE_CLEAR(p2);
}
}
}
if (p2 != NULL && !got_int) {
difflen = (int)STRLEN(p2) - (int)xp->xp_pattern_len;
if (ccline.cmdlen + difflen + 4 > ccline.cmdbufflen) {
realloc_cmdbuff(ccline.cmdlen + difflen + 4);
xp->xp_pattern = ccline.cmdbuff + i;
}
assert(ccline.cmdpos <= ccline.cmdlen);
memmove(&ccline.cmdbuff[ccline.cmdpos + difflen],
&ccline.cmdbuff[ccline.cmdpos],
(size_t)ccline.cmdlen - (size_t)ccline.cmdpos + 1);
memmove(&ccline.cmdbuff[i], p2, STRLEN(p2));
ccline.cmdlen += difflen;
ccline.cmdpos += difflen;
}
xfree(p2);
redrawcmd();
cursorcmd();
if (xp->xp_context == EXPAND_MAPPINGS && p2 == NULL)
return FAIL;
if (xp->xp_numfiles <= 0 && p2 == NULL)
beep_flush();
else if (xp->xp_numfiles == 1)
(void)ExpandOne(xp, NULL, NULL, 0, WILD_FREE);
return OK;
}
char_u *
ExpandOne (
expand_T *xp,
char_u *str,
char_u *orig, 
int options,
int mode
)
{
char_u *ss = NULL;
static int findex;
static char_u *orig_save = NULL; 
int orig_saved = FALSE;
int i;
int non_suf_match; 
if (mode == WILD_NEXT || mode == WILD_PREV) {
if (xp->xp_numfiles > 0) {
if (mode == WILD_PREV) {
if (findex == -1)
findex = xp->xp_numfiles;
--findex;
} else 
++findex;
if (findex < 0) {
if (orig_save == NULL)
findex = xp->xp_numfiles - 1;
else
findex = -1;
}
if (findex >= xp->xp_numfiles) {
if (orig_save == NULL)
findex = 0;
else
findex = -1;
}
if (compl_match_array) {
compl_selected = findex;
cmdline_pum_display(false);
} else if (p_wmnu) {
win_redr_status_matches(xp, xp->xp_numfiles, xp->xp_files,
findex, cmd_showtail);
}
if (findex == -1) {
return vim_strsave(orig_save);
}
return vim_strsave(xp->xp_files[findex]);
} else
return NULL;
}
if (mode == WILD_CANCEL) {
ss = vim_strsave(orig_save);
} else if (mode == WILD_APPLY) {
ss = vim_strsave(findex == -1 ? orig_save : xp->xp_files[findex]);
}
if (xp->xp_numfiles != -1 && mode != WILD_ALL && mode != WILD_LONGEST) {
FreeWild(xp->xp_numfiles, xp->xp_files);
xp->xp_numfiles = -1;
XFREE_CLEAR(orig_save);
}
findex = 0;
if (mode == WILD_FREE) 
return NULL;
if (xp->xp_numfiles == -1 && mode != WILD_APPLY && mode != WILD_CANCEL) {
xfree(orig_save);
orig_save = orig;
orig_saved = TRUE;
if (ExpandFromContext(xp, str, &xp->xp_numfiles, &xp->xp_files,
options) == FAIL) {
#if defined(FNAME_ILLEGAL)
if (!(options & WILD_SILENT) && (options & WILD_LIST_NOTFOUND))
EMSG2(_(e_nomatch2), str);
#endif
} else if (xp->xp_numfiles == 0) {
if (!(options & WILD_SILENT))
EMSG2(_(e_nomatch2), str);
} else {
ExpandEscape(xp, str, xp->xp_numfiles, xp->xp_files, options);
if (mode != WILD_ALL && mode != WILD_ALL_KEEP
&& mode != WILD_LONGEST) {
if (xp->xp_numfiles)
non_suf_match = xp->xp_numfiles;
else
non_suf_match = 1;
if ((xp->xp_context == EXPAND_FILES
|| xp->xp_context == EXPAND_DIRECTORIES)
&& xp->xp_numfiles > 1) {
non_suf_match = 0;
for (i = 0; i < 2; ++i)
if (match_suffix(xp->xp_files[i]))
++non_suf_match;
}
if (non_suf_match != 1) {
if (!(options & WILD_SILENT))
EMSG(_(e_toomany));
else if (!(options & WILD_NO_BEEP))
beep_flush();
}
if (!(non_suf_match != 1 && mode == WILD_EXPAND_FREE))
ss = vim_strsave(xp->xp_files[0]);
}
}
}
if (mode == WILD_LONGEST && xp->xp_numfiles > 0) {
size_t len = 0;
for (size_t mb_len; xp->xp_files[0][len]; len += mb_len) {
mb_len = (size_t)utfc_ptr2len(&xp->xp_files[0][len]);
int c0 = utf_ptr2char(&xp->xp_files[0][len]);
for (i = 1; i < xp->xp_numfiles; i++) {
int ci = utf_ptr2char(&xp->xp_files[i][len]);
if (p_fic && (xp->xp_context == EXPAND_DIRECTORIES
|| xp->xp_context == EXPAND_FILES
|| xp->xp_context == EXPAND_SHELLCMD
|| xp->xp_context == EXPAND_BUFFERS)) {
if (mb_tolower(c0) != mb_tolower(ci)) {
break;
}
} else if (c0 != ci) {
break;
}
}
if (i < xp->xp_numfiles) {
if (!(options & WILD_NO_BEEP)) {
vim_beep(BO_WILD);
}
break;
}
}
ss = (char_u *)xstrndup((char *)xp->xp_files[0], len);
findex = -1; 
}
if (mode == WILD_ALL && xp->xp_numfiles > 0) {
size_t len = 0;
for (i = 0; i < xp->xp_numfiles; ++i)
len += STRLEN(xp->xp_files[i]) + 1;
ss = xmalloc(len);
*ss = NUL;
for (i = 0; i < xp->xp_numfiles; ++i) {
STRCAT(ss, xp->xp_files[i]);
if (i != xp->xp_numfiles - 1)
STRCAT(ss, (options & WILD_USE_NL) ? "\n" : " ");
}
}
if (mode == WILD_EXPAND_FREE || mode == WILD_ALL)
ExpandCleanup(xp);
if (!orig_saved)
xfree(orig);
return ss;
}
void ExpandInit(expand_T *xp)
{
xp->xp_pattern = NULL;
xp->xp_pattern_len = 0;
xp->xp_backslash = XP_BS_NONE;
#if !defined(BACKSLASH_IN_FILENAME)
xp->xp_shell = FALSE;
#endif
xp->xp_numfiles = -1;
xp->xp_files = NULL;
xp->xp_arg = NULL;
xp->xp_line = NULL;
}
void ExpandCleanup(expand_T *xp)
{
if (xp->xp_numfiles >= 0) {
FreeWild(xp->xp_numfiles, xp->xp_files);
xp->xp_numfiles = -1;
}
}
void ExpandEscape(expand_T *xp, char_u *str, int numfiles, char_u **files, int options)
{
int i;
char_u *p;
if (options & WILD_HOME_REPLACE)
tilde_replace(str, numfiles, files);
if (options & WILD_ESCAPE) {
if (xp->xp_context == EXPAND_FILES
|| xp->xp_context == EXPAND_FILES_IN_PATH
|| xp->xp_context == EXPAND_SHELLCMD
|| xp->xp_context == EXPAND_BUFFERS
|| xp->xp_context == EXPAND_DIRECTORIES) {
for (i = 0; i < numfiles; ++i) {
if (xp->xp_backslash == XP_BS_THREE) {
p = vim_strsave_escaped(files[i], (char_u *)" ");
xfree(files[i]);
files[i] = p;
#if defined(BACKSLASH_IN_FILENAME)
p = vim_strsave_escaped(files[i], (char_u *)" ");
xfree(files[i]);
files[i] = p;
#endif
}
#if defined(BACKSLASH_IN_FILENAME)
p = (char_u *)vim_strsave_fnameescape((const char *)files[i], false);
#else
p = (char_u *)vim_strsave_fnameescape((const char *)files[i],
xp->xp_shell);
#endif
xfree(files[i]);
files[i] = p;
if (str[0] == '\\' && str[1] == '~' && files[i][0] == '~')
escape_fname(&files[i]);
}
xp->xp_backslash = XP_BS_NONE;
if (*files[0] == '+')
escape_fname(&files[0]);
} else if (xp->xp_context == EXPAND_TAGS) {
for (i = 0; i < numfiles; ++i) {
p = vim_strsave_escaped(files[i], (char_u *)"\\|\"");
xfree(files[i]);
files[i] = p;
}
}
}
}
char *vim_strsave_fnameescape(const char *const fname, const bool shell)
FUNC_ATTR_NONNULL_RET FUNC_ATTR_MALLOC FUNC_ATTR_NONNULL_ALL
{
#if defined(BACKSLASH_IN_FILENAME)
#define PATH_ESC_CHARS " \t\n*?[{`%#'\"|!<"
char_u buf[sizeof(PATH_ESC_CHARS)];
int j = 0;
for (const char *s = PATH_ESC_CHARS; *s != NUL; s++) {
if ((*s != '[' && *s != '{' && *s != '!') || !vim_isfilec(*s)) {
buf[j++] = *s;
}
}
buf[j] = NUL;
char *p = (char *)vim_strsave_escaped((const char_u *)fname,
(const char_u *)buf);
#else
#define PATH_ESC_CHARS ((char_u *)" \t\n*?[{`$\\%#'\"|!<")
#define SHELL_ESC_CHARS ((char_u *)" \t\n*?[{`$\\%#'\"|!<>();&")
char *p = (char *)vim_strsave_escaped(
(const char_u *)fname, (shell ? SHELL_ESC_CHARS : PATH_ESC_CHARS));
if (shell && csh_like_shell()) {
char *s = (char *)vim_strsave_escaped((const char_u *)p,
(const char_u *)"!");
xfree(p);
p = s;
}
#endif
if (*p == '>' || *p == '+' || (*p == '-' && p[1] == NUL)) {
escape_fname((char_u **)&p);
}
return p;
}
static void escape_fname(char_u **pp)
{
char_u *p = xmalloc(STRLEN(*pp) + 2);
p[0] = '\\';
STRCPY(p + 1, *pp);
xfree(*pp);
*pp = p;
}
void tilde_replace(char_u *orig_pat, int num_files, char_u **files)
{
int i;
char_u *p;
if (orig_pat[0] == '~' && vim_ispathsep(orig_pat[1])) {
for (i = 0; i < num_files; ++i) {
p = home_replace_save(NULL, files[i]);
xfree(files[i]);
files[i] = p;
}
}
}
void cmdline_pum_display(bool changed_array)
{
pum_display(compl_match_array, compl_match_arraysize, compl_selected,
changed_array, compl_startcol);
}
static int showmatches(expand_T *xp, int wildmenu)
{
#define L_SHOWFILE(m) (showtail ? sm_gettail(files_found[m], false) : files_found[m])
int num_files;
char_u **files_found;
int i, j, k;
int maxlen;
int lines;
int columns;
char_u *p;
int lastlen;
int attr;
int showtail;
if (xp->xp_numfiles == -1) {
set_expand_context(xp);
i = expand_cmdline(xp, ccline.cmdbuff, ccline.cmdpos,
&num_files, &files_found);
showtail = expand_showtail(xp);
if (i != EXPAND_OK)
return i;
} else {
num_files = xp->xp_numfiles;
files_found = xp->xp_files;
showtail = cmd_showtail;
}
bool compl_use_pum = (ui_has(kUICmdline)
? ui_has(kUIPopupmenu)
: wildmenu && (wop_flags & WOP_PUM))
|| ui_has(kUIWildmenu);
if (compl_use_pum) {
assert(num_files >= 0);
compl_match_arraysize = num_files;
compl_match_array = xcalloc((size_t)compl_match_arraysize,
sizeof(pumitem_T));
for (i = 0; i < num_files; i++) {
compl_match_array[i].pum_text = L_SHOWFILE(i);
}
char_u *endpos = (showtail
? sm_gettail(xp->xp_pattern, true) : xp->xp_pattern);
if (ui_has(kUICmdline)) {
compl_startcol = (int)(endpos - ccline.cmdbuff);
} else {
compl_startcol = cmd_screencol((int)(endpos - ccline.cmdbuff));
}
compl_selected = -1;
cmdline_pum_display(true);
return EXPAND_OK;
}
if (!wildmenu) {
msg_didany = FALSE; 
msg_start(); 
msg_putchar('\n');
ui_flush();
cmdline_row = msg_row;
msg_didany = FALSE; 
msg_start(); 
}
if (got_int) {
got_int = false; 
} else if (wildmenu) {
win_redr_status_matches(xp, num_files, files_found, -1, showtail);
} else {
maxlen = 0;
for (i = 0; i < num_files; ++i) {
if (!showtail && (xp->xp_context == EXPAND_FILES
|| xp->xp_context == EXPAND_SHELLCMD
|| xp->xp_context == EXPAND_BUFFERS)) {
home_replace(NULL, files_found[i], NameBuff, MAXPATHL, TRUE);
j = vim_strsize(NameBuff);
} else
j = vim_strsize(L_SHOWFILE(i));
if (j > maxlen)
maxlen = j;
}
if (xp->xp_context == EXPAND_TAGS_LISTFILES) {
lines = num_files;
} else {
maxlen += 2; 
columns = (Columns + 2) / maxlen;
if (columns < 1) {
columns = 1;
}
lines = (num_files + columns - 1) / columns;
}
attr = HL_ATTR(HLF_D); 
if (xp->xp_context == EXPAND_TAGS_LISTFILES) {
MSG_PUTS_ATTR(_("tagname"), HL_ATTR(HLF_T));
msg_clr_eos();
msg_advance(maxlen - 3);
MSG_PUTS_ATTR(_(" kind file\n"), HL_ATTR(HLF_T));
}
for (i = 0; i < lines; ++i) {
lastlen = 999;
for (k = i; k < num_files; k += lines) {
if (xp->xp_context == EXPAND_TAGS_LISTFILES) {
msg_outtrans_attr(files_found[k], HL_ATTR(HLF_D));
p = files_found[k] + STRLEN(files_found[k]) + 1;
msg_advance(maxlen + 1);
msg_puts((const char *)p);
msg_advance(maxlen + 3);
msg_puts_long_attr(p + 2, HL_ATTR(HLF_D));
break;
}
for (j = maxlen - lastlen; --j >= 0; )
msg_putchar(' ');
if (xp->xp_context == EXPAND_FILES
|| xp->xp_context == EXPAND_SHELLCMD
|| xp->xp_context == EXPAND_BUFFERS) {
if (xp->xp_numfiles != -1) {
char_u *exp_path = expand_env_save_opt(files_found[k], true);
char_u *path = exp_path != NULL ? exp_path : files_found[k];
char_u *halved_slash = backslash_halve_save(path);
j = os_isdir(halved_slash);
xfree(exp_path);
if (halved_slash != path) {
xfree(halved_slash);
}
} else {
j = os_isdir(files_found[k]);
}
if (showtail) {
p = L_SHOWFILE(k);
} else {
home_replace(NULL, files_found[k], NameBuff, MAXPATHL,
TRUE);
p = NameBuff;
}
} else {
j = FALSE;
p = L_SHOWFILE(k);
}
lastlen = msg_outtrans_attr(p, j ? attr : 0);
}
if (msg_col > 0) { 
msg_clr_eos();
msg_putchar('\n');
}
ui_flush(); 
if (got_int) {
got_int = FALSE;
break;
}
}
cmdline_row = msg_row; 
}
if (xp->xp_numfiles == -1)
FreeWild(num_files, files_found);
return EXPAND_OK;
}
char_u *sm_gettail(char_u *s, bool eager)
{
char_u *p;
char_u *t = s;
int had_sep = FALSE;
for (p = s; *p != NUL; ) {
if (vim_ispathsep(*p)
#if defined(BACKSLASH_IN_FILENAME)
&& !rem_backslash(p)
#endif
) {
if (eager) {
t = p+1;
} else {
had_sep = true;
}
} else if (had_sep) {
t = p;
had_sep = FALSE;
}
MB_PTR_ADV(p);
}
return t;
}
static int expand_showtail(expand_T *xp)
{
char_u *s;
char_u *end;
if (xp->xp_context != EXPAND_FILES
&& xp->xp_context != EXPAND_SHELLCMD
&& xp->xp_context != EXPAND_DIRECTORIES)
return FALSE;
end = path_tail(xp->xp_pattern);
if (end == xp->xp_pattern) 
return FALSE;
for (s = xp->xp_pattern; s < end; s++) {
if (rem_backslash(s))
++s;
else if (vim_strchr((char_u *)"*?[", *s) != NULL)
return FALSE;
}
return TRUE;
}
char_u *addstar(char_u *fname, size_t len, int context)
FUNC_ATTR_NONNULL_RET
{
char_u *retval;
size_t i, j;
size_t new_len;
char_u *tail;
int ends_in_star;
if (context != EXPAND_FILES
&& context != EXPAND_FILES_IN_PATH
&& context != EXPAND_SHELLCMD
&& context != EXPAND_DIRECTORIES) {
if (context == EXPAND_HELP
|| context == EXPAND_CHECKHEALTH
|| context == EXPAND_COLORS
|| context == EXPAND_COMPILER
|| context == EXPAND_OWNSYNTAX
|| context == EXPAND_FILETYPE
|| context == EXPAND_PACKADD
|| ((context == EXPAND_TAGS_LISTFILES || context == EXPAND_TAGS)
&& fname[0] == '/')) {
retval = vim_strnsave(fname, len);
} else {
new_len = len + 2; 
for (i = 0; i < len; i++) {
if (fname[i] == '*' || fname[i] == '~')
new_len++; 
if (context == EXPAND_BUFFERS && fname[i] == '.')
new_len++; 
if ((context == EXPAND_USER_DEFINED
|| context == EXPAND_USER_LIST) && fname[i] == '\\')
new_len++; 
}
retval = xmalloc(new_len);
{
retval[0] = '^';
j = 1;
for (i = 0; i < len; i++, j++) {
if (context != EXPAND_USER_DEFINED
&& context != EXPAND_USER_LIST
&& fname[i] == '\\'
&& ++i == len)
break;
switch (fname[i]) {
case '*': retval[j++] = '.';
break;
case '~': retval[j++] = '\\';
break;
case '?': retval[j] = '.';
continue;
case '.': if (context == EXPAND_BUFFERS)
retval[j++] = '\\';
break;
case '\\': if (context == EXPAND_USER_DEFINED
|| context == EXPAND_USER_LIST)
retval[j++] = '\\';
break;
}
retval[j] = fname[i];
}
retval[j] = NUL;
}
}
} else {
retval = xmalloc(len + 4);
STRLCPY(retval, fname, len + 1);
tail = path_tail(retval);
ends_in_star = (len > 0 && retval[len - 1] == '*');
#if !defined(BACKSLASH_IN_FILENAME)
for (ssize_t k = (ssize_t)len - 2; k >= 0; k--) {
if (retval[k] != '\\') {
break;
}
ends_in_star = !ends_in_star;
}
#endif
if ((*retval != '~' || tail != retval)
&& !ends_in_star
&& vim_strchr(tail, '$') == NULL
&& vim_strchr(retval, '`') == NULL)
retval[len++] = '*';
else if (len > 0 && retval[len - 1] == '$')
--len;
retval[len] = NUL;
}
return retval;
}
static void set_expand_context(expand_T *xp)
{
if (ccline.cmdfirstc != ':'
&& ccline.cmdfirstc != '>' && ccline.cmdfirstc != '='
&& !ccline.input_fn
) {
xp->xp_context = EXPAND_NOTHING;
return;
}
set_cmd_context(xp, ccline.cmdbuff, ccline.cmdlen, ccline.cmdpos, true);
}
void 
set_cmd_context (
expand_T *xp,
char_u *str, 
int len, 
int col, 
int use_ccline 
)
{
char_u old_char = NUL;
if (col < len)
old_char = str[col];
str[col] = NUL;
const char *nextcomm = (const char *)str;
if (use_ccline && ccline.cmdfirstc == '=') {
set_context_for_expression(xp, str, CMD_SIZE);
} else if (use_ccline && ccline.input_fn) {
xp->xp_context = ccline.xp_context;
xp->xp_pattern = ccline.cmdbuff;
xp->xp_arg = ccline.xp_arg;
} else {
while (nextcomm != NULL) {
nextcomm = set_one_cmd_context(xp, nextcomm);
}
}
xp->xp_line = str;
xp->xp_col = col;
str[col] = old_char;
}
int 
expand_cmdline (
expand_T *xp,
char_u *str, 
int col, 
int *matchcount, 
char_u ***matches 
)
{
char_u *file_str = NULL;
int options = WILD_ADD_SLASH|WILD_SILENT;
if (xp->xp_context == EXPAND_UNSUCCESSFUL) {
beep_flush();
return EXPAND_UNSUCCESSFUL; 
}
if (xp->xp_context == EXPAND_NOTHING) {
return EXPAND_NOTHING;
}
assert((str + col) - xp->xp_pattern >= 0);
xp->xp_pattern_len = (size_t)((str + col) - xp->xp_pattern);
file_str = addstar(xp->xp_pattern, xp->xp_pattern_len, xp->xp_context);
if (p_wic)
options += WILD_ICASE;
if (ExpandFromContext(xp, file_str, matchcount, matches, options) == FAIL) {
*matchcount = 0;
*matches = NULL;
}
xfree(file_str);
return EXPAND_OK;
}
static void cleanup_help_tags(int num_file, char_u **file)
{
char_u buf[4];
char_u *p = buf;
if (p_hlg[0] != NUL && (p_hlg[0] != 'e' || p_hlg[1] != 'n')) {
*p++ = '@';
*p++ = p_hlg[0];
*p++ = p_hlg[1];
}
*p = NUL;
for (int i = 0; i < num_file; i++) {
int len = (int)STRLEN(file[i]) - 3;
if (len <= 0) {
continue;
}
if (STRCMP(file[i] + len, "@en") == 0) {
int j;
for (j = 0; j < num_file; j++) {
if (j != i
&& (int)STRLEN(file[j]) == len + 3
&& STRNCMP(file[i], file[j], len + 1) == 0) {
break;
}
}
if (j == num_file) {
file[i][len] = NUL;
}
}
}
if (*buf != NUL) {
for (int i = 0; i < num_file; i++) {
int len = (int)STRLEN(file[i]) - 3;
if (len <= 0) {
continue;
}
if (STRCMP(file[i] + len, buf) == 0) {
file[i][len] = NUL;
}
}
}
}
typedef char_u *(*ExpandFunc)(expand_T *, int);
static int 
ExpandFromContext (
expand_T *xp,
char_u *pat,
int *num_file,
char_u ***file,
int options 
)
{
regmatch_T regmatch;
int ret;
int flags;
flags = EW_DIR; 
if (options & WILD_LIST_NOTFOUND)
flags |= EW_NOTFOUND;
if (options & WILD_ADD_SLASH)
flags |= EW_ADDSLASH;
if (options & WILD_KEEP_ALL)
flags |= EW_KEEPALL;
if (options & WILD_SILENT)
flags |= EW_SILENT;
if (options & WILD_NOERROR) {
flags |= EW_NOERROR;
}
if (options & WILD_ALLLINKS) {
flags |= EW_ALLLINKS;
}
if (xp->xp_context == EXPAND_FILES
|| xp->xp_context == EXPAND_DIRECTORIES
|| xp->xp_context == EXPAND_FILES_IN_PATH) {
int free_pat = FALSE;
int i;
if (xp->xp_backslash != XP_BS_NONE) {
free_pat = TRUE;
pat = vim_strsave(pat);
for (i = 0; pat[i]; ++i)
if (pat[i] == '\\') {
if (xp->xp_backslash == XP_BS_THREE
&& pat[i + 1] == '\\'
&& pat[i + 2] == '\\'
&& pat[i + 3] == ' ')
STRMOVE(pat + i, pat + i + 3);
if (xp->xp_backslash == XP_BS_ONE
&& pat[i + 1] == ' ')
STRMOVE(pat + i, pat + i + 1);
}
}
if (xp->xp_context == EXPAND_FILES)
flags |= EW_FILE;
else if (xp->xp_context == EXPAND_FILES_IN_PATH)
flags |= (EW_FILE | EW_PATH);
else
flags = (flags | EW_DIR) & ~EW_FILE;
if (options & WILD_ICASE)
flags |= EW_ICASE;
ret = expand_wildcards_eval(&pat, num_file, file, flags);
if (free_pat)
xfree(pat);
return ret;
}
*file = NULL;
*num_file = 0;
if (xp->xp_context == EXPAND_HELP) {
if (find_help_tags(*pat == NUL ? (char_u *)"help" : pat,
num_file, file, false) == OK) {
cleanup_help_tags(*num_file, *file);
return OK;
}
return FAIL;
}
if (xp->xp_context == EXPAND_SHELLCMD) {
*file = NULL;
expand_shellcmd(pat, num_file, file, flags);
return OK;
}
if (xp->xp_context == EXPAND_OLD_SETTING) {
ExpandOldSetting(num_file, file);
return OK;
}
if (xp->xp_context == EXPAND_BUFFERS)
return ExpandBufnames(pat, num_file, file, options);
if (xp->xp_context == EXPAND_TAGS
|| xp->xp_context == EXPAND_TAGS_LISTFILES)
return expand_tags(xp->xp_context == EXPAND_TAGS, pat, num_file, file);
if (xp->xp_context == EXPAND_COLORS) {
char *directories[] = { "colors", NULL };
return ExpandRTDir(pat, DIP_START + DIP_OPT, num_file, file, directories);
}
if (xp->xp_context == EXPAND_COMPILER) {
char *directories[] = { "compiler", NULL };
return ExpandRTDir(pat, 0, num_file, file, directories);
}
if (xp->xp_context == EXPAND_OWNSYNTAX) {
char *directories[] = { "syntax", NULL };
return ExpandRTDir(pat, 0, num_file, file, directories);
}
if (xp->xp_context == EXPAND_FILETYPE) {
char *directories[] = { "syntax", "indent", "ftplugin", NULL };
return ExpandRTDir(pat, 0, num_file, file, directories);
}
if (xp->xp_context == EXPAND_CHECKHEALTH) {
char *directories[] = { "autoload/health", NULL };
return ExpandRTDir(pat, 0, num_file, file, directories);
}
if (xp->xp_context == EXPAND_USER_LIST) {
return ExpandUserList(xp, num_file, file);
}
if (xp->xp_context == EXPAND_PACKADD) {
return ExpandPackAddDir(pat, num_file, file);
}
regmatch.regprog = vim_regcomp(pat, p_magic ? RE_MAGIC : 0);
if (regmatch.regprog == NULL)
return FAIL;
regmatch.rm_ic = ignorecase(pat);
if (xp->xp_context == EXPAND_SETTINGS
|| xp->xp_context == EXPAND_BOOL_SETTINGS)
ret = ExpandSettings(xp, &regmatch, num_file, file);
else if (xp->xp_context == EXPAND_MAPPINGS)
ret = ExpandMappings(&regmatch, num_file, file);
else if (xp->xp_context == EXPAND_USER_DEFINED)
ret = ExpandUserDefined(xp, &regmatch, num_file, file);
else {
static struct expgen {
int context;
ExpandFunc func;
int ic;
int escaped;
} tab[] = {
{ EXPAND_COMMANDS, get_command_name, false, true },
{ EXPAND_BEHAVE, get_behave_arg, true, true },
{ EXPAND_MAPCLEAR, get_mapclear_arg, true, true },
{ EXPAND_MESSAGES, get_messages_arg, true, true },
{ EXPAND_HISTORY, get_history_arg, true, true },
{ EXPAND_USER_COMMANDS, get_user_commands, false, true },
{ EXPAND_USER_ADDR_TYPE, get_user_cmd_addr_type, false, true },
{ EXPAND_USER_CMD_FLAGS, get_user_cmd_flags, false, true },
{ EXPAND_USER_NARGS, get_user_cmd_nargs, false, true },
{ EXPAND_USER_COMPLETE, get_user_cmd_complete, false, true },
{ EXPAND_USER_VARS, get_user_var_name, false, true },
{ EXPAND_FUNCTIONS, get_function_name, false, true },
{ EXPAND_USER_FUNC, get_user_func_name, false, true },
{ EXPAND_EXPRESSION, get_expr_name, false, true },
{ EXPAND_MENUS, get_menu_name, false, true },
{ EXPAND_MENUNAMES, get_menu_names, false, true },
{ EXPAND_SYNTAX, get_syntax_name, true, true },
{ EXPAND_SYNTIME, get_syntime_arg, true, true },
{ EXPAND_HIGHLIGHT, (ExpandFunc)get_highlight_name, true, true },
{ EXPAND_EVENTS, get_event_name, true, true },
{ EXPAND_AUGROUP, get_augroup_name, true, true },
{ EXPAND_CSCOPE, get_cscope_name, true, true },
{ EXPAND_SIGN, get_sign_name, true, true },
{ EXPAND_PROFILE, get_profile_name, true, true },
#if defined(HAVE_WORKING_LIBINTL)
{ EXPAND_LANGUAGE, get_lang_arg, true, false },
{ EXPAND_LOCALES, get_locales, true, false },
#endif
{ EXPAND_ENV_VARS, get_env_name, true, true },
{ EXPAND_USER, get_users, true, false },
{ EXPAND_ARGLIST, get_arglist_name, true, false },
};
int i;
ret = FAIL;
for (i = 0; i < (int)ARRAY_SIZE(tab); ++i)
if (xp->xp_context == tab[i].context) {
if (tab[i].ic) {
regmatch.rm_ic = TRUE;
}
ExpandGeneric(xp, &regmatch, num_file, file, tab[i].func,
tab[i].escaped);
ret = OK;
break;
}
}
vim_regfree(regmatch.regprog);
return ret;
}
void ExpandGeneric(
expand_T *xp,
regmatch_T *regmatch,
int *num_file,
char_u ***file,
CompleteListItemGetter func, 
int escaped
)
{
int i;
size_t count = 0;
char_u *str;
for (i = 0;; ++i) {
str = (*func)(xp, i);
if (str == NULL) 
break;
if (*str == NUL) 
continue;
if (vim_regexec(regmatch, str, (colnr_T)0)) {
++count;
}
}
if (count == 0)
return;
assert(count < INT_MAX);
*num_file = (int)count;
*file = (char_u **)xmalloc(count * sizeof(char_u *));
count = 0;
for (i = 0;; i++) {
str = (*func)(xp, i);
if (str == NULL) { 
break;
}
if (*str == NUL) { 
continue;
}
if (vim_regexec(regmatch, str, (colnr_T)0)) {
if (escaped) {
str = vim_strsave_escaped(str, (char_u *)" \t\\.");
} else {
str = vim_strsave(str);
}
(*file)[count++] = str;
if (func == get_menu_names) {
str += STRLEN(str) - 1;
if (*str == '\001') {
*str = '.';
}
}
}
}
if (xp->xp_context != EXPAND_MENUNAMES && xp->xp_context != EXPAND_MENUS) {
if (xp->xp_context == EXPAND_EXPRESSION
|| xp->xp_context == EXPAND_FUNCTIONS
|| xp->xp_context == EXPAND_USER_FUNC)
qsort((void *)*file, (size_t)*num_file, sizeof(char_u *),
sort_func_compare);
else
sort_strings(*file, *num_file);
}
reset_expand_highlight();
}
static void expand_shellcmd(char_u *filepat, int *num_file, char_u ***file,
int flagsarg)
FUNC_ATTR_NONNULL_ALL
{
char_u *pat;
int i;
char_u *path = NULL;
garray_T ga;
char_u *buf = xmalloc(MAXPATHL);
size_t l;
char_u *s, *e;
int flags = flagsarg;
int ret;
bool did_curdir = false;
pat = vim_strsave(filepat);
for (i = 0; pat[i]; ++i)
if (pat[i] == '\\' && pat[i + 1] == ' ')
STRMOVE(pat + i, pat + i + 1);
flags |= EW_FILE | EW_EXEC | EW_SHELLCMD;
bool mustfree = false; 
if (pat[0] == '.' && (vim_ispathsep(pat[1])
|| (pat[1] == '.' && vim_ispathsep(pat[2])))) {
path = (char_u *)".";
} else {
if (!path_is_absolute(pat)) {
path = (char_u *)vim_getenv("PATH");
}
if (path == NULL) {
path = (char_u *)"";
} else {
mustfree = true;
}
}
ga_init(&ga, (int)sizeof(char *), 10);
hashtab_T found_ht;
hash_init(&found_ht);
for (s = path; ; s = e) {
e = vim_strchr(s, ENV_SEPCHAR);
if (e == NULL) {
e = s + STRLEN(s);
}
if (*s == NUL) {
if (did_curdir) {
break;
}
did_curdir = true;
flags |= EW_DIR;
} else if (STRNCMP(s, ".", e - s) == 0) {
did_curdir = true;
flags |= EW_DIR;
} else {
flags &= ~EW_DIR;
}
l = (size_t)(e - s);
if (l > MAXPATHL - 5) {
break;
}
STRLCPY(buf, s, l + 1);
add_pathsep((char *)buf);
l = STRLEN(buf);
STRLCPY(buf + l, pat, MAXPATHL - l);
ret = expand_wildcards(1, &buf, num_file, file, flags);
if (ret == OK) {
ga_grow(&ga, *num_file);
{
for (i = 0; i < *num_file; i++) {
char_u *name = (*file)[i];
if (STRLEN(name) > l) {
hash_T hash = hash_hash(name + l);
hashitem_T *hi =
hash_lookup(&found_ht, (const char *)(name + l),
STRLEN(name + l), hash);
if (HASHITEM_EMPTY(hi)) {
STRMOVE(name, name + l);
((char_u **)ga.ga_data)[ga.ga_len++] = name;
hash_add_item(&found_ht, hi, name, hash);
name = NULL;
}
}
xfree(name);
}
xfree(*file);
}
}
if (*e != NUL)
++e;
}
*file = ga.ga_data;
*num_file = ga.ga_len;
xfree(buf);
xfree(pat);
if (mustfree) {
xfree(path);
}
hash_clear(&found_ht);
}
static void * call_user_expand_func(user_expand_func_T user_expand_func,
expand_T *xp, int *num_file, char_u ***file)
FUNC_ATTR_NONNULL_ALL
{
char_u keep = 0;
typval_T args[4];
char_u *pat = NULL;
const sctx_T save_current_sctx = current_sctx;
struct cmdline_info save_ccline;
if (xp->xp_arg == NULL || xp->xp_arg[0] == '\0' || xp->xp_line == NULL)
return NULL;
*num_file = 0;
*file = NULL;
if (ccline.cmdbuff != NULL) {
keep = ccline.cmdbuff[ccline.cmdlen];
ccline.cmdbuff[ccline.cmdlen] = 0;
}
pat = vim_strnsave(xp->xp_pattern, xp->xp_pattern_len);
args[0].v_type = VAR_STRING;
args[1].v_type = VAR_STRING;
args[2].v_type = VAR_NUMBER;
args[3].v_type = VAR_UNKNOWN;
args[0].vval.v_string = pat;
args[1].vval.v_string = xp->xp_line;
args[2].vval.v_number = xp->xp_col;
save_ccline = ccline;
ccline.cmdbuff = NULL;
ccline.cmdprompt = NULL;
current_sctx = xp->xp_script_ctx;
void *const ret = user_expand_func(xp->xp_arg, 3, args);
ccline = save_ccline;
current_sctx = save_current_sctx;
if (ccline.cmdbuff != NULL) {
ccline.cmdbuff[ccline.cmdlen] = keep;
}
xfree(pat);
return ret;
}
static int ExpandUserDefined(expand_T *xp, regmatch_T *regmatch, int *num_file, char_u ***file)
{
char_u *e;
garray_T ga;
char_u *const retstr = call_user_expand_func(
(user_expand_func_T)call_func_retstr, xp, num_file, file);
if (retstr == NULL) {
return FAIL;
}
ga_init(&ga, (int)sizeof(char *), 3);
for (char_u *s = retstr; *s != NUL; s = e) {
e = vim_strchr(s, '\n');
if (e == NULL)
e = s + STRLEN(s);
const char_u keep = *e;
*e = NUL;
const bool skip = xp->xp_pattern[0]
&& vim_regexec(regmatch, s, (colnr_T)0) == 0;
*e = keep;
if (!skip) {
GA_APPEND(char_u *, &ga, vim_strnsave(s, (size_t)(e - s)));
}
if (*e != NUL) {
e++;
}
}
xfree(retstr);
*file = ga.ga_data;
*num_file = ga.ga_len;
return OK;
}
static int ExpandUserList(expand_T *xp, int *num_file, char_u ***file)
{
list_T *const retlist = call_user_expand_func(
(user_expand_func_T)call_func_retlist, xp, num_file, file);
if (retlist == NULL) {
return FAIL;
}
garray_T ga;
ga_init(&ga, (int)sizeof(char *), 3);
TV_LIST_ITER_CONST(retlist, li, {
if (TV_LIST_ITEM_TV(li)->v_type != VAR_STRING
|| TV_LIST_ITEM_TV(li)->vval.v_string == NULL) {
continue; 
}
GA_APPEND(char *, &ga, xstrdup(
(const char *)TV_LIST_ITEM_TV(li)->vval.v_string));
});
tv_list_unref(retlist);
*file = ga.ga_data;
*num_file = ga.ga_len;
return OK;
}
static int ExpandRTDir(char_u *pat, int flags, int *num_file, char_u ***file,
char *dirnames[])
{
*num_file = 0;
*file = NULL;
size_t pat_len = STRLEN(pat);
garray_T ga;
ga_init(&ga, (int)sizeof(char *), 10);
for (int i = 0; dirnames[i] != NULL; i++) {
size_t size = STRLEN(dirnames[i]) + pat_len + 7;
char_u *s = xmalloc(size);
snprintf((char *)s, size, "%s/%s*.vim", dirnames[i], pat);
globpath(p_rtp, s, &ga, 0);
xfree(s);
}
if (flags & DIP_START) {
for (int i = 0; dirnames[i] != NULL; i++) {
size_t size = STRLEN(dirnames[i]) + pat_len + 22;
char_u *s = xmalloc(size);
snprintf((char *)s, size, "pack/*/start/*/%s/%s*.vim", dirnames[i], pat); 
globpath(p_pp, s, &ga, 0);
xfree(s);
}
}
if (flags & DIP_OPT) {
for (int i = 0; dirnames[i] != NULL; i++) {
size_t size = STRLEN(dirnames[i]) + pat_len + 20;
char_u *s = xmalloc(size);
snprintf((char *)s, size, "pack/*/opt/*/%s/%s*.vim", dirnames[i], pat); 
globpath(p_pp, s, &ga, 0);
xfree(s);
}
}
for (int i = 0; i < ga.ga_len; i++) {
char_u *match = ((char_u **)ga.ga_data)[i];
char_u *s = match;
char_u *e = s + STRLEN(s);
if (e - s > 4 && STRNICMP(e - 4, ".vim", 4) == 0) {
e -= 4;
for (s = e; s > match; MB_PTR_BACK(match, s)) {
if (vim_ispathsep(*s)) {
break;
}
}
s++;
*e = NUL;
assert((e - s) + 1 >= 0);
memmove(match, s, (size_t)(e - s) + 1);
}
}
if (GA_EMPTY(&ga))
return FAIL;
ga_remove_duplicate_strings(&ga);
*file = ga.ga_data;
*num_file = ga.ga_len;
return OK;
}
static int ExpandPackAddDir(char_u *pat, int *num_file, char_u ***file)
{
garray_T ga;
*num_file = 0;
*file = NULL;
size_t pat_len = STRLEN(pat);
ga_init(&ga, (int)sizeof(char *), 10);
size_t buflen = pat_len + 26;
char_u *s = xmalloc(buflen);
snprintf((char *)s, buflen, "pack/*/opt/%s*", pat); 
globpath(p_pp, s, &ga, 0);
xfree(s);
for (int i = 0; i < ga.ga_len; i++) {
char_u *match = ((char_u **)ga.ga_data)[i];
s = path_tail(match);
memmove(match, s, STRLEN(s)+1);
}
if (GA_EMPTY(&ga)) {
return FAIL;
}
ga_remove_duplicate_strings(&ga);
*file = ga.ga_data;
*num_file = ga.ga_len;
return OK;
}
void globpath(char_u *path, char_u *file, garray_T *ga, int expand_options)
{
expand_T xpc;
ExpandInit(&xpc);
xpc.xp_context = EXPAND_FILES;
char_u *buf = xmalloc(MAXPATHL);
while (*path != NUL) {
copy_option_part(&path, buf, MAXPATHL, ",");
if (STRLEN(buf) + STRLEN(file) + 2 < MAXPATHL) {
add_pathsep((char *)buf);
STRCAT(buf, file); 
char_u **p;
int num_p = 0;
(void)ExpandFromContext(&xpc, buf, &num_p, &p,
WILD_SILENT | expand_options);
if (num_p > 0) {
ExpandEscape(&xpc, buf, num_p, p, WILD_SILENT | expand_options);
ga_grow(ga, num_p);
for (int i = 0; i < num_p; i++) {
((char_u **)ga->ga_data)[ga->ga_len] = vim_strsave(p[i]);
ga->ga_len++;
}
FreeWild(num_p, p);
}
}
}
xfree(buf);
}
static HistoryType hist_char2type(const int c)
FUNC_ATTR_CONST FUNC_ATTR_WARN_UNUSED_RESULT
{
switch (c) {
case ':': {
return HIST_CMD;
}
case '=': {
return HIST_EXPR;
}
case '@': {
return HIST_INPUT;
}
case '>': {
return HIST_DEBUG;
}
case NUL:
case '/':
case '?': {
return HIST_SEARCH;
}
default: {
return HIST_INVALID;
}
}
return 0;
}
static char *(history_names[]) =
{
"cmd",
"search",
"expr",
"input",
"debug",
NULL
};
static char_u *get_history_arg(expand_T *xp, int idx)
{
static char_u compl[2] = { NUL, NUL };
char *short_names = ":=@>?/";
int short_names_count = (int)STRLEN(short_names);
int history_name_count = ARRAY_SIZE(history_names) - 1;
if (idx < short_names_count) {
compl[0] = (char_u)short_names[idx];
return compl;
}
if (idx < short_names_count + history_name_count)
return (char_u *)history_names[idx - short_names_count];
if (idx == short_names_count + history_name_count)
return (char_u *)"all";
return NULL;
}
void init_history(void)
{
assert(p_hi >= 0 && p_hi <= INT_MAX);
int newlen = (int)p_hi;
int oldlen = hislen;
if (newlen != oldlen) {
for (int type = 0; type < HIST_COUNT; type++) {
histentry_T *temp = (newlen
? xmalloc((size_t)newlen * sizeof(*temp))
: NULL);
int j = hisidx[type];
if (j >= 0) {
int l1 = MIN(j + 1, newlen); 
int l2 = MIN(newlen, oldlen) - l1; 
int i1 = j + 1 - l1; 
int i2 = MAX(l1, oldlen - newlen + l1); 
if (newlen) {
memcpy(&temp[0], &history[type][i2], (size_t)l2 * sizeof(*temp));
memcpy(&temp[l2], &history[type][i1], (size_t)l1 * sizeof(*temp));
}
for (int i = 0; i < i1; i++) {
hist_free_entry(history[type] + i);
}
for (int i = i1 + l1; i < i2; i++) {
hist_free_entry(history[type] + i);
}
}
int l3 = j < 0 ? 0 : MIN(newlen, oldlen); 
if (newlen) {
memset(temp + l3, 0, (size_t)(newlen - l3) * sizeof(*temp));
}
hisidx[type] = l3 - 1;
xfree(history[type]);
history[type] = temp;
}
hislen = newlen;
}
}
static inline void hist_free_entry(histentry_T *hisptr)
FUNC_ATTR_NONNULL_ALL
{
xfree(hisptr->hisstr);
tv_list_unref(hisptr->additional_elements);
clear_hist_entry(hisptr);
}
static inline void clear_hist_entry(histentry_T *hisptr)
FUNC_ATTR_NONNULL_ALL
{
memset(hisptr, 0, sizeof(*hisptr));
}
static int 
in_history (
int type,
char_u *str,
int move_to_front, 
int sep
)
{
int i;
int last_i = -1;
char_u *p;
if (hisidx[type] < 0)
return FALSE;
i = hisidx[type];
do {
if (history[type][i].hisstr == NULL)
return FALSE;
p = history[type][i].hisstr;
if (STRCMP(str, p) == 0
&& (type != HIST_SEARCH || sep == p[STRLEN(p) + 1])) {
if (!move_to_front)
return TRUE;
last_i = i;
break;
}
if (--i < 0)
i = hislen - 1;
} while (i != hisidx[type]);
if (last_i >= 0) {
list_T *const list = history[type][i].additional_elements;
str = history[type][i].hisstr;
while (i != hisidx[type]) {
if (++i >= hislen)
i = 0;
history[type][last_i] = history[type][i];
last_i = i;
}
tv_list_unref(list);
history[type][i].hisnum = ++hisnum[type];
history[type][i].hisstr = str;
history[type][i].timestamp = os_time();
history[type][i].additional_elements = NULL;
return true;
}
return false;
}
HistoryType get_histtype(const char *const name, const size_t len,
const bool return_default)
FUNC_ATTR_PURE FUNC_ATTR_WARN_UNUSED_RESULT
{
if (len == 0) {
return return_default ? HIST_DEFAULT : hist_char2type(ccline.cmdfirstc);
}
for (HistoryType i = 0; history_names[i] != NULL; i++) {
if (STRNICMP(name, history_names[i], len) == 0) {
return i;
}
}
if (vim_strchr((char_u *)":=@>?/", name[0]) != NULL && len == 1) {
return hist_char2type(name[0]);
}
return HIST_INVALID;
}
static int last_maptick = -1; 
void 
add_to_history (
int histype,
char_u *new_entry,
int in_map, 
int sep 
)
{
histentry_T *hisptr;
if (hislen == 0 || histype == HIST_INVALID) { 
return;
}
assert(histype != HIST_DEFAULT);
if (cmdmod.keeppatterns && histype == HIST_SEARCH)
return;
if (histype == HIST_SEARCH && in_map) {
if (maptick == last_maptick && hisidx[HIST_SEARCH] >= 0) {
hisptr = &history[HIST_SEARCH][hisidx[HIST_SEARCH]];
hist_free_entry(hisptr);
--hisnum[histype];
if (--hisidx[HIST_SEARCH] < 0)
hisidx[HIST_SEARCH] = hislen - 1;
}
last_maptick = -1;
}
if (!in_history(histype, new_entry, true, sep)) {
if (++hisidx[histype] == hislen)
hisidx[histype] = 0;
hisptr = &history[histype][hisidx[histype]];
hist_free_entry(hisptr);
size_t len = STRLEN(new_entry);
hisptr->hisstr = vim_strnsave(new_entry, len + 2);
hisptr->timestamp = os_time();
hisptr->additional_elements = NULL;
hisptr->hisstr[len + 1] = (char_u)sep;
hisptr->hisnum = ++hisnum[histype];
if (histype == HIST_SEARCH && in_map)
last_maptick = maptick;
}
}
int get_history_idx(int histype)
{
if (hislen == 0 || histype < 0 || histype >= HIST_COUNT
|| hisidx[histype] < 0)
return -1;
return history[histype][hisidx[histype]].hisnum;
}
static struct cmdline_info *get_ccline_ptr(void)
{
if ((State & CMDLINE) == 0) {
return NULL;
} else if (ccline.cmdbuff != NULL) {
return &ccline;
} else if (ccline.prev_ccline && ccline.prev_ccline->cmdbuff != NULL) {
return ccline.prev_ccline;
} else {
return NULL;
}
}
char_u *get_cmdline_str(void)
{
if (cmdline_star > 0) {
return NULL;
}
struct cmdline_info *p = get_ccline_ptr();
if (p == NULL)
return NULL;
return vim_strnsave(p->cmdbuff, (size_t)p->cmdlen);
}
int get_cmdline_pos(void)
{
struct cmdline_info *p = get_ccline_ptr();
if (p == NULL)
return -1;
return p->cmdpos;
}
int set_cmdline_pos(int pos)
{
struct cmdline_info *p = get_ccline_ptr();
if (p == NULL)
return 1;
if (pos < 0)
new_cmdpos = 0;
else
new_cmdpos = pos;
return 0;
}
int get_cmdline_type(void)
{
struct cmdline_info *p = get_ccline_ptr();
if (p == NULL)
return NUL;
if (p->cmdfirstc == NUL)
return (p->input_fn) ? '@' : '-';
return p->cmdfirstc;
}
static int calc_hist_idx(int histype, int num)
{
int i;
histentry_T *hist;
int wrapped = FALSE;
if (hislen == 0 || histype < 0 || histype >= HIST_COUNT
|| (i = hisidx[histype]) < 0 || num == 0)
return -1;
hist = history[histype];
if (num > 0) {
while (hist[i].hisnum > num)
if (--i < 0) {
if (wrapped)
break;
i += hislen;
wrapped = TRUE;
}
if (hist[i].hisnum == num && hist[i].hisstr != NULL)
return i;
} else if (-num <= hislen) {
i += num + 1;
if (i < 0)
i += hislen;
if (hist[i].hisstr != NULL)
return i;
}
return -1;
}
char_u *get_history_entry(int histype, int idx)
{
idx = calc_hist_idx(histype, idx);
if (idx >= 0)
return history[histype][idx].hisstr;
else
return (char_u *)"";
}
int clr_history(const int histype)
{
if (hislen != 0 && histype >= 0 && histype < HIST_COUNT) {
histentry_T *hisptr = history[histype];
for (int i = hislen; i--; hisptr++) {
hist_free_entry(hisptr);
}
hisidx[histype] = -1; 
hisnum[histype] = 0; 
return OK;
}
return FAIL;
}
int del_history_entry(int histype, char_u *str)
{
regmatch_T regmatch;
histentry_T *hisptr;
int idx;
int i;
int last;
bool found = false;
regmatch.regprog = NULL;
regmatch.rm_ic = FALSE; 
if (hislen != 0
&& histype >= 0
&& histype < HIST_COUNT
&& *str != NUL
&& (idx = hisidx[histype]) >= 0
&& (regmatch.regprog = vim_regcomp(str, RE_MAGIC + RE_STRING))
!= NULL) {
i = last = idx;
do {
hisptr = &history[histype][i];
if (hisptr->hisstr == NULL)
break;
if (vim_regexec(&regmatch, hisptr->hisstr, (colnr_T)0)) {
found = true;
hist_free_entry(hisptr);
} else {
if (i != last) {
history[histype][last] = *hisptr;
clear_hist_entry(hisptr);
}
if (--last < 0)
last += hislen;
}
if (--i < 0)
i += hislen;
} while (i != idx);
if (history[histype][idx].hisstr == NULL)
hisidx[histype] = -1;
}
vim_regfree(regmatch.regprog);
return found;
}
int del_history_idx(int histype, int idx)
{
int i, j;
i = calc_hist_idx(histype, idx);
if (i < 0)
return FALSE;
idx = hisidx[histype];
hist_free_entry(&history[histype][i]);
if (histype == HIST_SEARCH && maptick == last_maptick && i == idx)
last_maptick = -1;
while (i != idx) {
j = (i + 1) % hislen;
history[histype][i] = history[histype][j];
i = j;
}
clear_hist_entry(&history[histype][idx]);
if (--i < 0) {
i += hislen;
}
hisidx[histype] = i;
return TRUE;
}
int get_list_range(char_u **str, int *num1, int *num2)
{
int len;
int first = false;
varnumber_T num;
*str = skipwhite(*str);
if (**str == '-' || ascii_isdigit(**str)) { 
vim_str2nr(*str, NULL, &len, 0, &num, NULL, 0);
*str += len;
*num1 = (int)num;
first = true;
}
*str = skipwhite(*str);
if (**str == ',') { 
*str = skipwhite(*str + 1);
vim_str2nr(*str, NULL, &len, 0, &num, NULL, 0);
if (len > 0) {
*num2 = (int)num;
*str = skipwhite(*str + len);
} else if (!first) { 
return FAIL;
}
} else if (first) { 
*num2 = *num1;
}
return OK;
}
void ex_history(exarg_T *eap)
{
histentry_T *hist;
int histype1 = HIST_CMD;
int histype2 = HIST_CMD;
int hisidx1 = 1;
int hisidx2 = -1;
int idx;
int i, j, k;
char_u *end;
char_u *arg = eap->arg;
if (hislen == 0) {
MSG(_("'history' option is zero"));
return;
}
if (!(ascii_isdigit(*arg) || *arg == '-' || *arg == ',')) {
end = arg;
while (ASCII_ISALPHA(*end)
|| vim_strchr((char_u *)":=@>/?", *end) != NULL)
end++;
histype1 = get_histtype((const char *)arg, (size_t)(end - arg), false);
if (histype1 == HIST_INVALID) {
if (STRNICMP(arg, "all", end - arg) == 0) {
histype1 = 0;
histype2 = HIST_COUNT-1;
} else {
EMSG(_(e_trailing));
return;
}
} else
histype2 = histype1;
} else {
end = arg;
}
if (!get_list_range(&end, &hisidx1, &hisidx2) || *end != NUL) {
EMSG(_(e_trailing));
return;
}
for (; !got_int && histype1 <= histype2; ++histype1) {
STRCPY(IObuff, "\n #");
assert(history_names[histype1] != NULL);
STRCAT(STRCAT(IObuff, history_names[histype1]), " history");
MSG_PUTS_TITLE(IObuff);
idx = hisidx[histype1];
hist = history[histype1];
j = hisidx1;
k = hisidx2;
if (j < 0)
j = (-j > hislen) ? 0 : hist[(hislen+j+idx+1) % hislen].hisnum;
if (k < 0)
k = (-k > hislen) ? 0 : hist[(hislen+k+idx+1) % hislen].hisnum;
if (idx >= 0 && j <= k)
for (i = idx + 1; !got_int; ++i) {
if (i == hislen)
i = 0;
if (hist[i].hisstr != NULL
&& hist[i].hisnum >= j && hist[i].hisnum <= k) {
msg_putchar('\n');
snprintf((char *)IObuff, IOSIZE, "%c%6d ", i == idx ? '>' : ' ',
hist[i].hisnum);
if (vim_strsize(hist[i].hisstr) > Columns - 10) {
trunc_string(hist[i].hisstr, IObuff + STRLEN(IObuff),
Columns - 10, IOSIZE - (int)STRLEN(IObuff));
} else {
STRCAT(IObuff, hist[i].hisstr);
}
msg_outtrans(IObuff);
ui_flush();
}
if (i == idx)
break;
}
}
}
int hist_type2char(int type)
FUNC_ATTR_CONST
{
switch (type) {
case HIST_CMD: {
return ':';
}
case HIST_SEARCH: {
return '/';
}
case HIST_EXPR: {
return '=';
}
case HIST_INPUT: {
return '@';
}
case HIST_DEBUG: {
return '>';
}
default: {
assert(false);
}
}
return NUL;
}
static int open_cmdwin(void)
{
struct cmdline_info save_ccline;
bufref_T old_curbuf;
bufref_T bufref;
win_T *old_curwin = curwin;
win_T *wp;
int i;
linenr_T lnum;
garray_T winsizes;
char_u typestr[2];
int save_restart_edit = restart_edit;
int save_State = State;
int save_exmode = exmode_active;
int save_cmdmsg_rl = cmdmsg_rl;
if (cmdwin_type != 0
|| cmdline_star > 0
) {
beep_flush();
return K_IGNORE;
}
set_bufref(&old_curbuf, curbuf);
win_size_save(&winsizes);
pum_undisplay(true);
cmdmod.tab = 0;
cmdmod.noswapfile = 1;
if (win_split((int)p_cwh, WSP_BOT) == FAIL) {
beep_flush();
return K_IGNORE;
}
cmdwin_type = get_cmdline_type();
cmdwin_level = ccline.level;
buf_open_scratch(0, "[Command Line]");
set_option_value("bh", 0L, "wipe", OPT_LOCAL);
curwin->w_p_rl = cmdmsg_rl;
cmdmsg_rl = false;
curbuf->b_p_ma = true;
curwin->w_p_fen = false;
curbuf_lock++;
need_wait_return = false;
const int histtype = hist_char2type(cmdwin_type);
if (histtype == HIST_CMD || histtype == HIST_DEBUG) {
if (p_wc == TAB) {
add_map((char_u *)"<buffer> <Tab> <C-X><C-V>", INSERT);
add_map((char_u *)"<buffer> <Tab> a<C-X><C-V>", NORMAL);
}
set_option_value("ft", 0L, "vim", OPT_LOCAL);
}
curbuf_lock--;
curbuf->b_p_tw = 0;
init_history();
if (hislen > 0 && histtype != HIST_INVALID) {
i = hisidx[histtype];
if (i >= 0) {
lnum = 0;
do {
if (++i == hislen)
i = 0;
if (history[histtype][i].hisstr != NULL) {
ml_append(lnum++, history[histtype][i].hisstr, (colnr_T)0, false);
}
} while (i != hisidx[histtype]);
}
}
ml_replace(curbuf->b_ml.ml_line_count, ccline.cmdbuff, true);
curwin->w_cursor.lnum = curbuf->b_ml.ml_line_count;
curwin->w_cursor.col = ccline.cmdpos;
changed_line_abv_curs();
invalidate_botline();
if (ui_has(kUICmdline)) {
ccline.redraw_state = kCmdRedrawNone;
ui_call_cmdline_hide(ccline.level);
}
redraw_later(SOME_VALID);
save_cmdline(&save_ccline);
exmode_active = 0;
State = NORMAL;
setmouse();
typestr[0] = (char_u)cmdwin_type;
typestr[1] = NUL;
apply_autocmds(EVENT_CMDWINENTER, typestr, typestr, false, curbuf);
if (restart_edit != 0) { 
stuffcharReadbuff(K_NOP);
}
i = RedrawingDisabled;
RedrawingDisabled = 0;
int save_count = save_batch_count();
cmdwin_result = 0;
normal_enter(true, false);
RedrawingDisabled = i;
restore_batch_count(save_count);
const bool save_KeyTyped = KeyTyped;
apply_autocmds(EVENT_CMDWINLEAVE, typestr, typestr, false, curbuf);
KeyTyped = save_KeyTyped;
restore_cmdline(&save_ccline);
cmdwin_type = 0;
cmdwin_level = 0;
exmode_active = save_exmode;
if (!win_valid(old_curwin) || !bufref_valid(&old_curbuf)) {
cmdwin_result = Ctrl_C;
EMSG(_("E199: Active window or buffer deleted"));
} else {
if (aborting() && cmdwin_result != K_IGNORE)
cmdwin_result = Ctrl_C;
xfree(ccline.cmdbuff);
if (cmdwin_result == K_XF1 || cmdwin_result == K_XF2) { 
const char *p = (cmdwin_result == K_XF2) ? "qa" : "qa!";
if (histtype == HIST_CMD) {
ccline.cmdbuff = (char_u *)xstrdup(p);
cmdwin_result = CAR;
} else {
ccline.cmdbuff = NULL;
stuffcharReadbuff(':');
stuffReadbuff(p);
stuffcharReadbuff(CAR);
}
} else if (cmdwin_result == Ctrl_C) {
ccline.cmdbuff = NULL;
} else
ccline.cmdbuff = vim_strsave(get_cursor_line_ptr());
if (ccline.cmdbuff == NULL) {
ccline.cmdbuff = vim_strsave((char_u *)"");
ccline.cmdlen = 0;
ccline.cmdbufflen = 1;
ccline.cmdpos = 0;
cmdwin_result = Ctrl_C;
} else {
ccline.cmdlen = (int)STRLEN(ccline.cmdbuff);
ccline.cmdbufflen = ccline.cmdlen + 1;
ccline.cmdpos = curwin->w_cursor.col;
if (ccline.cmdpos > ccline.cmdlen)
ccline.cmdpos = ccline.cmdlen;
if (cmdwin_result == K_IGNORE) {
ccline.cmdspos = cmd_screencol(ccline.cmdpos);
redrawcmd();
}
}
curwin->w_p_cole = 0;
wp = curwin;
set_bufref(&bufref, curbuf);
win_goto(old_curwin);
win_close(wp, true);
if (bufref_valid(&bufref)) {
close_buffer(NULL, bufref.br_buf, DOBUF_WIPE, false);
}
win_size_restore(&winsizes);
}
ga_clear(&winsizes);
restart_edit = save_restart_edit;
cmdmsg_rl = save_cmdmsg_rl;
State = save_State;
setmouse();
return cmdwin_result;
}
char *script_get(exarg_T *const eap, size_t *const lenp)
FUNC_ATTR_NONNULL_ALL FUNC_ATTR_WARN_UNUSED_RESULT FUNC_ATTR_MALLOC
{
const char *const cmd = (const char *)eap->arg;
if (cmd[0] != '<' || cmd[1] != '<' || eap->getline == NULL) {
*lenp = STRLEN(eap->arg);
return eap->skip ? NULL : xmemdupz(eap->arg, *lenp);
}
garray_T ga = { .ga_data = NULL, .ga_len = 0 };
if (!eap->skip) {
ga_init(&ga, 1, 0x400);
}
const char *const end_pattern = (
cmd[2] != NUL
? (const char *)skipwhite((const char_u *)cmd + 2)
: ".");
for (;;) {
char *const theline = (char *)eap->getline(
eap->cstack->cs_looplevel > 0 ? -1 :
NUL, eap->cookie, 0, true);
if (theline == NULL || strcmp(end_pattern, theline) == 0) {
xfree(theline);
break;
}
if (!eap->skip) {
ga_concat(&ga, (const char_u *)theline);
ga_append(&ga, '\n');
}
xfree(theline);
}
*lenp = (size_t)ga.ga_len; 
if (!eap->skip) {
ga_append(&ga, NUL);
}
return (char *)ga.ga_data;
}
const void *hist_iter(const void *const iter, const uint8_t history_type,
const bool zero, histentry_T *const hist)
FUNC_ATTR_WARN_UNUSED_RESULT FUNC_ATTR_NONNULL_ARG(4)
{
*hist = (histentry_T) {
.hisstr = NULL
};
if (hisidx[history_type] == -1) {
return NULL;
}
histentry_T *const hstart = &(history[history_type][0]);
histentry_T *const hlast = (
&(history[history_type][hisidx[history_type]]));
const histentry_T *const hend = &(history[history_type][hislen - 1]);
histentry_T *hiter;
if (iter == NULL) {
histentry_T *hfirst = hlast;
do {
hfirst++;
if (hfirst > hend) {
hfirst = hstart;
}
if (hfirst->hisstr != NULL) {
break;
}
} while (hfirst != hlast);
hiter = hfirst;
} else {
hiter = (histentry_T *) iter;
}
if (hiter == NULL) {
return NULL;
}
*hist = *hiter;
if (zero) {
memset(hiter, 0, sizeof(*hiter));
}
if (hiter == hlast) {
return NULL;
}
hiter++;
return (const void *) ((hiter > hend) ? hstart : hiter);
}
histentry_T *hist_get_array(const uint8_t history_type, int **const new_hisidx,
int **const new_hisnum)
FUNC_ATTR_WARN_UNUSED_RESULT FUNC_ATTR_NONNULL_ALL
{
init_history();
*new_hisidx = &(hisidx[history_type]);
*new_hisnum = &(hisnum[history_type]);
return history[history_type];
}
static void set_search_match(pos_T *t)
{
t->lnum += search_match_lines;
t->col = search_match_endcol;
if (t->lnum > curbuf->b_ml.ml_line_count) {
t->lnum = curbuf->b_ml.ml_line_count;
coladvance((colnr_T)MAXCOL);
}
}
