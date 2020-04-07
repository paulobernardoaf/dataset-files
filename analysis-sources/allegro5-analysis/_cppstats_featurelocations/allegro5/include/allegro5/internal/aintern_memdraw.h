#if !defined(__al_included_allegro5_aintern_memdraw_h)
#define __al_included_allegro5_aintern_memdraw_h

#if defined(__cplusplus)
extern "C" {
#endif


void _al_clear_bitmap_by_locking(ALLEGRO_BITMAP *bitmap, ALLEGRO_COLOR *color);
void _al_draw_pixel_memory(ALLEGRO_BITMAP *bmp, float x, float y, ALLEGRO_COLOR *color);


#if defined(__cplusplus)
}
#endif

#endif


