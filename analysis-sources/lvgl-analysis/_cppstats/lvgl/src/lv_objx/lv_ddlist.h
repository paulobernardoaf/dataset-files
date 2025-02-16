#if defined(__cplusplus)
extern "C" {
#endif
#if defined(LV_CONF_INCLUDE_SIMPLE)
#include "lv_conf.h"
#else
#include "../../../lv_conf.h"
#endif
#if LV_USE_DDLIST != 0
#if LV_USE_PAGE == 0
#error "lv_ddlist: lv_page is required. Enable it in lv_conf.h (LV_USE_PAGE 1) "
#endif
#if LV_USE_LABEL == 0
#error "lv_ddlist: lv_label is required. Enable it in lv_conf.h (LV_USE_LABEL 1) "
#endif
#include "../lv_core/lv_obj.h"
#include "../lv_objx/lv_page.h"
#include "../lv_objx/lv_label.h"
typedef struct
{
lv_page_ext_t page; 
lv_obj_t * label; 
const lv_style_t * sel_style; 
uint16_t option_cnt; 
uint16_t sel_opt_id; 
uint16_t sel_opt_id_ori; 
uint8_t opened : 1; 
uint8_t force_sel : 1; 
uint8_t draw_arrow : 1; 
uint8_t stay_open : 1; 
lv_coord_t fix_height; 
} lv_ddlist_ext_t;
enum {
LV_DDLIST_STYLE_BG,
LV_DDLIST_STYLE_SEL,
LV_DDLIST_STYLE_SB,
};
typedef uint8_t lv_ddlist_style_t;
lv_obj_t * lv_ddlist_create(lv_obj_t * par, const lv_obj_t * copy);
void lv_ddlist_set_options(lv_obj_t * ddlist, const char * options);
void lv_ddlist_set_selected(lv_obj_t * ddlist, uint16_t sel_opt);
void lv_ddlist_set_fix_height(lv_obj_t * ddlist, lv_coord_t h);
void lv_ddlist_set_fix_width(lv_obj_t * ddlist, lv_coord_t w);
void lv_ddlist_set_draw_arrow(lv_obj_t * ddlist, bool en);
void lv_ddlist_set_stay_open(lv_obj_t * ddlist, bool en);
static inline void lv_ddlist_set_sb_mode(lv_obj_t * ddlist, lv_sb_mode_t mode)
{
lv_page_set_sb_mode(ddlist, mode);
}
static inline void lv_ddlist_set_anim_time(lv_obj_t * ddlist, uint16_t anim_time)
{
lv_page_set_anim_time(ddlist, anim_time);
}
void lv_ddlist_set_style(lv_obj_t * ddlist, lv_ddlist_style_t type, const lv_style_t * style);
void lv_ddlist_set_align(lv_obj_t * ddlist, lv_label_align_t align);
const char * lv_ddlist_get_options(const lv_obj_t * ddlist);
uint16_t lv_ddlist_get_selected(const lv_obj_t * ddlist);
void lv_ddlist_get_selected_str(const lv_obj_t * ddlist, char * buf, uint16_t buf_size);
lv_coord_t lv_ddlist_get_fix_height(const lv_obj_t * ddlist);
bool lv_ddlist_get_draw_arrow(lv_obj_t * ddlist);
bool lv_ddlist_get_stay_open(lv_obj_t * ddlist);
static inline lv_sb_mode_t lv_ddlist_get_sb_mode(const lv_obj_t * ddlist)
{
return lv_page_get_sb_mode(ddlist);
}
static inline uint16_t lv_ddlist_get_anim_time(const lv_obj_t * ddlist)
{
return lv_page_get_anim_time(ddlist);
}
const lv_style_t * lv_ddlist_get_style(const lv_obj_t * ddlist, lv_ddlist_style_t type);
lv_label_align_t lv_ddlist_get_align(const lv_obj_t * ddlist);
void lv_ddlist_open(lv_obj_t * ddlist, lv_anim_enable_t anim);
void lv_ddlist_close(lv_obj_t * ddlist, lv_anim_enable_t anim);
#endif 
#if defined(__cplusplus)
} 
#endif
