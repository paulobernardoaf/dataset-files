




















#if !defined(AVCODEC_LZF_H)
#define AVCODEC_LZF_H

#include "bytestream.h"

int ff_lzf_uncompress(GetByteContext *gb, uint8_t **buf, int64_t *size);

#endif 
