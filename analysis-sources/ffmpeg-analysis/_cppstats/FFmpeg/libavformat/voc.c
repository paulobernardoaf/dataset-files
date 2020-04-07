#include "voc.h"
#include "internal.h"
const unsigned char ff_voc_magic[21] = "Creative Voice File\x1A";
const AVCodecTag ff_voc_codec_tags[] = {
{AV_CODEC_ID_PCM_U8, 0x00},
{AV_CODEC_ID_ADPCM_SBPRO_4, 0x01},
{AV_CODEC_ID_ADPCM_SBPRO_3, 0x02},
{AV_CODEC_ID_ADPCM_SBPRO_2, 0x03},
{AV_CODEC_ID_PCM_S16LE, 0x04},
{AV_CODEC_ID_PCM_ALAW, 0x06},
{AV_CODEC_ID_PCM_MULAW, 0x07},
{AV_CODEC_ID_ADPCM_CT, 0x0200},
{AV_CODEC_ID_NONE, 0},
};
