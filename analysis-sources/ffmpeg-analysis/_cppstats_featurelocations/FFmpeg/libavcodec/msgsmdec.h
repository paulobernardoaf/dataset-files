




















#if !defined(AVCODEC_MSGSMDEC_H)
#define AVCODEC_MSGSMDEC_H

#include "avcodec.h"

int ff_msgsm_decode_block(AVCodecContext *avctx, int16_t *samples,
const uint8_t *buf, int mode);

#endif 
