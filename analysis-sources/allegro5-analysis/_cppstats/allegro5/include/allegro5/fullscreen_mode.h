#include "allegro5/base.h"
#if defined(__cplusplus)
extern "C" {
#endif
typedef struct ALLEGRO_DISPLAY_MODE
{
int width;
int height;
int format;
int refresh_rate;
} ALLEGRO_DISPLAY_MODE;
AL_FUNC(int, al_get_num_display_modes, (void));
AL_FUNC(ALLEGRO_DISPLAY_MODE*, al_get_display_mode, (int index,
ALLEGRO_DISPLAY_MODE *mode));
#if defined(__cplusplus)
}
#endif
