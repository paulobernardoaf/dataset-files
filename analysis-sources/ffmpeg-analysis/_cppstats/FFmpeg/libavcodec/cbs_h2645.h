#include "h2645_parse.h"
typedef struct CodedBitstreamH2645Context {
int mp4;
int nal_length_size;
H2645Packet read_packet;
} CodedBitstreamH2645Context;
