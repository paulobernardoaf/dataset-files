#if defined(__cplusplus)
extern "C" {
#endif
#if defined(LV_CONF_INCLUDE_SIMPLE)
#include "lv_conf.h"
#else
#include "../../../lv_conf.h"
#endif
#if LV_USE_MBOX != 0
#if LV_USE_CONT == 0
#error "lv_mbox: lv_cont is required. Enable it in lv_conf.h (LV_USE_CONT 1) "
#endif
#if LV_USE_BTNM == 0
#error "lv_mbox: lv_btnm is required. Enable it in lv_conf.h (LV_USE_BTNM 1) "
#endif
#if LV_USE_LABEL == 0
#error "lv_mbox: lv_label is required. Enable it in lv_conf.h (LV_USE_LABEL 1) "
#endif
#include "../lv_core/lv_obj.h"
#include "lv_cont.h"
#include "lv_btnm.h"
#include "lv_label.h"
typedef struct
{
lv_cont_ext_t bg; 
lv_obj_t * text; 
lv_obj_t * btnm; 
#if LV_USE_ANIMATION
uint16_t anim_time; 
#endif
} lv_mbox_ext_t;
enum {
LV_MBOX_STYLE_BG,
LV_MBOX_STYLE_BTN_BG, 
LV_MBOX_STYLE_BTN_REL,
LV_MBOX_STYLE_BTN_PR,
LV_MBOX_STYLE_BTN_TGL_REL,
LV_MBOX_STYLE_BTN_TGL_PR,
LV_MBOX_STYLE_BTN_INA,
};
typedef uint8_t lv_mbox_style_t;
lv_obj_t * lv_mbox_create(lv_obj_t * par, const lv_obj_t * copy);
void lv_mbox_add_btns(lv_obj_t * mbox, const char * btn_mapaction[]);
void lv_mbox_set_text(lv_obj_t * mbox, const char * txt);
void lv_mbox_set_anim_time(lv_obj_t * mbox, uint16_t anim_time);
void lv_mbox_start_auto_close(lv_obj_t * mbox, uint16_t delay);
void lv_mbox_stop_auto_close(lv_obj_t * mbox);
void lv_mbox_set_style(lv_obj_t * mbox, lv_mbox_style_t type, const lv_style_t * style);
void lv_mbox_set_recolor(lv_obj_t * mbox, bool en);
const char * lv_mbox_get_text(const lv_obj_t * mbox);
uint16_t lv_mbox_get_active_btn(lv_obj_t * mbox);
const char * lv_mbox_get_active_btn_text(lv_obj_t * mbox);
uint16_t lv_mbox_get_anim_time(const lv_obj_t * mbox);
const lv_style_t * lv_mbox_get_style(const lv_obj_t * mbox, lv_mbox_style_t type);
bool lv_mbox_get_recolor(const lv_obj_t * mbox);
lv_obj_t * lv_mbox_get_btnm(lv_obj_t * mbox);
#endif 
#if defined(__cplusplus)
} 
#endif
