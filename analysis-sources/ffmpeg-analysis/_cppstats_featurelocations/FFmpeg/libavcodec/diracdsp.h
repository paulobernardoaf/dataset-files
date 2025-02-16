



















#if !defined(AVCODEC_DIRACDSP_H)
#define AVCODEC_DIRACDSP_H

#include <stdint.h>
#include <stddef.h>

typedef void (*dirac_weight_func)(uint8_t *block, int stride, int log2_denom, int weight, int h);
typedef void (*dirac_biweight_func)(uint8_t *dst, const uint8_t *src, int stride, int log2_denom, int weightd, int weights, int h);

typedef struct {
void (*dirac_hpel_filter)(uint8_t *dsth, uint8_t *dstv, uint8_t *dstc, const uint8_t *src, int stride, int width, int height);










void (*put_dirac_pixels_tab[3][4])(uint8_t *dst, const uint8_t *src[5], int stride, int h);
void (*avg_dirac_pixels_tab[3][4])(uint8_t *dst, const uint8_t *src[5], int stride, int h);

void (*put_signed_rect_clamped[3])(uint8_t *dst, int dst_stride, const uint8_t *src, int src_stride, int width, int height);
void (*put_rect_clamped)(uint8_t *dst, int dst_stride, const uint8_t *src, int src_stride, int width, int height);
void (*add_rect_clamped)(uint8_t *dst, const uint16_t *src, int stride, const int16_t *idwt, int idwt_stride, int width, int height);
void (*add_dirac_obmc[3])(uint16_t *dst, const uint8_t *src, int stride, const uint8_t *obmc_weight, int yblen);


void (*dequant_subband[4])(uint8_t *src, uint8_t *dst, ptrdiff_t stride, const int qf, const int qs, int tot_v, int tot_h);

dirac_weight_func weight_dirac_pixels_tab[3];
dirac_biweight_func biweight_dirac_pixels_tab[3];
} DiracDSPContext;

#define DECL_DIRAC_PIXOP(PFX, EXT) void ff_ ##PFX ##_dirac_pixels8_ ##EXT(uint8_t *dst, const uint8_t *src[5], int stride, int h); void ff_ ##PFX ##_dirac_pixels16_ ##EXT(uint8_t *dst, const uint8_t *src[5], int stride, int h); void ff_ ##PFX ##_dirac_pixels32_ ##EXT(uint8_t *dst, const uint8_t *src[5], int stride, int h)




DECL_DIRAC_PIXOP(put, c);
DECL_DIRAC_PIXOP(avg, c);
DECL_DIRAC_PIXOP(put, l2_c);
DECL_DIRAC_PIXOP(avg, l2_c);
DECL_DIRAC_PIXOP(put, l4_c);
DECL_DIRAC_PIXOP(avg, l4_c);

void ff_diracdsp_init(DiracDSPContext *c);
void ff_diracdsp_init_x86(DiracDSPContext* c);

#endif 
