





























#if !defined(__MFX_LOAD_DLL_H)
#define __MFX_LOAD_DLL_H

#include "mfx_dispatcher.h"

namespace MFX
{






mfxStatus mfx_get_rt_dll_name(msdk_disp_char *pPath, size_t pathSize);
mfxStatus mfx_get_default_dll_name(msdk_disp_char *pPath, size_t pathSize, eMfxImplType implType);
mfxStatus mfx_get_default_plugin_name(msdk_disp_char *pPath, size_t pathSize, eMfxImplType implType);

mfxStatus mfx_get_default_audio_dll_name(msdk_disp_char *pPath, size_t pathSize, eMfxImplType implType);


mfxModuleHandle mfx_dll_load(const msdk_disp_char *file_name);

mfxModuleHandle mfx_get_dll_handle(const msdk_disp_char *file_name);
mfxFunctionPointer mfx_dll_get_addr(mfxModuleHandle handle, const char *func_name);
bool mfx_dll_free(mfxModuleHandle handle);

} 

#endif 
