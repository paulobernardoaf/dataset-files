







#include "lv_anim.h"

#if LV_USE_ANIMATION
#include <stddef.h>
#include <string.h>
#include "../lv_core/lv_debug.h"
#include "../lv_hal/lv_hal_tick.h"
#include "lv_task.h"
#include "lv_math.h"
#include "lv_gc.h"

#if defined(LV_GC_INCLUDE)
#include LV_GC_INCLUDE
#endif 




#define LV_ANIM_RESOLUTION 1024
#define LV_ANIM_RES_SHIFT 10








static void anim_task(lv_task_t * param);
static bool anim_ready_handler(lv_anim_t * a);




static uint32_t last_task_run;
static bool anim_list_changed;












void lv_anim_core_init(void)
{
lv_ll_init(&LV_GC_ROOT(_lv_anim_ll), sizeof(lv_anim_t));
last_task_run = lv_tick_get();
lv_task_create(anim_task, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, NULL);
}










void lv_anim_init(lv_anim_t * a)
{
memset(a, 0, sizeof(lv_anim_t));
a->time = 500;
a->start = 0;
a->end = 100;
a->path_cb = lv_anim_path_linear;
}




void lv_anim_create(lv_anim_t * a)
{
LV_LOG_TRACE("animation create started")

if(a->exec_cb != NULL) lv_anim_del(a->var, a->exec_cb); 


lv_anim_t * new_anim = lv_ll_ins_head(&LV_GC_ROOT(_lv_anim_ll));
LV_ASSERT_MEM(new_anim);
if(new_anim == NULL) return;


a->playback_now = 0;
memcpy(new_anim, a, sizeof(lv_anim_t));


if(new_anim->exec_cb) new_anim->exec_cb(new_anim->var, new_anim->start);



anim_list_changed = true;

LV_LOG_TRACE("animation created")
}








bool lv_anim_del(void * var, lv_anim_exec_xcb_t exec_cb)
{
lv_anim_t * a;
lv_anim_t * a_next;
bool del = false;
a = lv_ll_get_head(&LV_GC_ROOT(_lv_anim_ll));
while(a != NULL) {

a_next = lv_ll_get_next(&LV_GC_ROOT(_lv_anim_ll), a);

if(a->var == var && (a->exec_cb == exec_cb || exec_cb == NULL)) {
lv_ll_rem(&LV_GC_ROOT(_lv_anim_ll), a);
lv_mem_free(a);
anim_list_changed = true; 

del = true;
}

a = a_next;
}

return del;
}





uint16_t lv_anim_count_running(void)
{
uint16_t cnt = 0;
lv_anim_t * a;
LV_LL_READ(LV_GC_ROOT(_lv_anim_ll), a) cnt++;

return cnt++;
}








uint16_t lv_anim_speed_to_time(uint16_t speed, lv_anim_value_t start, lv_anim_value_t end)
{
int32_t d = LV_MATH_ABS((int32_t)start - end);
uint32_t time = (int32_t)((int32_t)(d * 1000) / speed);

if(time > UINT16_MAX) time = UINT16_MAX;

if(time == 0) {
time++;
}

return time;
}






lv_anim_value_t lv_anim_path_linear(const lv_anim_t * a)
{

uint32_t step;
if(a->time == a->act_time) {
step = LV_ANIM_RESOLUTION; 
} else {
step = ((int32_t)a->act_time * LV_ANIM_RESOLUTION) / a->time;
}



int32_t new_value;
new_value = (int32_t)step * (a->end - a->start);
new_value = new_value >> LV_ANIM_RES_SHIFT;
new_value += a->start;

return (lv_anim_value_t)new_value;
}






lv_anim_value_t lv_anim_path_ease_in(const lv_anim_t * a)
{

uint32_t t;
if(a->time == a->act_time)
t = 1024;
else
t = (uint32_t)((uint32_t)a->act_time * 1024) / a->time;

int32_t step = lv_bezier3(t, 0, 1, 1, 1024);

int32_t new_value;
new_value = (int32_t)step * (a->end - a->start);
new_value = new_value >> 10;
new_value += a->start;

return (lv_anim_value_t)new_value;
}






lv_anim_value_t lv_anim_path_ease_out(const lv_anim_t * a)
{


uint32_t t;
if(a->time == a->act_time)
t = 1024;
else
t = (uint32_t)((uint32_t)a->act_time * 1024) / a->time;

int32_t step = lv_bezier3(t, 0, 1023, 1023, 1024);

int32_t new_value;
new_value = (int32_t)step * (a->end - a->start);
new_value = new_value >> 10;
new_value += a->start;

return (lv_anim_value_t)new_value;
}






lv_anim_value_t lv_anim_path_ease_in_out(const lv_anim_t * a)
{


uint32_t t;
if(a->time == a->act_time)
t = 1024;
else
t = (uint32_t)((uint32_t)a->act_time * 1024) / a->time;

int32_t step = lv_bezier3(t, 0, 100, 924, 1024);

int32_t new_value;
new_value = (int32_t)step * (a->end - a->start);
new_value = new_value >> 10;
new_value += a->start;

return (lv_anim_value_t)new_value;
}






lv_anim_value_t lv_anim_path_overshoot(const lv_anim_t * a)
{


uint32_t t;
if(a->time == a->act_time)
t = 1024;
else
t = (uint32_t)((uint32_t)a->act_time * 1024) / a->time;

int32_t step = lv_bezier3(t, 0, 600, 1300, 1024);

int32_t new_value;
new_value = (int32_t)step * (a->end - a->start);
new_value = new_value >> 10;
new_value += a->start;

return (lv_anim_value_t)new_value;
}






lv_anim_value_t lv_anim_path_bounce(const lv_anim_t * a)
{

uint32_t t;
if(a->time == a->act_time)
t = 1024;
else
t = (uint32_t)((uint32_t)a->act_time * 1024) / a->time;

int32_t diff = (a->end - a->start);



if(t < 408) {

t = (t * 2500) >> 10; 
} else if(t >= 408 && t < 614) {

t -= 408;
t = t * 5; 
t = 1024 - t;
diff = diff / 6;
} else if(t >= 614 && t < 819) {

t -= 614;
t = t * 5; 
diff = diff / 6;
} else if(t >= 819 && t < 921) {

t -= 819;
t = t * 10; 
t = 1024 - t;
diff = diff / 16;
} else if(t >= 921 && t <= 1024) {

t -= 921;
t = t * 10; 
diff = diff / 16;
}

if(t > 1024) t = 1024;

int32_t step = lv_bezier3(t, 1024, 1024, 800, 0);

int32_t new_value;
new_value = (int32_t)step * diff;
new_value = new_value >> 10;
new_value = a->end - new_value;

return (lv_anim_value_t)new_value;
}







lv_anim_value_t lv_anim_path_step(const lv_anim_t * a)
{
if(a->act_time >= a->time)
return a->end;
else
return a->start;
}









static void anim_task(lv_task_t * param)
{
(void)param;

lv_anim_t * a;
LV_LL_READ(LV_GC_ROOT(_lv_anim_ll), a)
{
a->has_run = 0;
}

uint32_t elaps = lv_tick_elaps(last_task_run);

a = lv_ll_get_head(&LV_GC_ROOT(_lv_anim_ll));

while(a != NULL) {




anim_list_changed = false;

if(!a->has_run) {
a->has_run = 1; 
a->act_time += elaps;
if(a->act_time >= 0) {
if(a->act_time > a->time) a->act_time = a->time;

int32_t new_value;
new_value = a->path_cb(a);


if(a->exec_cb) a->exec_cb(a->var, new_value);


if(a->act_time >= a->time) {
anim_ready_handler(a);
}
}
}



if(anim_list_changed)
a = lv_ll_get_head(&LV_GC_ROOT(_lv_anim_ll));
else
a = lv_ll_get_next(&LV_GC_ROOT(_lv_anim_ll), a);
}

last_task_run = lv_tick_get();
}







static bool anim_ready_handler(lv_anim_t * a)
{




if((a->repeat == 0 && a->playback == 0) || (a->repeat == 0 && a->playback == 1 && a->playback_now == 1)) {



lv_anim_t a_tmp;
memcpy(&a_tmp, a, sizeof(lv_anim_t));
lv_ll_rem(&LV_GC_ROOT(_lv_anim_ll), a);
lv_mem_free(a);
anim_list_changed = true;


if(a_tmp.ready_cb != NULL) a_tmp.ready_cb(&a_tmp);
}

else {
a->act_time = -a->repeat_pause; 

if(a->playback != 0) {

if(a->playback_now == 0) a->act_time = -a->playback_pause;


a->playback_now = a->playback_now == 0 ? 1 : 0;

int32_t tmp;
tmp = a->start;
a->start = a->end;
a->end = tmp;
}
}

return anim_list_changed;
}
#endif
