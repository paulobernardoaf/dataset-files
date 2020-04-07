




#if !defined(LV_WIN_H)
#define LV_WIN_H

#if defined(__cplusplus)
extern "C" {
#endif




#if defined(LV_CONF_INCLUDE_SIMPLE)
#include "lv_conf.h"
#else
#include "../../../lv_conf.h"
#endif

#if LV_USE_WIN != 0


#if LV_USE_BTN == 0
#error "lv_win: lv_btn is required. Enable it in lv_conf.h (LV_USE_BTN 1) "
#endif

#if LV_USE_LABEL == 0
#error "lv_win: lv_label is required. Enable it in lv_conf.h (LV_USE_LABEL 1) "
#endif

#if LV_USE_IMG == 0
#error "lv_win: lv_img is required. Enable it in lv_conf.h (LV_USE_IMG 1) "
#endif

#if LV_USE_PAGE == 0
#error "lv_win: lv_page is required. Enable it in lv_conf.h (LV_USE_PAGE 1) "
#endif

#include "../lv_core/lv_obj.h"
#include "lv_cont.h"
#include "lv_btn.h"
#include "lv_label.h"
#include "lv_img.h"
#include "lv_page.h"










typedef struct
{


lv_obj_t * page; 
lv_obj_t * header; 
lv_obj_t * title; 
const lv_style_t * style_btn_rel; 
const lv_style_t * style_btn_pr; 
lv_coord_t btn_size; 
} lv_win_ext_t;


enum {
LV_WIN_STYLE_BG, 
LV_WIN_STYLE_CONTENT, 
LV_WIN_STYLE_SB, 
LV_WIN_STYLE_HEADER, 
LV_WIN_STYLE_BTN_REL, 
LV_WIN_STYLE_BTN_PR,
};
typedef uint8_t lv_win_style_t;











lv_obj_t * lv_win_create(lv_obj_t * par, const lv_obj_t * copy);





void lv_win_clean(lv_obj_t * win);











lv_obj_t * lv_win_add_btn(lv_obj_t * win, const void * img_src);










void lv_win_close_event_cb(lv_obj_t * btn, lv_event_t event);






void lv_win_set_title(lv_obj_t * win, const char * title);






void lv_win_set_btn_size(lv_obj_t * win, lv_coord_t size);








void lv_win_set_content_size(lv_obj_t * win, lv_coord_t w, lv_coord_t h);






void lv_win_set_layout(lv_obj_t * win, lv_layout_t layout);






void lv_win_set_sb_mode(lv_obj_t * win, lv_sb_mode_t sb_mode);






void lv_win_set_anim_time(lv_obj_t * win, uint16_t anim_time);







void lv_win_set_style(lv_obj_t * win, lv_win_style_t type, const lv_style_t * style);






void lv_win_set_drag(lv_obj_t * win, bool en);










const char * lv_win_get_title(const lv_obj_t * win);






lv_obj_t * lv_win_get_content(const lv_obj_t * win);






lv_coord_t lv_win_get_btn_size(const lv_obj_t * win);







lv_obj_t * lv_win_get_from_btn(const lv_obj_t * ctrl_btn);






lv_layout_t lv_win_get_layout(lv_obj_t * win);






lv_sb_mode_t lv_win_get_sb_mode(lv_obj_t * win);






uint16_t lv_win_get_anim_time(const lv_obj_t * win);






lv_coord_t lv_win_get_width(lv_obj_t * win);







const lv_style_t * lv_win_get_style(const lv_obj_t * win, lv_win_style_t type);






static inline bool lv_win_get_drag(const lv_obj_t * win)
{
return lv_obj_get_drag(win);
}











void lv_win_focus(lv_obj_t * win, lv_obj_t * obj, lv_anim_enable_t anim_en);






static inline void lv_win_scroll_hor(lv_obj_t * win, lv_coord_t dist)
{
lv_win_ext_t * ext = (lv_win_ext_t *)lv_obj_get_ext_attr(win);
lv_page_scroll_hor(ext->page, dist);
}





static inline void lv_win_scroll_ver(lv_obj_t * win, lv_coord_t dist)
{
lv_win_ext_t * ext = (lv_win_ext_t *)lv_obj_get_ext_attr(win);
lv_page_scroll_ver(ext->page, dist);
}





#endif 

#if defined(__cplusplus)
} 
#endif

#endif 
