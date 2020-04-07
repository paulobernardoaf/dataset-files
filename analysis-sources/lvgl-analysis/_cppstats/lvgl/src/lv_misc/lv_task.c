#include <stddef.h>
#include "lv_task.h"
#include "../lv_core/lv_debug.h"
#include "../lv_hal/lv_hal_tick.h"
#include "lv_gc.h"
#if defined(LV_GC_INCLUDE)
#include LV_GC_INCLUDE
#endif 
#define IDLE_MEAS_PERIOD 500 
#define DEF_PRIO LV_TASK_PRIO_MID
#define DEF_PERIOD 500
static bool lv_task_exec(lv_task_t * task);
static bool lv_task_run = false;
static uint8_t idle_last = 0;
static bool task_deleted;
static bool task_created;
void lv_task_core_init(void)
{
lv_ll_init(&LV_GC_ROOT(_lv_task_ll), sizeof(lv_task_t));
lv_task_enable(true);
}
LV_ATTRIBUTE_TASK_HANDLER void lv_task_handler(void)
{
LV_LOG_TRACE("lv_task_handler started");
static bool already_running = false;
if(already_running) return;
already_running = true;
static uint32_t idle_period_start = 0;
static uint32_t handler_start = 0;
static uint32_t busy_time = 0;
if(lv_task_run == false) {
already_running = false; 
return;
}
handler_start = lv_tick_get();
lv_task_t * task_interrupter = NULL;
lv_task_t * next;
bool end_flag;
do {
end_flag = true;
task_deleted = false;
task_created = false;
LV_GC_ROOT(_lv_task_act) = lv_ll_get_head(&LV_GC_ROOT(_lv_task_ll));
while(LV_GC_ROOT(_lv_task_act)) {
next = lv_ll_get_next(&LV_GC_ROOT(_lv_task_ll), LV_GC_ROOT(_lv_task_act));
if(((lv_task_t *)LV_GC_ROOT(_lv_task_act))->prio == LV_TASK_PRIO_OFF) {
break;
}
if(LV_GC_ROOT(_lv_task_act) == task_interrupter) {
task_interrupter = NULL; 
LV_GC_ROOT(_lv_task_act) = next;
continue; 
}
if(((lv_task_t *)LV_GC_ROOT(_lv_task_act))->prio == LV_TASK_PRIO_HIGHEST) {
lv_task_exec(LV_GC_ROOT(_lv_task_act));
}
else if(task_interrupter) {
if(((lv_task_t *)LV_GC_ROOT(_lv_task_act))->prio > task_interrupter->prio) {
if(lv_task_exec(LV_GC_ROOT(_lv_task_act))) {
if(!task_created && !task_deleted) {
task_interrupter = LV_GC_ROOT(_lv_task_act);
end_flag = false;
break;
}
}
}
}
else {
if(lv_task_exec(LV_GC_ROOT(_lv_task_act))) {
if(!task_created && !task_deleted) {
task_interrupter = LV_GC_ROOT(_lv_task_act); 
end_flag = false;
break;
}
}
}
if(task_created || task_deleted) {
task_interrupter = NULL;
break;
}
LV_GC_ROOT(_lv_task_act) = next; 
}
} while(!end_flag);
busy_time += lv_tick_elaps(handler_start);
uint32_t idle_period_time = lv_tick_elaps(idle_period_start);
if(idle_period_time >= IDLE_MEAS_PERIOD) {
idle_last = (uint32_t)((uint32_t)busy_time * 100) / IDLE_MEAS_PERIOD; 
idle_last = idle_last > 100 ? 0 : 100 - idle_last; 
busy_time = 0;
idle_period_start = lv_tick_get();
}
already_running = false; 
LV_LOG_TRACE("lv_task_handler ready");
}
lv_task_t * lv_task_create_basic(void)
{
lv_task_t * new_task = NULL;
lv_task_t * tmp;
tmp = lv_ll_get_head(&LV_GC_ROOT(_lv_task_ll));
if(NULL == tmp) {
new_task = lv_ll_ins_head(&LV_GC_ROOT(_lv_task_ll));
LV_ASSERT_MEM(new_task);
if(new_task == NULL) return NULL;
}
else {
do {
if(tmp->prio <= DEF_PRIO) {
new_task = lv_ll_ins_prev(&LV_GC_ROOT(_lv_task_ll), tmp);
LV_ASSERT_MEM(new_task);
if(new_task == NULL) return NULL;
break;
}
tmp = lv_ll_get_next(&LV_GC_ROOT(_lv_task_ll), tmp);
} while(tmp != NULL);
if(tmp == NULL) {
new_task = lv_ll_ins_tail(&LV_GC_ROOT(_lv_task_ll));
LV_ASSERT_MEM(new_task);
if(new_task == NULL) return NULL;
}
}
new_task->period = DEF_PERIOD;
new_task->task_cb = NULL;
new_task->prio = DEF_PRIO;
new_task->once = 0;
new_task->last_run = lv_tick_get();
new_task->user_data = NULL;
task_created = true;
return new_task;
}
lv_task_t * lv_task_create(lv_task_cb_t task_cb, uint32_t period, lv_task_prio_t prio, void * user_data)
{
lv_task_t * new_task = lv_task_create_basic();
LV_ASSERT_MEM(new_task);
if(new_task == NULL) return NULL;
lv_task_set_cb(new_task, task_cb);
lv_task_set_period(new_task, period);
lv_task_set_prio(new_task, prio);
new_task->user_data = user_data;
return new_task;
}
void lv_task_set_cb(lv_task_t * task, lv_task_cb_t task_cb)
{
task->task_cb = task_cb;
}
void lv_task_del(lv_task_t * task)
{
lv_ll_rem(&LV_GC_ROOT(_lv_task_ll), task);
lv_mem_free(task);
if(LV_GC_ROOT(_lv_task_act) == task) task_deleted = true; 
}
void lv_task_set_prio(lv_task_t * task, lv_task_prio_t prio)
{
if(task->prio == prio) return;
lv_task_t * i;
LV_LL_READ(LV_GC_ROOT(_lv_task_ll), i)
{
if(i->prio <= prio) {
if(i != task) lv_ll_move_before(&LV_GC_ROOT(_lv_task_ll), task, i);
break;
}
}
if(i == NULL) {
lv_ll_move_before(&LV_GC_ROOT(_lv_task_ll), task, NULL);
}
task->prio = prio;
}
void lv_task_set_period(lv_task_t * task, uint32_t period)
{
task->period = period;
}
void lv_task_ready(lv_task_t * task)
{
task->last_run = lv_tick_get() - task->period - 1;
}
void lv_task_once(lv_task_t * task)
{
task->once = 1;
}
void lv_task_reset(lv_task_t * task)
{
task->last_run = lv_tick_get();
}
void lv_task_enable(bool en)
{
lv_task_run = en;
}
uint8_t lv_task_get_idle(void)
{
return idle_last;
}
static bool lv_task_exec(lv_task_t * task)
{
bool exec = false;
uint32_t elp = lv_tick_elaps(task->last_run);
if(elp >= task->period) {
task->last_run = lv_tick_get();
task_deleted = false;
task_created = false;
if(task->task_cb) task->task_cb(task);
if(task_deleted == false) { 
if(task->once != 0) {
lv_task_del(task);
}
}
exec = true;
}
return exec;
}
