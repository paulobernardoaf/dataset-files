#include <stdbool.h>
#include <uv.h>
#include "nvim/api/private/defs.h"
#include "nvim/event/socket.h"
#include "nvim/event/process.h"
#include "nvim/vim.h"
#include "nvim/channel.h"
#define METHOD_MAXLEN 512
EXTERN MultiQueue *ch_before_blocking_events INIT(= NULL);
#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "msgpack_rpc/channel.h.generated.h"
#endif
