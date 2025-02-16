#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#define COBJMACROS
#define INITGUID
#if !defined(UNICODE)
#define UNICODE
#endif
#include <vlc_common.h>
#if defined(__MINGW32__)
#include <w32api.h>
#endif
#include <direct.h>
#include <shlobj.h>
#include "../libvlc.h"
#include <vlc_charset.h>
#include <vlc_configuration.h>
#include "config/configuration.h"
#include <assert.h>
#include <limits.h>
#if VLC_WINSTORE_APP
#include <winstring.h>
#include <windows.storage.h>
#include <roapi.h>
#if !defined(CSIDL_LOCAL_APPDATA)
#define CSIDL_LOCAL_APPDATA 0x001C
#endif
static HRESULT WinRTSHGetFolderPath(HWND hwnd, int csidl, HANDLE hToken, DWORD dwFlags, LPWSTR pszPath)
{
VLC_UNUSED(hwnd);
VLC_UNUSED(hToken);
HRESULT hr;
IStorageFolder *folder;
if (dwFlags != SHGFP_TYPE_CURRENT)
return E_NOTIMPL;
folder = NULL;
csidl &= ~0xFF00; 
if (csidl == CSIDL_APPDATA) {
IApplicationDataStatics *appDataStatics = NULL;
IApplicationData *appData = NULL;
static const WCHAR *className = L"Windows.Storage.ApplicationData";
const UINT32 clen = wcslen(className);
HSTRING hClassName = NULL;
HSTRING_HEADER header;
hr = WindowsCreateStringReference(className, clen, &header, &hClassName);
if (FAILED(hr))
goto end_appdata;
hr = RoGetActivationFactory(hClassName, &IID_IApplicationDataStatics, (void**)&appDataStatics);
if (FAILED(hr))
goto end_appdata;
if (!appDataStatics) {
hr = E_FAIL;
goto end_appdata;
}
hr = IApplicationDataStatics_get_Current(appDataStatics, &appData);
if (FAILED(hr))
goto end_appdata;
if (!appData) {
hr = E_FAIL;
goto end_appdata;
}
hr = IApplicationData_get_LocalFolder(appData, &folder);
end_appdata:
WindowsDeleteString(hClassName);
if (appDataStatics)
IApplicationDataStatics_Release(appDataStatics);
if (appData)
IApplicationData_Release(appData);
}
else
{
IKnownFoldersStatics *knownFoldersStatics = NULL;
static const WCHAR *className = L"Windows.Storage.KnownFolders";
const UINT32 clen = wcslen(className);
HSTRING hClassName = NULL;
HSTRING_HEADER header;
hr = WindowsCreateStringReference(className, clen, &header, &hClassName);
if (FAILED(hr))
goto end_other;
hr = RoGetActivationFactory(hClassName, &IID_IKnownFoldersStatics, (void**)&knownFoldersStatics);
if (FAILED(hr))
goto end_other;
if (!knownFoldersStatics) {
hr = E_FAIL;
goto end_other;
}
switch (csidl) {
case CSIDL_PERSONAL:
hr = IKnownFoldersStatics_get_DocumentsLibrary(knownFoldersStatics, &folder);
break;
case CSIDL_MYMUSIC:
hr = IKnownFoldersStatics_get_MusicLibrary(knownFoldersStatics, &folder);
break;
case CSIDL_MYPICTURES:
hr = IKnownFoldersStatics_get_PicturesLibrary(knownFoldersStatics, &folder);
break;
case CSIDL_MYVIDEO:
hr = IKnownFoldersStatics_get_VideosLibrary(knownFoldersStatics, &folder);
break;
default:
hr = E_NOTIMPL;
}
end_other:
WindowsDeleteString(hClassName);
if (knownFoldersStatics)
IKnownFoldersStatics_Release(knownFoldersStatics);
}
if( SUCCEEDED(hr) && folder != NULL )
{
HSTRING path = NULL;
IStorageItem *item = NULL;
PCWSTR pszPathTemp;
hr = IStorageFolder_QueryInterface(folder, &IID_IStorageItem, (void**)&item);
if (FAILED(hr))
goto end_folder;
hr = IStorageItem_get_Path(item, &path);
if (FAILED(hr))
goto end_folder;
pszPathTemp = WindowsGetStringRawBuffer(path, NULL);
wcscpy(pszPath, pszPathTemp);
end_folder:
WindowsDeleteString(path);
IStorageFolder_Release(folder);
if (item)
IStorageItem_Release(item);
}
return hr;
}
#define SHGetFolderPathW WinRTSHGetFolderPath
#endif
char *config_GetLibDir (void)
{
#if VLC_WINSTORE_APP
return NULL;
#else
MEMORY_BASIC_INFORMATION mbi;
if (!VirtualQuery (config_GetLibDir, &mbi, sizeof(mbi)))
goto error;
wchar_t wpath[MAX_PATH];
if (!GetModuleFileName ((HMODULE) mbi.AllocationBase, wpath, MAX_PATH))
goto error;
wchar_t *file = wcsrchr (wpath, L'\\');
if (file == NULL)
goto error;
*file = L'\0';
return FromWide (wpath);
error:
abort ();
#endif
}
static char *config_GetDataDir(void)
{
const char *path = getenv ("VLC_DATA_PATH");
return (path != NULL) ? strdup (path) : config_GetLibDir ();
}
char *config_GetSysPath(vlc_sysdir_t type, const char *filename)
{
char *dir = NULL;
switch (type)
{
case VLC_PKG_DATA_DIR:
dir = config_GetDataDir();
break;
case VLC_PKG_LIB_DIR:
case VLC_PKG_LIBEXEC_DIR:
dir = config_GetLibDir();
break;
case VLC_SYSDATA_DIR:
break;
case VLC_LOCALE_DIR:
dir = config_GetSysPath(VLC_PKG_DATA_DIR, "locale");
break;
default:
vlc_assert_unreachable();
}
if (filename == NULL || unlikely(dir == NULL))
return dir;
char *path;
if (unlikely(asprintf(&path, "%s/%s", dir, filename) == -1))
path = NULL;
free(dir);
return path;
}
static char *config_GetShellDir (int csidl)
{
wchar_t wdir[MAX_PATH];
if (SHGetFolderPathW (NULL, csidl | CSIDL_FLAG_CREATE,
NULL, SHGFP_TYPE_CURRENT, wdir ) == S_OK)
return FromWide (wdir);
return NULL;
}
static char *config_GetAppDir (void)
{
#if !VLC_WINSTORE_APP
WCHAR path[MAX_PATH];
if (GetModuleFileName (NULL, path, MAX_PATH))
{
WCHAR *lastDir = wcsrchr (path, TEXT('\\'));
if (lastDir)
{
wcscpy (lastDir + 1, TEXT("portable"));
DWORD attrib = GetFileAttributes (path);
if (attrib != INVALID_FILE_ATTRIBUTES &&
(attrib & FILE_ATTRIBUTE_DIRECTORY))
return FromWide (path);
}
}
#endif
char *psz_dir;
char *psz_parent = config_GetShellDir (CSIDL_APPDATA);
if (psz_parent == NULL
|| asprintf (&psz_dir, "%s\\vlc", psz_parent) == -1)
psz_dir = NULL;
free (psz_parent);
return psz_dir;
}
#warning FIXME Use known folders on Vista and above
char *config_GetUserDir (vlc_userdir_t type)
{
switch (type)
{
case VLC_HOME_DIR:
return config_GetShellDir (CSIDL_PERSONAL);
case VLC_CONFIG_DIR:
case VLC_USERDATA_DIR:
return config_GetAppDir ();
case VLC_CACHE_DIR:
#if !VLC_WINSTORE_APP
return config_GetAppDir ();
#else
return config_GetShellDir (CSIDL_LOCAL_APPDATA);
#endif
case VLC_DESKTOP_DIR:
case VLC_DOWNLOAD_DIR:
case VLC_TEMPLATES_DIR:
case VLC_PUBLICSHARE_DIR:
case VLC_DOCUMENTS_DIR:
return config_GetUserDir(VLC_HOME_DIR);
case VLC_MUSIC_DIR:
return config_GetShellDir (CSIDL_MYMUSIC);
case VLC_PICTURES_DIR:
return config_GetShellDir (CSIDL_MYPICTURES);
case VLC_VIDEOS_DIR:
return config_GetShellDir (CSIDL_MYVIDEO);
}
vlc_assert_unreachable ();
}
