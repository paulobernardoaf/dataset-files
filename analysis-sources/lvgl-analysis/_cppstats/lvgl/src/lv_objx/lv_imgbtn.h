#if defined(__cplusplus)
extern "C" {
#endif
#if defined(LV_CONF_INCLUDE_SIMPLE)
#include "lv_conf.h"
#else
#include "../../../lv_conf.h"
#endif
#if LV_USE_IMGBTN != 0
#if LV_USE_BTN == 0
#error "lv_imgbtn: lv_btn is required. Enable it in lv_conf.h (LV_USE_BTN 1) "
#endif
#include "../lv_core/lv_obj.h"
#include "lv_btn.h"
#include "../lv_draw/lv_draw_img.h"
typedef struct
{
lv_btn_ext_t btn; 
#if LV_IMGBTN_TILED == 0
const void * img_src[_LV_BTN_STATE_NUM]; 
#else
const void * img_src_left[_LV_BTN_STATE_NUM]; 
const void * img_src_mid[_LV_BTN_STATE_NUM]; 
const void * img_src_right[_LV_BTN_STATE_NUM]; 
#endif
lv_img_cf_t act_cf; 
} lv_imgbtn_ext_t;
enum {
LV_IMGBTN_STYLE_REL, 
LV_IMGBTN_STYLE_PR,
LV_IMGBTN_STYLE_TGL_REL,
LV_IMGBTN_STYLE_TGL_PR,
LV_IMGBTN_STYLE_INA,
};
typedef uint8_t lv_imgbtn_style_t;
lv_obj_t * lv_imgbtn_create(lv_obj_t * par, const lv_obj_t * copy);
#if LV_IMGBTN_TILED == 0
void lv_imgbtn_set_src(lv_obj_t * imgbtn, lv_btn_state_t state, const void * src);
#else
void lv_imgbtn_set_src(lv_obj_t * imgbtn, lv_btn_state_t state, const void * src_left, const void * src_mid,
const void * src_right);
#endif
static inline void lv_imgbtn_set_toggle(lv_obj_t * imgbtn, bool tgl)
{
lv_btn_set_toggle(imgbtn, tgl);
}
static inline void lv_imgbtn_set_state(lv_obj_t * imgbtn, lv_btn_state_t state)
{
lv_btn_set_state(imgbtn, state);
}
static inline void lv_imgbtn_toggle(lv_obj_t * imgbtn)
{
lv_btn_toggle(imgbtn);
}
void lv_imgbtn_set_style(lv_obj_t * imgbtn, lv_imgbtn_style_t type, const lv_style_t * style);
#if LV_IMGBTN_TILED == 0
const void * lv_imgbtn_get_src(lv_obj_t * imgbtn, lv_btn_state_t state);
#else
const void * lv_imgbtn_get_src_left(lv_obj_t * imgbtn, lv_btn_state_t state);
const void * lv_imgbtn_get_src_middle(lv_obj_t * imgbtn, lv_btn_state_t state);
const void * lv_imgbtn_get_src_right(lv_obj_t * imgbtn, lv_btn_state_t state);
#endif
static inline lv_btn_state_t lv_imgbtn_get_state(const lv_obj_t * imgbtn)
{
return lv_btn_get_state(imgbtn);
}
static inline bool lv_imgbtn_get_toggle(const lv_obj_t * imgbtn)
{
return lv_btn_get_toggle(imgbtn);
}
const lv_style_t * lv_imgbtn_get_style(const lv_obj_t * imgbtn, lv_imgbtn_style_t type);
#endif 
#if defined(__cplusplus)
} 
#endif
