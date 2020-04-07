
















#include "allegro5/allegro.h"
#include "allegro5/internal/aintern.h"
#include "allegro5/internal/aintern_bitmap.h"
#include "allegro5/internal/aintern_system.h"

#if defined ALLEGRO_WITH_XWINDOWS
#if !defined(ALLEGRO_RASPBERRYPI)
#include "allegro5/platform/aintxglx.h"
#else
#include "allegro5/internal/aintern_raspberrypi.h"
#endif
#endif






void _al_register_system_interfaces(void)
{
ALLEGRO_SYSTEM_INTERFACE **add;
#if defined ALLEGRO_WITH_XWINDOWS && !defined ALLEGRO_RASPBERRYPI
add = _al_vector_alloc_back(&_al_system_interfaces);
*add = _al_system_xglx_driver();
#elif defined ALLEGRO_RASPBERRYPI
add = _al_vector_alloc_back(&_al_system_interfaces);
*add = _al_system_raspberrypi_driver();
#endif
}

