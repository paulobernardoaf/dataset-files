#include "checkasm.h"
#include "libavfilter/vf_nlmeans.h"
#include "libavutil/avassert.h"
#define randomize_buffer(buf, size) do { int i; for (i = 0; i < size / 4; i++) ((uint32_t *)buf)[i] = rnd(); } while (0)
void checkasm_check_nlmeans(void)
{
NLMeansDSPContext dsp = {0};
const int w = 123; 
const int h = 45; 
const int p = 3; 
const int r = 2; 
ff_nlmeans_init(&dsp);
if (check_func(dsp.compute_safe_ssd_integral_image, "ssd_integral_image")) {
int offx, offy;
const int e = p + r;
const int ii_w = w + e*2;
const int ii_h = h + e*2;
const int ii_lz_32 = FFALIGN(ii_w + 1, 4);
uint32_t *ii_orig_ref = av_mallocz_array(ii_h + 1, ii_lz_32 * sizeof(*ii_orig_ref));
uint32_t *ii_ref = ii_orig_ref + ii_lz_32 + 1;
uint32_t *ii_orig_new = av_mallocz_array(ii_h + 1, ii_lz_32 * sizeof(*ii_orig_new));
uint32_t *ii_new = ii_orig_new + ii_lz_32 + 1;
const int src_lz = FFALIGN(w, 16);
uint8_t *src = av_mallocz_array(h, src_lz);
declare_func(void, uint32_t *dst, ptrdiff_t dst_linesize_32,
const uint8_t *s1, ptrdiff_t linesize1,
const uint8_t *s2, ptrdiff_t linesize2,
int w, int h);
randomize_buffer(src, h * src_lz);
for (offy = -r; offy <= r; offy++) {
for (offx = -r; offx <= r; offx++) {
if (offx || offy) {
const int s1x = e;
const int s1y = e;
const int s2x = e + offx;
const int s2y = e + offy;
const int startx_safe = FFMAX(s1x, s2x);
const int starty_safe = FFMAX(s1y, s2y);
const int u_endx_safe = FFMIN(s1x + w, s2x + w);
const int endy_safe = FFMIN(s1y + h, s2y + h);
const int safe_pw = (u_endx_safe - startx_safe) & ~0xf;
const int safe_ph = endy_safe - starty_safe;
av_assert0(safe_pw && safe_ph);
av_assert0(startx_safe - s1x >= 0); av_assert0(startx_safe - s1x < w);
av_assert0(starty_safe - s1y >= 0); av_assert0(starty_safe - s1y < h);
av_assert0(startx_safe - s2x >= 0); av_assert0(startx_safe - s2x < w);
av_assert0(starty_safe - s2y >= 0); av_assert0(starty_safe - s2y < h);
memset(ii_ref, 0, (ii_lz_32 * ii_h - 1) * sizeof(*ii_ref));
memset(ii_new, 0, (ii_lz_32 * ii_h - 1) * sizeof(*ii_new));
call_ref(ii_ref + starty_safe*ii_lz_32 + startx_safe, ii_lz_32,
src + (starty_safe - s1y) * src_lz + (startx_safe - s1x), src_lz,
src + (starty_safe - s2y) * src_lz + (startx_safe - s2x), src_lz,
safe_pw, safe_ph);
call_new(ii_new + starty_safe*ii_lz_32 + startx_safe, ii_lz_32,
src + (starty_safe - s1y) * src_lz + (startx_safe - s1x), src_lz,
src + (starty_safe - s2y) * src_lz + (startx_safe - s2x), src_lz,
safe_pw, safe_ph);
if (memcmp(ii_ref, ii_new, (ii_lz_32 * ii_h - 1) * sizeof(*ii_ref)))
fail();
memset(ii_new, 0, (ii_lz_32 * ii_h - 1) * sizeof(*ii_new));
bench_new(ii_new + starty_safe*ii_lz_32 + startx_safe, ii_lz_32,
src + (starty_safe - s1y) * src_lz + (startx_safe - s1x), src_lz,
src + (starty_safe - s2y) * src_lz + (startx_safe - s2x), src_lz,
safe_pw, safe_ph);
}
}
}
av_freep(&ii_orig_ref);
av_freep(&ii_orig_new);
av_freep(&src);
}
report("dsp");
}
