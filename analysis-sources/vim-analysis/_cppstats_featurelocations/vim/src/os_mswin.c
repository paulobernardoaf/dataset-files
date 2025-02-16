














#include "vim.h"

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

#if defined(FEAT_PRINTER) && !defined(FEAT_POSTSCRIPT)
#include <dlgs.h>
#include <winspool.h>
#include <commdlg.h>
#endif

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





#if defined(PROTO)
#define WINAPI
#define WINBASEAPI
typedef int BOOL;
typedef int CALLBACK;
typedef int COLORREF;
typedef int CONSOLE_CURSOR_INFO;
typedef int COORD;
typedef int DWORD;
typedef int ENUMLOGFONTW;
typedef int HANDLE;
typedef int HDC;
typedef int HFONT;
typedef int HICON;
typedef int HWND;
typedef int INPUT_RECORD;
typedef int KEY_EVENT_RECORD;
typedef int LOGFONTW;
typedef int LPARAM;
typedef int LPBOOL;
typedef int LPCSTR;
typedef int LPCWSTR;
typedef int LPDWORD;
typedef int LPSTR;
typedef int LPTSTR;
typedef int LPVOID;
typedef int LPWSTR;
typedef int LRESULT;
typedef int MOUSE_EVENT_RECORD;
typedef int NEWTEXTMETRICW;
typedef int PACL;
typedef int PRINTDLGW;
typedef int PSECURITY_DESCRIPTOR;
typedef int PSID;
typedef int SECURITY_INFORMATION;
typedef int SHORT;
typedef int SMALL_RECT;
typedef int TEXTMETRIC;
typedef int UINT;
typedef int WCHAR;
typedef int WNDENUMPROC;
typedef int WORD;
typedef int WPARAM;
typedef void VOID;
#endif




#if defined(MCH_WRITE_DUMP)
FILE* fdDump = NULL;
#endif

#if !defined(FEAT_GUI_MSWIN) || defined(VIMDLL)
extern char g_szOrigTitle[];
#endif

#if defined(FEAT_GUI)
extern HWND s_hwnd;
#else
static HWND s_hwnd = 0; 
#endif

#if defined(FEAT_JOB_CHANNEL)
int WSInitialized = FALSE; 
#endif



#if defined(__GNUC__) && !defined(PROTO)
#if !defined(__MINGW32__)
int _stricoll(char *a, char *b)
{

char a_buff[512], b_buff[512]; 
strxfrm(a_buff, a, 512);
strxfrm(b_buff, b, 512);
return strcoll(a_buff, b_buff);
}

char * _fullpath(char *buf, char *fname, int len)
{
LPTSTR toss;

return (char *)GetFullPathName(fname, len, buf, &toss);
}
#endif

#if !defined(__MINGW32__) || (__GNUC__ < 4)
int _chdrive(int drive)
{
char temp [3] = "-:";
temp[0] = drive + 'A' - 1;
return !SetCurrentDirectory(temp);
}
#endif
#endif


#if !defined(PROTO)



void
SaveInst(HINSTANCE hInst)
{
g_hinst = hInst;
}
#endif

#if defined(FEAT_GUI_MSWIN) || defined(PROTO)





void
mch_exit_g(int r)
{
exiting = TRUE;

display_errors();

ml_close_all(TRUE); 

#if defined(FEAT_OLE)
UninitOLE();
#endif
#if defined(FEAT_JOB_CHANNEL)
if (WSInitialized)
{
WSInitialized = FALSE;
WSACleanup();
}
#endif
#if defined(DYNAMIC_GETTEXT)
dyn_libintl_end();
#endif

if (gui.in_use)
gui_exit(r);

#if defined(EXITFREE)
free_all_mem();
#endif

exit(r);
}

#endif 





void
mch_early_init(void)
{
int i;

PlatformId();


for (i = 0; i < 256; ++i)
toupper_tab[i] = tolower_tab[i] = i;
CharUpperBuff((LPSTR)toupper_tab, 256);
CharLowerBuff((LPSTR)tolower_tab, 256);
}





int
mch_input_isatty(void)
{
#if defined(FEAT_GUI_MSWIN)
#if defined(VIMDLL)
if (gui.in_use)
#endif
return TRUE; 
#endif
#if !defined(FEAT_GUI_MSWIN) || defined(VIMDLL)
if (isatty(read_cmd_fd))
return TRUE;
return FALSE;
#endif
}

#if defined(FEAT_TITLE)



void
mch_settitle(
char_u *title,
char_u *icon UNUSED)
{
#if defined(FEAT_GUI_MSWIN)
#if defined(VIMDLL)
if (gui.in_use)
#endif
{
gui_mch_settitle(title, icon);
return;
}
#endif
#if !defined(FEAT_GUI_MSWIN) || defined(VIMDLL)
if (title != NULL)
{
WCHAR *wp = enc_to_utf16(title, NULL);

if (wp == NULL)
return;

SetConsoleTitleW(wp);
vim_free(wp);
return;
}
#endif
}









void
mch_restore_title(int which UNUSED)
{
#if !defined(FEAT_GUI_MSWIN) || defined(VIMDLL)
#if defined(VIMDLL)
if (!gui.in_use)
#endif
SetConsoleTitle(g_szOrigTitle);
#endif
}





int
mch_can_restore_title(void)
{
return TRUE;
}





int
mch_can_restore_icon(void)
{
return FALSE;
}
#endif 









int
mch_FullName(
char_u *fname,
char_u *buf,
int len,
int force UNUSED)
{
int nResult = FAIL;
WCHAR *wname;
WCHAR wbuf[MAX_PATH];
char_u *cname = NULL;

wname = enc_to_utf16(fname, NULL);
if (wname != NULL && _wfullpath(wbuf, wname, MAX_PATH) != NULL)
{
cname = utf16_to_enc((short_u *)wbuf, NULL);
if (cname != NULL)
{
vim_strncpy(buf, cname, len - 1);
nResult = OK;
}
}
vim_free(wname);
vim_free(cname);

#if defined(USE_FNAME_CASE)
fname_case(buf, len);
#else
slash_adjust(buf);
#endif

return nResult;
}





int
mch_isFullName(char_u *fname)
{



char szName[_MAX_PATH * 3 + 1];


if ((fname[0] && fname[1] == ':' && (fname[2] == '/' || fname[2] == '\\'))
|| (fname[0] == fname[1] && (fname[0] == '/' || fname[0] == '\\')))
return TRUE;


if (mch_FullName(fname, (char_u *)szName, sizeof(szName) - 1, FALSE) == FAIL)
return FALSE;

return pathcmp((const char *)fname, (const char *)szName, -1) == 0;
}











void
slash_adjust(char_u *p)
{
if (path_with_url(p))
return;

if (*p == '`')
{
size_t len = STRLEN(p);


if (len > 2 && *(p + len - 1) == '`')
return;
}

while (*p)
{
if (*p == psepcN)
*p = psepc;
MB_PTR_ADV(p);
}
}


#if defined(HAVE_STAT64)
#undef stat
#undef _stat
#undef _wstat
#undef _fstat
#define stat _stat64
#define _stat _stat64
#define _wstat _wstat64
#define _fstat _fstat64
#endif

#if (defined(_MSC_VER) && (_MSC_VER >= 1300)) || defined(__MINGW32__)
#define OPEN_OH_ARGTYPE intptr_t
#else
#define OPEN_OH_ARGTYPE long
#endif

static int
wstat_symlink_aware(const WCHAR *name, stat_T *stp)
{
#if (defined(_MSC_VER) && (_MSC_VER < 1900)) || defined(__MINGW32__)









int n;
BOOL is_symlink = FALSE;
HANDLE hFind, h;
DWORD attr = 0;
WIN32_FIND_DATAW findDataW;

hFind = FindFirstFileW(name, &findDataW);
if (hFind != INVALID_HANDLE_VALUE)
{
attr = findDataW.dwFileAttributes;
if ((attr & FILE_ATTRIBUTE_REPARSE_POINT)
&& (findDataW.dwReserved0 == IO_REPARSE_TAG_SYMLINK))
is_symlink = TRUE;
FindClose(hFind);
}
if (is_symlink)
{
h = CreateFileW(name, FILE_READ_ATTRIBUTES,
FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
OPEN_EXISTING,
(attr & FILE_ATTRIBUTE_DIRECTORY)
? FILE_FLAG_BACKUP_SEMANTICS : 0,
NULL);
if (h != INVALID_HANDLE_VALUE)
{
int fd;

fd = _open_osfhandle((OPEN_OH_ARGTYPE)h, _O_RDONLY);
n = _fstat(fd, (struct _stat *)stp);
if ((n == 0) && (attr & FILE_ATTRIBUTE_DIRECTORY))
stp->st_mode = (stp->st_mode & ~S_IFREG) | S_IFDIR;
_close(fd);
return n;
}
}
#endif
return _wstat(name, (struct _stat *)stp);
}




int
vim_stat(const char *name, stat_T *stp)
{



char_u buf[_MAX_PATH * 3 + 1];
char_u *p;
WCHAR *wp;
int n;

vim_strncpy((char_u *)buf, (char_u *)name, sizeof(buf) - 1);
p = buf + STRLEN(buf);
if (p > buf)
MB_PTR_BACK(buf, p);


if (p > buf && (*p == '\\' || *p == '/') && p[-1] != ':')
*p = NUL;

if ((buf[0] == '\\' && buf[1] == '\\') || (buf[0] == '/' && buf[1] == '/'))
{

p = vim_strpbrk(buf + 2, (char_u *)"\\/");
if (p != NULL)
{
p = vim_strpbrk(p + 1, (char_u *)"\\/");
if (p == NULL)
STRCAT(buf, "\\");
}
}

wp = enc_to_utf16(buf, NULL);
if (wp == NULL)
return -1;

n = wstat_symlink_aware(wp, stp);
vim_free(wp);
return n;
}

#if (defined(FEAT_GUI_MSWIN) && !defined(VIMDLL)) || defined(PROTO)
void
mch_settmode(int tmode UNUSED)
{

}

int
mch_get_shellsize(void)
{

return OK;
}

void
mch_set_shellsize(void)
{

}




void
mch_new_shellsize(void)
{

}

#endif




void
mch_suspend(void)
{
suspend_shell();
}

#if defined(USE_MCH_ERRMSG) || defined(PROTO)

#if defined(display_errors)
#undef display_errors
#endif




void
display_errors(void)
{
#if defined(FEAT_GUI)
char *p;

#if defined(VIMDLL)
if (gui.in_use || gui.starting)
#endif
{
if (error_ga.ga_data != NULL)
{

for (p = (char *)error_ga.ga_data; *p; ++p)
if (!isspace(*p))
{
(void)gui_mch_dialog(
gui.starting ? VIM_INFO :
VIM_ERROR,
gui.starting ? (char_u *)_("Message") :
(char_u *)_("Error"),
(char_u *)p, (char_u *)_("&Ok"),
1, NULL, FALSE);
break;
}
ga_clear(&error_ga);
}
return;
}
#endif
#if !defined(FEAT_GUI) || defined(VIMDLL)
FlushFileBuffers(GetStdHandle(STD_ERROR_HANDLE));
#endif
}
#endif






int
mch_has_exp_wildcard(char_u *p)
{
for ( ; *p; MB_PTR_ADV(p))
{
if (vim_strchr((char_u *)"?*[", *p) != NULL
|| (*p == '~' && p[1] != NUL))
return TRUE;
}
return FALSE;
}





int
mch_has_wildcard(char_u *p)
{
for ( ; *p; MB_PTR_ADV(p))
{
if (vim_strchr((char_u *)
#if defined(VIM_BACKTICK)
"?*$[`"
#else
"?*$["
#endif
, *p) != NULL
|| (*p == '~' && p[1] != NUL))
return TRUE;
}
return FALSE;
}






int
mch_chdir(char *path)
{
WCHAR *p;
int n;

if (path[0] == NUL) 
return -1;

if (p_verbose >= 5)
{
verbose_enter();
smsg("chdir(%s)", path);
verbose_leave();
}
if (isalpha(path[0]) && path[1] == ':') 
{



if (_chdrive(TOLOWER_ASC(path[0]) - 'a' + 1) == 0)
path += 2;
}

if (*path == NUL) 
return 0;

p = enc_to_utf16((char_u *)path, NULL);
if (p == NULL)
return -1;

n = _wchdir(p);
vim_free(p);
return n;
}


#if defined(FEAT_GUI_MSWIN) && !defined(VIMDLL)



int
mch_char_avail(void)
{

return TRUE;
}

#if defined(FEAT_TERMINAL) || defined(PROTO)



int
mch_check_messages(void)
{

return TRUE;
}
#endif
#endif


#if defined(FEAT_LIBCALL) || defined(PROTO)





typedef LPTSTR (*MYSTRPROCSTR)(LPTSTR);
typedef LPTSTR (*MYINTPROCSTR)(int);
typedef int (*MYSTRPROCINT)(LPTSTR);
typedef int (*MYINTPROCINT)(int);






static size_t
check_str_len(char_u *str)
{
SYSTEM_INFO si;
MEMORY_BASIC_INFORMATION mbi;
size_t length = 0;
size_t i;
const char_u *p;


GetSystemInfo(&si);


if (VirtualQuery(str, &mbi, sizeof(mbi)))
{

long_u dwStr = (long_u)str;
long_u dwBaseAddress = (long_u)mbi.BaseAddress;


long_u strPage = dwStr - (dwStr - dwBaseAddress) % si.dwPageSize;


long_u pageLength = si.dwPageSize - (dwStr - strPage);

for (p = str; !IsBadReadPtr(p, (UINT)pageLength);
p += pageLength, pageLength = si.dwPageSize)
for (i = 0; i < pageLength; ++i, ++length)
if (p[i] == NUL)
return length + 1;
}

return 0;
}




static void
mch_icon_load_cb(char_u *fname, void *cookie)
{
HANDLE *h = (HANDLE *)cookie;

*h = LoadImage(NULL,
(LPSTR)fname,
IMAGE_ICON,
64,
64,
LR_LOADFROMFILE | LR_LOADMAP3DCOLORS);
}




int
mch_icon_load(HANDLE *iconp)
{
return do_in_runtimepath((char_u *)"bitmaps/vim.ico",
0, mch_icon_load_cb, iconp);
}

int
mch_libcall(
char_u *libname,
char_u *funcname,
char_u *argstring, 
int argint,
char_u **string_result,
int *number_result)
{
HINSTANCE hinstLib;
MYSTRPROCSTR ProcAdd;
MYINTPROCSTR ProcAddI;
char_u *retval_str = NULL;
int retval_int = 0;
size_t len;

BOOL fRunTimeLinkSuccess = FALSE;


hinstLib = vimLoadLib((char *)libname);


if (hinstLib != NULL)
{
#if defined(HAVE_TRY_EXCEPT)
__try
{
#endif
if (argstring != NULL)
{

ProcAdd = (MYSTRPROCSTR)GetProcAddress(hinstLib, (LPCSTR)funcname);
if ((fRunTimeLinkSuccess = (ProcAdd != NULL)) != 0)
{
if (string_result == NULL)
retval_int = ((MYSTRPROCINT)ProcAdd)((LPSTR)argstring);
else
retval_str = (char_u *)(ProcAdd)((LPSTR)argstring);
}
}
else
{

ProcAddI = (MYINTPROCSTR) GetProcAddress(hinstLib, (LPCSTR)funcname);
if ((fRunTimeLinkSuccess = (ProcAddI != NULL)) != 0)
{
if (string_result == NULL)
retval_int = ((MYINTPROCINT)ProcAddI)(argint);
else
retval_str = (char_u *)(ProcAddI)(argint);
}
}



if (string_result == NULL)
*number_result = retval_int;
else if (retval_str != NULL
&& (len = check_str_len(retval_str)) > 0)
{
*string_result = alloc(len);
if (*string_result != NULL)
mch_memmove(*string_result, retval_str, len);
}

#if defined(HAVE_TRY_EXCEPT)
}
__except(EXCEPTION_EXECUTE_HANDLER)
{
if (GetExceptionCode() == EXCEPTION_STACK_OVERFLOW)
RESETSTKOFLW();
fRunTimeLinkSuccess = 0;
}
#endif


(void)FreeLibrary(hinstLib);
}

if (!fRunTimeLinkSuccess)
{
semsg(_(e_libcall), funcname);
return FAIL;
}

return OK;
}
#endif




void
DumpPutS(const char *psz UNUSED)
{
#if defined(MCH_WRITE_DUMP)
if (fdDump)
{
fputs(psz, fdDump);
if (psz[strlen(psz) - 1] != '\n')
fputc('\n', fdDump);
fflush(fdDump);
}
#endif
}

#if defined(_DEBUG)

void __cdecl
Trace(
char *pszFormat,
...)
{
CHAR szBuff[2048];
va_list args;

va_start(args, pszFormat);
vsprintf(szBuff, pszFormat, args);
va_end(args);

OutputDebugString(szBuff);
}

#endif 

#if !defined(FEAT_GUI) || defined(VIMDLL) || defined(PROTO)
#if defined(FEAT_TITLE)
extern HWND g_hWnd; 
#endif






static void
GetConsoleHwnd(void)
{

if (s_hwnd != 0)
return;

#if defined(FEAT_TITLE)

if (g_hWnd != 0)
{
s_hwnd = g_hWnd;
return;
}
#endif

s_hwnd = GetConsoleWindow();
}




int
mch_get_winpos(int *x, int *y)
{
RECT rect;

GetConsoleHwnd();
GetWindowRect(s_hwnd, &rect);
*x = rect.left;
*y = rect.top;
return OK;
}




void
mch_set_winpos(int x, int y)
{
GetConsoleHwnd();
SetWindowPos(s_hwnd, NULL, x, y, 0, 0,
SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
}
#endif

#if (defined(FEAT_PRINTER) && !defined(FEAT_POSTSCRIPT)) || defined(PROTO)




static HFONT prt_font_handles[2][2][2];
static PRINTDLGW prt_dlg;
static const int boldface[2] = {FW_REGULAR, FW_BOLD};
static TEXTMETRIC prt_tm;
static int prt_line_height;
static int prt_number_width;
static int prt_left_margin;
static int prt_right_margin;
static int prt_top_margin;
static char_u szAppName[] = TEXT("VIM");
static HWND hDlgPrint;
static int *bUserAbort = NULL;
static char_u *prt_name = NULL;


#define IDC_BOX1 400
#define IDC_PRINTTEXT1 401
#define IDC_PRINTTEXT2 402
#define IDC_PROGRESS 403

static BOOL
vimSetDlgItemText(HWND hDlg, int nIDDlgItem, char_u *s)
{
WCHAR *wp;
BOOL ret;

wp = enc_to_utf16(s, NULL);
if (wp == NULL)
return FALSE;

ret = SetDlgItemTextW(hDlg, nIDDlgItem, wp);
vim_free(wp);
return ret;
}




static COLORREF
swap_me(COLORREF colorref)
{
int temp;
char *ptr = (char *)&colorref;

temp = *(ptr);
*(ptr ) = *(ptr + 2);
*(ptr + 2) = temp;
return colorref;
}


#if !defined(_WIN64) && (!defined(_MSC_VER) || _MSC_VER < 1300)
#define PDP_RETVAL BOOL
#else
#define PDP_RETVAL INT_PTR
#endif

static PDP_RETVAL CALLBACK
PrintDlgProc(
HWND hDlg,
UINT message,
WPARAM wParam UNUSED,
LPARAM lParam UNUSED)
{
#if defined(FEAT_GETTEXT)
NONCLIENTMETRICS nm;
static HFONT hfont;
#endif

switch (message)
{
case WM_INITDIALOG:
#if defined(FEAT_GETTEXT)
nm.cbSize = sizeof(NONCLIENTMETRICS);
if (SystemParametersInfo(
SPI_GETNONCLIENTMETRICS,
sizeof(NONCLIENTMETRICS),
&nm,
0))
{
char buff[MAX_PATH];
int i;


hfont = CreateFontIndirect(&nm.lfMessageFont);
for (i = IDC_PRINTTEXT1; i <= IDC_PROGRESS; i++)
{
SendDlgItemMessage(hDlg, i, WM_SETFONT, (WPARAM)hfont, 1);
if (GetDlgItemText(hDlg,i, buff, sizeof(buff)))
vimSetDlgItemText(hDlg,i, (char_u *)_(buff));
}
SendDlgItemMessage(hDlg, IDCANCEL,
WM_SETFONT, (WPARAM)hfont, 1);
if (GetDlgItemText(hDlg,IDCANCEL, buff, sizeof(buff)))
vimSetDlgItemText(hDlg,IDCANCEL, (char_u *)_(buff));
}
#endif
SetWindowText(hDlg, (LPCSTR)szAppName);
if (prt_name != NULL)
{
vimSetDlgItemText(hDlg, IDC_PRINTTEXT2, (char_u *)prt_name);
VIM_CLEAR(prt_name);
}
EnableMenuItem(GetSystemMenu(hDlg, FALSE), SC_CLOSE, MF_GRAYED);
#if !defined(FEAT_GUI) || defined(VIMDLL)
#if defined(VIMDLL)
if (!gui.in_use)
#endif
BringWindowToTop(s_hwnd);
#endif
return TRUE;

case WM_COMMAND:
*bUserAbort = TRUE;
EnableWindow(GetParent(hDlg), TRUE);
DestroyWindow(hDlg);
hDlgPrint = NULL;
#if defined(FEAT_GETTEXT)
DeleteObject(hfont);
#endif
return TRUE;
}
return FALSE;
}

static BOOL CALLBACK
AbortProc(HDC hdcPrn UNUSED, int iCode UNUSED)
{
MSG msg;

while (!*bUserAbort && pPeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
{
if (!hDlgPrint || !pIsDialogMessage(hDlgPrint, &msg))
{
TranslateMessage(&msg);
pDispatchMessage(&msg);
}
}
return !*bUserAbort;
}

#if !defined(FEAT_GUI) || defined(VIMDLL)

static UINT_PTR CALLBACK
PrintHookProc(
HWND hDlg, 
UINT uiMsg, 
WPARAM wParam UNUSED, 
LPARAM lParam 
)
{
HWND hwndOwner;
RECT rc, rcDlg, rcOwner;
PRINTDLGW *pPD;

if (uiMsg == WM_INITDIALOG)
{

if ((hwndOwner = GetParent(hDlg)) == NULL)
hwndOwner = GetDesktopWindow();

GetWindowRect(hwndOwner, &rcOwner);
GetWindowRect(hDlg, &rcDlg);
CopyRect(&rc, &rcOwner);






OffsetRect(&rcDlg, -rcDlg.left, -rcDlg.top);
OffsetRect(&rc, -rc.left, -rc.top);
OffsetRect(&rc, -rcDlg.right, -rcDlg.bottom);




SetWindowPos(hDlg,
HWND_TOP,
rcOwner.left + (rc.right / 2),
rcOwner.top + (rc.bottom / 2),
0, 0, 
SWP_NOSIZE);


pPD = (PRINTDLGW *)lParam;
pPD->nCopies = (WORD)pPD->lCustData;
SetDlgItemInt( hDlg, edt3, pPD->nCopies, FALSE );

BringWindowToTop(GetParent(hDlg));
SetForegroundWindow(hDlg);
}

return FALSE;
}
#endif

void
mch_print_cleanup(void)
{
int pifItalic;
int pifBold;
int pifUnderline;

for (pifBold = 0; pifBold <= 1; pifBold++)
for (pifItalic = 0; pifItalic <= 1; pifItalic++)
for (pifUnderline = 0; pifUnderline <= 1; pifUnderline++)
DeleteObject(prt_font_handles[pifBold][pifItalic][pifUnderline]);

if (prt_dlg.hDC != NULL)
DeleteDC(prt_dlg.hDC);
if (!*bUserAbort)
SendMessage(hDlgPrint, WM_COMMAND, 0, 0);
}

static int
to_device_units(int idx, int dpi, int physsize, int offset, int def_number)
{
int ret = 0;
int u;
int nr;

u = prt_get_unit(idx);
if (u == PRT_UNIT_NONE)
{
u = PRT_UNIT_PERC;
nr = def_number;
}
else
nr = printer_opts[idx].number;

switch (u)
{
case PRT_UNIT_PERC:
ret = (physsize * nr) / 100;
break;
case PRT_UNIT_INCH:
ret = (nr * dpi);
break;
case PRT_UNIT_MM:
ret = (nr * 10 * dpi) / 254;
break;
case PRT_UNIT_POINT:
ret = (nr * 10 * dpi) / 720;
break;
}

if (ret < offset)
return 0;
else
return ret - offset;
}

static int
prt_get_cpl(void)
{
int hr;
int phyw;
int dvoff;
int rev_offset;
int dpi;

GetTextMetrics(prt_dlg.hDC, &prt_tm);
prt_line_height = prt_tm.tmHeight + prt_tm.tmExternalLeading;

hr = GetDeviceCaps(prt_dlg.hDC, HORZRES);
phyw = GetDeviceCaps(prt_dlg.hDC, PHYSICALWIDTH);
dvoff = GetDeviceCaps(prt_dlg.hDC, PHYSICALOFFSETX);
dpi = GetDeviceCaps(prt_dlg.hDC, LOGPIXELSX);

rev_offset = phyw - (dvoff + hr);

prt_left_margin = to_device_units(OPT_PRINT_LEFT, dpi, phyw, dvoff, 10);
if (prt_use_number())
{
prt_number_width = PRINT_NUMBER_WIDTH * prt_tm.tmAveCharWidth;
prt_left_margin += prt_number_width;
}
else
prt_number_width = 0;

prt_right_margin = hr - to_device_units(OPT_PRINT_RIGHT, dpi, phyw,
rev_offset, 5);

return (prt_right_margin - prt_left_margin) / prt_tm.tmAveCharWidth;
}

static int
prt_get_lpp(void)
{
int vr;
int phyw;
int dvoff;
int rev_offset;
int bottom_margin;
int dpi;

vr = GetDeviceCaps(prt_dlg.hDC, VERTRES);
phyw = GetDeviceCaps(prt_dlg.hDC, PHYSICALHEIGHT);
dvoff = GetDeviceCaps(prt_dlg.hDC, PHYSICALOFFSETY);
dpi = GetDeviceCaps(prt_dlg.hDC, LOGPIXELSY);

rev_offset = phyw - (dvoff + vr);

prt_top_margin = to_device_units(OPT_PRINT_TOP, dpi, phyw, dvoff, 5);


prt_top_margin += prt_line_height * prt_header_height();

bottom_margin = vr - to_device_units(OPT_PRINT_BOT, dpi, phyw,
rev_offset, 5);

return (bottom_margin - prt_top_margin) / prt_line_height;
}

int
mch_print_init(prt_settings_T *psettings, char_u *jobname, int forceit)
{
static HGLOBAL stored_dm = NULL;
static HGLOBAL stored_devn = NULL;
static int stored_nCopies = 1;
static int stored_nFlags = 0;

LOGFONTW fLogFont;
int pifItalic;
int pifBold;
int pifUnderline;

DEVMODEW *mem;
DEVNAMES *devname;
int i;

bUserAbort = &(psettings->user_abort);
vim_memset(&prt_dlg, 0, sizeof(PRINTDLGW));
prt_dlg.lStructSize = sizeof(PRINTDLGW);
#if !defined(FEAT_GUI) || defined(VIMDLL)
#if defined(VIMDLL)
if (!gui.in_use)
#endif
GetConsoleHwnd(); 
#endif
prt_dlg.hwndOwner = s_hwnd;
prt_dlg.Flags = PD_NOPAGENUMS | PD_NOSELECTION | PD_RETURNDC;
if (!forceit)
{
prt_dlg.hDevMode = stored_dm;
prt_dlg.hDevNames = stored_devn;
prt_dlg.lCustData = stored_nCopies; 
#if !defined(FEAT_GUI) || defined(VIMDLL)
#if defined(VIMDLL)
if (!gui.in_use)
#endif
{



prt_dlg.lpfnPrintHook = PrintHookProc;
prt_dlg.Flags |= PD_ENABLEPRINTHOOK;
}
#endif
prt_dlg.Flags |= stored_nFlags;
}





if (forceit
#if !defined(FEAT_GUI) || defined(VIMDLL)
#if defined(VIMDLL)
|| (!gui.in_use && !term_console)
#else
|| !term_console
#endif
#endif
)
{
prt_dlg.Flags |= PD_RETURNDEFAULT;




if (*p_pdev != NUL)
prt_dlg.hDC = CreateDC(NULL, (LPCSTR)p_pdev, NULL, NULL);
else
{
prt_dlg.Flags |= PD_RETURNDEFAULT;
if (PrintDlgW(&prt_dlg) == 0)
goto init_fail_dlg;
}
}
else if (PrintDlgW(&prt_dlg) == 0)
goto init_fail_dlg;
else
{



stored_dm = prt_dlg.hDevMode;
stored_devn = prt_dlg.hDevNames;
stored_nFlags = prt_dlg.Flags;
stored_nCopies = prt_dlg.nCopies;
}

if (prt_dlg.hDC == NULL)
{
emsg(_("E237: Printer selection failed"));
mch_print_cleanup();
return FALSE;
}




i = GetDeviceCaps(prt_dlg.hDC, NUMCOLORS);
psettings->has_color = (GetDeviceCaps(prt_dlg.hDC, BITSPIXEL) > 1
|| GetDeviceCaps(prt_dlg.hDC, PLANES) > 1
|| i > 2 || i == -1);


SetTextAlign(prt_dlg.hDC, TA_BASELINE|TA_LEFT);






mem = (DEVMODEW *)GlobalLock(prt_dlg.hDevMode);
if (mem != NULL)
{
if (mem->dmCopies != 1)
stored_nCopies = mem->dmCopies;
if ((mem->dmFields & DM_DUPLEX) && (mem->dmDuplex & ~DMDUP_SIMPLEX))
psettings->duplex = TRUE;
if ((mem->dmFields & DM_COLOR) && (mem->dmColor & DMCOLOR_COLOR))
psettings->has_color = TRUE;
}
GlobalUnlock(prt_dlg.hDevMode);

devname = (DEVNAMES *)GlobalLock(prt_dlg.hDevNames);
if (devname != 0)
{
WCHAR *wprinter_name = (WCHAR *)devname + devname->wDeviceOffset;
WCHAR *wport_name = (WCHAR *)devname + devname->wOutputOffset;
char_u *text = (char_u *)_("to %s on %s");
char_u *printer_name = utf16_to_enc(wprinter_name, NULL);
char_u *port_name = utf16_to_enc(wport_name, NULL);

if (printer_name != NULL && port_name != NULL)
prt_name = alloc(STRLEN(printer_name)
+ STRLEN(port_name) + STRLEN(text));
if (prt_name != NULL)
wsprintf((char *)prt_name, (const char *)text,
printer_name, port_name);
vim_free(printer_name);
vim_free(port_name);
}
GlobalUnlock(prt_dlg.hDevNames);




vim_memset(&fLogFont, 0, sizeof(fLogFont));
if (get_logfont(&fLogFont, p_pfn, prt_dlg.hDC, TRUE) == FAIL)
{
semsg(_("E613: Unknown printer font: %s"), p_pfn);
mch_print_cleanup();
return FALSE;
}

for (pifBold = 0; pifBold <= 1; pifBold++)
for (pifItalic = 0; pifItalic <= 1; pifItalic++)
for (pifUnderline = 0; pifUnderline <= 1; pifUnderline++)
{
fLogFont.lfWeight = boldface[pifBold];
fLogFont.lfItalic = pifItalic;
fLogFont.lfUnderline = pifUnderline;
prt_font_handles[pifBold][pifItalic][pifUnderline]
= CreateFontIndirectW(&fLogFont);
}

SetBkMode(prt_dlg.hDC, OPAQUE);
SelectObject(prt_dlg.hDC, prt_font_handles[0][0][0]);




psettings->chars_per_line = prt_get_cpl();
psettings->lines_per_page = prt_get_lpp();
if (prt_dlg.Flags & PD_USEDEVMODECOPIESANDCOLLATE)
{
psettings->n_collated_copies = (prt_dlg.Flags & PD_COLLATE)
? prt_dlg.nCopies : 1;
psettings->n_uncollated_copies = (prt_dlg.Flags & PD_COLLATE)
? 1 : prt_dlg.nCopies;

if (psettings->n_collated_copies == 0)
psettings->n_collated_copies = 1;

if (psettings->n_uncollated_copies == 0)
psettings->n_uncollated_copies = 1;
}
else
{
psettings->n_collated_copies = 1;
psettings->n_uncollated_copies = 1;
}

psettings->jobname = jobname;

return TRUE;

init_fail_dlg:
{
DWORD err = CommDlgExtendedError();

if (err)
{
char_u *buf;



FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
FORMAT_MESSAGE_FROM_SYSTEM |
FORMAT_MESSAGE_IGNORE_INSERTS,
NULL, err, 0, (LPTSTR)(&buf), 0, NULL);
semsg(_("E238: Print error: %s"),
buf == NULL ? (char_u *)_("Unknown") : buf);
LocalFree((LPVOID)(buf));
}
else
msg_clr_eos(); 

mch_print_cleanup();
return FALSE;
}
}


int
mch_print_begin(prt_settings_T *psettings)
{
int ret = 0;
char szBuffer[300];
WCHAR *wp;

hDlgPrint = CreateDialog(g_hinst, TEXT("PrintDlgBox"),
prt_dlg.hwndOwner, PrintDlgProc);
SetAbortProc(prt_dlg.hDC, AbortProc);
wsprintf(szBuffer, _("Printing '%s'"), gettail(psettings->jobname));
vimSetDlgItemText(hDlgPrint, IDC_PRINTTEXT1, (char_u *)szBuffer);

wp = enc_to_utf16(psettings->jobname, NULL);
if (wp != NULL)
{
DOCINFOW di;

vim_memset(&di, 0, sizeof(di));
di.cbSize = sizeof(di);
di.lpszDocName = wp;
ret = StartDocW(prt_dlg.hDC, &di);
vim_free(wp);
}

#if defined(FEAT_GUI)

#if defined(VIMDLL)
if (gui.in_use)
#endif
SetFocus(s_hwnd);
#endif

return (ret > 0);
}

void
mch_print_end(prt_settings_T *psettings UNUSED)
{
EndDoc(prt_dlg.hDC);
if (!*bUserAbort)
SendMessage(hDlgPrint, WM_COMMAND, 0, 0);
}

int
mch_print_end_page(void)
{
return (EndPage(prt_dlg.hDC) > 0);
}

int
mch_print_begin_page(char_u *msg)
{
if (msg != NULL)
vimSetDlgItemText(hDlgPrint, IDC_PROGRESS, msg);
return (StartPage(prt_dlg.hDC) > 0);
}

int
mch_print_blank_page(void)
{
return (mch_print_begin_page(NULL) ? (mch_print_end_page()) : FALSE);
}

static int prt_pos_x = 0;
static int prt_pos_y = 0;

void
mch_print_start_line(int margin, int page_line)
{
if (margin)
prt_pos_x = -prt_number_width;
else
prt_pos_x = 0;
prt_pos_y = page_line * prt_line_height
+ prt_tm.tmAscent + prt_tm.tmExternalLeading;
}

int
mch_print_text_out(char_u *p, int len)
{
SIZE sz;
WCHAR *wp;
int wlen = len;
int ret = FALSE;

wp = enc_to_utf16(p, &wlen);
if (wp == NULL)
return FALSE;

TextOutW(prt_dlg.hDC, prt_pos_x + prt_left_margin,
prt_pos_y + prt_top_margin, wp, wlen);
GetTextExtentPoint32W(prt_dlg.hDC, wp, wlen, &sz);
vim_free(wp);
prt_pos_x += (sz.cx - prt_tm.tmOverhang);

if (p[len] != NUL)
{
wlen = mb_ptr2len(p + len);
wp = enc_to_utf16(p + len, &wlen);
if (wp != NULL)
{
GetTextExtentPoint32W(prt_dlg.hDC, wp, 1, &sz);
ret = (prt_pos_x + prt_left_margin + sz.cx > prt_right_margin);
vim_free(wp);
}
}
return ret;
}

void
mch_print_set_font(int iBold, int iItalic, int iUnderline)
{
SelectObject(prt_dlg.hDC, prt_font_handles[iBold][iItalic][iUnderline]);
}

void
mch_print_set_bg(long_u bgcol)
{
SetBkColor(prt_dlg.hDC, GetNearestColor(prt_dlg.hDC,
swap_me((COLORREF)bgcol)));




if (bgcol == 0xffffffUL)
SetBkMode(prt_dlg.hDC, TRANSPARENT);
else
SetBkMode(prt_dlg.hDC, OPAQUE);
}

void
mch_print_set_fg(long_u fgcol)
{
SetTextColor(prt_dlg.hDC, GetNearestColor(prt_dlg.hDC,
swap_me((COLORREF)fgcol)));
}

#endif 



#if defined(FEAT_SHORTCUT) || defined(PROTO)
#if !defined(PROTO)
#include <shlobj.h>
#endif

typedef BOOL (WINAPI *pfnGetFinalPathNameByHandleW)(
HANDLE hFile,
LPWSTR lpszFilePath,
DWORD cchFilePath,
DWORD dwFlags);
static pfnGetFinalPathNameByHandleW pGetFinalPathNameByHandleW = NULL;

#define is_path_sep(c) ((c) == L'\\' || (c) == L'/')

static int
is_reparse_point_included(LPCWSTR fname)
{
LPCWSTR p = fname, q;
WCHAR buf[MAX_PATH];
DWORD attr;

if (isalpha(p[0]) && p[1] == L':' && is_path_sep(p[2]))
p += 3;
else if (is_path_sep(p[0]) && is_path_sep(p[1]))
p += 2;

while (*p != L'\0')
{
q = wcspbrk(p, L"\\/");
if (q == NULL)
p = q = fname + wcslen(fname);
else
p = q + 1;
if (q - fname >= MAX_PATH)
return FALSE;
wcsncpy(buf, fname, q - fname);
buf[q - fname] = L'\0';
attr = GetFileAttributesW(buf);
if (attr != INVALID_FILE_ATTRIBUTES
&& (attr & FILE_ATTRIBUTE_REPARSE_POINT) != 0)
return TRUE;
}
return FALSE;
}

static char_u *
resolve_reparse_point(char_u *fname)
{
HANDLE h = INVALID_HANDLE_VALUE;
DWORD size;
WCHAR *p, *wp;
char_u *rfname = NULL;
WCHAR *buff = NULL;
static BOOL loaded = FALSE;

if (pGetFinalPathNameByHandleW == NULL)
{
HMODULE hmod = GetModuleHandle("kernel32.dll");

if (loaded == TRUE)
return NULL;
pGetFinalPathNameByHandleW = (pfnGetFinalPathNameByHandleW)
GetProcAddress(hmod, "GetFinalPathNameByHandleW");
loaded = TRUE;
if (pGetFinalPathNameByHandleW == NULL)
return NULL;
}

p = enc_to_utf16(fname, NULL);
if (p == NULL)
goto fail;

if (!is_reparse_point_included(p))
{
vim_free(p);
goto fail;
}

h = CreateFileW(p, 0, 0, NULL, OPEN_EXISTING,
FILE_FLAG_BACKUP_SEMANTICS, NULL);
vim_free(p);

if (h == INVALID_HANDLE_VALUE)
goto fail;

size = pGetFinalPathNameByHandleW(h, NULL, 0, 0);
if (size == 0)
goto fail;
buff = ALLOC_MULT(WCHAR, size);
if (buff == NULL)
goto fail;
if (pGetFinalPathNameByHandleW(h, buff, size, 0) == 0)
goto fail;

if (wcsncmp(buff, L"\\\\?\\UNC\\", 8) == 0)
{
buff[6] = L'\\';
wp = buff + 6;
}
else if (wcsncmp(buff, L"\\\\?\\", 4) == 0)
wp = buff + 4;
else
wp = buff;

rfname = utf16_to_enc(wp, NULL);

fail:
if (h != INVALID_HANDLE_VALUE)
CloseHandle(h);
if (buff != NULL)
vim_free(buff);

return rfname;
}






static char_u *
resolve_shortcut(char_u *fname)
{
HRESULT hr;
IShellLink *psl = NULL;
IPersistFile *ppf = NULL;
OLECHAR wsz[MAX_PATH];
char_u *rfname = NULL;
int len;
IShellLinkW *pslw = NULL;
WIN32_FIND_DATAW ffdw; 



if (fname == NULL)
return rfname;
len = (int)STRLEN(fname);
if (len <= 4 || STRNICMP(fname + len - 4, ".lnk", 4) != 0)
return rfname;

CoInitialize(NULL);


hr = CoCreateInstance(
&CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
&IID_IShellLinkW, (void**)&pslw);
if (hr == S_OK)
{
WCHAR *p = enc_to_utf16(fname, NULL);

if (p != NULL)
{

hr = pslw->lpVtbl->QueryInterface(
pslw, &IID_IPersistFile, (void**)&ppf);
if (hr != S_OK)
goto shortcut_errorw;


hr = ppf->lpVtbl->Load(ppf, p, STGM_READ);
if (hr != S_OK)
goto shortcut_errorw;
#if 0 
hr = pslw->lpVtbl->Resolve(pslw, NULL, SLR_NO_UI);
if (hr != S_OK)
goto shortcut_errorw;
#endif


ZeroMemory(wsz, MAX_PATH * sizeof(WCHAR));
hr = pslw->lpVtbl->GetPath(pslw, wsz, MAX_PATH, &ffdw, 0);
if (hr == S_OK && wsz[0] != NUL)
rfname = utf16_to_enc(wsz, NULL);

shortcut_errorw:
vim_free(p);
}
}


if (ppf != NULL)
ppf->lpVtbl->Release(ppf);
if (psl != NULL)
psl->lpVtbl->Release(psl);
if (pslw != NULL)
pslw->lpVtbl->Release(pslw);

CoUninitialize();
return rfname;
}

char_u *
mch_resolve_path(char_u *fname, int reparse_point)
{
char_u *path = resolve_shortcut(fname);

if (path == NULL && reparse_point)
path = resolve_reparse_point(fname);
return path;
}
#endif

#if (defined(FEAT_EVAL) && (!defined(FEAT_GUI) || defined(VIMDLL))) || defined(PROTO)



void
win32_set_foreground(void)
{
GetConsoleHwnd(); 
if (s_hwnd != 0)
SetForegroundWindow(s_hwnd);
}
#endif

#if defined(FEAT_CLIENTSERVER) || defined(PROTO)












HWND message_window = 0; 

#define VIM_CLASSNAME "VIM_MESSAGES"
#define VIM_CLASSNAME_LEN (sizeof(VIM_CLASSNAME) - 1)



#define COPYDATA_KEYS 0
#define COPYDATA_REPLY 1
#define COPYDATA_EXPR 10
#define COPYDATA_RESULT 11
#define COPYDATA_ERROR_RESULT 12
#define COPYDATA_ENCODING 20


struct server_id
{
HWND hwnd;
char_u *name;
};


static char_u *client_enc = NULL;





static void
serverSendEnc(HWND target)
{
COPYDATASTRUCT data;

data.dwData = COPYDATA_ENCODING;
data.cbData = (DWORD)STRLEN(p_enc) + 1;
data.lpData = p_enc;
(void)SendMessage(target, WM_COPYDATA, (WPARAM)message_window,
(LPARAM)(&data));
}




static void
CleanUpMessaging(void)
{
if (message_window != 0)
{
DestroyWindow(message_window);
message_window = 0;
}
}

static int save_reply(HWND server, char_u *reply, int expr);











static LRESULT CALLBACK
Messaging_WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
if (msg == WM_COPYDATA)
{




















COPYDATASTRUCT *data = (COPYDATASTRUCT*)lParam;
HWND sender = (HWND)wParam;
COPYDATASTRUCT reply;
char_u *res;
int retval;
char_u *str;
char_u *tofree;

switch (data->dwData)
{
case COPYDATA_ENCODING:

vim_free(client_enc);
client_enc = enc_canonize((char_u *)data->lpData);
return 1;

case COPYDATA_KEYS:

clientWindow = sender;



str = serverConvert(client_enc, (char_u *)data->lpData, &tofree);
server_to_input_buf(str);
vim_free(tofree);

#if defined(FEAT_GUI)

#if defined(VIMDLL)
if (gui.in_use)
#endif
if (s_hwnd != 0)
PostMessage(s_hwnd, WM_NULL, 0, 0);
#endif
return 1;

case COPYDATA_EXPR:

clientWindow = sender;

str = serverConvert(client_enc, (char_u *)data->lpData, &tofree);
res = eval_client_expr_to_string(str);

if (res == NULL)
{
char *err = _(e_invexprmsg);
size_t len = STRLEN(str) + STRLEN(err) + 5;

res = alloc(len);
if (res != NULL)
vim_snprintf((char *)res, len, "%s: \"%s\"", err, str);
reply.dwData = COPYDATA_ERROR_RESULT;
}
else
reply.dwData = COPYDATA_RESULT;
reply.lpData = res;
reply.cbData = (DWORD)STRLEN(res) + 1;

serverSendEnc(sender);
retval = (int)SendMessage(sender, WM_COPYDATA,
(WPARAM)message_window, (LPARAM)(&reply));
vim_free(tofree);
vim_free(res);
return retval;

case COPYDATA_REPLY:
case COPYDATA_RESULT:
case COPYDATA_ERROR_RESULT:
if (data->lpData != NULL)
{
str = serverConvert(client_enc, (char_u *)data->lpData,
&tofree);
if (tofree == NULL)
str = vim_strsave(str);
if (save_reply(sender, str,
(data->dwData == COPYDATA_REPLY ? 0 :
(data->dwData == COPYDATA_RESULT ? 1 :
2))) == FAIL)
vim_free(str);
else if (data->dwData == COPYDATA_REPLY)
{
char_u winstr[30];

sprintf((char *)winstr, PRINTF_HEX_LONG_U, (long_u)sender);
apply_autocmds(EVENT_REMOTEREPLY, winstr, str,
TRUE, curbuf);
}
}
return 1;
}

return 0;
}

else if (msg == WM_ACTIVATE && wParam == WA_ACTIVE)
{


#if !defined(FEAT_GUI) || defined(VIMDLL)
#if defined(VIMDLL)
if (!gui.in_use)
#endif
GetConsoleHwnd(); 
#endif
if (s_hwnd != 0)
{
SetForegroundWindow(s_hwnd);
return 0;
}
}

return DefWindowProc(hwnd, msg, wParam, lParam);
}





void
serverInitMessaging(void)
{
WNDCLASS wndclass;


atexit(CleanUpMessaging);



wndclass.style = 0;
wndclass.lpfnWndProc = Messaging_WndProc;
wndclass.cbClsExtra = 0;
wndclass.cbWndExtra = 0;
wndclass.hInstance = g_hinst;
wndclass.hIcon = NULL;
wndclass.hCursor = NULL;
wndclass.hbrBackground = NULL;
wndclass.lpszMenuName = NULL;
wndclass.lpszClassName = VIM_CLASSNAME;
RegisterClass(&wndclass);




message_window = CreateWindow(VIM_CLASSNAME, "",
WS_POPUPWINDOW | WS_CAPTION,
CW_USEDEFAULT, CW_USEDEFAULT,
100, 100, NULL, NULL,
g_hinst, NULL);
}


static char_u *altname_buf_ptr = NULL;






static int
getVimServerName(HWND hwnd, char *name, int namelen)
{
int len;
char buffer[VIM_CLASSNAME_LEN + 1];


len = GetClassName(hwnd, buffer, sizeof(buffer));
if (len != VIM_CLASSNAME_LEN || STRCMP(buffer, VIM_CLASSNAME) != 0)
return 0;


return GetWindowText(hwnd, name, namelen);
}

static BOOL CALLBACK
enumWindowsGetServer(HWND hwnd, LPARAM lparam)
{
struct server_id *id = (struct server_id *)lparam;
char server[MAX_PATH];


if (getVimServerName(hwnd, server, sizeof(server)) == 0)
return TRUE;


if (STRICMP(server, id->name) == 0)
{
id->hwnd = hwnd;
return FALSE;
}


if (altname_buf_ptr != NULL
&& STRNICMP(server, id->name, STRLEN(id->name)) == 0
&& vim_isdigit(server[STRLEN(id->name)]))
{
STRCPY(altname_buf_ptr, server);
altname_buf_ptr = NULL; 
}


return TRUE;
}

static BOOL CALLBACK
enumWindowsGetNames(HWND hwnd, LPARAM lparam)
{
garray_T *ga = (garray_T *)lparam;
char server[MAX_PATH];


if (getVimServerName(hwnd, server, sizeof(server)) == 0)
return TRUE;


ga_concat(ga, (char_u *)server);
ga_concat(ga, (char_u *)"\n");
return TRUE;
}

struct enum_windows_s
{
WNDENUMPROC lpEnumFunc;
LPARAM lParam;
};

static BOOL CALLBACK
enum_windows_child(HWND hwnd, LPARAM lParam)
{
struct enum_windows_s *ew = (struct enum_windows_s *)lParam;

return (ew->lpEnumFunc)(hwnd, ew->lParam);
}

static BOOL CALLBACK
enum_windows_toplevel(HWND hwnd, LPARAM lParam)
{
struct enum_windows_s *ew = (struct enum_windows_s *)lParam;

if ((ew->lpEnumFunc)(hwnd, ew->lParam))
return TRUE;
return EnumChildWindows(hwnd, enum_windows_child, lParam);
}




static BOOL
enum_windows(WNDENUMPROC lpEnumFunc, LPARAM lParam)
{
struct enum_windows_s ew;

ew.lpEnumFunc = lpEnumFunc;
ew.lParam = lParam;
return EnumWindows(enum_windows_toplevel, (LPARAM)&ew);
}

static HWND
findServer(char_u *name)
{
struct server_id id;

id.name = name;
id.hwnd = 0;

enum_windows(enumWindowsGetServer, (LPARAM)(&id));

return id.hwnd;
}

void
serverSetName(char_u *name)
{
char_u *ok_name;
HWND hwnd = 0;
int i = 0;
char_u *p;


ok_name = alloc(STRLEN(name) + 10);

STRCPY(ok_name, name);
p = ok_name + STRLEN(name);

for (;;)
{



hwnd = findServer(ok_name);
if (hwnd == 0)
break;

++i;
if (i >= 1000)
break;

sprintf((char *)p, "%d", i);
}

if (hwnd != 0)
vim_free(ok_name);
else
{

serverName = ok_name;
#if defined(FEAT_TITLE)
need_maketitle = TRUE; 
#endif


SetWindowText(message_window, (LPCSTR)ok_name);

#if defined(FEAT_EVAL)

set_vim_var_string(VV_SEND_SERVER, serverName, -1);
#endif
}
}

char_u *
serverGetVimNames(void)
{
garray_T ga;

ga_init2(&ga, 1, 100);

enum_windows(enumWindowsGetNames, (LPARAM)(&ga));
ga_append(&ga, NUL);

return ga.ga_data;
}

int
serverSendReply(
char_u *name, 
char_u *reply) 
{
HWND target;
COPYDATASTRUCT data;
long_u n = 0;




sscanf((char *)name, SCANF_HEX_LONG_U, &n);
if (n == 0)
return -1;

target = (HWND)n;
if (!IsWindow(target))
return -1;

data.dwData = COPYDATA_REPLY;
data.cbData = (DWORD)STRLEN(reply) + 1;
data.lpData = reply;

serverSendEnc(target);
if (SendMessage(target, WM_COPYDATA, (WPARAM)message_window,
(LPARAM)(&data)))
return 0;

return -1;
}

int
serverSendToVim(
char_u *name, 
char_u *cmd, 
char_u **result, 
void *ptarget, 
int asExpr, 
int timeout, 
int silent) 
{
HWND target;
COPYDATASTRUCT data;
char_u *retval = NULL;
int retcode = 0;
char_u altname_buf[MAX_PATH];


if (serverName != NULL && STRICMP(name, serverName) == 0)
return sendToLocalVim(cmd, asExpr, result);



if (STRLEN(name) > 1 && !vim_isdigit(name[STRLEN(name) - 1]))
altname_buf_ptr = altname_buf;
altname_buf[0] = NUL;
target = findServer(name);
altname_buf_ptr = NULL;
if (target == 0 && altname_buf[0] != NUL)

target = findServer(altname_buf);

if (target == 0)
{
if (!silent)
semsg(_(e_noserver), name);
return -1;
}

if (ptarget)
*(HWND *)ptarget = target;

data.dwData = asExpr ? COPYDATA_EXPR : COPYDATA_KEYS;
data.cbData = (DWORD)STRLEN(cmd) + 1;
data.lpData = cmd;

serverSendEnc(target);
if (SendMessage(target, WM_COPYDATA, (WPARAM)message_window,
(LPARAM)(&data)) == 0)
return -1;

if (asExpr)
retval = serverGetReply(target, &retcode, TRUE, TRUE, timeout);

if (result == NULL)
vim_free(retval);
else
*result = retval; 

return retcode;
}




void
serverForeground(char_u *name)
{
HWND target = findServer(name);

if (target != 0)
SetForegroundWindow(target);
}








typedef struct
{
HWND server; 
char_u *reply; 
int expr_result; 
} reply_T;

static garray_T reply_list = {0, 0, sizeof(reply_T), 5, 0};

#define REPLY_ITEM(i) ((reply_T *)(reply_list.ga_data) + (i))
#define REPLY_COUNT (reply_list.ga_len)


static int reply_received = 0;




static int
save_reply(HWND server, char_u *reply, int expr)
{
reply_T *rep;

if (ga_grow(&reply_list, 1) == FAIL)
return FAIL;

rep = REPLY_ITEM(REPLY_COUNT);
rep->server = server;
rep->reply = reply;
rep->expr_result = expr;
if (rep->reply == NULL)
return FAIL;

++REPLY_COUNT;
reply_received = 1;
return OK;
}









char_u *
serverGetReply(HWND server, int *expr_res, int remove, int wait, int timeout)
{
int i;
char_u *reply;
reply_T *rep;
int did_process = FALSE;
time_t start;
time_t now;


time(&start);
for (;;)
{


reply_received = 0;

for (i = 0; i < REPLY_COUNT; ++i)
{
rep = REPLY_ITEM(i);
if (rep->server == server
&& ((rep->expr_result != 0) == (expr_res != NULL)))
{

reply = rep->reply;
if (expr_res != NULL)
*expr_res = rep->expr_result == 1 ? 0 : -1;

if (remove)
{

mch_memmove(rep, rep + 1,
(REPLY_COUNT - i - 1) * sizeof(reply_T));
--REPLY_COUNT;
}



return reply;
}
}



if (!wait)
{


if (!did_process)
{
did_process = TRUE;
serverProcessPendingMessages();
continue;
}
break;
}





while (reply_received == 0)
{
#if defined(FEAT_TIMERS)

check_due_timer();
#endif
time(&now);
if (timeout > 0 && (now - start) >= timeout)
break;




MsgWaitForMultipleObjects(0, NULL, TRUE, 1000, QS_ALLINPUT);


if (!IsWindow(server))
return NULL;

serverProcessPendingMessages();
}
}

return NULL;
}




void
serverProcessPendingMessages(void)
{
MSG msg;

while (pPeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
{
TranslateMessage(&msg);
pDispatchMessage(&msg);
}
}

#endif 

#if defined(FEAT_GUI) || (defined(FEAT_PRINTER) && !defined(FEAT_POSTSCRIPT)) || defined(PROTO)


struct charset_pair
{
char *name;
BYTE charset;
};

static struct charset_pair
charset_pairs[] =
{
{"ANSI", ANSI_CHARSET},
{"CHINESEBIG5", CHINESEBIG5_CHARSET},
{"DEFAULT", DEFAULT_CHARSET},
{"HANGEUL", HANGEUL_CHARSET},
{"OEM", OEM_CHARSET},
{"SHIFTJIS", SHIFTJIS_CHARSET},
{"SYMBOL", SYMBOL_CHARSET},
{"ARABIC", ARABIC_CHARSET},
{"BALTIC", BALTIC_CHARSET},
{"EASTEUROPE", EASTEUROPE_CHARSET},
{"GB2312", GB2312_CHARSET},
{"GREEK", GREEK_CHARSET},
{"HEBREW", HEBREW_CHARSET},
{"JOHAB", JOHAB_CHARSET},
{"MAC", MAC_CHARSET},
{"RUSSIAN", RUSSIAN_CHARSET},
{"THAI", THAI_CHARSET},
{"TURKISH", TURKISH_CHARSET},
#if defined(VIETNAMESE_CHARSET)
{"VIETNAMESE", VIETNAMESE_CHARSET},
#endif
{NULL, 0}
};

struct quality_pair
{
char *name;
DWORD quality;
};

static struct quality_pair
quality_pairs[] = {
#if defined(CLEARTYPE_QUALITY)
{"CLEARTYPE", CLEARTYPE_QUALITY},
#endif
#if defined(ANTIALIASED_QUALITY)
{"ANTIALIASED", ANTIALIASED_QUALITY},
#endif
#if defined(NONANTIALIASED_QUALITY)
{"NONANTIALIASED", NONANTIALIASED_QUALITY},
#endif
#if defined(PROOF_QUALITY)
{"PROOF", PROOF_QUALITY},
#endif
#if defined(DRAFT_QUALITY)
{"DRAFT", DRAFT_QUALITY},
#endif
{"DEFAULT", DEFAULT_QUALITY},
{NULL, 0}
};





char *
charset_id2name(int id)
{
struct charset_pair *cp;

for (cp = charset_pairs; cp->name != NULL; ++cp)
if ((BYTE)id == cp->charset)
break;
return cp->name;
}





char *
quality_id2name(DWORD id)
{
struct quality_pair *qp;

for (qp = quality_pairs; qp->name != NULL; ++qp)
if (id == qp->quality)
break;
return qp->name;
}

static const LOGFONTW s_lfDefault =
{
-12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
PROOF_QUALITY, FIXED_PITCH | FF_DONTCARE,
L"Fixedsys" 
};





int current_font_height = -12; 








static int
points_to_pixels(WCHAR *str, WCHAR **end, int vertical, long_i pprinter_dc)
{
int pixels;
int points = 0;
int divisor = 0;
HWND hwnd = (HWND)0;
HDC hdc;
HDC printer_dc = (HDC)pprinter_dc;

while (*str != NUL)
{
if (*str == L'.' && divisor == 0)
{

divisor = 1;
}
else
{
if (!VIM_ISDIGIT(*str))
break;

points *= 10;
points += *str - L'0';
divisor *= 10;
}
++str;
}

if (divisor == 0)
divisor = 1;

if (printer_dc == NULL)
{
hwnd = GetDesktopWindow();
hdc = GetWindowDC(hwnd);
}
else
hdc = printer_dc;

pixels = MulDiv(points,
GetDeviceCaps(hdc, vertical ? LOGPIXELSY : LOGPIXELSX),
72 * divisor);

if (printer_dc == NULL)
ReleaseDC(hwnd, hdc);

*end = str;
return pixels;
}

static int CALLBACK
font_enumproc(
ENUMLOGFONTW *elf,
NEWTEXTMETRICW *ntm UNUSED,
DWORD type UNUSED,
LPARAM lparam)
{










LOGFONTW *lf = (LOGFONTW *)(lparam);

#if !defined(FEAT_PROPORTIONAL_FONTS)

if ((ntm->tmPitchAndFamily & 1) != 0)
return 1;
#endif


*lf = elf->elfLogFont;


if (lf->lfCharSet == ANSI_CHARSET
|| lf->lfCharSet == OEM_CHARSET
|| lf->lfCharSet == DEFAULT_CHARSET)
return 0;


return 2;
}

static int
init_logfont(LOGFONTW *lf)
{
int n;
HWND hwnd = GetDesktopWindow();
HDC hdc = GetWindowDC(hwnd);

n = EnumFontFamiliesW(hdc,
lf->lfFaceName,
(FONTENUMPROCW)font_enumproc,
(LPARAM)lf);

ReleaseDC(hwnd, hdc);


if (n == 1)
return FAIL;




lf->lfHeight = current_font_height;
lf->lfWidth = 0;
lf->lfItalic = FALSE;
lf->lfUnderline = FALSE;
lf->lfStrikeOut = FALSE;
lf->lfWeight = FW_NORMAL;


return OK;
}





static int
utf16ascncmp(const WCHAR *w, const char *p, size_t n)
{
size_t i;

for (i = 0; i < n; i++)
{
if (w[i] == 0 || w[i] != p[i])
return w[i] - p[i];
}
return 0;
}





int
get_logfont(
LOGFONTW *lf,
char_u *name,
HDC printer_dc,
int verbose)
{
WCHAR *p;
int i;
int ret = FAIL;
static LOGFONTW *lastlf = NULL;
WCHAR *wname;

*lf = s_lfDefault;
if (name == NULL)
return OK;

wname = enc_to_utf16(name, NULL);
if (wname == NULL)
return FAIL;

if (wcscmp(wname, L"*") == 0)
{
#if defined(FEAT_GUI_MSWIN)
CHOOSEFONTW cf;

vim_memset(&cf, 0, sizeof(cf));
cf.lStructSize = sizeof(cf);
cf.hwndOwner = s_hwnd;
cf.Flags = CF_SCREENFONTS | CF_FIXEDPITCHONLY | CF_INITTOLOGFONTSTRUCT;
if (lastlf != NULL)
*lf = *lastlf;
cf.lpLogFont = lf;
cf.nFontType = 0 ; 
if (ChooseFontW(&cf))
ret = OK;
#endif
goto theend;
}




for (p = wname; *p && *p != L':'; p++)
{
if (p - wname + 1 >= LF_FACESIZE)
goto theend; 
lf->lfFaceName[p - wname] = *p;
}
if (p != wname)
lf->lfFaceName[p - wname] = NUL;


lf->lfHeight = -12;
lf->lfWidth = 0;
lf->lfWeight = FW_NORMAL;
lf->lfItalic = FALSE;
lf->lfUnderline = FALSE;
lf->lfStrikeOut = FALSE;




if (init_logfont(lf) == FAIL)
{
int did_replace = FALSE;

for (i = 0; lf->lfFaceName[i]; ++i)
if (lf->lfFaceName[i] == L'_')
{
lf->lfFaceName[i] = L' ';
did_replace = TRUE;
}
if (!did_replace || init_logfont(lf) == FAIL)
goto theend;
}

while (*p == L':')
p++;


while (*p)
{
switch (*p++)
{
case L'h':
lf->lfHeight = - points_to_pixels(p, &p, TRUE, (long_i)printer_dc);
break;
case L'w':
lf->lfWidth = points_to_pixels(p, &p, FALSE, (long_i)printer_dc);
break;
case L'W':
lf->lfWeight = wcstol(p, &p, 10);
break;
case L'b':
lf->lfWeight = FW_BOLD;
break;
case L'i':
lf->lfItalic = TRUE;
break;
case L'u':
lf->lfUnderline = TRUE;
break;
case L's':
lf->lfStrikeOut = TRUE;
break;
case L'c':
{
struct charset_pair *cp;

for (cp = charset_pairs; cp->name != NULL; ++cp)
if (utf16ascncmp(p, cp->name, strlen(cp->name)) == 0)
{
lf->lfCharSet = cp->charset;
p += strlen(cp->name);
break;
}
if (cp->name == NULL && verbose)
{
char_u *s = utf16_to_enc(p, NULL);
semsg(_("E244: Illegal charset name \"%s\" in font name \"%s\""), s, name);
vim_free(s);
break;
}
break;
}
case L'q':
{
struct quality_pair *qp;

for (qp = quality_pairs; qp->name != NULL; ++qp)
if (utf16ascncmp(p, qp->name, strlen(qp->name)) == 0)
{
lf->lfQuality = qp->quality;
p += strlen(qp->name);
break;
}
if (qp->name == NULL && verbose)
{
char_u *s = utf16_to_enc(p, NULL);
semsg(_("E244: Illegal quality name \"%s\" in font name \"%s\""), s, name);
vim_free(s);
break;
}
break;
}
default:
if (verbose)
semsg(_("E245: Illegal char '%c' in font name \"%s\""), p[-1], name);
goto theend;
}
while (*p == L':')
p++;
}
ret = OK;

theend:

if (ret == OK && printer_dc == NULL)
{
vim_free(lastlf);
lastlf = ALLOC_ONE(LOGFONTW);
if (lastlf != NULL)
mch_memmove(lastlf, lf, sizeof(LOGFONTW));
}
vim_free(wname);

return ret;
}

#endif 

#if defined(FEAT_JOB_CHANNEL) || defined(PROTO)



void
channel_init_winsock(void)
{
WSADATA wsaData;
int wsaerr;

if (WSInitialized)
return;

wsaerr = WSAStartup(MAKEWORD(2, 2), &wsaData);
if (wsaerr == 0)
WSInitialized = TRUE;
}
#endif
