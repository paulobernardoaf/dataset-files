#include <assert.h>
#include <math.h>
#include "libavcodec/aac_defines.h"
#include "libavutil/attributes.h"
#include "libavutil/common.h"
#if !USE_FIXED
SINETABLE120960(120);
SINETABLE120960(960);
#endif
#if !CONFIG_HARDCODED_TABLES
SINETABLE( 32);
SINETABLE( 64);
SINETABLE( 128);
SINETABLE( 256);
SINETABLE( 512);
SINETABLE(1024);
SINETABLE(2048);
SINETABLE(4096);
SINETABLE(8192);
#else
#if USE_FIXED
#include "libavcodec/sinewin_fixed_tables.h"
#else
#include "libavcodec/sinewin_tables.h"
#endif
#endif
#if USE_FIXED
#define SIN_FIX(a) (int)floor((a) * 0x80000000 + 0.5)
#else
#define SIN_FIX(a) a
#endif
SINETABLE_CONST INTFLOAT * const AAC_RENAME(ff_sine_windows)[] = {
NULL, NULL, NULL, NULL, NULL, 
AAC_RENAME(ff_sine_32) , AAC_RENAME(ff_sine_64), AAC_RENAME(ff_sine_128),
AAC_RENAME(ff_sine_256), AAC_RENAME(ff_sine_512), AAC_RENAME(ff_sine_1024),
AAC_RENAME(ff_sine_2048), AAC_RENAME(ff_sine_4096), AAC_RENAME(ff_sine_8192),
};
av_cold void AAC_RENAME(ff_sine_window_init)(INTFLOAT *window, int n) {
int i;
for(i = 0; i < n; i++)
window[i] = SIN_FIX(sinf((i + 0.5) * (M_PI / (2.0 * n))));
}
av_cold void AAC_RENAME(ff_init_ff_sine_windows)(int index) {
assert(index >= 0 && index < FF_ARRAY_ELEMS(AAC_RENAME(ff_sine_windows)));
#if !CONFIG_HARDCODED_TABLES
AAC_RENAME(ff_sine_window_init)(AAC_RENAME(ff_sine_windows)[index], 1 << index);
#endif
}
