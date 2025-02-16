





















#if !defined(AVCODEC_MEDIACODECDEC_COMMON_H)
#define AVCODEC_MEDIACODECDEC_COMMON_H

#include <stdint.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <sys/types.h>

#include "libavutil/frame.h"
#include "libavutil/pixfmt.h"

#include "avcodec.h"
#include "mediacodec_wrapper.h"

typedef struct MediaCodecDecContext {

AVCodecContext *avctx;
atomic_int refcount;
atomic_int hw_buffer_count;

char *codec_name;

FFAMediaCodec *codec;
FFAMediaFormat *format;

void *surface;

int started;
int draining;
int flushing;
int eos;

int width;
int height;
int stride;
int slice_height;
int color_format;
int crop_top;
int crop_bottom;
int crop_left;
int crop_right;
int display_width;
int display_height;

uint64_t output_buffer_count;
ssize_t current_input_buffer;

bool delay_flush;
atomic_int serial;

} MediaCodecDecContext;

int ff_mediacodec_dec_init(AVCodecContext *avctx,
MediaCodecDecContext *s,
const char *mime,
FFAMediaFormat *format);

int ff_mediacodec_dec_send(AVCodecContext *avctx,
MediaCodecDecContext *s,
AVPacket *pkt,
bool wait);

int ff_mediacodec_dec_receive(AVCodecContext *avctx,
MediaCodecDecContext *s,
AVFrame *frame,
bool wait);

int ff_mediacodec_dec_flush(AVCodecContext *avctx,
MediaCodecDecContext *s);

int ff_mediacodec_dec_close(AVCodecContext *avctx,
MediaCodecDecContext *s);

int ff_mediacodec_dec_is_flushing(AVCodecContext *avctx,
MediaCodecDecContext *s);

typedef struct MediaCodecBuffer {

MediaCodecDecContext *ctx;
ssize_t index;
int64_t pts;
atomic_int released;
int serial;

} MediaCodecBuffer;

#endif 
