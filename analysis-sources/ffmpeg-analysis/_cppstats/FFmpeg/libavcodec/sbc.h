#include "avcodec.h"
#include "libavutil/crc.h"
#define MSBC_BLOCKS 15
#define SBC_FREQ_16000 0x00
#define SBC_FREQ_32000 0x01
#define SBC_FREQ_44100 0x02
#define SBC_FREQ_48000 0x03
#define SBC_BLK_4 0x00
#define SBC_BLK_8 0x01
#define SBC_BLK_12 0x02
#define SBC_BLK_16 0x03
#define SBC_MODE_MONO 0x00
#define SBC_MODE_DUAL_CHANNEL 0x01
#define SBC_MODE_STEREO 0x02
#define SBC_MODE_JOINT_STEREO 0x03
#define SBC_AM_LOUDNESS 0x00
#define SBC_AM_SNR 0x01
#define SBC_SB_4 0x00
#define SBC_SB_8 0x01
#define SBC_SYNCWORD 0x9C
#define MSBC_SYNCWORD 0xAD
#define SBCDEC_FIXED_EXTRA_BITS 2
#define SBC_ALIGN 16
struct sbc_frame {
uint8_t frequency;
uint8_t blocks;
enum {
MONO = SBC_MODE_MONO,
DUAL_CHANNEL = SBC_MODE_DUAL_CHANNEL,
STEREO = SBC_MODE_STEREO,
JOINT_STEREO = SBC_MODE_JOINT_STEREO
} mode;
uint8_t channels;
enum {
LOUDNESS = SBC_AM_LOUDNESS,
SNR = SBC_AM_SNR
} allocation;
uint8_t subbands;
uint8_t bitpool;
uint16_t codesize;
uint8_t joint;
DECLARE_ALIGNED(SBC_ALIGN, uint32_t, scale_factor)[2][8];
DECLARE_ALIGNED(SBC_ALIGN, int32_t, sb_sample_f)[16][2][8];
DECLARE_ALIGNED(SBC_ALIGN, int32_t, sb_sample)[16][2][8];
const AVCRC *crc_ctx;
};
uint8_t ff_sbc_crc8(const AVCRC *crc_ctx, const uint8_t *data, size_t len);
void ff_sbc_calculate_bits(const struct sbc_frame *frame, int (*bits)[8]);
