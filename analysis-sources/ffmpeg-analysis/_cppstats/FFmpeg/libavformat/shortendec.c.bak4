






















#include "avformat.h"
#include "rawdec.h"
#include "libavcodec/golomb.h"

static int shn_probe(const AVProbeData *p)
{
GetBitContext gb;
int version, internal_ftype, channels, blocksize;

if (AV_RB32(p->buf) != 0x616a6b67)
return 0;
version = p->buf[4];
if (init_get_bits8(&gb, p->buf + 5, p->buf_size - 5 - AV_INPUT_BUFFER_PADDING_SIZE) < 0)
return 0;
if (!version) {
internal_ftype = get_ur_golomb_shorten(&gb, 4);
channels = get_ur_golomb_shorten(&gb, 0);
blocksize = 256;
} else {
unsigned k;
k = get_ur_golomb_shorten(&gb, 2);
if (k > 31)
return 0;
internal_ftype = get_ur_golomb_shorten(&gb, k);
k = get_ur_golomb_shorten(&gb, 2);
if (k > 31)
return 0;
channels = get_ur_golomb_shorten(&gb, k);
k = get_ur_golomb_shorten(&gb, 2);
if (k > 31)
return 0;
blocksize = get_ur_golomb_shorten(&gb, k);
}

if (internal_ftype != 2 && internal_ftype != 3 && internal_ftype != 5)
return 0;
if (channels < 1 || channels > 8)
return 0;
if (blocksize < 1 || blocksize > 65535)
return 0;

return AVPROBE_SCORE_EXTENSION + 1;
}

FF_RAW_DEMUXER_CLASS(shorten)
AVInputFormat ff_shorten_demuxer = {
.name = "shn",
.long_name = NULL_IF_CONFIG_SMALL("raw Shorten"),
.read_probe = shn_probe,
.read_header = ff_raw_audio_read_header,
.read_packet = ff_raw_read_partial_packet,
.flags = AVFMT_NOBINSEARCH | AVFMT_NOGENSEARCH | AVFMT_NO_BYTE_SEEK | AVFMT_NOTIMESTAMPS,
.extensions = "shn",
.raw_codec_id = AV_CODEC_ID_SHORTEN,
.priv_data_size = sizeof(FFRawDemuxerContext),
.priv_class = &shorten_demuxer_class,
};
