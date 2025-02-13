#include "lv_slider.h"
#if LV_USE_SLIDER != 0
#include "../lv_core/lv_debug.h"
#include "../lv_core/lv_group.h"
#include "../lv_core/lv_indev.h"
#include "../lv_draw/lv_draw.h"
#include "../lv_themes/lv_theme.h"
#include "../lv_misc/lv_math.h"
#define LV_OBJX_NAME "lv_slider"
#define LV_SLIDER_SIZE_MIN 4 
#define LV_SLIDER_NOT_PRESSED INT16_MIN
static bool lv_slider_design(lv_obj_t * slider, const lv_area_t * mask, lv_design_mode_t mode);
static lv_res_t lv_slider_signal(lv_obj_t * slider, lv_signal_t sign, void * param);
static lv_design_cb_t ancestor_design_f;
static lv_signal_cb_t ancestor_signal;
lv_obj_t * lv_slider_create(lv_obj_t * par, const lv_obj_t * copy)
{
LV_LOG_TRACE("slider create started");
lv_obj_t * new_slider = lv_bar_create(par, copy);
LV_ASSERT_MEM(new_slider);
if(new_slider == NULL) return NULL;
if(ancestor_design_f == NULL) ancestor_design_f = lv_obj_get_design_cb(new_slider);
if(ancestor_signal == NULL) ancestor_signal = lv_obj_get_signal_cb(new_slider);
lv_slider_ext_t * ext = lv_obj_allocate_ext_attr(new_slider, sizeof(lv_slider_ext_t));
LV_ASSERT_MEM(ext);
if(ext == NULL) return NULL;
ext->drag_value = LV_SLIDER_NOT_PRESSED;
ext->style_knob = &lv_style_pretty;
ext->knob_in = 0;
lv_obj_set_signal_cb(new_slider, lv_slider_signal);
lv_obj_set_design_cb(new_slider, lv_slider_design);
if(copy == NULL) {
lv_obj_set_click(new_slider, true);
lv_obj_set_protect(new_slider, LV_PROTECT_PRESS_LOST);
lv_theme_t * th = lv_theme_get_current();
if(th) {
lv_slider_set_style(new_slider, LV_SLIDER_STYLE_BG, th->style.slider.bg);
lv_slider_set_style(new_slider, LV_SLIDER_STYLE_INDIC, th->style.slider.indic);
lv_slider_set_style(new_slider, LV_SLIDER_STYLE_KNOB, th->style.slider.knob);
} else {
lv_slider_set_style(new_slider, LV_SLIDER_STYLE_KNOB, ext->style_knob);
}
}
else {
lv_slider_ext_t * copy_ext = lv_obj_get_ext_attr(copy);
ext->style_knob = copy_ext->style_knob;
ext->knob_in = copy_ext->knob_in;
lv_obj_refresh_style(new_slider);
}
LV_LOG_INFO("slider created");
return new_slider;
}
void lv_slider_set_knob_in(lv_obj_t * slider, bool in)
{
LV_ASSERT_OBJ(slider, LV_OBJX_NAME);
lv_slider_ext_t * ext = lv_obj_get_ext_attr(slider);
if(ext->knob_in == in) return;
ext->knob_in = in == false ? 0 : 1;
lv_obj_invalidate(slider);
}
void lv_slider_set_style(lv_obj_t * slider, lv_slider_style_t type, const lv_style_t * style)
{
LV_ASSERT_OBJ(slider, LV_OBJX_NAME);
lv_slider_ext_t * ext = lv_obj_get_ext_attr(slider);
switch(type) {
case LV_SLIDER_STYLE_BG: lv_bar_set_style(slider, LV_BAR_STYLE_BG, style); break;
case LV_SLIDER_STYLE_INDIC: lv_bar_set_style(slider, LV_BAR_STYLE_INDIC, style); break;
case LV_SLIDER_STYLE_KNOB:
ext->style_knob = style;
lv_obj_refresh_ext_draw_pad(slider);
break;
}
}
int16_t lv_slider_get_value(const lv_obj_t * slider)
{
LV_ASSERT_OBJ(slider, LV_OBJX_NAME);
lv_slider_ext_t * ext = lv_obj_get_ext_attr(slider);
if(ext->drag_value != LV_SLIDER_NOT_PRESSED)
return ext->drag_value;
else
return lv_bar_get_value(slider);
}
bool lv_slider_is_dragged(const lv_obj_t * slider)
{
LV_ASSERT_OBJ(slider, LV_OBJX_NAME);
lv_slider_ext_t * ext = lv_obj_get_ext_attr(slider);
return ext->drag_value == LV_SLIDER_NOT_PRESSED ? false : true;
}
bool lv_slider_get_knob_in(const lv_obj_t * slider)
{
LV_ASSERT_OBJ(slider, LV_OBJX_NAME);
lv_slider_ext_t * ext = lv_obj_get_ext_attr(slider);
return ext->knob_in == 0 ? false : true;
}
const lv_style_t * lv_slider_get_style(const lv_obj_t * slider, lv_slider_style_t type)
{
LV_ASSERT_OBJ(slider, LV_OBJX_NAME);
const lv_style_t * style = NULL;
lv_slider_ext_t * ext = lv_obj_get_ext_attr(slider);
switch(type) {
case LV_SLIDER_STYLE_BG: style = lv_bar_get_style(slider, LV_BAR_STYLE_BG); break;
case LV_SLIDER_STYLE_INDIC: style = lv_bar_get_style(slider, LV_BAR_STYLE_INDIC); break;
case LV_SLIDER_STYLE_KNOB: style = ext->style_knob; break;
default: style = NULL; break;
}
return style;
}
static bool lv_slider_design(lv_obj_t * slider, const lv_area_t * mask, lv_design_mode_t mode)
{
if(mode == LV_DESIGN_COVER_CHK) {
return false;
}
else if(mode == LV_DESIGN_DRAW_MAIN) {
lv_slider_ext_t * ext = lv_obj_get_ext_attr(slider);
const lv_style_t * style_bg = lv_slider_get_style(slider, LV_SLIDER_STYLE_BG);
const lv_style_t * style_knob = lv_slider_get_style(slider, LV_SLIDER_STYLE_KNOB);
const lv_style_t * style_indic = lv_slider_get_style(slider, LV_SLIDER_STYLE_INDIC);
lv_opa_t opa_scale = lv_obj_get_opa_scale(slider);
lv_coord_t slider_w = lv_area_get_width(&slider->coords);
lv_coord_t slider_h = lv_area_get_height(&slider->coords);
lv_area_t area_bg;
lv_area_copy(&area_bg, &slider->coords);
lv_coord_t pad_top_bg = style_bg->body.padding.top;
lv_coord_t pad_bottom_bg = style_bg->body.padding.bottom;
lv_coord_t pad_left_bg = style_bg->body.padding.left;
lv_coord_t pad_right_bg = style_bg->body.padding.right;
if(pad_top_bg + pad_bottom_bg + LV_SLIDER_SIZE_MIN > lv_area_get_height(&area_bg)) {
pad_top_bg = (lv_area_get_height(&area_bg) - LV_SLIDER_SIZE_MIN) >> 1;
pad_bottom_bg = pad_top_bg;
}
if(pad_left_bg + pad_right_bg + LV_SLIDER_SIZE_MIN > lv_area_get_width(&area_bg)) {
pad_left_bg = (lv_area_get_width(&area_bg) - LV_SLIDER_SIZE_MIN) >> 1;
pad_right_bg = (lv_area_get_width(&area_bg) - LV_SLIDER_SIZE_MIN) >> 1;
}
if(ext->knob_in) { 
area_bg.x1 += pad_left_bg;
area_bg.x2 -= pad_right_bg;
area_bg.y1 += pad_top_bg;
area_bg.y2 -= pad_bottom_bg;
} else { 
area_bg.x1 += slider_w < slider_h ? pad_left_bg : 0; 
area_bg.x2 -= slider_w < slider_h ? pad_right_bg : 0; 
area_bg.y1 += slider_w > slider_h ? pad_top_bg : 0; 
area_bg.y2 -= slider_w > slider_h ? pad_bottom_bg : 0; 
}
#if LV_USE_GROUP == 0
lv_draw_rect(&area_bg, mask, style_bg, lv_obj_get_opa_scale(slider));
#else
if(lv_obj_is_focused(slider)) {
lv_style_t style_tmp;
lv_style_copy(&style_tmp, style_bg);
style_tmp.body.border.width = 0;
lv_draw_rect(&area_bg, mask, &style_tmp, opa_scale);
} else {
lv_draw_rect(&area_bg, mask, style_bg, opa_scale);
}
#endif
lv_area_t area_indic;
lv_area_copy(&area_indic, &area_bg);
lv_coord_t pad_top_indic = style_indic->body.padding.top;
lv_coord_t pad_bottom_indic = style_indic->body.padding.bottom;
lv_coord_t pad_left_indic = style_indic->body.padding.left;
lv_coord_t pad_right_indic = style_indic->body.padding.right;
if(pad_top_indic + pad_bottom_indic + LV_SLIDER_SIZE_MIN > lv_area_get_height(&area_bg)) {
pad_top_indic = (lv_area_get_height(&area_bg) - LV_SLIDER_SIZE_MIN) >> 1;
pad_bottom_indic = pad_top_indic;
}
if(pad_left_indic + pad_right_indic + LV_SLIDER_SIZE_MIN > lv_area_get_width(&area_bg)) {
pad_left_indic = (lv_area_get_width(&area_bg) - LV_SLIDER_SIZE_MIN) >> 1;
pad_right_indic = pad_left_indic;
}
area_indic.x1 += pad_left_indic;
area_indic.x2 -= pad_right_indic;
area_indic.y1 += pad_top_indic;
area_indic.y2 -= pad_bottom_indic;
lv_coord_t cur_value = lv_slider_get_value(slider);
lv_coord_t min_value = lv_slider_get_min_value(slider);
lv_coord_t max_value = lv_slider_get_max_value(slider);
if(ext->drag_value != LV_SLIDER_NOT_PRESSED) cur_value = ext->drag_value;
bool sym = false;
if(ext->bar.sym && ext->bar.min_value < 0 && ext->bar.max_value > 0) sym = true;
if(slider_w >= slider_h) {
lv_coord_t indic_w = lv_area_get_width(&area_indic);
#if LV_USE_ANIMATION
if(ext->bar.anim_state != LV_BAR_ANIM_STATE_INV) {
lv_coord_t anim_start_x =
(int32_t)((int32_t)indic_w * (ext->bar.anim_start - min_value)) / (max_value - min_value);
lv_coord_t anim_end_x =
(int32_t)((int32_t)indic_w * (ext->bar.anim_end - min_value)) / (max_value - min_value);
area_indic.x2 = anim_start_x + (((anim_end_x - anim_start_x) * ext->bar.anim_state) >> 8);
} else
#endif
{
area_indic.x2 = (int32_t)((int32_t)indic_w * (cur_value - min_value)) / (max_value - min_value);
}
area_indic.x2 = area_indic.x1 + area_indic.x2 - 1;
if(sym) {
lv_coord_t zero;
zero = area_indic.x1 + (-ext->bar.min_value * slider_w) / (ext->bar.max_value - ext->bar.min_value);
if(area_indic.x2 > zero)
area_indic.x1 = zero;
else {
area_indic.x1 = area_indic.x2;
area_indic.x2 = zero;
}
}
if(area_indic.x1 != area_indic.x2) lv_draw_rect(&area_indic, mask, style_indic, opa_scale);
} else {
lv_coord_t indic_h = lv_area_get_height(&area_indic);
#if LV_USE_ANIMATION
if(ext->bar.anim_state != LV_BAR_ANIM_STATE_INV) {
lv_coord_t anim_start_y =
(int32_t)((int32_t)indic_h * (ext->bar.anim_start - min_value)) / (max_value - min_value);
lv_coord_t anim_end_y =
(int32_t)((int32_t)indic_h * (ext->bar.anim_end - min_value)) / (max_value - min_value);
area_indic.y1 = anim_start_y + (((anim_end_y - anim_start_y) * ext->bar.anim_state) >> 8);
} else
#endif
{
area_indic.y1 = (int32_t)((int32_t)indic_h * (cur_value - min_value)) / (max_value - min_value);
}
area_indic.y1 = area_indic.y2 - area_indic.y1 + 1;
if(sym) {
lv_coord_t zero;
zero = area_indic.y2 - (-ext->bar.min_value * slider_h) / (ext->bar.max_value - ext->bar.min_value);
if(area_indic.y1 < zero)
area_indic.y2 = zero;
else {
area_indic.y2 = area_indic.y1;
area_indic.y1 = zero;
}
}
if(area_indic.x1 != area_indic.x2) lv_draw_rect(&area_indic, mask, style_indic, opa_scale);
}
#if LV_USE_GROUP
if(lv_obj_is_focused(slider)) {
lv_style_t style_tmp;
lv_style_copy(&style_tmp, style_bg);
style_tmp.body.opa = LV_OPA_TRANSP;
style_tmp.body.shadow.width = 0;
lv_draw_rect(&area_bg, mask, &style_tmp, opa_scale);
}
#endif
lv_area_t knob_area;
lv_area_copy(&knob_area, &slider->coords);
if(slider_w >= slider_h) {
if(ext->knob_in == 0) {
if(sym == false) {
knob_area.x1 = area_indic.x2 - slider_h / 2;
} else {
if(cur_value > 0) knob_area.x1 = area_indic.x2 - slider_h / 2;
else knob_area.x1 = area_indic.x1 - slider_h / 2;
}
knob_area.x2 = knob_area.x1 + slider_h - 1;
} else {
#if LV_USE_ANIMATION
if(ext->bar.anim_state != LV_BAR_ANIM_STATE_INV) {
lv_coord_t w = slider_w - slider_h - 1;
lv_coord_t anim_start_x =
(int32_t)((int32_t)w * (ext->bar.anim_start - min_value)) / (max_value - min_value);
lv_coord_t anim_end_x =
(int32_t)((int32_t)w * (ext->bar.anim_end - min_value)) / (max_value - min_value);
knob_area.x1 = anim_start_x + (((anim_end_x - anim_start_x) * ext->bar.anim_state) >> 8);
} else
#endif
{
knob_area.x1 = (int32_t)((int32_t)(slider_w - slider_h - 1) * (cur_value - min_value)) /
(max_value - min_value);
}
knob_area.x1 += slider->coords.x1;
knob_area.x2 = knob_area.x1 + slider_h - 1;
}
knob_area.y1 = slider->coords.y1;
knob_area.y2 = slider->coords.y2;
} else {
if(ext->knob_in == 0) {
if(sym == false) {
knob_area.y1 = area_indic.y1 - slider_w / 2;
} else {
if(cur_value > 0) knob_area.y1 = area_indic.y1 - slider_w / 2;
else knob_area.y1 = area_indic.y2 - slider_w / 2;
}
knob_area.y2 = knob_area.y1 + slider_w - 1;
} else {
#if LV_USE_ANIMATION
if(ext->bar.anim_state != LV_BAR_ANIM_STATE_INV) {
lv_coord_t h = slider_h - slider_w - 1;
lv_coord_t anim_start_x =
(int32_t)((int32_t)h * (ext->bar.anim_start - min_value)) / (max_value - min_value);
lv_coord_t anim_end_x =
(int32_t)((int32_t)h * (ext->bar.anim_end - min_value)) / (max_value - min_value);
knob_area.y2 = anim_start_x + (((anim_end_x - anim_start_x) * ext->bar.anim_state) >> 8);
} else
#endif
{
knob_area.y2 = (int32_t)((int32_t)(slider_h - slider_w - 1) * (cur_value - min_value)) /
(max_value - min_value);
}
knob_area.y2 = slider->coords.y2 - knob_area.y2;
knob_area.y1 = knob_area.y2 - slider_w - 1;
}
knob_area.x1 = slider->coords.x1;
knob_area.x2 = slider->coords.x2;
}
lv_draw_rect(&knob_area, mask, style_knob, opa_scale);
}
else if(mode == LV_DESIGN_DRAW_POST) {
}
return true;
}
static lv_res_t lv_slider_signal(lv_obj_t * slider, lv_signal_t sign, void * param)
{
lv_res_t res;
res = ancestor_signal(slider, sign, param);
if(res != LV_RES_OK) return res;
if(sign == LV_SIGNAL_GET_TYPE) return lv_obj_handle_get_type_signal(param, LV_OBJX_NAME);
lv_slider_ext_t * ext = lv_obj_get_ext_attr(slider);
lv_point_t p;
lv_coord_t w = lv_obj_get_width(slider);
lv_coord_t h = lv_obj_get_height(slider);
if(sign == LV_SIGNAL_PRESSED) {
ext->drag_value = lv_slider_get_value(slider);
} else if(sign == LV_SIGNAL_PRESSING) {
lv_indev_get_point(param, &p);
int16_t tmp = 0;
if(w > h) {
lv_coord_t knob_w = h;
p.x -=
slider->coords.x1 + h / 2; 
tmp = (int32_t)((int32_t)p.x * (ext->bar.max_value - ext->bar.min_value + 1)) / (w - knob_w);
tmp += ext->bar.min_value;
} else {
lv_coord_t knob_h = w;
p.y -=
slider->coords.y1 + w / 2; 
tmp = (int32_t)((int32_t)p.y * (ext->bar.max_value - ext->bar.min_value + 1)) / (h - knob_h);
tmp = ext->bar.max_value - tmp; 
}
if(tmp < ext->bar.min_value)
tmp = ext->bar.min_value;
else if(tmp > ext->bar.max_value)
tmp = ext->bar.max_value;
if(tmp != ext->drag_value) {
ext->drag_value = tmp;
lv_obj_invalidate(slider);
res = lv_event_send(slider, LV_EVENT_VALUE_CHANGED, NULL);
if(res != LV_RES_OK) return res;
}
} else if(sign == LV_SIGNAL_RELEASED || sign == LV_SIGNAL_PRESS_LOST) {
if(ext->drag_value != LV_SLIDER_NOT_PRESSED) lv_slider_set_value(slider, ext->drag_value, false);
ext->drag_value = LV_SLIDER_NOT_PRESSED;
#if LV_USE_GROUP
lv_group_t * g = lv_obj_get_group(slider);
bool editing = lv_group_get_editing(g);
lv_indev_type_t indev_type = lv_indev_get_type(lv_indev_get_act());
if(indev_type == LV_INDEV_TYPE_ENCODER) {
if(editing) lv_group_set_editing(g, false);
}
#endif
} else if(sign == LV_SIGNAL_CORD_CHG) {
if(lv_obj_get_width(slider) != lv_area_get_width(param) ||
lv_obj_get_height(slider) != lv_area_get_height(param)) {
slider->signal_cb(slider, LV_SIGNAL_REFR_EXT_DRAW_PAD, NULL);
}
} else if(sign == LV_SIGNAL_REFR_EXT_DRAW_PAD) {
const lv_style_t * style = lv_slider_get_style(slider, LV_SLIDER_STYLE_BG);
const lv_style_t * knob_style = lv_slider_get_style(slider, LV_SLIDER_STYLE_KNOB);
lv_coord_t shadow_w = knob_style->body.shadow.width;
if(ext->knob_in == 0) {
lv_coord_t x = LV_MATH_MIN(w / 2 + 1 + shadow_w, h / 2 + 1 + shadow_w);
if(slider->ext_draw_pad < x) slider->ext_draw_pad = x;
} else {
lv_coord_t pad = 0;
pad = LV_MATH_MIN(pad, style->body.padding.top);
pad = LV_MATH_MIN(pad, style->body.padding.bottom);
pad = LV_MATH_MIN(pad, style->body.padding.left);
pad = LV_MATH_MIN(pad, style->body.padding.right);
if(pad < 0) pad = -pad;
if(slider->ext_draw_pad < pad) slider->ext_draw_pad = pad;
if(slider->ext_draw_pad < shadow_w) slider->ext_draw_pad = shadow_w;
}
} else if(sign == LV_SIGNAL_CONTROL) {
char c = *((char *)param);
ext->drag_value = LV_SLIDER_NOT_PRESSED;
if(c == LV_KEY_RIGHT || c == LV_KEY_UP) {
lv_slider_set_value(slider, lv_slider_get_value(slider) + 1, true);
res = lv_event_send(slider, LV_EVENT_VALUE_CHANGED, NULL);
if(res != LV_RES_OK) return res;
} else if(c == LV_KEY_LEFT || c == LV_KEY_DOWN) {
lv_slider_set_value(slider, lv_slider_get_value(slider) - 1, true);
res = lv_event_send(slider, LV_EVENT_VALUE_CHANGED, NULL);
if(res != LV_RES_OK) return res;
}
} else if(sign == LV_SIGNAL_GET_EDITABLE) {
bool * editable = (bool *)param;
*editable = true;
}
return res;
}
#endif
