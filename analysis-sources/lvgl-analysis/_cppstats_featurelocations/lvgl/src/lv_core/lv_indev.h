




#if !defined(LV_INDEV_H)
#define LV_INDEV_H

#if defined(__cplusplus)
extern "C" {
#endif




#include "lv_obj.h"
#include "../lv_hal/lv_hal_indev.h"
#include "../lv_core/lv_group.h"
















void lv_indev_init(void);





void lv_indev_read_task(lv_task_t * task);






lv_indev_t * lv_indev_get_act(void);






lv_indev_type_t lv_indev_get_type(const lv_indev_t * indev);





void lv_indev_reset(lv_indev_t * indev);





void lv_indev_reset_long_press(lv_indev_t * indev);






void lv_indev_enable(lv_indev_t * indev, bool en);






void lv_indev_set_cursor(lv_indev_t * indev, lv_obj_t * cur_obj);

#if LV_USE_GROUP





void lv_indev_set_group(lv_indev_t * indev, lv_group_t * group);
#endif







void lv_indev_set_button_points(lv_indev_t * indev, const lv_point_t * points);






void lv_indev_get_point(const lv_indev_t * indev, lv_point_t * point);






uint32_t lv_indev_get_key(const lv_indev_t * indev);







bool lv_indev_is_dragging(const lv_indev_t * indev);







void lv_indev_get_vect(const lv_indev_t * indev, lv_point_t * point);





void lv_indev_wait_release(lv_indev_t * indev);







lv_task_t * lv_indev_get_read_task(lv_disp_t * indev);






lv_obj_t * lv_indev_get_obj_act(void);





#if defined(__cplusplus)
} 
#endif

#endif 
