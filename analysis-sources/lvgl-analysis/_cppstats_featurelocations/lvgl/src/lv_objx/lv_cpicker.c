








#include "lv_cpicker.h"
#if LV_USE_CPICKER != 0

#include "../lv_core/lv_debug.h"
#include "../lv_draw/lv_draw_arc.h"
#include "../lv_themes/lv_theme.h"
#include "../lv_core/lv_indev.h"
#include "../lv_core/lv_refr.h"
#include "../lv_misc/lv_math.h"




#define LV_OBJX_NAME "lv_cpicker"

#if !defined(LV_CPICKER_DEF_TYPE)
#define LV_CPICKER_DEF_TYPE LV_CPICKER_TYPE_DISC
#endif

#if !defined(LV_CPICKER_DEF_HUE)
#define LV_CPICKER_DEF_HUE 0
#endif

#if !defined(LV_CPICKER_DEF_SATURATION)
#define LV_CPICKER_DEF_SATURATION 100
#endif

#if !defined(LV_CPICKER_DEF_VALUE)
#define LV_CPICKER_DEF_VALUE 100
#endif

#if !defined(LV_CPICKER_DEF_HSV)
#define LV_CPICKER_DEF_HSV ((lv_color_hsv_t){LV_CPICKER_DEF_HUE, LV_CPICKER_DEF_SATURATION, LV_CPICKER_DEF_VALUE})
#endif

#if !defined(LV_CPICKER_DEF_QF)
#define LV_CPICKER_DEF_QF 3
#endif

#define TRI_OFFSET 2








static bool lv_cpicker_design(lv_obj_t * cpicker, const lv_area_t * mask, lv_design_mode_t mode);
static lv_res_t lv_cpicker_signal(lv_obj_t * cpicker, lv_signal_t sign, void * param);

static void draw_rect_grad(lv_obj_t * cpicker, const lv_area_t * mask, lv_opa_t opa_scale);
static void draw_disc_grad(lv_obj_t * cpicker, const lv_area_t * mask, lv_opa_t opa_scale);
static void draw_indic(lv_obj_t * cpicker, const lv_area_t * mask, lv_opa_t opa_scale);
static void invalidate_indic(lv_obj_t * cpicker);
static lv_area_t get_indic_area(lv_obj_t * cpicker);

static void next_color_mode(lv_obj_t * cpicker);
static lv_res_t double_click_reset(lv_obj_t * cpicker);
static void refr_indic_pos(lv_obj_t * cpicker);
static lv_color_t angle_to_mode_color(lv_obj_t * cpicker, uint16_t angle);
static uint16_t get_angle(lv_obj_t * cpicker);




static lv_signal_cb_t ancestor_signal;
static lv_design_cb_t ancestor_design;















lv_obj_t * lv_cpicker_create(lv_obj_t * par, const lv_obj_t * copy)
{
LV_LOG_TRACE("color_picker create started");

lv_obj_t * new_cpicker = lv_obj_create(par, copy);
LV_ASSERT_MEM(new_cpicker);
if(new_cpicker == NULL) return NULL;

if(ancestor_signal == NULL) ancestor_signal = lv_obj_get_signal_cb(new_cpicker);
if(ancestor_design == NULL) ancestor_design = lv_obj_get_design_cb(new_cpicker);


lv_cpicker_ext_t * ext = lv_obj_allocate_ext_attr(new_cpicker, sizeof(lv_cpicker_ext_t));
LV_ASSERT_MEM(ext);
if(ext == NULL) return NULL;


ext->type = LV_CPICKER_DEF_TYPE;
ext->hsv = LV_CPICKER_DEF_HSV;
ext->indic.style = &lv_style_plain;
ext->indic.colored = 0;
ext->color_mode = LV_CPICKER_COLOR_MODE_HUE;
ext->color_mode_fixed = 0;
ext->preview = 0;
ext->last_click_time = 0;
ext->last_change_time = 0;


lv_obj_set_signal_cb(new_cpicker, lv_cpicker_signal);
lv_obj_set_design_cb(new_cpicker, lv_cpicker_design);


if(copy == NULL) {
lv_obj_set_size(new_cpicker, LV_DPI * 2, LV_DPI * 2);
lv_obj_set_protect(new_cpicker, LV_PROTECT_PRESS_LOST);
lv_theme_t * th = lv_theme_get_current();
if(th) {
lv_cpicker_set_style(new_cpicker, LV_CPICKER_STYLE_MAIN, th->style.bg);
} else {
lv_cpicker_set_style(new_cpicker, LV_CPICKER_STYLE_MAIN, &lv_style_plain);
}
}

else {
lv_cpicker_ext_t * copy_ext = lv_obj_get_ext_attr(copy);
ext->type = copy_ext->type;
ext->color_mode = copy_ext->color_mode;
ext->color_mode_fixed = copy_ext->color_mode_fixed;
ext->preview = copy_ext->preview;
ext->hsv = copy_ext->hsv;
ext->indic.colored = copy_ext->indic.colored;
ext->indic.style = copy_ext->indic.style;


lv_obj_refresh_style(new_cpicker);
}
refr_indic_pos(new_cpicker);

LV_LOG_INFO("color_picker created");

return new_cpicker;
}










void lv_cpicker_set_type(lv_obj_t * cpicker, lv_cpicker_type_t type)
{
LV_ASSERT_OBJ(cpicker, LV_OBJX_NAME);

lv_cpicker_ext_t * ext = lv_obj_get_ext_attr(cpicker);
if(ext->type == type) return;

ext->type = type;
lv_obj_refresh_ext_draw_pad(cpicker);
refr_indic_pos(cpicker);

lv_obj_invalidate(cpicker);
}







void lv_cpicker_set_style(lv_obj_t * cpicker, lv_cpicker_style_t type, lv_style_t * style)
{
LV_ASSERT_OBJ(cpicker, LV_OBJX_NAME);

lv_cpicker_ext_t * ext = lv_obj_get_ext_attr(cpicker);

switch(type) {
case LV_CPICKER_STYLE_MAIN:
lv_obj_set_style(cpicker, style);
break;
case LV_CPICKER_STYLE_INDICATOR:
ext->indic.style = style;
lv_obj_invalidate(cpicker);
break;
}
}







bool lv_cpicker_set_hue(lv_obj_t * cpicker, uint16_t hue)
{
lv_color_hsv_t hsv = lv_cpicker_get_hsv(cpicker);
hsv.h = hue;
return lv_cpicker_set_hsv(cpicker, hsv);
}







bool lv_cpicker_set_saturation(lv_obj_t * cpicker, uint8_t saturation)
{
lv_color_hsv_t hsv = lv_cpicker_get_hsv(cpicker);
hsv.s = saturation;
return lv_cpicker_set_hsv(cpicker, hsv);
}







bool lv_cpicker_set_value(lv_obj_t * cpicker, uint8_t val)
{
lv_color_hsv_t hsv = lv_cpicker_get_hsv(cpicker);
hsv.v = val;
return lv_cpicker_set_hsv(cpicker, hsv);
}







bool lv_cpicker_set_hsv(lv_obj_t * cpicker, lv_color_hsv_t hsv)
{
LV_ASSERT_OBJ(cpicker, LV_OBJX_NAME);

if (hsv.h > 360) hsv.h %= 360;
if (hsv.s > 100) hsv.s = 100;
if (hsv.v > 100) hsv.v = 100;

lv_cpicker_ext_t * ext = lv_obj_get_ext_attr(cpicker);

if (ext->hsv.h == hsv.h && ext->hsv.s == hsv.s && ext->hsv.v == hsv.v) return false;

ext->hsv = hsv;

refr_indic_pos(cpicker);

if (ext->preview && ext->type == LV_CPICKER_TYPE_DISC) {
lv_obj_invalidate(cpicker);
}

return true;
}







bool lv_cpicker_set_color(lv_obj_t * cpicker, lv_color_t color)
{
lv_color32_t c32;
c32.full = lv_color_to32(color);

return lv_cpicker_set_hsv(cpicker,
lv_color_rgb_to_hsv(c32.ch.red, c32.ch.green, c32.ch.blue));
}






void lv_cpicker_set_color_mode(lv_obj_t * cpicker, lv_cpicker_color_mode_t mode)
{
LV_ASSERT_OBJ(cpicker, LV_OBJX_NAME);

lv_cpicker_ext_t * ext = lv_obj_get_ext_attr(cpicker);

ext->color_mode = mode;
refr_indic_pos(cpicker);
lv_obj_invalidate(cpicker);
}






void lv_cpicker_set_color_mode_fixed(lv_obj_t * cpicker, bool fixed)
{
LV_ASSERT_OBJ(cpicker, LV_OBJX_NAME);

lv_cpicker_ext_t * ext = lv_obj_get_ext_attr(cpicker);

ext->color_mode_fixed = fixed;
}






void lv_cpicker_set_indic_colored(lv_obj_t * cpicker, bool en)
{
LV_ASSERT_OBJ(cpicker, LV_OBJX_NAME);

lv_cpicker_ext_t * ext = lv_obj_get_ext_attr(cpicker);
ext->indic.colored = en ? 1 : 0;
invalidate_indic(cpicker);
}






void lv_cpicker_set_preview(lv_obj_t * cpicker, bool en)
{
LV_ASSERT_OBJ(cpicker, LV_OBJX_NAME);

lv_cpicker_ext_t * ext = lv_obj_get_ext_attr(cpicker);
ext->preview = en ? 1 : 0;
lv_obj_invalidate(cpicker);
}









lv_cpicker_color_mode_t lv_cpicker_get_color_mode(lv_obj_t * cpicker)
{
LV_ASSERT_OBJ(cpicker, LV_OBJX_NAME);

lv_cpicker_ext_t * ext = lv_obj_get_ext_attr(cpicker);

return ext->color_mode;
}






bool lv_cpicker_get_color_mode_fixed(lv_obj_t * cpicker)
{
LV_ASSERT_OBJ(cpicker, LV_OBJX_NAME);

lv_cpicker_ext_t * ext = lv_obj_get_ext_attr(cpicker);

return ext->color_mode_fixed;
}







const lv_style_t * lv_cpicker_get_style(const lv_obj_t * cpicker, lv_cpicker_style_t type)
{
LV_ASSERT_OBJ(cpicker, LV_OBJX_NAME);

lv_cpicker_ext_t * ext = lv_obj_get_ext_attr(cpicker);

switch(type) {
case LV_CPICKER_STYLE_MAIN:
return lv_obj_get_style(cpicker);
case LV_CPICKER_STYLE_INDICATOR:
return ext->indic.style;
default:
return NULL;
}


return NULL;
}






uint16_t lv_cpicker_get_hue(lv_obj_t * cpicker)
{
LV_ASSERT_OBJ(cpicker, LV_OBJX_NAME);

lv_cpicker_ext_t * ext = lv_obj_get_ext_attr(cpicker);

return ext->hsv.h;
}






uint8_t lv_cpicker_get_saturation(lv_obj_t * cpicker)
{
LV_ASSERT_OBJ(cpicker, LV_OBJX_NAME);

lv_cpicker_ext_t * ext = lv_obj_get_ext_attr(cpicker);

return ext->hsv.s;
}






uint8_t lv_cpicker_get_value(lv_obj_t * cpicker)
{
LV_ASSERT_OBJ(cpicker, LV_OBJX_NAME);

lv_cpicker_ext_t * ext = lv_obj_get_ext_attr(cpicker);

return ext->hsv.v;
}






lv_color_hsv_t lv_cpicker_get_hsv(lv_obj_t * cpicker)
{
LV_ASSERT_OBJ(cpicker, LV_OBJX_NAME);

lv_cpicker_ext_t * ext = lv_obj_get_ext_attr(cpicker);

return ext->hsv;
}






lv_color_t lv_cpicker_get_color(lv_obj_t * cpicker)
{
LV_ASSERT_OBJ(cpicker, LV_OBJX_NAME);

lv_cpicker_ext_t * ext = lv_obj_get_ext_attr(cpicker);

return lv_color_hsv_to_rgb(ext->hsv.h, ext->hsv.s, ext->hsv.v);
}






bool lv_cpicker_get_indic_colored(lv_obj_t * cpicker)
{
LV_ASSERT_OBJ(cpicker, LV_OBJX_NAME);

lv_cpicker_ext_t * ext = lv_obj_get_ext_attr(cpicker);

return ext->indic.colored ? true : false;
}






bool lv_cpicker_get_preview(lv_obj_t * cpicker)
{
LV_ASSERT_OBJ(cpicker, LV_OBJX_NAME);

lv_cpicker_ext_t * ext = lv_obj_get_ext_attr(cpicker);

return ext->preview ? true : false;
}





















static bool lv_cpicker_design(lv_obj_t * cpicker, const lv_area_t * mask, lv_design_mode_t mode)
{

if(mode == LV_DESIGN_COVER_CHK) {
return false;
}

else if(mode == LV_DESIGN_DRAW_MAIN) {
lv_cpicker_ext_t * ext = lv_obj_get_ext_attr(cpicker);
lv_opa_t opa_scale = lv_obj_get_opa_scale(cpicker);

if(ext->type == LV_CPICKER_TYPE_DISC) {
draw_disc_grad(cpicker, mask, opa_scale);
} else if(ext->type == LV_CPICKER_TYPE_RECT) {
draw_rect_grad(cpicker, mask, opa_scale);
}

draw_indic(cpicker, mask, opa_scale);
}

else if(mode == LV_DESIGN_DRAW_POST) {
}

return true;
}

static void draw_disc_grad(lv_obj_t * cpicker, const lv_area_t * mask, lv_opa_t opa_scale)
{
lv_cpicker_ext_t * ext = lv_obj_get_ext_attr(cpicker);
int16_t start_angle = 0; 
int16_t end_angle = 360 - LV_CPICKER_DEF_QF; 

lv_coord_t w = lv_obj_get_width(cpicker);
lv_coord_t h = lv_obj_get_height(cpicker);
lv_coord_t cx = cpicker->coords.x1 + w / 2;
lv_coord_t cy = cpicker->coords.y1 + h / 2;
lv_coord_t r = w / 2;





lv_point_t center = {cx, cy};
if(!lv_area_is_point_on(mask, &center))
{

int16_t dr, ur, ul, dl;
dr = lv_atan2(mask->x2 - cx, mask->y2 - cy);
ur = lv_atan2(mask->x2 - cx, mask->y1 - cy);
ul = lv_atan2(mask->x1 - cx, mask->y1 - cy);
dl = lv_atan2(mask->x1 - cx, mask->y2 - cy);


bool left = (mask->x2 < cx && mask->x1 < cx) ? true : false;
bool onYaxis = (mask->x2 > cx && mask->x1 < cx) ? true : false;
bool right = (mask->x2 > cx && mask->x1 > cx) ? true : false;
bool top = (mask->y2 < cy && mask->y1 < cy) ? true : false;
bool onXaxis = (mask->y2 > cy && mask->y1 < cy) ? true : false;
bool bottom = (mask->y2 > cy && mask->y1 > cy) ? true : false;


if(right && bottom) {
start_angle = dl;
end_angle = ur;
} else if(right && onXaxis) {
start_angle = dl;
end_angle = ul;
} else if(right && top) {
start_angle = dr;
end_angle = ul;
} else if(onYaxis && top) {
start_angle = dr;
end_angle = dl;
} else if(left && top) {
start_angle = ur;
end_angle = dl;
} else if(left && onXaxis) {
start_angle = ur;
end_angle = dr;
} else if(left && bottom) {
start_angle = ul;
end_angle = dr;
} else if(onYaxis && bottom) {
start_angle = ul;
end_angle = ur;
}


if(start_angle > end_angle) end_angle += 360;


start_angle = (start_angle/LV_CPICKER_DEF_QF) * LV_CPICKER_DEF_QF;
end_angle = (end_angle / LV_CPICKER_DEF_QF) * LV_CPICKER_DEF_QF;


if((start_angle - LV_CPICKER_DEF_QF) < 0) {
start_angle += 360;
end_angle += 360;
}


start_angle -= LV_CPICKER_DEF_QF;
end_angle += LV_CPICKER_DEF_QF;
}

lv_point_t triangle_points[3];
lv_style_t style;
lv_style_copy(&style, &lv_style_plain);
uint16_t i;
for(i = start_angle; i <= end_angle; i+= LV_CPICKER_DEF_QF) {
style.body.main_color = angle_to_mode_color(cpicker, i);
style.body.grad_color = style.body.main_color;

triangle_points[0].x = cx;
triangle_points[0].y = cy;

triangle_points[1].x = cx + (r * lv_trigo_sin(i) >> LV_TRIGO_SHIFT);
triangle_points[1].y = cy + (r * lv_trigo_sin(i + 90) >> LV_TRIGO_SHIFT);

if(i == end_angle || i == (360 - LV_CPICKER_DEF_QF)) {

triangle_points[2].x = cx + (r * lv_trigo_sin(i + LV_CPICKER_DEF_QF) >> LV_TRIGO_SHIFT);
triangle_points[2].y = cy + (r * lv_trigo_sin(i + LV_CPICKER_DEF_QF + 90) >> LV_TRIGO_SHIFT);
} else {
triangle_points[2].x = cx + (r * lv_trigo_sin(i + LV_CPICKER_DEF_QF + TRI_OFFSET) >> LV_TRIGO_SHIFT);
triangle_points[2].y = cy + (r * lv_trigo_sin(i + LV_CPICKER_DEF_QF + TRI_OFFSET + 90) >> LV_TRIGO_SHIFT);
}

lv_draw_triangle(triangle_points, mask, &style, LV_OPA_COVER);
}


const lv_style_t * style_main = lv_cpicker_get_style(cpicker, LV_CPICKER_STYLE_MAIN);
lv_style_copy(&style, style_main);
style.body.radius = LV_RADIUS_CIRCLE;
lv_area_t area_mid;
lv_area_copy(&area_mid, &cpicker->coords);
lv_area_increment(&area_mid, -style_main->line.width);

lv_draw_rect(&area_mid, mask, &style, opa_scale);

if(ext->preview) {
lv_color_t color = lv_cpicker_get_color(cpicker);
style.body.main_color = color;
style.body.grad_color = color;
lv_area_increment(&area_mid, -style_main->line.width / 2);

lv_draw_rect(&area_mid, mask, &style, opa_scale);
}
}

static void draw_rect_grad(lv_obj_t * cpicker, const lv_area_t * mask, lv_opa_t opa_scale)
{
lv_style_t style;
lv_style_copy(&style, lv_cpicker_get_style(cpicker, LV_CPICKER_STYLE_MAIN));

lv_area_t grad_area;
lv_obj_get_coords(cpicker, &grad_area);

if(style.body.radius) {
lv_coord_t h = lv_obj_get_height(cpicker);
lv_coord_t r = style.body.radius;
if(r > h / 2) r = h / 2;

grad_area.x1 += r;
grad_area.x2 -= r;


lv_area_t rounded_edge_area;
lv_obj_get_coords(cpicker, &rounded_edge_area);
rounded_edge_area.x2 = rounded_edge_area.x1 + 2 * r;

style.body.main_color = angle_to_mode_color(cpicker, 0);
style.body.grad_color = style.body.main_color;

lv_draw_rect(&rounded_edge_area, mask, &style, opa_scale);


lv_obj_get_coords(cpicker, &rounded_edge_area);
rounded_edge_area.x1 = rounded_edge_area.x2 - 2 * r;

style.body.main_color = angle_to_mode_color(cpicker, 359);
style.body.grad_color = style.body.main_color;

lv_draw_rect(&rounded_edge_area, mask, &style, opa_scale);
}

lv_coord_t grad_w = lv_area_get_width(&grad_area);
uint16_t i_step = LV_MATH_MAX(LV_CPICKER_DEF_QF, 360 / grad_w);
style.body.radius = 0;
style.body.border.width = 0;
style.body.shadow.width = 0;
style.body.opa = LV_OPA_COVER;

uint16_t i;
for(i = 0; i < 360; i += i_step) {
style.body.main_color = angle_to_mode_color(cpicker, i);
style.body.grad_color = style.body.main_color;


lv_area_t rect_area;


lv_coord_t xi = (i * grad_w) / 360;

rect_area.x1 = LV_MATH_MIN(grad_area.x1 + xi, grad_area.x1 + grad_w - i_step);
rect_area.y1 = grad_area.y1;
rect_area.x2 = rect_area.x1 + i_step;
rect_area.y2 = grad_area.y2;

lv_draw_rect(&rect_area, mask, &style, opa_scale);
}
}

static void draw_indic(lv_obj_t * cpicker, const lv_area_t * mask, lv_opa_t opa_scale)
{
lv_cpicker_ext_t * ext = lv_obj_get_ext_attr(cpicker);

lv_style_t style_cir;
lv_style_copy(&style_cir, ext->indic.style);
style_cir.body.radius = LV_RADIUS_CIRCLE;

if(ext->indic.colored) {
style_cir.body.main_color = lv_cpicker_get_color(cpicker);
style_cir.body.grad_color = style_cir.body.main_color;
}

lv_area_t indic_area = get_indic_area(cpicker);

lv_draw_rect(&indic_area, mask, &style_cir, opa_scale);
}

static void invalidate_indic(lv_obj_t * cpicker)
{
lv_area_t indic_area = get_indic_area(cpicker);

lv_obj_invalidate_area(cpicker, &indic_area);
}

static lv_area_t get_indic_area(lv_obj_t * cpicker)
{
lv_cpicker_ext_t * ext = lv_obj_get_ext_attr(cpicker);
const lv_style_t * style_main = lv_cpicker_get_style(cpicker, LV_CPICKER_STYLE_MAIN);
const lv_style_t * style_indic = lv_cpicker_get_style(cpicker, LV_CPICKER_STYLE_INDICATOR);

uint16_t r = 0;
if(ext->type == LV_CPICKER_TYPE_DISC) r = style_main->line.width / 2;
else if(ext->type == LV_CPICKER_TYPE_RECT) {
lv_coord_t h = lv_obj_get_height(cpicker);
r = h / 2;
}

lv_area_t indic_area;
indic_area.x1 = cpicker->coords.x1 + ext->indic.pos.x - r - style_indic->body.padding.left;
indic_area.y1 = cpicker->coords.y1 + ext->indic.pos.y - r - style_indic->body.padding.right;
indic_area.x2 = cpicker->coords.x1 + ext->indic.pos.x + r + style_indic->body.padding.top;
indic_area.y2 = cpicker->coords.y1 + ext->indic.pos.y + r + style_indic->body.padding.bottom;

return indic_area;
}








static lv_res_t lv_cpicker_signal(lv_obj_t * cpicker, lv_signal_t sign, void * param)
{

lv_res_t res = ancestor_signal(cpicker, sign, param);
if(res != LV_RES_OK) return res;
if(sign == LV_SIGNAL_GET_TYPE) return lv_obj_handle_get_type_signal(param, LV_OBJX_NAME);

lv_cpicker_ext_t * ext = lv_obj_get_ext_attr(cpicker);

if(sign == LV_SIGNAL_CLEANUP) {

} else if(sign == LV_SIGNAL_REFR_EXT_DRAW_PAD) {
const lv_style_t * style_indic = lv_cpicker_get_style(cpicker, LV_CPICKER_STYLE_INDICATOR);
lv_coord_t indic_pad = LV_MATH_MAX(style_indic->body.padding.left, style_indic->body.padding.right);
indic_pad = LV_MATH_MAX(indic_pad, style_indic->body.padding.top);
indic_pad = LV_MATH_MAX(indic_pad, style_indic->body.padding.bottom);

if(ext->type == LV_CPICKER_TYPE_RECT) indic_pad += LV_MATH_MAX(indic_pad, lv_obj_get_height(cpicker) / 2);

cpicker->ext_draw_pad = LV_MATH_MAX(cpicker->ext_draw_pad, indic_pad);
} else if(sign == LV_SIGNAL_CORD_CHG) {

if(lv_obj_get_width(cpicker) != lv_area_get_width(param) ||
lv_obj_get_height(cpicker) != lv_area_get_height(param)) {
lv_obj_refresh_ext_draw_pad(cpicker);
refr_indic_pos(cpicker);
}
} else if(sign == LV_SIGNAL_STYLE_CHG) {

lv_obj_refresh_ext_draw_pad(cpicker);
refr_indic_pos(cpicker);
}
else if(sign == LV_SIGNAL_CONTROL) {
uint32_t c = *((uint32_t *)param); 
if(c == LV_KEY_RIGHT || c == LV_KEY_UP) {
lv_color_hsv_t hsv_cur;
hsv_cur = ext->hsv;

switch(ext->color_mode) {
case LV_CPICKER_COLOR_MODE_HUE:
hsv_cur.h = (ext->hsv.h + 1) % 360;
break;
case LV_CPICKER_COLOR_MODE_SATURATION:
hsv_cur.s = (ext->hsv.s + 1) % 100;
break;
case LV_CPICKER_COLOR_MODE_VALUE:
hsv_cur.v = (ext->hsv.v + 1) % 100;
break;
}

if (lv_cpicker_set_hsv(cpicker, hsv_cur)) {
res = lv_event_send(cpicker, LV_EVENT_VALUE_CHANGED, NULL);
if(res != LV_RES_OK) return res;
}
}
else if(c == LV_KEY_LEFT || c == LV_KEY_DOWN) {
lv_color_hsv_t hsv_cur;
hsv_cur = ext->hsv;

switch(ext->color_mode) {
case LV_CPICKER_COLOR_MODE_HUE:
hsv_cur.h = ext->hsv.h > 0?(ext->hsv.h - 1) : 360;
break;
case LV_CPICKER_COLOR_MODE_SATURATION:
hsv_cur.s = ext->hsv.s > 0?(ext->hsv.s - 1) : 100;
break;
case LV_CPICKER_COLOR_MODE_VALUE:
hsv_cur.v = ext->hsv.v > 0?(ext->hsv.v - 1) : 100;
break;
}

if (lv_cpicker_set_hsv(cpicker, hsv_cur)) {
res = lv_event_send(cpicker, LV_EVENT_VALUE_CHANGED, NULL);
if(res != LV_RES_OK) return res;
}
}
}
else if(sign == LV_SIGNAL_PRESSED) {
ext->last_change_time = lv_tick_get();
lv_indev_get_point(lv_indev_get_act(), &ext->last_press_point);
res = double_click_reset(cpicker);
if(res != LV_RES_OK) return res;
} else if(sign == LV_SIGNAL_PRESSING) {
lv_indev_t * indev = lv_indev_get_act();
if(indev == NULL) return res;

lv_point_t p;
lv_indev_get_point(indev, &p);

if((LV_MATH_ABS(p.x - ext->last_press_point.x) > indev->driver.drag_limit / 2) ||
(LV_MATH_ABS(p.y - ext->last_press_point.y) > indev->driver.drag_limit / 2)) {
ext->last_change_time = lv_tick_get();
ext->last_press_point.x = p.x;
ext->last_press_point.y = p.y;
}

p.x -= cpicker->coords.x1;
p.y -= cpicker->coords.y1;


uint16_t w = lv_obj_get_width(cpicker);

int16_t angle = 0;

if(ext->type == LV_CPICKER_TYPE_RECT) {

uint32_t diff = lv_tick_elaps(ext->last_change_time);
if(diff > (uint32_t)indev->driver.long_press_time * 2 && !ext->color_mode_fixed) {
next_color_mode(cpicker);
lv_indev_wait_release(lv_indev_get_act());
return res;
}

angle = (p.x * 360) / w;
if(angle < 0) angle = 0;
if(angle >= 360) angle = 359;

} else if(ext->type == LV_CPICKER_TYPE_DISC) {
const lv_style_t * style_main = lv_cpicker_get_style(cpicker, LV_CPICKER_STYLE_MAIN);
lv_coord_t r_in = w / 2;
p.x -= r_in;
p.y -= r_in;
r_in -= style_main->line.width;

if(r_in > LV_DPI / 2) {
r_in -= style_main->line.width; 

if(r_in < LV_DPI / 2) r_in = LV_DPI / 2;
}


if(p.x * p.x + p.y * p.y < r_in * r_in) {
uint32_t diff = lv_tick_elaps(ext->last_change_time);
if(diff > indev->driver.long_press_time && !ext->color_mode_fixed) {
next_color_mode(cpicker);
lv_indev_wait_release(lv_indev_get_act());
}
return res;
}

angle = lv_atan2(p.x, p.y) % 360;
}

lv_color_hsv_t hsv_cur;
hsv_cur = ext->hsv;

switch(ext->color_mode) {
case LV_CPICKER_COLOR_MODE_HUE:
hsv_cur.h = angle;
break;
case LV_CPICKER_COLOR_MODE_SATURATION:
hsv_cur.s = (angle * 100) / 360;
break;
case LV_CPICKER_COLOR_MODE_VALUE:
hsv_cur.v = (angle * 100) / 360;
break;
}

if (lv_cpicker_set_hsv(cpicker, hsv_cur)) {
res = lv_event_send(cpicker, LV_EVENT_VALUE_CHANGED, NULL);
if(res != LV_RES_OK) return res;
}
}

return res;
}

static void next_color_mode(lv_obj_t * cpicker)
{
lv_cpicker_ext_t * ext = lv_obj_get_ext_attr(cpicker);
ext->color_mode = (ext->color_mode + 1) % 3;
refr_indic_pos(cpicker);
lv_obj_invalidate(cpicker);
}

static void refr_indic_pos(lv_obj_t * cpicker)
{
invalidate_indic(cpicker);

lv_cpicker_ext_t * ext = lv_obj_get_ext_attr(cpicker);
lv_coord_t w = lv_obj_get_width(cpicker);
lv_coord_t h = lv_obj_get_height(cpicker);

if(ext->type == LV_CPICKER_TYPE_RECT) {
lv_coord_t ind_pos = 0;
switch(ext->color_mode) {
case LV_CPICKER_COLOR_MODE_HUE:
ind_pos += (ext->hsv.h * w) / 360;
break;
case LV_CPICKER_COLOR_MODE_SATURATION:
ind_pos += (ext->hsv.s * w) / 100;
break;
case LV_CPICKER_COLOR_MODE_VALUE:
ind_pos += (ext->hsv.v * w) / 100;
break;
}

ext->indic.pos.x = ind_pos;
ext->indic.pos.y = h / 2;
} else if(ext->type == LV_CPICKER_TYPE_DISC) {
const lv_style_t * style_main = lv_cpicker_get_style(cpicker, LV_CPICKER_STYLE_MAIN);
lv_coord_t r = w / 2 - style_main->line.width / 2;
uint16_t angle = get_angle(cpicker);
ext->indic.pos.x = (((int32_t)r * lv_trigo_sin(angle)) >> LV_TRIGO_SHIFT);
ext->indic.pos.y = (((int32_t)r * lv_trigo_sin(angle + 90)) >> LV_TRIGO_SHIFT);
ext->indic.pos.x = ext->indic.pos.x + w / 2;
ext->indic.pos.y = ext->indic.pos.y + h / 2;
}

invalidate_indic(cpicker);
}

static lv_res_t double_click_reset(lv_obj_t * cpicker)
{
lv_cpicker_ext_t * ext = lv_obj_get_ext_attr(cpicker);
lv_indev_t * indev = lv_indev_get_act();

if(lv_tick_elaps(ext->last_click_time) < indev->driver.long_press_time) {
lv_color_hsv_t hsv_cur;
hsv_cur = ext->hsv;

switch(ext->color_mode) {
case LV_CPICKER_COLOR_MODE_HUE:
hsv_cur.h = LV_CPICKER_DEF_HUE;
break;
case LV_CPICKER_COLOR_MODE_SATURATION:
hsv_cur.s = LV_CPICKER_DEF_SATURATION;
break;
case LV_CPICKER_COLOR_MODE_VALUE:
hsv_cur.v = LV_CPICKER_DEF_VALUE;
break;
}

if (lv_cpicker_set_hsv(cpicker, hsv_cur)) {
lv_res_t res = lv_event_send(cpicker, LV_EVENT_VALUE_CHANGED, NULL);
if(res != LV_RES_OK) return res;
}
}
ext->last_click_time = lv_tick_get();

return LV_RES_OK;
}

static lv_color_t angle_to_mode_color(lv_obj_t * cpicker, uint16_t angle)
{
lv_cpicker_ext_t * ext = lv_obj_get_ext_attr(cpicker);
lv_color_t color;
switch(ext->color_mode)
{
default:
case LV_CPICKER_COLOR_MODE_HUE:
color = lv_color_hsv_to_rgb(angle % 360, ext->hsv.s, ext->hsv.v);
break;
case LV_CPICKER_COLOR_MODE_SATURATION:
color = lv_color_hsv_to_rgb(ext->hsv.h, ((angle % 360) * 100) / 360, ext->hsv.v);
break;
case LV_CPICKER_COLOR_MODE_VALUE:
color = lv_color_hsv_to_rgb(ext->hsv.h, ext->hsv.s, ((angle % 360) * 100) / 360);
break;
}
return color;
}

static uint16_t get_angle(lv_obj_t * cpicker)
{
lv_cpicker_ext_t * ext = lv_obj_get_ext_attr(cpicker);
uint16_t angle;
switch(ext->color_mode)
{
default:
case LV_CPICKER_COLOR_MODE_HUE:
angle = ext->hsv.h;
break;
case LV_CPICKER_COLOR_MODE_SATURATION:
angle = (ext->hsv.s * 360) / 100;
break;
case LV_CPICKER_COLOR_MODE_VALUE:
angle = (ext->hsv.v * 360) / 100 ;
break;
}
return angle;
}

#endif 
