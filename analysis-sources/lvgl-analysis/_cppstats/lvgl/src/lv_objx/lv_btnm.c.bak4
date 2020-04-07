







#include "lv_btnm.h"
#if LV_USE_BTNM != 0

#include "../lv_core/lv_debug.h"
#include "../lv_core/lv_group.h"
#include "../lv_draw/lv_draw.h"
#include "../lv_core/lv_refr.h"
#include "../lv_themes/lv_theme.h"
#include "../lv_misc/lv_txt.h"




#define LV_OBJX_NAME "lv_btnm"








static lv_res_t lv_btnm_signal(lv_obj_t * btnm, lv_signal_t sign, void * param);
static bool lv_btnm_design(lv_obj_t * btnm, const lv_area_t * mask, lv_design_mode_t mode);
static uint8_t get_button_width(lv_btnm_ctrl_t ctrl_bits);
static bool button_is_hidden(lv_btnm_ctrl_t ctrl_bits);
static bool button_is_repeat_disabled(lv_btnm_ctrl_t ctrl_bits);
static bool button_is_inactive(lv_btnm_ctrl_t ctrl_bits);
static bool button_is_click_trig(lv_btnm_ctrl_t ctrl_bits);
static bool button_is_tgl_enabled(lv_btnm_ctrl_t ctrl_bits);
static bool button_get_tgl_state(lv_btnm_ctrl_t ctrl_bits);
static uint16_t get_button_from_point(lv_obj_t * btnm, lv_point_t * p);
static void allocate_btn_areas_and_controls(const lv_obj_t * btnm, const char ** map);
static void invalidate_button_area(const lv_obj_t * btnm, uint16_t btn_idx);
static bool maps_are_identical(const char ** map1, const char ** map2);
static void make_one_button_toggled(lv_obj_t * btnm, uint16_t btn_idx);




static const char * lv_btnm_def_map[] = {"Btn1", "Btn2", "Btn3", "\n", "Btn4", "Btn5", ""};

static lv_design_cb_t ancestor_design_f;
static lv_signal_cb_t ancestor_signal;
















lv_obj_t * lv_btnm_create(lv_obj_t * par, const lv_obj_t * copy)
{
LV_LOG_TRACE("button matrix create started");


lv_obj_t * new_btnm = lv_obj_create(par, copy);
LV_ASSERT_MEM(new_btnm);
if(new_btnm == NULL) return NULL;

if(ancestor_signal == NULL) ancestor_signal = lv_obj_get_signal_cb(new_btnm);


lv_btnm_ext_t * ext = lv_obj_allocate_ext_attr(new_btnm, sizeof(lv_btnm_ext_t));
LV_ASSERT_MEM(ext);
if(ext == NULL) return NULL;

ext->btn_cnt = 0;
ext->btn_id_pr = LV_BTNM_BTN_NONE;
ext->btn_id_act = LV_BTNM_BTN_NONE;
ext->button_areas = NULL;
ext->ctrl_bits = NULL;
ext->map_p = NULL;
ext->recolor = 0;
ext->one_toggle = 0;
ext->styles_btn[LV_BTN_STATE_REL] = &lv_style_btn_rel;
ext->styles_btn[LV_BTN_STATE_PR] = &lv_style_btn_pr;
ext->styles_btn[LV_BTN_STATE_TGL_REL] = &lv_style_btn_tgl_rel;
ext->styles_btn[LV_BTN_STATE_TGL_PR] = &lv_style_btn_tgl_pr;
ext->styles_btn[LV_BTN_STATE_INA] = &lv_style_btn_ina;

if(ancestor_design_f == NULL) ancestor_design_f = lv_obj_get_design_cb(new_btnm);

lv_obj_set_signal_cb(new_btnm, lv_btnm_signal);
lv_obj_set_design_cb(new_btnm, lv_btnm_design);


if(copy == NULL) {
lv_btnm_set_map(new_btnm, lv_btnm_def_map);
lv_obj_set_size(new_btnm, LV_DPI * 3, LV_DPI * 2);


lv_theme_t * th = lv_theme_get_current();
if(th) {
lv_btnm_set_style(new_btnm, LV_BTNM_STYLE_BG, th->style.btnm.bg);
lv_btnm_set_style(new_btnm, LV_BTNM_STYLE_BTN_REL, th->style.btnm.btn.rel);
lv_btnm_set_style(new_btnm, LV_BTNM_STYLE_BTN_PR, th->style.btnm.btn.pr);
lv_btnm_set_style(new_btnm, LV_BTNM_STYLE_BTN_TGL_REL, th->style.btnm.btn.tgl_rel);
lv_btnm_set_style(new_btnm, LV_BTNM_STYLE_BTN_TGL_PR, th->style.btnm.btn.tgl_pr);
lv_btnm_set_style(new_btnm, LV_BTNM_STYLE_BTN_INA, th->style.btnm.btn.ina);
} else {
lv_obj_set_style(new_btnm, &lv_style_pretty);
}
}

else {
lv_btnm_ext_t * copy_ext = lv_obj_get_ext_attr(copy);
memcpy((void*)ext->styles_btn, copy_ext->styles_btn, sizeof(ext->styles_btn));
lv_btnm_set_map(new_btnm, lv_btnm_get_map_array(copy));
}

LV_LOG_INFO("button matrix created");

return new_btnm;
}












void lv_btnm_set_map(const lv_obj_t * btnm, const char * map[])
{
LV_ASSERT_OBJ(btnm, LV_OBJX_NAME);
LV_ASSERT_NULL(map);












lv_btnm_ext_t * ext = lv_obj_get_ext_attr(btnm);
if(!maps_are_identical(ext->map_p, map)) {


allocate_btn_areas_and_controls(btnm, map);
}
ext->map_p = map;


const lv_style_t * style_bg = lv_btnm_get_style(btnm, LV_BTNM_STYLE_BG);
lv_coord_t max_w = lv_obj_get_width(btnm) - style_bg->body.padding.left - style_bg->body.padding.right;
lv_coord_t max_h = lv_obj_get_height(btnm) - style_bg->body.padding.top - style_bg->body.padding.bottom;
lv_coord_t act_y = style_bg->body.padding.top;


uint8_t line_cnt = 1;
uint8_t li;
for(li = 0; strlen(map[li]) != 0; li++) {
if(strcmp(map[li], "\n") == 0) line_cnt++;
}

lv_coord_t btn_h = max_h - ((line_cnt - 1) * style_bg->body.padding.inner);
btn_h = btn_h / line_cnt;
btn_h--; 



uint16_t unit_cnt; 
uint16_t unit_act_cnt; 
uint16_t btn_cnt; 
uint16_t i_tot = 0; 
uint16_t btn_i = 0; 
const char ** map_p_tmp = map;


while(1) {
unit_cnt = 0;
btn_cnt = 0;

while(strcmp(map_p_tmp[btn_cnt], "\n") != 0 && strlen(map_p_tmp[btn_cnt]) != 0) { 
unit_cnt += get_button_width(ext->ctrl_bits[btn_i + btn_cnt]);
btn_cnt++;
}


if(map_p_tmp[btn_cnt][0] == '\0') { 
btn_h = lv_obj_get_height(btnm)- act_y - style_bg->body.padding.bottom - 1;
}

lv_bidi_dir_t base_dir = lv_obj_get_base_dir(btnm);


if(btn_cnt != 0) {

lv_coord_t all_unit_w = max_w - ((btn_cnt - 1) * style_bg->body.padding.inner);


uint16_t i;
lv_coord_t act_x;

lv_coord_t act_unit_w;
unit_act_cnt = 0;
for(i = 0; i < btn_cnt; i++) {



act_unit_w = (all_unit_w * get_button_width(ext->ctrl_bits[btn_i])) / unit_cnt;
act_unit_w--; 


if(base_dir == LV_BIDI_DIR_RTL) {
act_x = (unit_act_cnt * all_unit_w) / unit_cnt + i * style_bg->body.padding.inner;
act_x = lv_obj_get_width(btnm) - style_bg->body.padding.right - act_x - act_unit_w - 1;
} else {
act_x = (unit_act_cnt * all_unit_w) / unit_cnt + i * style_bg->body.padding.inner +
style_bg->body.padding.left;
}



if(style_bg->body.padding.inner == 0 && act_x != style_bg->body.padding.left) {
lv_area_set(&ext->button_areas[btn_i], ext->button_areas[btn_i - 1].x2, act_y, act_x + act_unit_w,
act_y + btn_h);
} else {
lv_area_set(&ext->button_areas[btn_i], act_x, act_y, act_x + act_unit_w, act_y + btn_h);
}

unit_act_cnt += get_button_width(ext->ctrl_bits[btn_i]);

i_tot++;
btn_i++;
}
}
act_y += btn_h + style_bg->body.padding.inner + 1;

if(strlen(map_p_tmp[btn_cnt]) == 0) break; 
map_p_tmp = &map_p_tmp[btn_cnt + 1]; 
i_tot++; 
}

lv_obj_invalidate(btnm);
}













void lv_btnm_set_ctrl_map(const lv_obj_t * btnm, const lv_btnm_ctrl_t ctrl_map[])
{
LV_ASSERT_OBJ(btnm, LV_OBJX_NAME);

lv_btnm_ext_t * ext = lv_obj_get_ext_attr(btnm);
memcpy(ext->ctrl_bits, ctrl_map, sizeof(lv_btnm_ctrl_t) * ext->btn_cnt);

lv_btnm_set_map(btnm, ext->map_p);
}







void lv_btnm_set_pressed(const lv_obj_t * btnm, uint16_t id)
{
LV_ASSERT_OBJ(btnm, LV_OBJX_NAME);

lv_btnm_ext_t * ext = lv_obj_get_ext_attr(btnm);

if(id >= ext->btn_cnt && id != LV_BTNM_BTN_NONE) return;

if(id == ext->btn_id_pr) return;

ext->btn_id_pr = id;
lv_obj_invalidate(btnm);
}







void lv_btnm_set_style(lv_obj_t * btnm, lv_btnm_style_t type, const lv_style_t * style)
{
LV_ASSERT_OBJ(btnm, LV_OBJX_NAME);

lv_btnm_ext_t * ext = lv_obj_get_ext_attr(btnm);

switch(type) {
case LV_BTNM_STYLE_BG: lv_obj_set_style(btnm, style); break;
case LV_BTNM_STYLE_BTN_REL:
ext->styles_btn[LV_BTN_STATE_REL] = style;
lv_obj_invalidate(btnm);
break;
case LV_BTNM_STYLE_BTN_PR:
ext->styles_btn[LV_BTN_STATE_PR] = style;
lv_obj_invalidate(btnm);
break;
case LV_BTNM_STYLE_BTN_TGL_REL:
ext->styles_btn[LV_BTN_STATE_TGL_REL] = style;
lv_obj_invalidate(btnm);
break;
case LV_BTNM_STYLE_BTN_TGL_PR:
ext->styles_btn[LV_BTN_STATE_TGL_PR] = style;
lv_obj_invalidate(btnm);
break;
case LV_BTNM_STYLE_BTN_INA:
ext->styles_btn[LV_BTN_STATE_INA] = style;
lv_obj_invalidate(btnm);
break;
}
}






void lv_btnm_set_recolor(const lv_obj_t * btnm, bool en)
{
LV_ASSERT_OBJ(btnm, LV_OBJX_NAME);

lv_btnm_ext_t * ext = lv_obj_get_ext_attr(btnm);

ext->recolor = en;
lv_obj_invalidate(btnm);
}






void lv_btnm_set_btn_ctrl(const lv_obj_t * btnm, uint16_t btn_id, lv_btnm_ctrl_t ctrl)
{
LV_ASSERT_OBJ(btnm, LV_OBJX_NAME);

lv_btnm_ext_t * ext = lv_obj_get_ext_attr(btnm);

if(btn_id >= ext->btn_cnt) return;

ext->ctrl_bits[btn_id] |= ctrl;
invalidate_button_area(btnm, btn_id);
}






void lv_btnm_clear_btn_ctrl(const lv_obj_t * btnm, uint16_t btn_id, lv_btnm_ctrl_t ctrl)
{
LV_ASSERT_OBJ(btnm, LV_OBJX_NAME);

lv_btnm_ext_t * ext = lv_obj_get_ext_attr(btnm);

if(btn_id >= ext->btn_cnt) return;

ext->ctrl_bits[btn_id] &= (~ctrl);
invalidate_button_area(btnm, btn_id);
}






void lv_btnm_set_btn_ctrl_all(lv_obj_t * btnm, lv_btnm_ctrl_t ctrl)
{
LV_ASSERT_OBJ(btnm, LV_OBJX_NAME);

lv_btnm_ext_t * ext = lv_obj_get_ext_attr(btnm);
uint16_t i;
for(i = 0; i < ext->btn_cnt; i++) {
lv_btnm_set_btn_ctrl(btnm, i, ctrl);
}
}







void lv_btnm_clear_btn_ctrl_all(lv_obj_t * btnm, lv_btnm_ctrl_t ctrl)
{
LV_ASSERT_OBJ(btnm, LV_OBJX_NAME);

lv_btnm_ext_t * ext = lv_obj_get_ext_attr(btnm);
uint16_t i;
for(i = 0; i < ext->btn_cnt; i++) {
lv_btnm_clear_btn_ctrl(btnm, i, ctrl);
}
}










void lv_btnm_set_btn_width(const lv_obj_t * btnm, uint16_t btn_id, uint8_t width)
{
LV_ASSERT_OBJ(btnm, LV_OBJX_NAME);


lv_btnm_ext_t * ext = lv_obj_get_ext_attr(btnm);
if(btn_id >= ext->btn_cnt) return;
ext->ctrl_bits[btn_id] &= (~LV_BTNM_WIDTH_MASK);
ext->ctrl_bits[btn_id] |= (LV_BTNM_WIDTH_MASK & width);

lv_btnm_set_map(btnm, ext->map_p);
}










void lv_btnm_set_one_toggle(lv_obj_t * btnm, bool one_toggle)
{
LV_ASSERT_OBJ(btnm, LV_OBJX_NAME);

lv_btnm_ext_t * ext = lv_obj_get_ext_attr(btnm);
ext->one_toggle = one_toggle;


make_one_button_toggled(btnm, 0);
}










const char ** lv_btnm_get_map_array(const lv_obj_t * btnm)
{
LV_ASSERT_OBJ(btnm, LV_OBJX_NAME);

lv_btnm_ext_t * ext = lv_obj_get_ext_attr(btnm);
return ext->map_p;
}






bool lv_btnm_get_recolor(const lv_obj_t * btnm)
{
LV_ASSERT_OBJ(btnm, LV_OBJX_NAME);

lv_btnm_ext_t * ext = lv_obj_get_ext_attr(btnm);

return ext->recolor;
}







uint16_t lv_btnm_get_active_btn(const lv_obj_t * btnm)
{
LV_ASSERT_OBJ(btnm, LV_OBJX_NAME);

lv_btnm_ext_t * ext = lv_obj_get_ext_attr(btnm);
return ext->btn_id_act;
}







const char * lv_btnm_get_active_btn_text(const lv_obj_t * btnm)
{
LV_ASSERT_OBJ(btnm, LV_OBJX_NAME);

lv_btnm_ext_t * ext = lv_obj_get_ext_attr(btnm);
if(ext->btn_id_act != LV_BTNM_BTN_NONE) {
return lv_btnm_get_btn_text(btnm, ext->btn_id_act);
} else {
return NULL;
}
}







uint16_t lv_btnm_get_pressed_btn(const lv_obj_t * btnm)
{
LV_ASSERT_OBJ(btnm, LV_OBJX_NAME);

lv_btnm_ext_t * ext = lv_obj_get_ext_attr(btnm);
return ext->btn_id_pr;
}








const char * lv_btnm_get_btn_text(const lv_obj_t * btnm, uint16_t btn_id)
{
LV_ASSERT_OBJ(btnm, LV_OBJX_NAME);

lv_btnm_ext_t * ext = lv_obj_get_ext_attr(btnm);
if(btn_id > ext->btn_cnt) return NULL;

uint16_t txt_i = 0;
uint16_t btn_i = 0;



while(btn_i != btn_id) {
btn_i++;
txt_i++;
if(strcmp(ext->map_p[txt_i], "\n") == 0) txt_i++;
}

if(btn_i == ext->btn_cnt) return NULL;

return ext->map_p[txt_i];
}









bool lv_btnm_get_btn_ctrl(lv_obj_t * btnm, uint16_t btn_id, lv_btnm_ctrl_t ctrl)
{
LV_ASSERT_OBJ(btnm, LV_OBJX_NAME);

lv_btnm_ext_t * ext = lv_obj_get_ext_attr(btnm);
if(btn_id >= ext->btn_cnt) return false;

return ext->ctrl_bits[btn_id] & ctrl ? true : false;
}







const lv_style_t * lv_btnm_get_style(const lv_obj_t * btnm, lv_btnm_style_t type)
{
LV_ASSERT_OBJ(btnm, LV_OBJX_NAME);

const lv_style_t * style = NULL;
lv_btnm_ext_t * ext = lv_obj_get_ext_attr(btnm);

switch(type) {
case LV_BTNM_STYLE_BG: style = lv_obj_get_style(btnm); break;
case LV_BTNM_STYLE_BTN_REL: style = ext->styles_btn[LV_BTN_STATE_REL]; break;
case LV_BTNM_STYLE_BTN_PR: style = ext->styles_btn[LV_BTN_STATE_PR]; break;
case LV_BTNM_STYLE_BTN_TGL_REL: style = ext->styles_btn[LV_BTN_STATE_TGL_REL]; break;
case LV_BTNM_STYLE_BTN_TGL_PR: style = ext->styles_btn[LV_BTN_STATE_TGL_PR]; break;
case LV_BTNM_STYLE_BTN_INA: style = ext->styles_btn[LV_BTN_STATE_INA]; break;
default: style = NULL; break;
}

return style;
}






bool lv_btnm_get_one_toggle(const lv_obj_t * btnm)
{
LV_ASSERT_OBJ(btnm, LV_OBJX_NAME);

lv_btnm_ext_t * ext = lv_obj_get_ext_attr(btnm);

return ext->one_toggle;
}















static bool lv_btnm_design(lv_obj_t * btnm, const lv_area_t * mask, lv_design_mode_t mode)
{
if(mode == LV_DESIGN_COVER_CHK) {
return ancestor_design_f(btnm, mask, mode);

}

else if(mode == LV_DESIGN_DRAW_MAIN) {
ancestor_design_f(btnm, mask, mode);

lv_btnm_ext_t * ext = lv_obj_get_ext_attr(btnm);
const lv_style_t * bg_style = lv_obj_get_style(btnm);
const lv_style_t * btn_style;
lv_opa_t opa_scale = lv_obj_get_opa_scale(btnm);

lv_area_t area_btnm;
lv_obj_get_coords(btnm, &area_btnm);

lv_area_t area_tmp;
lv_coord_t btn_w;
lv_coord_t btn_h;

uint16_t btn_i = 0;
uint16_t txt_i = 0;
lv_style_t style_tmp;
lv_txt_flag_t txt_flag = LV_TXT_FLAG_NONE;

if(ext->recolor) txt_flag = LV_TXT_FLAG_RECOLOR;
for(btn_i = 0; btn_i < ext->btn_cnt; btn_i++, txt_i++) {

while(strcmp(ext->map_p[txt_i], "\n") == 0) {
txt_i++;
}


if(button_is_hidden(ext->ctrl_bits[btn_i])) continue;

lv_area_copy(&area_tmp, &ext->button_areas[btn_i]);
area_tmp.x1 += area_btnm.x1;
area_tmp.y1 += area_btnm.y1;
area_tmp.x2 += area_btnm.x1;
area_tmp.y2 += area_btnm.y1;

btn_w = lv_area_get_width(&area_tmp);
btn_h = lv_area_get_height(&area_tmp);


bool tgl_state = button_get_tgl_state(ext->ctrl_bits[btn_i]);
if(button_is_inactive(ext->ctrl_bits[btn_i]))
btn_style = lv_btnm_get_style(btnm, LV_BTNM_STYLE_BTN_INA);
else if(btn_i != ext->btn_id_pr && tgl_state == false)
btn_style = lv_btnm_get_style(btnm, LV_BTNM_STYLE_BTN_REL);
else if(btn_i == ext->btn_id_pr && tgl_state == false)
btn_style = lv_btnm_get_style(btnm, LV_BTNM_STYLE_BTN_PR);
else if(btn_i != ext->btn_id_pr && tgl_state == true)
btn_style = lv_btnm_get_style(btnm, LV_BTNM_STYLE_BTN_TGL_REL);
else if(btn_i == ext->btn_id_pr && tgl_state == true)
btn_style = lv_btnm_get_style(btnm, LV_BTNM_STYLE_BTN_TGL_PR);
else
btn_style = lv_btnm_get_style(btnm, LV_BTNM_STYLE_BTN_REL); 

lv_style_copy(&style_tmp, btn_style);


if(style_tmp.body.border.part & LV_BORDER_INTERNAL) {
if(area_tmp.y1 == btnm->coords.y1 + bg_style->body.padding.top) {
style_tmp.body.border.part &= ~LV_BORDER_TOP;
}
if(area_tmp.y2 == btnm->coords.y2 - bg_style->body.padding.bottom) {
style_tmp.body.border.part &= ~LV_BORDER_BOTTOM;
}

if(txt_i == 0) {
style_tmp.body.border.part &= ~LV_BORDER_LEFT;
} else if(strcmp(ext->map_p[txt_i - 1], "\n") == 0) {
style_tmp.body.border.part &= ~LV_BORDER_LEFT;
}

if(ext->map_p[txt_i + 1][0] == '\0' || strcmp(ext->map_p[txt_i + 1], "\n") == 0) {
style_tmp.body.border.part &= ~LV_BORDER_RIGHT;
}
}
lv_draw_rect(&area_tmp, mask, &style_tmp, opa_scale);


if(btn_style->glass) btn_style = bg_style;
const lv_font_t * font = btn_style->text.font;
lv_point_t txt_size;
lv_txt_get_size(&txt_size, ext->map_p[txt_i], font, btn_style->text.letter_space,
btn_style->text.line_space, lv_area_get_width(&area_btnm), txt_flag);

area_tmp.x1 += (btn_w - txt_size.x) / 2;
area_tmp.y1 += (btn_h - txt_size.y) / 2;
area_tmp.x2 = area_tmp.x1 + txt_size.x;
area_tmp.y2 = area_tmp.y1 + txt_size.y;

lv_draw_label(&area_tmp, mask, btn_style, opa_scale, ext->map_p[txt_i], txt_flag, NULL, NULL, NULL, lv_obj_get_base_dir(btnm));
}
}

return true;
}








static lv_res_t lv_btnm_signal(lv_obj_t * btnm, lv_signal_t sign, void * param)
{
lv_res_t res;


res = ancestor_signal(btnm, sign, param);
if(res != LV_RES_OK) return res;
if(sign == LV_SIGNAL_GET_TYPE) return lv_obj_handle_get_type_signal(param, LV_OBJX_NAME);

lv_btnm_ext_t * ext = lv_obj_get_ext_attr(btnm);
lv_point_t p;
if(sign == LV_SIGNAL_CLEANUP) {
lv_mem_free(ext->button_areas);
lv_mem_free(ext->ctrl_bits);
} else if(sign == LV_SIGNAL_STYLE_CHG || sign == LV_SIGNAL_CORD_CHG) {
lv_btnm_set_map(btnm, ext->map_p);
} else if(sign == LV_SIGNAL_PRESSED) {
lv_indev_t * indev = lv_indev_get_act();
if(lv_indev_get_type(indev) == LV_INDEV_TYPE_POINTER || lv_indev_get_type(indev) == LV_INDEV_TYPE_BUTTON) {
uint16_t btn_pr;

lv_indev_get_point(param, &p);
btn_pr = get_button_from_point(btnm, &p);

invalidate_button_area(btnm, ext->btn_id_pr) ;
ext->btn_id_pr = btn_pr;
ext->btn_id_act = btn_pr;
invalidate_button_area(btnm, ext->btn_id_pr); 
}
if(ext->btn_id_act != LV_BTNM_BTN_NONE) {
if(button_is_click_trig(ext->ctrl_bits[ext->btn_id_act]) == false &&
button_is_inactive(ext->ctrl_bits[ext->btn_id_act]) == false &&
button_is_hidden(ext->ctrl_bits[ext->btn_id_act]) == false) {
uint32_t b = ext->btn_id_act;
res = lv_event_send(btnm, LV_EVENT_VALUE_CHANGED, &b);
}
}
} else if(sign == LV_SIGNAL_PRESSING) {
uint16_t btn_pr;

lv_indev_get_point(param, &p);
btn_pr = get_button_from_point(btnm, &p);
;
if(btn_pr != ext->btn_id_pr) {
lv_indev_reset_long_press(param); 
if(ext->btn_id_pr != LV_BTNM_BTN_NONE) {
invalidate_button_area(btnm, ext->btn_id_pr);
}
if(btn_pr != LV_BTNM_BTN_NONE) {
uint32_t b = ext->btn_id_act;
res = lv_event_send(btnm, LV_EVENT_VALUE_CHANGED, &b);
if(res == LV_RES_OK) {
invalidate_button_area(btnm, btn_pr);
}
}
}

ext->btn_id_pr = btn_pr;
ext->btn_id_act = btn_pr;
} else if(sign == LV_SIGNAL_RELEASED) {
if(ext->btn_id_pr != LV_BTNM_BTN_NONE) {

if(button_is_tgl_enabled(ext->ctrl_bits[ext->btn_id_pr]) &&
!button_is_inactive(ext->ctrl_bits[ext->btn_id_pr])) {
if(button_get_tgl_state(ext->ctrl_bits[ext->btn_id_pr])) {
ext->ctrl_bits[ext->btn_id_pr] &= (~LV_BTNM_CTRL_TGL_STATE);
} else {
ext->ctrl_bits[ext->btn_id_pr] |= LV_BTNM_CTRL_TGL_STATE;
}
if(ext->one_toggle) make_one_button_toggled(btnm, ext->btn_id_pr);
}

;
invalidate_button_area(btnm, ext->btn_id_pr);

#if LV_USE_GROUP

lv_group_t * g = lv_obj_get_group(btnm);
if(lv_group_get_focused(g) != btnm) {
ext->btn_id_pr = LV_BTNM_BTN_NONE;
}
#else
ext->btn_id_pr = LV_BTNM_BTN_NONE;
#endif

if(button_is_click_trig(ext->ctrl_bits[ext->btn_id_act]) == true &&
button_is_inactive(ext->ctrl_bits[ext->btn_id_act]) == false &&
button_is_hidden(ext->ctrl_bits[ext->btn_id_act]) == false) {
uint32_t b = ext->btn_id_act;
res = lv_event_send(btnm, LV_EVENT_VALUE_CHANGED, &b);
}
}
} else if(sign == LV_SIGNAL_LONG_PRESS_REP) {
if(ext->btn_id_act != LV_BTNM_BTN_NONE) {
if(button_is_repeat_disabled(ext->ctrl_bits[ext->btn_id_act]) == false &&
button_is_inactive(ext->ctrl_bits[ext->btn_id_act]) == false &&
button_is_hidden(ext->ctrl_bits[ext->btn_id_act]) == false) {
uint32_t b = ext->btn_id_act;
res = lv_event_send(btnm, LV_EVENT_VALUE_CHANGED, &b);
}
}
} else if(sign == LV_SIGNAL_PRESS_LOST || sign == LV_SIGNAL_DEFOCUS) {
ext->btn_id_pr = LV_BTNM_BTN_NONE;
ext->btn_id_act = LV_BTNM_BTN_NONE;
lv_obj_invalidate(btnm);
} else if(sign == LV_SIGNAL_FOCUS) {
#if LV_USE_GROUP
lv_indev_t * indev = lv_indev_get_act();
lv_indev_type_t indev_type = lv_indev_get_type(indev);


if(indev == NULL) {
indev = lv_indev_get_next(NULL);
indev_type = lv_indev_get_type(indev);
}

if(indev_type == LV_INDEV_TYPE_POINTER) {

lv_point_t p1;
lv_indev_get_point(indev, &p1);
uint16_t btn_i = get_button_from_point(btnm, &p1);
ext->btn_id_pr = btn_i;

} else if(indev_type == LV_INDEV_TYPE_ENCODER) {

if(lv_group_get_editing(lv_obj_get_group(btnm)))
ext->btn_id_pr = 0;
else
ext->btn_id_pr = LV_BTNM_BTN_NONE;
} else {
ext->btn_id_pr = 0;
}
#else
ext->btn_id_pr = 0;
#endif

ext->btn_id_act = ext->btn_id_pr;
lv_obj_invalidate(btnm);
} else if(sign == LV_SIGNAL_CONTROL) {
char c = *((char *)param);
if(c == LV_KEY_RIGHT) {
if(ext->btn_id_pr == LV_BTNM_BTN_NONE)
ext->btn_id_pr = 0;
else
ext->btn_id_pr++;
if(ext->btn_id_pr >= ext->btn_cnt - 1) ext->btn_id_pr = ext->btn_cnt - 1;
ext->btn_id_act = ext->btn_id_pr;
lv_obj_invalidate(btnm);
} else if(c == LV_KEY_LEFT) {
if(ext->btn_id_pr == LV_BTNM_BTN_NONE) ext->btn_id_pr = 0;
if(ext->btn_id_pr > 0) ext->btn_id_pr--;
ext->btn_id_act = ext->btn_id_pr;
lv_obj_invalidate(btnm);
} else if(c == LV_KEY_DOWN) {
const lv_style_t * style = lv_btnm_get_style(btnm, LV_BTNM_STYLE_BG);

if(ext->btn_id_pr == LV_BTNM_BTN_NONE) {
ext->btn_id_pr = 0;
} else {
uint16_t area_below;
lv_coord_t pr_center =
ext->button_areas[ext->btn_id_pr].x1 + (lv_area_get_width(&ext->button_areas[ext->btn_id_pr]) >> 1);

for(area_below = ext->btn_id_pr; area_below < ext->btn_cnt; area_below++) {
if(ext->button_areas[area_below].y1 > ext->button_areas[ext->btn_id_pr].y1 &&
pr_center >= ext->button_areas[area_below].x1 &&
pr_center <= ext->button_areas[area_below].x2 + style->body.padding.inner) {
break;
}
}

if(area_below < ext->btn_cnt) ext->btn_id_pr = area_below;
}
ext->btn_id_act = ext->btn_id_pr;
lv_obj_invalidate(btnm);
} else if(c == LV_KEY_UP) {
const lv_style_t * style = lv_btnm_get_style(btnm, LV_BTNM_STYLE_BG);

if(ext->btn_id_pr == LV_BTNM_BTN_NONE) {
ext->btn_id_pr = 0;
} else {
int16_t area_above;
lv_coord_t pr_center =
ext->button_areas[ext->btn_id_pr].x1 + (lv_area_get_width(&ext->button_areas[ext->btn_id_pr]) >> 1);

for(area_above = ext->btn_id_pr; area_above >= 0; area_above--) {
if(ext->button_areas[area_above].y1 < ext->button_areas[ext->btn_id_pr].y1 &&
pr_center >= ext->button_areas[area_above].x1 - style->body.padding.inner &&
pr_center <= ext->button_areas[area_above].x2) {
break;
}
}
if(area_above >= 0) ext->btn_id_pr = area_above;
}
ext->btn_id_act = ext->btn_id_pr;
lv_obj_invalidate(btnm);
}
} else if(sign == LV_SIGNAL_GET_EDITABLE) {
bool * editable = (bool *)param;
*editable = true;
}
return res;
}






static void allocate_btn_areas_and_controls(const lv_obj_t * btnm, const char ** map)
{

uint16_t btn_cnt = 0;
uint16_t i = 0;
while(strlen(map[i]) != 0) {
if(strcmp(map[i], "\n") != 0) { 
btn_cnt++;
}
i++;
}

lv_btnm_ext_t * ext = lv_obj_get_ext_attr(btnm);

if(ext->button_areas != NULL) {
lv_mem_free(ext->button_areas);
ext->button_areas = NULL;
}
if(ext->ctrl_bits != NULL) {
lv_mem_free(ext->ctrl_bits);
ext->ctrl_bits = NULL;
}

ext->button_areas = lv_mem_alloc(sizeof(lv_area_t) * btn_cnt);
LV_ASSERT_MEM(ext->button_areas);
ext->ctrl_bits = lv_mem_alloc(sizeof(lv_btnm_ctrl_t) * btn_cnt);
LV_ASSERT_MEM(ext->ctrl_bits);
if(ext->button_areas == NULL || ext->ctrl_bits == NULL) btn_cnt = 0;

memset(ext->ctrl_bits, 0, sizeof(lv_btnm_ctrl_t) * btn_cnt);

ext->btn_cnt = btn_cnt;
}






static uint8_t get_button_width(lv_btnm_ctrl_t ctrl_bits)
{
uint8_t w = ctrl_bits & LV_BTNM_WIDTH_MASK;
return w != 0 ? w : 1;
}

static bool button_is_hidden(lv_btnm_ctrl_t ctrl_bits)
{
return ctrl_bits & LV_BTNM_CTRL_HIDDEN ? true : false;
}

static bool button_is_repeat_disabled(lv_btnm_ctrl_t ctrl_bits)
{
return ctrl_bits & LV_BTNM_CTRL_NO_REPEAT ? true : false;
}

static bool button_is_inactive(lv_btnm_ctrl_t ctrl_bits)
{
return ctrl_bits & LV_BTNM_CTRL_INACTIVE ? true : false;
}

static bool button_is_click_trig(lv_btnm_ctrl_t ctrl_bits)
{
return ctrl_bits & LV_BTNM_CTRL_CLICK_TRIG ? true : false;
}

static bool button_is_tgl_enabled(lv_btnm_ctrl_t ctrl_bits)
{
return ctrl_bits & LV_BTNM_CTRL_TGL_ENABLE ? true : false;
}

static bool button_get_tgl_state(lv_btnm_ctrl_t ctrl_bits)
{
return ctrl_bits & LV_BTNM_CTRL_TGL_STATE ? true : false;
}







static uint16_t get_button_from_point(lv_obj_t * btnm, lv_point_t * p)
{
lv_area_t btnm_cords;
lv_area_t btn_area;
lv_btnm_ext_t * ext = lv_obj_get_ext_attr(btnm);
uint16_t i;
lv_obj_get_coords(btnm, &btnm_cords);

for(i = 0; i < ext->btn_cnt; i++) {
lv_area_copy(&btn_area, &ext->button_areas[i]);
btn_area.x1 += btnm_cords.x1;
btn_area.y1 += btnm_cords.y1;
btn_area.x2 += btnm_cords.x1;
btn_area.y2 += btnm_cords.y1;
if(lv_area_is_point_on(&btn_area, p) != false) {
break;
}
}

if(i == ext->btn_cnt) i = LV_BTNM_BTN_NONE;

return i;
}

static void invalidate_button_area(const lv_obj_t * btnm, uint16_t btn_idx)
{
if(btn_idx == LV_BTNM_BTN_NONE) return;

lv_area_t btn_area;
lv_area_t btnm_area;

lv_btnm_ext_t * ext = lv_obj_get_ext_attr(btnm);
lv_area_copy(&btn_area, &ext->button_areas[btn_idx]);
lv_obj_get_coords(btnm, &btnm_area);


btn_area.x1 += btnm_area.x1;
btn_area.y1 += btnm_area.y1;
btn_area.x2 += btnm_area.x1;
btn_area.y2 += btnm_area.y1;

lv_obj_invalidate_area(btnm, &btn_area);
}







static bool maps_are_identical(const char ** map1, const char ** map2)
{
if(map1 == map2) return true;
if(map1 == NULL || map2 == NULL) return map1 == map2;

uint16_t i = 0;
while(map1[i][0] != '\0' && map2[i][0] != '\0') {
if(strcmp(map1[i], map2[i]) != 0) return false;
i++;
}
return map1[i][0] == '\0' && map2[i][0] == '\0';
}







static void make_one_button_toggled(lv_obj_t * btnm, uint16_t btn_idx)
{

bool was_toggled = lv_btnm_get_btn_ctrl(btnm, btn_idx, LV_BTNM_CTRL_TGL_STATE);

lv_btnm_clear_btn_ctrl_all(btnm, LV_BTNM_CTRL_TGL_STATE);

if(was_toggled) lv_btnm_set_btn_ctrl(btnm, btn_idx, LV_BTNM_CTRL_TGL_STATE);
}

#endif
