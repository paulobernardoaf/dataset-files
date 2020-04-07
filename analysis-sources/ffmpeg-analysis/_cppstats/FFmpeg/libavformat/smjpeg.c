#include "avformat.h"
#include "internal.h"
#include "smjpeg.h"
const AVCodecTag ff_codec_smjpeg_video_tags[] = {
{ AV_CODEC_ID_MJPEG, MKTAG('J', 'F', 'I', 'F') },
{ AV_CODEC_ID_NONE, 0 },
};
const AVCodecTag ff_codec_smjpeg_audio_tags[] = {
{ AV_CODEC_ID_ADPCM_IMA_SMJPEG, MKTAG('A', 'P', 'C', 'M') },
{ AV_CODEC_ID_PCM_S16LE, MKTAG('N', 'O', 'N', 'E') },
{ AV_CODEC_ID_NONE, 0 },
};
