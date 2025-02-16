#if defined(__cplusplus)
extern "C" {
#endif
#if defined(LV_CONF_INCLUDE_SIMPLE)
#include "lv_conf.h"
#else
#include "../../../lv_conf.h"
#endif
#if LV_USE_CPICKER != 0
#include "../lv_core/lv_obj.h"
enum {
LV_CPICKER_TYPE_RECT,
LV_CPICKER_TYPE_DISC,
};
typedef uint8_t lv_cpicker_type_t;
enum {
LV_CPICKER_COLOR_MODE_HUE,
LV_CPICKER_COLOR_MODE_SATURATION,
LV_CPICKER_COLOR_MODE_VALUE
};
typedef uint8_t lv_cpicker_color_mode_t;
typedef struct {
lv_color_hsv_t hsv;
struct {
lv_style_t * style;
lv_point_t pos;
uint8_t colored :1;
} indic;
uint32_t last_click_time;
uint32_t last_change_time;
lv_point_t last_press_point;
lv_cpicker_color_mode_t color_mode :2;
uint8_t color_mode_fixed :1;
lv_cpicker_type_t type :1;
uint8_t preview :1;
} lv_cpicker_ext_t;
enum {
LV_CPICKER_STYLE_MAIN,
LV_CPICKER_STYLE_INDICATOR,
};
typedef uint8_t lv_cpicker_style_t;
lv_obj_t * lv_cpicker_create(lv_obj_t * par, const lv_obj_t * copy);
void lv_cpicker_set_type(lv_obj_t * cpicker, lv_cpicker_type_t type);
void lv_cpicker_set_style(lv_obj_t * cpicker, lv_cpicker_style_t type, lv_style_t *style);
bool lv_cpicker_set_hue(lv_obj_t * cpicker, uint16_t hue);
bool lv_cpicker_set_saturation(lv_obj_t * cpicker, uint8_t saturation);
bool lv_cpicker_set_value(lv_obj_t * cpicker, uint8_t val);
bool lv_cpicker_set_hsv(lv_obj_t * cpicker, lv_color_hsv_t hsv);
bool lv_cpicker_set_color(lv_obj_t * cpicker, lv_color_t color);
void lv_cpicker_set_color_mode(lv_obj_t * cpicker, lv_cpicker_color_mode_t mode);
void lv_cpicker_set_color_mode_fixed(lv_obj_t * cpicker, bool fixed);
void lv_cpicker_set_indic_colored(lv_obj_t * cpicker, bool en);
void lv_cpicker_set_preview(lv_obj_t * cpicker, bool en);
lv_cpicker_color_mode_t lv_cpicker_get_color_mode(lv_obj_t * cpicker);
bool lv_cpicker_get_color_mode_fixed(lv_obj_t * cpicker);
const lv_style_t * lv_cpicker_get_style(const lv_obj_t * cpicker, lv_cpicker_style_t type);
uint16_t lv_cpicker_get_hue(lv_obj_t * cpicker);
uint8_t lv_cpicker_get_saturation(lv_obj_t * cpicker);
uint8_t lv_cpicker_get_value(lv_obj_t * cpicker);
lv_color_hsv_t lv_cpicker_get_hsv(lv_obj_t * cpicker);
lv_color_t lv_cpicker_get_color(lv_obj_t * cpicker);
bool lv_cpicker_get_indic_colored(lv_obj_t * cpicker);
bool lv_cpicker_get_preview(lv_obj_t * cpicker);
#endif 
#if defined(__cplusplus)
} 
#endif
