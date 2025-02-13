








#include "lv_async.h"













static void lv_async_task_cb(lv_task_t *task);













lv_res_t lv_async_call(lv_async_cb_t async_xcb, void * user_data)
{

lv_async_info_t *info = lv_mem_alloc(sizeof(lv_async_info_t));

if(info == NULL)
return LV_RES_INV;



lv_task_t *task = lv_task_create(lv_async_task_cb, 0, LV_TASK_PRIO_HIGHEST, info);

if(task == NULL) {
lv_mem_free(info);
return LV_RES_INV;
}

info->cb = async_xcb;
info->user_data = user_data;


task->user_data = info;
lv_task_once(task);
return LV_RES_OK;
}





static void lv_async_task_cb(lv_task_t *task)
{
lv_async_info_t *info = (lv_async_info_t *)task->user_data;

info->cb(info->user_data);

lv_mem_free(info);
}
