#include "libavutil/frame.h"
typedef struct FFFrameBucket {
AVFrame *frame;
} FFFrameBucket;
typedef struct FFFrameQueueGlobal {
char dummy; 
} FFFrameQueueGlobal;
typedef struct FFFrameQueue {
FFFrameBucket *queue;
size_t allocated;
size_t tail;
size_t queued;
FFFrameBucket first_bucket;
uint64_t total_frames_head;
uint64_t total_frames_tail;
uint64_t total_samples_head;
uint64_t total_samples_tail;
int samples_skipped;
} FFFrameQueue;
void ff_framequeue_global_init(FFFrameQueueGlobal *fqg);
void ff_framequeue_init(FFFrameQueue *fq, FFFrameQueueGlobal *fqg);
void ff_framequeue_free(FFFrameQueue *fq);
int ff_framequeue_add(FFFrameQueue *fq, AVFrame *frame);
AVFrame *ff_framequeue_take(FFFrameQueue *fq);
AVFrame *ff_framequeue_peek(FFFrameQueue *fq, size_t idx);
static inline size_t ff_framequeue_queued_frames(const FFFrameQueue *fq)
{
return fq->queued;
}
static inline uint64_t ff_framequeue_queued_samples(const FFFrameQueue *fq)
{
return fq->total_samples_head - fq->total_samples_tail;
}
static inline void ff_framequeue_update_peeked(FFFrameQueue *fq, size_t idx)
{
}
void ff_framequeue_skip_samples(FFFrameQueue *fq, size_t samples, AVRational time_base);
