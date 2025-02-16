














#if !defined(__al_included_allegro5_clipboard_h)
#define __al_included_allegro5_clipboard_h

#include "allegro5/base.h"
#include "allegro5/display.h"
#include "allegro5/utf8.h"

#if defined(__cplusplus)
extern "C" {
#endif

AL_FUNC(char *, al_get_clipboard_text, (ALLEGRO_DISPLAY *display));
AL_FUNC(bool, al_set_clipboard_text, (ALLEGRO_DISPLAY *display, const char *text));
AL_FUNC(bool, al_clipboard_has_text, (ALLEGRO_DISPLAY *display));

#if defined(__cplusplus)
}
#endif

#endif







