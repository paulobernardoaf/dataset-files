







#include <stdlib.h>
#include "lv_canvas.h"
#include "../lv_core/lv_debug.h"
#include "../lv_misc/lv_math.h"
#include "../lv_draw/lv_draw.h"
#include "../lv_core/lv_refr.h"

#if LV_USE_CANVAS != 0




#define LV_OBJX_NAME "lv_canvas"








static lv_res_t lv_canvas_signal(lv_obj_t * canvas, lv_signal_t sign, void * param);




static lv_signal_cb_t ancestor_signal;
static lv_design_cb_t ancestor_design;















lv_obj_t * lv_canvas_create(lv_obj_t * par, const lv_obj_t * copy)
{
LV_LOG_TRACE("canvas create started");


lv_obj_t * new_canvas = lv_img_create(par, copy);
LV_ASSERT_MEM(new_canvas);
if(new_canvas == NULL) return NULL;


lv_canvas_ext_t * ext = lv_obj_allocate_ext_attr(new_canvas, sizeof(lv_canvas_ext_t));
LV_ASSERT_MEM(ext);
if(ext == NULL) return NULL;
if(ancestor_signal == NULL) ancestor_signal = lv_obj_get_signal_cb(new_canvas);
if(ancestor_design == NULL) ancestor_design = lv_obj_get_design_cb(new_canvas);


ext->dsc.header.always_zero = 0;
ext->dsc.header.cf = LV_IMG_CF_TRUE_COLOR;
ext->dsc.header.h = 0;
ext->dsc.header.w = 0;
ext->dsc.data_size = 0;
ext->dsc.data = NULL;

lv_img_set_src(new_canvas, &ext->dsc);


lv_obj_set_signal_cb(new_canvas, lv_canvas_signal);


if(copy == NULL) {

}

else {

lv_obj_refresh_style(new_canvas);
}

LV_LOG_INFO("canvas created");

return new_canvas;
}



















void lv_canvas_set_buffer(lv_obj_t * canvas, void * buf, lv_coord_t w, lv_coord_t h, lv_img_cf_t cf)
{
LV_ASSERT_OBJ(canvas, LV_OBJX_NAME);
LV_ASSERT_NULL(buf);

lv_canvas_ext_t * ext = lv_obj_get_ext_attr(canvas);

ext->dsc.header.cf = cf;
ext->dsc.header.w = w;
ext->dsc.header.h = h;
ext->dsc.data = buf;
ext->dsc.data_size = (lv_img_color_format_get_px_size(cf) * w * h) / 8;

lv_img_set_src(canvas, &ext->dsc);
}








void lv_canvas_set_px(lv_obj_t * canvas, lv_coord_t x, lv_coord_t y, lv_color_t c)
{
LV_ASSERT_OBJ(canvas, LV_OBJX_NAME);

lv_canvas_ext_t * ext = lv_obj_get_ext_attr(canvas);

lv_img_buf_set_px_color(&ext->dsc, x, y, c);
lv_obj_invalidate(canvas);
}











void lv_canvas_set_palette(lv_obj_t * canvas, uint8_t id, lv_color_t c)
{
LV_ASSERT_OBJ(canvas, LV_OBJX_NAME);

lv_canvas_ext_t * ext = lv_obj_get_ext_attr(canvas);

lv_img_buf_set_palette(&ext->dsc, id, c);
lv_obj_invalidate(canvas);
}







void lv_canvas_set_style(lv_obj_t * canvas, lv_canvas_style_t type, const lv_style_t * style)
{
LV_ASSERT_OBJ(canvas, LV_OBJX_NAME);

switch(type) {
case LV_CANVAS_STYLE_MAIN: lv_img_set_style(canvas, LV_IMG_STYLE_MAIN, style); break;
}
}












lv_color_t lv_canvas_get_px(lv_obj_t * canvas, lv_coord_t x, lv_coord_t y)
{
LV_ASSERT_OBJ(canvas, LV_OBJX_NAME);

lv_canvas_ext_t * ext = lv_obj_get_ext_attr(canvas);
const lv_style_t * style = lv_canvas_get_style(canvas, LV_CANVAS_STYLE_MAIN);

return lv_img_buf_get_px_color(&ext->dsc, x, y, style);
}






lv_img_dsc_t * lv_canvas_get_img(lv_obj_t * canvas)
{
LV_ASSERT_OBJ(canvas, LV_OBJX_NAME);

lv_canvas_ext_t * ext = lv_obj_get_ext_attr(canvas);

return &ext->dsc;
}







const lv_style_t * lv_canvas_get_style(const lv_obj_t * canvas, lv_canvas_style_t type)
{
LV_ASSERT_OBJ(canvas, LV_OBJX_NAME);

const lv_style_t * style = NULL;

switch(type) {
case LV_CANVAS_STYLE_MAIN: style = lv_img_get_style(canvas, LV_IMG_STYLE_MAIN); break;
default: style = NULL;
}

return style;
}















void lv_canvas_copy_buf(lv_obj_t * canvas, const void * to_copy, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h)
{
LV_ASSERT_OBJ(canvas, LV_OBJX_NAME);
LV_ASSERT_NULL(to_copy);

lv_canvas_ext_t * ext = lv_obj_get_ext_attr(canvas);
if(x + w >= (lv_coord_t)ext->dsc.header.w || y + h >= (lv_coord_t)ext->dsc.header.h) {
LV_LOG_WARN("lv_canvas_copy_buf: x or y out of the canvas");
return;
}

uint32_t px_size = lv_img_color_format_get_px_size(ext->dsc.header.cf) >> 3;
uint32_t px = ext->dsc.header.w * y * px_size + x * px_size;
uint8_t * to_copy8 = (uint8_t *)to_copy;
lv_coord_t i;
for(i = 0; i < h; i++) {
memcpy((void *)&ext->dsc.data[px], to_copy8, w * px_size);
px += ext->dsc.header.w * px_size;
to_copy8 += w * px_size;
}

lv_obj_invalidate(canvas);
}














void lv_canvas_rotate(lv_obj_t * canvas, lv_img_dsc_t * img, int16_t angle, lv_coord_t offset_x, lv_coord_t offset_y,
int32_t pivot_x, int32_t pivot_y)
{
LV_ASSERT_OBJ(canvas, LV_OBJX_NAME);
LV_ASSERT_NULL(img);

lv_canvas_ext_t * ext_dst = lv_obj_get_ext_attr(canvas);
const lv_style_t * style = lv_canvas_get_style(canvas, LV_CANVAS_STYLE_MAIN);
int32_t sinma = lv_trigo_sin(-angle);
int32_t cosma = lv_trigo_sin(-angle + 90); 

int32_t img_width = img->header.w;
int32_t img_height = img->header.h;
int32_t dest_width = ext_dst->dsc.header.w;
int32_t dest_height = ext_dst->dsc.header.h;

int32_t x;
int32_t y;
for(x = -offset_x; x < dest_width - offset_x; x++) {
for(y = -offset_y; y < dest_height - offset_y; y++) {

int32_t xt = x - pivot_x;
int32_t yt = y - pivot_y;


int32_t xs = ((cosma * xt - sinma * yt) >> (LV_TRIGO_SHIFT - 8)) + pivot_x * 256;
int32_t ys = ((sinma * xt + cosma * yt) >> (LV_TRIGO_SHIFT - 8)) + pivot_y * 256;


int xs_int = xs >> 8;
int ys_int = ys >> 8;

if(xs_int >= img_width)
continue;
else if(xs_int < 0)
continue;

if(ys_int >= img_height)
continue;
else if(ys_int < 0)
continue;


int xs_fract = xs & 0xff;
int ys_fract = ys & 0xff;





int xn; 
lv_opa_t xr; 
if(xs_fract < 0x70) {
xn = xs_int - 1;
xr = xs_fract * 2;
} else if(xs_fract > 0x90) {
xn = xs_int + 1;
xr = (0xFF - xs_fract) * 2;
} else {
xn = xs_int;
xr = 0xFF;
}


if(xn >= img_width)
continue;
else if(xn < 0)
continue;

int yn; 
lv_opa_t yr; 
if(ys_fract < 0x70) {
yn = ys_int - 1;
yr = ys_fract * 2;
} else if(ys_fract > 0x90) {
yn = ys_int + 1;
yr = (0xFF - ys_fract) * 2;
} else {
yn = ys_int;
yr = 0xFF;
}


if(yn >= img_height)
continue;
else if(yn < 0)
continue;


lv_color_t c_dest_int = lv_img_buf_get_px_color(img, xs_int, ys_int, style);

if(lv_img_color_format_is_chroma_keyed(img->header.cf)) {
lv_color_t ct = LV_COLOR_TRANSP;
if(c_dest_int.full == ct.full) continue;
}

lv_color_t c_dest_xn = lv_img_buf_get_px_color(img, xn, ys_int, style);
lv_color_t c_dest_yn = lv_img_buf_get_px_color(img, xs_int, yn, style);
lv_color_t x_dest = lv_color_mix(c_dest_int, c_dest_xn, xr);
lv_color_t y_dest = lv_color_mix(c_dest_int, c_dest_yn, yr);
lv_color_t color_res = lv_color_mix(x_dest, y_dest, LV_OPA_50);

if(x + offset_x >= 0 && x + offset_x < dest_width && y + offset_y >= 0 && y + offset_y < dest_height) {

if(lv_img_color_format_has_alpha(img->header.cf) == false) {
lv_img_buf_set_px_color(&ext_dst->dsc, x + offset_x, y + offset_y, color_res);
} else {

lv_opa_t opa_int = lv_img_buf_get_px_alpha(img, xs_int, ys_int);
lv_opa_t opa_xn = lv_img_buf_get_px_alpha(img, xn, ys_int);
lv_opa_t opa_yn = lv_img_buf_get_px_alpha(img, xs_int, yn);
lv_opa_t opa_x = (opa_int * xr + (opa_xn * (255 - xr))) >> 8;
lv_opa_t opa_y = (opa_int * yr + (opa_yn * (255 - yr))) >> 8;
lv_opa_t opa_res = (opa_x + opa_y) / 2;
if(opa_res <= LV_OPA_MIN) continue;

lv_color_t bg_color = lv_img_buf_get_px_color(&ext_dst->dsc, x + offset_x, y + offset_y, style);



if(lv_img_color_format_has_alpha(ext_dst->dsc.header.cf) == false) {
if(opa_res < LV_OPA_MAX) color_res = lv_color_mix(color_res, bg_color, opa_res);
lv_img_buf_set_px_color(&ext_dst->dsc, x + offset_x, y + offset_y, color_res);
}


else {
lv_opa_t bg_opa = lv_img_buf_get_px_alpha(&ext_dst->dsc, x + offset_x, y + offset_y);


if(opa_res >= LV_OPA_MAX || bg_opa <= LV_OPA_MIN) {
lv_img_buf_set_px_color(&ext_dst->dsc, x + offset_x, y + offset_y, color_res);
lv_img_buf_set_px_alpha(&ext_dst->dsc, x + offset_x, y + offset_y, opa_res);
}

else if(bg_opa >= LV_OPA_MAX) {
lv_img_buf_set_px_color(&ext_dst->dsc, x + offset_x, y + offset_y,
lv_color_mix(color_res, bg_color, opa_res));
}

else {



lv_opa_t opa_res_2 = 255 - ((uint16_t)((uint16_t)(255 - opa_res) * (255 - bg_opa)) >> 8);
if(opa_res_2 == 0) {
opa_res_2 = 1; 
}
lv_opa_t ratio = (uint16_t)((uint16_t)opa_res * 255) / opa_res_2;

lv_img_buf_set_px_color(&ext_dst->dsc, x + offset_x, y + offset_y,
lv_color_mix(color_res, bg_color, ratio));
lv_img_buf_set_px_alpha(&ext_dst->dsc, x + offset_x, y + offset_y, opa_res_2);
}
}
}
}
}
}

lv_obj_invalidate(canvas);
}






void lv_canvas_fill_bg(lv_obj_t * canvas, lv_color_t color)
{
LV_ASSERT_OBJ(canvas, LV_OBJX_NAME);

lv_img_dsc_t * dsc = lv_canvas_get_img(canvas);

uint32_t x = dsc->header.w * dsc->header.h;
uint32_t y;
for(y = 0; y < dsc->header.h; y++) {
for(x = 0; x < dsc->header.w; x++) {
lv_img_buf_set_px_color(dsc, x, y, color);
}
}
lv_obj_invalidate(canvas);
}










void lv_canvas_draw_rect(lv_obj_t * canvas, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h,
const lv_style_t * style)
{
LV_ASSERT_OBJ(canvas, LV_OBJX_NAME);
LV_ASSERT_NULL(style);

lv_img_dsc_t * dsc = lv_canvas_get_img(canvas);



lv_area_t mask;
mask.x1 = 0;
mask.x2 = dsc->header.w - 1;
mask.y1 = 0;
mask.y2 = dsc->header.h - 1;

lv_area_t coords;
coords.x1 = x;
coords.y1 = y;
coords.x2 = x + w - 1;
coords.y2 = y + h - 1;

lv_disp_t disp;
memset(&disp, 0, sizeof(lv_disp_t));

lv_disp_buf_t disp_buf;
lv_disp_buf_init(&disp_buf, (void *)dsc->data, NULL, dsc->header.w * dsc->header.h);
lv_area_copy(&disp_buf.area, &mask);

lv_disp_drv_init(&disp.driver);

disp.driver.buffer = &disp_buf;
disp.driver.hor_res = dsc->header.w;
disp.driver.ver_res = dsc->header.h;

#if LV_ANTIALIAS

lv_color_t ctransp = LV_COLOR_TRANSP;
if(dsc->header.cf == LV_IMG_CF_TRUE_COLOR_CHROMA_KEYED &&
style->body.main_color.full == ctransp.full &&
style->body.grad_color.full == ctransp.full)
{
disp.driver.antialiasing = 0;
}
#endif

lv_disp_t * refr_ori = lv_refr_get_disp_refreshing();
lv_refr_set_disp_refreshing(&disp);

lv_draw_rect(&coords, &mask, style, LV_OPA_COVER);

lv_refr_set_disp_refreshing(refr_ori);
lv_obj_invalidate(canvas);
}











void lv_canvas_draw_text(lv_obj_t * canvas, lv_coord_t x, lv_coord_t y, lv_coord_t max_w, const lv_style_t * style,
const char * txt, lv_label_align_t align)
{
LV_ASSERT_OBJ(canvas, LV_OBJX_NAME);
LV_ASSERT_NULL(style);

lv_img_dsc_t * dsc = lv_canvas_get_img(canvas);



lv_area_t mask;
mask.x1 = 0;
mask.x2 = dsc->header.w - 1;
mask.y1 = 0;
mask.y2 = dsc->header.h - 1;

lv_area_t coords;
coords.x1 = x;
coords.y1 = y;
coords.x2 = x + max_w - 1;
coords.y2 = dsc->header.h - 1;

lv_disp_t disp;
memset(&disp, 0, sizeof(lv_disp_t));

lv_disp_buf_t disp_buf;
lv_disp_buf_init(&disp_buf, (void *)dsc->data, NULL, dsc->header.w * dsc->header.h);
lv_area_copy(&disp_buf.area, &mask);

lv_disp_drv_init(&disp.driver);

disp.driver.buffer = &disp_buf;
disp.driver.hor_res = dsc->header.w;
disp.driver.ver_res = dsc->header.h;

lv_disp_t * refr_ori = lv_refr_get_disp_refreshing();
lv_refr_set_disp_refreshing(&disp);

lv_txt_flag_t flag;
switch(align) {
case LV_LABEL_ALIGN_LEFT: flag = LV_TXT_FLAG_NONE; break;
case LV_LABEL_ALIGN_RIGHT: flag = LV_TXT_FLAG_RIGHT; break;
case LV_LABEL_ALIGN_CENTER: flag = LV_TXT_FLAG_CENTER; break;
default: flag = LV_TXT_FLAG_NONE; break;
}

lv_draw_label(&coords, &mask, style, LV_OPA_COVER, txt, flag, NULL, NULL, NULL, lv_obj_get_base_dir(canvas));

lv_refr_set_disp_refreshing(refr_ori);
lv_obj_invalidate(canvas);
}







void lv_canvas_draw_img(lv_obj_t * canvas, lv_coord_t x, lv_coord_t y, const void * src, const lv_style_t * style)
{
LV_ASSERT_OBJ(canvas, LV_OBJX_NAME);
LV_ASSERT_NULL(style);

lv_img_dsc_t * dsc = lv_canvas_get_img(canvas);



lv_area_t mask;
mask.x1 = 0;
mask.x2 = dsc->header.w - 1;
mask.y1 = 0;
mask.y2 = dsc->header.h - 1;

lv_img_header_t header;
lv_res_t res = lv_img_decoder_get_info(src, &header);
if(res != LV_RES_OK) {
LV_LOG_WARN("lv_canvas_draw_img: Couldn't get the image data.");
return;
}

lv_area_t coords;
coords.x1 = x;
coords.y1 = y;
coords.x2 = x + header.w - 1;
coords.y2 = y + header.h - 1;

lv_disp_t disp;
memset(&disp, 0, sizeof(lv_disp_t));

lv_disp_buf_t disp_buf;
lv_disp_buf_init(&disp_buf, (void *)dsc->data, NULL, dsc->header.w * dsc->header.h);
lv_area_copy(&disp_buf.area, &mask);

lv_disp_drv_init(&disp.driver);

disp.driver.buffer = &disp_buf;
disp.driver.hor_res = dsc->header.w;
disp.driver.ver_res = dsc->header.h;

lv_disp_t * refr_ori = lv_refr_get_disp_refreshing();
lv_refr_set_disp_refreshing(&disp);

lv_draw_img(&coords, &mask, src, style, LV_OPA_COVER);

lv_refr_set_disp_refreshing(refr_ori);
lv_obj_invalidate(canvas);
}








void lv_canvas_draw_line(lv_obj_t * canvas, const lv_point_t * points, uint32_t point_cnt, const lv_style_t * style)
{
LV_ASSERT_OBJ(canvas, LV_OBJX_NAME);
LV_ASSERT_NULL(style);

lv_img_dsc_t * dsc = lv_canvas_get_img(canvas);



lv_area_t mask;
mask.x1 = 0;
mask.x2 = dsc->header.w - 1;
mask.y1 = 0;
mask.y2 = dsc->header.h - 1;

lv_disp_t disp;
memset(&disp, 0, sizeof(lv_disp_t));

lv_disp_buf_t disp_buf;
lv_disp_buf_init(&disp_buf, (void *)dsc->data, NULL, dsc->header.w * dsc->header.h);
lv_area_copy(&disp_buf.area, &mask);

lv_disp_drv_init(&disp.driver);

disp.driver.buffer = &disp_buf;
disp.driver.hor_res = dsc->header.w;
disp.driver.ver_res = dsc->header.h;

#if LV_ANTIALIAS

lv_color_t ctransp = LV_COLOR_TRANSP;
if(dsc->header.cf == LV_IMG_CF_TRUE_COLOR_CHROMA_KEYED &&
style->body.main_color.full == ctransp.full &&
style->body.grad_color.full == ctransp.full)
{
disp.driver.antialiasing = 0;
}
#endif

lv_disp_t * refr_ori = lv_refr_get_disp_refreshing();
lv_refr_set_disp_refreshing(&disp);

lv_style_t circle_style_tmp; 
if(style->line.rounded) {
lv_style_copy(&circle_style_tmp, style);
circle_style_tmp.body.radius = LV_RADIUS_CIRCLE;
circle_style_tmp.body.main_color = style->line.color;
circle_style_tmp.body.grad_color = style->line.color;
circle_style_tmp.body.opa = style->line.opa;
}
lv_area_t circle_area;
uint32_t i;
for(i = 0; i < point_cnt - 1; i++) {
lv_draw_line(&points[i], &points[i + 1], &mask, style, LV_OPA_COVER);


if(style->line.rounded) {
circle_area.x1 = points[i].x - ((style->line.width - 1) >> 1) - ((style->line.width - 1) & 0x1);
circle_area.y1 = points[i].y - ((style->line.width - 1) >> 1) - ((style->line.width - 1) & 0x1);
circle_area.x2 = points[i].x + ((style->line.width - 1) >> 1);
circle_area.y2 = points[i].y + ((style->line.width - 1) >> 1);
lv_draw_rect(&circle_area, &mask, &circle_style_tmp, LV_OPA_COVER);
}
}

if(style->line.rounded) {
circle_area.x1 = points[i].x - ((style->line.width - 1) >> 1) - ((style->line.width - 1) & 0x1);
circle_area.y1 = points[i].y - ((style->line.width - 1) >> 1) - ((style->line.width - 1) & 0x1);
circle_area.x2 = points[i].x + ((style->line.width - 1) >> 1);
circle_area.y2 = points[i].y + ((style->line.width - 1) >> 1);
lv_draw_rect(&circle_area, &mask, &circle_style_tmp, LV_OPA_COVER);
}

lv_refr_set_disp_refreshing(refr_ori);
lv_obj_invalidate(canvas);
}








void lv_canvas_draw_polygon(lv_obj_t * canvas, const lv_point_t * points, uint32_t point_cnt, const lv_style_t * style)
{
LV_ASSERT_OBJ(canvas, LV_OBJX_NAME);
LV_ASSERT_NULL(style);

lv_img_dsc_t * dsc = lv_canvas_get_img(canvas);



lv_area_t mask;
mask.x1 = 0;
mask.x2 = dsc->header.w - 1;
mask.y1 = 0;
mask.y2 = dsc->header.h - 1;

lv_disp_t disp;
memset(&disp, 0, sizeof(lv_disp_t));

lv_disp_buf_t disp_buf;
lv_disp_buf_init(&disp_buf, (void *)dsc->data, NULL, dsc->header.w * dsc->header.h);
lv_area_copy(&disp_buf.area, &mask);

lv_disp_drv_init(&disp.driver);

disp.driver.buffer = &disp_buf;
disp.driver.hor_res = dsc->header.w;
disp.driver.ver_res = dsc->header.h;

#if LV_ANTIALIAS

lv_color_t ctransp = LV_COLOR_TRANSP;
if(dsc->header.cf == LV_IMG_CF_TRUE_COLOR_CHROMA_KEYED &&
style->body.main_color.full == ctransp.full &&
style->body.grad_color.full == ctransp.full)
{
disp.driver.antialiasing = 0;
}
#endif

lv_disp_t * refr_ori = lv_refr_get_disp_refreshing();
lv_refr_set_disp_refreshing(&disp);

lv_draw_polygon(points, point_cnt, &mask, style, LV_OPA_COVER);

lv_refr_set_disp_refreshing(refr_ori);
lv_obj_invalidate(canvas);
}











void lv_canvas_draw_arc(lv_obj_t * canvas, lv_coord_t x, lv_coord_t y, lv_coord_t r, int32_t start_angle,
int32_t end_angle, const lv_style_t * style)
{
LV_ASSERT_OBJ(canvas, LV_OBJX_NAME);
LV_ASSERT_NULL(style);

lv_img_dsc_t * dsc = lv_canvas_get_img(canvas);



lv_area_t mask;
mask.x1 = 0;
mask.x2 = dsc->header.w - 1;
mask.y1 = 0;
mask.y2 = dsc->header.h - 1;

lv_disp_t disp;
memset(&disp, 0, sizeof(lv_disp_t));

lv_disp_buf_t disp_buf;
lv_disp_buf_init(&disp_buf, (void *)dsc->data, NULL, dsc->header.w * dsc->header.h);
lv_area_copy(&disp_buf.area, &mask);

lv_disp_drv_init(&disp.driver);

disp.driver.buffer = &disp_buf;
disp.driver.hor_res = dsc->header.w;
disp.driver.ver_res = dsc->header.h;

#if LV_ANTIALIAS

lv_color_t ctransp = LV_COLOR_TRANSP;
if(dsc->header.cf == LV_IMG_CF_TRUE_COLOR_CHROMA_KEYED &&
style->body.main_color.full == ctransp.full &&
style->body.grad_color.full == ctransp.full)
{
disp.driver.antialiasing = 0;
}
#endif

lv_disp_t * refr_ori = lv_refr_get_disp_refreshing();
lv_refr_set_disp_refreshing(&disp);

lv_draw_arc(x, y, r, &mask, start_angle, end_angle, style, LV_OPA_COVER);

lv_refr_set_disp_refreshing(refr_ori);
lv_obj_invalidate(canvas);
}












static lv_res_t lv_canvas_signal(lv_obj_t * canvas, lv_signal_t sign, void * param)
{
lv_res_t res;


res = ancestor_signal(canvas, sign, param);
if(res != LV_RES_OK) return res;
if(sign == LV_SIGNAL_GET_TYPE) return lv_obj_handle_get_type_signal(param, LV_OBJX_NAME);

if(sign == LV_SIGNAL_CLEANUP) {

}

return res;
}

#endif
