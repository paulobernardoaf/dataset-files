






































#include <windows.h>
#include <process.h>
#include <stdio.h>





BOOL (WINAPI *_try_enter_critical_section)(LPCRITICAL_SECTION) = NULL;




static HINSTANCE _h_kernel32;


int
main()
{
LPCRITICAL_SECTION lpcs = NULL;

SetLastError(0);

printf("Last Error [main enter] %ld\n", (long) GetLastError());




_h_kernel32 = LoadLibrary(TEXT("KERNEL32.DLL"));
_try_enter_critical_section =
(BOOL (PT_STDCALL *)(LPCRITICAL_SECTION))
GetProcAddress(_h_kernel32,
(LPCSTR) "TryEnterCriticalSection");

if (_try_enter_critical_section != NULL)
{
SetLastError(0);

(*_try_enter_critical_section)(lpcs);

printf("Last Error [try enter] %ld\n", (long) GetLastError());
}

(void) FreeLibrary(_h_kernel32);

printf("This system %s TryEnterCriticalSection.\n",
(_try_enter_critical_section == NULL) ? "DOES NOT SUPPORT" : "SUPPORTS");
printf("POSIX Mutexes will be based on Win32 %s.\n",
(_try_enter_critical_section == NULL) ? "Mutexes" : "Critical Sections");

return(0);
}

