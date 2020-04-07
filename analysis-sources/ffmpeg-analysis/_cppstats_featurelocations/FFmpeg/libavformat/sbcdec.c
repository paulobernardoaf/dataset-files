




















#include "avformat.h"
#include "rawdec.h"

FF_RAW_DEMUXER_CLASS(sbc)
AVInputFormat ff_sbc_demuxer = {
.name = "sbc",
.long_name = NULL_IF_CONFIG_SMALL("raw SBC (low-complexity subband codec)"),
.extensions = "sbc,msbc",
.raw_codec_id = AV_CODEC_ID_SBC,
.read_header = ff_raw_audio_read_header,
.read_packet = ff_raw_read_partial_packet,
.flags = AVFMT_GENERIC_INDEX,
.priv_data_size = sizeof(FFRawDemuxerContext),
.priv_class = &sbc_demuxer_class,
};
