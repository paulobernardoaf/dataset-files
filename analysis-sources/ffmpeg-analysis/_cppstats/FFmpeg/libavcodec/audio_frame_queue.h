#include "avcodec.h"
typedef struct AudioFrame {
int64_t pts;
int duration;
} AudioFrame;
typedef struct AudioFrameQueue {
AVCodecContext *avctx;
int remaining_delay;
int remaining_samples;
AudioFrame *frames;
unsigned frame_count;
unsigned frame_alloc;
} AudioFrameQueue;
void ff_af_queue_init(AVCodecContext *avctx, AudioFrameQueue *afq);
void ff_af_queue_close(AudioFrameQueue *afq);
int ff_af_queue_add(AudioFrameQueue *afq, const AVFrame *f);
void ff_af_queue_remove(AudioFrameQueue *afq, int nb_samples, int64_t *pts,
int64_t *duration);
