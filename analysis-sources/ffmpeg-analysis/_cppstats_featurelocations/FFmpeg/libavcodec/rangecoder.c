































#include <string.h>

#include "libavutil/attributes.h"
#include "libavutil/avassert.h"
#include "libavutil/intreadwrite.h"

#include "avcodec.h"
#include "rangecoder.h"

av_cold void ff_init_range_encoder(RangeCoder *c, uint8_t *buf, int buf_size)
{
c->bytestream_start =
c->bytestream = buf;
c->bytestream_end = buf + buf_size;
c->low = 0;
c->range = 0xFF00;
c->outstanding_count = 0;
c->outstanding_byte = -1;
}

av_cold void ff_init_range_decoder(RangeCoder *c, const uint8_t *buf,
int buf_size)
{

ff_init_range_encoder(c, (uint8_t *)buf, buf_size);

c->low = AV_RB16(c->bytestream);
c->bytestream += 2;
c->overread = 0;
if (c->low >= 0xFF00) {
c->low = 0xFF00;
c->bytestream_end = c->bytestream;
}
}

void ff_build_rac_states(RangeCoder *c, int factor, int max_p)
{
const int64_t one = 1LL << 32;
int64_t p;
int last_p8, p8, i;

memset(c->zero_state, 0, sizeof(c->zero_state));
memset(c->one_state, 0, sizeof(c->one_state));

last_p8 = 0;
p = one / 2;
for (i = 0; i < 128; i++) {
p8 = (256 * p + one / 2) >> 32; 
if (p8 <= last_p8)
p8 = last_p8 + 1;
if (last_p8 && last_p8 < 256 && p8 <= max_p)
c->one_state[last_p8] = p8;

p += ((one - p) * factor + one / 2) >> 32;
last_p8 = p8;
}

for (i = 256 - max_p; i <= max_p; i++) {
if (c->one_state[i])
continue;

p = (i * one + 128) >> 8;
p += ((one - p) * factor + one / 2) >> 32;
p8 = (256 * p + one / 2) >> 32; 
if (p8 <= i)
p8 = i + 1;
if (p8 > max_p)
p8 = max_p;
c->one_state[i] = p8;
}

for (i = 1; i < 255; i++)
c->zero_state[i] = 256 - c->one_state[256 - i];
}


int ff_rac_terminate(RangeCoder *c, int version)
{
if (version == 1)
put_rac(c, (uint8_t[]) { 129 }, 0);
c->range = 0xFF;
c->low += 0xFF;
renorm_encoder(c);
c->range = 0xFF;
renorm_encoder(c);

av_assert1(c->low == 0);
av_assert1(c->range >= 0x100);

return c->bytestream - c->bytestream_start;
}

int ff_rac_check_termination(RangeCoder *c, int version)
{
if (version == 1) {
RangeCoder tmp = *c;
get_rac(c, (uint8_t[]) { 129 });

if (c->bytestream == tmp.bytestream && c->bytestream > c->bytestream_start)
tmp.low -= *--tmp.bytestream;
tmp.bytestream_end = tmp.bytestream;

if (get_rac(&tmp, (uint8_t[]) { 129 }))
return AVERROR_INVALIDDATA;
} else {
if (c->bytestream_end != c->bytestream)
return AVERROR_INVALIDDATA;
}
return 0;
}
