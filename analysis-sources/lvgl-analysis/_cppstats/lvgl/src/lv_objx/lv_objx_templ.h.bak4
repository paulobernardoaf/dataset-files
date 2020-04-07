











#if !defined(LV_TEMPL_H)
#define LV_TEMPL_H

#if defined(__cplusplus)
extern "C" {
#endif




#if defined(LV_CONF_INCLUDE_SIMPLE)
#include "lv_conf.h"
#else
#include "../../../lv_conf.h"
#endif

#if LV_USE_TEMPL != 0

#include "../lv_core/lv_obj.h"









typedef struct
{
lv_ANCESTOR_ext_t ANCESTOR; 

} lv_templ_ext_t;


enum {
LV_TEMPL_STYLE_X,
LV_TEMPL_STYLE_Y,
};
typedef uint8_t lv_templ_style_t;











lv_obj_t * lv_templ_create(lv_obj_t * par, const lv_obj_t * copy);















void lv_templ_set_style(lv_obj_t * templ, lv_templ_style_t type, const lv_style_t * style);











lv_style_t * lv_templ_get_style(const lv_obj_t * templ, lv_templ_style_t type);









#endif 

#if defined(__cplusplus)
} 
#endif

#endif 
