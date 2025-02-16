#if !defined(R2_DEBUG_H)
#define R2_DEBUG_H

#include <r_types.h>
#include <r_anal.h>
#include <r_cons.h>
#include <r_hash.h>
#include <r_util.h>
#include <r_reg.h>
#include <r_egg.h>
#include <r_bp.h>
#include <r_io.h>
#include <r_syscall.h>

#include <r_config.h>
#include "r_bind.h"
#if defined(__cplusplus)
extern "C" {
#endif

R_LIB_VERSION_HEADER(r_debug);



#if __BSD__
#include <machine/reg.h>


#undef reg
#undef fpreg
#undef fpstate
#undef trapframe
#undef rwindow

#if defined(PTRACE_SYSCALL)

#undef PTRACE_SYSCALL
#endif

#define PTRACE_PEEKTEXT PT_READ_I
#define PTRACE_POKETEXT PT_WRITE_I
#define PTRACE_PEEKDATA PT_READ_D
#define PTRACE_POKEDATA PT_WRITE_D
#define PTRACE_ATTACH PT_ATTACH
#define PTRACE_DETACH PT_DETACH
#define PTRACE_SINGLESTEP PT_STEP
#define PTRACE_CONT PT_CONTINUE
#define PTRACE_GETREGS PT_GETREGS
#define PTRACE_SETREGS PT_SETREGS
#define PTRACE_SYSCALL PT_STEP
#endif

#define SNAP_PAGE_SIZE 4096
#define CHECK_POINT_LIMIT 0x100000 



typedef enum {
R_DBG_PROC_STOP = 's',
R_DBG_PROC_RUN = 'r',
R_DBG_PROC_SLEEP = 'S',
R_DBG_PROC_ZOMBIE = 'z',
R_DBG_PROC_DEAD = 'd',
R_DBG_PROC_RAISED = 'R' 
} RDebugPidState;



typedef enum {
R_DBG_SIGNAL_IGNORE = 0, 
R_DBG_SIGNAL_CONT = 1, 
R_DBG_SIGNAL_SKIP = 2, 

} RDebugSignalMode;






typedef enum {
R_DBG_RECOIL_NONE = 0,
R_DBG_RECOIL_STEP,
R_DBG_RECOIL_CONTINUE
} RDebugRecoilMode;




typedef enum {
R_DEBUG_REASON_DEAD = -1,
R_DEBUG_REASON_NONE = 0,
R_DEBUG_REASON_SIGNAL,
R_DEBUG_REASON_SEGFAULT,
R_DEBUG_REASON_BREAKPOINT,
R_DEBUG_REASON_TRACEPOINT,
R_DEBUG_REASON_COND,
R_DEBUG_REASON_READERR,
R_DEBUG_REASON_STEP,
R_DEBUG_REASON_ABORT,
R_DEBUG_REASON_WRITERR,
R_DEBUG_REASON_DIVBYZERO,
R_DEBUG_REASON_ILLEGAL,
R_DEBUG_REASON_UNKNOWN,
R_DEBUG_REASON_ERROR,
R_DEBUG_REASON_NEW_PID,
R_DEBUG_REASON_NEW_TID,
R_DEBUG_REASON_NEW_LIB,
R_DEBUG_REASON_EXIT_PID,
R_DEBUG_REASON_EXIT_TID,
R_DEBUG_REASON_EXIT_LIB,
R_DEBUG_REASON_TRAP,
R_DEBUG_REASON_SWI,
R_DEBUG_REASON_INT,
R_DEBUG_REASON_FPU,
R_DEBUG_REASON_USERSUSP,
} RDebugReasonType;



typedef struct r_debug_frame_t {
ut64 addr;
int size;
ut64 sp;
ut64 bp;
} RDebugFrame;

typedef struct r_debug_reason_t {
int type;
int tid;
int signum;
ut64 bp_addr;
ut64 timestamp;
ut64 addr;
ut64 ptr;
} RDebugReason;

typedef struct r_debug_map_t {
char *name;
ut64 addr;
ut64 addr_end;
ut64 size;
ut64 offset;
char *file;
int perm;
int user;
bool shared;
} RDebugMap;

typedef struct r_debug_signal_t {
int type;
int num;
ut64 handler;
} RDebugSignal;

typedef struct r_debug_desc_t {
int fd;
char *path;
int perm;
int type;
ut64 off;
} RDebugDesc;

struct r_debug_snap_diff_t;
typedef struct r_page_data_t {
struct r_debug_snap_diff_t *diff; 
ut32 page_off;
ut8 *data;
ut8 hash[128];
} RPageData;

struct r_debug_snap_t;
typedef struct r_debug_snap_diff_t {
struct r_debug_snap_t *base;
RList *pages; 
RPageData **last_changes; 
} RDebugSnapDiff;

typedef struct r_debug_snap_t {
ut64 addr;
ut64 addr_end;
ut8 *data;
ut32 size;
ut32 page_num;
ut64 timestamp;
RHash *hash_ctx;
ut8 **hashes; 
RList *history; 
int perm;
char *comment;
} RDebugSnap;

typedef struct r_debug_key {
ut64 addr;
ut32 id;
} RDebugKey;

typedef struct r_debug_session_t {
RDebugKey key;
RListIter *reg[R_REG_TYPE_LAST];
RList *memlist; 


char *comment;
} RDebugSession;


typedef struct r_session_header {
ut64 addr;
ut32 id;
ut32 difflist_len;
} RSessionHeader;

typedef struct r_diff_entry {
ut32 base_idx;
ut32 pages_len;
} RDiffEntry;

typedef struct r_snap_entry {
ut64 addr;
ut32 size;
ut64 timestamp;
int perm;
} RSnapEntry;

typedef struct r_debug_trace_t {
RList *traces;
int count;
int enabled;

int tag;
int dup;
char *addresses;

Sdb *db;
} RDebugTrace;

typedef struct r_debug_tracepoint_t {
ut64 addr;
ut64 tags; 
int tag; 
int size;
int count;
int times;
ut64 stamp;
} RDebugTracepoint;

typedef struct r_debug_t {
char *arch;
int bits; 
int hitinfo;

int main_pid;
int pid; 
int tid; 
int forked_pid; 
int n_threads;
RList *threads; 

char *malloc; 




int bpsize; 
char *btalgo; 
int btdepth; 
int regcols; 
int swstep; 
int stop_all_threads; 
int trace_forks; 
int trace_execs; 
int trace_aftersyscall; 
int trace_clone; 
int follow_child; 
char *glob_libs; 
char *glob_unlibs; 
bool consbreak; 
bool continue_all_threads;


int steps; 
RDebugReason reason; 
RDebugRecoilMode recoil_mode; 
ut64 stopaddr; 


RDebugTrace *trace;
Sdb *tracenodes;
RTree *tree;
RList *call_frames;

RReg *reg;
RList *q_regs;
const char *creg; 
RBreakpoint *bp;
void *user; 
char *snap_path;


PrintfCallback cb_printf;
RIOBind iob;

struct r_debug_plugin_t *h;
RList *plugins;

bool pc_at_bp; 
bool pc_at_bp_set; 

REvent *ev;

RAnal *anal;
RList *maps; 
RList *maps_user; 
RList *snaps; 
RList *sessions; 
Sdb *sgnls;
RCoreBind corebind;

int _mode;
RNum *num;
REgg *egg;
bool verbose;
} RDebug;

typedef struct r_debug_desc_plugin_t {
int (*open)(const char *path);
int (*close)(int fd);
int (*read)(int fd, ut64 addr, int len);
int (*write)(int fd, ut64 addr, int len);
int (*seek)(int fd, ut64 addr);
int (*dup)(int fd, int newfd);
RList* (*list)(int pid);
} RDebugDescPlugin;

typedef struct r_debug_info_t {
int pid;
int tid;
int uid;
int gid;
char *usr;
char *exe;
char *cmdline;
char *libname;
char *cwd;
int status; 
int signum;
void * lib;
void * thread;
char *kernel_stack;




} RDebugInfo;


typedef struct r_debug_plugin_t {
const char *name;
const char *license;
const char *author;
const char *version;

ut32 bits;
const char *arch;
int canstep;
int keepio;

RDebugInfo* (*info)(RDebug *dbg, const char *arg);
int (*startv)(int argc, char **argv);
int (*attach)(RDebug *dbg, int pid);
int (*detach)(RDebug *dbg, int pid);
int (*select)(RDebug *dbg, int pid, int tid);
RList *(*threads)(RDebug *dbg, int pid);
RList *(*pids)(RDebug *dbg, int pid);
RList *(*tids)(RDebug *dbg, int pid);
RFList (*backtrace)(RDebug *dbg, int count);

int (*stop)(RDebug *dbg);
int (*step)(RDebug *dbg);
int (*step_over)(RDebug *dbg);
int (*cont)(RDebug *dbg, int pid, int tid, int sig);
int (*wait)(RDebug *dbg, int pid);
bool (*gcore)(RDebug *dbg, RBuffer *dest);
bool (*kill)(RDebug *dbg, int pid, int tid, int sig);
RList* (*kill_list)(RDebug *dbg);
int (*contsc)(RDebug *dbg, int pid, int sc);
RList* (*frames)(RDebug *dbg, ut64 at);
RBreakpointCallback breakpoint;

int (*reg_read)(RDebug *dbg, int type, ut8 *buf, int size);
int (*reg_write)(RDebug *dbg, int type, const ut8 *buf, int size); 
char* (*reg_profile)(RDebug *dbg);
int (*set_reg_profile)(const char *str);

RList *(*map_get)(RDebug *dbg);
RList *(*modules_get)(RDebug *dbg);
RDebugMap* (*map_alloc)(RDebug *dbg, ut64 addr, int size, bool thp);
int (*map_dealloc)(RDebug *dbg, ut64 addr, int size);
int (*map_protect)(RDebug *dbg, ut64 addr, int size, int perms);
int (*init)(RDebug *dbg);
int (*drx)(RDebug *dbg, int n, ut64 addr, int size, int rwx, int g, int api_type);
RDebugDescPlugin desc;

} RDebugPlugin;


typedef struct r_debug_pid_t {
int pid;
int ppid;
char status; 
int runnable; 
bool signalled;
char *path;
int uid;
int gid;
ut64 pc;
} RDebugPid;







#if defined(R_API)
R_API RDebug *r_debug_new(int hard);
R_API RDebug *r_debug_free(RDebug *dbg);

R_API int r_debug_attach(RDebug *dbg, int pid);
R_API int r_debug_detach(RDebug *dbg, int pid);
R_API int r_debug_startv(RDebug *dbg, int argc, char **argv);
R_API int r_debug_start(RDebug *dbg, const char *cmd);


R_API RDebugReasonType r_debug_stop_reason(RDebug *dbg);
R_API const char *r_debug_reason_to_string(int type);


R_API RDebugReasonType r_debug_wait(RDebug *dbg, RBreakpointItem **bp);


R_API int r_debug_step(RDebug *dbg, int steps);
R_API int r_debug_step_over(RDebug *dbg, int steps);
R_API int r_debug_continue_until(RDebug *dbg, ut64 addr);
R_API int r_debug_continue_until_nonblock(RDebug *dbg, ut64 addr);
R_API int r_debug_continue_until_optype(RDebug *dbg, int type, int over);
R_API int r_debug_continue_until_nontraced(RDebug *dbg);
R_API int r_debug_continue_syscall(RDebug *dbg, int sc);
R_API int r_debug_continue_syscalls(RDebug *dbg, int *sc, int n_sc);
R_API int r_debug_continue(RDebug *dbg);
R_API int r_debug_continue_kill(RDebug *dbg, int signal);
#if __WINDOWS__
R_API int r_debug_continue_pass_exception(RDebug *dbg);
#endif


R_API bool r_debug_select(RDebug *dbg, int pid, int tid);




R_API int r_debug_pid_list(RDebug *dbg, int pid, char fmt);
R_API RDebugPid *r_debug_pid_new(const char *path, int pid, int uid, char status, ut64 pc);
R_API RDebugPid *r_debug_pid_free(RDebugPid *pid);
R_API RList *r_debug_pids(RDebug *dbg, int pid);

R_API bool r_debug_set_arch(RDebug *dbg, const char *arch, int bits);
R_API bool r_debug_use(RDebug *dbg, const char *str);

R_API RDebugInfo *r_debug_info(RDebug *dbg, const char *arg);
R_API void r_debug_info_free (RDebugInfo *rdi);

R_API ut64 r_debug_get_baddr(RDebug *dbg, const char *file);


R_API void r_debug_signal_init(RDebug *dbg);
R_API int r_debug_signal_send(RDebug *dbg, int num);
R_API int r_debug_signal_what(RDebug *dbg, int num);
R_API int r_debug_signal_resolve(RDebug *dbg, const char *signame);
R_API const char *r_debug_signal_resolve_i(RDebug *dbg, int signum);
R_API void r_debug_signal_setup(RDebug *dbg, int num, int opt);
R_API int r_debug_signal_set(RDebug *dbg, int num, ut64 addr);
R_API void r_debug_signal_list(RDebug *dbg, int mode);
R_API int r_debug_kill(RDebug *dbg, int pid, int tid, int sig);
R_API RList *r_debug_kill_list(RDebug *dbg);

R_API int r_debug_kill_setup(RDebug *dbg, int sig, int action);


R_API void r_debug_plugin_init(RDebug *dbg);
R_API int r_debug_plugin_set(RDebug *dbg, const char *str);
R_API int r_debug_plugin_list(RDebug *dbg, int mode);
R_API bool r_debug_plugin_add(RDebug *dbg, RDebugPlugin *foo);
R_API bool r_debug_plugin_set_reg_profile(RDebug *dbg, const char *str);


R_API RList *r_debug_modules_list(RDebug*);
R_API RDebugMap *r_debug_map_alloc(RDebug *dbg, ut64 addr, int size, bool thp);
R_API int r_debug_map_dealloc(RDebug *dbg, RDebugMap *map);
R_API RList *r_debug_map_list_new(void);
R_API RDebugMap *r_debug_map_get(RDebug *dbg, ut64 addr);
R_API RDebugMap *r_debug_map_new (char *name, ut64 addr, ut64 addr_end, int perm, int user);
R_API void r_debug_map_free(RDebugMap *map);
R_API void r_debug_map_list(RDebug *dbg, ut64 addr, const char *input);
R_API void r_debug_map_list_visual(RDebug *dbg, ut64 addr, const char *input, int colors);


R_API RDebugDesc *r_debug_desc_new (int fd, char* path, int perm, int type, int off);
R_API void r_debug_desc_free (RDebugDesc *p);
R_API int r_debug_desc_open(RDebug *dbg, const char *path);
R_API int r_debug_desc_close(RDebug *dbg, int fd);
R_API int r_debug_desc_dup(RDebug *dbg, int fd, int newfd);
R_API int r_debug_desc_read(RDebug *dbg, int fd, ut64 addr, int len);
R_API int r_debug_desc_seek(RDebug *dbg, int fd, ut64 addr); 
R_API int r_debug_desc_write(RDebug *dbg, int fd, ut64 addr, int len);
R_API int r_debug_desc_list(RDebug *dbg, int rad);


R_API int r_debug_reg_sync(RDebug *dbg, int type, int write);
R_API int r_debug_reg_list(RDebug *dbg, int type, int size, int rad, const char *use_color);
R_API int r_debug_reg_set(RDebug *dbg, const char *name, ut64 num);
R_API ut64 r_debug_reg_get(RDebug *dbg, const char *name);
R_API ut64 r_debug_reg_get_err(RDebug *dbg, const char *name, int *err, utX *value);

R_API ut64 r_debug_execute(RDebug *dbg, const ut8 *buf, int len, int restore);
R_API bool r_debug_map_sync(RDebug *dbg);

R_API int r_debug_stop(RDebug *dbg);


R_API RList *r_debug_frames(RDebug *dbg, ut64 at);

R_API bool r_debug_is_dead(RDebug *dbg);
R_API int r_debug_map_protect(RDebug *dbg, ut64 addr, int size, int perms);

R_API ut64 r_debug_arg_get(RDebug *dbg, int fast, int num);
R_API bool r_debug_arg_set(RDebug *dbg, int fast, int num, ut64 value);


R_API RBreakpointItem *r_debug_bp_add(RDebug *dbg, ut64 addr, int hw, bool watch, int rw, char *module, st64 m_delta);
R_API void r_debug_bp_rebase(RDebug *dbg, ut64 old_base, ut64 new_base);
R_API void r_debug_bp_update(RDebug *dbg);


R_API int r_debug_thread_list(RDebug *dbg, int pid, char fmt);

R_API void r_debug_tracenodes_reset(RDebug *dbg);

R_API void r_debug_trace_reset(RDebug *dbg);
R_API int r_debug_trace_pc(RDebug *dbg, ut64 pc);
R_API void r_debug_trace_at(RDebug *dbg, const char *str);
R_API RDebugTracepoint *r_debug_trace_get(RDebug *dbg, ut64 addr);
R_API void r_debug_trace_list(RDebug *dbg, int mode, ut64 offset);
R_API RDebugTracepoint *r_debug_trace_add(RDebug *dbg, ut64 addr, int size);
R_API RDebugTrace *r_debug_trace_new(void);
R_API void r_debug_trace_free(RDebugTrace *dbg);
R_API int r_debug_trace_tag(RDebug *dbg, int tag);
R_API int r_debug_child_fork(RDebug *dbg);
R_API int r_debug_child_clone(RDebug *dbg);

R_API void r_debug_drx_list(RDebug *dbg);
R_API int r_debug_drx_set(RDebug *dbg, int idx, ut64 addr, int len, int rwx, int g);
R_API int r_debug_drx_unset(RDebug *dbg, int idx);


R_API ut64 r_debug_num_callback(RNum *userptr, const char *str, int *ok);
R_API int r_debug_esil_stepi(RDebug *dbg);
R_API ut64 r_debug_esil_step(RDebug *dbg, ut32 count);
R_API ut64 r_debug_esil_continue(RDebug *dbg);
R_API void r_debug_esil_watch(RDebug *dbg, int rwx, int dev, const char *expr);
R_API void r_debug_esil_watch_reset(RDebug *dbg);
R_API void r_debug_esil_watch_list(RDebug *dbg);
R_API int r_debug_esil_watch_empty(RDebug *dbg);
R_API void r_debug_esil_prestep (RDebug *d, int p);


R_API RDebugSnap *r_debug_snap_new(void);
R_API void r_debug_snap_free(void *snap);
R_API int r_debug_snap_delete(RDebug *dbg, int idx);
R_API void r_debug_snap_list(RDebug *dbg, int idx, int mode);
R_API int r_debug_snap(RDebug *dbg, ut64 addr);
R_API int r_debug_snap_comment(RDebug *dbg, int idx, const char *msg);
R_API RDebugSnapDiff *r_debug_snap_map(RDebug *dbg, RDebugMap *map);
R_API int r_debug_snap_all(RDebug *dbg, int perms);
R_API RDebugSnap *r_debug_snap_get(RDebug *dbg, ut64 addr);
R_API int r_debug_snap_set_idx(RDebug *dbg, int idx);
R_API int r_debug_snap_set(RDebug *dbg, RDebugSnap *snap);


R_API void r_debug_diff_free(void *p);
R_API RDebugSnapDiff *r_debug_diff_add(RDebug *dbg, RDebugSnap *base);
R_API void r_debug_diff_set(RDebug *dbg, RDebugSnapDiff *diff);
R_API void r_debug_diff_set_base(RDebug *dbg, RDebugSnap *base);


R_API void r_page_data_free(void *p);


R_API void r_debug_session_free(void *p);
R_API void r_debug_session_list(RDebug *dbg);
R_API RDebugSession *r_debug_session_add(RDebug *dbg, RListIter **tail);
R_API bool r_debug_session_delete(RDebug *dbg, int idx);
R_API bool r_debug_session_comment(RDebug *dbg, int idx, const char *msg);
R_API void r_debug_session_path(RDebug *dbg, const char *path);
R_API void r_debug_session_set(RDebug *dbg, RDebugSession *session);
R_API bool r_debug_session_set_idx(RDebug *dbg, int idx);
R_API RDebugSession *r_debug_session_get(RDebug *dbg, RListIter *tail);
R_API void r_debug_session_save(RDebug *dbg, const char *file);
R_API void r_debug_session_restore(RDebug *dbg, const char *file);
R_API bool r_debug_step_back(RDebug *dbg);
R_API bool r_debug_continue_back(RDebug *dbg);


#if HAVE_PTRACE
static inline long r_debug_ptrace(RDebug *dbg, r_ptrace_request_t request, pid_t pid, void *addr, r_ptrace_data_t data) {
return dbg->iob.ptrace (dbg->iob.io, request, pid, addr, data);
}

static inline void *r_debug_ptrace_func(RDebug *dbg, void *(*func)(void *), void *user) {
return dbg->iob.ptrace_func (dbg->iob.io, func, user);
}
#endif


extern RDebugPlugin r_debug_plugin_native;
extern RDebugPlugin r_debug_plugin_esil;
extern RDebugPlugin r_debug_plugin_rap;
extern RDebugPlugin r_debug_plugin_gdb;
extern RDebugPlugin r_debug_plugin_bf;
extern RDebugPlugin r_debug_plugin_io;
extern RDebugPlugin r_debug_plugin_windbg;
extern RDebugPlugin r_debug_plugin_bochs;
extern RDebugPlugin r_debug_plugin_qnx;
extern RDebugPlugin r_debug_plugin_null;
#endif

#if defined(__cplusplus)
}
#endif

#endif







#if 0
Missing callbacks
=================
- alloc
- dealloc
- list maps (memory regions)
- change memory protections
- touchtrace
- filedescriptor set/get/mod..
- get/set signals
- get regs, set regs

#endif
