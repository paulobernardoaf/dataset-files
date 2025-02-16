







#include "lv_draw_rect.h"
#include "../lv_misc/lv_circ.h"
#include "../lv_misc/lv_math.h"
#include "../lv_core/lv_refr.h"






#define CIRCLE_AA_NON_LINEAR_OPA_THRESHOLD 1


#define SHADOW_OPA_EXTRA_PRECISION 8


#define SHADOW_BOTTOM_AA_EXTRA_RADIUS 3








static void lv_draw_rect_main_mid(const lv_area_t * coords, const lv_area_t * mask, const lv_style_t * style,
lv_opa_t opa_scale);
static void lv_draw_rect_main_corner(const lv_area_t * coords, const lv_area_t * mask, const lv_style_t * style,
lv_opa_t opa_scale);
static void lv_draw_rect_border_straight(const lv_area_t * coords, const lv_area_t * mask, const lv_style_t * style,
lv_opa_t opa_scale);
static void lv_draw_rect_border_corner(const lv_area_t * coords, const lv_area_t * mask, const lv_style_t * style,
lv_opa_t opa_scale);

#if LV_USE_SHADOW
static void lv_draw_shadow(const lv_area_t * coords, const lv_area_t * mask, const lv_style_t * style,
lv_opa_t opa_scale);
static void lv_draw_shadow_full(const lv_area_t * coords, const lv_area_t * mask, const lv_style_t * style,
lv_opa_t opa_scale);
static void lv_draw_shadow_bottom(const lv_area_t * coords, const lv_area_t * mask, const lv_style_t * style,
lv_opa_t opa_scale);
static void lv_draw_shadow_full_straight(const lv_area_t * coords, const lv_area_t * mask, const lv_style_t * style,
const lv_opa_t * map);
#endif

static uint16_t lv_draw_cont_radius_corr(uint16_t r, lv_coord_t w, lv_coord_t h);

#if LV_ANTIALIAS
static lv_opa_t antialias_get_opa_circ(lv_coord_t seg, lv_coord_t px_id, lv_opa_t opa);
#endif




















void lv_draw_rect(const lv_area_t * coords, const lv_area_t * mask, const lv_style_t * style, lv_opa_t opa_scale)
{
if(lv_area_get_height(coords) < 1 || lv_area_get_width(coords) < 1) return;

#if LV_USE_SHADOW
if(style->body.shadow.width != 0) {
lv_draw_shadow(coords, mask, style, opa_scale);
}
#endif



if(lv_area_is_on(coords, mask) == false) return;

if(style->body.opa > LV_OPA_MIN) {
lv_draw_rect_main_mid(coords, mask, style, opa_scale);

if(style->body.radius != 0) {
lv_draw_rect_main_corner(coords, mask, style, opa_scale);
}
}

if(style->body.border.width != 0 && style->body.border.part != LV_BORDER_NONE &&
style->body.border.opa >= LV_OPA_MIN) {
lv_draw_rect_border_straight(coords, mask, style, opa_scale);

if(style->body.radius != 0) {
lv_draw_rect_border_corner(coords, mask, style, opa_scale);
}
}
}












static void lv_draw_rect_main_mid(const lv_area_t * coords, const lv_area_t * mask, const lv_style_t * style,
lv_opa_t opa_scale)
{
uint16_t radius = style->body.radius;
bool aa = lv_disp_get_antialiasing(lv_refr_get_disp_refreshing());

lv_color_t mcolor = style->body.main_color;
lv_color_t gcolor = style->body.grad_color;
uint8_t mix;
lv_coord_t height = lv_area_get_height(coords);
lv_coord_t width = lv_area_get_width(coords);
lv_opa_t opa = opa_scale == LV_OPA_COVER ? style->body.opa : (uint16_t)((uint16_t)style->body.opa * opa_scale) >> 8;

radius = lv_draw_cont_radius_corr(radius, width, height);


if(radius > height / 2) return;

lv_area_t work_area;
work_area.x1 = coords->x1;
work_area.x2 = coords->x2;

if(mcolor.full == gcolor.full) {
work_area.y1 = coords->y1 + radius;
work_area.y2 = coords->y2 - radius;

if(style->body.radius != 0) {

if(aa) {
work_area.y1 += 2;
work_area.y2 -= 2;
} else {
work_area.y1 += 1;
work_area.y2 -= 1;
}
}

lv_draw_fill(&work_area, mask, mcolor, opa);
} else {
lv_coord_t row;
lv_coord_t row_start = coords->y1 + radius;
lv_coord_t row_end = coords->y2 - radius;
lv_color_t act_color;

if(style->body.radius != 0) {
if(aa) {
row_start += 2;
row_end -= 2;
} else {
row_start += 1;
row_end -= 1;
}
}
if(row_start < 0) row_start = 0;

for(row = row_start; row <= row_end; row++) {
work_area.y1 = row;
work_area.y2 = row;
mix = (uint32_t)((uint32_t)(coords->y2 - work_area.y1) * 255) / height;
act_color = lv_color_mix(mcolor, gcolor, mix);

lv_draw_fill(&work_area, mask, act_color, opa);
}
}
}







static void lv_draw_rect_main_corner(const lv_area_t * coords, const lv_area_t * mask, const lv_style_t * style,
lv_opa_t opa_scale)
{
uint16_t radius = style->body.radius;
bool aa = lv_disp_get_antialiasing(lv_refr_get_disp_refreshing());

lv_color_t mcolor = style->body.main_color;
lv_color_t gcolor = style->body.grad_color;
lv_color_t act_color;
lv_opa_t opa = opa_scale == LV_OPA_COVER ? style->body.opa : (uint16_t)((uint16_t)style->body.opa * opa_scale) >> 8;
uint8_t mix;
lv_coord_t height = lv_area_get_height(coords);
lv_coord_t width = lv_area_get_width(coords);

radius = lv_draw_cont_radius_corr(radius, width, height);

lv_point_t lt_origo; 
lv_point_t lb_origo; 
lv_point_t rt_origo; 
lv_point_t rb_origo; 

lt_origo.x = coords->x1 + radius + aa;
lt_origo.y = coords->y1 + radius + aa;

lb_origo.x = coords->x1 + radius + aa;
lb_origo.y = coords->y2 - radius - aa;

rt_origo.x = coords->x2 - radius - aa;
rt_origo.y = coords->y1 + radius + aa;

rb_origo.x = coords->x2 - radius - aa;
rb_origo.y = coords->y2 - radius - aa;

lv_area_t edge_top_area;
lv_area_t mid_top_area;
lv_area_t mid_bot_area;
lv_area_t edge_bot_area;

lv_point_t cir;
lv_coord_t cir_tmp;
lv_circ_init(&cir, &cir_tmp, radius);


lv_area_set(&mid_bot_area, lb_origo.x + LV_CIRC_OCT4_X(cir), lb_origo.y + LV_CIRC_OCT4_Y(cir),
rb_origo.x + LV_CIRC_OCT1_X(cir), rb_origo.y + LV_CIRC_OCT1_Y(cir));

lv_area_set(&edge_bot_area, lb_origo.x + LV_CIRC_OCT3_X(cir), lb_origo.y + LV_CIRC_OCT3_Y(cir),
rb_origo.x + LV_CIRC_OCT2_X(cir), rb_origo.y + LV_CIRC_OCT2_Y(cir));

lv_area_set(&mid_top_area, lt_origo.x + LV_CIRC_OCT5_X(cir), lt_origo.y + LV_CIRC_OCT5_Y(cir),
rt_origo.x + LV_CIRC_OCT8_X(cir), rt_origo.y + LV_CIRC_OCT8_Y(cir));

lv_area_set(&edge_top_area, lt_origo.x + LV_CIRC_OCT6_X(cir), lt_origo.y + LV_CIRC_OCT6_Y(cir),
rt_origo.x + LV_CIRC_OCT7_X(cir), rt_origo.y + LV_CIRC_OCT7_Y(cir));
#if LV_ANTIALIAS

lv_coord_t out_y_seg_start = 0;
lv_coord_t out_y_seg_end = 0;
lv_coord_t out_x_last = radius;

lv_color_t aa_color_hor_top;
lv_color_t aa_color_hor_bottom;
lv_color_t aa_color_ver;
#endif

while(lv_circ_cont(&cir)) {
#if LV_ANTIALIAS
if(aa) {

if(out_x_last != cir.x) {
out_y_seg_end = cir.y;
lv_coord_t seg_size = out_y_seg_end - out_y_seg_start;
lv_point_t aa_p;

aa_p.x = out_x_last;
aa_p.y = out_y_seg_start;

mix = (uint32_t)((uint32_t)(radius - out_x_last) * 255) / height;
aa_color_hor_top = lv_color_mix(gcolor, mcolor, mix);
aa_color_hor_bottom = lv_color_mix(mcolor, gcolor, mix);

lv_coord_t i;
for(i = 0; i < seg_size; i++) {
lv_opa_t aa_opa;
if(seg_size > CIRCLE_AA_NON_LINEAR_OPA_THRESHOLD) { 

aa_opa = antialias_get_opa_circ(seg_size, i, opa);
} else {
aa_opa = opa - lv_draw_aa_get_opa(seg_size, i, opa);
}

lv_draw_px(rb_origo.x + LV_CIRC_OCT2_X(aa_p) + i, rb_origo.y + LV_CIRC_OCT2_Y(aa_p) + 1, mask,
aa_color_hor_bottom, aa_opa);
lv_draw_px(lb_origo.x + LV_CIRC_OCT3_X(aa_p) - i, lb_origo.y + LV_CIRC_OCT3_Y(aa_p) + 1, mask,
aa_color_hor_bottom, aa_opa);
lv_draw_px(lt_origo.x + LV_CIRC_OCT6_X(aa_p) - i, lt_origo.y + LV_CIRC_OCT6_Y(aa_p) - 1, mask,
aa_color_hor_top, aa_opa);
lv_draw_px(rt_origo.x + LV_CIRC_OCT7_X(aa_p) + i, rt_origo.y + LV_CIRC_OCT7_Y(aa_p) - 1, mask,
aa_color_hor_top, aa_opa);

mix = (uint32_t)((uint32_t)(radius - out_y_seg_start + i) * 255) / height;
aa_color_ver = lv_color_mix(mcolor, gcolor, mix);
lv_draw_px(rb_origo.x + LV_CIRC_OCT1_X(aa_p) + 1, rb_origo.y + LV_CIRC_OCT1_Y(aa_p) + i, mask,
aa_color_ver, aa_opa);
lv_draw_px(lb_origo.x + LV_CIRC_OCT4_X(aa_p) - 1, lb_origo.y + LV_CIRC_OCT4_Y(aa_p) + i, mask,
aa_color_ver, aa_opa);

aa_color_ver = lv_color_mix(gcolor, mcolor, mix);
lv_draw_px(lt_origo.x + LV_CIRC_OCT5_X(aa_p) - 1, lt_origo.y + LV_CIRC_OCT5_Y(aa_p) - i, mask,
aa_color_ver, aa_opa);
lv_draw_px(rt_origo.x + LV_CIRC_OCT8_X(aa_p) + 1, rt_origo.y + LV_CIRC_OCT8_Y(aa_p) - i, mask,
aa_color_ver, aa_opa);
}

out_x_last = cir.x;
out_y_seg_start = out_y_seg_end;
}
}
#endif
uint8_t edge_top_refr = 0;
uint8_t mid_top_refr = 0;
uint8_t mid_bot_refr = 0;
uint8_t edge_bot_refr = 0;



if(mid_bot_area.y1 != LV_CIRC_OCT4_Y(cir) + lb_origo.y) mid_bot_refr = 1;

if(edge_bot_area.y1 != LV_CIRC_OCT2_Y(cir) + lb_origo.y) edge_bot_refr = 1;

if(mid_top_area.y1 != LV_CIRC_OCT8_Y(cir) + lt_origo.y) mid_top_refr = 1;

if(edge_top_area.y1 != LV_CIRC_OCT7_Y(cir) + lt_origo.y) edge_top_refr = 1;


if(edge_top_refr != 0) {
if(mcolor.full == gcolor.full)
act_color = mcolor;
else {
mix = (uint32_t)((uint32_t)(coords->y2 - edge_top_area.y1) * 255) / height;
act_color = lv_color_mix(mcolor, gcolor, mix);
}
lv_draw_fill(&edge_top_area, mask, act_color, opa);
}

if(mid_top_refr != 0) {
if(mcolor.full == gcolor.full)
act_color = mcolor;
else {
mix = (uint32_t)((uint32_t)(coords->y2 - mid_top_area.y1) * 255) / height;
act_color = lv_color_mix(mcolor, gcolor, mix);
}
lv_draw_fill(&mid_top_area, mask, act_color, opa);
}

if(mid_bot_refr != 0) {
if(mcolor.full == gcolor.full)
act_color = mcolor;
else {
mix = (uint32_t)((uint32_t)(coords->y2 - mid_bot_area.y1) * 255) / height;
act_color = lv_color_mix(mcolor, gcolor, mix);
}
lv_draw_fill(&mid_bot_area, mask, act_color, opa);
}

if(edge_bot_refr != 0) {

if(mcolor.full == gcolor.full)
act_color = mcolor;
else {
mix = (uint32_t)((uint32_t)(coords->y2 - edge_bot_area.y1) * 255) / height;
act_color = lv_color_mix(mcolor, gcolor, mix);
}
lv_draw_fill(&edge_bot_area, mask, act_color, opa);
}


lv_area_set(&mid_bot_area, lb_origo.x + LV_CIRC_OCT4_X(cir), lb_origo.y + LV_CIRC_OCT4_Y(cir),
rb_origo.x + LV_CIRC_OCT1_X(cir), rb_origo.y + LV_CIRC_OCT1_Y(cir));

lv_area_set(&edge_bot_area, lb_origo.x + LV_CIRC_OCT3_X(cir), lb_origo.y + LV_CIRC_OCT3_Y(cir),
rb_origo.x + LV_CIRC_OCT2_X(cir), rb_origo.y + LV_CIRC_OCT2_Y(cir));

lv_area_set(&mid_top_area, lt_origo.x + LV_CIRC_OCT5_X(cir), lt_origo.y + LV_CIRC_OCT5_Y(cir),
rt_origo.x + LV_CIRC_OCT8_X(cir), rt_origo.y + LV_CIRC_OCT8_Y(cir));

lv_area_set(&edge_top_area, lt_origo.x + LV_CIRC_OCT6_X(cir), lt_origo.y + LV_CIRC_OCT6_Y(cir),
rt_origo.x + LV_CIRC_OCT7_X(cir), rt_origo.y + LV_CIRC_OCT7_Y(cir));

lv_circ_next(&cir, &cir_tmp);
}

if(mcolor.full == gcolor.full)
act_color = mcolor;
else {
mix = (uint32_t)((uint32_t)(coords->y2 - edge_top_area.y1) * 255) / height;
act_color = lv_color_mix(mcolor, gcolor, mix);
}
lv_draw_fill(&edge_top_area, mask, act_color, opa);

if(edge_top_area.y1 != mid_top_area.y1) {

if(mcolor.full == gcolor.full)
act_color = mcolor;
else {
mix = (uint32_t)((uint32_t)(coords->y2 - mid_top_area.y1) * 255) / height;
act_color = lv_color_mix(mcolor, gcolor, mix);
}
lv_draw_fill(&mid_top_area, mask, act_color, opa);
}

if(mcolor.full == gcolor.full)
act_color = mcolor;
else {
mix = (uint32_t)((uint32_t)(coords->y2 - mid_bot_area.y1) * 255) / height;
act_color = lv_color_mix(mcolor, gcolor, mix);
}
lv_draw_fill(&mid_bot_area, mask, act_color, opa);

if(edge_bot_area.y1 != mid_bot_area.y1) {

if(mcolor.full == gcolor.full)
act_color = mcolor;
else {
mix = (uint32_t)((uint32_t)(coords->y2 - edge_bot_area.y1) * 255) / height;
act_color = lv_color_mix(mcolor, gcolor, mix);
}
lv_draw_fill(&edge_bot_area, mask, act_color, opa);
}

#if LV_ANTIALIAS
if(aa) {

edge_top_area.x1 = coords->x1 + radius + 2;
edge_top_area.x2 = coords->x2 - radius - 2;
edge_top_area.y1 = coords->y1;
edge_top_area.y2 = coords->y1;
lv_draw_fill(&edge_top_area, mask, style->body.main_color, opa);

edge_top_area.y1 = coords->y2;
edge_top_area.y2 = coords->y2;
lv_draw_fill(&edge_top_area, mask, style->body.grad_color, opa);


out_y_seg_end = cir.y;
lv_coord_t seg_size = out_y_seg_end - out_y_seg_start;
lv_point_t aa_p;

aa_p.x = out_x_last;
aa_p.y = out_y_seg_start;

mix = (uint32_t)((uint32_t)(radius - out_x_last) * 255) / height;
aa_color_hor_bottom = lv_color_mix(gcolor, mcolor, mix);
aa_color_hor_top = lv_color_mix(mcolor, gcolor, mix);

lv_coord_t i;
for(i = 0; i < seg_size; i++) {
lv_opa_t aa_opa = opa - lv_draw_aa_get_opa(seg_size, i, opa);
lv_draw_px(rb_origo.x + LV_CIRC_OCT2_X(aa_p) + i, rb_origo.y + LV_CIRC_OCT2_Y(aa_p) + 1, mask,
aa_color_hor_top, aa_opa);
lv_draw_px(lb_origo.x + LV_CIRC_OCT3_X(aa_p) - i, lb_origo.y + LV_CIRC_OCT3_Y(aa_p) + 1, mask,
aa_color_hor_top, aa_opa);
lv_draw_px(lt_origo.x + LV_CIRC_OCT6_X(aa_p) - i, lt_origo.y + LV_CIRC_OCT6_Y(aa_p) - 1, mask,
aa_color_hor_bottom, aa_opa);
lv_draw_px(rt_origo.x + LV_CIRC_OCT7_X(aa_p) + i, rt_origo.y + LV_CIRC_OCT7_Y(aa_p) - 1, mask,
aa_color_hor_bottom, aa_opa);

mix = (uint32_t)((uint32_t)(radius - out_y_seg_start + i) * 255) / height;
aa_color_ver = lv_color_mix(mcolor, gcolor, mix);
lv_draw_px(rb_origo.x + LV_CIRC_OCT1_X(aa_p) + 1, rb_origo.y + LV_CIRC_OCT1_Y(aa_p) + i, mask, aa_color_ver,
aa_opa);
lv_draw_px(lb_origo.x + LV_CIRC_OCT4_X(aa_p) - 1, lb_origo.y + LV_CIRC_OCT4_Y(aa_p) + i, mask, aa_color_ver,
aa_opa);

aa_color_ver = lv_color_mix(gcolor, mcolor, mix);
lv_draw_px(lt_origo.x + LV_CIRC_OCT5_X(aa_p) - 1, lt_origo.y + LV_CIRC_OCT5_Y(aa_p) - i, mask, aa_color_ver,
aa_opa);
lv_draw_px(rt_origo.x + LV_CIRC_OCT8_X(aa_p) + 1, rt_origo.y + LV_CIRC_OCT8_Y(aa_p) - i, mask, aa_color_ver,
aa_opa);
}


if(LV_MATH_ABS(aa_p.x - aa_p.y) == seg_size) {
aa_p.x = out_x_last;
aa_p.y = out_x_last;

mix = (uint32_t)((uint32_t)(out_x_last)*255) / height;
aa_color_hor_top = lv_color_mix(gcolor, mcolor, mix);
aa_color_hor_bottom = lv_color_mix(mcolor, gcolor, mix);

lv_opa_t aa_opa = opa >> 1;
lv_draw_px(rb_origo.x + LV_CIRC_OCT2_X(aa_p), rb_origo.y + LV_CIRC_OCT2_Y(aa_p), mask, aa_color_hor_bottom,
aa_opa);
lv_draw_px(lb_origo.x + LV_CIRC_OCT4_X(aa_p), lb_origo.y + LV_CIRC_OCT4_Y(aa_p), mask, aa_color_hor_bottom,
aa_opa);
lv_draw_px(lt_origo.x + LV_CIRC_OCT6_X(aa_p), lt_origo.y + LV_CIRC_OCT6_Y(aa_p), mask, aa_color_hor_top,
aa_opa);
lv_draw_px(rt_origo.x + LV_CIRC_OCT8_X(aa_p), rt_origo.y + LV_CIRC_OCT8_Y(aa_p), mask, aa_color_hor_top,
aa_opa);
}
}
#endif
}








static void lv_draw_rect_border_straight(const lv_area_t * coords, const lv_area_t * mask, const lv_style_t * style,
lv_opa_t opa_scale)
{
uint16_t radius = style->body.radius;
bool aa = lv_disp_get_antialiasing(lv_refr_get_disp_refreshing());

lv_coord_t width = lv_area_get_width(coords);
lv_coord_t height = lv_area_get_height(coords);
lv_coord_t bwidth = style->body.border.width;
lv_opa_t opa = opa_scale == LV_OPA_COVER ? style->body.border.opa
: (uint16_t)((uint16_t)style->body.border.opa * opa_scale) >> 8;
lv_border_part_t part = style->body.border.part;
lv_color_t color = style->body.border.color;
lv_area_t work_area;
lv_coord_t length_corr = 0;
lv_coord_t corner_size = 0;


bwidth--;

radius = lv_draw_cont_radius_corr(radius, width, height);

if(radius < bwidth) {
length_corr = bwidth - radius - aa;
corner_size = bwidth;
} else {
corner_size = radius + aa;
}


if(style->body.radius == 0) {

if(part & LV_BORDER_TOP) {
work_area.x1 = coords->x1;
work_area.x2 = coords->x2;
work_area.y1 = coords->y1;
work_area.y2 = coords->y1 + bwidth;
lv_draw_fill(&work_area, mask, color, opa);
}


if(part & LV_BORDER_RIGHT) {
work_area.x1 = coords->x2 - bwidth;
work_area.x2 = coords->x2;
work_area.y1 = coords->y1 + (part & LV_BORDER_TOP ? bwidth + 1 : 0);
work_area.y2 = coords->y2 - (part & LV_BORDER_BOTTOM ? bwidth + 1 : 0);
lv_draw_fill(&work_area, mask, color, opa);
}


if(part & LV_BORDER_LEFT) {
work_area.x1 = coords->x1;
work_area.x2 = coords->x1 + bwidth;
work_area.y1 = coords->y1 + (part & LV_BORDER_TOP ? bwidth + 1 : 0);
work_area.y2 = coords->y2 - (part & LV_BORDER_BOTTOM ? bwidth + 1 : 0);
lv_draw_fill(&work_area, mask, color, opa);
}


if(part & LV_BORDER_BOTTOM) {
work_area.x1 = coords->x1;
work_area.x2 = coords->x2;
work_area.y1 = coords->y2 - bwidth;
work_area.y2 = coords->y2;
lv_draw_fill(&work_area, mask, color, opa);
}
return;
}


corner_size++;


if(part & LV_BORDER_TOP)
work_area.y1 = coords->y1 + corner_size;
else
work_area.y1 = coords->y1 + radius;

if(part & LV_BORDER_BOTTOM)
work_area.y2 = coords->y2 - corner_size;
else
work_area.y2 = coords->y2 - radius;


if(part & LV_BORDER_LEFT) {
work_area.x1 = coords->x1;
work_area.x2 = work_area.x1 + bwidth;
lv_draw_fill(&work_area, mask, color, opa);
}


if(part & LV_BORDER_RIGHT) {
work_area.x2 = coords->x2;
work_area.x1 = work_area.x2 - bwidth;
lv_draw_fill(&work_area, mask, color, opa);
}

work_area.x1 = coords->x1 + corner_size - length_corr;
work_area.x2 = coords->x2 - corner_size + length_corr;


if(part & LV_BORDER_TOP) {
work_area.y1 = coords->y1;
work_area.y2 = coords->y1 + bwidth;
lv_draw_fill(&work_area, mask, color, opa);
}


if(part & LV_BORDER_BOTTOM) {
work_area.y2 = coords->y2;
work_area.y1 = work_area.y2 - bwidth;
lv_draw_fill(&work_area, mask, color, opa);
}


if(length_corr != 0) {

if((part & LV_BORDER_TOP) && (part & LV_BORDER_LEFT)) {
work_area.x1 = coords->x1;
work_area.x2 = coords->x1 + radius + aa;
work_area.y1 = coords->y1 + radius + 1 + aa;
work_area.y2 = coords->y1 + bwidth;
lv_draw_fill(&work_area, mask, color, opa);
}


if((part & LV_BORDER_TOP) && (part & LV_BORDER_RIGHT)) {
work_area.x1 = coords->x2 - radius - aa;
work_area.x2 = coords->x2;
work_area.y1 = coords->y1 + radius + 1 + aa;
work_area.y2 = coords->y1 + bwidth;
lv_draw_fill(&work_area, mask, color, opa);
}


if((part & LV_BORDER_BOTTOM) && (part & LV_BORDER_LEFT)) {
work_area.x1 = coords->x1;
work_area.x2 = coords->x1 + radius + aa;
work_area.y1 = coords->y2 - bwidth;
work_area.y2 = coords->y2 - radius - 1 - aa;
lv_draw_fill(&work_area, mask, color, opa);
}


if((part & LV_BORDER_BOTTOM) && (part & LV_BORDER_RIGHT)) {
work_area.x1 = coords->x2 - radius - aa;
work_area.x2 = coords->x2;
work_area.y1 = coords->y2 - bwidth;
work_area.y2 = coords->y2 - radius - 1 - aa;
lv_draw_fill(&work_area, mask, color, opa);
}
}
}








static void lv_draw_rect_border_corner(const lv_area_t * coords, const lv_area_t * mask, const lv_style_t * style,
lv_opa_t opa_scale)
{
uint16_t radius = style->body.radius;
bool aa = lv_disp_get_antialiasing(lv_refr_get_disp_refreshing());
lv_coord_t bwidth = style->body.border.width;
lv_color_t color = style->body.border.color;
lv_border_part_t part = style->body.border.part;
lv_opa_t opa = opa_scale == LV_OPA_COVER ? style->body.border.opa
: (uint16_t)((uint16_t)style->body.border.opa * opa_scale) >> 8;

bwidth--;

#if LV_ANTIALIAS
if(aa) bwidth--; 
#endif

lv_coord_t width = lv_area_get_width(coords);
lv_coord_t height = lv_area_get_height(coords);

radius = lv_draw_cont_radius_corr(radius, width, height);

lv_point_t lt_origo; 
lv_point_t lb_origo; 
lv_point_t rt_origo; 
lv_point_t rb_origo; 

lt_origo.x = coords->x1 + radius + aa;
lt_origo.y = coords->y1 + radius + aa;

lb_origo.x = coords->x1 + radius + aa;
lb_origo.y = coords->y2 - radius - aa;

rt_origo.x = coords->x2 - radius - aa;
rt_origo.y = coords->y1 + radius + aa;

rb_origo.x = coords->x2 - radius - aa;
rb_origo.y = coords->y2 - radius - aa;

lv_point_t cir_out;
lv_coord_t tmp_out;
lv_circ_init(&cir_out, &tmp_out, radius);

lv_point_t cir_in;
lv_coord_t tmp_in;
lv_coord_t radius_in = radius - bwidth;

if(radius_in < 0) {
radius_in = 0;
}

lv_circ_init(&cir_in, &tmp_in, radius_in);

lv_area_t circ_area;
lv_coord_t act_w1;
lv_coord_t act_w2;

#if LV_ANTIALIAS

lv_coord_t out_y_seg_start = 0;
lv_coord_t out_y_seg_end = 0;
lv_coord_t out_x_last = radius;

lv_coord_t in_y_seg_start = 0;
lv_coord_t in_y_seg_end = 0;
lv_coord_t in_x_last = radius - bwidth;
#endif

while(cir_out.y <= cir_out.x) {


if(cir_in.y < cir_in.x) {
act_w1 = cir_out.x - cir_in.x;
act_w2 = act_w1;
} else {
act_w1 = cir_out.x - cir_out.y;
act_w2 = act_w1 - 1;
}

#if LV_ANTIALIAS
if(aa) {

if(out_x_last != cir_out.x) {
out_y_seg_end = cir_out.y;
lv_coord_t seg_size = out_y_seg_end - out_y_seg_start;
lv_point_t aa_p;

aa_p.x = out_x_last;
aa_p.y = out_y_seg_start;

lv_coord_t i;
for(i = 0; i < seg_size; i++) {
lv_opa_t aa_opa;

if(seg_size > CIRCLE_AA_NON_LINEAR_OPA_THRESHOLD) { 

aa_opa = antialias_get_opa_circ(seg_size, i, opa);
} else {
aa_opa = opa - lv_draw_aa_get_opa(seg_size, i, opa);
}

if((part & LV_BORDER_BOTTOM) && (part & LV_BORDER_RIGHT)) {
lv_draw_px(rb_origo.x + LV_CIRC_OCT1_X(aa_p) + 1, rb_origo.y + LV_CIRC_OCT1_Y(aa_p) + i, mask,
style->body.border.color, aa_opa);
lv_draw_px(rb_origo.x + LV_CIRC_OCT2_X(aa_p) + i, rb_origo.y + LV_CIRC_OCT2_Y(aa_p) + 1, mask,
style->body.border.color, aa_opa);
}

if((part & LV_BORDER_BOTTOM) && (part & LV_BORDER_LEFT)) {
lv_draw_px(lb_origo.x + LV_CIRC_OCT3_X(aa_p) - i, lb_origo.y + LV_CIRC_OCT3_Y(aa_p) + 1, mask,
style->body.border.color, aa_opa);
lv_draw_px(lb_origo.x + LV_CIRC_OCT4_X(aa_p) - 1, lb_origo.y + LV_CIRC_OCT4_Y(aa_p) + i, mask,
style->body.border.color, aa_opa);
}

if((part & LV_BORDER_TOP) && (part & LV_BORDER_LEFT)) {
lv_draw_px(lt_origo.x + LV_CIRC_OCT5_X(aa_p) - 1, lt_origo.y + LV_CIRC_OCT5_Y(aa_p) - i, mask,
style->body.border.color, aa_opa);
lv_draw_px(lt_origo.x + LV_CIRC_OCT6_X(aa_p) - i, lt_origo.y + LV_CIRC_OCT6_Y(aa_p) - 1, mask,
style->body.border.color, aa_opa);
}

if((part & LV_BORDER_TOP) && (part & LV_BORDER_RIGHT)) {
lv_draw_px(rt_origo.x + LV_CIRC_OCT7_X(aa_p) + i, rt_origo.y + LV_CIRC_OCT7_Y(aa_p) - 1, mask,
style->body.border.color, aa_opa);
lv_draw_px(rt_origo.x + LV_CIRC_OCT8_X(aa_p) + 1, rt_origo.y + LV_CIRC_OCT8_Y(aa_p) - i, mask,
style->body.border.color, aa_opa);
}
}

out_x_last = cir_out.x;
out_y_seg_start = out_y_seg_end;
}


if(in_x_last != cir_in.x) {
in_y_seg_end = cir_out.y;
lv_coord_t seg_size = in_y_seg_end - in_y_seg_start;
lv_point_t aa_p;

aa_p.x = in_x_last;
aa_p.y = in_y_seg_start;

lv_coord_t i;
for(i = 0; i < seg_size; i++) {
lv_opa_t aa_opa;

if(seg_size > CIRCLE_AA_NON_LINEAR_OPA_THRESHOLD) { 

aa_opa = opa - antialias_get_opa_circ(seg_size, i, opa);
} else {
aa_opa = lv_draw_aa_get_opa(seg_size, i, opa);
}

if((part & LV_BORDER_BOTTOM) && (part & LV_BORDER_RIGHT)) {
lv_draw_px(rb_origo.x + LV_CIRC_OCT1_X(aa_p) - 1, rb_origo.y + LV_CIRC_OCT1_Y(aa_p) + i, mask,
style->body.border.color, aa_opa);
}

if((part & LV_BORDER_BOTTOM) && (part & LV_BORDER_LEFT)) {
lv_draw_px(lb_origo.x + LV_CIRC_OCT3_X(aa_p) - i, lb_origo.y + LV_CIRC_OCT3_Y(aa_p) - 1, mask,
style->body.border.color, aa_opa);
}

if((part & LV_BORDER_TOP) && (part & LV_BORDER_LEFT)) {
lv_draw_px(lt_origo.x + LV_CIRC_OCT5_X(aa_p) + 1, lt_origo.y + LV_CIRC_OCT5_Y(aa_p) - i, mask,
style->body.border.color, aa_opa);
}

if((part & LV_BORDER_TOP) && (part & LV_BORDER_RIGHT)) {
lv_draw_px(rt_origo.x + LV_CIRC_OCT7_X(aa_p) + i, rt_origo.y + LV_CIRC_OCT7_Y(aa_p) + 1, mask,
style->body.border.color, aa_opa);
}


if(LV_CIRC_OCT1_X(aa_p) - 1 != LV_CIRC_OCT2_X(aa_p) + i) {
if((part & LV_BORDER_BOTTOM) && (part & LV_BORDER_RIGHT)) {
lv_draw_px(rb_origo.x + LV_CIRC_OCT2_X(aa_p) + i, rb_origo.y + LV_CIRC_OCT2_Y(aa_p) - 1,
mask, style->body.border.color, aa_opa);
}

if((part & LV_BORDER_BOTTOM) && (part & LV_BORDER_LEFT)) {
lv_draw_px(lb_origo.x + LV_CIRC_OCT4_X(aa_p) + 1, lb_origo.y + LV_CIRC_OCT4_Y(aa_p) + i,
mask, style->body.border.color, aa_opa);
}

if((part & LV_BORDER_TOP) && (part & LV_BORDER_LEFT)) {
lv_draw_px(lt_origo.x + LV_CIRC_OCT6_X(aa_p) - i, lt_origo.y + LV_CIRC_OCT6_Y(aa_p) + 1,
mask, style->body.border.color, aa_opa);
}

if((part & LV_BORDER_TOP) && (part & LV_BORDER_RIGHT)) {
lv_draw_px(rt_origo.x + LV_CIRC_OCT8_X(aa_p) - 1, rt_origo.y + LV_CIRC_OCT8_Y(aa_p) - i,
mask, style->body.border.color, aa_opa);
}
}
}

in_x_last = cir_in.x;
in_y_seg_start = in_y_seg_end;
}
}
#endif


if((part & LV_BORDER_BOTTOM) && (part & LV_BORDER_RIGHT)) {
circ_area.x1 = rb_origo.x + LV_CIRC_OCT1_X(cir_out) - act_w2;
circ_area.x2 = rb_origo.x + LV_CIRC_OCT1_X(cir_out);
circ_area.y1 = rb_origo.y + LV_CIRC_OCT1_Y(cir_out);
circ_area.y2 = rb_origo.y + LV_CIRC_OCT1_Y(cir_out);
lv_draw_fill(&circ_area, mask, color, opa);

circ_area.x1 = rb_origo.x + LV_CIRC_OCT2_X(cir_out);
circ_area.x2 = rb_origo.x + LV_CIRC_OCT2_X(cir_out);
circ_area.y1 = rb_origo.y + LV_CIRC_OCT2_Y(cir_out) - act_w1;
circ_area.y2 = rb_origo.y + LV_CIRC_OCT2_Y(cir_out);
lv_draw_fill(&circ_area, mask, color, opa);
}


if((part & LV_BORDER_BOTTOM) && (part & LV_BORDER_LEFT)) {
circ_area.x1 = lb_origo.x + LV_CIRC_OCT3_X(cir_out);
circ_area.x2 = lb_origo.x + LV_CIRC_OCT3_X(cir_out);
circ_area.y1 = lb_origo.y + LV_CIRC_OCT3_Y(cir_out) - act_w2;
circ_area.y2 = lb_origo.y + LV_CIRC_OCT3_Y(cir_out);
lv_draw_fill(&circ_area, mask, color, opa);

circ_area.x1 = lb_origo.x + LV_CIRC_OCT4_X(cir_out);
circ_area.x2 = lb_origo.x + LV_CIRC_OCT4_X(cir_out) + act_w1;
circ_area.y1 = lb_origo.y + LV_CIRC_OCT4_Y(cir_out);
circ_area.y2 = lb_origo.y + LV_CIRC_OCT4_Y(cir_out);
lv_draw_fill(&circ_area, mask, color, opa);
}


if((part & LV_BORDER_TOP) && (part & LV_BORDER_LEFT)) {
if(lb_origo.y + LV_CIRC_OCT4_Y(cir_out) > lt_origo.y + LV_CIRC_OCT5_Y(cir_out)) {

circ_area.x1 = lt_origo.x + LV_CIRC_OCT5_X(cir_out);
circ_area.x2 = lt_origo.x + LV_CIRC_OCT5_X(cir_out) + act_w2;
circ_area.y1 = lt_origo.y + LV_CIRC_OCT5_Y(cir_out);
circ_area.y2 = lt_origo.y + LV_CIRC_OCT5_Y(cir_out);
lv_draw_fill(&circ_area, mask, color, opa);
}

circ_area.x1 = lt_origo.x + LV_CIRC_OCT6_X(cir_out);
circ_area.x2 = lt_origo.x + LV_CIRC_OCT6_X(cir_out);
circ_area.y1 = lt_origo.y + LV_CIRC_OCT6_Y(cir_out);
circ_area.y2 = lt_origo.y + LV_CIRC_OCT6_Y(cir_out) + act_w1;
lv_draw_fill(&circ_area, mask, color, opa);
}


if((part & LV_BORDER_TOP) && (part & LV_BORDER_RIGHT)) {
circ_area.x1 = rt_origo.x + LV_CIRC_OCT7_X(cir_out);
circ_area.x2 = rt_origo.x + LV_CIRC_OCT7_X(cir_out);
circ_area.y1 = rt_origo.y + LV_CIRC_OCT7_Y(cir_out);
circ_area.y2 = rt_origo.y + LV_CIRC_OCT7_Y(cir_out) + act_w2;
lv_draw_fill(&circ_area, mask, color, opa);


if(rb_origo.y + LV_CIRC_OCT1_Y(cir_out) > rt_origo.y + LV_CIRC_OCT8_Y(cir_out)) {
circ_area.x1 = rt_origo.x + LV_CIRC_OCT8_X(cir_out) - act_w1;
circ_area.x2 = rt_origo.x + LV_CIRC_OCT8_X(cir_out);
circ_area.y1 = rt_origo.y + LV_CIRC_OCT8_Y(cir_out);
circ_area.y2 = rt_origo.y + LV_CIRC_OCT8_Y(cir_out);
lv_draw_fill(&circ_area, mask, color, opa);
}
}
lv_circ_next(&cir_out, &tmp_out);



if(cir_in.y < cir_in.x) {
lv_circ_next(&cir_in, &tmp_in);
}
}

#if LV_ANTIALIAS
if(aa) {

out_y_seg_end = cir_out.y;
lv_coord_t seg_size = out_y_seg_end - out_y_seg_start;
lv_point_t aa_p;

aa_p.x = out_x_last;
aa_p.y = out_y_seg_start;

lv_coord_t i;
for(i = 0; i < seg_size; i++) {
lv_opa_t aa_opa = opa - lv_draw_aa_get_opa(seg_size, i, opa);
if((part & LV_BORDER_BOTTOM) && (part & LV_BORDER_RIGHT)) {
lv_draw_px(rb_origo.x + LV_CIRC_OCT1_X(aa_p) + 1, rb_origo.y + LV_CIRC_OCT1_Y(aa_p) + i, mask,
style->body.border.color, aa_opa);
lv_draw_px(rb_origo.x + LV_CIRC_OCT2_X(aa_p) + i, rb_origo.y + LV_CIRC_OCT2_Y(aa_p) + 1, mask,
style->body.border.color, aa_opa);
}

if((part & LV_BORDER_BOTTOM) && (part & LV_BORDER_LEFT)) {
lv_draw_px(lb_origo.x + LV_CIRC_OCT3_X(aa_p) - i, lb_origo.y + LV_CIRC_OCT3_Y(aa_p) + 1, mask,
style->body.border.color, aa_opa);
lv_draw_px(lb_origo.x + LV_CIRC_OCT4_X(aa_p) - 1, lb_origo.y + LV_CIRC_OCT4_Y(aa_p) + i, mask,
style->body.border.color, aa_opa);
}

if((part & LV_BORDER_TOP) && (part & LV_BORDER_LEFT)) {
lv_draw_px(lt_origo.x + LV_CIRC_OCT5_X(aa_p) - 1, lt_origo.y + LV_CIRC_OCT5_Y(aa_p) - i, mask,
style->body.border.color, aa_opa);
lv_draw_px(lt_origo.x + LV_CIRC_OCT6_X(aa_p) - i, lt_origo.y + LV_CIRC_OCT6_Y(aa_p) - 1, mask,
style->body.border.color, aa_opa);
}

if((part & LV_BORDER_TOP) && (part & LV_BORDER_RIGHT)) {
lv_draw_px(rt_origo.x + LV_CIRC_OCT7_X(aa_p) + i, rt_origo.y + LV_CIRC_OCT7_Y(aa_p) - 1, mask,
style->body.border.color, aa_opa);
lv_draw_px(rt_origo.x + LV_CIRC_OCT8_X(aa_p) + 1, rt_origo.y + LV_CIRC_OCT8_Y(aa_p) - i, mask,
style->body.border.color, aa_opa);
}
}


if(LV_MATH_ABS(aa_p.x - aa_p.y) == seg_size) {
aa_p.x = out_x_last;
aa_p.y = out_x_last;

lv_opa_t aa_opa = opa >> 1;

if((part & LV_BORDER_BOTTOM) && (part & LV_BORDER_RIGHT)) {
lv_draw_px(rb_origo.x + LV_CIRC_OCT2_X(aa_p), rb_origo.y + LV_CIRC_OCT2_Y(aa_p), mask,
style->body.border.color, aa_opa);
}

if((part & LV_BORDER_BOTTOM) && (part & LV_BORDER_LEFT)) {
lv_draw_px(lb_origo.x + LV_CIRC_OCT4_X(aa_p), lb_origo.y + LV_CIRC_OCT4_Y(aa_p), mask,
style->body.border.color, aa_opa);
}

if((part & LV_BORDER_TOP) && (part & LV_BORDER_LEFT)) {
lv_draw_px(lt_origo.x + LV_CIRC_OCT6_X(aa_p), lt_origo.y + LV_CIRC_OCT6_Y(aa_p), mask,
style->body.border.color, aa_opa);
}

if((part & LV_BORDER_TOP) && (part & LV_BORDER_RIGHT)) {
lv_draw_px(rt_origo.x + LV_CIRC_OCT8_X(aa_p), rt_origo.y + LV_CIRC_OCT8_Y(aa_p), mask,
style->body.border.color, aa_opa);
}
}


in_y_seg_end = cir_in.y;
aa_p.x = in_x_last;
aa_p.y = in_y_seg_start;
seg_size = in_y_seg_end - in_y_seg_start;

for(i = 0; i < seg_size; i++) {
lv_opa_t aa_opa = lv_draw_aa_get_opa(seg_size, i, opa);
if((part & LV_BORDER_BOTTOM) && (part & LV_BORDER_RIGHT)) {
lv_draw_px(rb_origo.x + LV_CIRC_OCT1_X(aa_p) - 1, rb_origo.y + LV_CIRC_OCT1_Y(aa_p) + i, mask,
style->body.border.color, aa_opa);
}

if((part & LV_BORDER_BOTTOM) && (part & LV_BORDER_LEFT)) {
lv_draw_px(lb_origo.x + LV_CIRC_OCT3_X(aa_p) - i, lb_origo.y + LV_CIRC_OCT3_Y(aa_p) - 1, mask,
style->body.border.color, aa_opa);
}

if((part & LV_BORDER_TOP) && (part & LV_BORDER_LEFT)) {
lv_draw_px(lt_origo.x + LV_CIRC_OCT5_X(aa_p) + 1, lt_origo.y + LV_CIRC_OCT5_Y(aa_p) - i, mask,
style->body.border.color, aa_opa);
}

if((part & LV_BORDER_TOP) && (part & LV_BORDER_RIGHT)) {
lv_draw_px(rt_origo.x + LV_CIRC_OCT7_X(aa_p) + i, rt_origo.y + LV_CIRC_OCT7_Y(aa_p) + 1, mask,
style->body.border.color, aa_opa);
}

if(LV_CIRC_OCT1_X(aa_p) - 1 != LV_CIRC_OCT2_X(aa_p) + i) {
if((part & LV_BORDER_BOTTOM) && (part & LV_BORDER_RIGHT)) {
lv_draw_px(rb_origo.x + LV_CIRC_OCT2_X(aa_p) + i, rb_origo.y + LV_CIRC_OCT2_Y(aa_p) - 1, mask,
style->body.border.color, aa_opa);
}

if((part & LV_BORDER_BOTTOM) && (part & LV_BORDER_LEFT)) {
lv_draw_px(lb_origo.x + LV_CIRC_OCT4_X(aa_p) + 1, lb_origo.y + LV_CIRC_OCT4_Y(aa_p) + i, mask,
style->body.border.color, aa_opa);
}

if((part & LV_BORDER_TOP) && (part & LV_BORDER_LEFT)) {
lv_draw_px(lt_origo.x + LV_CIRC_OCT6_X(aa_p) - i, lt_origo.y + LV_CIRC_OCT6_Y(aa_p) + 1, mask,
style->body.border.color, aa_opa);
}

if((part & LV_BORDER_TOP) && (part & LV_BORDER_RIGHT)) {
lv_draw_px(rt_origo.x + LV_CIRC_OCT8_X(aa_p) - 1, rt_origo.y + LV_CIRC_OCT8_Y(aa_p) - i, mask,
style->body.border.color, aa_opa);
}
}
}
}
#endif
}

#if LV_USE_SHADOW







static void lv_draw_shadow(const lv_area_t * coords, const lv_area_t * mask, const lv_style_t * style,
lv_opa_t opa_scale)
{

lv_coord_t radius = style->body.radius;
lv_coord_t width = lv_area_get_width(coords);
lv_coord_t height = lv_area_get_height(coords);
radius = lv_draw_cont_radius_corr(radius, width, height);
lv_area_t area_tmp;


lv_area_copy(&area_tmp, coords);
area_tmp.x1 += radius;
area_tmp.x2 -= radius;
if(lv_area_is_in(mask, &area_tmp) != false) return;


lv_area_copy(&area_tmp, coords);
area_tmp.y1 += radius;
area_tmp.y2 -= radius;
if(lv_area_is_in(mask, &area_tmp) != false) return;

if(style->body.shadow.type == LV_SHADOW_FULL) {
lv_draw_shadow_full(coords, mask, style, opa_scale);
} else if(style->body.shadow.type == LV_SHADOW_BOTTOM) {
lv_draw_shadow_bottom(coords, mask, style, opa_scale);
}
}

static void lv_draw_shadow_full(const lv_area_t * coords, const lv_area_t * mask, const lv_style_t * style,
lv_opa_t opa_scale)
{












bool aa = lv_disp_get_antialiasing(lv_refr_get_disp_refreshing());

lv_coord_t radius = style->body.radius;
lv_coord_t swidth = style->body.shadow.width;

lv_coord_t width = lv_area_get_width(coords);
lv_coord_t height = lv_area_get_height(coords);

radius = lv_draw_cont_radius_corr(radius, width, height);

radius += aa;


int16_t filter_width = 2 * swidth + 1;
uint32_t curve_x_size = ((radius + swidth + 1) + 3) & ~0x3; 
curve_x_size *= sizeof(lv_coord_t);
uint32_t line_1d_blur_size = (filter_width + 3) & ~0x3; 
line_1d_blur_size *= sizeof(uint32_t);
uint32_t line_2d_blur_size = ((radius + swidth + 1) + 3) & ~0x3; 
line_2d_blur_size *= sizeof(lv_opa_t);

uint8_t * draw_buf = lv_draw_get_buf(curve_x_size + line_1d_blur_size + line_2d_blur_size);


lv_coord_t * curve_x = (lv_coord_t *)&draw_buf[0]; 
uint32_t * line_1d_blur = (uint32_t *)&draw_buf[curve_x_size];
lv_opa_t * line_2d_blur = (lv_opa_t *)&draw_buf[curve_x_size + line_1d_blur_size];

memset(curve_x, 0, curve_x_size);
lv_point_t circ;
lv_coord_t circ_tmp;
lv_circ_init(&circ, &circ_tmp, radius);
while(lv_circ_cont(&circ)) {
curve_x[LV_CIRC_OCT1_Y(circ)] = LV_CIRC_OCT1_X(circ);
curve_x[LV_CIRC_OCT2_Y(circ)] = LV_CIRC_OCT2_X(circ);
lv_circ_next(&circ, &circ_tmp);
}
int16_t line;

lv_opa_t opa = opa_scale == LV_OPA_COVER ? style->body.opa : (uint16_t)((uint16_t)style->body.opa * opa_scale) >> 8;
for(line = 0; line < filter_width; line++) {
line_1d_blur[line] = (uint32_t)((uint32_t)(filter_width - line) * (opa * 2) << SHADOW_OPA_EXTRA_PRECISION) /
(filter_width * filter_width);
}

uint16_t col;

lv_point_t point_rt;
lv_point_t point_rb;
lv_point_t point_lt;
lv_point_t point_lb;
lv_point_t ofs_rb;
lv_point_t ofs_rt;
lv_point_t ofs_lb;
lv_point_t ofs_lt;
ofs_rb.x = coords->x2 - radius - aa;
ofs_rb.y = coords->y2 - radius - aa;

ofs_rt.x = coords->x2 - radius - aa;
ofs_rt.y = coords->y1 + radius + aa;

ofs_lb.x = coords->x1 + radius + aa;
ofs_lb.y = coords->y2 - radius - aa;

ofs_lt.x = coords->x1 + radius + aa;
ofs_lt.y = coords->y1 + radius + aa;
bool line_ready;
for(line = 0; line <= radius + swidth; line++) { 
line_ready = false;
for(col = 0; col <= radius + swidth; col++) { 



int16_t line_rel;
uint32_t px_opa_sum = 0;
for(line_rel = -swidth; line_rel <= swidth; line_rel++) {

int16_t col_rel;
if(line + line_rel < 0) { 
col_rel = radius - curve_x[line] - col;
} else if(line + line_rel > radius) { 
break;
} else { 
col_rel = curve_x[line + line_rel] - curve_x[line] - col;
}


if(col_rel < -swidth) { 
if(line_rel == -swidth)
line_ready = true; 

break; 
} else if(col_rel > swidth)
px_opa_sum += line_1d_blur[0]; 
else
px_opa_sum += line_1d_blur[swidth - col_rel]; 
}

line_2d_blur[col] = px_opa_sum >> SHADOW_OPA_EXTRA_PRECISION;
if(line_ready) {
col++; 
break;
}
}


point_rt.x = curve_x[line] + ofs_rt.x + 1;
point_rt.y = ofs_rt.y - line;

point_rb.x = curve_x[line] + ofs_rb.x + 1;
point_rb.y = ofs_rb.y + line;

point_lt.x = ofs_lt.x - curve_x[line] - 1;
point_lt.y = ofs_lt.y - line;

point_lb.x = ofs_lb.x - curve_x[line] - 1;
point_lb.y = ofs_lb.y + line;

uint16_t d;
for(d = 1; d < col; d++) {

if(point_lt.x < ofs_lt.x && point_lt.y < ofs_lt.y) {
lv_draw_px(point_lt.x, point_lt.y, mask, style->body.shadow.color, line_2d_blur[d]);
}

if(point_lb.x < ofs_lb.x && point_lb.y > ofs_lb.y) {
lv_draw_px(point_lb.x, point_lb.y, mask, style->body.shadow.color, line_2d_blur[d]);
}

if(point_rt.x > ofs_rt.x && point_rt.y < ofs_rt.y) {
lv_draw_px(point_rt.x, point_rt.y, mask, style->body.shadow.color, line_2d_blur[d]);
}

if(point_rb.x > ofs_rb.x && point_rb.y > ofs_rb.y) {
lv_draw_px(point_rb.x, point_rb.y, mask, style->body.shadow.color, line_2d_blur[d]);
}

point_rb.x++;
point_lb.x--;

point_rt.x++;
point_lt.x--;
}




if(line == 0) lv_draw_shadow_full_straight(coords, mask, style, line_2d_blur);
}
}

static void lv_draw_shadow_bottom(const lv_area_t * coords, const lv_area_t * mask, const lv_style_t * style,
lv_opa_t opa_scale)
{
bool aa = lv_disp_get_antialiasing(lv_refr_get_disp_refreshing());
lv_coord_t radius = style->body.radius;
lv_coord_t swidth = style->body.shadow.width;
lv_coord_t width = lv_area_get_width(coords);
lv_coord_t height = lv_area_get_height(coords);

radius = lv_draw_cont_radius_corr(radius, width, height);
radius += aa * SHADOW_BOTTOM_AA_EXTRA_RADIUS;
swidth += aa;

uint32_t curve_x_size = ((radius + 1) + 3) & ~0x3; 
curve_x_size *= sizeof(lv_coord_t);
lv_opa_t line_1d_blur_size = (swidth + 3) & ~0x3; 
line_1d_blur_size *= sizeof(lv_opa_t);

uint8_t * draw_buf = lv_draw_get_buf(curve_x_size + line_1d_blur_size);


lv_coord_t * curve_x = (lv_coord_t *)&draw_buf[0]; 
lv_opa_t * line_1d_blur = (lv_opa_t *)&draw_buf[curve_x_size];

lv_point_t circ;
lv_coord_t circ_tmp;
lv_circ_init(&circ, &circ_tmp, radius);
while(lv_circ_cont(&circ)) {
curve_x[LV_CIRC_OCT1_Y(circ)] = LV_CIRC_OCT1_X(circ);
curve_x[LV_CIRC_OCT2_Y(circ)] = LV_CIRC_OCT2_X(circ);
lv_circ_next(&circ, &circ_tmp);
}

int16_t col;

lv_opa_t opa = opa_scale == LV_OPA_COVER ? style->body.opa : (uint16_t)((uint16_t)style->body.opa * opa_scale) >> 8;
for(col = 0; col < swidth; col++) {
line_1d_blur[col] = (uint32_t)((uint32_t)(swidth - col) * opa / 2) / (swidth);
}

lv_point_t point_l;
lv_point_t point_r;
lv_area_t area_mid;
lv_point_t ofs_l;
lv_point_t ofs_r;

ofs_l.x = coords->x1 + radius;
ofs_l.y = coords->y2 - radius + 1 - aa;

ofs_r.x = coords->x2 - radius;
ofs_r.y = coords->y2 - radius + 1 - aa;

for(col = 0; col <= radius; col++) {
point_l.x = ofs_l.x - col;
point_l.y = ofs_l.y + curve_x[col];

point_r.x = ofs_r.x + col;
point_r.y = ofs_r.y + curve_x[col];

lv_opa_t px_opa;
int16_t diff = col == 0 ? 0 : curve_x[col - 1] - curve_x[col];
uint16_t d;
for(d = 0; d < swidth; d++) {


if(diff == 0) {
px_opa = line_1d_blur[d];
} else {
px_opa = (uint16_t)((uint16_t)line_1d_blur[d] + line_1d_blur[d - diff]) >> 1;
}
lv_draw_px(point_l.x, point_l.y, mask, style->body.shadow.color, px_opa);
point_l.y++;


if(point_r.x > ofs_l.x) {
lv_draw_px(point_r.x, point_r.y, mask, style->body.shadow.color, px_opa);
}
point_r.y++;
}
}

area_mid.x1 = ofs_l.x + 1;
area_mid.y1 = ofs_l.y + radius;
area_mid.x2 = ofs_r.x - 1;
area_mid.y2 = area_mid.y1;

uint16_t d;
for(d = 0; d < swidth; d++) {
lv_draw_fill(&area_mid, mask, style->body.shadow.color, line_1d_blur[d]);
area_mid.y1++;
area_mid.y2++;
}
}

static void lv_draw_shadow_full_straight(const lv_area_t * coords, const lv_area_t * mask, const lv_style_t * style,
const lv_opa_t * map)
{
bool aa = lv_disp_get_antialiasing(lv_refr_get_disp_refreshing());
lv_coord_t radius = style->body.radius;
lv_coord_t swidth = style->body.shadow.width;
lv_coord_t width = lv_area_get_width(coords);
lv_coord_t height = lv_area_get_height(coords);

radius = lv_draw_cont_radius_corr(radius, width, height);
radius += aa;

lv_area_t right_area;
right_area.x1 = coords->x2 + 1 - aa;
right_area.y1 = coords->y1 + radius + aa;
right_area.x2 = right_area.x1;
right_area.y2 = coords->y2 - radius - aa;

lv_area_t left_area;
left_area.x1 = coords->x1 - 1 + aa;
left_area.y1 = coords->y1 + radius + aa;
left_area.x2 = left_area.x1;
left_area.y2 = coords->y2 - radius - aa;

lv_area_t top_area;
top_area.x1 = coords->x1 + radius + aa;
top_area.y1 = coords->y1 - 1 + aa;
top_area.x2 = coords->x2 - radius - aa;
top_area.y2 = top_area.y1;

lv_area_t bottom_area;
bottom_area.x1 = coords->x1 + radius + aa;
bottom_area.y1 = coords->y2 + 1 - aa;
bottom_area.x2 = coords->x2 - radius - aa;
bottom_area.y2 = bottom_area.y1;

lv_opa_t opa_act;
int16_t d;
for(d = 1 ; d <= swidth ; d++) {
opa_act = map[d];

lv_draw_fill(&right_area, mask, style->body.shadow.color, opa_act);
right_area.x1++;
right_area.x2++;

lv_draw_fill(&left_area, mask, style->body.shadow.color, opa_act);
left_area.x1--;
left_area.x2--;

lv_draw_fill(&top_area, mask, style->body.shadow.color, opa_act);
top_area.y1--;
top_area.y2--;

lv_draw_fill(&bottom_area, mask, style->body.shadow.color, opa_act);
bottom_area.y1++;
bottom_area.y2++;
}
}

#endif

static uint16_t lv_draw_cont_radius_corr(uint16_t r, lv_coord_t w, lv_coord_t h)
{
bool aa = lv_disp_get_antialiasing(lv_refr_get_disp_refreshing());

if(r >= (w >> 1)) {
r = (w >> 1);
if(r != 0) r--;
}
if(r >= (h >> 1)) {
r = (h >> 1);
if(r != 0) r--;
}

if(r > 0) r -= aa;

return r;
}

#if LV_ANTIALIAS









static lv_opa_t antialias_get_opa_circ(lv_coord_t seg, lv_coord_t px_id, lv_opa_t opa)
{

static const lv_opa_t opa_map2[2] = {210, 80};
static const lv_opa_t opa_map3[3] = {230, 150, 60};
static const lv_opa_t opa_map4[4] = {235, 185, 125, 50};
static const lv_opa_t opa_map8[8] = {250, 242, 219, 191, 158, 117, 76, 40};

#if CIRCLE_AA_NON_LINEAR_OPA_THRESHOLD < 1
if(seg == 1) return 170;
#endif

#if CIRCLE_AA_NON_LINEAR_OPA_THRESHOLD < 2
if(seg == 2) return (opa_map2[px_id] * opa) >> 8;
#endif

#if CIRCLE_AA_NON_LINEAR_OPA_THRESHOLD < 3
if(seg == 3) return (opa_map3[px_id] * opa) >> 8;
#endif

#if CIRCLE_AA_NON_LINEAR_OPA_THRESHOLD < 4
if(seg == 4) return (opa_map4[px_id] * opa) >> 8;
#endif

uint8_t id = (uint32_t)((uint32_t)px_id * (sizeof(opa_map8) - 1)) / (seg - 1);
return (uint32_t)((uint32_t)opa_map8[id] * opa) >> 8;
}

#endif
