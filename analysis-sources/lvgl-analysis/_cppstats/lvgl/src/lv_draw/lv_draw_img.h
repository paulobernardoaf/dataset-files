#if defined(__cplusplus)
extern "C" {
#endif
#include "lv_draw.h"
#include "lv_img_decoder.h"
#define LV_IMG_BUF_SIZE_TRUE_COLOR(w, h) ((LV_COLOR_SIZE / 8) * w * h)
#define LV_IMG_BUF_SIZE_TRUE_COLOR_CHROMA_KEYED(w, h) ((LV_COLOR_SIZE / 8) * w * h)
#define LV_IMG_BUF_SIZE_TRUE_COLOR_ALPHA(w, h) (LV_IMG_PX_SIZE_ALPHA_BYTE * w * h)
#define LV_IMG_BUF_SIZE_ALPHA_1BIT(w, h) ((((w / 8) + 1) * h))
#define LV_IMG_BUF_SIZE_ALPHA_2BIT(w, h) ((((w / 4) + 1) * h))
#define LV_IMG_BUF_SIZE_ALPHA_4BIT(w, h) ((((w / 2) + 1) * h))
#define LV_IMG_BUF_SIZE_ALPHA_8BIT(w, h) ((w * h))
#define LV_IMG_BUF_SIZE_INDEXED_1BIT(w, h) (LV_IMG_BUF_SIZE_ALPHA_1BIT(w, h) + 4 * 2)
#define LV_IMG_BUF_SIZE_INDEXED_2BIT(w, h) (LV_IMG_BUF_SIZE_ALPHA_2BIT(w, h) + 4 * 4)
#define LV_IMG_BUF_SIZE_INDEXED_4BIT(w, h) (LV_IMG_BUF_SIZE_ALPHA_4BIT(w, h) + 4 * 16)
#define LV_IMG_BUF_SIZE_INDEXED_8BIT(w, h) (LV_IMG_BUF_SIZE_ALPHA_8BIT(w, h) + 4 * 256)
void lv_draw_img(const lv_area_t * coords, const lv_area_t * mask, const void * src, const lv_style_t * style,
lv_opa_t opa_scale);
lv_img_src_t lv_img_src_get_type(const void * src);
lv_color_t lv_img_buf_get_px_color(lv_img_dsc_t * dsc, lv_coord_t x, lv_coord_t y, const lv_style_t * style);
lv_opa_t lv_img_buf_get_px_alpha(lv_img_dsc_t * dsc, lv_coord_t x, lv_coord_t y);
void lv_img_buf_set_px_color(lv_img_dsc_t * dsc, lv_coord_t x, lv_coord_t y, lv_color_t c);
void lv_img_buf_set_px_alpha(lv_img_dsc_t * dsc, lv_coord_t x, lv_coord_t y, lv_opa_t opa);
void lv_img_buf_set_palette(lv_img_dsc_t * dsc, uint8_t id, lv_color_t c);
uint8_t lv_img_color_format_get_px_size(lv_img_cf_t cf);
bool lv_img_color_format_is_chroma_keyed(lv_img_cf_t cf);
bool lv_img_color_format_has_alpha(lv_img_cf_t cf);
lv_img_dsc_t *lv_img_buf_alloc(lv_coord_t w, lv_coord_t h, lv_img_cf_t cf);
void lv_img_buf_free(lv_img_dsc_t *dsc);
uint32_t lv_img_buf_get_img_size(lv_coord_t w, lv_coord_t h, lv_img_cf_t cf);
#if defined(__cplusplus)
} 
#endif
