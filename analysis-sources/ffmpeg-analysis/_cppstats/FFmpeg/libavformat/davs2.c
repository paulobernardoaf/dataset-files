#include "avformat.h"
#include "rawdec.h"
#include "libavcodec/internal.h"
#include "libavutil/intreadwrite.h"
#define ISSQH(x) ((x) == 0xB0 )
#define ISEND(x) ((x) == 0xB1 )
#define ISPIC(x) ((x) == 0xB3 || (x) == 0xB6)
#define ISUNIT(x) ( ISSQH(x) || ISEND(x) || (x) == 0xB2 || ISPIC(x) || (x) == 0xB5 || (x) == 0xB7 )
#define ISAVS2(x) ((x) == 0x20 || (x) == 0x22 || (x) == 0x30 || (x) == 0x32 )
static int avs2_probe(const AVProbeData *p)
{
uint32_t code= -1, hds=0, pic=0, seq=0;
uint8_t state=0;
const uint8_t *ptr = p->buf, *end = p->buf + p->buf_size, *sqb=0;
if (AV_RB32(p->buf) != 0x1B0){
return 0;
}
while (ptr < end) {
ptr = avpriv_find_start_code(ptr, end, &code);
state = code & 0xFF;
if ((code & 0xffffff00) == 0x100) {
if (ISUNIT(state)) {
if (sqb && !hds) {
hds = ptr - sqb;
}
if (ISSQH(state)) {
if (!ISAVS2(*ptr))
return 0;
sqb = ptr;
seq++;
} else if (ISPIC(state)) {
pic++;
} else if (ISEND(state)) {
break;
}
}
}
}
if (seq && hds >= 21 && pic){
return AVPROBE_SCORE_EXTENSION + 2; 
}
return 0;
}
FF_DEF_RAWVIDEO_DEMUXER(avs2, "raw AVS2-P2/IEEE1857.4", avs2_probe, "avs,avs2", AV_CODEC_ID_AVS2)
