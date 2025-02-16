#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#define VLC_MODULE_LICENSE VLC_LICENSE_GPL_2_PLUS
#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_interface.h>
#include <vlc_playlist.h>
#include <vlc_url.h> 
#include <vlc_input_item.h>
#include <windows.h>
struct intf_sys_t
{
HWND window;
HANDLE ready;
vlc_thread_t thread;
};
typedef struct
{
int argc;
int enqueue;
char data[];
} vlc_ipc_data_t;
static void add_to_playlist(intf_thread_t *intf, const char *uri,
bool play_now, int options_count,
const char *const *options)
{
vlc_playlist_t *playlist = vlc_intf_GetMainPlaylist(intf);
input_item_t *media = input_item_New(uri, NULL);
if (!media)
return;
input_item_AddOptions(media, options_count, options, VLC_INPUT_OPTION_TRUSTED);
vlc_playlist_Lock(playlist);
vlc_playlist_AppendOne(playlist, media);
if (play_now)
vlc_playlist_Start(playlist);
vlc_playlist_Unlock(playlist);
input_item_Release(media);
}
static LRESULT CALLBACK WMCOPYWNDPROC(HWND hwnd, UINT uMsg,
WPARAM wParam, LPARAM lParam)
{
if( uMsg == WM_QUIT )
{
PostQuitMessage( 0 );
}
else if( uMsg == WM_COPYDATA )
{
COPYDATASTRUCT *pwm_data = (COPYDATASTRUCT*)lParam;
intf_thread_t *intf = (intf_thread_t *)(uintptr_t)
GetWindowLongPtr( hwnd, GWLP_USERDATA );
if( intf == NULL )
return 0; 
if( pwm_data->lpData )
{
char **ppsz_argv;
vlc_ipc_data_t *p_data = (vlc_ipc_data_t *)pwm_data->lpData;
size_t i_data = 0;
int i_argc = p_data->argc, i_opt, i_options;
ppsz_argv = vlc_alloc( i_argc, sizeof(char *) );
for( i_opt = 0; i_opt < i_argc; i_opt++ )
{
ppsz_argv[i_opt] = p_data->data + i_data + sizeof(size_t);
i_data += sizeof(size_t) + *((size_t *)(p_data->data + i_data));
}
for( i_opt = 0; i_opt < i_argc; i_opt++ )
{
i_options = 0;
while( i_opt + i_options + 1 < i_argc &&
*ppsz_argv[ i_opt + i_options + 1 ] == ':' )
{
i_options++;
}
#warning URI conversion must be done in calling process instead!
char *psz_URI = NULL;
if( strstr( ppsz_argv[i_opt], "://" ) == NULL )
psz_URI = vlc_path2uri( ppsz_argv[i_opt], NULL );
add_to_playlist(intf, (psz_URI != NULL) ? psz_URI : ppsz_argv[i_opt],
(i_opt == 0 && !p_data->enqueue), i_options,
(char const **)( i_options ? &ppsz_argv[i_opt+1] : NULL ));
i_opt += i_options;
free( psz_URI );
}
free( ppsz_argv );
}
}
return DefWindowProc( hwnd, uMsg, wParam, lParam );
}
static void *HelperThread(void *data)
{
intf_thread_t *intf = data;
intf_sys_t *sys = intf->p_sys;
HWND ipcwindow =
CreateWindow(L"STATIC", 
L"VLC ipc " TEXT(VERSION), 
0, 
0, 
0, 
0, 
0, 
NULL, 
NULL, 
GetModuleHandle(NULL), 
NULL) ; 
SetWindowLongPtr(ipcwindow, GWLP_WNDPROC, (LRESULT)WMCOPYWNDPROC);
SetWindowLongPtr(ipcwindow, GWLP_USERDATA, (uintptr_t)data);
sys->window = ipcwindow;
SetEvent(sys->ready);
MSG message;
while (GetMessage(&message, NULL, 0, 0))
{
TranslateMessage(&message);
DispatchMessage(&message);
}
return NULL;
}
static int Open(vlc_object_t *obj)
{
intf_thread_t *intf = (intf_thread_t *)obj;
intf_sys_t *sys = malloc(sizeof (*sys));
if (unlikely(sys == NULL))
return VLC_ENOMEM;
intf->p_sys = sys;
sys->ready = CreateEvent(NULL, FALSE, FALSE, NULL);
if (vlc_clone(&sys->thread, HelperThread, intf, VLC_THREAD_PRIORITY_LOW))
{
free(sys);
msg_Err(intf, "one instance mode DISABLED "
"(IPC helper thread couldn't be created)");
return VLC_ENOMEM;
}
WaitForSingleObject(sys->ready, INFINITE);
CloseHandle(sys->ready);
return VLC_SUCCESS;
}
static void Close(vlc_object_t *obj)
{
intf_thread_t *intf = (intf_thread_t *)obj;
intf_sys_t *sys = intf->p_sys;
SendMessage(sys->window, WM_QUIT, 0, 0);
vlc_join(sys->thread, NULL);
free(sys);
}
vlc_module_begin()
set_shortname(N_("WinMsg"))
set_description(N_("Windows messages interface"))
set_category(CAT_INTERFACE)
set_subcategory(SUBCAT_INTERFACE_CONTROL)
set_capability("interface", 0)
set_callbacks(Open, Close)
vlc_module_end()
