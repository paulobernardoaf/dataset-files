




#if !defined(LV_GROUP_H)
#define LV_GROUP_H

#if defined(__cplusplus)
extern "C" {
#endif




#if defined(LV_CONF_INCLUDE_SIMPLE)
#include "lv_conf.h"
#else
#include "../../../lv_conf.h"
#endif

#include "lv_obj.h"







enum {
LV_KEY_UP = 17, 
LV_KEY_DOWN = 18, 
LV_KEY_RIGHT = 19, 
LV_KEY_LEFT = 20, 
LV_KEY_ESC = 27, 
LV_KEY_DEL = 127, 
LV_KEY_BACKSPACE = 8, 
LV_KEY_ENTER = 10, 
LV_KEY_NEXT = 9, 
LV_KEY_PREV = 11, 
LV_KEY_HOME = 2, 
LV_KEY_END = 3, 
};
typedef uint8_t lv_key_t;

#if LV_USE_GROUP != 0



struct _lv_group_t;

typedef void (*lv_group_style_mod_cb_t)(struct _lv_group_t *, lv_style_t *);
typedef void (*lv_group_focus_cb_t)(struct _lv_group_t *);





typedef struct _lv_group_t
{
lv_ll_t obj_ll; 
lv_obj_t ** obj_focus; 

lv_group_style_mod_cb_t style_mod_cb; 
lv_group_style_mod_cb_t style_mod_edit_cb; 
lv_group_focus_cb_t focus_cb; 
lv_style_t style_tmp; 
#if LV_USE_USER_DATA
lv_group_user_data_t user_data;
#endif

uint8_t frozen : 1; 
uint8_t editing : 1; 
uint8_t click_focus : 1; 

uint8_t refocus_policy : 1; 

uint8_t wrap : 1; 

} lv_group_t;

enum { LV_GROUP_REFOCUS_POLICY_NEXT = 0, LV_GROUP_REFOCUS_POLICY_PREV = 1 };
typedef uint8_t lv_group_refocus_policy_t;









void lv_group_init(void);





lv_group_t * lv_group_create(void);





void lv_group_del(lv_group_t * group);






void lv_group_add_obj(lv_group_t * group, lv_obj_t * obj);





void lv_group_remove_obj(lv_obj_t * obj);





void lv_group_remove_all_objs(lv_group_t * group);





void lv_group_focus_obj(lv_obj_t * obj);





void lv_group_focus_next(lv_group_t * group);





void lv_group_focus_prev(lv_group_t * group);






void lv_group_focus_freeze(lv_group_t * group, bool en);







lv_res_t lv_group_send_data(lv_group_t * group, uint32_t c);






void lv_group_set_style_mod_cb(lv_group_t * group, lv_group_style_mod_cb_t style_mod_cb);






void lv_group_set_style_mod_edit_cb(lv_group_t * group, lv_group_style_mod_cb_t style_mod_edit_cb);






void lv_group_set_focus_cb(lv_group_t * group, lv_group_focus_cb_t focus_cb);







void lv_group_set_refocus_policy(lv_group_t * group, lv_group_refocus_policy_t policy);






void lv_group_set_editing(lv_group_t * group, bool edit);






void lv_group_set_click_focus(lv_group_t * group, bool en);






void lv_group_set_wrap(lv_group_t * group, bool en);







lv_style_t * lv_group_mod_style(lv_group_t * group, const lv_style_t * style);






lv_obj_t * lv_group_get_focused(const lv_group_t * group);

#if LV_USE_USER_DATA





lv_group_user_data_t * lv_group_get_user_data(lv_group_t * group);

#endif






lv_group_style_mod_cb_t lv_group_get_style_mod_cb(const lv_group_t * group);






lv_group_style_mod_cb_t lv_group_get_style_mod_edit_cb(const lv_group_t * group);






lv_group_focus_cb_t lv_group_get_focus_cb(const lv_group_t * group);






bool lv_group_get_editing(const lv_group_t * group);






bool lv_group_get_click_focus(const lv_group_t * group);






bool lv_group_get_wrap(lv_group_t * group);






void lv_group_report_style_mod(lv_group_t * group);





#endif 

#if defined(__cplusplus)
} 
#endif

#endif 
