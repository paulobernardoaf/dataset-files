




















#include <inttypes.h>

#include "libavutil/adler32.h"
#include "avformat.h"

typedef struct CRCState {
uint32_t crcval;
} CRCState;

static int crc_write_header(struct AVFormatContext *s)
{
CRCState *crc = s->priv_data;


crc->crcval = 1;

return 0;
}

static int crc_write_packet(struct AVFormatContext *s, AVPacket *pkt)
{
CRCState *crc = s->priv_data;
crc->crcval = av_adler32_update(crc->crcval, pkt->data, pkt->size);
return 0;
}

static int crc_write_trailer(struct AVFormatContext *s)
{
CRCState *crc = s->priv_data;
char buf[64];

snprintf(buf, sizeof(buf), "CRC=0x%08"PRIx32"\n", crc->crcval);
avio_write(s->pb, buf, strlen(buf));

return 0;
}

AVOutputFormat ff_crc_muxer = {
.name = "crc",
.long_name = NULL_IF_CONFIG_SMALL("CRC testing"),
.priv_data_size = sizeof(CRCState),
.audio_codec = AV_CODEC_ID_PCM_S16LE,
.video_codec = AV_CODEC_ID_RAWVIDEO,
.write_header = crc_write_header,
.write_packet = crc_write_packet,
.write_trailer = crc_write_trailer,
.flags = AVFMT_NOTIMESTAMPS,
};
