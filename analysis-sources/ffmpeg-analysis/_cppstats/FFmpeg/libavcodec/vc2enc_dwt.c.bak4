





















#include "libavutil/attributes.h"
#include "libavutil/mem.h"
#include "vc2enc_dwt.h"




static av_always_inline void deinterleave(dwtcoef *linell, ptrdiff_t stride,
int width, int height, dwtcoef *synthl)
{
int x, y;
ptrdiff_t synthw = width << 1;
dwtcoef *linehl = linell + width;
dwtcoef *linelh = linell + height*stride;
dwtcoef *linehh = linelh + width;


for (y = 0; y < height; y++) {
for (x = 0; x < width; x++) {
linell[x] = synthl[(x << 1)];
linehl[x] = synthl[(x << 1) + 1];
linelh[x] = synthl[(x << 1) + synthw];
linehh[x] = synthl[(x << 1) + synthw + 1];
}
synthl += synthw << 1;
linell += stride;
linelh += stride;
linehl += stride;
linehh += stride;
}
}

static void vc2_subband_dwt_97(VC2TransformContext *t, dwtcoef *data,
ptrdiff_t stride, int width, int height)
{
int x, y;
dwtcoef *datal = data, *synth = t->buffer, *synthl = synth;
const ptrdiff_t synth_width = width << 1;
const ptrdiff_t synth_height = height << 1;





for (y = 0; y < synth_height; y++) {
for (x = 0; x < synth_width; x++)
synthl[x] = datal[x] * 2;
synthl += synth_width;
datal += stride;
}


synthl = synth;
for (y = 0; y < synth_height; y++) {

synthl[1] -= (8*synthl[0] + 9*synthl[2] - synthl[4] + 8) >> 4;
for (x = 1; x < width - 2; x++)
synthl[2*x + 1] -= (9*synthl[2*x] + 9*synthl[2*x + 2] - synthl[2*x + 4] -
synthl[2 * x - 2] + 8) >> 4;
synthl[synth_width - 1] -= (17*synthl[synth_width - 2] -
synthl[synth_width - 4] + 8) >> 4;
synthl[synth_width - 3] -= (8*synthl[synth_width - 2] +
9*synthl[synth_width - 4] -
synthl[synth_width - 6] + 8) >> 4;

synthl[0] += (synthl[1] + synthl[1] + 2) >> 2;
for (x = 1; x < width - 1; x++)
synthl[2*x] += (synthl[2*x - 1] + synthl[2*x + 1] + 2) >> 2;

synthl[synth_width - 2] += (synthl[synth_width - 3] +
synthl[synth_width - 1] + 2) >> 2;
synthl += synth_width;
}


synthl = synth + synth_width;
for (x = 0; x < synth_width; x++)
synthl[x] -= (8*synthl[x - synth_width] + 9*synthl[x + synth_width] -
synthl[x + 3 * synth_width] + 8) >> 4;

synthl = synth + (synth_width << 1);
for (y = 1; y < height - 2; y++) {
for (x = 0; x < synth_width; x++)
synthl[x + synth_width] -= (9*synthl[x] +
9*synthl[x + 2 * synth_width] -
synthl[x - 2 * synth_width] -
synthl[x + 4 * synth_width] + 8) >> 4;
synthl += synth_width << 1;
}

synthl = synth + (synth_height - 1) * synth_width;
for (x = 0; x < synth_width; x++) {
synthl[x] -= (17*synthl[x - synth_width] -
synthl[x - 3*synth_width] + 8) >> 4;
synthl[x - 2*synth_width] -= (9*synthl[x - 3*synth_width] +
8*synthl[x - 1*synth_width] - synthl[x - 5*synth_width] + 8) >> 4;
}


synthl = synth;
for (x = 0; x < synth_width; x++)
synthl[x] += (synthl[x + synth_width] + synthl[x + synth_width] + 2) >> 2;

synthl = synth + (synth_width << 1);
for (y = 1; y < height - 1; y++) {
for (x = 0; x < synth_width; x++)
synthl[x] += (synthl[x - synth_width] + synthl[x + synth_width] + 2) >> 2;
synthl += synth_width << 1;
}

synthl = synth + (synth_height - 2) * synth_width;
for (x = 0; x < synth_width; x++)
synthl[x] += (synthl[x - synth_width] + synthl[x + synth_width] + 2) >> 2;

deinterleave(data, stride, width, height, synth);
}

static void vc2_subband_dwt_53(VC2TransformContext *t, dwtcoef *data,
ptrdiff_t stride, int width, int height)
{
int x, y;
dwtcoef *synth = t->buffer, *synthl = synth, *datal = data;
const ptrdiff_t synth_width = width << 1;
const ptrdiff_t synth_height = height << 1;





for (y = 0; y < synth_height; y++) {
for (x = 0; x < synth_width; x++)
synthl[x] = datal[x] << 1;
synthl += synth_width;
datal += stride;
}


synthl = synth;
for (y = 0; y < synth_height; y++) {

for (x = 0; x < width - 1; x++)
synthl[2 * x + 1] -= (synthl[2 * x] + synthl[2 * x + 2] + 1) >> 1;

synthl[synth_width - 1] -= (2*synthl[synth_width - 2] + 1) >> 1;


synthl[0] += (2*synthl[1] + 2) >> 2;
for (x = 1; x < width - 1; x++)
synthl[2 * x] += (synthl[2 * x - 1] + synthl[2 * x + 1] + 2) >> 2;

synthl[synth_width - 2] += (synthl[synth_width - 3] + synthl[synth_width - 1] + 2) >> 2;

synthl += synth_width;
}


synthl = synth + synth_width;
for (x = 0; x < synth_width; x++)
synthl[x] -= (synthl[x - synth_width] + synthl[x + synth_width] + 1) >> 1;

synthl = synth + (synth_width << 1);
for (y = 1; y < height - 1; y++) {
for (x = 0; x < synth_width; x++)
synthl[x + synth_width] -= (synthl[x] + synthl[x + synth_width * 2] + 1) >> 1;
synthl += (synth_width << 1);
}

synthl = synth + (synth_height - 1) * synth_width;
for (x = 0; x < synth_width; x++)
synthl[x] -= (2*synthl[x - synth_width] + 1) >> 1;


synthl = synth;
for (x = 0; x < synth_width; x++)
synthl[x] += (2*synthl[synth_width + x] + 2) >> 2;

synthl = synth + (synth_width << 1);
for (y = 1; y < height - 1; y++) {
for (x = 0; x < synth_width; x++)
synthl[x] += (synthl[x + synth_width] + synthl[x - synth_width] + 2) >> 2;
synthl += (synth_width << 1);
}

synthl = synth + (synth_height - 2)*synth_width;
for (x = 0; x < synth_width; x++)
synthl[x] += (synthl[x - synth_width] + synthl[x + synth_width] + 2) >> 2;


deinterleave(data, stride, width, height, synth);
}

static av_always_inline void dwt_haar(VC2TransformContext *t, dwtcoef *data,
ptrdiff_t stride, int width, int height,
const int s)
{
int x, y;
dwtcoef *synth = t->buffer, *synthl = synth, *datal = data;
const ptrdiff_t synth_width = width << 1;
const ptrdiff_t synth_height = height << 1;


for (y = 0; y < synth_height; y++) {
for (x = 0; x < synth_width; x += 2) {
synthl[y*synth_width + x + 1] = (datal[y*stride + x + 1] << s) -
(datal[y*stride + x] << s);
synthl[y*synth_width + x] = (datal[y*stride + x + 0] << s) +
((synthl[y*synth_width + x + 1] + 1) >> 1);
}
}


for (x = 0; x < synth_width; x++) {
for (y = 0; y < synth_height; y += 2) {
synthl[(y + 1)*synth_width + x] = synthl[(y + 1)*synth_width + x] -
synthl[y*synth_width + x];
synthl[y*synth_width + x] = synthl[y*synth_width + x] +
((synthl[(y + 1)*synth_width + x] + 1) >> 1);
}
}

deinterleave(data, stride, width, height, synth);
}

static void vc2_subband_dwt_haar(VC2TransformContext *t, dwtcoef *data,
ptrdiff_t stride, int width, int height)
{
dwt_haar(t, data, stride, width, height, 0);
}

static void vc2_subband_dwt_haar_shift(VC2TransformContext *t, dwtcoef *data,
ptrdiff_t stride, int width, int height)
{
dwt_haar(t, data, stride, width, height, 1);
}

av_cold int ff_vc2enc_init_transforms(VC2TransformContext *s, int p_stride,
int p_height, int slice_w, int slice_h)
{
s->vc2_subband_dwt[VC2_TRANSFORM_9_7] = vc2_subband_dwt_97;
s->vc2_subband_dwt[VC2_TRANSFORM_5_3] = vc2_subband_dwt_53;
s->vc2_subband_dwt[VC2_TRANSFORM_HAAR] = vc2_subband_dwt_haar;
s->vc2_subband_dwt[VC2_TRANSFORM_HAAR_S] = vc2_subband_dwt_haar_shift;


s->buffer = av_calloc((p_stride + slice_w)*(p_height + slice_h), sizeof(dwtcoef));
if (!s->buffer)
return 1;

s->padding = (slice_h >> 1)*p_stride + (slice_w >> 1);
s->buffer += s->padding;

return 0;
}

av_cold void ff_vc2enc_free_transforms(VC2TransformContext *s)
{
av_free(s->buffer - s->padding);
s->buffer = NULL;
}
