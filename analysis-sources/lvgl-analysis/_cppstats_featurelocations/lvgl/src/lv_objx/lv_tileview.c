







#include "lv_tileview.h"
#if LV_USE_TILEVIEW != 0

#include <stdbool.h>
#include "lv_cont.h"
#include "../lv_core/lv_debug.h"
#include "../lv_themes/lv_theme.h"




#define LV_OBJX_NAME "lv_tileview"

#if LV_USE_ANIMATION
#if !defined(LV_TILEVIEW_DEF_ANIM_TIME)
#define LV_TILEVIEW_DEF_ANIM_TIME 300 
#endif
#else
#undef LV_TILEVIEW_DEF_ANIM_TIME
#define LV_TILEVIEW_DEF_ANIM_TIME 0 
#endif








static lv_res_t lv_tileview_signal(lv_obj_t * tileview, lv_signal_t sign, void * param);
static lv_res_t lv_tileview_scrl_signal(lv_obj_t * scrl, lv_signal_t sign, void * param);
static void tileview_scrl_event_cb(lv_obj_t * scrl, lv_event_t event);
static void drag_end_handler(lv_obj_t * tileview);
static bool set_valid_drag_dirs(lv_obj_t * tileview);




static lv_signal_cb_t ancestor_signal;
static lv_signal_cb_t ancestor_scrl_signal;
static lv_design_cb_t ancestor_design;















lv_obj_t * lv_tileview_create(lv_obj_t * par, const lv_obj_t * copy)
{
LV_LOG_TRACE("tileview create started");


lv_obj_t * new_tileview = lv_page_create(par, copy);
LV_ASSERT_MEM(new_tileview);
if(new_tileview == NULL) return NULL;


lv_tileview_ext_t * ext = lv_obj_allocate_ext_attr(new_tileview, sizeof(lv_tileview_ext_t));
LV_ASSERT_MEM(ext);
if(ext == NULL) return NULL;
if(ancestor_signal == NULL) ancestor_signal = lv_obj_get_signal_cb(new_tileview);
if(ancestor_scrl_signal == NULL) ancestor_scrl_signal = lv_obj_get_signal_cb(lv_page_get_scrl(new_tileview));
if(ancestor_design == NULL) ancestor_design = lv_obj_get_design_cb(new_tileview);


#if LV_USE_ANIMATION
ext->anim_time = LV_TILEVIEW_DEF_ANIM_TIME;
#endif
ext->act_id.x = 0;
ext->act_id.y = 0;
ext->valid_pos = NULL;
ext->valid_pos_cnt = 0;


lv_obj_set_signal_cb(new_tileview, lv_tileview_signal);
lv_obj_set_signal_cb(lv_page_get_scrl(new_tileview), lv_tileview_scrl_signal);


if(copy == NULL) {



lv_coord_t w;
lv_coord_t h;
if(par) {
w = lv_obj_get_width_fit(lv_obj_get_parent(new_tileview));
h = lv_obj_get_height_fit(lv_obj_get_parent(new_tileview));
} else {
w = lv_disp_get_hor_res(NULL);
h = lv_disp_get_ver_res(NULL);
}

lv_obj_set_size(new_tileview, w, h);

lv_obj_set_drag_throw(lv_page_get_scrl(new_tileview), false);
lv_page_set_scrl_fit(new_tileview, LV_FIT_TIGHT);
lv_obj_set_event_cb(ext->page.scrl, tileview_scrl_event_cb);

lv_theme_t * th = lv_theme_get_current();
if(th) {
lv_page_set_style(new_tileview, LV_PAGE_STYLE_BG, th->style.tileview.bg);
lv_page_set_style(new_tileview, LV_PAGE_STYLE_SCRL, th->style.tileview.scrl);
lv_page_set_style(new_tileview, LV_PAGE_STYLE_SB, th->style.tileview.sb);
} else {
lv_page_set_style(new_tileview, LV_PAGE_STYLE_BG, &lv_style_transp_tight);
lv_page_set_style(new_tileview, LV_PAGE_STYLE_SCRL, &lv_style_transp_tight);
}
}

else {
lv_tileview_ext_t * copy_ext = lv_obj_get_ext_attr(copy);
ext->act_id.x = copy_ext->act_id.x;
ext->act_id.y = copy_ext->act_id.y;
ext->valid_pos = copy_ext->valid_pos;
ext->valid_pos_cnt = copy_ext->valid_pos_cnt;
#if LV_USE_ANIMATION
ext->anim_time = copy_ext->anim_time;
#endif


lv_obj_refresh_style(new_tileview);
}

LV_LOG_INFO("tileview created");

return new_tileview;
}










void lv_tileview_add_element(lv_obj_t * tileview, lv_obj_t * element)
{
LV_ASSERT_OBJ(tileview, LV_OBJX_NAME);
LV_ASSERT_NULL(tileview);



element->parent_event = 1;
lv_obj_set_drag_parent(element, true);




lv_tileview_ext_t * ext = lv_obj_get_ext_attr(tileview);
lv_tileview_set_tile_act(tileview, ext->act_id.x, ext->act_id.y, false);
}












void lv_tileview_set_valid_positions(lv_obj_t * tileview, const lv_point_t valid_pos[], uint16_t valid_pos_cnt)
{
LV_ASSERT_OBJ(tileview, LV_OBJX_NAME);
LV_ASSERT_NULL(valid_pos);

lv_tileview_ext_t * ext = lv_obj_get_ext_attr(tileview);
ext->valid_pos = valid_pos;
ext->valid_pos_cnt = valid_pos_cnt;


uint16_t i;
for(i = 0; i < valid_pos_cnt; i++) {
if(valid_pos[i].x == ext->act_id.x && valid_pos[i].y == ext->act_id.y) {
return;
}
}


if(valid_pos_cnt > 0) {
lv_tileview_set_tile_act(tileview, valid_pos[0].x, valid_pos[0].y, LV_ANIM_OFF);
}
}








void lv_tileview_set_tile_act(lv_obj_t * tileview, lv_coord_t x, lv_coord_t y, lv_anim_enable_t anim)
{
LV_ASSERT_OBJ(tileview, LV_OBJX_NAME);

#if LV_USE_ANIMATION == 0
anim = LV_ANIM_OFF;
#endif

lv_tileview_ext_t * ext = lv_obj_get_ext_attr(tileview);

uint32_t tile_id;
bool valid = false;
for(tile_id = 0; tile_id < ext->valid_pos_cnt; tile_id++) {
if(ext->valid_pos[tile_id].x == x && ext->valid_pos[tile_id].y == y) {
valid = true;
break;
}
}

if(valid == false) return; 

ext->act_id.x = x;
ext->act_id.y = y;

lv_coord_t x_coord = -x * lv_obj_get_width(tileview);
lv_coord_t y_coord = -y * lv_obj_get_height(tileview);
lv_obj_t * scrl = lv_page_get_scrl(tileview);
if(anim) {
#if LV_USE_ANIMATION
lv_coord_t x_act = lv_obj_get_x(scrl);
lv_coord_t y_act = lv_obj_get_y(scrl);

lv_anim_t a;
a.var = scrl;
a.exec_cb = (lv_anim_exec_xcb_t)lv_obj_set_x;
a.path_cb = lv_anim_path_linear;
a.ready_cb = NULL;
a.act_time = 0;
a.time = ext->anim_time;
a.playback = 0;
a.playback_pause = 0;
a.repeat = 0;
a.repeat_pause = 0;

if(x_coord != x_act) {
a.start = x_act;
a.end = x_coord;
lv_anim_create(&a);
}

if(y_coord != y_act) {
a.start = y_act;
a.end = y_coord;
a.exec_cb = (lv_anim_exec_xcb_t)lv_obj_set_y;
lv_anim_create(&a);
}
#endif
} else {
lv_obj_set_pos(scrl, x_coord, y_coord);
}

lv_res_t res = LV_RES_OK;
res = lv_event_send(tileview, LV_EVENT_VALUE_CHANGED, &tile_id);
if(res != LV_RES_OK) return; 
}







void lv_tileview_set_style(lv_obj_t * tileview, lv_tileview_style_t type, const lv_style_t * style)
{
LV_ASSERT_OBJ(tileview, LV_OBJX_NAME);

switch(type) {
case LV_TILEVIEW_STYLE_MAIN: lv_obj_set_style(tileview, style); break;
}
}















const lv_style_t * lv_tileview_get_style(const lv_obj_t * tileview, lv_tileview_style_t type)
{
LV_ASSERT_OBJ(tileview, LV_OBJX_NAME);

const lv_style_t * style = NULL;
switch(type) {
case LV_TILEVIEW_STYLE_MAIN: style = lv_obj_get_style(tileview); break;
default: style = NULL;
}

return style;
}




















static lv_res_t lv_tileview_signal(lv_obj_t * tileview, lv_signal_t sign, void * param)
{
lv_res_t res;


res = ancestor_signal(tileview, sign, param);
if(res != LV_RES_OK) return res;
if(sign == LV_SIGNAL_GET_TYPE) return lv_obj_handle_get_type_signal(param, LV_OBJX_NAME);

if(sign == LV_SIGNAL_CLEANUP) {

}

return res;
}








static lv_res_t lv_tileview_scrl_signal(lv_obj_t * scrl, lv_signal_t sign, void * param)
{

lv_res_t res;


res = ancestor_scrl_signal(scrl, sign, param);
if(res != LV_RES_OK) return res;
if(sign == LV_SIGNAL_GET_TYPE) return lv_obj_handle_get_type_signal(param, "");

lv_obj_t * tileview = lv_obj_get_parent(scrl);
const lv_style_t * style_bg = lv_tileview_get_style(tileview, LV_TILEVIEW_STYLE_MAIN);


if(sign == LV_SIGNAL_CORD_CHG) {
lv_indev_t * indev = lv_indev_get_act();
if(indev) {
lv_tileview_ext_t * ext = lv_obj_get_ext_attr(tileview);



if(ext->drag_ver == 0 &&
((ext->drag_right_en && indev->proc.types.pointer.drag_sum.x <= -LV_INDEV_DEF_DRAG_LIMIT) ||
(ext->drag_left_en && indev->proc.types.pointer.drag_sum.x >= LV_INDEV_DEF_DRAG_LIMIT))) {
ext->drag_hor = 1;
}


if(ext->drag_hor == 0 &&
((ext->drag_bottom_en && indev->proc.types.pointer.drag_sum.y <= -LV_INDEV_DEF_DRAG_LIMIT) ||
(ext->drag_top_en && indev->proc.types.pointer.drag_sum.y >= LV_INDEV_DEF_DRAG_LIMIT))) {
ext->drag_ver = 1;
}

#if LV_USE_ANIMATION
if(ext->drag_hor) {
ext->page.edge_flash.top_ip = 0;
ext->page.edge_flash.bottom_ip = 0;
}

if(ext->drag_ver) {
ext->page.edge_flash.right_ip = 0;
ext->page.edge_flash.left_ip = 0;
}
#endif

lv_coord_t x = lv_obj_get_x(scrl);
lv_coord_t y = lv_obj_get_y(scrl);
lv_coord_t h = lv_obj_get_height(tileview);
lv_coord_t w = lv_obj_get_width(tileview);
if(ext->drag_top_en == 0) {
if(y > -(ext->act_id.y * h) && indev->proc.types.pointer.vect.y > 0 && ext->drag_hor == 0) {
#if LV_USE_ANIMATION
if(ext->page.edge_flash.enabled && ext->page.edge_flash.left_ip == 0 &&
ext->page.edge_flash.right_ip == 0 && ext->page.edge_flash.top_ip == 0 &&
ext->page.edge_flash.bottom_ip == 0) {
ext->page.edge_flash.top_ip = 1;
lv_page_start_edge_flash(tileview);
}
#endif

lv_obj_set_y(scrl, -ext->act_id.y * h + style_bg->body.padding.top);
}
}
if(ext->drag_bottom_en == 0 && indev->proc.types.pointer.vect.y < 0 && ext->drag_hor == 0) {
if(y < -(ext->act_id.y * h)) {
#if LV_USE_ANIMATION
if(ext->page.edge_flash.enabled && ext->page.edge_flash.left_ip == 0 &&
ext->page.edge_flash.right_ip == 0 && ext->page.edge_flash.top_ip == 0 &&
ext->page.edge_flash.bottom_ip == 0) {
ext->page.edge_flash.bottom_ip = 1;
lv_page_start_edge_flash(tileview);
}
#endif
}

lv_obj_set_y(scrl, -ext->act_id.y * h + style_bg->body.padding.top);
}
if(ext->drag_left_en == 0) {
if(x > -(ext->act_id.x * w) && indev->proc.types.pointer.vect.x > 0 && ext->drag_ver == 0) {
#if LV_USE_ANIMATION
if(ext->page.edge_flash.enabled && ext->page.edge_flash.left_ip == 0 &&
ext->page.edge_flash.right_ip == 0 && ext->page.edge_flash.top_ip == 0 &&
ext->page.edge_flash.bottom_ip == 0) {
ext->page.edge_flash.left_ip = 1;
lv_page_start_edge_flash(tileview);
}
#endif

lv_obj_set_x(scrl, -ext->act_id.x * w + style_bg->body.padding.left);
}
}
if(ext->drag_right_en == 0 && indev->proc.types.pointer.vect.x < 0 && ext->drag_ver == 0) {
if(x < -(ext->act_id.x * w)) {
#if LV_USE_ANIMATION
if(ext->page.edge_flash.enabled && ext->page.edge_flash.left_ip == 0 &&
ext->page.edge_flash.right_ip == 0 && ext->page.edge_flash.top_ip == 0 &&
ext->page.edge_flash.bottom_ip == 0) {
ext->page.edge_flash.right_ip = 1;
lv_page_start_edge_flash(tileview);
}
#endif
}

lv_obj_set_x(scrl, -ext->act_id.x * w + style_bg->body.padding.top);
}


if(ext->drag_ver == 0)
lv_obj_set_y(scrl, -ext->act_id.y * lv_obj_get_height(tileview) + style_bg->body.padding.top);
if(ext->drag_hor == 0)
lv_obj_set_x(scrl, -ext->act_id.x * lv_obj_get_width(tileview) + style_bg->body.padding.left);
}
}
return res;
}

static void tileview_scrl_event_cb(lv_obj_t * scrl, lv_event_t event)
{
lv_obj_t * tileview = lv_obj_get_parent(scrl);


if(event == LV_EVENT_PRESSED) {
lv_tileview_ext_t * ext = lv_obj_get_ext_attr(tileview);
ext->drag_hor = 0;
ext->drag_ver = 0;
set_valid_drag_dirs(tileview);
}

else if(event == LV_EVENT_PRESS_LOST || event == LV_EVENT_RELEASED) {


lv_indev_t * indev = lv_indev_get_act();
lv_tileview_ext_t * ext = lv_obj_get_ext_attr(tileview);
if(lv_indev_is_dragging(indev) && (ext->drag_hor || ext->drag_ver)) {
indev->proc.types.pointer.drag_in_prog = 0;
drag_end_handler(tileview);
}

}
}





static void drag_end_handler(lv_obj_t * tileview)
{
lv_tileview_ext_t * ext = lv_obj_get_ext_attr(tileview);
lv_indev_t * indev = lv_indev_get_act();
lv_point_t point_act;
lv_indev_get_point(indev, &point_act);
lv_obj_t * scrl = lv_page_get_scrl(tileview);
lv_point_t p;

p.x = -(scrl->coords.x1 - lv_obj_get_width(tileview) / 2);
p.y = -(scrl->coords.y1 - lv_obj_get_height(tileview) / 2);


if(ext->drag_hor) {
lv_point_t vect;
lv_indev_get_vect(indev, &vect);
lv_coord_t predict = 0;

while(vect.x != 0) {
predict += vect.x;
vect.x = vect.x * (100 - LV_INDEV_DEF_DRAG_THROW) / 100;
}

p.x -= predict;
} else if(ext->drag_ver) {
lv_point_t vect;
lv_indev_get_vect(indev, &vect);
lv_coord_t predict = 0;

while(vect.y != 0) {
predict += vect.y;
vect.y = vect.y * (100 - LV_INDEV_DEF_DRAG_THROW) / 100;
}

p.y -= predict;
}


p.x = p.x / lv_obj_get_width(tileview);
p.y = p.y / lv_obj_get_height(tileview);


lv_coord_t x_move = p.x - ext->act_id.x;
lv_coord_t y_move = p.y - ext->act_id.y;
if(x_move < -1) x_move = -1;
if(x_move > 1) x_move = 1;
if(y_move < -1) y_move = -1;
if(y_move > 1) y_move = 1;


lv_tileview_set_tile_act(tileview, ext->act_id.x + x_move, ext->act_id.y + y_move, true);
}

static bool set_valid_drag_dirs(lv_obj_t * tileview)
{

lv_tileview_ext_t * ext = lv_obj_get_ext_attr(tileview);
if(ext->valid_pos == NULL) return false;

ext->drag_bottom_en = 0;
ext->drag_top_en = 0;
ext->drag_left_en = 0;
ext->drag_right_en = 0;

uint16_t i;
for(i = 0; i < ext->valid_pos_cnt; i++) {
if(ext->valid_pos[i].x == ext->act_id.x && ext->valid_pos[i].y == ext->act_id.y - 1) ext->drag_top_en = 1;
if(ext->valid_pos[i].x == ext->act_id.x && ext->valid_pos[i].y == ext->act_id.y + 1) ext->drag_bottom_en = 1;
if(ext->valid_pos[i].x == ext->act_id.x - 1 && ext->valid_pos[i].y == ext->act_id.y) ext->drag_left_en = 1;
if(ext->valid_pos[i].x == ext->act_id.x + 1 && ext->valid_pos[i].y == ext->act_id.y) ext->drag_right_en = 1;
}

return true;
}

#endif
