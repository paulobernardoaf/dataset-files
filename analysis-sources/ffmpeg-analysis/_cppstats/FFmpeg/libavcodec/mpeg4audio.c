#include "get_bits.h"
#include "put_bits.h"
#include "mpeg4audio.h"
static int parse_config_ALS(GetBitContext *gb, MPEG4AudioConfig *c)
{
if (get_bits_left(gb) < 112)
return AVERROR_INVALIDDATA;
if (get_bits_long(gb, 32) != MKBETAG('A','L','S','\0'))
return AVERROR_INVALIDDATA;
c->sample_rate = get_bits_long(gb, 32);
if (c->sample_rate <= 0) {
av_log(NULL, AV_LOG_ERROR, "Invalid sample rate %d\n", c->sample_rate);
return AVERROR_INVALIDDATA;
}
skip_bits_long(gb, 32);
c->chan_config = 0;
c->channels = get_bits(gb, 16) + 1;
return 0;
}
const int avpriv_mpeg4audio_sample_rates[16] = {
96000, 88200, 64000, 48000, 44100, 32000,
24000, 22050, 16000, 12000, 11025, 8000, 7350
};
const uint8_t ff_mpeg4audio_channels[8] = {
0, 1, 2, 3, 4, 5, 6, 8
};
static inline int get_object_type(GetBitContext *gb)
{
int object_type = get_bits(gb, 5);
if (object_type == AOT_ESCAPE)
object_type = 32 + get_bits(gb, 6);
return object_type;
}
static inline int get_sample_rate(GetBitContext *gb, int *index)
{
*index = get_bits(gb, 4);
return *index == 0x0f ? get_bits(gb, 24) :
avpriv_mpeg4audio_sample_rates[*index];
}
int ff_mpeg4audio_get_config_gb(MPEG4AudioConfig *c, GetBitContext *gb,
int sync_extension, void *logctx)
{
int specific_config_bitindex, ret;
int start_bit_index = get_bits_count(gb);
c->object_type = get_object_type(gb);
c->sample_rate = get_sample_rate(gb, &c->sampling_index);
c->chan_config = get_bits(gb, 4);
if (c->chan_config < FF_ARRAY_ELEMS(ff_mpeg4audio_channels))
c->channels = ff_mpeg4audio_channels[c->chan_config];
else {
av_log(logctx, AV_LOG_ERROR, "Invalid chan_config %d\n", c->chan_config);
return AVERROR_INVALIDDATA;
}
c->sbr = -1;
c->ps = -1;
if (c->object_type == AOT_SBR || (c->object_type == AOT_PS &&
!(show_bits(gb, 3) & 0x03 && !(show_bits(gb, 9) & 0x3F)))) {
if (c->object_type == AOT_PS)
c->ps = 1;
c->ext_object_type = AOT_SBR;
c->sbr = 1;
c->ext_sample_rate = get_sample_rate(gb, &c->ext_sampling_index);
c->object_type = get_object_type(gb);
if (c->object_type == AOT_ER_BSAC)
c->ext_chan_config = get_bits(gb, 4);
} else {
c->ext_object_type = AOT_NULL;
c->ext_sample_rate = 0;
}
specific_config_bitindex = get_bits_count(gb);
if (c->object_type == AOT_ALS) {
skip_bits(gb, 5);
if (show_bits(gb, 24) != MKBETAG('\0','A','L','S'))
skip_bits(gb, 24);
specific_config_bitindex = get_bits_count(gb);
ret = parse_config_ALS(gb, c);
if (ret < 0)
return ret;
}
if (c->ext_object_type != AOT_SBR && sync_extension) {
while (get_bits_left(gb) > 15) {
if (show_bits(gb, 11) == 0x2b7) { 
get_bits(gb, 11);
c->ext_object_type = get_object_type(gb);
if (c->ext_object_type == AOT_SBR && (c->sbr = get_bits1(gb)) == 1) {
c->ext_sample_rate = get_sample_rate(gb, &c->ext_sampling_index);
if (c->ext_sample_rate == c->sample_rate)
c->sbr = -1;
}
if (get_bits_left(gb) > 11 && get_bits(gb, 11) == 0x548)
c->ps = get_bits1(gb);
break;
} else
get_bits1(gb); 
}
}
if (!c->sbr)
c->ps = 0;
if ((c->ps == -1 && c->object_type != AOT_AAC_LC) || c->channels & ~0x01)
c->ps = 0;
return specific_config_bitindex - start_bit_index;
}
#if LIBAVCODEC_VERSION_MAJOR < 59
int avpriv_mpeg4audio_get_config(MPEG4AudioConfig *c, const uint8_t *buf,
int bit_size, int sync_extension)
{
GetBitContext gb;
int ret;
if (bit_size <= 0)
return AVERROR_INVALIDDATA;
ret = init_get_bits(&gb, buf, bit_size);
if (ret < 0)
return ret;
return ff_mpeg4audio_get_config_gb(c, &gb, sync_extension, NULL);
}
#endif
int avpriv_mpeg4audio_get_config2(MPEG4AudioConfig *c, const uint8_t *buf,
int size, int sync_extension, void *logctx)
{
GetBitContext gb;
int ret;
if (size <= 0)
return AVERROR_INVALIDDATA;
ret = init_get_bits8(&gb, buf, size);
if (ret < 0)
return ret;
return ff_mpeg4audio_get_config_gb(c, &gb, sync_extension, logctx);
}
