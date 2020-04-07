#define DOSINST
#include "dosinst.h"
#include <io.h>
#define GVIMEXT64_PATH "GvimExt64\\gvimext.dll"
#define GVIMEXT32_PATH "GvimExt32\\gvimext.dll"
#define CHECK_REG_ERROR(code) do { if (code != ERROR_SUCCESS) { printf("%ld error number: %ld\n", (long)__LINE__, (long)code); return 1; } } while (0)
int has_vim = 0; 
int has_gvim = 0; 
char oldvimrc[BUFSIZE]; 
char vimrc[BUFSIZE]; 
char *default_bat_dir = NULL; 
char *default_vim_dir = NULL; 
struct choice
{
int active; 
char *text; 
void (*changefunc)(int idx); 
int arg; 
void (*installfunc)(int idx); 
};
struct choice choices[30]; 
int choice_count = 0; 
#define TABLE_SIZE(s) (int)(sizeof(s) / sizeof(*s))
enum
{
compat_vi = 1,
compat_vim,
compat_some_enhancements,
compat_all_enhancements
};
char *(compat_choices[]) =
{
"\nChoose the default way to run Vim:",
"Vi compatible",
"Vim default",
"with some Vim enhancements",
"with syntax highlighting and other features switched on",
};
int compat_choice = (int)compat_all_enhancements;
char *compat_text = "- run Vim %s";
enum
{
remap_no = 1,
remap_win
};
char *(remap_choices[]) =
{
"\nChoose:",
"Do not remap keys for Windows behavior",
"Remap a few keys for Windows behavior (CTRL-V, CTRL-C, CTRL-F, etc)",
};
int remap_choice = (int)remap_no;
char *remap_text = "- %s";
enum
{
mouse_xterm = 1,
mouse_mswin,
mouse_default
};
char *(mouse_choices[]) =
{
"\nChoose the way how Vim uses the mouse:",
"right button extends selection (the Unix way)",
"right button has a popup menu, left button starts select mode (the Windows way)",
"right button has a popup menu, left button starts visual mode",
};
int mouse_choice = (int)mouse_default;
char *mouse_text = "- The mouse %s";
enum
{
vimfiles_dir_none = 1,
vimfiles_dir_vim,
vimfiles_dir_home
};
static char *(vimfiles_dir_choices[]) =
{
"\nCreate plugin directories:",
"No",
"In the VIM directory",
"In your HOME directory",
};
static int install_popup = 0;
static int install_openwith = 0;
static int need_uninstall_entry = 0;
static char *(vimfiles_subdirs[]) =
{
"colors",
"compiler",
"doc",
"ftdetect",
"ftplugin",
"indent",
"keymap",
"plugin",
"syntax",
};
static int
get_choice(char **table, int entries)
{
int answer;
int idx;
char dummy[100];
do
{
for (idx = 0; idx < entries; ++idx)
{
if (idx)
printf("%2d ", idx);
puts(table[idx]);
}
printf("Choice: ");
if (scanf("%d", &answer) != 1)
{
scanf("%99s", dummy);
answer = 0;
}
}
while (answer < 1 || answer >= entries);
return answer;
}
static void
check_unpack(void)
{
char buf[BUFSIZE];
FILE *fd;
struct stat st;
runtimeidx = strlen(installdir) - strlen(VIM_VERSION_NODOT);
if (runtimeidx <= 0
|| stricmp(installdir + runtimeidx, VIM_VERSION_NODOT) != 0
|| (installdir[runtimeidx - 1] != '/'
&& installdir[runtimeidx - 1] != '\\'))
{
printf("ERROR: Install program not in directory \"%s\"\n",
VIM_VERSION_NODOT);
printf("This program can only work when it is located in its original directory\n");
myexit(1);
}
sprintf(buf, "%s\\filetype.vim", installdir);
if (stat(buf, &st) < 0)
{
printf("ERROR: Cannot find filetype.vim in \"%s\"\n", installdir);
printf("It looks like you did not unpack the runtime archive.\n");
printf("You must unpack the runtime archive \"vim%srt.zip\" before installing.\n",
VIM_VERSION_NODOT + 3);
myexit(1);
}
if ((fd = fopen("gvim.exe", "r")) != NULL)
{
fclose(fd);
has_gvim = 1;
}
if ((fd = fopen("vim.exe", "r")) != NULL)
{
fclose(fd);
has_vim = 1;
}
if (!has_gvim && !has_vim)
{
printf("ERROR: Cannot find any Vim executables in \"%s\"\n\n",
installdir);
myexit(1);
}
}
static int
pathcmp(char *p, int plen, char *q, int qlen)
{
int i;
if (plen < 0)
plen = strlen(p);
if (qlen < 0)
qlen = strlen(q);
for (i = 0; ; ++i)
{
if (i == plen)
{
if (i == qlen) 
return 0;
if (i == qlen - 1 && (q[i] == '\\' || q[i] == '/'))
return 0; 
return 1; 
}
if (i == qlen)
{
if (i == plen) 
return 0;
if (i == plen - 1 && (p[i] == '\\' || p[i] == '/'))
return 0; 
return 1; 
}
if (!(mytoupper(p[i]) == mytoupper(q[i])
|| ((p[i] == '/' || p[i] == '\\')
&& (q[i] == '/' || q[i] == '\\'))))
return 1; 
}
}
static void
findoldfile(char **destination)
{
char *bp = *destination;
size_t indir_l = strlen(installdir);
char *cp;
char *tmpname;
char *farname;
if (bp == NULL || strnicmp(bp, installdir, indir_l) != 0)
return;
cp = bp + indir_l;
if (strchr("/\\", *cp++) == NULL
|| strchr(cp, '\\') != NULL
|| strchr(cp, '/') != NULL)
return;
tmpname = alloc(strlen(cp) + 1);
strcpy(tmpname, cp);
tmpname[strlen(tmpname) - 1] = 'x'; 
if (access(tmpname, 0) == 0)
{
printf("\nERROR: %s and %s clash. Remove or rename %s.\n",
tmpname, cp, tmpname);
myexit(1);
}
if (rename(cp, tmpname) != 0)
{
printf("\nERROR: failed to rename %s to %s: %s\n",
cp, tmpname, strerror(0));
myexit(1);
}
farname = searchpath_save(cp);
if (rename(tmpname, cp) != 0)
{
printf("\nERROR: failed to rename %s back to %s: %s\n",
tmpname, cp, strerror(0));
myexit(1);
}
free(*destination);
free(tmpname);
*destination = farname;
}
static void
find_bat_exe(int check_bat_only)
{
int i;
mch_chdir(sysdrive);
mch_chdir("\\");
for (i = 1; i < TARGET_COUNT; ++i)
{
targets[i].oldbat = searchpath_save(targets[i].batname);
if (!check_bat_only)
targets[i].oldexe = searchpath_save(targets[i].exename);
if (default_bat_dir == NULL && targets[i].oldbat != NULL)
{
default_bat_dir = alloc(strlen(targets[i].oldbat) + 1);
strcpy(default_bat_dir, targets[i].oldbat);
remove_tail(default_bat_dir);
}
if (check_bat_only && targets[i].oldbat != NULL)
{
free(targets[i].oldbat);
targets[i].oldbat = NULL;
}
}
mch_chdir(installdir);
}
static void
get_vim_env(void)
{
char *vim;
char buf[BUFSIZE];
FILE *fd;
char fname[BUFSIZE];
vim = getenv("VIMRUNTIME");
if (vim != NULL && *vim != 0 && strlen(vim) < sizeof(buf))
{
strcpy(buf, vim);
remove_tail(buf);
vim = buf;
}
else
{
vim = getenv("VIM");
if (vim == NULL || *vim == 0)
{
if (default_vim_dir != NULL)
vim = default_vim_dir;
else
vim = "";
}
}
GetTempPath(sizeof(fname) - 12, fname);
add_pathsep(fname);
strcat(fname, "vimini.ini");
fd = fopen(fname, "w");
if (fd != NULL)
{
fprintf(fd, "[vimini]\n");
fprintf(fd, "dir=\"%s\"\n", vim);
fclose(fd);
}
else
{
printf("Failed to open %s\n", fname);
sleep(2);
}
}
static int num_windows;
static BOOL CALLBACK
window_cb(HWND hwnd, LPARAM lparam)
{
char title[256];
title[0] = 0;
GetWindowText(hwnd, title, 256);
if (strstr(title, "Vim ") != NULL && strstr(title, " Uninstall") != NULL)
++num_windows;
return TRUE;
}
static int
run_silent_uninstall(char *uninst_exe)
{
char vimrt_dir[BUFSIZE];
char temp_uninst[BUFSIZE];
char temp_dir[MAX_PATH];
char buf[BUFSIZE * 2 + 10];
int i;
DWORD tick;
strcpy(vimrt_dir, uninst_exe);
remove_tail(vimrt_dir);
if (!GetTempPath(sizeof(temp_dir), temp_dir))
return FAIL;
tick = GetTickCount();
for (i = 0; ; i++)
{
sprintf(temp_uninst, "%s\\vimun%04X.exe", temp_dir,
(unsigned int)((i + tick) & 0xFFFF));
if (CopyFile(uninst_exe, temp_uninst, TRUE))
break;
if (GetLastError() != ERROR_FILE_EXISTS)
return FAIL;
if (i == 65535)
return FAIL;
}
if (strchr(temp_uninst, ' ') != NULL)
sprintf(buf, "\"%s\" /S _?=%s", temp_uninst, vimrt_dir);
else
sprintf(buf, "%s /S _?=%s", temp_uninst, vimrt_dir);
run_command(buf);
DeleteFile(temp_uninst);
return OK;
}
static int
uninstall_check(int skip_question)
{
HKEY key_handle;
HKEY uninstall_key_handle;
char *uninstall_key = "software\\Microsoft\\Windows\\CurrentVersion\\Uninstall";
char subkey_name_buff[BUFSIZE];
char temp_string_buffer[BUFSIZE-2];
DWORD local_bufsize;
FILETIME temp_pfiletime;
DWORD key_index;
char input;
long code;
DWORD value_type;
DWORD orig_num_keys;
DWORD new_num_keys;
DWORD allow_silent;
int foundone = 0;
code = RegOpenKeyEx(HKEY_LOCAL_MACHINE, uninstall_key, 0,
KEY_WOW64_64KEY | KEY_READ, &key_handle);
CHECK_REG_ERROR(code);
key_index = 0;
while (TRUE)
{
local_bufsize = sizeof(subkey_name_buff);
if (RegEnumKeyEx(key_handle, key_index, subkey_name_buff, &local_bufsize,
NULL, NULL, NULL, &temp_pfiletime) == ERROR_NO_MORE_ITEMS)
break;
if (strncmp("Vim", subkey_name_buff, 3) == 0)
{
code = RegOpenKeyEx(key_handle, subkey_name_buff, 0,
KEY_WOW64_64KEY | KEY_READ, &uninstall_key_handle);
CHECK_REG_ERROR(code);
local_bufsize = sizeof(temp_string_buffer);
code = RegQueryValueEx(uninstall_key_handle, "displayname", 0,
&value_type, (LPBYTE)temp_string_buffer,
&local_bufsize);
CHECK_REG_ERROR(code);
allow_silent = 0;
if (skip_question)
{
DWORD varsize = sizeof(DWORD);
RegQueryValueEx(uninstall_key_handle, "AllowSilent", 0,
&value_type, (LPBYTE)&allow_silent,
&varsize);
}
foundone = 1;
printf("\n*********************************************************\n");
printf("Vim Install found what looks like an existing Vim version.\n");
printf("The name of the entry is:\n");
printf("\n \"%s\"\n\n", temp_string_buffer);
printf("Installing the new version will disable part of the existing version.\n");
printf("(The batch files used in a console and the \"Edit with Vim\" entry in\n");
printf("the popup menu will use the new version)\n");
if (skip_question)
printf("\nRunning uninstall program for \"%s\"\n", temp_string_buffer);
else
printf("\nDo you want to uninstall \"%s\" now?\n(y)es/(n)o) ", temp_string_buffer);
fflush(stdout);
local_bufsize = sizeof(temp_string_buffer);
code = RegQueryValueEx(uninstall_key_handle, "uninstallstring", 0,
&value_type, (LPBYTE)temp_string_buffer, &local_bufsize);
CHECK_REG_ERROR(code);
default_vim_dir = alloc(strlen(temp_string_buffer) + 1);
strcpy(default_vim_dir, temp_string_buffer);
remove_tail(default_vim_dir);
remove_tail(default_vim_dir);
input = 'n';
do
{
if (input != 'n')
printf("%c is an invalid reply. Please enter either 'y' or 'n'\n", input);
if (skip_question)
input = 'y';
else
{
rewind(stdin);
scanf("%c", &input);
}
switch (input)
{
case 'y':
case 'Y':
RegQueryInfoKey(key_handle, NULL, NULL, NULL,
&orig_num_keys, NULL, NULL, NULL,
NULL, NULL, NULL, NULL);
find_bat_exe(TRUE);
if (allow_silent)
{
if (run_silent_uninstall(temp_string_buffer)
== FAIL)
allow_silent = 0; 
}
if (!allow_silent)
{
{
char buf[BUFSIZE];
if (strchr(temp_string_buffer, ' ') != NULL)
sprintf(buf, "\"%s\"", temp_string_buffer);
else
strcpy(buf, temp_string_buffer);
run_command(buf);
}
sleep(1); 
num_windows = 0;
EnumWindows(window_cb, 0);
if (num_windows == 0)
{
printf("Press Enter when the uninstaller is finished\n");
rewind(stdin);
(void)getchar();
}
else
{
printf("Waiting for the uninstaller to finish (press CTRL-C to abort).");
do
{
printf(".");
fflush(stdout);
sleep(1); 
num_windows = 0;
EnumWindows(window_cb, 0);
} while (num_windows > 0);
}
}
printf("\nDone!\n");
RegQueryInfoKey(key_handle, NULL, NULL, NULL,
&new_num_keys, NULL, NULL, NULL,
NULL, NULL, NULL, NULL);
if (new_num_keys < orig_num_keys)
key_index--;
input = 'y';
break;
case 'n':
case 'N':
input = 'n';
break;
default: 
break;
}
} while (input != 'n' && input != 'y');
RegCloseKey(uninstall_key_handle);
}
key_index++;
}
RegCloseKey(key_handle);
return foundone;
}
static void
inspect_system(void)
{
char *p;
char buf[BUFSIZE];
FILE *fd;
int i;
int foundone;
printf("Inspecting system...\n");
p = getenv("VIM");
if (p != NULL && pathcmp(p, -1, installdir, runtimeidx - 1) != 0)
{
printf("------------------------------------------------------\n");
printf("$VIM is set to \"%s\".\n", p);
printf("This is different from where this version of Vim is:\n");
strcpy(buf, installdir);
*(buf + runtimeidx - 1) = NUL;
printf("\"%s\"\n", buf);
printf("You must adjust or remove the setting of $VIM,\n");
if (interactive)
{
printf("to be able to use this install program.\n");
myexit(1);
}
printf("otherwise Vim WILL NOT WORK properly!\n");
printf("------------------------------------------------------\n");
}
p = getenv("VIMRUNTIME");
if (p != NULL && pathcmp(p, -1, installdir, -1) != 0)
{
printf("------------------------------------------------------\n");
printf("$VIMRUNTIME is set to \"%s\".\n", p);
printf("This is different from where this version of Vim is:\n");
printf("\"%s\"\n", installdir);
printf("You must adjust or remove the setting of $VIMRUNTIME,\n");
if (interactive)
{
printf("to be able to use this install program.\n");
myexit(1);
}
printf("otherwise Vim WILL NOT WORK properly!\n");
printf("------------------------------------------------------\n");
}
find_bat_exe(FALSE);
foundone = 0;
for (i = 1; i < TARGET_COUNT; ++i)
{
findoldfile(&(targets[i].oldexe));
if (targets[i].oldexe != NULL)
foundone = 1;
}
if (foundone)
{
printf("Warning: Found Vim executable(s) in your $PATH:\n");
for (i = 1; i < TARGET_COUNT; ++i)
if (targets[i].oldexe != NULL)
printf("%s\n", targets[i].oldexe);
printf("It will be used instead of the version you are installing.\n");
printf("Please delete or rename it, or adjust your $PATH setting.\n");
}
strcpy(oldvimrc, installdir);
strcpy(oldvimrc + runtimeidx, "_vimrc");
if ((fd = fopen(oldvimrc, "r")) == NULL)
{
strcpy(oldvimrc + runtimeidx, "vimrc~1"); 
if ((fd = fopen(oldvimrc, "r")) == NULL)
{
strcpy(oldvimrc + runtimeidx, ".vimrc");
fd = fopen(oldvimrc, "r");
}
}
if (fd != NULL)
fclose(fd);
else
*oldvimrc = NUL;
}
static void
add_dummy_choice(void)
{
choices[choice_count].installfunc = NULL;
choices[choice_count].active = 0;
choices[choice_count].changefunc = NULL;
choices[choice_count].text = NULL;
choices[choice_count].arg = 0;
++choice_count;
}
static void
install_bat_choice(int idx)
{
char *batpath = targets[choices[idx].arg].batpath;
char *oldname = targets[choices[idx].arg].oldbat;
char *exename = targets[choices[idx].arg].exenamearg;
char *vimarg = targets[choices[idx].arg].exearg;
FILE *fd;
if (*batpath != NUL)
{
fd = fopen(batpath, "w");
if (fd == NULL)
printf("\nERROR: Cannot open \"%s\" for writing.\n", batpath);
else
{
need_uninstall_entry = 1;
fprintf(fd, "@echo off\n");
fprintf(fd, "rem -- Run Vim --\n");
fprintf(fd, VIMBAT_UNINSTKEY "\n");
fprintf(fd, "\n");
fprintf(fd, "setlocal\n");
fprintf(fd, "set VIM_EXE_DIR=%s\n", installdir);
fprintf(fd, "if exist \"%%VIM%%\\%s\\%s\" set VIM_EXE_DIR=%%VIM%%\\%s\n",
VIM_VERSION_NODOT, exename, VIM_VERSION_NODOT);
fprintf(fd, "if exist \"%%VIMRUNTIME%%\\%s\" set VIM_EXE_DIR=%%VIMRUNTIME%%\n", exename);
fprintf(fd, "\n");
fprintf(fd, "if not exist \"%%VIM_EXE_DIR%%\\%s\" (\n", exename);
fprintf(fd, " echo \"%%VIM_EXE_DIR%%\\%s\" not found\n", exename);
fprintf(fd, " goto :eof\n");
fprintf(fd, ")\n");
fprintf(fd, "\n");
if (*exename == 'g')
{
fprintf(fd, "rem check --nofork argument\n");
fprintf(fd, "set VIMNOFORK=\n");
fprintf(fd, ":loopstart\n");
fprintf(fd, "if .%%1==. goto loopend\n");
fprintf(fd, "if .%%1==.--nofork (\n");
fprintf(fd, " set VIMNOFORK=1\n");
fprintf(fd, ") else if .%%1==.-f (\n");
fprintf(fd, " set VIMNOFORK=1\n");
fprintf(fd, ")\n");
fprintf(fd, "shift\n");
fprintf(fd, "goto loopstart\n");
fprintf(fd, ":loopend\n");
fprintf(fd, "\n");
}
if (*exename == 'g')
{
fprintf(fd, "if .%%VIMNOFORK%%==.1 (\n");
fprintf(fd, " start \"dummy\" /b /wait ");
fprintf(fd, "\"%%VIM_EXE_DIR%%\\%s\" %s %%*\n",
exename, vimarg);
fprintf(fd, ") else (\n");
fprintf(fd, " start \"dummy\" /b ");
fprintf(fd, "\"%%VIM_EXE_DIR%%\\%s\" %s %%*\n",
exename, vimarg);
fprintf(fd, ")\n");
}
else
{
fprintf(fd, "\"%%VIM_EXE_DIR%%\\%s\" %s %%*\n",
exename, vimarg);
}
fclose(fd);
printf("%s has been %s\n", batpath,
oldname == NULL ? "created" : "overwritten");
}
}
}
static void
alloc_text(int idx, char *fmt, char *arg)
{
if (choices[idx].text != NULL)
free(choices[idx].text);
choices[idx].text = alloc(strlen(fmt) + strlen(arg) - 1);
sprintf(choices[idx].text, fmt, arg);
}
static void
toggle_bat_choice(int idx)
{
char *batname = targets[choices[idx].arg].batpath;
char *oldname = targets[choices[idx].arg].oldbat;
if (*batname == NUL)
{
alloc_text(idx, " Overwrite %s", oldname);
strcpy(batname, oldname);
}
else
{
alloc_text(idx, " Do NOT overwrite %s", oldname);
*batname = NUL;
}
}
static void
set_bat_text(int idx, char *batpath, char *name)
{
strcat(batpath, name);
alloc_text(idx, " Create %s", batpath);
}
static void
change_bat_choice(int idx)
{
char *path;
char *batpath;
char *name;
int n;
char *s;
char *p;
int count;
char **names = NULL;
int i;
int target = choices[idx].arg;
name = targets[target].batname;
batpath = targets[target].batpath;
path = getenv("PATH");
if (path == NULL)
{
printf("\nERROR: The variable $PATH is not set\n");
return;
}
for (;;)
{
count = 1;
for (p = path; *p; )
{
s = strchr(p, ';');
if (s == NULL)
s = p + strlen(p);
if (names != NULL)
{
names[count] = alloc(s - p + 1);
strncpy(names[count], p, s - p);
names[count][s - p] = NUL;
}
++count;
p = s;
if (*p != NUL)
++p;
}
if (names != NULL)
break;
names = alloc((count + 1) * sizeof(char *));
}
names[0] = alloc(50);
sprintf(names[0], "Select directory to create %s in:", name);
names[count] = alloc(50);
if (choices[idx].arg == 0)
sprintf(names[count], "Do not create any .bat file.");
else
sprintf(names[count], "Do not create a %s file.", name);
n = get_choice(names, count + 1);
if (n == count)
{
*batpath = NUL;
if (choices[idx].arg != 0)
alloc_text(idx, " Do NOT create %s", name);
}
else
{
strcpy(batpath, names[n]);
add_pathsep(batpath);
if (choices[idx].arg != 0)
set_bat_text(idx, batpath, name);
}
for (i = 0; i <= count; ++i)
free(names[i]);
free(names);
}
char *bat_text_yes = "Install .bat files to use Vim at the command line:";
char *bat_text_no = "do NOT install .bat files to use Vim at the command line";
static void
change_main_bat_choice(int idx)
{
int i;
change_bat_choice(idx);
if (targets[0].batpath[0] != NUL)
choices[idx].text = bat_text_yes;
else
choices[idx].text = bat_text_no;
for (i = 1; i < TARGET_COUNT; ++i)
{
if (targets[0].batpath[0] != NUL
&& choices[idx + i].changefunc != NULL)
{
choices[idx + i].active = 1;
if (choices[idx + i].changefunc == change_bat_choice
&& targets[i].batpath[0] != NUL)
{
strcpy(targets[i].batpath, targets[0].batpath);
set_bat_text(idx + i, targets[i].batpath, targets[i].batname);
}
}
else
choices[idx + i].active = 0;
}
}
static void
init_bat_choice(int target)
{
char *batpath = targets[target].batpath;
char *oldbat = targets[target].oldbat;
char *p;
int i;
choices[choice_count].arg = target;
choices[choice_count].installfunc = install_bat_choice;
choices[choice_count].active = 1;
choices[choice_count].text = NULL; 
if (oldbat != NULL)
{
choices[choice_count].changefunc = toggle_bat_choice;
*batpath = NUL;
toggle_bat_choice(choice_count);
}
else
{
if (default_bat_dir != NULL)
strcpy(batpath, default_bat_dir);
else
{
p = getenv("WINDIR");
if (p != NULL && *p != NUL)
strcpy(batpath, p);
else
{
p = getenv("PATH");
if (p == NULL || *p == NUL) 
strcpy(batpath, "C:/Windows");
else
{
i = 0;
while (*p != NUL && *p != ';')
batpath[i++] = *p++;
batpath[i] = NUL;
}
}
}
add_pathsep(batpath);
set_bat_text(choice_count, batpath, targets[target].batname);
choices[choice_count].changefunc = change_bat_choice;
}
++choice_count;
}
static void
init_bat_choices(void)
{
int i;
choices[choice_count].text = bat_text_yes;
choices[choice_count].changefunc = change_main_bat_choice;
choices[choice_count].installfunc = NULL;
choices[choice_count].active = 1;
choices[choice_count].arg = 0;
++choice_count;
for (i = 1; i < TARGET_COUNT; ++i)
if (targets[i].oldexe == NULL
&& (targets[i].exenamearg[0] == 'g' ? has_gvim : has_vim))
init_bat_choice(i);
else
add_dummy_choice();
}
static void
install_vimrc(int idx)
{
FILE *fd, *tfd;
char *fname;
if (*oldvimrc != NUL)
fname = oldvimrc;
else
fname = vimrc;
fd = fopen(fname, "w");
if (fd == NULL)
{
printf("\nERROR: Cannot open \"%s\" for writing.\n", fname);
return;
}
switch (compat_choice)
{
case compat_vi:
fprintf(fd, "\" Vi compatible\n");
fprintf(fd, "set compatible\n");
break;
case compat_vim:
fprintf(fd, "\" Vim's default behavior\n");
fprintf(fd, "if &compatible\n");
fprintf(fd, " set nocompatible\n");
fprintf(fd, "endif\n");
break;
case compat_some_enhancements:
fprintf(fd, "\" Vim with some enhancements\n");
fprintf(fd, "source $VIMRUNTIME/defaults.vim\n");
break;
case compat_all_enhancements:
fprintf(fd, "\" Vim with all enhancements\n");
fprintf(fd, "source $VIMRUNTIME/vimrc_example.vim\n");
break;
}
switch (remap_choice)
{
case remap_no:
break;
case remap_win:
fprintf(fd, "\n");
fprintf(fd, "\" Remap a few keys for Windows behavior\n");
fprintf(fd, "source $VIMRUNTIME/mswin.vim\n");
break;
}
switch (mouse_choice)
{
case mouse_xterm:
fprintf(fd, "\n");
fprintf(fd, "\" Mouse behavior (the Unix way)\n");
fprintf(fd, "behave xterm\n");
break;
case mouse_mswin:
fprintf(fd, "\n");
fprintf(fd, "\" Mouse behavior (the Windows way)\n");
fprintf(fd, "behave mswin\n");
break;
case mouse_default:
break;
}
if ((tfd = fopen("diff.exe", "r")) != NULL)
{
fclose(tfd);
fprintf(fd, "\n");
fprintf(fd, "\" Use the internal diff if available.\n");
fprintf(fd, "\" Otherwise use the special 'diffexpr' for Windows.\n");
fprintf(fd, "if &diffopt !~#'internal'\n");
fprintf(fd, " set diffexpr=MyDiff()\n");
fprintf(fd, "endif\n");
fprintf(fd, "function MyDiff()\n");
fprintf(fd, " let opt = '-a --binary '\n");
fprintf(fd, " if &diffopt =~ 'icase' | let opt = opt . '-i ' | endif\n");
fprintf(fd, " if &diffopt =~ 'iwhite' | let opt = opt . '-b ' | endif\n");
fprintf(fd, " let arg1 = v:fname_in\n");
fprintf(fd, " if arg1 =~ ' ' | let arg1 = '\"' . arg1 . '\"' | endif\n");
fprintf(fd, " let arg1 = substitute(arg1, '!', '\\!', 'g')\n");
fprintf(fd, " let arg2 = v:fname_new\n");
fprintf(fd, " if arg2 =~ ' ' | let arg2 = '\"' . arg2 . '\"' | endif\n");
fprintf(fd, " let arg2 = substitute(arg2, '!', '\\!', 'g')\n");
fprintf(fd, " let arg3 = v:fname_out\n");
fprintf(fd, " if arg3 =~ ' ' | let arg3 = '\"' . arg3 . '\"' | endif\n");
fprintf(fd, " let arg3 = substitute(arg3, '!', '\\!', 'g')\n");
fprintf(fd, " if $VIMRUNTIME =~ ' '\n");
fprintf(fd, " if &sh =~ '\\<cmd'\n");
fprintf(fd, " if empty(&shellxquote)\n");
fprintf(fd, " let l:shxq_sav = ''\n");
fprintf(fd, " set shellxquote&\n");
fprintf(fd, " endif\n");
fprintf(fd, " let cmd = '\"' . $VIMRUNTIME . '\\diff\"'\n");
fprintf(fd, " else\n");
fprintf(fd, " let cmd = substitute($VIMRUNTIME, ' ', '\" ', '') . '\\diff\"'\n");
fprintf(fd, " endif\n");
fprintf(fd, " else\n");
fprintf(fd, " let cmd = $VIMRUNTIME . '\\diff'\n");
fprintf(fd, " endif\n");
fprintf(fd, " let cmd = substitute(cmd, '!', '\\!', 'g')\n");
fprintf(fd, " silent execute '!' . cmd . ' ' . opt . arg1 . ' ' . arg2 . ' > ' . arg3\n");
fprintf(fd, " if exists('l:shxq_sav')\n");
fprintf(fd, " let &shellxquote=l:shxq_sav\n");
fprintf(fd, " endif\n");
fprintf(fd, "endfunction\n");
fprintf(fd, "\n");
}
fclose(fd);
printf("%s has been written\n", fname);
}
static void
change_vimrc_choice(int idx)
{
if (choices[idx].installfunc != NULL)
{
if (*oldvimrc != NUL)
alloc_text(idx, "Do NOT change startup file %s", oldvimrc);
else
alloc_text(idx, "Do NOT create startup file %s", vimrc);
choices[idx].installfunc = NULL;
choices[idx + 1].active = 0;
choices[idx + 2].active = 0;
choices[idx + 3].active = 0;
}
else
{
if (*oldvimrc != NUL)
alloc_text(idx, "Overwrite startup file %s with:", oldvimrc);
else
alloc_text(idx, "Create startup file %s with:", vimrc);
choices[idx].installfunc = install_vimrc;
choices[idx + 1].active = 1;
choices[idx + 2].active = 1;
choices[idx + 3].active = 1;
}
}
static void
change_run_choice(int idx)
{
compat_choice = get_choice(compat_choices, TABLE_SIZE(compat_choices));
alloc_text(idx, compat_text, compat_choices[compat_choice]);
}
static void
change_remap_choice(int idx)
{
remap_choice = get_choice(remap_choices, TABLE_SIZE(remap_choices));
alloc_text(idx, remap_text, remap_choices[remap_choice]);
}
static void
change_mouse_choice(int idx)
{
mouse_choice = get_choice(mouse_choices, TABLE_SIZE(mouse_choices));
alloc_text(idx, mouse_text, mouse_choices[mouse_choice]);
}
static void
init_vimrc_choices(void)
{
strcpy(vimrc, installdir);
strcpy(vimrc + runtimeidx, "_vimrc");
if (*oldvimrc == NUL)
choices[choice_count].installfunc = NULL;
else
choices[choice_count].installfunc = install_vimrc;
choices[choice_count].text = NULL;
change_vimrc_choice(choice_count);
choices[choice_count].changefunc = change_vimrc_choice;
choices[choice_count].active = 1;
++choice_count;
alloc_text(choice_count, compat_text, compat_choices[compat_choice]);
choices[choice_count].changefunc = change_run_choice;
choices[choice_count].installfunc = NULL;
choices[choice_count].active = (*oldvimrc == NUL);
++choice_count;
alloc_text(choice_count, remap_text , remap_choices[remap_choice]);
choices[choice_count].changefunc = change_remap_choice;
choices[choice_count].installfunc = NULL;
choices[choice_count].active = (*oldvimrc == NUL);
++choice_count;
alloc_text(choice_count, mouse_text, mouse_choices[mouse_choice]);
choices[choice_count].changefunc = change_mouse_choice;
choices[choice_count].installfunc = NULL;
choices[choice_count].active = (*oldvimrc == NUL);
++choice_count;
}
static LONG
reg_create_key(
HKEY root,
const char *subkey,
PHKEY phKey,
DWORD flag)
{
DWORD disp;
*phKey = NULL;
return RegCreateKeyEx(
root, subkey,
0, NULL, REG_OPTION_NON_VOLATILE,
flag | KEY_WRITE,
NULL, phKey, &disp);
}
static LONG
reg_set_string_value(
HKEY hKey,
const char *value_name,
const char *data)
{
return RegSetValueEx(hKey, value_name, 0, REG_SZ,
(LPBYTE)data, (DWORD)(1 + strlen(data)));
}
static LONG
reg_create_key_and_value(
HKEY hRootKey,
const char *subkey,
const char *value_name,
const char *data,
DWORD flag)
{
HKEY hKey;
LONG lRet = reg_create_key(hRootKey, subkey, &hKey, flag);
if (ERROR_SUCCESS == lRet)
{
lRet = reg_set_string_value(hKey, value_name, data);
RegCloseKey(hKey);
}
return lRet;
}
static LONG
register_inproc_server(
HKEY hRootKey,
const char *clsid,
const char *extname,
const char *module,
const char *threading_model,
DWORD flag)
{
CHAR subkey[BUFSIZE];
LONG lRet;
sprintf(subkey, "CLSID\\%s", clsid);
lRet = reg_create_key_and_value(hRootKey, subkey, NULL, extname, flag);
if (ERROR_SUCCESS == lRet)
{
sprintf(subkey, "CLSID\\%s\\InProcServer32", clsid);
lRet = reg_create_key_and_value(hRootKey, subkey, NULL, module, flag);
if (ERROR_SUCCESS == lRet)
{
lRet = reg_create_key_and_value(hRootKey, subkey,
"ThreadingModel", threading_model, flag);
}
}
return lRet;
}
static LONG
register_shellex(
HKEY hRootKey,
const char *clsid,
const char *name,
const char *exe_path,
DWORD flag)
{
LONG lRet = reg_create_key_and_value(
hRootKey,
"*\\shellex\\ContextMenuHandlers\\gvim",
NULL,
clsid,
flag);
if (ERROR_SUCCESS == lRet)
{
lRet = reg_create_key_and_value(
HKEY_LOCAL_MACHINE,
"Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved",
clsid,
name,
flag);
if (ERROR_SUCCESS == lRet)
{
lRet = reg_create_key_and_value(
HKEY_LOCAL_MACHINE,
"Software\\Vim\\Gvim",
"path",
exe_path,
flag);
}
}
return lRet;
}
static LONG
register_openwith(
HKEY hRootKey,
const char *exe_path,
DWORD flag)
{
char exe_cmd[BUFSIZE];
LONG lRet;
sprintf(exe_cmd, "\"%s\" \"%%1\"", exe_path);
lRet = reg_create_key_and_value(
hRootKey,
"Applications\\gvim.exe\\shell\\edit\\command",
NULL,
exe_cmd,
flag);
if (ERROR_SUCCESS == lRet)
{
int i;
static const char *openwith[] = {
".htm\\OpenWithList\\gvim.exe",
".vim\\OpenWithList\\gvim.exe",
"*\\OpenWithList\\gvim.exe",
};
for (i = 0; ERROR_SUCCESS == lRet
&& i < sizeof(openwith) / sizeof(openwith[0]); i++)
lRet = reg_create_key_and_value(hRootKey, openwith[i], NULL, "", flag);
}
return lRet;
}
static LONG
register_uninstall(
HKEY hRootKey,
const char *appname,
const char *display_name,
const char *uninstall_string,
const char *display_icon,
const char *display_version,
const char *publisher)
{
LONG lRet = reg_create_key_and_value(hRootKey, appname,
"DisplayName", display_name, KEY_WOW64_64KEY);
if (ERROR_SUCCESS == lRet)
lRet = reg_create_key_and_value(hRootKey, appname,
"UninstallString", uninstall_string, KEY_WOW64_64KEY);
if (ERROR_SUCCESS == lRet)
lRet = reg_create_key_and_value(hRootKey, appname,
"DisplayIcon", display_icon, KEY_WOW64_64KEY);
if (ERROR_SUCCESS == lRet)
lRet = reg_create_key_and_value(hRootKey, appname,
"DisplayVersion", display_version, KEY_WOW64_64KEY);
if (ERROR_SUCCESS == lRet)
lRet = reg_create_key_and_value(hRootKey, appname,
"Publisher", publisher, KEY_WOW64_64KEY);
return lRet;
}
static int
install_registry(void)
{
LONG lRet = ERROR_SUCCESS;
const char *vim_ext_ThreadingModel = "Apartment";
const char *vim_ext_name = "Vim Shell Extension";
const char *vim_ext_clsid = "{51EEE242-AD87-11d3-9C1E-0090278BBD99}";
char vim_exe_path[MAX_PATH];
char display_name[BUFSIZE];
char uninstall_string[BUFSIZE];
char icon_string[BUFSIZE];
int i;
int loop_count = is_64bit_os() ? 2 : 1;
DWORD flag;
sprintf(vim_exe_path, "%s\\gvim.exe", installdir);
if (install_popup)
{
char bufg[BUFSIZE];
printf("Creating \"Edit with Vim\" popup menu entry\n");
for (i = 0; i < loop_count; i++)
{
if (i == 0)
{
sprintf(bufg, "%s\\" GVIMEXT32_PATH, installdir);
flag = KEY_WOW64_32KEY;
}
else
{
sprintf(bufg, "%s\\" GVIMEXT64_PATH, installdir);
flag = KEY_WOW64_64KEY;
}
lRet = register_inproc_server(
HKEY_CLASSES_ROOT, vim_ext_clsid, vim_ext_name,
bufg, vim_ext_ThreadingModel, flag);
if (ERROR_SUCCESS != lRet)
return FAIL;
lRet = register_shellex(
HKEY_CLASSES_ROOT, vim_ext_clsid, vim_ext_name,
vim_exe_path, flag);
if (ERROR_SUCCESS != lRet)
return FAIL;
}
}
if (install_openwith)
{
printf("Creating \"Open with ...\" list entry\n");
for (i = 0; i < loop_count; i++)
{
if (i == 0)
flag = KEY_WOW64_32KEY;
else
flag = KEY_WOW64_64KEY;
lRet = register_openwith(HKEY_CLASSES_ROOT, vim_exe_path, flag);
if (ERROR_SUCCESS != lRet)
return FAIL;
}
}
printf("Creating an uninstall entry\n");
sprintf(display_name, "Vim " VIM_VERSION_SHORT
#if defined(_M_ARM64)
" (arm64)"
#elif _M_X64
" (x64)"
#endif
);
if (interactive)
sprintf(uninstall_string, "%s\\uninstall.exe", installdir);
else
sprintf(uninstall_string, "%s\\uninstall-gui.exe", installdir);
sprintf(icon_string, "%s\\gvim.exe,0", installdir);
lRet = register_uninstall(
HKEY_LOCAL_MACHINE,
"Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Vim " VIM_VERSION_SHORT,
display_name,
uninstall_string,
icon_string,
VIM_VERSION_SHORT,
"Bram Moolenaar et al.");
if (ERROR_SUCCESS != lRet)
return FAIL;
return OK;
}
static void
change_popup_choice(int idx)
{
if (install_popup == 0)
{
choices[idx].text = "Install an entry for Vim in the popup menu for the right\n mouse button so that you can edit any file with Vim";
install_popup = 1;
}
else
{
choices[idx].text = "Do NOT install an entry for Vim in the popup menu for the\n right mouse button to edit any file with Vim";
install_popup = 0;
}
}
static void
init_popup_choice(void)
{
struct stat st;
if (has_gvim
&& (stat(GVIMEXT32_PATH, &st) >= 0
|| stat(GVIMEXT64_PATH, &st) >= 0))
{
choices[choice_count].changefunc = change_popup_choice;
choices[choice_count].installfunc = NULL;
choices[choice_count].active = 1;
change_popup_choice(choice_count); 
++choice_count;
}
else
add_dummy_choice();
}
static void
change_openwith_choice(int idx)
{
if (install_openwith == 0)
{
choices[idx].text = "Add Vim to the \"Open With...\" list in the popup menu for the right\n mouse button so that you can edit any file with Vim";
install_openwith = 1;
}
else
{
choices[idx].text = "Do NOT add Vim to the \"Open With...\" list in the popup menu for the\n right mouse button to edit any file with Vim";
install_openwith = 0;
}
}
static void
init_openwith_choice(void)
{
if (has_gvim)
{
choices[choice_count].changefunc = change_openwith_choice;
choices[choice_count].installfunc = NULL;
choices[choice_count].active = 1;
change_openwith_choice(choice_count); 
++choice_count;
}
else
add_dummy_choice();
}
int
create_shortcut(
const char *shortcut_name,
const char *iconfile_path,
int iconindex,
const char *shortcut_target,
const char *shortcut_args,
const char *workingdir
)
{
IShellLink *shelllink_ptr;
HRESULT hres;
IPersistFile *persistfile_ptr;
hres = CoInitialize(NULL);
if (!SUCCEEDED(hres))
{
printf("Error: Could not open the COM library. Not creating shortcut.\n");
return FAIL;
}
hres = CoCreateInstance(&CLSID_ShellLink,
NULL,
CLSCTX_INPROC_SERVER,
&IID_IShellLink,
(void **) &shelllink_ptr);
if (SUCCEEDED(hres)) 
{
hres = shelllink_ptr->lpVtbl->QueryInterface(shelllink_ptr,
&IID_IPersistFile, (void **) &persistfile_ptr);
if (SUCCEEDED(hres))
{
wchar_t wsz[BUFSIZE];
MultiByteToWideChar(CP_ACP, 0, shortcut_name, -1, wsz, sizeof(wsz)/sizeof(wsz[0]));
shelllink_ptr->lpVtbl->SetPath(shelllink_ptr, shortcut_target);
shelllink_ptr->lpVtbl->SetWorkingDirectory(shelllink_ptr,
workingdir);
shelllink_ptr->lpVtbl->SetIconLocation(shelllink_ptr,
iconfile_path, iconindex);
shelllink_ptr->lpVtbl->SetArguments(shelllink_ptr, shortcut_args);
persistfile_ptr->lpVtbl->Save(persistfile_ptr, wsz, 1);
persistfile_ptr->lpVtbl->Release(persistfile_ptr);
}
else
{
printf("QueryInterface Error\n");
return FAIL;
}
shelllink_ptr->lpVtbl->Release(shelllink_ptr);
}
else
{
printf("CoCreateInstance Error - hres = %08x\n", (int)hres);
return FAIL;
}
return OK;
}
int
build_link_name(
char *link_path,
const char *link_name,
const char *shell_folder_name)
{
char shell_folder_path[MAX_PATH];
if (get_shell_folder_path(shell_folder_path, shell_folder_name) == FAIL)
{
printf("An error occurred while attempting to find the path to %s.\n",
shell_folder_name);
return FAIL;
}
vim_mkdir(shell_folder_path, 0755);
sprintf(link_path, "%s\\%s.lnk", shell_folder_path, link_name);
return OK;
}
static int
build_shortcut(
const char *name, 
const char *exename, 
const char *args,
const char *shell_folder,
const char *workingdir)
{
char executable_path[BUFSIZE];
char link_name[BUFSIZE];
sprintf(executable_path, "%s\\%s", installdir, exename);
if (build_link_name(link_name, name, shell_folder) == FAIL)
{
printf("An error has occurred. A shortcut to %s will not be created %s.\n",
name,
*shell_folder == 'd' ? "on the desktop" : "in the Start menu");
return FAIL;
}
return create_shortcut(link_name, executable_path, 0,
executable_path, args, workingdir);
}
#define WORKDIR "%HOMEDRIVE%%HOMEPATH%"
static void
install_start_menu(int idx)
{
need_uninstall_entry = 1;
printf("Creating start menu\n");
if (has_vim)
{
if (build_shortcut("Vim", "vim.exe", "",
VIM_STARTMENU, WORKDIR) == FAIL)
return;
if (build_shortcut("Vim Read-only", "vim.exe", "-R",
VIM_STARTMENU, WORKDIR) == FAIL)
return;
if (build_shortcut("Vim Diff", "vim.exe", "-d",
VIM_STARTMENU, WORKDIR) == FAIL)
return;
}
if (has_gvim)
{
if (build_shortcut("gVim", "gvim.exe", "",
VIM_STARTMENU, WORKDIR) == FAIL)
return;
if (build_shortcut("gVim Easy", "gvim.exe", "-y",
VIM_STARTMENU, WORKDIR) == FAIL)
return;
if (build_shortcut("gVim Read-only", "gvim.exe", "-R",
VIM_STARTMENU, WORKDIR) == FAIL)
return;
if (build_shortcut("gVim Diff", "gvim.exe", "-d",
VIM_STARTMENU, WORKDIR) == FAIL)
return;
}
if (build_shortcut("Uninstall",
interactive ? "uninstall.exe" : "uninstall-gui.exe", "",
VIM_STARTMENU, installdir) == FAIL)
return;
if (build_shortcut("Vim tutor", "vimtutor.bat", "",
VIM_STARTMENU, installdir) == FAIL)
return;
if (build_shortcut("Help", has_gvim ? "gvim.exe" : "vim.exe", "-c h",
VIM_STARTMENU, WORKDIR) == FAIL)
return;
{
char shell_folder_path[BUFSIZE];
if (get_shell_folder_path(shell_folder_path, VIM_STARTMENU) == FAIL)
{
printf("Finding the path of the Start menu failed\n");
return ;
}
add_pathsep(shell_folder_path);
strcat(shell_folder_path, "Vim Online.url");
if (!WritePrivateProfileString("InternetShortcut", "URL",
"https://www.vim.org/", shell_folder_path))
{
printf("Creating the Vim online URL failed\n");
return;
}
}
}
static void
toggle_startmenu_choice(int idx)
{
if (choices[idx].installfunc == NULL)
{
choices[idx].installfunc = install_start_menu;
choices[idx].text = "Add Vim to the Start menu";
}
else
{
choices[idx].installfunc = NULL;
choices[idx].text = "Do NOT add Vim to the Start menu";
}
}
void
install_shortcut_gvim(int idx)
{
if (choices[idx].arg)
{
(void)build_shortcut(icon_names[0], "gvim.exe",
"", "desktop", WORKDIR);
need_uninstall_entry = 1;
}
}
void
install_shortcut_evim(int idx)
{
if (choices[idx].arg)
{
(void)build_shortcut(icon_names[1], "gvim.exe",
"-y", "desktop", WORKDIR);
need_uninstall_entry = 1;
}
}
void
install_shortcut_gview(int idx)
{
if (choices[idx].arg)
{
(void)build_shortcut(icon_names[2], "gvim.exe",
"-R", "desktop", WORKDIR);
need_uninstall_entry = 1;
}
}
void
toggle_shortcut_choice(int idx)
{
char *arg;
if (choices[idx].installfunc == install_shortcut_gvim)
arg = "gVim";
else if (choices[idx].installfunc == install_shortcut_evim)
arg = "gVim Easy";
else
arg = "gVim Read-only";
if (choices[idx].arg)
{
choices[idx].arg = 0;
alloc_text(idx, "Do NOT create a desktop icon for %s", arg);
}
else
{
choices[idx].arg = 1;
alloc_text(idx, "Create a desktop icon for %s", arg);
}
}
static void
init_startmenu_choice(void)
{
choices[choice_count].changefunc = toggle_startmenu_choice;
choices[choice_count].installfunc = NULL;
choices[choice_count].active = 1;
toggle_startmenu_choice(choice_count); 
++choice_count;
}
static void
init_shortcut_choices(void)
{
choices[choice_count].text = NULL;
choices[choice_count].arg = 0;
choices[choice_count].active = has_gvim;
choices[choice_count].changefunc = toggle_shortcut_choice;
choices[choice_count].installfunc = install_shortcut_gvim;
toggle_shortcut_choice(choice_count);
++choice_count;
choices[choice_count].text = NULL;
choices[choice_count].arg = 0;
choices[choice_count].active = has_gvim;
choices[choice_count].changefunc = toggle_shortcut_choice;
choices[choice_count].installfunc = install_shortcut_evim;
toggle_shortcut_choice(choice_count);
++choice_count;
choices[choice_count].text = NULL;
choices[choice_count].arg = 0;
choices[choice_count].active = has_gvim;
choices[choice_count].changefunc = toggle_shortcut_choice;
choices[choice_count].installfunc = install_shortcut_gview;
toggle_shortcut_choice(choice_count);
++choice_count;
}
static void
install_OLE_register(void)
{
char register_command_string[BUFSIZE + 30];
printf("\n--- Attempting to register Vim with OLE ---\n");
printf("(There is no message whether this works or not.)\n");
sprintf(register_command_string, "\"%s\\gvim.exe\" -silent -register", installdir);
system(register_command_string);
}
static void
dir_remove_last(const char *path, char to[MAX_PATH])
{
char c;
long last_char_to_copy;
long path_length = strlen(path);
last_char_to_copy = path_length - 2;
c = path[last_char_to_copy];
while (c != '\\')
{
last_char_to_copy--;
c = path[last_char_to_copy];
}
strncpy(to, path, (size_t)last_char_to_copy);
to[last_char_to_copy] = NUL;
}
static void
set_directories_text(int idx)
{
int vimfiles_dir_choice = choices[idx].arg;
if (vimfiles_dir_choice == (int)vimfiles_dir_none)
alloc_text(idx, "Do NOT create plugin directories%s", "");
else
alloc_text(idx, "Create plugin directories: %s",
vimfiles_dir_choices[vimfiles_dir_choice]);
}
static char *homedir = NULL;
void
init_homedir(void)
{
char *var;
char buf[MAX_PATH];
if (homedir != NULL)
{
free(homedir);
homedir = NULL;
}
var = getenv("HOME");
if (var == NULL || *var == NUL)
{
char *homedrive, *homepath;
homedrive = getenv("HOMEDRIVE");
homepath = getenv("HOMEPATH");
if (homepath == NULL || *homepath == NUL)
homepath = "\\";
if (homedrive != NULL
&& strlen(homedrive) + strlen(homepath) < sizeof(buf))
{
sprintf(buf, "%s%s", homedrive, homepath);
if (buf[0] != NUL)
var = buf;
}
}
if (var == NULL)
var = getenv("USERPROFILE");
if (var != NULL && *var == '%')
{
char *p;
char *exp;
p = strchr(var + 1, '%');
if (p != NULL)
{
strncpy(buf, var + 1, p - (var + 1));
buf[p - (var + 1)] = NUL;
exp = getenv(buf);
if (exp != NULL && *exp != NUL
&& strlen(exp) + strlen(p) < sizeof(buf))
{
sprintf(buf, "%s%s", exp, p + 1);
var = buf;
}
}
}
if (var != NULL && *var == NUL) 
var = NULL;
if (var == NULL)
homedir = NULL;
else
homedir = _strdup(var);
}
static void
change_directories_choice(int idx)
{
int choice_count = TABLE_SIZE(vimfiles_dir_choices);
if (homedir == NULL)
--choice_count;
choices[idx].arg = get_choice(vimfiles_dir_choices, choice_count);
set_directories_text(idx);
}
static void
install_vimfilesdir(int idx)
{
int i;
int vimfiles_dir_choice = choices[idx].arg;
char *p;
char vimdir_path[MAX_PATH];
char vimfiles_path[MAX_PATH + 9];
char tmp_dirname[BUFSIZE];
switch (vimfiles_dir_choice)
{
case vimfiles_dir_vim:
{
p = getenv("VIM");
if (p == NULL) 
dir_remove_last(installdir, vimdir_path);
else
strcpy(vimdir_path, p);
break;
}
case vimfiles_dir_home:
{
p = homedir;
if (p == NULL)
{
printf("Internal error: $HOME is NULL\n");
p = "c:\\";
}
strcpy(vimdir_path, p);
break;
}
case vimfiles_dir_none:
{
return;
}
}
sprintf(vimfiles_path, "%s\\vimfiles", vimdir_path);
vim_mkdir(vimfiles_path, 0755);
printf("Creating the following directories in \"%s\":\n", vimfiles_path);
for (i = 0; i < TABLE_SIZE(vimfiles_subdirs); i++)
{
sprintf(tmp_dirname, "%s\\%s", vimfiles_path, vimfiles_subdirs[i]);
printf(" %s", vimfiles_subdirs[i]);
vim_mkdir(tmp_dirname, 0755);
}
printf("\n");
}
static void
init_directories_choice(void)
{
struct stat st;
char tmp_dirname[BUFSIZE];
char *p;
int vimfiles_dir_choice;
choices[choice_count].text = alloc(150);
choices[choice_count].changefunc = change_directories_choice;
choices[choice_count].installfunc = install_vimfilesdir;
choices[choice_count].active = 1;
p = getenv("HOME");
if (p != NULL)
{
vimfiles_dir_choice = (int)vimfiles_dir_home;
sprintf(tmp_dirname, "%s\\vimfiles\\compiler", p);
if (stat(tmp_dirname, &st) == 0)
vimfiles_dir_choice = (int)vimfiles_dir_none;
}
else
{
vimfiles_dir_choice = (int)vimfiles_dir_vim;
p = getenv("VIM");
if (p == NULL) 
dir_remove_last(installdir, tmp_dirname);
else
strcpy(tmp_dirname, p);
strcat(tmp_dirname, "\\vimfiles\\compiler");
if (stat(tmp_dirname, &st) == 0)
vimfiles_dir_choice = (int)vimfiles_dir_none;
}
choices[choice_count].arg = vimfiles_dir_choice;
set_directories_text(choice_count);
++choice_count;
}
static void
setup_choices(void)
{
init_bat_choices();
init_vimrc_choices();
init_popup_choice();
init_openwith_choice();
init_startmenu_choice();
init_shortcut_choices();
init_directories_choice();
}
static void
print_cmd_line_help(void)
{
printf("Vim installer non-interactive command line arguments:\n");
printf("\n");
printf("-create-batfiles [vim gvim evim view gview vimdiff gvimdiff]\n");
printf(" Create .bat files for Vim variants in the Windows directory.\n");
printf("-create-vimrc\n");
printf(" Create a default _vimrc file if one does not already exist.\n");
printf("-vimrc-remap [no|win]\n");
printf(" Remap keys when creating a default _vimrc file.\n");
printf("-vimrc-behave [unix|mswin|default]\n");
printf(" Set mouse behavior when creating a default _vimrc file.\n");
printf("-vimrc-compat [vi|vim|defaults|all]\n");
printf(" Set Vi compatibility when creating a default _vimrc file.\n");
printf("-install-popup\n");
printf(" Install the Edit-with-Vim context menu entry\n");
printf("-install-openwith\n");
printf(" Add Vim to the \"Open With...\" context menu list\n");
printf("-add-start-menu");
printf(" Add Vim to the start menu\n");
printf("-install-icons");
printf(" Create icons for gVim executables on the desktop\n");
printf("-create-directories [vim|home]\n");
printf(" Create runtime directories to drop plugins into; in the $VIM\n");
printf(" or $HOME directory\n");
printf("-register-OLE");
printf(" Ignored\n");
printf("\n");
}
static void
command_line_setup_choices(int argc, char **argv)
{
int i, j;
for (i = 1; i < argc; i++)
{
if (strcmp(argv[i], "-create-batfiles") == 0)
{
if (i + 1 == argc)
continue;
while (argv[i + 1][0] != '-' && i < argc)
{
i++;
for (j = 1; j < TARGET_COUNT; ++j)
if ((targets[j].exenamearg[0] == 'g' ? has_gvim : has_vim)
&& strcmp(argv[i], targets[j].name) == 0)
{
init_bat_choice(j);
break;
}
if (j == TARGET_COUNT)
printf("%s is not a valid choice for -create-batfiles\n",
argv[i]);
if (i + 1 == argc)
break;
}
}
else if (strcmp(argv[i], "-create-vimrc") == 0)
{
init_vimrc_choices();
}
else if (strcmp(argv[i], "-vimrc-remap") == 0)
{
if (i + 1 == argc)
break;
i++;
if (strcmp(argv[i], "no") == 0)
remap_choice = remap_no;
else if (strcmp(argv[i], "win") == 0)
remap_choice = remap_win;
}
else if (strcmp(argv[i], "-vimrc-behave") == 0)
{
if (i + 1 == argc)
break;
i++;
if (strcmp(argv[i], "unix") == 0)
mouse_choice = mouse_xterm;
else if (strcmp(argv[i], "mswin") == 0)
mouse_choice = mouse_mswin;
else if (strcmp(argv[i], "default") == 0)
mouse_choice = mouse_default;
}
else if (strcmp(argv[i], "-vimrc-compat") == 0)
{
if (i + 1 == argc)
break;
i++;
if (strcmp(argv[i], "vi") == 0)
compat_choice = compat_vi;
else if (strcmp(argv[i], "vim") == 0)
compat_choice = compat_vim;
else if (strcmp(argv[i], "defaults") == 0)
compat_choice = compat_some_enhancements;
else if (strcmp(argv[i], "all") == 0)
compat_choice = compat_all_enhancements;
}
else if (strcmp(argv[i], "-install-popup") == 0)
{
init_popup_choice();
}
else if (strcmp(argv[i], "-install-openwith") == 0)
{
init_openwith_choice();
}
else if (strcmp(argv[i], "-add-start-menu") == 0)
{
init_startmenu_choice();
}
else if (strcmp(argv[i], "-install-icons") == 0)
{
init_shortcut_choices();
}
else if (strcmp(argv[i], "-create-directories") == 0)
{
int vimfiles_dir_choice = (int)vimfiles_dir_none;
init_directories_choice();
if (argv[i + 1][0] != '-')
{
i++;
if (strcmp(argv[i], "vim") == 0)
vimfiles_dir_choice = (int)vimfiles_dir_vim;
else if (strcmp(argv[i], "home") == 0)
{
if (homedir == NULL) 
vimfiles_dir_choice = (int)vimfiles_dir_none;
else
vimfiles_dir_choice = (int)vimfiles_dir_home;
}
else
{
printf("Unknown argument for -create-directories: %s\n",
argv[i]);
print_cmd_line_help();
}
}
else 
vimfiles_dir_choice = (int)vimfiles_dir_vim;
choices[choice_count - 1].arg = vimfiles_dir_choice;
}
else if (strcmp(argv[i], "-register-OLE") == 0)
{
}
else 
{
printf("Got unknown argument argv[%d] = %s\n", i, argv[i]);
print_cmd_line_help();
}
}
}
static void
show_help(void)
{
static char *(items[]) =
{
"Installing .bat files\n"
"---------------------\n"
"The vim.bat file is written in one of the directories in $PATH.\n"
"This makes it possible to start Vim from the command line.\n"
"If vim.exe can be found in $PATH, the choice for vim.bat will not be\n"
"present. It is assumed you will use the existing vim.exe.\n"
"If vim.bat can already be found in $PATH this is probably for an old\n"
"version of Vim (but this is not checked!). You can overwrite it.\n"
"If no vim.bat already exists, you can select one of the directories in\n"
"$PATH for creating the batch file, or disable creating a vim.bat file.\n"
"\n"
"If you choose not to create the vim.bat file, Vim can still be executed\n"
"in other ways, but not from the command line.\n"
"\n"
"The same applies to choices for gvim, evim, (g)view, and (g)vimdiff.\n"
"The first item can be used to change the path for all of them.\n"
,
"Creating a _vimrc file\n"
"----------------------\n"
"The _vimrc file is used to set options for how Vim behaves.\n"
"The install program can create a _vimrc file with a few basic choices.\n"
"You can edit this file later to tune your preferences.\n"
"If you already have a _vimrc or .vimrc file it can be overwritten.\n"
"Don't do that if you have made changes to it.\n"
,
"Vim features\n"
"------------\n"
"(this choice is only available when creating a _vimrc file)\n"
"1. Vim can run in Vi-compatible mode. Many nice Vim features are then\n"
" disabled. Only choose Vi-compatible if you really need full Vi\n"
" compatibility.\n"
"2. Vim runs in not-Vi-compatible mode. Vim is still mostly Vi compatible,\n"
" but adds nice features like multi-level undo.\n"
"3. Running Vim with some enhancements is useful when you want some of\n"
" the nice Vim features, but have a slow computer and want to keep it\n"
" really fast.\n"
"4. Syntax highlighting shows many files in color. Not only does this look\n"
" nice, it also makes it easier to spot errors and you can work faster.\n"
" The other features include editing compressed files.\n"
,
"Windows key mapping\n"
"-------------------\n"
"(this choice is only available when creating a _vimrc file)\n"
"Under MS-Windows the CTRL-C key copies text to the clipboard and CTRL-V\n"
"pastes text from the clipboard. There are a few more keys like these.\n"
"Unfortunately, in Vim these keys normally have another meaning.\n"
"1. Choose to have the keys like they normally are in Vim (useful if you\n"
" also use Vim on other systems).\n"
"2. Choose to have the keys work like they are used on MS-Windows (useful\n"
" if you mostly work on MS-Windows).\n"
,
"Mouse use\n"
"---------\n"
"(this choice is only available when creating a _vimrc file)\n"
"The right mouse button can be used in two ways:\n"
"1. The Unix way is to extend an existing selection. The popup menu is\n"
" not available.\n"
"2. The MS-Windows way is to show a popup menu, which allows you to\n"
" copy/paste text, undo/redo, etc. Extending the selection can still be\n"
" done by keeping SHIFT pressed while using the left mouse button\n"
,
"Edit-with-Vim context menu entry\n"
"--------------------------------\n"
"(this choice is only available when gvim.exe and gvimext.dll are present)\n"
"You can associate different file types with Vim, so that you can (double)\n"
"click on a file to edit it with Vim. This means you have to individually\n"
"select each file type.\n"
"An alternative is the option offered here: Install an \"Edit with Vim\"\n"
"entry in the popup menu for the right mouse button. This means you can\n"
"edit any file with Vim.\n"
,
"\"Open With...\" context menu entry\n"
"--------------------------------\n"
"(this choice is only available when gvim.exe is present)\n"
"This option adds Vim to the \"Open With...\" entry in the popup menu for\n"
"the right mouse button. This also makes it possible to edit HTML files\n"
"directly from Internet Explorer.\n"
,
"Add Vim to the Start menu\n"
"-------------------------\n"
"In Windows 95 and later, Vim can be added to the Start menu. This will\n"
"create a submenu with an entry for vim, gvim, evim, vimdiff, etc..\n"
,
"Icons on the desktop\n"
"--------------------\n"
"(these choices are only available when installing gvim)\n"
"In Windows 95 and later, shortcuts (icons) can be created on the Desktop.\n"
,
"Create plugin directories\n"
"-------------------------\n"
"Plugin directories allow extending Vim by dropping a file into a directory.\n"
"This choice allows creating them in $HOME (if you have a home directory) or\n"
"$VIM (used for everybody on the system).\n"
,
NULL
};
int i;
int c;
rewind(stdin);
printf("\n");
for (i = 0; items[i] != NULL; ++i)
{
puts(items[i]);
printf("Hit Enter to continue, b (back) or q (quit help): ");
c = getchar();
rewind(stdin);
if (c == 'b' || c == 'B')
{
if (i == 0)
--i;
else
i -= 2;
}
if (c == 'q' || c == 'Q')
break;
printf("\n");
}
}
static void
install(void)
{
int i;
for (i = 0; i < choice_count; ++i)
if (choices[i].installfunc != NULL && choices[i].active)
(choices[i].installfunc)(i);
if (install_popup
|| install_openwith
|| (need_uninstall_entry && interactive)
|| !interactive)
install_registry();
if (has_gvim)
install_OLE_register();
}
static void
request_choice(void)
{
int i;
printf("\n\nInstall will do for you:\n");
for (i = 0; i < choice_count; ++i)
if (choices[i].active)
printf("%2d %s\n", i + 1, choices[i].text);
printf("To change an item, enter its number\n\n");
printf("Enter item number, h (help), d (do it) or q (quit): ");
}
int
main(int argc, char **argv)
{
int i;
char buf[BUFSIZE];
if (argc > 1)
interactive = 0;
else
interactive = 1;
do_inits(argv);
init_homedir();
if (argc > 1 && strcmp(argv[1], "-uninstall-check") == 0)
{
i = uninstall_check(1);
get_vim_env();
if (i && _isatty(1))
sleep(3);
exit(0);
}
printf("This program sets up the installation of Vim "
VIM_VERSION_MEDIUM "\n\n");
check_unpack();
if (interactive)
uninstall_check(0);
inspect_system();
if (interactive)
{
setup_choices();
for (;;)
{
request_choice();
rewind(stdin);
if (scanf("%99s", buf) == 1)
{
if (isdigit(buf[0]))
{
i = atoi(buf);
if (i > 0 && i <= choice_count && choices[i - 1].active)
(choices[i - 1].changefunc)(i - 1);
else
printf("\nIllegal choice\n");
}
else if (buf[0] == 'h' || buf[0] == 'H')
{
show_help();
}
else if (buf[0] == 'd' || buf[0] == 'D')
{
install();
printf("\nThat finishes the installation. Happy Vimming!\n");
break;
}
else if (buf[0] == 'q' || buf[0] == 'Q')
{
printf("\nExiting without anything done\n");
break;
}
else
printf("\nIllegal choice\n");
}
}
printf("\n");
myexit(0);
}
else
{
command_line_setup_choices(argc, argv);
install();
sleep(2);
}
return 0;
}
