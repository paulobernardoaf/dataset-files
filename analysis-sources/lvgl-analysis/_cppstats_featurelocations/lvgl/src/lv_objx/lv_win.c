







#include "lv_win.h"
#if LV_USE_WIN != 0

#include "../lv_core/lv_debug.h"
#include "../lv_themes/lv_theme.h"
#include "../lv_core/lv_disp.h"




#define LV_OBJX_NAME "lv_win"








static lv_res_t lv_win_signal(lv_obj_t * win, lv_signal_t sign, void * param);
static void lv_win_realign(lv_obj_t * win);




static lv_signal_cb_t ancestor_signal;















lv_obj_t * lv_win_create(lv_obj_t * par, const lv_obj_t * copy)
{
LV_LOG_TRACE("window create started");


lv_obj_t * new_win = lv_obj_create(par, copy);
LV_ASSERT_MEM(new_win);
if(new_win == NULL) return NULL;

if(ancestor_signal == NULL) ancestor_signal = lv_obj_get_signal_cb(new_win);


lv_win_ext_t * ext = lv_obj_allocate_ext_attr(new_win, sizeof(lv_win_ext_t));
LV_ASSERT_MEM(ext);
if(ext == NULL) return NULL;

ext->page = NULL;
ext->header = NULL;
ext->title = NULL;
ext->style_btn_rel = &lv_style_btn_rel;
ext->style_btn_pr = &lv_style_btn_pr;
ext->btn_size = (LV_DPI) / 2;


if(copy == NULL) {



lv_coord_t w;
lv_coord_t h;
if(par) {
w = lv_obj_get_width_fit(lv_obj_get_parent(new_win));
h = lv_obj_get_height_fit(lv_obj_get_parent(new_win));
} else {
w = lv_disp_get_hor_res(NULL);
h = lv_disp_get_ver_res(NULL);
}

lv_obj_set_size(new_win, w, h);

lv_obj_set_style(new_win, &lv_style_pretty);

ext->page = lv_page_create(new_win, NULL);
lv_obj_set_protect(ext->page, LV_PROTECT_PARENT);
lv_page_set_sb_mode(ext->page, LV_SB_MODE_AUTO);
lv_page_set_style(ext->page, LV_PAGE_STYLE_BG, &lv_style_transp_fit);


ext->header = lv_obj_create(new_win, NULL);

lv_obj_set_protect(ext->header, LV_PROTECT_PARENT);
lv_obj_set_parent(ext->header, new_win);


ext->title = lv_label_create(ext->header, NULL);
lv_label_set_text(ext->title, "My title");

lv_obj_set_signal_cb(new_win, lv_win_signal);


lv_theme_t * th = lv_theme_get_current();
if(th) {
lv_win_set_style(new_win, LV_WIN_STYLE_BG, th->style.win.bg);
lv_win_set_style(new_win, LV_WIN_STYLE_SB, th->style.win.sb);
lv_win_set_style(new_win, LV_WIN_STYLE_HEADER, th->style.win.header);
lv_win_set_style(new_win, LV_WIN_STYLE_CONTENT, th->style.win.content);
lv_win_set_style(new_win, LV_WIN_STYLE_BTN_REL, th->style.win.btn.rel);
lv_win_set_style(new_win, LV_WIN_STYLE_BTN_PR, th->style.win.btn.pr);
} else {
lv_win_set_style(new_win, LV_WIN_STYLE_BG, &lv_style_plain);
lv_win_set_style(new_win, LV_WIN_STYLE_CONTENT, &lv_style_transp);
lv_win_set_style(new_win, LV_WIN_STYLE_HEADER, &lv_style_plain_color);
}
}

else {
lv_win_ext_t * copy_ext = lv_obj_get_ext_attr(copy);

ext->header = lv_obj_create(new_win, copy_ext->header);
ext->title = lv_label_create(ext->header, copy_ext->title);
ext->page = lv_page_create(new_win, copy_ext->page);
ext->btn_size = copy_ext->btn_size;


lv_obj_t * child;
lv_obj_t * cbtn;
child = lv_obj_get_child_back(copy_ext->header, NULL);
child = lv_obj_get_child_back(copy_ext->header, child); 
while(child != NULL) {
cbtn = lv_btn_create(ext->header, child);
lv_img_create(cbtn, lv_obj_get_child(child, NULL));
child = lv_obj_get_child_back(copy_ext->header, child);
}

lv_obj_set_signal_cb(new_win, lv_win_signal);
}


lv_obj_refresh_style(new_win);

lv_win_realign(new_win);

LV_LOG_INFO("window created");

return new_win;
}





void lv_win_clean(lv_obj_t * win)
{
LV_ASSERT_OBJ(win, LV_OBJX_NAME);

lv_obj_t * scrl = lv_page_get_scrl(win);
lv_obj_clean(scrl);
}











lv_obj_t * lv_win_add_btn(lv_obj_t * win, const void * img_src)
{
LV_ASSERT_OBJ(win, LV_OBJX_NAME);
LV_ASSERT_NULL(img_src);

lv_win_ext_t * ext = lv_obj_get_ext_attr(win);

lv_obj_t * btn = lv_btn_create(ext->header, NULL);
lv_btn_set_style(btn, LV_BTN_STYLE_REL, ext->style_btn_rel);
lv_btn_set_style(btn, LV_BTN_STYLE_PR, ext->style_btn_pr);
lv_obj_set_size(btn, ext->btn_size, ext->btn_size);

lv_obj_t * img = lv_img_create(btn, NULL);
lv_obj_set_click(img, false);
lv_img_set_src(img, img_src);

lv_win_realign(win);

return btn;
}










void lv_win_close_event_cb(lv_obj_t * btn, lv_event_t event)
{
LV_ASSERT_OBJ(btn, "lv_btn");

if(event == LV_EVENT_RELEASED) {
lv_obj_t * win = lv_win_get_from_btn(btn);

lv_obj_del(win);
}
}






void lv_win_set_title(lv_obj_t * win, const char * title)
{
LV_ASSERT_OBJ(win, LV_OBJX_NAME);
LV_ASSERT_STR(title);

lv_win_ext_t * ext = lv_obj_get_ext_attr(win);

lv_label_set_text(ext->title, title);
lv_win_realign(win);
}






void lv_win_set_btn_size(lv_obj_t * win, lv_coord_t size)
{
LV_ASSERT_OBJ(win, LV_OBJX_NAME);

lv_win_ext_t * ext = lv_obj_get_ext_attr(win);
if(ext->btn_size == size) return;

ext->btn_size = size;

lv_win_realign(win);
}







void lv_win_set_content_size(lv_obj_t * win, lv_coord_t w, lv_coord_t h)
{
LV_ASSERT_OBJ(win, LV_OBJX_NAME);

lv_win_ext_t * ext = lv_obj_get_ext_attr(win);
h += lv_obj_get_height(ext->header);

lv_obj_set_size(win, w, h);
}






void lv_win_set_layout(lv_obj_t * win, lv_layout_t layout)
{
LV_ASSERT_OBJ(win, LV_OBJX_NAME);

lv_win_ext_t * ext = lv_obj_get_ext_attr(win);
lv_page_set_scrl_layout(ext->page, layout);
}






void lv_win_set_sb_mode(lv_obj_t * win, lv_sb_mode_t sb_mode)
{
LV_ASSERT_OBJ(win, LV_OBJX_NAME);

lv_win_ext_t * ext = lv_obj_get_ext_attr(win);
lv_page_set_sb_mode(ext->page, sb_mode);
}





void lv_win_set_anim_time(lv_obj_t * win, uint16_t anim_time)
{
LV_ASSERT_OBJ(win, LV_OBJX_NAME);

lv_page_set_anim_time(lv_win_get_content(win), anim_time);
}







void lv_win_set_style(lv_obj_t * win, lv_win_style_t type, const lv_style_t * style)
{
LV_ASSERT_OBJ(win, LV_OBJX_NAME);

lv_win_ext_t * ext = lv_obj_get_ext_attr(win);

switch(type) {
case LV_WIN_STYLE_BG:
lv_obj_set_style(win, style);
lv_win_realign(win);
break;
case LV_WIN_STYLE_CONTENT: lv_page_set_style(ext->page, LV_PAGE_STYLE_SCRL, style); break;
case LV_WIN_STYLE_SB: lv_page_set_style(ext->page, LV_PAGE_STYLE_SB, style); break;
case LV_WIN_STYLE_HEADER:
lv_obj_set_style(ext->header, style);
lv_win_realign(win);
break;
case LV_WIN_STYLE_BTN_REL: ext->style_btn_rel = style; break;
case LV_WIN_STYLE_BTN_PR: ext->style_btn_pr = style; break;
}


if(type == LV_WIN_STYLE_BTN_REL || type == LV_WIN_STYLE_BTN_PR) {
lv_obj_t * btn;
btn = lv_obj_get_child_back(ext->header, NULL);
btn = lv_obj_get_child_back(ext->header, btn); 
while(btn != NULL) {
if(type == LV_WIN_STYLE_BTN_REL)
lv_btn_set_style(btn, LV_BTN_STYLE_REL, style);
else
lv_btn_set_style(btn, LV_BTN_STYLE_PR, style);
btn = lv_obj_get_child_back(ext->header, btn);
}
}
}






void lv_win_set_drag(lv_obj_t * win, bool en)
{
LV_ASSERT_OBJ(win, LV_OBJX_NAME);

lv_win_ext_t * ext = lv_obj_get_ext_attr(win);
lv_obj_t * win_header = ext->header;
lv_obj_set_drag_parent(win_header, en);
lv_obj_set_drag(win, en);
}










const char * lv_win_get_title(const lv_obj_t * win)
{
LV_ASSERT_OBJ(win, LV_OBJX_NAME);

lv_win_ext_t * ext = lv_obj_get_ext_attr(win);
return lv_label_get_text(ext->title);
}






lv_obj_t * lv_win_get_content(const lv_obj_t * win)
{
LV_ASSERT_OBJ(win, LV_OBJX_NAME);

lv_win_ext_t * ext = lv_obj_get_ext_attr(win);
return ext->page;
}






lv_coord_t lv_win_get_btn_size(const lv_obj_t * win)
{
LV_ASSERT_OBJ(win, LV_OBJX_NAME);

lv_win_ext_t * ext = lv_obj_get_ext_attr(win);
return ext->btn_size;
}







lv_obj_t * lv_win_get_from_btn(const lv_obj_t * ctrl_btn)
{
LV_ASSERT_OBJ(ctrl_btn, "lv_btn");

lv_obj_t * header = lv_obj_get_parent(ctrl_btn);
lv_obj_t * win = lv_obj_get_parent(header);

return win;
}






lv_layout_t lv_win_get_layout(lv_obj_t * win)
{
LV_ASSERT_OBJ(win, LV_OBJX_NAME);

lv_win_ext_t * ext = lv_obj_get_ext_attr(win);
return lv_page_get_scrl_layout(ext->page);
}






lv_sb_mode_t lv_win_get_sb_mode(lv_obj_t * win)
{
LV_ASSERT_OBJ(win, LV_OBJX_NAME);

lv_win_ext_t * ext = lv_obj_get_ext_attr(win);
return lv_page_get_sb_mode(ext->page);
}






uint16_t lv_win_get_anim_time(const lv_obj_t * win)
{
LV_ASSERT_OBJ(win, LV_OBJX_NAME);

return lv_page_get_anim_time(lv_win_get_content(win));
}






lv_coord_t lv_win_get_width(lv_obj_t * win)
{
LV_ASSERT_OBJ(win, LV_OBJX_NAME);

lv_win_ext_t * ext = lv_obj_get_ext_attr(win);
lv_obj_t * scrl = lv_page_get_scrl(ext->page);
const lv_style_t * style_scrl = lv_obj_get_style(scrl);

return lv_obj_get_width(scrl) - style_scrl->body.padding.left - style_scrl->body.padding.right;
}







const lv_style_t * lv_win_get_style(const lv_obj_t * win, lv_win_style_t type)
{
LV_ASSERT_OBJ(win, LV_OBJX_NAME);

const lv_style_t * style = NULL;
lv_win_ext_t * ext = lv_obj_get_ext_attr(win);

switch(type) {
case LV_WIN_STYLE_BG: style = lv_obj_get_style(win); break;
case LV_WIN_STYLE_CONTENT: style = lv_page_get_style(ext->page, LV_PAGE_STYLE_SCRL); break;
case LV_WIN_STYLE_SB: style = lv_page_get_style(ext->page, LV_PAGE_STYLE_SB); break;
case LV_WIN_STYLE_HEADER: style = lv_obj_get_style(ext->header); break;
case LV_WIN_STYLE_BTN_REL: style = ext->style_btn_rel; break;
case LV_WIN_STYLE_BTN_PR: style = ext->style_btn_pr; break;
default: style = NULL; break;
}

return style;
}











void lv_win_focus(lv_obj_t * win, lv_obj_t * obj, lv_anim_enable_t anim_en)
{
LV_ASSERT_OBJ(win, LV_OBJX_NAME);
LV_ASSERT_OBJ(obj, "");


lv_win_ext_t * ext = lv_obj_get_ext_attr(win);
lv_page_focus(ext->page, obj, anim_en);
}












static lv_res_t lv_win_signal(lv_obj_t * win, lv_signal_t sign, void * param)
{
lv_res_t res;


res = ancestor_signal(win, sign, param);
if(res != LV_RES_OK) return res;
if(sign == LV_SIGNAL_GET_TYPE) return lv_obj_handle_get_type_signal(param, LV_OBJX_NAME);

lv_win_ext_t * ext = lv_obj_get_ext_attr(win);
if(sign == LV_SIGNAL_CHILD_CHG) { 
lv_obj_t * page = ext->page;
if(page != NULL) {
lv_obj_t * child;
child = lv_obj_get_child(win, NULL);
while(child != NULL) {
if(lv_obj_is_protected(child, LV_PROTECT_PARENT) == false) {
lv_obj_t * tmp = child;
child = lv_obj_get_child(win, child); 
lv_obj_set_parent(tmp, page);
} else {
child = lv_obj_get_child(win, child);
}
}
}
} else if(sign == LV_SIGNAL_STYLE_CHG) {
lv_win_realign(win);
} else if(sign == LV_SIGNAL_CORD_CHG) {

if(lv_area_get_width(param) != lv_obj_get_width(win) || lv_area_get_height(param) != lv_obj_get_height(win)) {
lv_win_realign(win);
}
} else if(sign == LV_SIGNAL_CLEANUP) {
ext->header = NULL; 
ext->page = NULL;
ext->title = NULL;
} else if(sign == LV_SIGNAL_CONTROL) {

ext->page->signal_cb(ext->page, sign, param);
}

return res;
}





static void lv_win_realign(lv_obj_t * win)
{
lv_win_ext_t * ext = lv_obj_get_ext_attr(win);

if(ext->page == NULL || ext->header == NULL || ext->title == NULL) return;

const lv_style_t * header_style = lv_win_get_style(win, LV_WIN_STYLE_HEADER);
lv_obj_set_size(ext->header, lv_obj_get_width(win),
ext->btn_size + header_style->body.padding.top + header_style->body.padding.bottom);

bool first_btn = true;
lv_obj_t * btn;
lv_obj_t * btn_prev = NULL;

btn = lv_obj_get_child_back(ext->header, NULL);
btn = lv_obj_get_child_back(ext->header, btn); 
while(btn != NULL) {
lv_obj_set_size(btn, ext->btn_size, ext->btn_size);
if(first_btn) {
lv_obj_align(btn, ext->header, LV_ALIGN_IN_RIGHT_MID, -header_style->body.padding.right, 0);
first_btn = false;
} else {
lv_obj_align(btn, btn_prev, LV_ALIGN_OUT_LEFT_MID, -header_style->body.padding.inner, 0);
}
btn_prev = btn;
btn = lv_obj_get_child_back(ext->header, btn);
}

const lv_style_t * style_header = lv_win_get_style(win, LV_WIN_STYLE_HEADER);
lv_obj_align(ext->title, NULL, LV_ALIGN_IN_LEFT_MID, style_header->body.padding.left, 0);

lv_obj_set_pos(ext->header, 0, 0);

lv_obj_set_size(ext->page, lv_obj_get_width(win), lv_obj_get_height(win) - lv_obj_get_height(ext->header));
lv_obj_align(ext->page, ext->header, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
}

#endif
