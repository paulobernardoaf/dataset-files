#include "libavutil/intreadwrite.h"
#include "avcodec.h"
#include "get_bits.h"
#include "internal.h"
typedef struct HEntry {
int16_t l, r;
} HEntry;
typedef struct HCOMContext {
AVCodecContext *avctx;
uint8_t first_sample;
uint8_t sample;
int dict_entries;
int dict_entry;
int delta_compression;
HEntry *dict;
} HCOMContext;
static av_cold int hcom_init(AVCodecContext *avctx)
{
HCOMContext *s = avctx->priv_data;
if (avctx->channels != 1) {
av_log(avctx, AV_LOG_ERROR, "invalid number of channels\n");
return AVERROR_INVALIDDATA;
}
if (avctx->extradata_size <= 7)
return AVERROR_INVALIDDATA;
s->dict_entries = AV_RB16(avctx->extradata);
if (avctx->extradata_size < s->dict_entries * 4 + 7 ||
s->dict_entries == 0)
return AVERROR_INVALIDDATA;
s->delta_compression = AV_RB32(avctx->extradata + 2);
s->sample = s->first_sample = avctx->extradata[avctx->extradata_size - 1];
s->dict = av_calloc(s->dict_entries, sizeof(*s->dict));
if (!s->dict)
return AVERROR(ENOMEM);
for (int i = 0; i < s->dict_entries; i++) {
s->dict[i].l = AV_RB16(avctx->extradata + 6 + 4 * i);
s->dict[i].r = AV_RB16(avctx->extradata + 6 + 4 * i + 2);
if (s->dict[i].l >= 0 &&
(s->dict[i].l >= s->dict_entries ||
s->dict[i].r >= s->dict_entries ||
s->dict[i].r < 0 )) {
av_freep(&s->dict);
return AVERROR_INVALIDDATA;
}
}
if (s->dict[0].l < 0) {
av_freep(&s->dict);
return AVERROR_INVALIDDATA;
}
avctx->sample_fmt = AV_SAMPLE_FMT_U8;
s->dict_entry = 0;
return 0;
}
static int hcom_decode(AVCodecContext *avctx, void *data,
int *got_frame, AVPacket *pkt)
{
HCOMContext *s = avctx->priv_data;
AVFrame *frame = data;
GetBitContext gb;
int ret, n = 0;
if (pkt->size > INT16_MAX)
return AVERROR_INVALIDDATA;
frame->nb_samples = pkt->size * 8;
if ((ret = ff_get_buffer(avctx, frame, 0)) < 0)
return ret;
if ((ret = init_get_bits8(&gb, pkt->data, pkt->size)) < 0)
return ret;
while (get_bits_left(&gb) > 0) {
if (get_bits1(&gb))
s->dict_entry = s->dict[s->dict_entry].r;
else
s->dict_entry = s->dict[s->dict_entry].l;
if (s->dict[s->dict_entry].l < 0) {
int16_t datum;
datum = s->dict[s->dict_entry].r;
if (!s->delta_compression)
s->sample = 0;
s->sample = (s->sample + datum) & 0xFF;
frame->data[0][n++] = s->sample;
s->dict_entry = 0;
}
}
frame->nb_samples = n;
*got_frame = 1;
return pkt->size;
}
static av_cold int hcom_close(AVCodecContext *avctx)
{
HCOMContext *s = avctx->priv_data;
av_freep(&s->dict);
return 0;
}
AVCodec ff_hcom_decoder = {
.name = "hcom",
.long_name = NULL_IF_CONFIG_SMALL("HCOM Audio"),
.type = AVMEDIA_TYPE_AUDIO,
.id = AV_CODEC_ID_HCOM,
.priv_data_size = sizeof(HCOMContext),
.init = hcom_init,
.close = hcom_close,
.decode = hcom_decode,
.capabilities = AV_CODEC_CAP_DR1,
};
