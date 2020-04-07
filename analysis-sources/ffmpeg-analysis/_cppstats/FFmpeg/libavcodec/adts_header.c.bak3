






















#include "aac_ac3_parser.h"
#include "adts_header.h"
#include "adts_parser.h"
#include "get_bits.h"
#include "mpeg4audio.h"

int ff_adts_header_parse(GetBitContext *gbc, AACADTSHeaderInfo *hdr)
{
int size, rdb, ch, sr;
int aot, crc_abs;

if (get_bits(gbc, 12) != 0xfff)
return AAC_AC3_PARSE_ERROR_SYNC;

skip_bits1(gbc); 
skip_bits(gbc, 2); 
crc_abs = get_bits1(gbc); 
aot = get_bits(gbc, 2); 
sr = get_bits(gbc, 4); 
if (!avpriv_mpeg4audio_sample_rates[sr])
return AAC_AC3_PARSE_ERROR_SAMPLE_RATE;
skip_bits1(gbc); 
ch = get_bits(gbc, 3); 

skip_bits1(gbc); 
skip_bits1(gbc); 


skip_bits1(gbc); 
skip_bits1(gbc); 
size = get_bits(gbc, 13); 
if (size < AV_AAC_ADTS_HEADER_SIZE)
return AAC_AC3_PARSE_ERROR_FRAME_SIZE;

skip_bits(gbc, 11); 
rdb = get_bits(gbc, 2); 

hdr->object_type = aot + 1;
hdr->chan_config = ch;
hdr->crc_absent = crc_abs;
hdr->num_aac_frames = rdb + 1;
hdr->sampling_index = sr;
hdr->sample_rate = avpriv_mpeg4audio_sample_rates[sr];
hdr->samples = (rdb + 1) * 1024;
hdr->bit_rate = size * 8 * hdr->sample_rate / hdr->samples;

return size;
}
