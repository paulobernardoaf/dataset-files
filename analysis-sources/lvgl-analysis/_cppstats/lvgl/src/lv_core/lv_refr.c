#include <stddef.h>
#include "lv_refr.h"
#include "lv_disp.h"
#include "../lv_hal/lv_hal_tick.h"
#include "../lv_hal/lv_hal_disp.h"
#include "../lv_misc/lv_task.h"
#include "../lv_misc/lv_mem.h"
#include "../lv_misc/lv_gc.h"
#include "../lv_draw/lv_draw.h"
#if defined(LV_GC_INCLUDE)
#include LV_GC_INCLUDE
#endif 
#define MASK_AREA_DEBUG 0
static void lv_refr_join_area(void);
static void lv_refr_areas(void);
static void lv_refr_area(const lv_area_t * area_p);
static void lv_refr_area_part(const lv_area_t * area_p);
static lv_obj_t * lv_refr_get_top_obj(const lv_area_t * area_p, lv_obj_t * obj);
static void lv_refr_obj_and_children(lv_obj_t * top_p, const lv_area_t * mask_p);
static void lv_refr_obj(lv_obj_t * obj, const lv_area_t * mask_ori_p);
static void lv_refr_vdb_flush(void);
static uint32_t px_num;
static lv_disp_t * disp_refr; 
void lv_refr_init(void)
{
}
void lv_refr_now(lv_disp_t * disp)
{
if(disp) {
lv_disp_refr_task(disp->refr_task);
} else {
lv_disp_t * d;
d = lv_disp_get_next(NULL);
while(d) {
lv_disp_refr_task(d->refr_task);
d = lv_disp_get_next(d);
}
}
}
void lv_inv_area(lv_disp_t * disp, const lv_area_t * area_p)
{
if(!disp) disp = lv_disp_get_default();
if(!disp) return;
if(area_p == NULL) {
disp->inv_p = 0;
return;
}
lv_area_t scr_area;
scr_area.x1 = 0;
scr_area.y1 = 0;
scr_area.x2 = lv_disp_get_hor_res(disp) - 1;
scr_area.y2 = lv_disp_get_ver_res(disp) - 1;
lv_area_t com_area;
bool suc;
suc = lv_area_intersect(&com_area, area_p, &scr_area);
if(suc != false) {
if(disp->driver.rounder_cb) disp->driver.rounder_cb(&disp->driver, &com_area);
uint16_t i;
for(i = 0; i < disp->inv_p; i++) {
if(lv_area_is_in(&com_area, &disp->inv_areas[i]) != false) return;
}
if(disp->inv_p < LV_INV_BUF_SIZE) {
lv_area_copy(&disp->inv_areas[disp->inv_p], &com_area);
} else { 
disp->inv_p = 0;
lv_area_copy(&disp->inv_areas[disp->inv_p], &scr_area);
}
disp->inv_p++;
}
}
lv_disp_t * lv_refr_get_disp_refreshing(void)
{
return disp_refr;
}
void lv_refr_set_disp_refreshing(lv_disp_t * disp)
{
disp_refr = disp;
}
void lv_disp_refr_task(lv_task_t * task)
{
LV_LOG_TRACE("lv_refr_task: started");
uint32_t start = lv_tick_get();
disp_refr = task->user_data;
lv_refr_join_area();
lv_refr_areas();
if(disp_refr->inv_p != 0) {
if(lv_disp_is_true_double_buf(disp_refr) && disp_refr->driver.set_px_cb == NULL) {
lv_disp_buf_t * vdb = lv_disp_get_buf(disp_refr);
lv_refr_vdb_flush();
while(vdb->flushing)
;
uint8_t * buf_act = (uint8_t *)vdb->buf_act;
uint8_t * buf_ina = (uint8_t *)vdb->buf_act == vdb->buf1 ? vdb->buf2 : vdb->buf1;
lv_coord_t hres = lv_disp_get_hor_res(disp_refr);
uint16_t a;
for(a = 0; a < disp_refr->inv_p; a++) {
if(disp_refr->inv_area_joined[a] == 0) {
lv_coord_t y;
uint32_t start_offs =
(hres * disp_refr->inv_areas[a].y1 + disp_refr->inv_areas[a].x1) * sizeof(lv_color_t);
uint32_t line_length = lv_area_get_width(&disp_refr->inv_areas[a]) * sizeof(lv_color_t);
for(y = disp_refr->inv_areas[a].y1; y <= disp_refr->inv_areas[a].y2; y++) {
memcpy(buf_act + start_offs, buf_ina + start_offs, line_length);
start_offs += hres * sizeof(lv_color_t);
}
}
}
} 
memset(disp_refr->inv_areas, 0, sizeof(disp_refr->inv_areas));
memset(disp_refr->inv_area_joined, 0, sizeof(disp_refr->inv_area_joined));
disp_refr->inv_p = 0;
if(disp_refr->driver.monitor_cb) {
disp_refr->driver.monitor_cb(&disp_refr->driver, lv_tick_elaps(start), px_num);
}
}
lv_draw_free_buf();
LV_LOG_TRACE("lv_refr_task: ready");
}
static void lv_refr_join_area(void)
{
uint32_t join_from;
uint32_t join_in;
lv_area_t joined_area;
for(join_in = 0; join_in < disp_refr->inv_p; join_in++) {
if(disp_refr->inv_area_joined[join_in] != 0) continue;
for(join_from = 0; join_from < disp_refr->inv_p; join_from++) {
if(disp_refr->inv_area_joined[join_from] != 0 || join_in == join_from) {
continue;
}
if(lv_area_is_on(&disp_refr->inv_areas[join_in], &disp_refr->inv_areas[join_from]) == false) {
continue;
}
lv_area_join(&joined_area, &disp_refr->inv_areas[join_in], &disp_refr->inv_areas[join_from]);
if(lv_area_get_size(&joined_area) < (lv_area_get_size(&disp_refr->inv_areas[join_in]) +
lv_area_get_size(&disp_refr->inv_areas[join_from]))) {
lv_area_copy(&disp_refr->inv_areas[join_in], &joined_area);
disp_refr->inv_area_joined[join_from] = 1;
}
}
}
}
static void lv_refr_areas(void)
{
px_num = 0;
uint32_t i;
for(i = 0; i < disp_refr->inv_p; i++) {
if(disp_refr->inv_area_joined[i] == 0) {
lv_refr_area(&disp_refr->inv_areas[i]);
if(disp_refr->driver.monitor_cb) px_num += lv_area_get_size(&disp_refr->inv_areas[i]);
}
}
}
static void lv_refr_area(const lv_area_t * area_p)
{
if(lv_disp_is_true_double_buf(disp_refr)) {
lv_disp_buf_t * vdb = lv_disp_get_buf(disp_refr);
vdb->area.x1 = 0;
vdb->area.x2 = lv_disp_get_hor_res(disp_refr) - 1;
vdb->area.y1 = 0;
vdb->area.y2 = lv_disp_get_ver_res(disp_refr) - 1;
lv_refr_area_part(area_p);
}
else {
lv_disp_buf_t * vdb = lv_disp_get_buf(disp_refr);
lv_coord_t w = lv_area_get_width(area_p);
lv_coord_t h = lv_area_get_height(area_p);
lv_coord_t y2 =
area_p->y2 >= lv_disp_get_ver_res(disp_refr) ? y2 = lv_disp_get_ver_res(disp_refr) - 1 : area_p->y2;
int32_t max_row = (uint32_t)vdb->size / w;
if(max_row > h) max_row = h;
if(disp_refr->driver.rounder_cb) {
lv_area_t tmp;
tmp.x1 = 0;
tmp.x2 = 0;
tmp.y1 = 0;
lv_coord_t h_tmp = max_row;
do {
tmp.y2 = h_tmp - 1;
disp_refr->driver.rounder_cb(&disp_refr->driver, &tmp);
if(lv_area_get_height(&tmp) <= max_row) break;
h_tmp--;
} while(h_tmp > 0);
if(h_tmp <= 0) {
LV_LOG_WARN("Can't set VDB height using the round function. (Wrong round_cb or to "
"small VDB)");
return;
} else {
max_row = tmp.y2 + 1;
}
}
lv_coord_t row;
lv_coord_t row_last = 0;
for(row = area_p->y1; row + max_row - 1 <= y2; row += max_row) {
vdb->area.x1 = area_p->x1;
vdb->area.x2 = area_p->x2;
vdb->area.y1 = row;
vdb->area.y2 = row + max_row - 1;
if(vdb->area.y2 > y2) vdb->area.y2 = y2;
row_last = vdb->area.y2;
lv_refr_area_part(area_p);
}
if(y2 != row_last) {
vdb->area.x1 = area_p->x1;
vdb->area.x2 = area_p->x2;
vdb->area.y1 = row;
vdb->area.y2 = y2;
lv_refr_area_part(area_p);
}
}
}
static void lv_refr_area_part(const lv_area_t * area_p)
{
lv_disp_buf_t * vdb = lv_disp_get_buf(disp_refr);
if(lv_disp_is_double_buf(disp_refr) == false) {
while(vdb->flushing)
;
}
lv_obj_t * top_p;
lv_area_t start_mask;
lv_area_intersect(&start_mask, area_p, &vdb->area);
top_p = lv_refr_get_top_obj(&start_mask, lv_disp_get_scr_act(disp_refr));
lv_refr_obj_and_children(top_p, &start_mask);
lv_refr_obj_and_children(lv_disp_get_layer_top(disp_refr), &start_mask);
lv_refr_obj_and_children(lv_disp_get_layer_sys(disp_refr), &start_mask);
if(lv_disp_is_true_double_buf(disp_refr) == false) {
lv_refr_vdb_flush();
}
}
static lv_obj_t * lv_refr_get_top_obj(const lv_area_t * area_p, lv_obj_t * obj)
{
lv_obj_t * found_p = NULL;
if(lv_area_is_in(area_p, &obj->coords) && obj->hidden == 0) {
lv_obj_t * i;
LV_LL_READ(obj->child_ll, i)
{
found_p = lv_refr_get_top_obj(area_p, i);
if(found_p != NULL) {
break;
}
}
if(found_p == NULL) {
const lv_style_t * style = lv_obj_get_style(obj);
if(style->body.opa == LV_OPA_COVER && obj->design_cb(obj, area_p, LV_DESIGN_COVER_CHK) != false &&
lv_obj_get_opa_scale(obj) == LV_OPA_COVER) {
found_p = obj;
}
}
}
return found_p;
}
static void lv_refr_obj_and_children(lv_obj_t * top_p, const lv_area_t * mask_p)
{
if(top_p == NULL) top_p = lv_disp_get_scr_act(disp_refr);
lv_refr_obj(top_p, mask_p);
lv_obj_t * par;
lv_obj_t * border_p = top_p;
par = lv_obj_get_parent(top_p);
while(par != NULL) {
lv_obj_t * i = lv_ll_get_prev(&(par->child_ll), border_p);
while(i != NULL) {
lv_refr_obj(i, mask_p);
i = lv_ll_get_prev(&(par->child_ll), i);
}
par->design_cb(par, mask_p, LV_DESIGN_DRAW_POST);
border_p = par;
par = lv_obj_get_parent(par);
}
}
static void lv_refr_obj(lv_obj_t * obj, const lv_area_t * mask_ori_p)
{
if(obj->hidden != 0) return;
bool union_ok; 
lv_area_t obj_mask;
lv_area_t obj_ext_mask;
lv_area_t obj_area;
lv_coord_t ext_size = obj->ext_draw_pad;
lv_obj_get_coords(obj, &obj_area);
obj_area.x1 -= ext_size;
obj_area.y1 -= ext_size;
obj_area.x2 += ext_size;
obj_area.y2 += ext_size;
union_ok = lv_area_intersect(&obj_ext_mask, mask_ori_p, &obj_area);
if(union_ok != false) {
obj->design_cb(obj, &obj_ext_mask, LV_DESIGN_DRAW_MAIN);
#if MASK_AREA_DEBUG
static lv_color_t debug_color = LV_COLOR_RED;
lv_draw_fill(&obj_ext_mask, &obj_ext_mask, debug_color, LV_OPA_50);
debug_color.full *= 17;
debug_color.full += 0xA1;
#endif
lv_obj_get_coords(obj, &obj_area);
union_ok = lv_area_intersect(&obj_mask, mask_ori_p, &obj_area);
if(union_ok != false) {
lv_area_t mask_child; 
lv_obj_t * child_p;
lv_area_t child_area;
LV_LL_READ_BACK(obj->child_ll, child_p)
{
lv_obj_get_coords(child_p, &child_area);
ext_size = child_p->ext_draw_pad;
child_area.x1 -= ext_size;
child_area.y1 -= ext_size;
child_area.x2 += ext_size;
child_area.y2 += ext_size;
union_ok = lv_area_intersect(&mask_child, &obj_mask, &child_area);
if(union_ok) {
lv_refr_obj(child_p, &mask_child);
}
}
}
obj->design_cb(obj, &obj_ext_mask, LV_DESIGN_DRAW_POST);
}
}
static void lv_refr_vdb_flush(void)
{
lv_disp_buf_t * vdb = lv_disp_get_buf(disp_refr);
if(lv_disp_is_double_buf(disp_refr)) {
while(vdb->flushing)
;
}
vdb->flushing = 1;
lv_disp_t * disp = lv_refr_get_disp_refreshing();
if(disp->driver.flush_cb) disp->driver.flush_cb(&disp->driver, &vdb->area, vdb->buf_act);
if(vdb->buf1 && vdb->buf2) {
if(vdb->buf_act == vdb->buf1)
vdb->buf_act = vdb->buf2;
else
vdb->buf_act = vdb->buf1;
}
}
