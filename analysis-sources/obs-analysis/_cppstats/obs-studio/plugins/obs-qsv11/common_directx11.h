#pragma once
#include "common_utils.h"
#include <windows.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <atlbase.h>
#define DEVICE_MGR_TYPE MFX_HANDLE_D3D11_DEVICE
mfxStatus CreateHWDevice(mfxSession session, mfxHDL *deviceHandle, HWND hWnd,
bool bCreateSharedHandles);
void CleanupHWDevice();
void SetHWDeviceContext(CComPtr<ID3D11DeviceContext> devCtx);
CComPtr<ID3D11DeviceContext> GetHWDeviceContext();
void ClearYUVSurfaceD3D(mfxMemId memId);
void ClearRGBSurfaceD3D(mfxMemId memId);
