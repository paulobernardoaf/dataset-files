#include "avfilter.h"
typedef struct GradFunContext {
const AVClass *class;
float strength;
int thresh; 
int radius; 
int chroma_w; 
int chroma_h; 
int chroma_r; 
uint16_t *buf; 
void (*filter_line) (uint8_t *dst, const uint8_t *src, const uint16_t *dc, int width, int thresh, const uint16_t *dithers);
void (*blur_line) (uint16_t *dc, uint16_t *buf, const uint16_t *buf1, const uint8_t *src, int src_linesize, int width);
} GradFunContext;
void ff_gradfun_init_x86(GradFunContext *gf);
void ff_gradfun_filter_line_c(uint8_t *dst, const uint8_t *src, const uint16_t *dc, int width, int thresh, const uint16_t *dithers);
void ff_gradfun_blur_line_c(uint16_t *dc, uint16_t *buf, const uint16_t *buf1, const uint8_t *src, int src_linesize, int width);
