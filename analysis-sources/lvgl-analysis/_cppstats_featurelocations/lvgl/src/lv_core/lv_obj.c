







#include "lv_obj.h"
#include "lv_indev.h"
#include "lv_refr.h"
#include "lv_group.h"
#include "lv_disp.h"
#include "../lv_core/lv_debug.h"
#include "../lv_themes/lv_theme.h"
#include "../lv_draw/lv_draw.h"
#include "../lv_misc/lv_anim.h"
#include "../lv_misc/lv_task.h"
#include "../lv_misc/lv_async.h"
#include "../lv_misc/lv_fs.h"
#include "../lv_hal/lv_hal.h"
#include <stdint.h>
#include <string.h>
#include "../lv_misc/lv_gc.h"
#include "../lv_misc/lv_math.h"

#if defined(LV_GC_INCLUDE)
#include LV_GC_INCLUDE
#endif 




#define LV_OBJX_NAME "lv_obj"
#define LV_OBJ_DEF_WIDTH (LV_DPI)
#define LV_OBJ_DEF_HEIGHT (2 * LV_DPI / 3)




typedef struct _lv_event_temp_data
{
lv_obj_t * obj;
bool deleted;
struct _lv_event_temp_data * prev;
} lv_event_temp_data_t;




static void refresh_children_position(lv_obj_t * obj, lv_coord_t x_diff, lv_coord_t y_diff);
static void report_style_mod_core(void * style_p, lv_obj_t * obj);
static void refresh_children_style(lv_obj_t * obj);
static void delete_children(lv_obj_t * obj);
static void base_dir_refr_children(lv_obj_t * obj);
static void lv_event_mark_deleted(lv_obj_t * obj);
static void lv_obj_del_async_cb(void * obj);
static bool lv_obj_design(lv_obj_t * obj, const lv_area_t * mask_p, lv_design_mode_t mode);
static lv_res_t lv_obj_signal(lv_obj_t * obj, lv_signal_t sign, void * param);




static bool lv_initialized = false;
static lv_event_temp_data_t * event_temp_data_head;
static const void * event_act_data;












void lv_init(void)
{

if(lv_initialized) {
LV_LOG_WARN("lv_init: already inited");
return;
}

LV_LOG_TRACE("lv_init started");


lv_mem_init();
lv_task_core_init();

#if LV_USE_FILESYSTEM
lv_fs_init();
#endif

#if LV_USE_ANIMATION
lv_anim_core_init();
#endif

#if LV_USE_GROUP
lv_group_init();
#endif


lv_style_init();


lv_refr_init();

lv_ll_init(&LV_GC_ROOT(_lv_disp_ll), sizeof(lv_disp_t));
lv_ll_init(&LV_GC_ROOT(_lv_indev_ll), sizeof(lv_indev_t));


lv_indev_init();

lv_img_decoder_init();
lv_img_cache_set_size(LV_IMG_CACHE_DEF_SIZE);

lv_initialized = true;
LV_LOG_INFO("lv_init ready");
}

#if LV_ENABLE_GC || !LV_MEM_CUSTOM
void lv_deinit(void)
{
lv_gc_clear_roots();
#if LV_USE_LOG
lv_log_register_print_cb(NULL);
#endif
lv_disp_set_default(NULL);
lv_mem_deinit();
lv_initialized = false;
LV_LOG_INFO("lv_deinit done");
}
#endif












lv_obj_t * lv_obj_create(lv_obj_t * parent, const lv_obj_t * copy)
{

lv_obj_t * new_obj = NULL;

if(parent == NULL) {
LV_LOG_TRACE("Screen create started");
lv_disp_t * disp = lv_disp_get_default();
if(!disp) {
LV_LOG_WARN("lv_obj_create: not display created to so far. No place to assign the new screen");
return NULL;
}

new_obj = lv_ll_ins_head(&disp->scr_ll);
LV_ASSERT_MEM(new_obj);
if(new_obj == NULL) return NULL;

new_obj->par = NULL; 
lv_ll_init(&(new_obj->child_ll), sizeof(lv_obj_t));


new_obj->signal_cb = lv_obj_signal;
new_obj->design_cb = lv_obj_design;
new_obj->event_cb = NULL;


new_obj->coords.x1 = 0;
new_obj->coords.y1 = 0;
new_obj->coords.x2 = lv_disp_get_hor_res(NULL) - 1;
new_obj->coords.y2 = lv_disp_get_ver_res(NULL) - 1;
new_obj->ext_draw_pad = 0;

#if LV_USE_EXT_CLICK_AREA == LV_EXT_CLICK_AREA_FULL
memset(&new_obj->ext_click_pad, 0, sizeof(new_obj->ext_click_pad));
#endif

#if LV_USE_EXT_CLICK_AREA == LV_EXT_CLICK_AREA_TINY
new_obj->ext_click_pad_hor = 0;
new_obj->ext_click_pad_ver = 0;
#endif


#if LV_USE_OBJ_REALIGN
new_obj->realign.align = LV_ALIGN_CENTER;
new_obj->realign.xofs = 0;
new_obj->realign.yofs = 0;
new_obj->realign.base = NULL;
new_obj->realign.auto_realign = 0;
#endif


lv_theme_t * th = lv_theme_get_current();
if(th) {
new_obj->style_p = th->style.scr;
} else {
new_obj->style_p = &lv_style_scr;
}


#if LV_USE_USER_DATA
memset(&new_obj->user_data, 0, sizeof(lv_obj_user_data_t));
#endif

#if LV_USE_GROUP
new_obj->group_p = NULL;
#endif

new_obj->click = 0;
new_obj->drag = 0;
new_obj->drag_throw = 0;
new_obj->drag_parent = 0;
new_obj->hidden = 0;
new_obj->top = 0;
new_obj->protect = LV_PROTECT_NONE;
new_obj->opa_scale_en = 0;
new_obj->opa_scale = LV_OPA_COVER;
new_obj->parent_event = 0;
#if LV_USE_BIDI
new_obj->base_dir = LV_BIDI_BASE_DIR_DEF;
#else
new_obj->base_dir = LV_BIDI_DIR_LTR;
#endif

new_obj->reserved = 0;

new_obj->ext_attr = NULL;

LV_LOG_INFO("Screen create ready");
}

else {
LV_LOG_TRACE("Object create started");
LV_ASSERT_OBJ(parent, LV_OBJX_NAME);

new_obj = lv_ll_ins_head(&parent->child_ll);
LV_ASSERT_MEM(new_obj);
if(new_obj == NULL) return NULL;

new_obj->par = parent; 
lv_ll_init(&(new_obj->child_ll), sizeof(lv_obj_t));


new_obj->signal_cb = lv_obj_signal;
new_obj->design_cb = lv_obj_design;
new_obj->event_cb = NULL;

#if LV_USE_BIDI
new_obj->base_dir = LV_BIDI_DIR_INHERIT;
#else
new_obj->base_dir = LV_BIDI_DIR_LTR;
#endif


new_obj->coords.y1 = parent->coords.y1;
new_obj->coords.y2 = parent->coords.y1 + LV_OBJ_DEF_HEIGHT;
if(lv_obj_get_base_dir(new_obj) == LV_BIDI_DIR_RTL) {
new_obj->coords.x2 = parent->coords.x2;
new_obj->coords.x1 = parent->coords.x2 - LV_OBJ_DEF_WIDTH;
} else {
new_obj->coords.x1 = parent->coords.x1;
new_obj->coords.x2 = parent->coords.x1 + LV_OBJ_DEF_WIDTH;
}
new_obj->ext_draw_pad = 0;

#if LV_USE_EXT_CLICK_AREA == LV_EXT_CLICK_AREA_FULL
memset(&new_obj->ext_click_pad, 0, sizeof(new_obj->ext_click_pad));
#endif

#if LV_USE_EXT_CLICK_AREA == LV_EXT_CLICK_AREA_TINY
new_obj->ext_click_pad_hor = 0;
new_obj->ext_click_pad_ver = 0;
#endif


#if LV_USE_OBJ_REALIGN
new_obj->realign.align = LV_ALIGN_CENTER;
new_obj->realign.xofs = 0;
new_obj->realign.yofs = 0;
new_obj->realign.base = NULL;
new_obj->realign.auto_realign = 0;
#endif

lv_theme_t * th = lv_theme_get_current();
if(th) {
new_obj->style_p = th->style.panel;
} else {
new_obj->style_p = &lv_style_plain_color;
}

#if LV_USE_EXT_CLICK_AREA == LV_EXT_CLICK_AREA_FULL
memset(&new_obj->ext_click_pad, 0, sizeof(new_obj->ext_click_pad));
#endif

#if LV_USE_EXT_CLICK_AREA == LV_EXT_CLICK_AREA_TINY
new_obj->ext_click_pad_hor = 0;
new_obj->ext_click_pad_ver = 0;
#endif


#if LV_USE_USER_DATA
memset(&new_obj->user_data, 0, sizeof(lv_obj_user_data_t));
#endif

#if LV_USE_GROUP
new_obj->group_p = NULL;
#endif


new_obj->click = 1;
new_obj->drag = 0;
new_obj->drag_dir = LV_DRAG_DIR_ALL;
new_obj->drag_throw = 0;
new_obj->drag_parent = 0;
new_obj->hidden = 0;
new_obj->top = 0;
new_obj->protect = LV_PROTECT_NONE;
new_obj->opa_scale = LV_OPA_COVER;
new_obj->opa_scale_en = 0;
new_obj->parent_event = 0;
new_obj->reserved = 0;

new_obj->ext_attr = NULL;
}


if(copy != NULL) {
LV_ASSERT_OBJ(copy, LV_OBJX_NAME);
lv_area_copy(&new_obj->coords, &copy->coords);
new_obj->ext_draw_pad = copy->ext_draw_pad;

#if LV_USE_EXT_CLICK_AREA == LV_EXT_CLICK_AREA_FULL
lv_area_copy(&new_obj->ext_click_pad, &copy->ext_click_pad);
#endif

#if LV_USE_EXT_CLICK_AREA == LV_EXT_CLICK_AREA_TINY
new_obj->ext_click_pad_hor = copy->ext_click_pad_hor;
new_obj->ext_click_pad_ver = copy->ext_click_pad_ver;
#endif


#if LV_USE_USER_DATA
memcpy(&new_obj->user_data, &copy->user_data, sizeof(lv_obj_user_data_t));
#endif

#if LV_USE_OBJ_REALIGN
new_obj->realign.align = copy->realign.align;
new_obj->realign.xofs = copy->realign.xofs;
new_obj->realign.yofs = copy->realign.yofs;
new_obj->realign.base = copy->realign.base;
new_obj->realign.auto_realign = copy->realign.auto_realign;
#endif



new_obj->event_cb = copy->event_cb;


new_obj->click = copy->click;
new_obj->drag = copy->drag;
new_obj->drag_dir = copy->drag_dir;
new_obj->drag_throw = copy->drag_throw;
new_obj->drag_parent = copy->drag_parent;
new_obj->hidden = copy->hidden;
new_obj->top = copy->top;
new_obj->parent_event = copy->parent_event;

new_obj->opa_scale_en = copy->opa_scale_en;
new_obj->protect = copy->protect;
new_obj->opa_scale = copy->opa_scale;

new_obj->style_p = copy->style_p;

#if LV_USE_GROUP

if(copy->group_p != NULL) {
lv_group_add_obj(copy->group_p, new_obj);
}
#endif


if(lv_obj_get_parent(copy) != NULL && parent != NULL) {
lv_obj_set_pos(new_obj, lv_obj_get_x(copy), lv_obj_get_y(copy));
} else {
lv_obj_set_pos(new_obj, 0, 0);
}

LV_LOG_INFO("Object create ready");
}


if(parent != NULL) {
parent->signal_cb(parent, LV_SIGNAL_CHILD_CHG, new_obj);


lv_obj_invalidate(new_obj);
}

return new_obj;
}






lv_res_t lv_obj_del(lv_obj_t * obj)
{
LV_ASSERT_OBJ(obj, LV_OBJX_NAME);
lv_obj_invalidate(obj);


#if LV_USE_GROUP
lv_group_t * group = lv_obj_get_group(obj);
if(group) lv_group_remove_obj(obj);
#endif


#if LV_USE_ANIMATION
lv_anim_del(obj, NULL);
#endif


lv_obj_t * i;
lv_obj_t * i_next;
i = lv_ll_get_head(&(obj->child_ll));
while(i != NULL) {

i_next = lv_ll_get_next(&(obj->child_ll), i);


delete_children(i);


i = i_next;
}


lv_event_send(obj, LV_EVENT_DELETE, NULL);

lv_event_mark_deleted(obj);


lv_indev_t * indev = lv_indev_get_next(NULL);
while(indev) {
if(indev->proc.types.pointer.act_obj == obj || indev->proc.types.pointer.last_obj == obj) {
lv_indev_reset(indev);
}
if(indev->proc.types.pointer.last_pressed == obj) {
indev->proc.types.pointer.last_pressed = NULL;
}

#if LV_USE_GROUP
if(indev->group == group && obj == lv_indev_get_obj_act()) {
lv_indev_reset(indev);
}
#endif
indev = lv_indev_get_next(indev);
}



obj->signal_cb(obj, LV_SIGNAL_CLEANUP, NULL);


lv_obj_t * par = lv_obj_get_parent(obj);
if(par == NULL) { 
lv_disp_t * d = lv_obj_get_disp(obj);
lv_ll_rem(&d->scr_ll, obj);
} else {
lv_ll_rem(&(par->child_ll), obj);
}


if(obj->ext_attr != NULL) lv_mem_free(obj->ext_attr);
lv_mem_free(obj); 


if(par != NULL) {
par->signal_cb(par, LV_SIGNAL_CHILD_CHG, NULL);
}

return LV_RES_INV;
}







void lv_obj_del_async(lv_obj_t * obj)
{
LV_ASSERT_OBJ(obj, LV_OBJX_NAME);
lv_async_call(lv_obj_del_async_cb, obj);
}





void lv_obj_clean(lv_obj_t * obj)
{
LV_ASSERT_OBJ(obj, LV_OBJX_NAME);
lv_obj_t * child = lv_obj_get_child(obj, NULL);
lv_obj_t * child_next;
while(child) {


child_next = lv_obj_get_child(obj, child);
lv_obj_del(child);
child = child_next;
}
}







void lv_obj_invalidate_area(const lv_obj_t * obj, const lv_area_t * area)
{
LV_ASSERT_OBJ(obj, LV_OBJX_NAME);

if(lv_obj_get_hidden(obj)) return;


lv_obj_t * obj_scr = lv_obj_get_screen(obj);
lv_disp_t * disp = lv_obj_get_disp(obj_scr);
if(obj_scr == lv_disp_get_scr_act(disp) || obj_scr == lv_disp_get_layer_top(disp) ||
obj_scr == lv_disp_get_layer_sys(disp)) {


lv_area_t obj_coords;
lv_coord_t ext_size = obj->ext_draw_pad;
lv_area_copy(&obj_coords, &obj->coords);
obj_coords.x1 -= ext_size;
obj_coords.y1 -= ext_size;
obj_coords.x2 += ext_size;
obj_coords.y2 += ext_size;

bool is_common;
lv_area_t area_trunc;

is_common = lv_area_intersect(&area_trunc, area, &obj_coords);
if(is_common == false) return; 


lv_obj_t * par = lv_obj_get_parent(obj);
while(par != NULL) {
is_common = lv_area_intersect(&area_trunc, &area_trunc, &par->coords);
if(is_common == false) break; 
if(lv_obj_get_hidden(par)) return; 

par = lv_obj_get_parent(par);
}

if(is_common) lv_inv_area(disp, &area_trunc);
}
}





void lv_obj_invalidate(const lv_obj_t * obj)
{
LV_ASSERT_OBJ(obj, LV_OBJX_NAME);


lv_area_t obj_coords;
lv_coord_t ext_size = obj->ext_draw_pad;
lv_area_copy(&obj_coords, &obj->coords);
obj_coords.x1 -= ext_size;
obj_coords.y1 -= ext_size;
obj_coords.x2 += ext_size;
obj_coords.y2 += ext_size;

lv_obj_invalidate_area(obj, &obj_coords);

}













void lv_obj_set_parent(lv_obj_t * obj, lv_obj_t * parent)
{
LV_ASSERT_OBJ(obj, LV_OBJX_NAME);
LV_ASSERT_OBJ(parent, LV_OBJX_NAME);

if(obj->par == NULL) {
LV_LOG_WARN("Can't set the parent of a screen");
return;
}

if(parent == NULL) {
LV_LOG_WARN("Can't set parent == NULL to an object");
return;
}

lv_obj_invalidate(obj);

lv_point_t old_pos;
old_pos.x = lv_obj_get_x(obj);
old_pos.y = lv_obj_get_y(obj);

lv_obj_t * old_par = obj->par;

lv_ll_chg_list(&obj->par->child_ll, &parent->child_ll, obj, true);
obj->par = parent;
lv_obj_set_pos(obj, old_pos.x, old_pos.y);


old_par->signal_cb(old_par, LV_SIGNAL_CHILD_CHG, NULL);


parent->signal_cb(parent, LV_SIGNAL_CHILD_CHG, obj);

lv_obj_invalidate(obj);
}





void lv_obj_move_foreground(lv_obj_t * obj)
{
LV_ASSERT_OBJ(obj, LV_OBJX_NAME);

lv_obj_t * parent = lv_obj_get_parent(obj);


if(lv_ll_get_head(&parent->child_ll) == obj) return;

lv_obj_invalidate(parent);

lv_ll_chg_list(&parent->child_ll, &parent->child_ll, obj, true);


parent->signal_cb(parent, LV_SIGNAL_CHILD_CHG, obj);

lv_obj_invalidate(parent);
}





void lv_obj_move_background(lv_obj_t * obj)
{
LV_ASSERT_OBJ(obj, LV_OBJX_NAME);

lv_obj_t * parent = lv_obj_get_parent(obj);


if(lv_ll_get_tail(&parent->child_ll) == obj) return;

lv_obj_invalidate(parent);

lv_ll_chg_list(&parent->child_ll, &parent->child_ll, obj, false);


parent->signal_cb(parent, LV_SIGNAL_CHILD_CHG, obj);

lv_obj_invalidate(parent);
}











void lv_obj_set_pos(lv_obj_t * obj, lv_coord_t x, lv_coord_t y)
{
LV_ASSERT_OBJ(obj, LV_OBJX_NAME);


lv_obj_t * par = obj->par;

x = x + par->coords.x1;
y = y + par->coords.y1;


lv_point_t diff;
diff.x = x - obj->coords.x1;
diff.y = y - obj->coords.y1;




if(diff.x == 0 && diff.y == 0) return;


lv_obj_invalidate(obj);


lv_area_t ori;
lv_obj_get_coords(obj, &ori);

obj->coords.x1 += diff.x;
obj->coords.y1 += diff.y;
obj->coords.x2 += diff.x;
obj->coords.y2 += diff.y;

refresh_children_position(obj, diff.x, diff.y);


obj->signal_cb(obj, LV_SIGNAL_CORD_CHG, &ori);


par->signal_cb(par, LV_SIGNAL_CHILD_CHG, obj);


lv_obj_invalidate(obj);
}






void lv_obj_set_x(lv_obj_t * obj, lv_coord_t x)
{
LV_ASSERT_OBJ(obj, LV_OBJX_NAME);

lv_obj_set_pos(obj, x, lv_obj_get_y(obj));
}






void lv_obj_set_y(lv_obj_t * obj, lv_coord_t y)
{
LV_ASSERT_OBJ(obj, LV_OBJX_NAME);

lv_obj_set_pos(obj, lv_obj_get_x(obj), y);
}







void lv_obj_set_size(lv_obj_t * obj, lv_coord_t w, lv_coord_t h)
{
LV_ASSERT_OBJ(obj, LV_OBJX_NAME);





if(lv_obj_get_width(obj) == w && lv_obj_get_height(obj) == h) {
return;
}


lv_obj_invalidate(obj);


lv_area_t ori;
lv_obj_get_coords(obj, &ori);


obj->coords.y2 = obj->coords.y1 + h - 1;
if(lv_obj_get_base_dir(obj) == LV_BIDI_DIR_RTL) {
obj->coords.x1 = obj->coords.x2 - w + 1;
} else {
obj->coords.x2 = obj->coords.x1 + w - 1;
}


obj->signal_cb(obj, LV_SIGNAL_CORD_CHG, &ori);


lv_obj_t * par = lv_obj_get_parent(obj);
if(par != NULL) par->signal_cb(par, LV_SIGNAL_CHILD_CHG, obj);


lv_obj_t * i;
LV_LL_READ(obj->child_ll, i)
{
i->signal_cb(i, LV_SIGNAL_PARENT_SIZE_CHG, NULL);
}


lv_obj_invalidate(obj);


#if LV_USE_OBJ_REALIGN
if(obj->realign.auto_realign) lv_obj_realign(obj);
#endif
}






void lv_obj_set_width(lv_obj_t * obj, lv_coord_t w)
{
LV_ASSERT_OBJ(obj, LV_OBJX_NAME);

lv_obj_set_size(obj, w, lv_obj_get_height(obj));
}






void lv_obj_set_height(lv_obj_t * obj, lv_coord_t h)
{
LV_ASSERT_OBJ(obj, LV_OBJX_NAME);

lv_obj_set_size(obj, lv_obj_get_width(obj), h);
}









void lv_obj_align(lv_obj_t * obj, const lv_obj_t * base, lv_align_t align, lv_coord_t x_mod, lv_coord_t y_mod)
{
LV_ASSERT_OBJ(obj, LV_OBJX_NAME);

lv_coord_t new_x = lv_obj_get_x(obj);
lv_coord_t new_y = lv_obj_get_y(obj);

if(base == NULL) {
base = lv_obj_get_parent(obj);
}

LV_ASSERT_OBJ(base, LV_OBJX_NAME);


switch(align) {
case LV_ALIGN_CENTER:
new_x = lv_obj_get_width(base) / 2 - lv_obj_get_width(obj) / 2;
new_y = lv_obj_get_height(base) / 2 - lv_obj_get_height(obj) / 2;
break;

case LV_ALIGN_IN_TOP_LEFT:
new_x = 0;
new_y = 0;
break;
case LV_ALIGN_IN_TOP_MID:
new_x = lv_obj_get_width(base) / 2 - lv_obj_get_width(obj) / 2;
new_y = 0;
break;

case LV_ALIGN_IN_TOP_RIGHT:
new_x = lv_obj_get_width(base) - lv_obj_get_width(obj);
new_y = 0;
break;

case LV_ALIGN_IN_BOTTOM_LEFT:
new_x = 0;
new_y = lv_obj_get_height(base) - lv_obj_get_height(obj);
break;
case LV_ALIGN_IN_BOTTOM_MID:
new_x = lv_obj_get_width(base) / 2 - lv_obj_get_width(obj) / 2;
new_y = lv_obj_get_height(base) - lv_obj_get_height(obj);
break;

case LV_ALIGN_IN_BOTTOM_RIGHT:
new_x = lv_obj_get_width(base) - lv_obj_get_width(obj);
new_y = lv_obj_get_height(base) - lv_obj_get_height(obj);
break;

case LV_ALIGN_IN_LEFT_MID:
new_x = 0;
new_y = lv_obj_get_height(base) / 2 - lv_obj_get_height(obj) / 2;
break;

case LV_ALIGN_IN_RIGHT_MID:
new_x = lv_obj_get_width(base) - lv_obj_get_width(obj);
new_y = lv_obj_get_height(base) / 2 - lv_obj_get_height(obj) / 2;
break;

case LV_ALIGN_OUT_TOP_LEFT:
new_x = 0;
new_y = -lv_obj_get_height(obj);
break;

case LV_ALIGN_OUT_TOP_MID:
new_x = lv_obj_get_width(base) / 2 - lv_obj_get_width(obj) / 2;
new_y = -lv_obj_get_height(obj);
break;

case LV_ALIGN_OUT_TOP_RIGHT:
new_x = lv_obj_get_width(base) - lv_obj_get_width(obj);
new_y = -lv_obj_get_height(obj);
break;

case LV_ALIGN_OUT_BOTTOM_LEFT:
new_x = 0;
new_y = lv_obj_get_height(base);
break;

case LV_ALIGN_OUT_BOTTOM_MID:
new_x = lv_obj_get_width(base) / 2 - lv_obj_get_width(obj) / 2;
new_y = lv_obj_get_height(base);
break;

case LV_ALIGN_OUT_BOTTOM_RIGHT:
new_x = lv_obj_get_width(base) - lv_obj_get_width(obj);
new_y = lv_obj_get_height(base);
break;

case LV_ALIGN_OUT_LEFT_TOP:
new_x = -lv_obj_get_width(obj);
new_y = 0;
break;

case LV_ALIGN_OUT_LEFT_MID:
new_x = -lv_obj_get_width(obj);
new_y = lv_obj_get_height(base) / 2 - lv_obj_get_height(obj) / 2;
break;

case LV_ALIGN_OUT_LEFT_BOTTOM:
new_x = -lv_obj_get_width(obj);
new_y = lv_obj_get_height(base) - lv_obj_get_height(obj);
break;

case LV_ALIGN_OUT_RIGHT_TOP:
new_x = lv_obj_get_width(base);
new_y = 0;
break;

case LV_ALIGN_OUT_RIGHT_MID:
new_x = lv_obj_get_width(base);
new_y = lv_obj_get_height(base) / 2 - lv_obj_get_height(obj) / 2;
break;

case LV_ALIGN_OUT_RIGHT_BOTTOM:
new_x = lv_obj_get_width(base);
new_y = lv_obj_get_height(base) - lv_obj_get_height(obj);
break;
}


lv_obj_t * par = lv_obj_get_parent(obj);
lv_coord_t base_abs_x = base->coords.x1;
lv_coord_t base_abs_y = base->coords.y1;
lv_coord_t par_abs_x = par->coords.x1;
lv_coord_t par_abs_y = par->coords.y1;
new_x += x_mod + base_abs_x;
new_y += y_mod + base_abs_y;
new_x -= par_abs_x;
new_y -= par_abs_y;

lv_obj_set_pos(obj, new_x, new_y);

#if LV_USE_OBJ_REALIGN

obj->realign.align = align;
obj->realign.xofs = x_mod;
obj->realign.yofs = y_mod;
obj->realign.base = base;
obj->realign.origo_align = 0;
#endif
}









void lv_obj_align_origo(lv_obj_t * obj, const lv_obj_t * base, lv_align_t align, lv_coord_t x_mod, lv_coord_t y_mod)
{
LV_ASSERT_OBJ(obj, LV_OBJX_NAME);

lv_coord_t new_x = lv_obj_get_x(obj);
lv_coord_t new_y = lv_obj_get_y(obj);

lv_coord_t obj_w_half = lv_obj_get_width(obj) / 2;
lv_coord_t obj_h_half = lv_obj_get_height(obj) / 2;

if(base == NULL) {
base = lv_obj_get_parent(obj);
}

LV_ASSERT_OBJ(base, LV_OBJX_NAME);


switch(align) {
case LV_ALIGN_CENTER:
new_x = lv_obj_get_width(base) / 2 - obj_w_half;
new_y = lv_obj_get_height(base) / 2 - obj_h_half;
break;

case LV_ALIGN_IN_TOP_LEFT:
new_x = -obj_w_half;
new_y = -obj_h_half;
break;
case LV_ALIGN_IN_TOP_MID:
new_x = lv_obj_get_width(base) / 2 - obj_w_half;
new_y = -obj_h_half;
break;

case LV_ALIGN_IN_TOP_RIGHT:
new_x = lv_obj_get_width(base) - obj_w_half;
new_y = -obj_h_half;
break;

case LV_ALIGN_IN_BOTTOM_LEFT:
new_x = -obj_w_half;
new_y = lv_obj_get_height(base) - obj_h_half;
break;
case LV_ALIGN_IN_BOTTOM_MID:
new_x = lv_obj_get_width(base) / 2 - obj_w_half;
new_y = lv_obj_get_height(base) - obj_h_half;
break;

case LV_ALIGN_IN_BOTTOM_RIGHT:
new_x = lv_obj_get_width(base) - obj_w_half;
new_y = lv_obj_get_height(base) - obj_h_half;
break;

case LV_ALIGN_IN_LEFT_MID:
new_x = -obj_w_half;
new_y = lv_obj_get_height(base) / 2 - obj_h_half;
break;

case LV_ALIGN_IN_RIGHT_MID:
new_x = lv_obj_get_width(base) - obj_w_half;
new_y = lv_obj_get_height(base) / 2 - obj_h_half;
break;

case LV_ALIGN_OUT_TOP_LEFT:
new_x = -obj_w_half;
new_y = -obj_h_half;
break;

case LV_ALIGN_OUT_TOP_MID:
new_x = lv_obj_get_width(base) / 2 - obj_w_half;
new_y = -obj_h_half;
break;

case LV_ALIGN_OUT_TOP_RIGHT:
new_x = lv_obj_get_width(base) - obj_w_half;
new_y = -obj_h_half;
break;

case LV_ALIGN_OUT_BOTTOM_LEFT:
new_x = -obj_w_half;
new_y = lv_obj_get_height(base) - obj_h_half;
break;

case LV_ALIGN_OUT_BOTTOM_MID:
new_x = lv_obj_get_width(base) / 2 - obj_w_half;
new_y = lv_obj_get_height(base) - obj_h_half;
break;

case LV_ALIGN_OUT_BOTTOM_RIGHT:
new_x = lv_obj_get_width(base) - obj_w_half;
new_y = lv_obj_get_height(base) - obj_h_half;
break;

case LV_ALIGN_OUT_LEFT_TOP:
new_x = -obj_w_half;
new_y = -obj_h_half;
break;

case LV_ALIGN_OUT_LEFT_MID:
new_x = -obj_w_half;
new_y = lv_obj_get_height(base) / 2 - obj_h_half;
break;

case LV_ALIGN_OUT_LEFT_BOTTOM:
new_x = -obj_w_half;
new_y = lv_obj_get_height(base) - obj_h_half;
break;

case LV_ALIGN_OUT_RIGHT_TOP:
new_x = lv_obj_get_width(base) - obj_w_half;
new_y = -obj_h_half;
break;

case LV_ALIGN_OUT_RIGHT_MID:
new_x = lv_obj_get_width(base) - obj_w_half;
new_y = lv_obj_get_height(base) / 2 - obj_h_half;
break;

case LV_ALIGN_OUT_RIGHT_BOTTOM:
new_x = lv_obj_get_width(base) - obj_w_half;
new_y = lv_obj_get_height(base) - obj_h_half;
break;
}


lv_obj_t * par = lv_obj_get_parent(obj);
lv_coord_t base_abs_x = base->coords.x1;
lv_coord_t base_abs_y = base->coords.y1;
lv_coord_t par_abs_x = par->coords.x1;
lv_coord_t par_abs_y = par->coords.y1;
new_x += x_mod + base_abs_x;
new_y += y_mod + base_abs_y;
new_x -= par_abs_x;
new_y -= par_abs_y;

lv_obj_set_pos(obj, new_x, new_y);

#if LV_USE_OBJ_REALIGN

obj->realign.align = align;
obj->realign.xofs = x_mod;
obj->realign.yofs = y_mod;
obj->realign.base = base;
obj->realign.origo_align = 1;
#endif
}





void lv_obj_realign(lv_obj_t * obj)
{
LV_ASSERT_OBJ(obj, LV_OBJX_NAME);

#if LV_USE_OBJ_REALIGN
if(obj->realign.origo_align)
lv_obj_align_origo(obj, obj->realign.base, obj->realign.align, obj->realign.xofs, obj->realign.yofs);
else
lv_obj_align(obj, obj->realign.base, obj->realign.align, obj->realign.xofs, obj->realign.yofs);
#else
(void)obj;
LV_LOG_WARN("lv_obj_realaign: no effect because LV_USE_OBJ_REALIGN = 0");
#endif
}







void lv_obj_set_auto_realign(lv_obj_t * obj, bool en)
{
LV_ASSERT_OBJ(obj, LV_OBJX_NAME);

#if LV_USE_OBJ_REALIGN
obj->realign.auto_realign = en ? 1 : 0;
#else
(void)obj;
(void)en;
LV_LOG_WARN("lv_obj_set_auto_realign: no effect because LV_USE_OBJ_REALIGN = 0");
#endif
}












void lv_obj_set_ext_click_area(lv_obj_t * obj, lv_coord_t left, lv_coord_t right, lv_coord_t top, lv_coord_t bottom)
{
LV_ASSERT_OBJ(obj, LV_OBJX_NAME);

#if LV_USE_EXT_CLICK_AREA == LV_EXT_CLICK_AREA_FULL
obj->ext_click_pad.x1 = left;
obj->ext_click_pad.x2 = right;
obj->ext_click_pad.y1 = top;
obj->ext_click_pad.y2 = bottom;
#elif LV_USE_EXT_CLICK_AREA == LV_EXT_CLICK_AREA_TINY
obj->ext_click_pad_hor = LV_MATH_MAX(left, right);
obj->ext_click_pad_ver = LV_MATH_MAX(top, bottom);
#else
(void)obj; 
(void)left; 
(void)right; 
(void)top; 
(void)bottom; 
#endif
}










void lv_obj_set_style(lv_obj_t * obj, const lv_style_t * style)
{
LV_ASSERT_OBJ(obj, LV_OBJX_NAME);
LV_ASSERT_STYLE(style);

obj->style_p = style;


refresh_children_style(obj);


lv_obj_refresh_style(obj);
}





void lv_obj_refresh_style(lv_obj_t * obj)
{
LV_ASSERT_OBJ(obj, LV_OBJX_NAME);

lv_obj_invalidate(obj);
obj->signal_cb(obj, LV_SIGNAL_STYLE_CHG, NULL);
lv_obj_invalidate(obj);
}






void lv_obj_report_style_mod(lv_style_t * style)
{
LV_ASSERT_STYLE(style);

lv_disp_t * d = lv_disp_get_next(NULL);

while(d) {
lv_obj_t * i;
LV_LL_READ(d->scr_ll, i)
{
if(i->style_p == style || style == NULL) {
lv_obj_refresh_style(i);
}

report_style_mod_core(style, i);
}
d = lv_disp_get_next(d);
}
}










void lv_obj_set_hidden(lv_obj_t * obj, bool en)
{
LV_ASSERT_OBJ(obj, LV_OBJX_NAME);

if(!obj->hidden) lv_obj_invalidate(obj); 

obj->hidden = en == false ? 0 : 1;

if(!obj->hidden) lv_obj_invalidate(obj); 

lv_obj_t * par = lv_obj_get_parent(obj);
par->signal_cb(par, LV_SIGNAL_CHILD_CHG, obj);
}






void lv_obj_set_click(lv_obj_t * obj, bool en)
{
LV_ASSERT_OBJ(obj, LV_OBJX_NAME);

obj->click = (en == true ? 1 : 0);
}







void lv_obj_set_top(lv_obj_t * obj, bool en)
{
LV_ASSERT_OBJ(obj, LV_OBJX_NAME);

obj->top = (en == true ? 1 : 0);
}






void lv_obj_set_drag(lv_obj_t * obj, bool en)
{
LV_ASSERT_OBJ(obj, LV_OBJX_NAME);

if(en == true) lv_obj_set_click(obj, true); 
obj->drag = (en == true ? 1 : 0);
}






void lv_obj_set_drag_dir(lv_obj_t * obj, lv_drag_dir_t drag_dir)
{
LV_ASSERT_OBJ(obj, LV_OBJX_NAME);

obj->drag_dir = drag_dir;

if(obj->drag_dir != 0) lv_obj_set_drag(obj, true); 
}






void lv_obj_set_drag_throw(lv_obj_t * obj, bool en)
{
LV_ASSERT_OBJ(obj, LV_OBJX_NAME);

obj->drag_throw = (en == true ? 1 : 0);
}







void lv_obj_set_drag_parent(lv_obj_t * obj, bool en)
{
LV_ASSERT_OBJ(obj, LV_OBJX_NAME);

obj->drag_parent = (en == true ? 1 : 0);
}






void lv_obj_set_parent_event(lv_obj_t * obj, bool en)
{
LV_ASSERT_OBJ(obj, LV_OBJX_NAME);

obj->parent_event = (en == true ? 1 : 0);
}

void lv_obj_set_base_dir(lv_obj_t * obj, lv_bidi_dir_t dir)
{
if(dir != LV_BIDI_DIR_LTR && dir != LV_BIDI_DIR_RTL &&
dir != LV_BIDI_DIR_AUTO && dir != LV_BIDI_DIR_INHERIT) {

LV_LOG_WARN("lv_obj_set_base_dir: invalid base dir");
return;
}

obj->base_dir = dir;
lv_signal_send(obj, LV_SIGNAL_BASE_DIR_CHG, NULL);



base_dir_refr_children(obj);
}






void lv_obj_set_opa_scale_enable(lv_obj_t * obj, bool en)
{
LV_ASSERT_OBJ(obj, LV_OBJX_NAME);

obj->opa_scale_en = en ? 1 : 0;
}









void lv_obj_set_opa_scale(lv_obj_t * obj, lv_opa_t opa_scale)
{
LV_ASSERT_OBJ(obj, LV_OBJX_NAME);

obj->opa_scale = opa_scale;
lv_obj_invalidate(obj);
}






void lv_obj_set_protect(lv_obj_t * obj, uint8_t prot)
{
LV_ASSERT_OBJ(obj, LV_OBJX_NAME);

obj->protect |= prot;
}






void lv_obj_clear_protect(lv_obj_t * obj, uint8_t prot)
{
LV_ASSERT_OBJ(obj, LV_OBJX_NAME);

prot = (~prot) & 0xFF;
obj->protect &= prot;
}







void lv_obj_set_event_cb(lv_obj_t * obj, lv_event_cb_t event_cb)
{
LV_ASSERT_OBJ(obj, LV_OBJX_NAME);

obj->event_cb = event_cb;
}








lv_res_t lv_event_send(lv_obj_t * obj, lv_event_t event, const void * data)
{
if(obj == NULL) return LV_RES_OK;

LV_ASSERT_OBJ(obj, LV_OBJX_NAME);

lv_res_t res;
res = lv_event_send_func(obj->event_cb, obj, event, data);
return res;
}











lv_res_t lv_event_send_func(lv_event_cb_t event_xcb, lv_obj_t * obj, lv_event_t event, const void * data)
{
if(obj != NULL) {
LV_ASSERT_OBJ(obj, LV_OBJX_NAME);
}




lv_event_temp_data_t event_temp_data;
event_temp_data.obj = obj;
event_temp_data.deleted = false;
event_temp_data.prev = NULL;

if(event_temp_data_head) {
event_temp_data.prev = event_temp_data_head;
}
event_temp_data_head = &event_temp_data;

const void * event_act_data_save = event_act_data;
event_act_data = data;


lv_indev_t * indev_act = lv_indev_get_act();
if(indev_act) {
if(indev_act->driver.feedback_cb) indev_act->driver.feedback_cb(&indev_act->driver, event);
}


if(event_xcb) event_xcb(obj, event);


event_act_data = event_act_data_save;


event_temp_data_head = event_temp_data_head->prev;

if(event_temp_data.deleted) {
return LV_RES_INV;
}

if(obj) {
if(obj->parent_event && obj->par) {
lv_res_t res = lv_event_send(obj->par, event, data);
if(res != LV_RES_OK) {
return LV_RES_INV;
}
}
}

return LV_RES_OK;
}





const void * lv_event_get_data(void)
{
return event_act_data;
}







void lv_obj_set_signal_cb(lv_obj_t * obj, lv_signal_cb_t signal_cb)
{
LV_ASSERT_OBJ(obj, LV_OBJX_NAME);

obj->signal_cb = signal_cb;
}






void lv_signal_send(lv_obj_t * obj, lv_signal_t signal, void * param)
{
LV_ASSERT_OBJ(obj, LV_OBJX_NAME);

if(obj->signal_cb) obj->signal_cb(obj, signal, param);
}






void lv_obj_set_design_cb(lv_obj_t * obj, lv_design_cb_t design_cb)
{
LV_ASSERT_OBJ(obj, LV_OBJX_NAME);

obj->design_cb = design_cb;
}











void * lv_obj_allocate_ext_attr(lv_obj_t * obj, uint16_t ext_size)
{
LV_ASSERT_OBJ(obj, LV_OBJX_NAME);

obj->ext_attr = lv_mem_realloc(obj->ext_attr, ext_size);

return (void *)obj->ext_attr;
}





void lv_obj_refresh_ext_draw_pad(lv_obj_t * obj)
{
LV_ASSERT_OBJ(obj, LV_OBJX_NAME);

obj->ext_draw_pad = 0;
obj->signal_cb(obj, LV_SIGNAL_REFR_EXT_DRAW_PAD, NULL);

lv_obj_invalidate(obj);
}










lv_obj_t * lv_obj_get_screen(const lv_obj_t * obj)
{
LV_ASSERT_OBJ(obj, LV_OBJX_NAME);

const lv_obj_t * par = obj;
const lv_obj_t * act_p;

do {
act_p = par;
par = lv_obj_get_parent(act_p);
} while(par != NULL);

return (lv_obj_t *)act_p;
}






lv_disp_t * lv_obj_get_disp(const lv_obj_t * obj)
{
LV_ASSERT_OBJ(obj, LV_OBJX_NAME);

const lv_obj_t * scr;

if(obj->par == NULL)
scr = obj; 
else
scr = lv_obj_get_screen(obj); 

lv_disp_t * d;
LV_LL_READ(LV_GC_ROOT(_lv_disp_ll), d)
{
lv_obj_t * s;
LV_LL_READ(d->scr_ll, s)
{
if(s == scr) return d;
}
}

LV_LOG_WARN("lv_scr_get_disp: screen not found")
return NULL;
}










lv_obj_t * lv_obj_get_parent(const lv_obj_t * obj)
{
LV_ASSERT_OBJ(obj, LV_OBJX_NAME);

return obj->par;
}








lv_obj_t * lv_obj_get_child(const lv_obj_t * obj, const lv_obj_t * child)
{
LV_ASSERT_OBJ(obj, LV_OBJX_NAME);

lv_obj_t * result = NULL;

if(child == NULL) {
result = lv_ll_get_head(&obj->child_ll);
} else {
result = lv_ll_get_next(&obj->child_ll, child);
}

return result;
}








lv_obj_t * lv_obj_get_child_back(const lv_obj_t * obj, const lv_obj_t * child)
{
LV_ASSERT_OBJ(obj, LV_OBJX_NAME);

lv_obj_t * result = NULL;

if(child == NULL) {
result = lv_ll_get_tail(&obj->child_ll);
} else {
result = lv_ll_get_prev(&obj->child_ll, child);
}

return result;
}






uint16_t lv_obj_count_children(const lv_obj_t * obj)
{
LV_ASSERT_OBJ(obj, LV_OBJX_NAME);

lv_obj_t * i;
uint16_t cnt = 0;

LV_LL_READ(obj->child_ll, i) cnt++;

return cnt;
}





uint16_t lv_obj_count_children_recursive(const lv_obj_t * obj)
{
LV_ASSERT_OBJ(obj, LV_OBJX_NAME);

lv_obj_t * i;
uint16_t cnt = 0;

LV_LL_READ(obj->child_ll, i)
{
cnt++; 
cnt += lv_obj_count_children_recursive(i); 
}

return cnt;
}










void lv_obj_get_coords(const lv_obj_t * obj, lv_area_t * cords_p)
{
LV_ASSERT_OBJ(obj, LV_OBJX_NAME);

lv_area_copy(cords_p, &obj->coords);
}






void lv_obj_get_inner_coords(const lv_obj_t * obj, lv_area_t * coords_p)
{
LV_ASSERT_OBJ(obj, LV_OBJX_NAME);

const lv_style_t * style = lv_obj_get_style(obj);
if(style->body.border.part & LV_BORDER_LEFT) coords_p->x1 += style->body.border.width;

if(style->body.border.part & LV_BORDER_RIGHT) coords_p->x2 -= style->body.border.width;

if(style->body.border.part & LV_BORDER_TOP) coords_p->y1 += style->body.border.width;

if(style->body.border.part & LV_BORDER_BOTTOM) coords_p->y2 -= style->body.border.width;
}






lv_coord_t lv_obj_get_x(const lv_obj_t * obj)
{
LV_ASSERT_OBJ(obj, LV_OBJX_NAME);

lv_coord_t rel_x;
lv_obj_t * parent = lv_obj_get_parent(obj);
if(parent) {
rel_x = obj->coords.x1 - parent->coords.x1;
} else {
rel_x = obj->coords.x1;
}
return rel_x;
}






lv_coord_t lv_obj_get_y(const lv_obj_t * obj)
{
LV_ASSERT_OBJ(obj, LV_OBJX_NAME);

lv_coord_t rel_y;
lv_obj_t * parent = lv_obj_get_parent(obj);
if(parent) {
rel_y = obj->coords.y1 - parent->coords.y1;
} else {
rel_y = obj->coords.y1;
}
return rel_y;
}






lv_coord_t lv_obj_get_width(const lv_obj_t * obj)
{
LV_ASSERT_OBJ(obj, LV_OBJX_NAME);

return lv_area_get_width(&obj->coords);
}






lv_coord_t lv_obj_get_height(const lv_obj_t * obj)
{
LV_ASSERT_OBJ(obj, LV_OBJX_NAME);

return lv_area_get_height(&obj->coords);
}






lv_coord_t lv_obj_get_width_fit(const lv_obj_t * obj)
{
LV_ASSERT_OBJ(obj, LV_OBJX_NAME);

const lv_style_t * style = lv_obj_get_style(obj);

return lv_obj_get_width(obj) - style->body.padding.left - style->body.padding.right;
}






lv_coord_t lv_obj_get_height_fit(const lv_obj_t * obj)
{
LV_ASSERT_OBJ(obj, LV_OBJX_NAME);

const lv_style_t * style = lv_obj_get_style(obj);

return lv_obj_get_height(obj) - style->body.padding.top - style->body.padding.bottom;
}






bool lv_obj_get_auto_realign(const lv_obj_t * obj)
{
LV_ASSERT_OBJ(obj, LV_OBJX_NAME);

#if LV_USE_OBJ_REALIGN
return obj->realign.auto_realign ? true : false;
#else
(void)obj;
return false;
#endif
}






lv_coord_t lv_obj_get_ext_click_pad_left(const lv_obj_t * obj)
{
LV_ASSERT_OBJ(obj, LV_OBJX_NAME);

#if LV_USE_EXT_CLICK_AREA == LV_EXT_CLICK_AREA_TINY
return obj->ext_click_pad_hor;
#elif LV_USE_EXT_CLICK_AREA == LV_EXT_CLICK_AREA_FULL
return obj->ext_click_pad.x1;
#else
(void)obj; 
return 0;
#endif
}






lv_coord_t lv_obj_get_ext_click_pad_right(const lv_obj_t * obj)
{
LV_ASSERT_OBJ(obj, LV_OBJX_NAME);

#if LV_USE_EXT_CLICK_AREA == LV_EXT_CLICK_AREA_TINY
return obj->ext_click_pad_hor;
#elif LV_USE_EXT_CLICK_AREA == LV_EXT_CLICK_AREA_FULL
return obj->ext_click_pad.x2;
#else
(void)obj; 
return 0;
#endif
}






lv_coord_t lv_obj_get_ext_click_pad_top(const lv_obj_t * obj)
{
LV_ASSERT_OBJ(obj, LV_OBJX_NAME);

#if LV_USE_EXT_CLICK_AREA == LV_EXT_CLICK_AREA_TINY
return obj->ext_click_pad_ver;
#elif LV_USE_EXT_CLICK_AREA == LV_EXT_CLICK_AREA_FULL
return obj->ext_click_pad.y1;
#else
(void)obj; 
return 0;
#endif
}






lv_coord_t lv_obj_get_ext_click_pad_bottom(const lv_obj_t * obj)
{
LV_ASSERT_OBJ(obj, LV_OBJX_NAME);

#if LV_USE_EXT_CLICK_AREA == LV_EXT_CLICK_AREA_TINY
return obj->ext_click_pad_ver;
#elif LV_USE_EXT_CLICK_AREA == LV_EXT_CLICK_AREA_FULL
return obj->ext_click_pad.y2;
#else
(void)obj; 
return 0;
#endif
}






lv_coord_t lv_obj_get_ext_draw_pad(const lv_obj_t * obj)
{
LV_ASSERT_OBJ(obj, LV_OBJX_NAME);

return obj->ext_draw_pad;
}










const lv_style_t * lv_obj_get_style(const lv_obj_t * obj)
{
LV_ASSERT_OBJ(obj, LV_OBJX_NAME);

const lv_style_t * style_act = obj->style_p;
if(style_act == NULL) {
lv_obj_t * par = obj->par;

while(par) {
if(par->style_p) {
if(par->style_p->glass == 0) {
#if LV_USE_GROUP == 0
style_act = par->style_p;
#else

lv_group_t * g = lv_obj_get_group(par);
if(lv_group_get_focused(g) == par) {
style_act = lv_group_mod_style(g, par->style_p);
} else {
style_act = par->style_p;
}
#endif
break;
}
}
par = par->par;
}
}
#if LV_USE_GROUP
if(obj->group_p) {
if(lv_group_get_focused(obj->group_p) == obj) {
style_act = lv_group_mod_style(obj->group_p, style_act);
}
}
#endif

if(style_act == NULL) style_act = &lv_style_plain;

return style_act;
}










bool lv_obj_get_hidden(const lv_obj_t * obj)
{
LV_ASSERT_OBJ(obj, LV_OBJX_NAME);

return obj->hidden == 0 ? false : true;
}






bool lv_obj_get_click(const lv_obj_t * obj)
{
LV_ASSERT_OBJ(obj, LV_OBJX_NAME);

return obj->click == 0 ? false : true;
}






bool lv_obj_get_top(const lv_obj_t * obj)
{
LV_ASSERT_OBJ(obj, LV_OBJX_NAME);

return obj->top == 0 ? false : true;
}






bool lv_obj_get_drag(const lv_obj_t * obj)
{
LV_ASSERT_OBJ(obj, LV_OBJX_NAME);

return obj->drag == 0 ? false : true;
}






lv_drag_dir_t lv_obj_get_drag_dir(const lv_obj_t * obj)
{
LV_ASSERT_OBJ(obj, LV_OBJX_NAME);

return obj->drag_dir;
}






bool lv_obj_get_drag_throw(const lv_obj_t * obj)
{
LV_ASSERT_OBJ(obj, LV_OBJX_NAME);

return obj->drag_throw == 0 ? false : true;
}






bool lv_obj_get_drag_parent(const lv_obj_t * obj)
{
return obj->drag_parent == 0 ? false : true;
}






bool lv_obj_get_parent_event(const lv_obj_t * obj)
{
LV_ASSERT_OBJ(obj, LV_OBJX_NAME);

return obj->parent_event == 0 ? false : true;
}


lv_bidi_dir_t lv_obj_get_base_dir(const lv_obj_t * obj)
{
#if LV_USE_BIDI
const lv_obj_t * parent = obj;

while(parent) {
if(parent->base_dir != LV_BIDI_DIR_INHERIT) return parent->base_dir;

parent = lv_obj_get_parent(parent);
}

return LV_BIDI_BASE_DIR_DEF;
#else
(void) obj; 
return LV_BIDI_DIR_LTR;
#endif
}







lv_opa_t lv_obj_get_opa_scale_enable(const lv_obj_t * obj)
{
LV_ASSERT_OBJ(obj, LV_OBJX_NAME);

return obj->opa_scale_en == 0 ? false : true;
}






lv_opa_t lv_obj_get_opa_scale(const lv_obj_t * obj)
{
LV_ASSERT_OBJ(obj, LV_OBJX_NAME);

const lv_obj_t * parent = obj;

while(parent) {
if(parent->opa_scale_en) return parent->opa_scale;
parent = lv_obj_get_parent(parent);
}

return LV_OPA_COVER;
}






uint8_t lv_obj_get_protect(const lv_obj_t * obj)
{
LV_ASSERT_OBJ(obj, LV_OBJX_NAME);

return obj->protect;
}







bool lv_obj_is_protected(const lv_obj_t * obj, uint8_t prot)
{
LV_ASSERT_OBJ(obj, LV_OBJX_NAME);

return (obj->protect & prot) == 0 ? false : true;
}






lv_signal_cb_t lv_obj_get_signal_cb(const lv_obj_t * obj)
{
LV_ASSERT_OBJ(obj, LV_OBJX_NAME);

return obj->signal_cb;
}






lv_design_cb_t lv_obj_get_design_cb(const lv_obj_t * obj)
{
LV_ASSERT_OBJ(obj, LV_OBJX_NAME);

return obj->design_cb;
}






lv_event_cb_t lv_obj_get_event_cb(const lv_obj_t * obj)
{
LV_ASSERT_OBJ(obj, LV_OBJX_NAME);

return obj->event_cb;
}











void * lv_obj_get_ext_attr(const lv_obj_t * obj)
{
LV_ASSERT_OBJ(obj, LV_OBJX_NAME);

return obj->ext_attr;
}







void lv_obj_get_type(const lv_obj_t * obj, lv_obj_type_t * buf)
{
LV_ASSERT_NULL(buf);
LV_ASSERT_NULL(obj);

lv_obj_type_t tmp;

memset(buf, 0, sizeof(lv_obj_type_t));
memset(&tmp, 0, sizeof(lv_obj_type_t));

obj->signal_cb((lv_obj_t *)obj, LV_SIGNAL_GET_TYPE, &tmp);

uint8_t cnt;
for(cnt = 0; cnt < LV_MAX_ANCESTOR_NUM; cnt++) {
if(tmp.type[cnt] == NULL) break;
}


uint8_t i;
for(i = 0; i < cnt; i++) {
buf->type[i] = tmp.type[cnt - 1 - i];
}
}

#if LV_USE_USER_DATA






lv_obj_user_data_t lv_obj_get_user_data(const lv_obj_t * obj)
{
LV_ASSERT_OBJ(obj, LV_OBJX_NAME);

return obj->user_data;
}






lv_obj_user_data_t * lv_obj_get_user_data_ptr(const lv_obj_t * obj)
{
LV_ASSERT_OBJ(obj, LV_OBJX_NAME);

return (lv_obj_user_data_t *)&obj->user_data;
}






void lv_obj_set_user_data(lv_obj_t * obj, lv_obj_user_data_t data)
{
LV_ASSERT_OBJ(obj, LV_OBJX_NAME);

memcpy(&obj->user_data, &data, sizeof(lv_obj_user_data_t));
}
#endif

#if LV_USE_GROUP





void * lv_obj_get_group(const lv_obj_t * obj)
{
LV_ASSERT_OBJ(obj, LV_OBJX_NAME);

return obj->group_p;
}






bool lv_obj_is_focused(const lv_obj_t * obj)
{
LV_ASSERT_OBJ(obj, LV_OBJX_NAME);

if(obj->group_p) {
if(lv_group_get_focused(obj->group_p) == obj) return true;
}

return false;
}
#endif













lv_res_t lv_obj_handle_get_type_signal(lv_obj_type_t * buf, const char * name)
{
uint8_t i;
for(i = 0; i < LV_MAX_ANCESTOR_NUM - 1; i++) { 
if(buf->type[i] == NULL) break;
}
buf->type[i] = name;

return LV_RES_OK;
}





static void lv_obj_del_async_cb(void * obj)
{
LV_ASSERT_OBJ(obj, LV_OBJX_NAME);

lv_obj_del(obj);
}










static bool lv_obj_design(lv_obj_t * obj, const lv_area_t * mask_p, lv_design_mode_t mode)
{
if(mode == LV_DESIGN_COVER_CHK) {


if(lv_area_is_in(mask_p, &obj->coords) == false) return false;


const lv_style_t * style = lv_obj_get_style(obj);
if(style->body.opa < LV_OPA_MAX) return false;



lv_coord_t r = style->body.radius;

if(r == LV_RADIUS_CIRCLE) return false;

lv_area_t area_tmp;


lv_obj_get_coords(obj, &area_tmp);
area_tmp.x1 += r;
area_tmp.x2 -= r;
if(lv_area_is_in(mask_p, &area_tmp) == false) return false;


lv_obj_get_coords(obj, &area_tmp);
area_tmp.y1 += r;
area_tmp.y2 -= r;
if(lv_area_is_in(mask_p, &area_tmp) == false) return false;

} else if(mode == LV_DESIGN_DRAW_MAIN) {
const lv_style_t * style = lv_obj_get_style(obj);
lv_draw_rect(&obj->coords, mask_p, style, lv_obj_get_opa_scale(obj));
}

return true;
}








static lv_res_t lv_obj_signal(lv_obj_t * obj, lv_signal_t sign, void * param)
{
if(sign == LV_SIGNAL_GET_TYPE) return lv_obj_handle_get_type_signal(param, LV_OBJX_NAME);

lv_res_t res = LV_RES_OK;

if(sign == LV_SIGNAL_CHILD_CHG) {

if(lv_obj_is_protected(obj, LV_PROTECT_CHILD_CHG) != false) res = LV_RES_INV;
} else if(sign == LV_SIGNAL_REFR_EXT_DRAW_PAD) {
const lv_style_t * style = lv_obj_get_style(obj);
if(style->body.shadow.width > obj->ext_draw_pad) obj->ext_draw_pad = style->body.shadow.width;
} else if(sign == LV_SIGNAL_STYLE_CHG) {
lv_obj_refresh_ext_draw_pad(obj);
}
return res;
}







static void refresh_children_position(lv_obj_t * obj, lv_coord_t x_diff, lv_coord_t y_diff)
{
lv_obj_t * i;
LV_LL_READ(obj->child_ll, i)
{
i->coords.x1 += x_diff;
i->coords.y1 += y_diff;
i->coords.x2 += x_diff;
i->coords.y2 += y_diff;

refresh_children_position(i, x_diff, y_diff);
}
}






static void report_style_mod_core(void * style_p, lv_obj_t * obj)
{
lv_obj_t * i;
LV_LL_READ(obj->child_ll, i)
{
if(i->style_p == style_p || style_p == NULL) {
refresh_children_style(i);
lv_obj_refresh_style(i);
}

report_style_mod_core(style_p, i);
}
}






static void refresh_children_style(lv_obj_t * obj)
{
lv_obj_t * child = lv_obj_get_child(obj, NULL);
while(child != NULL) {
if(child->style_p == NULL) {
refresh_children_style(child); 
lv_obj_refresh_style(child); 
} else if(child->style_p->glass) {

refresh_children_style(child);
}
child = lv_obj_get_child(obj, child);
}
}





static void delete_children(lv_obj_t * obj)
{
lv_obj_t * i;
lv_obj_t * i_next;
i = lv_ll_get_head(&(obj->child_ll));




#if LV_USE_GROUP
lv_group_t * group = lv_obj_get_group(obj);
if(group) lv_group_remove_obj(obj);
#endif

while(i != NULL) {

i_next = lv_ll_get_next(&(obj->child_ll), i);


delete_children(i);


i = i_next;
}


lv_event_send(obj, LV_EVENT_DELETE, NULL);

lv_event_mark_deleted(obj);


#if LV_USE_ANIMATION
lv_anim_del(obj, NULL);
#endif



lv_indev_t * indev = lv_indev_get_next(NULL);
while(indev) {
if(indev->proc.types.pointer.act_obj == obj || indev->proc.types.pointer.last_obj == obj) {
lv_indev_reset(indev);
}

if(indev->proc.types.pointer.last_pressed == obj) {
indev->proc.types.pointer.last_pressed = NULL;
}
#if LV_USE_GROUP
if(indev->group == group && obj == lv_indev_get_obj_act()) {
lv_indev_reset(indev);
}
#endif
indev = lv_indev_get_next(indev);
}


obj->signal_cb(obj, LV_SIGNAL_CLEANUP, NULL);


lv_obj_t * par = lv_obj_get_parent(obj);
lv_ll_rem(&(par->child_ll), obj);


if(obj->ext_attr != NULL) lv_mem_free(obj->ext_attr);
lv_mem_free(obj); 
}

static void base_dir_refr_children(lv_obj_t * obj)
{
lv_obj_t * child;
child = lv_obj_get_child(obj, NULL);

while(child) {
if(child->base_dir == LV_BIDI_DIR_INHERIT) {
lv_signal_send(child, LV_SIGNAL_BASE_DIR_CHG, NULL);
base_dir_refr_children(child);
}

child = lv_obj_get_child(obj, child);
}
}


static void lv_event_mark_deleted(lv_obj_t * obj)
{
lv_event_temp_data_t * t = event_temp_data_head;

while(t) {
if(t->obj == obj) t->deleted = true;
t = t->prev;
}
}
