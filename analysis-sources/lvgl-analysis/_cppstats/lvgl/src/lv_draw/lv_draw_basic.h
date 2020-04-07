#if defined(__cplusplus)
extern "C" {
#endif
#if defined(LV_CONF_INCLUDE_SIMPLE)
#include "lv_conf.h"
#else
#include "../../../lv_conf.h"
#endif
#include "../lv_font/lv_font.h"
#include "../lv_misc/lv_color.h"
#include "../lv_misc/lv_area.h"
void lv_draw_px(lv_coord_t x, lv_coord_t y, const lv_area_t * mask_p, lv_color_t color, lv_opa_t opa);
void lv_draw_fill(const lv_area_t * cords_p, const lv_area_t * mask_p, lv_color_t color, lv_opa_t opa);
void lv_draw_letter(const lv_point_t * pos_p, const lv_area_t * mask_p, const lv_font_t * font_p, uint32_t letter,
lv_color_t color, lv_opa_t opa);
void lv_draw_map(const lv_area_t * cords_p, const lv_area_t * mask_p, const uint8_t * map_p, lv_opa_t opa,
bool chroma_key, bool alpha_byte, lv_color_t recolor, lv_opa_t recolor_opa);
#if defined(__cplusplus)
} 
#endif
