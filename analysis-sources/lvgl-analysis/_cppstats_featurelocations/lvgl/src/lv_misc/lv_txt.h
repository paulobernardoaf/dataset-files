




#if !defined(LV_TXT_H)
#define LV_TXT_H

#if defined(__cplusplus)
extern "C" {
#endif




#if defined(LV_CONF_INCLUDE_SIMPLE)
#include "lv_conf.h"
#else
#include "../../../lv_conf.h"
#endif

#include <stdbool.h>
#include "lv_area.h"
#include "lv_area.h"
#include "../lv_font/lv_font.h"




#if !defined(LV_TXT_COLOR_CMD)
#define LV_TXT_COLOR_CMD "#"
#endif

#define LV_TXT_ENC_UTF8 1
#define LV_TXT_ENC_ASCII 2







enum {
LV_TXT_FLAG_NONE = 0x00,
LV_TXT_FLAG_RECOLOR = 0x01, 
LV_TXT_FLAG_EXPAND = 0x02, 
LV_TXT_FLAG_CENTER = 0x04, 
LV_TXT_FLAG_RIGHT = 0x08, 
};
typedef uint8_t lv_txt_flag_t;



enum {
LV_TXT_CMD_STATE_WAIT, 
LV_TXT_CMD_STATE_PAR, 
LV_TXT_CMD_STATE_IN, 
};
typedef uint8_t lv_txt_cmd_state_t;
















void lv_txt_get_size(lv_point_t * size_res, const char * text, const lv_font_t * font, lv_coord_t letter_space,
lv_coord_t line_space, lv_coord_t max_width, lv_txt_flag_t flag);












uint16_t lv_txt_get_next_line(const char * txt, const lv_font_t * font, lv_coord_t letter_space, lv_coord_t max_width,
lv_txt_flag_t flag);











lv_coord_t lv_txt_get_width(const char * txt, uint16_t length, const lv_font_t * font, lv_coord_t letter_space,
lv_txt_flag_t flag);









bool lv_txt_is_cmd(lv_txt_cmd_state_t * state, uint32_t c);







void lv_txt_ins(char * txt_buf, uint32_t pos, const char * ins_txt);








void lv_txt_cut(char * txt, uint32_t pos, uint32_t len);










extern uint8_t (*lv_txt_encoded_size)(const char *);






extern uint32_t (*lv_txt_unicode_to_encoded)(uint32_t);






extern uint32_t (*lv_txt_encoded_conv_wc)(uint32_t c);









extern uint32_t (*lv_txt_encoded_next)(const char *, uint32_t *);








extern uint32_t (*lv_txt_encoded_prev)(const char *, uint32_t *);








extern uint32_t (*lv_txt_encoded_get_byte_id)(const char *, uint32_t);








extern uint32_t (*lv_txt_encoded_get_char_id)(const char *, uint32_t);







extern uint32_t (*lv_txt_get_encoded_length)(const char *);





#if defined(__cplusplus)
} 
#endif

#endif 
