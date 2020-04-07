#include <stddef.h>
#include <stdint.h>
typedef struct NLMeansDSPContext {
void (*compute_safe_ssd_integral_image)(uint32_t *dst, ptrdiff_t dst_linesize_32,
const uint8_t *s1, ptrdiff_t linesize1,
const uint8_t *s2, ptrdiff_t linesize2,
int w, int h);
} NLMeansDSPContext;
void ff_nlmeans_init(NLMeansDSPContext *dsp);
void ff_nlmeans_init_aarch64(NLMeansDSPContext *dsp);
