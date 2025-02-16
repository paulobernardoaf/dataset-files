#if !defined(_WIN32_WINNT) || _WIN32_WINNT < 0x0602
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0602
#endif
#include <stdint.h>
#include <d3d9.h>
#include <dxva2api.h>
#define FF_DXVA2_WORKAROUND_SCALING_LIST_ZIGZAG 1 
#define FF_DXVA2_WORKAROUND_INTEL_CLEARVIDEO 2 
struct dxva_context {
IDirectXVideoDecoder *decoder;
const DXVA2_ConfigPictureDecode *cfg;
unsigned surface_count;
LPDIRECT3DSURFACE9 *surface;
uint64_t workaround;
unsigned report_id;
};
