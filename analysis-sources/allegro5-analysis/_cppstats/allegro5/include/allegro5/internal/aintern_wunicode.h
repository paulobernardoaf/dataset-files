#if defined(ALLEGRO_WINDOWS)
#include <tchar.h>
#if defined(__cplusplus)
extern "C" {
#endif
AL_FUNC(wchar_t *, _al_win_ustr_to_utf16, (const ALLEGRO_USTR *u));
AL_FUNC(char *, _al_win_ustr_to_ansi, (const ALLEGRO_USTR *u));
AL_FUNC(wchar_t *, _al_win_utf8_to_utf16, (const char *us));
AL_FUNC(char *, _al_win_utf16_to_utf8, (const wchar_t *ws));
AL_FUNC(char *, _al_win_utf8_to_ansi, (const char* us));
AL_FUNC(char *, _al_win_ansi_to_utf8, (const char *s));
AL_FUNC(char *, _al_win_copy_utf16_to_utf8, (char* us, const wchar_t* ws, size_t uslen));
AL_FUNC(char *, _al_win_copy_utf8_to_utf16, (wchar_t* ws, const char *us, size_t wslen));
AL_FUNC(char *, _al_win_copy_ansi_to_utf8, (char* us, const char *s, size_t uslen));
AL_FUNC(char *, _al_win_copy_utf8_to_ansi, (char* s, const char *us, size_t slen));
#if defined(UNICODE)
#define _twin_tchar_strlen wcslen
#define _twin_ustr_to_tchar _al_win_ustr_to_utf16
#define _twin_utf8_to_tchar _al_win_utf8_to_utf16
#define _twin_tchar_to_utf8 _al_win_utf16_to_utf8
#define _twin_copy_tchar_to_utf8 _al_win_copy_utf16_to_utf8
#define _twin_copy_utf8_to_tchar _al_win_copy_utf8_to_utf16
#else
#define _twin_tchar_strlen strlen
#define _twin_ustr_to_tchar _al_win_ustr_to_ansi
#define _twin_utf8_to_tchar _al_win_utf8_to_ansi
#define _twin_tchar_to_utf8 _al_win_ansi_to_utf8
#define _twin_copy_tchar_to_utf8 _al_win_copy_ansi_to_utf8
#define _twin_copy_utf8_to_tchar _al_win_copy_utf8_to_ansi
#endif
#if defined(__cplusplus)
}
#endif
#endif
