




#if !defined(LV_SLIDER_H)
#define LV_SLIDER_H

#if defined(__cplusplus)
extern "C" {
#endif




#if defined(LV_CONF_INCLUDE_SIMPLE)
#include "lv_conf.h"
#else
#include "../../../lv_conf.h"
#endif

#if LV_USE_SLIDER != 0


#if LV_USE_BAR == 0
#error "lv_slider: lv_bar is required. Enable it in lv_conf.h (LV_USE_BAR 1) "
#endif

#include "../lv_core/lv_obj.h"
#include "lv_bar.h"









typedef struct
{
lv_bar_ext_t bar; 

const lv_style_t * style_knob; 
int16_t drag_value; 
uint8_t knob_in : 1; 
} lv_slider_ext_t;


enum {
LV_SLIDER_STYLE_BG, 
LV_SLIDER_STYLE_INDIC, 
LV_SLIDER_STYLE_KNOB, 
};
typedef uint8_t lv_slider_style_t;











lv_obj_t * lv_slider_create(lv_obj_t * par, const lv_obj_t * copy);











static inline void lv_slider_set_value(lv_obj_t * slider, int16_t value, lv_anim_enable_t anim)
{
lv_bar_set_value(slider, value, anim);
}







static inline void lv_slider_set_range(lv_obj_t * slider, int16_t min, int16_t max)
{
lv_bar_set_range(slider, min, max);
}







static inline void lv_slider_set_anim_time(lv_obj_t * slider, uint16_t anim_time)
{
lv_bar_set_anim_time(slider, anim_time);
}






static inline void lv_slider_set_sym(lv_obj_t * slider, bool en)
{
lv_bar_set_sym(slider, en);
}







void lv_slider_set_knob_in(lv_obj_t * slider, bool in);







void lv_slider_set_style(lv_obj_t * slider, lv_slider_style_t type, const lv_style_t * style);










int16_t lv_slider_get_value(const lv_obj_t * slider);






static inline int16_t lv_slider_get_min_value(const lv_obj_t * slider)
{
return lv_bar_get_min_value(slider);
}






static inline int16_t lv_slider_get_max_value(const lv_obj_t * slider)
{
return lv_bar_get_max_value(slider);
}






bool lv_slider_is_dragged(const lv_obj_t * slider);






static inline uint16_t lv_slider_get_anim_time(lv_obj_t * slider)
{
return lv_bar_get_anim_time(slider);
}






static inline bool lv_slider_get_sym(lv_obj_t * slider)
{
return lv_bar_get_sym(slider);
}







bool lv_slider_get_knob_in(const lv_obj_t * slider);







const lv_style_t * lv_slider_get_style(const lv_obj_t * slider, lv_slider_style_t type);





#endif 

#if defined(__cplusplus)
} 
#endif

#endif 
