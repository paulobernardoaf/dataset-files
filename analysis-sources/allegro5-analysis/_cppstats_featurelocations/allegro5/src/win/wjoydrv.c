


















#include "allegro5/allegro.h"
#include "allegro5/internal/aintern.h"
#include "allegro5/internal/aintern_joystick.h"

#if !defined(ALLEGRO_WINDOWS)
#error something is wrong with the makefile
#endif



_AL_BEGIN_JOYSTICK_DRIVER_LIST
_AL_JOYSTICK_DRIVER_DIRECTX
#if defined(ALLEGRO_CFG_XINPUT)
_AL_JOYSTICK_DRIVER_XINPUT
_AL_JOYSTICK_DRIVER_WINDOWS_ALL
#endif
_AL_END_JOYSTICK_DRIVER_LIST
