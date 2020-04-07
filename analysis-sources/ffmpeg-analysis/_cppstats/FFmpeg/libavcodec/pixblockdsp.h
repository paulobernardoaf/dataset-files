#include <stdint.h>
#include "config.h"
#include "avcodec.h"
typedef struct PixblockDSPContext {
void (*get_pixels)(int16_t *av_restrict block ,
const uint8_t *pixels ,
ptrdiff_t stride);
void (*diff_pixels)(int16_t *av_restrict block ,
const uint8_t *s1 ,
const uint8_t *s2 ,
ptrdiff_t stride);
void (*diff_pixels_unaligned)(int16_t *av_restrict block ,
const uint8_t *s1,
const uint8_t *s2,
ptrdiff_t stride);
} PixblockDSPContext;
void ff_pixblockdsp_init(PixblockDSPContext *c, AVCodecContext *avctx);
void ff_pixblockdsp_init_alpha(PixblockDSPContext *c, AVCodecContext *avctx,
unsigned high_bit_depth);
void ff_pixblockdsp_init_arm(PixblockDSPContext *c, AVCodecContext *avctx,
unsigned high_bit_depth);
void ff_pixblockdsp_init_ppc(PixblockDSPContext *c, AVCodecContext *avctx,
unsigned high_bit_depth);
void ff_pixblockdsp_init_x86(PixblockDSPContext *c, AVCodecContext *avctx,
unsigned high_bit_depth);
void ff_pixblockdsp_init_mips(PixblockDSPContext *c, AVCodecContext *avctx,
unsigned high_bit_depth);
