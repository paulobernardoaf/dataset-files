

















#include "allegro5/allegro.h"
#include "allegro5/internal/aintern.h"
#include "allegro5/internal/aintern_driver.h"
#include "allegro5/platform/aintunix.h"
#include "allegro5/platform/aintlnx.h"

#if defined(ALLEGRO_RASPBERRYPI)
#include "allegro5/internal/aintern_raspberrypi.h"
#endif


_AL_DRIVER_INFO _al_linux_mouse_driver_list[] =
{









#if defined ALLEGRO_HAVE_LINUX_INPUT_H || defined ALLEGRO_RASPBERRYPI
{ AL_MOUSEDRV_LINUX_EVDEV, &_al_mousedrv_linux_evdev, true },
#endif
{ 0, NULL, 0 }
};

