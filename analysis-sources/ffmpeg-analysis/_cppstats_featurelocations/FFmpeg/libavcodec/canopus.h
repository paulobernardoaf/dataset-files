




















#if !defined(AVCODEC_CANOPUS_H)
#define AVCODEC_CANOPUS_H

#include <stdint.h>

#include "avcodec.h"

int ff_canopus_parse_info_tag(AVCodecContext *avctx,
const uint8_t *src, size_t size);

#endif 
