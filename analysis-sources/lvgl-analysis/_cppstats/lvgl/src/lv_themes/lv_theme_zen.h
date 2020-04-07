#if defined(__cplusplus)
extern "C" {
#endif
#if defined(LV_CONF_INCLUDE_SIMPLE)
#include "lv_conf.h"
#else
#include "../../../lv_conf.h"
#endif
#if LV_USE_THEME_ZEN
lv_theme_t * lv_theme_zen_init(uint16_t hue, lv_font_t * font);
lv_theme_t * lv_theme_get_zen(void);
#endif
#if defined(__cplusplus)
} 
#endif
