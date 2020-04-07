#if defined(__cplusplus)
extern "C" {
#endif
#if defined(LV_CONF_INCLUDE_SIMPLE)
#include "lv_conf.h"
#else
#include "../../../lv_conf.h"
#endif
#if LV_USE_TABLE != 0
#if LV_USE_LABEL == 0
#error "lv_table: lv_label is required. Enable it in lv_conf.h (LV_USE_LABEL 1) "
#endif
#include "../lv_core/lv_obj.h"
#include "lv_label.h"
#if !defined(LV_TABLE_COL_MAX)
#define LV_TABLE_COL_MAX 12
#endif
#define LV_TABLE_CELL_STYLE_CNT 4
typedef union
{
struct
{
uint8_t align : 2;
uint8_t right_merge : 1;
uint8_t type : 2;
uint8_t crop : 1;
} s;
uint8_t format_byte;
} lv_table_cell_format_t;
typedef struct
{
uint16_t col_cnt;
uint16_t row_cnt;
char ** cell_data;
const lv_style_t * cell_style[LV_TABLE_CELL_STYLE_CNT];
lv_coord_t col_w[LV_TABLE_COL_MAX];
} lv_table_ext_t;
enum {
LV_TABLE_STYLE_BG,
LV_TABLE_STYLE_CELL1,
LV_TABLE_STYLE_CELL2,
LV_TABLE_STYLE_CELL3,
LV_TABLE_STYLE_CELL4,
};
typedef uint8_t lv_table_style_t;
lv_obj_t * lv_table_create(lv_obj_t * par, const lv_obj_t * copy);
void lv_table_set_cell_value(lv_obj_t * table, uint16_t row, uint16_t col, const char * txt);
void lv_table_set_row_cnt(lv_obj_t * table, uint16_t row_cnt);
void lv_table_set_col_cnt(lv_obj_t * table, uint16_t col_cnt);
void lv_table_set_col_width(lv_obj_t * table, uint16_t col_id, lv_coord_t w);
void lv_table_set_cell_align(lv_obj_t * table, uint16_t row, uint16_t col, lv_label_align_t align);
void lv_table_set_cell_type(lv_obj_t * table, uint16_t row, uint16_t col, uint8_t type);
void lv_table_set_cell_crop(lv_obj_t * table, uint16_t row, uint16_t col, bool crop);
void lv_table_set_cell_merge_right(lv_obj_t * table, uint16_t row, uint16_t col, bool en);
void lv_table_set_style(lv_obj_t * table, lv_table_style_t type, const lv_style_t * style);
const char * lv_table_get_cell_value(lv_obj_t * table, uint16_t row, uint16_t col);
uint16_t lv_table_get_row_cnt(lv_obj_t * table);
uint16_t lv_table_get_col_cnt(lv_obj_t * table);
lv_coord_t lv_table_get_col_width(lv_obj_t * table, uint16_t col_id);
lv_label_align_t lv_table_get_cell_align(lv_obj_t * table, uint16_t row, uint16_t col);
lv_label_align_t lv_table_get_cell_type(lv_obj_t * table, uint16_t row, uint16_t col);
lv_label_align_t lv_table_get_cell_crop(lv_obj_t * table, uint16_t row, uint16_t col);
bool lv_table_get_cell_merge_right(lv_obj_t * table, uint16_t row, uint16_t col);
const lv_style_t * lv_table_get_style(const lv_obj_t * table, lv_table_style_t type);
#endif 
#if defined(__cplusplus)
} 
#endif
