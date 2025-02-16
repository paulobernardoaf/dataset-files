#include <stdint.h>
#include "libavutil/audio_fifo.h"
#include "libavutil/float_dsp.h"
#include "libavutil/frame.h"
#include "libswresample/swresample.h"
#include "avcodec.h"
#include "opus_rc.h"
#define MAX_FRAME_SIZE 1275
#define MAX_FRAMES 48
#define MAX_PACKET_DUR 5760
#define CELT_SHORT_BLOCKSIZE 120
#define CELT_OVERLAP CELT_SHORT_BLOCKSIZE
#define CELT_MAX_LOG_BLOCKS 3
#define CELT_MAX_FRAME_SIZE (CELT_SHORT_BLOCKSIZE * (1 << CELT_MAX_LOG_BLOCKS))
#define CELT_MAX_BANDS 21
#define SILK_HISTORY 322
#define SILK_MAX_LPC 16
#define ROUND_MULL(a,b,s) (((MUL64(a, b) >> ((s) - 1)) + 1) >> 1)
#define ROUND_MUL16(a,b) ((MUL16(a, b) + 16384) >> 15)
#define OPUS_TS_HEADER 0x7FE0 
#define OPUS_TS_MASK 0xFFE0 
static const uint8_t opus_default_extradata[30] = {
'O', 'p', 'u', 's', 'H', 'e', 'a', 'd',
1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};
enum OpusMode {
OPUS_MODE_SILK,
OPUS_MODE_HYBRID,
OPUS_MODE_CELT,
OPUS_MODE_NB
};
enum OpusBandwidth {
OPUS_BANDWIDTH_NARROWBAND,
OPUS_BANDWIDTH_MEDIUMBAND,
OPUS_BANDWIDTH_WIDEBAND,
OPUS_BANDWIDTH_SUPERWIDEBAND,
OPUS_BANDWIDTH_FULLBAND,
OPUS_BANDWITH_NB
};
typedef struct SilkContext SilkContext;
typedef struct CeltFrame CeltFrame;
typedef struct OpusPacket {
int packet_size; 
int data_size; 
int code; 
int stereo; 
int vbr; 
int config; 
int frame_count; 
int frame_offset[MAX_FRAMES]; 
int frame_size[MAX_FRAMES]; 
int frame_duration; 
enum OpusMode mode; 
enum OpusBandwidth bandwidth; 
} OpusPacket;
typedef struct OpusStreamContext {
AVCodecContext *avctx;
int output_channels;
OpusRangeCoder rc;
OpusRangeCoder redundancy_rc;
SilkContext *silk;
CeltFrame *celt;
AVFloatDSPContext *fdsp;
float silk_buf[2][960];
float *silk_output[2];
DECLARE_ALIGNED(32, float, celt_buf)[2][960];
float *celt_output[2];
DECLARE_ALIGNED(32, float, redundancy_buf)[2][960];
float *redundancy_output[2];
float *out[2];
int out_size;
float *out_dummy;
int out_dummy_allocated_size;
SwrContext *swr;
AVAudioFifo *celt_delay;
int silk_samplerate;
int delayed_samples;
OpusPacket packet;
int redundancy_idx;
} OpusStreamContext;
typedef struct ChannelMap {
int stream_idx;
int channel_idx;
int copy;
int copy_idx;
int silence;
} ChannelMap;
typedef struct OpusContext {
AVClass *av_class;
OpusStreamContext *streams;
int apply_phase_inv;
float **out;
int *out_size;
AVAudioFifo **sync_buffers;
int *decoded_samples;
int nb_streams;
int nb_stereo_streams;
AVFloatDSPContext *fdsp;
int16_t gain_i;
float gain;
ChannelMap *channel_maps;
} OpusContext;
int ff_opus_parse_packet(OpusPacket *pkt, const uint8_t *buf, int buf_size,
int self_delimited);
int ff_opus_parse_extradata(AVCodecContext *avctx, OpusContext *s);
int ff_silk_init(AVCodecContext *avctx, SilkContext **ps, int output_channels);
void ff_silk_free(SilkContext **ps);
void ff_silk_flush(SilkContext *s);
int ff_silk_decode_superframe(SilkContext *s, OpusRangeCoder *rc,
float *output[2],
enum OpusBandwidth bandwidth, int coded_channels,
int duration_ms);
void ff_celt_quant_bands(CeltFrame *f, OpusRangeCoder *rc);
void ff_celt_bitalloc(CeltFrame *f, OpusRangeCoder *rc, int encode);
