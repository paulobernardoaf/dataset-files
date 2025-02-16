#if defined(__cplusplus)
extern "C" {
#endif
#if defined(LV_CONF_INCLUDE_SIMPLE)
#include "lv_conf.h"
#else
#include "../../../lv_conf.h"
#endif
#if LV_USE_GAUGE != 0
#if LV_USE_LMETER == 0
#error "lv_gauge: lv_lmeter is required. Enable it in lv_conf.h (LV_USE_LMETER 1) "
#endif
#include "../lv_core/lv_obj.h"
#include "lv_lmeter.h"
#include "lv_label.h"
#include "lv_line.h"
typedef struct
{
lv_lmeter_ext_t lmeter; 
int16_t * values; 
const lv_color_t * needle_colors; 
uint8_t needle_count; 
uint8_t label_count; 
} lv_gauge_ext_t;
enum {
LV_GAUGE_STYLE_MAIN,
};
typedef uint8_t lv_gauge_style_t;
lv_obj_t * lv_gauge_create(lv_obj_t * par, const lv_obj_t * copy);
void lv_gauge_set_needle_count(lv_obj_t * gauge, uint8_t needle_cnt, const lv_color_t colors[]);
void lv_gauge_set_value(lv_obj_t * gauge, uint8_t needle_id, int16_t value);
static inline void lv_gauge_set_range(lv_obj_t * gauge, int16_t min, int16_t max)
{
lv_lmeter_set_range(gauge, min, max);
}
static inline void lv_gauge_set_critical_value(lv_obj_t * gauge, int16_t value)
{
lv_lmeter_set_value(gauge, value);
}
void lv_gauge_set_scale(lv_obj_t * gauge, uint16_t angle, uint8_t line_cnt, uint8_t label_cnt);
static inline void lv_gauge_set_style(lv_obj_t * gauge, lv_gauge_style_t type, lv_style_t * style)
{
(void)type; 
lv_obj_set_style(gauge, style);
}
int16_t lv_gauge_get_value(const lv_obj_t * gauge, uint8_t needle);
uint8_t lv_gauge_get_needle_count(const lv_obj_t * gauge);
static inline int16_t lv_gauge_get_min_value(const lv_obj_t * lmeter)
{
return lv_lmeter_get_min_value(lmeter);
}
static inline int16_t lv_gauge_get_max_value(const lv_obj_t * lmeter)
{
return lv_lmeter_get_max_value(lmeter);
}
static inline int16_t lv_gauge_get_critical_value(const lv_obj_t * gauge)
{
return lv_lmeter_get_value(gauge);
}
uint8_t lv_gauge_get_label_count(const lv_obj_t * gauge);
static inline uint16_t lv_gauge_get_line_count(const lv_obj_t * gauge)
{
return lv_lmeter_get_line_count(gauge);
}
static inline uint16_t lv_gauge_get_scale_angle(const lv_obj_t * gauge)
{
return lv_lmeter_get_scale_angle(gauge);
}
static inline const lv_style_t * lv_gauge_get_style(const lv_obj_t * gauge, lv_gauge_style_t type)
{
(void)type; 
return lv_obj_get_style(gauge);
}
#endif 
#if defined(__cplusplus)
} 
#endif
