













#include "SDL.h"

#include "allegro5/altime.h"
#include "allegro5/platform/allegro_internal_sdl.h"
#include "allegro5/debug.h"

ALLEGRO_DEBUG_CHANNEL("SDL")

double _al_sdl_get_time(void)
{
return 1.0 * SDL_GetPerformanceCounter() / SDL_GetPerformanceFrequency();
}



void _al_sdl_rest(double seconds)
{
SDL_Delay(seconds * 1000);
}



void _al_sdl_init_timeout(ALLEGRO_TIMEOUT *timeout, double seconds)
{
ALLEGRO_TIMEOUT_SDL *timeout_sdl = (void *)timeout;
timeout_sdl->ms = seconds * 1000;
}


