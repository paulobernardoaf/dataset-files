#include "config.h"
#include <stddef.h>
#include <stdint.h>
#include "adts_header.h"
#include "adts_parser.h"
int av_adts_header_parse(const uint8_t *buf, uint32_t *samples, uint8_t *frames)
{
#if CONFIG_ADTS_HEADER
GetBitContext gb;
AACADTSHeaderInfo hdr;
int err = init_get_bits8(&gb, buf, AV_AAC_ADTS_HEADER_SIZE);
if (err < 0)
return err;
err = ff_adts_header_parse(&gb, &hdr);
if (err < 0)
return err;
*samples = hdr.samples;
*frames = hdr.num_aac_frames;
return 0;
#else
return AVERROR(ENOSYS);
#endif
}
