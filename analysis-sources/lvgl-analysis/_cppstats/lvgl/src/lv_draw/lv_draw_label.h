#if defined(__cplusplus)
extern "C" {
#endif
#include "lv_draw.h"
#include "../lv_misc/lv_bidi.h"
#define LV_DRAW_LABEL_NO_TXT_SEL (0xFFFF)
typedef struct
{
uint16_t start;
uint16_t end;
}lv_draw_label_txt_sel_t;
typedef struct {
int32_t line_start;
int32_t y;
int32_t coord_y;
}lv_draw_label_hint_t;
void lv_draw_label(const lv_area_t * coords, const lv_area_t * mask, const lv_style_t * style, lv_opa_t opa_scale,
const char * txt, lv_txt_flag_t flag, lv_point_t * offset, lv_draw_label_txt_sel_t * sel,
lv_draw_label_hint_t * hint, lv_bidi_dir_t bidi_dir);
#if defined(__cplusplus)
} 
#endif
