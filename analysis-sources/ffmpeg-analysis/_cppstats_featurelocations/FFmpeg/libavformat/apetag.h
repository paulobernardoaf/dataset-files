





















#if !defined(AVFORMAT_APETAG_H)
#define AVFORMAT_APETAG_H

#include "avformat.h"

#define APE_TAG_PREAMBLE "APETAGEX"
#define APE_TAG_VERSION 2000
#define APE_TAG_FOOTER_BYTES 32
#define APE_TAG_HEADER_BYTES 32






int64_t ff_ape_parse_tag(AVFormatContext *s);




int ff_ape_write_tag(AVFormatContext *s);

#endif 
