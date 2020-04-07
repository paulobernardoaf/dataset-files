



























#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include "win32touch.h"

#include <vlc_common.h>
#include <vlc_vout_display.h>

#include <stdatomic.h>
#include <windows.h>
#include <windowsx.h> 

#include "events.h"
#include "common.h"




struct event_thread_t
{
vlc_object_t *obj;


vlc_thread_t thread;
vlc_mutex_t lock;
vlc_cond_t wait;
bool b_ready;
atomic_bool b_done;
bool b_error;


bool is_projected;


win32_gesture_sys_t *p_gesture;

RECT window_area;


vout_window_t *parent_window;
WCHAR class_video[256];
HWND hparent;
HWND hvideownd;
};





static int Win32VoutCreateWindow( event_thread_t * );
static void Win32VoutCloseWindow ( event_thread_t * );









static void *EventThread( void *p_this )
{
event_thread_t *p_event = (event_thread_t *)p_this;
MSG msg;
int canc = vlc_savecancel ();


vlc_mutex_lock( &p_event->lock );



if( Win32VoutCreateWindow( p_event ) )
p_event->b_error = true;

p_event->b_ready = true;
vlc_cond_signal( &p_event->wait );

const bool b_error = p_event->b_error;
vlc_mutex_unlock( &p_event->lock );

if( b_error )
{
vlc_restorecancel( canc );
return NULL;
}



for( ;; )
{
if( !GetMessage( &msg, 0, 0, 0 ) || atomic_load( &p_event->b_done ) )
{
break;
}



TranslateMessage(&msg);
DispatchMessage(&msg);

} 

msg_Dbg( p_event->obj, "Win32 Vout EventThread terminating" );

Win32VoutCloseWindow( p_event );
vlc_restorecancel(canc);
return NULL;
}

event_thread_t *EventThreadCreate( vlc_object_t *obj, vout_window_t *parent_window)
{
if (parent_window->type != VOUT_WINDOW_TYPE_HWND)
return NULL;







msg_Dbg( obj, "creating Vout EventThread" );
event_thread_t *p_event = malloc( sizeof(*p_event) );
if( !p_event )
return NULL;

p_event->obj = obj;
vlc_mutex_init( &p_event->lock );
vlc_cond_init( &p_event->wait );
atomic_init( &p_event->b_done, false );

p_event->parent_window = parent_window;

_snwprintf( p_event->class_video, ARRAY_SIZE(p_event->class_video),
TEXT("VLC video output %p"), (void *)p_event );
return p_event;
}

void EventThreadDestroy( event_thread_t *p_event )
{
free( p_event );
}

int EventThreadStart( event_thread_t *p_event, event_hwnd_t *p_hwnd, const event_cfg_t *p_cfg )
{
p_event->is_projected = p_cfg->is_projected;
p_event->window_area.left = 0;
p_event->window_area.top = 0;
p_event->window_area.right = p_cfg->width;
p_event->window_area.bottom = p_cfg->height;

p_event->b_ready = false;
atomic_store( &p_event->b_done, false);
p_event->b_error = false;

if( vlc_clone( &p_event->thread, EventThread, p_event,
VLC_THREAD_PRIORITY_LOW ) )
{
msg_Err( p_event->obj, "cannot create Vout EventThread" );
return VLC_EGENERIC;
}

vlc_mutex_lock( &p_event->lock );
while( !p_event->b_ready )
vlc_cond_wait( &p_event->wait, &p_event->lock );
const bool b_error = p_event->b_error;
vlc_mutex_unlock( &p_event->lock );

if( b_error )
{
vlc_join( p_event->thread, NULL );
p_event->b_ready = false;
return VLC_EGENERIC;
}
msg_Dbg( p_event->obj, "Vout EventThread running" );


p_hwnd->parent_window = p_event->parent_window;
p_hwnd->hparent = p_event->hparent;
p_hwnd->hvideownd = p_event->hvideownd;
return VLC_SUCCESS;
}

void EventThreadStop( event_thread_t *p_event )
{
if( !p_event->b_ready )
return;

atomic_store( &p_event->b_done, true );



if( p_event->hvideownd )
PostMessage( p_event->hvideownd, WM_NULL, 0, 0);

vlc_join( p_event->thread, NULL );
p_event->b_ready = false;
}





static long FAR PASCAL VideoEventProc( HWND hwnd, UINT message,
WPARAM wParam, LPARAM lParam )
{
if( message == WM_CREATE )
{

CREATESTRUCT *c = (CREATESTRUCT *)lParam;
SetWindowLongPtr( hwnd, GWLP_USERDATA, (LONG_PTR)c->lpCreateParams );
return 0;
}

LONG_PTR p_user_data = GetWindowLongPtr( hwnd, GWLP_USERDATA );
if( p_user_data == 0 ) 
return DefWindowProc(hwnd, message, wParam, lParam);
event_thread_t *p_event = (event_thread_t *)p_user_data;


return DefWindowProc(hwnd, message, wParam, lParam);
switch( message )
{

case WM_CLOSE:
vout_window_ReportClose(p_event->parent_window);
return 0;


case WM_DESTROY:
msg_Dbg( p_event->obj, "WinProc WM_DESTROY" );

PostQuitMessage( 0 );
return 0;

case WM_GESTURE:
return DecodeGesture( p_event->obj, p_event->p_gesture, hwnd, message, wParam, lParam );






case WM_ERASEBKGND:

return 1;
case WM_PAINT:

ValidateRect(hwnd, NULL);

default:
return DefWindowProc(hwnd, message, wParam, lParam);
}
}








static int Win32VoutCreateWindow( event_thread_t *p_event )
{
HINSTANCE hInstance;
WNDCLASS wc; 
int i_style;

msg_Dbg( p_event->obj, "Win32VoutCreateWindow" );


hInstance = GetModuleHandle(NULL);


assert( p_event->parent_window->type == VOUT_WINDOW_TYPE_HWND );
p_event->hparent = p_event->parent_window->handle.hwnd;


wc.style = CS_OWNDC|CS_DBLCLKS; 
wc.lpfnWndProc = (WNDPROC)VideoEventProc; 
wc.cbClsExtra = 0; 
wc.cbWndExtra = 0; 
wc.hInstance = hInstance; 
wc.hIcon = 0;
wc.hCursor = 0;
wc.hbrBackground = GetStockObject(BLACK_BRUSH); 
wc.lpszMenuName = NULL; 
wc.lpszClassName = p_event->class_video; 


if( !RegisterClass(&wc) )
{
msg_Err( p_event->obj, "Win32VoutCreateWindow RegisterClass FAILED (err=%lu)", GetLastError() );
return VLC_EGENERIC;
}

i_style = WS_VISIBLE|WS_CLIPCHILDREN|WS_CHILD|WS_DISABLED;


p_event->hvideownd =
CreateWindowEx( WS_EX_NOPARENTNOTIFY | WS_EX_NOACTIVATE,
p_event->class_video, 
TEXT(VOUT_TITLE) TEXT(" (VLC Video Output)"),
i_style, 
CW_USEDEFAULT, 
CW_USEDEFAULT, 
RECTWidth(p_event->window_area), 
RECTHeight(p_event->window_area), 
p_event->hparent, 
NULL, 
hInstance, 
(LPVOID)p_event ); 

if( !p_event->hvideownd )
{
msg_Warn( p_event->obj, "Win32VoutCreateWindow create window FAILED (err=%lu)", GetLastError() );
return VLC_EGENERIC;
}
msg_Dbg( p_event->obj, "created video window" );


LONG parent_style = GetWindowLong( p_event->hparent, GWL_STYLE );
if( !(parent_style & WS_CLIPCHILDREN) )

SetWindowLong( p_event->hparent, GWL_STYLE,
parent_style | WS_CLIPCHILDREN );

InitGestures( p_event->hvideownd, &p_event->p_gesture, p_event->is_projected );


ShowWindow( p_event->hvideownd, SW_SHOWNOACTIVATE );

return VLC_SUCCESS;
}






static void Win32VoutCloseWindow( event_thread_t *p_event )
{
msg_Dbg( p_event->obj, "Win32VoutCloseWindow" );

DestroyWindow( p_event->hvideownd );

HINSTANCE hInstance = GetModuleHandle(NULL);
UnregisterClass( p_event->class_video, hInstance );

CloseGestures( p_event->p_gesture);
}
