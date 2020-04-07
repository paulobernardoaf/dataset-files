




#if !defined(LV_CONT_H)
#define LV_CONT_H

#if defined(__cplusplus)
extern "C" {
#endif




#if defined(LV_CONF_INCLUDE_SIMPLE)
#include "lv_conf.h"
#else
#include "../../../lv_conf.h"
#endif

#if LV_USE_CONT != 0

#include "../lv_core/lv_obj.h"










enum {
LV_LAYOUT_OFF = 0, 
LV_LAYOUT_CENTER, 
LV_LAYOUT_COL_L, 
LV_LAYOUT_COL_M, 
LV_LAYOUT_COL_R, 
LV_LAYOUT_ROW_T, 
LV_LAYOUT_ROW_M, 
LV_LAYOUT_ROW_B, 
LV_LAYOUT_PRETTY, 
LV_LAYOUT_GRID, 
_LV_LAYOUT_NUM
};
typedef uint8_t lv_layout_t;




enum {
LV_FIT_NONE, 
LV_FIT_TIGHT, 
LV_FIT_FLOOD, 
LV_FIT_FILL, 

_LV_FIT_NUM
};
typedef uint8_t lv_fit_t;

typedef struct
{


uint8_t layout : 4; 
uint8_t fit_left : 2; 
uint8_t fit_right : 2; 
uint8_t fit_top : 2; 
uint8_t fit_bottom : 2; 
} lv_cont_ext_t;


enum {
LV_CONT_STYLE_MAIN,
};
typedef uint8_t lv_cont_style_t;











lv_obj_t * lv_cont_create(lv_obj_t * par, const lv_obj_t * copy);










void lv_cont_set_layout(lv_obj_t * cont, lv_layout_t layout);










void lv_cont_set_fit4(lv_obj_t * cont, lv_fit_t left, lv_fit_t right, lv_fit_t top, lv_fit_t bottom);








static inline void lv_cont_set_fit2(lv_obj_t * cont, lv_fit_t hor, lv_fit_t ver)
{
lv_cont_set_fit4(cont, hor, hor, ver, ver);
}







static inline void lv_cont_set_fit(lv_obj_t * cont, lv_fit_t fit)
{
lv_cont_set_fit4(cont, fit, fit, fit, fit);
}







static inline void lv_cont_set_style(lv_obj_t * cont, lv_cont_style_t type, const lv_style_t * style)
{
(void)type; 
lv_obj_set_style(cont, style);
}










lv_layout_t lv_cont_get_layout(const lv_obj_t * cont);






lv_fit_t lv_cont_get_fit_left(const lv_obj_t * cont);






lv_fit_t lv_cont_get_fit_right(const lv_obj_t * cont);






lv_fit_t lv_cont_get_fit_top(const lv_obj_t * cont);






lv_fit_t lv_cont_get_fit_bottom(const lv_obj_t * cont);







static inline const lv_style_t * lv_cont_get_style(const lv_obj_t * cont, lv_cont_style_t type)
{
(void)type; 
return lv_obj_get_style(cont);
}





#endif 

#if defined(__cplusplus)
} 
#endif

#endif 
