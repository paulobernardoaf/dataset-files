#include <stdint.h>
#include <stdatomic.h>
#include "avcodec.h"
#include "me_cmp.h"
#include "thread.h"
#define VP_START 1
#define ER_AC_ERROR 2
#define ER_DC_ERROR 4
#define ER_MV_ERROR 8
#define ER_AC_END 16
#define ER_DC_END 32
#define ER_MV_END 64
#define ER_MB_ERROR (ER_AC_ERROR|ER_DC_ERROR|ER_MV_ERROR)
#define ER_MB_END (ER_AC_END|ER_DC_END|ER_MV_END)
typedef struct ERPicture {
AVFrame *f;
ThreadFrame *tf;
int16_t (*motion_val[2])[2];
int8_t *ref_index[2];
uint32_t *mb_type;
int field_picture;
} ERPicture;
typedef struct ERContext {
AVCodecContext *avctx;
MECmpContext mecc;
int mecc_inited;
int *mb_index2xy;
int mb_num;
int mb_width, mb_height;
ptrdiff_t mb_stride;
ptrdiff_t b8_stride;
atomic_int error_count;
int error_occurred;
uint8_t *error_status_table;
uint8_t *er_temp_buffer;
int16_t *dc_val[3];
uint8_t *mbskip_table;
uint8_t *mbintra_table;
int mv[2][4][2];
ERPicture cur_pic;
ERPicture last_pic;
ERPicture next_pic;
AVBufferRef *ref_index_buf[2];
AVBufferRef *motion_val_buf[2];
uint16_t pp_time;
uint16_t pb_time;
int quarter_sample;
int partitioned_frame;
int ref_count;
void (*decode_mb)(void *opaque, int ref, int mv_dir, int mv_type,
int (*mv)[2][4][2],
int mb_x, int mb_y, int mb_intra, int mb_skipped);
void *opaque;
} ERContext;
void ff_er_frame_start(ERContext *s);
void ff_er_frame_end(ERContext *s);
void ff_er_add_slice(ERContext *s, int startx, int starty, int endx, int endy,
int status);
