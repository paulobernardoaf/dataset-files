#if defined(__cplusplus)
extern "C" {
#endif
#if defined(LV_CONF_INCLUDE_SIMPLE)
#include "lv_conf.h"
#else
#include "../../../lv_conf.h"
#endif
#if LV_USE_CB != 0
#if LV_USE_BTN == 0
#error "lv_cb: lv_btn is required. Enable it in lv_conf.h (LV_USE_BTN 1) "
#endif
#if LV_USE_LABEL == 0
#error "lv_cb: lv_label is required. Enable it in lv_conf.h (LV_USE_LABEL 1) "
#endif
#include "../lv_core/lv_obj.h"
#include "lv_btn.h"
#include "lv_label.h"
typedef struct
{
lv_btn_ext_t bg_btn; 
lv_obj_t * bullet; 
lv_obj_t * label; 
} lv_cb_ext_t;
enum {
LV_CB_STYLE_BG, 
LV_CB_STYLE_BOX_REL, 
LV_CB_STYLE_BOX_PR, 
LV_CB_STYLE_BOX_TGL_REL, 
LV_CB_STYLE_BOX_TGL_PR, 
LV_CB_STYLE_BOX_INA, 
};
typedef uint8_t lv_cb_style_t;
lv_obj_t * lv_cb_create(lv_obj_t * par, const lv_obj_t * copy);
void lv_cb_set_text(lv_obj_t * cb, const char * txt);
void lv_cb_set_static_text(lv_obj_t * cb, const char * txt);
static inline void lv_cb_set_checked(lv_obj_t * cb, bool checked)
{
lv_btn_set_state(cb, checked ? LV_BTN_STATE_TGL_REL : LV_BTN_STATE_REL);
}
static inline void lv_cb_set_inactive(lv_obj_t * cb)
{
lv_btn_set_state(cb, LV_BTN_STATE_INA);
}
void lv_cb_set_style(lv_obj_t * cb, lv_cb_style_t type, const lv_style_t * style);
const char * lv_cb_get_text(const lv_obj_t * cb);
static inline bool lv_cb_is_checked(const lv_obj_t * cb)
{
return lv_btn_get_state(cb) == LV_BTN_STATE_REL ? false : true;
}
static inline bool lv_cb_is_inactive(const lv_obj_t * cb)
{
return lv_btn_get_state(cb) == LV_BTN_STATE_INA ? true :false;
}
const lv_style_t * lv_cb_get_style(const lv_obj_t * cb, lv_cb_style_t type);
#endif 
#if defined(__cplusplus)
} 
#endif
