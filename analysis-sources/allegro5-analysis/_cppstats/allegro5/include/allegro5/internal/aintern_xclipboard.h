#include "allegro5/internal/aintern_display.h"
void _al_xwin_display_selection_notify(ALLEGRO_DISPLAY *display, XSelectionEvent *xselection);
void _al_xwin_display_selection_request(ALLEGRO_DISPLAY *display, XSelectionRequestEvent *xselectionrequest);
void _al_xwin_add_clipboard_functions(ALLEGRO_DISPLAY_INTERFACE *vt);
