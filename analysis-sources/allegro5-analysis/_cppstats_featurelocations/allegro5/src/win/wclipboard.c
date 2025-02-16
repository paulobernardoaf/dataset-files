

















#define _WIN32_WINNT 0x0501
#if !defined(WINVER)
#define WINVER 0x0501
#endif

#include <windows.h>
#include <windowsx.h>


#include <allegro5/allegro.h>

#include "allegro5/allegro_windows.h"
#include "allegro5/internal/aintern.h"
#include "allegro5/internal/aintern_wclipboard.h"
#include "allegro5/internal/aintern_display.h"
#include "allegro5/internal/aintern_wunicode.h"
#include "allegro5/platform/aintwin.h"


ALLEGRO_DEBUG_CHANNEL("clipboard")

#if defined(UNICODE)
#define TEXT_FORMAT CF_UNICODETEXT
#else
#define TEXT_FORMAT CF_TEXT
#endif



static HWND get_window_handle(ALLEGRO_DISPLAY *display)
{
ALLEGRO_DISPLAY_WIN *win_display = (ALLEGRO_DISPLAY_WIN *)display;
if (!win_display)
return NULL;

return win_display->window;
}

static bool win_set_clipboard_text(ALLEGRO_DISPLAY *display, const char *text)
{
HWND handle = get_window_handle(display);
HANDLE hMem = NULL;
TCHAR *tstr = NULL;
size_t size;
size_t len;
LPTSTR dst;

if (!OpenClipboard(handle)) {
ALLEGRO_DEBUG("Could not open clipboard for handle %p", handle);
return false;
}


tstr = _twin_utf8_to_tchar(text);
len = _twin_tchar_strlen(tstr);
size = (len+1) * sizeof(TCHAR);

hMem = GlobalAlloc(GMEM_MOVEABLE, size);

if (!hMem) {
al_free(tstr);
ALLEGRO_DEBUG("GlobalAlloc failed to allocate memory for the clipboard data");
return false;
}

dst = (LPTSTR)GlobalLock(hMem);


memmove(dst, tstr, size);
dst[len] = 0;
GlobalUnlock(hMem);
EmptyClipboard();
if (!SetClipboardData(TEXT_FORMAT, hMem)) {
al_free(tstr);
ALLEGRO_DEBUG("Couldn't set clipboard data");
return false;
}
al_free(tstr);
CloseClipboard();
return true;
}

static char *win_get_clipboard_text(ALLEGRO_DISPLAY *display)
{
char *text;

text = NULL;
if (IsClipboardFormatAvailable(TEXT_FORMAT) &&
OpenClipboard(get_window_handle(display))) {
HANDLE hMem;
LPTSTR tstr;

hMem = GetClipboardData(TEXT_FORMAT);
if (hMem) {
tstr = (LPTSTR)GlobalLock(hMem);
text = _twin_tchar_to_utf8(tstr);
GlobalUnlock(hMem);
} else {
ALLEGRO_DEBUG("Couldn't get clipboard data");
}
CloseClipboard();
}
return text;
}

static bool win_has_clipboard_text(ALLEGRO_DISPLAY *display)
{
if (!IsClipboardFormatAvailable(TEXT_FORMAT))
return false;
if (!OpenClipboard(get_window_handle(display)))
return false;

CloseClipboard();
return true;
}


void _al_win_add_clipboard_functions(ALLEGRO_DISPLAY_INTERFACE *vt)
{
vt->set_clipboard_text = win_set_clipboard_text;
vt->get_clipboard_text = win_get_clipboard_text;
vt->has_clipboard_text = win_has_clipboard_text;
}


