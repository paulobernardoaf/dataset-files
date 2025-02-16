







#include "lv_sw.h"

#if LV_USE_SW != 0


#if LV_USE_SLIDER == 0
#error "lv_sw: lv_slider is required. Enable it in lv_conf.h (LV_USE_SLIDER 1) "
#endif

#include "../lv_core/lv_debug.h"
#include "../lv_themes/lv_theme.h"
#include "../lv_misc/lv_math.h"
#include "../lv_core/lv_indev.h"




#define LV_OBJX_NAME "lv_sw"








static lv_res_t lv_sw_signal(lv_obj_t * sw, lv_signal_t sign, void * param);




static lv_signal_cb_t ancestor_signal;















lv_obj_t * lv_sw_create(lv_obj_t * par, const lv_obj_t * copy)
{
LV_LOG_TRACE("switch create started");


lv_obj_t * new_sw = lv_slider_create(par, copy);
LV_ASSERT_MEM(new_sw);
if(new_sw == NULL) return NULL;

if(ancestor_signal == NULL) ancestor_signal = lv_obj_get_signal_cb(new_sw);


lv_sw_ext_t * ext = lv_obj_allocate_ext_attr(new_sw, sizeof(lv_sw_ext_t));
LV_ASSERT_MEM(ext);
if(ext == NULL) return NULL;


ext->changed = 0;
#if LV_USE_ANIMATION
ext->anim_time = 0;
#endif
ext->style_knob_off = ext->slider.style_knob;
ext->style_knob_on = ext->slider.style_knob;


lv_obj_set_signal_cb(new_sw, lv_sw_signal);


if(copy == NULL) {
lv_obj_set_size(new_sw, 2 * LV_DPI / 3, LV_DPI / 3);
lv_slider_set_knob_in(new_sw, true);
lv_slider_set_range(new_sw, 0, LV_SW_MAX_VALUE);


lv_theme_t * th = lv_theme_get_current();
if(th) {
lv_sw_set_style(new_sw, LV_SW_STYLE_BG, th->style.sw.bg);
lv_sw_set_style(new_sw, LV_SW_STYLE_INDIC, th->style.sw.indic);
lv_sw_set_style(new_sw, LV_SW_STYLE_KNOB_OFF, th->style.sw.knob_off);
lv_sw_set_style(new_sw, LV_SW_STYLE_KNOB_ON, th->style.sw.knob_on);
} else {

}

}

else {
lv_sw_ext_t * copy_ext = lv_obj_get_ext_attr(copy);
ext->style_knob_off = copy_ext->style_knob_off;
ext->style_knob_on = copy_ext->style_knob_on;
#if LV_USE_ANIMATION
ext->anim_time = copy_ext->anim_time;
#endif

if(lv_sw_get_state(new_sw))
lv_slider_set_style(new_sw, LV_SLIDER_STYLE_KNOB, ext->style_knob_on);
else
lv_slider_set_style(new_sw, LV_SLIDER_STYLE_KNOB, ext->style_knob_off);


lv_obj_refresh_style(new_sw);
}

LV_LOG_INFO("switch created");

return new_sw;
}










void lv_sw_on(lv_obj_t * sw, lv_anim_enable_t anim)
{
LV_ASSERT_OBJ(sw, LV_OBJX_NAME);

#if LV_USE_ANIMATION == 0
anim = LV_ANIM_OFF;
#endif
lv_sw_ext_t * ext = lv_obj_get_ext_attr(sw);
lv_slider_set_value(sw, LV_SW_MAX_VALUE, anim);
lv_slider_set_style(sw, LV_SLIDER_STYLE_KNOB, ext->style_knob_on);
}






void lv_sw_off(lv_obj_t * sw, lv_anim_enable_t anim)
{
LV_ASSERT_OBJ(sw, LV_OBJX_NAME);

#if LV_USE_ANIMATION == 0
anim = LV_ANIM_OFF;
#endif
lv_sw_ext_t * ext = lv_obj_get_ext_attr(sw);
lv_slider_set_value(sw, 0, anim);
lv_slider_set_style(sw, LV_SLIDER_STYLE_KNOB, ext->style_knob_off);
}







bool lv_sw_toggle(lv_obj_t * sw, lv_anim_enable_t anim)
{
LV_ASSERT_OBJ(sw, LV_OBJX_NAME);

#if LV_USE_ANIMATION == 0
anim = LV_ANIM_OFF;
#endif

bool state = lv_sw_get_state(sw);
if(state)
lv_sw_off(sw, anim);
else
lv_sw_on(sw, anim);

return !state;
}







void lv_sw_set_style(lv_obj_t * sw, lv_sw_style_t type, const lv_style_t * style)
{
LV_ASSERT_OBJ(sw, LV_OBJX_NAME);

lv_sw_ext_t * ext = lv_obj_get_ext_attr(sw);

switch(type) {
case LV_SLIDER_STYLE_BG: lv_slider_set_style(sw, LV_SLIDER_STYLE_BG, style); break;
case LV_SLIDER_STYLE_INDIC: lv_bar_set_style(sw, LV_SLIDER_STYLE_INDIC, style); break;
case LV_SW_STYLE_KNOB_OFF:
ext->style_knob_off = style;
if(lv_sw_get_state(sw) == 0) lv_slider_set_style(sw, LV_SLIDER_STYLE_KNOB, style);
break;
case LV_SW_STYLE_KNOB_ON:
ext->style_knob_on = style;
if(lv_sw_get_state(sw) != 0) lv_slider_set_style(sw, LV_SLIDER_STYLE_KNOB, style);
break;
}
}

void lv_sw_set_anim_time(lv_obj_t * sw, uint16_t anim_time)
{
LV_ASSERT_OBJ(sw, LV_OBJX_NAME);

#if LV_USE_ANIMATION
lv_sw_ext_t * ext = lv_obj_get_ext_attr(sw);
ext->anim_time = anim_time;
#else
(void)sw;
(void)anim_time;
#endif
}











const lv_style_t * lv_sw_get_style(const lv_obj_t * sw, lv_sw_style_t type)
{
LV_ASSERT_OBJ(sw, LV_OBJX_NAME);

const lv_style_t * style = NULL;
lv_sw_ext_t * ext = lv_obj_get_ext_attr(sw);

switch(type) {
case LV_SW_STYLE_BG: style = lv_slider_get_style(sw, LV_SLIDER_STYLE_BG); break;
case LV_SW_STYLE_INDIC: style = lv_slider_get_style(sw, LV_SLIDER_STYLE_INDIC); break;
case LV_SW_STYLE_KNOB_OFF: style = ext->style_knob_off; break;
case LV_SW_STYLE_KNOB_ON: style = ext->style_knob_on; break;
default: style = NULL; break;
}

return style;
}

uint16_t lv_sw_get_anim_time(const lv_obj_t * sw)
{
LV_ASSERT_OBJ(sw, LV_OBJX_NAME);

#if LV_USE_ANIMATION
lv_sw_ext_t * ext = lv_obj_get_ext_attr(sw);
return ext->anim_time;
#else
(void)sw; 
return 0;
#endif
}












static lv_res_t lv_sw_signal(lv_obj_t * sw, lv_signal_t sign, void * param)
{
lv_res_t res;
if(sign == LV_SIGNAL_GET_TYPE) {
res = ancestor_signal(sw, sign, param);
if(res != LV_RES_OK) return res;
return lv_obj_handle_get_type_signal(param, LV_OBJX_NAME);
}

lv_sw_ext_t * ext = lv_obj_get_ext_attr(sw);



int16_t old_val;
if(sign == LV_SIGNAL_PRESSING)
old_val = ext->slider.drag_value;
else
old_val = lv_slider_get_value(sw);


lv_event_cb_t event_cb = sw->event_cb;
sw->event_cb = NULL;


res = ancestor_signal(sw, sign, param);
if(res != LV_RES_OK) return res;

sw->event_cb = event_cb;

if(sign == LV_SIGNAL_CLEANUP) {

} else if(sign == LV_SIGNAL_PRESSED) {


lv_indev_t * indev = lv_indev_get_act();
if(indev) {
lv_point_t p;
lv_indev_get_point(indev, &p);
ext->start_x = p.x;
}
ext->slided = 0;
ext->changed = 0;
} else if(sign == LV_SIGNAL_PRESSING) {

lv_indev_t * indev = lv_indev_get_act();
if(indev) {
lv_point_t p = {0, 0};
lv_indev_get_point(indev, &p);
if(LV_MATH_ABS(p.x - ext->start_x) > LV_INDEV_DEF_DRAG_LIMIT) ext->slided = 1;
}



if(ext->slided == 0) {
if(lv_sw_get_state(sw))
ext->slider.drag_value = LV_SW_MAX_VALUE;
else
ext->slider.drag_value = 0;
}


int16_t threshold = LV_SW_MAX_VALUE / 2;
if((old_val < threshold && ext->slider.drag_value > threshold) ||
(old_val > threshold && ext->slider.drag_value < threshold)) {
ext->changed = 1;
}
} else if(sign == LV_SIGNAL_PRESS_LOST) {
if(lv_sw_get_state(sw)) {
lv_slider_set_style(sw, LV_SLIDER_STYLE_KNOB, ext->style_knob_on);
lv_slider_set_value(sw, LV_SW_MAX_VALUE, LV_ANIM_ON);
if(res != LV_RES_OK) return res;
} else {
lv_slider_set_style(sw, LV_SLIDER_STYLE_KNOB, ext->style_knob_off);
lv_slider_set_value(sw, 0, LV_ANIM_ON);
if(res != LV_RES_OK) return res;
}
} else if(sign == LV_SIGNAL_RELEASED) {

if(ext->changed == 0) {
int32_t state;
if(lv_sw_get_state(sw)) {
lv_sw_off(sw, LV_ANIM_ON);
state = 0;
} else {
lv_sw_on(sw, LV_ANIM_ON);
state = 1;
}

res = lv_event_send(sw, LV_EVENT_VALUE_CHANGED, &state);
if(res != LV_RES_OK) return res;
}

else {
int16_t v = lv_slider_get_value(sw);
int32_t state;
if(v > LV_SW_MAX_VALUE / 2) {
lv_sw_on(sw, LV_ANIM_ON);
state = 1;
} else {
lv_sw_off(sw, LV_ANIM_ON);
state = 0;
}
res = lv_event_send(sw, LV_EVENT_VALUE_CHANGED, &state);
if(res != LV_RES_OK) return res;
}
} else if(sign == LV_SIGNAL_CONTROL) {
char c = *((char *)param);
uint32_t state;
if(c == LV_KEY_RIGHT || c == LV_KEY_UP) {
lv_slider_set_value(sw, LV_SW_MAX_VALUE, true);
state = 1;
res = lv_event_send(sw, LV_EVENT_VALUE_CHANGED, &state);
if(res != LV_RES_OK) return res;
} else if(c == LV_KEY_LEFT || c == LV_KEY_DOWN) {
lv_slider_set_value(sw, 0, true);
state = 0;
res = lv_event_send(sw, LV_EVENT_VALUE_CHANGED, &state);
if(res != LV_RES_OK) return res;
}
} else if(sign == LV_SIGNAL_GET_EDITABLE) {
bool * editable = (bool *)param;
*editable = false; 
}

return res;
}

#endif
