




















#include "libavutil/intreadwrite.h"
#include "avformat.h"
#include "rawdec.h"

#if CONFIG_MJPEG_2000_DEMUXER
static int mjpeg2000_probe(const AVProbeData *p)
{
const uint8_t *b = p->buf;
int i, marker, marker_size;
int frames = 0, invalid = 0;

for (i = 0; i < p->buf_size - 5; i++) {
if (AV_RB32(b) == 0xff4fff51){
marker_size = AV_RB16(b+4);
if (marker_size + i < p->buf_size - 4) {
marker = AV_RB8(b+4+marker_size);
if (marker == 0xff)
frames++;
else
invalid++;
}
}
b += 1;
}
if (invalid*4 + 1 < frames) {
if (invalid == 0 && frames > 2)
return AVPROBE_SCORE_EXTENSION / 2;
return AVPROBE_SCORE_EXTENSION / 4;
}
return 0;
}
FF_DEF_RAWVIDEO_DEMUXER2(mjpeg_2000, "raw MJPEG 2000 video", mjpeg2000_probe, "j2k", AV_CODEC_ID_JPEG2000, AVFMT_GENERIC_INDEX|AVFMT_NOTIMESTAMPS)
#endif
