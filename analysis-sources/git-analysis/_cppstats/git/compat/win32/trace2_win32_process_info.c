#include "../../cache.h"
#include "../../json-writer.h"
#include "lazyload.h"
#include <Psapi.h>
#include <tlHelp32.h>
#define NR_PIDS_LIMIT 10
static int find_pid(DWORD pid, HANDLE hSnapshot, PROCESSENTRY32 *pe32)
{
pe32->dwSize = sizeof(PROCESSENTRY32);
if (Process32First(hSnapshot, pe32)) {
do {
if (pe32->th32ProcessID == pid)
return 1;
} while (Process32Next(hSnapshot, pe32));
}
return 0;
}
static void get_processes(struct json_writer *jw, HANDLE hSnapshot)
{
PROCESSENTRY32 pe32;
DWORD pid;
DWORD pid_list[NR_PIDS_LIMIT];
int k, nr_pids = 0;
pid = GetCurrentProcessId();
while (find_pid(pid, hSnapshot, &pe32)) {
if (nr_pids)
jw_array_string(jw, pe32.szExeFile);
for (k = 0; k < nr_pids; k++)
if (pid == pid_list[k]) {
jw_array_string(jw, "(cycle)");
return;
}
if (nr_pids == NR_PIDS_LIMIT) {
jw_array_string(jw, "(truncated)");
return;
}
pid_list[nr_pids++] = pid;
pid = pe32.th32ParentProcessID;
}
}
static void get_ancestry(void)
{
HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
if (hSnapshot != INVALID_HANDLE_VALUE) {
struct json_writer jw = JSON_WRITER_INIT;
jw_array_begin(&jw, 0);
get_processes(&jw, hSnapshot);
jw_end(&jw);
trace2_data_json("process", the_repository, "windows/ancestry",
&jw);
jw_release(&jw);
CloseHandle(hSnapshot);
}
}
static void get_is_being_debugged(void)
{
if (IsDebuggerPresent())
trace2_data_intmax("process", the_repository,
"windows/debugger_present", 1);
}
static void get_peak_memory_info(void)
{
DECLARE_PROC_ADDR(psapi.dll, BOOL, GetProcessMemoryInfo, HANDLE,
PPROCESS_MEMORY_COUNTERS, DWORD);
if (INIT_PROC_ADDR(GetProcessMemoryInfo)) {
PROCESS_MEMORY_COUNTERS pmc;
if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc,
sizeof(pmc))) {
struct json_writer jw = JSON_WRITER_INIT;
jw_object_begin(&jw, 0);
#define KV(kv) #kv, (intmax_t)pmc.kv
jw_object_intmax(&jw, KV(PageFaultCount));
jw_object_intmax(&jw, KV(PeakWorkingSetSize));
jw_object_intmax(&jw, KV(PeakPagefileUsage));
jw_end(&jw);
trace2_data_json("process", the_repository,
"windows/memory", &jw);
jw_release(&jw);
}
}
}
void trace2_collect_process_info(enum trace2_process_info_reason reason)
{
if (!trace2_is_enabled())
return;
switch (reason) {
case TRACE2_PROCESS_INFO_STARTUP:
get_is_being_debugged();
get_ancestry();
return;
case TRACE2_PROCESS_INFO_EXIT:
get_peak_memory_info();
return;
default:
BUG("trace2_collect_process_info: unknown reason '%d'", reason);
}
}
