#include "metadata.h"
#include "lrc.h"
const AVMetadataConv ff_lrc_metadata_conv[] = {
{"ti", "title"},
{"al", "album"},
{"ar", "artist"},
{"au", "author"},
{"by", "creator"},
{"re", "encoder"},
{"ve", "encoder_version"},
{0, 0}
};
