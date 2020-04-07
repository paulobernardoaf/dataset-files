



















#include "libavutil/attributes.h"
#include "libavutil/cpu.h"
#include "libavutil/x86/cpu.h"
#include "libavfilter/vf_overlay.h"

int ff_overlay_row_44_sse4(uint8_t *d, uint8_t *da, uint8_t *s, uint8_t *a,
int w, ptrdiff_t alinesize);

int ff_overlay_row_20_sse4(uint8_t *d, uint8_t *da, uint8_t *s, uint8_t *a,
int w, ptrdiff_t alinesize);

int ff_overlay_row_22_sse4(uint8_t *d, uint8_t *da, uint8_t *s, uint8_t *a,
int w, ptrdiff_t alinesize);

av_cold void ff_overlay_init_x86(OverlayContext *s, int format, int pix_format,
int alpha_format, int main_has_alpha)
{
int cpu_flags = av_get_cpu_flags();

if (EXTERNAL_SSE4(cpu_flags) &&
(format == OVERLAY_FORMAT_YUV444 ||
format == OVERLAY_FORMAT_GBRP) &&
alpha_format == 0 && main_has_alpha == 0) {
s->blend_row[0] = ff_overlay_row_44_sse4;
s->blend_row[1] = ff_overlay_row_44_sse4;
s->blend_row[2] = ff_overlay_row_44_sse4;
}

if (EXTERNAL_SSE4(cpu_flags) &&
(pix_format == AV_PIX_FMT_YUV420P) &&
(format == OVERLAY_FORMAT_YUV420) &&
alpha_format == 0 && main_has_alpha == 0) {
s->blend_row[0] = ff_overlay_row_44_sse4;
s->blend_row[1] = ff_overlay_row_20_sse4;
s->blend_row[2] = ff_overlay_row_20_sse4;
}

if (EXTERNAL_SSE4(cpu_flags) &&
(format == OVERLAY_FORMAT_YUV422) &&
alpha_format == 0 && main_has_alpha == 0) {
s->blend_row[0] = ff_overlay_row_44_sse4;
s->blend_row[1] = ff_overlay_row_22_sse4;
s->blend_row[2] = ff_overlay_row_22_sse4;
}
}
