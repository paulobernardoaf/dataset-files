














#include <string.h> 
#include "allegro5/allegro.h"
#include "allegro5/internal/aintern_bitmap.h"
#include "allegro5/internal/aintern_blend.h"
#include "allegro5/internal/aintern_pixels.h"

ALLEGRO_DEBUG_CHANNEL("bitmap")




ALLEGRO_COLOR al_get_pixel(ALLEGRO_BITMAP *bitmap, int x, int y)
{
ALLEGRO_LOCKED_REGION *lr;
char *data;
ALLEGRO_COLOR color = al_map_rgba_f(0, 0, 0, 0);

if (bitmap->parent) {
x += bitmap->xofs;
y += bitmap->yofs;
bitmap = bitmap->parent;
}

if (bitmap->locked) {
if (_al_pixel_format_is_video_only(bitmap->locked_region.format)) {
ALLEGRO_ERROR("Invalid lock format.");
return color;
}
x -= bitmap->lock_x;
y -= bitmap->lock_y;
if (x < 0 || y < 0 || x >= bitmap->lock_w || y >= bitmap->lock_h) {
ALLEGRO_ERROR("Out of bounds.");
return color;
}

data = bitmap->locked_region.data;
data += y * bitmap->locked_region.pitch;
data += x * al_get_pixel_size(bitmap->locked_region.format);

_AL_INLINE_GET_PIXEL(bitmap->locked_region.format, data, color, false);
}
else {

if (x < 0 || y < 0 || x >= bitmap->w || y >= bitmap->h) {
return color;
}

if (!(lr = al_lock_bitmap_region(bitmap, x, y, 1, 1,
ALLEGRO_PIXEL_FORMAT_ANY, ALLEGRO_LOCK_READONLY))) {
return color;
}



data = bitmap->lock_data;
_AL_INLINE_GET_PIXEL(lr->format, data, color, false);

al_unlock_bitmap(bitmap);
}

return color;
}


void _al_put_pixel(ALLEGRO_BITMAP *bitmap, int x, int y, ALLEGRO_COLOR color)
{
ALLEGRO_LOCKED_REGION *lr;
char *data;

if (bitmap->parent) {
x += bitmap->xofs;
y += bitmap->yofs;
bitmap = bitmap->parent;
}

if (x < bitmap->cl || y < bitmap->ct ||
x >= bitmap->cr_excl || y >= bitmap->cb_excl) {
return;
}

if (bitmap->locked) {
if (_al_pixel_format_is_video_only(bitmap->locked_region.format)) {
ALLEGRO_ERROR("Invalid lock format.");
return;
}
x -= bitmap->lock_x;
y -= bitmap->lock_y;
if (x < 0 || y < 0 || x >= bitmap->lock_w || y >= bitmap->lock_h) {
return;
}

data = bitmap->locked_region.data;
data += y * bitmap->locked_region.pitch;
data += x * al_get_pixel_size(bitmap->locked_region.format);

_AL_INLINE_PUT_PIXEL(bitmap->locked_region.format, data, color, false);
}
else {
lr = al_lock_bitmap_region(bitmap, x, y, 1, 1,
ALLEGRO_PIXEL_FORMAT_ANY, ALLEGRO_LOCK_WRITEONLY);
if (!lr)
return;



data = bitmap->lock_data;
_AL_INLINE_PUT_PIXEL(lr->format, data, color, false);

al_unlock_bitmap(bitmap);
}
}




void al_put_pixel(int x, int y, ALLEGRO_COLOR color)
{
_al_put_pixel(al_get_target_bitmap(), x, y, color);
}




void al_put_blended_pixel(int x, int y, ALLEGRO_COLOR color)
{
ALLEGRO_COLOR result;
ALLEGRO_BITMAP* bitmap = al_get_target_bitmap();
_al_blend_memory(&color, bitmap, x, y, &result);
_al_put_pixel(bitmap, x, y, result);
}



