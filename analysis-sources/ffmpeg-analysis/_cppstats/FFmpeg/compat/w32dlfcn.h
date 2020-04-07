#if defined(_WIN32)
#include <windows.h>
#include "config.h"
#if (_WIN32_WINNT < 0x0602) || HAVE_WINRT
#include "libavutil/wchar_filename.h"
#endif
static inline HMODULE win32_dlopen(const char *name)
{
#if _WIN32_WINNT < 0x0602
if (!GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "SetDefaultDllDirectories")) {
HMODULE module = NULL;
wchar_t *path = NULL, *name_w = NULL;
DWORD pathlen;
if (utf8towchar(name, &name_w))
goto exit;
path = (wchar_t *)av_mallocz_array(MAX_PATH, sizeof(wchar_t));
pathlen = GetModuleFileNameW(NULL, path, MAX_PATH);
pathlen = wcsrchr(path, '\\') - path;
if (pathlen == 0 || pathlen + wcslen(name_w) + 2 > MAX_PATH)
goto exit;
path[pathlen] = '\\';
wcscpy(path + pathlen + 1, name_w);
module = LoadLibraryExW(path, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
if (module == NULL) {
pathlen = GetSystemDirectoryW(path, MAX_PATH);
if (pathlen == 0 || pathlen + wcslen(name_w) + 2 > MAX_PATH)
goto exit;
path[pathlen] = '\\';
wcscpy(path + pathlen + 1, name_w);
module = LoadLibraryExW(path, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
}
exit:
av_free(path);
av_free(name_w);
return module;
}
#endif
#if !defined(LOAD_LIBRARY_SEARCH_APPLICATION_DIR)
#define LOAD_LIBRARY_SEARCH_APPLICATION_DIR 0x00000200
#endif
#if !defined(LOAD_LIBRARY_SEARCH_SYSTEM32)
#define LOAD_LIBRARY_SEARCH_SYSTEM32 0x00000800
#endif
#if HAVE_WINRT
wchar_t *name_w = NULL;
int ret;
if (utf8towchar(name, &name_w))
return NULL;
ret = LoadPackagedLibrary(name_w, 0);
av_free(name_w);
return ret;
#else
return LoadLibraryExA(name, NULL, LOAD_LIBRARY_SEARCH_APPLICATION_DIR | LOAD_LIBRARY_SEARCH_SYSTEM32);
#endif
}
#define dlopen(name, flags) win32_dlopen(name)
#define dlclose FreeLibrary
#define dlsym GetProcAddress
#else
#include <dlfcn.h>
#endif
