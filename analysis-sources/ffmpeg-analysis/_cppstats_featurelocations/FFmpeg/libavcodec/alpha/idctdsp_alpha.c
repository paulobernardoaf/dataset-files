



















#include "libavutil/attributes.h"
#include "libavcodec/idctdsp.h"
#include "idctdsp_alpha.h"
#include "asm.h"

void put_pixels_clamped_mvi_asm(const int16_t *block, uint8_t *pixels,
ptrdiff_t line_size);
void add_pixels_clamped_mvi_asm(const int16_t *block, uint8_t *pixels,
ptrdiff_t line_size);

void (*put_pixels_clamped_axp_p)(const int16_t *block, uint8_t *pixels,
ptrdiff_t line_size);
void (*add_pixels_clamped_axp_p)(const int16_t *block, uint8_t *pixels,
ptrdiff_t line_size);

#if 0


static void put_pixels_clamped_mvi(const int16_t *block, uint8_t *pixels,
ptrdiff_t line_size)
{
int i = 8;
uint64_t clampmask = zap(-1, 0xaa); 

do {
uint64_t shorts0, shorts1;

shorts0 = ldq(block);
shorts0 = maxsw4(shorts0, 0);
shorts0 = minsw4(shorts0, clampmask);
stl(pkwb(shorts0), pixels);

shorts1 = ldq(block + 4);
shorts1 = maxsw4(shorts1, 0);
shorts1 = minsw4(shorts1, clampmask);
stl(pkwb(shorts1), pixels + 4);

pixels += line_size;
block += 8;
} while (--i);
}

void add_pixels_clamped_mvi(const int16_t *block, uint8_t *pixels,
ptrdiff_t line_size)
{
int h = 8;


uint64_t clampmask = zap(-1, 0xaa); 
uint64_t signmask = zap(-1, 0x33);
signmask ^= signmask >> 1; 

do {
uint64_t shorts0, pix0, signs0;
uint64_t shorts1, pix1, signs1;

shorts0 = ldq(block);
shorts1 = ldq(block + 4);

pix0 = unpkbw(ldl(pixels));

signs0 = shorts0 & signmask;
shorts0 &= ~signmask;
shorts0 += pix0;
shorts0 ^= signs0;

shorts0 = maxsw4(shorts0, 0);
shorts0 = minsw4(shorts0, clampmask);


pix1 = unpkbw(ldl(pixels + 4));
signs1 = shorts1 & signmask;
shorts1 &= ~signmask;
shorts1 += pix1;
shorts1 ^= signs1;
shorts1 = maxsw4(shorts1, 0);
shorts1 = minsw4(shorts1, clampmask);

stl(pkwb(shorts0), pixels);
stl(pkwb(shorts1), pixels + 4);

pixels += line_size;
block += 8;
} while (--h);
}
#endif

av_cold void ff_idctdsp_init_alpha(IDCTDSPContext *c, AVCodecContext *avctx,
unsigned high_bit_depth)
{

if (amask(AMASK_MVI) == 0) {
c->put_pixels_clamped = put_pixels_clamped_mvi_asm;
c->add_pixels_clamped = add_pixels_clamped_mvi_asm;
}

put_pixels_clamped_axp_p = c->put_pixels_clamped;
add_pixels_clamped_axp_p = c->add_pixels_clamped;

if (!high_bit_depth && !avctx->lowres &&
(avctx->idct_algo == FF_IDCT_AUTO)) {
c->idct_put = ff_simple_idct_put_axp;
c->idct_add = ff_simple_idct_add_axp;
c->idct = ff_simple_idct_axp;
}
}
