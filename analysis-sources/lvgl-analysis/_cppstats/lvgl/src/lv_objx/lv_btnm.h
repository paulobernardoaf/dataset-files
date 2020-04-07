#if defined(__cplusplus)
extern "C" {
#endif
#if defined(LV_CONF_INCLUDE_SIMPLE)
#include "lv_conf.h"
#else
#include "../../../lv_conf.h"
#endif
#if LV_USE_BTNM != 0
#include "../lv_core/lv_obj.h"
#include "lv_label.h"
#include "lv_btn.h"
#define LV_BTNM_WIDTH_MASK 0x0007
#define LV_BTNM_BTN_NONE 0xFFFF
LV_EXPORT_CONST_INT(LV_BTNM_BTN_NONE);
enum {
LV_BTNM_CTRL_HIDDEN = 0x0008, 
LV_BTNM_CTRL_NO_REPEAT = 0x0010, 
LV_BTNM_CTRL_INACTIVE = 0x0020, 
LV_BTNM_CTRL_TGL_ENABLE = 0x0040, 
LV_BTNM_CTRL_TGL_STATE = 0x0080, 
LV_BTNM_CTRL_CLICK_TRIG = 0x0100, 
};
typedef uint16_t lv_btnm_ctrl_t;
typedef struct
{
const char ** map_p; 
lv_area_t * button_areas; 
lv_btnm_ctrl_t * ctrl_bits; 
const lv_style_t * styles_btn[_LV_BTN_STATE_NUM]; 
uint16_t btn_cnt; 
uint16_t btn_id_pr; 
uint16_t btn_id_act; 
uint8_t recolor : 1; 
uint8_t one_toggle : 1; 
} lv_btnm_ext_t;
enum {
LV_BTNM_STYLE_BG,
LV_BTNM_STYLE_BTN_REL,
LV_BTNM_STYLE_BTN_PR,
LV_BTNM_STYLE_BTN_TGL_REL,
LV_BTNM_STYLE_BTN_TGL_PR,
LV_BTNM_STYLE_BTN_INA,
};
typedef uint8_t lv_btnm_style_t;
lv_obj_t * lv_btnm_create(lv_obj_t * par, const lv_obj_t * copy);
void lv_btnm_set_map(const lv_obj_t * btnm, const char * map[]);
void lv_btnm_set_ctrl_map(const lv_obj_t * btnm, const lv_btnm_ctrl_t ctrl_map[]);
void lv_btnm_set_pressed(const lv_obj_t * btnm, uint16_t id);
void lv_btnm_set_style(lv_obj_t * btnm, lv_btnm_style_t type, const lv_style_t * style);
void lv_btnm_set_recolor(const lv_obj_t * btnm, bool en);
void lv_btnm_set_btn_ctrl(const lv_obj_t * btnm, uint16_t btn_id, lv_btnm_ctrl_t ctrl);
void lv_btnm_clear_btn_ctrl(const lv_obj_t * btnm, uint16_t btn_id, lv_btnm_ctrl_t ctrl);
void lv_btnm_set_btn_ctrl_all(lv_obj_t * btnm, lv_btnm_ctrl_t ctrl);
void lv_btnm_clear_btn_ctrl_all(lv_obj_t * btnm, lv_btnm_ctrl_t ctrl);
void lv_btnm_set_btn_width(const lv_obj_t * btnm, uint16_t btn_id, uint8_t width);
void lv_btnm_set_one_toggle(lv_obj_t * btnm, bool one_toggle);
const char ** lv_btnm_get_map_array(const lv_obj_t * btnm);
bool lv_btnm_get_recolor(const lv_obj_t * btnm);
uint16_t lv_btnm_get_active_btn(const lv_obj_t * btnm);
const char * lv_btnm_get_active_btn_text(const lv_obj_t * btnm);
uint16_t lv_btnm_get_pressed_btn(const lv_obj_t * btnm);
const char * lv_btnm_get_btn_text(const lv_obj_t * btnm, uint16_t btn_id);
bool lv_btnm_get_btn_ctrl(lv_obj_t * btnm, uint16_t btn_id, lv_btnm_ctrl_t ctrl);
const lv_style_t * lv_btnm_get_style(const lv_obj_t * btnm, lv_btnm_style_t type);
bool lv_btnm_get_one_toggle(const lv_obj_t * btnm);
#endif 
#if defined(__cplusplus)
} 
#endif
