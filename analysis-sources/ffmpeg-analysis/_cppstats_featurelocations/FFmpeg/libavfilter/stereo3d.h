



















#if !defined(AVFILTER_STEREO3D_H)
#define AVFILTER_STEREO3D_H

#include <stddef.h>
#include <stdint.h>

typedef struct Stereo3DDSPContext {
void (*anaglyph)(uint8_t *dst, uint8_t *lsrc, uint8_t *rsrc,
ptrdiff_t dst_linesize, ptrdiff_t l_linesize, ptrdiff_t r_linesize,
int width, int height,
const int *ana_matrix_r, const int *ana_matrix_g, const int *ana_matrix_b);
} Stereo3DDSPContext;

void ff_stereo3d_init_x86(Stereo3DDSPContext *dsp);

#endif 
