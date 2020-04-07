#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

#include "allegro5/allegro.h"
#include "allegro5/allegro_x.h"
#include "allegro5/internal/aintern_x.h"
#include "allegro5/internal/aintern_xdisplay.h"
#include "allegro5/internal/aintern_xsystem.h"
#include "allegro5/internal/aintern_xwindow.h"

#if defined(ALLEGRO_RASPBERRYPI)
#include "allegro5/internal/aintern_raspberrypi.h"
#define ALLEGRO_SYSTEM_XGLX ALLEGRO_SYSTEM_RASPBERRYPI
#define ALLEGRO_DISPLAY_XGLX ALLEGRO_DISPLAY_RASPBERRYPI
#endif

ALLEGRO_DEBUG_CHANNEL("xwindow")

#define X11_ATOM(x) XInternAtom(x11, #x, False);

void _al_xwin_set_size_hints(ALLEGRO_DISPLAY *d, int x_off, int y_off)
{
ALLEGRO_SYSTEM_XGLX *system = (void *)al_get_system_driver();
ALLEGRO_DISPLAY_XGLX *glx = (void *)d;
XSizeHints *sizehints;
XWMHints *wmhints;
XClassHint *classhints;
int w, h;

sizehints = XAllocSizeHints();
sizehints->flags = 0;

#if defined(ALLEGRO_RASPBERRYPI)
int x, y;
_al_raspberrypi_get_screen_info(&x, &y, &w, &h);
#else
w = d->w;
h = d->h;
#endif



if (!(d->flags & ALLEGRO_RESIZABLE) && !(d->flags & ALLEGRO_FULLSCREEN)) {
sizehints->flags |= PMinSize | PMaxSize | PBaseSize;
sizehints->min_width = sizehints->max_width = sizehints->base_width = w;
sizehints->min_height = sizehints->max_height = sizehints->base_height = h;
}


if (d->use_constraints && (d->flags & ALLEGRO_RESIZABLE) &&
(d->min_w > 0 || d->min_h > 0 || d->max_w > 0 || d->max_h > 0))
{
sizehints->flags |= PMinSize | PMaxSize | PBaseSize;
sizehints->min_width = (d->min_w > 0) ? d->min_w : 0;
sizehints->min_height = (d->min_h > 0) ? d->min_h : 0;
sizehints->max_width = (d->max_w > 0) ? d->max_w : INT_MAX;
sizehints->max_height = (d->max_h > 0) ? d->max_h : INT_MAX;
sizehints->base_width = w;
sizehints->base_height = h;
}



if (x_off != INT_MAX && y_off != INT_MAX) {
ALLEGRO_DEBUG("Force window position to %d, %d.\n", x_off, y_off);
sizehints->flags |= PPosition;
sizehints->x = x_off;
sizehints->y = y_off;
}

if (d->flags & ALLEGRO_FULLSCREEN) {




sizehints->flags |= PBaseSize;
sizehints->base_width = w;
sizehints->base_height = h;
}


wmhints = XAllocWMHints();
wmhints->input = True;
wmhints->flags = InputHint;

ALLEGRO_PATH *exepath = al_get_standard_path(ALLEGRO_EXENAME_PATH);




classhints = XAllocClassHint();
classhints->res_name = strdup(al_get_path_basename(exepath));
classhints->res_class = strdup(al_get_path_basename(exepath));


XSetWMProperties(system->x11display, glx->window, NULL, NULL, NULL, 0,
sizehints, wmhints, classhints);

free(classhints->res_name);
free(classhints->res_class);
XFree(sizehints);
XFree(wmhints);
XFree(classhints);

al_destroy_path(exepath);
}


void _al_xwin_reset_size_hints(ALLEGRO_DISPLAY *d)
{
ALLEGRO_SYSTEM_XGLX *system = (void *)al_get_system_driver();
ALLEGRO_DISPLAY_XGLX *glx = (void *)d;
XSizeHints *hints;

hints = XAllocSizeHints();
hints->flags = PMinSize | PMaxSize;
hints->min_width = 0;
hints->min_height = 0;

hints->max_width = 32768;
hints->max_height = 32768;
XSetWMNormalHints(system->x11display, glx->window, hints);

XFree(hints);
}





void _al_xwin_set_fullscreen_window(ALLEGRO_DISPLAY *display, int value)
{
ALLEGRO_SYSTEM_XGLX *system = (void *)al_get_system_driver();
ALLEGRO_DISPLAY_XGLX *glx = (ALLEGRO_DISPLAY_XGLX *)display;
Display *x11 = system->x11display;
#if !defined(ALLEGRO_RASPBERRYPI)
int old_resize_count = glx->resize_count;
#endif

ALLEGRO_DEBUG("Toggling _NET_WM_STATE_FULLSCREEN hint: %d\n", value);

XEvent xev;
xev.xclient.type = ClientMessage;
xev.xclient.serial = 0;
xev.xclient.send_event = True;
xev.xclient.message_type = X11_ATOM(_NET_WM_STATE);
xev.xclient.window = glx->window;
xev.xclient.format = 32;



xev.xclient.data.l[0] = value; 

xev.xclient.data.l[1] = X11_ATOM(_NET_WM_STATE_FULLSCREEN);
xev.xclient.data.l[2] = 0;
xev.xclient.data.l[3] = 0;
xev.xclient.data.l[4] = 1;

XSendEvent(
x11,
#if !defined ALLEGRO_RASPBERRYPI
RootWindowOfScreen(ScreenOfDisplay(x11, glx->xscreen)),
#else
RootWindowOfScreen(ScreenOfDisplay(x11, DefaultScreen(x11))),
#endif
False,
SubstructureRedirectMask | SubstructureNotifyMask, &xev);

#if !defined ALLEGRO_RASPBERRYPI
if (value == 2) {

_al_display_xglx_await_resize(display, old_resize_count, true);
}
#endif
}


void _al_xwin_set_above(ALLEGRO_DISPLAY *display, int value)
{
ALLEGRO_SYSTEM_XGLX *system = (void *)al_get_system_driver();
ALLEGRO_DISPLAY_XGLX *glx = (ALLEGRO_DISPLAY_XGLX *)display;
Display *x11 = system->x11display;

ALLEGRO_DEBUG("Toggling _NET_WM_STATE_ABOVE hint: %d\n", value);

XEvent xev;
xev.xclient.type = ClientMessage;
xev.xclient.serial = 0;
xev.xclient.send_event = True;
xev.xclient.message_type = X11_ATOM(_NET_WM_STATE);
xev.xclient.window = glx->window;
xev.xclient.format = 32;



xev.xclient.data.l[0] = value; 

xev.xclient.data.l[1] = X11_ATOM(_NET_WM_STATE_ABOVE);
xev.xclient.data.l[2] = 0;
xev.xclient.data.l[3] = 0;
xev.xclient.data.l[4] = 1;

XSendEvent(x11, DefaultRootWindow(x11), False,
SubstructureRedirectMask | SubstructureNotifyMask, &xev);
}


void _al_xwin_set_frame(ALLEGRO_DISPLAY *display, bool frame_on)
{
ALLEGRO_SYSTEM_XGLX *system = (void *)al_get_system_driver();
ALLEGRO_DISPLAY_XGLX *glx = (ALLEGRO_DISPLAY_XGLX *)display;
Display *x11 = system->x11display;
Atom hints;

_al_mutex_lock(&system->lock);

#if 1





hints = XInternAtom(x11, "_MOTIF_WM_HINTS", True);
if (hints) {
struct {
unsigned long flags;
unsigned long functions;
unsigned long decorations;
long input_mode;
unsigned long status;
} motif = {2, 0, frame_on, 0, 0};
XChangeProperty(x11, glx->window, hints, hints, 32, PropModeReplace,
(void *)&motif, sizeof motif / 4);

if (frame_on)
display->flags &= ~ALLEGRO_FRAMELESS;
else
display->flags |= ALLEGRO_FRAMELESS;
}
#endif

_al_mutex_unlock(&system->lock);
}









static bool xdpy_set_icon_inner(Display *x11display, Window window,
ALLEGRO_BITMAP *bitmap, int prop_mode)
{
int w, h;
int data_size;
unsigned long *data; 
ALLEGRO_LOCKED_REGION *lr;
bool ret;

w = al_get_bitmap_width(bitmap);
h = al_get_bitmap_height(bitmap);
data_size = 2 + w * h;
data = al_malloc(data_size * sizeof(data[0]));
if (!data)
return false;

lr = al_lock_bitmap(bitmap, ALLEGRO_PIXEL_FORMAT_ANY_WITH_ALPHA,
ALLEGRO_LOCK_READONLY);
if (lr) {
int x, y;
ALLEGRO_COLOR c;
unsigned char r, g, b, a;
Atom _NET_WM_ICON;

data[0] = w;
data[1] = h;
for (y = 0; y < h; y++) {
for (x = 0; x < w; x++) {
c = al_get_pixel(bitmap, x, y);
al_unmap_rgba(c, &r, &g, &b, &a);
data[2 + y*w + x] = (a << 24) | (r << 16) | (g << 8) | b;
}
}

_NET_WM_ICON = XInternAtom(x11display, "_NET_WM_ICON", False);
XChangeProperty(x11display, window, _NET_WM_ICON, XA_CARDINAL, 32,
prop_mode, (unsigned char *)data, data_size);

al_unlock_bitmap(bitmap);
ret = true;
}
else {
ret = false;
}

al_free(data);

return ret;
}


void _al_xwin_set_icons(ALLEGRO_DISPLAY *d,
int num_icons, ALLEGRO_BITMAP *bitmaps[])
{
ALLEGRO_SYSTEM_XGLX *system = (ALLEGRO_SYSTEM_XGLX *)al_get_system_driver();
ALLEGRO_DISPLAY_XGLX *glx = (ALLEGRO_DISPLAY_XGLX *)d;
int prop_mode = PropModeReplace;
int i;

_al_mutex_lock(&system->lock);

for (i = 0; i < num_icons; i++) {
if (xdpy_set_icon_inner(system->x11display, glx->window, bitmaps[i],
prop_mode)) {
prop_mode = PropModeAppend;
}
}

_al_mutex_unlock(&system->lock);
}


void _al_xwin_maximize(ALLEGRO_DISPLAY *display, bool maximized)
{
#if !defined(ALLEGRO_RASPBERRYPI)
if (!!(display->flags & ALLEGRO_MAXIMIZED) == maximized)
return;
ALLEGRO_SYSTEM_XGLX *system = (void *)al_get_system_driver();
ALLEGRO_DISPLAY_XGLX *glx = (ALLEGRO_DISPLAY_XGLX *)display;
Display *x11 = system->x11display;
int old_resize_count = glx->resize_count;

XEvent xev;
xev.xclient.type = ClientMessage;
xev.xclient.serial = 0;
xev.xclient.send_event = True;
xev.xclient.message_type = X11_ATOM(_NET_WM_STATE);
xev.xclient.window = glx->window;
xev.xclient.format = 32;

xev.xclient.data.l[0] = maximized ? 1 : 0;
xev.xclient.data.l[1] = X11_ATOM(_NET_WM_STATE_MAXIMIZED_HORZ);
xev.xclient.data.l[2] = X11_ATOM(_NET_WM_STATE_MAXIMIZED_VERT);
xev.xclient.data.l[3] = 0;

XSendEvent(
x11,
RootWindowOfScreen(ScreenOfDisplay(x11, glx->xscreen)),
False,
SubstructureRedirectMask | SubstructureNotifyMask, &xev);

_al_display_xglx_await_resize(display, old_resize_count, true);
#endif
}


void _al_xwin_check_maximized(ALLEGRO_DISPLAY *display)
{
#if !defined(ALLEGRO_RASPBERRYPI)
ALLEGRO_SYSTEM_XGLX *system = (void *)al_get_system_driver();
ALLEGRO_DISPLAY_XGLX *glx = (ALLEGRO_DISPLAY_XGLX *)display;
Display *x11 = system->x11display;
Atom type;
Atom horz = X11_ATOM(_NET_WM_STATE_MAXIMIZED_HORZ);
Atom vert = X11_ATOM(_NET_WM_STATE_MAXIMIZED_VERT);
Atom property = X11_ATOM(_NET_WM_STATE);
int format;
int maximized = 0;
unsigned long n, remaining, i, *p32;
unsigned char *p8 = NULL;
if (XGetWindowProperty(x11, glx->window, property, 0, INT_MAX,
False, AnyPropertyType, &type, &format, &n, &remaining, &p8)
!= Success) {
return;
}
p32 = (unsigned long *)p8;
for (i = 0; i < n; i++) {
if (p32[i] == horz)
maximized |= 1;
if (p32[i] == vert)
maximized |= 2;
}
XFree(p8);
display->flags &= ~ALLEGRO_MAXIMIZED;
if (maximized == 3)
display->flags |= ALLEGRO_MAXIMIZED;
#endif
}




XID al_get_x_window_id(ALLEGRO_DISPLAY *display)
{
ASSERT(display != NULL);
return ((ALLEGRO_DISPLAY_XGLX*)display)->window;
}


