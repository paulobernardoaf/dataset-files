
















#include "allegro5/allegro.h"
#include "allegro5/internal/aintern.h"
#include "allegro5/internal/aintern_bitmap.h"
#include "allegro5/internal/aintern_blend.h"
#include "allegro5/internal/aintern_display.h"
#include <string.h>

void _al_blend_memory(ALLEGRO_COLOR *scol,
ALLEGRO_BITMAP *dest,
int dx, int dy, ALLEGRO_COLOR *result)
{
ALLEGRO_COLOR dcol;
ALLEGRO_COLOR constcol;
int op, src_blend, dest_blend, alpha_op, alpha_src_blend, alpha_dest_blend;
dcol = al_get_pixel(dest, dx, dy);
al_get_separate_bitmap_blender(&op, &src_blend, &dest_blend,
&alpha_op, &alpha_src_blend,
&alpha_dest_blend);
constcol = al_get_blend_color();
_al_blend_inline(scol, &dcol,
op, src_blend, dest_blend,
alpha_op, alpha_src_blend, alpha_dest_blend,
&constcol, result);
(void) _al_blend_alpha_inline; 
}
