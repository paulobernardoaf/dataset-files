#include "../lv_core/lv_debug.h"
#if defined(LV_USE_TEMPL) && LV_USE_TEMPL != 0
#define LV_OBJX_NAME "lv_templ"
static bool lv_templ_design(lv_obj_t * templ, const lv_area_t * mask, lv_design_mode_t mode);
static lv_res_t lv_templ_signal(lv_obj_t * templ, lv_signal_t sign, void * param);
static lv_signal_func_t ancestor_signal;
static lv_design_func_t ancestor_design;
lv_obj_t * lv_templ_create(lv_obj_t * par, const lv_obj_t * copy)
{
LV_LOG_TRACE("template create started");
lv_obj_t * new_templ = lv_ANCESTOR_create(par, copy);
LV_ASSERT_MEM(new_templ);
if(new_templ == NULL) return NULL;
lv_templ_ext_t * ext = lv_obj_allocate_ext_attr(new_templ, sizeof(lv_templ_ext_t));
LV_ASSERT_MEM(ext);
if(ext == NULL) return NULL;
if(ancestor_signal == NULL) ancestor_signal = lv_obj_get_signal_func(new_templ);
if(ancestor_design == NULL) ancestor_design = lv_obj_get_design_func(new_templ);
ext->xyz = 0;
lv_obj_set_signal_func(new_templ, lv_templ_signal);
lv_obj_set_design_func(new_templ, lv_templ_design);
if(copy == NULL) {
}
else {
lv_templ_ext_t * copy_ext = lv_obj_get_ext_attr(copy);
lv_obj_refresh_style(new_templ);
}
LV_LOG_INFO("template created");
return new_templ;
}
void lv_templ_set_style(lv_obj_t * templ, lv_templ_style_t type, const lv_style_t * style)
{
LV_ASSERT_OBJ(templ, LV_OBJX_NAME);
lv_templ_ext_t * ext = lv_obj_get_ext_attr(templ);
switch(type) {
case LV_TEMPL_STYLE_X: break;
case LV_TEMPL_STYLE_Y: break;
}
}
lv_style_t * lv_templ_get_style(const lv_obj_t * templ, lv_templ_style_t type)
{
LV_ASSERT_OBJ(templ, LV_OBJX_NAME);
lv_templ_ext_t * ext = lv_obj_get_ext_attr(templ);
lv_style_t * style = NULL;
switch(type) {
case LV_TEMPL_STYLE_X: style = NULL; 
case LV_TEMPL_STYLE_Y: style = NULL; 
default: style = NULL;
}
return style;
}
static bool lv_templ_design(lv_obj_t * templ, const lv_area_t * mask, lv_design_mode_t mode)
{
if(mode == LV_DESIGN_COVER_CHK) {
return false;
}
else if(mode == LV_DESIGN_DRAW_MAIN) {
}
else if(mode == LV_DESIGN_DRAW_POST) {
}
return true;
}
static lv_res_t lv_templ_signal(lv_obj_t * templ, lv_signal_t sign, void * param)
{
lv_res_t res;
res = ancestor_signal(templ, sign, param);
if(res != LV_RES_OK) return res;
if(sign == LV_SIGNAL_GET_TYPE) return lv_obj_handle_get_type_signal(param, LV_OBJX_NAME);
if(sign == LV_SIGNAL_CLEANUP) {
}
return res;
}
#else 
typedef int keep_pedantic_happy;
#endif
