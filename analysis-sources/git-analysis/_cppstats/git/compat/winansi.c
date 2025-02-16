#undef NOGDI
#include "../git-compat-util.h"
#include <wingdi.h>
#include <winreg.h>
#include "win32.h"
#include "win32/lazyload.h"
static int fd_is_interactive[3] = { 0, 0, 0 };
#define FD_CONSOLE 0x1
#define FD_SWAPPED 0x2
#define FD_MSYS 0x4
static HANDLE console;
static WORD plain_attr;
static WORD attr;
static int negative;
static int non_ascii_used = 0;
static HANDLE hthread, hread, hwrite;
static HANDLE hconsole1, hconsole2;
#if defined(__MINGW32__)
#if !defined(__MINGW64_VERSION_MAJOR) || __MINGW64_VERSION_MAJOR < 5
typedef struct _CONSOLE_FONT_INFOEX {
ULONG cbSize;
DWORD nFont;
COORD dwFontSize;
UINT FontFamily;
UINT FontWeight;
WCHAR FaceName[LF_FACESIZE];
} CONSOLE_FONT_INFOEX, *PCONSOLE_FONT_INFOEX;
#endif
#endif
static void warn_if_raster_font(void)
{
DWORD fontFamily = 0;
DECLARE_PROC_ADDR(kernel32.dll, BOOL, GetCurrentConsoleFontEx,
HANDLE, BOOL, PCONSOLE_FONT_INFOEX);
if (!non_ascii_used)
return;
if (INIT_PROC_ADDR(GetCurrentConsoleFontEx)) {
CONSOLE_FONT_INFOEX cfi;
cfi.cbSize = sizeof(cfi);
if (GetCurrentConsoleFontEx(console, 0, &cfi))
fontFamily = cfi.FontFamily;
} else {
HKEY hkey;
if (ERROR_SUCCESS == RegOpenKeyExA(HKEY_CURRENT_USER, "Console",
0, KEY_READ, &hkey)) {
DWORD size = sizeof(fontFamily);
RegQueryValueExA(hkey, "FontFamily", NULL, NULL,
(LPVOID) &fontFamily, &size);
RegCloseKey(hkey);
}
}
if (!(fontFamily & TMPF_TRUETYPE)) {
const wchar_t *msg = L"\nWarning: Your console font probably "
L"doesn\'t support Unicode. If you experience strange "
L"characters in the output, consider switching to a "
L"TrueType font such as Consolas!\n";
DWORD dummy;
WriteConsoleW(console, msg, wcslen(msg), &dummy, NULL);
}
}
static int is_console(int fd)
{
CONSOLE_SCREEN_BUFFER_INFO sbi;
DWORD mode;
HANDLE hcon;
static int initialized = 0;
hcon = (HANDLE) _get_osfhandle(fd);
if (hcon == INVALID_HANDLE_VALUE)
return 0;
if (GetFileType(hcon) != FILE_TYPE_CHAR)
return 0;
if (!fd) {
if (!GetConsoleMode(hcon, &mode))
return 0;
sbi.wAttributes = 0;
} else if (!GetConsoleScreenBufferInfo(hcon, &sbi))
return 0;
if (fd >= 0 && fd <= 2)
fd_is_interactive[fd] |= FD_CONSOLE;
if (!initialized) {
console = hcon;
attr = plain_attr = sbi.wAttributes;
negative = 0;
initialized = 1;
}
return 1;
}
#define BUFFER_SIZE 4096
#define MAX_PARAMS 16
static void write_console(unsigned char *str, size_t len)
{
static wchar_t wbuf[2 * BUFFER_SIZE + 1];
DWORD dummy;
int wlen = xutftowcsn(wbuf, (char*) str, ARRAY_SIZE(wbuf), len);
if (wlen < 0) {
wchar_t *err = L"[invalid]";
WriteConsoleW(console, err, wcslen(err), &dummy, NULL);
return;
}
WriteConsoleW(console, wbuf, wlen, &dummy, NULL);
if (wlen != len)
non_ascii_used = 1;
}
#define FOREGROUND_ALL (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)
#define BACKGROUND_ALL (BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE)
static void set_console_attr(void)
{
WORD attributes = attr;
if (negative) {
attributes &= ~FOREGROUND_ALL;
attributes &= ~BACKGROUND_ALL;
if (attr & FOREGROUND_RED)
attributes |= BACKGROUND_RED;
if (attr & FOREGROUND_GREEN)
attributes |= BACKGROUND_GREEN;
if (attr & FOREGROUND_BLUE)
attributes |= BACKGROUND_BLUE;
if (attr & BACKGROUND_RED)
attributes |= FOREGROUND_RED;
if (attr & BACKGROUND_GREEN)
attributes |= FOREGROUND_GREEN;
if (attr & BACKGROUND_BLUE)
attributes |= FOREGROUND_BLUE;
}
SetConsoleTextAttribute(console, attributes);
}
static void erase_in_line(void)
{
CONSOLE_SCREEN_BUFFER_INFO sbi;
DWORD dummy; 
if (!console)
return;
GetConsoleScreenBufferInfo(console, &sbi);
FillConsoleOutputCharacterA(console, ' ',
sbi.dwSize.X - sbi.dwCursorPosition.X, sbi.dwCursorPosition,
&dummy);
}
static void set_attr(char func, const int *params, int paramlen)
{
int i;
switch (func) {
case 'm':
for (i = 0; i < paramlen; i++) {
switch (params[i]) {
case 0: 
attr = plain_attr;
negative = 0;
break;
case 1: 
attr |= FOREGROUND_INTENSITY;
break;
case 2: 
case 22: 
attr &= ~FOREGROUND_INTENSITY;
break;
case 3: 
break;
case 4: 
case 21: 
break;
case 24: 
break;
case 5: 
case 6: 
attr |= BACKGROUND_INTENSITY;
break;
case 25: 
attr &= ~BACKGROUND_INTENSITY;
break;
case 7: 
negative = 1;
break;
case 27: 
negative = 0;
break;
case 8: 
case 28: 
break;
case 30: 
attr &= ~FOREGROUND_ALL;
break;
case 31: 
attr &= ~FOREGROUND_ALL;
attr |= FOREGROUND_RED;
break;
case 32: 
attr &= ~FOREGROUND_ALL;
attr |= FOREGROUND_GREEN;
break;
case 33: 
attr &= ~FOREGROUND_ALL;
attr |= FOREGROUND_RED | FOREGROUND_GREEN;
break;
case 34: 
attr &= ~FOREGROUND_ALL;
attr |= FOREGROUND_BLUE;
break;
case 35: 
attr &= ~FOREGROUND_ALL;
attr |= FOREGROUND_RED | FOREGROUND_BLUE;
break;
case 36: 
attr &= ~FOREGROUND_ALL;
attr |= FOREGROUND_GREEN | FOREGROUND_BLUE;
break;
case 37: 
attr |= FOREGROUND_RED |
FOREGROUND_GREEN |
FOREGROUND_BLUE;
break;
case 38: 
break;
case 39: 
attr &= ~FOREGROUND_ALL;
attr |= (plain_attr & FOREGROUND_ALL);
break;
case 40: 
attr &= ~BACKGROUND_ALL;
break;
case 41: 
attr &= ~BACKGROUND_ALL;
attr |= BACKGROUND_RED;
break;
case 42: 
attr &= ~BACKGROUND_ALL;
attr |= BACKGROUND_GREEN;
break;
case 43: 
attr &= ~BACKGROUND_ALL;
attr |= BACKGROUND_RED | BACKGROUND_GREEN;
break;
case 44: 
attr &= ~BACKGROUND_ALL;
attr |= BACKGROUND_BLUE;
break;
case 45: 
attr &= ~BACKGROUND_ALL;
attr |= BACKGROUND_RED | BACKGROUND_BLUE;
break;
case 46: 
attr &= ~BACKGROUND_ALL;
attr |= BACKGROUND_GREEN | BACKGROUND_BLUE;
break;
case 47: 
attr |= BACKGROUND_RED |
BACKGROUND_GREEN |
BACKGROUND_BLUE;
break;
case 48: 
break;
case 49: 
attr &= ~BACKGROUND_ALL;
attr |= (plain_attr & BACKGROUND_ALL);
break;
default:
break;
}
}
set_console_attr();
break;
case 'K':
erase_in_line();
break;
default:
break;
}
}
enum {
TEXT = 0, ESCAPE = 033, BRACKET = '['
};
static DWORD WINAPI console_thread(LPVOID unused)
{
unsigned char buffer[BUFFER_SIZE];
DWORD bytes;
int start, end = 0, c, parampos = 0, state = TEXT;
int params[MAX_PARAMS];
while (1) {
if (!ReadFile(hread, buffer + end, BUFFER_SIZE - end, &bytes,
NULL)) {
if (GetLastError() == ERROR_PIPE_NOT_CONNECTED ||
GetLastError() == ERROR_BROKEN_PIPE)
break;
continue;
}
bytes += end;
start = end = 0;
while (end < bytes) {
c = buffer[end++];
switch (state) {
case TEXT:
if (c == ESCAPE) {
if (end - 1 > start)
write_console(buffer + start,
end - 1 - start);
start = end - 1;
memset(params, 0, sizeof(params));
parampos = 0;
state = ESCAPE;
}
break;
case ESCAPE:
state = (c == BRACKET) ? BRACKET : TEXT;
break;
case BRACKET:
if (c >= '0' && c <= '9') {
params[parampos] *= 10;
params[parampos] += c - '0';
} else if (c == ';') {
parampos++;
if (parampos >= MAX_PARAMS)
state = TEXT;
} else {
set_attr(c, params, parampos + 1);
start = end;
state = TEXT;
}
break;
}
}
if (state == TEXT && end > start) {
if (buffer[end - 1] >= 0x80) {
if (buffer[end -1] >= 0xc0)
end--;
else if (end - 1 > start &&
buffer[end - 2] >= 0xe0)
end -= 2;
else if (end - 2 > start &&
buffer[end - 3] >= 0xf0)
end -= 3;
}
if (end > start)
write_console(buffer + start, end - start);
if (end < bytes)
memmove(buffer, buffer + end, bytes - end);
end = bytes - end;
} else {
end = 0;
}
}
warn_if_raster_font();
CloseHandle(hread);
return 0;
}
static void winansi_exit(void)
{
_flushall();
FlushFileBuffers(hwrite);
DisconnectNamedPipe(hwrite);
WaitForSingleObject(hthread, INFINITE);
CloseHandle(hwrite);
CloseHandle(hthread);
}
static void die_lasterr(const char *fmt, ...)
{
va_list params;
va_start(params, fmt);
errno = err_win_to_posix(GetLastError());
die_errno(fmt, params);
va_end(params);
}
#undef dup2
int winansi_dup2(int oldfd, int newfd)
{
int ret = dup2(oldfd, newfd);
if (!ret && newfd >= 0 && newfd <= 2)
fd_is_interactive[newfd] = oldfd < 0 || oldfd > 2 ?
0 : fd_is_interactive[oldfd];
return ret;
}
static HANDLE duplicate_handle(HANDLE hnd)
{
HANDLE hresult, hproc = GetCurrentProcess();
if (!DuplicateHandle(hproc, hnd, hproc, &hresult, 0, TRUE,
DUPLICATE_SAME_ACCESS))
die_lasterr("DuplicateHandle(%li) failed",
(long) (intptr_t) hnd);
return hresult;
}
static HANDLE swap_osfhnd(int fd, HANDLE new_handle)
{
HANDLE handle = (HANDLE)_get_osfhandle(fd);
HANDLE duplicate = duplicate_handle(handle);
int new_fd = _open_osfhandle((intptr_t)new_handle, O_BINARY);
assert((fd == 1) || (fd == 2));
if (console == handle)
console = duplicate;
dup2(new_fd, fd);
close(new_fd);
if (fd == 2)
setvbuf(stderr, NULL, _IONBF, BUFSIZ);
fd_is_interactive[fd] |= FD_SWAPPED;
return duplicate;
}
#if defined(DETECT_MSYS_TTY)
#include <winternl.h>
#if defined(_MSC_VER)
typedef struct _OBJECT_NAME_INFORMATION
{
UNICODE_STRING Name;
WCHAR NameBuffer[FLEX_ARRAY];
} OBJECT_NAME_INFORMATION, *POBJECT_NAME_INFORMATION;
#define ObjectNameInformation 1
#else
#include <ntstatus.h>
#endif
static void detect_msys_tty(int fd)
{
ULONG result;
BYTE buffer[1024];
POBJECT_NAME_INFORMATION nameinfo = (POBJECT_NAME_INFORMATION) buffer;
PWSTR name;
HANDLE h = (HANDLE) _get_osfhandle(fd);
if (GetFileType(h) != FILE_TYPE_PIPE)
return;
if (!NT_SUCCESS(NtQueryObject(h, ObjectNameInformation,
buffer, sizeof(buffer) - 2, &result)))
return;
name = nameinfo->Name.Buffer;
name[nameinfo->Name.Length / sizeof(*name)] = 0;
if ((!wcsstr(name, L"msys-") && !wcsstr(name, L"cygwin-")) ||
!wcsstr(name, L"-pty"))
return;
if (fd == 2)
setvbuf(stderr, NULL, _IONBF, BUFSIZ);
fd_is_interactive[fd] |= FD_MSYS;
}
#endif
#undef isatty
int winansi_isatty(int fd)
{
if (fd >= 0 && fd <= 2)
return fd_is_interactive[fd] != 0;
return isatty(fd);
}
void winansi_init(void)
{
int con1, con2;
wchar_t name[32];
con1 = is_console(1);
con2 = is_console(2);
is_console(0);
if (!con1 && !con2) {
#if defined(DETECT_MSYS_TTY)
detect_msys_tty(0);
detect_msys_tty(1);
detect_msys_tty(2);
#endif
return;
}
if (swprintf(name, ARRAY_SIZE(name) - 1, L"\\\\.\\pipe\\winansi%lu",
GetCurrentProcessId()) < 0)
die("Could not initialize winansi pipe name");
hwrite = CreateNamedPipeW(name, PIPE_ACCESS_OUTBOUND,
PIPE_TYPE_BYTE | PIPE_WAIT, 1, BUFFER_SIZE, 0, 0, NULL);
if (hwrite == INVALID_HANDLE_VALUE)
die_lasterr("CreateNamedPipe failed");
hread = CreateFileW(name, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
if (hread == INVALID_HANDLE_VALUE)
die_lasterr("CreateFile for named pipe failed");
hthread = CreateThread(NULL, 0, console_thread, NULL, 0, NULL);
if (hthread == INVALID_HANDLE_VALUE)
die_lasterr("CreateThread(console_thread) failed");
if (atexit(winansi_exit))
die_errno("atexit(winansi_exit) failed");
if (con1)
hconsole1 = swap_osfhnd(1, duplicate_handle(hwrite));
if (con2)
hconsole2 = swap_osfhnd(2, duplicate_handle(hwrite));
}
HANDLE winansi_get_osfhandle(int fd)
{
HANDLE ret;
if (fd == 1 && (fd_is_interactive[1] & FD_SWAPPED))
return hconsole1;
if (fd == 2 && (fd_is_interactive[2] & FD_SWAPPED))
return hconsole2;
ret = (HANDLE)_get_osfhandle(fd);
return ret == (HANDLE)-2 ? INVALID_HANDLE_VALUE : ret;
}
