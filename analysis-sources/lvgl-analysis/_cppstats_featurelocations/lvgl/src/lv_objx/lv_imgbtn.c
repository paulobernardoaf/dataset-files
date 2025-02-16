








#include "../lv_core/lv_debug.h"
#include "lv_imgbtn.h"
#include "lv_label.h"

#if LV_USE_IMGBTN != 0




#define LV_OBJX_NAME "lv_imgbtn"








static bool lv_imgbtn_design(lv_obj_t * imgbtn, const lv_area_t * mask, lv_design_mode_t mode);
static lv_res_t lv_imgbtn_signal(lv_obj_t * imgbtn, lv_signal_t sign, void * param);
static void refr_img(lv_obj_t * imgbtn);




static lv_signal_cb_t ancestor_signal;
static lv_design_cb_t ancestor_design;
















lv_obj_t * lv_imgbtn_create(lv_obj_t * par, const lv_obj_t * copy)
{
LV_LOG_TRACE("image button create started");


lv_obj_t * new_imgbtn = lv_btn_create(par, copy);
LV_ASSERT_MEM(new_imgbtn);
if(new_imgbtn == NULL) return NULL;


lv_imgbtn_ext_t * ext = lv_obj_allocate_ext_attr(new_imgbtn, sizeof(lv_imgbtn_ext_t));
LV_ASSERT_MEM(ext);
if(ext == NULL) return NULL;
if(ancestor_signal == NULL) ancestor_signal = lv_obj_get_signal_cb(new_imgbtn);
if(ancestor_design == NULL) ancestor_design = lv_obj_get_design_cb(new_imgbtn);


#if LV_IMGBTN_TILED == 0
memset((void*)ext->img_src, 0, sizeof(ext->img_src));
#else
memset(ext->img_src_left, 0, sizeof(ext->img_src_left));
memset(ext->img_src_mid, 0, sizeof(ext->img_src_mid));
memset(ext->img_src_right, 0, sizeof(ext->img_src_right));
#endif

ext->act_cf = LV_IMG_CF_UNKNOWN;


lv_obj_set_signal_cb(new_imgbtn, lv_imgbtn_signal);
lv_obj_set_design_cb(new_imgbtn, lv_imgbtn_design);


if(copy == NULL) {

}

else {
lv_imgbtn_ext_t * copy_ext = lv_obj_get_ext_attr(copy);
#if LV_IMGBTN_TILED == 0
memcpy((void*)ext->img_src, copy_ext->img_src, sizeof(ext->img_src));
#else
memcpy((void*)ext->img_src_left, copy_ext->img_src_left, sizeof(ext->img_src_left));
memcpy((void*)ext->img_src_mid, copy_ext->img_src_mid, sizeof(ext->img_src_mid));
memcpy((void*)ext->img_src_right, copy_ext->img_src_right, sizeof(ext->img_src_right));
#endif

lv_obj_refresh_style(new_imgbtn);
}

LV_LOG_INFO("image button created");

return new_imgbtn;
}





#if LV_IMGBTN_TILED == 0






void lv_imgbtn_set_src(lv_obj_t * imgbtn, lv_btn_state_t state, const void * src)
{
LV_ASSERT_OBJ(imgbtn, LV_OBJX_NAME);

lv_imgbtn_ext_t * ext = lv_obj_get_ext_attr(imgbtn);

ext->img_src[state] = src;

refr_img(imgbtn);
}

#else











void lv_imgbtn_set_src(lv_obj_t * imgbtn, lv_btn_state_t state, const void * src_left, const void * src_mid,
const void * src_right)
{
LV_ASSERT_OBJ(imgbtn, LV_OBJX_NAME);


if(lv_img_src_get_type(src_left) == LV_IMG_SRC_SYMBOL ||
lv_img_src_get_type(src_mid) == LV_IMG_SRC_SYMBOL ||
lv_img_src_get_type(src_right) == LV_IMG_SRC_SYMBOL )
{
LV_LOG_WARN("lv_imgbtn_set_src: symbols are not supported in tiled mode");
return;
}

lv_imgbtn_ext_t * ext = lv_obj_get_ext_attr(imgbtn);

ext->img_src_left[state] = src_left;
ext->img_src_mid[state] = src_mid;
ext->img_src_right[state] = src_right;

refr_img(imgbtn);
}

#endif







void lv_imgbtn_set_style(lv_obj_t * imgbtn, lv_imgbtn_style_t type, const lv_style_t * style)
{
LV_ASSERT_OBJ(imgbtn, LV_OBJX_NAME);

lv_btn_set_style(imgbtn, type, style);
}





#if LV_IMGBTN_TILED == 0






const void * lv_imgbtn_get_src(lv_obj_t * imgbtn, lv_btn_state_t state)
{
LV_ASSERT_OBJ(imgbtn, LV_OBJX_NAME);

lv_imgbtn_ext_t * ext = lv_obj_get_ext_attr(imgbtn);

return ext->img_src[state];
}
#else







const void * lv_imgbtn_get_src_left(lv_obj_t * imgbtn, lv_btn_state_t state)
{
LV_ASSERT_OBJ(imgbtn, LV_OBJX_NAME);

lv_imgbtn_ext_t * ext = lv_obj_get_ext_attr(imgbtn);

return ext->img_src_left[state];
}







const void * lv_imgbtn_get_src_middle(lv_obj_t * imgbtn, lv_btn_state_t state)
{
LV_ASSERT_OBJ(imgbtn, LV_OBJX_NAME);

lv_imgbtn_ext_t * ext = lv_obj_get_ext_attr(imgbtn);

return ext->img_src_mid[state];
}







const void * lv_imgbtn_get_src_right(lv_obj_t * imgbtn, lv_btn_state_t state)
{
LV_ASSERT_OBJ(imgbtn, LV_OBJX_NAME);

lv_imgbtn_ext_t * ext = lv_obj_get_ext_attr(imgbtn);

return ext->img_src_right[state];
}

#endif







const lv_style_t * lv_imgbtn_get_style(const lv_obj_t * imgbtn, lv_imgbtn_style_t type)
{
LV_ASSERT_OBJ(imgbtn, LV_OBJX_NAME);

return lv_btn_get_style(imgbtn, type);
}























static bool lv_imgbtn_design(lv_obj_t * imgbtn, const lv_area_t * mask, lv_design_mode_t mode)
{

if(mode == LV_DESIGN_COVER_CHK) {
lv_imgbtn_ext_t * ext = lv_obj_get_ext_attr(imgbtn);
bool cover = false;
if(ext->act_cf == LV_IMG_CF_TRUE_COLOR || ext->act_cf == LV_IMG_CF_RAW) {
cover = lv_area_is_in(mask, &imgbtn->coords);
}

return cover;
}

else if(mode == LV_DESIGN_DRAW_MAIN) {

lv_imgbtn_ext_t * ext = lv_obj_get_ext_attr(imgbtn);
lv_btn_state_t state = lv_imgbtn_get_state(imgbtn);
const lv_style_t * style = lv_imgbtn_get_style(imgbtn, state);
lv_opa_t opa_scale = lv_obj_get_opa_scale(imgbtn);



#if LV_IMGBTN_TILED == 0
const void * src = ext->img_src[state];
if(lv_img_src_get_type(src) == LV_IMG_SRC_SYMBOL) {
lv_draw_label(&imgbtn->coords, mask, style, opa_scale, src, LV_TXT_FLAG_NONE, NULL, NULL, NULL, lv_obj_get_base_dir(imgbtn));
} else {
lv_draw_img(&imgbtn->coords, mask, src, style, opa_scale);
}
#else
const void * src;
src = ext->img_src_left[state];
if(lv_img_src_get_type(src) == LV_IMG_SRC_SYMBOL) {
LV_LOG_WARN("lv_imgbtn_design: SYMBOLS are not supported in tiled mode")
return true;
}

lv_img_header_t header;
lv_area_t coords;
lv_coord_t left_w = 0;
lv_coord_t right_w = 0;

if(src) {
lv_img_decoder_get_info(src, &header);
left_w = header.w;
coords.x1 = imgbtn->coords.x1;
coords.y1 = imgbtn->coords.y1;
coords.x2 = coords.x1 + header.w - 1;
coords.y2 = coords.y1 + header.h - 1;
lv_draw_img(&coords, mask, src, style, opa_scale);
}

src = ext->img_src_right[state];
if(src) {
lv_img_decoder_get_info(src, &header);
right_w = header.w;
coords.x1 = imgbtn->coords.x2 - header.w + 1;
coords.y1 = imgbtn->coords.y1;
coords.x2 = imgbtn->coords.x2;
coords.y2 = imgbtn->coords.y1 + header.h - 1;
lv_draw_img(&coords, mask, src, style, opa_scale);
}

src = ext->img_src_mid[state];
if(src) {
lv_coord_t obj_w = lv_obj_get_width(imgbtn);
lv_coord_t i;
lv_img_decoder_get_info(src, &header);

coords.x1 = imgbtn->coords.x1 + left_w;
coords.y1 = imgbtn->coords.y1;
coords.x2 = coords.x1 + header.w - 1;
coords.y2 = imgbtn->coords.y1 + header.h - 1;

for(i = 0; i < obj_w - right_w - left_w; i += header.w) {
lv_draw_img(&coords, mask, src, style, opa_scale);
coords.x1 = coords.x2 + 1;
coords.x2 += header.w;
}
}

#endif

}

else if(mode == LV_DESIGN_DRAW_POST) {
}

return true;
}








static lv_res_t lv_imgbtn_signal(lv_obj_t * imgbtn, lv_signal_t sign, void * param)
{
lv_res_t res;


res = ancestor_signal(imgbtn, sign, param);
if(res != LV_RES_OK) return res;
if(sign == LV_SIGNAL_GET_TYPE) return lv_obj_handle_get_type_signal(param, LV_OBJX_NAME);

if(sign == LV_SIGNAL_STYLE_CHG) {


refr_img(imgbtn);
} else if(sign == LV_SIGNAL_CLEANUP) {

}

return res;
}

static void refr_img(lv_obj_t * imgbtn)
{
lv_imgbtn_ext_t * ext = lv_obj_get_ext_attr(imgbtn);
lv_btn_state_t state = lv_imgbtn_get_state(imgbtn);
lv_img_header_t header;

#if LV_IMGBTN_TILED == 0
const void * src = ext->img_src[state];
#else
const void * src = ext->img_src_mid[state];
#endif

lv_res_t info_res = LV_RES_OK;
if(lv_img_src_get_type(src) == LV_IMG_SRC_SYMBOL) {
const lv_style_t * style = ext->btn.styles[state];
header.h = lv_font_get_line_height(style->text.font);
header.w = lv_txt_get_width(src, (uint16_t)strlen(src), style->text.font, style->text.letter_space, LV_TXT_FLAG_NONE);
header.always_zero = 0;
header.cf = LV_IMG_CF_ALPHA_1BIT;
} else {
info_res = lv_img_decoder_get_info(src, &header);
}

if(info_res == LV_RES_OK) {
ext->act_cf = header.cf;
#if LV_IMGBTN_TILED == 0
lv_obj_set_size(imgbtn, header.w, header.h);
#else
lv_obj_set_height(imgbtn, header.h);
#endif
} else {
ext->act_cf = LV_IMG_CF_UNKNOWN;
}

lv_obj_invalidate(imgbtn);
}

#endif
