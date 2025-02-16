#if defined(__cplusplus)
extern "C" {
#endif
#if defined(LV_CONF_INCLUDE_SIMPLE)
#include "lv_conf.h"
#else
#include "../../../lv_conf.h"
#endif
#if LV_USE_ARC != 0
#include "../lv_core/lv_obj.h"
typedef struct
{
lv_coord_t angle_start;
lv_coord_t angle_end;
} lv_arc_ext_t;
enum {
LV_ARC_STYLE_MAIN,
};
typedef uint8_t lv_arc_style_t;
lv_obj_t * lv_arc_create(lv_obj_t * par, const lv_obj_t * copy);
void lv_arc_set_angles(lv_obj_t * arc, uint16_t start, uint16_t end);
void lv_arc_set_style(lv_obj_t * arc, lv_arc_style_t type, const lv_style_t * style);
uint16_t lv_arc_get_angle_start(lv_obj_t * arc);
uint16_t lv_arc_get_angle_end(lv_obj_t * arc);
const lv_style_t * lv_arc_get_style(const lv_obj_t * arc, lv_arc_style_t type);
#endif 
#if defined(__cplusplus)
} 
#endif
