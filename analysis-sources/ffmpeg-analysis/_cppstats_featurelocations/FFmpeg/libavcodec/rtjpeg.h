




















#if !defined(AVCODEC_RTJPEG_H)
#define AVCODEC_RTJPEG_H

#include <stdint.h>

#include "libavutil/mem.h"
#include "idctdsp.h"

#define RTJPEG_FILE_VERSION 0
#define RTJPEG_HEADER_SIZE 12

typedef struct RTJpegContext {
int w, h;
IDCTDSPContext idsp;
uint8_t scan[64];
uint32_t lquant[64];
uint32_t cquant[64];
DECLARE_ALIGNED(16, int16_t, block)[64];
} RTJpegContext;

void ff_rtjpeg_init(RTJpegContext *c, AVCodecContext *avctx);

void ff_rtjpeg_decode_init(RTJpegContext *c, int width, int height,
const uint32_t *lquant, const uint32_t *cquant);

int ff_rtjpeg_decode_frame_yuv420(RTJpegContext *c, AVFrame *f,
const uint8_t *buf, int buf_size);
#endif 
