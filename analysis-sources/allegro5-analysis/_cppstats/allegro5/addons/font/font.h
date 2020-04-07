#include "allegro5/internal/aintern_font.h"
extern ALLEGRO_FONT_VTABLE _al_font_vtable_color;
typedef struct ALLEGRO_FONT_COLOR_DATA
{
int begin, end; 
ALLEGRO_BITMAP *glyphs; 
ALLEGRO_BITMAP **bitmaps; 
struct ALLEGRO_FONT_COLOR_DATA *next; 
} ALLEGRO_FONT_COLOR_DATA;
ALLEGRO_FONT *_al_load_bitmap_font(const char *filename,
int size, int flags);
ALLEGRO_FONT *_al_load_bmfont_xml(const char *filename,
int size, int flags);
