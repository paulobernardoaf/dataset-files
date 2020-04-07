

























#include <stdint.h>

#include "libavutil/error.h"
#include "libavutil/ffmath.h"

#include "opus_celt.h"
#include "opustab.h"
#include "internal.h"
#include "vorbis.h"

static const uint16_t opus_frame_duration[32] = {
480, 960, 1920, 2880,
480, 960, 1920, 2880,
480, 960, 1920, 2880,
480, 960,
480, 960,
120, 240, 480, 960,
120, 240, 480, 960,
120, 240, 480, 960,
120, 240, 480, 960,
};




static inline int xiph_lacing_16bit(const uint8_t **ptr, const uint8_t *end)
{
int val;

if (*ptr >= end)
return AVERROR_INVALIDDATA;
val = *(*ptr)++;
if (val >= 252) {
if (*ptr >= end)
return AVERROR_INVALIDDATA;
val += 4 * *(*ptr)++;
}
return val;
}




static inline int xiph_lacing_full(const uint8_t **ptr, const uint8_t *end)
{
int val = 0;
int next;

while (1) {
if (*ptr >= end || val > INT_MAX - 254)
return AVERROR_INVALIDDATA;
next = *(*ptr)++;
val += next;
if (next < 255)
break;
else
val--;
}
return val;
}




int ff_opus_parse_packet(OpusPacket *pkt, const uint8_t *buf, int buf_size,
int self_delimiting)
{
const uint8_t *ptr = buf;
const uint8_t *end = buf + buf_size;
int padding = 0;
int frame_bytes, i;

if (buf_size < 1)
goto fail;


i = *ptr++;
pkt->code = (i ) & 0x3;
pkt->stereo = (i >> 2) & 0x1;
pkt->config = (i >> 3) & 0x1F;


if (pkt->code >= 2 && buf_size < 2)
goto fail;

switch (pkt->code) {
case 0:

pkt->frame_count = 1;
pkt->vbr = 0;

if (self_delimiting) {
int len = xiph_lacing_16bit(&ptr, end);
if (len < 0 || len > end - ptr)
goto fail;
end = ptr + len;
buf_size = end - buf;
}

frame_bytes = end - ptr;
if (frame_bytes > MAX_FRAME_SIZE)
goto fail;
pkt->frame_offset[0] = ptr - buf;
pkt->frame_size[0] = frame_bytes;
break;
case 1:

pkt->frame_count = 2;
pkt->vbr = 0;

if (self_delimiting) {
int len = xiph_lacing_16bit(&ptr, end);
if (len < 0 || 2 * len > end - ptr)
goto fail;
end = ptr + 2 * len;
buf_size = end - buf;
}

frame_bytes = end - ptr;
if (frame_bytes & 1 || frame_bytes >> 1 > MAX_FRAME_SIZE)
goto fail;
pkt->frame_offset[0] = ptr - buf;
pkt->frame_size[0] = frame_bytes >> 1;
pkt->frame_offset[1] = pkt->frame_offset[0] + pkt->frame_size[0];
pkt->frame_size[1] = frame_bytes >> 1;
break;
case 2:

pkt->frame_count = 2;
pkt->vbr = 1;


frame_bytes = xiph_lacing_16bit(&ptr, end);
if (frame_bytes < 0)
goto fail;

if (self_delimiting) {
int len = xiph_lacing_16bit(&ptr, end);
if (len < 0 || len + frame_bytes > end - ptr)
goto fail;
end = ptr + frame_bytes + len;
buf_size = end - buf;
}

pkt->frame_offset[0] = ptr - buf;
pkt->frame_size[0] = frame_bytes;


frame_bytes = end - ptr - pkt->frame_size[0];
if (frame_bytes < 0 || frame_bytes > MAX_FRAME_SIZE)
goto fail;
pkt->frame_offset[1] = pkt->frame_offset[0] + pkt->frame_size[0];
pkt->frame_size[1] = frame_bytes;
break;
case 3:

i = *ptr++;
pkt->frame_count = (i ) & 0x3F;
padding = (i >> 6) & 0x01;
pkt->vbr = (i >> 7) & 0x01;

if (pkt->frame_count == 0 || pkt->frame_count > MAX_FRAMES)
goto fail;


if (padding) {
padding = xiph_lacing_full(&ptr, end);
if (padding < 0)
goto fail;
}


if (pkt->vbr) {


int total_bytes = 0;
for (i = 0; i < pkt->frame_count - 1; i++) {
frame_bytes = xiph_lacing_16bit(&ptr, end);
if (frame_bytes < 0)
goto fail;
pkt->frame_size[i] = frame_bytes;
total_bytes += frame_bytes;
}

if (self_delimiting) {
int len = xiph_lacing_16bit(&ptr, end);
if (len < 0 || len + total_bytes + padding > end - ptr)
goto fail;
end = ptr + total_bytes + len + padding;
buf_size = end - buf;
}

frame_bytes = end - ptr - padding;
if (total_bytes > frame_bytes)
goto fail;
pkt->frame_offset[0] = ptr - buf;
for (i = 1; i < pkt->frame_count; i++)
pkt->frame_offset[i] = pkt->frame_offset[i-1] + pkt->frame_size[i-1];
pkt->frame_size[pkt->frame_count-1] = frame_bytes - total_bytes;
} else {


if (self_delimiting) {
frame_bytes = xiph_lacing_16bit(&ptr, end);
if (frame_bytes < 0 || pkt->frame_count * frame_bytes + padding > end - ptr)
goto fail;
end = ptr + pkt->frame_count * frame_bytes + padding;
buf_size = end - buf;
} else {
frame_bytes = end - ptr - padding;
if (frame_bytes % pkt->frame_count ||
frame_bytes / pkt->frame_count > MAX_FRAME_SIZE)
goto fail;
frame_bytes /= pkt->frame_count;
}

pkt->frame_offset[0] = ptr - buf;
pkt->frame_size[0] = frame_bytes;
for (i = 1; i < pkt->frame_count; i++) {
pkt->frame_offset[i] = pkt->frame_offset[i-1] + pkt->frame_size[i-1];
pkt->frame_size[i] = frame_bytes;
}
}
}

pkt->packet_size = buf_size;
pkt->data_size = pkt->packet_size - padding;


pkt->frame_duration = opus_frame_duration[pkt->config];
if (pkt->frame_duration * pkt->frame_count > MAX_PACKET_DUR)
goto fail;


if (pkt->config < 12) {
pkt->mode = OPUS_MODE_SILK;
pkt->bandwidth = pkt->config >> 2;
} else if (pkt->config < 16) {
pkt->mode = OPUS_MODE_HYBRID;
pkt->bandwidth = OPUS_BANDWIDTH_SUPERWIDEBAND + (pkt->config >= 14);
} else {
pkt->mode = OPUS_MODE_CELT;
pkt->bandwidth = (pkt->config - 16) >> 2;

if (pkt->bandwidth)
pkt->bandwidth++;
}

return 0;

fail:
memset(pkt, 0, sizeof(*pkt));
return AVERROR_INVALIDDATA;
}

static int channel_reorder_vorbis(int nb_channels, int channel_idx)
{
return ff_vorbis_channel_layout_offsets[nb_channels - 1][channel_idx];
}

static int channel_reorder_unknown(int nb_channels, int channel_idx)
{
return channel_idx;
}

av_cold int ff_opus_parse_extradata(AVCodecContext *avctx,
OpusContext *s)
{
static const uint8_t default_channel_map[2] = { 0, 1 };

int (*channel_reorder)(int, int) = channel_reorder_unknown;

const uint8_t *extradata, *channel_map;
int extradata_size;
int version, channels, map_type, streams, stereo_streams, i, j;
uint64_t layout;

if (!avctx->extradata) {
if (avctx->channels > 2) {
av_log(avctx, AV_LOG_ERROR,
"Multichannel configuration without extradata.\n");
return AVERROR(EINVAL);
}
extradata = opus_default_extradata;
extradata_size = sizeof(opus_default_extradata);
} else {
extradata = avctx->extradata;
extradata_size = avctx->extradata_size;
}

if (extradata_size < 19) {
av_log(avctx, AV_LOG_ERROR, "Invalid extradata size: %d\n",
extradata_size);
return AVERROR_INVALIDDATA;
}

version = extradata[8];
if (version > 15) {
avpriv_request_sample(avctx, "Extradata version %d", version);
return AVERROR_PATCHWELCOME;
}

avctx->delay = AV_RL16(extradata + 10);
if (avctx->internal)
avctx->internal->skip_samples = avctx->delay;

channels = avctx->extradata ? extradata[9] : (avctx->channels == 1) ? 1 : 2;
if (!channels) {
av_log(avctx, AV_LOG_ERROR, "Zero channel count specified in the extradata\n");
return AVERROR_INVALIDDATA;
}

s->gain_i = AV_RL16(extradata + 16);
if (s->gain_i)
s->gain = ff_exp10(s->gain_i / (20.0 * 256));

map_type = extradata[18];
if (!map_type) {
if (channels > 2) {
av_log(avctx, AV_LOG_ERROR,
"Channel mapping 0 is only specified for up to 2 channels\n");
return AVERROR_INVALIDDATA;
}
layout = (channels == 1) ? AV_CH_LAYOUT_MONO : AV_CH_LAYOUT_STEREO;
streams = 1;
stereo_streams = channels - 1;
channel_map = default_channel_map;
} else if (map_type == 1 || map_type == 2 || map_type == 255) {
if (extradata_size < 21 + channels) {
av_log(avctx, AV_LOG_ERROR, "Invalid extradata size: %d\n",
extradata_size);
return AVERROR_INVALIDDATA;
}

streams = extradata[19];
stereo_streams = extradata[20];
if (!streams || stereo_streams > streams ||
streams + stereo_streams > 255) {
av_log(avctx, AV_LOG_ERROR,
"Invalid stream/stereo stream count: %d/%d\n", streams, stereo_streams);
return AVERROR_INVALIDDATA;
}

if (map_type == 1) {
if (channels > 8) {
av_log(avctx, AV_LOG_ERROR,
"Channel mapping 1 is only specified for up to 8 channels\n");
return AVERROR_INVALIDDATA;
}
layout = ff_vorbis_channel_layouts[channels - 1];
channel_reorder = channel_reorder_vorbis;
} else if (map_type == 2) {
int ambisonic_order = ff_sqrt(channels) - 1;
if (channels != ((ambisonic_order + 1) * (ambisonic_order + 1)) &&
channels != ((ambisonic_order + 1) * (ambisonic_order + 1) + 2)) {
av_log(avctx, AV_LOG_ERROR,
"Channel mapping 2 is only specified for channel counts"
" which can be written as (n + 1)^2 or (n + 1)^2 + 2"
" for nonnegative integer n\n");
return AVERROR_INVALIDDATA;
}
if (channels > 227) {
av_log(avctx, AV_LOG_ERROR, "Too many channels\n");
return AVERROR_INVALIDDATA;
}
layout = 0;
} else
layout = 0;

channel_map = extradata + 21;
} else {
avpriv_request_sample(avctx, "Mapping type %d", map_type);
return AVERROR_PATCHWELCOME;
}

s->channel_maps = av_mallocz_array(channels, sizeof(*s->channel_maps));
if (!s->channel_maps)
return AVERROR(ENOMEM);

for (i = 0; i < channels; i++) {
ChannelMap *map = &s->channel_maps[i];
uint8_t idx = channel_map[channel_reorder(channels, i)];

if (idx == 255) {
map->silence = 1;
continue;
} else if (idx >= streams + stereo_streams) {
av_log(avctx, AV_LOG_ERROR,
"Invalid channel map for output channel %d: %d\n", i, idx);
av_freep(&s->channel_maps);
return AVERROR_INVALIDDATA;
}


map->copy = 0;
for (j = 0; j < i; j++)
if (channel_map[channel_reorder(channels, j)] == idx) {
map->copy = 1;
map->copy_idx = j;
break;
}

if (idx < 2 * stereo_streams) {
map->stream_idx = idx / 2;
map->channel_idx = idx & 1;
} else {
map->stream_idx = idx - stereo_streams;
map->channel_idx = 0;
}
}

avctx->channels = channels;
avctx->channel_layout = layout;
s->nb_streams = streams;
s->nb_stereo_streams = stereo_streams;

return 0;
}

void ff_celt_quant_bands(CeltFrame *f, OpusRangeCoder *rc)
{
float lowband_scratch[8 * 22];
float norm1[2 * 8 * 100];
float *norm2 = norm1 + 8 * 100;

int totalbits = (f->framebits << 3) - f->anticollapse_needed;

int update_lowband = 1;
int lowband_offset = 0;

int i, j;

for (i = f->start_band; i < f->end_band; i++) {
uint32_t cm[2] = { (1 << f->blocks) - 1, (1 << f->blocks) - 1 };
int band_offset = ff_celt_freq_bands[i] << f->size;
int band_size = ff_celt_freq_range[i] << f->size;
float *X = f->block[0].coeffs + band_offset;
float *Y = (f->channels == 2) ? f->block[1].coeffs + band_offset : NULL;
float *norm_loc1, *norm_loc2;

int consumed = opus_rc_tell_frac(rc);
int effective_lowband = -1;
int b = 0;


if (i != f->start_band)
f->remaining -= consumed;
f->remaining2 = totalbits - consumed - 1;
if (i <= f->coded_bands - 1) {
int curr_balance = f->remaining / FFMIN(3, f->coded_bands-i);
b = av_clip_uintp2(FFMIN(f->remaining2 + 1, f->pulses[i] + curr_balance), 14);
}

if ((ff_celt_freq_bands[i] - ff_celt_freq_range[i] >= ff_celt_freq_bands[f->start_band] ||
i == f->start_band + 1) && (update_lowband || lowband_offset == 0))
lowband_offset = i;

if (i == f->start_band + 1) {


int count = (ff_celt_freq_range[i] - ff_celt_freq_range[i-1]) << f->size;

memcpy(&norm1[band_offset], &norm1[band_offset - count], count * sizeof(float));

if (f->channels == 2)
memcpy(&norm2[band_offset], &norm2[band_offset - count], count * sizeof(float));
}



if (lowband_offset != 0 && (f->spread != CELT_SPREAD_AGGRESSIVE ||
f->blocks > 1 || f->tf_change[i] < 0)) {
int foldstart, foldend;


effective_lowband = FFMAX(ff_celt_freq_bands[f->start_band],
ff_celt_freq_bands[lowband_offset] - ff_celt_freq_range[i]);
foldstart = lowband_offset;
while (ff_celt_freq_bands[--foldstart] > effective_lowband);
foldend = lowband_offset - 1;
while (++foldend < i && ff_celt_freq_bands[foldend] < effective_lowband + ff_celt_freq_range[i]);

cm[0] = cm[1] = 0;
for (j = foldstart; j < foldend; j++) {
cm[0] |= f->block[0].collapse_masks[j];
cm[1] |= f->block[f->channels - 1].collapse_masks[j];
}
}

if (f->dual_stereo && i == f->intensity_stereo) {

f->dual_stereo = 0;
for (j = ff_celt_freq_bands[f->start_band] << f->size; j < band_offset; j++)
norm1[j] = (norm1[j] + norm2[j]) / 2;
}

norm_loc1 = effective_lowband != -1 ? norm1 + (effective_lowband << f->size) : NULL;
norm_loc2 = effective_lowband != -1 ? norm2 + (effective_lowband << f->size) : NULL;

if (f->dual_stereo) {
cm[0] = f->pvq->quant_band(f->pvq, f, rc, i, X, NULL, band_size, b >> 1,
f->blocks, norm_loc1, f->size,
norm1 + band_offset, 0, 1.0f,
lowband_scratch, cm[0]);

cm[1] = f->pvq->quant_band(f->pvq, f, rc, i, Y, NULL, band_size, b >> 1,
f->blocks, norm_loc2, f->size,
norm2 + band_offset, 0, 1.0f,
lowband_scratch, cm[1]);
} else {
cm[0] = f->pvq->quant_band(f->pvq, f, rc, i, X, Y, band_size, b >> 0,
f->blocks, norm_loc1, f->size,
norm1 + band_offset, 0, 1.0f,
lowband_scratch, cm[0] | cm[1]);
cm[1] = cm[0];
}

f->block[0].collapse_masks[i] = (uint8_t)cm[0];
f->block[f->channels - 1].collapse_masks[i] = (uint8_t)cm[1];
f->remaining += f->pulses[i] + consumed;


update_lowband = (b > band_size << 3);
}
}

#define NORMC(bits) ((bits) << (f->channels - 1) << f->size >> 2)

void ff_celt_bitalloc(CeltFrame *f, OpusRangeCoder *rc, int encode)
{
int i, j, low, high, total, done, bandbits, remaining, tbits_8ths;
int skip_startband = f->start_band;
int skip_bit = 0;
int intensitystereo_bit = 0;
int dualstereo_bit = 0;
int dynalloc = 6;
int extrabits = 0;

int boost[CELT_MAX_BANDS] = { 0 };
int trim_offset[CELT_MAX_BANDS];
int threshold[CELT_MAX_BANDS];
int bits1[CELT_MAX_BANDS];
int bits2[CELT_MAX_BANDS];


if (opus_rc_tell(rc) + 4 <= f->framebits) {
if (encode)
ff_opus_rc_enc_cdf(rc, f->spread, ff_celt_model_spread);
else
f->spread = ff_opus_rc_dec_cdf(rc, ff_celt_model_spread);
} else {
f->spread = CELT_SPREAD_NORMAL;
}


for (i = 0; i < CELT_MAX_BANDS; i++)
f->caps[i] = NORMC((ff_celt_static_caps[f->size][f->channels - 1][i] + 64) * ff_celt_freq_range[i]);


tbits_8ths = f->framebits << 3;
for (i = f->start_band; i < f->end_band; i++) {
int quanta = ff_celt_freq_range[i] << (f->channels - 1) << f->size;
int b_dynalloc = dynalloc;
int boost_amount = f->alloc_boost[i];
quanta = FFMIN(quanta << 3, FFMAX(6 << 3, quanta));

while (opus_rc_tell_frac(rc) + (b_dynalloc << 3) < tbits_8ths && boost[i] < f->caps[i]) {
int is_boost;
if (encode) {
is_boost = boost_amount--;
ff_opus_rc_enc_log(rc, is_boost, b_dynalloc);
} else {
is_boost = ff_opus_rc_dec_log(rc, b_dynalloc);
}

if (!is_boost)
break;

boost[i] += quanta;
tbits_8ths -= quanta;

b_dynalloc = 1;
}

if (boost[i])
dynalloc = FFMAX(dynalloc - 1, 2);
}


if (opus_rc_tell_frac(rc) + (6 << 3) <= tbits_8ths)
if (encode)
ff_opus_rc_enc_cdf(rc, f->alloc_trim, ff_celt_model_alloc_trim);
else
f->alloc_trim = ff_opus_rc_dec_cdf(rc, ff_celt_model_alloc_trim);


tbits_8ths = (f->framebits << 3) - opus_rc_tell_frac(rc) - 1;
f->anticollapse_needed = 0;
if (f->transient && f->size >= 2 && tbits_8ths >= ((f->size + 2) << 3))
f->anticollapse_needed = 1 << 3;
tbits_8ths -= f->anticollapse_needed;


if (tbits_8ths >= 1 << 3)
skip_bit = 1 << 3;
tbits_8ths -= skip_bit;


if (f->channels == 2) {
intensitystereo_bit = ff_celt_log2_frac[f->end_band - f->start_band];
if (intensitystereo_bit <= tbits_8ths) {
tbits_8ths -= intensitystereo_bit;
if (tbits_8ths >= 1 << 3) {
dualstereo_bit = 1 << 3;
tbits_8ths -= 1 << 3;
}
} else {
intensitystereo_bit = 0;
}
}


for (i = f->start_band; i < f->end_band; i++) {
int trim = f->alloc_trim - 5 - f->size;
int band = ff_celt_freq_range[i] * (f->end_band - i - 1);
int duration = f->size + 3;
int scale = duration + f->channels - 1;



threshold[i] = FFMAX(3 * ff_celt_freq_range[i] << duration >> 4,
f->channels << 3);

trim_offset[i] = trim * (band << scale) >> 6;

if (ff_celt_freq_range[i] << f->size == 1)
trim_offset[i] -= f->channels << 3;
}


low = 1;
high = CELT_VECTORS - 1;
while (low <= high) {
int center = (low + high) >> 1;
done = total = 0;

for (i = f->end_band - 1; i >= f->start_band; i--) {
bandbits = NORMC(ff_celt_freq_range[i] * ff_celt_static_alloc[center][i]);

if (bandbits)
bandbits = FFMAX(bandbits + trim_offset[i], 0);
bandbits += boost[i];

if (bandbits >= threshold[i] || done) {
done = 1;
total += FFMIN(bandbits, f->caps[i]);
} else if (bandbits >= f->channels << 3) {
total += f->channels << 3;
}
}

if (total > tbits_8ths)
high = center - 1;
else
low = center + 1;
}
high = low--;


for (i = f->start_band; i < f->end_band; i++) {
bits1[i] = NORMC(ff_celt_freq_range[i] * ff_celt_static_alloc[low][i]);
bits2[i] = high >= CELT_VECTORS ? f->caps[i] :
NORMC(ff_celt_freq_range[i] * ff_celt_static_alloc[high][i]);

if (bits1[i])
bits1[i] = FFMAX(bits1[i] + trim_offset[i], 0);
if (bits2[i])
bits2[i] = FFMAX(bits2[i] + trim_offset[i], 0);

if (low)
bits1[i] += boost[i];
bits2[i] += boost[i];

if (boost[i])
skip_startband = i;
bits2[i] = FFMAX(bits2[i] - bits1[i], 0);
}


low = 0;
high = 1 << CELT_ALLOC_STEPS;
for (i = 0; i < CELT_ALLOC_STEPS; i++) {
int center = (low + high) >> 1;
done = total = 0;

for (j = f->end_band - 1; j >= f->start_band; j--) {
bandbits = bits1[j] + (center * bits2[j] >> CELT_ALLOC_STEPS);

if (bandbits >= threshold[j] || done) {
done = 1;
total += FFMIN(bandbits, f->caps[j]);
} else if (bandbits >= f->channels << 3)
total += f->channels << 3;
}
if (total > tbits_8ths)
high = center;
else
low = center;
}


done = total = 0;
for (i = f->end_band - 1; i >= f->start_band; i--) {
bandbits = bits1[i] + (low * bits2[i] >> CELT_ALLOC_STEPS);

if (bandbits >= threshold[i] || done)
done = 1;
else
bandbits = (bandbits >= f->channels << 3) ?
f->channels << 3 : 0;

bandbits = FFMIN(bandbits, f->caps[i]);
f->pulses[i] = bandbits;
total += bandbits;
}


for (f->coded_bands = f->end_band; ; f->coded_bands--) {
int allocation;
j = f->coded_bands - 1;

if (j == skip_startband) {

tbits_8ths += skip_bit;
break;
}


remaining = tbits_8ths - total;
bandbits = remaining / (ff_celt_freq_bands[j+1] - ff_celt_freq_bands[f->start_band]);
remaining -= bandbits * (ff_celt_freq_bands[j+1] - ff_celt_freq_bands[f->start_band]);
allocation = f->pulses[j] + bandbits * ff_celt_freq_range[j];
allocation += FFMAX(remaining - (ff_celt_freq_bands[j] - ff_celt_freq_bands[f->start_band]), 0);



if (allocation >= FFMAX(threshold[j], (f->channels + 1) << 3)) {
int do_not_skip;
if (encode) {
do_not_skip = f->coded_bands <= f->skip_band_floor;
ff_opus_rc_enc_log(rc, do_not_skip, 1);
} else {
do_not_skip = ff_opus_rc_dec_log(rc, 1);
}

if (do_not_skip)
break;

total += 1 << 3;
allocation -= 1 << 3;
}


total -= f->pulses[j];
if (intensitystereo_bit) {
total -= intensitystereo_bit;
intensitystereo_bit = ff_celt_log2_frac[j - f->start_band];
total += intensitystereo_bit;
}

total += f->pulses[j] = (allocation >= f->channels << 3) ? f->channels << 3 : 0;
}


if (encode) {
if (intensitystereo_bit) {
f->intensity_stereo = FFMIN(f->intensity_stereo, f->coded_bands);
ff_opus_rc_enc_uint(rc, f->intensity_stereo, f->coded_bands + 1 - f->start_band);
}
} else {
f->intensity_stereo = f->dual_stereo = 0;
if (intensitystereo_bit)
f->intensity_stereo = f->start_band + ff_opus_rc_dec_uint(rc, f->coded_bands + 1 - f->start_band);
}


if (f->intensity_stereo <= f->start_band)
tbits_8ths += dualstereo_bit; 
else if (dualstereo_bit)
if (encode)
ff_opus_rc_enc_log(rc, f->dual_stereo, 1);
else
f->dual_stereo = ff_opus_rc_dec_log(rc, 1);


remaining = tbits_8ths - total;
bandbits = remaining / (ff_celt_freq_bands[f->coded_bands] - ff_celt_freq_bands[f->start_band]);
remaining -= bandbits * (ff_celt_freq_bands[f->coded_bands] - ff_celt_freq_bands[f->start_band]);
for (i = f->start_band; i < f->coded_bands; i++) {
const int bits = FFMIN(remaining, ff_celt_freq_range[i]);
f->pulses[i] += bits + bandbits * ff_celt_freq_range[i];
remaining -= bits;
}


for (i = f->start_band; i < f->coded_bands; i++) {
int N = ff_celt_freq_range[i] << f->size;
int prev_extra = extrabits;
f->pulses[i] += extrabits;

if (N > 1) {
int dof; 
int temp; 
int fine_bits;
int max_bits;
int offset; 



extrabits = FFMAX(f->pulses[i] - f->caps[i], 0);
f->pulses[i] -= extrabits;


dof = N * f->channels + (f->channels == 2 && N > 2 && !f->dual_stereo && i < f->intensity_stereo);
temp = dof * (ff_celt_log_freq_range[i] + (f->size << 3));
offset = (temp >> 1) - dof * CELT_FINE_OFFSET;
if (N == 2) 
offset += dof << 1;


if (f->pulses[i] + offset < 2 * (dof << 3))
offset += temp >> 2;
else if (f->pulses[i] + offset < 3 * (dof << 3))
offset += temp >> 3;

fine_bits = (f->pulses[i] + offset + (dof << 2)) / (dof << 3);
max_bits = FFMIN((f->pulses[i] >> 3) >> (f->channels - 1), CELT_MAX_FINE_BITS);
max_bits = FFMAX(max_bits, 0);
f->fine_bits[i] = av_clip(fine_bits, 0, max_bits);



f->fine_priority[i] = (f->fine_bits[i] * (dof << 3) >= f->pulses[i] + offset);


f->pulses[i] -= f->fine_bits[i] << (f->channels - 1) << 3;
} else {

extrabits = FFMAX(f->pulses[i] - (f->channels << 3), 0);
f->pulses[i] -= extrabits;
f->fine_bits[i] = 0;
f->fine_priority[i] = 1;
}


if (extrabits > 0) {
int fineextra = FFMIN(extrabits >> (f->channels + 2),
CELT_MAX_FINE_BITS - f->fine_bits[i]);
f->fine_bits[i] += fineextra;

fineextra <<= f->channels + 2;
f->fine_priority[i] = (fineextra >= extrabits - prev_extra);
extrabits -= fineextra;
}
}
f->remaining = extrabits;


for (; i < f->end_band; i++) {
f->fine_bits[i] = f->pulses[i] >> (f->channels - 1) >> 3;
f->pulses[i] = 0;
f->fine_priority[i] = f->fine_bits[i] < 1;
}
}
