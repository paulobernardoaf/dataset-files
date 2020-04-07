







#include "lv_cb.h"
#if LV_USE_CB != 0

#include "../lv_core/lv_debug.h"
#include "../lv_core/lv_group.h"
#include "../lv_themes/lv_theme.h"




#define LV_OBJX_NAME "lv_cb"








static bool lv_cb_design(lv_obj_t * cb, const lv_area_t * mask, lv_design_mode_t mode);
static bool lv_bullet_design(lv_obj_t * bullet, const lv_area_t * mask, lv_design_mode_t mode);
static lv_res_t lv_cb_signal(lv_obj_t * cb, lv_signal_t sign, void * param);




static lv_design_cb_t ancestor_bg_design;
static lv_design_cb_t ancestor_bullet_design;
static lv_signal_cb_t ancestor_signal;















lv_obj_t * lv_cb_create(lv_obj_t * par, const lv_obj_t * copy)
{

LV_LOG_TRACE("check box create started");


lv_obj_t * new_cb = lv_btn_create(par, copy);
LV_ASSERT_MEM(new_cb);
if(new_cb == NULL) return NULL;

if(ancestor_signal == NULL) ancestor_signal = lv_obj_get_signal_cb(new_cb);
if(ancestor_bg_design == NULL) ancestor_bg_design = lv_obj_get_design_cb(new_cb);

lv_cb_ext_t * ext = lv_obj_allocate_ext_attr(new_cb, sizeof(lv_cb_ext_t));
LV_ASSERT_MEM(ext);
if(ext == NULL) return NULL;

ext->bullet = NULL;
ext->label = NULL;

lv_obj_set_signal_cb(new_cb, lv_cb_signal);
lv_obj_set_design_cb(new_cb, lv_cb_design);


if(copy == NULL) {
ext->bullet = lv_btn_create(new_cb, NULL);
if(ancestor_bullet_design == NULL) ancestor_bullet_design = lv_obj_get_design_cb(ext->bullet);
lv_obj_set_click(ext->bullet, false);

ext->label = lv_label_create(new_cb, NULL);

lv_cb_set_text(new_cb, "Check box");
lv_btn_set_layout(new_cb, LV_LAYOUT_ROW_M);
lv_btn_set_fit(new_cb, LV_FIT_TIGHT);
lv_btn_set_toggle(new_cb, true);
lv_obj_set_protect(new_cb, LV_PROTECT_PRESS_LOST);


lv_theme_t * th = lv_theme_get_current();
if(th) {
lv_cb_set_style(new_cb, LV_CB_STYLE_BG, th->style.cb.bg);
lv_cb_set_style(new_cb, LV_CB_STYLE_BOX_REL, th->style.cb.box.rel);
lv_cb_set_style(new_cb, LV_CB_STYLE_BOX_PR, th->style.cb.box.pr);
lv_cb_set_style(new_cb, LV_CB_STYLE_BOX_TGL_REL, th->style.cb.box.tgl_rel);
lv_cb_set_style(new_cb, LV_CB_STYLE_BOX_TGL_PR, th->style.cb.box.tgl_pr);
lv_cb_set_style(new_cb, LV_CB_STYLE_BOX_INA, th->style.cb.box.ina);
} else {
lv_cb_set_style(new_cb, LV_CB_STYLE_BG, &lv_style_transp);
lv_cb_set_style(new_cb, LV_CB_STYLE_BOX_REL, &lv_style_pretty);
}
} else {
lv_cb_ext_t * copy_ext = lv_obj_get_ext_attr(copy);
ext->bullet = lv_btn_create(new_cb, copy_ext->bullet);
ext->label = lv_label_create(new_cb, copy_ext->label);


lv_obj_refresh_style(new_cb);
}

lv_obj_set_design_cb(ext->bullet, lv_bullet_design);

LV_LOG_INFO("check box created");

return new_cb;
}











void lv_cb_set_text(lv_obj_t * cb, const char * txt)
{
LV_ASSERT_OBJ(cb, LV_OBJX_NAME);

lv_cb_ext_t * ext = lv_obj_get_ext_attr(cb);
lv_label_set_text(ext->label, txt);
}







void lv_cb_set_static_text(lv_obj_t * cb, const char * txt)
{
LV_ASSERT_OBJ(cb, LV_OBJX_NAME);

lv_cb_ext_t * ext = lv_obj_get_ext_attr(cb);
lv_label_set_static_text(ext->label, txt);
}







void lv_cb_set_style(lv_obj_t * cb, lv_cb_style_t type, const lv_style_t * style)
{
LV_ASSERT_OBJ(cb, LV_OBJX_NAME);

lv_cb_ext_t * ext = lv_obj_get_ext_attr(cb);

switch(type) {
case LV_CB_STYLE_BG:
lv_btn_set_style(cb, LV_BTN_STYLE_REL, style);
lv_btn_set_style(cb, LV_BTN_STYLE_PR, style);
lv_btn_set_style(cb, LV_BTN_STYLE_TGL_REL, style);
lv_btn_set_style(cb, LV_BTN_STYLE_TGL_PR, style);
lv_btn_set_style(cb, LV_BTN_STYLE_INA, style);
break;
case LV_CB_STYLE_BOX_REL: lv_btn_set_style(ext->bullet, LV_BTN_STYLE_REL, style); break;
case LV_CB_STYLE_BOX_PR: lv_btn_set_style(ext->bullet, LV_BTN_STYLE_PR, style); break;
case LV_CB_STYLE_BOX_TGL_REL: lv_btn_set_style(ext->bullet, LV_BTN_STYLE_TGL_REL, style); break;
case LV_CB_STYLE_BOX_TGL_PR: lv_btn_set_style(ext->bullet, LV_BTN_STYLE_TGL_PR, style); break;
case LV_CB_STYLE_BOX_INA: lv_btn_set_style(ext->bullet, LV_BTN_STYLE_INA, style); break;
}
}










const char * lv_cb_get_text(const lv_obj_t * cb)
{
LV_ASSERT_OBJ(cb, LV_OBJX_NAME);

lv_cb_ext_t * ext = lv_obj_get_ext_attr(cb);
return lv_label_get_text(ext->label);
}







const lv_style_t * lv_cb_get_style(const lv_obj_t * cb, lv_cb_style_t type)
{
LV_ASSERT_OBJ(cb, LV_OBJX_NAME);

const lv_style_t * style = NULL;
lv_cb_ext_t * ext = lv_obj_get_ext_attr(cb);

switch(type) {
case LV_CB_STYLE_BG: style = lv_btn_get_style(cb, LV_BTN_STYLE_REL); break;
case LV_CB_STYLE_BOX_REL: style = lv_btn_get_style(ext->bullet, LV_BTN_STYLE_REL); break;
case LV_CB_STYLE_BOX_PR: style = lv_btn_get_style(ext->bullet, LV_BTN_STYLE_PR); break;
case LV_CB_STYLE_BOX_TGL_REL: style = lv_btn_get_style(ext->bullet, LV_BTN_STYLE_TGL_REL); break;
case LV_CB_STYLE_BOX_TGL_PR: style = lv_btn_get_style(ext->bullet, LV_BTN_STYLE_TGL_PR); break;
case LV_CB_STYLE_BOX_INA: style = lv_btn_get_style(ext->bullet, LV_BTN_STYLE_INA); break;
default: style = NULL; break;
}

return style;
}















static bool lv_cb_design(lv_obj_t * cb, const lv_area_t * mask, lv_design_mode_t mode)
{
bool result = true;

if(mode == LV_DESIGN_COVER_CHK) {

result = ancestor_bg_design(cb, mask, mode);
} else if(mode == LV_DESIGN_DRAW_MAIN || mode == LV_DESIGN_DRAW_POST) {
lv_cb_ext_t * cb_ext = lv_obj_get_ext_attr(cb);
lv_btn_ext_t * bullet_ext = lv_obj_get_ext_attr(cb_ext->bullet);


bullet_ext->state = cb_ext->bg_btn.state;

result = ancestor_bg_design(cb, mask, mode);

} else {
result = ancestor_bg_design(cb, mask, mode);
}

return result;
}











static bool lv_bullet_design(lv_obj_t * bullet, const lv_area_t * mask, lv_design_mode_t mode)
{
if(mode == LV_DESIGN_COVER_CHK) {
return ancestor_bullet_design(bullet, mask, mode);
} else if(mode == LV_DESIGN_DRAW_MAIN) {
#if LV_USE_GROUP



const lv_style_t * style_ori = lv_obj_get_style(bullet);
lv_obj_t * bg = lv_obj_get_parent(bullet);
const lv_style_t * style_page = lv_obj_get_style(bg);
lv_group_t * g = lv_obj_get_group(bg);
if(style_page->body.opa == LV_OPA_TRANSP) { 
if(lv_group_get_focused(g) == bg) {
lv_style_t * style_mod;
style_mod = lv_group_mod_style(g, style_ori);
bullet->style_p = style_mod; 
}
}
#endif
ancestor_bullet_design(bullet, mask, mode);

#if LV_USE_GROUP
bullet->style_p = style_ori; 
#endif
} else if(mode == LV_DESIGN_DRAW_POST) {
ancestor_bullet_design(bullet, mask, mode);
}

return true;
}








static lv_res_t lv_cb_signal(lv_obj_t * cb, lv_signal_t sign, void * param)
{
lv_res_t res;


res = ancestor_signal(cb, sign, param);
if(res != LV_RES_OK) return res;
if(sign == LV_SIGNAL_GET_TYPE) return lv_obj_handle_get_type_signal(param, LV_OBJX_NAME);

lv_cb_ext_t * ext = lv_obj_get_ext_attr(cb);

if(sign == LV_SIGNAL_STYLE_CHG) {
const lv_style_t * label_style = lv_label_get_style(ext->label, LV_LABEL_STYLE_MAIN);
lv_obj_set_size(ext->bullet, lv_font_get_line_height(label_style->text.font),
lv_font_get_line_height(label_style->text.font));
lv_btn_set_state(ext->bullet, lv_btn_get_state(cb));
} else if(sign == LV_SIGNAL_PRESSED || sign == LV_SIGNAL_RELEASED || sign == LV_SIGNAL_PRESS_LOST) {
lv_btn_set_state(ext->bullet, lv_btn_get_state(cb));
} else if(sign == LV_SIGNAL_CONTROL) {
char c = *((char *)param);
if(c == LV_KEY_RIGHT || c == LV_KEY_DOWN || c == LV_KEY_LEFT || c == LV_KEY_UP) {

lv_btn_set_state(ext->bullet, lv_btn_get_state(cb));
}
}

return res;
}

#endif
