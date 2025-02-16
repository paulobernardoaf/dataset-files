#include "lv_gauge.h"
#if LV_USE_GAUGE != 0
#include "../lv_core/lv_debug.h"
#include "../lv_draw/lv_draw.h"
#include "../lv_themes/lv_theme.h"
#include "../lv_misc/lv_txt.h"
#include "../lv_misc/lv_math.h"
#include "../lv_misc/lv_utils.h"
#include <stdio.h>
#include <string.h>
#define LV_OBJX_NAME "lv_gauge"
#define LV_GAUGE_DEF_NEEDLE_COLOR LV_COLOR_RED
#define LV_GAUGE_DEF_LABEL_COUNT 6
#define LV_GAUGE_DEF_LINE_COUNT 21 
#define LV_GAUGE_DEF_ANGLE 220
#define LV_GAUGE_INTERPOLATE_SHIFT 5 
#define LV_GAUGE_INTERPOLATE_MASK 0x1F
static bool lv_gauge_design(lv_obj_t * gauge, const lv_area_t * mask, lv_design_mode_t mode);
static lv_res_t lv_gauge_signal(lv_obj_t * gauge, lv_signal_t sign, void * param);
static void lv_gauge_draw_scale(lv_obj_t * gauge, const lv_area_t * mask);
static void lv_gauge_draw_needle(lv_obj_t * gauge, const lv_area_t * mask);
static lv_design_cb_t ancestor_design;
static lv_signal_cb_t ancestor_signal;
lv_obj_t * lv_gauge_create(lv_obj_t * par, const lv_obj_t * copy)
{
LV_LOG_TRACE("gauge create started");
lv_obj_t * new_gauge = lv_lmeter_create(par, copy);
LV_ASSERT_MEM(new_gauge);
if(new_gauge == NULL) return NULL;
lv_gauge_ext_t * ext = lv_obj_allocate_ext_attr(new_gauge, sizeof(lv_gauge_ext_t));
LV_ASSERT_MEM(ext);
if(ext == NULL) return NULL;
ext->needle_count = 0;
ext->values = NULL;
ext->needle_colors = NULL;
ext->label_count = LV_GAUGE_DEF_LABEL_COUNT;
if(ancestor_signal == NULL) ancestor_signal = lv_obj_get_signal_cb(new_gauge);
if(ancestor_design == NULL) ancestor_design = lv_obj_get_design_cb(new_gauge);
lv_obj_set_signal_cb(new_gauge, lv_gauge_signal);
lv_obj_set_design_cb(new_gauge, lv_gauge_design);
if(copy == NULL) {
lv_gauge_set_scale(new_gauge, LV_GAUGE_DEF_ANGLE, LV_GAUGE_DEF_LINE_COUNT, LV_GAUGE_DEF_LABEL_COUNT);
lv_gauge_set_needle_count(new_gauge, 1, NULL);
lv_gauge_set_critical_value(new_gauge, 80);
lv_obj_set_size(new_gauge, 2 * LV_DPI, 2 * LV_DPI);
lv_theme_t * th = lv_theme_get_current();
if(th) {
lv_gauge_set_style(new_gauge, LV_GAUGE_STYLE_MAIN, th->style.gauge);
} else {
lv_gauge_set_style(new_gauge, LV_GAUGE_STYLE_MAIN, &lv_style_pretty_color);
}
}
else {
lv_gauge_ext_t * copy_ext = lv_obj_get_ext_attr(copy);
lv_gauge_set_needle_count(new_gauge, copy_ext->needle_count, copy_ext->needle_colors);
uint8_t i;
for(i = 0; i < ext->needle_count; i++) {
ext->values[i] = copy_ext->values[i];
}
ext->label_count = copy_ext->label_count;
lv_obj_refresh_style(new_gauge);
}
LV_LOG_INFO("gauge created");
return new_gauge;
}
void lv_gauge_set_needle_count(lv_obj_t * gauge, uint8_t needle_cnt, const lv_color_t colors[])
{
LV_ASSERT_OBJ(gauge, LV_OBJX_NAME);
lv_gauge_ext_t * ext = lv_obj_get_ext_attr(gauge);
if(ext->needle_count != needle_cnt) {
if(ext->values != NULL) {
lv_mem_free(ext->values);
ext->values = NULL;
}
ext->values = lv_mem_realloc(ext->values, needle_cnt * sizeof(int16_t));
LV_ASSERT_MEM(ext->values);
if(ext->values == NULL) return;
int16_t min = lv_gauge_get_min_value(gauge);
uint8_t n;
for(n = ext->needle_count; n < needle_cnt; n++) {
ext->values[n] = min;
}
ext->needle_count = needle_cnt;
}
ext->needle_colors = colors;
lv_obj_invalidate(gauge);
}
void lv_gauge_set_value(lv_obj_t * gauge, uint8_t needle_id, int16_t value)
{
LV_ASSERT_OBJ(gauge, LV_OBJX_NAME);
lv_gauge_ext_t * ext = lv_obj_get_ext_attr(gauge);
if(needle_id >= ext->needle_count) return;
if(ext->values[needle_id] == value) return;
int16_t min = lv_gauge_get_min_value(gauge);
int16_t max = lv_gauge_get_max_value(gauge);
if(value > max)
value = max;
else if(value < min)
value = min;
ext->values[needle_id] = value;
lv_obj_invalidate(gauge);
}
void lv_gauge_set_scale(lv_obj_t * gauge, uint16_t angle, uint8_t line_cnt, uint8_t label_cnt)
{
LV_ASSERT_OBJ(gauge, LV_OBJX_NAME);
lv_lmeter_set_scale(gauge, angle, line_cnt);
lv_gauge_ext_t * ext = lv_obj_get_ext_attr(gauge);
ext->label_count = label_cnt;
lv_obj_invalidate(gauge);
}
int16_t lv_gauge_get_value(const lv_obj_t * gauge, uint8_t needle)
{
LV_ASSERT_OBJ(gauge, LV_OBJX_NAME);
lv_gauge_ext_t * ext = lv_obj_get_ext_attr(gauge);
int16_t min = lv_gauge_get_min_value(gauge);
if(needle >= ext->needle_count) return min;
return ext->values[needle];
}
uint8_t lv_gauge_get_needle_count(const lv_obj_t * gauge)
{
LV_ASSERT_OBJ(gauge, LV_OBJX_NAME);
lv_gauge_ext_t * ext = lv_obj_get_ext_attr(gauge);
return ext->needle_count;
}
uint8_t lv_gauge_get_label_count(const lv_obj_t * gauge)
{
LV_ASSERT_OBJ(gauge, LV_OBJX_NAME);
lv_gauge_ext_t * ext = lv_obj_get_ext_attr(gauge);
return ext->label_count;
}
static bool lv_gauge_design(lv_obj_t * gauge, const lv_area_t * mask, lv_design_mode_t mode)
{
if(mode == LV_DESIGN_COVER_CHK) {
return false;
}
else if(mode == LV_DESIGN_DRAW_MAIN) {
const lv_style_t * style_ori_p = gauge->style_p;
const lv_style_t * style = lv_obj_get_style(gauge);
lv_gauge_ext_t * ext = lv_obj_get_ext_attr(gauge);
lv_gauge_draw_scale(gauge, mask);
uint16_t line_cnt_tmp = ext->lmeter.line_cnt;
ancestor_design(gauge, mask, mode); 
lv_style_t style_tmp;
lv_style_copy(&style_tmp, style);
ext->lmeter.line_cnt = ext->label_count; 
style_tmp.body.padding.left = style_tmp.body.padding.left * 2; 
style_tmp.body.padding.right = style_tmp.body.padding.right * 2; 
gauge->style_p = &style_tmp;
ancestor_design(gauge, mask, mode); 
ext->lmeter.line_cnt = line_cnt_tmp; 
gauge->style_p = style_ori_p; 
lv_gauge_draw_needle(gauge, mask);
}
else if(mode == LV_DESIGN_DRAW_POST) {
ancestor_design(gauge, mask, mode);
}
return true;
}
static lv_res_t lv_gauge_signal(lv_obj_t * gauge, lv_signal_t sign, void * param)
{
lv_res_t res;
res = ancestor_signal(gauge, sign, param);
if(res != LV_RES_OK) return res;
if(sign == LV_SIGNAL_GET_TYPE) return lv_obj_handle_get_type_signal(param, LV_OBJX_NAME);
lv_gauge_ext_t * ext = lv_obj_get_ext_attr(gauge);
if(sign == LV_SIGNAL_CLEANUP) {
lv_mem_free(ext->values);
ext->values = NULL;
}
return res;
}
static void lv_gauge_draw_scale(lv_obj_t * gauge, const lv_area_t * mask)
{
char scale_txt[16];
lv_gauge_ext_t * ext = lv_obj_get_ext_attr(gauge);
const lv_style_t * style = lv_obj_get_style(gauge);
lv_opa_t opa_scale = lv_obj_get_opa_scale(gauge);
lv_coord_t r = lv_obj_get_width(gauge) / 2 - (3 * style->body.padding.left) - style->body.padding.inner;
lv_coord_t x_ofs = lv_obj_get_width(gauge) / 2 + gauge->coords.x1;
lv_coord_t y_ofs = lv_obj_get_height(gauge) / 2 + gauge->coords.y1;
int16_t scale_angle = lv_lmeter_get_scale_angle(gauge);
uint16_t label_num = ext->label_count;
int16_t angle_ofs = 90 + (360 - scale_angle) / 2;
int16_t min = lv_gauge_get_min_value(gauge);
int16_t max = lv_gauge_get_max_value(gauge);
uint8_t i;
for(i = 0; i < label_num; i++) {
int16_t angle = (i * scale_angle) / (label_num - 1) + angle_ofs;
lv_coord_t y = (int32_t)((int32_t)lv_trigo_sin(angle) * r) / LV_TRIGO_SIN_MAX;
y += y_ofs;
lv_coord_t x = (int32_t)((int32_t)lv_trigo_sin(angle + 90) * r) / LV_TRIGO_SIN_MAX;
x += x_ofs;
int16_t scale_act = (int32_t)((int32_t)(max - min) * i) / (label_num - 1);
scale_act += min;
lv_utils_num_to_str(scale_act, scale_txt);
lv_area_t label_cord;
lv_point_t label_size;
lv_txt_get_size(&label_size, scale_txt, style->text.font, style->text.letter_space, style->text.line_space,
LV_COORD_MAX, LV_TXT_FLAG_NONE);
label_cord.x1 = x - label_size.x / 2;
label_cord.y1 = y - label_size.y / 2;
label_cord.x2 = label_cord.x1 + label_size.x;
label_cord.y2 = label_cord.y1 + label_size.y;
lv_draw_label(&label_cord, mask, style, opa_scale, scale_txt, LV_TXT_FLAG_NONE, NULL, NULL, NULL, lv_obj_get_base_dir(gauge));
}
}
static void lv_gauge_draw_needle(lv_obj_t * gauge, const lv_area_t * mask)
{
lv_style_t style_needle;
lv_gauge_ext_t * ext = lv_obj_get_ext_attr(gauge);
const lv_style_t * style = lv_gauge_get_style(gauge, LV_GAUGE_STYLE_MAIN);
lv_opa_t opa_scale = lv_obj_get_opa_scale(gauge);
lv_coord_t r = lv_obj_get_width(gauge) / 2 - style->body.padding.left;
lv_coord_t x_ofs = lv_obj_get_width(gauge) / 2 + gauge->coords.x1;
lv_coord_t y_ofs = lv_obj_get_height(gauge) / 2 + gauge->coords.y1;
uint16_t angle = lv_lmeter_get_scale_angle(gauge);
int16_t angle_ofs = 90 + (360 - angle) / 2;
int16_t min = lv_gauge_get_min_value(gauge);
int16_t max = lv_gauge_get_max_value(gauge);
lv_point_t p_mid;
lv_point_t p_end;
lv_point_t p_end_low;
lv_point_t p_end_high;
uint8_t i;
lv_style_copy(&style_needle, style);
p_mid.x = x_ofs;
p_mid.y = y_ofs;
for(i = 0; i < ext->needle_count; i++) {
int16_t needle_angle =
(ext->values[i] - min) * angle * (1 << LV_GAUGE_INTERPOLATE_SHIFT) / (max - min);
int16_t needle_angle_low = (needle_angle >> LV_GAUGE_INTERPOLATE_SHIFT) + angle_ofs;
int16_t needle_angle_high = needle_angle_low + 1;
p_end_low.y = (lv_trigo_sin(needle_angle_low) * r) / LV_TRIGO_SIN_MAX + y_ofs;
p_end_low.x = (lv_trigo_sin(needle_angle_low + 90) * r) / LV_TRIGO_SIN_MAX + x_ofs;
p_end_high.y = (lv_trigo_sin(needle_angle_high) * r) / LV_TRIGO_SIN_MAX + y_ofs;
p_end_high.x = (lv_trigo_sin(needle_angle_high + 90) * r) / LV_TRIGO_SIN_MAX + x_ofs;
uint16_t rem = needle_angle & ((1 << LV_GAUGE_INTERPOLATE_SHIFT) - 1);
int16_t x_mod = ((LV_MATH_ABS(p_end_high.x - p_end_low.x)) * rem) >> LV_GAUGE_INTERPOLATE_SHIFT;
int16_t y_mod = ((LV_MATH_ABS(p_end_high.y - p_end_low.y)) * rem) >> LV_GAUGE_INTERPOLATE_SHIFT;
if(p_end_high.x < p_end_low.x) x_mod = -x_mod;
if(p_end_high.y < p_end_low.y) y_mod = -y_mod;
p_end.x = p_end_low.x + x_mod;
p_end.y = p_end_low.y + y_mod;
if(ext->needle_colors == NULL)
style_needle.line.color = LV_GAUGE_DEF_NEEDLE_COLOR;
else
style_needle.line.color = ext->needle_colors[i];
lv_draw_line(&p_mid, &p_end, mask, &style_needle, opa_scale);
}
lv_style_t style_neddle_mid;
lv_style_copy(&style_neddle_mid, &lv_style_plain);
style_neddle_mid.body.main_color = style->body.border.color;
style_neddle_mid.body.grad_color = style->body.border.color;
style_neddle_mid.body.radius = LV_RADIUS_CIRCLE;
lv_area_t nm_cord;
nm_cord.x1 = x_ofs - style->body.radius;
nm_cord.y1 = y_ofs - style->body.radius;
nm_cord.x2 = x_ofs + style->body.radius;
nm_cord.y2 = y_ofs + style->body.radius;
lv_draw_rect(&nm_cord, mask, &style_neddle_mid, lv_obj_get_opa_scale(gauge));
}
#endif
