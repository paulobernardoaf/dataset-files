















#include "allegro5/allegro.h"
#include "allegro5/internal/aintern_system.h"




int al_get_num_display_modes(void)
{
ALLEGRO_SYSTEM *system = al_get_system_driver();
return system->vt->get_num_display_modes();
}




ALLEGRO_DISPLAY_MODE *al_get_display_mode(int index, ALLEGRO_DISPLAY_MODE *mode)
{
ALLEGRO_SYSTEM *system = al_get_system_driver();
return system->vt->get_display_mode(index, mode);
}



