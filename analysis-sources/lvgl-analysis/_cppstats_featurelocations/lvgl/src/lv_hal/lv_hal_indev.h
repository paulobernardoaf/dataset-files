






#if !defined(LV_HAL_INDEV_H)
#define LV_HAL_INDEV_H

#if defined(__cplusplus)
extern "C" {
#endif




#if defined(LV_CONF_INCLUDE_SIMPLE)
#include "lv_conf.h"
#else
#include "../../../lv_conf.h"
#endif

#include <stdbool.h>
#include <stdint.h>
#include "../lv_misc/lv_area.h"
#include "../lv_misc/lv_task.h"









struct _lv_obj_t;
struct _disp_t;
struct _lv_indev_t;
struct _lv_indev_drv_t;


enum {
LV_INDEV_TYPE_NONE, 
LV_INDEV_TYPE_POINTER, 
LV_INDEV_TYPE_KEYPAD, 
LV_INDEV_TYPE_BUTTON, 

LV_INDEV_TYPE_ENCODER, 
};
typedef uint8_t lv_indev_type_t;


enum { LV_INDEV_STATE_REL = 0, LV_INDEV_STATE_PR };
typedef uint8_t lv_indev_state_t;


typedef struct
{
lv_point_t point; 
uint32_t key; 
uint32_t btn_id; 
int16_t enc_diff; 

lv_indev_state_t state; 
} lv_indev_data_t;


typedef struct _lv_indev_drv_t
{


lv_indev_type_t type;




bool (*read_cb)(struct _lv_indev_drv_t * indev_drv, lv_indev_data_t * data);



void (*feedback_cb)(struct _lv_indev_drv_t *, uint8_t);

#if LV_USE_USER_DATA
lv_indev_drv_user_data_t user_data;
#endif


struct _disp_t * disp;


lv_task_t * read_task;


uint8_t drag_limit;


uint8_t drag_throw;


uint16_t long_press_time;


uint16_t long_press_rep_time;
} lv_indev_drv_t;




typedef struct _lv_indev_proc_t
{
lv_indev_state_t state; 
union
{
struct
{ 
lv_point_t act_point; 
lv_point_t last_point; 
lv_point_t vect; 
lv_point_t drag_sum; 
lv_point_t drag_throw_vect;
struct _lv_obj_t * act_obj; 
struct _lv_obj_t * last_obj; 

struct _lv_obj_t * last_pressed; 


uint8_t drag_limit_out : 1;
uint8_t drag_in_prog : 1;
} pointer;
struct
{ 
lv_indev_state_t last_state;
uint32_t last_key;
} keypad;
} types;

uint32_t pr_timestamp; 
uint32_t longpr_rep_timestamp; 


uint8_t long_pr_sent : 1;
uint8_t reset_query : 1;
uint8_t disabled : 1;
uint8_t wait_until_release : 1;
} lv_indev_proc_t;

struct _lv_obj_t;
struct _lv_group_t;



typedef struct _lv_indev_t
{
lv_indev_drv_t driver;
lv_indev_proc_t proc;
struct _lv_obj_t * cursor; 
struct _lv_group_t * group; 
const lv_point_t * btn_points; 

} lv_indev_t;











void lv_indev_drv_init(lv_indev_drv_t * driver);






lv_indev_t * lv_indev_drv_register(lv_indev_drv_t * driver);






void lv_indev_drv_update(lv_indev_t * indev, lv_indev_drv_t * new_drv);







lv_indev_t * lv_indev_get_next(lv_indev_t * indev);







bool lv_indev_read(lv_indev_t * indev, lv_indev_data_t * data);





#if defined(__cplusplus)
} 
#endif

#endif
