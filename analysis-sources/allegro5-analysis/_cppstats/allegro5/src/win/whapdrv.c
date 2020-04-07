#include "allegro5/allegro.h"
#include "allegro5/internal/aintern.h"
#include "allegro5/internal/aintern_haptic.h"
#if !defined(ALLEGRO_WINDOWS)
#error something is wrong with the makefile
#endif
_AL_BEGIN_HAPTIC_DRIVER_LIST
_AL_HAPTIC_DRIVER_DIRECTX
_AL_END_HAPTIC_DRIVER_LIST
