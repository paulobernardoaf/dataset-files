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
#if !defined(LV_ATTRIBUTE_TICK_INC)
#define LV_ATTRIBUTE_TICK_INC
#endif
LV_ATTRIBUTE_TICK_INC void lv_tick_inc(uint32_t tick_period);
uint32_t lv_tick_get(void);
uint32_t lv_tick_elaps(uint32_t prev_tick);
#if defined(__cplusplus)
} 
#endif
