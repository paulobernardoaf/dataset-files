





















#if !defined(VLC_D3D11_PROCESSOR_H)
#define VLC_D3D11_PROCESSOR_H

#include <vlc_common.h>

#include "../../video_chroma/d3d11_fmt.h"

#if defined(ID3D11VideoContext_VideoProcessorBlt)
typedef struct
{
ID3D11VideoDevice *d3dviddev;
ID3D11VideoContext *d3dvidctx;
ID3D11VideoProcessorEnumerator *procEnumerator;
ID3D11VideoProcessor *videoProcessor;
} d3d11_processor_t;

int D3D11_CreateProcessor(vlc_object_t *, d3d11_device_t *,
D3D11_VIDEO_FRAME_FORMAT srcFields,
const video_format_t *fmt_in, const video_format_t *fmt_out,
d3d11_processor_t *out);
#define D3D11_CreateProcessor(a,b,c,d,e,f) D3D11_CreateProcessor(VLC_OBJECT(a),b,c,d,e,f)

void D3D11_ReleaseProcessor(d3d11_processor_t *);

HRESULT D3D11_Assert_ProcessorInput(vlc_object_t *, d3d11_processor_t *, picture_sys_d3d11_t *);
#define D3D11_Assert_ProcessorInput(a,b,c) D3D11_Assert_ProcessorInput(VLC_OBJECT(a),b,c)
#endif

#endif 
