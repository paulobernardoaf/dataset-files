#if defined(__cplusplus)
extern "C" {
#endif
#include <stddef.h>
#include "lv_area.h"
#define LV_CIRC_OCT1_X(p) (p.x)
#define LV_CIRC_OCT1_Y(p) (p.y)
#define LV_CIRC_OCT2_X(p) (p.y)
#define LV_CIRC_OCT2_Y(p) (p.x)
#define LV_CIRC_OCT3_X(p) (-p.y)
#define LV_CIRC_OCT3_Y(p) (p.x)
#define LV_CIRC_OCT4_X(p) (-p.x)
#define LV_CIRC_OCT4_Y(p) (p.y)
#define LV_CIRC_OCT5_X(p) (-p.x)
#define LV_CIRC_OCT5_Y(p) (-p.y)
#define LV_CIRC_OCT6_X(p) (-p.y)
#define LV_CIRC_OCT6_Y(p) (-p.x)
#define LV_CIRC_OCT7_X(p) (p.y)
#define LV_CIRC_OCT7_Y(p) (-p.x)
#define LV_CIRC_OCT8_X(p) (p.x)
#define LV_CIRC_OCT8_Y(p) (-p.y)
void lv_circ_init(lv_point_t * c, lv_coord_t * tmp, lv_coord_t radius);
bool lv_circ_cont(lv_point_t * c);
void lv_circ_next(lv_point_t * c, lv_coord_t * tmp);
#if defined(__cplusplus)
} 
#endif
