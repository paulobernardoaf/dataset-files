#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <string.h>
#include "nvim/vim.h"
#include "nvim/ascii.h"
#include "nvim/ops.h"
#include "nvim/buffer.h"
#include "nvim/change.h"
#include "nvim/charset.h"
#include "nvim/cursor.h"
#include "nvim/assert.h"
#include "nvim/edit.h"
#include "nvim/eval.h"
#include "nvim/eval/typval.h"
#include "nvim/ex_cmds.h"
#include "nvim/ex_cmds2.h"
#include "nvim/ex_getln.h"
#include "nvim/fileio.h"
#include "nvim/fold.h"
#include "nvim/getchar.h"
#include "nvim/indent.h"
#include "nvim/log.h"
#include "nvim/mark.h"
#include "nvim/extmark.h"
#include "nvim/mbyte.h"
#include "nvim/memline.h"
#include "nvim/memory.h"
#include "nvim/message.h"
#include "nvim/misc1.h"
#include "nvim/move.h"
#include "nvim/normal.h"
#include "nvim/option.h"
#include "nvim/path.h"
#include "nvim/screen.h"
#include "nvim/search.h"
#include "nvim/state.h"
#include "nvim/strings.h"
#include "nvim/terminal.h"
#include "nvim/ui.h"
#include "nvim/undo.h"
#include "nvim/macros.h"
#include "nvim/window.h"
#include "nvim/lib/kvec.h"
#include "nvim/os/input.h"
#include "nvim/os/time.h"
static yankreg_T y_regs[NUM_REGISTERS];
static yankreg_T *y_previous = NULL; 
static int batch_change_count = 0; 
static bool clipboard_delay_update = false; 
static bool clipboard_needs_update = false; 
static bool clipboard_didwarn = false;
struct block_def {
int startspaces; 
int endspaces; 
int textlen; 
char_u *textstart; 
colnr_T textcol; 
colnr_T start_vcol; 
colnr_T end_vcol; 
int is_short; 
int is_MAX; 
int is_oneChar; 
int pre_whitesp; 
int pre_whitesp_c; 
colnr_T end_char_vcols; 
colnr_T start_char_vcols; 
};
#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "ops.c.generated.h"
#endif
#define OPF_LINES 1 
#define OPF_CHANGE 2 
static char opchars[][3] =
{
{ NUL, NUL, 0 }, 
{ 'd', NUL, OPF_CHANGE }, 
{ 'y', NUL, 0 }, 
{ 'c', NUL, OPF_CHANGE }, 
{ '<', NUL, OPF_LINES | OPF_CHANGE }, 
{ '>', NUL, OPF_LINES | OPF_CHANGE }, 
{ '!', NUL, OPF_LINES | OPF_CHANGE }, 
{ 'g', '~', OPF_CHANGE }, 
{ '=', NUL, OPF_LINES | OPF_CHANGE }, 
{ 'g', 'q', OPF_LINES | OPF_CHANGE }, 
{ ':', NUL, OPF_LINES }, 
{ 'g', 'U', OPF_CHANGE }, 
{ 'g', 'u', OPF_CHANGE }, 
{ 'J', NUL, OPF_LINES | OPF_CHANGE }, 
{ 'g', 'J', OPF_LINES | OPF_CHANGE }, 
{ 'g', '?', OPF_CHANGE }, 
{ 'r', NUL, OPF_CHANGE }, 
{ 'I', NUL, OPF_CHANGE }, 
{ 'A', NUL, OPF_CHANGE }, 
{ 'z', 'f', OPF_LINES }, 
{ 'z', 'o', OPF_LINES }, 
{ 'z', 'O', OPF_LINES }, 
{ 'z', 'c', OPF_LINES }, 
{ 'z', 'C', OPF_LINES }, 
{ 'z', 'd', OPF_LINES }, 
{ 'z', 'D', OPF_LINES }, 
{ 'g', 'w', OPF_LINES | OPF_CHANGE }, 
{ 'g', '@', OPF_CHANGE }, 
{ Ctrl_A, NUL, OPF_CHANGE }, 
{ Ctrl_X, NUL, OPF_CHANGE }, 
};
int get_op_type(int char1, int char2)
{
int i;
if (char1 == 'r') {
return OP_REPLACE;
}
if (char1 == '~') {
return OP_TILDE;
}
if (char1 == 'g' && char2 == Ctrl_A) {
return OP_NR_ADD;
}
if (char1 == 'g' && char2 == Ctrl_X) {
return OP_NR_SUB;
}
for (i = 0;; i++) {
if (opchars[i][0] == char1 && opchars[i][1] == char2) {
break;
}
if (i == (int)(ARRAY_SIZE(opchars) - 1)) {
internal_error("get_op_type()");
break;
}
}
return i;
}
int op_on_lines(int op)
{
return opchars[op][2] & OPF_LINES;
}
int op_is_change(int op)
{
return opchars[op][2] & OPF_CHANGE;
}
int get_op_char(int optype)
{
return opchars[optype][0];
}
int get_extra_op_char(int optype)
{
return opchars[optype][1];
}
void op_shift(oparg_T *oap, int curs_top, int amount)
{
long i;
int first_char;
char_u *s;
int block_col = 0;
if (u_save((linenr_T)(oap->start.lnum - 1),
(linenr_T)(oap->end.lnum + 1)) == FAIL)
return;
if (oap->motion_type == kMTBlockWise) {
block_col = curwin->w_cursor.col;
}
for (i = oap->line_count - 1; i >= 0; i--) {
first_char = *get_cursor_line_ptr();
if (first_char == NUL) { 
curwin->w_cursor.col = 0;
} else if (oap->motion_type == kMTBlockWise) {
shift_block(oap, amount);
} else if (first_char != '#' || !preprocs_left()) {
shift_line(oap->op_type == OP_LSHIFT, p_sr, amount, false);
}
++curwin->w_cursor.lnum;
}
if (oap->motion_type == kMTBlockWise) {
curwin->w_cursor.lnum = oap->start.lnum;
curwin->w_cursor.col = block_col;
} else if (curs_top) { 
curwin->w_cursor.lnum = oap->start.lnum;
beginline(BL_SOL | BL_FIX); 
} else
--curwin->w_cursor.lnum; 
foldOpenCursor();
if (oap->line_count > p_report) {
if (oap->op_type == OP_RSHIFT)
s = (char_u *)">";
else
s = (char_u *)"<";
if (oap->line_count == 1) {
if (amount == 1)
sprintf((char *)IObuff, _("1 line %sed 1 time"), s);
else
sprintf((char *)IObuff, _("1 line %sed %d times"), s, amount);
} else {
if (amount == 1)
sprintf((char *)IObuff, _("%" PRId64 " lines %sed 1 time"),
(int64_t)oap->line_count, s);
else
sprintf((char *)IObuff, _("%" PRId64 " lines %sed %d times"),
(int64_t)oap->line_count, s, amount);
}
msg(IObuff);
}
curbuf->b_op_start = oap->start;
curbuf->b_op_end.lnum = oap->end.lnum;
curbuf->b_op_end.col = (colnr_T)STRLEN(ml_get(oap->end.lnum));
if (curbuf->b_op_end.col > 0) {
curbuf->b_op_end.col--;
}
changed_lines(oap->start.lnum, 0, oap->end.lnum + 1, 0L, true);
}
void shift_line(
int left,
int round,
int amount,
int call_changed_bytes 
)
{
int count;
int i, j;
int p_sw = get_sw_value(curbuf);
count = get_indent(); 
if (round) { 
i = count / p_sw; 
j = count % p_sw; 
if (j && left) { 
amount--;
}
if (left) {
i -= amount;
if (i < 0)
i = 0;
} else
i += amount;
count = i * p_sw;
} else { 
if (left) {
count -= p_sw * amount;
if (count < 0)
count = 0;
} else
count += p_sw * amount;
}
if (State & VREPLACE_FLAG) {
change_indent(INDENT_SET, count, false, NUL, call_changed_bytes);
} else {
(void)set_indent(count, call_changed_bytes ? SIN_CHANGED : 0);
}
}
static void shift_block(oparg_T *oap, int amount)
{
const bool left = (oap->op_type == OP_LSHIFT);
const int oldstate = State;
char_u *newp;
const int oldcol = curwin->w_cursor.col;
const int p_sw = get_sw_value(curbuf);
const int p_ts = (int)curbuf->b_p_ts;
struct block_def bd;
int incr;
int i = 0, j = 0;
const int old_p_ri = p_ri;
p_ri = 0; 
State = INSERT; 
block_prep(oap, &bd, curwin->w_cursor.lnum, true);
if (bd.is_short) {
return;
}
int total = (int)((unsigned)amount * (unsigned)p_sw);
if ((total / p_sw) != amount) {
return; 
}
char_u *const oldp = get_cursor_line_ptr();
int startcol, oldlen, newlen;
if (!left) {
total += bd.pre_whitesp; 
colnr_T ws_vcol = bd.start_vcol - bd.pre_whitesp;
char_u * old_textstart = bd.textstart;
if (bd.startspaces) {
if (utfc_ptr2len(bd.textstart) == 1) {
bd.textstart++;
} else {
ws_vcol = 0;
bd.startspaces = 0;
}
}
for (; ascii_iswhite(*bd.textstart); ) {
incr = lbr_chartabsize_adv(bd.textstart, &bd.textstart, (colnr_T)(bd.start_vcol));
total += incr;
bd.start_vcol += incr;
}
if (!curbuf->b_p_et)
i = ((ws_vcol % p_ts) + total) / p_ts; 
if (i)
j = ((ws_vcol % p_ts) + total) % p_ts; 
else
j = total;
int col_pre = bd.pre_whitesp_c - (bd.startspaces != 0);
bd.textcol -= col_pre;
const int len = (int)STRLEN(bd.textstart) + 1;
int col = bd.textcol + i +j + len;
assert(col >= 0);
newp = (char_u *)xmalloc((size_t)col);
memset(newp, NUL, (size_t)col);
memmove(newp, oldp, (size_t)bd.textcol);
startcol = bd.textcol;
oldlen = (int)(bd.textstart-old_textstart) + col_pre;
newlen = i+j;
memset(newp + bd.textcol, TAB, (size_t)i);
memset(newp + bd.textcol + i, ' ', (size_t)j);
memmove(newp + bd.textcol + i + j, bd.textstart, (size_t)len);
} else { 
colnr_T destination_col; 
char_u *verbatim_copy_end; 
colnr_T verbatim_copy_width; 
size_t fill; 
size_t new_line_len; 
char_u *non_white = bd.textstart;
if (bd.startspaces) {
MB_PTR_ADV(non_white);
}
colnr_T non_white_col = bd.start_vcol;
while (ascii_iswhite(*non_white)) {
incr = lbr_chartabsize_adv(bd.textstart, &non_white, non_white_col);
non_white_col += incr;
}
const colnr_T block_space_width = non_white_col - oap->start_vcol;
const colnr_T shift_amount = block_space_width < total
? block_space_width
: total;
destination_col = non_white_col - shift_amount;
verbatim_copy_end = bd.textstart;
verbatim_copy_width = bd.start_vcol;
if (bd.startspaces)
verbatim_copy_width -= bd.start_char_vcols;
while (verbatim_copy_width < destination_col) {
char_u *line = verbatim_copy_end;
incr = lbr_chartabsize(line, verbatim_copy_end, verbatim_copy_width);
if (verbatim_copy_width + incr > destination_col)
break;
verbatim_copy_width += incr;
MB_PTR_ADV(verbatim_copy_end);
}
assert(destination_col - verbatim_copy_width >= 0);
fill = (size_t)(destination_col - verbatim_copy_width);
assert(verbatim_copy_end - oldp >= 0);
const size_t verbatim_diff = (size_t)(verbatim_copy_end - oldp);
new_line_len = verbatim_diff + fill + STRLEN(non_white) + 1;
newp = (char_u *)xmalloc(new_line_len);
startcol = (int)verbatim_diff;
oldlen = bd.textcol + (int)(non_white - bd.textstart) - (int)verbatim_diff;
newlen = (int)fill;
memmove(newp, oldp, verbatim_diff);
memset(newp + verbatim_diff, ' ', fill);
STRMOVE(newp + verbatim_diff + fill, non_white);
}
ml_replace(curwin->w_cursor.lnum, newp, false);
changed_bytes(curwin->w_cursor.lnum, (colnr_T)bd.textcol);
extmark_splice(curbuf, (int)curwin->w_cursor.lnum-1, startcol,
0, oldlen, 0, newlen,
kExtmarkUndo);
State = oldstate;
curwin->w_cursor.col = oldcol;
p_ri = old_p_ri;
}
static void block_insert(oparg_T *oap, char_u *s, int b_insert, struct block_def *bdp)
{
int p_ts;
int count = 0; 
int spaces = 0; 
colnr_T offset; 
size_t s_len = STRLEN(s);
char_u *newp, *oldp; 
linenr_T lnum; 
int oldstate = State;
State = INSERT; 
for (lnum = oap->start.lnum + 1; lnum <= oap->end.lnum; lnum++) {
block_prep(oap, bdp, lnum, true);
if (bdp->is_short && b_insert) {
continue; 
}
oldp = ml_get(lnum);
if (b_insert) {
p_ts = bdp->start_char_vcols;
spaces = bdp->startspaces;
if (spaces != 0)
count = p_ts - 1; 
offset = bdp->textcol;
} else { 
p_ts = bdp->end_char_vcols;
if (!bdp->is_short) { 
spaces = (bdp->endspaces ? p_ts - bdp->endspaces : 0);
if (spaces != 0)
count = p_ts - 1; 
offset = bdp->textcol + bdp->textlen - (spaces != 0);
} else { 
if (!bdp->is_MAX)
spaces = (oap->end_vcol - bdp->end_vcol) + 1;
count = spaces;
offset = bdp->textcol + bdp->textlen;
}
}
if (spaces > 0) {
int off;
if (b_insert) {
off = utf_head_off(oldp, oldp + offset + spaces);
} else {
off = (*mb_off_next)(oldp, oldp + offset);
offset += off;
}
spaces -= off;
count -= off;
}
assert(count >= 0);
newp = (char_u *)xmalloc(STRLEN(oldp) + s_len + (size_t)count + 1);
memmove(newp, oldp, (size_t)offset);
oldp += offset;
int startcol = offset;
memset(newp + offset, ' ', (size_t)spaces);
memmove(newp + offset + spaces, s, s_len);
offset += (int)s_len;
int skipped = 0;
if (spaces && !bdp->is_short) {
memset(newp + offset + spaces, ' ', (size_t)(p_ts - spaces));
oldp++;
count++;
skipped = 1;
}
if (spaces > 0)
offset += count;
STRMOVE(newp + offset, oldp);
ml_replace(lnum, newp, false);
extmark_splice(curbuf, (int)lnum-1, startcol,
0, skipped,
0, offset-startcol, kExtmarkUndo);
if (lnum == oap->end.lnum) {
curbuf->b_op_end.lnum = oap->end.lnum;
curbuf->b_op_end.col = offset;
}
} 
changed_lines(oap->start.lnum + 1, 0, oap->end.lnum + 1, 0L, true);
State = oldstate;
}
void op_reindent(oparg_T *oap, Indenter how)
{
long i;
char_u *l;
int amount;
linenr_T first_changed = 0;
linenr_T last_changed = 0;
linenr_T start_lnum = curwin->w_cursor.lnum;
if (!MODIFIABLE(curbuf)) {
EMSG(_(e_modifiable));
return;
}
for (i = oap->line_count - 1; i >= 0 && !got_int; i--) {
if (i > 1
&& (i % 50 == 0 || i == oap->line_count - 1)
&& oap->line_count > p_report)
smsg(_("%" PRId64 " lines to indent... "), (int64_t)i);
if (i != oap->line_count - 1 || oap->line_count == 1
|| how != get_lisp_indent) {
l = skipwhite(get_cursor_line_ptr());
if (*l == NUL) 
amount = 0;
else
amount = how(); 
if (amount >= 0 && set_indent(amount, SIN_UNDO)) {
if (first_changed == 0) {
first_changed = curwin->w_cursor.lnum;
}
last_changed = curwin->w_cursor.lnum;
}
}
++curwin->w_cursor.lnum;
curwin->w_cursor.col = 0; 
}
curwin->w_cursor.lnum = start_lnum;
beginline(BL_SOL | BL_FIX);
if (last_changed != 0) {
changed_lines(first_changed, 0,
oap->is_VIsual ? start_lnum + oap->line_count :
last_changed + 1, 0L, true);
} else if (oap->is_VIsual) {
redraw_curbuf_later(INVERTED);
}
if (oap->line_count > p_report) {
i = oap->line_count - (i + 1);
if (i == 1)
MSG(_("1 line indented "));
else
smsg(_("%" PRId64 " lines indented "), (int64_t)i);
}
curbuf->b_op_start = oap->start;
curbuf->b_op_end = oap->end;
}
static char_u *expr_line = NULL;
int get_expr_register(void)
{
char_u *new_line;
new_line = getcmdline('=', 0L, 0, true);
if (new_line == NULL) {
return NUL;
}
if (*new_line == NUL) { 
xfree(new_line);
} else {
set_expr_line(new_line);
}
return '=';
}
void set_expr_line(char_u *new_line)
{
xfree(expr_line);
expr_line = new_line;
}
char_u *get_expr_line(void)
{
char_u *expr_copy;
char_u *rv;
static int nested = 0;
if (expr_line == NULL)
return NULL;
expr_copy = vim_strsave(expr_line);
if (nested >= 10)
return expr_copy;
++nested;
rv = eval_to_string(expr_copy, NULL, TRUE);
--nested;
xfree(expr_copy);
return rv;
}
char_u *get_expr_line_src(void)
{
if (expr_line == NULL)
return NULL;
return vim_strsave(expr_line);
}
bool valid_yank_reg(int regname, bool writing)
{
if ((regname > 0 && ASCII_ISALNUM(regname))
|| (!writing && vim_strchr((char_u *) "/.%:=" , regname) != NULL)
|| regname == '#'
|| regname == '"'
|| regname == '-'
|| regname == '_'
|| regname == '*'
|| regname == '+') {
return true;
}
return false;
}
typedef enum {
YREG_PASTE,
YREG_YANK,
YREG_PUT,
} yreg_mode_t;
yankreg_T *get_yank_register(int regname, int mode)
{
yankreg_T *reg;
if (mode == YREG_PASTE && get_clipboard(regname, &reg, false)) {
return reg;
} else if (mode != YREG_YANK
&& (regname == 0 || regname == '"' || regname == '*' || regname == '+')
&& y_previous != NULL) {
return y_previous;
}
int i = op_reg_index(regname);
if (i == -1) {
i = 0;
}
reg = &y_regs[i];
if (mode == YREG_YANK) {
y_previous = reg;
}
return reg;
}
static bool is_append_register(int regname)
{
return ASCII_ISUPPER(regname);
}
static inline bool is_literal_register(int regname)
FUNC_ATTR_CONST
{
return regname == '*' || regname == '+';
}
yankreg_T *copy_register(int name)
FUNC_ATTR_NONNULL_RET
{
yankreg_T *reg = get_yank_register(name, YREG_PASTE);
yankreg_T *copy = xmalloc(sizeof(yankreg_T));
*copy = *reg;
if (copy->y_size == 0) {
copy->y_array = NULL;
} else {
copy->y_array = xcalloc(copy->y_size, sizeof(char_u *));
for (size_t i = 0; i < copy->y_size; i++) {
copy->y_array[i] = vim_strsave(reg->y_array[i]);
}
}
return copy;
}
bool yank_register_mline(int regname)
{
if (regname != 0 && !valid_yank_reg(regname, false)) {
return false;
}
if (regname == '_') { 
return false;
}
yankreg_T *reg = get_yank_register(regname, YREG_PASTE);
return reg->y_type == kMTLineWise;
}
int do_record(int c)
{
char_u *p;
static int regname;
yankreg_T *old_y_previous;
int retval;
if (reg_recording == 0) {
if (c < 0 || (!ASCII_ISALNUM(c) && c != '"')) {
retval = FAIL;
} else {
reg_recording = c;
showmode();
regname = c;
retval = OK;
}
} else { 
reg_recording = 0;
if (ui_has(kUIMessages)) {
showmode();
} else {
MSG("");
}
p = get_recorded();
if (p == NULL)
retval = FAIL;
else {
vim_unescape_csi(p);
old_y_previous = y_previous;
retval = stuff_yank(regname, p);
y_previous = old_y_previous;
}
}
return retval;
}
static void set_yreg_additional_data(yankreg_T *reg, dict_T *additional_data)
FUNC_ATTR_NONNULL_ARG(1)
{
if (reg->additional_data == additional_data) {
return;
}
tv_dict_unref(reg->additional_data);
reg->additional_data = additional_data;
}
static int stuff_yank(int regname, char_u *p)
{
if (regname != 0 && !valid_yank_reg(regname, true)) {
xfree(p);
return FAIL;
}
if (regname == '_') { 
xfree(p);
return OK;
}
yankreg_T *reg = get_yank_register(regname, YREG_YANK);
if (is_append_register(regname) && reg->y_array != NULL) {
char_u **pp = &(reg->y_array[reg->y_size - 1]);
char_u *lp = xmalloc(STRLEN(*pp) + STRLEN(p) + 1);
STRCPY(lp, *pp);
STRCAT(lp, p);
xfree(p);
xfree(*pp);
*pp = lp;
} else {
free_register(reg);
set_yreg_additional_data(reg, NULL);
reg->y_array = (char_u **)xmalloc(sizeof(char_u *));
reg->y_array[0] = p;
reg->y_size = 1;
reg->y_type = kMTCharWise;
}
reg->timestamp = os_time();
return OK;
}
static int execreg_lastc = NUL;
int
do_execreg(
int regname,
int colon, 
int addcr, 
int silent 
)
{
char_u *p;
int retval = OK;
if (regname == '@') { 
if (execreg_lastc == NUL) {
EMSG(_("E748: No previously used register"));
return FAIL;
}
regname = execreg_lastc;
}
if (regname == '%' || regname == '#' || !valid_yank_reg(regname, false)) {
emsg_invreg(regname);
return FAIL;
}
execreg_lastc = regname;
if (regname == '_') 
return OK;
if (regname == ':') { 
if (last_cmdline == NULL) {
EMSG(_(e_nolastcmd));
return FAIL;
}
XFREE_CLEAR(new_last_cmdline);
p = vim_strsave_escaped_ext(
last_cmdline,
(char_u *)"\001\002\003\004\005\006\007"
"\010\011\012\013\014\015\016\017"
"\020\021\022\023\024\025\026\027"
"\030\031\032\033\034\035\036\037",
Ctrl_V, false);
if (VIsual_active && STRNCMP(p, "'<,'>", 5) == 0) {
retval = put_in_typebuf(p + 5, true, true, silent);
} else {
retval = put_in_typebuf(p, true, true, silent);
}
xfree(p);
} else if (regname == '=') {
p = get_expr_line();
if (p == NULL)
return FAIL;
retval = put_in_typebuf(p, true, colon, silent);
xfree(p);
} else if (regname == '.') { 
p = get_last_insert_save();
if (p == NULL) {
EMSG(_(e_noinstext));
return FAIL;
}
retval = put_in_typebuf(p, false, colon, silent);
xfree(p);
} else {
yankreg_T *reg = get_yank_register(regname, YREG_PASTE);
if (reg->y_array == NULL)
return FAIL;
int remap = colon ? REMAP_NONE : REMAP_YES;
put_reedit_in_typebuf(silent);
char_u *escaped;
for (size_t i = reg->y_size; i-- > 0;) { 
if (reg->y_type == kMTLineWise || i < reg->y_size - 1 || addcr) {
if (ins_typebuf((char_u *)"\n", remap, 0, true, silent) == FAIL) {
return FAIL;
}
}
escaped = vim_strsave_escape_csi(reg->y_array[i]);
retval = ins_typebuf(escaped, remap, 0, TRUE, silent);
xfree(escaped);
if (retval == FAIL)
return FAIL;
if (colon && ins_typebuf((char_u *)":", remap, 0, TRUE, silent)
== FAIL)
return FAIL;
}
reg_executing = regname == 0 ? '"' : regname; 
}
return retval;
}
static void put_reedit_in_typebuf(int silent)
{
char_u buf[3];
if (restart_edit != NUL) {
if (restart_edit == 'V') {
buf[0] = 'g';
buf[1] = 'R';
buf[2] = NUL;
} else {
buf[0] = (char_u)(restart_edit == 'I' ? 'i' : restart_edit);
buf[1] = NUL;
}
if (ins_typebuf(buf, REMAP_NONE, 0, TRUE, silent) == OK)
restart_edit = NUL;
}
}
static int put_in_typebuf(
char_u *s,
bool esc,
bool colon, 
int silent
)
{
int retval = OK;
put_reedit_in_typebuf(silent);
if (colon)
retval = ins_typebuf((char_u *)"\n", REMAP_NONE, 0, TRUE, silent);
if (retval == OK) {
char_u *p;
if (esc)
p = vim_strsave_escape_csi(s);
else
p = s;
if (p == NULL)
retval = FAIL;
else
retval = ins_typebuf(p, esc ? REMAP_NONE : REMAP_YES,
0, TRUE, silent);
if (esc)
xfree(p);
}
if (colon && retval == OK)
retval = ins_typebuf((char_u *)":", REMAP_NONE, 0, TRUE, silent);
return retval;
}
int insert_reg(
int regname,
bool literally_arg 
)
{
int retval = OK;
bool allocated;
const bool literally = literally_arg || is_literal_register(regname);
os_breakcheck();
if (got_int)
return FAIL;
if (regname != NUL && !valid_yank_reg(regname, false))
return FAIL;
char_u *arg;
if (regname == '.') { 
retval = stuff_inserted(NUL, 1L, true);
} else if (get_spec_reg(regname, &arg, &allocated, true)) {
if (arg == NULL) {
return FAIL;
}
stuffescaped((const char *)arg, literally);
if (allocated) {
xfree(arg);
}
} else { 
yankreg_T *reg = get_yank_register(regname, YREG_PASTE);
if (reg->y_array == NULL) {
retval = FAIL;
} else {
for (size_t i = 0; i < reg->y_size; i++) {
stuffescaped((const char *)reg->y_array[i], literally);
if (reg->y_type == kMTLineWise || i < reg->y_size - 1) {
stuffcharReadbuff('\n');
}
}
}
}
return retval;
}
static void stuffescaped(const char *arg, int literally)
{
while (*arg != NUL) {
const char *const start = arg;
while ((*arg >= ' ' && *arg < DEL) || ((uint8_t)(*arg) == K_SPECIAL
&& !literally)) {
arg++;
}
if (arg > start) {
stuffReadbuffLen(start, (long)(arg - start));
}
if (*arg != NUL) {
const int c = mb_cptr2char_adv((const char_u **)&arg);
if (literally && ((c < ' ' && c != TAB) || c == DEL)) {
stuffcharReadbuff(Ctrl_V);
}
stuffcharReadbuff(c);
}
}
}
bool get_spec_reg(
int regname,
char_u **argp,
bool *allocated, 
bool errmsg 
)
{
size_t cnt;
*argp = NULL;
*allocated = false;
switch (regname) {
case '%': 
if (errmsg)
check_fname(); 
*argp = curbuf->b_fname;
return true;
case '#': 
*argp = getaltfname(errmsg); 
return true;
case '=': 
*argp = get_expr_line();
*allocated = true;
return true;
case ':': 
if (last_cmdline == NULL && errmsg)
EMSG(_(e_nolastcmd));
*argp = last_cmdline;
return true;
case '/': 
if (last_search_pat() == NULL && errmsg)
EMSG(_(e_noprevre));
*argp = last_search_pat();
return true;
case '.': 
*argp = get_last_insert_save();
*allocated = true;
if (*argp == NULL && errmsg) {
EMSG(_(e_noinstext));
}
return true;
case Ctrl_F: 
case Ctrl_P: 
if (!errmsg) {
return false;
}
*argp = file_name_at_cursor(
FNAME_MESS | FNAME_HYP | (regname == Ctrl_P ? FNAME_EXP : 0),
1L, NULL);
*allocated = true;
return true;
case Ctrl_W: 
case Ctrl_A: 
if (!errmsg) {
return false;
}
cnt = find_ident_under_cursor(argp, (regname == Ctrl_W
? (FIND_IDENT|FIND_STRING)
: FIND_STRING));
*argp = cnt ? vim_strnsave(*argp, cnt) : NULL;
*allocated = true;
return true;
case Ctrl_L: 
if (!errmsg) {
return false;
}
*argp = ml_get_buf(curwin->w_buffer, curwin->w_cursor.lnum, false);
return true;
case '_': 
*argp = (char_u *)"";
return true;
}
return false;
}
bool cmdline_paste_reg(int regname, bool literally_arg, bool remcr)
{
const bool literally = literally_arg || is_literal_register(regname);
yankreg_T *reg = get_yank_register(regname, YREG_PASTE);
if (reg->y_array == NULL)
return FAIL;
for (size_t i = 0; i < reg->y_size; i++) {
cmdline_paste_str(reg->y_array[i], literally);
if (i < reg->y_size - 1 && !remcr) {
cmdline_paste_str((char_u *)"\r", literally);
}
os_breakcheck();
if (got_int)
return FAIL;
}
return OK;
}
static void shift_delete_registers(bool y_append)
{
free_register(&y_regs[9]); 
for (int n = 9; n > 1; n--) {
y_regs[n] = y_regs[n - 1];
}
if (!y_append) {
y_previous = &y_regs[1];
}
y_regs[1].y_array = NULL; 
}
int op_delete(oparg_T *oap)
{
int n;
linenr_T lnum;
char_u *ptr;
char_u *newp, *oldp;
struct block_def bd = { 0 };
linenr_T old_lcount = curbuf->b_ml.ml_line_count;
if (curbuf->b_ml.ml_flags & ML_EMPTY) { 
return OK;
}
if (oap->empty) {
return u_save_cursor();
}
if (!MODIFIABLE(curbuf)) {
EMSG(_(e_modifiable));
return FAIL;
}
mb_adjust_opend(oap);
if (oap->motion_type == kMTCharWise
&& !oap->is_VIsual
&& oap->line_count > 1
&& oap->motion_force == NUL
&& oap->op_type == OP_DELETE) {
ptr = ml_get(oap->end.lnum) + oap->end.col;
if (*ptr != NUL)
ptr += oap->inclusive;
ptr = skipwhite(ptr);
if (*ptr == NUL && inindent(0)) {
oap->motion_type = kMTLineWise;
}
}
if (oap->motion_type != kMTLineWise
&& oap->line_count == 1
&& oap->op_type == OP_DELETE
&& *ml_get(oap->start.lnum) == NUL) {
if (virtual_op) {
goto setmarks;
}
if (vim_strchr(p_cpo, CPO_EMPTYREGION) != NULL) {
beep_flush();
}
return OK;
}
if (oap->regname != '_') {
yankreg_T *reg = NULL;
int did_yank = false;
if (oap->regname != 0) {
did_yank = op_yank(oap, false, true);
if (!did_yank) {
return OK;
}
}
if (oap->regname != 0 || oap->motion_type == kMTLineWise
|| oap->line_count > 1 || oap->use_reg_one) {
shift_delete_registers(is_append_register(oap->regname));
reg = &y_regs[1];
op_yank_reg(oap, false, reg, false);
did_yank = true;
}
if (oap->regname == 0 && oap->motion_type != kMTLineWise
&& oap->line_count == 1) {
reg = get_yank_register('-', YREG_YANK);
op_yank_reg(oap, false, reg, false);
did_yank = true;
}
if (did_yank || oap->regname == 0) {
if (reg == NULL) {
abort();
}
set_clipboard(oap->regname, reg);
do_autocmd_textyankpost(oap, reg);
}
}
if (oap->motion_type == kMTBlockWise) {
if (u_save((linenr_T)(oap->start.lnum - 1),
(linenr_T)(oap->end.lnum + 1)) == FAIL) {
return FAIL;
}
for (lnum = curwin->w_cursor.lnum; lnum <= oap->end.lnum; lnum++) {
block_prep(oap, &bd, lnum, true);
if (bd.textlen == 0) { 
continue;
}
if (lnum == curwin->w_cursor.lnum) {
curwin->w_cursor.col = bd.textcol + bd.startspaces;
curwin->w_cursor.coladd = 0;
}
n = bd.textlen - bd.startspaces - bd.endspaces;
oldp = ml_get(lnum);
newp = (char_u *)xmalloc(STRLEN(oldp) - (size_t)n + 1);
memmove(newp, oldp, (size_t)bd.textcol);
memset(newp + bd.textcol, ' ', (size_t)bd.startspaces +
(size_t)bd.endspaces);
oldp += bd.textcol + bd.textlen;
STRMOVE(newp + bd.textcol + bd.startspaces + bd.endspaces, oldp);
ml_replace(lnum, newp, false);
extmark_splice(curbuf, (int)lnum-1, bd.textcol,
0, bd.textlen,
0, bd.startspaces+bd.endspaces,
kExtmarkUndo);
}
check_cursor_col();
changed_lines(curwin->w_cursor.lnum, curwin->w_cursor.col,
oap->end.lnum + 1, 0L, true);
oap->line_count = 0; 
} else if (oap->motion_type == kMTLineWise) {
if (oap->op_type == OP_CHANGE) {
if (oap->line_count > 1) {
lnum = curwin->w_cursor.lnum;
++curwin->w_cursor.lnum;
del_lines(oap->line_count - 1, TRUE);
curwin->w_cursor.lnum = lnum;
}
if (u_save_cursor() == FAIL)
return FAIL;
if (curbuf->b_p_ai) { 
beginline(BL_WHITE); 
did_ai = true; 
ai_col = curwin->w_cursor.col;
} else
beginline(0); 
truncate_line(FALSE); 
if (oap->line_count > 1)
u_clearline(); 
} else {
del_lines(oap->line_count, TRUE);
beginline(BL_WHITE | BL_FIX);
u_clearline(); 
}
} else {
if (virtual_op) {
int endcol = 0;
if (gchar_pos(&oap->start) == '\t') {
if (u_save_cursor() == FAIL) 
return FAIL;
if (oap->line_count == 1)
endcol = getviscol2(oap->end.col, oap->end.coladd);
coladvance_force(getviscol2(oap->start.col, oap->start.coladd));
oap->start = curwin->w_cursor;
if (oap->line_count == 1) {
coladvance(endcol);
oap->end.col = curwin->w_cursor.col;
oap->end.coladd = curwin->w_cursor.coladd;
curwin->w_cursor = oap->start;
}
}
if (gchar_pos(&oap->end) == '\t'
&& oap->end.coladd == 0
&& oap->inclusive) {
if (u_save((linenr_T)(oap->end.lnum - 1),
(linenr_T)(oap->end.lnum + 1)) == FAIL)
return FAIL;
curwin->w_cursor = oap->end;
coladvance_force(getviscol2(oap->end.col, oap->end.coladd));
oap->end = curwin->w_cursor;
curwin->w_cursor = oap->start;
}
mb_adjust_opend(oap);
}
if (oap->line_count == 1) { 
if (u_save_cursor() == FAIL) 
return FAIL;
if ( vim_strchr(p_cpo, CPO_DOLLAR) != NULL
&& oap->op_type == OP_CHANGE
&& oap->end.lnum == curwin->w_cursor.lnum
&& !oap->is_VIsual
)
display_dollar(oap->end.col - !oap->inclusive);
n = oap->end.col - oap->start.col + 1 - !oap->inclusive;
if (virtual_op) {
char_u *curline = get_cursor_line_ptr();
int len = (int)STRLEN(curline);
if (oap->end.coladd != 0
&& (int)oap->end.col >= len - 1
&& !(oap->start.coladd && (int)oap->end.col >= len - 1))
n++;
if (n == 0 && oap->start.coladd != oap->end.coladd)
n = 1;
if (gchar_cursor() != NUL)
curwin->w_cursor.coladd = 0;
}
(void)del_bytes((colnr_T)n, !virtual_op,
oap->op_type == OP_DELETE && !oap->is_VIsual);
} else {
pos_T curpos;
if (u_save((linenr_T)(curwin->w_cursor.lnum - 1),
(linenr_T)(curwin->w_cursor.lnum + oap->line_count)) == FAIL)
return FAIL;
curbuf_splice_pending++;
pos_T startpos = curwin->w_cursor; 
truncate_line(true); 
curpos = curwin->w_cursor; 
curwin->w_cursor.lnum++;
del_lines(oap->line_count - 2, false);
n = (oap->end.col + 1 - !oap->inclusive);
curwin->w_cursor.col = 0;
(void)del_bytes((colnr_T)n, !virtual_op,
oap->op_type == OP_DELETE && !oap->is_VIsual);
curwin->w_cursor = curpos; 
(void)do_join(2, false, false, false, false);
curbuf_splice_pending--;
extmark_splice(curbuf, (int)startpos.lnum-1, startpos.col,
(int)oap->line_count-1, n, 0, 0, kExtmarkUndo);
}
}
msgmore(curbuf->b_ml.ml_line_count - old_lcount);
setmarks:
if (oap->motion_type == kMTBlockWise) {
curbuf->b_op_end.lnum = oap->end.lnum;
curbuf->b_op_end.col = oap->start.col;
} else {
curbuf->b_op_end = oap->start;
}
curbuf->b_op_start = oap->start;
return OK;
}
static void mb_adjust_opend(oparg_T *oap)
{
char_u *p;
if (oap->inclusive) {
p = ml_get(oap->end.lnum);
oap->end.col += mb_tail_off(p, p + oap->end.col);
}
}
static inline void pbyte(pos_T lp, int c)
{
assert(c <= UCHAR_MAX);
*(ml_get_buf(curbuf, lp.lnum, true) + lp.col) = (char_u)c;
if (!curbuf_splice_pending) {
extmark_splice(curbuf, (int)lp.lnum-1, lp.col, 0, 1, 0, 1, kExtmarkUndo);
}
}
static void replace_character(int c)
{
const int n = State;
State = REPLACE;
ins_char(c);
State = n;
dec_cursor();
}
int op_replace(oparg_T *oap, int c)
{
int n, numc;
int num_chars;
char_u *newp, *oldp;
colnr_T oldlen;
struct block_def bd;
char_u *after_p = NULL;
int had_ctrl_v_cr = false;
if ((curbuf->b_ml.ml_flags & ML_EMPTY ) || oap->empty)
return OK; 
if (c == REPLACE_CR_NCHAR) {
had_ctrl_v_cr = true;
c = CAR;
} else if (c == REPLACE_NL_NCHAR) {
had_ctrl_v_cr = true;
c = NL;
}
mb_adjust_opend(oap);
if (u_save((linenr_T)(oap->start.lnum - 1),
(linenr_T)(oap->end.lnum + 1)) == FAIL)
return FAIL;
if (oap->motion_type == kMTBlockWise) {
bd.is_MAX = (curwin->w_curswant == MAXCOL);
for (; curwin->w_cursor.lnum <= oap->end.lnum; curwin->w_cursor.lnum++) {
curwin->w_cursor.col = 0; 
block_prep(oap, &bd, curwin->w_cursor.lnum, true);
if (bd.textlen == 0 && (!virtual_op || bd.is_MAX)) {
continue; 
}
if (virtual_op && bd.is_short && *bd.textstart == NUL) {
pos_T vpos;
vpos.lnum = curwin->w_cursor.lnum;
getvpos(&vpos, oap->start_vcol);
bd.startspaces += vpos.coladd;
n = bd.startspaces;
} else
n = (bd.startspaces ? bd.start_char_vcols - 1 : 0);
n += (bd.endspaces
&& !bd.is_oneChar
&& bd.end_char_vcols > 0) ? bd.end_char_vcols - 1 : 0;
numc = oap->end_vcol - oap->start_vcol + 1;
if (bd.is_short && (!virtual_op || bd.is_MAX))
numc -= (oap->end_vcol - bd.end_vcol) + 1;
if ((*mb_char2cells)(c) > 1) {
if ((numc & 1) && !bd.is_short) {
++bd.endspaces;
++n;
}
numc = numc / 2;
}
num_chars = numc;
numc *= (*mb_char2len)(c);
oldp = get_cursor_line_ptr();
oldlen = (int)STRLEN(oldp);
size_t newp_size = (size_t)bd.textcol + (size_t)bd.startspaces;
if (had_ctrl_v_cr || (c != '\r' && c != '\n')) {
newp_size += (size_t)numc;
if (!bd.is_short) {
newp_size += (size_t)(bd.endspaces + oldlen
- bd.textcol - bd.textlen);
}
}
newp = xmallocz(newp_size);
memmove(newp, oldp, (size_t)bd.textcol);
oldp += bd.textcol + bd.textlen;
memset(newp + bd.textcol, ' ', (size_t)bd.startspaces);
size_t after_p_len = 0;
int col = oldlen - bd.textcol - bd.textlen + 1;
assert(col >= 0);
int newrows = 0, newcols = 0;
if (had_ctrl_v_cr || (c != '\r' && c != '\n')) {
int newp_len = bd.textcol + bd.startspaces;
while (--num_chars >= 0) {
newp_len += utf_char2bytes(c, newp + newp_len);
}
if (!bd.is_short) {
memset(newp + newp_len, ' ', (size_t)bd.endspaces);
newp_len += bd.endspaces;
memmove(newp + newp_len, oldp, (size_t)col);
}
newcols = newp_len - bd.textcol;
} else {
after_p_len = (size_t)col;
after_p = (char_u *)xmalloc(after_p_len);
memmove(after_p, oldp, after_p_len);
newrows = 1;
}
ml_replace(curwin->w_cursor.lnum, newp, false);
linenr_T baselnum = curwin->w_cursor.lnum;
if (after_p != NULL) {
ml_append(curwin->w_cursor.lnum++, after_p, (int)after_p_len, false);
appended_lines_mark(curwin->w_cursor.lnum, 1L);
oap->end.lnum++;
xfree(after_p);
}
extmark_splice(curbuf, (int)baselnum-1, bd.textcol,
0, bd.textlen,
newrows, newcols, kExtmarkUndo);
}
} else {
if (oap->motion_type == kMTLineWise) {
oap->start.col = 0;
curwin->w_cursor.col = 0;
oap->end.col = (colnr_T)STRLEN(ml_get(oap->end.lnum));
if (oap->end.col)
--oap->end.col;
} else if (!oap->inclusive)
dec(&(oap->end));
while (ltoreq(curwin->w_cursor, oap->end)) {
n = gchar_cursor();
if (n != NUL) {
if ((*mb_char2len)(c) > 1 || (*mb_char2len)(n) > 1) {
if (curwin->w_cursor.lnum == oap->end.lnum)
oap->end.col += (*mb_char2len)(c) - (*mb_char2len)(n);
replace_character(c);
} else {
if (n == TAB) {
int end_vcol = 0;
if (curwin->w_cursor.lnum == oap->end.lnum) {
end_vcol = getviscol2(oap->end.col,
oap->end.coladd);
}
coladvance_force(getviscol());
if (curwin->w_cursor.lnum == oap->end.lnum)
getvpos(&oap->end, end_vcol);
}
pbyte(curwin->w_cursor, c);
}
} else if (virtual_op && curwin->w_cursor.lnum == oap->end.lnum) {
int virtcols = oap->end.coladd;
if (curwin->w_cursor.lnum == oap->start.lnum
&& oap->start.col == oap->end.col && oap->start.coladd)
virtcols -= oap->start.coladd;
coladvance_force(getviscol2(oap->end.col, oap->end.coladd) + 1);
curwin->w_cursor.col -= (virtcols + 1);
for (; virtcols >= 0; virtcols--) {
if (utf_char2len(c) > 1) {
replace_character(c);
} else {
pbyte(curwin->w_cursor, c);
}
if (inc(&curwin->w_cursor) == -1) {
break;
}
}
}
if (inc_cursor() == -1)
break;
}
}
curwin->w_cursor = oap->start;
check_cursor();
changed_lines(oap->start.lnum, oap->start.col, oap->end.lnum + 1, 0L, true);
curbuf->b_op_start = oap->start;
curbuf->b_op_end = oap->end;
return OK;
}
void op_tilde(oparg_T *oap)
{
pos_T pos;
struct block_def bd;
int did_change = FALSE;
if (u_save((linenr_T)(oap->start.lnum - 1),
(linenr_T)(oap->end.lnum + 1)) == FAIL)
return;
pos = oap->start;
if (oap->motion_type == kMTBlockWise) { 
for (; pos.lnum <= oap->end.lnum; pos.lnum++) {
int one_change;
block_prep(oap, &bd, pos.lnum, false);
pos.col = bd.textcol;
one_change = swapchars(oap->op_type, &pos, bd.textlen);
did_change |= one_change;
}
if (did_change) {
changed_lines(oap->start.lnum, 0, oap->end.lnum + 1, 0L, true);
}
} else { 
if (oap->motion_type == kMTLineWise) {
oap->start.col = 0;
pos.col = 0;
oap->end.col = (colnr_T)STRLEN(ml_get(oap->end.lnum));
if (oap->end.col)
--oap->end.col;
} else if (!oap->inclusive)
dec(&(oap->end));
if (pos.lnum == oap->end.lnum)
did_change = swapchars(oap->op_type, &pos,
oap->end.col - pos.col + 1);
else
for (;; ) {
did_change |= swapchars(oap->op_type, &pos,
pos.lnum == oap->end.lnum ? oap->end.col + 1 :
(int)STRLEN(ml_get_pos(&pos)));
if (ltoreq(oap->end, pos) || inc(&pos) == -1)
break;
}
if (did_change) {
changed_lines(oap->start.lnum, oap->start.col, oap->end.lnum + 1,
0L, true);
}
}
if (!did_change && oap->is_VIsual)
redraw_curbuf_later(INVERTED);
curbuf->b_op_start = oap->start;
curbuf->b_op_end = oap->end;
if (oap->line_count > p_report) {
if (oap->line_count == 1)
MSG(_("1 line changed"));
else
smsg(_("%" PRId64 " lines changed"), (int64_t)oap->line_count);
}
}
static int swapchars(int op_type, pos_T *pos, int length)
FUNC_ATTR_NONNULL_ALL
{
int did_change = 0;
for (int todo = length; todo > 0; todo--) {
const int len = utfc_ptr2len(ml_get_pos(pos));
if (len > 0) {
todo -= len - 1;
}
did_change |= swapchar(op_type, pos);
if (inc(pos) == -1) 
break;
}
return did_change;
}
bool swapchar(int op_type, pos_T *pos)
FUNC_ATTR_NONNULL_ARG(2)
{
const int c = gchar_pos(pos);
if (c >= 0x80 && op_type == OP_ROT13) {
return false;
}
if (op_type == OP_UPPER && c == 0xdf) {
pos_T sp = curwin->w_cursor;
curwin->w_cursor = *pos;
del_char(false);
ins_char('S');
ins_char('S');
curwin->w_cursor = sp;
inc(pos);
}
int nc = c;
if (mb_islower(c)) {
if (op_type == OP_ROT13) {
nc = ROT13(c, 'a');
} else if (op_type != OP_LOWER) {
nc = mb_toupper(c);
}
} else if (mb_isupper(c)) {
if (op_type == OP_ROT13) {
nc = ROT13(c, 'A');
} else if (op_type != OP_UPPER) {
nc = mb_tolower(c);
}
}
if (nc != c) {
if (c >= 0x80 || nc >= 0x80) {
pos_T sp = curwin->w_cursor;
curwin->w_cursor = *pos;
del_bytes(utf_ptr2len(get_cursor_pos_ptr()), false, false);
ins_char(nc);
curwin->w_cursor = sp;
} else {
pbyte(*pos, nc);
}
return true;
}
return false;
}
void op_insert(oparg_T *oap, long count1)
{
long ins_len, pre_textlen = 0;
char_u *firstline, *ins_text;
colnr_T ind_pre = 0;
struct block_def bd;
int i;
pos_T t1;
bd.is_MAX = (curwin->w_curswant == MAXCOL);
curwin->w_cursor.lnum = oap->start.lnum;
update_screen(INVERTED);
if (oap->motion_type == kMTBlockWise) {
if (curwin->w_cursor.coladd > 0) {
unsigned old_ve_flags = ve_flags;
ve_flags = VE_ALL;
if (u_save_cursor() == FAIL)
return;
coladvance_force(oap->op_type == OP_APPEND
? oap->end_vcol + 1 : getviscol());
if (oap->op_type == OP_APPEND)
--curwin->w_cursor.col;
ve_flags = old_ve_flags;
}
block_prep(oap, &bd, oap->start.lnum, true);
ind_pre = (colnr_T)getwhitecols_curline();
firstline = ml_get(oap->start.lnum) + bd.textcol;
if (oap->op_type == OP_APPEND) {
firstline += bd.textlen;
}
pre_textlen = (long)STRLEN(firstline);
}
if (oap->op_type == OP_APPEND) {
if (oap->motion_type == kMTBlockWise
&& curwin->w_cursor.coladd == 0
) {
curwin->w_set_curswant = TRUE;
while (*get_cursor_pos_ptr() != NUL
&& (curwin->w_cursor.col < bd.textcol + bd.textlen))
++curwin->w_cursor.col;
if (bd.is_short && !bd.is_MAX) {
if (u_save_cursor() == FAIL)
return;
for (i = 0; i < bd.endspaces; i++) {
ins_char(' ');
}
bd.textlen += bd.endspaces;
}
} else {
curwin->w_cursor = oap->end;
check_cursor_col();
if (!LINEEMPTY(curwin->w_cursor.lnum)
&& oap->start_vcol != oap->end_vcol) {
inc_cursor();
}
}
}
t1 = oap->start;
(void)edit(NUL, false, (linenr_T)count1);
if (t1.lnum == curbuf->b_op_start_orig.lnum
&& lt(curbuf->b_op_start_orig, t1)) {
oap->start = curbuf->b_op_start_orig;
}
if (curwin->w_cursor.lnum != oap->start.lnum || got_int)
return;
if (oap->motion_type == kMTBlockWise) {
struct block_def bd2;
bool did_indent = false;
const colnr_T ind_post = (colnr_T)getwhitecols_curline();
if (curbuf->b_op_start.col > ind_pre && ind_post > ind_pre) {
bd.textcol += ind_post - ind_pre;
bd.start_vcol += ind_post - ind_pre;
did_indent = true;
}
if (oap->start.lnum == curbuf->b_op_start_orig.lnum
&& !bd.is_MAX
&& !did_indent) {
if (oap->op_type == OP_INSERT
&& oap->start.col + oap->start.coladd
!= curbuf->b_op_start_orig.col + curbuf->b_op_start_orig.coladd) {
int t = getviscol2(curbuf->b_op_start_orig.col,
curbuf->b_op_start_orig.coladd);
oap->start.col = curbuf->b_op_start_orig.col;
pre_textlen -= t - oap->start_vcol;
oap->start_vcol = t;
} else if (oap->op_type == OP_APPEND
&& oap->end.col + oap->end.coladd
>= curbuf->b_op_start_orig.col
+ curbuf->b_op_start_orig.coladd) {
int t = getviscol2(curbuf->b_op_start_orig.col,
curbuf->b_op_start_orig.coladd);
oap->start.col = curbuf->b_op_start_orig.col;
pre_textlen += bd.textlen;
pre_textlen -= t - oap->start_vcol;
oap->start_vcol = t;
oap->op_type = OP_INSERT;
}
}
block_prep(oap, &bd2, oap->start.lnum, true);
if (!bd.is_MAX || bd2.textlen < bd.textlen) {
if (oap->op_type == OP_APPEND) {
pre_textlen += bd2.textlen - bd.textlen;
if (bd2.endspaces)
--bd2.textlen;
}
bd.textcol = bd2.textcol;
bd.textlen = bd2.textlen;
}
firstline = ml_get(oap->start.lnum);
const size_t len = STRLEN(firstline);
colnr_T add = bd.textcol;
if (oap->op_type == OP_APPEND) {
add += bd.textlen;
}
if ((size_t)add > len) {
firstline += len; 
} else {
firstline += add;
}
ins_len = (long)STRLEN(firstline) - pre_textlen;
if (pre_textlen >= 0 && ins_len > 0) {
ins_text = vim_strnsave(firstline, (size_t)ins_len);
if (u_save(oap->start.lnum, (linenr_T)(oap->end.lnum + 1)) == OK) {
block_insert(oap, ins_text, (oap->op_type == OP_INSERT), &bd);
}
curwin->w_cursor.col = oap->start.col;
check_cursor();
xfree(ins_text);
}
}
}
int op_change(oparg_T *oap)
{
colnr_T l;
int retval;
long offset;
linenr_T linenr;
long ins_len;
long pre_textlen = 0;
long pre_indent = 0;
char_u *newp;
char_u *firstline;
char_u *ins_text;
char_u *oldp;
struct block_def bd;
l = oap->start.col;
if (oap->motion_type == kMTLineWise) {
l = 0;
if (!p_paste && curbuf->b_p_si
&& !curbuf->b_p_cin
)
can_si = true; 
}
if (curbuf->b_ml.ml_flags & ML_EMPTY) {
if (u_save_cursor() == FAIL)
return FALSE;
} else if (op_delete(oap) == FAIL)
return FALSE;
if ((l > curwin->w_cursor.col) && !LINEEMPTY(curwin->w_cursor.lnum)
&& !virtual_op) {
inc_cursor();
}
if (oap->motion_type == kMTBlockWise) {
if (virtual_op && (curwin->w_cursor.coladd > 0
|| gchar_cursor() == NUL)) {
coladvance_force(getviscol());
}
firstline = ml_get(oap->start.lnum);
pre_textlen = (long)STRLEN(firstline);
pre_indent = (long)getwhitecols(firstline);
bd.textcol = curwin->w_cursor.col;
}
if (oap->motion_type == kMTLineWise) {
fix_indent();
}
retval = edit(NUL, FALSE, (linenr_T)1);
if (oap->motion_type == kMTBlockWise
&& oap->start.lnum != oap->end.lnum && !got_int) {
firstline = ml_get(oap->start.lnum);
if (bd.textcol > (colnr_T)pre_indent) {
long new_indent = (long)getwhitecols(firstline);
pre_textlen += new_indent - pre_indent;
bd.textcol += (colnr_T)(new_indent - pre_indent);
}
ins_len = (long)STRLEN(firstline) - pre_textlen;
if (ins_len > 0) {
ins_text = (char_u *)xmalloc((size_t)(ins_len + 1));
STRLCPY(ins_text, firstline + bd.textcol, ins_len + 1);
for (linenr = oap->start.lnum + 1; linenr <= oap->end.lnum;
linenr++) {
block_prep(oap, &bd, linenr, true);
if (!bd.is_short || virtual_op) {
pos_T vpos;
if (bd.is_short) {
vpos.lnum = linenr;
(void)getvpos(&vpos, oap->start_vcol);
} else {
vpos.coladd = 0;
}
oldp = ml_get(linenr);
newp = xmalloc(STRLEN(oldp) + (size_t)vpos.coladd
+ (size_t)ins_len + 1);
memmove(newp, oldp, (size_t)bd.textcol);
offset = bd.textcol;
memset(newp + offset, ' ', (size_t)vpos.coladd);
offset += vpos.coladd;
memmove(newp + offset, ins_text, (size_t)ins_len);
offset += ins_len;
oldp += bd.textcol;
STRMOVE(newp + offset, oldp);
ml_replace(linenr, newp, false);
extmark_splice(curbuf, (int)linenr-1, bd.textcol,
0, 0,
0, vpos.coladd+(int)ins_len, kExtmarkUndo);
}
}
check_cursor();
changed_lines(oap->start.lnum + 1, 0, oap->end.lnum + 1, 0L, true);
xfree(ins_text);
}
}
return retval;
}
void init_yank(void)
{
memset(&(y_regs[0]), 0, sizeof(y_regs));
}
#if defined(EXITFREE)
void clear_registers(void)
{
int i;
for (i = 0; i < NUM_REGISTERS; i++) {
free_register(&y_regs[i]);
}
}
#endif
void free_register(yankreg_T *reg)
FUNC_ATTR_NONNULL_ALL
{
set_yreg_additional_data(reg, NULL);
if (reg->y_array != NULL) {
for (size_t i = reg->y_size; i-- > 0;) { 
xfree(reg->y_array[i]);
}
XFREE_CLEAR(reg->y_array);
}
}
bool op_yank(oparg_T *oap, bool message, int deleting)
FUNC_ATTR_NONNULL_ALL
{
if (oap->regname != 0 && !valid_yank_reg(oap->regname, true)) {
beep_flush();
return false;
}
if (oap->regname == '_') {
return true; 
}
yankreg_T *reg = get_yank_register(oap->regname, YREG_YANK);
op_yank_reg(oap, message, reg, is_append_register(oap->regname));
if (!deleting) {
set_clipboard(oap->regname, reg);
do_autocmd_textyankpost(oap, reg);
}
return true;
}
static void op_yank_reg(oparg_T *oap, bool message, yankreg_T *reg, bool append)
{
yankreg_T newreg; 
char_u **new_ptr;
linenr_T lnum; 
size_t j;
MotionType yank_type = oap->motion_type;
size_t yanklines = (size_t)oap->line_count;
linenr_T yankendlnum = oap->end.lnum;
char_u *p;
char_u *pnew;
struct block_def bd;
yankreg_T *curr = reg; 
if (append && reg->y_array != NULL) {
reg = &newreg;
} else {
free_register(reg); 
}
if (oap->motion_type == kMTCharWise
&& oap->start.col == 0
&& !oap->inclusive
&& (!oap->is_VIsual || *p_sel == 'o')
&& oap->end.col == 0
&& yanklines > 1) {
yank_type = kMTLineWise;
yankendlnum--;
yanklines--;
}
reg->y_size = yanklines;
reg->y_type = yank_type; 
reg->y_width = 0;
reg->y_array = xcalloc(yanklines, sizeof(char_u *));
reg->additional_data = NULL;
reg->timestamp = os_time();
size_t y_idx = 0; 
lnum = oap->start.lnum;
if (yank_type == kMTBlockWise) {
reg->y_width = oap->end_vcol - oap->start_vcol;
if (curwin->w_curswant == MAXCOL && reg->y_width > 0)
reg->y_width--;
}
for (; lnum <= yankendlnum; lnum++, y_idx++) {
switch (reg->y_type) {
case kMTBlockWise:
block_prep(oap, &bd, lnum, false);
yank_copy_line(reg, &bd, y_idx);
break;
case kMTLineWise:
reg->y_array[y_idx] = vim_strsave(ml_get(lnum));
break;
case kMTCharWise:
{
colnr_T startcol = 0, endcol = MAXCOL;
int is_oneChar = false;
colnr_T cs, ce;
p = ml_get(lnum);
bd.startspaces = 0;
bd.endspaces = 0;
if (lnum == oap->start.lnum) {
startcol = oap->start.col;
if (virtual_op) {
getvcol(curwin, &oap->start, &cs, NULL, &ce);
if (ce != cs && oap->start.coladd > 0) {
bd.startspaces = (ce - cs + 1)
- oap->start.coladd;
startcol++;
}
}
}
if (lnum == oap->end.lnum) {
endcol = oap->end.col;
if (virtual_op) {
getvcol(curwin, &oap->end, &cs, NULL, &ce);
if (p[endcol] == NUL || (cs + oap->end.coladd < ce
&& utf_head_off(p, p + endcol) == 0)) {
if (oap->start.lnum == oap->end.lnum
&& oap->start.col == oap->end.col) {
is_oneChar = true;
bd.startspaces = oap->end.coladd
- oap->start.coladd + oap->inclusive;
endcol = startcol;
} else {
bd.endspaces = oap->end.coladd
+ oap->inclusive;
endcol -= oap->inclusive;
}
}
}
}
if (endcol == MAXCOL)
endcol = (colnr_T)STRLEN(p);
if (startcol > endcol
|| is_oneChar
) {
bd.textlen = 0;
} else {
bd.textlen = endcol - startcol + oap->inclusive;
}
bd.textstart = p + startcol;
yank_copy_line(reg, &bd, y_idx);
break;
}
case kMTUnknown:
assert(false);
}
}
if (curr != reg) { 
new_ptr = xmalloc(sizeof(char_u *) * (curr->y_size + reg->y_size));
for (j = 0; j < curr->y_size; ++j)
new_ptr[j] = curr->y_array[j];
xfree(curr->y_array);
curr->y_array = new_ptr;
if (yank_type == kMTLineWise) {
curr->y_type = kMTLineWise;
}
if (curr->y_type == kMTCharWise
&& vim_strchr(p_cpo, CPO_REGAPPEND) == NULL) {
pnew = xmalloc(STRLEN(curr->y_array[curr->y_size - 1])
+ STRLEN(reg->y_array[0]) + 1);
STRCPY(pnew, curr->y_array[--j]);
STRCAT(pnew, reg->y_array[0]);
xfree(curr->y_array[j]);
xfree(reg->y_array[0]);
curr->y_array[j++] = pnew;
y_idx = 1;
} else
y_idx = 0;
while (y_idx < reg->y_size)
curr->y_array[j++] = reg->y_array[y_idx++];
curr->y_size = j;
xfree(reg->y_array);
}
if (curwin->w_p_rnu) {
redraw_later(SOME_VALID); 
}
if (message) { 
if (yank_type == kMTCharWise && yanklines == 1) {
yanklines = 0;
}
if (yanklines > (size_t)p_report) {
char namebuf[100];
if (oap->regname == NUL) {
*namebuf = NUL;
} else {
vim_snprintf(namebuf, sizeof(namebuf), _(" into \"%c"), oap->regname);
}
update_topline_redraw();
if (yanklines == 1) {
if (yank_type == kMTBlockWise) {
smsg(_("block of 1 line yanked%s"), namebuf);
} else {
smsg(_("1 line yanked%s"), namebuf);
}
} else if (yank_type == kMTBlockWise) {
smsg(_("block of %" PRId64 " lines yanked%s"),
(int64_t)yanklines, namebuf);
} else {
smsg(_("%" PRId64 " lines yanked%s"), (int64_t)yanklines, namebuf);
}
}
}
curbuf->b_op_start = oap->start;
curbuf->b_op_end = oap->end;
if (yank_type == kMTLineWise) {
curbuf->b_op_start.col = 0;
curbuf->b_op_end.col = MAXCOL;
}
return;
}
static void yank_copy_line(yankreg_T *reg, struct block_def *bd, size_t y_idx)
{
int size = bd->startspaces + bd->endspaces + bd->textlen;
assert(size >= 0);
char_u *pnew = xmallocz((size_t)size);
reg->y_array[y_idx] = pnew;
memset(pnew, ' ', (size_t)bd->startspaces);
pnew += bd->startspaces;
memmove(pnew, bd->textstart, (size_t)bd->textlen);
pnew += bd->textlen;
memset(pnew, ' ', (size_t)bd->endspaces);
pnew += bd->endspaces;
*pnew = NUL;
}
static void do_autocmd_textyankpost(oparg_T *oap, yankreg_T *reg)
FUNC_ATTR_NONNULL_ALL
{
static bool recursive = false;
if (recursive || !has_event(EVENT_TEXTYANKPOST)) {
return;
}
recursive = true;
dict_T *dict = get_vim_var_dict(VV_EVENT);
list_T *const list = tv_list_alloc((ptrdiff_t)reg->y_size);
for (size_t i = 0; i < reg->y_size; i++) {
tv_list_append_string(list, (const char *)reg->y_array[i], -1);
}
tv_list_set_lock(list, VAR_FIXED);
tv_dict_add_list(dict, S_LEN("regcontents"), list);
char buf[NUMBUFLEN+2];
format_reg_type(reg->y_type, reg->y_width, buf, ARRAY_SIZE(buf));
tv_dict_add_str(dict, S_LEN("regtype"), buf);
buf[0] = (char)oap->regname;
buf[1] = NUL;
tv_dict_add_str(dict, S_LEN("regname"), buf);
tv_dict_add_special(dict, S_LEN("inclusive"),
oap->inclusive ? kSpecialVarTrue : kSpecialVarFalse);
buf[0] = (char)get_op_char(oap->op_type);
buf[1] = NUL;
tv_dict_add_str(dict, S_LEN("operator"), buf);
tv_dict_set_keys_readonly(dict);
textlock++;
apply_autocmds(EVENT_TEXTYANKPOST, NULL, NULL, false, curbuf);
textlock--;
tv_dict_clear(dict);
recursive = false;
}
void do_put(int regname, yankreg_T *reg, int dir, long count, int flags)
{
char_u *ptr;
char_u *newp;
char_u *oldp;
int yanklen;
size_t totlen = 0; 
linenr_T lnum = 0;
colnr_T col = 0;
size_t i; 
MotionType y_type;
size_t y_size;
size_t oldlen;
int y_width = 0;
colnr_T vcol;
int delcount;
int incr = 0;
struct block_def bd;
char_u **y_array = NULL;
long nr_lines = 0;
pos_T new_cursor;
int indent;
int orig_indent = 0; 
int indent_diff = 0; 
int first_indent = TRUE;
int lendiff = 0;
pos_T old_pos;
char_u *insert_string = NULL;
bool allocated = false;
long cnt;
if (flags & PUT_FIXINDENT)
orig_indent = get_indent();
curbuf->b_op_start = curwin->w_cursor; 
curbuf->b_op_end = curwin->w_cursor; 
if (regname == '.' && !reg) {
bool non_linewise_vis = (VIsual_active && VIsual_mode != 'V');
char command_start_char = non_linewise_vis ? 'c' :
(flags & PUT_LINE ? 'i' : (dir == FORWARD ? 'a' : 'i'));
if (flags & PUT_LINE) {
do_put('_', NULL, dir, 1, PUT_LINE);
}
if (flags & PUT_LINE) {
stuffcharReadbuff(command_start_char);
for (; count > 0; count--) {
(void)stuff_inserted(NUL, 1, count != 1);
if (count != 1) {
stuffReadbuff("\n ");
stuffcharReadbuff(Ctrl_U);
}
}
} else {
(void)stuff_inserted(command_start_char, count, false);
}
if (flags & PUT_CURSEND) {
if (flags & PUT_LINE) {
stuffReadbuff("j0");
} else {
char_u *cursor_pos = get_cursor_pos_ptr();
bool one_past_line = (*cursor_pos == NUL);
bool eol = false;
if (!one_past_line) {
eol = (*(cursor_pos + mb_ptr2len(cursor_pos)) == NUL);
}
bool ve_allows = (ve_flags == VE_ALL || ve_flags == VE_ONEMORE);
bool eof = curbuf->b_ml.ml_line_count == curwin->w_cursor.lnum
&& one_past_line;
if (ve_allows || !(eol || eof)) {
stuffcharReadbuff('l');
}
}
} else if (flags & PUT_LINE) {
stuffReadbuff("g'[");
}
if (command_start_char == 'a') {
if (u_save(curwin->w_cursor.lnum, curwin->w_cursor.lnum + 1) == FAIL) {
return;
}
}
return;
}
if (!reg && get_spec_reg(regname, &insert_string, &allocated, true)) {
if (insert_string == NULL) {
return;
}
}
if (!curbuf->terminal) {
if (u_save(curwin->w_cursor.lnum, curwin->w_cursor.lnum + 1) == FAIL) {
return;
}
}
if (insert_string != NULL) {
y_type = kMTCharWise;
if (regname == '=') {
for (;; ) {
y_size = 0;
ptr = insert_string;
while (ptr != NULL) {
if (y_array != NULL)
y_array[y_size] = ptr;
++y_size;
ptr = vim_strchr(ptr, '\n');
if (ptr != NULL) {
if (y_array != NULL)
*ptr = NUL;
++ptr;
if (*ptr == NUL) {
y_type = kMTLineWise;
break;
}
}
}
if (y_array != NULL)
break;
y_array = (char_u **)xmalloc(y_size * sizeof(char_u *));
}
} else {
y_size = 1; 
y_array = &insert_string;
}
} else {
if (reg == NULL) {
reg = get_yank_register(regname, YREG_PASTE);
}
y_type = reg->y_type;
y_width = reg->y_width;
y_size = reg->y_size;
y_array = reg->y_array;
}
if (curbuf->terminal) {
terminal_paste(count, y_array, y_size);
return;
}
if (y_type == kMTLineWise) {
if (flags & PUT_LINE_SPLIT) {
if (u_save_cursor() == FAIL) {
goto end;
}
char_u *p = get_cursor_pos_ptr();
if (dir == FORWARD && *p != NUL) {
MB_PTR_ADV(p);
}
ptr = vim_strsave(p);
ml_append(curwin->w_cursor.lnum, ptr, (colnr_T)0, false);
xfree(ptr);
oldp = get_cursor_line_ptr();
p = oldp + curwin->w_cursor.col;
if (dir == FORWARD && *p != NUL) {
MB_PTR_ADV(p);
}
ptr = vim_strnsave(oldp, (size_t)(p - oldp));
ml_replace(curwin->w_cursor.lnum, ptr, false);
nr_lines++;
dir = FORWARD;
}
if (flags & PUT_LINE_FORWARD) {
curwin->w_cursor = curbuf->b_visual.vi_end;
dir = FORWARD;
}
curbuf->b_op_start = curwin->w_cursor; 
curbuf->b_op_end = curwin->w_cursor; 
}
if (flags & PUT_LINE) { 
y_type = kMTLineWise;
}
if (y_size == 0 || y_array == NULL) {
EMSG2(_("E353: Nothing in register %s"),
regname == 0 ? (char_u *)"\"" : transchar(regname));
goto end;
}
if (y_type == kMTBlockWise) {
lnum = curwin->w_cursor.lnum + (linenr_T)y_size + 1;
if (lnum > curbuf->b_ml.ml_line_count) {
lnum = curbuf->b_ml.ml_line_count + 1;
}
if (u_save(curwin->w_cursor.lnum - 1, lnum) == FAIL) {
goto end;
}
} else if (y_type == kMTLineWise) {
lnum = curwin->w_cursor.lnum;
if (dir == BACKWARD) {
(void)hasFolding(lnum, &lnum, NULL);
} else {
(void)hasFolding(lnum, NULL, &lnum);
}
if (dir == FORWARD) {
lnum++;
}
if ((BUFEMPTY() ? u_save(0, 2) : u_save(lnum - 1, lnum)) == FAIL) {
goto end;
}
if (dir == FORWARD) {
curwin->w_cursor.lnum = lnum - 1;
} else {
curwin->w_cursor.lnum = lnum;
}
curbuf->b_op_start = curwin->w_cursor; 
} else if (u_save_cursor() == FAIL) {
goto end;
}
yanklen = (int)STRLEN(y_array[0]);
if (ve_flags == VE_ALL && y_type == kMTCharWise) {
if (gchar_cursor() == TAB) {
if (dir == FORWARD
? (int)curwin->w_cursor.coladd < curbuf->b_p_ts - 1
: curwin->w_cursor.coladd > 0)
coladvance_force(getviscol());
else
curwin->w_cursor.coladd = 0;
} else if (curwin->w_cursor.coladd > 0 || gchar_cursor() == NUL)
coladvance_force(getviscol() + (dir == FORWARD));
}
lnum = curwin->w_cursor.lnum;
col = curwin->w_cursor.col;
if (y_type == kMTBlockWise) {
int c = gchar_cursor();
colnr_T endcol2 = 0;
if (dir == FORWARD && c != NUL) {
if (ve_flags == VE_ALL)
getvcol(curwin, &curwin->w_cursor, &col, NULL, &endcol2);
else
getvcol(curwin, &curwin->w_cursor, NULL, NULL, &col);
curwin->w_cursor.col += utfc_ptr2len(get_cursor_pos_ptr());
col++;
} else {
getvcol(curwin, &curwin->w_cursor, &col, NULL, &endcol2);
}
col += curwin->w_cursor.coladd;
if (ve_flags == VE_ALL
&& (curwin->w_cursor.coladd > 0
|| endcol2 == curwin->w_cursor.col)) {
if (dir == FORWARD && c == NUL)
++col;
if (dir != FORWARD && c != NUL)
++curwin->w_cursor.col;
if (c == TAB) {
if (dir == BACKWARD && curwin->w_cursor.col)
curwin->w_cursor.col--;
if (dir == FORWARD && col - 1 == endcol2)
curwin->w_cursor.col++;
}
}
curwin->w_cursor.coladd = 0;
bd.textcol = 0;
for (i = 0; i < y_size; i++) {
int spaces;
char shortline;
bd.startspaces = 0;
bd.endspaces = 0;
vcol = 0;
delcount = 0;
if (curwin->w_cursor.lnum > curbuf->b_ml.ml_line_count) {
if (ml_append(curbuf->b_ml.ml_line_count, (char_u *)"",
(colnr_T)1, false) == FAIL) {
break;
}
nr_lines++;
}
oldp = get_cursor_line_ptr();
oldlen = STRLEN(oldp);
for (ptr = oldp; vcol < col && *ptr; ) {
incr = lbr_chartabsize_adv(oldp, &ptr, (colnr_T)vcol);
vcol += incr;
}
bd.textcol = (colnr_T)(ptr - oldp);
shortline = (vcol < col) || (vcol == col && !*ptr);
if (vcol < col) 
bd.startspaces = col - vcol;
else if (vcol > col) {
bd.endspaces = vcol - col;
bd.startspaces = incr - bd.endspaces;
--bd.textcol;
delcount = 1;
bd.textcol -= utf_head_off(oldp, oldp + bd.textcol);
if (oldp[bd.textcol] != TAB) {
delcount = 0;
bd.endspaces = 0;
}
}
yanklen = (int)STRLEN(y_array[i]);
spaces = y_width + 1;
for (long j = 0; j < yanklen; j++) {
spaces -= lbr_chartabsize(NULL, &y_array[i][j], 0);
}
if (spaces < 0) {
spaces = 0;
}
totlen = (size_t)(count * (yanklen + spaces)
+ bd.startspaces + bd.endspaces);
newp = (char_u *) xmalloc(totlen + oldlen + 1);
ptr = newp;
memmove(ptr, oldp, (size_t)bd.textcol);
ptr += bd.textcol;
memset(ptr, ' ', (size_t)bd.startspaces);
ptr += bd.startspaces;
for (long j = 0; j < count; j++) {
memmove(ptr, y_array[i], (size_t)yanklen);
ptr += yanklen;
if ((j < count - 1 || !shortline) && spaces) {
memset(ptr, ' ', (size_t)spaces);
ptr += spaces;
}
}
memset(ptr, ' ', (size_t)bd.endspaces);
ptr += bd.endspaces;
int columns = (int)oldlen - bd.textcol - delcount + 1;
assert(columns >= 0);
memmove(ptr, oldp + bd.textcol + delcount, (size_t)columns);
ml_replace(curwin->w_cursor.lnum, newp, false);
extmark_splice(curbuf, (int)curwin->w_cursor.lnum-1, bd.textcol,
0, delcount,
0, (int)totlen,
kExtmarkUndo);
++curwin->w_cursor.lnum;
if (i == 0)
curwin->w_cursor.col += bd.startspaces;
}
changed_lines(lnum, 0, curwin->w_cursor.lnum, nr_lines, true);
curbuf->b_op_start = curwin->w_cursor;
curbuf->b_op_start.lnum = lnum;
curbuf->b_op_end.lnum = curwin->w_cursor.lnum - 1;
curbuf->b_op_end.col = bd.textcol + (colnr_T)totlen - 1;
curbuf->b_op_end.coladd = 0;
if (flags & PUT_CURSEND) {
colnr_T len;
curwin->w_cursor = curbuf->b_op_end;
curwin->w_cursor.col++;
len = (colnr_T)STRLEN(get_cursor_line_ptr());
if (curwin->w_cursor.col > len)
curwin->w_cursor.col = len;
} else
curwin->w_cursor.lnum = lnum;
} else {
if (y_type == kMTCharWise) {
if (dir == FORWARD && gchar_cursor() != NUL) {
int bytelen = (*mb_ptr2len)(get_cursor_pos_ptr());
col += bytelen;
if (yanklen) {
curwin->w_cursor.col += bytelen;
curbuf->b_op_end.col += bytelen;
}
}
curbuf->b_op_start = curwin->w_cursor;
}
else if (dir == BACKWARD)
--lnum;
new_cursor = curwin->w_cursor;
if (y_type == kMTCharWise && y_size == 1) {
linenr_T end_lnum = 0; 
if (VIsual_active) {
end_lnum = curbuf->b_visual.vi_end.lnum;
if (end_lnum < curbuf->b_visual.vi_start.lnum) {
end_lnum = curbuf->b_visual.vi_start.lnum;
}
}
do {
totlen = (size_t)(count * yanklen);
if (totlen > 0) {
oldp = ml_get(lnum);
if (VIsual_active && col > (int)STRLEN(oldp)) {
lnum++;
continue;
}
newp = (char_u *)xmalloc((size_t)(STRLEN(oldp) + totlen + 1));
memmove(newp, oldp, (size_t)col);
ptr = newp + col;
for (i = 0; i < (size_t)count; i++) {
memmove(ptr, y_array[0], (size_t)yanklen);
ptr += yanklen;
}
STRMOVE(ptr, oldp + col);
ml_replace(lnum, newp, false);
if (lnum == curwin->w_cursor.lnum) {
changed_cline_bef_curs();
curwin->w_cursor.col += (colnr_T)(totlen - 1);
}
}
if (VIsual_active) {
lnum++;
}
} while (VIsual_active && lnum <= end_lnum);
if (VIsual_active) { 
lnum--;
}
curbuf->b_op_end = curwin->w_cursor;
if (totlen && (restart_edit != 0 || (flags & PUT_CURSEND)))
++curwin->w_cursor.col;
changed_bytes(lnum, col);
extmark_splice(curbuf, (int)lnum-1, col,
0, 0,
0, (int)totlen, kExtmarkUndo);
} else {
for (cnt = 1; cnt <= count; cnt++) {
i = 0;
if (y_type == kMTCharWise) {
lnum = new_cursor.lnum;
ptr = ml_get(lnum) + col;
totlen = STRLEN(y_array[y_size - 1]);
newp = (char_u *) xmalloc((size_t)(STRLEN(ptr) + totlen + 1));
STRCPY(newp, y_array[y_size - 1]);
STRCAT(newp, ptr);
ml_append(lnum, newp, (colnr_T)0, false);
xfree(newp);
oldp = ml_get(lnum);
newp = (char_u *)xmalloc((size_t)col + (size_t)yanklen + 1);
memmove(newp, oldp, (size_t)col);
memmove(newp + col, y_array[0], (size_t)yanklen + 1);
ml_replace(lnum, newp, false);
curwin->w_cursor.lnum = lnum;
i = 1;
}
for (; i < y_size; i++) {
if ((y_type != kMTCharWise || i < y_size - 1)
&& ml_append(lnum, y_array[i], (colnr_T)0, false)
== FAIL) {
goto error;
}
lnum++;
++nr_lines;
if (flags & PUT_FIXINDENT) {
old_pos = curwin->w_cursor;
curwin->w_cursor.lnum = lnum;
ptr = ml_get(lnum);
if (cnt == count && i == y_size - 1)
lendiff = (int)STRLEN(ptr);
if (*ptr == '#' && preprocs_left())
indent = 0; 
else if (*ptr == NUL)
indent = 0; 
else if (first_indent) {
indent_diff = orig_indent - get_indent();
indent = orig_indent;
first_indent = FALSE;
} else if ((indent = get_indent() + indent_diff) < 0)
indent = 0;
(void)set_indent(indent, SIN_NOMARK);
curwin->w_cursor = old_pos;
if (cnt == count && i == y_size - 1)
lendiff -= (int)STRLEN(ml_get(lnum));
}
}
if (y_type == kMTCharWise) {
extmark_splice(curbuf, (int)new_cursor.lnum-1, col, 0, 0,
(int)y_size-1, (int)STRLEN(y_array[y_size-1]),
kExtmarkUndo);
} else if (y_type == kMTLineWise && flags & PUT_LINE_SPLIT) {
extmark_splice(curbuf, (int)new_cursor.lnum-1, col, 0, 0,
(int)y_size+1, 0, kExtmarkUndo);
}
}
error:
if (y_type == kMTLineWise) {
curbuf->b_op_start.col = 0;
if (dir == FORWARD)
curbuf->b_op_start.lnum++;
}
if (curbuf->b_op_start.lnum + (y_type == kMTCharWise) - 1 + nr_lines
< curbuf->b_ml.ml_line_count) {
ExtmarkOp kind = (y_type == kMTLineWise && !(flags & PUT_LINE_SPLIT))
? kExtmarkUndo : kExtmarkNOOP;
mark_adjust(curbuf->b_op_start.lnum + (y_type == kMTCharWise),
(linenr_T)MAXLNUM, nr_lines, 0L, kind);
}
if (y_type == kMTCharWise) {
changed_lines(curwin->w_cursor.lnum, col,
curwin->w_cursor.lnum + 1, nr_lines, true);
} else {
changed_lines(curbuf->b_op_start.lnum, 0,
curbuf->b_op_start.lnum, nr_lines, true);
}
curbuf->b_op_end.lnum = lnum;
col = (colnr_T)STRLEN(y_array[y_size - 1]) - lendiff;
if (col > 1)
curbuf->b_op_end.col = col - 1;
else
curbuf->b_op_end.col = 0;
if (flags & PUT_CURSLINE) {
curwin->w_cursor.lnum = lnum;
beginline(BL_WHITE | BL_FIX);
} else if (flags & PUT_CURSEND) {
if (y_type == kMTLineWise) {
if (lnum >= curbuf->b_ml.ml_line_count) {
curwin->w_cursor.lnum = curbuf->b_ml.ml_line_count;
} else {
curwin->w_cursor.lnum = lnum + 1;
}
curwin->w_cursor.col = 0;
} else {
curwin->w_cursor.lnum = lnum;
curwin->w_cursor.col = col;
}
} else if (y_type == kMTLineWise) {
curwin->w_cursor.col = 0;
if (dir == FORWARD)
++curwin->w_cursor.lnum;
beginline(BL_WHITE | BL_FIX);
} else 
curwin->w_cursor = new_cursor;
}
}
msgmore(nr_lines);
curwin->w_set_curswant = TRUE;
end:
if (allocated)
xfree(insert_string);
if (regname == '=')
xfree(y_array);
VIsual_active = FALSE;
adjust_cursor_eol();
} 
void adjust_cursor_eol(void)
{
if (curwin->w_cursor.col > 0
&& gchar_cursor() == NUL
&& (ve_flags & VE_ONEMORE) == 0
&& !(restart_edit || (State & INSERT))) {
dec_cursor();
if (ve_flags == VE_ALL) {
colnr_T scol, ecol;
getvcol(curwin, &curwin->w_cursor, &scol, NULL, &ecol);
curwin->w_cursor.coladd = ecol - scol + 1;
}
}
}
int preprocs_left(void)
{
return ((curbuf->b_p_si && !curbuf->b_p_cin)
|| (curbuf->b_p_cin && in_cinkeys('#', ' ', true)
&& curbuf->b_ind_hash_comment == 0));
}
int get_register_name(int num)
{
if (num == -1)
return '"';
else if (num < 10)
return num + '0';
else if (num == DELETION_REGISTER)
return '-';
else if (num == STAR_REGISTER)
return '*';
else if (num == PLUS_REGISTER)
return '+';
else {
return num + 'a' - 10;
}
}
void ex_display(exarg_T *eap)
{
char_u *p;
yankreg_T *yb;
int name;
char_u *arg = eap->arg;
int clen;
if (arg != NULL && *arg == NUL)
arg = NULL;
int attr = HL_ATTR(HLF_8);
MSG_PUTS_TITLE(_("\n--- Registers ---"));
for (int i = -1; i < NUM_REGISTERS && !got_int; i++) {
name = get_register_name(i);
if (arg != NULL && vim_strchr(arg, name) == NULL) {
continue; 
}
if (i == -1) {
if (y_previous != NULL)
yb = y_previous;
else
yb = &(y_regs[0]);
} else
yb = &(y_regs[i]);
get_clipboard(name, &yb, true);
if (name == mb_tolower(redir_reg)
|| (redir_reg == '"' && yb == y_previous)) {
continue; 
}
if (yb->y_array != NULL) {
msg_putchar('\n');
msg_putchar('"');
msg_putchar(name);
MSG_PUTS(" ");
int n = Columns - 6;
for (size_t j = 0; j < yb->y_size && n > 1; j++) {
if (j) {
MSG_PUTS_ATTR("^J", attr);
n -= 2;
}
for (p = yb->y_array[j]; *p && (n -= ptr2cells(p)) >= 0; p++) { 
clen = (*mb_ptr2len)(p);
msg_outtrans_len(p, clen);
p += clen - 1;
}
}
if (n > 1 && yb->y_type == kMTLineWise) {
MSG_PUTS_ATTR("^J", attr);
}
ui_flush(); 
}
os_breakcheck();
}
if ((p = get_last_insert()) != NULL
&& (arg == NULL || vim_strchr(arg, '.') != NULL) && !got_int) {
MSG_PUTS("\n\". ");
dis_msg(p, TRUE);
}
if (last_cmdline != NULL && (arg == NULL || vim_strchr(arg, ':') != NULL)
&& !got_int) {
MSG_PUTS("\n\": ");
dis_msg(last_cmdline, FALSE);
}
if (curbuf->b_fname != NULL
&& (arg == NULL || vim_strchr(arg, '%') != NULL) && !got_int) {
MSG_PUTS("\n\"% ");
dis_msg(curbuf->b_fname, FALSE);
}
if ((arg == NULL || vim_strchr(arg, '%') != NULL) && !got_int) {
char_u *fname;
linenr_T dummy;
if (buflist_name_nr(0, &fname, &dummy) != FAIL) {
MSG_PUTS("\n\"#");
dis_msg(fname, FALSE);
}
}
if (last_search_pat() != NULL
&& (arg == NULL || vim_strchr(arg, '/') != NULL) && !got_int) {
MSG_PUTS("\n\"/ ");
dis_msg(last_search_pat(), FALSE);
}
if (expr_line != NULL && (arg == NULL || vim_strchr(arg, '=') != NULL)
&& !got_int) {
MSG_PUTS("\n\"= ");
dis_msg(expr_line, FALSE);
}
}
static void
dis_msg(
char_u *p,
int skip_esc 
)
{
int n;
int l;
n = Columns - 6;
while (*p != NUL
&& !(*p == ESC && skip_esc && *(p + 1) == NUL)
&& (n -= ptr2cells(p)) >= 0) {
if ((l = utfc_ptr2len(p)) > 1) {
msg_outtrans_len(p, l);
p += l;
} else
msg_outtrans_len(p++, 1);
}
os_breakcheck();
}
char_u *skip_comment(
char_u *line, bool process, bool include_space, bool *is_comment
)
{
char_u *comment_flags = NULL;
int lead_len;
int leader_offset = get_last_leader_offset(line, &comment_flags);
*is_comment = false;
if (leader_offset != -1) {
while (*comment_flags) {
if (*comment_flags == COM_END
|| *comment_flags == ':') {
break;
}
comment_flags++;
}
if (*comment_flags != COM_END) {
*is_comment = true;
}
}
if (process == false) {
return line;
}
lead_len = get_leader_len(line, &comment_flags, false, include_space);
if (lead_len == 0)
return line;
while (*comment_flags) {
if (*comment_flags == COM_END
|| *comment_flags == ':') {
break;
}
++comment_flags;
}
if (*comment_flags == ':' || *comment_flags == NUL) {
line += lead_len;
}
return line;
}
int do_join(size_t count,
int insert_space,
int save_undo,
int use_formatoptions,
bool setmark)
{
char_u *curr = NULL;
char_u *curr_start = NULL;
char_u *cend;
char_u *newp;
char_u *spaces; 
int endcurr1 = NUL;
int endcurr2 = NUL;
int currsize = 0; 
int sumsize = 0; 
linenr_T t;
colnr_T col = 0;
int ret = OK;
int *comments = NULL;
int remove_comments = (use_formatoptions == TRUE)
&& has_format_option(FO_REMOVE_COMS);
bool prev_was_comment = false;
assert(count >= 1);
if (save_undo && u_save(curwin->w_cursor.lnum - 1,
curwin->w_cursor.lnum + (linenr_T)count) == FAIL) {
return FAIL;
}
spaces = xcalloc(count, 1);
if (remove_comments) {
comments = xcalloc(count, sizeof(*comments));
}
for (t = 0; t < (linenr_T)count; t++) {
curr = curr_start = ml_get((linenr_T)(curwin->w_cursor.lnum + t));
if (t == 0 && setmark) {
curwin->w_buffer->b_op_start.lnum = curwin->w_cursor.lnum;
curwin->w_buffer->b_op_start.col = (colnr_T)STRLEN(curr);
}
if (remove_comments) {
if (t > 0 && prev_was_comment) {
char_u *new_curr = skip_comment(curr, true, insert_space,
&prev_was_comment);
comments[t] = (int)(new_curr - curr);
curr = new_curr;
} else {
curr = skip_comment(curr, false, insert_space, &prev_was_comment);
}
}
if (insert_space && t > 0) {
curr = skipwhite(curr);
if (*curr != NUL
&& *curr != ')'
&& sumsize != 0
&& endcurr1 != TAB
&& (!has_format_option(FO_MBYTE_JOIN)
|| (utf_ptr2char(curr) < 0x100 && endcurr1 < 0x100))
&& (!has_format_option(FO_MBYTE_JOIN2)
|| utf_ptr2char(curr) < 0x100 || endcurr1 < 0x100)
) {
if (endcurr1 == ' ')
endcurr1 = endcurr2;
else
++spaces[t];
if (p_js && (endcurr1 == '.' || endcurr1 == '?' || endcurr1 == '!')) {
++spaces[t];
}
}
}
if (t > 0 && curbuf_splice_pending == 0) {
extmark_splice(curbuf, (int)curwin->w_cursor.lnum-1, sumsize,
1, (int)(curr- curr_start),
0, spaces[t],
kExtmarkUndo);
}
currsize = (int)STRLEN(curr);
sumsize += currsize + spaces[t];
endcurr1 = endcurr2 = NUL;
if (insert_space && currsize > 0) {
cend = curr + currsize;
MB_PTR_BACK(curr, cend);
endcurr1 = utf_ptr2char(cend);
if (cend > curr) {
MB_PTR_BACK(curr, cend);
endcurr2 = utf_ptr2char(cend);
}
}
line_breakcheck();
if (got_int) {
ret = FAIL;
goto theend;
}
}
col = sumsize - currsize - spaces[count - 1];
newp = (char_u *)xmalloc((size_t)sumsize + 1);
cend = newp + sumsize;
*cend = 0;
curbuf_splice_pending++;
for (t = (linenr_T)count - 1;; t--) {
cend -= currsize;
memmove(cend, curr, (size_t)currsize);
if (spaces[t] > 0) {
cend -= spaces[t];
memset(cend, ' ', (size_t)(spaces[t]));
}
const int spaces_removed = (int)((curr - curr_start) - spaces[t]);
linenr_T lnum = curwin->w_cursor.lnum + t;
colnr_T mincol = (colnr_T)0;
long lnum_amount = (linenr_T)-t;
long col_amount = (long)(cend - newp - spaces_removed);
mark_col_adjust(lnum, mincol, lnum_amount, col_amount, spaces_removed);
if (t == 0) {
break;
}
curr = curr_start = ml_get((linenr_T)(curwin->w_cursor.lnum + t - 1));
if (remove_comments)
curr += comments[t - 1];
if (insert_space && t > 1)
curr = skipwhite(curr);
currsize = (int)STRLEN(curr);
}
ml_replace(curwin->w_cursor.lnum, newp, false);
if (setmark) {
curwin->w_buffer->b_op_end.lnum = curwin->w_cursor.lnum;
curwin->w_buffer->b_op_end.col = sumsize;
}
changed_lines(curwin->w_cursor.lnum, currsize,
curwin->w_cursor.lnum + 1, 0L, true);
t = curwin->w_cursor.lnum;
curwin->w_cursor.lnum++;
del_lines((long)count - 1, false);
curwin->w_cursor.lnum = t;
curbuf_splice_pending--;
curwin->w_cursor.col =
(vim_strchr(p_cpo, CPO_JOINCOL) != NULL ? currsize : col);
check_cursor_col();
curwin->w_cursor.coladd = 0;
curwin->w_set_curswant = TRUE;
theend:
xfree(spaces);
if (remove_comments)
xfree(comments);
return ret;
}
static int same_leader(linenr_T lnum, int leader1_len, char_u *leader1_flags, int leader2_len, char_u *leader2_flags)
{
int idx1 = 0, idx2 = 0;
char_u *p;
char_u *line1;
char_u *line2;
if (leader1_len == 0)
return leader2_len == 0;
if (leader1_flags != NULL) {
for (p = leader1_flags; *p && *p != ':'; ++p) {
if (*p == COM_FIRST)
return leader2_len == 0;
if (*p == COM_END)
return FALSE;
if (*p == COM_START) {
if (*(ml_get(lnum) + leader1_len) == NUL)
return FALSE;
if (leader2_flags == NULL || leader2_len == 0)
return FALSE;
for (p = leader2_flags; *p && *p != ':'; ++p)
if (*p == COM_MIDDLE)
return TRUE;
return FALSE;
}
}
}
line1 = vim_strsave(ml_get(lnum));
for (idx1 = 0; ascii_iswhite(line1[idx1]); ++idx1)
;
line2 = ml_get(lnum + 1);
for (idx2 = 0; idx2 < leader2_len; ++idx2) {
if (!ascii_iswhite(line2[idx2])) {
if (line1[idx1++] != line2[idx2])
break;
} else
while (ascii_iswhite(line1[idx1]))
++idx1;
}
xfree(line1);
return idx2 == leader2_len && idx1 == leader1_len;
}
void
op_format(
oparg_T *oap,
int keep_cursor 
)
{
long old_line_count = curbuf->b_ml.ml_line_count;
curwin->w_cursor = oap->cursor_start;
if (u_save((linenr_T)(oap->start.lnum - 1),
(linenr_T)(oap->end.lnum + 1)) == FAIL)
return;
curwin->w_cursor = oap->start;
if (oap->is_VIsual)
redraw_curbuf_later(INVERTED);
curbuf->b_op_start = oap->start;
if (keep_cursor)
saved_cursor = oap->cursor_start;
format_lines(oap->line_count, keep_cursor);
if (oap->end_adjusted && curwin->w_cursor.lnum < curbuf->b_ml.ml_line_count)
++curwin->w_cursor.lnum;
beginline(BL_WHITE | BL_FIX);
old_line_count = curbuf->b_ml.ml_line_count - old_line_count;
msgmore(old_line_count);
curbuf->b_op_end = curwin->w_cursor;
if (keep_cursor) {
curwin->w_cursor = saved_cursor;
saved_cursor.lnum = 0;
}
if (oap->is_VIsual) {
FOR_ALL_WINDOWS_IN_TAB(wp, curtab) {
if (wp->w_old_cursor_lnum != 0) {
if (wp->w_old_cursor_lnum > wp->w_old_visual_lnum) {
wp->w_old_cursor_lnum += old_line_count;
} else {
wp->w_old_visual_lnum += old_line_count;
}
}
}
}
}
void op_formatexpr(oparg_T *oap)
{
if (oap->is_VIsual)
redraw_curbuf_later(INVERTED);
if (fex_format(oap->start.lnum, oap->line_count, NUL) != 0)
op_format(oap, FALSE);
}
int
fex_format(
linenr_T lnum,
long count,
int c 
)
{
int use_sandbox = was_set_insecurely((char_u *)"formatexpr",
OPT_LOCAL);
int r;
char_u *fex;
set_vim_var_nr(VV_LNUM, (varnumber_T)lnum);
set_vim_var_nr(VV_COUNT, (varnumber_T)count);
set_vim_var_char(c);
fex = vim_strsave(curbuf->b_p_fex);
if (use_sandbox) {
sandbox++;
}
r = (int)eval_to_number(fex);
if (use_sandbox) {
sandbox--;
}
set_vim_var_string(VV_CHAR, NULL, -1);
xfree(fex);
return r;
}
void
format_lines(
linenr_T line_count,
int avoid_fex 
)
{
int max_len;
int is_not_par; 
int next_is_not_par; 
int is_end_par; 
int prev_is_end_par = FALSE; 
int next_is_start_par = FALSE;
int leader_len = 0; 
int next_leader_len; 
char_u *leader_flags = NULL; 
char_u *next_leader_flags; 
int do_comments; 
int do_comments_list = 0; 
int advance = TRUE;
int second_indent = -1; 
int do_second_indent;
int do_number_indent;
int do_trail_white;
int first_par_line = TRUE;
int smd_save;
long count;
int need_set_indent = TRUE; 
int force_format = FALSE;
int old_State = State;
max_len = comp_textwidth(TRUE) * 3;
do_comments = has_format_option(FO_Q_COMS);
do_second_indent = has_format_option(FO_Q_SECOND);
do_number_indent = has_format_option(FO_Q_NUMBER);
do_trail_white = has_format_option(FO_WHITE_PAR);
if (curwin->w_cursor.lnum > 1)
is_not_par = fmt_check_par(curwin->w_cursor.lnum - 1
, &leader_len, &leader_flags, do_comments
);
else
is_not_par = TRUE;
next_is_not_par = fmt_check_par(curwin->w_cursor.lnum
, &next_leader_len, &next_leader_flags, do_comments
);
is_end_par = (is_not_par || next_is_not_par);
if (!is_end_par && do_trail_white)
is_end_par = !ends_in_white(curwin->w_cursor.lnum - 1);
curwin->w_cursor.lnum--;
for (count = line_count; count != 0 && !got_int; --count) {
if (advance) {
curwin->w_cursor.lnum++;
prev_is_end_par = is_end_par;
is_not_par = next_is_not_par;
leader_len = next_leader_len;
leader_flags = next_leader_flags;
}
if (count == 1 || curwin->w_cursor.lnum == curbuf->b_ml.ml_line_count) {
next_is_not_par = TRUE;
next_leader_len = 0;
next_leader_flags = NULL;
} else {
next_is_not_par = fmt_check_par(curwin->w_cursor.lnum + 1
, &next_leader_len, &next_leader_flags, do_comments
);
if (do_number_indent)
next_is_start_par =
(get_number_indent(curwin->w_cursor.lnum + 1) > 0);
}
advance = TRUE;
is_end_par = (is_not_par || next_is_not_par || next_is_start_par);
if (!is_end_par && do_trail_white)
is_end_par = !ends_in_white(curwin->w_cursor.lnum);
if (is_not_par) {
if (line_count < 0)
break;
} else {
if (first_par_line
&& (do_second_indent || do_number_indent)
&& prev_is_end_par
&& curwin->w_cursor.lnum < curbuf->b_ml.ml_line_count) {
if (do_second_indent && !LINEEMPTY(curwin->w_cursor.lnum + 1)) {
if (leader_len == 0 && next_leader_len == 0) {
second_indent =
get_indent_lnum(curwin->w_cursor.lnum + 1);
} else {
second_indent = next_leader_len;
do_comments_list = 1;
}
} else if (do_number_indent) {
if (leader_len == 0 && next_leader_len == 0) {
second_indent =
get_number_indent(curwin->w_cursor.lnum);
} else {
second_indent =
get_number_indent(curwin->w_cursor.lnum);
do_comments_list = 1;
}
}
}
if (curwin->w_cursor.lnum >= curbuf->b_ml.ml_line_count
|| !same_leader(curwin->w_cursor.lnum,
leader_len, leader_flags,
next_leader_len, next_leader_flags)
)
is_end_par = TRUE;
if (is_end_par || force_format) {
if (need_set_indent)
(void)set_indent(get_indent(), SIN_CHANGED);
State = NORMAL; 
coladvance((colnr_T)MAXCOL);
while (curwin->w_cursor.col && ascii_isspace(gchar_cursor()))
dec_cursor();
State = INSERT; 
smd_save = p_smd;
p_smd = FALSE;
insertchar(NUL, INSCHAR_FORMAT
+ (do_comments ? INSCHAR_DO_COM : 0)
+ (do_comments && do_comments_list
? INSCHAR_COM_LIST : 0)
+ (avoid_fex ? INSCHAR_NO_FEX : 0), second_indent);
State = old_State;
p_smd = smd_save;
second_indent = -1;
need_set_indent = is_end_par;
if (is_end_par) {
if (line_count < 0)
break;
first_par_line = TRUE;
}
force_format = FALSE;
}
if (!is_end_par) {
advance = FALSE;
curwin->w_cursor.lnum++;
curwin->w_cursor.col = 0;
if (line_count < 0 && u_save_cursor() == FAIL)
break;
if (next_leader_len > 0) {
(void)del_bytes(next_leader_len, false, false);
mark_col_adjust(curwin->w_cursor.lnum, (colnr_T)0, 0L,
(long)-next_leader_len, 0);
} else if (second_indent > 0) { 
int indent = (int)getwhitecols_curline();
if (indent > 0) {
(void)del_bytes(indent, false, false);
mark_col_adjust(curwin->w_cursor.lnum,
(colnr_T)0, 0L, (long)-indent, 0);
}
}
curwin->w_cursor.lnum--;
if (do_join(2, TRUE, FALSE, FALSE, false) == FAIL) {
beep_flush();
break;
}
first_par_line = FALSE;
if (STRLEN(get_cursor_line_ptr()) > (size_t)max_len)
force_format = TRUE;
else
force_format = FALSE;
}
}
line_breakcheck();
}
}
static int ends_in_white(linenr_T lnum)
{
char_u *s = ml_get(lnum);
size_t l;
if (*s == NUL)
return FALSE;
l = STRLEN(s) - 1;
return ascii_iswhite(s[l]);
}
static int fmt_check_par(linenr_T lnum, int *leader_len, char_u **leader_flags, int do_comments)
{
char_u *flags = NULL; 
char_u *ptr;
ptr = ml_get(lnum);
if (do_comments)
*leader_len = get_leader_len(ptr, leader_flags, FALSE, TRUE);
else
*leader_len = 0;
if (*leader_len > 0) {
flags = *leader_flags;
while (*flags && *flags != ':' && *flags != COM_END)
++flags;
}
return *skipwhite(ptr + *leader_len) == NUL
|| (*leader_len > 0 && *flags == COM_END)
|| startPS(lnum, NUL, FALSE);
}
int paragraph_start(linenr_T lnum)
{
char_u *p;
int leader_len = 0; 
char_u *leader_flags = NULL; 
int next_leader_len = 0; 
char_u *next_leader_flags = NULL; 
int do_comments; 
if (lnum <= 1)
return TRUE; 
p = ml_get(lnum - 1);
if (*p == NUL)
return TRUE; 
do_comments = has_format_option(FO_Q_COMS);
if (fmt_check_par(lnum - 1, &leader_len, &leader_flags, do_comments)) {
return true; 
}
if (fmt_check_par(lnum, &next_leader_len, &next_leader_flags, do_comments)) {
return true; 
}
if (has_format_option(FO_WHITE_PAR) && !ends_in_white(lnum - 1))
return TRUE; 
if (has_format_option(FO_Q_NUMBER) && (get_number_indent(lnum) > 0))
return TRUE; 
if (!same_leader(lnum - 1, leader_len, leader_flags,
next_leader_len, next_leader_flags))
return TRUE; 
return FALSE;
}
static void block_prep(oparg_T *oap, struct block_def *bdp, linenr_T lnum,
bool is_del)
{
int incr = 0;
char_u *pend;
char_u *pstart;
char_u *line;
char_u *prev_pstart;
char_u *prev_pend;
const int lbr_saved = curwin->w_p_lbr;
curwin->w_p_lbr = false;
bdp->startspaces = 0;
bdp->endspaces = 0;
bdp->textlen = 0;
bdp->start_vcol = 0;
bdp->end_vcol = 0;
bdp->is_short = false;
bdp->is_oneChar = false;
bdp->pre_whitesp = 0;
bdp->pre_whitesp_c = 0;
bdp->end_char_vcols = 0;
bdp->start_char_vcols = 0;
line = ml_get(lnum);
pstart = line;
prev_pstart = line;
while (bdp->start_vcol < oap->start_vcol && *pstart) {
incr = lbr_chartabsize(line, pstart, (colnr_T)bdp->start_vcol);
bdp->start_vcol += incr;
if (ascii_iswhite(*pstart)) {
bdp->pre_whitesp += incr;
bdp->pre_whitesp_c++;
} else {
bdp->pre_whitesp = 0;
bdp->pre_whitesp_c = 0;
}
prev_pstart = pstart;
MB_PTR_ADV(pstart);
}
bdp->start_char_vcols = incr;
if (bdp->start_vcol < oap->start_vcol) { 
bdp->end_vcol = bdp->start_vcol;
bdp->is_short = true;
if (!is_del || oap->op_type == OP_APPEND) {
bdp->endspaces = oap->end_vcol - oap->start_vcol + 1;
}
} else {
bdp->startspaces = bdp->start_vcol - oap->start_vcol;
if (is_del && bdp->startspaces)
bdp->startspaces = bdp->start_char_vcols - bdp->startspaces;
pend = pstart;
bdp->end_vcol = bdp->start_vcol;
if (bdp->end_vcol > oap->end_vcol) { 
bdp->is_oneChar = true;
if (oap->op_type == OP_INSERT) {
bdp->endspaces = bdp->start_char_vcols - bdp->startspaces;
} else if (oap->op_type == OP_APPEND) {
bdp->startspaces += oap->end_vcol - oap->start_vcol + 1;
bdp->endspaces = bdp->start_char_vcols - bdp->startspaces;
} else {
bdp->startspaces = oap->end_vcol - oap->start_vcol + 1;
if (is_del && oap->op_type != OP_LSHIFT) {
bdp->startspaces = bdp->start_char_vcols
- (bdp->start_vcol - oap->start_vcol);
bdp->endspaces = bdp->end_vcol - oap->end_vcol - 1;
}
}
} else {
prev_pend = pend;
while (bdp->end_vcol <= oap->end_vcol && *pend != NUL) {
prev_pend = pend;
incr = lbr_chartabsize_adv(line, &pend, (colnr_T)bdp->end_vcol);
bdp->end_vcol += incr;
}
if (bdp->end_vcol <= oap->end_vcol
&& (!is_del
|| oap->op_type == OP_APPEND
|| oap->op_type == OP_REPLACE)) { 
bdp->is_short = true;
if (oap->op_type == OP_APPEND || virtual_op) {
bdp->endspaces = oap->end_vcol - bdp->end_vcol
+ oap->inclusive;
}
} else if (bdp->end_vcol > oap->end_vcol) {
bdp->endspaces = bdp->end_vcol - oap->end_vcol - 1;
if (!is_del && bdp->endspaces) {
bdp->endspaces = incr - bdp->endspaces;
if (pend != pstart)
pend = prev_pend;
}
}
}
bdp->end_char_vcols = incr;
if (is_del && bdp->startspaces)
pstart = prev_pstart;
bdp->textlen = (int)(pend - pstart);
}
bdp->textcol = (colnr_T) (pstart - line);
bdp->textstart = pstart;
curwin->w_p_lbr = lbr_saved;
}
void op_addsub(oparg_T *oap, linenr_T Prenum1, bool g_cmd)
{
pos_T pos;
struct block_def bd;
ssize_t change_cnt = 0;
linenr_T amount = Prenum1;
if (!VIsual_active) {
pos = curwin->w_cursor;
if (u_save_cursor() == FAIL) {
return;
}
change_cnt = do_addsub(oap->op_type, &pos, 0, amount);
if (change_cnt) {
changed_lines(pos.lnum, 0, pos.lnum + 1, 0L, true);
}
} else {
int one_change;
int length;
pos_T startpos;
if (u_save((linenr_T)(oap->start.lnum - 1),
(linenr_T)(oap->end.lnum + 1)) == FAIL) {
return;
}
pos = oap->start;
for (; pos.lnum <= oap->end.lnum; pos.lnum++) {
if (oap->motion_type == kMTBlockWise) {
block_prep(oap, &bd, pos.lnum, false);
pos.col = bd.textcol;
length = bd.textlen;
} else if (oap->motion_type == kMTLineWise) {
curwin->w_cursor.col = 0;
pos.col = 0;
length = (colnr_T)STRLEN(ml_get(pos.lnum));
} else {
if (pos.lnum == oap->start.lnum && !oap->inclusive) {
dec(&(oap->end));
}
length = (colnr_T)STRLEN(ml_get(pos.lnum));
pos.col = 0;
if (pos.lnum == oap->start.lnum) {
pos.col += oap->start.col;
length -= oap->start.col;
}
if (pos.lnum == oap->end.lnum) {
length = (int)STRLEN(ml_get(oap->end.lnum));
if (oap->end.col >= length) {
oap->end.col = length - 1;
}
length = oap->end.col - pos.col + 1;
}
}
one_change = do_addsub(oap->op_type, &pos, length, amount);
if (one_change) {
if (change_cnt == 0) {
startpos = curbuf->b_op_start;
}
change_cnt++;
}
if (g_cmd && one_change) {
amount += Prenum1;
}
}
if (change_cnt) {
changed_lines(oap->start.lnum, 0, oap->end.lnum + 1, 0L, true);
}
if (!change_cnt && oap->is_VIsual) {
redraw_curbuf_later(INVERTED);
}
if (change_cnt > 0) {
curbuf->b_op_start = startpos;
}
if (change_cnt > p_report) {
if (change_cnt == 1) {
MSG(_("1 line changed"));
} else {
smsg((char *)_("%" PRId64 " lines changed"), (int64_t)change_cnt);
}
}
}
}
int do_addsub(int op_type, pos_T *pos, int length, linenr_T Prenum1)
{
int col;
char_u *buf1 = NULL;
char_u buf2[NUMBUFLEN];
int pre; 
static bool hexupper = false; 
uvarnumber_T n;
uvarnumber_T oldn;
char_u *ptr;
int c;
int todel;
bool dohex;
bool dooct;
bool dobin;
bool doalp;
int firstdigit;
bool subtract;
bool negative = false;
bool was_positive = true;
bool visual = VIsual_active;
bool did_change = false;
pos_T save_cursor = curwin->w_cursor;
int maxlen = 0;
pos_T startpos;
pos_T endpos;
dohex = (vim_strchr(curbuf->b_p_nf, 'x') != NULL); 
dooct = (vim_strchr(curbuf->b_p_nf, 'o') != NULL); 
dobin = (vim_strchr(curbuf->b_p_nf, 'b') != NULL); 
doalp = (vim_strchr(curbuf->b_p_nf, 'p') != NULL); 
curwin->w_cursor = *pos;
ptr = ml_get(pos->lnum);
col = pos->col;
if (*ptr == NUL) {
goto theend;
}
if (!VIsual_active) {
if (dobin) {
while (col > 0 && ascii_isbdigit(ptr[col])) {
col--;
col -= utf_head_off(ptr, ptr + col);
}
}
if (dohex) {
while (col > 0 && ascii_isxdigit(ptr[col])) {
col--;
col -= utf_head_off(ptr, ptr + col);
}
}
if (dobin
&& dohex
&& !((col > 0
&& (ptr[col] == 'X' || ptr[col] == 'x')
&& ptr[col - 1] == '0'
&& !utf_head_off(ptr, ptr + col - 1)
&& ascii_isxdigit(ptr[col + 1])))) {
col = curwin->w_cursor.col;
while (col > 0 && ascii_isdigit(ptr[col])) {
col--;
col -= utf_head_off(ptr, ptr + col);
}
}
if ((dohex
&& col > 0
&& (ptr[col] == 'X' || ptr[col] == 'x')
&& ptr[col - 1] == '0'
&& !utf_head_off(ptr, ptr + col - 1)
&& ascii_isxdigit(ptr[col + 1]))
|| (dobin
&& col > 0
&& (ptr[col] == 'B' || ptr[col] == 'b')
&& ptr[col - 1] == '0'
&& !utf_head_off(ptr, ptr + col - 1)
&& ascii_isbdigit(ptr[col + 1]))) {
col--;
col -= utf_head_off(ptr, ptr + col);
} else {
col = pos->col;
while (ptr[col] != NUL
&& !ascii_isdigit(ptr[col])
&& !(doalp && ASCII_ISALPHA(ptr[col]))) {
col++;
}
while (col > 0
&& ascii_isdigit(ptr[col - 1])
&& !(doalp && ASCII_ISALPHA(ptr[col]))) {
col--;
}
}
}
if (visual) {
while (ptr[col] != NUL && length > 0 && !ascii_isdigit(ptr[col])
&& !(doalp && ASCII_ISALPHA(ptr[col]))) {
int mb_len = utfc_ptr2len(ptr + col);
col += mb_len;
length -= mb_len;
}
if (length == 0) {
goto theend;
}
if (col > pos->col && ptr[col - 1] == '-'
&& !utf_head_off(ptr, ptr + col - 1)) {
negative = true;
was_positive = false;
}
}
firstdigit = ptr[col];
if (!ascii_isdigit(firstdigit) && !(doalp && ASCII_ISALPHA(firstdigit))) {
beep_flush();
goto theend;
}
if (doalp && ASCII_ISALPHA(firstdigit)) {
if (op_type == OP_NR_SUB) {
if (CharOrd(firstdigit) < Prenum1) {
if (isupper(firstdigit)) {
firstdigit = 'A';
} else {
firstdigit = 'a';
}
} else {
firstdigit -= (int)Prenum1;
}
} else {
if (26 - CharOrd(firstdigit) - 1 < Prenum1) {
if (isupper(firstdigit)) {
firstdigit = 'Z';
} else {
firstdigit = 'z';
}
} else {
firstdigit += (int)Prenum1;
}
}
curwin->w_cursor.col = col;
startpos = curwin->w_cursor;
did_change = true;
(void)del_char(false);
ins_char(firstdigit);
endpos = curwin->w_cursor;
curwin->w_cursor.col = col;
} else {
if (col > 0 && ptr[col - 1] == '-'
&& !utf_head_off(ptr, ptr + col - 1) && !visual) {
col--;
negative = true;
}
if (visual && VIsual_mode != 'V') {
maxlen = (curbuf->b_visual.vi_curswant == MAXCOL
? (int)STRLEN(ptr) - col
: length);
}
vim_str2nr(ptr + col, &pre, &length,
0 + (dobin ? STR2NR_BIN : 0)
+ (dooct ? STR2NR_OCT : 0)
+ (dohex ? STR2NR_HEX : 0),
NULL, &n, maxlen);
if (pre && negative) {
col++;
length--;
negative = false;
}
subtract = false;
if (op_type == OP_NR_SUB) {
subtract ^= true;
}
if (negative) {
subtract ^= true;
}
oldn = n;
n = subtract ? n - (uvarnumber_T)Prenum1
: n + (uvarnumber_T)Prenum1;
if (!pre) {
if (subtract) {
if (n > oldn) {
n = 1 + (n ^ (uvarnumber_T)-1);
negative ^= true;
}
} else {
if (n < oldn) {
n = (n ^ (uvarnumber_T)-1);
negative ^= true;
}
}
if (n == 0) {
negative = false;
}
}
if (visual && !was_positive && !negative && col > 0) {
col--;
length++;
}
curwin->w_cursor.col = col;
startpos = curwin->w_cursor;
did_change = true;
todel = length;
c = gchar_cursor();
if (c == '-') {
length--;
}
while (todel-- > 0) {
if (c < 0x100 && isalpha(c)) {
if (isupper(c)) {
hexupper = true;
} else {
hexupper = false;
}
}
(void)del_char(false);
c = gchar_cursor();
}
buf1 = xmalloc((size_t)length + NUMBUFLEN);
ptr = buf1;
if (negative && (!visual || was_positive)) {
*ptr++ = '-';
}
if (pre) {
*ptr++ = '0';
length--;
}
if (pre == 'b' || pre == 'B' || pre == 'x' || pre == 'X') {
*ptr++ = (char_u)pre;
length--;
}
if (pre == 'b' || pre == 'B') {
size_t bits = 0;
size_t i = 0;
for (bits = 8 * sizeof(n); bits > 0; bits--) {
if ((n >> (bits - 1)) & 0x1) {
break;
}
}
while (bits > 0) {
buf2[i++] = ((n >> --bits) & 0x1) ? '1' : '0';
}
buf2[i] = '\0';
} else if (pre == 0) {
vim_snprintf((char *)buf2, ARRAY_SIZE(buf2), "%" PRIu64, (uint64_t)n);
} else if (pre == '0') {
vim_snprintf((char *)buf2, ARRAY_SIZE(buf2), "%" PRIo64, (uint64_t)n);
} else if (hexupper) {
vim_snprintf((char *)buf2, ARRAY_SIZE(buf2), "%" PRIX64, (uint64_t)n);
} else {
vim_snprintf((char *)buf2, ARRAY_SIZE(buf2), "%" PRIx64, (uint64_t)n);
}
length -= (int)STRLEN(buf2);
if (firstdigit == '0' && !(dooct && pre == 0)) {
while (length-- > 0) {
*ptr++ = '0';
}
}
*ptr = NUL;
STRCAT(buf1, buf2);
ins_str(buf1); 
endpos = curwin->w_cursor;
if (curwin->w_cursor.col) {
curwin->w_cursor.col--;
}
}
curbuf->b_op_start = startpos;
curbuf->b_op_end = endpos;
if (curbuf->b_op_end.col > 0) {
curbuf->b_op_end.col--;
}
theend:
xfree(buf1);
if (visual) {
curwin->w_cursor = save_cursor;
} else if (did_change) {
curwin->w_set_curswant = true;
}
return did_change;
}
MotionType get_reg_type(int regname, colnr_T *reg_width)
{
switch (regname) {
case '%': 
case '#': 
case '=': 
case ':': 
case '/': 
case '.': 
case Ctrl_F: 
case Ctrl_P: 
case Ctrl_W: 
case Ctrl_A: 
case '_': 
return kMTCharWise;
}
if (regname != NUL && !valid_yank_reg(regname, false)) {
return kMTUnknown;
}
yankreg_T *reg = get_yank_register(regname, YREG_PASTE);
if (reg->y_array != NULL) {
if (reg_width != NULL && reg->y_type == kMTBlockWise) {
*reg_width = reg->y_width;
}
return reg->y_type;
}
return kMTUnknown;
}
void format_reg_type(MotionType reg_type, colnr_T reg_width,
char *buf, size_t buf_len)
FUNC_ATTR_NONNULL_ALL
{
assert(buf_len > 1);
switch (reg_type) {
case kMTLineWise:
buf[0] = 'V';
buf[1] = NUL;
break;
case kMTCharWise:
buf[0] = 'v';
buf[1] = NUL;
break;
case kMTBlockWise:
snprintf(buf, buf_len, CTRL_V_STR "%" PRIdCOLNR, reg_width + 1);
break;
case kMTUnknown:
buf[0] = NUL;
break;
}
}
static void *get_reg_wrap_one_line(char_u *s, int flags)
{
if (!(flags & kGRegList)) {
return s;
}
list_T *const list = tv_list_alloc(1);
tv_list_append_allocated_string(list, (char *)s);
return list;
}
void *get_reg_contents(int regname, int flags)
{
if (regname == '=') {
if (flags & kGRegNoExpr) {
return NULL;
}
if (flags & kGRegExprSrc) {
return get_reg_wrap_one_line(get_expr_line_src(), flags);
}
return get_reg_wrap_one_line(get_expr_line(), flags);
}
if (regname == '@') 
regname = '"';
if (regname != NUL && !valid_yank_reg(regname, false))
return NULL;
char_u *retval;
bool allocated;
if (get_spec_reg(regname, &retval, &allocated, false)) {
if (retval == NULL) {
return NULL;
}
if (allocated) {
return get_reg_wrap_one_line(retval, flags);
}
return get_reg_wrap_one_line(vim_strsave(retval), flags);
}
yankreg_T *reg = get_yank_register(regname, YREG_PASTE);
if (reg->y_array == NULL)
return NULL;
if (flags & kGRegList) {
list_T *const list = tv_list_alloc((ptrdiff_t)reg->y_size);
for (size_t i = 0; i < reg->y_size; i++) {
tv_list_append_string(list, (const char *)reg->y_array[i], -1);
}
return list;
}
size_t len = 0;
for (size_t i = 0; i < reg->y_size; i++) {
len += STRLEN(reg->y_array[i]);
if (reg->y_type == kMTLineWise || i < reg->y_size - 1) {
len++;
}
}
retval = xmalloc(len + 1);
len = 0;
for (size_t i = 0; i < reg->y_size; i++) {
STRCPY(retval + len, reg->y_array[i]);
len += STRLEN(retval + len);
if (reg->y_type == kMTLineWise || i < reg->y_size - 1) {
retval[len++] = '\n';
}
}
retval[len] = NUL;
return retval;
}
static yankreg_T *init_write_reg(int name, yankreg_T **old_y_previous, bool must_append)
{
if (!valid_yank_reg(name, true)) { 
emsg_invreg(name);
return NULL;
}
*old_y_previous = y_previous;
yankreg_T *reg = get_yank_register(name, YREG_YANK);
if (!is_append_register(name) && !must_append) {
free_register(reg);
}
return reg;
}
static void finish_write_reg(int name, yankreg_T *reg, yankreg_T *old_y_previous)
{
set_clipboard(name, reg);
if (name != '"') {
y_previous = old_y_previous;
}
}
void write_reg_contents(int name, const char_u *str, ssize_t len,
int must_append)
{
write_reg_contents_ex(name, str, len, must_append, kMTUnknown, 0L);
}
void write_reg_contents_lst(int name, char_u **strings,
bool must_append, MotionType yank_type,
colnr_T block_len)
{
if (name == '/' || name == '=') {
char_u *s = strings[0];
if (strings[0] == NULL) {
s = (char_u *)"";
} else if (strings[1] != NULL) {
EMSG(_("E883: search pattern and expression register may not "
"contain two or more lines"));
return;
}
write_reg_contents_ex(name, s, -1, must_append, yank_type, block_len);
return;
}
if (name == '_') {
return;
}
yankreg_T *old_y_previous, *reg;
if (!(reg = init_write_reg(name, &old_y_previous, must_append))) {
return;
}
str_to_reg(reg, yank_type, (char_u *)strings, STRLEN((char_u *)strings),
block_len, true);
finish_write_reg(name, reg, old_y_previous);
}
void write_reg_contents_ex(int name,
const char_u *str,
ssize_t len,
bool must_append,
MotionType yank_type,
colnr_T block_len)
{
if (len < 0) {
len = (ssize_t) STRLEN(str);
}
if (name == '/') {
set_last_search_pat(str, RE_SEARCH, TRUE, TRUE);
return;
}
if (name == '#') {
buf_T *buf;
if (ascii_isdigit(*str)) {
int num = atoi((char *)str);
buf = buflist_findnr(num);
if (buf == NULL) {
EMSGN(_(e_nobufnr), (long)num);
}
} else {
buf = buflist_findnr(buflist_findpat(str, str + STRLEN(str),
true, false, false));
}
if (buf == NULL) {
return;
}
curwin->w_alt_fnum = buf->b_fnum;
return;
}
if (name == '=') {
size_t offset = 0;
size_t totlen = (size_t) len;
if (must_append && expr_line) {
size_t exprlen = STRLEN(expr_line);
totlen += exprlen;
offset = exprlen;
}
expr_line = xrealloc(expr_line, totlen + 1);
memcpy(expr_line + offset, str, (size_t)len);
expr_line[totlen] = NUL;
return;
}
if (name == '_') { 
return;
}
yankreg_T *old_y_previous, *reg;
if (!(reg = init_write_reg(name, &old_y_previous, must_append))) {
return;
}
str_to_reg(reg, yank_type, str, (size_t)len, block_len, false);
finish_write_reg(name, reg, old_y_previous);
}
static void str_to_reg(yankreg_T *y_ptr, MotionType yank_type,
const char_u *str, size_t len, colnr_T blocklen,
bool str_list)
FUNC_ATTR_NONNULL_ALL
{
if (y_ptr->y_array == NULL) { 
y_ptr->y_size = 0;
}
if (yank_type == kMTUnknown) {
yank_type = ((str_list
|| (len > 0 && (str[len - 1] == NL || str[len - 1] == CAR)))
? kMTLineWise : kMTCharWise);
}
size_t newlines = 0;
bool extraline = false; 
bool append = false; 
if (str_list) {
for (char_u **ss = (char_u **) str; *ss != NULL; ++ss) {
newlines++;
}
} else {
newlines = memcnt(str, '\n', len);
if (yank_type == kMTCharWise || len == 0 || str[len - 1] != '\n') {
extraline = 1;
++newlines; 
}
if (y_ptr->y_size > 0 && y_ptr->y_type == kMTCharWise) {
append = true;
--newlines; 
}
}
char_u **pp = xrealloc(y_ptr->y_array,
(y_ptr->y_size + newlines) * sizeof(char_u *));
y_ptr->y_array = pp;
size_t lnum = y_ptr->y_size; 
size_t maxlen = 0;
if (str_list) {
for (char_u **ss = (char_u **) str; *ss != NULL; ++ss, ++lnum) {
size_t ss_len = STRLEN(*ss);
pp[lnum] = xmemdupz(*ss, ss_len);
if (ss_len > maxlen) {
maxlen = ss_len;
}
}
} else {
size_t line_len;
for (const char_u *start = str, *end = str + len;
start < end + extraline;
start += line_len + 1, lnum++) {
assert(end - start >= 0);
line_len = (size_t)((char_u *)xmemscan(start, '\n',
(size_t)(end - start)) - start);
if (line_len > maxlen) {
maxlen = line_len;
}
size_t extra = append ? STRLEN(pp[--lnum]) : 0;
char_u *s = xmallocz(line_len + extra);
memcpy(s, pp[lnum], extra);
memcpy(s + extra, start, line_len);
size_t s_len = extra + line_len;
if (append) {
xfree(pp[lnum]);
append = false; 
}
pp[lnum] = s;
memchrsub(pp[lnum], NUL, '\n', s_len);
}
}
y_ptr->y_type = yank_type;
y_ptr->y_size = lnum;
set_yreg_additional_data(y_ptr, NULL);
y_ptr->timestamp = os_time();
if (yank_type == kMTBlockWise) {
y_ptr->y_width = (blocklen == -1 ? (colnr_T) maxlen - 1 : blocklen);
} else {
y_ptr->y_width = 0;
}
}
void clear_oparg(oparg_T *oap)
{
memset(oap, 0, sizeof(oparg_T));
}
static varnumber_T line_count_info(char_u *line, varnumber_T *wc,
varnumber_T *cc, varnumber_T limit,
int eol_size)
{
varnumber_T i;
varnumber_T words = 0;
varnumber_T chars = 0;
int is_word = 0;
for (i = 0; i < limit && line[i] != NUL; ) {
if (is_word) {
if (ascii_isspace(line[i])) {
words++;
is_word = 0;
}
} else if (!ascii_isspace(line[i]))
is_word = 1;
++chars;
i += (*mb_ptr2len)(line + i);
}
if (is_word)
words++;
*wc += words;
if (i < limit && line[i] == NUL) {
i += eol_size;
chars += eol_size;
}
*cc += chars;
return i;
}
void cursor_pos_info(dict_T *dict)
{
char_u *p;
char_u buf1[50];
char_u buf2[40];
linenr_T lnum;
varnumber_T byte_count = 0;
varnumber_T bom_count = 0;
varnumber_T byte_count_cursor = 0;
varnumber_T char_count = 0;
varnumber_T char_count_cursor = 0;
varnumber_T word_count = 0;
varnumber_T word_count_cursor = 0;
int eol_size;
varnumber_T last_check = 100000L;
long line_count_selected = 0;
pos_T min_pos, max_pos;
oparg_T oparg;
struct block_def bd;
const int l_VIsual_active = VIsual_active;
const int l_VIsual_mode = VIsual_mode;
if (curbuf->b_ml.ml_flags & ML_EMPTY) {
if (dict == NULL) {
MSG(_(no_lines_msg));
return;
}
} else {
if (get_fileformat(curbuf) == EOL_DOS)
eol_size = 2;
else
eol_size = 1;
if (l_VIsual_active) {
if (lt(VIsual, curwin->w_cursor)) {
min_pos = VIsual;
max_pos = curwin->w_cursor;
} else {
min_pos = curwin->w_cursor;
max_pos = VIsual;
}
if (*p_sel == 'e' && max_pos.col > 0)
--max_pos.col;
if (l_VIsual_mode == Ctrl_V) {
char_u * saved_sbr = p_sbr;
p_sbr = empty_option;
oparg.is_VIsual = true;
oparg.motion_type = kMTBlockWise;
oparg.op_type = OP_NOP;
getvcols(curwin, &min_pos, &max_pos,
&oparg.start_vcol, &oparg.end_vcol);
p_sbr = saved_sbr;
if (curwin->w_curswant == MAXCOL)
oparg.end_vcol = MAXCOL;
if (oparg.end_vcol < oparg.start_vcol) {
oparg.end_vcol += oparg.start_vcol;
oparg.start_vcol = oparg.end_vcol - oparg.start_vcol;
oparg.end_vcol -= oparg.start_vcol;
}
}
line_count_selected = max_pos.lnum - min_pos.lnum + 1;
}
for (lnum = 1; lnum <= curbuf->b_ml.ml_line_count; ++lnum) {
if (byte_count > last_check) {
os_breakcheck();
if (got_int)
return;
last_check = byte_count + 100000L;
}
if (l_VIsual_active
&& lnum >= min_pos.lnum && lnum <= max_pos.lnum) {
char_u *s = NULL;
long len = 0L;
switch (l_VIsual_mode) {
case Ctrl_V:
virtual_op = virtual_active();
block_prep(&oparg, &bd, lnum, false);
virtual_op = kNone;
s = bd.textstart;
len = (long)bd.textlen;
break;
case 'V':
s = ml_get(lnum);
len = MAXCOL;
break;
case 'v':
{
colnr_T start_col = (lnum == min_pos.lnum)
? min_pos.col : 0;
colnr_T end_col = (lnum == max_pos.lnum)
? max_pos.col - start_col + 1 : MAXCOL;
s = ml_get(lnum) + start_col;
len = end_col;
}
break;
}
if (s != NULL) {
byte_count_cursor += line_count_info(s, &word_count_cursor,
&char_count_cursor, len, eol_size);
if (lnum == curbuf->b_ml.ml_line_count
&& !curbuf->b_p_eol
&& (curbuf->b_p_bin || !curbuf->b_p_fixeol)
&& (long)STRLEN(s) < len)
byte_count_cursor -= eol_size;
}
} else {
if (lnum == curwin->w_cursor.lnum) {
word_count_cursor += word_count;
char_count_cursor += char_count;
byte_count_cursor = byte_count
+ line_count_info(ml_get(lnum), &word_count_cursor,
&char_count_cursor,
(varnumber_T)curwin->w_cursor.col + 1,
eol_size);
}
}
byte_count += line_count_info(ml_get(lnum), &word_count, &char_count,
(varnumber_T)MAXCOL, eol_size);
}
if (!curbuf->b_p_eol && (curbuf->b_p_bin || !curbuf->b_p_fixeol)) {
byte_count -= eol_size;
}
if (dict == NULL) {
if (l_VIsual_active) {
if (l_VIsual_mode == Ctrl_V && curwin->w_curswant < MAXCOL) {
getvcols(curwin, &min_pos, &max_pos, &min_pos.col, &max_pos.col);
int64_t cols;
STRICT_SUB(oparg.end_vcol + 1, oparg.start_vcol, &cols, int64_t);
vim_snprintf((char *)buf1, sizeof(buf1), _("%" PRId64 " Cols; "),
cols);
} else {
buf1[0] = NUL;
}
if (char_count_cursor == byte_count_cursor
&& char_count == byte_count) {
vim_snprintf((char *)IObuff, IOSIZE,
_("Selected %s%" PRId64 " of %" PRId64 " Lines;"
" %" PRId64 " of %" PRId64 " Words;"
" %" PRId64 " of %" PRId64 " Bytes"),
buf1, (int64_t)line_count_selected,
(int64_t)curbuf->b_ml.ml_line_count,
(int64_t)word_count_cursor, (int64_t)word_count,
(int64_t)byte_count_cursor, (int64_t)byte_count);
} else {
vim_snprintf((char *)IObuff, IOSIZE,
_("Selected %s%" PRId64 " of %" PRId64 " Lines;"
" %" PRId64 " of %" PRId64 " Words;"
" %" PRId64 " of %" PRId64 " Chars;"
" %" PRId64 " of %" PRId64 " Bytes"),
buf1, (int64_t)line_count_selected,
(int64_t)curbuf->b_ml.ml_line_count,
(int64_t)word_count_cursor, (int64_t)word_count,
(int64_t)char_count_cursor, (int64_t)char_count,
(int64_t)byte_count_cursor, (int64_t)byte_count);
}
} else {
p = get_cursor_line_ptr();
validate_virtcol();
col_print(buf1, sizeof(buf1), (int)curwin->w_cursor.col + 1,
(int)curwin->w_virtcol + 1);
col_print(buf2, sizeof(buf2), (int)STRLEN(p), linetabsize(p));
if (char_count_cursor == byte_count_cursor
&& char_count == byte_count) {
vim_snprintf((char *)IObuff, IOSIZE,
_("Col %s of %s; Line %" PRId64 " of %" PRId64 ";"
" Word %" PRId64 " of %" PRId64 ";"
" Byte %" PRId64 " of %" PRId64 ""),
(char *)buf1, (char *)buf2,
(int64_t)curwin->w_cursor.lnum,
(int64_t)curbuf->b_ml.ml_line_count,
(int64_t)word_count_cursor, (int64_t)word_count,
(int64_t)byte_count_cursor, (int64_t)byte_count);
} else {
vim_snprintf((char *)IObuff, IOSIZE,
_("Col %s of %s; Line %" PRId64 " of %" PRId64 ";"
" Word %" PRId64 " of %" PRId64 ";"
" Char %" PRId64 " of %" PRId64 ";"
" Byte %" PRId64 " of %" PRId64 ""),
(char *)buf1, (char *)buf2,
(int64_t)curwin->w_cursor.lnum,
(int64_t)curbuf->b_ml.ml_line_count,
(int64_t)word_count_cursor, (int64_t)word_count,
(int64_t)char_count_cursor, (int64_t)char_count,
(int64_t)byte_count_cursor, (int64_t)byte_count);
}
}
}
bom_count = bomb_size();
if (dict == NULL && bom_count > 0) {
const size_t len = STRLEN(IObuff);
vim_snprintf((char *)IObuff + len, IOSIZE - len,
_("(+%" PRId64 " for BOM)"), (int64_t)bom_count);
}
if (dict == NULL) {
p = p_shm;
p_shm = (char_u *)"";
msg(IObuff);
p_shm = p;
}
}
if (dict != NULL) {
tv_dict_add_nr(dict, S_LEN("words"), (varnumber_T)word_count);
tv_dict_add_nr(dict, S_LEN("chars"), (varnumber_T)char_count);
tv_dict_add_nr(dict, S_LEN("bytes"), (varnumber_T)(byte_count + bom_count));
STATIC_ASSERT(sizeof("visual") == sizeof("cursor"),
"key_len argument in tv_dict_add_nr is wrong");
tv_dict_add_nr(dict, l_VIsual_active ? "visual_bytes" : "cursor_bytes",
sizeof("visual_bytes") - 1, (varnumber_T)byte_count_cursor);
tv_dict_add_nr(dict, l_VIsual_active ? "visual_chars" : "cursor_chars",
sizeof("visual_chars") - 1, (varnumber_T)char_count_cursor);
tv_dict_add_nr(dict, l_VIsual_active ? "visual_words" : "cursor_words",
sizeof("visual_words") - 1, (varnumber_T)word_count_cursor);
}
}
int get_default_register_name(void)
{
int name = NUL;
adjust_clipboard_name(&name, true, false);
return name;
}
static yankreg_T *adjust_clipboard_name(int *name, bool quiet, bool writing)
{
#define MSG_NO_CLIP "clipboard: No provider. " "Try \":checkhealth\" or \":h clipboard\"."
yankreg_T *target = NULL;
bool explicit_cb_reg = (*name == '*' || *name == '+');
bool implicit_cb_reg = (*name == NUL) && (cb_flags & CB_UNNAMEDMASK);
if (!explicit_cb_reg && !implicit_cb_reg) {
goto end;
}
if (!eval_has_provider("clipboard")) {
if (batch_change_count == 1 && !quiet
&& (!clipboard_didwarn || (explicit_cb_reg && !redirecting()))) {
clipboard_didwarn = true;
msg((char_u *)MSG_NO_CLIP);
}
goto end;
}
if (explicit_cb_reg) {
target = &y_regs[*name == '*' ? STAR_REGISTER : PLUS_REGISTER];
if (writing && (cb_flags & (*name == '*' ? CB_UNNAMED : CB_UNNAMEDPLUS))) {
clipboard_needs_update = false;
}
goto end;
} else { 
if (writing && clipboard_delay_update) {
clipboard_needs_update = true;
goto end;
} else if (!writing && clipboard_needs_update) {
goto end;
}
if (cb_flags & CB_UNNAMEDPLUS) {
*name = (cb_flags & CB_UNNAMED && writing) ? '"': '+';
target = &y_regs[PLUS_REGISTER];
} else {
*name = '*';
target = &y_regs[STAR_REGISTER];
}
goto end;
}
end:
return target;
}
bool prepare_yankreg_from_object(yankreg_T *reg, String regtype, size_t lines)
{
char type = regtype.data ? regtype.data[0] : NUL;
switch (type) {
case 0:
reg->y_type = kMTUnknown;
break;
case 'v': case 'c':
reg->y_type = kMTCharWise;
break;
case 'V': case 'l':
reg->y_type = kMTLineWise;
break;
case 'b': case Ctrl_V:
reg->y_type = kMTBlockWise;
break;
default:
return false;
}
reg->y_width = 0;
if (regtype.size > 1) {
if (reg->y_type != kMTBlockWise) {
return false;
}
if (!ascii_isdigit(regtype.data[1])) {
return false;
}
const char *p = regtype.data+1;
reg->y_width = getdigits_int((char_u **)&p, false, 1) - 1;
if (regtype.size > (size_t)(p-regtype.data)) {
return false;
}
}
reg->y_array = xcalloc(lines, sizeof(uint8_t *));
reg->y_size = lines;
reg->additional_data = NULL;
reg->timestamp = 0;
return true;
}
void finish_yankreg_from_object(yankreg_T *reg, bool clipboard_adjust)
{
if (reg->y_size > 0 && strlen((char *)reg->y_array[reg->y_size-1]) == 0) {
if (reg->y_type != kMTCharWise) {
if (reg->y_type == kMTUnknown || clipboard_adjust) {
xfree(reg->y_array[reg->y_size-1]);
reg->y_size--;
}
if (reg->y_type == kMTUnknown) {
reg->y_type = kMTLineWise;
}
}
} else {
if (reg->y_type == kMTUnknown) {
reg->y_type = kMTCharWise;
}
}
if (reg->y_type == kMTBlockWise) {
size_t maxlen = 0;
for (size_t i = 0; i < reg->y_size; i++) {
size_t rowlen = STRLEN(reg->y_array[i]);
if (rowlen > maxlen) {
maxlen = rowlen;
}
}
assert(maxlen <= INT_MAX);
reg->y_width = MAX(reg->y_width, (int)maxlen - 1);
}
}
static bool get_clipboard(int name, yankreg_T **target, bool quiet)
{
bool errmsg = true;
yankreg_T *reg = adjust_clipboard_name(&name, quiet, false);
if (reg == NULL) {
return false;
}
free_register(reg);
list_T *const args = tv_list_alloc(1);
const char regname = (char)name;
tv_list_append_string(args, &regname, 1);
typval_T result = eval_call_provider("clipboard", "get", args);
if (result.v_type != VAR_LIST) {
if (result.v_type == VAR_NUMBER && result.vval.v_number == 0) {
errmsg = false;
}
goto err;
}
list_T *res = result.vval.v_list;
list_T *lines = NULL;
if (tv_list_len(res) == 2
&& TV_LIST_ITEM_TV(tv_list_first(res))->v_type == VAR_LIST) {
lines = TV_LIST_ITEM_TV(tv_list_first(res))->vval.v_list;
if (TV_LIST_ITEM_TV(tv_list_last(res))->v_type != VAR_STRING) {
goto err;
}
char_u *regtype = TV_LIST_ITEM_TV(tv_list_last(res))->vval.v_string;
if (regtype == NULL || strlen((char *)regtype) > 1) {
goto err;
}
switch (regtype[0]) {
case 0:
reg->y_type = kMTUnknown;
break;
case 'v': case 'c':
reg->y_type = kMTCharWise;
break;
case 'V': case 'l':
reg->y_type = kMTLineWise;
break;
case 'b': case Ctrl_V:
reg->y_type = kMTBlockWise;
break;
default:
goto err;
}
} else {
lines = res;
reg->y_type = kMTUnknown;
}
reg->y_array = xcalloc((size_t)tv_list_len(lines), sizeof(char_u *));
reg->y_size = (size_t)tv_list_len(lines);
reg->additional_data = NULL;
reg->timestamp = 0;
size_t tv_idx = 0;
TV_LIST_ITER_CONST(lines, li, {
if (TV_LIST_ITEM_TV(li)->v_type != VAR_STRING) {
goto err;
}
reg->y_array[tv_idx++] = (char_u *)xstrdupnul(
(const char *)TV_LIST_ITEM_TV(li)->vval.v_string);
});
if (reg->y_size > 0 && strlen((char*)reg->y_array[reg->y_size-1]) == 0) {
if (reg->y_type != kMTCharWise) {
xfree(reg->y_array[reg->y_size-1]);
reg->y_size--;
if (reg->y_type == kMTUnknown) {
reg->y_type = kMTLineWise;
}
}
} else {
if (reg->y_type == kMTUnknown) {
reg->y_type = kMTCharWise;
}
}
if (reg->y_type == kMTBlockWise) {
size_t maxlen = 0;
for (size_t i = 0; i < reg->y_size; i++) {
size_t rowlen = STRLEN(reg->y_array[i]);
if (rowlen > maxlen) {
maxlen = rowlen;
}
}
assert(maxlen <= INT_MAX);
reg->y_width = (int)maxlen - 1;
}
*target = reg;
return true;
err:
if (reg->y_array) {
for (size_t i = 0; i < reg->y_size; i++) {
xfree(reg->y_array[i]);
}
xfree(reg->y_array);
}
reg->y_array = NULL;
reg->y_size = 0;
reg->additional_data = NULL;
reg->timestamp = 0;
if (errmsg) {
EMSG("clipboard: provider returned invalid data");
}
*target = reg;
return false;
}
static void set_clipboard(int name, yankreg_T *reg)
{
if (!adjust_clipboard_name(&name, false, true)) {
return;
}
list_T *const lines = tv_list_alloc(
(ptrdiff_t)reg->y_size + (reg->y_type != kMTCharWise));
for (size_t i = 0; i < reg->y_size; i++) {
tv_list_append_string(lines, (const char *)reg->y_array[i], -1);
}
char regtype;
switch (reg->y_type) {
case kMTLineWise: {
regtype = 'V';
tv_list_append_string(lines, NULL, 0);
break;
}
case kMTCharWise: {
regtype = 'v';
break;
}
case kMTBlockWise: {
regtype = 'b';
tv_list_append_string(lines, NULL, 0);
break;
}
case kMTUnknown: {
assert(false);
}
}
list_T *args = tv_list_alloc(3);
tv_list_append_list(args, lines);
tv_list_append_string(args, &regtype, 1); 
tv_list_append_string(args, ((char[]) { (char)name }), 1);
(void)eval_call_provider("clipboard", "set", args);
}
void start_batch_changes(void)
{
if (++batch_change_count > 1) {
return;
}
clipboard_delay_update = true;
}
void end_batch_changes(void)
{
if (--batch_change_count > 0) {
return;
}
clipboard_delay_update = false;
if (clipboard_needs_update) {
clipboard_needs_update = false;
set_clipboard(NUL, y_previous);
}
}
int save_batch_count(void)
{
int save_count = batch_change_count;
batch_change_count = 0;
clipboard_delay_update = false;
if (clipboard_needs_update) {
clipboard_needs_update = false;
set_clipboard(NUL, y_previous);
}
return save_count;
}
void restore_batch_count(int save_count)
{
assert(batch_change_count == 0);
batch_change_count = save_count;
if (batch_change_count > 0) {
clipboard_delay_update = true;
}
}
static inline bool reg_empty(const yankreg_T *const reg)
FUNC_ATTR_PURE
{
return (reg->y_array == NULL
|| reg->y_size == 0
|| (reg->y_size == 1
&& reg->y_type == kMTCharWise
&& *(reg->y_array[0]) == NUL));
}
const void *op_global_reg_iter(const void *const iter, char *const name,
yankreg_T *const reg, bool *is_unnamed)
FUNC_ATTR_NONNULL_ARG(2, 3, 4) FUNC_ATTR_WARN_UNUSED_RESULT
{
return op_reg_iter(iter, y_regs, name, reg, is_unnamed);
}
const void *op_reg_iter(const void *const iter, const yankreg_T *const regs,
char *const name, yankreg_T *const reg,
bool *is_unnamed)
FUNC_ATTR_NONNULL_ARG(3, 4, 5) FUNC_ATTR_WARN_UNUSED_RESULT
{
*name = NUL;
const yankreg_T *iter_reg = (iter == NULL
? &(regs[0])
: (const yankreg_T *const)iter);
while (iter_reg - &(regs[0]) < NUM_SAVED_REGISTERS && reg_empty(iter_reg)) {
iter_reg++;
}
if (iter_reg - &(regs[0]) == NUM_SAVED_REGISTERS || reg_empty(iter_reg)) {
return NULL;
}
int iter_off = (int)(iter_reg - &(regs[0]));
*name = (char)get_register_name(iter_off);
*reg = *iter_reg;
*is_unnamed = (iter_reg == y_previous);
while (++iter_reg - &(regs[0]) < NUM_SAVED_REGISTERS) {
if (!reg_empty(iter_reg)) {
return (void *) iter_reg;
}
}
return NULL;
}
size_t op_reg_amount(void)
FUNC_ATTR_WARN_UNUSED_RESULT
{
size_t ret = 0;
for (size_t i = 0; i < NUM_SAVED_REGISTERS; i++) {
if (!reg_empty(y_regs + i)) {
ret++;
}
}
return ret;
}
bool op_reg_set(const char name, const yankreg_T reg, bool is_unnamed)
{
int i = op_reg_index(name);
if (i == -1) {
return false;
}
free_register(&y_regs[i]);
y_regs[i] = reg;
if (is_unnamed) {
y_previous = &y_regs[i];
}
return true;
}
const yankreg_T *op_reg_get(const char name)
{
int i = op_reg_index(name);
if (i == -1) {
return NULL;
}
return &y_regs[i];
}
bool op_reg_set_previous(const char name)
FUNC_ATTR_WARN_UNUSED_RESULT
{
int i = op_reg_index(name);
if (i == -1) {
return false;
}
y_previous = &y_regs[i];
return true;
}
