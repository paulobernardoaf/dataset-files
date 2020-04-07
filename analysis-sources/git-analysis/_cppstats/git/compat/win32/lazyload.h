struct proc_addr {
const char *const dll;
const char *const function;
FARPROC pfunction;
unsigned initialized : 1;
};
#define DECLARE_PROC_ADDR(dll, rettype, function, ...) static struct proc_addr proc_addr_##function = { #dll, #function, NULL, 0 }; static rettype (WINAPI *function)(__VA_ARGS__)
#define INIT_PROC_ADDR(function) (function = get_proc_addr(&proc_addr_##function))
static inline void *get_proc_addr(struct proc_addr *proc)
{
if (!proc->initialized) {
HANDLE hnd;
proc->initialized = 1;
hnd = LoadLibraryExA(proc->dll, NULL,
LOAD_LIBRARY_SEARCH_SYSTEM32);
if (hnd)
proc->pfunction = GetProcAddress(hnd, proc->function);
}
if (!proc->pfunction)
errno = ENOSYS;
return proc->pfunction;
}
