#include "libavutil/attributes.h"
#include "avformat.h"
#include "rtpdec_formats.h"
#define RTP_G726_HANDLER(bitrate) static av_cold int g726_ ##bitrate ##_init(AVFormatContext *s, int st_index, PayloadContext *data) { AVStream *stream = s->streams[st_index]; AVCodecParameters *par = stream->codecpar; par->bits_per_coded_sample = bitrate/8; par->bit_rate = par->bits_per_coded_sample * par->sample_rate; return 0; } const RTPDynamicProtocolHandler ff_g726_ ##bitrate ##_dynamic_handler = { .enc_name = "AAL2-G726-" #bitrate, .codec_type = AVMEDIA_TYPE_AUDIO, .codec_id = AV_CODEC_ID_ADPCM_G726, .init = g726_ ##bitrate ##_init, }; const RTPDynamicProtocolHandler ff_g726le_ ##bitrate ##_dynamic_handler = { .enc_name = "G726-" #bitrate, .codec_type = AVMEDIA_TYPE_AUDIO, .codec_id = AV_CODEC_ID_ADPCM_G726LE, .init = g726_ ##bitrate ##_init, }
RTP_G726_HANDLER(16);
RTP_G726_HANDLER(24);
RTP_G726_HANDLER(32);
RTP_G726_HANDLER(40);
