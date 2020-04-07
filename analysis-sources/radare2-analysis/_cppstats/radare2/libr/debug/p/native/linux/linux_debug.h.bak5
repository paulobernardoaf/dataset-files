







#include <limits.h>
#include <sys/ptrace.h>

struct user_regs_struct_x86_64 {
ut64 r15; ut64 r14; ut64 r13; ut64 r12; ut64 rbp; ut64 rbx; ut64 r11;
ut64 r10; ut64 r9; ut64 r8; ut64 rax; ut64 rcx; ut64 rdx; ut64 rsi;
ut64 rdi; ut64 orig_rax; ut64 rip; ut64 cs; ut64 eflags; ut64 rsp;
ut64 ss; ut64 fs_base; ut64 gs_base; ut64 ds; ut64 es; ut64 fs; ut64 gs;
};

struct user_regs_struct_x86_32 {
ut32 ebx; ut32 ecx; ut32 edx; ut32 esi; ut32 edi; ut32 ebp; ut32 eax;
ut32 xds; ut32 xes; ut32 xfs; ut32 xgs; ut32 orig_eax; ut32 eip;
ut32 xcs; ut32 eflags; ut32 esp; ut32 xss;
};

#if __ANDROID__

#if __arm64__ || __aarch64__
#define R_DEBUG_REG_T struct user_pt_regs

#if !defined(NT_PRSTATUS)
#define NT_PRSTATUS 1
#endif

#else
#define R_DEBUG_REG_T struct pt_regs
#endif

#else

#include <sys/user.h>
#if __i386__ || __x86_64__
#define R_DEBUG_REG_T struct user_regs_struct
#elif __s390x__ || __s390__
#define R_DEBUG_REG_T struct _user_regs_struct
#if 0

50 struct _user_regs_struct
51 {
52 struct _user_psw_struct psw; 
53 unsigned long gprs[16]; 
54 unsigned int acrs[16]; 
55 unsigned long orig_gpr2; 
56 struct _user_fpregs_struct fp_regs; 
57 struct _user_per_struct per_info; 
58 unsigned long ieee_instruction_pointer; 
59 };
#endif
#elif __arm64__ || __aarch64__
#include <asm/ptrace.h>
#if !defined(NT_PRSTATUS)
#define NT_PRSTATUS 1
#endif
#define R_DEBUG_REG_T struct user_pt_regs
#elif __arm__
#define R_DEBUG_REG_T struct user_regs
#elif __POWERPC__
struct powerpc_regs_t {
unsigned long gpr[32];
unsigned long nip;
unsigned long msr;
unsigned long orig_gpr3; 
unsigned long ctr;
unsigned long link;
unsigned long xer;
unsigned long ccr;
#if defined(__powerpc64__)
unsigned long softe; 
#else
unsigned long mq; 

#endif
unsigned long trap; 


unsigned long dar; 
unsigned long dsisr; 
unsigned long result; 
};
#define R_DEBUG_REG_T struct powerpc_regs_t
#elif __riscv || __riscv__ || __riscv64__

#include <sys/ucontext.h>
#include <asm/ptrace.h>



#define R_DEBUG_REG_T struct user_regs_struct 


#elif __mips__

#include <sys/ucontext.h>
typedef ut64 mips64_regs_t [274];
#define R_DEBUG_REG_T mips64_regs_t
#endif
#endif


bool linux_set_options(RDebug *dbg, int pid);
int linux_step(RDebug *dbg);
RDebugReasonType linux_ptrace_event(RDebug *dbg, int pid, int status);
int linux_attach(RDebug *dbg, int pid);
bool linux_attach_new_process(RDebug *dbg, int pid);
RDebugInfo *linux_info(RDebug *dbg, const char *arg);
RList *linux_pid_list(int pid, RList *list);
RList *linux_thread_list(RDebug *dbg, int pid, RList *list);
bool linux_select(RDebug *dbg, int pid, int tid);
RDebugPid *fill_pid_info(const char *info, const char *path, int tid);
int linux_reg_read(RDebug *dbg, int type, ut8 *buf, int size);
int linux_reg_write(RDebug *dbg, int type, const ut8 *buf, int size);
RList *linux_desc_list(int pid);
bool linux_stop_threads(RDebug *dbg, int except);
int linux_handle_signals(RDebug *dbg, int tid);
int linux_dbg_wait(RDebug *dbg, int pid);
char *linux_reg_profile(RDebug *dbg);
int match_pid(const void *pid_o, const void *th_o);
