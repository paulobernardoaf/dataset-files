#include "vim.h"
#if defined(FEAT_MZSCHEME)
#include "if_mzsch.h"
#endif
#include <sys/types.h>
#include <signal.h>
#include <limits.h>
#if !defined(PROTO)
#include <process.h>
#endif
#undef chdir
#if defined(__GNUC__)
#if !defined(__MINGW32__)
#include <dirent.h>
#endif
#else
#include <direct.h>
#endif
#if !defined(PROTO)
#if defined(FEAT_TITLE) && !defined(FEAT_GUI_MSWIN)
#include <shellapi.h>
#endif
#endif
#if defined(FEAT_JOB_CHANNEL)
#include <tlhelp32.h>
#endif
#if defined(__MINGW32__)
#if !defined(FROM_LEFT_1ST_BUTTON_PRESSED)
#define FROM_LEFT_1ST_BUTTON_PRESSED 0x0001
#endif
#if !defined(RIGHTMOST_BUTTON_PRESSED)
#define RIGHTMOST_BUTTON_PRESSED 0x0002
#endif
#if !defined(FROM_LEFT_2ND_BUTTON_PRESSED)
#define FROM_LEFT_2ND_BUTTON_PRESSED 0x0004
#endif
#if !defined(FROM_LEFT_3RD_BUTTON_PRESSED)
#define FROM_LEFT_3RD_BUTTON_PRESSED 0x0008
#endif
#if !defined(FROM_LEFT_4TH_BUTTON_PRESSED)
#define FROM_LEFT_4TH_BUTTON_PRESSED 0x0010
#endif
#if !defined(MOUSE_MOVED)
#define MOUSE_MOVED 0x0001
#endif
#if !defined(DOUBLE_CLICK)
#define DOUBLE_CLICK 0x0002
#endif
#endif
#if defined(MCH_WRITE_DUMP)
FILE* fdDump = NULL;
#endif
#if defined(PROTO)
#define WINAPI
typedef char * LPCSTR;
typedef char * LPWSTR;
typedef int ACCESS_MASK;
typedef int BOOL;
typedef int COLORREF;
typedef int CONSOLE_CURSOR_INFO;
typedef int COORD;
typedef int DWORD;
typedef int HANDLE;
typedef int LPHANDLE;
typedef int HDC;
typedef int HFONT;
typedef int HICON;
typedef int HINSTANCE;
typedef int HWND;
typedef int INPUT_RECORD;
typedef int INT;
typedef int KEY_EVENT_RECORD;
typedef int LOGFONT;
typedef int LPBOOL;
typedef int LPCTSTR;
typedef int LPDWORD;
typedef int LPSTR;
typedef int LPTSTR;
typedef int LPVOID;
typedef int MOUSE_EVENT_RECORD;
typedef int PACL;
typedef int PDWORD;
typedef int PHANDLE;
typedef int PRINTDLG;
typedef int PSECURITY_DESCRIPTOR;
typedef int PSID;
typedef int SECURITY_INFORMATION;
typedef int SHORT;
typedef int SMALL_RECT;
typedef int TEXTMETRIC;
typedef int TOKEN_INFORMATION_CLASS;
typedef int TRUSTEE;
typedef int WORD;
typedef int WCHAR;
typedef void VOID;
typedef int BY_HANDLE_FILE_INFORMATION;
typedef int SE_OBJECT_TYPE;
typedef int PSNSECINFO;
typedef int PSNSECINFOW;
typedef int STARTUPINFO;
typedef int PROCESS_INFORMATION;
typedef int LPSECURITY_ATTRIBUTES;
#define __stdcall 
#endif
#if !defined(FEAT_GUI_MSWIN) || defined(VIMDLL)
static HANDLE g_hConIn = INVALID_HANDLE_VALUE;
static HANDLE g_hConOut = INVALID_HANDLE_VALUE;
static SMALL_RECT g_srScrollRegion;
static COORD g_coord; 
static WORD g_attrDefault = 7; 
static WORD g_attrCurrent;
static int g_fCBrkPressed = FALSE; 
static int g_fCtrlCPressed = FALSE; 
static int g_fForceExit = FALSE; 
static void scroll(unsigned cLines);
static void set_scroll_region(unsigned left, unsigned top,
unsigned right, unsigned bottom);
static void set_scroll_region_tb(unsigned top, unsigned bottom);
static void set_scroll_region_lr(unsigned left, unsigned right);
static void insert_lines(unsigned cLines);
static void delete_lines(unsigned cLines);
static void gotoxy(unsigned x, unsigned y);
static void standout(void);
static int s_cursor_visible = TRUE;
static int did_create_conin = FALSE;
#endif
#if defined(FEAT_GUI_MSWIN)
static int s_dont_use_vimrun = TRUE;
static int need_vimrun_warning = FALSE;
static char *vimrun_path = "vimrun ";
#endif
static int win32_getattrs(char_u *name);
static int win32_setattrs(char_u *name, int attrs);
static int win32_set_archive(char_u *name);
static int conpty_working = 0;
static int conpty_type = 0;
static int conpty_stable = 0;
static void vtp_flag_init();
#if !defined(FEAT_GUI_MSWIN) || defined(VIMDLL)
static int vtp_working = 0;
static void vtp_init();
static void vtp_exit();
static int vtp_printf(char *format, ...);
static void vtp_sgr_bulk(int arg);
static void vtp_sgr_bulks(int argc, int *argv);
static guicolor_T save_console_bg_rgb;
static guicolor_T save_console_fg_rgb;
static guicolor_T store_console_bg_rgb;
static guicolor_T store_console_fg_rgb;
static int g_color_index_bg = 0;
static int g_color_index_fg = 7;
#if defined(FEAT_TERMGUICOLORS)
static int default_console_color_bg = 0x000000; 
static int default_console_color_fg = 0xc0c0c0; 
#endif
#if defined(FEAT_TERMGUICOLORS)
#define USE_VTP (vtp_working && is_term_win32() && (p_tgc || (!p_tgc && t_colors >= 256)))
#else
#define USE_VTP 0
#endif
static void set_console_color_rgb(void);
static void reset_console_color_rgb(void);
static void restore_console_color_rgb(void);
#endif
#if !defined(ENABLE_VIRTUAL_TERMINAL_PROCESSING)
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif
#if !defined(FEAT_GUI_MSWIN) || defined(VIMDLL)
static int suppress_winsize = 1; 
#endif
static char_u *exe_path = NULL;
static BOOL win8_or_later = FALSE;
#if !defined(FEAT_GUI_MSWIN) || defined(VIMDLL)
typedef struct _DYN_CONSOLE_SCREEN_BUFFER_INFOEX
{
ULONG cbSize;
COORD dwSize;
COORD dwCursorPosition;
WORD wAttributes;
SMALL_RECT srWindow;
COORD dwMaximumWindowSize;
WORD wPopupAttributes;
BOOL bFullscreenSupported;
COLORREF ColorTable[16];
} DYN_CONSOLE_SCREEN_BUFFER_INFOEX, *PDYN_CONSOLE_SCREEN_BUFFER_INFOEX;
typedef BOOL (WINAPI *PfnGetConsoleScreenBufferInfoEx)(HANDLE, PDYN_CONSOLE_SCREEN_BUFFER_INFOEX);
static PfnGetConsoleScreenBufferInfoEx pGetConsoleScreenBufferInfoEx;
typedef BOOL (WINAPI *PfnSetConsoleScreenBufferInfoEx)(HANDLE, PDYN_CONSOLE_SCREEN_BUFFER_INFOEX);
static PfnSetConsoleScreenBufferInfoEx pSetConsoleScreenBufferInfoEx;
static BOOL has_csbiex = FALSE;
#endif
typedef BOOL (WINAPI *PfnRtlGetVersion)(LPOSVERSIONINFOW);
#define MAKE_VER(major, minor, build) (((major) << 24) | ((minor) << 16) | (build))
static DWORD
get_build_number(void)
{
OSVERSIONINFOW osver = {sizeof(OSVERSIONINFOW)};
HMODULE hNtdll;
PfnRtlGetVersion pRtlGetVersion;
DWORD ver = MAKE_VER(0, 0, 0);
hNtdll = GetModuleHandle("ntdll.dll");
if (hNtdll != NULL)
{
pRtlGetVersion =
(PfnRtlGetVersion)GetProcAddress(hNtdll, "RtlGetVersion");
pRtlGetVersion(&osver);
ver = MAKE_VER(min(osver.dwMajorVersion, 255),
min(osver.dwMinorVersion, 255),
min(osver.dwBuildNumber, 32767));
}
return ver;
}
#if !defined(FEAT_GUI_MSWIN) || defined(VIMDLL)
static BOOL
read_console_input(
HANDLE hInput,
INPUT_RECORD *lpBuffer,
DWORD nLength,
LPDWORD lpEvents)
{
enum
{
IRSIZE = 10
};
static INPUT_RECORD s_irCache[IRSIZE];
static DWORD s_dwIndex = 0;
static DWORD s_dwMax = 0;
DWORD dwEvents;
int head;
int tail;
int i;
if (nLength == -2)
return (s_dwMax > 0) ? TRUE : FALSE;
if (!win8_or_later)
{
if (nLength == -1)
return PeekConsoleInputW(hInput, lpBuffer, 1, lpEvents);
return ReadConsoleInputW(hInput, lpBuffer, 1, &dwEvents);
}
if (s_dwMax == 0)
{
if (nLength == -1)
return PeekConsoleInputW(hInput, lpBuffer, 1, lpEvents);
if (!ReadConsoleInputW(hInput, s_irCache, IRSIZE, &dwEvents))
return FALSE;
s_dwIndex = 0;
s_dwMax = dwEvents;
if (dwEvents == 0)
{
*lpEvents = 0;
return TRUE;
}
if (s_dwMax > 1)
{
head = 0;
tail = s_dwMax - 1;
while (head != tail)
{
if (s_irCache[head].EventType == WINDOW_BUFFER_SIZE_EVENT
&& s_irCache[head + 1].EventType
== WINDOW_BUFFER_SIZE_EVENT)
{
for (i = head; i < tail; ++i)
s_irCache[i] = s_irCache[i + 1];
--tail;
continue;
}
head++;
}
s_dwMax = tail + 1;
}
}
*lpBuffer = s_irCache[s_dwIndex];
if (!(nLength == -1 || nLength == -2) && ++s_dwIndex >= s_dwMax)
s_dwMax = 0;
*lpEvents = 1;
return TRUE;
}
static BOOL
peek_console_input(
HANDLE hInput,
INPUT_RECORD *lpBuffer,
DWORD nLength UNUSED,
LPDWORD lpEvents)
{
return read_console_input(hInput, lpBuffer, -1, lpEvents);
}
#if defined(FEAT_CLIENTSERVER)
static DWORD
msg_wait_for_multiple_objects(
DWORD nCount,
LPHANDLE pHandles,
BOOL fWaitAll,
DWORD dwMilliseconds,
DWORD dwWakeMask)
{
if (read_console_input(NULL, NULL, -2, NULL))
return WAIT_OBJECT_0;
return MsgWaitForMultipleObjects(nCount, pHandles, fWaitAll,
dwMilliseconds, dwWakeMask);
}
#endif
#if !defined(FEAT_CLIENTSERVER)
static DWORD
wait_for_single_object(
HANDLE hHandle,
DWORD dwMilliseconds)
{
if (read_console_input(NULL, NULL, -2, NULL))
return WAIT_OBJECT_0;
return WaitForSingleObject(hHandle, dwMilliseconds);
}
#endif
#endif
static void
get_exe_name(void)
{
#define MAX_ENV_PATH_LEN 8192
char temp[MAX_ENV_PATH_LEN];
char_u *p;
if (exe_name == NULL)
{
GetModuleFileName(NULL, temp, MAX_ENV_PATH_LEN - 1);
if (*temp != NUL)
exe_name = FullName_save((char_u *)temp, FALSE);
}
if (exe_path == NULL && exe_name != NULL)
{
exe_path = vim_strnsave(exe_name,
(int)(gettail_sep(exe_name) - exe_name));
if (exe_path != NULL)
{
p = mch_getenv("PATH");
if (p == NULL
|| STRLEN(p) + STRLEN(exe_path) + 2 < MAX_ENV_PATH_LEN)
{
if (p == NULL || *p == NUL)
temp[0] = NUL;
else
{
STRCPY(temp, p);
STRCAT(temp, ";");
}
STRCAT(temp, exe_path);
vim_setenv((char_u *)"PATH", (char_u *)temp);
}
}
}
}
static void
unescape_shellxquote(char_u *p, char_u *escaped)
{
int l = (int)STRLEN(p);
int n;
while (*p != NUL)
{
if (*p == '^' && vim_strchr(escaped, p[1]) != NULL)
mch_memmove(p, p + 1, l--);
n = (*mb_ptr2len)(p);
p += n;
l -= n;
}
}
HINSTANCE
vimLoadLib(char *name)
{
HINSTANCE dll = NULL;
if (exe_path == NULL)
get_exe_name();
if (exe_path != NULL)
{
WCHAR old_dirw[MAXPATHL];
if (GetCurrentDirectoryW(MAXPATHL, old_dirw) != 0)
{
SetCurrentDirectory((LPCSTR)exe_path);
dll = LoadLibrary(name);
SetCurrentDirectoryW(old_dirw);
return dll;
}
}
return dll;
}
#if defined(VIMDLL) || defined(PROTO)
int
mch_is_gui_executable(void)
{
PBYTE pImage = (PBYTE)GetModuleHandle(NULL);
PIMAGE_DOS_HEADER pDOS = (PIMAGE_DOS_HEADER)pImage;
PIMAGE_NT_HEADERS pPE;
if (pDOS->e_magic != IMAGE_DOS_SIGNATURE)
return FALSE;
pPE = (PIMAGE_NT_HEADERS)(pImage + pDOS->e_lfanew);
if (pPE->Signature != IMAGE_NT_SIGNATURE)
return FALSE;
if (pPE->OptionalHeader.Subsystem == IMAGE_SUBSYSTEM_WINDOWS_GUI)
return TRUE;
return FALSE;
}
#endif
#if defined(DYNAMIC_ICONV) || defined(DYNAMIC_GETTEXT) || defined(PROTO)
static void *
get_imported_func_info(HINSTANCE hInst, const char *funcname, int info)
{
PBYTE pImage = (PBYTE)hInst;
PIMAGE_DOS_HEADER pDOS = (PIMAGE_DOS_HEADER)hInst;
PIMAGE_NT_HEADERS pPE;
PIMAGE_IMPORT_DESCRIPTOR pImpDesc;
PIMAGE_THUNK_DATA pIAT; 
PIMAGE_THUNK_DATA pINT; 
PIMAGE_IMPORT_BY_NAME pImpName;
if (pDOS->e_magic != IMAGE_DOS_SIGNATURE)
return NULL;
pPE = (PIMAGE_NT_HEADERS)(pImage + pDOS->e_lfanew);
if (pPE->Signature != IMAGE_NT_SIGNATURE)
return NULL;
pImpDesc = (PIMAGE_IMPORT_DESCRIPTOR)(pImage
+ pPE->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT]
.VirtualAddress);
for (; pImpDesc->FirstThunk; ++pImpDesc)
{
if (!pImpDesc->OriginalFirstThunk)
continue;
pIAT = (PIMAGE_THUNK_DATA)(pImage + pImpDesc->FirstThunk);
pINT = (PIMAGE_THUNK_DATA)(pImage + pImpDesc->OriginalFirstThunk);
for (; pIAT->u1.Function; ++pIAT, ++pINT)
{
if (IMAGE_SNAP_BY_ORDINAL(pINT->u1.Ordinal))
continue;
pImpName = (PIMAGE_IMPORT_BY_NAME)(pImage
+ (UINT_PTR)(pINT->u1.AddressOfData));
if (strcmp((char *)pImpName->Name, funcname) == 0)
{
switch (info)
{
case 0:
return (void *)pIAT->u1.Function;
case 1:
return (void *)(pImage + pImpDesc->Name);
default:
return NULL;
}
}
}
}
return NULL;
}
HINSTANCE
find_imported_module_by_funcname(HINSTANCE hInst, const char *funcname)
{
char *modulename;
modulename = (char *)get_imported_func_info(hInst, funcname, 1);
if (modulename != NULL)
return GetModuleHandleA(modulename);
return NULL;
}
void *
get_dll_import_func(HINSTANCE hInst, const char *funcname)
{
return get_imported_func_info(hInst, funcname, 0);
}
#endif
#if defined(DYNAMIC_GETTEXT) || defined(PROTO)
#if !defined(GETTEXT_DLL)
#define GETTEXT_DLL "libintl.dll"
#define GETTEXT_DLL_ALT1 "libintl-8.dll"
#define GETTEXT_DLL_ALT2 "intl.dll"
#endif
static char *null_libintl_gettext(const char *);
static char *null_libintl_ngettext(const char *, const char *, unsigned long n);
static char *null_libintl_textdomain(const char *);
static char *null_libintl_bindtextdomain(const char *, const char *);
static char *null_libintl_bind_textdomain_codeset(const char *, const char *);
static int null_libintl_wputenv(const wchar_t *);
static HINSTANCE hLibintlDLL = NULL;
char *(*dyn_libintl_gettext)(const char *) = null_libintl_gettext;
char *(*dyn_libintl_ngettext)(const char *, const char *, unsigned long n)
= null_libintl_ngettext;
char *(*dyn_libintl_textdomain)(const char *) = null_libintl_textdomain;
char *(*dyn_libintl_bindtextdomain)(const char *, const char *)
= null_libintl_bindtextdomain;
char *(*dyn_libintl_bind_textdomain_codeset)(const char *, const char *)
= null_libintl_bind_textdomain_codeset;
int (*dyn_libintl_wputenv)(const wchar_t *) = null_libintl_wputenv;
int
dyn_libintl_init(void)
{
int i;
static struct
{
char *name;
FARPROC *ptr;
} libintl_entry[] =
{
{"gettext", (FARPROC*)&dyn_libintl_gettext},
{"ngettext", (FARPROC*)&dyn_libintl_ngettext},
{"textdomain", (FARPROC*)&dyn_libintl_textdomain},
{"bindtextdomain", (FARPROC*)&dyn_libintl_bindtextdomain},
{NULL, NULL}
};
HINSTANCE hmsvcrt;
if (hLibintlDLL != NULL)
return 1;
hLibintlDLL = vimLoadLib(GETTEXT_DLL);
#if defined(GETTEXT_DLL_ALT1)
if (!hLibintlDLL)
hLibintlDLL = vimLoadLib(GETTEXT_DLL_ALT1);
#endif
#if defined(GETTEXT_DLL_ALT2)
if (!hLibintlDLL)
hLibintlDLL = vimLoadLib(GETTEXT_DLL_ALT2);
#endif
if (!hLibintlDLL)
{
if (p_verbose > 0)
{
verbose_enter();
semsg(_(e_loadlib), GETTEXT_DLL);
verbose_leave();
}
return 0;
}
for (i = 0; libintl_entry[i].name != NULL
&& libintl_entry[i].ptr != NULL; ++i)
{
if ((*libintl_entry[i].ptr = (FARPROC)GetProcAddress(hLibintlDLL,
libintl_entry[i].name)) == NULL)
{
dyn_libintl_end();
if (p_verbose > 0)
{
verbose_enter();
semsg(_(e_loadfunc), libintl_entry[i].name);
verbose_leave();
}
return 0;
}
}
dyn_libintl_bind_textdomain_codeset = (void *)GetProcAddress(hLibintlDLL,
"bind_textdomain_codeset");
if (dyn_libintl_bind_textdomain_codeset == NULL)
dyn_libintl_bind_textdomain_codeset =
null_libintl_bind_textdomain_codeset;
hmsvcrt = find_imported_module_by_funcname(hLibintlDLL, "getenv");
if (hmsvcrt != NULL)
dyn_libintl_wputenv = (void *)GetProcAddress(hmsvcrt, "_wputenv");
if (dyn_libintl_wputenv == NULL || dyn_libintl_wputenv == _wputenv)
dyn_libintl_wputenv = null_libintl_wputenv;
return 1;
}
void
dyn_libintl_end(void)
{
if (hLibintlDLL)
FreeLibrary(hLibintlDLL);
hLibintlDLL = NULL;
dyn_libintl_gettext = null_libintl_gettext;
dyn_libintl_ngettext = null_libintl_ngettext;
dyn_libintl_textdomain = null_libintl_textdomain;
dyn_libintl_bindtextdomain = null_libintl_bindtextdomain;
dyn_libintl_bind_textdomain_codeset = null_libintl_bind_textdomain_codeset;
dyn_libintl_wputenv = null_libintl_wputenv;
}
static char *
null_libintl_gettext(const char *msgid)
{
return (char*)msgid;
}
static char *
null_libintl_ngettext(
const char *msgid,
const char *msgid_plural,
unsigned long n)
{
return (char *)(n == 1 ? msgid : msgid_plural);
}
static char *
null_libintl_bindtextdomain(
const char *domainname UNUSED,
const char *dirname UNUSED)
{
return NULL;
}
static char *
null_libintl_bind_textdomain_codeset(
const char *domainname UNUSED,
const char *codeset UNUSED)
{
return NULL;
}
static char *
null_libintl_textdomain(const char *domainname UNUSED)
{
return NULL;
}
static int
null_libintl_wputenv(const wchar_t *envstring UNUSED)
{
return 0;
}
#endif 
#if !defined(VER_PLATFORM_WIN32_WINDOWS)
#define VER_PLATFORM_WIN32_WINDOWS 1
#endif
#if defined(HAVE_ACL)
#if !defined(PROTO)
#include <aclapi.h>
#endif
#if !defined(PROTECTED_DACL_SECURITY_INFORMATION)
#define PROTECTED_DACL_SECURITY_INFORMATION 0x80000000L
#endif
#endif
#if defined(HAVE_ACL)
static BOOL
win32_enable_privilege(LPTSTR lpszPrivilege, BOOL bEnable)
{
BOOL bResult;
LUID luid;
HANDLE hToken;
TOKEN_PRIVILEGES tokenPrivileges;
if (!OpenProcessToken(GetCurrentProcess(),
TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
return FALSE;
if (!LookupPrivilegeValue(NULL, lpszPrivilege, &luid))
{
CloseHandle(hToken);
return FALSE;
}
tokenPrivileges.PrivilegeCount = 1;
tokenPrivileges.Privileges[0].Luid = luid;
tokenPrivileges.Privileges[0].Attributes = bEnable ?
SE_PRIVILEGE_ENABLED : 0;
bResult = AdjustTokenPrivileges(hToken, FALSE, &tokenPrivileges,
sizeof(TOKEN_PRIVILEGES), NULL, NULL);
CloseHandle(hToken);
return bResult && GetLastError() == ERROR_SUCCESS;
}
#endif
void
PlatformId(void)
{
static int done = FALSE;
if (!done)
{
OSVERSIONINFO ovi;
ovi.dwOSVersionInfoSize = sizeof(ovi);
GetVersionEx(&ovi);
vim_snprintf(windowsVersion, sizeof(windowsVersion), "%d.%d",
(int)ovi.dwMajorVersion, (int)ovi.dwMinorVersion);
if ((ovi.dwMajorVersion == 6 && ovi.dwMinorVersion >= 2)
|| ovi.dwMajorVersion > 6)
win8_or_later = TRUE;
#if defined(HAVE_ACL)
win32_enable_privilege(SE_SECURITY_NAME, TRUE);
#endif
done = TRUE;
}
}
#if !defined(FEAT_GUI_MSWIN) || defined(VIMDLL)
#define SHIFT (SHIFT_PRESSED)
#define CTRL (RIGHT_CTRL_PRESSED | LEFT_CTRL_PRESSED)
#define ALT (RIGHT_ALT_PRESSED | LEFT_ALT_PRESSED)
#define ALT_GR (RIGHT_ALT_PRESSED | LEFT_CTRL_PRESSED)
static const struct
{
WORD wVirtKey;
BOOL fAnsiKey;
int chAlone;
int chShift;
int chCtrl;
int chAlt;
} VirtKeyMap[] =
{
{ VK_ESCAPE,FALSE, ESC, ESC, ESC, ESC, },
{ VK_F1, TRUE, ';', 'T', '^', 'h', },
{ VK_F2, TRUE, '<', 'U', '_', 'i', },
{ VK_F3, TRUE, '=', 'V', '`', 'j', },
{ VK_F4, TRUE, '>', 'W', 'a', 'k', },
{ VK_F5, TRUE, '?', 'X', 'b', 'l', },
{ VK_F6, TRUE, '@', 'Y', 'c', 'm', },
{ VK_F7, TRUE, 'A', 'Z', 'd', 'n', },
{ VK_F8, TRUE, 'B', '[', 'e', 'o', },
{ VK_F9, TRUE, 'C', '\\', 'f', 'p', },
{ VK_F10, TRUE, 'D', ']', 'g', 'q', },
{ VK_F11, TRUE, '\205', '\207', '\211', '\213', },
{ VK_F12, TRUE, '\206', '\210', '\212', '\214', },
{ VK_HOME, TRUE, 'G', '\302', 'w', '\303', },
{ VK_UP, TRUE, 'H', '\304', '\305', '\306', },
{ VK_PRIOR, TRUE, 'I', '\307', '\204', '\310', }, 
{ VK_LEFT, TRUE, 'K', '\311', 's', '\312', },
{ VK_RIGHT, TRUE, 'M', '\313', 't', '\314', },
{ VK_END, TRUE, 'O', '\315', 'u', '\316', },
{ VK_DOWN, TRUE, 'P', '\317', '\320', '\321', },
{ VK_NEXT, TRUE, 'Q', '\322', 'v', '\323', }, 
{ VK_INSERT,TRUE, 'R', '\324', '\325', '\326', },
{ VK_DELETE,TRUE, 'S', '\327', '\330', '\331', },
{ VK_BACK, TRUE, 'x', 'y', 'z', '{', }, 
{ VK_SNAPSHOT,TRUE, 0, 0, 0, 'r', }, 
#if 0
{ VK_F13, TRUE, '\332', '\333', '\334', '\335', },
{ VK_F14, TRUE, '\336', '\337', '\340', '\341', },
{ VK_F15, TRUE, '\342', '\343', '\344', '\345', },
{ VK_F16, TRUE, '\346', '\347', '\350', '\351', },
{ VK_F17, TRUE, '\352', '\353', '\354', '\355', },
{ VK_F18, TRUE, '\356', '\357', '\360', '\361', },
{ VK_F19, TRUE, '\362', '\363', '\364', '\365', },
{ VK_F20, TRUE, '\366', '\367', '\370', '\371', },
#endif
{ VK_ADD, TRUE, 'N', 'N', 'N', 'N', }, 
{ VK_SUBTRACT, TRUE,'J', 'J', 'J', 'J', }, 
{ VK_MULTIPLY, TRUE,'7', '7', '7', '7', }, 
{ VK_NUMPAD0,TRUE, '\332', '\333', '\334', '\335', },
{ VK_NUMPAD1,TRUE, '\336', '\337', '\340', '\341', },
{ VK_NUMPAD2,TRUE, '\342', '\343', '\344', '\345', },
{ VK_NUMPAD3,TRUE, '\346', '\347', '\350', '\351', },
{ VK_NUMPAD4,TRUE, '\352', '\353', '\354', '\355', },
{ VK_NUMPAD5,TRUE, '\356', '\357', '\360', '\361', },
{ VK_NUMPAD6,TRUE, '\362', '\363', '\364', '\365', },
{ VK_NUMPAD7,TRUE, '\366', '\367', '\370', '\371', },
{ VK_NUMPAD8,TRUE, '\372', '\373', '\374', '\375', },
{ VK_NUMPAD9,TRUE, '\376', '\377', '|', '}', },
};
#if defined(__GNUC__) && !defined(__MINGW32__) && !defined(__CYGWIN__)
#define UChar UnicodeChar
#else
#define UChar uChar.UnicodeChar
#endif
static int
win32_kbd_patch_key(
KEY_EVENT_RECORD *pker)
{
UINT uMods = pker->dwControlKeyState;
static int s_iIsDead = 0;
static WORD awAnsiCode[2];
static BYTE abKeystate[256];
if (s_iIsDead == 2)
{
pker->UChar = (WCHAR) awAnsiCode[1];
s_iIsDead = 0;
return 1;
}
if (pker->UChar != 0)
return 1;
vim_memset(abKeystate, 0, sizeof (abKeystate));
ToUnicode(VK_SPACE, MapVirtualKey(VK_SPACE, 0), abKeystate, awAnsiCode, 2, 0);
if (uMods & SHIFT_PRESSED)
abKeystate[VK_SHIFT] = 0x80;
if (uMods & CAPSLOCK_ON)
abKeystate[VK_CAPITAL] = 1;
if ((uMods & ALT_GR) == ALT_GR)
{
abKeystate[VK_CONTROL] = abKeystate[VK_LCONTROL] =
abKeystate[VK_MENU] = abKeystate[VK_RMENU] = 0x80;
}
s_iIsDead = ToUnicode(pker->wVirtualKeyCode, pker->wVirtualScanCode,
abKeystate, awAnsiCode, 2, 0);
if (s_iIsDead > 0)
pker->UChar = (WCHAR) awAnsiCode[0];
return s_iIsDead;
}
static BOOL g_fJustGotFocus = FALSE;
static BOOL
decode_key_event(
KEY_EVENT_RECORD *pker,
WCHAR *pch,
WCHAR *pch2,
int *pmodifiers,
BOOL fDoPost UNUSED)
{
int i;
const int nModifs = pker->dwControlKeyState & (SHIFT | ALT | CTRL);
*pch = *pch2 = NUL;
g_fJustGotFocus = FALSE;
if (!pker->bKeyDown)
return FALSE;
switch (pker->wVirtualKeyCode)
{
case VK_SHIFT:
case VK_CONTROL:
case VK_MENU: 
return FALSE;
default:
break;
}
if ((nModifs & CTRL) != 0 && (nModifs & ~CTRL) == 0 && pker->UChar == NUL)
{
if (pker->wVirtualKeyCode == '6')
{
*pch = Ctrl_HAT;
return TRUE;
}
else if (pker->wVirtualKeyCode == '2')
{
*pch = NUL;
return TRUE;
}
else if (pker->wVirtualKeyCode == 0xBD)
{
*pch = Ctrl__;
return TRUE;
}
}
if (pker->wVirtualKeyCode == VK_TAB && (nModifs & SHIFT_PRESSED))
{
*pch = K_NUL;
*pch2 = '\017';
return TRUE;
}
for (i = sizeof(VirtKeyMap) / sizeof(VirtKeyMap[0]); --i >= 0; )
{
if (VirtKeyMap[i].wVirtKey == pker->wVirtualKeyCode)
{
if (nModifs == 0)
*pch = VirtKeyMap[i].chAlone;
else if ((nModifs & SHIFT) != 0 && (nModifs & ~SHIFT) == 0)
*pch = VirtKeyMap[i].chShift;
else if ((nModifs & CTRL) != 0 && (nModifs & ~CTRL) == 0)
*pch = VirtKeyMap[i].chCtrl;
else if ((nModifs & ALT) != 0 && (nModifs & ~ALT) == 0)
*pch = VirtKeyMap[i].chAlt;
if (*pch != 0)
{
if (VirtKeyMap[i].fAnsiKey)
{
*pch2 = *pch;
*pch = K_NUL;
}
return TRUE;
}
}
}
i = win32_kbd_patch_key(pker);
if (i < 0)
*pch = NUL;
else
{
*pch = (i > 0) ? pker->UChar : NUL;
if (pmodifiers != NULL)
{
if ((nModifs & ALT) != 0 && (nModifs & CTRL) == 0)
*pmodifiers |= MOD_MASK_ALT;
if ((nModifs & SHIFT) != 0 && *pch <= 0x20)
*pmodifiers |= MOD_MASK_SHIFT;
if ((nModifs & CTRL) != 0 && (nModifs & ALT) == 0
&& *pch >= 0x20 && *pch < 0x80)
*pmodifiers |= MOD_MASK_CTRL;
}
}
return (*pch != NUL);
}
#endif 
#if defined(FEAT_GUI_MSWIN) && !defined(VIMDLL)
void
mch_setmouse(int on UNUSED)
{
}
#else
static int g_fMouseAvail = FALSE; 
static int g_fMouseActive = FALSE; 
static int g_nMouseClick = -1; 
static int g_xMouse; 
static int g_yMouse; 
void
mch_setmouse(int on)
{
DWORD cmodein;
#if defined(VIMDLL)
if (gui.in_use)
return;
#endif
if (!g_fMouseAvail)
return;
g_fMouseActive = on;
GetConsoleMode(g_hConIn, &cmodein);
if (g_fMouseActive)
cmodein |= ENABLE_MOUSE_INPUT;
else
cmodein &= ~ENABLE_MOUSE_INPUT;
SetConsoleMode(g_hConIn, cmodein);
}
#if defined(FEAT_BEVAL_TERM) || defined(PROTO)
void
mch_bevalterm_changed(void)
{
mch_setmouse(g_fMouseActive);
}
#endif
static BOOL
decode_mouse_event(
MOUSE_EVENT_RECORD *pmer)
{
static int s_nOldButton = -1;
static int s_nOldMouseClick = -1;
static int s_xOldMouse = -1;
static int s_yOldMouse = -1;
static linenr_T s_old_topline = 0;
#if defined(FEAT_DIFF)
static int s_old_topfill = 0;
#endif
static int s_cClicks = 1;
static BOOL s_fReleased = TRUE;
static DWORD s_dwLastClickTime = 0;
static BOOL s_fNextIsMiddle = FALSE;
static DWORD cButtons = 0; 
const DWORD LEFT = FROM_LEFT_1ST_BUTTON_PRESSED;
const DWORD MIDDLE = FROM_LEFT_2ND_BUTTON_PRESSED;
const DWORD RIGHT = RIGHTMOST_BUTTON_PRESSED;
const DWORD LEFT_RIGHT = LEFT | RIGHT;
int nButton;
if (cButtons == 0 && !GetNumberOfConsoleMouseButtons(&cButtons))
cButtons = 2;
if (!g_fMouseAvail || !g_fMouseActive)
{
g_nMouseClick = -1;
return FALSE;
}
if (g_fJustGotFocus)
{
g_fJustGotFocus = FALSE;
return FALSE;
}
if (g_nMouseClick != -1)
return TRUE;
nButton = -1;
g_xMouse = pmer->dwMousePosition.X;
g_yMouse = pmer->dwMousePosition.Y;
if (pmer->dwEventFlags == MOUSE_MOVED)
{
if (s_xOldMouse == g_xMouse && s_yOldMouse == g_yMouse)
return FALSE;
}
if ((pmer->dwButtonState & ((1 << cButtons) - 1)) == 0)
{
nButton = MOUSE_RELEASE;
if (s_fReleased)
{
#if defined(FEAT_BEVAL_TERM)
if (p_bevalterm)
nButton = MOUSE_DRAG;
else
#endif
return FALSE;
}
s_fReleased = TRUE;
}
else 
{
if (cButtons == 2 && s_nOldButton != MOUSE_DRAG)
{
DWORD dwLR = (pmer->dwButtonState & LEFT_RIGHT);
if (dwLR == LEFT || dwLR == RIGHT)
{
for (;;)
{
if (WaitForSingleObject(g_hConIn, p_mouset / 3)
!= WAIT_OBJECT_0)
break;
else
{
DWORD cRecords = 0;
INPUT_RECORD ir;
MOUSE_EVENT_RECORD* pmer2 = &ir.Event.MouseEvent;
peek_console_input(g_hConIn, &ir, 1, &cRecords);
if (cRecords == 0 || ir.EventType != MOUSE_EVENT
|| !(pmer2->dwButtonState & LEFT_RIGHT))
break;
else
{
if (pmer2->dwEventFlags != MOUSE_MOVED)
{
read_console_input(g_hConIn, &ir, 1, &cRecords);
return decode_mouse_event(pmer2);
}
else if (s_xOldMouse == pmer2->dwMousePosition.X &&
s_yOldMouse == pmer2->dwMousePosition.Y)
{
read_console_input(g_hConIn, &ir, 1, &cRecords);
peek_console_input(g_hConIn, &ir, 1, &cRecords);
if (cRecords==0 || ir.EventType != MOUSE_EVENT)
break;
}
else
break;
}
}
}
}
}
if (s_fNextIsMiddle)
{
nButton = (pmer->dwEventFlags == MOUSE_MOVED)
? MOUSE_DRAG : MOUSE_MIDDLE;
s_fNextIsMiddle = FALSE;
}
else if (cButtons == 2 &&
((pmer->dwButtonState & LEFT_RIGHT) == LEFT_RIGHT))
{
nButton = MOUSE_MIDDLE;
if (! s_fReleased && pmer->dwEventFlags != MOUSE_MOVED)
{
s_fNextIsMiddle = TRUE;
nButton = MOUSE_RELEASE;
}
}
else if ((pmer->dwButtonState & LEFT) == LEFT)
nButton = MOUSE_LEFT;
else if ((pmer->dwButtonState & MIDDLE) == MIDDLE)
nButton = MOUSE_MIDDLE;
else if ((pmer->dwButtonState & RIGHT) == RIGHT)
nButton = MOUSE_RIGHT;
if (! s_fReleased && ! s_fNextIsMiddle
&& nButton != s_nOldButton && s_nOldButton != MOUSE_DRAG)
return FALSE;
s_fReleased = s_fNextIsMiddle;
}
if (pmer->dwEventFlags == 0 || pmer->dwEventFlags == DOUBLE_CLICK)
{
if (nButton != -1 && nButton != MOUSE_RELEASE)
{
DWORD dwCurrentTime = GetTickCount();
if (s_xOldMouse != g_xMouse
|| s_yOldMouse != g_yMouse
|| s_nOldButton != nButton
|| s_old_topline != curwin->w_topline
#if defined(FEAT_DIFF)
|| s_old_topfill != curwin->w_topfill
#endif
|| (int)(dwCurrentTime - s_dwLastClickTime) > p_mouset)
{
s_cClicks = 1;
}
else if (++s_cClicks > 4)
{
s_cClicks = 1;
}
s_dwLastClickTime = dwCurrentTime;
}
}
else if (pmer->dwEventFlags == MOUSE_MOVED)
{
if (nButton != -1 && nButton != MOUSE_RELEASE)
nButton = MOUSE_DRAG;
s_cClicks = 1;
}
if (nButton == -1)
return FALSE;
if (nButton != MOUSE_RELEASE)
s_nOldButton = nButton;
g_nMouseClick = nButton;
if (pmer->dwControlKeyState & SHIFT_PRESSED)
g_nMouseClick |= MOUSE_SHIFT;
if (pmer->dwControlKeyState & (RIGHT_CTRL_PRESSED | LEFT_CTRL_PRESSED))
g_nMouseClick |= MOUSE_CTRL;
if (pmer->dwControlKeyState & (RIGHT_ALT_PRESSED | LEFT_ALT_PRESSED))
g_nMouseClick |= MOUSE_ALT;
if (nButton != MOUSE_DRAG && nButton != MOUSE_RELEASE)
SET_NUM_MOUSE_CLICKS(g_nMouseClick, s_cClicks);
if (s_xOldMouse == g_xMouse
&& s_yOldMouse == g_yMouse
&& s_nOldMouseClick == g_nMouseClick)
{
g_nMouseClick = -1;
return FALSE;
}
s_xOldMouse = g_xMouse;
s_yOldMouse = g_yMouse;
s_old_topline = curwin->w_topline;
#if defined(FEAT_DIFF)
s_old_topfill = curwin->w_topfill;
#endif
s_nOldMouseClick = g_nMouseClick;
return TRUE;
}
#endif 
#if defined(MCH_CURSOR_SHAPE)
static void
mch_set_cursor_shape(int thickness)
{
CONSOLE_CURSOR_INFO ConsoleCursorInfo;
ConsoleCursorInfo.dwSize = thickness;
ConsoleCursorInfo.bVisible = s_cursor_visible;
SetConsoleCursorInfo(g_hConOut, &ConsoleCursorInfo);
if (s_cursor_visible)
SetConsoleCursorPosition(g_hConOut, g_coord);
}
void
mch_update_cursor(void)
{
int idx;
int thickness;
#if defined(VIMDLL)
if (gui.in_use)
return;
#endif
idx = get_shape_idx(FALSE);
if (shape_table[idx].shape == SHAPE_BLOCK)
thickness = 99; 
else
thickness = shape_table[idx].percentage;
mch_set_cursor_shape(thickness);
}
#endif
#if !defined(FEAT_GUI_MSWIN) || defined(VIMDLL)
static void
handle_focus_event(INPUT_RECORD ir)
{
g_fJustGotFocus = ir.Event.FocusEvent.bSetFocus;
ui_focus_change((int)g_fJustGotFocus);
}
static void ResizeConBuf(HANDLE hConsole, COORD coordScreen);
static int
WaitForChar(long msec, int ignore_input)
{
DWORD dwNow = 0, dwEndTime = 0;
INPUT_RECORD ir;
DWORD cRecords;
WCHAR ch, ch2;
#if defined(FEAT_TIMERS)
int tb_change_cnt = typebuf.tb_change_cnt;
#endif
if (msec > 0)
dwEndTime = GetTickCount() + msec;
else if (msec < 0)
dwEndTime = INFINITE;
for (;;)
{
if (msec != 0)
{
#if defined(MESSAGE_QUEUE)
parse_queued_messages();
#endif
#if defined(FEAT_MZSCHEME)
mzvim_check_threads();
#endif
#if defined(FEAT_CLIENTSERVER)
serverProcessPendingMessages();
#endif
}
if (g_nMouseClick != -1
#if defined(FEAT_CLIENTSERVER)
|| (!ignore_input && input_available())
#endif
)
return TRUE;
if (msec > 0)
{
dwNow = GetTickCount();
if ((int)(dwNow - dwEndTime) >= 0)
break;
}
if (msec != 0)
{
DWORD dwWaitTime = dwEndTime - dwNow;
#if defined(FEAT_JOB_CHANNEL)
if (dwWaitTime > 100)
{
dwWaitTime = 100;
if (channel_any_readahead())
dwWaitTime = 10;
}
#endif
#if defined(FEAT_BEVAL_GUI)
if (p_beval && dwWaitTime > 100)
dwWaitTime = 100;
#endif
#if defined(FEAT_MZSCHEME)
if (mzthreads_allowed() && p_mzq > 0
&& (msec < 0 || (long)dwWaitTime > p_mzq))
dwWaitTime = p_mzq; 
#endif
#if defined(FEAT_TIMERS)
if (dwWaitTime > 10)
{
long due_time;
due_time = check_due_timer();
if (typebuf.tb_change_cnt != tb_change_cnt)
{
return FALSE;
}
if (due_time > 0 && dwWaitTime > (DWORD)due_time)
dwWaitTime = due_time;
}
#endif
if (
#if defined(FEAT_CLIENTSERVER)
msg_wait_for_multiple_objects(1, &g_hConIn, FALSE,
dwWaitTime, QS_SENDMESSAGE) != WAIT_OBJECT_0
#else
wait_for_single_object(g_hConIn, dwWaitTime)
!= WAIT_OBJECT_0
#endif
)
continue;
}
cRecords = 0;
peek_console_input(g_hConIn, &ir, 1, &cRecords);
#if defined(FEAT_MBYTE_IME)
if (State & CMDLINE && msg_row == Rows - 1)
{
CONSOLE_SCREEN_BUFFER_INFO csbi;
if (GetConsoleScreenBufferInfo(g_hConOut, &csbi))
{
if (csbi.dwCursorPosition.Y != msg_row)
{
redraw_all_later(CLEAR);
cmdline_row -= (msg_row - csbi.dwCursorPosition.Y);
redrawcmd();
}
}
}
#endif
if (cRecords > 0)
{
if (ir.EventType == KEY_EVENT && ir.Event.KeyEvent.bKeyDown)
{
#if defined(FEAT_MBYTE_IME)
if (ir.Event.KeyEvent.UChar == 0
&& ir.Event.KeyEvent.wVirtualKeyCode == 13)
{
read_console_input(g_hConIn, &ir, 1, &cRecords);
continue;
}
#endif
if (decode_key_event(&ir.Event.KeyEvent, &ch, &ch2,
NULL, FALSE))
return TRUE;
}
read_console_input(g_hConIn, &ir, 1, &cRecords);
if (ir.EventType == FOCUS_EVENT)
handle_focus_event(ir);
else if (ir.EventType == WINDOW_BUFFER_SIZE_EVENT)
{
COORD dwSize = ir.Event.WindowBufferSizeEvent.dwSize;
if (dwSize.X != Columns || dwSize.Y != Rows)
{
CONSOLE_SCREEN_BUFFER_INFO csbi;
GetConsoleScreenBufferInfo(g_hConOut, &csbi);
dwSize.X = csbi.srWindow.Right - csbi.srWindow.Left + 1;
dwSize.Y = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
if (dwSize.X != Columns || dwSize.Y != Rows)
{
ResizeConBuf(g_hConOut, dwSize);
shell_resized();
}
}
}
else if (ir.EventType == MOUSE_EVENT
&& decode_mouse_event(&ir.Event.MouseEvent))
return TRUE;
}
else if (msec == 0)
break;
}
#if defined(FEAT_CLIENTSERVER)
if (input_available())
return TRUE;
#endif
return FALSE;
}
int
mch_char_avail(void)
{
#if defined(VIMDLL)
if (gui.in_use)
return TRUE;
#endif
return WaitForChar(0L, FALSE);
}
#if defined(FEAT_TERMINAL) || defined(PROTO)
int
mch_check_messages(void)
{
#if defined(VIMDLL)
if (gui.in_use)
return TRUE;
#endif
return WaitForChar(0L, TRUE);
}
#endif
static void
create_conin(void)
{
g_hConIn = CreateFile("CONIN$", GENERIC_READ|GENERIC_WRITE,
FILE_SHARE_READ|FILE_SHARE_WRITE,
(LPSECURITY_ATTRIBUTES) NULL,
OPEN_EXISTING, 0, (HANDLE)NULL);
did_create_conin = TRUE;
}
static WCHAR
tgetch(int *pmodifiers, WCHAR *pch2)
{
WCHAR ch;
for (;;)
{
INPUT_RECORD ir;
DWORD cRecords = 0;
#if defined(FEAT_CLIENTSERVER)
(void)WaitForChar(-1L, FALSE);
if (input_available())
return 0;
if (g_nMouseClick != -1)
return 0;
#endif
if (read_console_input(g_hConIn, &ir, 1, &cRecords) == 0)
{
if (did_create_conin)
read_error_exit();
create_conin();
continue;
}
if (ir.EventType == KEY_EVENT)
{
if (decode_key_event(&ir.Event.KeyEvent, &ch, pch2,
pmodifiers, TRUE))
return ch;
}
else if (ir.EventType == FOCUS_EVENT)
handle_focus_event(ir);
else if (ir.EventType == WINDOW_BUFFER_SIZE_EVENT)
shell_resized();
else if (ir.EventType == MOUSE_EVENT)
{
if (decode_mouse_event(&ir.Event.MouseEvent))
return 0;
}
}
}
#endif 
int
mch_inchar(
char_u *buf UNUSED,
int maxlen UNUSED,
long time UNUSED,
int tb_change_cnt UNUSED)
{
#if !defined(FEAT_GUI_MSWIN) || defined(VIMDLL)
int len;
int c;
#if defined(VIMDLL)
#define TYPEAHEADSPACE 6
#else
#define TYPEAHEADSPACE 0
#endif
#define TYPEAHEADLEN (20 + TYPEAHEADSPACE)
static char_u typeahead[TYPEAHEADLEN]; 
static int typeaheadlen = 0;
#if defined(VIMDLL)
if (gui.in_use)
return 0;
#endif
if (typeaheadlen > 0)
goto theend;
if (time >= 0)
{
if (!WaitForChar(time, FALSE)) 
return 0;
}
else 
{
mch_set_winsize_now(); 
if (!WaitForChar(p_ut, FALSE))
{
if (trigger_cursorhold() && maxlen >= 3)
{
buf[0] = K_SPECIAL;
buf[1] = KS_EXTRA;
buf[2] = (int)KE_CURSORHOLD;
return 3;
}
before_blocking();
}
}
g_fCBrkPressed = FALSE;
#if defined(MCH_WRITE_DUMP)
if (fdDump)
fputc('[', fdDump);
#endif
while ((typeaheadlen == 0 || WaitForChar(0L, FALSE))
&& typeaheadlen + 5 + TYPEAHEADSPACE <= TYPEAHEADLEN)
{
if (typebuf_changed(tb_change_cnt))
{
typeaheadlen = 0;
break;
}
if (g_nMouseClick != -1)
{
#if defined(MCH_WRITE_DUMP)
if (fdDump)
fprintf(fdDump, "{%02x @ %d, %d}",
g_nMouseClick, g_xMouse, g_yMouse);
#endif
typeahead[typeaheadlen++] = ESC + 128;
typeahead[typeaheadlen++] = 'M';
typeahead[typeaheadlen++] = g_nMouseClick;
typeahead[typeaheadlen++] = g_xMouse + '!';
typeahead[typeaheadlen++] = g_yMouse + '!';
g_nMouseClick = -1;
}
else
{
WCHAR ch2 = NUL;
int modifiers = 0;
c = tgetch(&modifiers, &ch2);
if (typebuf_changed(tb_change_cnt))
{
typeaheadlen = 0;
break;
}
if (c == Ctrl_C && ctrl_c_interrupts)
{
#if defined(FEAT_CLIENTSERVER)
trash_input_buf();
#endif
got_int = TRUE;
}
if (g_nMouseClick == -1)
{
int n = 1;
if (ch2 == NUL)
{
int i, j;
char_u *p;
WCHAR ch[2];
ch[0] = c;
if (c >= 0xD800 && c <= 0xDBFF) 
{
ch[1] = tgetch(&modifiers, &ch2);
n++;
}
p = utf16_to_enc(ch, &n);
if (p != NULL)
{
for (i = 0, j = 0; i < n; i++)
{
typeahead[typeaheadlen + j++] = p[i];
#if defined(VIMDLL)
if (p[i] == CSI)
{
typeahead[typeaheadlen + j++] = KS_EXTRA;
typeahead[typeaheadlen + j++] = KE_CSI;
}
#endif
}
n = j;
vim_free(p);
}
}
else
{
typeahead[typeaheadlen] = c;
#if defined(VIMDLL)
if (c == CSI)
{
typeahead[typeaheadlen + 1] = KS_EXTRA;
typeahead[typeaheadlen + 2] = KE_CSI;
n = 3;
}
#endif
}
if (ch2 != NUL)
{
if (c == K_NUL)
{
switch (ch2)
{
case (WCHAR)'\324': 
case (WCHAR)'\325': 
case (WCHAR)'\327': 
case (WCHAR)'\330': 
typeahead[typeaheadlen + n] = (char_u)ch2;
n++;
break;
default:
typeahead[typeaheadlen + n] = 3;
typeahead[typeaheadlen + n + 1] = (char_u)ch2;
n += 2;
break;
}
}
else
{
typeahead[typeaheadlen + n] = 3;
typeahead[typeaheadlen + n + 1] = (char_u)ch2;
n += 2;
}
}
if ((modifiers & MOD_MASK_ALT)
&& n == 1
&& (typeahead[typeaheadlen] & 0x80) == 0
&& !enc_dbcs
)
{
n = (*mb_char2bytes)(typeahead[typeaheadlen] | 0x80,
typeahead + typeaheadlen);
modifiers &= ~MOD_MASK_ALT;
}
if (modifiers != 0)
{
mch_memmove(typeahead + typeaheadlen + 3,
typeahead + typeaheadlen, n);
typeahead[typeaheadlen++] = K_SPECIAL;
typeahead[typeaheadlen++] = (char_u)KS_MODIFIER;
typeahead[typeaheadlen++] = modifiers;
}
typeaheadlen += n;
#if defined(MCH_WRITE_DUMP)
if (fdDump)
fputc(c, fdDump);
#endif
}
}
}
#if defined(MCH_WRITE_DUMP)
if (fdDump)
{
fputs("]\n", fdDump);
fflush(fdDump);
}
#endif
theend:
len = 0;
while (len < maxlen && typeaheadlen > 0)
{
buf[len++] = typeahead[0];
mch_memmove(typeahead, typeahead + 1, --typeaheadlen);
}
return len;
#else 
return 0;
#endif 
}
#if !defined(PROTO)
#if !defined(__MINGW32__)
#include <shellapi.h> 
#endif
#endif
static int
executable_exists(char *name, char_u **path, int use_path)
{
WCHAR *p;
WCHAR fnamew[_MAX_PATH];
WCHAR *dumw;
WCHAR *wcurpath, *wnewpath;
long n;
if (!use_path)
{
if (mch_getperm((char_u *)name) != -1 && !mch_isdir((char_u *)name))
{
if (path != NULL)
{
if (mch_isFullName((char_u *)name))
*path = vim_strsave((char_u *)name);
else
*path = FullName_save((char_u *)name, FALSE);
}
return TRUE;
}
return FALSE;
}
p = enc_to_utf16((char_u *)name, NULL);
if (p == NULL)
return FALSE;
wcurpath = _wgetenv(L"PATH");
wnewpath = ALLOC_MULT(WCHAR, wcslen(wcurpath) + 3);
if (wnewpath == NULL)
return FALSE;
wcscpy(wnewpath, L".;");
wcscat(wnewpath, wcurpath);
n = (long)SearchPathW(wnewpath, p, NULL, _MAX_PATH, fnamew, &dumw);
vim_free(wnewpath);
vim_free(p);
if (n == 0)
return FALSE;
if (GetFileAttributesW(fnamew) & FILE_ATTRIBUTE_DIRECTORY)
return FALSE;
if (path != NULL)
*path = utf16_to_enc(fnamew, NULL);
return TRUE;
}
#if (defined(__MINGW32__) && __MSVCRT_VERSION__ >= 0x800) || (defined(_MSC_VER) && _MSC_VER >= 1400)
static void
bad_param_handler(const wchar_t *expression,
const wchar_t *function,
const wchar_t *file,
unsigned int line,
uintptr_t pReserved)
{
}
#define SET_INVALID_PARAM_HANDLER ((void)_set_invalid_parameter_handler(bad_param_handler))
#else
#define SET_INVALID_PARAM_HANDLER
#endif
#if defined(FEAT_GUI_MSWIN)
static void
mch_init_g(void)
{
#if !defined(__MINGW32__)
extern int _fmode;
#endif
SET_INVALID_PARAM_HANDLER;
SetErrorMode(SEM_FAILCRITICALERRORS);
_fmode = O_BINARY; 
Rows = 25;
Columns = 80;
{
char_u vimrun_location[_MAX_PATH + 4];
STRCPY(vimrun_location, exe_name);
STRCPY(gettail(vimrun_location), "vimrun.exe");
if (mch_getperm(vimrun_location) >= 0)
{
if (*skiptowhite(vimrun_location) != NUL)
{
mch_memmove(vimrun_location + 1, vimrun_location,
STRLEN(vimrun_location) + 1);
*vimrun_location = '"';
STRCPY(gettail(vimrun_location), "vimrun\" ");
}
else
STRCPY(gettail(vimrun_location), "vimrun ");
vimrun_path = (char *)vim_strsave(vimrun_location);
s_dont_use_vimrun = FALSE;
}
else if (executable_exists("vimrun.exe", NULL, TRUE))
s_dont_use_vimrun = FALSE;
if (s_dont_use_vimrun)
need_vimrun_warning = TRUE;
}
if (!executable_exists("findstr.exe", NULL, TRUE))
set_option_value((char_u *)"grepprg", 0, (char_u *)"grep -n", 0);
#if defined(FEAT_CLIPBOARD)
win_clip_init();
#endif
vtp_flag_init();
}
#endif 
#if !defined(FEAT_GUI_MSWIN) || defined(VIMDLL)
#define SRWIDTH(sr) ((sr).Right - (sr).Left + 1)
#define SRHEIGHT(sr) ((sr).Bottom - (sr).Top + 1)
static BOOL
ClearConsoleBuffer(WORD wAttribute)
{
CONSOLE_SCREEN_BUFFER_INFO csbi;
COORD coord;
DWORD NumCells, dummy;
if (!GetConsoleScreenBufferInfo(g_hConOut, &csbi))
return FALSE;
NumCells = csbi.dwSize.X * csbi.dwSize.Y;
coord.X = 0;
coord.Y = 0;
if (!FillConsoleOutputCharacter(g_hConOut, ' ', NumCells,
coord, &dummy))
return FALSE;
if (!FillConsoleOutputAttribute(g_hConOut, wAttribute, NumCells,
coord, &dummy))
return FALSE;
return TRUE;
}
static BOOL
FitConsoleWindow(
COORD dwBufferSize,
BOOL WantAdjust)
{
CONSOLE_SCREEN_BUFFER_INFO csbi;
COORD dwWindowSize;
BOOL NeedAdjust = FALSE;
if (GetConsoleScreenBufferInfo(g_hConOut, &csbi))
{
if (csbi.srWindow.Right >= dwBufferSize.X)
{
dwWindowSize.X = SRWIDTH(csbi.srWindow);
if (dwWindowSize.X > dwBufferSize.X)
dwWindowSize.X = dwBufferSize.X;
csbi.srWindow.Right = dwBufferSize.X - 1;
csbi.srWindow.Left = dwBufferSize.X - dwWindowSize.X;
NeedAdjust = TRUE;
}
if (csbi.srWindow.Bottom >= dwBufferSize.Y)
{
dwWindowSize.Y = SRHEIGHT(csbi.srWindow);
if (dwWindowSize.Y > dwBufferSize.Y)
dwWindowSize.Y = dwBufferSize.Y;
csbi.srWindow.Bottom = dwBufferSize.Y - 1;
csbi.srWindow.Top = dwBufferSize.Y - dwWindowSize.Y;
NeedAdjust = TRUE;
}
if (NeedAdjust && WantAdjust)
{
if (!SetConsoleWindowInfo(g_hConOut, TRUE, &csbi.srWindow))
return FALSE;
}
return TRUE;
}
return FALSE;
}
typedef struct ConsoleBufferStruct
{
BOOL IsValid;
CONSOLE_SCREEN_BUFFER_INFO Info;
PCHAR_INFO Buffer;
COORD BufferSize;
PSMALL_RECT Regions;
int NumRegions;
} ConsoleBuffer;
static BOOL
SaveConsoleBuffer(
ConsoleBuffer *cb)
{
DWORD NumCells;
COORD BufferCoord;
SMALL_RECT ReadRegion;
WORD Y, Y_incr;
int i, numregions;
if (cb == NULL)
return FALSE;
if (!GetConsoleScreenBufferInfo(g_hConOut, &cb->Info))
{
cb->IsValid = FALSE;
return FALSE;
}
cb->IsValid = TRUE;
if (!cb->IsValid || cb->Buffer == NULL ||
cb->BufferSize.X != cb->Info.dwSize.X ||
cb->BufferSize.Y != cb->Info.dwSize.Y)
{
cb->BufferSize.X = cb->Info.dwSize.X;
cb->BufferSize.Y = cb->Info.dwSize.Y;
NumCells = cb->BufferSize.X * cb->BufferSize.Y;
vim_free(cb->Buffer);
cb->Buffer = ALLOC_MULT(CHAR_INFO, NumCells);
if (cb->Buffer == NULL)
return FALSE;
}
BufferCoord.X = 0;
ReadRegion.Left = 0;
ReadRegion.Right = cb->Info.dwSize.X - 1;
Y_incr = 12000 / cb->Info.dwSize.X;
numregions = (cb->Info.dwSize.Y + Y_incr - 1) / Y_incr;
if (cb->Regions == NULL || numregions != cb->NumRegions)
{
cb->NumRegions = numregions;
vim_free(cb->Regions);
cb->Regions = ALLOC_MULT(SMALL_RECT, cb->NumRegions);
if (cb->Regions == NULL)
{
VIM_CLEAR(cb->Buffer);
return FALSE;
}
}
for (i = 0, Y = 0; i < cb->NumRegions; i++, Y += Y_incr)
{
BufferCoord.Y = Y;
ReadRegion.Top = Y;
ReadRegion.Bottom = Y + Y_incr - 1;
if (!ReadConsoleOutputW(g_hConOut, 
cb->Buffer, 
cb->BufferSize, 
BufferCoord, 
&ReadRegion)) 
{
VIM_CLEAR(cb->Buffer);
VIM_CLEAR(cb->Regions);
return FALSE;
}
cb->Regions[i] = ReadRegion;
}
return TRUE;
}
static BOOL
RestoreConsoleBuffer(
ConsoleBuffer *cb,
BOOL RestoreScreen)
{
COORD BufferCoord;
SMALL_RECT WriteRegion;
int i;
if (cb == NULL || !cb->IsValid)
return FALSE;
if (RestoreScreen)
ClearConsoleBuffer(cb->Info.wAttributes);
FitConsoleWindow(cb->Info.dwSize, TRUE);
if (!SetConsoleScreenBufferSize(g_hConOut, cb->Info.dwSize))
return FALSE;
if (!SetConsoleTextAttribute(g_hConOut, cb->Info.wAttributes))
return FALSE;
if (!RestoreScreen)
{
return TRUE;
}
if (!SetConsoleCursorPosition(g_hConOut, cb->Info.dwCursorPosition))
return FALSE;
if (!SetConsoleWindowInfo(g_hConOut, TRUE, &cb->Info.srWindow))
return FALSE;
if (cb->Buffer != NULL)
{
for (i = 0; i < cb->NumRegions; i++)
{
BufferCoord.X = cb->Regions[i].Left;
BufferCoord.Y = cb->Regions[i].Top;
WriteRegion = cb->Regions[i];
if (!WriteConsoleOutputW(g_hConOut, 
cb->Buffer, 
cb->BufferSize, 
BufferCoord, 
&WriteRegion)) 
return FALSE;
}
}
return TRUE;
}
#define FEAT_RESTORE_ORIG_SCREEN
#if defined(FEAT_RESTORE_ORIG_SCREEN)
static ConsoleBuffer g_cbOrig = { 0 };
#endif
static ConsoleBuffer g_cbNonTermcap = { 0 };
static ConsoleBuffer g_cbTermcap = { 0 };
#if defined(FEAT_TITLE)
char g_szOrigTitle[256] = { 0 };
HWND g_hWnd = NULL; 
static HICON g_hOrigIconSmall = NULL;
static HICON g_hOrigIcon = NULL;
static HICON g_hVimIcon = NULL;
static BOOL g_fCanChangeIcon = FALSE;
#if !defined(ICON_SMALL)
#define ICON_SMALL 0
#endif
#if !defined(ICON_BIG)
#define ICON_BIG 1
#endif
static BOOL
GetConsoleIcon(
HWND hWnd,
HICON *phIconSmall,
HICON *phIcon)
{
if (hWnd == NULL)
return FALSE;
if (phIconSmall != NULL)
*phIconSmall = (HICON)SendMessage(hWnd, WM_GETICON,
(WPARAM)ICON_SMALL, (LPARAM)0);
if (phIcon != NULL)
*phIcon = (HICON)SendMessage(hWnd, WM_GETICON,
(WPARAM)ICON_BIG, (LPARAM)0);
return TRUE;
}
static BOOL
SetConsoleIcon(
HWND hWnd,
HICON hIconSmall,
HICON hIcon)
{
if (hWnd == NULL)
return FALSE;
if (hIconSmall != NULL)
SendMessage(hWnd, WM_SETICON,
(WPARAM)ICON_SMALL, (LPARAM)hIconSmall);
if (hIcon != NULL)
SendMessage(hWnd, WM_SETICON,
(WPARAM)ICON_BIG, (LPARAM) hIcon);
return TRUE;
}
static void
SaveConsoleTitleAndIcon(void)
{
if (!GetConsoleTitle(g_szOrigTitle, sizeof(g_szOrigTitle)))
return;
g_hWnd = GetConsoleWindow();
if (g_hWnd == NULL)
return;
GetConsoleIcon(g_hWnd, &g_hOrigIconSmall, &g_hOrigIcon);
if (g_hOrigIconSmall == NULL || g_hOrigIcon == NULL)
return;
if (mch_icon_load((HANDLE *)&g_hVimIcon) == FAIL || g_hVimIcon == NULL)
g_hVimIcon = ExtractIcon(NULL, (LPCSTR)exe_name, 0);
if (g_hVimIcon != NULL)
g_fCanChangeIcon = TRUE;
}
#endif
static int g_fWindInitCalled = FALSE;
static int g_fTermcapMode = FALSE;
static CONSOLE_CURSOR_INFO g_cci;
static DWORD g_cmodein = 0;
static DWORD g_cmodeout = 0;
static void
mch_init_c(void)
{
#if !defined(FEAT_RESTORE_ORIG_SCREEN)
CONSOLE_SCREEN_BUFFER_INFO csbi;
#endif
#if !defined(__MINGW32__)
extern int _fmode;
#endif
SET_INVALID_PARAM_HANDLER;
SetErrorMode(SEM_FAILCRITICALERRORS);
_fmode = O_BINARY; 
out_flush();
if (read_cmd_fd == 0)
g_hConIn = GetStdHandle(STD_INPUT_HANDLE);
else
create_conin();
g_hConOut = GetStdHandle(STD_OUTPUT_HANDLE);
#if defined(FEAT_RESTORE_ORIG_SCREEN)
SaveConsoleBuffer(&g_cbOrig);
g_attrCurrent = g_attrDefault = g_cbOrig.Info.wAttributes;
#else
GetConsoleScreenBufferInfo(g_hConOut, &csbi);
g_attrCurrent = g_attrDefault = csbi.wAttributes;
#endif
if (cterm_normal_fg_color == 0)
cterm_normal_fg_color = (g_attrCurrent & 0xf) + 1;
if (cterm_normal_bg_color == 0)
cterm_normal_bg_color = ((g_attrCurrent >> 4) & 0xf) + 1;
g_color_index_fg = g_attrDefault & 0xf;
g_color_index_bg = (g_attrDefault >> 4) & 0xf;
update_tcap(g_attrCurrent);
GetConsoleCursorInfo(g_hConOut, &g_cci);
GetConsoleMode(g_hConIn, &g_cmodein);
GetConsoleMode(g_hConOut, &g_cmodeout);
#if defined(FEAT_TITLE)
SaveConsoleTitleAndIcon();
if (g_fCanChangeIcon)
SetConsoleIcon(g_hWnd, g_hVimIcon, g_hVimIcon);
#endif
ui_get_shellsize();
#if defined(MCH_WRITE_DUMP)
fdDump = fopen("dump", "wt");
if (fdDump)
{
time_t t;
time(&t);
fputs(ctime(&t), fdDump);
fflush(fdDump);
}
#endif
g_fWindInitCalled = TRUE;
g_fMouseAvail = GetSystemMetrics(SM_MOUSEPRESENT);
#if defined(FEAT_CLIPBOARD)
win_clip_init();
#endif
vtp_flag_init();
vtp_init();
}
static void
mch_exit_c(int r)
{
exiting = TRUE;
vtp_exit();
stoptermcap();
if (g_fWindInitCalled)
settmode(TMODE_COOK);
ml_close_all(TRUE); 
if (g_fWindInitCalled)
{
#if defined(FEAT_TITLE)
mch_restore_title(SAVE_RESTORE_BOTH);
if (g_fCanChangeIcon && !g_fForceExit)
SetConsoleIcon(g_hWnd, g_hOrigIconSmall, g_hOrigIcon);
#endif
#if defined(MCH_WRITE_DUMP)
if (fdDump)
{
time_t t;
time(&t);
fputs(ctime(&t), fdDump);
fclose(fdDump);
}
fdDump = NULL;
#endif
}
SetConsoleCursorInfo(g_hConOut, &g_cci);
SetConsoleMode(g_hConIn, g_cmodein);
SetConsoleMode(g_hConOut, g_cmodeout);
#if defined(DYNAMIC_GETTEXT)
dyn_libintl_end();
#endif
exit(r);
}
#endif 
void
mch_init(void)
{
#if defined(VIMDLL)
if (gui.starting)
mch_init_g();
else
mch_init_c();
#elif defined(FEAT_GUI_MSWIN)
mch_init_g();
#else
mch_init_c();
#endif
}
void
mch_exit(int r)
{
#if defined(VIMDLL)
if (gui.in_use || gui.starting)
mch_exit_g(r);
else
mch_exit_c(r);
#elif defined(FEAT_GUI_MSWIN)
mch_exit_g(r);
#else
mch_exit_c(r);
#endif
}
int
mch_check_win(
int argc UNUSED,
char **argv UNUSED)
{
get_exe_name();
#if defined(FEAT_GUI_MSWIN) && !defined(VIMDLL)
return OK; 
#else
#if defined(VIMDLL)
if (gui.in_use)
return OK;
#endif
if (isatty(1))
return OK;
return FAIL;
#endif
}
void
fname_case(
char_u *name,
int len)
{
int flen;
WCHAR *p;
WCHAR buf[_MAX_PATH + 1];
flen = (int)STRLEN(name);
if (flen == 0)
return;
slash_adjust(name);
p = enc_to_utf16(name, NULL);
if (p == NULL)
return;
if (GetLongPathNameW(p, buf, _MAX_PATH))
{
char_u *q = utf16_to_enc(buf, NULL);
if (q != NULL)
{
if (len > 0 || flen >= (int)STRLEN(q))
vim_strncpy(name, q, (len > 0) ? len - 1 : flen);
vim_free(q);
}
}
vim_free(p);
}
int
mch_get_user_name(
char_u *s,
int len)
{
WCHAR wszUserName[256 + 1]; 
DWORD wcch = sizeof(wszUserName) / sizeof(WCHAR);
if (GetUserNameW(wszUserName, &wcch))
{
char_u *p = utf16_to_enc(wszUserName, NULL);
if (p != NULL)
{
vim_strncpy(s, p, len - 1);
vim_free(p);
return OK;
}
}
s[0] = NUL;
return FAIL;
}
void
mch_get_host_name(
char_u *s,
int len)
{
WCHAR wszHostName[256 + 1];
DWORD wcch = sizeof(wszHostName) / sizeof(WCHAR);
if (GetComputerNameW(wszHostName, &wcch))
{
char_u *p = utf16_to_enc(wszHostName, NULL);
if (p != NULL)
{
vim_strncpy(s, p, len - 1);
vim_free(p);
return;
}
}
}
long
mch_get_pid(void)
{
return (long)GetCurrentProcessId();
}
int
mch_process_running(long pid)
{
HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, 0, (DWORD)pid);
DWORD status = 0;
int ret = FALSE;
if (hProcess == NULL)
return FALSE; 
if (GetExitCodeProcess(hProcess, &status) )
ret = status == STILL_ACTIVE;
CloseHandle(hProcess);
return ret;
}
int
mch_dirname(
char_u *buf,
int len)
{
WCHAR wbuf[_MAX_PATH + 1];
if (GetCurrentDirectoryW(_MAX_PATH, wbuf) != 0)
{
WCHAR wcbuf[_MAX_PATH + 1];
char_u *p = NULL;
if (GetLongPathNameW(wbuf, wcbuf, _MAX_PATH) != 0)
{
p = utf16_to_enc(wcbuf, NULL);
if (STRLEN(p) >= (size_t)len)
{
vim_free(p);
p = NULL;
}
}
if (p == NULL)
p = utf16_to_enc(wbuf, NULL);
if (p != NULL)
{
vim_strncpy(buf, p, len - 1);
vim_free(p);
return OK;
}
}
return FAIL;
}
long
mch_getperm(char_u *name)
{
stat_T st;
int n;
n = mch_stat((char *)name, &st);
return n == 0 ? (long)(unsigned short)st.st_mode : -1L;
}
int
mch_setperm(char_u *name, long perm)
{
long n;
WCHAR *p;
p = enc_to_utf16(name, NULL);
if (p == NULL)
return FAIL;
n = _wchmod(p, perm);
vim_free(p);
if (n == -1)
return FAIL;
win32_set_archive(name);
return OK;
}
void
mch_hide(char_u *name)
{
int attrs = win32_getattrs(name);
if (attrs == -1)
return;
attrs |= FILE_ATTRIBUTE_HIDDEN;
win32_setattrs(name, attrs);
}
int
mch_ishidden(char_u *name)
{
int f = win32_getattrs(name);
if (f == -1)
return FALSE; 
return (f & FILE_ATTRIBUTE_HIDDEN) != 0;
}
int
mch_isdir(char_u *name)
{
int f = win32_getattrs(name);
if (f == -1)
return FALSE; 
return (f & FILE_ATTRIBUTE_DIRECTORY) != 0;
}
int
mch_isrealdir(char_u *name)
{
return mch_isdir(name) && !mch_is_symbolic_link(name);
}
int
mch_mkdir(char_u *name)
{
WCHAR *p;
int retval;
p = enc_to_utf16(name, NULL);
if (p == NULL)
return -1;
retval = _wmkdir(p);
vim_free(p);
return retval;
}
int
mch_rmdir(char_u *name)
{
WCHAR *p;
int retval;
p = enc_to_utf16(name, NULL);
if (p == NULL)
return -1;
retval = _wrmdir(p);
vim_free(p);
return retval;
}
int
mch_is_hard_link(char_u *fname)
{
BY_HANDLE_FILE_INFORMATION info;
return win32_fileinfo(fname, &info) == FILEINFO_OK
&& info.nNumberOfLinks > 1;
}
int
mch_is_symbolic_link(char_u *name)
{
HANDLE hFind;
int res = FALSE;
DWORD fileFlags = 0, reparseTag = 0;
WCHAR *wn;
WIN32_FIND_DATAW findDataW;
wn = enc_to_utf16(name, NULL);
if (wn == NULL)
return FALSE;
hFind = FindFirstFileW(wn, &findDataW);
vim_free(wn);
if (hFind != INVALID_HANDLE_VALUE)
{
fileFlags = findDataW.dwFileAttributes;
reparseTag = findDataW.dwReserved0;
FindClose(hFind);
}
if ((fileFlags & FILE_ATTRIBUTE_REPARSE_POINT)
&& (reparseTag == IO_REPARSE_TAG_SYMLINK
|| reparseTag == IO_REPARSE_TAG_MOUNT_POINT))
res = TRUE;
return res;
}
int
mch_is_linked(char_u *fname)
{
if (mch_is_hard_link(fname) || mch_is_symbolic_link(fname))
return TRUE;
return FALSE;
}
int
win32_fileinfo(char_u *fname, BY_HANDLE_FILE_INFORMATION *info)
{
HANDLE hFile;
int res = FILEINFO_READ_FAIL;
WCHAR *wn;
wn = enc_to_utf16(fname, NULL);
if (wn == NULL)
return FILEINFO_ENC_FAIL;
hFile = CreateFileW(wn, 
GENERIC_READ, 
FILE_SHARE_READ | FILE_SHARE_WRITE, 
NULL, 
OPEN_EXISTING, 
FILE_FLAG_BACKUP_SEMANTICS, 
NULL); 
vim_free(wn);
if (hFile != INVALID_HANDLE_VALUE)
{
if (GetFileInformationByHandle(hFile, info) != 0)
res = FILEINFO_OK;
else
res = FILEINFO_INFO_FAIL;
CloseHandle(hFile);
}
return res;
}
static int
win32_getattrs(char_u *name)
{
int attr;
WCHAR *p;
p = enc_to_utf16(name, NULL);
if (p == NULL)
return INVALID_FILE_ATTRIBUTES;
attr = GetFileAttributesW(p);
vim_free(p);
return attr;
}
static int
win32_setattrs(char_u *name, int attrs)
{
int res;
WCHAR *p;
p = enc_to_utf16(name, NULL);
if (p == NULL)
return -1;
res = SetFileAttributesW(p, attrs);
vim_free(p);
return res ? 0 : -1;
}
static int
win32_set_archive(char_u *name)
{
int attrs = win32_getattrs(name);
if (attrs == -1)
return -1;
attrs |= FILE_ATTRIBUTE_ARCHIVE;
return win32_setattrs(name, attrs);
}
int
mch_writable(char_u *name)
{
int attrs = win32_getattrs(name);
return (attrs != -1 && (!(attrs & FILE_ATTRIBUTE_READONLY)
|| (attrs & FILE_ATTRIBUTE_DIRECTORY)));
}
int
mch_can_exe(char_u *name, char_u **path, int use_path)
{
char_u buf[_MAX_PATH * 3];
int len = (int)STRLEN(name);
char_u *p, *saved;
if (len >= sizeof(buf)) 
return FALSE;
if (strstr((char *)gettail(p_sh), "sh") != NULL)
if (executable_exists((char *)name, path, use_path))
return TRUE;
p = mch_getenv("PATHEXT");
if (p == NULL)
p = (char_u *)".com;.exe;.bat;.cmd";
saved = vim_strsave(p);
if (saved == NULL)
return FALSE;
p = saved;
while (*p)
{
char_u *tmp = vim_strchr(p, ';');
if (tmp != NULL)
*tmp = NUL;
if (_stricoll((char *)name + len - STRLEN(p), (char *)p) == 0
&& executable_exists((char *)name, path, use_path))
{
vim_free(saved);
return TRUE;
}
if (tmp == NULL)
break;
p = tmp + 1;
}
vim_free(saved);
vim_strncpy(buf, name, sizeof(buf) - 1);
p = mch_getenv("PATHEXT");
if (p == NULL)
p = (char_u *)".com;.exe;.bat;.cmd";
while (*p)
{
if (p[0] == '.' && (p[1] == NUL || p[1] == ';'))
{
buf[len] = NUL;
++p;
if (*p)
++p;
}
else
copy_option_part(&p, buf + len, sizeof(buf) - len, ";");
if (executable_exists((char *)buf, path, use_path))
return TRUE;
}
return FALSE;
}
int
mch_nodetype(char_u *name)
{
HANDLE hFile;
int type;
WCHAR *wn;
if (STRNCMP(name, "\\\\.\\", 4) == 0)
return NODE_WRITABLE;
wn = enc_to_utf16(name, NULL);
if (wn == NULL)
return NODE_NORMAL;
hFile = CreateFileW(wn, 
GENERIC_WRITE, 
0, 
NULL, 
OPEN_EXISTING, 
0, 
NULL); 
vim_free(wn);
if (hFile == INVALID_HANDLE_VALUE)
return NODE_NORMAL;
type = GetFileType(hFile);
CloseHandle(hFile);
if (type == FILE_TYPE_CHAR)
return NODE_WRITABLE;
if (type == FILE_TYPE_DISK)
return NODE_NORMAL;
return NODE_OTHER;
}
#if defined(HAVE_ACL)
struct my_acl
{
PSECURITY_DESCRIPTOR pSecurityDescriptor;
PSID pSidOwner;
PSID pSidGroup;
PACL pDacl;
PACL pSacl;
};
#endif
vim_acl_T
mch_get_acl(char_u *fname)
{
#if !defined(HAVE_ACL)
return (vim_acl_T)NULL;
#else
struct my_acl *p = NULL;
DWORD err;
p = ALLOC_CLEAR_ONE(struct my_acl);
if (p != NULL)
{
WCHAR *wn;
wn = enc_to_utf16(fname, NULL);
if (wn == NULL)
return NULL;
err = GetNamedSecurityInfoW(
wn, 
SE_FILE_OBJECT, 
OWNER_SECURITY_INFORMATION |
GROUP_SECURITY_INFORMATION |
DACL_SECURITY_INFORMATION |
SACL_SECURITY_INFORMATION,
&p->pSidOwner, 
&p->pSidGroup, 
&p->pDacl, 
&p->pSacl, 
&p->pSecurityDescriptor);
if (err == ERROR_ACCESS_DENIED ||
err == ERROR_PRIVILEGE_NOT_HELD)
{
(void)GetNamedSecurityInfoW(
wn,
SE_FILE_OBJECT,
DACL_SECURITY_INFORMATION,
NULL,
NULL,
&p->pDacl,
NULL,
&p->pSecurityDescriptor);
}
if (p->pSecurityDescriptor == NULL)
{
mch_free_acl((vim_acl_T)p);
p = NULL;
}
vim_free(wn);
}
return (vim_acl_T)p;
#endif
}
#if defined(HAVE_ACL)
static BOOL
is_acl_inherited(PACL acl)
{
DWORD i;
ACL_SIZE_INFORMATION acl_info;
PACCESS_ALLOWED_ACE ace;
acl_info.AceCount = 0;
GetAclInformation(acl, &acl_info, sizeof(acl_info), AclSizeInformation);
for (i = 0; i < acl_info.AceCount; i++)
{
GetAce(acl, i, (LPVOID *)&ace);
if (ace->Header.AceFlags & INHERITED_ACE)
return TRUE;
}
return FALSE;
}
#endif
void
mch_set_acl(char_u *fname, vim_acl_T acl)
{
#if defined(HAVE_ACL)
struct my_acl *p = (struct my_acl *)acl;
SECURITY_INFORMATION sec_info = 0;
WCHAR *wn;
if (p == NULL)
return;
wn = enc_to_utf16(fname, NULL);
if (wn == NULL)
return;
if (p->pSidOwner)
sec_info |= OWNER_SECURITY_INFORMATION;
if (p->pSidGroup)
sec_info |= GROUP_SECURITY_INFORMATION;
if (p->pDacl)
{
sec_info |= DACL_SECURITY_INFORMATION;
if (!is_acl_inherited(p->pDacl))
sec_info |= PROTECTED_DACL_SECURITY_INFORMATION;
}
if (p->pSacl)
sec_info |= SACL_SECURITY_INFORMATION;
(void)SetNamedSecurityInfoW(
wn, 
SE_FILE_OBJECT, 
sec_info,
p->pSidOwner, 
p->pSidGroup, 
p->pDacl, 
p->pSacl 
);
vim_free(wn);
#endif
}
void
mch_free_acl(vim_acl_T acl)
{
#if defined(HAVE_ACL)
struct my_acl *p = (struct my_acl *)acl;
if (p != NULL)
{
LocalFree(p->pSecurityDescriptor); 
vim_free(p);
}
#endif
}
#if !defined(FEAT_GUI_MSWIN) || defined(VIMDLL)
static BOOL WINAPI
handler_routine(
DWORD dwCtrlType)
{
INPUT_RECORD ir;
DWORD out;
switch (dwCtrlType)
{
case CTRL_C_EVENT:
if (ctrl_c_interrupts)
g_fCtrlCPressed = TRUE;
return TRUE;
case CTRL_BREAK_EVENT:
g_fCBrkPressed = TRUE;
ctrl_break_was_pressed = TRUE;
ir.EventType = KEY_EVENT;
ir.Event.KeyEvent.bKeyDown = TRUE;
ir.Event.KeyEvent.wRepeatCount = 1;
ir.Event.KeyEvent.wVirtualKeyCode = VK_CANCEL;
ir.Event.KeyEvent.wVirtualScanCode = 0;
ir.Event.KeyEvent.dwControlKeyState = 0;
ir.Event.KeyEvent.uChar.UnicodeChar = 0;
WriteConsoleInput(g_hConIn, &ir, 1, &out);
return TRUE;
case CTRL_CLOSE_EVENT:
case CTRL_LOGOFF_EVENT:
case CTRL_SHUTDOWN_EVENT:
windgoto((int)Rows - 1, 0);
g_fForceExit = TRUE;
vim_snprintf((char *)IObuff, IOSIZE, _("Vim: Caught %s event\n"),
(dwCtrlType == CTRL_CLOSE_EVENT
? _("close")
: dwCtrlType == CTRL_LOGOFF_EVENT
? _("logoff")
: _("shutdown")));
#if defined(DEBUG)
OutputDebugString(IObuff);
#endif
preserve_exit(); 
return TRUE; 
default:
return FALSE;
}
}
void
mch_settmode(int tmode)
{
DWORD cmodein;
DWORD cmodeout;
BOOL bEnableHandler;
#if defined(VIMDLL)
if (gui.in_use)
return;
#endif
GetConsoleMode(g_hConIn, &cmodein);
GetConsoleMode(g_hConOut, &cmodeout);
if (tmode == TMODE_RAW)
{
cmodein &= ~(ENABLE_LINE_INPUT | ENABLE_PROCESSED_INPUT |
ENABLE_ECHO_INPUT);
if (g_fMouseActive)
cmodein |= ENABLE_MOUSE_INPUT;
cmodeout &= ~(
#if defined(FEAT_TERMGUICOLORS)
((vtp_working) ? 0 : ENABLE_PROCESSED_OUTPUT) |
#else
ENABLE_PROCESSED_OUTPUT |
#endif
ENABLE_WRAP_AT_EOL_OUTPUT);
bEnableHandler = TRUE;
}
else 
{
cmodein |= (ENABLE_LINE_INPUT | ENABLE_PROCESSED_INPUT |
ENABLE_ECHO_INPUT);
cmodeout |= (ENABLE_PROCESSED_OUTPUT | ENABLE_WRAP_AT_EOL_OUTPUT);
bEnableHandler = FALSE;
}
SetConsoleMode(g_hConIn, cmodein);
SetConsoleMode(g_hConOut, cmodeout);
SetConsoleCtrlHandler(handler_routine, bEnableHandler);
#if defined(MCH_WRITE_DUMP)
if (fdDump)
{
fprintf(fdDump, "mch_settmode(%s, in = %x, out = %x)\n",
tmode == TMODE_RAW ? "raw" :
tmode == TMODE_COOK ? "cooked" : "normal",
cmodein, cmodeout);
fflush(fdDump);
}
#endif
}
int
mch_get_shellsize(void)
{
CONSOLE_SCREEN_BUFFER_INFO csbi;
#if defined(VIMDLL)
if (gui.in_use)
return OK;
#endif
if (!g_fTermcapMode && g_cbTermcap.IsValid)
{
Rows = g_cbTermcap.Info.dwSize.Y;
Columns = g_cbTermcap.Info.dwSize.X;
}
else if (GetConsoleScreenBufferInfo(g_hConOut, &csbi))
{
Rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
Columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;
}
else
{
Rows = 25;
Columns = 80;
}
return OK;
}
static void
ResizeConBuf(
HANDLE hConsole,
COORD coordScreen)
{
if (!SetConsoleScreenBufferSize(hConsole, coordScreen))
{
#if defined(MCH_WRITE_DUMP)
if (fdDump)
{
fprintf(fdDump, "SetConsoleScreenBufferSize failed: %lx\n",
GetLastError());
fflush(fdDump);
}
#endif
}
}
static void
ResizeWindow(
HANDLE hConsole,
SMALL_RECT srWindowRect)
{
if (!SetConsoleWindowInfo(hConsole, TRUE, &srWindowRect))
{
#if defined(MCH_WRITE_DUMP)
if (fdDump)
{
fprintf(fdDump, "SetConsoleWindowInfo failed: %lx\n",
GetLastError());
fflush(fdDump);
}
#endif
}
}
static void
ResizeConBufAndWindow(
HANDLE hConsole,
int xSize,
int ySize)
{
CONSOLE_SCREEN_BUFFER_INFO csbi; 
SMALL_RECT srWindowRect; 
COORD coordScreen;
COORD cursor;
static int resized = FALSE;
#if defined(MCH_WRITE_DUMP)
if (fdDump)
{
fprintf(fdDump, "ResizeConBufAndWindow(%d, %d)\n", xSize, ySize);
fflush(fdDump);
}
#endif
coordScreen = GetLargestConsoleWindowSize(hConsole);
srWindowRect.Left = srWindowRect.Top = (SHORT) 0;
srWindowRect.Right = (SHORT) (min(xSize, coordScreen.X) - 1);
srWindowRect.Bottom = (SHORT) (min(ySize, coordScreen.Y) - 1);
if (GetConsoleScreenBufferInfo(g_hConOut, &csbi))
{
int sx, sy;
sx = csbi.srWindow.Right - csbi.srWindow.Left + 1;
sy = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
if (sy < ySize || sx < xSize)
{
if (sy < ySize)
coordScreen.Y = ySize;
else
coordScreen.Y = sy;
if (sx < xSize)
coordScreen.X = xSize;
else
coordScreen.X = sx;
SetConsoleScreenBufferSize(hConsole, coordScreen);
}
}
coordScreen.X = xSize;
coordScreen.Y = ySize;
if (!vtp_working || resized)
{
ResizeWindow(hConsole, srWindowRect);
ResizeConBuf(hConsole, coordScreen);
}
else
{
cursor.X = srWindowRect.Left;
cursor.Y = srWindowRect.Top;
SetConsoleCursorPosition(hConsole, cursor);
ResizeConBuf(hConsole, coordScreen);
ResizeWindow(hConsole, srWindowRect);
resized = TRUE;
}
}
void
mch_set_shellsize(void)
{
COORD coordScreen;
#if defined(VIMDLL)
if (gui.in_use)
return;
#endif
if (suppress_winsize != 0)
{
suppress_winsize = 2;
return;
}
if (term_console)
{
coordScreen = GetLargestConsoleWindowSize(g_hConOut);
if (Rows > coordScreen.Y)
Rows = coordScreen.Y;
if (Columns > coordScreen.X)
Columns = coordScreen.X;
ResizeConBufAndWindow(g_hConOut, Columns, Rows);
}
}
void
mch_new_shellsize(void)
{
#if defined(VIMDLL)
if (gui.in_use)
return;
#endif
set_scroll_region(0, 0, Columns - 1, Rows - 1);
}
void
mch_set_winsize_now(void)
{
if (suppress_winsize == 2)
{
suppress_winsize = 0;
mch_set_shellsize();
shell_resized();
}
suppress_winsize = 0;
}
#endif 
static BOOL
vim_create_process(
char *cmd,
BOOL inherit_handles,
DWORD flags,
STARTUPINFO *si,
PROCESS_INFORMATION *pi,
LPVOID *env,
char *cwd)
{
BOOL ret = FALSE;
WCHAR *wcmd, *wcwd = NULL;
wcmd = enc_to_utf16((char_u *)cmd, NULL);
if (wcmd == NULL)
return FALSE;
if (cwd != NULL)
{
wcwd = enc_to_utf16((char_u *)cwd, NULL);
if (wcwd == NULL)
goto theend;
}
ret = CreateProcessW(
NULL, 
wcmd, 
NULL, 
NULL, 
inherit_handles, 
flags, 
env, 
wcwd, 
(LPSTARTUPINFOW)si, 
pi); 
theend:
vim_free(wcmd);
vim_free(wcwd);
return ret;
}
static HINSTANCE
vim_shell_execute(
char *cmd,
INT n_show_cmd)
{
HINSTANCE ret;
WCHAR *wcmd;
wcmd = enc_to_utf16((char_u *)cmd, NULL);
if (wcmd == NULL)
return (HINSTANCE) 0;
ret = ShellExecuteW(NULL, NULL, wcmd, NULL, NULL, n_show_cmd);
vim_free(wcmd);
return ret;
}
#if defined(FEAT_GUI_MSWIN) || defined(PROTO)
static int
mch_system_classic(char *cmd, int options)
{
STARTUPINFO si;
PROCESS_INFORMATION pi;
DWORD ret = 0;
HWND hwnd = GetFocus();
si.cb = sizeof(si);
si.lpReserved = NULL;
si.lpDesktop = NULL;
si.lpTitle = NULL;
si.dwFlags = STARTF_USESHOWWINDOW;
if (options & SHELL_DOOUT)
si.wShowWindow = SW_SHOWMINNOACTIVE;
else
si.wShowWindow = SW_SHOWNORMAL;
si.cbReserved2 = 0;
si.lpReserved2 = NULL;
vim_create_process(cmd, FALSE,
CREATE_DEFAULT_ERROR_MODE | CREATE_NEW_CONSOLE,
&si, &pi, NULL, NULL);
{
#if defined(FEAT_GUI)
int delay = 1;
for (;;)
{
MSG msg;
if (pPeekMessage(&msg, (HWND)NULL, 0, 0, PM_REMOVE))
{
TranslateMessage(&msg);
pDispatchMessage(&msg);
delay = 1;
continue;
}
if (WaitForSingleObject(pi.hProcess, delay) != WAIT_TIMEOUT)
break;
if (delay < 50)
delay += 10;
}
#else
WaitForSingleObject(pi.hProcess, INFINITE);
#endif
GetExitCodeProcess(pi.hProcess, &ret);
}
CloseHandle(pi.hThread);
CloseHandle(pi.hProcess);
PostMessage(hwnd, WM_SETFOCUS, 0, 0);
return ret;
}
static unsigned int __stdcall
sub_process_writer(LPVOID param)
{
HANDLE g_hChildStd_IN_Wr = param;
linenr_T lnum = curbuf->b_op_start.lnum;
DWORD len = 0;
DWORD l;
char_u *lp = ml_get(lnum);
char_u *s;
int written = 0;
for (;;)
{
l = (DWORD)STRLEN(lp + written);
if (l == 0)
len = 0;
else if (lp[written] == NL)
{
WriteFile(g_hChildStd_IN_Wr, "", 1, &len, NULL);
}
else
{
s = vim_strchr(lp + written, NL);
WriteFile(g_hChildStd_IN_Wr, (char *)lp + written,
s == NULL ? l : (DWORD)(s - (lp + written)),
&len, NULL);
}
if (len == (int)l)
{
if (lnum != curbuf->b_op_end.lnum
|| (!curbuf->b_p_bin
&& curbuf->b_p_fixeol)
|| (lnum != curbuf->b_no_eol_lnum
&& (lnum != curbuf->b_ml.ml_line_count
|| curbuf->b_p_eol)))
{
WriteFile(g_hChildStd_IN_Wr, "\n", 1,
(LPDWORD)&vim_ignored, NULL);
}
++lnum;
if (lnum > curbuf->b_op_end.lnum)
break;
lp = ml_get(lnum);
written = 0;
}
else if (len > 0)
written += len;
}
CloseHandle(g_hChildStd_IN_Wr);
return 0;
}
#define BUFLEN 100 
static void
dump_pipe(int options,
HANDLE g_hChildStd_OUT_Rd,
garray_T *ga,
char_u buffer[],
DWORD *buffer_off)
{
DWORD availableBytes = 0;
DWORD i;
int ret;
DWORD len;
DWORD toRead;
int repeatCount;
ret = PeekNamedPipe(g_hChildStd_OUT_Rd, 
NULL, 
0, 
NULL, 
&availableBytes, 
NULL); 
repeatCount = 0;
while (ret != 0 && availableBytes > 0)
{
repeatCount++;
toRead = (DWORD)(BUFLEN - *buffer_off);
toRead = availableBytes < toRead ? availableBytes : toRead;
ReadFile(g_hChildStd_OUT_Rd, buffer + *buffer_off, toRead , &len, NULL);
if (len == 0)
break;
availableBytes -= len;
if (options & SHELL_READ)
{
for (i = 0; i < len; ++i)
{
if (buffer[i] == NL)
append_ga_line(ga);
else if (buffer[i] == NUL)
ga_append(ga, NL);
else
ga_append(ga, buffer[i]);
}
}
else if (has_mbyte)
{
int l;
int c;
char_u *p;
len += *buffer_off;
buffer[len] = NUL;
for (p = buffer; p < buffer + len; p += l)
{
l = MB_CPTR2LEN(p);
if (l == 0)
l = 1; 
else if (MB_BYTE2LEN(*p) != l)
break;
}
if (p == buffer) 
{
if (len >= 12)
++p;
else
{
*buffer_off = len;
return;
}
}
c = *p;
*p = NUL;
msg_puts((char *)buffer);
if (p < buffer + len)
{
*p = c;
*buffer_off = (DWORD)((buffer + len) - p);
mch_memmove(buffer, p, *buffer_off);
return;
}
*buffer_off = 0;
}
else
{
buffer[len] = NUL;
msg_puts((char *)buffer);
}
windgoto(msg_row, msg_col);
cursor_on();
out_flush();
}
}
static int
mch_system_piped(char *cmd, int options)
{
STARTUPINFO si;
PROCESS_INFORMATION pi;
DWORD ret = 0;
HANDLE g_hChildStd_IN_Rd = NULL;
HANDLE g_hChildStd_IN_Wr = NULL;
HANDLE g_hChildStd_OUT_Rd = NULL;
HANDLE g_hChildStd_OUT_Wr = NULL;
char_u buffer[BUFLEN + 1]; 
DWORD len;
char_u ta_buf[BUFLEN + 1]; 
int ta_len = 0; 
DWORD i;
int noread_cnt = 0;
garray_T ga;
int delay = 1;
DWORD buffer_off = 0; 
char *p = NULL;
SECURITY_ATTRIBUTES saAttr;
saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
saAttr.bInheritHandle = TRUE;
saAttr.lpSecurityDescriptor = NULL;
if ( ! CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &saAttr, 0)
|| ! SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0)
|| ! CreatePipe(&g_hChildStd_IN_Rd, &g_hChildStd_IN_Wr, &saAttr, 0)
|| ! SetHandleInformation(g_hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0) )
{
CloseHandle(g_hChildStd_IN_Rd);
CloseHandle(g_hChildStd_IN_Wr);
CloseHandle(g_hChildStd_OUT_Rd);
CloseHandle(g_hChildStd_OUT_Wr);
msg_puts(_("\nCannot create pipes\n"));
}
si.cb = sizeof(si);
si.lpReserved = NULL;
si.lpDesktop = NULL;
si.lpTitle = NULL;
si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
si.hStdError = g_hChildStd_OUT_Wr;
si.hStdOutput = g_hChildStd_OUT_Wr;
si.hStdInput = g_hChildStd_IN_Rd;
si.wShowWindow = SW_HIDE;
si.cbReserved2 = 0;
si.lpReserved2 = NULL;
if (options & SHELL_READ)
ga_init2(&ga, 1, BUFLEN);
if (cmd != NULL)
{
p = (char *)vim_strsave((char_u *)cmd);
if (p != NULL)
unescape_shellxquote((char_u *)p, p_sxe);
else
p = cmd;
}
vim_create_process(p, TRUE, CREATE_DEFAULT_ERROR_MODE,
&si, &pi, NULL, NULL);
if (p != cmd)
vim_free(p);
CloseHandle(g_hChildStd_IN_Rd);
CloseHandle(g_hChildStd_OUT_Wr);
if (options & SHELL_WRITE)
{
HANDLE thread = (HANDLE)
_beginthreadex(NULL, 
0, 
sub_process_writer, 
g_hChildStd_IN_Wr, 
0, 
NULL); 
CloseHandle(thread);
g_hChildStd_IN_Wr = NULL;
}
for (;;)
{
MSG msg;
if (pPeekMessage(&msg, (HWND)NULL, 0, 0, PM_REMOVE))
{
TranslateMessage(&msg);
pDispatchMessage(&msg);
}
if ((options & (SHELL_READ|SHELL_WRITE))
#if defined(FEAT_GUI)
|| gui.in_use
#endif
)
{
len = 0;
if (!(options & SHELL_EXPAND)
&& ((options &
(SHELL_READ|SHELL_WRITE|SHELL_COOKED))
!= (SHELL_READ|SHELL_WRITE|SHELL_COOKED)
#if defined(FEAT_GUI)
|| gui.in_use
#endif
)
&& (ta_len > 0 || noread_cnt > 4))
{
if (ta_len == 0)
{
noread_cnt = 0;
len = ui_inchar(ta_buf, BUFLEN, 10L, 0);
}
if (ta_len > 0 || len > 0)
{
if (len == 1 && cmd != NULL)
{
if (ta_buf[ta_len] == Ctrl_C)
{
TerminateProcess(pi.hProcess, 9);
}
if (ta_buf[ta_len] == Ctrl_D)
{
CloseHandle(g_hChildStd_IN_Wr);
g_hChildStd_IN_Wr = NULL;
}
}
term_replace_bs_del_keycode(ta_buf, ta_len, len);
for (i = ta_len; i < ta_len + len; ++i)
{
if (ta_buf[i] == '\n' || ta_buf[i] == '\b')
msg_putchar(ta_buf[i]);
else if (has_mbyte)
{
int l = (*mb_ptr2len)(ta_buf + i);
msg_outtrans_len(ta_buf + i, l);
i += l - 1;
}
else
msg_outtrans_len(ta_buf + i, 1);
}
windgoto(msg_row, msg_col);
out_flush();
ta_len += len;
if (options & SHELL_WRITE)
ta_len = 0;
else if (g_hChildStd_IN_Wr != NULL)
{
WriteFile(g_hChildStd_IN_Wr, (char*)ta_buf,
1, &len, NULL);
delay = 1;
if (len > 0)
{
ta_len -= len;
mch_memmove(ta_buf, ta_buf + len, ta_len);
}
}
}
}
}
if (ta_len)
ui_inchar_undo(ta_buf, ta_len);
if (WaitForSingleObject(pi.hProcess, delay) != WAIT_TIMEOUT)
{
dump_pipe(options, g_hChildStd_OUT_Rd, &ga, buffer, &buffer_off);
break;
}
++noread_cnt;
dump_pipe(options, g_hChildStd_OUT_Rd, &ga, buffer, &buffer_off);
if (delay < 50)
delay += 10;
}
CloseHandle(g_hChildStd_OUT_Rd);
if (g_hChildStd_IN_Wr != NULL)
CloseHandle(g_hChildStd_IN_Wr);
WaitForSingleObject(pi.hProcess, INFINITE);
GetExitCodeProcess(pi.hProcess, &ret);
if (options & SHELL_READ)
{
if (ga.ga_len > 0)
{
append_ga_line(&ga);
curbuf->b_no_eol_lnum = curwin->w_cursor.lnum;
}
else
curbuf->b_no_eol_lnum = 0;
ga_clear(&ga);
}
CloseHandle(pi.hThread);
CloseHandle(pi.hProcess);
return ret;
}
static int
mch_system_g(char *cmd, int options)
{
if (!p_stmp)
return mch_system_piped(cmd, options);
else
return mch_system_classic(cmd, options);
}
#endif
#if !defined(FEAT_GUI_MSWIN) || defined(VIMDLL)
static int
mch_system_c(char *cmd, int options UNUSED)
{
int ret;
WCHAR *wcmd;
char_u *buf;
size_t len;
len = STRLEN(cmd);
if (len >= 2 && cmd[0] == '"' && cmd[len - 1] == '"')
{
len += 3;
buf = alloc(len);
if (buf == NULL)
return -1;
vim_snprintf((char *)buf, len, "(%s)", cmd);
wcmd = enc_to_utf16(buf, NULL);
free(buf);
}
else
wcmd = enc_to_utf16((char_u *)cmd, NULL);
if (wcmd == NULL)
return -1;
ret = _wsystem(wcmd);
vim_free(wcmd);
return ret;
}
#endif
static int
mch_system(char *cmd, int options)
{
#if defined(VIMDLL)
if (gui.in_use || gui.starting)
return mch_system_g(cmd, options);
else
return mch_system_c(cmd, options);
#elif defined(FEAT_GUI_MSWIN)
return mch_system_g(cmd, options);
#else
return mch_system_c(cmd, options);
#endif
}
#if defined(FEAT_GUI) && defined(FEAT_TERMINAL)
static int
mch_call_shell_terminal(
char_u *cmd,
int options UNUSED) 
{
jobopt_T opt;
char_u *newcmd = NULL;
typval_T argvar[2];
long_u cmdlen;
int retval = -1;
buf_T *buf;
job_T *job;
aco_save_T aco;
oparg_T oa; 
if (cmd == NULL)
cmdlen = STRLEN(p_sh) + 1;
else
cmdlen = STRLEN(p_sh) + STRLEN(p_shcf) + STRLEN(cmd) + 10;
newcmd = alloc(cmdlen);
if (newcmd == NULL)
return 255;
if (cmd == NULL)
{
STRCPY(newcmd, p_sh);
ch_log(NULL, "starting terminal to run a shell");
}
else
{
vim_snprintf((char *)newcmd, cmdlen, "%s %s %s", p_sh, p_shcf, cmd);
ch_log(NULL, "starting terminal for system command '%s'", cmd);
}
init_job_options(&opt);
argvar[0].v_type = VAR_STRING;
argvar[0].vval.v_string = newcmd;
argvar[1].v_type = VAR_UNKNOWN;
buf = term_start(argvar, NULL, &opt, TERM_START_SYSTEM);
if (buf == NULL)
{
vim_free(newcmd);
return 255;
}
job = term_getjob(buf->b_term);
++job->jv_refcount;
aucmd_prepbuf(&aco, buf);
clear_oparg(&oa);
while (term_use_loop())
{
if (oa.op_type == OP_NOP && oa.regname == NUL && !VIsual_active)
{
if (terminal_loop(TRUE) == OK)
normal_cmd(&oa, TRUE);
}
else
normal_cmd(&oa, TRUE);
}
retval = job->jv_exitval;
ch_log(NULL, "system command finished");
job_unref(job);
aucmd_restbuf(&aco);
wait_return(TRUE);
do_buffer(DOBUF_WIPE, DOBUF_FIRST, FORWARD, buf->b_fnum, TRUE);
vim_free(newcmd);
return retval;
}
#endif
int
mch_call_shell(
char_u *cmd,
int options) 
{
int x = 0;
int tmode = cur_tmode;
#if defined(FEAT_TITLE)
WCHAR szShellTitle[512];
if (GetConsoleTitleW(szShellTitle,
sizeof(szShellTitle)/sizeof(WCHAR) - 4) > 0)
{
if (cmd == NULL)
wcscat(szShellTitle, L" :sh");
else
{
WCHAR *wn = enc_to_utf16((char_u *)cmd, NULL);
if (wn != NULL)
{
wcscat(szShellTitle, L" - !");
if ((wcslen(szShellTitle) + wcslen(wn) <
sizeof(szShellTitle)/sizeof(WCHAR)))
wcscat(szShellTitle, wn);
SetConsoleTitleW(szShellTitle);
vim_free(wn);
}
}
}
#endif
out_flush();
#if defined(MCH_WRITE_DUMP)
if (fdDump)
{
fprintf(fdDump, "mch_call_shell(\"%s\", %d)\n", cmd, options);
fflush(fdDump);
}
#endif
#if defined(FEAT_GUI) && defined(FEAT_TERMINAL)
if (
#if defined(VIMDLL)
gui.in_use &&
#endif
vim_strchr(p_go, GO_TERMINAL) != NULL
&& (options & (SHELL_FILTER|SHELL_DOOUT|SHELL_WRITE|SHELL_READ)) == 0)
{
char_u *cmdbase = cmd;
if (cmdbase != NULL)
while (*cmdbase == '"' || *cmdbase == '(')
++cmdbase;
if (cmdbase == NULL || STRNICMP(cmdbase, "start", 5) != 0
|| !VIM_ISWHITE(cmdbase[5]))
{
x = mch_call_shell_terminal(cmd, options);
#if defined(FEAT_TITLE)
resettitle();
#endif
return x;
}
}
#endif
signal(SIGINT, SIG_IGN);
#if defined(__GNUC__) && !defined(__MINGW32__)
signal(SIGKILL, SIG_IGN);
#else
signal(SIGBREAK, SIG_IGN);
#endif
signal(SIGILL, SIG_IGN);
signal(SIGFPE, SIG_IGN);
signal(SIGSEGV, SIG_IGN);
signal(SIGTERM, SIG_IGN);
signal(SIGABRT, SIG_IGN);
if (options & SHELL_COOKED)
settmode(TMODE_COOK); 
if (cmd == NULL)
{
x = mch_system((char *)p_sh, options);
}
else
{
char_u *newcmd = NULL;
char_u *cmdbase = cmd;
long_u cmdlen;
if (*cmdbase == '"' )
++cmdbase;
if (*cmdbase == '(')
++cmdbase;
if ((STRNICMP(cmdbase, "start", 5) == 0) && VIM_ISWHITE(cmdbase[5]))
{
STARTUPINFO si;
PROCESS_INFORMATION pi;
DWORD flags = CREATE_NEW_CONSOLE;
INT n_show_cmd = SW_SHOWNORMAL;
char_u *p;
ZeroMemory(&si, sizeof(si));
si.cb = sizeof(si);
si.lpReserved = NULL;
si.lpDesktop = NULL;
si.lpTitle = NULL;
si.dwFlags = 0;
si.cbReserved2 = 0;
si.lpReserved2 = NULL;
cmdbase = skipwhite(cmdbase + 5);
if ((STRNICMP(cmdbase, "/min", 4) == 0)
&& VIM_ISWHITE(cmdbase[4]))
{
cmdbase = skipwhite(cmdbase + 4);
si.dwFlags = STARTF_USESHOWWINDOW;
si.wShowWindow = SW_SHOWMINNOACTIVE;
n_show_cmd = SW_SHOWMINNOACTIVE;
}
else if ((STRNICMP(cmdbase, "/b", 2) == 0)
&& VIM_ISWHITE(cmdbase[2]))
{
cmdbase = skipwhite(cmdbase + 2);
flags = CREATE_NO_WINDOW;
si.dwFlags = STARTF_USESTDHANDLES;
si.hStdInput = CreateFile("\\\\.\\NUL", 
GENERIC_READ, 
0, 
NULL, 
OPEN_EXISTING, 
FILE_ATTRIBUTE_NORMAL, 
NULL); 
si.hStdOutput = si.hStdInput;
si.hStdError = si.hStdInput;
}
if (cmdbase > cmd)
{
p = cmdbase + STRLEN(cmdbase);
if (p > cmdbase && p[-1] == '"' && *cmd == '"')
*--p = NUL;
if (p > cmdbase && p[-1] == ')'
&& (*cmd =='(' || cmd[1] == '('))
*--p = NUL;
}
newcmd = cmdbase;
unescape_shellxquote(cmdbase, p_sxe);
if (flags != CREATE_NEW_CONSOLE)
{
char_u *subcmd;
char_u *cmd_shell = mch_getenv("COMSPEC");
if (cmd_shell == NULL || *cmd_shell == NUL)
cmd_shell = (char_u *)default_shell();
subcmd = vim_strsave_escaped_ext(cmdbase,
(char_u *)"|", '^', FALSE);
if (subcmd != NULL)
{
cmdlen = STRLEN(cmd_shell) + STRLEN(subcmd) + 5;
newcmd = alloc(cmdlen);
if (newcmd != NULL)
vim_snprintf((char *)newcmd, cmdlen, "%s /c %s",
cmd_shell, subcmd);
else
newcmd = cmdbase;
vim_free(subcmd);
}
}
if (vim_create_process((char *)newcmd, FALSE, flags,
&si, &pi, NULL, NULL))
x = 0;
else if (vim_shell_execute((char *)newcmd, n_show_cmd)
> (HINSTANCE)32)
x = 0;
else
{
x = -1;
#if defined(FEAT_GUI_MSWIN)
#if defined(VIMDLL)
if (gui.in_use)
#endif
emsg(_("E371: Command not found"));
#endif
}
if (newcmd != cmdbase)
vim_free(newcmd);
if (si.dwFlags == STARTF_USESTDHANDLES && si.hStdInput != NULL)
{
CloseHandle(si.hStdInput);
}
CloseHandle(pi.hThread);
CloseHandle(pi.hProcess);
}
else
{
cmdlen =
#if defined(FEAT_GUI_MSWIN)
((gui.in_use || gui.starting) ?
(!s_dont_use_vimrun && p_stmp ?
STRLEN(vimrun_path) : STRLEN(p_sh) + STRLEN(p_shcf))
: 0) +
#endif
STRLEN(p_sh) + STRLEN(p_shcf) + STRLEN(cmd) + 10;
newcmd = alloc(cmdlen);
if (newcmd != NULL)
{
#if defined(FEAT_GUI_MSWIN)
if (
#if defined(VIMDLL)
(gui.in_use || gui.starting) &&
#endif
need_vimrun_warning)
{
char *msg = _("VIMRUN.EXE not found in your $PATH.\n"
"External commands will not pause after completion.\n"
"See :help win32-vimrun for more information.");
char *title = _("Vim Warning");
WCHAR *wmsg = enc_to_utf16((char_u *)msg, NULL);
WCHAR *wtitle = enc_to_utf16((char_u *)title, NULL);
if (wmsg != NULL && wtitle != NULL)
MessageBoxW(NULL, wmsg, wtitle, MB_ICONWARNING);
vim_free(wmsg);
vim_free(wtitle);
need_vimrun_warning = FALSE;
}
if (
#if defined(VIMDLL)
(gui.in_use || gui.starting) &&
#endif
!s_dont_use_vimrun && p_stmp)
vim_snprintf((char *)newcmd, cmdlen, "%s%s%s %s %s",
vimrun_path,
(msg_silent != 0 || (options & SHELL_DOOUT))
? "-s " : "",
p_sh, p_shcf, cmd);
else if (
#if defined(VIMDLL)
(gui.in_use || gui.starting) &&
#endif
s_dont_use_vimrun && STRCMP(p_shcf, "/c") == 0)
vim_snprintf((char *)newcmd, cmdlen, "%s %s %s %s %s",
p_sh, p_shcf, p_sh, p_shcf, cmd);
else
#endif
vim_snprintf((char *)newcmd, cmdlen, "%s %s %s",
p_sh, p_shcf, cmd);
x = mch_system((char *)newcmd, options);
vim_free(newcmd);
}
}
}
if (tmode == TMODE_RAW)
settmode(TMODE_RAW); 
if (x != 0 && !(options & SHELL_SILENT) && !emsg_silent
#if defined(FEAT_GUI_MSWIN)
&& ((gui.in_use || gui.starting) ?
((options & SHELL_DOOUT) || s_dont_use_vimrun || !p_stmp) : 1)
#endif
)
{
smsg(_("shell returned %d"), x);
msg_putchar('\n');
}
#if defined(FEAT_TITLE)
resettitle();
#endif
signal(SIGINT, SIG_DFL);
#if defined(__GNUC__) && !defined(__MINGW32__)
signal(SIGKILL, SIG_DFL);
#else
signal(SIGBREAK, SIG_DFL);
#endif
signal(SIGILL, SIG_DFL);
signal(SIGFPE, SIG_DFL);
signal(SIGSEGV, SIG_DFL);
signal(SIGTERM, SIG_DFL);
signal(SIGABRT, SIG_DFL);
return x;
}
#if defined(FEAT_JOB_CHANNEL) || defined(PROTO)
static HANDLE
job_io_file_open(
char_u *fname,
DWORD dwDesiredAccess,
DWORD dwShareMode,
LPSECURITY_ATTRIBUTES lpSecurityAttributes,
DWORD dwCreationDisposition,
DWORD dwFlagsAndAttributes)
{
HANDLE h;
WCHAR *wn;
wn = enc_to_utf16(fname, NULL);
if (wn == NULL)
return INVALID_HANDLE_VALUE;
h = CreateFileW(wn, dwDesiredAccess, dwShareMode,
lpSecurityAttributes, dwCreationDisposition,
dwFlagsAndAttributes, NULL);
vim_free(wn);
return h;
}
void
win32_build_env(dict_T *env, garray_T *gap, int is_terminal)
{
hashitem_T *hi;
long_u todo = env != NULL ? env->dv_hashtab.ht_used : 0;
LPVOID base = GetEnvironmentStringsW();
if (ga_grow(gap, 1) == FAIL)
return;
if (env != NULL)
{
for (hi = env->dv_hashtab.ht_array; todo > 0; ++hi)
{
if (!HASHITEM_EMPTY(hi))
{
typval_T *item = &dict_lookup(hi)->di_tv;
WCHAR *wkey = enc_to_utf16((char_u *)hi->hi_key, NULL);
WCHAR *wval = enc_to_utf16(tv_get_string(item), NULL);
--todo;
if (wkey != NULL && wval != NULL)
{
size_t n;
size_t lkey = wcslen(wkey);
size_t lval = wcslen(wval);
if (ga_grow(gap, (int)(lkey + lval + 2)) != OK)
continue;
for (n = 0; n < lkey; n++)
*((WCHAR*)gap->ga_data + gap->ga_len++) = wkey[n];
*((WCHAR*)gap->ga_data + gap->ga_len++) = L'=';
for (n = 0; n < lval; n++)
*((WCHAR*)gap->ga_data + gap->ga_len++) = wval[n];
*((WCHAR*)gap->ga_data + gap->ga_len++) = L'\0';
}
vim_free(wkey);
vim_free(wval);
}
}
}
if (base)
{
WCHAR *p = (WCHAR*) base;
if (ga_grow(gap, 1) == FAIL)
return;
while (*p != 0 || *(p + 1) != 0)
{
if (ga_grow(gap, 1) == OK)
*((WCHAR*)gap->ga_data + gap->ga_len++) = *p;
p++;
}
FreeEnvironmentStrings(base);
*((WCHAR*)gap->ga_data + gap->ga_len++) = L'\0';
}
#if defined(FEAT_CLIENTSERVER) || defined(FEAT_TERMINAL)
{
#if defined(FEAT_CLIENTSERVER)
char_u *servername = get_vim_var_str(VV_SEND_SERVER);
size_t servername_len = STRLEN(servername);
#endif
#if defined(FEAT_TERMINAL)
char_u *version = get_vim_var_str(VV_VERSION);
size_t version_len = STRLEN(version);
#endif
size_t n = 0
#if defined(FEAT_CLIENTSERVER)
+ 15 + servername_len
#endif
#if defined(FEAT_TERMINAL)
+ 13 + version_len + 2
#endif
;
if (ga_grow(gap, (int)n) == OK)
{
#if defined(FEAT_CLIENTSERVER)
for (n = 0; n < 15; n++)
*((WCHAR*)gap->ga_data + gap->ga_len++) =
(WCHAR)"VIM_SERVERNAME="[n];
for (n = 0; n < servername_len; n++)
*((WCHAR*)gap->ga_data + gap->ga_len++) =
(WCHAR)servername[n];
*((WCHAR*)gap->ga_data + gap->ga_len++) = L'\0';
#endif
#if defined(FEAT_TERMINAL)
if (is_terminal)
{
for (n = 0; n < 13; n++)
*((WCHAR*)gap->ga_data + gap->ga_len++) =
(WCHAR)"VIM_TERMINAL="[n];
for (n = 0; n < version_len; n++)
*((WCHAR*)gap->ga_data + gap->ga_len++) =
(WCHAR)version[n];
*((WCHAR*)gap->ga_data + gap->ga_len++) = L'\0';
}
#endif
}
}
#endif
}
static BOOL
create_pipe_pair(HANDLE handles[2])
{
static LONG s;
char name[64];
SECURITY_ATTRIBUTES sa;
sprintf(name, "\\\\?\\pipe\\vim-%08lx-%08lx",
GetCurrentProcessId(),
InterlockedIncrement(&s));
handles[1] = CreateNamedPipe(
name,
PIPE_ACCESS_OUTBOUND | FILE_FLAG_OVERLAPPED,
PIPE_TYPE_BYTE | PIPE_NOWAIT,
1, MAX_NAMED_PIPE_SIZE, 0, 0, NULL);
if (handles[1] == INVALID_HANDLE_VALUE)
return FALSE;
sa.nLength = sizeof(sa);
sa.bInheritHandle = TRUE;
sa.lpSecurityDescriptor = NULL;
handles[0] = CreateFile(name,
FILE_GENERIC_READ,
FILE_SHARE_READ, &sa,
OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
if (handles[0] == INVALID_HANDLE_VALUE)
{
CloseHandle(handles[1]);
return FALSE;
}
return TRUE;
}
void
mch_job_start(char *cmd, job_T *job, jobopt_T *options)
{
STARTUPINFO si;
PROCESS_INFORMATION pi;
HANDLE jo;
SECURITY_ATTRIBUTES saAttr;
channel_T *channel = NULL;
HANDLE ifd[2];
HANDLE ofd[2];
HANDLE efd[2];
garray_T ga;
int use_null_for_in = options->jo_io[PART_IN] == JIO_NULL;
int use_null_for_out = options->jo_io[PART_OUT] == JIO_NULL;
int use_null_for_err = options->jo_io[PART_ERR] == JIO_NULL;
int use_file_for_in = options->jo_io[PART_IN] == JIO_FILE;
int use_file_for_out = options->jo_io[PART_OUT] == JIO_FILE;
int use_file_for_err = options->jo_io[PART_ERR] == JIO_FILE;
int use_out_for_err = options->jo_io[PART_ERR] == JIO_OUT;
if (use_out_for_err && use_null_for_out)
use_null_for_err = TRUE;
ifd[0] = INVALID_HANDLE_VALUE;
ifd[1] = INVALID_HANDLE_VALUE;
ofd[0] = INVALID_HANDLE_VALUE;
ofd[1] = INVALID_HANDLE_VALUE;
efd[0] = INVALID_HANDLE_VALUE;
efd[1] = INVALID_HANDLE_VALUE;
ga_init2(&ga, (int)sizeof(wchar_t), 500);
jo = CreateJobObject(NULL, NULL);
if (jo == NULL)
{
job->jv_status = JOB_FAILED;
goto failed;
}
if (options->jo_env != NULL)
win32_build_env(options->jo_env, &ga, FALSE);
ZeroMemory(&pi, sizeof(pi));
ZeroMemory(&si, sizeof(si));
si.cb = sizeof(si);
si.dwFlags |= STARTF_USESHOWWINDOW;
si.wShowWindow = SW_HIDE;
saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
saAttr.bInheritHandle = TRUE;
saAttr.lpSecurityDescriptor = NULL;
if (use_file_for_in)
{
char_u *fname = options->jo_io_name[PART_IN];
ifd[0] = job_io_file_open(fname, GENERIC_READ,
FILE_SHARE_READ | FILE_SHARE_WRITE,
&saAttr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL);
if (ifd[0] == INVALID_HANDLE_VALUE)
{
semsg(_(e_notopen), fname);
goto failed;
}
}
else if (!use_null_for_in
&& (!create_pipe_pair(ifd)
|| !SetHandleInformation(ifd[1], HANDLE_FLAG_INHERIT, 0)))
goto failed;
if (use_file_for_out)
{
char_u *fname = options->jo_io_name[PART_OUT];
ofd[1] = job_io_file_open(fname, GENERIC_WRITE,
FILE_SHARE_READ | FILE_SHARE_WRITE,
&saAttr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL);
if (ofd[1] == INVALID_HANDLE_VALUE)
{
semsg(_(e_notopen), fname);
goto failed;
}
}
else if (!use_null_for_out &&
(!CreatePipe(&ofd[0], &ofd[1], &saAttr, 0)
|| !SetHandleInformation(ofd[0], HANDLE_FLAG_INHERIT, 0)))
goto failed;
if (use_file_for_err)
{
char_u *fname = options->jo_io_name[PART_ERR];
efd[1] = job_io_file_open(fname, GENERIC_WRITE,
FILE_SHARE_READ | FILE_SHARE_WRITE,
&saAttr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL);
if (efd[1] == INVALID_HANDLE_VALUE)
{
semsg(_(e_notopen), fname);
goto failed;
}
}
else if (!use_out_for_err && !use_null_for_err &&
(!CreatePipe(&efd[0], &efd[1], &saAttr, 0)
|| !SetHandleInformation(efd[0], HANDLE_FLAG_INHERIT, 0)))
goto failed;
si.dwFlags |= STARTF_USESTDHANDLES;
si.hStdInput = ifd[0];
si.hStdOutput = ofd[1];
si.hStdError = use_out_for_err ? ofd[1] : efd[1];
if (!use_null_for_in || !use_null_for_out || !use_null_for_err)
{
if (options->jo_set & JO_CHANNEL)
{
channel = options->jo_channel;
if (channel != NULL)
++channel->ch_refcount;
}
else
channel = add_channel();
if (channel == NULL)
goto failed;
}
if (!vim_create_process(cmd, TRUE,
CREATE_SUSPENDED |
CREATE_DEFAULT_ERROR_MODE |
CREATE_NEW_PROCESS_GROUP |
CREATE_UNICODE_ENVIRONMENT |
CREATE_NEW_CONSOLE,
&si, &pi,
ga.ga_data,
(char *)options->jo_cwd))
{
CloseHandle(jo);
job->jv_status = JOB_FAILED;
goto failed;
}
ga_clear(&ga);
if (!AssignProcessToJobObject(jo, pi.hProcess))
{
CloseHandle(jo);
jo = NULL;
}
ResumeThread(pi.hThread);
CloseHandle(pi.hThread);
job->jv_proc_info = pi;
job->jv_job_object = jo;
job->jv_status = JOB_STARTED;
CloseHandle(ifd[0]);
CloseHandle(ofd[1]);
if (!use_out_for_err && !use_null_for_err)
CloseHandle(efd[1]);
job->jv_channel = channel;
if (channel != NULL)
{
channel_set_pipes(channel,
use_file_for_in || use_null_for_in
? INVALID_FD : (sock_T)ifd[1],
use_file_for_out || use_null_for_out
? INVALID_FD : (sock_T)ofd[0],
use_out_for_err || use_file_for_err || use_null_for_err
? INVALID_FD : (sock_T)efd[0]);
channel_set_job(channel, job, options);
}
return;
failed:
CloseHandle(ifd[0]);
CloseHandle(ofd[0]);
CloseHandle(efd[0]);
CloseHandle(ifd[1]);
CloseHandle(ofd[1]);
CloseHandle(efd[1]);
channel_unref(channel);
ga_clear(&ga);
}
char *
mch_job_status(job_T *job)
{
DWORD dwExitCode = 0;
if (!GetExitCodeProcess(job->jv_proc_info.hProcess, &dwExitCode)
|| dwExitCode != STILL_ACTIVE)
{
job->jv_exitval = (int)dwExitCode;
if (job->jv_status < JOB_ENDED)
{
ch_log(job->jv_channel, "Job ended");
job->jv_status = JOB_ENDED;
}
return "dead";
}
return "run";
}
job_T *
mch_detect_ended_job(job_T *job_list)
{
HANDLE jobHandles[MAXIMUM_WAIT_OBJECTS];
job_T *jobArray[MAXIMUM_WAIT_OBJECTS];
job_T *job = job_list;
while (job != NULL)
{
DWORD n;
DWORD result;
for (n = 0; n < MAXIMUM_WAIT_OBJECTS
&& job != NULL; job = job->jv_next)
{
if (job->jv_status == JOB_STARTED)
{
jobHandles[n] = job->jv_proc_info.hProcess;
jobArray[n] = job;
++n;
}
}
if (n == 0)
continue;
result = WaitForMultipleObjects(n, jobHandles, FALSE, 0);
if (result >= WAIT_OBJECT_0 && result < WAIT_OBJECT_0 + n)
{
job_T *wait_job = jobArray[result - WAIT_OBJECT_0];
if (STRCMP(mch_job_status(wait_job), "dead") == 0)
return wait_job;
}
}
return NULL;
}
static BOOL
terminate_all(HANDLE process, int code)
{
PROCESSENTRY32 pe;
HANDLE h = INVALID_HANDLE_VALUE;
DWORD pid = GetProcessId(process);
if (pid != 0)
{
h = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
if (h != INVALID_HANDLE_VALUE)
{
pe.dwSize = sizeof(PROCESSENTRY32);
if (!Process32First(h, &pe))
goto theend;
do
{
if (pe.th32ParentProcessID == pid)
{
HANDLE ph = OpenProcess(
PROCESS_ALL_ACCESS, FALSE, pe.th32ProcessID);
if (ph != NULL)
{
terminate_all(ph, code);
CloseHandle(ph);
}
}
} while (Process32Next(h, &pe));
CloseHandle(h);
}
}
theend:
return TerminateProcess(process, code);
}
int
mch_signal_job(job_T *job, char_u *how)
{
int ret;
if (STRCMP(how, "term") == 0 || STRCMP(how, "kill") == 0 || *how == NUL)
{
if (job->jv_job_object != NULL)
{
if (job->jv_channel != NULL && job->jv_channel->ch_anonymous_pipe)
job->jv_channel->ch_killing = TRUE;
return TerminateJobObject(job->jv_job_object, -1) ? OK : FAIL;
}
return terminate_all(job->jv_proc_info.hProcess, -1) ? OK : FAIL;
}
if (!AttachConsole(job->jv_proc_info.dwProcessId))
return FAIL;
ret = GenerateConsoleCtrlEvent(
STRCMP(how, "int") == 0 ? CTRL_C_EVENT : CTRL_BREAK_EVENT,
job->jv_proc_info.dwProcessId)
? OK : FAIL;
FreeConsole();
return ret;
}
void
mch_clear_job(job_T *job)
{
if (job->jv_status != JOB_FAILED)
{
if (job->jv_job_object != NULL)
CloseHandle(job->jv_job_object);
CloseHandle(job->jv_proc_info.hProcess);
}
}
#endif
#if !defined(FEAT_GUI_MSWIN) || defined(VIMDLL)
static void
termcap_mode_start(void)
{
DWORD cmodein;
if (g_fTermcapMode)
return;
if (!p_rs && USE_VTP)
vtp_printf("\033[?1049h");
SaveConsoleBuffer(&g_cbNonTermcap);
if (g_cbTermcap.IsValid)
{
RestoreConsoleBuffer(&g_cbTermcap, FALSE);
reset_console_color_rgb();
SetConsoleWindowInfo(g_hConOut, TRUE, &g_cbTermcap.Info.srWindow);
Rows = g_cbTermcap.Info.dwSize.Y;
Columns = g_cbTermcap.Info.dwSize.X;
}
else
{
ClearConsoleBuffer(g_attrCurrent);
set_console_color_rgb();
ResizeConBufAndWindow(g_hConOut, Columns, Rows);
}
#if defined(FEAT_TITLE)
resettitle();
#endif
GetConsoleMode(g_hConIn, &cmodein);
if (g_fMouseActive)
cmodein |= ENABLE_MOUSE_INPUT;
else
cmodein &= ~ENABLE_MOUSE_INPUT;
cmodein |= ENABLE_WINDOW_INPUT;
SetConsoleMode(g_hConIn, cmodein);
redraw_later_clear();
g_fTermcapMode = TRUE;
}
static void
termcap_mode_end(void)
{
DWORD cmodein;
ConsoleBuffer *cb;
COORD coord;
DWORD dwDummy;
if (!g_fTermcapMode)
return;
SaveConsoleBuffer(&g_cbTermcap);
GetConsoleMode(g_hConIn, &cmodein);
cmodein &= ~(ENABLE_MOUSE_INPUT | ENABLE_WINDOW_INPUT);
SetConsoleMode(g_hConIn, cmodein);
#if defined(FEAT_RESTORE_ORIG_SCREEN)
cb = exiting ? &g_cbOrig : &g_cbNonTermcap;
#else
cb = &g_cbNonTermcap;
#endif
RestoreConsoleBuffer(cb, p_rs);
restore_console_color_rgb();
SetConsoleCursorInfo(g_hConOut, &g_cci);
if (p_rs || exiting)
{
coord.X = 0;
coord.Y = (SHORT) (p_rs ? cb->Info.dwCursorPosition.Y : (Rows - 1));
FillConsoleOutputCharacter(g_hConOut, ' ',
cb->Info.dwSize.X, coord, &dwDummy);
if (exiting && !p_rs)
coord.Y--;
SetConsoleCursorPosition(g_hConOut, coord);
}
if (!p_rs && USE_VTP)
vtp_printf("\033[?1049l");
g_fTermcapMode = FALSE;
}
#endif 
#if defined(FEAT_GUI_MSWIN) && !defined(VIMDLL)
void
mch_write(
char_u *s UNUSED,
int len UNUSED)
{
}
#else
static void
clear_chars(
COORD coord,
DWORD n)
{
DWORD dwDummy;
FillConsoleOutputCharacter(g_hConOut, ' ', n, coord, &dwDummy);
if (!USE_VTP)
FillConsoleOutputAttribute(g_hConOut, g_attrCurrent, n, coord, &dwDummy);
else
{
set_console_color_rgb();
gotoxy(coord.X + 1, coord.Y + 1);
vtp_printf("\033[%dX", n);
}
}
static void
clear_screen(void)
{
g_coord.X = g_coord.Y = 0;
if (!USE_VTP)
clear_chars(g_coord, Rows * Columns);
else
{
set_console_color_rgb();
gotoxy(1, 1);
vtp_printf("\033[2J");
}
}
static void
clear_to_end_of_display(void)
{
COORD save = g_coord;
if (!USE_VTP)
clear_chars(g_coord, (Rows - g_coord.Y - 1)
* Columns + (Columns - g_coord.X));
else
{
set_console_color_rgb();
gotoxy(g_coord.X + 1, g_coord.Y + 1);
vtp_printf("\033[0J");
gotoxy(save.X + 1, save.Y + 1);
g_coord = save;
}
}
static void
clear_to_end_of_line(void)
{
COORD save = g_coord;
if (!USE_VTP)
clear_chars(g_coord, Columns - g_coord.X);
else
{
set_console_color_rgb();
gotoxy(g_coord.X + 1, g_coord.Y + 1);
vtp_printf("\033[0K");
gotoxy(save.X + 1, save.Y + 1);
g_coord = save;
}
}
static void
scroll(unsigned cLines)
{
COORD oldcoord = g_coord;
gotoxy(g_srScrollRegion.Left + 1, g_srScrollRegion.Top + 1);
delete_lines(cLines);
g_coord = oldcoord;
}
static void
set_scroll_region(
unsigned left,
unsigned top,
unsigned right,
unsigned bottom)
{
if (left >= right
|| top >= bottom
|| right > (unsigned) Columns - 1
|| bottom > (unsigned) Rows - 1)
return;
g_srScrollRegion.Left = left;
g_srScrollRegion.Top = top;
g_srScrollRegion.Right = right;
g_srScrollRegion.Bottom = bottom;
}
static void
set_scroll_region_tb(
unsigned top,
unsigned bottom)
{
if (top >= bottom || bottom > (unsigned)Rows - 1)
return;
g_srScrollRegion.Top = top;
g_srScrollRegion.Bottom = bottom;
}
static void
set_scroll_region_lr(
unsigned left,
unsigned right)
{
if (left >= right || right > (unsigned)Columns - 1)
return;
g_srScrollRegion.Left = left;
g_srScrollRegion.Right = right;
}
static void
insert_lines(unsigned cLines)
{
SMALL_RECT source, clip;
COORD dest;
CHAR_INFO fill;
gotoxy(g_srScrollRegion.Left + 1, g_srScrollRegion.Top + 1);
dest.X = g_srScrollRegion.Left;
dest.Y = g_coord.Y + cLines;
source.Left = g_srScrollRegion.Left;
source.Top = g_coord.Y;
source.Right = g_srScrollRegion.Right;
source.Bottom = g_srScrollRegion.Bottom - cLines;
clip.Left = g_srScrollRegion.Left;
clip.Top = g_coord.Y;
clip.Right = g_srScrollRegion.Right;
clip.Bottom = g_srScrollRegion.Bottom;
fill.Char.AsciiChar = ' ';
if (!USE_VTP)
fill.Attributes = g_attrCurrent;
else
fill.Attributes = g_attrDefault;
set_console_color_rgb();
ScrollConsoleScreenBuffer(g_hConOut, &source, &clip, dest, &fill);
if (source.Bottom < dest.Y)
{
COORD coord;
int i;
coord.X = source.Left;
for (i = clip.Top; i < dest.Y; ++i)
{
coord.Y = i;
clear_chars(coord, source.Right - source.Left + 1);
}
}
}
static void
delete_lines(unsigned cLines)
{
SMALL_RECT source, clip;
COORD dest;
CHAR_INFO fill;
int nb;
gotoxy(g_srScrollRegion.Left + 1, g_srScrollRegion.Top + 1);
dest.X = g_srScrollRegion.Left;
dest.Y = g_coord.Y;
source.Left = g_srScrollRegion.Left;
source.Top = g_coord.Y + cLines;
source.Right = g_srScrollRegion.Right;
source.Bottom = g_srScrollRegion.Bottom;
clip.Left = g_srScrollRegion.Left;
clip.Top = g_coord.Y;
clip.Right = g_srScrollRegion.Right;
clip.Bottom = g_srScrollRegion.Bottom;
fill.Char.AsciiChar = ' ';
if (!USE_VTP)
fill.Attributes = g_attrCurrent;
else
fill.Attributes = g_attrDefault;
set_console_color_rgb();
ScrollConsoleScreenBuffer(g_hConOut, &source, &clip, dest, &fill);
nb = dest.Y + (source.Bottom - source.Top) + 1;
if (nb < source.Top)
{
COORD coord;
int i;
coord.X = source.Left;
for (i = nb; i < clip.Bottom; ++i)
{
coord.Y = i;
clear_chars(coord, source.Right - source.Left + 1);
}
}
}
static void
gotoxy(
unsigned x,
unsigned y)
{
if (x < 1 || x > (unsigned)Columns || y < 1 || y > (unsigned)Rows)
return;
if (!USE_VTP)
{
g_coord.X = 0;
SetConsoleCursorPosition(g_hConOut, g_coord);
g_coord.X = x - 1;
g_coord.Y = y - 1;
SetConsoleCursorPosition(g_hConOut, g_coord);
}
else
{
vtp_printf("\033[%d;%dH", g_coord.Y + 1, 1);
vtp_printf("\033[%d;%dH", y, x);
g_coord.X = x - 1;
g_coord.Y = y - 1;
}
}
static void
textattr(WORD wAttr)
{
g_attrCurrent = wAttr & 0xff;
SetConsoleTextAttribute(g_hConOut, wAttr);
}
static void
textcolor(WORD wAttr)
{
g_attrCurrent = (g_attrCurrent & 0xf0) + (wAttr & 0x0f);
if (!USE_VTP)
SetConsoleTextAttribute(g_hConOut, g_attrCurrent);
else
vtp_sgr_bulk(wAttr);
}
static void
textbackground(WORD wAttr)
{
g_attrCurrent = (g_attrCurrent & 0x0f) + ((wAttr & 0x0f) << 4);
if (!USE_VTP)
SetConsoleTextAttribute(g_hConOut, g_attrCurrent);
else
vtp_sgr_bulk(wAttr);
}
static void
normvideo(void)
{
if (!USE_VTP)
textattr(g_attrDefault);
else
vtp_sgr_bulk(0);
}
static WORD g_attrPreStandout = 0;
static void
standout(void)
{
g_attrPreStandout = g_attrCurrent;
textattr((WORD) (g_attrCurrent|FOREGROUND_INTENSITY|BACKGROUND_INTENSITY));
}
static void
standend(void)
{
if (g_attrPreStandout)
textattr(g_attrPreStandout);
g_attrPreStandout = 0;
}
void
mch_set_normal_colors(void)
{
char_u *p;
int n;
cterm_normal_fg_color = (g_attrDefault & 0xf) + 1;
cterm_normal_bg_color = ((g_attrDefault >> 4) & 0xf) + 1;
if (
#if defined(FEAT_TERMGUICOLORS)
!p_tgc &&
#endif
T_ME[0] == ESC && T_ME[1] == '|')
{
p = T_ME + 2;
n = getdigits(&p);
if (*p == 'm' && n > 0)
{
cterm_normal_fg_color = (n & 0xf) + 1;
cterm_normal_bg_color = ((n >> 4) & 0xf) + 1;
}
}
#if defined(FEAT_TERMGUICOLORS)
cterm_normal_fg_gui_color = INVALCOLOR;
cterm_normal_bg_gui_color = INVALCOLOR;
#endif
}
static void
visual_bell(void)
{
COORD coordOrigin = {0, 0};
WORD attrFlash = ~g_attrCurrent & 0xff;
DWORD dwDummy;
LPWORD oldattrs = ALLOC_MULT(WORD, Rows * Columns);
if (oldattrs == NULL)
return;
ReadConsoleOutputAttribute(g_hConOut, oldattrs, Rows * Columns,
coordOrigin, &dwDummy);
FillConsoleOutputAttribute(g_hConOut, attrFlash, Rows * Columns,
coordOrigin, &dwDummy);
Sleep(15); 
if (!USE_VTP)
WriteConsoleOutputAttribute(g_hConOut, oldattrs, Rows * Columns,
coordOrigin, &dwDummy);
vim_free(oldattrs);
}
static void
cursor_visible(BOOL fVisible)
{
s_cursor_visible = fVisible;
#if defined(MCH_CURSOR_SHAPE)
mch_update_cursor();
#endif
}
static DWORD
write_chars(
char_u *pchBuf,
DWORD cbToWrite)
{
COORD coord = g_coord;
DWORD written;
DWORD n, cchwritten, cells;
static WCHAR *unicodebuf = NULL;
static int unibuflen = 0;
int length;
int cp = enc_utf8 ? CP_UTF8 : enc_codepage;
length = MultiByteToWideChar(cp, 0, (LPCSTR)pchBuf, cbToWrite, 0, 0);
if (unicodebuf == NULL || length > unibuflen)
{
vim_free(unicodebuf);
unicodebuf = LALLOC_MULT(WCHAR, length);
unibuflen = length;
}
MultiByteToWideChar(cp, 0, (LPCSTR)pchBuf, cbToWrite,
unicodebuf, unibuflen);
cells = mb_string2cells(pchBuf, cbToWrite);
if (!USE_VTP)
{
FillConsoleOutputAttribute(g_hConOut, g_attrCurrent, cells,
coord, &written);
if (WriteConsoleOutputCharacterW(g_hConOut, unicodebuf, length,
coord, &cchwritten) == 0
|| cchwritten == 0 || cchwritten == (DWORD)-1)
cchwritten = 1;
}
else
{
if (WriteConsoleW(g_hConOut, unicodebuf, length, &cchwritten,
NULL) == 0 || cchwritten == 0)
cchwritten = 1;
}
if (cchwritten == length)
{
written = cbToWrite;
g_coord.X += (SHORT)cells;
}
else
{
char_u *p = pchBuf;
for (n = 0; n < cchwritten; n++)
MB_CPTR_ADV(p);
written = p - pchBuf;
g_coord.X += (SHORT)mb_string2cells(pchBuf, written);
}
while (g_coord.X > g_srScrollRegion.Right)
{
g_coord.X -= (SHORT) Columns;
if (g_coord.Y < g_srScrollRegion.Bottom)
g_coord.Y++;
}
gotoxy(g_coord.X + 1, g_coord.Y + 1);
return written;
}
void
mch_write(
char_u *s,
int len)
{
#if defined(VIMDLL)
if (gui.in_use)
return;
#endif
s[len] = NUL;
if (!term_console)
{
write(1, s, (unsigned)len);
return;
}
while (len--)
{
DWORD prefix = (DWORD)strcspn((char *)s, "\n\r\b\a\033");
if (p_wd)
{
WaitForChar(p_wd, FALSE);
if (prefix != 0)
prefix = 1;
}
if (prefix != 0)
{
DWORD nWritten;
nWritten = write_chars(s, prefix);
#if defined(MCH_WRITE_DUMP)
if (fdDump)
{
fputc('>', fdDump);
fwrite(s, sizeof(char_u), nWritten, fdDump);
fputs("<\n", fdDump);
}
#endif
len -= (nWritten - 1);
s += nWritten;
}
else if (s[0] == '\n')
{
if (g_coord.Y == g_srScrollRegion.Bottom)
{
scroll(1);
gotoxy(g_srScrollRegion.Left + 1, g_srScrollRegion.Bottom + 1);
}
else
{
gotoxy(g_srScrollRegion.Left + 1, g_coord.Y + 2);
}
#if defined(MCH_WRITE_DUMP)
if (fdDump)
fputs("\\n\n", fdDump);
#endif
s++;
}
else if (s[0] == '\r')
{
gotoxy(g_srScrollRegion.Left+1, g_coord.Y + 1);
#if defined(MCH_WRITE_DUMP)
if (fdDump)
fputs("\\r\n", fdDump);
#endif
s++;
}
else if (s[0] == '\b')
{
if (g_coord.X > g_srScrollRegion.Left)
g_coord.X--;
else if (g_coord.Y > g_srScrollRegion.Top)
{
g_coord.X = g_srScrollRegion.Right;
g_coord.Y--;
}
gotoxy(g_coord.X + 1, g_coord.Y + 1);
#if defined(MCH_WRITE_DUMP)
if (fdDump)
fputs("\\b\n", fdDump);
#endif
s++;
}
else if (s[0] == '\a')
{
MessageBeep(0xFFFFFFFF);
#if defined(MCH_WRITE_DUMP)
if (fdDump)
fputs("\\a\n", fdDump);
#endif
s++;
}
else if (s[0] == ESC && len >= 3-1 && s[1] == '|')
{
#if defined(MCH_WRITE_DUMP)
char_u *old_s = s;
#endif
char_u *p;
int arg1 = 0, arg2 = 0, argc = 0, args[16];
switch (s[2])
{
case '0': case '1': case '2': case '3': case '4':
case '5': case '6': case '7': case '8': case '9':
p = s + 1;
do
{
++p;
args[argc] = getdigits(&p);
argc += (argc < 15) ? 1 : 0;
if (p > s + len)
break;
} while (*p == ';');
if (p > s + len)
break;
arg1 = args[0];
arg2 = args[1];
if (*p == 'm')
{
if (argc == 1 && args[0] == 0)
normvideo();
else if (argc == 1)
{
if (USE_VTP)
textcolor((WORD) arg1);
else
textattr((WORD) arg1);
}
else if (USE_VTP)
vtp_sgr_bulks(argc, args);
}
else if (argc == 2 && *p == 'H')
{
gotoxy(arg2, arg1);
}
else if (argc == 2 && *p == 'r')
{
set_scroll_region(0, arg1 - 1, Columns - 1, arg2 - 1);
}
else if (argc == 2 && *p == 'R')
{
set_scroll_region_tb(arg1, arg2);
}
else if (argc == 2 && *p == 'V')
{
set_scroll_region_lr(arg1, arg2);
}
else if (argc == 1 && *p == 'A')
{
gotoxy(g_coord.X + 1,
max(g_srScrollRegion.Top, g_coord.Y - arg1) + 1);
}
else if (argc == 1 && *p == 'b')
{
textbackground((WORD) arg1);
}
else if (argc == 1 && *p == 'C')
{
gotoxy(min(g_srScrollRegion.Right, g_coord.X + arg1) + 1,
g_coord.Y + 1);
}
else if (argc == 1 && *p == 'f')
{
textcolor((WORD) arg1);
}
else if (argc == 1 && *p == 'H')
{
gotoxy(1, arg1);
}
else if (argc == 1 && *p == 'L')
{
insert_lines(arg1);
}
else if (argc == 1 && *p == 'M')
{
delete_lines(arg1);
}
len -= (int)(p - s);
s = p + 1;
break;
case 'A':
gotoxy(g_coord.X + 1,
max(g_srScrollRegion.Top, g_coord.Y - 1) + 1);
goto got3;
case 'B':
visual_bell();
goto got3;
case 'C':
gotoxy(min(g_srScrollRegion.Right, g_coord.X + 1) + 1,
g_coord.Y + 1);
goto got3;
case 'E':
termcap_mode_end();
goto got3;
case 'F':
standout();
goto got3;
case 'f':
standend();
goto got3;
case 'H':
gotoxy(1, 1);
goto got3;
case 'j':
clear_to_end_of_display();
goto got3;
case 'J':
clear_screen();
goto got3;
case 'K':
clear_to_end_of_line();
goto got3;
case 'L':
insert_lines(1);
goto got3;
case 'M':
delete_lines(1);
goto got3;
case 'S':
termcap_mode_start();
goto got3;
case 'V':
cursor_visible(TRUE);
goto got3;
case 'v':
cursor_visible(FALSE);
goto got3;
got3:
s += 3;
len -= 2;
}
#if defined(MCH_WRITE_DUMP)
if (fdDump)
{
fputs("ESC | ", fdDump);
fwrite(old_s + 2, sizeof(char_u), s - old_s - 2, fdDump);
fputc('\n', fdDump);
}
#endif
}
else
{
DWORD nWritten;
nWritten = write_chars(s, 1);
#if defined(MCH_WRITE_DUMP)
if (fdDump)
{
fputc('>', fdDump);
fwrite(s, sizeof(char_u), nWritten, fdDump);
fputs("<\n", fdDump);
}
#endif
len -= (nWritten - 1);
s += nWritten;
}
}
#if defined(MCH_WRITE_DUMP)
if (fdDump)
fflush(fdDump);
#endif
}
#endif 
void
mch_delay(
long msec,
int ignoreinput UNUSED)
{
#if defined(FEAT_GUI_MSWIN) && !defined(VIMDLL)
Sleep((int)msec); 
#else 
#if defined(VIMDLL)
if (gui.in_use)
{
Sleep((int)msec); 
return;
}
#endif
if (ignoreinput)
#if defined(FEAT_MZSCHEME)
if (mzthreads_allowed() && p_mzq > 0 && msec > p_mzq)
{
int towait = p_mzq;
while (msec > 0)
{
mzvim_check_threads();
if (msec < towait)
towait = msec;
Sleep(towait);
msec -= towait;
}
}
else
#endif
Sleep((int)msec);
else
WaitForChar(msec, FALSE);
#endif
}
int
mch_remove(char_u *name)
{
WCHAR *wn;
int n;
if (mch_isdir(name) && mch_is_symbolic_link(name))
return mch_rmdir(name);
win32_setattrs(name, FILE_ATTRIBUTE_NORMAL);
wn = enc_to_utf16(name, NULL);
if (wn == NULL)
return -1;
n = DeleteFileW(wn) ? 0 : -1;
vim_free(wn);
return n;
}
void
mch_breakcheck(int force UNUSED)
{
#if !defined(FEAT_GUI_MSWIN) || defined(VIMDLL)
#if defined(VIMDLL)
if (!gui.in_use)
#endif
if (g_fCtrlCPressed || g_fCBrkPressed)
{
ctrl_break_was_pressed = g_fCBrkPressed;
g_fCtrlCPressed = g_fCBrkPressed = FALSE;
got_int = TRUE;
}
#endif
}
#define WINNT_RESERVE_BYTES (256*1024*1024)
long_u
mch_total_mem(int special UNUSED)
{
MEMORYSTATUSEX ms;
ms.dwLength = sizeof(MEMORYSTATUSEX);
GlobalMemoryStatusEx(&ms);
if (ms.ullAvailVirtual < ms.ullTotalPhys)
{
return (long_u)(ms.ullAvailVirtual / 1024);
}
if (ms.ullTotalPhys <= WINNT_RESERVE_BYTES)
{
return (long_u)((ms.ullTotalPhys / 2) / 1024);
}
return (long_u)((ms.ullTotalPhys - WINNT_RESERVE_BYTES) / 1024);
}
int
mch_wrename(WCHAR *wold, WCHAR *wnew)
{
WCHAR *p;
int i;
WCHAR szTempFile[_MAX_PATH + 1];
WCHAR szNewPath[_MAX_PATH + 1];
HANDLE hf;
p = wold;
for (i = 0; wold[i] != NUL; ++i)
if ((wold[i] == '/' || wold[i] == '\\' || wold[i] == ':')
&& wold[i + 1] != 0)
p = wold + i + 1;
if ((int)(wold + i - p) < 8 || p[6] != '~')
return (MoveFileW(wold, wnew) == 0);
if (GetFullPathNameW(wnew, _MAX_PATH, szNewPath, &p) == 0 || p == NULL)
return -1;
*p = NUL;
if (GetTempFileNameW(szNewPath, L"VIM", 0, szTempFile) == 0)
return -2;
if (!DeleteFileW(szTempFile))
return -3;
if (!MoveFileW(wold, szTempFile))
return -4;
if ((hf = CreateFileW(wold, GENERIC_WRITE, 0, NULL, CREATE_NEW,
FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
return -5;
if (!CloseHandle(hf))
return -6;
if (!MoveFileW(szTempFile, wnew))
{
(void)MoveFileW(szTempFile, wold);
return -7;
}
DeleteFileW(szTempFile);
if (!DeleteFileW(wold))
return -8;
return 0;
}
int
mch_rename(
const char *pszOldFile,
const char *pszNewFile)
{
WCHAR *wold = NULL;
WCHAR *wnew = NULL;
int retval = -1;
wold = enc_to_utf16((char_u *)pszOldFile, NULL);
wnew = enc_to_utf16((char_u *)pszNewFile, NULL);
if (wold != NULL && wnew != NULL)
retval = mch_wrename(wold, wnew);
vim_free(wold);
vim_free(wnew);
return retval;
}
char *
default_shell(void)
{
return "cmd.exe";
}
int
mch_access(char *n, int p)
{
HANDLE hFile;
int retval = -1; 
WCHAR *wn;
wn = enc_to_utf16((char_u *)n, NULL);
if (wn == NULL)
return -1;
if (mch_isdir((char_u *)n))
{
WCHAR TempNameW[_MAX_PATH + 16] = L"";
if (p & R_OK)
{
int i;
WIN32_FIND_DATAW d;
for (i = 0; i < _MAX_PATH && wn[i] != 0; ++i)
TempNameW[i] = wn[i];
if (TempNameW[i - 1] != '\\' && TempNameW[i - 1] != '/')
TempNameW[i++] = '\\';
TempNameW[i++] = '*';
TempNameW[i++] = 0;
hFile = FindFirstFileW(TempNameW, &d);
if (hFile == INVALID_HANDLE_VALUE)
goto getout;
else
(void)FindClose(hFile);
}
if (p & W_OK)
{
if (!GetTempFileNameW(wn, L"VIM", 0, TempNameW))
goto getout;
else
DeleteFileW(TempNameW);
}
}
else
{
DWORD share_mode = FILE_SHARE_READ | FILE_SHARE_WRITE;
DWORD access_mode = ((p & W_OK) ? GENERIC_WRITE : 0)
| ((p & R_OK) ? GENERIC_READ : 0);
hFile = CreateFileW(wn, access_mode, share_mode,
NULL, OPEN_EXISTING, 0, NULL);
if (hFile == INVALID_HANDLE_VALUE)
goto getout;
CloseHandle(hFile);
}
retval = 0; 
getout:
vim_free(wn);
return retval;
}
int
mch_open(const char *name, int flags, int mode)
{
WCHAR *wn;
int f;
wn = enc_to_utf16((char_u *)name, NULL);
if (wn == NULL)
return -1;
f = _wopen(wn, flags, mode);
vim_free(wn);
return f;
}
FILE *
mch_fopen(const char *name, const char *mode)
{
WCHAR *wn, *wm;
FILE *f = NULL;
#if defined(DEBUG) && _MSC_VER >= 1400
char newMode = mode[strlen(mode) - 1];
int oldMode = 0;
_get_fmode(&oldMode);
if (newMode == 't')
_set_fmode(_O_TEXT);
else if (newMode == 'b')
_set_fmode(_O_BINARY);
#endif
wn = enc_to_utf16((char_u *)name, NULL);
wm = enc_to_utf16((char_u *)mode, NULL);
if (wn != NULL && wm != NULL)
f = _wfopen(wn, wm);
vim_free(wn);
vim_free(wm);
#if defined(DEBUG) && _MSC_VER >= 1400
_set_fmode(oldMode);
#endif
return f;
}
static void
copy_substream(HANDLE sh, void *context, WCHAR *to, WCHAR *substream, long len)
{
HANDLE hTo;
WCHAR *to_name;
to_name = malloc((wcslen(to) + wcslen(substream) + 1) * sizeof(WCHAR));
wcscpy(to_name, to);
wcscat(to_name, substream);
hTo = CreateFileW(to_name, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS,
FILE_ATTRIBUTE_NORMAL, NULL);
if (hTo != INVALID_HANDLE_VALUE)
{
long done;
DWORD todo;
DWORD readcnt, written;
char buf[4096];
for (done = 0; done < len; done += written)
{
todo = (DWORD)((size_t)(len - done) > sizeof(buf) ? sizeof(buf)
: (size_t)(len - done));
if (!BackupRead(sh, (LPBYTE)buf, todo, &readcnt,
FALSE, FALSE, context)
|| readcnt != todo
|| !WriteFile(hTo, buf, todo, &written, NULL)
|| written != todo)
break;
}
CloseHandle(hTo);
}
free(to_name);
}
static void
copy_infostreams(char_u *from, char_u *to)
{
WCHAR *fromw;
WCHAR *tow;
HANDLE sh;
WIN32_STREAM_ID sid;
int headersize;
WCHAR streamname[_MAX_PATH];
DWORD readcount;
void *context = NULL;
DWORD lo, hi;
int len;
fromw = enc_to_utf16(from, NULL);
tow = enc_to_utf16(to, NULL);
if (fromw != NULL && tow != NULL)
{
sh = CreateFileW(fromw, GENERIC_READ, FILE_SHARE_READ, NULL,
OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
if (sh != INVALID_HANDLE_VALUE)
{
for (;;)
{
ZeroMemory(&sid, sizeof(WIN32_STREAM_ID));
headersize = (int)((char *)&sid.cStreamName - (char *)&sid.dwStreamId);
if (!BackupRead(sh, (LPBYTE)&sid, headersize,
&readcount, FALSE, FALSE, &context)
|| readcount == 0)
break;
if (sid.dwStreamNameSize > 0)
{
if (!BackupRead(sh, (LPBYTE)streamname,
sid.dwStreamNameSize,
&readcount, FALSE, FALSE, &context))
break;
len = readcount / sizeof(WCHAR);
streamname[len] = 0;
if (len > 7 && wcsicmp(streamname + len - 6,
L":$DATA") == 0)
{
streamname[len - 6] = 0;
copy_substream(sh, &context, tow, streamname,
(long)sid.Size.u.LowPart);
}
}
(void)BackupSeek(sh, sid.Size.u.LowPart, sid.Size.u.HighPart,
&lo, &hi, &context);
}
(void)BackupRead(sh, NULL, 0, &readcount, TRUE, FALSE, &context);
CloseHandle(sh);
}
}
vim_free(fromw);
vim_free(tow);
}
int
mch_copy_file_attribute(char_u *from, char_u *to)
{
copy_infostreams(from, to);
return 0;
}
#if defined(MYRESETSTKOFLW) || defined(PROTO)
#define MIN_STACK_WINNT 2
int
myresetstkoflw(void)
{
BYTE *pStackPtr;
BYTE *pGuardPage;
BYTE *pStackBase;
BYTE *pLowestPossiblePage;
MEMORY_BASIC_INFORMATION mbi;
SYSTEM_INFO si;
DWORD nPageSize;
DWORD dummy;
GetSystemInfo(&si);
nPageSize = si.dwPageSize;
pStackPtr = (BYTE*)_alloca(1);
if (VirtualQuery(pStackPtr, &mbi, sizeof mbi) == 0)
return 0;
pStackBase = (BYTE*)mbi.AllocationBase;
pLowestPossiblePage = pStackBase + MIN_STACK_WINNT * nPageSize;
{
BYTE *pBlock = pStackBase;
for (;;)
{
if (VirtualQuery(pBlock, &mbi, sizeof mbi) == 0)
return 0;
pBlock += mbi.RegionSize;
if (mbi.State & MEM_COMMIT)
break;
}
if (mbi.Protect & PAGE_GUARD)
return 1;
if ((long_u)(mbi.BaseAddress) < (long_u)pLowestPossiblePage)
pGuardPage = pLowestPossiblePage;
else
pGuardPage = (BYTE*)mbi.BaseAddress;
if (!VirtualAlloc(pGuardPage, nPageSize, MEM_COMMIT, PAGE_READWRITE))
return 0;
if (!VirtualProtect(pGuardPage, nPageSize, PAGE_READWRITE | PAGE_GUARD,
&dummy))
return 0;
}
return 1;
}
#endif
static int nArgsW = 0;
static LPWSTR *ArglistW = NULL;
static int global_argc = 0;
static char **global_argv;
static int used_file_argc = 0; 
static int *used_file_indexes = NULL; 
static int used_file_count = 0; 
static int used_file_literal = FALSE; 
static int used_file_full_path = FALSE; 
static int used_file_diff_mode = FALSE; 
static int used_alist_count = 0;
int
get_cmd_argsW(char ***argvp)
{
char **argv = NULL;
int argc = 0;
int i;
free_cmd_argsW();
ArglistW = CommandLineToArgvW(GetCommandLineW(), &nArgsW);
if (ArglistW != NULL)
{
argv = malloc((nArgsW + 1) * sizeof(char *));
if (argv != NULL)
{
argc = nArgsW;
argv[argc] = NULL;
for (i = 0; i < argc; ++i)
{
int len;
WideCharToMultiByte_alloc(GetACP(), 0,
ArglistW[i], (int)wcslen(ArglistW[i]) + 1,
(LPSTR *)&argv[i], &len, 0, 0);
if (argv[i] == NULL)
{
while (i > 0)
free(argv[--i]);
free(argv);
argv = NULL;
argc = 0;
}
}
}
}
global_argc = argc;
global_argv = argv;
if (argc > 0)
{
if (used_file_indexes != NULL)
free(used_file_indexes);
used_file_indexes = malloc(argc * sizeof(int));
}
if (argvp != NULL)
*argvp = argv;
return argc;
}
void
free_cmd_argsW(void)
{
if (ArglistW != NULL)
{
GlobalFree(ArglistW);
ArglistW = NULL;
}
}
void
used_file_arg(char *name, int literal, int full_path, int diff_mode)
{
int i;
if (used_file_indexes == NULL)
return;
for (i = used_file_argc + 1; i < global_argc; ++i)
if (STRCMP(global_argv[i], name) == 0)
{
used_file_argc = i;
used_file_indexes[used_file_count++] = i;
break;
}
used_file_literal = literal;
used_file_full_path = full_path;
used_file_diff_mode = diff_mode;
}
void
set_alist_count(void)
{
used_alist_count = GARGCOUNT;
}
void
fix_arg_enc(void)
{
int i;
int idx;
char_u *str;
int *fnum_list;
if (global_argc != nArgsW
|| ArglistW == NULL
|| used_file_indexes == NULL
|| used_file_count == 0
|| used_alist_count != GARGCOUNT)
return;
fnum_list = ALLOC_MULT(int, GARGCOUNT);
if (fnum_list == NULL)
return; 
for (i = 0; i < GARGCOUNT; ++i)
fnum_list[i] = GARGLIST[i].ae_fnum;
alist_clear(&global_alist);
if (ga_grow(&global_alist.al_ga, used_file_count) == FAIL)
return; 
for (i = 0; i < used_file_count; ++i)
{
idx = used_file_indexes[i];
str = utf16_to_enc(ArglistW[idx], NULL);
if (str != NULL)
{
int literal = used_file_literal;
#if defined(FEAT_DIFF)
if (used_file_diff_mode && mch_isdir(str) && GARGCOUNT > 0
&& !mch_isdir(alist_name(&GARGLIST[0])))
{
char_u *r;
r = concat_fnames(str, gettail(alist_name(&GARGLIST[0])), TRUE);
if (r != NULL)
{
vim_free(str);
str = r;
}
}
#endif
if (used_file_literal)
buf_set_name(fnum_list[i], str);
if (literal == FALSE)
{
size_t len = STRLEN(str);
if (len > 2 && *str == '`' && *(str + len - 1) == '`')
literal = TRUE;
}
alist_add(&global_alist, str, literal ? 2 : 0);
}
}
if (!used_file_literal)
{
do_cmdline_cmd((char_u *)":let SaVe_ISF = &isf|set isf+=(,)");
do_cmdline_cmd((char_u *)":let SaVe_WIG = &wig|set wig=");
alist_expand(fnum_list, used_alist_count);
do_cmdline_cmd((char_u *)":let &isf = SaVe_ISF|unlet SaVe_ISF");
do_cmdline_cmd((char_u *)":let &wig = SaVe_WIG|unlet SaVe_WIG");
}
if (curwin->w_arg_idx == 0 && curbuf->b_fname == NULL)
{
do_cmdline_cmd((char_u *)":rewind");
if (GARGCOUNT == 1 && used_file_full_path)
(void)vim_chdirfile(alist_name(&GARGLIST[0]), "drop");
}
set_alist_count();
}
int
mch_setenv(char *var, char *value, int x UNUSED)
{
char_u *envbuf;
WCHAR *p;
envbuf = alloc(STRLEN(var) + STRLEN(value) + 2);
if (envbuf == NULL)
return -1;
sprintf((char *)envbuf, "%s=%s", var, value);
p = enc_to_utf16(envbuf, NULL);
vim_free(envbuf);
if (p == NULL)
return -1;
_wputenv(p);
#if defined(libintl_wputenv)
libintl_wputenv(p);
#endif
vim_free(p);
return 0;
}
#define VTP_FIRST_SUPPORT_BUILD MAKE_VER(10, 0, 15063)
#define CONPTY_FIRST_SUPPORT_BUILD MAKE_VER(10, 0, 17763)
#define CONPTY_1903_BUILD MAKE_VER(10, 0, 18362)
#define CONPTY_1909_BUILD MAKE_VER(10, 0, 18363)
#define CONPTY_INSIDER_BUILD MAKE_VER(10, 0, 18995)
#define CONPTY_STABLE_BUILD MAKE_VER(10, 0, 32767) 
static void
vtp_flag_init(void)
{
DWORD ver = get_build_number();
#if !defined(FEAT_GUI_MSWIN) || defined(VIMDLL)
DWORD mode;
HANDLE out;
#if defined(VIMDLL)
if (!gui.in_use)
#endif
{
out = GetStdHandle(STD_OUTPUT_HANDLE);
vtp_working = (ver >= VTP_FIRST_SUPPORT_BUILD) ? 1 : 0;
GetConsoleMode(out, &mode);
mode |= (ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
if (SetConsoleMode(out, mode) == 0)
vtp_working = 0;
}
#endif
if (ver >= CONPTY_FIRST_SUPPORT_BUILD)
conpty_working = 1;
if (ver >= CONPTY_STABLE_BUILD)
conpty_stable = 1;
if (ver <= CONPTY_INSIDER_BUILD)
conpty_type = 3;
if (ver <= CONPTY_1909_BUILD)
conpty_type = 2;
if (ver <= CONPTY_1903_BUILD)
conpty_type = 2;
if (ver < CONPTY_FIRST_SUPPORT_BUILD)
conpty_type = 1;
}
#if !defined(FEAT_GUI_MSWIN) || defined(VIMDLL) || defined(PROTO)
static void
vtp_init(void)
{
HMODULE hKerneldll;
DYN_CONSOLE_SCREEN_BUFFER_INFOEX csbi;
#if defined(FEAT_TERMGUICOLORS)
COLORREF fg, bg;
#endif
hKerneldll = GetModuleHandle("kernel32.dll");
if (hKerneldll != NULL)
{
pGetConsoleScreenBufferInfoEx =
(PfnGetConsoleScreenBufferInfoEx)GetProcAddress(
hKerneldll, "GetConsoleScreenBufferInfoEx");
pSetConsoleScreenBufferInfoEx =
(PfnSetConsoleScreenBufferInfoEx)GetProcAddress(
hKerneldll, "SetConsoleScreenBufferInfoEx");
if (pGetConsoleScreenBufferInfoEx != NULL
&& pSetConsoleScreenBufferInfoEx != NULL)
has_csbiex = TRUE;
}
csbi.cbSize = sizeof(csbi);
if (has_csbiex)
pGetConsoleScreenBufferInfoEx(g_hConOut, &csbi);
save_console_bg_rgb = (guicolor_T)csbi.ColorTable[g_color_index_bg];
save_console_fg_rgb = (guicolor_T)csbi.ColorTable[g_color_index_fg];
store_console_bg_rgb = save_console_bg_rgb;
store_console_fg_rgb = save_console_fg_rgb;
#if defined(FEAT_TERMGUICOLORS)
bg = (COLORREF)csbi.ColorTable[g_color_index_bg];
fg = (COLORREF)csbi.ColorTable[g_color_index_fg];
bg = (GetRValue(bg) << 16) | (GetGValue(bg) << 8) | GetBValue(bg);
fg = (GetRValue(fg) << 16) | (GetGValue(fg) << 8) | GetBValue(fg);
default_console_color_bg = bg;
default_console_color_fg = fg;
#endif
set_console_color_rgb();
}
static void
vtp_exit(void)
{
restore_console_color_rgb();
}
static int
vtp_printf(
char *format,
...)
{
char_u buf[100];
va_list list;
DWORD result;
va_start(list, format);
vim_vsnprintf((char *)buf, 100, (char *)format, list);
va_end(list);
WriteConsoleA(g_hConOut, buf, (DWORD)STRLEN(buf), &result, NULL);
return (int)result;
}
static void
vtp_sgr_bulk(
int arg)
{
int args[1];
args[0] = arg;
vtp_sgr_bulks(1, args);
}
static void
vtp_sgr_bulks(
int argc,
int *args
)
{
char_u buf[2 + (4 * 16) + 1];
char_u *p;
int i;
p = buf;
*p++ = '\033';
*p++ = '[';
for (i = 0; i < argc; ++i)
{
p += vim_snprintf((char *)p, 4, "%d", args[i] & 0xff);
*p++ = ';';
}
p--;
*p++ = 'm';
*p = NUL;
vtp_printf((char *)buf);
}
#if defined(FEAT_TERMGUICOLORS)
static int
ctermtoxterm(
int cterm)
{
char_u r, g, b, idx;
cterm_color2rgb(cterm, &r, &g, &b, &idx);
return (((int)r << 16) | ((int)g << 8) | (int)b);
}
#endif
static void
set_console_color_rgb(void)
{
#if defined(FEAT_TERMGUICOLORS)
DYN_CONSOLE_SCREEN_BUFFER_INFOEX csbi;
guicolor_T fg, bg;
int ctermfg, ctermbg;
if (!USE_VTP)
return;
get_default_console_color(&ctermfg, &ctermbg, &fg, &bg);
fg = (GetRValue(fg) << 16) | (GetGValue(fg) << 8) | GetBValue(fg);
bg = (GetRValue(bg) << 16) | (GetGValue(bg) << 8) | GetBValue(bg);
csbi.cbSize = sizeof(csbi);
if (has_csbiex)
pGetConsoleScreenBufferInfoEx(g_hConOut, &csbi);
csbi.cbSize = sizeof(csbi);
csbi.srWindow.Right += 1;
csbi.srWindow.Bottom += 1;
store_console_bg_rgb = csbi.ColorTable[g_color_index_bg];
store_console_fg_rgb = csbi.ColorTable[g_color_index_fg];
csbi.ColorTable[g_color_index_bg] = (COLORREF)bg;
csbi.ColorTable[g_color_index_fg] = (COLORREF)fg;
if (has_csbiex)
pSetConsoleScreenBufferInfoEx(g_hConOut, &csbi);
#endif
}
#if defined(FEAT_TERMGUICOLORS) || defined(PROTO)
void
get_default_console_color(
int *cterm_fg,
int *cterm_bg,
guicolor_T *gui_fg,
guicolor_T *gui_bg)
{
int id;
guicolor_T guifg = INVALCOLOR;
guicolor_T guibg = INVALCOLOR;
int ctermfg = 0;
int ctermbg = 0;
id = syn_name2id((char_u *)"Normal");
if (id > 0 && p_tgc)
syn_id2colors(id, &guifg, &guibg);
if (guifg == INVALCOLOR)
{
ctermfg = -1;
if (id > 0)
syn_id2cterm_bg(id, &ctermfg, &ctermbg);
guifg = ctermfg != -1 ? ctermtoxterm(ctermfg)
: default_console_color_fg;
cterm_normal_fg_gui_color = guifg;
ctermfg = ctermfg < 0 ? 0 : ctermfg;
}
if (guibg == INVALCOLOR)
{
ctermbg = -1;
if (id > 0)
syn_id2cterm_bg(id, &ctermfg, &ctermbg);
guibg = ctermbg != -1 ? ctermtoxterm(ctermbg)
: default_console_color_bg;
cterm_normal_bg_gui_color = guibg;
ctermbg = ctermbg < 0 ? 0 : ctermbg;
}
*cterm_fg = ctermfg;
*cterm_bg = ctermbg;
*gui_fg = guifg;
*gui_bg = guibg;
}
#endif
static void
reset_console_color_rgb(void)
{
#if defined(FEAT_TERMGUICOLORS)
DYN_CONSOLE_SCREEN_BUFFER_INFOEX csbi;
csbi.cbSize = sizeof(csbi);
if (has_csbiex)
pGetConsoleScreenBufferInfoEx(g_hConOut, &csbi);
csbi.cbSize = sizeof(csbi);
csbi.srWindow.Right += 1;
csbi.srWindow.Bottom += 1;
csbi.ColorTable[g_color_index_bg] = (COLORREF)store_console_bg_rgb;
csbi.ColorTable[g_color_index_fg] = (COLORREF)store_console_fg_rgb;
if (has_csbiex)
pSetConsoleScreenBufferInfoEx(g_hConOut, &csbi);
#endif
}
static void
restore_console_color_rgb(void)
{
#if defined(FEAT_TERMGUICOLORS)
DYN_CONSOLE_SCREEN_BUFFER_INFOEX csbi;
csbi.cbSize = sizeof(csbi);
if (has_csbiex)
pGetConsoleScreenBufferInfoEx(g_hConOut, &csbi);
csbi.cbSize = sizeof(csbi);
csbi.srWindow.Right += 1;
csbi.srWindow.Bottom += 1;
csbi.ColorTable[g_color_index_bg] = (COLORREF)save_console_bg_rgb;
csbi.ColorTable[g_color_index_fg] = (COLORREF)save_console_fg_rgb;
if (has_csbiex)
pSetConsoleScreenBufferInfoEx(g_hConOut, &csbi);
#endif
}
void
control_console_color_rgb(void)
{
if (USE_VTP)
set_console_color_rgb();
else
reset_console_color_rgb();
}
int
use_vtp(void)
{
return USE_VTP;
}
int
is_term_win32(void)
{
return T_NAME != NULL && STRCMP(T_NAME, "win32") == 0;
}
int
has_vtp_working(void)
{
return vtp_working;
}
#endif
int
has_conpty_working(void)
{
return conpty_working;
}
int
get_conpty_type(void)
{
return conpty_type;
}
int
is_conpty_stable(void)
{
return conpty_stable;
}
#if !defined(FEAT_GUI_MSWIN) || defined(VIMDLL) || defined(PROTO)
void
resize_console_buf(void)
{
CONSOLE_SCREEN_BUFFER_INFO csbi;
COORD coord;
SMALL_RECT newsize;
if (GetConsoleScreenBufferInfo(g_hConOut, &csbi))
{
coord.X = SRWIDTH(csbi.srWindow);
coord.Y = SRHEIGHT(csbi.srWindow);
SetConsoleScreenBufferSize(g_hConOut, coord);
newsize.Left = 0;
newsize.Top = 0;
newsize.Right = coord.X - 1;
newsize.Bottom = coord.Y - 1;
SetConsoleWindowInfo(g_hConOut, TRUE, &newsize);
SetConsoleScreenBufferSize(g_hConOut, coord);
}
}
#endif
