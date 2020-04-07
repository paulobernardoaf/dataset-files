




















#include "avformat.h"
#include "rawdec.h"

static int h263_probe(const AVProbeData *p)
{
uint64_t code= -1;
int i;
int valid_psc=0;
int invalid_psc=0;
int res_change=0;
int src_fmt, last_src_fmt=-1;
int last_gn=0;
int tr, last_tr = -1;

for(i=0; i<p->buf_size; i++){
code = (code<<8) + p->buf[i];
if ((code & 0xfffffc000000) == 0x80000000) {
tr = (code >> 18) & 0xFF;
src_fmt= (code>>10)&7;
if( src_fmt != last_src_fmt
&& last_src_fmt>0 && last_src_fmt<6
&& src_fmt<6)
res_change++;

if (tr == last_tr) {
invalid_psc++;
continue;
}

if (src_fmt != 7 && !(code&(1<<9)) && (code&(1<<5))) {
invalid_psc++;
continue;
}

if((code&0x30000)==0x20000 && src_fmt){
valid_psc++;
last_gn=0;
}else
invalid_psc++;
last_src_fmt= src_fmt;
last_tr = tr;
} else if((code & 0xffff80000000) == 0x80000000) {
int gn= (code>>(31-5)) & 0x1F;
if(gn<last_gn){
invalid_psc++;
}else
last_gn= gn;
}
}
if(valid_psc > 2*invalid_psc + 2*res_change + 3){
return AVPROBE_SCORE_EXTENSION;
}else if(valid_psc > 2*invalid_psc)
return AVPROBE_SCORE_EXTENSION / 2;
return 0;
}

FF_DEF_RAWVIDEO_DEMUXER(h263, "raw H.263", h263_probe, NULL, AV_CODEC_ID_H263)
