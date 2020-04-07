#include "libavutil/intreadwrite.h"
#include "libavutil/log.h"
#include "avcodec.h"
#include "internal.h"
#define SIN_BITS 14
#define WS_MAX_CHANNELS 32
#define INF_TS 0x7FFFFFFFFFFFFFFF
#define PINK_UNIT 128
enum ws_interval_type {
WS_SINE = MKTAG('S','I','N','E'),
WS_NOISE = MKTAG('N','O','I','S'),
};
struct ws_interval {
int64_t ts_start, ts_end;
uint64_t phi0, dphi0, ddphi;
uint64_t amp0, damp;
uint64_t phi, dphi, amp;
uint32_t channels;
enum ws_interval_type type;
int next;
};
struct wavesynth_context {
int64_t cur_ts;
int64_t next_ts;
int32_t *sin;
struct ws_interval *inter;
uint32_t dither_state;
uint32_t pink_state;
int32_t pink_pool[PINK_UNIT];
unsigned pink_need, pink_pos;
int nb_inter;
int cur_inter;
int next_inter;
};
#define LCG_A 1284865837
#define LCG_C 4150755663
#define LCG_AI 849225893 
static uint32_t lcg_next(uint32_t *s)
{
*s = *s * LCG_A + LCG_C;
return *s;
}
static void lcg_seek(uint32_t *s, uint32_t dt)
{
uint32_t a, c, t = *s;
a = LCG_A;
c = LCG_C;
while (dt) {
if (dt & 1)
t = a * t + c;
c *= a + 1; 
a *= a;
dt >>= 1;
}
*s = t;
}
static void pink_fill(struct wavesynth_context *ws)
{
int32_t vt[7] = { 0 }, v = 0;
int i, j;
ws->pink_pos = 0;
if (!ws->pink_need)
return;
for (i = 0; i < PINK_UNIT; i++) {
for (j = 0; j < 7; j++) {
if ((i >> j) & 1)
break;
v -= vt[j];
vt[j] = (int32_t)lcg_next(&ws->pink_state) >> 3;
v += vt[j];
}
ws->pink_pool[i] = v + ((int32_t)lcg_next(&ws->pink_state) >> 3);
}
lcg_next(&ws->pink_state); 
}
static uint64_t frac64(uint64_t a, uint64_t b)
{
uint64_t r = 0;
int i;
if (b < (uint64_t)1 << 32) { 
a <<= 32;
return ((a / b) << 32) | ((a % b) << 32) / b;
}
if (b < (uint64_t)1 << 48) { 
for (i = 0; i < 4; i++) {
a <<= 16;
r = (r << 16) | (a / b);
a %= b;
}
return r;
}
for (i = 63; i >= 0; i--) {
if (a >= (uint64_t)1 << 63 || a << 1 >= b) {
r |= (uint64_t)1 << i;
a = (a << 1) - b;
} else {
a <<= 1;
}
}
return r;
}
static uint64_t phi_at(struct ws_interval *in, int64_t ts)
{
uint64_t dt = ts - in->ts_start;
uint64_t dt2 = dt & 1 ? 
dt * ((dt - 1) >> 1) : (dt >> 1) * (dt - 1);
return in->phi0 + dt * in->dphi0 + dt2 * in->ddphi;
}
static void wavesynth_seek(struct wavesynth_context *ws, int64_t ts)
{
int *last, i;
struct ws_interval *in;
last = &ws->cur_inter;
for (i = 0; i < ws->nb_inter; i++) {
in = &ws->inter[i];
if (ts < in->ts_start)
break;
if (ts >= in->ts_end)
continue;
*last = i;
last = &in->next;
in->phi = phi_at(in, ts);
in->dphi = in->dphi0 + (ts - in->ts_start) * in->ddphi;
in->amp = in->amp0 + (ts - in->ts_start) * in->damp;
}
ws->next_inter = i;
ws->next_ts = i < ws->nb_inter ? ws->inter[i].ts_start : INF_TS;
*last = -1;
lcg_seek(&ws->dither_state, (uint32_t)ts - (uint32_t)ws->cur_ts);
if (ws->pink_need) {
uint64_t pink_ts_cur = (ws->cur_ts + PINK_UNIT - 1) & ~(PINK_UNIT - 1);
uint64_t pink_ts_next = ts & ~(PINK_UNIT - 1);
int pos = ts & (PINK_UNIT - 1);
lcg_seek(&ws->pink_state, (uint32_t)(pink_ts_next - pink_ts_cur) * 2);
if (pos) {
pink_fill(ws);
ws->pink_pos = pos;
} else {
ws->pink_pos = PINK_UNIT;
}
}
ws->cur_ts = ts;
}
static int wavesynth_parse_extradata(AVCodecContext *avc)
{
struct wavesynth_context *ws = avc->priv_data;
struct ws_interval *in;
uint8_t *edata, *edata_end;
int32_t f1, f2, a1, a2;
uint32_t phi;
int64_t dphi1, dphi2, dt, cur_ts = -0x8000000000000000;
int i;
if (avc->extradata_size < 4)
return AVERROR(EINVAL);
edata = avc->extradata;
edata_end = edata + avc->extradata_size;
ws->nb_inter = AV_RL32(edata);
edata += 4;
if (ws->nb_inter < 0 || (edata_end - edata) / 24 < ws->nb_inter)
return AVERROR(EINVAL);
ws->inter = av_calloc(ws->nb_inter, sizeof(*ws->inter));
if (!ws->inter)
return AVERROR(ENOMEM);
for (i = 0; i < ws->nb_inter; i++) {
in = &ws->inter[i];
if (edata_end - edata < 24)
return AVERROR(EINVAL);
in->ts_start = AV_RL64(edata + 0);
in->ts_end = AV_RL64(edata + 8);
in->type = AV_RL32(edata + 16);
in->channels = AV_RL32(edata + 20);
edata += 24;
if (in->ts_start < cur_ts ||
in->ts_end <= in->ts_start ||
(uint64_t)in->ts_end - in->ts_start > INT64_MAX
)
return AVERROR(EINVAL);
cur_ts = in->ts_start;
dt = in->ts_end - in->ts_start;
switch (in->type) {
case WS_SINE:
if (edata_end - edata < 20 || avc->sample_rate <= 0)
return AVERROR(EINVAL);
f1 = AV_RL32(edata + 0);
f2 = AV_RL32(edata + 4);
a1 = AV_RL32(edata + 8);
a2 = AV_RL32(edata + 12);
phi = AV_RL32(edata + 16);
edata += 20;
dphi1 = frac64(f1, (int64_t)avc->sample_rate << 16);
dphi2 = frac64(f2, (int64_t)avc->sample_rate << 16);
in->dphi0 = dphi1;
in->ddphi = (int64_t)(dphi2 - (uint64_t)dphi1) / dt;
if (phi & 0x80000000) {
phi &= ~0x80000000;
if (phi >= i)
return AVERROR(EINVAL);
in->phi0 = phi_at(&ws->inter[phi], in->ts_start);
} else {
in->phi0 = (uint64_t)phi << 33;
}
break;
case WS_NOISE:
if (edata_end - edata < 8)
return AVERROR(EINVAL);
a1 = AV_RL32(edata + 0);
a2 = AV_RL32(edata + 4);
edata += 8;
break;
default:
return AVERROR(EINVAL);
}
in->amp0 = (uint64_t)a1 << 32;
in->damp = (int64_t)(((uint64_t)a2 << 32) - ((uint64_t)a1 << 32)) / dt;
}
if (edata != edata_end)
return AVERROR(EINVAL);
return 0;
}
static av_cold int wavesynth_init(AVCodecContext *avc)
{
struct wavesynth_context *ws = avc->priv_data;
int i, r;
if (avc->channels > WS_MAX_CHANNELS) {
av_log(avc, AV_LOG_ERROR,
"This implementation is limited to %d channels.\n",
WS_MAX_CHANNELS);
return AVERROR(EINVAL);
}
r = wavesynth_parse_extradata(avc);
if (r < 0) {
av_log(avc, AV_LOG_ERROR, "Invalid intervals definitions.\n");
goto fail;
}
ws->sin = av_malloc(sizeof(*ws->sin) << SIN_BITS);
if (!ws->sin) {
r = AVERROR(ENOMEM);
goto fail;
}
for (i = 0; i < 1 << SIN_BITS; i++)
ws->sin[i] = floor(32767 * sin(2 * M_PI * i / (1 << SIN_BITS)));
ws->dither_state = MKTAG('D','I','T','H');
for (i = 0; i < ws->nb_inter; i++)
ws->pink_need += ws->inter[i].type == WS_NOISE;
ws->pink_state = MKTAG('P','I','N','K');
ws->pink_pos = PINK_UNIT;
wavesynth_seek(ws, 0);
avc->sample_fmt = AV_SAMPLE_FMT_S16;
return 0;
fail:
av_freep(&ws->inter);
av_freep(&ws->sin);
return r;
}
static void wavesynth_synth_sample(struct wavesynth_context *ws, int64_t ts,
int32_t *channels)
{
int32_t amp, *cv;
unsigned val;
struct ws_interval *in;
int i, *last, pink;
uint32_t c, all_ch = 0;
i = ws->cur_inter;
last = &ws->cur_inter;
if (ws->pink_pos == PINK_UNIT)
pink_fill(ws);
pink = ws->pink_pool[ws->pink_pos++] >> 16;
while (i >= 0) {
in = &ws->inter[i];
i = in->next;
if (ts >= in->ts_end) {
*last = i;
continue;
}
last = &in->next;
amp = in->amp >> 32;
in->amp += in->damp;
switch (in->type) {
case WS_SINE:
val = amp * ws->sin[in->phi >> (64 - SIN_BITS)];
in->phi += in->dphi;
in->dphi += in->ddphi;
break;
case WS_NOISE:
val = amp * (unsigned)pink;
break;
default:
val = 0;
}
all_ch |= in->channels;
for (c = in->channels, cv = channels; c; c >>= 1, cv++)
if (c & 1)
*cv += (unsigned)val;
}
val = (int32_t)lcg_next(&ws->dither_state) >> 16;
for (c = all_ch, cv = channels; c; c >>= 1, cv++)
if (c & 1)
*cv += val;
}
static void wavesynth_enter_intervals(struct wavesynth_context *ws, int64_t ts)
{
int *last, i;
struct ws_interval *in;
last = &ws->cur_inter;
for (i = ws->cur_inter; i >= 0; i = ws->inter[i].next)
last = &ws->inter[i].next;
for (i = ws->next_inter; i < ws->nb_inter; i++) {
in = &ws->inter[i];
if (ts < in->ts_start)
break;
if (ts >= in->ts_end)
continue;
*last = i;
last = &in->next;
in->phi = in->phi0;
in->dphi = in->dphi0;
in->amp = in->amp0;
}
ws->next_inter = i;
ws->next_ts = i < ws->nb_inter ? ws->inter[i].ts_start : INF_TS;
*last = -1;
}
static int wavesynth_decode(AVCodecContext *avc, void *rframe, int *rgot_frame,
AVPacket *packet)
{
struct wavesynth_context *ws = avc->priv_data;
AVFrame *frame = rframe;
int64_t ts;
int duration;
int s, c, r;
int16_t *pcm;
int32_t channels[WS_MAX_CHANNELS];
*rgot_frame = 0;
if (packet->size != 12)
return AVERROR_INVALIDDATA;
ts = AV_RL64(packet->data);
if (ts != ws->cur_ts)
wavesynth_seek(ws, ts);
duration = AV_RL32(packet->data + 8);
if (duration <= 0)
return AVERROR(EINVAL);
frame->nb_samples = duration;
r = ff_get_buffer(avc, frame, 0);
if (r < 0)
return r;
pcm = (int16_t *)frame->data[0];
for (s = 0; s < duration; s++, ts++) {
memset(channels, 0, avc->channels * sizeof(*channels));
if (ts >= ws->next_ts)
wavesynth_enter_intervals(ws, ts);
wavesynth_synth_sample(ws, ts, channels);
for (c = 0; c < avc->channels; c++)
*(pcm++) = channels[c] >> 16;
}
ws->cur_ts += duration;
*rgot_frame = 1;
return packet->size;
}
static av_cold int wavesynth_close(AVCodecContext *avc)
{
struct wavesynth_context *ws = avc->priv_data;
av_freep(&ws->sin);
av_freep(&ws->inter);
return 0;
}
AVCodec ff_ffwavesynth_decoder = {
.name = "wavesynth",
.long_name = NULL_IF_CONFIG_SMALL("Wave synthesis pseudo-codec"),
.type = AVMEDIA_TYPE_AUDIO,
.id = AV_CODEC_ID_FFWAVESYNTH,
.priv_data_size = sizeof(struct wavesynth_context),
.init = wavesynth_init,
.close = wavesynth_close,
.decode = wavesynth_decode,
.capabilities = AV_CODEC_CAP_DR1,
};
