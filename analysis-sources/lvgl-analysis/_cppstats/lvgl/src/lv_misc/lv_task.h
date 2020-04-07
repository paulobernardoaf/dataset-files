#if defined(__cplusplus)
extern "C" {
#endif
#if defined(LV_CONF_INCLUDE_SIMPLE)
#include "lv_conf.h"
#else
#include "../../../lv_conf.h"
#endif
#include <stdint.h>
#include <stdbool.h>
#include "lv_mem.h"
#include "lv_ll.h"
#if !defined(LV_ATTRIBUTE_TASK_HANDLER)
#define LV_ATTRIBUTE_TASK_HANDLER
#endif
struct _lv_task_t;
typedef void (*lv_task_cb_t)(struct _lv_task_t *);
enum {
LV_TASK_PRIO_OFF = 0,
LV_TASK_PRIO_LOWEST,
LV_TASK_PRIO_LOW,
LV_TASK_PRIO_MID,
LV_TASK_PRIO_HIGH,
LV_TASK_PRIO_HIGHEST,
_LV_TASK_PRIO_NUM,
};
typedef uint8_t lv_task_prio_t;
typedef struct _lv_task_t
{
uint32_t period; 
uint32_t last_run; 
lv_task_cb_t task_cb; 
void * user_data; 
uint8_t prio : 3; 
uint8_t once : 1; 
} lv_task_t;
void lv_task_core_init(void);
LV_ATTRIBUTE_TASK_HANDLER void lv_task_handler(void);
lv_task_t * lv_task_create_basic(void);
lv_task_t * lv_task_create(lv_task_cb_t task_xcb, uint32_t period, lv_task_prio_t prio, void * user_data);
void lv_task_del(lv_task_t * task);
void lv_task_set_cb(lv_task_t * task, lv_task_cb_t task_cb);
void lv_task_set_prio(lv_task_t * task, lv_task_prio_t prio);
void lv_task_set_period(lv_task_t * task, uint32_t period);
void lv_task_ready(lv_task_t * task);
void lv_task_once(lv_task_t * task);
void lv_task_reset(lv_task_t * task);
void lv_task_enable(bool en);
uint8_t lv_task_get_idle(void);
#if defined(__cplusplus)
} 
#endif
