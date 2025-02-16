#include <stdint.h>
#include "libavutil/frame.h"
#include "avcodec.h"
#include "motion_est.h"
#include "thread.h"
#define MAX_PICTURE_COUNT 36
#define EDGE_WIDTH 16
typedef struct ScratchpadContext {
uint8_t *edge_emu_buffer; 
uint8_t *rd_scratchpad; 
uint8_t *obmc_scratchpad;
uint8_t *b_scratchpad; 
} ScratchpadContext;
typedef struct Picture {
struct AVFrame *f;
ThreadFrame tf;
AVBufferRef *qscale_table_buf;
int8_t *qscale_table;
AVBufferRef *motion_val_buf[2];
int16_t (*motion_val[2])[2];
AVBufferRef *mb_type_buf;
uint32_t *mb_type; 
AVBufferRef *mbskip_table_buf;
uint8_t *mbskip_table;
AVBufferRef *ref_index_buf[2];
int8_t *ref_index[2];
AVBufferRef *mb_var_buf;
uint16_t *mb_var; 
AVBufferRef *mc_mb_var_buf;
uint16_t *mc_mb_var; 
int alloc_mb_width; 
int alloc_mb_height; 
AVBufferRef *mb_mean_buf;
uint8_t *mb_mean; 
AVBufferRef *hwaccel_priv_buf;
void *hwaccel_picture_private; 
int field_picture; 
int64_t mb_var_sum; 
int64_t mc_mb_var_sum; 
int b_frame_score;
int needs_realloc; 
int reference;
int shared;
uint64_t encoding_error[AV_NUM_DATA_POINTERS];
} Picture;
int ff_alloc_picture(AVCodecContext *avctx, Picture *pic, MotionEstContext *me,
ScratchpadContext *sc, int shared, int encoding,
int chroma_x_shift, int chroma_y_shift, int out_format,
int mb_stride, int mb_width, int mb_height, int b8_stride,
ptrdiff_t *linesize, ptrdiff_t *uvlinesize);
int ff_mpeg_framesize_alloc(AVCodecContext *avctx, MotionEstContext *me,
ScratchpadContext *sc, int linesize);
int ff_mpeg_ref_picture(AVCodecContext *avctx, Picture *dst, Picture *src);
void ff_mpeg_unref_picture(AVCodecContext *avctx, Picture *picture);
void ff_free_picture_tables(Picture *pic);
int ff_update_picture_tables(Picture *dst, Picture *src);
int ff_find_unused_picture(AVCodecContext *avctx, Picture *picture, int shared);
