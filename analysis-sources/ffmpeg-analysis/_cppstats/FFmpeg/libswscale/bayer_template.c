#if defined(BAYER_BGGR) || defined(BAYER_GBRG)
#define BAYER_R 0
#define BAYER_G 1
#define BAYER_B 2
#endif
#if defined(BAYER_RGGB) || defined(BAYER_GRBG)
#define BAYER_R 2
#define BAYER_G 1
#define BAYER_B 0
#endif
#if defined(BAYER_8)
#define BAYER_READ(x) (x)
#define BAYER_SIZEOF 1
#define BAYER_SHIFT 0
#endif
#if defined(BAYER_16LE)
#define BAYER_READ(x) AV_RL16(&(x))
#define BAYER_SIZEOF 2
#define BAYER_SHIFT 8
#endif
#if defined(BAYER_16BE)
#define BAYER_READ(x) AV_RB16(&(x))
#define BAYER_SIZEOF 2
#define BAYER_SHIFT 8
#endif
#define S(y, x) BAYER_READ(src[(y)*src_stride + BAYER_SIZEOF*(x)])
#define T(y, x) (unsigned int)S(y, x)
#define R(y, x) dst[(y)*dst_stride + (x)*3 + BAYER_R]
#define G(y, x) dst[(y)*dst_stride + (x)*3 + BAYER_G]
#define B(y, x) dst[(y)*dst_stride + (x)*3 + BAYER_B]
#if defined(BAYER_BGGR) || defined(BAYER_RGGB)
#define BAYER_TO_RGB24_COPY R(0, 0) = R(0, 1) = R(1, 1) = R(1, 0) = S(1, 1) >> BAYER_SHIFT; G(0, 1) = S(0, 1) >> BAYER_SHIFT; G(0, 0) = G(1, 1) = (T(0, 1) + T(1, 0)) >> (1 + BAYER_SHIFT); G(1, 0) = S(1, 0) >> BAYER_SHIFT; B(1, 1) = B(0, 0) = B(0, 1) = B(1, 0) = S(0, 0) >> BAYER_SHIFT;
#define BAYER_TO_RGB24_INTERPOLATE R(0, 0) = (T(-1, -1) + T(-1, 1) + T(1, -1) + T(1, 1)) >> (2 + BAYER_SHIFT); G(0, 0) = (T(-1, 0) + T( 0, -1) + T(0, 1) + T(1, 0)) >> (2 + BAYER_SHIFT); B(0, 0) = S(0, 0) >> BAYER_SHIFT; R(0, 1) = (T(-1, 1) + T(1, 1)) >> (1 + BAYER_SHIFT); G(0, 1) = S(0, 1) >> BAYER_SHIFT; B(0, 1) = (T(0, 0) + T(0, 2)) >> (1 + BAYER_SHIFT); R(1, 0) = (T(1, -1) + T(1, 1)) >> (1 + BAYER_SHIFT); G(1, 0) = S(1, 0) >> BAYER_SHIFT; B(1, 0) = (T(0, 0) + T(2, 0)) >> (1 + BAYER_SHIFT); R(1, 1) = S(1, 1) >> BAYER_SHIFT; G(1, 1) = (T(0, 1) + T(1, 0) + T(1, 2) + T(2, 1)) >> (2 + BAYER_SHIFT); B(1, 1) = (T(0, 0) + T(0, 2) + T(2, 0) + T(2, 2)) >> (2 + BAYER_SHIFT);
#else
#define BAYER_TO_RGB24_COPY R(0, 0) = R(0, 1) = R(1, 1) = R(1, 0) = S(1, 0) >> BAYER_SHIFT; G(0, 0) = S(0, 0) >> BAYER_SHIFT; G(1, 1) = S(1, 1) >> BAYER_SHIFT; G(0, 1) = G(1, 0) = (T(0, 0) + T(1, 1)) >> (1 + BAYER_SHIFT); B(1, 1) = B(0, 0) = B(0, 1) = B(1, 0) = S(0, 1) >> BAYER_SHIFT;
#define BAYER_TO_RGB24_INTERPOLATE R(0, 0) = (T(-1, 0) + T(1, 0)) >> (1 + BAYER_SHIFT); G(0, 0) = S(0, 0) >> BAYER_SHIFT; B(0, 0) = (T(0, -1) + T(0, 1)) >> (1 + BAYER_SHIFT); R(0, 1) = (T(-1, 0) + T(-1, 2) + T(1, 0) + T(1, 2)) >> (2 + BAYER_SHIFT); G(0, 1) = (T(-1, 1) + T(0, 0) + T(0, 2) + T(1, 1)) >> (2 + BAYER_SHIFT); B(0, 1) = S(0, 1) >> BAYER_SHIFT; R(1, 0) = S(1, 0) >> BAYER_SHIFT; G(1, 0) = (T(0, 0) + T(1, -1) + T(1, 1) + T(2, 0)) >> (2 + BAYER_SHIFT); B(1, 0) = (T(0, -1) + T(0, 1) + T(2, -1) + T(2, 1)) >> (2 + BAYER_SHIFT); R(1, 1) = (T(1, 0) + T(1, 2)) >> (1 + BAYER_SHIFT); G(1, 1) = S(1, 1) >> BAYER_SHIFT; B(1, 1) = (T(0, 1) + T(2, 1)) >> (1 + BAYER_SHIFT);
#endif
#define rgb24toyv12_2x2(src, dstY, dstU, dstV, luma_stride, src_stride, rgb2yuv) ff_rgb24toyv12(src, dstY, dstV, dstU, 2, 2, luma_stride, 0, src_stride, rgb2yuv)
static void BAYER_RENAME(rgb24_copy)(const uint8_t *src, int src_stride, uint8_t *dst, int dst_stride, int width)
{
int i;
for (i = 0 ; i < width; i+= 2) {
BAYER_TO_RGB24_COPY
src += 2 * BAYER_SIZEOF;
dst += 6;
}
}
static void BAYER_RENAME(rgb24_interpolate)(const uint8_t *src, int src_stride, uint8_t *dst, int dst_stride, int width)
{
int i;
BAYER_TO_RGB24_COPY
src += 2 * BAYER_SIZEOF;
dst += 6;
for (i = 2 ; i < width - 2; i+= 2) {
BAYER_TO_RGB24_INTERPOLATE
src += 2 * BAYER_SIZEOF;
dst += 6;
}
if (width > 2) {
BAYER_TO_RGB24_COPY
}
}
static void BAYER_RENAME(yv12_copy)(const uint8_t *src, int src_stride, uint8_t *dstY, uint8_t *dstU, uint8_t *dstV, int luma_stride, int width, int32_t *rgb2yuv)
{
uint8_t dst[12];
const int dst_stride = 6;
int i;
for (i = 0 ; i < width; i+= 2) {
BAYER_TO_RGB24_COPY
rgb24toyv12_2x2(dst, dstY, dstU, dstV, luma_stride, dst_stride, rgb2yuv);
src += 2 * BAYER_SIZEOF;
dstY += 2;
dstU++;
dstV++;
}
}
static void BAYER_RENAME(yv12_interpolate)(const uint8_t *src, int src_stride, uint8_t *dstY, uint8_t *dstU, uint8_t *dstV, int luma_stride, int width, int32_t *rgb2yuv)
{
uint8_t dst[12];
const int dst_stride = 6;
int i;
BAYER_TO_RGB24_COPY
rgb24toyv12_2x2(dst, dstY, dstU, dstV, luma_stride, dst_stride, rgb2yuv);
src += 2 * BAYER_SIZEOF;
dstY += 2;
dstU++;
dstV++;
for (i = 2 ; i < width - 2; i+= 2) {
BAYER_TO_RGB24_INTERPOLATE
rgb24toyv12_2x2(dst, dstY, dstU, dstV, luma_stride, dst_stride, rgb2yuv);
src += 2 * BAYER_SIZEOF;
dstY += 2;
dstU++;
dstV++;
}
if (width > 2) {
BAYER_TO_RGB24_COPY
rgb24toyv12_2x2(dst, dstY, dstU, dstV, luma_stride, dst_stride, rgb2yuv);
}
}
#undef S
#undef T
#undef R
#undef G
#undef B
#undef BAYER_TO_RGB24_COPY
#undef BAYER_TO_RGB24_INTERPOLATE
#undef BAYER_RENAME
#undef BAYER_R
#undef BAYER_G
#undef BAYER_B
#undef BAYER_READ
#undef BAYER_SIZEOF
#undef BAYER_SHIFT
#if defined(BAYER_BGGR)
#undef BAYER_BGGR
#endif
#if defined(BAYER_RGGB)
#undef BAYER_RGGB
#endif
#if defined(BAYER_GBRG)
#undef BAYER_GBRG
#endif
#if defined(BAYER_GRBG)
#undef BAYER_GRBG
#endif
#if defined(BAYER_8)
#undef BAYER_8
#endif
#if defined(BAYER_16LE)
#undef BAYER_16LE
#endif
#if defined(BAYER_16BE)
#undef BAYER_16BE
#endif
