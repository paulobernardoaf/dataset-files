#if !defined(__al_included_allegro5_aintern_xdisplay_h)
#define __al_included_allegro5_aintern_xdisplay_h





#if !defined(ALLEGRO_RASPBERRYPI)

#include <GL/glx.h>

#include "allegro5/internal/aintern_display.h"
#include "allegro5/internal/aintern_x.h"

typedef struct ALLEGRO_DISPLAY_XGLX_GTK ALLEGRO_DISPLAY_XGLX_GTK;
typedef struct ALLEGRO_XWIN_DISPLAY_OVERRIDABLE_INTERFACE ALLEGRO_XWIN_DISPLAY_OVERRIDABLE_INTERFACE;


struct ALLEGRO_DISPLAY_XGLX
{

ALLEGRO_DISPLAY display;



const struct ALLEGRO_XWIN_DISPLAY_OVERRIDABLE_INTERFACE *overridable_vt;

Window window;
int xscreen; 
int adapter; 
GLXWindow glxwindow;
GLXContext context;
Atom wm_delete_window_atom;
XVisualInfo *xvinfo; 
GLXFBConfig *fbc; 
int glx_version; 




ALLEGRO_DISPLAY_XGLX_GTK *gtk;




Window embedder_window;

_AL_COND mapped; 
bool is_mapped; 

int resize_count; 
bool programmatic_resize; 


Cursor invisible_cursor;
Cursor current_cursor;
bool cursor_hidden;


Pixmap icon, icon_mask;


int x, y;


bool mouse_warp;

_AL_COND selectioned; 
bool is_selectioned; 

};

void _al_display_xglx_await_resize(ALLEGRO_DISPLAY *d, int old_resize_count, bool delay_hack);
void _al_xglx_display_configure(ALLEGRO_DISPLAY *d, int x, int y, int width, int height, bool setglxy);
void _al_xglx_display_configure_event(ALLEGRO_DISPLAY *d, XEvent *event);
void _al_xwin_display_switch_handler(ALLEGRO_DISPLAY *d,
XFocusChangeEvent *event);
void _al_xwin_display_switch_handler_inner(ALLEGRO_DISPLAY *d, bool focus_in);
void _al_xwin_display_expose(ALLEGRO_DISPLAY *display, XExposeEvent *xevent);





struct ALLEGRO_XWIN_DISPLAY_OVERRIDABLE_INTERFACE
{
bool (*create_display_hook)(ALLEGRO_DISPLAY *d, int w, int h);
void (*destroy_display_hook)(ALLEGRO_DISPLAY *d, bool is_last);
bool (*resize_display)(ALLEGRO_DISPLAY *d, int w, int h);
void (*set_window_title)(ALLEGRO_DISPLAY *display, const char *title);
void (*set_fullscreen_window)(ALLEGRO_DISPLAY *display, bool onoff);
void (*set_window_position)(ALLEGRO_DISPLAY *display, int x, int y);
bool (*set_window_constraints)(ALLEGRO_DISPLAY *display, int min_w, int min_h, int max_w, int max_h);
};

bool _al_xwin_set_gtk_display_overridable_interface(uint32_t check_version,
const ALLEGRO_XWIN_DISPLAY_OVERRIDABLE_INTERFACE *vt);

#endif 

#endif


