#include <stdint.h>
#include <stddef.h>
#include "lv_hal.h"
#include "../lv_core/lv_debug.h"
#include "../lv_misc/lv_mem.h"
#include "../lv_core/lv_obj.h"
#include "../lv_core/lv_refr.h"
#include "../lv_misc/lv_gc.h"
#if defined(LV_GC_INCLUDE)
#include LV_GC_INCLUDE
#endif 
static lv_disp_t * disp_def;
void lv_disp_drv_init(lv_disp_drv_t * driver)
{
memset(driver, 0, sizeof(lv_disp_drv_t));
driver->flush_cb = NULL;
driver->hor_res = LV_HOR_RES_MAX;
driver->ver_res = LV_VER_RES_MAX;
driver->buffer = NULL;
driver->rotated = 0;
driver->color_chroma_key = LV_COLOR_TRANSP;
#if LV_ANTIALIAS
driver->antialiasing = true;
#endif
#if LV_COLOR_SCREEN_TRANSP
driver->screen_transp = 1;
#endif
#if LV_USE_GPU
driver->gpu_blend_cb = NULL;
driver->gpu_fill_cb = NULL;
#endif
#if LV_USE_USER_DATA
driver->user_data = NULL;
#endif
driver->set_px_cb = NULL;
}
void lv_disp_buf_init(lv_disp_buf_t * disp_buf, void * buf1, void * buf2, uint32_t size_in_px_cnt)
{
memset(disp_buf, 0, sizeof(lv_disp_buf_t));
disp_buf->buf1 = buf1;
disp_buf->buf2 = buf2;
disp_buf->buf_act = disp_buf->buf1;
disp_buf->size = size_in_px_cnt;
}
lv_disp_t * lv_disp_drv_register(lv_disp_drv_t * driver)
{
lv_disp_t * disp = lv_ll_ins_head(&LV_GC_ROOT(_lv_disp_ll));
if(!disp) {
LV_ASSERT_MEM(disp);
return NULL;
}
memcpy(&disp->driver, driver, sizeof(lv_disp_drv_t));
memset(&disp->inv_area_joined, 0, sizeof(disp->inv_area_joined));
memset(&disp->inv_areas, 0, sizeof(disp->inv_areas));
lv_ll_init(&disp->scr_ll, sizeof(lv_obj_t));
disp->last_activity_time = 0;
if(disp_def == NULL) disp_def = disp;
lv_disp_t * disp_def_tmp = disp_def;
disp_def = disp; 
disp->inv_p = 0;
disp->act_scr = lv_obj_create(NULL, NULL); 
disp->top_layer = lv_obj_create(NULL, NULL); 
disp->sys_layer = lv_obj_create(NULL, NULL); 
lv_obj_set_style(disp->top_layer, &lv_style_transp);
lv_obj_set_style(disp->sys_layer, &lv_style_transp);
lv_obj_invalidate(disp->act_scr);
disp_def = disp_def_tmp; 
disp->refr_task = lv_task_create(lv_disp_refr_task, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, disp);
LV_ASSERT_MEM(disp->refr_task);
if(disp->refr_task == NULL) return NULL;
lv_task_ready(disp->refr_task); 
return disp;
}
void lv_disp_drv_update(lv_disp_t * disp, lv_disp_drv_t * new_drv)
{
memcpy(&disp->driver, new_drv, sizeof(lv_disp_drv_t));
lv_obj_t * scr;
LV_LL_READ(disp->scr_ll, scr)
{
lv_obj_set_size(scr, lv_disp_get_hor_res(disp), lv_disp_get_ver_res(disp));
}
}
void lv_disp_remove(lv_disp_t * disp)
{
bool was_default = false;
if(disp == lv_disp_get_default()) was_default = true;
lv_indev_t * indev;
indev = lv_indev_get_next(NULL);
while(indev) {
if(indev->driver.disp == disp) {
indev->driver.disp = NULL;
}
indev = lv_indev_get_next(indev);
}
lv_ll_rem(&LV_GC_ROOT(_lv_disp_ll), disp);
lv_mem_free(disp);
if(was_default) lv_disp_set_default(lv_ll_get_head(&LV_GC_ROOT(_lv_disp_ll)));
}
void lv_disp_set_default(lv_disp_t * disp)
{
disp_def = disp;
}
lv_disp_t * lv_disp_get_default(void)
{
return disp_def;
}
lv_coord_t lv_disp_get_hor_res(lv_disp_t * disp)
{
if(disp == NULL) disp = lv_disp_get_default();
if(disp == NULL)
return LV_HOR_RES_MAX;
else
return disp->driver.rotated == 0 ? disp->driver.hor_res : disp->driver.ver_res;
}
lv_coord_t lv_disp_get_ver_res(lv_disp_t * disp)
{
if(disp == NULL) disp = lv_disp_get_default();
if(disp == NULL)
return LV_VER_RES_MAX;
else
return disp->driver.rotated == 0 ? disp->driver.ver_res : disp->driver.hor_res;
}
bool lv_disp_get_antialiasing(lv_disp_t * disp)
{
#if LV_ANTIALIAS == 0
return false;
#else
if(disp == NULL) disp = lv_disp_get_default();
if(disp == NULL) return false;
return disp->driver.antialiasing ? true : false;
#endif
}
LV_ATTRIBUTE_FLUSH_READY void lv_disp_flush_ready(lv_disp_drv_t * disp_drv)
{
#if LV_COLOR_SCREEN_TRANSP
if(disp_drv->screen_transp) {
memset(disp_drv->buffer->buf_act, 0x00, disp_drv->buffer->size * sizeof(lv_color32_t));
}
#endif
disp_drv->buffer->flushing = 0;
}
lv_disp_t * lv_disp_get_next(lv_disp_t * disp)
{
if(disp == NULL)
return lv_ll_get_head(&LV_GC_ROOT(_lv_disp_ll));
else
return lv_ll_get_next(&LV_GC_ROOT(_lv_disp_ll), disp);
}
lv_disp_buf_t * lv_disp_get_buf(lv_disp_t * disp)
{
return disp->driver.buffer;
}
uint16_t lv_disp_get_inv_buf_size(lv_disp_t * disp)
{
return disp->inv_p;
}
void lv_disp_pop_from_inv_buf(lv_disp_t * disp, uint16_t num)
{
if(disp->inv_p < num)
disp->inv_p = 0;
else
disp->inv_p -= num;
}
bool lv_disp_is_double_buf(lv_disp_t * disp)
{
if(disp->driver.buffer->buf1 && disp->driver.buffer->buf2)
return true;
else
return false;
}
bool lv_disp_is_true_double_buf(lv_disp_t * disp)
{
uint32_t scr_size = disp->driver.hor_res * disp->driver.ver_res;
if(lv_disp_is_double_buf(disp) && disp->driver.buffer->size == scr_size) {
return true;
} else {
return false;
}
}
