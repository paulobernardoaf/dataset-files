








#include "lv_font.h"
#include "../lv_misc/lv_utils.h"
#include "../lv_misc/lv_log.h"



































const uint8_t * lv_font_get_glyph_bitmap(const lv_font_t * font_p, uint32_t letter)
{
return font_p->get_glyph_bitmap(font_p, letter);
}









bool lv_font_get_glyph_dsc(const lv_font_t * font_p, lv_font_glyph_dsc_t * dsc_out, uint32_t letter, uint32_t letter_next)
{
return font_p->get_glyph_dsc(font_p, dsc_out, letter, letter_next);
}








uint16_t lv_font_get_glyph_width(const lv_font_t * font, uint32_t letter, uint32_t letter_next)
{
lv_font_glyph_dsc_t g;
bool ret;
ret = lv_font_get_glyph_dsc(font, &g, letter, letter_next);
if(ret) return g.adv_w;
else return 0;
}




