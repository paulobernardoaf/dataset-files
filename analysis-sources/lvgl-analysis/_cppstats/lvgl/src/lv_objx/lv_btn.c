#include "lv_btn.h"
#if LV_USE_BTN != 0
#include <string.h>
#include "../lv_core/lv_group.h"
#include "../lv_core/lv_debug.h"
#include "../lv_draw/lv_draw.h"
#include "../lv_themes/lv_theme.h"
#include "../lv_misc/lv_area.h"
#include "../lv_misc/lv_color.h"
#include "../lv_misc/lv_math.h"
#define LV_OBJX_NAME "lv_btn"
#define LV_BTN_INK_VALUE_MAX 256
#define LV_BTN_INK_VALUE_MAX_SHIFT 8
static bool lv_btn_design(lv_obj_t * btn, const lv_area_t * mask, lv_design_mode_t mode);
static lv_res_t lv_btn_signal(lv_obj_t * btn, lv_signal_t sign, void * param);
#if LV_USE_ANIMATION && LV_BTN_INK_EFFECT
static void lv_btn_ink_effect_anim(lv_obj_t * btn, lv_anim_value_t val);
static void lv_btn_ink_effect_anim_ready(lv_anim_t * a);
#endif
static lv_signal_cb_t ancestor_signal;
static lv_design_cb_t ancestor_design;
#if LV_USE_ANIMATION && LV_BTN_INK_EFFECT
static lv_coord_t ink_act_value;
static lv_obj_t * ink_obj;
static lv_btn_state_t ink_bg_state;
static lv_btn_state_t ink_top_state;
static bool ink_ready;
static bool ink_playback;
static lv_point_t ink_point;
#endif
lv_obj_t * lv_btn_create(lv_obj_t * par, const lv_obj_t * copy)
{
LV_LOG_TRACE("button create started");
lv_obj_t * new_btn;
new_btn = lv_cont_create(par, copy);
LV_ASSERT_MEM(new_btn);
if(new_btn == NULL) return NULL;
if(ancestor_signal == NULL) ancestor_signal = lv_obj_get_signal_cb(new_btn);
if(ancestor_design == NULL) ancestor_design = lv_obj_get_design_cb(new_btn);
lv_btn_ext_t * ext = lv_obj_allocate_ext_attr(new_btn, sizeof(lv_btn_ext_t));
LV_ASSERT_MEM(ext);
if(ext == NULL) return NULL;
ext->state = LV_BTN_STATE_REL;
ext->styles[LV_BTN_STATE_REL] = &lv_style_btn_rel;
ext->styles[LV_BTN_STATE_PR] = &lv_style_btn_pr;
ext->styles[LV_BTN_STATE_TGL_REL] = &lv_style_btn_tgl_rel;
ext->styles[LV_BTN_STATE_TGL_PR] = &lv_style_btn_tgl_pr;
ext->styles[LV_BTN_STATE_INA] = &lv_style_btn_ina;
ext->toggle = 0;
#if LV_USE_ANIMATION && LV_BTN_INK_EFFECT
ext->ink_in_time = 0;
ext->ink_wait_time = 0;
ext->ink_out_time = 0;
#endif
lv_obj_set_signal_cb(new_btn, lv_btn_signal);
lv_obj_set_design_cb(new_btn, lv_btn_design);
if(copy == NULL) {
if(par != NULL) {
lv_btn_set_layout(new_btn, LV_LAYOUT_CENTER);
}
lv_obj_set_click(new_btn, true); 
lv_theme_t * th = lv_theme_get_current();
if(th) {
lv_btn_set_style(new_btn, LV_BTN_STYLE_REL, th->style.btn.rel);
lv_btn_set_style(new_btn, LV_BTN_STYLE_PR, th->style.btn.pr);
lv_btn_set_style(new_btn, LV_BTN_STYLE_TGL_REL, th->style.btn.tgl_rel);
lv_btn_set_style(new_btn, LV_BTN_STYLE_TGL_PR, th->style.btn.tgl_pr);
lv_btn_set_style(new_btn, LV_BTN_STYLE_INA, th->style.btn.ina);
} else {
lv_obj_set_style(new_btn, ext->styles[LV_BTN_STATE_REL]);
}
}
else {
lv_btn_ext_t * copy_ext = lv_obj_get_ext_attr(copy);
ext->state = copy_ext->state;
ext->toggle = copy_ext->toggle;
#if LV_USE_ANIMATION && LV_BTN_INK_EFFECT
ext->ink_in_time = copy_ext->ink_in_time;
ext->ink_wait_time = copy_ext->ink_wait_time;
ext->ink_out_time = copy_ext->ink_out_time;
#endif
memcpy((void*) ext->styles, copy_ext->styles, sizeof(ext->styles));
lv_obj_refresh_style(new_btn);
}
LV_LOG_INFO("button created");
return new_btn;
}
void lv_btn_set_toggle(lv_obj_t * btn, bool tgl)
{
LV_ASSERT_OBJ(btn, LV_OBJX_NAME);
lv_btn_ext_t * ext = lv_obj_get_ext_attr(btn);
ext->toggle = tgl != false ? 1 : 0;
}
void lv_btn_set_state(lv_obj_t * btn, lv_btn_state_t state)
{
LV_ASSERT_OBJ(btn, LV_OBJX_NAME);
lv_btn_ext_t * ext = lv_obj_get_ext_attr(btn);
if(ext->state != state) {
ext->state = state;
lv_obj_set_style(btn, ext->styles[state]);
}
}
void lv_btn_toggle(lv_obj_t * btn)
{
LV_ASSERT_OBJ(btn, LV_OBJX_NAME);
lv_btn_ext_t * ext = lv_obj_get_ext_attr(btn);
switch(ext->state) {
case LV_BTN_STATE_REL: lv_btn_set_state(btn, LV_BTN_STATE_TGL_REL); break;
case LV_BTN_STATE_PR: lv_btn_set_state(btn, LV_BTN_STATE_TGL_PR); break;
case LV_BTN_STATE_TGL_REL: lv_btn_set_state(btn, LV_BTN_STATE_REL); break;
case LV_BTN_STATE_TGL_PR: lv_btn_set_state(btn, LV_BTN_STATE_PR); break;
default: break;
}
}
void lv_btn_set_ink_in_time(lv_obj_t * btn, uint16_t time)
{
LV_ASSERT_OBJ(btn, LV_OBJX_NAME);
#if LV_USE_ANIMATION && LV_BTN_INK_EFFECT
lv_btn_ext_t * ext = lv_obj_get_ext_attr(btn);
ext->ink_in_time = time;
#else
(void)btn; 
(void)time; 
LV_LOG_WARN("`lv_btn_set_ink_ink_time` has no effect if LV_BTN_INK_EFEFCT or LV_USE_ANIMATION "
"is disabled")
#endif
}
void lv_btn_set_ink_wait_time(lv_obj_t * btn, uint16_t time)
{
LV_ASSERT_OBJ(btn, LV_OBJX_NAME);
#if LV_USE_ANIMATION && LV_BTN_INK_EFFECT
lv_btn_ext_t * ext = lv_obj_get_ext_attr(btn);
ext->ink_wait_time = time;
#else
(void)btn; 
(void)time; 
LV_LOG_WARN("`lv_btn_set_ink_wait_time` has no effect if LV_BTN_INK_EFEFCT or LV_USE_ANIMATION "
"is disabled")
#endif
}
void lv_btn_set_ink_out_time(lv_obj_t * btn, uint16_t time)
{
LV_ASSERT_OBJ(btn, LV_OBJX_NAME);
#if LV_USE_ANIMATION && LV_BTN_INK_EFFECT
lv_btn_ext_t * ext = lv_obj_get_ext_attr(btn);
ext->ink_out_time = time;
#else
(void)btn; 
(void)time; 
LV_LOG_WARN("`lv_btn_set_ink_out_time` has no effect if LV_BTN_INK_EFEFCT or LV_USE_ANIMATION "
"is disabled")
#endif
}
void lv_btn_set_style(lv_obj_t * btn, lv_btn_style_t type, const lv_style_t * style)
{
LV_ASSERT_OBJ(btn, LV_OBJX_NAME);
lv_btn_ext_t * ext = lv_obj_get_ext_attr(btn);
switch(type) {
case LV_BTN_STYLE_REL: ext->styles[LV_BTN_STATE_REL] = style; break;
case LV_BTN_STYLE_PR: ext->styles[LV_BTN_STATE_PR] = style; break;
case LV_BTN_STYLE_TGL_REL: ext->styles[LV_BTN_STATE_TGL_REL] = style; break;
case LV_BTN_STYLE_TGL_PR: ext->styles[LV_BTN_STATE_TGL_PR] = style; break;
case LV_BTN_STYLE_INA: ext->styles[LV_BTN_STATE_INA] = style; break;
}
lv_obj_set_style(btn, ext->styles[ext->state]);
}
lv_btn_state_t lv_btn_get_state(const lv_obj_t * btn)
{
LV_ASSERT_OBJ(btn, LV_OBJX_NAME);
lv_btn_ext_t * ext = lv_obj_get_ext_attr(btn);
return ext->state;
}
bool lv_btn_get_toggle(const lv_obj_t * btn)
{
LV_ASSERT_OBJ(btn, LV_OBJX_NAME);
lv_btn_ext_t * ext = lv_obj_get_ext_attr(btn);
return ext->toggle != 0 ? true : false;
}
uint16_t lv_btn_get_ink_in_time(const lv_obj_t * btn)
{
LV_ASSERT_OBJ(btn, LV_OBJX_NAME);
#if LV_USE_ANIMATION && LV_BTN_INK_EFFECT
lv_btn_ext_t * ext = lv_obj_get_ext_attr(btn);
return ext->ink_in_time;
#else
(void)btn; 
return 0;
#endif
}
uint16_t lv_btn_get_ink_wait_time(const lv_obj_t * btn)
{
LV_ASSERT_OBJ(btn, LV_OBJX_NAME);
#if LV_USE_ANIMATION && LV_BTN_INK_EFFECT
lv_btn_ext_t * ext = lv_obj_get_ext_attr(btn);
return ext->ink_wait_time;
#else
(void)btn; 
return 0;
#endif
}
uint16_t lv_btn_get_ink_out_time(const lv_obj_t * btn)
{
LV_ASSERT_OBJ(btn, LV_OBJX_NAME);
#if LV_USE_ANIMATION && LV_BTN_INK_EFFECT
lv_btn_ext_t * ext = lv_obj_get_ext_attr(btn);
return ext->ink_out_time;
#else
(void)btn; 
return 0;
#endif
}
const lv_style_t * lv_btn_get_style(const lv_obj_t * btn, lv_btn_style_t type)
{
LV_ASSERT_OBJ(btn, LV_OBJX_NAME);
const lv_style_t * style = NULL;
lv_btn_ext_t * ext = lv_obj_get_ext_attr(btn);
lv_btn_state_t state = lv_btn_get_state(btn);
if((type == LV_BTN_STYLE_REL && state == LV_BTN_STATE_REL) ||
(type == LV_BTN_STYLE_PR && state == LV_BTN_STATE_PR) ||
(type == LV_BTN_STYLE_TGL_REL && state == LV_BTN_STATE_TGL_REL) ||
(type == LV_BTN_STYLE_TGL_PR && state == LV_BTN_STATE_TGL_PR) ||
(type == LV_BTN_STYLE_INA && state == LV_BTN_STATE_INA)) {
style = lv_obj_get_style(btn);
} else {
switch(type) {
case LV_BTN_STYLE_REL: style = ext->styles[LV_BTN_STATE_REL]; break;
case LV_BTN_STYLE_PR: style = ext->styles[LV_BTN_STATE_PR]; break;
case LV_BTN_STYLE_TGL_REL: style = ext->styles[LV_BTN_STATE_TGL_REL]; break;
case LV_BTN_STYLE_TGL_PR: style = ext->styles[LV_BTN_STATE_TGL_PR]; break;
case LV_BTN_STYLE_INA: style = ext->styles[LV_BTN_STATE_INA]; break;
default: style = NULL; break;
}
}
return style;
}
static bool lv_btn_design(lv_obj_t * btn, const lv_area_t * mask, lv_design_mode_t mode)
{
if(mode == LV_DESIGN_COVER_CHK) {
return false;
} else if(mode == LV_DESIGN_DRAW_MAIN) {
#if LV_USE_ANIMATION && LV_BTN_INK_EFFECT
if(btn != ink_obj) {
ancestor_design(btn, mask, mode);
} else {
lv_opa_t opa_scale = lv_obj_get_opa_scale(btn);
lv_btn_ext_t * ext = lv_obj_get_ext_attr(btn);
if(ink_playback == false) {
lv_style_t style_tmp;
lv_style_copy(&style_tmp, ext->styles[ink_bg_state]);
style_tmp.body.shadow.width = ext->styles[ink_top_state]->body.shadow.width;
lv_draw_rect(&btn->coords, mask, &style_tmp, opa_scale);
lv_coord_t w = lv_obj_get_width(btn);
lv_coord_t h = lv_obj_get_height(btn);
lv_coord_t r_max = LV_MATH_MIN(w, h) / 2;
lv_area_t cir_area;
lv_coord_t coord_state =
ink_act_value < LV_BTN_INK_VALUE_MAX / 2 ? ink_act_value : LV_BTN_INK_VALUE_MAX / 2;
lv_point_t p_act;
p_act.x = ink_point.x;
p_act.y = ink_point.y;
lv_coord_t x_err = (btn->coords.x1 + w / 2) - p_act.x;
lv_coord_t y_err = (btn->coords.y1 + h / 2) - p_act.y;
p_act.x += (x_err * coord_state) >> (LV_BTN_INK_VALUE_MAX_SHIFT - 1);
p_act.y += (y_err * coord_state) >> (LV_BTN_INK_VALUE_MAX_SHIFT - 1);
lv_coord_t half_side = LV_MATH_MAX(w, h) / 2;
cir_area.x1 = p_act.x - ((half_side * coord_state) >> (LV_BTN_INK_VALUE_MAX_SHIFT - 1));
cir_area.y1 = p_act.y - ((half_side * coord_state) >> (LV_BTN_INK_VALUE_MAX_SHIFT - 1));
cir_area.x2 = p_act.x + ((half_side * coord_state) >> (LV_BTN_INK_VALUE_MAX_SHIFT - 1));
cir_area.y2 = p_act.y + ((half_side * coord_state) >> (LV_BTN_INK_VALUE_MAX_SHIFT - 1));
lv_area_intersect(&cir_area, &btn->coords,
&cir_area); 
lv_coord_t r_state =
ink_act_value > LV_BTN_INK_VALUE_MAX / 2 ? ink_act_value - LV_BTN_INK_VALUE_MAX / 2 : 0;
lv_style_copy(&style_tmp, ext->styles[ink_top_state]);
style_tmp.body.radius = r_max + (((ext->styles[ink_bg_state]->body.radius - r_max) * r_state) >>
(LV_BTN_INK_VALUE_MAX_SHIFT - 1));
style_tmp.body.border.width = 0;
lv_draw_rect(&cir_area, mask, &style_tmp, opa_scale);
} else {
lv_style_t res;
lv_style_copy(&res, ext->styles[ink_bg_state]);
lv_style_mix(ext->styles[ink_bg_state], ext->styles[ink_top_state], &res, ink_act_value);
lv_draw_rect(&btn->coords, mask, &res, opa_scale);
}
}
#else
ancestor_design(btn, mask, mode);
#endif
} else if(mode == LV_DESIGN_DRAW_POST) {
ancestor_design(btn, mask, mode);
}
return true;
}
static lv_res_t lv_btn_signal(lv_obj_t * btn, lv_signal_t sign, void * param)
{
lv_res_t res;
res = ancestor_signal(btn, sign, param);
if(res != LV_RES_OK) return res;
if(sign == LV_SIGNAL_GET_TYPE) return lv_obj_handle_get_type_signal(param, LV_OBJX_NAME);
lv_btn_ext_t * ext = lv_obj_get_ext_attr(btn);
bool tgl = lv_btn_get_toggle(btn);
if(sign == LV_SIGNAL_PRESSED) {
if(ext->state == LV_BTN_STATE_REL) {
lv_btn_set_state(btn, LV_BTN_STATE_PR);
#if LV_USE_ANIMATION && LV_BTN_INK_EFFECT
ink_bg_state = LV_BTN_STATE_REL;
ink_top_state = LV_BTN_STATE_PR;
#endif
} else if(ext->state == LV_BTN_STATE_TGL_REL) {
lv_btn_set_state(btn, LV_BTN_STATE_TGL_PR);
#if LV_USE_ANIMATION && LV_BTN_INK_EFFECT
ink_bg_state = LV_BTN_STATE_TGL_REL;
ink_top_state = LV_BTN_STATE_TGL_PR;
#endif
}
#if LV_USE_ANIMATION && LV_BTN_INK_EFFECT
if(ink_obj != NULL && ink_obj != btn) {
lv_anim_del(ink_obj, (lv_anim_exec_xcb_t)lv_btn_ink_effect_anim);
lv_obj_invalidate(ink_obj);
ink_obj = NULL;
}
if(ext->ink_in_time > 0) {
ink_obj = btn;
ink_playback = false;
ink_ready = false;
lv_indev_get_point(lv_indev_get_act(), &ink_point);
lv_anim_t a;
a.var = btn;
a.start = 0;
a.end = LV_BTN_INK_VALUE_MAX;
a.exec_cb = (lv_anim_exec_xcb_t)lv_btn_ink_effect_anim;
a.path_cb = lv_anim_path_linear;
a.ready_cb = lv_btn_ink_effect_anim_ready;
a.act_time = 0;
a.time = ext->ink_in_time;
a.playback = 0;
a.playback_pause = 0;
a.repeat = 0;
a.repeat_pause = 0;
lv_anim_create(&a);
}
#endif
} else if(sign == LV_SIGNAL_PRESS_LOST) {
if(ext->state == LV_BTN_STATE_PR)
lv_btn_set_state(btn, LV_BTN_STATE_REL);
else if(ext->state == LV_BTN_STATE_TGL_PR)
lv_btn_set_state(btn, LV_BTN_STATE_TGL_REL);
} else if(sign == LV_SIGNAL_PRESSING) {
if(lv_indev_is_dragging(param) != false) {
if(ext->state == LV_BTN_STATE_PR)
lv_btn_set_state(btn, LV_BTN_STATE_REL);
else if(ext->state == LV_BTN_STATE_TGL_PR)
lv_btn_set_state(btn, LV_BTN_STATE_TGL_REL);
}
} else if(sign == LV_SIGNAL_RELEASED) {
if(lv_indev_is_dragging(param) == false) {
uint32_t toggled = 0;
if(ext->state == LV_BTN_STATE_PR && tgl == false) {
lv_btn_set_state(btn, LV_BTN_STATE_REL);
toggled = 0;
} else if(ext->state == LV_BTN_STATE_TGL_PR && tgl == false) {
lv_btn_set_state(btn, LV_BTN_STATE_TGL_REL);
toggled = 1;
} else if(ext->state == LV_BTN_STATE_PR && tgl == true) {
lv_btn_set_state(btn, LV_BTN_STATE_TGL_REL);
toggled = 1;
} else if(ext->state == LV_BTN_STATE_TGL_PR && tgl == true) {
lv_btn_set_state(btn, LV_BTN_STATE_REL);
toggled = 0;
}
if(tgl) {
res = lv_event_send(btn, LV_EVENT_VALUE_CHANGED, &toggled);
if(res != LV_RES_OK) return res;
}
} else { 
if(ext->state == LV_BTN_STATE_PR) {
lv_btn_set_state(btn, LV_BTN_STATE_REL);
} else if(ext->state == LV_BTN_STATE_TGL_PR) {
lv_btn_set_state(btn, LV_BTN_STATE_TGL_REL);
}
}
#if LV_USE_ANIMATION && LV_BTN_INK_EFFECT
if(ext->toggle) {
ink_top_state = ext->state;
}
else if(ink_ready && ext->ink_out_time > 0) {
ink_obj = btn;
ink_playback = true; 
lv_indev_get_point(lv_indev_get_act(), &ink_point);
lv_anim_t a;
a.var = ink_obj;
a.start = LV_BTN_INK_VALUE_MAX;
a.end = 0;
a.exec_cb = (lv_anim_exec_xcb_t)lv_btn_ink_effect_anim;
a.path_cb = lv_anim_path_linear;
a.ready_cb = lv_btn_ink_effect_anim_ready;
a.act_time = 0;
a.time = ext->ink_out_time;
a.playback = 0;
a.playback_pause = 0;
a.repeat = 0;
a.repeat_pause = 0;
lv_anim_create(&a);
}
#endif
} else if(sign == LV_SIGNAL_CONTROL) {
char c = *((char *)param);
if(c == LV_KEY_RIGHT || c == LV_KEY_UP) {
if(lv_btn_get_toggle(btn)) {
lv_btn_set_state(btn, LV_BTN_STATE_TGL_REL);
uint32_t state = 1;
res = lv_event_send(btn, LV_EVENT_VALUE_CHANGED, &state);
if(res != LV_RES_OK) return res;
}
} else if(c == LV_KEY_LEFT || c == LV_KEY_DOWN) {
if(lv_btn_get_toggle(btn)) {
lv_btn_set_state(btn, LV_BTN_STATE_REL);
uint32_t state = 0;
res = lv_event_send(btn, LV_EVENT_VALUE_CHANGED, &state);
if(res != LV_RES_OK) return res;
}
}
} else if(sign == LV_SIGNAL_CLEANUP) {
#if LV_USE_ANIMATION && LV_BTN_INK_EFFECT
if(btn == ink_obj) {
lv_anim_del(ink_obj, (lv_anim_exec_xcb_t)lv_btn_ink_effect_anim);
ink_obj = NULL;
}
#endif
}
return res;
}
#if LV_USE_ANIMATION && LV_BTN_INK_EFFECT
static void lv_btn_ink_effect_anim(lv_obj_t * btn, lv_anim_value_t val)
{
if(btn) {
ink_act_value = val;
lv_obj_invalidate(btn);
}
}
static void lv_btn_ink_effect_anim_ready(lv_anim_t * a)
{
(void)a; 
lv_btn_ext_t * ext = lv_obj_get_ext_attr(ink_obj);
lv_btn_state_t state = lv_btn_get_state(ink_obj);
lv_obj_invalidate(ink_obj);
ink_ready = true;
if((state == LV_BTN_STATE_REL || state == LV_BTN_STATE_TGL_REL) && ext->toggle == 0 && ink_playback == false) {
lv_anim_t new_a;
new_a.var = ink_obj;
new_a.start = LV_BTN_INK_VALUE_MAX;
new_a.end = 0;
new_a.exec_cb = (lv_anim_exec_xcb_t)lv_btn_ink_effect_anim;
new_a.path_cb = lv_anim_path_linear;
new_a.ready_cb = lv_btn_ink_effect_anim_ready;
new_a.act_time = -ext->ink_wait_time;
new_a.time = ext->ink_out_time;
new_a.playback = 0;
new_a.playback_pause = 0;
new_a.repeat = 0;
new_a.repeat_pause = 0;
lv_anim_create(&new_a);
ink_playback = true;
} else {
ink_obj = NULL;
}
}
#endif 
#endif
