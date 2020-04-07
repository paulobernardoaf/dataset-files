#if defined(__cplusplus)
extern "C" {
#endif
#if defined(LV_CONF_INCLUDE_SIMPLE)
#include "lv_conf.h"
#else
#include "../../../lv_conf.h"
#endif
#include <stddef.h>
#include <stdbool.h>
#include "lv_style.h"
#include "../lv_misc/lv_types.h"
#include "../lv_misc/lv_area.h"
#include "../lv_misc/lv_mem.h"
#include "../lv_misc/lv_ll.h"
#include "../lv_misc/lv_color.h"
#include "../lv_misc/lv_log.h"
#include "../lv_misc/lv_bidi.h"
#include "../lv_hal/lv_hal.h"
#if LV_HOR_RES_MAX == 0 || LV_VER_RES_MAX == 0
#error "LittlevGL: LV_HOR_RES_MAX and LV_VER_RES_MAX must be greater than 0"
#endif
#if LV_ANTIALIAS > 1
#error "LittlevGL: LV_ANTIALIAS can be only 0 or 1"
#endif
#define LV_MAX_ANCESTOR_NUM 8
#define LV_EXT_CLICK_AREA_OFF 0
#define LV_EXT_CLICK_AREA_TINY 1
#define LV_EXT_CLICK_AREA_FULL 2
struct _lv_obj_t;
enum {
LV_DESIGN_DRAW_MAIN, 
LV_DESIGN_DRAW_POST, 
LV_DESIGN_COVER_CHK, 
};
typedef uint8_t lv_design_mode_t;
typedef bool (*lv_design_cb_t)(struct _lv_obj_t * obj, const lv_area_t * mask_p, lv_design_mode_t mode);
enum {
LV_EVENT_PRESSED, 
LV_EVENT_PRESSING, 
LV_EVENT_PRESS_LOST, 
LV_EVENT_SHORT_CLICKED, 
LV_EVENT_LONG_PRESSED, 
LV_EVENT_LONG_PRESSED_REPEAT, 
LV_EVENT_CLICKED, 
LV_EVENT_RELEASED, 
LV_EVENT_DRAG_BEGIN, 
LV_EVENT_DRAG_END,
LV_EVENT_DRAG_THROW_BEGIN,
LV_EVENT_KEY,
LV_EVENT_FOCUSED,
LV_EVENT_DEFOCUSED,
LV_EVENT_VALUE_CHANGED, 
LV_EVENT_INSERT,
LV_EVENT_REFRESH,
LV_EVENT_APPLY, 
LV_EVENT_CANCEL, 
LV_EVENT_DELETE, 
};
typedef uint8_t lv_event_t; 
typedef void (*lv_event_cb_t)(struct _lv_obj_t * obj, lv_event_t event);
enum {
LV_SIGNAL_CLEANUP, 
LV_SIGNAL_CHILD_CHG, 
LV_SIGNAL_CORD_CHG, 
LV_SIGNAL_PARENT_SIZE_CHG, 
LV_SIGNAL_STYLE_CHG, 
LV_SIGNAL_BASE_DIR_CHG, 
LV_SIGNAL_REFR_EXT_DRAW_PAD, 
LV_SIGNAL_GET_TYPE, 
LV_SIGNAL_PRESSED, 
LV_SIGNAL_PRESSING, 
LV_SIGNAL_PRESS_LOST, 
LV_SIGNAL_RELEASED, 
LV_SIGNAL_LONG_PRESS, 
LV_SIGNAL_LONG_PRESS_REP, 
LV_SIGNAL_DRAG_BEGIN, 
LV_SIGNAL_DRAG_END,
LV_SIGNAL_FOCUS,
LV_SIGNAL_DEFOCUS,
LV_SIGNAL_CONTROL,
LV_SIGNAL_GET_EDITABLE,
};
typedef uint8_t lv_signal_t;
typedef lv_res_t (*lv_signal_cb_t)(struct _lv_obj_t * obj, lv_signal_t sign, void * param);
enum {
LV_ALIGN_CENTER = 0,
LV_ALIGN_IN_TOP_LEFT,
LV_ALIGN_IN_TOP_MID,
LV_ALIGN_IN_TOP_RIGHT,
LV_ALIGN_IN_BOTTOM_LEFT,
LV_ALIGN_IN_BOTTOM_MID,
LV_ALIGN_IN_BOTTOM_RIGHT,
LV_ALIGN_IN_LEFT_MID,
LV_ALIGN_IN_RIGHT_MID,
LV_ALIGN_OUT_TOP_LEFT,
LV_ALIGN_OUT_TOP_MID,
LV_ALIGN_OUT_TOP_RIGHT,
LV_ALIGN_OUT_BOTTOM_LEFT,
LV_ALIGN_OUT_BOTTOM_MID,
LV_ALIGN_OUT_BOTTOM_RIGHT,
LV_ALIGN_OUT_LEFT_TOP,
LV_ALIGN_OUT_LEFT_MID,
LV_ALIGN_OUT_LEFT_BOTTOM,
LV_ALIGN_OUT_RIGHT_TOP,
LV_ALIGN_OUT_RIGHT_MID,
LV_ALIGN_OUT_RIGHT_BOTTOM,
};
typedef uint8_t lv_align_t;
#if LV_USE_OBJ_REALIGN
typedef struct
{
const struct _lv_obj_t * base;
lv_coord_t xofs;
lv_coord_t yofs;
lv_align_t align;
uint8_t auto_realign : 1;
uint8_t origo_align : 1; 
} lv_reailgn_t;
#endif
enum {
LV_DRAG_DIR_HOR = 0x1, 
LV_DRAG_DIR_VER = 0x2, 
LV_DRAG_DIR_ALL = 0x3, 
};
typedef uint8_t lv_drag_dir_t;
typedef struct _lv_obj_t
{
struct _lv_obj_t * par; 
lv_ll_t child_ll; 
lv_area_t coords; 
lv_event_cb_t event_cb; 
lv_signal_cb_t signal_cb; 
lv_design_cb_t design_cb; 
void * ext_attr; 
const lv_style_t * style_p; 
#if LV_USE_GROUP != 0
void * group_p; 
#endif
#if LV_USE_EXT_CLICK_AREA == LV_EXT_CLICK_AREA_TINY
uint8_t ext_click_pad_hor; 
uint8_t ext_click_pad_ver; 
#endif
#if LV_USE_EXT_CLICK_AREA == LV_EXT_CLICK_AREA_FULL
lv_area_t ext_click_pad; 
#endif
uint8_t click : 1; 
uint8_t drag : 1; 
uint8_t drag_throw : 1; 
uint8_t drag_parent : 1; 
uint8_t hidden : 1; 
uint8_t top : 1; 
uint8_t opa_scale_en : 1; 
uint8_t parent_event : 1; 
lv_drag_dir_t drag_dir : 2; 
lv_bidi_dir_t base_dir : 2; 
uint8_t reserved : 3; 
uint8_t protect; 
lv_opa_t opa_scale; 
lv_coord_t ext_draw_pad; 
#if LV_USE_OBJ_REALIGN
lv_reailgn_t realign; 
#endif
#if LV_USE_USER_DATA
lv_obj_user_data_t user_data; 
#endif
} lv_obj_t;
enum {
LV_PROTECT_NONE = 0x00,
LV_PROTECT_CHILD_CHG = 0x01, 
LV_PROTECT_PARENT = 0x02, 
LV_PROTECT_POS = 0x04, 
LV_PROTECT_FOLLOW = 0x08, 
LV_PROTECT_PRESS_LOST = 0x10, 
LV_PROTECT_CLICK_FOCUS = 0x20, 
};
typedef uint8_t lv_protect_t;
typedef struct
{
const char * type[LV_MAX_ANCESTOR_NUM]; 
} lv_obj_type_t;
void lv_init(void);
#if LV_ENABLE_GC || !LV_MEM_CUSTOM
void lv_deinit(void);
#endif
lv_obj_t * lv_obj_create(lv_obj_t * parent, const lv_obj_t * copy);
lv_res_t lv_obj_del(lv_obj_t * obj);
void lv_obj_del_async(struct _lv_obj_t *obj);
void lv_obj_clean(lv_obj_t * obj);
void lv_obj_invalidate_area(const lv_obj_t * obj, const lv_area_t * area);
void lv_obj_invalidate(const lv_obj_t * obj);
void lv_obj_set_parent(lv_obj_t * obj, lv_obj_t * parent);
void lv_obj_move_foreground(lv_obj_t * obj);
void lv_obj_move_background(lv_obj_t * obj);
void lv_obj_set_pos(lv_obj_t * obj, lv_coord_t x, lv_coord_t y);
void lv_obj_set_x(lv_obj_t * obj, lv_coord_t x);
void lv_obj_set_y(lv_obj_t * obj, lv_coord_t y);
void lv_obj_set_size(lv_obj_t * obj, lv_coord_t w, lv_coord_t h);
void lv_obj_set_width(lv_obj_t * obj, lv_coord_t w);
void lv_obj_set_height(lv_obj_t * obj, lv_coord_t h);
void lv_obj_align(lv_obj_t * obj, const lv_obj_t * base, lv_align_t align, lv_coord_t x_mod, lv_coord_t y_mod);
void lv_obj_align_origo(lv_obj_t * obj, const lv_obj_t * base, lv_align_t align, lv_coord_t x_mod, lv_coord_t y_mod);
void lv_obj_realign(lv_obj_t * obj);
void lv_obj_set_auto_realign(lv_obj_t * obj, bool en);
void lv_obj_set_ext_click_area(lv_obj_t * obj, lv_coord_t left, lv_coord_t right, lv_coord_t top, lv_coord_t bottom);
void lv_obj_set_style(lv_obj_t * obj, const lv_style_t * style);
void lv_obj_refresh_style(lv_obj_t * obj);
void lv_obj_report_style_mod(lv_style_t * style);
void lv_obj_set_hidden(lv_obj_t * obj, bool en);
void lv_obj_set_click(lv_obj_t * obj, bool en);
void lv_obj_set_top(lv_obj_t * obj, bool en);
void lv_obj_set_drag(lv_obj_t * obj, bool en);
void lv_obj_set_drag_dir(lv_obj_t * obj, lv_drag_dir_t drag_dir);
void lv_obj_set_drag_throw(lv_obj_t * obj, bool en);
void lv_obj_set_drag_parent(lv_obj_t * obj, bool en);
void lv_obj_set_parent_event(lv_obj_t * obj, bool en);
void lv_obj_set_base_dir(lv_obj_t * obj, lv_bidi_dir_t dir);
void lv_obj_set_opa_scale_enable(lv_obj_t * obj, bool en);
void lv_obj_set_opa_scale(lv_obj_t * obj, lv_opa_t opa_scale);
void lv_obj_set_protect(lv_obj_t * obj, uint8_t prot);
void lv_obj_clear_protect(lv_obj_t * obj, uint8_t prot);
void lv_obj_set_event_cb(lv_obj_t * obj, lv_event_cb_t event_cb);
lv_res_t lv_event_send(lv_obj_t * obj, lv_event_t event, const void * data);
lv_res_t lv_event_send_func(lv_event_cb_t event_xcb, lv_obj_t * obj, lv_event_t event, const void * data);
const void * lv_event_get_data(void);
void lv_obj_set_signal_cb(lv_obj_t * obj, lv_signal_cb_t signal_cb);
void lv_signal_send(lv_obj_t * obj, lv_signal_t signal, void * param);
void lv_obj_set_design_cb(lv_obj_t * obj, lv_design_cb_t design_cb);
void * lv_obj_allocate_ext_attr(lv_obj_t * obj, uint16_t ext_size);
void lv_obj_refresh_ext_draw_pad(lv_obj_t * obj);
lv_obj_t * lv_obj_get_screen(const lv_obj_t * obj);
lv_disp_t * lv_obj_get_disp(const lv_obj_t * obj);
lv_obj_t * lv_obj_get_parent(const lv_obj_t * obj);
lv_obj_t * lv_obj_get_child(const lv_obj_t * obj, const lv_obj_t * child);
lv_obj_t * lv_obj_get_child_back(const lv_obj_t * obj, const lv_obj_t * child);
uint16_t lv_obj_count_children(const lv_obj_t * obj);
uint16_t lv_obj_count_children_recursive(const lv_obj_t * obj);
void lv_obj_get_coords(const lv_obj_t * obj, lv_area_t * cords_p);
void lv_obj_get_inner_coords(const lv_obj_t * obj, lv_area_t * coords_p);
lv_coord_t lv_obj_get_x(const lv_obj_t * obj);
lv_coord_t lv_obj_get_y(const lv_obj_t * obj);
lv_coord_t lv_obj_get_width(const lv_obj_t * obj);
lv_coord_t lv_obj_get_height(const lv_obj_t * obj);
lv_coord_t lv_obj_get_width_fit(const lv_obj_t * obj);
lv_coord_t lv_obj_get_height_fit(const lv_obj_t * obj);
bool lv_obj_get_auto_realign(const lv_obj_t * obj);
lv_coord_t lv_obj_get_ext_click_pad_left(const lv_obj_t * obj);
lv_coord_t lv_obj_get_ext_click_pad_right(const lv_obj_t * obj);
lv_coord_t lv_obj_get_ext_click_pad_top(const lv_obj_t * obj);
lv_coord_t lv_obj_get_ext_click_pad_bottom(const lv_obj_t * obj);
lv_coord_t lv_obj_get_ext_draw_pad(const lv_obj_t * obj);
const lv_style_t * lv_obj_get_style(const lv_obj_t * obj);
bool lv_obj_get_hidden(const lv_obj_t * obj);
bool lv_obj_get_click(const lv_obj_t * obj);
bool lv_obj_get_top(const lv_obj_t * obj);
bool lv_obj_get_drag(const lv_obj_t * obj);
lv_drag_dir_t lv_obj_get_drag_dir(const lv_obj_t * obj);
bool lv_obj_get_drag_throw(const lv_obj_t * obj);
bool lv_obj_get_drag_parent(const lv_obj_t * obj);
bool lv_obj_get_parent_event(const lv_obj_t * obj);
lv_bidi_dir_t lv_obj_get_base_dir(const lv_obj_t * obj);
lv_opa_t lv_obj_get_opa_scale_enable(const lv_obj_t * obj);
lv_opa_t lv_obj_get_opa_scale(const lv_obj_t * obj);
uint8_t lv_obj_get_protect(const lv_obj_t * obj);
bool lv_obj_is_protected(const lv_obj_t * obj, uint8_t prot);
lv_signal_cb_t lv_obj_get_signal_cb(const lv_obj_t * obj);
lv_design_cb_t lv_obj_get_design_cb(const lv_obj_t * obj);
lv_event_cb_t lv_obj_get_event_cb(const lv_obj_t * obj);
void * lv_obj_get_ext_attr(const lv_obj_t * obj);
void lv_obj_get_type(const lv_obj_t * obj, lv_obj_type_t * buf);
#if LV_USE_USER_DATA
lv_obj_user_data_t lv_obj_get_user_data(const lv_obj_t * obj);
lv_obj_user_data_t * lv_obj_get_user_data_ptr(const lv_obj_t * obj);
void lv_obj_set_user_data(lv_obj_t * obj, lv_obj_user_data_t data);
#endif
#if LV_USE_GROUP
void * lv_obj_get_group(const lv_obj_t * obj);
bool lv_obj_is_focused(const lv_obj_t * obj);
#endif
lv_res_t lv_obj_handle_get_type_signal(lv_obj_type_t * buf, const char * name);
#define LV_EVENT_CB_DECLARE(name) void name(lv_obj_t * obj, lv_event_t e)
#if defined(__cplusplus)
} 
#endif
