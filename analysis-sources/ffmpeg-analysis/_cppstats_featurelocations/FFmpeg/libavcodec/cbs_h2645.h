

















#if !defined(AVCODEC_CBS_H2645_H)
#define AVCODEC_CBS_H2645_H

#include "h2645_parse.h"


typedef struct CodedBitstreamH2645Context {


int mp4;

int nal_length_size;

H2645Packet read_packet;
} CodedBitstreamH2645Context;


#endif 
