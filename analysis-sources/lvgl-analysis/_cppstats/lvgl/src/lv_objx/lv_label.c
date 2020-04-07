#include "lv_label.h"
#if LV_USE_LABEL != 0
#include "../lv_core/lv_obj.h"
#include "../lv_core/lv_debug.h"
#include "../lv_core/lv_group.h"
#include "../lv_misc/lv_color.h"
#include "../lv_misc/lv_math.h"
#include "../lv_misc/lv_bidi.h"
#include "../lv_misc/lv_printf.h"
#define LV_OBJX_NAME "lv_label"
#if !defined(LV_LABEL_DEF_SCROLL_SPEED)
#define LV_LABEL_DEF_SCROLL_SPEED (25)
#endif
#define LV_LABEL_DOT_END_INV 0xFFFF
#define LV_LABEL_HINT_HEIGHT_LIMIT 1024 
static lv_res_t lv_label_signal(lv_obj_t * label, lv_signal_t sign, void * param);
static bool lv_label_design(lv_obj_t * label, const lv_area_t * mask, lv_design_mode_t mode);
static void lv_label_refr_text(lv_obj_t * label);
static void lv_label_revert_dots(lv_obj_t * label);
#if LV_USE_ANIMATION
static void lv_label_set_offset_x(lv_obj_t * label, lv_coord_t x);
static void lv_label_set_offset_y(lv_obj_t * label, lv_coord_t y);
#endif
static bool lv_label_set_dot_tmp(lv_obj_t * label, char * data, uint16_t len);
static char * lv_label_get_dot_tmp(lv_obj_t * label);
static void lv_label_dot_tmp_free(lv_obj_t * label);
static lv_signal_cb_t ancestor_signal;
lv_obj_t * lv_label_create(lv_obj_t * par, const lv_obj_t * copy)
{
LV_LOG_TRACE("label create started");
lv_obj_t * new_label = lv_obj_create(par, copy);
LV_ASSERT_MEM(new_label);
if(new_label == NULL) return NULL;
if(ancestor_signal == NULL) ancestor_signal = lv_obj_get_signal_cb(new_label);
lv_obj_allocate_ext_attr(new_label, sizeof(lv_label_ext_t));
lv_label_ext_t * ext = lv_obj_get_ext_attr(new_label);
LV_ASSERT_MEM(ext);
if(ext == NULL) return NULL;
ext->text = NULL;
ext->static_txt = 0;
ext->recolor = 0;
ext->body_draw = 0;
ext->align = LV_LABEL_ALIGN_AUTO;
ext->dot_end = LV_LABEL_DOT_END_INV;
ext->long_mode = LV_LABEL_LONG_EXPAND;
#if LV_USE_ANIMATION
ext->anim_speed = LV_LABEL_DEF_SCROLL_SPEED;
#endif
ext->offset.x = 0;
ext->offset.y = 0;
#if LV_LABEL_LONG_TXT_HINT
ext->hint.line_start = -1;
ext->hint.coord_y = 0;
ext->hint.y = 0;
#endif
#if LV_LABEL_TEXT_SEL
ext->txt_sel_start = LV_DRAW_LABEL_NO_TXT_SEL;
ext->txt_sel_end = LV_DRAW_LABEL_NO_TXT_SEL;
#endif
ext->dot.tmp_ptr = NULL;
ext->dot_tmp_alloc = 0;
lv_obj_set_design_cb(new_label, lv_label_design);
lv_obj_set_signal_cb(new_label, lv_label_signal);
if(copy == NULL) {
lv_obj_set_click(new_label, false);
lv_label_set_long_mode(new_label, LV_LABEL_LONG_EXPAND);
lv_label_set_text(new_label, "Text");
lv_label_set_style(new_label, LV_LABEL_STYLE_MAIN, NULL); 
}
else {
lv_label_ext_t * copy_ext = lv_obj_get_ext_attr(copy);
lv_label_set_long_mode(new_label, lv_label_get_long_mode(copy));
lv_label_set_recolor(new_label, lv_label_get_recolor(copy));
lv_label_set_body_draw(new_label, lv_label_get_body_draw(copy));
lv_label_set_align(new_label, lv_label_get_align(copy));
if(copy_ext->static_txt == 0)
lv_label_set_text(new_label, lv_label_get_text(copy));
else
lv_label_set_static_text(new_label, lv_label_get_text(copy));
if(copy_ext->long_mode == LV_LABEL_LONG_DOT) {
ext->text = lv_mem_realloc(ext->text, lv_mem_get_size(copy_ext->text));
LV_ASSERT_MEM(ext->text);
if(ext->text == NULL) return NULL;
memcpy(ext->text, copy_ext->text, lv_mem_get_size(copy_ext->text));
}
if(copy_ext->dot_tmp_alloc && copy_ext->dot.tmp_ptr) {
uint16_t len = (uint16_t )strlen(copy_ext->dot.tmp_ptr);
lv_label_set_dot_tmp(new_label, ext->dot.tmp_ptr, len);
} else {
memcpy(ext->dot.tmp, copy_ext->dot.tmp, sizeof(ext->dot.tmp));
}
ext->dot_tmp_alloc = copy_ext->dot_tmp_alloc;
ext->dot_end = copy_ext->dot_end;
lv_obj_refresh_style(new_label);
}
LV_LOG_INFO("label created");
return new_label;
}
void lv_label_set_text(lv_obj_t * label, const char * text)
{
LV_ASSERT_OBJ(label, LV_OBJX_NAME);
lv_obj_invalidate(label);
lv_label_ext_t * ext = lv_obj_get_ext_attr(label);
if(text == NULL) {
lv_label_refr_text(label);
return;
}
LV_ASSERT_STR(text);
if(ext->text == text) {
ext->text = lv_mem_realloc(ext->text, strlen(ext->text) + 1);
LV_ASSERT_MEM(ext->text);
if(ext->text == NULL) return;
} else {
size_t len = strlen(text) + 1;
if(ext->text != NULL && ext->static_txt == 0) {
lv_mem_free(ext->text);
ext->text = NULL;
}
ext->text = lv_mem_alloc(len);
LV_ASSERT_MEM(ext->text);
if(ext->text == NULL) return;
strcpy(ext->text, text);
ext->static_txt = 0;
}
lv_label_refr_text(label);
}
void lv_label_set_text_fmt(lv_obj_t * label, const char * fmt, ...)
{
LV_ASSERT_OBJ(label, LV_OBJX_NAME);
LV_ASSERT_STR(fmt);
lv_obj_invalidate(label);
lv_label_ext_t * ext = lv_obj_get_ext_attr(label);
if(fmt == NULL) {
lv_label_refr_text(label);
return;
}
if(ext->text != NULL && ext->static_txt == 0) {
lv_mem_free(ext->text);
ext->text = NULL;
}
va_list ap, ap2;
va_start(ap, fmt);
va_copy(ap2, ap);
uint32_t len = lv_vsnprintf(NULL, 0, fmt, ap);
va_end(ap);
ext->text = lv_mem_alloc(len+1);
LV_ASSERT_MEM(ext->text);
if(ext->text == NULL) return;
ext->text[len-1] = 0; 
lv_vsnprintf(ext->text, len+1, fmt, ap2);
va_end(ap2);
ext->static_txt = 0; 
lv_label_refr_text(label);
}
void lv_label_set_array_text(lv_obj_t * label, const char * array, uint16_t size)
{
LV_ASSERT_OBJ(label, LV_OBJX_NAME);
lv_obj_invalidate(label);
lv_label_ext_t * ext = lv_obj_get_ext_attr(label);
if(array == ext->text || array == NULL) {
lv_label_refr_text(label);
return;
}
if(ext->text != NULL && ext->static_txt == 0) {
lv_mem_free(ext->text);
ext->text = NULL;
}
ext->text = lv_mem_alloc(size + 1);
LV_ASSERT_MEM(ext->text);
if(ext->text == NULL) return;
memcpy(ext->text, array, size);
ext->text[size] = '\0';
ext->static_txt = 0; 
lv_label_refr_text(label);
}
void lv_label_set_static_text(lv_obj_t * label, const char * text)
{
LV_ASSERT_OBJ(label, LV_OBJX_NAME);
lv_label_ext_t * ext = lv_obj_get_ext_attr(label);
if(ext->static_txt == 0 && ext->text != NULL) {
lv_mem_free(ext->text);
ext->text = NULL;
}
if(text != NULL) {
ext->static_txt = 1;
ext->text = (char *)text;
}
lv_label_refr_text(label);
}
void lv_label_set_long_mode(lv_obj_t * label, lv_label_long_mode_t long_mode)
{
LV_ASSERT_OBJ(label, LV_OBJX_NAME);
lv_label_ext_t * ext = lv_obj_get_ext_attr(label);
#if LV_USE_ANIMATION
lv_anim_del(label, (lv_anim_exec_xcb_t)lv_obj_set_x);
lv_anim_del(label, (lv_anim_exec_xcb_t)lv_obj_set_y);
lv_anim_del(label, (lv_anim_exec_xcb_t)lv_label_set_offset_x);
lv_anim_del(label, (lv_anim_exec_xcb_t)lv_label_set_offset_y);
#endif
ext->offset.x = 0;
ext->offset.y = 0;
if(long_mode == LV_LABEL_LONG_SROLL || long_mode == LV_LABEL_LONG_SROLL_CIRC || long_mode == LV_LABEL_LONG_CROP)
ext->expand = 1;
else
ext->expand = 0;
if(ext->long_mode == LV_LABEL_LONG_DOT && ext->dot_end != LV_LABEL_DOT_END_INV) {
lv_label_revert_dots(label);
}
ext->long_mode = long_mode;
lv_label_refr_text(label);
}
void lv_label_set_align(lv_obj_t * label, lv_label_align_t align)
{
LV_ASSERT_OBJ(label, LV_OBJX_NAME);
lv_label_ext_t * ext = lv_obj_get_ext_attr(label);
if(ext->align == align) return;
ext->align = align;
lv_obj_invalidate(label); 
}
void lv_label_set_recolor(lv_obj_t * label, bool en)
{
LV_ASSERT_OBJ(label, LV_OBJX_NAME);
lv_label_ext_t * ext = lv_obj_get_ext_attr(label);
if(ext->recolor == en) return;
ext->recolor = en == false ? 0 : 1;
lv_label_refr_text(label); 
}
void lv_label_set_body_draw(lv_obj_t * label, bool en)
{
LV_ASSERT_OBJ(label, LV_OBJX_NAME);
lv_label_ext_t * ext = lv_obj_get_ext_attr(label);
if(ext->body_draw == en) return;
ext->body_draw = en == false ? 0 : 1;
lv_obj_refresh_ext_draw_pad(label);
lv_obj_invalidate(label);
}
void lv_label_set_anim_speed(lv_obj_t * label, uint16_t anim_speed)
{
LV_ASSERT_OBJ(label, LV_OBJX_NAME);
#if LV_USE_ANIMATION
lv_label_ext_t * ext = lv_obj_get_ext_attr(label);
if(ext->anim_speed == anim_speed) return;
ext->anim_speed = anim_speed;
if(ext->long_mode == LV_LABEL_LONG_SROLL || ext->long_mode == LV_LABEL_LONG_SROLL_CIRC) {
lv_label_refr_text(label);
}
#else
(void)label; 
(void)anim_speed; 
#endif
}
void lv_label_set_text_sel_start(lv_obj_t * label, uint16_t index)
{
LV_ASSERT_OBJ(label, LV_OBJX_NAME);
#if LV_LABEL_TEXT_SEL
lv_label_ext_t * ext = lv_obj_get_ext_attr(label);
ext->txt_sel_start = index;
lv_obj_invalidate(label);
#else
(void)label; 
(void)index; 
#endif
}
void lv_label_set_text_sel_end(lv_obj_t * label, uint16_t index)
{
LV_ASSERT_OBJ(label, LV_OBJX_NAME);
#if LV_LABEL_TEXT_SEL
lv_label_ext_t * ext = lv_obj_get_ext_attr(label);
ext->txt_sel_end = index;
lv_obj_invalidate(label);
#else
(void)label; 
(void)index; 
#endif
}
char * lv_label_get_text(const lv_obj_t * label)
{
LV_ASSERT_OBJ(label, LV_OBJX_NAME);
lv_label_ext_t * ext = lv_obj_get_ext_attr(label);
return ext->text;
}
lv_label_long_mode_t lv_label_get_long_mode(const lv_obj_t * label)
{
LV_ASSERT_OBJ(label, LV_OBJX_NAME);
lv_label_ext_t * ext = lv_obj_get_ext_attr(label);
return ext->long_mode;
}
lv_label_align_t lv_label_get_align(const lv_obj_t * label)
{
LV_ASSERT_OBJ(label, LV_OBJX_NAME);
lv_label_ext_t * ext = lv_obj_get_ext_attr(label);
lv_label_align_t align = ext->align;
if(align == LV_LABEL_ALIGN_AUTO) {
#if LV_USE_BIDI
lv_bidi_dir_t base_dir = lv_obj_get_base_dir(label);
if(base_dir == LV_BIDI_DIR_AUTO) base_dir = lv_bidi_detect_base_dir(ext->text);
if(base_dir == LV_BIDI_DIR_LTR) align = LV_LABEL_ALIGN_LEFT;
else if (base_dir == LV_BIDI_DIR_RTL) align = LV_LABEL_ALIGN_RIGHT;
#else
align = LV_LABEL_ALIGN_LEFT;
#endif
}
return align;
}
bool lv_label_get_recolor(const lv_obj_t * label)
{
LV_ASSERT_OBJ(label, LV_OBJX_NAME);
lv_label_ext_t * ext = lv_obj_get_ext_attr(label);
return ext->recolor == 0 ? false : true;
}
bool lv_label_get_body_draw(const lv_obj_t * label)
{
LV_ASSERT_OBJ(label, LV_OBJX_NAME);
lv_label_ext_t * ext = lv_obj_get_ext_attr(label);
return ext->body_draw == 0 ? false : true;
}
uint16_t lv_label_get_anim_speed(const lv_obj_t * label)
{
LV_ASSERT_OBJ(label, LV_OBJX_NAME);
#if LV_USE_ANIMATION
lv_label_ext_t * ext = lv_obj_get_ext_attr(label);
return ext->anim_speed;
#else
(void)label; 
return 0;
#endif
}
void lv_label_get_letter_pos(const lv_obj_t * label, uint16_t char_id, lv_point_t * pos)
{
LV_ASSERT_OBJ(label, LV_OBJX_NAME);
LV_ASSERT_NULL(pos);
const char * txt = lv_label_get_text(label);
lv_label_ext_t * ext = lv_obj_get_ext_attr(label);
uint32_t line_start = 0;
uint32_t new_line_start = 0;
lv_coord_t max_w = lv_obj_get_width(label);
const lv_style_t * style = lv_obj_get_style(label);
const lv_font_t * font = style->text.font;
uint8_t letter_height = lv_font_get_line_height(font);
lv_coord_t y = 0;
lv_txt_flag_t flag = LV_TXT_FLAG_NONE;
if(ext->recolor != 0) flag |= LV_TXT_FLAG_RECOLOR;
if(ext->expand != 0) flag |= LV_TXT_FLAG_EXPAND;
lv_label_align_t align = lv_label_get_align(label);
if(align == LV_LABEL_ALIGN_CENTER) flag |= LV_TXT_FLAG_CENTER;
if(align == LV_LABEL_ALIGN_RIGHT) flag |= LV_TXT_FLAG_RIGHT;
if(ext->long_mode == LV_LABEL_LONG_EXPAND) {
max_w = LV_COORD_MAX;
}
uint16_t byte_id = lv_txt_encoded_get_byte_id(txt, char_id);
;
while(txt[new_line_start] != '\0') {
new_line_start += lv_txt_get_next_line(&txt[line_start], font, style->text.letter_space, max_w, flag);
if(byte_id < new_line_start || txt[new_line_start] == '\0')
break; 
y += letter_height + style->text.line_space;
line_start = new_line_start;
}
if(byte_id > 0) {
if((txt[byte_id - 1] == '\n' || txt[byte_id - 1] == '\r') && txt[byte_id] == '\0') {
y += letter_height + style->text.line_space;
line_start = byte_id;
}
}
const char *bidi_txt;
uint16_t visual_byte_pos;
#if LV_USE_BIDI
if(new_line_start == byte_id) {
visual_byte_pos = byte_id - line_start;
bidi_txt = &txt[line_start];
}
else {
uint16_t line_char_id = lv_txt_encoded_get_char_id(&txt[line_start], byte_id - line_start);
bool is_rtl;
char *mutable_bidi_txt;
uint16_t visual_char_pos = lv_bidi_get_visual_pos(&txt[line_start], &mutable_bidi_txt, new_line_start - line_start, lv_obj_get_base_dir(label), line_char_id, &is_rtl);
bidi_txt = mutable_bidi_txt;
if (is_rtl) visual_char_pos++;
visual_byte_pos = lv_txt_encoded_get_byte_id(bidi_txt, visual_char_pos);
}
#else
bidi_txt = &txt[line_start];
visual_byte_pos = byte_id - line_start;
#endif
lv_coord_t x = lv_txt_get_width(bidi_txt, visual_byte_pos, font, style->text.letter_space, flag);
if(char_id != line_start) x += style->text.letter_space;
if(align == LV_LABEL_ALIGN_CENTER) {
lv_coord_t line_w;
line_w = lv_txt_get_width(bidi_txt, new_line_start - line_start, font, style->text.letter_space, flag);
x += lv_obj_get_width(label) / 2 - line_w / 2;
} else if(align == LV_LABEL_ALIGN_RIGHT) {
lv_coord_t line_w;
line_w = lv_txt_get_width(bidi_txt, new_line_start - line_start, font, style->text.letter_space, flag);
x += lv_obj_get_width(label) - line_w;
}
pos->x = x;
pos->y = y;
}
uint16_t lv_label_get_letter_on(const lv_obj_t * label, lv_point_t * pos)
{
LV_ASSERT_OBJ(label, LV_OBJX_NAME);
LV_ASSERT_NULL(pos);
const char * txt = lv_label_get_text(label);
lv_label_ext_t * ext = lv_obj_get_ext_attr(label);
uint32_t line_start = 0;
uint32_t new_line_start = 0;
lv_coord_t max_w = lv_obj_get_width(label);
const lv_style_t * style = lv_obj_get_style(label);
const lv_font_t * font = style->text.font;
uint8_t letter_height = lv_font_get_line_height(font);
lv_coord_t y = 0;
lv_txt_flag_t flag = LV_TXT_FLAG_NONE;
uint16_t logical_pos;
char *bidi_txt;
if(ext->recolor != 0) flag |= LV_TXT_FLAG_RECOLOR;
if(ext->expand != 0) flag |= LV_TXT_FLAG_EXPAND;
lv_label_align_t align = lv_label_get_align(label);
if(align == LV_LABEL_ALIGN_CENTER) flag |= LV_TXT_FLAG_CENTER;
if(align == LV_LABEL_ALIGN_RIGHT) flag |= LV_TXT_FLAG_RIGHT;
if(ext->long_mode == LV_LABEL_LONG_EXPAND) {
max_w = LV_COORD_MAX;
}
;
while(txt[line_start] != '\0') {
new_line_start += lv_txt_get_next_line(&txt[line_start], font, style->text.letter_space, max_w, flag);
if(pos->y <= y + letter_height) {
uint32_t tmp = new_line_start;
uint32_t letter;
letter = lv_txt_encoded_prev(txt, &tmp);
if(letter != '\n' && txt[new_line_start] == '\0' ) new_line_start++;
break;
}
y += letter_height + style->text.line_space;
line_start = new_line_start;
}
#if LV_USE_BIDI
bidi_txt = lv_draw_get_buf(new_line_start - line_start + 1);
uint16_t txt_len = new_line_start - line_start;
if(bidi_txt[new_line_start] == '\0') txt_len--;
lv_bidi_process_paragraph(txt + line_start, bidi_txt, txt_len, lv_obj_get_base_dir(label), NULL, 0);
#else
bidi_txt = (char*)txt + line_start;
#endif
lv_coord_t x = 0;
if(align == LV_LABEL_ALIGN_CENTER) {
lv_coord_t line_w;
line_w = lv_txt_get_width(bidi_txt, new_line_start - line_start, font, style->text.letter_space, flag);
x += lv_obj_get_width(label) / 2 - line_w / 2;
}
else if(align == LV_LABEL_ALIGN_RIGHT) {
lv_coord_t line_w;
line_w = lv_txt_get_width(bidi_txt, new_line_start - line_start, font, style->text.letter_space, flag);
x += lv_obj_get_width(label) - line_w;
}
lv_txt_cmd_state_t cmd_state = LV_TXT_CMD_STATE_WAIT;
uint32_t i = 0;
uint32_t i_act = i;
uint32_t letter;
uint32_t letter_next;
if(new_line_start > 0) {
while(i + line_start < new_line_start) {
letter = lv_txt_encoded_next(bidi_txt, &i);
letter_next = lv_txt_encoded_next(&bidi_txt[i], NULL);
if((flag & LV_TXT_FLAG_RECOLOR) != 0) {
if(lv_txt_is_cmd(&cmd_state, bidi_txt[i]) != false) {
continue; 
}
}
x += lv_font_get_glyph_width(font, letter, letter_next);
if(pos->x < x || i + line_start == new_line_start) {
i = i_act;
break;
}
x += style->text.letter_space;
i_act = i;
}
}
#if LV_USE_BIDI
bool is_rtl;
logical_pos = lv_bidi_get_logical_pos(&txt[line_start], NULL, txt_len, lv_obj_get_base_dir(label), lv_txt_encoded_get_char_id(bidi_txt, i), &is_rtl);
if (is_rtl) logical_pos++;
#else
logical_pos = lv_txt_encoded_get_char_id(bidi_txt, i);
#endif
return logical_pos + lv_txt_encoded_get_char_id(txt, line_start);
}
uint16_t lv_label_get_text_sel_start(const lv_obj_t * label)
{
LV_ASSERT_OBJ(label, LV_OBJX_NAME);
#if LV_LABEL_TEXT_SEL
lv_label_ext_t * ext = lv_obj_get_ext_attr(label);
return ext->txt_sel_start;
#else
(void)label; 
return LV_LABEL_TEXT_SEL_OFF;
#endif
}
uint16_t lv_label_get_text_sel_end(const lv_obj_t * label)
{
LV_ASSERT_OBJ(label, LV_OBJX_NAME);
#if LV_LABEL_TEXT_SEL
lv_label_ext_t * ext = lv_obj_get_ext_attr(label);
return ext->txt_sel_end;
#else
(void)label; 
return LV_LABEL_TEXT_SEL_OFF;
#endif
}
bool lv_label_is_char_under_pos(const lv_obj_t * label, lv_point_t * pos)
{
LV_ASSERT_OBJ(label, LV_OBJX_NAME);
LV_ASSERT_NULL(pos);
const char * txt = lv_label_get_text(label);
lv_label_ext_t * ext = lv_obj_get_ext_attr(label);
uint32_t line_start = 0;
uint32_t new_line_start = 0;
lv_coord_t max_w = lv_obj_get_width(label);
const lv_style_t * style = lv_obj_get_style(label);
const lv_font_t * font = style->text.font;
uint8_t letter_height = lv_font_get_line_height(font);
lv_coord_t y = 0;
lv_txt_flag_t flag = LV_TXT_FLAG_NONE;
lv_label_align_t align = lv_label_get_align(label);
if(ext->recolor != 0) flag |= LV_TXT_FLAG_RECOLOR;
if(ext->expand != 0) flag |= LV_TXT_FLAG_EXPAND;
if(align == LV_LABEL_ALIGN_CENTER) flag |= LV_TXT_FLAG_CENTER;
if(ext->long_mode == LV_LABEL_LONG_EXPAND) {
max_w = LV_COORD_MAX;
}
;
while(txt[line_start] != '\0') {
new_line_start += lv_txt_get_next_line(&txt[line_start], font, style->text.letter_space, max_w, flag);
if(pos->y <= y + letter_height) break; 
y += letter_height + style->text.line_space;
line_start = new_line_start;
}
lv_coord_t x = 0;
lv_coord_t last_x = 0;
if(align == LV_LABEL_ALIGN_CENTER) {
lv_coord_t line_w;
line_w = lv_txt_get_width(&txt[line_start], new_line_start - line_start, font, style->text.letter_space, flag);
x += lv_obj_get_width(label) / 2 - line_w / 2;
}
else if(align == LV_LABEL_ALIGN_RIGHT) {
lv_coord_t line_w;
line_w = lv_txt_get_width(&txt[line_start], new_line_start - line_start, font, style->text.letter_space, flag);
x += lv_obj_get_width(label) - line_w;
}
lv_txt_cmd_state_t cmd_state = LV_TXT_CMD_STATE_WAIT;
uint32_t i = line_start;
uint32_t i_current = i;
uint32_t letter = '\0';
uint32_t letter_next = '\0';
if(new_line_start > 0) {
while(i <= new_line_start - 1) {
letter = lv_txt_encoded_next(txt, &i);
letter_next = lv_txt_encoded_next(&txt[i], NULL);
if((flag & LV_TXT_FLAG_RECOLOR) != 0) {
if(lv_txt_is_cmd(&cmd_state, txt[i]) != false) {
continue; 
}
}
last_x = x;
x += lv_font_get_glyph_width(font, letter, letter_next);
if(pos->x < x) {
i = i_current;
break;
}
x += style->text.letter_space;
i_current = i;
}
}
int32_t max_diff = lv_font_get_glyph_width(font, letter, letter_next) + style->text.letter_space + 1;
return (pos->x >= (last_x - style->text.letter_space) && pos->x <= (last_x + max_diff));
}
void lv_label_ins_text(lv_obj_t * label, uint32_t pos, const char * txt)
{
LV_ASSERT_OBJ(label, LV_OBJX_NAME);
LV_ASSERT_STR(txt);
lv_label_ext_t * ext = lv_obj_get_ext_attr(label);
if(ext->static_txt != 0) return;
lv_obj_invalidate(label);
size_t old_len = strlen(ext->text);
size_t ins_len = strlen(txt);
size_t new_len = ins_len + old_len;
ext->text = lv_mem_realloc(ext->text, new_len + 1);
LV_ASSERT_MEM(ext->text);
if(ext->text == NULL) return;
if(pos == LV_LABEL_POS_LAST) {
pos = lv_txt_get_encoded_length(ext->text);
}
lv_txt_ins(ext->text, pos, txt);
lv_label_refr_text(label);
}
void lv_label_cut_text(lv_obj_t * label, uint32_t pos, uint32_t cnt)
{
LV_ASSERT_OBJ(label, LV_OBJX_NAME);
lv_label_ext_t * ext = lv_obj_get_ext_attr(label);
if(ext->static_txt != 0) return;
lv_obj_invalidate(label);
char * label_txt = lv_label_get_text(label);
lv_txt_cut(label_txt, pos, cnt);
lv_label_refr_text(label);
}
static bool lv_label_design(lv_obj_t * label, const lv_area_t * mask, lv_design_mode_t mode)
{
if(mode == LV_DESIGN_COVER_CHK)
return false;
else if(mode == LV_DESIGN_DRAW_MAIN) {
lv_area_t coords;
const lv_style_t * style = lv_obj_get_style(label);
lv_opa_t opa_scale = lv_obj_get_opa_scale(label);
lv_obj_get_coords(label, &coords);
#if LV_USE_GROUP
lv_group_t * g = lv_obj_get_group(label);
if(lv_group_get_focused(g) == label) {
lv_draw_rect(&coords, mask, style, opa_scale);
}
#endif
lv_label_ext_t * ext = lv_obj_get_ext_attr(label);
if(ext->body_draw) {
lv_area_t bg;
lv_obj_get_coords(label, &bg);
bg.x1 -= style->body.padding.left;
bg.x2 += style->body.padding.right;
bg.y1 -= style->body.padding.top;
bg.y2 += style->body.padding.bottom;
lv_draw_rect(&bg, mask, style, lv_obj_get_opa_scale(label));
}
lv_label_align_t align = lv_label_get_align(label);
lv_txt_flag_t flag = LV_TXT_FLAG_NONE;
if(ext->recolor != 0) flag |= LV_TXT_FLAG_RECOLOR;
if(ext->expand != 0) flag |= LV_TXT_FLAG_EXPAND;
if(align == LV_LABEL_ALIGN_CENTER) flag |= LV_TXT_FLAG_CENTER;
if(align == LV_LABEL_ALIGN_RIGHT) flag |= LV_TXT_FLAG_RIGHT;
if((ext->long_mode == LV_LABEL_LONG_SROLL || ext->long_mode == LV_LABEL_LONG_SROLL_CIRC) &&
(ext->align == LV_LABEL_ALIGN_CENTER || ext->align == LV_LABEL_ALIGN_RIGHT)) {
lv_point_t size;
lv_txt_get_size(&size, ext->text, style->text.font, style->text.letter_space, style->text.line_space,
LV_COORD_MAX, flag);
if(size.x > lv_obj_get_width(label)) {
flag &= ~LV_TXT_FLAG_RIGHT;
flag &= ~LV_TXT_FLAG_CENTER;
}
}
#if LV_LABEL_LONG_TXT_HINT
lv_draw_label_hint_t * hint = &ext->hint;
if(ext->long_mode == LV_LABEL_LONG_SROLL_CIRC || lv_obj_get_height(label) < LV_LABEL_HINT_HEIGHT_LIMIT)
hint = NULL;
#else
lv_draw_label_hint_t * hint = NULL;
#endif
lv_draw_label_txt_sel_t sel;
sel.start = lv_label_get_text_sel_start(label);
sel.end = lv_label_get_text_sel_end(label);
lv_draw_label(&coords, mask, style, opa_scale, ext->text, flag, &ext->offset, &sel, hint, lv_obj_get_base_dir(label));
if(ext->long_mode == LV_LABEL_LONG_SROLL_CIRC) {
lv_point_t size;
lv_txt_get_size(&size, ext->text, style->text.font, style->text.letter_space, style->text.line_space,
LV_COORD_MAX, flag);
lv_point_t ofs;
if(size.x > lv_obj_get_width(label)) {
ofs.x = ext->offset.x + size.x +
lv_font_get_glyph_width(style->text.font, ' ', ' ') * LV_LABEL_WAIT_CHAR_COUNT;
ofs.y = ext->offset.y;
lv_draw_label(&coords, mask, style, opa_scale, ext->text, flag, &ofs, &sel, NULL, lv_obj_get_base_dir(label));
}
if(size.y > lv_obj_get_height(label)) {
ofs.x = ext->offset.x;
ofs.y = ext->offset.y + size.y + lv_font_get_line_height(style->text.font);
lv_draw_label(&coords, mask, style, opa_scale, ext->text, flag, &ofs, &sel, NULL, lv_obj_get_base_dir(label));
}
}
}
return true;
}
static lv_res_t lv_label_signal(lv_obj_t * label, lv_signal_t sign, void * param)
{
lv_res_t res;
res = ancestor_signal(label, sign, param);
if(res != LV_RES_OK) return res;
if(sign == LV_SIGNAL_GET_TYPE) return lv_obj_handle_get_type_signal(param, LV_OBJX_NAME);
lv_label_ext_t * ext = lv_obj_get_ext_attr(label);
if(sign == LV_SIGNAL_CLEANUP) {
if(ext->static_txt == 0) {
lv_mem_free(ext->text);
ext->text = NULL;
}
lv_label_dot_tmp_free(label);
} else if(sign == LV_SIGNAL_STYLE_CHG) {
lv_label_revert_dots(label);
lv_label_refr_text(label);
} else if(sign == LV_SIGNAL_CORD_CHG) {
if(lv_area_get_width(&label->coords) != lv_area_get_width(param) ||
lv_area_get_height(&label->coords) != lv_area_get_height(param)) {
lv_label_revert_dots(label);
lv_label_refr_text(label);
}
} else if(sign == LV_SIGNAL_REFR_EXT_DRAW_PAD) {
if(ext->body_draw) {
const lv_style_t * style = lv_label_get_style(label, LV_LABEL_STYLE_MAIN);
label->ext_draw_pad = LV_MATH_MAX(label->ext_draw_pad, style->body.padding.left);
label->ext_draw_pad = LV_MATH_MAX(label->ext_draw_pad, style->body.padding.right);
label->ext_draw_pad = LV_MATH_MAX(label->ext_draw_pad, style->body.padding.top);
label->ext_draw_pad = LV_MATH_MAX(label->ext_draw_pad, style->body.padding.bottom);
}
}
else if(sign == LV_SIGNAL_BASE_DIR_CHG) {
#if LV_USE_BIDI
if(ext->static_txt == 0) lv_label_set_text(label, NULL);
#endif
} else if(sign == LV_SIGNAL_GET_TYPE) {
lv_obj_type_t * buf = param;
uint8_t i;
for(i = 0; i < LV_MAX_ANCESTOR_NUM - 1; i++) { 
if(buf->type[i] == NULL) break;
}
buf->type[i] = "lv_label";
}
return res;
}
static void lv_label_refr_text(lv_obj_t * label)
{
lv_label_ext_t * ext = lv_obj_get_ext_attr(label);
if(ext->text == NULL) return;
#if LV_LABEL_LONG_TXT_HINT
ext->hint.line_start = -1; 
#endif
lv_coord_t max_w = lv_obj_get_width(label);
const lv_style_t * style = lv_obj_get_style(label);
const lv_font_t * font = style->text.font;
if(ext->long_mode == LV_LABEL_LONG_EXPAND) {
max_w = LV_COORD_MAX;
}
lv_point_t size;
lv_txt_flag_t flag = LV_TXT_FLAG_NONE;
if(ext->recolor != 0) flag |= LV_TXT_FLAG_RECOLOR;
if(ext->expand != 0) flag |= LV_TXT_FLAG_EXPAND;
lv_txt_get_size(&size, ext->text, font, style->text.letter_space, style->text.line_space, max_w, flag);
if(ext->long_mode == LV_LABEL_LONG_EXPAND) {
lv_obj_set_size(label, size.x, size.y);
}
else if(ext->long_mode == LV_LABEL_LONG_SROLL) {
#if LV_USE_ANIMATION
lv_anim_t anim;
anim.var = label;
anim.repeat = 1;
anim.playback = 1;
anim.start = 0;
anim.ready_cb = NULL;
anim.path_cb = lv_anim_path_linear;
anim.playback_pause =
(((lv_font_get_glyph_width(style->text.font, ' ', ' ') + style->text.letter_space) * 1000) /
ext->anim_speed) *
LV_LABEL_WAIT_CHAR_COUNT;
anim.repeat_pause = anim.playback_pause;
anim.act_time = -anim.playback_pause;
bool hor_anim = false;
if(size.x > lv_obj_get_width(label)) {
anim.end = lv_obj_get_width(label) - size.x;
anim.exec_cb = (lv_anim_exec_xcb_t)lv_label_set_offset_x;
anim.time = lv_anim_speed_to_time(ext->anim_speed, anim.start, anim.end);
lv_anim_create(&anim);
hor_anim = true;
} else {
lv_anim_del(label, (lv_anim_exec_xcb_t)lv_label_set_offset_x);
ext->offset.x = 0;
}
if(size.y > lv_obj_get_height(label) && hor_anim == false) {
anim.end = lv_obj_get_height(label) - size.y - (lv_font_get_line_height(font));
anim.exec_cb = (lv_anim_exec_xcb_t)lv_label_set_offset_y;
anim.time = lv_anim_speed_to_time(ext->anim_speed, anim.start, anim.end);
lv_anim_create(&anim);
} else {
lv_anim_del(label, (lv_anim_exec_xcb_t)lv_label_set_offset_y);
ext->offset.y = 0;
}
#endif
}
else if(ext->long_mode == LV_LABEL_LONG_SROLL_CIRC) {
#if LV_USE_ANIMATION
lv_label_align_t align = lv_label_get_align(label);
lv_anim_t anim;
anim.var = label;
anim.repeat = 1;
anim.playback = 0;
anim.act_time = -(((lv_font_get_glyph_width(style->text.font, ' ', ' ') + style->text.letter_space) * 1000) /
ext->anim_speed) *
LV_LABEL_WAIT_CHAR_COUNT;
anim.ready_cb = NULL;
anim.path_cb = lv_anim_path_linear;
anim.playback_pause = 0;
anim.repeat_pause = 0;
bool hor_anim = false;
if(size.x > lv_obj_get_width(label)) {
if(align == LV_LABEL_ALIGN_RIGHT) {
anim.end = 0;
anim.start = -size.x - lv_font_get_glyph_width(font, ' ', ' ') * LV_LABEL_WAIT_CHAR_COUNT;
} else {
anim.start = 0;
anim.end = -size.x - lv_font_get_glyph_width(font, ' ', ' ') * LV_LABEL_WAIT_CHAR_COUNT;
}
anim.exec_cb = (lv_anim_exec_xcb_t)lv_label_set_offset_x;
anim.time = lv_anim_speed_to_time(ext->anim_speed, anim.start, anim.end);
lv_anim_create(&anim);
hor_anim = true;
} else {
lv_anim_del(label, (lv_anim_exec_xcb_t)lv_label_set_offset_x);
ext->offset.x = 0;
}
if(size.y > lv_obj_get_height(label) && hor_anim == false) {
if(align == LV_LABEL_ALIGN_RIGHT) {
anim.end = 0;
anim.start = -size.y - (lv_font_get_line_height(font));
} else {
anim.start = 0;
anim.end = -size.y - (lv_font_get_line_height(font));
}
anim.exec_cb = (lv_anim_exec_xcb_t)lv_label_set_offset_y;
anim.time = lv_anim_speed_to_time(ext->anim_speed, anim.start, anim.end);
lv_anim_create(&anim);
} else {
lv_anim_del(label, (lv_anim_exec_xcb_t)lv_label_set_offset_y);
ext->offset.y = 0;
}
#endif
} else if(ext->long_mode == LV_LABEL_LONG_DOT) {
if(size.y <= lv_obj_get_height(label)) { 
ext->dot_end = LV_LABEL_DOT_END_INV;
} else if(lv_txt_get_encoded_length(ext->text) <= LV_LABEL_DOT_NUM) { 
ext->dot_end = LV_LABEL_DOT_END_INV;
} else {
lv_point_t p;
p.x = lv_obj_get_width(label) -
(lv_font_get_glyph_width(style->text.font, '.', '.') + style->text.letter_space) *
LV_LABEL_DOT_NUM; 
p.y = lv_obj_get_height(label);
p.y -= p.y %
(lv_font_get_line_height(style->text.font) + style->text.line_space); 
p.y -= style->text.line_space; 
uint32_t letter_id = lv_label_get_letter_on(label, &p);
size_t txt_len = strlen(ext->text);
uint32_t byte_id = lv_txt_encoded_get_byte_id(ext->text, letter_id);
while(byte_id + LV_LABEL_DOT_NUM > txt_len) {
byte_id -= lv_txt_encoded_size(&ext->text[byte_id]);
letter_id--;
}
uint32_t byte_id_ori = byte_id;
uint32_t i;
uint8_t len = 0;
for(i = 0; i <= LV_LABEL_DOT_NUM; i++) {
len += lv_txt_encoded_size(&ext->text[byte_id]);
lv_txt_encoded_next(ext->text, &byte_id);
}
if(lv_label_set_dot_tmp(label, &ext->text[byte_id_ori], len)) {
for(i = 0; i < LV_LABEL_DOT_NUM; i++) {
ext->text[byte_id_ori + i] = '.';
}
ext->text[byte_id_ori + LV_LABEL_DOT_NUM] = '\0';
ext->dot_end = letter_id + LV_LABEL_DOT_NUM;
}
}
}
else if(ext->long_mode == LV_LABEL_LONG_BREAK) {
lv_obj_set_height(label, size.y);
}
else if(ext->long_mode == LV_LABEL_LONG_CROP) {
}
lv_obj_invalidate(label);
}
static void lv_label_revert_dots(lv_obj_t * label)
{
lv_label_ext_t * ext = lv_obj_get_ext_attr(label);
if(ext->long_mode != LV_LABEL_LONG_DOT) return;
if(ext->dot_end == LV_LABEL_DOT_END_INV) return;
uint32_t letter_i = ext->dot_end - LV_LABEL_DOT_NUM;
uint32_t byte_i = lv_txt_encoded_get_byte_id(ext->text, letter_i);
uint8_t i = 0;
char * dot_tmp = lv_label_get_dot_tmp(label);
while(ext->text[byte_i + i] != '\0') {
ext->text[byte_i + i] = dot_tmp[i];
i++;
}
ext->text[byte_i + i] = dot_tmp[i];
lv_label_dot_tmp_free(label);
ext->dot_end = LV_LABEL_DOT_END_INV;
}
#if LV_USE_ANIMATION
static void lv_label_set_offset_x(lv_obj_t * label, lv_coord_t x)
{
lv_label_ext_t * ext = lv_obj_get_ext_attr(label);
ext->offset.x = x;
lv_obj_invalidate(label);
}
static void lv_label_set_offset_y(lv_obj_t * label, lv_coord_t y)
{
lv_label_ext_t * ext = lv_obj_get_ext_attr(label);
ext->offset.y = y;
lv_obj_invalidate(label);
}
#endif
static bool lv_label_set_dot_tmp(lv_obj_t * label, char * data, uint16_t len)
{
lv_label_ext_t * ext = lv_obj_get_ext_attr(label);
lv_label_dot_tmp_free(label); 
if(len > sizeof(char *)) {
ext->dot.tmp_ptr = lv_mem_alloc(len + 1);
if(ext->dot.tmp_ptr == NULL) {
LV_LOG_ERROR("Failed to allocate memory for dot_tmp_ptr");
return false;
}
memcpy(ext->dot.tmp_ptr, data, len);
ext->dot.tmp_ptr[len] = '\0';
ext->dot_tmp_alloc = true;
} else {
ext->dot_tmp_alloc = false;
memcpy(ext->dot.tmp, data, len);
}
return true;
}
static char * lv_label_get_dot_tmp(lv_obj_t * label)
{
lv_label_ext_t * ext = lv_obj_get_ext_attr(label);
if(ext->dot_tmp_alloc) {
return ext->dot.tmp_ptr;
} else {
return ext->dot.tmp;
}
}
static void lv_label_dot_tmp_free(lv_obj_t * label)
{
lv_label_ext_t * ext = lv_obj_get_ext_attr(label);
if(ext->dot_tmp_alloc && ext->dot.tmp_ptr) {
lv_mem_free(ext->dot.tmp_ptr);
}
ext->dot_tmp_alloc = false;
ext->dot.tmp_ptr = NULL;
}
#endif
