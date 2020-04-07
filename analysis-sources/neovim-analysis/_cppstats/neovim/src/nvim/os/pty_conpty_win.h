#if !defined(HPCON)
#define HPCON VOID *
#endif
extern HRESULT (WINAPI *pCreatePseudoConsole) 
(COORD, HANDLE, HANDLE, DWORD, HPCON *);
extern HRESULT (WINAPI *pResizePseudoConsole)(HPCON, COORD);
extern void (WINAPI *pClosePseudoConsole)(HPCON);
typedef struct conpty {
HPCON pty;
STARTUPINFOEXW si_ex;
} conpty_t;
#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "os/pty_conpty_win.h.generated.h"
#endif
