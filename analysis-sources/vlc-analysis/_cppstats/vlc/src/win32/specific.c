#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#if !defined(UNICODE)
#define UNICODE
#endif
#include <vlc_common.h>
#include "libvlc.h"
#include "../lib/libvlc_internal.h"
#include "config/vlc_getopt.h"
#include <mmsystem.h>
#include <winsock2.h>
static int system_InitWSA(int hi, int lo)
{
WSADATA data;
if (WSAStartup(MAKEWORD(hi, lo), &data) == 0)
{
if (LOBYTE(data.wVersion) == 2 && HIBYTE(data.wVersion) == 2)
return 0;
WSACleanup( );
}
return -1;
}
void system_Init(void)
{
if (system_InitWSA(2, 2) && system_InitWSA(1, 1))
fputs("Error: cannot initialize Winsocks\n", stderr);
}
typedef struct
{
int argc;
int enqueue;
char data[];
} vlc_ipc_data_t;
void system_Configure( libvlc_int_t *p_this, int i_argc, const char *const ppsz_argv[] )
{
#if !VLC_WINSTORE_APP
if( var_InheritBool( p_this, "one-instance" )
|| ( var_InheritBool( p_this, "one-instance-when-started-from-file" )
&& var_InheritBool( p_this, "started-from-file" ) ) )
{
HANDLE hmutex;
msg_Info( p_this, "one instance mode ENABLED");
if( !( hmutex = CreateMutex( 0, TRUE, L"VLC ipc " TEXT(VERSION) ) ) )
{
msg_Err( p_this, "one instance mode DISABLED "
"(mutex couldn't be created)" );
return;
}
if( GetLastError() != ERROR_ALREADY_EXISTS )
{
libvlc_InternalAddIntf( p_this, "win_msg,none" );
ReleaseMutex( hmutex );
}
else
{
HWND ipcwindow;
WaitForSingleObject( hmutex, INFINITE );
if( !( ipcwindow = FindWindow( 0, L"VLC ipc " TEXT(VERSION) ) ) )
{
msg_Err( p_this, "one instance mode DISABLED "
"(couldn't find 1st instance of program)" );
ReleaseMutex( hmutex );
return;
}
if( i_argc > 0 )
{
COPYDATASTRUCT wm_data;
int i_opt;
vlc_ipc_data_t *p_data;
size_t i_data = sizeof (*p_data);
for( i_opt = 0; i_opt < i_argc; i_opt++ )
{
i_data += sizeof (size_t);
i_data += strlen( ppsz_argv[ i_opt ] ) + 1;
}
p_data = malloc( i_data );
p_data->argc = i_argc;
p_data->enqueue = var_InheritBool( p_this, "playlist-enqueue" );
i_data = 0;
for( i_opt = 0; i_opt < i_argc; i_opt++ )
{
size_t i_len = strlen( ppsz_argv[ i_opt ] ) + 1;
*((size_t *)(p_data->data + i_data)) = i_len;
i_data += sizeof (size_t);
memcpy( &p_data->data[i_data], ppsz_argv[ i_opt ], i_len );
i_data += i_len;
}
i_data += sizeof (*p_data);
wm_data.dwData = 0;
wm_data.cbData = i_data;
wm_data.lpData = p_data;
SendMessage( ipcwindow, WM_COPYDATA, 0, (LPARAM)&wm_data );
}
ReleaseMutex( hmutex );
system_End( );
exit( 0 );
}
}
#endif
}
void system_End(void)
{
WSACleanup();
}
