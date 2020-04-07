#include "lv_list.h"
#if LV_USE_LIST != 0
#include "../lv_core/lv_debug.h"
#include "../lv_core/lv_group.h"
#include "../lv_themes/lv_theme.h"
#include "../lv_misc/lv_anim.h"
#include "../lv_misc/lv_math.h"
#define LV_OBJX_NAME "lv_list"
#define LV_LIST_LAYOUT_DEF LV_LAYOUT_COL_M
#if LV_USE_ANIMATION == 0
#undef LV_LIST_DEF_ANIM_TIME
#define LV_LIST_DEF_ANIM_TIME 0
#endif
static lv_res_t lv_list_signal(lv_obj_t * list, lv_signal_t sign, void * param);
static lv_res_t lv_list_btn_signal(lv_obj_t * btn, lv_signal_t sign, void * param);
static void lv_list_btn_single_select(lv_obj_t * btn);
static bool lv_list_is_list_btn(lv_obj_t * list_btn);
static bool lv_list_is_list_img(lv_obj_t * list_btn);
static bool lv_list_is_list_label(lv_obj_t * list_btn);
#if LV_USE_IMG
static lv_signal_cb_t img_signal;
#endif
static lv_signal_cb_t label_signal;
static lv_signal_cb_t ancestor_page_signal;
static lv_signal_cb_t ancestor_btn_signal;
lv_obj_t * lv_list_create(lv_obj_t * par, const lv_obj_t * copy)
{
LV_LOG_TRACE("list create started");
lv_obj_t * new_list = lv_page_create(par, copy);
LV_ASSERT_MEM(new_list);
if(new_list == NULL) return NULL;
if(ancestor_page_signal == NULL) ancestor_page_signal = lv_obj_get_signal_cb(new_list);
lv_list_ext_t * ext = lv_obj_allocate_ext_attr(new_list, sizeof(lv_list_ext_t));
LV_ASSERT_MEM(ext);
if(ext == NULL) return NULL;
ext->style_img = NULL;
ext->styles_btn[LV_BTN_STATE_REL] = &lv_style_btn_rel;
ext->styles_btn[LV_BTN_STATE_PR] = &lv_style_btn_pr;
ext->styles_btn[LV_BTN_STATE_TGL_REL] = &lv_style_btn_tgl_rel;
ext->styles_btn[LV_BTN_STATE_TGL_PR] = &lv_style_btn_tgl_pr;
ext->styles_btn[LV_BTN_STATE_INA] = &lv_style_btn_ina;
ext->single_mode = false;
ext->size = 0;
#if LV_USE_GROUP
ext->last_sel = NULL;
ext->selected_btn = NULL;
ext->last_clicked_btn = NULL;
#endif
lv_obj_set_signal_cb(new_list, lv_list_signal);
if(copy == NULL) {
lv_page_set_anim_time(new_list, LV_LIST_DEF_ANIM_TIME);
lv_page_set_scrl_fit2(new_list, LV_FIT_FLOOD, LV_FIT_TIGHT);
lv_obj_set_size(new_list, 2 * LV_DPI, 3 * LV_DPI);
lv_page_set_scrl_layout(new_list, LV_LIST_LAYOUT_DEF);
lv_list_set_sb_mode(new_list, LV_SB_MODE_DRAG);
lv_theme_t * th = lv_theme_get_current();
if(th) {
lv_list_set_style(new_list, LV_LIST_STYLE_BG, th->style.list.bg);
lv_list_set_style(new_list, LV_LIST_STYLE_SCRL, th->style.list.scrl);
lv_list_set_style(new_list, LV_LIST_STYLE_SB, th->style.list.sb);
lv_list_set_style(new_list, LV_LIST_STYLE_BTN_REL, th->style.list.btn.rel);
lv_list_set_style(new_list, LV_LIST_STYLE_BTN_PR, th->style.list.btn.pr);
lv_list_set_style(new_list, LV_LIST_STYLE_BTN_TGL_REL, th->style.list.btn.tgl_rel);
lv_list_set_style(new_list, LV_LIST_STYLE_BTN_TGL_PR, th->style.list.btn.tgl_pr);
lv_list_set_style(new_list, LV_LIST_STYLE_BTN_INA, th->style.list.btn.ina);
} else {
lv_list_set_style(new_list, LV_LIST_STYLE_BG, &lv_style_transp_fit);
lv_list_set_style(new_list, LV_LIST_STYLE_SCRL, &lv_style_pretty);
}
} else {
lv_list_ext_t * copy_ext = lv_obj_get_ext_attr(copy);
lv_obj_t * copy_btn = lv_list_get_next_btn(copy, NULL);
while(copy_btn) {
const void * img_src = NULL;
#if LV_USE_IMG
lv_obj_t * copy_img = lv_list_get_btn_img(copy_btn);
if(copy_img) img_src = lv_img_get_src(copy_img);
#endif
lv_list_add_btn(new_list, img_src, lv_list_get_btn_text(copy_btn));
copy_btn = lv_list_get_next_btn(copy, copy_btn);
}
lv_list_set_style(new_list, LV_LIST_STYLE_BTN_REL, copy_ext->styles_btn[LV_BTN_STATE_REL]);
lv_list_set_style(new_list, LV_LIST_STYLE_BTN_PR, copy_ext->styles_btn[LV_BTN_STATE_PR]);
lv_list_set_style(new_list, LV_LIST_STYLE_BTN_TGL_REL, copy_ext->styles_btn[LV_BTN_STATE_TGL_REL]);
lv_list_set_style(new_list, LV_LIST_STYLE_BTN_TGL_PR, copy_ext->styles_btn[LV_BTN_STATE_TGL_REL]);
lv_list_set_style(new_list, LV_LIST_STYLE_BTN_INA, copy_ext->styles_btn[LV_BTN_STATE_INA]);
lv_obj_refresh_style(new_list);
}
LV_LOG_INFO("list created");
return new_list;
}
void lv_list_clean(lv_obj_t * list)
{
LV_ASSERT_OBJ(list, LV_OBJX_NAME);
lv_obj_t * scrl = lv_page_get_scrl(list);
lv_obj_clean(scrl);
lv_list_ext_t * ext = lv_obj_get_ext_attr(list);
ext->size = 0;
}
lv_obj_t * lv_list_add_btn(lv_obj_t * list, const void * img_src, const char * txt)
{
LV_ASSERT_OBJ(list, LV_OBJX_NAME);
lv_obj_t * last_btn = lv_list_get_prev_btn(list, NULL);
lv_coord_t pos_x_ori = lv_obj_get_x(list);
lv_coord_t pos_y_ori = lv_obj_get_y(list);
lv_list_ext_t * ext = lv_obj_get_ext_attr(list);
ext->size++;
lv_obj_t * liste;
liste = lv_btn_create(list, NULL);
if(ancestor_btn_signal == NULL) ancestor_btn_signal = lv_obj_get_signal_cb(liste);
lv_btn_set_style(liste, LV_BTN_STYLE_REL, ext->styles_btn[LV_BTN_STATE_REL]);
lv_btn_set_style(liste, LV_BTN_STYLE_PR, ext->styles_btn[LV_BTN_STATE_PR]);
lv_btn_set_style(liste, LV_BTN_STYLE_TGL_REL, ext->styles_btn[LV_BTN_STATE_TGL_REL]);
lv_btn_set_style(liste, LV_BTN_STYLE_TGL_PR, ext->styles_btn[LV_BTN_STATE_TGL_PR]);
lv_btn_set_style(liste, LV_BTN_STYLE_INA, ext->styles_btn[LV_BTN_STATE_INA]);
lv_page_glue_obj(liste, true);
lv_btn_set_layout(liste, LV_LAYOUT_ROW_M);
lv_layout_t list_layout = lv_list_get_layout(list);
bool layout_ver = false;
if(list_layout == LV_LAYOUT_COL_M || list_layout == LV_LAYOUT_COL_L || list_layout == LV_LAYOUT_COL_R) {
layout_ver = true;
}
if(layout_ver) {
lv_btn_set_fit2(liste, LV_FIT_FLOOD, LV_FIT_TIGHT);
} else {
lv_coord_t w = last_btn ? lv_obj_get_width(last_btn) : (LV_DPI * 3) / 2;
lv_btn_set_fit2(liste, LV_FIT_NONE, LV_FIT_TIGHT);
lv_obj_set_width(liste, w);
}
lv_obj_set_protect(liste, LV_PROTECT_PRESS_LOST);
lv_obj_set_signal_cb(liste, lv_list_btn_signal);
#if LV_USE_IMG != 0
lv_obj_t * img = NULL;
if(img_src) {
img = lv_img_create(liste, NULL);
lv_img_set_src(img, img_src);
lv_obj_set_style(img, ext->style_img);
lv_obj_set_click(img, false);
if(img_signal == NULL) img_signal = lv_obj_get_signal_cb(img);
}
#endif
if(txt != NULL) {
lv_coord_t btn_hor_pad = ext->styles_btn[LV_BTN_STYLE_REL]->body.padding.left -
ext->styles_btn[LV_BTN_STYLE_REL]->body.padding.right;
lv_obj_t * label = lv_label_create(liste, NULL);
lv_label_set_text(label, txt);
lv_obj_set_click(label, false);
lv_label_set_long_mode(label, LV_LABEL_LONG_SROLL_CIRC);
if(lv_obj_get_base_dir(liste) == LV_BIDI_DIR_RTL) lv_obj_set_width(label, label->coords.x2 - liste->coords.x1 - btn_hor_pad);
else lv_obj_set_width(label, liste->coords.x2 - label->coords.x1 - btn_hor_pad);
if(label_signal == NULL) label_signal = lv_obj_get_signal_cb(label);
}
#if LV_USE_GROUP
{
lv_group_t * g = lv_obj_get_group(list);
if(ext->size == 1 && lv_group_get_focused(g) == list) {
lv_list_set_btn_selected(list, liste);
}
}
#endif
lv_obj_set_pos(list, pos_x_ori, pos_y_ori);
return liste;
}
bool lv_list_remove(const lv_obj_t * list, uint16_t index)
{
LV_ASSERT_OBJ(list, LV_OBJX_NAME);
lv_list_ext_t * ext = lv_obj_get_ext_attr(list);
if(index >= ext->size) return false;
uint16_t count = 0;
lv_obj_t * e = lv_list_get_next_btn(list, NULL);
while(e != NULL) {
if(count == index) {
lv_obj_del(e);
ext->size--;
return true;
}
e = lv_list_get_next_btn(list, e);
count++;
}
return false;
}
void lv_list_set_single_mode(lv_obj_t * list, bool mode)
{
LV_ASSERT_OBJ(list, LV_OBJX_NAME);
lv_list_ext_t * ext = lv_obj_get_ext_attr(list);
ext->single_mode = mode;
}
#if LV_USE_GROUP
void lv_list_set_btn_selected(lv_obj_t * list, lv_obj_t * btn)
{
LV_ASSERT_OBJ(list, LV_OBJX_NAME);
if(btn) LV_ASSERT_OBJ(btn, "lv_btn");
lv_list_ext_t * ext = lv_obj_get_ext_attr(list);
if(ext->selected_btn) {
lv_btn_state_t s = lv_btn_get_state(ext->selected_btn);
if(s == LV_BTN_STATE_PR)
lv_btn_set_state(ext->selected_btn, LV_BTN_STATE_REL);
else if(s == LV_BTN_STATE_TGL_PR)
lv_btn_set_state(ext->selected_btn, LV_BTN_STATE_TGL_REL);
}
ext->selected_btn = btn;
if(btn != NULL) {
ext->last_sel = btn;
}
if(ext->selected_btn) {
lv_btn_state_t s = lv_btn_get_state(ext->selected_btn);
if(s == LV_BTN_STATE_REL)
lv_btn_set_state(ext->selected_btn, LV_BTN_STATE_PR);
else if(s == LV_BTN_STATE_TGL_REL)
lv_btn_set_state(ext->selected_btn, LV_BTN_STATE_TGL_PR);
lv_page_focus(list, ext->selected_btn, LV_ANIM_ON);
}
}
#endif
void lv_list_set_style(lv_obj_t * list, lv_list_style_t type, const lv_style_t * style)
{
LV_ASSERT_OBJ(list, LV_OBJX_NAME);
lv_list_ext_t * ext = lv_obj_get_ext_attr(list);
lv_btn_style_t btn_style_refr = LV_BTN_STYLE_REL;
lv_obj_t * btn;
switch(type) {
case LV_LIST_STYLE_BG:
lv_page_set_style(list, LV_PAGE_STYLE_BG, style);
break;
case LV_LIST_STYLE_SCRL: lv_page_set_style(list, LV_PAGE_STYLE_SCRL, style); break;
case LV_LIST_STYLE_SB: lv_page_set_style(list, LV_PAGE_STYLE_SB, style); break;
case LV_LIST_STYLE_EDGE_FLASH: lv_page_set_style(list, LV_PAGE_STYLE_EDGE_FLASH, style); break;
case LV_LIST_STYLE_BTN_REL:
ext->styles_btn[LV_BTN_STATE_REL] = style;
btn_style_refr = LV_BTN_STYLE_REL;
break;
case LV_LIST_STYLE_BTN_PR:
ext->styles_btn[LV_BTN_STATE_PR] = style;
btn_style_refr = LV_BTN_STYLE_PR;
break;
case LV_LIST_STYLE_BTN_TGL_REL:
ext->styles_btn[LV_BTN_STATE_TGL_REL] = style;
btn_style_refr = LV_BTN_STYLE_TGL_REL;
break;
case LV_LIST_STYLE_BTN_TGL_PR:
ext->styles_btn[LV_BTN_STATE_TGL_PR] = style;
btn_style_refr = LV_BTN_STYLE_TGL_PR;
break;
case LV_LIST_STYLE_BTN_INA:
ext->styles_btn[LV_BTN_STATE_INA] = style;
btn_style_refr = LV_BTN_STYLE_INA;
break;
}
if(type == LV_LIST_STYLE_BTN_PR || type == LV_LIST_STYLE_BTN_REL || type == LV_LIST_STYLE_BTN_TGL_REL ||
type == LV_LIST_STYLE_BTN_TGL_PR || type == LV_LIST_STYLE_BTN_INA) {
btn = lv_list_get_prev_btn(list, NULL);
while(btn != NULL) {
lv_btn_set_style(btn, btn_style_refr, ext->styles_btn[btn_style_refr]);
btn = lv_list_get_prev_btn(list, btn);
}
}
}
void lv_list_set_layout(lv_obj_t * list, lv_layout_t layout)
{
LV_ASSERT_OBJ(list, LV_OBJX_NAME);
if (layout == lv_list_get_layout(list)) return;
lv_obj_t * btn = lv_list_get_prev_btn(list, NULL);
while(btn != NULL) {
if(layout == LV_LAYOUT_COL_M || layout == LV_LAYOUT_COL_L || layout == LV_LAYOUT_COL_R) {
lv_btn_set_fit2(btn, LV_FIT_FLOOD, LV_FIT_TIGHT);
}
else if (layout == LV_LAYOUT_ROW_M || layout == LV_LAYOUT_ROW_T || layout == LV_LAYOUT_ROW_B) {
lv_btn_set_fit(btn, LV_FIT_TIGHT);
}
btn = lv_list_get_prev_btn(list, btn);
}
if(layout == LV_LAYOUT_COL_M || layout == LV_LAYOUT_COL_L || layout == LV_LAYOUT_COL_R) {
lv_page_set_scrl_fit2(list, LV_FIT_FLOOD, LV_FIT_TIGHT);
} else if (layout == LV_LAYOUT_ROW_M || layout == LV_LAYOUT_ROW_T || layout == LV_LAYOUT_ROW_B) {
lv_page_set_scrl_fit2(list, LV_FIT_TIGHT, LV_FIT_TIGHT);
lv_cont_set_fit2(list, LV_FIT_NONE, LV_FIT_TIGHT);
}
lv_page_set_scrl_layout(list, layout);
}
bool lv_list_get_single_mode(lv_obj_t * list)
{
LV_ASSERT_OBJ(list, LV_OBJX_NAME);
lv_list_ext_t * ext = lv_obj_get_ext_attr(list);
return (ext->single_mode);
}
const char * lv_list_get_btn_text(const lv_obj_t * btn)
{
LV_ASSERT_OBJ(btn, "lv_btn");
lv_obj_t * label = lv_list_get_btn_label(btn);
if(label == NULL) return "";
return lv_label_get_text(label);
}
lv_obj_t * lv_list_get_btn_label(const lv_obj_t * btn)
{
LV_ASSERT_OBJ(btn, "lv_btn");
lv_obj_t * label = lv_obj_get_child(btn, NULL);
if(label == NULL) return NULL;
while(lv_list_is_list_label(label) == false) {
label = lv_obj_get_child(btn, label);
if(label == NULL) break;
}
return label;
}
lv_obj_t * lv_list_get_btn_img(const lv_obj_t * btn)
{
LV_ASSERT_OBJ(btn, "lv_btn");
#if LV_USE_IMG != 0
lv_obj_t * img = lv_obj_get_child(btn, NULL);
if(img == NULL) return NULL;
while(lv_list_is_list_img(img) == false) {
img = lv_obj_get_child(btn, img);
if(img == NULL) break;
}
return img;
#else
return NULL;
#endif
}
lv_obj_t * lv_list_get_prev_btn(const lv_obj_t * list, lv_obj_t * prev_btn)
{
LV_ASSERT_OBJ(list, LV_OBJX_NAME);
lv_obj_t * btn;
lv_obj_t * scrl = lv_page_get_scrl(list);
btn = lv_obj_get_child(scrl, prev_btn);
if(btn == NULL) return NULL;
while(lv_list_is_list_btn(btn) == false) {
btn = lv_obj_get_child(scrl, btn);
if(btn == NULL) break;
}
return btn;
}
lv_obj_t * lv_list_get_next_btn(const lv_obj_t * list, lv_obj_t * prev_btn)
{
LV_ASSERT_OBJ(list, LV_OBJX_NAME);
lv_obj_t * btn;
lv_obj_t * scrl = lv_page_get_scrl(list);
btn = lv_obj_get_child_back(scrl, prev_btn);
if(btn == NULL) return NULL;
while(lv_list_is_list_btn(btn) == false) {
btn = lv_obj_get_child_back(scrl, btn);
if(btn == NULL) break;
}
return btn;
}
int32_t lv_list_get_btn_index(const lv_obj_t * list, const lv_obj_t * btn)
{
LV_ASSERT_OBJ(list, LV_OBJX_NAME);
LV_ASSERT_OBJ(btn, "lv_btn");
int index = 0;
if(list == NULL) {
list = lv_obj_get_parent(lv_obj_get_parent(btn));
}
lv_obj_t * e = lv_list_get_next_btn(list, NULL);
while(e != NULL) {
if(e == btn) {
return index;
}
index++;
e = lv_list_get_next_btn(list, e);
}
return -1;
}
uint16_t lv_list_get_size(const lv_obj_t * list)
{
LV_ASSERT_OBJ(list, LV_OBJX_NAME);
lv_list_ext_t * ext = lv_obj_get_ext_attr(list);
return ext->size;
}
#if LV_USE_GROUP
lv_obj_t * lv_list_get_btn_selected(const lv_obj_t * list)
{
LV_ASSERT_OBJ(list, LV_OBJX_NAME);
lv_list_ext_t * ext = lv_obj_get_ext_attr(list);
return ext->selected_btn;
}
#endif
lv_layout_t lv_list_get_layout(lv_obj_t * list)
{
LV_ASSERT_OBJ(list, LV_OBJX_NAME);
return lv_page_get_scrl_layout(list);
}
const lv_style_t * lv_list_get_style(const lv_obj_t * list, lv_list_style_t type)
{
LV_ASSERT_OBJ(list, LV_OBJX_NAME);
const lv_style_t * style = NULL;
lv_list_ext_t * ext = lv_obj_get_ext_attr(list);
switch(type) {
case LV_LIST_STYLE_BG: style = lv_page_get_style(list, LV_PAGE_STYLE_BG); break;
case LV_LIST_STYLE_SCRL: style = lv_page_get_style(list, LV_PAGE_STYLE_SCRL); break;
case LV_LIST_STYLE_SB: style = lv_page_get_style(list, LV_PAGE_STYLE_SB); break;
case LV_LIST_STYLE_EDGE_FLASH: style = lv_page_get_style(list, LV_PAGE_STYLE_EDGE_FLASH); break;
case LV_LIST_STYLE_BTN_REL: style = ext->styles_btn[LV_BTN_STATE_REL]; break;
case LV_LIST_STYLE_BTN_PR: style = ext->styles_btn[LV_BTN_STATE_PR]; break;
case LV_LIST_STYLE_BTN_TGL_REL: style = ext->styles_btn[LV_BTN_STATE_TGL_REL]; break;
case LV_LIST_STYLE_BTN_TGL_PR: style = ext->styles_btn[LV_BTN_STATE_TGL_PR]; break;
case LV_LIST_STYLE_BTN_INA: style = ext->styles_btn[LV_BTN_STATE_INA]; break;
default: style = NULL; break;
}
return style;
}
void lv_list_up(const lv_obj_t * list)
{
LV_ASSERT_OBJ(list, LV_OBJX_NAME);
lv_obj_t * scrl = lv_page_get_scrl(list);
lv_obj_t * e;
lv_obj_t * e_prev = NULL;
e = lv_list_get_prev_btn(list, NULL);
while(e != NULL) {
if(e->coords.y2 <= list->coords.y2) {
if(e_prev != NULL) {
lv_coord_t new_y = lv_obj_get_height(list) - (lv_obj_get_y(e_prev) + lv_obj_get_height(e_prev));
if(lv_list_get_anim_time(list) == 0) {
lv_obj_set_y(scrl, new_y);
} else {
#if LV_USE_ANIMATION
lv_anim_t a;
a.var = scrl;
a.start = lv_obj_get_y(scrl);
a.end = new_y;
a.exec_cb = (lv_anim_exec_xcb_t)lv_obj_set_y;
a.path_cb = lv_anim_path_linear;
a.ready_cb = NULL;
a.act_time = 0;
a.time = LV_LIST_DEF_ANIM_TIME;
a.playback = 0;
a.playback_pause = 0;
a.repeat = 0;
a.repeat_pause = 0;
lv_anim_create(&a);
#endif
}
}
break;
}
e_prev = e;
e = lv_list_get_prev_btn(list, e);
}
}
void lv_list_down(const lv_obj_t * list)
{
LV_ASSERT_OBJ(list, LV_OBJX_NAME);
lv_obj_t * scrl = lv_page_get_scrl(list);
lv_obj_t * e;
e = lv_list_get_prev_btn(list, NULL);
while(e != NULL) {
if(e->coords.y1 < list->coords.y1) {
lv_coord_t new_y = -lv_obj_get_y(e);
if(lv_list_get_anim_time(list) == 0) {
lv_obj_set_y(scrl, new_y);
} else {
#if LV_USE_ANIMATION
lv_anim_t a;
a.var = scrl;
a.start = lv_obj_get_y(scrl);
a.end = new_y;
a.exec_cb = (lv_anim_exec_xcb_t)lv_obj_set_y;
a.path_cb = lv_anim_path_linear;
a.ready_cb = NULL;
a.act_time = 0;
a.time = LV_LIST_DEF_ANIM_TIME;
a.playback = 0;
a.playback_pause = 0;
a.repeat = 0;
a.repeat_pause = 0;
lv_anim_create(&a);
#endif
}
break;
}
e = lv_list_get_prev_btn(list, e);
}
}
void lv_list_focus(const lv_obj_t * btn, lv_anim_enable_t anim)
{
LV_ASSERT_OBJ(btn, "");
#if LV_USE_ANIMATION == 0
anim = false;
#endif
lv_obj_t * list = lv_obj_get_parent(lv_obj_get_parent(btn));
lv_page_focus(list, btn, anim == LV_ANIM_OFF ? 0 : lv_list_get_anim_time(list));
}
static lv_res_t lv_list_signal(lv_obj_t * list, lv_signal_t sign, void * param)
{
lv_res_t res;
res = ancestor_page_signal(list, sign, param);
if(res != LV_RES_OK) return res;
if(sign == LV_SIGNAL_GET_TYPE) return lv_obj_handle_get_type_signal(param, LV_OBJX_NAME);
if(sign == LV_SIGNAL_RELEASED || sign == LV_SIGNAL_PRESSED || sign == LV_SIGNAL_PRESSING ||
sign == LV_SIGNAL_LONG_PRESS || sign == LV_SIGNAL_LONG_PRESS_REP) {
#if LV_USE_GROUP
lv_indev_t * indev = lv_indev_get_act();
lv_indev_type_t indev_type = lv_indev_get_type(indev);
if(indev_type == LV_INDEV_TYPE_KEYPAD ||
(indev_type == LV_INDEV_TYPE_ENCODER && lv_group_get_editing(lv_obj_get_group(list)))) {
lv_obj_t * btn = NULL;
btn = lv_list_get_prev_btn(list, btn);
while(btn != NULL) {
if(lv_btn_get_state(btn) == LV_BTN_STATE_PR) break;
btn = lv_list_get_prev_btn(list, btn);
}
lv_list_ext_t * ext = lv_obj_get_ext_attr(list);
if(btn) {
if(sign == LV_SIGNAL_PRESSED) {
res = lv_event_send(btn, LV_EVENT_PRESSED, NULL);
} else if(sign == LV_SIGNAL_PRESSING) {
res = lv_event_send(btn, LV_EVENT_PRESSING, NULL);
} else if(sign == LV_SIGNAL_LONG_PRESS) {
res = lv_event_send(btn, LV_EVENT_LONG_PRESSED, NULL);
} else if(sign == LV_SIGNAL_LONG_PRESS_REP) {
res = lv_event_send(btn, LV_EVENT_LONG_PRESSED_REPEAT, NULL);
} else if(sign == LV_SIGNAL_RELEASED) {
#if LV_USE_GROUP
ext->last_sel = btn;
#endif
if(indev->proc.long_pr_sent == 0) {
res = lv_event_send(btn, LV_EVENT_SHORT_CLICKED, NULL);
}
if(lv_indev_is_dragging(indev) == false && res == LV_RES_OK) {
res = lv_event_send(btn, LV_EVENT_CLICKED, NULL);
}
if(res == LV_RES_OK) {
res = lv_event_send(btn, LV_EVENT_RELEASED, NULL);
}
}
}
}
#endif
} else if(sign == LV_SIGNAL_FOCUS) {
#if LV_USE_GROUP
lv_indev_type_t indev_type = lv_indev_get_type(lv_indev_get_act());
if(indev_type == LV_INDEV_TYPE_ENCODER) {
lv_group_t * g = lv_obj_get_group(list);
if(lv_group_get_editing(g)) {
lv_list_ext_t * ext = lv_obj_get_ext_attr(list);
if(ext->last_sel) {
lv_list_set_btn_selected(list, ext->last_sel);
} else {
lv_list_set_btn_selected(list, lv_list_get_next_btn(list, NULL));
}
} else {
lv_list_set_btn_selected(list, NULL);
}
}
else {
lv_list_ext_t * ext = lv_obj_get_ext_attr(list);
if(ext->last_clicked_btn) {
lv_list_set_btn_selected(list, ext->last_clicked_btn);
ext->last_clicked_btn = NULL;
} else {
if(ext->last_sel) {
lv_list_set_btn_selected(list, ext->last_sel);
} else {
lv_list_set_btn_selected(list, lv_list_get_next_btn(list, NULL));
}
}
}
#endif
} else if(sign == LV_SIGNAL_DEFOCUS) {
#if LV_USE_GROUP
lv_list_set_btn_selected(list, NULL);
lv_list_ext_t * ext = lv_obj_get_ext_attr(list);
ext->last_clicked_btn = NULL; 
ext->selected_btn = NULL;
#endif
} else if(sign == LV_SIGNAL_GET_EDITABLE) {
bool * editable = (bool *)param;
*editable = true;
} else if(sign == LV_SIGNAL_CONTROL) {
#if LV_USE_GROUP
char c = *((char *)param);
if(c == LV_KEY_RIGHT || c == LV_KEY_DOWN) {
lv_list_ext_t * ext = lv_obj_get_ext_attr(list);
if(ext->selected_btn) {
lv_obj_t * btn_prev = lv_list_get_next_btn(list, ext->selected_btn);
if(btn_prev) lv_list_set_btn_selected(list, btn_prev);
}
else {
lv_obj_t * btn = lv_list_get_next_btn(list, NULL);
if(btn)
lv_list_set_btn_selected(list,
btn); 
}
} else if(c == LV_KEY_LEFT || c == LV_KEY_UP) {
lv_list_ext_t * ext = lv_obj_get_ext_attr(list);
if(ext->selected_btn != NULL) {
lv_obj_t * btn_next = lv_list_get_prev_btn(list, ext->selected_btn);
if(btn_next) lv_list_set_btn_selected(list, btn_next);
}
else {
lv_obj_t * btn = lv_list_get_next_btn(list, NULL);
if(btn) lv_list_set_btn_selected(list, btn);
}
}
#endif
}
return res;
}
static lv_res_t lv_list_btn_signal(lv_obj_t * btn, lv_signal_t sign, void * param)
{
lv_res_t res;
res = ancestor_btn_signal(btn, sign, param);
if(res != LV_RES_OK) return res;
if(sign == LV_SIGNAL_GET_TYPE) return lv_obj_handle_get_type_signal(param, "");
if(sign == LV_SIGNAL_RELEASED) {
lv_obj_t * list = lv_obj_get_parent(lv_obj_get_parent(btn));
lv_list_ext_t * ext = lv_obj_get_ext_attr(list);
ext->page.scroll_prop_ip = 0;
#if LV_USE_GROUP
lv_group_t * g = lv_obj_get_group(list);
if(lv_group_get_focused(g) == list && lv_indev_is_dragging(lv_indev_get_act()) == false) {
lv_obj_t * btn_i = lv_list_get_prev_btn(list, NULL);
while(btn_i) {
lv_btn_state_t s = lv_btn_get_state(btn_i);
if(s == LV_BTN_STATE_PR)
lv_btn_set_state(btn_i, LV_BTN_STATE_REL);
else if(s == LV_BTN_STATE_TGL_PR)
lv_btn_set_state(btn_i, LV_BTN_STATE_TGL_REL);
btn_i = lv_list_get_prev_btn(list, btn_i);
}
lv_list_set_btn_selected(list, btn);
}
ext->last_clicked_btn = btn;
#endif
if(lv_indev_is_dragging(lv_indev_get_act()) == false && ext->single_mode) {
lv_list_btn_single_select(btn);
}
} else if(sign == LV_SIGNAL_PRESS_LOST) {
lv_obj_t * list = lv_obj_get_parent(lv_obj_get_parent(btn));
lv_list_ext_t * ext = lv_obj_get_ext_attr(list);
ext->page.scroll_prop_ip = 0;
} else if(sign == LV_SIGNAL_CLEANUP) {
#if LV_USE_GROUP
lv_obj_t * list = lv_obj_get_parent(lv_obj_get_parent(btn));
lv_obj_t * sel = lv_list_get_btn_selected(list);
if(sel == btn) lv_list_set_btn_selected(list, lv_list_get_next_btn(list, btn));
#endif
}
return res;
}
static void lv_list_btn_single_select(lv_obj_t * btn)
{
lv_obj_t * list = lv_obj_get_parent(lv_obj_get_parent(btn));
lv_obj_t * e = lv_list_get_next_btn(list, NULL);
do {
if(e == btn) {
lv_btn_set_state(e, LV_BTN_STATE_TGL_REL);
} else {
lv_btn_set_state(e, LV_BTN_STATE_REL);
}
e = lv_list_get_next_btn(list, e);
} while(e != NULL);
}
static bool lv_list_is_list_btn(lv_obj_t * list_btn)
{
lv_obj_type_t type;
lv_obj_get_type(list_btn, &type);
uint8_t cnt;
for(cnt = 0; cnt < LV_MAX_ANCESTOR_NUM; cnt++) {
if(type.type[cnt] == NULL) break;
if(!strcmp(type.type[cnt], "lv_btn")) return true;
}
return false;
}
static bool lv_list_is_list_label(lv_obj_t * list_label)
{
lv_obj_type_t type;
lv_obj_get_type(list_label, &type);
uint8_t cnt;
for(cnt = 0; cnt < LV_MAX_ANCESTOR_NUM; cnt++) {
if(type.type[cnt] == NULL) break;
if(!strcmp(type.type[cnt], "lv_label")) return true;
}
return false;
}
static bool lv_list_is_list_img(lv_obj_t * list_img)
{
lv_obj_type_t type;
lv_obj_get_type(list_img, &type);
uint8_t cnt;
for(cnt = 0; cnt < LV_MAX_ANCESTOR_NUM; cnt++) {
if(type.type[cnt] == NULL) break;
if(!strcmp(type.type[cnt], "lv_img")) return true;
}
return false;
}
#endif
