




#if !defined(LV_BIDI_H)
#define LV_BIDI_H

#if defined(__cplusplus)
extern "C" {
#endif




#if defined(LV_CONF_INCLUDE_SIMPLE)
#include "lv_conf.h"
#else
#include "../../../lv_conf.h"
#endif

#include <stdbool.h>
#include <stdint.h>






#define LV_BIDI_LRO "\xE2\x80\xAD" 
#define LV_BIDI_RLO "\xE2\x80\xAE" 




enum
{

LV_BIDI_DIR_LTR = 0x00,
LV_BIDI_DIR_RTL = 0x01,
LV_BIDI_DIR_AUTO = 0x02,
LV_BIDI_DIR_INHERIT = 0x03,

LV_BIDI_DIR_NEUTRAL = 0x20,
LV_BIDI_DIR_WEAK = 0x21,
};

typedef uint8_t lv_bidi_dir_t;




#if LV_USE_BIDI

void lv_bidi_process(const char * str_in, char * str_out, lv_bidi_dir_t base_dir);
void lv_bidi_process_paragraph(const char * str_in, char * str_out, uint32_t len, lv_bidi_dir_t base_dir, uint16_t *pos_conv_out, uint16_t pos_conv_len);
uint32_t lv_bidi_get_next_paragraph(const char * txt);
lv_bidi_dir_t lv_bidi_detect_base_dir(const char * txt);
lv_bidi_dir_t lv_bidi_get_letter_dir(uint32_t letter);
bool lv_bidi_letter_is_weak(uint32_t letter);
bool lv_bidi_letter_is_rtl(uint32_t letter);
bool lv_bidi_letter_is_neutral(uint32_t letter);
uint16_t lv_bidi_get_logical_pos(const char * str_in, char **bidi_txt, uint32_t len, lv_bidi_dir_t base_dir, uint32_t visual_pos, bool *is_rtl);
uint16_t lv_bidi_get_visual_pos(const char * str_in, char **bidi_txt, uint16_t len, lv_bidi_dir_t base_dir, uint32_t logical_pos, bool *is_rtl);





#endif 

#if defined(__cplusplus)
} 
#endif

#endif 
