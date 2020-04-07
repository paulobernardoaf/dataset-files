#include "lv_spinbox.h"
#if LV_USE_SPINBOX != 0
#include "../lv_core/lv_debug.h"
#include "../lv_themes/lv_theme.h"
#include "../lv_misc/lv_math.h"
#include "../lv_misc/lv_utils.h"
#define LV_OBJX_NAME "lv_spinbox"
static lv_res_t lv_spinbox_signal(lv_obj_t * spinbox, lv_signal_t sign, void * param);
static void lv_spinbox_updatevalue(lv_obj_t * spinbox);
static lv_signal_cb_t ancestor_signal;
static lv_design_cb_t ancestor_design;
lv_obj_t * lv_spinbox_create(lv_obj_t * par, const lv_obj_t * copy)
{
LV_LOG_TRACE("spinbox create started");
lv_obj_t * new_spinbox = lv_ta_create(par, copy);
LV_ASSERT_MEM(new_spinbox);
if(new_spinbox == NULL) return NULL;
lv_spinbox_ext_t * ext = lv_obj_allocate_ext_attr(new_spinbox, sizeof(lv_spinbox_ext_t));
LV_ASSERT_MEM(ext);
if(ext == NULL) return NULL;
if(ancestor_signal == NULL) ancestor_signal = lv_obj_get_signal_cb(new_spinbox);
if(ancestor_design == NULL) ancestor_design = lv_obj_get_design_cb(new_spinbox);
ext->value = 0;
ext->dec_point_pos = 0;
ext->digit_count = 5;
ext->digit_padding_left = 0;
ext->step = 1;
ext->range_max = 99999;
ext->range_min = -99999;
lv_ta_set_cursor_type(new_spinbox, LV_CURSOR_BLOCK);
lv_ta_set_one_line(new_spinbox, true);
lv_ta_set_cursor_click_pos(new_spinbox, false);
lv_obj_set_signal_cb(new_spinbox, lv_spinbox_signal);
lv_obj_set_design_cb(new_spinbox, ancestor_design); 
if(copy == NULL) {
lv_theme_t * th = lv_theme_get_current();
if(th) {
lv_spinbox_set_style(new_spinbox, LV_SPINBOX_STYLE_BG, th->style.spinbox.bg);
lv_spinbox_set_style(new_spinbox, LV_SPINBOX_STYLE_CURSOR, th->style.spinbox.cursor);
lv_spinbox_set_style(new_spinbox, LV_SPINBOX_STYLE_SB, th->style.spinbox.sb);
}
}
else {
lv_spinbox_ext_t * copy_ext = lv_obj_get_ext_attr(copy);
lv_spinbox_set_value(new_spinbox, copy_ext->value);
lv_spinbox_set_digit_format(new_spinbox, (uint8_t)copy_ext->digit_count, (uint8_t)copy_ext->dec_point_pos);
lv_spinbox_set_range(new_spinbox, copy_ext->range_min, copy_ext->range_max);
lv_spinbox_set_step(new_spinbox, copy_ext->step);
lv_obj_refresh_style(new_spinbox);
}
lv_spinbox_updatevalue(new_spinbox);
LV_LOG_INFO("spinbox created");
return new_spinbox;
}
void lv_spinbox_set_value(lv_obj_t * spinbox, int32_t i)
{
LV_ASSERT_OBJ(spinbox, LV_OBJX_NAME);
lv_spinbox_ext_t * ext = lv_obj_get_ext_attr(spinbox);
if(ext == NULL) return;
if(i > ext->range_max) i = ext->range_max;
if(i < ext->range_min) i = ext->range_min;
ext->value = i;
lv_spinbox_updatevalue(spinbox);
}
void lv_spinbox_set_digit_format(lv_obj_t * spinbox, uint8_t digit_count, uint8_t separator_position)
{
LV_ASSERT_OBJ(spinbox, LV_OBJX_NAME);
lv_spinbox_ext_t * ext = lv_obj_get_ext_attr(spinbox);
if(ext == NULL) return;
if(digit_count > LV_SPINBOX_MAX_DIGIT_COUNT) digit_count = LV_SPINBOX_MAX_DIGIT_COUNT;
if(separator_position > LV_SPINBOX_MAX_DIGIT_COUNT) separator_position = LV_SPINBOX_MAX_DIGIT_COUNT;
ext->digit_count = digit_count;
ext->dec_point_pos = separator_position;
lv_spinbox_updatevalue(spinbox);
}
void lv_spinbox_set_step(lv_obj_t * spinbox, uint32_t step)
{
LV_ASSERT_OBJ(spinbox, LV_OBJX_NAME);
lv_spinbox_ext_t * ext = lv_obj_get_ext_attr(spinbox);
if(ext == NULL) return;
ext->step = step;
}
void lv_spinbox_set_range(lv_obj_t * spinbox, int32_t range_min, int32_t range_max)
{
LV_ASSERT_OBJ(spinbox, LV_OBJX_NAME);
lv_spinbox_ext_t * ext = lv_obj_get_ext_attr(spinbox);
if(ext == NULL) return;
ext->range_max = range_max;
ext->range_min = range_min;
if(ext->value > ext->range_max) {
ext->value = ext->range_max;
lv_obj_invalidate(spinbox);
}
if(ext->value < ext->range_min) {
ext->value = ext->range_min;
lv_obj_invalidate(spinbox);
}
}
void lv_spinbox_set_padding_left(lv_obj_t * spinbox, uint8_t padding)
{
LV_ASSERT_OBJ(spinbox, LV_OBJX_NAME);
lv_spinbox_ext_t * ext = lv_obj_get_ext_attr(spinbox);
ext->digit_padding_left = padding;
lv_spinbox_updatevalue(spinbox);
}
int32_t lv_spinbox_get_value(lv_obj_t * spinbox)
{
LV_ASSERT_OBJ(spinbox, LV_OBJX_NAME);
lv_spinbox_ext_t * ext = lv_obj_get_ext_attr(spinbox);
return ext->value;
}
void lv_spinbox_step_next(lv_obj_t * spinbox)
{
LV_ASSERT_OBJ(spinbox, LV_OBJX_NAME);
lv_spinbox_ext_t * ext = lv_obj_get_ext_attr(spinbox);
int32_t new_step = ext->step / 10;
if((new_step) > 0)
ext->step = new_step;
else
ext->step = 1;
lv_spinbox_updatevalue(spinbox);
}
void lv_spinbox_step_prev(lv_obj_t * spinbox)
{
LV_ASSERT_OBJ(spinbox, LV_OBJX_NAME);
lv_spinbox_ext_t * ext = lv_obj_get_ext_attr(spinbox);
int32_t step_limit;
step_limit = LV_MATH_MAX(ext->range_max, (ext->range_min < 0 ? (-ext->range_min) : ext->range_min));
int32_t new_step = ext->step * 10;
if(new_step <= step_limit) ext->step = new_step;
lv_spinbox_updatevalue(spinbox);
}
void lv_spinbox_increment(lv_obj_t * spinbox)
{
LV_ASSERT_OBJ(spinbox, LV_OBJX_NAME);
lv_spinbox_ext_t * ext = lv_obj_get_ext_attr(spinbox);
if(ext->value + ext->step <= ext->range_max) {
if((ext->value + ext->step) > 0 && ext->value < 0) ext->value = -ext->value;
ext->value += ext->step;
} else {
ext->value = ext->range_max;
}
lv_spinbox_updatevalue(spinbox);
}
void lv_spinbox_decrement(lv_obj_t * spinbox)
{
LV_ASSERT_OBJ(spinbox, LV_OBJX_NAME);
lv_spinbox_ext_t * ext = lv_obj_get_ext_attr(spinbox);
if(ext->value - ext->step >= ext->range_min) {
if((ext->value - ext->step) < 0 && ext->value > 0) ext->value = -ext->value;
ext->value -= ext->step;
} else {
ext->value = ext->range_min;
}
lv_spinbox_updatevalue(spinbox);
}
static lv_res_t lv_spinbox_signal(lv_obj_t * spinbox, lv_signal_t sign, void * param)
{
lv_res_t res = LV_RES_OK;
if(sign != LV_SIGNAL_CONTROL) {
res = ancestor_signal(spinbox, sign, param);
if(res != LV_RES_OK) return res;
}
if(sign == LV_SIGNAL_GET_TYPE) return lv_obj_handle_get_type_signal(param, LV_OBJX_NAME);
lv_spinbox_ext_t * ext = lv_obj_get_ext_attr(spinbox);
if(sign == LV_SIGNAL_CLEANUP) {
} else if(sign == LV_SIGNAL_GET_TYPE) {
lv_obj_type_t * buf = param;
uint8_t i;
for(i = 0; i < LV_MAX_ANCESTOR_NUM - 1; i++) { 
if(buf->type[i] == NULL) break;
}
buf->type[i] = "lv_spinbox";
} else if(sign == LV_SIGNAL_RELEASED) {
#if LV_USE_GROUP
lv_indev_t * indev = lv_indev_get_act();
if(lv_indev_get_type(indev) == LV_INDEV_TYPE_ENCODER) {
if(lv_group_get_editing(lv_obj_get_group(spinbox))) {
if(ext->step > 1) {
lv_spinbox_step_next(spinbox);
} else {
ext->step = 1;
uint32_t i;
for(i = 0; i < ext->digit_count; i++) {
int32_t new_step = ext->step * 10;
if(new_step >= ext->range_max) break;
ext->step = new_step;
}
lv_spinbox_step_prev(spinbox);
}
}
}
#endif
} else if(sign == LV_SIGNAL_CONTROL) {
lv_indev_type_t indev_type = lv_indev_get_type(lv_indev_get_act());
uint32_t c = *((uint32_t *)param); 
if(c == LV_KEY_RIGHT) {
if(indev_type == LV_INDEV_TYPE_ENCODER)
lv_spinbox_increment(spinbox);
else
lv_spinbox_step_next(spinbox);
} else if(c == LV_KEY_LEFT) {
if(indev_type == LV_INDEV_TYPE_ENCODER)
lv_spinbox_decrement(spinbox);
else
lv_spinbox_step_prev(spinbox);
} else if(c == LV_KEY_UP) {
lv_spinbox_increment(spinbox);
} else if(c == LV_KEY_DOWN) {
lv_spinbox_decrement(spinbox);
} else {
lv_ta_add_char(spinbox, c);
}
}
return res;
}
static void lv_spinbox_updatevalue(lv_obj_t * spinbox)
{
lv_spinbox_ext_t * ext = lv_obj_get_ext_attr(spinbox);
char buf[LV_SPINBOX_MAX_DIGIT_COUNT + 8];
memset(buf, 0, sizeof(buf));
char * buf_p = buf;
uint8_t cur_shift_left = 0;
if (ext->range_min < 0) { 
(*buf_p) = ext->value >= 0 ? '+' : '-';
buf_p++;
} else {
cur_shift_left++;
}
int32_t i;
for(i = 0; i < ext->digit_padding_left; i++) {
(*buf_p) = ' ';
buf_p++;
}
char digits[64];
lv_utils_num_to_str(ext->value < 0 ? -ext->value : ext->value, digits);
int lz_cnt = ext->digit_count - (int)strlen(digits);
if(lz_cnt > 0) {
for(i = (uint16_t)strlen(digits); i >= 0; i--) {
digits[i + lz_cnt] = digits[i];
}
for(i = 0; i < lz_cnt; i++) {
digits[i] = '0';
}
}
int32_t intDigits;
intDigits = (ext->dec_point_pos == 0) ? ext->digit_count : ext->dec_point_pos;
for(i = 0; i < intDigits && digits[i] != '\0'; i++) {
(*buf_p) = digits[i];
buf_p++;
}
if(ext->dec_point_pos != 0) {
(*buf_p) = '.';
buf_p++;
for(; i < ext->digit_count && digits[i] != '\0'; i++) {
(*buf_p) = digits[i];
buf_p++;
}
}
lv_ta_set_text(spinbox, (char *)buf);
int32_t step = ext->step;
uint8_t cur_pos = (uint8_t)ext->digit_count;
while(step >= 10) {
step /= 10;
cur_pos--;
}
if(cur_pos > intDigits) cur_pos++; 
cur_pos += (ext->digit_padding_left - cur_shift_left);
lv_ta_set_cursor_pos(spinbox, cur_pos);
}
#endif
