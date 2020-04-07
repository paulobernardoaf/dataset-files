

























#if !defined(AVFORMAT_VPCC_H)
#define AVFORMAT_VPCC_H

#include <stdint.h>
#include "avio.h"
#include "avformat.h"
#include "libavcodec/avcodec.h"

typedef struct VPCC {
int profile;
int level;
int bitdepth;
int chroma_subsampling;
int full_range_flag;
} VPCC;










int ff_isom_write_vpcc(AVFormatContext *s, AVIOContext *pb,
AVCodecParameters *par);

int ff_isom_get_vpcc_features(AVFormatContext *s, AVCodecParameters *par,
AVRational *frame_rate, VPCC *vpcc);

#endif 
