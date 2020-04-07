#include "libavcodec/dvbtxt.h"
#include "avformat.h"
#include "rawdec.h"
static int dvbtxt_probe(const AVProbeData *p)
{
const uint8_t *end = p->buf + p->buf_size;
const uint8_t *buf;
if ((p->buf_size + 45) % 184 != 0)
return 0;
if (!ff_data_identifier_is_teletext(p->buf[0]))
return 0;
for (buf = p->buf + 1; buf < end; buf += 46) {
if (!ff_data_unit_id_is_teletext(buf[0]) && buf[0] != 0xff)
return 0;
if (buf[1] != 0x2c) 
return 0;
}
return AVPROBE_SCORE_MAX / 2;
}
FF_DEF_RAWSUB_DEMUXER(dvbtxt, "dvbtxt", dvbtxt_probe, NULL, AV_CODEC_ID_DVB_TELETEXT, 0)
