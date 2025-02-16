#if defined(__cplusplus)
extern "C" {
#endif
#include "lv_obj.h"
#include <stdbool.h>
void lv_refr_init(void);
void lv_refr_now(lv_disp_t * disp);
void lv_inv_area(lv_disp_t * disp, const lv_area_t * area_p);
lv_disp_t * lv_refr_get_disp_refreshing(void);
void lv_refr_set_disp_refreshing(lv_disp_t * disp);
void lv_disp_refr_task(lv_task_t * task);
#if defined(__cplusplus)
} 
#endif
