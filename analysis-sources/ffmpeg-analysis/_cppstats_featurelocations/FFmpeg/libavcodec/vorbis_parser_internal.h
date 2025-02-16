


























#if !defined(AVCODEC_VORBIS_PARSER_INTERNAL_H)
#define AVCODEC_VORBIS_PARSER_INTERNAL_H

#include "avcodec.h"
#include "vorbis_parser.h"

struct AVVorbisParseContext {
const AVClass *class;
int extradata_parsed; 
int valid_extradata; 
int blocksize[2]; 
int previous_blocksize; 
int mode_blocksize[64]; 
int mode_count; 
int mode_mask; 
int prev_mask; 
};

#endif 
