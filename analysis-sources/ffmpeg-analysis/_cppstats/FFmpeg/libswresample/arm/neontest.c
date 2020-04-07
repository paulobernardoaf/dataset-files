#include "libswresample/swresample.h"
#include "libavutil/arm/neontest.h"
wrap(swr_convert(struct SwrContext *s, uint8_t **out, int out_count,
const uint8_t **in , int in_count))
{
testneonclobbers(swr_convert, s, out, out_count, in, in_count);
}
