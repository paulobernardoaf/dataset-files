




















#include "avformat.h"
#include "rawdec.h"
#include "libavutil/intreadwrite.h"
#include "libavcodec/vc1_common.h"

static int vc1_probe(const AVProbeData *p)
{
int seq = 0, entry = 0, invalid = 0, frame = 0, i;

for (i = 0; i < p->buf_size + 5; i++) {
uint32_t code = AV_RB32(p->buf + i);
if ((code & 0xffffffe0) == 0x100) {
int type = code & 0x11f;
i += 4;
switch (type) {
case VC1_CODE_SEQHDR: {
int profile, level, chromaformat;
profile = (p->buf[i] & 0xc0) >> 6;
if (profile != PROFILE_ADVANCED) {
seq = 0;
invalid++;
continue;
}
level = (p->buf[i] & 0x38) >> 3;
if (level >= 5) {
seq = 0;
invalid++;
continue;
}
chromaformat = (p->buf[i] & 0x6) >> 1;
if (chromaformat != 1) {
seq = 0;
invalid++;
continue;
}
seq++;
i += 6;
break;
}
case VC1_CODE_ENTRYPOINT:
if (!seq) {
invalid++;
continue;
}
entry++;
i += 2;
break;
case VC1_CODE_FRAME:
case VC1_CODE_FIELD:
case VC1_CODE_SLICE:
if (seq && entry)
frame++;
break;
}
}
}

if (frame > 1 && frame >> 1 > invalid)
return AVPROBE_SCORE_EXTENSION / 2 + 1;
if (frame >= 1)
return AVPROBE_SCORE_EXTENSION / 4;
return 0;
}

FF_DEF_RAWVIDEO_DEMUXER2(vc1, "raw VC-1", vc1_probe, "vc1", AV_CODEC_ID_VC1, AVFMT_GENERIC_INDEX|AVFMT_NOTIMESTAMPS)
