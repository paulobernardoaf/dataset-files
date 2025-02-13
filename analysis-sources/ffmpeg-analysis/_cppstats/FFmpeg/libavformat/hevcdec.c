#include "libavcodec/hevc.h"
#include "avformat.h"
#include "rawdec.h"
static int hevc_probe(const AVProbeData *p)
{
uint32_t code = -1;
int vps = 0, sps = 0, pps = 0, irap = 0;
int i;
for (i = 0; i < p->buf_size - 1; i++) {
code = (code << 8) + p->buf[i];
if ((code & 0xffffff00) == 0x100) {
uint8_t nal2 = p->buf[i + 1];
int type = (code & 0x7E) >> 1;
if (code & 0x81) 
return 0;
if (nal2 & 0xf8) 
return 0;
switch (type) {
case HEVC_NAL_VPS: vps++; break;
case HEVC_NAL_SPS: sps++; break;
case HEVC_NAL_PPS: pps++; break;
case HEVC_NAL_BLA_N_LP:
case HEVC_NAL_BLA_W_LP:
case HEVC_NAL_BLA_W_RADL:
case HEVC_NAL_CRA_NUT:
case HEVC_NAL_IDR_N_LP:
case HEVC_NAL_IDR_W_RADL: irap++; break;
}
}
}
if (vps && sps && pps && irap)
return AVPROBE_SCORE_EXTENSION + 1; 
return 0;
}
FF_DEF_RAWVIDEO_DEMUXER(hevc, "raw HEVC video", hevc_probe, "hevc,h265,265", AV_CODEC_ID_HEVC)
