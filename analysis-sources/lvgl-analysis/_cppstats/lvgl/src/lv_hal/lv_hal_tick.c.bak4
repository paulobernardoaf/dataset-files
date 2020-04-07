







#if defined(LV_CONF_INCLUDE_SIMPLE)
#include "lv_conf.h"
#else
#include "../../../lv_conf.h"
#endif

#include "lv_hal_tick.h"
#include <stddef.h>

#if LV_TICK_CUSTOM == 1
#include LV_TICK_CUSTOM_INCLUDE
#endif
















static uint32_t sys_time = 0;
static volatile uint8_t tick_irq_flag;













LV_ATTRIBUTE_TICK_INC void lv_tick_inc(uint32_t tick_period)
{
tick_irq_flag = 0;
sys_time += tick_period;
}





uint32_t lv_tick_get(void)
{
#if LV_TICK_CUSTOM == 0
uint32_t result;
do {
tick_irq_flag = 1;
result = sys_time;
} while(!tick_irq_flag); 


return result;
#else
return LV_TICK_CUSTOM_SYS_TIME_EXPR;
#endif
}






uint32_t lv_tick_elaps(uint32_t prev_tick)
{
uint32_t act_time = lv_tick_get();


if(act_time >= prev_tick) {
prev_tick = act_time - prev_tick;
} else {
prev_tick = UINT32_MAX - prev_tick + 1;
prev_tick += act_time;
}

return prev_tick;
}




