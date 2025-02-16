#include <stddef.h>
#include <stdint.h>
#include "libavutil/intreadwrite.h"
static inline void copy_block2(uint8_t *dst, const uint8_t *src, ptrdiff_t dstStride, ptrdiff_t srcStride, int h)
{
int i;
for (i = 0; i < h; i++) {
AV_COPY16U(dst, src);
dst += dstStride;
src += srcStride;
}
}
static inline void copy_block4(uint8_t *dst, const uint8_t *src, ptrdiff_t dstStride, ptrdiff_t srcStride, int h)
{
int i;
for (i = 0; i < h; i++) {
AV_COPY32U(dst, src);
dst += dstStride;
src += srcStride;
}
}
static inline void copy_block8(uint8_t *dst, const uint8_t *src, ptrdiff_t dstStride, ptrdiff_t srcStride, int h)
{
int i;
for (i = 0; i < h; i++) {
AV_COPY64U(dst, src);
dst += dstStride;
src += srcStride;
}
}
static inline void copy_block9(uint8_t *dst, const uint8_t *src, ptrdiff_t dstStride, ptrdiff_t srcStride, int h)
{
int i;
for (i = 0; i < h; i++) {
AV_COPY64U(dst, src);
dst[8] = src[8];
dst += dstStride;
src += srcStride;
}
}
static inline void copy_block16(uint8_t *dst, const uint8_t *src, ptrdiff_t dstStride, ptrdiff_t srcStride, int h)
{
int i;
for (i = 0; i < h; i++) {
AV_COPY128U(dst, src);
dst += dstStride;
src += srcStride;
}
}
static inline void copy_block17(uint8_t *dst, const uint8_t *src, ptrdiff_t dstStride, ptrdiff_t srcStride, int h)
{
int i;
for (i = 0; i < h; i++) {
AV_COPY128U(dst, src);
dst[16] = src[16];
dst += dstStride;
src += srcStride;
}
}
