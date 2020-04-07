#include <allegro5/allegro.h>
#include "vcontroller.h"
VCONTROLLER *create_keyboard_controller(const char *config_path);
bool key_down(int k);
bool key_pressed(int k);
void keyboard_event(ALLEGRO_EVENT *event);
void keyboard_tick(void);
int unicode_char(bool remove);
