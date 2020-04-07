




#if !defined(LV_LINE_H)
#define LV_LINE_H

#if defined(__cplusplus)
extern "C" {
#endif




#if defined(LV_CONF_INCLUDE_SIMPLE)
#include "lv_conf.h"
#else
#include "../../../lv_conf.h"
#endif

#if LV_USE_LINE != 0

#include "../lv_core/lv_obj.h"










typedef struct
{

const lv_point_t * point_array; 
uint16_t point_num; 
uint8_t auto_size : 1; 
uint8_t y_inv : 1; 
} lv_line_ext_t;


enum {
LV_LINE_STYLE_MAIN,
};
typedef uint8_t lv_line_style_t;










lv_obj_t * lv_line_create(lv_obj_t * par, const lv_obj_t * copy);












void lv_line_set_points(lv_obj_t * line, const lv_point_t point_a[], uint16_t point_num);







void lv_line_set_auto_size(lv_obj_t * line, bool en);








void lv_line_set_y_invert(lv_obj_t * line, bool en);

#define lv_line_set_y_inv lv_line_set_y_invert 









static inline void lv_line_set_style(lv_obj_t * line, lv_line_style_t type, const lv_style_t * style)
{
(void)type; 
lv_obj_set_style(line, style);
}










bool lv_line_get_auto_size(const lv_obj_t * line);






bool lv_line_get_y_invert(const lv_obj_t * line);







static inline const lv_style_t * lv_line_get_style(const lv_obj_t * line, lv_line_style_t type)
{
(void)type; 
return lv_obj_get_style(line);
}





#endif 

#if defined(__cplusplus)
} 
#endif

#endif 
