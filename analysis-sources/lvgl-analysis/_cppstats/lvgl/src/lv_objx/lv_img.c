#include "lv_img.h"
#if LV_USE_IMG != 0
#if LV_USE_LABEL == 0
#error "lv_img: lv_label is required. Enable it in lv_conf.h (LV_USE_LABEL 1) "
#endif
#include "../lv_core/lv_debug.h"
#include "../lv_themes/lv_theme.h"
#include "../lv_draw/lv_img_decoder.h"
#include "../lv_misc/lv_fs.h"
#include "../lv_misc/lv_txt.h"
#include "../lv_misc/lv_log.h"
#define LV_OBJX_NAME "lv_img"
static bool lv_img_design(lv_obj_t * img, const lv_area_t * mask, lv_design_mode_t mode);
static lv_res_t lv_img_signal(lv_obj_t * img, lv_signal_t sign, void * param);
static lv_signal_cb_t ancestor_signal;
lv_obj_t * lv_img_create(lv_obj_t * par, const lv_obj_t * copy)
{
LV_LOG_TRACE("image create started");
lv_obj_t * new_img = NULL;
new_img = lv_obj_create(par, copy);
LV_ASSERT_MEM(new_img);
if(new_img == NULL) return NULL;
if(ancestor_signal == NULL) ancestor_signal = lv_obj_get_signal_cb(new_img);
lv_img_ext_t * ext = lv_obj_allocate_ext_attr(new_img, sizeof(lv_img_ext_t));
LV_ASSERT_MEM(ext);
if(ext == NULL) return NULL;
ext->src = NULL;
ext->src_type = LV_IMG_SRC_UNKNOWN;
ext->cf = LV_IMG_CF_UNKNOWN;
ext->w = lv_obj_get_width(new_img);
ext->h = lv_obj_get_height(new_img);
ext->auto_size = 1;
ext->offset.x = 0;
ext->offset.y = 0;
lv_obj_set_signal_cb(new_img, lv_img_signal);
lv_obj_set_design_cb(new_img, lv_img_design);
if(copy == NULL) {
lv_obj_set_click(new_img, false);
if(par != NULL) {
ext->auto_size = 1;
lv_obj_set_style(new_img, NULL); 
} else {
ext->auto_size = 0;
lv_obj_set_style(new_img, &lv_style_plain); 
}
} else {
lv_img_ext_t * copy_ext = lv_obj_get_ext_attr(copy);
ext->auto_size = copy_ext->auto_size;
lv_img_set_src(new_img, copy_ext->src);
lv_obj_refresh_style(new_img);
}
LV_LOG_INFO("image created");
return new_img;
}
void lv_img_set_src(lv_obj_t * img, const void * src_img)
{
LV_ASSERT_OBJ(img, LV_OBJX_NAME);
lv_img_src_t src_type = lv_img_src_get_type(src_img);
lv_img_ext_t * ext = lv_obj_get_ext_attr(img);
#if LV_USE_LOG && LV_LOG_LEVEL >= LV_LOG_LEVEL_INFO
switch(src_type) {
case LV_IMG_SRC_FILE: LV_LOG_TRACE("lv_img_set_src: `LV_IMG_SRC_FILE` type found"); break;
case LV_IMG_SRC_VARIABLE: LV_LOG_TRACE("lv_img_set_src: `LV_IMG_SRC_VARIABLE` type found"); break;
case LV_IMG_SRC_SYMBOL: LV_LOG_TRACE("lv_img_set_src: `LV_IMG_SRC_SYMBOL` type found"); break;
default: LV_LOG_WARN("lv_img_set_src: unknown type");
}
#endif
if(src_type == LV_IMG_SRC_UNKNOWN) {
LV_LOG_WARN("lv_img_set_src: unknown image type");
if(ext->src_type == LV_IMG_SRC_SYMBOL || ext->src_type == LV_IMG_SRC_FILE) {
lv_mem_free(ext->src);
}
ext->src = NULL;
ext->src_type = LV_IMG_SRC_UNKNOWN;
return;
}
lv_img_header_t header;
lv_img_decoder_get_info(src_img, &header);
if(src_type == LV_IMG_SRC_VARIABLE) {
LV_LOG_INFO("lv_img_set_src: `LV_IMG_SRC_VARIABLE` type found");
if(ext->src_type == LV_IMG_SRC_FILE || ext->src_type == LV_IMG_SRC_SYMBOL) {
lv_mem_free(ext->src);
}
ext->src = src_img;
} else if(src_type == LV_IMG_SRC_FILE || src_type == LV_IMG_SRC_SYMBOL) {
if(ext->src != src_img) {
const void * old_src = NULL;
if(ext->src_type == LV_IMG_SRC_FILE || ext->src_type == LV_IMG_SRC_SYMBOL) {
old_src = ext->src;
}
char * new_str = lv_mem_alloc(strlen(src_img) + 1);
LV_ASSERT_MEM(new_str);
if(new_str == NULL) return;
strcpy(new_str, src_img);
ext->src = new_str;
if(old_src) lv_mem_free(old_src);
}
}
if(src_type == LV_IMG_SRC_SYMBOL) {
const lv_style_t * style = lv_img_get_style(img, LV_IMG_STYLE_MAIN);
lv_point_t size;
lv_txt_get_size(&size, src_img, style->text.font, style->text.letter_space, style->text.line_space,
LV_COORD_MAX, LV_TXT_FLAG_NONE);
header.w = size.x;
header.h = size.y;
}
ext->src_type = src_type;
ext->w = header.w;
ext->h = header.h;
ext->cf = header.cf;
if(lv_img_get_auto_size(img) != false) {
lv_obj_set_size(img, ext->w, ext->h);
}
lv_obj_invalidate(img);
}
void lv_img_set_auto_size(lv_obj_t * img, bool en)
{
LV_ASSERT_OBJ(img, LV_OBJX_NAME);
lv_img_ext_t * ext = lv_obj_get_ext_attr(img);
ext->auto_size = (en == false ? 0 : 1);
}
void lv_img_set_offset_x(lv_obj_t * img, lv_coord_t x)
{
LV_ASSERT_OBJ(img, LV_OBJX_NAME);
lv_img_ext_t * ext = lv_obj_get_ext_attr(img);
if(x < ext->w - 1) {
ext->offset.x = x;
lv_obj_invalidate(img);
}
}
void lv_img_set_offset_y(lv_obj_t * img, lv_coord_t y)
{
LV_ASSERT_OBJ(img, LV_OBJX_NAME);
lv_img_ext_t * ext = lv_obj_get_ext_attr(img);
if(y < ext->h - 1) {
ext->offset.y = y;
lv_obj_invalidate(img);
}
}
const void * lv_img_get_src(lv_obj_t * img)
{
LV_ASSERT_OBJ(img, LV_OBJX_NAME);
lv_img_ext_t * ext = lv_obj_get_ext_attr(img);
return ext->src;
}
const char * lv_img_get_file_name(const lv_obj_t * img)
{
LV_ASSERT_OBJ(img, LV_OBJX_NAME);
lv_img_ext_t * ext = lv_obj_get_ext_attr(img);
if(ext->src_type == LV_IMG_SRC_FILE)
return ext->src;
else
return "";
}
bool lv_img_get_auto_size(const lv_obj_t * img)
{
LV_ASSERT_OBJ(img, LV_OBJX_NAME);
lv_img_ext_t * ext = lv_obj_get_ext_attr(img);
return ext->auto_size == 0 ? false : true;
}
lv_coord_t lv_img_get_offset_x(lv_obj_t * img)
{
LV_ASSERT_OBJ(img, LV_OBJX_NAME);
lv_img_ext_t * ext = lv_obj_get_ext_attr(img);
return ext->offset.x;
}
lv_coord_t lv_img_get_offset_y(lv_obj_t * img)
{
LV_ASSERT_OBJ(img, LV_OBJX_NAME);
lv_img_ext_t * ext = lv_obj_get_ext_attr(img);
return ext->offset.y;
}
static bool lv_img_design(lv_obj_t * img, const lv_area_t * mask, lv_design_mode_t mode)
{
const lv_style_t * style = lv_obj_get_style(img);
lv_img_ext_t * ext = lv_obj_get_ext_attr(img);
if(mode == LV_DESIGN_COVER_CHK) {
bool cover = false;
if(ext->src_type == LV_IMG_SRC_UNKNOWN || ext->src_type == LV_IMG_SRC_SYMBOL) return false;
if(ext->cf == LV_IMG_CF_TRUE_COLOR || ext->cf == LV_IMG_CF_RAW) cover = lv_area_is_in(mask, &img->coords);
const lv_style_t * style = lv_img_get_style(img, LV_IMG_STYLE_MAIN);
if(style->image.opa < LV_OPA_MAX) return false;
return cover;
} else if(mode == LV_DESIGN_DRAW_MAIN) {
if(ext->h == 0 || ext->w == 0) return true;
lv_area_t coords;
lv_opa_t opa_scale = lv_obj_get_opa_scale(img);
lv_obj_get_coords(img, &coords);
if(ext->src_type == LV_IMG_SRC_FILE || ext->src_type == LV_IMG_SRC_VARIABLE) {
coords.x1 -= ext->offset.x;
coords.y1 -= ext->offset.y;
LV_LOG_TRACE("lv_img_design: start to draw image");
lv_area_t cords_tmp;
cords_tmp.y1 = coords.y1;
cords_tmp.y2 = coords.y1 + ext->h - 1;
for(; cords_tmp.y1 <= coords.y2; cords_tmp.y1 += ext->h, cords_tmp.y2 += ext->h) {
cords_tmp.x1 = coords.x1;
cords_tmp.x2 = coords.x1 + ext->w - 1;
for(; cords_tmp.x1 <= coords.x2; cords_tmp.x1 += ext->w, cords_tmp.x2 += ext->w) {
lv_draw_img(&cords_tmp, mask, ext->src, style, opa_scale);
}
}
} else if(ext->src_type == LV_IMG_SRC_SYMBOL) {
LV_LOG_TRACE("lv_img_design: start to draw symbol");
lv_style_t style_mod;
lv_style_copy(&style_mod, style);
style_mod.text.color = style->image.color;
lv_draw_label(&coords, mask, &style_mod, opa_scale, ext->src, LV_TXT_FLAG_NONE, NULL, NULL, NULL, lv_obj_get_base_dir(img));
} else {
LV_LOG_WARN("lv_img_design: image source type is unknown");
lv_draw_img(&img->coords, mask, NULL, style, opa_scale);
}
}
return true;
}
static lv_res_t lv_img_signal(lv_obj_t * img, lv_signal_t sign, void * param)
{
lv_res_t res;
res = ancestor_signal(img, sign, param);
if(res != LV_RES_OK) return res;
if(sign == LV_SIGNAL_GET_TYPE) return lv_obj_handle_get_type_signal(param, LV_OBJX_NAME);
lv_img_ext_t * ext = lv_obj_get_ext_attr(img);
if(sign == LV_SIGNAL_CLEANUP) {
if(ext->src_type == LV_IMG_SRC_FILE || ext->src_type == LV_IMG_SRC_SYMBOL) {
lv_mem_free(ext->src);
ext->src = NULL;
ext->src_type = LV_IMG_SRC_UNKNOWN;
}
} else if(sign == LV_SIGNAL_STYLE_CHG) {
if(ext->src_type == LV_IMG_SRC_SYMBOL) {
lv_img_set_src(img, ext->src);
}
}
return res;
}
#endif
