







#include "lv_disp.h"
#include "../lv_misc/lv_math.h"































lv_obj_t * lv_disp_get_scr_act(lv_disp_t * disp)
{
if(!disp) disp = lv_disp_get_default();
if(!disp) {
LV_LOG_WARN("lv_scr_act: no display registered to get its top layer");
return NULL;
}

return disp->act_scr;
}





void lv_disp_load_scr(lv_obj_t * scr)
{
lv_disp_t * d = lv_obj_get_disp(scr);

d->act_scr = scr;

lv_obj_invalidate(scr);
}






lv_obj_t * lv_disp_get_layer_top(lv_disp_t * disp)
{
if(!disp) disp = lv_disp_get_default();
if(!disp) {
LV_LOG_WARN("lv_layer_top: no display registered to get its top layer");
return NULL;
}

return disp->top_layer;
}







lv_obj_t * lv_disp_get_layer_sys(lv_disp_t * disp)
{
if(!disp) disp = lv_disp_get_default();
if(!disp) {
LV_LOG_WARN("lv_layer_sys: no display registered to get its top layer");
return NULL;
}

return disp->sys_layer;
}






void lv_disp_assign_screen(lv_disp_t * disp, lv_obj_t * scr)
{
if(lv_obj_get_parent(scr) != NULL) {
LV_LOG_WARN("lv_disp_assign_screen: try to assign a non-screen object");
return;
}

lv_disp_t * old_disp = lv_obj_get_disp(scr);

if(old_disp == disp) return;

lv_ll_chg_list(&old_disp->scr_ll, &disp->scr_ll, scr, true);
}







lv_task_t * lv_disp_get_refr_task(lv_disp_t * disp)
{
if(!disp) disp = lv_disp_get_default();
if(!disp) {
LV_LOG_WARN("lv_disp_get_refr_task: no display registered");
return NULL;
}

return disp->refr_task;
}






uint32_t lv_disp_get_inactive_time(const lv_disp_t * disp)
{
if(!disp) disp = lv_disp_get_default();
if(!disp) {
LV_LOG_WARN("lv_disp_get_inactive_time: no display registered");
return 0;
}

if(disp) return lv_tick_elaps(disp->last_activity_time);

lv_disp_t * d;
uint32_t t = UINT32_MAX;
d = lv_disp_get_next(NULL);
while(d) {
t = LV_MATH_MIN(t, lv_tick_elaps(d->last_activity_time));
d = lv_disp_get_next(d);
}

return t;
}





void lv_disp_trig_activity(lv_disp_t * disp)
{
if(!disp) disp = lv_disp_get_default();
if(!disp) {
LV_LOG_WARN("lv_disp_trig_activity: no display registered");
return;
}

disp->last_activity_time = lv_tick_get();
}




