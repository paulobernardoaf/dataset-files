




#if !defined(LV_LED_H)
#define LV_LED_H

#if defined(__cplusplus)
extern "C" {
#endif




#if defined(LV_CONF_INCLUDE_SIMPLE)
#include "lv_conf.h"
#else
#include "../../../lv_conf.h"
#endif

#if LV_USE_LED != 0

#include "../lv_core/lv_obj.h"










typedef struct
{


uint8_t bright; 
} lv_led_ext_t;


enum {
LV_LED_STYLE_MAIN,
};
typedef uint8_t lv_led_style_t;











lv_obj_t * lv_led_create(lv_obj_t * par, const lv_obj_t * copy);






void lv_led_set_bright(lv_obj_t * led, uint8_t bright);





void lv_led_on(lv_obj_t * led);





void lv_led_off(lv_obj_t * led);





void lv_led_toggle(lv_obj_t * led);







static inline void lv_led_set_style(lv_obj_t * led, lv_led_style_t type, const lv_style_t * style)
{
(void)type; 
lv_obj_set_style(led, style);
}






uint8_t lv_led_get_bright(const lv_obj_t * led);







static inline const lv_style_t * lv_led_get_style(const lv_obj_t * led, lv_led_style_t type)
{
(void)type; 
return lv_obj_get_style(led);
}





#endif 

#if defined(__cplusplus)
} 
#endif

#endif 
