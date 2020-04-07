#include "lv_ta.h"
#if LV_USE_TA != 0
#include <string.h>
#include "../lv_core/lv_debug.h"
#include "../lv_core/lv_group.h"
#include "../lv_core/lv_refr.h"
#include "../lv_draw/lv_draw.h"
#include "../lv_themes/lv_theme.h"
#include "../lv_misc/lv_anim.h"
#include "../lv_misc/lv_txt.h"
#include "../lv_misc/lv_math.h"
#define LV_OBJX_NAME "lv_ta"
#if !defined(LV_TA_DEF_CURSOR_BLINK_TIME)
#define LV_TA_DEF_CURSOR_BLINK_TIME 400 
#endif
#if !defined(LV_TA_DEF_PWD_SHOW_TIME)
#define LV_TA_DEF_PWD_SHOW_TIME 1500 
#endif
#define LV_TA_DEF_WIDTH (2 * LV_DPI)
#define LV_TA_DEF_HEIGHT (1 * LV_DPI)
static bool lv_ta_design(lv_obj_t * ta, const lv_area_t * mask, lv_design_mode_t mode);
static bool lv_ta_scrollable_design(lv_obj_t * scrl, const lv_area_t * mask, lv_design_mode_t mode);
static lv_res_t lv_ta_signal(lv_obj_t * ta, lv_signal_t sign, void * param);
static lv_res_t lv_ta_scrollable_signal(lv_obj_t * scrl, lv_signal_t sign, void * param);
#if LV_USE_ANIMATION
static void cursor_blink_anim(lv_obj_t * ta, lv_anim_value_t show);
static void pwd_char_hider_anim(lv_obj_t * ta, lv_anim_value_t x);
static void pwd_char_hider_anim_ready(lv_anim_t * a);
#endif
static void pwd_char_hider(lv_obj_t * ta);
static bool char_is_accepted(lv_obj_t * ta, uint32_t c);
static void get_cursor_style(lv_obj_t * ta, lv_style_t * style_res);
static void refr_cursor_area(lv_obj_t * ta);
static void placeholder_update(lv_obj_t * ta);
static void update_cursor_position_on_click(lv_obj_t * ta, lv_signal_t sign, lv_indev_t * click_source);
static lv_design_cb_t ancestor_design;
static lv_design_cb_t scrl_design;
static lv_signal_cb_t ancestor_signal;
static lv_signal_cb_t scrl_signal;
static const char * ta_insert_replace;
lv_obj_t * lv_ta_create(lv_obj_t * par, const lv_obj_t * copy)
{
LV_LOG_TRACE("text area create started");
lv_obj_t * new_ta = lv_page_create(par, copy);
LV_ASSERT_MEM(new_ta);
if(new_ta == NULL) return NULL;
if(ancestor_signal == NULL) ancestor_signal = lv_obj_get_signal_cb(new_ta);
if(ancestor_design == NULL) ancestor_design = lv_obj_get_design_cb(new_ta);
if(scrl_signal == NULL) scrl_signal = lv_obj_get_signal_cb(lv_page_get_scrl(new_ta));
if(scrl_design == NULL) scrl_design = lv_obj_get_design_cb(lv_page_get_scrl(new_ta));
lv_ta_ext_t * ext = lv_obj_allocate_ext_attr(new_ta, sizeof(lv_ta_ext_t));
LV_ASSERT_MEM(ext);
if(ext == NULL) return NULL;
ext->cursor.state = 1;
ext->pwd_mode = 0;
ext->pwd_tmp = NULL;
ext->pwd_show_time = LV_TA_DEF_PWD_SHOW_TIME;
ext->accapted_chars = NULL;
ext->max_length = 0;
ext->cursor.style = NULL;
ext->cursor.blink_time = LV_TA_DEF_CURSOR_BLINK_TIME;
ext->cursor.pos = 0;
ext->cursor.click_pos = 1;
ext->cursor.type = LV_CURSOR_LINE;
ext->cursor.valid_x = 0;
ext->one_line = 0;
#if LV_LABEL_TEXT_SEL
ext->text_sel_en = 0;
#endif
ext->label = NULL;
ext->placeholder = NULL;
#if LV_USE_ANIMATION == 0
ext->pwd_show_time = 0;
ext->cursor.blink_time = 0;
#endif
lv_obj_set_signal_cb(new_ta, lv_ta_signal);
lv_obj_set_signal_cb(lv_page_get_scrl(new_ta), lv_ta_scrollable_signal);
lv_obj_set_design_cb(new_ta, lv_ta_design);
if(copy == NULL) {
lv_page_set_scrl_fit2(new_ta, LV_FIT_FLOOD, LV_FIT_TIGHT);
ext->label = lv_label_create(new_ta, NULL);
lv_obj_set_design_cb(ext->page.scrl, lv_ta_scrollable_design);
lv_label_set_long_mode(ext->label, LV_LABEL_LONG_BREAK);
lv_label_set_text(ext->label, "Text area");
lv_obj_set_click(ext->label, false);
lv_obj_set_size(new_ta, LV_TA_DEF_WIDTH, LV_TA_DEF_HEIGHT);
lv_ta_set_sb_mode(new_ta, LV_SB_MODE_DRAG);
lv_page_set_style(new_ta, LV_PAGE_STYLE_SCRL, &lv_style_transp_tight);
lv_theme_t * th = lv_theme_get_current();
if(th) {
lv_ta_set_style(new_ta, LV_TA_STYLE_BG, th->style.ta.area);
lv_ta_set_style(new_ta, LV_TA_STYLE_SB, th->style.ta.sb);
} else {
lv_ta_set_style(new_ta, LV_TA_STYLE_BG, &lv_style_pretty);
}
}
else {
lv_obj_set_design_cb(ext->page.scrl, lv_ta_scrollable_design);
lv_ta_ext_t * copy_ext = lv_obj_get_ext_attr(copy);
ext->label = lv_label_create(new_ta, copy_ext->label);
ext->pwd_mode = copy_ext->pwd_mode;
ext->accapted_chars = copy_ext->accapted_chars;
ext->max_length = copy_ext->max_length;
ext->cursor.style = copy_ext->cursor.style;
ext->cursor.pos = copy_ext->cursor.pos;
ext->cursor.valid_x = copy_ext->cursor.valid_x;
ext->cursor.type = copy_ext->cursor.type;
if(ext->pwd_mode != 0) pwd_char_hider( new_ta);
if(copy_ext->placeholder != NULL)
ext->placeholder = lv_label_create(new_ta, copy_ext->placeholder);
else
ext->placeholder = NULL;
if(copy_ext->pwd_tmp) {
uint16_t len = lv_mem_get_size(copy_ext->pwd_tmp);
ext->pwd_tmp = lv_mem_alloc(len);
LV_ASSERT_MEM(ext->pwd_tmp);
if(ext->pwd_tmp == NULL) return NULL;
memcpy(ext->pwd_tmp, copy_ext->pwd_tmp, len);
}
if(copy_ext->one_line) lv_ta_set_one_line(new_ta, true);
lv_ta_set_style(new_ta, LV_TA_STYLE_CURSOR, lv_ta_get_style(copy, LV_TA_STYLE_CURSOR));
lv_obj_refresh_style(new_ta);
}
#if LV_USE_ANIMATION
if(ext->cursor.blink_time) {
lv_anim_t a;
a.var = new_ta;
a.exec_cb = (lv_anim_exec_xcb_t)cursor_blink_anim;
a.time = ext->cursor.blink_time;
a.act_time = 0;
a.ready_cb = NULL;
a.start = 1;
a.end = 0;
a.repeat = 1;
a.repeat_pause = 0;
a.playback = 1;
a.playback_pause = 0;
a.path_cb = lv_anim_path_step;
lv_anim_create(&a);
}
#endif
LV_LOG_INFO("text area created");
return new_ta;
}
void lv_ta_add_char(lv_obj_t * ta, uint32_t c)
{
LV_ASSERT_OBJ(ta, LV_OBJX_NAME);
lv_ta_ext_t * ext = lv_obj_get_ext_attr(ta);
uint32_t letter_buf[2];
letter_buf[0] = c;
letter_buf[1] = '\0';
ta_insert_replace = NULL;
lv_event_send(ta, LV_EVENT_INSERT, letter_buf);
if(ta_insert_replace) {
if(ta_insert_replace[0] == '\0') return; 
if(strcmp(ta_insert_replace, (char *)letter_buf)) {
lv_ta_add_text(ta, ta_insert_replace);
return;
}
}
if(ext->one_line && (c == '\n' || c == '\r')) {
LV_LOG_INFO("Text area: line break ignored in one-line mode");
return;
}
uint32_t c_uni = lv_txt_encoded_next((const char *)&c, NULL);
if(char_is_accepted(ta, c_uni) == false) {
LV_LOG_INFO("Character is no accepted by the text area (too long text or not in the "
"accepted list)");
return;
}
bool edge_flash_en = lv_ta_get_edge_flash(ta);
lv_ta_set_edge_flash(ta, false);
if(ext->pwd_mode != 0) pwd_char_hider(ta); 
lv_label_ins_text(ext->label, ext->cursor.pos, (const char *)letter_buf); 
lv_ta_clear_selection(ta); 
if(ext->pwd_mode != 0) {
ext->pwd_tmp = lv_mem_realloc(ext->pwd_tmp, strlen(ext->pwd_tmp) + 2); 
LV_ASSERT_MEM(ext->pwd_tmp);
if(ext->pwd_tmp == NULL) return;
lv_txt_ins(ext->pwd_tmp, ext->cursor.pos, (const char *)letter_buf);
#if LV_USE_ANIMATION
lv_anim_t a;
a.var = ta;
a.exec_cb = (lv_anim_exec_xcb_t)pwd_char_hider_anim;
a.time = ext->pwd_show_time;
a.act_time = 0;
a.ready_cb = pwd_char_hider_anim_ready;
a.start = 0;
a.end = 1;
a.repeat = 0;
a.repeat_pause = 0;
a.playback = 0;
a.playback_pause = 0;
a.path_cb = lv_anim_path_step;
lv_anim_create(&a);
#else
pwd_char_hider(ta);
#endif
}
lv_ta_set_cursor_pos(ta, lv_ta_get_cursor_pos(ta) + 1);
lv_ta_set_edge_flash(ta, edge_flash_en);
placeholder_update(ta);
lv_event_send(ta, LV_EVENT_VALUE_CHANGED, NULL);
}
void lv_ta_add_text(lv_obj_t * ta, const char * txt)
{
LV_ASSERT_OBJ(ta, LV_OBJX_NAME);
LV_ASSERT_NULL(txt);
lv_ta_ext_t * ext = lv_obj_get_ext_attr(ta);
ta_insert_replace = NULL;
lv_event_send(ta, LV_EVENT_INSERT, txt);
if(ta_insert_replace) {
if(ta_insert_replace[0] == '\0') return; 
if(strcmp(ta_insert_replace, txt)) {
lv_ta_add_text(ta, ta_insert_replace);
return;
}
}
if(ext->pwd_mode != 0) pwd_char_hider(ta); 
if(lv_ta_get_accepted_chars(ta) || lv_ta_get_max_length(ta)) {
uint32_t i = 0;
while(txt[i] != '\0') {
uint32_t c = lv_txt_encoded_next(txt, &i);
lv_ta_add_char(ta, lv_txt_unicode_to_encoded(c));
}
return;
}
bool edge_flash_en = lv_ta_get_edge_flash(ta);
lv_ta_set_edge_flash(ta, false);
lv_label_ins_text(ext->label, ext->cursor.pos, txt);
lv_ta_clear_selection(ta);
if(ext->pwd_mode != 0) {
ext->pwd_tmp = lv_mem_realloc(ext->pwd_tmp, strlen(ext->pwd_tmp) + strlen(txt) + 1);
LV_ASSERT_MEM(ext->pwd_tmp);
if(ext->pwd_tmp == NULL) return;
lv_txt_ins(ext->pwd_tmp, ext->cursor.pos, txt);
#if LV_USE_ANIMATION
lv_anim_t a;
a.var = ta;
a.exec_cb = (lv_anim_exec_xcb_t)pwd_char_hider_anim;
a.time = ext->pwd_show_time;
a.act_time = 0;
a.ready_cb = pwd_char_hider_anim_ready;
a.start = 0;
a.end = 1;
a.repeat = 0;
a.repeat_pause = 0;
a.playback = 0;
a.playback_pause = 0;
a.path_cb = lv_anim_path_step;
lv_anim_create(&a);
#else
pwd_char_hider(ta);
#endif
}
lv_ta_set_cursor_pos(ta, lv_ta_get_cursor_pos(ta) + lv_txt_get_encoded_length(txt));
lv_ta_set_edge_flash(ta, edge_flash_en);
placeholder_update(ta);
lv_event_send(ta, LV_EVENT_VALUE_CHANGED, NULL);
}
void lv_ta_del_char(lv_obj_t * ta)
{
LV_ASSERT_OBJ(ta, LV_OBJX_NAME);
lv_ta_ext_t * ext = lv_obj_get_ext_attr(ta);
uint16_t cur_pos = ext->cursor.pos;
if(cur_pos == 0) return;
ta_insert_replace = NULL;
char del_buf[2] = {LV_KEY_DEL, '\0'};
lv_event_send(ta, LV_EVENT_INSERT, del_buf);
if(ta_insert_replace) {
if(ta_insert_replace[0] == '\0') return; 
if(strcmp(ta_insert_replace, del_buf)) {
lv_ta_add_text(ta, ta_insert_replace);
return;
}
}
char * label_txt = lv_label_get_text(ext->label);
lv_txt_cut(label_txt, ext->cursor.pos - 1, 1);
lv_label_set_text(ext->label, label_txt);
lv_ta_clear_selection(ta);
if(lv_obj_get_width(ext->label) == 0) {
const lv_style_t * style = lv_obj_get_style(ext->label);
lv_obj_set_width(ext->label, style->line.width);
}
if(ext->pwd_mode != 0) {
uint32_t byte_pos = lv_txt_encoded_get_byte_id(ext->pwd_tmp, ext->cursor.pos - 1);
lv_txt_cut(ext->pwd_tmp, ext->cursor.pos - 1, lv_txt_encoded_size(&label_txt[byte_pos]));
ext->pwd_tmp = lv_mem_realloc(ext->pwd_tmp, strlen(ext->pwd_tmp) + 1);
LV_ASSERT_MEM(ext->pwd_tmp);
if(ext->pwd_tmp == NULL) return;
}
lv_ta_set_cursor_pos(ta, ext->cursor.pos - 1);
placeholder_update(ta);
lv_event_send(ta, LV_EVENT_VALUE_CHANGED, NULL);
}
void lv_ta_del_char_forward(lv_obj_t * ta)
{
LV_ASSERT_OBJ(ta, LV_OBJX_NAME);
uint16_t cp = lv_ta_get_cursor_pos(ta);
lv_ta_set_cursor_pos(ta, cp + 1);
if(cp != lv_ta_get_cursor_pos(ta)) lv_ta_del_char(ta);
}
void lv_ta_set_text(lv_obj_t * ta, const char * txt)
{
LV_ASSERT_OBJ(ta, LV_OBJX_NAME);
LV_ASSERT_NULL(txt);
lv_ta_ext_t * ext = lv_obj_get_ext_attr(ta);
lv_ta_clear_selection(ta);
if(lv_ta_get_accepted_chars(ta) || lv_ta_get_max_length(ta)) {
lv_label_set_text(ext->label, "");
lv_ta_set_cursor_pos(ta, LV_TA_CURSOR_LAST);
if(ext->pwd_mode != 0) {
ext->pwd_tmp[0] = '\0'; 
}
uint32_t i = 0;
while(txt[i] != '\0') {
uint32_t c = lv_txt_encoded_next(txt, &i);
lv_ta_add_char(ta, lv_txt_unicode_to_encoded(c));
}
} else {
lv_label_set_text(ext->label, txt);
lv_ta_set_cursor_pos(ta, LV_TA_CURSOR_LAST);
}
if(lv_obj_get_width(ext->label) == 0) {
const lv_style_t * style = lv_obj_get_style(ext->label);
lv_obj_set_width(ext->label, lv_font_get_glyph_width(style->text.font, ' ', '\0'));
}
if(ext->pwd_mode != 0) {
ext->pwd_tmp = lv_mem_realloc(ext->pwd_tmp, strlen(txt) + 1);
LV_ASSERT_MEM(ext->pwd_tmp);
if(ext->pwd_tmp == NULL) return;
strcpy(ext->pwd_tmp, txt);
#if LV_USE_ANIMATION
lv_anim_t a;
a.var = ta;
a.exec_cb = (lv_anim_exec_xcb_t)pwd_char_hider_anim;
a.time = ext->pwd_show_time;
a.act_time = 0;
a.ready_cb = pwd_char_hider_anim_ready;
a.start = 0;
a.end = 1;
a.repeat = 0;
a.repeat_pause = 0;
a.playback = 0;
a.playback_pause = 0;
a.path_cb = lv_anim_path_step;
lv_anim_create(&a);
#else
pwd_char_hider(ta);
#endif
}
placeholder_update(ta);
lv_event_send(ta, LV_EVENT_VALUE_CHANGED, NULL);
}
void lv_ta_set_placeholder_text(lv_obj_t * ta, const char * txt)
{
LV_ASSERT_OBJ(ta, LV_OBJX_NAME);
LV_ASSERT_NULL(txt);
lv_ta_ext_t * ext = lv_obj_get_ext_attr(ta);
if(ext->placeholder == NULL) {
ext->placeholder = lv_label_create(ta, NULL);
if(ext->one_line) {
lv_label_set_long_mode(ext->placeholder, LV_LABEL_LONG_EXPAND);
} else {
lv_label_set_long_mode(ext->placeholder, LV_LABEL_LONG_BREAK);
}
}
lv_label_set_text(ext->placeholder, txt);
lv_ta_set_text_align(ta, lv_label_get_align(ext->label));
placeholder_update(ta);
}
void lv_ta_set_cursor_pos(lv_obj_t * ta, int16_t pos)
{
LV_ASSERT_OBJ(ta, LV_OBJX_NAME);
lv_ta_ext_t * ext = lv_obj_get_ext_attr(ta);
if(ext->cursor.pos == pos) return;
uint16_t len = lv_txt_get_encoded_length(lv_label_get_text(ext->label));
if(pos < 0) pos = len + pos;
if(pos > len || pos == LV_TA_CURSOR_LAST) pos = len;
ext->cursor.pos = pos;
lv_obj_t * label_par = lv_obj_get_parent(ext->label);
lv_point_t cur_pos;
const lv_style_t * style = lv_obj_get_style(ta);
const lv_font_t * font_p = style->text.font;
lv_area_t label_cords;
lv_area_t ta_cords;
lv_label_get_letter_pos(ext->label, pos, &cur_pos);
lv_obj_get_coords(ta, &ta_cords);
lv_obj_get_coords(ext->label, &label_cords);
lv_coord_t font_h = lv_font_get_line_height(font_p);
if(lv_obj_get_y(label_par) + cur_pos.y < 0) {
lv_obj_set_y(label_par, -cur_pos.y + style->body.padding.top);
}
if(label_cords.y1 + cur_pos.y + font_h + style->body.padding.bottom > ta_cords.y2) {
lv_obj_set_y(label_par, -(cur_pos.y - lv_obj_get_height(ta) + font_h + style->body.padding.top +
style->body.padding.bottom));
}
if(lv_obj_get_x(label_par) + cur_pos.x < font_h) {
lv_obj_set_x(label_par, -cur_pos.x + font_h);
}
if(label_cords.x1 + cur_pos.x + font_h + style->body.padding.right > ta_cords.x2) {
lv_obj_set_x(label_par, -(cur_pos.x - lv_obj_get_width(ta) + font_h + style->body.padding.left +
style->body.padding.right));
}
ext->cursor.valid_x = cur_pos.x;
#if LV_USE_ANIMATION
if(ext->cursor.blink_time) {
lv_anim_t a;
a.var = ta;
a.exec_cb = (lv_anim_exec_xcb_t)cursor_blink_anim;
a.time = ext->cursor.blink_time;
a.act_time = 0;
a.ready_cb = NULL;
a.start = 1;
a.end = 0;
a.repeat = 1;
a.repeat_pause = 0;
a.playback = 1;
a.playback_pause = 0;
a.path_cb = lv_anim_path_step;
lv_anim_create(&a);
}
#endif
refr_cursor_area(ta);
}
void lv_ta_set_cursor_type(lv_obj_t * ta, lv_cursor_type_t cur_type)
{
LV_ASSERT_OBJ(ta, LV_OBJX_NAME);
lv_ta_ext_t * ext = lv_obj_get_ext_attr(ta);
if(ext->cursor.type == cur_type) return;
ext->cursor.type = cur_type;
refr_cursor_area(ta);
}
void lv_ta_set_cursor_click_pos(lv_obj_t * ta, bool en)
{
LV_ASSERT_OBJ(ta, LV_OBJX_NAME);
lv_ta_ext_t * ext = lv_obj_get_ext_attr(ta);
ext->cursor.click_pos = en ? 1 : 0;
}
void lv_ta_set_pwd_mode(lv_obj_t * ta, bool en)
{
LV_ASSERT_OBJ(ta, LV_OBJX_NAME);
lv_ta_ext_t * ext = lv_obj_get_ext_attr(ta);
if(ext->pwd_mode == en) return;
if(ext->pwd_mode == 0 && en != false) {
char * txt = lv_label_get_text(ext->label);
size_t len = strlen(txt);
ext->pwd_tmp = lv_mem_alloc(len + 1);
LV_ASSERT_MEM(ext->pwd_tmp);
if(ext->pwd_tmp == NULL) return;
strcpy(ext->pwd_tmp, txt);
uint16_t i;
uint16_t encoded_len = lv_txt_get_encoded_length(txt); 
for(i = 0; i < encoded_len; i++) {
txt[i] = '*'; 
}
txt[i] = '\0';
lv_ta_clear_selection(ta);
lv_label_set_text(ext->label, NULL);
}
else if(ext->pwd_mode == 1 && en == false) {
lv_ta_clear_selection(ta);
lv_label_set_text(ext->label, ext->pwd_tmp);
lv_mem_free(ext->pwd_tmp);
ext->pwd_tmp = NULL;
}
ext->pwd_mode = en == false ? 0 : 1;
refr_cursor_area(ta);
}
void lv_ta_set_one_line(lv_obj_t * ta, bool en)
{
LV_ASSERT_OBJ(ta, LV_OBJX_NAME);
lv_ta_ext_t * ext = lv_obj_get_ext_attr(ta);
if(ext->one_line == en) return;
lv_label_align_t old_align = lv_label_get_align(ext->label);
if(en) {
const lv_style_t * style_ta = lv_obj_get_style(ta);
const lv_style_t * style_scrl = lv_obj_get_style(lv_page_get_scrl(ta));
const lv_style_t * style_label = lv_obj_get_style(ext->label);
lv_coord_t font_h = lv_font_get_line_height(style_label->text.font);
ext->one_line = 1;
lv_page_set_scrl_fit2(ta, LV_FIT_TIGHT, LV_FIT_FLOOD);
lv_obj_set_height(ta, font_h + style_ta->body.padding.top + style_ta->body.padding.bottom +
style_scrl->body.padding.top + style_scrl->body.padding.bottom);
lv_label_set_long_mode(ext->label, LV_LABEL_LONG_EXPAND);
if(ext->placeholder) lv_label_set_long_mode(ext->placeholder, LV_LABEL_LONG_EXPAND);
lv_obj_set_pos(lv_page_get_scrl(ta), style_ta->body.padding.left, style_ta->body.padding.top);
} else {
const lv_style_t * style_ta = lv_obj_get_style(ta);
ext->one_line = 0;
lv_page_set_scrl_fit2(ta, LV_FIT_FLOOD, LV_FIT_TIGHT);
lv_label_set_long_mode(ext->label, LV_LABEL_LONG_BREAK);
if(ext->placeholder) lv_label_set_long_mode(ext->placeholder, LV_LABEL_LONG_BREAK);
lv_obj_set_height(ta, LV_TA_DEF_HEIGHT);
lv_obj_set_pos(lv_page_get_scrl(ta), style_ta->body.padding.left, style_ta->body.padding.top);
}
placeholder_update(ta);
lv_ta_set_text_align(ta, old_align);
}
void lv_ta_set_text_align(lv_obj_t * ta, lv_label_align_t align)
{
LV_ASSERT_OBJ(ta, LV_OBJX_NAME);
lv_ta_ext_t * ext = lv_obj_get_ext_attr(ta);
lv_obj_t * label = lv_ta_get_label(ta);
if(!ext->one_line) {
lv_label_set_align(label, align);
if(ext->placeholder) lv_label_set_align(ext->placeholder, align);
} else {
if(align == LV_LABEL_ALIGN_LEFT) {
lv_label_set_long_mode(label, LV_LABEL_LONG_EXPAND);
lv_page_set_scrl_fit2(ta, LV_FIT_TIGHT, LV_FIT_FLOOD);
lv_label_set_align(label, align);
if(ext->placeholder) lv_label_set_align(ext->placeholder, align);
}
else {
lv_label_set_long_mode(label, LV_LABEL_LONG_CROP);
lv_page_set_scrl_fit2(ta, LV_FIT_FLOOD, LV_FIT_FLOOD);
lv_label_set_align(label, align);
if(ext->placeholder) lv_label_set_align(ext->placeholder, align);
lv_obj_set_width(label, lv_page_get_fit_width(ta));
}
}
refr_cursor_area(ta);
}
void lv_ta_set_accepted_chars(lv_obj_t * ta, const char * list)
{
LV_ASSERT_OBJ(ta, LV_OBJX_NAME);
lv_ta_ext_t * ext = lv_obj_get_ext_attr(ta);
ext->accapted_chars = list;
}
void lv_ta_set_max_length(lv_obj_t * ta, uint16_t num)
{
LV_ASSERT_OBJ(ta, LV_OBJX_NAME);
lv_ta_ext_t * ext = lv_obj_get_ext_attr(ta);
ext->max_length = num;
}
void lv_ta_set_insert_replace(lv_obj_t * ta, const char * txt)
{
LV_ASSERT_OBJ(ta, LV_OBJX_NAME);
(void)ta; 
ta_insert_replace = txt;
}
void lv_ta_set_style(lv_obj_t * ta, lv_ta_style_t type, const lv_style_t * style)
{
LV_ASSERT_OBJ(ta, LV_OBJX_NAME);
lv_ta_ext_t * ext = lv_obj_get_ext_attr(ta);
switch(type) {
case LV_TA_STYLE_BG: lv_page_set_style(ta, LV_PAGE_STYLE_BG, style); break;
case LV_TA_STYLE_SB: lv_page_set_style(ta, LV_PAGE_STYLE_SB, style); break;
case LV_TA_STYLE_EDGE_FLASH: lv_page_set_style(ta, LV_PAGE_STYLE_EDGE_FLASH, style); break;
case LV_TA_STYLE_CURSOR:
ext->cursor.style = style;
lv_obj_refresh_ext_draw_pad(lv_page_get_scrl(ta)); 
refr_cursor_area(ta);
break;
case LV_TA_STYLE_PLACEHOLDER:
if(ext->placeholder) lv_label_set_style(ext->placeholder, LV_LABEL_STYLE_MAIN, style);
break;
}
}
void lv_ta_set_text_sel(lv_obj_t * ta, bool en)
{
LV_ASSERT_OBJ(ta, LV_OBJX_NAME);
#if LV_LABEL_TEXT_SEL
lv_ta_ext_t * ext = lv_obj_get_ext_attr(ta);
ext->text_sel_en = en;
if(!en) lv_ta_clear_selection(ta);
#else
(void)ta; 
(void)en; 
#endif
}
void lv_ta_set_pwd_show_time(lv_obj_t * ta, uint16_t time)
{
LV_ASSERT_OBJ(ta, LV_OBJX_NAME);
#if LV_USE_ANIMATION == 0
time = 0;
#endif
lv_ta_ext_t * ext = lv_obj_get_ext_attr(ta);
ext->pwd_show_time = time;
}
void lv_ta_set_cursor_blink_time(lv_obj_t * ta, uint16_t time)
{
LV_ASSERT_OBJ(ta, LV_OBJX_NAME);
#if LV_USE_ANIMATION == 0
time = 0;
#endif
lv_ta_ext_t * ext = lv_obj_get_ext_attr(ta);
ext->cursor.blink_time = time;
#if LV_USE_ANIMATION
if(ext->cursor.blink_time) {
lv_anim_t a;
a.var = ta;
a.exec_cb = (lv_anim_exec_xcb_t)cursor_blink_anim;
a.time = ext->cursor.blink_time;
a.act_time = 0;
a.ready_cb = NULL;
a.start = 1;
a.end = 0;
a.repeat = 1;
a.repeat_pause = 0;
a.playback = 1;
a.playback_pause = 0;
a.path_cb = lv_anim_path_step;
lv_anim_create(&a);
} else {
lv_anim_del(ta, (lv_anim_exec_xcb_t)cursor_blink_anim);
ext->cursor.state = 1;
}
#else
ext->cursor.state = 1;
#endif
}
const char * lv_ta_get_text(const lv_obj_t * ta)
{
LV_ASSERT_OBJ(ta, LV_OBJX_NAME);
lv_ta_ext_t * ext = lv_obj_get_ext_attr(ta);
const char * txt;
if(ext->pwd_mode == 0) {
txt = lv_label_get_text(ext->label);
} else {
txt = ext->pwd_tmp;
}
return txt;
}
const char * lv_ta_get_placeholder_text(lv_obj_t * ta)
{
LV_ASSERT_OBJ(ta, LV_OBJX_NAME);
lv_ta_ext_t * ext = lv_obj_get_ext_attr(ta);
const char * txt = NULL;
if(ext->placeholder) txt = lv_label_get_text(ext->placeholder);
return txt;
}
lv_obj_t * lv_ta_get_label(const lv_obj_t * ta)
{
LV_ASSERT_OBJ(ta, LV_OBJX_NAME);
lv_ta_ext_t * ext = lv_obj_get_ext_attr(ta);
return ext->label;
}
uint16_t lv_ta_get_cursor_pos(const lv_obj_t * ta)
{
LV_ASSERT_OBJ(ta, LV_OBJX_NAME);
lv_ta_ext_t * ext = lv_obj_get_ext_attr(ta);
return ext->cursor.pos;
}
lv_cursor_type_t lv_ta_get_cursor_type(const lv_obj_t * ta)
{
LV_ASSERT_OBJ(ta, LV_OBJX_NAME);
lv_ta_ext_t * ext = lv_obj_get_ext_attr(ta);
return ext->cursor.type;
}
bool lv_ta_get_cursor_click_pos(lv_obj_t * ta)
{
LV_ASSERT_OBJ(ta, LV_OBJX_NAME);
lv_ta_ext_t * ext = lv_obj_get_ext_attr(ta);
return ext->cursor.click_pos ? true : false;
}
bool lv_ta_get_pwd_mode(const lv_obj_t * ta)
{
LV_ASSERT_OBJ(ta, LV_OBJX_NAME);
lv_ta_ext_t * ext = lv_obj_get_ext_attr(ta);
return ext->pwd_mode == 0 ? false : true;
}
bool lv_ta_get_one_line(const lv_obj_t * ta)
{
LV_ASSERT_OBJ(ta, LV_OBJX_NAME);
lv_ta_ext_t * ext = lv_obj_get_ext_attr(ta);
return ext->one_line == 0 ? false : true;
}
const char * lv_ta_get_accepted_chars(lv_obj_t * ta)
{
LV_ASSERT_OBJ(ta, LV_OBJX_NAME);
lv_ta_ext_t * ext = lv_obj_get_ext_attr(ta);
return ext->accapted_chars;
}
uint16_t lv_ta_get_max_length(lv_obj_t * ta)
{
LV_ASSERT_OBJ(ta, LV_OBJX_NAME);
lv_ta_ext_t * ext = lv_obj_get_ext_attr(ta);
return ext->max_length;
}
const lv_style_t * lv_ta_get_style(const lv_obj_t * ta, lv_ta_style_t type)
{
LV_ASSERT_OBJ(ta, LV_OBJX_NAME);
const lv_style_t * style = NULL;
lv_ta_ext_t * ext = lv_obj_get_ext_attr(ta);
switch(type) {
case LV_TA_STYLE_BG: style = lv_page_get_style(ta, LV_PAGE_STYLE_BG); break;
case LV_TA_STYLE_SB: style = lv_page_get_style(ta, LV_PAGE_STYLE_SB); break;
case LV_TA_STYLE_EDGE_FLASH: style = lv_page_get_style(ta, LV_PAGE_STYLE_EDGE_FLASH); break;
case LV_TA_STYLE_CURSOR: style = ext->cursor.style; break;
case LV_TA_STYLE_PLACEHOLDER:
if(ext->placeholder) style = lv_label_get_style(ext->placeholder, LV_LABEL_STYLE_MAIN);
break;
default: style = NULL; break;
}
return style;
}
bool lv_ta_text_is_selected(const lv_obj_t * ta)
{
LV_ASSERT_OBJ(ta, LV_OBJX_NAME);
#if LV_LABEL_TEXT_SEL
lv_ta_ext_t * ext = lv_obj_get_ext_attr(ta);
if((lv_label_get_text_sel_start(ext->label) == LV_DRAW_LABEL_NO_TXT_SEL ||
lv_label_get_text_sel_end(ext->label) == LV_DRAW_LABEL_NO_TXT_SEL)) {
return true;
} else {
return false;
}
#else
(void)ta; 
return false;
#endif
}
bool lv_ta_get_text_sel_en(lv_obj_t * ta)
{
LV_ASSERT_OBJ(ta, LV_OBJX_NAME);
#if LV_LABEL_TEXT_SEL
lv_ta_ext_t * ext = lv_obj_get_ext_attr(ta);
return ext->text_sel_en;
#else
(void)ta; 
return false;
#endif
}
uint16_t lv_ta_get_pwd_show_time(lv_obj_t * ta)
{
LV_ASSERT_OBJ(ta, LV_OBJX_NAME);
lv_ta_ext_t * ext = lv_obj_get_ext_attr(ta);
return ext->pwd_show_time;
}
uint16_t lv_ta_get_cursor_blink_time(lv_obj_t * ta)
{
LV_ASSERT_OBJ(ta, LV_OBJX_NAME);
lv_ta_ext_t * ext = lv_obj_get_ext_attr(ta);
return ext->cursor.blink_time;
}
void lv_ta_clear_selection(lv_obj_t * ta)
{
LV_ASSERT_OBJ(ta, LV_OBJX_NAME);
#if LV_LABEL_TEXT_SEL
lv_ta_ext_t * ext = lv_obj_get_ext_attr(ta);
if(lv_label_get_text_sel_start(ext->label) != LV_DRAW_LABEL_NO_TXT_SEL ||
lv_label_get_text_sel_end(ext->label) != LV_DRAW_LABEL_NO_TXT_SEL) {
lv_label_set_text_sel_start(ext->label, LV_DRAW_LABEL_NO_TXT_SEL);
lv_label_set_text_sel_end(ext->label, LV_DRAW_LABEL_NO_TXT_SEL);
}
#else
(void)ta; 
#endif
}
void lv_ta_cursor_right(lv_obj_t * ta)
{
LV_ASSERT_OBJ(ta, LV_OBJX_NAME);
uint16_t cp = lv_ta_get_cursor_pos(ta);
cp++;
lv_ta_set_cursor_pos(ta, cp);
}
void lv_ta_cursor_left(lv_obj_t * ta)
{
LV_ASSERT_OBJ(ta, LV_OBJX_NAME);
uint16_t cp = lv_ta_get_cursor_pos(ta);
if(cp > 0) {
cp--;
lv_ta_set_cursor_pos(ta, cp);
}
}
void lv_ta_cursor_down(lv_obj_t * ta)
{
LV_ASSERT_OBJ(ta, LV_OBJX_NAME);
lv_ta_ext_t * ext = lv_obj_get_ext_attr(ta);
lv_point_t pos;
lv_label_get_letter_pos(ext->label, lv_ta_get_cursor_pos(ta), &pos);
const lv_style_t * label_style = lv_obj_get_style(ext->label);
const lv_font_t * font_p = label_style->text.font;
lv_coord_t font_h = lv_font_get_line_height(font_p);
pos.y += font_h + label_style->text.line_space + 1;
pos.x = ext->cursor.valid_x;
if(pos.y < lv_obj_get_height(ext->label)) {
uint16_t new_cur_pos = lv_label_get_letter_on(ext->label, &pos);
lv_coord_t cur_valid_x_tmp = ext->cursor.valid_x; 
lv_ta_set_cursor_pos(ta, new_cur_pos);
ext->cursor.valid_x = cur_valid_x_tmp;
}
}
void lv_ta_cursor_up(lv_obj_t * ta)
{
LV_ASSERT_OBJ(ta, LV_OBJX_NAME);
lv_ta_ext_t * ext = lv_obj_get_ext_attr(ta);
lv_point_t pos;
lv_label_get_letter_pos(ext->label, lv_ta_get_cursor_pos(ta), &pos);
const lv_style_t * label_style = lv_obj_get_style(ext->label);
const lv_font_t * font = label_style->text.font;
lv_coord_t font_h = lv_font_get_line_height(font);
pos.y -= font_h + label_style->text.line_space - 1;
pos.x = ext->cursor.valid_x;
uint16_t new_cur_pos = lv_label_get_letter_on(ext->label, &pos);
lv_coord_t cur_valid_x_tmp = ext->cursor.valid_x; 
lv_ta_set_cursor_pos(ta, new_cur_pos);
ext->cursor.valid_x = cur_valid_x_tmp;
}
static bool lv_ta_design(lv_obj_t * ta, const lv_area_t * mask, lv_design_mode_t mode)
{
if(mode == LV_DESIGN_COVER_CHK) {
return ancestor_design(ta, mask, mode);
} else if(mode == LV_DESIGN_DRAW_MAIN) {
ancestor_design(ta, mask, mode);
} else if(mode == LV_DESIGN_DRAW_POST) {
ancestor_design(ta, mask, mode);
}
return true;
}
static bool lv_ta_scrollable_design(lv_obj_t * scrl, const lv_area_t * mask, lv_design_mode_t mode)
{
if(mode == LV_DESIGN_COVER_CHK) {
return scrl_design(scrl, mask, mode);
} else if(mode == LV_DESIGN_DRAW_MAIN) {
scrl_design(scrl, mask, mode);
} else if(mode == LV_DESIGN_DRAW_POST) {
scrl_design(scrl, mask, mode);
lv_obj_t * ta = lv_obj_get_parent(scrl);
lv_ta_ext_t * ext = lv_obj_get_ext_attr(ta);
if(ext->cursor.type == LV_CURSOR_NONE || (ext->cursor.type & LV_CURSOR_HIDDEN) || ext->cursor.state == 0) {
return true; 
}
lv_style_t cur_style;
get_cursor_style(ta, &cur_style);
const char * txt = lv_label_get_text(ext->label);
lv_area_t cur_area;
lv_area_copy(&cur_area, &ext->cursor.area);
cur_area.x1 += ext->label->coords.x1;
cur_area.y1 += ext->label->coords.y1;
cur_area.x2 += ext->label->coords.x1;
cur_area.y2 += ext->label->coords.y1;
lv_opa_t opa_scale = lv_obj_get_opa_scale(ta);
if(ext->cursor.type == LV_CURSOR_LINE) {
lv_draw_rect(&cur_area, mask, &cur_style, opa_scale);
} else if(ext->cursor.type == LV_CURSOR_BLOCK) {
lv_draw_rect(&cur_area, mask, &cur_style, opa_scale);
char letter_buf[8] = {0};
memcpy(letter_buf, &txt[ext->cursor.txt_byte_pos], lv_txt_encoded_size(&txt[ext->cursor.txt_byte_pos]));
cur_area.x1 += cur_style.body.padding.left;
cur_area.y1 += cur_style.body.padding.top;
lv_draw_label(&cur_area, mask, &cur_style, opa_scale, letter_buf, LV_TXT_FLAG_NONE, NULL, NULL, NULL, lv_obj_get_base_dir(ta));
} else if(ext->cursor.type == LV_CURSOR_OUTLINE) {
cur_style.body.opa = LV_OPA_TRANSP;
if(cur_style.body.border.width == 0) cur_style.body.border.width = 1; 
lv_draw_rect(&cur_area, mask, &cur_style, opa_scale);
} else if(ext->cursor.type == LV_CURSOR_UNDERLINE) {
lv_draw_rect(&cur_area, mask, &cur_style, opa_scale);
}
}
return true;
}
static lv_res_t lv_ta_signal(lv_obj_t * ta, lv_signal_t sign, void * param)
{
lv_res_t res;
res = ancestor_signal(ta, sign, param);
if(res != LV_RES_OK) return res;
if(sign == LV_SIGNAL_GET_TYPE) return lv_obj_handle_get_type_signal(param, LV_OBJX_NAME);
lv_ta_ext_t * ext = lv_obj_get_ext_attr(ta);
if(sign == LV_SIGNAL_CLEANUP) {
if(ext->pwd_tmp != NULL) lv_mem_free(ext->pwd_tmp);
} else if(sign == LV_SIGNAL_STYLE_CHG) {
if(ext->label) {
lv_obj_t * scrl = lv_page_get_scrl(ta);
const lv_style_t * style_ta = lv_obj_get_style(ta);
const lv_style_t * style_scrl = lv_obj_get_style(scrl);
if(ext->one_line) {
const lv_style_t * style_label = lv_obj_get_style(ext->label);
lv_coord_t font_h = lv_font_get_line_height(style_label->text.font);
lv_obj_set_height(ext->label, font_h);
lv_obj_set_height(ta, font_h + style_ta->body.padding.top + style_ta->body.padding.bottom +
style_scrl->body.padding.top + style_scrl->body.padding.bottom);
} else {
lv_obj_set_width(ext->label, lv_page_get_fit_width(ta));
lv_obj_set_pos(ext->label, style_scrl->body.padding.left,
style_scrl->body.padding.right); 
if(ext->placeholder) {
lv_obj_set_width(ext->placeholder, lv_page_get_fit_width(ta));
lv_obj_set_pos(ext->placeholder, style_scrl->body.padding.left,
style_scrl->body.padding.top); 
}
}
lv_label_set_text(ext->label, NULL);
}
} else if(sign == LV_SIGNAL_CORD_CHG) {
if(ext->label) {
if(lv_obj_get_width(ta) != lv_area_get_width(param) || lv_obj_get_height(ta) != lv_area_get_height(param)) {
lv_obj_t * scrl = lv_page_get_scrl(ta);
const lv_style_t * style_scrl = lv_obj_get_style(scrl);
lv_obj_set_width(ext->label, lv_page_get_fit_width(ta));
lv_obj_set_pos(ext->label, style_scrl->body.padding.left, style_scrl->body.padding.top);
lv_label_set_text(ext->label, NULL); 
refr_cursor_area(ta);
}
}
if(ext->placeholder) {
if(lv_obj_get_width(ta) != lv_area_get_width(param) || lv_obj_get_height(ta) != lv_area_get_height(param)) {
lv_obj_t * scrl = lv_page_get_scrl(ta);
const lv_style_t * style_scrl = lv_obj_get_style(scrl);
lv_obj_set_width(ext->placeholder, lv_page_get_fit_width(ta));
lv_obj_set_pos(ext->placeholder, style_scrl->body.padding.left, style_scrl->body.padding.top);
lv_label_set_text(ext->placeholder, NULL); 
refr_cursor_area(ta);
}
}
} else if(sign == LV_SIGNAL_CONTROL) {
uint32_t c = *((uint32_t *)param); 
if(c == LV_KEY_RIGHT)
lv_ta_cursor_right(ta);
else if(c == LV_KEY_LEFT)
lv_ta_cursor_left(ta);
else if(c == LV_KEY_UP)
lv_ta_cursor_up(ta);
else if(c == LV_KEY_DOWN)
lv_ta_cursor_down(ta);
else if(c == LV_KEY_BACKSPACE)
lv_ta_del_char(ta);
else if(c == LV_KEY_DEL)
lv_ta_del_char_forward(ta);
else if(c == LV_KEY_HOME)
lv_ta_set_cursor_pos(ta, 0);
else if(c == LV_KEY_END)
lv_ta_set_cursor_pos(ta, LV_TA_CURSOR_LAST);
else {
lv_ta_add_char(ta, c);
}
} else if(sign == LV_SIGNAL_GET_EDITABLE) {
bool * editable = (bool *)param;
*editable = true;
} else if(sign == LV_SIGNAL_DEFOCUS) {
lv_cursor_type_t cur_type;
cur_type = lv_ta_get_cursor_type(ta);
lv_ta_set_cursor_type(ta, cur_type | LV_CURSOR_HIDDEN);
} else if(sign == LV_SIGNAL_FOCUS) {
#if LV_USE_GROUP
lv_cursor_type_t cur_type;
cur_type = lv_ta_get_cursor_type(ta);
lv_group_t * g = lv_obj_get_group(ta);
bool editing = lv_group_get_editing(g);
lv_indev_type_t indev_type = lv_indev_get_type(lv_indev_get_act());
if(indev_type == LV_INDEV_TYPE_ENCODER) {
if(editing)
lv_ta_set_cursor_type(ta, cur_type & (~LV_CURSOR_HIDDEN));
else
lv_ta_set_cursor_type(ta, cur_type | LV_CURSOR_HIDDEN);
} else {
lv_ta_set_cursor_type(ta, cur_type & (~LV_CURSOR_HIDDEN));
}
#endif
} else if(sign == LV_SIGNAL_PRESSED || sign == LV_SIGNAL_PRESSING || sign == LV_SIGNAL_PRESS_LOST ||
sign == LV_SIGNAL_RELEASED) {
update_cursor_position_on_click(ta, sign, (lv_indev_t *)param);
}
return res;
}
static lv_res_t lv_ta_scrollable_signal(lv_obj_t * scrl, lv_signal_t sign, void * param)
{
lv_res_t res;
res = scrl_signal(scrl, sign, param);
if(res != LV_RES_OK) return res;
if(sign == LV_SIGNAL_GET_TYPE) return lv_obj_handle_get_type_signal(param, "");
lv_obj_t * ta = lv_obj_get_parent(scrl);
lv_ta_ext_t * ext = lv_obj_get_ext_attr(ta);
if(sign == LV_SIGNAL_REFR_EXT_DRAW_PAD) {
const lv_style_t * style_label = lv_obj_get_style(ext->label);
lv_coord_t font_h = lv_font_get_line_height(style_label->text.font);
scrl->ext_draw_pad = LV_MATH_MAX(scrl->ext_draw_pad, style_label->text.line_space + font_h);
} else if(sign == LV_SIGNAL_CORD_CHG) {
if(ext->label) {
if(lv_obj_get_width(scrl) != lv_area_get_width(param) ||
lv_obj_get_height(scrl) != lv_area_get_height(param)) {
const lv_style_t * style_scrl = lv_obj_get_style(scrl);
lv_obj_set_width(ext->label, lv_page_get_fit_width(ta));
lv_obj_set_pos(ext->label, style_scrl->body.padding.left, style_scrl->body.padding.top);
lv_label_set_text(ext->label, NULL); 
refr_cursor_area(ta);
}
}
} else if(sign == LV_SIGNAL_PRESSING || sign == LV_SIGNAL_PRESSED || sign == LV_SIGNAL_PRESS_LOST ||
sign == LV_SIGNAL_RELEASED) {
update_cursor_position_on_click(ta, sign, (lv_indev_t *)param);
}
return res;
}
#if LV_USE_ANIMATION
static void cursor_blink_anim(lv_obj_t * ta, lv_anim_value_t show)
{
lv_ta_ext_t * ext = lv_obj_get_ext_attr(ta);
if(show != ext->cursor.state) {
ext->cursor.state = show == 0 ? 0 : 1;
if(ext->cursor.type != LV_CURSOR_NONE && (ext->cursor.type & LV_CURSOR_HIDDEN) == 0) {
lv_area_t area_tmp;
lv_area_copy(&area_tmp, &ext->cursor.area);
area_tmp.x1 += ext->label->coords.x1;
area_tmp.y1 += ext->label->coords.y1;
area_tmp.x2 += ext->label->coords.x1;
area_tmp.y2 += ext->label->coords.y1;
lv_obj_invalidate_area(ta, &area_tmp);
}
}
}
static void pwd_char_hider_anim(lv_obj_t * ta, lv_anim_value_t x)
{
(void)ta;
(void)x;
}
static void pwd_char_hider_anim_ready(lv_anim_t * a)
{
lv_obj_t * ta = a->var;
pwd_char_hider(ta);
}
#endif
static void pwd_char_hider(lv_obj_t * ta)
{
lv_ta_ext_t * ext = lv_obj_get_ext_attr(ta);
if(ext->pwd_mode != 0) {
char * txt = lv_label_get_text(ext->label);
int16_t len = lv_txt_get_encoded_length(txt);
bool refr = false;
uint16_t i;
for(i = 0; i < len; i++) {
txt[i] = '*';
refr = true;
}
txt[i] = '\0';
if(refr != false) lv_label_set_text(ext->label, txt);
}
}
static bool char_is_accepted(lv_obj_t * ta, uint32_t c)
{
lv_ta_ext_t * ext = lv_obj_get_ext_attr(ta);
if(ext->accapted_chars == NULL && ext->max_length == 0) return true;
if(ext->max_length > 0 && lv_txt_get_encoded_length(lv_ta_get_text(ta)) >= ext->max_length) {
return false;
}
if(ext->accapted_chars) {
uint32_t i = 0;
uint32_t a;
while(ext->accapted_chars[i] != '\0') {
a = lv_txt_encoded_next(ext->accapted_chars, &i);
if(a == c) return true; 
}
return false; 
} else {
return true; 
}
}
static void get_cursor_style(lv_obj_t * ta, lv_style_t * style_res)
{
lv_ta_ext_t * ext = lv_obj_get_ext_attr(ta);
const lv_style_t * label_style = lv_obj_get_style(ext->label);
if(ext->cursor.style) {
lv_style_copy(style_res, ext->cursor.style);
} else {
lv_style_copy(style_res, label_style);
lv_color_t clv_color_tmp = style_res->text.color; 
style_res->text.color =
style_res->body.main_color; 
style_res->body.main_color = clv_color_tmp;
style_res->body.grad_color = clv_color_tmp;
style_res->body.border.color = clv_color_tmp;
style_res->body.border.opa = LV_OPA_COVER;
style_res->body.border.width = 1;
style_res->body.shadow.width = 0;
style_res->body.radius = 0;
style_res->body.opa = LV_OPA_COVER;
style_res->body.padding.left = 0;
style_res->body.padding.right = 0;
style_res->body.padding.top = 0;
style_res->body.padding.bottom = 0;
style_res->line.width = 1;
}
}
static void refr_cursor_area(lv_obj_t * ta)
{
lv_ta_ext_t * ext = lv_obj_get_ext_attr(ta);
const lv_style_t * label_style = lv_obj_get_style(ext->label);
lv_style_t cur_style;
get_cursor_style(ta, &cur_style);
uint16_t cur_pos = lv_ta_get_cursor_pos(ta);
const char * txt = lv_label_get_text(ext->label);
uint32_t byte_pos;
byte_pos = lv_txt_encoded_get_byte_id(txt, cur_pos);
uint32_t letter = lv_txt_encoded_next(&txt[byte_pos], NULL);
lv_coord_t letter_h = lv_font_get_line_height(label_style->text.font);
lv_coord_t letter_w;
if(letter == '\0' || letter == '\n' || letter == '\r') {
letter_w = lv_font_get_glyph_width(label_style->text.font, ' ', '\0');
} else {
letter_w = lv_font_get_glyph_width(label_style->text.font, letter, '\0');
}
lv_point_t letter_pos;
lv_label_get_letter_pos(ext->label, cur_pos, &letter_pos);
if(letter_pos.x + ext->label->coords.x1 + letter_w > ext->label->coords.x2 && ext->one_line == 0 &&
lv_label_get_align(ext->label) != LV_LABEL_ALIGN_RIGHT) {
letter_pos.x = 0;
letter_pos.y += letter_h + label_style->text.line_space;
if(letter != '\0') {
byte_pos += lv_txt_encoded_size(&txt[byte_pos]);
letter = lv_txt_encoded_next(&txt[byte_pos], NULL);
}
if(letter == '\0' || letter == '\n' || letter == '\r') {
letter_w = lv_font_get_glyph_width(label_style->text.font, ' ', '\0');
} else {
letter_w = lv_font_get_glyph_width(label_style->text.font, letter, '\0');
}
}
ext->cursor.txt_byte_pos = byte_pos;
lv_area_t cur_area;
if(ext->cursor.type == LV_CURSOR_LINE) {
cur_area.x1 =
letter_pos.x + cur_style.body.padding.left - (cur_style.line.width >> 1) - (cur_style.line.width & 0x1);
cur_area.y1 = letter_pos.y + cur_style.body.padding.top;
cur_area.x2 = letter_pos.x + cur_style.body.padding.right + (cur_style.line.width >> 1);
cur_area.y2 = letter_pos.y + cur_style.body.padding.bottom + letter_h;
} else if(ext->cursor.type == LV_CURSOR_BLOCK) {
cur_area.x1 = letter_pos.x - cur_style.body.padding.left;
cur_area.y1 = letter_pos.y - cur_style.body.padding.top;
cur_area.x2 = letter_pos.x + cur_style.body.padding.right + letter_w;
cur_area.y2 = letter_pos.y + cur_style.body.padding.bottom + letter_h;
} else if(ext->cursor.type == LV_CURSOR_OUTLINE) {
cur_area.x1 = letter_pos.x - cur_style.body.padding.left;
cur_area.y1 = letter_pos.y - cur_style.body.padding.top;
cur_area.x2 = letter_pos.x + cur_style.body.padding.right + letter_w;
cur_area.y2 = letter_pos.y + cur_style.body.padding.bottom + letter_h;
} else if(ext->cursor.type == LV_CURSOR_UNDERLINE) {
cur_area.x1 = letter_pos.x + cur_style.body.padding.left;
cur_area.y1 = letter_pos.y + cur_style.body.padding.top + letter_h - (cur_style.line.width >> 1);
cur_area.x2 = letter_pos.x + cur_style.body.padding.right + letter_w;
cur_area.y2 = letter_pos.y + cur_style.body.padding.bottom + letter_h + (cur_style.line.width >> 1) +
(cur_style.line.width & 0x1);
}
lv_area_t area_tmp;
lv_area_copy(&area_tmp, &ext->cursor.area);
area_tmp.x1 += ext->label->coords.x1;
area_tmp.y1 += ext->label->coords.y1;
area_tmp.x2 += ext->label->coords.x1;
area_tmp.y2 += ext->label->coords.y1;
lv_obj_invalidate_area(ta, &area_tmp);
lv_area_copy(&ext->cursor.area, &cur_area);
lv_area_copy(&area_tmp, &ext->cursor.area);
area_tmp.x1 += ext->label->coords.x1;
area_tmp.y1 += ext->label->coords.y1;
area_tmp.x2 += ext->label->coords.x1;
area_tmp.y2 += ext->label->coords.y1;
lv_obj_invalidate_area(ta, &area_tmp);
}
static void placeholder_update(lv_obj_t * ta)
{
lv_ta_ext_t * ext = lv_obj_get_ext_attr(ta);
const char * ta_text;
if(ext->placeholder == NULL) return;
ta_text = lv_ta_get_text(ta);
if(ta_text[0] == '\0') {
lv_obj_t * scrl = lv_page_get_scrl(ta);
const lv_style_t * style_scrl = lv_obj_get_style(scrl);
lv_obj_set_pos(ext->placeholder, style_scrl->body.padding.left, style_scrl->body.padding.top);
lv_obj_set_pos(ext->label, style_scrl->body.padding.left, style_scrl->body.padding.top);
lv_obj_set_width(ext->placeholder, lv_page_get_fit_width(ta));
lv_obj_set_hidden(ext->placeholder, false);
} else {
lv_obj_set_hidden(ext->placeholder, true);
}
}
static void update_cursor_position_on_click(lv_obj_t * ta, lv_signal_t sign, lv_indev_t * click_source)
{
if(click_source == NULL) return;
lv_ta_ext_t * ext = lv_obj_get_ext_attr(ta);
if(ext->cursor.click_pos == 0) return;
if(ext->cursor.type == LV_CURSOR_NONE) return;
if(lv_indev_get_type(click_source) == LV_INDEV_TYPE_KEYPAD ||
lv_indev_get_type(click_source) == LV_INDEV_TYPE_ENCODER) {
return;
}
lv_area_t label_coords;
lv_obj_get_coords(ext->label, &label_coords);
lv_point_t point_act, vect_act;
lv_indev_get_point(click_source, &point_act);
lv_indev_get_vect(click_source, &vect_act);
if(point_act.x < 0 || point_act.y < 0) return; 
lv_point_t rel_pos;
rel_pos.x = point_act.x - label_coords.x1;
rel_pos.y = point_act.y - label_coords.y1;
lv_coord_t label_width = lv_obj_get_width(ext->label);
uint16_t char_id_at_click;
#if LV_LABEL_TEXT_SEL
lv_label_ext_t * ext_label = lv_obj_get_ext_attr(ext->label);
bool click_outside_label;
if(rel_pos.x < 0) {
char_id_at_click = 0;
click_outside_label = true;
}
else if(rel_pos.x >= label_width) {
char_id_at_click = LV_TA_CURSOR_LAST;
click_outside_label = true;
} else {
char_id_at_click = lv_label_get_letter_on(ext->label, &rel_pos);
click_outside_label = !lv_label_is_char_under_pos(ext->label, &rel_pos);
}
if(ext->text_sel_en) {
if(!ext->text_sel_in_prog && !click_outside_label && sign == LV_SIGNAL_PRESSED) {
ext->sel.start = char_id_at_click;
ext->sel.end = LV_LABEL_TEXT_SEL_OFF;
ext->text_sel_in_prog = 1;
lv_obj_set_drag(lv_page_get_scrl(ta), false);
} else if(ext->text_sel_in_prog && sign == LV_SIGNAL_PRESSING) {
ext->sel.end = char_id_at_click;
} else if(ext->text_sel_in_prog && (sign == LV_SIGNAL_PRESS_LOST || sign == LV_SIGNAL_RELEASED)) {
lv_obj_set_drag(lv_page_get_scrl(ta), true);
}
}
if(ext->text_sel_in_prog || sign == LV_SIGNAL_PRESSED) lv_ta_set_cursor_pos(ta, char_id_at_click);
if(ext->text_sel_in_prog) {
if(ext->sel.start > ext->sel.end) {
if(ext_label->txt_sel_start != ext->sel.end || ext_label->txt_sel_end != ext->sel.start) {
ext_label->txt_sel_start = ext->sel.end;
ext_label->txt_sel_end = ext->sel.start;
lv_obj_invalidate(ta);
}
} else if(ext->sel.start < ext->sel.end) {
if(ext_label->txt_sel_start != ext->sel.start || ext_label->txt_sel_end != ext->sel.end) {
ext_label->txt_sel_start = ext->sel.start;
ext_label->txt_sel_end = ext->sel.end;
lv_obj_invalidate(ta);
}
} else {
if(ext_label->txt_sel_start != LV_DRAW_LABEL_NO_TXT_SEL || ext_label->txt_sel_end != LV_DRAW_LABEL_NO_TXT_SEL) {
ext_label->txt_sel_start = LV_DRAW_LABEL_NO_TXT_SEL;
ext_label->txt_sel_end = LV_DRAW_LABEL_NO_TXT_SEL;
lv_obj_invalidate(ta);
}
}
if(sign == LV_SIGNAL_PRESS_LOST || sign == LV_SIGNAL_RELEASED) {
ext->text_sel_in_prog = 0;
}
}
#else
if(rel_pos.x < 0) {
char_id_at_click = 0;
}
else if(rel_pos.x >= label_width) {
char_id_at_click = LV_TA_CURSOR_LAST;
} else {
char_id_at_click = lv_label_get_letter_on(ext->label, &rel_pos);
}
if(sign == LV_SIGNAL_PRESSED) lv_ta_set_cursor_pos(ta, char_id_at_click);
#endif
}
#endif
