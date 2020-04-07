




#if !defined(LV_DRAW_TRIANGLE_H)
#define LV_DRAW_TRIANGLE_H

#if defined(__cplusplus)
extern "C" {
#endif




#include "lv_draw.h"




















void lv_draw_triangle(const lv_point_t * points, const lv_area_t * mask, const lv_style_t * style, lv_opa_t opa_scale);









void lv_draw_polygon(const lv_point_t * points, uint32_t point_cnt, const lv_area_t * mask, const lv_style_t * style,
lv_opa_t opa_scale);





#if defined(__cplusplus)
} 
#endif

#endif 
