




















#include "adts_header.h"
#include "adts_parser.h"
#include "avcodec.h"
#include "bsf.h"
#include "put_bits.h"
#include "get_bits.h"
#include "mpeg4audio.h"
#include "internal.h"

typedef struct AACBSFContext {
int first_frame_done;
} AACBSFContext;





static int aac_adtstoasc_filter(AVBSFContext *bsfc, AVPacket *pkt)
{
AACBSFContext *ctx = bsfc->priv_data;

GetBitContext gb;
PutBitContext pb;
AACADTSHeaderInfo hdr;
int ret;

ret = ff_bsf_get_packet_ref(bsfc, pkt);
if (ret < 0)
return ret;

if (bsfc->par_in->extradata && pkt->size >= 2 && (AV_RB16(pkt->data) >> 4) != 0xfff)
return 0;

if (pkt->size < AV_AAC_ADTS_HEADER_SIZE)
goto packet_too_small;

init_get_bits(&gb, pkt->data, AV_AAC_ADTS_HEADER_SIZE * 8);

if (ff_adts_header_parse(&gb, &hdr) < 0) {
av_log(bsfc, AV_LOG_ERROR, "Error parsing ADTS frame header!\n");
ret = AVERROR_INVALIDDATA;
goto fail;
}

if (!hdr.crc_absent && hdr.num_aac_frames > 1) {
avpriv_report_missing_feature(bsfc,
"Multiple RDBs per frame with CRC");
ret = AVERROR_PATCHWELCOME;
goto fail;
}

pkt->size -= AV_AAC_ADTS_HEADER_SIZE + 2 * !hdr.crc_absent;
if (pkt->size <= 0)
goto packet_too_small;
pkt->data += AV_AAC_ADTS_HEADER_SIZE + 2 * !hdr.crc_absent;

if (!ctx->first_frame_done) {
int pce_size = 0;
uint8_t pce_data[MAX_PCE_SIZE];
uint8_t *extradata;

if (!hdr.chan_config) {
init_get_bits(&gb, pkt->data, pkt->size * 8);
if (get_bits(&gb, 3) != 5) {
avpriv_report_missing_feature(bsfc,
"PCE-based channel configuration "
"without PCE as first syntax "
"element");
ret = AVERROR_PATCHWELCOME;
goto fail;
}
init_put_bits(&pb, pce_data, MAX_PCE_SIZE);
pce_size = ff_copy_pce_data(&pb, &gb) / 8;
flush_put_bits(&pb);
pkt->size -= get_bits_count(&gb)/8;
pkt->data += get_bits_count(&gb)/8;
}

extradata = av_packet_new_side_data(pkt, AV_PKT_DATA_NEW_EXTRADATA,
2 + pce_size);
if (!extradata) {
ret = AVERROR(ENOMEM);
goto fail;
}

init_put_bits(&pb, extradata, 2 + pce_size);
put_bits(&pb, 5, hdr.object_type);
put_bits(&pb, 4, hdr.sampling_index);
put_bits(&pb, 4, hdr.chan_config);
put_bits(&pb, 1, 0); 
put_bits(&pb, 1, 0); 
put_bits(&pb, 1, 0); 
flush_put_bits(&pb);
if (pce_size) {
memcpy(extradata + 2, pce_data, pce_size);
}

ctx->first_frame_done = 1;
}

return 0;

packet_too_small:
av_log(bsfc, AV_LOG_ERROR, "Input packet too small\n");
ret = AVERROR_INVALIDDATA;
fail:
av_packet_unref(pkt);
return ret;
}

static int aac_adtstoasc_init(AVBSFContext *ctx)
{

if (ctx->par_in->extradata) {
MPEG4AudioConfig mp4ac;
int ret = avpriv_mpeg4audio_get_config2(&mp4ac, ctx->par_in->extradata,
ctx->par_in->extradata_size, 1, ctx);
if (ret < 0) {
av_log(ctx, AV_LOG_ERROR, "Error parsing AudioSpecificConfig extradata!\n");
return ret;
}
}

return 0;
}

static const enum AVCodecID codec_ids[] = {
AV_CODEC_ID_AAC, AV_CODEC_ID_NONE,
};

const AVBitStreamFilter ff_aac_adtstoasc_bsf = {
.name = "aac_adtstoasc",
.priv_data_size = sizeof(AACBSFContext),
.init = aac_adtstoasc_init,
.filter = aac_adtstoasc_filter,
.codec_ids = codec_ids,
};
