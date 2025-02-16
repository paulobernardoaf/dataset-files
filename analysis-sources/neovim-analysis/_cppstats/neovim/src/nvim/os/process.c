#include <uv.h> 
#if defined(WIN32)
#include <tlhelp32.h> 
#endif
#if defined(__FreeBSD__) 
#include <string.h>
#include <sys/types.h>
#include <sys/user.h>
#endif
#if defined(__NetBSD__) || defined(__OpenBSD__)
#include <sys/param.h>
#endif
#if defined(__APPLE__) || defined(BSD)
#include <sys/sysctl.h>
#include <pwd.h>
#endif
#include "nvim/globals.h"
#include "nvim/log.h"
#include "nvim/os/process.h"
#include "nvim/os/os.h"
#include "nvim/os/os_defs.h"
#include "nvim/api/private/helpers.h"
#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "os/process.c.generated.h"
#endif
#if defined(WIN32)
static bool os_proc_tree_kill_rec(HANDLE process, int sig)
{
if (process == NULL) {
return false;
}
PROCESSENTRY32 pe;
DWORD pid = GetProcessId(process);
if (pid != 0) {
HANDLE h = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
if (h != INVALID_HANDLE_VALUE) {
pe.dwSize = sizeof(PROCESSENTRY32);
if (!Process32First(h, &pe)) {
goto theend;
}
do {
if (pe.th32ParentProcessID == pid) {
HANDLE ph = OpenProcess(PROCESS_ALL_ACCESS, false, pe.th32ProcessID);
if (ph != NULL) {
os_proc_tree_kill_rec(ph, sig);
CloseHandle(ph);
}
}
} while (Process32Next(h, &pe));
CloseHandle(h);
}
}
theend:
return (bool)TerminateProcess(process, (unsigned int)sig);
}
bool os_proc_tree_kill(int pid, int sig)
{
assert(sig >= 0);
assert(sig == SIGTERM || sig == SIGKILL);
if (pid > 0) {
ILOG("terminating process tree: %d", pid);
HANDLE h = OpenProcess(PROCESS_ALL_ACCESS, false, (DWORD)pid);
return os_proc_tree_kill_rec(h, sig);
} else {
ELOG("invalid pid: %d", pid);
}
return false;
}
#else
bool os_proc_tree_kill(int pid, int sig)
{
assert(sig == SIGTERM || sig == SIGKILL);
if (pid == 0) {
return false;
}
ILOG("sending %s to PID %d", sig == SIGTERM ? "SIGTERM" : "SIGKILL", -pid);
return uv_kill(-pid, sig) == 0;
}
#endif
int os_proc_children(int ppid, int **proc_list, size_t *proc_count)
{
if (ppid < 0) {
return 2;
}
int *temp = NULL;
*proc_list = NULL;
*proc_count = 0;
#if defined(WIN32)
PROCESSENTRY32 pe;
HANDLE h = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
if (h == INVALID_HANDLE_VALUE) {
return 2;
}
pe.dwSize = sizeof(PROCESSENTRY32);
if (!Process32First(h, &pe)) {
CloseHandle(h);
return 2;
}
do {
if (pe.th32ParentProcessID == (DWORD)ppid) {
temp = xrealloc(temp, (*proc_count + 1) * sizeof(*temp));
temp[*proc_count] = (int)pe.th32ProcessID;
(*proc_count)++;
}
} while (Process32Next(h, &pe));
CloseHandle(h);
#elif defined(__APPLE__) || defined(BSD)
#if defined(__APPLE__)
#define KP_PID(o) o.kp_proc.p_pid
#define KP_PPID(o) o.kp_eproc.e_ppid
#elif defined(__FreeBSD__)
#define KP_PID(o) o.ki_pid
#define KP_PPID(o) o.ki_ppid
#else
#define KP_PID(o) o.p_pid
#define KP_PPID(o) o.p_ppid
#endif
#if defined(__NetBSD__)
static int name[] = {
CTL_KERN, KERN_PROC2, KERN_PROC_ALL, 0, (int)(sizeof(struct kinfo_proc2)), 0
};
#else
static int name[] = { CTL_KERN, KERN_PROC, KERN_PROC_ALL, 0 };
#endif
size_t len = 0;
int rv = sysctl(name, ARRAY_SIZE(name) - 1, NULL, &len, NULL, 0);
if (rv) {
return 2;
}
#if defined(__NetBSD__)
struct kinfo_proc2 *p_list = xmalloc(len);
#else
struct kinfo_proc *p_list = xmalloc(len);
#endif
rv = sysctl(name, ARRAY_SIZE(name) - 1, p_list, &len, NULL, 0);
if (rv) {
xfree(p_list);
return 2;
}
bool exists = false;
size_t p_count = len / sizeof(*p_list);
for (size_t i = 0; i < p_count; i++) {
exists = exists || KP_PID(p_list[i]) == ppid;
if (KP_PPID(p_list[i]) == ppid) {
temp = xrealloc(temp, (*proc_count + 1) * sizeof(*temp));
temp[*proc_count] = KP_PID(p_list[i]);
(*proc_count)++;
}
}
xfree(p_list);
if (!exists) {
return 1; 
}
#elif defined(__linux__)
char proc_p[256] = { 0 };
snprintf(proc_p, sizeof(proc_p), "/proc/%d/task/%d/children", ppid, ppid);
FILE *fp = fopen(proc_p, "r");
if (fp == NULL) {
return 2; 
}
int match_pid;
while (fscanf(fp, "%d", &match_pid) > 0) {
temp = xrealloc(temp, (*proc_count + 1) * sizeof(*temp));
temp[*proc_count] = match_pid;
(*proc_count)++;
}
fclose(fp);
#endif
*proc_list = temp;
return 0;
}
#if defined(WIN32)
Dictionary os_proc_info(int pid)
{
Dictionary pinfo = ARRAY_DICT_INIT;
PROCESSENTRY32 pe;
HANDLE h = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
if (h == INVALID_HANDLE_VALUE) {
return pinfo; 
}
pe.dwSize = sizeof(PROCESSENTRY32);
if (!Process32First(h, &pe)) {
CloseHandle(h);
return pinfo; 
}
do {
if (pe.th32ProcessID == (DWORD)pid) {
break;
}
} while (Process32Next(h, &pe));
CloseHandle(h);
if (pe.th32ProcessID == (DWORD)pid) {
PUT(pinfo, "pid", INTEGER_OBJ(pid));
PUT(pinfo, "ppid", INTEGER_OBJ((int)pe.th32ParentProcessID));
PUT(pinfo, "name", STRING_OBJ(cstr_to_string(pe.szExeFile)));
}
return pinfo;
}
#endif
bool os_proc_running(int pid)
{
return uv_kill(pid, 0) == 0;
}
