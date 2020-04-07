#include "libavutil/x86/w64xmmtest.h"
#include "libswscale/swscale.h"
wrap(sws_scale(struct SwsContext *c, const uint8_t *const srcSlice[],
const int srcStride[], int srcSliceY, int srcSliceH,
uint8_t *const dst[], const int dstStride[]))
{
testxmmclobbers(sws_scale, c, srcSlice, srcStride, srcSliceY,
srcSliceH, dst, dstStride);
}
