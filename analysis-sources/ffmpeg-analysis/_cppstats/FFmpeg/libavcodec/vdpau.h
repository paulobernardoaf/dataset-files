#include <vdpau/vdpau.h>
#include "libavutil/avconfig.h"
#include "libavutil/attributes.h"
#include "avcodec.h"
#include "version.h"
struct AVCodecContext;
struct AVFrame;
typedef int (*AVVDPAU_Render2)(struct AVCodecContext *, struct AVFrame *,
const VdpPictureInfo *, uint32_t,
const VdpBitstreamBuffer *);
typedef struct AVVDPAUContext {
VdpDecoder decoder;
VdpDecoderRender *render;
AVVDPAU_Render2 render2;
} AVVDPAUContext;
AVVDPAUContext *av_alloc_vdpaucontext(void);
AVVDPAU_Render2 av_vdpau_hwaccel_get_render2(const AVVDPAUContext *);
void av_vdpau_hwaccel_set_render2(AVVDPAUContext *, AVVDPAU_Render2);
int av_vdpau_bind_context(AVCodecContext *avctx, VdpDevice device,
VdpGetProcAddress *get_proc_address, unsigned flags);
int av_vdpau_get_surface_parameters(AVCodecContext *avctx, VdpChromaType *type,
uint32_t *width, uint32_t *height);
AVVDPAUContext *av_vdpau_alloc_context(void);
#if FF_API_VDPAU_PROFILE
attribute_deprecated
int av_vdpau_get_profile(AVCodecContext *avctx, VdpDecoderProfile *profile);
#endif
