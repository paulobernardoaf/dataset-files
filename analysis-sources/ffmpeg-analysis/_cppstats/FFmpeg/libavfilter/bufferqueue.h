#if !defined(FF_BUFQUEUE_SIZE)
#define FF_BUFQUEUE_SIZE 64
#endif
#include "avfilter.h"
#include "libavutil/avassert.h"
struct FFBufQueue {
AVFrame *queue[FF_BUFQUEUE_SIZE];
unsigned short head;
unsigned short available; 
};
#define BUCKET(i) queue->queue[(queue->head + (i)) % FF_BUFQUEUE_SIZE]
static inline int ff_bufqueue_is_full(struct FFBufQueue *queue)
{
return queue->available == FF_BUFQUEUE_SIZE;
}
static inline void ff_bufqueue_add(void *log, struct FFBufQueue *queue,
AVFrame *buf)
{
if (ff_bufqueue_is_full(queue)) {
av_log(log, AV_LOG_WARNING, "Buffer queue overflow, dropping.\n");
av_frame_free(&BUCKET(--queue->available));
}
BUCKET(queue->available++) = buf;
}
static inline AVFrame *ff_bufqueue_peek(struct FFBufQueue *queue,
unsigned index)
{
return index < queue->available ? BUCKET(index) : NULL;
}
static inline AVFrame *ff_bufqueue_get(struct FFBufQueue *queue)
{
AVFrame *ret = queue->queue[queue->head];
av_assert0(queue->available);
queue->available--;
queue->queue[queue->head] = NULL;
queue->head = (queue->head + 1) % FF_BUFQUEUE_SIZE;
return ret;
}
static inline void ff_bufqueue_discard_all(struct FFBufQueue *queue)
{
while (queue->available) {
AVFrame *buf = ff_bufqueue_get(queue);
av_frame_free(&buf);
}
}
#undef BUCKET
