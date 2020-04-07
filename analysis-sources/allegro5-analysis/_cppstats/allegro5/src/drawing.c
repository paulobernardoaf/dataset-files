#include "allegro5/allegro.h"
#include "allegro5/internal/aintern_bitmap.h"
#include "allegro5/internal/aintern_display.h"
#include "allegro5/internal/aintern_memdraw.h"
#include "allegro5/internal/aintern_pixels.h"
void al_clear_to_color(ALLEGRO_COLOR color)
{
ALLEGRO_BITMAP *target = al_get_target_bitmap();
ASSERT(target);
if (al_get_bitmap_flags(target) & ALLEGRO_MEMORY_BITMAP ||
_al_pixel_format_is_compressed(al_get_bitmap_format(target))) {
_al_clear_bitmap_by_locking(target, &color);
}
else {
ALLEGRO_DISPLAY *display = _al_get_bitmap_display(target);
ASSERT(display);
ASSERT(display->vt);
display->vt->clear(display, &color);
}
}
void al_clear_depth_buffer(float z)
{
ALLEGRO_BITMAP *target = al_get_target_bitmap();
ASSERT(target);
if (al_get_bitmap_flags(target) & ALLEGRO_MEMORY_BITMAP) {
}
else {
ALLEGRO_DISPLAY *display = _al_get_bitmap_display(target);
ASSERT(display);
display->vt->clear_depth_buffer(display, z);
}
}
void al_draw_pixel(float x, float y, ALLEGRO_COLOR color)
{
ALLEGRO_BITMAP *target = al_get_target_bitmap();
ASSERT(target);
if (al_get_bitmap_flags(target) & ALLEGRO_MEMORY_BITMAP ||
_al_pixel_format_is_compressed(al_get_bitmap_format(target))) {
_al_draw_pixel_memory(target, x, y, &color);
}
else {
ALLEGRO_DISPLAY *display = _al_get_bitmap_display(target);
ASSERT(display);
ASSERT(display->vt);
display->vt->draw_pixel(display, x, y, &color);
}
}
