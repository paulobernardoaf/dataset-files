



















#if !defined(VLC_CODEC_VTUTILS_H_)
#define VLC_CODEC_VTUTILS_H_

#include <VideoToolbox/VideoToolbox.h>
#include <vlc_picture.h>

CFMutableDictionaryRef cfdict_create(CFIndex capacity);

void cfdict_set_int32(CFMutableDictionaryRef dict, CFStringRef key, int value);







int cvpxpic_attach(picture_t *p_pic, CVPixelBufferRef cvpx, vlc_video_context *vctx,
void (*on_released_cb)(vlc_video_context *vctx, unsigned));




CVPixelBufferRef cvpxpic_get_ref(picture_t *pic);










picture_t *cvpxpic_create_mapped(const video_format_t *fmt,
CVPixelBufferRef cvpx, vlc_video_context *vctx,
bool readonly);







picture_t *cvpxpic_unmap(picture_t *mapped_pic);








CVPixelBufferPoolRef cvpxpool_create(const video_format_t *fmt, unsigned count);




CVPixelBufferRef cvpxpool_new_cvpx(CVPixelBufferPoolRef pool);

enum cvpx_video_context_type
{
CVPX_VIDEO_CONTEXT_DEFAULT,
CVPX_VIDEO_CONTEXT_VIDEOTOOLBOX,
CVPX_VIDEO_CONTEXT_CIFILTERS,
};






vlc_video_context *
vlc_video_context_CreateCVPX(vlc_decoder_device *device,
enum cvpx_video_context_type type, size_t type_size,
const struct vlc_video_context_operations *ops);




void *
vlc_video_context_GetCVPXPrivate(vlc_video_context *vctx,
enum cvpx_video_context_type type);

#endif
