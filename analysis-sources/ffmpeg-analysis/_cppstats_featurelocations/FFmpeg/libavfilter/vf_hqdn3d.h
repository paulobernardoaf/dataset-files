





















#if !defined(AVFILTER_HQDN3D_H)
#define AVFILTER_HQDN3D_H

#include <stddef.h>
#include <stdint.h>

#include "libavutil/opt.h"

typedef struct HQDN3DContext {
const AVClass *class;
int16_t *coefs[4];
uint16_t *line[3];
uint16_t *frame_prev[3];
double strength[4];
int hsub, vsub;
int depth;
void (*denoise_row[17])(uint8_t *src, uint8_t *dst, uint16_t *line_ant, uint16_t *frame_ant, ptrdiff_t w, int16_t *spatial, int16_t *temporal);
} HQDN3DContext;

#define LUMA_SPATIAL 0
#define LUMA_TMP 1
#define CHROMA_SPATIAL 2
#define CHROMA_TMP 3

void ff_hqdn3d_init_x86(HQDN3DContext *hqdn3d);

#endif 
