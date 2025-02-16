#if defined(__cplusplus)
extern "C" {
#endif
#if defined(LV_CONF_INCLUDE_SIMPLE)
#include "lv_conf.h"
#else
#include "../../../lv_conf.h"
#endif
#if LV_USE_TILEVIEW != 0
#include "../lv_objx/lv_page.h"
typedef struct
{
lv_page_ext_t page;
const lv_point_t * valid_pos;
uint16_t valid_pos_cnt;
#if LV_USE_ANIMATION
uint16_t anim_time;
#endif
lv_point_t act_id;
uint8_t drag_top_en : 1;
uint8_t drag_bottom_en : 1;
uint8_t drag_left_en : 1;
uint8_t drag_right_en : 1;
uint8_t drag_hor : 1;
uint8_t drag_ver : 1;
} lv_tileview_ext_t;
enum {
LV_TILEVIEW_STYLE_MAIN,
};
typedef uint8_t lv_tileview_style_t;
lv_obj_t * lv_tileview_create(lv_obj_t * par, const lv_obj_t * copy);
void lv_tileview_add_element(lv_obj_t * tileview, lv_obj_t * element);
void lv_tileview_set_valid_positions(lv_obj_t * tileview, const lv_point_t valid_pos[], uint16_t valid_pos_cnt);
void lv_tileview_set_tile_act(lv_obj_t * tileview, lv_coord_t x, lv_coord_t y, lv_anim_enable_t anim);
static inline void lv_tileview_set_edge_flash(lv_obj_t * tileview, bool en)
{
lv_page_set_edge_flash(tileview, en);
}
static inline void lv_tileview_set_anim_time(lv_obj_t * tileview, uint16_t anim_time)
{
lv_page_set_anim_time(tileview, anim_time);
}
void lv_tileview_set_style(lv_obj_t * tileview, lv_tileview_style_t type, const lv_style_t * style);
static inline bool lv_tileview_get_edge_flash(lv_obj_t * tileview)
{
return lv_page_get_edge_flash(tileview);
}
static inline uint16_t lv_tileview_get_anim_time(lv_obj_t * tileview)
{
return lv_page_get_anim_time(tileview);
}
const lv_style_t * lv_tileview_get_style(const lv_obj_t * tileview, lv_tileview_style_t type);
#endif 
#if defined(__cplusplus)
} 
#endif
