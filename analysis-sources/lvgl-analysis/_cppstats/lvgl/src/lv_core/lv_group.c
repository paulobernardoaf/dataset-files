#include "lv_group.h"
#if LV_USE_GROUP != 0
#include <stddef.h>
#include "../lv_core/lv_debug.h"
#include "../lv_themes/lv_theme.h"
#include "../lv_misc/lv_gc.h"
#if defined(LV_GC_INCLUDE)
#include LV_GC_INCLUDE
#endif 
static void style_mod_def(lv_group_t * group, lv_style_t * style);
static void style_mod_edit_def(lv_group_t * group, lv_style_t * style);
static void refresh_theme(lv_group_t * g, lv_theme_t * th);
static void focus_next_core(lv_group_t * group, void * (*begin)(const lv_ll_t *),
void * (*move)(const lv_ll_t *, const void *));
static void lv_group_refocus(lv_group_t * g);
static void obj_to_foreground(lv_obj_t * obj);
void lv_group_init(void)
{
lv_ll_init(&LV_GC_ROOT(_lv_group_ll), sizeof(lv_group_t));
}
lv_group_t * lv_group_create(void)
{
lv_group_t * group = lv_ll_ins_head(&LV_GC_ROOT(_lv_group_ll));
LV_ASSERT_MEM(group);
if(group == NULL) return NULL;
lv_ll_init(&group->obj_ll, sizeof(lv_obj_t *));
group->obj_focus = NULL;
group->frozen = 0;
group->focus_cb = NULL;
group->click_focus = 1;
group->editing = 0;
group->refocus_policy = LV_GROUP_REFOCUS_POLICY_PREV;
group->wrap = 1;
#if LV_USE_USER_DATA
memset(&group->user_data, 0, sizeof(lv_group_user_data_t));
#endif
refresh_theme(group, lv_theme_get_current());
return group;
}
void lv_group_del(lv_group_t * group)
{
if(group->obj_focus != NULL) {
(*group->obj_focus)->signal_cb(*group->obj_focus, LV_SIGNAL_DEFOCUS, NULL);
lv_obj_invalidate(*group->obj_focus);
}
lv_obj_t ** obj;
LV_LL_READ(group->obj_ll, obj)
{
(*obj)->group_p = NULL;
}
lv_ll_clear(&(group->obj_ll));
lv_ll_rem(&LV_GC_ROOT(_lv_group_ll), group);
lv_mem_free(group);
}
void lv_group_add_obj(lv_group_t * group, lv_obj_t * obj)
{
if(group == NULL) return;
lv_obj_t ** obj_i;
LV_LL_READ(group->obj_ll, obj_i)
{
if((*obj_i) == obj) {
LV_LOG_INFO("lv_group_add_obj: the object is already added to this group");
return;
}
}
if(obj->group_p) {
if(lv_obj_is_focused(obj)) {
lv_group_refocus(obj->group_p);
LV_LOG_INFO("lv_group_add_obj: assign object to an other group");
}
}
obj->group_p = group;
lv_obj_t ** next = lv_ll_ins_tail(&group->obj_ll);
LV_ASSERT_MEM(next);
if(next == NULL) return;
*next = obj;
if(lv_ll_get_head(&group->obj_ll) == next) {
lv_group_refocus(group);
}
}
void lv_group_remove_obj(lv_obj_t * obj)
{
lv_group_t * g = obj->group_p;
if(g == NULL) return;
if(g->obj_focus == NULL) return; 
if(*g->obj_focus == obj) {
if(lv_ll_get_head(&g->obj_ll) == g->obj_focus && lv_ll_get_tail(&g->obj_ll) == g->obj_focus) {
(*g->obj_focus)->signal_cb(*g->obj_focus, LV_SIGNAL_DEFOCUS, NULL);
}
else {
lv_group_refocus(g);
}
}
if(*g->obj_focus == obj) {
g->obj_focus = NULL;
}
lv_obj_t ** i;
LV_LL_READ(g->obj_ll, i)
{
if(*i == obj) {
lv_ll_rem(&g->obj_ll, i);
lv_mem_free(i);
obj->group_p = NULL;
break;
}
}
}
void lv_group_remove_all_objs(lv_group_t * group)
{
if(group->obj_focus != NULL) {
(*group->obj_focus)->signal_cb(*group->obj_focus, LV_SIGNAL_DEFOCUS, NULL);
lv_obj_invalidate(*group->obj_focus);
group->obj_focus = NULL;
}
lv_obj_t ** obj;
LV_LL_READ(group->obj_ll, obj)
{
(*obj)->group_p = NULL;
}
lv_ll_clear(&(group->obj_ll));
}
void lv_group_focus_obj(lv_obj_t * obj)
{
if(obj == NULL) return;
lv_group_t * g = obj->group_p;
if(g == NULL) return;
if(g->frozen != 0) return;
lv_group_set_editing(g, false);
lv_obj_t ** i;
LV_LL_READ(g->obj_ll, i)
{
if(*i == obj) {
if(g->obj_focus != NULL) {
(*g->obj_focus)->signal_cb(*g->obj_focus, LV_SIGNAL_DEFOCUS, NULL);
lv_res_t res = lv_event_send(*g->obj_focus, LV_EVENT_DEFOCUSED, NULL);
if(res != LV_RES_OK) return;
lv_obj_invalidate(*g->obj_focus);
}
g->obj_focus = i;
if(g->obj_focus != NULL) {
(*g->obj_focus)->signal_cb(*g->obj_focus, LV_SIGNAL_FOCUS, NULL);
if(g->focus_cb) g->focus_cb(g);
lv_res_t res = lv_event_send(*g->obj_focus, LV_EVENT_FOCUSED, NULL);
if(res != LV_RES_OK) return;
lv_obj_invalidate(*g->obj_focus);
obj_to_foreground(*g->obj_focus);
}
break;
}
}
}
void lv_group_focus_next(lv_group_t * group)
{
focus_next_core(group, lv_ll_get_head, lv_ll_get_next);
}
void lv_group_focus_prev(lv_group_t * group)
{
focus_next_core(group, lv_ll_get_tail, lv_ll_get_prev);
}
void lv_group_focus_freeze(lv_group_t * group, bool en)
{
if(en == false)
group->frozen = 0;
else
group->frozen = 1;
}
lv_res_t lv_group_send_data(lv_group_t * group, uint32_t c)
{
lv_obj_t * act = lv_group_get_focused(group);
if(act == NULL) return LV_RES_OK;
lv_res_t res;
res = act->signal_cb(act, LV_SIGNAL_CONTROL, &c);
if(res != LV_RES_OK) return res;
res = lv_event_send(act, LV_EVENT_KEY, &c);
if(res != LV_RES_OK) return res;
return res;
}
void lv_group_set_style_mod_cb(lv_group_t * group, lv_group_style_mod_cb_t style_mod_cb)
{
group->style_mod_cb = style_mod_cb;
if(group->obj_focus != NULL) lv_obj_invalidate(*group->obj_focus);
}
void lv_group_set_style_mod_edit_cb(lv_group_t * group, lv_group_style_mod_cb_t style_mod_edit_cb)
{
group->style_mod_edit_cb = style_mod_edit_cb;
if(group->obj_focus != NULL) lv_obj_invalidate(*group->obj_focus);
}
void lv_group_set_focus_cb(lv_group_t * group, lv_group_focus_cb_t focus_cb)
{
group->focus_cb = focus_cb;
}
void lv_group_set_editing(lv_group_t * group, bool edit)
{
uint8_t en_val = edit ? 1 : 0;
if(en_val == group->editing) return; 
group->editing = en_val;
lv_obj_t * focused = lv_group_get_focused(group);
if(focused) {
focused->signal_cb(focused, LV_SIGNAL_FOCUS, NULL); 
lv_res_t res = lv_event_send(*group->obj_focus, LV_EVENT_FOCUSED, NULL);
if(res != LV_RES_OK) return;
}
lv_obj_invalidate(focused);
}
void lv_group_set_click_focus(lv_group_t * group, bool en)
{
group->click_focus = en ? 1 : 0;
}
void lv_group_set_refocus_policy(lv_group_t * group, lv_group_refocus_policy_t policy)
{
group->refocus_policy = policy & 0x01;
}
void lv_group_set_wrap(lv_group_t * group, bool en)
{
group->wrap = en ? 1 : 0;
}
lv_style_t * lv_group_mod_style(lv_group_t * group, const lv_style_t * style)
{
lv_style_copy(&group->style_tmp, style);
if(group->editing) {
if(group->style_mod_edit_cb) group->style_mod_edit_cb(group, &group->style_tmp);
} else {
if(group->style_mod_cb) group->style_mod_cb(group, &group->style_tmp);
}
return &group->style_tmp;
}
lv_obj_t * lv_group_get_focused(const lv_group_t * group)
{
if(!group) return NULL;
if(group->obj_focus == NULL) return NULL;
return *group->obj_focus;
}
#if LV_USE_USER_DATA
lv_group_user_data_t * lv_group_get_user_data(lv_group_t * group)
{
return &group->user_data;
}
#endif
lv_group_style_mod_cb_t lv_group_get_style_mod_cb(const lv_group_t * group)
{
if(!group) return false;
return group->style_mod_cb;
}
lv_group_style_mod_cb_t lv_group_get_style_mod_edit_cb(const lv_group_t * group)
{
if(!group) return false;
return group->style_mod_edit_cb;
}
lv_group_focus_cb_t lv_group_get_focus_cb(const lv_group_t * group)
{
if(!group) return false;
return group->focus_cb;
}
bool lv_group_get_editing(const lv_group_t * group)
{
if(!group) return false;
return group->editing ? true : false;
}
bool lv_group_get_click_focus(const lv_group_t * group)
{
if(!group) return false;
return group->click_focus ? true : false;
}
bool lv_group_get_wrap(lv_group_t * group)
{
if(!group) return false;
return group->wrap ? true : false;
}
void lv_group_report_style_mod(lv_group_t * group)
{
lv_theme_t * th = lv_theme_get_current();
if(group != NULL) {
refresh_theme(group, th);
return;
}
lv_group_t * i;
LV_LL_READ(LV_GC_ROOT(_lv_group_ll), i)
{
refresh_theme(i, th);
}
}
static void lv_group_refocus(lv_group_t * g)
{
uint8_t temp_wrap = g->wrap;
g->wrap = 1;
if(g->refocus_policy == LV_GROUP_REFOCUS_POLICY_NEXT)
lv_group_focus_next(g);
else if(g->refocus_policy == LV_GROUP_REFOCUS_POLICY_PREV)
lv_group_focus_prev(g);
g->wrap = temp_wrap;
}
static void style_mod_def(lv_group_t * group, lv_style_t * style)
{
(void)group; 
#if LV_COLOR_DEPTH != 1
style->body.border.opa = LV_OPA_COVER;
style->body.border.color = LV_COLOR_ORANGE;
if(style->body.opa != LV_OPA_TRANSP || style->body.border.width != 0) style->body.border.width = LV_DPI / 20;
style->body.main_color = lv_color_mix(style->body.main_color, LV_COLOR_ORANGE, LV_OPA_70);
style->body.grad_color = lv_color_mix(style->body.grad_color, LV_COLOR_ORANGE, LV_OPA_70);
style->body.shadow.color = lv_color_mix(style->body.shadow.color, LV_COLOR_ORANGE, LV_OPA_60);
style->text.color = lv_color_mix(style->text.color, LV_COLOR_ORANGE, LV_OPA_70);
if(style->image.intense < LV_OPA_MIN) {
style->image.color = LV_COLOR_ORANGE;
style->image.intense = LV_OPA_40;
}
#else
style->body.border.opa = LV_OPA_COVER;
style->body.border.color = LV_COLOR_BLACK;
style->body.border.width = 2;
#endif
}
static void style_mod_edit_def(lv_group_t * group, lv_style_t * style)
{
(void)group; 
#if LV_COLOR_DEPTH != 1
style->body.border.opa = LV_OPA_COVER;
style->body.border.color = LV_COLOR_GREEN;
if(style->body.opa != LV_OPA_TRANSP || style->body.border.width != 0) style->body.border.width = LV_DPI / 20;
style->body.main_color = lv_color_mix(style->body.main_color, LV_COLOR_GREEN, LV_OPA_70);
style->body.grad_color = lv_color_mix(style->body.grad_color, LV_COLOR_GREEN, LV_OPA_70);
style->body.shadow.color = lv_color_mix(style->body.shadow.color, LV_COLOR_GREEN, LV_OPA_60);
style->text.color = lv_color_mix(style->text.color, LV_COLOR_GREEN, LV_OPA_70);
if(style->image.intense < LV_OPA_MIN) {
style->image.color = LV_COLOR_GREEN;
style->image.intense = LV_OPA_40;
}
#else
style->body.border.opa = LV_OPA_COVER;
style->body.border.color = LV_COLOR_BLACK;
style->body.border.width = 3;
#endif
}
static void refresh_theme(lv_group_t * g, lv_theme_t * th)
{
g->style_mod_cb = style_mod_def;
g->style_mod_edit_cb = style_mod_edit_def;
if(th) {
if(th->group.style_mod_xcb) g->style_mod_cb = th->group.style_mod_xcb;
if(th->group.style_mod_edit_xcb) g->style_mod_edit_cb = th->group.style_mod_edit_xcb;
}
}
static void focus_next_core(lv_group_t * group, void * (*begin)(const lv_ll_t *),
void * (*move)(const lv_ll_t *, const void *))
{
if(group->frozen) return;
lv_obj_t ** obj_next = group->obj_focus;
lv_obj_t ** obj_sentinel = NULL;
bool can_move = true;
bool can_begin = true;
for(;;) {
if(obj_next == NULL) {
if(group->wrap || obj_sentinel == NULL) {
if(!can_begin) return;
obj_next = begin(&group->obj_ll);
can_move = false;
can_begin = false;
} else {
return;
}
}
if(obj_sentinel == NULL) {
obj_sentinel = obj_next;
if(obj_sentinel == NULL) return; 
}
if(can_move) {
obj_next = move(&group->obj_ll, obj_next);
if(obj_next == obj_sentinel) return;
}
can_move = true;
if(obj_next == NULL) continue;
if(!lv_obj_get_hidden(*obj_next)) break;
}
if(obj_next == group->obj_focus) return; 
if(group->obj_focus) {
(*group->obj_focus)->signal_cb(*group->obj_focus, LV_SIGNAL_DEFOCUS, NULL);
lv_res_t res = lv_event_send(*group->obj_focus, LV_EVENT_DEFOCUSED, NULL);
if(res != LV_RES_OK) return;
lv_obj_invalidate(*group->obj_focus);
}
group->obj_focus = obj_next;
(*group->obj_focus)->signal_cb(*group->obj_focus, LV_SIGNAL_FOCUS, NULL);
lv_res_t res = lv_event_send(*group->obj_focus, LV_EVENT_FOCUSED, NULL);
if(res != LV_RES_OK) return;
obj_to_foreground(*group->obj_focus);
lv_obj_invalidate(*group->obj_focus);
if(group->focus_cb) group->focus_cb(group);
}
static void obj_to_foreground(lv_obj_t * obj)
{
lv_obj_t * i = obj;
lv_obj_t * last_top = NULL;
while(i != NULL) {
if(i->top != 0) last_top = i;
i = lv_obj_get_parent(i);
}
if(last_top != NULL) {
lv_obj_move_foreground(last_top);
}
}
#endif 
