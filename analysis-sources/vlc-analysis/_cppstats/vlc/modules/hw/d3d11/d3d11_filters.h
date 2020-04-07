#include <vlc_common.h>
#include <vlc_vout_display.h>
#include "../../video_chroma/d3d11_fmt.h"
int D3D11OpenDeinterlace(vlc_object_t *);
void D3D11CloseDeinterlace(vlc_object_t *);
int D3D11OpenConverter(vlc_object_t *);
void D3D11CloseConverter(vlc_object_t *);
int D3D11OpenCPUConverter(vlc_object_t *);
void D3D11CloseCPUConverter(vlc_object_t *);
int D3D11OpenDecoderDeviceW8(vlc_decoder_device *, vout_window_t *);
int D3D11OpenDecoderDeviceAny(vlc_decoder_device *, vout_window_t *);
