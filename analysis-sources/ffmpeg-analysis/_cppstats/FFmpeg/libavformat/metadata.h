#include "avformat.h"
#include "libavutil/dict.h"
typedef struct AVMetadataConv {
const char *native;
const char *generic;
} AVMetadataConv;
void ff_metadata_conv(AVDictionary **pm, const AVMetadataConv *d_conv,
const AVMetadataConv *s_conv);
void ff_metadata_conv_ctx(AVFormatContext *ctx, const AVMetadataConv *d_conv,
const AVMetadataConv *s_conv);
