#if defined(__cplusplus)
extern "C" {
#endif
#if defined(LV_CONF_INCLUDE_SIMPLE)
#include "lv_conf.h"
#else
#include "../../../lv_conf.h"
#endif
#if LV_USE_SW != 0
#if LV_USE_SLIDER == 0
#error "lv_sw: lv_slider is required. Enable it in lv_conf.h (LV_USE_SLIDER 1)"
#endif
#include "../lv_core/lv_obj.h"
#include "lv_slider.h"
#define LV_SW_MAX_VALUE 100
typedef struct
{
lv_slider_ext_t slider; 
const lv_style_t * style_knob_off; 
const lv_style_t * style_knob_on; 
lv_coord_t start_x;
uint8_t changed : 1; 
uint8_t slided : 1;
#if LV_USE_ANIMATION
uint16_t anim_time; 
#endif
} lv_sw_ext_t;
enum {
LV_SW_STYLE_BG, 
LV_SW_STYLE_INDIC, 
LV_SW_STYLE_KNOB_OFF, 
LV_SW_STYLE_KNOB_ON, 
};
typedef uint8_t lv_sw_style_t;
lv_obj_t * lv_sw_create(lv_obj_t * par, const lv_obj_t * copy);
void lv_sw_on(lv_obj_t * sw, lv_anim_enable_t anim);
void lv_sw_off(lv_obj_t * sw, lv_anim_enable_t anim);
bool lv_sw_toggle(lv_obj_t * sw, lv_anim_enable_t anim);
void lv_sw_set_style(lv_obj_t * sw, lv_sw_style_t type, const lv_style_t * style);
void lv_sw_set_anim_time(lv_obj_t * sw, uint16_t anim_time);
static inline bool lv_sw_get_state(const lv_obj_t * sw)
{
return lv_bar_get_value(sw) < LV_SW_MAX_VALUE / 2 ? false : true;
}
const lv_style_t * lv_sw_get_style(const lv_obj_t * sw, lv_sw_style_t type);
uint16_t lv_sw_get_anim_time(const lv_obj_t * sw);
#endif 
#if defined(__cplusplus)
} 
#endif
