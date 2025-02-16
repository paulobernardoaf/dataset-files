#include "libavutil/intreadwrite.h"
#include "avformat.h"
#include "rawdec.h"
#include "libavcodec/dnxhddata.h"
static int dnxhd_probe(const AVProbeData *p)
{
int w, h, compression_id;
if (p->buf_size < 0x2c)
return 0;
if (ff_dnxhd_parse_header_prefix(p->buf) == 0)
return 0;
h = AV_RB16(p->buf + 0x18);
w = AV_RB16(p->buf + 0x1a);
if (!w || !h)
return 0;
compression_id = AV_RB32(p->buf + 0x28);
if ((compression_id < 1235 || compression_id > 1260) &&
(compression_id < 1270 || compression_id > 1274))
return 0;
return AVPROBE_SCORE_MAX;
}
FF_DEF_RAWVIDEO_DEMUXER(dnxhd, "raw DNxHD (SMPTE VC-3)", dnxhd_probe, NULL, AV_CODEC_ID_DNXHD)
