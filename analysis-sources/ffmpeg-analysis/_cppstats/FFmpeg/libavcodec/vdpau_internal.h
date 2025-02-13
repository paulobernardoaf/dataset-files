#include <stdint.h>
#include <vdpau/vdpau.h>
#include "libavutil/frame.h"
#include "libavutil/hwcontext.h"
#include "libavutil/hwcontext_vdpau.h"
#include "avcodec.h"
#include "vdpau.h"
static inline uintptr_t ff_vdpau_get_surface_id(AVFrame *pic)
{
return (uintptr_t)pic->data[3];
}
union VDPAUPictureInfo {
VdpPictureInfoH264 h264;
VdpPictureInfoMPEG1Or2 mpeg;
VdpPictureInfoVC1 vc1;
VdpPictureInfoMPEG4Part2 mpeg4;
#if defined(VDP_DECODER_PROFILE_H264_HIGH_444_PREDICTIVE)
VdpPictureInfoH264Predictive h264_predictive;
#endif
#if defined(VDP_DECODER_PROFILE_HEVC_MAIN)
VdpPictureInfoHEVC hevc;
#endif
#if defined(VDP_YCBCR_FORMAT_Y_U_V_444)
VdpPictureInfoHEVC444 hevc_444;
#endif
#if defined(VDP_DECODER_PROFILE_VP9_PROFILE_0)
VdpPictureInfoVP9 vp9;
#endif
};
typedef struct VDPAUHWContext {
AVVDPAUContext context;
VdpDevice device;
VdpGetProcAddress *get_proc_address;
char reset;
unsigned char flags;
} VDPAUHWContext;
typedef struct VDPAUContext {
VdpDevice device;
VdpDecoder decoder;
VdpGetProcAddress *get_proc_address;
VdpDecoderRender *render;
uint32_t width;
uint32_t height;
} VDPAUContext;
struct vdpau_picture_context {
union VDPAUPictureInfo info;
int bitstream_buffers_allocated;
int bitstream_buffers_used;
VdpBitstreamBuffer *bitstream_buffers;
};
int ff_vdpau_common_init(AVCodecContext *avctx, VdpDecoderProfile profile,
int level);
int ff_vdpau_common_uninit(AVCodecContext *avctx);
int ff_vdpau_common_start_frame(struct vdpau_picture_context *pic,
const uint8_t *buffer, uint32_t size);
int ff_vdpau_common_end_frame(AVCodecContext *avctx, AVFrame *frame,
struct vdpau_picture_context *pic);
int ff_vdpau_mpeg_end_frame(AVCodecContext *avctx);
int ff_vdpau_add_buffer(struct vdpau_picture_context *pic, const uint8_t *buf,
uint32_t buf_size);
int ff_vdpau_common_frame_params(AVCodecContext *avctx,
AVBufferRef *hw_frames_ctx);
