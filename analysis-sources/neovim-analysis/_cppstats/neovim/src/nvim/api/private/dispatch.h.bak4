#if !defined(NVIM_API_PRIVATE_DISPATCH_H)
#define NVIM_API_PRIVATE_DISPATCH_H

#include "nvim/api/private/defs.h"

typedef Object (*ApiDispatchWrapper)(uint64_t channel_id,
Array args,
Error *error);



typedef struct {
ApiDispatchWrapper fn;
bool fast; 



} MsgpackRpcRequestHandler;

#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "api/private/dispatch.h.generated.h"
#include "api/private/dispatch_wrappers.h.generated.h"
#endif

#endif 
