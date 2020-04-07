#if defined(__cplusplus)
extern "C" {
#endif
#if defined(LV_CONF_INCLUDE_SIMPLE)
#include "lv_conf.h"
#else
#include "../../../lv_conf.h"
#endif
#if LV_USE_LIST != 0
#if LV_USE_PAGE == 0
#error "lv_list: lv_page is required. Enable it in lv_conf.h (LV_USE_PAGE 1) "
#endif
#if LV_USE_BTN == 0
#error "lv_list: lv_btn is required. Enable it in lv_conf.h (LV_USE_BTN 1) "
#endif
#if LV_USE_LABEL == 0
#error "lv_list: lv_label is required. Enable it in lv_conf.h (LV_USE_LABEL 1) "
#endif
#include "../lv_core/lv_obj.h"
#include "lv_page.h"
#include "lv_btn.h"
#include "lv_label.h"
#include "lv_img.h"
typedef struct
{
lv_page_ext_t page; 
const lv_style_t * styles_btn[_LV_BTN_STATE_NUM]; 
const lv_style_t * style_img; 
uint16_t size; 
uint8_t single_mode : 1; 
#if LV_USE_GROUP
lv_obj_t * last_sel; 
lv_obj_t * selected_btn; 
lv_obj_t * last_clicked_btn;
#endif
} lv_list_ext_t;
enum {
LV_LIST_STYLE_BG, 
LV_LIST_STYLE_SCRL, 
LV_LIST_STYLE_SB, 
LV_LIST_STYLE_EDGE_FLASH, 
LV_LIST_STYLE_BTN_REL, 
LV_LIST_STYLE_BTN_PR,
LV_LIST_STYLE_BTN_TGL_REL,
LV_LIST_STYLE_BTN_TGL_PR,
LV_LIST_STYLE_BTN_INA,
};
typedef uint8_t lv_list_style_t;
lv_obj_t * lv_list_create(lv_obj_t * par, const lv_obj_t * copy);
void lv_list_clean(lv_obj_t * list);
lv_obj_t * lv_list_add_btn(lv_obj_t * list, const void * img_src, const char * txt);
bool lv_list_remove(const lv_obj_t * list, uint16_t index);
void lv_list_set_single_mode(lv_obj_t * list, bool mode);
#if LV_USE_GROUP
void lv_list_set_btn_selected(lv_obj_t * list, lv_obj_t * btn);
#endif
static inline void lv_list_set_sb_mode(lv_obj_t * list, lv_sb_mode_t mode)
{
lv_page_set_sb_mode(list, mode);
}
static inline void lv_list_set_scroll_propagation(lv_obj_t * list, bool en)
{
lv_page_set_scroll_propagation(list, en);
}
static inline void lv_list_set_edge_flash(lv_obj_t * list, bool en)
{
lv_page_set_edge_flash(list, en);
}
static inline void lv_list_set_anim_time(lv_obj_t * list, uint16_t anim_time)
{
lv_page_set_anim_time(list, anim_time);
}
void lv_list_set_style(lv_obj_t * list, lv_list_style_t type, const lv_style_t * style);
void lv_list_set_layout(lv_obj_t * list, lv_layout_t layout);
bool lv_list_get_single_mode(lv_obj_t * list);
const char * lv_list_get_btn_text(const lv_obj_t * btn);
lv_obj_t * lv_list_get_btn_label(const lv_obj_t * btn);
lv_obj_t * lv_list_get_btn_img(const lv_obj_t * btn);
lv_obj_t * lv_list_get_prev_btn(const lv_obj_t * list, lv_obj_t * prev_btn);
lv_obj_t * lv_list_get_next_btn(const lv_obj_t * list, lv_obj_t * prev_btn);
int32_t lv_list_get_btn_index(const lv_obj_t * list, const lv_obj_t * btn);
uint16_t lv_list_get_size(const lv_obj_t * list);
#if LV_USE_GROUP
lv_obj_t * lv_list_get_btn_selected(const lv_obj_t * list);
#endif
lv_layout_t lv_list_get_layout(lv_obj_t * list);
static inline lv_sb_mode_t lv_list_get_sb_mode(const lv_obj_t * list)
{
return lv_page_get_sb_mode(list);
}
static inline bool lv_list_get_scroll_propagation(lv_obj_t * list)
{
return lv_page_get_scroll_propagation(list);
}
static inline bool lv_list_get_edge_flash(lv_obj_t * list)
{
return lv_page_get_edge_flash(list);
}
static inline uint16_t lv_list_get_anim_time(const lv_obj_t * list)
{
return lv_page_get_anim_time(list);
}
const lv_style_t * lv_list_get_style(const lv_obj_t * list, lv_list_style_t type);
void lv_list_up(const lv_obj_t * list);
void lv_list_down(const lv_obj_t * list);
void lv_list_focus(const lv_obj_t * btn, lv_anim_enable_t anim);
#endif 
#if defined(__cplusplus)
} 
#endif
