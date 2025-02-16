#define ALLEGRO_NO_COMPATIBILITY
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <ctype.h>
#include <X11/Xlocale.h>
#include <X11/Xos.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xproto.h>
#include <X11/XKBlib.h>
#include "allegro5/allegro.h"
#include "allegro5/internal/aintern.h"
#include "allegro5/internal/aintern_events.h"
#include "allegro5/internal/aintern_keyboard.h"
#include "allegro5/internal/aintern_x.h"
#include "allegro5/internal/aintern_xdisplay.h"
#include "allegro5/internal/aintern_xkeyboard.h"
#include "allegro5/internal/aintern_xsystem.h"
#if defined(ALLEGRO_RASPBERRYPI)
#include "allegro5/internal/aintern_raspberrypi.h"
#define ALLEGRO_SYSTEM_XGLX ALLEGRO_SYSTEM_RASPBERRYPI
#endif
ALLEGRO_DEBUG_CHANNEL("keyboard")
static void handle_key_press(int mycode, int unichar, int filtered,
unsigned int modifiers, ALLEGRO_DISPLAY *display);
static void handle_key_release(int mycode, ALLEGRO_DISPLAY *display);
static int _key_shifts;
typedef struct ALLEGRO_KEYBOARD_XWIN
{
ALLEGRO_KEYBOARD parent;
ALLEGRO_KEYBOARD_STATE state;
bool three_finger_flag;
} ALLEGRO_KEYBOARD_XWIN;
typedef struct ALLEGRO_KEY_REPEAT_DATA {
XKeyEvent *event;
bool found;
} ALLEGRO_KEY_REPEAT_DATA;
static ALLEGRO_KEYBOARD_XWIN the_keyboard;
static int last_press_code = -1;
#if defined(ALLEGRO_XWINDOWS_WITH_XIM)
static XIM xim = NULL;
static XIC xic = NULL;
#endif
static XModifierKeymap *xmodmap = NULL;
static int xkeyboard_installed = 0;
static int used[ALLEGRO_KEY_MAX];
static int sym_per_key;
static int min_keycode, max_keycode;
static KeySym *keysyms = NULL;
static int main_pid; 
static int pause_key = 0; 
static int keycode_to_scancode[256];
static struct {
KeySym keysym;
int allegro_key;
}
translation_table[] = {
{XK_a, ALLEGRO_KEY_A},
{XK_b, ALLEGRO_KEY_B},
{XK_c, ALLEGRO_KEY_C},
{XK_d, ALLEGRO_KEY_D},
{XK_e, ALLEGRO_KEY_E},
{XK_f, ALLEGRO_KEY_F},
{XK_g, ALLEGRO_KEY_G},
{XK_h, ALLEGRO_KEY_H},
{XK_i, ALLEGRO_KEY_I},
{XK_j, ALLEGRO_KEY_J},
{XK_k, ALLEGRO_KEY_K},
{XK_l, ALLEGRO_KEY_L},
{XK_m, ALLEGRO_KEY_M},
{XK_n, ALLEGRO_KEY_N},
{XK_o, ALLEGRO_KEY_O},
{XK_p, ALLEGRO_KEY_P},
{XK_q, ALLEGRO_KEY_Q},
{XK_r, ALLEGRO_KEY_R},
{XK_s, ALLEGRO_KEY_S},
{XK_t, ALLEGRO_KEY_T},
{XK_u, ALLEGRO_KEY_U},
{XK_v, ALLEGRO_KEY_V},
{XK_w, ALLEGRO_KEY_W},
{XK_x, ALLEGRO_KEY_X},
{XK_y, ALLEGRO_KEY_Y},
{XK_z, ALLEGRO_KEY_Z},
{XK_0, ALLEGRO_KEY_0},
{XK_1, ALLEGRO_KEY_1},
{XK_2, ALLEGRO_KEY_2},
{XK_3, ALLEGRO_KEY_3},
{XK_4, ALLEGRO_KEY_4},
{XK_5, ALLEGRO_KEY_5},
{XK_6, ALLEGRO_KEY_6},
{XK_7, ALLEGRO_KEY_7},
{XK_8, ALLEGRO_KEY_8},
{XK_9, ALLEGRO_KEY_9},
{XK_KP_0, ALLEGRO_KEY_PAD_0},
{XK_KP_Insert, ALLEGRO_KEY_PAD_0},
{XK_KP_1, ALLEGRO_KEY_PAD_1},
{XK_KP_End, ALLEGRO_KEY_PAD_1},
{XK_KP_2, ALLEGRO_KEY_PAD_2},
{XK_KP_Down, ALLEGRO_KEY_PAD_2},
{XK_KP_3, ALLEGRO_KEY_PAD_3},
{XK_KP_Page_Down, ALLEGRO_KEY_PAD_3},
{XK_KP_4, ALLEGRO_KEY_PAD_4},
{XK_KP_Left, ALLEGRO_KEY_PAD_4},
{XK_KP_5, ALLEGRO_KEY_PAD_5},
{XK_KP_Begin, ALLEGRO_KEY_PAD_5},
{XK_KP_6, ALLEGRO_KEY_PAD_6},
{XK_KP_Right, ALLEGRO_KEY_PAD_6},
{XK_KP_7, ALLEGRO_KEY_PAD_7},
{XK_KP_Home, ALLEGRO_KEY_PAD_7},
{XK_KP_8, ALLEGRO_KEY_PAD_8},
{XK_KP_Up, ALLEGRO_KEY_PAD_8},
{XK_KP_9, ALLEGRO_KEY_PAD_9},
{XK_KP_Page_Up, ALLEGRO_KEY_PAD_9},
{XK_KP_Delete, ALLEGRO_KEY_PAD_DELETE},
{XK_KP_Decimal, ALLEGRO_KEY_PAD_DELETE},
{XK_Pause, ALLEGRO_KEY_PAUSE},
{XK_Break, ALLEGRO_KEY_PAUSE},
{XK_F1, ALLEGRO_KEY_F1},
{XK_F2, ALLEGRO_KEY_F2},
{XK_F3, ALLEGRO_KEY_F3},
{XK_F4, ALLEGRO_KEY_F4},
{XK_F5, ALLEGRO_KEY_F5},
{XK_F6, ALLEGRO_KEY_F6},
{XK_F7, ALLEGRO_KEY_F7},
{XK_F8, ALLEGRO_KEY_F8},
{XK_F9, ALLEGRO_KEY_F9},
{XK_F10, ALLEGRO_KEY_F10},
{XK_F11, ALLEGRO_KEY_F11},
{XK_F12, ALLEGRO_KEY_F12},
{XK_Escape, ALLEGRO_KEY_ESCAPE},
{XK_grave, ALLEGRO_KEY_TILDE}, 
{XK_minus, ALLEGRO_KEY_MINUS}, 
{XK_equal, ALLEGRO_KEY_EQUALS}, 
{XK_BackSpace, ALLEGRO_KEY_BACKSPACE},
{XK_Tab, ALLEGRO_KEY_TAB},
{XK_bracketleft, ALLEGRO_KEY_OPENBRACE}, 
{XK_bracketright, ALLEGRO_KEY_CLOSEBRACE}, 
{XK_Return, ALLEGRO_KEY_ENTER},
{XK_semicolon, ALLEGRO_KEY_SEMICOLON}, 
{XK_apostrophe, ALLEGRO_KEY_QUOTE}, 
{XK_backslash, ALLEGRO_KEY_BACKSLASH}, 
{XK_less, ALLEGRO_KEY_BACKSLASH2}, 
{XK_comma, ALLEGRO_KEY_COMMA}, 
{XK_period, ALLEGRO_KEY_FULLSTOP}, 
{XK_slash, ALLEGRO_KEY_SLASH}, 
{XK_space, ALLEGRO_KEY_SPACE},
{XK_Insert, ALLEGRO_KEY_INSERT},
{XK_Delete, ALLEGRO_KEY_DELETE},
{XK_Home, ALLEGRO_KEY_HOME},
{XK_End, ALLEGRO_KEY_END},
{XK_Page_Up, ALLEGRO_KEY_PGUP},
{XK_Page_Down, ALLEGRO_KEY_PGDN},
{XK_Left, ALLEGRO_KEY_LEFT},
{XK_Right, ALLEGRO_KEY_RIGHT},
{XK_Up, ALLEGRO_KEY_UP},
{XK_Down, ALLEGRO_KEY_DOWN},
{XK_KP_Divide, ALLEGRO_KEY_PAD_SLASH},
{XK_KP_Multiply, ALLEGRO_KEY_PAD_ASTERISK},
{XK_KP_Subtract, ALLEGRO_KEY_PAD_MINUS},
{XK_KP_Add, ALLEGRO_KEY_PAD_PLUS},
{XK_KP_Enter, ALLEGRO_KEY_PAD_ENTER},
{XK_Print, ALLEGRO_KEY_PRINTSCREEN},
{XK_KP_Equal, ALLEGRO_KEY_PAD_EQUALS}, 
{XK_Shift_L, ALLEGRO_KEY_LSHIFT},
{XK_Shift_R, ALLEGRO_KEY_RSHIFT},
{XK_Control_L, ALLEGRO_KEY_LCTRL},
{XK_Control_R, ALLEGRO_KEY_RCTRL},
{XK_Alt_L, ALLEGRO_KEY_ALT},
{XK_Alt_R, ALLEGRO_KEY_ALTGR},
{XK_ISO_Level3_Shift, ALLEGRO_KEY_ALTGR},
{XK_Meta_L, ALLEGRO_KEY_LWIN},
{XK_Super_L, ALLEGRO_KEY_LWIN},
{XK_Meta_R, ALLEGRO_KEY_RWIN},
{XK_Super_R, ALLEGRO_KEY_RWIN},
{XK_Menu, ALLEGRO_KEY_MENU},
{XK_Scroll_Lock, ALLEGRO_KEY_SCROLLLOCK},
{XK_Num_Lock, ALLEGRO_KEY_NUMLOCK},
{XK_Caps_Lock, ALLEGRO_KEY_CAPSLOCK}
};
static int modifier_flags[8][3] = {
{ALLEGRO_KEYMOD_SHIFT, ShiftMask, 0},
{ALLEGRO_KEYMOD_CAPSLOCK, LockMask, 1},
{ALLEGRO_KEYMOD_CTRL, ControlMask, 0},
{ALLEGRO_KEYMOD_ALT, Mod1Mask, 0},
{ALLEGRO_KEYMOD_NUMLOCK, Mod2Mask, 1},
{ALLEGRO_KEYMOD_SCROLLLOCK, Mod3Mask, 1},
{ALLEGRO_KEYMOD_LWIN | ALLEGRO_KEYMOD_RWIN, Mod4Mask, 0}, 
{ALLEGRO_KEYMOD_MENU, Mod5Mask, 0} 
};
static char const *key_names[ALLEGRO_KEY_MAX];
static void update_shifts(XKeyEvent *event)
{
int mask = 0;
int i;
for (i = 0; i < 8; i++) {
int j;
if (event->state & modifier_flags[i][1])
mask |= modifier_flags[i][0];
for (j = 0; j < xmodmap->max_keypermod; j++) {
if (event->keycode && event->keycode ==
xmodmap->modifiermap[i * xmodmap->max_keypermod + j]) {
if (event->type == KeyPress) {
if (modifier_flags[i][2])
mask ^= modifier_flags[i][0];
else
mask |= modifier_flags[i][0];
}
else if (event->type == KeyRelease) {
if (!modifier_flags[i][2])
mask &= ~modifier_flags[i][0];
}
}
}
}
_key_shifts = mask;
}
static int find_unknown_key_assignment(int i)
{
int j;
for (j = 1; j < ALLEGRO_KEY_MAX; j++) {
if (!used[j]) {
const char *str;
keycode_to_scancode[i] = j;
str = XKeysymToString(keysyms[sym_per_key * (i - min_keycode)]);
if (str)
key_names[j] = str;
else {
key_names[j] = _al_keyboard_common_names[j];
}
used[j] = 1;
break;
}
}
if (j == ALLEGRO_KEY_MAX) {
ALLEGRO_ERROR("You have more keys reported by X than Allegro's "
"maximum of %i keys. Please send a bug report.\n", ALLEGRO_KEY_MAX);
keycode_to_scancode[i] = 0;
}
char str[1024];
sprintf(str, "Key %i missing:", i);
for (j = 0; j < sym_per_key; j++) {
char *sym_str = XKeysymToString(keysyms[sym_per_key * (i - min_keycode) + j]);
sprintf(str + strlen(str), " %s", sym_str ? sym_str : "NULL");
}
ALLEGRO_DEBUG("%s assigned to %i.\n", str, keycode_to_scancode[i]);
return keycode_to_scancode[i];
}
static Bool check_for_repeat(Display *display, XEvent *event, XPointer arg)
{
ALLEGRO_KEY_REPEAT_DATA *d = (ALLEGRO_KEY_REPEAT_DATA *)arg;
(void)display;
if (event->type == KeyPress &&
event->xkey.keycode == d->event->keycode &&
(event->xkey.time - d->event->time) < 4) {
d->found = true;
}
return False;
}
void _al_xwin_keyboard_handler(XKeyEvent *event, ALLEGRO_DISPLAY *display)
{
int keycode;
if (!xkeyboard_installed)
return;
keycode = keycode_to_scancode[event->keycode];
if (keycode == -1)
keycode = find_unknown_key_assignment(event->keycode);
update_shifts(event);
if (keycode == ALLEGRO_KEY_PAUSE) {
if (event->type == KeyRelease)
return;
if (pause_key) {
event->type = KeyRelease;
pause_key = 0;
}
else {
pause_key = 1;
}
}
if (event->type == KeyPress) { 
int len;
char buffer[16];
int unicode = 0;
int filtered = 0;
#if defined (ALLEGRO_XWINDOWS_WITH_XIM) && defined(X_HAVE_UTF8_STRING)
if (xic) {
len = Xutf8LookupString(xic, event, buffer, sizeof buffer, NULL, NULL);
}
else
#endif
{
len = XLookupString(event, buffer, sizeof buffer, NULL, NULL);
}
buffer[len] = '\0';
ALLEGRO_USTR_INFO info;
const ALLEGRO_USTR *ustr = al_ref_cstr(&info, buffer);
unicode = al_ustr_get(ustr, 0);
if (unicode < 0)
unicode = 0;
#if defined(ALLEGRO_XWINDOWS_WITH_XIM)
ALLEGRO_DISPLAY_XGLX *glx = (void *)display;
filtered = XFilterEvent((XEvent *)event, glx->window);
#endif
if (keycode || unicode) {
handle_key_press(keycode, unicode, filtered, _key_shifts, display);
}
}
else { 
if (XPending(event->display) > 0) {
ALLEGRO_KEY_REPEAT_DATA d;
XEvent dummy;
d.event = event;
d.found = false;
XCheckIfEvent(event->display, &dummy, check_for_repeat, (XPointer)&d);
if (d.found) {
return;
}
}
handle_key_release(keycode, display);
}
}
void _al_xwin_keyboard_switch_handler(ALLEGRO_DISPLAY *display, bool focus_in)
{
_al_event_source_lock(&the_keyboard.parent.es);
if (focus_in) {
the_keyboard.state.display = display;
#if defined(ALLEGRO_XWINDOWS_WITH_XIM)
if (xic) {
ALLEGRO_DISPLAY_XGLX *display_glx = (void *)display;
XSetICValues(xic, XNClientWindow, display_glx->window, NULL);
}
#endif
}
else {
the_keyboard.state.display = NULL;
}
_al_event_source_unlock(&the_keyboard.parent.es);
}
static int find_allegro_key(KeySym sym)
{
int i;
int n = sizeof translation_table / sizeof *translation_table;
for (i = 0; i < n; i++) {
if (translation_table[i].keysym == sym)
return translation_table[i].allegro_key;
}
return 0;
}
static const char *x_scancode_to_name(int scancode)
{
ASSERT (scancode >= 0 && scancode < ALLEGRO_KEY_MAX);
return key_names[scancode];
}
static bool _al_xwin_get_keyboard_mapping(void)
{
int i;
int count;
int missing = 0;
ALLEGRO_SYSTEM_XGLX *system = (void *)al_get_system_driver();
memset(used, 0, sizeof used);
memset(keycode_to_scancode, 0, sizeof keycode_to_scancode);
XDisplayKeycodes(system->x11display, &min_keycode, &max_keycode);
count = 1 + max_keycode - min_keycode;
if (keysyms) {
XFree(keysyms);
}
keysyms = XGetKeyboardMapping(system->x11display, min_keycode,
count, &sym_per_key);
ALLEGRO_INFO("%i keys, %i symbols per key.\n", count, sym_per_key);
if (sym_per_key <= 0) {
return false;
}
missing = 0;
for (i = min_keycode; i <= max_keycode; i++) {
KeySym sym = keysyms[sym_per_key * (i - min_keycode)];
KeySym sym2 = keysyms[sym_per_key * (i - min_keycode) + 1];
char *sym_str, *sym2_str;
int allegro_key = 0;
char str[1024];
sym_str = XKeysymToString(sym);
sym2_str = XKeysymToString(sym2);
snprintf(str, sizeof str, "key [%i: %s %s]", i,
sym_str ? sym_str : "NULL", sym2_str ? sym2_str : "NULL");
if (sym2 >= XK_0 && sym2 <= XK_9) {
allegro_key = find_allegro_key(sym2);
}
if (!allegro_key) {
if (sym != NoSymbol) {
allegro_key = find_allegro_key(sym);
if (allegro_key == 0) {
missing++;
ALLEGRO_DEBUG("%s defering.\n", str);
}
}
else {
keycode_to_scancode[i] = -1;
ALLEGRO_DEBUG("%s not assigned.\n", str);
}
}
if (allegro_key) {
bool is_double = false;
if (used[allegro_key]) {
is_double = true;
}
keycode_to_scancode[i] = allegro_key;
key_names[allegro_key] =
XKeysymToString(keysyms[sym_per_key * (i - min_keycode)]);
used[allegro_key] = 1;
ALLEGRO_DEBUG("%s%s assigned to %i.\n", str,
is_double ? " *double*" : "", allegro_key);
}
}
if (missing) {
for (i = min_keycode; i <= max_keycode; i++) {
if (keycode_to_scancode[i] == 0) {
find_unknown_key_assignment(i);
}
}
}
if (xmodmap)
XFreeModifiermap(xmodmap);
xmodmap = XGetModifierMapping(system->x11display);
for (i = 0; i < 8; i++) {
int j;
char str[1024];
sprintf(str, "Modifier %d:", i + 1);
for (j = 0; j < xmodmap->max_keypermod; j++) {
KeyCode keycode = xmodmap->modifiermap[i * xmodmap->max_keypermod + j];
KeySym sym = XkbKeycodeToKeysym(system->x11display, keycode, 0, 0);
char *sym_str = XKeysymToString(sym);
sprintf(str + strlen(str), " %s", sym_str ? sym_str : "NULL");
}
ALLEGRO_DEBUG("%s\n", str);
}
ALLEGRO_CONFIG *c = al_get_system_config();
char const *key;
ALLEGRO_CONFIG_ENTRY *it;
key = al_get_first_config_entry(c, "xkeymap", &it);
while (key) {
char const *val;
val = al_get_config_value(c, "xkeymap", key);
int keycode = strtol(key, NULL, 10);
int scancode = strtol(val, NULL, 10);
if (keycode > 0 && scancode > 0) {
keycode_to_scancode[keycode] = scancode;
ALLEGRO_WARN("User override: KeySym %i assigned to %i.\n",
keycode, scancode);
}
key = al_get_next_config_entry(&it);
}
return true;
}
static void x_set_leds(int leds)
{
XKeyboardControl values;
ALLEGRO_SYSTEM_XGLX *system = (void *)al_get_system_driver();
ASSERT(xkeyboard_installed);
_al_mutex_lock(&system->lock);
values.led = 1;
values.led_mode = leds & ALLEGRO_KEYMOD_NUMLOCK ? LedModeOn : LedModeOff;
XChangeKeyboardControl(system->x11display, KBLed | KBLedMode, &values);
values.led = 2;
values.led_mode = leds & ALLEGRO_KEYMOD_CAPSLOCK ? LedModeOn : LedModeOff;
XChangeKeyboardControl(system->x11display, KBLed | KBLedMode, &values);
values.led = 3;
values.led_mode = leds & ALLEGRO_KEYMOD_SCROLLLOCK ? LedModeOn : LedModeOff;
XChangeKeyboardControl(system->x11display, KBLed | KBLedMode, &values);
_al_mutex_unlock(&system->lock);
}
static int x_keyboard_init(void)
{
#if defined(ALLEGRO_XWINDOWS_WITH_XIM)
char *old_locale;
XIMStyles *xim_styles;
XIMStyle xim_style = 0;
char *imvalret;
int i;
#endif
ALLEGRO_SYSTEM_XGLX *s = (void *)al_get_system_driver();
if (xkeyboard_installed)
return 0;
if (s->x11display == NULL)
return 0;
main_pid = getpid();
memcpy(key_names, _al_keyboard_common_names, sizeof key_names);
_al_mutex_lock(&s->lock);
Bool supported;
XkbSetDetectableAutoRepeat(s->x11display, True, &supported);
if (!supported) {
ALLEGRO_WARN("XkbSetDetectableAutoRepeat failed.\n");
}
#if defined(ALLEGRO_XWINDOWS_WITH_XIM)
ALLEGRO_INFO("Using X Input Method.\n");
old_locale = setlocale(LC_CTYPE, NULL);
ALLEGRO_DEBUG("Old locale: %s\n", old_locale ? old_locale : "(null)");
if (old_locale) {
old_locale = strdup(old_locale);
}
if (setlocale(LC_CTYPE, "") == NULL) {
ALLEGRO_WARN("Could not set default locale.\n");
}
char const *modifiers = XSetLocaleModifiers("@im=none");
if (modifiers == NULL) {
ALLEGRO_WARN("XSetLocaleModifiers failed.\n");
}
xim = XOpenIM(s->x11display, NULL, NULL, NULL);
if (xim == NULL) {
ALLEGRO_WARN("XOpenIM failed.\n");
}
if (old_locale) {
ALLEGRO_DEBUG("Restoring old locale: %s\n", old_locale);
setlocale(LC_CTYPE, old_locale);
free(old_locale);
}
if (xim) {
imvalret = XGetIMValues(xim, XNQueryInputStyle, &xim_styles, NULL);
if (imvalret != NULL || xim_styles == NULL) {
ALLEGRO_WARN("Input method doesn't support any styles.\n");
}
if (xim_styles) {
xim_style = 0;
for (i = 0; i < xim_styles->count_styles; i++) {
if (xim_styles->supported_styles[i] ==
(XIMPreeditNothing | XIMStatusNothing)) {
xim_style = xim_styles->supported_styles[i];
break;
}
}
if (xim_style == 0) {
ALLEGRO_WARN("Input method doesn't support the style we support.\n");
}
else {
ALLEGRO_INFO("Input method style = %ld\n", xim_style);
}
XFree(xim_styles);
}
}
if (xim && xim_style) {
xic = XCreateIC(xim,
XNInputStyle, xim_style,
NULL);
if (xic == NULL) {
ALLEGRO_WARN("XCreateIC failed.\n");
}
else {
ALLEGRO_INFO("XCreateIC succeeded.\n");
}
ALLEGRO_DISPLAY *display = al_get_current_display();
ALLEGRO_DISPLAY_XGLX *display_glx = (void *)display;
if (display_glx && xic)
XSetICValues(xic, XNClientWindow, display_glx->window, NULL);
}
#endif
if (!_al_xwin_get_keyboard_mapping()) {
return 1;
}
_al_mutex_unlock(&s->lock);
xkeyboard_installed = 1;
return 0;
}
static void x_keyboard_exit(void)
{
if (!xkeyboard_installed)
return;
xkeyboard_installed = 0;
ALLEGRO_SYSTEM_XGLX *s = (void *)al_get_system_driver();
_al_mutex_lock(&s->lock);
#if defined(ALLEGRO_XWINDOWS_WITH_XIM)
if (xic) {
XDestroyIC(xic);
xic = NULL;
}
if (xim) {
XCloseIM(xim);
xim = NULL;
}
#endif
if (xmodmap) {
XFreeModifiermap(xmodmap);
xmodmap = NULL;
}
if (keysyms) {
XFree(keysyms);
keysyms = NULL;
}
_al_mutex_unlock(&s->lock);
}
static bool xkeybd_init_keyboard(void);
static void xkeybd_exit_keyboard(void);
static ALLEGRO_KEYBOARD *xkeybd_get_keyboard(void);
static bool xkeybd_set_keyboard_leds(int leds);
static const char *xkeybd_keycode_to_name(int keycode);
static void xkeybd_get_keyboard_state(ALLEGRO_KEYBOARD_STATE *ret_state);
static void xkeybd_clear_keyboard_state(void);
#define KEYDRV_XWIN AL_ID('X','W','I','N')
static ALLEGRO_KEYBOARD_DRIVER keydrv_xwin =
{
KEYDRV_XWIN,
"",
"",
"X11 keyboard",
xkeybd_init_keyboard,
xkeybd_exit_keyboard,
xkeybd_get_keyboard,
xkeybd_set_keyboard_leds,
xkeybd_keycode_to_name,
xkeybd_get_keyboard_state,
xkeybd_clear_keyboard_state
};
ALLEGRO_KEYBOARD_DRIVER *_al_xwin_keyboard_driver(void)
{
return &keydrv_xwin;
}
static bool xkeybd_init_keyboard(void)
{
if (x_keyboard_init() != 0)
return false;
memset(&the_keyboard, 0, sizeof the_keyboard);
_al_event_source_init(&the_keyboard.parent.es);
the_keyboard.three_finger_flag = true;
const char *value = al_get_config_value(al_get_system_config(),
"keyboard", "enable_three_finger_exit");
if (value) {
the_keyboard.three_finger_flag = !strncmp(value, "true", 4);
}
ALLEGRO_DEBUG("Three finger flag enabled: %s\n",
the_keyboard.three_finger_flag ? "true" : "false");
main_pid = getpid();
return true;
}
static void xkeybd_exit_keyboard(void)
{
x_keyboard_exit();
_al_event_source_free(&the_keyboard.parent.es);
}
static ALLEGRO_KEYBOARD *xkeybd_get_keyboard(void)
{
return (ALLEGRO_KEYBOARD *)&the_keyboard;
}
static bool xkeybd_set_keyboard_leds(int leds)
{
x_set_leds(leds);
return true;
}
static const char *xkeybd_keycode_to_name(int keycode)
{
return x_scancode_to_name(keycode);
}
static void xkeybd_get_keyboard_state(ALLEGRO_KEYBOARD_STATE *ret_state)
{
_al_event_source_lock(&the_keyboard.parent.es);
{
*ret_state = the_keyboard.state;
}
_al_event_source_unlock(&the_keyboard.parent.es);
}
static void xkeybd_clear_keyboard_state(void)
{
_al_event_source_lock(&the_keyboard.parent.es);
{
last_press_code = -1;
memset(&the_keyboard.state, 0, sizeof(the_keyboard.state));
}
_al_event_source_unlock(&the_keyboard.parent.es);
}
static void handle_key_press(int mycode, int unichar, int filtered,
unsigned int modifiers, ALLEGRO_DISPLAY *display)
{
bool is_repeat;
is_repeat = (last_press_code == mycode);
if (mycode > 0)
last_press_code = mycode;
_al_event_source_lock(&the_keyboard.parent.es);
{
_AL_KEYBOARD_STATE_SET_KEY_DOWN(the_keyboard.state, mycode);
if (_al_event_source_needs_to_generate_event(&the_keyboard.parent.es)) {
ALLEGRO_EVENT event;
event.keyboard.type = ALLEGRO_EVENT_KEY_DOWN;
event.keyboard.timestamp = al_get_time();
event.keyboard.display = display;
event.keyboard.keycode = last_press_code;
event.keyboard.unichar = 0;
event.keyboard.modifiers = 0;
event.keyboard.repeat = false;
if (mycode > 0 && !is_repeat) {
_al_event_source_emit_event(&the_keyboard.parent.es, &event);
}
if (!filtered && mycode < ALLEGRO_KEY_MODIFIERS) {
event.keyboard.type = ALLEGRO_EVENT_KEY_CHAR;
event.keyboard.unichar = unichar;
event.keyboard.modifiers = modifiers;
event.keyboard.repeat = is_repeat;
_al_event_source_emit_event(&the_keyboard.parent.es, &event);
}
}
}
_al_event_source_unlock(&the_keyboard.parent.es);
#if !defined(ALLEGRO_RASPBERRYPI)
if (last_press_code && !is_repeat) {
ALLEGRO_SYSTEM_XGLX *system = (void *)al_get_system_driver();
if (system->toggle_mouse_grab_keycode &&
system->toggle_mouse_grab_keycode == mycode &&
(modifiers & system->toggle_mouse_grab_modifiers)
== system->toggle_mouse_grab_modifiers)
{
if (system->mouse_grab_display == display)
al_ungrab_mouse();
else
al_grab_mouse(display);
}
}
#endif
if ((the_keyboard.three_finger_flag)
&& ((mycode == ALLEGRO_KEY_DELETE) || (mycode == ALLEGRO_KEY_END))
&& (modifiers & ALLEGRO_KEYMOD_CTRL)
&& (modifiers & (ALLEGRO_KEYMOD_ALT | ALLEGRO_KEYMOD_ALTGR)))
{
ALLEGRO_WARN("Three finger combo detected. SIGTERMing "
"pid %d\n", main_pid);
kill(main_pid, SIGTERM);
}
}
static void handle_key_release(int mycode, ALLEGRO_DISPLAY *display)
{
if (last_press_code == mycode)
last_press_code = -1;
_al_event_source_lock(&the_keyboard.parent.es);
{
_AL_KEYBOARD_STATE_CLEAR_KEY_DOWN(the_keyboard.state, mycode);
if (_al_event_source_needs_to_generate_event(&the_keyboard.parent.es)) {
ALLEGRO_EVENT event;
event.keyboard.type = ALLEGRO_EVENT_KEY_UP;
event.keyboard.timestamp = al_get_time();
event.keyboard.display = display;
event.keyboard.keycode = mycode;
event.keyboard.unichar = 0;
event.keyboard.modifiers = 0;
_al_event_source_emit_event(&the_keyboard.parent.es, &event);
}
}
_al_event_source_unlock(&the_keyboard.parent.es);
}
