




















#if !defined(AVCODEC_H263_PARSER_H)
#define AVCODEC_H263_PARSER_H

#include "parser.h"

int ff_h263_find_frame_end(ParseContext *pc, const uint8_t *buf, int buf_size);

#endif 
