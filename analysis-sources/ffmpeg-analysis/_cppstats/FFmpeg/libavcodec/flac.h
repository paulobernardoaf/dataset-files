#include "avcodec.h"
#include "bytestream.h"
#include "get_bits.h"
#define FLAC_STREAMINFO_SIZE 34
#define FLAC_MAX_CHANNELS 8
#define FLAC_MIN_BLOCKSIZE 16
#define FLAC_MAX_BLOCKSIZE 65535
#define FLAC_MIN_FRAME_SIZE 11
enum {
FLAC_CHMODE_INDEPENDENT = 0,
FLAC_CHMODE_LEFT_SIDE = 1,
FLAC_CHMODE_RIGHT_SIDE = 2,
FLAC_CHMODE_MID_SIDE = 3,
};
enum {
FLAC_METADATA_TYPE_STREAMINFO = 0,
FLAC_METADATA_TYPE_PADDING,
FLAC_METADATA_TYPE_APPLICATION,
FLAC_METADATA_TYPE_SEEKTABLE,
FLAC_METADATA_TYPE_VORBIS_COMMENT,
FLAC_METADATA_TYPE_CUESHEET,
FLAC_METADATA_TYPE_PICTURE,
FLAC_METADATA_TYPE_INVALID = 127
};
enum FLACExtradataFormat {
FLAC_EXTRADATA_FORMAT_STREAMINFO = 0,
FLAC_EXTRADATA_FORMAT_FULL_HEADER = 1
};
#define FLACCOMMONINFO int samplerate; int channels; int bps; 
#define FLACSTREAMINFO FLACCOMMONINFO int max_blocksize; int max_framesize; int64_t samples; 
typedef struct FLACStreaminfo {
FLACSTREAMINFO
} FLACStreaminfo;
typedef struct FLACFrameInfo {
FLACCOMMONINFO
int blocksize; 
int ch_mode; 
int64_t frame_or_sample_num; 
int is_var_size; 
} FLACFrameInfo;
int ff_flac_parse_streaminfo(AVCodecContext *avctx, struct FLACStreaminfo *s,
const uint8_t *buffer);
int ff_flac_is_extradata_valid(AVCodecContext *avctx,
enum FLACExtradataFormat *format,
uint8_t **streaminfo_start);
int ff_flac_get_max_frame_size(int blocksize, int ch, int bps);
int ff_flac_decode_frame_header(AVCodecContext *avctx, GetBitContext *gb,
FLACFrameInfo *fi, int log_level_offset);
void ff_flac_set_channel_layout(AVCodecContext *avctx);
static av_always_inline void flac_parse_block_header(const uint8_t *block_header,
int *last, int *type, int *size)
{
int tmp = bytestream_get_byte(&block_header);
if (last)
*last = tmp & 0x80;
if (type)
*type = tmp & 0x7F;
if (size)
*size = bytestream_get_be24(&block_header);
}
