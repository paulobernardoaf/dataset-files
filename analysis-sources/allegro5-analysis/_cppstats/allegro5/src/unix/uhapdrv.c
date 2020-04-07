#include "allegro5/allegro.h"
#include "allegro5/platform/aintunix.h"
#include "allegro5/internal/aintern.h"
#include "allegro5/internal/aintern_haptic.h"
_AL_BEGIN_HAPTIC_DRIVER_LIST
#if defined ALLEGRO_HAVE_LINUX_INPUT_H && (defined ALLEGRO_WITH_XWINDOWS || defined ALLEGRO_RASPBERRYPI)
{ _ALLEGRO_HAPDRV_LINUX, &_al_hapdrv_linux, true },
#endif
_AL_END_HAPTIC_DRIVER_LIST
