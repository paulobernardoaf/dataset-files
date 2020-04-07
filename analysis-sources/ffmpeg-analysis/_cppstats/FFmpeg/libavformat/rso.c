#include "avformat.h"
#include "internal.h"
#include "rso.h"
const AVCodecTag ff_codec_rso_tags[] = {
{ AV_CODEC_ID_PCM_U8, 0x0100 },
{ AV_CODEC_ID_ADPCM_IMA_WAV, 0x0101 },
{ AV_CODEC_ID_NONE, 0 },
};
