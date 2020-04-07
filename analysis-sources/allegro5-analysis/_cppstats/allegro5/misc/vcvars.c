#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <io.h>
int read_registry(char *path, char *var, char *val, int size)
{
HKEY key;
if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, path, 0, KEY_READ, &key) != ERROR_SUCCESS)
return 0;
if (RegQueryValueEx(key, var, NULL, NULL, val, &size) != ERROR_SUCCESS) {
RegCloseKey(key);
return 0;
}
RegCloseKey(key);
return 1;
}
void find_vcvars()
{
char data[256], name[256];
int i;
for (i=9; i>=4; i--) {
sprintf(name, "Software\\Microsoft\\DevStudio\\%d.0\\Products\\Microsoft Visual C++", i);
if (read_registry(name, "ProductDir", data, sizeof(data))) {
strcat(data, "\\bin\\vcvars32.bat");
if (access(data, 4) == 0) {
printf("Found %s\n", data);
break;
}
}
data[0] = 0;
}
if (!data[0]) {
printf("\n Unable to find MSVC ProductDir information in your registry!\n\n");
printf(" To install Allegro, I need to know the path where your compiler is\n");
printf(" installed. Somewhere in your MSVC installation directory there will\n");
printf(" be a file called vcvars32.bat, which contains this information.\n");
printf(" Please enter the full path to where I can find that file, for example\n");
printf(" c:\\Program Files\\Microsoft Visual Studio\\VC98\\bin\\vcvars32.bat\n");
for (;;) {
printf("\n> ");
fflush(stdout);
if (gets(data)) {
i = strlen(data) - 12;
if (i < 0)
i = 0;
if (stricmp(data+i, "vcvars32.bat") != 0)
printf("\nError: that path doesn't end in vcvars32.bat!\n");
else if (access(data, 4) != 0)
printf("\nError: can't find a vcvars32.bat file there!\n");
else {
printf("\nUsing %s\n", data);
break;
}
}
data[0] = 0;
}
}
strcpy(name, "VCVARS=");
strcat(name, data);
putenv(name);
}
int main(int argc, char *argv[])
{
char cmd[256];
int i;
find_vcvars();
if ((getenv("OS")) && (stricmp(getenv("OS"), "Windows_NT") == 0)) {
if (argc > 1) {
strcpy(cmd, "cmd.exe /e:8192 /c");
}
else {
sprintf(cmd, "cmd.exe /e:8192 /k \"%s\"", getenv("VCVARS"));
}
}
else {
if (argc > 1) {
strcpy(cmd, "command.com /e:8192 /c");
}
else {
sprintf(cmd, "command.com /e:8192 /k \"%s\"", getenv("VCVARS"));
}
}
for (i=1; i<argc; i++) {
strcat(cmd, " ");
strcat(cmd, argv[i]);
}
printf("Invoking nested command interpreter\n");
system(cmd);
return 0;
}
