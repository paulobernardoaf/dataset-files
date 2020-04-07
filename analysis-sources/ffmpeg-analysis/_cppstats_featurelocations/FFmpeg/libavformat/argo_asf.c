




















#include "avformat.h"
#include "internal.h"
#include "libavutil/intreadwrite.h"
#include "libavutil/avassert.h"

#define ASF_TAG MKTAG('A', 'S', 'F', '\0')
#define ASF_FILE_HEADER_SIZE 24
#define ASF_CHUNK_HEADER_SIZE 20

typedef struct ArgoASFFileHeader {
uint32_t magic; 
uint16_t version_major; 
uint16_t version_minor; 
uint32_t num_chunks; 
uint32_t chunk_offset; 
int8_t name[8]; 
} ArgoASFFileHeader;

typedef struct ArgoASFChunkHeader {
uint32_t num_blocks; 
uint32_t num_samples; 
uint32_t unk1; 
uint16_t sample_rate; 
uint16_t unk2; 
uint32_t flags; 
} ArgoASFChunkHeader;

enum {
ASF_CF_BITS_PER_SAMPLE = (1 << 0), 
ASF_CF_STEREO = (1 << 1), 
ASF_CF_ALWAYS1_1 = (1 << 2), 
ASF_CF_ALWAYS1_2 = (1 << 3), 

ASF_CF_ALWAYS1 = ASF_CF_ALWAYS1_1 | ASF_CF_ALWAYS1_2,
ASF_CF_ALWAYS0 = ~(ASF_CF_BITS_PER_SAMPLE | ASF_CF_STEREO | ASF_CF_ALWAYS1)
};

typedef struct ArgoASFDemuxContext {
ArgoASFFileHeader fhdr;
ArgoASFChunkHeader ckhdr;
uint32_t blocks_read;
} ArgoASFDemuxContext;

static void argo_asf_parse_file_header(ArgoASFFileHeader *hdr, const uint8_t *buf)
{
hdr->magic = AV_RL32(buf + 0);
hdr->version_major = AV_RL16(buf + 4);
hdr->version_minor = AV_RL16(buf + 6);
hdr->num_chunks = AV_RL32(buf + 8);
hdr->chunk_offset = AV_RL32(buf + 12);
for (int i = 0; i < FF_ARRAY_ELEMS(hdr->name); i++)
hdr->name[i] = AV_RL8(buf + 16 + i);
}

static void argo_asf_parse_chunk_header(ArgoASFChunkHeader *hdr, const uint8_t *buf)
{
hdr->num_blocks = AV_RL32(buf + 0);
hdr->num_samples = AV_RL32(buf + 4);
hdr->unk1 = AV_RL32(buf + 8);
hdr->sample_rate = AV_RL16(buf + 12);
hdr->unk2 = AV_RL16(buf + 14);
hdr->flags = AV_RL32(buf + 16);
}







static int argo_asf_is_known_version(const ArgoASFFileHeader *hdr)
{
return (hdr->version_major == 1 && hdr->version_minor == 1) ||
(hdr->version_major == 1 && hdr->version_minor == 2) ||
(hdr->version_major == 2 && hdr->version_minor == 1);
}

static int argo_asf_probe(const AVProbeData *p)
{
ArgoASFFileHeader hdr;

av_assert0(AVPROBE_PADDING_SIZE >= ASF_FILE_HEADER_SIZE);

argo_asf_parse_file_header(&hdr, p->buf);

if (hdr.magic != ASF_TAG)
return 0;

if (!argo_asf_is_known_version(&hdr))
return AVPROBE_SCORE_EXTENSION / 2;

return AVPROBE_SCORE_EXTENSION + 1;
}

static int argo_asf_read_header(AVFormatContext *s)
{
int64_t ret;
AVIOContext *pb = s->pb;
AVStream *st;
ArgoASFDemuxContext *asf = s->priv_data;
uint8_t buf[FFMAX(ASF_FILE_HEADER_SIZE, ASF_CHUNK_HEADER_SIZE)];

if (!(st = avformat_new_stream(s, NULL)))
return AVERROR(ENOMEM);

if ((ret = avio_read(pb, buf, ASF_FILE_HEADER_SIZE)) < 0)
return ret;
else if (ret != ASF_FILE_HEADER_SIZE)
return AVERROR(EIO);

argo_asf_parse_file_header(&asf->fhdr, buf);

if (!argo_asf_is_known_version(&asf->fhdr)) {
avpriv_request_sample(s, "Version %hu.%hu",
asf->fhdr.version_major, asf->fhdr.version_minor
);
return AVERROR_PATCHWELCOME;
}

if (asf->fhdr.num_chunks == 0) {
return AVERROR_INVALIDDATA;
} else if (asf->fhdr.num_chunks > 1) {
avpriv_request_sample(s, ">1 chunk");
return AVERROR_PATCHWELCOME;
}

if (asf->fhdr.chunk_offset < ASF_FILE_HEADER_SIZE)
return AVERROR_INVALIDDATA;

if ((ret = avio_skip(pb, asf->fhdr.chunk_offset - ASF_FILE_HEADER_SIZE)) < 0)
return ret;

if ((ret = avio_read(pb, buf, ASF_CHUNK_HEADER_SIZE)) < 0)
return ret;
else if (ret != ASF_CHUNK_HEADER_SIZE)
return AVERROR(EIO);

argo_asf_parse_chunk_header(&asf->ckhdr, buf);

if ((asf->ckhdr.flags & ASF_CF_ALWAYS1) != ASF_CF_ALWAYS1 || (asf->ckhdr.flags & ASF_CF_ALWAYS0) != 0) {
avpriv_request_sample(s, "Nonstandard flags (0x%08X)", asf->ckhdr.flags);
return AVERROR_PATCHWELCOME;
}

st->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;
st->codecpar->codec_id = AV_CODEC_ID_ADPCM_ARGO;
st->codecpar->format = AV_SAMPLE_FMT_S16P;

if (asf->ckhdr.flags & ASF_CF_STEREO) {
st->codecpar->channel_layout = AV_CH_LAYOUT_STEREO;
st->codecpar->channels = 2;
} else {
st->codecpar->channel_layout = AV_CH_LAYOUT_MONO;
st->codecpar->channels = 1;
}

st->codecpar->sample_rate = asf->ckhdr.sample_rate;

st->codecpar->bits_per_coded_sample = 4;

if (asf->ckhdr.flags & ASF_CF_BITS_PER_SAMPLE)
st->codecpar->bits_per_raw_sample = 16;
else
st->codecpar->bits_per_raw_sample = 8;

if (st->codecpar->bits_per_raw_sample != 16) {

avpriv_request_sample(s, "Non 16-bit samples");
return AVERROR_PATCHWELCOME;
}





st->codecpar->frame_size = st->codecpar->channels +
(asf->ckhdr.num_samples / 2) *
st->codecpar->channels;

st->codecpar->block_align = st->codecpar->frame_size;

st->codecpar->bit_rate = st->codecpar->channels *
st->codecpar->sample_rate *
st->codecpar->bits_per_coded_sample;

avpriv_set_pts_info(st, 64, 1, st->codecpar->sample_rate);
st->start_time = 0;
st->duration = asf->ckhdr.num_blocks * asf->ckhdr.num_samples;
st->nb_frames = asf->ckhdr.num_blocks;
return 0;
}

static int argo_asf_read_packet(AVFormatContext *s, AVPacket *pkt)
{
ArgoASFDemuxContext *asf = s->priv_data;

AVStream *st = s->streams[0];
AVIOContext *pb = s->pb;
int ret;

if (asf->blocks_read >= asf->ckhdr.num_blocks)
return AVERROR_EOF;

if ((ret = av_get_packet(pb, pkt, st->codecpar->frame_size)) < 0)
return ret;
else if (ret != st->codecpar->frame_size)
return AVERROR_INVALIDDATA;

pkt->stream_index = st->index;
pkt->duration = asf->ckhdr.num_samples;

++asf->blocks_read;
return 0;
}






AVInputFormat ff_argo_asf_demuxer = {
.name = "argo_asf",
.long_name = NULL_IF_CONFIG_SMALL("Argonaut Games ASF"),
.priv_data_size = sizeof(ArgoASFDemuxContext),
.read_probe = argo_asf_probe,
.read_header = argo_asf_read_header,
.read_packet = argo_asf_read_packet
};
