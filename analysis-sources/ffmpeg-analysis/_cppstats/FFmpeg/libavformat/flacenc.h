#include "libavcodec/flac.h"
#include "libavcodec/bytestream.h"
#include "avformat.h"
int ff_flac_write_header(AVIOContext *pb, const uint8_t *extradata,
int extradata_size, int last_block);
int ff_flac_is_native_layout(uint64_t channel_layout);
