




#if !defined(LV_IMG_CACHE_H)
#define LV_IMG_CACHE_H

#if defined(__cplusplus)
extern "C" {
#endif




#include "lv_img_decoder.h"














typedef struct
{
lv_img_decoder_dsc_t dec_dsc; 




int32_t life;
} lv_img_cache_entry_t;













lv_img_cache_entry_t * lv_img_cache_open(const void * src, const lv_style_t * style);







void lv_img_cache_set_size(uint16_t new_slot_num);






void lv_img_cache_invalidate_src(const void * src);





#if defined(__cplusplus)
} 
#endif

#endif 
