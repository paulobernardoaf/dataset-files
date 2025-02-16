#include "avformat.h"
#include "internal.h"
const AVCodecTag ff_codec_ircam_le_tags[] = {
{ AV_CODEC_ID_PCM_ALAW, 0x10001 },
{ AV_CODEC_ID_PCM_F32LE, 0x00004 },
{ AV_CODEC_ID_PCM_F64LE, 0x00008 },
{ AV_CODEC_ID_PCM_MULAW, 0x20001 },
{ AV_CODEC_ID_PCM_S16LE, 0x00002 },
{ AV_CODEC_ID_PCM_S24LE, 0x00003 },
{ AV_CODEC_ID_PCM_S32LE, 0x40004 },
{ AV_CODEC_ID_PCM_S8, 0x00001 },
{ AV_CODEC_ID_NONE, 0 },
};
const AVCodecTag ff_codec_ircam_be_tags[] = {
{ AV_CODEC_ID_PCM_ALAW, 0x10001 },
{ AV_CODEC_ID_PCM_F32BE, 0x00004 },
{ AV_CODEC_ID_PCM_F64BE, 0x00008 },
{ AV_CODEC_ID_PCM_MULAW, 0x20001 },
{ AV_CODEC_ID_PCM_S16BE, 0x00002 },
{ AV_CODEC_ID_PCM_S24BE, 0x00003 },
{ AV_CODEC_ID_PCM_S32BE, 0x40004 },
{ AV_CODEC_ID_PCM_S8, 0x00001 },
{ AV_CODEC_ID_NONE, 0 },
};
