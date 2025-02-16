




#if !defined(LV_IMG_DEOCER_H)
#define LV_IMG_DEOCER_H

#if defined(__cplusplus)
extern "C" {
#endif




#if defined(LV_CONF_INCLUDE_SIMPLE)
#include "lv_conf.h"
#else
#include "../../../lv_conf.h"
#endif

#include <stdint.h>
#include "../lv_misc/lv_fs.h"
#include "../lv_misc/lv_types.h"
#include "../lv_misc/lv_area.h"
#include "../lv_core/lv_style.h"





#if LV_COLOR_DEPTH == 1 || LV_COLOR_DEPTH == 8
#define LV_IMG_PX_SIZE_ALPHA_BYTE 2
#elif LV_COLOR_DEPTH == 16
#define LV_IMG_PX_SIZE_ALPHA_BYTE 3
#elif LV_COLOR_DEPTH == 32
#define LV_IMG_PX_SIZE_ALPHA_BYTE 4
#endif







enum {
LV_IMG_SRC_VARIABLE, 
LV_IMG_SRC_FILE, 
LV_IMG_SRC_SYMBOL, 
LV_IMG_SRC_UNKNOWN, 
};

typedef uint8_t lv_img_src_t;



typedef struct
{



uint32_t cf : 5; 
uint32_t always_zero : 3; 


uint32_t reserved : 2; 

uint32_t w : 11; 
uint32_t h : 11; 
} lv_img_header_t;


enum {
LV_IMG_CF_UNKNOWN = 0,

LV_IMG_CF_RAW, 
LV_IMG_CF_RAW_ALPHA, 

LV_IMG_CF_RAW_CHROMA_KEYED, 


LV_IMG_CF_TRUE_COLOR, 
LV_IMG_CF_TRUE_COLOR_ALPHA, 
LV_IMG_CF_TRUE_COLOR_CHROMA_KEYED, 


LV_IMG_CF_INDEXED_1BIT, 
LV_IMG_CF_INDEXED_2BIT, 
LV_IMG_CF_INDEXED_4BIT, 
LV_IMG_CF_INDEXED_8BIT, 

LV_IMG_CF_ALPHA_1BIT, 
LV_IMG_CF_ALPHA_2BIT, 
LV_IMG_CF_ALPHA_4BIT, 
LV_IMG_CF_ALPHA_8BIT, 

LV_IMG_CF_RESERVED_15, 
LV_IMG_CF_RESERVED_16, 
LV_IMG_CF_RESERVED_17, 
LV_IMG_CF_RESERVED_18, 
LV_IMG_CF_RESERVED_19, 
LV_IMG_CF_RESERVED_20, 
LV_IMG_CF_RESERVED_21, 
LV_IMG_CF_RESERVED_22, 
LV_IMG_CF_RESERVED_23, 

LV_IMG_CF_USER_ENCODED_0, 
LV_IMG_CF_USER_ENCODED_1, 
LV_IMG_CF_USER_ENCODED_2, 
LV_IMG_CF_USER_ENCODED_3, 
LV_IMG_CF_USER_ENCODED_4, 
LV_IMG_CF_USER_ENCODED_5, 
LV_IMG_CF_USER_ENCODED_6, 
LV_IMG_CF_USER_ENCODED_7, 
};
typedef uint8_t lv_img_cf_t;



typedef struct
{
lv_img_header_t header;
uint32_t data_size;
const uint8_t * data;
} lv_img_dsc_t;



struct _lv_img_decoder;
struct _lv_img_decoder_dsc;








typedef lv_res_t (*lv_img_decoder_info_f_t)(struct _lv_img_decoder * decoder, const void * src,
lv_img_header_t * header);






typedef lv_res_t (*lv_img_decoder_open_f_t)(struct _lv_img_decoder * decoder, struct _lv_img_decoder_dsc * dsc);












typedef lv_res_t (*lv_img_decoder_read_line_f_t)(struct _lv_img_decoder * decoder, struct _lv_img_decoder_dsc * dsc,
lv_coord_t x, lv_coord_t y, lv_coord_t len, uint8_t * buf);






typedef void (*lv_img_decoder_close_f_t)(struct _lv_img_decoder * decoder, struct _lv_img_decoder_dsc * dsc);

typedef struct _lv_img_decoder
{
lv_img_decoder_info_f_t info_cb;
lv_img_decoder_open_f_t open_cb;
lv_img_decoder_read_line_f_t read_line_cb;
lv_img_decoder_close_f_t close_cb;

#if LV_USE_USER_DATA
lv_img_decoder_user_data_t user_data;
#endif
} lv_img_decoder_t;


typedef struct _lv_img_decoder_dsc
{

lv_img_decoder_t * decoder;


const void * src;


const lv_style_t * style;


lv_img_src_t src_type;


lv_img_header_t header;



const uint8_t * img_data;



uint32_t time_to_open;



const char * error_msg;


void * user_data;
} lv_img_decoder_dsc_t;








void lv_img_decoder_init(void);











lv_res_t lv_img_decoder_get_info(const char * src, lv_img_header_t * header);













lv_res_t lv_img_decoder_open(lv_img_decoder_dsc_t * dsc, const void * src, const lv_style_t * style);










lv_res_t lv_img_decoder_read_line(lv_img_decoder_dsc_t * dsc, lv_coord_t x, lv_coord_t y, lv_coord_t len,
uint8_t * buf);





void lv_img_decoder_close(lv_img_decoder_dsc_t * dsc);





lv_img_decoder_t * lv_img_decoder_create(void);





void lv_img_decoder_delete(lv_img_decoder_t * decoder);






void lv_img_decoder_set_info_cb(lv_img_decoder_t * decoder, lv_img_decoder_info_f_t info_cb);






void lv_img_decoder_set_open_cb(lv_img_decoder_t * decoder, lv_img_decoder_open_f_t open_cb);






void lv_img_decoder_set_read_line_cb(lv_img_decoder_t * decoder, lv_img_decoder_read_line_f_t read_line_cb);






void lv_img_decoder_set_close_cb(lv_img_decoder_t * decoder, lv_img_decoder_close_f_t close_cb);










lv_res_t lv_img_decoder_built_in_info(lv_img_decoder_t * decoder, const void * src, lv_img_header_t * header);







lv_res_t lv_img_decoder_built_in_open(lv_img_decoder_t * decoder, lv_img_decoder_dsc_t * dsc);












lv_res_t lv_img_decoder_built_in_read_line(lv_img_decoder_t * decoder, lv_img_decoder_dsc_t * dsc, lv_coord_t x,
lv_coord_t y, lv_coord_t len, uint8_t * buf);






void lv_img_decoder_built_in_close(lv_img_decoder_t * decoder, lv_img_decoder_dsc_t * dsc);





#if defined(__cplusplus)
} 
#endif

#endif 
