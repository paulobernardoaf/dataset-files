#include "lv_led.h"
#if LV_USE_LED != 0
#include "../lv_core/lv_debug.h"
#include "../lv_themes/lv_theme.h"
#include "../lv_draw/lv_draw.h"
#define LV_OBJX_NAME "lv_led"
#define LV_LED_WIDTH_DEF (LV_DPI / 3)
#define LV_LED_HEIGHT_DEF (LV_DPI / 3)
#define LV_LED_BRIGHT_OFF 100
#define LV_LED_BRIGHT_ON 255
static bool lv_led_design(lv_obj_t * led, const lv_area_t * mask, lv_design_mode_t mode);
static lv_res_t lv_led_signal(lv_obj_t * led, lv_signal_t sign, void * param);
static lv_design_cb_t ancestor_design_f;
static lv_signal_cb_t ancestor_signal;
lv_obj_t * lv_led_create(lv_obj_t * par, const lv_obj_t * copy)
{
LV_LOG_TRACE("led create started");
lv_obj_t * new_led = lv_obj_create(par, copy);
LV_ASSERT_MEM(new_led);
if(new_led == NULL) return NULL;
if(ancestor_signal == NULL) ancestor_signal = lv_obj_get_signal_cb(new_led);
if(ancestor_design_f == NULL) ancestor_design_f = lv_obj_get_design_cb(new_led);
lv_led_ext_t * ext = lv_obj_allocate_ext_attr(new_led, sizeof(lv_led_ext_t));
LV_ASSERT_MEM(ext);
if(ext == NULL) return NULL;
ext->bright = LV_LED_BRIGHT_ON;
lv_obj_set_signal_cb(new_led, lv_led_signal);
lv_obj_set_design_cb(new_led, lv_led_design);
if(copy == NULL) {
lv_obj_set_size(new_led, LV_LED_WIDTH_DEF, LV_LED_HEIGHT_DEF);
lv_theme_t * th = lv_theme_get_current();
if(th) {
lv_led_set_style(new_led, LV_LED_STYLE_MAIN, th->style.led);
} else {
lv_led_set_style(new_led, LV_LED_STYLE_MAIN, &lv_style_pretty_color);
}
}
else {
lv_led_ext_t * copy_ext = lv_obj_get_ext_attr(copy);
ext->bright = copy_ext->bright;
lv_obj_refresh_style(new_led);
}
LV_LOG_INFO("led created");
return new_led;
}
void lv_led_set_bright(lv_obj_t * led, uint8_t bright)
{
LV_ASSERT_OBJ(led, LV_OBJX_NAME);
lv_led_ext_t * ext = lv_obj_get_ext_attr(led);
if(ext->bright == bright) return;
ext->bright = bright;
lv_obj_invalidate(led);
}
void lv_led_on(lv_obj_t * led)
{
LV_ASSERT_OBJ(led, LV_OBJX_NAME);
lv_led_set_bright(led, LV_LED_BRIGHT_ON);
}
void lv_led_off(lv_obj_t * led)
{
LV_ASSERT_OBJ(led, LV_OBJX_NAME);
lv_led_set_bright(led, LV_LED_BRIGHT_OFF);
}
void lv_led_toggle(lv_obj_t * led)
{
LV_ASSERT_OBJ(led, LV_OBJX_NAME);
uint8_t bright = lv_led_get_bright(led);
if(bright > (LV_LED_BRIGHT_OFF + LV_LED_BRIGHT_ON) >> 1)
lv_led_off(led);
else
lv_led_on(led);
}
uint8_t lv_led_get_bright(const lv_obj_t * led)
{
LV_ASSERT_OBJ(led, LV_OBJX_NAME);
lv_led_ext_t * ext = lv_obj_get_ext_attr(led);
return ext->bright;
}
static bool lv_led_design(lv_obj_t * led, const lv_area_t * mask, lv_design_mode_t mode)
{
if(mode == LV_DESIGN_COVER_CHK) {
return ancestor_design_f(led, mask, mode);
} else if(mode == LV_DESIGN_DRAW_MAIN) {
lv_led_ext_t * ext = lv_obj_get_ext_attr(led);
const lv_style_t * style = lv_obj_get_style(led);
const lv_style_t * style_ori_p = led->style_p;
lv_style_t leds_tmp;
memcpy(&leds_tmp, style, sizeof(leds_tmp));
leds_tmp.body.main_color = lv_color_mix(leds_tmp.body.main_color, LV_COLOR_BLACK, ext->bright);
leds_tmp.body.grad_color = lv_color_mix(leds_tmp.body.grad_color, LV_COLOR_BLACK, ext->bright);
leds_tmp.body.border.color = lv_color_mix(leds_tmp.body.border.color, LV_COLOR_BLACK, ext->bright);
uint16_t bright_tmp = ext->bright;
leds_tmp.body.shadow.width =
((bright_tmp - LV_LED_BRIGHT_OFF) * style->body.shadow.width) / (LV_LED_BRIGHT_ON - LV_LED_BRIGHT_OFF);
led->style_p = &leds_tmp;
ancestor_design_f(led, mask, mode);
led->style_p = style_ori_p; 
}
return true;
}
static lv_res_t lv_led_signal(lv_obj_t * led, lv_signal_t sign, void * param)
{
lv_res_t res;
res = ancestor_signal(led, sign, param);
if(res != LV_RES_OK) return res;
if(sign == LV_SIGNAL_GET_TYPE) {
lv_obj_type_t * buf = param;
uint8_t i;
for(i = 0; i < LV_MAX_ANCESTOR_NUM - 1; i++) { 
if(buf->type[i] == NULL) break;
}
buf->type[i] = "lv_led";
}
return res;
}
#endif
