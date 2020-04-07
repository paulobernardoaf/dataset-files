#include <assert.h>
#include <inttypes.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "nvim/log.h"
#include "nvim/vim.h"
#include "nvim/ascii.h"
#include "nvim/normal.h"
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
#include "nvim/ex_docmd.h"
#include "nvim/ex_getln.h"
#include "nvim/fileio.h"
#include "nvim/fold.h"
#include "nvim/getchar.h"
#include "nvim/indent.h"
#include "nvim/indent_c.h"
#include "nvim/main.h"
#include "nvim/mark.h"
#include "nvim/memline.h"
#include "nvim/memory.h"
#include "nvim/message.h"
#include "nvim/misc1.h"
#include "nvim/keymap.h"
#include "nvim/move.h"
#include "nvim/mouse.h"
#include "nvim/ops.h"
#include "nvim/option.h"
#include "nvim/quickfix.h"
#include "nvim/screen.h"
#include "nvim/search.h"
#include "nvim/spell.h"
#include "nvim/spellfile.h"
#include "nvim/strings.h"
#include "nvim/syntax.h"
#include "nvim/tag.h"
#include "nvim/ui.h"
#include "nvim/mouse.h"
#include "nvim/undo.h"
#include "nvim/window.h"
#include "nvim/state.h"
#include "nvim/event/loop.h"
#include "nvim/os/time.h"
#include "nvim/os/input.h"
#include "nvim/api/private/helpers.h"
typedef struct normal_state {
VimState state;
bool command_finished;
bool ctrl_w;
bool need_flushbuf;
bool set_prevcount;
bool previous_got_int; 
bool cmdwin; 
bool noexmode; 
bool toplevel; 
oparg_T oa; 
cmdarg_T ca; 
int mapped_len;
int old_mapped_len;
int idx;
int c;
int old_col;
pos_T old_pos;
} NormalState;
static int resel_VIsual_mode = NUL; 
static linenr_T resel_VIsual_line_count; 
static colnr_T resel_VIsual_vcol; 
static int VIsual_mode_orig = NUL; 
static int restart_VIsual_select = 0;
#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "normal.c.generated.h"
#endif
static inline void normal_state_init(NormalState *s)
{
memset(s, 0, sizeof(NormalState));
s->state.check = normal_check;
s->state.execute = normal_execute;
}
static char *e_noident = N_("E349: No identifier under cursor");
typedef void (*nv_func_T)(cmdarg_T *cap);
#define NV_NCH 0x01 
#define NV_NCH_NOP (0x02|NV_NCH) 
#define NV_NCH_ALW (0x04|NV_NCH) 
#define NV_LANG 0x08 
#define NV_SS 0x10 
#define NV_SSS 0x20 
#define NV_STS 0x40 
#define NV_RL 0x80 
#define NV_KEEPREG 0x100 
#define NV_NCW 0x200 
static const struct nv_cmd {
int cmd_char; 
nv_func_T cmd_func; 
uint16_t cmd_flags; 
short cmd_arg; 
} nv_cmds[] =
{
{ NUL, nv_error, 0, 0 },
{ Ctrl_A, nv_addsub, 0, 0 },
{ Ctrl_B, nv_page, NV_STS, BACKWARD },
{ Ctrl_C, nv_esc, 0, true },
{ Ctrl_D, nv_halfpage, 0, 0 },
{ Ctrl_E, nv_scroll_line, 0, true },
{ Ctrl_F, nv_page, NV_STS, FORWARD },
{ Ctrl_G, nv_ctrlg, 0, 0 },
{ Ctrl_H, nv_ctrlh, 0, 0 },
{ Ctrl_I, nv_pcmark, 0, 0 },
{ NL, nv_down, 0, false },
{ Ctrl_K, nv_error, 0, 0 },
{ Ctrl_L, nv_clear, 0, 0 },
{ Ctrl_M, nv_down, 0, true },
{ Ctrl_N, nv_down, NV_STS, false },
{ Ctrl_O, nv_ctrlo, 0, 0 },
{ Ctrl_P, nv_up, NV_STS, false },
{ Ctrl_Q, nv_visual, 0, false },
{ Ctrl_R, nv_redo, 0, 0 },
{ Ctrl_S, nv_ignore, 0, 0 },
{ Ctrl_T, nv_tagpop, NV_NCW, 0 },
{ Ctrl_U, nv_halfpage, 0, 0 },
{ Ctrl_V, nv_visual, 0, false },
{ 'V', nv_visual, 0, false },
{ 'v', nv_visual, 0, false },
{ Ctrl_W, nv_window, 0, 0 },
{ Ctrl_X, nv_addsub, 0, 0 },
{ Ctrl_Y, nv_scroll_line, 0, false },
{ Ctrl_Z, nv_suspend, 0, 0 },
{ ESC, nv_esc, 0, false },
{ Ctrl_BSL, nv_normal, NV_NCH_ALW, 0 },
{ Ctrl_RSB, nv_ident, NV_NCW, 0 },
{ Ctrl_HAT, nv_hat, NV_NCW, 0 },
{ Ctrl__, nv_error, 0, 0 },
{ ' ', nv_right, 0, 0 },
{ '!', nv_operator, 0, 0 },
{ '"', nv_regname, NV_NCH_NOP|NV_KEEPREG, 0 },
{ '#', nv_ident, 0, 0 },
{ '$', nv_dollar, 0, 0 },
{ '%', nv_percent, 0, 0 },
{ '&', nv_optrans, 0, 0 },
{ '\'', nv_gomark, NV_NCH_ALW, true },
{ '(', nv_brace, 0, BACKWARD },
{ ')', nv_brace, 0, FORWARD },
{ '*', nv_ident, 0, 0 },
{ '+', nv_down, 0, true },
{ ',', nv_csearch, 0, true },
{ '-', nv_up, 0, true },
{ '.', nv_dot, NV_KEEPREG, 0 },
{ '/', nv_search, 0, false },
{ '0', nv_beginline, 0, 0 },
{ '1', nv_ignore, 0, 0 },
{ '2', nv_ignore, 0, 0 },
{ '3', nv_ignore, 0, 0 },
{ '4', nv_ignore, 0, 0 },
{ '5', nv_ignore, 0, 0 },
{ '6', nv_ignore, 0, 0 },
{ '7', nv_ignore, 0, 0 },
{ '8', nv_ignore, 0, 0 },
{ '9', nv_ignore, 0, 0 },
{ ':', nv_colon, 0, 0 },
{ ';', nv_csearch, 0, false },
{ '<', nv_operator, NV_RL, 0 },
{ '=', nv_operator, 0, 0 },
{ '>', nv_operator, NV_RL, 0 },
{ '?', nv_search, 0, false },
{ '@', nv_at, NV_NCH_NOP, false },
{ 'A', nv_edit, 0, 0 },
{ 'B', nv_bck_word, 0, 1 },
{ 'C', nv_abbrev, NV_KEEPREG, 0 },
{ 'D', nv_abbrev, NV_KEEPREG, 0 },
{ 'E', nv_wordcmd, 0, true },
{ 'F', nv_csearch, NV_NCH_ALW|NV_LANG, BACKWARD },
{ 'G', nv_goto, 0, true },
{ 'H', nv_scroll, 0, 0 },
{ 'I', nv_edit, 0, 0 },
{ 'J', nv_join, 0, 0 },
{ 'K', nv_ident, 0, 0 },
{ 'L', nv_scroll, 0, 0 },
{ 'M', nv_scroll, 0, 0 },
{ 'N', nv_next, 0, SEARCH_REV },
{ 'O', nv_open, 0, 0 },
{ 'P', nv_put, 0, 0 },
{ 'Q', nv_exmode, NV_NCW, 0 },
{ 'R', nv_Replace, 0, false },
{ 'S', nv_subst, NV_KEEPREG, 0 },
{ 'T', nv_csearch, NV_NCH_ALW|NV_LANG, BACKWARD },
{ 'U', nv_Undo, 0, 0 },
{ 'W', nv_wordcmd, 0, true },
{ 'X', nv_abbrev, NV_KEEPREG, 0 },
{ 'Y', nv_abbrev, NV_KEEPREG, 0 },
{ 'Z', nv_Zet, NV_NCH_NOP|NV_NCW, 0 },
{ '[', nv_brackets, NV_NCH_ALW, BACKWARD },
{ '\\', nv_error, 0, 0 },
{ ']', nv_brackets, NV_NCH_ALW, FORWARD },
{ '^', nv_beginline, 0, BL_WHITE | BL_FIX },
{ '_', nv_lineop, 0, 0 },
{ '`', nv_gomark, NV_NCH_ALW, false },
{ 'a', nv_edit, NV_NCH, 0 },
{ 'b', nv_bck_word, 0, 0 },
{ 'c', nv_operator, 0, 0 },
{ 'd', nv_operator, 0, 0 },
{ 'e', nv_wordcmd, 0, false },
{ 'f', nv_csearch, NV_NCH_ALW|NV_LANG, FORWARD },
{ 'g', nv_g_cmd, NV_NCH_ALW, false },
{ 'h', nv_left, NV_RL, 0 },
{ 'i', nv_edit, NV_NCH, 0 },
{ 'j', nv_down, 0, false },
{ 'k', nv_up, 0, false },
{ 'l', nv_right, NV_RL, 0 },
{ 'm', nv_mark, NV_NCH_NOP, 0 },
{ 'n', nv_next, 0, 0 },
{ 'o', nv_open, 0, 0 },
{ 'p', nv_put, 0, 0 },
{ 'q', nv_record, NV_NCH, 0 },
{ 'r', nv_replace, NV_NCH_NOP|NV_LANG, 0 },
{ 's', nv_subst, NV_KEEPREG, 0 },
{ 't', nv_csearch, NV_NCH_ALW|NV_LANG, FORWARD },
{ 'u', nv_undo, 0, 0 },
{ 'w', nv_wordcmd, 0, false },
{ 'x', nv_abbrev, NV_KEEPREG, 0 },
{ 'y', nv_operator, 0, 0 },
{ 'z', nv_zet, NV_NCH_ALW, 0 },
{ '{', nv_findpar, 0, BACKWARD },
{ '|', nv_pipe, 0, 0 },
{ '}', nv_findpar, 0, FORWARD },
{ '~', nv_tilde, 0, 0 },
{ POUND, nv_ident, 0, 0 },
{ K_MOUSEUP, nv_mousescroll, 0, MSCR_UP },
{ K_MOUSEDOWN, nv_mousescroll, 0, MSCR_DOWN },
{ K_MOUSELEFT, nv_mousescroll, 0, MSCR_LEFT },
{ K_MOUSERIGHT, nv_mousescroll, 0, MSCR_RIGHT },
{ K_LEFTMOUSE, nv_mouse, 0, 0 },
{ K_LEFTMOUSE_NM, nv_mouse, 0, 0 },
{ K_LEFTDRAG, nv_mouse, 0, 0 },
{ K_LEFTRELEASE, nv_mouse, 0, 0 },
{ K_LEFTRELEASE_NM, nv_mouse, 0, 0 },
{ K_MIDDLEMOUSE, nv_mouse, 0, 0 },
{ K_MIDDLEDRAG, nv_mouse, 0, 0 },
{ K_MIDDLERELEASE, nv_mouse, 0, 0 },
{ K_RIGHTMOUSE, nv_mouse, 0, 0 },
{ K_RIGHTDRAG, nv_mouse, 0, 0 },
{ K_RIGHTRELEASE, nv_mouse, 0, 0 },
{ K_X1MOUSE, nv_mouse, 0, 0 },
{ K_X1DRAG, nv_mouse, 0, 0 },
{ K_X1RELEASE, nv_mouse, 0, 0 },
{ K_X2MOUSE, nv_mouse, 0, 0 },
{ K_X2DRAG, nv_mouse, 0, 0 },
{ K_X2RELEASE, nv_mouse, 0, 0 },
{ K_IGNORE, nv_ignore, NV_KEEPREG, 0 },
{ K_NOP, nv_nop, 0, 0 },
{ K_INS, nv_edit, 0, 0 },
{ K_KINS, nv_edit, 0, 0 },
{ K_BS, nv_ctrlh, 0, 0 },
{ K_UP, nv_up, NV_SSS|NV_STS, false },
{ K_S_UP, nv_page, NV_SS, BACKWARD },
{ K_DOWN, nv_down, NV_SSS|NV_STS, false },
{ K_S_DOWN, nv_page, NV_SS, FORWARD },
{ K_LEFT, nv_left, NV_SSS|NV_STS|NV_RL, 0 },
{ K_S_LEFT, nv_bck_word, NV_SS|NV_RL, 0 },
{ K_C_LEFT, nv_bck_word, NV_SSS|NV_RL|NV_STS, 1 },
{ K_RIGHT, nv_right, NV_SSS|NV_STS|NV_RL, 0 },
{ K_S_RIGHT, nv_wordcmd, NV_SS|NV_RL, false },
{ K_C_RIGHT, nv_wordcmd, NV_SSS|NV_RL|NV_STS, true },
{ K_PAGEUP, nv_page, NV_SSS|NV_STS, BACKWARD },
{ K_KPAGEUP, nv_page, NV_SSS|NV_STS, BACKWARD },
{ K_PAGEDOWN, nv_page, NV_SSS|NV_STS, FORWARD },
{ K_KPAGEDOWN, nv_page, NV_SSS|NV_STS, FORWARD },
{ K_END, nv_end, NV_SSS|NV_STS, false },
{ K_KEND, nv_end, NV_SSS|NV_STS, false },
{ K_S_END, nv_end, NV_SS, false },
{ K_C_END, nv_end, NV_SSS|NV_STS, true },
{ K_HOME, nv_home, NV_SSS|NV_STS, 0 },
{ K_KHOME, nv_home, NV_SSS|NV_STS, 0 },
{ K_S_HOME, nv_home, NV_SS, 0 },
{ K_C_HOME, nv_goto, NV_SSS|NV_STS, false },
{ K_DEL, nv_abbrev, 0, 0 },
{ K_KDEL, nv_abbrev, 0, 0 },
{ K_UNDO, nv_kundo, 0, 0 },
{ K_HELP, nv_help, NV_NCW, 0 },
{ K_F1, nv_help, NV_NCW, 0 },
{ K_XF1, nv_help, NV_NCW, 0 },
{ K_SELECT, nv_select, 0, 0 },
{ K_EVENT, nv_event, NV_KEEPREG, 0 },
{ K_COMMAND, nv_colon, 0, 0 },
};
#define NV_CMDS_SIZE ARRAY_SIZE(nv_cmds)
static short nv_cmd_idx[NV_CMDS_SIZE];
static int nv_max_linear;
static int nv_compare(const void *s1, const void *s2)
{
int c1, c2;
c1 = nv_cmds[*(const short *)s1].cmd_char;
c2 = nv_cmds[*(const short *)s2].cmd_char;
if (c1 < 0)
c1 = -c1;
if (c2 < 0)
c2 = -c2;
return c1 - c2;
}
void init_normal_cmds(void)
{
assert(NV_CMDS_SIZE <= SHRT_MAX);
for (short int i = 0; i < (short int)NV_CMDS_SIZE; ++i) {
nv_cmd_idx[i] = i;
}
qsort(&nv_cmd_idx, NV_CMDS_SIZE, sizeof(short), nv_compare);
short int i;
for (i = 0; i < (short int)NV_CMDS_SIZE; ++i) {
if (i != nv_cmds[nv_cmd_idx[i]].cmd_char) {
break;
}
}
nv_max_linear = i - 1;
}
static int find_command(int cmdchar)
{
int i;
int idx;
int top, bot;
int c;
if (cmdchar >= 0x100)
return -1;
if (cmdchar < 0)
cmdchar = -cmdchar;
assert(nv_max_linear < (int)NV_CMDS_SIZE);
if (cmdchar <= nv_max_linear)
return nv_cmd_idx[cmdchar];
bot = nv_max_linear + 1;
top = NV_CMDS_SIZE - 1;
idx = -1;
while (bot <= top) {
i = (top + bot) / 2;
c = nv_cmds[nv_cmd_idx[i]].cmd_char;
if (c < 0)
c = -c;
if (cmdchar == c) {
idx = nv_cmd_idx[i];
break;
}
if (cmdchar > c)
bot = i + 1;
else
top = i - 1;
}
return idx;
}
void normal_enter(bool cmdwin, bool noexmode)
{
NormalState state;
normal_state_init(&state);
state.cmdwin = cmdwin;
state.noexmode = noexmode;
state.toplevel = (!cmdwin || cmdwin_result == 0) && !noexmode;
state_enter(&state.state);
}
static void normal_prepare(NormalState *s)
{
memset(&s->ca, 0, sizeof(s->ca)); 
s->ca.oap = &s->oa;
s->ca.opcount = opcount;
int c = finish_op;
finish_op = (s->oa.op_type != OP_NOP);
if (finish_op != c) {
ui_cursor_shape(); 
}
if (!finish_op && !s->oa.regname) {
s->ca.opcount = 0;
s->set_prevcount = true;
}
if (s->oa.prev_opcount > 0 || s->oa.prev_count0 > 0) {
s->ca.opcount = s->oa.prev_opcount;
s->ca.count0 = s->oa.prev_count0;
s->oa.prev_opcount = 0;
s->oa.prev_count0 = 0;
}
s->mapped_len = typebuf_maplen();
State = NORMAL_BUSY;
if (s->toplevel && readbuf1_empty()) {
set_vcount_ca(&s->ca, &s->set_prevcount);
}
}
static bool normal_handle_special_visual_command(NormalState *s)
{
if (km_stopsel
&& (nv_cmds[s->idx].cmd_flags & NV_STS)
&& !(mod_mask & MOD_MASK_SHIFT)) {
end_visual_mode();
redraw_curbuf_later(INVERTED);
}
if (km_startsel) {
if (nv_cmds[s->idx].cmd_flags & NV_SS) {
unshift_special(&s->ca);
s->idx = find_command(s->ca.cmdchar);
if (s->idx < 0) {
clearopbeep(&s->oa);
return true;
}
} else if ((nv_cmds[s->idx].cmd_flags & NV_SSS)
&& (mod_mask & MOD_MASK_SHIFT)) {
mod_mask &= ~MOD_MASK_SHIFT;
}
}
return false;
}
static bool normal_need_additional_char(NormalState *s)
{
int flags = nv_cmds[s->idx].cmd_flags;
bool pending_op = s->oa.op_type != OP_NOP;
int cmdchar = s->ca.cmdchar;
return flags & NV_NCH && (
((flags & NV_NCH_NOP) == NV_NCH_NOP && !pending_op)
|| (flags & NV_NCH_ALW) == NV_NCH_ALW
|| (cmdchar == 'q'
&& !pending_op
&& reg_recording == 0
&& reg_executing == 0)
|| ((cmdchar == 'a' || cmdchar == 'i') && (pending_op || VIsual_active)));
}
static bool normal_need_redraw_mode_message(NormalState *s)
{
return (
((p_smd && msg_silent == 0
&& (restart_edit != 0 || (VIsual_active
&& s->old_pos.lnum == curwin->w_cursor.lnum
&& s->old_pos.col == curwin->w_cursor.col))
&& (clear_cmdline || redraw_cmdline)
&& (msg_didout || (msg_didany && msg_scroll))
&& !msg_nowait
&& KeyTyped)
|| (restart_edit != 0 && !VIsual_active && msg_scroll
&& emsg_on_display))
&& s->oa.regname == 0
&& !(s->ca.retval & CA_COMMAND_BUSY)
&& stuff_empty()
&& typebuf_typed()
&& emsg_silent == 0
&& !did_wait_return
&& s->oa.op_type == OP_NOP);
}
static void normal_redraw_mode_message(NormalState *s)
{
int save_State = State;
if (restart_edit != 0) {
State = INSERT;
}
if (must_redraw && keep_msg != NULL && !emsg_on_display) {
char_u *kmsg;
kmsg = keep_msg;
keep_msg = NULL;
update_screen(0);
keep_msg = kmsg;
msg_attr((const char *)kmsg, keep_msg_attr);
xfree(kmsg);
}
setcursor();
ui_flush();
if (msg_scroll || emsg_on_display) {
os_delay(1000L, true); 
}
os_delay(3000L, false); 
State = save_State;
msg_scroll = false;
emsg_on_display = false;
}
static void normal_get_additional_char(NormalState *s)
{
int *cp;
bool repl = false; 
bool lit = false; 
bool langmap_active = false; 
int lang; 
no_mapping++;
did_cursorhold = true;
if (s->ca.cmdchar == 'g') {
s->ca.nchar = plain_vgetc();
LANGMAP_ADJUST(s->ca.nchar, true);
s->need_flushbuf |= add_to_showcmd(s->ca.nchar);
if (s->ca.nchar == 'r' || s->ca.nchar == '\'' || s->ca.nchar == '`'
|| s->ca.nchar == Ctrl_BSL) {
cp = &s->ca.extra_char; 
if (s->ca.nchar != 'r') {
lit = true; 
} else {
repl = true; 
}
} else {
cp = NULL; 
}
} else {
if (s->ca.cmdchar == 'r') {
repl = true;
}
cp = &s->ca.nchar;
}
lang = (repl || (nv_cmds[s->idx].cmd_flags & NV_LANG));
if (cp != NULL) {
if (repl) {
State = REPLACE; 
ui_cursor_shape(); 
}
if (lang && curbuf->b_p_iminsert == B_IMODE_LMAP) {
no_mapping--;
if (repl) {
State = LREPLACE;
} else {
State = LANGMAP;
}
langmap_active = true;
}
*cp = plain_vgetc();
if (langmap_active) {
no_mapping++;
}
State = NORMAL_BUSY;
s->need_flushbuf |= add_to_showcmd(*cp);
if (!lit) {
if (*cp == Ctrl_K && ((nv_cmds[s->idx].cmd_flags & NV_LANG)
|| cp == &s->ca.extra_char)
&& vim_strchr(p_cpo, CPO_DIGRAPH) == NULL) {
s->c = get_digraph(false);
if (s->c > 0) {
*cp = s->c;
del_from_showcmd(3);
s->need_flushbuf |= add_to_showcmd(*cp);
}
}
LANGMAP_ADJUST(*cp, !lang);
if (p_hkmap && lang && KeyTyped) {
*cp = hkmap(*cp);
}
}
if (cp == &s->ca.extra_char
&& s->ca.nchar == Ctrl_BSL
&& (s->ca.extra_char == Ctrl_N || s->ca.extra_char == Ctrl_G)) {
s->ca.cmdchar = Ctrl_BSL;
s->ca.nchar = s->ca.extra_char;
s->idx = find_command(s->ca.cmdchar);
} else if ((s->ca.nchar == 'n' || s->ca.nchar == 'N')
&& s->ca.cmdchar == 'g') {
s->ca.oap->op_type = get_op_type(*cp, NUL);
} else if (*cp == Ctrl_BSL) {
long towait = (p_ttm >= 0 ? p_ttm : p_tm);
while ((s->c = vpeekc()) <= 0 && towait > 0L) {
do_sleep(towait > 50L ? 50L : towait);
towait -= 50L;
}
if (s->c > 0) {
s->c = plain_vgetc();
if (s->c != Ctrl_N && s->c != Ctrl_G) {
vungetc(s->c);
} else {
s->ca.cmdchar = Ctrl_BSL;
s->ca.nchar = s->c;
s->idx = find_command(s->ca.cmdchar);
assert(s->idx >= 0);
}
}
}
no_mapping--;
while (lang && (s->c = vpeekc()) > 0
&& (s->c >= 0x100 || MB_BYTE2LEN(vpeekc()) > 1)) {
s->c = plain_vgetc();
if (!utf_iscomposing(s->c)) {
vungetc(s->c); 
break;
} else if (s->ca.ncharC1 == 0) {
s->ca.ncharC1 = s->c;
} else {
s->ca.ncharC2 = s->c;
}
}
no_mapping++;
}
no_mapping--;
}
static void normal_invert_horizontal(NormalState *s)
{
switch (s->ca.cmdchar) {
case 'l': s->ca.cmdchar = 'h'; break;
case K_RIGHT: s->ca.cmdchar = K_LEFT; break;
case K_S_RIGHT: s->ca.cmdchar = K_S_LEFT; break;
case K_C_RIGHT: s->ca.cmdchar = K_C_LEFT; break;
case 'h': s->ca.cmdchar = 'l'; break;
case K_LEFT: s->ca.cmdchar = K_RIGHT; break;
case K_S_LEFT: s->ca.cmdchar = K_S_RIGHT; break;
case K_C_LEFT: s->ca.cmdchar = K_C_RIGHT; break;
case '>': s->ca.cmdchar = '<'; break;
case '<': s->ca.cmdchar = '>'; break;
}
s->idx = find_command(s->ca.cmdchar);
}
static bool normal_get_command_count(NormalState *s)
{
if (VIsual_active && VIsual_select) {
return false;
}
while ((s->c >= '1' && s->c <= '9') || (s->ca.count0 != 0
&& (s->c == K_DEL || s->c == K_KDEL || s->c == '0'))) {
if (s->c == K_DEL || s->c == K_KDEL) {
s->ca.count0 /= 10;
del_from_showcmd(4); 
} else {
s->ca.count0 = s->ca.count0 * 10 + (s->c - '0');
}
if (s->ca.count0 < 0) {
s->ca.count0 = 999999999L;
}
if (s->toplevel && readbuf1_empty()) {
set_vcount_ca(&s->ca, &s->set_prevcount);
}
if (s->ctrl_w) {
no_mapping++;
}
++no_zero_mapping; 
s->c = plain_vgetc();
LANGMAP_ADJUST(s->c, true);
--no_zero_mapping;
if (s->ctrl_w) {
no_mapping--;
}
s->need_flushbuf |= add_to_showcmd(s->c);
}
if (s->c == Ctrl_W && !s->ctrl_w && s->oa.op_type == OP_NOP) {
s->ctrl_w = true;
s->ca.opcount = s->ca.count0; 
s->ca.count0 = 0;
no_mapping++;
s->c = plain_vgetc(); 
LANGMAP_ADJUST(s->c, true);
no_mapping--;
s->need_flushbuf |= add_to_showcmd(s->c);
return true;
}
return false;
}
static void normal_finish_command(NormalState *s)
{
if (s->command_finished) {
goto normal_end;
}
if (!finish_op
&& !s->oa.op_type
&& (s->idx < 0 || !(nv_cmds[s->idx].cmd_flags & NV_KEEPREG))) {
clearop(&s->oa);
set_reg_var(get_default_register_name());
}
if (s->old_mapped_len > 0) {
s->old_mapped_len = typebuf_maplen();
}
if (s->ca.cmdchar != K_IGNORE) {
do_pending_operator(&s->ca, s->old_col, false);
}
if (normal_need_redraw_mode_message(s)) {
normal_redraw_mode_message(s);
}
normal_end:
msg_nowait = false;
s->c = finish_op;
finish_op = false;
if (s->c || s->ca.cmdchar == 'r') {
ui_cursor_shape(); 
}
if (s->oa.op_type == OP_NOP && s->oa.regname == 0
&& s->ca.cmdchar != K_EVENT) {
clear_showcmd();
}
checkpcmark(); 
xfree(s->ca.searchbuf);
mb_check_adjust_col(curwin); 
if (curwin->w_p_scb && s->toplevel) {
validate_cursor(); 
do_check_scrollbind(true);
}
if (curwin->w_p_crb && s->toplevel) {
validate_cursor(); 
do_check_cursorbind();
}
if (s->oa.op_type == OP_NOP
&& ((restart_edit != 0 && !VIsual_active && s->old_mapped_len == 0)
|| restart_VIsual_select == 1)
&& !(s->ca.retval & CA_COMMAND_BUSY)
&& stuff_empty()
&& s->oa.regname == 0) {
if (restart_VIsual_select == 1) {
VIsual_select = true;
showmode();
restart_VIsual_select = 0;
}
if (restart_edit != 0 && !VIsual_active && s->old_mapped_len == 0) {
(void)edit(restart_edit, false, 1L);
}
}
if (restart_VIsual_select == 2) {
restart_VIsual_select = 1;
}
opcount = s->ca.opcount;
}
static int normal_execute(VimState *state, int key)
{
NormalState *s = (NormalState *)state;
s->command_finished = false;
s->ctrl_w = false; 
s->old_col = curwin->w_curswant;
s->c = key;
LANGMAP_ADJUST(s->c, get_real_state() != SELECTMODE);
if (restart_edit == 0) {
s->old_mapped_len = 0;
} else if (s->old_mapped_len || (VIsual_active && s->mapped_len == 0
&& typebuf_maplen() > 0)) {
s->old_mapped_len = typebuf_maplen();
}
if (s->c == NUL) {
s->c = K_ZERO;
}
if (VIsual_active && VIsual_select && (vim_isprintc(s->c)
|| s->c == NL || s->c == CAR || s->c == K_KENTER)) {
ins_char_typebuf(s->c);
if (restart_edit != 0) {
s->c = 'd';
} else {
s->c = 'c';
}
msg_nowait = true; 
s->old_mapped_len = 0; 
}
s->need_flushbuf = add_to_showcmd(s->c);
while (normal_get_command_count(s)) continue;
if (s->c == K_EVENT) {
s->oa.prev_opcount = s->ca.opcount;
s->oa.prev_count0 = s->ca.count0;
} else if (s->ca.opcount != 0) {
if (s->ca.count0) {
s->ca.count0 *= s->ca.opcount;
} else {
s->ca.count0 = s->ca.opcount;
}
}
s->ca.opcount = s->ca.count0;
s->ca.count1 = (s->ca.count0 == 0 ? 1 : s->ca.count0);
if (s->toplevel && readbuf1_empty()) {
set_vcount(s->ca.count0, s->ca.count1, s->set_prevcount);
}
if (s->ctrl_w) {
s->ca.nchar = s->c;
s->ca.cmdchar = Ctrl_W;
} else {
s->ca.cmdchar = s->c;
}
s->idx = find_command(s->ca.cmdchar);
if (s->idx < 0) {
clearopbeep(&s->oa);
s->command_finished = true;
goto finish;
}
if (text_locked() && (nv_cmds[s->idx].cmd_flags & NV_NCW)) {
clearopbeep(&s->oa);
text_locked_msg();
s->command_finished = true;
goto finish;
}
if ((nv_cmds[s->idx].cmd_flags & NV_NCW) && curbuf_locked()) {
s->command_finished = true;
goto finish;
}
if (VIsual_active && normal_handle_special_visual_command(s)) {
s->command_finished = true;
goto finish;
}
if (curwin->w_p_rl && KeyTyped && !KeyStuffed
&& (nv_cmds[s->idx].cmd_flags & NV_RL)) {
normal_invert_horizontal(s);
}
if (normal_need_additional_char(s)) {
normal_get_additional_char(s);
}
if (s->need_flushbuf) {
ui_flush();
}
if (s->ca.cmdchar != K_IGNORE && s->ca.cmdchar != K_EVENT) {
did_cursorhold = false;
}
State = NORMAL;
if (s->ca.nchar == ESC) {
clearop(&s->oa);
if (restart_edit == 0 && goto_im()) {
restart_edit = 'a';
}
s->command_finished = true;
goto finish;
}
if (s->ca.cmdchar != K_IGNORE) {
msg_didout = false; 
msg_col = 0;
}
s->old_pos = curwin->w_cursor; 
if (!VIsual_active && km_startsel) {
if (nv_cmds[s->idx].cmd_flags & NV_SS) {
start_selection();
unshift_special(&s->ca);
s->idx = find_command(s->ca.cmdchar);
assert(s->idx >= 0);
} else if ((nv_cmds[s->idx].cmd_flags & NV_SSS)
&& (mod_mask & MOD_MASK_SHIFT)) {
start_selection();
mod_mask &= ~MOD_MASK_SHIFT;
}
}
s->ca.arg = nv_cmds[s->idx].cmd_arg;
(nv_cmds[s->idx].cmd_func)(&s->ca);
finish:
normal_finish_command(s);
return 1;
}
static void normal_check_stuff_buffer(NormalState *s)
{
if (stuff_empty()) {
did_check_timestamps = false;
if (need_check_timestamps) {
check_timestamps(false);
}
if (need_wait_return) {
wait_return(false);
}
if (need_start_insertmode && goto_im() && !VIsual_active) {
need_start_insertmode = false;
stuffReadbuff("i"); 
need_fileinfo = false;
}
}
}
static void normal_check_interrupt(NormalState *s)
{
if (got_int) {
if (s->noexmode && global_busy && !exmode_active
&& s->previous_got_int) {
exmode_active = EXMODE_NORMAL;
State = NORMAL;
} else if (!global_busy || !exmode_active) {
if (!quit_more) {
(void)vgetc();
}
got_int = false;
}
s->previous_got_int = true;
} else {
s->previous_got_int = false;
}
}
static void normal_check_cursor_moved(NormalState *s)
{
if (!finish_op && (has_event(EVENT_CURSORMOVED) || curwin->w_p_cole > 0)
&& !equalpos(curwin->w_last_cursormoved, curwin->w_cursor)) {
if (has_event(EVENT_CURSORMOVED)) {
apply_autocmds(EVENT_CURSORMOVED, NULL, NULL, false, curbuf);
}
curwin->w_last_cursormoved = curwin->w_cursor;
}
}
static void normal_check_text_changed(NormalState *s)
{
if (!finish_op && has_event(EVENT_TEXTCHANGED)
&& curbuf->b_last_changedtick != buf_get_changedtick(curbuf)) {
apply_autocmds(EVENT_TEXTCHANGED, NULL, NULL, false, curbuf);
curbuf->b_last_changedtick = buf_get_changedtick(curbuf);
}
}
static void normal_check_folds(NormalState *s)
{
foldAdjustVisual();
if (hasAnyFolding(curwin) && !char_avail()) {
foldCheckClose();
if (fdo_flags & FDO_ALL) {
foldOpenCursor();
}
}
}
static void normal_redraw(NormalState *s)
{
update_topline();
validate_cursor();
if (curwin->w_p_cole > 0 && conceal_cursor_line(curwin)) {
redrawWinline(curwin, curwin->w_cursor.lnum);
}
if (VIsual_active) {
update_curbuf(INVERTED); 
} else if (must_redraw) {
update_screen(0);
} else if (redraw_cmdline || clear_cmdline) {
showmode();
}
redraw_statuslines();
if (need_maketitle) {
maketitle();
}
if (keep_msg != NULL && !msg_ext_is_visible()) {
char *p = (char *)keep_msg;
msg_attr(p, keep_msg_attr);
xfree(p);
}
if (need_fileinfo && !shortmess(SHM_FILEINFO)) {
fileinfo(false, true, false);
need_fileinfo = false;
}
emsg_on_display = false; 
did_emsg = false;
msg_didany = false; 
may_clear_sb_text(); 
showruler(false);
setcursor();
}
static int normal_check(VimState *state)
{
NormalState *s = (NormalState *)state;
normal_check_stuff_buffer(s);
normal_check_interrupt(s);
if (!exmode_active) {
msg_scroll = false;
}
quit_more = false;
if (skip_redraw || exmode_active) {
skip_redraw = false;
} else if (do_redraw || stuff_empty()) {
normal_check_cursor_moved(s);
normal_check_text_changed(s);
if (curtab->tp_diff_update || curtab->tp_diff_invalid) {
ex_diffupdate(NULL);
curtab->tp_diff_update = false;
}
if (diff_need_scrollbind) {
check_scrollbind((linenr_T)0, 0L);
diff_need_scrollbind = false;
}
normal_check_folds(s);
normal_redraw(s);
do_redraw = false;
if (time_fd != NULL) {
TIME_MSG("first screen update");
TIME_MSG("--- NVIM STARTED ---");
fclose(time_fd);
time_fd = NULL;
}
}
may_garbage_collect = !s->cmdwin && !s->noexmode;
update_curswant();
if (exmode_active) {
if (s->noexmode) {
return 0;
}
do_exmode(exmode_active == EXMODE_VIM);
return -1;
}
if (s->cmdwin && cmdwin_result != 0) {
return 0;
}
normal_prepare(s);
return 1;
}
static void set_vcount_ca(cmdarg_T *cap, bool *set_prevcount)
{
long count = cap->count0;
if (cap->opcount != 0)
count = cap->opcount * (count == 0 ? 1 : count);
set_vcount(count, count == 0 ? 1 : count, *set_prevcount);
*set_prevcount = false; 
}
void do_pending_operator(cmdarg_T *cap, int old_col, bool gui_yank)
{
oparg_T *oap = cap->oap;
pos_T old_cursor;
bool empty_region_error;
int restart_edit_save;
int lbr_saved = curwin->w_p_lbr;
static int redo_VIsual_mode = NUL; 
static linenr_T redo_VIsual_line_count; 
static colnr_T redo_VIsual_vcol; 
static long redo_VIsual_count; 
static int redo_VIsual_arg; 
bool include_line_break = false;
old_cursor = curwin->w_cursor;
if ((finish_op
|| VIsual_active)
&& oap->op_type != OP_NOP) {
const bool redo_yank = vim_strchr(p_cpo, CPO_YANK) != NULL && !gui_yank;
if (curwin->w_p_lbr) {
curwin->w_valid &= ~VALID_VIRTCOL;
}
curwin->w_p_lbr = false;
oap->is_VIsual = VIsual_active;
if (oap->motion_force == 'V') {
oap->motion_type = kMTLineWise;
} else if (oap->motion_force == 'v') {
if (oap->motion_type == kMTLineWise) {
oap->inclusive = false;
} else if (oap->motion_type == kMTCharWise) {
oap->inclusive = !oap->inclusive;
}
oap->motion_type = kMTCharWise;
} else if (oap->motion_force == Ctrl_V) {
if (!VIsual_active) {
VIsual_active = true;
VIsual = oap->start;
}
VIsual_mode = Ctrl_V;
VIsual_select = false;
VIsual_reselect = false;
}
if ((redo_yank || oap->op_type != OP_YANK)
&& ((!VIsual_active || oap->motion_force)
|| (cap->cmdchar == ':' && oap->op_type != OP_COLON))
&& cap->cmdchar != 'D'
&& oap->op_type != OP_FOLD
&& oap->op_type != OP_FOLDOPEN
&& oap->op_type != OP_FOLDOPENREC
&& oap->op_type != OP_FOLDCLOSE
&& oap->op_type != OP_FOLDCLOSEREC
&& oap->op_type != OP_FOLDDEL
&& oap->op_type != OP_FOLDDELREC
) {
prep_redo(oap->regname, cap->count0,
get_op_char(oap->op_type), get_extra_op_char(oap->op_type),
oap->motion_force, cap->cmdchar, cap->nchar);
if (cap->cmdchar == '/' || cap->cmdchar == '?') { 
if (vim_strchr(p_cpo, CPO_REDO) == NULL) {
AppendToRedobuffLit(cap->searchbuf, -1);
}
AppendToRedobuff(NL_STR);
} else if (cap->cmdchar == ':' || cap->cmdchar == K_COMMAND) {
if (repeat_cmdline == NULL) {
ResetRedobuff();
} else {
AppendToRedobuffLit(repeat_cmdline, -1);
AppendToRedobuff(NL_STR);
XFREE_CLEAR(repeat_cmdline);
}
}
}
if (redo_VIsual_busy) {
oap->start = curwin->w_cursor;
curwin->w_cursor.lnum += redo_VIsual_line_count - 1;
if (curwin->w_cursor.lnum > curbuf->b_ml.ml_line_count)
curwin->w_cursor.lnum = curbuf->b_ml.ml_line_count;
VIsual_mode = redo_VIsual_mode;
if (redo_VIsual_vcol == MAXCOL || VIsual_mode == 'v') {
if (VIsual_mode == 'v') {
if (redo_VIsual_line_count <= 1) {
validate_virtcol();
curwin->w_curswant =
curwin->w_virtcol + redo_VIsual_vcol - 1;
} else
curwin->w_curswant = redo_VIsual_vcol;
} else {
curwin->w_curswant = MAXCOL;
}
coladvance(curwin->w_curswant);
}
cap->count0 = redo_VIsual_count;
cap->count1 = (cap->count0 == 0 ? 1 : cap->count0);
} else if (VIsual_active) {
if (!gui_yank) {
curbuf->b_visual.vi_start = VIsual;
curbuf->b_visual.vi_end = curwin->w_cursor;
curbuf->b_visual.vi_mode = VIsual_mode;
if (VIsual_mode_orig != NUL) {
curbuf->b_visual.vi_mode = VIsual_mode_orig;
VIsual_mode_orig = NUL;
}
curbuf->b_visual.vi_curswant = curwin->w_curswant;
curbuf->b_visual_mode_eval = VIsual_mode;
}
if (VIsual_select && VIsual_mode == 'V'
&& cap->oap->op_type != OP_DELETE) {
if (lt(VIsual, curwin->w_cursor)) {
VIsual.col = 0;
curwin->w_cursor.col =
(colnr_T)STRLEN(ml_get(curwin->w_cursor.lnum));
} else {
curwin->w_cursor.col = 0;
VIsual.col = (colnr_T)STRLEN(ml_get(VIsual.lnum));
}
VIsual_mode = 'v';
}
else if (VIsual_mode == 'v') {
include_line_break =
unadjust_for_sel();
}
oap->start = VIsual;
if (VIsual_mode == 'V') {
oap->start.col = 0;
oap->start.coladd = 0;
}
}
if (lt(oap->start, curwin->w_cursor)) {
if (!VIsual_active) {
if (hasFolding(oap->start.lnum, &oap->start.lnum, NULL))
oap->start.col = 0;
if ((curwin->w_cursor.col > 0
|| oap->inclusive
|| oap->motion_type == kMTLineWise)
&& hasFolding(curwin->w_cursor.lnum, NULL,
&curwin->w_cursor.lnum)) {
curwin->w_cursor.col = (colnr_T)STRLEN(get_cursor_line_ptr());
}
}
oap->end = curwin->w_cursor;
curwin->w_cursor = oap->start;
curwin->w_valid &= ~VALID_VIRTCOL;
} else {
if (!VIsual_active && oap->motion_type == kMTLineWise) {
if (hasFolding(curwin->w_cursor.lnum, &curwin->w_cursor.lnum,
NULL)) {
curwin->w_cursor.col = 0;
}
if (hasFolding(oap->start.lnum, NULL, &oap->start.lnum)) {
oap->start.col = (colnr_T)STRLEN(ml_get(oap->start.lnum));
}
}
oap->end = oap->start;
oap->start = curwin->w_cursor;
}
check_pos(curwin->w_buffer, &oap->end);
oap->line_count = oap->end.lnum - oap->start.lnum + 1;
virtual_op = virtual_active();
if (VIsual_active || redo_VIsual_busy) {
get_op_vcol(oap, redo_VIsual_vcol, true);
if (!redo_VIsual_busy && !gui_yank) {
resel_VIsual_mode = VIsual_mode;
if (curwin->w_curswant == MAXCOL)
resel_VIsual_vcol = MAXCOL;
else {
if (VIsual_mode != Ctrl_V)
getvvcol(curwin, &(oap->end),
NULL, NULL, &oap->end_vcol);
if (VIsual_mode == Ctrl_V || oap->line_count <= 1) {
if (VIsual_mode != Ctrl_V)
getvvcol(curwin, &(oap->start),
&oap->start_vcol, NULL, NULL);
resel_VIsual_vcol = oap->end_vcol - oap->start_vcol + 1;
} else
resel_VIsual_vcol = oap->end_vcol;
}
resel_VIsual_line_count = oap->line_count;
}
if ((redo_yank || oap->op_type != OP_YANK)
&& oap->op_type != OP_COLON
&& oap->op_type != OP_FOLD
&& oap->op_type != OP_FOLDOPEN
&& oap->op_type != OP_FOLDOPENREC
&& oap->op_type != OP_FOLDCLOSE
&& oap->op_type != OP_FOLDCLOSEREC
&& oap->op_type != OP_FOLDDEL
&& oap->op_type != OP_FOLDDELREC
&& oap->motion_force == NUL
) {
if (cap->cmdchar == 'g' && (cap->nchar == 'n'
|| cap->nchar == 'N')) {
prep_redo(oap->regname, cap->count0,
get_op_char(oap->op_type), get_extra_op_char(oap->op_type),
oap->motion_force, cap->cmdchar, cap->nchar);
} else if (cap->cmdchar != ':') {
int nchar = oap->op_type == OP_REPLACE ? cap->nchar : NUL;
if (nchar == REPLACE_CR_NCHAR) {
nchar = CAR;
} else if (nchar == REPLACE_NL_NCHAR) {
nchar = NL;
}
prep_redo(oap->regname, 0L, NUL, 'v', get_op_char(oap->op_type),
get_extra_op_char(oap->op_type), nchar);
}
if (!redo_VIsual_busy) {
redo_VIsual_mode = resel_VIsual_mode;
redo_VIsual_vcol = resel_VIsual_vcol;
redo_VIsual_line_count = resel_VIsual_line_count;
redo_VIsual_count = cap->count0;
redo_VIsual_arg = cap->arg;
}
}
if (oap->motion_force == NUL || oap->motion_type == kMTLineWise) {
oap->inclusive = true;
}
if (VIsual_mode == 'V') {
oap->motion_type = kMTLineWise;
} else if (VIsual_mode == 'v') {
oap->motion_type = kMTCharWise;
if (*ml_get_pos(&(oap->end)) == NUL
&& (include_line_break || !virtual_op)
) {
oap->inclusive = false;
if (*p_sel != 'o'
&& !op_on_lines(oap->op_type)
&& oap->end.lnum < curbuf->b_ml.ml_line_count) {
oap->end.lnum++;
oap->end.col = 0;
oap->end.coladd = 0;
oap->line_count++;
}
}
}
redo_VIsual_busy = false;
if (!gui_yank) {
VIsual_active = false;
setmouse();
mouse_dragging = 0;
may_clear_cmdline();
if ((oap->op_type == OP_YANK
|| oap->op_type == OP_COLON
|| oap->op_type == OP_FUNCTION
|| oap->op_type == OP_FILTER)
&& oap->motion_force == NUL) {
curwin->w_p_lbr = lbr_saved;
redraw_curbuf_later(INVERTED);
}
}
}
if (oap->inclusive) {
const int l = utfc_ptr2len(ml_get_pos(&oap->end));
if (l > 1) {
oap->end.col += l - 1;
}
}
curwin->w_set_curswant = true;
oap->empty = (oap->motion_type != kMTLineWise
&& (!oap->inclusive
|| (oap->op_type == OP_YANK
&& gchar_pos(&oap->end) == NUL))
&& equalpos(oap->start, oap->end)
&& !(virtual_op && oap->start.coladd != oap->end.coladd)
);
empty_region_error = (oap->empty
&& vim_strchr(p_cpo, CPO_EMPTYREGION) != NULL);
if (oap->is_VIsual && (oap->empty || !MODIFIABLE(curbuf)
|| oap->op_type == OP_FOLD
)) {
curwin->w_p_lbr = lbr_saved;
redraw_curbuf_later(INVERTED);
}
if (oap->motion_type == kMTCharWise
&& oap->inclusive == false
&& !(cap->retval & CA_NO_ADJ_OP_END)
&& oap->end.col == 0
&& (!oap->is_VIsual || *p_sel == 'o')
&& oap->line_count > 1) {
oap->end_adjusted = true; 
oap->line_count--;
oap->end.lnum--;
if (inindent(0)) {
oap->motion_type = kMTLineWise;
} else {
oap->end.col = (colnr_T)STRLEN(ml_get(oap->end.lnum));
if (oap->end.col) {
--oap->end.col;
oap->inclusive = true;
}
}
} else
oap->end_adjusted = false;
switch (oap->op_type) {
case OP_LSHIFT:
case OP_RSHIFT:
op_shift(oap, true,
oap->is_VIsual ? (int)cap->count1 :
1);
auto_format(false, true);
break;
case OP_JOIN_NS:
case OP_JOIN:
if (oap->line_count < 2)
oap->line_count = 2;
if (curwin->w_cursor.lnum + oap->line_count - 1 >
curbuf->b_ml.ml_line_count) {
beep_flush();
} else {
do_join((size_t)oap->line_count, oap->op_type == OP_JOIN,
true, true, true);
auto_format(false, true);
}
break;
case OP_DELETE:
VIsual_reselect = false; 
if (empty_region_error) {
vim_beep(BO_OPER);
CancelRedo();
} else {
(void)op_delete(oap);
if (oap->motion_type == kMTLineWise && has_format_option(FO_AUTO)) {
if (u_save_cursor() == FAIL) {
break;
}
}
auto_format(false, true);
}
break;
case OP_YANK:
if (empty_region_error) {
if (!gui_yank) {
vim_beep(BO_OPER);
CancelRedo();
}
} else {
curwin->w_p_lbr = lbr_saved;
(void)op_yank(oap, !gui_yank, false);
}
check_cursor_col();
break;
case OP_CHANGE:
VIsual_reselect = false; 
if (empty_region_error) {
vim_beep(BO_OPER);
CancelRedo();
} else {
if (p_im || !KeyTyped)
restart_edit_save = restart_edit;
else
restart_edit_save = 0;
restart_edit = 0;
curwin->w_p_lbr = lbr_saved;
finish_op = false;
if (op_change(oap)) 
cap->retval |= CA_COMMAND_BUSY;
if (restart_edit == 0)
restart_edit = restart_edit_save;
}
break;
case OP_FILTER:
if (vim_strchr(p_cpo, CPO_FILTER) != NULL) {
AppendToRedobuff("!\r"); 
} else {
bangredo = true; 
}
FALLTHROUGH;
case OP_INDENT:
case OP_COLON:
if (oap->op_type == OP_INDENT && *get_equalprg() == NUL) {
if (curbuf->b_p_lisp) {
op_reindent(oap, get_lisp_indent);
break;
}
op_reindent(oap,
*curbuf->b_p_inde != NUL ? get_expr_indent :
get_c_indent);
break;
}
op_colon(oap);
break;
case OP_TILDE:
case OP_UPPER:
case OP_LOWER:
case OP_ROT13:
if (empty_region_error) {
vim_beep(BO_OPER);
CancelRedo();
} else
op_tilde(oap);
check_cursor_col();
break;
case OP_FORMAT:
if (*curbuf->b_p_fex != NUL) {
op_formatexpr(oap); 
} else if (*p_fp != NUL || *curbuf->b_p_fp != NUL) {
op_colon(oap); 
} else {
op_format(oap, false); 
}
break;
case OP_FORMAT2:
op_format(oap, true); 
break;
case OP_FUNCTION:
curwin->w_p_lbr = lbr_saved;
op_function(oap); 
break;
case OP_INSERT:
case OP_APPEND:
VIsual_reselect = false; 
if (empty_region_error) {
vim_beep(BO_OPER);
CancelRedo();
} else {
restart_edit_save = restart_edit;
restart_edit = 0;
curwin->w_p_lbr = lbr_saved;
op_insert(oap, cap->count1);
curwin->w_p_lbr = false;
auto_format(false, true);
if (restart_edit == 0) {
restart_edit = restart_edit_save;
} else {
cap->retval |= CA_COMMAND_BUSY;
}
}
break;
case OP_REPLACE:
VIsual_reselect = false; 
if (empty_region_error) {
vim_beep(BO_OPER);
CancelRedo();
} else {
curwin->w_p_lbr = lbr_saved;
op_replace(oap, cap->nchar);
}
break;
case OP_FOLD:
VIsual_reselect = false; 
foldCreate(oap->start.lnum, oap->end.lnum);
break;
case OP_FOLDOPEN:
case OP_FOLDOPENREC:
case OP_FOLDCLOSE:
case OP_FOLDCLOSEREC:
VIsual_reselect = false; 
opFoldRange(oap->start.lnum, oap->end.lnum,
oap->op_type == OP_FOLDOPEN
|| oap->op_type == OP_FOLDOPENREC,
oap->op_type == OP_FOLDOPENREC
|| oap->op_type == OP_FOLDCLOSEREC,
oap->is_VIsual);
break;
case OP_FOLDDEL:
case OP_FOLDDELREC:
VIsual_reselect = false; 
deleteFold(oap->start.lnum, oap->end.lnum,
oap->op_type == OP_FOLDDELREC, oap->is_VIsual);
break;
case OP_NR_ADD:
case OP_NR_SUB:
if (empty_region_error) {
vim_beep(BO_OPER);
CancelRedo();
} else {
VIsual_active = true;
curwin->w_p_lbr = lbr_saved;
op_addsub(oap, cap->count1, redo_VIsual_arg);
VIsual_active = false;
}
check_cursor_col();
break;
default:
clearopbeep(oap);
}
virtual_op = kNone;
if (!gui_yank) {
if (!p_sol && oap->motion_type == kMTLineWise && !oap->end_adjusted
&& (oap->op_type == OP_LSHIFT || oap->op_type == OP_RSHIFT
|| oap->op_type == OP_DELETE)) {
curwin->w_p_lbr = false;
coladvance(curwin->w_curswant = old_col);
}
} else {
curwin->w_cursor = old_cursor;
}
clearop(oap);
motion_force = NUL;
}
curwin->w_p_lbr = lbr_saved;
}
static void op_colon(oparg_T *oap)
{
stuffcharReadbuff(':');
if (oap->is_VIsual) {
stuffReadbuff("'<,'>");
} else {
if (oap->start.lnum == curwin->w_cursor.lnum) {
stuffcharReadbuff('.');
} else {
stuffnumReadbuff((long)oap->start.lnum);
}
if (oap->end.lnum != oap->start.lnum) {
stuffcharReadbuff(',');
if (oap->end.lnum == curwin->w_cursor.lnum) {
stuffcharReadbuff('.');
} else if (oap->end.lnum == curbuf->b_ml.ml_line_count) {
stuffcharReadbuff('$');
} else if (oap->start.lnum == curwin->w_cursor.lnum) {
stuffReadbuff(".+");
stuffnumReadbuff(oap->line_count - 1);
} else {
stuffnumReadbuff((long)oap->end.lnum);
}
}
}
if (oap->op_type != OP_COLON) {
stuffReadbuff("!");
}
if (oap->op_type == OP_INDENT) {
stuffReadbuff((const char *)get_equalprg());
stuffReadbuff("\n");
} else if (oap->op_type == OP_FORMAT) {
if (*curbuf->b_p_fp != NUL) {
stuffReadbuff((const char *)curbuf->b_p_fp);
} else if (*p_fp != NUL) {
stuffReadbuff((const char *)p_fp);
} else {
stuffReadbuff("fmt");
}
stuffReadbuff("\n']");
}
}
static void op_function(const oparg_T *oap)
FUNC_ATTR_NONNULL_ALL
{
const TriState save_virtual_op = virtual_op;
if (*p_opfunc == NUL)
EMSG(_("E774: 'operatorfunc' is empty"));
else {
curbuf->b_op_start = oap->start;
curbuf->b_op_end = oap->end;
if (oap->motion_type != kMTLineWise && !oap->inclusive) {
decl(&curbuf->b_op_end);
}
typval_T argv[2];
argv[0].v_type = VAR_STRING;
argv[1].v_type = VAR_UNKNOWN;
argv[0].vval.v_string =
(char_u *)(((const char *const[]) {
[kMTBlockWise] = "block",
[kMTLineWise] = "line",
[kMTCharWise] = "char",
})[oap->motion_type]);
virtual_op = kNone;
(void)call_func_retnr(p_opfunc, 1, argv);
virtual_op = save_virtual_op;
}
}
static void move_tab_to_mouse(void)
{
int tabnr = tab_page_click_defs[mouse_col].tabnr;
if (tabnr <= 0) {
tabpage_move(9999);
} else if (tabnr < tabpage_index(curtab)) {
tabpage_move(tabnr - 1);
} else {
tabpage_move(tabnr);
}
}
bool
do_mouse (
oparg_T *oap, 
int c, 
int dir, 
long count,
bool fixindent 
)
{
static bool got_click = false; 
int which_button; 
bool is_click; 
bool is_drag; 
int jump_flags = 0; 
pos_T start_visual;
bool moved; 
bool in_status_line; 
static bool in_tab_line = false; 
bool in_sep_line; 
int c1, c2;
pos_T save_cursor;
win_T *old_curwin = curwin;
static pos_T orig_cursor;
colnr_T leftcol, rightcol;
pos_T end_visual;
long diff;
int old_active = VIsual_active;
int old_mode = VIsual_mode;
int regname;
save_cursor = curwin->w_cursor;
for (;; ) {
which_button = get_mouse_button(KEY2TERMCAP1(c), &is_click, &is_drag);
if (is_drag) {
if (vpeekc() != NUL) {
int nc;
int save_mouse_grid = mouse_grid;
int save_mouse_row = mouse_row;
int save_mouse_col = mouse_col;
nc = safe_vgetc();
if (c == nc)
continue;
vungetc(nc);
mouse_grid = save_mouse_grid;
mouse_row = save_mouse_row;
mouse_col = save_mouse_col;
}
}
break;
}
if (is_click)
got_click = true;
else {
if (!got_click) 
return false;
if (!is_drag) { 
got_click = false;
if (in_tab_line) {
in_tab_line = false;
return false;
}
}
}
if (is_click && (mod_mask & MOD_MASK_CTRL) && which_button == MOUSE_RIGHT) {
if (State & INSERT)
stuffcharReadbuff(Ctrl_O);
if (count > 1)
stuffnumReadbuff(count);
stuffcharReadbuff(Ctrl_T);
got_click = false; 
return false;
}
if ((mod_mask & MOD_MASK_CTRL) && which_button != MOUSE_LEFT)
return false;
if ((mod_mask & (MOD_MASK_SHIFT | MOD_MASK_CTRL | MOD_MASK_ALT
| MOD_MASK_META))
&& (!is_click
|| (mod_mask & MOD_MASK_MULTI_CLICK)
|| which_button == MOUSE_MIDDLE)
&& !((mod_mask & (MOD_MASK_SHIFT|MOD_MASK_ALT))
&& mouse_model_popup()
&& which_button == MOUSE_LEFT)
&& !((mod_mask & MOD_MASK_ALT)
&& !mouse_model_popup()
&& which_button == MOUSE_RIGHT)
)
return false;
if (!is_click && which_button == MOUSE_MIDDLE)
return false;
if (oap != NULL)
regname = oap->regname;
else
regname = 0;
if (which_button == MOUSE_MIDDLE) {
if (State == NORMAL) {
if (oap != NULL && oap->op_type != OP_NOP) {
clearopbeep(oap);
return false;
}
if (VIsual_active) {
if (VIsual_select) {
stuffcharReadbuff(Ctrl_G);
stuffReadbuff("\"+p");
} else {
stuffcharReadbuff('y');
stuffcharReadbuff(K_MIDDLEMOUSE);
}
return false;
}
} else if ((State & INSERT) == 0)
return false;
if ((State & INSERT) || !mouse_has(MOUSE_NORMAL)) {
if (regname == '.')
insert_reg(regname, true);
else {
if (regname == 0 && eval_has_provider("clipboard")) {
regname = '*';
}
if ((State & REPLACE_FLAG) && !yank_register_mline(regname)) {
insert_reg(regname, true);
} else {
do_put(regname, NULL, BACKWARD, 1L,
(fixindent ? PUT_FIXINDENT : 0) | PUT_CURSEND);
AppendCharToRedobuff(Ctrl_R);
AppendCharToRedobuff(fixindent ? Ctrl_P : Ctrl_O);
AppendCharToRedobuff(regname == 0 ? '"' : regname);
}
}
return false;
}
}
if (!is_click)
jump_flags |= MOUSE_FOCUS | MOUSE_DID_MOVE;
start_visual.lnum = 0;
if (mouse_row == 0 && firstwin->w_winrow > 0) {
if (is_drag) {
if (in_tab_line) {
move_tab_to_mouse();
}
return false;
}
if (is_click
&& cmdwin_type == 0
&& mouse_col < Columns) {
in_tab_line = true;
c1 = tab_page_click_defs[mouse_col].tabnr;
switch (tab_page_click_defs[mouse_col].type) {
case kStlClickDisabled: {
break;
}
case kStlClickTabClose: {
tabpage_T *tp;
if (c1 == 999) {
tp = curtab;
} else {
tp = find_tabpage(c1);
}
if (tp == curtab) {
if (first_tabpage->tp_next != NULL) {
tabpage_close(false);
}
} else if (tp != NULL) {
tabpage_close_other(tp, false);
}
break;
}
case kStlClickTabSwitch: {
if ((mod_mask & MOD_MASK_MULTI_CLICK) == MOD_MASK_2CLICK) {
end_visual_mode();
tabpage_new();
tabpage_move(c1 == 0 ? 9999 : c1 - 1);
} else {
goto_tabpage(c1);
if (curwin != old_curwin) {
end_visual_mode();
}
}
break;
}
case kStlClickFuncRun: {
typval_T argv[] = {
{
.v_lock = VAR_FIXED,
.v_type = VAR_NUMBER,
.vval = {
.v_number = (varnumber_T) tab_page_click_defs[mouse_col].tabnr
},
},
{
.v_lock = VAR_FIXED,
.v_type = VAR_NUMBER,
.vval = {
.v_number = (((mod_mask & MOD_MASK_MULTI_CLICK)
== MOD_MASK_4CLICK)
? 4
: ((mod_mask & MOD_MASK_MULTI_CLICK)
== MOD_MASK_3CLICK)
? 3
: ((mod_mask & MOD_MASK_MULTI_CLICK)
== MOD_MASK_2CLICK)
? 2
: 1)
},
},
{
.v_lock = VAR_FIXED,
.v_type = VAR_STRING,
.vval = { .v_string = (char_u *) (which_button == MOUSE_LEFT
? "l"
: which_button == MOUSE_RIGHT
? "r"
: which_button == MOUSE_MIDDLE
? "m"
: "?") },
},
{
.v_lock = VAR_FIXED,
.v_type = VAR_STRING,
.vval = {
.v_string = (char_u[]) {
(char_u) (mod_mask & MOD_MASK_SHIFT ? 's' : ' '),
(char_u) (mod_mask & MOD_MASK_CTRL ? 'c' : ' '),
(char_u) (mod_mask & MOD_MASK_ALT ? 'a' : ' '),
(char_u) (mod_mask & MOD_MASK_META ? 'm' : ' '),
NUL
}
},
}
};
typval_T rettv;
int doesrange;
(void)call_func((char_u *)tab_page_click_defs[mouse_col].func,
(int)strlen(tab_page_click_defs[mouse_col].func),
&rettv, ARRAY_SIZE(argv), argv, NULL,
curwin->w_cursor.lnum, curwin->w_cursor.lnum,
&doesrange, true, NULL, NULL);
tv_clear(&rettv);
break;
}
}
}
return true;
} else if (is_drag && in_tab_line) {
move_tab_to_mouse();
return false;
}
if (mouse_model_popup()) {
if (which_button == MOUSE_RIGHT
&& !(mod_mask & (MOD_MASK_SHIFT | MOD_MASK_CTRL))) {
return false;
}
if (which_button == MOUSE_LEFT
&& (mod_mask & (MOD_MASK_SHIFT|MOD_MASK_ALT))) {
which_button = MOUSE_RIGHT;
mod_mask &= ~MOD_MASK_SHIFT;
}
}
if ((State & (NORMAL | INSERT))
&& !(mod_mask & (MOD_MASK_SHIFT | MOD_MASK_CTRL))) {
if (which_button == MOUSE_LEFT) {
if (is_click) {
if (VIsual_active)
jump_flags |= MOUSE_MAY_STOP_VIS;
} else if (mouse_has(MOUSE_VISUAL))
jump_flags |= MOUSE_MAY_VIS;
} else if (which_button == MOUSE_RIGHT) {
if (is_click && VIsual_active) {
if (lt(curwin->w_cursor, VIsual)) {
start_visual = curwin->w_cursor;
end_visual = VIsual;
} else {
start_visual = VIsual;
end_visual = curwin->w_cursor;
}
}
jump_flags |= MOUSE_FOCUS;
if (mouse_has(MOUSE_VISUAL))
jump_flags |= MOUSE_MAY_VIS;
}
}
if (!is_drag && oap != NULL && oap->op_type != OP_NOP) {
got_click = false;
oap->motion_type = kMTCharWise;
}
if (!is_click && !is_drag)
jump_flags |= MOUSE_RELEASED;
jump_flags = jump_to_mouse(jump_flags,
oap == NULL ? NULL : &(oap->inclusive), which_button);
moved = (jump_flags & CURSOR_MOVED);
in_status_line = (jump_flags & IN_STATUS_LINE);
in_sep_line = (jump_flags & IN_SEP_LINE);
if (curwin != old_curwin && oap != NULL && oap->op_type != OP_NOP)
clearop(oap);
if (mod_mask == 0
&& !is_drag
&& (jump_flags & (MOUSE_FOLD_CLOSE | MOUSE_FOLD_OPEN))
&& which_button == MOUSE_LEFT) {
if (jump_flags & MOUSE_FOLD_OPEN)
openFold(curwin->w_cursor.lnum, 1L);
else
closeFold(curwin->w_cursor.lnum, 1L);
if (curwin == old_curwin)
curwin->w_cursor = save_cursor;
}
if (VIsual_active && is_drag && get_scrolloff_value()) {
if (mouse_row == 0) {
mouse_dragging = 2;
} else {
mouse_dragging = 1;
}
}
if (is_drag && mouse_row < 0 && !in_status_line) {
scroll_redraw(false, 1L);
mouse_row = 0;
}
if (start_visual.lnum) { 
if (mod_mask & MOD_MASK_ALT)
VIsual_mode = Ctrl_V;
if (VIsual_mode == Ctrl_V) {
getvcols(curwin, &start_visual, &end_visual, &leftcol, &rightcol);
if (curwin->w_curswant > (leftcol + rightcol) / 2)
end_visual.col = leftcol;
else
end_visual.col = rightcol;
if (curwin->w_cursor.lnum >=
(start_visual.lnum + end_visual.lnum) / 2) {
end_visual.lnum = start_visual.lnum;
}
start_visual = curwin->w_cursor; 
curwin->w_cursor = end_visual;
coladvance(end_visual.col);
VIsual = curwin->w_cursor;
curwin->w_cursor = start_visual; 
} else {
if (lt(curwin->w_cursor, start_visual))
VIsual = end_visual;
else if (lt(end_visual, curwin->w_cursor))
VIsual = start_visual;
else {
if (end_visual.lnum == start_visual.lnum) {
if (curwin->w_cursor.col - start_visual.col >
end_visual.col - curwin->w_cursor.col)
VIsual = start_visual;
else
VIsual = end_visual;
}
else {
diff = (curwin->w_cursor.lnum - start_visual.lnum) -
(end_visual.lnum - curwin->w_cursor.lnum);
if (diff > 0) 
VIsual = start_visual;
else if (diff < 0) 
VIsual = end_visual;
else { 
if (curwin->w_cursor.col <
(start_visual.col + end_visual.col) / 2)
VIsual = end_visual;
else
VIsual = start_visual;
}
}
}
}
}
else if ((State & INSERT) && VIsual_active)
stuffcharReadbuff(Ctrl_O);
if (which_button == MOUSE_MIDDLE) {
if (regname == 0 && eval_has_provider("clipboard")) {
regname = '*';
}
if (yank_register_mline(regname)) {
if (mouse_past_bottom)
dir = FORWARD;
} else if (mouse_past_eol)
dir = FORWARD;
if (fixindent) {
c1 = (dir == BACKWARD) ? '[' : ']';
c2 = 'p';
} else {
c1 = (dir == FORWARD) ? 'p' : 'P';
c2 = NUL;
}
prep_redo(regname, count, NUL, c1, NUL, c2, NUL);
if (restart_edit != 0)
where_paste_started = curwin->w_cursor;
do_put(regname, NULL, dir, count,
(fixindent ? PUT_FIXINDENT : 0)| PUT_CURSEND);
}
else if (((mod_mask & MOD_MASK_CTRL)
|| (mod_mask & MOD_MASK_MULTI_CLICK) == MOD_MASK_2CLICK)
&& bt_quickfix(curbuf)) {
if (curwin->w_llist_ref == NULL) { 
do_cmdline_cmd(".cc");
} else { 
do_cmdline_cmd(".ll");
}
got_click = false; 
}
else if ((mod_mask & MOD_MASK_CTRL) || (curbuf->b_help
&& (mod_mask &
MOD_MASK_MULTI_CLICK) ==
MOD_MASK_2CLICK)) {
if (State & INSERT)
stuffcharReadbuff(Ctrl_O);
stuffcharReadbuff(Ctrl_RSB);
got_click = false; 
}
else if ((mod_mask & MOD_MASK_SHIFT)) {
if (State & INSERT
|| (VIsual_active && VIsual_select)
)
stuffcharReadbuff(Ctrl_O);
if (which_button == MOUSE_LEFT)
stuffcharReadbuff('*');
else 
stuffcharReadbuff('#');
}
else if (in_status_line) {
} else if (in_sep_line) {
} else if ((mod_mask & MOD_MASK_MULTI_CLICK) && (State & (NORMAL | INSERT))
&& mouse_has(MOUSE_VISUAL)) {
if (is_click || !VIsual_active) {
if (VIsual_active) {
orig_cursor = VIsual;
} else {
VIsual = curwin->w_cursor;
orig_cursor = VIsual;
VIsual_active = true;
VIsual_reselect = true;
may_start_select('o');
setmouse();
}
if ((mod_mask & MOD_MASK_MULTI_CLICK) == MOD_MASK_2CLICK) {
if (mod_mask & MOD_MASK_ALT)
VIsual_mode = Ctrl_V;
else
VIsual_mode = 'v';
} else if ((mod_mask & MOD_MASK_MULTI_CLICK) == MOD_MASK_3CLICK)
VIsual_mode = 'V';
else if ((mod_mask & MOD_MASK_MULTI_CLICK) == MOD_MASK_4CLICK)
VIsual_mode = Ctrl_V;
}
if ((mod_mask & MOD_MASK_MULTI_CLICK) == MOD_MASK_2CLICK) {
pos_T *pos = NULL;
int gc;
if (is_click) {
end_visual = curwin->w_cursor;
while (gc = gchar_pos(&end_visual), ascii_iswhite(gc))
inc(&end_visual);
if (oap != NULL) {
oap->motion_type = kMTCharWise;
}
if (oap != NULL
&& VIsual_mode == 'v'
&& !vim_iswordc(gchar_pos(&end_visual))
&& equalpos(curwin->w_cursor, VIsual)
&& (pos = findmatch(oap, NUL)) != NULL) {
curwin->w_cursor = *pos;
if (oap->motion_type == kMTLineWise) {
VIsual_mode = 'V';
} else if (*p_sel == 'e') {
if (lt(curwin->w_cursor, VIsual)) {
VIsual.col++;
} else {
curwin->w_cursor.col++;
}
}
}
}
if (pos == NULL && (is_click || is_drag)) {
if (lt(curwin->w_cursor, orig_cursor)) {
find_start_of_word(&curwin->w_cursor);
find_end_of_word(&VIsual);
} else {
find_start_of_word(&VIsual);
if (*p_sel == 'e' && *get_cursor_pos_ptr() != NUL)
curwin->w_cursor.col +=
(*mb_ptr2len)(get_cursor_pos_ptr());
find_end_of_word(&curwin->w_cursor);
}
}
curwin->w_set_curswant = true;
}
if (is_click)
redraw_curbuf_later(INVERTED); 
} else if (VIsual_active && !old_active) {
if (mod_mask & MOD_MASK_ALT)
VIsual_mode = Ctrl_V;
else
VIsual_mode = 'v';
}
if ((!VIsual_active && old_active && mode_displayed)
|| (VIsual_active && p_smd && msg_silent == 0
&& (!old_active || VIsual_mode != old_mode)))
redraw_cmdline = true;
return moved;
}
static void find_start_of_word(pos_T *pos)
{
char_u *line;
int cclass;
int col;
line = ml_get(pos->lnum);
cclass = get_mouse_class(line + pos->col);
while (pos->col > 0) {
col = pos->col - 1;
col -= utf_head_off(line, line + col);
if (get_mouse_class(line + col) != cclass) {
break;
}
pos->col = col;
}
}
static void find_end_of_word(pos_T *pos)
{
char_u *line;
int cclass;
int col;
line = ml_get(pos->lnum);
if (*p_sel == 'e' && pos->col > 0) {
pos->col--;
pos->col -= utf_head_off(line, line + pos->col);
}
cclass = get_mouse_class(line + pos->col);
while (line[pos->col] != NUL) {
col = pos->col + (*mb_ptr2len)(line + pos->col);
if (get_mouse_class(line + col) != cclass) {
if (*p_sel == 'e')
pos->col = col;
break;
}
pos->col = col;
}
}
static int get_mouse_class(char_u *p)
{
if (MB_BYTE2LEN(p[0]) > 1) {
return mb_get_class(p);
}
const int c = *p;
if (c == ' ' || c == '\t') {
return 0;
}
if (vim_iswordc(c)) {
return 2;
}
if (c != NUL && vim_strchr((char_u *)"-+*/%<>&|^!=", c) != NULL)
return 1;
return c;
}
void end_visual_mode(void)
{
VIsual_active = false;
setmouse();
mouse_dragging = 0;
curbuf->b_visual.vi_mode = VIsual_mode;
curbuf->b_visual.vi_start = VIsual;
curbuf->b_visual.vi_end = curwin->w_cursor;
curbuf->b_visual.vi_curswant = curwin->w_curswant;
curbuf->b_visual_mode_eval = VIsual_mode;
if (!virtual_active())
curwin->w_cursor.coladd = 0;
may_clear_cmdline();
adjust_cursor_eol();
}
void reset_VIsual_and_resel(void)
{
if (VIsual_active) {
end_visual_mode();
redraw_curbuf_later(INVERTED); 
}
VIsual_reselect = false;
}
void reset_VIsual(void)
{
if (VIsual_active) {
end_visual_mode();
redraw_curbuf_later(INVERTED); 
VIsual_reselect = false;
}
}
static bool find_is_eval_item(
const char_u *const ptr,
int *const colp,
int *const bnp,
const int dir)
{
if ((*ptr == ']' && dir == BACKWARD) || (*ptr == '[' && dir == FORWARD)) {
*bnp += 1;
}
if (*bnp > 0) {
if ((*ptr == '[' && dir == BACKWARD) || (*ptr == ']' && dir == FORWARD)) {
*bnp -= 1;
}
return true;
}
if (*ptr == '.') {
return true;
}
if (ptr[dir == BACKWARD ? 0 : 1] == '>'
&& ptr[dir == BACKWARD ? -1 : 0] == '-') {
*colp += dir;
return true;
}
return false;
}
size_t find_ident_under_cursor(char_u **string, int find_type)
{
return find_ident_at_pos(curwin, curwin->w_cursor.lnum,
curwin->w_cursor.col, string, find_type);
}
size_t find_ident_at_pos(win_T *wp, linenr_T lnum, colnr_T startcol,
char_u **string, int find_type)
{
char_u *ptr;
int col = 0; 
int i;
int this_class = 0;
int prev_class;
int prevcol;
int bn = 0; 
ptr = ml_get_buf(wp->w_buffer, lnum, false);
for (i = (find_type & FIND_IDENT) ? 0 : 1; i < 2; ++i) {
col = startcol;
while (ptr[col] != NUL) {
if ((find_type & FIND_EVAL) && ptr[col] == ']') {
break;
}
this_class = mb_get_class(ptr + col);
if (this_class != 0 && (i == 1 || this_class != 1)) {
break;
}
col += utfc_ptr2len(ptr + col);
}
bn = ptr[col] == ']';
if ((find_type & FIND_EVAL) && ptr[col] == ']') {
this_class = mb_get_class((char_u *)"a");
} else {
this_class = mb_get_class(ptr + col);
}
while (col > 0 && this_class != 0) {
prevcol = col - 1 - utf_head_off(ptr, ptr + col - 1);
prev_class = mb_get_class(ptr + prevcol);
if (this_class != prev_class
&& (i == 0
|| prev_class == 0
|| (find_type & FIND_IDENT))
&& (!(find_type & FIND_EVAL)
|| prevcol == 0
|| !find_is_eval_item(ptr + prevcol, &prevcol, &bn, BACKWARD))) {
break;
}
col = prevcol;
}
if (this_class > 2) {
this_class = 2;
}
if (!(find_type & FIND_STRING) || this_class == 2) {
break;
}
}
if (ptr[col] == NUL || (i == 0 && this_class != 2)) {
if (find_type & FIND_STRING) {
EMSG(_("E348: No string under cursor"));
} else {
EMSG(_(e_noident));
}
return 0;
}
ptr += col;
*string = ptr;
bn = 0;
startcol -= col;
col = 0;
this_class = mb_get_class(ptr);
while (ptr[col] != NUL
&& ((i == 0
? mb_get_class(ptr + col) == this_class
: mb_get_class(ptr + col) != 0)
|| ((find_type & FIND_EVAL)
&& col <= (int)startcol
&& find_is_eval_item(ptr + col, &col, &bn, FORWARD)))) {
col += utfc_ptr2len(ptr + col);
}
assert(col >= 0);
return (size_t)col;
}
static void prep_redo_cmd(cmdarg_T *cap)
{
prep_redo(cap->oap->regname, cap->count0,
NUL, cap->cmdchar, NUL, NUL, cap->nchar);
}
static void prep_redo(int regname, long num, int cmd1, int cmd2, int cmd3, int cmd4, int cmd5)
{
ResetRedobuff();
if (regname != 0) { 
AppendCharToRedobuff('"');
AppendCharToRedobuff(regname);
}
if (num)
AppendNumberToRedobuff(num);
if (cmd1 != NUL)
AppendCharToRedobuff(cmd1);
if (cmd2 != NUL)
AppendCharToRedobuff(cmd2);
if (cmd3 != NUL)
AppendCharToRedobuff(cmd3);
if (cmd4 != NUL)
AppendCharToRedobuff(cmd4);
if (cmd5 != NUL)
AppendCharToRedobuff(cmd5);
}
static bool checkclearop(oparg_T *oap)
{
if (oap->op_type == OP_NOP)
return false;
clearopbeep(oap);
return true;
}
static bool checkclearopq(oparg_T *oap)
{
if (oap->op_type == OP_NOP
&& !VIsual_active
)
return false;
clearopbeep(oap);
return true;
}
static void clearop(oparg_T *oap)
{
oap->op_type = OP_NOP;
oap->regname = 0;
oap->motion_force = NUL;
oap->use_reg_one = false;
}
static void clearopbeep(oparg_T *oap)
{
clearop(oap);
beep_flush();
}
static void unshift_special(cmdarg_T *cap)
{
switch (cap->cmdchar) {
case K_S_RIGHT: cap->cmdchar = K_RIGHT; break;
case K_S_LEFT: cap->cmdchar = K_LEFT; break;
case K_S_UP: cap->cmdchar = K_UP; break;
case K_S_DOWN: cap->cmdchar = K_DOWN; break;
case K_S_HOME: cap->cmdchar = K_HOME; break;
case K_S_END: cap->cmdchar = K_END; break;
}
cap->cmdchar = simplify_key(cap->cmdchar, &mod_mask);
}
static void may_clear_cmdline(void)
{
if (mode_displayed) {
clear_cmdline = true;
} else {
clear_showcmd();
}
}
#define SHOWCMD_BUFLEN SHOWCMD_COLS + 1 + 30
static char_u showcmd_buf[SHOWCMD_BUFLEN];
static char_u old_showcmd_buf[SHOWCMD_BUFLEN]; 
static bool showcmd_is_clear = true;
static bool showcmd_visual = false;
void clear_showcmd(void)
{
if (!p_sc)
return;
if (VIsual_active && !char_avail()) {
int cursor_bot = lt(VIsual, curwin->w_cursor);
long lines;
colnr_T leftcol, rightcol;
linenr_T top, bot;
if (cursor_bot) {
top = VIsual.lnum;
bot = curwin->w_cursor.lnum;
} else {
top = curwin->w_cursor.lnum;
bot = VIsual.lnum;
}
(void)hasFolding(top, &top, NULL);
(void)hasFolding(bot, NULL, &bot);
lines = bot - top + 1;
if (VIsual_mode == Ctrl_V) {
char_u *saved_sbr = p_sbr;
p_sbr = empty_option;
getvcols(curwin, &curwin->w_cursor, &VIsual, &leftcol, &rightcol);
p_sbr = saved_sbr;
snprintf((char *)showcmd_buf, SHOWCMD_BUFLEN, "%" PRId64 "x%" PRId64,
(int64_t)lines, (int64_t)rightcol - leftcol + 1);
} else if (VIsual_mode == 'V' || VIsual.lnum != curwin->w_cursor.lnum) {
snprintf((char *)showcmd_buf, SHOWCMD_BUFLEN, "%" PRId64, (int64_t)lines);
} else {
char_u *s, *e;
int l;
int bytes = 0;
int chars = 0;
if (cursor_bot) {
s = ml_get_pos(&VIsual);
e = get_cursor_pos_ptr();
} else {
s = get_cursor_pos_ptr();
e = ml_get_pos(&VIsual);
}
while ((*p_sel != 'e') ? s <= e : s < e) {
l = (*mb_ptr2len)(s);
if (l == 0) {
++bytes;
++chars;
break; 
}
bytes += l;
++chars;
s += l;
}
if (bytes == chars)
sprintf((char *)showcmd_buf, "%d", chars);
else
sprintf((char *)showcmd_buf, "%d-%d", chars, bytes);
}
int limit = ui_has(kUIMessages) ? SHOWCMD_BUFLEN-1 : SHOWCMD_COLS;
showcmd_buf[limit] = NUL; 
showcmd_visual = true;
} else {
showcmd_buf[0] = NUL;
showcmd_visual = false;
if (showcmd_is_clear)
return;
}
display_showcmd();
}
bool add_to_showcmd(int c)
{
char_u *p;
int i;
static int ignore[] =
{
K_IGNORE,
K_LEFTMOUSE, K_LEFTDRAG, K_LEFTRELEASE,
K_MIDDLEMOUSE, K_MIDDLEDRAG, K_MIDDLERELEASE,
K_RIGHTMOUSE, K_RIGHTDRAG, K_RIGHTRELEASE,
K_MOUSEDOWN, K_MOUSEUP, K_MOUSELEFT, K_MOUSERIGHT,
K_X1MOUSE, K_X1DRAG, K_X1RELEASE, K_X2MOUSE, K_X2DRAG, K_X2RELEASE,
K_EVENT,
0
};
if (!p_sc || msg_silent != 0)
return false;
if (showcmd_visual) {
showcmd_buf[0] = NUL;
showcmd_visual = false;
}
if (IS_SPECIAL(c))
for (i = 0; ignore[i] != 0; ++i)
if (ignore[i] == c)
return false;
p = transchar(c);
if (*p == ' ')
STRCPY(p, "<20>");
size_t old_len = STRLEN(showcmd_buf);
size_t extra_len = STRLEN(p);
size_t limit = ui_has(kUIMessages) ? SHOWCMD_BUFLEN-1 : SHOWCMD_COLS;
if (old_len + extra_len > limit) {
size_t overflow = old_len + extra_len - limit;
memmove(showcmd_buf, showcmd_buf + overflow, old_len - overflow + 1);
}
STRCAT(showcmd_buf, p);
if (char_avail())
return false;
display_showcmd();
return true;
}
void add_to_showcmd_c(int c)
{
add_to_showcmd(c);
setcursor();
}
static void del_from_showcmd(int len)
{
int old_len;
if (!p_sc)
return;
old_len = (int)STRLEN(showcmd_buf);
if (len > old_len)
len = old_len;
showcmd_buf[old_len - len] = NUL;
if (!char_avail())
display_showcmd();
}
void push_showcmd(void)
{
if (p_sc)
STRCPY(old_showcmd_buf, showcmd_buf);
}
void pop_showcmd(void)
{
if (!p_sc)
return;
STRCPY(showcmd_buf, old_showcmd_buf);
display_showcmd();
}
static void display_showcmd(void)
{
int len;
len = (int)STRLEN(showcmd_buf);
showcmd_is_clear = (len == 0);
if (ui_has(kUIMessages)) {
Array content = ARRAY_DICT_INIT;
if (len > 0) {
Array chunk = ARRAY_DICT_INIT;
ADD(chunk, INTEGER_OBJ(0));
ADD(chunk, STRING_OBJ(cstr_to_string((char *)showcmd_buf)));
ADD(content, ARRAY_OBJ(chunk));
}
ui_call_msg_showcmd(content);
return;
}
msg_grid_validate();
int showcmd_row = Rows - 1;
grid_puts_line_start(&msg_grid_adj, showcmd_row);
if (!showcmd_is_clear) {
grid_puts(&msg_grid_adj, showcmd_buf, showcmd_row, sc_col,
HL_ATTR(HLF_MSG));
}
grid_puts(&msg_grid_adj, (char_u *)" " + len, showcmd_row,
sc_col + len, HL_ATTR(HLF_MSG));
grid_puts_line_flush(false);
}
void do_check_scrollbind(bool check)
{
static win_T *old_curwin = NULL;
static linenr_T old_topline = 0;
static int old_topfill = 0;
static buf_T *old_buf = NULL;
static colnr_T old_leftcol = 0;
if (check && curwin->w_p_scb) {
if (did_syncbind)
did_syncbind = false;
else if (curwin == old_curwin) {
if ((curwin->w_buffer == old_buf
|| curwin->w_p_diff
)
&& (curwin->w_topline != old_topline
|| curwin->w_topfill != old_topfill
|| curwin->w_leftcol != old_leftcol)) {
check_scrollbind(curwin->w_topline - old_topline,
(long)(curwin->w_leftcol - old_leftcol));
}
} else if (vim_strchr(p_sbo, 'j')) { 
check_scrollbind(curwin->w_topline - curwin->w_scbind_pos, 0L);
}
curwin->w_scbind_pos = curwin->w_topline;
}
old_curwin = curwin;
old_topline = curwin->w_topline;
old_topfill = curwin->w_topfill;
old_buf = curwin->w_buffer;
old_leftcol = curwin->w_leftcol;
}
void check_scrollbind(linenr_T topline_diff, long leftcol_diff)
{
bool want_ver;
bool want_hor;
win_T *old_curwin = curwin;
buf_T *old_curbuf = curbuf;
int old_VIsual_select = VIsual_select;
int old_VIsual_active = VIsual_active;
colnr_T tgt_leftcol = curwin->w_leftcol;
long topline;
long y;
want_ver = (vim_strchr(p_sbo, 'v') && topline_diff != 0);
want_ver |= old_curwin->w_p_diff;
want_hor = (vim_strchr(p_sbo, 'h') && (leftcol_diff || topline_diff != 0));
VIsual_select = VIsual_active = 0;
FOR_ALL_WINDOWS_IN_TAB(wp, curtab) {
curwin = wp;
curbuf = curwin->w_buffer;
if (curwin == old_curwin || !curwin->w_p_scb) {
continue;
}
if (want_ver) {
if (old_curwin->w_p_diff && curwin->w_p_diff) {
diff_set_topline(old_curwin, curwin);
} else {
curwin->w_scbind_pos += topline_diff;
topline = curwin->w_scbind_pos;
if (topline > curbuf->b_ml.ml_line_count)
topline = curbuf->b_ml.ml_line_count;
if (topline < 1)
topline = 1;
y = topline - curwin->w_topline;
if (y > 0)
scrollup(y, false);
else
scrolldown(-y, false);
}
redraw_later(VALID);
cursor_correct();
curwin->w_redr_status = true;
}
if (want_hor && curwin->w_leftcol != tgt_leftcol) {
curwin->w_leftcol = tgt_leftcol;
leftcol_changed();
}
}
VIsual_select = old_VIsual_select;
VIsual_active = old_VIsual_active;
curwin = old_curwin;
curbuf = old_curbuf;
}
static void nv_ignore(cmdarg_T *cap)
{
cap->retval |= CA_COMMAND_BUSY; 
}
static void nv_nop(cmdarg_T *cap)
{
}
static void nv_error(cmdarg_T *cap)
{
clearopbeep(cap->oap);
}
static void nv_help(cmdarg_T *cap)
{
if (!checkclearopq(cap->oap))
ex_help(NULL);
}
static void nv_addsub(cmdarg_T *cap)
{
if (bt_prompt(curbuf) && !prompt_curpos_editable()) {
clearopbeep(cap->oap);
} else if (!VIsual_active && cap->oap->op_type == OP_NOP) {
prep_redo_cmd(cap);
cap->oap->op_type = cap->cmdchar == Ctrl_A ? OP_NR_ADD : OP_NR_SUB;
op_addsub(cap->oap, cap->count1, cap->arg);
cap->oap->op_type = OP_NOP;
} else if (VIsual_active) {
nv_operator(cap);
} else {
clearop(cap->oap);
}
}
static void nv_page(cmdarg_T *cap)
{
if (!checkclearop(cap->oap)) {
if (mod_mask & MOD_MASK_CTRL) {
if (cap->arg == BACKWARD)
goto_tabpage(-(int)cap->count1);
else
goto_tabpage((int)cap->count0);
} else
(void)onepage(cap->arg, cap->count1);
}
}
static void
nv_gd (
oparg_T *oap,
int nchar,
int thisblock 
)
{
size_t len;
char_u *ptr;
if ((len = find_ident_under_cursor(&ptr, FIND_IDENT)) == 0
|| !find_decl(ptr, len, nchar == 'd', thisblock, SEARCH_START)) {
clearopbeep(oap);
} else if ((fdo_flags & FDO_SEARCH) && KeyTyped && oap->op_type == OP_NOP) {
foldOpenCursor();
}
}
static bool is_ident(char_u *line, int offset)
{
bool incomment = false;
int instring = 0;
int prev = 0;
for (int i = 0; i < offset && line[i] != NUL; i++) {
if (instring != 0) {
if (prev != '\\' && line[i] == instring) {
instring = 0;
}
} else if ((line[i] == '"' || line[i] == '\'') && !incomment) {
instring = line[i];
} else {
if (incomment) {
if (prev == '*' && line[i] == '/') {
incomment = false;
}
} else if (prev == '/' && line[i] == '*') {
incomment = true;
} else if (prev == '/' && line[i] == '/') {
return false;
}
}
prev = line[i];
}
return incomment == false && instring == 0;
}
bool
find_decl (
char_u *ptr,
size_t len,
bool locally,
bool thisblock,
int flags_arg 
)
{
char_u *pat;
pos_T old_pos;
pos_T par_pos;
pos_T found_pos;
bool t;
bool save_p_ws;
bool save_p_scs;
bool retval = true;
bool incll;
int searchflags = flags_arg;
pat = xmalloc(len + 7);
assert(len <= INT_MAX);
sprintf((char *)pat, vim_iswordp(ptr) ? "\\V\\<%.*s\\>" : "\\V%.*s",
(int)len, ptr);
old_pos = curwin->w_cursor;
save_p_ws = p_ws;
save_p_scs = p_scs;
p_ws = false; 
p_scs = false; 
if (!locally || !findpar(&incll, BACKWARD, 1L, '{', false)) {
setpcmark(); 
curwin->w_cursor.lnum = 1;
par_pos = curwin->w_cursor;
} else {
par_pos = curwin->w_cursor;
while (curwin->w_cursor.lnum > 1
&& *skipwhite(get_cursor_line_ptr()) != NUL)
--curwin->w_cursor.lnum;
}
curwin->w_cursor.col = 0;
clearpos(&found_pos);
for (;; ) {
t = searchit(curwin, curbuf, &curwin->w_cursor, NULL, FORWARD,
pat, 1L, searchflags, RE_LAST, NULL);
if (curwin->w_cursor.lnum >= old_pos.lnum) {
t = false; 
}
if (thisblock && t != false) {
const int64_t maxtravel = old_pos.lnum - curwin->w_cursor.lnum + 1;
const pos_T *pos = findmatchlimit(NULL, '}', FM_FORWARD, maxtravel);
if (pos != NULL && pos->lnum < old_pos.lnum) {
curwin->w_cursor = *pos;
continue;
}
}
if (t == false) {
if (found_pos.lnum != 0) {
curwin->w_cursor = found_pos;
t = true;
}
break;
}
if (get_leader_len(get_cursor_line_ptr(), NULL, false, true) > 0) {
++curwin->w_cursor.lnum;
curwin->w_cursor.col = 0;
continue;
}
bool valid = is_ident(get_cursor_line_ptr(), curwin->w_cursor.col);
if (!valid && found_pos.lnum != 0) {
curwin->w_cursor = found_pos;
break;
}
if (valid && !locally) {
break;
}
if (valid && curwin->w_cursor.lnum >= par_pos.lnum) {
if (found_pos.lnum != 0) {
curwin->w_cursor = found_pos;
}
break;
}
if (!valid) {
clearpos(&found_pos);
} else {
found_pos = curwin->w_cursor;
}
searchflags &= ~SEARCH_START;
}
if (t == false) {
retval = false;
curwin->w_cursor = old_pos;
} else {
curwin->w_set_curswant = true;
reset_search_dir();
}
xfree(pat);
p_ws = save_p_ws;
p_scs = save_p_scs;
return retval;
}
static bool nv_screengo(oparg_T *oap, int dir, long dist)
{
int linelen = linetabsize(get_cursor_line_ptr());
bool retval = true;
bool atend = false;
int n;
int col_off1; 
int col_off2; 
int width1; 
int width2; 
oap->motion_type = kMTCharWise;
oap->inclusive = (curwin->w_curswant == MAXCOL);
col_off1 = curwin_col_off();
col_off2 = col_off1 - curwin_col_off2();
width1 = curwin->w_width_inner - col_off1;
width2 = curwin->w_width_inner - col_off2;
if (width2 == 0) {
width2 = 1; 
}
if (curwin->w_width_inner != 0) {
if (curwin->w_curswant == MAXCOL) {
atend = true;
validate_virtcol();
if (width1 <= 0)
curwin->w_curswant = 0;
else {
curwin->w_curswant = width1 - 1;
if (curwin->w_virtcol > curwin->w_curswant)
curwin->w_curswant += ((curwin->w_virtcol
- curwin->w_curswant -
1) / width2 + 1) * width2;
}
} else {
if (linelen > width1)
n = ((linelen - width1 - 1) / width2 + 1) * width2 + width1;
else
n = width1;
if (curwin->w_curswant >= n) {
curwin->w_curswant = n - 1;
}
}
while (dist--) {
if (dir == BACKWARD) {
if (curwin->w_curswant >= width1) {
curwin->w_curswant -= width2;
} else {
if (curwin->w_cursor.lnum == 1) {
retval = false;
break;
}
--curwin->w_cursor.lnum;
if (!(fdo_flags & FDO_ALL))
(void)hasFolding(curwin->w_cursor.lnum,
&curwin->w_cursor.lnum, NULL);
linelen = linetabsize(get_cursor_line_ptr());
if (linelen > width1) {
int w = (((linelen - width1 - 1) / width2) + 1) * width2;
assert(curwin->w_curswant <= INT_MAX - w);
curwin->w_curswant += w;
}
}
} else { 
if (linelen > width1)
n = ((linelen - width1 - 1) / width2 + 1) * width2 + width1;
else
n = width1;
if (curwin->w_curswant + width2 < (colnr_T)n)
curwin->w_curswant += width2;
else {
(void)hasFolding(curwin->w_cursor.lnum, NULL,
&curwin->w_cursor.lnum);
if (curwin->w_cursor.lnum == curbuf->b_ml.ml_line_count) {
retval = false;
break;
}
curwin->w_cursor.lnum++;
curwin->w_curswant %= width2;
if (curwin->w_curswant >= width1) {
curwin->w_curswant -= width2;
}
linelen = linetabsize(get_cursor_line_ptr());
}
}
}
}
if (virtual_active() && atend)
coladvance(MAXCOL);
else
coladvance(curwin->w_curswant);
if (curwin->w_cursor.col > 0 && curwin->w_p_wrap) {
validate_virtcol();
colnr_T virtcol = curwin->w_virtcol;
if (virtcol > (colnr_T)width1 && *p_sbr != NUL)
virtcol -= vim_strsize(p_sbr);
if (virtcol > curwin->w_curswant
&& (curwin->w_curswant < (colnr_T)width1
? (curwin->w_curswant > (colnr_T)width1 / 2)
: ((curwin->w_curswant - width1) % width2
> (colnr_T)width2 / 2)))
--curwin->w_cursor.col;
}
if (atend)
curwin->w_curswant = MAXCOL; 
return retval;
}
static void nv_mousescroll(cmdarg_T *cap)
{
win_T *old_curwin = curwin;
if (mouse_row >= 0 && mouse_col >= 0) {
int grid, row, col;
grid = mouse_grid;
row = mouse_row;
col = mouse_col;
win_T *wp = mouse_find_win(&grid, &row, &col);
if (wp == NULL) {
return;
}
curwin = wp;
curbuf = curwin->w_buffer;
}
if (cap->arg == MSCR_UP || cap->arg == MSCR_DOWN) {
if (mod_mask & (MOD_MASK_SHIFT | MOD_MASK_CTRL)) {
(void)onepage(cap->arg ? FORWARD : BACKWARD, 1L);
} else {
cap->count1 = 3;
cap->count0 = 3;
nv_scroll_line(cap);
}
} else {
mouse_scroll_horiz(cap->arg);
}
if (curwin != old_curwin && curwin->w_p_cul) {
redraw_for_cursorline(curwin);
}
curwin->w_redr_status = true;
curwin = old_curwin;
curbuf = curwin->w_buffer;
}
static void nv_mouse(cmdarg_T *cap)
{
(void)do_mouse(cap->oap, cap->cmdchar, BACKWARD, cap->count1, 0);
}
static void nv_scroll_line(cmdarg_T *cap)
{
if (!checkclearop(cap->oap))
scroll_redraw(cap->arg, cap->count1);
}
void scroll_redraw(int up, long count)
{
linenr_T prev_topline = curwin->w_topline;
int prev_topfill = curwin->w_topfill;
linenr_T prev_lnum = curwin->w_cursor.lnum;
if (up)
scrollup(count, true);
else
scrolldown(count, true);
if (get_scrolloff_value()) {
cursor_correct();
check_cursor_moved(curwin);
curwin->w_valid |= VALID_TOPLINE;
while (curwin->w_topline == prev_topline
&& curwin->w_topfill == prev_topfill
) {
if (up) {
if (curwin->w_cursor.lnum > prev_lnum
|| cursor_down(1L, false) == false)
break;
} else {
if (curwin->w_cursor.lnum < prev_lnum
|| prev_topline == 1L
|| cursor_up(1L, false) == false)
break;
}
check_cursor_moved(curwin);
curwin->w_valid |= VALID_TOPLINE;
}
}
if (curwin->w_cursor.lnum != prev_lnum)
coladvance(curwin->w_curswant);
curwin->w_viewport_invalid = true;
redraw_later(VALID);
}
static void nv_zet(cmdarg_T *cap)
{
int n;
colnr_T col;
int nchar = cap->nchar;
long old_fdl = curwin->w_p_fdl;
int old_fen = curwin->w_p_fen;
bool undo = false;
int l_p_siso = (int)get_sidescrolloff_value();
assert(l_p_siso <= INT_MAX);
if (ascii_isdigit(nchar)) {
if (checkclearop(cap->oap))
return;
n = nchar - '0';
for (;; ) {
no_mapping++;
nchar = plain_vgetc();
LANGMAP_ADJUST(nchar, true);
no_mapping--;
(void)add_to_showcmd(nchar);
if (nchar == K_DEL || nchar == K_KDEL)
n /= 10;
else if (ascii_isdigit(nchar))
n = n * 10 + (nchar - '0');
else if (nchar == CAR) {
win_setheight(n);
break;
} else if (nchar == 'l'
|| nchar == 'h'
|| nchar == K_LEFT
|| nchar == K_RIGHT) {
cap->count1 = n ? n * cap->count1 : cap->count1;
goto dozet;
} else {
clearopbeep(cap->oap);
break;
}
}
cap->oap->op_type = OP_NOP;
return;
}
dozet:
if (cap->nchar != 'f' && cap->nchar != 'F'
&& !(VIsual_active && vim_strchr((char_u *)"dcCoO", cap->nchar))
&& cap->nchar != 'j' && cap->nchar != 'k'
&& checkclearop(cap->oap)) {
return;
}
if ((vim_strchr((char_u *)"+\r\nt.z^-b", nchar) != NULL)
&& cap->count0
&& cap->count0 != curwin->w_cursor.lnum) {
setpcmark();
if (cap->count0 > curbuf->b_ml.ml_line_count)
curwin->w_cursor.lnum = curbuf->b_ml.ml_line_count;
else
curwin->w_cursor.lnum = cap->count0;
check_cursor_col();
}
switch (nchar) {
case '+':
if (cap->count0 == 0) {
validate_botline(); 
if (curwin->w_botline > curbuf->b_ml.ml_line_count)
curwin->w_cursor.lnum = curbuf->b_ml.ml_line_count;
else
curwin->w_cursor.lnum = curwin->w_botline;
}
FALLTHROUGH;
case NL:
case CAR:
case K_KENTER:
beginline(BL_WHITE | BL_FIX);
FALLTHROUGH;
case 't': scroll_cursor_top(0, true);
redraw_later(VALID);
set_fraction(curwin);
break;
case '.': beginline(BL_WHITE | BL_FIX);
FALLTHROUGH;
case 'z': scroll_cursor_halfway(true);
redraw_later(VALID);
set_fraction(curwin);
break;
case '^': 
if (cap->count0 != 0) {
scroll_cursor_bot(0, true);
curwin->w_cursor.lnum = curwin->w_topline;
} else if (curwin->w_topline == 1)
curwin->w_cursor.lnum = 1;
else
curwin->w_cursor.lnum = curwin->w_topline - 1;
FALLTHROUGH;
case '-':
beginline(BL_WHITE | BL_FIX);
FALLTHROUGH;
case 'b': scroll_cursor_bot(0, true);
redraw_later(VALID);
set_fraction(curwin);
break;
case 'H':
cap->count1 *= curwin->w_width_inner / 2;
FALLTHROUGH;
case 'h':
case K_LEFT:
if (!curwin->w_p_wrap) {
if ((colnr_T)cap->count1 > curwin->w_leftcol)
curwin->w_leftcol = 0;
else
curwin->w_leftcol -= (colnr_T)cap->count1;
leftcol_changed();
}
break;
case 'L': cap->count1 *= curwin->w_width_inner / 2;
FALLTHROUGH;
case 'l':
case K_RIGHT:
if (!curwin->w_p_wrap) {
curwin->w_leftcol += (colnr_T)cap->count1;
leftcol_changed();
}
break;
case 's': if (!curwin->w_p_wrap) {
if (hasFolding(curwin->w_cursor.lnum, NULL, NULL))
col = 0; 
else
getvcol(curwin, &curwin->w_cursor, &col, NULL, NULL);
if (col > l_p_siso)
col -= l_p_siso;
else
col = 0;
if (curwin->w_leftcol != col) {
curwin->w_leftcol = col;
redraw_later(NOT_VALID);
}
}
break;
case 'e': if (!curwin->w_p_wrap) {
if (hasFolding(curwin->w_cursor.lnum, NULL, NULL))
col = 0; 
else
getvcol(curwin, &curwin->w_cursor, NULL, NULL, &col);
n = curwin->w_width_inner - curwin_col_off();
if (col + l_p_siso < n) {
col = 0;
} else {
col = col + l_p_siso - n + 1;
}
if (curwin->w_leftcol != col) {
curwin->w_leftcol = col;
redraw_later(NOT_VALID);
}
}
break;
case 'F':
case 'f': if (foldManualAllowed(true)) {
cap->nchar = 'f';
nv_operator(cap);
curwin->w_p_fen = true;
if (nchar == 'F' && cap->oap->op_type == OP_FOLD) {
nv_operator(cap);
finish_op = true;
}
} else
clearopbeep(cap->oap);
break;
case 'd':
case 'D': if (foldManualAllowed(false)) {
if (VIsual_active)
nv_operator(cap);
else
deleteFold(curwin->w_cursor.lnum,
curwin->w_cursor.lnum, nchar == 'D', false);
}
break;
case 'E': if (foldmethodIsManual(curwin)) {
clearFolding(curwin);
changed_window_setting();
} else if (foldmethodIsMarker(curwin))
deleteFold((linenr_T)1, curbuf->b_ml.ml_line_count,
true, false);
else
EMSG(_("E352: Cannot erase folds with current 'foldmethod'"));
break;
case 'n': curwin->w_p_fen = false;
break;
case 'N': curwin->w_p_fen = true;
break;
case 'i': curwin->w_p_fen = !curwin->w_p_fen;
break;
case 'a': if (hasFolding(curwin->w_cursor.lnum, NULL, NULL))
openFold(curwin->w_cursor.lnum, cap->count1);
else {
closeFold(curwin->w_cursor.lnum, cap->count1);
curwin->w_p_fen = true;
}
break;
case 'A': if (hasFolding(curwin->w_cursor.lnum, NULL, NULL))
openFoldRecurse(curwin->w_cursor.lnum);
else {
closeFoldRecurse(curwin->w_cursor.lnum);
curwin->w_p_fen = true;
}
break;
case 'o': if (VIsual_active)
nv_operator(cap);
else
openFold(curwin->w_cursor.lnum, cap->count1);
break;
case 'O': if (VIsual_active)
nv_operator(cap);
else
openFoldRecurse(curwin->w_cursor.lnum);
break;
case 'c': if (VIsual_active)
nv_operator(cap);
else
closeFold(curwin->w_cursor.lnum, cap->count1);
curwin->w_p_fen = true;
break;
case 'C': if (VIsual_active)
nv_operator(cap);
else
closeFoldRecurse(curwin->w_cursor.lnum);
curwin->w_p_fen = true;
break;
case 'v': foldOpenCursor();
break;
case 'x': curwin->w_p_fen = true;
curwin->w_foldinvalid = true; 
newFoldLevel(); 
foldOpenCursor();
break;
case 'X': curwin->w_p_fen = true;
curwin->w_foldinvalid = true; 
old_fdl = -1; 
break;
case 'm':
if (curwin->w_p_fdl > 0) {
curwin->w_p_fdl -= cap->count1;
if (curwin->w_p_fdl < 0) {
curwin->w_p_fdl = 0;
}
}
old_fdl = -1; 
curwin->w_p_fen = true;
break;
case 'M': curwin->w_p_fdl = 0;
old_fdl = -1; 
curwin->w_p_fen = true;
break;
case 'r':
curwin->w_p_fdl += cap->count1;
{
int d = getDeepestNesting(curwin);
if (curwin->w_p_fdl >= d) {
curwin->w_p_fdl = d;
}
}
break;
case 'R': 
curwin->w_p_fdl = getDeepestNesting(curwin);
old_fdl = -1; 
break;
case 'j': 
case 'k': 
if (foldMoveTo(true, nchar == 'j' ? FORWARD : BACKWARD,
cap->count1) == false)
clearopbeep(cap->oap);
break;
case 'u': 
no_mapping++;
nchar = plain_vgetc();
LANGMAP_ADJUST(nchar, true);
no_mapping--;
(void)add_to_showcmd(nchar);
if (vim_strchr((char_u *)"gGwW", nchar) == NULL) {
clearopbeep(cap->oap);
break;
}
undo = true;
FALLTHROUGH;
case 'g': 
case 'w': 
case 'G': 
case 'W': 
{
char_u *ptr = NULL;
size_t len;
if (checkclearop(cap->oap))
break;
if (VIsual_active && !get_visual_text(cap, &ptr, &len))
return;
if (ptr == NULL) {
pos_T pos = curwin->w_cursor;
emsg_off++;
len = spell_move_to(curwin, FORWARD, true, true, NULL);
emsg_off--;
if (len != 0 && curwin->w_cursor.col <= pos.col)
ptr = ml_get_pos(&curwin->w_cursor);
curwin->w_cursor = pos;
}
if (ptr == NULL && (len = find_ident_under_cursor(&ptr, FIND_IDENT)) == 0)
return;
assert(len <= INT_MAX);
spell_add_word(ptr, (int)len, nchar == 'w' || nchar == 'W',
(nchar == 'G' || nchar == 'W') ? 0 : (int)cap->count1,
undo);
}
break;
case '=': 
if (!checkclearop(cap->oap))
spell_suggest((int)cap->count0);
break;
default: clearopbeep(cap->oap);
}
if (old_fen != curwin->w_p_fen) {
if (foldmethodIsDiff(curwin) && curwin->w_p_scb) {
FOR_ALL_WINDOWS_IN_TAB(wp, curtab) {
if (wp != curwin && foldmethodIsDiff(wp) && wp->w_p_scb) {
wp->w_p_fen = curwin->w_p_fen;
changed_window_setting_win(wp);
}
}
}
changed_window_setting();
}
if (old_fdl != curwin->w_p_fdl)
newFoldLevel();
}
static void nv_exmode(cmdarg_T *cap)
{
if (VIsual_active) {
vim_beep(BO_EX);
} else if (!checkclearop(cap->oap)) {
do_exmode(false);
}
}
static void nv_colon(cmdarg_T *cap)
{
int old_p_im;
bool cmd_result;
bool is_cmdkey = cap->cmdchar == K_COMMAND;
if (VIsual_active && !is_cmdkey) {
nv_operator(cap);
} else {
if (cap->oap->op_type != OP_NOP) {
cap->oap->motion_type = kMTCharWise;
cap->oap->inclusive = false;
} else if (cap->count0 && !is_cmdkey) {
stuffcharReadbuff('.');
if (cap->count0 > 1) {
stuffReadbuff(",.+");
stuffnumReadbuff(cap->count0 - 1L);
}
}
if (KeyTyped)
compute_cmdrow();
old_p_im = p_im;
cmd_result = do_cmdline(NULL, is_cmdkey ? getcmdkeycmd : getexline, NULL,
cap->oap->op_type != OP_NOP ? DOCMD_KEEPLINE : 0);
if (p_im != old_p_im) {
if (p_im)
restart_edit = 'i';
else
restart_edit = 0;
}
if (cmd_result == false)
clearop(cap->oap);
else if (cap->oap->op_type != OP_NOP
&& (cap->oap->start.lnum > curbuf->b_ml.ml_line_count
|| cap->oap->start.col >
(colnr_T)STRLEN(ml_get(cap->oap->start.lnum))
|| did_emsg
))
clearopbeep(cap->oap);
}
}
static void nv_ctrlg(cmdarg_T *cap)
{
if (VIsual_active) { 
VIsual_select = !VIsual_select;
showmode();
} else if (!checkclearop(cap->oap))
fileinfo((int)cap->count0, false, true);
}
static void nv_ctrlh(cmdarg_T *cap)
{
if (VIsual_active && VIsual_select) {
cap->cmdchar = 'x'; 
v_visop(cap);
} else
nv_left(cap);
}
static void nv_clear(cmdarg_T *cap)
{
if (!checkclearop(cap->oap)) {
syn_stack_free_all(curwin->w_s);
FOR_ALL_WINDOWS_IN_TAB(wp, curtab) {
wp->w_s->b_syn_slow = false;
}
redraw_later(CLEAR);
}
}
static void nv_ctrlo(cmdarg_T *cap)
{
if (VIsual_active && VIsual_select) {
VIsual_select = false;
showmode();
restart_VIsual_select = 2; 
} else {
cap->count1 = -cap->count1;
nv_pcmark(cap);
}
}
static void nv_hat(cmdarg_T *cap)
{
if (!checkclearopq(cap->oap))
(void)buflist_getfile((int)cap->count0, (linenr_T)0,
GETF_SETMARK|GETF_ALT, false);
}
static void nv_Zet(cmdarg_T *cap)
{
if (!checkclearopq(cap->oap)) {
switch (cap->nchar) {
case 'Z': do_cmdline_cmd("x");
break;
case 'Q': do_cmdline_cmd("q!");
break;
default: clearopbeep(cap->oap);
}
}
}
void do_nv_ident(int c1, int c2)
{
oparg_T oa;
cmdarg_T ca;
clear_oparg(&oa);
memset(&ca, 0, sizeof(ca));
ca.oap = &oa;
ca.cmdchar = c1;
ca.nchar = c2;
nv_ident(&ca);
}
static void nv_ident(cmdarg_T *cap)
{
char_u *ptr = NULL;
char_u *p;
size_t n = 0; 
int cmdchar;
bool g_cmd; 
bool tag_cmd = false;
char_u *aux_ptr;
if (cap->cmdchar == 'g') { 
cmdchar = cap->nchar;
g_cmd = true;
} else {
cmdchar = cap->cmdchar;
g_cmd = false;
}
if (cmdchar == POUND) 
cmdchar = '#';
if (cmdchar == ']' || cmdchar == Ctrl_RSB || cmdchar == 'K') {
if (VIsual_active && get_visual_text(cap, &ptr, &n) == false)
return;
if (checkclearopq(cap->oap))
return;
}
if (ptr == NULL && (n = find_ident_under_cursor(&ptr,
((cmdchar == '*'
|| cmdchar == '#')
? FIND_IDENT|FIND_STRING
: FIND_IDENT))) == 0) {
clearop(cap->oap);
return;
}
char_u *kp = *curbuf->b_p_kp == NUL ? p_kp : curbuf->b_p_kp; 
assert(*kp != NUL); 
bool kp_ex = (*kp == ':'); 
bool kp_help = (STRCMP(kp, ":he") == 0 || STRCMP(kp, ":help") == 0);
if (kp_help && *skipwhite(ptr) == NUL) {
EMSG(_(e_noident)); 
return;
}
size_t buf_size = n * 2 + 30 + STRLEN(kp);
char *buf = xmalloc(buf_size);
buf[0] = NUL;
switch (cmdchar) {
case '*':
case '#':
setpcmark();
curwin->w_cursor.col = (colnr_T) (ptr - get_cursor_line_ptr());
if (!g_cmd && vim_iswordp(ptr))
STRCPY(buf, "\\<");
no_smartcase = true; 
break;
case 'K':
if (kp_help) {
STRCPY(buf, "he! ");
} else if (kp_ex) {
if (cap->count0 != 0) { 
snprintf(buf, buf_size, "%" PRId64, (int64_t)(cap->count0));
}
STRCAT(buf, kp);
STRCAT(buf, " ");
} else {
while (*ptr == '-' && n > 0) {
++ptr;
--n;
}
if (n == 0) {
EMSG(_(e_noident)); 
xfree(buf);
return;
}
bool isman = (STRCMP(kp, "man") == 0);
bool isman_s = (STRCMP(kp, "man -s") == 0);
if (cap->count0 != 0 && !(isman || isman_s)) {
snprintf(buf, buf_size, ".,.+%" PRId64, (int64_t)(cap->count0 - 1));
}
STRCAT(buf, "! ");
if (cap->count0 == 0 && isman_s) {
STRCAT(buf, "man");
} else {
STRCAT(buf, kp);
}
STRCAT(buf, " ");
if (cap->count0 != 0 && (isman || isman_s)) {
snprintf(buf + STRLEN(buf), buf_size - STRLEN(buf), "%" PRId64,
(int64_t)cap->count0);
STRCAT(buf, " ");
}
}
break;
case ']':
tag_cmd = true;
if (p_cst)
STRCPY(buf, "cstag ");
else
STRCPY(buf, "ts ");
break;
default:
tag_cmd = true;
if (curbuf->b_help)
STRCPY(buf, "he! ");
else {
if (g_cmd)
STRCPY(buf, "tj ");
else
snprintf(buf, buf_size, "%" PRId64 "ta ", (int64_t)cap->count0);
}
}
if (cmdchar == 'K' && !kp_help) {
ptr = vim_strnsave(ptr, n);
if (kp_ex) {
p = (char_u *)vim_strsave_fnameescape((const char *)ptr, false);
} else {
p = vim_strsave_shellescape(ptr, true, true);
}
xfree(ptr);
char *newbuf = xrealloc(buf, STRLEN(buf) + STRLEN(p) + 1);
buf = newbuf;
STRCAT(buf, p);
xfree(p);
} else {
if (cmdchar == '*')
aux_ptr = (char_u *)(p_magic ? "/.*~[^$\\" : "/^$\\");
else if (cmdchar == '#')
aux_ptr = (char_u *)(p_magic ? "/?.*~[^$\\" : "/?^$\\");
else if (tag_cmd) {
if (curbuf->b_help)
aux_ptr = (char_u *)"";
else
aux_ptr = (char_u *)"\\|\"\n[";
} else
aux_ptr = (char_u *)"\\|\"\n*?[";
p = (char_u *)buf + STRLEN(buf);
while (n-- > 0) {
if (vim_strchr(aux_ptr, *ptr) != NULL)
*p++ = '\\';
const size_t len = (size_t)(utfc_ptr2len(ptr) - 1);
for (size_t i = 0; i < len && n > 0; i++, n--) {
*p++ = *ptr++;
}
*p++ = *ptr++;
}
*p = NUL;
}
if (cmdchar == '*' || cmdchar == '#') {
if (!g_cmd
&& vim_iswordp(mb_prevptr(get_cursor_line_ptr(), ptr))) {
STRCAT(buf, "\\>");
}
init_history();
add_to_history(HIST_SEARCH, (char_u *)buf, true, NUL);
(void)normal_search(cap, cmdchar == '*' ? '/' : '?', (char_u *)buf, 0,
NULL);
} else {
g_tag_at_cursor = true;
do_cmdline_cmd(buf);
g_tag_at_cursor = false;
}
xfree(buf);
}
bool
get_visual_text (
cmdarg_T *cap,
char_u **pp, 
size_t *lenp 
)
{
if (VIsual_mode != 'V')
unadjust_for_sel();
if (VIsual.lnum != curwin->w_cursor.lnum) {
if (cap != NULL)
clearopbeep(cap->oap);
return false;
}
if (VIsual_mode == 'V') {
*pp = get_cursor_line_ptr();
*lenp = STRLEN(*pp);
} else {
if (lt(curwin->w_cursor, VIsual)) {
*pp = ml_get_pos(&curwin->w_cursor);
*lenp = (size_t)VIsual.col - (size_t)curwin->w_cursor.col + 1;
} else {
*pp = ml_get_pos(&VIsual);
*lenp = (size_t)curwin->w_cursor.col - (size_t)VIsual.col + 1;
}
*lenp += (size_t)(utfc_ptr2len(*pp + (*lenp - 1)) - 1);
}
reset_VIsual_and_resel();
return true;
}
static void nv_tagpop(cmdarg_T *cap)
{
if (!checkclearopq(cap->oap))
do_tag((char_u *)"", DT_POP, (int)cap->count1, false, true);
}
static void nv_scroll(cmdarg_T *cap)
{
int used = 0;
long n;
linenr_T lnum;
int half;
cap->oap->motion_type = kMTLineWise;
setpcmark();
if (cap->cmdchar == 'L') {
validate_botline(); 
curwin->w_cursor.lnum = curwin->w_botline - 1;
if (cap->count1 - 1 >= curwin->w_cursor.lnum)
curwin->w_cursor.lnum = 1;
else {
if (hasAnyFolding(curwin)) {
for (n = cap->count1 - 1; n > 0
&& curwin->w_cursor.lnum > curwin->w_topline; --n) {
(void)hasFolding(curwin->w_cursor.lnum,
&curwin->w_cursor.lnum, NULL);
--curwin->w_cursor.lnum;
}
} else
curwin->w_cursor.lnum -= cap->count1 - 1;
}
} else {
if (cap->cmdchar == 'M') {
used -= diff_check_fill(curwin, curwin->w_topline)
- curwin->w_topfill;
validate_botline(); 
half = (curwin->w_height_inner - curwin->w_empty_rows + 1) / 2;
for (n = 0; curwin->w_topline + n < curbuf->b_ml.ml_line_count; n++) {
if (n > 0 && used + diff_check_fill(curwin, curwin->w_topline
+ n) / 2 >= half) {
--n;
break;
}
used += plines(curwin->w_topline + n);
if (used >= half)
break;
if (hasFolding(curwin->w_topline + n, NULL, &lnum))
n = lnum - curwin->w_topline;
}
if (n > 0 && used > curwin->w_height_inner) {
n--;
}
} else { 
n = cap->count1 - 1;
if (hasAnyFolding(curwin)) {
lnum = curwin->w_topline;
while (n-- > 0 && lnum < curwin->w_botline - 1) {
hasFolding(lnum, NULL, &lnum);
++lnum;
}
n = lnum - curwin->w_topline;
}
}
curwin->w_cursor.lnum = curwin->w_topline + n;
if (curwin->w_cursor.lnum > curbuf->b_ml.ml_line_count)
curwin->w_cursor.lnum = curbuf->b_ml.ml_line_count;
}
if (cap->oap->op_type == OP_NOP) {
cursor_correct();
}
beginline(BL_SOL | BL_FIX);
}
static void nv_right(cmdarg_T *cap)
{
long n;
int PAST_LINE;
if (mod_mask & (MOD_MASK_SHIFT | MOD_MASK_CTRL)) {
if (mod_mask & MOD_MASK_CTRL)
cap->arg = true;
nv_wordcmd(cap);
return;
}
cap->oap->motion_type = kMTCharWise;
cap->oap->inclusive = false;
PAST_LINE = (VIsual_active && *p_sel != 'o');
if (virtual_active())
PAST_LINE = 0;
for (n = cap->count1; n > 0; --n) {
if ((!PAST_LINE && oneright() == false)
|| (PAST_LINE && *get_cursor_pos_ptr() == NUL)
) {
if (((cap->cmdchar == ' ' && vim_strchr(p_ww, 's') != NULL)
|| (cap->cmdchar == 'l' && vim_strchr(p_ww, 'l') != NULL)
|| (cap->cmdchar == K_RIGHT && vim_strchr(p_ww, '>') != NULL))
&& curwin->w_cursor.lnum < curbuf->b_ml.ml_line_count) {
if (cap->oap->op_type != OP_NOP
&& !cap->oap->inclusive
&& !LINEEMPTY(curwin->w_cursor.lnum)) {
cap->oap->inclusive = true;
} else {
++curwin->w_cursor.lnum;
curwin->w_cursor.col = 0;
curwin->w_cursor.coladd = 0;
curwin->w_set_curswant = true;
cap->oap->inclusive = false;
}
continue;
}
if (cap->oap->op_type == OP_NOP) {
if (n == cap->count1) {
beep_flush();
}
} else {
if (!LINEEMPTY(curwin->w_cursor.lnum)) {
cap->oap->inclusive = true;
}
}
break;
} else if (PAST_LINE) {
curwin->w_set_curswant = true;
if (virtual_active()) {
oneright();
} else {
curwin->w_cursor.col += (*mb_ptr2len)(get_cursor_pos_ptr());
}
}
}
if (n != cap->count1 && (fdo_flags & FDO_HOR) && KeyTyped
&& cap->oap->op_type == OP_NOP)
foldOpenCursor();
}
static void nv_left(cmdarg_T *cap)
{
long n;
if (mod_mask & (MOD_MASK_SHIFT | MOD_MASK_CTRL)) {
if (mod_mask & MOD_MASK_CTRL)
cap->arg = 1;
nv_bck_word(cap);
return;
}
cap->oap->motion_type = kMTCharWise;
cap->oap->inclusive = false;
for (n = cap->count1; n > 0; --n) {
if (oneleft() == false) {
if ( (((cap->cmdchar == K_BS
|| cap->cmdchar == Ctrl_H)
&& vim_strchr(p_ww, 'b') != NULL)
|| (cap->cmdchar == 'h'
&& vim_strchr(p_ww, 'h') != NULL)
|| (cap->cmdchar == K_LEFT
&& vim_strchr(p_ww, '<') != NULL))
&& curwin->w_cursor.lnum > 1) {
--(curwin->w_cursor.lnum);
coladvance((colnr_T)MAXCOL);
curwin->w_set_curswant = true;
if ((cap->oap->op_type == OP_DELETE || cap->oap->op_type == OP_CHANGE)
&& !LINEEMPTY(curwin->w_cursor.lnum)) {
char_u *cp = get_cursor_pos_ptr();
if (*cp != NUL) {
curwin->w_cursor.col += utfc_ptr2len(cp);
}
cap->retval |= CA_NO_ADJ_OP_END;
}
continue;
}
else if (cap->oap->op_type == OP_NOP && n == cap->count1)
beep_flush();
break;
}
}
if (n != cap->count1 && (fdo_flags & FDO_HOR) && KeyTyped
&& cap->oap->op_type == OP_NOP)
foldOpenCursor();
}
static void nv_up(cmdarg_T *cap)
{
if (mod_mask & MOD_MASK_SHIFT) {
cap->arg = BACKWARD;
nv_page(cap);
} else {
cap->oap->motion_type = kMTLineWise;
if (cursor_up(cap->count1, cap->oap->op_type == OP_NOP) == false) {
clearopbeep(cap->oap);
} else if (cap->arg) {
beginline(BL_WHITE | BL_FIX);
}
}
}
static void nv_down(cmdarg_T *cap)
{
if (mod_mask & MOD_MASK_SHIFT) {
cap->arg = FORWARD;
nv_page(cap);
} else if (bt_quickfix(curbuf) && cap->cmdchar == CAR) {
qf_view_result(false);
} else {
if (cmdwin_type != 0 && cap->cmdchar == CAR) {
cmdwin_result = CAR;
} else if (bt_prompt(curbuf) && cap->cmdchar == CAR
&& curwin->w_cursor.lnum == curbuf->b_ml.ml_line_count) {
invoke_prompt_callback();
if (restart_edit == 0) {
restart_edit = 'a';
}
} else {
cap->oap->motion_type = kMTLineWise;
if (cursor_down(cap->count1, cap->oap->op_type == OP_NOP) == false) {
clearopbeep(cap->oap);
} else if (cap->arg) {
beginline(BL_WHITE | BL_FIX);
}
}
}
}
static void nv_gotofile(cmdarg_T *cap)
{
char_u *ptr;
linenr_T lnum = -1;
if (text_locked()) {
clearopbeep(cap->oap);
text_locked_msg();
return;
}
if (curbuf_locked()) {
clearop(cap->oap);
return;
}
ptr = grab_file_name(cap->count1, &lnum);
if (ptr != NULL) {
if (curbufIsChanged() && curbuf->b_nwindows <= 1 && !buf_hide(curbuf)) {
(void)autowrite(curbuf, false);
}
setpcmark();
if (do_ecmd(0, ptr, NULL, NULL, ECMD_LAST,
buf_hide(curbuf) ? ECMD_HIDE : 0, curwin) == OK
&& cap->nchar == 'F' && lnum >= 0) {
curwin->w_cursor.lnum = lnum;
check_cursor_lnum();
beginline(BL_SOL | BL_FIX);
}
xfree(ptr);
} else
clearop(cap->oap);
}
static void nv_end(cmdarg_T *cap)
{
if (cap->arg || (mod_mask & MOD_MASK_CTRL)) { 
cap->arg = true;
nv_goto(cap);
cap->count1 = 1; 
}
nv_dollar(cap);
}
static void nv_dollar(cmdarg_T *cap)
{
cap->oap->motion_type = kMTCharWise;
cap->oap->inclusive = true;
if (!virtual_active() || gchar_cursor() != NUL
|| cap->oap->op_type == OP_NOP)
curwin->w_curswant = MAXCOL; 
if (cursor_down(cap->count1 - 1,
cap->oap->op_type == OP_NOP) == false)
clearopbeep(cap->oap);
else if ((fdo_flags & FDO_HOR) && KeyTyped && cap->oap->op_type == OP_NOP)
foldOpenCursor();
}
static void nv_search(cmdarg_T *cap)
{
oparg_T *oap = cap->oap;
pos_T save_cursor = curwin->w_cursor;
if (cap->cmdchar == '?' && cap->oap->op_type == OP_ROT13) {
cap->cmdchar = 'g';
cap->nchar = '?';
nv_operator(cap);
return;
}
cap->searchbuf = getcmdline(cap->cmdchar, cap->count1, 0, true);
if (cap->searchbuf == NULL) {
clearop(oap);
return;
}
(void)normal_search(cap, cap->cmdchar, cap->searchbuf,
(cap->arg || !equalpos(save_cursor, curwin->w_cursor))
? 0 : SEARCH_MARK, NULL);
}
static void nv_next(cmdarg_T *cap)
{
pos_T old = curwin->w_cursor;
int wrapped = false;
int i = normal_search(cap, 0, NULL, SEARCH_MARK | cap->arg, &wrapped);
if (i == 1 && !wrapped && equalpos(old, curwin->w_cursor)) {
cap->count1 += 1;
(void)normal_search(cap, 0, NULL, SEARCH_MARK | cap->arg, NULL);
cap->count1 -= 1;
}
}
static int normal_search(
cmdarg_T *cap,
int dir,
char_u *pat,
int opt, 
int *wrapped
)
{
int i;
searchit_arg_T sia;
cap->oap->motion_type = kMTCharWise;
cap->oap->inclusive = false;
cap->oap->use_reg_one = true;
curwin->w_set_curswant = true;
memset(&sia, 0, sizeof(sia));
i = do_search(cap->oap, dir, pat, cap->count1,
opt | SEARCH_OPT | SEARCH_ECHO | SEARCH_MSG, &sia);
if (wrapped != NULL) {
*wrapped = sia.sa_wrapped;
}
if (i == 0) {
clearop(cap->oap);
} else {
if (i == 2) {
cap->oap->motion_type = kMTLineWise;
}
curwin->w_cursor.coladd = 0;
if (cap->oap->op_type == OP_NOP && (fdo_flags & FDO_SEARCH) && KeyTyped)
foldOpenCursor();
}
check_cursor();
return i;
}
static void nv_csearch(cmdarg_T *cap)
{
bool t_cmd;
if (cap->cmdchar == 't' || cap->cmdchar == 'T')
t_cmd = true;
else
t_cmd = false;
cap->oap->motion_type = kMTCharWise;
if (IS_SPECIAL(cap->nchar) || searchc(cap, t_cmd) == false) {
clearopbeep(cap->oap);
} else {
curwin->w_set_curswant = true;
if (gchar_cursor() == TAB && virtual_active() && cap->arg == FORWARD
&& (t_cmd || cap->oap->op_type != OP_NOP)) {
colnr_T scol, ecol;
getvcol(curwin, &curwin->w_cursor, &scol, NULL, &ecol);
curwin->w_cursor.coladd = ecol - scol;
} else
curwin->w_cursor.coladd = 0;
adjust_for_sel(cap);
if ((fdo_flags & FDO_HOR) && KeyTyped && cap->oap->op_type == OP_NOP)
foldOpenCursor();
}
}
static void nv_brackets(cmdarg_T *cap)
{
pos_T new_pos = { 0, 0, 0 };
pos_T prev_pos;
pos_T *pos = NULL; 
pos_T old_pos; 
int flag;
long n;
int findc;
int c;
cap->oap->motion_type = kMTCharWise;
cap->oap->inclusive = false;
old_pos = curwin->w_cursor;
curwin->w_cursor.coladd = 0; 
if (cap->nchar == 'f')
nv_gotofile(cap);
else
if (vim_strchr((char_u *)
"iI\011dD\004",
cap->nchar) != NULL) {
char_u *ptr;
size_t len;
if ((len = find_ident_under_cursor(&ptr, FIND_IDENT)) == 0)
clearop(cap->oap);
else {
find_pattern_in_path(ptr, 0, len, true,
cap->count0 == 0 ? !isupper(cap->nchar) : false,
(((cap->nchar & 0xf) == ('d' & 0xf))
? FIND_DEFINE
: FIND_ANY),
cap->count1,
(isupper(cap->nchar) ? ACTION_SHOW_ALL :
islower(cap->nchar) ? ACTION_SHOW :
ACTION_GOTO),
(cap->cmdchar == ']'
? curwin->w_cursor.lnum + 1
: (linenr_T)1),
MAXLNUM);
curwin->w_set_curswant = true;
}
} else
if ( (cap->cmdchar == '['
&& vim_strchr((char_u *)"{(*/#mM", cap->nchar) != NULL)
|| (cap->cmdchar == ']'
&& vim_strchr((char_u *)"})*/#mM", cap->nchar) != NULL)) {
if (cap->nchar == '*')
cap->nchar = '/';
prev_pos.lnum = 0;
if (cap->nchar == 'm' || cap->nchar == 'M') {
if (cap->cmdchar == '[')
findc = '{';
else
findc = '}';
n = 9999;
} else {
findc = cap->nchar;
n = cap->count1;
}
for (; n > 0; --n) {
if ((pos = findmatchlimit(cap->oap, findc,
(cap->cmdchar == '[') ? FM_BACKWARD : FM_FORWARD, 0)) == NULL) {
if (new_pos.lnum == 0) { 
if (cap->nchar != 'm' && cap->nchar != 'M')
clearopbeep(cap->oap);
} else
pos = &new_pos; 
break;
}
prev_pos = new_pos;
curwin->w_cursor = *pos;
new_pos = *pos;
}
curwin->w_cursor = old_pos;
if (cap->nchar == 'm' || cap->nchar == 'M') {
int norm = ((findc == '{') == (cap->nchar == 'm'));
n = cap->count1;
if (prev_pos.lnum != 0) {
pos = &prev_pos;
curwin->w_cursor = prev_pos;
if (norm)
--n;
} else
pos = NULL;
while (n > 0) {
for (;; ) {
if ((findc == '{' ? dec_cursor() : inc_cursor()) < 0) {
if (pos == NULL)
clearopbeep(cap->oap);
n = 0;
break;
}
c = gchar_cursor();
if (c == '{' || c == '}') {
if ((c == findc && norm) || (n == 1 && !norm)) {
new_pos = curwin->w_cursor;
pos = &new_pos;
n = 0;
}
else if (new_pos.lnum == 0) {
new_pos = curwin->w_cursor;
pos = &new_pos;
}
else if ((pos = findmatchlimit(cap->oap, findc,
(cap->cmdchar == '[') ? FM_BACKWARD : FM_FORWARD,
0)) == NULL)
n = 0;
else
curwin->w_cursor = *pos;
break;
}
}
--n;
}
curwin->w_cursor = old_pos;
if (pos == NULL && new_pos.lnum != 0)
clearopbeep(cap->oap);
}
if (pos != NULL) {
setpcmark();
curwin->w_cursor = *pos;
curwin->w_set_curswant = true;
if ((fdo_flags & FDO_BLOCK) && KeyTyped
&& cap->oap->op_type == OP_NOP)
foldOpenCursor();
}
}
else if (cap->nchar == '[' || cap->nchar == ']') {
if (cap->nchar == cap->cmdchar) 
flag = '{';
else
flag = '}'; 
curwin->w_set_curswant = true;
if (!findpar(&cap->oap->inclusive, cap->arg, cap->count1, flag,
(cap->oap->op_type != OP_NOP
&& cap->arg == FORWARD && flag == '{')))
clearopbeep(cap->oap);
else {
if (cap->oap->op_type == OP_NOP)
beginline(BL_WHITE | BL_FIX);
if ((fdo_flags & FDO_BLOCK) && KeyTyped && cap->oap->op_type == OP_NOP)
foldOpenCursor();
}
} else if (cap->nchar == 'p' || cap->nchar == 'P') {
nv_put_opt(cap, true);
}
else if (cap->nchar == '\'' || cap->nchar == '`') {
pos = &curwin->w_cursor;
for (n = cap->count1; n > 0; --n) {
prev_pos = *pos;
pos = getnextmark(pos, cap->cmdchar == '[' ? BACKWARD : FORWARD,
cap->nchar == '\'');
if (pos == NULL)
break;
}
if (pos == NULL)
pos = &prev_pos;
nv_cursormark(cap, cap->nchar == '\'', pos);
}
else if (cap->nchar >= K_RIGHTRELEASE && cap->nchar <= K_LEFTMOUSE) {
(void)do_mouse(cap->oap, cap->nchar,
(cap->cmdchar == ']') ? FORWARD : BACKWARD,
cap->count1, PUT_FIXINDENT);
}
else if (cap->nchar == 'z') {
if (foldMoveTo(false, cap->cmdchar == ']' ? FORWARD : BACKWARD,
cap->count1) == false)
clearopbeep(cap->oap);
}
else if (cap->nchar == 'c') {
if (diff_move_to(cap->cmdchar == ']' ? FORWARD : BACKWARD,
cap->count1) == false)
clearopbeep(cap->oap);
}
else if (cap->nchar == 's' || cap->nchar == 'S') {
setpcmark();
for (n = 0; n < cap->count1; ++n)
if (spell_move_to(curwin, cap->cmdchar == ']' ? FORWARD : BACKWARD,
cap->nchar == 's', false, NULL) == 0) {
clearopbeep(cap->oap);
break;
} else {
curwin->w_set_curswant = true;
}
if (cap->oap->op_type == OP_NOP && (fdo_flags & FDO_SEARCH) && KeyTyped)
foldOpenCursor();
}
else
clearopbeep(cap->oap);
}
static void nv_percent(cmdarg_T *cap)
{
pos_T *pos;
linenr_T lnum = curwin->w_cursor.lnum;
cap->oap->inclusive = true;
if (cap->count0) { 
if (cap->count0 > 100) {
clearopbeep(cap->oap);
} else {
cap->oap->motion_type = kMTLineWise;
setpcmark();
if (curbuf->b_ml.ml_line_count > 1000000)
curwin->w_cursor.lnum = (curbuf->b_ml.ml_line_count + 99L)
/ 100L * cap->count0;
else
curwin->w_cursor.lnum = (curbuf->b_ml.ml_line_count *
cap->count0 + 99L) / 100L;
if (curwin->w_cursor.lnum > curbuf->b_ml.ml_line_count)
curwin->w_cursor.lnum = curbuf->b_ml.ml_line_count;
beginline(BL_SOL | BL_FIX);
}
} else { 
cap->oap->motion_type = kMTCharWise;
cap->oap->use_reg_one = true;
if ((pos = findmatch(cap->oap, NUL)) == NULL)
clearopbeep(cap->oap);
else {
setpcmark();
curwin->w_cursor = *pos;
curwin->w_set_curswant = true;
curwin->w_cursor.coladd = 0;
adjust_for_sel(cap);
}
}
if (cap->oap->op_type == OP_NOP
&& lnum != curwin->w_cursor.lnum
&& (fdo_flags & FDO_PERCENT)
&& KeyTyped)
foldOpenCursor();
}
static void nv_brace(cmdarg_T *cap)
{
cap->oap->motion_type = kMTCharWise;
cap->oap->use_reg_one = true;
cap->oap->inclusive = false;
curwin->w_set_curswant = true;
if (findsent(cap->arg, cap->count1) == false)
clearopbeep(cap->oap);
else {
adjust_cursor(cap->oap);
curwin->w_cursor.coladd = 0;
if ((fdo_flags & FDO_BLOCK) && KeyTyped && cap->oap->op_type == OP_NOP)
foldOpenCursor();
}
}
static void nv_mark(cmdarg_T *cap)
{
if (!checkclearop(cap->oap)) {
if (setmark(cap->nchar) == false)
clearopbeep(cap->oap);
}
}
static void nv_findpar(cmdarg_T *cap)
{
cap->oap->motion_type = kMTCharWise;
cap->oap->inclusive = false;
cap->oap->use_reg_one = true;
curwin->w_set_curswant = true;
if (!findpar(&cap->oap->inclusive, cap->arg, cap->count1, NUL, false))
clearopbeep(cap->oap);
else {
curwin->w_cursor.coladd = 0;
if ((fdo_flags & FDO_BLOCK) && KeyTyped && cap->oap->op_type == OP_NOP)
foldOpenCursor();
}
}
static void nv_undo(cmdarg_T *cap)
{
if (cap->oap->op_type == OP_LOWER
|| VIsual_active
) {
cap->cmdchar = 'g';
cap->nchar = 'u';
nv_operator(cap);
} else
nv_kundo(cap);
}
static void nv_kundo(cmdarg_T *cap)
{
if (!checkclearopq(cap->oap)) {
if (bt_prompt(curbuf)) {
clearopbeep(cap->oap);
return;
}
u_undo((int)cap->count1);
curwin->w_set_curswant = true;
}
}
static void nv_replace(cmdarg_T *cap)
{
char_u *ptr;
int had_ctrl_v;
if (checkclearop(cap->oap)) {
return;
}
if (bt_prompt(curbuf) && !prompt_curpos_editable()) {
clearopbeep(cap->oap);
return;
}
if (cap->nchar == Ctrl_V) {
had_ctrl_v = Ctrl_V;
cap->nchar = get_literal();
if (cap->nchar > DEL)
had_ctrl_v = NUL;
} else
had_ctrl_v = NUL;
if (IS_SPECIAL(cap->nchar)) {
clearopbeep(cap->oap);
return;
}
if (VIsual_active) {
if (got_int)
reset_VIsual();
if (had_ctrl_v) {
if (cap->nchar == CAR) {
cap->nchar = REPLACE_CR_NCHAR;
} else if (cap->nchar == NL) {
cap->nchar = REPLACE_NL_NCHAR;
}
}
nv_operator(cap);
return;
}
if (virtual_active()) {
if (u_save_cursor() == false)
return;
if (gchar_cursor() == NUL) {
coladvance_force((colnr_T)(getviscol() + cap->count1));
assert(cap->count1 <= INT_MAX);
curwin->w_cursor.col -= (colnr_T)cap->count1;
} else if (gchar_cursor() == TAB)
coladvance_force(getviscol());
}
ptr = get_cursor_pos_ptr();
if (STRLEN(ptr) < (unsigned)cap->count1
|| (mb_charlen(ptr) < cap->count1)
) {
clearopbeep(cap->oap);
return;
}
if (had_ctrl_v != Ctrl_V && cap->nchar == '\t' && (curbuf->b_p_et || p_sta)) {
stuffnumReadbuff(cap->count1);
stuffcharReadbuff('R');
stuffcharReadbuff('\t');
stuffcharReadbuff(ESC);
return;
}
if (u_save_cursor() == false)
return;
if (had_ctrl_v != Ctrl_V && (cap->nchar == '\r' || cap->nchar == '\n')) {
(void)del_chars(cap->count1, false); 
stuffcharReadbuff('\r');
stuffcharReadbuff(ESC);
invoke_edit(cap, true, 'r', false);
} else {
prep_redo(cap->oap->regname, cap->count1,
NUL, 'r', NUL, had_ctrl_v, cap->nchar);
curbuf->b_op_start = curwin->w_cursor;
const int old_State = State;
if (cap->ncharC1 != 0) {
AppendCharToRedobuff(cap->ncharC1);
}
if (cap->ncharC2 != 0) {
AppendCharToRedobuff(cap->ncharC2);
}
for (long n = cap->count1; n > 0; n--) {
State = REPLACE;
if (cap->nchar == Ctrl_E || cap->nchar == Ctrl_Y) {
int c = ins_copychar(curwin->w_cursor.lnum
+ (cap->nchar == Ctrl_Y ? -1 : 1));
if (c != NUL) {
ins_char(c);
} else {
curwin->w_cursor.col++;
}
} else {
ins_char(cap->nchar);
}
State = old_State;
if (cap->ncharC1 != 0) {
ins_char(cap->ncharC1);
}
if (cap->ncharC2 != 0) {
ins_char(cap->ncharC2);
}
}
--curwin->w_cursor.col; 
mb_adjust_cursor();
curbuf->b_op_end = curwin->w_cursor;
curwin->w_set_curswant = true;
set_last_insert(cap->nchar);
}
foldUpdateAfterInsert();
}
static void v_swap_corners(int cmdchar)
{
pos_T old_cursor;
colnr_T left, right;
if (cmdchar == 'O' && VIsual_mode == Ctrl_V) {
old_cursor = curwin->w_cursor;
getvcols(curwin, &old_cursor, &VIsual, &left, &right);
curwin->w_cursor.lnum = VIsual.lnum;
coladvance(left);
VIsual = curwin->w_cursor;
curwin->w_cursor.lnum = old_cursor.lnum;
curwin->w_curswant = right;
if (old_cursor.lnum >= VIsual.lnum && *p_sel == 'e')
++curwin->w_curswant;
coladvance(curwin->w_curswant);
if (curwin->w_cursor.col == old_cursor.col
&& (!virtual_active()
|| curwin->w_cursor.coladd == old_cursor.coladd)
) {
curwin->w_cursor.lnum = VIsual.lnum;
if (old_cursor.lnum <= VIsual.lnum && *p_sel == 'e')
++right;
coladvance(right);
VIsual = curwin->w_cursor;
curwin->w_cursor.lnum = old_cursor.lnum;
coladvance(left);
curwin->w_curswant = left;
}
} else {
old_cursor = curwin->w_cursor;
curwin->w_cursor = VIsual;
VIsual = old_cursor;
curwin->w_set_curswant = true;
}
}
static void nv_Replace(cmdarg_T *cap)
{
if (VIsual_active) { 
cap->cmdchar = 'c';
cap->nchar = NUL;
VIsual_mode_orig = VIsual_mode; 
VIsual_mode = 'V';
nv_operator(cap);
} else if (!checkclearopq(cap->oap)) {
if (!MODIFIABLE(curbuf)) {
EMSG(_(e_modifiable));
} else {
if (virtual_active())
coladvance(getviscol());
invoke_edit(cap, false, cap->arg ? 'V' : 'R', false);
}
}
}
static void nv_vreplace(cmdarg_T *cap)
{
if (VIsual_active) {
cap->cmdchar = 'r';
cap->nchar = cap->extra_char;
nv_replace(cap); 
} else if (!checkclearopq(cap->oap)) {
if (!MODIFIABLE(curbuf)) {
EMSG(_(e_modifiable));
} else {
if (cap->extra_char == Ctrl_V) 
cap->extra_char = get_literal();
stuffcharReadbuff(cap->extra_char);
stuffcharReadbuff(ESC);
if (virtual_active())
coladvance(getviscol());
invoke_edit(cap, true, 'v', false);
}
}
}
static void n_swapchar(cmdarg_T *cap)
{
long n;
pos_T startpos;
int did_change = 0;
if (checkclearopq(cap->oap)) {
return;
}
if (LINEEMPTY(curwin->w_cursor.lnum) && vim_strchr(p_ww, '~') == NULL) {
clearopbeep(cap->oap);
return;
}
prep_redo_cmd(cap);
if (u_save_cursor() == false)
return;
startpos = curwin->w_cursor;
for (n = cap->count1; n > 0; --n) {
did_change |= swapchar(cap->oap->op_type, &curwin->w_cursor);
inc_cursor();
if (gchar_cursor() == NUL) {
if (vim_strchr(p_ww, '~') != NULL
&& curwin->w_cursor.lnum < curbuf->b_ml.ml_line_count) {
++curwin->w_cursor.lnum;
curwin->w_cursor.col = 0;
if (n > 1) {
if (u_savesub(curwin->w_cursor.lnum) == false)
break;
u_clearline();
}
} else
break;
}
}
check_cursor();
curwin->w_set_curswant = true;
if (did_change) {
changed_lines(startpos.lnum, startpos.col, curwin->w_cursor.lnum + 1,
0L, true);
curbuf->b_op_start = startpos;
curbuf->b_op_end = curwin->w_cursor;
if (curbuf->b_op_end.col > 0)
--curbuf->b_op_end.col;
}
}
static void nv_cursormark(cmdarg_T *cap, int flag, pos_T *pos)
{
if (check_mark(pos) == false)
clearop(cap->oap);
else {
if (cap->cmdchar == '\''
|| cap->cmdchar == '`'
|| cap->cmdchar == '['
|| cap->cmdchar == ']')
setpcmark();
curwin->w_cursor = *pos;
if (flag)
beginline(BL_WHITE | BL_FIX);
else
check_cursor();
}
cap->oap->motion_type = flag ? kMTLineWise : kMTCharWise;
if (cap->cmdchar == '`') {
cap->oap->use_reg_one = true;
}
cap->oap->inclusive = false; 
curwin->w_set_curswant = true;
}
static void v_visop(cmdarg_T *cap)
{
static char_u trans[] = "YyDdCcxdXdAAIIrr";
if (isupper(cap->cmdchar)) {
if (VIsual_mode != Ctrl_V) {
VIsual_mode_orig = VIsual_mode;
VIsual_mode = 'V';
} else if (cap->cmdchar == 'C' || cap->cmdchar == 'D')
curwin->w_curswant = MAXCOL;
}
cap->cmdchar = *(vim_strchr(trans, cap->cmdchar) + 1);
nv_operator(cap);
}
static void nv_subst(cmdarg_T *cap)
{
if (bt_prompt(curbuf) && !prompt_curpos_editable()) {
clearopbeep(cap->oap);
return;
}
if (VIsual_active) { 
if (cap->cmdchar == 'S') {
VIsual_mode_orig = VIsual_mode;
VIsual_mode = 'V';
}
cap->cmdchar = 'c';
nv_operator(cap);
} else
nv_optrans(cap);
}
static void nv_abbrev(cmdarg_T *cap)
{
if (cap->cmdchar == K_DEL || cap->cmdchar == K_KDEL)
cap->cmdchar = 'x'; 
if (VIsual_active)
v_visop(cap);
else
nv_optrans(cap);
}
static void nv_optrans(cmdarg_T *cap)
{
static const char *(ar[]) = { "dl", "dh", "d$", "c$", "cl", "cc", "yy",
":s\r" };
static const char *str = "xXDCsSY&";
if (!checkclearopq(cap->oap)) {
if (cap->count0) {
stuffnumReadbuff(cap->count0);
}
stuffReadbuff(ar[strchr(str, (char)cap->cmdchar) - str]);
}
cap->opcount = 0;
}
static void nv_gomark(cmdarg_T *cap)
{
pos_T *pos;
int c;
pos_T old_cursor = curwin->w_cursor;
const bool old_KeyTyped = KeyTyped; 
if (cap->cmdchar == 'g')
c = cap->extra_char;
else
c = cap->nchar;
pos = getmark(c, (cap->oap->op_type == OP_NOP));
if (pos == (pos_T *)-1) { 
if (cap->arg) {
check_cursor_lnum();
beginline(BL_WHITE | BL_FIX);
} else
check_cursor();
} else
nv_cursormark(cap, cap->arg, pos);
if (!virtual_active()) {
curwin->w_cursor.coladd = 0;
}
check_cursor_col();
if (cap->oap->op_type == OP_NOP
&& pos != NULL
&& (pos == (pos_T *)-1 || !equalpos(old_cursor, *pos))
&& (fdo_flags & FDO_MARK)
&& old_KeyTyped) {
foldOpenCursor();
}
}
static void nv_pcmark(cmdarg_T *cap)
{
pos_T *pos;
linenr_T lnum = curwin->w_cursor.lnum;
const bool old_KeyTyped = KeyTyped; 
if (!checkclearopq(cap->oap)) {
if (cap->cmdchar == TAB && mod_mask == MOD_MASK_CTRL) {
goto_tabpage_lastused();
return;
}
if (cap->cmdchar == 'g') {
pos = movechangelist((int)cap->count1);
} else {
pos = movemark((int)cap->count1);
}
if (pos == (pos_T *)-1) { 
curwin->w_set_curswant = true;
check_cursor();
} else if (pos != NULL) 
nv_cursormark(cap, false, pos);
else if (cap->cmdchar == 'g') {
if (curbuf->b_changelistlen == 0)
EMSG(_("E664: changelist is empty"));
else if (cap->count1 < 0)
EMSG(_("E662: At start of changelist"));
else
EMSG(_("E663: At end of changelist"));
} else
clearopbeep(cap->oap);
if (cap->oap->op_type == OP_NOP
&& (pos == (pos_T *)-1 || lnum != curwin->w_cursor.lnum)
&& (fdo_flags & FDO_MARK)
&& old_KeyTyped)
foldOpenCursor();
}
}
static void nv_regname(cmdarg_T *cap)
{
if (checkclearop(cap->oap))
return;
if (cap->nchar == '=')
cap->nchar = get_expr_register();
if (cap->nchar != NUL && valid_yank_reg(cap->nchar, false)) {
cap->oap->regname = cap->nchar;
cap->opcount = cap->count0; 
set_reg_var(cap->oap->regname);
} else
clearopbeep(cap->oap);
}
static void nv_visual(cmdarg_T *cap)
{
if (cap->cmdchar == Ctrl_Q)
cap->cmdchar = Ctrl_V;
if (cap->oap->op_type != OP_NOP) {
motion_force = cap->oap->motion_force = cap->cmdchar;
finish_op = false; 
return;
}
VIsual_select = cap->arg;
if (VIsual_active) { 
if (VIsual_mode == cap->cmdchar) 
end_visual_mode();
else { 
VIsual_mode = cap->cmdchar;
showmode();
}
redraw_curbuf_later(INVERTED); 
} else { 
if (cap->count0 > 0 && resel_VIsual_mode != NUL) {
VIsual = curwin->w_cursor;
VIsual_active = true;
VIsual_reselect = true;
if (!cap->arg)
may_start_select('c');
setmouse();
if (p_smd && msg_silent == 0)
redraw_cmdline = true; 
if (resel_VIsual_mode != 'v' || resel_VIsual_line_count > 1) {
curwin->w_cursor.lnum +=
resel_VIsual_line_count * cap->count0 - 1;
if (curwin->w_cursor.lnum > curbuf->b_ml.ml_line_count)
curwin->w_cursor.lnum = curbuf->b_ml.ml_line_count;
}
VIsual_mode = resel_VIsual_mode;
if (VIsual_mode == 'v') {
if (resel_VIsual_line_count <= 1) {
validate_virtcol();
assert(cap->count0 >= INT_MIN && cap->count0 <= INT_MAX);
curwin->w_curswant = (curwin->w_virtcol
+ resel_VIsual_vcol * (int)cap->count0 - 1);
} else
curwin->w_curswant = resel_VIsual_vcol;
coladvance(curwin->w_curswant);
}
if (resel_VIsual_vcol == MAXCOL) {
curwin->w_curswant = MAXCOL;
coladvance((colnr_T)MAXCOL);
} else if (VIsual_mode == Ctrl_V) {
validate_virtcol();
assert(cap->count0 >= INT_MIN && cap->count0 <= INT_MAX);
curwin->w_curswant = (curwin->w_virtcol
+ resel_VIsual_vcol * (int)cap->count0 - 1);
coladvance(curwin->w_curswant);
} else
curwin->w_set_curswant = true;
redraw_curbuf_later(INVERTED); 
} else {
if (!cap->arg)
may_start_select('c');
n_start_visual_mode(cap->cmdchar);
if (VIsual_mode != 'V' && *p_sel == 'e')
++cap->count1; 
if (cap->count0 > 0 && --cap->count1 > 0) {
if (VIsual_mode == 'v' || VIsual_mode == Ctrl_V)
nv_right(cap);
else if (VIsual_mode == 'V')
nv_down(cap);
}
}
}
}
void start_selection(void)
{
may_start_select('k');
n_start_visual_mode('v');
}
void may_start_select(int c)
{
VIsual_select = (stuff_empty() && typebuf_typed()
&& (vim_strchr(p_slm, c) != NULL));
}
static void n_start_visual_mode(int c)
{
VIsual_mode = c;
VIsual_active = true;
VIsual_reselect = true;
if (c == Ctrl_V && (ve_flags & VE_BLOCK) && gchar_cursor() == TAB) {
validate_virtcol();
coladvance(curwin->w_virtcol);
}
VIsual = curwin->w_cursor;
foldAdjustVisual();
setmouse();
conceal_check_cursor_line();
if (p_smd && msg_silent == 0)
redraw_cmdline = true; 
if (curwin->w_redr_type < INVERTED) {
curwin->w_old_cursor_lnum = curwin->w_cursor.lnum;
curwin->w_old_visual_lnum = curwin->w_cursor.lnum;
}
}
static void nv_window(cmdarg_T *cap)
{
if (cap->nchar == ':') {
cap->cmdchar = ':';
cap->nchar = NUL;
nv_colon(cap);
} else if (!checkclearop(cap->oap)) {
do_window(cap->nchar, cap->count0, NUL); 
}
}
static void nv_suspend(cmdarg_T *cap)
{
clearop(cap->oap);
if (VIsual_active)
end_visual_mode(); 
do_cmdline_cmd("st");
}
static void nv_g_cmd(cmdarg_T *cap)
{
oparg_T *oap = cap->oap;
pos_T tpos;
int i;
bool flag = false;
switch (cap->nchar) {
case Ctrl_A:
case Ctrl_X:
if (VIsual_active) {
cap->arg = true;
cap->cmdchar = cap->nchar;
cap->nchar = NUL;
nv_addsub(cap);
} else {
clearopbeep(oap);
}
break;
case 'R':
cap->arg = true;
nv_Replace(cap);
break;
case 'r':
nv_vreplace(cap);
break;
case '&':
do_cmdline_cmd("%s//~/&");
break;
case 'v':
if (checkclearop(oap))
break;
if ( curbuf->b_visual.vi_start.lnum == 0
|| curbuf->b_visual.vi_start.lnum > curbuf->b_ml.ml_line_count
|| curbuf->b_visual.vi_end.lnum == 0)
beep_flush();
else {
if (VIsual_active) {
i = VIsual_mode;
VIsual_mode = curbuf->b_visual.vi_mode;
curbuf->b_visual.vi_mode = i;
curbuf->b_visual_mode_eval = i;
i = curwin->w_curswant;
curwin->w_curswant = curbuf->b_visual.vi_curswant;
curbuf->b_visual.vi_curswant = i;
tpos = curbuf->b_visual.vi_end;
curbuf->b_visual.vi_end = curwin->w_cursor;
curwin->w_cursor = curbuf->b_visual.vi_start;
curbuf->b_visual.vi_start = VIsual;
} else {
VIsual_mode = curbuf->b_visual.vi_mode;
curwin->w_curswant = curbuf->b_visual.vi_curswant;
tpos = curbuf->b_visual.vi_end;
curwin->w_cursor = curbuf->b_visual.vi_start;
}
VIsual_active = true;
VIsual_reselect = true;
check_cursor();
VIsual = curwin->w_cursor;
curwin->w_cursor = tpos;
check_cursor();
update_topline();
if (cap->arg)
VIsual_select = true;
else
may_start_select('c');
setmouse();
redraw_curbuf_later(INVERTED);
showmode();
}
break;
case 'V':
VIsual_reselect = false;
break;
case K_BS:
cap->nchar = Ctrl_H;
FALLTHROUGH;
case 'h':
case 'H':
case Ctrl_H:
cap->cmdchar = cap->nchar + ('v' - 'h');
cap->arg = true;
nv_visual(cap);
break;
case 'N':
case 'n':
if (!current_search(cap->count1, cap->nchar == 'n'))
clearopbeep(oap);
break;
case 'j':
case K_DOWN:
if (!curwin->w_p_wrap
|| hasFolding(curwin->w_cursor.lnum, NULL, NULL)
) {
oap->motion_type = kMTLineWise;
i = cursor_down(cap->count1, oap->op_type == OP_NOP);
} else
i = nv_screengo(oap, FORWARD, cap->count1);
if (!i)
clearopbeep(oap);
break;
case 'k':
case K_UP:
if (!curwin->w_p_wrap
|| hasFolding(curwin->w_cursor.lnum, NULL, NULL)
) {
oap->motion_type = kMTLineWise;
i = cursor_up(cap->count1, oap->op_type == OP_NOP);
} else
i = nv_screengo(oap, BACKWARD, cap->count1);
if (!i)
clearopbeep(oap);
break;
case 'J':
nv_join(cap);
break;
case '^':
flag = true;
FALLTHROUGH;
case '0':
case 'm':
case K_HOME:
case K_KHOME:
oap->motion_type = kMTCharWise;
oap->inclusive = false;
if (curwin->w_p_wrap
&& curwin->w_width_inner != 0
) {
int width1 = curwin->w_width_inner - curwin_col_off();
int width2 = width1 + curwin_col_off2();
validate_virtcol();
i = 0;
if (curwin->w_virtcol >= (colnr_T)width1 && width2 > 0)
i = (curwin->w_virtcol - width1) / width2 * width2 + width1;
} else
i = curwin->w_leftcol;
if (cap->nchar == 'm') {
i += (curwin->w_width_inner - curwin_col_off()
+ ((curwin->w_p_wrap && i > 0)
? curwin_col_off2() : 0)) / 2;
}
coladvance((colnr_T)i);
if (flag) {
do
i = gchar_cursor();
while (ascii_iswhite(i) && oneright());
}
curwin->w_set_curswant = true;
break;
case 'M':
{
const char_u *const ptr = get_cursor_line_ptr();
oap->motion_type = kMTCharWise;
oap->inclusive = false;
i = (int)mb_string2cells_len(ptr, STRLEN(ptr));
if (cap->count0 > 0 && cap->count0 <= 100) {
coladvance((colnr_T)(i * cap->count0 / 100));
} else {
coladvance((colnr_T)(i / 2));
}
curwin->w_set_curswant = true;
}
break;
case '_':
cap->oap->motion_type = kMTCharWise;
cap->oap->inclusive = true;
curwin->w_curswant = MAXCOL;
if (cursor_down(cap->count1 - 1,
cap->oap->op_type == OP_NOP) == false)
clearopbeep(cap->oap);
else {
char_u *ptr = get_cursor_line_ptr();
if (curwin->w_cursor.col > 0 && ptr[curwin->w_cursor.col] == NUL)
--curwin->w_cursor.col;
while (curwin->w_cursor.col > 0
&& ascii_iswhite(ptr[curwin->w_cursor.col]))
--curwin->w_cursor.col;
curwin->w_set_curswant = true;
adjust_for_sel(cap);
}
break;
case '$':
case K_END:
case K_KEND:
{
int col_off = curwin_col_off();
oap->motion_type = kMTCharWise;
oap->inclusive = true;
if (curwin->w_p_wrap
&& curwin->w_width_inner != 0
) {
curwin->w_curswant = MAXCOL; 
if (cap->count1 == 1) {
int width1 = curwin->w_width_inner - col_off;
int width2 = width1 + curwin_col_off2();
validate_virtcol();
i = width1 - 1;
if (curwin->w_virtcol >= (colnr_T)width1)
i += ((curwin->w_virtcol - width1) / width2 + 1)
* width2;
coladvance((colnr_T)i);
validate_virtcol();
curwin->w_curswant = curwin->w_virtcol;
curwin->w_set_curswant = false;
if (curwin->w_cursor.col > 0 && curwin->w_p_wrap) {
if (curwin->w_virtcol > (colnr_T)i)
--curwin->w_cursor.col;
}
} else if (nv_screengo(oap, FORWARD, cap->count1 - 1) == false)
clearopbeep(oap);
} else {
if (cap->count1 > 1) {
cursor_down(cap->count1 - 1, false);
}
i = curwin->w_leftcol + curwin->w_width_inner - col_off - 1;
coladvance((colnr_T)i);
validate_virtcol();
curwin->w_curswant = curwin->w_virtcol;
curwin->w_set_curswant = false;
}
}
break;
case '*':
case '#':
#if POUND != '#'
case POUND: 
#endif
case Ctrl_RSB: 
case ']': 
nv_ident(cap);
break;
case 'e':
case 'E':
oap->motion_type = kMTCharWise;
curwin->w_set_curswant = true;
oap->inclusive = true;
if (bckend_word(cap->count1, cap->nchar == 'E', false) == false)
clearopbeep(oap);
break;
case Ctrl_G:
cursor_pos_info(NULL);
break;
case 'i':
if (curbuf->b_last_insert.mark.lnum != 0) {
curwin->w_cursor = curbuf->b_last_insert.mark;
check_cursor_lnum();
i = (int)STRLEN(get_cursor_line_ptr());
if (curwin->w_cursor.col > (colnr_T)i) {
if (virtual_active())
curwin->w_cursor.coladd += curwin->w_cursor.col - i;
curwin->w_cursor.col = i;
}
}
cap->cmdchar = 'i';
nv_edit(cap);
break;
case 'I':
beginline(0);
if (!checkclearopq(oap))
invoke_edit(cap, false, 'g', false);
break;
case 'f':
case 'F':
nv_gotofile(cap);
break;
case '\'':
cap->arg = true;
FALLTHROUGH;
case '`':
nv_gomark(cap);
break;
case 's':
do_sleep(cap->count1 * 1000L);
break;
case 'a':
do_ascii(NULL);
break;
case '8':
if (cap->count0 == 8)
utf_find_illegal();
else
show_utf8();
break;
case '<':
show_sb_text();
break;
case 'g':
cap->arg = false;
nv_goto(cap);
break;
case 'q':
case 'w':
oap->cursor_start = curwin->w_cursor;
FALLTHROUGH;
case '~':
case 'u':
case 'U':
case '?':
case '@':
nv_operator(cap);
break;
case 'd':
case 'D':
nv_gd(oap, cap->nchar, (int)cap->count0);
break;
case K_MIDDLEMOUSE:
case K_MIDDLEDRAG:
case K_MIDDLERELEASE:
case K_LEFTMOUSE:
case K_LEFTDRAG:
case K_LEFTRELEASE:
case K_RIGHTMOUSE:
case K_RIGHTDRAG:
case K_RIGHTRELEASE:
case K_X1MOUSE:
case K_X1DRAG:
case K_X1RELEASE:
case K_X2MOUSE:
case K_X2DRAG:
case K_X2RELEASE:
mod_mask = MOD_MASK_CTRL;
(void)do_mouse(oap, cap->nchar, BACKWARD, cap->count1, 0);
break;
case K_IGNORE:
break;
case 'p':
case 'P':
nv_put(cap);
break;
case 'o':
goto_byte(cap->count0);
break;
case 'Q':
if (text_locked()) {
clearopbeep(cap->oap);
text_locked_msg();
break;
}
if (!checkclearopq(oap))
do_exmode(true);
break;
case ',':
nv_pcmark(cap);
break;
case ';':
cap->count1 = -cap->count1;
nv_pcmark(cap);
break;
case 't':
if (!checkclearop(oap))
goto_tabpage((int)cap->count0);
break;
case 'T':
if (!checkclearop(oap))
goto_tabpage(-(int)cap->count1);
break;
case TAB:
if (!checkclearop(oap)) {
goto_tabpage_lastused();
}
break;
case '+':
case '-': 
if (!checkclearopq(oap))
undo_time(cap->nchar == '-' ? -cap->count1 : cap->count1,
false, false, false);
break;
default:
clearopbeep(oap);
break;
}
}
static void n_opencmd(cmdarg_T *cap)
{
if (!checkclearopq(cap->oap)) {
if (cap->cmdchar == 'O')
(void)hasFolding(curwin->w_cursor.lnum,
&curwin->w_cursor.lnum, NULL);
else
(void)hasFolding(curwin->w_cursor.lnum,
NULL, &curwin->w_cursor.lnum);
if (u_save((linenr_T)(curwin->w_cursor.lnum -
(cap->cmdchar == 'O' ? 1 : 0)),
(linenr_T)(curwin->w_cursor.lnum +
(cap->cmdchar == 'o' ? 1 : 0))
)
&& open_line(cap->cmdchar == 'O' ? BACKWARD : FORWARD,
has_format_option(FO_OPEN_COMS)
? OPENLINE_DO_COM : 0,
0)) {
if (win_cursorline_standout(curwin)) {
curwin->w_valid &= ~VALID_CROW;
}
invoke_edit(cap, false, cap->cmdchar, true);
}
}
}
static void nv_dot(cmdarg_T *cap)
{
if (!checkclearopq(cap->oap)) {
if (start_redo(cap->count0, restart_edit != 0 && !arrow_used) == false)
clearopbeep(cap->oap);
}
}
static void nv_redo(cmdarg_T *cap)
{
if (!checkclearopq(cap->oap)) {
u_redo((int)cap->count1);
curwin->w_set_curswant = true;
}
}
static void nv_Undo(cmdarg_T *cap)
{
if (cap->oap->op_type == OP_UPPER
|| VIsual_active
) {
cap->cmdchar = 'g';
cap->nchar = 'U';
nv_operator(cap);
} else if (!checkclearopq(cap->oap)) {
u_undoline();
curwin->w_set_curswant = true;
}
}
static void nv_tilde(cmdarg_T *cap)
{
if (!p_to
&& !VIsual_active
&& cap->oap->op_type != OP_TILDE) {
if (bt_prompt(curbuf) && !prompt_curpos_editable()) {
clearopbeep(cap->oap);
return;
}
n_swapchar(cap);
} else {
nv_operator(cap);
}
}
static void nv_operator(cmdarg_T *cap)
{
int op_type;
op_type = get_op_type(cap->cmdchar, cap->nchar);
if (bt_prompt(curbuf) && op_is_change(op_type)
&& !prompt_curpos_editable()) {
clearopbeep(cap->oap);
return;
}
if (op_type == cap->oap->op_type) 
nv_lineop(cap);
else if (!checkclearop(cap->oap)) {
cap->oap->start = curwin->w_cursor;
cap->oap->op_type = op_type;
set_op_var(op_type);
}
}
static void set_op_var(int optype)
{
if (optype == OP_NOP) {
set_vim_var_string(VV_OP, NULL, 0);
} else {
char opchars[3];
int opchar0 = get_op_char(optype);
assert(opchar0 >= 0 && opchar0 <= UCHAR_MAX);
opchars[0] = (char) opchar0;
int opchar1 = get_extra_op_char(optype);
assert(opchar1 >= 0 && opchar1 <= UCHAR_MAX);
opchars[1] = (char) opchar1;
opchars[2] = NUL;
set_vim_var_string(VV_OP, opchars, -1);
}
}
static void nv_lineop(cmdarg_T *cap)
{
cap->oap->motion_type = kMTLineWise;
if (cursor_down(cap->count1 - 1L, cap->oap->op_type == OP_NOP) == false) {
clearopbeep(cap->oap);
} else if ((cap->oap->op_type == OP_DELETE
&& cap->oap->motion_force != 'v'
&& cap->oap->motion_force != Ctrl_V)
|| cap->oap->op_type == OP_LSHIFT
|| cap->oap->op_type == OP_RSHIFT) {
beginline(BL_SOL | BL_FIX);
} else if (cap->oap->op_type != OP_YANK) { 
beginline(BL_WHITE | BL_FIX);
}
}
static void nv_home(cmdarg_T *cap)
{
if (mod_mask & MOD_MASK_CTRL)
nv_goto(cap);
else {
cap->count0 = 1;
nv_pipe(cap);
}
ins_at_eol = false; 
}
static void nv_pipe(cmdarg_T *cap)
{
cap->oap->motion_type = kMTCharWise;
cap->oap->inclusive = false;
beginline(0);
if (cap->count0 > 0) {
coladvance((colnr_T)(cap->count0 - 1));
curwin->w_curswant = (colnr_T)(cap->count0 - 1);
} else
curwin->w_curswant = 0;
curwin->w_set_curswant = false;
}
static void nv_bck_word(cmdarg_T *cap)
{
cap->oap->motion_type = kMTCharWise;
cap->oap->inclusive = false;
curwin->w_set_curswant = true;
if (bck_word(cap->count1, cap->arg, false) == false)
clearopbeep(cap->oap);
else if ((fdo_flags & FDO_HOR) && KeyTyped && cap->oap->op_type == OP_NOP)
foldOpenCursor();
}
static void nv_wordcmd(cmdarg_T *cap)
{
int n;
bool word_end;
bool flag = false;
pos_T startpos = curwin->w_cursor;
if (cap->cmdchar == 'e' || cap->cmdchar == 'E')
word_end = true;
else
word_end = false;
cap->oap->inclusive = word_end;
if (!word_end && cap->oap->op_type == OP_CHANGE) {
n = gchar_cursor();
if (n != NUL && !ascii_iswhite(n)) {
if (vim_strchr(p_cpo, CPO_CHANGEW) != NULL) {
cap->oap->inclusive = true;
word_end = true;
}
flag = true;
}
}
cap->oap->motion_type = kMTCharWise;
curwin->w_set_curswant = true;
if (word_end)
n = end_word(cap->count1, cap->arg, flag, false);
else
n = fwd_word(cap->count1, cap->arg, cap->oap->op_type != OP_NOP);
if (lt(startpos, curwin->w_cursor))
adjust_cursor(cap->oap);
if (n == false && cap->oap->op_type == OP_NOP)
clearopbeep(cap->oap);
else {
adjust_for_sel(cap);
if ((fdo_flags & FDO_HOR) && KeyTyped && cap->oap->op_type == OP_NOP)
foldOpenCursor();
}
}
static void adjust_cursor(oparg_T *oap)
{
if (curwin->w_cursor.col > 0 && gchar_cursor() == NUL
&& (!VIsual_active || *p_sel == 'o')
&& !virtual_active() && (ve_flags & VE_ONEMORE) == 0
) {
curwin->w_cursor.col--;
mb_adjust_cursor();
oap->inclusive = true;
}
}
static void nv_beginline(cmdarg_T *cap)
{
cap->oap->motion_type = kMTCharWise;
cap->oap->inclusive = false;
beginline(cap->arg);
if ((fdo_flags & FDO_HOR) && KeyTyped && cap->oap->op_type == OP_NOP)
foldOpenCursor();
ins_at_eol = false; 
}
static void adjust_for_sel(cmdarg_T *cap)
{
if (VIsual_active && cap->oap->inclusive && *p_sel == 'e'
&& gchar_cursor() != NUL && lt(VIsual, curwin->w_cursor)) {
inc_cursor();
cap->oap->inclusive = false;
}
}
static bool unadjust_for_sel(void)
{
pos_T *pp;
if (*p_sel == 'e' && !equalpos(VIsual, curwin->w_cursor)) {
if (lt(VIsual, curwin->w_cursor))
pp = &curwin->w_cursor;
else
pp = &VIsual;
if (pp->coladd > 0) {
pp->coladd--;
} else if (pp->col > 0) {
pp->col--;
mark_mb_adjustpos(curbuf, pp);
} else if (pp->lnum > 1) {
--pp->lnum;
pp->col = (colnr_T)STRLEN(ml_get(pp->lnum));
return true;
}
}
return false;
}
static void nv_select(cmdarg_T *cap)
{
if (VIsual_active)
VIsual_select = true;
else if (VIsual_reselect) {
cap->nchar = 'v'; 
cap->arg = true;
nv_g_cmd(cap);
}
}
static void nv_goto(cmdarg_T *cap)
{
linenr_T lnum;
if (cap->arg)
lnum = curbuf->b_ml.ml_line_count;
else
lnum = 1L;
cap->oap->motion_type = kMTLineWise;
setpcmark();
if (cap->count0 != 0)
lnum = cap->count0;
if (lnum < 1L)
lnum = 1L;
else if (lnum > curbuf->b_ml.ml_line_count)
lnum = curbuf->b_ml.ml_line_count;
curwin->w_cursor.lnum = lnum;
beginline(BL_SOL | BL_FIX);
if ((fdo_flags & FDO_JUMP) && KeyTyped && cap->oap->op_type == OP_NOP)
foldOpenCursor();
}
static void nv_normal(cmdarg_T *cap)
{
if (cap->nchar == Ctrl_N || cap->nchar == Ctrl_G) {
clearop(cap->oap);
if (restart_edit != 0 && mode_displayed)
clear_cmdline = true; 
restart_edit = 0;
if (cmdwin_type != 0)
cmdwin_result = Ctrl_C;
if (VIsual_active) {
end_visual_mode(); 
redraw_curbuf_later(INVERTED);
}
if (cap->nchar == Ctrl_G && p_im)
restart_edit = 'a';
} else
clearopbeep(cap->oap);
}
static void nv_esc(cmdarg_T *cap)
{
int no_reason;
no_reason = (cap->oap->op_type == OP_NOP
&& cap->opcount == 0
&& cap->count0 == 0
&& cap->oap->regname == 0
&& !p_im);
if (cap->arg) { 
if (restart_edit == 0
&& cmdwin_type == 0
&& !VIsual_active
&& no_reason) {
if (anyBufIsChanged()) {
MSG(_("Type :qa! and press <Enter> to abandon all changes"
" and exit Nvim"));
} else {
MSG(_("Type :qa and press <Enter> to exit Nvim"));
}
}
if (!p_im)
restart_edit = 0;
if (cmdwin_type != 0) {
cmdwin_result = K_IGNORE;
got_int = false; 
return;
}
}
if (VIsual_active) {
end_visual_mode(); 
check_cursor_col(); 
curwin->w_set_curswant = true;
redraw_curbuf_later(INVERTED);
} else if (no_reason) {
vim_beep(BO_ESC);
}
clearop(cap->oap);
if (restart_edit == 0 && goto_im()
&& ex_normal_busy == 0
)
restart_edit = 'a';
}
void set_cursor_for_append_to_line(void)
{
curwin->w_set_curswant = true;
if (ve_flags == VE_ALL) {
const int save_State = State;
State = INSERT;
coladvance((colnr_T)MAXCOL);
State = save_State;
} else {
curwin->w_cursor.col += (colnr_T)STRLEN(get_cursor_pos_ptr());
}
}
static void nv_edit(cmdarg_T *cap)
{
if (cap->cmdchar == K_INS || cap->cmdchar == K_KINS) {
cap->cmdchar = 'i';
}
if (VIsual_active && (cap->cmdchar == 'A' || cap->cmdchar == 'I')) {
v_visop(cap);
} else if ((cap->cmdchar == 'a' || cap->cmdchar == 'i')
&& (cap->oap->op_type != OP_NOP || VIsual_active)) {
nv_object(cap);
} else if (!curbuf->b_p_ma && !p_im && !curbuf->terminal) {
EMSG(_(e_modifiable));
clearop(cap->oap);
} else if (!checkclearopq(cap->oap)) {
switch (cap->cmdchar) {
case 'A': 
set_cursor_for_append_to_line();
break;
case 'I': 
beginline(BL_WHITE);
break;
case 'a': 
if (virtual_active()
&& (curwin->w_cursor.coladd > 0
|| *get_cursor_pos_ptr() == NUL
|| *get_cursor_pos_ptr() == TAB))
curwin->w_cursor.coladd++;
else if (*get_cursor_pos_ptr() != NUL)
inc_cursor();
break;
}
if (curwin->w_cursor.coladd && cap->cmdchar != 'A') {
int save_State = State;
State = INSERT;
coladvance(getviscol());
State = save_State;
}
invoke_edit(cap, false, cap->cmdchar, false);
}
}
static void
invoke_edit (
cmdarg_T *cap,
int repl, 
int cmd,
int startln
)
{
int restart_edit_save = 0;
if (repl || !stuff_empty())
restart_edit_save = restart_edit;
else
restart_edit_save = 0;
restart_edit = 0;
if (edit(cmd, startln, cap->count1))
cap->retval |= CA_COMMAND_BUSY;
if (restart_edit == 0)
restart_edit = restart_edit_save;
}
static void nv_object(cmdarg_T *cap)
{
bool flag;
bool include;
char_u *mps_save;
if (cap->cmdchar == 'i')
include = false; 
else
include = true; 
mps_save = curbuf->b_p_mps;
curbuf->b_p_mps = (char_u *)"(:),{:},[:],<:>";
switch (cap->nchar) {
case 'w': 
flag = current_word(cap->oap, cap->count1, include, false);
break;
case 'W': 
flag = current_word(cap->oap, cap->count1, include, true);
break;
case 'b': 
case '(':
case ')':
flag = current_block(cap->oap, cap->count1, include, '(', ')');
break;
case 'B': 
case '{':
case '}':
flag = current_block(cap->oap, cap->count1, include, '{', '}');
break;
case '[': 
case ']':
flag = current_block(cap->oap, cap->count1, include, '[', ']');
break;
case '<': 
case '>':
flag = current_block(cap->oap, cap->count1, include, '<', '>');
break;
case 't': 
cap->retval |= CA_NO_ADJ_OP_END;
flag = current_tagblock(cap->oap, cap->count1, include);
break;
case 'p': 
flag = current_par(cap->oap, cap->count1, include, 'p');
break;
case 's': 
flag = current_sent(cap->oap, cap->count1, include);
break;
case '"': 
case '\'': 
case '`': 
flag = current_quote(cap->oap, cap->count1, include,
cap->nchar);
break;
default:
flag = false;
break;
}
curbuf->b_p_mps = mps_save;
if (!flag)
clearopbeep(cap->oap);
adjust_cursor_col();
curwin->w_set_curswant = true;
}
static void nv_record(cmdarg_T *cap)
{
if (cap->oap->op_type == OP_FORMAT) {
cap->cmdchar = 'g';
cap->nchar = 'q';
nv_operator(cap);
} else if (!checkclearop(cap->oap)) {
if (cap->nchar == ':' || cap->nchar == '/' || cap->nchar == '?') {
stuffcharReadbuff(cap->nchar);
stuffcharReadbuff(K_CMDWIN);
} else {
if (reg_executing == 0 && do_record(cap->nchar) == FAIL) {
clearopbeep(cap->oap);
}
}
}
}
static void nv_at(cmdarg_T *cap)
{
if (checkclearop(cap->oap))
return;
if (cap->nchar == '=') {
if (get_expr_register() == NUL)
return;
}
while (cap->count1-- && !got_int) {
if (do_execreg(cap->nchar, false, false, false) == false) {
clearopbeep(cap->oap);
break;
}
line_breakcheck();
}
}
static void nv_halfpage(cmdarg_T *cap)
{
if ((cap->cmdchar == Ctrl_U && curwin->w_cursor.lnum == 1)
|| (cap->cmdchar == Ctrl_D
&& curwin->w_cursor.lnum == curbuf->b_ml.ml_line_count))
clearopbeep(cap->oap);
else if (!checkclearop(cap->oap))
halfpage(cap->cmdchar == Ctrl_D, cap->count0);
}
static void nv_join(cmdarg_T *cap)
{
if (VIsual_active) { 
nv_operator(cap);
} else if (!checkclearop(cap->oap)) {
if (cap->count0 <= 1) {
cap->count0 = 2; 
}
if (curwin->w_cursor.lnum + cap->count0 - 1 >
curbuf->b_ml.ml_line_count) {
if (cap->count0 <= 2) {
clearopbeep(cap->oap);
return;
}
cap->count0 = curbuf->b_ml.ml_line_count - curwin->w_cursor.lnum + 1;
}
prep_redo(cap->oap->regname, cap->count0,
NUL, cap->cmdchar, NUL, NUL, cap->nchar);
do_join((size_t)cap->count0, cap->nchar == NUL, true, true, true);
}
}
static void nv_put(cmdarg_T *cap)
{
nv_put_opt(cap, false);
}
static void nv_put_opt(cmdarg_T *cap, bool fix_indent)
{
int regname = 0;
yankreg_T *savereg = NULL;
bool empty = false;
bool was_visual = false;
int dir;
int flags = 0;
if (cap->oap->op_type != OP_NOP) {
if (cap->oap->op_type == OP_DELETE && cap->cmdchar == 'p') {
clearop(cap->oap);
assert(cap->opcount >= 0);
nv_diffgetput(true, (size_t)cap->opcount);
} else {
clearopbeep(cap->oap);
}
} else if (bt_prompt(curbuf) && !prompt_curpos_editable()) {
clearopbeep(cap->oap);
} else {
if (fix_indent) {
dir = (cap->cmdchar == ']' && cap->nchar == 'p')
? FORWARD : BACKWARD;
flags |= PUT_FIXINDENT;
} else {
dir = (cap->cmdchar == 'P'
|| (cap->cmdchar == 'g' && cap->nchar == 'P'))
? BACKWARD : FORWARD;
}
prep_redo_cmd(cap);
if (cap->cmdchar == 'g') {
flags |= PUT_CURSEND;
}
if (VIsual_active) {
was_visual = true;
regname = cap->oap->regname;
bool clipoverwrite = (regname == '+' || regname == '*')
&& (cb_flags & CB_UNNAMEDMASK);
if (regname == 0 || regname == '"' || clipoverwrite
|| ascii_isdigit(regname) || regname == '-') {
savereg = copy_register(regname);
}
if (!VIsual_active || VIsual_mode == 'V' || regname != '.') {
cap->cmdchar = 'd';
cap->nchar = NUL;
cap->oap->regname = NUL;
msg_silent++;
nv_operator(cap);
do_pending_operator(cap, 0, false);
empty = (curbuf->b_ml.ml_flags & ML_EMPTY);
msg_silent--;
cap->oap->regname = regname;
}
if (VIsual_mode == 'V') {
flags |= PUT_LINE;
} else if (VIsual_mode == 'v') {
flags |= PUT_LINE_SPLIT;
}
if (VIsual_mode == Ctrl_V && dir == FORWARD) {
flags |= PUT_LINE_FORWARD;
}
dir = BACKWARD;
if ((VIsual_mode != 'V'
&& curwin->w_cursor.col < curbuf->b_op_start.col)
|| (VIsual_mode == 'V'
&& curwin->w_cursor.lnum < curbuf->b_op_start.lnum))
dir = FORWARD;
VIsual_active = true;
}
do_put(cap->oap->regname, savereg, dir, cap->count1, flags);
if (savereg != NULL) {
free_register(savereg);
xfree(savereg);
}
if (was_visual) {
curbuf->b_visual.vi_start = curbuf->b_op_start;
curbuf->b_visual.vi_end = curbuf->b_op_end;
if (*p_sel == 'e') {
inc(&curbuf->b_visual.vi_end);
}
}
if (empty && *ml_get(curbuf->b_ml.ml_line_count) == NUL) {
ml_delete(curbuf->b_ml.ml_line_count, true);
deleted_lines(curbuf->b_ml.ml_line_count + 1, 1);
if (curwin->w_cursor.lnum > curbuf->b_ml.ml_line_count) {
curwin->w_cursor.lnum = curbuf->b_ml.ml_line_count;
coladvance((colnr_T)MAXCOL);
}
}
auto_format(false, true);
}
}
static void nv_open(cmdarg_T *cap)
{
if (cap->oap->op_type == OP_DELETE && cap->cmdchar == 'o') {
clearop(cap->oap);
assert(cap->opcount >= 0);
nv_diffgetput(false, (size_t)cap->opcount);
} else if (VIsual_active) {
v_swap_corners(cap->cmdchar);
} else if (bt_prompt(curbuf)) {
clearopbeep(cap->oap);
} else {
n_opencmd(cap);
}
}
static void get_op_vcol(
oparg_T *oap,
colnr_T redo_VIsual_vcol,
bool initial 
)
{
colnr_T start;
colnr_T end;
if (VIsual_mode != Ctrl_V
|| (!initial && oap->end.col < curwin->w_width_inner)) {
return;
}
oap->motion_type = kMTBlockWise;
mark_mb_adjustpos(curwin->w_buffer, &oap->end);
getvvcol(curwin, &(oap->start), &oap->start_vcol, NULL, &oap->end_vcol);
if (!redo_VIsual_busy) {
getvvcol(curwin, &(oap->end), &start, NULL, &end);
if (start < oap->start_vcol) {
oap->start_vcol = start;
}
if (end > oap->end_vcol) {
if (initial && *p_sel == 'e'
&& start >= 1
&& start - 1 >= oap->end_vcol) {
oap->end_vcol = start - 1;
} else {
oap->end_vcol = end;
}
}
}
if (curwin->w_curswant == MAXCOL) {
curwin->w_cursor.col = MAXCOL;
oap->end_vcol = 0;
for (curwin->w_cursor.lnum = oap->start.lnum;
curwin->w_cursor.lnum <= oap->end.lnum; ++curwin->w_cursor.lnum) {
getvvcol(curwin, &curwin->w_cursor, NULL, NULL, &end);
if (end > oap->end_vcol) {
oap->end_vcol = end;
}
}
} else if (redo_VIsual_busy) {
oap->end_vcol = oap->start_vcol + redo_VIsual_vcol - 1;
}
curwin->w_cursor.lnum = oap->end.lnum;
coladvance(oap->end_vcol);
oap->end = curwin->w_cursor;
curwin->w_cursor = oap->start;
coladvance(oap->start_vcol);
oap->start = curwin->w_cursor;
}
static void nv_event(cmdarg_T *cap)
{
may_garbage_collect = false;
bool may_restart = (restart_edit != 0);
multiqueue_process_events(main_loop.events);
finish_op = false;
if (may_restart) {
cap->retval |= CA_COMMAND_BUSY; 
}
}
static int mouse_model_popup(void)
{
return p_mousem[0] == 'p';
}
void normal_cmd(oparg_T *oap, bool toplevel)
{
NormalState s;
normal_state_init(&s);
s.toplevel = toplevel;
s.oa = *oap;
normal_prepare(&s);
(void)normal_execute(&s.state, safe_vgetc());
*oap = s.oa;
}
