#include <assert.h>
#include <inttypes.h>
#include <limits.h>
#include "nvim/vim.h"
#include "nvim/ascii.h"
#include "nvim/keymap.h"
#include "nvim/charset.h"
#include "nvim/memory.h"
#include "nvim/edit.h"
#include "nvim/eval.h"
#include "nvim/message.h"
#include "nvim/strings.h"
#include "nvim/mouse.h"
#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "keymap.c.generated.h"
#endif
static const struct modmasktable {
uint16_t mod_mask; 
uint16_t mod_flag; 
char_u name; 
} mod_mask_table[] = {
{ MOD_MASK_ALT, MOD_MASK_ALT, (char_u)'M' },
{ MOD_MASK_META, MOD_MASK_META, (char_u)'T' },
{ MOD_MASK_CTRL, MOD_MASK_CTRL, (char_u)'C' },
{ MOD_MASK_SHIFT, MOD_MASK_SHIFT, (char_u)'S' },
{ MOD_MASK_MULTI_CLICK, MOD_MASK_2CLICK, (char_u)'2' },
{ MOD_MASK_MULTI_CLICK, MOD_MASK_3CLICK, (char_u)'3' },
{ MOD_MASK_MULTI_CLICK, MOD_MASK_4CLICK, (char_u)'4' },
{ MOD_MASK_CMD, MOD_MASK_CMD, (char_u)'D' },
{ MOD_MASK_ALT, MOD_MASK_ALT, (char_u)'A' },
{ 0, 0, NUL }
};
#define MOD_KEYS_ENTRY_SIZE 5
static char_u modifier_keys_table[] =
{
MOD_MASK_SHIFT, '&', '9', '@', '1', 
MOD_MASK_SHIFT, '&', '0', '@', '2', 
MOD_MASK_SHIFT, '*', '1', '@', '4', 
MOD_MASK_SHIFT, '*', '2', '@', '5', 
MOD_MASK_SHIFT, '*', '3', '@', '6', 
MOD_MASK_SHIFT, '*', '4', 'k', 'D', 
MOD_MASK_SHIFT, '*', '5', 'k', 'L', 
MOD_MASK_SHIFT, '*', '7', '@', '7', 
MOD_MASK_CTRL, KS_EXTRA, (int)KE_C_END, '@', '7', 
MOD_MASK_SHIFT, '*', '9', '@', '9', 
MOD_MASK_SHIFT, '*', '0', '@', '0', 
MOD_MASK_SHIFT, '#', '1', '%', '1', 
MOD_MASK_SHIFT, '#', '2', 'k', 'h', 
MOD_MASK_CTRL, KS_EXTRA, (int)KE_C_HOME, 'k', 'h', 
MOD_MASK_SHIFT, '#', '3', 'k', 'I', 
MOD_MASK_SHIFT, '#', '4', 'k', 'l', 
MOD_MASK_CTRL, KS_EXTRA, (int)KE_C_LEFT, 'k', 'l', 
MOD_MASK_SHIFT, '%', 'a', '%', '3', 
MOD_MASK_SHIFT, '%', 'b', '%', '4', 
MOD_MASK_SHIFT, '%', 'c', '%', '5', 
MOD_MASK_SHIFT, '%', 'd', '%', '7', 
MOD_MASK_SHIFT, '%', 'e', '%', '8', 
MOD_MASK_SHIFT, '%', 'f', '%', '9', 
MOD_MASK_SHIFT, '%', 'g', '%', '0', 
MOD_MASK_SHIFT, '%', 'h', '&', '3', 
MOD_MASK_SHIFT, '%', 'i', 'k', 'r', 
MOD_MASK_CTRL, KS_EXTRA, (int)KE_C_RIGHT, 'k', 'r', 
MOD_MASK_SHIFT, '%', 'j', '&', '5', 
MOD_MASK_SHIFT, '!', '1', '&', '6', 
MOD_MASK_SHIFT, '!', '2', '&', '7', 
MOD_MASK_SHIFT, '!', '3', '&', '8', 
MOD_MASK_SHIFT, KS_EXTRA, (int)KE_S_UP, 'k', 'u', 
MOD_MASK_SHIFT, KS_EXTRA, (int)KE_S_DOWN, 'k', 'd', 
MOD_MASK_SHIFT, KS_EXTRA, (int)KE_S_XF1, KS_EXTRA, (int)KE_XF1,
MOD_MASK_SHIFT, KS_EXTRA, (int)KE_S_XF2, KS_EXTRA, (int)KE_XF2,
MOD_MASK_SHIFT, KS_EXTRA, (int)KE_S_XF3, KS_EXTRA, (int)KE_XF3,
MOD_MASK_SHIFT, KS_EXTRA, (int)KE_S_XF4, KS_EXTRA, (int)KE_XF4,
MOD_MASK_SHIFT, KS_EXTRA, (int)KE_S_F1, 'k', '1', 
MOD_MASK_SHIFT, KS_EXTRA, (int)KE_S_F2, 'k', '2',
MOD_MASK_SHIFT, KS_EXTRA, (int)KE_S_F3, 'k', '3',
MOD_MASK_SHIFT, KS_EXTRA, (int)KE_S_F4, 'k', '4',
MOD_MASK_SHIFT, KS_EXTRA, (int)KE_S_F5, 'k', '5',
MOD_MASK_SHIFT, KS_EXTRA, (int)KE_S_F6, 'k', '6',
MOD_MASK_SHIFT, KS_EXTRA, (int)KE_S_F7, 'k', '7',
MOD_MASK_SHIFT, KS_EXTRA, (int)KE_S_F8, 'k', '8',
MOD_MASK_SHIFT, KS_EXTRA, (int)KE_S_F9, 'k', '9',
MOD_MASK_SHIFT, KS_EXTRA, (int)KE_S_F10, 'k', ';', 
MOD_MASK_SHIFT, KS_EXTRA, (int)KE_S_F11, 'F', '1',
MOD_MASK_SHIFT, KS_EXTRA, (int)KE_S_F12, 'F', '2',
MOD_MASK_SHIFT, KS_EXTRA, (int)KE_S_F13, 'F', '3',
MOD_MASK_SHIFT, KS_EXTRA, (int)KE_S_F14, 'F', '4',
MOD_MASK_SHIFT, KS_EXTRA, (int)KE_S_F15, 'F', '5',
MOD_MASK_SHIFT, KS_EXTRA, (int)KE_S_F16, 'F', '6',
MOD_MASK_SHIFT, KS_EXTRA, (int)KE_S_F17, 'F', '7',
MOD_MASK_SHIFT, KS_EXTRA, (int)KE_S_F18, 'F', '8',
MOD_MASK_SHIFT, KS_EXTRA, (int)KE_S_F19, 'F', '9',
MOD_MASK_SHIFT, KS_EXTRA, (int)KE_S_F20, 'F', 'A',
MOD_MASK_SHIFT, KS_EXTRA, (int)KE_S_F21, 'F', 'B',
MOD_MASK_SHIFT, KS_EXTRA, (int)KE_S_F22, 'F', 'C',
MOD_MASK_SHIFT, KS_EXTRA, (int)KE_S_F23, 'F', 'D',
MOD_MASK_SHIFT, KS_EXTRA, (int)KE_S_F24, 'F', 'E',
MOD_MASK_SHIFT, KS_EXTRA, (int)KE_S_F25, 'F', 'F',
MOD_MASK_SHIFT, KS_EXTRA, (int)KE_S_F26, 'F', 'G',
MOD_MASK_SHIFT, KS_EXTRA, (int)KE_S_F27, 'F', 'H',
MOD_MASK_SHIFT, KS_EXTRA, (int)KE_S_F28, 'F', 'I',
MOD_MASK_SHIFT, KS_EXTRA, (int)KE_S_F29, 'F', 'J',
MOD_MASK_SHIFT, KS_EXTRA, (int)KE_S_F30, 'F', 'K',
MOD_MASK_SHIFT, KS_EXTRA, (int)KE_S_F31, 'F', 'L',
MOD_MASK_SHIFT, KS_EXTRA, (int)KE_S_F32, 'F', 'M',
MOD_MASK_SHIFT, KS_EXTRA, (int)KE_S_F33, 'F', 'N',
MOD_MASK_SHIFT, KS_EXTRA, (int)KE_S_F34, 'F', 'O',
MOD_MASK_SHIFT, KS_EXTRA, (int)KE_S_F35, 'F', 'P',
MOD_MASK_SHIFT, KS_EXTRA, (int)KE_S_F36, 'F', 'Q',
MOD_MASK_SHIFT, KS_EXTRA, (int)KE_S_F37, 'F', 'R',
MOD_MASK_SHIFT, 'k', 'B', KS_EXTRA, (int)KE_TAB,
NUL
};
static const struct key_name_entry {
int key; 
const char *name; 
} key_names_table[] = {
{ ' ', "Space" },
{ TAB, "Tab" },
{ K_TAB, "Tab" },
{ NL, "NL" },
{ NL, "NewLine" }, 
{ NL, "LineFeed" }, 
{ NL, "LF" }, 
{ CAR, "CR" },
{ CAR, "Return" }, 
{ CAR, "Enter" }, 
{ K_BS, "BS" },
{ K_BS, "BackSpace" }, 
{ ESC, "Esc" },
{ CSI, "CSI" },
{ K_CSI, "xCSI" },
{ '|', "Bar" },
{ '\\', "Bslash" },
{ K_DEL, "Del" },
{ K_DEL, "Delete" }, 
{ K_KDEL, "kDel" },
{ K_KDEL, "KPPeriod" }, 
{ K_UP, "Up" },
{ K_DOWN, "Down" },
{ K_LEFT, "Left" },
{ K_RIGHT, "Right" },
{ K_XUP, "xUp" },
{ K_XDOWN, "xDown" },
{ K_XLEFT, "xLeft" },
{ K_XRIGHT, "xRight" },
{ K_KUP, "kUp" },
{ K_KUP, "KP8" },
{ K_KDOWN, "kDown" },
{ K_KDOWN, "KP2" },
{ K_KLEFT, "kLeft" },
{ K_KLEFT, "KP4" },
{ K_KRIGHT, "kRight" },
{ K_KRIGHT, "KP6" },
{ K_F1, "F1" },
{ K_F2, "F2" },
{ K_F3, "F3" },
{ K_F4, "F4" },
{ K_F5, "F5" },
{ K_F6, "F6" },
{ K_F7, "F7" },
{ K_F8, "F8" },
{ K_F9, "F9" },
{ K_F10, "F10" },
{ K_F11, "F11" },
{ K_F12, "F12" },
{ K_F13, "F13" },
{ K_F14, "F14" },
{ K_F15, "F15" },
{ K_F16, "F16" },
{ K_F17, "F17" },
{ K_F18, "F18" },
{ K_F19, "F19" },
{ K_F20, "F20" },
{ K_F21, "F21" },
{ K_F22, "F22" },
{ K_F23, "F23" },
{ K_F24, "F24" },
{ K_F25, "F25" },
{ K_F26, "F26" },
{ K_F27, "F27" },
{ K_F28, "F28" },
{ K_F29, "F29" },
{ K_F30, "F30" },
{ K_F31, "F31" },
{ K_F32, "F32" },
{ K_F33, "F33" },
{ K_F34, "F34" },
{ K_F35, "F35" },
{ K_F36, "F36" },
{ K_F37, "F37" },
{ K_XF1, "xF1" },
{ K_XF2, "xF2" },
{ K_XF3, "xF3" },
{ K_XF4, "xF4" },
{ K_HELP, "Help" },
{ K_UNDO, "Undo" },
{ K_INS, "Insert" },
{ K_INS, "Ins" }, 
{ K_KINS, "kInsert" },
{ K_KINS, "KP0" },
{ K_HOME, "Home" },
{ K_KHOME, "kHome" },
{ K_KHOME, "KP7" },
{ K_XHOME, "xHome" },
{ K_ZHOME, "zHome" },
{ K_END, "End" },
{ K_KEND, "kEnd" },
{ K_KEND, "KP1" },
{ K_XEND, "xEnd" },
{ K_ZEND, "zEnd" },
{ K_PAGEUP, "PageUp" },
{ K_PAGEDOWN, "PageDown" },
{ K_KPAGEUP, "kPageUp" },
{ K_KPAGEUP, "KP9" },
{ K_KPAGEDOWN, "kPageDown" },
{ K_KPAGEDOWN, "KP3" },
{ K_KORIGIN, "kOrigin" },
{ K_KORIGIN, "KP5" },
{ K_KPLUS, "kPlus" },
{ K_KPLUS, "KPPlus" },
{ K_KMINUS, "kMinus" },
{ K_KMINUS, "KPMinus" },
{ K_KDIVIDE, "kDivide" },
{ K_KDIVIDE, "KPDiv" },
{ K_KMULTIPLY, "kMultiply" },
{ K_KMULTIPLY, "KPMult" },
{ K_KENTER, "kEnter" },
{ K_KENTER, "KPEnter" },
{ K_KPOINT, "kPoint" },
{ K_KCOMMA, "kComma" },
{ K_KCOMMA, "KPComma" },
{ K_KEQUAL, "kEqual" },
{ K_KEQUAL, "KPEquals" },
{ K_K0, "k0" },
{ K_K1, "k1" },
{ K_K2, "k2" },
{ K_K3, "k3" },
{ K_K4, "k4" },
{ K_K5, "k5" },
{ K_K6, "k6" },
{ K_K7, "k7" },
{ K_K8, "k8" },
{ K_K9, "k9" },
{ '<', "lt" },
{ K_MOUSE, "Mouse" },
{ K_LEFTMOUSE, "LeftMouse" },
{ K_LEFTMOUSE_NM, "LeftMouseNM" },
{ K_LEFTDRAG, "LeftDrag" },
{ K_LEFTRELEASE, "LeftRelease" },
{ K_LEFTRELEASE_NM, "LeftReleaseNM" },
{ K_MIDDLEMOUSE, "MiddleMouse" },
{ K_MIDDLEDRAG, "MiddleDrag" },
{ K_MIDDLERELEASE, "MiddleRelease" },
{ K_RIGHTMOUSE, "RightMouse" },
{ K_RIGHTDRAG, "RightDrag" },
{ K_RIGHTRELEASE, "RightRelease" },
{ K_MOUSEDOWN, "ScrollWheelUp" },
{ K_MOUSEUP, "ScrollWheelDown" },
{ K_MOUSELEFT, "ScrollWheelRight" },
{ K_MOUSERIGHT, "ScrollWheelLeft" },
{ K_MOUSEDOWN, "MouseDown" }, 
{ K_MOUSEUP, "MouseUp" }, 
{ K_X1MOUSE, "X1Mouse" },
{ K_X1DRAG, "X1Drag" },
{ K_X1RELEASE, "X1Release" },
{ K_X2MOUSE, "X2Mouse" },
{ K_X2DRAG, "X2Drag" },
{ K_X2RELEASE, "X2Release" },
{ K_DROP, "Drop" },
{ K_ZERO, "Nul" },
{ K_SNR, "SNR" },
{ K_PLUG, "Plug" },
{ K_IGNORE, "Ignore" },
{ K_COMMAND, "Cmd" },
{ 0, NULL }
};
static struct mousetable {
int pseudo_code; 
int button; 
int is_click; 
int is_drag; 
} mouse_table[] =
{
{(int)KE_LEFTMOUSE, MOUSE_LEFT, TRUE, FALSE},
{(int)KE_LEFTDRAG, MOUSE_LEFT, FALSE, TRUE},
{(int)KE_LEFTRELEASE, MOUSE_LEFT, FALSE, FALSE},
{(int)KE_MIDDLEMOUSE, MOUSE_MIDDLE, TRUE, FALSE},
{(int)KE_MIDDLEDRAG, MOUSE_MIDDLE, FALSE, TRUE},
{(int)KE_MIDDLERELEASE, MOUSE_MIDDLE, FALSE, FALSE},
{(int)KE_RIGHTMOUSE, MOUSE_RIGHT, TRUE, FALSE},
{(int)KE_RIGHTDRAG, MOUSE_RIGHT, FALSE, TRUE},
{(int)KE_RIGHTRELEASE, MOUSE_RIGHT, FALSE, FALSE},
{(int)KE_X1MOUSE, MOUSE_X1, TRUE, FALSE},
{(int)KE_X1DRAG, MOUSE_X1, FALSE, TRUE},
{(int)KE_X1RELEASE, MOUSE_X1, FALSE, FALSE},
{(int)KE_X2MOUSE, MOUSE_X2, TRUE, FALSE},
{(int)KE_X2DRAG, MOUSE_X2, FALSE, TRUE},
{(int)KE_X2RELEASE, MOUSE_X2, FALSE, FALSE},
{(int)KE_IGNORE, MOUSE_RELEASE, FALSE, TRUE},
{(int)KE_IGNORE, MOUSE_RELEASE, FALSE, FALSE},
{0, 0, 0, 0},
};
int name_to_mod_mask(int c)
FUNC_ATTR_CONST FUNC_ATTR_WARN_UNUSED_RESULT
{
c = TOUPPER_ASC(c);
for (size_t i = 0; mod_mask_table[i].mod_mask != 0; i++) {
if (c == mod_mask_table[i].name) {
return mod_mask_table[i].mod_flag;
}
}
return 0;
}
int simplify_key(const int key, int *modifiers)
FUNC_ATTR_WARN_UNUSED_RESULT FUNC_ATTR_NONNULL_ALL
{
if (*modifiers & (MOD_MASK_SHIFT | MOD_MASK_CTRL | MOD_MASK_ALT)) {
if (key == TAB && (*modifiers & MOD_MASK_SHIFT)) {
*modifiers &= ~MOD_MASK_SHIFT;
return K_S_TAB;
}
const int key0 = KEY2TERMCAP0(key);
const int key1 = KEY2TERMCAP1(key);
for (int i = 0; modifier_keys_table[i] != NUL; i += MOD_KEYS_ENTRY_SIZE) {
if (key0 == modifier_keys_table[i + 3]
&& key1 == modifier_keys_table[i + 4]
&& (*modifiers & modifier_keys_table[i])) {
*modifiers &= ~modifier_keys_table[i];
return TERMCAP2KEY(modifier_keys_table[i + 1],
modifier_keys_table[i + 2]);
}
}
}
return key;
}
int handle_x_keys(const int key)
FUNC_ATTR_CONST FUNC_ATTR_WARN_UNUSED_RESULT
{
switch (key) {
case K_XUP: return K_UP;
case K_XDOWN: return K_DOWN;
case K_XLEFT: return K_LEFT;
case K_XRIGHT: return K_RIGHT;
case K_XHOME: return K_HOME;
case K_ZHOME: return K_HOME;
case K_XEND: return K_END;
case K_ZEND: return K_END;
case K_XF1: return K_F1;
case K_XF2: return K_F2;
case K_XF3: return K_F3;
case K_XF4: return K_F4;
case K_S_XF1: return K_S_F1;
case K_S_XF2: return K_S_F2;
case K_S_XF3: return K_S_F3;
case K_S_XF4: return K_S_F4;
}
return key;
}
char_u *get_special_key_name(int c, int modifiers)
{
static char_u string[MAX_KEY_NAME_LEN + 1];
int i, idx;
int table_idx;
char_u *s;
string[0] = '<';
idx = 1;
if (IS_SPECIAL(c) && KEY2TERMCAP0(c) == KS_KEY)
c = KEY2TERMCAP1(c);
if (IS_SPECIAL(c)) {
for (i = 0; modifier_keys_table[i] != 0; i += MOD_KEYS_ENTRY_SIZE)
if ( KEY2TERMCAP0(c) == (int)modifier_keys_table[i + 1]
&& (int)KEY2TERMCAP1(c) == (int)modifier_keys_table[i + 2]) {
modifiers |= modifier_keys_table[i];
c = TERMCAP2KEY(modifier_keys_table[i + 3],
modifier_keys_table[i + 4]);
break;
}
}
table_idx = find_special_key_in_table(c);
if (c > 0
&& (*mb_char2len)(c) == 1
) {
if (table_idx < 0
&& (!vim_isprintc(c) || (c & 0x7f) == ' ')
&& (c & 0x80)) {
c &= 0x7f;
modifiers |= MOD_MASK_ALT;
table_idx = find_special_key_in_table(c);
}
if (table_idx < 0 && !vim_isprintc(c) && c < ' ') {
c += '@';
modifiers |= MOD_MASK_CTRL;
}
}
for (i = 0; mod_mask_table[i].name != 'A'; i++)
if ((modifiers & mod_mask_table[i].mod_mask)
== mod_mask_table[i].mod_flag) {
string[idx++] = mod_mask_table[i].name;
string[idx++] = (char_u)'-';
}
if (table_idx < 0) { 
if (IS_SPECIAL(c)) {
string[idx++] = 't';
string[idx++] = '_';
string[idx++] = (char_u)KEY2TERMCAP0(c);
string[idx++] = KEY2TERMCAP1(c);
} else {
if (utf_char2len(c) > 1) {
idx += utf_char2bytes(c, string + idx);
} else if (vim_isprintc(c)) {
string[idx++] = (char_u)c;
} else {
s = transchar(c);
while (*s)
string[idx++] = *s++;
}
}
} else { 
size_t len = STRLEN(key_names_table[table_idx].name);
if ((int)len + idx + 2 <= MAX_KEY_NAME_LEN) {
STRCPY(string + idx, key_names_table[table_idx].name);
idx += (int)len;
}
}
string[idx++] = '>';
string[idx] = NUL;
return string;
}
unsigned int trans_special(const char_u **srcp, const size_t src_len,
char_u *const dst, const bool keycode,
const bool in_string)
FUNC_ATTR_NONNULL_ALL FUNC_ATTR_WARN_UNUSED_RESULT
{
int modifiers = 0;
int key;
unsigned int dlen = 0;
key = find_special_key(srcp, src_len, &modifiers, keycode, false, in_string);
if (key == 0) {
return 0;
}
if (modifiers != 0) {
dst[dlen++] = K_SPECIAL;
dst[dlen++] = KS_MODIFIER;
dst[dlen++] = (char_u)modifiers;
}
if (IS_SPECIAL(key)) {
dst[dlen++] = K_SPECIAL;
dst[dlen++] = (char_u)KEY2TERMCAP0(key);
dst[dlen++] = KEY2TERMCAP1(key);
} else if (!keycode) {
dlen += (unsigned int)utf_char2bytes(key, dst + dlen);
} else {
char_u *after = add_char2buf(key, dst + dlen);
assert(after >= dst && (uintmax_t)(after - dst) <= UINT_MAX);
dlen = (unsigned int)(after - dst);
}
return dlen;
}
int find_special_key(const char_u **srcp, const size_t src_len, int *const modp,
const bool keycode, const bool keep_x_key,
const bool in_string)
FUNC_ATTR_WARN_UNUSED_RESULT FUNC_ATTR_NONNULL_ALL
{
const char_u *last_dash;
const char_u *end_of_name;
const char_u *src;
const char_u *bp;
const char_u *const end = *srcp + src_len - 1;
int modifiers;
int bit;
int key;
uvarnumber_T n;
int l;
if (src_len == 0) {
return 0;
}
src = *srcp;
if (src[0] != '<') {
return 0;
}
last_dash = src;
for (bp = src + 1; bp <= end && (*bp == '-' || ascii_isident(*bp)); bp++) {
if (*bp == '-') {
last_dash = bp;
if (bp + 1 <= end) {
l = utfc_ptr2len_len(bp + 1, (int)(end - bp) + 1);
if (end - bp > l && !(in_string && bp[1] == '"') && bp[l+1] == '>') {
bp += l;
} else if (end - bp > 2 && in_string && bp[1] == '\\'
&& bp[2] == '"' && bp[3] == '>') {
bp += 2;
}
}
}
if (end - bp > 3 && bp[0] == 't' && bp[1] == '_') {
bp += 3; 
} else if (end - bp > 4 && STRNICMP(bp, "char-", 5) == 0) {
vim_str2nr(bp + 5, NULL, &l, STR2NR_ALL, NULL, NULL, 0);
bp += l + 5;
break;
}
}
if (bp <= end && *bp == '>') { 
end_of_name = bp + 1;
modifiers = 0x0;
for (bp = src + 1; bp < last_dash; bp++) {
if (*bp != '-') {
bit = name_to_mod_mask(*bp);
if (bit == 0x0) {
break; 
}
modifiers |= bit;
}
}
if (bp >= last_dash) {
if (STRNICMP(last_dash + 1, "char-", 5) == 0
&& ascii_isdigit(last_dash[6])) {
vim_str2nr(last_dash + 6, NULL, NULL, STR2NR_ALL, NULL, &n, 0);
key = (int)n;
} else {
int off = 1;
if (in_string && last_dash[1] == '\\' && last_dash[2] == '"') {
off = l = 2;
} else {
l = mb_ptr2len(last_dash + 1);
}
if (modifiers != 0 && last_dash[l + 1] == '>') {
key = PTR2CHAR(last_dash + off);
} else {
key = get_special_key_code(last_dash + off);
if (!keep_x_key) {
key = handle_x_keys(key);
}
}
}
if (key != NUL) {
key = simplify_key(key, &modifiers);
if (!keycode) {
if (key == K_BS) {
key = BS;
} else if (key == K_DEL || key == K_KDEL) {
key = DEL;
}
}
if (!IS_SPECIAL(key)) {
key = extract_modifiers(key, &modifiers);
}
*modp = modifiers;
*srcp = end_of_name;
return key;
} 
}
}
return 0;
}
static int extract_modifiers(int key, int *modp)
{
int modifiers = *modp;
if (!(modifiers & MOD_MASK_CMD)) { 
if ((modifiers & MOD_MASK_SHIFT) && ASCII_ISALPHA(key)) {
key = TOUPPER_ASC(key);
modifiers &= ~MOD_MASK_SHIFT;
}
}
if ((modifiers & MOD_MASK_CTRL)
&& ((key >= '?' && key <= '_') || ASCII_ISALPHA(key))) {
key = Ctrl_chr(key);
modifiers &= ~MOD_MASK_CTRL;
if (key == 0) { 
key = K_ZERO;
}
}
*modp = modifiers;
return key;
}
int find_special_key_in_table(int c)
{
int i;
for (i = 0; key_names_table[i].name != NULL; i++) {
if (c == key_names_table[i].key) {
break;
}
}
if (key_names_table[i].name == NULL) {
i = -1;
}
return i;
}
int get_special_key_code(const char_u *name)
FUNC_ATTR_NONNULL_ALL FUNC_ATTR_PURE FUNC_ATTR_WARN_UNUSED_RESULT
{
for (int i = 0; key_names_table[i].name != NULL; i++) {
const char *const table_name = key_names_table[i].name;
int j;
for (j = 0; ascii_isident(name[j]) && table_name[j] != NUL; j++) {
if (TOLOWER_ASC(table_name[j]) != TOLOWER_ASC(name[j])) {
break;
}
}
if (!ascii_isident(name[j]) && table_name[j] == NUL) {
return key_names_table[i].key;
}
}
return 0;
}
int get_mouse_button(int code, bool *is_click, bool *is_drag)
{
int i;
for (i = 0; mouse_table[i].pseudo_code; i++)
if (code == mouse_table[i].pseudo_code) {
*is_click = mouse_table[i].is_click;
*is_drag = mouse_table[i].is_drag;
return mouse_table[i].button;
}
return 0; 
}
char_u *replace_termcodes(const char_u *from, const size_t from_len,
char_u **bufp, const bool from_part, const bool do_lt,
const bool special, int cpo_flags)
FUNC_ATTR_NONNULL_ALL
{
ssize_t i;
size_t slen;
char_u key;
size_t dlen = 0;
const char_u *src;
const char_u *const end = from + from_len - 1;
int do_backslash; 
char_u *result; 
do_backslash = !(cpo_flags&FLAG_CPO_BSLASH);
const size_t buf_len = from_len * 6 + 1;
result = xmalloc(buf_len);
src = from;
if (from_part && from_len > 1 && src[0] == '#'
&& ascii_isdigit(src[1])) { 
result[dlen++] = K_SPECIAL;
result[dlen++] = 'k';
if (src[1] == '0') {
result[dlen++] = ';'; 
} else {
result[dlen++] = src[1]; 
}
src += 2;
}
while (src <= end) {
if (special && (do_lt || ((end - src) >= 3
&& STRNCMP(src, "<lt>", 4) != 0))) {
if (end - src >= 4 && STRNICMP(src, "<SID>", 5) == 0) {
if (current_sctx.sc_sid <= 0) {
EMSG(_(e_usingsid));
} else {
src += 5;
result[dlen++] = K_SPECIAL;
result[dlen++] = (int)KS_EXTRA;
result[dlen++] = (int)KE_SNR;
snprintf((char *)result + dlen, buf_len - dlen, "%" PRId64,
(int64_t)current_sctx.sc_sid);
dlen += STRLEN(result + dlen);
result[dlen++] = '_';
continue;
}
}
slen = trans_special(&src, (size_t)(end - src) + 1, result + dlen, true,
false);
if (slen) {
dlen += slen;
continue;
}
}
if (special) {
char_u *p, *s, len;
if (end - src >= 7 && STRNICMP(src, "<Leader>", 8) == 0) {
len = 8;
p = get_var_value("g:mapleader");
} else if (end - src >= 12 && STRNICMP(src, "<LocalLeader>", 13) == 0) {
len = 13;
p = get_var_value("g:maplocalleader");
} else {
len = 0;
p = NULL;
}
if (len != 0) {
if (p == NULL || *p == NUL || STRLEN(p) > 8 * 6) {
s = (char_u *)"\\";
} else {
s = p;
}
while (*s != NUL) {
result[dlen++] = *s++;
}
src += len;
continue;
}
}
key = *src;
if (key == Ctrl_V || (do_backslash && key == '\\')) {
src++; 
if (src > end) {
if (from_part) {
result[dlen++] = key;
}
break;
}
}
for (i = utfc_ptr2len_len(src, (int)(end - src) + 1); i > 0; i--) {
if (*src == K_SPECIAL) {
result[dlen++] = K_SPECIAL;
result[dlen++] = KS_SPECIAL;
result[dlen++] = KE_FILLER;
} else {
result[dlen++] = *src;
}
++src;
}
}
result[dlen] = NUL;
*bufp = xrealloc(result, dlen + 1);
return *bufp;
}
void log_key(int log_level, int key)
{
if (log_level < MIN_LOG_LEVEL) {
return;
}
char *keyname = key == K_EVENT
? "K_EVENT"
: (char *)get_special_key_name(key, mod_mask);
LOG(log_level, "input: %s", keyname);
}
