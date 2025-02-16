#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
int
main(void)
{
const wchar_t *p;
wchar_t *cmd;
size_t cmdlen;
int retval;
int inquote = 0;
int silent = 0;
HANDLE hstdout;
DWORD written;
p = (const wchar_t *)GetCommandLineW();
while (*p)
{
if (*p == L'"')
inquote = !inquote;
else if (!inquote && *p == L' ')
{
++p;
break;
}
++p;
}
while (*p == L' ')
++p;
if (p[0] == L'-' && p[1] == L's' && p[2] == L' ')
{
silent = 1;
p += 3;
while (*p == L' ')
++p;
}
hstdout = GetStdHandle(STD_OUTPUT_HANDLE);
WriteConsoleW(hstdout, p, wcslen(p), &written, NULL);
WriteConsoleW(hstdout, L"\r\n", 2, &written, NULL);
cmd = NULL;
cmdlen = wcslen(p);
if (cmdlen >= 2 && p[0] == L'"' && p[cmdlen - 1] == L'"')
{
cmdlen += 3;
cmd = (wchar_t *)malloc(cmdlen * sizeof(wchar_t));
if (cmd == NULL)
{
perror("vimrun malloc(): ");
return -1;
}
_snwprintf(cmd, cmdlen, L"(%s)", p);
p = cmd;
}
retval = _wsystem(p);
if (cmd)
free(cmd);
if (retval == -1)
perror("vimrun system(): ");
else if (retval != 0)
printf("shell returned %d\n", retval);
if (!silent)
{
puts("Hit any key to close this window...");
while (_kbhit())
(void)_getch();
(void)_getch();
}
return retval;
}
