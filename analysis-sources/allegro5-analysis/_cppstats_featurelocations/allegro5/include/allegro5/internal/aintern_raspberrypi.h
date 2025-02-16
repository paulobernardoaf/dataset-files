#if !defined(_ALLEGRO_INTERNAL_RASPBERRYPI_SYSTEM)
#define _ALLEGRO_INTERNAL_RASPBERRYPI_SYSTEM

#include "allegro5/allegro.h"
#include <allegro5/internal/aintern_system.h>
#include <allegro5/internal/aintern_display.h>
#include <allegro5/internal/aintern_mouse.h>

#include <X11/Xlib.h>

typedef struct ALLEGRO_SYSTEM_RASPBERRYPI {
ALLEGRO_SYSTEM system;
Display *x11display;
_AL_MUTEX lock; 
_AL_THREAD thread; 
ALLEGRO_DISPLAY *mouse_grab_display; 
bool inhibit_screensaver; 
Atom AllegroAtom;
} ALLEGRO_SYSTEM_RASPBERRYPI;

typedef struct ALLEGRO_DISPLAY_RASPBERRYPI_EXTRA
ALLEGRO_DISPLAY_RASPBERRYPI_EXTRA;

typedef struct ALLEGRO_DISPLAY_RASPBERRYPI {
ALLEGRO_DISPLAY display;
ALLEGRO_DISPLAY_RASPBERRYPI_EXTRA *extra;
Window window;

int screen_width, screen_height;

bool mouse_warp;
uint32_t *cursor_data;
int cursor_width;
int cursor_height;
int cursor_offset_x, cursor_offset_y;
Atom wm_delete_window_atom;
} ALLEGRO_DISPLAY_RASPBERRYPI;

typedef struct ALLEGRO_MOUSE_CURSOR_RASPBERRYPI {
ALLEGRO_BITMAP *bitmap;
} ALLEGRO_MOUSE_CURSOR_RASPBERRYPI;

ALLEGRO_SYSTEM_INTERFACE *_al_system_raspberrypi_driver(void);
ALLEGRO_DISPLAY_INTERFACE *_al_get_raspberrypi_display_interface(void);
void _al_raspberrypi_get_screen_info(int *dx, int *dy,
int *screen_width, int *screen_height);

bool _al_evdev_set_mouse_range(int x1, int y1, int x2, int y2); 
void _al_raspberrypi_get_mouse_scale_ratios(float *x, float *y); 

ALLEGRO_MOUSE_DRIVER _al_mousedrv_linux_evdev;

#endif
