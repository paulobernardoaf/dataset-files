#include "avcodec.h"
#include "bytestream.h"
int ff_msrle_decode(AVCodecContext *avctx, AVFrame *pic,
int depth, GetByteContext *gb);
