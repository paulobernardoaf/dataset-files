#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <vlc_common.h>
#include <vlc_charset.h>
#include "modules/modules.h"
#include <windows.h>
#include <wchar.h>
char *vlc_dlerror(void)
{
wchar_t wmsg[256];
int i = 0, i_error = GetLastError();
FormatMessageW( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
NULL, i_error, MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),
wmsg, 256, NULL );
while( !wmemchr( L"\r\n\0", wmsg[i], 3 ) )
i++;
snwprintf( wmsg + i, 256 - i, L" (error %i)", i_error );
return FromWide( wmsg );
}
void *vlc_dlopen(const char *psz_file, bool lazy)
{
wchar_t *wfile = ToWide (psz_file);
if (wfile == NULL)
return NULL;
HMODULE handle = NULL;
#if !VLC_WINSTORE_APP
DWORD mode;
if (SetThreadErrorMode (SEM_FAILCRITICALERRORS, &mode) != 0)
{
handle = LoadLibraryExW(wfile, NULL, LOAD_LIBRARY_SEARCH_SYSTEM32);
SetThreadErrorMode (mode, NULL);
}
#else
handle = LoadPackagedLibrary( wfile, 0 );
#endif
free (wfile);
(void) lazy;
return handle;
}
int vlc_dlclose(void *handle)
{
FreeLibrary( handle );
return 0;
}
void *vlc_dlsym(void *handle, const char *psz_function)
{
return (void *)GetProcAddress(handle, psz_function);
}
