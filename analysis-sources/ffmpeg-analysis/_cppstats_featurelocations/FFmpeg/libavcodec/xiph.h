



















#if !defined(AVCODEC_XIPH_H)
#define AVCODEC_XIPH_H

#include "libavutil/common.h"














int avpriv_split_xiph_headers(const uint8_t *extradata, int extradata_size,
int first_header_size, const uint8_t *header_start[3],
int header_len[3]);

#endif 
