





























#include "libavutil/channel_layout.h"
#include "libavutil/intfloat.h"

#define BITSTREAM_READER_LE
#include "avcodec.h"
#include "dct.h"
#include "decode.h"
#include "get_bits.h"
#include "internal.h"
#include "rdft.h"
#include "wma_freqs.h"

static float quant_table[96];

#define MAX_CHANNELS 2
#define BINK_BLOCK_MAX_SIZE (MAX_CHANNELS << 11)

typedef struct BinkAudioContext {
GetBitContext gb;
int version_b; 
int first;
int channels;
int frame_len; 
int overlap_len; 
int block_size;
int num_bands;
unsigned int *bands;
float root;
DECLARE_ALIGNED(32, FFTSample, coeffs)[BINK_BLOCK_MAX_SIZE];
float previous[MAX_CHANNELS][BINK_BLOCK_MAX_SIZE / 16]; 
AVPacket *pkt;
union {
RDFTContext rdft;
DCTContext dct;
} trans;
} BinkAudioContext;


static av_cold int decode_init(AVCodecContext *avctx)
{
BinkAudioContext *s = avctx->priv_data;
int sample_rate = avctx->sample_rate;
int sample_rate_half;
int i;
int frame_len_bits;


if (avctx->sample_rate < 22050) {
frame_len_bits = 9;
} else if (avctx->sample_rate < 44100) {
frame_len_bits = 10;
} else {
frame_len_bits = 11;
}

if (avctx->channels < 1 || avctx->channels > MAX_CHANNELS) {
av_log(avctx, AV_LOG_ERROR, "invalid number of channels: %d\n", avctx->channels);
return AVERROR_INVALIDDATA;
}
avctx->channel_layout = avctx->channels == 1 ? AV_CH_LAYOUT_MONO :
AV_CH_LAYOUT_STEREO;

s->version_b = avctx->extradata_size >= 4 && avctx->extradata[3] == 'b';

if (avctx->codec->id == AV_CODEC_ID_BINKAUDIO_RDFT) {

avctx->sample_fmt = AV_SAMPLE_FMT_FLT;
if (sample_rate > INT_MAX / avctx->channels)
return AVERROR_INVALIDDATA;
sample_rate *= avctx->channels;
s->channels = 1;
if (!s->version_b)
frame_len_bits += av_log2(avctx->channels);
} else {
s->channels = avctx->channels;
avctx->sample_fmt = AV_SAMPLE_FMT_FLTP;
}

s->frame_len = 1 << frame_len_bits;
s->overlap_len = s->frame_len / 16;
s->block_size = (s->frame_len - s->overlap_len) * s->channels;
sample_rate_half = (sample_rate + 1) / 2;
if (avctx->codec->id == AV_CODEC_ID_BINKAUDIO_RDFT)
s->root = 2.0 / (sqrt(s->frame_len) * 32768.0);
else
s->root = s->frame_len / (sqrt(s->frame_len) * 32768.0);
for (i = 0; i < 96; i++) {

quant_table[i] = expf(i * 0.15289164787221953823f) * s->root;
}


for (s->num_bands = 1; s->num_bands < 25; s->num_bands++)
if (sample_rate_half <= ff_wma_critical_freqs[s->num_bands - 1])
break;

s->bands = av_malloc((s->num_bands + 1) * sizeof(*s->bands));
if (!s->bands)
return AVERROR(ENOMEM);


s->bands[0] = 2;
for (i = 1; i < s->num_bands; i++)
s->bands[i] = (ff_wma_critical_freqs[i - 1] * s->frame_len / sample_rate_half) & ~1;
s->bands[s->num_bands] = s->frame_len;

s->first = 1;

if (CONFIG_BINKAUDIO_RDFT_DECODER && avctx->codec->id == AV_CODEC_ID_BINKAUDIO_RDFT)
ff_rdft_init(&s->trans.rdft, frame_len_bits, DFT_C2R);
else if (CONFIG_BINKAUDIO_DCT_DECODER)
ff_dct_init(&s->trans.dct, frame_len_bits, DCT_III);
else
av_assert0(0);

s->pkt = av_packet_alloc();
if (!s->pkt)
return AVERROR(ENOMEM);

return 0;
}

static float get_float(GetBitContext *gb)
{
int power = get_bits(gb, 5);
float f = ldexpf(get_bits(gb, 23), power - 23);
if (get_bits1(gb))
f = -f;
return f;
}

static const uint8_t rle_length_tab[16] = {
2, 3, 4, 5, 6, 8, 9, 10, 11, 12, 13, 14, 15, 16, 32, 64
};






static int decode_block(BinkAudioContext *s, float **out, int use_dct)
{
int ch, i, j, k;
float q, quant[25];
int width, coeff;
GetBitContext *gb = &s->gb;

if (use_dct)
skip_bits(gb, 2);

for (ch = 0; ch < s->channels; ch++) {
FFTSample *coeffs = out[ch];

if (s->version_b) {
if (get_bits_left(gb) < 64)
return AVERROR_INVALIDDATA;
coeffs[0] = av_int2float(get_bits_long(gb, 32)) * s->root;
coeffs[1] = av_int2float(get_bits_long(gb, 32)) * s->root;
} else {
if (get_bits_left(gb) < 58)
return AVERROR_INVALIDDATA;
coeffs[0] = get_float(gb) * s->root;
coeffs[1] = get_float(gb) * s->root;
}

if (get_bits_left(gb) < s->num_bands * 8)
return AVERROR_INVALIDDATA;
for (i = 0; i < s->num_bands; i++) {
int value = get_bits(gb, 8);
quant[i] = quant_table[FFMIN(value, 95)];
}

k = 0;
q = quant[0];


i = 2;
while (i < s->frame_len) {
if (s->version_b) {
j = i + 16;
} else {
int v = get_bits1(gb);
if (v) {
v = get_bits(gb, 4);
j = i + rle_length_tab[v] * 8;
} else {
j = i + 8;
}
}

j = FFMIN(j, s->frame_len);

width = get_bits(gb, 4);
if (width == 0) {
memset(coeffs + i, 0, (j - i) * sizeof(*coeffs));
i = j;
while (s->bands[k] < i)
q = quant[k++];
} else {
while (i < j) {
if (s->bands[k] == i)
q = quant[k++];
coeff = get_bits(gb, width);
if (coeff) {
int v;
v = get_bits1(gb);
if (v)
coeffs[i] = -q * coeff;
else
coeffs[i] = q * coeff;
} else {
coeffs[i] = 0.0f;
}
i++;
}
}
}

if (CONFIG_BINKAUDIO_DCT_DECODER && use_dct) {
coeffs[0] /= 0.5;
s->trans.dct.dct_calc(&s->trans.dct, coeffs);
}
else if (CONFIG_BINKAUDIO_RDFT_DECODER)
s->trans.rdft.rdft_calc(&s->trans.rdft, coeffs);
}

for (ch = 0; ch < s->channels; ch++) {
int j;
int count = s->overlap_len * s->channels;
if (!s->first) {
j = ch;
for (i = 0; i < s->overlap_len; i++, j += s->channels)
out[ch][i] = (s->previous[ch][i] * (count - j) +
out[ch][i] * j) / count;
}
memcpy(s->previous[ch], &out[ch][s->frame_len - s->overlap_len],
s->overlap_len * sizeof(*s->previous[ch]));
}

s->first = 0;

return 0;
}

static av_cold int decode_end(AVCodecContext *avctx)
{
BinkAudioContext * s = avctx->priv_data;
av_freep(&s->bands);
if (CONFIG_BINKAUDIO_RDFT_DECODER && avctx->codec->id == AV_CODEC_ID_BINKAUDIO_RDFT)
ff_rdft_end(&s->trans.rdft);
else if (CONFIG_BINKAUDIO_DCT_DECODER)
ff_dct_end(&s->trans.dct);

av_packet_free(&s->pkt);

return 0;
}

static void get_bits_align32(GetBitContext *s)
{
int n = (-get_bits_count(s)) & 31;
if (n) skip_bits(s, n);
}

static int binkaudio_receive_frame(AVCodecContext *avctx, AVFrame *frame)
{
BinkAudioContext *s = avctx->priv_data;
GetBitContext *gb = &s->gb;
int ret;

if (!s->pkt->data) {
ret = ff_decode_get_packet(avctx, s->pkt);
if (ret < 0)
return ret;

if (s->pkt->size < 4) {
av_log(avctx, AV_LOG_ERROR, "Packet is too small\n");
ret = AVERROR_INVALIDDATA;
goto fail;
}

ret = init_get_bits8(gb, s->pkt->data, s->pkt->size);
if (ret < 0)
goto fail;


skip_bits_long(gb, 32);
}


frame->nb_samples = s->frame_len;
if ((ret = ff_get_buffer(avctx, frame, 0)) < 0)
return ret;

if (decode_block(s, (float **)frame->extended_data,
avctx->codec->id == AV_CODEC_ID_BINKAUDIO_DCT)) {
av_log(avctx, AV_LOG_ERROR, "Incomplete packet\n");
return AVERROR_INVALIDDATA;
}
get_bits_align32(gb);
if (!get_bits_left(gb)) {
memset(gb, 0, sizeof(*gb));
av_packet_unref(s->pkt);
}

frame->nb_samples = s->block_size / avctx->channels;

return 0;
fail:
av_packet_unref(s->pkt);
return ret;
}

AVCodec ff_binkaudio_rdft_decoder = {
.name = "binkaudio_rdft",
.long_name = NULL_IF_CONFIG_SMALL("Bink Audio (RDFT)"),
.type = AVMEDIA_TYPE_AUDIO,
.id = AV_CODEC_ID_BINKAUDIO_RDFT,
.priv_data_size = sizeof(BinkAudioContext),
.init = decode_init,
.close = decode_end,
.receive_frame = binkaudio_receive_frame,
.capabilities = AV_CODEC_CAP_DELAY | AV_CODEC_CAP_DR1,
};

AVCodec ff_binkaudio_dct_decoder = {
.name = "binkaudio_dct",
.long_name = NULL_IF_CONFIG_SMALL("Bink Audio (DCT)"),
.type = AVMEDIA_TYPE_AUDIO,
.id = AV_CODEC_ID_BINKAUDIO_DCT,
.priv_data_size = sizeof(BinkAudioContext),
.init = decode_init,
.close = decode_end,
.receive_frame = binkaudio_receive_frame,
.capabilities = AV_CODEC_CAP_DELAY | AV_CODEC_CAP_DR1,
};
