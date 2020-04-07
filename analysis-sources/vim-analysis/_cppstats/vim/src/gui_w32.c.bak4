
























#include "vim.h"

#if defined(FEAT_DIRECTX)
#include "gui_dwrite.h"
#endif

#if defined(FEAT_DIRECTX)
static DWriteContext *s_dwc = NULL;
static int s_directx_enabled = 0;
static int s_directx_load_attempted = 0;
#define IS_ENABLE_DIRECTX() (s_directx_enabled && s_dwc != NULL && enc_utf8)
static int directx_enabled(void);
static void directx_binddc(void);
#endif

#if defined(FEAT_MENU)
static int gui_mswin_get_menu_height(int fix_window);
#endif

#if defined(FEAT_RENDER_OPTIONS) || defined(PROTO)
int
gui_mch_set_rendering_options(char_u *s)
{
#if defined(FEAT_DIRECTX)
char_u *p, *q;

int dx_enable = 0;
int dx_flags = 0;
float dx_gamma = 0.0f;
float dx_contrast = 0.0f;
float dx_level = 0.0f;
int dx_geom = 0;
int dx_renmode = 0;
int dx_taamode = 0;


for (p = s; p != NULL && *p != NUL; )
{
char_u item[256];
char_u name[128];
char_u value[128];

copy_option_part(&p, item, sizeof(item), ",");
if (p == NULL)
break;
q = &item[0];
copy_option_part(&q, name, sizeof(name), ":");
if (q == NULL)
return FAIL;
copy_option_part(&q, value, sizeof(value), ":");

if (STRCMP(name, "type") == 0)
{
if (STRCMP(value, "directx") == 0)
dx_enable = 1;
else
return FAIL;
}
else if (STRCMP(name, "gamma") == 0)
{
dx_flags |= 1 << 0;
dx_gamma = (float)atof((char *)value);
}
else if (STRCMP(name, "contrast") == 0)
{
dx_flags |= 1 << 1;
dx_contrast = (float)atof((char *)value);
}
else if (STRCMP(name, "level") == 0)
{
dx_flags |= 1 << 2;
dx_level = (float)atof((char *)value);
}
else if (STRCMP(name, "geom") == 0)
{
dx_flags |= 1 << 3;
dx_geom = atoi((char *)value);
if (dx_geom < 0 || dx_geom > 2)
return FAIL;
}
else if (STRCMP(name, "renmode") == 0)
{
dx_flags |= 1 << 4;
dx_renmode = atoi((char *)value);
if (dx_renmode < 0 || dx_renmode > 6)
return FAIL;
}
else if (STRCMP(name, "taamode") == 0)
{
dx_flags |= 1 << 5;
dx_taamode = atoi((char *)value);
if (dx_taamode < 0 || dx_taamode > 3)
return FAIL;
}
else if (STRCMP(name, "scrlines") == 0)
{

}
else
return FAIL;
}

if (!gui.in_use)
return OK; 


if (dx_enable)
{
if (!directx_enabled())
return FAIL;
DWriteContext_SetRenderingParams(s_dwc, NULL);
if (dx_flags)
{
DWriteRenderingParams param;
DWriteContext_GetRenderingParams(s_dwc, &param);
if (dx_flags & (1 << 0))
param.gamma = dx_gamma;
if (dx_flags & (1 << 1))
param.enhancedContrast = dx_contrast;
if (dx_flags & (1 << 2))
param.clearTypeLevel = dx_level;
if (dx_flags & (1 << 3))
param.pixelGeometry = dx_geom;
if (dx_flags & (1 << 4))
param.renderingMode = dx_renmode;
if (dx_flags & (1 << 5))
param.textAntialiasMode = dx_taamode;
DWriteContext_SetRenderingParams(s_dwc, &param);
}
}
s_directx_enabled = dx_enable;

return OK;
#else
return FAIL;
#endif
}
#endif




#if !defined(HANDLE_WM_XBUTTONUP)
#define HANDLE_WM_XBUTTONUP(hwnd, wParam, lParam, fn) ((fn)((hwnd), (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (UINT)(wParam)), 0L)

#endif
#if !defined(HANDLE_WM_XBUTTONDOWN)
#define HANDLE_WM_XBUTTONDOWN(hwnd, wParam, lParam, fn) ((fn)((hwnd), FALSE, (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (UINT)(wParam)), 0L)

#endif
#if !defined(HANDLE_WM_XBUTTONDBLCLK)
#define HANDLE_WM_XBUTTONDBLCLK(hwnd, wParam, lParam, fn) ((fn)((hwnd), TRUE, (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (UINT)(wParam)), 0L)

#endif


#include "version.h" 
#if defined(DEBUG)
#include <tchar.h>
#endif


#if !defined(PROTO)

#if !defined(__MINGW32__)
#include <shellapi.h>
#endif
#if defined(FEAT_TOOLBAR) || defined(FEAT_BEVAL_GUI) || defined(FEAT_GUI_TABLINE)
#include <commctrl.h>
#endif
#include <windowsx.h>

#if defined(GLOBAL_IME)
#include "glbl_ime.h"
#endif

#endif 

#if defined(FEAT_MENU)
#define MENUHINTS 
#endif


#define DLG_PADDING_X 10
#define DLG_PADDING_Y 10
#define DLG_OLD_STYLE_PADDING_X 5
#define DLG_OLD_STYLE_PADDING_Y 5
#define DLG_VERT_PADDING_X 4 
#define DLG_VERT_PADDING_Y 4
#define DLG_ICON_WIDTH 34
#define DLG_ICON_HEIGHT 34
#define DLG_MIN_WIDTH 150
#define DLG_FONT_NAME "MS Sans Serif"
#define DLG_FONT_POINT_SIZE 8
#define DLG_MIN_MAX_WIDTH 400
#define DLG_MIN_MAX_HEIGHT 400

#define DLG_NONBUTTON_CONTROL 5000 

#if !defined(WM_XBUTTONDOWN)
#define WM_XBUTTONDOWN 0x020B
#define WM_XBUTTONUP 0x020C
#define WM_XBUTTONDBLCLK 0x020D
#define MK_XBUTTON1 0x0020
#define MK_XBUTTON2 0x0040
#endif

#if defined(PROTO)




#define APIENTRY
#define CALLBACK
#define CONST
#define FAR
#define NEAR
#undef _cdecl
#define _cdecl
typedef int BOOL;
typedef int BYTE;
typedef int DWORD;
typedef int WCHAR;
typedef int ENUMLOGFONT;
typedef int FINDREPLACE;
typedef int HANDLE;
typedef int HBITMAP;
typedef int HBRUSH;
typedef int HDROP;
typedef int INT;
typedef int LOGFONTW[];
typedef int LPARAM;
typedef int LPCREATESTRUCT;
typedef int LPCSTR;
typedef int LPCTSTR;
typedef int LPRECT;
typedef int LPSTR;
typedef int LPWINDOWPOS;
typedef int LPWORD;
typedef int LRESULT;
typedef int HRESULT;
#undef MSG
typedef int MSG;
typedef int NEWTEXTMETRIC;
typedef int OSVERSIONINFO;
typedef int PWORD;
typedef int RECT;
typedef int UINT;
typedef int WORD;
typedef int WPARAM;
typedef int POINT;
typedef void *HINSTANCE;
typedef void *HMENU;
typedef void *HWND;
typedef void *HDC;
typedef void VOID;
typedef int LPNMHDR;
typedef int LONG;
typedef int WNDPROC;
typedef int UINT_PTR;
typedef int COLORREF;
typedef int HCURSOR;
#endif

#if !defined(GET_X_LPARAM)
#define GET_X_LPARAM(lp) ((int)(short)LOWORD(lp))
#endif

static void _OnPaint( HWND hwnd);
static void fill_rect(const RECT *rcp, HBRUSH hbr, COLORREF color);
static void clear_rect(RECT *rcp);

static WORD s_dlgfntheight; 
static WORD s_dlgfntwidth; 

#if defined(FEAT_MENU)
static HMENU s_menuBar = NULL;
#endif
#if defined(FEAT_TEAROFF)
static void rebuild_tearoff(vimmenu_T *menu);
static HBITMAP s_htearbitmap; 
#endif



static int s_busy_processing = FALSE;

static int destroying = FALSE; 

#if defined(MSWIN_FIND_REPLACE)
static UINT s_findrep_msg = 0; 
static FINDREPLACEW s_findrep_struct;
static HWND s_findrep_hwnd = NULL;
static int s_findrep_is_find; 

#endif

#if !defined(FEAT_GUI)
static
#endif
HWND s_hwnd = NULL;
static HDC s_hdc = NULL;
static HBRUSH s_brush = NULL;

#if defined(FEAT_TOOLBAR)
static HWND s_toolbarhwnd = NULL;
static WNDPROC s_toolbar_wndproc = NULL;
#endif

#if defined(FEAT_GUI_TABLINE)
static HWND s_tabhwnd = NULL;
static WNDPROC s_tabline_wndproc = NULL;
static int showing_tabline = 0;
#endif

static WPARAM s_wParam = 0;
static LPARAM s_lParam = 0;

static HWND s_textArea = NULL;
static UINT s_uMsg = 0;

static char_u *s_textfield; 

static int s_need_activate = FALSE;





static int allow_scrollbar = FALSE;

#if defined(GLOBAL_IME)
#define MyTranslateMessage(x) global_ime_TranslateMessage(x)
#else
#define MyTranslateMessage(x) TranslateMessage(x)
#endif

#if defined(FEAT_DIRECTX)
static int
directx_enabled(void)
{
if (s_dwc != NULL)
return 1;
else if (s_directx_load_attempted)
return 0;

DWrite_Init();
s_directx_load_attempted = 1;
s_dwc = DWriteContext_Open();
directx_binddc();
return s_dwc != NULL ? 1 : 0;
}

static void
directx_binddc(void)
{
if (s_textArea != NULL)
{
RECT rect;
GetClientRect(s_textArea, &rect);
DWriteContext_BindDC(s_dwc, s_hdc, &rect);
}
}
#endif


#define MyWindowProc vim_WindowProc

extern int current_font_height; 

static struct
{
UINT key_sym;
char_u vim_code0;
char_u vim_code1;
} special_keys[] =
{
{VK_UP, 'k', 'u'},
{VK_DOWN, 'k', 'd'},
{VK_LEFT, 'k', 'l'},
{VK_RIGHT, 'k', 'r'},

{VK_F1, 'k', '1'},
{VK_F2, 'k', '2'},
{VK_F3, 'k', '3'},
{VK_F4, 'k', '4'},
{VK_F5, 'k', '5'},
{VK_F6, 'k', '6'},
{VK_F7, 'k', '7'},
{VK_F8, 'k', '8'},
{VK_F9, 'k', '9'},
{VK_F10, 'k', ';'},

{VK_F11, 'F', '1'},
{VK_F12, 'F', '2'},
{VK_F13, 'F', '3'},
{VK_F14, 'F', '4'},
{VK_F15, 'F', '5'},
{VK_F16, 'F', '6'},
{VK_F17, 'F', '7'},
{VK_F18, 'F', '8'},
{VK_F19, 'F', '9'},
{VK_F20, 'F', 'A'},

{VK_F21, 'F', 'B'},
#if defined(FEAT_NETBEANS_INTG)
{VK_PAUSE, 'F', 'B'}, 
#endif
{VK_F22, 'F', 'C'},
{VK_F23, 'F', 'D'},
{VK_F24, 'F', 'E'}, 

{VK_HELP, '%', '1'},
{VK_BACK, 'k', 'b'},
{VK_INSERT, 'k', 'I'},
{VK_DELETE, 'k', 'D'},
{VK_HOME, 'k', 'h'},
{VK_END, '@', '7'},
{VK_PRIOR, 'k', 'P'},
{VK_NEXT, 'k', 'N'},
{VK_PRINT, '%', '9'},
{VK_ADD, 'K', '6'},
{VK_SUBTRACT, 'K', '7'},
{VK_DIVIDE, 'K', '8'},
{VK_MULTIPLY, 'K', '9'},
{VK_SEPARATOR, 'K', 'A'}, 
{VK_DECIMAL, 'K', 'B'},

{VK_NUMPAD0, 'K', 'C'},
{VK_NUMPAD1, 'K', 'D'},
{VK_NUMPAD2, 'K', 'E'},
{VK_NUMPAD3, 'K', 'F'},
{VK_NUMPAD4, 'K', 'G'},
{VK_NUMPAD5, 'K', 'H'},
{VK_NUMPAD6, 'K', 'I'},
{VK_NUMPAD7, 'K', 'J'},
{VK_NUMPAD8, 'K', 'K'},
{VK_NUMPAD9, 'K', 'L'},


{VK_SPACE, ' ', NUL},
{VK_TAB, TAB, NUL},
{VK_ESCAPE, ESC, NUL},
{NL, NL, NUL},
{CAR, CAR, NUL},


{0, 0, 0}
};


static int s_button_pending = -1;



static int s_getting_focus = FALSE;

static int s_x_pending;
static int s_y_pending;
static UINT s_kFlags_pending;
static UINT s_wait_timer = 0; 
static int s_timed_out = FALSE;
static int dead_key = 0; 
static UINT surrogate_pending_ch = 0; 


#if defined(FEAT_BEVAL_GUI)

static void Handle_WM_Notify(HWND hwnd, LPNMHDR pnmh);
static void TrackUserActivity(UINT uMsg);
#endif






#if defined(FEAT_MBYTE_IME) || defined(GLOBAL_IME)

static LOGFONTW norm_logfont;
#endif
#if defined(FEAT_MBYTE_IME)

static LOGFONTW sub_logfont;
#endif

#if defined(FEAT_MBYTE_IME)
static LRESULT _OnImeNotify(HWND hWnd, DWORD dwCommand, DWORD dwData);
#endif

#if defined(FEAT_BROWSE)
static char_u *convert_filter(char_u *s);
#endif

#if defined(DEBUG_PRINT_ERROR)



static void
print_windows_error(void)
{
LPVOID lpMsgBuf;

FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
NULL, GetLastError(),
MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
(LPTSTR) &lpMsgBuf, 0, NULL);
TRACE1("Error: %s\n", lpMsgBuf);
LocalFree(lpMsgBuf);
}
#endif










#define BLINK_NONE 0
#define BLINK_OFF 1
#define BLINK_ON 2

static int blink_state = BLINK_NONE;
static long_u blink_waittime = 700;
static long_u blink_ontime = 400;
static long_u blink_offtime = 250;
static UINT blink_timer = 0;

int
gui_mch_is_blinking(void)
{
return blink_state != BLINK_NONE;
}

int
gui_mch_is_blink_off(void)
{
return blink_state == BLINK_OFF;
}

void
gui_mch_set_blinking(long wait, long on, long off)
{
blink_waittime = wait;
blink_ontime = on;
blink_offtime = off;
}

static VOID CALLBACK
_OnBlinkTimer(
HWND hwnd,
UINT uMsg UNUSED,
UINT idEvent,
DWORD dwTime UNUSED)
{
MSG msg;





KillTimer(NULL, idEvent);


while (pPeekMessage(&msg, hwnd, WM_TIMER, WM_TIMER, PM_REMOVE))
;

if (blink_state == BLINK_ON)
{
gui_undraw_cursor();
blink_state = BLINK_OFF;
blink_timer = (UINT) SetTimer(NULL, 0, (UINT)blink_offtime,
(TIMERPROC)_OnBlinkTimer);
}
else
{
gui_update_cursor(TRUE, FALSE);
blink_state = BLINK_ON;
blink_timer = (UINT) SetTimer(NULL, 0, (UINT)blink_ontime,
(TIMERPROC)_OnBlinkTimer);
}
gui_mch_flush();
}

static void
gui_mswin_rm_blink_timer(void)
{
MSG msg;

if (blink_timer != 0)
{
KillTimer(NULL, blink_timer);

while (pPeekMessage(&msg, s_hwnd, WM_TIMER, WM_TIMER, PM_REMOVE))
;
blink_timer = 0;
}
}




void
gui_mch_stop_blink(int may_call_gui_update_cursor)
{
gui_mswin_rm_blink_timer();
if (blink_state == BLINK_OFF && may_call_gui_update_cursor)
{
gui_update_cursor(TRUE, FALSE);
gui_mch_flush();
}
blink_state = BLINK_NONE;
}





void
gui_mch_start_blink(void)
{
gui_mswin_rm_blink_timer();


if (blink_waittime && blink_ontime && blink_offtime && gui.in_focus)
{
blink_timer = (UINT)SetTimer(NULL, 0, (UINT)blink_waittime,
(TIMERPROC)_OnBlinkTimer);
blink_state = BLINK_ON;
gui_update_cursor(TRUE, FALSE);
gui_mch_flush();
}
}





static VOID CALLBACK
_OnTimer(
HWND hwnd,
UINT uMsg UNUSED,
UINT idEvent,
DWORD dwTime UNUSED)
{
MSG msg;




KillTimer(NULL, idEvent);
s_timed_out = TRUE;


while (pPeekMessage(&msg, hwnd, WM_TIMER, WM_TIMER, PM_REMOVE))
;
if (idEvent == s_wait_timer)
s_wait_timer = 0;
}

static void
_OnDeadChar(
HWND hwnd UNUSED,
UINT ch UNUSED,
int cRepeat UNUSED)
{
dead_key = 1;
}












static int
char_to_string(int ch, char_u *string, int slen, int had_alt)
{
int len;
int i;
WCHAR wstring[2];
char_u *ws = NULL;

if (surrogate_pending_ch != 0)
{


wstring[0] = surrogate_pending_ch;
wstring[1] = ch;
surrogate_pending_ch = 0;
len = 2;
}
else if (ch >= 0xD800 && ch <= 0xDBFF) 
{


surrogate_pending_ch = ch;
return 0;
}
else
{
wstring[0] = ch;
len = 1;
}




if (enc_codepage > 0)
{
len = WideCharToMultiByte(enc_codepage, 0, wstring, len,
(LPSTR)string, slen, 0, NULL);




if (had_alt && len == 1 && ch >= 0x80 && string[0] < 0x80)
{
wstring[0] = ch & 0x7f;
len = WideCharToMultiByte(enc_codepage, 0, wstring, len,
(LPSTR)string, slen, 0, NULL);
if (len == 1) 
string[0] |= 0x80;
}
}
else
{
ws = utf16_to_enc(wstring, &len);
if (ws == NULL)
len = 0;
else
{
if (len > slen) 
len = slen;
mch_memmove(string, ws, len);
vim_free(ws);
}
}

if (len == 0)
{
string[0] = ch;
len = 1;
}

for (i = 0; i < len; ++i)
if (string[i] == CSI && len <= slen - 2)
{

mch_memmove(string + i + 3, string + i + 1, len - i - 1);
string[++i] = KS_EXTRA;
string[++i] = (int)KE_CSI;
len += 2;
}

return len;
}




static void
_OnChar(
HWND hwnd UNUSED,
UINT ch,
int cRepeat UNUSED)
{
char_u string[40];
int len = 0;

dead_key = 0;

len = char_to_string(ch, string, 40, FALSE);
if (len == 1 && string[0] == Ctrl_C && ctrl_c_interrupts)
{
trash_input_buf();
got_int = TRUE;
}

add_to_input_buf(string, len);
}




static void
_OnSysChar(
HWND hwnd UNUSED,
UINT cch,
int cRepeat UNUSED)
{
char_u string[40]; 
int len;
int modifiers;
int ch = cch; 

dead_key = 0;







modifiers = MOD_MASK_ALT;
if (GetKeyState(VK_SHIFT) & 0x8000)
modifiers |= MOD_MASK_SHIFT;
if (GetKeyState(VK_CONTROL) & 0x8000)
modifiers |= MOD_MASK_CTRL;

ch = simplify_key(ch, &modifiers);


if (ch < 0x100 && !isalpha(ch) && isprint(ch))
modifiers &= ~MOD_MASK_SHIFT;


ch = extract_modifiers(ch, &modifiers, TRUE, NULL);
if (ch == CSI)
ch = K_CSI;

len = 0;
if (modifiers)
{
string[len++] = CSI;
string[len++] = KS_MODIFIER;
string[len++] = modifiers;
}

if (IS_SPECIAL((int)ch))
{
string[len++] = CSI;
string[len++] = K_SECOND((int)ch);
string[len++] = K_THIRD((int)ch);
}
else
{


len += char_to_string(ch, string + len, 40 - len, TRUE);
}

add_to_input_buf(string, len);
}

static void
_OnMouseEvent(
int button,
int x,
int y,
int repeated_click,
UINT keyFlags)
{
int vim_modifiers = 0x0;

s_getting_focus = FALSE;

if (keyFlags & MK_SHIFT)
vim_modifiers |= MOUSE_SHIFT;
if (keyFlags & MK_CONTROL)
vim_modifiers |= MOUSE_CTRL;
if (GetKeyState(VK_MENU) & 0x8000)
vim_modifiers |= MOUSE_ALT;

gui_send_mouse_event(button, x, y, repeated_click, vim_modifiers);
}

static void
_OnMouseButtonDown(
HWND hwnd UNUSED,
BOOL fDoubleClick UNUSED,
int x,
int y,
UINT keyFlags)
{
static LONG s_prevTime = 0;

LONG currentTime = GetMessageTime();
int button = -1;
int repeated_click;


(void)SetFocus(s_hwnd);

if (s_uMsg == WM_LBUTTONDOWN || s_uMsg == WM_LBUTTONDBLCLK)
button = MOUSE_LEFT;
else if (s_uMsg == WM_MBUTTONDOWN || s_uMsg == WM_MBUTTONDBLCLK)
button = MOUSE_MIDDLE;
else if (s_uMsg == WM_RBUTTONDOWN || s_uMsg == WM_RBUTTONDBLCLK)
button = MOUSE_RIGHT;
else if (s_uMsg == WM_XBUTTONDOWN || s_uMsg == WM_XBUTTONDBLCLK)
{
#if !defined(GET_XBUTTON_WPARAM)
#define GET_XBUTTON_WPARAM(wParam) (HIWORD(wParam))
#endif
button = ((GET_XBUTTON_WPARAM(s_wParam) == 1) ? MOUSE_X1 : MOUSE_X2);
}
else if (s_uMsg == WM_CAPTURECHANGED)
{


if (s_button_pending == MOUSE_LEFT)
button = MOUSE_RIGHT;
else
button = MOUSE_LEFT;
}
if (button >= 0)
{
repeated_click = ((int)(currentTime - s_prevTime) < p_mouset);





if (repeated_click
&& ((button == MOUSE_LEFT && s_button_pending == MOUSE_RIGHT)
|| (button == MOUSE_RIGHT
&& s_button_pending == MOUSE_LEFT)))
{




gui_send_mouse_event(MOUSE_RELEASE, x, y, FALSE, 0x0);
button = MOUSE_MIDDLE;
repeated_click = FALSE;
s_button_pending = -1;
_OnMouseEvent(button, x, y, repeated_click, keyFlags);
}
else if ((repeated_click)
|| (mouse_model_popup() && (button == MOUSE_RIGHT)))
{
if (s_button_pending > -1)
{
_OnMouseEvent(s_button_pending, x, y, FALSE, keyFlags);
s_button_pending = -1;
}

_OnMouseEvent(button, x, y, repeated_click, keyFlags);
}
else
{













s_button_pending = button;
s_x_pending = x;
s_y_pending = y;
s_kFlags_pending = keyFlags;
}

s_prevTime = currentTime;
}
}

static void
_OnMouseMoveOrRelease(
HWND hwnd UNUSED,
int x,
int y,
UINT keyFlags)
{
int button;

s_getting_focus = FALSE;
if (s_button_pending > -1)
{

_OnMouseEvent(s_button_pending, s_x_pending,
s_y_pending, FALSE, s_kFlags_pending);
s_button_pending = -1;
}
if (s_uMsg == WM_MOUSEMOVE)
{




if (!(keyFlags & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON
| MK_XBUTTON1 | MK_XBUTTON2)))
{
gui_mouse_moved(x, y);
return;
}





SetCapture(s_textArea);
button = MOUSE_DRAG;

}
else
{
ReleaseCapture();
button = MOUSE_RELEASE;

}

_OnMouseEvent(button, x, y, FALSE, keyFlags);
}

static void
_OnSizeTextArea(
HWND hwnd UNUSED,
UINT state UNUSED,
int cx UNUSED,
int cy UNUSED)
{
#if defined(FEAT_DIRECTX)
if (IS_ENABLE_DIRECTX())
directx_binddc();
#endif
}

#if defined(FEAT_MENU)



static vimmenu_T *
gui_mswin_find_menu(
vimmenu_T *pMenu,
int id)
{
vimmenu_T *pChildMenu;

while (pMenu)
{
if (pMenu->id == (UINT)id)
break;
if (pMenu->children != NULL)
{
pChildMenu = gui_mswin_find_menu(pMenu->children, id);
if (pChildMenu)
{
pMenu = pChildMenu;
break;
}
}
pMenu = pMenu->next;
}
return pMenu;
}

static void
_OnMenu(
HWND hwnd UNUSED,
int id,
HWND hwndCtl UNUSED,
UINT codeNotify UNUSED)
{
vimmenu_T *pMenu;

pMenu = gui_mswin_find_menu(root_menu, id);
if (pMenu)
gui_menu_cb(pMenu);
}
#endif

#if defined(MSWIN_FIND_REPLACE)



static void
_OnFindRepl(void)
{
int flags = 0;
int down;

if (s_findrep_struct.Flags & FR_DIALOGTERM)

(void)SetFocus(s_hwnd);

if (s_findrep_struct.Flags & FR_FINDNEXT)
{
flags = FRD_FINDNEXT;



(void)SetFocus(s_hwnd);
}
else if (s_findrep_struct.Flags & FR_REPLACE)
{
flags = FRD_REPLACE;



(void)SetFocus(s_hwnd);
}
else if (s_findrep_struct.Flags & FR_REPLACEALL)
{
flags = FRD_REPLACEALL;
}

if (flags != 0)
{
char_u *p, *q;


if (s_findrep_struct.Flags & FR_WHOLEWORD)
flags |= FRD_WHOLE_WORD;
if (s_findrep_struct.Flags & FR_MATCHCASE)
flags |= FRD_MATCH_CASE;
down = (s_findrep_struct.Flags & FR_DOWN) != 0;
p = utf16_to_enc(s_findrep_struct.lpstrFindWhat, NULL);
q = utf16_to_enc(s_findrep_struct.lpstrReplaceWith, NULL);
if (p != NULL && q != NULL)
gui_do_findrepl(flags, p, q, down);
vim_free(p);
vim_free(q);
}
}
#endif

static void
HandleMouseHide(UINT uMsg, LPARAM lParam)
{
static LPARAM last_lParam = 0L;


if (uMsg == WM_MOUSEMOVE || uMsg == WM_NCMOUSEMOVE)
{
if (lParam == last_lParam)
return;
last_lParam = lParam;
}




switch (uMsg)
{
case WM_KEYUP:
case WM_CHAR:



if (p_mh)
gui_mch_mousehide(TRUE);
break;

case WM_SYSKEYUP: 
case WM_SYSCHAR:
case WM_MOUSEMOVE: 
case WM_LBUTTONDOWN:
case WM_LBUTTONUP:
case WM_MBUTTONDOWN:
case WM_MBUTTONUP:
case WM_RBUTTONDOWN:
case WM_RBUTTONUP:
case WM_XBUTTONDOWN:
case WM_XBUTTONUP:
case WM_NCMOUSEMOVE:
case WM_NCLBUTTONDOWN:
case WM_NCLBUTTONUP:
case WM_NCMBUTTONDOWN:
case WM_NCMBUTTONUP:
case WM_NCRBUTTONDOWN:
case WM_NCRBUTTONUP:
case WM_KILLFOCUS:



gui_mch_mousehide(FALSE);
break;
}
}

static LRESULT CALLBACK
_TextAreaWndProc(
HWND hwnd,
UINT uMsg,
WPARAM wParam,
LPARAM lParam)
{





HandleMouseHide(uMsg, lParam);

s_uMsg = uMsg;
s_wParam = wParam;
s_lParam = lParam;

#if defined(FEAT_BEVAL_GUI)
TrackUserActivity(uMsg);
#endif

switch (uMsg)
{
HANDLE_MSG(hwnd, WM_LBUTTONDBLCLK,_OnMouseButtonDown);
HANDLE_MSG(hwnd, WM_LBUTTONDOWN,_OnMouseButtonDown);
HANDLE_MSG(hwnd, WM_LBUTTONUP, _OnMouseMoveOrRelease);
HANDLE_MSG(hwnd, WM_MBUTTONDBLCLK,_OnMouseButtonDown);
HANDLE_MSG(hwnd, WM_MBUTTONDOWN,_OnMouseButtonDown);
HANDLE_MSG(hwnd, WM_MBUTTONUP, _OnMouseMoveOrRelease);
HANDLE_MSG(hwnd, WM_MOUSEMOVE, _OnMouseMoveOrRelease);
HANDLE_MSG(hwnd, WM_PAINT, _OnPaint);
HANDLE_MSG(hwnd, WM_RBUTTONDBLCLK,_OnMouseButtonDown);
HANDLE_MSG(hwnd, WM_RBUTTONDOWN,_OnMouseButtonDown);
HANDLE_MSG(hwnd, WM_RBUTTONUP, _OnMouseMoveOrRelease);
HANDLE_MSG(hwnd, WM_XBUTTONDBLCLK,_OnMouseButtonDown);
HANDLE_MSG(hwnd, WM_XBUTTONDOWN,_OnMouseButtonDown);
HANDLE_MSG(hwnd, WM_XBUTTONUP, _OnMouseMoveOrRelease);
HANDLE_MSG(hwnd, WM_SIZE, _OnSizeTextArea);

#if defined(FEAT_BEVAL_GUI)
case WM_NOTIFY: Handle_WM_Notify(hwnd, (LPNMHDR)lParam);
return TRUE;
#endif
default:
return MyWindowProc(hwnd, uMsg, wParam, lParam);
}
}

#if defined(PROTO)
typedef int WINAPI;
#endif

LRESULT WINAPI
vim_WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
#if defined(GLOBAL_IME)
return global_ime_DefWindowProc(hwnd, message, wParam, lParam);
#else
return DefWindowProcW(hwnd, message, wParam, lParam);
#endif
}




void
gui_mch_new_colors(void)
{
HBRUSH prevBrush;

s_brush = CreateSolidBrush(gui.back_pixel);
prevBrush = (HBRUSH)SetClassLongPtr(
s_hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)s_brush);
InvalidateRect(s_hwnd, NULL, TRUE);
DeleteObject(prevBrush);
}




void
gui_mch_def_colors(void)
{
gui.norm_pixel = GetSysColor(COLOR_WINDOWTEXT);
gui.back_pixel = GetSysColor(COLOR_WINDOW);
gui.def_norm_pixel = gui.norm_pixel;
gui.def_back_pixel = gui.back_pixel;
}




int
gui_mch_open(void)
{


if (!IsWindowVisible(s_hwnd))
ShowWindow(s_hwnd, SW_SHOWDEFAULT);

#if defined(MSWIN_FIND_REPLACE)


s_findrep_struct.lpstrReplaceWith[0] = NUL;
#endif

return OK;
}




int
gui_mch_get_winpos(int *x, int *y)
{
RECT rect;

GetWindowRect(s_hwnd, &rect);
*x = rect.left;
*y = rect.top;
return OK;
}





void
gui_mch_set_winpos(int x, int y)
{
SetWindowPos(s_hwnd, NULL, x, y, 0, 0,
SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
}
void
gui_mch_set_text_area_pos(int x, int y, int w, int h)
{
static int oldx = 0;
static int oldy = 0;

SetWindowPos(s_textArea, NULL, x, y, w, h, SWP_NOZORDER | SWP_NOACTIVATE);

#if defined(FEAT_TOOLBAR)
if (vim_strchr(p_go, GO_TOOLBAR) != NULL)
SendMessage(s_toolbarhwnd, WM_SIZE,
(WPARAM)0, (LPARAM)(w + ((long)(TOOLBAR_BUTTON_HEIGHT+8)<<16)));
#endif
#if defined(FEAT_GUI_TABLINE)
if (showing_tabline)
{
int top = 0;
RECT rect;

#if defined(FEAT_TOOLBAR)
if (vim_strchr(p_go, GO_TOOLBAR) != NULL)
top = TOOLBAR_BUTTON_HEIGHT + TOOLBAR_BORDER_HEIGHT;
#endif
GetClientRect(s_hwnd, &rect);
MoveWindow(s_tabhwnd, 0, top, rect.right, gui.tabline_height, TRUE);
}
#endif




if (oldx != x || oldy != y)
{
InvalidateRect(s_hwnd, NULL, FALSE);
oldx = x;
oldy = y;
}
}






void
gui_mch_enable_scrollbar(
scrollbar_T *sb,
int flag)
{
ShowScrollBar(sb->id, SB_CTL, flag);




}

void
gui_mch_set_scrollbar_pos(
scrollbar_T *sb,
int x,
int y,
int w,
int h)
{
SetWindowPos(sb->id, NULL, x, y, w, h,
SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);
}

void
gui_mch_create_scrollbar(
scrollbar_T *sb,
int orient) 
{
sb->id = CreateWindow(
"SCROLLBAR", "Scrollbar",
WS_CHILD | ((orient == SBAR_VERT) ? SBS_VERT : SBS_HORZ), 0, 0,
10, 
10, 
s_hwnd, NULL,
g_hinst, NULL);
}




static scrollbar_T *
gui_mswin_find_scrollbar(HWND hwnd)
{
win_T *wp;

if (gui.bottom_sbar.id == hwnd)
return &gui.bottom_sbar;
FOR_ALL_WINDOWS(wp)
{
if (wp->w_scrollbars[SBAR_LEFT].id == hwnd)
return &wp->w_scrollbars[SBAR_LEFT];
if (wp->w_scrollbars[SBAR_RIGHT].id == hwnd)
return &wp->w_scrollbars[SBAR_RIGHT];
}
return NULL;
}




static void
GetFontSize(GuiFont font)
{
HWND hwnd = GetDesktopWindow();
HDC hdc = GetWindowDC(hwnd);
HFONT hfntOld = SelectFont(hdc, (HFONT)font);
SIZE size;
TEXTMETRIC tm;

GetTextMetrics(hdc, &tm);


GetTextExtentPoint(hdc,
"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz",
52, &size);
gui.char_width = (size.cx / 26 + 1) / 2 + tm.tmOverhang;

gui.char_height = tm.tmHeight + p_linespace;

SelectFont(hdc, hfntOld);

ReleaseDC(hwnd, hdc);
}




int
gui_mch_adjust_charheight(void)
{
GetFontSize(gui.norm_font);
return OK;
}

static GuiFont
get_font_handle(LOGFONTW *lf)
{
HFONT font = NULL;


font = CreateFontIndirectW(lf);

if (font == NULL)
return NOFONT;

return (GuiFont)font;
}

static int
pixels_to_points(int pixels, int vertical)
{
int points;
HWND hwnd;
HDC hdc;

hwnd = GetDesktopWindow();
hdc = GetWindowDC(hwnd);

points = MulDiv(pixels, 72,
GetDeviceCaps(hdc, vertical ? LOGPIXELSY : LOGPIXELSX));

ReleaseDC(hwnd, hdc);

return points;
}

GuiFont
gui_mch_get_font(
char_u *name,
int giveErrorIfMissing)
{
LOGFONTW lf;
GuiFont font = NOFONT;

if (get_logfont(&lf, name, NULL, giveErrorIfMissing) == OK)
font = get_font_handle(&lf);
if (font == NOFONT && giveErrorIfMissing)
semsg(_(e_font), name);
return font;
}

#if defined(FEAT_EVAL) || defined(PROTO)




char_u *
gui_mch_get_fontname(GuiFont font UNUSED, char_u *name)
{
if (name == NULL)
return NULL;
return vim_strsave(name);
}
#endif

void
gui_mch_free_font(GuiFont font)
{
if (font)
DeleteObject((HFONT)font);
}





guicolor_T
gui_mch_get_color(char_u *name)
{
int i;

typedef struct SysColorTable
{
char *name;
int color;
} SysColorTable;

static SysColorTable sys_table[] =
{
{"SYS_3DDKSHADOW", COLOR_3DDKSHADOW},
{"SYS_3DHILIGHT", COLOR_3DHILIGHT},
#if defined(COLOR_3DHIGHLIGHT)
{"SYS_3DHIGHLIGHT", COLOR_3DHIGHLIGHT},
#endif
{"SYS_BTNHILIGHT", COLOR_BTNHILIGHT},
{"SYS_BTNHIGHLIGHT", COLOR_BTNHIGHLIGHT},
{"SYS_3DLIGHT", COLOR_3DLIGHT},
{"SYS_3DSHADOW", COLOR_3DSHADOW},
{"SYS_DESKTOP", COLOR_DESKTOP},
{"SYS_INFOBK", COLOR_INFOBK},
{"SYS_INFOTEXT", COLOR_INFOTEXT},
{"SYS_3DFACE", COLOR_3DFACE},
{"SYS_BTNFACE", COLOR_BTNFACE},
{"SYS_BTNSHADOW", COLOR_BTNSHADOW},
{"SYS_ACTIVEBORDER", COLOR_ACTIVEBORDER},
{"SYS_ACTIVECAPTION", COLOR_ACTIVECAPTION},
{"SYS_APPWORKSPACE", COLOR_APPWORKSPACE},
{"SYS_BACKGROUND", COLOR_BACKGROUND},
{"SYS_BTNTEXT", COLOR_BTNTEXT},
{"SYS_CAPTIONTEXT", COLOR_CAPTIONTEXT},
{"SYS_GRAYTEXT", COLOR_GRAYTEXT},
{"SYS_HIGHLIGHT", COLOR_HIGHLIGHT},
{"SYS_HIGHLIGHTTEXT", COLOR_HIGHLIGHTTEXT},
{"SYS_INACTIVEBORDER", COLOR_INACTIVEBORDER},
{"SYS_INACTIVECAPTION", COLOR_INACTIVECAPTION},
{"SYS_INACTIVECAPTIONTEXT", COLOR_INACTIVECAPTIONTEXT},
{"SYS_MENU", COLOR_MENU},
{"SYS_MENUTEXT", COLOR_MENUTEXT},
{"SYS_SCROLLBAR", COLOR_SCROLLBAR},
{"SYS_WINDOW", COLOR_WINDOW},
{"SYS_WINDOWFRAME", COLOR_WINDOWFRAME},
{"SYS_WINDOWTEXT", COLOR_WINDOWTEXT}
};




for (i = 0; i < sizeof(sys_table) / sizeof(sys_table[0]); i++)
if (STRICMP(name, sys_table[i].name) == 0)
return GetSysColor(sys_table[i].color);

return gui_get_color_cmn(name);
}

guicolor_T
gui_mch_get_rgb_color(int r, int g, int b)
{
return gui_get_rgb_color_cmn(r, g, b);
}




int
gui_mch_haskey(char_u *name)
{
int i;

for (i = 0; special_keys[i].vim_code1 != NUL; i++)
if (name[0] == special_keys[i].vim_code0 &&
name[1] == special_keys[i].vim_code1)
return OK;
return FAIL;
}

void
gui_mch_beep(void)
{
MessageBeep(MB_OK);
}



void
gui_mch_invert_rectangle(
int r,
int c,
int nr,
int nc)
{
RECT rc;

#if defined(FEAT_DIRECTX)
if (IS_ENABLE_DIRECTX())
DWriteContext_Flush(s_dwc);
#endif




rc.left = FILL_X(c);
rc.top = FILL_Y(r);
rc.right = rc.left + nc * gui.char_width;
rc.bottom = rc.top + nr * gui.char_height;
InvertRect(s_hdc, &rc);
}




void
gui_mch_iconify(void)
{
ShowWindow(s_hwnd, SW_MINIMIZE);
}




void
gui_mch_draw_hollow_cursor(guicolor_T color)
{
HBRUSH hbr;
RECT rc;

#if defined(FEAT_DIRECTX)
if (IS_ENABLE_DIRECTX())
DWriteContext_Flush(s_dwc);
#endif




rc.left = FILL_X(gui.col);
rc.top = FILL_Y(gui.row);
rc.right = rc.left + gui.char_width;
if (mb_lefthalve(gui.row, gui.col))
rc.right += gui.char_width;
rc.bottom = rc.top + gui.char_height;
hbr = CreateSolidBrush(color);
FrameRect(s_hdc, &rc, hbr);
DeleteBrush(hbr);
}




void
gui_mch_draw_part_cursor(
int w,
int h,
guicolor_T color)
{
RECT rc;




rc.left =
#if defined(FEAT_RIGHTLEFT)

CURSOR_BAR_RIGHT ? FILL_X(gui.col + 1) - w :
#endif
FILL_X(gui.col);
rc.top = FILL_Y(gui.row) + gui.char_height - h;
rc.right = rc.left + w;
rc.bottom = rc.top + h;

fill_rect(&rc, NULL, color);
}






static void
outputDeadKey_rePost(MSG originalMsg)
{
static MSG deadCharExpel;

if (!dead_key)
return;

dead_key = 0;


deadCharExpel.message = originalMsg.message;
deadCharExpel.hwnd = originalMsg.hwnd;
deadCharExpel.wParam = VK_SPACE;

MyTranslateMessage(&deadCharExpel);


PostMessage(originalMsg.hwnd, originalMsg.message, originalMsg.wParam,
originalMsg.lParam);
}






static void
process_message(void)
{
MSG msg;
UINT vk = 0; 
char_u string[40];
int i;
int modifiers = 0;
int key;
#if defined(FEAT_MENU)
static char_u k10[] = {K_SPECIAL, 'k', ';', 0};
#endif

pGetMessage(&msg, NULL, 0, 0);

#if defined(FEAT_OLE)

if (msg.message == WM_OLE)
{
char_u *str = (char_u *)msg.lParam;
if (str == NULL || *str == NUL)
{


pDispatchMessage(&msg);
}
else
{
add_to_input_buf(str, (int)STRLEN(str));
vim_free(str); 
}
return;
}
#endif

#if defined(MSWIN_FIND_REPLACE)

if (s_findrep_hwnd != NULL && pIsDialogMessage(s_findrep_hwnd, &msg))
{
HandleMouseHide(msg.message, msg.lParam);
return;
}
#endif





if (msg.message == WM_KEYDOWN || msg.message == WM_SYSKEYDOWN)
{
vk = (int) msg.wParam;















if (dead_key)
{












if ((vk == VK_SPACE || vk == VK_BACK || vk == VK_ESCAPE))
{
dead_key = 0;
MyTranslateMessage(&msg);
return;
}


else if (!(get_real_state() & (INSERT | CMDLINE | SELECTMODE)))
{
outputDeadKey_rePost(msg);
return;
}
}


if (vk == VK_CANCEL)
{
trash_input_buf();
got_int = TRUE;
ctrl_break_was_pressed = TRUE;
string[0] = Ctrl_C;
add_to_input_buf(string, 1);
}

for (i = 0; special_keys[i].key_sym != 0; i++)
{

if (special_keys[i].key_sym == vk
&& (vk != VK_SPACE || !(GetKeyState(VK_MENU) & 0x8000)))
{






if (dead_key && (special_keys[i].vim_code0 == 'K'
|| vk == VK_TAB || vk == CAR))
{
outputDeadKey_rePost(msg);
return;
}

#if defined(FEAT_MENU)


if (vk == VK_F10
&& gui.menu_is_active
&& check_map(k10, State, FALSE, TRUE, FALSE,
NULL, NULL) == NULL)
break;
#endif
if (GetKeyState(VK_SHIFT) & 0x8000)
modifiers |= MOD_MASK_SHIFT;









if (GetKeyState(VK_CONTROL) & 0x8000)
modifiers |= MOD_MASK_CTRL;
if (GetKeyState(VK_MENU) & 0x8000)
modifiers |= MOD_MASK_ALT;

if (special_keys[i].vim_code1 == NUL)
key = special_keys[i].vim_code0;
else
key = TO_SPECIAL(special_keys[i].vim_code0,
special_keys[i].vim_code1);
key = simplify_key(key, &modifiers);
if (key == CSI)
key = K_CSI;

if (modifiers)
{
string[0] = CSI;
string[1] = KS_MODIFIER;
string[2] = modifiers;
add_to_input_buf(string, 3);
}

if (IS_SPECIAL(key))
{
string[0] = CSI;
string[1] = K_SECOND(key);
string[2] = K_THIRD(key);
add_to_input_buf(string, 3);
}
else
{
int len;


len = char_to_string(key, string, 40, FALSE);
add_to_input_buf(string, len);
}
break;
}
}
if (special_keys[i].key_sym == 0)
{



if (vk != 0xff
&& (GetKeyState(VK_CONTROL) & 0x8000)
&& !(GetKeyState(VK_SHIFT) & 0x8000)
&& !(GetKeyState(VK_MENU) & 0x8000))
{

if (vk == '6' || MapVirtualKey(vk, 2) == (UINT)'^')
{
string[0] = Ctrl_HAT;
add_to_input_buf(string, 1);
}

else if (vk == 0xBD) 
{
string[0] = Ctrl__;
add_to_input_buf(string, 1);
}

else if (vk == '2' || MapVirtualKey(vk, 2) == (UINT)'@')
{
string[0] = Ctrl_AT;
add_to_input_buf(string, 1);
}
else
MyTranslateMessage(&msg);
}
else
MyTranslateMessage(&msg);
}
}
#if defined(FEAT_MBYTE_IME)
else if (msg.message == WM_IME_NOTIFY)
_OnImeNotify(msg.hwnd, (DWORD)msg.wParam, (DWORD)msg.lParam);
else if (msg.message == WM_KEYUP && im_get_status())

MyTranslateMessage(&msg);
#endif
#if !defined(FEAT_MBYTE_IME) && defined(GLOBAL_IME)

else if (msg.message == WM_IME_STARTCOMPOSITION)
{
POINT point;

global_ime_set_font(&norm_logfont);
point.x = FILL_X(gui.col);
point.y = FILL_Y(gui.row);
MapWindowPoints(s_textArea, s_hwnd, &point, 1);
global_ime_set_position(&point);
}
#endif

#if defined(FEAT_MENU)



if (vk != VK_F10 || check_map(k10, State, FALSE, TRUE, FALSE,
NULL, NULL) == NULL)
#endif
pDispatchMessage(&msg);
}







void
gui_mch_update(void)
{
MSG msg;

if (!s_busy_processing)
while (pPeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)
&& !vim_is_input_buf_full())
process_message();
}

static void
remove_any_timer(void)
{
MSG msg;

if (s_wait_timer != 0 && !s_timed_out)
{
KillTimer(NULL, s_wait_timer);


while (pPeekMessage(&msg, s_hwnd, WM_TIMER, WM_TIMER, PM_REMOVE))
;
s_wait_timer = 0;
}
}










int
gui_mch_wait_for_chars(int wtime)
{
int focus;

s_timed_out = FALSE;

if (wtime >= 0)
{

if (s_busy_processing)
return FAIL;


s_wait_timer = (UINT)SetTimer(NULL, 0, (UINT)(wtime == 0 ? 1 : wtime),
(TIMERPROC)_OnTimer);
}

allow_scrollbar = TRUE;

focus = gui.in_focus;
while (!s_timed_out)
{

if (gui.in_focus != focus)
{
if (gui.in_focus)
gui_mch_start_blink();
else
gui_mch_stop_blink(TRUE);
focus = gui.in_focus;
}

if (s_need_activate)
{
(void)SetForegroundWindow(s_hwnd);
s_need_activate = FALSE;
}

#if defined(FEAT_TIMERS)
did_add_timer = FALSE;
#endif
#if defined(MESSAGE_QUEUE)

for (;;)
{
MSG msg;

parse_queued_messages();
#if defined(FEAT_TIMERS)
if (did_add_timer)
break;
#endif
if (pPeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
{
process_message();
break;
}
else if (input_available()
|| MsgWaitForMultipleObjects(0, NULL, FALSE, 100,
QS_ALLINPUT) != WAIT_TIMEOUT)
break;
}
#else




process_message();
#endif

if (input_available())
{
remove_any_timer();
allow_scrollbar = FALSE;




if (!s_getting_focus)
s_button_pending = -1;

return OK;
}

#if defined(FEAT_TIMERS)
if (did_add_timer)
{

remove_any_timer();
break;
}
#endif
}
allow_scrollbar = FALSE;
return FAIL;
}





void
gui_mch_clear_block(
int row1,
int col1,
int row2,
int col2)
{
RECT rc;






rc.left = FILL_X(col1);
rc.top = FILL_Y(row1);
rc.right = FILL_X(col2 + 1) + (col2 == Columns - 1);
rc.bottom = FILL_Y(row2 + 1);
clear_rect(&rc);
}




void
gui_mch_clear_all(void)
{
RECT rc;

rc.left = 0;
rc.top = 0;
rc.right = Columns * gui.char_width + 2 * gui.border_width;
rc.bottom = Rows * gui.char_height + 2 * gui.border_width;
clear_rect(&rc);
}




void
gui_mch_enable_menu(int flag)
{
#if defined(FEAT_MENU)
SetMenu(s_hwnd, flag ? s_menuBar : NULL);
#endif
}

void
gui_mch_set_menu_pos(
int x UNUSED,
int y UNUSED,
int w UNUSED,
int h UNUSED)
{

}

#if defined(FEAT_MENU) || defined(PROTO)



void
gui_mch_menu_hidden(
vimmenu_T *menu,
int hidden)
{









gui_mch_menu_grey(menu, hidden);
}




void
gui_mch_draw_menubar(void)
{
DrawMenuBar(s_hwnd);
}
#endif 




guicolor_T
gui_mch_get_rgb(guicolor_T pixel)
{
return (guicolor_T)((GetRValue(pixel) << 16) + (GetGValue(pixel) << 8)
+ GetBValue(pixel));
}

#if defined(FEAT_GUI_DIALOG) || defined(PROTO)



static WORD
PixelToDialogX(int numPixels)
{
return (WORD)((numPixels * 4) / s_dlgfntwidth);
}




static WORD
PixelToDialogY(int numPixels)
{
return (WORD)((numPixels * 8) / s_dlgfntheight);
}




static int
GetTextWidth(HDC hdc, char_u *str, int len)
{
SIZE size;

GetTextExtentPoint(hdc, (LPCSTR)str, len, &size);
return size.cx;
}





static int
GetTextWidthEnc(HDC hdc, char_u *str, int len)
{
SIZE size;
WCHAR *wstr;
int n;
int wlen = len;

wstr = enc_to_utf16(str, &wlen);
if (wstr == NULL)
return 0;

n = GetTextExtentPointW(hdc, wstr, wlen, &size);
vim_free(wstr);
if (n)
return size.cx;
return 0;
}

static void get_work_area(RECT *spi_rect);






static BOOL
CenterWindow(
HWND hwndChild,
HWND hwndParent)
{
HMONITOR mon;
MONITORINFO moninfo;
RECT rChild, rParent, rScreen;
int wChild, hChild, wParent, hParent;
int xNew, yNew;
HDC hdc;

GetWindowRect(hwndChild, &rChild);
wChild = rChild.right - rChild.left;
hChild = rChild.bottom - rChild.top;


if (hwndParent == NULL || IsMinimized(hwndParent))
get_work_area(&rParent);
else
GetWindowRect(hwndParent, &rParent);
wParent = rParent.right - rParent.left;
hParent = rParent.bottom - rParent.top;

moninfo.cbSize = sizeof(MONITORINFO);
mon = MonitorFromWindow(hwndChild, MONITOR_DEFAULTTOPRIMARY);
if (mon != NULL && GetMonitorInfo(mon, &moninfo))
{
rScreen = moninfo.rcWork;
}
else
{
hdc = GetDC(hwndChild);
rScreen.left = 0;
rScreen.top = 0;
rScreen.right = GetDeviceCaps(hdc, HORZRES);
rScreen.bottom = GetDeviceCaps(hdc, VERTRES);
ReleaseDC(hwndChild, hdc);
}

xNew = rParent.left + ((wParent - wChild) / 2);
if (xNew < rScreen.left)
xNew = rScreen.left;
else if ((xNew + wChild) > rScreen.right)
xNew = rScreen.right - wChild;

yNew = rParent.top + ((hParent - hChild) / 2);
if (yNew < rScreen.top)
yNew = rScreen.top;
else if ((yNew + hChild) > rScreen.bottom)
yNew = rScreen.bottom - hChild;

return SetWindowPos(hwndChild, NULL, xNew, yNew, 0, 0,
SWP_NOSIZE | SWP_NOZORDER);
}
#endif 

#if defined(FEAT_TOOLBAR) || defined(PROTO)
void
gui_mch_show_toolbar(int showit)
{
if (s_toolbarhwnd == NULL)
return;

if (showit)
{
#if !defined(TB_SETUNICODEFORMAT)

#define TB_SETUNICODEFORMAT 0x2005
#endif

SendMessage(s_toolbarhwnd, TB_SETUNICODEFORMAT, (WPARAM)TRUE,
(LPARAM)0);
ShowWindow(s_toolbarhwnd, SW_SHOW);
}
else
ShowWindow(s_toolbarhwnd, SW_HIDE);
}


#define TOOLBAR_BITMAP_COUNT 31

#endif

#if defined(FEAT_GUI_TABLINE) || defined(PROTO)
static void
add_tabline_popup_menu_entry(HMENU pmenu, UINT item_id, char_u *item_text)
{
WCHAR *wn;
MENUITEMINFOW infow;

wn = enc_to_utf16(item_text, NULL);
if (wn == NULL)
return;

infow.cbSize = sizeof(infow);
infow.fMask = MIIM_TYPE | MIIM_ID;
infow.wID = item_id;
infow.fType = MFT_STRING;
infow.dwTypeData = wn;
infow.cch = (UINT)wcslen(wn);
InsertMenuItemW(pmenu, item_id, FALSE, &infow);
vim_free(wn);
}

static void
show_tabline_popup_menu(void)
{
HMENU tab_pmenu;
long rval;
POINT pt;


if (hold_gui_events
#if defined(FEAT_CMDWIN)
|| cmdwin_type != 0
#endif
)
return;

tab_pmenu = CreatePopupMenu();
if (tab_pmenu == NULL)
return;

if (first_tabpage->tp_next != NULL)
add_tabline_popup_menu_entry(tab_pmenu,
TABLINE_MENU_CLOSE, (char_u *)_("Close tab"));
add_tabline_popup_menu_entry(tab_pmenu,
TABLINE_MENU_NEW, (char_u *)_("New tab"));
add_tabline_popup_menu_entry(tab_pmenu,
TABLINE_MENU_OPEN, (char_u *)_("Open tab..."));

GetCursorPos(&pt);
rval = TrackPopupMenuEx(tab_pmenu, TPM_RETURNCMD, pt.x, pt.y, s_tabhwnd,
NULL);

DestroyMenu(tab_pmenu);


if (rval > 0)
{
TCHITTESTINFO htinfo;
int idx;

if (ScreenToClient(s_tabhwnd, &pt) == 0)
return;

htinfo.pt.x = pt.x;
htinfo.pt.y = pt.y;
idx = TabCtrl_HitTest(s_tabhwnd, &htinfo);
if (idx == -1)
idx = 0;
else
idx += 1;

send_tabline_menu_event(idx, (int)rval);
}
}




void
gui_mch_show_tabline(int showit)
{
if (s_tabhwnd == NULL)
return;

if (!showit != !showing_tabline)
{
if (showit)
ShowWindow(s_tabhwnd, SW_SHOW);
else
ShowWindow(s_tabhwnd, SW_HIDE);
showing_tabline = showit;
}
}




int
gui_mch_showing_tabline(void)
{
return s_tabhwnd != NULL && showing_tabline;
}




void
gui_mch_update_tabline(void)
{
tabpage_T *tp;
TCITEM tie;
int nr = 0;
int curtabidx = 0;
int tabadded = 0;
WCHAR *wstr = NULL;

if (s_tabhwnd == NULL)
return;

#if !defined(CCM_SETUNICODEFORMAT)

#define CCM_SETUNICODEFORMAT 0x2005
#endif

SendMessage(s_tabhwnd, CCM_SETUNICODEFORMAT, (WPARAM)TRUE, (LPARAM)0);

tie.mask = TCIF_TEXT;
tie.iImage = -1;


SendMessage(s_tabhwnd, WM_SETREDRAW, (WPARAM)FALSE, 0);


for (tp = first_tabpage; tp != NULL; tp = tp->tp_next, ++nr)
{
if (tp == curtab)
curtabidx = nr;

if (nr >= TabCtrl_GetItemCount(s_tabhwnd))
{

tie.pszText = "-Empty-";
TabCtrl_InsertItem(s_tabhwnd, nr, &tie);
tabadded = 1;
}

get_tabline_label(tp, FALSE);
tie.pszText = (LPSTR)NameBuff;

wstr = enc_to_utf16(NameBuff, NULL);
if (wstr != NULL)
{
TCITEMW tiw;

tiw.mask = TCIF_TEXT;
tiw.iImage = -1;
tiw.pszText = wstr;
SendMessage(s_tabhwnd, TCM_SETITEMW, (WPARAM)nr, (LPARAM)&tiw);
vim_free(wstr);
}
}


while (nr < TabCtrl_GetItemCount(s_tabhwnd))
TabCtrl_DeleteItem(s_tabhwnd, nr);

if (!tabadded && TabCtrl_GetCurSel(s_tabhwnd) != curtabidx)
TabCtrl_SetCurSel(s_tabhwnd, curtabidx);


SendMessage(s_tabhwnd, WM_SETREDRAW, (WPARAM)TRUE, 0);
RedrawWindow(s_tabhwnd, NULL, NULL,
RDW_ERASE | RDW_FRAME | RDW_INVALIDATE | RDW_ALLCHILDREN);

if (tabadded && TabCtrl_GetCurSel(s_tabhwnd) != curtabidx)
TabCtrl_SetCurSel(s_tabhwnd, curtabidx);
}




void
gui_mch_set_curtab(int nr)
{
if (s_tabhwnd == NULL)
return;

if (TabCtrl_GetCurSel(s_tabhwnd) != nr - 1)
TabCtrl_SetCurSel(s_tabhwnd, nr - 1);
}

#endif




void
ex_simalt(exarg_T *eap)
{
char_u *keys = eap->arg;
int fill_typebuf = FALSE;
char_u key_name[4];

PostMessage(s_hwnd, WM_SYSCOMMAND, (WPARAM)SC_KEYMENU, (LPARAM)0);
while (*keys)
{
if (*keys == '~')
*keys = ' '; 
PostMessage(s_hwnd, WM_CHAR, (WPARAM)*keys, (LPARAM)0);
keys++;
fill_typebuf = TRUE;
}
if (fill_typebuf)
{


key_name[0] = K_SPECIAL;
key_name[1] = KS_EXTRA;
key_name[2] = KE_NOP;
key_name[3] = NUL;
#if defined(FEAT_CLIENTSERVER) || defined(FEAT_EVAL)
typebuf_was_filled = TRUE;
#endif
(void)ins_typebuf(key_name, REMAP_NONE, 0, TRUE, FALSE);
}
}






#if defined(MSWIN_FIND_REPLACE)
static void
initialise_findrep(char_u *initial_string)
{
int wword = FALSE;
int mcase = !p_ic;
char_u *entry_text;


entry_text = get_find_dialog_text(initial_string, &wword, &mcase);

s_findrep_struct.hwndOwner = s_hwnd;
s_findrep_struct.Flags = FR_DOWN;
if (mcase)
s_findrep_struct.Flags |= FR_MATCHCASE;
if (wword)
s_findrep_struct.Flags |= FR_WHOLEWORD;
if (entry_text != NULL && *entry_text != NUL)
{
WCHAR *p = enc_to_utf16(entry_text, NULL);
if (p != NULL)
{
int len = s_findrep_struct.wFindWhatLen - 1;

wcsncpy(s_findrep_struct.lpstrFindWhat, p, len);
s_findrep_struct.lpstrFindWhat[len] = NUL;
vim_free(p);
}
}
vim_free(entry_text);
}
#endif

static void
set_window_title(HWND hwnd, char *title)
{
if (title != NULL)
{
WCHAR *wbuf;


wbuf = (WCHAR *)enc_to_utf16((char_u *)title, NULL);
if (wbuf != NULL)
{
SetWindowTextW(hwnd, wbuf);
vim_free(wbuf);
}
}
else
(void)SetWindowTextW(hwnd, NULL);
}

void
gui_mch_find_dialog(exarg_T *eap)
{
#if defined(MSWIN_FIND_REPLACE)
if (s_findrep_msg != 0)
{
if (IsWindow(s_findrep_hwnd) && !s_findrep_is_find)
DestroyWindow(s_findrep_hwnd);

if (!IsWindow(s_findrep_hwnd))
{
initialise_findrep(eap->arg);
s_findrep_hwnd = FindTextW((LPFINDREPLACEW) &s_findrep_struct);
}

set_window_title(s_findrep_hwnd, _("Find string"));
(void)SetFocus(s_findrep_hwnd);

s_findrep_is_find = TRUE;
}
#endif
}


void
gui_mch_replace_dialog(exarg_T *eap)
{
#if defined(MSWIN_FIND_REPLACE)
if (s_findrep_msg != 0)
{
if (IsWindow(s_findrep_hwnd) && s_findrep_is_find)
DestroyWindow(s_findrep_hwnd);

if (!IsWindow(s_findrep_hwnd))
{
initialise_findrep(eap->arg);
s_findrep_hwnd = ReplaceTextW((LPFINDREPLACEW) &s_findrep_struct);
}

set_window_title(s_findrep_hwnd, _("Find & Replace"));
(void)SetFocus(s_findrep_hwnd);

s_findrep_is_find = FALSE;
}
#endif
}





void
gui_mch_mousehide(int hide)
{
if (hide != gui.pointer_hidden)
{
ShowCursor(!hide);
gui.pointer_hidden = hide;
}
}

#if defined(FEAT_MENU)
static void
gui_mch_show_popupmenu_at(vimmenu_T *menu, int x, int y)
{

gui_mch_mousehide(FALSE);

(void)TrackPopupMenu(
(HMENU)menu->submenu_id,
TPM_LEFTALIGN | TPM_LEFTBUTTON,
x, y,
(int)0, 
s_hwnd,
NULL);




}
#endif




static void
_OnEndSession(void)
{
getout_preserve_modified(1);
}





static void
_OnClose(HWND hwnd UNUSED)
{
gui_shell_closed();
}




static void
_OnDestroy(HWND hwnd)
{
if (!destroying)
_OnClose(hwnd);
}

static void
_OnPaint(
HWND hwnd)
{
if (!IsMinimized(hwnd))
{
PAINTSTRUCT ps;

out_flush(); 
(void)BeginPaint(hwnd, &ps);



if (has_mbyte)
{
RECT rect;

GetClientRect(hwnd, &rect);
ps.rcPaint.left = rect.left;
ps.rcPaint.right = rect.right;
}

if (!IsRectEmpty(&ps.rcPaint))
{
gui_redraw(ps.rcPaint.left, ps.rcPaint.top,
ps.rcPaint.right - ps.rcPaint.left + 1,
ps.rcPaint.bottom - ps.rcPaint.top + 1);
}

EndPaint(hwnd, &ps);
}
}

static void
_OnSize(
HWND hwnd,
UINT state UNUSED,
int cx,
int cy)
{
if (!IsMinimized(hwnd))
{
gui_resize_shell(cx, cy);

#if defined(FEAT_MENU)

gui_mswin_get_menu_height(TRUE);
#endif
}
}

static void
_OnSetFocus(
HWND hwnd,
HWND hwndOldFocus)
{
gui_focus_change(TRUE);
s_getting_focus = TRUE;
(void)MyWindowProc(hwnd, WM_SETFOCUS, (WPARAM)hwndOldFocus, 0);
}

static void
_OnKillFocus(
HWND hwnd,
HWND hwndNewFocus)
{
gui_focus_change(FALSE);
s_getting_focus = FALSE;
(void)MyWindowProc(hwnd, WM_KILLFOCUS, (WPARAM)hwndNewFocus, 0);
}




static LRESULT
_OnActivateApp(
HWND hwnd,
BOOL fActivate,
DWORD dwThreadId)
{


return MyWindowProc(hwnd, WM_ACTIVATEAPP, fActivate, (DWORD)dwThreadId);
}

void
gui_mch_destroy_scrollbar(scrollbar_T *sb)
{
DestroyWindow(sb->id);
}




void
gui_mch_getmouse(int *x, int *y)
{
RECT rct;
POINT mp;

(void)GetWindowRect(s_textArea, &rct);
(void)GetCursorPos((LPPOINT)&mp);
*x = (int)(mp.x - rct.left);
*y = (int)(mp.y - rct.top);
}




void
gui_mch_setmouse(int x, int y)
{
RECT rct;

(void)GetWindowRect(s_textArea, &rct);
(void)SetCursorPos(x + gui.border_offset + rct.left,
y + gui.border_offset + rct.top);
}

static void
gui_mswin_get_valid_dimensions(
int w,
int h,
int *valid_w,
int *valid_h)
{
int base_width, base_height;

base_width = gui_get_base_width()
+ (GetSystemMetrics(SM_CXFRAME) +
GetSystemMetrics(SM_CXPADDEDBORDER)) * 2;
base_height = gui_get_base_height()
+ (GetSystemMetrics(SM_CYFRAME) +
GetSystemMetrics(SM_CXPADDEDBORDER)) * 2
+ GetSystemMetrics(SM_CYCAPTION)
#if defined(FEAT_MENU)
+ gui_mswin_get_menu_height(FALSE)
#endif
;
*valid_w = base_width +
((w - base_width) / gui.char_width) * gui.char_width;
*valid_h = base_height +
((h - base_height) / gui.char_height) * gui.char_height;
}

void
gui_mch_flash(int msec)
{
RECT rc;

#if defined(FEAT_DIRECTX)
if (IS_ENABLE_DIRECTX())
DWriteContext_Flush(s_dwc);
#endif




rc.left = 0;
rc.top = 0;
rc.right = gui.num_cols * gui.char_width;
rc.bottom = gui.num_rows * gui.char_height;
InvertRect(s_hdc, &rc);
gui_mch_flush(); 

ui_delay((long)msec, TRUE); 

InvertRect(s_hdc, &rc);
}






static int
get_scroll_flags(void)
{
HWND hwnd;
RECT rcVim, rcOther, rcDest;

GetWindowRect(s_hwnd, &rcVim);




if (rcVim.top < 0 || rcVim.bottom > GetSystemMetrics(SM_CYFULLSCREEN))
return SW_INVALIDATE;


for (hwnd = s_hwnd; (hwnd = GetWindow(hwnd, GW_HWNDPREV)) != (HWND)0; )
if (IsWindowVisible(hwnd))
{
GetWindowRect(hwnd, &rcOther);
if (IntersectRect(&rcDest, &rcVim, &rcOther))
return SW_INVALIDATE;
}
return 0;
}









static void
intel_gpu_workaround(void)
{
GetPixel(s_hdc, FILL_X(gui.col), FILL_Y(gui.row));
}





void
gui_mch_delete_lines(
int row,
int num_lines)
{
RECT rc;

rc.left = FILL_X(gui.scroll_region_left);
rc.right = FILL_X(gui.scroll_region_right + 1);
rc.top = FILL_Y(row);
rc.bottom = FILL_Y(gui.scroll_region_bot + 1);

#if defined(FEAT_DIRECTX)
if (IS_ENABLE_DIRECTX())
{
DWriteContext_Scroll(s_dwc, 0, -num_lines * gui.char_height, &rc);
DWriteContext_Flush(s_dwc);
}
else
#endif
{
intel_gpu_workaround();
ScrollWindowEx(s_textArea, 0, -num_lines * gui.char_height,
&rc, &rc, NULL, NULL, get_scroll_flags());
UpdateWindow(s_textArea);
}







gui_clear_block(gui.scroll_region_bot - num_lines + 1,
gui.scroll_region_left,
gui.scroll_region_bot, gui.scroll_region_right);
}





void
gui_mch_insert_lines(
int row,
int num_lines)
{
RECT rc;

rc.left = FILL_X(gui.scroll_region_left);
rc.right = FILL_X(gui.scroll_region_right + 1);
rc.top = FILL_Y(row);
rc.bottom = FILL_Y(gui.scroll_region_bot + 1);

#if defined(FEAT_DIRECTX)
if (IS_ENABLE_DIRECTX())
{
DWriteContext_Scroll(s_dwc, 0, num_lines * gui.char_height, &rc);
DWriteContext_Flush(s_dwc);
}
else
#endif
{
intel_gpu_workaround();


ScrollWindowEx(s_textArea, 0, num_lines * gui.char_height,
&rc, &rc, NULL, NULL, get_scroll_flags());
UpdateWindow(s_textArea);
}

gui_clear_block(row, gui.scroll_region_left,
row + num_lines - 1, gui.scroll_region_right);
}


void
gui_mch_exit(int rc UNUSED)
{
#if defined(FEAT_DIRECTX)
DWriteContext_Close(s_dwc);
DWrite_Final();
s_dwc = NULL;
#endif

ReleaseDC(s_textArea, s_hdc);
DeleteObject(s_brush);

#if defined(FEAT_TEAROFF)

(void)DeleteObject((HGDIOBJ)s_htearbitmap);
#endif


if (s_hwnd != NULL)
{
destroying = TRUE; 
DestroyWindow(s_hwnd);
}

#if defined(GLOBAL_IME)
global_ime_end();
#endif
}

static char_u *
logfont2name(LOGFONTW lf)
{
char *p;
char *res;
char *charset_name;
char *quality_name;
char *font_name;
int points;

font_name = (char *)utf16_to_enc(lf.lfFaceName, NULL);
if (font_name == NULL)
return NULL;
charset_name = charset_id2name((int)lf.lfCharSet);
quality_name = quality_id2name((int)lf.lfQuality);

res = alloc(strlen(font_name) + 30
+ (charset_name == NULL ? 0 : strlen(charset_name) + 2)
+ (quality_name == NULL ? 0 : strlen(quality_name) + 2));
if (res != NULL)
{
p = res;

points = pixels_to_points(
lf.lfHeight < 0 ? -lf.lfHeight : lf.lfHeight, TRUE);
if (lf.lfWeight == FW_NORMAL || lf.lfWeight == FW_BOLD)
sprintf((char *)p, "%s:h%d", font_name, points);
else
sprintf((char *)p, "%s:h%d:W%ld", font_name, points, lf.lfWeight);
while (*p)
{
if (*p == ' ')
*p = '_';
++p;
}
if (lf.lfItalic)
STRCAT(p, ":i");
if (lf.lfWeight == FW_BOLD)
STRCAT(p, ":b");
if (lf.lfUnderline)
STRCAT(p, ":u");
if (lf.lfStrikeOut)
STRCAT(p, ":s");
if (charset_name != NULL)
{
STRCAT(p, ":c");
STRCAT(p, charset_name);
}
if (quality_name != NULL)
{
STRCAT(p, ":q");
STRCAT(p, quality_name);
}
}

vim_free(font_name);
return (char_u *)res;
}


#if defined(FEAT_MBYTE_IME)




static void
update_im_font(void)
{
LOGFONTW lf_wide;

if (p_guifontwide != NULL && *p_guifontwide != NUL
&& gui.wide_font != NOFONT
&& GetObjectW((HFONT)gui.wide_font, sizeof(lf_wide), &lf_wide))
norm_logfont = lf_wide;
else
norm_logfont = sub_logfont;
im_set_font(&norm_logfont);
}
#endif




void
gui_mch_wide_font_changed(void)
{
LOGFONTW lf;

#if defined(FEAT_MBYTE_IME)
update_im_font();
#endif

gui_mch_free_font(gui.wide_ital_font);
gui.wide_ital_font = NOFONT;
gui_mch_free_font(gui.wide_bold_font);
gui.wide_bold_font = NOFONT;
gui_mch_free_font(gui.wide_boldital_font);
gui.wide_boldital_font = NOFONT;

if (gui.wide_font
&& GetObjectW((HFONT)gui.wide_font, sizeof(lf), &lf))
{
if (!lf.lfItalic)
{
lf.lfItalic = TRUE;
gui.wide_ital_font = get_font_handle(&lf);
lf.lfItalic = FALSE;
}
if (lf.lfWeight < FW_BOLD)
{
lf.lfWeight = FW_BOLD;
gui.wide_bold_font = get_font_handle(&lf);
if (!lf.lfItalic)
{
lf.lfItalic = TRUE;
gui.wide_boldital_font = get_font_handle(&lf);
}
}
}
}





int
gui_mch_init_font(char_u *font_name, int fontset UNUSED)
{
LOGFONTW lf;
GuiFont font = NOFONT;
char_u *p;


if (get_logfont(&lf, font_name, NULL, TRUE) == OK)
font = get_font_handle(&lf);
if (font == NOFONT)
return FAIL;

if (font_name == NULL)
font_name = (char_u *)lf.lfFaceName;
#if defined(FEAT_MBYTE_IME) || defined(GLOBAL_IME)
norm_logfont = lf;
#endif
#if defined(FEAT_MBYTE_IME)
sub_logfont = lf;
#endif
#if defined(FEAT_MBYTE_IME)
update_im_font();
#endif
gui_mch_free_font(gui.norm_font);
gui.norm_font = font;
current_font_height = lf.lfHeight;
GetFontSize(font);

p = logfont2name(lf);
if (p != NULL)
{
hl_set_font_name(p);



if (STRCMP(font_name, "*") == 0 && STRCMP(p_guifont, "*") == 0)
{
vim_free(p_guifont);
p_guifont = p;
}
else
vim_free(p);
}

gui_mch_free_font(gui.ital_font);
gui.ital_font = NOFONT;
gui_mch_free_font(gui.bold_font);
gui.bold_font = NOFONT;
gui_mch_free_font(gui.boldital_font);
gui.boldital_font = NOFONT;

if (!lf.lfItalic)
{
lf.lfItalic = TRUE;
gui.ital_font = get_font_handle(&lf);
lf.lfItalic = FALSE;
}
if (lf.lfWeight < FW_BOLD)
{
lf.lfWeight = FW_BOLD;
gui.bold_font = get_font_handle(&lf);
if (!lf.lfItalic)
{
lf.lfItalic = TRUE;
gui.boldital_font = get_font_handle(&lf);
}
}

return OK;
}

#if !defined(WPF_RESTORETOMAXIMIZED)
#define WPF_RESTORETOMAXIMIZED 2 
#endif




int
gui_mch_maximized(void)
{
WINDOWPLACEMENT wp;

wp.length = sizeof(WINDOWPLACEMENT);
if (GetWindowPlacement(s_hwnd, &wp))
return wp.showCmd == SW_SHOWMAXIMIZED
|| (wp.showCmd == SW_SHOWMINIMIZED
&& wp.flags == WPF_RESTORETOMAXIMIZED);

return 0;
}






void
gui_mch_newfont(void)
{
RECT rect;

GetWindowRect(s_hwnd, &rect);
if (win_socket_id == 0)
{
gui_resize_shell(rect.right - rect.left
- (GetSystemMetrics(SM_CXFRAME) +
GetSystemMetrics(SM_CXPADDEDBORDER)) * 2,
rect.bottom - rect.top
- (GetSystemMetrics(SM_CYFRAME) +
GetSystemMetrics(SM_CXPADDEDBORDER)) * 2
- GetSystemMetrics(SM_CYCAPTION)
#if defined(FEAT_MENU)
- gui_mswin_get_menu_height(FALSE)
#endif
);
}
else
{

gui_resize_shell(rect.right - rect.left,
rect.bottom - rect.top
#if defined(FEAT_MENU)
- gui_mswin_get_menu_height(FALSE)
#endif
);
}
}




void
gui_mch_settitle(
char_u *title,
char_u *icon UNUSED)
{
set_window_title(s_hwnd, (title == NULL ? "VIM" : (char *)title));
}

#if defined(FEAT_MOUSESHAPE) || defined(PROTO)


static LPCSTR mshape_idcs[] =
{
IDC_ARROW, 
MAKEINTRESOURCE(0), 
IDC_IBEAM, 
IDC_SIZENS, 
IDC_SIZENS, 
IDC_SIZEWE, 
IDC_SIZEWE, 
IDC_WAIT, 
IDC_NO, 
IDC_ARROW, 
IDC_ARROW, 
IDC_ARROW, 
IDC_ARROW, 
IDC_ARROW, 
IDC_ARROW, 
IDC_UPARROW, 
IDC_ARROW 
};

void
mch_set_mouse_shape(int shape)
{
LPCSTR idc;

if (shape == MSHAPE_HIDE)
ShowCursor(FALSE);
else
{
if (shape >= MSHAPE_NUMBERED)
idc = IDC_ARROW;
else
idc = mshape_idcs[shape];
SetClassLongPtr(s_textArea, GCLP_HCURSOR, (LONG_PTR)LoadCursor(NULL, idc));
if (!p_mh)
{
POINT mp;


(void)GetCursorPos((LPPOINT)&mp);
(void)SetCursorPos(mp.x, mp.y);
ShowCursor(TRUE);
}
}
}
#endif

#if defined(FEAT_BROWSE) || defined(PROTO)



static WCHAR *
convert_filterW(char_u *s)
{
char_u *tmp;
int len;
WCHAR *res;

tmp = convert_filter(s);
if (tmp == NULL)
return NULL;
len = (int)STRLEN(s) + 3;
res = enc_to_utf16(tmp, &len);
vim_free(tmp);
return res;
}











char_u *
gui_mch_browse(
int saving,
char_u *title,
char_u *dflt,
char_u *ext,
char_u *initdir,
char_u *filter)
{


OPENFILENAMEW fileStruct;
WCHAR fileBuf[MAXPATHL];
WCHAR *wp;
int i;
WCHAR *titlep = NULL;
WCHAR *extp = NULL;
WCHAR *initdirp = NULL;
WCHAR *filterp;
char_u *p, *q;

if (dflt == NULL)
fileBuf[0] = NUL;
else
{
wp = enc_to_utf16(dflt, NULL);
if (wp == NULL)
fileBuf[0] = NUL;
else
{
for (i = 0; wp[i] != NUL && i < MAXPATHL - 1; ++i)
fileBuf[i] = wp[i];
fileBuf[i] = NUL;
vim_free(wp);
}
}


filterp = convert_filterW(filter);

vim_memset(&fileStruct, 0, sizeof(OPENFILENAMEW));
#if defined(OPENFILENAME_SIZE_VERSION_400W)

fileStruct.lStructSize = OPENFILENAME_SIZE_VERSION_400W;
#else
fileStruct.lStructSize = sizeof(fileStruct);
#endif

if (title != NULL)
titlep = enc_to_utf16(title, NULL);
fileStruct.lpstrTitle = titlep;

if (ext != NULL)
extp = enc_to_utf16(ext, NULL);
fileStruct.lpstrDefExt = extp;

fileStruct.lpstrFile = fileBuf;
fileStruct.nMaxFile = MAXPATHL;
fileStruct.lpstrFilter = filterp;
fileStruct.hwndOwner = s_hwnd; 

if (initdir != NULL && *initdir != NUL)
{

initdirp = enc_to_utf16(initdir, NULL);
if (initdirp != NULL)
{
for (wp = initdirp; *wp != NUL; ++wp)
if (*wp == '/')
*wp = '\\';
}
fileStruct.lpstrInitialDir = initdirp;
}









fileStruct.Flags = (OFN_NOCHANGEDIR | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY);
#if defined(FEAT_SHORTCUT)
if (curbuf->b_p_bin)
fileStruct.Flags |= OFN_NODEREFERENCELINKS;
#endif
if (saving)
{
if (!GetSaveFileNameW(&fileStruct))
return NULL;
}
else
{
if (!GetOpenFileNameW(&fileStruct))
return NULL;
}

vim_free(filterp);
vim_free(initdirp);
vim_free(titlep);
vim_free(extp);


p = utf16_to_enc(fileBuf, NULL);
if (p == NULL)
return NULL;


SetFocus(s_hwnd);


q = vim_strsave(shorten_fname1(p));
vim_free(p);
return q;
}









static char_u *
convert_filter(char_u *s)
{
char_u *res;
unsigned s_len = (unsigned)STRLEN(s);
unsigned i;

res = alloc(s_len + 3);
if (res != NULL)
{
for (i = 0; i < s_len; ++i)
if (s[i] == '\t' || s[i] == '\n')
res[i] = '\0';
else
res[i] = s[i];
res[s_len] = NUL;

res[s_len + 1] = NUL;
res[s_len + 2] = NUL;
}
return res;
}




char_u *
gui_mch_browsedir(char_u *title, char_u *initdir)
{


return gui_mch_browse(0, title, (char_u *)_("Not Used"), NULL,
initdir, (char_u *)_("Directory\t*.nothing\n"));
}
#endif 

static void
_OnDropFiles(
HWND hwnd UNUSED,
HDROP hDrop)
{
#define BUFPATHLEN _MAX_PATH
#define DRAGQVAL 0xFFFFFFFF
WCHAR wszFile[BUFPATHLEN];
char szFile[BUFPATHLEN];
UINT cFiles = DragQueryFile(hDrop, DRAGQVAL, NULL, 0);
UINT i;
char_u **fnames;
POINT pt;
int_u modifiers = 0;




DragQueryPoint(hDrop, &pt);
MapWindowPoints(s_hwnd, s_textArea, &pt, 1);

reset_VIsual();

fnames = ALLOC_MULT(char_u *, cFiles);

if (fnames != NULL)
for (i = 0; i < cFiles; ++i)
{
if (DragQueryFileW(hDrop, i, wszFile, BUFPATHLEN) > 0)
fnames[i] = utf16_to_enc(wszFile, NULL);
else
{
DragQueryFile(hDrop, i, szFile, BUFPATHLEN);
fnames[i] = vim_strsave((char_u *)szFile);
}
}

DragFinish(hDrop);

if (fnames != NULL)
{
if ((GetKeyState(VK_SHIFT) & 0x8000) != 0)
modifiers |= MOUSE_SHIFT;
if ((GetKeyState(VK_CONTROL) & 0x8000) != 0)
modifiers |= MOUSE_CTRL;
if ((GetKeyState(VK_MENU) & 0x8000) != 0)
modifiers |= MOUSE_ALT;

gui_handle_drop(pt.x, pt.y, modifiers, fnames, cFiles);

s_need_activate = TRUE;
}
}

static int
_OnScroll(
HWND hwnd UNUSED,
HWND hwndCtl,
UINT code,
int pos)
{
static UINT prev_code = 0; 
scrollbar_T *sb, *sb_info;
long val;
int dragging = FALSE;
int dont_scroll_save = dont_scroll;
SCROLLINFO si;

si.cbSize = sizeof(si);
si.fMask = SIF_POS;

sb = gui_mswin_find_scrollbar(hwndCtl);
if (sb == NULL)
return 0;

if (sb->wp != NULL) 
{





sb_info = &sb->wp->w_scrollbars[0];
}
else 
sb_info = sb;
val = sb_info->value;

switch (code)
{
case SB_THUMBTRACK:
val = pos;
dragging = TRUE;
if (sb->scroll_shift > 0)
val <<= sb->scroll_shift;
break;
case SB_LINEDOWN:
val++;
break;
case SB_LINEUP:
val--;
break;
case SB_PAGEDOWN:
val += (sb_info->size > 2 ? sb_info->size - 2 : 1);
break;
case SB_PAGEUP:
val -= (sb_info->size > 2 ? sb_info->size - 2 : 1);
break;
case SB_TOP:
val = 0;
break;
case SB_BOTTOM:
val = sb_info->max;
break;
case SB_ENDSCROLL:
if (prev_code == SB_THUMBTRACK)
{





val = GetScrollPos(hwndCtl, SB_CTL);
if (sb->scroll_shift > 0)
val <<= sb->scroll_shift;
}
break;

default:

return 0;
}
prev_code = code;

si.nPos = (sb->scroll_shift > 0) ? val >> sb->scroll_shift : val;
SetScrollInfo(hwndCtl, SB_CTL, &si, TRUE);




if (sb->wp != NULL)
{
scrollbar_T *sba = sb->wp->w_scrollbars;
HWND id = sba[ (sb == sba + SBAR_LEFT) ? SBAR_RIGHT : SBAR_LEFT].id;

SetScrollInfo(id, SB_CTL, &si, TRUE);
}


s_busy_processing = TRUE;



dont_scroll = !allow_scrollbar;

mch_disable_flush();
gui_drag_scrollbar(sb, val, dragging);
mch_enable_flush();
gui_may_flush();

s_busy_processing = FALSE;
dont_scroll = dont_scroll_save;

return 0;
}


#if defined(FEAT_XPM_W32)
#include "xpm_w32.h"
#endif

#if defined(PROTO)
#define WINAPI
#endif

#if defined(__MINGW32__)




#if !defined(IsMinimized)
#define IsMinimized(hwnd) IsIconic(hwnd)
#endif
#if !defined(IsMaximized)
#define IsMaximized(hwnd) IsZoomed(hwnd)
#endif
#if !defined(SelectFont)
#define SelectFont(hdc, hfont) ((HFONT)SelectObject((hdc), (HGDIOBJ)(HFONT)(hfont)))
#endif
#if !defined(GetStockBrush)
#define GetStockBrush(i) ((HBRUSH)GetStockObject(i))
#endif
#if !defined(DeleteBrush)
#define DeleteBrush(hbr) DeleteObject((HGDIOBJ)(HBRUSH)(hbr))
#endif

#if !defined(HANDLE_WM_RBUTTONDBLCLK)
#define HANDLE_WM_RBUTTONDBLCLK(hwnd, wParam, lParam, fn) ((fn)((hwnd), TRUE, (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (UINT)(wParam)), 0L)

#endif
#if !defined(HANDLE_WM_MBUTTONUP)
#define HANDLE_WM_MBUTTONUP(hwnd, wParam, lParam, fn) ((fn)((hwnd), (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (UINT)(wParam)), 0L)

#endif
#if !defined(HANDLE_WM_MBUTTONDBLCLK)
#define HANDLE_WM_MBUTTONDBLCLK(hwnd, wParam, lParam, fn) ((fn)((hwnd), TRUE, (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (UINT)(wParam)), 0L)

#endif
#if !defined(HANDLE_WM_LBUTTONDBLCLK)
#define HANDLE_WM_LBUTTONDBLCLK(hwnd, wParam, lParam, fn) ((fn)((hwnd), TRUE, (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (UINT)(wParam)), 0L)

#endif
#if !defined(HANDLE_WM_RBUTTONDOWN)
#define HANDLE_WM_RBUTTONDOWN(hwnd, wParam, lParam, fn) ((fn)((hwnd), FALSE, (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (UINT)(wParam)), 0L)

#endif
#if !defined(HANDLE_WM_MOUSEMOVE)
#define HANDLE_WM_MOUSEMOVE(hwnd, wParam, lParam, fn) ((fn)((hwnd), (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (UINT)(wParam)), 0L)

#endif
#if !defined(HANDLE_WM_RBUTTONUP)
#define HANDLE_WM_RBUTTONUP(hwnd, wParam, lParam, fn) ((fn)((hwnd), (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (UINT)(wParam)), 0L)

#endif
#if !defined(HANDLE_WM_MBUTTONDOWN)
#define HANDLE_WM_MBUTTONDOWN(hwnd, wParam, lParam, fn) ((fn)((hwnd), FALSE, (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (UINT)(wParam)), 0L)

#endif
#if !defined(HANDLE_WM_LBUTTONUP)
#define HANDLE_WM_LBUTTONUP(hwnd, wParam, lParam, fn) ((fn)((hwnd), (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (UINT)(wParam)), 0L)

#endif
#if !defined(HANDLE_WM_LBUTTONDOWN)
#define HANDLE_WM_LBUTTONDOWN(hwnd, wParam, lParam, fn) ((fn)((hwnd), FALSE, (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (UINT)(wParam)), 0L)

#endif
#if !defined(HANDLE_WM_SYSCHAR)
#define HANDLE_WM_SYSCHAR(hwnd, wParam, lParam, fn) ((fn)((hwnd), (TCHAR)(wParam), (int)(short)LOWORD(lParam)), 0L)

#endif
#if !defined(HANDLE_WM_ACTIVATEAPP)
#define HANDLE_WM_ACTIVATEAPP(hwnd, wParam, lParam, fn) ((fn)((hwnd), (BOOL)(wParam), (DWORD)(lParam)), 0L)

#endif
#if !defined(HANDLE_WM_WINDOWPOSCHANGING)
#define HANDLE_WM_WINDOWPOSCHANGING(hwnd, wParam, lParam, fn) (LRESULT)(DWORD)(BOOL)(fn)((hwnd), (LPWINDOWPOS)(lParam))

#endif
#if !defined(HANDLE_WM_VSCROLL)
#define HANDLE_WM_VSCROLL(hwnd, wParam, lParam, fn) ((fn)((hwnd), (HWND)(lParam), (UINT)(LOWORD(wParam)), (int)(short)HIWORD(wParam)), 0L)

#endif
#if !defined(HANDLE_WM_SETFOCUS)
#define HANDLE_WM_SETFOCUS(hwnd, wParam, lParam, fn) ((fn)((hwnd), (HWND)(wParam)), 0L)

#endif
#if !defined(HANDLE_WM_KILLFOCUS)
#define HANDLE_WM_KILLFOCUS(hwnd, wParam, lParam, fn) ((fn)((hwnd), (HWND)(wParam)), 0L)

#endif
#if !defined(HANDLE_WM_HSCROLL)
#define HANDLE_WM_HSCROLL(hwnd, wParam, lParam, fn) ((fn)((hwnd), (HWND)(lParam), (UINT)(LOWORD(wParam)), (int)(short)HIWORD(wParam)), 0L)

#endif
#if !defined(HANDLE_WM_DROPFILES)
#define HANDLE_WM_DROPFILES(hwnd, wParam, lParam, fn) ((fn)((hwnd), (HDROP)(wParam)), 0L)

#endif
#if !defined(HANDLE_WM_CHAR)
#define HANDLE_WM_CHAR(hwnd, wParam, lParam, fn) ((fn)((hwnd), (TCHAR)(wParam), (int)(short)LOWORD(lParam)), 0L)

#endif
#if !defined(HANDLE_WM_SYSDEADCHAR)
#define HANDLE_WM_SYSDEADCHAR(hwnd, wParam, lParam, fn) ((fn)((hwnd), (TCHAR)(wParam), (int)(short)LOWORD(lParam)), 0L)

#endif
#if !defined(HANDLE_WM_DEADCHAR)
#define HANDLE_WM_DEADCHAR(hwnd, wParam, lParam, fn) ((fn)((hwnd), (TCHAR)(wParam), (int)(short)LOWORD(lParam)), 0L)

#endif
#endif 



#define TEAROFF_PADDING_X 2
#define TEAROFF_BUTTON_PAD_X 8
#define TEAROFF_MIN_WIDTH 200
#define TEAROFF_SUBMENU_LABEL ">>"
#define TEAROFF_COLUMN_PADDING 3 



#if !defined(WM_MOUSEWHEEL)
#define WM_MOUSEWHEEL 0x20a
#endif


#if defined(FEAT_BEVAL_GUI)
#define ID_BEVAL_TOOLTIP 200
#define BEVAL_TEXT_LEN MAXPATHL

#if (defined(_MSC_VER) && _MSC_VER < 1300) || !defined(MAXULONG_PTR)


#undef UINT_PTR
#define UINT_PTR UINT
#endif

static BalloonEval *cur_beval = NULL;
static UINT_PTR BevalTimerId = 0;
static DWORD LastActivity = 0;



#if !defined(PROTO)





#include <pshpack1.h>

#endif

typedef struct _DllVersionInfo
{
DWORD cbSize;
DWORD dwMajorVersion;
DWORD dwMinorVersion;
DWORD dwBuildNumber;
DWORD dwPlatformID;
} DLLVERSIONINFO;

#if !defined(PROTO)
#include <poppack.h>
#endif

typedef struct tagTOOLINFOA_NEW
{
UINT cbSize;
UINT uFlags;
HWND hwnd;
UINT_PTR uId;
RECT rect;
HINSTANCE hinst;
LPSTR lpszText;
LPARAM lParam;
} TOOLINFO_NEW;

typedef struct tagNMTTDISPINFO_NEW
{
NMHDR hdr;
LPSTR lpszText;
char szText[80];
HINSTANCE hinst;
UINT uFlags;
LPARAM lParam;
} NMTTDISPINFO_NEW;

typedef struct tagTOOLINFOW_NEW
{
UINT cbSize;
UINT uFlags;
HWND hwnd;
UINT_PTR uId;
RECT rect;
HINSTANCE hinst;
LPWSTR lpszText;
LPARAM lParam;
void *lpReserved;
} TOOLINFOW_NEW;

typedef struct tagNMTTDISPINFOW_NEW
{
NMHDR hdr;
LPWSTR lpszText;
WCHAR szText[80];
HINSTANCE hinst;
UINT uFlags;
LPARAM lParam;
} NMTTDISPINFOW_NEW;


typedef HRESULT (WINAPI* DLLGETVERSIONPROC)(DLLVERSIONINFO *);
#if !defined(TTM_SETMAXTIPWIDTH)
#define TTM_SETMAXTIPWIDTH (WM_USER+24)
#endif

#if !defined(TTF_DI_SETITEM)
#define TTF_DI_SETITEM 0x8000
#endif

#if !defined(TTN_GETDISPINFO)
#define TTN_GETDISPINFO (TTN_FIRST - 0)
#endif

#endif 

#if defined(FEAT_TOOLBAR) || defined(FEAT_GUI_TABLINE)




#if !defined(LPNMTTDISPINFO) && defined(_MSC_VER)
typedef struct tagNMTTDISPINFOA {
NMHDR hdr;
LPSTR lpszText;
char szText[80];
HINSTANCE hinst;
UINT uFlags;
LPARAM lParam;
} NMTTDISPINFOA, *LPNMTTDISPINFOA;
#define LPNMTTDISPINFO LPNMTTDISPINFOA

typedef struct tagNMTTDISPINFOW {
NMHDR hdr;
LPWSTR lpszText;
WCHAR szText[80];
HINSTANCE hinst;
UINT uFlags;
LPARAM lParam;
} NMTTDISPINFOW, *LPNMTTDISPINFOW;
#endif
#endif

#if !defined(TTN_GETDISPINFOW)
#define TTN_GETDISPINFOW (TTN_FIRST - 10)
#endif



#if defined(FEAT_MENU)
static UINT s_menu_id = 100;
#endif





#define USE_SYSMENU_FONT

#define VIM_NAME "vim"
#define VIM_CLASSW L"Vim"




#define DLG_ALLOC_SIZE 16 * 1024




static PWORD
add_dialog_element(
PWORD p,
DWORD lStyle,
WORD x,
WORD y,
WORD w,
WORD h,
WORD Id,
WORD clss,
const char *caption);
static LPWORD lpwAlign(LPWORD);
static int nCopyAnsiToWideChar(LPWORD, LPSTR, BOOL);
#if defined(FEAT_MENU) && defined(FEAT_TEAROFF)
static void gui_mch_tearoff(char_u *title, vimmenu_T *menu, int initX, int initY);
#endif
static void get_dialog_font_metrics(void);

static int dialog_default_button = -1;


static int mouse_scroll_lines = 0;

static int s_usenewlook; 
#if defined(FEAT_TOOLBAR)
static void initialise_toolbar(void);
static LRESULT CALLBACK toolbar_wndproc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static int get_toolbar_bitmap(vimmenu_T *menu);
#endif

#if defined(FEAT_GUI_TABLINE)
static void initialise_tabline(void);
static LRESULT CALLBACK tabline_wndproc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#endif

#if defined(FEAT_MBYTE_IME)
static LRESULT _OnImeComposition(HWND hwnd, WPARAM dbcs, LPARAM param);
static char_u *GetResultStr(HWND hwnd, int GCS, int *lenp);
#endif
#if defined(FEAT_MBYTE_IME) && defined(DYNAMIC_IME)
#if defined(NOIME)
typedef struct tagCOMPOSITIONFORM {
DWORD dwStyle;
POINT ptCurrentPos;
RECT rcArea;
} COMPOSITIONFORM, *PCOMPOSITIONFORM, NEAR *NPCOMPOSITIONFORM, FAR *LPCOMPOSITIONFORM;
typedef HANDLE HIMC;
#endif

static HINSTANCE hLibImm = NULL;
static LONG (WINAPI *pImmGetCompositionStringA)(HIMC, DWORD, LPVOID, DWORD);
static LONG (WINAPI *pImmGetCompositionStringW)(HIMC, DWORD, LPVOID, DWORD);
static HIMC (WINAPI *pImmGetContext)(HWND);
static HIMC (WINAPI *pImmAssociateContext)(HWND, HIMC);
static BOOL (WINAPI *pImmReleaseContext)(HWND, HIMC);
static BOOL (WINAPI *pImmGetOpenStatus)(HIMC);
static BOOL (WINAPI *pImmSetOpenStatus)(HIMC, BOOL);
static BOOL (WINAPI *pImmGetCompositionFontW)(HIMC, LPLOGFONTW);
static BOOL (WINAPI *pImmSetCompositionFontW)(HIMC, LPLOGFONTW);
static BOOL (WINAPI *pImmSetCompositionWindow)(HIMC, LPCOMPOSITIONFORM);
static BOOL (WINAPI *pImmGetConversionStatus)(HIMC, LPDWORD, LPDWORD);
static BOOL (WINAPI *pImmSetConversionStatus)(HIMC, DWORD, DWORD);
static void dyn_imm_load(void);
#else
#define pImmGetCompositionStringA ImmGetCompositionStringA
#define pImmGetCompositionStringW ImmGetCompositionStringW
#define pImmGetContext ImmGetContext
#define pImmAssociateContext ImmAssociateContext
#define pImmReleaseContext ImmReleaseContext
#define pImmGetOpenStatus ImmGetOpenStatus
#define pImmSetOpenStatus ImmSetOpenStatus
#define pImmGetCompositionFontW ImmGetCompositionFontW
#define pImmSetCompositionFontW ImmSetCompositionFontW
#define pImmSetCompositionWindow ImmSetCompositionWindow
#define pImmGetConversionStatus ImmGetConversionStatus
#define pImmSetConversionStatus ImmSetConversionStatus
#endif

#if defined(FEAT_MENU)



static int
gui_mswin_get_menu_height(
int fix_window) 
{
static int old_menu_height = -1;

RECT rc1, rc2;
int num;
int menu_height;

if (gui.menu_is_active)
num = GetMenuItemCount(s_menuBar);
else
num = 0;

if (num == 0)
menu_height = 0;
else if (IsMinimized(s_hwnd))
{




menu_height = old_menu_height == -1 ? 0 : old_menu_height;
}
else
{







GetMenuItemRect(s_hwnd, s_menuBar, 0, &rc1);
if (gui.starting)
menu_height = rc1.bottom - rc1.top + 1;
else
{
GetMenuItemRect(s_hwnd, s_menuBar, num - 1, &rc2);
menu_height = rc2.bottom - rc1.top + 1;
}
}

if (fix_window && menu_height != old_menu_height)
gui_set_shellsize(FALSE, FALSE, RESIZE_VERT);
old_menu_height = menu_height;

return menu_height;
}
#endif 





static void
init_mouse_wheel(void)
{

#if !defined(SPI_GETWHEELSCROLLLINES)
#define SPI_GETWHEELSCROLLLINES 104
#endif
#if !defined(SPI_SETWHEELSCROLLLINES)
#define SPI_SETWHEELSCROLLLINES 105
#endif

#define VMOUSEZ_CLASSNAME "MouseZ" 
#define VMOUSEZ_TITLE "Magellan MSWHEEL" 
#define VMSH_MOUSEWHEEL "MSWHEEL_ROLLMSG"
#define VMSH_SCROLL_LINES "MSH_SCROLL_LINES_MSG"

mouse_scroll_lines = 3; 


SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0,
&mouse_scroll_lines, 0);
}






static void
_OnMouseWheel(
HWND hwnd,
short zDelta)
{
int i;
int size;
HWND hwndCtl;
win_T *wp;

if (mouse_scroll_lines == 0)
init_mouse_wheel();

wp = gui_mouse_window(FIND_POPUP);

#if defined(FEAT_PROP_POPUP)
if (wp != NULL && popup_is_popup(wp))
{
cmdarg_T cap;
oparg_T oa;


mouse_row = wp->w_winrow;
mouse_col = wp->w_wincol;
vim_memset(&cap, 0, sizeof(cap));
cap.arg = zDelta < 0 ? MSCR_UP : MSCR_DOWN;
cap.cmdchar = zDelta < 0 ? K_MOUSEUP : K_MOUSEDOWN;
clear_oparg(&oa);
cap.oap = &oa;
nv_mousescroll(&cap);
update_screen(0);
setcursor();
out_flush();
return;
}
#endif

if (wp == NULL || !p_scf)
wp = curwin;

if (wp->w_scrollbars[SBAR_RIGHT].id != 0)
hwndCtl = wp->w_scrollbars[SBAR_RIGHT].id;
else if (wp->w_scrollbars[SBAR_LEFT].id != 0)
hwndCtl = wp->w_scrollbars[SBAR_LEFT].id;
else
return;
size = wp->w_height;

mch_disable_flush();
if (mouse_scroll_lines > 0
&& mouse_scroll_lines < (size > 2 ? size - 2 : 1))
{
for (i = mouse_scroll_lines; i > 0; --i)
_OnScroll(hwnd, hwndCtl, zDelta >= 0 ? SB_LINEUP : SB_LINEDOWN, 0);
}
else
_OnScroll(hwnd, hwndCtl, zDelta >= 0 ? SB_PAGEUP : SB_PAGEDOWN, 0);
mch_enable_flush();
gui_may_flush();
}

#if defined(USE_SYSMENU_FONT)




static int
gui_w32_get_menu_font(LOGFONTW *lf)
{
NONCLIENTMETRICSW nm;

nm.cbSize = sizeof(NONCLIENTMETRICSW);
if (!SystemParametersInfoW(
SPI_GETNONCLIENTMETRICS,
sizeof(NONCLIENTMETRICSW),
&nm,
0))
return FAIL;
*lf = nm.lfMenuFont;
return OK;
}
#endif


#if defined(FEAT_GUI_TABLINE) && defined(USE_SYSMENU_FONT)



static void
set_tabline_font(void)
{
LOGFONTW lfSysmenu;
HFONT font;
HWND hwnd;
HDC hdc;
HFONT hfntOld;
TEXTMETRIC tm;

if (gui_w32_get_menu_font(&lfSysmenu) != OK)
return;

font = CreateFontIndirectW(&lfSysmenu);

SendMessage(s_tabhwnd, WM_SETFONT, (WPARAM)font, TRUE);




hwnd = GetDesktopWindow();
hdc = GetWindowDC(hwnd);
hfntOld = SelectFont(hdc, font);

GetTextMetrics(hdc, &tm);

SelectFont(hdc, hfntOld);
ReleaseDC(hwnd, hdc);





gui.tabline_height = tm.tmHeight + tm.tmInternalLeading + 7;
}
#endif




static LRESULT CALLBACK
_OnSettingChange(UINT n)
{
if (n == SPI_SETWHEELSCROLLLINES)
SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0,
&mouse_scroll_lines, 0);
#if defined(FEAT_GUI_TABLINE) && defined(USE_SYSMENU_FONT)
if (n == SPI_SETNONCLIENTMETRICS)
set_tabline_font();
#endif
return 0;
}

#if defined(FEAT_NETBEANS_INTG)
static void
_OnWindowPosChanged(
HWND hwnd,
const LPWINDOWPOS lpwpos)
{
static int x = 0, y = 0, cx = 0, cy = 0;
extern int WSInitialized;

if (WSInitialized && (lpwpos->x != x || lpwpos->y != y
|| lpwpos->cx != cx || lpwpos->cy != cy))
{
x = lpwpos->x;
y = lpwpos->y;
cx = lpwpos->cx;
cy = lpwpos->cy;
netbeans_frame_moved(x, y);
}

FORWARD_WM_WINDOWPOSCHANGED(hwnd, lpwpos, MyWindowProc);
}
#endif

static int
_DuringSizing(
UINT fwSide,
LPRECT lprc)
{
int w, h;
int valid_w, valid_h;
int w_offset, h_offset;

w = lprc->right - lprc->left;
h = lprc->bottom - lprc->top;
gui_mswin_get_valid_dimensions(w, h, &valid_w, &valid_h);
w_offset = w - valid_w;
h_offset = h - valid_h;

if (fwSide == WMSZ_LEFT || fwSide == WMSZ_TOPLEFT
|| fwSide == WMSZ_BOTTOMLEFT)
lprc->left += w_offset;
else if (fwSide == WMSZ_RIGHT || fwSide == WMSZ_TOPRIGHT
|| fwSide == WMSZ_BOTTOMRIGHT)
lprc->right -= w_offset;

if (fwSide == WMSZ_TOP || fwSide == WMSZ_TOPLEFT
|| fwSide == WMSZ_TOPRIGHT)
lprc->top += h_offset;
else if (fwSide == WMSZ_BOTTOM || fwSide == WMSZ_BOTTOMLEFT
|| fwSide == WMSZ_BOTTOMRIGHT)
lprc->bottom -= h_offset;
return TRUE;
}



static LRESULT CALLBACK
_WndProc(
HWND hwnd,
UINT uMsg,
WPARAM wParam,
LPARAM lParam)
{





HandleMouseHide(uMsg, lParam);

s_uMsg = uMsg;
s_wParam = wParam;
s_lParam = lParam;

switch (uMsg)
{
HANDLE_MSG(hwnd, WM_DEADCHAR, _OnDeadChar);
HANDLE_MSG(hwnd, WM_SYSDEADCHAR, _OnDeadChar);

HANDLE_MSG(hwnd, WM_CLOSE, _OnClose);

HANDLE_MSG(hwnd, WM_DESTROY, _OnDestroy);
HANDLE_MSG(hwnd, WM_DROPFILES, _OnDropFiles);
HANDLE_MSG(hwnd, WM_HSCROLL, _OnScroll);
HANDLE_MSG(hwnd, WM_KILLFOCUS, _OnKillFocus);
#if defined(FEAT_MENU)
HANDLE_MSG(hwnd, WM_COMMAND, _OnMenu);
#endif


HANDLE_MSG(hwnd, WM_SETFOCUS, _OnSetFocus);
HANDLE_MSG(hwnd, WM_SIZE, _OnSize);


HANDLE_MSG(hwnd, WM_VSCROLL, _OnScroll);

HANDLE_MSG(hwnd, WM_ACTIVATEAPP, _OnActivateApp);
#if defined(FEAT_NETBEANS_INTG)
HANDLE_MSG(hwnd, WM_WINDOWPOSCHANGED, _OnWindowPosChanged);
#endif

#if defined(FEAT_GUI_TABLINE)
case WM_RBUTTONUP:
{
if (gui_mch_showing_tabline())
{
POINT pt;
RECT rect;




GetCursorPos((LPPOINT)&pt);
GetWindowRect(s_textArea, &rect);
if (pt.y < rect.top)
{
show_tabline_popup_menu();
return 0L;
}
}
return MyWindowProc(hwnd, uMsg, wParam, lParam);
}
case WM_LBUTTONDBLCLK:
{



if (gui_mch_showing_tabline())
{
POINT pt;
RECT rect;

GetCursorPos((LPPOINT)&pt);
GetWindowRect(s_textArea, &rect);
if (pt.y < rect.top)
send_tabline_menu_event(0, TABLINE_MENU_NEW);
}
return MyWindowProc(hwnd, uMsg, wParam, lParam);
}
#endif

case WM_QUERYENDSESSION: 
gui_shell_closed(); 
return FALSE; 

case WM_ENDSESSION:
if (wParam) 
{
_OnEndSession();
return 0L;
}
break;

case WM_CHAR:


_OnChar(hwnd, (UINT)wParam, (int)(short)LOWORD(lParam));
return 0L;

case WM_SYSCHAR:





#if defined(FEAT_MENU)
if ( !gui.menu_is_active
|| p_wak[0] == 'n'
|| (p_wak[0] == 'm' && !gui_is_menu_shortcut((int)wParam))
)
#endif
{
_OnSysChar(hwnd, (UINT)wParam, (int)(short)LOWORD(lParam));
return 0L;
}
#if defined(FEAT_MENU)
else
return MyWindowProc(hwnd, uMsg, wParam, lParam);
#endif

case WM_SYSKEYUP:
#if defined(FEAT_MENU)




return MyWindowProc(hwnd, uMsg, wParam, lParam);
#else
return 0L;
#endif

case WM_SIZING: 
return _DuringSizing((UINT)wParam, (LPRECT)lParam);

case WM_MOUSEWHEEL:
_OnMouseWheel(hwnd, HIWORD(wParam));
return 0L;


case WM_SETTINGCHANGE:
return _OnSettingChange((UINT)wParam);

#if defined(FEAT_TOOLBAR) || defined(FEAT_GUI_TABLINE)
case WM_NOTIFY:
switch (((LPNMHDR) lParam)->code)
{
case TTN_GETDISPINFOW:
case TTN_GETDISPINFO:
{
LPNMHDR hdr = (LPNMHDR)lParam;
char_u *str = NULL;
static void *tt_text = NULL;

VIM_CLEAR(tt_text);

#if defined(FEAT_GUI_TABLINE)
if (gui_mch_showing_tabline()
&& hdr->hwndFrom == TabCtrl_GetToolTips(s_tabhwnd))
{
POINT pt;






GetCursorPos(&pt);
if (ScreenToClient(s_tabhwnd, &pt) != 0)
{
TCHITTESTINFO htinfo;
int idx;




htinfo.pt.x = pt.x;
htinfo.pt.y = pt.y;
idx = TabCtrl_HitTest(s_tabhwnd, &htinfo);
if (idx != -1)
{
tabpage_T *tp;

tp = find_tabpage(idx + 1);
if (tp != NULL)
{
get_tabline_label(tp, TRUE);
str = NameBuff;
}
}
}
}
#endif
#if defined(FEAT_TOOLBAR)
#if defined(FEAT_GUI_TABLINE)
else
#endif
{
UINT idButton;
vimmenu_T *pMenu;

idButton = (UINT) hdr->idFrom;
pMenu = gui_mswin_find_menu(root_menu, idButton);
if (pMenu)
str = pMenu->strings[MENU_INDEX_TIP];
}
#endif
if (str != NULL)
{
if (hdr->code == TTN_GETDISPINFOW)
{
LPNMTTDISPINFOW lpdi = (LPNMTTDISPINFOW)lParam;



SendMessage(lpdi->hdr.hwndFrom, TTM_SETMAXTIPWIDTH,
0, 500);

tt_text = enc_to_utf16(str, NULL);
lpdi->lpszText = tt_text;

}
else
{
LPNMTTDISPINFO lpdi = (LPNMTTDISPINFO)lParam;



SendMessage(lpdi->hdr.hwndFrom, TTM_SETMAXTIPWIDTH,
0, 500);

if (STRLEN(str) < sizeof(lpdi->szText)
|| ((tt_text = vim_strsave(str)) == NULL))
vim_strncpy((char_u *)lpdi->szText, str,
sizeof(lpdi->szText) - 1);
else
lpdi->lpszText = tt_text;
}
}
}
break;
#if defined(FEAT_GUI_TABLINE)
case TCN_SELCHANGE:
if (gui_mch_showing_tabline()
&& ((LPNMHDR)lParam)->hwndFrom == s_tabhwnd)
{
send_tabline_event(TabCtrl_GetCurSel(s_tabhwnd) + 1);
return 0L;
}
break;

case NM_RCLICK:
if (gui_mch_showing_tabline()
&& ((LPNMHDR)lParam)->hwndFrom == s_tabhwnd)
{
show_tabline_popup_menu();
return 0L;
}
break;
#endif
default:
#if defined(FEAT_GUI_TABLINE)
if (gui_mch_showing_tabline()
&& ((LPNMHDR)lParam)->hwndFrom == s_tabhwnd)
return MyWindowProc(hwnd, uMsg, wParam, lParam);
#endif
break;
}
break;
#endif
#if defined(MENUHINTS) && defined(FEAT_MENU)
case WM_MENUSELECT:
if (((UINT) HIWORD(wParam)
& (0xffff ^ (MF_MOUSESELECT + MF_BITMAP + MF_POPUP)))
== MF_HILITE
&& (State & CMDLINE) == 0)
{
UINT idButton;
vimmenu_T *pMenu;
static int did_menu_tip = FALSE;

if (did_menu_tip)
{
msg_clr_cmdline();
setcursor();
out_flush();
did_menu_tip = FALSE;
}

idButton = (UINT)LOWORD(wParam);
pMenu = gui_mswin_find_menu(root_menu, idButton);
if (pMenu != NULL && pMenu->strings[MENU_INDEX_TIP] != 0
&& GetMenuState(s_menuBar, pMenu->id, MF_BYCOMMAND) != -1)
{
++msg_hist_off;
msg((char *)pMenu->strings[MENU_INDEX_TIP]);
--msg_hist_off;
setcursor();
out_flush();
did_menu_tip = TRUE;
}
return 0L;
}
break;
#endif
case WM_NCHITTEST:
{
LRESULT result;
int x, y;
int xPos = GET_X_LPARAM(lParam);

result = MyWindowProc(hwnd, uMsg, wParam, lParam);
if (result == HTCLIENT)
{
#if defined(FEAT_GUI_TABLINE)
if (gui_mch_showing_tabline())
{
int yPos = GET_Y_LPARAM(lParam);
RECT rct;



GetWindowRect(s_textArea, &rct);
if (yPos < rct.top)
return result;
}
#endif
(void)gui_mch_get_winpos(&x, &y);
xPos -= x;

if (xPos < 48) 
return HTBOTTOMLEFT;
else
return HTBOTTOMRIGHT;
}
else
return result;
}


#if defined(FEAT_MBYTE_IME)
case WM_IME_NOTIFY:
if (!_OnImeNotify(hwnd, (DWORD)wParam, (DWORD)lParam))
return MyWindowProc(hwnd, uMsg, wParam, lParam);
return 1L;

case WM_IME_COMPOSITION:
if (!_OnImeComposition(hwnd, wParam, lParam))
return MyWindowProc(hwnd, uMsg, wParam, lParam);
return 1L;
#endif

default:
#if defined(MSWIN_FIND_REPLACE)
if (uMsg == s_findrep_msg && s_findrep_msg != 0)
_OnFindRepl();
#endif
return MyWindowProc(hwnd, uMsg, wParam, lParam);
}

return DefWindowProc(hwnd, uMsg, wParam, lParam);
}






HWND vim_parent_hwnd = NULL;

static BOOL CALLBACK
FindWindowTitle(HWND hwnd, LPARAM lParam)
{
char buf[2048];
char *title = (char *)lParam;

if (GetWindowText(hwnd, buf, sizeof(buf)))
{
if (strstr(buf, title) != NULL)
{


vim_parent_hwnd = FindWindowEx(hwnd, NULL, "MDIClient", NULL);
if (vim_parent_hwnd != NULL)
return FALSE;
}
}
return TRUE; 
}





void
gui_mch_set_parent(char *title)
{
EnumWindows(FindWindowTitle, (LPARAM)title);
if (vim_parent_hwnd == NULL)
{
semsg(_("E671: Cannot find window title \"%s\""), title);
mch_exit(2);
}
}

#if !defined(FEAT_OLE)
static void
ole_error(char *arg)
{
char buf[IOSIZE];

#if defined(VIMDLL)
gui.in_use = mch_is_gui_executable();
#endif


vim_snprintf(buf, IOSIZE,
_("E243: Argument not supported: \"-%s\"; Use the OLE version."),
arg);
mch_errmsg(buf);
}
#endif

#if defined(GUI_MAY_SPAWN) || defined(PROTO)
static char *
gvim_error(void)
{
char *msg = _("E988: GUI cannot be used. Cannot execute gvim.exe.");

if (starting)
{
mch_errmsg(msg);
mch_errmsg("\n");
mch_exit(2);
}
return msg;
}

char *
gui_mch_do_spawn(char_u *arg)
{
int len;
#if defined(FEAT_SESSION) && defined(EXPERIMENTAL_GUI_CMD)
char_u *session = NULL;
LPWSTR tofree1 = NULL;
#endif
WCHAR name[MAX_PATH];
LPWSTR cmd, newcmd = NULL, p, warg, tofree2 = NULL;
STARTUPINFOW si = {sizeof(si)};
PROCESS_INFORMATION pi;

if (!GetModuleFileNameW(g_hinst, name, MAX_PATH))
goto error;
p = wcsrchr(name, L'\\');
if (p == NULL)
goto error;

#if defined(DEBUG)
wcscpy(p + 1, L"gvimd.exe");
#else
wcscpy(p + 1, L"gvim.exe");
#endif

#if defined(FEAT_SESSION) && defined(EXPERIMENTAL_GUI_CMD)
if (starting)
#endif
{

p = GetCommandLineW();

while (*p && *p != L' ' && *p != L'\t')
{
if (*p == L'"')
{
while (*p && *p != L'"')
++p;
if (*p)
++p;
}
else
++p;
}
cmd = p;
}
#if defined(FEAT_SESSION) && defined(EXPERIMENTAL_GUI_CMD)
else
{

LPWSTR wsession;
char_u *savebg;
int ret;

session = vim_tempname('s', FALSE);
if (session == NULL)
goto error;
savebg = p_bg;
p_bg = vim_strsave((char_u *)"light"); 
ret = write_session_file(session);
vim_free(p_bg);
p_bg = savebg;
if (!ret)
goto error;
wsession = enc_to_utf16(session, NULL);
if (wsession == NULL)
goto error;
len = (int)wcslen(wsession) * 2 + 27 + 1;
cmd = ALLOC_MULT(WCHAR, len);
if (cmd == NULL)
{
vim_free(wsession);
goto error;
}
tofree1 = cmd;
_snwprintf(cmd, len, L" -S \"%s\" -c \"call delete('%s')\"",
wsession, wsession);
vim_free(wsession);
}
#endif


if (arg != NULL)
{
warg = enc_to_utf16(arg, NULL);
if (warg == NULL)
goto error;
tofree2 = warg;
}
else
warg = L"";


len = (int)wcslen(name) + (int)wcslen(cmd) + (int)wcslen(warg) + 4;
newcmd = ALLOC_MULT(WCHAR, len);
if (newcmd == NULL)
goto error;
_snwprintf(newcmd, len, L"\"%s\"%s %s", name, cmd, warg);


if (!CreateProcessW(NULL, newcmd, NULL, NULL, TRUE, 0,
NULL, NULL, &si, &pi))
goto error;
CloseHandle(pi.hProcess);
CloseHandle(pi.hThread);
mch_exit(0);

error:
#if defined(FEAT_SESSION) && defined(EXPERIMENTAL_GUI_CMD)
if (session)
mch_remove(session);
vim_free(session);
vim_free(tofree1);
#endif
vim_free(newcmd);
vim_free(tofree2);
return gvim_error();
}
#endif






void
gui_mch_prepare(int *argc, char **argv)
{
int silent = FALSE;
int idx;


if ((*argc == 2 || *argc == 3) && (argv[1][0] == '-' || argv[1][0] == '/'))
{

if (*argc == 3 && STRICMP(argv[1] + 1, "silent") == 0
&& (argv[2][0] == '-' || argv[2][0] == '/'))
{
silent = TRUE;
idx = 2;
}
else
idx = 1;


if (STRICMP(argv[idx] + 1, "register") == 0)
{
#if defined(FEAT_OLE)
RegisterMe(silent);
mch_exit(0);
#else
if (!silent)
ole_error("register");
mch_exit(2);
#endif
}


if (STRICMP(argv[idx] + 1, "unregister") == 0)
{
#if defined(FEAT_OLE)
UnregisterMe(!silent);
mch_exit(0);
#else
if (!silent)
ole_error("unregister");
mch_exit(2);
#endif
}





if (STRICMP(argv[idx] + 1, "embedding") == 0)
{
#if defined(FEAT_OLE)
*argc = 1;
#else
ole_error("embedding");
mch_exit(2);
#endif
}
}

#if defined(FEAT_OLE)
{
int bDoRestart = FALSE;

InitOLE(&bDoRestart);

if (bDoRestart)
mch_exit(0);
}
#endif

#if defined(FEAT_NETBEANS_INTG)
{

int arg;

for (arg = 1; arg < *argc; arg++)
if (strncmp("-nb", argv[arg], 3) == 0)
{
netbeansArg = argv[arg];
mch_memmove(&argv[arg], &argv[arg + 1],
(--*argc - arg) * sizeof(char *));
argv[*argc] = NULL;
break; 
}
}
#endif
}





int
gui_mch_init(void)
{
const WCHAR szVimWndClassW[] = VIM_CLASSW;
const WCHAR szTextAreaClassW[] = L"VimTextArea";
WNDCLASSW wndclassw;
#if defined(GLOBAL_IME)
ATOM atom;
#endif



if (s_hwnd != NULL)
goto theend;




#if defined(FEAT_TEAROFF)
s_htearbitmap = LoadBitmap(g_hinst, "IDB_TEAROFF");
#endif

gui.scrollbar_width = GetSystemMetrics(SM_CXVSCROLL);
gui.scrollbar_height = GetSystemMetrics(SM_CYHSCROLL);
#if defined(FEAT_MENU)
gui.menu_height = 0; 
#endif
gui.border_width = 0;

s_brush = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));



if (GetClassInfoW(g_hinst, szVimWndClassW, &wndclassw) == 0)
{
wndclassw.style = CS_DBLCLKS;
wndclassw.lpfnWndProc = _WndProc;
wndclassw.cbClsExtra = 0;
wndclassw.cbWndExtra = 0;
wndclassw.hInstance = g_hinst;
wndclassw.hIcon = LoadIcon(wndclassw.hInstance, "IDR_VIM");
wndclassw.hCursor = LoadCursor(NULL, IDC_ARROW);
wndclassw.hbrBackground = s_brush;
wndclassw.lpszMenuName = NULL;
wndclassw.lpszClassName = szVimWndClassW;

if ((
#if defined(GLOBAL_IME)
atom =
#endif
RegisterClassW(&wndclassw)) == 0)
return FAIL;
}

if (vim_parent_hwnd != NULL)
{
#if defined(HAVE_TRY_EXCEPT)
__try
{
#endif



s_hwnd = CreateWindowExW(
WS_EX_MDICHILD,
szVimWndClassW, L"Vim MSWindows GUI",
WS_OVERLAPPEDWINDOW | WS_CHILD
| WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 0xC000,
gui_win_x == -1 ? CW_USEDEFAULT : gui_win_x,
gui_win_y == -1 ? CW_USEDEFAULT : gui_win_y,
100, 
100, 
vim_parent_hwnd, NULL,
g_hinst, NULL);
#if defined(HAVE_TRY_EXCEPT)
}
__except(EXCEPTION_EXECUTE_HANDLER)
{

}
#endif
if (s_hwnd == NULL)
{
emsg(_("E672: Unable to open window inside MDI application"));
mch_exit(2);
}
}
else
{


if (IsWindow((HWND)win_socket_id) <= 0)
win_socket_id = 0;



s_hwnd = CreateWindowW(
szVimWndClassW, L"Vim MSWindows GUI",
(win_socket_id == 0 ? WS_OVERLAPPEDWINDOW : WS_POPUP)
| WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
gui_win_x == -1 ? CW_USEDEFAULT : gui_win_x,
gui_win_y == -1 ? CW_USEDEFAULT : gui_win_y,
100, 
100, 
NULL, NULL,
g_hinst, NULL);
if (s_hwnd != NULL && win_socket_id != 0)
{
SetParent(s_hwnd, (HWND)win_socket_id);
ShowWindow(s_hwnd, SW_SHOWMAXIMIZED);
}
}

if (s_hwnd == NULL)
return FAIL;

#if defined(GLOBAL_IME)
global_ime_init(atom, s_hwnd);
#endif
#if defined(FEAT_MBYTE_IME) && defined(DYNAMIC_IME)
dyn_imm_load();
#endif


if (GetClassInfoW(g_hinst, szTextAreaClassW, &wndclassw) == 0)
{
wndclassw.style = CS_OWNDC;
wndclassw.lpfnWndProc = _TextAreaWndProc;
wndclassw.cbClsExtra = 0;
wndclassw.cbWndExtra = 0;
wndclassw.hInstance = g_hinst;
wndclassw.hIcon = NULL;
wndclassw.hCursor = LoadCursor(NULL, IDC_ARROW);
wndclassw.hbrBackground = NULL;
wndclassw.lpszMenuName = NULL;
wndclassw.lpszClassName = szTextAreaClassW;

if (RegisterClassW(&wndclassw) == 0)
return FAIL;
}

s_textArea = CreateWindowExW(
0,
szTextAreaClassW, L"Vim text area",
WS_CHILD | WS_VISIBLE, 0, 0,
100, 
100, 
s_hwnd, NULL,
g_hinst, NULL);

if (s_textArea == NULL)
return FAIL;

#if defined(FEAT_LIBCALL)

{
HANDLE hIcon = NULL;

if (mch_icon_load(&hIcon) == OK && hIcon != NULL)
SendMessage(s_hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
}
#endif

#if defined(FEAT_MENU)
s_menuBar = CreateMenu();
#endif
s_hdc = GetDC(s_textArea);

DragAcceptFiles(s_hwnd, TRUE);





gui_mch_def_colors();



set_normal_colors();





gui_check_colors();
gui.def_norm_pixel = gui.norm_pixel;
gui.def_back_pixel = gui.back_pixel;



highlight_gui_started();




gui.border_offset = gui.border_width;




init_mouse_wheel();




get_dialog_font_metrics();
#if defined(FEAT_TOOLBAR)



initialise_toolbar();
#endif
#if defined(FEAT_GUI_TABLINE)



initialise_tabline();
#endif
#if defined(MSWIN_FIND_REPLACE)



s_findrep_msg = RegisterWindowMessage(FINDMSGSTRING);


s_findrep_struct.lStructSize = sizeof(s_findrep_struct);
s_findrep_struct.lpstrFindWhat = ALLOC_MULT(WCHAR, MSWIN_FR_BUFSIZE);
s_findrep_struct.lpstrFindWhat[0] = NUL;
s_findrep_struct.lpstrReplaceWith = ALLOC_MULT(WCHAR, MSWIN_FR_BUFSIZE);
s_findrep_struct.lpstrReplaceWith[0] = NUL;
s_findrep_struct.wFindWhatLen = MSWIN_FR_BUFSIZE;
s_findrep_struct.wReplaceWithLen = MSWIN_FR_BUFSIZE;
#endif

#if defined(FEAT_EVAL)
#if !defined(_MSC_VER) || (_MSC_VER < 1400)

#if !defined(HandleToLong)
#define HandleToLong(h) ((long)(intptr_t)(h))
#endif
#endif

set_vim_var_nr(VV_WINDOWID, HandleToLong(s_hwnd));
#endif

#if defined(FEAT_RENDER_OPTIONS)
if (p_rop)
(void)gui_mch_set_rendering_options(p_rop);
#endif

theend:

display_errors();

return OK;
}





static void
get_work_area(RECT *spi_rect)
{
HMONITOR mon;
MONITORINFO moninfo;


mon = MonitorFromWindow(s_hwnd, MONITOR_DEFAULTTOPRIMARY);
if (mon != NULL)
{
moninfo.cbSize = sizeof(MONITORINFO);
if (GetMonitorInfo(mon, &moninfo))
{
*spi_rect = moninfo.rcWork;
return;
}
}

SystemParametersInfo(SPI_GETWORKAREA, 0, spi_rect, 0);
}




void
gui_mch_set_shellsize(
int width,
int height,
int min_width UNUSED,
int min_height UNUSED,
int base_width UNUSED,
int base_height UNUSED,
int direction)
{
RECT workarea_rect;
int win_width, win_height;
WINDOWPLACEMENT wndpl;




get_work_area(&workarea_rect);



wndpl.length = sizeof(WINDOWPLACEMENT);
GetWindowPlacement(s_hwnd, &wndpl);




if (wndpl.showCmd == SW_SHOWMAXIMIZED && starting == 0)
{
ShowWindow(s_hwnd, SW_SHOWNORMAL);

GetWindowPlacement(s_hwnd, &wndpl);
}


win_width = width + (GetSystemMetrics(SM_CXFRAME) +
GetSystemMetrics(SM_CXPADDEDBORDER)) * 2;
win_height = height + (GetSystemMetrics(SM_CYFRAME) +
GetSystemMetrics(SM_CXPADDEDBORDER)) * 2
+ GetSystemMetrics(SM_CYCAPTION)
#if defined(FEAT_MENU)
+ gui_mswin_get_menu_height(FALSE)
#endif
;




wndpl.rcNormalPosition.right = wndpl.rcNormalPosition.left + win_width;
wndpl.rcNormalPosition.bottom = wndpl.rcNormalPosition.top + win_height;


if ((direction & RESIZE_HOR)
&& wndpl.rcNormalPosition.right > workarea_rect.right)
OffsetRect(&wndpl.rcNormalPosition,
workarea_rect.right - wndpl.rcNormalPosition.right, 0);

if ((direction & RESIZE_HOR)
&& wndpl.rcNormalPosition.left < workarea_rect.left)
OffsetRect(&wndpl.rcNormalPosition,
workarea_rect.left - wndpl.rcNormalPosition.left, 0);

if ((direction & RESIZE_VERT)
&& wndpl.rcNormalPosition.bottom > workarea_rect.bottom)
OffsetRect(&wndpl.rcNormalPosition,
0, workarea_rect.bottom - wndpl.rcNormalPosition.bottom);

if ((direction & RESIZE_VERT)
&& wndpl.rcNormalPosition.top < workarea_rect.top)
OffsetRect(&wndpl.rcNormalPosition,
0, workarea_rect.top - wndpl.rcNormalPosition.top);




SetWindowPlacement(s_hwnd, &wndpl);

SetActiveWindow(s_hwnd);
SetFocus(s_hwnd);

#if defined(FEAT_MENU)

gui_mswin_get_menu_height(!gui.starting);
#endif
}


void
gui_mch_set_scrollbar_thumb(
scrollbar_T *sb,
long val,
long size,
long max)
{
SCROLLINFO info;

sb->scroll_shift = 0;
while (max > 32767)
{
max = (max + 1) >> 1;
val >>= 1;
size >>= 1;
++sb->scroll_shift;
}

if (sb->scroll_shift > 0)
++size;

info.cbSize = sizeof(info);
info.fMask = SIF_POS | SIF_RANGE | SIF_PAGE;
info.nPos = val;
info.nMin = 0;
info.nMax = max;
info.nPage = size;
SetScrollInfo(sb->id, SB_CTL, &info, TRUE);
}





void
gui_mch_set_font(GuiFont font)
{
gui.currFont = font;
}





void
gui_mch_set_fg_color(guicolor_T color)
{
gui.currFgColor = color;
}




void
gui_mch_set_bg_color(guicolor_T color)
{
gui.currBgColor = color;
}




void
gui_mch_set_sp_color(guicolor_T color)
{
gui.currSpColor = color;
}

#if defined(FEAT_MBYTE_IME)




#if defined(_MSC_VER)
#include <ime.h> 
#endif
#include <imm.h>




static LRESULT
_OnImeNotify(HWND hWnd, DWORD dwCommand, DWORD dwData UNUSED)
{
LRESULT lResult = 0;
HIMC hImc;

if (!pImmGetContext || (hImc = pImmGetContext(hWnd)) == (HIMC)0)
return lResult;
switch (dwCommand)
{
case IMN_SETOPENSTATUS:
if (pImmGetOpenStatus(hImc))
{
pImmSetCompositionFontW(hImc, &norm_logfont);
im_set_position(gui.row, gui.col);


State &= ~LANGMAP;
if (State & INSERT)
{
#if defined(FEAT_KEYMAP)

if (curbuf->b_p_iminsert == B_IMODE_LMAP)
{

int old_row = gui.row;
int old_col = gui.col;




showmode();
status_redraw_curbuf();
update_screen(0);

gui.row = old_row;
gui.col = old_col;
}
#endif
}
}
gui_update_cursor(TRUE, FALSE);
gui_mch_flush();
lResult = 0;
break;
}
pImmReleaseContext(hWnd, hImc);
return lResult;
}

static LRESULT
_OnImeComposition(HWND hwnd, WPARAM dbcs UNUSED, LPARAM param)
{
char_u *ret;
int len;

if ((param & GCS_RESULTSTR) == 0) 
return 0;

ret = GetResultStr(hwnd, GCS_RESULTSTR, &len);
if (ret != NULL)
{
add_to_input_buf_csi(ret, len);
vim_free(ret);
return 1;
}
return 0;
}





static short_u *
GetCompositionString_inUCS2(HIMC hIMC, DWORD GCS, int *lenp)
{
LONG ret;
LPWSTR wbuf = NULL;
char_u *buf;

if (!pImmGetContext)
return NULL; 


ret = pImmGetCompositionStringW(hIMC, GCS, NULL, 0);
if (ret == 0)
return NULL; 

if (ret > 0)
{

wbuf = alloc(ret + sizeof(WCHAR));
if (wbuf != NULL)
{
pImmGetCompositionStringW(hIMC, GCS, wbuf, ret);
*lenp = ret / sizeof(WCHAR);
}
return (short_u *)wbuf;
}




ret = pImmGetCompositionStringA(hIMC, GCS, NULL, 0);
if (ret <= 0)
return NULL; 

buf = alloc(ret);
if (buf == NULL)
return NULL;
pImmGetCompositionStringA(hIMC, GCS, buf, ret);


MultiByteToWideChar_alloc(GetACP(), 0, (LPCSTR)buf, ret, &wbuf, lenp);
vim_free(buf);

return (short_u *)wbuf;
}







static char_u *
GetResultStr(HWND hwnd, int GCS, int *lenp)
{
HIMC hIMC; 
short_u *buf = NULL;
char_u *convbuf = NULL;

if (!pImmGetContext || (hIMC = pImmGetContext(hwnd)) == (HIMC)0)
return NULL;


buf = GetCompositionString_inUCS2(hIMC, GCS, lenp);
if (buf == NULL)
return NULL;

convbuf = utf16_to_enc(buf, lenp);
pImmReleaseContext(hwnd, hIMC);
vim_free(buf);
return convbuf;
}
#endif


#if defined(FEAT_MBYTE_IME) || defined(PROTO)




void
im_set_font(LOGFONTW *lf)
{
HIMC hImc;

if (pImmGetContext && (hImc = pImmGetContext(s_hwnd)) != (HIMC)0)
{
pImmSetCompositionFontW(hImc, lf);
pImmReleaseContext(s_hwnd, hImc);
}
}




void
im_set_position(int row, int col)
{
HIMC hImc;

if (pImmGetContext && (hImc = pImmGetContext(s_hwnd)) != (HIMC)0)
{
COMPOSITIONFORM cfs;

cfs.dwStyle = CFS_POINT;
cfs.ptCurrentPos.x = FILL_X(col);
cfs.ptCurrentPos.y = FILL_Y(row);
MapWindowPoints(s_textArea, s_hwnd, &cfs.ptCurrentPos, 1);
pImmSetCompositionWindow(hImc, &cfs);

pImmReleaseContext(s_hwnd, hImc);
}
}




void
im_set_active(int active)
{
HIMC hImc;
static HIMC hImcOld = (HIMC)0;

#if defined(VIMDLL)
if (!gui.in_use && !gui.starting)
{
mbyte_im_set_active(active);
return;
}
#endif

if (pImmGetContext) 
{
if (p_imdisable)
{
if (hImcOld == (HIMC)0)
{
hImcOld = pImmGetContext(s_hwnd);
if (hImcOld)
pImmAssociateContext(s_hwnd, (HIMC)0);
}
active = FALSE;
}
else if (hImcOld != (HIMC)0)
{
pImmAssociateContext(s_hwnd, hImcOld);
hImcOld = (HIMC)0;
}

hImc = pImmGetContext(s_hwnd);
if (hImc)
{



HKL hKL = GetKeyboardLayout(0);

if (LOWORD(hKL) == MAKELANGID(LANG_KOREAN, SUBLANG_KOREAN))
{
static DWORD dwConversionSaved = 0, dwSentenceSaved = 0;
static BOOL bSaved = FALSE;

if (active)
{

if (bSaved)
pImmSetConversionStatus(hImc, dwConversionSaved,
dwSentenceSaved);
bSaved = FALSE;
}
else
{

if (pImmGetConversionStatus(hImc, &dwConversionSaved,
&dwSentenceSaved))
{
bSaved = TRUE;
pImmSetConversionStatus(hImc,
dwConversionSaved & ~(IME_CMODE_NATIVE
| IME_CMODE_FULLSHAPE),
dwSentenceSaved);
}
}
}

pImmSetOpenStatus(hImc, active);
pImmReleaseContext(s_hwnd, hImc);
}
}
}




int
im_get_status(void)
{
int status = 0;
HIMC hImc;

#if defined(VIMDLL)
if (!gui.in_use && !gui.starting)
return mbyte_im_get_status();
#endif

if (pImmGetContext && (hImc = pImmGetContext(s_hwnd)) != (HIMC)0)
{
status = pImmGetOpenStatus(hImc) ? 1 : 0;
pImmReleaseContext(s_hwnd, hImc);
}
return status;
}

#endif 

#if !defined(FEAT_MBYTE_IME) && defined(GLOBAL_IME)





void
im_set_position(int row, int col)
{

POINT p;

p.x = FILL_X(col);
p.y = FILL_Y(row);
MapWindowPoints(s_textArea, s_hwnd, &p, 1);
global_ime_set_position(&p);
}




void
im_set_active(int active)
{
global_ime_set_status(active);
}




int
im_get_status(void)
{
return global_ime_get_status();
}
#endif




static void
latin9_to_ucs(char_u *text, int len, WCHAR *unicodebuf)
{
int c;

while (--len >= 0)
{
c = *text++;
switch (c)
{
case 0xa4: c = 0x20ac; break; 
case 0xa6: c = 0x0160; break; 
case 0xa8: c = 0x0161; break; 
case 0xb4: c = 0x017d; break; 
case 0xb8: c = 0x017e; break; 
case 0xbc: c = 0x0152; break; 
case 0xbd: c = 0x0153; break; 
case 0xbe: c = 0x0178; break; 
}
*unicodebuf++ = c;
}
}

#if defined(FEAT_RIGHTLEFT)












static void
RevOut( HDC s_hdc,
int col,
int row,
UINT foptions,
CONST RECT *pcliprect,
LPCTSTR text,
UINT len,
CONST INT *padding)
{
int ix;

for (ix = 0; ix < (int)len; ++ix)
ExtTextOut(s_hdc, col + TEXT_X(ix), row, foptions,
pcliprect, text + ix, 1, padding);
}
#endif

static void
draw_line(
int x1,
int y1,
int x2,
int y2,
COLORREF color)
{
#if defined(FEAT_DIRECTX)
if (IS_ENABLE_DIRECTX())
DWriteContext_DrawLine(s_dwc, x1, y1, x2, y2, color);
else
#endif
{
HPEN hpen = CreatePen(PS_SOLID, 1, color);
HPEN old_pen = SelectObject(s_hdc, hpen);
MoveToEx(s_hdc, x1, y1, NULL);

LineTo(s_hdc, x2, y2);
DeleteObject(SelectObject(s_hdc, old_pen));
}
}

static void
set_pixel(
int x,
int y,
COLORREF color)
{
#if defined(FEAT_DIRECTX)
if (IS_ENABLE_DIRECTX())
DWriteContext_SetPixel(s_dwc, x, y, color);
else
#endif
SetPixel(s_hdc, x, y, color);
}

static void
fill_rect(
const RECT *rcp,
HBRUSH hbr,
COLORREF color)
{
#if defined(FEAT_DIRECTX)
if (IS_ENABLE_DIRECTX())
DWriteContext_FillRect(s_dwc, rcp, color);
else
#endif
{
HBRUSH hbr2;

if (hbr == NULL)
hbr2 = CreateSolidBrush(color);
else
hbr2 = hbr;
FillRect(s_hdc, rcp, hbr2);
if (hbr == NULL)
DeleteBrush(hbr2);
}
}

void
gui_mch_draw_string(
int row,
int col,
char_u *text,
int len,
int flags)
{
static int *padding = NULL;
static int pad_size = 0;
int i;
const RECT *pcliprect = NULL;
UINT foptions = 0;
static WCHAR *unicodebuf = NULL;
static int *unicodepdy = NULL;
static int unibuflen = 0;
int n = 0;
int y;














static HBRUSH hbr_cache[2] = {NULL, NULL};
static guicolor_T brush_color[2] = {INVALCOLOR, INVALCOLOR};
static int brush_lru = 0;
HBRUSH hbr;
RECT rc;

if (!(flags & DRAW_TRANSP))
{




rc.left = FILL_X(col);
rc.top = FILL_Y(row);
if (has_mbyte)
{

rc.right = FILL_X(col + mb_string2cells(text, len));
}
else
rc.right = FILL_X(col + len);
rc.bottom = FILL_Y(row + 1);



if (gui.currBgColor == brush_color[0])
{
hbr = hbr_cache[0];
brush_lru = 1;
}
else if (gui.currBgColor == brush_color[1])
{
hbr = hbr_cache[1];
brush_lru = 0;
}
else
{
if (hbr_cache[brush_lru] != NULL)
DeleteBrush(hbr_cache[brush_lru]);
hbr_cache[brush_lru] = CreateSolidBrush(gui.currBgColor);
brush_color[brush_lru] = gui.currBgColor;
hbr = hbr_cache[brush_lru];
brush_lru = !brush_lru;
}

fill_rect(&rc, hbr, gui.currBgColor);

SetBkMode(s_hdc, TRANSPARENT);





if (flags & DRAW_CURSOR)
{
pcliprect = &rc;
foptions = ETO_CLIPPED;
}
}
SetTextColor(s_hdc, gui.currFgColor);
SelectFont(s_hdc, gui.currFont);

#if defined(FEAT_DIRECTX)
if (IS_ENABLE_DIRECTX())
DWriteContext_SetFont(s_dwc, (HFONT)gui.currFont);
#endif

if (pad_size != Columns || padding == NULL || padding[0] != gui.char_width)
{
vim_free(padding);
pad_size = Columns;



padding = LALLOC_MULT(int, pad_size);
if (padding != NULL)
for (i = 0; i < pad_size; i++)
padding[i] = gui.char_width;
}










if (enc_utf8)
for (n = 0; n < len; ++n)
if (text[n] >= 0x80)
break;

#if defined(FEAT_DIRECTX)



if (IS_ENABLE_DIRECTX())
n = 0; 
#endif




if ((enc_utf8
|| (enc_codepage > 0 && (int)GetACP() != enc_codepage)
|| enc_latin9)
&& (unicodebuf == NULL || len > unibuflen))
{
vim_free(unicodebuf);
unicodebuf = LALLOC_MULT(WCHAR, len);

vim_free(unicodepdy);
unicodepdy = LALLOC_MULT(int, len);

unibuflen = len;
}

if (enc_utf8 && n < len && unicodebuf != NULL)
{


int i;
int wlen; 
int clen; 
int cells; 
int cw; 
int c;

wlen = 0;
clen = 0;
cells = 0;
for (i = 0; i < len; )
{
c = utf_ptr2char(text + i);
if (c >= 0x10000)
{

unicodebuf[wlen++] = ((c - 0x10000) >> 10) + 0xD800;
unicodebuf[wlen++] = ((c - 0x10000) & 0x3ff) + 0xDC00;
}
else
{
unicodebuf[wlen++] = c;
}

if (utf_iscomposing(c))
cw = 0;
else
{
cw = utf_char2cells(c);
if (cw > 2) 
cw = 1;
}

if (unicodepdy != NULL)
{



if (c >= 0x10000)
{
unicodepdy[wlen - 2] = cw * gui.char_width;
unicodepdy[wlen - 1] = 0;
}
else
unicodepdy[wlen - 1] = cw * gui.char_width;
}
cells += cw;
i += utf_ptr2len_len(text + i, len - i);
++clen;
}
#if defined(FEAT_DIRECTX)
if (IS_ENABLE_DIRECTX())
{

DWriteContext_DrawText(s_dwc, unicodebuf, wlen,
TEXT_X(col), TEXT_Y(row),
FILL_X(cells + 1), FILL_Y(1) - p_linespace,
gui.char_width, gui.currFgColor,
foptions, pcliprect, unicodepdy);
}
else
#endif
ExtTextOutW(s_hdc, TEXT_X(col), TEXT_Y(row),
foptions, pcliprect, unicodebuf, wlen, unicodepdy);
len = cells; 
}
else if ((enc_codepage > 0 && (int)GetACP() != enc_codepage) || enc_latin9)
{


if (unicodebuf != NULL)
{
if (enc_latin9)
latin9_to_ucs(text, len, unicodebuf);
else
len = MultiByteToWideChar(enc_codepage,
MB_PRECOMPOSED,
(char *)text, len,
(LPWSTR)unicodebuf, unibuflen);
if (len != 0)
{



if (unicodepdy != NULL)
{
int i;
int cw;

for (i = 0; i < len; ++i)
{
cw = utf_char2cells(unicodebuf[i]);
if (cw > 2)
cw = 1;
unicodepdy[i] = cw * gui.char_width;
}
}
ExtTextOutW(s_hdc, TEXT_X(col), TEXT_Y(row),
foptions, pcliprect, unicodebuf, len, unicodepdy);
}
}
}
else
{
#if defined(FEAT_RIGHTLEFT)


if (curwin->w_p_rl)
RevOut(s_hdc, TEXT_X(col), TEXT_Y(row),
foptions, pcliprect, (char *)text, len, padding);
else
#endif
ExtTextOut(s_hdc, TEXT_X(col), TEXT_Y(row),
foptions, pcliprect, (char *)text, len, padding);
}


if (flags & DRAW_UNDERL)
{


y = FILL_Y(row + 1) - 1;
if (p_linespace > 1)
y -= p_linespace - 1;
draw_line(FILL_X(col), y, FILL_X(col + len), y, gui.currFgColor);
}


if (flags & DRAW_STRIKE)
{
y = FILL_Y(row + 1) - gui.char_height/2;
draw_line(FILL_X(col), y, FILL_X(col + len), y, gui.currSpColor);
}


if (flags & DRAW_UNDERC)
{
int x;
int offset;
static const int val[8] = {1, 0, 0, 0, 1, 2, 2, 2 };

y = FILL_Y(row + 1) - 1;
for (x = FILL_X(col); x < FILL_X(col + len); ++x)
{
offset = val[x % 8];
set_pixel(x, y - offset, gui.currSpColor);
}
}
}









void
gui_mch_flush(void)
{
#if defined(FEAT_DIRECTX)
if (IS_ENABLE_DIRECTX())
DWriteContext_Flush(s_dwc);
#endif

GdiFlush();
}

static void
clear_rect(RECT *rcp)
{
fill_rect(rcp, NULL, gui.back_pixel);
}


void
gui_mch_get_screen_dimensions(int *screen_w, int *screen_h)
{
RECT workarea_rect;

get_work_area(&workarea_rect);

*screen_w = workarea_rect.right - workarea_rect.left
- (GetSystemMetrics(SM_CXFRAME) +
GetSystemMetrics(SM_CXPADDEDBORDER)) * 2;




*screen_h = workarea_rect.bottom - workarea_rect.top
- (GetSystemMetrics(SM_CYFRAME) +
GetSystemMetrics(SM_CXPADDEDBORDER)) * 2
- GetSystemMetrics(SM_CYCAPTION)
#if defined(FEAT_MENU)
- gui_mswin_get_menu_height(FALSE)
#endif
;
}


#if defined(FEAT_MENU) || defined(PROTO)



void
gui_mch_add_menu(
vimmenu_T *menu,
int pos)
{
vimmenu_T *parent = menu->parent;

menu->submenu_id = CreatePopupMenu();
menu->id = s_menu_id++;

if (menu_is_menubar(menu->name))
{
WCHAR *wn;
MENUITEMINFOW infow;

wn = enc_to_utf16(menu->name, NULL);
if (wn == NULL)
return;

infow.cbSize = sizeof(infow);
infow.fMask = MIIM_DATA | MIIM_TYPE | MIIM_ID
| MIIM_SUBMENU;
infow.dwItemData = (long_u)menu;
infow.wID = menu->id;
infow.fType = MFT_STRING;
infow.dwTypeData = wn;
infow.cch = (UINT)wcslen(wn);
infow.hSubMenu = menu->submenu_id;
InsertMenuItemW((parent == NULL)
? s_menuBar : parent->submenu_id,
(UINT)pos, TRUE, &infow);
vim_free(wn);
}


if (parent == NULL)
gui_mswin_get_menu_height(!gui.starting);
#if defined(FEAT_TEAROFF)
else if (IsWindow(parent->tearoff_handle))
rebuild_tearoff(parent);
#endif
}

void
gui_mch_show_popupmenu(vimmenu_T *menu)
{
POINT mp;

(void)GetCursorPos((LPPOINT)&mp);
gui_mch_show_popupmenu_at(menu, (int)mp.x, (int)mp.y);
}

void
gui_make_popup(char_u *path_name, int mouse_pos)
{
vimmenu_T *menu = gui_find_menu(path_name);

if (menu != NULL)
{
POINT p;


GetDCOrgEx(s_hdc, &p);
if (mouse_pos)
{
int mx, my;

gui_mch_getmouse(&mx, &my);
p.x += mx;
p.y += my;
}
else if (curwin != NULL)
{
p.x += TEXT_X(curwin->w_wincol + curwin->w_wcol + 1);
p.y += TEXT_Y(W_WINROW(curwin) + curwin->w_wrow + 1);
}
msg_scroll = FALSE;
gui_mch_show_popupmenu_at(menu, (int)p.x, (int)p.y);
}
}

#if defined(FEAT_TEAROFF) || defined(PROTO)




void
gui_make_tearoff(char_u *path_name)
{
vimmenu_T *menu = gui_find_menu(path_name);


if (menu != NULL)
gui_mch_tearoff(menu->dname, menu, 0xffffL, 0xffffL);
}
#endif




void
gui_mch_add_menu_item(
vimmenu_T *menu,
int idx)
{
vimmenu_T *parent = menu->parent;

menu->id = s_menu_id++;
menu->submenu_id = NULL;

#if defined(FEAT_TEAROFF)
if (STRNCMP(menu->name, TEAR_STRING, TEAR_LEN) == 0)
{
InsertMenu(parent->submenu_id, (UINT)idx, MF_BITMAP|MF_BYPOSITION,
(UINT)menu->id, (LPCTSTR) s_htearbitmap);
}
else
#endif
#if defined(FEAT_TOOLBAR)
if (menu_is_toolbar(parent->name))
{
TBBUTTON newtb;

vim_memset(&newtb, 0, sizeof(newtb));
if (menu_is_separator(menu->name))
{
newtb.iBitmap = 0;
newtb.fsStyle = TBSTYLE_SEP;
}
else
{
newtb.iBitmap = get_toolbar_bitmap(menu);
newtb.fsStyle = TBSTYLE_BUTTON;
}
newtb.idCommand = menu->id;
newtb.fsState = TBSTATE_ENABLED;
newtb.iString = 0;
SendMessage(s_toolbarhwnd, TB_INSERTBUTTON, (WPARAM)idx,
(LPARAM)&newtb);
menu->submenu_id = (HMENU)-1;
}
else
#endif
{
WCHAR *wn;

wn = enc_to_utf16(menu->name, NULL);
if (wn != NULL)
{
InsertMenuW(parent->submenu_id, (UINT)idx,
(menu_is_separator(menu->name)
? MF_SEPARATOR : MF_STRING) | MF_BYPOSITION,
(UINT)menu->id, wn);
vim_free(wn);
}
#if defined(FEAT_TEAROFF)
if (IsWindow(parent->tearoff_handle))
rebuild_tearoff(parent);
#endif
}
}




void
gui_mch_destroy_menu(vimmenu_T *menu)
{
#if defined(FEAT_TOOLBAR)



if (menu->submenu_id == (HMENU)-1)
{
int iButton;

iButton = (int)SendMessage(s_toolbarhwnd, TB_COMMANDTOINDEX,
(WPARAM)menu->id, 0);
SendMessage(s_toolbarhwnd, TB_DELETEBUTTON, (WPARAM)iButton, 0);
}
else
#endif
{
if (menu->parent != NULL
&& menu_is_popup(menu->parent->dname)
&& menu->parent->submenu_id != NULL)
RemoveMenu(menu->parent->submenu_id, menu->id, MF_BYCOMMAND);
else
RemoveMenu(s_menuBar, menu->id, MF_BYCOMMAND);
if (menu->submenu_id != NULL)
DestroyMenu(menu->submenu_id);
#if defined(FEAT_TEAROFF)
if (IsWindow(menu->tearoff_handle))
DestroyWindow(menu->tearoff_handle);
if (menu->parent != NULL
&& menu->parent->children != NULL
&& IsWindow(menu->parent->tearoff_handle))
{

menu->modes = 0;
rebuild_tearoff(menu->parent);
}
#endif
}
}

#if defined(FEAT_TEAROFF)
static void
rebuild_tearoff(vimmenu_T *menu)
{

char_u tbuf[128];
RECT trect;
RECT rct;
RECT roct;
int x, y;

HWND thwnd = menu->tearoff_handle;

GetWindowText(thwnd, (LPSTR)tbuf, 127);
if (GetWindowRect(thwnd, &trect)
&& GetWindowRect(s_hwnd, &rct)
&& GetClientRect(s_hwnd, &roct))
{
x = trect.left - rct.left;
y = (trect.top - rct.bottom + roct.bottom);
}
else
{
x = y = 0xffffL;
}
DestroyWindow(thwnd);
if (menu->children != NULL)
{
gui_mch_tearoff(tbuf, menu, x, y);
if (IsWindow(menu->tearoff_handle))
(void) SetWindowPos(menu->tearoff_handle,
NULL,
(int)trect.left,
(int)trect.top,
0, 0,
SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}
}
#endif 




void
gui_mch_menu_grey(
vimmenu_T *menu,
int grey)
{
#if defined(FEAT_TOOLBAR)



if (menu->submenu_id == (HMENU)-1)
{
SendMessage(s_toolbarhwnd, TB_ENABLEBUTTON,
(WPARAM)menu->id, (LPARAM) MAKELONG((grey ? FALSE : TRUE), 0) );
}
else
#endif
(void)EnableMenuItem(menu->parent ? menu->parent->submenu_id : s_menuBar,
menu->id, MF_BYCOMMAND | (grey ? MF_GRAYED : MF_ENABLED));

#if defined(FEAT_TEAROFF)
if ((menu->parent != NULL) && (IsWindow(menu->parent->tearoff_handle)))
{
WORD menuID;
HWND menuHandle;




if (menu->children == NULL)
menuID = (WORD)(menu->id);
else
menuID = (WORD)((long_u)(menu->submenu_id) | (DWORD)0x8000);
menuHandle = GetDlgItem(menu->parent->tearoff_handle, menuID);
if (menuHandle)
EnableWindow(menuHandle, !grey);

}
#endif
}

#endif 




#define add_string(s) strcpy((LPSTR)p, s); (LPSTR)p += (strlen((LPSTR)p) + 1)
#define add_word(x) *p++ = (x)
#define add_long(x) dwp = (DWORD *)p; *dwp++ = (x); p = (WORD *)dwp

#if defined(FEAT_GUI_DIALOG) || defined(PROTO)











static LRESULT CALLBACK
dialog_callback(
HWND hwnd,
UINT message,
WPARAM wParam,
LPARAM lParam UNUSED)
{
if (message == WM_INITDIALOG)
{
CenterWindow(hwnd, GetWindow(hwnd, GW_OWNER));

(void)SetFocus(hwnd);
if (dialog_default_button > IDCANCEL)
(void)SetFocus(GetDlgItem(hwnd, dialog_default_button));
else


(void)SetFocus(GetDlgItem(hwnd, DLG_NONBUTTON_CONTROL));
return FALSE;
}

if (message == WM_COMMAND)
{
int button = LOWORD(wParam);



if (button >= DLG_NONBUTTON_CONTROL)
return TRUE;


if (s_textfield != NULL)
{
WCHAR *wp = ALLOC_MULT(WCHAR, IOSIZE);
char_u *p;

GetDlgItemTextW(hwnd, DLG_NONBUTTON_CONTROL + 2, wp, IOSIZE);
p = utf16_to_enc(wp, NULL);
vim_strncpy(s_textfield, p, IOSIZE);
vim_free(p);
vim_free(wp);
}





if (button == IDOK)
{
if (dialog_default_button > IDCANCEL)
EndDialog(hwnd, dialog_default_button);
}
else
EndDialog(hwnd, button - IDCANCEL);
return TRUE;
}

if ((message == WM_SYSCOMMAND) && (wParam == SC_CLOSE))
{
EndDialog(hwnd, 0);
return TRUE;
}
return FALSE;
}


















static const char *dlg_icons[] = 
{
"IDR_VIM",
"IDR_VIM_ERROR",
"IDR_VIM_ALERT",
"IDR_VIM_INFO",
"IDR_VIM_QUESTION"
};

int
gui_mch_dialog(
int type,
char_u *title,
char_u *message,
char_u *buttons,
int dfltbutton,
char_u *textfield,
int ex_cmd UNUSED)
{
WORD *p, *pdlgtemplate, *pnumitems;
DWORD *dwp;
int numButtons;
int *buttonWidths, *buttonPositions;
int buttonYpos;
int nchar, i;
DWORD lStyle;
int dlgwidth = 0;
int dlgheight;
int editboxheight;
int horizWidth = 0;
int msgheight;
char_u *pstart;
char_u *pend;
char_u *last_white;
char_u *tbuffer;
RECT rect;
HWND hwnd;
HDC hdc;
HFONT font, oldFont;
TEXTMETRIC fontInfo;
int fontHeight;
int textWidth, minButtonWidth, messageWidth;
int maxDialogWidth;
int maxDialogHeight;
int scroll_flag = 0;
int vertical;
int dlgPaddingX;
int dlgPaddingY;
#if defined(USE_SYSMENU_FONT)
LOGFONTW lfSysmenu;
int use_lfSysmenu = FALSE;
#endif
garray_T ga;
int l;

#if !defined(NO_CONSOLE)

#if defined(VIMDLL)
if (!(gui.in_use || gui.starting))
#endif
if (silent_mode)
return dfltbutton; 
#endif

if (s_hwnd == NULL)
get_dialog_font_metrics();

if ((type < 0) || (type > VIM_LAST_TYPE))
type = 0;



pdlgtemplate = p = (PWORD)LocalAlloc(LPTR,
DLG_ALLOC_SIZE + STRLEN(message) * 2);

if (p == NULL)
return -1;






tbuffer = vim_strsave(buttons);
if (tbuffer == NULL)
return -1;

--dfltbutton; 


numButtons = 1;
for (i = 0; tbuffer[i] != '\0'; i++)
{
if (tbuffer[i] == DLG_BUTTON_SEP)
numButtons++;
}
if (dfltbutton >= numButtons)
dfltbutton = -1;


buttonWidths = ALLOC_MULT(int, numButtons);
if (buttonWidths == NULL)
return -1;


buttonPositions = ALLOC_MULT(int, numButtons);
if (buttonPositions == NULL)
return -1;




hwnd = GetDesktopWindow();
hdc = GetWindowDC(hwnd);
#if defined(USE_SYSMENU_FONT)
if (gui_w32_get_menu_font(&lfSysmenu) == OK)
{
font = CreateFontIndirectW(&lfSysmenu);
use_lfSysmenu = TRUE;
}
else
#endif
font = CreateFont(-DLG_FONT_POINT_SIZE, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
VARIABLE_PITCH , DLG_FONT_NAME);
if (s_usenewlook)
{
oldFont = SelectFont(hdc, font);
dlgPaddingX = DLG_PADDING_X;
dlgPaddingY = DLG_PADDING_Y;
}
else
{
oldFont = SelectFont(hdc, GetStockObject(SYSTEM_FONT));
dlgPaddingX = DLG_OLD_STYLE_PADDING_X;
dlgPaddingY = DLG_OLD_STYLE_PADDING_Y;
}
GetTextMetrics(hdc, &fontInfo);
fontHeight = fontInfo.tmHeight;


minButtonWidth = GetTextWidth(hdc, (char_u *)"Cancel", 6);


if (s_hwnd == NULL)
{
RECT workarea_rect;


get_work_area(&workarea_rect);
maxDialogWidth = workarea_rect.right - workarea_rect.left - 100;
if (maxDialogWidth > 600)
maxDialogWidth = 600;

maxDialogHeight = workarea_rect.bottom - workarea_rect.top - 150;
}
else
{

GetWindowRect(s_hwnd, &rect);
maxDialogWidth = rect.right - rect.left
- (GetSystemMetrics(SM_CXFRAME) +
GetSystemMetrics(SM_CXPADDEDBORDER)) * 2;
if (maxDialogWidth < DLG_MIN_MAX_WIDTH)
maxDialogWidth = DLG_MIN_MAX_WIDTH;

maxDialogHeight = rect.bottom - rect.top
- (GetSystemMetrics(SM_CYFRAME) +
GetSystemMetrics(SM_CXPADDEDBORDER)) * 4
- GetSystemMetrics(SM_CYCAPTION);
if (maxDialogHeight < DLG_MIN_MAX_HEIGHT)
maxDialogHeight = DLG_MIN_MAX_HEIGHT;
}




pstart = message;
messageWidth = 0;
msgheight = 0;
ga_init2(&ga, sizeof(char), 500);
do
{
msgheight += fontHeight; 




textWidth = 0;
last_white = NULL;
for (pend = pstart; *pend != NUL && *pend != '\n'; )
{
l = (*mb_ptr2len)(pend);
if (l == 1 && VIM_ISWHITE(*pend)
&& textWidth > maxDialogWidth * 3 / 4)
last_white = pend;
textWidth += GetTextWidthEnc(hdc, pend, l);
if (textWidth >= maxDialogWidth)
{

messageWidth = maxDialogWidth;
msgheight += fontHeight;
textWidth = 0;

if (last_white != NULL)
{

ga.ga_len -= (int)(pend - (last_white + 1));
pend = last_white + 1;
last_white = NULL;
}
ga_append(&ga, '\r');
ga_append(&ga, '\n');
continue;
}

while (--l >= 0)
ga_append(&ga, *pend++);
}
if (textWidth > messageWidth)
messageWidth = textWidth;

ga_append(&ga, '\r');
ga_append(&ga, '\n');
pstart = pend + 1;
} while (*pend != NUL);

if (ga.ga_data != NULL)
message = ga.ga_data;

messageWidth += 10; 


dlgwidth = messageWidth + DLG_ICON_WIDTH + 3 * dlgPaddingX
+ GetSystemMetrics(SM_CXVSCROLL);

if (msgheight < DLG_ICON_HEIGHT)
msgheight = DLG_ICON_HEIGHT;





vertical = (p_go != NULL && vim_strchr(p_go, GO_VERTICAL) != NULL);
if (!vertical)
{

horizWidth = dlgPaddingX;
pstart = tbuffer;
i = 0;
do
{
pend = vim_strchr(pstart, DLG_BUTTON_SEP);
if (pend == NULL)
pend = pstart + STRLEN(pstart); 
textWidth = GetTextWidthEnc(hdc, pstart, (int)(pend - pstart));
if (textWidth < minButtonWidth)
textWidth = minButtonWidth;
textWidth += dlgPaddingX; 
buttonWidths[i] = textWidth;
buttonPositions[i++] = horizWidth;
horizWidth += textWidth + dlgPaddingX; 
pstart = pend + 1;
} while (*pend != NUL);

if (horizWidth > maxDialogWidth)
vertical = TRUE; 
else if (horizWidth > dlgwidth)
dlgwidth = horizWidth;
}

if (vertical)
{

pstart = tbuffer;
do
{
pend = vim_strchr(pstart, DLG_BUTTON_SEP);
if (pend == NULL)
pend = pstart + STRLEN(pstart); 
textWidth = GetTextWidthEnc(hdc, pstart, (int)(pend - pstart));
textWidth += dlgPaddingX; 
textWidth += DLG_VERT_PADDING_X * 2; 
if (textWidth > dlgwidth)
dlgwidth = textWidth;
pstart = pend + 1;
} while (*pend != NUL);
}

if (dlgwidth < DLG_MIN_WIDTH)
dlgwidth = DLG_MIN_WIDTH; 


if (s_usenewlook)
lStyle = DS_MODALFRAME | WS_CAPTION |DS_3DLOOK| WS_VISIBLE |DS_SETFONT;
else
lStyle = DS_MODALFRAME | WS_CAPTION |DS_3DLOOK| WS_VISIBLE;

add_long(lStyle);
add_long(0); 
pnumitems = p; 
add_word(0); 
add_word(10); 
add_word(10); 
add_word(PixelToDialogX(dlgwidth)); 


if (vertical)
dlgheight = msgheight + 2 * dlgPaddingY
+ DLG_VERT_PADDING_Y + 2 * fontHeight * numButtons;
else
dlgheight = msgheight + 3 * dlgPaddingY + 2 * fontHeight;


editboxheight = fontHeight + dlgPaddingY + 4 * DLG_VERT_PADDING_Y;
if (textfield != NULL)
dlgheight += editboxheight;


if (dlgheight > maxDialogHeight)
{
msgheight = msgheight - (dlgheight - maxDialogHeight);
dlgheight = maxDialogHeight;
scroll_flag = WS_VSCROLL;

messageWidth = dlgwidth - DLG_ICON_WIDTH - 3 * dlgPaddingX;
}

add_word(PixelToDialogY(dlgheight));

add_word(0); 
add_word(0); 


nchar = nCopyAnsiToWideChar(p, (title ? (LPSTR)title
: (LPSTR)("Vim "VIM_VERSION_MEDIUM)), TRUE);
p += nchar;

if (s_usenewlook)
{

#if defined(USE_SYSMENU_FONT)
if (use_lfSysmenu)
{

*p++ = -MulDiv(lfSysmenu.lfHeight, 72,
GetDeviceCaps(hdc, LOGPIXELSY));
wcscpy(p, lfSysmenu.lfFaceName);
nchar = (int)wcslen(lfSysmenu.lfFaceName) + 1;
}
else
#endif
{
*p++ = DLG_FONT_POINT_SIZE; 
nchar = nCopyAnsiToWideChar(p, DLG_FONT_NAME, FALSE);
}
p += nchar;
}

buttonYpos = msgheight + 2 * dlgPaddingY;

if (textfield != NULL)
buttonYpos += editboxheight;

pstart = tbuffer;
if (!vertical)
horizWidth = (dlgwidth - horizWidth) / 2; 
for (i = 0; i < numButtons; i++)
{

for ( pend = pstart;
*pend && (*pend != DLG_BUTTON_SEP);
pend++)
;

if (*pend)
*pend = '\0';














if (vertical)
{
p = add_dialog_element(p,
(i == dfltbutton
? BS_DEFPUSHBUTTON : BS_PUSHBUTTON) | WS_TABSTOP,
PixelToDialogX(DLG_VERT_PADDING_X),
PixelToDialogY(buttonYpos 
+ 2 * fontHeight * i),
PixelToDialogX(dlgwidth - 2 * DLG_VERT_PADDING_X),
(WORD)(PixelToDialogY(2 * fontHeight) - 1),
(WORD)(IDCANCEL + 1 + i), (WORD)0x0080, (char *)pstart);
}
else
{
p = add_dialog_element(p,
(i == dfltbutton
? BS_DEFPUSHBUTTON : BS_PUSHBUTTON) | WS_TABSTOP,
PixelToDialogX(horizWidth + buttonPositions[i]),
PixelToDialogY(buttonYpos), 
PixelToDialogX(buttonWidths[i]),
(WORD)(PixelToDialogY(2 * fontHeight) - 1),
(WORD)(IDCANCEL + 1 + i), (WORD)0x0080, (char *)pstart);
}
pstart = pend + 1; 
}
*pnumitems += numButtons;


p = add_dialog_element(p, SS_ICON,
PixelToDialogX(dlgPaddingX),
PixelToDialogY(dlgPaddingY),
PixelToDialogX(DLG_ICON_WIDTH),
PixelToDialogY(DLG_ICON_HEIGHT),
DLG_NONBUTTON_CONTROL + 0, (WORD)0x0082,
dlg_icons[type]);


p = add_dialog_element(p, ES_LEFT|scroll_flag|ES_MULTILINE|ES_READONLY,
PixelToDialogX(2 * dlgPaddingX + DLG_ICON_WIDTH),
PixelToDialogY(dlgPaddingY),
(WORD)(PixelToDialogX(messageWidth) + 1),
PixelToDialogY(msgheight),
DLG_NONBUTTON_CONTROL + 1, (WORD)0x0081, (char *)message);


if (textfield != NULL)
{
p = add_dialog_element(p, ES_LEFT|ES_AUTOHSCROLL|WS_TABSTOP|WS_BORDER,
PixelToDialogX(2 * dlgPaddingX),
PixelToDialogY(2 * dlgPaddingY + msgheight),
PixelToDialogX(dlgwidth - 4 * dlgPaddingX),
PixelToDialogY(fontHeight + dlgPaddingY),
DLG_NONBUTTON_CONTROL + 2, (WORD)0x0081, (char *)textfield);
*pnumitems += 1;
}

*pnumitems += 2;

SelectFont(hdc, oldFont);
DeleteObject(font);
ReleaseDC(hwnd, hdc);





if (textfield != NULL)
{
dialog_default_button = DLG_NONBUTTON_CONTROL + 2;
s_textfield = textfield;
}
else
{
dialog_default_button = IDCANCEL + 1 + dfltbutton;
s_textfield = NULL;
}


nchar = (int)DialogBoxIndirect(
g_hinst,
(LPDLGTEMPLATE)pdlgtemplate,
s_hwnd,
(DLGPROC)dialog_callback);

LocalFree(LocalHandle(pdlgtemplate));
vim_free(tbuffer);
vim_free(buttonWidths);
vim_free(buttonPositions);
vim_free(ga.ga_data);


(void)SetFocus(s_hwnd);

return nchar;
}

#endif 

















static PWORD
add_dialog_element(
PWORD p,
DWORD lStyle,
WORD x,
WORD y,
WORD w,
WORD h,
WORD Id,
WORD clss,
const char *caption)
{
int nchar;

p = lpwAlign(p); 
lStyle = lStyle | WS_VISIBLE | WS_CHILD;
*p++ = LOWORD(lStyle);
*p++ = HIWORD(lStyle);
*p++ = 0; 
*p++ = 0; 
*p++ = x;
*p++ = y;
*p++ = w;
*p++ = h;
*p++ = Id; 

*p++ = (WORD)0xffff;
*p++ = clss; 

nchar = nCopyAnsiToWideChar(p, (LPSTR)caption, TRUE); 
p += nchar;

*p++ = 0; 

return p; 

}






static LPWORD
lpwAlign(
LPWORD lpIn)
{
long_u ul;

ul = (long_u)lpIn;
ul += 3;
ul >>= 2;
ul <<= 2;
return (LPWORD)ul;
}








static int
nCopyAnsiToWideChar(
LPWORD lpWCStr,
LPSTR lpAnsiIn,
BOOL use_enc)
{
int nChar = 0;
int len = lstrlen(lpAnsiIn) + 1; 
int i;
WCHAR *wn;

if (use_enc && enc_codepage >= 0 && (int)GetACP() != enc_codepage)
{

wn = enc_to_utf16((char_u *)lpAnsiIn, NULL);
if (wn != NULL)
{
wcscpy(lpWCStr, wn);
nChar = (int)wcslen(wn) + 1;
vim_free(wn);
}
}
if (nChar == 0)

nChar = MultiByteToWideChar(
enc_codepage > 0 ? enc_codepage : CP_ACP,
MB_PRECOMPOSED,
lpAnsiIn, len,
lpWCStr, len);
for (i = 0; i < nChar; ++i)
if (lpWCStr[i] == (WORD)'\t') 
lpWCStr[i] = (WORD)' ';

return nChar;
}


#if defined(FEAT_TEAROFF)



static HMENU
tearoff_lookup_menuhandle(
vimmenu_T *menu,
WORD menu_id)
{
for ( ; menu != NULL; menu = menu->next)
{
if (menu->modes == 0) 
continue;
if (menu_is_separator(menu->dname))
continue;
if ((WORD)((long_u)(menu->submenu_id) | (DWORD)0x8000) == menu_id)
return menu->submenu_id;
}
return NULL;
}






static LRESULT CALLBACK
tearoff_callback(
HWND hwnd,
UINT message,
WPARAM wParam,
LPARAM lParam)
{
if (message == WM_INITDIALOG)
{
SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)lParam);
return (TRUE);
}


HandleMouseHide(message, lParam);

if (message == WM_COMMAND)
{
if ((WORD)(LOWORD(wParam)) & 0x8000)
{
POINT mp;
RECT rect;

if (GetCursorPos(&mp) && GetWindowRect(hwnd, &rect))
{
vimmenu_T *menu;

menu = (vimmenu_T*)GetWindowLongPtr(hwnd, DWLP_USER);
(void)TrackPopupMenu(
tearoff_lookup_menuhandle(menu, LOWORD(wParam)),
TPM_LEFTALIGN | TPM_LEFTBUTTON,
(int)rect.right - 8,
(int)mp.y,
(int)0, 
s_hwnd,
NULL);




}
}
else

PostMessage(s_hwnd, WM_COMMAND, LOWORD(wParam), 0);





(void)SetFocus(s_hwnd);
return TRUE;
}
if ((message == WM_SYSCOMMAND) && (wParam == SC_CLOSE))
{
DestroyWindow(hwnd);
return TRUE;
}


if (message == WM_EXITSIZEMOVE)
(void)SetActiveWindow(s_hwnd);

return FALSE;
}
#endif







static void
get_dialog_font_metrics(void)
{
HDC hdc;
HFONT hfontTools = 0;
DWORD dlgFontSize;
SIZE size;
#if defined(USE_SYSMENU_FONT)
LOGFONTW lfSysmenu;
#endif

s_usenewlook = FALSE;

#if defined(USE_SYSMENU_FONT)
if (gui_w32_get_menu_font(&lfSysmenu) == OK)
hfontTools = CreateFontIndirectW(&lfSysmenu);
else
#endif
hfontTools = CreateFont(-DLG_FONT_POINT_SIZE, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, VARIABLE_PITCH , DLG_FONT_NAME);

if (hfontTools)
{
hdc = GetDC(s_hwnd);
SelectObject(hdc, hfontTools);





GetTextExtentPoint(hdc,
"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz",
52, &size);
ReleaseDC(s_hwnd, hdc);

s_dlgfntwidth = (WORD)((size.cx / 26 + 1) / 2);
s_dlgfntheight = (WORD)size.cy;
s_usenewlook = TRUE;
}

if (!s_usenewlook)
{
dlgFontSize = GetDialogBaseUnits(); 
s_dlgfntwidth = LOWORD(dlgFontSize);
s_dlgfntheight = HIWORD(dlgFontSize);
}
}

#if defined(FEAT_MENU) && defined(FEAT_TEAROFF)




static void
gui_mch_tearoff(
char_u *title,
vimmenu_T *menu,
int initX,
int initY)
{
WORD *p, *pdlgtemplate, *pnumitems, *ptrueheight;
int template_len;
int nchar, textWidth, submenuWidth;
DWORD lStyle;
DWORD lExtendedStyle;
WORD dlgwidth;
WORD menuID;
vimmenu_T *pmenu;
vimmenu_T *top_menu;
vimmenu_T *the_menu = menu;
HWND hwnd;
HDC hdc;
HFONT font, oldFont;
int col, spaceWidth, len;
int columnWidths[2];
char_u *label, *text;
int acLen = 0;
int nameLen;
int padding0, padding1, padding2 = 0;
int sepPadding=0;
int x;
int y;
#if defined(USE_SYSMENU_FONT)
LOGFONTW lfSysmenu;
int use_lfSysmenu = FALSE;
#endif




if (IsWindow(menu->tearoff_handle))
{
POINT mp;
if (GetCursorPos((LPPOINT)&mp))
{
SetWindowPos(menu->tearoff_handle, NULL, mp.x, mp.y, 0, 0,
SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);
}
return;
}




if (*title == MNU_HIDDEN_CHAR)
title++;



template_len = DLG_ALLOC_SIZE;
pdlgtemplate = p = (WORD *)LocalAlloc(LPTR, template_len);
if (p == NULL)
return;

hwnd = GetDesktopWindow();
hdc = GetWindowDC(hwnd);
#if defined(USE_SYSMENU_FONT)
if (gui_w32_get_menu_font(&lfSysmenu) == OK)
{
font = CreateFontIndirectW(&lfSysmenu);
use_lfSysmenu = TRUE;
}
else
#endif
font = CreateFont(-DLG_FONT_POINT_SIZE, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
VARIABLE_PITCH , DLG_FONT_NAME);
if (s_usenewlook)
oldFont = SelectFont(hdc, font);
else
oldFont = SelectFont(hdc, GetStockObject(SYSTEM_FONT));



spaceWidth = GetTextWidth(hdc, (char_u *)" ", 1);



submenuWidth = 0;
for (col = 0; col < 2; col++)
{
columnWidths[col] = 0;
for (pmenu = menu->children; pmenu != NULL; pmenu = pmenu->next)
{

text = (col == 0) ? pmenu->dname : pmenu->actext;
if (text != NULL && *text != NUL)
{
textWidth = GetTextWidthEnc(hdc, text, (int)STRLEN(text));
if (textWidth > columnWidths[col])
columnWidths[col] = textWidth;
}
if (pmenu->children != NULL)
submenuWidth = TEAROFF_COLUMN_PADDING * spaceWidth;
}
}
if (columnWidths[1] == 0)
{

if (submenuWidth != 0)
columnWidths[0] += submenuWidth;
else
columnWidths[0] += spaceWidth;
}
else
{

columnWidths[0] += TEAROFF_COLUMN_PADDING * spaceWidth;
columnWidths[1] += submenuWidth;
}




textWidth = columnWidths[0] + columnWidths[1];
if (submenuWidth != 0)
{
submenuWidth = GetTextWidth(hdc, (char_u *)TEAROFF_SUBMENU_LABEL,
(int)STRLEN(TEAROFF_SUBMENU_LABEL));
textWidth += submenuWidth;
}
dlgwidth = GetTextWidthEnc(hdc, title, (int)STRLEN(title));
if (textWidth > dlgwidth)
dlgwidth = textWidth;
dlgwidth += 2 * TEAROFF_PADDING_X + TEAROFF_BUTTON_PAD_X;


if (s_usenewlook)
lStyle = DS_MODALFRAME | WS_CAPTION| WS_SYSMENU |DS_SETFONT| WS_VISIBLE;
else
lStyle = DS_MODALFRAME | WS_CAPTION| WS_SYSMENU | WS_VISIBLE;

lExtendedStyle = WS_EX_TOOLWINDOW|WS_EX_STATICEDGE;
*p++ = LOWORD(lStyle);
*p++ = HIWORD(lStyle);
*p++ = LOWORD(lExtendedStyle);
*p++ = HIWORD(lExtendedStyle);
pnumitems = p; 
*p++ = 0; 
gui_mch_getmouse(&x, &y);
if (initX == 0xffffL)
*p++ = PixelToDialogX(x); 
else
*p++ = PixelToDialogX(initX); 
if (initY == 0xffffL)
*p++ = PixelToDialogY(y); 
else
*p++ = PixelToDialogY(initY); 
*p++ = PixelToDialogX(dlgwidth); 
ptrueheight = p;
*p++ = 0; 
*p++ = 0; 
*p++ = 0; 


nchar = nCopyAnsiToWideChar(p, ((*title)
? (LPSTR)title
: (LPSTR)("Vim "VIM_VERSION_MEDIUM)), TRUE);
p += nchar;

if (s_usenewlook)
{

#if defined(USE_SYSMENU_FONT)
if (use_lfSysmenu)
{

*p++ = -MulDiv(lfSysmenu.lfHeight, 72,
GetDeviceCaps(hdc, LOGPIXELSY));
wcscpy(p, lfSysmenu.lfFaceName);
nchar = (int)wcslen(lfSysmenu.lfFaceName) + 1;
}
else
#endif
{
*p++ = DLG_FONT_POINT_SIZE; 
nchar = nCopyAnsiToWideChar(p, DLG_FONT_NAME, FALSE);
}
p += nchar;
}





if (STRCMP(menu->children->name, TEAR_STRING) == 0)
menu = menu->children->next;
else
menu = menu->children;
top_menu = menu;
for ( ; menu != NULL; menu = menu->next)
{
if (menu->modes == 0) 
continue;
if (menu_is_separator(menu->dname))
{
sepPadding += 3;
continue;
}



if (((char *)p - (char *)pdlgtemplate) + 1000 > template_len)
{
WORD *newp;

newp = (WORD *)LocalAlloc(LPTR, template_len + 4096);
if (newp != NULL)
{
template_len += 4096;
mch_memmove(newp, pdlgtemplate,
(char *)p - (char *)pdlgtemplate);
p = newp + (p - pdlgtemplate);
pnumitems = newp + (pnumitems - pdlgtemplate);
ptrueheight = newp + (ptrueheight - pdlgtemplate);
LocalFree(LocalHandle(pdlgtemplate));
pdlgtemplate = newp;
}
}




len = nameLen = (int)STRLEN(menu->name);
padding0 = (columnWidths[0] - GetTextWidthEnc(hdc, menu->dname,
(int)STRLEN(menu->dname))) / spaceWidth;
len += padding0;

if (menu->actext != NULL)
{
acLen = (int)STRLEN(menu->actext);
len += acLen;
textWidth = GetTextWidthEnc(hdc, menu->actext, acLen);
}
else
textWidth = 0;
padding1 = (columnWidths[1] - textWidth) / spaceWidth;
len += padding1;

if (menu->children == NULL)
{
padding2 = submenuWidth / spaceWidth;
len += padding2;
menuID = (WORD)(menu->id);
}
else
{
len += (int)STRLEN(TEAROFF_SUBMENU_LABEL);
menuID = (WORD)((long_u)(menu->submenu_id) | (DWORD)0x8000);
}


text = label = alloc(len + 1);
if (label == NULL)
break;

vim_strncpy(text, menu->name, nameLen);
text = vim_strchr(text, TAB); 
if (text == NULL)
text = label + nameLen; 
while (padding0-- > 0)
*text++ = ' ';
if (menu->actext != NULL)
{
STRNCPY(text, menu->actext, acLen);
text += acLen;
}
while (padding1-- > 0)
*text++ = ' ';
if (menu->children != NULL)
{
STRCPY(text, TEAROFF_SUBMENU_LABEL);
text += STRLEN(TEAROFF_SUBMENU_LABEL);
}
else
{
while (padding2-- > 0)
*text++ = ' ';
}
*text = NUL;





p = add_dialog_element(p,
BS_PUSHBUTTON|BS_LEFT,
(WORD)PixelToDialogX(TEAROFF_PADDING_X),
(WORD)(sepPadding + 1 + 13 * (*pnumitems)),
(WORD)PixelToDialogX(dlgwidth - 2 * TEAROFF_PADDING_X),
(WORD)12,
menuID, (WORD)0x0080, (char *)label);
vim_free(label);
(*pnumitems)++;
}

*ptrueheight = (WORD)(sepPadding + 1 + 13 * (*pnumitems));



the_menu->tearoff_handle = CreateDialogIndirectParam(
g_hinst,
(LPDLGTEMPLATE)pdlgtemplate,
s_hwnd,
(DLGPROC)tearoff_callback,
(LPARAM)top_menu);

LocalFree(LocalHandle(pdlgtemplate));
SelectFont(hdc, oldFont);
DeleteObject(font);
ReleaseDC(hwnd, hdc);






(void)SetActiveWindow(s_hwnd);


force_menu_update = TRUE;
}
#endif

#if defined(FEAT_TOOLBAR) || defined(PROTO)
#include "gui_w32_rc.h"


#if !defined(TBSTYLE_FLAT)
#define TBSTYLE_FLAT 0x0800
#endif






static void
initialise_toolbar(void)
{
InitCommonControls();
s_toolbarhwnd = CreateToolbarEx(
s_hwnd,
WS_CHILD | TBSTYLE_TOOLTIPS | TBSTYLE_FLAT,
4000, 
31, 
g_hinst,
IDR_TOOLBAR1, 
NULL,
0, 
TOOLBAR_BUTTON_WIDTH, 
TOOLBAR_BUTTON_HEIGHT,
TOOLBAR_BUTTON_WIDTH,
TOOLBAR_BUTTON_HEIGHT,
sizeof(TBBUTTON)
);



SendMessage(s_toolbarhwnd, TB_SETSTYLE, 0,
SendMessage(s_toolbarhwnd, TB_GETSTYLE, 0, 0) & ~TBSTYLE_TRANSPARENT);

s_toolbar_wndproc = SubclassWindow(s_toolbarhwnd, toolbar_wndproc);

gui_mch_show_toolbar(vim_strchr(p_go, GO_TOOLBAR) != NULL);
}

static LRESULT CALLBACK
toolbar_wndproc(
HWND hwnd,
UINT uMsg,
WPARAM wParam,
LPARAM lParam)
{
HandleMouseHide(uMsg, lParam);
return CallWindowProc(s_toolbar_wndproc, hwnd, uMsg, wParam, lParam);
}

static int
get_toolbar_bitmap(vimmenu_T *menu)
{
int i = -1;




if (!menu->icon_builtin)
{
char_u fname[MAXPATHL];
HANDLE hbitmap = NULL;

if (menu->iconfile != NULL)
{
gui_find_iconfile(menu->iconfile, fname, "bmp");
hbitmap = LoadImage(
NULL,
(LPCSTR)fname,
IMAGE_BITMAP,
TOOLBAR_BUTTON_WIDTH,
TOOLBAR_BUTTON_HEIGHT,
LR_LOADFROMFILE |
LR_LOADMAP3DCOLORS
);
}





if (hbitmap == NULL
&& (gui_find_bitmap(
#if defined(FEAT_MULTI_LANG)
menu->en_dname != NULL ? menu->en_dname :
#endif
menu->dname, fname, "bmp") == OK))
hbitmap = LoadImage(
NULL,
(LPCSTR)fname,
IMAGE_BITMAP,
TOOLBAR_BUTTON_WIDTH,
TOOLBAR_BUTTON_HEIGHT,
LR_LOADFROMFILE |
LR_LOADMAP3DCOLORS
);

if (hbitmap != NULL)
{
TBADDBITMAP tbAddBitmap;

tbAddBitmap.hInst = NULL;
tbAddBitmap.nID = (long_u)hbitmap;

i = (int)SendMessage(s_toolbarhwnd, TB_ADDBITMAP,
(WPARAM)1, (LPARAM)&tbAddBitmap);

}
}
if (i == -1 && menu->iconidx >= 0 && menu->iconidx < TOOLBAR_BITMAP_COUNT)
i = menu->iconidx;

return i;
}
#endif

#if defined(FEAT_GUI_TABLINE) || defined(PROTO)
static void
initialise_tabline(void)
{
InitCommonControls();

s_tabhwnd = CreateWindow(WC_TABCONTROL, "Vim tabline",
WS_CHILD|TCS_FOCUSNEVER|TCS_TOOLTIPS,
CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
CW_USEDEFAULT, s_hwnd, NULL, g_hinst, NULL);
s_tabline_wndproc = SubclassWindow(s_tabhwnd, tabline_wndproc);

gui.tabline_height = TABLINE_HEIGHT;

#if defined(USE_SYSMENU_FONT)
set_tabline_font();
#endif
}




static tabpage_T *
GetTabFromPoint(
HWND hWnd,
POINT pt)
{
tabpage_T *ptp = NULL;

if (gui_mch_showing_tabline())
{
TCHITTESTINFO htinfo;
htinfo.pt = pt;

if (s_tabhwnd == hWnd)
{
int idx = TabCtrl_HitTest(s_tabhwnd, &htinfo);
if (idx != -1)
ptp = find_tabpage(idx + 1);
}
}
return ptp;
}

static POINT s_pt = {0, 0};
static HCURSOR s_hCursor = NULL;

static LRESULT CALLBACK
tabline_wndproc(
HWND hwnd,
UINT uMsg,
WPARAM wParam,
LPARAM lParam)
{
POINT pt;
tabpage_T *tp;
RECT rect;
int nCenter;
int idx0;
int idx1;

HandleMouseHide(uMsg, lParam);

switch (uMsg)
{
case WM_LBUTTONDOWN:
{
s_pt.x = GET_X_LPARAM(lParam);
s_pt.y = GET_Y_LPARAM(lParam);
SetCapture(hwnd);
s_hCursor = GetCursor(); 
break;
}
case WM_MOUSEMOVE:
if (GetCapture() == hwnd
&& ((wParam & MK_LBUTTON)) != 0)
{
pt.x = GET_X_LPARAM(lParam);
pt.y = s_pt.y;
if (abs(pt.x - s_pt.x) > GetSystemMetrics(SM_CXDRAG))
{
SetCursor(LoadCursor(NULL, IDC_SIZEWE));

tp = GetTabFromPoint(hwnd, pt);
if (tp != NULL)
{
idx0 = tabpage_index(curtab) - 1;
idx1 = tabpage_index(tp) - 1;

TabCtrl_GetItemRect(hwnd, idx1, &rect);
nCenter = rect.left + (rect.right - rect.left) / 2;



if ((idx0 < idx1) && (nCenter < pt.x))
{
tabpage_move(idx1 + 1);
update_screen(0);
}
else if ((idx1 < idx0) && (pt.x < nCenter))
{
tabpage_move(idx1);
update_screen(0);
}
}
}
}
break;
case WM_LBUTTONUP:
{
if (GetCapture() == hwnd)
{
SetCursor(s_hCursor);
ReleaseCapture();
}
break;
}
default:
break;
}

return CallWindowProc(s_tabline_wndproc, hwnd, uMsg, wParam, lParam);
}
#endif

#if defined(FEAT_OLE) || defined(FEAT_EVAL) || defined(PROTO)



void
gui_mch_set_foreground(void)
{
if (IsIconic(s_hwnd))
SendMessage(s_hwnd, WM_SYSCOMMAND, SC_RESTORE, 0);
SetForegroundWindow(s_hwnd);
}
#endif

#if defined(FEAT_MBYTE_IME) && defined(DYNAMIC_IME)
static void
dyn_imm_load(void)
{
hLibImm = vimLoadLib("imm32.dll");
if (hLibImm == NULL)
return;

pImmGetCompositionStringA
= (void *)GetProcAddress(hLibImm, "ImmGetCompositionStringA");
pImmGetCompositionStringW
= (void *)GetProcAddress(hLibImm, "ImmGetCompositionStringW");
pImmGetContext
= (void *)GetProcAddress(hLibImm, "ImmGetContext");
pImmAssociateContext
= (void *)GetProcAddress(hLibImm, "ImmAssociateContext");
pImmReleaseContext
= (void *)GetProcAddress(hLibImm, "ImmReleaseContext");
pImmGetOpenStatus
= (void *)GetProcAddress(hLibImm, "ImmGetOpenStatus");
pImmSetOpenStatus
= (void *)GetProcAddress(hLibImm, "ImmSetOpenStatus");
pImmGetCompositionFontW
= (void *)GetProcAddress(hLibImm, "ImmGetCompositionFontW");
pImmSetCompositionFontW
= (void *)GetProcAddress(hLibImm, "ImmSetCompositionFontW");
pImmSetCompositionWindow
= (void *)GetProcAddress(hLibImm, "ImmSetCompositionWindow");
pImmGetConversionStatus
= (void *)GetProcAddress(hLibImm, "ImmGetConversionStatus");
pImmSetConversionStatus
= (void *)GetProcAddress(hLibImm, "ImmSetConversionStatus");

if ( pImmGetCompositionStringA == NULL
|| pImmGetCompositionStringW == NULL
|| pImmGetContext == NULL
|| pImmAssociateContext == NULL
|| pImmReleaseContext == NULL
|| pImmGetOpenStatus == NULL
|| pImmSetOpenStatus == NULL
|| pImmGetCompositionFontW == NULL
|| pImmSetCompositionFontW == NULL
|| pImmSetCompositionWindow == NULL
|| pImmGetConversionStatus == NULL
|| pImmSetConversionStatus == NULL)
{
FreeLibrary(hLibImm);
hLibImm = NULL;
pImmGetContext = NULL;
return;
}

return;
}

#endif

#if defined(FEAT_SIGN_ICONS) || defined(PROTO)

#if defined(FEAT_XPM_W32)
#define IMAGE_XPM 100
#endif

typedef struct _signicon_t
{
HANDLE hImage;
UINT uType;
#if defined(FEAT_XPM_W32)
HANDLE hShape; 
#endif
} signicon_t;

void
gui_mch_drawsign(int row, int col, int typenr)
{
signicon_t *sign;
int x, y, w, h;

if (!gui.in_use || (sign = (signicon_t *)sign_get_image(typenr)) == NULL)
return;

#if defined(FEAT_DIRECTX)
if (IS_ENABLE_DIRECTX())
DWriteContext_Flush(s_dwc);
#endif

x = TEXT_X(col);
y = TEXT_Y(row);
w = gui.char_width * 2;
h = gui.char_height;
switch (sign->uType)
{
case IMAGE_BITMAP:
{
HDC hdcMem;
HBITMAP hbmpOld;

hdcMem = CreateCompatibleDC(s_hdc);
hbmpOld = (HBITMAP)SelectObject(hdcMem, sign->hImage);
BitBlt(s_hdc, x, y, w, h, hdcMem, 0, 0, SRCCOPY);
SelectObject(hdcMem, hbmpOld);
DeleteDC(hdcMem);
}
break;
case IMAGE_ICON:
case IMAGE_CURSOR:
DrawIconEx(s_hdc, x, y, (HICON)sign->hImage, w, h, 0, NULL, DI_NORMAL);
break;
#if defined(FEAT_XPM_W32)
case IMAGE_XPM:
{
HDC hdcMem;
HBITMAP hbmpOld;

hdcMem = CreateCompatibleDC(s_hdc);
hbmpOld = (HBITMAP)SelectObject(hdcMem, sign->hShape);

BitBlt(s_hdc, x, y, w, h, hdcMem, 0, 0, SRCAND);

SelectObject(hdcMem, sign->hImage);

BitBlt(s_hdc, x, y, w, h, hdcMem, 0, 0, SRCPAINT);
SelectObject(hdcMem, hbmpOld);
DeleteDC(hdcMem);
}
break;
#endif
}
}

static void
close_signicon_image(signicon_t *sign)
{
if (sign)
switch (sign->uType)
{
case IMAGE_BITMAP:
DeleteObject((HGDIOBJ)sign->hImage);
break;
case IMAGE_CURSOR:
DestroyCursor((HCURSOR)sign->hImage);
break;
case IMAGE_ICON:
DestroyIcon((HICON)sign->hImage);
break;
#if defined(FEAT_XPM_W32)
case IMAGE_XPM:
DeleteObject((HBITMAP)sign->hImage);
DeleteObject((HBITMAP)sign->hShape);
break;
#endif
}
}

void *
gui_mch_register_sign(char_u *signfile)
{
signicon_t sign, *psign;
char_u *ext;

sign.hImage = NULL;
ext = signfile + STRLEN(signfile) - 4; 
if (ext > signfile)
{
int do_load = 1;

if (!STRICMP(ext, ".bmp"))
sign.uType = IMAGE_BITMAP;
else if (!STRICMP(ext, ".ico"))
sign.uType = IMAGE_ICON;
else if (!STRICMP(ext, ".cur") || !STRICMP(ext, ".ani"))
sign.uType = IMAGE_CURSOR;
else
do_load = 0;

if (do_load)
sign.hImage = (HANDLE)LoadImage(NULL, (LPCSTR)signfile, sign.uType,
gui.char_width * 2, gui.char_height,
LR_LOADFROMFILE | LR_CREATEDIBSECTION);
#if defined(FEAT_XPM_W32)
if (!STRICMP(ext, ".xpm"))
{
sign.uType = IMAGE_XPM;
LoadXpmImage((char *)signfile, (HBITMAP *)&sign.hImage,
(HBITMAP *)&sign.hShape);
}
#endif
}

psign = NULL;
if (sign.hImage && (psign = ALLOC_ONE(signicon_t)) != NULL)
*psign = sign;

if (!psign)
{
if (sign.hImage)
close_signicon_image(&sign);
emsg(_(e_signdata));
}
return (void *)psign;

}

void
gui_mch_destroy_sign(void *sign)
{
if (sign)
{
close_signicon_image((signicon_t *)sign);
vim_free(sign);
}
}
#endif

#if defined(FEAT_BEVAL_GUI) || defined(PROTO)


























int
multiline_balloon_available(void)
{
HINSTANCE hDll;
static char comctl_dll[] = "comctl32.dll";
static int multiline_tip = MAYBE;

if (multiline_tip != MAYBE)
return multiline_tip;

hDll = GetModuleHandle(comctl_dll);
if (hDll != NULL)
{
DLLGETVERSIONPROC pGetVer;
pGetVer = (DLLGETVERSIONPROC)GetProcAddress(hDll, "DllGetVersion");

if (pGetVer != NULL)
{
DLLVERSIONINFO dvi;
HRESULT hr;

ZeroMemory(&dvi, sizeof(dvi));
dvi.cbSize = sizeof(dvi);

hr = (*pGetVer)(&dvi);

if (SUCCEEDED(hr)
&& (dvi.dwMajorVersion > 4
|| (dvi.dwMajorVersion == 4
&& dvi.dwMinorVersion >= 70)))
{
multiline_tip = TRUE;
return multiline_tip;
}
}
else
{


DWORD dwHandle = 0;
DWORD len = GetFileVersionInfoSize(comctl_dll, &dwHandle);
if (len > 0)
{
VS_FIXEDFILEINFO *ver;
UINT vlen = 0;
void *data = alloc(len);

if ((data != NULL
&& GetFileVersionInfo(comctl_dll, 0, len, data)
&& VerQueryValue(data, "\\", (void **)&ver, &vlen)
&& vlen
&& HIWORD(ver->dwFileVersionMS) > 4)
|| ((HIWORD(ver->dwFileVersionMS) == 4
&& LOWORD(ver->dwFileVersionMS) >= 70)))
{
vim_free(data);
multiline_tip = TRUE;
return multiline_tip;
}
vim_free(data);
}
}
}
multiline_tip = FALSE;
return multiline_tip;
}

static void
make_tooltip(BalloonEval *beval, char *text, POINT pt)
{
TOOLINFOW *pti;
int ToolInfoSize;

if (multiline_balloon_available() == TRUE)
ToolInfoSize = sizeof(TOOLINFOW_NEW);
else
ToolInfoSize = sizeof(TOOLINFOW);

pti = alloc(ToolInfoSize);
if (pti == NULL)
return;

beval->balloon = CreateWindowExW(WS_EX_TOPMOST, TOOLTIPS_CLASSW,
NULL, WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,
CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
beval->target, NULL, g_hinst, NULL);

SetWindowPos(beval->balloon, HWND_TOPMOST, 0, 0, 0, 0,
SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

pti->cbSize = ToolInfoSize;
pti->uFlags = TTF_SUBCLASS;
pti->hwnd = beval->target;
pti->hinst = 0; 
pti->uId = ID_BEVAL_TOOLTIP;

if (multiline_balloon_available() == TRUE)
{
RECT rect;
TOOLINFOW_NEW *ptin = (TOOLINFOW_NEW *)pti;
pti->lpszText = LPSTR_TEXTCALLBACKW;
beval->tofree = enc_to_utf16((char_u*)text, NULL);
ptin->lParam = (LPARAM)beval->tofree;

if (GetClientRect(s_textArea, &rect))
SendMessageW(beval->balloon, TTM_SETMAXTIPWIDTH, 0,
(LPARAM)rect.right);
}
else
{

beval->tofree = enc_to_utf16((char_u*)text, NULL);
pti->lpszText = (LPWSTR)beval->tofree;
}


pti->rect.left = pt.x - 3;
pti->rect.top = pt.y - 3;
pti->rect.right = pt.x + 3;
pti->rect.bottom = pt.y + 3;

SendMessageW(beval->balloon, TTM_ADDTOOLW, 0, (LPARAM)pti);

SendMessageW(beval->balloon, TTM_SETDELAYTIME, TTDT_INITIAL, 10);


SendMessageW(beval->balloon, TTM_SETDELAYTIME, TTDT_AUTOPOP, 30000);





mouse_event(MOUSEEVENTF_MOVE, 2, 2, 0, 0);
mouse_event(MOUSEEVENTF_MOVE, (DWORD)-1, (DWORD)-1, 0, 0);
vim_free(pti);
}

static void
delete_tooltip(BalloonEval *beval)
{
PostMessage(beval->balloon, WM_CLOSE, 0, 0);
}

static VOID CALLBACK
BevalTimerProc(
HWND hwnd UNUSED,
UINT uMsg UNUSED,
UINT_PTR idEvent UNUSED,
DWORD dwTime)
{
POINT pt;
RECT rect;

if (cur_beval == NULL || cur_beval->showState == ShS_SHOWING || !p_beval)
return;

GetCursorPos(&pt);
if (WindowFromPoint(pt) != s_textArea)
return;

ScreenToClient(s_textArea, &pt);
GetClientRect(s_textArea, &rect);
if (!PtInRect(&rect, pt))
return;

if (LastActivity > 0
&& (dwTime - LastActivity) >= (DWORD)p_bdlay
&& (cur_beval->showState != ShS_PENDING
|| abs(cur_beval->x - pt.x) > 3
|| abs(cur_beval->y - pt.y) > 3))
{


cur_beval->showState = ShS_PENDING;
cur_beval->x = pt.x;
cur_beval->y = pt.y;



if (cur_beval->msgCB != NULL)
(*cur_beval->msgCB)(cur_beval, 0);
}
}

void
gui_mch_disable_beval_area(BalloonEval *beval UNUSED)
{

KillTimer(s_textArea, BevalTimerId);

}

void
gui_mch_enable_beval_area(BalloonEval *beval)
{

if (beval == NULL)
return;

BevalTimerId = SetTimer(s_textArea, 0, (UINT)(p_bdlay / 2), BevalTimerProc);

}

void
gui_mch_post_balloon(BalloonEval *beval, char_u *mesg)
{
POINT pt;

vim_free(beval->msg);
beval->msg = mesg == NULL ? NULL : vim_strsave(mesg);
if (beval->msg == NULL)
{
delete_tooltip(beval);
beval->showState = ShS_NEUTRAL;
return;
}


if (beval->showState == ShS_SHOWING)
return;
GetCursorPos(&pt);
ScreenToClient(s_textArea, &pt);

if (abs(beval->x - pt.x) < 3 && abs(beval->y - pt.y) < 3)
{

gui_mch_disable_beval_area(cur_beval);
beval->showState = ShS_SHOWING;
make_tooltip(beval, (char *)mesg, pt);
}

}

BalloonEval *
gui_mch_create_beval_area(
void *target UNUSED, 
char_u *mesg,
void (*mesgCB)(BalloonEval *, int),
void *clientData)
{

BalloonEval *beval;

if (mesg != NULL && mesgCB != NULL)
{
iemsg(_("E232: Cannot create BalloonEval with both message and callback"));
return NULL;
}

beval = ALLOC_CLEAR_ONE(BalloonEval);
if (beval != NULL)
{
beval->target = s_textArea;

beval->showState = ShS_NEUTRAL;
beval->msg = mesg;
beval->msgCB = mesgCB;
beval->clientData = clientData;

InitCommonControls();
cur_beval = beval;

if (p_beval)
gui_mch_enable_beval_area(beval);
}
return beval;
}

static void
Handle_WM_Notify(HWND hwnd UNUSED, LPNMHDR pnmh)
{
if (pnmh->idFrom != ID_BEVAL_TOOLTIP) 
return;

if (cur_beval != NULL)
{
switch (pnmh->code)
{
case TTN_SHOW:


break;
case TTN_POP: 

delete_tooltip(cur_beval);
gui_mch_enable_beval_area(cur_beval);


cur_beval->showState = ShS_NEUTRAL;
break;
case TTN_GETDISPINFO:
{

NMTTDISPINFO_NEW *info = (NMTTDISPINFO_NEW *)pnmh;
info->lpszText = (LPSTR)info->lParam;
info->uFlags |= TTF_DI_SETITEM;
}
break;
case TTN_GETDISPINFOW:
{

NMTTDISPINFOW_NEW *info = (NMTTDISPINFOW_NEW *)pnmh;
info->lpszText = (LPWSTR)info->lParam;
info->uFlags |= TTF_DI_SETITEM;
}
break;
}
}
}

static void
TrackUserActivity(UINT uMsg)
{
if ((uMsg >= WM_MOUSEFIRST && uMsg <= WM_MOUSELAST)
|| (uMsg >= WM_KEYFIRST && uMsg <= WM_KEYLAST))
LastActivity = GetTickCount();
}

void
gui_mch_destroy_beval_area(BalloonEval *beval)
{
#if defined(FEAT_VARTABS)
vim_free(beval->vts);
#endif
vim_free(beval->tofree);
vim_free(beval);
}
#endif 

#if defined(FEAT_NETBEANS_INTG) || defined(PROTO)




void
netbeans_draw_multisign_indicator(int row)
{
int i;
int y;
int x;

if (!netbeans_active())
return;

x = 0;
y = TEXT_Y(row);

#if defined(FEAT_DIRECTX)
if (IS_ENABLE_DIRECTX())
DWriteContext_Flush(s_dwc);
#endif

for (i = 0; i < gui.char_height - 3; i++)
SetPixel(s_hdc, x+2, y++, gui.currFgColor);

SetPixel(s_hdc, x+0, y, gui.currFgColor);
SetPixel(s_hdc, x+2, y, gui.currFgColor);
SetPixel(s_hdc, x+4, y++, gui.currFgColor);
SetPixel(s_hdc, x+1, y, gui.currFgColor);
SetPixel(s_hdc, x+2, y, gui.currFgColor);
SetPixel(s_hdc, x+3, y++, gui.currFgColor);
SetPixel(s_hdc, x+2, y, gui.currFgColor);
}
#endif
