#include "parser.h"
#include "aac_ac3_parser.h"
#include "adts_header.h"
#include "adts_parser.h"
#include "get_bits.h"
#include "mpeg4audio.h"
static int aac_sync(uint64_t state, AACAC3ParseContext *hdr_info,
int *need_next_header, int *new_frame_start)
{
GetBitContext bits;
AACADTSHeaderInfo hdr;
int size;
union {
uint64_t u64;
uint8_t u8[8 + AV_INPUT_BUFFER_PADDING_SIZE];
} tmp;
tmp.u64 = av_be2ne64(state);
init_get_bits(&bits, tmp.u8 + 8 - AV_AAC_ADTS_HEADER_SIZE,
AV_AAC_ADTS_HEADER_SIZE * 8);
if ((size = ff_adts_header_parse(&bits, &hdr)) < 0)
return 0;
*need_next_header = 0;
*new_frame_start = 1;
hdr_info->sample_rate = hdr.sample_rate;
hdr_info->channels = ff_mpeg4audio_channels[hdr.chan_config];
hdr_info->samples = hdr.samples;
hdr_info->bit_rate = hdr.bit_rate;
return size;
}
static av_cold int aac_parse_init(AVCodecParserContext *s1)
{
AACAC3ParseContext *s = s1->priv_data;
s->header_size = AV_AAC_ADTS_HEADER_SIZE;
s->sync = aac_sync;
return 0;
}
AVCodecParser ff_aac_parser = {
.codec_ids = { AV_CODEC_ID_AAC },
.priv_data_size = sizeof(AACAC3ParseContext),
.parser_init = aac_parse_init,
.parser_parse = ff_aac_ac3_parse,
.parser_close = ff_parse_close,
};
