#include "vim.h"
#if defined(VIMDLL)
__declspec(dllimport)
#endif
int VimMain(int argc, char **argv);
#if !defined(VIMDLL)
void SaveInst(HINSTANCE hInst);
#endif
#if !defined(PROTO)
#if defined(FEAT_GUI)
int WINAPI
wWinMain(
HINSTANCE hInstance,
HINSTANCE hPrevInst UNUSED,
LPWSTR lpszCmdLine UNUSED,
int nCmdShow UNUSED)
#else
int
wmain(int argc UNUSED, wchar_t **argv UNUSED)
#endif
{
#if !defined(VIMDLL)
#if defined(FEAT_GUI)
SaveInst(hInstance);
#else
SaveInst(GetModuleHandleW(NULL));
#endif
#endif
VimMain(0, NULL);
return 0;
}
#endif
