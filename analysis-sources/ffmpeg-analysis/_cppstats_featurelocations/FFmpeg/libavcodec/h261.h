


























#if !defined(AVCODEC_H261_H)
#define AVCODEC_H261_H

#include "mpegvideo.h"
#include "rl.h"




typedef struct H261Context {
MpegEncContext s;

int current_mba;
int mba_diff;
int mtype;
int current_mv_x;
int current_mv_y;
int gob_number;
int gob_start_code_skipped; 
} H261Context;

#define MB_TYPE_H261_FIL 0x800000

extern uint8_t ff_h261_rl_table_store[2][2 * MAX_RUN + MAX_LEVEL + 3];

extern const uint8_t ff_h261_mba_code[35];
extern const uint8_t ff_h261_mba_bits[35];
extern const uint8_t ff_h261_mtype_code[10];
extern const uint8_t ff_h261_mtype_bits[10];
extern const int ff_h261_mtype_map[10];
extern const uint8_t ff_h261_mv_tab[17][2];
extern const uint8_t ff_h261_cbp_tab[63][2];
extern RLTable ff_h261_rl_tcoeff;

void ff_h261_loop_filter(MpegEncContext *s);
void ff_h261_common_init(void);

int ff_h261_get_picture_format(int width, int height);
void ff_h261_reorder_mb_index(MpegEncContext *s);
void ff_h261_encode_mb(MpegEncContext *s, int16_t block[6][64],
int motion_x, int motion_y);
void ff_h261_encode_picture_header(MpegEncContext *s, int picture_number);
void ff_h261_encode_init(MpegEncContext *s);

#endif 
