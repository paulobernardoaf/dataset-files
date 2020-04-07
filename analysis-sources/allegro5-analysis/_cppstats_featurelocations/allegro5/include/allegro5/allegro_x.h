















#if !defined(__al_included_allegro5_allegro_x_h)
#define __al_included_allegro5_allegro_x_h

#include <X11/Xlib.h>

#include "allegro5/base.h"
#include "allegro5/display.h"

#if defined(__cplusplus)
extern "C" {
#endif




AL_FUNC(XID, al_get_x_window_id, (ALLEGRO_DISPLAY *display));
#if defined(ALLEGRO_UNSTABLE) || defined(ALLEGRO_INTERNAL_UNSTABLE) || defined(ALLEGRO_SRC)
AL_FUNC(bool, al_x_set_initial_icon, (ALLEGRO_BITMAP *bitmap));
#endif

#if defined(__cplusplus)
}
#endif

#endif


