#include "vterm_internal.h"
#include <stdio.h>
#include <string.h>
#define strneq(a,b,n) (strncmp(a,b,n)==0)
#if defined(DEBUG) && DEBUG > 1
#define DEBUG_GLYPH_COMBINE
#endif
static int on_resize(int rows, int cols, void *user);
static void putglyph(VTermState *state, const uint32_t chars[], int width, VTermPos pos)
{
VTermGlyphInfo info;
info.chars = chars;
info.width = width;
info.protected_cell = state->protected_cell;
info.dwl = state->lineinfo[pos.row].doublewidth;
info.dhl = state->lineinfo[pos.row].doubleheight;
if(state->callbacks && state->callbacks->putglyph)
if((*state->callbacks->putglyph)(&info, pos, state->cbdata))
return;
DEBUG_LOG3("libvterm: Unhandled putglyph U+%04x at (%d,%d)\n", chars[0], pos.col, pos.row);
}
static void updatecursor(VTermState *state, VTermPos *oldpos, int cancel_phantom)
{
if(state->pos.col == oldpos->col && state->pos.row == oldpos->row)
return;
if(cancel_phantom)
state->at_phantom = 0;
if(state->callbacks && state->callbacks->movecursor)
if((*state->callbacks->movecursor)(state->pos, *oldpos, state->mode.cursor_visible, state->cbdata))
return;
}
static void erase(VTermState *state, VTermRect rect, int selective)
{
if(state->callbacks && state->callbacks->erase)
if((*state->callbacks->erase)(rect, selective, state->cbdata))
return;
}
static VTermState *vterm_state_new(VTerm *vt)
{
VTermState *state = vterm_allocator_malloc(vt, sizeof(VTermState));
if (state == NULL)
return NULL;
state->vt = vt;
state->rows = vt->rows;
state->cols = vt->cols;
state->mouse_col = 0;
state->mouse_row = 0;
state->mouse_buttons = 0;
state->mouse_protocol = MOUSE_X10;
state->callbacks = NULL;
state->cbdata = NULL;
vterm_state_newpen(state);
state->bold_is_highbright = 0;
return state;
}
INTERNAL void vterm_state_free(VTermState *state)
{
vterm_allocator_free(state->vt, state->tabstops);
vterm_allocator_free(state->vt, state->lineinfo);
vterm_allocator_free(state->vt, state->combine_chars);
vterm_allocator_free(state->vt, state);
}
static void scroll(VTermState *state, VTermRect rect, int downward, int rightward)
{
int rows;
int cols;
if(!downward && !rightward)
return;
rows = rect.end_row - rect.start_row;
if(downward > rows)
downward = rows;
else if(downward < -rows)
downward = -rows;
cols = rect.end_col - rect.start_col;
if(rightward > cols)
rightward = cols;
else if(rightward < -cols)
rightward = -cols;
if(rect.start_col == 0 && rect.end_col == state->cols && rightward == 0) {
int height = rect.end_row - rect.start_row - abs(downward);
if(downward > 0)
memmove(state->lineinfo + rect.start_row,
state->lineinfo + rect.start_row + downward,
height * sizeof(state->lineinfo[0]));
else
memmove(state->lineinfo + rect.start_row - downward,
state->lineinfo + rect.start_row,
height * sizeof(state->lineinfo[0]));
}
if(state->callbacks && state->callbacks->scrollrect)
if((*state->callbacks->scrollrect)(rect, downward, rightward, state->cbdata))
return;
if(state->callbacks)
vterm_scroll_rect(rect, downward, rightward,
state->callbacks->moverect, state->callbacks->erase, state->cbdata);
}
static void linefeed(VTermState *state)
{
if(state->pos.row == SCROLLREGION_BOTTOM(state) - 1) {
VTermRect rect;
rect.start_row = state->scrollregion_top;
rect.end_row = SCROLLREGION_BOTTOM(state);
rect.start_col = SCROLLREGION_LEFT(state);
rect.end_col = SCROLLREGION_RIGHT(state);
scroll(state, rect, 1, 0);
}
else if(state->pos.row < state->rows-1)
state->pos.row++;
}
static void grow_combine_buffer(VTermState *state)
{
size_t new_size = state->combine_chars_size * 2;
uint32_t *new_chars = vterm_allocator_malloc(state->vt, new_size * sizeof(new_chars[0]));
memcpy(new_chars, state->combine_chars, state->combine_chars_size * sizeof(new_chars[0]));
vterm_allocator_free(state->vt, state->combine_chars);
state->combine_chars = new_chars;
state->combine_chars_size = new_size;
}
static void set_col_tabstop(VTermState *state, int col)
{
unsigned char mask = 1 << (col & 7);
state->tabstops[col >> 3] |= mask;
}
static void clear_col_tabstop(VTermState *state, int col)
{
unsigned char mask = 1 << (col & 7);
state->tabstops[col >> 3] &= ~mask;
}
static int is_col_tabstop(VTermState *state, int col)
{
unsigned char mask = 1 << (col & 7);
return state->tabstops[col >> 3] & mask;
}
static int is_cursor_in_scrollregion(const VTermState *state)
{
if(state->pos.row < state->scrollregion_top ||
state->pos.row >= SCROLLREGION_BOTTOM(state))
return 0;
if(state->pos.col < SCROLLREGION_LEFT(state) ||
state->pos.col >= SCROLLREGION_RIGHT(state))
return 0;
return 1;
}
static void tab(VTermState *state, int count, int direction)
{
while(count > 0) {
if(direction > 0) {
if(state->pos.col >= THISROWWIDTH(state)-1)
return;
state->pos.col++;
}
else if(direction < 0) {
if(state->pos.col < 1)
return;
state->pos.col--;
}
if(is_col_tabstop(state, state->pos.col))
count--;
}
}
#define NO_FORCE 0
#define FORCE 1
#define DWL_OFF 0
#define DWL_ON 1
#define DHL_OFF 0
#define DHL_TOP 1
#define DHL_BOTTOM 2
static void set_lineinfo(VTermState *state, int row, int force, int dwl, int dhl)
{
VTermLineInfo info = state->lineinfo[row];
if(dwl == DWL_OFF)
info.doublewidth = DWL_OFF;
else if(dwl == DWL_ON)
info.doublewidth = DWL_ON;
if(dhl == DHL_OFF)
info.doubleheight = DHL_OFF;
else if(dhl == DHL_TOP)
info.doubleheight = DHL_TOP;
else if(dhl == DHL_BOTTOM)
info.doubleheight = DHL_BOTTOM;
if((state->callbacks &&
state->callbacks->setlineinfo &&
(*state->callbacks->setlineinfo)(row, &info, state->lineinfo + row, state->cbdata))
|| force)
state->lineinfo[row] = info;
}
static int on_text(const char bytes[], size_t len, void *user)
{
VTermState *state = user;
uint32_t *codepoints;
int npoints = 0;
size_t eaten = 0;
VTermEncodingInstance *encoding;
int i = 0;
VTermPos oldpos = state->pos;
codepoints = vterm_allocator_malloc(state->vt, (len + 1) * sizeof(uint32_t));
if (codepoints == NULL)
return 0;
encoding =
state->gsingle_set ? &state->encoding[state->gsingle_set] :
!(bytes[eaten] & 0x80) ? &state->encoding[state->gl_set] :
state->vt->mode.utf8 ? &state->encoding_utf8 :
&state->encoding[state->gr_set];
(*encoding->enc->decode)(encoding->enc, encoding->data,
codepoints, &npoints, state->gsingle_set ? 1 : (int)len,
bytes, &eaten, len);
if(!npoints)
{
vterm_allocator_free(state->vt, codepoints);
return (int)eaten;
}
if(state->gsingle_set && npoints)
state->gsingle_set = 0;
if(vterm_unicode_is_combining(codepoints[i])) {
if(state->pos.row == state->combine_pos.row && state->pos.col == state->combine_pos.col + state->combine_width) {
#if defined(DEBUG_GLYPH_COMBINE)
int printpos;
printf("DEBUG: COMBINING SPLIT GLYPH of chars {");
for(printpos = 0; state->combine_chars[printpos]; printpos++)
printf("U+%04x ", state->combine_chars[printpos]);
printf("} + {");
#endif
int saved_i = 0;
while(state->combine_chars[saved_i])
saved_i++;
while(i < npoints && vterm_unicode_is_combining(codepoints[i])) {
if(saved_i >= (int)state->combine_chars_size)
grow_combine_buffer(state);
state->combine_chars[saved_i++] = codepoints[i++];
}
if(saved_i >= (int)state->combine_chars_size)
grow_combine_buffer(state);
state->combine_chars[saved_i] = 0;
#if defined(DEBUG_GLYPH_COMBINE)
for(; state->combine_chars[printpos]; printpos++)
printf("U+%04x ", state->combine_chars[printpos]);
printf("}\n");
#endif
putglyph(state, state->combine_chars, state->combine_width, state->combine_pos);
}
else {
DEBUG_LOG("libvterm: TODO: Skip over split char+combining\n");
}
}
for(; i < npoints; i++) {
int glyph_starts = i;
int glyph_ends;
int width = 0;
uint32_t *chars;
for(glyph_ends = i + 1; glyph_ends < npoints; glyph_ends++)
if(!vterm_unicode_is_combining(codepoints[glyph_ends]))
break;
chars = vterm_allocator_malloc(state->vt, (glyph_ends - glyph_starts + 1) * sizeof(uint32_t));
if (chars == NULL)
break;
for( ; i < glyph_ends; i++) {
int this_width;
if(vterm_get_special_pty_type() == 2) {
state->vt->in_backspace -= (state->vt->in_backspace > 0) ? 1 : 0;
if(state->vt->in_backspace == 1)
codepoints[i] = 0; 
}
chars[i - glyph_starts] = codepoints[i];
this_width = vterm_unicode_width(codepoints[i]);
#if defined(DEBUG)
if(this_width < 0) {
fprintf(stderr, "Text with negative-width codepoint U+%04x\n", codepoints[i]);
abort();
}
#endif
width += this_width;
}
chars[glyph_ends - glyph_starts] = 0;
i--;
#if defined(DEBUG_GLYPH_COMBINE)
int printpos;
printf("DEBUG: COMBINED GLYPH of %d chars {", glyph_ends - glyph_starts);
for(printpos = 0; printpos < glyph_ends - glyph_starts; printpos++)
printf("U+%04x ", chars[printpos]);
printf("}, onscreen width %d\n", width);
#endif
if(state->at_phantom || state->pos.col + width > THISROWWIDTH(state)) {
linefeed(state);
state->pos.col = 0;
state->at_phantom = 0;
}
if(state->mode.insert) {
VTermRect rect;
rect.start_row = state->pos.row;
rect.end_row = state->pos.row + 1;
rect.start_col = state->pos.col;
rect.end_col = THISROWWIDTH(state);
scroll(state, rect, 0, -1);
}
putglyph(state, chars, width, state->pos);
if(i == npoints - 1) {
int save_i;
for(save_i = 0; chars[save_i]; save_i++) {
if(save_i >= (int)state->combine_chars_size)
grow_combine_buffer(state);
state->combine_chars[save_i] = chars[save_i];
}
if(save_i >= (int)state->combine_chars_size)
grow_combine_buffer(state);
state->combine_chars[save_i] = 0;
state->combine_width = width;
state->combine_pos = state->pos;
}
if(state->pos.col + width >= THISROWWIDTH(state)) {
if(state->mode.autowrap)
state->at_phantom = 1;
}
else {
state->pos.col += width;
}
vterm_allocator_free(state->vt, chars);
}
updatecursor(state, &oldpos, 0);
#if defined(DEBUG)
if(state->pos.row < 0 || state->pos.row >= state->rows ||
state->pos.col < 0 || state->pos.col >= state->cols) {
fprintf(stderr, "Position out of bounds after text: (%d,%d)\n",
state->pos.row, state->pos.col);
abort();
}
#endif
vterm_allocator_free(state->vt, codepoints);
return (int)eaten;
}
static int on_control(unsigned char control, void *user)
{
VTermState *state = user;
VTermPos oldpos = state->pos;
VTermScreenCell cell;
VTermPos leadpos = state->pos;
leadpos.col -= (leadpos.col >= 2 ? 2 : 0);
switch(control) {
case 0x07: 
if(state->callbacks && state->callbacks->bell)
(*state->callbacks->bell)(state->cbdata);
break;
case 0x08: 
if(state->pos.col > 0)
state->pos.col--;
if(vterm_get_special_pty_type() == 2) {
vterm_screen_get_cell(state->vt->screen, leadpos, &cell);
if(vterm_unicode_width(cell.chars[0]) == 2)
state->pos.col--;
}
break;
case 0x09: 
tab(state, 1, +1);
break;
case 0x0a: 
case 0x0b: 
case 0x0c: 
linefeed(state);
if(state->mode.newline)
state->pos.col = 0;
break;
case 0x0d: 
state->pos.col = 0;
break;
case 0x0e: 
state->gl_set = 1;
break;
case 0x0f: 
state->gl_set = 0;
break;
case 0x84: 
linefeed(state);
break;
case 0x85: 
linefeed(state);
state->pos.col = 0;
break;
case 0x88: 
set_col_tabstop(state, state->pos.col);
break;
case 0x8d: 
if(state->pos.row == state->scrollregion_top) {
VTermRect rect;
rect.start_row = state->scrollregion_top;
rect.end_row = SCROLLREGION_BOTTOM(state);
rect.start_col = SCROLLREGION_LEFT(state);
rect.end_col = SCROLLREGION_RIGHT(state);
scroll(state, rect, -1, 0);
}
else if(state->pos.row > 0)
state->pos.row--;
break;
case 0x8e: 
state->gsingle_set = 2;
break;
case 0x8f: 
state->gsingle_set = 3;
break;
default:
if(state->fallbacks && state->fallbacks->control)
if((*state->fallbacks->control)(control, state->fbdata))
return 1;
return 0;
}
updatecursor(state, &oldpos, 1);
#if defined(DEBUG)
if(state->pos.row < 0 || state->pos.row >= state->rows ||
state->pos.col < 0 || state->pos.col >= state->cols) {
fprintf(stderr, "Position out of bounds after Ctrl %02x: (%d,%d)\n",
control, state->pos.row, state->pos.col);
abort();
}
#endif
return 1;
}
static int settermprop_bool(VTermState *state, VTermProp prop, int v)
{
VTermValue val;
val.boolean = v;
return vterm_state_set_termprop(state, prop, &val);
}
static int settermprop_int(VTermState *state, VTermProp prop, int v)
{
VTermValue val;
val.number = v;
return vterm_state_set_termprop(state, prop, &val);
}
static int settermprop_string(VTermState *state, VTermProp prop, const char *str, size_t len)
{
char *strvalue;
int r;
VTermValue val;
strvalue = vterm_allocator_malloc(state->vt, (len+1) * sizeof(char));
strncpy(strvalue, str, len);
strvalue[len] = 0;
val.string = strvalue;
r = vterm_state_set_termprop(state, prop, &val);
vterm_allocator_free(state->vt, strvalue);
return r;
}
static void savecursor(VTermState *state, int save)
{
if(save) {
state->saved.pos = state->pos;
state->saved.mode.cursor_visible = state->mode.cursor_visible;
state->saved.mode.cursor_blink = state->mode.cursor_blink;
state->saved.mode.cursor_shape = state->mode.cursor_shape;
vterm_state_savepen(state, 1);
}
else {
VTermPos oldpos = state->pos;
state->pos = state->saved.pos;
settermprop_bool(state, VTERM_PROP_CURSORVISIBLE, state->saved.mode.cursor_visible);
settermprop_bool(state, VTERM_PROP_CURSORBLINK, state->saved.mode.cursor_blink);
settermprop_int (state, VTERM_PROP_CURSORSHAPE, state->saved.mode.cursor_shape);
vterm_state_savepen(state, 0);
updatecursor(state, &oldpos, 1);
}
}
static int on_escape(const char *bytes, size_t len, void *user)
{
VTermState *state = user;
switch(bytes[0]) {
case ' ':
if(len != 2)
return 0;
switch(bytes[1]) {
case 'F': 
state->vt->mode.ctrl8bit = 0;
break;
case 'G': 
state->vt->mode.ctrl8bit = 1;
break;
default:
return 0;
}
return 2;
case '#':
if(len != 2)
return 0;
switch(bytes[1]) {
case '3': 
if(state->mode.leftrightmargin)
break;
set_lineinfo(state, state->pos.row, NO_FORCE, DWL_ON, DHL_TOP);
break;
case '4': 
if(state->mode.leftrightmargin)
break;
set_lineinfo(state, state->pos.row, NO_FORCE, DWL_ON, DHL_BOTTOM);
break;
case '5': 
if(state->mode.leftrightmargin)
break;
set_lineinfo(state, state->pos.row, NO_FORCE, DWL_OFF, DHL_OFF);
break;
case '6': 
if(state->mode.leftrightmargin)
break;
set_lineinfo(state, state->pos.row, NO_FORCE, DWL_ON, DHL_OFF);
break;
case '8': 
{
VTermPos pos;
uint32_t E[] = { 'E', 0 };
for(pos.row = 0; pos.row < state->rows; pos.row++)
for(pos.col = 0; pos.col < ROWWIDTH(state, pos.row); pos.col++)
putglyph(state, E, 1, pos);
break;
}
default:
return 0;
}
return 2;
case '(': case ')': case '*': case '+': 
if(len != 2)
return 0;
{
int setnum = bytes[0] - 0x28;
VTermEncoding *newenc = vterm_lookup_encoding(ENC_SINGLE_94, bytes[1]);
if(newenc) {
state->encoding[setnum].enc = newenc;
if(newenc->init)
(*newenc->init)(newenc, state->encoding[setnum].data);
}
}
return 2;
case '7': 
savecursor(state, 1);
return 1;
case '8': 
savecursor(state, 0);
return 1;
case '<': 
return 1;
case '=': 
state->mode.keypad = 1;
return 1;
case '>': 
state->mode.keypad = 0;
return 1;
case 'c': 
{
VTermPos oldpos = state->pos;
vterm_state_reset(state, 1);
if(state->callbacks && state->callbacks->movecursor)
(*state->callbacks->movecursor)(state->pos, oldpos, state->mode.cursor_visible, state->cbdata);
return 1;
}
case 'n': 
state->gl_set = 2;
return 1;
case 'o': 
state->gl_set = 3;
return 1;
case '~': 
state->gr_set = 1;
return 1;
case '}': 
state->gr_set = 2;
return 1;
case '|': 
state->gr_set = 3;
return 1;
default:
return 0;
}
}
static void set_mode(VTermState *state, int num, int val)
{
switch(num) {
case 4: 
state->mode.insert = val;
break;
case 20: 
state->mode.newline = val;
break;
default:
DEBUG_LOG1("libvterm: Unknown mode %d\n", num);
return;
}
}
static void set_dec_mode(VTermState *state, int num, int val)
{
switch(num) {
case 1:
state->mode.cursor = val;
break;
case 5: 
settermprop_bool(state, VTERM_PROP_REVERSE, val);
break;
case 6: 
{
VTermPos oldpos = state->pos;
state->mode.origin = val;
state->pos.row = state->mode.origin ? state->scrollregion_top : 0;
state->pos.col = state->mode.origin ? SCROLLREGION_LEFT(state) : 0;
updatecursor(state, &oldpos, 1);
}
break;
case 7:
state->mode.autowrap = val;
break;
case 12:
settermprop_bool(state, VTERM_PROP_CURSORBLINK, val);
break;
case 25:
settermprop_bool(state, VTERM_PROP_CURSORVISIBLE, val);
break;
case 69: 
state->mode.leftrightmargin = val;
if(val) {
int row;
for(row = 0; row < state->rows; row++)
set_lineinfo(state, row, FORCE, DWL_OFF, DHL_OFF);
}
break;
case 1000:
case 1002:
case 1003:
settermprop_int(state, VTERM_PROP_MOUSE,
!val ? VTERM_PROP_MOUSE_NONE :
(num == 1000) ? VTERM_PROP_MOUSE_CLICK :
(num == 1002) ? VTERM_PROP_MOUSE_DRAG :
VTERM_PROP_MOUSE_MOVE);
break;
case 1004:
state->mode.report_focus = val;
break;
case 1005:
state->mouse_protocol = val ? MOUSE_UTF8 : MOUSE_X10;
break;
case 1006:
state->mouse_protocol = val ? MOUSE_SGR : MOUSE_X10;
break;
case 1015:
state->mouse_protocol = val ? MOUSE_RXVT : MOUSE_X10;
break;
case 1047:
settermprop_bool(state, VTERM_PROP_ALTSCREEN, val);
break;
case 1048:
savecursor(state, val);
break;
case 1049:
settermprop_bool(state, VTERM_PROP_ALTSCREEN, val);
savecursor(state, val);
break;
case 2004:
state->mode.bracketpaste = val;
break;
default:
DEBUG_LOG1("libvterm: Unknown DEC mode %d\n", num);
return;
}
}
static void request_dec_mode(VTermState *state, int num)
{
int reply;
switch(num) {
case 1:
reply = state->mode.cursor;
break;
case 5:
reply = state->mode.screen;
break;
case 6:
reply = state->mode.origin;
break;
case 7:
reply = state->mode.autowrap;
break;
case 12:
reply = state->mode.cursor_blink;
break;
case 25:
reply = state->mode.cursor_visible;
break;
case 69:
reply = state->mode.leftrightmargin;
break;
case 1000:
reply = state->mouse_flags == MOUSE_WANT_CLICK;
break;
case 1002:
reply = state->mouse_flags == (MOUSE_WANT_CLICK|MOUSE_WANT_DRAG);
break;
case 1003:
reply = state->mouse_flags == (MOUSE_WANT_CLICK|MOUSE_WANT_MOVE);
break;
case 1004:
reply = state->mode.report_focus;
break;
case 1005:
reply = state->mouse_protocol == MOUSE_UTF8;
break;
case 1006:
reply = state->mouse_protocol == MOUSE_SGR;
break;
case 1015:
reply = state->mouse_protocol == MOUSE_RXVT;
break;
case 1047:
reply = state->mode.alt_screen;
break;
case 2004:
reply = state->mode.bracketpaste;
break;
default:
vterm_push_output_sprintf_ctrl(state->vt, C1_CSI, "?%d;%d$y", num, 0);
return;
}
vterm_push_output_sprintf_ctrl(state->vt, C1_CSI, "?%d;%d$y", num, reply ? 1 : 2);
}
static int on_csi(const char *leader, const long args[], int argcount, const char *intermed, char command, void *user)
{
VTermState *state = user;
int leader_byte = 0;
int intermed_byte = 0;
VTermPos oldpos = state->pos;
int handled = 1;
int count, val;
int row, col;
VTermRect rect;
int selective;
if(leader && leader[0]) {
if(leader[1]) 
return 0;
switch(leader[0]) {
case '?':
case '>':
leader_byte = leader[0];
break;
default:
return 0;
}
}
if(intermed && intermed[0]) {
if(intermed[1]) 
return 0;
switch(intermed[0]) {
case ' ':
case '"':
case '$':
case '\'':
intermed_byte = intermed[0];
break;
default:
return 0;
}
}
oldpos = state->pos;
#define LBOUND(v,min) if((v) < (min)) (v) = (min)
#define UBOUND(v,max) if((v) > (max)) (v) = (max)
#define LEADER(l,b) ((l << 8) | b)
#define INTERMED(i,b) ((i << 16) | b)
switch(intermed_byte << 16 | leader_byte << 8 | command) {
case 0x40: 
count = CSI_ARG_COUNT(args[0]);
if(!is_cursor_in_scrollregion(state))
break;
rect.start_row = state->pos.row;
rect.end_row = state->pos.row + 1;
rect.start_col = state->pos.col;
if(state->mode.leftrightmargin)
rect.end_col = SCROLLREGION_RIGHT(state);
else
rect.end_col = THISROWWIDTH(state);
scroll(state, rect, 0, -count);
break;
case 0x41: 
count = CSI_ARG_COUNT(args[0]);
state->pos.row -= count;
state->at_phantom = 0;
break;
case 0x42: 
count = CSI_ARG_COUNT(args[0]);
state->pos.row += count;
state->at_phantom = 0;
break;
case 0x43: 
count = CSI_ARG_COUNT(args[0]);
state->pos.col += count;
state->at_phantom = 0;
break;
case 0x44: 
count = CSI_ARG_COUNT(args[0]);
state->pos.col -= count;
state->at_phantom = 0;
break;
case 0x45: 
count = CSI_ARG_COUNT(args[0]);
state->pos.col = 0;
state->pos.row += count;
state->at_phantom = 0;
break;
case 0x46: 
count = CSI_ARG_COUNT(args[0]);
state->pos.col = 0;
state->pos.row -= count;
state->at_phantom = 0;
break;
case 0x47: 
val = CSI_ARG_OR(args[0], 1);
state->pos.col = val-1;
state->at_phantom = 0;
break;
case 0x48: 
row = CSI_ARG_OR(args[0], 1);
col = argcount < 2 || CSI_ARG_IS_MISSING(args[1]) ? 1 : CSI_ARG(args[1]);
if(vterm_get_special_pty_type() == 2) {
if(state->pos.row == row - 1) {
int cnt, ptr = 0;
for(cnt = 0; cnt < col - 1; ++cnt) {
VTermPos p;
VTermScreenCell c0, c1;
p.row = row - 1;
p.col = ptr;
vterm_screen_get_cell(state->vt->screen, p, &c0);
p.col++;
vterm_screen_get_cell(state->vt->screen, p, &c1);
ptr += (c1.chars[0] == (uint32_t)-1) 
? (vterm_unicode_is_ambiguous(c0.chars[0])) 
? vterm_unicode_width(0x00a1) : 1 
: 1; 
}
col = ptr + 1;
}
}
state->pos.row = row-1;
state->pos.col = col-1;
if(state->mode.origin) {
state->pos.row += state->scrollregion_top;
state->pos.col += SCROLLREGION_LEFT(state);
}
state->at_phantom = 0;
break;
case 0x49: 
count = CSI_ARG_COUNT(args[0]);
tab(state, count, +1);
break;
case 0x4a: 
case LEADER('?', 0x4a): 
selective = (leader_byte == '?');
switch(CSI_ARG(args[0])) {
case CSI_ARG_MISSING:
case 0:
rect.start_row = state->pos.row; rect.end_row = state->pos.row + 1;
rect.start_col = state->pos.col; rect.end_col = state->cols;
if(rect.end_col > rect.start_col)
erase(state, rect, selective);
rect.start_row = state->pos.row + 1; rect.end_row = state->rows;
rect.start_col = 0;
for(row = rect.start_row; row < rect.end_row; row++)
set_lineinfo(state, row, FORCE, DWL_OFF, DHL_OFF);
if(rect.end_row > rect.start_row)
erase(state, rect, selective);
break;
case 1:
rect.start_row = 0; rect.end_row = state->pos.row;
rect.start_col = 0; rect.end_col = state->cols;
for(row = rect.start_row; row < rect.end_row; row++)
set_lineinfo(state, row, FORCE, DWL_OFF, DHL_OFF);
if(rect.end_col > rect.start_col)
erase(state, rect, selective);
rect.start_row = state->pos.row; rect.end_row = state->pos.row + 1;
rect.end_col = state->pos.col + 1;
if(rect.end_row > rect.start_row)
erase(state, rect, selective);
break;
case 2:
rect.start_row = 0; rect.end_row = state->rows;
rect.start_col = 0; rect.end_col = state->cols;
for(row = rect.start_row; row < rect.end_row; row++)
set_lineinfo(state, row, FORCE, DWL_OFF, DHL_OFF);
erase(state, rect, selective);
break;
}
break;
case 0x4b: 
case LEADER('?', 0x4b): 
selective = (leader_byte == '?');
rect.start_row = state->pos.row;
rect.end_row = state->pos.row + 1;
switch(CSI_ARG(args[0])) {
case CSI_ARG_MISSING:
case 0:
rect.start_col = state->pos.col; rect.end_col = THISROWWIDTH(state); break;
case 1:
rect.start_col = 0; rect.end_col = state->pos.col + 1; break;
case 2:
rect.start_col = 0; rect.end_col = THISROWWIDTH(state); break;
default:
return 0;
}
if(rect.end_col > rect.start_col)
erase(state, rect, selective);
break;
case 0x4c: 
count = CSI_ARG_COUNT(args[0]);
if(!is_cursor_in_scrollregion(state))
break;
rect.start_row = state->pos.row;
rect.end_row = SCROLLREGION_BOTTOM(state);
rect.start_col = SCROLLREGION_LEFT(state);
rect.end_col = SCROLLREGION_RIGHT(state);
scroll(state, rect, -count, 0);
break;
case 0x4d: 
count = CSI_ARG_COUNT(args[0]);
if(!is_cursor_in_scrollregion(state))
break;
rect.start_row = state->pos.row;
rect.end_row = SCROLLREGION_BOTTOM(state);
rect.start_col = SCROLLREGION_LEFT(state);
rect.end_col = SCROLLREGION_RIGHT(state);
scroll(state, rect, count, 0);
break;
case 0x50: 
count = CSI_ARG_COUNT(args[0]);
if(!is_cursor_in_scrollregion(state))
break;
rect.start_row = state->pos.row;
rect.end_row = state->pos.row + 1;
rect.start_col = state->pos.col;
if(state->mode.leftrightmargin)
rect.end_col = SCROLLREGION_RIGHT(state);
else
rect.end_col = THISROWWIDTH(state);
scroll(state, rect, 0, count);
break;
case 0x53: 
count = CSI_ARG_COUNT(args[0]);
rect.start_row = state->scrollregion_top;
rect.end_row = SCROLLREGION_BOTTOM(state);
rect.start_col = SCROLLREGION_LEFT(state);
rect.end_col = SCROLLREGION_RIGHT(state);
scroll(state, rect, count, 0);
break;
case 0x54: 
count = CSI_ARG_COUNT(args[0]);
rect.start_row = state->scrollregion_top;
rect.end_row = SCROLLREGION_BOTTOM(state);
rect.start_col = SCROLLREGION_LEFT(state);
rect.end_col = SCROLLREGION_RIGHT(state);
scroll(state, rect, -count, 0);
break;
case 0x58: 
count = CSI_ARG_COUNT(args[0]);
rect.start_row = state->pos.row;
rect.end_row = state->pos.row + 1;
rect.start_col = state->pos.col;
rect.end_col = state->pos.col + count;
UBOUND(rect.end_col, THISROWWIDTH(state));
erase(state, rect, 0);
break;
case 0x5a: 
count = CSI_ARG_COUNT(args[0]);
tab(state, count, -1);
break;
case 0x60: 
col = CSI_ARG_OR(args[0], 1);
state->pos.col = col-1;
state->at_phantom = 0;
break;
case 0x61: 
count = CSI_ARG_COUNT(args[0]);
state->pos.col += count;
state->at_phantom = 0;
break;
case 0x63: 
val = CSI_ARG_OR(args[0], 0);
if(val == 0)
vterm_push_output_sprintf_ctrl(state->vt, C1_CSI, "?1;2c");
break;
case LEADER('>', 0x63): 
vterm_push_output_sprintf_ctrl(state->vt, C1_CSI, ">%d;%d;%dc", 0, 100, 0);
break;
case 0x64: 
row = CSI_ARG_OR(args[0], 1);
state->pos.row = row-1;
if(state->mode.origin)
state->pos.row += state->scrollregion_top;
state->at_phantom = 0;
break;
case 0x65: 
count = CSI_ARG_COUNT(args[0]);
state->pos.row += count;
state->at_phantom = 0;
break;
case 0x66: 
row = CSI_ARG_OR(args[0], 1);
col = argcount < 2 || CSI_ARG_IS_MISSING(args[1]) ? 1 : CSI_ARG(args[1]);
state->pos.row = row-1;
state->pos.col = col-1;
if(state->mode.origin) {
state->pos.row += state->scrollregion_top;
state->pos.col += SCROLLREGION_LEFT(state);
}
state->at_phantom = 0;
break;
case 0x67: 
val = CSI_ARG_OR(args[0], 0);
switch(val) {
case 0:
clear_col_tabstop(state, state->pos.col);
break;
case 3:
case 5:
for(col = 0; col < state->cols; col++)
clear_col_tabstop(state, col);
break;
case 1:
case 2:
case 4:
break;
default:
return 0;
}
break;
case 0x68: 
if(!CSI_ARG_IS_MISSING(args[0]))
set_mode(state, CSI_ARG(args[0]), 1);
break;
case LEADER('?', 0x68): 
if(!CSI_ARG_IS_MISSING(args[0]))
set_dec_mode(state, CSI_ARG(args[0]), 1);
break;
case 0x6a: 
count = CSI_ARG_COUNT(args[0]);
state->pos.col -= count;
state->at_phantom = 0;
break;
case 0x6b: 
count = CSI_ARG_COUNT(args[0]);
state->pos.row -= count;
state->at_phantom = 0;
break;
case 0x6c: 
if(!CSI_ARG_IS_MISSING(args[0]))
set_mode(state, CSI_ARG(args[0]), 0);
break;
case LEADER('?', 0x6c): 
if(!CSI_ARG_IS_MISSING(args[0]))
set_dec_mode(state, CSI_ARG(args[0]), 0);
break;
case 0x6d: 
vterm_state_setpen(state, args, argcount);
break;
case LEADER('>', 0x6d): 
if (argcount == 2 && args[0] == 4)
state->mode.modify_other_keys = args[1] == 2;
break;
case 0x6e: 
case LEADER('?', 0x6e): 
val = CSI_ARG_OR(args[0], 0);
{
char *qmark = (leader_byte == '?') ? "?" : "";
switch(val) {
case 0: case 1: case 2: case 3: case 4:
break;
case 5:
vterm_push_output_sprintf_ctrl(state->vt, C1_CSI, "%s0n", qmark);
break;
case 6: 
vterm_push_output_sprintf_ctrl(state->vt, C1_CSI, "%s%d;%dR", qmark, state->pos.row + 1, state->pos.col + 1);
break;
}
}
break;
case LEADER('!', 0x70): 
vterm_state_reset(state, 0);
break;
case LEADER('?', INTERMED('$', 0x70)):
request_dec_mode(state, CSI_ARG(args[0]));
break;
case INTERMED(' ', 0x71): 
val = CSI_ARG_OR(args[0], 1);
switch(val) {
case 0: case 1:
settermprop_bool(state, VTERM_PROP_CURSORBLINK, 1);
settermprop_int (state, VTERM_PROP_CURSORSHAPE, VTERM_PROP_CURSORSHAPE_BLOCK);
break;
case 2:
settermprop_bool(state, VTERM_PROP_CURSORBLINK, 0);
settermprop_int (state, VTERM_PROP_CURSORSHAPE, VTERM_PROP_CURSORSHAPE_BLOCK);
break;
case 3:
settermprop_bool(state, VTERM_PROP_CURSORBLINK, 1);
settermprop_int (state, VTERM_PROP_CURSORSHAPE, VTERM_PROP_CURSORSHAPE_UNDERLINE);
break;
case 4:
settermprop_bool(state, VTERM_PROP_CURSORBLINK, 0);
settermprop_int (state, VTERM_PROP_CURSORSHAPE, VTERM_PROP_CURSORSHAPE_UNDERLINE);
break;
case 5:
settermprop_bool(state, VTERM_PROP_CURSORBLINK, 1);
settermprop_int (state, VTERM_PROP_CURSORSHAPE, VTERM_PROP_CURSORSHAPE_BAR_LEFT);
break;
case 6:
settermprop_bool(state, VTERM_PROP_CURSORBLINK, 0);
settermprop_int (state, VTERM_PROP_CURSORSHAPE, VTERM_PROP_CURSORSHAPE_BAR_LEFT);
break;
}
break;
case INTERMED('"', 0x71): 
val = CSI_ARG_OR(args[0], 0);
switch(val) {
case 0: case 2:
state->protected_cell = 0;
break;
case 1:
state->protected_cell = 1;
break;
}
break;
case 0x72: 
state->scrollregion_top = CSI_ARG_OR(args[0], 1) - 1;
state->scrollregion_bottom = argcount < 2 || CSI_ARG_IS_MISSING(args[1]) ? -1 : CSI_ARG(args[1]);
LBOUND(state->scrollregion_top, 0);
UBOUND(state->scrollregion_top, state->rows);
LBOUND(state->scrollregion_bottom, -1);
if(state->scrollregion_top == 0 && state->scrollregion_bottom == state->rows)
state->scrollregion_bottom = -1;
else
UBOUND(state->scrollregion_bottom, state->rows);
if(SCROLLREGION_BOTTOM(state) <= state->scrollregion_top) {
state->scrollregion_top = 0;
state->scrollregion_bottom = -1;
}
break;
case 0x73: 
state->scrollregion_left = CSI_ARG_OR(args[0], 1) - 1;
state->scrollregion_right = argcount < 2 || CSI_ARG_IS_MISSING(args[1]) ? -1 : CSI_ARG(args[1]);
LBOUND(state->scrollregion_left, 0);
UBOUND(state->scrollregion_left, state->cols);
LBOUND(state->scrollregion_right, -1);
if(state->scrollregion_left == 0 && state->scrollregion_right == state->cols)
state->scrollregion_right = -1;
else
UBOUND(state->scrollregion_right, state->cols);
if(state->scrollregion_right > -1 &&
state->scrollregion_right <= state->scrollregion_left) {
state->scrollregion_left = 0;
state->scrollregion_right = -1;
}
break;
case 0x74:
switch(CSI_ARG(args[0])) {
case 8: 
if (argcount == 3)
on_resize(CSI_ARG(args[1]), CSI_ARG(args[2]), state);
break;
default:
handled = 0;
break;
}
break;
case INTERMED('\'', 0x7D): 
count = CSI_ARG_COUNT(args[0]);
if(!is_cursor_in_scrollregion(state))
break;
rect.start_row = state->scrollregion_top;
rect.end_row = SCROLLREGION_BOTTOM(state);
rect.start_col = state->pos.col;
rect.end_col = SCROLLREGION_RIGHT(state);
scroll(state, rect, 0, -count);
break;
case INTERMED('\'', 0x7E): 
count = CSI_ARG_COUNT(args[0]);
if(!is_cursor_in_scrollregion(state))
break;
rect.start_row = state->scrollregion_top;
rect.end_row = SCROLLREGION_BOTTOM(state);
rect.start_col = state->pos.col;
rect.end_col = SCROLLREGION_RIGHT(state);
scroll(state, rect, 0, count);
break;
default:
handled = 0;
break;
}
if (!handled) {
if(state->fallbacks && state->fallbacks->csi)
if((*state->fallbacks->csi)(leader, args, argcount, intermed, command, state->fbdata))
return 1;
return 0;
}
if(state->mode.origin) {
LBOUND(state->pos.row, state->scrollregion_top);
UBOUND(state->pos.row, SCROLLREGION_BOTTOM(state)-1);
LBOUND(state->pos.col, SCROLLREGION_LEFT(state));
UBOUND(state->pos.col, SCROLLREGION_RIGHT(state)-1);
}
else {
LBOUND(state->pos.row, 0);
UBOUND(state->pos.row, state->rows-1);
LBOUND(state->pos.col, 0);
UBOUND(state->pos.col, THISROWWIDTH(state)-1);
}
updatecursor(state, &oldpos, 1);
#if defined(DEBUG)
if(state->pos.row < 0 || state->pos.row >= state->rows ||
state->pos.col < 0 || state->pos.col >= state->cols) {
fprintf(stderr, "Position out of bounds after CSI %c: (%d,%d)\n",
command, state->pos.row, state->pos.col);
abort();
}
if(SCROLLREGION_BOTTOM(state) <= state->scrollregion_top) {
fprintf(stderr, "Scroll region height out of bounds after CSI %c: %d <= %d\n",
command, SCROLLREGION_BOTTOM(state), state->scrollregion_top);
abort();
}
if(SCROLLREGION_RIGHT(state) <= SCROLLREGION_LEFT(state)) {
fprintf(stderr, "Scroll region width out of bounds after CSI %c: %d <= %d\n",
command, SCROLLREGION_RIGHT(state), SCROLLREGION_LEFT(state));
abort();
}
#endif
return 1;
}
static int on_osc(const char *command, size_t cmdlen, void *user)
{
VTermState *state = user;
if(cmdlen < 2)
return 0;
if(strneq(command, "0;", 2)) {
settermprop_string(state, VTERM_PROP_ICONNAME, command + 2, cmdlen - 2);
settermprop_string(state, VTERM_PROP_TITLE, command + 2, cmdlen - 2);
return 1;
}
else if(strneq(command, "1;", 2)) {
settermprop_string(state, VTERM_PROP_ICONNAME, command + 2, cmdlen - 2);
return 1;
}
else if(strneq(command, "2;", 2)) {
settermprop_string(state, VTERM_PROP_TITLE, command + 2, cmdlen - 2);
return 1;
}
else if(strneq(command, "10;", 3)) {
int red = state->default_fg.red;
int blue = state->default_fg.blue;
int green = state->default_fg.green;
vterm_push_output_sprintf_ctrl(state->vt, C1_OSC, "10;rgb:%02x%02x/%02x%02x/%02x%02x\x07", red, red, green, green, blue, blue);
return 1;
}
else if(strneq(command, "11;", 3)) {
int red = state->default_bg.red;
int blue = state->default_bg.blue;
int green = state->default_bg.green;
vterm_push_output_sprintf_ctrl(state->vt, C1_OSC, "11;rgb:%02x%02x/%02x%02x/%02x%02x\x07", red, red, green, green, blue, blue);
return 1;
}
else if(strneq(command, "12;", 3)) {
settermprop_string(state, VTERM_PROP_CURSORCOLOR, command + 3, cmdlen - 3);
return 1;
}
else if(state->fallbacks && state->fallbacks->osc)
if((*state->fallbacks->osc)(command, cmdlen, state->fbdata))
return 1;
return 0;
}
static void request_status_string(VTermState *state, const char *command, size_t cmdlen)
{
if(cmdlen == 1)
switch(command[0]) {
case 'm': 
{
long args[20];
int argc = vterm_state_getpen(state, args, sizeof(args)/sizeof(args[0]));
int argi;
vterm_push_output_sprintf_ctrl(state->vt, C1_DCS, "1$r");
for(argi = 0; argi < argc; argi++)
vterm_push_output_sprintf(state->vt,
argi == argc - 1 ? "%d" :
CSI_ARG_HAS_MORE(args[argi]) ? "%d:" :
"%d;",
CSI_ARG(args[argi]));
vterm_push_output_sprintf(state->vt, "m");
vterm_push_output_sprintf_ctrl(state->vt, C1_ST, "");
}
return;
case 'r': 
vterm_push_output_sprintf_dcs(state->vt, "1$r%d;%dr", state->scrollregion_top+1, SCROLLREGION_BOTTOM(state));
return;
case 's': 
vterm_push_output_sprintf_dcs(state->vt, "1$r%d;%ds", SCROLLREGION_LEFT(state)+1, SCROLLREGION_RIGHT(state));
return;
}
if(cmdlen == 2) {
if(strneq(command, " q", 2)) {
int reply;
switch(state->mode.cursor_shape) {
case VTERM_PROP_CURSORSHAPE_BLOCK: reply = 2; break;
case VTERM_PROP_CURSORSHAPE_UNDERLINE: reply = 4; break;
default: reply = 6; break;
}
if(state->mode.cursor_blink)
reply--;
vterm_push_output_sprintf_dcs(state->vt, "1$r%d q", reply);
return;
}
else if(strneq(command, "\"q", 2)) {
vterm_push_output_sprintf_dcs(state->vt, "1$r%d\"q", state->protected_cell ? 1 : 2);
return;
}
}
vterm_push_output_sprintf_dcs(state->vt, "0$r%.s", (int)cmdlen, command);
}
static int on_dcs(const char *command, size_t cmdlen, void *user)
{
VTermState *state = user;
if(cmdlen >= 2 && strneq(command, "$q", 2)) {
request_status_string(state, command+2, cmdlen-2);
return 1;
}
else if(state->fallbacks && state->fallbacks->dcs)
if((*state->fallbacks->dcs)(command, cmdlen, state->fbdata))
return 1;
return 0;
}
static int on_resize(int rows, int cols, void *user)
{
VTermState *state = user;
VTermPos oldpos = state->pos;
VTermPos delta = { 0, 0 };
if(cols != state->cols) {
int col;
unsigned char *newtabstops = vterm_allocator_malloc(state->vt, (cols + 7) / 8);
if (newtabstops == NULL)
return 0;
for(col = 0; col < state->cols && col < cols; col++) {
unsigned char mask = 1 << (col & 7);
if(state->tabstops[col >> 3] & mask)
newtabstops[col >> 3] |= mask;
else
newtabstops[col >> 3] &= ~mask;
}
for( ; col < cols; col++) {
unsigned char mask = 1 << (col & 7);
if(col % 8 == 0)
newtabstops[col >> 3] |= mask;
else
newtabstops[col >> 3] &= ~mask;
}
vterm_allocator_free(state->vt, state->tabstops);
state->tabstops = newtabstops;
}
if(rows != state->rows) {
int row;
VTermLineInfo *newlineinfo = vterm_allocator_malloc(state->vt, rows * sizeof(VTermLineInfo));
if (newlineinfo == NULL)
return 0;
for(row = 0; row < state->rows && row < rows; row++) {
newlineinfo[row] = state->lineinfo[row];
}
for( ; row < rows; row++) {
newlineinfo[row].doublewidth = 0;
newlineinfo[row].doubleheight = 0;
}
vterm_allocator_free(state->vt, state->lineinfo);
state->lineinfo = newlineinfo;
}
state->rows = rows;
state->cols = cols;
if(state->scrollregion_bottom > -1)
UBOUND(state->scrollregion_bottom, state->rows);
if(state->scrollregion_right > -1)
UBOUND(state->scrollregion_right, state->cols);
if(state->callbacks && state->callbacks->resize)
(*state->callbacks->resize)(rows, cols, &delta, state->cbdata);
if(state->at_phantom && state->pos.col < cols-1) {
state->at_phantom = 0;
state->pos.col++;
}
state->pos.row += delta.row;
state->pos.col += delta.col;
if(state->pos.row >= rows)
state->pos.row = rows - 1;
if(state->pos.col >= cols)
state->pos.col = cols - 1;
updatecursor(state, &oldpos, 1);
return 1;
}
static const VTermParserCallbacks parser_callbacks = {
on_text, 
on_control, 
on_escape, 
on_csi, 
on_osc, 
on_dcs, 
on_resize 
};
VTermState *vterm_obtain_state(VTerm *vt)
{
VTermState *state;
if(vt->state)
return vt->state;
state = vterm_state_new(vt);
if (state == NULL)
return NULL;
vt->state = state;
state->combine_chars_size = 16;
state->combine_chars = vterm_allocator_malloc(state->vt, state->combine_chars_size * sizeof(state->combine_chars[0]));
state->tabstops = vterm_allocator_malloc(state->vt, (state->cols + 7) / 8);
state->lineinfo = vterm_allocator_malloc(state->vt, state->rows * sizeof(VTermLineInfo));
state->encoding_utf8.enc = vterm_lookup_encoding(ENC_UTF8, 'u');
if(*state->encoding_utf8.enc->init != NULL)
(*state->encoding_utf8.enc->init)(state->encoding_utf8.enc, state->encoding_utf8.data);
vterm_parser_set_callbacks(vt, &parser_callbacks, state);
return state;
}
void vterm_state_reset(VTermState *state, int hard)
{
VTermEncoding *default_enc;
state->scrollregion_top = 0;
state->scrollregion_bottom = -1;
state->scrollregion_left = 0;
state->scrollregion_right = -1;
state->mode.keypad = 0;
state->mode.cursor = 0;
state->mode.autowrap = 1;
state->mode.insert = 0;
state->mode.newline = 0;
state->mode.alt_screen = 0;
state->mode.origin = 0;
state->mode.leftrightmargin = 0;
state->mode.bracketpaste = 0;
state->mode.report_focus = 0;
state->vt->mode.ctrl8bit = 0;
{
int col;
for(col = 0; col < state->cols; col++)
if(col % 8 == 0)
set_col_tabstop(state, col);
else
clear_col_tabstop(state, col);
}
{
int row;
for(row = 0; row < state->rows; row++)
set_lineinfo(state, row, FORCE, DWL_OFF, DHL_OFF);
}
if(state->callbacks && state->callbacks->initpen)
(*state->callbacks->initpen)(state->cbdata);
vterm_state_resetpen(state);
default_enc = state->vt->mode.utf8 ?
vterm_lookup_encoding(ENC_UTF8, 'u') :
vterm_lookup_encoding(ENC_SINGLE_94, 'B');
{
int i;
for(i = 0; i < 4; i++) {
state->encoding[i].enc = default_enc;
if(default_enc->init)
(*default_enc->init)(default_enc, state->encoding[i].data);
}
}
state->gl_set = 0;
state->gr_set = 1;
state->gsingle_set = 0;
state->protected_cell = 0;
settermprop_bool(state, VTERM_PROP_CURSORVISIBLE, 1);
settermprop_bool(state, VTERM_PROP_CURSORBLINK, 1);
settermprop_int (state, VTERM_PROP_CURSORSHAPE, VTERM_PROP_CURSORSHAPE_BLOCK);
if(hard) {
VTermRect rect = { 0, 0, 0, 0 };
state->pos.row = 0;
state->pos.col = 0;
state->at_phantom = 0;
rect.end_row = state->rows;
rect.end_col = state->cols;
erase(state, rect, 0);
}
}
void vterm_state_get_cursorpos(const VTermState *state, VTermPos *cursorpos)
{
*cursorpos = state->pos;
}
void vterm_state_get_mousestate(const VTermState *state, VTermMouseState *mousestate)
{
mousestate->pos.col = state->mouse_col;
mousestate->pos.row = state->mouse_row;
mousestate->buttons = state->mouse_buttons;
mousestate->flags = state->mouse_flags;
}
void vterm_state_set_callbacks(VTermState *state, const VTermStateCallbacks *callbacks, void *user)
{
if(callbacks) {
state->callbacks = callbacks;
state->cbdata = user;
if(state->callbacks && state->callbacks->initpen)
(*state->callbacks->initpen)(state->cbdata);
}
else {
state->callbacks = NULL;
state->cbdata = NULL;
}
}
void *vterm_state_get_cbdata(VTermState *state)
{
return state->cbdata;
}
void vterm_state_set_unrecognised_fallbacks(VTermState *state, const VTermParserCallbacks *fallbacks, void *user)
{
if(fallbacks) {
state->fallbacks = fallbacks;
state->fbdata = user;
}
else {
state->fallbacks = NULL;
state->fbdata = NULL;
}
}
void *vterm_state_get_unrecognised_fbdata(VTermState *state)
{
return state->fbdata;
}
int vterm_state_set_termprop(VTermState *state, VTermProp prop, VTermValue *val)
{
if(state->callbacks && state->callbacks->settermprop)
if(!(*state->callbacks->settermprop)(prop, val, state->cbdata))
return 0;
switch(prop) {
case VTERM_PROP_TITLE:
case VTERM_PROP_ICONNAME:
case VTERM_PROP_CURSORCOLOR:
return 1;
case VTERM_PROP_CURSORVISIBLE:
state->mode.cursor_visible = val->boolean;
return 1;
case VTERM_PROP_CURSORBLINK:
state->mode.cursor_blink = val->boolean;
return 1;
case VTERM_PROP_CURSORSHAPE:
state->mode.cursor_shape = val->number;
return 1;
case VTERM_PROP_REVERSE:
state->mode.screen = val->boolean;
return 1;
case VTERM_PROP_ALTSCREEN:
state->mode.alt_screen = val->boolean;
if(state->mode.alt_screen) {
VTermRect rect = {0, 0, 0, 0};
rect.end_row = state->rows;
rect.end_col = state->cols;
erase(state, rect, 0);
}
return 1;
case VTERM_PROP_MOUSE:
state->mouse_flags = 0;
if(val->number)
state->mouse_flags |= MOUSE_WANT_CLICK;
if(val->number == VTERM_PROP_MOUSE_DRAG)
state->mouse_flags |= MOUSE_WANT_DRAG;
if(val->number == VTERM_PROP_MOUSE_MOVE)
state->mouse_flags |= MOUSE_WANT_MOVE;
return 1;
case VTERM_N_PROPS:
return 0;
}
return 0;
}
void vterm_state_focus_in(VTermState *state)
{
if(state->mode.report_focus)
vterm_push_output_sprintf_ctrl(state->vt, C1_CSI, "I");
}
void vterm_state_focus_out(VTermState *state)
{
if(state->mode.report_focus)
vterm_push_output_sprintf_ctrl(state->vt, C1_CSI, "O");
}
const VTermLineInfo *vterm_state_get_lineinfo(const VTermState *state, int row)
{
return state->lineinfo + row;
}
