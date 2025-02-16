#include "lv_theme.h"
#include "../lv_core/lv_obj.h"
#if LV_THEME_LIVE_UPDATE == 0
static lv_theme_t * current_theme;
#else
static lv_style_t th_styles[LV_THEME_STYLE_COUNT];
static bool inited = false;
static lv_theme_t current_theme;
#endif
void lv_theme_set_current(lv_theme_t * th)
{
#if LV_THEME_LIVE_UPDATE == 0
current_theme = th;
#if LV_USE_GROUP
memcpy(&current_theme->group, &th->group, sizeof(th->group));
#endif
lv_obj_report_style_mod(NULL);
#else
uint32_t style_num = sizeof(th->style) / sizeof(lv_style_t *); 
if(!inited) {
uint16_t i;
lv_style_t ** cur_th_style_p = (lv_style_t **)&current_theme.style;
for(i = 0; i < style_num; i++) {
uintptr_t adr = (uintptr_t)&th_styles[i];
memcpy(&cur_th_style_p[i], &adr, sizeof(lv_style_t *));
}
inited = true;
}
uint16_t i;
lv_style_t ** th_style = (lv_style_t **)&th->style;
for(i = 0; i < style_num; i++) {
uintptr_t s = (uintptr_t)th_style[i];
if(s) memcpy(&th_styles[i], (void *)s, sizeof(lv_style_t));
}
#if LV_USE_GROUP
memcpy(&current_theme.group, &th->group, sizeof(th->group));
#endif
lv_obj_report_style_mod(NULL);
#endif
#if LV_USE_GROUP
lv_group_report_style_mod(NULL);
#endif
}
lv_theme_t * lv_theme_get_current(void)
{
#if LV_THEME_LIVE_UPDATE == 0
return current_theme;
#else
if(!inited)
return NULL;
else
return &current_theme;
#endif
}
