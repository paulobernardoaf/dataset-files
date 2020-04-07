




#if !defined(LV_FONT_H)
#define LV_FONT_H

#if defined(__cplusplus)
extern "C" {
#endif




#if defined(LV_CONF_INCLUDE_SIMPLE)
#include "lv_conf.h"
#else
#include "../../../lv_conf.h"
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "lv_symbol_def.h"





#define LV_FONT_WIDTH_FRACT_DIGIT 4

#define LV_FONT_KERN_POSITIVE 0
#define LV_FONT_KERN_NEGATIVE 1










typedef struct
{
uint16_t adv_w; 
uint8_t box_w; 
uint8_t box_h; 
int8_t ofs_x; 
int8_t ofs_y; 
uint8_t bpp; 
}lv_font_glyph_dsc_t;



enum {
LV_FONT_SUBPX_NONE,
LV_FONT_SUBPX_HOR,
LV_FONT_SUBPX_VER,
LV_FONT_SUBPX_BOTH,
};

typedef uint8_t lv_font_subpx_t;


typedef struct _lv_font_struct
{

bool (*get_glyph_dsc)(const struct _lv_font_struct *, lv_font_glyph_dsc_t *, uint32_t letter, uint32_t letter_next);


const uint8_t * (*get_glyph_bitmap)(const struct _lv_font_struct *, uint32_t);


uint8_t line_height; 
int8_t base_line; 
uint8_t subpx :2; 
void * dsc; 
#if LV_USE_USER_DATA
lv_font_user_data_t user_data; 
#endif


} lv_font_t;











const uint8_t * lv_font_get_glyph_bitmap(const lv_font_t * font_p, uint32_t letter);









bool lv_font_get_glyph_dsc(const lv_font_t * font_p, lv_font_glyph_dsc_t * dsc_out, uint32_t letter, uint32_t letter_next);








uint16_t lv_font_get_glyph_width(const lv_font_t * font, uint32_t letter, uint32_t letter_next);






static inline uint8_t lv_font_get_line_height(const lv_font_t * font_p)
{
return font_p->line_height;
}





#define LV_FONT_DECLARE(font_name) extern lv_font_t font_name;

#if LV_FONT_ROBOTO_12
LV_FONT_DECLARE(lv_font_roboto_12)
#endif

#if LV_FONT_ROBOTO_16
LV_FONT_DECLARE(lv_font_roboto_16)
#endif

#if LV_FONT_ROBOTO_22
LV_FONT_DECLARE(lv_font_roboto_22)
#endif

#if LV_FONT_ROBOTO_28
LV_FONT_DECLARE(lv_font_roboto_28)
#endif

#if LV_FONT_UNSCII_8
LV_FONT_DECLARE(lv_font_unscii_8)
#endif


#if defined(LV_FONT_CUSTOM_DECLARE)
LV_FONT_CUSTOM_DECLARE
#endif

#if defined(__cplusplus)
} 
#endif

#endif 
