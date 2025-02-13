



























#if !defined(AVCODEC_HUFFYUV_H)
#define AVCODEC_HUFFYUV_H

#include <stdint.h>

#include "avcodec.h"
#include "bswapdsp.h"
#include "get_bits.h"
#include "huffyuvdsp.h"
#include "huffyuvencdsp.h"
#include "put_bits.h"
#include "lossless_videodsp.h"
#include "lossless_videoencdsp.h"

#define VLC_BITS 12

#define MAX_BITS 16
#define MAX_N (1<<MAX_BITS)
#define MAX_VLC_N 16384

typedef enum Predictor {
LEFT = 0,
PLANE,
MEDIAN,
} Predictor;

typedef struct HYuvContext {
AVClass *class;
AVCodecContext *avctx;
Predictor predictor;
GetBitContext gb;
PutBitContext pb;
int interlaced;
int decorrelate;
int bitstream_bpp;
int version;
int yuy2; 
int bgr32; 
int bps;
int n; 
int vlc_n; 
int alpha;
int chroma;
int yuv;
int chroma_h_shift;
int chroma_v_shift;
int width, height;
int flags;
int context;
int picture_number;
int last_slice_end;
uint8_t *temp[3];
uint16_t *temp16[3]; 
uint64_t stats[4][MAX_VLC_N];
uint8_t len[4][MAX_VLC_N];
uint32_t bits[4][MAX_VLC_N];
uint32_t pix_bgr_map[1<<VLC_BITS];
VLC vlc[8]; 
uint8_t *bitstream_buffer;
unsigned int bitstream_buffer_size;
BswapDSPContext bdsp;
HuffYUVDSPContext hdsp;
HuffYUVEncDSPContext hencdsp;
LLVidDSPContext llviddsp;
LLVidEncDSPContext llvidencdsp;
int non_determ; 
} HYuvContext;

void ff_huffyuv_common_init(AVCodecContext *s);
void ff_huffyuv_common_end(HYuvContext *s);
int ff_huffyuv_alloc_temp(HYuvContext *s);
int ff_huffyuv_generate_bits_table(uint32_t *dst, const uint8_t *len_table, int n);

#endif 
