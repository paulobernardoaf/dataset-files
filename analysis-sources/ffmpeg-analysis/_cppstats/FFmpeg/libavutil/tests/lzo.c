#include <stdio.h>
#include <lzo/lzo1x.h>
#include "libavutil/log.h"
#include "libavutil/lzo.h"
#include "libavutil/mem.h"
#define MAXSZ (10*1024*1024)
#define BENCHMARK_LIBLZO_SAFE 0
#define BENCHMARK_LIBLZO_UNSAFE 0
int main(int argc, char *argv[]) {
FILE *in = fopen(argv[1], "rb");
int comp_level = argc > 2 ? atoi(argv[2]) : 0;
uint8_t *orig = av_malloc(MAXSZ + 16);
uint8_t *comp = av_malloc(2*MAXSZ + 16);
uint8_t *decomp = av_malloc(MAXSZ + 16);
size_t s = fread(orig, 1, MAXSZ, in);
lzo_uint clen = 0;
long tmp[LZO1X_MEM_COMPRESS];
int inlen, outlen;
int i;
av_log_set_level(AV_LOG_DEBUG);
if (comp_level == 0) {
lzo1x_1_compress(orig, s, comp, &clen, tmp);
} else if (comp_level == 11) {
lzo1x_1_11_compress(orig, s, comp, &clen, tmp);
} else if (comp_level == 12) {
lzo1x_1_12_compress(orig, s, comp, &clen, tmp);
} else if (comp_level == 15) {
lzo1x_1_15_compress(orig, s, comp, &clen, tmp);
} else
lzo1x_999_compress(orig, s, comp, &clen, tmp);
for (i = 0; i < 300; i++) {
START_TIMER
inlen = clen; outlen = MAXSZ;
#if BENCHMARK_LIBLZO_SAFE
if (lzo1x_decompress_safe(comp, inlen, decomp, &outlen, NULL))
#elif BENCHMARK_LIBLZO_UNSAFE
if (lzo1x_decompress(comp, inlen, decomp, &outlen, NULL))
#else
if (av_lzo1x_decode(decomp, &outlen, comp, &inlen))
#endif
av_log(NULL, AV_LOG_ERROR, "decompression error\n");
STOP_TIMER("lzod")
}
if (memcmp(orig, decomp, s))
av_log(NULL, AV_LOG_ERROR, "decompression incorrect\n");
else
av_log(NULL, AV_LOG_ERROR, "decompression OK\n");
fclose(in);
av_free(orig);
av_free(comp);
av_free(decomp);
return 0;
}
