





















#if !defined(AVCODEC_QSVDEC_H)
#define AVCODEC_QSVDEC_H

#include <stdint.h>
#include <sys/types.h>

#include <mfx/mfxvideo.h>

#include "libavutil/fifo.h"
#include "libavutil/frame.h"
#include "libavutil/pixfmt.h"

#include "avcodec.h"
#include "hwaccel.h"
#include "qsv_internal.h"

typedef struct QSVContext {

mfxSession session;



QSVSession internal_qs;

QSVFramesContext frames_ctx;




QSVFrame *work_frames;

AVFifoBuffer *async_fifo;
int zero_consume_run;
int buffered_count;
int reinit_flag;

enum AVPixelFormat orig_pix_fmt;
uint32_t fourcc;
mfxFrameInfo frame_info;
AVBufferPool *pool;

int initialized;


int async_depth;
int iopattern;
int gpu_copy;

char *load_plugins;

mfxExtBuffer **ext_buffers;
int nb_ext_buffers;
} QSVContext;

extern const AVCodecHWConfigInternal *ff_qsv_hw_configs[];

int ff_qsv_process_data(AVCodecContext *avctx, QSVContext *q,
AVFrame *frame, int *got_frame, AVPacket *pkt);

void ff_qsv_decode_flush(AVCodecContext *avctx, QSVContext *q);

int ff_qsv_decode_close(QSVContext *q);

#endif 
