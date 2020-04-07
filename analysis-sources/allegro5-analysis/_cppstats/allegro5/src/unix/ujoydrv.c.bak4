

















#include "allegro5/allegro.h"
#include "allegro5/platform/aintunix.h"
#include "allegro5/internal/aintern.h"
#include "allegro5/internal/aintern_joystick.h"



_AL_BEGIN_JOYSTICK_DRIVER_LIST
#if defined ALLEGRO_HAVE_LINUX_INPUT_H && (defined ALLEGRO_WITH_XWINDOWS || defined ALLEGRO_RASPBERRYPI)
{ _ALLEGRO_JOYDRV_LINUX, &_al_joydrv_linux, true },
#endif
_AL_END_JOYSTICK_DRIVER_LIST
