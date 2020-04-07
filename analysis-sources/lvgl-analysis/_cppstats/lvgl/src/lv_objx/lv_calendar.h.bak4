




#if !defined(LV_CALENDAR_H)
#define LV_CALENDAR_H

#if defined(__cplusplus)
extern "C" {
#endif




#if defined(LV_CONF_INCLUDE_SIMPLE)
#include "lv_conf.h"
#else
#include "../../../lv_conf.h"
#endif

#if LV_USE_CALENDAR != 0

#include "../lv_core/lv_obj.h"












typedef struct
{
uint16_t year;
int8_t month;
int8_t day;
} lv_calendar_date_t;


typedef struct
{


lv_calendar_date_t today; 
lv_calendar_date_t showed_date; 
lv_calendar_date_t * highlighted_dates; 

int8_t btn_pressing; 
uint16_t highlighted_dates_num; 
lv_calendar_date_t pressed_date;
const char ** day_names; 
const char ** month_names; 


const lv_style_t * style_header;
const lv_style_t * style_header_pr;
const lv_style_t * style_day_names;
const lv_style_t * style_highlighted_days;
const lv_style_t * style_inactive_days;
const lv_style_t * style_week_box;
const lv_style_t * style_today_box;
} lv_calendar_ext_t;


enum {
LV_CALENDAR_STYLE_BG, 
LV_CALENDAR_STYLE_HEADER, 
LV_CALENDAR_STYLE_HEADER_PR, 
LV_CALENDAR_STYLE_DAY_NAMES, 
LV_CALENDAR_STYLE_HIGHLIGHTED_DAYS, 
LV_CALENDAR_STYLE_INACTIVE_DAYS, 
LV_CALENDAR_STYLE_WEEK_BOX, 
LV_CALENDAR_STYLE_TODAY_BOX, 
};
typedef uint8_t lv_calendar_style_t;











lv_obj_t * lv_calendar_create(lv_obj_t * par, const lv_obj_t * copy);















void lv_calendar_set_today_date(lv_obj_t * calendar, lv_calendar_date_t * today);







void lv_calendar_set_showed_date(lv_obj_t * calendar, lv_calendar_date_t * showed);








void lv_calendar_set_highlighted_dates(lv_obj_t * calendar, lv_calendar_date_t highlighted[], uint16_t date_num);








void lv_calendar_set_day_names(lv_obj_t * calendar, const char ** day_names);








void lv_calendar_set_month_names(lv_obj_t * calendar, const char ** month_names);







void lv_calendar_set_style(lv_obj_t * calendar, lv_calendar_style_t type, const lv_style_t * style);










lv_calendar_date_t * lv_calendar_get_today_date(const lv_obj_t * calendar);






lv_calendar_date_t * lv_calendar_get_showed_date(const lv_obj_t * calendar);







lv_calendar_date_t * lv_calendar_get_pressed_date(const lv_obj_t * calendar);






lv_calendar_date_t * lv_calendar_get_highlighted_dates(const lv_obj_t * calendar);






uint16_t lv_calendar_get_highlighted_dates_num(const lv_obj_t * calendar);






const char ** lv_calendar_get_day_names(const lv_obj_t * calendar);






const char ** lv_calendar_get_month_names(const lv_obj_t * calendar);







const lv_style_t * lv_calendar_get_style(const lv_obj_t * calendar, lv_calendar_style_t type);









#endif 

#if defined(__cplusplus)
} 
#endif

#endif 
