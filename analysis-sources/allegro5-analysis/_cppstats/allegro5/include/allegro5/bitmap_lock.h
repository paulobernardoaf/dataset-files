#include "allegro5/bitmap.h"
#if defined(__cplusplus)
extern "C" {
#endif
enum {
ALLEGRO_LOCK_READWRITE = 0,
ALLEGRO_LOCK_READONLY = 1,
ALLEGRO_LOCK_WRITEONLY = 2
};
typedef struct ALLEGRO_LOCKED_REGION ALLEGRO_LOCKED_REGION;
struct ALLEGRO_LOCKED_REGION {
void *data;
int format;
int pitch;
int pixel_size;
};
AL_FUNC(ALLEGRO_LOCKED_REGION*, al_lock_bitmap, (ALLEGRO_BITMAP *bitmap, int format, int flags));
AL_FUNC(ALLEGRO_LOCKED_REGION*, al_lock_bitmap_region, (ALLEGRO_BITMAP *bitmap, int x, int y, int width, int height, int format, int flags));
AL_FUNC(ALLEGRO_LOCKED_REGION*, al_lock_bitmap_blocked, (ALLEGRO_BITMAP *bitmap, int flags));
AL_FUNC(ALLEGRO_LOCKED_REGION*, al_lock_bitmap_region_blocked, (ALLEGRO_BITMAP *bitmap, int x_block, int y_block,
int width_block, int height_block, int flags));
AL_FUNC(void, al_unlock_bitmap, (ALLEGRO_BITMAP *bitmap));
AL_FUNC(bool, al_is_bitmap_locked, (ALLEGRO_BITMAP *bitmap));
#if defined(__cplusplus)
}
#endif
