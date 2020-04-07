







#include "lv_line.h"

#if LV_USE_LINE != 0
#include "../lv_core/lv_debug.h"
#include "../lv_draw/lv_draw.h"
#include "../lv_misc/lv_math.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>




#define LV_OBJX_NAME "lv_line"








static bool lv_line_design(lv_obj_t * line, const lv_area_t * mask, lv_design_mode_t mode);
static lv_res_t lv_line_signal(lv_obj_t * line, lv_signal_t sign, void * param);




static lv_signal_cb_t ancestor_signal;














lv_obj_t * lv_line_create(lv_obj_t * par, const lv_obj_t * copy)
{
LV_LOG_TRACE("line create started");


lv_obj_t * new_line = lv_obj_create(par, copy);
LV_ASSERT_MEM(new_line);
if(new_line == NULL) return NULL;

if(ancestor_signal == NULL) ancestor_signal = lv_obj_get_signal_cb(new_line);


lv_line_ext_t * ext = lv_obj_allocate_ext_attr(new_line, sizeof(lv_line_ext_t));
LV_ASSERT_MEM(ext);
if(ext == NULL) return NULL;

ext->point_num = 0;
ext->point_array = NULL;
ext->auto_size = 1;
ext->y_inv = 0;

lv_obj_set_design_cb(new_line, lv_line_design);
lv_obj_set_signal_cb(new_line, lv_line_signal);


if(copy == NULL) {
lv_obj_set_size(new_line, LV_DPI,
LV_DPI); 
lv_obj_set_style(new_line, NULL); 
lv_obj_set_click(new_line, false);
}

else {
lv_line_ext_t * copy_ext = lv_obj_get_ext_attr(copy);
lv_line_set_auto_size(new_line, lv_line_get_auto_size(copy));
lv_line_set_y_invert(new_line, lv_line_get_y_invert(copy));
lv_line_set_auto_size(new_line, lv_line_get_auto_size(copy));
lv_line_set_points(new_line, copy_ext->point_array, copy_ext->point_num);

lv_obj_refresh_style(new_line);
}

LV_LOG_INFO("line created");

return new_line;
}












void lv_line_set_points(lv_obj_t * line, const lv_point_t point_a[], uint16_t point_num)
{
LV_ASSERT_OBJ(line, LV_OBJX_NAME);

lv_line_ext_t * ext = lv_obj_get_ext_attr(line);
ext->point_array = point_a;
ext->point_num = point_num;

if(point_num > 0 && ext->auto_size != 0) {
uint16_t i;
lv_coord_t xmax = LV_COORD_MIN;
lv_coord_t ymax = LV_COORD_MIN;
for(i = 0; i < point_num; i++) {
xmax = LV_MATH_MAX(point_a[i].x, xmax);
ymax = LV_MATH_MAX(point_a[i].y, ymax);
}

const lv_style_t * style = lv_line_get_style(line, LV_LINE_STYLE_MAIN);
lv_obj_set_size(line, xmax + style->line.width, ymax + style->line.width);
}

lv_obj_invalidate(line);
}







void lv_line_set_auto_size(lv_obj_t * line, bool en)
{
LV_ASSERT_OBJ(line, LV_OBJX_NAME);

lv_line_ext_t * ext = lv_obj_get_ext_attr(line);
if(ext->auto_size == en) return;

ext->auto_size = en == false ? 0 : 1;


if(en) lv_line_set_points(line, ext->point_array, ext->point_num);
}








void lv_line_set_y_invert(lv_obj_t * line, bool en)
{
LV_ASSERT_OBJ(line, LV_OBJX_NAME);

lv_line_ext_t * ext = lv_obj_get_ext_attr(line);
if(ext->y_inv == en) return;

ext->y_inv = en == false ? 0 : 1;

lv_obj_invalidate(line);
}










bool lv_line_get_auto_size(const lv_obj_t * line)
{
LV_ASSERT_OBJ(line, LV_OBJX_NAME);

lv_line_ext_t * ext = lv_obj_get_ext_attr(line);

return ext->auto_size == 0 ? false : true;
}






bool lv_line_get_y_invert(const lv_obj_t * line)
{
LV_ASSERT_OBJ(line, LV_OBJX_NAME);

lv_line_ext_t * ext = lv_obj_get_ext_attr(line);

return ext->y_inv == 0 ? false : true;
}















static bool lv_line_design(lv_obj_t * line, const lv_area_t * mask, lv_design_mode_t mode)
{

if(mode == LV_DESIGN_COVER_CHK)
return false;
else if(mode == LV_DESIGN_DRAW_MAIN) {
lv_line_ext_t * ext = lv_obj_get_ext_attr(line);

if(ext->point_num == 0 || ext->point_array == NULL) return false;

const lv_style_t * style = lv_obj_get_style(line);
lv_opa_t opa_scale = lv_obj_get_opa_scale(line);
lv_area_t area;
lv_obj_get_coords(line, &area);
lv_coord_t x_ofs = area.x1;
lv_coord_t y_ofs = area.y1;
lv_point_t p1;
lv_point_t p2;
lv_coord_t h = lv_obj_get_height(line);
uint16_t i;

lv_style_t circle_style_tmp; 
if(style->line.rounded) {
lv_style_copy(&circle_style_tmp, style);
circle_style_tmp.body.radius = LV_RADIUS_CIRCLE;
circle_style_tmp.body.main_color = style->line.color;
circle_style_tmp.body.grad_color = style->line.color;
circle_style_tmp.body.opa = style->line.opa;
}
lv_area_t circle_area;


for(i = 0; i < ext->point_num - 1; i++) {

p1.x = ext->point_array[i].x + x_ofs;
p2.x = ext->point_array[i + 1].x + x_ofs;

if(ext->y_inv == 0) {
p1.y = ext->point_array[i].y + y_ofs;
p2.y = ext->point_array[i + 1].y + y_ofs;
} else {
p1.y = h - ext->point_array[i].y + y_ofs;
p2.y = h - ext->point_array[i + 1].y + y_ofs;
}
lv_draw_line(&p1, &p2, mask, style, opa_scale);


if(style->line.rounded) {
circle_area.x1 = p1.x - ((style->line.width - 1) >> 1) - ((style->line.width - 1) & 0x1);
circle_area.y1 = p1.y - ((style->line.width - 1) >> 1) - ((style->line.width - 1) & 0x1);
circle_area.x2 = p1.x + ((style->line.width - 1) >> 1);
circle_area.y2 = p1.y + ((style->line.width - 1) >> 1);
lv_draw_rect(&circle_area, mask, &circle_style_tmp, opa_scale);
}
}


if(style->line.rounded) {
circle_area.x1 = p2.x - ((style->line.width - 1) >> 1) - ((style->line.width - 1) & 0x1);
circle_area.y1 = p2.y - ((style->line.width - 1) >> 1) - ((style->line.width - 1) & 0x1);
circle_area.x2 = p2.x + ((style->line.width - 1) >> 1);
circle_area.y2 = p2.y + ((style->line.width - 1) >> 1);
lv_draw_rect(&circle_area, mask, &circle_style_tmp, opa_scale);
}
}
return true;
}







static lv_res_t lv_line_signal(lv_obj_t * line, lv_signal_t sign, void * param)
{
lv_res_t res;


res = ancestor_signal(line, sign, param);
if(res != LV_RES_OK) return res;
if(sign == LV_SIGNAL_GET_TYPE) return lv_obj_handle_get_type_signal(param, LV_OBJX_NAME);

if(sign == LV_SIGNAL_REFR_EXT_DRAW_PAD) {
const lv_style_t * style = lv_line_get_style(line, LV_LINE_STYLE_MAIN);
if(line->ext_draw_pad < style->line.width) line->ext_draw_pad = style->line.width;
}

return res;
}
#endif
