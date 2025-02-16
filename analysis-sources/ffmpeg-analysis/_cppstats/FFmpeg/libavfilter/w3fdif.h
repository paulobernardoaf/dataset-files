#include <stddef.h>
#include <stdint.h>
typedef struct W3FDIFDSPContext {
void (*filter_simple_low)(int32_t *work_line,
uint8_t *in_lines_cur[2],
const int16_t *coef, int linesize);
void (*filter_complex_low)(int32_t *work_line,
uint8_t *in_lines_cur[4],
const int16_t *coef, int linesize);
void (*filter_simple_high)(int32_t *work_line,
uint8_t *in_lines_cur[3],
uint8_t *in_lines_adj[3],
const int16_t *coef, int linesize);
void (*filter_complex_high)(int32_t *work_line,
uint8_t *in_lines_cur[5],
uint8_t *in_lines_adj[5],
const int16_t *coef, int linesize);
void (*filter_scale)(uint8_t *out_pixel, const int32_t *work_pixel,
int linesize, int max);
} W3FDIFDSPContext;
void ff_w3fdif_init_x86(W3FDIFDSPContext *dsp, int depth);
