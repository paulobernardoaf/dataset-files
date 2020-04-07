#include "libavcodec/mpeg12.h"
#include "libavcodec/mpeg12data.h"
int main(void)
{
int i;
#define TEST_MATCH(frame_rate, code, ext_n, ext_d) do { AVRational fr = frame_rate; int c, n, d; ff_mpeg12_find_best_frame_rate(fr, &c, &n, &d, 0); if (c != code || n != ext_n || d != ext_d) { av_log(NULL, AV_LOG_ERROR, "Failed to match %d/%d: " "code = %d, ext_n = %d, ext_d = %d.\n", fr.num, fr.den, c, n, d); return 1; } } while (0)
#define TEST_EXACT(frn, frd) do { AVRational fr = (AVRational) { frn, frd }; int c, n, d; ff_mpeg12_find_best_frame_rate(fr, &c, &n, &d, 0); if (av_cmp_q(fr, av_mul_q(ff_mpeg12_frame_rate_tab[c], (AVRational) { n + 1, d + 1 })) != 0) { av_log(NULL, AV_LOG_ERROR, "Failed to find exact %d/%d: " "code = %d, ext_n = %d, ext_d = %d.\n", fr.num, fr.den, c, n, d); return 1; } } while (0)
for (i = 1; i <= 8; i++)
TEST_MATCH(ff_mpeg12_frame_rate_tab[i], i, 0, 0);
for (i = 1; i <= 8; i++) {
TEST_MATCH(av_sub_q(ff_mpeg12_frame_rate_tab[i],
(AVRational) { 1, 1000 }), i, 0, 0);
TEST_MATCH(av_add_q(ff_mpeg12_frame_rate_tab[i],
(AVRational) { 1, 1000 }), i, 0, 0);
}
TEST_EXACT( 1, 1);
TEST_EXACT( 2, 1);
TEST_EXACT( 12, 1);
TEST_EXACT( 15000, 1001);
TEST_EXACT( 15, 1);
TEST_EXACT( 120, 1);
TEST_EXACT(120000, 1001);
TEST_EXACT( 200, 1);
TEST_EXACT( 240, 1);
for (i = 240; i < 1000; i += 10)
TEST_MATCH(((AVRational) { i, 1 }), 8, 3, 0);
for (i = 74; i > 0; i--)
TEST_MATCH(((AVRational) { i, 100 }), 1, 0, 31);
return 0;
}
