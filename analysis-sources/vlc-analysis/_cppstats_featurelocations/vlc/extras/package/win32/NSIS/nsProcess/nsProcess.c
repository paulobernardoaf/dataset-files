



















#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tlhelp32.h>
#include "pluginapi.h"


#define NSIS_MAX_STRLEN 1024

#define SystemProcessInformation 5
#define STATUS_SUCCESS 0x00000000L
#define STATUS_INFO_LENGTH_MISMATCH 0xC0000004L

typedef struct _SYSTEM_THREAD_INFO {
FILETIME ftCreationTime;
DWORD dwUnknown1;
DWORD dwStartAddress;
DWORD dwOwningPID;
DWORD dwThreadID;
DWORD dwCurrentPriority;
DWORD dwBasePriority;
DWORD dwContextSwitches;
DWORD dwThreadState;
DWORD dwUnknown2;
DWORD dwUnknown3;
DWORD dwUnknown4;
DWORD dwUnknown5;
DWORD dwUnknown6;
DWORD dwUnknown7;
} SYSTEM_THREAD_INFO;

typedef struct _SYSTEM_PROCESS_INFO {
DWORD dwOffset;
DWORD dwThreadCount;
DWORD dwUnkown1[6];
FILETIME ftCreationTime;
DWORD dwUnkown2;
DWORD dwUnkown3;
DWORD dwUnkown4;
DWORD dwUnkown5;
DWORD dwUnkown6;
WCHAR *pszProcessName;
DWORD dwBasePriority;
DWORD dwProcessID;
DWORD dwParentProcessID;
DWORD dwHandleCount;
DWORD dwUnkown7;
DWORD dwUnkown8;
DWORD dwVirtualBytesPeak;
DWORD dwVirtualBytes;
DWORD dwPageFaults;
DWORD dwWorkingSetPeak;
DWORD dwWorkingSet;
DWORD dwUnkown9;
DWORD dwPagedPool;
DWORD dwUnkown10;
DWORD dwNonPagedPool;
DWORD dwPageFileBytesPeak;
DWORD dwPageFileBytes;
DWORD dwPrivateBytes;
DWORD dwUnkown11;
DWORD dwUnkown12;
DWORD dwUnkown13;
DWORD dwUnkown14;
SYSTEM_THREAD_INFO ati[ANYSIZE_ARRAY];
} SYSTEM_PROCESS_INFO;







TCHAR szBuf[NSIS_MAX_STRLEN];


int FIND_PROC_BY_NAME(TCHAR *szProcessName, BOOL bTerminate, BOOL bClose);


void __declspec(dllexport) _FindProcess(HWND hwndParent, int string_size,
TCHAR *variables, stack_t **stacktop, extra_parameters *extra)
{
EXDLL_INIT();
{
int nError;

popstringn(szBuf, NSIS_MAX_STRLEN);
nError=FIND_PROC_BY_NAME(szBuf, FALSE, FALSE);
pushint(nError);
}
}

void __declspec(dllexport) _KillProcess(HWND hwndParent, int string_size,
TCHAR *variables, stack_t **stacktop, extra_parameters *extra)
{
EXDLL_INIT();
{
int nError=0;

popstringn(szBuf, NSIS_MAX_STRLEN);
nError=FIND_PROC_BY_NAME(szBuf, TRUE, FALSE);
pushint(nError);
}
}

void __declspec(dllexport) _CloseProcess(HWND hwndParent, int string_size,
TCHAR *variables, stack_t **stacktop, extra_parameters *extra)
{
EXDLL_INIT();
{
int nError=0;

popstringn(szBuf, NSIS_MAX_STRLEN);
nError=FIND_PROC_BY_NAME(szBuf, TRUE, TRUE);
pushint(nError);
}
}

void __declspec(dllexport) _Unload(HWND hwndParent, int string_size,
TCHAR *variables, stack_t **stacktop, extra_parameters *extra)
{
}

BOOL WINAPI DllMain(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
return TRUE;
}

BOOL CALLBACK EnumWindowsProc( HWND hwnd,
LPARAM lParam
)
{
HANDLE *data = lParam;
DWORD pid;
GetWindowThreadProcessId(hwnd, &pid);
if (pid == data[0])
{
PostMessage(data[1], WM_CLOSE, 0, 0);
data[1] = hwnd;
}
return TRUE;
}

void NiceTerminate(DWORD id, BOOL bClose, BOOL *bSuccess, BOOL *bFailed)
{
HANDLE hProc;
HANDLE data[2];
DWORD ec;
BOOL bDone = FALSE;
if (hProc=OpenProcess(PROCESS_TERMINATE | PROCESS_QUERY_INFORMATION | SYNCHRONIZE, FALSE, id))
{
data[0] = id;
data[1] = NULL;

if (bClose)
EnumWindows(EnumWindowsProc, data);
if (data[1] != NULL)
{ 
if (GetExitCodeProcess(hProc,&ec) && ec == STILL_ACTIVE)
if (WaitForSingleObject(hProc, 3000) == WAIT_OBJECT_0)
{
*bSuccess = bDone = TRUE;
}
else;
else 
{
*bSuccess = bDone = TRUE;
}
}
if (!bDone)
{

if (TerminateProcess(hProc, 0))
*bSuccess=TRUE;
else
*bFailed=TRUE;
}
CloseHandle(hProc);
}
}

int FIND_PROC_BY_NAME(TCHAR *szProcessName, BOOL bTerminate, BOOL bClose)


















































{
TCHAR szName[MAX_PATH];
OSVERSIONINFO osvi;
HMODULE hLib;
HANDLE hProc;
ULONG uError;
BOOL bFound=FALSE;
BOOL bSuccess=FALSE;
BOOL bFailed=FALSE;


osvi.dwOSVersionInfoSize=sizeof(OSVERSIONINFO);
if (!GetVersionEx(&osvi)) return 604;

if (osvi.dwPlatformId != VER_PLATFORM_WIN32_NT &&
osvi.dwPlatformId != VER_PLATFORM_WIN32_WINDOWS)
return 605;

if (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT)
{


SYSTEM_PROCESS_INFO *spi;
SYSTEM_PROCESS_INFO *spiCount;
DWORD dwSize=0x4000;
DWORD dwData;
ULONG (WINAPI *NtQuerySystemInformationPtr)(ULONG, PVOID, LONG, PULONG);

if (hLib=LoadLibraryW(L"NTDLL.DLL"))
{
NtQuerySystemInformationPtr=(ULONG(WINAPI *)(ULONG, PVOID, LONG, PULONG))GetProcAddress(hLib, "NtQuerySystemInformation");

if (NtQuerySystemInformationPtr)
{
while (1)
{
if (spi=LocalAlloc(LMEM_FIXED, dwSize))
{
uError=(*NtQuerySystemInformationPtr)(SystemProcessInformation, spi, dwSize, &dwData);

if (uError == STATUS_SUCCESS) break;

LocalFree(spi);

if (uError != STATUS_INFO_LENGTH_MISMATCH)
{
uError=608;
break;
}
}
else
{
uError=608;
break;
}
dwSize*=2;
}
}
else uError=607;

FreeLibrary(hLib);
}
else uError=606;

if (uError != STATUS_SUCCESS) return uError;

spiCount=spi;

while (1)
{
if (spiCount->pszProcessName)
{

#if defined(UNICODE)
lstrcpyn(szName, spiCount->pszProcessName, MAX_PATH);
#else
WideCharToMultiByte(CP_ACP, 0, spiCount->pszProcessName, -1, szName, MAX_PATH, NULL, NULL);
#endif 

if (!lstrcmpi(szName, szProcessName))
{

bFound=TRUE;

if (bTerminate == TRUE)
{
NiceTerminate(spiCount->dwProcessID, bClose, &bSuccess, &bFailed);
}
else break;
}
}
if (spiCount->dwOffset == 0) break;
spiCount=(SYSTEM_PROCESS_INFO *)((char *)spiCount + spiCount->dwOffset);
}
LocalFree(spi);
}
else
{


PROCESSENTRY32 pe;
char *pName;
HANDLE hSnapShot;
BOOL bResult;
HANDLE (WINAPI *CreateToolhelp32SnapshotPtr)(DWORD, DWORD);
BOOL (WINAPI *Process32FirstPtr)(HANDLE, LPPROCESSENTRY32);
BOOL (WINAPI *Process32NextPtr)(HANDLE, LPPROCESSENTRY32);

if (hLib=LoadLibraryA("KERNEL32.DLL"))
{
CreateToolhelp32SnapshotPtr=(HANDLE(WINAPI *)(DWORD, DWORD)) GetProcAddress(hLib, "CreateToolhelp32Snapshot");
Process32FirstPtr=(BOOL(WINAPI *)(HANDLE, LPPROCESSENTRY32)) GetProcAddress(hLib, "Process32First");
Process32NextPtr=(BOOL(WINAPI *)(HANDLE, LPPROCESSENTRY32)) GetProcAddress(hLib, "Process32Next");

if (CreateToolhelp32SnapshotPtr && Process32NextPtr && Process32FirstPtr)
{

if ((hSnapShot=(*CreateToolhelp32SnapshotPtr)(TH32CS_SNAPPROCESS, 0)) != INVALID_HANDLE_VALUE)
{

pe.dwSize=sizeof(PROCESSENTRY32);
bResult=(*Process32FirstPtr)(hSnapShot, &pe);


while (bResult)
{

for (pName=pe.szExeFile + lstrlen(pe.szExeFile) - 1; *pName != '\\' && *pName != '\0'; --pName);

++pName;

#if defined(UNICODE)
MultiByteToWideChar(CP_ACP, 0, pName, lstrlenA(pName)+1, szName, MAX_PATH);
#else
lstrcpyn(szName, pName, MAX_PATH);
#endif 

if (!lstrcmpi(szName, szProcessName))
{

bFound=TRUE;

if (bTerminate == TRUE)
{

NiceTerminate(pe.th32ProcessID, bClose, &bSuccess, &bFailed);
}
else break;
}

bResult=(*Process32NextPtr)(hSnapShot, &pe);
}
CloseHandle(hSnapShot);
}
else uError=611;
}
else uError=610;

FreeLibrary(hLib);
}
else uError=609;
}

if (bFound == FALSE) return 603;
if (bTerminate == TRUE)
{
if (bSuccess == FALSE) return 601;
if (bFailed == TRUE) return 602;
}
return 0;
}
