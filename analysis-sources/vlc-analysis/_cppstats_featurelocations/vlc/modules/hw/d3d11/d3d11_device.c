

























#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <vlc_common.h>
#include <vlc_codec.h>

#include "d3d11_filters.h"

static void D3D11CloseDecoderDevice(vlc_decoder_device *device)
{
d3d11_decoder_device_t *dec_device = device->opaque;
D3D11_ReleaseDevice( dec_device );
}

static const struct vlc_decoder_device_operations d3d11_dev_ops = {
.close = D3D11CloseDecoderDevice,
};

static int D3D11OpenDecoderDevice(vlc_decoder_device *device, bool forced, vout_window_t *wnd)
{
VLC_UNUSED(wnd);

d3d11_decoder_device_t *dec_device;
dec_device = D3D11_CreateDevice( device, NULL, true ,
forced );
if ( dec_device == NULL )
return VLC_EGENERIC;

device->ops = &d3d11_dev_ops;
device->opaque = dec_device;
device->type = VLC_DECODER_DEVICE_D3D11VA;
device->sys = NULL;

return VLC_SUCCESS;
}

int D3D11OpenDecoderDeviceW8(vlc_decoder_device *device, vout_window_t *wnd)
{
return D3D11OpenDecoderDevice(device, false, wnd);
}

int D3D11OpenDecoderDeviceAny(vlc_decoder_device *device, vout_window_t *wnd)
{
return D3D11OpenDecoderDevice(device, true, wnd);
}
