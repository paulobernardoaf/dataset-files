




#if !defined(LV_CANVAS_H)
#define LV_CANVAS_H

#if defined(__cplusplus)
extern "C" {
#endif




#if defined(LV_CONF_INCLUDE_SIMPLE)
#include "lv_conf.h"
#else
#include "../../../lv_conf.h"
#endif

#if LV_USE_CANVAS != 0

#include "../lv_core/lv_obj.h"
#include "../lv_objx/lv_img.h"
#include "../lv_draw/lv_draw_img.h"









typedef struct
{
lv_img_ext_t img; 

lv_img_dsc_t dsc;
} lv_canvas_ext_t;


enum {
LV_CANVAS_STYLE_MAIN,
};
typedef uint8_t lv_canvas_style_t;











lv_obj_t * lv_canvas_create(lv_obj_t * par, const lv_obj_t * copy);

















void lv_canvas_set_buffer(lv_obj_t * canvas, void * buf, lv_coord_t w, lv_coord_t h, lv_img_cf_t cf);








void lv_canvas_set_px(lv_obj_t * canvas, lv_coord_t x, lv_coord_t y, lv_color_t c);











void lv_canvas_set_palette(lv_obj_t * canvas, uint8_t id, lv_color_t c);







void lv_canvas_set_style(lv_obj_t * canvas, lv_canvas_style_t type, const lv_style_t * style);












lv_color_t lv_canvas_get_px(lv_obj_t * canvas, lv_coord_t x, lv_coord_t y);






lv_img_dsc_t * lv_canvas_get_img(lv_obj_t * canvas);







const lv_style_t * lv_canvas_get_style(const lv_obj_t * canvas, lv_canvas_style_t type);















void lv_canvas_copy_buf(lv_obj_t * canvas, const void * to_copy, lv_coord_t x, lv_coord_t y, lv_coord_t w,
lv_coord_t h);














void lv_canvas_rotate(lv_obj_t * canvas, lv_img_dsc_t * img, int16_t angle, lv_coord_t offset_x, lv_coord_t offset_y,
int32_t pivot_x, int32_t pivot_y);






void lv_canvas_fill_bg(lv_obj_t * canvas, lv_color_t color);










void lv_canvas_draw_rect(lv_obj_t * canvas, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h,
const lv_style_t * style);











void lv_canvas_draw_text(lv_obj_t * canvas, lv_coord_t x, lv_coord_t y, lv_coord_t max_w, const lv_style_t * style,
const char * txt, lv_label_align_t align);







void lv_canvas_draw_img(lv_obj_t * canvas, lv_coord_t x, lv_coord_t y, const void * src, const lv_style_t * style);








void lv_canvas_draw_line(lv_obj_t * canvas, const lv_point_t * points, uint32_t point_cnt, const lv_style_t * style);








void lv_canvas_draw_polygon(lv_obj_t * canvas, const lv_point_t * points, uint32_t point_cnt, const lv_style_t * style);











void lv_canvas_draw_arc(lv_obj_t * canvas, lv_coord_t x, lv_coord_t y, lv_coord_t r, int32_t start_angle,
int32_t end_angle, const lv_style_t * style);




#define LV_CANVAS_BUF_SIZE_TRUE_COLOR(w, h) LV_IMG_BUF_SIZE_TRUE_COLOR(w, h)
#define LV_CANVAS_BUF_SIZE_TRUE_COLOR_CHROMA_KEYED(w, h) LV_IMG_BUF_SIZE_TRUE_COLOR_CHROMA_KEYED(w, h)
#define LV_CANVAS_BUF_SIZE_TRUE_COLOR_ALPHA(w, h) LV_IMG_BUF_SIZE_TRUE_COLOR_ALPHA(w, h)


#define LV_CANVAS_BUF_SIZE_ALPHA_1BIT(w, h) LV_IMG_BUF_SIZE_ALPHA_1BIT(w, h)
#define LV_CANVAS_BUF_SIZE_ALPHA_2BIT(w, h) LV_IMG_BUF_SIZE_ALPHA_2BIT(w, h)
#define LV_CANVAS_BUF_SIZE_ALPHA_4BIT(w, h) LV_IMG_BUF_SIZE_ALPHA_4BIT(w, h)
#define LV_CANVAS_BUF_SIZE_ALPHA_8BIT(w, h) LV_IMG_BUF_SIZE_ALPHA_8BIT(w, h)


#define LV_CANVAS_BUF_SIZE_INDEXED_1BIT(w, h) LV_IMG_BUF_SIZE_INDEXED_1BIT(w, h)
#define LV_CANVAS_BUF_SIZE_INDEXED_2BIT(w, h) LV_IMG_BUF_SIZE_INDEXED_2BIT(w, h)
#define LV_CANVAS_BUF_SIZE_INDEXED_4BIT(w, h) LV_IMG_BUF_SIZE_INDEXED_4BIT(w, h)
#define LV_CANVAS_BUF_SIZE_INDEXED_8BIT(w, h) LV_IMG_BUF_SIZE_INDEXED_8BIT(w, h)

#endif 

#if defined(__cplusplus)
} 
#endif

#endif 
