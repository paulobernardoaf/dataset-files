




















#if !defined(AVCODEC_MSRLEDEC_H)
#define AVCODEC_MSRLEDEC_H

#include "avcodec.h"
#include "bytestream.h"









int ff_msrle_decode(AVCodecContext *avctx, AVFrame *pic,
int depth, GetByteContext *gb);

#endif 
