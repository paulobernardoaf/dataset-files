#include <stdint.h>
#include <stdbool.h>
#include <uv.h>
#include "nvim/event/loop.h"
#include "nvim/event/stream.h"
typedef struct wbuffer WBuffer;
typedef void (*wbuffer_data_finalizer)(void *data);
struct wbuffer {
size_t size, refcount;
char *data;
wbuffer_data_finalizer cb;
};
#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "event/wstream.h.generated.h"
#endif
