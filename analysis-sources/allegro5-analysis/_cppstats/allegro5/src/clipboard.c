#include "allegro5/allegro.h"
#include "allegro5/internal/aintern.h"
#include "allegro5/internal/aintern_display.h"
char *al_get_clipboard_text(ALLEGRO_DISPLAY *display)
{
if (!display)
display = al_get_current_display();
if (!display)
return NULL;
if (!display->vt->get_clipboard_text)
return NULL;
return display->vt->get_clipboard_text(display);
}
bool al_set_clipboard_text(ALLEGRO_DISPLAY *display, const char *text)
{
if (!display)
display = al_get_current_display();
if (!display)
return false;
if (!display->vt->set_clipboard_text)
return false;
return display->vt->set_clipboard_text(display, text);
}
bool al_clipboard_has_text(ALLEGRO_DISPLAY *display)
{
if (!display)
display = al_get_current_display();
if (!display)
return false;
if (!display->vt->has_clipboard_text)
return false;
return display->vt->has_clipboard_text(display);
}
