









#pragma once

#include "common_utils.h"
#include <initguid.h>
#include <d3d9.h>
#include <dxva2api.h>
#include <dxva.h>
#include <windows.h>

#define VIDEO_MAIN_FORMAT D3DFMT_YUY2

class IGFXS3DControl;







template<class T> class safe_array {
public:
safe_array(T *ptr = 0)
: m_ptr(ptr){

};
~safe_array() { reset(0); }
T *get()
{ 
return m_ptr;
}
T *release()
{ 
T *ptr = m_ptr;
m_ptr = 0;
return ptr;
}
void reset(T *ptr)
{ 
if (m_ptr) {
delete[] m_ptr;
}
m_ptr = ptr;
}

protected:
T *m_ptr; 
};

mfxStatus dx9_simple_alloc(mfxHDL pthis, mfxFrameAllocRequest *request,
mfxFrameAllocResponse *response);
mfxStatus dx9_simple_lock(mfxHDL pthis, mfxMemId mid, mfxFrameData *ptr);
mfxStatus dx9_simple_unlock(mfxHDL pthis, mfxMemId mid, mfxFrameData *ptr);
mfxStatus dx9_simple_gethdl(mfxHDL pthis, mfxMemId mid, mfxHDL *handle);
mfxStatus dx9_simple_free(mfxHDL pthis, mfxFrameAllocResponse *response);

mfxStatus DX9_CreateHWDevice(mfxSession session, mfxHDL *deviceHandle,
HWND hWnd, bool bCreateSharedHandles);
void DX9_CleanupHWDevice();
