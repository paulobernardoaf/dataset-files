#include "lv_indev.h"
#include "lv_disp.h"
#include "lv_obj.h"
#include "../lv_hal/lv_hal_tick.h"
#include "../lv_core/lv_group.h"
#include "../lv_core/lv_refr.h"
#include "../lv_misc/lv_task.h"
#include "../lv_misc/lv_math.h"
#if LV_INDEV_DEF_DRAG_THROW <= 0
#warning "LV_INDEV_DRAG_THROW must be greater than 0"
#endif
static void indev_pointer_proc(lv_indev_t * i, lv_indev_data_t * data);
static void indev_keypad_proc(lv_indev_t * i, lv_indev_data_t * data);
static void indev_encoder_proc(lv_indev_t * i, lv_indev_data_t * data);
static void indev_button_proc(lv_indev_t * i, lv_indev_data_t * data);
static void indev_proc_press(lv_indev_proc_t * proc);
static void indev_proc_release(lv_indev_proc_t * proc);
static void indev_proc_reset_query_handler(lv_indev_t * indev);
static lv_obj_t * indev_search_obj(const lv_indev_proc_t * proc, lv_obj_t * obj);
static void indev_drag(lv_indev_proc_t * state);
static void indev_drag_throw(lv_indev_proc_t * proc);
static bool indev_reset_check(lv_indev_proc_t * proc);
static lv_indev_t * indev_act;
static lv_obj_t * indev_obj_act = NULL;
void lv_indev_init(void)
{
lv_indev_reset(NULL); 
}
void lv_indev_read_task(lv_task_t * task)
{
LV_LOG_TRACE("indev read task started");
lv_indev_data_t data;
indev_act = task->user_data;
if(indev_act->driver.disp == NULL) return; 
indev_proc_reset_query_handler(indev_act);
if(indev_act->proc.disabled) return;
bool more_to_read;
do {
more_to_read = lv_indev_read(indev_act, &data);
indev_proc_reset_query_handler(indev_act);
indev_obj_act = NULL;
indev_act->proc.state = data.state;
if(indev_act->proc.state == LV_INDEV_STATE_PR) {
indev_act->driver.disp->last_activity_time = lv_tick_get();
} else if(indev_act->driver.type == LV_INDEV_TYPE_ENCODER && data.enc_diff) {
indev_act->driver.disp->last_activity_time = lv_tick_get();
}
if(indev_act->driver.type == LV_INDEV_TYPE_POINTER) {
indev_pointer_proc(indev_act, &data);
} else if(indev_act->driver.type == LV_INDEV_TYPE_KEYPAD) {
indev_keypad_proc(indev_act, &data);
} else if(indev_act->driver.type == LV_INDEV_TYPE_ENCODER) {
indev_encoder_proc(indev_act, &data);
} else if(indev_act->driver.type == LV_INDEV_TYPE_BUTTON) {
indev_button_proc(indev_act, &data);
}
indev_proc_reset_query_handler(indev_act);
} while(more_to_read);
indev_act = NULL;
indev_obj_act = NULL;
LV_LOG_TRACE("indev read task finished");
}
lv_indev_t * lv_indev_get_act(void)
{
return indev_act;
}
lv_indev_type_t lv_indev_get_type(const lv_indev_t * indev)
{
if(indev == NULL) return LV_INDEV_TYPE_NONE;
return indev->driver.type;
}
void lv_indev_reset(lv_indev_t * indev)
{
if(indev)
indev->proc.reset_query = 1;
else {
lv_indev_t * i = lv_indev_get_next(NULL);
while(i) {
i->proc.reset_query = 1;
i = lv_indev_get_next(i);
}
}
}
void lv_indev_reset_long_press(lv_indev_t * indev)
{
indev->proc.long_pr_sent = 0;
indev->proc.longpr_rep_timestamp = lv_tick_get();
indev->proc.pr_timestamp = lv_tick_get();
}
void lv_indev_enable(lv_indev_t * indev, bool en)
{
if(!indev) return;
indev->proc.disabled = en ? 0 : 1;
}
void lv_indev_set_cursor(lv_indev_t * indev, lv_obj_t * cur_obj)
{
if(indev->driver.type != LV_INDEV_TYPE_POINTER) return;
indev->cursor = cur_obj;
lv_obj_set_parent(indev->cursor, lv_disp_get_layer_sys(indev->driver.disp));
lv_obj_set_pos(indev->cursor, indev->proc.types.pointer.act_point.x, indev->proc.types.pointer.act_point.y);
}
#if LV_USE_GROUP
void lv_indev_set_group(lv_indev_t * indev, lv_group_t * group)
{
if(indev->driver.type == LV_INDEV_TYPE_KEYPAD || indev->driver.type == LV_INDEV_TYPE_ENCODER) {
indev->group = group;
}
}
#endif
void lv_indev_set_button_points(lv_indev_t * indev, const lv_point_t * points)
{
if(indev->driver.type == LV_INDEV_TYPE_BUTTON) {
indev->btn_points = points;
}
}
void lv_indev_get_point(const lv_indev_t * indev, lv_point_t * point)
{
if(indev->driver.type != LV_INDEV_TYPE_POINTER && indev->driver.type != LV_INDEV_TYPE_BUTTON) {
point->x = -1;
point->y = -1;
} else {
point->x = indev->proc.types.pointer.act_point.x;
point->y = indev->proc.types.pointer.act_point.y;
}
}
uint32_t lv_indev_get_key(const lv_indev_t * indev)
{
if(indev->driver.type != LV_INDEV_TYPE_KEYPAD)
return 0;
else
return indev->proc.types.keypad.last_key;
}
bool lv_indev_is_dragging(const lv_indev_t * indev)
{
if(indev == NULL) return false;
if(indev->driver.type != LV_INDEV_TYPE_POINTER && indev->driver.type != LV_INDEV_TYPE_BUTTON) return false;
return indev->proc.types.pointer.drag_in_prog == 0 ? false : true;
}
void lv_indev_get_vect(const lv_indev_t * indev, lv_point_t * point)
{
if(indev == NULL) {
point->x = 0;
point->y = 0;
return;
}
if(indev->driver.type != LV_INDEV_TYPE_POINTER && indev->driver.type != LV_INDEV_TYPE_BUTTON) {
point->x = 0;
point->y = 0;
} else {
point->x = indev->proc.types.pointer.vect.x;
point->y = indev->proc.types.pointer.vect.y;
}
}
void lv_indev_wait_release(lv_indev_t * indev)
{
if(indev == NULL)return;
indev->proc.wait_until_release = 1;
}
lv_task_t * lv_indev_get_read_task(lv_disp_t * indev)
{
if(!indev) {
LV_LOG_WARN("lv_indev_get_read_task: indev was NULL");
return NULL;
}
return indev->refr_task;
}
lv_obj_t * lv_indev_get_obj_act(void)
{
return indev_obj_act;
}
static void indev_pointer_proc(lv_indev_t * i, lv_indev_data_t * data)
{
if(i->cursor != NULL &&
(i->proc.types.pointer.last_point.x != data->point.x || i->proc.types.pointer.last_point.y != data->point.y)) {
lv_obj_set_pos(i->cursor, data->point.x, data->point.y);
}
i->proc.types.pointer.act_point.x = data->point.x;
i->proc.types.pointer.act_point.y = data->point.y;
if(i->proc.state == LV_INDEV_STATE_PR) {
indev_proc_press(&i->proc);
} else {
indev_proc_release(&i->proc);
}
i->proc.types.pointer.last_point.x = i->proc.types.pointer.act_point.x;
i->proc.types.pointer.last_point.y = i->proc.types.pointer.act_point.y;
}
static void indev_keypad_proc(lv_indev_t * i, lv_indev_data_t * data)
{
#if LV_USE_GROUP
if(data->state == LV_INDEV_STATE_PR && i->proc.wait_until_release) return;
if(i->proc.wait_until_release) {
i->proc.wait_until_release = 0;
i->proc.pr_timestamp = 0;
i->proc.long_pr_sent = 0;
i->proc.types.keypad.last_state = LV_INDEV_STATE_REL; 
}
lv_group_t * g = i->group;
if(g == NULL) return;
indev_obj_act = lv_group_get_focused(g);
if(indev_obj_act == NULL) return;
uint32_t prev_key = i->proc.types.keypad.last_key;
i->proc.types.keypad.last_key = data->key;
uint32_t prev_state = i->proc.types.keypad.last_state;
i->proc.types.keypad.last_state = data->state;
if(data->state == LV_INDEV_STATE_PR && prev_state == LV_INDEV_STATE_REL) {
i->proc.pr_timestamp = lv_tick_get();
if(data->key == LV_KEY_ENTER) {
lv_group_send_data(g, LV_KEY_ENTER);
indev_obj_act->signal_cb(indev_obj_act, LV_SIGNAL_PRESSED, NULL);
if(indev_reset_check(&i->proc)) return;
lv_event_send(indev_obj_act, LV_EVENT_PRESSED, NULL);
if(indev_reset_check(&i->proc)) return;
} else if(data->key == LV_KEY_ESC) {
lv_group_send_data(g, LV_KEY_ESC);
lv_event_send(indev_obj_act, LV_EVENT_CANCEL, NULL);
if(indev_reset_check(&i->proc)) return;
}
else if(data->key == LV_KEY_NEXT) {
lv_group_set_editing(g, false); 
lv_group_focus_next(g);
if(indev_reset_check(&i->proc)) return;
}
else if(data->key == LV_KEY_PREV) {
lv_group_set_editing(g, false); 
lv_group_focus_prev(g);
if(indev_reset_check(&i->proc)) return;
}
else {
lv_group_send_data(g, data->key);
}
}
else if(data->state == LV_INDEV_STATE_PR && prev_state == LV_INDEV_STATE_PR) {
if(i->proc.long_pr_sent == 0 && lv_tick_elaps(i->proc.pr_timestamp) > i->driver.long_press_time) {
i->proc.long_pr_sent = 1;
if(data->key == LV_KEY_ENTER) {
i->proc.longpr_rep_timestamp = lv_tick_get();
indev_obj_act->signal_cb(indev_obj_act, LV_SIGNAL_LONG_PRESS, NULL);
if(indev_reset_check(&i->proc)) return;
lv_event_send(indev_obj_act, LV_EVENT_LONG_PRESSED, NULL);
if(indev_reset_check(&i->proc)) return;
}
}
else if(i->proc.long_pr_sent != 0 &&
lv_tick_elaps(i->proc.longpr_rep_timestamp) > i->driver.long_press_rep_time) {
i->proc.longpr_rep_timestamp = lv_tick_get();
if(data->key == LV_KEY_ENTER) {
indev_obj_act->signal_cb(indev_obj_act, LV_SIGNAL_LONG_PRESS_REP, NULL);
if(indev_reset_check(&i->proc)) return;
lv_event_send(indev_obj_act, LV_EVENT_LONG_PRESSED_REPEAT, NULL);
if(indev_reset_check(&i->proc)) return;
}
else if(data->key == LV_KEY_NEXT) {
lv_group_set_editing(g, false); 
lv_group_focus_next(g);
if(indev_reset_check(&i->proc)) return;
}
else if(data->key == LV_KEY_PREV) {
lv_group_set_editing(g, false); 
lv_group_focus_prev(g);
if(indev_reset_check(&i->proc)) return;
}
else {
lv_group_send_data(g, data->key);
if(indev_reset_check(&i->proc)) return;
}
}
}
else if(data->state == LV_INDEV_STATE_REL && prev_state == LV_INDEV_STATE_PR) {
data->key = prev_key;
if(data->key == LV_KEY_ENTER) {
indev_obj_act->signal_cb(indev_obj_act, LV_SIGNAL_RELEASED, NULL);
if(indev_reset_check(&i->proc)) return;
if(i->proc.long_pr_sent == 0) {
lv_event_send(indev_obj_act, LV_EVENT_SHORT_CLICKED, NULL);
if(indev_reset_check(&i->proc)) return;
}
lv_event_send(indev_obj_act, LV_EVENT_CLICKED, NULL);
if(indev_reset_check(&i->proc)) return;
lv_event_send(indev_obj_act, LV_EVENT_RELEASED, NULL);
if(indev_reset_check(&i->proc)) return;
}
i->proc.pr_timestamp = 0;
i->proc.long_pr_sent = 0;
}
indev_obj_act = NULL;
#else
(void)data; 
(void)i; 
#endif
}
static void indev_encoder_proc(lv_indev_t * i, lv_indev_data_t * data)
{
#if LV_USE_GROUP
if(data->state == LV_INDEV_STATE_PR && i->proc.wait_until_release) return;
if(i->proc.wait_until_release) {
i->proc.wait_until_release = 0;
i->proc.pr_timestamp = 0;
i->proc.long_pr_sent = 0;
i->proc.types.keypad.last_state = LV_INDEV_STATE_REL; 
}
lv_indev_state_t last_state = i->proc.types.keypad.last_state;
i->proc.types.keypad.last_state = data->state;
i->proc.types.keypad.last_key = data->key;
lv_group_t * g = i->group;
if(g == NULL) return;
indev_obj_act = lv_group_get_focused(g);
if(indev_obj_act == NULL) return;
if(data->state == LV_INDEV_STATE_REL) {
if(lv_group_get_editing(g)) {
int32_t s;
if(data->enc_diff < 0) {
for(s = 0; s < -data->enc_diff; s++) lv_group_send_data(g, LV_KEY_LEFT);
} else if(data->enc_diff > 0) {
for(s = 0; s < data->enc_diff; s++) lv_group_send_data(g, LV_KEY_RIGHT);
}
}
else {
int32_t s;
if(data->enc_diff < 0) {
for(s = 0; s < -data->enc_diff; s++) lv_group_focus_prev(g);
} else if(data->enc_diff > 0) {
for(s = 0; s < data->enc_diff; s++) lv_group_focus_next(g);
}
}
}
indev_obj_act = lv_group_get_focused(g);
if(indev_obj_act == NULL) return;
if(data->state == LV_INDEV_STATE_PR && last_state == LV_INDEV_STATE_REL) {
bool editable = false;
indev_obj_act->signal_cb(indev_obj_act, LV_SIGNAL_GET_EDITABLE, &editable);
i->proc.pr_timestamp = lv_tick_get();
if(lv_group_get_editing(g) == true || editable == false) {
indev_obj_act->signal_cb(indev_obj_act, LV_SIGNAL_PRESSED, NULL);
if(indev_reset_check(&i->proc)) return;
lv_event_send(indev_obj_act, LV_EVENT_PRESSED, NULL);
if(indev_reset_check(&i->proc)) return;
}
}
else if(data->state == LV_INDEV_STATE_PR && last_state == LV_INDEV_STATE_PR) {
if(i->proc.long_pr_sent == 0 && lv_tick_elaps(i->proc.pr_timestamp) > i->driver.long_press_time) {
bool editable = false;
indev_obj_act->signal_cb(indev_obj_act, LV_SIGNAL_GET_EDITABLE, &editable);
if(editable) {
if(lv_ll_is_empty(&g->obj_ll) == false) {
lv_group_set_editing(g, lv_group_get_editing(g) ? false : true); 
}
}
else {
indev_obj_act->signal_cb(indev_obj_act, LV_SIGNAL_LONG_PRESS, NULL);
if(indev_reset_check(&i->proc)) return;
lv_event_send(indev_obj_act, LV_EVENT_LONG_PRESSED, NULL);
if(indev_reset_check(&i->proc)) return;
}
i->proc.long_pr_sent = 1;
}
}
else if(data->state == LV_INDEV_STATE_REL && last_state == LV_INDEV_STATE_PR) {
bool editable = false;
indev_obj_act->signal_cb(indev_obj_act, LV_SIGNAL_GET_EDITABLE, &editable);
if(editable == false) {
indev_obj_act->signal_cb(indev_obj_act, LV_SIGNAL_RELEASED, NULL);
if(indev_reset_check(&i->proc)) return;
if(i->proc.long_pr_sent == 0) lv_event_send(indev_obj_act, LV_EVENT_SHORT_CLICKED, NULL);
if(indev_reset_check(&i->proc)) return;
lv_event_send(indev_obj_act, LV_EVENT_CLICKED, NULL);
if(indev_reset_check(&i->proc)) return;
lv_event_send(indev_obj_act, LV_EVENT_RELEASED, NULL);
if(indev_reset_check(&i->proc)) return;
}
else if(g->editing) {
if(!i->proc.long_pr_sent || lv_ll_is_empty(&g->obj_ll)) {
indev_obj_act->signal_cb(indev_obj_act, LV_SIGNAL_RELEASED, NULL);
if(indev_reset_check(&i->proc)) return;
lv_event_send(indev_obj_act, LV_EVENT_SHORT_CLICKED, NULL);
if(indev_reset_check(&i->proc)) return;
lv_event_send(indev_obj_act, LV_EVENT_CLICKED, NULL);
if(indev_reset_check(&i->proc)) return;
lv_event_send(indev_obj_act, LV_EVENT_RELEASED, NULL);
if(indev_reset_check(&i->proc)) return;
lv_group_send_data(g, LV_KEY_ENTER);
}
}
else if(editable && !g->editing && !i->proc.long_pr_sent) {
lv_group_set_editing(g, true); 
}
i->proc.pr_timestamp = 0;
i->proc.long_pr_sent = 0;
}
indev_obj_act = NULL;
#else
(void)data; 
(void)i; 
#endif
}
static void indev_button_proc(lv_indev_t * i, lv_indev_data_t * data)
{
i->proc.types.pointer.act_point.x = i->btn_points[data->btn_id].x;
i->proc.types.pointer.act_point.y = i->btn_points[data->btn_id].y;
if(i->proc.types.pointer.last_point.x == i->proc.types.pointer.act_point.x &&
i->proc.types.pointer.last_point.y == i->proc.types.pointer.act_point.y && data->state == LV_INDEV_STATE_PR) {
indev_proc_press(&i->proc);
} else {
indev_proc_release(&i->proc);
}
i->proc.types.pointer.last_point.x = i->proc.types.pointer.act_point.x;
i->proc.types.pointer.last_point.y = i->proc.types.pointer.act_point.y;
}
static void indev_proc_press(lv_indev_proc_t * proc)
{
indev_obj_act = proc->types.pointer.act_obj;
if(proc->wait_until_release != 0) return;
lv_disp_t * disp = indev_act->driver.disp;
bool new_obj_searched = false;
if(indev_obj_act == NULL) {
indev_obj_act = indev_search_obj(proc, lv_disp_get_layer_sys(disp));
if(indev_obj_act == NULL) indev_obj_act = indev_search_obj(proc, lv_disp_get_layer_top(disp));
if(indev_obj_act == NULL) indev_obj_act = indev_search_obj(proc, lv_disp_get_scr_act(disp));
new_obj_searched = true;
}
else if(proc->types.pointer.drag_in_prog == 0 &&
lv_obj_is_protected(indev_obj_act, LV_PROTECT_PRESS_LOST) == false) {
indev_obj_act = indev_search_obj(proc, lv_disp_get_layer_sys(disp));
if(indev_obj_act == NULL) indev_obj_act = indev_search_obj(proc, lv_disp_get_layer_top(disp));
if(indev_obj_act == NULL) indev_obj_act = indev_search_obj(proc, lv_disp_get_scr_act(disp));
new_obj_searched = true;
}
else {
}
if(new_obj_searched && proc->types.pointer.last_obj) {
proc->types.pointer.drag_throw_vect.x = 0;
proc->types.pointer.drag_throw_vect.y = 0;
indev_drag_throw(proc);
}
if(indev_obj_act != proc->types.pointer.act_obj) {
proc->types.pointer.last_point.x = proc->types.pointer.act_point.x;
proc->types.pointer.last_point.y = proc->types.pointer.act_point.y;
if(proc->types.pointer.act_obj != NULL) {
lv_obj_t * last_obj = proc->types.pointer.act_obj;
last_obj->signal_cb(last_obj, LV_SIGNAL_PRESS_LOST, indev_act);
if(indev_reset_check(proc)) return;
lv_event_send(last_obj, LV_EVENT_PRESS_LOST, NULL);
if(indev_reset_check(proc)) return;
}
proc->types.pointer.act_obj = indev_obj_act; 
proc->types.pointer.last_obj = indev_obj_act;
if(indev_obj_act != NULL) {
proc->pr_timestamp = lv_tick_get();
proc->long_pr_sent = 0;
proc->types.pointer.drag_limit_out = 0;
proc->types.pointer.drag_in_prog = 0;
proc->types.pointer.drag_sum.x = 0;
proc->types.pointer.drag_sum.y = 0;
proc->types.pointer.vect.x = 0;
proc->types.pointer.vect.y = 0;
lv_obj_t * i = indev_obj_act;
lv_obj_t * last_top = NULL;
while(i != NULL) {
if(i->top) last_top = i;
i = lv_obj_get_parent(i);
}
if(last_top != NULL) {
lv_obj_move_foreground(last_top);
}
indev_obj_act->signal_cb(indev_obj_act, LV_SIGNAL_PRESSED, indev_act);
if(indev_reset_check(proc)) return;
lv_event_send(indev_obj_act, LV_EVENT_PRESSED, NULL);
if(indev_reset_check(proc)) return;
}
}
proc->types.pointer.vect.x = proc->types.pointer.act_point.x - proc->types.pointer.last_point.x;
proc->types.pointer.vect.y = proc->types.pointer.act_point.y - proc->types.pointer.last_point.y;
proc->types.pointer.drag_throw_vect.x = (proc->types.pointer.drag_throw_vect.x * 5) >> 3;
proc->types.pointer.drag_throw_vect.y = (proc->types.pointer.drag_throw_vect.y * 5) >> 3;
if(proc->types.pointer.drag_throw_vect.x < 0)
proc->types.pointer.drag_throw_vect.x++;
else if(proc->types.pointer.drag_throw_vect.x > 0)
proc->types.pointer.drag_throw_vect.x--;
if(proc->types.pointer.drag_throw_vect.y < 0)
proc->types.pointer.drag_throw_vect.y++;
else if(proc->types.pointer.drag_throw_vect.y > 0)
proc->types.pointer.drag_throw_vect.y--;
proc->types.pointer.drag_throw_vect.x += (proc->types.pointer.vect.x * 4) >> 3;
proc->types.pointer.drag_throw_vect.y += (proc->types.pointer.vect.y * 4) >> 3;
if(indev_obj_act != NULL) {
indev_obj_act->signal_cb(indev_obj_act, LV_SIGNAL_PRESSING, indev_act);
if(indev_reset_check(proc)) return;
lv_event_send(indev_obj_act, LV_EVENT_PRESSING, NULL);
if(indev_reset_check(proc)) return;
indev_drag(proc);
if(indev_reset_check(proc)) return;
if(proc->types.pointer.drag_in_prog == 0 && proc->long_pr_sent == 0) {
if(lv_tick_elaps(proc->pr_timestamp) > indev_act->driver.long_press_time) {
indev_obj_act->signal_cb(indev_obj_act, LV_SIGNAL_LONG_PRESS, indev_act);
if(indev_reset_check(proc)) return;
lv_event_send(indev_obj_act, LV_EVENT_LONG_PRESSED, NULL);
if(indev_reset_check(proc)) return;
proc->long_pr_sent = 1;
proc->longpr_rep_timestamp = lv_tick_get();
}
}
if(proc->types.pointer.drag_in_prog == 0 && proc->long_pr_sent == 1) {
if(lv_tick_elaps(proc->longpr_rep_timestamp) > indev_act->driver.long_press_rep_time) {
indev_obj_act->signal_cb(indev_obj_act, LV_SIGNAL_LONG_PRESS_REP, indev_act);
if(indev_reset_check(proc)) return;
lv_event_send(indev_obj_act, LV_EVENT_LONG_PRESSED_REPEAT, NULL);
if(indev_reset_check(proc)) return;
proc->longpr_rep_timestamp = lv_tick_get();
}
}
}
}
static void indev_proc_release(lv_indev_proc_t * proc)
{
if(proc->wait_until_release != 0) {
proc->types.pointer.act_obj = NULL;
proc->types.pointer.last_obj = NULL;
proc->pr_timestamp = 0;
proc->longpr_rep_timestamp = 0;
proc->wait_until_release = 0;
}
indev_obj_act = proc->types.pointer.act_obj;
if(indev_obj_act) {
if(lv_obj_is_protected(indev_obj_act, LV_PROTECT_PRESS_LOST)) {
indev_obj_act->signal_cb(indev_obj_act, LV_SIGNAL_RELEASED, indev_act);
if(indev_reset_check(proc)) return;
if(proc->types.pointer.drag_in_prog == 0) {
if(proc->long_pr_sent == 0) {
lv_event_send(indev_obj_act, LV_EVENT_SHORT_CLICKED, NULL);
if(indev_reset_check(proc)) return;
}
lv_event_send(indev_obj_act, LV_EVENT_CLICKED, NULL);
if(indev_reset_check(proc)) return;
}
lv_event_send(indev_obj_act, LV_EVENT_RELEASED, NULL);
if(indev_reset_check(proc)) return;
}
else {
indev_obj_act->signal_cb(indev_obj_act, LV_SIGNAL_RELEASED, indev_act);
if(indev_reset_check(proc)) return;
if(proc->long_pr_sent == 0 && proc->types.pointer.drag_in_prog == 0) {
lv_event_send(indev_obj_act, LV_EVENT_SHORT_CLICKED, NULL);
if(indev_reset_check(proc)) return;
}
if(proc->types.pointer.drag_in_prog == 0) {
lv_event_send(indev_obj_act, LV_EVENT_CLICKED, NULL);
if(indev_reset_check(proc)) return;
}
lv_event_send(indev_obj_act, LV_EVENT_RELEASED, NULL);
if(indev_reset_check(proc)) return;
}
if(indev_reset_check(proc)) return;
bool click_focus_sent = false;
#if LV_USE_GROUP
lv_group_t * g = lv_obj_get_group(indev_obj_act);
if(lv_obj_is_protected(indev_obj_act, LV_PROTECT_CLICK_FOCUS) == false) {
lv_obj_t * parent = indev_obj_act;
while(g == NULL) {
parent = lv_obj_get_parent(parent);
if(parent == NULL) break;
if(lv_obj_is_protected(parent, LV_PROTECT_CLICK_FOCUS)) {
parent = NULL;
break;
}
g = lv_obj_get_group(parent);
}
if(g && parent) {
if(lv_group_get_click_focus(g)) {
click_focus_sent = true;
lv_group_focus_obj(parent);
}
}
}
#endif
if(proc->types.pointer.last_pressed != indev_obj_act && click_focus_sent == false) {
lv_event_send(proc->types.pointer.last_pressed, LV_EVENT_DEFOCUSED, NULL);
if(indev_reset_check(proc)) return;
lv_event_send(proc->types.pointer.act_obj, LV_EVENT_FOCUSED, NULL);
if(indev_reset_check(proc)) return;
proc->types.pointer.last_pressed = indev_obj_act;
}
if(indev_reset_check(proc)) return;
lv_obj_t * drag_obj = indev_obj_act;
while(lv_obj_get_drag_parent(drag_obj) != false && drag_obj != NULL) {
drag_obj = lv_obj_get_parent(drag_obj);
}
if(drag_obj) {
if(lv_obj_get_drag_throw(drag_obj) && proc->types.pointer.drag_in_prog) {
lv_event_send(drag_obj, LV_EVENT_DRAG_THROW_BEGIN, NULL);
if(indev_reset_check(proc)) return;
}
}
proc->types.pointer.act_obj = NULL;
proc->pr_timestamp = 0;
proc->longpr_rep_timestamp = 0;
}
if(proc->types.pointer.last_obj != NULL && proc->reset_query == 0) {
indev_drag_throw(proc);
if(indev_reset_check(proc)) return;
}
}
static void indev_proc_reset_query_handler(lv_indev_t * indev)
{
if(indev->proc.reset_query) {
indev->proc.types.pointer.act_obj = NULL;
indev->proc.types.pointer.last_obj = NULL;
indev->proc.types.pointer.last_pressed = NULL;
indev->proc.types.pointer.drag_limit_out = 0;
indev->proc.types.pointer.drag_in_prog = 0;
indev->proc.long_pr_sent = 0;
indev->proc.pr_timestamp = 0;
indev->proc.longpr_rep_timestamp = 0;
indev->proc.types.pointer.drag_sum.x = 0;
indev->proc.types.pointer.drag_sum.y = 0;
indev->proc.types.pointer.drag_throw_vect.x = 0;
indev->proc.types.pointer.drag_throw_vect.y = 0;
indev->proc.reset_query = 0;
indev_obj_act = NULL;
}
}
static lv_obj_t * indev_search_obj(const lv_indev_proc_t * proc, lv_obj_t * obj)
{
lv_obj_t * found_p = NULL;
#if LV_USE_EXT_CLICK_AREA == LV_EXT_CLICK_AREA_TINY
lv_area_t ext_area;
ext_area.x1 = obj->coords.x1 - obj->ext_click_pad_hor;
ext_area.x2 = obj->coords.x2 + obj->ext_click_pad_hor;
ext_area.y1 = obj->coords.y1 - obj->ext_click_pad_ver;
ext_area.y2 = obj->coords.y2 + obj->ext_click_pad_ver;
if(lv_area_is_point_on(&ext_area, &proc->types.pointer.act_point)) {
#elif LV_USE_EXT_CLICK_AREA == LV_EXT_CLICK_AREA_FULL
lv_area_t ext_area;
ext_area.x1 = obj->coords.x1 - obj->ext_click_pad.x1;
ext_area.x2 = obj->coords.x2 + obj->ext_click_pad.x2;
ext_area.y1 = obj->coords.y1 - obj->ext_click_pad.y1;
ext_area.y2 = obj->coords.y2 + obj->ext_click_pad.y2;
if(lv_area_is_point_on(&ext_area, &proc->types.pointer.act_point)) {
#else
if(lv_area_is_point_on(&obj->coords, &proc->types.pointer.act_point)) {
#endif
lv_obj_t * i;
LV_LL_READ(obj->child_ll, i)
{
found_p = indev_search_obj(proc, i);
if(found_p != NULL) {
break;
}
}
if(found_p == NULL && lv_obj_get_click(obj) != false) {
lv_obj_t * hidden_i = obj;
while(hidden_i != NULL) {
if(lv_obj_get_hidden(hidden_i) == true) break;
hidden_i = lv_obj_get_parent(hidden_i);
}
if(hidden_i == NULL) found_p = obj;
}
}
return found_p;
}
static void indev_drag(lv_indev_proc_t * state)
{
lv_obj_t * drag_obj = state->types.pointer.act_obj;
bool drag_just_started = false;
while(lv_obj_get_drag_parent(drag_obj) != false && drag_obj != NULL) {
drag_obj = lv_obj_get_parent(drag_obj);
}
if(drag_obj == NULL) return;
if(lv_obj_get_drag(drag_obj) == false) return;
lv_drag_dir_t allowed_dirs = lv_obj_get_drag_dir(drag_obj);
state->types.pointer.drag_sum.x += state->types.pointer.vect.x;
state->types.pointer.drag_sum.y += state->types.pointer.vect.y;
if(state->types.pointer.drag_limit_out == 0) {
if(((allowed_dirs & LV_DRAG_DIR_HOR) &&
LV_MATH_ABS(state->types.pointer.drag_sum.x) >= indev_act->driver.drag_limit) ||
((allowed_dirs & LV_DRAG_DIR_VER) &&
LV_MATH_ABS(state->types.pointer.drag_sum.y) >= indev_act->driver.drag_limit)) {
state->types.pointer.drag_limit_out = 1;
drag_just_started = true;
}
}
if(state->types.pointer.drag_limit_out != 0) {
if(state->types.pointer.vect.x != 0 || state->types.pointer.vect.y != 0) {
uint16_t inv_buf_size =
lv_disp_get_inv_buf_size(indev_act->driver.disp); 
lv_coord_t prev_x = drag_obj->coords.x1;
lv_coord_t prev_y = drag_obj->coords.y1;
lv_coord_t prev_par_w = lv_obj_get_width(lv_obj_get_parent(drag_obj));
lv_coord_t prev_par_h = lv_obj_get_height(lv_obj_get_parent(drag_obj));
lv_coord_t act_x = lv_obj_get_x(drag_obj);
lv_coord_t act_y = lv_obj_get_y(drag_obj);
if(allowed_dirs == LV_DRAG_DIR_ALL) {
if(drag_just_started) {
act_x += state->types.pointer.drag_sum.x;
act_y += state->types.pointer.drag_sum.y;
}
lv_obj_set_pos(drag_obj, act_x + state->types.pointer.vect.x, act_y + state->types.pointer.vect.y);
} else if(allowed_dirs & LV_DRAG_DIR_HOR) {
if(drag_just_started) {
act_x += state->types.pointer.drag_sum.x;
}
lv_obj_set_x(drag_obj, act_x + state->types.pointer.vect.x);
} else if(allowed_dirs & LV_DRAG_DIR_VER) {
if(drag_just_started) {
act_y += state->types.pointer.drag_sum.y;
}
lv_obj_set_y(drag_obj, act_y + state->types.pointer.vect.y);
}
if(drag_obj->coords.x1 == prev_x && drag_obj->coords.y1 == prev_y) {
lv_coord_t act_par_w = lv_obj_get_width(lv_obj_get_parent(drag_obj));
lv_coord_t act_par_h = lv_obj_get_height(lv_obj_get_parent(drag_obj));
if(act_par_w == prev_par_w && act_par_h == prev_par_h) {
uint16_t new_inv_buf_size = lv_disp_get_inv_buf_size(indev_act->driver.disp);
lv_disp_pop_from_inv_buf(indev_act->driver.disp, new_inv_buf_size - inv_buf_size);
}
} else {
state->types.pointer.drag_in_prog = 1;
if(drag_just_started) {
drag_obj->signal_cb(drag_obj, LV_SIGNAL_DRAG_BEGIN, indev_act);
if(indev_reset_check(state)) return;
lv_event_send(drag_obj, LV_EVENT_DRAG_BEGIN, NULL);
if(indev_reset_check(state)) return;
}
}
}
}
}
static void indev_drag_throw(lv_indev_proc_t * proc)
{
if(proc->types.pointer.drag_in_prog == 0) return;
lv_obj_t * drag_obj = proc->types.pointer.last_obj;
while(lv_obj_get_drag_parent(drag_obj) != false && drag_obj != NULL) {
drag_obj = lv_obj_get_parent(drag_obj);
}
if(drag_obj == NULL) {
return;
}
if(lv_obj_get_drag_throw(drag_obj) == false) {
proc->types.pointer.drag_in_prog = 0;
drag_obj->signal_cb(drag_obj, LV_SIGNAL_DRAG_END, indev_act);
lv_event_send(drag_obj, LV_EVENT_DRAG_END, NULL);
if(indev_reset_check(proc)) return;
lv_event_send(drag_obj, LV_EVENT_DRAG_END, NULL);
return;
}
lv_drag_dir_t allowed_dirs = lv_obj_get_drag_dir(drag_obj);
proc->types.pointer.drag_throw_vect.x =
proc->types.pointer.drag_throw_vect.x * (100 - indev_act->driver.drag_throw) / 100;
proc->types.pointer.drag_throw_vect.y =
proc->types.pointer.drag_throw_vect.y * (100 - indev_act->driver.drag_throw) / 100;
if(proc->types.pointer.drag_throw_vect.x != 0 || proc->types.pointer.drag_throw_vect.y != 0) {
lv_area_t coords_ori;
lv_obj_get_coords(drag_obj, &coords_ori);
lv_coord_t act_x = lv_obj_get_x(drag_obj) + proc->types.pointer.drag_throw_vect.x;
lv_coord_t act_y = lv_obj_get_y(drag_obj) + proc->types.pointer.drag_throw_vect.y;
if(allowed_dirs == LV_DRAG_DIR_ALL)
lv_obj_set_pos(drag_obj, act_x, act_y);
else if(allowed_dirs & LV_DRAG_DIR_HOR)
lv_obj_set_x(drag_obj, act_x);
else if(allowed_dirs & LV_DRAG_DIR_VER)
lv_obj_set_y(drag_obj, act_y);
lv_area_t coord_new;
lv_obj_get_coords(drag_obj, &coord_new);
if((coords_ori.x1 == coord_new.x1 || proc->types.pointer.drag_throw_vect.x == 0) &&
(coords_ori.y1 == coord_new.y1 || proc->types.pointer.drag_throw_vect.y == 0)) {
proc->types.pointer.drag_in_prog = 0;
proc->types.pointer.vect.x = 0;
proc->types.pointer.vect.y = 0;
proc->types.pointer.drag_throw_vect.x = 0;
proc->types.pointer.drag_throw_vect.y = 0;
drag_obj->signal_cb(drag_obj, LV_SIGNAL_DRAG_END, indev_act);
if(indev_reset_check(proc)) return;
lv_event_send(drag_obj, LV_EVENT_DRAG_END, NULL);
if(indev_reset_check(proc)) return;
}
}
else {
proc->types.pointer.drag_in_prog = 0;
drag_obj->signal_cb(drag_obj, LV_SIGNAL_DRAG_END, indev_act);
if(indev_reset_check(proc)) return;
lv_event_send(drag_obj, LV_EVENT_DRAG_END, NULL);
if(indev_reset_check(proc)) return;
}
}
static bool indev_reset_check(lv_indev_proc_t * proc)
{
if(proc->reset_query) {
indev_obj_act = NULL;
}
return proc->reset_query ? true : false;
}
