#include <msgpack.h>
#include "nvim/api/private/defs.h"
#include "nvim/lib/kvec.h"
typedef struct {
msgpack_sbuffer regs; 
msgpack_sbuffer jumps; 
msgpack_sbuffer bufs; 
msgpack_sbuffer gvars; 
Array funcs; 
} Context;
typedef kvec_t(Context) ContextVec;
#define MSGPACK_SBUFFER_INIT (msgpack_sbuffer) { .size = 0, .data = NULL, .alloc = 0, }
#define CONTEXT_INIT (Context) { .regs = MSGPACK_SBUFFER_INIT, .jumps = MSGPACK_SBUFFER_INIT, .bufs = MSGPACK_SBUFFER_INIT, .gvars = MSGPACK_SBUFFER_INIT, .funcs = ARRAY_DICT_INIT, }
typedef enum {
kCtxRegs = 1, 
kCtxJumps = 2, 
kCtxBufs = 4, 
kCtxGVars = 8, 
kCtxSFuncs = 16, 
kCtxFuncs = 32, 
} ContextTypeFlags;
extern int kCtxAll;
#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "context.h.generated.h"
#endif
