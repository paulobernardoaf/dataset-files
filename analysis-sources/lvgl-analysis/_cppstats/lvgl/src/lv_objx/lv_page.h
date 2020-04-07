#if defined(__cplusplus)
extern "C" {
#endif
#if defined(LV_CONF_INCLUDE_SIMPLE)
#include "lv_conf.h"
#else
#include "../../../lv_conf.h"
#endif
#if LV_USE_PAGE != 0
#if LV_USE_CONT == 0
#error "lv_page: lv_cont is required. Enable it in lv_conf.h (LV_USE_CONT 1) "
#endif
#include "lv_cont.h"
#include "../lv_core/lv_indev.h"
#include "../lv_misc/lv_anim.h"
enum {
LV_SB_MODE_OFF = 0x0, 
LV_SB_MODE_ON = 0x1, 
LV_SB_MODE_DRAG = 0x2, 
LV_SB_MODE_AUTO = 0x3, 
LV_SB_MODE_HIDE = 0x4, 
LV_SB_MODE_UNHIDE = 0x5, 
};
typedef uint8_t lv_sb_mode_t;
enum { LV_PAGE_EDGE_LEFT = 0x1, LV_PAGE_EDGE_TOP = 0x2, LV_PAGE_EDGE_RIGHT = 0x4, LV_PAGE_EDGE_BOTTOM = 0x8 };
typedef uint8_t lv_page_edge_t;
typedef struct
{
lv_cont_ext_t bg; 
lv_obj_t * scrl; 
struct
{
const lv_style_t * style; 
lv_area_t hor_area; 
lv_area_t ver_area; 
uint8_t hor_draw : 1; 
uint8_t ver_draw : 1; 
lv_sb_mode_t mode : 3; 
} sb;
#if LV_USE_ANIMATION
struct
{
lv_anim_value_t state; 
const lv_style_t * style; 
uint8_t enabled : 1; 
uint8_t top_ip : 1; 
uint8_t bottom_ip : 1; 
uint8_t right_ip : 1; 
uint8_t left_ip : 1; 
} edge_flash;
uint16_t anim_time; 
#endif
uint8_t scroll_prop : 1; 
uint8_t scroll_prop_ip : 1; 
} lv_page_ext_t;
enum {
LV_PAGE_STYLE_BG,
LV_PAGE_STYLE_SCRL,
LV_PAGE_STYLE_SB,
LV_PAGE_STYLE_EDGE_FLASH,
};
typedef uint8_t lv_page_style_t;
lv_obj_t * lv_page_create(lv_obj_t * par, const lv_obj_t * copy);
void lv_page_clean(lv_obj_t * page);
lv_obj_t * lv_page_get_scrl(const lv_obj_t * page);
uint16_t lv_page_get_anim_time(const lv_obj_t * page);
void lv_page_set_sb_mode(lv_obj_t * page, lv_sb_mode_t sb_mode);
void lv_page_set_anim_time(lv_obj_t * page, uint16_t anim_time);
void lv_page_set_scroll_propagation(lv_obj_t * page, bool en);
void lv_page_set_edge_flash(lv_obj_t * page, bool en);
static inline void lv_page_set_scrl_fit4(lv_obj_t * page, lv_fit_t left, lv_fit_t right, lv_fit_t top, lv_fit_t bottom)
{
lv_cont_set_fit4(lv_page_get_scrl(page), left, right, top, bottom);
}
static inline void lv_page_set_scrl_fit2(lv_obj_t * page, lv_fit_t hor, lv_fit_t ver)
{
lv_cont_set_fit2(lv_page_get_scrl(page), hor, ver);
}
static inline void lv_page_set_scrl_fit(lv_obj_t * page, lv_fit_t fit)
{
lv_cont_set_fit(lv_page_get_scrl(page), fit);
}
static inline void lv_page_set_scrl_width(lv_obj_t * page, lv_coord_t w)
{
lv_obj_set_width(lv_page_get_scrl(page), w);
}
static inline void lv_page_set_scrl_height(lv_obj_t * page, lv_coord_t h)
{
lv_obj_set_height(lv_page_get_scrl(page), h);
}
static inline void lv_page_set_scrl_layout(lv_obj_t * page, lv_layout_t layout)
{
lv_cont_set_layout(lv_page_get_scrl(page), layout);
}
void lv_page_set_style(lv_obj_t * page, lv_page_style_t type, const lv_style_t * style);
lv_sb_mode_t lv_page_get_sb_mode(const lv_obj_t * page);
bool lv_page_get_scroll_propagation(lv_obj_t * page);
bool lv_page_get_edge_flash(lv_obj_t * page);
lv_coord_t lv_page_get_fit_width(lv_obj_t * page);
lv_coord_t lv_page_get_fit_height(lv_obj_t * page);
static inline lv_coord_t lv_page_get_scrl_width(const lv_obj_t * page)
{
return lv_obj_get_width(lv_page_get_scrl(page));
}
static inline lv_coord_t lv_page_get_scrl_height(const lv_obj_t * page)
{
return lv_obj_get_height(lv_page_get_scrl(page));
}
static inline lv_layout_t lv_page_get_scrl_layout(const lv_obj_t * page)
{
return lv_cont_get_layout(lv_page_get_scrl(page));
}
static inline lv_fit_t lv_page_get_scrl_fit_left(const lv_obj_t * page)
{
return lv_cont_get_fit_left(lv_page_get_scrl(page));
}
static inline lv_fit_t lv_page_get_scrl_fit_right(const lv_obj_t * page)
{
return lv_cont_get_fit_right(lv_page_get_scrl(page));
}
static inline lv_fit_t lv_page_get_scrl_fit_top(const lv_obj_t * page)
{
return lv_cont_get_fit_top(lv_page_get_scrl(page));
}
static inline lv_fit_t lv_page_get_scrl_fit_bottom(const lv_obj_t * page)
{
return lv_cont_get_fit_bottom(lv_page_get_scrl(page));
}
const lv_style_t * lv_page_get_style(const lv_obj_t * page, lv_page_style_t type);
bool lv_page_on_edge(lv_obj_t * page, lv_page_edge_t edge);
void lv_page_glue_obj(lv_obj_t * obj, bool glue);
void lv_page_focus(lv_obj_t * page, const lv_obj_t * obj, lv_anim_enable_t anim_en);
void lv_page_scroll_hor(lv_obj_t * page, lv_coord_t dist);
void lv_page_scroll_ver(lv_obj_t * page, lv_coord_t dist);
void lv_page_start_edge_flash(lv_obj_t * page);
#endif 
#if defined(__cplusplus)
} 
#endif
