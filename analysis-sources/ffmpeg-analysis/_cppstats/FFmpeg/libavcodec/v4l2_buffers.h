#include <stdatomic.h>
#include <linux/videodev2.h>
#include "avcodec.h"
enum V4L2Buffer_status {
V4L2BUF_AVAILABLE,
V4L2BUF_IN_DRIVER,
V4L2BUF_RET_USER,
};
typedef struct V4L2Buffer {
struct V4L2Context *context;
AVBufferRef *context_ref;
atomic_uint context_refcount;
struct V4L2Plane_info {
int bytesperline;
void * mm_addr;
size_t length;
} plane_info[VIDEO_MAX_PLANES];
int num_planes;
struct v4l2_buffer buf;
struct v4l2_plane planes[VIDEO_MAX_PLANES];
int flags;
enum V4L2Buffer_status status;
} V4L2Buffer;
int ff_v4l2_buffer_buf_to_avframe(AVFrame *frame, V4L2Buffer *buf);
int ff_v4l2_buffer_buf_to_avpkt(AVPacket *pkt, V4L2Buffer *buf);
int ff_v4l2_buffer_avpkt_to_buf(const AVPacket *pkt, V4L2Buffer *out);
int ff_v4l2_buffer_avframe_to_buf(const AVFrame *frame, V4L2Buffer *out);
int ff_v4l2_buffer_initialize(V4L2Buffer* avbuf, int index);
int ff_v4l2_buffer_enqueue(V4L2Buffer* avbuf);
