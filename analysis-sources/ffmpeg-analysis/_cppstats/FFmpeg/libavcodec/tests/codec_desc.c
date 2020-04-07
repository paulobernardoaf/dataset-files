#include "libavcodec/avcodec.h"
int main(int argc, char **argv)
{
const AVCodecDescriptor *old_desc = NULL, *desc;
while (desc = avcodec_descriptor_next(old_desc)) {
if (old_desc && old_desc->id >= desc->id) {
av_log(NULL, AV_LOG_FATAL, "Unsorted codec_descriptors '%s' and '%s'.\n", old_desc->name, desc->name);
return 1;
}
if (avcodec_descriptor_get(desc->id) != desc) {
av_log(NULL, AV_LOG_FATAL, "avcodec_descriptor_get() failed with '%s'.\n", desc->name);
return 1;
}
if (avcodec_descriptor_get_by_name(desc->name) != desc) {
av_log(NULL, AV_LOG_FATAL, "avcodec_descriptor_get_by_name() failed with '%s'.\n", desc->name);
return 1;
}
old_desc = desc;
}
return 0;
}
