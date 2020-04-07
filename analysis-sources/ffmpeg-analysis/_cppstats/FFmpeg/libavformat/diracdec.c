#include "libavutil/intreadwrite.h"
#include "avformat.h"
#include "rawdec.h"
static int dirac_probe(const AVProbeData *p)
{
unsigned size;
if (AV_RL32(p->buf) != MKTAG('B', 'B', 'C', 'D'))
return 0;
size = AV_RB32(p->buf+5);
if (size < 13)
return 0;
if (size + 13LL > p->buf_size)
return AVPROBE_SCORE_MAX / 4;
if (AV_RL32(p->buf + size) != MKTAG('B', 'B', 'C', 'D'))
return 0;
return AVPROBE_SCORE_MAX;
}
FF_DEF_RAWVIDEO_DEMUXER(dirac, "raw Dirac", dirac_probe, NULL, AV_CODEC_ID_DIRAC)
