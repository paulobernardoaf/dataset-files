#define ALLEGRO_NO_COMPATIBILITY
#include <X11/Xlib.h>
#include <stdio.h>
#include "allegro5/allegro.h"
#include "allegro5/internal/aintern.h"
#include "allegro5/internal/aintern_mouse.h"
#include "allegro5/internal/aintern_x.h"
#include "allegro5/internal/aintern_xdisplay.h"
#include "allegro5/internal/aintern_xmouse.h"
#include "allegro5/internal/aintern_xsystem.h"
#if defined(ALLEGRO_RASPBERRYPI)
#include "allegro5/internal/aintern_raspberrypi.h"
#include "allegro5/internal/aintern_vector.h"
#define ALLEGRO_SYSTEM_XGLX ALLEGRO_SYSTEM_RASPBERRYPI
#define ALLEGRO_DISPLAY_XGLX ALLEGRO_DISPLAY_RASPBERRYPI
#endif
ALLEGRO_DEBUG_CHANNEL("mouse")
typedef struct ALLEGRO_MOUSE_XWIN
{
ALLEGRO_MOUSE parent;
ALLEGRO_MOUSE_STATE state;
int min_x, min_y;
int max_x, max_y;
} ALLEGRO_MOUSE_XWIN;
static bool xmouse_installed = false;
static ALLEGRO_MOUSE_XWIN the_mouse;
static bool xmouse_init(void);
static void xmouse_exit(void);
static ALLEGRO_MOUSE *xmouse_get_mouse(void);
static unsigned int xmouse_get_mouse_num_buttons(void);
static unsigned int xmouse_get_mouse_num_axes(void);
static bool xmouse_set_mouse_xy(ALLEGRO_DISPLAY *,int x, int y);
static bool xmouse_set_mouse_axis(int which, int z);
static void xmouse_get_state(ALLEGRO_MOUSE_STATE *ret_state);
static void wheel_motion_handler(int x_button, ALLEGRO_DISPLAY *display);
static unsigned int x_button_to_al_button(unsigned int x_button);
static void generate_mouse_event(unsigned int type,
int x, int y, int z, int w, float pressure,
int dx, int dy, int dz, int dw,
unsigned int button,
ALLEGRO_DISPLAY *display);
#define MOUSEDRV_XWIN AL_ID('X','W','I','N')
static ALLEGRO_MOUSE_DRIVER mousedrv_xwin =
{
MOUSEDRV_XWIN,
"",
"",
"X-Windows mouse",
xmouse_init,
xmouse_exit,
xmouse_get_mouse,
xmouse_get_mouse_num_buttons,
xmouse_get_mouse_num_axes,
xmouse_set_mouse_xy,
xmouse_set_mouse_axis,
xmouse_get_state
};
ALLEGRO_MOUSE_DRIVER *_al_xwin_mouse_driver(void)
{
return &mousedrv_xwin;
}
static bool xmouse_init(void)
{
ALLEGRO_SYSTEM_XGLX *system = (void *)al_get_system_driver();
ALLEGRO_DISPLAY *display;
int x, y;
if (system->x11display == NULL)
return false;
if (xmouse_installed)
return false;
display = the_mouse.state.display;
x = the_mouse.state.x;
y = the_mouse.state.y;
memset(&the_mouse, 0, sizeof the_mouse);
the_mouse.state.display = display;
the_mouse.state.x = x;
the_mouse.state.y = y;
_al_event_source_init(&the_mouse.parent.es);
xmouse_installed = true;
return true;
}
static void xmouse_exit(void)
{
if (!xmouse_installed)
return;
xmouse_installed = false;
_al_event_source_free(&the_mouse.parent.es);
}
static ALLEGRO_MOUSE *xmouse_get_mouse(void)
{
ASSERT(xmouse_installed);
return (ALLEGRO_MOUSE *)&the_mouse;
}
static unsigned int xmouse_get_mouse_num_buttons(void)
{
int num_buttons;
unsigned char map[32];
ALLEGRO_SYSTEM_XGLX *system = (void *)al_get_system_driver();
ASSERT(xmouse_installed);
_al_mutex_lock(&system->lock);
num_buttons = XGetPointerMapping(system->x11display, map, sizeof(map));
_al_mutex_unlock(&system->lock);
if (num_buttons > (int)sizeof(map))
num_buttons = sizeof(map);
#if defined(DEBUGMODE)
char debug[num_buttons * 4 + 1];
debug[0] = 0;
int i;
for (i = 0; i < num_buttons; i++) {
sprintf(debug + strlen(debug), "%2d,", map[i]);
}
ALLEGRO_DEBUG("XGetPointerMapping: %s\n", debug);
#endif
if (num_buttons < 1)
num_buttons = 1;
return num_buttons;
}
static unsigned int xmouse_get_mouse_num_axes(void)
{
ASSERT(xmouse_installed);
return 4;
}
static bool xmouse_set_mouse_xy(ALLEGRO_DISPLAY *display, int x, int y)
{
if (!xmouse_installed)
return false;
ALLEGRO_SYSTEM_XGLX *system = (void *)al_get_system_driver();
Display *x11display = system->x11display;
ALLEGRO_DISPLAY_XGLX *d = (void *)display;
int window_width = al_get_display_width(display);
int window_height = al_get_display_height(display);
if (x < 0 || y < 0 || x >= window_width || y >= window_height)
return false;
the_mouse.state.x = x;
the_mouse.state.y = y;
#if defined(ALLEGRO_RASPBERRYPI)
float scale_x, scale_y;
_al_raspberrypi_get_mouse_scale_ratios(&scale_x, &scale_y);
x /= scale_x;
y /= scale_y;
#endif
_al_mutex_lock(&system->lock);
XEvent event;
memset(&event, 0, sizeof event);
event.xclient.type = ClientMessage;
event.xclient.serial = 0;
event.xclient.send_event = True;
event.xclient.display = x11display;
event.xclient.window = d->window;
event.xclient.message_type = system->AllegroAtom;
event.xclient.format = 32;
XSendEvent(x11display, d->window, False, NoEventMask, &event);
XWarpPointer(x11display, None, d->window, 0, 0, 0, 0, x, y);
_al_mutex_unlock(&system->lock);
return true;
}
static bool xmouse_set_mouse_axis(int which, int v)
{
ASSERT(xmouse_installed);
if (which != 2 && which != 3) {
return false;
}
_al_event_source_lock(&the_mouse.parent.es);
{
int z = which == 2 ? v : the_mouse.state.z;
int w = which == 3 ? v : the_mouse.state.w;
int dz = z - the_mouse.state.z;
int dw = w - the_mouse.state.w;
if (dz != 0 || dw != 0) {
the_mouse.state.z = z;
the_mouse.state.w = w;
generate_mouse_event(
ALLEGRO_EVENT_MOUSE_AXES,
the_mouse.state.x, the_mouse.state.y, the_mouse.state.z,
the_mouse.state.w, the_mouse.state.pressure,
0, 0, dz, dw,
0, the_mouse.state.display);
}
}
_al_event_source_unlock(&the_mouse.parent.es);
return true;
}
static void xmouse_get_state(ALLEGRO_MOUSE_STATE *ret_state)
{
ASSERT(xmouse_installed);
_al_event_source_lock(&the_mouse.parent.es);
{
*ret_state = the_mouse.state;
}
_al_event_source_unlock(&the_mouse.parent.es);
}
void _al_xwin_mouse_button_press_handler(int x_button,
ALLEGRO_DISPLAY *display)
{
unsigned int al_button;
if (!xmouse_installed)
return;
wheel_motion_handler(x_button, display);
al_button = x_button_to_al_button(x_button);
if (al_button == 0)
return;
_al_event_source_lock(&the_mouse.parent.es);
{
the_mouse.state.buttons |= (1 << (al_button - 1));
the_mouse.state.pressure = the_mouse.state.buttons ? 1.0 : 0.0; 
generate_mouse_event(
ALLEGRO_EVENT_MOUSE_BUTTON_DOWN,
the_mouse.state.x, the_mouse.state.y, the_mouse.state.z,
the_mouse.state.w, the_mouse.state.pressure,
0, 0, 0, 0,
al_button, display);
}
_al_event_source_unlock(&the_mouse.parent.es);
}
static void wheel_motion_handler(int x_button, ALLEGRO_DISPLAY *display)
{
int dz = 0, dw = 0;
if (x_button == Button4) dz = 1;
if (x_button == Button5) dz = -1;
if (x_button == 6) dw = -1;
if (x_button == 7) dw = 1;
if (dz == 0 && dw == 0) return;
dz *= al_get_mouse_wheel_precision();
dw *= al_get_mouse_wheel_precision();
_al_event_source_lock(&the_mouse.parent.es);
{
the_mouse.state.z += dz;
the_mouse.state.w += dw;
generate_mouse_event(
ALLEGRO_EVENT_MOUSE_AXES,
the_mouse.state.x, the_mouse.state.y, the_mouse.state.z,
the_mouse.state.w, the_mouse.state.pressure,
0, 0, dz, dw,
0, display);
}
_al_event_source_unlock(&the_mouse.parent.es);
}
void _al_xwin_mouse_button_release_handler(int x_button,
ALLEGRO_DISPLAY *display)
{
int al_button;
if (!xmouse_installed)
return;
al_button = x_button_to_al_button(x_button);
if (al_button == 0)
return;
_al_event_source_lock(&the_mouse.parent.es);
{
the_mouse.state.buttons &=~ (1 << (al_button - 1));
the_mouse.state.pressure = the_mouse.state.buttons ? 1.0 : 0.0; 
generate_mouse_event(
ALLEGRO_EVENT_MOUSE_BUTTON_UP,
the_mouse.state.x, the_mouse.state.y, the_mouse.state.z,
the_mouse.state.w, the_mouse.state.pressure,
0, 0, 0, 0,
al_button, display);
}
_al_event_source_unlock(&the_mouse.parent.es);
}
void _al_xwin_mouse_motion_notify_handler(int x, int y,
ALLEGRO_DISPLAY *display)
{
ALLEGRO_DISPLAY_XGLX *glx = (void *)display;
int event_type = ALLEGRO_EVENT_MOUSE_AXES;
if (!xmouse_installed)
return;
if (glx->mouse_warp) {
glx->mouse_warp = false;
event_type = ALLEGRO_EVENT_MOUSE_WARPED;
}
_al_event_source_lock(&the_mouse.parent.es);
#if defined(ALLEGRO_RASPBERRYPI)
float scale_x, scale_y;
_al_raspberrypi_get_mouse_scale_ratios(&scale_x, &scale_y);
x *= scale_x;
y *= scale_y;
#endif
int dx = x - the_mouse.state.x;
int dy = y - the_mouse.state.y;
the_mouse.state.x = x;
the_mouse.state.y = y;
the_mouse.state.display = display;
generate_mouse_event(
event_type,
the_mouse.state.x, the_mouse.state.y, the_mouse.state.z,
the_mouse.state.w, the_mouse.state.pressure,
dx, dy, 0, 0,
0, display);
_al_event_source_unlock(&the_mouse.parent.es);
}
static unsigned int x_button_to_al_button(unsigned int x_button)
{
switch (x_button) {
case Button1:
return 1;
case Button2:
return 3;
case Button3:
return 2;
case Button4:
case Button5:
case 6:
case 7:
return 0;
default:
if (x_button >= 8 && x_button <= 36) {
return 4 + x_button - 8;
}
return 0;
}
}
static void generate_mouse_event(unsigned int type,
int x, int y, int z, int w, float pressure,
int dx, int dy, int dz, int dw,
unsigned int button,
ALLEGRO_DISPLAY *display)
{
ALLEGRO_EVENT event;
if (!_al_event_source_needs_to_generate_event(&the_mouse.parent.es))
return;
event.mouse.type = type;
event.mouse.timestamp = al_get_time();
event.mouse.display = display;
event.mouse.x = x;
event.mouse.y = y;
event.mouse.z = z;
event.mouse.w = w;
event.mouse.dx = dx;
event.mouse.dy = dy;
event.mouse.dz = dz;
event.mouse.dw = dw;
event.mouse.button = button;
event.mouse.pressure = pressure;
_al_event_source_emit_event(&the_mouse.parent.es, &event);
}
void _al_xwin_mouse_switch_handler(ALLEGRO_DISPLAY *display,
const XCrossingEvent *event)
{
int event_type;
if (event->state & (Button1Mask | Button2Mask | Button3Mask |
Button4Mask | Button5Mask)) {
return;
}
_al_event_source_lock(&the_mouse.parent.es);
switch (event->type) {
case EnterNotify:
the_mouse.state.display = display;
the_mouse.state.x = event->x;
the_mouse.state.y = event->y;
event_type = ALLEGRO_EVENT_MOUSE_ENTER_DISPLAY;
break;
case LeaveNotify:
the_mouse.state.display = NULL;
the_mouse.state.x = event->x;
the_mouse.state.y = event->y;
event_type = ALLEGRO_EVENT_MOUSE_LEAVE_DISPLAY;
break;
default:
ASSERT(false);
event_type = 0;
break;
}
generate_mouse_event(
event_type,
the_mouse.state.x, the_mouse.state.y, the_mouse.state.z, the_mouse.state.pressure,
the_mouse.state.w,
0, 0, 0, 0,
0, display);
_al_event_source_unlock(&the_mouse.parent.es);
}
bool _al_xwin_grab_mouse(ALLEGRO_DISPLAY *display)
{
ALLEGRO_SYSTEM_XGLX *system = (ALLEGRO_SYSTEM_XGLX *)al_get_system_driver();
ALLEGRO_DISPLAY_XGLX *glx = (ALLEGRO_DISPLAY_XGLX *)display;
int grab;
bool ret;
_al_mutex_lock(&system->lock);
grab = XGrabPointer(system->x11display, glx->window, False,
PointerMotionMask | ButtonPressMask | ButtonReleaseMask,
GrabModeAsync, GrabModeAsync, glx->window, None, CurrentTime);
if (grab == GrabSuccess) {
system->mouse_grab_display = display;
ret = true;
}
else {
ret = false;
}
_al_mutex_unlock(&system->lock);
return ret;
}
bool _al_xwin_ungrab_mouse(void)
{
ALLEGRO_SYSTEM_XGLX *system = (void *)al_get_system_driver();
_al_mutex_lock(&system->lock);
XUngrabPointer(system->x11display, CurrentTime);
system->mouse_grab_display = NULL;
_al_mutex_unlock(&system->lock);
return true;
}
