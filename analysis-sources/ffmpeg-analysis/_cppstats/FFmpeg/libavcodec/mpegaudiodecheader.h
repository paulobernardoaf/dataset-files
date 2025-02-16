#include "avcodec.h"
#define MP3_MASK 0xFFFE0CCF
#define MPA_DECODE_HEADER int frame_size; int error_protection; int layer; int sample_rate; int sample_rate_index; int bit_rate; int nb_channels; int mode; int mode_ext; int lsf;
typedef struct MPADecodeHeader {
MPA_DECODE_HEADER
} MPADecodeHeader;
int avpriv_mpegaudio_decode_header(MPADecodeHeader *s, uint32_t header);
int ff_mpa_decode_header(uint32_t head, int *sample_rate,
int *channels, int *frame_size, int *bitrate, enum AVCodecID *codec_id);
static inline int ff_mpa_check_header(uint32_t header){
if ((header & 0xffe00000) != 0xffe00000)
return -1;
if ((header & (3<<19)) == 1<<19)
return -1;
if ((header & (3<<17)) == 0)
return -1;
if ((header & (0xf<<12)) == 0xf<<12)
return -1;
if ((header & (3<<10)) == 3<<10)
return -1;
return 0;
}
