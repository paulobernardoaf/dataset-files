#include <vlc_common.h>
#include <vlc_codec.h>
#include "../../video_chroma/d3d11_fmt.h"
void *CreateLocalSwapchainHandle(vlc_object_t *, HWND, d3d11_device_t *d3d_dev);
void LocalSwapchainCleanupDevice( void *opaque );
void LocalSwapchainSwap( void *opaque );
bool LocalSwapchainUpdateOutput( void *opaque, const libvlc_video_render_cfg_t *cfg, libvlc_video_output_cfg_t *out );
bool LocalSwapchainStartEndRendering( void *opaque, bool enter );
void LocalSwapchainSetMetadata( void *opaque, libvlc_video_metadata_type_t, const void * );
bool LocalSwapchainSelectPlane( void *opaque, size_t plane );
