#pragma once
#include "mfxdefs.h"
#include <cstring>
#if defined(MFX_DISPATCHER_LOG)
#include <string>
#include <string.h>
#endif
#define MAX_PLUGIN_PATH 4096
#define MAX_PLUGIN_NAME 4096
#if defined(_WIN32) || defined(_WIN64)
typedef wchar_t msdk_disp_char;
#define MSDK2WIDE(x) x
#if _MSC_VER >= 1400
#define msdk_disp_char_cpy_s(to, to_size, from) wcscpy_s(to,to_size, from)
#else
#define msdk_disp_char_cpy_s(to, to_size, from) wcscpy(to, from)
#endif
#else
typedef char msdk_disp_char;
inline void msdk_disp_char_cpy_s(char * to, size_t to_size, const char * from)
{
size_t source_len = strlen(from);
size_t num_chars = (to_size - 1) < source_len ? (to_size - 1) : source_len;
strncpy(to, from, num_chars);
to[num_chars] = 0;
}
#if defined(MFX_DISPATCHER_LOG)
#define MSDK2WIDE(x) getWideString(x).c_str()
inline std::wstring getWideString(const char * string)
{
size_t len = strlen(string);
return std::wstring(string, string + len);
}
#else
#define MSDK2WIDE(x) x 
#endif
#endif
#if defined(__GNUC__) && !defined(_WIN32) && !defined(_WIN64)
#define sscanf_s sscanf
#define swscanf_s swscanf
#endif
typedef void * mfxModuleHandle;
typedef void (MFX_CDECL * mfxFunctionPointer)(void);
