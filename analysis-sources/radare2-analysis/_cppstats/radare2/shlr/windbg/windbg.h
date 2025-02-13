#include <stdint.h>
#include "kd.h"
typedef struct _WindCtx WindCtx;
typedef struct WindProc {
ut64 eprocess;
ut32 uniqueid;
ut64 vadroot;
ut64 dir_base_table;
ut64 peb;
char name[17];
} WindProc;
typedef struct WindThread {
ut32 uniqueid;
bool runnable;
char status;
ut64 ethread;
ut64 entrypoint;
} WindThread;
typedef struct WindModule {
char *name;
ut64 addr;
ut64 size;
} WindModule;
enum {
K_PaeEnabled = 0x036,
K_PsActiveProcessHead = 0x050,
K_CmNtCSDVersion = 0x268,
};
enum {
E_ActiveProcessLinks, 
E_UniqueProcessId, 
E_Peb, 
E_ImageFileName, 
E_VadRoot, 
E_ThreadListHead, 
P_DirectoryTableBase, 
P_ImageBaseAddress, 
P_ProcessParameters, 
R_ImagePathName, 
ET_Tcb, 
ET_ThreadListEntry, 
ET_Win32StartAddress, 
ET_Cid, 
C_UniqueThread, 
O_Max,
};
typedef struct {
int build;
int sp;
int bits;
int flags;
int f[O_Max];
} Profile;
int windbg_get_bits(WindCtx *ctx);
ut64 windbg_get_target_base(WindCtx *ctx);
ut32 windbg_get_target(WindCtx *ctx);
bool windbg_set_target(WindCtx *ctx, ut32 pid);
RList *windbg_list_process(WindCtx *ctx);
RList *windbg_list_threads(WindCtx *ctx);
RList *windbg_list_modules(WindCtx *ctx);
int windbg_get_cpus(WindCtx *ctx);
bool windbg_set_cpu(WindCtx *ctx, int cpu);
int windbg_get_cpu(WindCtx *ctx);
WindCtx * windbg_ctx_new(void *io_ptr);
void windbg_ctx_free(WindCtx **ctx);
int windbg_wait_packet(WindCtx *ctx, const ut32 type, kd_packet_t **p);
int windbg_sync(WindCtx *ctx);
bool windbg_read_ver(WindCtx *ctx);
int windbg_continue(WindCtx *ctx);
bool windbg_write_reg(WindCtx *ctx, const uint8_t *buf, int size);
int windbg_read_reg(WindCtx *ctx, uint8_t *buf, int size);
int windbg_query_mem(WindCtx *ctx, const ut64 addr, int *address_space, int *flags);
int windbg_bkpt(WindCtx *ctx, const ut64 addr, const int set, const int hw, int *handle);
int windbg_read_at(WindCtx *ctx, uint8_t *buf, const ut64 offset, const int count);
int windbg_read_at_uva(WindCtx *ctx, uint8_t *buf, ut64 offset, int count);
int windbg_read_at_phys(WindCtx *ctx, uint8_t *buf, const ut64 offset, const int count);
int windbg_write_at(WindCtx *ctx, const uint8_t *buf, const ut64 offset, const int count);
int windbg_write_at_uva(WindCtx *ctx, const uint8_t *buf, ut64 offset, int count);
int windbg_write_at_phys(WindCtx *ctx, const uint8_t *buf, const ut64 offset, const int count);
bool windbg_va_to_pa(WindCtx *ctx, ut64 va, ut64 *pa);
void windbg_break(void *ctx);
int windbg_break_read(WindCtx *ctx);
bool windbg_lock_enter(WindCtx *ctx);
bool windbg_lock_leave(WindCtx *ctx);
bool windbg_lock_tryenter(WindCtx *ctx);
