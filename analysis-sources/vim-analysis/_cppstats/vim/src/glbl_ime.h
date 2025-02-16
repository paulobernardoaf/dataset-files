#if defined(GLOBAL_IME)
#if !defined(_INC_GLOBAL_IME)
#define _INC_GLOBAL_IME
#if defined(__cplusplus)
extern "C" {
#endif
void global_ime_init(ATOM, HWND);
void global_ime_end(void);
LRESULT WINAPI global_ime_DefWindowProc(HWND, UINT, WPARAM, LPARAM);
BOOL WINAPI global_ime_TranslateMessage(CONST MSG *);
void WINAPI global_ime_set_position(POINT*);
void WINAPI global_ime_set_font(LOGFONT*);
#if 0
void WINAPI global_ime_status_evacuate(void);
void WINAPI global_ime_status_restore(void);
#endif
void WINAPI global_ime_set_status(int status);
int WINAPI global_ime_get_status(void);
#if defined(__cplusplus)
}
#endif
#endif 
#endif 
