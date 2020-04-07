#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include "nvim/api/private/handle.h"
#include "nvim/api/private/helpers.h"
#include "nvim/vim.h"
#include "nvim/ascii.h"
#include "nvim/window.h"
#include "nvim/buffer.h"
#include "nvim/charset.h"
#include "nvim/cursor.h"
#include "nvim/diff.h"
#include "nvim/edit.h"
#include "nvim/eval.h"
#include "nvim/ex_cmds.h"
#include "nvim/ex_cmds2.h"
#include "nvim/ex_docmd.h"
#include "nvim/ex_eval.h"
#include "nvim/ex_getln.h"
#include "nvim/fileio.h"
#include "nvim/fold.h"
#include "nvim/getchar.h"
#include "nvim/hashtab.h"
#include "nvim/main.h"
#include "nvim/mark.h"
#include "nvim/memline.h"
#include "nvim/memory.h"
#include "nvim/message.h"
#include "nvim/misc1.h"
#include "nvim/file_search.h"
#include "nvim/garray.h"
#include "nvim/move.h"
#include "nvim/mouse.h"
#include "nvim/normal.h"
#include "nvim/option.h"
#include "nvim/os_unix.h"
#include "nvim/path.h"
#include "nvim/quickfix.h"
#include "nvim/regexp.h"
#include "nvim/screen.h"
#include "nvim/search.h"
#include "nvim/state.h"
#include "nvim/strings.h"
#include "nvim/syntax.h"
#include "nvim/terminal.h"
#include "nvim/undo.h"
#include "nvim/ui.h"
#include "nvim/ui_compositor.h"
#include "nvim/os/os.h"
#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "window.c.generated.h"
#endif
#define NOWIN (win_T *)-1 
#define ROWS_AVAIL (Rows - p_ch - tabline_height())
static char *m_onlyone = N_("Already only one window");
void
do_window(
int nchar,
long Prenum,
int xchar 
)
{
long Prenum1;
win_T *wp;
char_u *ptr;
linenr_T lnum = -1;
int type = FIND_DEFINE;
size_t len;
char cbuf[40];
Prenum1 = Prenum == 0 ? 1 : Prenum;
#define CHECK_CMDWIN do { if (cmdwin_type != 0) { EMSG(_(e_cmdwin)); return; } } while (0)
switch (nchar) {
case 'S':
case Ctrl_S:
case 's':
CHECK_CMDWIN;
reset_VIsual_and_resel(); 
if (bt_quickfix(curbuf)) {
goto newwindow;
}
(void)win_split((int)Prenum, 0);
break;
case Ctrl_V:
case 'v':
CHECK_CMDWIN;
reset_VIsual_and_resel(); 
if (bt_quickfix(curbuf)) {
goto newwindow;
}
(void)win_split((int)Prenum, WSP_VERT);
break;
case Ctrl_HAT:
case '^':
CHECK_CMDWIN;
reset_VIsual_and_resel(); 
if (buflist_findnr(Prenum == 0 ? curwin->w_alt_fnum : Prenum) == NULL) {
if (Prenum == 0) {
EMSG(_(e_noalt));
} else {
EMSGN(_("E92: Buffer %" PRId64 " not found"), Prenum);
}
break;
}
if (!curbuf_locked() && win_split(0, 0) == OK) {
(void)buflist_getfile(Prenum == 0 ? curwin->w_alt_fnum : Prenum,
(linenr_T)0, GETF_ALT, false);
}
break;
case Ctrl_N:
case 'n':
CHECK_CMDWIN;
reset_VIsual_and_resel(); 
newwindow:
if (Prenum)
vim_snprintf(cbuf, sizeof(cbuf) - 5, "%" PRId64, (int64_t)Prenum);
else
cbuf[0] = NUL;
if (nchar == 'v' || nchar == Ctrl_V) {
xstrlcat(cbuf, "v", sizeof(cbuf));
}
xstrlcat(cbuf, "new", sizeof(cbuf));
do_cmdline_cmd(cbuf);
break;
case Ctrl_Q:
case 'q':
reset_VIsual_and_resel(); 
cmd_with_count("quit", (char_u *)cbuf, sizeof(cbuf), Prenum);
do_cmdline_cmd(cbuf);
break;
case Ctrl_C:
case 'c':
reset_VIsual_and_resel(); 
cmd_with_count("close", (char_u *)cbuf, sizeof(cbuf), Prenum);
do_cmdline_cmd(cbuf);
break;
case Ctrl_Z:
case 'z':
CHECK_CMDWIN;
reset_VIsual_and_resel(); 
do_cmdline_cmd("pclose");
break;
case 'P':
wp = NULL;
FOR_ALL_WINDOWS_IN_TAB(wp2, curtab) {
if (wp2->w_p_pvw) {
wp = wp2;
break;
}
}
if (wp == NULL) {
EMSG(_("E441: There is no preview window"));
} else {
win_goto(wp);
}
break;
case Ctrl_O:
case 'o':
CHECK_CMDWIN;
reset_VIsual_and_resel(); 
cmd_with_count("only", (char_u *)cbuf, sizeof(cbuf), Prenum);
do_cmdline_cmd(cbuf);
break;
case Ctrl_W:
case 'w':
case 'W':
CHECK_CMDWIN;
if (ONE_WINDOW && Prenum != 1) { 
beep_flush();
} else {
if (Prenum) { 
for (wp = firstwin; --Prenum > 0; ) {
if (wp->w_next == NULL)
break;
else
wp = wp->w_next;
}
} else {
if (nchar == 'W') { 
wp = curwin->w_prev;
if (wp == NULL) {
wp = lastwin; 
}
while (wp != NULL && wp->w_floating
&& !wp->w_float_config.focusable) {
wp = wp->w_prev;
}
} else { 
wp = curwin->w_next;
while (wp != NULL && wp->w_floating
&& !wp->w_float_config.focusable) {
wp = wp->w_next;
}
if (wp == NULL) {
wp = firstwin; 
}
}
}
win_goto(wp);
}
break;
case 'j':
case K_DOWN:
case Ctrl_J:
CHECK_CMDWIN;
win_goto_ver(false, Prenum1);
break;
case 'k':
case K_UP:
case Ctrl_K:
CHECK_CMDWIN;
win_goto_ver(true, Prenum1);
break;
case 'h':
case K_LEFT:
case Ctrl_H:
case K_BS:
CHECK_CMDWIN;
win_goto_hor(true, Prenum1);
break;
case 'l':
case K_RIGHT:
case Ctrl_L:
CHECK_CMDWIN;
win_goto_hor(false, Prenum1);
break;
case 'T':
if (one_window())
MSG(_(m_onlyone));
else {
tabpage_T *oldtab = curtab;
tabpage_T *newtab;
wp = curwin;
if (win_new_tabpage((int)Prenum, NULL) == OK
&& valid_tabpage(oldtab)) {
newtab = curtab;
goto_tabpage_tp(oldtab, true, true);
if (curwin == wp) {
win_close(curwin, false);
}
if (valid_tabpage(newtab)) {
goto_tabpage_tp(newtab, true, true);
apply_autocmds(EVENT_TABNEWENTERED, NULL, NULL, false, curbuf);
}
}
}
break;
case 't':
case Ctrl_T:
win_goto(firstwin);
break;
case 'b':
case Ctrl_B:
win_goto(lastwin_nofloating());
break;
case 'p':
case Ctrl_P:
if (!win_valid(prevwin)) {
beep_flush();
} else {
win_goto(prevwin);
}
break;
case 'x':
case Ctrl_X:
CHECK_CMDWIN;
win_exchange(Prenum);
break;
case Ctrl_R:
case 'r':
CHECK_CMDWIN;
reset_VIsual_and_resel(); 
win_rotate(false, (int)Prenum1); 
break;
case 'R':
CHECK_CMDWIN;
reset_VIsual_and_resel(); 
win_rotate(true, (int)Prenum1); 
break;
case 'K':
case 'J':
case 'H':
case 'L':
CHECK_CMDWIN;
win_totop((int)Prenum,
((nchar == 'H' || nchar == 'L') ? WSP_VERT : 0)
| ((nchar == 'H' || nchar == 'K') ? WSP_TOP : WSP_BOT));
break;
case '=':
win_equal(NULL, false, 'b');
break;
case '+':
win_setheight(curwin->w_height + (int)Prenum1);
break;
case '-':
win_setheight(curwin->w_height - (int)Prenum1);
break;
case Ctrl__:
case '_':
win_setheight(Prenum ? (int)Prenum : Rows-1);
break;
case '>':
win_setwidth(curwin->w_width + (int)Prenum1);
break;
case '<':
win_setwidth(curwin->w_width - (int)Prenum1);
break;
case '|':
win_setwidth(Prenum != 0 ? (int)Prenum : Columns);
break;
case '}':
CHECK_CMDWIN;
if (Prenum) {
g_do_tagpreview = Prenum;
} else {
g_do_tagpreview = p_pvh;
}
FALLTHROUGH;
case ']':
case Ctrl_RSB:
CHECK_CMDWIN;
if (Prenum)
postponed_split = Prenum;
else
postponed_split = -1;
if (nchar != '}') {
g_do_tagpreview = 0;
}
do_nv_ident(Ctrl_RSB, NUL);
break;
case 'f':
case 'F':
case Ctrl_F:
wingotofile:
CHECK_CMDWIN;
ptr = grab_file_name(Prenum1, &lnum);
if (ptr != NULL) {
tabpage_T *oldtab = curtab;
win_T *oldwin = curwin;
setpcmark();
if (win_split(0, 0) == OK) {
RESET_BINDING(curwin);
if (do_ecmd(0, ptr, NULL, NULL, ECMD_LASTL, ECMD_HIDE, NULL) == FAIL) {
win_close(curwin, false);
goto_tabpage_win(oldtab, oldwin);
} else if (nchar == 'F' && lnum >= 0) {
curwin->w_cursor.lnum = lnum;
check_cursor_lnum();
beginline(BL_SOL | BL_FIX);
}
}
xfree(ptr);
}
break;
case 'i': 
case Ctrl_I:
type = FIND_ANY;
FALLTHROUGH;
case 'd': 
case Ctrl_D:
CHECK_CMDWIN;
if ((len = find_ident_under_cursor(&ptr, FIND_IDENT)) == 0) {
break;
}
find_pattern_in_path(ptr, 0, len, true, Prenum == 0,
type, Prenum1, ACTION_SPLIT, 1, MAXLNUM);
curwin->w_set_curswant = TRUE;
break;
case K_KENTER:
case CAR:
if (bt_quickfix(curbuf)) {
qf_view_result(true);
}
break;
case 'g':
case Ctrl_G:
CHECK_CMDWIN;
no_mapping++;
if (xchar == NUL) {
xchar = plain_vgetc();
}
LANGMAP_ADJUST(xchar, true);
no_mapping--;
(void)add_to_showcmd(xchar);
switch (xchar) {
case '}':
xchar = Ctrl_RSB;
if (Prenum)
g_do_tagpreview = Prenum;
else
g_do_tagpreview = p_pvh;
FALLTHROUGH;
case ']':
case Ctrl_RSB:
if (Prenum)
postponed_split = Prenum;
else
postponed_split = -1;
do_nv_ident('g', xchar);
break;
case TAB:
goto_tabpage_lastused();
break;
case 'f': 
case 'F': 
cmdmod.tab = tabpage_index(curtab) + 1;
nchar = xchar;
goto wingotofile;
case 't': 
goto_tabpage((int)Prenum);
break;
case 'T': 
goto_tabpage(-(int)Prenum1);
break;
case 'e':
if (curwin->w_floating || !ui_has(kUIMultigrid)) {
beep_flush();
break;
}
FloatConfig config = FLOAT_CONFIG_INIT;
config.width = curwin->w_width;
config.height = curwin->w_height;
config.external = true;
Error err = ERROR_INIT;
if (!win_new_float(curwin, config, &err)) {
EMSG(err.msg);
api_clear_error(&err);
beep_flush();
}
break;
default:
beep_flush();
break;
}
break;
default: beep_flush();
break;
}
}
static void cmd_with_count(char *cmd, char_u *bufp, size_t bufsize,
int64_t Prenum)
{
size_t len = xstrlcpy((char *)bufp, cmd, bufsize);
if (Prenum > 0 && len < bufsize) {
vim_snprintf((char *)bufp + len, bufsize - len, "%" PRId64, Prenum);
}
}
win_T *win_new_float(win_T *wp, FloatConfig fconfig, Error *err)
{
if (wp == NULL) {
wp = win_alloc(lastwin_nofloating(), false);
win_init(wp, curwin, 0);
} else {
assert(!wp->w_floating);
if (firstwin == wp && lastwin_nofloating() == wp) {
api_set_error(err, kErrorTypeException,
"Cannot change last window into float");
return NULL;
} else if (!win_valid(wp)) {
api_set_error(err, kErrorTypeException,
"Cannot change window from different tabpage into float");
return NULL;
}
int dir;
winframe_remove(wp, &dir, NULL);
XFREE_CLEAR(wp->w_frame);
(void)win_comp_pos(); 
win_remove(wp, NULL);
win_append(lastwin_nofloating(), wp);
}
wp->w_floating = 1;
wp->w_status_height = 0;
wp->w_vsep_width = 0;
win_config_float(wp, fconfig);
wp->w_pos_changed = true;
redraw_win_later(wp, VALID);
return wp;
}
void win_set_minimal_style(win_T *wp)
{
wp->w_p_nu = false;
wp->w_p_rnu = false;
wp->w_p_cul = false;
wp->w_p_cuc = false;
wp->w_p_spell = false;
wp->w_p_list = false;
if (wp->w_p_fcs_chars.eob != ' ') {
char_u *old = wp->w_p_fcs;
wp->w_p_fcs = ((*old == NUL)
? (char_u *)xstrdup("eob: ")
: concat_str(old, (char_u *)",eob: "));
xfree(old);
}
if (wp->w_hl_ids[HLF_EOB] != -1) {
char_u *old = wp->w_p_winhl;
wp->w_p_winhl = ((*old == NUL)
? (char_u *)xstrdup("EndOfBuffer:")
: concat_str(old, (char_u *)",EndOfBuffer:"));
xfree(old);
}
if (wp->w_p_scl[0] != 'a') {
xfree(wp->w_p_scl);
wp->w_p_scl = (char_u *)xstrdup("auto");
}
if (wp->w_p_fdc[0] != '0') {
xfree(wp->w_p_fdc);
wp->w_p_fdc = (char_u *)xstrdup("0");
}
if (wp->w_p_cc != NULL && *wp->w_p_cc != NUL) {
xfree(wp->w_p_cc);
wp->w_p_cc = (char_u *)xstrdup("");
}
}
void win_config_float(win_T *wp, FloatConfig fconfig)
{
wp->w_width = MAX(fconfig.width, 1);
wp->w_height = MAX(fconfig.height, 1);
if (fconfig.relative == kFloatRelativeCursor) {
fconfig.relative = kFloatRelativeWindow;
fconfig.row += curwin->w_wrow;
fconfig.col += curwin->w_wcol;
fconfig.window = curwin->handle;
}
bool change_external = fconfig.external != wp->w_float_config.external;
wp->w_float_config = fconfig;
if (!ui_has(kUIMultigrid)) {
wp->w_height = MIN(wp->w_height, Rows-1);
wp->w_width = MIN(wp->w_width, Columns);
}
win_set_inner_size(wp);
must_redraw = MAX(must_redraw, VALID);
wp->w_pos_changed = true;
if (change_external) {
wp->w_hl_needs_update = true;
redraw_win_later(wp, NOT_VALID);
}
}
void win_check_anchored_floats(win_T *win)
{
for (win_T *wp = lastwin; wp && wp->w_floating; wp = wp->w_prev) {
if (wp->w_float_config.relative == kFloatRelativeWindow
&& wp->w_float_config.window == win->handle) {
wp->w_pos_changed = true;
}
}
}
int win_fdccol_count(win_T *wp)
{
const char *fdc = (const char *)wp->w_p_fdc;
if (strncmp(fdc, "auto:", 5) == 0) {
int needed_fdccols = getDeepestNesting(wp);
return MIN(fdc[5] - '0', needed_fdccols);
} else {
return fdc[0] - '0';
}
}
static void ui_ext_win_position(win_T *wp)
{
if (!wp->w_floating) {
ui_call_win_pos(wp->w_grid.handle, wp->handle, wp->w_winrow,
wp->w_wincol, wp->w_width, wp->w_height);
return;
}
FloatConfig c = wp->w_float_config;
if (!c.external) {
ScreenGrid *grid = &default_grid;
float row = c.row, col = c.col;
if (c.relative == kFloatRelativeWindow) {
Error dummy = ERROR_INIT;
win_T *win = find_window_by_handle(c.window, &dummy);
if (win) {
grid = &win->w_grid;
int row_off = 0, col_off = 0;
screen_adjust_grid(&grid, &row_off, &col_off);
row += row_off;
col += col_off;
if (c.bufpos.lnum >= 0) {
pos_T pos = { c.bufpos.lnum+1, c.bufpos.col, 0 };
int trow, tcol, tcolc, tcole;
textpos2screenpos(win, &pos, &trow, &tcol, &tcolc, &tcole, true);
row += trow-1;
col += tcol-1;
}
}
api_clear_error(&dummy);
}
if (ui_has(kUIMultigrid)) {
String anchor = cstr_to_string(float_anchor_str[c.anchor]);
ui_call_win_float_pos(wp->w_grid.handle, wp->handle, anchor, grid->handle,
row, col, c.focusable);
} else {
bool east = c.anchor & kFloatAnchorEast;
bool south = c.anchor & kFloatAnchorSouth;
int comp_row = (int)row - (south ? wp->w_height : 0);
int comp_col = (int)col - (east ? wp->w_width : 0);
comp_row = MAX(MIN(comp_row, Rows-wp->w_height-1), 0);
comp_col = MAX(MIN(comp_col, Columns-wp->w_width), 0);
wp->w_winrow = comp_row;
wp->w_wincol = comp_col;
bool valid = (wp->w_redr_type == 0);
bool on_top = (curwin == wp) || !curwin->w_floating;
ui_comp_put_grid(&wp->w_grid, comp_row, comp_col, wp->w_height,
wp->w_width, valid, on_top);
ui_check_cursor_grid(wp->w_grid.handle);
wp->w_grid.focusable = wp->w_float_config.focusable;
if (!valid) {
wp->w_grid.valid = false;
redraw_win_later(wp, NOT_VALID);
}
}
} else {
ui_call_win_external_pos(wp->w_grid.handle, wp->handle);
}
}
void ui_ext_win_viewport(win_T *wp)
{
if ((wp == curwin || ui_has(kUIMultigrid)) && wp->w_viewport_invalid) {
int botline = wp->w_botline;
if (botline == wp->w_buffer->b_ml.ml_line_count+1
&& wp->w_empty_rows == 0) {
botline = wp->w_buffer->b_ml.ml_line_count;
}
ui_call_win_viewport(wp->w_grid.handle, wp->handle, wp->w_topline-1,
botline, wp->w_cursor.lnum-1, wp->w_cursor.col);
wp->w_viewport_invalid = false;
}
}
static bool parse_float_anchor(String anchor, FloatAnchor *out)
{
if (anchor.size == 0) {
*out = (FloatAnchor)0;
}
char *str = anchor.data;
if (striequal(str, "NW")) {
*out = 0; 
} else if (striequal(str, "NE")) {
*out = kFloatAnchorEast;
} else if (striequal(str, "SW")) {
*out = kFloatAnchorSouth;
} else if (striequal(str, "SE")) {
*out = kFloatAnchorSouth | kFloatAnchorEast;
} else {
return false;
}
return true;
}
static bool parse_float_relative(String relative, FloatRelative *out)
{
char *str = relative.data;
if (striequal(str, "editor")) {
*out = kFloatRelativeEditor;
} else if (striequal(str, "win")) {
*out = kFloatRelativeWindow;
} else if (striequal(str, "cursor")) {
*out = kFloatRelativeCursor;
} else {
return false;
}
return true;
}
static bool parse_float_bufpos(Array bufpos, lpos_T *out)
{
if (bufpos.size != 2
|| bufpos.items[0].type != kObjectTypeInteger
|| bufpos.items[1].type != kObjectTypeInteger) {
return false;
}
out->lnum = bufpos.items[0].data.integer;
out->col = bufpos.items[1].data.integer;
return true;
}
bool parse_float_config(Dictionary config, FloatConfig *fconfig, bool reconf,
Error *err)
{
bool has_row = false, has_col = false, has_relative = false;
bool has_external = false, has_window = false;
bool has_width = false, has_height = false;
bool has_bufpos = false;
for (size_t i = 0; i < config.size; i++) {
char *key = config.items[i].key.data;
Object val = config.items[i].value;
if (!strcmp(key, "row")) {
has_row = true;
if (val.type == kObjectTypeInteger) {
fconfig->row = val.data.integer;
} else if (val.type == kObjectTypeFloat) {
fconfig->row = val.data.floating;
} else {
api_set_error(err, kErrorTypeValidation,
"'row' key must be Integer or Float");
return false;
}
} else if (!strcmp(key, "col")) {
has_col = true;
if (val.type == kObjectTypeInteger) {
fconfig->col = val.data.integer;
} else if (val.type == kObjectTypeFloat) {
fconfig->col = val.data.floating;
} else {
api_set_error(err, kErrorTypeValidation,
"'col' key must be Integer or Float");
return false;
}
} else if (strequal(key, "width")) {
has_width = true;
if (val.type == kObjectTypeInteger && val.data.integer > 0) {
fconfig->width = val.data.integer;
} else {
api_set_error(err, kErrorTypeValidation,
"'width' key must be a positive Integer");
return false;
}
} else if (strequal(key, "height")) {
has_height = true;
if (val.type == kObjectTypeInteger && val.data.integer > 0) {
fconfig->height= val.data.integer;
} else {
api_set_error(err, kErrorTypeValidation,
"'height' key must be a positive Integer");
return false;
}
} else if (!strcmp(key, "anchor")) {
if (val.type != kObjectTypeString) {
api_set_error(err, kErrorTypeValidation,
"'anchor' key must be String");
return false;
}
if (!parse_float_anchor(val.data.string, &fconfig->anchor)) {
api_set_error(err, kErrorTypeValidation,
"Invalid value of 'anchor' key");
return false;
}
} else if (!strcmp(key, "relative")) {
if (val.type != kObjectTypeString) {
api_set_error(err, kErrorTypeValidation,
"'relative' key must be String");
return false;
}
if (val.data.string.size > 0) {
has_relative = true;
if (!parse_float_relative(val.data.string, &fconfig->relative)) {
api_set_error(err, kErrorTypeValidation,
"Invalid value of 'relative' key");
return false;
}
}
} else if (!strcmp(key, "win")) {
has_window = true;
if (val.type != kObjectTypeInteger
&& val.type != kObjectTypeWindow) {
api_set_error(err, kErrorTypeValidation,
"'win' key must be Integer or Window");
return false;
}
fconfig->window = val.data.integer;
} else if (!strcmp(key, "bufpos")) {
if (val.type != kObjectTypeArray) {
api_set_error(err, kErrorTypeValidation,
"'bufpos' key must be Array");
return false;
}
if (!parse_float_bufpos(val.data.array, &fconfig->bufpos)) {
api_set_error(err, kErrorTypeValidation,
"Invalid value of 'bufpos' key");
return false;
}
has_bufpos = true;
} else if (!strcmp(key, "external")) {
if (val.type == kObjectTypeInteger) {
fconfig->external = val.data.integer;
} else if (val.type == kObjectTypeBoolean) {
fconfig->external = val.data.boolean;
} else {
api_set_error(err, kErrorTypeValidation,
"'external' key must be Boolean");
return false;
}
has_external = fconfig->external;
} else if (!strcmp(key, "focusable")) {
if (val.type == kObjectTypeInteger) {
fconfig->focusable = val.data.integer;
} else if (val.type == kObjectTypeBoolean) {
fconfig->focusable = val.data.boolean;
} else {
api_set_error(err, kErrorTypeValidation,
"'focusable' key must be Boolean");
return false;
}
} else if (!strcmp(key, "style")) {
if (val.type != kObjectTypeString) {
api_set_error(err, kErrorTypeValidation,
"'style' key must be String");
return false;
}
if (val.data.string.data[0] == NUL) {
fconfig->style = kWinStyleUnused;
} else if (striequal(val.data.string.data, "minimal")) {
fconfig->style = kWinStyleMinimal;
} else {
api_set_error(err, kErrorTypeValidation,
"Invalid value of 'style' key");
}
} else {
api_set_error(err, kErrorTypeValidation,
"Invalid key '%s'", key);
return false;
}
}
if (has_window && !(has_relative
&& fconfig->relative == kFloatRelativeWindow)) {
api_set_error(err, kErrorTypeValidation,
"'win' key is only valid with relative='win'");
return false;
}
if ((has_relative && fconfig->relative == kFloatRelativeWindow)
&& (!has_window || fconfig->window == 0)) {
fconfig->window = curwin->handle;
}
if (has_window && !has_bufpos) {
fconfig->bufpos.lnum = -1;
}
if (has_bufpos) {
if (!has_row) {
fconfig->row = (fconfig->anchor & kFloatAnchorSouth) ? 0 : 1;
has_row = true;
}
if (!has_col) {
fconfig->col = 0;
has_col = true;
}
}
if (has_relative && has_external) {
api_set_error(err, kErrorTypeValidation,
"Only one of 'relative' and 'external' must be used");
return false;
} else if (!reconf && !has_relative && !has_external) {
api_set_error(err, kErrorTypeValidation,
"One of 'relative' and 'external' must be used");
return false;
} else if (has_relative) {
fconfig->external = false;
}
if (!reconf && !(has_height && has_width)) {
api_set_error(err, kErrorTypeValidation,
"Must specify 'width' and 'height'");
return false;
}
if (fconfig->external && !ui_has(kUIMultigrid)) {
api_set_error(err, kErrorTypeValidation,
"UI doesn't support external windows");
return false;
}
if (has_relative != has_row || has_row != has_col) {
api_set_error(err, kErrorTypeValidation,
"'relative' requires 'row'/'col' or 'bufpos'");
return false;
}
return true;
}
int win_split(int size, int flags)
{
if (may_open_tabpage() == OK)
return OK;
flags |= cmdmod.split;
if ((flags & WSP_TOP) && (flags & WSP_BOT)) {
EMSG(_("E442: Can't split topleft and botright at the same time"));
return FAIL;
}
if (flags & WSP_HELP)
make_snapshot(SNAP_HELP_IDX);
else
clear_snapshot(curtab, SNAP_HELP_IDX);
return win_split_ins(size, flags, NULL, 0);
}
int win_split_ins(int size, int flags, win_T *new_wp, int dir)
{
win_T *wp = new_wp;
win_T *oldwin;
int new_size = size;
int i;
int need_status = 0;
int do_equal = FALSE;
int needed;
int available;
int oldwin_height = 0;
int layout;
frame_T *frp, *curfrp, *frp2, *prevfrp;
int before;
int minheight;
int wmh1;
bool did_set_fraction = false;
if (flags & WSP_TOP) {
oldwin = firstwin;
} else if (flags & WSP_BOT || curwin->w_floating) {
oldwin = lastwin_nofloating();
} else {
oldwin = curwin;
}
bool new_in_layout = (new_wp == NULL || new_wp->w_floating);
if (one_nonfloat() && p_ls == 1 && oldwin->w_status_height == 0) {
if (oldwin->w_height <= p_wmh && new_in_layout) {
EMSG(_(e_noroom));
return FAIL;
}
need_status = STATUS_HEIGHT;
}
if (flags & WSP_VERT) {
int wmw1;
int minwidth;
layout = FR_ROW;
wmw1 = (p_wmw == 0 ? 1 : p_wmw);
needed = wmw1 + 1;
if (flags & WSP_ROOM) {
needed += p_wiw - wmw1;
}
if (flags & (WSP_BOT | WSP_TOP)) {
minwidth = frame_minwidth(topframe, NOWIN);
available = topframe->fr_width;
needed += minwidth;
} else if (p_ea) {
minwidth = frame_minwidth(oldwin->w_frame, NOWIN);
prevfrp = oldwin->w_frame;
for (frp = oldwin->w_frame->fr_parent; frp != NULL;
frp = frp->fr_parent) {
if (frp->fr_layout == FR_ROW) {
FOR_ALL_FRAMES(frp2, frp->fr_child) {
if (frp2 != prevfrp) {
minwidth += frame_minwidth(frp2, NOWIN);
}
}
}
prevfrp = frp;
}
available = topframe->fr_width;
needed += minwidth;
} else {
minwidth = frame_minwidth(oldwin->w_frame, NOWIN);
available = oldwin->w_frame->fr_width;
needed += minwidth;
}
if (available < needed && new_in_layout) {
EMSG(_(e_noroom));
return FAIL;
}
if (new_size == 0)
new_size = oldwin->w_width / 2;
if (new_size > available - minwidth - 1) {
new_size = available - minwidth - 1;
}
if (new_size < wmw1) {
new_size = wmw1;
}
if (oldwin->w_width - new_size - 1 < p_wmw)
do_equal = TRUE;
if (oldwin->w_p_wfw) {
win_setwidth_win(oldwin->w_width + new_size + 1, oldwin);
}
if (!do_equal && p_ea && size == 0 && *p_ead != 'v'
&& oldwin->w_frame->fr_parent != NULL) {
frp = oldwin->w_frame->fr_parent->fr_child;
while (frp != NULL) {
if (frp->fr_win != oldwin && frp->fr_win != NULL
&& (frp->fr_win->w_width > new_size
|| frp->fr_win->w_width > oldwin->w_width
- new_size - 1)) {
do_equal = TRUE;
break;
}
frp = frp->fr_next;
}
}
} else {
layout = FR_COL;
wmh1 = (p_wmh == 0 ? 1 : p_wmh);
needed = wmh1 + STATUS_HEIGHT;
if (flags & WSP_ROOM) {
needed += p_wh - wmh1;
}
if (flags & (WSP_BOT | WSP_TOP)) {
minheight = frame_minheight(topframe, NOWIN) + need_status;
available = topframe->fr_height;
needed += minheight;
} else if (p_ea) {
minheight = frame_minheight(oldwin->w_frame, NOWIN) + need_status;
prevfrp = oldwin->w_frame;
for (frp = oldwin->w_frame->fr_parent; frp != NULL;
frp = frp->fr_parent) {
if (frp->fr_layout == FR_COL) {
FOR_ALL_FRAMES(frp2, frp->fr_child) {
if (frp2 != prevfrp) {
minheight += frame_minheight(frp2, NOWIN);
}
}
}
prevfrp = frp;
}
available = topframe->fr_height;
needed += minheight;
} else {
minheight = frame_minheight(oldwin->w_frame, NOWIN) + need_status;
available = oldwin->w_frame->fr_height;
needed += minheight;
}
if (available < needed && new_in_layout) {
EMSG(_(e_noroom));
return FAIL;
}
oldwin_height = oldwin->w_height;
if (need_status) {
oldwin->w_status_height = STATUS_HEIGHT;
oldwin_height -= STATUS_HEIGHT;
}
if (new_size == 0)
new_size = oldwin_height / 2;
if (new_size > available - minheight - STATUS_HEIGHT) {
new_size = available - minheight - STATUS_HEIGHT;
}
if (new_size < wmh1) {
new_size = wmh1;
}
if (oldwin_height - new_size - STATUS_HEIGHT < p_wmh)
do_equal = TRUE;
if (oldwin->w_p_wfh) {
set_fraction(oldwin);
did_set_fraction = true;
win_setheight_win(oldwin->w_height + new_size + STATUS_HEIGHT,
oldwin);
oldwin_height = oldwin->w_height;
if (need_status)
oldwin_height -= STATUS_HEIGHT;
}
if (!do_equal && p_ea && size == 0
&& *p_ead != 'h'
&& oldwin->w_frame->fr_parent != NULL) {
frp = oldwin->w_frame->fr_parent->fr_child;
while (frp != NULL) {
if (frp->fr_win != oldwin && frp->fr_win != NULL
&& (frp->fr_win->w_height > new_size
|| frp->fr_win->w_height > oldwin_height - new_size
- STATUS_HEIGHT)) {
do_equal = TRUE;
break;
}
frp = frp->fr_next;
}
}
}
if ((flags & WSP_TOP) == 0
&& ((flags & WSP_BOT)
|| (flags & WSP_BELOW)
|| (!(flags & WSP_ABOVE)
&& (
(flags & WSP_VERT) ? p_spr :
p_sb)))) {
if (new_wp == NULL)
wp = win_alloc(oldwin, FALSE);
else
win_append(oldwin, wp);
} else {
if (new_wp == NULL)
wp = win_alloc(oldwin->w_prev, FALSE);
else
win_append(oldwin->w_prev, wp);
}
if (new_wp == NULL) {
if (wp == NULL)
return FAIL;
new_frame(wp);
win_init(wp, curwin, flags);
} else if (wp->w_floating) {
new_frame(wp);
wp->w_floating = false;
wp->w_float_config = FLOAT_CONFIG_INIT;
}
if (flags & (WSP_TOP | WSP_BOT)) {
if ((topframe->fr_layout == FR_COL && (flags & WSP_VERT) == 0)
|| (topframe->fr_layout == FR_ROW && (flags & WSP_VERT) != 0)) {
curfrp = topframe->fr_child;
if (flags & WSP_BOT)
while (curfrp->fr_next != NULL)
curfrp = curfrp->fr_next;
} else
curfrp = topframe;
before = (flags & WSP_TOP);
} else {
curfrp = oldwin->w_frame;
if (flags & WSP_BELOW)
before = FALSE;
else if (flags & WSP_ABOVE)
before = TRUE;
else if (flags & WSP_VERT)
before = !p_spr;
else
before = !p_sb;
}
if (curfrp->fr_parent == NULL || curfrp->fr_parent->fr_layout != layout) {
frp = xcalloc(1, sizeof(frame_T));
*frp = *curfrp;
curfrp->fr_layout = layout;
frp->fr_parent = curfrp;
frp->fr_next = NULL;
frp->fr_prev = NULL;
curfrp->fr_child = frp;
curfrp->fr_win = NULL;
curfrp = frp;
if (frp->fr_win != NULL) {
oldwin->w_frame = frp;
} else {
FOR_ALL_FRAMES(frp, frp->fr_child) {
frp->fr_parent = curfrp;
}
}
}
if (new_wp == NULL)
frp = wp->w_frame;
else
frp = new_wp->w_frame;
frp->fr_parent = curfrp->fr_parent;
if (before)
frame_insert(curfrp, frp);
else
frame_append(curfrp, frp);
if (!did_set_fraction) {
set_fraction(oldwin);
}
wp->w_fraction = oldwin->w_fraction;
if (flags & WSP_VERT) {
wp->w_p_scr = curwin->w_p_scr;
if (need_status) {
win_new_height(oldwin, oldwin->w_height - 1);
oldwin->w_status_height = need_status;
}
if (flags & (WSP_TOP | WSP_BOT)) {
wp->w_winrow = tabline_height();
win_new_height(wp, curfrp->fr_height - (p_ls > 0));
wp->w_status_height = (p_ls > 0);
} else {
wp->w_winrow = oldwin->w_winrow;
win_new_height(wp, oldwin->w_height);
wp->w_status_height = oldwin->w_status_height;
}
frp->fr_height = curfrp->fr_height;
win_new_width(wp, new_size);
if (before)
wp->w_vsep_width = 1;
else {
wp->w_vsep_width = oldwin->w_vsep_width;
oldwin->w_vsep_width = 1;
}
if (flags & (WSP_TOP | WSP_BOT)) {
if (flags & WSP_BOT)
frame_add_vsep(curfrp);
frame_new_width(curfrp, curfrp->fr_width
- (new_size + ((flags & WSP_TOP) != 0)), flags & WSP_TOP,
FALSE);
} else
win_new_width(oldwin, oldwin->w_width - (new_size + 1));
if (before) { 
wp->w_wincol = oldwin->w_wincol;
oldwin->w_wincol += new_size + 1;
} else 
wp->w_wincol = oldwin->w_wincol + oldwin->w_width + 1;
frame_fix_width(oldwin);
frame_fix_width(wp);
} else {
if (flags & (WSP_TOP | WSP_BOT)) {
wp->w_wincol = 0;
win_new_width(wp, Columns);
wp->w_vsep_width = 0;
} else {
wp->w_wincol = oldwin->w_wincol;
win_new_width(wp, oldwin->w_width);
wp->w_vsep_width = oldwin->w_vsep_width;
}
frp->fr_width = curfrp->fr_width;
win_new_height(wp, new_size);
if (flags & (WSP_TOP | WSP_BOT)) {
int new_fr_height = curfrp->fr_height - new_size;
if (!((flags & WSP_BOT) && p_ls == 0)) {
new_fr_height -= STATUS_HEIGHT;
}
frame_new_height(curfrp, new_fr_height, flags & WSP_TOP, false);
} else {
win_new_height(oldwin, oldwin_height - (new_size + STATUS_HEIGHT));
}
if (before) { 
wp->w_winrow = oldwin->w_winrow;
wp->w_status_height = STATUS_HEIGHT;
oldwin->w_winrow += wp->w_height + STATUS_HEIGHT;
} else { 
wp->w_winrow = oldwin->w_winrow + oldwin->w_height + STATUS_HEIGHT;
wp->w_status_height = oldwin->w_status_height;
if (!(flags & WSP_BOT)) {
oldwin->w_status_height = STATUS_HEIGHT;
}
}
if (flags & WSP_BOT)
frame_add_statusline(curfrp);
frame_fix_height(wp);
frame_fix_height(oldwin);
}
if (flags & (WSP_TOP | WSP_BOT))
(void)win_comp_pos();
redraw_win_later(wp, NOT_VALID);
wp->w_redr_status = TRUE;
redraw_win_later(oldwin, NOT_VALID);
oldwin->w_redr_status = TRUE;
if (need_status) {
msg_row = Rows - 1;
msg_col = sc_col;
msg_clr_eos_force(); 
comp_col();
msg_row = Rows - 1;
msg_col = 0; 
}
if (do_equal || dir != 0)
win_equal(wp, true,
(flags & WSP_VERT) ? (dir == 'v' ? 'b' : 'h')
: dir == 'h' ? 'b' :
'v');
if (flags & WSP_VERT) {
i = p_wiw;
if (size != 0)
p_wiw = size;
} else {
i = p_wh;
if (size != 0)
p_wh = size;
}
wp->w_changelistidx = oldwin->w_changelistidx;
win_enter_ext(wp, false, false, true, true, true);
if (flags & WSP_VERT) {
p_wiw = i;
} else {
p_wh = i;
}
oldwin->w_pos_changed = true;
return OK;
}
static void win_init(win_T *newp, win_T *oldp, int flags)
{
int i;
newp->w_buffer = oldp->w_buffer;
newp->w_s = &(oldp->w_buffer->b_s);
oldp->w_buffer->b_nwindows++;
newp->w_cursor = oldp->w_cursor;
newp->w_valid = 0;
newp->w_curswant = oldp->w_curswant;
newp->w_set_curswant = oldp->w_set_curswant;
newp->w_topline = oldp->w_topline;
newp->w_topfill = oldp->w_topfill;
newp->w_leftcol = oldp->w_leftcol;
newp->w_pcmark = oldp->w_pcmark;
newp->w_prev_pcmark = oldp->w_prev_pcmark;
newp->w_alt_fnum = oldp->w_alt_fnum;
newp->w_wrow = oldp->w_wrow;
newp->w_fraction = oldp->w_fraction;
newp->w_prev_fraction_row = oldp->w_prev_fraction_row;
copy_jumplist(oldp, newp);
if (flags & WSP_NEWLOC) {
newp->w_llist = NULL;
newp->w_llist_ref = NULL;
} else {
copy_loclist_stack(oldp, newp);
}
newp->w_localdir = (oldp->w_localdir == NULL)
? NULL : vim_strsave(oldp->w_localdir);
for (i = 0; i < oldp->w_tagstacklen; i++) {
taggy_T *tag = &newp->w_tagstack[i];
*tag = oldp->w_tagstack[i];
if (tag->tagname != NULL) {
tag->tagname = vim_strsave(tag->tagname);
}
if (tag->user_data != NULL) {
tag->user_data = vim_strsave(tag->user_data);
}
}
newp->w_tagstackidx = oldp->w_tagstackidx;
newp->w_tagstacklen = oldp->w_tagstacklen;
copyFoldingState(oldp, newp);
win_init_some(newp, oldp);
didset_window_options(newp);
}
static void win_init_some(win_T *newp, win_T *oldp)
{
newp->w_alist = oldp->w_alist;
++newp->w_alist->al_refcount;
newp->w_arg_idx = oldp->w_arg_idx;
win_copy_options(oldp, newp);
}
bool win_valid(const win_T *win) FUNC_ATTR_PURE FUNC_ATTR_WARN_UNUSED_RESULT
{
if (win == NULL) {
return false;
}
FOR_ALL_WINDOWS_IN_TAB(wp, curtab) {
if (wp == win) {
return true;
}
}
return false;
}
bool win_valid_any_tab(win_T *win) FUNC_ATTR_PURE FUNC_ATTR_WARN_UNUSED_RESULT
{
if (win == NULL) {
return false;
}
FOR_ALL_TAB_WINDOWS(tp, wp) {
if (wp == win) {
return true;
}
}
return false;
}
int win_count(void)
{
int count = 0;
FOR_ALL_WINDOWS_IN_TAB(wp, curtab) {
++count;
}
return count;
}
int 
make_windows (
int count,
int vertical 
)
{
int maxcount;
int todo;
if (vertical) {
maxcount = (curwin->w_width + curwin->w_vsep_width
- (p_wiw - p_wmw)) / (p_wmw + 1);
} else {
maxcount = (curwin->w_height + curwin->w_status_height
- (p_wh - p_wmh)) / (p_wmh + STATUS_HEIGHT);
}
if (maxcount < 2)
maxcount = 2;
if (count > maxcount)
count = maxcount;
if (count > 1)
last_status(TRUE);
block_autocmds();
for (todo = count - 1; todo > 0; --todo)
if (vertical) {
if (win_split(curwin->w_width - (curwin->w_width - todo)
/ (todo + 1) - 1, WSP_VERT | WSP_ABOVE) == FAIL)
break;
} else {
if (win_split(curwin->w_height - (curwin->w_height - todo
* STATUS_HEIGHT) / (todo + 1)
- STATUS_HEIGHT, WSP_ABOVE) == FAIL)
break;
}
unblock_autocmds();
return count - todo;
}
static void win_exchange(long Prenum)
{
frame_T *frp;
frame_T *frp2;
win_T *wp;
win_T *wp2;
int temp;
if (curwin->w_floating) {
EMSG(e_floatexchange);
return;
}
if (firstwin == curwin && lastwin_nofloating() == curwin) {
beep_flush();
return;
}
if (Prenum) {
frp = curwin->w_frame->fr_parent->fr_child;
while (frp != NULL && --Prenum > 0)
frp = frp->fr_next;
} else if (curwin->w_frame->fr_next != NULL) 
frp = curwin->w_frame->fr_next;
else 
frp = curwin->w_frame->fr_prev;
if (frp == NULL || frp->fr_win == NULL || frp->fr_win == curwin)
return;
wp = frp->fr_win;
wp2 = curwin->w_prev;
frp2 = curwin->w_frame->fr_prev;
if (wp->w_prev != curwin) {
win_remove(curwin, NULL);
frame_remove(curwin->w_frame);
win_append(wp->w_prev, curwin);
frame_insert(frp, curwin->w_frame);
}
if (wp != wp2) {
win_remove(wp, NULL);
frame_remove(wp->w_frame);
win_append(wp2, wp);
if (frp2 == NULL)
frame_insert(wp->w_frame->fr_parent->fr_child, wp->w_frame);
else
frame_append(frp2, wp->w_frame);
}
temp = curwin->w_status_height;
curwin->w_status_height = wp->w_status_height;
wp->w_status_height = temp;
temp = curwin->w_vsep_width;
curwin->w_vsep_width = wp->w_vsep_width;
wp->w_vsep_width = temp;
if (curwin->w_frame->fr_parent != wp->w_frame->fr_parent) {
temp = curwin->w_height;
curwin->w_height = wp->w_height;
wp->w_height = temp;
temp = curwin->w_width;
curwin->w_width = wp->w_width;
wp->w_width = temp;
} else {
frame_fix_height(curwin);
frame_fix_height(wp);
frame_fix_width(curwin);
frame_fix_width(wp);
}
(void)win_comp_pos(); 
win_enter(wp, true);
redraw_later(NOT_VALID);
redraw_win_later(wp, NOT_VALID);
}
static void win_rotate(bool upwards, int count)
{
win_T *wp1;
win_T *wp2;
frame_T *frp;
int n;
if (curwin->w_floating) {
EMSG(e_floatexchange);
return;
}
if (count <= 0 || (firstwin == curwin && lastwin_nofloating() == curwin)) {
beep_flush();
return;
}
FOR_ALL_FRAMES(frp, curwin->w_frame->fr_parent->fr_child) {
if (frp->fr_win == NULL) {
EMSG(_("E443: Cannot rotate when another window is split"));
return;
}
}
while (count--) {
if (upwards) { 
frp = curwin->w_frame->fr_parent->fr_child;
assert(frp != NULL);
wp1 = frp->fr_win;
win_remove(wp1, NULL);
frame_remove(frp);
assert(frp->fr_parent->fr_child);
for (; frp->fr_next != NULL; frp = frp->fr_next)
;
win_append(frp->fr_win, wp1);
frame_append(frp, wp1->w_frame);
wp2 = frp->fr_win; 
} else { 
for (frp = curwin->w_frame; frp->fr_next != NULL;
frp = frp->fr_next)
;
wp1 = frp->fr_win;
wp2 = wp1->w_prev; 
win_remove(wp1, NULL);
frame_remove(frp);
assert(frp->fr_parent->fr_child);
win_append(frp->fr_parent->fr_child->fr_win->w_prev, wp1);
frame_insert(frp->fr_parent->fr_child, frp);
}
n = wp2->w_status_height;
wp2->w_status_height = wp1->w_status_height;
wp1->w_status_height = n;
frame_fix_height(wp1);
frame_fix_height(wp2);
n = wp2->w_vsep_width;
wp2->w_vsep_width = wp1->w_vsep_width;
wp1->w_vsep_width = n;
frame_fix_width(wp1);
frame_fix_width(wp2);
(void)win_comp_pos();
}
wp1->w_pos_changed = true;
wp2->w_pos_changed = true;
redraw_all_later(NOT_VALID);
}
static void win_totop(int size, int flags)
{
int dir = 0;
int height = curwin->w_height;
if (firstwin == curwin && lastwin_nofloating() == curwin) {
beep_flush();
return;
}
if (curwin->w_floating) {
ui_comp_remove_grid(&curwin->w_grid);
if (ui_has(kUIMultigrid)) {
curwin->w_pos_changed = true;
} else {
ui_call_win_hide(curwin->w_grid.handle);
win_free_grid(curwin, false);
}
} else {
(void)winframe_remove(curwin, &dir, NULL);
}
win_remove(curwin, NULL);
last_status(FALSE); 
(void)win_comp_pos(); 
(void)win_split_ins(size, flags, curwin, dir);
if (!(flags & WSP_VERT)) {
win_setheight(height);
if (p_ea)
win_equal(curwin, true, 'v');
}
}
void win_move_after(win_T *win1, win_T *win2)
{
int height;
if (win1 == win2)
return;
if (win2->w_next != win1) {
if (win1 == lastwin) {
height = win1->w_prev->w_status_height;
win1->w_prev->w_status_height = win1->w_status_height;
win1->w_status_height = height;
if (win1->w_prev->w_vsep_width == 1) {
win1->w_prev->w_vsep_width = 0;
win1->w_prev->w_frame->fr_width -= 1;
win1->w_vsep_width = 1;
win1->w_frame->fr_width += 1;
}
} else if (win2 == lastwin) {
height = win1->w_status_height;
win1->w_status_height = win2->w_status_height;
win2->w_status_height = height;
if (win1->w_vsep_width == 1) {
win2->w_vsep_width = 1;
win2->w_frame->fr_width += 1;
win1->w_vsep_width = 0;
win1->w_frame->fr_width -= 1;
}
}
win_remove(win1, NULL);
frame_remove(win1->w_frame);
win_append(win2, win1);
frame_append(win2->w_frame, win1->w_frame);
(void)win_comp_pos(); 
redraw_later(NOT_VALID);
}
win_enter(win1, false);
win1->w_pos_changed = true;
win2->w_pos_changed = true;
}
void win_equal(
win_T *next_curwin, 
bool current, 
int dir 
)
{
if (dir == 0)
dir = *p_ead;
win_equal_rec(next_curwin == NULL ? curwin : next_curwin, current,
topframe, dir, 0, tabline_height(),
Columns, topframe->fr_height);
}
static void win_equal_rec(
win_T *next_curwin, 
bool current, 
frame_T *topfr, 
int dir, 
int col, 
int row, 
int width, 
int height 
)
{
int n, m;
int extra_sep = 0;
int wincount, totwincount = 0;
frame_T *fr;
int next_curwin_size = 0;
int room = 0;
int new_size;
int has_next_curwin = 0;
int hnc;
if (topfr->fr_layout == FR_LEAF) {
if (topfr->fr_height != height || topfr->fr_win->w_winrow != row
|| topfr->fr_width != width || topfr->fr_win->w_wincol != col
) {
topfr->fr_win->w_winrow = row;
frame_new_height(topfr, height, false, false);
topfr->fr_win->w_wincol = col;
frame_new_width(topfr, width, false, false);
redraw_all_later(NOT_VALID);
}
} else if (topfr->fr_layout == FR_ROW) {
topfr->fr_width = width;
topfr->fr_height = height;
if (dir != 'v') { 
n = frame_minwidth(topfr, NOWIN);
if (col + width == Columns)
extra_sep = 1;
else
extra_sep = 0;
totwincount = (n + extra_sep) / (p_wmw + 1);
has_next_curwin = frame_has_win(topfr, next_curwin);
m = frame_minwidth(topfr, next_curwin);
room = width - m;
if (room < 0) {
next_curwin_size = p_wiw + room;
room = 0;
} else {
next_curwin_size = -1;
FOR_ALL_FRAMES(fr, topfr->fr_child) {
if (!frame_fixed_width(fr)) {
continue;
}
n = frame_minwidth(fr, NOWIN);
new_size = fr->fr_width;
if (frame_has_win(fr, next_curwin)) {
room += p_wiw - p_wmw;
next_curwin_size = 0;
if (new_size < p_wiw)
new_size = p_wiw;
} else
totwincount -= (n + (fr->fr_next == NULL
? extra_sep : 0)) / (p_wmw + 1);
room -= new_size - n;
if (room < 0) {
new_size += room;
room = 0;
}
fr->fr_newwidth = new_size;
}
if (next_curwin_size == -1) {
if (!has_next_curwin)
next_curwin_size = 0;
else if (totwincount > 1
&& (room + (totwincount - 2))
/ (totwincount - 1) > p_wiw) {
next_curwin_size = (room + p_wiw
+ (totwincount - 1) * p_wmw
+ (totwincount - 1)) / totwincount;
room -= next_curwin_size - p_wiw;
} else
next_curwin_size = p_wiw;
}
}
if (has_next_curwin)
--totwincount; 
}
FOR_ALL_FRAMES(fr, topfr->fr_child) {
wincount = 1;
if (fr->fr_next == NULL)
new_size = width;
else if (dir == 'v')
new_size = fr->fr_width;
else if (frame_fixed_width(fr)) {
new_size = fr->fr_newwidth;
wincount = 0; 
} else {
n = frame_minwidth(fr, NOWIN);
wincount = (n + (fr->fr_next == NULL ? extra_sep : 0))
/ (p_wmw + 1);
m = frame_minwidth(fr, next_curwin);
if (has_next_curwin)
hnc = frame_has_win(fr, next_curwin);
else
hnc = FALSE;
if (hnc) 
--wincount;
if (totwincount == 0)
new_size = room;
else
new_size = (wincount * room + (totwincount / 2)) / totwincount;
if (hnc) { 
next_curwin_size -= p_wiw - (m - n);
new_size += next_curwin_size;
room -= new_size - next_curwin_size;
} else
room -= new_size;
new_size += n;
}
if (!current || dir != 'v' || topfr->fr_parent != NULL
|| (new_size != fr->fr_width)
|| frame_has_win(fr, next_curwin))
win_equal_rec(next_curwin, current, fr, dir, col, row,
new_size, height);
col += new_size;
width -= new_size;
totwincount -= wincount;
}
} else { 
topfr->fr_width = width;
topfr->fr_height = height;
if (dir != 'h') { 
n = frame_minheight(topfr, NOWIN);
if (row + height == cmdline_row && p_ls == 0)
extra_sep = 1;
else
extra_sep = 0;
totwincount = (n + extra_sep) / (p_wmh + 1);
has_next_curwin = frame_has_win(topfr, next_curwin);
m = frame_minheight(topfr, next_curwin);
room = height - m;
if (room < 0) {
next_curwin_size = p_wh + room;
room = 0;
} else {
next_curwin_size = -1;
FOR_ALL_FRAMES(fr, topfr->fr_child) {
if (!frame_fixed_height(fr)) {
continue;
}
n = frame_minheight(fr, NOWIN);
new_size = fr->fr_height;
if (frame_has_win(fr, next_curwin)) {
room += p_wh - p_wmh;
next_curwin_size = 0;
if (new_size < p_wh)
new_size = p_wh;
} else
totwincount -= (n + (fr->fr_next == NULL
? extra_sep : 0)) / (p_wmh + 1);
room -= new_size - n;
if (room < 0) {
new_size += room;
room = 0;
}
fr->fr_newheight = new_size;
}
if (next_curwin_size == -1) {
if (!has_next_curwin)
next_curwin_size = 0;
else if (totwincount > 1
&& (room + (totwincount - 2))
/ (totwincount - 1) > p_wh) {
next_curwin_size = (room + p_wh
+ (totwincount - 1) * p_wmh
+ (totwincount - 1)) / totwincount;
room -= next_curwin_size - p_wh;
} else
next_curwin_size = p_wh;
}
}
if (has_next_curwin)
--totwincount; 
}
FOR_ALL_FRAMES(fr, topfr->fr_child) {
wincount = 1;
if (fr->fr_next == NULL)
new_size = height;
else if (dir == 'h')
new_size = fr->fr_height;
else if (frame_fixed_height(fr)) {
new_size = fr->fr_newheight;
wincount = 0; 
} else {
n = frame_minheight(fr, NOWIN);
wincount = (n + (fr->fr_next == NULL ? extra_sep : 0))
/ (p_wmh + 1);
m = frame_minheight(fr, next_curwin);
if (has_next_curwin)
hnc = frame_has_win(fr, next_curwin);
else
hnc = FALSE;
if (hnc) 
--wincount;
if (totwincount == 0)
new_size = room;
else
new_size = (wincount * room + (totwincount / 2)) / totwincount;
if (hnc) { 
next_curwin_size -= p_wh - (m - n);
new_size += next_curwin_size;
room -= new_size - next_curwin_size;
} else
room -= new_size;
new_size += n;
}
if (!current || dir != 'h' || topfr->fr_parent != NULL
|| (new_size != fr->fr_height)
|| frame_has_win(fr, next_curwin))
win_equal_rec(next_curwin, current, fr, dir, col, row,
width, new_size);
row += new_size;
height -= new_size;
totwincount -= wincount;
}
}
}
void close_windows(buf_T *buf, int keep_curwin)
{
tabpage_T *tp, *nexttp;
int h = tabline_height();
++RedrawingDisabled;
for (win_T *wp = firstwin; wp != NULL && !ONE_WINDOW; ) {
if (wp->w_buffer == buf && (!keep_curwin || wp != curwin)
&& !(wp->w_closing || wp->w_buffer->b_locked > 0)) {
if (win_close(wp, false) == FAIL) {
break;
}
wp = firstwin;
} else
wp = wp->w_next;
}
for (tp = first_tabpage; tp != NULL; tp = nexttp) {
nexttp = tp->tp_next;
if (tp != curtab) {
FOR_ALL_WINDOWS_IN_TAB(wp, tp) {
if (wp->w_buffer == buf
&& !(wp->w_closing || wp->w_buffer->b_locked > 0)) {
win_close_othertab(wp, false, tp);
nexttp = first_tabpage;
break;
}
}
}
}
--RedrawingDisabled;
redraw_tabline = true;
if (h != tabline_height()) {
shell_new_rows();
}
}
static bool last_window(void) FUNC_ATTR_PURE FUNC_ATTR_WARN_UNUSED_RESULT
{
return one_window() && first_tabpage->tp_next == NULL;
}
bool one_window(void) FUNC_ATTR_PURE FUNC_ATTR_WARN_UNUSED_RESULT
{
bool seen_one = false;
FOR_ALL_WINDOWS_IN_TAB(wp, curtab) {
if (wp != aucmd_win && (!wp->w_floating || wp == curwin)) {
if (seen_one) {
return false;
}
seen_one = true;
}
}
return true;
}
bool one_nonfloat(void) FUNC_ATTR_PURE FUNC_ATTR_WARN_UNUSED_RESULT
{
return firstwin->w_next == NULL || firstwin->w_next->w_floating;
}
bool last_nonfloat(win_T *wp) FUNC_ATTR_PURE FUNC_ATTR_WARN_UNUSED_RESULT
{
return firstwin == wp && !(wp->w_next && !wp->w_floating);
}
static bool close_last_window_tabpage(win_T *win, bool free_buf,
tabpage_T *prev_curtab)
FUNC_ATTR_NONNULL_ARG(1)
{
if (!ONE_WINDOW) {
return false;
}
buf_T *old_curbuf = curbuf;
Terminal *term = win->w_buffer ? win->w_buffer->terminal : NULL;
if (term) {
free_buf = false;
}
goto_tabpage_tp(alt_tabpage(), FALSE, TRUE);
redraw_tabline = TRUE;
char_u prev_idx[NUMBUFLEN];
sprintf((char *)prev_idx, "%i", tabpage_index(prev_curtab));
if (valid_tabpage(prev_curtab) && prev_curtab->tp_firstwin == win) {
int h = tabline_height();
win_close_othertab(win, free_buf, prev_curtab);
if (h != tabline_height())
shell_new_rows();
}
apply_autocmds(EVENT_WINENTER, NULL, NULL, false, curbuf);
apply_autocmds(EVENT_TABENTER, NULL, NULL, false, curbuf);
if (old_curbuf != curbuf) {
apply_autocmds(EVENT_BUFENTER, NULL, NULL, false, curbuf);
}
return true;
}
int win_close(win_T *win, bool free_buf)
{
win_T *wp;
int other_buffer = FALSE;
int close_curwin = FALSE;
int dir;
bool help_window = false;
tabpage_T *prev_curtab = curtab;
frame_T *win_frame = win->w_floating ? NULL : win->w_frame->fr_parent;
const bool had_diffmode = win->w_p_diff;
if (last_window() && !win->w_floating) {
EMSG(_("E444: Cannot close last window"));
return FAIL;
}
if (win->w_closing
|| (win->w_buffer != NULL && win->w_buffer->b_locked > 0)) {
return FAIL; 
}
if (win == aucmd_win) {
EMSG(_("E813: Cannot close autocmd window"));
return FAIL;
}
if ((firstwin == aucmd_win || lastwin == aucmd_win) && one_window()) {
EMSG(_("E814: Cannot close window, only autocmd window would remain"));
return FAIL;
}
if ((firstwin == win && lastwin_nofloating() == win)
&& lastwin->w_floating) {
EMSG(e_floatonly);
return FAIL;
}
if (close_last_window_tabpage(win, free_buf, prev_curtab))
return FAIL;
if (bt_help(win->w_buffer)) {
help_window = true;
} else {
clear_snapshot(curtab, SNAP_HELP_IDX);
}
if (win == curwin) {
if (!win->w_floating) {
wp = frame2win(win_altframe(win, NULL));
} else {
if (win_valid(prevwin) && prevwin != win) {
wp = prevwin;
} else {
wp = firstwin;
}
}
if (wp->w_buffer != curbuf) {
other_buffer = TRUE;
win->w_closing = true;
apply_autocmds(EVENT_BUFLEAVE, NULL, NULL, FALSE, curbuf);
if (!win_valid(win))
return FAIL;
win->w_closing = false;
if (last_window())
return FAIL;
}
win->w_closing = true;
apply_autocmds(EVENT_WINLEAVE, NULL, NULL, false, curbuf);
if (!win_valid(win)) {
return FAIL;
}
win->w_closing = false;
if (last_window())
return FAIL;
if (aborting())
return FAIL;
}
bool was_floating = win->w_floating;
if (ui_has(kUIMultigrid)) {
ui_call_win_close(win->w_grid.handle);
}
if (win->w_floating) {
ui_comp_remove_grid(&win->w_grid);
if (win->w_float_config.external) {
for (tabpage_T *tp = first_tabpage; tp != NULL; tp = tp->tp_next) {
if (tp == curtab) {
continue;
}
if (tp->tp_curwin == win) {
tp->tp_curwin = tp->tp_firstwin;
}
}
}
}
do_autocmd_winclosed(win);
if (!win_valid_any_tab(win)) {
return OK;
}
if (win->w_buffer != NULL)
reset_synblock(win);
if (win->w_buffer != NULL) {
bufref_T bufref;
set_bufref(&bufref, curbuf);
win->w_closing = true;
close_buffer(win, win->w_buffer, free_buf ? DOBUF_UNLOAD : 0, true);
if (win_valid_any_tab(win)) {
win->w_closing = false;
}
if (!bufref_valid(&bufref)) {
curbuf = firstbuf;
}
}
if (only_one_window() && win_valid(win) && win->w_buffer == NULL
&& (last_window() || curtab != prev_curtab
|| close_last_window_tabpage(win, free_buf, prev_curtab))
&& !win->w_floating) {
if (curwin->w_buffer == NULL) {
curwin->w_buffer = curbuf;
}
getout(0);
}
if (curtab != prev_curtab && win_valid_any_tab(win)
&& win->w_buffer == NULL) {
win_close_othertab(win, false, prev_curtab);
return FAIL;
}
if (!win_valid(win) || (!win->w_floating && last_window())
|| close_last_window_tabpage(win, free_buf, prev_curtab)) {
return FAIL;
}
win->w_closing = true;
wp = win_free_mem(win, &dir, NULL);
if (help_window) {
win_T *tmpwp = get_snapshot_focus(SNAP_HELP_IDX);
if (tmpwp != NULL) {
wp = tmpwp;
}
}
if (win == curwin) {
curwin = wp;
if (wp->w_p_pvw || bt_quickfix(wp->w_buffer)) {
for (;; ) {
if (wp->w_next == NULL)
wp = firstwin;
else
wp = wp->w_next;
if (wp == curwin)
break;
if (!wp->w_p_pvw && !bt_quickfix(wp->w_buffer)) {
curwin = wp;
break;
}
}
}
curbuf = curwin->w_buffer;
close_curwin = TRUE;
check_cursor();
}
if (!was_floating) {
if (!curwin->w_floating && p_ea && (*p_ead == 'b' || *p_ead == dir)) {
win_equal(curwin, curwin->w_frame->fr_parent == win_frame, dir);
} else {
win_comp_pos();
}
}
if (close_curwin) {
win_enter_ext(wp, false, true, false, true, true);
if (other_buffer) {
apply_autocmds(EVENT_BUFENTER, NULL, NULL, false, curbuf);
}
}
last_status(FALSE);
if (help_window)
restore_snapshot(SNAP_HELP_IDX, close_curwin);
if (diffopt_closeoff() && had_diffmode && curtab == prev_curtab) {
int diffcount = 0;
FOR_ALL_WINDOWS_IN_TAB(dwin, curtab) {
if (dwin->w_p_diff) {
diffcount++;
}
}
if (diffcount == 1) {
do_cmdline_cmd("diffoff!");
}
}
curwin->w_pos_changed = true;
redraw_all_later(NOT_VALID);
return OK;
}
static void do_autocmd_winclosed(win_T *win)
FUNC_ATTR_NONNULL_ALL
{
static bool recursive = false;
if (recursive || !has_event(EVENT_WINCLOSED)) {
return;
}
recursive = true;
char_u winid[NUMBUFLEN];
vim_snprintf((char *)winid, sizeof(winid), "%i", win->handle);
apply_autocmds(EVENT_WINCLOSED, winid, winid, false, win->w_buffer);
recursive = false;
}
void win_close_othertab(win_T *win, int free_buf, tabpage_T *tp)
{
int dir;
tabpage_T *ptp = NULL;
int free_tp = FALSE;
if (win->w_closing
|| (win->w_buffer != NULL && win->w_buffer->b_locked > 0)) {
return; 
}
do_autocmd_winclosed(win);
if (!win_valid_any_tab(win)) {
return;
}
if (win->w_buffer != NULL) {
close_buffer(win, win->w_buffer, free_buf ? DOBUF_UNLOAD : 0, false);
}
for (ptp = first_tabpage; ptp != NULL && ptp != tp; ptp = ptp->tp_next)
;
if (ptp == NULL || tp == curtab)
return;
{
bool found_window = false;
FOR_ALL_WINDOWS_IN_TAB(wp, tp) {
if (wp == win) {
found_window = true;
break;
}
}
if (!found_window) {
return;
}
}
if (tp->tp_firstwin == tp->tp_lastwin) {
char_u prev_idx[NUMBUFLEN];
if (has_event(EVENT_TABCLOSED)) {
vim_snprintf((char *)prev_idx, NUMBUFLEN, "%i", tabpage_index(tp));
}
if (tp == first_tabpage) {
first_tabpage = tp->tp_next;
} else {
for (ptp = first_tabpage; ptp != NULL && ptp->tp_next != tp;
ptp = ptp->tp_next) {
}
if (ptp == NULL) {
internal_error("win_close_othertab()");
return;
}
ptp->tp_next = tp->tp_next;
}
free_tp = true;
if (has_event(EVENT_TABCLOSED)) {
apply_autocmds(EVENT_TABCLOSED, prev_idx, prev_idx, false, win->w_buffer);
}
}
win_free_mem(win, &dir, tp);
if (free_tp)
free_tabpage(tp);
}
static win_T *win_free_mem(
win_T *win,
int *dirp, 
tabpage_T *tp 
)
{
frame_T *frp;
win_T *wp;
if (!win->w_floating) {
frp = win->w_frame;
wp = winframe_remove(win, dirp, tp);
xfree(frp);
} else {
*dirp = 'h'; 
if (win_valid(prevwin) && prevwin != win) {
wp = prevwin;
} else {
wp = firstwin;
}
}
win_free(win, tp);
if (tp != NULL && win == tp->tp_curwin) {
tp->tp_curwin = wp;
}
return wp;
}
#if defined(EXITFREE)
void win_free_all(void)
{
int dummy;
while (first_tabpage->tp_next != NULL)
tabpage_close(TRUE);
while (lastwin != NULL && lastwin->w_floating) {
win_T *wp = lastwin;
win_remove(lastwin, NULL);
(void)win_free_mem(wp, &dummy, NULL);
if (wp == aucmd_win) {
aucmd_win = NULL;
}
}
if (aucmd_win != NULL) {
(void)win_free_mem(aucmd_win, &dummy, NULL);
aucmd_win = NULL;
}
while (firstwin != NULL)
(void)win_free_mem(firstwin, &dummy, NULL);
curwin = NULL;
}
#endif
win_T *
winframe_remove (
win_T *win,
int *dirp, 
tabpage_T *tp 
)
{
frame_T *frp, *frp2, *frp3;
frame_T *frp_close = win->w_frame;
win_T *wp;
if (tp == NULL ? ONE_WINDOW : tp->tp_firstwin == tp->tp_lastwin)
return NULL;
frp2 = win_altframe(win, tp);
wp = frame2win(frp2);
frame_remove(frp_close);
if (frp_close->fr_parent->fr_layout == FR_COL) {
if (frp2->fr_win != NULL && frp2->fr_win->w_p_wfh) {
frp = frp_close->fr_prev;
frp3 = frp_close->fr_next;
while (frp != NULL || frp3 != NULL) {
if (frp != NULL) {
if (!frame_fixed_height(frp)) {
frp2 = frp;
wp = frame2win(frp2);
break;
}
frp = frp->fr_prev;
}
if (frp3 != NULL) {
if (frp3->fr_win != NULL && !frp3->fr_win->w_p_wfh) {
frp2 = frp3;
wp = frp3->fr_win;
break;
}
frp3 = frp3->fr_next;
}
}
}
frame_new_height(frp2, frp2->fr_height + frp_close->fr_height,
frp2 == frp_close->fr_next ? TRUE : FALSE, FALSE);
*dirp = 'v';
} else {
if (frp2->fr_win != NULL && frp2->fr_win->w_p_wfw) {
frp = frp_close->fr_prev;
frp3 = frp_close->fr_next;
while (frp != NULL || frp3 != NULL) {
if (frp != NULL) {
if (!frame_fixed_width(frp)) {
frp2 = frp;
wp = frame2win(frp2);
break;
}
frp = frp->fr_prev;
}
if (frp3 != NULL) {
if (frp3->fr_win != NULL && !frp3->fr_win->w_p_wfw) {
frp2 = frp3;
wp = frp3->fr_win;
break;
}
frp3 = frp3->fr_next;
}
}
}
frame_new_width(frp2, frp2->fr_width + frp_close->fr_width,
frp2 == frp_close->fr_next ? TRUE : FALSE, FALSE);
*dirp = 'h';
}
if (frp2 == frp_close->fr_next) {
int row = win->w_winrow;
int col = win->w_wincol;
frame_comp_pos(frp2, &row, &col);
}
if (frp2->fr_next == NULL && frp2->fr_prev == NULL) {
frp2->fr_parent->fr_layout = frp2->fr_layout;
frp2->fr_parent->fr_child = frp2->fr_child;
FOR_ALL_FRAMES(frp, frp2->fr_child) {
frp->fr_parent = frp2->fr_parent;
}
frp2->fr_parent->fr_win = frp2->fr_win;
if (frp2->fr_win != NULL)
frp2->fr_win->w_frame = frp2->fr_parent;
frp = frp2->fr_parent;
if (topframe->fr_child == frp2) {
topframe->fr_child = frp;
}
xfree(frp2);
frp2 = frp->fr_parent;
if (frp2 != NULL && frp2->fr_layout == frp->fr_layout) {
if (frp2->fr_child == frp)
frp2->fr_child = frp->fr_child;
assert(frp->fr_child);
frp->fr_child->fr_prev = frp->fr_prev;
if (frp->fr_prev != NULL)
frp->fr_prev->fr_next = frp->fr_child;
for (frp3 = frp->fr_child;; frp3 = frp3->fr_next) {
frp3->fr_parent = frp2;
if (frp3->fr_next == NULL) {
frp3->fr_next = frp->fr_next;
if (frp->fr_next != NULL)
frp->fr_next->fr_prev = frp3;
break;
}
}
if (topframe->fr_child == frp) {
topframe->fr_child = frp2;
}
xfree(frp);
}
}
return wp;
}
static frame_T *
win_altframe (
win_T *win,
tabpage_T *tp 
)
{
frame_T *frp;
if (tp == NULL ? ONE_WINDOW : tp->tp_firstwin == tp->tp_lastwin) {
return alt_tabpage()->tp_curwin->w_frame;
}
frp = win->w_frame;
if (frp->fr_prev == NULL) {
return frp->fr_next;
}
if (frp->fr_next == NULL) {
return frp->fr_prev;
}
frame_T *target_fr = frp->fr_next;
frame_T *other_fr = frp->fr_prev;
if (p_spr || p_sb) {
target_fr = frp->fr_prev;
other_fr = frp->fr_next;
}
if (frp->fr_parent != NULL && frp->fr_parent->fr_layout == FR_ROW) {
if (frame_fixed_width(target_fr) && !frame_fixed_width(other_fr)) {
target_fr = other_fr;
}
} else {
if (frame_fixed_height(target_fr) && !frame_fixed_height(other_fr)) {
target_fr = other_fr;
}
}
return target_fr;
}
static tabpage_T *alt_tabpage(void)
{
tabpage_T *tp;
if (curtab->tp_next != NULL)
return curtab->tp_next;
for (tp = first_tabpage; tp->tp_next != curtab; tp = tp->tp_next)
;
return tp;
}
static win_T *frame2win(frame_T *frp)
{
while (frp->fr_win == NULL)
frp = frp->fr_child;
return frp->fr_win;
}
static bool frame_has_win(const frame_T *frp, const win_T *wp)
FUNC_ATTR_PURE FUNC_ATTR_WARN_UNUSED_RESULT FUNC_ATTR_NONNULL_ARG(1)
{
if (frp->fr_layout == FR_LEAF) {
return frp->fr_win == wp;
}
const frame_T *p;
FOR_ALL_FRAMES(p, frp->fr_child) {
if (frame_has_win(p, wp)) {
return true;
}
}
return false;
}
static void 
frame_new_height (
frame_T *topfrp,
int height,
int topfirst, 
int wfh 
)
{
frame_T *frp;
int extra_lines;
int h;
if (topfrp->fr_win != NULL) {
win_new_height(topfrp->fr_win,
height - topfrp->fr_win->w_status_height);
} else if (topfrp->fr_layout == FR_ROW) {
do {
FOR_ALL_FRAMES(frp, topfrp->fr_child) {
frame_new_height(frp, height, topfirst, wfh);
if (frp->fr_height > height) {
height = frp->fr_height;
break;
}
}
} while (frp != NULL);
} else { 
frp = topfrp->fr_child;
if (wfh)
while (frame_fixed_height(frp)) {
frp = frp->fr_next;
if (frp == NULL)
return; 
}
if (!topfirst) {
while (frp->fr_next != NULL)
frp = frp->fr_next;
if (wfh)
while (frame_fixed_height(frp))
frp = frp->fr_prev;
}
extra_lines = height - topfrp->fr_height;
if (extra_lines < 0) {
while (frp != NULL) {
h = frame_minheight(frp, NULL);
if (frp->fr_height + extra_lines < h) {
extra_lines += frp->fr_height - h;
frame_new_height(frp, h, topfirst, wfh);
} else {
frame_new_height(frp, frp->fr_height + extra_lines,
topfirst, wfh);
break;
}
if (topfirst) {
do
frp = frp->fr_next;
while (wfh && frp != NULL && frame_fixed_height(frp));
} else {
do
frp = frp->fr_prev;
while (wfh && frp != NULL && frame_fixed_height(frp));
}
if (frp == NULL)
height -= extra_lines;
}
} else if (extra_lines > 0) {
frame_new_height(frp, frp->fr_height + extra_lines, topfirst, wfh);
}
}
topfrp->fr_height = height;
}
static bool frame_fixed_height(frame_T *frp)
FUNC_ATTR_PURE FUNC_ATTR_WARN_UNUSED_RESULT FUNC_ATTR_NONNULL_ALL
{
if (frp->fr_win != NULL) {
return frp->fr_win->w_p_wfh;
}
if (frp->fr_layout == FR_ROW) {
FOR_ALL_FRAMES(frp, frp->fr_child) {
if (frame_fixed_height(frp)) {
return true;
}
}
return false;
}
FOR_ALL_FRAMES(frp, frp->fr_child) {
if (!frame_fixed_height(frp)) {
return false;
}
}
return true;
}
static bool frame_fixed_width(frame_T *frp)
FUNC_ATTR_PURE FUNC_ATTR_WARN_UNUSED_RESULT FUNC_ATTR_NONNULL_ALL
{
if (frp->fr_win != NULL) {
return frp->fr_win->w_p_wfw;
}
if (frp->fr_layout == FR_COL) {
FOR_ALL_FRAMES(frp, frp->fr_child) {
if (frame_fixed_width(frp)) {
return true;
}
}
return false;
}
FOR_ALL_FRAMES(frp, frp->fr_child) {
if (!frame_fixed_width(frp)) {
return false;
}
}
return true;
}
static void frame_add_statusline(frame_T *frp)
{
win_T *wp;
if (frp->fr_layout == FR_LEAF) {
wp = frp->fr_win;
if (wp->w_status_height == 0) {
if (wp->w_height > 0) 
--wp->w_height;
wp->w_status_height = STATUS_HEIGHT;
}
} else if (frp->fr_layout == FR_ROW) {
FOR_ALL_FRAMES(frp, frp->fr_child) {
frame_add_statusline(frp);
}
} else {
assert(frp->fr_layout == FR_COL);
for (frp = frp->fr_child; frp->fr_next != NULL; frp = frp->fr_next) {
}
frame_add_statusline(frp);
}
}
static void 
frame_new_width (
frame_T *topfrp,
int width,
int leftfirst, 
int wfw 
)
{
frame_T *frp;
int extra_cols;
int w;
win_T *wp;
if (topfrp->fr_layout == FR_LEAF) {
wp = topfrp->fr_win;
for (frp = topfrp; frp->fr_parent != NULL; frp = frp->fr_parent)
if (frp->fr_parent->fr_layout == FR_ROW && frp->fr_next != NULL)
break;
if (frp->fr_parent == NULL)
wp->w_vsep_width = 0;
win_new_width(wp, width - wp->w_vsep_width);
} else if (topfrp->fr_layout == FR_COL) {
do {
FOR_ALL_FRAMES(frp, topfrp->fr_child) {
frame_new_width(frp, width, leftfirst, wfw);
if (frp->fr_width > width) {
width = frp->fr_width;
break;
}
}
} while (frp != NULL);
} else { 
frp = topfrp->fr_child;
if (wfw)
while (frame_fixed_width(frp)) {
frp = frp->fr_next;
if (frp == NULL)
return; 
}
if (!leftfirst) {
while (frp->fr_next != NULL)
frp = frp->fr_next;
if (wfw)
while (frame_fixed_width(frp))
frp = frp->fr_prev;
}
extra_cols = width - topfrp->fr_width;
if (extra_cols < 0) {
while (frp != NULL) {
w = frame_minwidth(frp, NULL);
if (frp->fr_width + extra_cols < w) {
extra_cols += frp->fr_width - w;
frame_new_width(frp, w, leftfirst, wfw);
} else {
frame_new_width(frp, frp->fr_width + extra_cols,
leftfirst, wfw);
break;
}
if (leftfirst) {
do
frp = frp->fr_next;
while (wfw && frp != NULL && frame_fixed_width(frp));
} else {
do
frp = frp->fr_prev;
while (wfw && frp != NULL && frame_fixed_width(frp));
}
if (frp == NULL)
width -= extra_cols;
}
} else if (extra_cols > 0) {
frame_new_width(frp, frp->fr_width + extra_cols, leftfirst, wfw);
}
}
topfrp->fr_width = width;
}
static void frame_add_vsep(const frame_T *frp)
FUNC_ATTR_NONNULL_ARG(1)
{
win_T *wp;
if (frp->fr_layout == FR_LEAF) {
wp = frp->fr_win;
if (wp->w_vsep_width == 0) {
if (wp->w_width > 0) 
--wp->w_width;
wp->w_vsep_width = 1;
}
} else if (frp->fr_layout == FR_COL) {
FOR_ALL_FRAMES(frp, frp->fr_child) {
frame_add_vsep(frp);
}
} else {
assert(frp->fr_layout == FR_ROW);
frp = frp->fr_child;
while (frp->fr_next != NULL)
frp = frp->fr_next;
frame_add_vsep(frp);
}
}
static void frame_fix_width(win_T *wp)
{
wp->w_frame->fr_width = wp->w_width + wp->w_vsep_width;
}
static void frame_fix_height(win_T *wp)
{
wp->w_frame->fr_height = wp->w_height + wp->w_status_height;
}
static int frame_minheight(frame_T *topfrp, win_T *next_curwin)
{
frame_T *frp;
int m;
int n;
if (topfrp->fr_win != NULL) {
if (topfrp->fr_win == next_curwin)
m = p_wh + topfrp->fr_win->w_status_height;
else {
m = p_wmh + topfrp->fr_win->w_status_height;
if (p_wmh == 0 && topfrp->fr_win == curwin && next_curwin == NULL)
++m;
}
} else if (topfrp->fr_layout == FR_ROW) {
m = 0;
FOR_ALL_FRAMES(frp, topfrp->fr_child) {
n = frame_minheight(frp, next_curwin);
if (n > m)
m = n;
}
} else {
m = 0;
FOR_ALL_FRAMES(frp, topfrp->fr_child) {
m += frame_minheight(frp, next_curwin);
}
}
return m;
}
static int 
frame_minwidth (
frame_T *topfrp,
win_T *next_curwin 
)
{
frame_T *frp;
int m, n;
if (topfrp->fr_win != NULL) {
if (topfrp->fr_win == next_curwin)
m = p_wiw + topfrp->fr_win->w_vsep_width;
else {
m = p_wmw + topfrp->fr_win->w_vsep_width;
if (p_wmw == 0 && topfrp->fr_win == curwin && next_curwin == NULL)
++m;
}
} else if (topfrp->fr_layout == FR_COL) {
m = 0;
FOR_ALL_FRAMES(frp, topfrp->fr_child) {
n = frame_minwidth(frp, next_curwin);
if (n > m)
m = n;
}
} else {
m = 0;
FOR_ALL_FRAMES(frp, topfrp->fr_child) {
m += frame_minwidth(frp, next_curwin);
}
}
return m;
}
void 
close_others (
int message,
int forceit 
)
{
win_T *wp;
win_T *nextwp;
int r;
if (curwin->w_floating) {
if (message && !autocmd_busy) {
EMSG(e_floatonly);
}
return;
}
if (one_window() && !lastwin->w_floating) {
if (message
&& !autocmd_busy
) {
MSG(_(m_onlyone));
}
return;
}
for (wp = firstwin; win_valid(wp); wp = nextwp) {
nextwp = wp->w_next;
if (wp == curwin) { 
continue;
}
r = can_abandon(wp->w_buffer, forceit);
if (!win_valid(wp)) { 
nextwp = firstwin;
continue;
}
if (!r) {
if (message && (p_confirm || cmdmod.confirm) && p_write) {
dialog_changed(wp->w_buffer, false);
if (!win_valid(wp)) { 
nextwp = firstwin;
continue;
}
}
if (bufIsChanged(wp->w_buffer))
continue;
}
win_close(wp, !buf_hide(wp->w_buffer) && !bufIsChanged(wp->w_buffer));
}
if (message && !ONE_WINDOW)
EMSG(_("E445: Other window contains changes"));
}
void curwin_init(void)
{
win_init_empty(curwin);
}
void win_init_empty(win_T *wp)
{
redraw_win_later(wp, NOT_VALID);
wp->w_lines_valid = 0;
wp->w_cursor.lnum = 1;
wp->w_curswant = wp->w_cursor.col = 0;
wp->w_cursor.coladd = 0;
wp->w_pcmark.lnum = 1; 
wp->w_pcmark.col = 0;
wp->w_prev_pcmark.lnum = 0;
wp->w_prev_pcmark.col = 0;
wp->w_topline = 1;
wp->w_topfill = 0;
wp->w_botline = 2;
wp->w_s = &wp->w_buffer->b_s;
}
int win_alloc_first(void)
{
if (win_alloc_firstwin(NULL) == FAIL)
return FAIL;
first_tabpage = alloc_tabpage();
first_tabpage->tp_topframe = topframe;
curtab = first_tabpage;
return OK;
}
void win_alloc_aucmd_win(void)
{
Error err = ERROR_INIT;
FloatConfig fconfig = FLOAT_CONFIG_INIT;
fconfig.width = Columns;
fconfig.height = 5;
fconfig.focusable = false;
aucmd_win = win_new_float(NULL, fconfig, &err);
aucmd_win->w_buffer->b_nwindows--;
RESET_BINDING(aucmd_win);
}
static int win_alloc_firstwin(win_T *oldwin)
{
curwin = win_alloc(NULL, FALSE);
if (oldwin == NULL) {
curbuf = buflist_new(NULL, NULL, 1L, BLN_LISTED);
if (curbuf == NULL) {
return FAIL;
}
curwin->w_buffer = curbuf;
curwin->w_s = &(curbuf->b_s);
curbuf->b_nwindows = 1; 
curwin->w_alist = &global_alist;
curwin_init(); 
} else {
win_init(curwin, oldwin, 0);
RESET_BINDING(curwin);
}
new_frame(curwin);
topframe = curwin->w_frame;
topframe->fr_width = Columns;
topframe->fr_height = Rows - p_ch;
return OK;
}
static void new_frame(win_T *wp)
{
frame_T *frp = xcalloc(1, sizeof(frame_T));
wp->w_frame = frp;
frp->fr_layout = FR_LEAF;
frp->fr_win = wp;
}
void win_init_size(void)
{
firstwin->w_height = ROWS_AVAIL;
firstwin->w_height_inner = firstwin->w_height;
topframe->fr_height = ROWS_AVAIL;
firstwin->w_width = Columns;
firstwin->w_width_inner = firstwin->w_width;
topframe->fr_width = Columns;
}
static tabpage_T *alloc_tabpage(void)
{
static int last_tp_handle = 0;
tabpage_T *tp = xcalloc(1, sizeof(tabpage_T));
tp->handle = ++last_tp_handle;
handle_register_tabpage(tp);
tp->tp_vars = tv_dict_alloc();
init_var_dict(tp->tp_vars, &tp->tp_winvar, VAR_SCOPE);
tp->tp_diff_invalid = TRUE;
tp->tp_ch_used = p_ch;
return tp;
}
void free_tabpage(tabpage_T *tp)
{
int idx;
handle_unregister_tabpage(tp);
diff_clear(tp);
for (idx = 0; idx < SNAP_COUNT; ++idx)
clear_snapshot(tp, idx);
vars_clear(&tp->tp_vars->dv_hashtab); 
hash_init(&tp->tp_vars->dv_hashtab);
unref_var_dict(tp->tp_vars);
if (tp == lastused_tabpage) {
lastused_tabpage = NULL;
}
xfree(tp->tp_localdir);
xfree(tp);
}
int win_new_tabpage(int after, char_u *filename)
{
tabpage_T *tp = curtab;
tabpage_T *newtp;
int n;
newtp = alloc_tabpage();
if (leave_tabpage(curbuf, TRUE) == FAIL) {
xfree(newtp);
return FAIL;
}
newtp->tp_localdir = tp->tp_localdir ? vim_strsave(tp->tp_localdir) : NULL;
curtab = newtp;
if (win_alloc_firstwin(tp->tp_curwin) == OK) {
if (after == 1) {
newtp->tp_next = first_tabpage;
first_tabpage = newtp;
} else {
if (after > 0) {
n = 2;
for (tp = first_tabpage; tp->tp_next != NULL
&& n < after; tp = tp->tp_next)
++n;
}
newtp->tp_next = tp->tp_next;
tp->tp_next = newtp;
}
win_init_size();
firstwin->w_winrow = tabline_height();
win_comp_scroll(curwin);
newtp->tp_topframe = topframe;
last_status(FALSE);
redraw_all_later(NOT_VALID);
tabpage_check_windows(tp);
lastused_tabpage = tp;
apply_autocmds(EVENT_WINNEW, NULL, NULL, false, curbuf);
apply_autocmds(EVENT_WINENTER, NULL, NULL, false, curbuf);
apply_autocmds(EVENT_TABNEW, filename, filename, false, curbuf);
apply_autocmds(EVENT_TABENTER, NULL, NULL, false, curbuf);
return OK;
}
enter_tabpage(curtab, curbuf, TRUE, TRUE);
return FAIL;
}
int may_open_tabpage(void)
{
int n = (cmdmod.tab == 0) ? postponed_split_tab : cmdmod.tab;
if (n != 0) {
cmdmod.tab = 0; 
postponed_split_tab = 0;
return win_new_tabpage(n, NULL);
}
return FAIL;
}
int make_tabpages(int maxcount)
{
int count = maxcount;
int todo;
if (count > p_tpm)
count = p_tpm;
block_autocmds();
for (todo = count - 1; todo > 0; --todo) {
if (win_new_tabpage(0, NULL) == FAIL) {
break;
}
}
unblock_autocmds();
return count - todo;
}
bool valid_tabpage(tabpage_T *tpc) FUNC_ATTR_PURE FUNC_ATTR_WARN_UNUSED_RESULT
{
FOR_ALL_TABS(tp) {
if (tp == tpc) {
return true;
}
}
return false;
}
int valid_tabpage_win(tabpage_T *tpc)
{
FOR_ALL_TABS(tp) {
if (tp == tpc) {
FOR_ALL_WINDOWS_IN_TAB(wp, tp) {
if (win_valid_any_tab(wp)) {
return true;
}
}
return false;
}
}
return false;
}
void close_tabpage(tabpage_T *tab)
{
tabpage_T *ptp;
if (tab == first_tabpage) {
first_tabpage = tab->tp_next;
ptp = first_tabpage;
} else {
for (ptp = first_tabpage; ptp != NULL && ptp->tp_next != tab;
ptp = ptp->tp_next) {
}
assert(ptp != NULL);
ptp->tp_next = tab->tp_next;
}
goto_tabpage_tp(ptp, false, false);
free_tabpage(tab);
}
tabpage_T *find_tabpage(int n)
{
tabpage_T *tp;
int i = 1;
for (tp = first_tabpage; tp != NULL && i != n; tp = tp->tp_next)
++i;
return tp;
}
int tabpage_index(tabpage_T *ftp)
{
int i = 1;
tabpage_T *tp;
for (tp = first_tabpage; tp != NULL && tp != ftp; tp = tp->tp_next)
++i;
return i;
}
static int 
leave_tabpage (
buf_T *new_curbuf, 
int trigger_leave_autocmds
)
{
tabpage_T *tp = curtab;
reset_VIsual_and_resel(); 
if (trigger_leave_autocmds) {
if (new_curbuf != curbuf) {
apply_autocmds(EVENT_BUFLEAVE, NULL, NULL, FALSE, curbuf);
if (curtab != tp)
return FAIL;
}
apply_autocmds(EVENT_WINLEAVE, NULL, NULL, FALSE, curbuf);
if (curtab != tp)
return FAIL;
apply_autocmds(EVENT_TABLEAVE, NULL, NULL, FALSE, curbuf);
if (curtab != tp)
return FAIL;
}
tp->tp_curwin = curwin;
tp->tp_prevwin = prevwin;
tp->tp_firstwin = firstwin;
tp->tp_lastwin = lastwin;
tp->tp_old_Rows = Rows;
tp->tp_old_Columns = Columns;
firstwin = NULL;
lastwin = NULL;
return OK;
}
static void enter_tabpage(tabpage_T *tp, buf_T *old_curbuf, int trigger_enter_autocmds, int trigger_leave_autocmds)
{
int old_off = tp->tp_firstwin->w_winrow;
win_T *next_prevwin = tp->tp_prevwin;
tabpage_T *old_curtab = curtab;
curtab = tp;
firstwin = tp->tp_firstwin;
lastwin = tp->tp_lastwin;
topframe = tp->tp_topframe;
if (old_curtab != curtab) {
tabpage_check_windows(old_curtab);
}
win_enter_ext(tp->tp_curwin, false, true, false,
trigger_enter_autocmds, trigger_leave_autocmds);
prevwin = next_prevwin;
last_status(false); 
(void)win_comp_pos(); 
diff_need_scrollbind = true;
if (p_ch != curtab->tp_ch_used) {
clear_cmdline = true;
}
p_ch = curtab->tp_ch_used;
if (curtab->tp_old_Rows != Rows || (old_off != firstwin->w_winrow
))
shell_new_rows();
if (curtab->tp_old_Columns != Columns && starting == 0)
shell_new_columns(); 
lastused_tabpage = old_curtab;
if (trigger_enter_autocmds) {
apply_autocmds(EVENT_TABENTER, NULL, NULL, FALSE, curbuf);
if (old_curbuf != curbuf)
apply_autocmds(EVENT_BUFENTER, NULL, NULL, FALSE, curbuf);
}
redraw_all_later(NOT_VALID);
}
static void tabpage_check_windows(tabpage_T *old_curtab)
{
win_T *next_wp;
for (win_T *wp = old_curtab->tp_firstwin; wp; wp = next_wp) {
next_wp = wp->w_next;
if (wp->w_floating) {
if (wp->w_float_config.external) {
win_remove(wp, old_curtab);
win_append(lastwin_nofloating(), wp);
} else {
ui_comp_remove_grid(&wp->w_grid);
}
}
wp->w_pos_changed = true;
}
for (win_T *wp = firstwin; wp; wp = wp->w_next) {
if (wp->w_floating && !wp->w_float_config.external) {
win_config_float(wp, wp->w_float_config);
}
wp->w_pos_changed = true;
}
}
void goto_tabpage(int n)
{
tabpage_T *tp = NULL; 
tabpage_T *ttp;
int i;
if (text_locked()) {
text_locked_msg();
return;
}
if (first_tabpage->tp_next == NULL) {
if (n > 1)
beep_flush();
return;
}
if (n == 0) {
if (curtab->tp_next == NULL)
tp = first_tabpage;
else
tp = curtab->tp_next;
} else if (n < 0) {
ttp = curtab;
for (i = n; i < 0; ++i) {
for (tp = first_tabpage; tp->tp_next != ttp && tp->tp_next != NULL;
tp = tp->tp_next)
;
ttp = tp;
}
} else if (n == 9999) {
for (tp = first_tabpage; tp->tp_next != NULL; tp = tp->tp_next)
;
} else {
tp = find_tabpage(n);
if (tp == NULL) {
beep_flush();
return;
}
}
goto_tabpage_tp(tp, TRUE, TRUE);
}
void goto_tabpage_tp(tabpage_T *tp, int trigger_enter_autocmds, int trigger_leave_autocmds)
{
set_keep_msg(NULL, 0);
if (tp != curtab && leave_tabpage(tp->tp_curwin->w_buffer,
trigger_leave_autocmds) == OK) {
if (valid_tabpage(tp))
enter_tabpage(tp, curbuf, trigger_enter_autocmds,
trigger_leave_autocmds);
else
enter_tabpage(curtab, curbuf, trigger_enter_autocmds,
trigger_leave_autocmds);
}
}
void goto_tabpage_lastused(void)
{
int index = tabpage_index(lastused_tabpage);
if (index < tabpage_index(NULL)) {
goto_tabpage(index);
}
}
void goto_tabpage_win(tabpage_T *tp, win_T *wp)
{
goto_tabpage_tp(tp, TRUE, TRUE);
if (curtab == tp && win_valid(wp)) {
win_enter(wp, true);
}
}
void tabpage_move(int nr)
{
int n = 1;
tabpage_T *tp;
tabpage_T *tp_dst;
assert(curtab != NULL);
if (first_tabpage->tp_next == NULL) {
return;
}
for (tp = first_tabpage; tp->tp_next != NULL && n < nr; tp = tp->tp_next) {
++n;
}
if (tp == curtab || (nr > 0 && tp->tp_next != NULL
&& tp->tp_next == curtab)) {
return;
}
tp_dst = tp;
if (curtab == first_tabpage) {
first_tabpage = curtab->tp_next;
} else {
tp = NULL;
FOR_ALL_TABS(tp2) {
if (tp2->tp_next == curtab) {
tp = tp2;
break;
}
}
if (tp == NULL) { 
return;
}
tp->tp_next = curtab->tp_next;
}
if (nr <= 0) {
curtab->tp_next = first_tabpage;
first_tabpage = curtab;
} else {
curtab->tp_next = tp_dst->tp_next;
tp_dst->tp_next = curtab;
}
redraw_tabline = TRUE;
}
void win_goto(win_T *wp)
{
win_T *owp = curwin;
if (text_locked()) {
beep_flush();
text_locked_msg();
return;
}
if (curbuf_locked())
return;
if (wp->w_buffer != curbuf)
reset_VIsual_and_resel();
else if (VIsual_active)
wp->w_cursor = curwin->w_cursor;
win_enter(wp, true);
if (win_valid(owp) && owp->w_p_cole > 0 && !msg_scrolled) {
redrawWinline(owp, owp->w_cursor.lnum);
}
if (curwin->w_p_cole > 0 && !msg_scrolled) {
redrawWinline(curwin, curwin->w_cursor.lnum);
}
}
tabpage_T *win_find_tabpage(win_T *win)
{
FOR_ALL_TAB_WINDOWS(tp, wp) {
if (wp == win) {
return tp;
}
}
return NULL;
}
win_T *win_vert_neighbor(tabpage_T *tp, win_T *wp, bool up, long count)
{
frame_T *fr;
frame_T *nfr;
frame_T *foundfr;
foundfr = wp->w_frame;
if (wp->w_floating) {
return win_valid(prevwin) && !prevwin->w_floating ? prevwin : firstwin;
}
while (count--) {
fr = foundfr;
for (;; ) {
if (fr == tp->tp_topframe) {
goto end;
}
if (up) {
nfr = fr->fr_prev;
} else {
nfr = fr->fr_next;
}
if (fr->fr_parent->fr_layout == FR_COL && nfr != NULL) {
break;
}
fr = fr->fr_parent;
}
for (;; ) {
if (nfr->fr_layout == FR_LEAF) {
foundfr = nfr;
break;
}
fr = nfr->fr_child;
if (nfr->fr_layout == FR_ROW) {
while (fr->fr_next != NULL
&& frame2win(fr)->w_wincol + fr->fr_width
<= wp->w_wincol + wp->w_wcol) {
fr = fr->fr_next;
}
}
if (nfr->fr_layout == FR_COL && up)
while (fr->fr_next != NULL)
fr = fr->fr_next;
nfr = fr;
}
}
end:
return foundfr != NULL ? foundfr->fr_win : NULL;
}
static void win_goto_ver(bool up, long count)
{
win_T *win = win_vert_neighbor(curtab, curwin, up, count);
if (win != NULL) {
win_goto(win);
}
}
win_T *win_horz_neighbor(tabpage_T *tp, win_T *wp, bool left, long count)
{
frame_T *fr;
frame_T *nfr;
frame_T *foundfr;
foundfr = wp->w_frame;
if (wp->w_floating) {
return win_valid(prevwin) && !prevwin->w_floating ? prevwin : firstwin;
}
while (count--) {
fr = foundfr;
for (;; ) {
if (fr == tp->tp_topframe) {
goto end;
}
if (left) {
nfr = fr->fr_prev;
} else {
nfr = fr->fr_next;
}
if (fr->fr_parent->fr_layout == FR_ROW && nfr != NULL) {
break;
}
fr = fr->fr_parent;
}
for (;; ) {
if (nfr->fr_layout == FR_LEAF) {
foundfr = nfr;
break;
}
fr = nfr->fr_child;
if (nfr->fr_layout == FR_COL) {
while (fr->fr_next != NULL
&& frame2win(fr)->w_winrow + fr->fr_height
<= wp->w_winrow + wp->w_wrow)
fr = fr->fr_next;
}
if (nfr->fr_layout == FR_ROW && left)
while (fr->fr_next != NULL)
fr = fr->fr_next;
nfr = fr;
}
}
end:
return foundfr != NULL ? foundfr->fr_win : NULL;
}
static void win_goto_hor(bool left, long count)
{
win_T *win = win_horz_neighbor(curtab, curwin, left, count);
if (win != NULL) {
win_goto(win);
}
}
void win_enter(win_T *wp, bool undo_sync)
{
win_enter_ext(wp, undo_sync, false, false, true, true);
}
static void win_enter_ext(win_T *wp, bool undo_sync, int curwin_invalid,
int trigger_new_autocmds, int trigger_enter_autocmds,
int trigger_leave_autocmds)
{
int other_buffer = FALSE;
if (wp == curwin && !curwin_invalid) 
return;
if (!curwin_invalid && trigger_leave_autocmds) {
if (wp->w_buffer != curbuf) {
apply_autocmds(EVENT_BUFLEAVE, NULL, NULL, FALSE, curbuf);
other_buffer = TRUE;
if (!win_valid(wp))
return;
}
apply_autocmds(EVENT_WINLEAVE, NULL, NULL, FALSE, curbuf);
if (!win_valid(wp))
return;
if (aborting())
return;
}
if (undo_sync && curbuf != wp->w_buffer) {
u_sync(FALSE);
}
update_topline();
if (wp->w_buffer != curbuf) {
buf_copy_options(wp->w_buffer, BCO_ENTER | BCO_NOHELP);
}
if (!curwin_invalid) {
prevwin = curwin; 
curwin->w_redr_status = TRUE;
}
curwin = wp;
curbuf = wp->w_buffer;
check_cursor();
if (!virtual_active())
curwin->w_cursor.coladd = 0;
changed_line_abv_curs(); 
char *new_dir = (char *)(curwin->w_localdir
? curwin->w_localdir : curtab->tp_localdir);
char cwd[MAXPATHL];
if (os_dirname((char_u *)cwd, MAXPATHL) != OK) {
cwd[0] = NUL;
}
if (new_dir) {
if (globaldir == NULL) {
if (cwd[0] != NUL) {
globaldir = (char_u *)xstrdup(cwd);
}
}
if (os_chdir(new_dir) == 0) {
if (!p_acd && !strequal(new_dir, cwd)) {
do_autocmd_dirchanged(new_dir, curwin->w_localdir
? kCdScopeWindow : kCdScopeTab);
}
shorten_fnames(true);
}
} else if (globaldir != NULL) {
if (os_chdir((char *)globaldir) == 0) {
if (!p_acd && !strequal((char *)globaldir, cwd)) {
do_autocmd_dirchanged((char *)globaldir, kCdScopeGlobal);
}
}
XFREE_CLEAR(globaldir);
shorten_fnames(true);
}
if (trigger_new_autocmds) {
apply_autocmds(EVENT_WINNEW, NULL, NULL, false, curbuf);
}
if (trigger_enter_autocmds) {
apply_autocmds(EVENT_WINENTER, NULL, NULL, false, curbuf);
if (other_buffer) {
apply_autocmds(EVENT_BUFENTER, NULL, NULL, false, curbuf);
}
apply_autocmds(EVENT_CURSORMOVED, NULL, NULL, false, curbuf);
curwin->w_last_cursormoved = curwin->w_cursor;
}
maketitle();
curwin->w_redr_status = TRUE;
redraw_tabline = TRUE;
if (restart_edit)
redraw_later(VALID); 
if (HL_ATTR(HLF_INACTIVE)
|| (prevwin && prevwin->w_hl_ids[HLF_INACTIVE])
|| curwin->w_hl_ids[HLF_INACTIVE]) {
redraw_all_later(NOT_VALID);
}
if (curwin->w_height < p_wh && !curwin->w_p_wfh)
win_setheight((int)p_wh);
else if (curwin->w_height == 0)
win_setheight(1);
if (curwin->w_width < p_wiw && !curwin->w_p_wfw && !wp->w_floating) {
win_setwidth((int)p_wiw);
}
setmouse(); 
do_autochdir();
}
win_T *buf_jump_open_win(buf_T *buf)
{
if (curwin->w_buffer == buf) {
win_enter(curwin, false);
return curwin;
} else {
FOR_ALL_WINDOWS_IN_TAB(wp, curtab) {
if (wp->w_buffer == buf) {
win_enter(wp, false);
return wp;
}
}
}
return NULL;
}
win_T *buf_jump_open_tab(buf_T *buf)
{
{
win_T *wp = buf_jump_open_win(buf);
if (wp != NULL)
return wp;
}
FOR_ALL_TABS(tp) {
if (tp == curtab) {
continue;
}
FOR_ALL_WINDOWS_IN_TAB(wp, tp) {
if (wp->w_buffer == buf) {
goto_tabpage_win(tp, wp);
if (curwin != wp) {
wp = NULL;
}
return wp;
}
}
}
return NULL;
}
static win_T *win_alloc(win_T *after, int hidden)
{
static int last_win_id = LOWEST_WIN_ID - 1;
win_T *new_wp = xcalloc(1, sizeof(win_T));
new_wp->handle = ++last_win_id;
handle_register_window(new_wp);
grid_assign_handle(&new_wp->w_grid);
new_wp->w_vars = tv_dict_alloc();
init_var_dict(new_wp->w_vars, &new_wp->w_winvar, VAR_SCOPE);
block_autocmds();
if (!hidden)
win_append(after, new_wp);
new_wp->w_wincol = 0;
new_wp->w_width = Columns;
new_wp->w_topline = 1;
new_wp->w_topfill = 0;
new_wp->w_botline = 2;
new_wp->w_cursor.lnum = 1;
new_wp->w_scbind_pos = 1;
new_wp->w_floating = 0;
new_wp->w_float_config = FLOAT_CONFIG_INIT;
new_wp->w_viewport_invalid = true;
new_wp->w_p_so = -1;
new_wp->w_p_siso = -1;
new_wp->w_fraction = 0;
new_wp->w_prev_fraction_row = -1;
foldInitWin(new_wp);
unblock_autocmds();
new_wp->w_match_head = NULL;
new_wp->w_next_match_id = 4;
return new_wp;
}
static void 
win_free (
win_T *wp,
tabpage_T *tp 
)
{
int i;
wininfo_T *wip;
handle_unregister_window(wp);
clearFolding(wp);
alist_unlink(wp->w_alist);
block_autocmds();
clear_winopt(&wp->w_onebuf_opt);
clear_winopt(&wp->w_allbuf_opt);
vars_clear(&wp->w_vars->dv_hashtab); 
hash_init(&wp->w_vars->dv_hashtab);
unref_var_dict(wp->w_vars);
if (prevwin == wp) {
prevwin = NULL;
}
FOR_ALL_TABS(ttp) {
if (ttp->tp_prevwin == wp) {
ttp->tp_prevwin = NULL;
}
}
xfree(wp->w_lines);
for (i = 0; i < wp->w_tagstacklen; i++) {
xfree(wp->w_tagstack[i].tagname);
xfree(wp->w_tagstack[i].user_data);
}
xfree(wp->w_localdir);
FOR_ALL_BUFFERS(buf) {
for (wip = buf->b_wininfo; wip != NULL; wip = wip->wi_next)
if (wip->wi_win == wp)
wip->wi_win = NULL;
}
clear_matches(wp);
free_jumplist(wp);
qf_free_all(wp);
xfree(wp->w_p_cc_cols);
win_free_grid(wp, false);
if (wp != aucmd_win)
win_remove(wp, tp);
if (autocmd_busy) {
wp->w_next = au_pending_free_win;
au_pending_free_win = wp;
} else {
xfree(wp);
}
unblock_autocmds();
}
void win_free_grid(win_T *wp, bool reinit)
{
if (wp->w_grid.handle != 0 && ui_has(kUIMultigrid)) {
ui_call_grid_destroy(wp->w_grid.handle);
wp->w_grid.handle = 0;
}
grid_free(&wp->w_grid);
if (reinit) {
memset(&wp->w_grid, 0, sizeof(wp->w_grid));
}
}
void win_append(win_T *after, win_T *wp)
{
win_T *before;
if (after == NULL) 
before = firstwin;
else
before = after->w_next;
wp->w_next = before;
wp->w_prev = after;
if (after == NULL)
firstwin = wp;
else
after->w_next = wp;
if (before == NULL)
lastwin = wp;
else
before->w_prev = wp;
}
void 
win_remove (
win_T *wp,
tabpage_T *tp 
)
{
if (wp->w_prev != NULL) {
wp->w_prev->w_next = wp->w_next;
} else if (tp == NULL) {
firstwin = curtab->tp_firstwin = wp->w_next;
} else {
tp->tp_firstwin = wp->w_next;
}
if (wp->w_next != NULL) {
wp->w_next->w_prev = wp->w_prev;
} else if (tp == NULL) {
lastwin = curtab->tp_lastwin = wp->w_prev;
} else {
tp->tp_lastwin = wp->w_prev;
}
}
static void frame_append(frame_T *after, frame_T *frp)
{
frp->fr_next = after->fr_next;
after->fr_next = frp;
if (frp->fr_next != NULL)
frp->fr_next->fr_prev = frp;
frp->fr_prev = after;
}
static void frame_insert(frame_T *before, frame_T *frp)
{
frp->fr_next = before;
frp->fr_prev = before->fr_prev;
before->fr_prev = frp;
if (frp->fr_prev != NULL)
frp->fr_prev->fr_next = frp;
else
frp->fr_parent->fr_child = frp;
}
static void frame_remove(frame_T *frp)
{
if (frp->fr_prev != NULL) {
frp->fr_prev->fr_next = frp->fr_next;
} else {
frp->fr_parent->fr_child = frp->fr_next;
if (topframe->fr_child == frp) {
topframe->fr_child = frp->fr_next;
}
}
if (frp->fr_next != NULL) {
frp->fr_next->fr_prev = frp->fr_prev;
}
}
void shell_new_rows(void)
{
int h = (int)ROWS_AVAIL;
if (firstwin == NULL) 
return;
if (h < frame_minheight(topframe, NULL))
h = frame_minheight(topframe, NULL);
frame_new_height(topframe, h, FALSE, TRUE);
if (!frame_check_height(topframe, h))
frame_new_height(topframe, h, FALSE, FALSE);
(void)win_comp_pos(); 
win_reconfig_floats(); 
compute_cmdrow();
curtab->tp_ch_used = p_ch;
}
void shell_new_columns(void)
{
if (firstwin == NULL) 
return;
frame_new_width(topframe, Columns, false, true);
if (!frame_check_width(topframe, Columns)) {
frame_new_width(topframe, Columns, false, false);
}
(void)win_comp_pos(); 
win_reconfig_floats(); 
}
void win_size_save(garray_T *gap)
{
ga_init(gap, (int)sizeof(int), 1);
ga_grow(gap, win_count() * 2);
FOR_ALL_WINDOWS_IN_TAB(wp, curtab) {
((int *)gap->ga_data)[gap->ga_len++] =
wp->w_width + wp->w_vsep_width;
((int *)gap->ga_data)[gap->ga_len++] = wp->w_height;
}
}
void win_size_restore(garray_T *gap)
{
if (win_count() * 2 == gap->ga_len) {
for (int j = 0; j < 2; ++j)
{
int i = 0;
FOR_ALL_WINDOWS_IN_TAB(wp, curtab) {
int width = ((int *)gap->ga_data)[i++];
int height = ((int *)gap->ga_data)[i++];
if (!wp->w_floating) {
frame_setwidth(wp->w_frame, width);
win_setheight_win(height, wp);
}
}
}
(void)win_comp_pos();
}
}
int win_comp_pos(void)
{
int row = tabline_height();
int col = 0;
frame_comp_pos(topframe, &row, &col);
for (win_T *wp = lastwin; wp && wp->w_floating; wp = wp->w_prev) {
if (wp->w_float_config.relative == kFloatRelativeWindow) {
wp->w_pos_changed = true;
}
}
return row;
}
void win_reconfig_floats(void)
{
for (win_T *wp = lastwin; wp && wp->w_floating; wp = wp->w_prev) {
win_config_float(wp, wp->w_float_config);
}
}
static void frame_comp_pos(frame_T *topfrp, int *row, int *col)
{
win_T *wp;
frame_T *frp;
int startcol;
int startrow;
wp = topfrp->fr_win;
if (wp != NULL) {
if (wp->w_winrow != *row
|| wp->w_wincol != *col
) {
wp->w_winrow = *row;
wp->w_wincol = *col;
redraw_win_later(wp, NOT_VALID);
wp->w_redr_status = true;
wp->w_pos_changed = true;
}
*row += wp->w_height + wp->w_status_height;
*col += wp->w_width + wp->w_vsep_width;
} else {
startrow = *row;
startcol = *col;
FOR_ALL_FRAMES(frp, topfrp->fr_child) {
if (topfrp->fr_layout == FR_ROW) {
*row = startrow; 
} else {
*col = startcol; 
}
frame_comp_pos(frp, row, col);
}
}
}
void win_setheight(int height)
{
win_setheight_win(height, curwin);
}
void win_setheight_win(int height, win_T *win)
{
if (win == curwin) {
if (height < p_wmh)
height = p_wmh;
if (height == 0)
height = 1;
}
if (win->w_floating) {
win->w_float_config.height = height;
win_config_float(win, win->w_float_config);
redraw_win_later(win, NOT_VALID);
} else {
frame_setheight(win->w_frame, height + win->w_status_height);
int row = win_comp_pos();
if (full_screen && msg_scrolled == 0 && row < cmdline_row) {
grid_fill(&default_grid, row, cmdline_row, 0, Columns, ' ', ' ', 0);
}
cmdline_row = row;
msg_row = row;
msg_col = 0;
redraw_all_later(NOT_VALID);
}
}
static void frame_setheight(frame_T *curfrp, int height)
{
int room; 
int take; 
int room_cmdline; 
int run;
frame_T *frp;
int h;
int room_reserved;
if (curfrp->fr_height == height)
return;
if (curfrp->fr_parent == NULL) {
if (height > ROWS_AVAIL)
height = ROWS_AVAIL;
if (height > 0)
frame_new_height(curfrp, height, FALSE, FALSE);
} else if (curfrp->fr_parent->fr_layout == FR_ROW) {
h = frame_minheight(curfrp->fr_parent, NULL);
if (height < h)
height = h;
frame_setheight(curfrp->fr_parent, height);
} else {
for (run = 1; run <= 2; ++run) {
room = 0;
room_reserved = 0;
FOR_ALL_FRAMES(frp, curfrp->fr_parent->fr_child) {
if (frp != curfrp
&& frp->fr_win != NULL
&& frp->fr_win->w_p_wfh) {
room_reserved += frp->fr_height;
}
room += frp->fr_height;
if (frp != curfrp) {
room -= frame_minheight(frp, NULL);
}
}
if (curfrp->fr_width != Columns) {
room_cmdline = 0;
} else {
win_T *wp = lastwin_nofloating();
room_cmdline = Rows - p_ch - (wp->w_winrow
+ wp->w_height +
wp->w_status_height);
if (room_cmdline < 0) {
room_cmdline = 0;
}
}
if (height <= room + room_cmdline) {
break;
}
if (run == 2 || curfrp->fr_width == Columns) {
height = room + room_cmdline;
break;
}
frame_setheight(curfrp->fr_parent, height
+ frame_minheight(curfrp->fr_parent, NOWIN) - (int)p_wmh - 1);
}
take = height - curfrp->fr_height;
if (height > room + room_cmdline - room_reserved)
room_reserved = room + room_cmdline - height;
if (take < 0 && room - curfrp->fr_height < room_reserved)
room_reserved = 0;
if (take > 0 && room_cmdline > 0) {
if (take < room_cmdline)
room_cmdline = take;
take -= room_cmdline;
topframe->fr_height += room_cmdline;
}
frame_new_height(curfrp, height, FALSE, FALSE);
for (run = 0; run < 2; ++run) {
if (run == 0)
frp = curfrp->fr_next; 
else
frp = curfrp->fr_prev; 
while (frp != NULL && take != 0) {
h = frame_minheight(frp, NULL);
if (room_reserved > 0
&& frp->fr_win != NULL
&& frp->fr_win->w_p_wfh) {
if (room_reserved >= frp->fr_height)
room_reserved -= frp->fr_height;
else {
if (frp->fr_height - room_reserved > take)
room_reserved = frp->fr_height - take;
take -= frp->fr_height - room_reserved;
frame_new_height(frp, room_reserved, FALSE, FALSE);
room_reserved = 0;
}
} else {
if (frp->fr_height - take < h) {
take -= frp->fr_height - h;
frame_new_height(frp, h, FALSE, FALSE);
} else {
frame_new_height(frp, frp->fr_height - take,
FALSE, FALSE);
take = 0;
}
}
if (run == 0)
frp = frp->fr_next;
else
frp = frp->fr_prev;
}
}
}
}
void win_setwidth(int width)
{
win_setwidth_win(width, curwin);
}
void win_setwidth_win(int width, win_T *wp)
{
if (wp == curwin) {
if (width < p_wmw)
width = p_wmw;
if (width == 0)
width = 1;
}
if (wp->w_floating) {
wp->w_float_config.width = width;
win_config_float(wp, wp->w_float_config);
redraw_win_later(wp, NOT_VALID);
} else {
frame_setwidth(wp->w_frame, width + wp->w_vsep_width);
(void)win_comp_pos();
redraw_all_later(NOT_VALID);
}
}
static void frame_setwidth(frame_T *curfrp, int width)
{
int room; 
int take; 
int run;
frame_T *frp;
int w;
int room_reserved;
if (curfrp->fr_width == width)
return;
if (curfrp->fr_parent == NULL)
return;
if (curfrp->fr_parent->fr_layout == FR_COL) {
w = frame_minwidth(curfrp->fr_parent, NULL);
if (width < w)
width = w;
frame_setwidth(curfrp->fr_parent, width);
} else {
for (run = 1; run <= 2; ++run) {
room = 0;
room_reserved = 0;
FOR_ALL_FRAMES(frp, curfrp->fr_parent->fr_child) {
if (frp != curfrp
&& frp->fr_win != NULL
&& frp->fr_win->w_p_wfw) {
room_reserved += frp->fr_width;
}
room += frp->fr_width;
if (frp != curfrp) {
room -= frame_minwidth(frp, NULL);
}
}
if (width <= room)
break;
if (run == 2 || curfrp->fr_height >= ROWS_AVAIL) {
width = room;
break;
}
frame_setwidth(curfrp->fr_parent, width
+ frame_minwidth(curfrp->fr_parent, NOWIN) - (int)p_wmw - 1);
}
take = width - curfrp->fr_width;
if (width > room - room_reserved)
room_reserved = room - width;
if (take < 0 && room - curfrp->fr_width < room_reserved)
room_reserved = 0;
frame_new_width(curfrp, width, FALSE, FALSE);
for (run = 0; run < 2; ++run) {
if (run == 0)
frp = curfrp->fr_next; 
else
frp = curfrp->fr_prev; 
while (frp != NULL && take != 0) {
w = frame_minwidth(frp, NULL);
if (room_reserved > 0
&& frp->fr_win != NULL
&& frp->fr_win->w_p_wfw) {
if (room_reserved >= frp->fr_width)
room_reserved -= frp->fr_width;
else {
if (frp->fr_width - room_reserved > take)
room_reserved = frp->fr_width - take;
take -= frp->fr_width - room_reserved;
frame_new_width(frp, room_reserved, FALSE, FALSE);
room_reserved = 0;
}
} else {
if (frp->fr_width - take < w) {
take -= frp->fr_width - w;
frame_new_width(frp, w, FALSE, FALSE);
} else {
frame_new_width(frp, frp->fr_width - take,
FALSE, FALSE);
take = 0;
}
}
if (run == 0)
frp = frp->fr_next;
else
frp = frp->fr_prev;
}
}
}
}
void win_setminheight(void)
{
bool first = true;
while (p_wmh > 0) {
const int room = Rows - p_ch;
const int needed = frame_minheight(topframe, NULL);
if (room >= needed) {
break;
}
p_wmh--;
if (first) {
EMSG(_(e_noroom));
first = false;
}
}
}
void win_setminwidth(void)
{
bool first = true;
while (p_wmw > 0) {
const int room = Columns;
const int needed = frame_minwidth(topframe, NULL);
if (room >= needed) {
break;
}
p_wmw--;
if (first) {
EMSG(_(e_noroom));
first = false;
}
}
}
void win_drag_status_line(win_T *dragwin, int offset)
{
frame_T *curfr;
frame_T *fr;
int room;
int row;
int up; 
int n;
fr = dragwin->w_frame;
curfr = fr;
if (fr != topframe) { 
fr = fr->fr_parent;
if (fr->fr_layout != FR_COL) {
curfr = fr;
if (fr != topframe) 
fr = fr->fr_parent;
}
}
while (curfr != topframe && curfr->fr_next == NULL) {
if (fr != topframe)
fr = fr->fr_parent;
curfr = fr;
if (fr != topframe)
fr = fr->fr_parent;
}
if (offset < 0) { 
up = TRUE;
offset = -offset;
if (fr == curfr) {
room = fr->fr_height - frame_minheight(fr, NULL);
} else {
room = 0;
for (fr = fr->fr_child;; fr = fr->fr_next) {
room += fr->fr_height - frame_minheight(fr, NULL);
if (fr == curfr)
break;
}
}
fr = curfr->fr_next; 
} else { 
up = FALSE;
room = Rows - cmdline_row;
if (curfr->fr_next == NULL)
room -= 1;
else
room -= p_ch;
if (room < 0)
room = 0;
FOR_ALL_FRAMES(fr, curfr->fr_next) {
room += fr->fr_height - frame_minheight(fr, NULL);
}
fr = curfr; 
}
if (room < offset) 
offset = room; 
if (offset <= 0)
return;
if (fr != NULL)
frame_new_height(fr, fr->fr_height + offset, up, FALSE);
if (up)
fr = curfr; 
else
fr = curfr->fr_next; 
while (fr != NULL && offset > 0) {
n = frame_minheight(fr, NULL);
if (fr->fr_height - offset <= n) {
offset -= fr->fr_height - n;
frame_new_height(fr, n, !up, FALSE);
} else {
frame_new_height(fr, fr->fr_height - offset, !up, FALSE);
break;
}
if (up)
fr = fr->fr_prev;
else
fr = fr->fr_next;
}
row = win_comp_pos();
grid_fill(&default_grid, row, cmdline_row, 0, Columns, ' ', ' ', 0);
if (msg_grid.chars) {
clear_cmdline = true;
}
cmdline_row = row;
p_ch = Rows - cmdline_row;
if (p_ch < 1)
p_ch = 1;
curtab->tp_ch_used = p_ch;
redraw_all_later(SOME_VALID);
showmode();
}
void win_drag_vsep_line(win_T *dragwin, int offset)
{
frame_T *curfr;
frame_T *fr;
int room;
int left; 
int n;
fr = dragwin->w_frame;
if (fr == topframe) 
return;
curfr = fr;
fr = fr->fr_parent;
if (fr->fr_layout != FR_ROW) {
if (fr == topframe) 
return;
curfr = fr;
fr = fr->fr_parent;
}
while (curfr->fr_next == NULL) {
if (fr == topframe)
break;
curfr = fr;
fr = fr->fr_parent;
if (fr != topframe) {
curfr = fr;
fr = fr->fr_parent;
}
}
if (offset < 0) { 
left = TRUE;
offset = -offset;
room = 0;
for (fr = fr->fr_child;; fr = fr->fr_next) {
room += fr->fr_width - frame_minwidth(fr, NULL);
if (fr == curfr)
break;
}
fr = curfr->fr_next; 
} else { 
left = FALSE;
room = 0;
FOR_ALL_FRAMES(fr, curfr->fr_next) {
room += fr->fr_width - frame_minwidth(fr, NULL);
}
fr = curfr; 
}
assert(fr);
if (room < offset) {
offset = room; 
}
if (offset <= 0) {
return;
}
if (fr == NULL) {
return; 
}
frame_new_width(fr, fr->fr_width + offset, left, FALSE);
if (left)
fr = curfr; 
else
fr = curfr->fr_next; 
while (fr != NULL && offset > 0) {
n = frame_minwidth(fr, NULL);
if (fr->fr_width - offset <= n) {
offset -= fr->fr_width - n;
frame_new_width(fr, n, !left, FALSE);
} else {
frame_new_width(fr, fr->fr_width - offset, !left, FALSE);
break;
}
if (left)
fr = fr->fr_prev;
else
fr = fr->fr_next;
}
(void)win_comp_pos();
redraw_all_later(NOT_VALID);
}
#define FRACTION_MULT 16384L
void set_fraction(win_T *wp)
{
if (wp->w_height_inner > 1) {
wp->w_fraction = ((long)wp->w_wrow * FRACTION_MULT + FRACTION_MULT / 2)
/ (long)wp->w_height_inner;
}
}
void win_new_height(win_T *wp, int height)
{
if (height < 0) {
height = 0;
}
if (wp->w_height == height) {
return; 
}
wp->w_height = height;
wp->w_pos_changed = true;
win_set_inner_size(wp);
}
void scroll_to_fraction(win_T *wp, int prev_height)
{
linenr_T lnum;
int sline, line_size;
int height = wp->w_height_inner;
if (height > 0
&& (!wp->w_p_scb || wp == curwin)
&& (height < wp->w_buffer->b_ml.ml_line_count || wp->w_topline > 1)
) {
lnum = wp->w_cursor.lnum;
if (lnum < 1) 
lnum = 1;
wp->w_wrow = ((long)wp->w_fraction * (long)height - 1L) / FRACTION_MULT;
line_size = plines_win_col(wp, lnum, (long)(wp->w_cursor.col)) - 1;
sline = wp->w_wrow - line_size;
if (sline >= 0) {
const int rows = plines_win(wp, lnum, false);
if (sline > wp->w_height_inner - rows) {
sline = wp->w_height_inner - rows;
wp->w_wrow -= rows - line_size;
}
}
if (sline < 0) {
wp->w_wrow = line_size;
if (wp->w_wrow >= wp->w_height_inner
&& (wp->w_width_inner - win_col_off(wp)) > 0) {
wp->w_skipcol += wp->w_width_inner - win_col_off(wp);
wp->w_wrow--;
while (wp->w_wrow >= wp->w_height_inner) {
wp->w_skipcol += wp->w_width_inner - win_col_off(wp)
+ win_col_off2(wp);
wp->w_wrow--;
}
}
} else if (sline > 0) {
while (sline > 0 && lnum > 1) {
(void)hasFoldingWin(wp, lnum, &lnum, NULL, true, NULL);
if (lnum == 1) {
line_size = 1;
--sline;
break;
}
lnum--;
if (lnum == wp->w_topline) {
line_size = plines_win_nofill(wp, lnum, true)
+ wp->w_topfill;
} else {
line_size = plines_win(wp, lnum, true);
}
sline -= line_size;
}
if (sline < 0) {
(void)hasFoldingWin(wp, lnum, NULL, &lnum, true, NULL);
lnum++;
wp->w_wrow -= line_size + sline;
} else if (sline > 0) {
lnum = 1;
wp->w_wrow -= sline;
}
}
set_topline(wp, lnum);
}
if (wp == curwin) {
if (get_scrolloff_value()) {
update_topline();
}
curs_columns(false); 
}
if (prev_height > 0) {
wp->w_prev_fraction_row = wp->w_wrow;
}
win_comp_scroll(wp);
redraw_win_later(wp, SOME_VALID);
wp->w_redr_status = TRUE;
invalidate_botline_win(wp);
}
void win_set_inner_size(win_T *wp)
{
int width = wp->w_width_request;
if (width == 0) {
width = wp->w_width;
}
int prev_height = wp->w_height_inner;
int height = wp->w_height_request;
if (height == 0) {
height = wp->w_height;
}
if (height != prev_height) {
if (height > 0) {
if (wp == curwin) {
validate_cursor();
}
if (wp->w_height_inner != prev_height) { 
return; 
}
if (wp->w_wrow != wp->w_prev_fraction_row) {
set_fraction(wp);
}
}
wp->w_height_inner = height;
wp->w_skipcol = 0;
if (!exiting) {
scroll_to_fraction(wp, prev_height);
}
redraw_win_later(wp, NOT_VALID); 
}
if (width != wp->w_width_inner) {
wp->w_width_inner = width;
wp->w_lines_valid = 0;
changed_line_abv_curs_win(wp);
invalidate_botline_win(wp);
if (wp == curwin) {
update_topline();
curs_columns(true); 
}
redraw_win_later(wp, NOT_VALID);
}
if (wp->w_buffer->terminal) {
terminal_check_size(wp->w_buffer->terminal);
}
}
void win_new_width(win_T *wp, int width)
{
wp->w_width = width;
win_set_inner_size(wp);
wp->w_redr_status = true;
wp->w_pos_changed = true;
}
void win_comp_scroll(win_T *wp)
{
wp->w_p_scr = wp->w_height / 2;
if (wp->w_p_scr == 0)
wp->w_p_scr = 1;
}
void command_height(void)
{
int h;
frame_T *frp;
int old_p_ch = curtab->tp_ch_used;
curtab->tp_ch_used = p_ch;
frp = lastwin_nofloating()->w_frame;
while (frp->fr_width != Columns && frp->fr_parent != NULL) {
frp = frp->fr_parent;
}
while (frp->fr_prev != NULL && frp->fr_layout == FR_LEAF
&& frp->fr_win->w_p_wfh)
frp = frp->fr_prev;
if (starting != NO_SCREEN) {
cmdline_row = Rows - p_ch;
if (p_ch > old_p_ch) { 
while (p_ch > old_p_ch) {
if (frp == NULL) {
EMSG(_(e_noroom));
p_ch = old_p_ch;
curtab->tp_ch_used = p_ch;
cmdline_row = Rows - p_ch;
break;
}
h = frp->fr_height - frame_minheight(frp, NULL);
if (h > p_ch - old_p_ch)
h = p_ch - old_p_ch;
old_p_ch += h;
frame_add_height(frp, -h);
frp = frp->fr_prev;
}
(void)win_comp_pos();
if (full_screen) {
grid_fill(&default_grid, cmdline_row, Rows, 0, Columns, ' ', ' ', 0);
}
msg_row = cmdline_row;
redraw_cmdline = TRUE;
return;
}
if (msg_row < cmdline_row)
msg_row = cmdline_row;
redraw_cmdline = TRUE;
}
frame_add_height(frp, (int)(old_p_ch - p_ch));
if (frp != lastwin->w_frame)
(void)win_comp_pos();
}
static void frame_add_height(frame_T *frp, int n)
{
frame_new_height(frp, frp->fr_height + n, FALSE, FALSE);
for (;; ) {
frp = frp->fr_parent;
if (frp == NULL)
break;
frp->fr_height += n;
}
}
char_u *grab_file_name(long count, linenr_T *file_lnum)
{
int options = FNAME_MESS | FNAME_EXP | FNAME_REL | FNAME_UNESC;
if (VIsual_active) {
size_t len;
char_u *ptr;
if (get_visual_text(NULL, &ptr, &len) == FAIL)
return NULL;
return find_file_name_in_path(ptr, len, options, count, curbuf->b_ffname);
}
return file_name_at_cursor(options | FNAME_HYP, count, file_lnum);
}
char_u *file_name_at_cursor(int options, long count, linenr_T *file_lnum)
{
return file_name_in_line(get_cursor_line_ptr(),
curwin->w_cursor.col, options, count, curbuf->b_ffname,
file_lnum);
}
char_u *
file_name_in_line (
char_u *line,
int col,
int options,
long count,
char_u *rel_fname, 
linenr_T *file_lnum 
)
{
char_u *ptr;
size_t len;
bool in_type = true;
bool is_url = false;
ptr = line + col;
while (*ptr != NUL && !vim_isfilec(*ptr)) {
MB_PTR_ADV(ptr);
}
if (*ptr == NUL) { 
if (options & FNAME_MESS) {
EMSG(_("E446: No file name under cursor"));
}
return NULL;
}
while (ptr > line) {
if ((len = (size_t)(utf_head_off(line, ptr - 1))) > 0) {
ptr -= len + 1;
} else if (vim_isfilec(ptr[-1])
|| ((options & FNAME_HYP) && path_is_url((char *)ptr - 1))) {
ptr--;
} else {
break;
}
}
len = 0;
while (vim_isfilec(ptr[len]) || (ptr[len] == '\\' && ptr[len + 1] == ' ')
|| ((options & FNAME_HYP) && path_is_url((char *)ptr + len))
|| (is_url && vim_strchr((char_u *)":?&=", ptr[len]) != NULL)) {
if ((ptr[len] >= 'A' && ptr[len] <= 'Z')
|| (ptr[len] >= 'a' && ptr[len] <= 'z')) {
if (in_type && path_is_url((char *)ptr + len + 1)) {
is_url = true;
}
} else {
in_type = false;
}
if (ptr[len] == '\\' && ptr[len + 1] == ' ') {
++len;
}
if (has_mbyte) {
len += (size_t)(*mb_ptr2len)(ptr + len);
} else {
++len;
}
}
if (len > 2 && vim_strchr((char_u *)".,:;!", ptr[len - 1]) != NULL
&& ptr[len - 2] != '.')
--len;
if (file_lnum != NULL) {
char_u *p;
const char *line_english = " line ";
const char *line_transl = _(line_msg);
p = ptr + len;
if (STRNCMP(p, line_english, STRLEN(line_english)) == 0) {
p += STRLEN(line_english);
} else if (STRNCMP(p, line_transl, STRLEN(line_transl)) == 0) {
p += STRLEN(line_transl);
} else {
p = skipwhite(p);
}
if (*p != NUL) {
if (!isdigit(*p)) {
p++; 
}
p = skipwhite(p);
if (isdigit(*p)) {
*file_lnum = getdigits_long(&p, false, 0);
}
}
}
return find_file_name_in_path(ptr, len, options, count, rel_fname);
}
void 
last_status (
int morewin 
)
{
last_status_rec(topframe, (p_ls == 2
|| (p_ls == 1 && (morewin || !one_window()))));
}
static void last_status_rec(frame_T *fr, int statusline)
{
frame_T *fp;
win_T *wp;
if (fr->fr_layout == FR_LEAF) {
wp = fr->fr_win;
if (wp->w_status_height != 0 && !statusline) {
win_new_height(wp, wp->w_height + 1);
wp->w_status_height = 0;
comp_col();
} else if (wp->w_status_height == 0 && statusline) {
fp = fr;
while (fp->fr_height <= frame_minheight(fp, NULL)) {
if (fp == topframe) {
EMSG(_(e_noroom));
return;
}
if (fp->fr_parent->fr_layout == FR_COL && fp->fr_prev != NULL)
fp = fp->fr_prev;
else
fp = fp->fr_parent;
}
wp->w_status_height = 1;
if (fp != fr) {
frame_new_height(fp, fp->fr_height - 1, FALSE, FALSE);
frame_fix_height(wp);
(void)win_comp_pos();
} else
win_new_height(wp, wp->w_height - 1);
comp_col();
redraw_all_later(SOME_VALID);
}
} else if (fr->fr_layout == FR_ROW) {
FOR_ALL_FRAMES(fp, fr->fr_child) {
last_status_rec(fp, statusline);
}
} else {
for (fp = fr->fr_child; fp->fr_next != NULL; fp = fp->fr_next)
;
last_status_rec(fp, statusline);
}
}
int tabline_height(void)
{
if (ui_has(kUITabline)) {
return 0;
}
assert(first_tabpage);
switch (p_stal) {
case 0: return 0;
case 1: return (first_tabpage->tp_next == NULL) ? 0 : 1;
}
return 1;
}
int min_rows(void)
{
if (firstwin == NULL) 
return MIN_LINES;
int total = 0;
FOR_ALL_TABS(tp) {
int n = frame_minheight(tp->tp_topframe, NULL);
if (total < n) {
total = n;
}
}
total += tabline_height();
total += 1; 
return total;
}
bool only_one_window(void) FUNC_ATTR_PURE FUNC_ATTR_WARN_UNUSED_RESULT
{
if (first_tabpage->tp_next != NULL) {
return false;
}
int count = 0;
FOR_ALL_WINDOWS_IN_TAB(wp, curtab) {
if (wp->w_buffer != NULL
&& (!((bt_help(wp->w_buffer) && !bt_help(curbuf)) || wp->w_floating
|| wp->w_p_pvw) || wp == curwin) && wp != aucmd_win) {
count++;
}
}
return count <= 1;
}
void check_lnums(int do_curwin)
{
FOR_ALL_TAB_WINDOWS(tp, wp) {
if ((do_curwin || wp != curwin) && wp->w_buffer == curbuf) {
wp->w_save_cursor.w_cursor_save = wp->w_cursor;
wp->w_save_cursor.w_topline_save = wp->w_topline;
if (wp->w_cursor.lnum > curbuf->b_ml.ml_line_count) {
wp->w_cursor.lnum = curbuf->b_ml.ml_line_count;
}
if (wp->w_topline > curbuf->b_ml.ml_line_count) {
wp->w_topline = curbuf->b_ml.ml_line_count;
}
wp->w_save_cursor.w_cursor_corr = wp->w_cursor;
wp->w_save_cursor.w_topline_corr = wp->w_topline;
}
}
}
void reset_lnums(void)
{
FOR_ALL_TAB_WINDOWS(tp, wp) {
if (wp->w_buffer == curbuf) {
if (equalpos(wp->w_save_cursor.w_cursor_corr, wp->w_cursor)) {
wp->w_cursor = wp->w_save_cursor.w_cursor_save;
}
if (wp->w_save_cursor.w_topline_corr == wp->w_topline) {
wp->w_topline = wp->w_save_cursor.w_topline_save;
}
}
}
}
void make_snapshot(int idx)
{
clear_snapshot(curtab, idx);
make_snapshot_rec(topframe, &curtab->tp_snapshot[idx]);
}
static void make_snapshot_rec(frame_T *fr, frame_T **frp)
{
*frp = xcalloc(1, sizeof(frame_T));
(*frp)->fr_layout = fr->fr_layout;
(*frp)->fr_width = fr->fr_width;
(*frp)->fr_height = fr->fr_height;
if (fr->fr_next != NULL)
make_snapshot_rec(fr->fr_next, &((*frp)->fr_next));
if (fr->fr_child != NULL)
make_snapshot_rec(fr->fr_child, &((*frp)->fr_child));
if (fr->fr_layout == FR_LEAF && fr->fr_win == curwin)
(*frp)->fr_win = curwin;
}
static void clear_snapshot(tabpage_T *tp, int idx)
{
clear_snapshot_rec(tp->tp_snapshot[idx]);
tp->tp_snapshot[idx] = NULL;
}
static void clear_snapshot_rec(frame_T *fr)
{
if (fr != NULL) {
clear_snapshot_rec(fr->fr_next);
clear_snapshot_rec(fr->fr_child);
xfree(fr);
}
}
void 
restore_snapshot (
int idx,
int close_curwin 
)
{
win_T *wp;
if (curtab->tp_snapshot[idx] != NULL
&& curtab->tp_snapshot[idx]->fr_width == topframe->fr_width
&& curtab->tp_snapshot[idx]->fr_height == topframe->fr_height
&& check_snapshot_rec(curtab->tp_snapshot[idx], topframe) == OK) {
wp = restore_snapshot_rec(curtab->tp_snapshot[idx], topframe);
win_comp_pos();
if (wp != NULL && close_curwin)
win_goto(wp);
redraw_all_later(NOT_VALID);
}
clear_snapshot(curtab, idx);
}
static int check_snapshot_rec(frame_T *sn, frame_T *fr)
{
if (sn->fr_layout != fr->fr_layout
|| (sn->fr_next == NULL) != (fr->fr_next == NULL)
|| (sn->fr_child == NULL) != (fr->fr_child == NULL)
|| (sn->fr_next != NULL
&& check_snapshot_rec(sn->fr_next, fr->fr_next) == FAIL)
|| (sn->fr_child != NULL
&& check_snapshot_rec(sn->fr_child, fr->fr_child) == FAIL)
|| (sn->fr_win != NULL && !win_valid(sn->fr_win)))
return FAIL;
return OK;
}
static win_T *restore_snapshot_rec(frame_T *sn, frame_T *fr)
{
win_T *wp = NULL;
win_T *wp2;
fr->fr_height = sn->fr_height;
fr->fr_width = sn->fr_width;
if (fr->fr_layout == FR_LEAF) {
frame_new_height(fr, fr->fr_height, FALSE, FALSE);
frame_new_width(fr, fr->fr_width, FALSE, FALSE);
wp = sn->fr_win;
}
if (sn->fr_next != NULL) {
wp2 = restore_snapshot_rec(sn->fr_next, fr->fr_next);
if (wp2 != NULL)
wp = wp2;
}
if (sn->fr_child != NULL) {
wp2 = restore_snapshot_rec(sn->fr_child, fr->fr_child);
if (wp2 != NULL)
wp = wp2;
}
return wp;
}
static win_T *get_snapshot_focus(int idx)
{
if (curtab->tp_snapshot[idx] == NULL) {
return NULL;
}
frame_T *sn = curtab->tp_snapshot[idx];
while (sn->fr_child != NULL || sn->fr_next != NULL) {
while (sn->fr_child != NULL) {
sn = sn->fr_child;
}
if (sn->fr_next != NULL) {
sn = sn->fr_next;
}
}
return win_valid(sn->fr_win) ? sn->fr_win : NULL;
}
int switch_win(win_T **save_curwin, tabpage_T **save_curtab, win_T *win, tabpage_T *tp, int no_display)
{
block_autocmds();
*save_curwin = curwin;
if (tp != NULL) {
*save_curtab = curtab;
if (no_display) {
curtab->tp_firstwin = firstwin;
curtab->tp_lastwin = lastwin;
curtab = tp;
firstwin = curtab->tp_firstwin;
lastwin = curtab->tp_lastwin;
} else
goto_tabpage_tp(tp, FALSE, FALSE);
}
if (!win_valid(win)) {
return FAIL;
}
curwin = win;
curbuf = curwin->w_buffer;
return OK;
}
void restore_win(win_T *save_curwin, tabpage_T *save_curtab, bool no_display)
{
if (save_curtab != NULL && valid_tabpage(save_curtab)) {
if (no_display) {
curtab->tp_firstwin = firstwin;
curtab->tp_lastwin = lastwin;
curtab = save_curtab;
firstwin = curtab->tp_firstwin;
lastwin = curtab->tp_lastwin;
} else
goto_tabpage_tp(save_curtab, FALSE, FALSE);
}
if (win_valid(save_curwin)) {
curwin = save_curwin;
curbuf = curwin->w_buffer;
}
unblock_autocmds();
}
void switch_buffer(bufref_T *save_curbuf, buf_T *buf)
{
block_autocmds();
set_bufref(save_curbuf, curbuf);
curbuf->b_nwindows--;
curbuf = buf;
curwin->w_buffer = buf;
curbuf->b_nwindows++;
}
void restore_buffer(bufref_T *save_curbuf)
{
unblock_autocmds();
if (bufref_valid(save_curbuf)) {
curbuf->b_nwindows--;
curwin->w_buffer = save_curbuf->br_buf;
curbuf = save_curbuf->br_buf;
curbuf->b_nwindows++;
}
}
int match_add(win_T *wp, const char *const grp, const char *const pat,
int prio, int id, list_T *pos_list,
const char *const conceal_char)
{
matchitem_T *cur;
matchitem_T *prev;
matchitem_T *m;
int hlg_id;
regprog_T *regprog = NULL;
int rtype = SOME_VALID;
if (*grp == NUL || (pat != NULL && *pat == NUL)) {
return -1;
}
if (id < -1 || id == 0) {
EMSGN(_("E799: Invalid ID: %" PRId64
" (must be greater than or equal to 1)"),
id);
return -1;
}
if (id != -1) {
cur = wp->w_match_head;
while (cur != NULL) {
if (cur->id == id) {
EMSGN(_("E801: ID already taken: %" PRId64), id);
return -1;
}
cur = cur->next;
}
}
if ((hlg_id = syn_name2id((const char_u *)grp)) == 0) {
EMSG2(_(e_nogroup), grp);
return -1;
}
if (pat != NULL && (regprog = vim_regcomp((char_u *)pat, RE_MAGIC)) == NULL) {
EMSG2(_(e_invarg2), pat);
return -1;
}
while (id == -1) {
cur = wp->w_match_head;
while (cur != NULL && cur->id != wp->w_next_match_id)
cur = cur->next;
if (cur == NULL)
id = wp->w_next_match_id;
wp->w_next_match_id++;
}
m = xcalloc(1, sizeof(matchitem_T));
m->id = id;
m->priority = prio;
m->pattern = pat == NULL ? NULL: (char_u *)xstrdup(pat);
m->hlg_id = hlg_id;
m->match.regprog = regprog;
m->match.rmm_ic = FALSE;
m->match.rmm_maxcol = 0;
m->conceal_char = 0;
if (conceal_char != NULL) {
m->conceal_char = utf_ptr2char((const char_u *)conceal_char);
}
if (pos_list != NULL) {
linenr_T toplnum = 0;
linenr_T botlnum = 0;
int i = 0;
TV_LIST_ITER(pos_list, li, {
linenr_T lnum = 0;
colnr_T col = 0;
int len = 1;
bool error = false;
if (TV_LIST_ITEM_TV(li)->v_type == VAR_LIST) {
const list_T *const subl = TV_LIST_ITEM_TV(li)->vval.v_list;
const listitem_T *subli = tv_list_first(subl);
if (subli == NULL) {
emsgf(_("E5030: Empty list at position %d"),
(int)tv_list_idx_of_item(pos_list, li));
goto fail;
}
lnum = tv_get_number_chk(TV_LIST_ITEM_TV(subli), &error);
if (error) {
goto fail;
}
if (lnum <= 0) {
continue;
}
m->pos.pos[i].lnum = lnum;
subli = TV_LIST_ITEM_NEXT(subl, subli);
if (subli != NULL) {
col = tv_get_number_chk(TV_LIST_ITEM_TV(subli), &error);
if (error) {
goto fail;
}
if (col < 0) {
continue;
}
subli = TV_LIST_ITEM_NEXT(subl, subli);
if (subli != NULL) {
len = tv_get_number_chk(TV_LIST_ITEM_TV(subli), &error);
if (len < 0) {
continue;
}
if (error) {
goto fail;
}
}
}
m->pos.pos[i].col = col;
m->pos.pos[i].len = len;
} else if (TV_LIST_ITEM_TV(li)->v_type == VAR_NUMBER) {
if (TV_LIST_ITEM_TV(li)->vval.v_number <= 0) {
continue;
}
m->pos.pos[i].lnum = TV_LIST_ITEM_TV(li)->vval.v_number;
m->pos.pos[i].col = 0;
m->pos.pos[i].len = 0;
} else {
emsgf(_("E5031: List or number required at position %d"),
(int)tv_list_idx_of_item(pos_list, li));
goto fail;
}
if (toplnum == 0 || lnum < toplnum) {
toplnum = lnum;
}
if (botlnum == 0 || lnum >= botlnum) {
botlnum = lnum + 1;
}
i++;
if (i >= MAXPOSMATCH) {
break;
}
});
if (toplnum != 0){
if (wp->w_buffer->b_mod_set) {
if (wp->w_buffer->b_mod_top > toplnum) {
wp->w_buffer->b_mod_top = toplnum;
}
if (wp->w_buffer->b_mod_bot < botlnum) {
wp->w_buffer->b_mod_bot = botlnum;
}
} else {
wp->w_buffer->b_mod_set = true;
wp->w_buffer->b_mod_top = toplnum;
wp->w_buffer->b_mod_bot = botlnum;
wp->w_buffer->b_mod_xlines = 0;
}
m->pos.toplnum = toplnum;
m->pos.botlnum = botlnum;
rtype = VALID;
}
}
cur = wp->w_match_head;
prev = cur;
while (cur != NULL && prio >= cur->priority) {
prev = cur;
cur = cur->next;
}
if (cur == prev)
wp->w_match_head = m;
else
prev->next = m;
m->next = cur;
redraw_win_later(wp, rtype);
return id;
fail:
xfree(m);
return -1;
}
int match_delete(win_T *wp, int id, int perr)
{
matchitem_T *cur = wp->w_match_head;
matchitem_T *prev = cur;
int rtype = SOME_VALID;
if (id < 1) {
if (perr) {
EMSGN(_("E802: Invalid ID: %" PRId64
" (must be greater than or equal to 1)"),
id);
}
return -1;
}
while (cur != NULL && cur->id != id) {
prev = cur;
cur = cur->next;
}
if (cur == NULL) {
if (perr) {
EMSGN(_("E803: ID not found: %" PRId64), id);
}
return -1;
}
if (cur == prev)
wp->w_match_head = cur->next;
else
prev->next = cur->next;
vim_regfree(cur->match.regprog);
xfree(cur->pattern);
if (cur->pos.toplnum != 0) {
if (wp->w_buffer->b_mod_set) {
if (wp->w_buffer->b_mod_top > cur->pos.toplnum) {
wp->w_buffer->b_mod_top = cur->pos.toplnum;
}
if (wp->w_buffer->b_mod_bot < cur->pos.botlnum) {
wp->w_buffer->b_mod_bot = cur->pos.botlnum;
}
} else {
wp->w_buffer->b_mod_set = true;
wp->w_buffer->b_mod_top = cur->pos.toplnum;
wp->w_buffer->b_mod_bot = cur->pos.botlnum;
wp->w_buffer->b_mod_xlines = 0;
}
rtype = VALID;
}
xfree(cur);
redraw_win_later(wp, rtype);
return 0;
}
void clear_matches(win_T *wp)
{
matchitem_T *m;
while (wp->w_match_head != NULL) {
m = wp->w_match_head->next;
vim_regfree(wp->w_match_head->match.regprog);
xfree(wp->w_match_head->pattern);
xfree(wp->w_match_head);
wp->w_match_head = m;
}
redraw_win_later(wp, SOME_VALID);
}
matchitem_T *get_match(win_T *wp, int id)
{
matchitem_T *cur = wp->w_match_head;
while (cur != NULL && cur->id != id)
cur = cur->next;
return cur;
}
static bool frame_check_height(const frame_T *topfrp, int height)
FUNC_ATTR_PURE FUNC_ATTR_WARN_UNUSED_RESULT FUNC_ATTR_NONNULL_ALL
{
if (topfrp->fr_height != height) {
return false;
}
if (topfrp->fr_layout == FR_ROW) {
const frame_T *frp;
FOR_ALL_FRAMES(frp, topfrp->fr_child) {
if (frp->fr_height != height) {
return false;
}
}
}
return true;
}
static bool frame_check_width(const frame_T *topfrp, int width)
FUNC_ATTR_PURE FUNC_ATTR_WARN_UNUSED_RESULT FUNC_ATTR_NONNULL_ALL
{
if (topfrp->fr_width != width) {
return false;
}
if (topfrp->fr_layout == FR_COL) {
const frame_T *frp;
FOR_ALL_FRAMES(frp, topfrp->fr_child) {
if (frp->fr_width != width) {
return false;
}
}
}
return true;
}
int win_getid(typval_T *argvars)
{
if (argvars[0].v_type == VAR_UNKNOWN) {
return curwin->handle;
}
int winnr = tv_get_number(&argvars[0]);
win_T *wp;
if (winnr > 0) {
if (argvars[1].v_type == VAR_UNKNOWN) {
wp = firstwin;
} else {
tabpage_T *tp = NULL;
int tabnr = tv_get_number(&argvars[1]);
FOR_ALL_TABS(tp2) {
if (--tabnr == 0) {
tp = tp2;
break;
}
}
if (tp == NULL) {
return -1;
}
if (tp == curtab) {
wp = firstwin;
} else {
wp = tp->tp_firstwin;
}
}
for ( ; wp != NULL; wp = wp->w_next) {
if (--winnr == 0) {
return wp->handle;
}
}
}
return 0;
}
int win_gotoid(typval_T *argvars)
{
int id = tv_get_number(&argvars[0]);
FOR_ALL_TAB_WINDOWS(tp, wp) {
if (wp->handle == id) {
goto_tabpage_win(tp, wp);
return 1;
}
}
return 0;
}
void win_get_tabwin(handle_T id, int *tabnr, int *winnr)
{
*tabnr = 0;
*winnr = 0;
int tnum = 1, wnum = 1;
FOR_ALL_TABS(tp) {
FOR_ALL_WINDOWS_IN_TAB(wp, tp) {
if (wp->handle == id) {
*winnr = wnum;
*tabnr = tnum;
return;
}
wnum++;
}
tnum++;
wnum = 1;
}
}
void win_id2tabwin(typval_T *const argvars, typval_T *const rettv)
{
int winnr = 1;
int tabnr = 1;
handle_T id = (handle_T)tv_get_number(&argvars[0]);
win_get_tabwin(id, &tabnr, &winnr);
list_T *const list = tv_list_alloc_ret(rettv, 2);
tv_list_append_number(list, tabnr);
tv_list_append_number(list, winnr);
}
win_T * win_id2wp(typval_T *argvars)
{
int id = tv_get_number(&argvars[0]);
FOR_ALL_TAB_WINDOWS(tp, wp) {
if (wp->handle == id) {
return wp;
}
}
return NULL;
}
int win_id2win(typval_T *argvars)
{
int nr = 1;
int id = tv_get_number(&argvars[0]);
FOR_ALL_WINDOWS_IN_TAB(wp, curtab) {
if (wp->handle == id) {
return nr;
}
nr++;
}
return 0;
}
void win_findbuf(typval_T *argvars, list_T *list)
{
int bufnr = tv_get_number(&argvars[0]);
FOR_ALL_TAB_WINDOWS(tp, wp) {
if (wp->w_buffer->b_fnum == bufnr) {
tv_list_append_number(list, wp->handle);
}
}
}
void get_framelayout(const frame_T *fr, list_T *l, bool outer)
{
list_T *fr_list;
if (fr == NULL) {
return;
}
if (outer) {
fr_list = l;
} else {
fr_list = tv_list_alloc(2);
tv_list_append_list(l, fr_list);
}
if (fr->fr_layout == FR_LEAF) {
if (fr->fr_win != NULL) {
tv_list_append_string(fr_list, "leaf", -1);
tv_list_append_number(fr_list, fr->fr_win->handle);
}
} else {
tv_list_append_string(fr_list, fr->fr_layout == FR_ROW ? "row" : "col", -1);
list_T *const win_list = tv_list_alloc(kListLenUnknown);
tv_list_append_list(fr_list, win_list);
const frame_T *child = fr->fr_child;
while (child != NULL) {
get_framelayout(child, win_list, false);
child = child->fr_next;
}
}
}
void win_ui_flush(void)
{
FOR_ALL_TAB_WINDOWS(tp, wp) {
if (wp->w_pos_changed && wp->w_grid.chars != NULL) {
if (tp == curtab) {
ui_ext_win_position(wp);
} else {
ui_call_win_hide(wp->w_grid.handle);
}
wp->w_pos_changed = false;
}
if (tp == curtab) {
ui_ext_win_viewport(wp);
}
}
}
win_T *lastwin_nofloating(void) {
win_T *res = lastwin;
while (res->w_floating) {
res = res->w_prev;
}
return res;
}
