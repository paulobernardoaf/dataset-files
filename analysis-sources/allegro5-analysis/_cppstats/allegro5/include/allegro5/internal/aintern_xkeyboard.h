#include "allegro5/internal/aintern_keyboard.h"
ALLEGRO_KEYBOARD_DRIVER *_al_xwin_keyboard_driver(void);
void _al_xwin_keyboard_handler(XKeyEvent *event, ALLEGRO_DISPLAY *display);
void _al_xwin_keyboard_switch_handler(ALLEGRO_DISPLAY *display, bool focus_in);
