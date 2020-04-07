







#include "lv_table.h"
#if LV_USE_TABLE != 0

#include "../lv_core/lv_debug.h"
#include "../lv_misc/lv_txt.h"
#include "../lv_misc/lv_math.h"
#include "../lv_draw/lv_draw_label.h"
#include "../lv_themes/lv_theme.h"




#define LV_OBJX_NAME "lv_table"








static bool lv_table_design(lv_obj_t * table, const lv_area_t * mask, lv_design_mode_t mode);
static lv_res_t lv_table_signal(lv_obj_t * table, lv_signal_t sign, void * param);
static lv_coord_t get_row_height(lv_obj_t * table, uint16_t row_id);
static void refr_size(lv_obj_t * table);




static lv_signal_cb_t ancestor_signal;
static lv_design_cb_t ancestor_scrl_design;















lv_obj_t * lv_table_create(lv_obj_t * par, const lv_obj_t * copy)
{
LV_LOG_TRACE("table create started");


lv_obj_t * new_table = lv_obj_create(par, copy);
LV_ASSERT_MEM(new_table);
if(new_table == NULL) return NULL;


lv_table_ext_t * ext = lv_obj_allocate_ext_attr(new_table, sizeof(lv_table_ext_t));
LV_ASSERT_MEM(ext);
if(ext == NULL) return NULL;
if(ancestor_signal == NULL) ancestor_signal = lv_obj_get_signal_cb(new_table);
if(ancestor_scrl_design == NULL) ancestor_scrl_design = lv_obj_get_design_cb(new_table);


ext->cell_data = NULL;
ext->cell_style[0] = &lv_style_plain;
ext->cell_style[1] = &lv_style_plain;
ext->cell_style[2] = &lv_style_plain;
ext->cell_style[3] = &lv_style_plain;
ext->col_cnt = 0;
ext->row_cnt = 0;

uint16_t i;
for(i = 0; i < LV_TABLE_COL_MAX; i++) {
ext->col_w[i] = LV_DPI;
}


lv_obj_set_signal_cb(new_table, lv_table_signal);
lv_obj_set_design_cb(new_table, lv_table_design);


if(copy == NULL) {

lv_theme_t * th = lv_theme_get_current();
if(th) {
lv_table_set_style(new_table, LV_TABLE_STYLE_BG, th->style.table.bg);
lv_table_set_style(new_table, LV_TABLE_STYLE_CELL1, th->style.table.cell);
lv_table_set_style(new_table, LV_TABLE_STYLE_CELL2, th->style.table.cell);
lv_table_set_style(new_table, LV_TABLE_STYLE_CELL3, th->style.table.cell);
lv_table_set_style(new_table, LV_TABLE_STYLE_CELL4, th->style.table.cell);
} else {
lv_table_set_style(new_table, LV_TABLE_STYLE_BG, &lv_style_plain_color);
}
lv_obj_set_click(new_table, false); 
}

else {
lv_table_ext_t * copy_ext = lv_obj_get_ext_attr(copy);
ext->cell_style[0] = copy_ext->cell_style[0];
ext->cell_style[1] = copy_ext->cell_style[1];
ext->cell_style[2] = copy_ext->cell_style[2];
ext->cell_style[3] = copy_ext->cell_style[3];
lv_table_set_row_cnt(new_table, copy_ext->row_cnt);
lv_table_set_col_cnt(new_table, copy_ext->col_cnt);


lv_obj_refresh_style(new_table);
}

LV_LOG_INFO("table created");

return new_table;
}













void lv_table_set_cell_value(lv_obj_t * table, uint16_t row, uint16_t col, const char * txt)
{
LV_ASSERT_OBJ(table, LV_OBJX_NAME);
LV_ASSERT_NULL(txt);

lv_table_ext_t * ext = lv_obj_get_ext_attr(table);
if(row >= ext->row_cnt || col >= ext->col_cnt) {
LV_LOG_WARN("lv_table_set_cell_value: invalid row or column");
return;
}
uint32_t cell = row * ext->col_cnt + col;
lv_table_cell_format_t format;


if(ext->cell_data[cell]) {
format.format_byte = ext->cell_data[cell][0];
}

else {
#if LV_USE_BIDI
lv_bidi_dir_t base_dir = lv_obj_get_base_dir(table);
if(base_dir == LV_BIDI_DIR_LTR) format.s.align = LV_LABEL_ALIGN_LEFT;
else if(base_dir == LV_BIDI_DIR_RTL) format.s.align = LV_LABEL_ALIGN_RIGHT;
else if(base_dir == LV_BIDI_DIR_AUTO) format.s.align = lv_bidi_detect_base_dir(txt);
#else
format.s.align = LV_LABEL_ALIGN_LEFT;
#endif

format.s.right_merge = 0;
format.s.type = 0;
format.s.crop = 0;
}

ext->cell_data[cell] = lv_mem_realloc(ext->cell_data[cell], strlen(txt) + 2); 
strcpy(ext->cell_data[cell] + 1, txt); 

ext->cell_data[cell][0] = format.format_byte;
refr_size(table);
}






void lv_table_set_row_cnt(lv_obj_t * table, uint16_t row_cnt)
{
LV_ASSERT_OBJ(table, LV_OBJX_NAME);

lv_table_ext_t * ext = lv_obj_get_ext_attr(table);
uint16_t old_row_cnt = ext->row_cnt;
ext->row_cnt = row_cnt;

if(ext->row_cnt > 0 && ext->col_cnt > 0) {
ext->cell_data = lv_mem_realloc(ext->cell_data, ext->row_cnt * ext->col_cnt * sizeof(char *));


if(old_row_cnt < row_cnt) {
uint16_t old_cell_cnt = old_row_cnt * ext->col_cnt;
uint32_t new_cell_cnt = ext->col_cnt * ext->row_cnt;
memset(&ext->cell_data[old_cell_cnt], 0, (new_cell_cnt - old_cell_cnt) * sizeof(ext->cell_data[0]));
}
} else {
lv_mem_free(ext->cell_data);
ext->cell_data = NULL;
}

refr_size(table);
}






void lv_table_set_col_cnt(lv_obj_t * table, uint16_t col_cnt)
{
LV_ASSERT_OBJ(table, LV_OBJX_NAME);

if(col_cnt >= LV_TABLE_COL_MAX) {
LV_LOG_WARN("lv_table_set_col_cnt: too many columns. Must be < LV_TABLE_COL_MAX.");
return;
}

lv_table_ext_t * ext = lv_obj_get_ext_attr(table);
uint16_t old_col_cnt = ext->col_cnt;
ext->col_cnt = col_cnt;

if(ext->row_cnt > 0 && ext->col_cnt > 0) {
ext->cell_data = lv_mem_realloc(ext->cell_data, ext->row_cnt * ext->col_cnt * sizeof(char *));

if(old_col_cnt < col_cnt) {
uint16_t old_cell_cnt = old_col_cnt * ext->row_cnt;
uint32_t new_cell_cnt = ext->col_cnt * ext->row_cnt;
memset(&ext->cell_data[old_cell_cnt], 0, (new_cell_cnt - old_cell_cnt) * sizeof(ext->cell_data[0]));
}

} else {
lv_mem_free(ext->cell_data);
ext->cell_data = NULL;
}
refr_size(table);
}







void lv_table_set_col_width(lv_obj_t * table, uint16_t col_id, lv_coord_t w)
{
LV_ASSERT_OBJ(table, LV_OBJX_NAME);

if(col_id >= LV_TABLE_COL_MAX) {
LV_LOG_WARN("lv_table_set_col_width: too big 'col_id'. Must be < LV_TABLE_COL_MAX.");
return;
}

lv_table_ext_t * ext = lv_obj_get_ext_attr(table);
ext->col_w[col_id] = w;
refr_size(table);
}








void lv_table_set_cell_align(lv_obj_t * table, uint16_t row, uint16_t col, lv_label_align_t align)
{
LV_ASSERT_OBJ(table, LV_OBJX_NAME);

lv_table_ext_t * ext = lv_obj_get_ext_attr(table);
if(row >= ext->row_cnt || col >= ext->col_cnt) {
LV_LOG_WARN("lv_table_set_cell_align: invalid row or column");
return;
}
uint32_t cell = row * ext->col_cnt + col;

if(ext->cell_data[cell] == NULL) {
ext->cell_data[cell] = lv_mem_alloc(2); 
ext->cell_data[cell][0] = 0;
ext->cell_data[cell][1] = '\0';
}

lv_table_cell_format_t format;
format.format_byte = ext->cell_data[cell][0];
format.s.align = align;
ext->cell_data[cell][0] = format.format_byte;
}








void lv_table_set_cell_type(lv_obj_t * table, uint16_t row, uint16_t col, uint8_t type)
{
LV_ASSERT_OBJ(table, LV_OBJX_NAME);

lv_table_ext_t * ext = lv_obj_get_ext_attr(table);
if(row >= ext->row_cnt || col >= ext->col_cnt) {
LV_LOG_WARN("lv_table_set_cell_type: invalid row or column");
return;
}
uint32_t cell = row * ext->col_cnt + col;

if(ext->cell_data[cell] == NULL) {
ext->cell_data[cell] = lv_mem_alloc(2); 
ext->cell_data[cell][0] = 0;
ext->cell_data[cell][1] = '\0';
}

if(type > 0) type--; 
if(type >= LV_TABLE_CELL_STYLE_CNT) type = LV_TABLE_CELL_STYLE_CNT - 1;

lv_table_cell_format_t format;
format.format_byte = ext->cell_data[cell][0];
format.s.type = type;
ext->cell_data[cell][0] = format.format_byte;
}








void lv_table_set_cell_crop(lv_obj_t * table, uint16_t row, uint16_t col, bool crop)
{
LV_ASSERT_OBJ(table, LV_OBJX_NAME);

lv_table_ext_t * ext = lv_obj_get_ext_attr(table);
if(row >= ext->row_cnt || col >= ext->col_cnt) {
LV_LOG_WARN("lv_table_set_cell_crop: invalid row or column");
return;
}
uint32_t cell = row * ext->col_cnt + col;

if(ext->cell_data[cell] == NULL) {
ext->cell_data[cell] = lv_mem_alloc(2); 
ext->cell_data[cell][0] = 0;
ext->cell_data[cell][1] = '\0';
}

lv_table_cell_format_t format;
format.format_byte = ext->cell_data[cell][0];
format.s.crop = crop;
ext->cell_data[cell][0] = format.format_byte;
}








void lv_table_set_cell_merge_right(lv_obj_t * table, uint16_t row, uint16_t col, bool en)
{
LV_ASSERT_OBJ(table, LV_OBJX_NAME);

lv_table_ext_t * ext = lv_obj_get_ext_attr(table);
if(row >= ext->row_cnt || col >= ext->col_cnt) {
LV_LOG_WARN("lv_table_set_cell_merge_right: invalid row or column");
return;
}

uint32_t cell = row * ext->col_cnt + col;

if(ext->cell_data[cell] == NULL) {
ext->cell_data[cell] = lv_mem_alloc(2); 
ext->cell_data[cell][0] = 0;
ext->cell_data[cell][1] = '\0';
}

lv_table_cell_format_t format;
format.format_byte = ext->cell_data[cell][0];
format.s.right_merge = en ? 1 : 0;
ext->cell_data[cell][0] = format.format_byte;
refr_size(table);
}







void lv_table_set_style(lv_obj_t * table, lv_table_style_t type, const lv_style_t * style)
{
LV_ASSERT_OBJ(table, LV_OBJX_NAME);

lv_table_ext_t * ext = lv_obj_get_ext_attr(table);

switch(type) {
case LV_TABLE_STYLE_BG:
lv_obj_set_style(table, style);
refr_size(table);
break;
case LV_TABLE_STYLE_CELL1:
ext->cell_style[0] = style;
refr_size(table);
break;
case LV_TABLE_STYLE_CELL2:
ext->cell_style[1] = style;
refr_size(table);
break;
case LV_TABLE_STYLE_CELL3:
ext->cell_style[2] = style;
refr_size(table);
break;
case LV_TABLE_STYLE_CELL4:
ext->cell_style[3] = style;
refr_size(table);
break;
}
}












const char * lv_table_get_cell_value(lv_obj_t * table, uint16_t row, uint16_t col)
{
LV_ASSERT_OBJ(table, LV_OBJX_NAME);

lv_table_ext_t * ext = lv_obj_get_ext_attr(table);
if(row >= ext->row_cnt || col >= ext->col_cnt) {
LV_LOG_WARN("lv_table_set_cell_value: invalid row or column");
return "";
}
uint32_t cell = row * ext->col_cnt + col;

if(ext->cell_data[cell] == NULL) return "";

return &ext->cell_data[cell][1]; 
}






uint16_t lv_table_get_row_cnt(lv_obj_t * table)
{
LV_ASSERT_OBJ(table, LV_OBJX_NAME);

lv_table_ext_t * ext = lv_obj_get_ext_attr(table);
return ext->row_cnt;
}






uint16_t lv_table_get_col_cnt(lv_obj_t * table)
{
LV_ASSERT_OBJ(table, LV_OBJX_NAME);

lv_table_ext_t * ext = lv_obj_get_ext_attr(table);
return ext->col_cnt;
}







lv_coord_t lv_table_get_col_width(lv_obj_t * table, uint16_t col_id)
{
LV_ASSERT_OBJ(table, LV_OBJX_NAME);

if(col_id >= LV_TABLE_COL_MAX) {
LV_LOG_WARN("lv_table_set_col_width: too big 'col_id'. Must be < LV_TABLE_COL_MAX.");
return 0;
}

lv_table_ext_t * ext = lv_obj_get_ext_attr(table);
return ext->col_w[col_id];
}









lv_label_align_t lv_table_get_cell_align(lv_obj_t * table, uint16_t row, uint16_t col)
{
LV_ASSERT_OBJ(table, LV_OBJX_NAME);

lv_table_ext_t * ext = lv_obj_get_ext_attr(table);
if(row >= ext->row_cnt || col >= ext->col_cnt) {
LV_LOG_WARN("lv_table_set_cell_align: invalid row or column");
return LV_LABEL_ALIGN_LEFT; 
}
uint32_t cell = row * ext->col_cnt + col;

if(ext->cell_data[cell] == NULL)
return LV_LABEL_ALIGN_LEFT; 
else {
lv_table_cell_format_t format;
format.format_byte = ext->cell_data[cell][0];
return format.s.align;
}
}








lv_label_align_t lv_table_get_cell_type(lv_obj_t * table, uint16_t row, uint16_t col)
{
LV_ASSERT_OBJ(table, LV_OBJX_NAME);

lv_table_ext_t * ext = lv_obj_get_ext_attr(table);
if(row >= ext->row_cnt || col >= ext->col_cnt) {
LV_LOG_WARN("lv_table_get_cell_type: invalid row or column");
return 1; 
}
uint32_t cell = row * ext->col_cnt + col;

if(ext->cell_data[cell] == NULL)
return 1; 
else {
lv_table_cell_format_t format;
format.format_byte = ext->cell_data[cell][0];
return format.s.type + 1; 
}
}








lv_label_align_t lv_table_get_cell_crop(lv_obj_t * table, uint16_t row, uint16_t col)
{
LV_ASSERT_OBJ(table, LV_OBJX_NAME);

lv_table_ext_t * ext = lv_obj_get_ext_attr(table);
if(row >= ext->row_cnt || col >= ext->col_cnt) {
LV_LOG_WARN("lv_table_get_cell_crop: invalid row or column");
return false; 
}
uint32_t cell = row * ext->col_cnt + col;

if(ext->cell_data[cell] == NULL)
return false; 
else {
lv_table_cell_format_t format;
format.format_byte = ext->cell_data[cell][0];
return format.s.crop;
}
}








bool lv_table_get_cell_merge_right(lv_obj_t * table, uint16_t row, uint16_t col)
{
LV_ASSERT_OBJ(table, LV_OBJX_NAME);

lv_table_ext_t * ext = lv_obj_get_ext_attr(table);
if(row >= ext->row_cnt || col >= ext->col_cnt) {
LV_LOG_WARN("lv_table_get_cell_merge_right: invalid row or column");
return false;
}

uint32_t cell = row * ext->col_cnt + col;

if(ext->cell_data[cell] == NULL)
return false;
else {
lv_table_cell_format_t format;
format.format_byte = ext->cell_data[cell][0];
return format.s.right_merge ? true : false;
}
}







const lv_style_t * lv_table_get_style(const lv_obj_t * table, lv_table_style_t type)
{
LV_ASSERT_OBJ(table, LV_OBJX_NAME);

lv_table_ext_t * ext = lv_obj_get_ext_attr(table);
const lv_style_t * style = NULL;

switch(type) {
case LV_TABLE_STYLE_BG: style = lv_obj_get_style(table); break;
case LV_TABLE_STYLE_CELL1: style = ext->cell_style[0]; break;
case LV_TABLE_STYLE_CELL2: style = ext->cell_style[1]; break;
case LV_TABLE_STYLE_CELL3: style = ext->cell_style[2]; break;
case LV_TABLE_STYLE_CELL4: style = ext->cell_style[3]; break;
default: return NULL;
}

return style;
}















static bool lv_table_design(lv_obj_t * table, const lv_area_t * mask, lv_design_mode_t mode)
{

if(mode == LV_DESIGN_COVER_CHK) {
return false;
}

else if(mode == LV_DESIGN_DRAW_MAIN) {
ancestor_scrl_design(table, mask, mode);

lv_table_ext_t * ext = lv_obj_get_ext_attr(table);
const lv_style_t * bg_style = lv_obj_get_style(table);
lv_coord_t h_row;
lv_point_t txt_size;
lv_area_t cell_area;
lv_area_t txt_area;
lv_txt_flag_t txt_flags;
lv_opa_t opa_scale = lv_obj_get_opa_scale(table);

uint16_t col;
uint16_t row;
uint16_t cell = 0;

cell_area.y2 = table->coords.y1 + bg_style->body.padding.top;
for(row = 0; row < ext->row_cnt; row++) {
h_row = get_row_height(table, row);

cell_area.y1 = cell_area.y2 + 1;
cell_area.y2 = cell_area.y1 + h_row - 1;

cell_area.x2 = table->coords.x1 + bg_style->body.padding.left;

for(col = 0; col < ext->col_cnt; col++) {

lv_table_cell_format_t format;
if(ext->cell_data[cell]) {
format.format_byte = ext->cell_data[cell][0];
} else {
format.s.right_merge = 0;
format.s.align = LV_LABEL_ALIGN_LEFT;
format.s.type = 0;
format.s.crop = 1;
}


lv_style_t cell_style;
lv_style_copy(&cell_style, ext->cell_style[format.s.type]);
cell_area.x1 = cell_area.x2 + 1;
cell_area.x2 = cell_area.x1 + ext->col_w[col] - 1;

uint16_t col_merge = 0;
for(col_merge = 0; col_merge + col < ext->col_cnt - 1; col_merge++) {

if(ext->cell_data[cell + col_merge] != NULL) {
format.format_byte = ext->cell_data[cell + col_merge][0];
if(format.s.right_merge)
cell_area.x2 += ext->col_w[col + col_merge + 1];
else
break;
} else {
break;
}
}

lv_draw_rect(&cell_area, mask, &cell_style, opa_scale);

if(ext->cell_data[cell]) {

txt_area.x1 = cell_area.x1 + cell_style.body.padding.left;
txt_area.x2 = cell_area.x2 - cell_style.body.padding.right;
txt_area.y1 = cell_area.y1 + cell_style.body.padding.top;
txt_area.y2 = cell_area.y2 - cell_style.body.padding.bottom;

if(format.s.crop == 0) {
txt_flags = LV_TXT_FLAG_NONE;
} else {
txt_flags = LV_TXT_FLAG_EXPAND;
}

lv_txt_get_size(&txt_size, ext->cell_data[cell] + 1, cell_style.text.font,
cell_style.text.letter_space, cell_style.text.line_space,
lv_area_get_width(&txt_area), txt_flags);


if(format.s.crop == 0) {
txt_area.y1 = cell_area.y1 + h_row / 2 - txt_size.y / 2;
txt_area.y2 = cell_area.y1 + h_row / 2 + txt_size.y / 2;
}

switch(format.s.align) {
default:
case LV_LABEL_ALIGN_LEFT: txt_flags |= LV_TXT_FLAG_NONE; break;
case LV_LABEL_ALIGN_RIGHT: txt_flags |= LV_TXT_FLAG_RIGHT; break;
case LV_LABEL_ALIGN_CENTER: txt_flags |= LV_TXT_FLAG_CENTER; break;
}

lv_area_t label_mask;
bool label_mask_ok;
label_mask_ok = lv_area_intersect(&label_mask, mask, &cell_area);
if(label_mask_ok) {
lv_draw_label(&txt_area, &label_mask, &cell_style, opa_scale, ext->cell_data[cell] + 1,
txt_flags, NULL, NULL, NULL, lv_obj_get_base_dir(table));
}

lv_point_t p1;
lv_point_t p2;
p1.x = cell_area.x1;
p2.x = cell_area.x2;
uint16_t i;
for(i = 1; ext->cell_data[cell][i] != '\0'; i++) {
if(ext->cell_data[cell][i] == '\n') {
ext->cell_data[cell][i] = '\0';
lv_txt_get_size(&txt_size, ext->cell_data[cell] + 1, cell_style.text.font,
cell_style.text.letter_space, cell_style.text.line_space,
lv_area_get_width(&txt_area), txt_flags);

p1.y = txt_area.y1 + txt_size.y + cell_style.text.line_space / 2;
p2.y = txt_area.y1 + txt_size.y + cell_style.text.line_space / 2;
lv_draw_line(&p1, &p2, mask, &cell_style, opa_scale);

ext->cell_data[cell][i] = '\n';
}
}
}

cell += col_merge + 1;
col += col_merge;
}
}
}

else if(mode == LV_DESIGN_DRAW_POST) {
}

return true;
}








static lv_res_t lv_table_signal(lv_obj_t * table, lv_signal_t sign, void * param)
{
lv_res_t res;


res = ancestor_signal(table, sign, param);
if(res != LV_RES_OK) return res;
if(sign == LV_SIGNAL_GET_TYPE) return lv_obj_handle_get_type_signal(param, LV_OBJX_NAME);

if(sign == LV_SIGNAL_CLEANUP) {

lv_table_ext_t * ext = lv_obj_get_ext_attr(table);
uint16_t cell;
for(cell = 0; cell < ext->col_cnt * ext->row_cnt; cell++) {
if(ext->cell_data[cell]) {
lv_mem_free(ext->cell_data[cell]);
ext->cell_data[cell] = NULL;
}
}
if(ext->cell_data != NULL)
lv_mem_free(ext->cell_data);
}

return res;
}

static void refr_size(lv_obj_t * table)
{
lv_coord_t h = 0;
lv_coord_t w = 0;

lv_table_ext_t * ext = lv_obj_get_ext_attr(table);

uint16_t i;
for(i = 0; i < ext->col_cnt; i++) {
w += ext->col_w[i];
}
for(i = 0; i < ext->row_cnt; i++) {
h += get_row_height(table, i);
}

const lv_style_t * bg_style = lv_obj_get_style(table);

w += bg_style->body.padding.left + bg_style->body.padding.right;
h += bg_style->body.padding.top + bg_style->body.padding.bottom;

lv_obj_set_size(table, w + 1, h + 1);
lv_obj_invalidate(table);
}

static lv_coord_t get_row_height(lv_obj_t * table, uint16_t row_id)
{
lv_table_ext_t * ext = lv_obj_get_ext_attr(table);
lv_point_t txt_size;
lv_coord_t txt_w;
const lv_style_t * cell_style;

uint16_t row_start = row_id * ext->col_cnt;
uint16_t cell;
uint16_t col;
lv_coord_t h_max = lv_font_get_line_height(ext->cell_style[0]->text.font) + ext->cell_style[0]->body.padding.top +
ext->cell_style[0]->body.padding.bottom;

for(cell = row_start, col = 0; cell < row_start + ext->col_cnt; cell++, col++) {
if(ext->cell_data[cell] != NULL) {

txt_w = ext->col_w[col];
uint16_t col_merge = 0;
for(col_merge = 0; col_merge + col < ext->col_cnt - 1; col_merge++) {

if(ext->cell_data[cell + col_merge] != NULL) {
lv_table_cell_format_t format;
format.format_byte = ext->cell_data[cell + col_merge][0];
if(format.s.right_merge)
txt_w += ext->col_w[col + col_merge + 1];
else
break;
} else {
break;
}
}

lv_table_cell_format_t format;
format.format_byte = ext->cell_data[cell][0];
cell_style = ext->cell_style[format.s.type];


if(format.s.crop) {
h_max = LV_MATH_MAX(lv_font_get_line_height(cell_style->text.font) + cell_style->body.padding.top +
cell_style->body.padding.bottom,
h_max);
}

else {
txt_w -= cell_style->body.padding.left + cell_style->body.padding.right;

lv_txt_get_size(&txt_size, ext->cell_data[cell] + 1, cell_style->text.font,
cell_style->text.letter_space, cell_style->text.line_space, txt_w, LV_TXT_FLAG_NONE);

h_max = LV_MATH_MAX(txt_size.y + cell_style->body.padding.top + cell_style->body.padding.bottom, h_max);
cell += col_merge;
col += col_merge;
}
}
}

return h_max;
}

#endif
