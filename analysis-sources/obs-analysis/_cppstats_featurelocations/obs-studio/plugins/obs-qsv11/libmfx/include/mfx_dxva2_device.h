





























#if !defined(__MFX_DXVA2_DEVICE_H)
#define __MFX_DXVA2_DEVICE_H

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>

#define TOSTRING(L) #L
#define STRINGIFY(L) TOSTRING(L)

#if defined(MEDIASDK_UWP_LOADER) || defined(MEDIASDK_UWP_PROCTABLE)
#if defined(MFX_D3D9_ENABLED) && !defined(MFX_FORCE_D3D9_ENABLED)
#undef MFX_D3D9_ENABLED

#pragma message("\n\nATTENTION:\nin file\n\t" __FILE__ " (" STRINGIFY(__LINE__) "):\nUsing of D3D9 disabled for UWP!\n\n")
#endif
#if defined(MFX_FORCE_D3D9_ENABLED)
#define MFX_D3D9_ENABLED
#endif
#else
#define MFX_D3D9_ENABLED
#pragma message("\n\nATTENTION:\nin file\n\t" __FILE__ " (" STRINGIFY(__LINE__) "):\nUsing of D3D9 enabled!\n\n")
#endif

#endif 

#include <mfxdefs.h>

#if defined(DXVA2DEVICE_LOG)
#include <stdio.h>
#define DXVA2DEVICE_TRACE(expr) printf expr;
#define DXVA2DEVICE_TRACE_OPERATION(expr) expr;
#else
#define DXVA2DEVICE_TRACE(expr)
#define DXVA2DEVICE_TRACE_OPERATION(expr)
#endif

namespace MFX
{

class DXDevice
{
public:

DXDevice(void);

virtual
~DXDevice(void) = 0;


virtual
bool Init(const mfxU32 adapterNum) = 0;


mfxU32 GetVendorID(void) const;
mfxU32 GetDeviceID(void) const;
mfxU64 GetDriverVersion(void) const;
mfxU64 GetLUID(void) const;


mfxU32 GetAdapterCount(void) const;


virtual
void Close(void);


void LoadDLLModule(const wchar_t *pModuleName);

protected:


void UnloadDLLModule(void);

#if defined(_WIN32) || defined(_WIN64)

HMODULE m_hModule;
#endif 


mfxU32 m_numAdapters;


mfxU32 m_vendorID;

mfxU32 m_deviceID;

mfxU64 m_driverVersion;

mfxU64 m_luid;

private:

DXDevice(const DXDevice &);
void operator=(const DXDevice &);
};


#if defined(_WIN32) || defined(_WIN64)

#if defined(MFX_D3D9_ENABLED)
class D3D9Device : public DXDevice
{
public:

D3D9Device(void);

virtual
~D3D9Device(void);


virtual
bool Init(const mfxU32 adapterNum);


virtual
void Close(void);

protected:


void *m_pD3D9;

void *m_pD3D9Ex;

};
#endif 

class DXGI1Device : public DXDevice
{
public:

DXGI1Device(void);

virtual
~DXGI1Device(void);


virtual
bool Init(const mfxU32 adapterNum);


virtual
void Close(void);

protected:


void *m_pDXGIFactory1;

void *m_pDXGIAdapter1;

};
#endif 

class DXVA2Device
{
public:

DXVA2Device(void);

~DXVA2Device(void);


bool InitD3D9(const mfxU32 adapterNum);


bool InitDXGI1(const mfxU32 adapterNum);


mfxU32 GetVendorID(void) const;
mfxU32 GetDeviceID(void) const;
mfxU64 GetDriverVersion(void) const;


mfxU32 GetAdapterCount(void) const;

void Close(void);

protected:

#if defined(MFX_D3D9_ENABLED)

void UseAlternativeWay(const D3D9Device *pD3D9Device);
#endif 

mfxU32 m_numAdapters;


mfxU32 m_vendorID;

mfxU32 m_deviceID;

mfxU64 m_driverVersion;

private:

DXVA2Device(const DXVA2Device &);
void operator=(const DXVA2Device &);
};

} 

#endif 
