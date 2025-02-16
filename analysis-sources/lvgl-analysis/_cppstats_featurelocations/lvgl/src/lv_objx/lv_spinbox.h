




#if !defined(LV_SPINBOX_H)
#define LV_SPINBOX_H

#if defined(__cplusplus)
extern "C" {
#endif




#if defined(LV_CONF_INCLUDE_SIMPLE)
#include "lv_conf.h"
#else
#include "../../../lv_conf.h"
#endif

#if LV_USE_SPINBOX != 0


#if LV_USE_TA == 0
#error "lv_spinbox: lv_ta is required. Enable it in lv_conf.h (LV_USE_TA 1) "
#endif

#include "../lv_core/lv_obj.h"
#include "../lv_objx/lv_ta.h"




#define LV_SPINBOX_MAX_DIGIT_COUNT 16






typedef struct
{
lv_ta_ext_t ta; 

int32_t value;
int32_t range_max;
int32_t range_min;
int32_t step;
uint16_t digit_count : 4;
uint16_t dec_point_pos : 4; 
uint16_t digit_padding_left : 4;
} lv_spinbox_ext_t;


enum {
LV_SPINBOX_STYLE_BG,
LV_SPINBOX_STYLE_SB,
LV_SPINBOX_STYLE_CURSOR,
};
typedef uint8_t lv_spinbox_style_t;











lv_obj_t * lv_spinbox_create(lv_obj_t * par, const lv_obj_t * copy);











static inline void lv_spinbox_set_style(lv_obj_t * spinbox, lv_spinbox_style_t type, lv_style_t * style)
{
lv_ta_set_style(spinbox, type, style);
}






void lv_spinbox_set_value(lv_obj_t * spinbox, int32_t i);








void lv_spinbox_set_digit_format(lv_obj_t * spinbox, uint8_t digit_count, uint8_t separator_position);






void lv_spinbox_set_step(lv_obj_t * spinbox, uint32_t step);







void lv_spinbox_set_range(lv_obj_t * spinbox, int32_t range_min, int32_t range_max);






void lv_spinbox_set_padding_left(lv_obj_t * spinbox, uint8_t padding);











static inline const lv_style_t * lv_spinbox_get_style(lv_obj_t * spinbox, lv_spinbox_style_t type)
{
return lv_ta_get_style(spinbox, type);
}






int32_t lv_spinbox_get_value(lv_obj_t * spinbox);









void lv_spinbox_step_next(lv_obj_t * spinbox);





void lv_spinbox_step_prev(lv_obj_t * spinbox);





void lv_spinbox_increment(lv_obj_t * spinbox);





void lv_spinbox_decrement(lv_obj_t * spinbox);





#endif 

#if defined(__cplusplus)
} 
#endif

#endif 
