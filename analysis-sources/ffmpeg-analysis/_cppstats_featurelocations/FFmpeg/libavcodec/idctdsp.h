

















#if !defined(AVCODEC_IDCTDSP_H)
#define AVCODEC_IDCTDSP_H

#include <stdint.h>

#include "config.h"

#include "avcodec.h"




typedef struct ScanTable {
const uint8_t *scantable;
uint8_t permutated[64];
uint8_t raster_end[64];
} ScanTable;

enum idct_permutation_type {
FF_IDCT_PERM_NONE,
FF_IDCT_PERM_LIBMPEG2,
FF_IDCT_PERM_SIMPLE,
FF_IDCT_PERM_TRANSPOSE,
FF_IDCT_PERM_PARTTRANS,
FF_IDCT_PERM_SSE2,
};

void ff_init_scantable(uint8_t *permutation, ScanTable *st,
const uint8_t *src_scantable);
void ff_init_scantable_permutation(uint8_t *idct_permutation,
enum idct_permutation_type perm_type);
int ff_init_scantable_permutation_x86(uint8_t *idct_permutation,
enum idct_permutation_type perm_type);

typedef struct IDCTDSPContext {

void (*put_pixels_clamped)(const int16_t *block ,
uint8_t *av_restrict pixels ,
ptrdiff_t line_size);
void (*put_signed_pixels_clamped)(const int16_t *block ,
uint8_t *av_restrict pixels ,
ptrdiff_t line_size);
void (*add_pixels_clamped)(const int16_t *block ,
uint8_t *av_restrict pixels ,
ptrdiff_t line_size);

void (*idct)(int16_t *block );






void (*idct_put)(uint8_t *dest ,
ptrdiff_t line_size, int16_t *block );





void (*idct_add)(uint8_t *dest ,
ptrdiff_t line_size, int16_t *block );















uint8_t idct_permutation[64];
enum idct_permutation_type perm_type;

int mpeg4_studio_profile;
} IDCTDSPContext;

void ff_put_pixels_clamped_c(const int16_t *block, uint8_t *av_restrict pixels,
ptrdiff_t line_size);
void ff_add_pixels_clamped_c(const int16_t *block, uint8_t *av_restrict pixels,
ptrdiff_t line_size);

void ff_idctdsp_init(IDCTDSPContext *c, AVCodecContext *avctx);

void ff_idctdsp_init_aarch64(IDCTDSPContext *c, AVCodecContext *avctx,
unsigned high_bit_depth);
void ff_idctdsp_init_alpha(IDCTDSPContext *c, AVCodecContext *avctx,
unsigned high_bit_depth);
void ff_idctdsp_init_arm(IDCTDSPContext *c, AVCodecContext *avctx,
unsigned high_bit_depth);
void ff_idctdsp_init_ppc(IDCTDSPContext *c, AVCodecContext *avctx,
unsigned high_bit_depth);
void ff_idctdsp_init_x86(IDCTDSPContext *c, AVCodecContext *avctx,
unsigned high_bit_depth);
void ff_idctdsp_init_mips(IDCTDSPContext *c, AVCodecContext *avctx,
unsigned high_bit_depth);

#endif 
