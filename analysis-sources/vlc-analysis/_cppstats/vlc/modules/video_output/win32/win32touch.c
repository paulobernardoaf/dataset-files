#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include "win32touch.h"
#include <vlc_actions.h>
#include <assert.h>
static BOOL DecodeGestureAction( vlc_object_t *p_this, win32_gesture_sys_t *p_gesture, const GESTUREINFO* p_gi );
static BOOL DecodeGestureProjection( vlc_object_t *p_this, win32_gesture_sys_t *p_gesture, const GESTUREINFO* p_gi );
LRESULT DecodeGesture( vlc_object_t *p_this, win32_gesture_sys_t *p_gesture,
HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
if( !p_gesture )
return DefWindowProc( hWnd, message, wParam, lParam );
GESTUREINFO gi;
ZeroMemory( &gi, sizeof( GESTUREINFO ) );
gi.cbSize = sizeof( GESTUREINFO );
BOOL bResult = p_gesture->OurGetGestureInfo((HGESTUREINFO)lParam, &gi);
BOOL bHandled = FALSE; 
if( bResult )
bHandled = p_gesture->DecodeGestureImpl(p_this, p_gesture, &gi);
else
{
DWORD dwErr = GetLastError();
if( dwErr > 0 )
msg_Err( p_this, "Could not retrieve a valid GESTUREINFO structure" );
}
if( bHandled )
{
p_gesture->OurCloseGestureInfoHandle((HGESTUREINFO)lParam);
return 0;
}
else
return DefWindowProc( hWnd, message, wParam, lParam );
}
static BOOL DecodeGestureAction( vlc_object_t *p_this, win32_gesture_sys_t *p_gesture, const GESTUREINFO* p_gi )
{
BOOL bHandled = FALSE; 
switch ( p_gi->dwID )
{
case GID_BEGIN:
p_gesture->i_beginx = p_gi->ptsLocation.x;
p_gesture->i_beginy = p_gi->ptsLocation.y;
p_gesture->i_lasty = p_gesture->i_beginy;
p_gesture->b_2fingers = false;
break;
case GID_END:
if( p_gesture->i_type != 0 &&
p_gesture->i_action == GESTURE_ACTION_JUMP )
{
int action_id;
if( p_gesture->i_beginx > p_gi->ptsLocation.x )
{
if( p_gesture->b_2fingers )
action_id = ACTIONID_JUMP_BACKWARD_MEDIUM;
else
action_id = ACTIONID_JUMP_BACKWARD_SHORT;
}
else
{
if( p_gesture->b_2fingers )
action_id = ACTIONID_JUMP_FORWARD_MEDIUM;
else
action_id = ACTIONID_JUMP_FORWARD_SHORT;
}
var_SetInteger( vlc_object_instance(p_this), "key-action", action_id );
}
p_gesture->i_action = GESTURE_ACTION_UNDEFINED;
p_gesture->i_type = p_gesture->i_beginx = p_gesture->i_beginy = -1;
p_gesture->b_2fingers = false;
break;
case GID_PAN:
p_gesture->i_type = GID_PAN;
bHandled = TRUE;
if (p_gi->dwFlags & GF_BEGIN) {
p_gesture->i_beginx = p_gi->ptsLocation.x;
p_gesture->i_beginy = p_gi->ptsLocation.y;
}
if( (DWORD)p_gi->ullArguments > 0 )
p_gesture->b_2fingers = true;
if( p_gesture->i_action == GESTURE_ACTION_UNDEFINED )
{
if( abs( p_gesture->i_beginx - p_gi->ptsLocation.x ) +
abs( p_gesture->i_beginy - p_gi->ptsLocation.y ) > 50 )
{
if( abs( p_gesture->i_beginx - p_gi->ptsLocation.x ) >
abs( p_gesture->i_beginy - p_gi->ptsLocation.y ) )
p_gesture->i_action = GESTURE_ACTION_JUMP;
else if ( p_gesture->b_2fingers )
p_gesture->i_action = GESTURE_ACTION_BRIGHTNESS;
else
p_gesture->i_action = GESTURE_ACTION_VOLUME;
}
}
if( p_gesture->i_action == GESTURE_ACTION_VOLUME )
{
int offset = p_gesture->i_lasty - p_gi->ptsLocation.y;
if( offset > 100)
var_SetInteger( vlc_object_instance(p_this), "key-action", ACTIONID_VOL_UP );
else if( offset < -100)
var_SetInteger( vlc_object_instance(p_this), "key-action", ACTIONID_VOL_DOWN );
else
break;
p_gesture->i_lasty = p_gi->ptsLocation.y;
}
else if ( p_gesture->i_action == GESTURE_ACTION_BRIGHTNESS )
{
}
break;
case GID_TWOFINGERTAP:
p_gesture->i_type = GID_TWOFINGERTAP;
var_SetInteger( vlc_object_instance(p_this), "key-action", ACTIONID_PLAY_PAUSE );
bHandled = TRUE;
break;
case GID_ZOOM:
p_gesture->i_type = GID_ZOOM;
switch( p_gi->dwFlags )
{
case GF_BEGIN:
p_gesture->i_ullArguments = p_gi->ullArguments;
break;
case GF_END:
{
double k = (double)(p_gi->ullArguments) /
(double)(p_gesture->i_ullArguments);
if( k > 1 )
var_SetInteger( vlc_object_instance(p_this), "key-action",
ACTIONID_TOGGLE_FULLSCREEN );
else
var_SetInteger( vlc_object_instance(p_this), "key-action",
ACTIONID_LEAVE_FULLSCREEN );
}
break;
default:
msg_Err( p_this, "Unmanaged dwFlag: %lx", p_gi->dwFlags );
}
bHandled = TRUE;
break;
case WM_VSCROLL:
bHandled = TRUE;
break;
default:
break;
}
return bHandled;
}
static BOOL DecodeGestureProjection( vlc_object_t *p_this, win32_gesture_sys_t *p_gesture, const GESTUREINFO* p_gi )
{
BOOL bHandled = FALSE; 
switch ( p_gi->dwID )
{
case GID_BEGIN:
p_gesture->i_beginx = p_gi->ptsLocation.x;
p_gesture->i_beginy = p_gi->ptsLocation.y;
p_gesture->i_lasty = p_gesture->i_beginy;
p_gesture->b_2fingers = false;
break;
case GID_END:
if( p_gesture->i_type != 0 &&
p_gesture->i_action == GESTURE_ACTION_JUMP )
{
int action_id;
if( p_gesture->b_2fingers )
{
if( p_gesture->i_beginx > p_gi->ptsLocation.x )
action_id = ACTIONID_JUMP_BACKWARD_SHORT;
else
action_id = ACTIONID_JUMP_FORWARD_SHORT;
var_SetInteger( vlc_object_instance(p_this), "key-action", action_id );
}
}
p_gesture->i_action = GESTURE_ACTION_UNDEFINED;
p_gesture->i_type = p_gesture->i_beginx = p_gesture->i_beginy = -1;
p_gesture->b_2fingers = false;
break;
case GID_PAN:
p_gesture->i_type = GID_PAN;
bHandled = TRUE;
if (p_gi->dwFlags & GF_BEGIN) {
p_gesture->i_beginx = p_gi->ptsLocation.x;
p_gesture->i_beginy = p_gi->ptsLocation.y;
}
if( (DWORD)p_gi->ullArguments > 0 )
p_gesture->b_2fingers = true;
if( p_gesture->b_2fingers && p_gesture->i_action == GESTURE_ACTION_UNDEFINED )
{
if( abs( p_gesture->i_beginx - p_gi->ptsLocation.x ) +
abs( p_gesture->i_beginy - p_gi->ptsLocation.y ) > 50 )
{
if( abs( p_gesture->i_beginx - p_gi->ptsLocation.x ) >
abs( p_gesture->i_beginy - p_gi->ptsLocation.y ) )
p_gesture->i_action = GESTURE_ACTION_JUMP;
else
p_gesture->i_action = GESTURE_ACTION_VOLUME;
}
}
if( p_gesture->i_action == GESTURE_ACTION_VOLUME )
{
int offset = p_gesture->i_lasty - p_gi->ptsLocation.y;
if( offset > 100)
var_SetInteger( vlc_object_instance(p_this), "key-action", ACTIONID_VOL_UP );
else if( offset < -100)
var_SetInteger( vlc_object_instance(p_this), "key-action", ACTIONID_VOL_DOWN );
else
break;
p_gesture->i_lasty = p_gi->ptsLocation.y;
}
break;
case GID_TWOFINGERTAP:
p_gesture->i_type = GID_TWOFINGERTAP;
var_SetInteger( vlc_object_instance(p_this), "key-action", ACTIONID_PLAY_PAUSE );
bHandled = TRUE;
break;
case GID_ZOOM:
p_gesture->i_type = GID_ZOOM;
bHandled = TRUE;
switch( p_gi->dwFlags )
{
case GF_BEGIN:
p_gesture->i_ullArguments = p_gi->ullArguments;
p_gesture->f_lastzoom = 1.0;
break;
default:
{
double k = (double)(p_gi->ullArguments) /
(double)(p_gesture->i_ullArguments);
if (k > p_gesture->f_lastzoom * 1.01)
{
var_SetInteger( vlc_object_instance(p_this), "key-action", ACTIONID_VIEWPOINT_FOV_IN );
p_gesture->f_lastzoom = k;
}
else if (k < p_gesture->f_lastzoom * 0.99)
{
var_SetInteger( vlc_object_instance(p_this), "key-action", ACTIONID_VIEWPOINT_FOV_OUT );
p_gesture->f_lastzoom = k;
}
break;
}
}
break;
case WM_VSCROLL:
bHandled = TRUE;
break;
default:
break;
}
return bHandled;
}
BOOL InitGestures( HWND hwnd, win32_gesture_sys_t **pp_gesture, bool b_isProjected )
{
BOOL result = FALSE;
GESTURECONFIG config = { 0, 0, 0 };
if (b_isProjected)
{
config.dwID = GID_PAN;
config.dwWant = GC_PAN;
config.dwBlock = GC_PAN_WITH_INERTIA;
}
else
{
config.dwID = GID_PAN;
config.dwWant = GC_PAN |
GC_PAN_WITH_SINGLE_FINGER_HORIZONTALLY |
GC_PAN_WITH_SINGLE_FINGER_VERTICALLY;
config.dwBlock = GC_PAN_WITH_INERTIA;
}
win32_gesture_sys_t *p_gesture = malloc( sizeof(win32_gesture_sys_t) );
if( !p_gesture )
{
*pp_gesture = NULL;
return FALSE;
}
HINSTANCE h_user32_dll = LoadLibrary(TEXT("user32.dll"));
if( !h_user32_dll )
{
*pp_gesture = NULL;
free( p_gesture );
return FALSE;
}
BOOL (WINAPI *OurSetGestureConfig) (HWND, DWORD, UINT, PGESTURECONFIG, UINT);
OurSetGestureConfig = (void *)GetProcAddress(h_user32_dll, "SetGestureConfig");
p_gesture->OurCloseGestureInfoHandle =
(void *)GetProcAddress(h_user32_dll, "CloseGestureInfoHandle" );
p_gesture->OurGetGestureInfo =
(void *)GetProcAddress(h_user32_dll, "GetGestureInfo");
if( OurSetGestureConfig )
{
result = OurSetGestureConfig(
hwnd,
0,
1,
&config,
sizeof( GESTURECONFIG )
);
}
if (b_isProjected)
p_gesture->DecodeGestureImpl = DecodeGestureProjection;
else
p_gesture->DecodeGestureImpl = DecodeGestureAction;
p_gesture->i_type = 0;
p_gesture->b_2fingers = false;
p_gesture->i_action = GESTURE_ACTION_UNDEFINED;
p_gesture->i_beginx = p_gesture->i_beginy = -1;
p_gesture->i_lasty = -1;
p_gesture->huser_dll = h_user32_dll;
*pp_gesture = p_gesture;
return result;
}
void CloseGestures( win32_gesture_sys_t *p_gesture )
{
if (p_gesture && p_gesture->huser_dll )
FreeLibrary( p_gesture->huser_dll );
free( p_gesture );
}
