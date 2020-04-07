#include "freetype.h"
typedef struct ruby_block_t ruby_block_t;
typedef struct line_desc_t line_desc_t;
typedef struct
{
FT_BitmapGlyph p_glyph;
FT_BitmapGlyph p_outline;
FT_BitmapGlyph p_shadow;
FT_BBox bbox;
const text_style_t *p_style;
const ruby_block_t *p_ruby;
int i_line_offset; 
int i_line_thickness; 
} line_character_t;
struct line_desc_t
{
line_desc_t *p_next;
int i_width;
int i_height;
int i_base_line;
int i_character_count;
int i_first_visible_char_index;
int i_last_visible_char_index;
line_character_t *p_character;
FT_BBox bbox;
};
void FreeLines( line_desc_t *p_lines );
line_desc_t *NewLine( int i_count );
struct ruby_block_t
{
uni_char_t *p_uchars; 
size_t i_count; 
text_style_t *p_style; 
line_desc_t *p_laid;
};
typedef struct
{
uni_char_t *p_uchars; 
text_style_t **pp_styles; 
ruby_block_t **pp_ruby; 
size_t i_count; 
bool b_balanced; 
bool b_grid; 
unsigned i_max_width; 
unsigned i_max_height; 
line_desc_t *p_laid;
} layout_text_block_t;
int LayoutTextBlock( filter_t *p_filter, const layout_text_block_t *p_textblock,
line_desc_t **pp_lines, FT_BBox *p_bbox, int *pi_max_face_height );
