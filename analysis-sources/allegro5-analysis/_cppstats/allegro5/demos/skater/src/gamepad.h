#include <allegro5/allegro.h>
#include "vcontroller.h"
VCONTROLLER *create_gamepad_controller(const char *config_path);
void gamepad_event(ALLEGRO_EVENT *event);
bool gamepad_button(void);
