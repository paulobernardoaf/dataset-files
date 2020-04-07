






















#if !defined(AVCODEC_V4L2_M2M_H)
#define AVCODEC_V4L2_M2M_H

#include <semaphore.h>
#include <unistd.h>
#include <dirent.h>
#include <linux/videodev2.h>

#include "libavcodec/avcodec.h"
#include "v4l2_context.h"

#define container_of(ptr, type, member) ({ const __typeof__(((type *)0)->member ) *__mptr = (ptr); (type *)((char *)__mptr - offsetof(type,member) );})



#define V4L_M2M_DEFAULT_OPTS { "num_output_buffers", "Number of buffers in the output context",OFFSET(num_output_buffers), AV_OPT_TYPE_INT, { .i64 = 16 }, 6, INT_MAX, FLAGS }



typedef struct V4L2m2mContext {
char devname[PATH_MAX];
int fd;


V4L2Context capture;
V4L2Context output;


AVCodecContext *avctx;
sem_t refsync;
atomic_uint refcount;
int reinit;


int draining;
AVPacket buf_pkt;


AVBufferRef *self_ref;


void *priv;
} V4L2m2mContext;

typedef struct V4L2m2mPriv {
AVClass *class;

V4L2m2mContext *context;
AVBufferRef *context_ref;

int num_output_buffers;
int num_capture_buffers;
} V4L2m2mPriv;









int ff_v4l2_m2m_create_context(V4L2m2mPriv *priv, V4L2m2mContext **s);









int ff_v4l2_m2m_codec_init(V4L2m2mPriv *priv);










int ff_v4l2_m2m_codec_end(V4L2m2mPriv *priv);









int ff_v4l2_m2m_codec_reinit(V4L2m2mContext *ctx);









int ff_v4l2_m2m_codec_full_reinit(V4L2m2mContext *ctx);

#endif 
