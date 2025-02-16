#include <stdint.h>
#include "libavutil/attributes.h"
#include "get_bits.h"
#include "internal.h"
#include "put_bits.h"
typedef struct MPEG4AudioConfig {
int object_type;
int sampling_index;
int sample_rate;
int chan_config;
int sbr; 
int ext_object_type;
int ext_sampling_index;
int ext_sample_rate;
int ext_chan_config;
int channels;
int ps; 
int frame_length_short;
} MPEG4AudioConfig;
extern av_export_avcodec const int avpriv_mpeg4audio_sample_rates[16];
extern const uint8_t ff_mpeg4audio_channels[8];
int ff_mpeg4audio_get_config_gb(MPEG4AudioConfig *c, GetBitContext *gb,
int sync_extension, void *logctx);
#if LIBAVCODEC_VERSION_MAJOR < 59
int avpriv_mpeg4audio_get_config(MPEG4AudioConfig *c, const uint8_t *buf,
int bit_size, int sync_extension);
#endif
int avpriv_mpeg4audio_get_config2(MPEG4AudioConfig *c, const uint8_t *buf,
int size, int sync_extension, void *logctx);
enum AudioObjectType {
AOT_NULL,
AOT_AAC_MAIN, 
AOT_AAC_LC, 
AOT_AAC_SSR, 
AOT_AAC_LTP, 
AOT_SBR, 
AOT_AAC_SCALABLE, 
AOT_TWINVQ, 
AOT_CELP, 
AOT_HVXC, 
AOT_TTSI = 12, 
AOT_MAINSYNTH, 
AOT_WAVESYNTH, 
AOT_MIDI, 
AOT_SAFX, 
AOT_ER_AAC_LC, 
AOT_ER_AAC_LTP = 19, 
AOT_ER_AAC_SCALABLE, 
AOT_ER_TWINVQ, 
AOT_ER_BSAC, 
AOT_ER_AAC_LD, 
AOT_ER_CELP, 
AOT_ER_HVXC, 
AOT_ER_HILN, 
AOT_ER_PARAM, 
AOT_SSC, 
AOT_PS, 
AOT_SURROUND, 
AOT_ESCAPE, 
AOT_L1, 
AOT_L2, 
AOT_L3, 
AOT_DST, 
AOT_ALS, 
AOT_SLS, 
AOT_SLS_NON_CORE, 
AOT_ER_AAC_ELD, 
AOT_SMR_SIMPLE, 
AOT_SMR_MAIN, 
AOT_USAC_NOSBR, 
AOT_SAOC, 
AOT_LD_SURROUND, 
AOT_USAC, 
};
#define MAX_PCE_SIZE 320 
static av_always_inline unsigned int ff_pce_copy_bits(PutBitContext *pb,
GetBitContext *gb,
int bits)
{
unsigned int el = get_bits(gb, bits);
put_bits(pb, bits, el);
return el;
}
static inline int ff_copy_pce_data(PutBitContext *pb, GetBitContext *gb)
{
int five_bit_ch, four_bit_ch, comment_size, bits;
int offset = put_bits_count(pb);
ff_pce_copy_bits(pb, gb, 10); 
five_bit_ch = ff_pce_copy_bits(pb, gb, 4); 
five_bit_ch += ff_pce_copy_bits(pb, gb, 4); 
five_bit_ch += ff_pce_copy_bits(pb, gb, 4); 
four_bit_ch = ff_pce_copy_bits(pb, gb, 2); 
four_bit_ch += ff_pce_copy_bits(pb, gb, 3); 
five_bit_ch += ff_pce_copy_bits(pb, gb, 4); 
if (ff_pce_copy_bits(pb, gb, 1)) 
ff_pce_copy_bits(pb, gb, 4);
if (ff_pce_copy_bits(pb, gb, 1)) 
ff_pce_copy_bits(pb, gb, 4);
if (ff_pce_copy_bits(pb, gb, 1)) 
ff_pce_copy_bits(pb, gb, 3);
for (bits = five_bit_ch*5+four_bit_ch*4; bits > 16; bits -= 16)
ff_pce_copy_bits(pb, gb, 16);
if (bits)
ff_pce_copy_bits(pb, gb, bits);
avpriv_align_put_bits(pb);
align_get_bits(gb);
comment_size = ff_pce_copy_bits(pb, gb, 8);
for (; comment_size > 0; comment_size--)
ff_pce_copy_bits(pb, gb, 8);
return put_bits_count(pb) - offset;
}
