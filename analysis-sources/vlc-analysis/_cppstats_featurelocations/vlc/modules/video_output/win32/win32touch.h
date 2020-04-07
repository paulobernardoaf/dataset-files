






















#if !defined(VLC_GESTURE_H_)
#define VLC_GESTURE_H_

#if !defined(_WIN32_WINNT) || _WIN32_WINNT < 0x601
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0601
#endif
#include <windows.h>
#include <winuser.h>

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <vlc_common.h>

#if !defined(WM_GESTURE)
#define WM_GESTURE 0x0119
#endif

#if defined(__cplusplus)
extern "C" {
#endif

enum {
GESTURE_ACTION_UNDEFINED = 0,
GESTURE_ACTION_VOLUME,
GESTURE_ACTION_JUMP,
GESTURE_ACTION_BRIGHTNESS
};


typedef struct win32_gesture_sys_t {
DWORD i_type; 
int i_action; 

int i_beginx; 
int i_beginy; 
int i_lasty; 
double f_lastzoom; 

ULONGLONG i_ullArguments; 
bool b_2fingers; 

BOOL (*DecodeGestureImpl)( vlc_object_t *p_this, struct win32_gesture_sys_t *p_gesture, const GESTUREINFO* p_gi );

HINSTANCE huser_dll; 
BOOL (WINAPI * OurCloseGestureInfoHandle)(HGESTUREINFO hGestureInfo);
BOOL (WINAPI * OurGetGestureInfo)(HGESTUREINFO hGestureInfo, PGESTUREINFO pGestureInfo);
} win32_gesture_sys_t;


BOOL InitGestures( HWND hwnd, win32_gesture_sys_t **p_gesture, bool b_isProjected );

LRESULT DecodeGesture( vlc_object_t *p_intf, win32_gesture_sys_t *p_gesture,
HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );

void CloseGestures( win32_gesture_sys_t *p_gesture );

#if defined(__cplusplus)
}
#endif

#endif
