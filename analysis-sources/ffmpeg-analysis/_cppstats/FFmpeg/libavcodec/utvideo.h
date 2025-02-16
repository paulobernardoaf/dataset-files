#include "libavutil/common.h"
#include "avcodec.h"
#include "bswapdsp.h"
#include "utvideodsp.h"
#include "lossless_videodsp.h"
#include "lossless_videoencdsp.h"
enum {
PRED_NONE = 0,
PRED_LEFT,
PRED_GRADIENT,
PRED_MEDIAN,
};
enum {
COMP_NONE = 0,
COMP_HUFF,
};
enum {
UTVIDEO_RGB = MKTAG(0x00, 0x00, 0x01, 0x18),
UTVIDEO_RGBA = MKTAG(0x00, 0x00, 0x02, 0x18),
UTVIDEO_420 = MKTAG('Y', 'V', '1', '2'),
UTVIDEO_422 = MKTAG('Y', 'U', 'Y', '2'),
UTVIDEO_444 = MKTAG('Y', 'V', '2', '4'),
};
extern const int ff_ut_pred_order[5];
typedef struct UtvideoContext {
const AVClass *class;
AVCodecContext *avctx;
UTVideoDSPContext utdsp;
BswapDSPContext bdsp;
LLVidDSPContext llviddsp;
LLVidEncDSPContext llvidencdsp;
uint32_t frame_info_size, flags, frame_info, offset;
int planes;
int slices;
int compression;
int interlaced;
int frame_pred;
int pro;
int pack;
ptrdiff_t slice_stride;
uint8_t *slice_bits, *slice_buffer[4];
int slice_bits_size;
const uint8_t *packed_stream[4][256];
size_t packed_stream_size[4][256];
const uint8_t *control_stream[4][256];
size_t control_stream_size[4][256];
} UtvideoContext;
typedef struct HuffEntry {
uint16_t sym;
uint8_t len;
uint32_t code;
} HuffEntry;
int ff_ut_huff_cmp_len(const void *a, const void *b);
int ff_ut10_huff_cmp_len(const void *a, const void *b);
