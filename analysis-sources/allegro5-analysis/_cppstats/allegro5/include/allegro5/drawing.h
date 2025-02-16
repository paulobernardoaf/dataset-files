#include "allegro5/color.h"
#if defined(__cplusplus)
extern "C" {
#endif
AL_FUNC(void, al_clear_to_color, (ALLEGRO_COLOR color));
AL_FUNC(void, al_clear_depth_buffer, (float x));
AL_FUNC(void, al_draw_pixel, (float x, float y, ALLEGRO_COLOR color));
#if defined(__cplusplus)
}
#endif
