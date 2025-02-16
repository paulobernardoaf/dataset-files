



















#if !defined(AVCODEC_SVQ1ENC_H)
#define AVCODEC_SVQ1ENC_H

#include <stdint.h>

#include "libavutil/frame.h"

#include "avcodec.h"
#include "hpeldsp.h"
#include "me_cmp.h"
#include "mpegvideo.h"
#include "put_bits.h"

typedef struct SVQ1EncContext {



MpegEncContext m;
AVCodecContext *avctx;
MECmpContext mecc;
HpelDSPContext hdsp;
AVFrame *current_picture;
AVFrame *last_picture;
PutBitContext pb;


enum AVPictureType pict_type;
int quality;



PutBitContext reorder_pb[6];

int frame_width;
int frame_height;


int y_block_width;
int y_block_height;


int c_block_width;
int c_block_height;

DECLARE_ALIGNED(16, int16_t, encoded_block_levels)[6][7][256];

uint16_t *mb_type;
uint32_t *dummy;
int16_t (*motion_val8[3])[2];
int16_t (*motion_val16[3])[2];

int64_t rd_total;

uint8_t *scratchbuf;

int motion_est;

int (*ssd_int8_vs_int16)(const int8_t *pix1, const int16_t *pix2,
intptr_t size);
} SVQ1EncContext;

void ff_svq1enc_init_ppc(SVQ1EncContext *c);
void ff_svq1enc_init_x86(SVQ1EncContext *c);

#endif 
