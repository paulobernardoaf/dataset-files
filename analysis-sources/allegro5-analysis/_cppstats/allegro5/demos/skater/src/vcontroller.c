#include <allegro5/allegro.h>
#include <stdlib.h>
#include "vcontroller.h"
void destroy_vcontroller(VCONTROLLER * controller, const char *config_path)
{
if (controller == 0) {
return;
}
if (controller->write_config != 0) {
controller->write_config(controller, config_path);
}
if (controller->private_data != 0) {
free(controller->private_data);
controller->private_data = 0;
}
free(controller);
controller = 0;
}
