




















#include "libavutil/channel_layout.h"
#include "libavutil/crc.h"
#include "libavutil/log.h"
#include "bytestream.h"
#include "get_bits.h"
#include "flac.h"
#include "flacdata.h"

static const int8_t sample_size_table[] = { 0, 8, 12, 0, 16, 20, 24, 0 };

static const uint64_t flac_channel_layouts[8] = {
AV_CH_LAYOUT_MONO,
AV_CH_LAYOUT_STEREO,
AV_CH_LAYOUT_SURROUND,
AV_CH_LAYOUT_QUAD,
AV_CH_LAYOUT_5POINT0,
AV_CH_LAYOUT_5POINT1,
AV_CH_LAYOUT_6POINT1,
AV_CH_LAYOUT_7POINT1
};

static int64_t get_utf8(GetBitContext *gb)
{
int64_t val;
GET_UTF8(val, get_bits(gb, 8), return -1;)
return val;
}

int ff_flac_decode_frame_header(AVCodecContext *avctx, GetBitContext *gb,
FLACFrameInfo *fi, int log_level_offset)
{
int bs_code, sr_code, bps_code;


if ((get_bits(gb, 15) & 0x7FFF) != 0x7FFC) {
av_log(avctx, AV_LOG_ERROR + log_level_offset, "invalid sync code\n");
return AVERROR_INVALIDDATA;
}


fi->is_var_size = get_bits1(gb);


bs_code = get_bits(gb, 4);
sr_code = get_bits(gb, 4);


fi->ch_mode = get_bits(gb, 4);
if (fi->ch_mode < FLAC_MAX_CHANNELS) {
fi->channels = fi->ch_mode + 1;
fi->ch_mode = FLAC_CHMODE_INDEPENDENT;
} else if (fi->ch_mode < FLAC_MAX_CHANNELS + FLAC_CHMODE_MID_SIDE) {
fi->channels = 2;
fi->ch_mode -= FLAC_MAX_CHANNELS - 1;
} else {
av_log(avctx, AV_LOG_ERROR + log_level_offset,
"invalid channel mode: %d\n", fi->ch_mode);
return AVERROR_INVALIDDATA;
}


bps_code = get_bits(gb, 3);
if (bps_code == 3 || bps_code == 7) {
av_log(avctx, AV_LOG_ERROR + log_level_offset,
"invalid sample size code (%d)\n",
bps_code);
return AVERROR_INVALIDDATA;
}
fi->bps = sample_size_table[bps_code];


if (get_bits1(gb)) {
av_log(avctx, AV_LOG_ERROR + log_level_offset,
"broken stream, invalid padding\n");
return AVERROR_INVALIDDATA;
}


fi->frame_or_sample_num = get_utf8(gb);
if (fi->frame_or_sample_num < 0) {
av_log(avctx, AV_LOG_ERROR + log_level_offset,
"sample/frame number invalid; utf8 fscked\n");
return AVERROR_INVALIDDATA;
}


if (bs_code == 0) {
av_log(avctx, AV_LOG_ERROR + log_level_offset,
"reserved blocksize code: 0\n");
return AVERROR_INVALIDDATA;
} else if (bs_code == 6) {
fi->blocksize = get_bits(gb, 8) + 1;
} else if (bs_code == 7) {
fi->blocksize = get_bits(gb, 16) + 1;
} else {
fi->blocksize = ff_flac_blocksize_table[bs_code];
}


if (sr_code < 12) {
fi->samplerate = ff_flac_sample_rate_table[sr_code];
} else if (sr_code == 12) {
fi->samplerate = get_bits(gb, 8) * 1000;
} else if (sr_code == 13) {
fi->samplerate = get_bits(gb, 16);
} else if (sr_code == 14) {
fi->samplerate = get_bits(gb, 16) * 10;
} else {
av_log(avctx, AV_LOG_ERROR + log_level_offset,
"illegal sample rate code %d\n",
sr_code);
return AVERROR_INVALIDDATA;
}


skip_bits(gb, 8);
if (av_crc(av_crc_get_table(AV_CRC_8_ATM), 0, gb->buffer,
get_bits_count(gb)/8)) {
av_log(avctx, AV_LOG_ERROR + log_level_offset,
"header crc mismatch\n");
return AVERROR_INVALIDDATA;
}

return 0;
}

int ff_flac_get_max_frame_size(int blocksize, int ch, int bps)
{




int count;

count = 16; 
count += ch * ((7+bps+7)/8); 
if (ch == 2) {

count += (( 2*bps+1) * blocksize + 7) / 8;
} else {
count += ( ch*bps * blocksize + 7) / 8;
}
count += 2; 

return count;
}

int ff_flac_is_extradata_valid(AVCodecContext *avctx,
enum FLACExtradataFormat *format,
uint8_t **streaminfo_start)
{
if (!avctx->extradata || avctx->extradata_size < FLAC_STREAMINFO_SIZE) {
av_log(avctx, AV_LOG_ERROR, "extradata NULL or too small.\n");
return 0;
}
if (AV_RL32(avctx->extradata) != MKTAG('f','L','a','C')) {

if (avctx->extradata_size != FLAC_STREAMINFO_SIZE) {
av_log(avctx, AV_LOG_WARNING, "extradata contains %d bytes too many.\n",
FLAC_STREAMINFO_SIZE-avctx->extradata_size);
}
*format = FLAC_EXTRADATA_FORMAT_STREAMINFO;
*streaminfo_start = avctx->extradata;
} else {
if (avctx->extradata_size < 8+FLAC_STREAMINFO_SIZE) {
av_log(avctx, AV_LOG_ERROR, "extradata too small.\n");
return 0;
}
*format = FLAC_EXTRADATA_FORMAT_FULL_HEADER;
*streaminfo_start = &avctx->extradata[8];
}
return 1;
}

void ff_flac_set_channel_layout(AVCodecContext *avctx)
{
if (avctx->channels <= FF_ARRAY_ELEMS(flac_channel_layouts))
avctx->channel_layout = flac_channel_layouts[avctx->channels - 1];
else
avctx->channel_layout = 0;
}

int ff_flac_parse_streaminfo(AVCodecContext *avctx, struct FLACStreaminfo *s,
const uint8_t *buffer)
{
GetBitContext gb;
init_get_bits(&gb, buffer, FLAC_STREAMINFO_SIZE*8);

skip_bits(&gb, 16); 
s->max_blocksize = get_bits(&gb, 16);
if (s->max_blocksize < FLAC_MIN_BLOCKSIZE) {
av_log(avctx, AV_LOG_WARNING, "invalid max blocksize: %d\n",
s->max_blocksize);
s->max_blocksize = 16;
return AVERROR_INVALIDDATA;
}

skip_bits(&gb, 24); 
s->max_framesize = get_bits(&gb, 24);

s->samplerate = get_bits(&gb, 20);
s->channels = get_bits(&gb, 3) + 1;
s->bps = get_bits(&gb, 5) + 1;

if (s->bps < 4) {
av_log(avctx, AV_LOG_ERROR, "invalid bps: %d\n", s->bps);
s->bps = 16;
return AVERROR_INVALIDDATA;
}

avctx->channels = s->channels;
avctx->sample_rate = s->samplerate;
avctx->bits_per_raw_sample = s->bps;

if (!avctx->channel_layout ||
av_get_channel_layout_nb_channels(avctx->channel_layout) != avctx->channels)
ff_flac_set_channel_layout(avctx);

s->samples = get_bits64(&gb, 36);

skip_bits_long(&gb, 64); 
skip_bits_long(&gb, 64); 

return 0;
}
