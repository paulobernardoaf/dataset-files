#include "libavcodec/avcodec.h"
int main(void){
AVCodec *codec = NULL;
int ret = 0;
while (codec = av_codec_next(codec)) {
if (av_codec_is_encoder(codec)) {
if (codec->type == AVMEDIA_TYPE_AUDIO) {
if (!codec->sample_fmts) {
av_log(NULL, AV_LOG_FATAL, "Encoder %s is missing the sample_fmts field\n", codec->name);
ret = 1;
}
}
}
}
return ret;
}
