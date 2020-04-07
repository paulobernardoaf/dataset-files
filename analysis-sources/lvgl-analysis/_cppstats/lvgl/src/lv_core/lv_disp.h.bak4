




#if !defined(LV_DISP_H)
#define LV_DISP_H

#if defined(__cplusplus)
extern "C" {
#endif




#include "../lv_hal/lv_hal.h"
#include "lv_obj.h"



















lv_obj_t * lv_disp_get_scr_act(lv_disp_t * disp);





void lv_disp_load_scr(lv_obj_t * scr);






lv_obj_t * lv_disp_get_layer_top(lv_disp_t * disp);







lv_obj_t * lv_disp_get_layer_sys(lv_disp_t * disp);






void lv_disp_assign_screen(lv_disp_t * disp, lv_obj_t * scr);







lv_task_t * lv_disp_get_refr_task(lv_disp_t * disp);






uint32_t lv_disp_get_inactive_time(const lv_disp_t * disp);





void lv_disp_trig_activity(lv_disp_t * disp);










static inline lv_obj_t * lv_scr_act(void)
{
return lv_disp_get_scr_act(lv_disp_get_default());
}





static inline lv_obj_t * lv_layer_top(void)
{
return lv_disp_get_layer_top(lv_disp_get_default());
}





static inline lv_obj_t * lv_layer_sys(void)
{
return lv_disp_get_layer_sys(lv_disp_get_default());
}

static inline void lv_scr_load(lv_obj_t * scr)
{
lv_disp_load_scr(scr);
}










#if !defined(LV_HOR_RES)



#define LV_HOR_RES lv_disp_get_hor_res(lv_disp_get_default())
#endif

#if !defined(LV_VER_RES)



#define LV_VER_RES lv_disp_get_ver_res(lv_disp_get_default())
#endif

#if defined(__cplusplus)
} 
#endif

#endif 
