#include <stdint.h>
#include "libavutil/buffer.h"
#include "libavutil/channel_layout.h"
#include "libavutil/mathematics.h"
#include "libavutil/pixfmt.h"
#include "avcodec.h"
#include "config.h"
#define FF_CODEC_CAP_INIT_THREADSAFE (1 << 0)
#define FF_CODEC_CAP_INIT_CLEANUP (1 << 1)
#define FF_CODEC_CAP_SETS_PKT_DTS (1 << 2)
#define FF_CODEC_CAP_SKIP_FRAME_FILL_PARAM (1 << 3)
#define FF_CODEC_CAP_EXPORTS_CROPPING (1 << 4)
#define FF_CODEC_CAP_SLICE_THREAD_HAS_MF (1 << 5)
#define FF_CODEC_TAGS_END -1
#if defined(TRACE)
#define ff_tlog(ctx, ...) av_log(ctx, AV_LOG_TRACE, __VA_ARGS__)
#else
#define ff_tlog(ctx, ...) do { } while(0)
#endif
#define FF_DEFAULT_QUANT_BIAS 999999
#define FF_QSCALE_TYPE_MPEG1 0
#define FF_QSCALE_TYPE_MPEG2 1
#define FF_QSCALE_TYPE_H264 2
#define FF_QSCALE_TYPE_VP56 3
#define FF_SANE_NB_CHANNELS 512U
#define FF_SIGNBIT(x) ((x) >> CHAR_BIT * sizeof(x) - 1)
#if HAVE_SIMD_ALIGN_64
#define STRIDE_ALIGN 64 
#elif HAVE_SIMD_ALIGN_32
#define STRIDE_ALIGN 32
#elif HAVE_SIMD_ALIGN_16
#define STRIDE_ALIGN 16
#else
#define STRIDE_ALIGN 8
#endif
typedef struct FramePool {
AVBufferPool *pools[4];
int format;
int width, height;
int stride_align[AV_NUM_DATA_POINTERS];
int linesize[4];
int planes;
int channels;
int samples;
} FramePool;
typedef struct DecodeSimpleContext {
AVPacket *in_pkt;
AVFrame *out_frame;
} DecodeSimpleContext;
typedef struct DecodeFilterContext {
AVBSFContext **bsfs;
int nb_bsfs;
} DecodeFilterContext;
typedef struct AVCodecInternal {
int is_copy;
int allocate_progress;
int last_audio_frame;
AVFrame *to_free;
FramePool *pool;
void *thread_ctx;
DecodeSimpleContext ds;
DecodeFilterContext filter;
AVPacket *last_pkt_props;
uint8_t *byte_buffer;
unsigned int byte_buffer_size;
void *frame_thread_encoder;
int skip_samples;
void *hwaccel_priv_data;
int draining;
AVPacket *buffer_pkt;
int buffer_pkt_valid; 
AVFrame *buffer_frame;
int draining_done;
int compat_decode;
int compat_decode_warned;
size_t compat_decode_consumed;
size_t compat_decode_partial_size;
AVFrame *compat_decode_frame;
int showed_multi_packet_warning;
int skip_samples_multiplier;
int nb_draining_errors;
int changed_frames_dropped;
int initial_format;
int initial_width, initial_height;
int initial_sample_rate;
int initial_channels;
uint64_t initial_channel_layout;
} AVCodecInternal;
struct AVCodecDefault {
const uint8_t *key;
const uint8_t *value;
};
extern const uint8_t ff_log2_run[41];
int ff_match_2uint16(const uint16_t (*tab)[2], int size, int a, int b);
unsigned int avpriv_toupper4(unsigned int x);
void ff_color_frame(AVFrame *frame, const int color[4]);
#define FF_MAX_EXTRADATA_SIZE ((1 << 28) - AV_INPUT_BUFFER_PADDING_SIZE)
int ff_alloc_packet2(AVCodecContext *avctx, AVPacket *avpkt, int64_t size, int64_t min_size);
attribute_deprecated int ff_alloc_packet(AVPacket *avpkt, int size);
static av_always_inline int64_t ff_samples_to_time_base(AVCodecContext *avctx,
int64_t samples)
{
if(samples == AV_NOPTS_VALUE)
return AV_NOPTS_VALUE;
return av_rescale_q(samples, (AVRational){ 1, avctx->sample_rate },
avctx->time_base);
}
static av_always_inline float ff_exp2fi(int x) {
if (-126 <= x && x <= 128)
return av_int2float((x+127) << 23);
else if (x > 128)
return INFINITY;
else if (x > -150)
return av_int2float(1 << (x+149));
else
return 0;
}
int ff_get_buffer(AVCodecContext *avctx, AVFrame *frame, int flags);
#define FF_REGET_BUFFER_FLAG_READONLY 1 
int ff_reget_buffer(AVCodecContext *avctx, AVFrame *frame, int flags);
int ff_thread_can_start_frame(AVCodecContext *avctx);
int avpriv_h264_has_num_reorder_frames(AVCodecContext *avctx);
int ff_codec_open2_recursive(AVCodecContext *avctx, const AVCodec *codec, AVDictionary **options);
int avpriv_bprint_to_extradata(AVCodecContext *avctx, struct AVBPrint *buf);
const uint8_t *avpriv_find_start_code(const uint8_t *p,
const uint8_t *end,
uint32_t *state);
int avpriv_codec_get_cap_skip_frame_fill_param(const AVCodec *codec);
int ff_set_dimensions(AVCodecContext *s, int width, int height);
int ff_set_sar(AVCodecContext *avctx, AVRational sar);
int ff_side_data_update_matrix_encoding(AVFrame *frame,
enum AVMatrixEncoding matrix_encoding);
int ff_get_format(AVCodecContext *avctx, const enum AVPixelFormat *fmt);
int ff_decode_frame_props(AVCodecContext *avctx, AVFrame *frame);
AVCPBProperties *ff_add_cpb_side_data(AVCodecContext *avctx);
int ff_side_data_set_encoder_stats(AVPacket *pkt, int quality, int64_t *error, int error_count, int pict_type);
int ff_side_data_set_prft(AVPacket *pkt, int64_t timestamp);
int ff_alloc_a53_sei(const AVFrame *frame, size_t prefix_len,
void **data, size_t *sei_size);
int64_t ff_guess_coded_bitrate(AVCodecContext *avctx);
int ff_int_from_list_or_default(void *ctx, const char * val_name, int val,
const int * array_valid_values, int default_value);
void ff_dvdsub_parse_palette(uint32_t *palette, const char *p);
#if defined(_WIN32) && CONFIG_SHARED && !defined(BUILDING_avcodec)
#define av_export_avcodec __declspec(dllimport)
#else
#define av_export_avcodec
#endif
