






#if !defined(LV_HAL_DISP_H)
#define LV_HAL_DISP_H

#if defined(__cplusplus)
extern "C" {
#endif




#include <stdint.h>
#include <stdbool.h>
#include "lv_hal.h"
#include "../lv_misc/lv_color.h"
#include "../lv_misc/lv_area.h"
#include "../lv_misc/lv_ll.h"
#include "../lv_misc/lv_task.h"




#if !defined(LV_INV_BUF_SIZE)
#define LV_INV_BUF_SIZE 32 
#endif

#if !defined(LV_ATTRIBUTE_FLUSH_READY)
#define LV_ATTRIBUTE_FLUSH_READY
#endif





struct _disp_t;
struct _disp_drv_t;




typedef struct
{
void * buf1; 
void * buf2; 


void * buf_act;
uint32_t size; 
lv_area_t area;
volatile uint32_t flushing : 1;
} lv_disp_buf_t;




typedef struct _disp_drv_t
{

lv_coord_t hor_res; 
lv_coord_t ver_res; 



lv_disp_buf_t * buffer;

#if LV_ANTIALIAS
uint32_t antialiasing : 1; 
#endif
uint32_t rotated : 1; 

#if LV_COLOR_SCREEN_TRANSP


uint32_t screen_transp : 1;
#endif



void (*flush_cb)(struct _disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p);



void (*rounder_cb)(struct _disp_drv_t * disp_drv, lv_area_t * area);




void (*set_px_cb)(struct _disp_drv_t * disp_drv, uint8_t * buf, lv_coord_t buf_w, lv_coord_t x, lv_coord_t y,
lv_color_t color, lv_opa_t opa);



void (*monitor_cb)(struct _disp_drv_t * disp_drv, uint32_t time, uint32_t px);

#if LV_USE_GPU

void (*gpu_blend_cb)(struct _disp_drv_t * disp_drv, lv_color_t * dest, const lv_color_t * src, uint32_t length,
lv_opa_t opa);


void (*gpu_fill_cb)(struct _disp_drv_t * disp_drv, lv_color_t * dest_buf, lv_coord_t dest_width,
const lv_area_t * fill_area, lv_color_t color);
#endif



lv_color_t color_chroma_key;

#if LV_USE_USER_DATA
lv_disp_drv_user_data_t user_data; 
#endif

} lv_disp_drv_t;

struct _lv_obj_t;





typedef struct _disp_t
{

lv_disp_drv_t driver;


lv_task_t * refr_task;


lv_ll_t scr_ll;
struct _lv_obj_t * act_scr; 
struct _lv_obj_t * top_layer; 
struct _lv_obj_t * sys_layer; 


lv_area_t inv_areas[LV_INV_BUF_SIZE];
uint8_t inv_area_joined[LV_INV_BUF_SIZE];
uint32_t inv_p : 10;


uint32_t last_activity_time; 
} lv_disp_t;











void lv_disp_drv_init(lv_disp_drv_t * driver);
















void lv_disp_buf_init(lv_disp_buf_t * disp_buf, void * buf1, void * buf2, uint32_t size_in_px_cnt);







lv_disp_t * lv_disp_drv_register(lv_disp_drv_t * driver);






void lv_disp_drv_update(lv_disp_t * disp, lv_disp_drv_t * new_drv);





void lv_disp_remove(lv_disp_t * disp);





void lv_disp_set_default(lv_disp_t * disp);





lv_disp_t * lv_disp_get_default(void);






lv_coord_t lv_disp_get_hor_res(lv_disp_t * disp);






lv_coord_t lv_disp_get_ver_res(lv_disp_t * disp);






bool lv_disp_get_antialiasing(lv_disp_t * disp);







LV_ATTRIBUTE_FLUSH_READY void lv_disp_flush_ready(lv_disp_drv_t * disp_drv);








lv_disp_t * lv_disp_get_next(lv_disp_t * disp);






lv_disp_buf_t * lv_disp_get_buf(lv_disp_t * disp);





uint16_t lv_disp_get_inv_buf_size(lv_disp_t * disp);





void lv_disp_pop_from_inv_buf(lv_disp_t * disp, uint16_t num);






bool lv_disp_is_double_buf(lv_disp_t * disp);







bool lv_disp_is_true_double_buf(lv_disp_t * disp);





#if defined(__cplusplus)
} 
#endif

#endif
