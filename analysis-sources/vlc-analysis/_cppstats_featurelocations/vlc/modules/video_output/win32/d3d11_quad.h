





















#if !defined(VLC_D3D11_QUAD_H)
#define VLC_D3D11_QUAD_H

#include "../../video_chroma/d3d11_fmt.h"
#include "d3d11_shaders.h"

#define SPHERE_RADIUS 1.f

#define PS_CONST_LUMI_BOUNDS 0
#define PS_CONST_COLORSPACE 1
#define PS_CONST_COUNT 2


typedef struct d3d_vertex_t {
struct {
FLOAT x;
FLOAT y;
FLOAT z;
} position;
struct {
FLOAT x;
FLOAT y;
} texture;
} d3d_vertex_t;

typedef bool (*d3d11_select_plane_t)(void *opaque, size_t plane_index);

void D3D11_RenderQuad(d3d11_device_t *, d3d_quad_t *, d3d_vshader_t *,
ID3D11ShaderResourceView *resourceViews[D3D11_MAX_SHADER_VIEW],
d3d11_select_plane_t selectPlane, void *selectOpaque);

int D3D11_AllocateQuad(vlc_object_t *, d3d11_device_t *, video_projection_mode_t, d3d_quad_t *);
#define D3D11_AllocateQuad(a,b,c,d) D3D11_AllocateQuad(VLC_OBJECT(a),b,c,d)

void D3D11_ReleaseQuad(d3d_quad_t *);

int D3D11_SetupQuad(vlc_object_t *, d3d11_device_t *, const video_format_t *, d3d_quad_t *,
const display_info_t *, const RECT *,
video_orientation_t);
#define D3D11_SetupQuad(a,b,c,d,e,f,g) D3D11_SetupQuad(VLC_OBJECT(a),b,c,d,e,f,g)

bool D3D11_UpdateQuadPosition( vlc_object_t *, d3d11_device_t *, d3d_quad_t *,
const RECT *output, video_orientation_t );
#define D3D11_UpdateQuadPosition(a,b,c,d,e) D3D11_UpdateQuadPosition(VLC_OBJECT(a),b,c,d,e)

void D3D11_UpdateViewport(d3d_quad_t *, const RECT *, const d3d_format_t *display);

void D3D11_UpdateQuadOpacity(vlc_object_t *, d3d11_device_t *, d3d_quad_t *, float opacity);
#define D3D11_UpdateQuadOpacity(a,b,c,d) D3D11_UpdateQuadOpacity(VLC_OBJECT(a),b,c,d)

void D3D11_UpdateQuadLuminanceScale(vlc_object_t *, d3d11_device_t *, d3d_quad_t *, float luminanceScale);
#define D3D11_UpdateQuadLuminanceScale(a,b,c,d) D3D11_UpdateQuadLuminanceScale(VLC_OBJECT(a),b,c,d)

#endif 
