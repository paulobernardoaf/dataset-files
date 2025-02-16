





























#define ALLEGRO_NO_COMPATIBILITY

#include <fcntl.h>
#include <linux/input.h>
#include <linux/kd.h>
#include <linux/keyboard.h>
#include <linux/vt.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

#include "allegro5/allegro.h"
#include "allegro5/internal/aintern.h"
#include "allegro5/internal/aintern_driver.h"
#include "allegro5/internal/aintern_events.h"
#include "allegro5/internal/aintern_keyboard.h"
#include "allegro5/platform/aintlnx.h"
#include "allegro5/platform/aintunix.h"


ALLEGRO_DEBUG_CHANNEL("keyboard")


#define PREFIX_I "al-ckey INFO: "
#define PREFIX_W "al-ckey WARNING: "
#define PREFIX_E "al-ckey ERROR: "



typedef struct ALLEGRO_KEYBOARD_LINUX
{
ALLEGRO_KEYBOARD parent;
int fd;
struct termios startup_termio;
struct termios work_termio;
int startup_kbmode;
ALLEGRO_KEYBOARD_STATE state;
unsigned int modifiers;

bool three_finger_flag;

bool key_led_flag;
} ALLEGRO_KEYBOARD_LINUX;




static ALLEGRO_KEYBOARD_LINUX the_keyboard;


static pid_t main_pid;




static bool lkeybd_init_keyboard(void);
static void lkeybd_exit_keyboard(void);
static ALLEGRO_KEYBOARD *lkeybd_get_keyboard(void);
static bool lkeybd_set_keyboard_leds(int leds);
static void lkeybd_get_keyboard_state(ALLEGRO_KEYBOARD_STATE *ret_state);
static void lkeybd_clear_keyboard_state(void);

static void process_new_data(void *unused);
static void process_character(unsigned char ch);
static void handle_key_press(int mycode, unsigned int ascii);
static void handle_key_release(int mycode);




#define KEYDRV_LINUX AL_ID('L','N','X','C')

static ALLEGRO_KEYBOARD_DRIVER keydrv_linux =
{
KEYDRV_LINUX,
"",
"",
"Linux console keyboard",
lkeybd_init_keyboard,
lkeybd_exit_keyboard,
lkeybd_get_keyboard,
lkeybd_set_keyboard_leds,
NULL, 
lkeybd_get_keyboard_state,
lkeybd_clear_keyboard_state
};




_AL_DRIVER_INFO _al_linux_keyboard_driver_list[] =
{
{ KEYDRV_LINUX, &keydrv_linux, true },
{ 0, NULL, 0 }
};








#define KB_MODIFIERS (ALLEGRO_KEYMOD_SHIFT | ALLEGRO_KEYMOD_CTRL | ALLEGRO_KEYMOD_ALT | ALLEGRO_KEYMOD_ALTGR | ALLEGRO_KEYMOD_LWIN | ALLEGRO_KEYMOD_RWIN | ALLEGRO_KEYMOD_MENU)



#define KB_LED_FLAGS (ALLEGRO_KEYMOD_SCROLLLOCK | ALLEGRO_KEYMOD_NUMLOCK | ALLEGRO_KEYMOD_CAPSLOCK)




static unsigned char kernel_to_mycode[128] =
{
0, ALLEGRO_KEY_ESCAPE, ALLEGRO_KEY_1, ALLEGRO_KEY_2,
ALLEGRO_KEY_3, ALLEGRO_KEY_4, ALLEGRO_KEY_5, ALLEGRO_KEY_6,
ALLEGRO_KEY_7, ALLEGRO_KEY_8, ALLEGRO_KEY_9, ALLEGRO_KEY_0,
ALLEGRO_KEY_MINUS, ALLEGRO_KEY_EQUALS, ALLEGRO_KEY_BACKSPACE, ALLEGRO_KEY_TAB,
ALLEGRO_KEY_Q, ALLEGRO_KEY_W, ALLEGRO_KEY_E, ALLEGRO_KEY_R,
ALLEGRO_KEY_T, ALLEGRO_KEY_Y, ALLEGRO_KEY_U, ALLEGRO_KEY_I,
ALLEGRO_KEY_O, ALLEGRO_KEY_P, ALLEGRO_KEY_OPENBRACE, ALLEGRO_KEY_CLOSEBRACE,
ALLEGRO_KEY_ENTER, ALLEGRO_KEY_LCTRL, ALLEGRO_KEY_A, ALLEGRO_KEY_S,
ALLEGRO_KEY_D, ALLEGRO_KEY_F, ALLEGRO_KEY_G, ALLEGRO_KEY_H,
ALLEGRO_KEY_J, ALLEGRO_KEY_K, ALLEGRO_KEY_L, ALLEGRO_KEY_SEMICOLON,
ALLEGRO_KEY_QUOTE, ALLEGRO_KEY_TILDE, ALLEGRO_KEY_LSHIFT, ALLEGRO_KEY_BACKSLASH,
ALLEGRO_KEY_Z, ALLEGRO_KEY_X, ALLEGRO_KEY_C, ALLEGRO_KEY_V,
ALLEGRO_KEY_B, ALLEGRO_KEY_N, ALLEGRO_KEY_M, ALLEGRO_KEY_COMMA,
ALLEGRO_KEY_FULLSTOP, ALLEGRO_KEY_SLASH, ALLEGRO_KEY_RSHIFT, ALLEGRO_KEY_PAD_ASTERISK,
ALLEGRO_KEY_ALT, ALLEGRO_KEY_SPACE, ALLEGRO_KEY_CAPSLOCK, ALLEGRO_KEY_F1,
ALLEGRO_KEY_F2, ALLEGRO_KEY_F3, ALLEGRO_KEY_F4, ALLEGRO_KEY_F5,
ALLEGRO_KEY_F6, ALLEGRO_KEY_F7, ALLEGRO_KEY_F8, ALLEGRO_KEY_F9,
ALLEGRO_KEY_F10, ALLEGRO_KEY_NUMLOCK, ALLEGRO_KEY_SCROLLLOCK, ALLEGRO_KEY_PAD_7,
ALLEGRO_KEY_PAD_8, ALLEGRO_KEY_PAD_9, ALLEGRO_KEY_PAD_MINUS, ALLEGRO_KEY_PAD_4,
ALLEGRO_KEY_PAD_5, ALLEGRO_KEY_PAD_6, ALLEGRO_KEY_PAD_PLUS, ALLEGRO_KEY_PAD_1,
ALLEGRO_KEY_PAD_2, ALLEGRO_KEY_PAD_3, ALLEGRO_KEY_PAD_0, ALLEGRO_KEY_PAD_DELETE,
ALLEGRO_KEY_PRINTSCREEN, 0, ALLEGRO_KEY_BACKSLASH2, ALLEGRO_KEY_F11,
ALLEGRO_KEY_F12, 0, 0, 0,
0, 0, 0, 0,
ALLEGRO_KEY_PAD_ENTER, ALLEGRO_KEY_RCTRL, ALLEGRO_KEY_PAD_SLASH, ALLEGRO_KEY_PRINTSCREEN,
ALLEGRO_KEY_ALTGR, ALLEGRO_KEY_PAUSE, ALLEGRO_KEY_HOME, ALLEGRO_KEY_UP,
ALLEGRO_KEY_PGUP, ALLEGRO_KEY_LEFT, ALLEGRO_KEY_RIGHT, ALLEGRO_KEY_END,
ALLEGRO_KEY_DOWN, ALLEGRO_KEY_PGDN, ALLEGRO_KEY_INSERT, ALLEGRO_KEY_DELETE,
0, 0, 0, 0,
0, 0, 0, ALLEGRO_KEY_PAUSE,
0, 0, 0, 0,
0, ALLEGRO_KEY_LWIN, ALLEGRO_KEY_RWIN, ALLEGRO_KEY_MENU








};



#define NUM_PAD_KEYS 17
static const char pad_asciis[NUM_PAD_KEYS] =
{
'0','1','2','3','4','5','6','7','8','9',
'+','-','*','/','\r',',','.'
};
static const char pad_asciis_no_numlock[NUM_PAD_KEYS] =
{
0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
'+', '-', '*', '/', '\r', 0, 0
};



static const unsigned int modifier_table[ALLEGRO_KEY_MAX - ALLEGRO_KEY_MODIFIERS] =
{
ALLEGRO_KEYMOD_SHIFT, ALLEGRO_KEYMOD_SHIFT, ALLEGRO_KEYMOD_CTRL,
ALLEGRO_KEYMOD_CTRL, ALLEGRO_KEYMOD_ALT, ALLEGRO_KEYMOD_ALTGR,
ALLEGRO_KEYMOD_LWIN, ALLEGRO_KEYMOD_RWIN, ALLEGRO_KEYMOD_MENU,
ALLEGRO_KEYMOD_SCROLLLOCK, ALLEGRO_KEYMOD_NUMLOCK, ALLEGRO_KEYMOD_CAPSLOCK
};












static int keycode_to_char(int keycode)
{
const unsigned int modifiers = the_keyboard.modifiers;
struct kbentry kbe;
int keymap;
int ascii;


keymap = 0;
if (modifiers & ALLEGRO_KEYMOD_SHIFT) keymap |= 1;
if (modifiers & ALLEGRO_KEYMOD_ALTGR) keymap |= 2;
if (modifiers & ALLEGRO_KEYMOD_CTRL) keymap |= 4;
if (modifiers & ALLEGRO_KEYMOD_ALT) keymap |= 8;


kbe.kb_table = keymap;
kbe.kb_index = keycode;
ioctl(the_keyboard.fd, KDGKBENT, &kbe);

if (keycode == KEY_BACKSPACE)
ascii = 8;
else {
if (kbe.kb_value == K_NOSUCHMAP) {



ascii = 0;
}
else {

ascii = KVAL(kbe.kb_value);
}
}


switch (KTYP(kbe.kb_value)) {

case KT_CONS:

return -( KVAL(kbe.kb_value)+1 );

case KT_LETTER:

if (modifiers & ALLEGRO_KEYMOD_CAPSLOCK)
return ascii ^ 0x20;
else
return ascii;

case KT_LATIN:
case KT_ASCII:
return ascii;

case KT_PAD:
{
int val = KVAL(kbe.kb_value);
if (modifiers & ALLEGRO_KEYMOD_NUMLOCK) {
if ((val >= 0) && (val < NUM_PAD_KEYS))
ascii = pad_asciis[val];
} else {
if ((val >= 0) && (val < NUM_PAD_KEYS))
ascii = pad_asciis_no_numlock[val];
}
return ascii;
}

case KT_SPEC:
if (keycode == KEY_ENTER)
return '\r';
else
return 0;

default:

return 0;
}
}






static bool lkeybd_init_keyboard(void)
{
bool can_restore_termio_and_kbmode = false;

memset(&the_keyboard, 0, sizeof the_keyboard);





the_keyboard.fd = open("/dev/tty", O_RDWR);




if (tcgetattr(the_keyboard.fd, &the_keyboard.startup_termio) != 0) {
goto Error;
}


if (ioctl(the_keyboard.fd, KDGKBMODE, &the_keyboard.startup_kbmode) != 0) {

}

can_restore_termio_and_kbmode = false;



















the_keyboard.work_termio = the_keyboard.startup_termio;
the_keyboard.work_termio.c_iflag &= ~(ISTRIP | IGNCR | ICRNL | INLCR | IXOFF | IXON);
the_keyboard.work_termio.c_cflag &= ~CSIZE;
the_keyboard.work_termio.c_cflag |= CS8;
the_keyboard.work_termio.c_lflag &= ~(ICANON | ECHO | ISIG);

if (tcsetattr(the_keyboard.fd, TCSANOW, &the_keyboard.work_termio) != 0) {
goto Error;
}


if (ioctl(the_keyboard.fd, KDSKBMODE, K_MEDIUMRAW) != 0) {

}

the_keyboard.three_finger_flag = true;
the_keyboard.key_led_flag = true;

const char *value = al_get_config_value(al_get_system_config(),
"keyboard", "enable_three_finger_exit");
if (value) {
the_keyboard.three_finger_flag = !strncmp(value, "true", 4);
}
value = al_get_config_value(al_get_system_config(),
"keyboard", "enable_key_led_toggle");
if (value) {
the_keyboard.key_led_flag = !strncmp(value, "true", 4);
}

ALLEGRO_DEBUG("Three finger flag enabled: %s\n",
the_keyboard.three_finger_flag ? "true" : "false");
ALLEGRO_DEBUG("Key LED toggle enabled: %s\n",
the_keyboard.key_led_flag ? "true" : "false");


_al_event_source_init(&the_keyboard.parent.es);


_al_unix_start_watching_fd(the_keyboard.fd, process_new_data, NULL);


main_pid = getpid();

return true;

Error:

if (can_restore_termio_and_kbmode) {
tcsetattr(the_keyboard.fd, TCSANOW, &the_keyboard.startup_termio);
ioctl(the_keyboard.fd, KDSKBMODE, the_keyboard.startup_kbmode);
}

close(the_keyboard.fd);





return false;
}






static void lkeybd_exit_keyboard(void)
{
_al_unix_stop_watching_fd(the_keyboard.fd);

_al_event_source_free(&the_keyboard.parent.es);


tcsetattr(the_keyboard.fd, TCSANOW, &the_keyboard.startup_termio);
ioctl(the_keyboard.fd, KDSKBMODE, the_keyboard.startup_kbmode);
ioctl(the_keyboard.fd, KDSETLED, 8);

close(the_keyboard.fd);






memset(&the_keyboard, 0, sizeof the_keyboard);
}






static ALLEGRO_KEYBOARD *lkeybd_get_keyboard(void)
{
return (ALLEGRO_KEYBOARD *)&the_keyboard;
}






static bool lkeybd_set_keyboard_leds(int leds)
{
int val = 0;

if (leds & ALLEGRO_KEYMOD_SCROLLLOCK) val |= LED_SCR;
if (leds & ALLEGRO_KEYMOD_NUMLOCK) val |= LED_NUM;
if (leds & ALLEGRO_KEYMOD_CAPSLOCK) val |= LED_CAP;

return (ioctl(the_keyboard.fd, KDSETLED, val) == 0) ? true : false;
}






static void lkeybd_get_keyboard_state(ALLEGRO_KEYBOARD_STATE *ret_state)
{
_al_event_source_lock(&the_keyboard.parent.es);
{
*ret_state = the_keyboard.state;
}
_al_event_source_unlock(&the_keyboard.parent.es);
}






static void lkeybd_clear_keyboard_state(void)
{
_al_event_source_lock(&the_keyboard.parent.es);
{
memset(&the_keyboard.state, 0, sizeof(the_keyboard.state));
}
_al_event_source_unlock(&the_keyboard.parent.es);
}






static void process_new_data(void *unused)
{
_al_event_source_lock(&the_keyboard.parent.es);
{
unsigned char buf[128];
size_t bytes_read;
unsigned int ch;

bytes_read = read(the_keyboard.fd, &buf, sizeof(buf));
for (ch = 0; ch < bytes_read; ch++)
process_character(buf[ch]);
}
_al_event_source_unlock(&the_keyboard.parent.es);

(void)unused;
}











static void process_character(unsigned char ch)
{

int keycode = ch & 0x7f;
bool press = !(ch & 0x80);
int mycode = kernel_to_mycode[keycode];


if (mycode == 0)
return;


if (mycode >= ALLEGRO_KEY_MODIFIERS) {
int flag = modifier_table[mycode - ALLEGRO_KEY_MODIFIERS];
if (press) {
if (flag & KB_MODIFIERS)
the_keyboard.modifiers |= flag;
else if ((flag & KB_LED_FLAGS) && the_keyboard.key_led_flag)
the_keyboard.modifiers ^= flag;
}
else {



if (flag & KB_MODIFIERS)
the_keyboard.modifiers &= ~flag;
}
}


if (press) {
int ascii = keycode_to_char(keycode);


if (ascii < 0) {
int console = -ascii;
int last_console;

ioctl(the_keyboard.fd, VT_OPENQRY, &last_console);
if (console < last_console)
if (ioctl(the_keyboard.fd, VT_ACTIVATE, console) == 0)
return;
}

handle_key_press(mycode, ascii);
}
else {
handle_key_release(mycode);
}


if ((the_keyboard.three_finger_flag)
&& ((mycode == ALLEGRO_KEY_DELETE) || (mycode == ALLEGRO_KEY_END))
&& (the_keyboard.modifiers & ALLEGRO_KEYMOD_CTRL)
&& (the_keyboard.modifiers & ALLEGRO_KEYMOD_ALT))
{
kill(main_pid, SIGTERM);
}
}






static void handle_key_press(int mycode, unsigned int ascii)
{
ALLEGRO_EVENT_TYPE event_type;
ALLEGRO_EVENT event;

event_type = (_AL_KEYBOARD_STATE_KEY_DOWN(the_keyboard.state, mycode)
? ALLEGRO_EVENT_KEY_CHAR
: ALLEGRO_EVENT_KEY_DOWN);


_AL_KEYBOARD_STATE_SET_KEY_DOWN(the_keyboard.state, mycode);


if (!_al_event_source_needs_to_generate_event(&the_keyboard.parent.es))
return;

event.keyboard.type = event_type;
event.keyboard.timestamp = al_get_time();
event.keyboard.display = NULL;
event.keyboard.keycode = mycode;
event.keyboard.unichar = ascii;
event.keyboard.modifiers = the_keyboard.modifiers;

_al_event_source_emit_event(&the_keyboard.parent.es, &event);


if (event_type == ALLEGRO_EVENT_KEY_DOWN) {
event.keyboard.type = ALLEGRO_EVENT_KEY_CHAR;
event.keyboard.timestamp = al_get_time();
_al_event_source_emit_event(&the_keyboard.parent.es, &event);
}
}






static void handle_key_release(int mycode)
{
ALLEGRO_EVENT event;




if (!_AL_KEYBOARD_STATE_KEY_DOWN(the_keyboard.state, mycode))
return;


_AL_KEYBOARD_STATE_CLEAR_KEY_DOWN(the_keyboard.state, mycode);


if (!_al_event_source_needs_to_generate_event(&the_keyboard.parent.es))
return;

event.keyboard.type = ALLEGRO_EVENT_KEY_UP;
event.keyboard.timestamp = al_get_time();
event.keyboard.display = NULL;
event.keyboard.keycode = mycode;
event.keyboard.unichar = 0;
event.keyboard.modifiers = 0;

_al_event_source_emit_event(&the_keyboard.parent.es, &event);
}









