#pragma once
#if defined(_WIN32) || defined(_WIN64)
#include "common_utils.h"
#pragma warning(disable : 4201)
#include <initguid.h>
#include <d3d9.h>
#include <dxva2api.h>
#include <dxva.h>
#include <windows.h>
#define VIDEO_MAIN_FORMAT D3DFMT_YUY2
class IGFXS3DControl;
class CHWDevice {
public:
virtual ~CHWDevice() {}
virtual mfxStatus Init(mfxHDL hWindow, mfxU16 nViews,
mfxU32 nAdapterNum) = 0;
virtual mfxStatus Reset() = 0;
virtual mfxStatus GetHandle(mfxHandleType type, mfxHDL *pHdl) = 0;
virtual mfxStatus SetHandle(mfxHandleType type, mfxHDL hdl) = 0;
virtual mfxStatus RenderFrame(mfxFrameSurface1 *pSurface,
mfxFrameAllocator *pmfxAlloc) = 0;
virtual void Close() = 0;
};
enum { MFX_HANDLE_GFXS3DCONTROL =
0x100, 
MFX_HANDLE_DEVICEWINDOW = 0x101 
}; 
class CD3D9Device : public CHWDevice {
public:
CD3D9Device();
virtual ~CD3D9Device();
virtual mfxStatus Init(mfxHDL hWindow, mfxU16 nViews,
mfxU32 nAdapterNum);
virtual mfxStatus Reset();
virtual mfxStatus GetHandle(mfxHandleType type, mfxHDL *pHdl);
virtual mfxStatus SetHandle(mfxHandleType type, mfxHDL hdl);
virtual mfxStatus RenderFrame(mfxFrameSurface1 *pSurface,
mfxFrameAllocator *pmfxAlloc);
virtual void UpdateTitle(double ) {}
virtual void Close();
void DefineFormat(bool isA2rgb10)
{
m_bIsA2rgb10 = (isA2rgb10) ? TRUE : FALSE;
}
protected:
mfxStatus CreateVideoProcessors();
bool CheckOverlaySupport();
virtual mfxStatus FillD3DPP(mfxHDL hWindow, mfxU16 nViews,
D3DPRESENT_PARAMETERS &D3DPP);
private:
IDirect3D9Ex *m_pD3D9;
IDirect3DDevice9Ex *m_pD3DD9;
IDirect3DDeviceManager9 *m_pDeviceManager9;
D3DPRESENT_PARAMETERS m_D3DPP;
UINT m_resetToken;
mfxU16 m_nViews;
IGFXS3DControl *m_pS3DControl;
D3DSURFACE_DESC m_backBufferDesc;
IDirectXVideoProcessorService *m_pDXVAVPS;
IDirectXVideoProcessor *m_pDXVAVP_Left;
IDirectXVideoProcessor *m_pDXVAVP_Right;
RECT m_targetRect;
DXVA2_VideoDesc m_VideoDesc;
DXVA2_VideoProcessBltParams m_BltParams;
DXVA2_VideoSample m_Sample;
BOOL m_bIsA2rgb10;
};
#endif
