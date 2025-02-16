#include <gst/gstbufferpool.h>
#include <gst/video/gstvideopool.h>
#include "gstvlcpictureplaneallocator.h"
typedef struct _GstVlcVideoPool GstVlcVideoPool;
typedef struct _GstVlcVideoPoolClass GstVlcVideoPoolClass;
#define GST_TYPE_VLC_VIDEO_POOL (gst_vlc_video_pool_get_type())
#define GST_IS_VLC_VIDEO_POOL(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GST_TYPE_VLC_VIDEO_POOL))
#define GST_VLC_VIDEO_POOL(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), GST_TYPE_VLC_VIDEO_POOL, GstVlcVideoPool))
#define GST_VLC_VIDEO_POOL_CAST(obj) ((GstVlcVideoPool*)(obj))
struct _GstVlcVideoPool
{
GstBufferPool bufferpool;
GstVlcPicturePlaneAllocator *p_allocator;
GstCaps *p_caps;
GstVideoInfo info;
GstVideoAlignment align;
bool b_add_metavideo;
bool b_need_aligned;
decoder_t *p_dec;
};
struct _GstVlcVideoPoolClass
{
GstBufferPoolClass parent_class;
};
GType gst_vlc_video_pool_get_type( void );
GstVlcVideoPool* gst_vlc_video_pool_new(
GstAllocator *p_allocator, decoder_t *p_dec );
