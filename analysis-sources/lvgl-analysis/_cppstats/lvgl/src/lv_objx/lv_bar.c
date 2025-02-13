#include "lv_bar.h"
#if LV_USE_BAR != 0
#include "../lv_core/lv_debug.h"
#include "../lv_draw/lv_draw.h"
#include "../lv_themes/lv_theme.h"
#include "../lv_misc/lv_anim.h"
#include <stdio.h>
#define LV_OBJX_NAME "lv_bar"
static bool lv_bar_design(lv_obj_t * bar, const lv_area_t * mask, lv_design_mode_t mode);
static lv_res_t lv_bar_signal(lv_obj_t * bar, lv_signal_t sign, void * param);
#if LV_USE_ANIMATION
static void lv_bar_anim(void * bar, lv_anim_value_t value);
static void lv_bar_anim_ready(lv_anim_t * a);
#endif
static lv_design_cb_t ancestor_design_f;
static lv_signal_cb_t ancestor_signal;
lv_obj_t * lv_bar_create(lv_obj_t * par, const lv_obj_t * copy)
{
LV_LOG_TRACE("lv_bar create started");
lv_obj_t * new_bar = lv_obj_create(par, copy);
LV_ASSERT_MEM(new_bar);
if(new_bar == NULL) return NULL;
if(ancestor_signal == NULL) ancestor_signal = lv_obj_get_signal_cb(new_bar);
if(ancestor_design_f == NULL) ancestor_design_f = lv_obj_get_design_cb(new_bar);
lv_bar_ext_t * ext = lv_obj_allocate_ext_attr(new_bar, sizeof(lv_bar_ext_t));
LV_ASSERT_MEM(ext);
if(ext == NULL) return NULL;
ext->min_value = 0;
ext->max_value = 100;
ext->cur_value = 0;
#if LV_USE_ANIMATION
ext->anim_time = 200;
ext->anim_start = 0;
ext->anim_end = 0;
ext->anim_state = LV_BAR_ANIM_STATE_INV;
#endif
ext->sym = 0;
ext->style_indic = &lv_style_pretty_color;
lv_obj_set_signal_cb(new_bar, lv_bar_signal);
lv_obj_set_design_cb(new_bar, lv_bar_design);
if(copy == NULL) {
lv_obj_set_click(new_bar, false);
lv_obj_set_size(new_bar, LV_DPI * 2, LV_DPI / 3);
lv_bar_set_value(new_bar, ext->cur_value, false);
lv_theme_t * th = lv_theme_get_current();
if(th) {
lv_bar_set_style(new_bar, LV_BAR_STYLE_BG, th->style.bar.bg);
lv_bar_set_style(new_bar, LV_BAR_STYLE_INDIC, th->style.bar.indic);
} else {
lv_obj_set_style(new_bar, &lv_style_pretty);
}
} else {
lv_bar_ext_t * ext_copy = lv_obj_get_ext_attr(copy);
ext->min_value = ext_copy->min_value;
ext->max_value = ext_copy->max_value;
ext->cur_value = ext_copy->cur_value;
ext->style_indic = ext_copy->style_indic;
ext->sym = ext_copy->sym;
lv_obj_refresh_style(new_bar);
lv_bar_set_value(new_bar, ext->cur_value, false);
}
LV_LOG_INFO("bar created");
return new_bar;
}
void lv_bar_set_value(lv_obj_t * bar, int16_t value, lv_anim_enable_t anim)
{
LV_ASSERT_OBJ(bar, LV_OBJX_NAME);
#if LV_USE_ANIMATION == 0
anim = false;
#endif
lv_bar_ext_t * ext = lv_obj_get_ext_attr(bar);
if(ext->cur_value == value) return;
int16_t new_value;
new_value = value > ext->max_value ? ext->max_value : value;
new_value = new_value < ext->min_value ? ext->min_value : new_value;
if(ext->cur_value == new_value) return;
if(anim == LV_ANIM_OFF) {
ext->cur_value = new_value;
lv_obj_invalidate(bar);
} else {
#if LV_USE_ANIMATION
if(ext->anim_state == LV_BAR_ANIM_STATE_INV) {
ext->anim_start = ext->cur_value;
ext->anim_end = new_value;
}
else {
ext->anim_start = ext->anim_end;
ext->anim_end = new_value;
}
lv_anim_t a;
a.var = bar;
a.start = LV_BAR_ANIM_STATE_START;
a.end = LV_BAR_ANIM_STATE_END;
a.exec_cb = (lv_anim_exec_xcb_t)lv_bar_anim;
a.path_cb = lv_anim_path_linear;
a.ready_cb = lv_bar_anim_ready;
a.act_time = 0;
a.time = ext->anim_time;
a.playback = 0;
a.playback_pause = 0;
a.repeat = 0;
a.repeat_pause = 0;
lv_anim_create(&a);
#endif
}
}
void lv_bar_set_range(lv_obj_t * bar, int16_t min, int16_t max)
{
LV_ASSERT_OBJ(bar, LV_OBJX_NAME);
lv_bar_ext_t * ext = lv_obj_get_ext_attr(bar);
if(ext->min_value == min && ext->max_value == max) return;
ext->max_value = max;
ext->min_value = min;
if(ext->cur_value > max) {
ext->cur_value = max;
lv_bar_set_value(bar, ext->cur_value, false);
}
if(ext->cur_value < min) {
ext->cur_value = min;
lv_bar_set_value(bar, ext->cur_value, false);
}
lv_obj_invalidate(bar);
}
void lv_bar_set_sym(lv_obj_t * bar, bool en)
{
LV_ASSERT_OBJ(bar, LV_OBJX_NAME);
lv_bar_ext_t * ext = lv_obj_get_ext_attr(bar);
ext->sym = en ? 1 : 0;
}
void lv_bar_set_anim_time(lv_obj_t * bar, uint16_t anim_time)
{
LV_ASSERT_OBJ(bar, LV_OBJX_NAME);
#if LV_USE_ANIMATION
lv_bar_ext_t * ext = lv_obj_get_ext_attr(bar);
ext->anim_time = anim_time;
#else
(void)bar; 
(void)anim_time; 
#endif
}
void lv_bar_set_style(lv_obj_t * bar, lv_bar_style_t type, const lv_style_t * style)
{
LV_ASSERT_OBJ(bar, LV_OBJX_NAME);
lv_bar_ext_t * ext = lv_obj_get_ext_attr(bar);
switch(type) {
case LV_BAR_STYLE_BG: lv_obj_set_style(bar, style); break;
case LV_BAR_STYLE_INDIC:
ext->style_indic = style;
lv_obj_refresh_ext_draw_pad(bar);
break;
}
}
int16_t lv_bar_get_value(const lv_obj_t * bar)
{
LV_ASSERT_OBJ(bar, LV_OBJX_NAME);
lv_bar_ext_t * ext = lv_obj_get_ext_attr(bar);
#if LV_USE_ANIMATION
if(ext->anim_state != LV_BAR_ANIM_STATE_INV) return ext->anim_end;
#endif
return ext->cur_value;
}
int16_t lv_bar_get_min_value(const lv_obj_t * bar)
{
LV_ASSERT_OBJ(bar, LV_OBJX_NAME);
lv_bar_ext_t * ext = lv_obj_get_ext_attr(bar);
return ext->min_value;
}
int16_t lv_bar_get_max_value(const lv_obj_t * bar)
{
LV_ASSERT_OBJ(bar, LV_OBJX_NAME);
lv_bar_ext_t * ext = lv_obj_get_ext_attr(bar);
return ext->max_value;
}
bool lv_bar_get_sym(lv_obj_t * bar)
{
LV_ASSERT_OBJ(bar, LV_OBJX_NAME);
lv_bar_ext_t * ext = lv_obj_get_ext_attr(bar);
return ext->sym ? true : false;
}
uint16_t lv_bar_get_anim_time(lv_obj_t * bar)
{
LV_ASSERT_OBJ(bar, LV_OBJX_NAME);
#if LV_USE_ANIMATION
lv_bar_ext_t * ext = lv_obj_get_ext_attr(bar);
return ext->anim_time;
#else
(void)bar; 
return 0;
#endif
}
const lv_style_t * lv_bar_get_style(const lv_obj_t * bar, lv_bar_style_t type)
{
LV_ASSERT_OBJ(bar, LV_OBJX_NAME);
const lv_style_t * style = NULL;
lv_bar_ext_t * ext = lv_obj_get_ext_attr(bar);
switch(type) {
case LV_BAR_STYLE_BG: style = lv_obj_get_style(bar); break;
case LV_BAR_STYLE_INDIC: style = ext->style_indic; break;
default: style = NULL; break;
}
return style;
}
static bool lv_bar_design(lv_obj_t * bar, const lv_area_t * mask, lv_design_mode_t mode)
{
if(mode == LV_DESIGN_COVER_CHK) {
return ancestor_design_f(bar, mask, mode);
} else if(mode == LV_DESIGN_DRAW_MAIN) {
lv_opa_t opa_scale = lv_obj_get_opa_scale(bar);
#if LV_USE_GROUP == 0
ancestor_design_f(bar, mask, mode);
#else
if(lv_obj_is_focused(bar)) {
const lv_style_t * style_bg = lv_bar_get_style(bar, LV_BAR_STYLE_BG);
lv_style_t style_tmp;
lv_style_copy(&style_tmp, style_bg);
style_tmp.body.border.width = 0;
lv_draw_rect(&bar->coords, mask, &style_tmp, opa_scale);
} else {
ancestor_design_f(bar, mask, mode);
}
#endif
lv_bar_ext_t * ext = lv_obj_get_ext_attr(bar);
if(ext->cur_value != ext->min_value || ext->sym
#if LV_USE_ANIMATION
|| ext->anim_start != LV_BAR_ANIM_STATE_INV
#endif
) {
const lv_style_t * style_indic = lv_bar_get_style(bar, LV_BAR_STYLE_INDIC);
lv_area_t indic_area;
lv_area_copy(&indic_area, &bar->coords);
indic_area.x1 += style_indic->body.padding.left;
indic_area.x2 -= style_indic->body.padding.right;
indic_area.y1 += style_indic->body.padding.top;
indic_area.y2 -= style_indic->body.padding.bottom;
lv_coord_t w = lv_area_get_width(&indic_area);
lv_coord_t h = lv_area_get_height(&indic_area);
if(w >= h) {
#if LV_USE_ANIMATION
if(ext->anim_state != LV_BAR_ANIM_STATE_INV) {
lv_coord_t anim_start_x =
(int32_t)((int32_t)w * (ext->anim_start - ext->min_value)) / (ext->max_value - ext->min_value);
lv_coord_t anim_end_x =
(int32_t)((int32_t)w * (ext->anim_end - ext->min_value)) / (ext->max_value - ext->min_value);
indic_area.x2 =
anim_start_x + (((anim_end_x - anim_start_x) * ext->anim_state) >> LV_BAR_ANIM_STATE_NORM);
} else
#endif
{
indic_area.x2 =
(int32_t)((int32_t)w * (ext->cur_value - ext->min_value)) / (ext->max_value - ext->min_value);
}
indic_area.x2 = indic_area.x1 + indic_area.x2 - 1;
if(ext->sym && ext->min_value < 0 && ext->max_value > 0) {
lv_coord_t zero;
zero = indic_area.x1 + (-ext->min_value * w) / (ext->max_value - ext->min_value);
if(indic_area.x2 > zero)
indic_area.x1 = zero;
else {
indic_area.x1 = indic_area.x2;
indic_area.x2 = zero;
}
}
} else {
#if LV_USE_ANIMATION
if(ext->anim_state != LV_BAR_ANIM_STATE_INV) {
lv_coord_t anim_start_y =
(int32_t)((int32_t)h * (ext->anim_start - ext->min_value)) / (ext->max_value - ext->min_value);
lv_coord_t anim_end_y =
(int32_t)((int32_t)h * (ext->anim_end - ext->min_value)) / (ext->max_value - ext->min_value);
indic_area.y1 =
anim_start_y + (((anim_end_y - anim_start_y) * ext->anim_state) >> LV_BAR_ANIM_STATE_NORM);
} else
#endif
{
indic_area.y1 =
(int32_t)((int32_t)h * (ext->cur_value - ext->min_value)) / (ext->max_value - ext->min_value);
}
indic_area.y1 = indic_area.y2 - indic_area.y1 + 1;
if(ext->sym && ext->min_value < 0 && ext->max_value > 0) {
lv_coord_t zero;
zero = indic_area.y2 - (-ext->min_value * h) / (ext->max_value - ext->min_value);
if(indic_area.y1 < zero)
indic_area.y2 = zero;
else {
indic_area.y2 = indic_area.y1;
indic_area.y1 = zero;
}
}
}
lv_draw_rect(&indic_area, mask, style_indic, opa_scale);
}
} else if(mode == LV_DESIGN_DRAW_POST) {
#if LV_USE_GROUP
if(lv_obj_is_focused(bar)) {
lv_opa_t opa_scale = lv_obj_get_opa_scale(bar);
const lv_style_t * style_bg = lv_bar_get_style(bar, LV_BAR_STYLE_BG);
lv_style_t style_tmp;
lv_style_copy(&style_tmp, style_bg);
style_tmp.body.opa = LV_OPA_TRANSP;
style_tmp.body.shadow.width = 0;
lv_draw_rect(&bar->coords, mask, &style_tmp, opa_scale);
}
#endif
}
return true;
}
static lv_res_t lv_bar_signal(lv_obj_t * bar, lv_signal_t sign, void * param)
{
lv_res_t res;
res = ancestor_signal(bar, sign, param);
if(res != LV_RES_OK) return res;
if(sign == LV_SIGNAL_GET_TYPE) return lv_obj_handle_get_type_signal(param, LV_OBJX_NAME);
if(sign == LV_SIGNAL_REFR_EXT_DRAW_PAD) {
const lv_style_t * style_indic = lv_bar_get_style(bar, LV_BAR_STYLE_INDIC);
if(style_indic->body.shadow.width > bar->ext_draw_pad) bar->ext_draw_pad = style_indic->body.shadow.width;
}
return res;
}
#if LV_USE_ANIMATION
static void lv_bar_anim(void * bar, lv_anim_value_t value)
{
lv_bar_ext_t * ext = lv_obj_get_ext_attr(bar);
ext->anim_state = value;
lv_obj_invalidate(bar);
}
static void lv_bar_anim_ready(lv_anim_t * a)
{
lv_bar_ext_t * ext = lv_obj_get_ext_attr(a->var);
ext->anim_state = LV_BAR_ANIM_STATE_INV;
lv_bar_set_value(a->var, ext->anim_end, false);
}
#endif
#endif
