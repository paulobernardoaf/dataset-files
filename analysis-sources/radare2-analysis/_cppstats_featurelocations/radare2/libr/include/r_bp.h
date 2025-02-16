#if !defined(R2_BP_H)
#define R2_BP_H

#include <r_types.h>
#include <r_lib.h>
#include <r_io.h>
#include <r_list.h>

#if defined(__cplusplus)
extern "C" {
#endif

R_LIB_VERSION_HEADER(r_bp);

#define R_BP_MAXPIDS 10
#define R_BP_CONT_NORMAL 0
#define R_BP_CONT_NORMAL 0

typedef struct r_bp_arch_t {
int bits;
int length;
int endian;
const ut8 *bytes;
} RBreakpointArch;

enum {
R_BP_TYPE_SW,
R_BP_TYPE_HW,
R_BP_TYPE_COND,
R_BP_TYPE_FAULT,
R_BP_TYPE_DELETE,
};

typedef struct r_bp_plugin_t {
char *name;
char *arch;
int type; 
int nbps;
RBreakpointArch *bps;
} RBreakpointPlugin;

typedef struct r_bp_item_t {
char *name;
char *module_name; 
st64 module_delta; 
ut64 addr;
ut64 delta;
int size; 
int recoil; 
bool swstep; 
int perm;
int hw;
int trace;
int internal; 
int enabled;
int togglehits; 
int hits;
ut8 *obytes; 
ut8 *bbytes; 
int pids[R_BP_MAXPIDS];
char *data;
char *cond; 
char *expr; 
} RBreakpointItem;

struct r_bp_t;
typedef int (*RBreakpointCallback)(struct r_bp_t *bp, RBreakpointItem *b, bool set);

typedef struct r_bp_t {
void *user;
int stepcont;
int endian;
int bits;
bool bpinmaps; 
RCoreBind corebind;
RIOBind iob; 
RBreakpointPlugin *cur;
RList *traces; 
RList *plugins;
PrintfCallback cb_printf;
RBreakpointCallback breakpoint;

int nbps;
int nhwbps;
RList *bps; 
RBreakpointItem **bps_idx;
int bps_idx_count;
st64 delta;
ut64 baddr;
} RBreakpoint;


enum {
R_BP_PROT_EXEC = 1,
R_BP_PROT_WRITE = 2,
R_BP_PROT_READ = 4,
R_BP_PROT_ACCESS = 8,
};

typedef struct r_bp_trace_t {
ut64 addr;
ut64 addr_end;
ut8 *traps;
ut8 *buffer;
ut8 *bits;
int length;
int bitlen;
} RBreakpointTrace;

#if defined(R_API)
R_API RBreakpoint *r_bp_new(void);
R_API RBreakpoint *r_bp_free(RBreakpoint *bp);

R_API int r_bp_del(RBreakpoint *bp, ut64 addr);
R_API int r_bp_del_all(RBreakpoint *bp);

R_API int r_bp_plugin_add(RBreakpoint *bp, RBreakpointPlugin *foo);
R_API int r_bp_use(RBreakpoint *bp, const char *name, int bits);
R_API int r_bp_plugin_del(RBreakpoint *bp, const char *name);
R_API void r_bp_plugin_list(RBreakpoint *bp);

R_API int r_bp_in(RBreakpoint *bp, ut64 addr, int perm);

R_API int r_bp_list(RBreakpoint *bp, int rad);
R_API int r_bp_size(RBreakpoint *bp);


R_API int r_bp_get_bytes(RBreakpoint *bp, ut8 *buf, int len, int endian, int idx);
R_API int r_bp_set_trace(RBreakpoint *bp, ut64 addr, int set);
R_API int r_bp_set_trace_all(RBreakpoint *bp, int set);
R_API RBreakpointItem *r_bp_enable(RBreakpoint *bp, ut64 addr, int set, int count);
R_API int r_bp_enable_all(RBreakpoint *bp, int set);


R_API int r_bp_del_index(RBreakpoint *bp, int idx);
R_API RBreakpointItem *r_bp_get_index(RBreakpoint *bp, int idx);
R_API int r_bp_get_index_at (RBreakpoint *bp, ut64 addr);
R_API RBreakpointItem *r_bp_item_new (RBreakpoint *bp);

R_API RBreakpointItem *r_bp_get_at (RBreakpoint *bp, ut64 addr);
R_API RBreakpointItem *r_bp_get_in (RBreakpoint *bp, ut64 addr, int perm);

R_API bool r_bp_is_valid(RBreakpoint *bp, RBreakpointItem *b);

R_API int r_bp_add_cond(RBreakpoint *bp, const char *cond);
R_API int r_bp_del_cond(RBreakpoint *bp, int idx);
R_API int r_bp_add_fault(RBreakpoint *bp, ut64 addr, int size, int perm);

R_API RBreakpointItem *r_bp_add_sw(RBreakpoint *bp, ut64 addr, int size, int perm);
R_API RBreakpointItem *r_bp_add_hw(RBreakpoint *bp, ut64 addr, int size, int perm);
R_API void r_bp_restore_one(RBreakpoint *bp, RBreakpointItem *b, bool set);
R_API int r_bp_restore(RBreakpoint *bp, bool set);
R_API bool r_bp_restore_except(RBreakpoint *bp, bool set, ut64 addr);


R_API void r_bp_traptrace_free(void *ptr);
R_API void r_bp_traptrace_enable(RBreakpoint *bp, int enable);
R_API void r_bp_traptrace_reset(RBreakpoint *bp, int hard);
R_API ut64 r_bp_traptrace_next(RBreakpoint *bp, ut64 addr);
R_API int r_bp_traptrace_add(RBreakpoint *bp, ut64 from, ut64 to);
R_API int r_bp_traptrace_free_at(RBreakpoint *bp, ut64 from);
R_API void r_bp_traptrace_list(RBreakpoint *bp);
R_API int r_bp_traptrace_at(RBreakpoint *bp, ut64 from, int len);
R_API RList *r_bp_traptrace_new(void);
R_API void r_bp_traptrace_enable(RBreakpoint *bp, int enable);


R_API RBreakpointItem *r_bp_watch_add(RBreakpoint *bp, ut64 addr, int size, int hw, int rw);


extern RBreakpointPlugin r_bp_plugin_x86;
extern RBreakpointPlugin r_bp_plugin_arm;
extern RBreakpointPlugin r_bp_plugin_mips;
extern RBreakpointPlugin r_bp_plugin_ppc;
extern RBreakpointPlugin r_bp_plugin_sh;
extern RBreakpointPlugin r_bp_plugin_bf;
#endif
#if defined(__cplusplus)
}
#endif

#endif
