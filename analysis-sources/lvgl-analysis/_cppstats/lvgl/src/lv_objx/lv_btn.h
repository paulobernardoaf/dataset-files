#if defined(__cplusplus)
extern "C" {
#endif
#if defined(LV_CONF_INCLUDE_SIMPLE)
#include "lv_conf.h"
#else
#include "../../../lv_conf.h"
#endif
#if LV_USE_BTN != 0
#if LV_USE_CONT == 0
#error "lv_btn: lv_cont is required. Enable it in lv_conf.h (LV_USE_CONT 1) "
#endif
#include "lv_cont.h"
#include "../lv_core/lv_indev.h"
enum {
LV_BTN_STATE_REL,
LV_BTN_STATE_PR,
LV_BTN_STATE_TGL_REL,
LV_BTN_STATE_TGL_PR,
LV_BTN_STATE_INA,
_LV_BTN_STATE_NUM,
};
typedef uint8_t lv_btn_state_t;
typedef struct
{
lv_cont_ext_t cont;
const lv_style_t * styles[_LV_BTN_STATE_NUM];
#if LV_BTN_INK_EFFECT
uint16_t ink_in_time;
uint16_t ink_wait_time;
uint16_t ink_out_time;
#endif
lv_btn_state_t state : 3;
uint8_t toggle : 1;
} lv_btn_ext_t;
enum {
LV_BTN_STYLE_REL,
LV_BTN_STYLE_PR,
LV_BTN_STYLE_TGL_REL,
LV_BTN_STYLE_TGL_PR,
LV_BTN_STYLE_INA,
};
typedef uint8_t lv_btn_style_t;
lv_obj_t * lv_btn_create(lv_obj_t * par, const lv_obj_t * copy);
void lv_btn_set_toggle(lv_obj_t * btn, bool tgl);
void lv_btn_set_state(lv_obj_t * btn, lv_btn_state_t state);
void lv_btn_toggle(lv_obj_t * btn);
static inline void lv_btn_set_layout(lv_obj_t * btn, lv_layout_t layout)
{
lv_cont_set_layout(btn, layout);
}
static inline void lv_btn_set_fit4(lv_obj_t * btn, lv_fit_t left, lv_fit_t right, lv_fit_t top, lv_fit_t bottom)
{
lv_cont_set_fit4(btn, left, right, top, bottom);
}
static inline void lv_btn_set_fit2(lv_obj_t * btn, lv_fit_t hor, lv_fit_t ver)
{
lv_cont_set_fit2(btn, hor, ver);
}
static inline void lv_btn_set_fit(lv_obj_t * btn, lv_fit_t fit)
{
lv_cont_set_fit(btn, fit);
}
void lv_btn_set_ink_in_time(lv_obj_t * btn, uint16_t time);
void lv_btn_set_ink_wait_time(lv_obj_t * btn, uint16_t time);
void lv_btn_set_ink_out_time(lv_obj_t * btn, uint16_t time);
void lv_btn_set_style(lv_obj_t * btn, lv_btn_style_t type, const lv_style_t * style);
lv_btn_state_t lv_btn_get_state(const lv_obj_t * btn);
bool lv_btn_get_toggle(const lv_obj_t * btn);
static inline lv_layout_t lv_btn_get_layout(const lv_obj_t * btn)
{
return lv_cont_get_layout(btn);
}
static inline lv_fit_t lv_btn_get_fit_left(const lv_obj_t * btn)
{
return lv_cont_get_fit_left(btn);
}
static inline lv_fit_t lv_btn_get_fit_right(const lv_obj_t * btn)
{
return lv_cont_get_fit_right(btn);
}
static inline lv_fit_t lv_btn_get_fit_top(const lv_obj_t * btn)
{
return lv_cont_get_fit_top(btn);
}
static inline lv_fit_t lv_btn_get_fit_bottom(const lv_obj_t * btn)
{
return lv_cont_get_fit_bottom(btn);
}
uint16_t lv_btn_get_ink_in_time(const lv_obj_t * btn);
uint16_t lv_btn_get_ink_wait_time(const lv_obj_t * btn);
uint16_t lv_btn_get_ink_out_time(const lv_obj_t * btn);
const lv_style_t * lv_btn_get_style(const lv_obj_t * btn, lv_btn_style_t type);
#endif 
#if defined(__cplusplus)
} 
#endif
