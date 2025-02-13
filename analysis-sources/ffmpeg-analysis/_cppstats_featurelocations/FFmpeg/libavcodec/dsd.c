






















#include "libavcodec/internal.h"
#include "libavcodec/mathops.h"
#include "avcodec.h"
#include "dsd_tablegen.h"
#include "dsd.h"

static av_cold void dsd_ctables_tableinit(void)
{
int t, e, m, sign;
double acc[CTABLES];
for (e = 0; e < 256; ++e) {
memset(acc, 0, sizeof(acc));
for (m = 0; m < 8; ++m) {
sign = (((e >> (7 - m)) & 1) * 2 - 1);
for (t = 0; t < CTABLES; ++t)
acc[t] += sign * htaps[t * 8 + m];
}
for (t = 0; t < CTABLES; ++t)
ctables[CTABLES - 1 - t][e] = acc[t];
}
}

av_cold void ff_init_dsd_data(void)
{
static int done = 0;
if (done)
return;
dsd_ctables_tableinit();
done = 1;
}

void ff_dsd2pcm_translate(DSDContext* s, size_t samples, int lsbf,
const uint8_t *src, ptrdiff_t src_stride,
float *dst, ptrdiff_t dst_stride)
{
uint8_t buf[FIFOSIZE];
unsigned pos, i;
uint8_t* p;
double sum;

pos = s->pos;

memcpy(buf, s->buf, sizeof(buf));

while (samples-- > 0) {
buf[pos] = lsbf ? ff_reverse[*src] : *src;
src += src_stride;

p = buf + ((pos - CTABLES) & FIFOMASK);
*p = ff_reverse[*p];

sum = 0.0;
for (i = 0; i < CTABLES; i++) {
uint8_t a = buf[(pos - i) & FIFOMASK];
uint8_t b = buf[(pos - (CTABLES*2 - 1) + i) & FIFOMASK];
sum += ctables[i][a] + ctables[i][b];
}

*dst = (float)sum;
dst += dst_stride;

pos = (pos + 1) & FIFOMASK;
}

s->pos = pos;
memcpy(s->buf, buf, sizeof(buf));
}
